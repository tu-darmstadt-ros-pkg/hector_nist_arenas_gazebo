#ifndef ARENASCENE_H
#define ARENASCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneDragDropEvent>
#include <QMap>

class Arena;
class ArenaElement;
class ArenaSceneElement;

const int ITEM_SIZE = 80;
const int SPACING = 15;
const int CELL_SIZE = ITEM_SIZE + SPACING;

class ArenaScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit ArenaScene(Arena* arena);

    static QPoint sceneToGrid(QPointF scenePos);
    static QPointF sceneToGridF(QPointF scenePos);
    static QPointF gridToScene(QPointF gridPos);

    static QPointF nearestGridPoint(QPointF scenePos);

    ArenaSceneElement *sceneElementFor(ArenaElement* element);

    QList<ArenaSceneElement*> selectedElements();

private slots:
    void slotElementAdded(ArenaElement *element);
    void slotElementRemoved(ArenaElement *element);
    void updateViewMargin();

private:
    void createGround();

    Arena * const m_arena;
    QMap<ArenaElement*, ArenaSceneElement*> m_elements;
};

#endif // ARENASCENE_H
