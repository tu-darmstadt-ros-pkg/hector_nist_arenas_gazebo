#include "arena.h"

#include "arenaelement.h"
#include "arenaelementtype.h"
#include "arenaelementtyperegistry.h"
#include "xmlloadingexception.h"

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

    writer.writeStartDocument();

    writer.writeStartElement("gazebo:world");
    writer.writeAttribute("xmlns:gazebo", "http://playerstage.sourceforge.net/gazebo/xmlschema/#gz");
    writer.writeAttribute("xmlns:model", "http://playerstage.sourceforge.net/gazebo/xmlschema/#model");
    writer.writeAttribute("xmlns:body", "http://playerstage.sourceforge.net/gazebo/xmlschema/#body");
    writer.writeAttribute("xmlns:geom", "http://playerstage.sourceforge.net/gazebo/xmlschema/#geom");
    writer.writeAttribute("xmlns:physics", "http://playerstage.sourceforge.net/gazebo/xmlschema/#physics");
    writer.writeAttribute("xmlns:rendering", "http://playerstage.sourceforge.net/gazebo/xmlschema/#rendering");

    writer.writeStartElement("physics:ode");
    writer.writeTextElement("stepTime", "0.001");
    writer.writeTextElement("gravity", "0 0 -9.81");
    // see http://opende.sourceforge.net/wiki/index.php/Manual_%28Concepts%29#Constraint_Force_Mixing_.28CFM.29
    // TODO: verify
    writer.writeTextElement("cfm", "10e-10");
    // see http://opende.sourceforge.net/wiki/index.php/Manual_%28Concepts%29#Joint_error_and_the_error_reduction_parameter_.28ERP.29
    // TODO: verify
    writer.writeTextElement("erp", "0.2");
    writer.writeTextElement("stepType", "quick");
    writer.writeTextElement("stepIters", "10");
    writer.writeTextElement("contactMaxCorrectingVel", "100.0");
    writer.writeTextElement("contactSurfaceLayer", "0.001");
    writer.writeTextElement("updateRate", "-0.1");
    // physics:ode
    writer.writeEndElement();

    writer.writeStartElement("rendering:gui");
    // The old gazebo (< 1.0.0) only supports FLTK
    writer.writeTextElement("type", "flkt");
    writer.writeTextElement("size", "800 600");
    writer.writeTextElement("pos", "0 0");

    writer.writeStartElement("frames");

    writer.writeStartElement("row");
    writer.writeAttribute("height", "100%");
    writer.writeStartElement("camera");
    writer.writeAttribute("width", "100%");

    writer.writeTextElement("xyz", "0 0 3");
    writer.writeTextElement("rpy", "0 90 90");

    // camera
    writer.writeEndElement();

    // row
    writer.writeEndElement();

    // frames
    writer.writeEndElement();

    // rendering:gui
    writer.writeEndElement();

    writer.writeStartElement("rendering:ogre");
    writer.writeTextElement("ambient", "0.2 0.2 0.2 1.0");
    writer.writeStartElement("sky");
    writer.writeTextElement("material", "Gazebo/CloudySky");
    // sky
    writer.writeEndElement();
    writer.writeTextElement("grid", "false");
    // rendering:ogre
    writer.writeEndElement();

    writer.writeStartElement("model:renderable");
    writer.writeAttribute("name", "point_white");
    writer.writeTextElement("xyz", "0 0 20");
    writer.writeTextElement("rpy", "0 50 50");
    writer.writeTextElement("enableGravity", "false");
    writer.writeStartElement("light");
    writer.writeTextElement("type", "directional");
    writer.writeTextElement("diffuseColor", "0.7 0.7 0.7");
    writer.writeTextElement("specularColor", "0.1 0.1 0.1");
    writer.writeTextElement("attenuation", "0.2 0.1 0");
    writer.writeTextElement("range", "100.0");

    writer.writeEndElement();
    writer.writeEndElement();

    /*
    writer.writeStartElement("model:physical");
    writer.writeAttribute("name", "plane_model");
    writer.writeTextElement("xyz", "0 0 0");
    writer.writeTextElement("rpy", "0 0 0");
    writer.writeTextElement("static", "true");
    writer.writeStartElement("body:plane");
    writer.writeAttribute("name", "plane_body");
    writer.writeStartElement("geom:plane");
    writer.writeAttribute("name", "plane_geom");
    writer.writeTextElement("normal", "0 0 1");
    writer.writeTextElement("size", "120 120");
    writer.writeTextElement("segments", "10 10");
    writer.writeTextElement("uvTile", "100 100");
    writer.writeTextElement("material", "Gazebo/GrayGrid");
    */

    writer.writeStartElement("model:physical");
    writer.writeAttribute("name", "gplane");

    writer.writeTextElement("xyz", "0 0 0");
    writer.writeTextElement("rpy", "0 0 0");
    writer.writeTextElement("static", "true");

    writer.writeStartElement("body:plane");
    writer.writeAttribute("name", "plane");

    writer.writeStartElement("geom:plane");
    writer.writeAttribute("name", "plane");

    writer.writeTextElement("laserRetro", "2000.0");
    writer.writeTextElement("mu1", "50.0");
    writer.writeTextElement("mu2", "50.0");

    writer.writeTextElement("kp", "1000000000.0");
    writer.writeTextElement("kd", "1.0");

    writer.writeTextElement("normal", "0 0 1");
    writer.writeTextElement("size", "120 120");

    writer.writeTextElement("segments", "10 10");
    writer.writeTextElement("uvTile", "100 100");

    writer.writeTextElement("material", "Gazebo/GrayGrid");


    // geom:plane
    writer.writeEndElement();
    // body:plane
    writer.writeEndElement();
    // model:physical
    writer.writeEndElement();

    foreach (ArenaElement *element, m_elements)
        element->saveWorld(writer);

    writer.writeEndElement();
    writer.writeEndDocument();

    out.close();
}

void Arena::saveWorldSdf(const QString& filename)
{
    QFile out(filename);
    out.open(QFile::WriteOnly);

    QXmlStreamWriter writer(&out);
    // Insert line-breaks for readability
    writer.setAutoFormatting(true);

    writer.writeStartDocument();

    writer.writeStartElement("gazebo");
    writer.writeAttribute("version", "1.0");

    writer.writeStartElement("world");
    writer.writeAttribute("name", "default");

    writer.writeStartElement("scene");
    writer.writeStartElement("ambient");
    writer.writeAttribute("rgba","0.5 0.5 0.5 1");
    writer.writeEndElement();

    writer.writeStartElement("background");
    writer.writeAttribute("rgba","0.5 0.5 0.5 1");
    writer.writeEndElement();

    writer.writeStartElement("shadows");
    writer.writeAttribute("enabled","false");
    writer.writeEndElement();

    writer.writeEndElement();

    writer.writeStartElement("physics");
    writer.writeAttribute("type","ode");

    writer.writeStartElement("gravity");
    writer.writeAttribute("xyz","0 0 -9.81");
    writer.writeEndElement();

    writer.writeStartElement("ode");

    writer.writeStartElement("solver");
    writer.writeAttribute("type","quick");
    writer.writeAttribute("dt","0.001");
    writer.writeAttribute("iters","10");
    writer.writeAttribute("sor","1.3");
    writer.writeEndElement();

    writer.writeStartElement("constraints");
    writer.writeAttribute("cfm","0.0");
    writer.writeAttribute("erp","0.2");
    writer.writeAttribute("contact_max_correcting_vel","10");
    writer.writeAttribute("contact_surface_layer","0.001");
    writer.writeEndElement();

    writer.writeEndElement();

    writer.writeEndElement();

    writer.writeStartElement("light");
    writer.writeAttribute("type","directional");
    writer.writeAttribute("name","directional_light_1");
    writer.writeAttribute("cast_shadows","false");

    writer.writeStartElement("origin");
    writer.writeAttribute("pose","0 0 30 0.1 0.1 0");
    writer.writeEndElement();

    writer.writeStartElement("diffuse");
    writer.writeAttribute("rgba",".5 .5 .5 1");
    writer.writeEndElement();

    writer.writeStartElement("specular");
    writer.writeAttribute("rgba",".1 .1 .1 1");
    writer.writeEndElement();

    writer.writeStartElement("attenuation");
    writer.writeAttribute("range","300");
    writer.writeEndElement();

    writer.writeStartElement("direction");
    writer.writeAttribute("xyz","0.1 0.1 -1");
    writer.writeEndElement();

    writer.writeEndElement();

    writer.writeStartElement("model");

    writer.writeAttribute("name", "plane_model");
    writer.writeAttribute("static", "true");

    writer.writeStartElement("link");
    writer.writeAttribute("name","plane_model_link");

    writer.writeStartElement("collision");
    writer.writeAttribute("name","plane_model_collision");

    writer.writeStartElement("geometry");

    writer.writeStartElement("plane");
    writer.writeAttribute("normal","0 0 1");
    writer.writeEndElement();

    writer.writeEndElement(); //geometry

    writer.writeStartElement("surface");

    writer.writeStartElement("friction");

    writer.writeStartElement("ode");
    writer.writeAttribute("mu","10.0");
    writer.writeAttribute("mu2","10.0");
    writer.writeAttribute("fdir1","0 0 0");
    writer.writeAttribute("slip1","0");
    writer.writeAttribute("slip2","0");
    writer.writeEndElement(); //ode

    writer.writeEndElement(); //friction

    writer.writeStartElement("bounce");
    writer.writeAttribute("restitution_coefficient","0");
    writer.writeAttribute("threshold","1000000.0");
    writer.writeEndElement(); //bounce

    writer.writeStartElement("contact");

    writer.writeStartElement("ode");
    writer.writeAttribute("soft_cfm","0");
    writer.writeAttribute("soft_erp","0.2");
    writer.writeAttribute("kp","1e10");
    writer.writeAttribute("kd","1");
    writer.writeAttribute("max_vel","100.0");
    writer.writeAttribute("min_depth","0.0001");
    writer.writeEndElement(); //ode

    writer.writeEndElement(); //contact

    writer.writeEndElement(); //surface
    writer.writeEndElement(); //collision


    writer.writeStartElement("visual");
    writer.writeAttribute("name","plane_model_visual");
    writer.writeAttribute("cast_shadows","false");

    writer.writeStartElement("geometry");

    writer.writeStartElement("plane");
    writer.writeAttribute("normal","0 0 1");
    writer.writeEndElement();

    writer.writeEndElement(); //geometry

    writer.writeStartElement("material");
    writer.writeAttribute("script","Gazebo/Grey");
    writer.writeEndElement();




    //writer.writeEndElement();

    writer.writeEndElement();

    writer.writeEndElement();

    writer.writeEndElement();










    foreach (ArenaElement *element, m_elements)
        element->saveWorldSdf(writer);


    writer.writeEndElement();

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
