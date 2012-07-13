#include "editor.h"
#include "ui_editor.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QFile>
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>

#include "../model/arena.h"
#include "../model/arenaelement.h"
#include "../model/arenaelementtype.h"
#include "../model/arenaelementtyperegistry.h"

#include "arenascene.h"
#include "arenaview.h"
#include "arenasceneelement.h"
#include "arenaelementtypescene.h"

#include "../global.h"

Editor::Editor(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
    , m_loadingSuccessful(false)
{
    m_ui->setupUi(this);

    if (parseRosPackageDirsFromCommandLineArguments())
    {
        qDebug() << "[Rescue Arena Designer] Successfully parsed hector_arena_gui and hector_arena_elements package dirs from command line arguments.";
    }
    else
    {
        qDebug() << "[Rescue Arena Designer] Using rospack to find hector_arena_gui and hector_arena_elements packages.";
        m_hector_arena_gui_package_dir = findRosPackage("hector_nist_arena_designer");
        // findRosPackage() pops up an error message if necessary
        if (m_hector_arena_gui_package_dir.isEmpty())
            qApp->quit();

        m_hector_arena_elements_package_dir = findRosPackage("hector_nist_arena_elements");
        if (m_hector_arena_elements_package_dir.isEmpty())
            qApp->quit();

        m_hector_arena_worlds_package_dir = findRosPackage("hector_nist_arena_worlds");
        if (m_hector_arena_worlds_package_dir.isEmpty())
            qApp->quit();
    }

    QString arenaElementDir = m_hector_arena_elements_package_dir + "/elements";

    m_openSaveDir = m_hector_arena_worlds_package_dir + "/arenas";
    m_exportDir = m_hector_arena_worlds_package_dir + "/worlds";

    m_typeRegistry = new ArenaElementTypeRegistry(arenaElementDir);
    m_arena = new Arena(m_typeRegistry);

    m_typeScene = new ArenaElementTypeScene(m_typeRegistry);
    m_arenaScene = new ArenaScene(m_arena);
    m_controller = new ArenaController(m_arena, m_arenaScene);

    m_ui->elementView->setScene(m_typeScene);

    m_arenaView = new ArenaView(m_controller, this);
    m_arenaView->setScene(m_arenaScene);
    m_arenaView->setArena(m_arena);
    m_arenaView->setAcceptDrops(true);
    m_arenaView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    m_ui->arenaViewContainer->layout()->addWidget(m_arenaView);
    m_ui->infoRot->setTextFormat(Qt::RichText);

    qDebug() << "[Rescue Arena Designer] hector_arena_gui = " << m_hector_arena_gui_package_dir;
    qDebug() << "[Rescue Arena Designer] hector_arena_elements = " << m_hector_arena_elements_package_dir;
    qDebug() << "[Rescue Arena Designer] hector_arena_worlds = " << m_hector_arena_worlds_package_dir;

    loadConfig(m_hector_arena_gui_package_dir + "/config.xml");

    connect(m_ui->actionRotateClockwise, SIGNAL(triggered()),
            this, SLOT(slotRotateClockwise()));

    connect(m_ui->actionRotateCounterClockwise, SIGNAL(triggered()),
            this, SLOT(slotRotateCounterClockwise()));

    connect(m_ui->actionRemove, SIGNAL(triggered()),
            this, SLOT(slotRemove()));

    connect(m_ui->infoItemMountPoint, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotItemMountPointChanged(int)));


    connect(m_ui->actionNew, SIGNAL(triggered()),
            this, SLOT(slotNew()));

    connect(m_ui->actionOpen, SIGNAL(triggered()),
            this, SLOT(slotOpen()));

    connect(m_ui->actionSave, SIGNAL(triggered()),
            this, SLOT(slotSave()));

    connect(m_ui->actionSaveAs, SIGNAL(triggered()),
            this, SLOT(slotSaveAs()));

    connect(m_ui->actionExport, SIGNAL(triggered()),
            this, SLOT(slotExport()));

    connect(m_ui->actionShowDocumentation, SIGNAL(triggered()),
            this, SLOT(slotShowDocumentation()));

    connect(m_arena, SIGNAL(modified()),
            this, SLOT(slotSelectionChanged()));

    connect(m_arenaScene, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(m_ui->actionZoomIn, SIGNAL(triggered()),
            m_arenaView, SLOT(slotZoomIn()));

    connect(m_ui->actionZoomOut, SIGNAL(triggered()),
            m_arenaView, SLOT(slotZoomOut()));

    connect(m_typeScene, SIGNAL(elementHovered(ArenaElement*)),
            this, SLOT(slotElementTypeHovered(ArenaElement*)));


    connect(m_arena, SIGNAL(modified()),
            this, SLOT(slotModified()));

    setUnifiedTitleAndToolBarOnMac(true);

    m_windowTitlePrefix = windowTitle();
    setWindowTitle(m_windowTitlePrefix + " - untitled.raf[*]");

    slotSelectionChanged();
    setWindowModified(false);
}

QString Editor::findRosPackage(const QString& name)
{
    QString result;

    QProcess rosFind;
    rosFind.start("rospack", QStringList() << "find" << name);
    if (!rosFind.waitForFinished())
    {
        if (rosFind.error() == QProcess::FailedToStart)
            QMessageBox::critical(this, "Can't find rospack",
                "The program 'rospack' is missing or not in your PATH. Check your ROS installation.");
        else
            QMessageBox::critical(this, "Can't find " + name + " ROS package",
                "The program 'rospack' used to locate ROS packages didn't terminate correctly. Check your ROS installation.");
        return "";
    }

    QByteArray rosFindOutput = rosFind.readAll();
    QList<QByteArray> rosFindOutputLines = rosFindOutput.split('\n');
    foreach (QByteArray line, rosFindOutputLines)
    {
        // last line also ends with "\n", thus there is one extra empty line that we'll ignore
        if (line.isEmpty())
            continue;
        // Debug output from rospack starts with [rospack]
        if (line.startsWith("["))
                continue;
        result = line;
    }

    if (result.isEmpty())
    {
        QMessageBox::critical(this, "Can't find " + name + " ROS package",
            "The program 'rospack' could not locate the package " + name + ". Check your installation.");
        return "";
    }

    return result;
}

bool Editor::parseRosPackageDirsFromCommandLineArguments()
{
    // First check if any arguments were specified
    if (qApp->argc() < 2)
        return false;

    QString gui_dir, elements_dir;
    QStringList args = qApp->arguments();
    // Ignore program name (first argument)
    args.removeFirst();
    foreach (QString arg, args)
    {
        QStringList splitted = arg.split('=');
        if (splitted.size() != 2)
            continue;
        if (splitted.at(0) == "-hector_arena_gui_dir")
            gui_dir = splitted.at(1);
        else if (splitted.at(0) == "-hector_arena_elements_dir")
            elements_dir = splitted.at(1);
    }

    if (!gui_dir.isEmpty() && !elements_dir.isEmpty())
    {
        if (!QDir(gui_dir).exists())
        {
            QMessageBox::critical(this, "Invalid path name",
                "The path " + gui_dir + " doesn't exist. Check your -hector_arena_gui command line argument. Falling back to 'rospack find'...");
            return false;
        }
        if (!QDir(elements_dir).exists())
        {
            QMessageBox::critical(this, "Invalid path name",
                "The path " + elements_dir + " doesn't exist. Check your -hector_arena_elements command line argument. Falling back to 'rospack find'...");
            return false;
        }
        m_hector_arena_gui_package_dir = gui_dir;
        m_hector_arena_elements_package_dir = elements_dir;
        return true;
    }
    else
    {
        qDebug() << "[Rescue Arena Designer] Invalid command line arguments:";
        qDebug() << "[Rescue Arena Designer]" << args;
    }
    return false;
}

void Editor::loadConfig(const QString &configFile)
{
    QFile in(configFile);
    in.open(QFile::ReadOnly);
    QDomDocument doc;
    bool configValid = doc.setContent(&in);
    in.close();

    if (configValid)
    {
        QDomElement rootElement = doc.firstChildElement();
        Q_ASSERT(rootElement.tagName() == "config");

        QDomNodeList childNodes = rootElement.childNodes();
        for (int i = 0; i < childNodes.count(); i++)
        {
            QDomElement childElement = childNodes.at(i).toElement();
            if (childElement.tagName() == "arena-elements")
            {
                m_typeScene->loadConfig(childElement);
            }
        }
        m_loadingSuccessful = true;
    }
    else
    {
        QMessageBox::warning(this, "Failed to load config.xml", "Your config.xml seems to be an invalid XML file. Make sure it is valid and try again.");
    }
}

Editor::~Editor()
{
    delete m_typeScene;
    delete m_arenaScene;
    delete m_arena;
    // This must be deleted last because it receives signals when e.g. an element
    // is removed
    delete m_ui;
}

void Editor::slotRotateClockwise()
{
    foreach (ArenaSceneElement *selection, m_arenaScene->selectedElements())
    {
        ArenaElement *element = selection->element();
        element->setRotation(element->rotation() + 90);
    }
}

void Editor::slotRotateCounterClockwise()
{
    foreach (ArenaSceneElement *selection, m_arenaScene->selectedElements())
    {
        ArenaElement *element = selection->element();
        element->setRotation(element->rotation() - 90);
    }
}

void Editor::slotRemove()
{
    foreach (ArenaSceneElement *selection, m_arenaScene->selectedElements())
    {
        ArenaElement *element = selection->element();
        m_arena->removeElement(element);
        delete element;
    }
}

void Editor::slotSelectionChanged()
{
    QList<ArenaSceneElement*> _selectedElements = m_arenaScene->selectedElements();
    bool hasSingleSelection = _selectedElements.count() == 1;
    m_ui->infoWidget->setVisible(hasSingleSelection);
    m_ui->infoHintLabel->setVisible(!hasSingleSelection);
    if (!hasSingleSelection)
        return;

    ArenaSceneElement *selection = dynamic_cast<ArenaSceneElement*>(_selectedElements.first());

    m_ui->infoWidget->show();
    m_ui->infoHintLabel->hide();

    ArenaElement *element = selection->element();
    const ArenaElementType *type = element->type();

    QString name;
    QTextStream(&name) << "(" << element->pos().x() << ", " << element->pos().y() << ")";
    m_ui->infoPos->setText(name);

    QString rot;
    QTextStream(&rot) << element->rotation() << "&deg;";
    m_ui->infoRot->setText(rot);

    if (element->isItem())
    {
        m_ui->infoOffsetLabel->setVisible(true);
        m_ui->infoOffset->setVisible(true);
        QString offsetText;
        QTextStream stream(&offsetText);
        stream.setRealNumberNotation(QTextStream::FixedNotation);
        stream.setRealNumberPrecision(3);
        stream << "(" << element->itemOffset().x() << ", " << element->itemOffset().y() << ")";
        m_ui->infoOffset->setText(offsetText);
    }
    else
    {
        m_ui->infoOffsetLabel->setVisible(false);
        m_ui->infoOffset->setVisible(false);
    }

    if (element->isMountableItem())
    {
        m_ui->infoItemMountPointLabel->setVisible(true);
        m_ui->infoItemMountPoint->setVisible(true);
        m_ui->infoItemMountPoint->clear();
        ArenaElement *contextElement = m_arena->contextElement(element);
        m_ui->infoItemMountPoint->setEnabled(contextElement);
        if (contextElement)
        {
            // Block signals, otherwise modifying the item mount point list will
            // cause an infinite loop (slotSelectionChanged() is called whenever
            // something in the arena changed, such as the item mount point index of
            // an element)
            m_ui->infoItemMountPoint->blockSignals(true);
            QList<ItemMountPoint> itemMountPoints = contextElement->type()->itemMountPoints();
            foreach (ItemMountPoint hole, itemMountPoints)
                m_ui->infoItemMountPoint->addItem(hole.first);
            if (element->itemMountPoint() >= 0)
                m_ui->infoItemMountPoint->setCurrentIndex(element->itemMountPoint());
            m_ui->infoItemMountPoint->blockSignals(false);
        }
    }
    else
    {
        m_ui->infoItemMountPointLabel->setVisible(false);
        m_ui->infoItemMountPoint->setVisible(false);
    }

    m_ui->metaInfoBox->setMetaInfos(type->metaInfos());
}

void Editor::slotItemMountPointChanged(int index)
{
    if (index < 0)
        return;
    QList<ArenaSceneElement*> _selectedElements = m_arenaScene->selectedElements();
    bool hasSingleSelection = _selectedElements.count() == 1;
    if (!hasSingleSelection)
        return;

    ArenaSceneElement *selection = dynamic_cast<ArenaSceneElement*>(_selectedElements.first());
    ArenaElement *element = selection->element();

    element->setItemMountPoint(index);
}

void Editor::askToSaveChangesIfModified()
{
    if (isWindowModified())
    {
        QString text = "Do you want to save changes to the current arena?";
        if (QMessageBox::question(this, "Save Arena?", text, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
            slotSave();
    }
}

void Editor::slotModified()
{
    setWindowModified(true);
}

void Editor::slotNew()
{
    askToSaveChangesIfModified();
    m_arena->clear();
    m_currentArenaFile.clear();

    setWindowModified(false);
}

void Editor::slotOpen()
{
    askToSaveChangesIfModified();
    QString target = QFileDialog::getOpenFileName(this, "Select arena file to open", m_openSaveDir, ".raf File (*.raf)");
    if (!target.isEmpty())
    {
        m_arena->clear();
        setCurrentArenaFile(target);
        m_arena->load(m_currentArenaFile);
        setWindowModified(false);
    }
}

void Editor::slotSave()
{
    if (!m_currentArenaFile.isEmpty())
        m_arena->save(m_currentArenaFile);
    else
        slotSaveAs();

    setWindowModified(false);
}

void Editor::slotSaveAs()
{
    QString target = QFileDialog::getSaveFileName(this, "Select file to save arena to", m_openSaveDir, ".raf File (*.raf)");
    if (!target.isEmpty())
    {
        m_currentArenaFile = target;
        m_arena->save(m_currentArenaFile);
        setWindowModified(false);
    }
}

void Editor::slotExport()
{
    QString target = QFileDialog::getSaveFileName(this, "Select file to export arena to", m_exportDir, "Gazebo .world File (*.world)");
    if (!target.isEmpty())
    {
        m_arena->saveWorld(target);
        m_exportDir = QFileInfo(target).path();
    }
}

void Editor::slotShowDocumentation()
{
    QString indexHtml = QString("file://") + m_hector_arena_gui_package_dir + "/doc/index.html";
    QDesktopServices::openUrl(indexHtml);
}

void Editor::slotElementTypeHovered(ArenaElement *element)
{
    // Show status bar info for 1s
    statusBar()->showMessage(element->type()->humanReadableName(), 1000);
}

void Editor::setCurrentArenaFile(QString fileName)
{
    m_currentArenaFile = fileName;
    setWindowTitle(m_windowTitlePrefix + " - " + QFileInfo(fileName).fileName() + "[*]");

    m_openSaveDir = QFileInfo(m_currentArenaFile).path();
}

void Editor::closeEvent(QCloseEvent *event)
{
    askToSaveChangesIfModified();

    QMainWindow::closeEvent(event);
}
