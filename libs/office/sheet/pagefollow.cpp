#include "sheet/pagefollow.h"

#include <algorithm>

namespace jp::office::sheet {

namespace {

// The breaks of one direction after rows (or columns) are inserted or deleted on the sheet.
void followBreaks(QVector<PageBreak> *breaks, const StructEdit &edit, bool rows, quint32 limit)
{
    using K = StructEdit::Kind;
    const bool inserts = edit.kind == (rows ? K::InsertRows : K::InsertColumns), deletes = edit.kind == (rows ? K::DeleteRows : K::DeleteColumns);
    if (!inserts && !deletes) return;
    QVector<PageBreak> out;
    for (PageBreak b : *breaks) {
        if (inserts) {
            if (b.before >= edit.first) b.before += edit.count;
            if (b.before >= limit) continue;   // (pushed off the sheet)
        } else {
            if (b.before >= edit.first + edit.count) b.before -= edit.count;
            else if (b.before > edit.first) continue;   // (the row it was before is gone)
        }
        // (two breaks can come to one place: the one before the first row deleted and the one after the last; the place has one break, set by hand if either was)
        bool merged = false;
        for (PageBreak &e : out)
            if (e.before == b.before) {
                e.manual = e.manual || b.manual;
                merged = true;
                break;
            }
        if (!merged) out.append(b);
    }
    *breaks = out;
}

} // namespace

void followStructEdit(PageSetup *setup, const StructEdit &edit, int ownSheet, const QStringList &sheetNames)
{
    if (ownSheet < 0 || ownSheet >= sheetNames.size()) return;
    if (edit.sheet == ownSheet) {
        followBreaks(&setup->rowBreaks, edit, true, kMaxRows);
        followBreaks(&setup->colBreaks, edit, false, kMaxColumns);
    }
    // the print area and the titles are names: the rewriting a name's text gets
    const QString name = sheetNames[ownSheet];
    if (setup->hasPrintArea()) {
        const QString text = printAreaText(*setup, name);
        const QString now = adjustFormula(text, ownSheet, sheetNames, edit, nullptr, true);
        if (now != text) readPrintAreaText(now, name, setup);
    }
    if (setup->titles.hasRows || setup->titles.hasCols) {
        const QString text = printTitlesText(*setup, name);
        const QString now = adjustFormula(text, ownSheet, sheetNames, edit, nullptr, true);
        if (now != text) readPrintTitlesText(now, name, setup);
    }
}

void followStructEdits(PageSetup *setup, const std::vector<StructEdit> &edits, int ownSheet, const QStringList &sheetNames)
{
    for (const StructEdit &e : edits) followStructEdit(setup, e, ownSheet, sheetNames);
}

} // namespace jp::office::sheet
