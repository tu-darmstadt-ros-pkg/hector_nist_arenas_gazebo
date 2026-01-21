#ifndef ARENAELEMENTLOADER_H
#define ARENAELEMENTLOADER_H

#include <QString>
#include <QMap>

class ArenaElement;
class ArenaElementType;

class ArenaElementTypeRegistry
{
public:
    /// Constructs a type registry from the directory specified containing
    /// arena element types
    ArenaElementTypeRegistry(const QString& dir);

    void load(const QString& dir);
    QList<ArenaElementType*> elementTypes();
    bool hasElement(const QString& typeName);
    ArenaElementType* elementType(const QString& typeName);
    ArenaElement* instantiateElement(const QString &typeName);

private:
    QMap<QString, ArenaElementType*> m_elementTypes;
};

#endif // ARENAELEMENTLOADER_H
