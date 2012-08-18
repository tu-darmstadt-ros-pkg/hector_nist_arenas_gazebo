#include "arena.h"
#include "arenaelement.h"
#include "arenaelementtype.h"
#include "xmlloadingexception.h"

#include <QList>
#include <QDebug>

#include <math.h>

ArenaElement::ArenaElement(const ArenaElementType * const type, int id)
    : m_type(type)
    , m_rotation(0)
    , m_instanceId(id)
    , m_itemMountPoint(-1)
{
}

bool ArenaElement::isWall() const
{
    return m_type->type() == ArenaElementType::WallType;
}

bool ArenaElement::isFloor() const
{
    return m_type->type() == ArenaElementType::FloorType;
}

bool ArenaElement::isMountableItem() const
{
    return m_type->type() == ArenaElementType::MountableItemType;
}

bool ArenaElement::isItem() const
{
    return m_type->type() == ArenaElementType::ItemType;
}

void ArenaElement::setPos(QPoint pos)
{
    // Do not ignore if pos is same as before! We always have to emit posChanged().
    //if (pos == m_pos)
    //    return;
    m_pos = pos;
    emit posChanged(this, pos);
    emit modified(this);
}

void ArenaElement::setItemOffset(const QPointF &offset)
{
    m_itemOffset = offset;
    emit posChanged(this, m_pos);
}

void ArenaElement::setRotation(int rotation)
{
    // Do not ignore if rotation is same as before! We always have to emit rotationChanged().
    //if (rotation == m_rotation)
    //    return;
    // normalize also negative number (-2 % 3 == -2 but should be 1)
    // (-2 % 3 + 3) % 3 == (-2 + 3) % 3 == 1
    int normalizedRotation = (rotation % 360 + 360) % 360;
    m_rotation = normalizedRotation;
    emit rotationChanged(this, normalizedRotation);
    emit modified(this);
}

void ArenaElement::setItemMountPoint(int mountPoint)
{
    m_itemMountPoint = mountPoint;
    emit modified(this);
}

void ArenaElement::load(const QDomElement& node) throw (XmlLoadingException)
{
    if (!node.hasAttribute("x"))
        throw XmlLoadingException("An arena element is missing an x coordinate.", node);
    if (!node.hasAttribute("y"))
        throw XmlLoadingException("An arena element is missing a y coordinate.", node);

    QPoint pos;
    pos.setX(node.attribute("x").toInt());
    pos.setY(node.attribute("y").toInt());
    setPos(pos);

    if (node.hasAttribute("offset-x") && node.hasAttribute("offset-y"))
    {
        QString offsetX = node.attribute("offset-x");
        QString offsetY = node.attribute("offset-y");

        QPointF offset = QPointF(offsetX.toFloat(), offsetY.toFloat());
        setItemOffset(offset);
    }

    if (node.hasAttribute("rotation"))
    {
        QString rotation = node.attribute("rotation");
        setRotation(rotation.toInt());
    }

    if (node.hasAttribute("mount-point"))
    {
        QString mountPointIndex = node.attribute("mount-point");
        setItemMountPoint(mountPointIndex.toInt());
    }
}

void ArenaElement::save(QXmlStreamWriter& writer)
{
    writer.writeStartElement("element");
    writer.writeAttribute("type", m_type->name());
    writer.writeAttribute("rotation", QString::number(rotation()));
    writer.writeAttribute("x", QString::number(m_pos.x()));
    writer.writeAttribute("y", QString::number(m_pos.y()));

    if (!m_itemOffset.isNull())
    {
        writer.writeAttribute("offset-x", QString::number(m_itemOffset.x()));
        writer.writeAttribute("offset-y", QString::number(m_itemOffset.y()));
    }
    if (m_itemMountPoint != -1)
        writer.writeAttribute("mount-point", QString::number(m_itemMountPoint));
    writer.writeEndElement();
}

void ArenaElement::saveWorld(QXmlStreamWriter& writer)
{
    writer.writeStartElement("model:physical");
    QString name;
    QTextStream(&name) << m_type->name() << "_" << m_instanceId;
    writer.writeAttribute("name", name);

    ArenaElement *context = m_arena->contextElement(this);
    QList<ItemMountPoint> contextItemMountPoints;
    if (context)
        contextItemMountPoints = context->type()->itemMountPoints();

    // Use "corrected" mount point index
    int itemMountPoint = m_itemMountPoint;
    if (!isMountableItem() || !context)
    {
        itemMountPoint = -1;
    }
    else
    {
        // When there are possible mount points, use the first if none is assigned
        itemMountPoint = qMax(itemMountPoint, 0);
        // But make sure the index doesn't exceed the last mount point
        if (contextItemMountPoints.count() <= itemMountPoint)
            itemMountPoint = -1;
    }

    qreal offsetX = 0;
    qreal offsetZ = 0;
    if (itemMountPoint >= 0)
    {
        Q_ASSERT(context);
        Q_ASSERT(contextItemMountPoints.count() > itemMountPoint);
        ItemMountPoint mountPoint = contextItemMountPoints[itemMountPoint];
        offsetX = mountPoint.second.x() - 0.6;
        offsetZ = mountPoint.second.y();
    }

    QTransform trans;
    trans.rotate(-m_rotation);
    QPointF mappedOffset = trans.map(QPointF(offsetX, 0));
    mappedOffset += m_itemOffset * 1.2;

    QString xyz;
    // One unit is 1.2 meters, gazebo is set up to think in meters
    QTextStream(&xyz) << m_pos.x() * 1.2 + mappedOffset.x() << " "
                      << m_pos.y() * 1.2 + mappedOffset.y() << " " << offsetZ;
    writer.writeTextElement("xyz", xyz);
    QString rpy;
    QTextStream(&rpy) << "0 0 " << -m_rotation;
    writer.writeTextElement("rpy", rpy);
    writer.writeTextElement("static", "true");

    writer.writeStartElement("body:trimesh");
    writer.writeAttribute("name", name + "_body");

    writer.writeStartElement("geom:trimesh");
    writer.writeAttribute("name", name + "_geom");

    writer.writeTextElement("mesh", m_type->mesh());
    writer.writeTextElement("scale", "1.0 1.0 1.0");
    /// TODO: mass?
    writer.writeTextElement("genTexCoord", "true");

    writer.writeStartElement("visual");
    writer.writeTextElement("scale", "1.0 1.0 1.0");
    writer.writeTextElement("rpy", "0 0 0");
    writer.writeTextElement("mesh", m_type->mesh());
    //writer.writeTextElement("material", "Gazebo/WoodPallet");
    // visual
    writer.writeEndElement();

    // geom:trimesh
    writer.writeEndElement();

    // body:trimesh
    writer.writeEndElement();

    // model:physical
    writer.writeEndElement();
}

void ArenaElement::saveWorldSdf(QXmlStreamWriter& writer)
{
    writer.writeStartElement("model");
    QString name;
    QTextStream(&name) << m_type->name() << "_" << m_instanceId;
    writer.writeAttribute("name", name);
    writer.writeAttribute("static", "true");

    ArenaElement *context = m_arena->contextElement(this);
    QList<ItemMountPoint> contextItemMountPoints;
    if (context)
        contextItemMountPoints = context->type()->itemMountPoints();

    // Use "corrected" mount point index
    int itemMountPoint = m_itemMountPoint;
    if (!isMountableItem() || !context)
    {
        itemMountPoint = -1;
    }
    else
    {
        // When there are possible mount points, use the first if none is assigned
        itemMountPoint = qMax(itemMountPoint, 0);
        // But make sure the index doesn't exceed the last mount point
        if (contextItemMountPoints.count() <= itemMountPoint)
            itemMountPoint = -1;
    }

    qreal offsetX = 0;
    qreal offsetZ = 0;
    if (itemMountPoint >= 0)
    {
        Q_ASSERT(context);
        Q_ASSERT(contextItemMountPoints.count() > itemMountPoint);
        ItemMountPoint mountPoint = contextItemMountPoints[itemMountPoint];
        offsetX = mountPoint.second.x() - 0.6;
        offsetZ = mountPoint.second.y();
    }

    QTransform trans;
    trans.rotate(-m_rotation);
    QPointF mappedOffset = trans.map(QPointF(offsetX, 0));
    mappedOffset += m_itemOffset * 1.2;

    QString xyz;
    // One unit is 1.2 meters, gazebo is set up to think in meters
    QTextStream(&xyz) << m_pos.x() * 1.2 + mappedOffset.x() << " "
                      << m_pos.y() * 1.2 + mappedOffset.y() << " " << offsetZ;

    QString rpy;
    QTextStream(&rpy) << "0 0 " << -(m_rotation/180.0)*M_PI;
    //writer.writeTextElement("rpy", rpy);

    writer.writeStartElement("link");
    writer.writeAttribute("name",name+"_link");

    writer.writeStartElement("origin");
    writer.writeAttribute("pose",xyz + " " + rpy);
    writer.writeEndElement();

    writer.writeStartElement("collision");
    writer.writeAttribute("name",name+"_collision");

    writer.writeStartElement("geometry");

    writer.writeStartElement("mesh");
    writer.writeAttribute("filename",m_type->mesh());
    writer.writeAttribute("scale","1 1 1");
    writer.writeEndElement();

    writer.writeEndElement();

    writer.writeEndElement();

    writer.writeStartElement("visual");
    writer.writeAttribute("name",name+"_visual");
    writer.writeAttribute("cast_shadows","false");

    writer.writeStartElement("geometry");

    writer.writeStartElement("mesh");
    writer.writeAttribute("filename",m_type->mesh());
    writer.writeAttribute("scale","1 1 1");
    writer.writeEndElement();

    writer.writeEndElement();

    writer.writeEndElement();

    writer.writeEndElement();

    writer.writeEndElement();

}









