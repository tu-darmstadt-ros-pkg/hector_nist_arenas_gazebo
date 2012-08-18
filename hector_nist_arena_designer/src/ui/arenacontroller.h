#ifndef ARENACONTROLLER_H
#define ARENACONTROLLER_H

#include <QPointF>
#include <QRectF>
#include <QList>
#include <QMap>

class QPainter;
class Arena;
class ArenaScene;
class ArenaSceneElement;

class ArenaController
{
public:
    ArenaController(Arena *arena, ArenaScene *scene);

    void startInsertion(const QString &elementType, const QPointF &pos);
    void cancelInsertion();

    void startDrag(ArenaSceneElement *sceneElement);
    void startDrag(QList<ArenaSceneElement *> sceneElements);

    void dragTo(const QPointF &to);
    void dragBy(const QPointF &by);

    void setSnapToGrid(bool snap) { m_snapToGrid = snap; }
    /// If set to true, walls are automatically rotated to best fit the
    /// current position of the mouse cursor
    void setAutoRotateWalls(bool autoRotate) { m_autoRotateWalls = autoRotate; }

    void endOperation();

    bool operationInProgress() const;

    void draw(QPainter *painter, const QRectF &rect);

private:
    void dragTo(ArenaSceneElement *sceneElement, const QPointF &to);

    Arena *m_arena;
    ArenaScene *m_scene;
    /// SceneElement currently being manipulated through this controller
    QList<ArenaSceneElement*> m_activeElements;
    QMap<ArenaSceneElement*, QPointF> m_origPositions;
    QPointF m_totalMouseOffset;

    bool m_autoRotateWalls;
    bool m_snapToGrid;

    /// True iff. endDrag() has been called after last startDrag()
    bool m_lastDragFinished;
};

#endif // ARENACONTROLLER_H
