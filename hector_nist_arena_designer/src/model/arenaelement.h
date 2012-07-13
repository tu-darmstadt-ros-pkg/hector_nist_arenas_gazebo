#ifndef ARENAELEMENT_H
#define ARENAELEMENT_H

#include <QObject>
#include <QPoint>
#include <QDomNode>
#include <QXmlStreamWriter>

#include "xmlloadingexception.h"

class Arena;
class ArenaElementType;

class ArenaElement : public QObject
{
    Q_OBJECT

    friend class ArenaElementType;

public:
    /// Returns the position in arena coordinates
    QPoint pos() const { return m_pos; }
    /// Sets the position in arena coordinates
    void setPos(QPoint pos);
    /// Returns the rotation in degrees
    int rotation() const { return m_rotation; }
    /// Sets the rotation in degrees
    void setRotation(int rotation);

    /// Deserializes this element from XML
    void load(const QDomElement& node) throw (XmlLoadingException);
    /// Serializes this element in XML
    void save(QXmlStreamWriter& writer);
    /// Writes .world format-compliant XML
    void saveWorld(QXmlStreamWriter& writer);

    /// Called from Arena::addElement()
    void setArena(Arena *arena) { m_arena = arena; }

    /// Returns a pointer to a global ArenaElementType instance that is the
    /// same for all objects of the same type
    ArenaElementType const * type() const { return m_type; }

    /// Convenience method returning true iff. this is a wall element
    bool isWall() const;
    /// Convenience method returning true iff. this is a floor element
    bool isFloor() const;
    /// Convenience method returning true iff. this is a mountable-item element
    bool isMountableItem() const;
    /// Convenience method returning true iff. this is an item element (freely movable)
    bool isItem() const;

    /// Returns the item's offset from the grid point with 0 <= x,y <= 1 ( (0,0) is bottom left)
    QPointF itemOffset() const { return m_itemOffset; }
    /// See itemOffset()
    void setItemOffset(const QPointF &offset);

    /// For MountableItemType elements, this returns the index of the item mount
    /// point the item is attached to. If no such index was specified and for all
    /// other elements, this method returns -1.
    int itemMountPoint() const { return m_itemMountPoint; }

    /// See itemMountPoint(). Use -1 to set index to "unspecified".
    void setItemMountPoint(int mountPoint);

signals:
    /// Signal emitted when the element is moved
    /// @param pos New position.
    void posChanged(ArenaElement *element, QPoint pos);
    /// Signal emitted when the element is rotated.
    /// @param rotation Rotation in degrees.
    void rotationChanged(ArenaElement *element, int rotation);
    /// Signal emitted when any property of this element changed
    /// @param rotation Rotation in degrees.
    void modified(ArenaElement *element);

private:
    /// Only meant for use by ArenaElementType::createInstance()
    explicit ArenaElement(const ArenaElementType * const type, int id);

    /// Arena this element currently belongs to (exactly one)
    Arena *m_arena;

    const ArenaElementType * const m_type;
    QPoint m_pos;
    /// See itemOffset()
    QPointF m_itemOffset;
    /// rotation in degrees (only multiples of 90 degrees)
    int m_rotation;
    /// Item mount point this element is attached to. Only applies to MountableItemType
    /// elements. -1 indicates that no index has been specified.
    int m_itemMountPoint;
    /// Unique ID set by ArenaElementType
    const int m_instanceId;
};

#endif // ARENAELEMENT_H
