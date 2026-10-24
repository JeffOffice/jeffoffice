// The check of saving a workbook of the older binary form (docs/jeffcrunch.md):
//   jcrunchtool xls-save-check <file.xls>   opens the workbook, writes it as the XML form (nothing changed), reads that back and compares it with the workbook cell by cell (values, text,
//                                           formulas, formats, merged ranges, widths and heights, panes, names, strings), then writes it again with one cell changed and checks that the
//                                           change is there and nothing else moved. One line of tab-separated fields: `xls-save-check`, the path, the result (ok, or the first
//                                           differences), the bytes of the file written, the sheets, the cells, and the milliseconds each of the two writes took.
//                                           (tools/crunch_xls_savecheck.py runs it over a list of workbooks and sums it up.)

#include "io/zip.h"
#include "office/package.h"
#include "sheet/sheetreader.h"
#include "sheet/xlsxsave.h"

#include <QElapsedTimer>
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

struct Diffs {
    QStringList found;
    void add(const QString &what)
    {
        if (found.size() < 4 && !found.contains(what)) found << what;
    }
};

bool sameNumber(double a, double b) { return a == b || (std::isnan(a) && std::isnan(b)); }

// What differs between the sheet as the old workbook reads and the same read from the file written.
void compareSheets(const Workbook &a, const Sheet &x, const Workbook &b, const Sheet &y, Diffs *d)
{
    if (x.cells.cellCount() != y.cells.cellCount()) d->add(QStringLiteral("cell count"));
    x.cells.forEach([&](const Cell &c) {
        const Cell *e = y.cells.find(c.row, c.col);
        if (!e) {
            d->add(QStringLiteral("a cell is missing"));
            return;
        }
        if (c.type != e->type) d->add(QStringLiteral("cell type"));
        else if (c.type == CellType::SharedString || c.type == CellType::InlineString || c.type == CellType::FormulaString) {
            if (x.text(c, a.sharedStrings()) != y.text(*e, b.sharedStrings())) d->add(QStringLiteral("cell text"));
        } else if (!sameNumber(c.number, e->number)) {
            d->add(QStringLiteral("cell number"));
        }
        if (c.style != e->style) d->add(QStringLiteral("cell style"));
        const quint8 kept = HasFormula | ArrayFormula | DataTable;
        if ((c.flags & kept) != (e->flags & kept)) d->add(QStringLiteral("formula kind"));
        else if (c.hasFormula() && x.formulaText(c) != y.formulaText(*e)) d->add(QStringLiteral("formula text"));
    });
    if (x.merges.size() != y.merges.size()) d->add(QStringLiteral("merges"));
    if (x.columns.size() != y.columns.size()) d->add(QStringLiteral("columns"));
    // (a row that only says its height is the sheet's own, which the XML form does not say, is no row of its own there)
    auto own = [](const Sheet &sh) {
        qint64 n = 0;
        for (const RowSpec &r : sh.rowSpecs)
            if (r.hidden || r.customFormat || sh.format.zeroHeight || (r.height >= 0 && std::abs(r.height - sh.format.defaultRowHeight) > 0.001)) ++n;
        return n;
    };
    if (own(x) != own(y)) d->add(QStringLiteral("rows"));
    if (x.view.frozen != y.view.frozen || x.view.frozenRows != y.view.frozenRows || x.view.frozenCols != y.view.frozenCols || x.view.zoom != y.view.zoom) d->add(QStringLiteral("view"));
    if (x.isProtected != y.isProtected) d->add(QStringLiteral("protection"));
}

} // namespace

int xlsSaveCheckMain(const QStringList &args)
{
    if (args.size() != 1) return -1;
    const QString path = args[0];
    auto line = [&](const QString &result, qint64 bytes = 0, int sheets = 0, qint64 cells = 0, qint64 firstMs = 0, qint64 secondMs = 0) {
        out() << "xls-save-check\t" << path << '\t' << result << '\t' << bytes << '\t' << sheets << '\t' << cells << '\t' << firstMs << '\t' << secondMs << "\n";
        out().flush();
    };
    Workbook book;
    QString why;
    if (!book.openFile(path, &why) || !book.isBinary()) {
        line(QStringLiteral("not opened: ") + why);
        return 1;
    }
    Diffs diffs;
    QElapsedTimer timer;
    timer.start();
    QByteArray bytes;
    if (!saveXlsx(book, SaveChanges(), &bytes, &why)) {
        line(QStringLiteral("save failed: ") + why);
        return 3;
    }
    const qint64 firstMs = timer.elapsed();
    Package package;
    Workbook back;
    if (!package.readLazy(bytes, &why) || !back.open(package, &why)) {
        line(QStringLiteral("the file written does not open: ") + why, bytes.size());
        return 3;
    }
    if (back.sharedStringsError().size() || back.stylesError().size()) diffs.add(QStringLiteral("strings or styles damaged"));
    const WorkbookInfo &ia = book.info(), &ib = back.info();
    if (ia.sheets.size() != ib.sheets.size()) diffs.add(QStringLiteral("sheet count"));
    if (ia.definedNames.size() != ib.definedNames.size()) diffs.add(QStringLiteral("names"));
    if (book.sharedStrings().strings != back.sharedStrings().strings) diffs.add(QStringLiteral("shared strings"));
    if (book.styles().formats.size() != back.styles().formats.size()) diffs.add(QStringLiteral("cell formats"));
    for (auto it = book.styles().numFmts.constBegin(); it != book.styles().numFmts.constEnd(); ++it)
        if (back.styles().numFmts.value(it.key()) != it.value()) diffs.add(QStringLiteral("number formats"));
    qint64 cells = 0;
    int firstSheetWithCell = -1;
    quint32 changeRow = 0, changeCol = 0;
    for (int i = 0; i < std::min(ia.sheets.size(), ib.sheets.size()); ++i) {
        if (ia.sheets[i].name != ib.sheets[i].name || ia.sheets[i].state != ib.sheets[i].state) diffs.add(QStringLiteral("sheet names"));
        Sheet x, y;
        book.readSheet(i, &x);
        if (!back.readSheet(i, &y)) diffs.add(QStringLiteral("a sheet does not read: ") + y.error);
        cells += x.cells.cellCount();
        compareSheets(book, x, back, y, &diffs);
        if (firstSheetWithCell < 0 && x.cells.cellCount() > 0) {
            firstSheetWithCell = i;
            const Cell *first = nullptr;
            x.cells.forEach([&](const Cell &c) {
                if (!first) first = &c;
            });
            changeRow = first->row;
            changeCol = first->col;
        }
    }
    // the same with one cell changed (the first cell of the first sheet that has one)
    qint64 secondMs = 0;
    if (firstSheetWithCell >= 0) {
        SaveChanges changes;
        SavedCell cell;
        cell.row = changeRow;
        cell.col = quint16(changeCol);
        cell.value.kind = SavedValue::Kind::Number;
        cell.value.number = 12345.5;
        changes.sheets[firstSheetWithCell].cells.push_back(cell);
        timer.restart();
        QByteArray second;
        if (!saveXlsx(book, changes, &second, &why)) {
            diffs.add(QStringLiteral("save with a change failed: ") + why);
        } else {
            secondMs = timer.elapsed();
            Package p2;
            Workbook w2;
            if (!p2.readLazy(second, &why) || !w2.open(p2, &why)) {
                diffs.add(QStringLiteral("the file with a change does not open"));
            } else {
                Sheet y;
                w2.readSheet(firstSheetWithCell, &y);
                const Cell *c = y.cells.find(changeRow, changeCol);
                if (!c || c->type != CellType::Number || c->number != 12345.5) diffs.add(QStringLiteral("the change is not there"));
                if (y.cells.cellCount() < 1) diffs.add(QStringLiteral("cells lost by the change"));
            }
        }
    }
    line(diffs.found.isEmpty() ? QStringLiteral("ok") : diffs.found.join(QStringLiteral(" | ")), bytes.size(), int(ia.sheets.size()), cells, firstMs, secondMs);
    return diffs.found.isEmpty() ? 0 : 4;
}

} // namespace jp::crunchtool
