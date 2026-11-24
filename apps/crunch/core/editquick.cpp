// What Quick Analysis does to the workbook: the totals it writes (planned first, so that the window can say what would be replaced, and then put in as one step), whether a block has enough
// in it to be offered, and the rule it shows on the sheet while a choice is looked at (put in the sheet's rules without a step of Undo, and taken out again).
#include "core/editsession.h"

#include "sheet/cellref.h"

#include <QCoreApplication>
#include <QLocale>

#include <algorithm>

namespace jp::crunch {

using namespace jp::office;
using namespace jp::office::sheet;

namespace {

QString refOf(quint32 row, quint32 col, bool absRow = false, bool absCol = false)
{
    return (absCol ? QStringLiteral("$") : QString()) + columnName(col) + (absRow ? QStringLiteral("$") : QString()) + QString::number(qint64(row) + 1);
}

QString blockOf(quint32 row1, quint32 col1, quint32 row2, quint32 col2, bool absolute = false)
{
    return refOf(row1, col1, absolute, absolute) + QLatin1Char(':') + refOf(row2, col2, absolute, absolute);
}

} // namespace

bool EditSession::quickHasData(int sheet, const CellRange &sel) const
{
    if (!m_book || !m_book->calc || sheet < 0 || sheet >= m_book->calc->sheetCount() || !sel.valid) return false;
    const Sheet &s = m_book->calc->sheet(sheet);
    if (s.extentRows == 0 || s.extentCols == 0 || sel.row1 >= s.extentRows || sel.col1 >= s.extentCols) return false;
    const quint32 row2 = std::min<quint32>(sel.row2, s.extentRows - 1), col2 = std::min<quint32>(sel.col2, s.extentCols - 1);
    qint64 looked = 0, found = 0;
    for (quint32 r = sel.row1; r <= row2; ++r)
        for (quint32 c = sel.col1; c <= col2; ++c) {
            if (++looked > kQuickProbe) return true;   // (a large block is taken to have what it needs: finding out would cost what the button is not to)
            if (!valueAt(sheet, r, c).isEmpty() || m_book->calc->content(sheet, r, c).isFormula)
                if (++found >= 2) return true;
        }
    return false;
}

EditSession::QuickTotalsPlan EditSession::planQuickTotals(int sheet, const CellRange &sel, QuickTotal kind, bool down) const
{
    QuickTotalsPlan plan;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        plan.problem = why;
        return plan;
    }
    if (!m_book || !m_book->calc || sheet < 0 || sheet >= m_book->calc->sheetCount() || !sel.valid) return plan;
    const Sheet &s = m_book->calc->sheet(sheet);
    if (s.extentRows == 0 || s.extentCols == 0 || sel.row1 >= s.extentRows || sel.col1 >= s.extentCols) {
        plan.problem = tr("There are no numbers here to total.");
        return plan;
    }
    quint32 r0 = sel.row1, c0 = sel.col1;
    const quint32 r1 = std::min<quint32>(sel.row2, s.extentRows - 1), c1 = std::min<quint32>(sel.col2, s.extentCols - 1);
    if (qint64(r1 - r0 + 1) * qint64(c1 - c0 + 1) > kQuickScan) {
        plan.problem = tr("That is more cells than Quick Analysis looks at (%1). Select fewer cells.").arg(QLocale().toString(kQuickScan));
        return plan;
    }
    auto isNumber = [&](quint32 r, quint32 c) { return valueAt(sheet, r, c).isNumber(); };
    auto anyNumber = [&](quint32 ra, quint32 ca, quint32 rb, quint32 cb) {
        for (quint32 r = ra; r <= rb; ++r)
            for (quint32 c = ca; c <= cb; ++c)
                if (isNumber(r, c)) return true;
        return false;
    };
    // (a first row or column with no number in it, over cells that have numbers, holds names)
    if (r1 > r0 && !anyNumber(r0, c0, r0, c1) && anyNumber(r0 + 1, c0, r1, c1)) ++r0;
    if (c1 > c0 && !anyNumber(r0, c0, r1, c0) && anyNumber(r0, c0 + 1, r1, c1)) ++c0;
    if (!anyNumber(r0, c0, r1, c1)) {
        plan.problem = tr("There are no numbers here to total.");
        return plan;
    }
    plan.data = CellRange{r0, c0, r1, c1, true};
    auto put = [&](quint32 row, quint32 col, const QString &formula, bool percent) {
        QuickCell cell;
        cell.row = row;
        cell.col = col;
        cell.formula = formula;
        cell.percent = percent;
        plan.cells.push_back(cell);
    };
    auto fn = [&](const QString &range) {
        switch (kind) {
        case QuickTotal::Sum: return QStringLiteral("SUM(") + range + QLatin1Char(')');
        case QuickTotal::Average: return QStringLiteral("AVERAGE(") + range + QLatin1Char(')');
        case QuickTotal::Count: return QStringLiteral("COUNT(") + range + QLatin1Char(')');
        default: return QStringLiteral("SUM(") + range + QStringLiteral(")/SUM(") + blockOf(r0, c0, r1, c1, true) + QLatin1Char(')');
        }
    };
    const quint32 lastRow = jp::calc::kGridRows - 1, lastCol = jp::calc::kGridCols - 1;
    if (kind == QuickTotal::RunningTotal) {
        if (down ? c1 + 1 + (c1 - c0) > lastCol : r1 + 1 + (r1 - r0) > lastRow) {
            plan.problem = tr("There is no room next to the cells for the running totals.");
            return plan;
        }
        for (quint32 c = c0; c <= c1; ++c) {
            if (down && !anyNumber(r0, c, r1, c)) continue;
            for (quint32 r = r0; r <= r1; ++r) {
                if (!down && !anyNumber(r, c0, r, c1)) continue;
                if (down) put(r, c1 + 1 + (c - c0), QStringLiteral("SUM(%1:%2)").arg(refOf(r0, c, true, false), refOf(r, c)), false);
                else put(r1 + 1 + (r - r0), c, QStringLiteral("SUM(%1:%2)").arg(refOf(r, c0, false, true), refOf(r, c)), false);
            }
        }
    } else if (down) {
        if (r1 + 1 > lastRow) {
            plan.problem = tr("There is no room under the cells for the totals.");
            return plan;
        }
        for (quint32 c = c0; c <= c1; ++c)
            if (anyNumber(r0, c, r1, c)) put(r1 + 1, c, fn(blockOf(r0, c, r1, c)), kind == QuickTotal::PercentOfTotal);
    } else {
        if (c1 + 1 > lastCol) {
            plan.problem = tr("There is no room beside the cells for the totals.");
            return plan;
        }
        for (quint32 r = r0; r <= r1; ++r)
            if (anyNumber(r, c0, r, c1)) put(r, c1 + 1, fn(blockOf(r, c0, r, c1)), kind == QuickTotal::PercentOfTotal);
    }
    if (plan.cells.empty()) {
        plan.problem = tr("There are no numbers here to total.");
        return plan;
    }
    if (qint64(plan.cells.size()) > kMaxCellsPerEdit) {
        plan.problem = tr("That is more cells than JeffCrunch changes at once (%1). Select fewer cells.").arg(QLocale().toString(kMaxCellsPerEdit));
        return plan;
    }
    for (const QuickCell &cell : plan.cells)
        if (!valueAt(sheet, cell.row, cell.col).isEmpty() || m_book->calc->content(sheet, cell.row, cell.col).isFormula) ++plan.overwrites;
    plan.ok = true;
    return plan;
}

EditSession::Outcome EditSession::applyQuickTotals(int sheet, const QuickTotalsPlan &plan, const QString &text)
{
    Outcome out;
    if (!plan.ok || plan.cells.empty()) {
        out.ok = false;
        out.problem = plan.problem;
        return out;
    }
    Step step;
    for (const QuickCell &cell : plan.cells) {
        QString problem;
        if (!canEditCell(sheet, cell.row, cell.col, &problem)) {
            out.ok = false;
            out.problem = problem;
            return out;
        }
        CellChange ch;
        bool none = false;
        if (!typedChange(sheet, cell.row, cell.col, QLatin1Char('=') + formulaToLocale(cell.formula, formulaLocale()), &ch, &none, &out)) return out;
        if (cell.percent) {
            const QString code = formatCodeOf(sheet, cell.row, cell.col);
            if (code == QLatin1String("General") || code.isEmpty()) ch.formatCode = QStringLiteral("0.0%");
        }
        if (!none || !ch.formatCode.isEmpty()) step.cells.push_back(std::move(ch));
    }
    if (step.cells.empty()) return out;
    pushStep(std::move(step), text);
    return out;
}

EditSession::Outcome EditSession::previewCondRule(int sheet, CondRule rule, const DifferentialFormat &format)
{
    Outcome out;
    endCondPreview();
    QString why;
    auto refuse = [&](const QString &text) {
        out.ok = false;
        out.problem = text;
        return out;
    };
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) return refuse(why);
    if (rule.ranges.isEmpty()) return refuse(tr("Select the cells the rule is for."));
    if (const QString no = condRulesRefusal(sheet); !no.isEmpty()) return refuse(no);
    const QVector<CondRule> now = condRules(sheet);
    qint32 dxf = -1;
    bool added = false;
    std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
    if (!hold) return refuse(tr("The workbook is busy. Try again in a moment."));
    const qsizetype before = m_book->book->addedDxfs().size();
    dxf = m_book->book->dxfFor(format);
    added = m_book->book->addedDxfs().size() > before;
    if (dxf < 0 && !format.isEmpty()) return refuse(tr("This workbook has no place to keep the look of a rule."));
    rule.dxf = dxf;
    rule.uid = 0;
    rule.priority = 1;
    QVector<CondRule> after = now;
    for (CondRule &r : after) r.priority += 1;
    after.prepend(rule);
    hold->calculation()->replaceConditionalFormats(sheet, after);
    m_previewSheet = sheet;
    m_previewBefore = now;
    m_previewDxf = added ? dxf : -1;
    hold.reset();
    Q_EMIT conditionalFormatsChanged(sheet);
    return out;
}

void EditSession::endCondPreview()
{
    if (m_previewSheet < 0) return;
    const int sheet = m_previewSheet;
    m_previewSheet = -1;
    if (std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold()) {
        hold->calculation()->replaceConditionalFormats(sheet, m_previewBefore);
        if (m_previewDxf >= 0) m_book->book->removeAddedDxf(m_previewDxf);
    }
    m_previewBefore.clear();
    m_previewDxf = -1;
    Q_EMIT conditionalFormatsChanged(sheet);
}

} // namespace jp::crunch
