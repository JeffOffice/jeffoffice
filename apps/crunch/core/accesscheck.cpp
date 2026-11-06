#include "core/accesscheck.h"

#include "core/sheetobjects.h"
#include "sheet/calculation.h"
#include "sheet/cellref.h"
#include "sheet/tables.h"

#include <QCoreApplication>
#include <QSet>
#include <cmath>
#include <QRegularExpression>

namespace jp::crunch {

using namespace jp::office::sheet;

namespace {

// The most cells looked at for the colors of their text (a sheet of millions of cells is not walked to the end).
constexpr qint64 kContrastWork = 3000000;

double lightness(quint32 rgb)
{
    auto channel = [](quint32 v) {
        const double c = double(v) / 255.0;
        return c <= 0.03928 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * channel((rgb >> 16) & 0xFF) + 0.7152 * channel((rgb >> 8) & 0xFF) + 0.0722 * channel(rgb & 0xFF);
}

// How much the text stands out from the fill, from 1 (not at all) to 21 (black on white).
double contrastRatio(quint32 text, quint32 fill)
{
    const double a = lightness(text), b = lightness(fill);
    return (std::max(a, b) + 0.05) / (std::min(a, b) + 0.05);
}

} // namespace

int firstObjectOutOfReadingOrder(const std::vector<std::pair<quint32, quint32>> &at)
{
    for (size_t i = 1; i < at.size(); ++i)
        if (at[i] < at[i - 1]) return int(i);
    return -1;
}

std::vector<AccessIssue> checkAccessibility(const EditSession &session, SheetObjects *objects, int most)
{
    std::vector<AccessIssue> out;
    const Workbook *book = session.workbook();
    const Calculation *calc = session.calculation();
    if (!book) return out;
    const char *ctx = "jp::crunch::AccessCheck";
    auto add = [&](AccessIssue::Kind kind, AccessIssue::Level level, int sheet, quint32 row, quint32 col, const QString &message) {
        if (int(out.size()) >= most) return;
        AccessIssue i;
        i.kind = kind;
        i.level = level;
        i.sheet = sheet;
        i.row = row;
        i.col = col;
        i.message = message;
        out.push_back(i);
    };
    static const QRegularExpression defaultName(QStringLiteral("^Sheet[0-9]+$"));
    const auto &sheets = book->info().sheets;
    for (int s = 0; s < int(sheets.size()) && int(out.size()) < most; ++s) {
        if (sheets[size_t(s)].deleted) continue;
        const QString name = sheets[size_t(s)].name;
        if (defaultName.match(name).hasMatch())
            add(AccessIssue::DefaultSheetName, AccessIssue::Tip, s, 0, 0, QCoreApplication::translate(ctx, "The sheet \"%1\" has the name it was given. A name that says what is on the sheet helps a reader find it.").arg(name));
        if (objects)
            for (const SheetObjects::ObjectInfo &o : objects->describe(s)) {
                using K = jp::office::sheet::ObjectKind;
                if (o.kind != K::Chart && o.kind != K::Picture) continue;
                if (!o.description.trimmed().isEmpty()) continue;
                add(AccessIssue::MissingAltText, AccessIssue::Error, s, o.row, o.col,
                    (o.kind == K::Chart ? QCoreApplication::translate(ctx, "The chart \"%1\" on %2 has no alt text, so a screen reader can't say what it shows.")
                                        : QCoreApplication::translate(ctx, "The picture \"%1\" on %2 has no alt text, so a screen reader can't say what it shows."))
                        .arg(o.name.isEmpty() ? QCoreApplication::translate(ctx, "(no name)") : o.name, name));
            }
        if (calc && s < calc->sheetCount())
            for (const jp::office::sheet::CellRange &m : calc->mergesOf(s))
                add(AccessIssue::MergedCells, AccessIssue::Warning, s, m.row1, m.col1,
                    QCoreApplication::translate(ctx, "The cells %1 on %2 are merged. Merged cells are hard to move through with the keyboard and a screen reader.").arg(rangeName(m), name));
    }
    // Text that is hard to read against its fill: once for each look (a font and a fill) the text of cells has, at the first cell that has it. A patterned fill is not judged.
    if (calc) {
        QSet<quint64> seen;
        qint64 work = 0;
        for (int s = 0; s < int(sheets.size()) && s < calc->sheetCount() && int(out.size()) < most && work < kContrastWork; ++s) {
            if (sheets[size_t(s)].deleted) continue;
            calc->sheet(s).cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
                if (++work > kContrastWork || int(out.size()) >= most || (c.type == CellType::Empty && !c.hasFormula())) return;
                const CellFormat &f = book->styles().format(c.style);
                const quint64 key = (quint64(f.font) << 32) | f.fill;
                if (seen.contains(key)) return;
                seen.insert(key);
                const FillSpec &fill = book->styles().fill(f.fill);
                if (fill.pattern != PatternKind::None && fill.pattern != PatternKind::Solid) return;
                const FontSpec &font = book->styles().font(f.font);
                const quint32 text = book->styles().resolve(font.color, 0x000000);
                const quint32 back = fill.pattern == PatternKind::Solid ? book->styles().resolve(fill.fg, 0xFFFFFF) : 0xFFFFFF;
                const bool large = font.size >= 18 || (font.bold && font.size >= 14);
                const double ratio = contrastRatio(text, back);
                if (ratio >= (large ? 3.0 : 4.5)) return;
                add(AccessIssue::LowContrast, AccessIssue::Warning, s, row, col,
                    QCoreApplication::translate(ctx, "The text in %1 on %2 is hard to read against the color behind it. Use a darker text or a lighter fill, or the other way round.").arg(cellName(row, col), sheets[size_t(s)].name));
            });
        }
    }
    // Blank rows and columns between cells that hold data (the first of each run of blanks, at most a few for each sheet): used to space things out, they read as the end of the data.
    if (calc) {
        constexpr int kMostBlankPerSheet = 5;
        qint64 work = 0;
        for (int s = 0; s < int(sheets.size()) && s < calc->sheetCount() && int(out.size()) < most; ++s) {
            if (sheets[size_t(s)].deleted) continue;
            QSet<quint32> rowsUsed, colsUsed;
            calc->sheet(s).cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
                if (++work > kContrastWork || (c.type == CellType::Empty && !c.hasFormula())) return;
                rowsUsed.insert(row);
                colsUsed.insert(col);
            });
            auto gaps = [&](const QSet<quint32> &used, bool rows) {
                QList<quint32> sorted(used.begin(), used.end());
                std::sort(sorted.begin(), sorted.end());
                int found = 0;
                for (int i = 1; i < sorted.size() && found < kMostBlankPerSheet; ++i) {
                    if (sorted[i] == sorted[i - 1] + 1) continue;
                    ++found;
                    const quint32 first = sorted[i - 1] + 1;
                    add(AccessIssue::BlankLayoutCells, AccessIssue::Tip, s, rows ? first : 0, rows ? 0 : first,
                        (rows ? QCoreApplication::translate(ctx, "Row %1 on %2 is blank between rows that hold data. A screen reader takes a blank row for the end of the table. Use the cell's own space (a row height, or a border) instead.").arg(first + 1)
                              : QCoreApplication::translate(ctx, "Column %1 on %2 is blank between columns that hold data. A screen reader takes a blank column for the end of the table. Use the cell's own space (a column width, or a border) instead.").arg(columnName(first)))
                            .arg(sheets[size_t(s)].name));
                }
            };
            gaps(rowsUsed, true);
            gaps(colsUsed, false);
        }
    }
    // Objects stacked in another order than they sit on the sheet are read in that order.
    if (objects)
        for (int s = 0; s < int(sheets.size()) && int(out.size()) < most; ++s) {
            if (sheets[size_t(s)].deleted) continue;
            std::vector<std::pair<quint32, quint32>> at;
            const auto described = objects->describe(s);
            for (const SheetObjects::ObjectInfo &o : described) at.emplace_back(o.row, o.col);
            const int bad = firstObjectOutOfReadingOrder(at);
            if (bad < 0) continue;
            const SheetObjects::ObjectInfo &o = described[size_t(bad)];
            add(AccessIssue::ReadingOrder, AccessIssue::Tip, s, o.row, o.col,
                QCoreApplication::translate(ctx, "On %1 a screen reader reaches \"%2\" in another order than it sits on the sheet. Put the objects in order from top to bottom, and then from left to right.")
                    .arg(sheets[size_t(s)].name, o.name.isEmpty() ? QCoreApplication::translate(ctx, "(no name)") : o.name));
        }
    for (const Table &t : book->tables()) {
        if (t.headerRows != 0 || t.sheet < 0 || t.sheet >= int(sheets.size()) || sheets[size_t(t.sheet)].deleted) continue;
        add(AccessIssue::TableWithoutHeader, AccessIssue::Error, t.sheet, t.row1, t.col1,
            QCoreApplication::translate(ctx, "The table %1 on %2 has no header row, so a screen reader can't name its columns.").arg(t.name, sheets[size_t(t.sheet)].name));
    }
    return out;
}

} // namespace jp::crunch
