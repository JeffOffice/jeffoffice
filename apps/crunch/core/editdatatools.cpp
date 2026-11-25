// The data tool of the Data tab that cuts the text of a column into several: Text to Columns, one step of Undo. (Remove Duplicates is the shared engine of edittables.cpp.)
#include "core/editsession.h"

#include "calc/value.h"

#include <QCoreApplication>
#include <QDate>
#include <QLocale>
#include <QRegularExpression>
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

namespace {

// The month a word names (1 to 12), in English or in the language of the program: a whole name, or a short form of at least three letters (Mar, Sept); 0 for any other word.
int monthNamed(const QString &word)
{
    if (word.size() < 3) return 0;
    for (const QLocale &locale : {QLocale(QLocale::English), QLocale()})
        for (int m = 1; m <= 12; ++m)
            if (locale.monthName(m, QLocale::LongFormat).startsWith(word, Qt::CaseInsensitive) || locale.monthName(m, QLocale::ShortFormat).startsWith(word, Qt::CaseInsensitive)) return m;
    return 0;
}

// The text of a date written in the order `order` (3 month, day, year; 4 day, month, year; 5 year, month, day) as the year, month, and day with dashes (and the time after it, when the text
// has one), which a cell reads as that date. The month may be a name, and the separators are / - . comma and space. Text that is not a date in that order (not three parts, a month or a day
// that does not exist, a year that is not of two or four digits, or a time after it that does not read) says so by returning false: the caller keeps it as text, never reads it in another order.
// A year of two digits is of this century up to 29, else of the last.
bool dateInOrder(const QString &text, int order, QString *out)
{
    QString rest = text.trimmed();
    QString time;
    QStringList words = rest.split(QRegularExpression(QStringLiteral("\\s+")));
    if (words.size() > 1 && (words.last().compare(QLatin1String("AM"), Qt::CaseInsensitive) == 0 || words.last().compare(QLatin1String("PM"), Qt::CaseInsensitive) == 0) && words[words.size() - 2].contains(QLatin1Char(':'))) {
        time = words.takeLast();
        time = words.takeLast() + QLatin1Char(' ') + time;
    } else if (words.size() > 1 && words.last().contains(QLatin1Char(':'))) {
        time = words.takeLast();
    }
    QString clock;
    if (!time.isEmpty()) {
        static const QRegularExpression timeShape(QStringLiteral("^(\\d{1,2}):(\\d{2})(?::(\\d{2}))?(?:\\s*([AaPp][Mm]))?$"));
        const QRegularExpressionMatch m = timeShape.match(time);
        if (!m.hasMatch()) return false;
        int h = m.captured(1).toInt();
        const int min = m.captured(2).toInt(), sec = m.captured(3).toInt();
        if (!m.captured(4).isEmpty()) {
            if (h < 1 || h > 12) return false;
            h = h % 12 + (m.captured(4).startsWith(QLatin1Char('P'), Qt::CaseInsensitive) ? 12 : 0);
        }
        if (h > 23 || min > 59 || sec > 59) return false;
        clock = QStringLiteral(" %1:%2:%3").arg(h, 2, 10, QLatin1Char('0')).arg(min, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0'));
    }
    static const QRegularExpression separators(QStringLiteral("[-/.,\\s]+"));
    QStringList parts = words.join(QLatin1Char(' ')).split(separators, Qt::SkipEmptyParts);
    if (parts.size() != 3) return false;
    static const QRegularExpression digits(QStringLiteral("^\\d+$"));
    const int monthAt = order == 3 ? 0 : 1;
    int v[3] = {0, 0, 0};
    for (int i = 0; i < 3; ++i) {
        if (i == monthAt && !digits.match(parts[i]).hasMatch()) {
            v[i] = monthNamed(parts[i]);
            if (v[i] == 0) return false;
        } else if (digits.match(parts[i]).hasMatch() && parts[i].size() <= 4) {
            v[i] = parts[i].toInt();
        } else {
            return false;
        }
    }
    const int yearAt = order == 5 ? 0 : 2;
    if (parts[yearAt].size() != 2 && parts[yearAt].size() != 4) return false;
    if (parts[monthAt].size() > 2 && digits.match(parts[monthAt]).hasMatch()) return false;
    int y = v[yearAt], m = v[monthAt], d = v[3 - yearAt - monthAt];
    if (y < 100) y += y < 30 ? 2000 : 1900;
    const QDate date(y, m, d);
    if (!date.isValid()) return false;
    *out = date.toString(Qt::ISODate) + clock;
    return true;
}

} // namespace

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
            else if (type >= 3 && type <= 5 && !text.isEmpty()) {
                QString date;
                // (not a date in the order asked for: kept as text exactly as typed, never read in another order)
                if (dateInOrder(text, type, &date)) text = date;
                else text.prepend(QLatin1Char('\''));
            }
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
