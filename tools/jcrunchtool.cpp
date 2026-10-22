// Command-line helper for JeffCrunch's readers and its yardstick (docs/jeffcrunch.md).
//   jcrunchtool info [--tsv] <file.xlsx>         the workbook's sheets, cells, formula cells, and formula cells with a stored result, the functions the formulas
//                                                call with how often, and the time the read took; --tsv is that as one line of tab-separated fields (sheets, cells,
//                                                formula cells, stored results, functions as NAME:count,..., problems, and the most that any one thing came to, for choosing limits:
//                                                cells in a sheet, XML bytes of a sheet, longest string, longest formula, shared formulas in a sheet, cells of the largest shared
//                                                range, formula characters expanded in a sheet, shared strings, their characters, sheets). Exits 0 if all was read, 2 if it was opened
//                                                but a sheet is only partly read, and 1 if it could not be opened.
//   jcrunchtool dump <file.xlsx> [sheet=1] [max=100]   the first cells of a sheet (counting from 1), one to a line: address, type, value, and formula
//   jcrunchtool scan <keys.txt> <out-dir> [jobs]   runs `info` on each workbook listed (one path to a line), each in a child process that has 120 seconds and 4 GB,
//                                                  and writes <out-dir>/books.tsv: path, exit, seconds, peak MB, sheets, cells, formula cells, stored results, and
//                                                  functions used. (tools/crunch_scan.py does the work; tools/crunch_summary.py <out-dir> sums the table up.)

#include "office/package.h"
#include "sheet/cellref.h"
#include "sheet/sheetreader.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QProcess>
#include <QTextStream>

#include <algorithm>

using namespace jp::office;
using namespace jp::office::sheet;

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
    qint64 cellsInSheet = 0, string = 0, formula = 0, sharedFormulas = 0, sharedRange = 0, expandedChars = 0, xmlBytes = 0;
    void take(const Sheet &s)
    {
        cellsInSheet = std::max(cellsInSheet, s.cells.cellCount());
        string = std::max(string, s.stats.longestString);
        formula = std::max(formula, s.stats.longestFormula);
        sharedFormulas = std::max(sharedFormulas, s.stats.sharedFormulas);
        sharedRange = std::max(sharedRange, s.stats.largestSharedRange);
        expandedChars = std::max(expandedChars, s.stats.expandedFormulaChars);
        xmlBytes = std::max(xmlBytes, s.stats.xmlBytes);
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
    Package package;
    QString why;
    if (!package.readLazyFile(path, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    Workbook book;
    if (!book.open(package, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    qint64 cells = 0, formulaCells = 0, storedResults = 0, valued = 0, worksheets = 0, merged = 0;
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
                         qint64(book.sharedStrings().strings.size()), book.sharedStrings().chars, qint64(book.info().sheets.size())})
            out() << '\t' << n;
        out() << "\n";
    } else {
        out() << "file:                  " << path << "\n";
        out() << "sheets:                " << worksheets << " worksheets (" << book.info().sheets.size() << " in the workbook), " << (book.info().dateSystem == DateSystem::Date1904 ? "1904" : "1900")
              << " dates\n";
        out() << "cells:                 " << cells << " (" << valued << " with a value, " << merged << " merged ranges)\n";
        out() << "formula cells:         " << formulaCells << "\n";
        out() << "with a stored result:  " << storedResults << "\n";
        out() << "shared strings:        " << book.sharedStrings().strings.size() << " (" << book.sharedStrings().chars << " characters)\n";
        out() << "largest:               " << largest.cellsInSheet << " cells in a sheet, " << largest.xmlBytes << " bytes of XML, a string of " << largest.string << " characters, a formula of "
              << largest.formula << ", " << largest.sharedFormulas << " shared formulas, a shared range of " << largest.sharedRange << " cells, " << largest.expandedChars
              << " formula characters expanded\n";
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
    Package package;
    QString why;
    Workbook book;
    if (!package.readLazyFile(path, &why) || !book.open(package, &why)) {
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

int scan(const QString &keys, const QString &outDir, const QString &jobs)
{
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
    if (!python.waitForFinished(-1)) {
        err() << "jcrunchtool: python3 could not be run\n";
        return 1;
    }
    return python.exitCode();
}

int usage()
{
    err() << "usage: jcrunchtool info [--tsv] <file.xlsx>\n"
             "       jcrunchtool dump <file.xlsx> [sheet=1] [max=100]\n"
             "       jcrunchtool scan <keys.txt> <out-dir> [jobs]\n";
    return 2;
}

} // namespace

int main(int argc, char **argv)
{
    qputenv("JP_NO_TELEMETRY", "1");
    QCoreApplication app(argc, argv);
    const QStringList args = app.arguments().mid(1);
    if (args.size() >= 2 && args[0] == QLatin1String("info")) {
        const bool tsv = args[1] == QLatin1String("--tsv");
        if (args.size() == (tsv ? 3 : 2)) return info(args.last(), tsv);
    }
    if (args.size() >= 2 && args.size() <= 4 && args[0] == QLatin1String("dump")) return dump(args[1], args.value(2, QStringLiteral("1")).toInt(), args.value(3, QStringLiteral("100")).toLongLong());
    if ((args.size() == 3 || args.size() == 4) && args[0] == QLatin1String("scan")) return scan(args[1], args[2], args.value(3));
    return usage();
}
