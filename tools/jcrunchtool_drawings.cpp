// The charts, pictures, and shapes of a workbook, as JeffCrunch shows them (docs/jeffcrunch.md): what a workbook's drawings hold and how many of them can be drawn, and a sweep of a list.
//   jcrunchtool grid-drawings-one [--live] <file.xlsx>   reads each sheet's drawing and makes every object, draws each at its anchor's size (without a window), and prints one line of
//                                                       tab-separated fields: path, exit (0 opened, 1 not opened), anchors, charts, pictures, shapes, groups, connectors, objects that
//                                                       can be drawn, objects left out (a sentence for each, last), hidden, anchors that hold nothing read, drawn without a failure, drawn
//                                                       with one, milliseconds to draw them all (the sum, and the longest), the chart sheets, the first sheet (from 1) that has an object
//                                                       drawn, the references of the charts and how many name cells that are found (with --live: the whole workbook is read, so the
//                                                       charts read its cells; else they have the numbers they store), and the reasons
//   jcrunchtool grid-drawings <keys.txt> <out-dir> [jobs]   runs it on each workbook listed (each in a child process with 120 seconds and 4 GB), and writes <out-dir>/drawings.tsv
//                                                       (tools/crunch_drawings.py does the work)

#include "core/loader.h"
#include "core/sheetobjects.h"
#include "grid/axis.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QProcess>
#include <QTextStream>

#include <algorithm>

using namespace jp;
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

int drawingsOne(const QString &path, bool live)
{
    std::shared_ptr<Workbook> book;
    std::unique_ptr<crunch::LoadedBook> loaded;
    QString why;
    if (live) {
        loaded = std::make_unique<crunch::LoadedBook>();
        if (!crunch::loadFirstScreen(path, loaded.get(), [] { return false; }, &why) || !crunch::loadWhole(loaded.get(), [] { return false; }, nullptr)) {
            err() << path << ": " << (why.isEmpty() ? QStringLiteral("not read whole") : why) << "\n";
            return 1;
        }
        book = loaded->book;
    } else {
        book = std::make_shared<Workbook>();
        if (!book->openFile(path, &why)) {
            err() << path << ": " << why << "\n";
            return 1;
        }
    }
    crunch::SheetObjects objects;
    objects.open(book);
    if (live && loaded->calc) objects.setCells(loaded->calc.get(), nullptr);
    crunch::DrawingCounts total;
    int chartSheets = 0, first = 0, ok = 0, failed = 0, refs = 0, found = 0;
    double sumMs = 0, maxMs = 0;
    for (int sheet = 0; sheet < book->info().sheets.size(); ++sheet) {
        const crunch::DrawingCounts c = objects.counts(sheet);
        if (objects.isChartSheet(sheet)) ++chartSheets;
        total.anchors += c.anchors;
        total.charts += c.charts;
        total.pictures += c.pictures;
        total.shapes += c.shapes;
        total.groups += c.groups;
        total.connectors += c.connectors;
        total.shown += c.shown;
        total.leftOut += c.leftOut;
        total.hidden += c.hidden;
        total.skipped += c.skipped;
        total.why << c.why;
        if (c.shown == 0) continue;
        objects.show(sheet);
        grid::ObjectLayer *layer = objects.layer();
        const grid::Axis rows(1048576, 20), cols(16384, 64);
        int drawnHere = 0;
        for (int i = 0; i < layer->count(); ++i) {
            const grid::LayerObject &o = layer->object(i);
            const grid::ObjectBox box = grid::anchorBox(o.anchor, rows, cols);
            grid::RenderRequest q;
            q.zoom = 1;
            q.units = QSizeF(std::clamp(box.w, 4.0, 4000.0), std::clamp(box.h, 4.0, 4000.0));
            const double m = o.margin;
            q.pixels = QSize(int(q.units.width() + 2 * m + 0.5), int(q.units.height() + 2 * m + 0.5));
            q.inner = QRectF(m, m, q.units.width(), q.units.height());
            QElapsedTimer t;
            t.start();
            const QImage img = o.render ? o.render(q) : QImage();
            const double e = double(t.nsecsElapsed()) / 1e6;
            sumMs += e;
            maxMs = std::max(maxMs, e);
            if (img.isNull()) {
                ++failed;
                total.why << QStringLiteral("an object that drew nothing (%1)").arg(o.name);
            } else {
                ++ok;
                ++drawnHere;
            }
            if (o.kind == ObjectKind::Chart) {
                int r = 0, f = 0;
                objects.chartRefCounts(sheet, o.id, &r, &f);
                refs += r;
                found += f;
            }
        }
        if (drawnHere > 0 && first == 0) first = sheet + 1;
    }
    total.why.removeDuplicates();
    out() << path << "\t0\t" << total.anchors << "\t" << total.charts << "\t" << total.pictures << "\t" << total.shapes << "\t" << total.groups << "\t" << total.connectors << "\t" << total.shown << "\t"
          << total.leftOut << "\t" << total.hidden << "\t" << total.skipped << "\t" << ok << "\t" << failed << "\t" << QString::number(sumMs, 'f', 1) << "\t" << QString::number(maxMs, 'f', 1) << "\t"
          << chartSheets << "\t" << first << "\t" << refs << "\t" << found << "\t" << total.why.join(QStringLiteral("; ")).replace(QLatin1Char('\t'), QLatin1Char(' ')).replace(QLatin1Char('\n'), QLatin1Char(' ')) << "\n";
    out().flush();
    return 0;
}

int drawingsSweep(const QString &keys, const QString &outDir, const QString &jobs)
{
    const QString script = QStringLiteral(JCRUNCH_TOOLS_DIR) + QStringLiteral("/crunch_drawings.py");
    QProcess python;
    python.setProcessChannelMode(QProcess::ForwardedChannels);
    QStringList args = {script, QCoreApplication::applicationFilePath(), keys, outDir};
    if (!jobs.isEmpty()) args << jobs;
    python.start(QStringLiteral("python3"), args);
    if (!python.waitForStarted()) {
        err() << "jcrunchtool: python3 could not be started, which grid-drawings needs\n";
        return 1;
    }
    python.waitForFinished(-1);
    return python.exitStatus() == QProcess::NormalExit ? python.exitCode() : 1;
}

} // namespace

namespace jp::crunchtool {

int drawingsMain(const QStringList &args)
{
    if (args.size() >= 2 && args[0] == QLatin1String("grid-drawings-one")) {
        const bool live = args[1] == QLatin1String("--live");
        if (args.size() == (live ? 3 : 2)) return drawingsOne(args.last(), live);
    }
    if (args.size() >= 3 && args.size() <= 4 && args[0] == QLatin1String("grid-drawings")) return drawingsSweep(args[1], args[2], args.value(3));
    return -1;
}

} // namespace jp::crunchtool
