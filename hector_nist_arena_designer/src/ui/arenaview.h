#ifndef ARENAVIEW_H
#define ARENAVIEW_H

#include <QGraphicsView>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

#include "arenacontroller.h"

class Arena;
class ArenaScene;
class ArenaSceneElement;

class ArenaView : public QGraphicsView
{
    Q_OBJECT
public:
    ArenaView(ArenaController *controller, QWidget *parent = 0);

    void drawForeground(QPainter *painter, const QRectF &rect);
    void drawBackground(QPainter *painter, const QRectF &rect);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    void setArena(Arena *arena) { m_arena = arena; }

public slots:
    void slotZoomIn();
    void slotZoomOut();

private slots:
    void slotGridPaintingDisabled();

private:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);

    void dragStarted();
    void dragEnded();

    ArenaScene* arenaScene();

    Arena* m_arena;
    // Remember if we're currently in rubber band selection mode
    bool m_rubberBandActive;
    QPoint m_lastMousePos;
    QPointF m_lastMousePosScene;
    ArenaController *m_controller;
};

#endif // ARENAVIEW_H
