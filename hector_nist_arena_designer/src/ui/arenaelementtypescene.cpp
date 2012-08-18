#include "arenaelementtypescene.h"

#include "../model/arenaelementtype.h"
#include "../model/arenaelement.h"
#include "../model/arenaelementtyperegistry.h"
#include "arenasceneelement.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QMoveEvent>
#include <QDebug>

ArenaElementTypeScene::ArenaElementTypeScene(ArenaElementTypeRegistry *registry)
    : m_registry(registry)
{
}

void ArenaElementTypeScene::loadConfig(const QDomElement &arenaElements)
{
    QDomNodeList childNodes = arenaElements.childNodes();
    int y = 0;
    for (int i = 0; i < childNodes.count(); i++)
    {
        QDomElement element = childNodes.at(i).toElement();
        if (element.tagName() != "element")
        {
            if (element.tagName() != "")
                qWarning() << "Warning: Ignored element \"" << element.tagName() << "\"";
            continue;
        }

        QString elementType = element.attribute("type");
        if (!m_registry->hasElement(elementType))
        {
            qDebug() << "[Rescue Arena Designer] Error in config.xml: Element of type" << elementType << "doesn't seem to exist in your hector_arena_elements package";
            continue;
        }
        ArenaElement *elementInstance = m_registry->instantiateElement(elementType);
        if (element.hasAttribute("rotation"))
        {
            QString elementRotation = element.attribute("rotation");
            elementInstance->setRotation(elementRotation.toInt());
        }
        ArenaSceneElement *sceneElement = new ArenaSceneElement(elementInstance);
        sceneElement->setPos(0, y);
        sceneElement->setIsEditorSample(true);
        addItem(sceneElement);

        int spacing = 10;
        y += sceneElement->boundingRect().height() + spacing;
    }
}

bool ArenaElementTypeScene::event(QEvent *event)
{
    QGraphicsSceneMouseEvent *_event = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
    if (event->type() == QEvent::GraphicsSceneMouseMove)
    {
        QGraphicsItem *item = itemAt(_event->scenePos());
        if (item)
        {
            ArenaSceneElement *sceneElement = dynamic_cast<ArenaSceneElement*>(item);
            Q_ASSERT(sceneElement);
            emit elementHovered(sceneElement->element());
        }
    }

    return QGraphicsScene::event(event);
}

//ArenaElementTypeScene::ArenaElementTypeScene()
//{
//    int posY = 0;
//    int spacing = 10;
//    foreach (ArenaElementType* type, registry->elementTypes())
//    {
//        ArenaElement *element = type->createInstance();
//        ArenaSceneElement *sceneElement = new ArenaSceneElement(element);
//        sceneElement->setIsEditorSample(true);
//        sceneElement->setPos(0, posY);

//        addItem(sceneElement);

//        posY += sceneElement->pixmap().height() + spacing;
//    }
//}
