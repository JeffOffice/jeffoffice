#include "app/highlightmenu.h"

#include "app/editor.h"
#include "app/icons.h"
#include "core/color.h"
#include "text/highlightcolors.h"

#include <QCoreApplication>
#include <QMenu>
#include <QPainter>
#include <QPixmap>

namespace jp {

namespace {

QIcon colorSquare(const QColor &c)
{
    QPixmap pm(16, 16);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setPen(QColor(0, 0, 0, 90));
    p.setBrush(c);
    p.drawRect(1, 1, 13, 13);
    return QIcon(pm);
}

QString colorName(const QString &key)
{
    using QCA = QCoreApplication;
    if (key == QLatin1String("yellow")) return QCA::translate("HighlightMenu", "Yellow");
    if (key == QLatin1String("green")) return QCA::translate("HighlightMenu", "Green");
    if (key == QLatin1String("cyan")) return QCA::translate("HighlightMenu", "Cyan");
    if (key == QLatin1String("magenta")) return QCA::translate("HighlightMenu", "Magenta");
    if (key == QLatin1String("blue")) return QCA::translate("HighlightMenu", "Blue");
    if (key == QLatin1String("red")) return QCA::translate("HighlightMenu", "Red");
    if (key == QLatin1String("darkBlue")) return QCA::translate("HighlightMenu", "Dark Blue");
    if (key == QLatin1String("darkCyan")) return QCA::translate("HighlightMenu", "Dark Cyan");
    if (key == QLatin1String("darkGreen")) return QCA::translate("HighlightMenu", "Dark Green");
    if (key == QLatin1String("darkMagenta")) return QCA::translate("HighlightMenu", "Dark Magenta");
    if (key == QLatin1String("darkRed")) return QCA::translate("HighlightMenu", "Dark Red");
    if (key == QLatin1String("darkYellow")) return QCA::translate("HighlightMenu", "Dark Yellow");
    if (key == QLatin1String("darkGray")) return QCA::translate("HighlightMenu", "Dark Gray");
    if (key == QLatin1String("lightGray")) return QCA::translate("HighlightMenu", "Light Gray");
    if (key == QLatin1String("black")) return QCA::translate("HighlightMenu", "Black");
    return QCA::translate("HighlightMenu", "White");
}

}   // namespace

QMenu *highlightMenu(QWidget *owner, Editor *ed, QAction *stop)
{
    auto *m = new QMenu(owner);
    int n = 0;
    const tp::HighlightColor *all = tp::highlightColors(&n);
    for (int i = 0; i < n; ++i) {
        const QColor color = QColor::fromRgb(all[i].rgb);
        QAction *a = m->addAction(colorSquare(color), colorName(QString::fromLatin1(all[i].name)));
        QObject::connect(a, &QAction::triggered, ed, [ed, color] { ed->chooseHighlight(ColorRef::rgb(color)); });
    }
    m->addSeparator();
    QAction *none = m->addAction(icon(QStringLiteral("circle-slash")), QCoreApplication::translate("HighlightMenu", "No Color"));
    QObject::connect(none, &QAction::triggered, ed, [ed] { ed->chooseHighlight(ColorRef()); });
    if (stop) m->addAction(stop);
    return m;
}

}   // namespace jp
