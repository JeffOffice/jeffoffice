#include "app/sectionui.h"

#include "app/icons.h"
#include "app/theme.h"

#include <QCoreApplication>
#include <QFontMetrics>
#include <QMenu>
#include <QPainter>
#include <QPolygonF>
#include <algorithm>

namespace jp {

void paintSectionHeader(QPainter *p, const QRect &r, const QString &name, int count, bool collapsed, bool hover)
{
    p->save();
    QColor text = uiText();
    if (hover) {
        QColor wash = uiAccent();
        wash.setAlphaF(uiDark() ? 0.18f : 0.10f);
        p->fillRect(r, wash);
    }
    // The triangle.
    const QPointF c(r.left() + 12, r.center().y() + 0.5);
    QPolygonF triangle;
    if (collapsed) triangle << c + QPointF(-2.5, -4.5) << c + QPointF(3, 0) << c + QPointF(-2.5, 4.5);
    else triangle << c + QPointF(-4.5, -2.5) << c + QPointF(4.5, -2.5) << c + QPointF(0, 3);
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(Qt::NoPen);
    p->setBrush(text);
    p->drawPolygon(triangle);
    p->setRenderHint(QPainter::Antialiasing, false);
    QFont f = p->font();
    f.setWeight(QFont::DemiBold);
    p->setFont(f);
    const QString number = QStringLiteral("(%1)").arg(count);
    QFontMetrics fm(f);
    const int left = r.left() + 24, countWidth = fm.horizontalAdvance(number);
    const QRect nameBox(left, r.top(), std::max(0, r.right() - left - countWidth - 12), r.height());
    p->setPen(text);
    p->drawText(nameBox, Qt::AlignVCenter | Qt::AlignLeft, fm.elidedText(name, Qt::ElideRight, nameBox.width()));
    text.setAlphaF(0.6f);
    p->setPen(text);
    const int used = std::min(nameBox.width(), fm.horizontalAdvance(name));
    p->drawText(QRect(left + used + 6, r.top(), countWidth + 6, r.height()), Qt::AlignVCenter | Qt::AlignLeft, number);
    p->setPen(QPen(uiLine(), 1));
    p->drawLine(r.left() + 4, r.bottom(), r.right() - 4, r.bottom());
    p->restore();
}

void fillSectionMenu(QMenu &menu, const SectionMenu &c)
{
    auto add = [&](const QString &text, const std::function<void()> &fn, bool enabled = true) {
        QAction *a = menu.addAction(text, fn);
        a->setEnabled(enabled && bool(fn));
    };
    add(QCoreApplication::translate("jp::SectionMenu", "Rename Section"), c.rename);
    add(QCoreApplication::translate("jp::SectionMenu", "Remove Section"), c.remove);
    add(QCoreApplication::translate("jp::SectionMenu", "Remove Section && Slides"), c.removeWithSlides);
    menu.addSeparator();
    add(QCoreApplication::translate("jp::SectionMenu", "Move Section Up"), c.moveUp, c.canMoveUp);
    add(QCoreApplication::translate("jp::SectionMenu", "Move Section Down"), c.moveDown, c.canMoveDown);
    menu.addSeparator();
    add(QCoreApplication::translate("jp::SectionMenu", "Collapse All"), c.collapseAll);
    add(QCoreApplication::translate("jp::SectionMenu", "Expand All"), c.expandAll);
}

} // namespace jp
