#include "arenaelementtyperegistry.h"

#include "arenaelementtype.h"
#include "arenaelement.h"
#include "xmlloadingexception.h"

#include "../global.h"

#include <QDebug>
#include <QDir>

ArenaElementTypeRegistry::ArenaElementTypeRegistry(const QString& dir)
{
    load(dir);
}

QList<ArenaElementType*> ArenaElementTypeRegistry::elementTypes()
{
    return m_elementTypes.values();
}

ArenaElementType* ArenaElementTypeRegistry::elementType(const QString &typeName)
{
    return m_elementTypes[typeName];
}

bool ArenaElementTypeRegistry::hasElement(const QString &typeName)
{
    return m_elementTypes.contains(typeName);
}

ArenaElement* ArenaElementTypeRegistry::instantiateElement(const QString &typeName)
{
    ArenaElementType *type = elementType(typeName);
    if (!type)
        return 0;
    ArenaElement *element = type->createInstance();
    return element;
}

void ArenaElementTypeRegistry::load(const QString &dir)
{
    QDir topDir(dir);
    QStringList entries = topDir.entryList(QDir::Dirs);
    //qDebug() << "[Rescue Arena Designer] ArenaElementTypeRegistry: Loading elements.";
    foreach (QString entry, entries)
    {
        //qDebug() << "[Rescue Arena Designer] ArenaElementTypeRegistry: Checking elements/" + entry;
        // These aren't really folders...
        if (entry == "." || entry == "..")
            continue;

        QDir entryDir(topDir.path() + "/" + entry);
        if (entryDir.exists("properties.xml"))
        {
            //qDebug() << "[Rescue Arena Designer] ArenaElementTypeRegistry: properties.xml exists. Loading element.";
            ArenaElementType *t = new ArenaElementType();
            /// Check if loading from properties.xml was successful
            if (t->load(entryDir))
                m_elementTypes[t->name()] = t;
            else
                delete t;
        }
    }
}
