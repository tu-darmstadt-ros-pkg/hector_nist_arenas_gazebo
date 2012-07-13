#include "arenascene.h"
#include "arenasceneelement.h"

#include "../model/arena.h"
#include "../model/arenaelement.h"
#include "../model/arenaelementtype.h"

#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QPen>
#include <QBrush>
#include <QMimeData>

#include <cmath>

ArenaScene::ArenaScene(Arena *arena)
    : m_arena(arena)
{
    connect(arena, SIGNAL(elementAdded(ArenaElement*)),
                   SLOT(slotElementAdded(ArenaElement*)));
    connect(arena, SIGNAL(elementRemoved(ArenaElement*)),
                   SLOT(slotElementRemoved(ArenaElement*)));
    connect(arena, SIGNAL(modified()),
                   SLOT(updateViewMargin()));

    foreach (ArenaElement *element, arena->elements())
        slotElementAdded(element);

    updateViewMargin();
}

QPoint ArenaScene::sceneToGrid(QPointF scenePos)
{
    QPointF gridPos = sceneToGridF(scenePos);
    return QPoint(qRound(gridPos.x()), qRound(gridPos.y()));
}

QPointF ArenaScene::sceneToGridF(QPointF scenePos)
{
#if 0
    qreal x = scenePos.x() + SPACING;
    // Screen y coordinates are reversed ((0,0) is bottom left, not top left)
    qreal y = -(scenePos.y() + SPACING);
    return QPointF(x / (qreal)CELL_SIZE - 0.5, y / (qreal)CELL_SIZE + 0.5);
#endif
    return QPointF(scenePos.x() / (qreal)CELL_SIZE, -scenePos.y() / (qreal)CELL_SIZE);
}

QPointF ArenaScene::gridToScene(QPointF gridPos)
{
    // Screen y coordinates are reversed
    return QPointF(gridPos.x() * CELL_SIZE, -gridPos.y() * CELL_SIZE);
}

QPointF ArenaScene::nearestGridPoint(QPointF scenePos)
{
    return gridToScene(sceneToGrid(scenePos));
}

QList<ArenaSceneElement*> ArenaScene::selectedElements()
{
    QList<ArenaSceneElement*> list;
    foreach(QGraphicsItem* item, selectedItems())
    {
        ArenaSceneElement* element = dynamic_cast<ArenaSceneElement*>(item);
        if (element)
            list.append(element);
    }

    return list;
}

ArenaSceneElement *ArenaScene::sceneElementFor(ArenaElement *element)
{
    return m_elements[element];
}

void ArenaScene::updateViewMargin()
{
    // The minimum arena is 3x3 (-1 <= x,y <= 1)
    int left = -1;
    int right = 1;
    int top = 1;
    int bottom = -1;
    foreach(ArenaSceneElement* element, m_elements)
    {
        QPoint pos = element->element()->pos();
        left   = qMin(left, pos.x());
        right  = qMax(right, pos.x());
        top    = qMax(top, pos.y());
        bottom = qMin(bottom, pos.y());
    }
    QPointF topLeft = gridToScene(QPoint(left, top));
    QPointF bottomRight = gridToScene(QPoint(right, bottom));
    setSceneRect(QRectF(topLeft, bottomRight));
}

void ArenaScene::slotElementAdded(ArenaElement *element)
{
    ArenaSceneElement *sceneElement = new ArenaSceneElement(element);

    m_elements[element] = sceneElement;
    addItem(sceneElement);

    updateViewMargin();
}

void ArenaScene::slotElementRemoved(ArenaElement *element)
{
    ArenaSceneElement *sceneElement = m_elements[element];
    Q_ASSERT(sceneElement);

    removeItem(sceneElement);
    m_elements.remove(element);
    delete sceneElement;

    updateViewMargin();
}
