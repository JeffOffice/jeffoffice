#include "app/listmenus.h"

#include "app/editor.h"
#include "app/settings.h"
#include "app/textdialogs.h"
#include "core/color.h"
#include "text/textprops.h"

#include <QApplication>
#include <QCoreApplication>
#include <QGridLayout>
#include <QMenu>
#include <QTextBlock>
#include <QTextList>
#include <QToolButton>
#include <QWidgetAction>

namespace jp {

namespace {

QString tr(const char *s) { return QCoreApplication::translate("jp::ListMenus", s); }

constexpr int kMostRecentBullets = 8;

QStringList recentBullets() { return Settings::get().value(QStringLiteral("lists/recentBullets")).toStringList(); }

void rememberBullet(const QString &character, const QString &font)
{
    QStringList recent = recentBullets();
    const QString entry = character + QLatin1Char('|') + font;
    recent.removeAll(entry);
    recent.prepend(entry);
    while (recent.size() > kMostRecentBullets) recent.removeLast();
    Settings::get().setValue(QStringLiteral("lists/recentBullets"), recent);
}

// A grid of buttons in a menu: each cell's text and tooltip, and what choosing it does (the menu closes first).
void gridAction(QMenu *menu, const QVector<std::pair<QString, QString>> &cells, int columns, const std::function<void(int)> &choose)
{
    auto *host = new QWidget(menu);
    auto *grid = new QGridLayout(host);
    grid->setContentsMargins(6, 4, 6, 4);
    grid->setSpacing(2);
    for (int i = 0; i < cells.size(); ++i) {
        auto *b = new QToolButton(host);
        b->setText(cells[i].first);
        b->setToolTip(cells[i].second);
        b->setAccessibleName(cells[i].second);
        b->setAutoRaise(true);
        b->setFocusPolicy(Qt::NoFocus);
        b->setMinimumSize(44, 34);
        QFont f = b->font();
        f.setPointSize(f.pointSize() + 3);
        b->setFont(f);
        QObject::connect(b, &QToolButton::clicked, menu, [menu, choose, i] {
            menu->close();
            choose(i);
        });
        grid->addWidget(b, i / columns, i % columns);
    }
    auto *action = new QWidgetAction(menu);
    action->setDefaultWidget(host);
    menu->addAction(action);
}

QWidget *dialogParent(QWidget *owner) { return owner ? owner->window() : nullptr; }

SymbolPicker symbolPicker()
{
    return [](QWidget *parent, QString *ch, QString *font) {
        bool picked = false;
        symbolDialog(parent, [&](QWidget *, const QString &c, const QString &f) {
            *ch = c;
            *font = f;
            picked = true;
            return true;
        });
        return picked;
    };
}

ListLevelLook numberLook(const char *fmt, const char *text)
{
    ListLevelLook l;
    l.bullet = false;
    l.numFmt = QString::fromLatin1(fmt);
    l.text = QString::fromLatin1(text);
    return l;
}

ListLevelLook bulletLook(const QString &ch)
{
    ListLevelLook l;
    l.text = ch;
    return l;
}

} // namespace

QTextListFormat listFormatOf(const ListLevelLook &look)
{
    QTextListFormat lf;
    lf.setIndent(0);
    if (look.bullet) {
        lf.setStyle(QTextListFormat::ListDisc);
        if (!look.text.isEmpty()) lf.setProperty(tp::BulletChar, look.text);
        if (!look.font.isEmpty()) lf.setProperty(tp::BulletFont, look.font);
        if (look.sizePoints > 0) lf.setProperty(tp::BulletSize, look.sizePoints);
        if (look.color.isValid()) lf.setProperty(tp::BulletColor, ColorRef::rgb(look.color).toString());
        if (!look.picture.isEmpty()) lf.setProperty(tp::BulletPicture, look.picture);
        return lf;
    }
    if (look.numFmt == QLatin1String("lowerLetter")) lf.setStyle(QTextListFormat::ListLowerAlpha);
    else if (look.numFmt == QLatin1String("upperLetter")) lf.setStyle(QTextListFormat::ListUpperAlpha);
    else if (look.numFmt == QLatin1String("lowerRoman")) lf.setStyle(QTextListFormat::ListLowerRoman);
    else if (look.numFmt == QLatin1String("upperRoman")) lf.setStyle(QTextListFormat::ListUpperRoman);
    else lf.setStyle(QTextListFormat::ListDecimal);
    const int at = look.text.indexOf(QLatin1String("%1"));
    lf.setNumberPrefix(at > 0 ? look.text.left(at) : QString());
    lf.setNumberSuffix(at >= 0 ? look.text.mid(at + 2) : QStringLiteral("."));
    lf.setStart(std::max(1, look.start));
    // The format number the apps keep for the seven forms they offer.
    const QString pre = lf.numberPrefix(), suf = lf.numberSuffix();
    int format = 1;
    if (look.numFmt == QLatin1String("lowerLetter")) format = 2;
    else if (look.numFmt == QLatin1String("upperLetter")) format = 3;
    else if (look.numFmt == QLatin1String("lowerRoman")) format = 4;
    else if (look.numFmt == QLatin1String("upperRoman")) format = 5;
    else if (suf == QLatin1String(")")) format = pre == QLatin1String("(") ? 7 : 6;
    lf.setProperty(tp::NumberFormat, format);
    return lf;
}

ListLevelLook listLookOf(const QTextListFormat &format)
{
    ListLevelLook look;
    const QTextListFormat::Style style = format.style();
    if (style == QTextListFormat::ListDisc || style == QTextListFormat::ListCircle || style == QTextListFormat::ListSquare || format.hasProperty(tp::BulletChar)) {
        look.bullet = true;
        look.text = format.stringProperty(tp::BulletChar);
        if (look.text.isEmpty()) look.text = style == QTextListFormat::ListCircle ? QStringLiteral("◦") : style == QTextListFormat::ListSquare ? QStringLiteral("▪") : QStringLiteral("•");
        look.font = format.stringProperty(tp::BulletFont);
        look.sizePoints = format.doubleProperty(tp::BulletSize);
        look.picture = format.property(tp::BulletPicture).toByteArray();
        if (const QString ref = format.stringProperty(tp::BulletColor); !ref.isEmpty()) look.color = ColorRef::fromString(ref).rgbValue();
        return look;
    }
    look.bullet = false;
    look.numFmt = style == QTextListFormat::ListLowerAlpha ? QStringLiteral("lowerLetter") : style == QTextListFormat::ListUpperAlpha ? QStringLiteral("upperLetter")
                : style == QTextListFormat::ListLowerRoman ? QStringLiteral("lowerRoman") : style == QTextListFormat::ListUpperRoman ? QStringLiteral("upperRoman") : QStringLiteral("decimal");
    look.text = format.numberPrefix() + QStringLiteral("%1") + format.numberSuffix();
    look.start = format.start();
    return look;
}

QVector<ListLevelLook> bulletPicks()
{
    QVector<ListLevelLook> out;
    for (const char *ch : {"•", "○", "■", "◆", "➢", "✓", "–"}) out << bulletLook(QString::fromUtf8(ch));
    return out;
}

QVector<ListLevelLook> numberPicks()
{
    return {numberLook("decimal", "%1."), numberLook("decimal", "%1)"), numberLook("upperRoman", "%1."), numberLook("upperLetter", "%1."), numberLook("lowerLetter", "%1)"),
            numberLook("lowerLetter", "%1."), numberLook("lowerRoman", "%1.")};
}

QVector<MultilevelPick> multilevelPicks()
{
    auto cycle = [](std::initializer_list<ListLevelLook> looks) {
        QVector<ListLevelLook> out;
        for (int i = 0; i < 9; ++i) out << *(looks.begin() + i % looks.size());
        return out;
    };
    return {{QStringLiteral("1)a)i)"), tr("1) a) i)"), cycle({numberLook("decimal", "%1)"), numberLook("lowerLetter", "%1)"), numberLook("lowerRoman", "%1)")})},
            {QStringLiteral("1.a.i."), tr("1. a. i."), cycle({numberLook("decimal", "%1."), numberLook("lowerLetter", "%1."), numberLook("lowerRoman", "%1.")})},
            {QStringLiteral("I.A.1."), tr("I. A. 1. a)"), cycle({numberLook("upperRoman", "%1."), numberLook("upperLetter", "%1."), numberLook("decimal", "%1."), numberLook("lowerLetter", "%1)")})},
            {QStringLiteral("bullets"), tr("Bullets, in three shapes"), cycle({bulletLook(QStringLiteral("•")), bulletLook(QStringLiteral("○")), bulletLook(QStringLiteral("■"))})}};
}

QMenu *bulletsMenu(QWidget *owner, Editor *ed, QAction *more, const ListBoxesChoice &mode)
{
    auto *m = new QMenu(owner);
    QObject::connect(m, &QMenu::aboutToShow, m, [m, owner, ed, more, mode] {
        m->clear();
        auto apply = [ed](const ListLevelLook &look) { ed->setListFormat(listFormatOf(look)); };
        QVector<std::pair<QString, QString>> recent;
        QVector<ListLevelLook> recentLooks;
        for (const QString &entry : recentBullets()) {
            const int bar = entry.indexOf(QLatin1Char('|'));
            ListLevelLook l = bulletLook(entry.left(bar));
            l.font = entry.mid(bar + 1);
            recentLooks << l;
            recent.push_back({l.text, tr("Recently used: %1").arg(l.text)});
        }
        if (!recent.isEmpty()) {
            m->addSection(tr("Recently Used Bullets"));
            gridAction(m, recent, 8, [apply, recentLooks](int i) { apply(recentLooks[i]); });
        }
        m->addSection(tr("Bullet Library"));
        const QVector<ListLevelLook> picks = bulletPicks();
        QVector<std::pair<QString, QString>> cells;
        for (const ListLevelLook &p : picks) cells.push_back({p.text, p.text});
        gridAction(m, cells, 7, [apply, picks](int i) {
            rememberBullet(picks[i].text, picks[i].font);
            apply(picks[i]);
        });
        QAction *none = m->addAction(tr("None"));
        QObject::connect(none, &QAction::triggered, m, [ed] { ed->setList(0); });
        m->addSeparator();
        QAction *define = m->addAction(tr("Define New Bullet…"));
        QObject::connect(define, &QAction::triggered, m, [owner, ed, apply, mode] {
            ListLevelLook look = listLookOf(ed->isEditingText() && ed->cursor().block().textList() ? ed->cursor().block().textList()->format() : QTextListFormat());
            if (!look.bullet) look = bulletLook(QStringLiteral("•"));
            if (!defineBulletDialog(dialogParent(owner), &look, symbolPicker(), mode ? mode() : ListBoxes::Plain)) return;
            rememberBullet(look.text, look.font);
            apply(look);
        });
        if (more) m->addAction(more);
    });
    return m;
}

QMenu *numberingMenu(QWidget *owner, Editor *ed, QAction *more, const ListBoxesChoice &mode)
{
    auto *m = new QMenu(owner);
    m->addSection(tr("Numbering Library"));
    const QVector<ListLevelLook> picks = numberPicks();
    QVector<std::pair<QString, QString>> cells;
    for (const ListLevelLook &p : picks) {
        QString shown = p.text;
        shown.replace(QStringLiteral("%1"), p.numFmt == QLatin1String("upperRoman") ? QStringLiteral("I") : p.numFmt == QLatin1String("upperLetter") ? QStringLiteral("A") :
                                           p.numFmt == QLatin1String("lowerLetter") ? QStringLiteral("a") : p.numFmt == QLatin1String("lowerRoman") ? QStringLiteral("i") : QStringLiteral("1"));
        cells.push_back({shown, shown});
    }
    gridAction(m, cells, 4, [ed, picks](int i) { ed->setListFormat(listFormatOf(picks[i])); });
    QAction *none = m->addAction(tr("None"));
    QObject::connect(none, &QAction::triggered, m, [ed] { ed->setList(0); });
    m->addSeparator();
    QAction *define = m->addAction(tr("Define New Number Format…"));
    QObject::connect(define, &QAction::triggered, m, [owner, ed, mode] {
        ListLevelLook look = numberLook("decimal", "%1.");
        if (ed->isEditingText() && ed->cursor().block().textList()) {
            const ListLevelLook now = listLookOf(ed->cursor().block().textList()->format());
            if (!now.bullet) look = now;
        }
        if (!defineNumberFormatDialog(dialogParent(owner), &look, mode ? mode() : ListBoxes::Plain)) return;
        ed->setListFormat(listFormatOf(look));
    });
    if (more) m->addAction(more);
    return m;
}

QMenu *multilevelMenu(QWidget *owner, Editor *ed, const ListBoxesChoice &mode)
{
    auto *m = new QMenu(owner);
    m->addSection(tr("List Library"));
    const QVector<MultilevelPick> picks = multilevelPicks();
    for (const MultilevelPick &p : picks) {
        QAction *a = m->addAction(p.name);
        const QVector<ListLevelLook> levels = p.levels;
        QObject::connect(a, &QAction::triggered, m, [ed, levels] {
            QVector<QTextListFormat> formats;
            for (const ListLevelLook &l : levels) formats << listFormatOf(l);
            ed->setMultilevelList(formats);
        });
    }
    m->addSeparator();
    QMenu *levels = m->addMenu(tr("Change List Level"));
    for (int i = 0; i < 9; ++i) {
        QAction *a = levels->addAction(tr("Level %1").arg(i + 1));
        QObject::connect(a, &QAction::triggered, m, [ed, i] { ed->setListLevel(i); });
    }
    QAction *define = m->addAction(tr("Define New Multilevel List…"));
    QObject::connect(define, &QAction::triggered, m, [owner, ed, mode] {
        QVector<ListLevelLook> looks = multilevelPicks().first().levels;
        if (!defineMultilevelPlainDialog(dialogParent(owner), &looks, symbolPicker(), mode ? mode() : ListBoxes::Plain)) return;
        QVector<QTextListFormat> formats;
        for (const ListLevelLook &l : looks) formats << listFormatOf(l);
        ed->setMultilevelList(formats);
    });
    return m;
}

} // namespace jp
