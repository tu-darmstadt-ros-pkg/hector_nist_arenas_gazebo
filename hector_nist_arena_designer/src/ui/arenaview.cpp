#include "arenaview.h"
#include "arenascene.h"
#include "arenasceneelement.h"
#include "../model/arena.h"
#include "../model/arenaelement.h"
#include "../model/arenaelementtype.h"

#include <QDebug>
#include <QScrollBar>
#include <QMouseEvent>
#include <QMessageBox>
#include <QTimer>

ArenaView::ArenaView(ArenaController *controller, QWidget *parent)
    : QGraphicsView(parent)
    , m_controller(controller)
{
    setDragMode(QGraphicsView::RubberBandDrag);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    m_rubberBandActive = false;
}

void drawCross(QPainter *painter, QPoint p, int lineLength)
{
    painter->drawLine(p.x() - lineLength, p.y(), p.x() + lineLength, p.y());
    painter->drawLine(p.x(), p.y() - lineLength, p.x(), p.y() + lineLength);
}

void ArenaView::slotGridPaintingDisabled()
{
    QMessageBox::warning(parentWidget(), "Grid Painting Monkey",
        "Aaargh. This is just too many grid points. Sorry. Falling back to white background.");
}

void ArenaView::drawBackground(QPainter *painter, const QRectF& rect)
{
    // When the grid becomes too small, don't draw anything because it won't
    // help and only consume (a lot of) CPU cycles
    qreal scale = qMin(transform().m11(), transform().m22());
    if (scale < 0.2001)
        return;

    // It is important to know that we effectively draw in *scene* coordinates,
    // not view coordinates! Before drawBackground() is called, the painter is
    // transformed accordingly by Qt.

    painter->setPen(0xE0E0E0);

    QPointF p0 = ArenaScene::nearestGridPoint(rect.topLeft());
    QRectF scRect = scene()->sceneRect();
    p0.rx() = qMax(p0.x(), scRect.left());
    p0.ry() = qMax(p0.y(), scRect.top());
    int visibleSceneWidth = qMin(rect.width(), scRect.width()) + CELL_SIZE;
    int visibleSceneHeight = qMin(rect.height(), scRect.height()) + CELL_SIZE;


    /*int numCells = (visibleSceneWidth / CELL_SIZE) * (visibleSceneHeight / CELL_SIZE);
    // Paintint a grid for > 1000 cells is just too much for the average CPU
    if (numCells > 1000)
    {
        static bool warned = false;
        if (!warned)
        {
            QTimer::singleShot(0, this, SLOT(slotGridPaintingDisabled()));
            warned = true;
        }
        return;
    }*/

    // One cell has a grid line to its left *and* to its right, therefore
    // we always have (number of cells + 1) grid lines
    for (int x = 0; x <= visibleSceneWidth; x += CELL_SIZE)
    {
        for (int y = 0; y <= visibleSceneHeight; y += CELL_SIZE)
        {
            int lineLength = 2;
            QPoint p = p0.toPoint() + QPoint(x, y)
                       - QPoint(ITEM_SIZE, ITEM_SIZE) / 2 - QPoint(SPACING, SPACING);

            p.rx() -= 1;
            p.ry() -= 1;
            drawCross(painter, p, lineLength);
            p.rx() += SPACING + 1;
            drawCross(painter, p, lineLength);
            p.rx() -= SPACING + 1;
            p.ry() += SPACING + 1;
            drawCross(painter, p, lineLength);
            p.rx() += SPACING + 1;
            drawCross(painter, p, lineLength);

        }
    }
}

void ArenaView::drawForeground(QPainter *painter, const QRectF &rect)
{
    m_controller->draw(painter, rect);
}

void ArenaView::mousePressEvent(QMouseEvent *event)
{
    bool forwardEvent = true;

    // Shortcut to inserting elements
    if (event->modifiers() & Qt::ShiftModifier && arenaScene()->selectedElements().count() == 1)
    {
        dragStarted();
        ArenaSceneElement *selection = arenaScene()->selectedElements().first();
        QString elementType = selection->element()->type()->name();
        m_controller->startInsertion(elementType, mapToScene(event->pos()));
        m_rubberBandActive = false;
        // Do not mess with item selection (set by ArenaController)
        forwardEvent = false;
    }
    else
    {
        m_rubberBandActive = itemAt(event->pos()) == 0;
    }

    if (m_rubberBandActive)
        setDragMode(QGraphicsView::RubberBandDrag);
    else
        setDragMode(QGraphicsView::NoDrag);

    m_lastMousePos = event->pos();
    m_lastMousePosScene = mapToScene(event->pos());

    if (forwardEvent)
        QGraphicsView::mousePressEvent(event);
}

void ArenaView::mouseMoveEvent(QMouseEvent *event)
{
    if (!arenaScene())
        return;

    bool forwardEvent = true;

    if (event->buttons() & Qt::LeftButton && !m_rubberBandActive)
    {
        QPointF mousePos = mapToScene(event->pos());
        QList<ArenaSceneElement*> selectedElements = arenaScene()->selectedElements();

        if (!m_controller->operationInProgress())
        {
            dragStarted();
            m_controller->startDrag(selectedElements);
        }

        // In this case, we will move the selected element directly to the
        // position of the mouse cursor
        if (selectedElements.count() == 1 &&
            !selectedElements.first()->element()->isItem())
        {
            m_controller->dragTo(mousePos);
        }
        else
        {
            // Offset the mouse has been moved by
            QPointF offset = mousePos - m_lastMousePosScene;
            m_controller->dragBy(offset);
        }

        forwardEvent = false;
    }
    else if (event->buttons() & Qt::RightButton)
    {
        QPoint diff = event->pos() - m_lastMousePos;

        int oldX = horizontalScrollBar()->value();
        int oldY = verticalScrollBar()->value();
        horizontalScrollBar()->setValue(oldX - diff.x());
        verticalScrollBar()->setValue(oldY - diff.y());
    }

    m_lastMousePos = event->pos();
    m_lastMousePosScene = mapToScene(event->pos());

    if (forwardEvent)
        QGraphicsView::mouseMoveEvent(event);
}

void ArenaView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    m_rubberBandActive = false;
    if (m_controller->operationInProgress())
    {
        m_controller->endOperation();
        // Update to make controller handles disappear
        update();
    }

    dragEnded();
}

ArenaScene *ArenaView::arenaScene()
{
    return dynamic_cast<ArenaScene*>(scene());
}

void ArenaView::wheelEvent(QWheelEvent *event)
{
    if (event->delta() > 0)
        slotZoomIn();
    else
        slotZoomOut();
}

#define MIN_SCALE 0.1
#define MAX_SCALE 5.0

void ArenaView::slotZoomIn()
{
    QTransform trans = transform().scale(2.0, 2.0);
    if (trans.m11() <= MAX_SCALE || trans.m22() <= MAX_SCALE)
        setTransform(trans);
}

void ArenaView::slotZoomOut()
{
    QTransform trans = transform().scale(0.5, 0.5);
    if (trans.m11() >= MIN_SCALE || trans.m22() >= MIN_SCALE)
        setTransform(trans);
}

void ArenaView::dragStarted()
{
    // Do not change visible scene rect while an element is being inserted
    if (scene())
        setSceneRect(scene()->sceneRect());
}

void ArenaView::dragEnded()
{
    // Change scene rect with scene again
    setSceneRect(QRectF());
}


void ArenaView::dragEnterEvent(QDragEnterEvent *event)
{
    dragStarted();

    QString elementTypeName = event->mimeData()->data("type");
    m_controller->startInsertion(elementTypeName, mapToScene(event->pos()));

    event->accept();
}

void ArenaView::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_controller->cancelInsertion();
    event->accept();

    dragEnded();
}

void ArenaView::dragMoveEvent(QDragMoveEvent *event)
{
    m_controller->dragTo(mapToScene(event->pos()));
    event->accept();
}

void ArenaView::dropEvent(QDropEvent *event)
{
    event->accept();
    m_controller->endOperation();
    // Make controller handles disappear
    update();
    dragEnded();
    //QGraphicsScene::dropEvent(event);
}
