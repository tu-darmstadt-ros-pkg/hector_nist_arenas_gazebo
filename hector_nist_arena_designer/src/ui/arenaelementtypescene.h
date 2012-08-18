#ifndef ARENAELEMENTTYPESCENE_H
#define ARENAELEMENTTYPESCENE_H

#include <QGraphicsScene>
#include <QDomElement>

class ArenaElement;
class ArenaElementTypeRegistry;

class ArenaElementTypeScene : public QGraphicsScene
{
    Q_OBJECT

public:
    ArenaElementTypeScene(ArenaElementTypeRegistry *registry);

    void loadConfig(const QDomElement& arenaElements);

    bool event(QEvent *event);

signals:
    /// Emitted when mouse hovers over element
    void elementHovered(ArenaElement *element);

private:
    ArenaElementTypeRegistry *m_registry;
};

#endif // ARENAELEMENTTYPESCENE_H
