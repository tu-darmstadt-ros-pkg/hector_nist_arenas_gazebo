#include "arenaelementtype.h"
#include "arenaelement.h"

#include <QDebug>

ArenaElementType::ArenaElementType()
    : m_instanceCount(0)
{
}

bool ArenaElementType::load(const QDir &folderDir)
{
    m_name = folderDir.dirName();

    QFile in(folderDir.filePath("properties.xml"));
    Q_ASSERT(in.exists());
    in.open(QFile::ReadOnly);
    QDomDocument doc;
    doc.setContent(&in);
    in.close();

    QDomElement node = doc.firstChild().toElement();
    Q_ASSERT(!node.isNull());
    if (node.tagName() != "properties")
    {
        qDebug() << "Error loading properties.xml in " << QFileInfo(in).filePath() << ": Root element must be <property>!";
        return false;
    }

    QDomNodeList childNodes = node.childNodes();
    for (int i = 0; i < childNodes.size(); i++)
    {
        QDomElement child = childNodes.at(i).toElement();
        Q_ASSERT(!child.isNull());
        /*
        if (child.tagName() == "name")
        {
            m_name = child.text();
        }
        else */if (child.tagName() == "type")
        {
            if (child.text() == "floor")
                m_type = FloorType;
            else if (child.text() == "wall")
                m_type = WallType;
            else if (child.text() == "mountable-item")
                m_type = MountableItemType;
            else if (child.text() == "item")
                m_type = ItemType;
            else
            {
                qDebug() << "Error loading properties.xml in " << QFileInfo(in).filePath() << ": Unknown element type!";
                return false;
            }
        }
        else if (child.tagName() == "pixmap")
        {
            m_pixmap = QPixmap(folderDir.filePath(child.text()));
        }
        else if (child.tagName() == "mesh")
        {
            m_mesh = child.text();
        }
        else if (child.tagName() == "meta-info")
        {
            QDomNodeList metaInfos = child.childNodes();
            for (int i = 0; i < metaInfos.size(); i++)
            {
                QDomElement metaInfo = metaInfos.at(i).toElement();
                Q_ASSERT(!metaInfo.isNull());
                // Only these elements are valid meta info nodes
                if (metaInfo.tagName() == "mi")
                {
                    if (!metaInfo.hasAttribute("desc"))
                    {
                        qDebug() << "Warning: meta-info element is missing \"desc\" attribute in "
                                 << QFileInfo(in).filePath();
                        continue;
                    }
                    QString desc = metaInfo.attribute("desc");

                    if (metaInfo.text().isEmpty())
                    {
                        qDebug() << "Warning: Empty meta-info element in "
                                 << QFileInfo(in).filePath();
                        continue;
                    }
                    QString value = metaInfo.text();
                    m_metaInfos.append(QStringPair(desc, value));
                }
            }
        }
        else if (child.tagName() == "item-mount-points")
        {
            QDomNodeList itemMountPoints = child.childNodes();
            for (int i = 0; i < itemMountPoints.size(); i++)
            {
                QDomElement itemMountPoint = itemMountPoints.at(i).toElement();
                Q_ASSERT(!itemMountPoint.isNull());
                // Only these elements are valid
                if (itemMountPoint.tagName() == "item-mount-point")
                {
                    QPointF pos;
                    QString desc;
                    if (itemMountPoint.hasAttribute("x"))
                        pos.rx() = itemMountPoint.attribute("x").toFloat();
                    if (itemMountPoint.hasAttribute("y"))
                        pos.ry() = itemMountPoint.attribute("y").toFloat();
                    if (itemMountPoint.hasAttribute("desc"))
                        desc = itemMountPoint.attribute("desc");
                    m_itemMountPoints.push_back(ItemMountPoint(desc, pos));
                }
            }
        }
    }
}

QString ArenaElementType::humanReadableName() const
{
    foreach (QStringPair metaInfo, m_metaInfos)
        if (metaInfo.first == "Name")
            return metaInfo.second;
    return name();
}

ArenaElement* ArenaElementType::createInstance() const
{
    ArenaElement *element = new ArenaElement(this, m_instanceCount++);
    return element;
}
