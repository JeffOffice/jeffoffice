// The commands on a block of cells that are not typing: AutoSum, fill, the clipboard, inserting and deleting, the sizes of rows and columns. Each asks the edit session for one step.
#include "app/crunchwindow.h"

#include "app/crunchinfo.h"
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
#include <QVBoxLayout>

namespace jp {

using jp::office::sheet::CellRange;

void CrunchWindow::createOpsActions()
{
    // AutoSum, and the functions of its menu
    mk("edit.autosum", tr("AutoSum"), "sigma", crunch::autoSumKey(), [this] { autoSum(QStringLiteral("SUM")); });
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
    mk("edit.fill", tr("Fill"), "arrow-down-to-line", QKeySequence(), [this] { fillSelection(crunch::EditSession::FillDirection::Down); });
    m_opsActions << QStringLiteral("fill.down") << QStringLiteral("fill.right") << QStringLiteral("fill.up") << QStringLiteral("fill.left") << QStringLiteral("fill.series") << QStringLiteral("edit.fill");
    connect(m_view, &grid::GridView::fillRequested, this, [this](const grid::GridRange &source, const grid::GridRange &extent, bool ctrl) { fillDragged(source, extent, ctrl); });
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
        notice(tr("AutoSum"), r.outcome.problem);
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
    const crunch::EditSession::Outcome o = m_session->fill(m_sheet, from, direction, quint32(std::min<qint64>(count, 0x7FFFFFFF)), options);
    if (!o.ok) {
        notice(tr("Fill"), o.problem);
        return;
    }
    if (m_loader.changesMade() != before) m_calculating = true;
    m_view->selectRange(extent.row1, extent.col1, extent.row2, extent.col2);
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

} // namespace jp
