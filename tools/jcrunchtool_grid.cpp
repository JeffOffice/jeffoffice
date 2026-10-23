// The grid's part of jcrunchtool (docs/jeffcrunch.md, wave 4): how fast a workbook is on the screen, and a sweep that opens every workbook of a list in the grid model.
//   jcrunchtool grid-one [--full] <file.xlsx>      reads the first rows as the window does, builds the grid model, and draws the first screen (1,280 by 800, offscreen); one line of
//                                                  tab-separated fields: path, exit (0 read whole or first screen drawn, 1 not opened, 2 opened with a sheet only in part),
//                                                  milliseconds from the start to the first screen drawn, milliseconds to draw it again (the middle of 30, and the 95th percentile),
//                                                  the rows and columns visited, cells looked at, runs of text, and whether a bound was reached; with --full also milliseconds to read the
//                                                  whole workbook, to work all its formulas out, the longest slice of that work (what the window would wait for), and the slices.
//   jcrunchtool grid-window <file.xlsx>            the window as a person sees it: milliseconds from opening the file to the first screen drawn, to the workbook read whole, and to
//                                                  every formula worked out
//   jcrunchtool grid-bench [rows=100000] [cols=20] a workbook made here of that many rows and columns, opened in the window: those times, and then 400 scrolls by a wheel step (at 100%),
//                                                  100 page jumps, and 100 whole-window draws, with the milliseconds each draw took (the middle, the 95th percentile, the longest)
//   jcrunchtool grid-sweep <keys.txt> <out-dir> [jobs]   runs `grid-one` on each workbook listed, each in a child process that has 120 seconds and 4 GB, and writes <out-dir>/grid.tsv;
//                                                  prints how many crashed or timed out and the times (tools/crunch_grid.py does the work)

#include "app/crunchwindow.h"
#include "core/loader.h"
#include "grid/gridview.h"
#include "grid/painter.h"
#include "io/zip.h"
#include "office/package.h"

#include <QApplication>
#include <QDir>
#include <QScrollBar>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QProcess>
#include <QTemporaryDir>
#include <QTextStream>
#include <QTimer>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace jp;
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

double percentile(std::vector<double> v, double p)
{
    if (v.empty()) return 0;
    std::sort(v.begin(), v.end());
    return v[std::min(v.size() - 1, size_t(std::floor(p * double(v.size()))))];
}

QString ms(double v) { return QString::number(v, 'f', 2); }

// ---- one workbook, in the grid model

int gridOne(const QString &path, bool full)
{
    QElapsedTimer start;
    start.start();
    crunch::LoadedBook book;
    QString why;
    if (!crunch::loadFirstScreen(path, &book, [] { return false; }, &why)) {
        err() << path << ": " << why << "\n";
        return 1;
    }
    grid::GridData data;
    data.styles = &book.book->styles();
    data.strings = &book.book->sharedStrings();
    data.sheet = book.preview.get();
    data.sheetIndex = book.previewSheet;
    data.dateSystem = book.book->info().dateSystem;
    grid::GridSheet sheet;
    sheet.setData(data);
    sheet.layout().setViewport(QSize(1280, 800));
    grid::GridPainter painter(&sheet);
    QImage img(1280, 800, QImage::Format_RGB32);
    grid::PaintOptions options;
    auto draw = [&] {
        QPainter p(&img);
        painter.paint(&p, QRect(0, 0, 1280, 800), options);
    };
    draw();
    const double firstMs = double(start.nsecsElapsed()) / 1e6;
    const grid::PaintStats stats = painter.stats();
    std::vector<double> warm;
    for (int i = 0; i < 30; ++i) {
        QElapsedTimer t;
        t.start();
        draw();
        warm.push_back(double(t.nsecsElapsed()) / 1e6);
    }
    QString fullFields;
    int exit = 0;
    if (full) {
        QElapsedTimer t;
        t.start();
        if (!crunch::loadWhole(&book, [] { return false; }, nullptr)) {
            err() << path << ": the read was stopped\n";
            return 1;
        }
        const double loadMs = double(book.firstScreenMs) + double(t.nsecsElapsed()) / 1e6;
        // the whole sheet in the model, drawn again
        data.sheet = &book.calc->sheet(book.previewSheet);
        data.calc = book.calc.get();
        sheet.setData(data);
        sheet.layout().setViewport(QSize(1280, 800));
        draw();
        // the formulas, a slice at a time as the window works them (a few milliseconds each, by the clock)
        Calculation *calc = book.calc.get();
        double longest = 0, total = 0;
        int slices = 0;
        QElapsedTimer calcTimer;
        calcTimer.start();
        while (calcTimer.elapsed() < 120000) {
            QElapsedTimer s;
            s.start();
            const Calculation::Progress pr = calc->recalculate(0, nullptr, crunch::kSliceMicros);
            const double sliceMs = double(s.nsecsElapsed()) / 1e6;
            if (qEnvironmentVariableIsSet("JC_GRID_SLICES")) err() << "slice " << slices << ": work " << pr.work << ", " << ms(sliceMs) << " ms, " << pr.evaluated << " formulas\n";
            longest = std::max(longest, sliceMs);
            total += sliceMs;
            ++slices;
            if (pr.finished) break;
        }
        // formulas that took more steps than one may (they keep the result the file stored)
        qint64 bounded = 0;
        calc->forEachFormula([&](const Calculation::FormulaCell &f) {
            if (f.kept == 'L') ++bounded;
        });
        fullFields = QStringLiteral("\t%1\t%2\t%3\t%4\t%5").arg(ms(loadMs), ms(total), ms(longest)).arg(slices).arg(bounded);
        if (!book.problem.isEmpty()) exit = 2;
    }
    out() << path << '\t' << exit << '\t' << ms(firstMs) << '\t' << ms(percentile(warm, 0.5)) << '\t' << ms(percentile(warm, 0.95)) << '\t' << stats.rowsVisited << '\t' << stats.colsVisited << '\t'
          << stats.cellsLooked << '\t' << stats.textRuns << '\t' << (stats.truncated ? 1 : 0) << fullFields << "\n";
    return exit;
}

// ---- the window

struct WindowTimes {
    double firstScreenMs = -1, firstPaintMs = -1, loadMs = -1, calcMs = -1;
    bool ok = false;
};

void spin(QEventLoop *loop, int msec)
{
    QTimer::singleShot(msec, loop, &QEventLoop::quit);
    loop->exec();
}

WindowTimes timeWindow(CrunchWindow *w, const QString &path, int limitSeconds = 300)
{
    WindowTimes t;
    QElapsedTimer clock;
    bool first = false, loaded = false, done = false, ok = true;
    size_t paintsAtFirst = 0;
    // (the draws counted when the first screen is given to the grid: the next is its first draw)
    QObject::connect(w, &CrunchWindow::firstScreenShown, w, [&] {
        first = true;
        t.firstScreenMs = double(clock.nsecsElapsed()) / 1e6;
        paintsAtFirst = w->view()->paintTimes().size();
    });
    QObject::connect(w, &CrunchWindow::loadFinished, w, [&](bool good) {
        loaded = true;
        ok = good;
        t.loadMs = double(clock.nsecsElapsed()) / 1e6;
    });
    QObject::connect(w, &CrunchWindow::calculationFinished, w, [&] { done = true; t.calcMs = double(clock.nsecsElapsed()) / 1e6; });
    w->view()->clearPaintTimes();
    clock.start();
    if (!w->openFile(path)) return t;
    QEventLoop loop;
    while (clock.elapsed() < limitSeconds * 1000 && !(loaded && (done || !ok))) {
        spin(&loop, 1);
        if (first && t.firstPaintMs < 0 && w->view()->paintTimes().size() > paintsAtFirst) t.firstPaintMs = double(clock.nsecsElapsed()) / 1e6;
    }
    t.ok = loaded && ok && done;
    return t;
}

int gridWindow(const QString &path)
{
    CrunchWindow w;
    w.setNotifier([](const QString &, const QString &) {});
    w.resize(1280, 800);
    w.show();
    const WindowTimes t = timeWindow(&w, path);
    out() << path << "\tfirst screen " << ms(t.firstScreenMs) << " ms\tfirst screen drawn " << ms(t.firstPaintMs) << " ms\tread whole " << ms(t.loadMs) << " ms\tcalculated " << ms(t.calcMs) << " ms\n";
    return t.ok ? 0 : 1;
}

// ---- a workbook made here

QByteArray bigWorkbook(int rows, int cols)
{
    QByteArray sheet;
    sheet.reserve(qsizetype(rows) * cols * 28);
    sheet += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
             "<sheetViews><sheetView workbookViewId=\"0\"><pane ySplit=\"1\" topLeftCell=\"A2\" activePane=\"bottomLeft\" state=\"frozen\"/></sheetView></sheetViews>"
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
            else if (c == cols - 1) sheet += "<c r=\"" + ref + "\" s=\"2\"><f>SUM(B" + QByteArray::number(r) + ":C" + QByteArray::number(r) + ")</f><v>" + QByteArray::number(double((r * 3 + c) % 1000) / 4 + double((r * 3 + 1) % 1000) / 4) + "</v></c>";
            else sheet += "<c r=\"" + ref + "\" s=\"2\"><v>" + QByteArray::number(double((r * 3 + c) % 1000) / 4) + "</v></c>";
        }
        sheet += "</row>";
    }
    sheet += "</sheetData></worksheet>";
    const char *main = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
    const QByteArray styles = QByteArray("<?xml version=\"1.0\"?>\n<styleSheet xmlns=\"") + main +
                              "\"><numFmts count=\"1\"><numFmt numFmtId=\"164\" formatCode=\"#,##0.00\"/></numFmts><fonts count=\"2\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font>"
                              "<font><b/><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts><fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                              "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders><cellStyleXfs count=\"1\"><xf/></cellStyleXfs>"
                              "<cellXfs count=\"3\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/><xf numFmtId=\"0\" fontId=\"1\" fillId=\"0\" borderId=\"0\"/>"
                              "<xf numFmtId=\"164\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellXfs></styleSheet>";
    PackageWriter w;
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

void summarize(const char *what, const std::vector<double> &v)
{
    out() << "  " << QString::fromLatin1(what).leftJustified(34) << v.size() << " draws: middle " << ms(percentile(v, 0.5)) << " ms, 95th percentile " << ms(percentile(v, 0.95)) << " ms, longest "
          << ms(v.empty() ? 0 : *std::max_element(v.begin(), v.end())) << " ms\n";
}

int gridBench(int rows, int cols)
{
    // (JC_BENCH_FILE names a file to keep the workbook in, and to use again when it is there: making it takes longer than the measuring)
    QTemporaryDir dir;
    const QString keep = qEnvironmentVariable("JC_BENCH_FILE");
    const QString path = keep.isEmpty() ? dir.filePath(QStringLiteral("bench.xlsx")) : keep;
    if (keep.isEmpty() || !QFile::exists(keep)) {
        QElapsedTimer made;
        made.start();
        const QByteArray bytes = bigWorkbook(rows, cols);
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size()) {
            err() << "grid-bench: the workbook could not be written\n";
            return 1;
        }
        f.close();
        out() << "workbook of " << rows << " rows by " << cols << " columns: " << bytes.size() / 1048576 << " MB, made in " << made.elapsed() << " ms\n";
    } else {
        out() << "workbook of " << rows << " rows by " << cols << " columns, from " << path << "\n";
    }
    CrunchWindow w;
    w.setNotifier([](const QString &, const QString &) {});
    w.resize(1280, 800);
    w.show();
    const WindowTimes t = timeWindow(&w, path);
    out() << "  first screen " << ms(t.firstScreenMs) << " ms (from opening the file), drawn by " << ms(t.firstPaintMs) << " ms; read whole by " << ms(t.loadMs) << " ms; every formula worked out by "
          << ms(t.calcMs) << " ms\n";
    if (!t.ok) return 1;
    QEventLoop loop;
    spin(&loop, 50);
    grid::GridView *v = w.view();
    // scrolling by wheel steps of three rows: the strip each leaves is what is drawn
    v->clearPaintTimes();
    for (int i = 0; i < 400; ++i) {
        QWheelEvent wheel(QPointF(300, 300), QPointF(300, 300), QPoint(), QPoint(0, -120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
        QApplication::sendEvent(v->viewport(), &wheel);
        QCoreApplication::processEvents();
    }
    summarize("scrolling, 3 rows at a time", v->paintTimes());
    const double scrollP95 = percentile(v->paintTimes(), 0.95);
    // page jumps: a screen of new rows each
    v->clearPaintTimes();
    for (int i = 0; i < 100; ++i) {
        QKeyEvent press(QEvent::KeyPress, Qt::Key_PageDown, Qt::NoModifier);
        QApplication::sendEvent(v, &press);
        QCoreApplication::processEvents();
    }
    summarize("page down", v->paintTimes());
    // the whole window drawn (a resize, a zoom, a jump from the other end of the sheet)
    v->clearPaintTimes();
    for (int i = 0; i < 100; ++i) {
        v->verticalScrollBar()->setValue(v->verticalScrollBar()->value() + 40000 * (i % 2 ? -1 : 1));
        v->viewport()->repaint();
    }
    summarize("the whole window", v->paintTimes());
    out() << "scrolling 95th percentile " << ms(scrollP95) << " ms (the bar: 8 ms)\n";
    return 0;
}

// ---- the sweep

int gridSweep(const QString &keys, const QString &outDir, const QString &jobs)
{
#ifdef JCRUNCH_TOOLS_DIR
    const QString script = QStringLiteral(JCRUNCH_TOOLS_DIR "/crunch_grid.py");
#else
    const QString script = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../tools/crunch_grid.py"));
#endif
    QProcess python;
    python.setProcessChannelMode(QProcess::ForwardedChannels);
    QStringList args = {script, QCoreApplication::applicationFilePath(), keys, outDir};
    if (!jobs.isEmpty()) args << jobs;
    python.start(QStringLiteral("python3"), args);
    if (!python.waitForStarted()) {
        err() << "jcrunchtool: python3 could not be started, which grid-sweep needs\n";
        return 1;
    }
    python.waitForFinished(-1);
    return python.exitStatus() == QProcess::NormalExit ? python.exitCode() : 1;
}

} // namespace

namespace jp::crunchtool {

int gridMain(const QStringList &args)
{
    if (args.size() >= 2 && args[0] == QLatin1String("grid-one")) {
        const bool full = args[1] == QLatin1String("--full");
        if (args.size() == (full ? 3 : 2)) return gridOne(args.last(), full);
    }
    if (args.size() == 2 && args[0] == QLatin1String("grid-window")) return gridWindow(args[1]);
    if (args[0] == QLatin1String("grid-bench") && args.size() <= 3) return gridBench(args.value(1, QStringLiteral("100000")).toInt(), args.value(2, QStringLiteral("20")).toInt());
    if (args[0] == QLatin1String("grid-sweep") && (args.size() == 3 || args.size() == 4)) return gridSweep(args[1], args[2], args.value(3));
    return -1;
}

} // namespace jp::crunchtool
