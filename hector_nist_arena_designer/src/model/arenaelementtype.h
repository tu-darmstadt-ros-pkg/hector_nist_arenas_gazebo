#ifndef ARENAELEMENTTYPE_H
#define ARENAELEMENTTYPE_H

#include <QString>
#include <QPixmap>
#include <QDomElement>
#include <QDir>
#include <QFileInfo>
#include <QPair>

#include "xmlloadingexception.h"

typedef QPair<QString, QString> QStringPair;
typedef QPair<QString, QPointF> ItemMountPoint;

class ArenaElement;

class ArenaElementType
{
    friend class ArenaElementTypeRegistry;

public:
    enum Type
    {
        FloorType,
        WallType,
        /// Item freely movable within a grid point
        ItemType,
        /// Item that can be mounted to a wall element
        MountableItemType
    };

    ArenaElementType();

    /// Loads this element type from the specified folder containing
    /// the properties.xml file specific to the type.
    /// @param folder containing the properties.xml
    /// @return true on success, false if element type could not be loaded from
    ///         the given folder.
    bool load(const QDir &folderDir);

    ArenaElement* createInstance() const;

    QString name() const { return m_name; }
    QPixmap pixmap() const { return m_pixmap; }
    QString mesh() const { return m_mesh; }
    Type type() const { return m_type; }
    QVector<QStringPair> metaInfos() const { return m_metaInfos; }

    /// Returns the "Name" meta info field if it exists, otherwise name()
    QString humanReadableName() const;

    /// Returns a list of item mount points. Currently, this method is only
    /// applicable to wall elements
    QList<ItemMountPoint> itemMountPoints() const { return m_itemMountPoints; }

private:
    QList<ItemMountPoint> m_itemMountPoints;
    QVector<QStringPair> m_metaInfos;
    QString m_name;
    QString m_mesh;
    QPixmap m_pixmap;
    Type m_type;
    mutable int m_instanceCount;
};

#endif // ARENAELEMENTTYPE_H
