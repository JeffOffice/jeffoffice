// The check of saving a workbook that has a filter (docs/jeffcrunch.md):
//   jcrunchtool filter-roundtrip <file.xlsx> [out.xlsx]   opens the workbook, writes it with the filter of each sheet that has one made again from what was read (the element the file has is taken out and
//                                              this one put in its place, and `filterMode` set to match), reads the file written, and compares it with the workbook: each sheet's filter (its
//                                              range and the conditions of its columns), the rows that are hidden (and which of them the filter hides), and every cell (its type, value or
//                                              text, formula, and format). One line of tab-separated fields: `filter-roundtrip`, the path, the result (ok, or the first differences), the
//                                              sheets, the sheets that have a filter, the columns with a condition, the cells, and the hidden rows in sheets with a filter.
//                                              (With a second name, the file written is kept there.)

#include "office/package.h"
#include "sheet/sheetreader.h"
#include "sheet/xlsxsave.h"

#include <QFile>
#include <QTextStream>

#include <cmath>

using namespace jp::office;
using namespace jp::office::sheet;

namespace jp::crunchtool {

namespace {

QTextStream &out()
{
    static QTextStream s(stdout);
    return s;
}

} // namespace

int filterRoundtripMain(const QStringList &args)
{
    if (args.size() != 1 && args.size() != 2) return -1;
    const QString path = args[0];
    auto line = [&](const QString &result, int sheets = 0, int filtered = 0, int conditions = 0, qint64 cells = 0, qint64 hidden = 0) {
        out() << "filter-roundtrip\t" << path << '\t' << result << '\t' << sheets << '\t' << filtered << '\t' << conditions << '\t' << cells << '\t' << hidden << "\n";
        out().flush();
    };
    Workbook book;
    QString why;
    if (!book.openFile(path, &why) || book.isConverted()) {
        line(QStringLiteral("not opened: ") + why);
        return 1;
    }
    SaveChanges changes;
    std::vector<Sheet> before(size_t(book.info().sheets.size()));
    int filtered = 0, conditions = 0;
    qint64 cells = 0, hidden = 0;
    for (int i = 0; i < book.info().sheets.size(); ++i) {
        if (!book.readSheet(i, &before[size_t(i)])) continue;
        const Sheet &s = before[size_t(i)];
        cells += s.cells.cellCount();
        if (!s.autoFilter.valid()) continue;
        ++filtered;
        conditions += int(s.autoFilter.conditionColumns().size());
        for (const RowSpec &r : s.rowSpecs) hidden += r.hidden ? 1 : 0;
        SheetChanges &sc = changes.sheets[i];
        sc.replaceAutoFilter = true;
        sc.autoFilter = s.autoFilter;
        sc.filterMode = s.autoFilter.active() ? 1 : 0;
    }
    if (filtered == 0) {
        line(QStringLiteral("no filter"), int(book.info().sheets.size()), 0, 0, cells);
        return 0;
    }
    QByteArray bytes;
    if (!saveXlsx(book, changes, &bytes, &why)) {
        line(QStringLiteral("save failed: ") + why, int(book.info().sheets.size()), filtered, conditions, cells, hidden);
        return 3;
    }
    if (args.size() == 2) {
        // (the file written is kept, for another program to open)
        QFile file(args[1]);
        if (file.open(QIODevice::WriteOnly)) file.write(bytes);
    }
    Package package;
    Workbook back;
    if (!package.readLazy(bytes, &why) || !back.open(package, &why)) {
        line(QStringLiteral("the file written does not open: ") + why, int(book.info().sheets.size()), filtered, conditions, cells, hidden);
        return 3;
    }
    QStringList diffs;
    auto add = [&](const QString &what) {
        if (diffs.size() < 4 && !diffs.contains(what)) diffs << what;
    };
    if (back.info().sheets.size() != book.info().sheets.size()) add(QStringLiteral("sheet count"));
    for (int i = 0; i < std::min<int>(book.info().sheets.size(), back.info().sheets.size()); ++i) {
        const Sheet &x = before[size_t(i)];
        Sheet y;
        if (!back.readSheet(i, &y)) {
            add(QStringLiteral("a sheet does not read"));
            continue;
        }
        if (!(x.autoFilter == y.autoFilter)) add(QStringLiteral("filter"));
        if (x.cells.cellCount() != y.cells.cellCount()) add(QStringLiteral("cell count"));
        x.cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
            const Cell *e = y.cells.find(row, col);
            if (!e) {
                add(QStringLiteral("a cell is missing"));
                return;
            }
            if (c.type != e->type || c.style != e->style) add(QStringLiteral("cell type or format"));
            else if (c.type == CellType::SharedString || c.type == CellType::InlineString || c.type == CellType::FormulaString) {
                if (x.text(c, book.sharedStrings()) != y.text(*e, back.sharedStrings())) add(QStringLiteral("cell text"));
            } else if (!(c.number == e->number || (std::isnan(c.number) && std::isnan(e->number)))) {
                add(QStringLiteral("cell number"));
            }
            if (c.hasFormula() != e->hasFormula() || (c.hasFormula() && x.formulaText(c) != y.formulaText(*e))) add(QStringLiteral("formula"));
        });
        if (x.rowSpecs.size() != y.rowSpecs.size()) add(QStringLiteral("row count"));
        else
            for (int r = 0; r < x.rowSpecs.size(); ++r)
                if (x.rowSpecs[r].row != y.rowSpecs[r].row || x.rowSpecs[r].hidden != y.rowSpecs[r].hidden || x.rowSpecs[r].filtered != y.rowSpecs[r].filtered) {
                    add(QStringLiteral("hidden rows"));
                    break;
                }
        if (x.merges.size() != y.merges.size() || x.validations.size() != y.validations.size() || x.conditionalFormats.size() != y.conditionalFormats.size()) add(QStringLiteral("merges or rules"));
    }
    line(diffs.isEmpty() ? QStringLiteral("ok") : diffs.join(QStringLiteral("; ")), int(book.info().sheets.size()), filtered, conditions, cells, hidden);
    return diffs.isEmpty() ? 0 : 2;
}

} // namespace jp::crunchtool
