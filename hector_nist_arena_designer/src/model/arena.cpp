#include <hector_nist_arena_designer/model/arena.h>

#include <hector_nist_arena_designer/model/arenaelement.h>
#include <hector_nist_arena_designer/model/arenaelementtype.h>
#include <hector_nist_arena_designer/model/arenaelementtyperegistry.h>
#include <hector_nist_arena_designer/model/xmlloadingexception.h>

#include <QFile>
#include <QDomDocument>
#include <QDebug>
#include <QList>
#include <QDomNodeList>
#include <QMessageBox>

Arena::Arena(ArenaElementTypeRegistry* typeRegistry)
    : m_typeRegistry(typeRegistry)
{
}

Arena::~Arena()
{
}

void Arena::load(const QString& filename)
{
    QFile in(filename);
    in.open(QFile::ReadOnly);

    QDomDocument doc;
    doc.setContent(&in);

    in.close();

    // Only if loading was successful we will add these elements
    QList<ArenaElement*> loadedElements;
    bool loadingSuccessful = true;

    try
    {
        QDomNodeList nodes = doc.elementsByTagName("element");
        for(int i = 0; i < nodes.count(); i++)
        {
            QDomElement node = nodes.at(i).toElement();
            QString type = node.attribute("type");
            if (type.isEmpty())
            {
                qDebug() << "[Rescue Arena Designer] Error: Empty element type"
                         << "specified in" << filename << ". Ignoring.";
                continue;
            }

            if (!m_typeRegistry->hasElement(type))
            {
                qDebug() << "[Rescue Arena Designer] Error: Unknown element type"
                         << type << "in" << filename << ". Ignoring.";
                continue;
            }
            ArenaElement *element = m_typeRegistry->instantiateElement(type);
            Q_ASSERT(element);
            loadedElements.append(element);

            element->load(node);
        }
    }
    catch (XmlLoadingException e)
    {
        loadingSuccessful = false;
        QString msg;
        QTextStream(&msg) << "There's an error in the arena XML file in line "
                          << e.lineNumber() << ", column " << e.columnNumber()
                          << ":\n" << e.msg();
        QMessageBox msgBox(QMessageBox::Critical,
                           QString("Error while loading arena XML file"),
                           msg);
        msgBox.exec();
    }

    // Finish loading
    if (loadingSuccessful)
    {
        foreach (ArenaElement *element, loadedElements)
        {
            addElement(element);
        }
    }
}

void Arena::save(const QString& filename)
{
    QFile out(filename);
    out.open(QFile::WriteOnly);

    if (out.error() != QFile::NoError)
        qDebug() << "[Hector Arena Designer] Error saving arena file:" << out.errorString();

    QXmlStreamWriter writer(&out);
    // Insert line-breaks for readability
    writer.setAutoFormatting(true);

    writer.writeStartDocument();
    writer.writeStartElement("elements");

    foreach (ArenaElement *element, m_elements)
        element->save(writer);

    writer.writeEndElement();
    writer.writeEndDocument();

    out.close();
}

void Arena::saveWorld(const QString& filename)
{
    QFile out(filename);
    out.open(QFile::WriteOnly);

    QXmlStreamWriter writer(&out);
    // Insert line-breaks for readability
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(2);

    writer.writeStartDocument();

    writer.writeStartElement("sdf");
    writer.writeAttribute("version", "1.4");

    writer.writeStartElement("world");
    writer.writeAttribute("name", "default");

    writer.writeStartElement("plugin");
    writer.writeAttribute("filename", "gz-sim-imu-system");
    writer.writeAttribute("name", "gz::sim::systems::Imu");
    writer.writeEndElement(); // plugin

    writer.writeStartElement("plugin");
    writer.writeAttribute("filename", "gz-sim-physics-system");
    writer.writeAttribute("name", "gz::sim::systems::Physics");
    writer.writeEndElement(); // plugin

    writer.writeStartElement("plugin");
    writer.writeAttribute("filename", "gz-sim-scene-broadcaster-system");
    writer.writeAttribute("name", "gz::sim::systems::SceneBroadcaster");
    writer.writeEndElement(); // plugin

    writer.writeStartElement("plugin");
    writer.writeAttribute("filename", "gz-sim-sensors-system");
    writer.writeAttribute("name", "gz::sim::systems::Sensors");
    writer.writeTextElement("render_engine", "ogre2");
    writer.writeEndElement(); // plugin

    writer.writeStartElement("plugin");
    writer.writeAttribute("filename", "gz-sim-user-commands-system");
    writer.writeAttribute("name", "gz::sim::systems::UserCommands");
    writer.writeEndElement(); // plugin

    writer.writeStartElement("scene");
    writer.writeTextElement("ambient", "0.7 0.7 0.7 1");
    writer.writeTextElement("background","0.8 0.8 1 1");

    writer.writeStartElement("sky");
    writer.writeTextElement("time", "8");
    writer.writeEndElement(); // sky
    
    writer.writeEndElement(); // scene

    writer.writeStartElement("physics");
    writer.writeAttribute("type", "ode");

    writer.writeTextElement("gravity", "0 0 -9.81");

    writer.writeStartElement("ode");

    writer.writeStartElement("solver");
    writer.writeTextElement("type","quick");
    writer.writeTextElement("iters","10");
    writer.writeTextElement("sor","1.3");
    writer.writeEndElement(); // solver

    writer.writeStartElement("constraints");
    writer.writeTextElement("cfm","0.0");
    writer.writeTextElement("erp","0.2");
    writer.writeTextElement("contact_max_correcting_vel","100.0");
    writer.writeTextElement("contact_surface_layer","0.001");
    writer.writeEndElement(); // constraints

    writer.writeEndElement(); // ode

    writer.writeTextElement("real_time_update_rate", "1000");
    writer.writeTextElement("max_step_size", "0.001");

    writer.writeEndElement(); // physics

    writer.writeStartElement("light");
    writer.writeAttribute("name","directional_light_1");
    writer.writeAttribute("type","directional");
    writer.writeTextElement("cast_shadows","false");

    writer.writeTextElement("pose","0 0 30 0.1 0.1 0");
    writer.writeTextElement("diffuse", "1 1 1 1");
    writer.writeTextElement("specular", ".1 .1 .1 1");

    writer.writeStartElement("attenuation");
    writer.writeTextElement("range", "300");
    writer.writeEndElement(); // attenuation

    writer.writeTextElement("direction", "0.1 0.1 -1");

    writer.writeEndElement(); // light

    writer.writeStartElement("model");
    writer.writeAttribute("name", "plane_model");
    writer.writeTextElement("static", "true");

    writer.writeStartElement("link");
    writer.writeAttribute("name","plane_model_link");

    writer.writeStartElement("collision");
    writer.writeAttribute("name","plane_model_collision");

    writer.writeStartElement("geometry");

    writer.writeStartElement("plane");
    writer.writeTextElement("normal", "0 0 1");
    writer.writeEndElement(); // plane

    writer.writeEndElement(); //geometry

    writer.writeStartElement("surface");

    writer.writeStartElement("friction");

    writer.writeStartElement("ode");
    writer.writeTextElement("mu","10.0");
    writer.writeTextElement("mu2","10.0");
    writer.writeTextElement("fdir1","0 0 0");
    writer.writeTextElement("slip1","0");
    writer.writeTextElement("slip2","0");
    writer.writeEndElement(); //ode

    writer.writeEndElement(); //friction

    writer.writeStartElement("bounce");
    writer.writeTextElement("restitution_coefficient","0");
    writer.writeTextElement("threshold","1e6");
    writer.writeEndElement(); //bounce

    writer.writeStartElement("contact");

    writer.writeStartElement("ode");
    writer.writeTextElement("soft_cfm","0");
    writer.writeTextElement("soft_erp","0.2");
    writer.writeTextElement("kp","1e10");
    writer.writeTextElement("kd","1");
    writer.writeTextElement("max_vel","100");
    writer.writeTextElement("min_depth","0.0001");
    writer.writeEndElement(); //ode

    writer.writeEndElement(); //contact
    writer.writeEndElement(); //surface

    writer.writeTextElement("max_contacts", "10");

    writer.writeEndElement(); //collision


    writer.writeStartElement("visual");
    writer.writeAttribute("name","plane_model_visual");
    writer.writeTextElement("cast_shadows","false");

    writer.writeStartElement("geometry");

    bool use_textured_ground_plane = true;
    
    if (!use_textured_ground_plane){
      writer.writeStartElement("plane");
      writer.writeTextElement("normal","0 0 1");
      writer.writeEndElement();
    } else {
      writer.writeStartElement("mesh");
      writer.writeTextElement("uri","model://ground_plane.dae");
      writer.writeTextElement("scale","1 1 1");
      writer.writeEndElement(); // mesh
    }
    
    writer.writeEndElement(); //geometry

    writer.writeStartElement("material");

    writer.writeStartElement("script");
    writer.writeTextElement("uri", "file://media/materials/scripts/gazebo.material");
    writer.writeTextElement("name", "Gazebo/Grey");
    writer.writeEndElement(); // script

    writer.writeEndElement(); // material
    writer.writeEndElement(); //visual
    writer.writeEndElement(); //link
    writer.writeEndElement(); //model

    foreach (ArenaElement *element, m_elements)
        element->saveWorldSdf(writer);

    writer.writeEndElement(); // world
    writer.writeEndElement(); // sdf

    writer.writeEndDocument();

    out.close();
}

void Arena::saveWorldXacro(const QString& filename)
{
    QFile out(filename);
    out.open(QFile::WriteOnly);

    QXmlStreamWriter writer(&out);
    // Insert line-breaks for readability
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(2);

    writer.writeStartDocument();

    writer.writeStartElement("sdf");
    writer.writeAttribute("version", "1.11");
    writer.writeAttribute("xmlns:xacro", "http://www.ros.org/wiki/xacro");

    // TODO? Property overrides

    writer.writeStartElement("xacro:macro");
    writer.writeAttribute("name", "world_data");
    writer.writeAttribute("params", "");

    for (ArenaElement* element : m_elements)
        element->saveWorldSdf(writer);

    writer.writeEndElement(); // xacro:macro
    writer.writeEndElement(); // sdf

    writer.writeEndDocument();

    out.close();
}

void Arena::slotModified()
{
    emit modified();
}

void Arena::addElement(ArenaElement *element)
{
    Q_ASSERT(!m_elements.contains(element));
    m_elements.append(element);

    connect(element, SIGNAL(posChanged(ArenaElement*,QPoint)),
            this, SLOT(slotModified()));
    connect(element, SIGNAL(rotationChanged(ArenaElement*,int)),
            this, SLOT(slotModified()));
    connect(element, SIGNAL(modified(ArenaElement*)),
            this, SIGNAL(modified()));

    element->setArena(this);

    emit elementAdded(element);
}

void Arena::removeElement(ArenaElement *element)
{
    Q_ASSERT(m_elements.contains(element));
    m_elements.removeAll(element);

    emit elementRemoved(element);
}

void Arena::clear()
{
    foreach (ArenaElement *element, m_elements)
    {
        m_elements.removeAll(element);
        emit elementRemoved(element);
    }
    m_elements.clear();

    emit modified();
}

ArenaElement* Arena::addElement(const QString &elementType)
{
    ArenaElement *element = m_typeRegistry->instantiateElement(elementType);
    if (element)
        addElement(element);
    return element;
}

QList<ArenaElement*> Arena::elementsAt(const QPoint &pos) const
{
    QList<ArenaElement*> res;
    foreach (ArenaElement* e, m_elements)
        if (e->pos() == pos)
            res.push_back(e);
    return res;
}

ArenaElement* Arena::contextElement(ArenaElement *element) const
{
    QList<ArenaElement*> elementsAtPos = elementsAt(element->pos());
    foreach (ArenaElement *e, elementsAtPos)
        if (e != element &&
            e->type()->type() == ArenaElementType::WallType &&
            e->rotation() == element->rotation())
            return e;
    return 0;
}
