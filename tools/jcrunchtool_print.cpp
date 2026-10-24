// The printing commands of jcrunchtool (docs/jeffcrunch.md): a workbook's pages as a PDF without a window, and where the pages are cut.
//   jcrunchtool print-pdf <file> <out.pdf> [--sheet N]       the pages of sheet N (counted from 1), or of every sheet that is shown, in a PDF; one line: pages, sheets, milliseconds
//   jcrunchtool print-sheets <file> <out-dir> [N ...]        each of the sheets named (all the sheets of cells if none), alone, as <out-dir>/sheet-NN.pdf, and <out-dir>/cuts.tsv with one line
//                                                            for each page: sheet, its name, the scale, the page's place, the rows and columns it holds, and its titles (what the scoring
//                                                            of the pages against the reference pages compares); one line: sheets, pages, milliseconds
//   jcrunchtool print-cuts <file> [N ...]                    the same lines as cuts.tsv, to the output

#include "core/loader.h"
#include "core/printjob.h"
#include "core/sheetobjects.h"
#include "sheet/cellref.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QTextStream>

using namespace jp;

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

// A workbook read whole, with the objects of its sheets, ready to be printed.
struct Opened {
    crunch::LoadedBook book;
    std::unique_ptr<crunch::SheetObjects> objects;
    bool open(const QString &path, QString *why)
    {
        if (!crunch::loadFirstScreen(path, &book, [] { return false; }, why)) return false;
        if (!crunch::loadWhole(&book, [] { return false; }, nullptr)) {
            if (why) *why = QStringLiteral("not read whole");
            return false;
        }
        objects = std::make_unique<crunch::SheetObjects>();
        objects->open(book.book);
        objects->setCells(book.calc.get(), nullptr);
        return true;
    }
    std::unique_ptr<crunch::PrintJob> job(const QString &path, crunch::PrintJobOptions options, QString *why)
    {
        options.fileName = QFileInfo(path).fileName();
        options.folder = QFileInfo(path).absolutePath();
        auto source = std::make_unique<crunch::BookPrintSource>(&book, nullptr, nullptr, objects.get());
        auto j = std::make_unique<crunch::PrintJob>(std::move(source), options);
        if (!j->build(why)) return nullptr;
        return j;
    }
};

QVector<int> sheetsAsked(const QStringList &args, const Opened &o)
{
    QVector<int> sheets;
    for (const QString &a : args) {
        bool ok = false;
        const int n = a.toInt(&ok);
        if (ok && n >= 1 && n <= o.book.book->info().sheets.size()) sheets << n - 1;
    }
    if (sheets.isEmpty() && args.isEmpty())
        for (int i = 0; i < o.book.book->info().sheets.size(); ++i)
            if (o.book.book->info().sheets[i].kind == jp::office::sheet::SheetKind::Worksheet) sheets << i;
    return sheets;
}

QString cutLine(int sheet, const QString &name, const jp::grid::SheetPrint &p, int page)
{
    const jp::grid::PageCut &c = p.pages[size_t(page)];
    return QStringLiteral("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14")
        .arg(sheet + 1).arg(name).arg(p.scalePercent).arg(page + 1).arg(c.row1 + 1).arg(c.row2 + 1).arg(c.col1 + 1).arg(c.col2 + 1)
        .arg(c.titleRow1 >= 0 ? c.titleRow1 + 1 : 0).arg(c.titleRow2 >= 0 ? c.titleRow2 + 1 : 0).arg(c.titleCol1 >= 0 ? c.titleCol1 + 1 : 0).arg(c.titleCol2 >= 0 ? c.titleCol2 + 1 : 0)
        .arg(int(p.paper.width)).arg(int(p.paper.height));
}

int printPdf(const QString &path, const QString &target, int sheet)
{
    QElapsedTimer t;
    t.start();
    Opened o;
    QString why;
    if (!o.open(path, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    crunch::PrintJobOptions options;
    if (sheet >= 1) {
        if (sheet > o.book.book->info().sheets.size()) {
            err() << path << ": there is no sheet " << sheet << "\n";
            return 2;
        }
        options.what = crunch::PrintWhat::ActiveSheet;
        options.activeSheet = sheet - 1;
    } else {
        options.what = crunch::PrintWhat::Workbook;
    }
    const std::unique_ptr<crunch::PrintJob> job = o.job(path, options, &why);
    if (!job) {
        err() << path << ": " << why << "\n";
        return 3;
    }
    QVector<int> pages;
    for (int i = 0; i < job->pageCount(); ++i) pages << i;
    if (!job->exportPdf(target, pages, &why)) {
        err() << path << ": " << why << "\n";
        return 4;
    }
    int sheets = 0, last = -2;
    for (int i = 0; i < job->pageCount(); ++i)
        if (job->sheetOf(i) != last) {
            last = job->sheetOf(i);
            ++sheets;
        }
    out() << job->pageCount() << "\t" << sheets << "\t" << t.elapsed() << "\n";
    for (const QString &m : job->messages()) err() << m << "\n";
    return 0;
}

int printSheets(const QString &path, const QString &dir, const QStringList &which, bool pdf)
{
    QElapsedTimer t;
    t.start();
    Opened o;
    QString why;
    if (!o.open(path, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    if (pdf) QDir().mkpath(dir);
    QStringList lines;
    int pagesTotal = 0, done = 0;
    for (const int sheet : sheetsAsked(which, o)) {
        crunch::PrintJobOptions options;
        options.what = crunch::PrintWhat::ActiveSheet;
        options.activeSheet = sheet;
        const std::unique_ptr<crunch::PrintJob> job = o.job(path, options, &why);
        if (!job) continue;
        const QString name = o.book.book->info().sheets[sheet].name;
        if (const jp::grid::SheetPrint *p = job->printOf(sheet))
            for (size_t i = 0; i < p->pages.size(); ++i) lines << cutLine(sheet, name, *p, int(i));
        pagesTotal += job->pageCount();
        ++done;
        if (pdf) {
            QVector<int> pages;
            for (int i = 0; i < job->pageCount(); ++i) pages << i;
            if (!job->exportPdf(QDir(dir).filePath(QStringLiteral("sheet-%1.pdf").arg(sheet + 1, 2, 10, QLatin1Char('0'))), pages, &why)) err() << path << ": sheet " << sheet + 1 << ": " << why << "\n";
        }
    }
    if (pdf) {
        QFile f(QDir(dir).filePath(QStringLiteral("cuts.tsv")));
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) f.write((lines.join(QLatin1Char('\n')) + QLatin1Char('\n')).toUtf8());
        out() << done << "\t" << pagesTotal << "\t" << t.elapsed() << "\n";
    } else {
        for (const QString &l : lines) out() << l << "\n";
    }
    return 0;
}

} // namespace

namespace jp::crunchtool {

int printMain(const QStringList &args)
{
    if (args.isEmpty()) return -1;
    if (args[0] == QLatin1String("print-pdf") && (args.size() == 3 || (args.size() == 5 && args[3] == QLatin1String("--sheet")))) return printPdf(args[1], args[2], args.size() == 5 ? args[4].toInt() : 0);
    if (args[0] == QLatin1String("print-sheets") && args.size() >= 3) return printSheets(args[1], args[2], args.mid(3), true);
    if (args[0] == QLatin1String("print-cuts") && args.size() >= 2) return printSheets(args[1], QString(), args.mid(2), false);
    return -1;
}

} // namespace jp::crunchtool
