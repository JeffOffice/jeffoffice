// The commands on a block of cells that are not typing: AutoSum, fill, the clipboard, inserting and deleting, the sizes of rows and columns. Each asks the edit session for one step.
#include "app/crunchwindow.h"

#include "app/crunchinfo.h"
#include "app/filldialogs.h"
#include "app/icons.h"
#include "sheet/cellref.h"

#include <QAction>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QMenu>
#include <QToolButton>
#include <QUndoStack>
#include <QFontMetricsF>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace jp {

using jp::office::sheet::CellRange;

void CrunchWindow::createOpsActions()
{
    // AutoSum, and the functions of its menu
    mk("edit.autosum", tr("Sum"), "sigma", crunch::autoSumKey(), [this] { autoSum(QStringLiteral("SUM")); });
    mk("edit.autosum.sum", tr("Sum"), "sigma", QKeySequence(), [this] { autoSum(QStringLiteral("SUM")); });
    mk("edit.autosum.average", tr("Average"), "", QKeySequence(), [this] { autoSum(QStringLiteral("AVERAGE")); });
    mk("edit.autosum.count", tr("How Many Numbers"), "", QKeySequence(), [this] { autoSum(QStringLiteral("COUNT")); });
    mk("edit.autosum.max", tr("Max"), "", QKeySequence(), [this] { autoSum(QStringLiteral("MAX")); });
    mk("edit.autosum.min", tr("Min"), "", QKeySequence(), [this] { autoSum(QStringLiteral("MIN")); });
    // fill
    mk("fill.down", tr("Down"), "arrow-down-to-line", QKeySequence(Qt::CTRL | Qt::Key_D), [this] { fillSelection(crunch::EditSession::FillDirection::Down); });
    mk("fill.right", tr("Right"), "arrow-right-to-line", QKeySequence(Qt::CTRL | Qt::Key_R), [this] { fillSelection(crunch::EditSession::FillDirection::Right); });
    mk("fill.up", tr("Up"), "arrow-up-to-line", QKeySequence(), [this] { fillSelection(crunch::EditSession::FillDirection::Up); });
    mk("fill.left", tr("Left"), "arrow-left-to-line", QKeySequence(), [this] { fillSelection(crunch::EditSession::FillDirection::Left); });
    mk("fill.series", tr("Series…"), "", QKeySequence(), [this] { fillSeriesDialog(); });
    mk("fill.justify", tr("Justify"), "", QKeySequence(), [this] { justifySelection(); });
    mk("fill.across", tr("Across Worksheets…"), "", QKeySequence(), [this] { fillAcrossDialog(); });
    m_opsActions << QStringLiteral("fill.justify") << QStringLiteral("fill.across");
    mk("edit.fill", tr("Fill"), "arrow-down-to-line", QKeySequence(), [this] { fillSelection(crunch::EditSession::FillDirection::Down); });
    m_opsActions << QStringLiteral("fill.down") << QStringLiteral("fill.right") << QStringLiteral("fill.up") << QStringLiteral("fill.left") << QStringLiteral("fill.series") << QStringLiteral("edit.fill");
    connect(m_view, &grid::GridView::fillRequested, this, [this](const grid::GridRange &source, const grid::GridRange &extent, bool ctrl) { fillDragged(source, extent, ctrl); });
    connect(m_view, &grid::GridView::fillHandleDoubleClicked, this, [this](const grid::GridRange &source) { fillHandleDoubleClicked(source); });
    connect(m_view, &grid::GridView::moveRequested, this, [this](const grid::GridRange &source, const grid::GridRange &target, bool copy) { moveDragged(source, target, copy); });
    m_opsActions << QStringLiteral("edit.autosum") << QStringLiteral("edit.autosum.sum") << QStringLiteral("edit.autosum.average") << QStringLiteral("edit.autosum.count")
                 << QStringLiteral("edit.autosum.max") << QStringLiteral("edit.autosum.min");
    updateEditActions();
}

// AutoSum (and Average, How Many Numbers, Max, and Min): with one cell selected the formula is put in the cell to be edited, with the range it guessed; with a block of cells the formulas
// are put in at once.
void CrunchWindow::autoSum(const QString &function)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    const grid::Selection &sel = m_view->selection();
    CellRange range;
    range.valid = true;
    range.row1 = quint32(std::max<qint64>(0, sel.range.row1));
    range.col1 = quint32(std::max<qint64>(0, sel.range.col1));
    range.row2 = quint32(std::max<qint64>(0, sel.range.row2));
    range.col2 = quint32(std::max<qint64>(0, sel.range.col2));
    const qint64 before = m_loader.changesMade();
    const crunch::EditSession::AutoSumResult r = m_session->autoSum(m_sheet, range, function);
    if (!r.outcome.ok) {
        notice(tr("Sum"), r.outcome.problem);
        return;
    }
    if (r.kind == crunch::EditSession::AutoSumResult::Kind::Edit) {
        m_view->selectCell(r.row, r.col);
        beginEdit(r.text, CellEditor::Mode::Editing, false);
        if (m_edit.active) {
            const int at = int(r.text.size()) - r.cursorBack;
            m_cellEditor->setCursorPosition(at);
            m_formulaBar->setCursorPosition(at);
        }
    } else if (r.kind == crunch::EditSession::AutoSumResult::Kind::Done && m_loader.changesMade() != before) {
        m_calculating = true;
    }
}


// Ctrl+D, Ctrl+R, and Fill's menu: the cells of the selection are filled from the first row (column) of it, or the last for up (left); a single row (column) is filled from the cell above
// it (left of it, below it, right of it). A copy, not a series.
void CrunchWindow::fillSelection(crunch::EditSession::FillDirection direction)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    using Dir = crunch::EditSession::FillDirection;
    const grid::GridRange r = m_view->selection().range;
    const bool vertical = direction == Dir::Down || direction == Dir::Up;
    const bool forward = direction == Dir::Down || direction == Dir::Right;
    CellRange source;
    source.valid = true;
    qint64 count = 0;
    if (vertical) {
        source.col1 = quint32(r.col1);
        source.col2 = quint32(r.col2);
        if (r.rows() > 1) {
            source.row1 = source.row2 = quint32(forward ? r.row1 : r.row2);
            count = r.rows() - 1;
        } else {
            const qint64 from = forward ? r.row1 - 1 : r.row1 + 1;
            if (from < 0 || from >= qint64(jp::calc::kGridRows)) return;
            source.row1 = source.row2 = quint32(from);
            count = 1;
        }
    } else {
        source.row1 = quint32(r.row1);
        source.row2 = quint32(r.row2);
        if (r.cols() > 1) {
            source.col1 = source.col2 = quint32(forward ? r.col1 : r.col2);
            count = r.cols() - 1;
        } else {
            const qint64 from = forward ? r.col1 - 1 : r.col1 + 1;
            if (from < 0 || from >= qint64(jp::calc::kGridCols)) return;
            source.col1 = source.col2 = quint32(from);
            count = 1;
        }
    }
    crunch::EditSession::FillOptions options;
    options.kind = crunch::EditSession::FillKind::Copy;
    const qint64 before = m_loader.changesMade();
    const crunch::EditSession::Outcome o = m_session->fill(m_sheet, source, direction, quint32(std::min<qint64>(count, 0x7FFFFFFF)), options);
    if (!o.ok) {
        notice(tr("Fill"), o.problem);
        return;
    }
    if (m_loader.changesMade() != before) m_calculating = true;
}

// The fill handle was dragged: the cells it reached are filled with the series the block makes (or copies, with Ctrl held; a single number with Ctrl counts up, as in the programs
// people know), and the selection is the block and what was filled.
// A double click on the fill handle fills down as far as the data beside the block goes: the column to its left if that has data under the block's last row, else the one to its right.
void CrunchWindow::fillHandleDoubleClicked(const grid::GridRange &source)
{
    if (!m_session || m_sheet < 0) return;
    constexpr qint64 kLook = 1000000;   // (what a column can hold; the walk is cut short at a million rows)
    for (const qint64 beside : {source.col1 - 1, source.col2 + 1}) {
        if (beside < 0 || beside >= jp::office::sheet::kMaxColumns) continue;
        qint64 last = source.row2;
        while (last + 1 < jp::office::sheet::kMaxRows && last - source.row2 < kLook && !m_session->valueAt(m_sheet, quint32(last + 1), quint32(beside)).isEmpty()) ++last;
        if (last == source.row2) continue;
        grid::GridRange extent = source;
        extent.row2 = last;
        fillDragged(source, extent, false);
        return;
    }
}

// The selection's border was dragged and dropped: the block moves there (what points at it follows), or with Ctrl a copy of it is pasted there. One step.
void CrunchWindow::moveDragged(const grid::GridRange &source, const grid::GridRange &target, bool copy)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || !m_session || m_sheet < 0) return;
    auto cells = [](const grid::GridRange &g) {
        CellRange r;
        r.valid = true;
        r.row1 = quint32(g.row1);
        r.col1 = quint32(g.col1);
        r.row2 = quint32(g.row2);
        r.col2 = quint32(g.col2);
        return r;
    };
    const CellRange from = cells(source), to = cells(target);
    const qint64 before = m_loader.changesMade();
    crunch::EditSession::Outcome o;
    if (copy) {
        const crunch::ClipData clip = m_session->copyCells(m_sheet, from, false);
        if (!clip.valid()) {
            notice(tr("Copy"), tr("There are too many cells to copy."));
            return;
        }
        o = m_session->paste(m_sheet, to, clip, crunch::EditSession::PasteOptions());
    } else {
        o = doStructural([&] { return m_session->moveCells(m_sheet, from, m_sheet, to.row1, to.col1); });
    }
    if (!o.ok) {
        if (!o.confirm) notice(copy ? tr("Copy") : tr("Move"), o.problem);
        return;
    }
    if (m_loader.changesMade() != before) m_calculating = true;
    widenForCells(m_sheet, to);
    m_view->selectRange(target.row1, target.col1, target.row2, target.col2);
}

void CrunchWindow::fillDragged(const grid::GridRange &source, const grid::GridRange &extent, bool ctrl)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    using Dir = crunch::EditSession::FillDirection;
    Dir direction = Dir::Down;
    qint64 count = 0;
    if (extent.row2 > source.row2) {
        direction = Dir::Down;
        count = extent.row2 - source.row2;
    } else if (extent.row1 < source.row1) {
        direction = Dir::Up;
        count = source.row1 - extent.row1;
    } else if (extent.col2 > source.col2) {
        direction = Dir::Right;
        count = extent.col2 - source.col2;
    } else if (extent.col1 < source.col1) {
        direction = Dir::Left;
        count = source.col1 - extent.col1;
    }
    if (count <= 0) return;
    CellRange from;
    from.valid = true;
    from.row1 = quint32(source.row1);
    from.col1 = quint32(source.col1);
    from.row2 = quint32(source.row2);
    from.col2 = quint32(source.col2);
    crunch::EditSession::FillOptions options;
    if (ctrl) {
        const bool single = source.rows() == 1 && source.cols() == 1;
        const bool number = single && m_session->valueAt(m_sheet, from.row1, from.col1).isNumber();
        options.kind = number ? crunch::EditSession::FillKind::Linear : crunch::EditSession::FillKind::Copy;
    }
    const qint64 before = m_loader.changesMade();
    const int stepsBefore = m_session->undoStack()->index();
    const crunch::EditSession::Outcome o = m_session->fill(m_sheet, from, direction, quint32(std::min<qint64>(count, 0x7FFFFFFF)), options);
    if (!o.ok) {
        notice(tr("Fill"), o.problem);
        return;
    }
    if (m_loader.changesMade() != before) m_calculating = true;
    m_view->selectRange(extent.row1, extent.col1, extent.row2, extent.col2);
    // the button that offers the fill another way (when the fill changed anything)
    hideFillOptions();
    if (m_session->undoStack()->index() > stepsBefore) {
        m_lastFill = LastFill();
        m_lastFill.valid = true;
        m_lastFill.sheet = m_sheet;
        m_lastFill.index = m_session->undoStack()->index();
        m_lastFill.from = from;
        m_lastFill.direction = direction;
        m_lastFill.count = quint32(std::min<qint64>(count, 0x7FFFFFFF));
        m_lastFill.kind = options.kind;
        m_lastFill.extent = extent;
        m_lastFill.single = source.rows() == 1 && source.cols() == 1;
        // (dates: the last cell of the block holds a number shown as a date)
        const quint32 lastRow = from.row2, lastCol = from.col2;
        const grid::GridSheet &gs = m_view->sheet();
        const grid::Cell *cell = gs.cellAt(lastRow, lastCol);
        m_lastFill.dates = cell && m_session->valueAt(m_sheet, lastRow, lastCol).isNumber() && gs.looks().look(gs.styleOf(lastRow, lastCol, cell)).format.kind() == jp::calc::FormatKind::Date;
        offerFillOptions();
    }
}

// The small button at the corner of what the handle filled: how the cells were filled, again another way (the fill is taken back and made again, so it stays one step).
void CrunchWindow::offerFillOptions()
{
    if (!m_lastFill.valid) return;
    if (!m_fillOptions) {
        m_fillOptions = new QToolButton(m_view->viewport());
        m_fillOptions->setObjectName(QStringLiteral("fillOptionsButton"));
        m_fillOptions->setIcon(icon(QStringLiteral("clipboard-list")));
        m_fillOptions->setToolTip(tr("Fill Options"));
        m_fillOptions->setAccessibleName(tr("Fill Options"));
        m_fillOptions->setPopupMode(QToolButton::InstantPopup);
        m_fillOptions->setAutoRaise(false);
        m_fillOptions->setFixedSize(24, 24);
        m_fillOptions->setMenu(new QMenu(m_fillOptions));
    }
    QMenu *menu = m_fillOptions->menu();
    menu->clear();
    auto add = [&](const QString &id, const QString &text) {
        QAction *a = menu->addAction(text);
        a->setObjectName(id);
        connect(a, &QAction::triggered, this, [this, id] { refill(id); });
    };
    add(QStringLiteral("fillOpt.copy"), tr("Copy Cells"));
    add(QStringLiteral("fillOpt.series"), tr("Fill Series"));
    add(QStringLiteral("fillOpt.formatsOnly"), tr("Fill Formatting Only"));
    add(QStringLiteral("fillOpt.valuesOnly"), tr("Fill Without Formatting"));
    if (m_lastFill.dates) {
        menu->addSeparator();
        add(QStringLiteral("fillOpt.days"), tr("Fill Days"));
        add(QStringLiteral("fillOpt.weekdays"), tr("Fill Weekdays"));
        add(QStringLiteral("fillOpt.months"), tr("Fill Months"));
        add(QStringLiteral("fillOpt.years"), tr("Fill Years"));
    }
    connect(m_session->undoStack(), &QUndoStack::indexChanged, this, &CrunchWindow::fillStackChanged, Qt::UniqueConnection);
    const QRect corner = m_view->cellRect(m_lastFill.extent.row2, m_lastFill.extent.col2);
    const QRect vp = m_view->viewport()->rect();
    const QPoint at(std::clamp(corner.right() + 2, 0, std::max(0, vp.width() - m_fillOptions->width())), std::clamp(corner.bottom() + 2, 0, std::max(0, vp.height() - m_fillOptions->height())));
    m_fillOptions->move(at);
    m_fillOptions->show();
    m_fillOptions->raise();
}

void CrunchWindow::hideFillOptions()
{
    m_lastFill.valid = false;
    if (m_fillOptions) m_fillOptions->hide();
}

void CrunchWindow::fillStackChanged()
{
    if (!m_fillRedoing && m_lastFill.valid && m_session && m_session->undoStack()->index() != m_lastFill.index) hideFillOptions();
}

void CrunchWindow::refill(const QString &id)
{
    using Kind = crunch::EditSession::FillKind;
    using What = crunch::EditSession::FillWhat;
    using Unit = crunch::EditSession::DateUnit;
    if (!m_lastFill.valid || !m_session || m_sheet != m_lastFill.sheet || m_session->undoStack()->index() != m_lastFill.index) {
        hideFillOptions();
        return;
    }
    crunch::EditSession::FillOptions options;
    options.kind = m_lastFill.kind;
    if (id == QLatin1String("fillOpt.copy")) {
        options.kind = Kind::Copy;
    } else if (id == QLatin1String("fillOpt.series")) {
        options.kind = m_lastFill.single && !m_lastFill.dates ? Kind::Linear : Kind::Auto;
    } else if (id == QLatin1String("fillOpt.formatsOnly")) {
        options.kind = Kind::Copy;
        options.what = What::FormatsOnly;
    } else if (id == QLatin1String("fillOpt.valuesOnly")) {
        options.what = What::ValuesOnly;
    } else {
        options.kind = Kind::Date;
        options.unit = id == QLatin1String("fillOpt.weekdays") ? Unit::Weekday : id == QLatin1String("fillOpt.months") ? Unit::Month : id == QLatin1String("fillOpt.years") ? Unit::Year : Unit::Day;
    }
    m_fillRedoing = true;
    m_session->undoStack()->undo();
    const qint64 before = m_loader.changesMade();
    const crunch::EditSession::Outcome o = m_session->fill(m_sheet, m_lastFill.from, m_lastFill.direction, m_lastFill.count, options);
    if (!o.ok) notice(tr("Fill"), o.problem);
    if (m_loader.changesMade() != before) m_calculating = true;
    m_fillRedoing = false;
    if (o.ok) m_lastFill.index = m_session->undoStack()->index();
    else hideFillOptions();
    m_view->selectRange(m_lastFill.extent.row1, m_lastFill.extent.col1, m_lastFill.extent.row2, m_lastFill.extent.col2);
}


// Fill > Series: the selection's first row (column) is the start, and the series goes on over the rest of it.
void CrunchWindow::fillSeriesDialog()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    const grid::GridRange r = m_view->selection().range;
    SeriesRequest request;
    request.inRows = r.rows() == 1 && r.cols() > 1;
    if (m_seriesAsker) {
        if (!m_seriesAsker(&request)) return;
    } else {
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Series"));
        auto *form = new QFormLayout;
        auto *layout = new QVBoxLayout(&dialog);
        layout->addLayout(form);
        auto *direction = new QComboBox;
        direction->addItems({tr("Columns"), tr("Rows")});
        direction->setCurrentIndex(request.inRows ? 1 : 0);
        form->addRow(tr("Series in"), direction);
        auto *type = new QComboBox;
        type->addItems({tr("Linear"), tr("Growth"), tr("Date"), tr("AutoFill")});
        type->setCurrentIndex(3);
        form->addRow(tr("Type"), type);
        auto *unit = new QComboBox;
        unit->addItems({tr("Day"), tr("Weekday"), tr("Month"), tr("Year")});
        form->addRow(tr("Date unit"), unit);
        auto *step = new QLineEdit(QStringLiteral("1"));
        form->addRow(tr("Step value"), step);
        auto *stop = new QLineEdit;
        form->addRow(tr("Stop value"), stop);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() != QDialog::Accepted) return;
        request.inRows = direction->currentIndex() == 1;
        request.options.kind = type->currentIndex() == 0 ? crunch::EditSession::FillKind::Linear : type->currentIndex() == 1 ? crunch::EditSession::FillKind::Growth
                               : type->currentIndex() == 2 ? crunch::EditSession::FillKind::Date : crunch::EditSession::FillKind::Auto;
        request.options.unit = crunch::EditSession::DateUnit(unit->currentIndex());
        bool ok = false;
        const double stepValue = QLocale().toDouble(step->text().trimmed(), &ok);
        request.options.hasStep = ok && !step->text().trimmed().isEmpty();
        request.options.step = stepValue;
        const double stopValue = QLocale().toDouble(stop->text().trimmed(), &ok);
        request.options.hasStop = ok && !stop->text().trimmed().isEmpty();
        request.options.stop = stopValue;
    }
    using Dir = crunch::EditSession::FillDirection;
    CellRange source;
    source.valid = true;
    qint64 count = 0;
    if (request.inRows) {
        source.row1 = quint32(r.row1);
        source.row2 = quint32(r.row2);
        source.col1 = source.col2 = quint32(r.col1);
        count = r.cols() - 1;
    } else {
        source.col1 = quint32(r.col1);
        source.col2 = quint32(r.col2);
        source.row1 = source.row2 = quint32(r.row1);
        count = r.rows() - 1;
    }
    if (count <= 0) {
        notice(tr("Fill"), tr("Select the cells to fill, with the first of them holding where the series begins."));
        return;
    }
    const qint64 before = m_loader.changesMade();
    const crunch::EditSession::Outcome o = m_session->fill(m_sheet, source, request.inRows ? Dir::Right : Dir::Down, quint32(std::min<qint64>(count, 0x7FFFFFFF)), request.options);
    if (!o.ok) {
        notice(tr("Fill"), o.problem);
        return;
    }
    if (m_loader.changesMade() != before) m_calculating = true;
}

// Fill > Justify: the text of the selected cells of a column is put into lines as wide as the column, in the cells from the first one down.
void CrunchWindow::justifySelection()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    const grid::GridRange r = m_view->selection().range;
    CellRange range;
    range.valid = true;
    range.row1 = quint32(r.row1);
    range.row2 = quint32(r.row2);
    range.col1 = quint32(r.col1);
    range.col2 = quint32(r.col2);
    // the font of the first cell, as the grid shows it, and the width of the column inside its margins
    const grid::GridSheet &gs = m_view->sheet();
    const grid::Cell *cell = gs.cellAt(range.row1, range.col1);
    const grid::CellLook &look = gs.looks().look(gs.styleOf(range.row1, range.col1, cell));
    QFont font(look.family.isEmpty() ? m_view->font().family() : look.family);
    font.setPixelSize(std::max(9, int(std::lround(look.pointSize * 96.0 / 72.0))));
    font.setBold(look.bold);
    font.setItalic(look.italic);
    const QFontMetricsF metrics(font);
    const double width = std::max(metrics.averageCharWidth(), gs.columnWidth(r.col1) - 6.0);
    const auto measure = [&metrics](const QString &text) { return metrics.horizontalAdvance(text); };
    const qint64 before = m_loader.changesMade();
    crunch::EditSession::Outcome o = m_session->justify(m_sheet, range, width, measure);
    if (!o.ok && o.confirm) {
        // (the text reaches below the cells selected)
        if (askBeforeLosing(tr("Justify"), o.problem, tr("Continue"), QString(), false) != LossAnswer::Continue) return;
        o = m_session->justify(m_sheet, range, width, measure, true);
    }
    if (!o.ok) {
        notice(tr("Justify"), o.problem);
        return;
    }
    if (m_loader.changesMade() != before) m_calculating = true;
}

// Fill > Across Worksheets: asks which sheets and what to copy, and copies the selected cells there.
void CrunchWindow::fillAcrossDialog()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || !m_loaded || !m_loaded->book) return;
    QStringList names;
    for (const auto &sheet : m_loaded->book->info().sheets) names << sheet.name;
    if (names.size() < 2) {
        notice(tr("Fill"), tr("This workbook has only one sheet, so there is no other sheet to fill."));
        return;
    }
    FillAcrossDialog dialog(names, m_sheet, this);
    if (dialog.exec() != QDialog::Accepted) return;
    fillAcrossSheets(dialog.sheets(), dialog.what());
}

bool CrunchWindow::fillAcrossSheets(const std::vector<int> &targets, crunch::EditSession::FillWhat what)
{
    if (!m_session || !editingAllowed()) return false;
    const grid::GridRange r = m_view->selection().range;
    CellRange range;
    range.valid = true;
    range.row1 = quint32(r.row1);
    range.row2 = quint32(r.row2);
    range.col1 = quint32(r.col1);
    range.col2 = quint32(r.col2);
    const qint64 before = m_loader.changesMade();
    const crunch::EditSession::Outcome o = m_session->fillAcrossSheets(m_sheet, range, targets, what);
    if (!o.ok) {
        notice(tr("Fill"), o.problem);
        return false;
    }
    if (m_loader.changesMade() != before) m_calculating = true;
    return true;
}

} // namespace jp
