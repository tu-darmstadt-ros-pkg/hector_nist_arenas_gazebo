#ifndef ARENA_H
#define ARENA_H

#include <QObject>
#include <QPoint>

class ArenaElement;
class ArenaElementTypeRegistry;

class Arena : public QObject
{
    Q_OBJECT

public:
    Arena(ArenaElementTypeRegistry* typeRegistry);
    ~Arena();

    /// Deserializes this arena from XML
    void load(const QString& filename);
    /// Serializes this arena in XML
    void save(const QString& filename);
    /// Writes .world format-compliant XML
    void saveWorld(const QString& filename);
    void saveWorldSdf(const QString& filename);

    /// Creates and adds an element of type elementType
    ArenaElement* addElement(const QString& elementType);
    /// Adds an existing element to this arena
    void addElement(ArenaElement *element);
    /// Removes an existing element from this arena. This does not delete it.
    /// The element can be re-added later.
    void removeElement(ArenaElement* element);

    /// Removes all elements from this arena.
    void clear();

    QList<ArenaElement*> elements() { return m_elements; }
    QList<ArenaElement*> elementsAt(const QPoint &pos) const;

    /// Returns either a wall at the same position with the same rotation as
    /// the given element or 0 if no such wall exists
    ArenaElement *contextElement(ArenaElement *element) const;

signals:
    void elementAdded(ArenaElement *element);
    void elementRemoved(ArenaElement *element);

    void modified();

private slots:
    void slotModified();

private:
    ArenaElementTypeRegistry * const m_typeRegistry;
    QList<ArenaElement*> m_elements;
};

#endif // ARENA_H
