// Sparklines in the window: Insert > Sparklines (Line, Column, Win/Loss) from the cells selected, and the Sparkline tab that shows while the selection has one: the data and where it goes, the
// kind, which points are marked, the colors and the line's weight, the axis, and what to do with empty and hidden cells, and the commands to group, ungroup, and clear. Each change is one step
// of Undo (EditSession::setSparklines). How a sparkline is drawn is in grid/sparklinepaint.h.
#include "app/crunchwindow.h"

#include "app/icons.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "core/editsession.h"
#include "sheet/chartmake.h"
#include "sheet/cellref.h"

#include <QAction>
#include <QActionGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QWidgetAction>

namespace jp {

using jp::office::sheet::CellRange;
using jp::office::sheet::ColorSpec;
using jp::office::sheet::Sparkline;
using jp::office::sheet::SparklineGroup;

namespace {

struct Pick {
    int group = 0, index = 0;
};

// The cells of a range as "A1:C1" or "A1" with the sheet's name left off (what a person types), and back.
bool parseData(const QString &text, QString *sheetName, CellRange *range)
{
    QString ref = text.trimmed();
    sheetName->clear();
    const qsizetype bang = ref.lastIndexOf(QLatin1Char('!'));
    if (bang >= 0) {
        QString name = ref.left(bang);
        ref = ref.mid(bang + 1);
        if (name.size() >= 2 && name.startsWith(QLatin1Char('\'')) && name.endsWith(QLatin1Char('\''))) name = name.mid(1, name.size() - 2).replace(QLatin1String("''"), QLatin1String("'"));
        *sheetName = name;
    }
    ref.remove(QLatin1Char('$'));
    return jp::office::sheet::parseRangeRef(ref, range) && range->valid;
}

QPixmap swatch(const QColor &c)
{
    QPixmap pm(16, 16);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setPen(QColor(0x80, 0x80, 0x80));
    p.setBrush(c);
    p.drawRect(1, 1, 13, 13);
    return pm;
}

const struct Look {
    const char *name;
    quint32 rgb;
} kLooks[] = {{QT_TR_NOOP("Blue"), 0x376092}, {QT_TR_NOOP("Orange"), 0xD2691E}, {QT_TR_NOOP("Gray"), 0x7F7F7F}, {QT_TR_NOOP("Gold"), 0xBF9000}, {QT_TR_NOOP("Green"), 0x2E7D32},
              {QT_TR_NOOP("Dark Blue"), 0x1F3864}, {QT_TR_NOOP("Dark Red"), 0x8B1A1A}, {QT_TR_NOOP("Teal"), 0x007C89}, {QT_TR_NOOP("Purple"), 0x6B3FA0}, {QT_TR_NOOP("Dark Gray"), 0x404040},
              {QT_TR_NOOP("Black"), 0x000000}, {QT_TR_NOOP("Red"), 0xC0392B}};

} // namespace

// The sparklines the selection holds: those drawn in a selected cell.
static QVector<Pick> picksIn(const QVector<SparklineGroup> &groups, const CellRange &sel)
{
    QVector<Pick> out;
    for (int g = 0; g < groups.size(); ++g)
        for (int i = 0; i < groups[g].sparklines.size(); ++i)
            if (sel.contains(groups[g].sparklines[i].row, groups[g].sparklines[i].col)) out.append({g, i});
    return out;
}

QVector<int> CrunchWindow::selectedSparklineGroups() const
{
    QVector<int> out;
    if (!m_session || m_sheet < 0) return out;
    for (const Pick &p : picksIn(m_session->sparklinesOf(m_sheet), selectedRange()))
        if (!out.contains(p.group)) out.append(p.group);
    return out;
}

void CrunchWindow::changeSparklineGroups(const std::function<void(jp::office::sheet::SparklineGroup *)> &change, const QString &text)
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed()) return;
    QVector<SparklineGroup> groups = m_session->sparklinesOf(m_sheet);
    const QVector<int> which = selectedSparklineGroups();
    if (which.isEmpty()) return;
    for (const int g : which) change(&groups[g]);
    const auto o = m_session->setSparklines(m_sheet, groups, text);
    if (!o.ok) notice(tr("Sparklines"), o.problem);
    updateSparklineTab();
}

// The sparklines for data and the cells they are drawn in: one for each row or column of the data, as many as there are cells (one cell takes all the data). False, with the reason, if the
// counts do not fit.
bool CrunchWindow::makeSparklines(const QString &dataText, const QString &whereText, QVector<Sparkline> *out, QString *why) const
{
    QString dataSheet, whereSheet;
    CellRange data, where;
    if (!parseData(dataText, &dataSheet, &data)) {
        *why = tr("The data range is not cells. Write it as A1:F3.");
        return false;
    }
    if (!parseData(whereText, &whereSheet, &where)) {
        *why = tr("The location range is not cells. Write it as G1:G3.");
        return false;
    }
    if (!whereSheet.isEmpty() && whereSheet.compare(m_loaded->book->info().sheets[m_sheet].name, Qt::CaseInsensitive) != 0) {
        *why = tr("The sparklines have to be in cells of this sheet.");
        return false;
    }
    if (dataSheet.isEmpty()) dataSheet = m_loaded->book->info().sheets[m_sheet].name;
    const qint64 cells = where.cellCount();
    const qint64 rows = qint64(data.row2) - data.row1 + 1, cols = qint64(data.col2) - data.col1 + 1;
    if (cells > jp::office::sheet::kMaxSparklines) {
        *why = tr("That is more sparklines than a sheet can have.");
        return false;
    }
    bool byRows;
    const bool vertical = where.col1 == where.col2;
    if (cells == 1) byRows = true;
    else if (vertical && cells == rows) byRows = true;
    else if (!vertical && cells == cols) byRows = false;
    else if (cells == rows) byRows = true;
    else if (cells == cols) byRows = false;
    else {
        *why = tr("The data has %1 rows and %2 columns, and there are %3 cells for the sparklines. Each sparkline reads one row or one column, so the counts must match.").arg(rows).arg(cols).arg(cells);
        return false;
    }
    out->clear();
    qint64 n = 0;
    for (quint32 r = where.row1; r <= where.row2; ++r)
        for (quint32 c = where.col1; c <= where.col2; ++c, ++n) {
            Sparkline s;
            s.row = r;
            s.col = c;
            CellRange part = data;
            if (cells > 1) {
                if (byRows) part.row1 = part.row2 = data.row1 + quint32(n);
                else part.col1 = part.col2 = data.col1 + quint32(n);
            }
            s.source = jp::office::sheet::chartRef(dataSheet, part);
            out->append(s);
        }
    return true;
}

void CrunchWindow::insertSparklinesCommand(jp::office::sheet::SparklineGroup::Type type)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || m_chartSheetShown) return;
    const CellRange sel = selectedRange();
    QString data = sel.cellCount() > 1 ? jp::office::sheet::rangeName(sel) : QString();
    QString where;
    if (m_sparklineAsker) {
        if (!m_sparklineAsker(&data, &where)) return;
    } else {
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Create Sparklines"));
        auto *form = new QFormLayout(&dialog);
        auto *dataBox = new QLineEdit(data, &dialog);
        dataBox->setObjectName(QStringLiteral("sparkline.data"));
        dataBox->setAccessibleName(tr("Data range"));
        auto *whereBox = new QLineEdit(where, &dialog);
        whereBox->setObjectName(QStringLiteral("sparkline.where"));
        whereBox->setAccessibleName(tr("Location range"));
        form->addRow(tr("Data range:"), dataBox);
        form->addRow(tr("Location range:"), whereBox);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        form->addRow(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() != QDialog::Accepted) return;
        data = dataBox->text();
        where = whereBox->text();
    }
    QVector<Sparkline> made;
    QString why;
    if (!makeSparklines(data, where, &made, &why)) {
        notice(tr("Sparklines"), why);
        return;
    }
    QVector<SparklineGroup> groups = m_session->sparklinesOf(m_sheet);
    // a cell has one sparkline: those in these cells are replaced
    for (SparklineGroup &g : groups)
        for (int i = g.sparklines.size() - 1; i >= 0; --i)
            for (const Sparkline &m : made)
                if (m.row == g.sparklines[i].row && m.col == g.sparklines[i].col) {
                    g.sparklines.removeAt(i);
                    break;
                }
    SparklineGroup g;
    g.type = type;
    g.empty = SparklineGroup::Empty::Gap;
    g.lineWeight = 0.75;
    g.series = ColorSpec::fromRgb(0x376092);
    g.negativeColor = ColorSpec::fromRgb(0xD00000);
    g.axisColor = ColorSpec::fromRgb(0x000000);
    g.markerColor = ColorSpec::fromRgb(0xD00000);
    g.firstColor = ColorSpec::fromRgb(0xD00000);
    g.lastColor = ColorSpec::fromRgb(0xD00000);
    g.highColor = ColorSpec::fromRgb(0xD00000);
    g.lowColor = ColorSpec::fromRgb(0xD00000);
    if (type == SparklineGroup::Type::WinLoss) g.negative = true;
    g.sparklines = made;
    groups.append(g);
    const auto o = m_session->setSparklines(m_sheet, groups, tr("Create Sparklines"));
    if (!o.ok) {
        notice(tr("Sparklines"), o.problem);
        return;
    }
    // the cursor goes to the first of them, so that the tab shows
    if (!made.isEmpty()) m_view->selectCell(made.first().row, made.first().col);
    updateSparklineTab();
}

// Edit Data: the data and the cells of the group the selection is in, or of the one sparkline.
void CrunchWindow::editSparklineData(bool wholeGroup)
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed()) return;
    QVector<SparklineGroup> groups = m_session->sparklinesOf(m_sheet);
    const QVector<Pick> picks = picksIn(groups, selectedRange());
    if (picks.isEmpty()) return;
    const Pick first = picks.first();
    const SparklineGroup &g = groups[first.group];
    // the data and the cells now: the block that holds them
    auto block = [&](const QVector<int> &which, QString *dataText, QString *whereText) {
        CellRange where, data;
        QString sheetName;
        for (const int i : which) {
            const Sparkline &s = g.sparklines[i];
            if (!where.valid) {
                where.row1 = where.row2 = s.row;
                where.col1 = where.col2 = s.col;
                where.valid = true;
            } else {
                where.row1 = std::min(where.row1, s.row);
                where.row2 = std::max(where.row2, s.row);
                where.col1 = std::min(where.col1, s.col);
                where.col2 = std::max(where.col2, s.col);
            }
            CellRange part;
            QString name;
            if (!parseData(s.source, &name, &part)) continue;
            if (sheetName.isEmpty()) sheetName = name;
            if (!data.valid) data = part;
            else {
                data.row1 = std::min(data.row1, part.row1);
                data.row2 = std::max(data.row2, part.row2);
                data.col1 = std::min(data.col1, part.col1);
                data.col2 = std::max(data.col2, part.col2);
            }
        }
        *whereText = where.valid ? jp::office::sheet::rangeName(where) : QString();
        *dataText = data.valid ? (sheetName.isEmpty() || sheetName.compare(m_loaded->book->info().sheets[m_sheet].name, Qt::CaseInsensitive) == 0 ? QString() : jp::office::sheet::chartRef(sheetName, data).section(QLatin1Char('!'), 0, 0) + QLatin1Char('!')) + jp::office::sheet::rangeName(data) : QString();
    };
    QVector<int> which;
    if (wholeGroup) {
        for (int i = 0; i < g.sparklines.size(); ++i) which.append(i);
    } else {
        which.append(first.index);
    }
    QString data, where;
    block(which, &data, &where);
    if (m_sparklineAsker) {
        if (!m_sparklineAsker(&data, &where)) return;
    } else {
        QDialog dialog(this);
        dialog.setWindowTitle(wholeGroup ? tr("Edit Sparklines") : tr("Edit Sparkline Data"));
        auto *form = new QFormLayout(&dialog);
        auto *dataBox = new QLineEdit(data, &dialog);
        dataBox->setObjectName(QStringLiteral("sparkline.data"));
        dataBox->setAccessibleName(tr("Data range"));
        auto *whereBox = new QLineEdit(where, &dialog);
        whereBox->setObjectName(QStringLiteral("sparkline.where"));
        whereBox->setAccessibleName(tr("Location range"));
        whereBox->setReadOnly(!wholeGroup);
        form->addRow(tr("Data range:"), dataBox);
        form->addRow(tr("Location range:"), whereBox);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        form->addRow(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() != QDialog::Accepted) return;
        data = dataBox->text();
        where = whereBox->text();
    }
    QVector<Sparkline> made;
    QString why;
    if (!makeSparklines(data, where, &made, &why)) {
        notice(tr("Sparklines"), why);
        return;
    }
    // the new ones take the place of those that were edited (and of any other in the cells they are for)
    SparklineGroup &edited = groups[first.group];
    QVector<Sparkline> keep;
    for (int i = 0; i < edited.sparklines.size(); ++i)
        if (!which.contains(i)) keep.append(edited.sparklines[i]);
    edited.sparklines = keep + made;
    for (int gi = 0; gi < groups.size(); ++gi)
        if (gi != first.group)
            for (int i = groups[gi].sparklines.size() - 1; i >= 0; --i)
                for (const Sparkline &m : made)
                    if (m.row == groups[gi].sparklines[i].row && m.col == groups[gi].sparklines[i].col) {
                        groups[gi].sparklines.removeAt(i);
                        break;
                    }
    const auto o = m_session->setSparklines(m_sheet, groups, wholeGroup ? tr("Edit Sparklines") : tr("Edit Sparkline Data"));
    if (!o.ok) notice(tr("Sparklines"), o.problem);
    updateSparklineTab();
}

void CrunchWindow::groupSparklines(bool together)
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed()) return;
    QVector<SparklineGroup> groups = m_session->sparklinesOf(m_sheet);
    const QVector<Pick> picks = picksIn(groups, selectedRange());
    if (picks.isEmpty()) return;
    if (together) {
        // one group with the look of the first: the selected sparklines leave their groups for it
        SparklineGroup merged = groups[picks.first().group];
        merged.sparklines.clear();
        for (const Pick &p : picks) merged.sparklines.append(groups[p.group].sparklines[p.index]);
        for (int k = picks.size() - 1; k >= 0; --k) groups[picks[k].group].sparklines.removeAt(picks[k].index);
        groups.append(merged);
        const auto o = m_session->setSparklines(m_sheet, groups, tr("Group Sparklines"));
        if (!o.ok) notice(tr("Sparklines"), o.problem);
    } else {
        // each selected sparkline is a group of its own, with the look it had
        QVector<SparklineGroup> alone;
        for (const Pick &p : picks) {
            SparklineGroup one = groups[p.group];
            one.sparklines = {groups[p.group].sparklines[p.index]};
            alone.append(one);
        }
        for (int k = picks.size() - 1; k >= 0; --k) groups[picks[k].group].sparklines.removeAt(picks[k].index);
        groups += alone;
        const auto o = m_session->setSparklines(m_sheet, groups, tr("Ungroup Sparklines"));
        if (!o.ok) notice(tr("Sparklines"), o.problem);
    }
    updateSparklineTab();
}

void CrunchWindow::clearSparklines(bool wholeGroups)
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed()) return;
    QVector<SparklineGroup> groups = m_session->sparklinesOf(m_sheet);
    const QVector<Pick> picks = picksIn(groups, selectedRange());
    if (picks.isEmpty()) return;
    if (wholeGroups) {
        QVector<int> gone;
        for (const Pick &p : picks)
            if (!gone.contains(p.group)) gone.append(p.group);
        std::sort(gone.begin(), gone.end(), std::greater<int>());
        for (const int g : gone) groups.removeAt(g);
    } else {
        for (int k = picks.size() - 1; k >= 0; --k) groups[picks[k].group].sparklines.removeAt(picks[k].index);
    }
    const auto o = m_session->setSparklines(m_sheet, groups, wholeGroups ? tr("Clear Sparkline Groups") : tr("Clear Sparklines"));
    if (!o.ok) notice(tr("Sparklines"), o.problem);
    updateSparklineTab();
}

void CrunchWindow::createSparklineActions()
{
    struct Kind {
        const char *id;
        const char *text;
        const char *icon;
        SparklineGroup::Type type;
    };
    static const Kind kinds[] = {{"line", QT_TR_NOOP("Line"), "activity", SparklineGroup::Type::Line},
                                 {"column", QT_TR_NOOP("Column"), "chart-column", SparklineGroup::Type::Column},
                                 {"winloss", QT_TR_NOOP("Win/Loss"), "arrow-up-down", SparklineGroup::Type::WinLoss}};
    for (const Kind &k : kinds) {
        const SparklineGroup::Type type = k.type;
        const QString text = QCoreApplication::translate("jp::CrunchWindow", k.text);
        mk(QStringLiteral("spark.insert.") + QLatin1String(k.id), text, k.icon, QKeySequence(), [this, type] { insertSparklinesCommand(type); });
        mk(QStringLiteral("spark.type.") + QLatin1String(k.id), text, k.icon, QKeySequence(), [this, type] {
            changeSparklineGroups([type](SparklineGroup *g) { g->type = type; }, tr("Sparkline Type"));
        }, true);
        m_opsActions << QStringLiteral("spark.insert.") + QLatin1String(k.id) << QStringLiteral("spark.type.") + QLatin1String(k.id);
    }
    struct Show {
        const char *id;
        const char *text;
        const char *icon;
        bool SparklineGroup::*field;
    };
    static const Show shows[] = {{"high", QT_TR_NOOP("High Point"), "arrow-up-to-line", &SparklineGroup::high},        {"low", QT_TR_NOOP("Low Point"), "arrow-down-to-line", &SparklineGroup::low},
                                 {"negative", QT_TR_NOOP("Negative Points"), "minus", &SparklineGroup::negative},      {"first", QT_TR_NOOP("First Point"), "arrow-left-to-line", &SparklineGroup::first},
                                 {"last", QT_TR_NOOP("Last Point"), "arrow-right-to-line", &SparklineGroup::last},     {"markers", QT_TR_NOOP("Markers"), "circle-dot", &SparklineGroup::markers}};
    for (const Show &s : shows) {
        auto field = s.field;
        const QString id = QStringLiteral("spark.show.") + QLatin1String(s.id);
        mk(id, QCoreApplication::translate("jp::CrunchWindow", s.text), s.icon, QKeySequence(), [this, id, field] {
            const bool on = act(id)->isChecked();
            changeSparklineGroups([field, on](SparklineGroup *g) { g->*field = on; }, tr("Show Sparkline Points"));
        }, true);
        m_opsActions << id;
    }
    mk("spark.editGroup", tr("Edit Group Location and Data…"), "square-pen", QKeySequence(), [this] { editSparklineData(true); });
    mk("spark.editOne", tr("Edit Single Sparkline's Data…"), "pencil", QKeySequence(), [this] { editSparklineData(false); });
    mk("spark.group", tr("Group"), "group", QKeySequence(), [this] { groupSparklines(true); });
    mk("spark.ungroup", tr("Ungroup"), "ungroup", QKeySequence(), [this] { groupSparklines(false); });
    mk("spark.clearSelected", tr("Clear Selected Sparklines"), "eraser", QKeySequence(), [this] { clearSparklines(false); });
    mk("spark.clearGroups", tr("Clear Selected Sparkline Groups"), "trash-2", QKeySequence(), [this] { clearSparklines(true); });
    struct Empty {
        const char *id;
        const char *text;
        SparklineGroup::Empty how;
    };
    static const Empty empties[] = {{"gaps", QT_TR_NOOP("Show Empty Cells as Gaps"), SparklineGroup::Empty::Gap},
                                    {"zero", QT_TR_NOOP("Show Empty Cells as Zero"), SparklineGroup::Empty::Zero},
                                    {"span", QT_TR_NOOP("Connect Data Points with a Line"), SparklineGroup::Empty::Span}};
    for (const Empty &e : empties) {
        const SparklineGroup::Empty how = e.how;
        const QString id = QStringLiteral("spark.empty.") + QLatin1String(e.id);
        mk(id, QCoreApplication::translate("jp::CrunchWindow", e.text), QString(), QKeySequence(), [this, how] {
            changeSparklineGroups([how](SparklineGroup *g) { g->empty = how; }, tr("Empty Cells in Sparklines"));
        }, true);
        m_opsActions << id;
    }
    mk("spark.hidden", tr("Show Data in Hidden Rows and Columns"), QString(), QKeySequence(), [this] {
        const bool on = act("spark.hidden")->isChecked();
        changeSparklineGroups([on](SparklineGroup *g) { g->hidden = on; }, tr("Hidden Cells in Sparklines"));
    }, true);
    mk("spark.axis.show", tr("Show Axis"), QString(), QKeySequence(), [this] {
        const bool on = act("spark.axis.show")->isChecked();
        changeSparklineGroups([on](SparklineGroup *g) { g->showAxis = on; }, tr("Sparkline Axis"));
    }, true);
    mk("spark.axis.rtl", tr("Plot Data Right to Left"), QString(), QKeySequence(), [this] {
        const bool on = act("spark.axis.rtl")->isChecked();
        changeSparklineGroups([on](SparklineGroup *g) { g->rightToLeft = on; }, tr("Sparkline Axis"));
    }, true);
    m_opsActions << QStringLiteral("spark.editGroup") << QStringLiteral("spark.editOne") << QStringLiteral("spark.group") << QStringLiteral("spark.ungroup") << QStringLiteral("spark.clearSelected")
                 << QStringLiteral("spark.clearGroups") << QStringLiteral("spark.hidden") << QStringLiteral("spark.axis.show") << QStringLiteral("spark.axis.rtl");
}

void CrunchWindow::updateSparklineTab()
{
    const QVector<int> which = selectedSparklineGroups();
    if (m_ribbon) m_ribbon->setContextVisible(QStringLiteral("Sparkline"), !which.isEmpty());
    if (which.isEmpty()) return;
    const SparklineGroup &g = m_session->sparklinesOf(m_sheet)[which.first()];
    auto set = [this](const QString &id, bool on) {
        if (QAction *a = act(id)) {
            const QSignalBlocker block(a);
            a->setChecked(on);
        }
    };
    set(QStringLiteral("spark.type.line"), g.type == SparklineGroup::Type::Line);
    set(QStringLiteral("spark.type.column"), g.type == SparklineGroup::Type::Column);
    set(QStringLiteral("spark.type.winloss"), g.type == SparklineGroup::Type::WinLoss);
    set(QStringLiteral("spark.show.high"), g.high);
    set(QStringLiteral("spark.show.low"), g.low);
    set(QStringLiteral("spark.show.negative"), g.negative);
    set(QStringLiteral("spark.show.first"), g.first);
    set(QStringLiteral("spark.show.last"), g.last);
    set(QStringLiteral("spark.show.markers"), g.markers);
    set(QStringLiteral("spark.empty.gaps"), g.empty == SparklineGroup::Empty::Gap);
    set(QStringLiteral("spark.empty.zero"), g.empty == SparklineGroup::Empty::Zero);
    set(QStringLiteral("spark.empty.span"), g.empty == SparklineGroup::Empty::Span);
    set(QStringLiteral("spark.hidden"), g.hidden);
    set(QStringLiteral("spark.axis.show"), g.showAxis);
    set(QStringLiteral("spark.axis.rtl"), g.rightToLeft);
    // (markers apply to a line only)
    if (QAction *a = act(QStringLiteral("spark.show.markers"))) a->setEnabled(g.type == SparklineGroup::Type::Line && editingAllowed());
}

void CrunchWindow::sparklineRibbonParts(RibbonParts &parts)
{
    parts.colors[QStringLiteral("sparklineDesign")] = QColor(0x4E, 0x7D, 0x3A);
    auto colorMenu = [this](const QString &name, const std::function<void(SparklineGroup *, const ColorSpec &)> &set) {
        return [this, name, set](QWidget *owner) -> QMenu * {
            auto *menu = new QMenu(owner);
            menu->setObjectName(name);
            connect(menu, &QMenu::aboutToShow, this, [this, menu, set] {
                menu->clear();
                auto *wa = new QWidgetAction(menu);
                wa->setDefaultWidget(colorGridWidget(workbookColors(), [this, menu, set](const ColorRef &c) {
                    menu->close();
                    const ColorSpec spec = colorSpecOf(c);
                    changeSparklineGroups([set, spec](SparklineGroup *g) { set(g, spec); }, tr("Sparkline Color"));
                }, menu));
                menu->addAction(wa);
            });
            return menu;
        };
    };
    parts.menus[QStringLiteral("sparkColor")] = [this, colorMenu](QWidget *owner) -> QMenu * {
        QMenu *menu = colorMenu(QStringLiteral("sparklineColorMenu"), [](SparklineGroup *g, const ColorSpec &c) { g->series = c; })(owner);
        connect(menu, &QMenu::aboutToShow, this, [this, menu] {
            menu->addSeparator();
            QMenu *weight = menu->addMenu(tr("Weight"));
            weight->setObjectName(QStringLiteral("sparklineWeightMenu"));
            const QVector<int> groups = selectedSparklineGroups();
            const double now = groups.isEmpty() ? 0.75 : m_session->sparklinesOf(m_sheet)[groups.first()].lineWeight;
            auto *radio = new QActionGroup(weight);
            for (const double w : {0.25, 0.5, 0.75, 1.0, 1.5, 2.25, 3.0, 4.5, 6.0}) {
                QAction *a = weight->addAction(tr("%1 pt").arg(w));
                a->setCheckable(true);
                a->setChecked(std::abs(now - w) < 0.001);
                radio->addAction(a);
                connect(a, &QAction::triggered, this, [this, w] { changeSparklineGroups([w](SparklineGroup *g) { g->lineWeight = w; }, tr("Sparkline Weight")); });
            }
        });
        return menu;
    };
    parts.menus[QStringLiteral("sparkMarkerColor")] = [this](QWidget *owner) -> QMenu * {
        auto *menu = new QMenu(owner);
        menu->setObjectName(QStringLiteral("sparklineMarkerColorMenu"));
        connect(menu, &QMenu::aboutToShow, this, [this, menu] {
            menu->clear();
            struct Part {
                const char *text;
                ColorSpec SparklineGroup::*field;
            };
            static const Part items[] = {{QT_TR_NOOP("Negative Points"), &SparklineGroup::negativeColor}, {QT_TR_NOOP("Markers"), &SparklineGroup::markerColor},
                                         {QT_TR_NOOP("High Point"), &SparklineGroup::highColor},         {QT_TR_NOOP("Low Point"), &SparklineGroup::lowColor},
                                         {QT_TR_NOOP("First Point"), &SparklineGroup::firstColor},       {QT_TR_NOOP("Last Point"), &SparklineGroup::lastColor}};
            for (const Part &p : items) {
                QMenu *sub = menu->addMenu(QCoreApplication::translate("jp::CrunchWindow", p.text));
                auto field = p.field;
                auto *wa = new QWidgetAction(sub);
                wa->setDefaultWidget(colorGridWidget(workbookColors(), [this, menu, field](const ColorRef &c) {
                    menu->close();
                    const ColorSpec spec = colorSpecOf(c);
                    changeSparklineGroups([field, spec](SparklineGroup *g) { g->*field = spec; }, tr("Sparkline Marker Color"));
                }, sub));
                sub->addAction(wa);
            }
        });
        return menu;
    };
    parts.menus[QStringLiteral("sparkLooks")] = [this](QWidget *owner) -> QMenu * {
        auto *menu = new QMenu(owner);
        menu->setObjectName(QStringLiteral("sparklineLooksMenu"));
        connect(menu, &QMenu::aboutToShow, this, [this, menu] {
            menu->clear();
            for (const Look &l : kLooks) {
                const quint32 rgb = l.rgb;
                QAction *a = menu->addAction(QIcon(swatch(QColor::fromRgb(rgb))), QCoreApplication::translate("jp::CrunchWindow", l.name));
                connect(a, &QAction::triggered, this, [this, rgb] {
                    changeSparklineGroups([rgb](SparklineGroup *g) {
                        const ColorSpec c = ColorSpec::fromRgb(rgb);
                        g->series = c;
                        g->markerColor = c;
                        g->firstColor = c;
                        g->lastColor = c;
                        g->highColor = ColorSpec::fromRgb(0x2E7D32);
                        g->lowColor = ColorSpec::fromRgb(0xD00000);
                        g->negativeColor = rgb == 0xC0392B || rgb == 0x8B1A1A ? ColorSpec::fromRgb(0x000000) : ColorSpec::fromRgb(0xD00000);
                    }, tr("Sparkline Style"));
                });
            }
        });
        return menu;
    };
    parts.menus[QStringLiteral("sparkAxis")] = [this](QWidget *owner) -> QMenu * {
        auto *menu = new QMenu(owner);
        menu->setObjectName(QStringLiteral("sparklineAxisMenu"));
        connect(menu, &QMenu::aboutToShow, this, [this, menu] {
            menu->clear();
            menu->addAction(act(QStringLiteral("spark.axis.show")));
            menu->addAction(act(QStringLiteral("spark.axis.rtl")));
            const QVector<int> picked = selectedSparklineGroups();
            if (picked.isEmpty()) return;
            const SparklineGroup &g = m_session->sparklinesOf(m_sheet)[picked.first()];
            for (const bool lowest : {true, false}) {
                menu->addSeparator();
                QMenu *sub = menu->addMenu(lowest ? tr("Vertical Axis Minimum") : tr("Vertical Axis Maximum"));
                auto *radio = new QActionGroup(sub);
                const SparklineGroup::Axis now = lowest ? g.minAxis : g.maxAxis;
                struct Way {
                    SparklineGroup::Axis axis;
                    QString text;
                };
                const Way ways[] = {{SparklineGroup::Axis::Individual, tr("Automatic for Each Sparkline")}, {SparklineGroup::Axis::Group, tr("Same for All Sparklines")}, {SparklineGroup::Axis::Custom, tr("Custom Value…")}};
                for (const Way &w : ways) {
                    QAction *a = sub->addAction(w.text);
                    a->setCheckable(true);
                    a->setChecked(now == w.axis);
                    radio->addAction(a);
                    const SparklineGroup::Axis axis = w.axis;
                    connect(a, &QAction::triggered, this, [this, lowest, axis] {
                        double value = 0;
                        if (axis == SparklineGroup::Axis::Custom) {
                            const QVector<int> sel = selectedSparklineGroups();
                            if (sel.isEmpty()) return;
                            const SparklineGroup &now = m_session->sparklinesOf(m_sheet)[sel.first()];
                            bool ok = false;
                            value = lowest ? now.manualMin : now.manualMax;
                            if (m_axisValueAsker) ok = m_axisValueAsker(lowest, &value);
                            else value = QInputDialog::getDouble(this, tr("Sparkline Axis"), lowest ? tr("Minimum value:") : tr("Maximum value:"), value, -1e15, 1e15, 4, &ok);
                            if (!ok) return;
                        }
                        changeSparklineGroups([lowest, axis, value](SparklineGroup *g) {
                            if (lowest) {
                                g->minAxis = axis;
                                if (axis == SparklineGroup::Axis::Custom) g->manualMin = value;
                            } else {
                                g->maxAxis = axis;
                                if (axis == SparklineGroup::Axis::Custom) g->manualMax = value;
                            }
                        }, tr("Sparkline Axis"));
                    });
                }
            }
        });
        return menu;
    };
}

} // namespace jp
