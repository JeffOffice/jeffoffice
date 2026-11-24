// What Quick Analysis does to the workbook: the totals it writes (planned first, so that the window can say what would be replaced, and then put in as one step), whether a block has enough
// in it to be offered, and the rule it shows on the sheet while a choice is looked at (put in the sheet's rules without a step of Undo, and taken out again).
#include "core/editsession.h"

#include "sheet/cellref.h"

#include "core/syslocale.h"

#include <QCoreApplication>

#include <algorithm>
#include <unordered_set>

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
        plan.problem = tr("That is more cells than Quick Analysis looks at (%1). Select fewer cells.").arg(jp::systemLocale().toString(kQuickScan));
        return plan;
    }
    // (one pass over the block: how many numbers each row and each column holds; every check after it reads these counts, not the cells)
    m_quickReads = 0;
    const quint32 rows = r1 - r0 + 1, cols = c1 - c0 + 1;
    std::vector<quint32> rowN(rows, 0), colN(cols, 0);
    qint64 total = 0;
    for (quint32 r = r0; r <= r1; ++r)
        for (quint32 c = c0; c <= c1; ++c) {
            ++m_quickReads;
            if (valueAt(sheet, r, c).isNumber()) {
                ++rowN[r - r0];
                ++colN[c - c0];
                ++total;
            }
        }
    if (total == 0) {
        plan.problem = tr("There are no numbers here to total.");
        return plan;
    }
    // (a first row or column with no number in it, over cells that have numbers, holds names: it had none to take out of the counts of the other lines)
    const quint32 rowBase = r0, colBase = c0;
    if (r1 > r0 && rowN[0] == 0) ++r0;
    if (c1 > c0 && colN[0] == 0) ++c0;
    plan.data = CellRange{r0, c0, r1, c1, true};
    plan.blockCells = qint64(r1 - r0 + 1) * qint64(c1 - c0 + 1);
    auto put = [&](quint32 row, quint32 col, const QString &formula, bool percent, const CellRange &range) {
        QuickCell cell;
        cell.row = row;
        cell.col = col;
        cell.formula = formula;
        cell.percent = percent;
        cell.range = range;
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
    const bool percent = kind == QuickTotal::PercentOfTotal;
    const quint32 lastRow = jp::calc::kGridRows - 1, lastCol = jp::calc::kGridCols - 1;
    if (kind == QuickTotal::RunningTotal) {
        if (down ? c1 + 1 + (c1 - c0) > lastCol : r1 + 1 + (r1 - r0) > lastRow) {
            plan.problem = tr("There is no room next to the cells for the running totals.");
            return plan;
        }
        for (quint32 c = c0; c <= c1; ++c) {
            if (down && colN[c - colBase] == 0) continue;
            for (quint32 r = r0; r <= r1; ++r) {
                if (!down && rowN[r - rowBase] == 0) continue;
                if (down) put(r, c1 + 1 + (c - c0), QStringLiteral("SUM(%1:%2)").arg(refOf(r0, c, true, false), refOf(r, c)), false, CellRange{r0, c, r, c, true});
                else put(r1 + 1 + (r - r0), c, QStringLiteral("SUM(%1:%2)").arg(refOf(r, c0, false, true), refOf(r, c)), false, CellRange{r, c0, r, c, true});
            }
        }
    } else if (down) {
        if (r1 + 1 > lastRow) {
            plan.problem = tr("There is no room under the cells for the totals.");
            return plan;
        }
        for (quint32 c = c0; c <= c1; ++c)
            if (colN[c - colBase] > 0) put(r1 + 1, c, fn(blockOf(r0, c, r1, c)), percent, CellRange{r0, c, r1, c, true});
    } else {
        if (c1 + 1 > lastCol) {
            plan.problem = tr("There is no room beside the cells for the totals.");
            return plan;
        }
        for (quint32 r = r0; r <= r1; ++r)
            if (rowN[r - rowBase] > 0) put(r, c1 + 1, fn(blockOf(r, c0, r, c1)), percent, CellRange{r, c0, r, c1, true});
    }
    if (plan.cells.empty()) {
        plan.problem = tr("There are no numbers here to total.");
        return plan;
    }
    if (qint64(plan.cells.size()) > kMaxCellsPerEdit) {
        plan.problem = tr("That is more cells than JeffCrunch changes at once (%1). Select fewer cells.").arg(jp::systemLocale().toString(kMaxCellsPerEdit));
        return plan;
    }
    // (the cells the totals go in: none may be filled by another formula's spill, or be covered by a merge)
    {
        std::unordered_set<quint64> places;
        quint32 top = plan.cells.front().row, bottom = top, left = plan.cells.front().col, right = left;
        for (const QuickCell &cell : plan.cells) {
            if (m_book->calc->isSpillMember(sheet, cell.row, cell.col)) {
                plan.problem = tr("The totals would block a spilled result next to the cells. Move the spilled result, or select other cells.");
                return plan;
            }
            places.insert((quint64(cell.row) << 32) | cell.col);
            top = std::min(top, cell.row);
            bottom = std::max(bottom, cell.row);
            left = std::min(left, cell.col);
            right = std::max(right, cell.col);
        }
        const QVector<CellRange> merges = m_book->calc->mergesOf(sheet);
        for (const CellRange &m : merges) {
            if (!m.valid || m.row2 < top || m.row1 > bottom || m.col2 < left || m.col1 > right) continue;
            for (quint32 r = std::max(m.row1, top); r <= std::min(m.row2, bottom); ++r)
                for (quint32 c = std::max(m.col1, left); c <= std::min(m.col2, right); ++c)
                    if ((r != m.row1 || c != m.col1) && places.count((quint64(r) << 32) | c)) {
                        plan.problem = tr("Some of the cells where the totals go are part of merged cells. Unmerge them, or select other cells.");
                        return plan;
                    }
        }
    }
    for (const QuickCell &cell : plan.cells)
        if (!valueAt(sheet, cell.row, cell.col).isEmpty() || m_book->calc->content(sheet, cell.row, cell.col).isFormula) ++plan.overwrites;
    plan.ok = true;
    return plan;
}

EditSession::QuickLook EditSession::lookQuickTotals(int sheet, const QuickTotalsPlan &plan, QuickTotal kind, int most) const
{
    QuickLook look;
    if (!plan.ok || !m_book || !m_book->calc) return look;
    const size_t n = std::min<size_t>(plan.cells.size(), size_t(std::max(0, most)));
    // (what the look would read is counted first, from the plan: the whole block once for a share of the whole, and the range of each cell shown)
    qint64 need = kind == QuickTotal::PercentOfTotal ? plan.blockCells : 0;
    for (size_t i = 0; i < n; ++i) need += plan.cells[i].range.cellCount();
    if (need > kQuickLookReads) {
        look.tooMany = true;
        return look;
    }
    struct Sum {
        double sum = 0;
        qint64 count = 0;
        bool error = false;
        jp::calc::ErrorKind kind = jp::calc::ErrorKind::Value;
    };
    auto sumOf = [&](const CellRange &r) {
        Sum out;
        for (quint32 row = r.row1; row <= r.row2; ++row)
            for (quint32 col = r.col1; col <= r.col2; ++col) {
                ++m_quickReads;
                const jp::calc::Value v = valueAt(sheet, row, col);
                if (v.isNumber()) {
                    out.sum += v.number();
                    ++out.count;
                } else if (v.isError() && !out.error) {
                    out.error = true;
                    out.kind = v.errorKind();
                }
            }
        return out;
    };
    Sum whole;
    if (kind == QuickTotal::PercentOfTotal) whole = sumOf(plan.data);
    auto error = [](jp::calc::ErrorKind e) { return jp::calc::errorKindText(e); };
    for (size_t i = 0; i < n; ++i) {
        const QuickCell &cell = plan.cells[i];
        const Sum part = sumOf(cell.range);
        QString text;
        if (part.error) {
            text = error(part.kind);
        } else if (kind == QuickTotal::Count) {
            text = m_locale.toString(part.count);
        } else if (kind == QuickTotal::Average) {
            text = part.count == 0 ? error(jp::calc::ErrorKind::Div0) : m_locale.toString(part.sum / double(part.count), 'g', 10);
        } else if (kind == QuickTotal::PercentOfTotal) {
            if (whole.error) text = error(whole.kind);
            else if (whole.sum == 0) text = error(jp::calc::ErrorKind::Div0);
            else text = jp::systemLocale().toString(part.sum / whole.sum * 100, 'f', 1) + QLatin1Char('%');
        } else {
            text = m_locale.toString(part.sum, 'g', 10);
        }
        look.shown.push_back(text);
    }
    return look;
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
    if (m_previewSheet >= 0) return refuse(tr("The workbook is busy. Try again in a moment."));   // (the rule of the look before is still on the sheet: it is not taken for the sheet's own)
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
    m_previewTimer.stop();
    if (m_previewSheet < 0) return;
    const int sheet = m_previewSheet;
    std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
    if (!hold) {
        // (the rule stays, and is what condPreviewShown says, until it is really out: the next try is a moment on, a few times at most; a save tries again)
        if (++m_previewTries <= kMostPreviewTries) m_previewTimer.start();
        return;
    }
    m_previewSheet = -1;
    m_previewTries = 0;
    hold->calculation()->replaceConditionalFormats(sheet, m_previewBefore);
    if (m_previewDxf >= 0) m_book->book->removeAddedDxf(m_previewDxf);
    hold.reset();
    m_previewBefore.clear();
    m_previewDxf = -1;
    Q_EMIT conditionalFormatsChanged(sheet);
}

} // namespace jp::crunch
