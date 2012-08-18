#include "metainfobox.h"

#include <QLabel>
#include <QGridLayout>
#include <QLabel>
#include <QDebug>

MetaInfoBox::MetaInfoBox(QWidget *parent)
    : QGroupBox(parent)
    , m_layout(new QGridLayout)
{
    setLayout(m_layout);
}

QString boldText(QString text)
{
    return QString("<b>") + text + QString("</b>");
}

void MetaInfoBox::setMetaInfos(QVector<QStringPair> metaInfos)
{
    if (m_metaInfos == metaInfos)
        return;

    m_metaInfos = metaInfos;

    qDeleteAll(m_labels);
    m_labels.clear();

    foreach (QStringPair metaInfo, metaInfos)
    {
        QLabel* keyLabel = new QLabel(boldText(metaInfo.first));
        QLabel* valueLabel = new QLabel(metaInfo.second);
        valueLabel->setWordWrap(true);
        // Number of meta infos we've got so far
        int metaInfoCount = m_labels.size() / 2;
        m_layout->addWidget(keyLabel, metaInfoCount, 0);
        m_layout->addWidget(valueLabel, metaInfoCount, 1);
        m_labels.append(keyLabel);
        m_labels.append(valueLabel);
    }
}

void MetaInfoBox::clear()
{
}

MetaInfoBox::~MetaInfoBox()
{
    clear();
}
