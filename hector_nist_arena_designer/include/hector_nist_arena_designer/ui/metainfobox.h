#ifndef METAINFOBOX_H
#define METAINFOBOX_H

#include <QGroupBox>
#include <QString>

typedef QPair<QString, QString> QStringPair;
class QGridLayout;
class QLabel;

class MetaInfoBox : public QGroupBox
{
    Q_OBJECT
    
public:
    explicit MetaInfoBox(QWidget *parent = 0);
    ~MetaInfoBox();

    /// Sets a list of key-value pairs as meta info
    void setMetaInfos(QVector<QStringPair> metaInfos);

private:
    /// Removes all meta info
    void clear();

    QGridLayout* m_layout;
    QVector<QLabel*> m_labels;

    /// To prevent flickering, avoid redrawing if meta info didn't change
    QVector<QStringPair> m_metaInfos;
};

#endif // METAINFOBOX_H
