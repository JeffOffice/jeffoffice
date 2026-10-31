// Tables on the window: how they are drawn (the style given to the grid), the buttons on their rows of names and of totals, Insert > Table, the Table Looks gallery, and the Table tab of the
// ribbon with the commands that change the table the cursor is in. The changes themselves are the edit session's (core/edittables.cpp).
#include "app/plaintext.h"
#include "app/crunchwindow.h"

#include "app/crunchinfo.h"
#include "core/formulaspot.h"
#include "app/icons.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "sheet/cellref.h"
#include "sheet/structref.h"
#include "sheet/tablestyle.h"

#include <QAction>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>

namespace jp {

using jp::office::sheet::CellRange;
using jp::office::sheet::DifferentialFormat;
using jp::office::sheet::Table;
using jp::office::sheet::TableStyle;

namespace {

// A picture of a table in a style: a header row, three rows of data, four columns; the colors the workbook's theme gives.
QPixmap swatchOf(const TableStyle &style, const jp::office::sheet::Styles &styles, bool stripes, bool firstColumn, qreal ratio)
{
    constexpr int cw = 15, ch = 9, cols = 4, rows = 5;
    QPixmap pm(int(cw * cols * ratio) + int(4 * ratio), int(ch * rows * ratio) + int(4 * ratio));
    pm.setDevicePixelRatio(ratio);
    pm.fill(Qt::white);
    QPainter p(&pm);
    Table t;
    t.row1 = 0;
    t.row2 = rows - 1;
    t.col1 = 0;
    t.col2 = cols - 1;
    t.headerRows = 1;
    t.showRowStripes = stripes;
    t.showFirstColumn = firstColumn;
    const QPoint origin(2, 2);
    for (quint32 r = 0; r < rows; ++r)
        for (quint32 c = 0; c < cols; ++c) {
            const DifferentialFormat d = jp::office::sheet::tableLookAt(style, t, r, c);
            const QRect cell(origin.x() + int(c) * cw, origin.y() + int(r) * ch, cw, ch);
            if (d.hasFill) {
                const jp::office::sheet::ColorSpec &spec = d.fill.bg.isSet() ? d.fill.bg : d.fill.fg;
                p.fillRect(cell, QColor::fromRgb(styles.resolve(spec, 0xFFFFFF)));
            }
            const bool dark = d.fontColor.isSet() && styles.resolve(d.fontColor, 0) == 0xFFFFFF;
            p.fillRect(QRect(cell.left() + 3, cell.top() + ch / 2 - 1, cw - 6, 2), dark ? QColor(255, 255, 255, 230) : QColor(90, 90, 90, 140));
            auto line = [&](const jp::office::sheet::BorderSide &side, QPoint a, QPoint b) {
                if (side.style == jp::office::sheet::BorderStyle::None) return;
                p.setPen(QPen(QColor::fromRgb(styles.resolve(side.color, 0x000000)), 1));
                p.drawLine(a, b);
            };
            if (d.hasBorder) {
                line(d.border.top, cell.topLeft(), cell.topRight());
                line(d.border.bottom, cell.bottomLeft(), cell.bottomRight());
                line(d.border.left, cell.topLeft(), cell.bottomLeft());
                line(d.border.right, cell.topRight(), cell.bottomRight());
            }
        }
    return pm;
}

// The gallery: the built-in looks in their groups, a button for each; a click chooses one.
class TableLooksPicker : public QWidget {
public:
    TableLooksPicker(const jp::office::sheet::Styles &styles, const jp::office::sheet::TableStyleBook &book, const QString &current, const std::function<void(const QString &)> &choose, QWidget *parent)
        : QWidget(parent)
    {
        setObjectName(QStringLiteral("tableLooksPicker"));
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(8, 6, 8, 6);
        const qreal ratio = devicePixelRatioF();
        for (const jp::office::sheet::TableStyleGroup &group : jp::office::sheet::tableStyleGroups()) {
            auto *title = new QLabel(group.title, this);
            title->setStyleSheet(QStringLiteral("font-weight: bold;"));
            layout->addWidget(title);
            auto *grid = new QGridLayout();
            grid->setSpacing(4);
            int n = 0;
            for (const QString &name : group.names) {
                const TableStyle *style = book.find(name);
                if (!style) continue;
                auto *b = new QToolButton(this);
                b->setObjectName(QStringLiteral("tableLook.") + name);
                b->setIcon(QIcon(swatchOf(*style, styles, true, false, ratio)));
                b->setIconSize(QSize(64, 49));
                b->setCheckable(true);
                b->setChecked(name.compare(current, Qt::CaseInsensitive) == 0);
                b->setAutoRaise(true);
                b->setToolTip(jp::plainToolTip(QCoreApplication::translate("jp::CrunchWindow", "Table look %1").arg(name.mid(10))));
                b->setAccessibleName(QCoreApplication::translate("jp::CrunchWindow", "Table look %1").arg(name.mid(10)));
                connect(b, &QToolButton::clicked, this, [choose, name, this] {
                    choose(name);
                    if (QWidget *menu = window()) menu->close();
                });
                grid->addWidget(b, n / 7, n % 7);
                ++n;
            }
            layout->addLayout(grid);
        }
    }
};

} // namespace

// ---------------------------------------------------------------- the table the cursor is in

int CrunchWindow::selectedTable() const
{
    if (!m_session || !m_loaded || !m_haveCalc || m_chartSheetShown) return -1;
    const grid::Selection &sel = m_view->selection();
    return m_session->tableAt(m_sheet, quint32(std::max<qint64>(0, sel.activeRow)), quint32(std::max<qint64>(0, sel.activeCol)));
}

// ---------------------------------------------------------------- how a table is drawn, and its buttons

void CrunchWindow::updateTableLooks()
{
    if (!m_loaded || !m_loaded->book) return;
    const jp::office::sheet::Styles &styles = m_loaded->book->styles();
    if (!m_tableStylesFor) {
        m_tableStyleBook.setCustom(styles.tableStyles, styles.dxfs);
        m_tableStylesFor = true;
    }
    std::vector<grid::GridSheet::TableSkin> skins;
    if (!m_chartSheetShown)
        for (const Table &t : m_loaded->book->tables()) {
            if (t.sheet != m_sheet) continue;
            const TableStyle *style = m_tableStyleBook.find(t.styleName);
            if (!style) continue;
            grid::GridSheet::TableSkin skin;
            skin.table = t;
            skin.style = style;
            skins.push_back(std::move(skin));
            if (skins.size() >= grid::GridSheet::kMaxTableSkins) break;
        }
    // (no skin before and none now, as for most sheets: nothing changes and nothing is drawn again)
    if (skins.empty() && !m_view->sheet().hasTableSkins()) {
        updateTableTab();
        return;
    }
    m_view->sheet().setTableSkins(std::move(skins));
    m_view->viewport()->update();
    updateTableTab();
}

void CrunchWindow::tableHeaderButtons(std::vector<grid::GridView::HeaderButton> *buttons)
{
    if (!m_loaded || !m_loaded->book || !m_session) return;
    for (int i = 0; i < m_loaded->book->tables().size(); ++i) {
        const Table &t = m_loaded->book->tables()[i];
        if (t.sheet != m_sheet) continue;
        const jp::office::sheet::AutoFilter f = m_session->tableFilter(i);
        if (t.filterButtons && t.headerRows > 0) {
            for (int c = 0; c < t.columns.size() && buttons->size() < grid::GridView::kMaxHeaderButtons; ++c) {
                const jp::office::sheet::FilterColumn *fc = f.columnAt(quint32(c));
                if (fc && fc->hideButton) continue;
                grid::GridView::HeaderButton b;
                b.row = t.row1;
                b.col = t.col1 + quint32(c);
                b.filtered = fc && fc->kind != jp::office::sheet::FilterColumn::Kind::None;
                buttons->push_back(b);
            }
        }
        if (t.totalsRows > 0)
            for (int c = 0; c < t.columns.size() && buttons->size() < grid::GridView::kMaxHeaderButtons; ++c) {
                grid::GridView::HeaderButton b;
                b.row = t.row2;
                b.col = t.col1 + quint32(c);
                buttons->push_back(b);
            }
    }
}

void CrunchWindow::onTablesChanged(int sheet)
{
    if (!m_loaded) return;
    // (a table's look, its buttons, and its tab are made again: the sheet's own picture, its sizes and cells, is not)
    if (sheet == m_sheet) updateFilterButtons();
}

// ---------------------------------------------------------------- the commands

void CrunchWindow::createTableActions()
{
    mk("insert.table", tr("Table"), "table", QKeySequence(Qt::CTRL | Qt::Key_T), [this] { insertTableCommand(); });
    if (QAction *a = act("insert.table")) a->setShortcuts({QKeySequence(Qt::CTRL | Qt::Key_T), QKeySequence(Qt::CTRL | Qt::Key_L)});
    mk("table.resize", tr("Change Table Size…"), "scaling", QKeySequence(), [this] { tableDialog(0); });
    mk("table.toRange", tr("Back to Plain Cells"), "table-2", QKeySequence(), [this] {
        const int t = selectedTable();
        if (t < 0 || !editingAllowed()) return;
        if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
        const auto o = m_session->tableToRange(t);
        if (!o.ok) notice(tr("Table"), o.problem);
    });
    auto mark = [this](const QString &id, const QString &text, const std::function<void(crunch::EditSession::TableMarks *, bool)> &set) {
        mk(id, text, "square-check", QKeySequence(), [this, id, set] {
            const int t = selectedTable();
            if (t < 0 || !editingAllowed()) return;
            if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
            const Table &table = m_session->workbook()->tables()[t];
            crunch::EditSession::TableMarks marks;
            marks.style = table.styleName;
            marks.firstColumn = table.showFirstColumn;
            marks.lastColumn = table.showLastColumn;
            marks.rowStripes = table.showRowStripes;
            marks.columnStripes = table.showColumnStripes;
            marks.filterButtons = table.filterButtons;
            set(&marks, act(id)->isChecked());
            const auto o = m_session->setTableMarks(t, marks);
            if (!o.ok) {
                notice(tr("Table"), o.problem);
                updateTableTab();
            }
        }, true);
    };
    mark(QStringLiteral("table.stripeRows"), tr("Striped Rows"), [](crunch::EditSession::TableMarks *m, bool on) { m->rowStripes = on; });
    mark(QStringLiteral("table.stripeCols"), tr("Striped Columns"), [](crunch::EditSession::TableMarks *m, bool on) { m->columnStripes = on; });
    mark(QStringLiteral("table.firstCol"), tr("First Column"), [](crunch::EditSession::TableMarks *m, bool on) { m->firstColumn = on; });
    mark(QStringLiteral("table.lastCol"), tr("Last Column"), [](crunch::EditSession::TableMarks *m, bool on) { m->lastColumn = on; });
    mark(QStringLiteral("table.buttons"), tr("Filter Buttons"), [](crunch::EditSession::TableMarks *m, bool on) { m->filterButtons = on; });
    mk("table.header", tr("Row of Names"), "square-check", QKeySequence(), [this] {
        const int t = selectedTable();
        if (t < 0 || !editingAllowed()) return;
        if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
        const auto o = m_session->setTableHeaderRow(t, act("table.header")->isChecked());
        if (!o.ok) {
            notice(tr("Table"), o.problem);
            updateTableTab();
        }
    }, true);
    mk("table.totals", tr("Totals Row"), "square-check", QKeySequence(), [this] {
        const int t = selectedTable();
        if (t < 0 || !editingAllowed()) return;
        if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
        const auto o = m_session->setTableTotalsRow(t, act("table.totals")->isChecked());
        if (!o.ok) {
            notice(tr("Table"), o.problem);
            updateTableTab();
        }
    }, true);
    for (const char *id : {"insert.table", "table.resize", "table.toRange", "table.stripeRows", "table.stripeCols", "table.firstCol", "table.lastCol", "table.buttons", "table.header", "table.totals"})
        m_opsActions << QString::fromLatin1(id);
}

void CrunchWindow::updateTableTab()
{
    const int t = selectedTable();
    if (m_ribbon) m_ribbon->setContextVisible(QStringLiteral("Table"), t >= 0);
    if (t < 0 || !m_session) return;
    const Table &table = m_session->workbook()->tables()[t];
    auto set = [this](const char *id, bool on) {
        if (QAction *a = act(QString::fromLatin1(id))) {
            const QSignalBlocker block(a);
            a->setChecked(on);
        }
    };
    set("table.stripeRows", table.showRowStripes);
    set("table.stripeCols", table.showColumnStripes);
    set("table.firstCol", table.showFirstColumn);
    set("table.lastCol", table.showLastColumn);
    set("table.buttons", table.filterButtons);
    set("table.header", table.headerRows > 0);
    set("table.totals", table.totalsRows > 0);
    for (QLineEdit *box : findChildren<QLineEdit *>(QStringLiteral("table.name"))) {
        const QSignalBlocker block(box);
        if (!box->hasFocus()) box->setText(table.name);
    }
}

// Insert > Table: the cells selected (the block round the cell, if one is), and whether its first row holds the names of the columns.
void CrunchWindow::insertTableCommand()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    if (selectedTable() >= 0) {
        notice(tr("Table"), tr("These cells are in a table already."));
        return;
    }
    CellRange range = selectedRange();
    const grid::Selection &sel = m_view->selection();
    if (range.row1 == range.row2 && range.col1 == range.col2 && sel.whole == grid::Selection::Whole::None) {
        bool truncated = false;
        range = crunch::currentRegion(*m_session, m_sheet, range.row1, range.col1, &truncated);
        if (truncated) {
            notice(tr("Table"), tr("The block of data around this cell is too big to find its edges quickly. Select the cells to make a table of."));
            return;
        }
    } else if (sel.whole != grid::Selection::Whole::None) {
        const jp::office::sheet::Sheet &s = m_session->calculation()->sheet(m_sheet);
        if (s.extentRows == 0) {
            notice(tr("Table"), tr("There is nothing here to make a table of."));
            return;
        }
        range.row2 = std::min(range.row2, s.extentRows - 1);
        range.col2 = std::min(range.col2, s.extentCols - 1);
    }
    bool headers = m_session->looksLikeHeader(m_sheet, range);
    QString where = jp::office::sheet::rangeName(range);
    if (m_tableMakeAsker) {
        if (!m_tableMakeAsker(&where, &headers)) return;
    } else {
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Make a Table"));
        auto *form = new QFormLayout(&dialog);
        auto *cells = new QLineEdit(where, &dialog);
        cells->setAccessibleName(tr("Cells for the table"));
        cells->setObjectName(QStringLiteral("table.cells"));
        auto *names = new QCheckBox(tr("The first row holds the names of the columns"), &dialog);
        names->setChecked(headers);
        names->setObjectName(QStringLiteral("table.hasNames"));
        form->addRow(tr("Cells:"), cells);
        form->addRow(names);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        form->addRow(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() != QDialog::Accepted) return;
        where = cells->text();
        headers = names->isChecked();
    }
    CellRange chosen;
    if (!jp::office::sheet::parseRangeRef(where.trimmed().remove(QLatin1Char('$')), &chosen)) {
        notice(tr("Table"), tr("Those are not cells. Write them as A1:D20."));
        return;
    }
    int made = -1;
    const auto o = m_session->makeTable(m_sheet, chosen, headers, QString(), &made);
    if (!o.ok) notice(tr("Table"), o.problem);
}

void CrunchWindow::tableGalleryChosen(const QString &style)
{
    const int t = selectedTable();
    if (t >= 0) {
        if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
        if (!editingAllowed()) return;
        const Table &table = m_session->workbook()->tables()[t];
        crunch::EditSession::TableMarks marks;
        marks.style = style;
        marks.firstColumn = table.showFirstColumn;
        marks.lastColumn = table.showLastColumn;
        marks.rowStripes = table.showRowStripes;
        marks.columnStripes = table.showColumnStripes;
        marks.filterButtons = table.filterButtons;
        const auto o = m_session->setTableMarks(t, marks);
        if (!o.ok) notice(tr("Table"), o.problem);
        return;
    }
    // cells that are not a table yet become one, in that look
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    CellRange range = selectedRange();
    const grid::Selection &sel = m_view->selection();
    if (range.row1 == range.row2 && range.col1 == range.col2 && sel.whole == grid::Selection::Whole::None) {
        bool truncated = false;
        range = crunch::currentRegion(*m_session, m_sheet, range.row1, range.col1, &truncated);
        if (truncated || range.cellCount() <= 1) {
            notice(tr("Table"), tr("Select the cells to make a table of."));
            return;
        }
    }
    const bool headers = m_session->looksLikeHeader(m_sheet, range);
    const auto o = m_session->makeTable(m_sheet, range, headers, style, nullptr);
    if (!o.ok) notice(tr("Table"), o.problem);
}

// mode 0: the size of the table.
void CrunchWindow::tableDialog(int mode)
{
    Q_UNUSED(mode);
    const int t = selectedTable();
    if (t < 0 || !editingAllowed()) return;
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    const Table table = m_session->workbook()->tables()[t];
    CellRange now;
    now.valid = true;
    now.row1 = table.row1;
    now.col1 = table.col1;
    now.row2 = table.row2;
    now.col2 = table.col2;
    CellRange chosen = now;
    if (m_tableSizeAsker) {
        if (!m_tableSizeAsker(&chosen)) return;
    } else {
        bool ok = false;
        const QString text = QInputDialog::getText(this, tr("Change Table Size"), tr("The cells the table is to cover (it keeps its first cell):"), QLineEdit::Normal, jp::office::sheet::rangeName(now), &ok);
        if (!ok) return;
        if (!jp::office::sheet::parseRangeRef(text.trimmed().remove(QLatin1Char('$')), &chosen)) {
            notice(tr("Table"), tr("Those are not cells. Write them as A1:D20."));
            return;
        }
    }
    const auto o = m_session->resizeTable(t, chosen);
    if (!o.ok) notice(tr("Table"), o.problem);
}

// While a formula is typed, a click on a cell of a table is a reference to it by the names the table gives it, as the programs people know write them: a cell of the formula's own row is
// Table[@Column] (or [@Column] inside the table), a cell of the row of names or of totals Table[[#Headers],[Column]] or Table[[#Totals],[Column]]; a cell of another row is its address.
bool CrunchWindow::pointAtTableCell(qint64 row, qint64 col)
{
    if (!m_edit.active || !m_session || !m_loaded || row < 0 || col < 0) return false;
    QLineEdit *line = m_formulaBar && m_formulaBar->hasFocus() ? static_cast<QLineEdit *>(m_formulaBar) : static_cast<QLineEdit *>(m_cellEditor);
    if (!line || !line->text().startsWith(QLatin1Char('='))) return false;
    const int index = m_session->tableAt(m_sheet, quint32(row), quint32(col));
    if (index < 0) return false;
    const crunch::FormulaSpot spot = crunch::spotAt(line->text(), line->cursorPosition(), crunch::argumentSeparator(m_session->locale()));
    if (!spot.canPoint) return false;
    const Table &t = m_session->workbook()->tables()[index];
    const int column = int(quint32(col) - t.col1);
    if (column < 0 || column >= t.columns.size()) return false;
    // the formula's own cell is where the entry is made: the short form is for a cell of the table's sheet, in the clicked row
    const int formulaSheet = m_edit.sheet;
    const quint32 formulaRow = m_edit.row, formulaCol = m_edit.col;
    const bool sameSheet = formulaSheet == t.sheet;
    const bool inside = sameSheet && m_session->tableAt(formulaSheet, formulaRow, formulaCol) == index;
    QString text;
    if (t.headerRows > 0 && quint32(row) == t.row1) text = jp::office::sheet::structuredSpecialRef(t.name, QStringLiteral("#Headers"), t.columns[column]);
    else if (t.totalsRows > 0 && quint32(row) == t.row2) text = jp::office::sheet::structuredSpecialRef(t.name, QStringLiteral("#Totals"), t.columns[column]);
    else if (sameSheet && formulaRow == quint32(row)) text = (inside ? QString() : t.name) + jp::office::sheet::structuredRowRef(t.name, t.columns[column], true);
    else text = (sameSheet ? QString() : jp::office::sheet::sheetNamePrefix(m_loaded->book->info().sheets[t.sheet].name)) + jp::office::sheet::cellName(quint32(row), quint32(col));
    line->insert(text);
    Q_EMIT line->textEdited(line->text());
    return true;
}

// The drop-down on a cell of the row of totals: what the column's total shows.
bool CrunchWindow::showTotalsMenu(qint64 row, qint64 col)
{
    if (!m_session || !m_loaded) return false;
    const int t = m_session->tableAt(m_sheet, quint32(row), quint32(col));
    if (t < 0) return false;
    const Table &table = m_session->workbook()->tables()[t];
    if (table.totalsRows == 0 || quint32(row) != table.row2) return false;
    const int column = int(quint32(col) - table.col1);
    if (column < 0 || column >= table.info.size()) return false;
    const QString current = table.info[column].totalsFunction;
    struct Choice {
        const char *word;
        QString text;
    };
    const Choice choices[] = {{"", tr("None")},          {"average", tr("Average")}, {"count", tr("Count")},       {"countNums", tr("How Many Numbers")}, {"max", tr("Largest")},
                              {"min", tr("Smallest")},   {"sum", tr("Sum")},         {"stdDev", tr("Standard Deviation")}, {"var", tr("Variance")},   {"custom", tr("Your Own Formula…")}};
    auto *menu = new QMenu(this);
    menu->setObjectName(QStringLiteral("totalsMenu"));
    menu->setAttribute(Qt::WA_DeleteOnClose);
    for (const Choice &c : choices) {
        QAction *a = menu->addAction(c.text);
        a->setCheckable(true);
        a->setChecked(current == QLatin1String(c.word));
        const QString word = QString::fromLatin1(c.word);
        connect(a, &QAction::triggered, this, [this, t, column, word] {
            if (!editingAllowed()) return;
            QString custom;
            if (word == QLatin1String("custom")) {
                bool ok = false;
                custom = QInputDialog::getText(this, tr("Total"), tr("The formula for this total:"), QLineEdit::Normal, QString(), &ok);
                if (!ok) return;
            }
            const auto o = m_session->setTableTotal(t, column, word, custom);
            if (!o.ok) notice(tr("Table"), o.problem);
        });
    }
    const QRect button = m_view->headerButtonRect(row, col);
    const QPoint at = m_view->viewport()->mapToGlobal(QPoint(button.isEmpty() ? m_view->cellRect(row, col).left() : button.left(), (button.isEmpty() ? m_view->cellRect(row, col).bottom() : button.bottom()) + 1));
    menu->popup(at);
    return true;
}

QWidget *CrunchWindow::makeTableLooksPicker(QWidget *parent)
{
    if (!m_loaded || !m_loaded->book) return new QWidget(parent);
    updateTableLooks();
    const int t = selectedTable();
    const QString current = t >= 0 ? m_session->workbook()->tables()[t].styleName : QString();
    return new TableLooksPicker(m_loaded->book->styles(), m_tableStyleBook, current, [this](const QString &style) { tableGalleryChosen(style); }, parent);
}

// The table's commands in the menu of the cells (right-click), when the cursor is in a table: rows and columns put in beside the selection (whole rows and columns of the sheet, as Insert makes them:
// a table that one is put beside grows by it) or taken out of the table, the row of totals, and back to cells.
void CrunchWindow::addTableMenu(QMenu *menu)
{
    const int ti = selectedTable();
    if (ti < 0 || !m_session) return;
    const Table t = m_session->workbook()->tables()[ti];
    const CellRange sel = selectedRange();
    const int sheet = m_sheet;
    // the rows of the selection that are the table's data, and its columns
    const quint32 dataFirst = t.dataFirst(), dataLast = quint32(std::max<qint64>(t.dataLast(), qint64(dataFirst)));
    const quint32 rowA = std::clamp(sel.row1, dataFirst, dataLast), rowB = std::clamp(sel.row2, dataFirst, dataLast);
    const quint32 colA = std::clamp(sel.col1, t.col1, t.col2), colB = std::clamp(sel.col2, t.col1, t.col2);
    const quint32 rowCount = rowB - rowA + 1, colCount = colB - colA + 1;
    auto run = [this, ti, sheet](const std::function<crunch::EditSession::Outcome()> &fn, const QString &title) {
        if (m_edit.active) commitEdit(CellEditor::Way::Stay);
        if (m_edit.active || !editingAllowed()) return;
        const auto o = fn();
        if (!o.ok) notice(title, o.problem);
        Q_UNUSED(ti);
        Q_UNUSED(sheet);
    };
    // a row or column put in just past the table is the table's: the table is made to take it
    auto grow = [this, ti](int sheetIndex, bool rows, quint32 count) {
        const Table &now = m_session->workbook()->tables()[ti];
        CellRange to;
        to.valid = true;
        to.row1 = now.row1;
        to.col1 = now.col1;
        to.row2 = now.row2 + (rows ? count : 0);
        to.col2 = now.col2 + (rows ? 0 : count);
        Q_UNUSED(sheetIndex);
        return m_session->resizeTable(ti, to);
    };
    QMenu *sub = menu->addMenu(icon(QStringLiteral("table")), tr("Table"));
    sub->setObjectName(QStringLiteral("tableMenu"));
    sub->addAction(tr("Insert Row Above"), this, [=] { run([=] { return m_session->insertRows(sheet, rowA, rowCount); }, tr("Table")); });
    sub->addAction(tr("Insert Row Below"), this, [=] {
        run([=] {
            const quint32 before = m_session->workbook()->tables()[ti].row2;
            auto o = m_session->insertRows(sheet, rowB + 1, rowCount);
            if (o.ok && m_session->workbook()->tables()[ti].row2 == before) o = grow(sheet, true, rowCount);
            return o;
        }, tr("Table"));
    });
    sub->addAction(tr("Insert Column Left"), this, [=] { run([=] { return m_session->insertColumns(sheet, colA, colCount); }, tr("Table")); });
    sub->addAction(tr("Insert Column Right"), this, [=] {
        run([=] {
            const quint32 before = m_session->workbook()->tables()[ti].col2;
            auto o = m_session->insertColumns(sheet, colB + 1, colCount);
            if (o.ok && m_session->workbook()->tables()[ti].col2 == before) o = grow(sheet, false, colCount);
            return o;
        }, tr("Table"));
    });
    sub->addSeparator();
    sub->addAction(tr("Delete Table Rows"), this, [=] { run([=] { return m_session->deleteRows(sheet, rowA, rowCount); }, tr("Table")); });
    sub->addAction(tr("Delete Table Columns"), this, [=] { run([=] { return m_session->deleteColumns(sheet, colA, colCount); }, tr("Table")); });
    sub->addSeparator();
    if (QAction *a = act(QStringLiteral("table.totals"))) sub->addAction(a);
    if (QAction *a = act(QStringLiteral("table.toRange"))) sub->addAction(a);
}

// ---------------------------------------------------------------- the ribbon's parts

void CrunchWindow::tableRibbonParts(RibbonParts &parts)
{
    parts.colors[QStringLiteral("tableDesign")] = QColor(0x2F, 0x5D, 0x9E);
    parts.widgets[QStringLiteral("tableName")] = [this]() -> QWidget * {
        auto *box = new QLineEdit();
        box->setObjectName(QStringLiteral("table.name"));
        box->setAccessibleName(tr("Table Name"));
        box->setMaximumWidth(140);
        connect(box, &QLineEdit::editingFinished, this, [this, box] {
            const int t = selectedTable();
            if (t < 0 || !m_session || !box->isModified()) return;
            box->setModified(false);
            const auto o = m_session->renameTable(t, box->text());
            if (!o.ok) {
                notice(tr("Table Name"), o.problem);
                updateTableTab();
            }
        });
        return box;
    };
    parts.menus[QStringLiteral("tableLooks")] = [this](QWidget *owner) -> QMenu * {
        auto *menu = new QMenu(owner);
        menu->setObjectName(QStringLiteral("tableLooksMenu"));
        connect(menu, &QMenu::aboutToShow, this, [this, menu] {
            menu->clear();
            if (!m_loaded || !m_loaded->book) return;
            updateTableLooks();
            const int t = selectedTable();
            const QString current = t >= 0 ? m_session->workbook()->tables()[t].styleName : QString();
            Q_UNUSED(current);
            auto *wa = new QWidgetAction(menu);
            wa->setDefaultWidget(makeTableLooksPicker(menu));
            menu->addAction(wa);
        });
        return menu;
    };
}

} // namespace jp
