// Tools of the Review, Data, and Formulas tabs that are not part of an older group: Workbook Statistics, Goal Seek, Subtotal, and Advanced Filter.
#include "app/crunchwindow.h"
#include "app/spellingdialog.h"
#include "app/translate.h"
#include "core/richadjust.h"

#include "app/advancedfilterdialog.h"
#include "app/consolidatedialog.h"
#include "app/errorcheckdialog.h"
#include "app/evaluateformuladialog.h"
#include "app/goalseekdialog.h"
#include "app/plaintext.h"
#include "app/proofingdialogs.h"
#include "app/subtotaldialog.h"
#include "app/validationdialog.h"
#include "app/watchwindow.h"
#include "core/accesscheck.h"
#include "core/advancedfilter.h"
#include "core/consolidate.h"
#include "core/gotospecial.h"
#include "core/goalseek.h"
#include "core/loader.h"
#include "core/sheetobjects.h"
#include "core/spellwalk.h"
#include "core/subtotal.h"
#include "core/validationedit.h"
#include "core/workbookstats.h"
#include "sheet/calculation.h"
#include "sheet/cellref.h"
#include "text/dictionaries.h"

#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>
#include <QElapsedTimer>
#include <QUndoStack>

#include <algorithm>
#include <cmath>
#include <optional>

namespace jp {

void CrunchWindow::createTabs2Actions()
{
    mk("review.statistics", tr("Workbook Statistics"), "chart-no-axes-column", QKeySequence(), [this] { workbookStatisticsDialog(); });
    mk("data.goalSeek", tr("Goal Seek"), "target", QKeySequence(), [this] { goalSeekDialog(); });
    mk("data.scenarioManager", tr("Scenario Manager…"), "clipboard-list", QKeySequence(), [this] { scenarioManagerDialog(); });
    mk("data.advancedFilter", tr("Advanced"), "list-filter", QKeySequence(), [this] { advancedFilterDialog(); });
    mk("formulas.errorChecking", tr("Error Checking…"), "triangle-alert", QKeySequence(), [this] { errorCheckingDialog(); });
    mk("formulas.watchWindow", tr("Watch Window"), "eye", QKeySequence(), [this] { showWatchWindow(); });
    mk("formulas.evaluate", tr("Evaluate Formula…"), "list-checks", QKeySequence(), [this] { evaluateFormulaDialog(); });
    mk("data.validation", tr("Data Validation"), "list-checks", QKeySequence(), [this] { dataValidationDialog(); });
    mk("data.circleInvalid", tr("Circle Invalid Data"), "circle-alert", QKeySequence(), [this] {
        const int n = circleInvalidData();
        if (n == 0) notice(tr("Circle Invalid Data"), tr("Every cell with data follows its rule."), false);
        else setStatus(tr("%1 cells with data that a rule does not allow are circled.").arg(n));
    });
    mk("data.clearCircles", tr("Clear Validation Circles"), "circle-slash", QKeySequence(), [this] { clearValidationCircles(); });
    act("data.clearCircles")->setEnabled(false);   // (on while some cells are circled)
    mk("review.spelling", tr("Spelling"), "spell-check", QKeySequence(Qt::Key_F7), [this] { spelling(); });
    mk("review.thesaurus", tr("Thesaurus"), "book-a", QKeySequence(Qt::SHIFT | Qt::Key_F7), [this] { thesaurus(); });
    mk("review.accessibility", tr("Check Accessibility"), "accessibility", QKeySequence(), [this] { showAccessibilityPane(); });
    mk("review.translate", tr("Translate"), "languages", QKeySequence(), [this] {
        jp::translateText(this, [this](const QString &message, int) { setStatus(message); }, textToTranslate(), false, m_urlOpener);
    });
    mk("review.language", tr("Set Proofing Language…"), "languages", QKeySequence(), [this] {
        if (!m_session || m_sheet < 0) return;
        const FormatView v = viewOfActiveCell();
        QString tag;
        bool noProof = false;
        if (!jp::languageDialog(this, v.language.isEmpty() ? crunch::proofingLanguage() : dict::match(v.language), v.noProof, &tag, &noProof)) return;
        jp::office::sheet::FormatChange change;
        change.language = tag;
        change.noProof = noProof;
        formatSelection(change, tr("Language"));
    });
    mk("review.noProof", tr("Do Not Check Spelling"), "spell-check-2", QKeySequence(), [this] {
        jp::office::sheet::FormatChange change;
        change.noProof = act(QStringLiteral("review.noProof"))->isChecked();
        formatSelection(change, tr("Do Not Check Spelling"));
    }, true);
    mk("data.consolidate", tr("Consolidate"), "combine", QKeySequence(), [this] { consolidateDialog(); });
    mk("data.subtotal", tr("Subtotal"), "sigma", QKeySequence(), [this] { subtotalDialog(); });
    for (const char *id : {"review.statistics", "data.goalSeek", "data.scenarioManager", "data.subtotal", "data.advancedFilter", "formulas.errorChecking", "formulas.watchWindow", "formulas.evaluate", "data.validation", "data.consolidate", "data.circleInvalid", "review.accessibility", "review.thesaurus", "review.spelling", "review.translate", "review.language", "review.noProof"}) m_opsActions << QString::fromLatin1(id);
    connect(this, &CrunchWindow::calculationFinished, this, [this] {
        refreshInvalidCircles();
        refreshCellFlags(true);
    });
    updateEditActions();
}

QVector<QPair<QString, QString>> CrunchWindow::statisticsRows() const
{
    QVector<QPair<QString, QString>> rows;
    if (!m_session || !m_loaded || !m_loaded->book) return rows;
    const crunch::WorkbookStats s = crunch::workbookStatistics(*m_session);
    int charts = 0;
    if (m_objects) {
        const auto &sheets = m_loaded->book->info().sheets;
        for (int i = 0; i < int(sheets.size()); ++i)
            if (!sheets[size_t(i)].deleted) charts += m_objects->counts(i).charts;
    }
    const QLocale locale;
    rows.push_back({tr("Sheets"), locale.toString(s.sheets)});
    rows.push_back({tr("Cells with data"), locale.toString(s.cells)});
    rows.push_back({tr("Formulas"), locale.toString(s.formulas)});
    rows.push_back({tr("Charts"), locale.toString(charts)});
    rows.push_back({tr("Pivot tables"), locale.toString(s.pivotTables)});
    return rows;
}

void CrunchWindow::workbookStatisticsDialog()
{
    if (!m_session) return;
    QString text;
    for (const auto &row : statisticsRows()) text += row.first + QStringLiteral(": ") + row.second + QLatin1Char('\n');
    notice(tr("Workbook Statistics"), text.trimmed(), false);
}

// Ends a step made of several changes. The undo stack says "the index changed" only now, and the window would then take the changes for new work for the worker (and wait for a batch that
// has already come, if the caller waited for the formulas). So the changes are noted as seen first; a caller that leaves work for the worker says so itself (m_calculating = true).
void CrunchWindow::endEditStep(QUndoStack *stack)
{
    m_changesNoted = m_loader.changesMade();
    stack->endMacro();
}

bool CrunchWindow::settleCalculation(int milliseconds)
{
    QElapsedTimer waited;
    waited.start();
    for (;;) {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 2);
        if (!m_calculating) {
            const auto result = m_loader.result();
            if (!result) return true;
            if (result->calcIdle.load()) {
                if (m_manualNow) return true;
                const auto hold = m_loader.hold();
                if (!hold || !hold->calculation()->isDirty()) break;
            }
        }
        if (waited.elapsed() > milliseconds) return false;
    }
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    return true;
}

void CrunchWindow::goalSeekDialog()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || !m_session) return;
    const grid::Selection &sel = m_view->selection();
    const QString here = jp::office::sheet::cellName(quint32(std::max<qint64>(0, sel.activeRow)), quint32(std::max<qint64>(0, sel.activeCol)));
    GoalSeekDialog dialog(here, this);
    if (dialog.exec() != QDialog::Accepted) return;
    goalSeekIn(dialog.setCell(), dialog.toValue(), dialog.changeCell());
}

bool CrunchWindow::goalSeekIn(const QString &setText, const QString &valueText, const QString &changeText)
{
    if (!m_session || !editingAllowed()) return false;
    const QString title = tr("Goal Seek");
    if (m_saving || !m_saveAfterCalculation.isNull() || !m_saveAfterTrial.isNull()) {
        setStatus(tr("Goal Seek can start when the workbook is saved."));
        return false;
    }
    quint32 setRow = 0, setCol = 0, changeRow = 0, changeCol = 0;
    if (!jp::office::sheet::parseCellRef(setText, &setRow, &setCol)) {
        notice(title, tr("Write the cell with the formula as a cell name, such as B3."));
        return false;
    }
    if (!jp::office::sheet::parseCellRef(changeText, &changeRow, &changeCol)) {
        notice(title, tr("Write the cell to change as a cell name, such as B1."));
        return false;
    }
    bool numeric = false;
    double target = QLocale().toDouble(valueText, &numeric);
    if (!numeric) target = QLocale::c().toDouble(valueText, &numeric);
    if (!numeric || !std::isfinite(target)) {
        notice(title, tr("Write the value you want as a number."));
        return false;
    }
    if (setRow == changeRow && setCol == changeCol) {
        notice(title, tr("The cell with the formula and the cell to change must be two cells."));
        return false;
    }
    if (!m_session->editText(m_sheet, setRow, setCol).startsWith(QLatin1Char('='))) {
        notice(title, tr("The cell with the formula, %1, holds no formula.").arg(setText.toUpper()));
        return false;
    }
    const QString changeHeld = m_session->editText(m_sheet, changeRow, changeCol);
    const jp::calc::Value changeValue = m_session->valueAt(m_sheet, changeRow, changeCol);
    if (changeHeld.startsWith(QLatin1Char('=')) || (!changeValue.isEmpty() && !changeValue.isNumber())) {
        notice(title, tr("The cell to change, %1, must hold a number, not a formula or text.").arg(changeText.toUpper()));
        return false;
    }
    QString why;
    if (!m_session->canEditCell(m_sheet, changeRow, changeCol, &why)) {
        notice(title, why);
        return false;
    }
    QLocale plain;
    plain.setNumberOptions(QLocale::OmitGroupSeparator);
    QUndoStack *stack = m_session->undoStack();
    bool failedToType = false;
    struct Trial {
        CrunchWindow *w;
        explicit Trial(CrunchWindow *window) : w(window) { ++w->m_trialMacros; }
        ~Trial()
        {
            if (--w->m_trialMacros == 0) w->resumeSaveAfterTrial();
        }
    } trialOpen(this);
    stack->beginMacro(title);
    auto put = [&](double x) {
        const qint64 before = m_loader.changesMade();
        const crunch::EditSession::Outcome o = m_session->type(m_sheet, changeRow, changeCol, plain.toString(x, 'g', 17), false);
        if (!o.ok) {
            failedToType = true;
            return false;
        }
        if (m_loader.changesMade() != before) {
            m_calculating = true;
            if (m_manualNow) m_loader.recalculate();
        }
        return settleCalculation();
    };
    auto trial = [&](double x) -> std::optional<double> {
        if (!put(x)) return std::nullopt;
        const jp::calc::Value v = m_session->valueAt(m_sheet, setRow, setCol);
        if (!v.isNumber()) return std::nullopt;
        return v.number();
    };
    const crunch::GoalSeekResult found = crunch::goalSeek(trial, changeValue.isNumber() ? changeValue.number() : 0.0, target);
    if (found.found) put(found.input);
    endEditStep(stack);
    if (failedToType) {
        stack->undo();
        notice(title, tr("The cell to change could not be changed."));
        return false;
    }
    if (!found.found) {
        stack->undo();
        notice(title, tr("Goal Seek could not make %1 come to %2 by changing %3. Try another number in %3 to start from, or check that %1 depends on %3.").arg(setText.toUpper(), QLocale().toString(target), changeText.toUpper()));
        return false;
    }
    setStatus(tr("Goal Seek found a solution: %1 is %2.").arg(changeText.toUpper(), QLocale().toString(found.input, 'g', 10)));
    return true;
}

jp::office::sheet::CellRange CrunchWindow::listRegion(const QString &title)
{
    jp::office::sheet::CellRange range = selectedRange();
    if (range.row1 == range.row2 && range.col1 == range.col2) {
        bool truncated = false;
        const jp::office::sheet::CellRange region = crunch::currentRegion(*m_session, m_sheet, range.row1, range.col1, &truncated);
        if (truncated || region.cellCount() <= 1) {
            notice(title, tr("Select the cells of the list, or a cell in it."));
            return jp::office::sheet::CellRange();
        }
        range = region;
    }
    return range;
}

void CrunchWindow::subtotalDialog()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || !m_session) return;
    const jp::office::sheet::CellRange range = listRegion(tr("Subtotal"));
    if (!range.valid) return;
    QStringList names;
    for (quint32 col = range.col1; col <= range.col2; ++col) {
        const jp::calc::Value top = m_session->valueAt(m_sheet, range.row1, col);
        names << (top.isText() && !top.text().isEmpty() ? top.text() : tr("Column %1").arg(jp::office::sheet::columnName(col)));
    }
    SubtotalDialog dialog(names, this);
    if (dialog.exec() != QDialog::Accepted) return;
    if (dialog.removeAll()) {
        removeSubtotalsIn();
        return;
    }
    std::vector<int> columns;
    for (const int c : dialog.totalColumns()) columns.push_back(c);
    subtotalIn(dialog.changeColumn(), dialog.function(), columns, dialog.replace(), dialog.pageBreaks());
}

bool CrunchWindow::subtotalIn(int changeColumn, int function, const std::vector<int> &columns, bool replace, bool pageBreaks)
{
    if (!m_session || !editingAllowed()) return false;
    const jp::office::sheet::CellRange range = listRegion(tr("Subtotal"));
    if (!range.valid) return false;
    crunch::SubtotalSpec spec;
    spec.column = range.col1 + quint32(std::max(0, changeColumn));
    spec.function = function;
    spec.replace = replace;
    spec.pageBreaks = pageBreaks;
    for (const int c : columns) spec.columns.push_back(range.col1 + quint32(std::max(0, c)));
    const qint64 before = m_loader.changesMade();
    int groups = 0;
    const crunch::EditSession::Outcome o = crunch::applySubtotals(*m_session, m_sheet, range, spec, &groups, [this] { m_changesNoted = m_loader.changesMade(); });
    if (m_loader.changesMade() != before) m_calculating = true;
    if (!o.ok) {
        notice(tr("Subtotal"), o.problem);
        return false;
    }
    setStatus(tr("Subtotals added under %1 groups of equal entries.").arg(groups));
    return true;
}

bool CrunchWindow::removeSubtotalsIn()
{
    if (!m_session || !editingAllowed()) return false;
    const jp::office::sheet::CellRange range = listRegion(tr("Subtotal"));
    if (!range.valid) return false;
    const qint64 before = m_loader.changesMade();
    int removed = 0;
    const crunch::EditSession::Outcome o = crunch::removeSubtotals(*m_session, m_sheet, range, &removed, [this] { m_changesNoted = m_loader.changesMade(); });
    if (m_loader.changesMade() != before) m_calculating = true;
    if (!o.ok) {
        notice(tr("Subtotal"), o.problem);
        return false;
    }
    if (removed == 0) notice(tr("Subtotal"), tr("There are no subtotals in this list."), false);
    return removed > 0;
}

void CrunchWindow::advancedFilterDialog()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || !m_session) return;
    QString list;
    {
        jp::office::sheet::CellRange range = selectedRange();
        if (range.row1 == range.row2 && range.col1 == range.col2) {
            bool truncated = false;
            const jp::office::sheet::CellRange region = crunch::currentRegion(*m_session, m_sheet, range.row1, range.col1, &truncated);
            if (!truncated && region.cellCount() > 1) range = region;
        }
        if (range.cellCount() > 1) list = jp::office::sheet::cellName(range.row1, range.col1) + QLatin1Char(':') + jp::office::sheet::cellName(range.row2, range.col2);
    }
    AdvancedFilterDialog dialog(list, this);
    if (dialog.exec() != QDialog::Accepted) return;
    advancedFilterIn(dialog.list(), dialog.criteria(), dialog.copyTo(), dialog.unique());
}

void CrunchWindow::clearAdvancedFilter()
{
    if (!m_advancedRows.valid || !m_session) return;
    crunch::EditSession::SizeChange show;
    show.setHidden = true;
    show.hidden = false;
    m_session->resizeRows(m_sheet, m_advancedRows.row1, m_advancedRows.row2, show, tr("Show All Rows"));
    m_advancedRows = jp::office::sheet::CellRange();
}

bool CrunchWindow::advancedFilterIn(const QString &listText, const QString &criteriaText, const QString &copyTo, bool unique)
{
    if (!m_session || !editingAllowed()) return false;
    const QString title = tr("Advanced Filter");
    crunch::AdvancedFilterSpec spec;
    spec.unique = unique;
    if (!jp::office::sheet::parseRangeRef(listText, &spec.list) || !spec.list.valid) {
        notice(title, tr("Write the list as a range of cells, such as A1:D40."));
        return false;
    }
    if (!jp::office::sheet::parseRangeRef(criteriaText, &spec.criteria) || !spec.criteria.valid) {
        notice(title, tr("Write the conditions as a range of cells, such as F1:G2."));
        return false;
    }
    const auto overlaps = [](const jp::office::sheet::CellRange &a, const jp::office::sheet::CellRange &b) {
        return a.row1 <= b.row2 && b.row1 <= a.row2 && a.col1 <= b.col2 && b.col1 <= a.col2;
    };
    if (overlaps(spec.list, spec.criteria)) {
        notice(title, tr("The conditions must not be inside the list."));
        return false;
    }
    quint32 toRow = 0, toCol = 0;
    const bool copying = !copyTo.isEmpty();
    if (copying && !jp::office::sheet::parseCellRef(copyTo, &toRow, &toCol)) {
        notice(title, tr("Write where to copy the rows as a cell name, such as H1."));
        return false;
    }
    if (copying) {
        jp::office::sheet::CellRange target;
        target.valid = true;
        target.row1 = toRow;
        target.col1 = toCol;
        target.row2 = toRow + (spec.list.row2 - spec.list.row1);
        target.col2 = toCol + (spec.list.col2 - spec.list.col1);
        if (overlaps(target, spec.list) || overlaps(target, spec.criteria)) {
            notice(title, tr("The rows would be copied over the list or the conditions. Choose a place beside or below them."));
            return false;
        }
    }
    const crunch::AdvancedFilterResult found = crunch::advancedFilterRows(*m_session, m_sheet, spec);
    if (!found.ok) {
        notice(title, found.problem);
        return false;
    }
    QUndoStack *stack = m_session->undoStack();
    const qint64 before = m_loader.changesMade();
    crunch::EditSession::Outcome out;
    stack->beginMacro(title);
    // runs of rows that go together
    std::vector<std::pair<quint32, quint32>> runs;
    for (const quint32 row : found.rows) {
        if (!runs.empty() && runs.back().second + 1 == row) runs.back().second = row;
        else runs.push_back({row, row});
    }
    if (copying) {
        auto rowRange = [&](quint32 first, quint32 last) {
            jp::office::sheet::CellRange r = spec.list;
            r.row1 = first;
            r.row2 = last;
            return r;
        };
        auto put = [&](quint32 first, quint32 last, quint32 atRow) {
            jp::office::sheet::CellRange target;
            target.valid = true;
            target.row1 = atRow;
            target.row2 = atRow + (last - first);
            target.col1 = toCol;
            target.col2 = toCol + (spec.list.col2 - spec.list.col1);
            return m_session->paste(m_sheet, target, m_session->copyCells(m_sheet, rowRange(first, last), false), crunch::EditSession::PasteOptions());
        };
        out = put(spec.list.row1, spec.list.row1, toRow);
        quint32 at = toRow + 1;
        for (size_t i = 0; i < runs.size() && out.ok; ++i) {
            out = put(runs[i].first, runs[i].second, at);
            at += runs[i].second - runs[i].first + 1;
        }
    } else {
        crunch::EditSession::SizeChange show, hide;
        show.setHidden = hide.setHidden = true;
        hide.hidden = true;
        out = m_session->resizeRows(m_sheet, spec.list.row1 + 1, spec.list.row2, show, title);
        quint32 next = spec.list.row1 + 1;
        for (size_t i = 0; i <= runs.size() && out.ok; ++i) {
            const quint32 stop = i < runs.size() ? runs[i].first : spec.list.row2 + 1;
            if (stop > next) out = m_session->resizeRows(m_sheet, next, stop - 1, hide, title);
            if (i < runs.size()) next = runs[i].second + 1;
        }
    }
    endEditStep(stack);
    if (m_loader.changesMade() != before) m_calculating = true;
    if (!out.ok) {
        stack->undo();
        notice(title, out.problem);
        return false;
    }
    if (!copying) {
        m_advancedRows = spec.list;
        m_advancedRows.row1 = spec.list.row1 + 1;
    }
    setStatus(tr("%1 of %2 rows meet the conditions.").arg(found.rows.size()).arg(spec.list.row2 - spec.list.row1));
    return true;
}

namespace {
quint16 ignoreBitOf(crunch::FormulaProblem::Kind kind)
{
    using jp::office::sheet::IgnoredError;
    switch (kind) {
    case crunch::FormulaProblem::Kind::ErrorValue: return IgnoredError::EvalError;
    case crunch::FormulaProblem::Kind::InconsistentFormula: return IgnoredError::Formula;
    case crunch::FormulaProblem::Kind::TwoDigitYear: return IgnoredError::TwoDigitTextYear;
    case crunch::FormulaProblem::Kind::UnlockedFormula: return IgnoredError::UnlockedFormula;
    default: return IgnoredError::NumberStoredAsText;
    }
}
}

namespace {
// The cells a person chose to ignore, by kind, so that a problem is looked up and not compared with each range: the ranges of a kind sorted by their first row, with the largest last row of those up to each.
struct IgnoreIndex {
    struct Entry {
        quint32 row1, row2, col1, col2;
    };
    struct Kind {
        std::vector<Entry> ranges;
        std::vector<quint32> reach;   // the largest row2 among ranges[0..i]
    };
    std::map<quint16, Kind> kinds;
    explicit IgnoreIndex(const QVector<jp::office::sheet::IgnoredError> &ignored)
    {
        for (const jp::office::sheet::IgnoredError &e : ignored)
            for (const quint16 bit : {quint16(1), quint16(2), quint16(4), quint16(8), quint16(16), quint16(32), quint16(64), quint16(128), quint16(256)})
                if (e.kinds & bit)
                    for (const jp::office::sheet::CellRange &r : e.ranges) kinds[bit].ranges.push_back({r.row1, r.row2, r.col1, r.col2});
        for (auto &[bit, k] : kinds) {
            std::sort(k.ranges.begin(), k.ranges.end(), [](const Entry &a, const Entry &b) { return a.row1 < b.row1; });
            quint32 top = 0;
            for (const Entry &e : k.ranges) k.reach.push_back(top = std::max(top, e.row2));
        }
    }
    bool has(quint16 bit, quint32 row, quint32 col) const
    {
        const auto found = kinds.find(bit);
        if (found == kinds.end()) return false;
        const Kind &k = found->second;
        // the ranges that begin at or above the row, from the last backwards while some of them still reach it
        size_t n = size_t(std::upper_bound(k.ranges.begin(), k.ranges.end(), row, [](quint32 r, const Entry &e) { return r < e.row1; }) - k.ranges.begin());
        while (n > 0 && k.reach[n - 1] >= row) {
            const Entry &e = k.ranges[--n];
            if (row <= e.row2 && col >= e.col1 && col <= e.col2) return true;
        }
        return false;
    }
};
} // namespace

std::vector<crunch::FormulaProblem> CrunchWindow::formulaProblems(bool patterns) const
{
    std::vector<crunch::FormulaProblem> out;
    if (!m_session) return out;
    std::vector<crunch::FormulaProblem> found = crunch::findFormulaProblems(*m_session, m_sheet, nullptr, patterns);
    // (a window that asks after each change has the neighbor rule from the slices that ran, for this sheet)
    if (!patterns && m_patternSheet == m_sheet) found = crunch::mergeInconsistent(std::move(found), m_patternFound);
    const IgnoreIndex ignored(m_session->ignoredErrorsOf(m_sheet));
    for (const crunch::FormulaProblem &p : found)
        if (!ignored.has(ignoreBitOf(p.kind), p.row, p.col)) out.push_back(p);
    return out;
}

void CrunchWindow::ignoreFormulaProblem(quint32 row, quint32 col)
{
    if (!m_session || !editingAllowed()) return;
    QVector<jp::office::sheet::IgnoredError> list = m_session->ignoredErrorsOf(m_sheet);
    bool added = false;
    for (const crunch::FormulaProblem &p : formulaProblems()) {
        if (p.row != row || p.col != col) continue;
        jp::office::sheet::IgnoredError e;
        e.kinds = ignoreBitOf(p.kind);
        jp::office::sheet::CellRange r;
        r.valid = true;
        r.row1 = r.row2 = row;
        r.col1 = r.col2 = col;
        e.ranges = {r};
        list.append(e);
        added = true;
    }
    if (!added) return;
    const crunch::EditSession::Outcome o = m_session->setIgnoredErrors(m_sheet, list, tr("Ignore Error"));
    if (!o.ok) notice(tr("Error Checking"), o.problem);
    refreshCellFlags();
}

void CrunchWindow::errorCheckingDialog()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !m_session) return;
    const std::vector<crunch::FormulaProblem> problems = formulaProblems();
    if (problems.empty()) {
        notice(tr("Error Checking"), tr("No errors were found on this sheet."), false);
        return;
    }
    QStringList cells, messages;
    int first = 0;
    const grid::Selection &sel = m_view->selection();
    for (size_t i = 0; i < problems.size(); ++i) {
        cells << jp::office::sheet::cellName(problems[i].row, problems[i].col);
        messages << problems[i].explanation;
        if (first == 0 && (qint64(problems[i].row) > sel.activeRow || (qint64(problems[i].row) == sel.activeRow && qint64(problems[i].col) >= sel.activeCol))) first = int(i);
    }
    auto list = std::make_shared<std::vector<crunch::FormulaProblem>>(problems);
    ErrorCheckDialog dialog(cells, messages, first, this);
    connect(&dialog, &ErrorCheckDialog::showing, this, [this, list](int i) {
        if (i >= 0 && i < int(list->size())) m_view->selectCell((*list)[size_t(i)].row, (*list)[size_t(i)].col);
    });
    connect(&dialog, &ErrorCheckDialog::ignored, this, [this, list](int i) {
        if (i < 0 || i >= int(list->size())) return;
        ignoreFormulaProblem((*list)[size_t(i)].row, (*list)[size_t(i)].col);
        list->erase(list->begin() + i);
    });
    dialog.exec();
}

bool CrunchWindow::addWatch(int sheet, quint32 row, quint32 col)
{
    constexpr int kMaxWatches = 500;
    if (!m_session || m_watches.size() >= kMaxWatches) return false;
    for (const Watch &w : m_watches)
        if (w.sheet == sheet && w.row == row && w.col == col) return false;
    m_watches.append({sheet, row, col});
    refreshWatches();
    return true;
}

void CrunchWindow::refreshWatches()
{
    if (!m_watchWindow) return;
    QVector<QStringList> rows;
    m_watchRows.clear();
    if (m_session && m_loaded && m_loaded->book) {
        const auto &sheets = m_loaded->book->info().sheets;
        for (int i = 0; i < int(m_watches.size()); ++i) {
            const Watch &w = m_watches[i];
            if (w.sheet < 0 || w.sheet >= int(sheets.size()) || sheets[size_t(w.sheet)].deleted) continue;
            m_watchRows.push_back(i);
            const jp::calc::Value v = m_session->valueAt(w.sheet, w.row, w.col);
            QString shown;
            if (v.isNumber()) shown = QLocale().toString(v.number(), 'g', 12);
            else if (v.isText()) shown = v.text();
            else if (v.isBool()) shown = v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
            else if (v.isError()) shown = jp::calc::errorKindText(v.errorKind());
            const QString held = m_session->editText(w.sheet, w.row, w.col);
            rows.push_back({sheets[size_t(w.sheet)].name, jp::office::sheet::cellName(w.row, w.col), shown, held.startsWith(QLatin1Char('=')) ? held : QString()});
        }
    }
    m_watchWindow->setRows(rows);
}

void CrunchWindow::showWatchWindow()
{
    if (!m_session) return;
    if (!m_watchWindow) {
        m_watchWindow = new WatchWindow(this);
        connect(m_watchWindow, &WatchWindow::addRequested, this, [this] {
            const jp::office::sheet::CellRange r = selectedRange();
            for (quint32 row = r.row1; row <= r.row2 && row < r.row1 + 100; ++row)
                for (quint32 col = r.col1; col <= r.col2 && col < r.col1 + 100; ++col)
                    if (!addWatch(m_sheet, row, col)) return;
        });
        connect(m_watchWindow, &WatchWindow::deleteRequested, this, [this](const QList<int> &rows) {
            // (the rows are the watches of sheets that are there: m_watchRows says which watch each row is)
            QList<int> picked;
            for (const int row : rows)
                if (row >= 0 && row < m_watchRows.size()) picked << m_watchRows[row];
            std::sort(picked.begin(), picked.end());
            for (int i = int(picked.size()) - 1; i >= 0; --i)
                if (picked[i] >= 0 && picked[i] < m_watches.size()) m_watches.removeAt(picked[i]);
            refreshWatches();
        });
        connect(m_watchWindow, &WatchWindow::goToRequested, this, [this](int row) {
            if (row < 0 || row >= m_watchRows.size() || m_watchRows[row] >= m_watches.size()) return;
            const Watch w = m_watches[m_watchRows[row]];
            if (w.sheet != m_sheet) setActiveSheet(w.sheet);
            if (w.sheet != m_sheet) return;
            m_view->selectCell(w.row, w.col);
        });
        connect(this, &CrunchWindow::calculationFinished, m_watchWindow, [this] { refreshWatches(); });
    }
    refreshWatches();
    m_watchWindow->show();
    m_watchWindow->raise();
}

std::unique_ptr<crunch::FormulaSteps> CrunchWindow::formulaStepsOfCursorCell()
{
    if (!m_session) return nullptr;
    const grid::Selection &sel = m_view->selection();
    const quint32 row = quint32(std::max<qint64>(0, sel.activeRow)), col = quint32(std::max<qint64>(0, sel.activeCol));
    const QString text = m_session->editText(m_sheet, row, col);
    if (!text.startsWith(QLatin1Char('=')) || text.size() < 2) return nullptr;
    const int sheet = m_sheet;
    auto eval = [this, sheet, row, col](const QString &expression, QString *problem) -> std::optional<QString> {
        const crunch::EditSession::Preview p = m_session->previewFormula(expression, sheet, row, col);
        if (p.ok) return p.shown;
        *problem = p.problem.isEmpty() ? tr("This part can't be worked out here.") : p.problem;
        return std::nullopt;
    };
    return std::make_unique<crunch::FormulaSteps>(text.mid(1), eval);
}

void CrunchWindow::evaluateFormulaDialog()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !m_session) return;
    const grid::Selection &sel = m_view->selection();
    const quint32 row = quint32(std::max<qint64>(0, sel.activeRow)), col = quint32(std::max<qint64>(0, sel.activeCol));
    const QString text = m_session->editText(m_sheet, row, col);
    if (!text.startsWith(QLatin1Char('=')) || text.size() < 2) {
        notice(tr("Evaluate Formula"), tr("Select a cell that holds a formula."));
        return;
    }
    const int sheet = m_sheet;
    auto eval = [this, sheet, row, col](const QString &expression, QString *problem) -> std::optional<QString> {
        const crunch::EditSession::Preview p = m_session->previewFormula(expression, sheet, row, col);
        if (p.ok) return p.shown;
        *problem = p.problem.isEmpty() ? tr("This part can't be worked out here.") : p.problem;
        return std::nullopt;
    };
    EvaluateFormulaDialog dialog(jp::office::sheet::cellName(row, col), text.mid(1), eval, this);
    dialog.exec();
}

bool CrunchWindow::setValidationOn(const jp::office::sheet::CellRange &where, const std::optional<jp::office::sheet::Validation> &rule)
{
    if (!m_session || !editingAllowed()) return false;
    const qint64 before = m_loader.changesMade();
    const crunch::EditSession::Outcome o = m_session->setValidations(m_sheet, crunch::withRuleOn(m_session->validationsOf(m_sheet), where, rule), tr("Data Validation"));
    if (m_loader.changesMade() != before) m_calculating = true;
    if (!o.ok) {
        notice(tr("Data Validation"), o.problem);
        return false;
    }
    updateBars();
    return true;
}

void CrunchWindow::dataValidationDialog()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || !m_session || !m_loaded || !m_loaded->calc) return;
    const jp::office::sheet::CellRange where = selectedRange();
    const grid::Selection &sel = m_view->selection();
    const jp::office::sheet::Validation *held = m_loaded->calc->validationAt(m_sheet, quint32(std::max<qint64>(0, sel.activeRow)), quint32(std::max<qint64>(0, sel.activeCol)));
    ValidationDialog dialog(held ? std::optional<jp::office::sheet::Validation>(*held) : std::nullopt, this);
    for (;;) {
        if (dialog.exec() != QDialog::Accepted) return;
        if (dialog.clearAll()) {
            setValidationOn(where, std::nullopt);
            return;
        }
        QString problem;
        const std::optional<jp::office::sheet::Validation> rule = dialog.rule(&problem);
        if (!rule) {
            notice(tr("Data Validation"), problem);
            continue;
        }
        setValidationOn(where, rule);
        return;
    }
}

void CrunchWindow::consolidateDialog()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || !m_session) return;
    const grid::Selection &sel = m_view->selection();
    ConsolidateDialog dialog(jp::office::sheet::cellName(quint32(std::max<qint64>(0, sel.activeRow)), quint32(std::max<qint64>(0, sel.activeCol))), this);
    if (dialog.exec() != QDialog::Accepted) return;
    consolidateIn(dialog.references(), dialog.function(), dialog.topLabels(), dialog.leftLabels(), dialog.links(), dialog.destination());
}

bool CrunchWindow::consolidateIn(const QStringList &references, int function, bool topLabels, bool leftLabels, bool links, const QString &destination)
{
    if (!m_session || !editingAllowed()) return false;
    const QString title = tr("Consolidate");
    crunch::ConsolidateSpec spec;
    spec.function = function;
    spec.topLabels = topLabels;
    spec.leftLabels = leftLabels;
    spec.links = links;
    spec.sheet = m_sheet;
    for (const QString &ref : references) {
        const crunch::GoToTarget t = crunch::resolveGoTo(*m_session, ref, m_sheet);
        if (!t.ok || t.wholeRows || t.wholeColumns) {
            notice(title, tr("%1 is not a range of cells. Write it as A1:C10, or Sheet2!A1:C10 for another sheet.").arg(ref));
            return false;
        }
        spec.sources.push_back({t.sheet, t.range});
    }
    if (!jp::office::sheet::parseCellRef(destination, &spec.row, &spec.col)) {
        notice(title, tr("Write where the result starts as a cell name, such as H1."));
        return false;
    }
    const qint64 before = m_loader.changesMade();
    jp::office::sheet::CellRange written;
    const crunch::EditSession::Outcome o = crunch::consolidate(*m_session, spec, &written, [this] { m_changesNoted = m_loader.changesMade(); });
    if (m_loader.changesMade() != before) m_calculating = true;
    if (!o.ok) {
        notice(title, o.problem);
        return false;
    }
    return true;
}

int CrunchWindow::circleInvalidData()
{
    if (!m_session || !m_loaded || !m_loaded->calc) return 0;
    std::vector<jp::office::sheet::CellRange> cells;
    {
        std::unique_ptr<crunch::WorkbookLoader::Hold> hold = m_loader.hold();
        if (!hold) return 0;
        cells = crunch::invalidCells(*hold->calculation(), m_sheet);
    }
    if (cells.empty()) m_invalidCircled.erase(m_sheet);
    else m_invalidCircled[m_sheet] = cells;
    refreshCellFlags();
    return int(cells.size());
}

void CrunchWindow::clearValidationCircles()
{
    m_invalidCircled.clear();
    refreshCellFlags();
}

void CrunchWindow::refreshInvalidCircles()
{
    if (m_invalidCircled.empty() || !m_loaded || !m_loaded->calc) return;
    std::unique_ptr<crunch::WorkbookLoader::Hold> hold = m_loader.hold();
    if (!hold) return;
    for (auto it = m_invalidCircled.begin(); it != m_invalidCircled.end();) {
        it->second = crunch::invalidCells(*hold->calculation(), it->first);
        it = it->second.empty() ? m_invalidCircled.erase(it) : std::next(it);
    }
}

std::vector<jp::office::sheet::CellRange> CrunchWindow::circledCells() const
{
    const auto it = m_invalidCircled.find(m_sheet);
    return it == m_invalidCircled.end() ? std::vector<jp::office::sheet::CellRange>() : it->second;
}

// The ovals and the green triangles of the sheet in view.
// The rule of inconsistent formulas runs a slice at a time while the window is idle, and starts again at each change; the corners it found last are shown meanwhile.
void CrunchWindow::startPatternScan()
{
    m_patternScan.reset();
    if (!m_session || !m_haveCalc || !m_session->calculation()) return;
    m_patternScan = std::make_unique<crunch::InconsistentScan>(*m_session, m_sheet);
    if (!m_patternTimer) {
        m_patternTimer = new QTimer(this);
        m_patternTimer->setSingleShot(true);
        connect(m_patternTimer, &QTimer::timeout, this, [this] {
            if (!m_patternScan || !m_session || !m_session->calculation()) return;
            if (m_calculating || m_loading || m_saving) {   // (the cells are being worked on: later)
                m_patternTimer->start(50);
                return;
            }
            if (!m_patternScan->step(1000)) {
                m_patternTimer->start(0);
                return;
            }
            m_patternFound = m_patternScan->found();
            m_patternSheet = m_patternScan->sheet();
            m_patternScan.reset();
            refreshCellFlags(false);
        });
    }
    m_patternTimer->start(0);
}

void CrunchWindow::refreshCellFlags(bool rescan)
{
    std::vector<grid::GridRange> ovals, corners;
    for (const auto &r : circledCells()) ovals.push_back(grid::GridRange{qint64(r.row1), qint64(r.col1), qint64(r.row2), qint64(r.col2)});
    if (m_session && m_loaded && m_haveCalc) {
        // (a green triangle on each cell with a problem)
        for (const crunch::FormulaProblem &p : formulaProblems(false)) corners.push_back(grid::GridRange{qint64(p.row), qint64(p.col), qint64(p.row), qint64(p.col)});
        if (rescan) startPatternScan();
    }
    if (QAction *a = act(QStringLiteral("data.clearCircles"))) a->setEnabled(!m_invalidCircled.empty());
    m_view->setCellFlags(std::move(ovals), std::move(corners));
}

std::vector<crunch::AccessIssue> CrunchWindow::accessibilityIssues()
{
    return m_session ? crunch::checkAccessibility(*m_session, m_objects.get()) : std::vector<crunch::AccessIssue>();
}

void CrunchWindow::checkAccessibilityNow()
{
    if (!m_accessPane) return;
    QVector<AccessFinding> findings;
    for (const crunch::AccessIssue &i : accessibilityIssues())
        findings.push_back({i.level == crunch::AccessIssue::Error ? AccessFinding::Error : i.level == crunch::AccessIssue::Warning ? AccessFinding::Warning : AccessFinding::Tip, i.message,
                            {i.sheet, i.row, i.col}});
    m_accessPane->setFindings(findings);
}

void CrunchWindow::showAccessibilityPane()
{
    if (!m_session) return;
    if (!m_accessPane) {
        m_accessPane = new AccessibilityPane(this);
        connect(m_accessPane, &AccessibilityPane::recheckRequested, this, [this] { checkAccessibilityNow(); });
        connect(m_accessPane, &AccessibilityPane::goToRequested, this, [this](const QVariantList &where) {
            if (where.size() != 3) return;
            const int sheet = where[0].toInt();
            const quint32 row = where[1].toUInt(), col = where[2].toUInt();
            if (sheet != m_sheet) setActiveSheet(sheet);
            if (sheet != m_sheet) return;
            m_view->selectCell(row, col);
            m_view->ensureVisible(row, col);
        });
    }
    checkAccessibilityNow();
    m_accessPane->show();
    m_accessPane->raise();
}

QString CrunchWindow::thesaurusWordAt(int sheet, quint32 row, quint32 col) const
{
    if (!m_session) return QString();
    const QString text = m_session->editText(sheet, row, col);
    if (text.startsWith(QLatin1Char('='))) return QString();
    static const QRegularExpression word(QStringLiteral("\\p{L}[\\p{L}'-]*"));
    const QRegularExpressionMatch m = word.match(text);
    return m.hasMatch() ? m.captured() : QString();
}

bool CrunchWindow::replaceWordInCell(int sheet, quint32 row, quint32 col, const QString &word, const QString &chosen)
{
    if (!m_session || word.isEmpty() || chosen.isEmpty()) return false;
    QString text = m_session->editText(sheet, row, col);
    const int at = int(text.indexOf(word));
    if (at < 0) return false;
    QString replacement = chosen;
    if (word.at(0).isUpper()) replacement[0] = replacement.at(0).toUpper();
    text.replace(at, word.size(), replacement);
    if (!editingAllowed()) return false;
    const qint64 before = m_loader.changesMade();
    const crunch::EditSession::Outcome o = m_session->type(sheet, row, col, text);
    if (!o.ok) {
        notice(tr("Thesaurus"), o.problem);
        return false;
    }
    if (m_loader.changesMade() != before) m_calculating = true;
    return true;
}

void CrunchWindow::showCell(int sheet, quint32 row, quint32 col)
{
    if (sheet != m_sheet) setActiveSheet(sheet);
    if (m_view->selection().isOneCell()) m_view->selectCell(row, col);
    else m_view->ensureVisible(row, col);
}

bool CrunchWindow::replaceCellTexts(const std::vector<CellText> &changes, const QString &stepText)
{
    if (!m_session || changes.empty() || !editingAllowed()) return false;
    // All or none: every cell is looked at first (a locked cell, a cell of an array, a text too long for a cell), so that a change that cannot be made stops before any is.
    for (const CellText &c : changes) {
        QString why;
        if (!m_session->canEditCell(c.sheet, c.row, c.col, &why)) {
            notice(stepText, why);
            return false;
        }
        if (c.text.size() > jp::office::sheet::kMaxCellChars) {
            notice(stepText, tr("A cell can hold up to %1 characters.").arg(QLocale().toString(jp::office::sheet::kMaxCellChars)));
            return false;
        }
    }
    const qint64 before = m_loader.changesMade();
    QUndoStack *stack = m_session->undoStack();
    const bool several = changes.size() > 1;
    const int index = stack->index();
    if (several) stack->beginMacro(stepText);
    bool ok = true;
    for (const CellText &c : changes) {
        // (the cell's formatting runs go with the words: only the stretch that changed is new)
        std::optional<jp::office::sheet::RichText> rich = m_session->richOf(c.sheet, c.row, c.col);
        if (rich && !rich->isEmpty()) crunch::adjustRuns(&*rich, c.text);
        else rich.reset();
        // The new text is text: with an apostrophe in front, it is not read as an entry again (a date, TRUE, a number, a formula), and the cell stays the text it was.
        const crunch::EditSession::Outcome o = m_session->type(c.sheet, c.row, c.col, QLatin1Char('\'') + c.text, false, rich ? &*rich : nullptr);
        if (!o.ok) {
            notice(stepText, o.problem);
            ok = false;
            break;
        }
    }
    if (several) stack->endMacro();
    if (!ok && stack->index() > index) stack->undo();   // (not one of them is kept)
    if (m_loader.changesMade() != before) m_calculating = true;
    return ok;
}

QString CrunchWindow::textToTranslate() const
{
    if (m_edit.active) {
        QLineEdit *line = m_point.line ? m_point.line.data() : static_cast<QLineEdit *>(m_cellEditor);
        return line->hasSelectedText() ? line->selectedText() : line->text();
    }
    const jp::office::sheet::Calculation *calc = m_session ? m_session->calculation() : nullptr;
    if (!calc || m_sheet < 0 || m_sheet >= calc->sheetCount()) return QString();
    const auto &s = calc->sheet(m_sheet);
    if (s.extentRows == 0 || s.extentCols == 0) return QString();
    const jp::office::sheet::CellRange range = selectedRange();
    const quint32 r2 = std::min(range.row2, s.extentRows - 1), c2 = std::min(range.col2, s.extentCols - 1);
    if (range.row1 > r2 || range.col1 > c2) return QString();
    // (the texts in reading order; more than the service takes are not collected)
    std::vector<std::pair<quint32, quint32>> cells;
    s.cells.forEachInRange(range.row1, range.col1, r2, c2, [&](quint32 row, quint32 col, const jp::office::sheet::Cell &) {
        cells.emplace_back(row, col);
        return cells.size() < 20000;
    });
    std::sort(cells.begin(), cells.end());
    QString text;
    quint32 lastRow = 0;
    bool first = true;
    for (const auto &rc : cells) {
        const jp::office::sheet::Calculation::CellContent c = calc->content(m_sheet, rc.first, rc.second);
        if (!c.exists || !c.value.isText() || c.value.text().isEmpty()) continue;
        text += first ? QString() : (rc.first != lastRow ? QStringLiteral("\n") : QStringLiteral(" "));
        text += c.value.text();
        first = false;
        lastRow = rc.first;
        if (text.size() > jp::kMostTranslateLetters) break;
    }
    return text;
}

void CrunchWindow::spelling()
{
    if (!m_session || m_sheet < 0 || !commitEdit()) return;
    const grid::Selection &sel = m_view->selection();
    jp::office::sheet::CellRange range{0, 0, jp::office::sheet::kMaxRows - 1, jp::office::sheet::kMaxColumns - 1, true};
    // (more than one cell selected: only those, as the original does; else the whole sheet)
    if (!sel.isOneCell()) range = selectedRange();
    SpellingDialog dlg(this, m_sheet, range);
    if (dlg.start()) dlg.exec();
}

void CrunchWindow::thesaurus()
{
    if (!m_session || !commitEdit()) return;
    const grid::Selection &sel = m_view->selection();
    const quint32 row = quint32(std::max<qint64>(0, sel.activeRow)), col = quint32(std::max<qint64>(0, sel.activeCol));
    const QString word = thesaurusWordAt(m_sheet, row, col);
    QDialog dlg(this);
    dlg.setObjectName(QStringLiteral("thesaurusDialog"));
    dlg.setWindowTitle(tr("Thesaurus"));
    auto *v = new QVBoxLayout(&dlg);
    v->addWidget(plainLabel(new QLabel(tr("Look up:"), &dlg)));
    auto *query = new QLineEdit(word, &dlg);
    query->setObjectName(QStringLiteral("thesaurusQuery"));
    v->addWidget(query);
    auto *list = new QListWidget(&dlg);
    list->setObjectName(QStringLiteral("thesaurusList"));
    v->addWidget(list);
    auto fill = [&] {
        list->clear();
        const QStringList found = jp::thesaurusLookup(query->text());
        if (found.isEmpty()) {
            auto *none = new QListWidgetItem(query->text().trimmed().isEmpty() ? tr("(Type a word to look up)") : tr("(No suggestions)"), list);
            none->setFlags(Qt::NoItemFlags);
            return;
        }
        for (const QString &entry : found) {
            // (an entry is the word and, in parentheses, the kind of word it is: "glad (adj)")
            QString text = entry;
            const int paren = int(text.lastIndexOf(QLatin1String(" (")));
            if (paren > 0) text.truncate(paren);
            auto *item = new QListWidgetItem(entry, list);
            item->setData(Qt::UserRole, text);
        }
        list->setCurrentRow(0);
    };
    fill();
    auto *buttons = new QDialogButtonBox(&dlg);
    auto *replace = buttons->addButton(tr("Replace"), QDialogButtonBox::AcceptRole);
    replace->setObjectName(QStringLiteral("thesaurusReplace"));
    auto *lookUp = buttons->addButton(tr("Look Up"), QDialogButtonBox::ActionRole);
    buttons->addButton(QDialogButtonBox::Close);
    connect(lookUp, &QPushButton::clicked, &dlg, [&] {
        if (list->currentItem() && list->currentItem()->data(Qt::UserRole).isValid()) query->setText(list->currentItem()->data(Qt::UserRole).toString());
        fill();
    });
    connect(query, &QLineEdit::returnPressed, &dlg, fill);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    connect(list, &QListWidget::itemDoubleClicked, &dlg, &QDialog::accept);
    v->addWidget(buttons);
    if (dlg.exec() != QDialog::Accepted || !list->currentItem() || !list->currentItem()->data(Qt::UserRole).isValid() || word.isEmpty()) return;
    replaceWordInCell(m_sheet, row, col, word, list->currentItem()->data(Qt::UserRole).toString());
}

} // namespace jp
