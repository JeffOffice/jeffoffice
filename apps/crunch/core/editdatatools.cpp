// The data tool of the Data tab that cuts the text of a column into several: Text to Columns, one step of Undo. (Remove Duplicates is the shared engine of edittables.cpp.)
#include "core/editsession.h"

#include "calc/value.h"

#include <QCoreApplication>
#include <QHash>
#include <QSet>
#include <QUndoStack>

#include <algorithm>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::Value;

QVector<QStringList> EditSession::splitText(const QStringList &lines, const SplitSpec &spec)
{
    QVector<QStringList> table;
    table.reserve(int(lines.size()));
    for (const QString &line : lines) {
        QStringList pieces;
        if (spec.fixedWidth) {
            std::vector<int> cuts = spec.breaks;
            std::sort(cuts.begin(), cuts.end());
            int from = 0;
            for (const int cut : cuts) {
                const int at = std::clamp(cut - 1, 0, int(line.size()));
                if (at <= from) continue;
                pieces << line.mid(from, at - from);
                from = at;
            }
            if (from < line.size() || pieces.isEmpty()) pieces << line.mid(from);
        } else {
            QString piece;
            bool quoted = false, lastWasDelimiter = false;
            const bool hasQualifier = !spec.qualifier.isNull();
            for (int i = 0; i < line.size(); ++i) {
                const QChar ch = line[i];
                if (hasQualifier && ch == spec.qualifier) {
                    if (quoted && i + 1 < line.size() && line[i + 1] == spec.qualifier) {
                        piece += ch;   // (a doubled qualifier inside is one)
                        ++i;
                    } else quoted = !quoted;
                    lastWasDelimiter = false;
                    continue;
                }
                if (!quoted && spec.delimiters.contains(ch)) {
                    if (!(spec.consecutiveAsOne && lastWasDelimiter)) pieces << piece;
                    if (pieces.size() > jp::calc::kGridCols) break;   // (more pieces than the sheet has columns: the caller says so)
                    piece.clear();
                    lastWasDelimiter = true;
                    continue;
                }
                piece += ch;
                lastWasDelimiter = false;
            }
            pieces << piece;
        }
        table.append(pieces);
    }
    return table;
}

EditSession::Outcome EditSession::textToColumns(int sheet, const CellRange &source, const SplitSpec &spec, quint32 destRow, quint32 destCol, CellRange *covered)
{
    Outcome out;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) return refuse(why);
    if (!source.valid || source.col1 != source.col2) return refuse(tr("Select cells in one column to split."));
    if (source.cellCount() > kMaxCellsPerEdit) return refuse(tr("There are too many cells to split at once. Select fewer."));
    if (!spec.fixedWidth && spec.delimiters.isEmpty()) return refuse(tr("Choose what to split the text at."));
    QStringList lines;
    for (quint32 row = source.row1; row <= source.row2; ++row) {
        const Value v = valueAt(sheet, row, source.col1);
        // (what the cell shows as text: a number as it is edited, a formula's result by its value)
        lines << (v.isEmpty() ? QString() : v.isText() ? v.text() : editText(sheet, row, source.col1).startsWith(QLatin1Char('=')) ? jp::calc::numberToText(v.number()) : editText(sheet, row, source.col1));
    }
    // (line by line, so that a line of very many pieces is met before the next ones are cut: none may make a block past what a step may hold many times over)
    QVector<QStringList> pieces;
    int columns = 0;
    for (const QString &line : lines) {
        pieces.append(splitText({line}, spec).value(0));
        columns = std::max<int>(columns, int(pieces.last().size()));
        if (qint64(columns) * lines.size() > 16 * kMaxCellsPerEdit) return refuse(tr("The pieces would fill too many cells at once. Select fewer cells, or split at fewer places."));
    }
    // how many columns are kept, before any row is made as wide as that
    int kept = 0;
    for (int c = 0; c < columns; ++c)
        if (!(c < int(spec.types.size()) && spec.types[size_t(c)] == 2)) ++kept;
    if (kept == 0) return refuse(tr("Every column was left out, so there is nothing to put in the sheet."));
    if (qint64(destCol) + kept > jp::calc::kGridCols) return refuse(tr("The pieces would go past the last column of the sheet."));
    if (qint64(kept) * pieces.size() > kMaxCellsPerEdit) return refuse(tr("The pieces would fill too many cells at once. Select fewer cells, or split at fewer places."));
    // the columns that are not imported are left out, and a column of text is marked as text
    QVector<QStringList> table;
    table.reserve(pieces.size());
    for (const QStringList &p : pieces) {
        QStringList row;
        for (int c = 0; c < columns; ++c) {
            const int type = c < int(spec.types.size()) ? spec.types[size_t(c)] : 0;
            if (type == 2) continue;
            QString text = c < p.size() ? p[c] : QString();
            if (type == 1 && !text.isEmpty()) text.prepend(QLatin1Char('\''));
            row << text;
        }
        table.append(row);
    }
    CellRange target;
    target.valid = true;
    target.row1 = destRow;
    target.row2 = destRow + quint32(table.size()) - 1;
    target.col1 = destCol;
    target.col2 = destCol + quint32(kept) - 1;
    PasteOptions options;
    Outcome pasted = pasteText(sheet, target, table, options);
    if (covered) *covered = target;
    return pasted;
}

} // namespace jp::crunch
