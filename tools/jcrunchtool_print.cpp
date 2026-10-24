// The printing commands of jcrunchtool (docs/jeffcrunch.md): a workbook's pages as a PDF without a window, and where the pages are cut.
//   jcrunchtool print-pdf <file> <out.pdf> [--sheet N]       the pages of sheet N (counted from 1), or of every sheet that is shown, in a PDF; one line: pages, sheets, milliseconds, then a line
//                                                            "S", the sheet, and its pages for each sheet that has pages
//   jcrunchtool print-sheets <file> <out-dir> [N ...]        each of the sheets named (all the sheets of cells if none), alone, as <out-dir>/sheet-NN.pdf, and <out-dir>/cuts.tsv with one line
//                                                            for each page: sheet, its name, the scale, the page's place, the rows and columns it holds, and its titles (what the scoring
//                                                            of the pages against the reference pages compares); one line: sheets, pages, milliseconds
//   jcrunchtool print-cuts <file> [N ...]                    the same lines as cuts.tsv, to the output
//   jcrunchtool print-shots <out-dir> [file]                 pictures of the printing screens of a workbook (a report made here without a file): the Print screen with its preview, the Page Setup
//                                                            dialog on each of its four tabs, and the sheet with the lines where its pages begin, and a printed page with a header and a footer
//   jcrunchtool print-bench [rows=100000] [cols=20]          a workbook made here (JC_BENCH_FILE names a file to keep it in and to use again), its pages cut, the first page drawn, 40 pages
//                                                            drawn as a person turns them, and every page written to a PDF: the milliseconds each took

#include "app/crunchwindow.h"
#include "app/pagesetupdialog.h"
#include "core/editsession.h"
#include "core/loader.h"
#include "core/printjob.h"
#include "grid/gridview.h"
#include "office/package.h"
#include "core/sheetobjects.h"
#include "sheet/cellref.h"

#include <algorithm>
#include <cmath>

#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QProcess>
#include <QTimer>
#include <QTemporaryDir>
#include <QMap>
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
        // JP_NOW (an ISO 8601 date and time, say 2026-10-15T19:31:36-04:00) is the moment the date and time fields show, so a page can be drawn again as of the day a reference was made
        // (a tool's; the app does not read it).
        if (const QDateTime moment = QDateTime::fromString(qEnvironmentVariable("JP_NOW"), Qt::ISODate); moment.isValid()) options.now = moment;
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
    QMap<int, int> perSheet;
    for (int i = 0; i < job->pageCount(); ++i) {
        if (job->sheetOf(i) != last) {
            last = job->sheetOf(i);
            ++sheets;
        }
        ++perSheet[job->sheetOf(i)];
    }
    out() << job->pageCount() << "\t" << sheets << "\t" << t.elapsed() << "\n";
    for (auto it = perSheet.begin(); it != perSheet.end(); ++it) out() << "S\t" << it.key() + 1 << "\t" << it.value() << "\n";
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

// ---- a workbook made here: rows of numbers and text, a header row, a number format, and a column of formulas (the shape of the grid's benchmark)

QByteArray benchWorkbook(int rows, int cols)
{
    QByteArray sheet;
    sheet.reserve(qsizetype(rows) * cols * 28);
    sheet += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
             "<cols><col min=\"1\" max=\"1\" width=\"14.7109375\" customWidth=\"1\"/></cols><sheetData>";
    auto letter = [](int c) {
        QByteArray s;
        for (int n = c; n >= 0; n = n / 26 - 1) s.prepend(char('A' + n % 26));
        return s;
    };
    sheet += "<row r=\"1\">";
    for (int c = 0; c < cols; ++c) sheet += "<c r=\"" + letter(c) + "1\" s=\"1\" t=\"inlineStr\"><is><t>Column " + QByteArray::number(c + 1) + "</t></is></c>";
    sheet += "</row>";
    for (int r = 2; r <= rows; ++r) {
        sheet += "<row r=\"" + QByteArray::number(r) + "\">";
        for (int c = 0; c < cols; ++c) {
            const QByteArray ref = letter(c) + QByteArray::number(r);
            if (c == 0) sheet += "<c r=\"" + ref + "\" t=\"inlineStr\"><is><t>Row " + QByteArray::number(r) + "</t></is></c>";
            else sheet += "<c r=\"" + ref + "\" s=\"2\"><v>" + QByteArray::number(double((r * 3 + c) % 1000) / 4) + "</v></c>";
        }
        sheet += "</row>";
    }
    sheet += "</sheetData></worksheet>";
    const char *main = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
    const QByteArray styles = QByteArray("<?xml version=\"1.0\"?>\n<styleSheet xmlns=\"") + main +
                              "\"><numFmts count=\"1\"><numFmt numFmtId=\"164\" formatCode=\"#,##0.00\"/></numFmts><fonts count=\"2\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font>"
                              "<font><b/><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts><fills count=\"3\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>"
                              "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FFD9EAD3\"/></patternFill></fill></fills>"
                              "<borders count=\"2\"><border><left/><right/><top/><bottom/><diagonal/></border>"
                              "<border><left/><right/><top/><bottom style=\"medium\"><color rgb=\"FF13773A\"/></bottom><diagonal/></border></borders><cellStyleXfs count=\"1\"><xf/></cellStyleXfs>"
                              "<cellXfs count=\"3\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/><xf numFmtId=\"0\" fontId=\"1\" fillId=\"2\" borderId=\"1\"/>"
                              "<xf numFmtId=\"164\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellXfs></styleSheet>";
    jp::office::PackageWriter w;
    const QString relBase = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    w.addPart(QStringLiteral("/xl/workbook.xml"), QByteArray(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    w.addRelationship(QStringLiteral("/"), relBase + QStringLiteral("officeDocument"), QStringLiteral("/xl/workbook.xml"));
    w.addPart(QStringLiteral("/xl/worksheets/sheet1.xml"), sheet, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
    const QString id = w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + QStringLiteral("worksheet"), QStringLiteral("/xl/worksheets/sheet1.xml"));
    w.addPart(QStringLiteral("/xl/styles.xml"), styles, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"));
    w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + QStringLiteral("styles"), QStringLiteral("/xl/styles.xml"));
    const QByteArray workbook = QByteArray("<?xml version=\"1.0\"?>\n<workbook xmlns=\"") + main + "\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><sheets><sheet name=\"Data\" sheetId=\"1\" r:id=\"" +
                                id.toUtf8() + "\"/></sheets></workbook>";
    w.addPart(QStringLiteral("/xl/workbook.xml"), workbook, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    return w.finish();
}

double percentile(std::vector<double> v, double p)
{
    if (v.empty()) return 0;
    std::sort(v.begin(), v.end());
    return v[std::min(v.size() - 1, size_t(std::floor(p * double(v.size()))))];
}

void spin(int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, &QEventLoop::quit);
    loop.exec();
}

int printShots(const QString &dir, const QString &file)
{
    QDir().mkpath(dir);
    QTemporaryDir tmp;
    QString path = file;
    if (path.isEmpty()) {
        path = tmp.filePath(QStringLiteral("Household budget.xlsx"));
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly) || f.write(benchWorkbook(90, 9)) < 0) return 1;
    }
    CrunchWindow w;
    w.setNotifier([](const QString &, const QString &) {});
    w.resize(1280, 800);
    w.show();
    bool loaded = false, done = false;
    QObject::connect(&w, &CrunchWindow::loadFinished, &w, [&] { loaded = true; });
    QObject::connect(&w, &CrunchWindow::calculationFinished, &w, [&] { done = true; });
    if (!w.openFile(path)) return 1;
    QElapsedTimer clock;
    clock.start();
    while (!(loaded && done) && clock.elapsed() < 60000) spin(5);
    // the setup the pictures show: a heading over the page, a footer with the numbers, the first row on every page, lines between the cells, and the page fitted across
    jp::office::sheet::PageSetup p = w.session()->pages().sheet(0);
    p.headerFooter.oddHeader = QStringLiteral("&L&\"Calibri,Bold\"&14Household budget&R&D");
    p.headerFooter.oddFooter = QStringLiteral("&L&A&CPage &P of &N&R&F");
    p.titles.hasRows = true;
    p.titles.row1 = p.titles.row2 = 0;
    p.gridLines = true;
    p.fitToPage = true;
    p.fitToWidth = 1;
    p.fitToHeight = 0;
    p.margins.header = 0.4;
    p.margins.footer = 0.4;
    w.session()->setPageSetup(0, p, QStringLiteral("Page"));
    w.showFileScreen(QStringLiteral("print"));
    spin(600);
    w.grab().save(QDir(dir).filePath(QStringLiteral("print-screen.png")));
    w.hideFileScreen();
    w.view()->ensureVisible(60, 0);
    spin(400);
    w.grab().save(QDir(dir).filePath(QStringLiteral("sheet-with-page-breaks.png")));
    static const char *const tabs[4] = {"page", "margins", "header-footer", "sheet"};
    for (int tab = 0; tab < 4; ++tab) {
        PageSetupRequest request;
        request.setup = w.session()->pages().sheet(0);
        request.sheetName = QStringLiteral("Budget");
        request.fileName = QStringLiteral("Household budget.xlsx");
        request.selectionText = QStringLiteral("A1:G13");
        request.tab = tab;
        PageSetupDialog dialog(request);
        dialog.resize(520, 560);
        dialog.show();
        spin(100);
        dialog.grab().save(QDir(dir).filePath(QStringLiteral("page-setup-%1.png").arg(QLatin1String(tabs[tab]))));
    }
    QString why;
    crunch::PrintJobOptions options;
    const std::unique_ptr<crunch::PrintJob> job = w.makePrintJob(options, &why);
    if (!job) {
        err() << why << "\n";
        return 2;
    }
    const QString pdf = QDir(dir).filePath(QStringLiteral("page-with-header-and-footer.pdf"));
    if (!job->exportPdf(pdf, {0, 1}, &why)) {
        err() << why << "\n";
        return 3;
    }
    QProcess::execute(QStringLiteral("pdftoppm"), {QStringLiteral("-r"), QStringLiteral("110"), QStringLiteral("-f"), QStringLiteral("2"), QStringLiteral("-l"), QStringLiteral("2"), QStringLiteral("-png"), pdf,
                                                   QDir(dir).filePath(QStringLiteral("page-with-header-and-footer"))});
    return 0;
}

int printBench(int rows, int cols)
{
    QTemporaryDir dir;
    const QString keep = qEnvironmentVariable("JC_BENCH_FILE");
    const QString path = keep.isEmpty() ? dir.filePath(QStringLiteral("bench.xlsx")) : keep;
    if (keep.isEmpty() || !QFile::exists(keep)) {
        QElapsedTimer made;
        made.start();
        const QByteArray bytes = benchWorkbook(rows, cols);
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size()) {
            err() << "print-bench: the workbook could not be written\n";
            return 1;
        }
        f.close();
        out() << "workbook of " << rows << " rows by " << cols << " columns: " << bytes.size() / 1048576 << " MB, made in " << made.elapsed() << " ms\n";
    }
    QElapsedTimer t;
    t.start();
    Opened o;
    QString why;
    if (!o.open(path, &why)) {
        err() << why << "\n";
        return 1;
    }
    out() << "  read whole: " << t.elapsed() << " ms\n";
    crunch::PrintJobOptions options;
    options.what = crunch::PrintWhat::ActiveSheet;
    options.activeSheet = 0;
    t.restart();
    const std::unique_ptr<crunch::PrintJob> job = o.job(path, options, &why);
    if (!job) {
        err() << why << "\n";
        return 2;
    }
    const qint64 cutMs = t.elapsed();
    t.restart();
    const QImage first = job->preview(0, 900);
    const qint64 firstMs = t.elapsed();
    out() << "  pages " << job->pageCount() << ", scale " << job->printOf(0)->scalePercent << "%: cut in " << cutMs << " ms, first page drawn in " << firstMs << " ms (so the first page is on the screen "
          << cutMs + firstMs << " ms after Print is chosen)\n";
    // pages turned one after another through the sheet, from the start and from the middle
    std::vector<double> flips;
    const int count = job->pageCount();
    for (int i = 1; i <= 40; ++i) {
        const int page = i <= 20 ? i % count : (count / 2 + i) % count;
        QElapsedTimer f;
        f.start();
        const QImage img = job->preview(page, 900);
        flips.push_back(double(f.nsecsElapsed()) / 1e6);
        if (img.isNull()) return 3;
    }
    out() << "  40 pages turned: middle " << QString::number(percentile(flips, 0.5), 'f', 1) << " ms, 95th percentile " << QString::number(percentile(flips, 0.95), 'f', 1) << " ms, longest "
          << QString::number(*std::max_element(flips.begin(), flips.end()), 'f', 1) << " ms\n";
    QVector<int> all;
    for (int i = 0; i < count; ++i) all << i;
    t.restart();
    const QString pdf = dir.filePath(QStringLiteral("bench.pdf"));
    if (!job->exportPdf(pdf, all, &why)) {
        err() << why << "\n";
        return 4;
    }
    out() << "  all " << count << " pages in a PDF: " << t.elapsed() << " ms (" << QFileInfo(pdf).size() / 1048576 << " MB)\n";
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
    if (args[0] == QLatin1String("print-shots") && (args.size() == 2 || args.size() == 3)) return printShots(args[1], args.value(2));
    if (args[0] == QLatin1String("print-bench") && args.size() <= 3) return printBench(args.value(1, QStringLiteral("100000")).toInt(), args.value(2, QStringLiteral("20")).toInt());
    return -1;
}

} // namespace jp::crunchtool
