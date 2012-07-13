#include "arenasceneelement.h"

#include "../model/arenaelement.h"
#include "../model/arenaelementtype.h"

#include "arenascene.h"

#include <QDebug>
#include <QMimeData>
#include <QDrag>
#include <QPixmap>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

ArenaSceneElement::ArenaSceneElement(ArenaElement *element)
    : m_element(element)
{
    setPixmap(element->type()->pixmap());
    m_isEditorSample = false;
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    connect(element, SIGNAL(posChanged(ArenaElement*, QPoint)),
                     SLOT(slotPosChanged(ArenaElement*, QPoint)));
    connect(element, SIGNAL(rotationChanged(ArenaElement*, int)),
                     SLOT(slotRotationChanged(ArenaElement*, int)));

    //setTransformOriginPoint(pixmap().width() / 2.0, pixmap().width() / 2.0);

#if 0
    switch (element->type()->type())
    {
    case ArenaElementType::WallType:
        //setTransformOriginPoint(-42.5, 12.5);
        break;
    case ArenaElementType::FloorType:
    case ArenaElementType::MountableItemType:
    case ArenaElementType::ItemType:
        setTransformOriginPoint(pixmap().width() / 2.0, pixmap().width() / 2.0);
        break;
    }
#endif

    slotPosChanged(element, element->pos());
    slotRotationChanged(element, element->rotation());
}

void ArenaSceneElement::setIsEditorSample(bool isEditorSample)
{
    m_isEditorSample = isEditorSample;
    setOffset(gridOffset());
}

QPointF ArenaSceneElement::gridOffset()
{
    if (m_isEditorSample)
        return QPointF(0, 0);

    // Offset of the pixmap before the rotation is performed!
    QPointF offset(-pixmap().width() / 2.0, -pixmap().height() / 2.0);

    switch(m_element->type()->type())
    {
    case ArenaElementType::WallType:
        offset += QPointF(0, -CELL_SIZE / 2);
        break;
    case ArenaElementType::FloorType:
    case ArenaElementType::MountableItemType:
    case ArenaElementType::ItemType:
        break;
    }

    return offset;
}

void ArenaSceneElement::slotRotationChanged(ArenaElement *element, int rotation)
{
    setOffset(gridOffset());
    setRotation(rotation);
}

void ArenaSceneElement::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isEditorSample)
    {
        QMimeData *data = new QMimeData;
        data->setData("type", m_element->type()->name().toAscii());
        data->setData("rotation", QString::number(m_element->rotation()).toAscii());
        QDrag *drag = new QDrag(event->widget());
        drag->setMimeData(data);
        drag->start();

        setCursor(Qt::OpenHandCursor);
    }

    // Z value 1 is used to put the item being moved on top
    setZValue(1);
    event->accept();
    QGraphicsPixmapItem::mousePressEvent(event);
}

void ArenaSceneElement::slotPosChanged(ArenaElement *element, QPoint pos)
{
    Q_ASSERT(element == m_element);

    QPointF newPos = ArenaScene::gridToScene(pos + element->itemOffset());
    setPos(newPos);
}
