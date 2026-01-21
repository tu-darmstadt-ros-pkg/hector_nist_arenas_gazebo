#ifndef ARENASCENEELEMENT_H
#define ARENASCENEELEMENT_H

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

class ArenaElement;

class ArenaSceneElement : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT

public:
    explicit ArenaSceneElement(ArenaElement *element);

    bool isEditorSample() { return m_isEditorSample; }
    void setIsEditorSample(bool isEditorSample);

    QPointF gridOffset();

    ArenaElement* element() const { return m_element; }

public slots:
    void slotPosChanged(ArenaElement *element, QPoint pos);
    void slotRotationChanged(ArenaElement *element, int rotation);

private:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    bool m_isEditorSample;
    ArenaElement *m_element;
};

#endif // ARENASCENEELEMENT_H
