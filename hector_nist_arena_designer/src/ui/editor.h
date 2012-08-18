#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDomElement>

namespace Ui {
    class MainWindow;
}

class QGraphicsScene;
class QCloseEvent;

class Arena;
class ArenaController;
class ArenaElement;
class ArenaScene;
class ArenaView;
class ArenaElementTypeRegistry;
class ArenaElementTypeScene;

class Editor : public QMainWindow
{
    Q_OBJECT

public:
    Editor(QWidget *parent = 0);
    ~Editor();

    void loadConfig(const QString& configFile);

    bool loadingSuccessful() { return m_loadingSuccessful; }

public slots:
    // -- Element Operations --
    void slotRotateClockwise();
    void slotRotateCounterClockwise();
    void slotRemove();
    void slotItemMountPointChanged(int index);

    // -- File Operations --
    void slotNew();
    void slotOpen();
    void slotSave();
    void slotSaveAs();
    void slotExport();
    void slotExportSdf();
    void slotSelectionChanged();

    // -- Help Menu --
    void slotShowDocumentation();

    // -- Views --
    void slotElementTypeHovered(ArenaElement *element);

private slots:
    void slotModified();

private:
    /// Returns a path to the specified ROS package. The returned is empty if
    /// package wasn't found.
    QString findRosPackage(const QString& name);
    /// Tries to determine ROS package dirs of hector_arena_gui and hector_arena_elements
    /// packages from command line arguments
    bool parseRosPackageDirsFromCommandLineArguments();
    void askToSaveChangesIfModified();
    void setCurrentArenaFile(QString fileName);
    void closeEvent(QCloseEvent *event);

    Ui::MainWindow *m_ui;

    bool m_loadingSuccessful;
    // Loaded from config.xml
    QString m_exportDir;
    QString m_openSaveDir;
    QString m_currentArenaFile;
    Arena *m_arena;
    ArenaController *m_controller;
    ArenaElementTypeScene *m_typeScene;
    ArenaScene *m_arenaScene;
    ArenaView *m_arenaView;
    ArenaElementTypeRegistry *m_typeRegistry;

    QString m_hector_arena_gui_package_dir;
    QString m_hector_arena_elements_package_dir;
    QString m_hector_arena_worlds_package_dir;

    // Default window title set in UI file
    QString m_windowTitlePrefix;
};

#endif // MAINWINDOW_H
