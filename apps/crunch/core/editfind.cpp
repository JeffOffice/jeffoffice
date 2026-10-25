// Find and Replace: the text a cell shows, and Replace as one step. See EditSession::replaceInCells; the search itself is FindRun.
#include "core/editsession.h"

#include "calc/numfmt.h"
#include "sheet/cellref.h"

#include <set>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::Value;

QString EditSession::displayText(int sheet, quint32 row, quint32 col) const
{
    if (!m_book || !m_book->calc || !m_book->book || sheet < 0 || sheet >= m_book->calc->sheetCount()) return {};
    const Cell *c = m_book->calc->sheet(sheet).cells.find(row, col);
    if (!c) return {};
    const Value v = valueAt(sheet, row, col);
    switch (v.kind()) {
    case Value::Kind::Empty: return {};
    case Value::Kind::Text: return v.text();
    case Value::Kind::Bool: return v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    case Value::Kind::Error: return jp::calc::errorKindText(v.errorKind());
    case Value::Kind::Array: return {};
    case Value::Kind::Number: break;
    }
    const jp::calc::NumberFormat format(m_book->book->styles().formatCode(c->style, QLocale::c()));
    const jp::calc::NumberSymbols symbols = jp::calc::NumberSymbols::of(m_locale);
    const jp::calc::Formatted f = format.format(v.number(), m_book->book->info().dateSystem, 0, &symbols);
    return f.failed ? jp::calc::numberToText(v.number()) : f.text;
}

EditSession::Outcome EditSession::replaceInCells(const std::vector<CellSpot> &cells, const TextPattern &pattern, const QString &with, ReplaceResult *result)
{
    Outcome out;
    if (result) *result = ReplaceResult();
    QString why;
    auto refuse = [&](const QString &problem) {
        out.ok = false;
        out.problem = problem;
        return out;
    };
    if (!canEdit(&why)) return refuse(why);
    const TextMatcher matcher(pattern);
    if (!matcher.valid()) return refuse(tr("Type the text to look for."));
    if (qint64(cells.size()) > kMaxCellsPerEdit)
        return refuse(tr("That is more cells than JeffCrunch changes at once (%1). Narrow the search, and replace again.").arg(QLocale().toString(kMaxCellsPerEdit)));
    const Calculation *calc = m_book->calc.get();
    Step step;
    int occurrences = 0;
    std::set<int> checked;
    for (const CellSpot &spot : cells) {
        if (checked.insert(spot.sheet).second && !canEditSheet(spot.sheet, &why)) return refuse(why);
        const QString where = cellName(spot.row, spot.col);
        const Calculation::CellContent before = calc->content(spot.sheet, spot.row, spot.col);
        if (!before.exists) continue;
        const QString text = editText(spot.sheet, spot.row, spot.col);
        int n = 0;
        const QString now = matcher.replaced(text, with, &n);
        if (n == 0) continue;
        if (!before.editable) return refuse(tr("%1 is part of an array or a data table, so it can't be changed one cell at a time. Nothing was replaced.").arg(where));
        if (isLocked(spot.sheet, before.style)) return refuse(tr("%1 is locked, and its sheet is protected, so it can't be changed. Nothing was replaced.").arg(where));
        if (m_book->book->tableHeaderAt(spot.sheet, spot.row, spot.col, nullptr) >= 0)
            return refuse(tr("%1 is in the first row of a table, which holds the names of its columns. Nothing was replaced.").arg(where));
        CellChange ch;
        bool none = false;
        Outcome problem;
        if (!typedChange(spot.sheet, spot.row, spot.col, now, &ch, &none, &problem))
            return refuse(tr("Replacing in %1 would leave \"%2\", which is not something a cell can hold: %3 Nothing was replaced.").arg(where, now.left(60), problem.problem));
        occurrences += n;
        if (none) continue;
        step.cells.push_back(std::move(ch));
    }
    if (result) {
        result->cells = int(step.cells.size());
        result->occurrences = occurrences;
    }
    if (step.cells.empty()) return out;
    pushStep(std::move(step), tr("Replacing"));
    return out;
}

} // namespace jp::crunch
