// Command-line helper for JeffCrunch's readers and its yardstick (docs/jeffcrunch.md).
//   jcrunchtool info [--tsv] <file.xlsx|file.xls>         the workbook's sheets, cells, formula cells, and formula cells with a stored result, the functions the formulas
//                                                call with how often, and the time the read took; --tsv is that as one line of tab-separated fields (sheets, cells,
//                                                formula cells, stored results, functions as NAME:count,..., problems, and the most that any one thing came to, for choosing limits:
//                                                cells in a sheet, XML bytes of a sheet, longest string, longest formula, shared formulas in a sheet, cells of the largest shared
//                                                range, formula characters expanded in a sheet, shared strings, their characters, sheets, bytes counted to hold the largest sheet). Exits 0 if all was read, 2 if it was opened
//                                                but a sheet is only partly read, and 1 if it could not be opened.
//   jcrunchtool names <file.xlsx|file.xls>                the defined names, one to a line (the sheet it belongs to, its name, and what it stands for), and what a binary workbook holds that is not read
//   jcrunchtool compare-one [--first-sheet] [--values-only] <ours.xls|.ods|.csv> <theirs.xlsx>   a workbook of the old form, an .ods, or a .csv read here, and the same workbook saved in the XML form by another program, cell by cell (see jcrunchtool_compare.cpp;
//                                                tools/crunch_xls_compare.py runs it over a list and sums it up)
//   jcrunchtool convert-fuzz <file.ods|file.fods|file.csv> [count=500] [seed=1] [seconds=60]   the same for an OpenDocument spreadsheet or a text file of values (see jcrunchtool_odsfuzz.cpp)
//   jcrunchtool xls-fuzz <file.xls> [count=500] [seed=1] [seconds=60]   the workbook damaged in a few places, `count` times, and each copy opened, read, and (some) worked out, as a sweep for crashes
//                                                and hangs of the reader of the old binary form (see jcrunchtool_xlsfuzz.cpp)
//   jcrunchtool filter-roundtrip <file.xlsx> [out.xlsx]   a workbook with a filter written with its filters made again, read back, and compared (see jcrunchtool_filter.cpp)
//   jcrunchtool xls-save-check <file.xls>   the workbook written as the XML form, read back, and compared with itself, and written again with one cell changed (see jcrunchtool_xlssave.cpp;
//                                                tools/crunch_xls_savecheck.py runs it over a list)
//   jcrunchtool dump <file.xlsx|file.xls> [sheet=1] [max=100]   the first cells of a sheet (counting from 1), one to a line: address, type, value, and formula
//   jcrunchtool scan <keys.txt> <out-dir> [jobs]   runs `info` on each workbook listed (one path to a line), each in a child process that has 120 seconds and 4 GB,
//                                                  and writes <out-dir>/books.tsv: path, exit, seconds, peak MB, sheets, cells, formula cells, stored results, and
//                                                  functions used. (tools/crunch_scan.py does the work; tools/crunch_summary.py <out-dir> sums the table up.)
//   jcrunchtool format <code> <number> [1904]   a number in a format code, as the engine shows it: the text, then the color, where a fill goes, and "failed" if it can't be shown
//   jcrunchtool formats-one <file.xlsx|file.xls>          the number formats checked in one workbook: a line `case` for each formula that is TEXT, FIXED, or DOLLAR of
//                                                a number (sheet!cell, function, value, code, the text the file stored, ours, 1 if they are the same), and a
//                                                line `use` for each format code the cells use (the code, how many cells with a value, 1 if it parses)
//   jcrunchtool formats <keys.txt> <out-dir> [jobs]   runs `formats-one` on each workbook listed under the same limits as `scan`, writes <out-dir>/formats.tsv
//                                                  and codes.tsv, and prints the share of cases that match (tools/crunch_formats.py does the work)
//   jcrunchtool recalc-one <file.xlsx|file.xls> [matches=3] [seconds=100]   every formula cell worked out once, reading what it refers to from the values the file stored, and compared with the
//                                                result stored with it (15 digits for a number; text, true or false, and the kind of an error exactly). Lines: `row` for each cell
//                                                that does not match (a few of each kind besides, and a few that match): sheet, cell, class (M match, X mismatch, U calls a function
//                                                not in the registry, V volatile, E another workbook, T a table's columns, P not parsed, N no result stored, D data table, L past the
//                                                work bound, S not reached in time), formula, stored, ours, and the functions it calls; `fn`: a function, calls, calls in cells that
//                                                matched, cells, matched cells; `unsup`: a function not in the registry and the cells that call it; `pat`: how mismatches look and
//                                                how many; and `book`: the formula cells, the cells of each class in the order above, the volatile ones that were worked out and
//                                                matched, whether the workbook calculates by hand, whether every sheet was read, and the seconds
//   jcrunchtool functions                      the functions the formula engine has: name, fewest and most arguments, and v (volatile) or f (written with the prefix _xlfn.)
//   jcrunchtool recalc-full-one <file.xlsx|file.xls> [matches=3] [seconds=100]   every formula worked out from the constants (not from the results the file stored), in dependency order, and
//                                                compared with its stored result; the formulas the engine cannot work out keep the stored result and mark what reads them (class Z);
//                                                prints the cells that do not match, the functions, and a line for the workbook with the seconds and the peak memory
//   jcrunchtool recalc-full <keys.txt> <out-dir> [jobs]   runs `recalc-full-one` on each workbook listed under the limits of `recalc`, writes <out-dir>/cells.tsv and books.tsv, and prints the
//                                                summary: the cells and the share that match, the workbooks that match whole, how each of the commonest functions fared, the commonest
//                                                mismatches, and the time and memory (tools/crunch_recalc_full.py does the work)
//   jcrunchtool mutate <file.xlsx|file.xls> [count=3000] [seed=1] [seconds=60]   the formulas of the workbook, each damaged in a few places (a character changed, taken out, or put in, a piece
//                                                repeated or moved), some grown by repeating them as the arguments of a sum or the parts of a text up to the 32,768 characters a formula
//                                                may have, and parsed and worked out against the workbook's cells with the engine's own bounds on steps and cells, as a sweep for crashes
//                                                and hangs; then edits (values, texts, errors, cleared cells, the damaged formulas put in cells) to the calculation of the workbook,
//                                                each followed by a recalculation, some in slices and some stopped; prints how many of each
//   jcrunchtool eval <file.xlsx|file.xls> <sheet> <cell> <formula>   the formula worked out as if it were in that cell of that sheet (a name, or a number counting from 1), reading the values the file stored:
//                                                the value, as `n:1.5`, `s:text`, `b:1`, `e:#N/A`, or `a:2x2[...]`
//   jcrunchtool recalc <keys.txt> <out-dir> [jobs]   runs `recalc-one` on each workbook listed under the same limits as `scan`, writes <out-dir>/cells.tsv (workbook, sheet, cell, formula,
//                                                  stored, ours, class: the cells that do not match, with a few that do) and books.tsv, and prints the summary: how many formula cells
//                                                  matched, how many workbooks matched whole, how each function fared, the commonest mismatches, and the commonest unsupported
//                                                  functions (tools/crunch_recalc.py does the work)

#include "office/package.h"
#include "calc/functions.h"
#include "calc/numfmt.h"
#include "sheet/calculation.h"
#include "sheet/cellref.h"
#include "sheet/formatcases.h"
#include "sheet/recalc.h"
#include "sheet/sheetreader.h"

#ifdef JC_GRID
#include "app/crunchinfo.h"
#include "app/crash.h"
#include "core/fonts.h"
#include <QApplication>
#endif
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QProcess>
#include <QTextStream>

#include <algorithm>
#include <atomic>
#include <random>

using namespace jp::office;
using namespace jp::office::sheet;

#ifdef JC_GRID
namespace jp::crunchtool {
// The grid's commands (tools/jcrunchtool_grid.cpp): the code to exit with, or -1 for a line that is none of them.
int gridMain(const QStringList &args);
// The editing commands (tools/jcrunchtool_edit.cpp: roundtrip, edit-bench), the same.
int editMain(const QStringList &args);
int drawingsMain(const QStringList &args);   // (jcrunchtool_drawings.cpp)
int printMain(const QStringList &args);      // (jcrunchtool_print.cpp)
int chartsMain(const QStringList &args);     // (jcrunchtool_charts.cpp)
}
#endif

namespace {

QTextStream &out()
{
    static QTextStream s(stdout);
    return s;
}

QTextStream &err()
{
    static QTextStream s(stderr);
    return s;
}

// The most any one thing came to, over the sheets: what the limits of the sheet reader are chosen above.
struct Largest {
    qint64 cellsInSheet = 0, string = 0, formula = 0, sharedFormulas = 0, sharedRange = 0, expandedChars = 0, xmlBytes = 0, heldBytes = 0;
    void take(const Sheet &s)
    {
        cellsInSheet = std::max(cellsInSheet, s.cells.cellCount());
        string = std::max(string, s.stats.longestString);
        formula = std::max(formula, s.stats.longestFormula);
        sharedFormulas = std::max(sharedFormulas, s.stats.sharedFormulas);
        sharedRange = std::max(sharedRange, s.stats.largestSharedRange);
        expandedChars = std::max(expandedChars, s.stats.expandedFormulaChars);
        xmlBytes = std::max(xmlBytes, s.stats.xmlBytes);
        heldBytes = std::max(heldBytes, s.stats.heldBytes);
    }
};

// The peak memory of this process in MB, where the system says it; -1 if it doesn't.
double peakMegabytes()
{
    QFile status(QStringLiteral("/proc/self/status"));
    if (!status.open(QIODevice::ReadOnly)) return -1;
    for (const QByteArray &line : status.readAll().split('\n'))
        if (line.startsWith("VmHWM:")) return line.mid(6).trimmed().split(' ').value(0).toDouble() / 1024.0;
    return -1;
}

int info(const QString &path, bool tsv)
{
    QElapsedTimer timer;
    timer.start();
    QString why;
    Workbook book;
    if (!book.openFile(path, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    qint64 cells = 0, formulaCells = 0, storedResults = 0, valued = 0, worksheets = 0, merged = 0, unread = 0;
    QHash<QString, qint64> functions;
    QStringList problems;
    Largest largest;
    for (int i = 0; i < book.info().sheets.size(); ++i) {
        const SheetEntry &entry = book.info().sheets[i];
        if (entry.kind == SheetKind::Chartsheet) continue;
        ++worksheets;
        Sheet sheet;
        if (!book.readSheet(i, &sheet)) problems << QStringLiteral("sheet %1 (%2): %3").arg(i + 1).arg(entry.name, sheet.error);
        cells += sheet.cells.cellCount();
        formulaCells += sheet.formulaCells;
        storedResults += sheet.storedResults;
        merged += sheet.mergeCount;
        unread += sheet.stats.unreadFormulas;
        largest.take(sheet);
        // Each distinct text of a formula is looked at once, whatever number of cells have it.
        QVector<qint64> uses(sheet.formulaTexts.size(), 0);
        sheet.cells.forEach([&](const Cell &c) {
            if (c.type != CellType::Empty) ++valued;
            if (c.hasFormula() && c.formula >= 0 && sheet.formulas[c.formula].text >= 0) ++uses[sheet.formulas[c.formula].text];
        });
        for (int t = 0; t < uses.size(); ++t) {
            if (uses[t] == 0) continue;
            for (const QString &name : formulaFunctions(sheet.formulaTexts[t])) functions[name] += uses[t];
        }
    }
    if (!book.sharedStringsError().isEmpty()) problems << QStringLiteral("shared strings: %1").arg(book.sharedStringsError());
    const double seconds = timer.elapsed() / 1000.0;
    QList<QPair<QString, qint64>> ranked;
    for (auto it = functions.cbegin(); it != functions.cend(); ++it) ranked.append({it.key(), it.value()});
    std::sort(ranked.begin(), ranked.end(), [](const auto &a, const auto &b) { return a.second != b.second ? a.second > b.second : a.first < b.first; });
    if (tsv) {
        QStringList used;
        for (const auto &f : ranked) used << QStringLiteral("%1:%2").arg(f.first).arg(f.second);
        out() << worksheets << '\t' << cells << '\t' << formulaCells << '\t' << storedResults << '\t' << used.join(',') << '\t' << problems.join(QStringLiteral("; ")).replace('\t', ' ');
        // and the most any one thing came to, for choosing the limits
        for (qint64 n : {largest.cellsInSheet, largest.xmlBytes, largest.string, largest.formula, largest.sharedFormulas, largest.sharedRange, largest.expandedChars,
                         qint64(book.sharedStrings().strings.size()), book.sharedStrings().chars, qint64(book.info().sheets.size()), largest.heldBytes, unread})
            out() << '\t' << n;
        out() << "\n";
    } else {
        out() << "file:                  " << path << "\n";
        out() << "sheets:                " << worksheets << " worksheets (" << book.info().sheets.size() << " in the workbook), " << (book.info().dateSystem == DateSystem::Date1904 ? "1904" : "1900")
              << " dates\n";
        out() << "cells:                 " << cells << " (" << valued << " with a value, " << merged << " merged ranges)\n";
        out() << "formula cells:         " << formulaCells << "\n";
        if (book.isConverted()) {
            out() << "formulas not read:     " << unread << " (formulas of a file of another form that could not be turned into text keep the result the file stored)\n";
            const QStringList skipped = book.skippedFeatures();
            if (!skipped.isEmpty()) out() << "not read:              " << skipped.join(QStringLiteral(", ")) << "\n";
        }
        out() << "with a stored result:  " << storedResults << "\n";
        out() << "shared strings:        " << book.sharedStrings().strings.size() << " (" << book.sharedStrings().chars << " characters)\n";
        out() << "largest:               " << largest.cellsInSheet << " cells in a sheet, " << largest.xmlBytes << " bytes of XML, a string of " << largest.string << " characters, a formula of "
              << largest.formula << ", " << largest.sharedFormulas << " shared formulas, a shared range of " << largest.sharedRange << " cells, " << largest.expandedChars
              << " formula characters expanded, " << largest.heldBytes << " bytes counted to hold the largest sheet\n";
        out() << "functions:             " << ranked.size() << " distinct\n";
        for (const auto &f : ranked) out() << "  " << f.first.leftJustified(20) << f.second << "\n";
        const double peak = peakMegabytes();
        out() << "read time:             " << QString::number(seconds, 'f', 3) << " s";
        if (peak >= 0) out() << ", peak memory " << QString::number(peak, 'f', 0) << " MB";
        out() << "\n";
        for (const QString &p : problems) out() << "problem: " << p << "\n";
    }
    return problems.isEmpty() ? 0 : 2;
}

int names(const QString &path)
{
    QString why;
    Workbook book;
    if (!book.openFile(path, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    for (const DefinedName &n : book.info().definedNames) out() << (n.localSheet >= 0 ? book.info().sheets.value(n.localSheet).name : QString()) << '\t' << n.name << '\t' << n.ref << (n.hidden ? "\thidden" : "") << "\n";
    const QStringList skipped = book.skippedFeatures();
    if (!skipped.isEmpty()) out() << "skipped: " << skipped.join(QStringLiteral(", ")) << "\n";
    return 0;
}

const char *typeName(CellType t)
{
    switch (t) {
    case CellType::Empty: return "empty";
    case CellType::Number: return "number";
    case CellType::SharedString: return "shared";
    case CellType::InlineString: return "inline";
    case CellType::FormulaString: return "text";
    case CellType::Bool: return "bool";
    case CellType::Error: return "error";
    case CellType::Date: return "date";
    }
    return "";
}

int dump(const QString &path, int sheetNumber, qint64 most)
{
    QString why;
    Workbook book;
    if (!book.openFile(path, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    Sheet sheet;
    if (!book.readSheet(sheetNumber - 1, &sheet)) err() << "problem: " << sheet.error << "\n";
    qint64 shown = 0;
    sheet.cells.forEach([&](const Cell &c) {
        if (shown++ >= most) return;
        QString value;
        switch (c.type) {
        case CellType::Number:
        case CellType::Date: value = QString::number(c.number, 'g', 17); break;
        case CellType::Bool: value = c.number != 0 ? QStringLiteral("TRUE") : QStringLiteral("FALSE"); break;
        case CellType::Error: value = errorText(CellError(int(c.number))); break;
        default: value = sheet.text(c, book.sharedStrings()); break;
        }
        const QString formula = sheet.formulaText(c);
        out() << cellName(c.row, c.col) << '\t' << typeName(c.type) << '\t' << value << '\t' << (c.hasFormula() ? QStringLiteral("=") + formula : QString()) << "\n";
    });
    return sheet.error.isEmpty() ? 0 : 2;
}

int formatOne(const QString &code, const QString &number, bool date1904)
{
    bool ok = false;
    const double v = number.toDouble(&ok);
    if (!ok) {
        err() << "jcrunchtool: not a number: " << number << "\n";
        return 2;
    }
    const jp::calc::NumberFormat f(code);
    const jp::calc::Formatted r = f.format(v, date1904 ? jp::calc::DateSystem::Date1904 : jp::calc::DateSystem::Date1900);
    out() << (f.isValid() ? "" : "(not valid: shown as General) ") << '[' << r.text << "]\tcolor " << r.color << "\tfill " << r.fillAt << (r.failed ? "\tfailed" : "") << "\n";
    return 0;
}

// Tabs and line ends in a field are written as \t, \n, \r, and the backslash as \\.
QString escapeField(const QString &s)
{
    QString r = s;
    r.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
    r.replace(QLatin1Char('\t'), QLatin1String("\\t"));
    r.replace(QLatin1Char('\n'), QLatin1String("\\n"));
    r.replace(QLatin1Char('\r'), QLatin1String("\\r"));
    return r;
}

int formatsOne(const QString &path)
{
    QString why;
    Workbook book;
    if (!book.openFile(path, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    FormatCaseFinder finder(book);
    QHash<quint32, qint64> styleCells;
    bool whole = true;
    for (int i = 0; i < book.info().sheets.size(); ++i) {
        if (book.info().sheets[i].kind == SheetKind::Chartsheet) continue;
        Sheet sheet;
        if (!book.readSheet(i, &sheet)) whole = false;
        sheet.cells.forEach([&](const Cell &c) {
            if (c.type != CellType::Empty) ++styleCells[c.style];
        });
        finder.addSheet(i, sheet);
    }
    QHash<QString, qint64> codeCells;
    for (auto it = styleCells.cbegin(); it != styleCells.cend(); ++it) codeCells[book.styles().formatCode(it.key())] += it.value();
    for (const FormatCase &c : finder.finish())
        out() << "case\t" << escapeField(c.sheet + QLatin1Char('!') + c.cell) << '\t' << c.function << '\t' << QString::number(c.value, 'g', 17) << '\t' << escapeField(c.code) << '\t'
              << escapeField(c.stored) << '\t' << escapeField(c.ours) << '\t' << (c.match ? '1' : '0') << "\n";
    for (auto it = codeCells.cbegin(); it != codeCells.cend(); ++it)
        out() << "use\t" << escapeField(it.key()) << '\t' << it.value() << '\t' << (jp::calc::NumberFormat(it.key()).isValid() ? '1' : '0') << "\n";
    return whole ? 0 : 2;
}

int recalcOne(const QString &path, int keepMatches, double seconds)
{
    QString why;
    Workbook book;
    if (!book.openFile(path, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    RecalcOptions options;
    options.keepMatches = keepMatches;
    options.seconds = seconds;
    RecalcReport report;
    QHash<char, int> printed;
    const auto each = [&](const RecalcCell &c) {
        const char cls = char(c.cls);
        const int cap = cls == 'X' ? 400 : cls == 'M' ? keepMatches : 25;
        if (printed[cls]++ >= cap) return;
        out() << "row\t" << escapeField(c.sheet) << '\t' << c.cell << '\t' << cls << '\t' << escapeField(c.formula) << '\t' << escapeField(c.stored) << '\t' << escapeField(c.ours) << '\t'
              << c.functions.join(QLatin1Char(',')) << "\n";
    };
    if (!recalcWorkbook(book, options, each, &report, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    for (auto it = report.functions.cbegin(); it != report.functions.cend(); ++it)
        out() << "fn\t" << it.key() << '\t' << it.value().calls << '\t' << it.value().matchedCalls << '\t' << it.value().cells << '\t' << it.value().matchedCells << "\n";
    for (auto it = report.unsupportedFunctions.cbegin(); it != report.unsupportedFunctions.cend(); ++it) out() << "unsup\t" << escapeField(it.key()) << '\t' << it.value() << "\n";
    for (auto it = report.patterns.cbegin(); it != report.patterns.cend(); ++it) out() << "pat\t" << escapeField(it.key()) << '\t' << it.value() << "\n";
    out() << "book\t" << report.formulaCells;
    for (const char cls : {'M', 'X', 'U', 'V', 'E', 'T', 'P', 'N', 'D', 'L', 'S', 'R'}) out() << '\t' << report.byClass.value(cls);
    out() << '\t' << report.volatileChecked << '\t' << report.volatileMatched << '\t' << (report.manualCalc ? 1 : 0) << '\t' << (report.whole ? 1 : 0) << '\t' << QString::number(report.seconds, 'f', 2) << "\n";
    return report.whole ? 0 : 2;
}

int recalcFullOne(const QString &path, int keepMatches, double seconds)
{
    QString why;
    Workbook book;
    if (!book.openFile(path, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    FullOptions options;
    options.keepMatches = keepMatches;
    options.seconds = seconds;
    FullReport report;
    QHash<char, int> printed;
    const auto each = [&](const RecalcCell &c) {
        const char cls = char(c.cls);
        // (a mismatch that reads another is a Y: the roots are the X, which the ones to look at first)
        const char shown = cls == 'X' && c.cascaded ? 'Y' : cls;
        const int cap = cls == 'X' ? (c.cascaded ? 100 : 400) : cls == 'M' ? keepMatches : 25;
        if (printed[shown]++ >= cap) return;
        out() << "row\t" << escapeField(c.sheet) << '\t' << c.cell << '\t' << shown << '\t' << escapeField(c.formula) << '\t' << escapeField(c.stored) << '\t' << escapeField(c.ours) << '\t'
              << c.functions.join(QLatin1Char(',')) << "\n";
    };
    if (!recalcFull(book, options, each, &report, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    for (auto it = report.functions.cbegin(); it != report.functions.cend(); ++it)
        out() << "fn\t" << it.key() << '\t' << it.value().calls << '\t' << it.value().matchedCalls << '\t' << it.value().cells << '\t' << it.value().matchedCells << "\n";
    for (auto it = report.unsupportedFunctions.cbegin(); it != report.unsupportedFunctions.cend(); ++it) out() << "unsup\t" << escapeField(it.key()) << '\t' << it.value() << "\n";
    for (auto it = report.patterns.cbegin(); it != report.patterns.cend(); ++it) out() << "pat\t" << escapeField(it.key()) << '\t' << it.value() << "\n";
    out() << "fullbook\t" << report.formulaCells;
    for (const char cls : {'M', 'X', 'U', 'V', 'E', 'T', 'P', 'N', 'D', 'L', 'I', 'C', 'R', 'Z', 'S'}) out() << '\t' << report.byClass.value(cls);
    out() << '\t' << report.volatileChecked << '\t' << report.volatileMatched << '\t' << report.taintedMatched << '\t' << report.circular << '\t' << report.circularStoredZero << '\t' << report.rootMismatches << '\t' << (report.manualCalc ? 1 : 0) << '\t'
          << (report.whole ? 1 : 0) << '\t' << QString::number(report.seconds, 'f', 2) << '\t' << QString::number(report.loadSeconds, 'f', 2) << '\t' << QString::number(report.calcSeconds, 'f', 2)
          << '\t' << QString::number(peakMegabytes(), 'f', 0) << '\t' << QString::number(double(report.sheetXmlBytes) / 1048576.0, 'f', 1) << '\t' << report.graphEntries << '\t' << report.evaluations << "\n";
    return report.whole ? 0 : 2;
}

int mutate(const QString &path, int count, quint32 seed, double seconds)
{
    QString why;
    Workbook book;
    if (!book.openFile(path, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    WorkbookSource source(book);
    if (qEnvironmentVariableIsSet("JCRUNCH_TRACE")) { fprintf(stderr, "opened\n"); fflush(stderr); }
    source.loadAll();
    if (qEnvironmentVariableIsSet("JCRUNCH_TRACE")) { fprintf(stderr, "read\n"); fflush(stderr); }
    QStringList formulas;
    for (int i = 0; i < source.sheetCount() && formulas.size() < 4000; ++i)
        for (const QString &f : source.sheet(i).formulaTexts)
            if (formulas.size() < 4000) formulas << f;
    if (formulas.isEmpty()) {
        out() << "mutate: no formulas\n";
        return 0;
    }
    static const char *const pieces[] = {"(", ")", ",", ";", "'", "\"", "{", "}", "[", "]", "!", ":", " ", "$", "#", "%", "^", "&", "<=", "<>", "-", "+", "*", "/", "A1", "B2:C3", "1:1", "A:A", "0", "1E308",
                                         "1E-308", "SUM(", "IF(", "INDEX(", "OFFSET(", "INDIRECT(", "VLOOKUP(", "REPT(", "SEARCH(", "Sheet1!", "'x'!", "TRUE", "#N/A", "{1,2;3,4}", "\\", "?", "*?", "~"};
    const int nPieces = int(sizeof(pieces) / sizeof(pieces[0]));
    std::mt19937 rng(seed);
    // one of the workbook's formulas, damaged in a few places, and now and then grown to the most characters a formula may have
    const auto damaged = [&]() {
        QString f = formulas[int(rng() % quint32(formulas.size()))];
        const int edits = 1 + int(rng() % 4);
        for (int k = 0; k < edits; ++k) {
            const int pos = f.isEmpty() ? 0 : int(rng() % quint32(f.size()));
            switch (rng() % 5) {
            case 0: f.replace(pos, 1, QString::fromLatin1(pieces[rng() % quint32(nPieces)])); break;
            case 1: f.remove(pos, 1 + int(rng() % 3)); break;
            case 2: f.insert(pos, QString::fromLatin1(pieces[rng() % quint32(nPieces)])); break;
            case 3: f.insert(pos, f.mid(int(rng() % quint32(std::max(1, int(f.size())))), 1 + int(rng() % 12))); break;
            default: f.replace(pos, 1, QString(QChar(ushort(32 + rng() % 95)))); break;
            }
            if (f.size() > jp::calc::kMaxFormulaChars + 8) f.truncate(jp::calc::kMaxFormulaChars + 8);
        }
        // a formula of the most characters a file may give: the damaged one repeated as the arguments of a sum, or the parts of a text (some a few characters too long, to be refused)
        if (rng() % 8 == 0 && !f.isEmpty()) {
            const int target = int(rng() % 4 == 0 ? jp::calc::kMaxFormulaChars + 8 : 2000 + rng() % 30000);
            const int copies = std::max(1, std::min(target / int(f.size() + 1), 60000));
            if (rng() % 2) f = QStringLiteral("SUM(") + (f + QLatin1Char(',')).repeated(copies) + QStringLiteral("0)");
            else f = (f + QLatin1Char('&')).repeated(copies) + QStringLiteral("\"\"");
            if (f.size() > jp::calc::kMaxFormulaChars + 8) f.truncate(jp::calc::kMaxFormulaChars + 8);
        }
        return f;
    };
    QElapsedTimer timer;
    timer.start();
    jp::calc::Evaluator evaluator;   // (with the engine's own limit on steps: a sweep with a smaller one never reaches what the real limit lets through)
    qint64 parsed = 0, worked = 0, tried = 0;
    for (int i = 0; i < count && timer.elapsed() < seconds * 500.0; ++i) {
        const QString f = damaged();
        ++tried;
        for (int mode = 0; mode < 2; ++mode) {
            jp::calc::ParseOptions po;
            po.arrayFormula = mode == 1;
            const jp::calc::ParseResult pr = jp::calc::parseFormula(f, po);
            if (!pr.ok()) continue;
            ++parsed;
            jp::calc::EvalContext ctx;
            ctx.source = &source;
            ctx.sheet = int(rng() % quint32(source.sheetCount()));
            ctx.row = rng() % 200;
            ctx.col = rng() % 30;
            ctx.shiftRows = int(rng() % 5);
            ctx.shiftCols = int(rng() % 5);
            ctx.now = 45366.5;
            if (qEnvironmentVariableIsSet("JCRUNCH_TRACE")) { fprintf(stderr, "evaluate (array %d) %s\n", mode, qPrintable(f.left(400))); fflush(stderr); }
            evaluator.evaluate(*pr.program, ctx);
            ++worked;
        }
    }
    // the calculation over the workbook: its formulas worked out, then edits (values, text, errors, cleared cells, the damaged formulas put in cells) each followed by a
    // recalculation, some in slices and some stopped, as a sweep for crashes and hangs
    qint64 edits = 0, sliced = 0;
    {
        CalcOptions options;
        options.now = 45366.5;
        if (qEnvironmentVariableIsSet("JCRUNCH_TRACE")) { fprintf(stderr, "phase 1 done after %lld ms\n", static_cast<long long>(timer.elapsed())); fflush(stderr); }
        Calculation calc(book, options);
        std::atomic<bool> stop{false};
        // a recalculation in slices of a few million steps, stopped when the time is out (a formula may take as long as the bounds let it, but not all of them in one go)
        const auto settle = [&]() {
            for (;;) {
                const Calculation::Progress p = calc.recalculate(3000000, &stop);
                if (p.finished || p.cancelled || timer.elapsed() > seconds * 1000.0) break;
            }
        };
        calc.load();
        if (qEnvironmentVariableIsSet("JCRUNCH_TRACE")) { fprintf(stderr, "loaded after %lld ms\n", static_cast<long long>(timer.elapsed())); fflush(stderr); }
        settle();
        if (qEnvironmentVariableIsSet("JCRUNCH_TRACE")) { fprintf(stderr, "calculated after %lld ms\n", static_cast<long long>(timer.elapsed())); fflush(stderr); }
        struct At {
            int sheet;
            quint32 row, col;
        };
        std::vector<At> places;
        for (int si = 0; si < source.sheetCount() && places.size() < 4000; ++si)
            calc.sheet(si).cells.forEach([&](const Cell &c) {
                if (c.hasFormula() && places.size() < 4000) places.push_back({si, c.row, c.col});
            });
        if (places.empty()) places.push_back({0, 0, 0});
        while (timer.elapsed() < seconds * 1000.0 && edits < count) {
            At at = places[rng() % places.size()];
            if (rng() % 4 == 0) at = {int(rng() % quint32(source.sheetCount())), rng() % 300, rng() % 40};
            const bool trace = qEnvironmentVariableIsSet("JCRUNCH_TRACE");
            if (trace) { fprintf(stderr, "edit %lld at %d:%u:%u\n", static_cast<long long>(edits), at.sheet, at.row, at.col); fflush(stderr); }
            switch (rng() % 6) {
            case 0: calc.setValue(at.sheet, at.row, at.col, jp::calc::Value::number(double(int(rng() % 2000)) / 7.0)); break;
            case 1: calc.setValue(at.sheet, at.row, at.col, jp::calc::Value::text(QStringLiteral("t%1").arg(rng() % 100))); break;
            case 2: calc.setValue(at.sheet, at.row, at.col, jp::calc::Value::error(jp::calc::ErrorKind(rng() % 7))); break;
            case 3: calc.clearCell(at.sheet, at.row, at.col); break;
            default: {
                const QString f = damaged();
                if (trace) { fprintf(stderr, "  formula %s\n", qPrintable(f.left(300))); fflush(stderr); }
                calc.setFormula(at.sheet, at.row, at.col, f);
                break;
            }
            }
            ++edits;
            if (rng() % 3 == 0) {
                ++sliced;
                stop = false;
                calc.recalculate(1 + qint64(rng() % 4000), &stop);
                if (rng() % 2) {
                    stop = true;
                    calc.recalculate(0, &stop);
                }
            }
            if (rng() % 2) settle();
        }
        stop = false;
        settle();
    }
    out() << "mutate: " << tried << " formulas damaged, " << parsed << " parsed, " << worked << " worked out, " << edits << " edits in " << sliced << " slices, "
          << QString::number(timer.elapsed() / 1000.0, 'f', 1) << " s\n";
    return 0;
}

int listFunctions()
{
    for (int i = 0; i < jp::calc::functionCount(); ++i) {
        const jp::calc::FunctionInfo &f = jp::calc::functionAt(i);
        out() << f.name << '\t' << int(f.minArgs) << '\t' << (f.maxArgs == 255 ? QStringLiteral("any") : QString::number(f.maxArgs)) << '\t' << ((f.flags & jp::calc::kFnVolatile) ? "v" : "")
              << ((f.flags & jp::calc::kFnFuture) ? "f" : "") << "\n";
    }
    out() << jp::calc::functionCount() << " functions\n";
    return 0;
}

int evalFormula(const QString &path, const QString &sheetArg, const QString &cell, const QString &formula)
{
    QString why;
    Workbook book;
    if (!book.openFile(path, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    WorkbookSource source(book);
    source.loadAll();
    int sheet = source.sheetIndex(sheetArg);
    if (sheet < 0) sheet = sheetArg.toInt() - 1;
    quint32 row = 0, col = 0;
    if (sheet < 0 || sheet >= source.sheetCount() || !parseCellRef(cell, &row, &col)) {
        err() << "jcrunchtool: no such sheet or cell\n";
        return 2;
    }
    const jp::calc::ParseResult parsed = jp::calc::parseFormula(formula);
    if (!parsed.ok()) {
        out() << "!parse: " << parsed.error << " at " << parsed.errorPos << "\n";
        return 2;
    }
    jp::calc::Evaluator evaluator;
    jp::calc::EvalContext ctx;
    ctx.source = &source;
    ctx.sheet = sheet;
    ctx.row = row;
    ctx.col = col;
    ctx.dateSystem = book.info().dateSystem;
    ctx.now = 45366.5;
    out() << evaluator.evaluate(*parsed.program, ctx).debug() << (parsed.program->supported() ? "" : "  (not all of it is worked out)") << "\n";
    return 0;
}

int formats(const QString &keys, const QString &outDir, const QString &jobs)
{
#ifndef Q_OS_LINUX
    Q_UNUSED(keys);
    Q_UNUSED(outDir);
    Q_UNUSED(jobs);
    err() << "jcrunchtool: formats runs on Linux only (it caps each workbook's memory with systemd-run); formats-one works on every system.\n";
    return 2;
#else
    QString script = QCoreApplication::applicationDirPath() + QStringLiteral("/../tools/crunch_formats.py");
#ifdef JCRUNCH_TOOLS_DIR
    if (!QFileInfo::exists(script)) script = QStringLiteral(JCRUNCH_TOOLS_DIR) + QStringLiteral("/crunch_formats.py");
#endif
    if (!QFileInfo::exists(script)) {
        err() << "jcrunchtool: crunch_formats.py is not beside the program\n";
        return 1;
    }
    QProcess python;
    python.setProcessChannelMode(QProcess::ForwardedChannels);
    QStringList args = {script, QCoreApplication::applicationFilePath(), keys, outDir};
    if (!jobs.isEmpty()) args << jobs;
    python.start(QStringLiteral("python3"), args);
    if (!python.waitForStarted()) {
        err() << "jcrunchtool: python3 could not be started, which formats needs\n";
        return 1;
    }
    python.waitForFinished(-1);
    return python.exitStatus() == QProcess::NormalExit ? python.exitCode() : 1;
#endif
}

int recalc(const QString &keys, const QString &outDir, const QString &jobs, const QString &scriptName = QStringLiteral("crunch_recalc.py"))
{
#ifndef Q_OS_LINUX
    Q_UNUSED(keys);
    Q_UNUSED(outDir);
    Q_UNUSED(jobs);
    err() << "jcrunchtool: recalc runs on Linux only (it caps each workbook's memory with systemd-run); recalc-one works on every system.\n";
    return 2;
#else
    QString script = QCoreApplication::applicationDirPath() + QStringLiteral("/../tools/") + scriptName;
#ifdef JCRUNCH_TOOLS_DIR
    if (!QFileInfo::exists(script)) script = QStringLiteral(JCRUNCH_TOOLS_DIR) + QLatin1Char('/') + scriptName;
#endif
    if (!QFileInfo::exists(script)) {
        err() << "jcrunchtool: " << scriptName << " is not beside the program\n";
        return 1;
    }
    QProcess python;
    python.setProcessChannelMode(QProcess::ForwardedChannels);
    QStringList args = {script, QCoreApplication::applicationFilePath(), keys, outDir};
    if (!jobs.isEmpty()) args << jobs;
    python.start(QStringLiteral("python3"), args);
    if (!python.waitForStarted()) {
        err() << "jcrunchtool: python3 could not be started, which recalc needs\n";
        return 1;
    }
    python.waitForFinished(-1);
    return python.exitStatus() == QProcess::NormalExit ? python.exitCode() : 1;
#endif
}

int scan(const QString &keys, const QString &outDir, const QString &jobs)
{
#ifndef Q_OS_LINUX
    Q_UNUSED(keys);
    Q_UNUSED(outDir);
    Q_UNUSED(jobs);
    err() << "jcrunchtool: scan runs on Linux only (it caps each workbook's memory with systemd-run); info and dump work on every system.\n";
    return 2;
#else
    QString script = QCoreApplication::applicationDirPath() + QStringLiteral("/../tools/crunch_scan.py");
#ifdef JCRUNCH_TOOLS_DIR
    if (!QFileInfo::exists(script)) script = QStringLiteral(JCRUNCH_TOOLS_DIR) + QStringLiteral("/crunch_scan.py");
#endif
    if (!QFileInfo::exists(script)) {
        err() << "jcrunchtool: crunch_scan.py is not beside the program\n";
        return 1;
    }
    QProcess python;
    python.setProcessChannelMode(QProcess::ForwardedChannels);
    QStringList args = {script, QCoreApplication::applicationFilePath(), keys, outDir};
    if (!jobs.isEmpty()) args << jobs;
    python.start(QStringLiteral("python3"), args);
    if (!python.waitForStarted()) {
        err() << "jcrunchtool: python3 could not be started, which scan needs\n";
        return 1;
    }
    python.waitForFinished(-1);
    return python.exitStatus() == QProcess::NormalExit ? python.exitCode() : 1;
#endif
}

int usage()
{
    err() << "usage: jcrunchtool info [--tsv] <file.xlsx|file.xls>\n"
             "       jcrunchtool dump <file.xlsx|file.xls> [sheet=1] [max=100]\n"
             "       jcrunchtool scan <keys.txt> <out-dir> [jobs]\n"
             "       jcrunchtool format <code> <number> [1904]\n"
             "       jcrunchtool formats-one <file.xlsx|file.xls>\n"
             "       jcrunchtool formats <keys.txt> <out-dir> [jobs]\n"
             "       jcrunchtool recalc-one <file.xlsx|file.xls> [matches=3] [seconds=100]\n"
             "       jcrunchtool mutate <file.xlsx|file.xls> [count=3000] [seed=1] [seconds=60]\n"
             "       jcrunchtool names <file.xlsx|file.xls>\n"
             "       jcrunchtool compare-one [--first-sheet] [--values-only] <ours.xls|.ods|.csv> <theirs.xlsx>\n"
             "       jcrunchtool convert-fuzz <file.ods|file.fods|file.csv> [count=500] [seed=1] [seconds=60]\n"
             "       jcrunchtool xls-fuzz <file.xls> [count=500] [seed=1] [seconds=60]\n"
             "       jcrunchtool filter-roundtrip <file.xlsx> [out.xlsx]\n"
             "       jcrunchtool xls-save-check <file.xls>\n"
             "       jcrunchtool functions\n"
             "       jcrunchtool eval <file.xlsx|file.xls> <sheet> <cell> <formula>\n"
             "       jcrunchtool recalc <keys.txt> <out-dir> [jobs]\n"
             "       jcrunchtool recalc-full-one <file.xlsx|file.xls> [matches=3] [seconds=100]\n"
             "       jcrunchtool recalc-full <keys.txt> <out-dir> [jobs]\n"
             "       jcrunchtool grid-one [--full] <file.xlsx|file.xls>\n"
             "       jcrunchtool grid-window <file.xlsx|file.xls>\n"
             "       jcrunchtool grid-bench [rows=100000] [cols=20] [ruled]\n"
             "       jcrunchtool grid-sweep <keys.txt> <out-dir> [jobs]\n"
             "       jcrunchtool grid-drawings-one [--live] <file.xlsx>\n"
             "       jcrunchtool grid-drawings-shot <file.xlsx> <out.png> [sheet] [WxH]\n"
             "       jcrunchtool grid-drawings-bench [charts=400] [shapes=0]\n"
             "       jcrunchtool grid-drawings <keys.txt> <out-dir> [jobs]\n"
             "       jcrunchtool print-pdf <file> <out.pdf> [--sheet N]\n"
             "       jcrunchtool print-sheets <file> <out-dir> [N ...]\n"
             "       jcrunchtool print-cuts <file> [N ...]\n"
             "       jcrunchtool print-shots <out-dir> [file]\n"
             "       jcrunchtool print-bench [rows=100000] [cols=20]\n"
             "       jcrunchtool charts-roundtrip <in.xlsx> <out.xlsx>\n"
             "       jcrunchtool charts-insert <in.xlsx> <out.xlsx> <sheet> <range> <kind> [kind...]\n"
             "       jcrunchtool charts-shots <in.xlsx> <out-dir>\n";
    return 2;
}

} // namespace

namespace jp::crunchtool {
int compareMain(const QStringList &args);   // (jcrunchtool_compare.cpp)
int xlsFuzzMain(const QStringList &args);   // (jcrunchtool_xlsfuzz.cpp)
int odsFuzzMain(const QStringList &args);   // (jcrunchtool_odsfuzz.cpp)
int xlsSaveCheckMain(const QStringList &args);   // (jcrunchtool_xlssave.cpp)
int filterRoundtripMain(const QStringList &args);   // (jcrunchtool_filter.cpp)
}

int main(int argc, char **argv)
{
    qputenv("JP_NO_TELEMETRY", "1");
#ifdef JC_GRID
    // the grid commands draw: they need the program's own window system parts (offscreen unless the person chose another)
    if (argc > 1 && (QByteArray(argv[1]).startsWith("grid-") || QByteArray(argv[1]).startsWith("roundtrip") || QByteArray(argv[1]).startsWith("edit-") || QByteArray(argv[1]).startsWith("print-") || QByteArray(argv[1]).startsWith("charts-"))) {
        if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "offscreen");
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
        ::jp::setAppInfo(::jp::crunchAppInfo());
        QApplication gui(argc, argv);
        // (JC_CRASH_REPORT names a file for the report of a crash: where the program stopped, which addr2line turns to lines)
        if (!qEnvironmentVariableIsEmpty("JC_CRASH_REPORT")) ::jp::crash::install(qEnvironmentVariable("JC_CRASH_REPORT"));
        ::jp::initCore();   // the program's own fonts, as JeffCrunch loads them
        int code = ::jp::crunchtool::gridMain(gui.arguments().mid(1));
        if (code < 0) code = ::jp::crunchtool::editMain(gui.arguments().mid(1));
        if (code < 0) code = ::jp::crunchtool::drawingsMain(gui.arguments().mid(1));
        if (code < 0) code = ::jp::crunchtool::printMain(gui.arguments().mid(1));
        if (code < 0) code = ::jp::crunchtool::chartsMain(gui.arguments().mid(1));
        return code >= 0 ? code : usage();
    }
#endif
    QCoreApplication app(argc, argv);
    const QStringList args = app.arguments().mid(1);
    if (args.size() >= 2 && args[0] == QLatin1String("info")) {
        const bool tsv = args[1] == QLatin1String("--tsv");
        if (args.size() == (tsv ? 3 : 2)) return info(args.last(), tsv);
    }
    if (args.size() == 2 && args[0] == QLatin1String("names")) return names(args[1]);
    if (args.size() >= 3 && args[0] == QLatin1String("compare-one")) return jp::crunchtool::compareMain(args.mid(1));
    if (args.size() >= 2 && args.size() <= 5 && args[0] == QLatin1String("convert-fuzz")) {
        const int code = jp::crunchtool::odsFuzzMain(args.mid(1));
        if (code >= 0) return code;
    }
    if (args.size() >= 2 && args.size() <= 5 && args[0] == QLatin1String("xls-fuzz")) {
        const int code = jp::crunchtool::xlsFuzzMain(args.mid(1));
        if (code >= 0) return code;
    }
    if ((args.size() == 2 || args.size() == 3) && args[0] == QLatin1String("filter-roundtrip")) {
        const int code = jp::crunchtool::filterRoundtripMain(args.mid(1));
        if (code >= 0) return code;
    }
    if (args.size() == 2 && args[0] == QLatin1String("xls-save-check")) {
        const int code = jp::crunchtool::xlsSaveCheckMain(args.mid(1));
        if (code >= 0) return code;
    }
    if (args.size() >= 2 && args.size() <= 4 && args[0] == QLatin1String("dump")) return dump(args[1], args.value(2, QStringLiteral("1")).toInt(), args.value(3, QStringLiteral("100")).toLongLong());
    if ((args.size() == 3 || args.size() == 4) && args[0] == QLatin1String("scan")) return scan(args[1], args[2], args.value(3));
    if ((args.size() == 3 || args.size() == 4) && args[0] == QLatin1String("format")) return formatOne(args[1], args[2], args.value(3) == QLatin1String("1904"));
    if (args.size() == 2 && args[0] == QLatin1String("formats-one")) return formatsOne(args[1]);
    if ((args.size() == 3 || args.size() == 4) && args[0] == QLatin1String("formats")) return formats(args[1], args[2], args.value(3));
    if (args.size() >= 2 && args.size() <= 4 && args[0] == QLatin1String("recalc-one")) return recalcOne(args[1], args.value(2, QStringLiteral("3")).toInt(), args.value(3, QStringLiteral("100")).toDouble());
    if ((args.size() == 3 || args.size() == 4) && args[0] == QLatin1String("recalc")) return recalc(args[1], args[2], args.value(3));
    if (args.size() >= 2 && args.size() <= 4 && args[0] == QLatin1String("recalc-full-one")) return recalcFullOne(args[1], args.value(2, QStringLiteral("3")).toInt(), args.value(3, QStringLiteral("100")).toDouble());
    if ((args.size() == 3 || args.size() == 4) && args[0] == QLatin1String("recalc-full")) return recalc(args[1], args[2], args.value(3), QStringLiteral("crunch_recalc_full.py"));
    if (args.size() == 1 && args[0] == QLatin1String("functions")) return listFunctions();
    if (args.size() >= 2 && args.size() <= 5 && args[0] == QLatin1String("mutate"))
        return mutate(args[1], args.value(2, QStringLiteral("3000")).toInt(), args.value(3, QStringLiteral("1")).toUInt(), args.value(4, QStringLiteral("60")).toDouble());
    if (args.size() == 5 && args[0] == QLatin1String("eval")) return evalFormula(args[1], args[2], args[3], args[4]);
    return usage();
}
