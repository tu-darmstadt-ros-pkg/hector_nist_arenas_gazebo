#include "arenacontroller.h"

#include "arenascene.h"
#include "arenasceneelement.h"
#include "../model/arena.h"
#include "../model/arenaelement.h"
#include "../model/arenaelementtype.h"

#include <QPainter>
#include <QDebug>

#include <math.h>

ArenaController::ArenaController(Arena *arena, ArenaScene *scene)
    : m_arena(arena)
    , m_scene(scene)
    , m_lastDragFinished(true)
    , m_autoRotateWalls(false)
    , m_snapToGrid(true)
{
}

qreal squaredDistance(const QPointF &a, const QPointF &b)
{
    qreal diffX = b.x() - a.x();
    qreal diffY = b.y() - a.y();
    return diffX*diffX + diffY*diffY;
}

int getSnapRotation(const QPointF &scenePos)
{
    qreal halfItemSize = ITEM_SIZE / 2.0;
    QPointF gridPos = ArenaScene::nearestGridPoint(scenePos);
    QPointF top    = gridPos - QPointF(0, halfItemSize);
    QPointF bottom = gridPos + QPointF(0, halfItemSize);
    QPointF left  = gridPos - QPointF(halfItemSize, 0);
    QPointF right = gridPos + QPointF(halfItemSize, 0);

    qreal distTop = squaredDistance(scenePos, top);
    qreal distBottom = squaredDistance(scenePos, bottom);
    qreal distLeft = squaredDistance(scenePos, left);
    qreal distRight = squaredDistance(scenePos, right);

    qreal minDist = qMin(distTop, qMin(distBottom, qMin(distLeft, distRight)));

    if (minDist == distTop)
        return 0;
    if (minDist == distRight)
        return 90;
    if (minDist == distBottom)
        return 180;
    //if (minDist == distLeft)
        return 270;
}

QPointF getRotationSnapPoint(const QPointF &scenePos)
{
    QPointF gridPos = ArenaScene::nearestGridPoint(scenePos);
    QPointF top = gridPos + QPointF(ITEM_SIZE / 2.0, 0.0);
    QPointF bottom = gridPos + QPointF(ITEM_SIZE / 2.0, ITEM_SIZE);
    QPointF left = gridPos + QPointF(0.0, ITEM_SIZE / 2.0);
    QPointF right = gridPos + QPointF(ITEM_SIZE, ITEM_SIZE / 2.0);

    qreal distTop = squaredDistance(scenePos, top);
    qreal distBottom = squaredDistance(scenePos, bottom);
    qreal distLeft = squaredDistance(scenePos, left);
    qreal distRight = squaredDistance(scenePos, right);

    qreal minDist = qMin(distTop, qMin(distBottom, qMin(distLeft, distRight)));

    if (minDist == distTop)
        return top;
    if (minDist == distRight)
        return right;
    if (minDist == distBottom)
        return bottom;
    //if (minDist == distLeft)
        return left;
}

void ArenaController::startDrag(ArenaSceneElement *sceneElement)
{
    startDrag(QList<ArenaSceneElement*>() << sceneElement);
}

void ArenaController::startDrag(QList<ArenaSceneElement*> sceneElements)
{
    m_activeElements = sceneElements;
    m_lastDragFinished = false;
    m_totalMouseOffset = QPointF(0, 0);

    m_origPositions.clear();
    foreach (ArenaSceneElement *sceneElement, sceneElements)
        m_origPositions[sceneElement] = sceneElement->pos();

    // Only freely movable items require snap to grid to be turned off
    setSnapToGrid(!(sceneElements.count() == 1 &&
                    sceneElements.first()->element()->isItem()));
    // Only auto-rotate walls if we are moving only one wall
    setAutoRotateWalls(sceneElements.count() == 1);
}

void ArenaController::startInsertion(const QString &elementType, const QPointF &pos)
{
    foreach (QGraphicsItem *item, m_scene->selectedItems())
        item->setSelected(false);

    ArenaElement *element = m_arena->addElement(elementType);
    Q_ASSERT(element);

    // ArenaSceneElement is automatically created when ArenaElement is added to arena
    ArenaSceneElement *sceneElement = m_scene->sceneElementFor(element);
    Q_ASSERT(sceneElement);

    sceneElement->setSelected(true);

    startDrag(sceneElement);

    dragTo(pos);
}

void ArenaController::cancelInsertion()
{
    foreach (ArenaSceneElement *activeElement, m_activeElements)
    {
        ArenaElement *element = activeElement->element();
        m_arena->removeElement(element);
        delete element;
        // activeElement is deleted automatically by the scene
    }

    endOperation();
}

void ArenaController::endOperation()
{
    m_activeElements.clear();
    m_lastDragFinished = true;
}

bool ArenaController::operationInProgress() const
{
    return !m_lastDragFinished;
}

void ArenaController::dragBy(const QPointF &by)
{
    m_totalMouseOffset += by;
    QPointF gridMouseOffset = m_snapToGrid ?
        ArenaScene::nearestGridPoint(m_totalMouseOffset) : m_totalMouseOffset;
    foreach (ArenaSceneElement *activeElement, m_activeElements)
    {
        QPointF origPos = m_origPositions[activeElement];
        QPointF newPos = origPos + gridMouseOffset;
        qDebug() << "origPos:" << origPos;
        dragTo(activeElement, newPos);
    }
}

void ArenaController::dragTo(const QPointF &to)
{
    foreach (ArenaSceneElement *activeElement, m_activeElements)
    {
        dragTo(activeElement, to);
    }
}

void ArenaController::dragTo(ArenaSceneElement *sceneElement, const QPointF &to)
{
    ArenaElement *element = sceneElement->element();

    element->setPos(ArenaScene::sceneToGrid(to));

    QPointF gridPointScene = ArenaScene::nearestGridPoint(to);
    if (element->isItem())
    {
        QPointF gridOffset = ArenaScene::sceneToGridF(to) -
                             ArenaScene::sceneToGridF(gridPointScene);
        element->setItemOffset(gridOffset/* + QPointF(0, 1)*/);
    }

    if (m_autoRotateWalls && element->isWall())
    {
        element->setRotation(getSnapRotation(to));
    }
}

void ArenaController::draw(QPainter *painter, const QRectF &rect)
{
    if (m_activeElements.count() != 1)
        return;

    ArenaSceneElement *activeElement = m_activeElements.first();
    if (activeElement->element()->isWall() || activeElement->element()->isItem())
    {
        QPointF gridPos = ArenaScene::nearestGridPoint(activeElement->pos());
        int r = 10;
        painter->setBrush(QBrush(0xE0E0E0));
        painter->setPen(0xE0E0E0);
        painter->drawEllipse(gridPos, r, r);

        /*
        if (m_activeElement->element()->isWall())
        {
            QPointF snapPos = getRotationSnapPoint(m_mousePos);
            QPointF lineDir = (snapPos - center);
            lineDir /= sqrt(lineDir.x()*lineDir.x() + lineDir.y()*lineDir.y());

            painter->drawLine(center + lineDir * r, snapPos);
        }
        */
    }
}
