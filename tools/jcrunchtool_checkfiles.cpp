// The check files of jcrunchtool (docs/jeffcrunch.md): workbooks JeffCrunch saves, made to be opened by the common spreadsheet program on another machine, to see that none of them asks to be repaired.
//   jcrunchtool make-check-files <out-dir>   (JC_CHECK_ONLY=part of a name makes only those files) builds a small workbook of its own, and from it a file for each feature the program writes (tables, charts of every kind, objects moved and pasted, notes,
//                                            links, protection, names, filters and sorts, page setup with header pictures, views, sheets inserted, copied, moved, renamed, and deleted, merged cells, formats
//                                            and styles, how it calculates, long text, many sheets), each made through the edit session and the window's own commands and saved; one line for each file:
//                                            name, whether it was saved, and what went wrong if not. The seed itself is saved as seed.xlsx (a file nothing was done to).
//   jcrunchtool check-files-practice <list.txt> <out-dir> [count=200] [step=40]   opens every step-th workbook of the list, types a number in a cell beyond its data, saves it as <out-dir>/NNNN.xlsx, and
//                                            writes <out-dir>/index.tsv (number, the workbook's path, whether it was saved)

#include "app/crunchwindow.h"
#include "core/editsession.h"
#include "core/sheetobjects.h"
#include "grid/gridview.h"
#include "io/zip.h"
#include "sheet/cellref.h"
#include "sheet/hfpictures.h"
#include "sheet/protection.h"

#include <QApplication>
#include <QBuffer>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QTemporaryDir>
#include <QTextStream>
#include <QThread>

#include <functional>

using namespace jp;
using namespace jp::office;
using namespace jp::office::sheet;

namespace {

using jp::office::sheet::CellRange;
using jp::office::sheet::PageSetup;

QTextStream &out()
{
    static QTextStream s(stdout);
    return s;
}

bool waitFor(const std::function<bool()> &done, int seconds)
{
    QElapsedTimer t;
    t.start();
    while (!done()) {
        if (t.elapsed() > seconds * 1000) return false;
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        QThread::msleep(2);
    }
    return true;
}

bool openWindow(CrunchWindow &w, const QString &path)
{
    bool loaded = false, ok = true;
    QObject::connect(&w, &CrunchWindow::loadFinished, &w, [&](bool good) {
        loaded = true;
        ok = good;
    });
    w.setNotifier([](const QString &, const QString &) {});
    w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
    if (!w.openFile(path)) return false;
    if (!waitFor([&] { return loaded; }, 120)) return false;
    waitFor([&] { return !w.isCalculating(); }, 120);
    QCoreApplication::processEvents();
    return ok;
}

// Waits until the worker has worked out what the edits made.
bool settle(CrunchWindow &w)
{
    if (!waitFor([&] { return !w.isCalculating(); }, 60)) return false;
    auto &loader = w.workbookLoader();
    if (!loader.result()) return true;
    QElapsedTimer waited;
    waited.start();
    for (;;) {
        QThread::msleep(1);
        QCoreApplication::processEvents();
        bool done = false;
        if (loader.result()->calcIdle.load()) {
            auto hold = loader.hold();
            done = !hold || !hold->calculation()->isDirty();
        }
        if (done) break;
        if (waited.elapsed() > 8000) return false;   // (a workbook that calculates when it is told stays dirty)
    }
    QCoreApplication::processEvents();
    return true;
}

bool saveWindow(CrunchWindow &w, const QString &path)
{
    settle(w);
    if (w.session()) w.session()->finishFills();
    bool done = false, ok = false;
    QObject::connect(&w, &CrunchWindow::saved, &w, [&](const QString &, bool good) {
        done = true;
        ok = good;
    });
    if (!w.saveAs(path)) return false;
    return waitFor([&] { return done; }, 120) && ok;
}

// ---- the seed workbook

QByteArray png(int w, int h, const QColor &color)
{
    QImage img(w, h, QImage::Format_ARGB32);
    img.fill(color);
    QPainter p(&img);
    p.setPen(Qt::white);
    p.drawLine(0, 0, w - 1, h - 1);
    p.end();
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG");
    return bytes;
}

const char *kDecl = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
const char *kMain = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
const char *kRel = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

QString text(const QString &ref, const QString &t, int style = 0)
{
    return QStringLiteral("<c r=\"%1\" s=\"%3\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, t.toHtmlEscaped()).arg(style);
}
QString num(const QString &ref, double v, int style = 0)
{
    return QStringLiteral("<c r=\"%1\" s=\"%3\"><v>%2</v></c>").arg(ref).arg(v, 0, 'g', 15).arg(style);
}

// A workbook of four sheets: Data (headers and eleven rows of numbers, a rule of conditional formatting, validations (a list from another sheet), an array formula, a picture, a shape, and a chart), Lists,
// Extra, and Empty. The workbook has a defined name.
QByteArray seedWorkbook()
{
    QString rows = QStringLiteral("<row r=\"1\">") + text("A1", "Region", 1) + text("B1", "Q1", 1) + text("C1", "Q2", 1) + text("D1", "Q3", 1) + text("E1", "Q4", 1) + QStringLiteral("</row>");
    const char *regions[] = {"North", "South", "East", "West", "Central", "Coast", "Hills", "Lakes", "Plains", "Desert", "Islands"};
    for (int i = 0; i < 11; ++i) {
        const int r = i + 2;
        QString cells = text(QStringLiteral("A%1").arg(r), QString::fromLatin1(regions[i]));
        for (int j = 0; j < 4; ++j) cells += num(QStringLiteral("%1%2").arg(QChar('B' + j)).arg(r), 10 + (i * 7 + j * 13) % 90 + 0.5 * j, j == 3 ? 2 : 0);
        rows += QStringLiteral("<row r=\"%1\">%2</row>").arg(r).arg(cells);
    }
    rows += QStringLiteral("<row r=\"13\">") + text("A13", "Total") + QStringLiteral("<c r=\"B13\"><f>SUM(B2:B12)</f><v>0</v></c><c r=\"G13\"><f t=\"array\" ref=\"G13\">SUM(B2:B12*2)</f><v>0</v></c></row>");
    const QString after = QStringLiteral(
        "<mergeCells count=\"1\"><mergeCell ref=\"G1:H1\"/></mergeCells>"
        "<conditionalFormatting sqref=\"B2:E12\"><cfRule type=\"cellIs\" dxfId=\"0\" priority=\"1\" operator=\"greaterThan\"><formula>50</formula></cfRule></conditionalFormatting>"
        "<dataValidations count=\"2\"><dataValidation type=\"list\" allowBlank=\"1\" showInputMessage=\"1\" showErrorMessage=\"1\" sqref=\"G2:G10\"><formula1>Lists!$A$1:$A$5</formula1></dataValidation>"
        "<dataValidation type=\"whole\" operator=\"between\" allowBlank=\"1\" showErrorMessage=\"1\" sqref=\"H2:H5\"><formula1>1</formula1><formula2>9</formula2></dataValidation></dataValidations>"
        "<pageMargins left=\"0.7\" right=\"0.7\" top=\"0.75\" bottom=\"0.75\" header=\"0.3\" footer=\"0.3\"/>"
        "<drawing r:id=\"rId1\"/>");
    auto sheetXml = [&](const QString &data, const QString &extra) {
        return QString::fromLatin1(kDecl) + QStringLiteral("<worksheet xmlns=\"%1\" xmlns:r=\"%2\"><sheetData>%3</sheetData>%4</worksheet>").arg(QString::fromLatin1(kMain), QString::fromLatin1(kRel), data, extra);
    };
    QString lists;
    const char *items[] = {"Red", "Green", "Blue", "Amber", "Violet"};
    for (int i = 0; i < 5; ++i) lists += QStringLiteral("<row r=\"%1\">%2</row>").arg(i + 1).arg(text(QStringLiteral("A%1").arg(i + 1), QString::fromLatin1(items[i])));
    QString extra;
    for (int i = 0; i < 6; ++i) extra += QStringLiteral("<row r=\"%1\">%2%3</row>").arg(i + 1).arg(num(QStringLiteral("A%1").arg(i + 1), i + 1), num(QStringLiteral("B%1").arg(i + 1), (i + 1) * 2.5));

    const QString drawing = QString::fromLatin1(kDecl)
        + QStringLiteral("<xdr:wsDr xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"%1\">"
                         "<xdr:twoCellAnchor editAs=\"oneCell\"><xdr:from><xdr:col>0</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>14</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from>"
                         "<xdr:to><xdr:col>2</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>18</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to>"
                         "<xdr:pic><xdr:nvPicPr><xdr:cNvPr id=\"2\" name=\"Picture 1\" descr=\"a picture\"/><xdr:cNvPicPr/></xdr:nvPicPr><xdr:blipFill><a:blip r:embed=\"rId2\"/><a:stretch><a:fillRect/></a:stretch></xdr:blipFill>"
                         "<xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1000000\" cy=\"700000\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></xdr:spPr></xdr:pic><xdr:clientData/></xdr:twoCellAnchor>"
                         "<xdr:twoCellAnchor><xdr:from><xdr:col>3</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>14</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from>"
                         "<xdr:to><xdr:col>5</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>18</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to>"
                         "<xdr:sp macro=\"\" textlink=\"\"><xdr:nvSpPr><xdr:cNvPr id=\"3\" name=\"Shape 1\"/><xdr:cNvSpPr/></xdr:nvSpPr><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1000000\" cy=\"700000\"/></a:xfrm>"
                         "<a:prstGeom prst=\"roundRect\"><a:avLst/></a:prstGeom><a:solidFill><a:srgbClr val=\"3366CC\"/></a:solidFill></xdr:spPr>"
                         "<xdr:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang=\"en-US\"/><a:t>Shape</a:t></a:r></a:p></xdr:txBody></xdr:sp><xdr:clientData/></xdr:twoCellAnchor>"
                         "<xdr:twoCellAnchor><xdr:from><xdr:col>6</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>14</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from>"
                         "<xdr:to><xdr:col>12</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>30</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to>"
                         "<xdr:graphicFrame macro=\"\"><xdr:nvGraphicFramePr><xdr:cNvPr id=\"4\" name=\"Chart 1\"/><xdr:cNvGraphicFramePr/></xdr:nvGraphicFramePr><xdr:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/></xdr:xfrm>"
                         "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"><c:chart xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" r:id=\"rId3\"/></a:graphicData></a:graphic></xdr:graphicFrame>"
                         "<xdr:clientData/></xdr:twoCellAnchor></xdr:wsDr>").arg(QString::fromLatin1(kRel));
    QString cache;
    for (int i = 0; i < 11; ++i) cache += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(10 + (i * 7) % 90);
    const QString chart = QString::fromLatin1(kDecl)
        + QStringLiteral("<c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"%1\"><c:chart><c:autoTitleDeleted val=\"1\"/><c:plotArea><c:layout/>"
                         "<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/><c:varyColors val=\"0\"/><c:ser><c:idx val=\"0\"/><c:order val=\"0\"/>"
                         "<c:tx><c:strRef><c:f>Data!$B$1</c:f><c:strCache><c:ptCount val=\"1\"/><c:pt idx=\"0\"><c:v>Q1</c:v></c:pt></c:strCache></c:strRef></c:tx>"
                         "<c:cat><c:strRef><c:f>Data!$A$2:$A$12</c:f><c:strCache><c:ptCount val=\"11\"/>")
              .arg(QString::fromLatin1(kRel));
    QString cats;
    for (int i = 0; i < 11; ++i) cats += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(QString::fromLatin1(regions[i]));
    const QString chartTail = QStringLiteral("</c:strCache></c:strRef></c:cat><c:val><c:numRef><c:f>Data!$B$2:$B$12</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"11\"/>%1</c:numCache></c:numRef></c:val></c:ser>"
                                             "<c:gapWidth val=\"150\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>"
                                             "<c:catAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:crossAx val=\"222\"/></c:catAx>"
                                             "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:crossAx val=\"111\"/></c:valAx></c:plotArea>"
                                             "<c:plotVisOnly val=\"1\"/></c:chart></c:chartSpace>")
                                  .arg(cache);
    const QString styles = QString::fromLatin1(kDecl)
        + QStringLiteral("<styleSheet xmlns=\"%1\"><numFmts count=\"1\"><numFmt numFmtId=\"164\" formatCode=\"0.0\"/></numFmts>"
                         "<fonts count=\"2\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/></font><font><b/><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/></font></fonts>"
                         "<fills count=\"3\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill><fill><patternFill patternType=\"solid\"><fgColor rgb=\"FFDDEBF7\"/><bgColor indexed=\"64\"/></patternFill></fill></fills>"
                         "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                         "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
                         "<cellXfs count=\"3\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/><xf numFmtId=\"0\" fontId=\"1\" fillId=\"2\" borderId=\"0\" xfId=\"0\" applyFont=\"1\" applyFill=\"1\"/>"
                         "<xf numFmtId=\"164\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyNumberFormat=\"1\"/></cellXfs>"
                         "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>"
                         "<dxfs count=\"1\"><dxf><font><b/><color rgb=\"FF9C0006\"/></font><fill><patternFill><bgColor rgb=\"FFFFC7CE\"/></patternFill></fill></dxf></dxfs></styleSheet>")
              .arg(QString::fromLatin1(kMain));
    const QString workbook = QString::fromLatin1(kDecl)
        + QStringLiteral("<workbook xmlns=\"%1\" xmlns:r=\"%2\"><sheets><sheet name=\"Data\" sheetId=\"1\" r:id=\"rId1\"/><sheet name=\"Lists\" sheetId=\"2\" r:id=\"rId2\"/><sheet name=\"Extra\" sheetId=\"3\" r:id=\"rId3\"/><sheet name=\"Empty\" sheetId=\"4\" r:id=\"rId4\"/></sheets>"
                         "<definedNames><definedName name=\"Totals\">Data!$B$2:$B$12</definedName></definedNames><calcPr calcId=\"191029\"/></workbook>")
              .arg(QString::fromLatin1(kMain), QString::fromLatin1(kRel));
    auto rel = [](const QString &id, const QString &type, const QString &target) {
        return QStringLiteral("<Relationship Id=\"%1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/%2\" Target=\"%3\"/>").arg(id, type, target);
    };
    const QString pkg = QStringLiteral("<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">");
    ZipWriter z;
    z.add(QStringLiteral("[Content_Types].xml"),
          (QString::fromLatin1(kDecl)
           + QStringLiteral("<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\"><Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/><Default Extension=\"xml\" ContentType=\"application/xml\"/>"
                            "<Default Extension=\"png\" ContentType=\"image/png\"/>"
                            "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
                            "<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
                            "<Override PartName=\"/xl/worksheets/sheet2.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
                            "<Override PartName=\"/xl/worksheets/sheet3.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
                            "<Override PartName=\"/xl/worksheets/sheet4.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
                            "<Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>"
                            "<Override PartName=\"/xl/drawings/drawing1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.drawing+xml\"/>"
                            "<Override PartName=\"/xl/charts/chart1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.drawingml.chart+xml\"/></Types>"))
              .toUtf8());
    z.add(QStringLiteral("_rels/.rels"), (QString::fromLatin1(kDecl) + pkg + rel("rId1", "officeDocument", "xl/workbook.xml") + QStringLiteral("</Relationships>")).toUtf8());
    z.add(QStringLiteral("xl/workbook.xml"), workbook.toUtf8());
    z.add(QStringLiteral("xl/_rels/workbook.xml.rels"),
          (QString::fromLatin1(kDecl) + pkg + rel("rId1", "worksheet", "worksheets/sheet1.xml") + rel("rId2", "worksheet", "worksheets/sheet2.xml") + rel("rId3", "worksheet", "worksheets/sheet3.xml")
           + rel("rId4", "worksheet", "worksheets/sheet4.xml") + rel("rId5", "styles", "styles.xml") + QStringLiteral("</Relationships>"))
              .toUtf8());
    z.add(QStringLiteral("xl/styles.xml"), styles.toUtf8());
    z.add(QStringLiteral("xl/worksheets/sheet1.xml"), sheetXml(rows, after).toUtf8());
    z.add(QStringLiteral("xl/worksheets/sheet2.xml"), sheetXml(lists, QString()).toUtf8());
    z.add(QStringLiteral("xl/worksheets/sheet3.xml"), sheetXml(extra, QString()).toUtf8());
    z.add(QStringLiteral("xl/worksheets/sheet4.xml"), sheetXml(QString(), QString()).toUtf8());
    z.add(QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels"), (QString::fromLatin1(kDecl) + pkg + rel("rId1", "drawing", "../drawings/drawing1.xml") + QStringLiteral("</Relationships>")).toUtf8());
    z.add(QStringLiteral("xl/drawings/drawing1.xml"), drawing.toUtf8());
    z.add(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"),
          (QString::fromLatin1(kDecl) + pkg + rel("rId2", "image", "../media/image1.png") + rel("rId3", "chart", "../charts/chart1.xml") + QStringLiteral("</Relationships>")).toUtf8());
    z.add(QStringLiteral("xl/media/image1.png"), png(80, 56, QColor(0xC0, 0x40, 0x40)));
    z.add(QStringLiteral("xl/charts/chart1.xml"), (chart + cats + chartTail).toUtf8());
    return z.finish();
}

// ---- the scenarios

struct Ctx {
    CrunchWindow *w;
    crunch::EditSession *s;
    int failed = 0;
    QStringList notes;
    void check(const crunch::EditSession::Outcome &o, const char *what)
    {
        if (!o.ok) {
            ++failed;
            notes << QStringLiteral("%1: %2").arg(QLatin1String(what), o.problem.left(80));
        }
        settle(*w);
    }
    void need(bool ok, const char *what)
    {
        if (!ok) {
            ++failed;
            notes << QStringLiteral("%1 failed").arg(QLatin1String(what));
        }
        settle(*w);
    }
};

CellRange rng(const char *text)
{
    CellRange r;
    parseRangeRef(QString::fromLatin1(text), &r);
    return r;
}

using Session = crunch::EditSession;

void select(Ctx &c, const char *range)
{
    const CellRange r = rng(range);
    c.w->view()->selectRange(r.row1, r.col1, r.row2, r.col2);
}

int firstObject(Ctx &c, ObjectKind kind)
{
    for (const int id : c.w->objects()->idsOf(0))
        if (c.w->objects()->kindOf(0, id) == kind) return id;
    return -1;
}

DrawingAnchor cellAnchor(int col1, int row1, int col2, int row2)
{
    DrawingAnchor a;
    a.kind = AnchorKind::TwoCell;
    a.editAs = AnchorKind::TwoCell;
    a.from.col = quint32(col1);
    a.from.row = quint32(row1);
    a.to.col = quint32(col2);
    a.to.row = quint32(row2);
    return a;
}

const ChartKind kAllKinds[] = {ChartKind::Column, ChartKind::ColumnStacked, ChartKind::ColumnPercent, ChartKind::Bar, ChartKind::BarStacked, ChartKind::BarPercent, ChartKind::Line, ChartKind::LineMarkers,
                               ChartKind::LineStacked, ChartKind::Pie, ChartKind::Doughnut, ChartKind::Area, ChartKind::AreaStacked, ChartKind::Scatter, ChartKind::ScatterLines, ChartKind::Combo};

void charts(Ctx &c, int from, int to)
{
    c.w->setActiveSheet(0);
    waitFor([&] { return c.w->objects() && c.w->objects()->shownSheet() == 0; }, 20);
    for (int i = from; i < to; ++i) {
        select(c, i % 2 ? "A1:D12" : "A1:C12");
        c.need(c.w->insertChart(kAllKinds[i]), "insert chart");
    }
}

PageSetup pageOf(Ctx &c, int sheet) { return c.s->pages().sheet(sheet); }

HeaderPicture headerPicture(const QString &place, int n)
{
    HeaderPicture p;
    p.place = place;
    p.image = png(20 + n, 10 + n, QColor::fromHsv((n * 37) % 360, 200, 200));
    p.extension = QStringLiteral("png");
    p.width = 30 + n;
    p.height = 15 + n;
    p.title = QStringLiteral("pic%1").arg(n);
    return p;
}

struct Scenario {
    QString name;
    std::function<void(Ctx &)> run;
};

std::vector<Scenario> scenarios()
{
    std::vector<Scenario> v;
    v.push_back({QStringLiteral("seed-untouched"), [](Ctx &c) { c.check(c.s->type(0, 20, 0, QStringLiteral("x")), "type"); }});
    // ---- tables
    v.push_back({QStringLiteral("table-basic"), [](Ctx &c) {
                     int made = -1;
                     c.check(c.s->makeTable(2, rng("A1:E6"), true, QStringLiteral("TableStyleMedium2"), &made), "table");
                     c.check(c.s->setTableTotalsRow(made, true), "totals row");
                     c.check(c.s->setTableTotal(made, 1, QStringLiteral("sum")), "total sum");
                     c.check(c.s->setTableTotal(made, 2, QStringLiteral("average")), "total average");
                     c.check(c.s->setTableTotal(made, 3, QStringLiteral("custom"), QStringLiteral("SUBTOTAL(109,[Column4])*2")), "total custom");
                     c.check(c.s->setTableTotal(made, 4, QStringLiteral("countNums")), "total count");
                     c.check(c.s->type(2, 1, 2, QStringLiteral("10")), "a number in the column");
                 }});
    v.push_back({QStringLiteral("table-marks"), [](Ctx &c) {
                     int made = -1;
                     c.check(c.s->makeTable(0, rng("A1:E12"), true, QStringLiteral("TableStyleLight9"), &made), "table");
                     Session::TableMarks m;
                     m.style = QStringLiteral("TableStyleDark3");
                     m.firstColumn = true;
                     m.lastColumn = true;
                     m.rowStripes = false;
                     m.columnStripes = true;
                     m.filterButtons = false;
                     c.check(c.s->setTableMarks(made, m), "marks");
                     c.check(c.s->renameTable(made, QStringLiteral("Sales_Table")), "rename");
                 }});
    v.push_back({QStringLiteral("table-noheader"), [](Ctx &c) {
                     int plain = -1, made = -1;
                     c.check(c.s->makeTable(2, rng("A1:B6"), false, QStringLiteral("TableStyleMedium9"), &plain), "table without headers");
                     c.check(c.s->makeTable(0, rng("A1:E12"), true, QStringLiteral("TableStyleMedium4"), &made), "table");
                     c.check(c.s->setTableHeaderRow(made, false), "header off");
                     c.check(c.s->setTableHeaderRow(made, true), "header on");
                 }});
    v.push_back({QStringLiteral("table-bad-column"), [](Ctx &c) {
                     int made = -1;
                     c.check(c.s->makeTable(2, rng("A1:E6"), true, QStringLiteral("TableStyleMedium2"), &made), "table");
                     c.check(c.s->type(2, 8, 0, QStringLiteral("=SUM(Table1[Nope])")), "a column the table has not");
                 }});
    v.push_back({QStringLiteral("table-bad-table"), [](Ctx &c) {
                     int made = -1;
                     c.check(c.s->makeTable(2, rng("A1:E6"), true, QStringLiteral("TableStyleMedium2"), &made), "table");
                     c.check(c.s->type(2, 9, 0, QStringLiteral("=SUM(Nope[Column3])")), "a table the workbook has not");
                 }});
    v.push_back({QStringLiteral("table-bad-bare"), [](Ctx &c) {
                     int made = -1;
                     c.check(c.s->makeTable(2, rng("A1:E6"), true, QStringLiteral("TableStyleMedium2"), &made), "table");
                     c.check(c.s->type(2, 10, 0, QStringLiteral("=[Nope]")), "a bare column the table has not");
                 }});
    v.push_back({QStringLiteral("table-bad-total"), [](Ctx &c) {
                     int made = -1;
                     c.check(c.s->makeTable(2, rng("A1:E6"), true, QStringLiteral("TableStyleMedium2"), &made), "table");
                     c.check(c.s->setTableTotalsRow(made, true), "totals row");
                     c.check(c.s->setTableTotal(made, 3, QStringLiteral("custom"), QStringLiteral("SUBTOTAL(109,[Q3])*2")), "total naming a column the table has not");
                 }});
    v.push_back({QStringLiteral("table-resize-torange"), [](Ctx &c) {
                     c.check(c.s->makeTable(0, rng("A1:E12"), true, QStringLiteral("TableStyleMedium2")), "table");
                     c.check(c.s->makeTable(2, rng("A1:B6"), true, QStringLiteral("TableStyleMedium3")), "table 2");
                     c.check(c.s->resizeTable(0, rng("A1:F12")), "resize");
                     c.check(c.s->tableToRange(1), "to range");
                     c.check(c.s->type(0, 0, 5, QStringLiteral("Extra")), "type into the header");
                     c.check(c.s->type(0, 2, 5, QStringLiteral("=B2*2")), "calculated column");
                 }});
    v.push_back({QStringLiteral("table-filter-sort"), [](Ctx &c) {
                     c.check(c.s->makeTable(0, rng("A1:E12"), true, QStringLiteral("TableStyleMedium2")), "table");
                     AutoFilter f = c.s->tableFilter(0);
                     FilterColumn col;
                     col.kind = FilterColumn::Kind::Custom;
                     col.column = 1;
                     FilterColumn::Condition cond;
                     cond.op = FilterColumn::Op::Greater;
                     cond.value = QStringLiteral("30");
                     col.conditions.append(cond);
                     f.columns.append(col);
                     c.check(c.s->setFilter(0, f, QStringLiteral("Filter"), 0), "table filter");
                 }});
    // ---- charts
    v.push_back({QStringLiteral("charts-1"), [](Ctx &c) { charts(c, 0, 8); }});
    v.push_back({QStringLiteral("charts-2"), [](Ctx &c) { charts(c, 8, 16); }});
    v.push_back({QStringLiteral("chart-changed"), [](Ctx &c) {
                     c.w->setActiveSheet(0);
                     waitFor([&] { return c.w->objects() && c.w->objects()->shownSheet() == 0; }, 20);
                     const int id = firstObject(c, ObjectKind::Chart);
                     c.w->objects()->layer()->select(id);
                     c.need(c.w->changePickedChart(QStringLiteral("Title"), [](Chart *ch) {
                         ch->title.present = true;
                         ch->title.automatic = false;
                         ch->title.text = QStringLiteral("Changed title & <more>");
                     }), "change chart");
                     c.need(c.w->moveObject(id, cellAnchor(7, 20, 14, 36)), "move chart");
                     select(c, "A1:C12");
                     c.need(c.w->insertChart(ChartKind::LineMarkers), "new chart");
                     c.need(c.w->changePickedChart(QStringLiteral("Title again"), [](Chart *ch) {
                         ch->title.present = true;
                         ch->title.text = QStringLiteral("Second");
                     }), "change new chart");
                 }});
    // ---- objects
    v.push_back({QStringLiteral("objects-moved-pasted"), [](Ctx &c) {
                     c.w->setActiveSheet(0);
                     waitFor([&] { return c.w->objects() && c.w->objects()->shownSheet() == 0; }, 20);
                     const int pic = firstObject(c, ObjectKind::Picture), shape = firstObject(c, ObjectKind::Shape), chart = firstObject(c, ObjectKind::Chart);
                     c.need(c.w->moveObject(pic, cellAnchor(0, 30, 3, 36)), "move picture");
                     c.need(c.w->moveObject(shape, cellAnchor(4, 30, 6, 34)), "move shape");
                     c.w->objects()->layer()->select(pic);
                     c.need(c.w->copyObject(false), "copy picture");
                     c.w->view()->selectCell(40, 0);
                     c.need(c.w->pasteObject(), "paste picture");
                     c.w->objects()->layer()->select(shape);
                     c.need(c.w->copyObject(false), "copy shape");
                     c.w->view()->selectCell(40, 4);
                     c.need(c.w->pasteObject(), "paste shape");
                     c.w->objects()->layer()->select(chart);
                     c.need(c.w->copyObject(false), "copy chart");
                     c.w->view()->selectCell(60, 0);
                     c.need(c.w->pasteObject(), "paste chart");
                     c.w->objects()->layer()->select(pic);
                     c.need(c.w->reorderObject(2), "to front");
                 }});
    v.push_back({QStringLiteral("objects-cut-delete"), [](Ctx &c) {
                     c.w->setActiveSheet(0);
                     waitFor([&] { return c.w->objects() && c.w->objects()->shownSheet() == 0; }, 20);
                     const int pic = firstObject(c, ObjectKind::Picture), shape = firstObject(c, ObjectKind::Shape);
                     c.w->objects()->layer()->select(shape);
                     c.need(c.w->copyObject(true), "cut shape");
                     c.w->view()->selectCell(45, 2);
                     c.need(c.w->pasteObject(), "paste shape");
                     c.check(c.s->deleteObject(0, pic, QStringLiteral("Delete"), c.w->objects()->stateOf(0, pic), [&] {
                         crunch::ObjectState st = c.w->objects()->stateOf(0, pic);
                         st.deleted = true;
                         return st;
                     }()), "delete picture");
                 }});
    v.push_back({QStringLiteral("objects-rows-inserted"), [](Ctx &c) {
                     c.check(c.s->insertRows(0, 5, 3), "insert rows above the objects");
                     c.check(c.s->insertColumns(0, 1, 2), "insert columns");
                     c.check(c.s->deleteRows(0, 20, 2), "delete rows through an object");
                 }});
    // ---- notes
    v.push_back({QStringLiteral("notes-new"), [](Ctx &c) {
                     c.check(c.s->setNote(0, 1, 1, QStringLiteral("First note"), QStringLiteral("Ann"), false, QStringLiteral("Note")), "note");
                     c.check(c.s->setNote(0, 3, 2, QStringLiteral("Shown & <escaped> note\nwith two lines"), QStringLiteral("Ann"), true, QStringLiteral("Note")), "shown note");
                     c.check(c.s->setNote(1, 0, 0, QStringLiteral("On another sheet"), QStringLiteral("Bob"), false, QStringLiteral("Note")), "note on sheet 2");
                 }});
    v.push_back({QStringLiteral("notes-edited-deleted"), [](Ctx &c) {
                     c.check(c.s->setNote(0, 1, 1, QStringLiteral("Original"), QStringLiteral("Ann"), false, QStringLiteral("Note")), "note");
                     c.check(c.s->setNote(0, 2, 1, QStringLiteral("Second"), QStringLiteral("Ann"), false, QStringLiteral("Note")), "note 2");
                     c.check(c.s->setNote(0, 1, 1, QStringLiteral("Edited"), QStringLiteral("Ann"), true, QStringLiteral("Edit note")), "edit note");
                     c.check(c.s->deleteNotes(0, {{2, 1}}, QStringLiteral("Delete note")), "delete one");
                 }});
    v.push_back({QStringLiteral("notes-none"), [](Ctx &c) {
                     c.check(c.s->setNote(0, 1, 1, QStringLiteral("Doomed"), QStringLiteral("Ann"), false, QStringLiteral("Note")), "note");
                     c.check(c.s->deleteNotes(0, {{1, 1}}, QStringLiteral("Delete note")), "delete all");
                     c.check(c.s->type(0, 20, 0, QStringLiteral("after")), "type");
                 }});
    // ---- links
    v.push_back({QStringLiteral("links"), [](Ctx &c) {
                     LinkSpec web;
                     web.range = rng("A2");
                     web.target = QStringLiteral("https://example.org/a?b=1&c=2");
                     web.display = QStringLiteral("North");
                     web.tip = QStringLiteral("Open <the> site");
                     c.check(c.s->setLink(0, web.range, web, QStringLiteral("Link")), "web link");
                     LinkSpec place;
                     place.range = rng("A3:A4");
                     place.location = QStringLiteral("Lists!B2");
                     c.check(c.s->setLink(0, place.range, place, QStringLiteral("Link")), "place link");
                     LinkSpec mail;
                     mail.range = rng("A5");
                     mail.target = QStringLiteral("mailto:a@example.org?subject=Hi there");
                     c.check(c.s->setLink(0, mail.range, mail, QStringLiteral("Link")), "mail link");
                     LinkSpec file;
                     file.range = rng("A6");
                     file.target = QStringLiteral("other file.xlsx");
                     c.check(c.s->setLink(0, file.range, file, QStringLiteral("Link")), "file link");
                     LinkSpec name;
                     name.range = rng("A7");
                     name.location = QStringLiteral("Totals");
                     c.check(c.s->setLink(0, name.range, name, QStringLiteral("Link")), "name link");
                     c.check(c.s->removeLinks(0, rng("A7"), QStringLiteral("Remove")), "remove link");
                 }});
    // ---- protection
    v.push_back({QStringLiteral("protect-sheet-plain"), [](Ctx &c) {
                     SheetProtection p;
                     p.enabled = true;
                     p.allowFormatCells = true;
                     p.allowSort = true;
                     p.allowFilter = true;
                     p.allowInsertRows = true;
                     c.check(c.s->setSheetProtection(0, p, QStringLiteral("Protect")), "protect");
                     SheetProtection q;
                     q.enabled = true;
                     q.objects = false;
                     q.scenarios = false;
                     q.allowSelectLocked = false;
                     c.check(c.s->setSheetProtection(1, q, QStringLiteral("Protect")), "protect 2");
                 }});
    v.push_back({QStringLiteral("protect-sheet-password"), [](Ctx &c) {
                     SheetProtection p;
                     p.enabled = true;
                     c.need(makePasswordHash(QStringLiteral("secret"), &p.password), "hash");
                     c.check(c.s->setSheetProtection(0, p, QStringLiteral("Protect")), "protect with password");
                     SheetProtection q;
                     q.enabled = true;
                     q.password.legacy = legacyPasswordHash(QStringLiteral("old"));
                     c.check(c.s->setSheetProtection(1, q, QStringLiteral("Protect")), "protect with the old hash");
                 }});
    v.push_back({QStringLiteral("protect-book"), [](Ctx &c) {
                     WorkbookProtection p;
                     p.lockStructure = true;
                     c.check(c.s->setWorkbookProtection(p, QStringLiteral("Protect")), "protect the structure");
                 }});
    v.push_back({QStringLiteral("protect-book-password"), [](Ctx &c) {
                     WorkbookProtection p;
                     p.lockStructure = true;
                     p.lockWindows = true;
                     c.need(makePasswordHash(QStringLiteral("secret"), &p.password), "hash");
                     c.check(c.s->setWorkbookProtection(p, QStringLiteral("Protect")), "protect with password");
                 }});
    // ---- names
    v.push_back({QStringLiteral("names"), [](Ctx &c) {
                     Session::NameDraft d;
                     d.name = QStringLiteral("Region_List");
                     d.text = QStringLiteral("=Data!$A$2:$A$12");
                     d.comment = QStringLiteral("The regions & <more>");
                     int id = -1;
                     c.check(c.s->defineName(d, &id), "define");
                     Session::NameDraft e;
                     e.name = QStringLiteral("Local");
                     e.scope = 1;
                     e.text = QStringLiteral("=Lists!$A$1");
                     c.check(c.s->defineName(e), "define scoped");
                     Session::NameDraft f;
                     f.name = QStringLiteral("Hidden_Const");
                     f.text = QStringLiteral("=42");
                     f.hidden = true;
                     c.check(c.s->defineName(f), "define constant");
                     d.name = QStringLiteral("Region_Names");
                     c.check(c.s->changeName(id, d), "rename");
                     c.check(c.s->type(0, 20, 0, QStringLiteral("=SUM(Totals)")), "formula using a name");
                     c.check(c.s->type(0, 21, 0, QStringLiteral("=COUNTA(Region_Names)")), "formula using the new name");
                 }});
    v.push_back({QStringLiteral("names-removed"), [](Ctx &c) {
                     for (int i = 0; i < 20; ++i)
                         if (c.s->nameListed(i)) {
                             c.check(c.s->removeName(i), "remove");
                             break;
                         }
                 }});
    // ---- conditional formats and validation (the seed's, carried through edits that move them)
    v.push_back({QStringLiteral("cf-dv-moved"), [](Ctx &c) {
                     c.check(c.s->insertRows(0, 3, 2), "rows");
                     c.check(c.s->insertColumns(0, 1, 1), "columns");
                     c.check(c.s->deleteRows(0, 8, 1), "delete row");
                     c.check(c.s->copySheet(0, 2, QStringLiteral("Copy of Data")), "copy");
                 }});
    // ---- filters and sorts
    v.push_back({QStringLiteral("filter-sort"), [](Ctx &c) {
                     AutoFilter f;
                     f.ref = rng("A1:E12");
                     FilterColumn col;
                     col.kind = FilterColumn::Kind::Custom;
                     col.column = 1;
                     FilterColumn::Condition a, b;
                     a.op = FilterColumn::Op::Greater;
                     a.value = QStringLiteral("20");
                     b.op = FilterColumn::Op::Less;
                     b.value = QStringLiteral("80");
                     col.conditions = {a, b};
                     col.both = true;
                     f.columns.append(col);
                     FilterColumn vals;
                     vals.kind = FilterColumn::Kind::Values;
                     vals.column = 0;
                     vals.values = {QStringLiteral("North"), QStringLiteral("South"), QStringLiteral("East"), QStringLiteral("Coast")};
                     f.columns.append(vals);
                     c.check(c.s->setFilter(0, f, QStringLiteral("Filter")), "filter");
                     crunch::SortSpec spec;
                     crunch::SortLevel lv;
                     lv.key = 2;
                     lv.descending = true;
                     spec.levels.append(lv);
                     crunch::SortLevel lv2;
                     lv2.key = 0;
                     spec.levels.append(lv2);
                     spec.hasHeader = true;
                     c.check(c.s->sortRange(0, rng("A1:E12"), spec), "sort");
                 }});
    v.push_back({QStringLiteral("filter-top10-cleared"), [](Ctx &c) {
                     AutoFilter f;
                     f.ref = rng("A1:E12");
                     FilterColumn col;
                     col.kind = FilterColumn::Kind::Top10;
                     col.column = 2;
                     col.top = true;
                     col.count = 3;
                     f.columns.append(col);
                     c.check(c.s->setFilter(0, f, QStringLiteral("Filter")), "top ten filter");
                     c.check(c.s->setFilter(0, c.s->filterCleared(0), QStringLiteral("Clear")), "clear");
                     AutoFilter g;
                     g.ref = rng("A1:E12");
                     c.check(c.s->setFilter(0, g, QStringLiteral("Filter on")), "filter on");
                 }});
    // ---- page setup
    v.push_back({QStringLiteral("page-basic"), [](Ctx &c) {
                     PageSetup p = pageOf(c, 0);
                     p.orientation = PageOrientation::Landscape;
                     p.paperSize = 9;
                     p.fitToPage = true;
                     p.fitToWidth = 1;
                     p.fitToHeight = 0;
                     p.margins.left = 0.5;
                     p.margins.top = 1.1;
                     p.gridLines = true;
                     p.headings = true;
                     p.centerHorizontally = true;
                     p.centerVertically = true;
                     p.blackAndWhite = true;
                     p.draft = true;
                     p.comments = PrintComments::AtEnd;
                     p.errors = PrintErrors::Dash;
                     p.pageOrder = PageOrder::OverThenDown;
                     p.firstPageNumber = 3;
                     p.useFirstPageNumber = true;
                     c.check(c.s->setPageSetup(0, p, QStringLiteral("Page")), "page setup");
                     PageSetup q = pageOf(c, 1);
                     q.scale = 150;
                     q.paperSize = 5;
                     q.orientation = PageOrientation::Portrait;
                     c.check(c.s->setPageSetup(1, q, QStringLiteral("Page")), "page setup 2");
                 }});
    v.push_back({QStringLiteral("page-area-titles-breaks"), [](Ctx &c) {
                     PageSetup p = pageOf(c, 0);
                     p.printArea = {rng("A1:E12"), rng("G1:H5")};
                     p.titles.hasRows = true;
                     p.titles.row1 = 0;
                     p.titles.row2 = 0;
                     p.titles.hasCols = true;
                     p.titles.col1 = 0;
                     p.titles.col2 = 0;
                     PageBreak rb;
                     rb.before = 6;
                     rb.max = 16383;
                     p.rowBreaks.append(rb);
                     PageBreak cb;
                     cb.before = 3;
                     cb.max = 1048575;
                     p.colBreaks.append(cb);
                     c.check(c.s->setPageSetup(0, p, QStringLiteral("Page")), "page setup");
                     PageSetup q = pageOf(c, 2);
                     q.printArea = {rng("A1:B6")};
                     q.titles.hasRows = true;
                     q.titles.row1 = 1;
                     q.titles.row2 = 2;
                     c.check(c.s->setPageSetup(2, q, QStringLiteral("Page")), "page setup 2");
                     c.check(c.s->insertRows(0, 2, 1), "rows moves the area");
                 }});
    v.push_back({QStringLiteral("page-header-text"), [](Ctx &c) {
                     PageSetup p = pageOf(c, 0);
                     p.headerFooter.oddHeader = QStringLiteral("&L&\"Arial,Bold\"&14Left &D&C&A&R&P of &N");
                     p.headerFooter.oddFooter = QStringLiteral("&LFile &F&CPage &P&R&T & <text>");
                     p.headerFooter.evenHeader = QStringLiteral("&LEven left&REven right");
                     p.headerFooter.evenFooter = QStringLiteral("&CEven footer");
                     p.headerFooter.firstHeader = QStringLiteral("&CFirst page");
                     p.headerFooter.firstFooter = QStringLiteral("&RFirst footer");
                     p.headerFooter.differentOddEven = true;
                     p.headerFooter.differentFirst = true;
                     p.headerFooter.scaleWithDoc = false;
                     p.headerFooter.alignWithMargins = false;
                     c.check(c.s->setPageSetup(0, p, QStringLiteral("Page")), "page setup");
                 }});
    v.push_back({QStringLiteral("page-header-pictures-plain"), [](Ctx &c) {
                     PageSetup p = pageOf(c, 0);
                     p.headerFooter.oddHeader = QStringLiteral("&L&G&C&G&R&G");
                     p.headerFooter.oddFooter = QStringLiteral("&L&G&C&G&R&G");
                     int n = 0;
                     for (const char *place : {"LH", "CH", "RH", "LF", "CF", "RF"}) p.headerFooter.pictures.append(headerPicture(QString::fromLatin1(place), n++));
                     c.check(c.s->setPageSetup(0, p, QStringLiteral("Page")), "page setup");
                 }});
    v.push_back({QStringLiteral("page-header-pictures-all"), [](Ctx &c) {
                     PageSetup p = pageOf(c, 0);
                     p.headerFooter.differentFirst = true;
                     p.headerFooter.differentOddEven = true;
                     const QString g = QStringLiteral("&L&G&C&G&R&G");
                     p.headerFooter.oddHeader = g;
                     p.headerFooter.oddFooter = g;
                     p.headerFooter.evenHeader = g;
                     p.headerFooter.evenFooter = g;
                     p.headerFooter.firstHeader = g;
                     p.headerFooter.firstFooter = g;
                     int n = 0;
                     for (const char *suffix : {"FIRST", "EVEN", ""})
                         for (const char *hf : {"H", "F"})
                             for (const char *part : {"L", "C", "R"}) p.headerFooter.pictures.append(headerPicture(QString::fromLatin1(part) + QString::fromLatin1(hf) + QString::fromLatin1(suffix), n++));
                     c.check(c.s->setPageSetup(0, p, QStringLiteral("Page")), "page setup");
                     PageSetup q = pageOf(c, 1);
                     q.headerFooter.firstHeader = QStringLiteral("&C&G");
                     q.headerFooter.differentFirst = true;
                     q.headerFooter.pictures.append(headerPicture(QStringLiteral("CHFIRST"), 3));
                     c.check(c.s->setPageSetup(1, q, QStringLiteral("Page")), "page setup 2");
                 }});
    v.push_back({QStringLiteral("page-views-zoom"), [](Ctx &c) {
                     c.s->setSheetView(0, 1, 60);
                     c.s->setSheetView(1, 2, 80);
                     c.s->setSheetView(2, 0, 150);
                     c.need(true, "views");
                     c.check(c.s->type(0, 20, 0, QStringLiteral("x")), "an edit so that the file is saved");
                 }});
    v.push_back({QStringLiteral("freeze-panes"), [](Ctx &c) {
                     c.w->view()->selectCell(1, 1);
                     c.w->act(QStringLiteral("view.freeze"))->trigger();
                     c.w->setActiveSheet(1);
                     settle(*c.w);
                     c.w->view()->selectCell(2, 0);
                     c.w->act(QStringLiteral("view.freeze"))->trigger();
                     c.check(c.s->type(0, 20, 0, QStringLiteral("x")), "an edit so that the file is saved");
                 }});
    // ---- sheets
    v.push_back({QStringLiteral("sheets-ops"), [](Ctx &c) {
                     int made = -1;
                     c.check(c.s->insertSheet(1, QStringLiteral("Inserted"), &made), "insert");
                     c.check(c.s->copySheet(0, 4, QStringLiteral("Data (2)"), &made), "copy");
                     c.check(c.s->renameSheet(2, QStringLiteral("Lists & more")), "rename");
                     c.check(c.s->moveSheet(0, -1), "move");
                     c.check(c.s->deleteSheet(3), "delete");
                     c.check(c.s->setSheetHidden(2, true), "hide");
                     ColorSpec col = ColorSpec::fromRgb(0x00B050);
                     c.check(c.s->setTabColor(1, col), "tab color");
                     c.check(c.s->type(1, 0, 0, QStringLiteral("=Data!B2+1")), "formula to a moved sheet");
                 }});
    v.push_back({QStringLiteral("sheets-copy-with-things"), [](Ctx &c) {
                     c.check(c.s->makeTable(0, rng("A1:E12"), true, QStringLiteral("TableStyleMedium2")), "table");
                     c.check(c.s->setNote(0, 1, 1, QStringLiteral("n"), QStringLiteral("Ann"), false, QStringLiteral("Note")), "note");
                     PageSetup p = pageOf(c, 0);
                     p.printArea = {rng("A1:E12")};
                     p.headerFooter.oddHeader = QStringLiteral("&L&G");
                     p.headerFooter.pictures.append(headerPicture(QStringLiteral("LH"), 2));
                     c.check(c.s->setPageSetup(0, p, QStringLiteral("Page")), "page setup");
                     c.check(c.s->copySheet(0, 4, QStringLiteral("Copy A")), "copy");
                     c.check(c.s->copySheet(0, 4, QStringLiteral("Copy B")), "copy again");
                     c.check(c.s->renameSheet(0, QStringLiteral("Renamed Data")), "rename");
                 }});
    v.push_back({QStringLiteral("sheets-many"), [](Ctx &c) {
                     for (int i = 0; i < 120; ++i) c.check(c.s->insertSheet(-1, QStringLiteral("Sheet %1").arg(i + 5)), "insert");
                 }});
    v.push_back({QStringLiteral("sheets-long-names"), [](Ctx &c) {
                     c.check(c.s->renameSheet(1, QStringLiteral("Thirty-one characters in a name")), "rename long");
                     c.check(c.s->renameSheet(2, QStringLiteral("It's 'quoted' & <odd>")), "rename odd");
                 }});
    // ---- merged cells, formats, styles
    v.push_back({QStringLiteral("merges"), [](Ctx &c) {
                     c.check(c.s->mergeCells(0, rng("A20:C20"), Session::MergeKind::MergeAndCenter), "merge and center");
                     c.check(c.s->mergeCells(0, rng("A22:C25"), Session::MergeKind::Across), "merge across");
                     c.check(c.s->mergeCells(0, rng("G1:H1"), Session::MergeKind::Unmerge), "unmerge");
                     c.check(c.s->mergeCells(0, rng("J1:K3"), Session::MergeKind::Merge), "merge");
                 }});
    v.push_back({QStringLiteral("formats-styles"), [](Ctx &c) {
                     FormatChange f;
                     f.fontName = QStringLiteral("Georgia");
                     f.fontSize = 13;
                     f.bold = true;
                     f.italic = true;
                     f.strike = true;
                     f.underline = Underline::Double;
                     f.fontColor = ColorSpec::fromRgb(0xC00000);
                     FillSpec fill;
                     fill.pattern = PatternKind::Solid;
                     fill.fg = ColorSpec::fromRgb(0xFFEB9C);
                     f.fill = fill;
                     f.numberFormat = QStringLiteral("#,##0.00;[Red]-#,##0.00");
                     f.horizontal = HAlign::Center;
                     f.vertical = jp::office::sheet::VAlign::Top;
                     f.wrap = true;
                     f.indent = 1;
                     f.rotation = 45;
                     f.locked = false;
                     c.check(c.s->formatCells(0, rng("B2:E5"), f, QStringLiteral("Format")), "format");
                     FormatChange g;
                     FillSpec pat;
                     pat.pattern = PatternKind::LightUp;
                     pat.fg = ColorSpec::fromRgb(0x808080);
                     pat.bg = ColorSpec::fromRgb(0xFFFFFF);
                     g.fill = pat;
                     g.numberFormat = QStringLiteral("yyyy-mm-dd hh:mm");
                     g.shrink = true;
                     c.check(c.s->formatCells(0, rng("B7:C9"), g, QStringLiteral("Format")), "format 2");
                     BorderSide line;
                     line.style = BorderStyle::Medium;
                     line.color = ColorSpec::fromRgb(0x0000FF);
                     c.check(c.s->setBorders(0, rng("A1:E12"), Session::EdgeLeft | Session::EdgeRight | Session::EdgeTop | Session::EdgeBottom | Session::EdgeInsideHorizontal, line, QStringLiteral("Borders")), "borders");
                     BorderSide dbl;
                     dbl.style = BorderStyle::Double;
                     c.check(c.s->setBorders(0, rng("A13:E13"), Session::EdgeTop, dbl, QStringLiteral("Borders")), "double border");
                     c.check(c.s->applyStyleNamed(0, rng("A15:C15"), QStringLiteral("Good"), QStringLiteral("Style")), "style Good");
                     c.check(c.s->applyStyleNamed(0, rng("A16:C16"), QStringLiteral("Heading 1"), QStringLiteral("Style")), "style Heading 1");
                     c.check(c.s->applyStyleNamed(0, rng("A17:C17"), QStringLiteral("Currency"), QStringLiteral("Style")), "style Currency");
                     c.check(c.s->clearFormats(0, rng("A16")), "clear formats");
                 }});
    v.push_back({QStringLiteral("sizes-fill-paste"), [](Ctx &c) {
                     Session::SizeChange w;
                     w.setSize = true;
                     w.size = 24;
                     c.check(c.s->resizeColumns(0, 0, 1, w, QStringLiteral("Width")), "width");
                     Session::SizeChange h;
                     h.setSize = true;
                     h.size = 30;
                     c.check(c.s->resizeRows(0, 1, 3, h, QStringLiteral("Height")), "height");
                     c.check(c.s->setDefaultSizes(0, 12.5, 18, QStringLiteral("Default sizes")), "default sizes");
                     c.check(c.s->fill(0, rng("B2:B3"), Session::FillDirection::Down, 5), "fill");
                     const crunch::ClipData clip = c.s->copyCells(0, rng("A1:E4"), false);
                     c.check(c.s->paste(0, rng("A30"), clip, Session::PasteOptions()), "paste");
                     c.check(c.s->sortRange(0, rng("A2:E12"), [] {
                         crunch::SortSpec s;
                         crunch::SortLevel l;
                         l.key = 1;
                         s.levels.append(l);
                         return s;
                     }()), "sort");
                 }});
    // ---- calculation, arrays, long text
    v.push_back({QStringLiteral("calc-manual-iterate"), [](Ctx &c) {
                     crunch::CalcSettings s;
                     s.mode = CalcMode::Manual;
                     s.iterate = true;
                     s.count = 50;
                     s.delta = 0.01;
                     c.check(c.s->setCalcSettings(s), "calc settings");
                     c.check(c.s->type(0, 20, 0, QStringLiteral("=A22+1")), "circular");
                     c.check(c.s->type(0, 21, 0, QStringLiteral("=A21+1")), "circular 2");
                 }});
    v.push_back({QStringLiteral("calc-nottables"), [](Ctx &c) {
                     crunch::CalcSettings s;
                     s.mode = CalcMode::AutoNoTable;
                     c.check(c.s->setCalcSettings(s), "calc settings");
                     c.check(c.s->type(0, 20, 0, QStringLiteral("=SUM(B2:B12)")), "formula");
                 }});
    v.push_back({QStringLiteral("long-text"), [](Ctx &c) {
                     c.check(c.s->type(0, 20, 0, QString(32767, QLatin1Char('x'))), "text of 32,767 characters");
                     c.check(c.s->type(0, 21, 0, QStringLiteral("multi\nline <&> text\twith tab and  spaces ")), "multi line text");
                     QString formula = QStringLiteral("=1");
                     for (int i = 0; i < 400; ++i) formula += QStringLiteral("+B2");
                     c.check(c.s->type(0, 22, 0, formula), "long formula");
                     c.check(c.s->type(0, 23, 0, QStringLiteral("'=not a formula")), "apostrophe");
                     c.check(c.s->type(0, 24, 0, QString::fromUtf8("unicode \xe2\x82\xac \xf0\x9f\x98\x80 \xe4\xb8\xad\xe6\x96\x87")), "unicode");
                     c.check(c.s->type(0, 25, 0, QStringLiteral("2024-03-05")), "date");
                     c.check(c.s->type(0, 26, 0, QStringLiteral("12:30")), "time");
                     c.check(c.s->type(0, 27, 0, QStringLiteral("#N/A")), "error");
                     c.check(c.s->type(0, 28, 0, QStringLiteral("TRUE")), "boolean");
                 }});
    v.push_back({QStringLiteral("array-formulas"), [](Ctx &c) {
                     c.check(c.s->insertRows(0, 5, 1), "insert rows");
                     c.check(c.s->type(0, 30, 0, QStringLiteral("=SUM(B2:B12*2)")), "formula");
                     c.check(c.s->fill(0, rng("B2:B2"), Session::FillDirection::Right, 3), "fill");
                     c.check(c.s->deleteColumns(0, 6, 1), "delete the column with the array formula");
                 }});
    // ---- the edges of what the program accepts: each in a file of its own
    v.push_back({QStringLiteral("edge-name-ref-error"), [](Ctx &c) {
                     Session::NameDraft d;
                     d.name = QStringLiteral("Gone");
                     d.text = QStringLiteral("=#REF!");
                     c.check(c.s->defineName(d), "name that is #REF!");
                     Session::NameDraft e;
                     e.name = QStringLiteral("Other_Sheet");
                     e.text = QStringLiteral("=Nowhere!$A$1");
                     c.check(c.s->defineName(e), "name on a sheet that is not there");
                     Session::NameDraft f;
                     f.name = QStringLiteral("R1C1x");
                     f.text = QStringLiteral("=Data!$A$1:$B$2,Data!$D$1");
                     c.check(c.s->defineName(f), "name of two blocks");
                 }});
    v.push_back({QStringLiteral("edge-header-text-long"), [](Ctx &c) {
                     PageSetup p = pageOf(c, 0);
                     p.headerFooter.oddHeader = QStringLiteral("&L") + QString(300, QLatin1Char('L')) + QStringLiteral("&C") + QString(300, QLatin1Char('C')) + QStringLiteral("&R") + QString(300, QLatin1Char('R'));
                     p.headerFooter.oddFooter = QString(700, QLatin1Char('f'));
                     c.check(c.s->setPageSetup(0, p, QStringLiteral("Page")), "long header and footer");
                 }});
    v.push_back({QStringLiteral("edge-note-long"), [](Ctx &c) {
                     c.check(c.s->setNote(0, 1, 1, QString(32767, QLatin1Char('n')), QStringLiteral("Ann"), false, QStringLiteral("Note")), "a note of 32,767 characters");
                     c.check(c.s->setNote(0, 2, 1, QStringLiteral("tab\there\nline\r\nline2 \x01 bell"), QString(), false, QStringLiteral("Note")), "a note with controls and no author");
                     c.check(c.s->setNote(0, 3, 1, QString::fromUtf8("\xf0\x9f\x98\x80 emoji & <tags> \"quotes\""), QStringLiteral("A \"B\" <C>"), true, QStringLiteral("Note")), "a note with markup");
                 }});
    v.push_back({QStringLiteral("edge-link-long"), [](Ctx &c) {
                     LinkSpec web;
                     web.range = rng("A2");
                     web.target = QStringLiteral("https://example.org/") + QString(2000, QLatin1Char('p'));
                     web.display = QString(400, QLatin1Char('d'));
                     web.tip = QString(400, QLatin1Char('t'));
                     c.check(c.s->setLink(0, web.range, web, QStringLiteral("Link")), "a long link");
                     LinkSpec place;
                     place.range = rng("A3");
                     place.location = QStringLiteral("'Nowhere sheet'!A1");
                     c.check(c.s->setLink(0, place.range, place, QStringLiteral("Link")), "a link to a sheet that is not there");
                 }});
    v.push_back({QStringLiteral("edge-table-odd-headers"), [](Ctx &c) {
                     c.check(c.s->type(2, 0, 0, QStringLiteral("A&B <x> \"q\"")), "header");
                     c.check(c.s->type(2, 0, 1, QStringLiteral("[bracket]")), "header");
                     c.check(c.s->type(2, 0, 2, QStringLiteral("a b")), "header");
                     c.check(c.s->type(2, 0, 3, QStringLiteral("a b")), "header twice");
                     c.check(c.s->type(2, 0, 4, QString(300, QLatin1Char('h'))), "header of 300 characters");
                     c.check(c.s->makeTable(2, rng("A1:E6"), true, QStringLiteral("TableStyleBogus")), "table with an unknown look");
                 }});
    v.push_back({QStringLiteral("edge-page-extremes"), [](Ctx &c) {
                     PageSetup p = pageOf(c, 0);
                     p.scale = 400;
                     p.paperSize = 119;
                     p.margins.left = 0;
                     p.margins.right = 0;
                     p.margins.top = 0;
                     p.margins.bottom = 0;
                     p.margins.header = 0;
                     p.margins.footer = 0;
                     p.firstPageNumber = -5;
                     p.useFirstPageNumber = true;
                     c.check(c.s->setPageSetup(0, p, QStringLiteral("Page")), "extremes");
                     PageSetup q = pageOf(c, 1);
                     q.fitToPage = true;
                     q.fitToWidth = 32767;
                     q.fitToHeight = 0;
                     q.printArea = {rng("A1:XFD1048576")};
                     c.check(c.s->setPageSetup(1, q, QStringLiteral("Page")), "fit and a whole-sheet area");
                     PageSetup r = pageOf(c, 2);
                     r.titles.hasRows = true;
                     r.titles.row1 = 0;
                     r.titles.row2 = 1048575;
                     r.titles.hasCols = true;
                     r.titles.col1 = 0;
                     r.titles.col2 = 16383;
                     c.check(c.s->setPageSetup(2, r, QStringLiteral("Page")), "titles of everything");
                 }});
    v.push_back({QStringLiteral("edge-sheets-hidden"), [](Ctx &c) {
                     c.check(c.s->setSheetHidden(1, true), "hide");
                     c.check(c.s->setSheetHidden(2, true), "hide");
                     c.check(c.s->setSheetHidden(3, true), "hide");
                     c.check(c.s->renameSheet(0, QStringLiteral("history")), "rename to a reserved word");
                 }});
    v.push_back({QStringLiteral("edge-cells"), [](Ctx &c) {
                     c.check(c.s->type(0, 1048575, 16383, QStringLiteral("corner")), "the last cell");
                     c.check(c.s->type(0, 0, 16383, QStringLiteral("=XFD1048576+1")), "a formula to the last cell");
                     c.check(c.s->type(0, 30, 0, QStringLiteral("1E+308")), "a huge number");
                     c.check(c.s->type(0, 31, 0, QStringLiteral("-1E-308")), "a tiny number");
                     c.check(c.s->type(0, 32, 0, QStringLiteral("=1/0")), "an error");
                     c.check(c.s->type(0, 33, 0, QStringLiteral("=NOW()")), "a volatile formula");
                     c.check(c.s->type(0, 34, 0, QStringLiteral("=INDIRECT(\"Data!A1\")+OFFSET(A1,1,1)")), "volatile references");
                     c.check(c.s->type(0, 35, 0, QStringLiteral("=\"a\"&\"\"\"\"&CHAR(10)&\"b\"")), "quotes and a line break in text");
                     c.check(c.s->type(0, 36, 0, QStringLiteral("=SUM(Lists!A1:Extra!B3)")), "a 3-D range");
                     c.check(c.s->type(0, 37, 0, QStringLiteral("=SUM(Nowhere!A1)")), "a sheet that is not there");
                 }});
    v.push_back({QStringLiteral("edge-formula-limits"), [](Ctx &c) {
                     QString args = QStringLiteral("=SUM(1");
                     for (int i = 1; i < 256; ++i) args += QStringLiteral(",1");
                     c.check(c.s->type(0, 30, 0, args + QLatin1Char(')')), "256 arguments");
                     QString long8192 = QStringLiteral("=1");
                     while (long8192.size() < 8191) long8192 += QStringLiteral("+1");
                     c.check(c.s->type(0, 31, 0, long8192), "a formula of 8,191 characters");
                     c.check(c.s->type(0, 32, 0, long8192 + QStringLiteral("+1")), "a formula of 8,193 characters");
                 }});
    v.push_back({QStringLiteral("edge-font-name"), [](Ctx &c) {
                     FormatChange f;
                     f.fontName = QString(32, QLatin1Char('F'));
                     c.check(c.s->formatCells(0, rng("A1:B2"), f, QStringLiteral("Format")), "a font name of 32 characters");
                     FormatChange g;
                     g.numberFormat = QString(256, QLatin1Char('#'));
                     c.check(c.s->formatCells(0, rng("A3:B4"), g, QStringLiteral("Format")), "a number format of 256 characters");
                 }});
    v.push_back({QStringLiteral("edge-page-breaks"), [](Ctx &c) {
                     PageSetup p = pageOf(c, 0);
                     for (int i = 1; i <= 1025; ++i) {
                         PageBreak rb;
                         rb.before = quint32(i);
                         rb.max = 16383;
                         p.rowBreaks.append(rb);
                         PageBreak cb;
                         cb.before = quint32(i);
                         cb.max = 1048575;
                         p.colBreaks.append(cb);
                     }
                     c.check(c.s->setPageSetup(0, p, QStringLiteral("Page")), "1,025 breaks of each kind");
                 }});
    v.push_back({QStringLiteral("sheet-empty-edit"), [](Ctx &c) {
                     c.check(c.s->type(3, 0, 0, QStringLiteral("first")), "type into the empty sheet");
                     c.check(c.s->type(3, 5, 5, QStringLiteral("=A1&\"!\"")), "formula");
                     c.check(c.s->setNote(3, 0, 0, QStringLiteral("note on the empty sheet"), QStringLiteral("Ann"), false, QStringLiteral("Note")), "note");
                 }});
    return v;
}

int makeCheckFiles(const QString &dir)
{
    QDir().mkpath(dir);
    const QByteArray seed = seedWorkbook();
    {
        QFile f(dir + QStringLiteral("/seed.xlsx"));
        if (!f.open(QIODevice::WriteOnly)) return 1;
        f.write(seed);
    }
    QTemporaryDir tmp;
    int bad = 0;
    const QString only = qEnvironmentVariable("JC_CHECK_ONLY");   // (a part of a name: make just those files)
    for (const Scenario &sc : scenarios()) {
        if (!only.isEmpty() && !sc.name.contains(only)) continue;
        const QString in = tmp.filePath(sc.name + QStringLiteral(".xlsx"));
        {
            QFile f(in);
            if (!f.open(QIODevice::WriteOnly)) return 1;
            f.write(seed);
        }
        QString result = QStringLiteral("saved");
        {
            CrunchWindow w;
            w.resize(1280, 800);
            w.show();
            if (!openWindow(w, in) || !w.session()) {
                result = QStringLiteral("not opened");
                ++bad;
            } else {
                w.session()->setLocale(QLocale::c());
                waitFor([&] { return w.objects() && w.objects()->shownSheet() == 0; }, 20);
                Ctx c{&w, w.session()};
                sc.run(c);
                const QString target = dir + QLatin1Char('/') + sc.name + QStringLiteral(".xlsx");
                if (!saveWindow(w, target)) {
                    result = QStringLiteral("not saved");
                    ++bad;
                } else if (c.failed > 0) {
                    result = QStringLiteral("saved, %1 steps refused: %2").arg(c.failed).arg(c.notes.join(QLatin1String("; ")));
                }
            }
        }
        out() << sc.name << '\t' << result << "\n";
        out().flush();
    }
    return bad ? 3 : 0;
}

int practiceFiles(const QString &list, const QString &dir, int count, int step)
{
    QDir().mkpath(dir);
    QFile f(list);
    if (!f.open(QIODevice::ReadOnly)) return 1;
    const QList<QByteArray> lines = f.readAll().split('\n');
    QFile index(dir + QStringLiteral("/index.tsv"));
    if (!index.open(QIODevice::WriteOnly)) return 1;
    int made = 0;
    for (int i = 0; i < lines.size() && made < count; i += std::max(1, step)) {
        const QString path = QString::fromUtf8(lines[i]).trimmed();
        if (path.isEmpty() || !path.endsWith(QLatin1String(".xlsx"), Qt::CaseInsensitive)) continue;
        QString result = QStringLiteral("saved");
        const QString target = QStringLiteral("%1/%2.xlsx").arg(dir).arg(made, 4, 10, QLatin1Char('0'));
        {
            CrunchWindow w;
            w.resize(1280, 800);
            w.show();
            if (!openWindow(w, path) || !w.session() || !w.loaded() || !w.loaded()->problem.isEmpty()) {
                result = QStringLiteral("not opened");
            } else {
                int sheet = -1;
                const auto &info = w.loaded()->book->info();
                for (int si = 0; si < int(info.sheets.size()) && sheet < 0; ++si)
                    if (info.sheets[si].kind == SheetKind::Worksheet && !w.calculation()->sheet(si).isProtected) sheet = si;
                if (sheet < 0) {
                    result = QStringLiteral("no sheet to edit");
                } else {
                    const quint32 row = std::min<quint32>(w.calculation()->sheet(sheet).extentRows + 1, kMaxRows - 2), col = 0;
                    w.session()->setLocale(QLocale::c());
                    const auto o = w.session()->type(sheet, row, col, QStringLiteral("12345"));
                    settle(w);
                    if (!o.ok) result = QStringLiteral("edit refused: ") + o.problem.left(60);
                    else if (!saveWindow(w, target)) result = QStringLiteral("not saved");
                }
            }
        }
        QTextStream(&index) << made << '\t' << path << '\t' << result << '\n';
        out() << made << '\t' << result << "\n";
        out().flush();
        if (result == QLatin1String("saved")) ++made;
        else QFile::remove(target);
    }
    return 0;
}

} // namespace

namespace jp::crunchtool {

int checkFilesMain(const QStringList &args)
{
    if (args.size() == 2 && args[0] == QLatin1String("make-check-files")) return makeCheckFiles(args[1]);
    if (args.size() >= 3 && args.size() <= 5 && args[0] == QLatin1String("check-files-practice"))
        return practiceFiles(args[1], args[2], args.value(3, QStringLiteral("200")).toInt(), args.value(4, QStringLiteral("40")).toInt());
    return -1;
}

} // namespace jp::crunchtool
