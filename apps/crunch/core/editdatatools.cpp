// The data tools of the Data tab that change cells: Remove Duplicates, which takes out the rows that repeat an earlier row and moves the rows under them up, and Text to Columns, which cuts the
// text of a column into several. Each is one step of Undo (a group of steps of the stack for the first, which moves a block of rows at a time).
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

namespace {

// A text that stands for a cell's value in a comparison of rows: the same for values that are equal as a spreadsheet counts them (a text without regard to case).
QString duplicateKeyOf(const Value &v)
{
    switch (v.kind()) {
    case Value::Kind::Empty: return QStringLiteral("e");
    case Value::Kind::Number: return QStringLiteral("n") + jp::calc::numberToText(v.number());
    case Value::Kind::Text: return QStringLiteral("t") + v.text().toCaseFolded();
    case Value::Kind::Bool: return v.boolean() ? QStringLiteral("bT") : QStringLiteral("bF");
    case Value::Kind::Error: return QStringLiteral("x") + jp::calc::errorKindText(v.errorKind());
    case Value::Kind::Array: return QStringLiteral("a");
    }
    return QString();
}

} // namespace

EditSession::Outcome EditSession::removeDuplicates(int sheet, const CellRange &range, const std::vector<int> &columns, bool hasHeader, qint64 *removed)
{
    Outcome out;
    if (removed) *removed = 0;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) return refuse(why);
    if (!range.valid) return refuse(tr("Select the cells to look for repeated rows in."));
    if (columns.empty()) return refuse(tr("Choose at least one column to compare."));
    const quint32 width = range.col2 - range.col1 + 1;
    for (const int c : columns)
        if (c < 0 || quint32(c) >= width) return refuse(tr("A column to compare is not in the cells selected."));
    if (range.cellCount() > kMaxCellsPerEdit) return refuse(tr("There are too many cells to look at at once. Select fewer."));
    const quint32 first = range.row1 + (hasHeader ? 1 : 0);
    if (first > range.row2) return refuse(tr("There are no rows of data under the header."));
    // which rows stay
    QSet<QString> seen;
    std::vector<char> keep(size_t(range.row2 - first + 1), 1);
    qint64 gone = 0;
    for (quint32 row = first; row <= range.row2; ++row) {
        QString key;
        for (const int c : columns) {
            key += duplicateKeyOf(valueAt(sheet, row, range.col1 + quint32(c)));
            key += QChar(0x1F);
        }
        if (seen.contains(key)) {
            keep[size_t(row - first)] = 0;
            ++gone;
        } else seen.insert(key);
    }
    if (gone == 0) return out;
    // the rows that stay and have rows taken out above them move up as blocks, from the top
    m_undo.beginMacro(tr("Removing duplicates"));
    quint32 shift = 0;
    quint32 row = first;
    bool failed = false;
    while (row <= range.row2 && !failed) {
        if (!keep[size_t(row - first)]) {
            ++shift;
            ++row;
            continue;
        }
        const quint32 start = row;
        while (row <= range.row2 && keep[size_t(row - first)]) ++row;
        if (shift == 0) continue;
        CellRange block = range;
        block.row1 = start;
        block.row2 = row - 1;
        const Outcome moved = moveCells(sheet, block, sheet, start - shift, range.col1);
        if (!moved.ok) {
            out = moved;
            failed = true;
        }
    }
    if (!failed) {
        // the rows at the bottom of the range that no longer hold a row
        const quint32 newLast = range.row2 - quint32(gone);
        const Outcome cleared = clear(sheet, qint64(newLast) + 1, range.col1, range.row2, range.col2);
        if (!cleared.ok) {
            out = cleared;
            failed = true;
        }
    }
    m_undo.endMacro();
    if (failed) {
        m_undo.undo();
        if (out.ok) out.ok = false;
        return out;
    }
    if (removed) *removed = gone;
    return out;
}

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
