#include "app/filterpopup.h"

#include "app/icons.h"

#include <QAction>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidgetAction>

namespace jp {

using jp::crunch::FilterList;
using jp::office::sheet::ColorSpec;
using jp::office::sheet::FilterColumn;

namespace {

constexpr int kKindRole = Qt::UserRole;      // 1 a value, 2 a year, 3 a month, 4 a day, 5 blanks, 6 select all
constexpr int kYearRole = Qt::UserRole + 1, kMonthRole = Qt::UserRole + 2, kDayRole = Qt::UserRole + 3;
enum ItemKind { Value = 1, Year, Month, Day, Blanks, All };

QIcon swatch(const QColor &color)
{
    QPixmap pm(14, 14);
    pm.fill(color);
    return QIcon(pm);
}

QString colorName(const QColor &c) { return c.name().toUpper(); }

} // namespace

FilterPopup::FilterPopup(const Input &input, QWidget *parent) : QMenu(parent), m_input(input)
{
    setObjectName(QStringLiteral("filterPopup"));
    setToolTipsVisible(true);
    buildMenus();
}

void FilterPopup::buildMenus()
{
    QAction *asc = addAction(jp::icon(QStringLiteral("arrow-down-a-z")), tr("Sort A to Z"));
    asc->setObjectName(QStringLiteral("filterSortAscending"));
    asc->setCheckable(true);
    asc->setChecked(m_input.sortDirection > 0);
    QAction *desc = addAction(jp::icon(QStringLiteral("arrow-up-z-a")), tr("Sort Z to A"));
    desc->setObjectName(QStringLiteral("filterSortDescending"));
    desc->setCheckable(true);
    desc->setChecked(m_input.sortDirection < 0);
    connect(asc, &QAction::triggered, this, [this] { Q_EMIT sortRequested(false); });
    connect(desc, &QAction::triggered, this, [this] { Q_EMIT sortRequested(true); });
    m_sortColors = addMenu(tr("Sort by Color"));
    m_sortColors->setObjectName(QStringLiteral("filterSortColors"));
    m_sortColors->setEnabled(false);
    addSeparator();
    m_clear = addAction(jp::icon(QStringLiteral("filter-x")), tr("Clear Filter From \"%1\"").arg(m_input.columnName.left(40)));
    m_clear->setObjectName(QStringLiteral("filterClear"));
    m_clear->setEnabled(m_input.filtered);
    connect(m_clear, &QAction::triggered, this, [this] { Q_EMIT clearRequested(); });
    m_filterColors = addMenu(tr("Filter by Color"));
    m_filterColors->setObjectName(QStringLiteral("filterColors"));
    m_filterColors->setEnabled(false);
    m_kindMenu = addMenu(tr("Filters"));
    m_kindMenu->setObjectName(QStringLiteral("filterKinds"));
    m_kindMenu->setEnabled(false);
    addSeparator();
    // the list: a search box, the values with check boxes, and OK and Cancel
    auto *host = new QWidget;
    auto *layout = new QVBoxLayout(host);
    layout->setContentsMargins(6, 4, 6, 6);
    m_search = new QLineEdit;
    m_search->setObjectName(QStringLiteral("filterSearch"));
    m_search->setPlaceholderText(tr("Search"));
    m_search->setAccessibleName(tr("Search the values"));
    m_search->setClearButtonEnabled(true);
    layout->addWidget(m_search);
    m_tree = new QTreeWidget;
    m_tree->setObjectName(QStringLiteral("filterValues"));
    m_tree->setAccessibleName(tr("Values to show"));
    m_tree->setHeaderHidden(true);
    m_tree->setRootIsDecorated(true);
    m_tree->setUniformRowHeights(true);
    m_tree->setMinimumSize(240, 180);
    layout->addWidget(m_tree, 1);
    m_note = new QLabel(tr("Listing the values…"));
    m_note->setObjectName(QStringLiteral("filterNote"));
    m_note->setWordWrap(true);
    layout->addWidget(m_note);
    auto *buttons = new QHBoxLayout;
    buttons->addStretch(1);
    m_ok = new QPushButton(tr("OK"));
    m_ok->setObjectName(QStringLiteral("filterOk"));
    m_ok->setEnabled(false);
    m_cancel = new QPushButton(tr("Cancel"));
    m_cancel->setObjectName(QStringLiteral("filterCancel"));
    buttons->addWidget(m_ok);
    buttons->addWidget(m_cancel);
    layout->addLayout(buttons);
    auto *action = new QWidgetAction(this);
    action->setDefaultWidget(host);
    addAction(action);
    connect(m_search, &QLineEdit::textChanged, this, [this](const QString &t) { searched(t); });
    connect(m_tree, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem *item) { itemChanged(item); });
    connect(m_ok, &QPushButton::clicked, this, [this] {
        const FilterColumn c = conditionOfList();
        close();
        Q_EMIT conditionChosen(c);
    });
    connect(m_cancel, &QPushButton::clicked, this, [this] { close(); });
}

void FilterPopup::setLoading(bool on)
{
    m_note->setText(on ? tr("Listing the values…") : QString());
    m_ok->setEnabled(!on && m_listed);
}

void FilterPopup::setList(const FilterList &list)
{
    m_list = list;
    m_listed = true;
    // colors
    auto fillColors = [&](QMenu *menu, bool font, const std::vector<ColorSpec> &colors, bool none, bool sort) {
        menu->clear();
        for (const ColorSpec &c : colors) {
            const QColor look = m_input.toColor ? m_input.toColor(c) : QColor(Qt::gray);
            QAction *a = menu->addAction(swatch(look), colorName(look));
            connect(a, &QAction::triggered, this, [this, c, font, sort] {
                if (sort) {
                    Q_EMIT sortByColorRequested(font, c);
                    return;
                }
                FilterColumn f;
                f.kind = FilterColumn::Kind::Color;
                f.color = c;
                f.fontColor = font;
                Q_EMIT conditionChosen(f);
            });
        }
        if (none) {
            QAction *a = menu->addAction(font ? tr("No font color") : tr("No fill"));
            connect(a, &QAction::triggered, this, [this, font, sort] {
                if (sort) {
                    Q_EMIT sortByColorRequested(font, ColorSpec());
                    return;
                }
                FilterColumn f;
                f.kind = FilterColumn::Kind::Color;
                f.fontColor = font;
                Q_EMIT conditionChosen(f);
            });
        }
    };
    const bool anyCell = !list.cellColors.empty() || (list.noCellColor && !list.cellColors.empty());
    const bool anyFont = !list.fontColors.empty();
    m_filterColors->clear();
    m_sortColors->clear();
    if (anyCell || anyFont) {
        auto addHeading = [](QMenu *menu, const QString &text) {
            QAction *h = menu->addAction(text);
            h->setEnabled(false);
        };
        for (int sort = 0; sort < 2; ++sort) {
            QMenu *menu = sort ? m_sortColors : m_filterColors;
            if (anyCell) {
                addHeading(menu, tr("By cell color"));
                QMenu scratch;
                fillColors(&scratch, false, list.cellColors, list.noCellColor, sort);
                for (QAction *a : scratch.actions()) {
                    scratch.removeAction(a);
                    a->setParent(menu);
                    menu->addAction(a);
                }
            }
            if (anyFont) {
                if (anyCell) menu->addSeparator();
                addHeading(menu, tr("By font color"));
                QMenu scratch;
                fillColors(&scratch, true, list.fontColors, list.noFontColor, sort);
                for (QAction *a : scratch.actions()) {
                    scratch.removeAction(a);
                    a->setParent(menu);
                    menu->addAction(a);
                }
            }
        }
        m_filterColors->setEnabled(true);
        m_sortColors->setEnabled(true);
    }
    // the filters of the kind of column
    m_kindMenu->clear();
    auto ask = [&](QMenu *menu, const QString &text, FilterAsk what) {
        QAction *a = menu->addAction(text);
        connect(a, &QAction::triggered, this, [this, what] { Q_EMIT askRequested(what); });
        return a;
    };
    auto dynamic = [&](QMenu *menu, const QString &text, const QString &type) {
        QAction *a = menu->addAction(text);
        connect(a, &QAction::triggered, this, [this, type] {
            FilterColumn f;
            f.kind = FilterColumn::Kind::Dynamic;
            f.dynamic = type;
            Q_EMIT conditionChosen(f);
        });
    };
    if (list.kind == FilterList::Kind::Number) {
        m_kindMenu->setTitle(tr("Number Filters"));
        ask(m_kindMenu, tr("Equals…"), FilterAsk::Equals);
        ask(m_kindMenu, tr("Does Not Equal…"), FilterAsk::NotEquals);
        ask(m_kindMenu, tr("Greater Than…"), FilterAsk::Greater);
        ask(m_kindMenu, tr("Greater Than Or Equal To…"), FilterAsk::GreaterEqual);
        ask(m_kindMenu, tr("Less Than…"), FilterAsk::Less);
        ask(m_kindMenu, tr("Less Than Or Equal To…"), FilterAsk::LessEqual);
        ask(m_kindMenu, tr("Between…"), FilterAsk::Between);
        m_kindMenu->addSeparator();
        ask(m_kindMenu, tr("Top 10…"), FilterAsk::Top10);
        dynamic(m_kindMenu, tr("Above Average"), QStringLiteral("aboveAverage"));
        dynamic(m_kindMenu, tr("Below Average"), QStringLiteral("belowAverage"));
        m_kindMenu->addSeparator();
        ask(m_kindMenu, tr("Custom Filter…"), FilterAsk::Custom);
    } else if (list.kind == FilterList::Kind::Date) {
        m_kindMenu->setTitle(tr("Date Filters"));
        ask(m_kindMenu, tr("Equals…"), FilterAsk::Equals);
        ask(m_kindMenu, tr("Before…"), FilterAsk::Before);
        ask(m_kindMenu, tr("After…"), FilterAsk::After);
        ask(m_kindMenu, tr("Between…"), FilterAsk::Between);
        m_kindMenu->addSeparator();
        dynamic(m_kindMenu, tr("Tomorrow"), QStringLiteral("tomorrow"));
        dynamic(m_kindMenu, tr("Today"), QStringLiteral("today"));
        dynamic(m_kindMenu, tr("Yesterday"), QStringLiteral("yesterday"));
        m_kindMenu->addSeparator();
        dynamic(m_kindMenu, tr("Next Week"), QStringLiteral("nextWeek"));
        dynamic(m_kindMenu, tr("This Week"), QStringLiteral("thisWeek"));
        dynamic(m_kindMenu, tr("Last Week"), QStringLiteral("lastWeek"));
        m_kindMenu->addSeparator();
        dynamic(m_kindMenu, tr("Next Month"), QStringLiteral("nextMonth"));
        dynamic(m_kindMenu, tr("This Month"), QStringLiteral("thisMonth"));
        dynamic(m_kindMenu, tr("Last Month"), QStringLiteral("lastMonth"));
        m_kindMenu->addSeparator();
        dynamic(m_kindMenu, tr("Next Quarter"), QStringLiteral("nextQuarter"));
        dynamic(m_kindMenu, tr("This Quarter"), QStringLiteral("thisQuarter"));
        dynamic(m_kindMenu, tr("Last Quarter"), QStringLiteral("lastQuarter"));
        m_kindMenu->addSeparator();
        dynamic(m_kindMenu, tr("Next Year"), QStringLiteral("nextYear"));
        dynamic(m_kindMenu, tr("This Year"), QStringLiteral("thisYear"));
        dynamic(m_kindMenu, tr("Last Year"), QStringLiteral("lastYear"));
        dynamic(m_kindMenu, tr("Year to Date"), QStringLiteral("yearToDate"));
        m_kindMenu->addSeparator();
        QMenu *period = m_kindMenu->addMenu(tr("All Dates in the Period"));
        QMenu *quarters = period->addMenu(tr("Quarter"));
        for (int q = 1; q <= 4; ++q) dynamic(quarters, tr("Quarter %1").arg(q), QStringLiteral("Q%1").arg(q));
        QMenu *months = period->addMenu(tr("Month"));
        for (int m = 1; m <= 12; ++m) dynamic(months, QLocale().monthName(m, QLocale::LongFormat), QStringLiteral("M%1").arg(m));
        m_kindMenu->addSeparator();
        ask(m_kindMenu, tr("Custom Filter…"), FilterAsk::Custom);
    } else {
        m_kindMenu->setTitle(tr("Text Filters"));
        ask(m_kindMenu, tr("Equals…"), FilterAsk::Equals);
        ask(m_kindMenu, tr("Does Not Equal…"), FilterAsk::NotEquals);
        m_kindMenu->addSeparator();
        ask(m_kindMenu, tr("Begins With…"), FilterAsk::BeginsWith);
        ask(m_kindMenu, tr("Ends With…"), FilterAsk::EndsWith);
        m_kindMenu->addSeparator();
        ask(m_kindMenu, tr("Contains…"), FilterAsk::Contains);
        ask(m_kindMenu, tr("Does Not Contain…"), FilterAsk::NotContains);
        m_kindMenu->addSeparator();
        ask(m_kindMenu, tr("Custom Filter…"), FilterAsk::Custom);
    }
    m_kindMenu->setEnabled(true);
    fillTree();
    m_note->setText(list.truncated ? tr("The column has more different values than the list holds (%1). Use a filter by condition to find the rest.").arg(jp::crunch::kMaxFilterItems) : QString());
    m_ok->setEnabled(true);
}

void FilterPopup::fillTree()
{
    const QSignalBlocker block(m_tree);
    m_tree->clear();
    m_all = new QTreeWidgetItem(m_tree, QStringList(tr("(Select All)")));
    m_all->setData(0, kKindRole, All);
    m_all->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    const FilterColumn &cur = m_input.current;
    const bool filtering = m_input.filtered && cur.kind == FilterColumn::Kind::Values;
    QSet<QString> shown;
    for (const QString &v : cur.values) shown.insert(v.toCaseFolded());
    auto checked = [&](const QString &text) { return !filtering || shown.contains(text.toCaseFolded()); };
    if (m_list.kind == FilterList::Kind::Date) {
        auto dateChecked = [&](int y, int m, int d) {
            if (!filtering) return true;
            using U = FilterColumn::DateGroup::Unit;
            for (const FilterColumn::DateGroup &g : cur.dates) {
                if (g.year != y) continue;
                if (g.unit == U::Year) return true;
                if (g.month != m) continue;
                if (g.unit == U::Month) return true;
                if (g.day == d && g.unit == U::Day) return true;
            }
            return false;
        };
        int items = 0;
        for (const auto &year : m_list.dates) {
            auto *yi = new QTreeWidgetItem(m_all, QStringList(QString::number(year.first)));
            yi->setData(0, kKindRole, Year);
            yi->setData(0, kYearRole, year.first);
            yi->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
            for (const auto &month : year.second) {
                auto *mi = new QTreeWidgetItem(yi, QStringList(QLocale().monthName(month.first, QLocale::LongFormat)));
                mi->setData(0, kKindRole, Month);
                mi->setData(0, kYearRole, year.first);
                mi->setData(0, kMonthRole, month.first);
                mi->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
                for (int day : month.second) {
                    if (++items > 40000) break;
                    auto *di = new QTreeWidgetItem(mi, QStringList(QString::number(day)));
                    di->setData(0, kKindRole, Day);
                    di->setData(0, kYearRole, year.first);
                    di->setData(0, kMonthRole, month.first);
                    di->setData(0, kDayRole, day);
                    di->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
                    di->setCheckState(0, dateChecked(year.first, month.first, day) ? Qt::Checked : Qt::Unchecked);
                }
            }
        }
    } else {
        for (const FilterList::Item &item : m_list.items) {
            auto *i = new QTreeWidgetItem(m_all, QStringList(item.text));
            i->setData(0, kKindRole, Value);
            const int cells = int(std::min<qint64>(item.count, 100000000));
            i->setToolTip(0, tr("%n cell(s)", nullptr, cells));
            i->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
            i->setCheckState(0, checked(item.text) ? Qt::Checked : Qt::Unchecked);
        }
    }
    m_blanks = nullptr;
    if (m_list.blanks) {
        m_blanks = new QTreeWidgetItem(m_all, QStringList(tr("(Blanks)")));
        m_blanks->setData(0, kKindRole, Blanks);
        m_blanks->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        m_blanks->setCheckState(0, !filtering || cur.blank ? Qt::Checked : Qt::Unchecked);
    }
    m_all->setCheckState(0, Qt::Checked);
    // the parents take their state from the leaves
    std::function<void(QTreeWidgetItem *)> settle = [&](QTreeWidgetItem *item) {
        if (item->childCount() == 0) return;
        int on = 0, off = 0;
        for (int i = 0; i < item->childCount(); ++i) {
            settle(item->child(i));
            const Qt::CheckState s = item->child(i)->checkState(0);
            if (s == Qt::Checked) ++on;
            else if (s == Qt::Unchecked) ++off;
        }
        item->setCheckState(0, off == 0 && on > 0 ? Qt::Checked : on == 0 ? Qt::Unchecked : Qt::PartiallyChecked);
    };
    settle(m_all);
    m_all->setExpanded(true);
}

bool FilterPopup::leafChecked(QTreeWidgetItem *item) const { return item->checkState(0) == Qt::Checked; }

// A check box was changed: what is under it follows (only what shows, if a search has hidden some), and what is over it is made to say what is under it.
void FilterPopup::itemChanged(QTreeWidgetItem *item)
{
    if (m_busy) return;
    m_busy = true;
    const Qt::CheckState state = item->checkState(0);
    if (state != Qt::PartiallyChecked && item->childCount() > 0) {
        std::function<void(QTreeWidgetItem *)> setBelow = [&](QTreeWidgetItem *parent) {
            for (int i = 0; i < parent->childCount(); ++i) {
                QTreeWidgetItem *c = parent->child(i);
                if (c->isHidden()) continue;
                c->setCheckState(0, state);
                setBelow(c);
            }
        };
        setBelow(item);
    }
    refreshParents(item);
    m_busy = false;
}

void FilterPopup::refreshParents(QTreeWidgetItem *from)
{
    for (QTreeWidgetItem *p = from->parent(); p; p = p->parent()) {
        int on = 0, off = 0;
        for (int i = 0; i < p->childCount(); ++i) {
            const Qt::CheckState s = p->child(i)->checkState(0);
            if (s == Qt::Checked) ++on;
            else if (s == Qt::Unchecked) ++off;
        }
        p->setCheckState(0, off == 0 && on > 0 ? Qt::Checked : on == 0 ? Qt::Unchecked : Qt::PartiallyChecked);
    }
}

// A search shows the values that have all the words; a date shows when it, or what it is in, has them.
void FilterPopup::searched(const QString &text)
{
    if (!m_all) return;
    const QStringList words = text.toCaseFolded().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    std::function<bool(QTreeWidgetItem *, const QString &)> visit = [&](QTreeWidgetItem *item, const QString &path) {
        const QString here = (path + QLatin1Char(' ') + item->text(0)).toCaseFolded();
        if (item->childCount() == 0) {
            bool all = true;
            for (const QString &w : words)
                if (!here.contains(w)) {
                    all = false;
                    break;
                }
            item->setHidden(!all);
            return all;
        }
        bool any = false;
        for (int i = 0; i < item->childCount(); ++i) any = visit(item->child(i), item == m_all ? QString() : here) || any;
        item->setHidden(!any && item != m_all);
        return any;
    };
    visit(m_all, QString());
    m_all->setHidden(false);
    m_all->setText(0, words.isEmpty() ? tr("(Select All)") : tr("(Select All Search Results)"));
    m_all->setExpanded(true);
    if (!words.isEmpty()) m_tree->expandAll();
}

FilterColumn FilterPopup::conditionOfList() const
{
    FilterColumn c;
    if (!m_all || !m_listed) return c;
    // everything checked: no condition
    if (m_all->checkState(0) == Qt::Checked) return c;   // (even when the list is cut short: nothing was left out by the person)
    c.kind = FilterColumn::Kind::Values;
    c.blank = m_blanks && m_blanks->checkState(0) == Qt::Checked;
    if (m_list.kind == FilterList::Kind::Date) {
        using G = FilterColumn::DateGroup;
        for (int y = 0; y < m_all->childCount(); ++y) {
            QTreeWidgetItem *yi = m_all->child(y);
            if (yi->data(0, kKindRole).toInt() != Year) continue;
            const int year = yi->data(0, kYearRole).toInt();
            if (yi->checkState(0) == Qt::Checked) {
                G g;
                g.unit = G::Unit::Year;
                g.year = year;
                c.dates.append(g);
                continue;
            }
            if (yi->checkState(0) == Qt::Unchecked) continue;
            for (int m = 0; m < yi->childCount(); ++m) {
                QTreeWidgetItem *mi = yi->child(m);
                const int month = mi->data(0, kMonthRole).toInt();
                if (mi->checkState(0) == Qt::Checked) {
                    G g;
                    g.unit = G::Unit::Month;
                    g.year = year;
                    g.month = month;
                    c.dates.append(g);
                    continue;
                }
                if (mi->checkState(0) == Qt::Unchecked) continue;
                for (int d = 0; d < mi->childCount(); ++d) {
                    QTreeWidgetItem *di = mi->child(d);
                    if (di->checkState(0) != Qt::Checked) continue;
                    G g;
                    g.unit = G::Unit::Day;
                    g.year = year;
                    g.month = month;
                    g.day = di->data(0, kDayRole).toInt();
                    c.dates.append(g);
                }
            }
        }
    } else {
        for (int i = 0; i < m_all->childCount(); ++i) {
            QTreeWidgetItem *item = m_all->child(i);
            if (item->data(0, kKindRole).toInt() == Value && item->checkState(0) == Qt::Checked) c.values.append(item->text(0));
        }
    }
    return c;
}

} // namespace jp
