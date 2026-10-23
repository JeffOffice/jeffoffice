#include "app/sheettabs.h"

#include <QPainter>
#include <QStylePainter>

namespace jp::crunch {

SheetTabBar::SheetTabBar(QWidget *parent) : QTabBar(parent)
{
    setObjectName(QStringLiteral("sheetTabs"));
    setShape(QTabBar::RoundedSouth);
    setExpanding(false);
    setDrawBase(false);
    setUsesScrollButtons(true);
    setElideMode(Qt::ElideRight);
    setAccessibleName(tr("Sheets"));
}

void SheetTabBar::setStripe(int tab, const QColor &color)
{
    if (tab < 0) return;
    if (m_stripes.size() <= tab) m_stripes.resize(tab + 1);
    m_stripes[tab] = color;
    update();
}

void SheetTabBar::clearTabs()
{
    while (count() > 0) removeTab(count() - 1);
    m_stripes.clear();
}

QSize SheetTabBar::tabSizeHint(int index) const
{
    QSize s = QTabBar::tabSizeHint(index);
    s.setHeight(std::max(s.height(), 24));
    s.setWidth(std::min(std::max(s.width(), 60), 220));
    return s;
}

void SheetTabBar::paintEvent(QPaintEvent *e)
{
    QTabBar::paintEvent(e);
    QPainter p(this);
    for (int i = 0; i < count() && i < m_stripes.size(); ++i) {
        if (!m_stripes[i].isValid()) continue;
        const QRect r = tabRect(i);
        p.fillRect(QRect(r.left() + 4, r.bottom() - 3, r.width() - 8, 3), m_stripes[i]);
    }
}

} // namespace jp::crunch
