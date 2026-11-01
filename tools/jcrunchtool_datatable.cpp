// The check of the what-if (data) tables (docs/jeffcrunch.md):
//   jcrunchtool datatable-check <file>   works the workbook out twice, once leaving each data table as the file stored it and once working the tables out, and compares the cells of each table. One
//                                        line of tab-separated fields: `datatable-check`, the path, the result (ok, differs, or why not), the tables, their cells, the cells that differ, and the first
//                                        difference (with the value stored and the value worked out).

#include "sheet/calculation.h"

#include <QTextStream>

#include <cmath>

using namespace jp::office;
using namespace jp::office::sheet;

namespace jp::crunchtool {

int dataTableCheckMain(const QStringList &args)
{
    if (args.size() != 1) return -1;
    QTextStream out(stdout);
    Workbook book;
    QString why;
    if (!book.openFile(args[0], &why)) {
        out << "datatable-check\t" << args[0] << "\tnot opened: " << why << "\n";
        return 1;
    }
    auto make = [&](bool tables) {
        CalcOptions options;
        options.keepApart = true;
        options.workOutWhatHasNoResult = true;
        options.goRoundCircles = true;
        options.workOutTables = true;
        options.workOutDataTables = tables;
        auto calc = std::make_unique<Calculation>(book, options);
        if (!calc->load()) return std::unique_ptr<Calculation>();
        while (!calc->recalculate().finished) {}
        return calc;
    };
    const auto stored = make(false), ours = make(true);
    if (!stored || !ours) {
        out << "datatable-check\t" << args[0] << "\tnot worked out\n";
        return 3;
    }
    qint64 tables = 0, cells = 0, differing = 0;
    QString first;
    for (int s = 0; s < book.info().sheets.size(); ++s) {
        if (book.info().sheets[s].deleted || book.info().sheets[s].kind != SheetKind::Worksheet) continue;
        Sheet sh;
        book.readSheet(s, &sh);
        sh.cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
            if (!(c.flags & DataTable)) return;
            const CellRange r = sh.formulaRange(c);
            if (!r.valid || r.row1 != row || r.col1 != col) return;
            ++tables;
            for (quint32 rr = r.row1; rr <= r.row2; ++rr)
                for (quint32 cc = r.col1; cc <= r.col2; ++cc) {
                    ++cells;
                    const jp::calc::Value a = stored->cell(s, rr, cc), b = ours->cell(s, rr, cc);
                    // (numbers within a part in a trillion: the program that stored them may have added in another order)
                    const bool same = a.isNumber() && b.isNumber() ? std::abs(a.number() - b.number()) <= 1e-12 * std::max(std::abs(a.number()), std::abs(b.number())) : a.debug() == b.debug();
                    if (!same) {
                        ++differing;
                        if (first.isEmpty()) first = QStringLiteral("%1!%2: stored %3, worked out %4").arg(book.info().sheets[s].name, cellName(rr, cc), a.debug(), b.debug());
                    }
                }
        });
    }
    out << "datatable-check\t" << args[0] << '\t' << (differing == 0 ? "ok" : "differs") << '\t' << tables << '\t' << cells << '\t' << differing << '\t' << first << "\n";
    return 0;
}

} // namespace jp::crunchtool
