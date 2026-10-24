// The charts, pictures, and shapes of a workbook, as JeffCrunch shows them (docs/jeffcrunch.md): what a workbook's drawings hold and how many of them can be drawn, and a sweep of a list.
//   jcrunchtool grid-drawings-one [--live] <file.xlsx>   reads each sheet's drawing and makes every object, draws each at its anchor's size (without a window), and prints one line of
//                                                       tab-separated fields: path, exit (0 opened, 1 not opened), anchors, charts, pictures, shapes, groups, connectors, objects that
//                                                       can be drawn, objects left out (a sentence for each, last), hidden, anchors that hold nothing read, drawn without a failure, drawn
//                                                       with one, milliseconds to draw them all (the sum, and the longest), the chart sheets, the first sheet (from 1) that has an object
//                                                       drawn, the references of the charts and how many name cells that are found (with --live: the whole workbook is read, so the
//                                                       charts read its cells; else they have the numbers they store), and the reasons
//   jcrunchtool grid-drawings-bench [charts=400] [shapes=0]           a workbook made here (a sheet of numbers, that many charts down it, a picture and a shape beside each tenth) opened in the
//                                                       window (1,280 by 800, offscreen): the milliseconds to the first screen, to the objects shown, and the pictures made; then 400
//                                                       scrolls of three rows with charts on the screen, with the cells the charts read changed as it goes, and a whole-window draw: the
//                                                       milliseconds each paint took (the middle, the 95th percentile, the longest), and what was drawn
//   jcrunchtool grid-drawings-shot <file.xlsx> <out.png> [sheet=first with objects] [WxH=1280x800]   the sheet in the window, scrolled to show its first objects, as a picture
//   jcrunchtool grid-drawings <keys.txt> <out-dir> [jobs]   runs it on each workbook listed (each in a child process with 120 seconds and 4 GB), and writes <out-dir>/drawings.tsv
//                                                       (tools/crunch_drawings.py does the work)

#include "app/crunchwindow.h"
#include "core/loader.h"
#include "core/threadtime.h"
#include "grid/gridview.h"
#include "io/zip.h"
#include "core/sheetobjects.h"
#include "grid/axis.h"

#include <QApplication>
#include <QBuffer>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QImage>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTimer>
#include <QWheelEvent>
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
                QStringList missed;
                objects.chartRefCounts(sheet, o.id, &r, &f, &missed);
                if (!qEnvironmentVariableIsEmpty("JC_DRAWINGS_SHOW"))
                    for (const QString &m : std::as_const(missed)) err() << "unresolved: " << m << "\n";
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


// ---- the bench

// A workbook: a sheet of `rows` rows of three columns of numbers, and `charts` charts down it (each over ten rows, reading thirty cells of column B), with a picture and a shape beside each tenth.
QByteArray benchWorkbook(int rows, int charts, int manyShapes)
{
    jp::ZipWriter zip;
    const char *xmlHead = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    QString types = QStringLiteral("<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\"><Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
                                   "<Default Extension=\"png\" ContentType=\"image/png\"/><Default Extension=\"xml\" ContentType=\"application/xml\"/>"
                                   "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
                                   "<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
                                   "<Override PartName=\"/xl/drawings/drawing1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.drawing+xml\"/>");
    for (int k = 0; k < charts; ++k) types += QStringLiteral("<Override PartName=\"/xl/charts/chart%1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.drawingml.chart+xml\"/>").arg(k + 1);
    types += QStringLiteral("</Types>");
    zip.add(QStringLiteral("[Content_Types].xml"), (QByteArray(xmlHead) + types.toUtf8()), true);
    zip.add(QStringLiteral("_rels/.rels"), QByteArray(xmlHead) + "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"><Relationship Id=\"rId1\" Type=\"" + rel.toUtf8() + "officeDocument\" Target=\"xl/workbook.xml\"/></Relationships>", true);
    zip.add(QStringLiteral("xl/workbook.xml"), QByteArray(xmlHead) + "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"" + rel.chopped(1).toUtf8() + "\"><sheets><sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"rId1\"/></sheets></workbook>", true);
    zip.add(QStringLiteral("xl/_rels/workbook.xml.rels"), QByteArray(xmlHead) + "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"><Relationship Id=\"rId1\" Type=\"" + rel.toUtf8() + "worksheet\" Target=\"worksheets/sheet1.xml\"/></Relationships>", true);
    zip.add(QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels"), QByteArray(xmlHead) + "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"><Relationship Id=\"rId1\" Type=\"" + rel.toUtf8() + "drawing\" Target=\"../drawings/drawing1.xml\"/></Relationships>", true);
    QByteArray sheet = QByteArray(xmlHead) + "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"" + rel.chopped(1).toUtf8() + "\"><sheetData>";
    for (int r = 1; r <= rows; ++r)
        sheet += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><v>%2</v></c><c r=\"C%1\"><f>B%1*2</f><v>%3</v></c></row>").arg(r).arg((r * 37) % 101).arg(((r * 37) % 101) * 2).toUtf8();
    sheet += "</sheetData><drawing r:id=\"rId1\"/></worksheet>";
    zip.add(QStringLiteral("xl/worksheets/sheet1.xml"), sheet, true);
    QImage png(64, 32, QImage::Format_ARGB32);
    png.fill(QColor(0x13, 0x77, 0x3A));
    QByteArray pngBytes;
    QBuffer buffer(&pngBytes);
    buffer.open(QIODevice::WriteOnly);
    png.save(&buffer, "PNG");
    zip.add(QStringLiteral("xl/media/image1.png"), pngBytes, false);
    QByteArray drawing = QByteArray(xmlHead) + "<xdr:wsDr xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"" + rel.chopped(1).toUtf8() + "\" xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\">";
    QByteArray drawingRels = QByteArray(xmlHead) + "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"><Relationship Id=\"rIdP\" Type=\"" + rel.toUtf8() + "image\" Target=\"../media/image1.png\"/>";
    for (int k = 0; k < charts; ++k) {
        const int top = 2 + k * 12, bottom = top + 10;
        drawing += QStringLiteral("<xdr:twoCellAnchor><xdr:from><xdr:col>4</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>%1</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from><xdr:to><xdr:col>12</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>%2</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to>"
                                   "<xdr:graphicFrame macro=\"\"><xdr:nvGraphicFramePr><xdr:cNvPr id=\"%3\" name=\"Chart %4\"/><xdr:cNvGraphicFramePr/></xdr:nvGraphicFramePr><xdr:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/></xdr:xfrm>"
                                   "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"><c:chart r:id=\"rIdC%4\"/></a:graphicData></a:graphic></xdr:graphicFrame><xdr:clientData/></xdr:twoCellAnchor>")
                     .arg(top).arg(bottom).arg(2 + k).arg(k + 1).toUtf8();
        drawingRels += QStringLiteral("<Relationship Id=\"rIdC%1\" Type=\"%2chart\" Target=\"../charts/chart%1.xml\"/>").arg(k + 1).arg(rel).toUtf8();
        const int first = top + 1, last = std::min(rows, first + 29);
        QString pts, cats;
        for (int i = 0; i < 30; ++i) {
            pts += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(((first + i) * 37) % 101);
            cats += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(first + i);
        }
        const QString chart = QStringLiteral("<c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"><c:chart><c:autoTitleDeleted val=\"1\"/><c:plotArea><c:layout/>"
                                              "<c:%5><c:varyColors val=\"0\"/><c:ser><c:idx val=\"0\"/><c:order val=\"0\"/><c:tx><c:v>Series</c:v></c:tx>"
                                              "<c:cat><c:numRef><c:f>Sheet1!$A$%1:$A$%2</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"30\"/>%3</c:numCache></c:numRef></c:cat>"
                                              "<c:val><c:numRef><c:f>Sheet1!$B$%1:$B$%2</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"30\"/>%4</c:numCache></c:numRef></c:val></c:ser>"
                                              "<c:axId val=\"1\"/><c:axId val=\"2\"/></c:%5><c:catAx><c:axId val=\"1\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:crossAx val=\"2\"/></c:catAx>"
                                              "<c:valAx><c:axId val=\"2\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorGridlines/><c:crossAx val=\"1\"/></c:valAx></c:plotArea>"
                                              "<c:legend><c:legendPos val=\"r\"/></c:legend><c:plotVisOnly val=\"1\"/></c:chart></c:chartSpace>")
                                  .arg(first).arg(last).arg(cats, pts).arg(k % 2 ? QStringLiteral("lineChart") : QStringLiteral("barChart"));
        zip.add(QStringLiteral("xl/charts/chart%1.xml").arg(k + 1), QByteArray(xmlHead) + chart.toUtf8(), true);
        if (k % 10 == 0) {
            drawing += QStringLiteral("<xdr:twoCellAnchor><xdr:from><xdr:col>0</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>%1</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from><xdr:to><xdr:col>2</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>%2</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to>"
                                       "<xdr:pic><xdr:nvPicPr><xdr:cNvPr id=\"%3\" name=\"Picture %4\"/><xdr:cNvPicPr/></xdr:nvPicPr><xdr:blipFill><a:blip r:embed=\"rIdP\"/><a:stretch><a:fillRect/></a:stretch></xdr:blipFill>"
                                       "<xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1\" cy=\"1\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></xdr:spPr></xdr:pic><xdr:clientData/></xdr:twoCellAnchor>")
                         .arg(top).arg(top + 4).arg(5000 + k).arg(k + 1).toUtf8();
            drawing += QStringLiteral("<xdr:twoCellAnchor><xdr:from><xdr:col>0</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>%1</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from><xdr:to><xdr:col>2</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>%2</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to>"
                                       "<xdr:sp macro=\"\" textlink=\"\"><xdr:nvSpPr><xdr:cNvPr id=\"%3\" name=\"Shape %4\"/><xdr:cNvSpPr/></xdr:nvSpPr><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1\" cy=\"1\"/></a:xfrm><a:prstGeom prst=\"roundRect\"><a:avLst/></a:prstGeom>"
                                       "<a:solidFill><a:srgbClr val=\"3366CC\"/></a:solidFill></xdr:spPr><xdr:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:t>Note %4</a:t></a:r></a:p></xdr:txBody></xdr:sp><xdr:clientData/></xdr:twoCellAnchor>")
                         .arg(top + 5).arg(top + 9).arg(6000 + k).arg(k + 1).toUtf8();
        }
    }
    // (and a crowd of small shapes down the sheet, one to a row, for what a paint costs in objects it does not draw)
    for (int i = 0; i < manyShapes; ++i)
        drawing += QStringLiteral("<xdr:twoCellAnchor><xdr:from><xdr:col>14</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>%1</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from><xdr:to><xdr:col>15</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>%2</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to>"
                                   "<xdr:sp macro=\"\" textlink=\"\"><xdr:nvSpPr><xdr:cNvPr id=\"%3\" name=\"Dot %3\"/><xdr:cNvSpPr/></xdr:nvSpPr><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1\" cy=\"1\"/></a:xfrm><a:prstGeom prst=\"ellipse\"><a:avLst/></a:prstGeom>"
                                   "<a:solidFill><a:srgbClr val=\"CC3333\"/></a:solidFill></xdr:spPr></xdr:sp><xdr:clientData/></xdr:twoCellAnchor>")
                     .arg(i % rows).arg(i % rows + 1).arg(10000 + i).toUtf8();
    drawing += "</xdr:wsDr>";
    drawingRels += "</Relationships>";
    zip.add(QStringLiteral("xl/drawings/drawing1.xml"), drawing, true);
    zip.add(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"), drawingRels, true);
    return zip.finish();
}

double percentileOf(std::vector<double> v, double p)
{
    if (v.empty()) return 0;
    std::sort(v.begin(), v.end());
    return v[std::min(v.size() - 1, size_t(p * double(v.size())))];
}

void report(const char *what, const std::vector<double> &wall, const std::vector<double> &cpu)
{
    out() << "  " << QString::fromLatin1(what).leftJustified(46) << wall.size() << " paints: middle " << QString::number(percentileOf(wall, 0.5), 'f', 2) << " ms, 95th percentile "
          << QString::number(percentileOf(wall, 0.95), 'f', 2) << " ms, longest " << QString::number(wall.empty() ? 0 : *std::max_element(wall.begin(), wall.end()), 'f', 2)
          << " ms; by the thread's own time: middle " << QString::number(percentileOf(cpu, 0.5), 'f', 2) << " ms, 95th percentile " << QString::number(percentileOf(cpu, 0.95), 'f', 2) << " ms, longest "
          << QString::number(cpu.empty() ? 0 : *std::max_element(cpu.begin(), cpu.end()), 'f', 2) << " ms\n";
}

void spinFor(int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, &QEventLoop::quit);
    loop.exec();
}

int drawingsBench(int charts, int shapes)
{
    const int rows = std::max(2000, charts * 12 + 40);
    QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("drawings-bench.xlsx"));
    QElapsedTimer made;
    made.start();
    const QByteArray bytes = benchWorkbook(rows, charts, shapes);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size()) {
        err() << "grid-drawings-bench: the workbook could not be written\n";
        return 1;
    }
    f.close();
    out() << "workbook of " << rows << " rows with " << charts << " charts, " << (charts + 9) / 10 << " pictures and " << (charts + 9) / 10 << " shapes" << (shapes > 0 ? QStringLiteral(" and %1 small shapes").arg(shapes) : QString()) << ": " << bytes.size() / 1024 << " KB, made in " << made.elapsed() << " ms\n";
    CrunchWindow w;
    w.setNotifier([](const QString &, const QString &) {});
    w.resize(1280, 800);
    w.show();
    QElapsedTimer clock;
    clock.start();
    bool done = false;
    QEventLoop loop;
    QObject::connect(&w, &CrunchWindow::calculationFinished, &loop, [&] {
        done = true;
        loop.quit();
    });
    QTimer::singleShot(120000, &loop, &QEventLoop::quit);
    if (!w.openFile(path)) return 1;
    loop.exec();
    if (!done) {
        err() << "grid-drawings-bench: the workbook was not read in 120 seconds\n";
        return 1;
    }
    const double loadMs = double(clock.nsecsElapsed()) / 1e6;
    crunch::SheetObjects *objects = w.objects();
    while (objects->shownSheet() != 0) spinFor(5);
    const double shownMs = double(clock.nsecsElapsed()) / 1e6;
    grid::ObjectLayer *layer = objects->layer();
    layer->prepareAll();
    const double madeMs = double(clock.nsecsElapsed()) / 1e6;
    out() << "  the workbook is read and calculated by " << QString::number(loadMs, 'f', 0) << " ms, its " << layer->count() << " objects are shown by " << QString::number(shownMs, 'f', 0) << " ms, the ones on the screen are made by "
          << QString::number(madeMs, 'f', 0) << " ms\n";
    spinFor(100);
    grid::GridView *v = w.view();
    // scrolling by wheel steps of three rows, with an idle moment after each (a person's wheel gives the program some between its events)
    const qint64 renders0 = layer->stats().renders, placeholders0 = layer->stats().placeholders;
    v->clearPaintTimes();
    for (int i = 0; i < 400; ++i) {
        QWheelEvent wheel(QPointF(300, 300), QPointF(300, 300), QPoint(), QPoint(0, -120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
        QApplication::sendEvent(v->viewport(), &wheel);
        QCoreApplication::processEvents();
        if (i % 4 == 3) spinFor(35);   // (the object layer's own time: the pictures near the screen are made)
    }
    report("scrolling, 3 rows at a time", v->paintTimes(), v->paintCpuTimes());
    const double scrollP95 = percentileOf(v->paintTimes(), 0.95);
    out() << "    pictures made during the scroll: " << layer->stats().renders - renders0 << ", objects drawn as an outline for want of a picture: " << layer->stats().placeholders - placeholders0 << "\n";
    // the same, with no idle moment, the whole window drawn each time: every picture a scroll uncovers is made in its paint (at most six in each part of the grid)
    v->verticalScrollBar()->setValue(0);
    spinFor(200);
    layer->invalidateAll();
    layer->prepareAll();
    const qint64 renders1 = layer->stats().renders;
    v->clearPaintTimes();
    for (int i = 0; i < 400; ++i) {
        QWheelEvent wheel(QPointF(300, 300), QPointF(300, 300), QPoint(), QPoint(0, -120 * 4), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
        QApplication::sendEvent(v->viewport(), &wheel);
        v->viewport()->repaint();
    }
    report("scrolling, no idle moment, whole window", v->paintTimes(), v->paintCpuTimes());
    out() << "    pictures made: " << layer->stats().renders - renders1 << "\n";
    spinFor(200);
    // the cells the charts read change as it scrolls (a calculation in progress): the old pictures are drawn, and new ones are made a moment later
    v->verticalScrollBar()->setValue(0);
    spinFor(100);
    v->clearPaintTimes();
    for (int i = 0; i < 400; ++i) {
        QWheelEvent wheel(QPointF(300, 300), QPointF(300, 300), QPoint(), QPoint(0, -120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
        QApplication::sendEvent(v->viewport(), &wheel);
        QCoreApplication::processEvents();
        if (i % 10 == 0) layer->invalidateAll();
        if (i % 4 == 3) spinFor(35);
    }
    report("scrolling while the charts' cells change", v->paintTimes(), v->paintCpuTimes());
    // the whole window drawn (a resize, a zoom, a jump)
    v->clearPaintTimes();
    for (int i = 0; i < 100; ++i) {
        v->verticalScrollBar()->setValue(v->verticalScrollBar()->value() + 4000 * (i % 2 ? -1 : 1));
        v->viewport()->repaint();
    }
    report("the whole window", v->paintTimes(), v->paintCpuTimes());
    out() << "scrolling with charts on the screen: 95th percentile " << QString::number(scrollP95, 'f', 2) << " ms (the bar: 8 ms)\n";
    return 0;
}

// A picture of the window with the sheet's objects in view: the sheet is the one asked for (from 1), or the first that has an object that can be drawn.
int drawingsShot(const QString &path, const QString &png, int sheetAsked, const QSize &size)
{
    CrunchWindow w;
    w.setNotifier([](const QString &, const QString &) {});
    w.resize(size);
    w.show();
    bool done = false;
    QEventLoop loop;
    QObject::connect(&w, &CrunchWindow::calculationFinished, &loop, [&] {
        done = true;
        loop.quit();
    });
    QTimer::singleShot(120000, &loop, &QEventLoop::quit);
    if (!w.openFile(path)) return 1;
    loop.exec();
    if (!done) {
        err() << path << ": not read in 120 seconds\n";
        return 1;
    }
    crunch::SheetObjects *objects = w.objects();
    int sheet = sheetAsked - 1;
    if (sheet < 0)
        for (int i = 0; i < w.loadedBook()->book->info().sheets.size() && sheet < 0; ++i)
            if (objects->counts(i).shown > 0) sheet = i;
    if (sheet < 0) {
        err() << path << ": no object that can be drawn\n";
        return 3;
    }
    w.setActiveSheet(sheet);
    while (objects->shownSheet() != sheet) spinFor(5);
    grid::ObjectLayer *layer = objects->layer();
    grid::GridView *v = w.view();
    // scrolled so that the first object that is drawn is whole in the window if it fits: the end of it first, then its start
    for (int i = 0; i < layer->count(); ++i) {
        const jp::office::sheet::DrawingAnchor &a = layer->object(i).anchor;
        if (a.kind != jp::office::sheet::AnchorKind::TwoCell) continue;
        v->ensureVisible(a.to.row, a.to.col);
        v->ensureVisible(a.from.row, a.from.col);
        break;
    }
    layer->prepareAll();
    spinFor(100);
    layer->prepareAll();
    const QPixmap shot = w.grab();
    if (!shot.save(png)) {
        err() << path << ": the picture could not be written\n";
        return 1;
    }
    out() << path << "\tsheet " << sheet + 1 << "\t" << layer->count() << " objects\n";
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
    if (args.size() >= 3 && args.size() <= 5 && args[0] == QLatin1String("grid-drawings-shot")) {
        const QStringList wh = args.value(4, QStringLiteral("1280x800")).split(QLatin1Char('x'));
        return drawingsShot(args[1], args[2], args.value(3, QStringLiteral("0")).toInt(), QSize(wh.value(0).toInt() > 0 ? wh.value(0).toInt() : 1280, wh.value(1).toInt() > 0 ? wh.value(1).toInt() : 800));
    }
    if (args.size() >= 1 && args.size() <= 3 && args[0] == QLatin1String("grid-drawings-bench")) return drawingsBench(args.value(1, QStringLiteral("400")).toInt(), args.value(2, QStringLiteral("0")).toInt());
    if (args.size() >= 3 && args.size() <= 4 && args[0] == QLatin1String("grid-drawings")) return drawingsSweep(args[1], args[2], args.value(3));
    return -1;
}

} // namespace jp::crunchtool
