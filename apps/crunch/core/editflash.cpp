// Flash Fill: fills a column from the pattern of a few cells typed by hand (core/flashfill.h).
#include "core/editsession.h"

#include "core/flashfill.h"
#include "sheet/cellref.h"

#include <algorithm>

namespace jp::crunch {

using namespace jp::office;
using namespace jp::office::sheet;
using jp::calc::Value;

namespace {

bool sameMeaning(const Calculation::CellContent &a, const Calculation::CellContent &b)
{
    return a.exists == b.exists && a.isFormula == b.isFormula && a.formula == b.formula && a.value == b.value;
}

} // namespace

EditSession::Outcome EditSession::flashFill(int sheet, quint32 row, quint32 col, int *filled)
{
    Outcome out;
    if (filled) *filled = 0;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    const Calculation *calc = m_book->calc.get();
    // the neighbors: the columns to the left, nearest first, then those to the right
    QVector<quint32> neighbors;
    for (qint64 c = qint64(col) - 1; c >= 0 && neighbors.size() < kFlashFillMaxSources / 2; --c) neighbors << quint32(c);
    for (quint32 c = col + 1; c < quint32(jp::calc::kGridCols) && neighbors.size() < kFlashFillMaxSources; ++c) {
        if (c > col + kFlashFillMaxSources / 2) break;
        neighbors << c;
    }
    const auto sourcesOf = [&](quint32 r) {
        QStringList list;
        bool any = false;
        for (const quint32 c : std::as_const(neighbors)) {
            const QString t = displayText(sheet, r, c);
            any = any || !t.isEmpty();
            list << t;
        }
        return any ? list : QStringList();
    };
    // the rows: those round `row` that have something beside them
    if (sourcesOf(row).isEmpty()) {
        out.ok = false;
        out.problem = tr("Flash Fill needs cells beside the column to read. Put the example next to the cells it comes from, and try again.");
        return out;
    }
    quint32 first = row;
    quint32 last = row;
    while (first > 0 && !sourcesOf(first - 1).isEmpty() && row - first < quint32(kMaxCellsPerEdit)) --first;
    while (last + 1 < quint32(jp::calc::kGridRows) && !sourcesOf(last + 1).isEmpty() && last - row < quint32(kMaxCellsPerEdit)) ++last;
    if (qint64(last) - first + 1 > kMaxCellsPerEdit) {
        out.ok = false;
        out.problem = tr("That is more cells than JeffCrunch changes at once (%1). Select fewer cells.").arg(QLocale().toString(kMaxCellsPerEdit));
        return out;
    }
    // the examples: what is typed in the column, as text or a number (not a formula)
    QVector<FlashFillExample> examples;
    QVector<quint32> exampleRows;
    QVector<quint32> empty;
    for (quint32 r = first; r <= last; ++r) {
        const Calculation::CellContent c = calc->content(sheet, r, col);
        if (c.exists && (c.isFormula || (!c.value.isEmpty() && !c.value.isText() && !c.value.isNumber()))) continue;
        if (!c.exists || c.value.isEmpty()) {
            empty << r;
            continue;
        }
        FlashFillExample e;
        e.sources = sourcesOf(r);
        e.output = displayText(sheet, r, col);
        if (e.sources.isEmpty() || e.output.isEmpty()) continue;
        examples << e;
        exampleRows << r;
    }
    if (examples.isEmpty()) {
        out.ok = false;
        out.problem = tr("Type what the first cell should hold, by hand, and use Flash Fill again.");
        return out;
    }
    if (empty.isEmpty()) {
        out.ok = false;
        out.problem = tr("There are no empty cells to fill in this column.");
        return out;
    }
    FlashFillPattern pattern = learnFlashFill(examples);
    if (!pattern.isValid() && examples.size() > 1) {
        // (the top cell may be a heading, which the others do not follow)
        examples.removeFirst();
        pattern = learnFlashFill(examples);
    }
    if (!pattern.isValid()) {
        out.ok = false;
        out.problem = tr("Flash Fill could not find a pattern in the cells you filled in. Fill in one or two more by hand, and try again.");
        return out;
    }
    const bool hasShared = !m_book->book->sharedStringsPart().isEmpty();
    Step step;
    for (const quint32 r : std::as_const(empty)) {
        const QString text = pattern.apply(sourcesOf(r));
        if (text.isEmpty()) continue;
        if (!canEditCell(sheet, r, col, &why)) continue;
        if (m_book->book->tableHeaderAt(sheet, r, col, nullptr) >= 0) continue;
        CellChange ch;
        ch.sheet = sheet;
        ch.row = r;
        ch.col = quint16(col);
        ch.before = calc->content(sheet, r, col);
        ch.after = ch.before;
        ch.after.isFormula = false;
        ch.after.formula.clear();
        ch.after.hasStored = false;
        ch.after.stored = Value();
        ch.after.sharedString = -1;
        ch.after.exists = true;
        ch.after.value = Value::text(text);
        ch.textToShare = hasShared;
        if (!sameMeaning(ch.before, ch.after)) step.cells.push_back(std::move(ch));
    }
    if (step.empty()) {
        out.ok = false;
        out.problem = tr("Flash Fill found a pattern, but the other rows have nothing it can read.");
        return out;
    }
    if (filled) *filled = int(step.cells.size());
    pushStep(std::move(step), tr("Flash Fill"));
    return out;
}

} // namespace jp::crunch
