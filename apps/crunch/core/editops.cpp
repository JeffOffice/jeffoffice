// What a person does with a block of cells besides typing in it: AutoSum, fill, the clipboard. Each makes the changes for one step of the undo stack.
#include "core/editsession.h"

#include "sheet/cellref.h"

#include <algorithm>

namespace jp::crunch {

using namespace jp::office;
using namespace jp::office::sheet;
using jp::calc::Value;

Value EditSession::valueAt(int sheet, quint32 row, quint32 col) const
{
    if (!m_book || !m_book->calc || sheet < 0 || sheet >= m_book->calc->sheetCount()) return Value();
    const Sheet &s = m_book->calc->sheet(sheet);
    const Cell *c = s.cells.find(row, col);
    if (!c) return Value();
    Value v;
    if ((c->hasFormula() || (c->flags & ArrayMember)) && m_values->find(sheet, row, col, &v)) return v;
    switch (c->type) {
    case CellType::Empty: return Value();
    case CellType::Number:
    case CellType::Date: return Value::number(c->number);
    case CellType::SharedString: return Value::text(m_book->book->sharedStrings().at(c->textIndex()));
    case CellType::InlineString:
    case CellType::FormulaString: return Value::text(s.texts.value(c->textIndex()));
    case CellType::Bool: return Value::boolean(c->number != 0);
    case CellType::Error: return Value::error(jp::calc::ErrorKind(std::max(0, std::min(int(jp::calc::ErrorKind::Unknown), int(c->number)))));
    }
    return Value();
}

namespace {

QString rangeText(quint32 row1, quint32 col1, quint32 row2, quint32 col2)
{
    return row1 == row2 && col1 == col2 ? cellName(row1, col1) : cellName(row1, col1) + QLatin1Char(':') + cellName(row2, col2);
}

} // namespace

EditSession::AutoSumResult EditSession::autoSum(int sheet, const CellRange &sel, const QString &function)
{
    AutoSumResult out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.outcome.ok = false;
        out.outcome.problem = why;
        return out;
    }
    auto isNumber = [&](quint32 r, quint32 c) { return valueAt(sheet, r, c).isNumber(); };
    auto isEmptyCell = [&](quint32 r, quint32 c) { return valueAt(sheet, r, c).isEmpty() && !m_book->calc->content(sheet, r, c).isFormula; };
    // the end of the data of a sheet that is selected whole is the end of the cells
    quint32 usedRows = 0, usedCols = 0;
    {
        const Sheet &s = m_book->calc->sheet(sheet);
        usedRows = s.extentRows;
        usedCols = s.extentCols;
    }
    const quint32 row1 = sel.row1, col1 = sel.col1;
    // (a selection of whole columns or rows reaches as far as the cells do)
    const quint32 lastRow = sel.row2 >= jp::calc::kGridRows - 1 ? std::min<quint32>(sel.row2, std::max<quint32>(usedRows, row1 + 1) - 1) : sel.row2;
    const quint32 lastCol = sel.col2 >= jp::calc::kGridCols - 1 ? std::min<quint32>(sel.col2, std::max<quint32>(usedCols, col1 + 1) - 1) : sel.col2;
    const bool oneCell = sel.row1 == sel.row2 && sel.col1 == sel.col2;
    auto formula = [&](quint32 r1, quint32 c1, quint32 r2, quint32 c2) { return function + QLatin1Char('(') + rangeText(r1, c1, r2, c2) + QLatin1Char(')'); };
    if (oneCell) {
        const quint32 row = sel.row1, col = sel.col1;
        QString text;
        int back = 1;
        if (row > 0 && isNumber(row - 1, col)) {
            quint32 top = row - 1;
            while (top > 0 && isNumber(top - 1, col)) --top;
            text = formula(top, col, row - 1, col);
            back = 0;
        } else if (col > 0 && isNumber(row, col - 1)) {
            quint32 left = col - 1;
            while (left > 0 && isNumber(row, left - 1)) --left;
            text = formula(row, left, row, col - 1);
            back = 0;
        } else {
            text = function + QStringLiteral("()");
        }
        QString problem;
        if (!canEditCell(sheet, row, col, &problem)) {
            out.outcome.ok = false;
            out.outcome.problem = problem;
            return out;
        }
        out.kind = AutoSumResult::Kind::Edit;
        out.row = row;
        out.col = col;
        out.text = QLatin1Char('=') + formulaToLocale(text, m_locale);
        out.cursorBack = back ? back : 0;
        return out;
    }
    // a block: the totals go under it, or right of it when it is one row
    Step step;
    auto put = [&](quint32 r, quint32 c, const QString &text) {
        QString problem;
        if (!canEditCell(sheet, r, c, &problem)) {
            out.outcome.ok = false;
            out.outcome.problem = problem;
            return false;
        }
        CellChange ch;
        bool none = false;
        if (!typedChange(sheet, r, c, QLatin1Char('=') + formulaToLocale(text, m_locale), &ch, &none, &out.outcome)) return false;
        if (!none) step.cells.push_back(std::move(ch));
        return true;
    };
    if (sel.row2 > sel.row1) {
        // the last row of the selection is where the totals go when it is empty; else under it
        bool emptyLast = true;
        for (quint32 c = col1; c <= lastCol && emptyLast; ++c) emptyLast = isEmptyCell(lastRow, c);
        const quint32 totals = emptyLast ? lastRow : lastRow + 1;
        if (totals > row1) {
            for (quint32 c = col1; c <= lastCol; ++c) {
                bool any = false;
                for (quint32 r = row1; r < totals && !any; ++r) any = isNumber(r, c);
                if (any && !put(totals, c, formula(row1, c, totals - 1, c))) return out;
            }
        }
    } else {
        bool emptyLast = isEmptyCell(row1, lastCol);
        const quint32 totals = emptyLast ? lastCol : lastCol + 1;
        if (totals > col1) {
            bool any = false;
            for (quint32 c = col1; c < totals && !any; ++c) any = isNumber(row1, c);
            if (any && !put(row1, totals, formula(row1, col1, row1, totals - 1))) return out;
        }
    }
    if (step.cells.empty()) return out;
    if (qint64(step.cells.size()) > kMaxCellsPerEdit) {
        out.outcome.ok = false;
        out.outcome.problem = tr("That is more cells than JeffCrunch changes at once (%1). Select fewer cells.").arg(QLocale().toString(kMaxCellsPerEdit));
        return out;
    }
    out.kind = AutoSumResult::Kind::Done;
    pushStep(std::move(step), tr("AutoSum"));
    return out;
}

} // namespace jp::crunch
