// Tests of JeffCrunch's embedded objects: a range, chart, or page of one of our apps is held as an object of the sheet (its package and its picture, not a picture), and an object another
// program made is kept as the file has it. Insert > Object, Paste Special as an object, copy and paste, move and size, delete, Undo and Redo, the edit in the app that made it, saving to
// .xlsx and .ods and opening again, the list in File > Info, and hostile files. Every file is made here in code; no other program is started (the edit's opener is a stand-in that notes the
// path).
#include "../sheet/drawinghelpers.h"
#include "../sheet/odshelpers.h"
#include "../withdialog.h"
#include "app/crunchwindow.h"
#include "app/objects.h"
#include "app/objectsdialog.h"
#include "app/theme.h"
#include "core/fonts.h"
#include "app/sheettabs.h"
#include "sheet/drawing.h"
#include "core/objectgroup.h"
#include "core/sheetobjects.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "io/zip.h"
#include "office/package.h"
#include "savedok.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QMimeData>
#include <QPushButton>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoStack>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;
namespace ole = jp::office::ole;
using jp::office::sheet::AnchorKind;
using jp::office::sheet::DrawingAnchor;

namespace {

objects::Grid gridOf(const QStringList &texts, int columns)
{
    objects::Grid grid;
    for (int i = 0; i < texts.size(); i += columns) {
        QVector<objects::CellValue> row;
        for (int c = 0; c < columns && i + c < texts.size(); ++c) {
            objects::CellValue v;
            v.text = texts[i + c];
            bool number = false;
            v.value = v.text.toDouble(&number);
            v.number = number;
            row.push_back(v);
        }
        grid.push_back(row);
    }
    return grid;
}

QByteArray docxOf(const QStringList &paragraphs)
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    QByteArray body;
    for (const QString &p : paragraphs) body += "<w:p><w:r><w:t xml:space=\"preserve\">" + p.toUtf8() + "</w:t></w:r></w:p>";
    w.addPart(QStringLiteral("/word/document.xml"),
              QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body>") + body
                  + "</w:body></w:document>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    return w.finish();
}

QString savedFile(const QTemporaryDir &dir, const QString &name, const QByteArray &bytes)
{
    const QString path = dir.filePath(name);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return QString();
    f.write(bytes);
    return path;
}

QMap<QString, QByteArray> unzip(const QByteArray &zip)
{
    QMap<QString, QByteArray> parts;
    readZip(zip, parts);
    return parts;
}

QMap<QString, QByteArray> unzipFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};
    return unzip(f.readAll());
}

// The places of the objects a sheet's XML names, in order: from column, its offset, from row, its offset, then the same for the end.
QList<QList<qint64>> savedAnchors(const QByteArray &sheetXml)
{
    static const QRegularExpression point(QStringLiteral("<(?:\\w+:)?col>(\\d+)</[^>]*><(?:\\w+:)?colOff>(-?\\d+)</[^>]*><(?:\\w+:)?row>(\\d+)</[^>]*><(?:\\w+:)?rowOff>(-?\\d+)</"));
    QList<QList<qint64>> out;
    const QString xml = QString::fromUtf8(sheetXml.mid(std::max<qsizetype>(0, sheetXml.indexOf("<oleObjects"))));
    qsizetype at = 0;
    for (;;) {
        const qsizetype from = xml.indexOf(QStringLiteral("from>"), at);
        if (from < 0) break;
        const qsizetype to = xml.indexOf(QStringLiteral("to>"), from);
        if (to < 0) break;
        QList<qint64> numbers;
        for (const qsizetype where : {from, to}) {
            const QRegularExpressionMatch m = point.match(xml, where);
            for (int i = 1; i <= 4; ++i) numbers << m.captured(i).toLongLong();
        }
        out << numbers;
        at = to + 3;
    }
    return out;
}

QList<qint64> flat(const DrawingAnchor &a)
{
    return {qint64(a.from.col), qint64(a.from.colOff), qint64(a.from.row), qint64(a.from.rowOff), qint64(a.to.col), qint64(a.to.colOff), qint64(a.to.row), qint64(a.to.rowOff)};
}

QStringList partsUnder(const QMap<QString, QByteArray> &parts, const QString &folder)
{
    QStringList out;
    for (auto it = parts.begin(); it != parts.end(); ++it)
        if (it.key().startsWith(folder)) out << it.key();
    return out;
}

// What one object of a built workbook is: its program, its package's bytes (not written if `write` is false), its picture, and where it is (two cells). A picture that is not a PNG
// (`previewExt`, `previewType`) is written under its own extension and type.
struct Spec {
    QString progId = QStringLiteral("Foo.Widget.1");
    QByteArray bytes = "foreign package bytes";
    bool write = true;
    QByteArray png;
    QString previewExt = QStringLiteral("png"), previewType = QStringLiteral("image/png");
    int fromCol = 1, fromRow = 1, toCol = 4, toRow = 6;
    QString offsets = QStringLiteral("0");
};

// The `oleObjects` element of the specs of a sheet, and their parts (numbered from `first`, the sheet's number from 0).
QString oleChildren(const QVector<Spec> &specs, int first, int sheet, QVector<sheettest::ExtraPart> *parts)
{
    QString children;
    for (int i = 0; i < specs.size(); ++i) {
        const Spec &s = specs[i];
        const int n = first + i;
        const QString anchor = QStringLiteral("<anchor moveWithCells=\"1\"><xdr:from><xdr:col>%1</xdr:col><xdr:colOff>%5</xdr:colOff><xdr:row>%2</xdr:row><xdr:rowOff>%5</xdr:rowOff></xdr:from><xdr:to><xdr:col>%3</xdr:col>"
                                              "<xdr:colOff>%5</xdr:colOff><xdr:row>%4</xdr:row><xdr:rowOff>%5</xdr:rowOff></xdr:to></anchor>")
                                   .arg(s.fromCol).arg(s.fromRow).arg(s.toCol).arg(s.toRow).arg(s.offsets);
        const QString pic = s.png.isEmpty() ? QString() : QStringLiteral(" r:id=\"rIdP%1\"").arg(n);
        children += QStringLiteral("<oleObject xmlns:xdr=\"%1\" progId=\"%2\" shapeId=\"%3\" r:id=\"rIdO%4\"><objectPr defaultSize=\"0\"%5>%6</objectPr></oleObject>")
                        .arg(drawtest::kXdr, s.progId).arg(1024 + n).arg(n).arg(pic, anchor);
        parts->append({s.write ? QStringLiteral("xl/embeddings/oleObject%1.bin").arg(n) : QStringLiteral("xl/unused/r%1.bin").arg(n), s.write ? s.bytes : QByteArray("x"), QString(), sheet,
                       QStringLiteral("rIdO%1").arg(n), QStringLiteral("oleObject"), QStringLiteral("/xl/embeddings/oleObject%1.bin").arg(n)});
        if (!s.png.isEmpty())
            parts->append({QStringLiteral("xl/media/p%1.%2").arg(n).arg(s.previewExt), s.png, s.previewExt == QLatin1String("png") ? QString() : s.previewType, sheet, QStringLiteral("rIdP%1").arg(n),
                           QStringLiteral("image"), QStringLiteral("/xl/media/p%1.%2").arg(n).arg(s.previewExt)});
    }
    return children.isEmpty() ? QString() : QStringLiteral("<oleObjects>") + children + QStringLiteral("</oleObjects>");
}

// A workbook (A1 of each sheet holds 1) with the objects of the specs in the `oleObjects` of its first sheet (and those of `second` in a second sheet's), each with a part and a picture of its own.
QByteArray bookWith(const QVector<Spec> &specs, const QVector<Spec> &second = QVector<Spec>())
{
    QVector<sheettest::ExtraPart> parts;
    const QString first = oleChildren(specs, 1, 0, &parts);
    const QString other = oleChildren(second, specs.size() + 1, 1, &parts);
    QVector<QByteArray> sheets{sheettest::worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), first)};
    if (!second.isEmpty()) sheets.append(sheettest::worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), other));
    return sheettest::withParts(sheettest::buildXlsx(sheets), parts);
}

// A PNG that says it is `side` pixels either way and holds one color (it compresses to a few kilobytes, and unpacks to side * side * 4 bytes).
QByteArray hugePng(int side)
{
    QImage image(side, side, QImage::Format_Mono);
    image.fill(0);
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return png;
}

bool ready(EditWindow &e, const QByteArray &book = sheetOf({"A1=1"}))
{
    if (!e.open(book) || !e.settle()) return false;
    return QTest::qWaitFor([&] { return e.win.w.objects()->shownSheet() == 0; }, 30000);
}


// An .ods with a binary object and a folder object of another program in its shapes, each with a picture.
QByteArray odsWithForeignObjects()
{
    const QString styles = QStringLiteral("<style:style style:name=\"co1\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"1in\"/></style:style>"
                                          "<style:style style:name=\"ro1\" style:family=\"table-row\"><style:table-row-properties style:row-height=\"0.5in\"/></style:style>");
    const QString frames = QStringLiteral("<draw:frame draw:name=\"O1\" svg:width=\"1.5in\" svg:height=\"0.5in\" svg:x=\"1in\" svg:y=\"0.1in\"><draw:object-ole xlink:href=\"./Object 1\"/><draw:image xlink:href=\"ObjectReplacements/Object 1\"/></draw:frame>"
                                           "<draw:frame draw:name=\"O2\" svg:width=\"1.5in\" svg:height=\"0.5in\" svg:x=\"1in\" svg:y=\"0.8in\"><draw:object xlink:href=\"./Object 2\"/><draw:image xlink:href=\"ObjectReplacements/Object 2\"/></draw:frame>");
    const QString body = QStringLiteral("<table:table table:name=\"S\"><table:shapes>%1</table:shapes><table:table-column table:style-name=\"co1\" table:number-columns-repeated=\"10\"/>"
                                        "<table:table-row table:style-name=\"ro1\" table:number-rows-repeated=\"10\"><table:table-cell table:number-columns-repeated=\"10\"/></table:table-row></table:table>").arg(frames);
    QByteArray blob;
    for (int i = 0; i < 2000; ++i) blob.append(char((i * 31 + 7) & 0xFF));
    const QByteArray png = drawtest::pngBytes(4, 4, Qt::red);
    const QMap<QString, QByteArray> parts{{QStringLiteral("Object 1"), blob},
                                          {QStringLiteral("Object 2/content.xml"), "<?xml version=\"1.0\"?><office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"><office:body><office:drawing/></office:body></office:document-content>"},
                                          {QStringLiteral("ObjectReplacements/Object 1"), png},
                                          {QStringLiteral("ObjectReplacements/Object 2"), png}};
    const QByteArray manifest = "<?xml version=\"1.0\"?><manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\"><manifest:file-entry manifest:full-path=\"/\" manifest:media-type=\"application/vnd.oasis.opendocument.spreadsheet\"/>"
                                "<manifest:file-entry manifest:full-path=\"Object 1\" manifest:media-type=\"application/vnd.sun.star.oleobject\"/>"
                                "<manifest:file-entry manifest:full-path=\"Object 2/\" manifest:media-type=\"application/vnd.oasis.opendocument.graphics\"/>"
                                "<manifest:file-entry manifest:full-path=\"Object 2/content.xml\" manifest:media-type=\"text/xml\"/>"
                                "<manifest:file-entry manifest:full-path=\"ObjectReplacements/Object 1\" manifest:media-type=\"image/png\"/>"
                                "<manifest:file-entry manifest:full-path=\"ObjectReplacements/Object 2\" manifest:media-type=\"image/png\"/></manifest:manifest>";
    return odstest::buildOds(odstest::contentXml(body, styles), QString(), QByteArray(), parts, QStringLiteral("application/vnd.oasis.opendocument.spreadsheet"), manifest);
}

// The bytes of every object's part (the folders' content.xml, the binary parts) of an .ods, as a list of what they are.
QByteArrayList odsObjectBytes(const QMap<QString, QByteArray> &parts)
{
    QByteArrayList out;
    for (auto it = parts.begin(); it != parts.end(); ++it)
        if (it.key().startsWith(QStringLiteral("Object ")) && !it.key().startsWith(QStringLiteral("ObjectReplacements"))) out << it.value();
    std::sort(out.begin(), out.end());
    return out;
}

bool readyAs(EditWindow &e, const QByteArray &bytes, const QString &name)
{
    if (!e.open(bytes, name) || !e.settle()) return false;
    return QTest::qWaitFor([&] { return e.win.w.objects()->shownSheet() == 0; }, 30000);
}

DrawingAnchor cells(quint32 c1, quint32 r1, quint32 c2, quint32 r2)
{
    DrawingAnchor a;
    a.kind = AnchorKind::TwoCell;
    a.from.col = c1;
    a.from.row = r1;
    a.to.col = c2;
    a.to.row = r2;
    return a;
}

bool sameCells(const DrawingAnchor &a, const DrawingAnchor &b)
{
    return a.from.col == b.from.col && a.from.row == b.from.row && a.to.col == b.to.col && a.to.row == b.to.row;
}

bool showSheet(EditWindow &e, int sheet)
{
    e.win.w.sheetTabs()->setCurrentIndex(sheet);
    return QTest::qWaitFor([&] { return e.win.w.activeSheet() == sheet && e.win.w.objects()->shownSheet() == sheet; }, 30000);
}

crunch::SheetEmbeds *embedsOf(EditWindow &e) { return e.win.w.objects()->embeds(); }

// The window's file, saved and read back: the saved bytes, as parts.
bool saveAndRead(EditWindow &e, QMap<QString, QByteArray> *parts)
{
    e.select(QStringLiteral("H20"));   // (a workbook nothing was changed in is not written)
    e.enter(QStringLiteral("7"));
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    if (!e.win.w.save() || !savedOk(saved)) return false;
    *parts = unzipFile(e.path);
    return !parts->isEmpty();
}

struct LimitsGuard {
    ~LimitsGuard() { ole::resetLimits(); }
};

} // namespace

class CrunchObjectTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // ---- what Crunch offers on a copy (the other apps paste it as an object)

    void copyingCellsOffersARangeObject()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        e.select(QStringLiteral("A1"));
        QApplication::clipboard()->clear();
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        QTRY_VERIFY(objects::hasObject(QApplication::clipboard()->mimeData()));
        const auto made = objects::objectOf(QApplication::clipboard()->mimeData());
        QVERIFY(made);
        QCOMPARE(made->kind(), ole::Kind::Sheet);
        QCOMPARE(objects::kindWords(*made), QStringLiteral("Spreadsheet range"));
        QVERIFY(QImage::fromData(made->previewPng, "PNG").width() > 0);
    }

    void copyingAChartOffersAChartObject()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "B1=4", "B2=5", "B3=6"})));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        e.select(QStringLiteral("A1"));
        QVERIFY(e.win.w.insertChart(jp::office::sheet::ChartKind::Column));
        QApplication::clipboard()->clear();
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        QTRY_VERIFY(objects::hasObject(QApplication::clipboard()->mimeData()));
        const auto made = objects::objectOf(QApplication::clipboard()->mimeData());
        QVERIFY(made);
        QCOMPARE(objects::kindWords(*made), QStringLiteral("Chart"));
        QVERIFY(QImage::fromData(made->previewPng, "PNG").width() > 0);
    }

    // ---- an object of another program

    void aForeignObjectOpensAsAnObjectAndStaysByteForByteOnSave()
    {
        Spec spec;
        spec.png = drawtest::pngBytes(40, 30, Qt::red);
        spec.bytes = QByteArray("not ours, and not a package: kept exactly") + QByteArray(1, '\0') + QByteArray("\xff\x01", 2);
        const QByteArray book = bookWith({spec});
        const auto before = unzip(book);
        EditWindow e;
        QVERIFY(ready(e, book));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        QTRY_COMPARE(embeds->countOf(0), 1);
        const int id = embeds->idsOf(0).front();
        const auto object = embeds->objectOf(0, id);
        QVERIFY(object);
        QVERIFY(!object->isOurs);
        QCOMPARE(object->previewPng, spec.png);
        QVERIFY(embeds->canMove(0, id));   // (moved and sized like ours: only its place in the file changes)
        // (it is in the layer with its picture, as an object, not a picture of the drawing)
        QCOMPARE(e.win.w.objects()->layer()->count(), 1);
        QCOMPARE(int(e.win.w.objects()->describe(0).size()), 0);
        const DrawingAnchor placed = embeds->anchorOf(0, id);
        QVERIFY(sameCells(placed, cells(1, 1, 4, 6)));
        // A cell is typed and the workbook saved: the object's part, its picture, and its element are as they were.
        QMap<QString, QByteArray> after;
        QVERIFY(saveAndRead(e, &after));
        const QStringList embedded = partsUnder(after, QStringLiteral("xl/embeddings/"));
        QCOMPARE(embedded.size(), 1);
        QCOMPARE(after.value(embedded.first()), spec.bytes);
        QCOMPARE(after.value(QStringLiteral("xl/media/p1.png")), spec.png);
        QCOMPARE(after.value(QStringLiteral("xl/worksheets/sheet1.xml")).mid(after.value(QStringLiteral("xl/worksheets/sheet1.xml")).indexOf("<oleObjects")),
                 before.value(QStringLiteral("xl/worksheets/sheet1.xml")).mid(before.value(QStringLiteral("xl/worksheets/sheet1.xml")).indexOf("<oleObjects")));
        // Opened again, it is the same object in the same place.
        EditWindow again;
        QVERIFY(ready(again, [&] {
            QFile f(e.path);
            f.open(QIODevice::ReadOnly);
            return f.readAll();
        }()));
        QTRY_COMPARE(embedsOf(again)->countOf(0), 1);
        const int id2 = embedsOf(again)->idsOf(0).front();
        QCOMPARE(embedsOf(again)->objectOf(0, id2)->partBytes, spec.bytes);
        QVERIFY(sameCells(embedsOf(again)->anchorOf(0, id2), placed));
    }

    void aForeignObjectCanBeDeletedAndItsPartGoesWithIt()
    {
        Spec one, two;
        one.png = drawtest::pngBytes(40, 30, Qt::red);
        two.png = drawtest::pngBytes(40, 30, Qt::blue);
        two.bytes = "the second one";
        two.fromCol = 6;
        two.toCol = 9;
        EditWindow e;
        QVERIFY(ready(e, bookWith({one, two})));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        QTRY_COMPARE(embeds->countOf(0), 2);
        const int first = embeds->idsOf(0).front();
        const int steps = e.win.w.undoStack()->count();
        e.win.w.objects()->layer()->select(first);
        e.win.w.act(QStringLiteral("object.delete"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(embeds->countOf(0), 1);
        // (Undo brings it back, Redo takes it away again, and the save leaves its element, its part, and its picture out)
        e.win.w.undoStack()->undo();
        QCOMPARE(embeds->countOf(0), 2);
        e.win.w.undoStack()->redo();
        QCOMPARE(embeds->countOf(0), 1);
        QMap<QString, QByteArray> after;
        QVERIFY(saveAndRead(e, &after));
        const QStringList embedded = partsUnder(after, QStringLiteral("xl/embeddings/"));
        QCOMPARE(embedded.size(), 1);
        QCOMPARE(after.value(embedded.first()), two.bytes);
        QCOMPARE(partsUnder(after, QStringLiteral("xl/media/")).size(), 1);
        QCOMPARE(after.value(partsUnder(after, QStringLiteral("xl/media/")).first()), two.png);
        QCOMPARE(after.value(QStringLiteral("xl/worksheets/sheet1.xml")).count("<oleObject "), 1);
    }

    void aForeignObjectSaysPlainlyItCantBeEditedHere()
    {
        Spec spec;
        spec.png = drawtest::pngBytes(40, 30, Qt::red);
        EditWindow e;
        QVERIFY(ready(e, bookWith({spec})));
        QTRY_COMPARE(embedsOf(e)->countOf(0), 1);
        const int id = embedsOf(e)->idsOf(0).front();
        QVERIFY(!e.win.w.editObject(0, id));
        const QString said = e.win.w.statusText();
        QVERIFY2(said.contains(QStringLiteral("another program")), qPrintable(said));
        QVERIFY(!said.contains(QStringLiteral("Foo")));
        // (the double click is the same)
        Q_EMIT e.win.w.objects()->layer()->objectActivated(id);
        QVERIFY(e.win.w.statusText().contains(QStringLiteral("can't be edited here")));
    }


    // ---- what a person does with an object of another program: it is moved and follows the rows like ours, and a save keeps it whole

    void undoOfADeleteAfterASaveBringsAForeignObjectBackIntoTheFile()
    {
        Spec spec;
        spec.png = drawtest::pngBytes(40, 30, Qt::red);
        EditWindow e;
        QVERIFY(ready(e, bookWith({spec})));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        QTRY_COMPARE(embeds->countOf(0), 1);
        e.win.w.objects()->layer()->select(embeds->idsOf(0).front());
        e.win.w.act(QStringLiteral("object.delete"))->trigger();
        QMap<QString, QByteArray> gone;
        QVERIFY(saveAndRead(e, &gone));
        QVERIFY(partsUnder(gone, QStringLiteral("xl/embeddings/")).isEmpty());
        // (the cell typed for the save, and then the delete)
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        QCOMPARE(embeds->countOf(0), 1);
        QMap<QString, QByteArray> back;
        QVERIFY(saveAndRead(e, &back));
        const QStringList embedded = partsUnder(back, QStringLiteral("xl/embeddings/"));
        QCOMPARE(embedded.size(), 1);
        QCOMPARE(back.value(embedded.first()), spec.bytes);
        const QStringList pictures = partsUnder(back, QStringLiteral("xl/media/"));
        QCOMPARE(pictures.size(), 1);
        QCOMPARE(back.value(pictures.first()), spec.png);
        QCOMPARE(back.value(QStringLiteral("xl/worksheets/sheet1.xml")).count("<oleObject "), 2);   // (the element and its plain fallback)
        QCOMPARE(savedAnchors(back.value(QStringLiteral("xl/worksheets/sheet1.xml"))).size(), 1);
        // (and it is the same object when the file is opened again)
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        EditWindow again;
        QVERIFY(ready(again, f.readAll()));
        QTRY_COMPARE(embedsOf(again)->countOf(0), 1);
        QCOMPARE(embedsOf(again)->objectOf(0, embedsOf(again)->idsOf(0).front())->partBytes, spec.bytes);
    }

    void aPictureThatIsNotAPngIsKeptThroughAMoveAndASave()
    {
        Spec spec;
        spec.png = QByteArray("EMF-LOOKING-PICTURE-BYTES");
        spec.previewExt = QStringLiteral("emf");
        spec.previewType = QStringLiteral("image/x-emf");
        EditWindow e;
        QVERIFY(ready(e, bookWith({spec})));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        QTRY_COMPARE(embeds->countOf(0), 1);
        const int id = embeds->idsOf(0).front();
        QVERIFY(embeds->objectOf(0, id)->previewPng.isEmpty());   // (nothing to draw it from: a box that says what it is)
        QVERIFY(e.win.w.moveObject(id, cells(5, 8, 9, 12)));
        QMap<QString, QByteArray> after;
        QVERIFY(saveAndRead(e, &after));
        QCOMPARE(after.value(QStringLiteral("xl/media/p1.emf")), spec.png);
        QCOMPARE(partsUnder(after, QStringLiteral("xl/embeddings/")).size(), 1);
        QCOMPARE(after.value(partsUnder(after, QStringLiteral("xl/embeddings/")).first()), spec.bytes);
        QCOMPARE(savedAnchors(after.value(QStringLiteral("xl/worksheets/sheet1.xml"))), QList<QList<qint64>>{flat(cells(5, 8, 9, 12))});
        // (the same after Undo, Redo, and a delete that is undone after a save: the picture is the file's own each time)
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->redo();
        e.win.w.undoStack()->redo();
        e.win.w.objects()->layer()->select(id);
        e.win.w.act(QStringLiteral("object.delete"))->trigger();
        QMap<QString, QByteArray> gone;
        QVERIFY(saveAndRead(e, &gone));
        QVERIFY(partsUnder(gone, QStringLiteral("xl/embeddings/")).isEmpty());
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        QMap<QString, QByteArray> back;
        QVERIFY(saveAndRead(e, &back));
        QCOMPARE(partsUnder(back, QStringLiteral("xl/embeddings/")).size(), 1);
        const QStringList pictures = partsUnder(back, QStringLiteral("xl/media/"));
        QCOMPARE(pictures.size(), 1);
        QCOMPARE(back.value(pictures.first()), spec.png);
        QVERIFY(pictures.first().endsWith(QStringLiteral(".emf")));
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        EditWindow again;
        QVERIFY(ready(again, f.readAll()));
        QTRY_COMPARE(embedsOf(again)->countOf(0), 1);
    }

    void anObjectOfOursWithAPictureThatIsNotAPngMovesAndSaves()
    {
        QTemporaryDir dir;
        const QByteArray package = objects::rangePackage(gridOf({"Name", "Total"}, 2));
        Spec spec;
        spec.progId = ole::progIdOf(ole::Kind::Sheet);
        spec.bytes = package;
        spec.png = QByteArray("WMF-LOOKING-PICTURE-BYTES");
        spec.previewExt = QStringLiteral("wmf");
        spec.previewType = QStringLiteral("image/x-wmf");
        EditWindow e;
        QVERIFY(ready(e, bookWith({spec})));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        QTRY_COMPARE(embeds->countOf(0), 1);
        const int id = embeds->idsOf(0).front();
        QVERIFY(embeds->objectOf(0, id)->isOurs);
        QVERIFY(e.win.w.moveObject(id, cells(5, 8, 8, 13)));
        QMap<QString, QByteArray> after;
        QVERIFY(saveAndRead(e, &after));   // (a save that fails over the picture is a failed save)
        QCOMPARE(after.value(QStringLiteral("xl/media/p1.wmf")), spec.png);
        QCOMPARE(after.value(partsUnder(after, QStringLiteral("xl/embeddings/")).first()), package);
        QCOMPARE(savedAnchors(after.value(QStringLiteral("xl/worksheets/sheet1.xml"))), QList<QList<qint64>>{flat(cells(5, 8, 8, 13))});
    }

    void aForeignObjectMovesByDragAndByAlignAndTheFileHasWhereTheScreenShows()
    {
        Spec one, two, three;
        one.png = drawtest::pngBytes(40, 30, Qt::red);
        two.png = drawtest::pngBytes(40, 30, Qt::blue);
        three.png = drawtest::pngBytes(40, 30, Qt::green);
        two.bytes = "the second";
        three.bytes = "the third";
        two.fromCol = 6;
        two.toCol = 9;
        two.fromRow = 8;
        two.toRow = 12;
        three.fromCol = 11;
        three.toCol = 14;
        three.fromRow = 3;
        three.toRow = 7;
        EditWindow e;
        QVERIFY(ready(e, bookWith({one, two, three})));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        QTRY_COMPARE(embeds->countOf(0), 3);
        const std::vector<int> ids = embeds->idsOf(0);
        QVERIFY(embeds->canMove(0, ids[0]));
        // the layer's own move (what a drag and an arrow key give)
        e.win.w.objects()->layer()->select(ids[0]);
        const int steps = e.win.w.undoStack()->index();
        QVERIFY(e.win.w.objects()->layer()->nudge(24, 0, false, false));
        QCOMPARE(e.win.w.undoStack()->index(), steps + 1);
        QVERIFY(e.win.w.moveObject(ids[1], cells(6, 10, 9, 14)));
        // Align Left: the three are lined up on the screen
        e.win.w.pickObjects(ids);
        QVERIFY(e.win.w.alignObjects(crunch::ObjectAlign::Left));
        QMap<QString, QByteArray> after;
        QVERIFY(saveAndRead(e, &after));
        const QList<QList<qint64>> saved = savedAnchors(after.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        QCOMPARE(saved.size(), 3);
        for (size_t i = 0; i < ids.size(); ++i) QCOMPARE(saved[int(i)], flat(embeds->anchorOf(0, ids[i])));
        QCOMPARE(saved[0][0], saved[1][0]);   // (the same left edge, as the screen has them)
        QCOMPARE(saved[0][1], saved[1][1]);
        QCOMPARE(saved[0][0], saved[2][0]);
        // (the parts are the file's own, byte for byte)
        QCOMPARE(partsUnder(after, QStringLiteral("xl/embeddings/")).size(), 3);
        QCOMPARE(after.value(QStringLiteral("xl/embeddings/oleObject1.bin")), one.bytes);
        QCOMPARE(after.value(QStringLiteral("xl/embeddings/oleObject2.bin")), two.bytes);
        QCOMPARE(after.value(QStringLiteral("xl/embeddings/oleObject3.bin")), three.bytes);
        QCOMPARE(after.value(QStringLiteral("xl/media/p2.png")), two.png);
        // Undo puts each where it was
        for (int i = 0; i < 4; ++i) e.win.w.undoStack()->undo();
        QVERIFY(sameCells(embeds->anchorOf(0, ids[0]), cells(1, 1, 4, 6)));
        QVERIFY(sameCells(embeds->anchorOf(0, ids[1]), cells(6, 8, 9, 12)));
    }

    void aForeignObjectFollowsRowsInsertedAboveItAndTheFileSaysSo()
    {
        Spec spec;
        spec.png = drawtest::pngBytes(40, 30, Qt::red);
        spec.offsets = QStringLiteral("9525");
        EditWindow e;
        QVERIFY(ready(e, bookWith({spec})));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        QTRY_COMPARE(embeds->countOf(0), 1);
        const int id = embeds->idsOf(0).front();
        e.select(QStringLiteral("A1"));
        e.win.w.act(QStringLiteral("cells.insert.rows"))->trigger();
        QTRY_COMPARE(int(embeds->anchorOf(0, id).from.row), 2);
        QMap<QString, QByteArray> after;
        QVERIFY(saveAndRead(e, &after));
        const QList<QList<qint64>> saved = savedAnchors(after.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        QCOMPARE(saved, QList<QList<qint64>>{flat(embeds->anchorOf(0, id))});
        QCOMPARE(saved.first()[2], qint64(2));
        QCOMPARE(saved.first()[6], qint64(7));
        QCOMPARE(saved.first()[3], qint64(9525));   // (an offset inside the cell is the file's)
        QCOMPARE(after.value(QStringLiteral("xl/embeddings/oleObject1.bin")), spec.bytes);
        QCOMPARE(after.value(QStringLiteral("xl/media/p1.png")), spec.png);
        // (the rows come out again: the file's anchor goes back)
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        QTRY_COMPARE(int(embeds->anchorOf(0, id).from.row), 1);
        QMap<QString, QByteArray> back;
        QVERIFY(saveAndRead(e, &back));
        QCOMPARE(savedAnchors(back.value(QStringLiteral("xl/worksheets/sheet1.xml"))).first()[2], qint64(1));
    }

    void anObjectWhoseRowsAreDeletedStaysOutOfTheFileOnceSaved()
    {
        Spec spec;
        spec.png = drawtest::pngBytes(40, 30, Qt::red);
        spec.fromRow = 2;
        spec.toRow = 3;
        EditWindow e;
        QVERIFY(ready(e, bookWith({spec})));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        QTRY_COMPARE(embeds->countOf(0), 1);
        e.win.w.view()->selectRows(1, 4);
        e.win.w.act(QStringLiteral("cells.delete.rows"))->trigger();
        QTRY_COMPARE(embeds->countOf(0), 0);
        QMap<QString, QByteArray> after;
        QVERIFY(saveAndRead(e, &after));
        QVERIFY(partsUnder(after, QStringLiteral("xl/embeddings/")).isEmpty());
        QCOMPARE(embeds->countOf(0), 0);   // (the window and the file agree)
    }

    void whatSavingReadsBackIsMappedByWhatItNamesNotByPlace()
    {
        // The limits are small enough that the first sheet's big object is left out because the second sheet's objects were counted before it (the window asked for the first sheet first
        // here, but the budget is the workbook's): a save that reads the sheet back with its own budget finds one more object than the window has, and must not mix them up.
        LimitsGuard guard;
        Spec x, big, small;
        x.png = drawtest::pngBytes(40, 30, Qt::red);
        x.bytes = QByteArray(3000, 'x');
        big.png = drawtest::pngBytes(40, 30, Qt::blue);
        big.bytes = QByteArray(800, 'b');
        small.png = drawtest::pngBytes(40, 30, Qt::green);
        small.bytes = QByteArray(100, 's');
        small.fromCol = 6;
        small.toCol = 9;
        ole::Limits l = ole::limits();
        l.totalBytes = qint64(x.bytes.size()) + x.png.size() + small.bytes.size() + small.png.size() + 50;
        ole::setLimits(l);
        EditWindow e;
        QVERIFY(ready(e, bookWith({x}, {big, small})));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        QTRY_COMPARE(embeds->countOf(0), 1);
        QCOMPARE(embeds->countOf(1), 1);   // (the small one: the big one did not fit)
        const int id = embeds->idsOf(1).front();
        QCOMPARE(embeds->objectOf(1, id)->partBytes, small.bytes);
        // moved and saved twice: the second move is of the small one, not of the big one the file also has
        QVERIFY(showSheet(e, 1));
        QVERIFY(e.win.w.moveObject(id, cells(2, 12, 5, 16)));
        QMap<QString, QByteArray> first;
        QVERIFY(saveAndRead(e, &first));
        QVERIFY(e.win.w.moveObject(id, cells(3, 14, 6, 18)));
        QMap<QString, QByteArray> second;
        QVERIFY(saveAndRead(e, &second));
        const QList<QList<qint64>> saved = savedAnchors(second.value(QStringLiteral("xl/worksheets/sheet2.xml")));
        QCOMPARE(saved.size(), 2);
        QCOMPARE(saved[0], flat(cells(1, 1, 4, 6)));   // (the big one is where the file had it)
        QCOMPARE(saved[1], flat(cells(3, 14, 6, 18)));
        QCOMPARE(partsUnder(second, QStringLiteral("xl/embeddings/")).size(), 3);
    }

    void aCopiedSheetHasItsOwnObjectsAndTheFileHasThemToo()
    {
        QTemporaryDir dir;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12"}, 2)));
        Spec foreign;
        foreign.png = drawtest::pngBytes(40, 30, Qt::red);
        foreign.fromCol = 8;
        foreign.toCol = 11;
        EditWindow e;
        QVERIFY(ready(e, bookWith({foreign})));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B2")));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        QCOMPARE(embeds->countOf(0), 2);
        const crunch::EditSession::Outcome copied = e.win.w.session()->copySheet(0, -1, QStringLiteral("Copy"));
        QVERIFY(copied.ok);
        const int copy = e.win.w.loaded()->book->info().sheets.size() - 1;
        QCOMPARE(embeds->countOf(copy), 2);
        // (objects of its own: editing the copy's does not touch the original's)
        QVERIFY(showSheet(e, copy));
        const std::vector<int> ids = embeds->idsOf(copy);
        int ours = -1;
        for (const int id : ids)
            if (embeds->objectOf(copy, id)->isOurs) ours = id;
        QVERIFY(ours >= 0);
        QVERIFY(e.win.w.moveObject(ours, cells(12, 20, 16, 24)));
        QMap<QString, QByteArray> after;
        QVERIFY(saveAndRead(e, &after));
        QCOMPARE(after.value(QStringLiteral("xl/worksheets/sheet1.xml")).count("<xdr:from>"), 2);
        QCOMPARE(after.value(QStringLiteral("xl/worksheets/sheet2.xml")).count("<xdr:from>"), 2);
        QCOMPARE(partsUnder(after, QStringLiteral("xl/embeddings/")).size(), 4);
        const QList<QList<qint64>> original = savedAnchors(after.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        const QList<QList<qint64>> copiedOnes = savedAnchors(after.value(QStringLiteral("xl/worksheets/sheet2.xml")));
        QVERIFY(copiedOnes.contains(flat(cells(12, 20, 16, 24))));
        QVERIFY(!original.contains(flat(cells(12, 20, 16, 24))));
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        EditWindow again;
        QVERIFY(ready(again, f.readAll()));
        QTRY_COMPARE(embedsOf(again)->countOf(0), 2);
        QCOMPARE(embedsOf(again)->countOf(1), 2);
    }

    // ---- an object of ours

    void aRangeFromAFileIsPutInAsAnObjectNotAPicture()
    {
        QTemporaryDir dir;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12"}, 2)));
        EditWindow e;
        QVERIFY(ready(e));
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B2")));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        crunch::SheetEmbeds *embeds = embedsOf(e);
        QCOMPARE(embeds->countOf(0), 1);
        const int id = embeds->idsOf(0).front();
        const auto object = embeds->objectOf(0, id);
        QVERIFY(object && object->isOurs);
        QCOMPARE(object->kind(), ole::Kind::Sheet);
        QVERIFY(QImage::fromData(object->previewPng, "PNG").width() > 0);
        QCOMPARE(e.win.w.objects()->layer()->count(), 1);
        QCOMPARE(int(e.win.w.objects()->describe(0).size()), 0);   // (not a picture of the drawing)
        QVERIFY(embeds->canMove(0, id));
        // A page past the end is said, and nothing is put in; the one step undoes.
        const QString page = savedFile(dir, QStringLiteral("page.docx"), docxOf({QStringLiteral("First page words")}));
        QVERIFY(!e.win.w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("9")));
        QVERIFY(e.win.w.statusText().contains(QStringLiteral("page")));
        QCOMPARE(embeds->countOf(0), 1);
        e.win.w.undoStack()->undo();
        QCOMPARE(embeds->countOf(0), 0);
        e.win.w.undoStack()->redo();
        QCOMPARE(embeds->countOf(0), 1);
    }

    void aPageFromAFileIsAnObject()
    {
        QTemporaryDir dir;
        const QString page = savedFile(dir, QStringLiteral("page.docx"), docxOf({QStringLiteral("First page words")}));
        EditWindow e;
        QVERIFY(ready(e));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        const auto object = embedsOf(e)->objectOf(0, embedsOf(e)->idsOf(0).front());
        QVERIFY(object && object->isOurs);
        QCOMPARE(objects::kindWords(*object), QStringLiteral("Document page"));
    }

    void anObjectMovesAndSizesAndEachIsOneStepOfUndo()
    {
        QTemporaryDir dir;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12"}, 2)));
        EditWindow e;
        QVERIFY(ready(e));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B2")));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        const int id = embeds->idsOf(0).front();
        const DrawingAnchor first = embeds->anchorOf(0, id);
        const DrawingAnchor moved = cells(5, 8, 9, 12), sized = cells(5, 8, 12, 20);
        int steps = e.win.w.undoStack()->count();
        QVERIFY(e.win.w.moveObject(id, moved));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QVERIFY(sameCells(embeds->anchorOf(0, id), moved));
        QVERIFY(e.win.w.moveObject(id, sized));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 2);
        QVERIFY(sameCells(embeds->anchorOf(0, id), sized));
        e.win.w.undoStack()->undo();
        QVERIFY(sameCells(embeds->anchorOf(0, id), moved));
        e.win.w.undoStack()->undo();
        QVERIFY(sameCells(embeds->anchorOf(0, id), first));
        e.win.w.undoStack()->redo();
        QVERIFY(sameCells(embeds->anchorOf(0, id), moved));
        // Delete is a step too, and Undo brings the object back where it was.
        steps = e.win.w.undoStack()->index();
        e.win.w.objects()->layer()->select(id);
        e.win.w.act(QStringLiteral("object.delete"))->trigger();
        QCOMPARE(e.win.w.undoStack()->index(), steps + 1);
        QCOMPARE(embeds->countOf(0), 0);
        e.win.w.undoStack()->undo();
        QCOMPARE(embeds->countOf(0), 1);
        QVERIFY(sameCells(embeds->anchorOf(0, id), moved));
        // The layer's own mouse and key moves reach the same step.
        e.win.w.objects()->layer()->select(id);
        steps = e.win.w.undoStack()->index();
        QVERIFY(e.win.w.objects()->layer()->nudge(24, 0, false, false));
        QCOMPARE(e.win.w.undoStack()->index(), steps + 1);
    }

    void rowsInsertedAboveMoveAnObjectOfOurs()
    {
        QTemporaryDir dir;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total"}, 2)));
        EditWindow e;
        QVERIFY(ready(e));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B1")));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        const int id = embeds->idsOf(0).front();
        QVERIFY(e.win.w.moveObject(id, cells(2, 10, 5, 14)));
        e.select(QStringLiteral("A3"));
        e.win.w.act(QStringLiteral("cells.insert.rows"))->trigger();
        QTRY_COMPARE(int(embeds->anchorOf(0, id).from.row), 11);
        e.win.w.undoStack()->undo();
        QTRY_COMPARE(int(embeds->anchorOf(0, id).from.row), 10);
    }

    void aCopyIsAnObjectOfItsOwnWithItsOwnPart()
    {
        QTemporaryDir dir;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12"}, 2)));
        EditWindow e;
        QVERIFY(ready(e));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B2")));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        const int first = embeds->idsOf(0).front();
        e.win.w.objects()->layer()->select(first);
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        // (other programs get the object from the same copy)
        QTRY_VERIFY(objects::hasObject(QApplication::clipboard()->mimeData()));
        const int steps = e.win.w.undoStack()->count();
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(embeds->countOf(0), 2);
        const std::vector<int> ids = embeds->idsOf(0);
        const int second = ids[0] == first ? ids[1] : ids[0];
        QVERIFY(second != first);
        const auto a = embeds->objectOf(0, first), b = embeds->objectOf(0, second);
        QVERIFY(a && b);
        QCOMPARE(b->partBytes, a->partBytes);
        QVERIFY(a.get() != b.get());   // (its own object: a change to one is no change to the other)
        // Undo takes the pasted one away alone, and Redo puts it back.
        e.win.w.undoStack()->undo();
        QCOMPARE(embeds->countOf(0), 1);
        e.win.w.undoStack()->redo();
        QCOMPARE(embeds->countOf(0), 2);
        // Saved, each has a part of its own.
        QMap<QString, QByteArray> after;
        QVERIFY(saveAndRead(e, &after));
        QCOMPARE(partsUnder(after, QStringLiteral("xl/embeddings/")).size(), 2);
    }

    void pasteSpecialAsObjectPutsAnObjectIn()
    {
        QTemporaryDir dir;
        const QString page = savedFile(dir, QStringLiteral("page.docx"), docxOf({QStringLiteral("Words")}));
        const auto object = objects::pageObjectFromFile(page, 1);
        QVERIFY(object);
        EditWindow e;
        QVERIFY(ready(e));
        auto *mime = new QMimeData;
        mime->setText(QStringLiteral("plain words"));
        QApplication::clipboard()->setMimeData(objects::withObject(mime, [object] { return object; }));
        QTRY_VERIFY(e.win.w.act(QStringLiteral("edit.pasteObject"))->isEnabled());
        e.win.w.act(QStringLiteral("edit.pasteObject"))->trigger();
        QCOMPARE(embedsOf(e)->countOf(0), 1);
        QCOMPARE(embedsOf(e)->objectOf(0, embedsOf(e)->idsOf(0).front())->partBytes, object->partBytes);
    }

    void anObjectSurvivesSaveToXlsxAndOpensAgainAsAnObject()
    {
        QTemporaryDir dir;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12"}, 2)));
        EditWindow e;
        QVERIFY(ready(e));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B2")));
        const int id = embedsOf(e)->idsOf(0).front();
        const auto made = embedsOf(e)->objectOf(0, id);
        QVERIFY(e.win.w.moveObject(id, cells(3, 4, 7, 9)));
        QMap<QString, QByteArray> after;
        QVERIFY(saveAndRead(e, &after));
        const QStringList embedded = partsUnder(after, QStringLiteral("xl/embeddings/"));
        QCOMPARE(embedded.size(), 1);
        QCOMPARE(after.value(embedded.first()), made->partBytes);   // (an ordinary package of the standard form, readable by other programs)
        QVERIFY(after.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("<oleObject "));
        // (a second save writes the same object again, and a change after it is one more)
        QVERIFY(e.win.w.moveObject(id, cells(3, 5, 7, 10)));
        QMap<QString, QByteArray> second;
        QVERIFY(saveAndRead(e, &second));
        QCOMPARE(partsUnder(second, QStringLiteral("xl/embeddings/")).size(), 1);
        EditWindow again;
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QVERIFY(ready(again, f.readAll()));
        QTRY_COMPARE(embedsOf(again)->countOf(0), 1);
        const int id2 = embedsOf(again)->idsOf(0).front();
        const auto read = embedsOf(again)->objectOf(0, id2);
        QVERIFY(read->isOurs);
        QCOMPARE(read->kind(), ole::Kind::Sheet);
        QCOMPARE(read->partBytes, made->partBytes);
        QCOMPARE(read->previewPng, embedsOf(e)->objectOf(0, id)->previewPng);   // (the picture it was drawn again with when it was sized)
        QVERIFY(sameCells(embedsOf(again)->anchorOf(0, id2), cells(3, 5, 7, 10)));
    }

    void anObjectIsWrittenToAnOdsAsAnObjectAndKeptWhenThatIsSavedAgain()
    {
        QTemporaryDir dir;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12"}, 2)));
        EditWindow e;
        QVERIFY(ready(e));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B2")));
        const auto made = embedsOf(e)->objectOf(0, embedsOf(e)->idsOf(0).front());
        const QString ods = dir.filePath(QStringLiteral("copy.ods"));
        QSignalSpy exported(&e.win.w, &CrunchWindow::copyExported);
        QVERIFY(e.win.w.exportCopy(ods));
        QVERIFY(exported.wait(60000) && exported.last().value(1).toBool());
        const auto parts = unzipFile(ods);
        QVERIFY(parts.value(QStringLiteral("content.xml")).contains("draw:object"));
        QCOMPARE(parts.value(QStringLiteral("Object 1")), made->partBytes);
        QVERIFY(!partsUnder(parts, QStringLiteral("Pictures/")).isEmpty());
        // Opened again it is kept as it is saved again.
        EditWindow again;
        QFile f(ods);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QVERIFY(again.open(f.readAll(), QStringLiteral("again.ods")));
        QVERIFY(again.settle());
        const QString second = dir.filePath(QStringLiteral("second.ods"));
        QSignalSpy exported2(&again.win.w, &CrunchWindow::copyExported);
        QVERIFY(again.win.w.exportCopy(second));
        QVERIFY(exported2.wait(60000) && exported2.last().value(1).toBool());
        QCOMPARE(unzipFile(second).value(QStringLiteral("Object 1")), made->partBytes);
    }


    // ---- what the window does with them

    void aResizedObjectOfOursIsDrawnAgainAtItsNewSizeAndAForeignOneKeepsItsOwnPicture()
    {
        QTemporaryDir dir;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12"}, 2)));
        Spec foreign;
        foreign.png = drawtest::pngBytes(40, 30, Qt::red);
        foreign.fromCol = 12;
        foreign.toCol = 15;
        EditWindow e;
        QVERIFY(ready(e, bookWith({foreign})));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B2")));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        int mine = -1, theirs = -1;
        for (const int id : embeds->idsOf(0)) (embeds->objectOf(0, id)->isOurs ? mine : theirs) = id;
        QVERIFY(mine >= 0 && theirs >= 0);
        const auto was = embeds->objectOf(0, mine);
        const QImage before = crunch::SheetEmbeds::previewImage(was->previewPng);
        // sized to a shape that is not the one it had: its picture is made again at that shape (it is not the old picture stretched)
        const DrawingAnchor tall = cells(5, 8, 7, 22);
        QVERIFY(e.win.w.moveObject(mine, tall));
        const auto now = embeds->objectOf(0, mine);
        QVERIFY(now->previewPng != was->previewPng);
        bool ok = false;
        const grid::ObjectBox box = e.win.w.objects()->layer()->boxNow(mine, &ok);
        QVERIFY(ok);
        const QImage after = crunch::SheetEmbeds::previewImage(now->previewPng);
        QVERIFY(!after.isNull());
        QVERIFY(std::abs(double(after.width()) / after.height() - box.w / box.h) < 0.06 * (box.w / box.h));
        QVERIFY(std::abs(double(before.width()) / before.height() - box.w / box.h) > 0.3 * (box.w / box.h));
        QCOMPARE(embeds->objectOf(0, mine)->partBytes, was->partBytes);
        // (one step: Undo gives back the picture it had, and the file has the picture it is shown with)
        e.win.w.undoStack()->undo();
        QCOMPARE(embeds->objectOf(0, mine)->previewPng, was->previewPng);
        e.win.w.undoStack()->redo();
        QMap<QString, QByteArray> saved;
        QVERIFY(saveAndRead(e, &saved));
        bool found = false;
        for (const QString &name : partsUnder(saved, QStringLiteral("xl/media/"))) found = found || saved.value(name) == embeds->objectOf(0, mine)->previewPng;
        QVERIFY(found);
        // another program's object has only its own picture, and keeps it at any size
        QVERIFY(e.win.w.moveObject(theirs, cells(12, 30, 20, 36)));
        QCOMPARE(embeds->objectOf(0, theirs)->previewPng, foreign.png);
    }

    void anObjectIsPutInAtTheSelectedCell()
    {
        QTemporaryDir dir;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12"}, 2)));
        EditWindow e;
        QVERIFY(ready(e));
        e.select(QStringLiteral("E10"));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B2")));
        const DrawingAnchor a = embedsOf(e)->anchorOf(0, embedsOf(e)->idsOf(0).front());
        QCOMPARE(int(a.from.col), 4);
        QCOMPARE(int(a.from.row), 9);
        e.select(QStringLiteral("H3"));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B2")));
        const std::vector<int> ids = embedsOf(e)->idsOf(0);
        QCOMPARE(int(embedsOf(e)->anchorOf(0, ids.back()).from.col), 7);
        QCOMPARE(int(embedsOf(e)->anchorOf(0, ids.back()).from.row), 2);
    }

    void groupAndTheOrderAreOffForAnObjectFromAnotherApp()
    {
        QTemporaryDir dir;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total"}, 2)));
        EditWindow e;
        QVERIFY(ready(e));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B1")));
        e.select(QStringLiteral("H12"));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B1")));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        const std::vector<int> ids = embeds->idsOf(0);
        QCOMPARE(int(ids.size()), 2);
        e.win.w.objects()->layer()->select(ids[0]);
        QTRY_VERIFY(e.win.w.act(QStringLiteral("object.delete"))->isEnabled());
        for (const char *name : {"object.front", "object.forward", "object.backward", "object.back"}) QVERIFY2(!e.win.w.act(QString::fromLatin1(name))->isEnabled(), name);
        e.win.w.pickObjects(ids);
        QVERIFY(e.win.w.pickedObjects().size() == 2);
        QTRY_VERIFY(!e.win.w.act(QStringLiteral("object.group"))->isEnabled());
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(!e.win.w.groupObjects());   // (it says so, and changes nothing)
        QCOMPARE(e.win.w.undoStack()->count(), steps);
    }

    void thePictureIsUnpackedOnceAndTheJobThatDrawsHoldsNothingOfTheObject()
    {
        Spec spec;
        spec.png = drawtest::pngBytes(40, 30, Qt::red);
        EditWindow e;
        QVERIFY(ready(e, bookWith({spec})));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        QTRY_COMPARE(embeds->countOf(0), 1);
        const int id = embeds->idsOf(0).front();
        const auto lo = embeds->layerObjectOf(0, id);
        QVERIFY(lo && lo->prepare);
        grid::RenderRequest q;
        q.pixels = QSize(80, 60);
        q.units = QSizeF(40, 30);
        q.inner = QRectF(0, 0, 80, 60);
        const qint64 reads = crunch::SheetEmbeds::previewReads();
        std::function<QImage()> job = lo->prepare(q);
        std::function<QImage()> again = lo->prepare(q);
        std::function<QImage()> third = lo->prepare(q);
        QVERIFY(job && again && third);
        QCOMPARE(crunch::SheetEmbeds::previewReads() - reads, qint64(1));   // (once, however many pictures are drawn of it)
        // the object is taken away and the workbook changed while the job has not run (print and PDF run it on another thread): the job does not look at the object
        e.win.w.objects()->layer()->select(id);
        e.win.w.act(QStringLiteral("object.delete"))->trigger();
        QCOMPARE(embeds->countOf(0), 0);
        const QImage drawn = job();
        QVERIFY(!drawn.isNull());
        QCOMPARE(drawn.size(), q.pixels);
        QVERIFY(drawn.pixelColor(40, 30).alpha() > 0);
    }

    void anObjectDeletedWhileItIsOpenInItsOwnAppSaysSoWhenTheChangeComes()
    {
        QTemporaryDir dir, temp;
        const QString page = savedFile(dir, QStringLiteral("page.docx"), docxOf({QStringLiteral("Words")}));
        EditWindow e;
        QVERIFY(ready(e));
        jp::EmbeddedEdit *ed = e.win.w.objectEditor();
        ed->setTempRoot(temp.path());
        ed->setPollInterval(600000);
        QStringList opened;
        ed->setOpener([&](const QString &path, ole::Kind) {
            opened << path;
            return true;
        });
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        const int id = embeds->idsOf(0).front();
        Q_EMIT e.win.w.objects()->layer()->objectActivated(id);
        QTRY_COMPARE(opened.size(), 1);
        e.win.w.objects()->layer()->select(id);
        e.win.w.act(QStringLiteral("object.delete"))->trigger();
        QCOMPARE(embeds->countOf(0), 0);
        QSignalSpy saves(ed, &jp::EmbeddedEdit::objectSaved);
        {
            QFile f(opened.first());
            QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
            f.write(docxOf({QStringLiteral("Words"), QStringLiteral("and more")}));
        }
        const int steps = e.win.w.undoStack()->count();
        QTRY_VERIFY((ed->pollNow(), saves.count() >= 1));
        QTRY_COMPARE(e.win.w.statusText(), QStringLiteral("The object was deleted, so the change wasn't kept."));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(embeds->countOf(0), 0);
    }

    void aSheetOverTheScanSizeKeepsItsObjectsUntouchedAndTakesNoMore()
    {
        struct Guard {
            ~Guard() { crunch::SheetEmbeds::setScanLimit(0); }
        } guard;
        QTemporaryDir dir;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total"}, 2)));
        Spec spec;
        spec.png = drawtest::pngBytes(40, 30, Qt::red);
        const QByteArray xlsx = bookWith({spec});
        const auto was = unzip(xlsx);
        const qint64 size = was.value(QStringLiteral("xl/worksheets/sheet1.xml")).size();
        crunch::SheetEmbeds::setScanLimit(size - 1);
        EditWindow e;
        QVERIFY(ready(e, xlsx));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        QCOMPARE(embeds->countOf(0), 0);
        QVERIFY(!embeds->holdsObjectsOf(0));
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(!e.win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B2")));
        QVERIFY(e.win.w.statusText().contains(QStringLiteral("too large")) || e.win.w.undoStack()->count() == steps);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QMap<QString, QByteArray> after;
        QVERIFY(saveAndRead(e, &after));
        const QByteArray was1 = was.value(QStringLiteral("xl/worksheets/sheet1.xml")), now1 = after.value(QStringLiteral("xl/worksheets/sheet1.xml"));
        QCOMPARE(now1.mid(now1.indexOf("<oleObjects")), was1.mid(was1.indexOf("<oleObjects")));
        QCOMPARE(after.value(QStringLiteral("xl/embeddings/oleObject1.bin")), spec.bytes);
    }

    void everySheetIsReadOnceWhenTheWorkbookIsOpenedAndNothingLaterReadsAgain()
    {
        Spec a, b;
        a.png = drawtest::pngBytes(40, 30, Qt::red);
        b.png = drawtest::pngBytes(40, 30, Qt::blue);
        b.bytes = "the other sheet's";
        EditWindow e;
        QVERIFY(ready(e, bookWith({a}, {b})));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        const qint64 reads = office::sheet::sheetTailReads();
        QCOMPARE(embeds->liveCount(), 2);   // (both sheets' objects are held: the second sheet was not asked for)
        QCOMPARE(embeds->countOf(1), 1);
        QCOMPARE(embeds->liveCount(), 2);
        QCOMPARE(office::sheet::sheetTailReads() - reads, qint64(0));
    }

    void anUndonePasteIsLetGoOfWhenTheStepsAreGone()
    {
        QTemporaryDir dir;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total"}, 2)));
        EditWindow e;
        QVERIFY(ready(e));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B1")));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        const int id = embeds->idsOf(0).front();
        std::weak_ptr<const ole::EmbeddedObject> held = embeds->objectOf(0, id);
        QVERIFY(!held.expired());
        e.win.w.undoStack()->undo();
        QCOMPARE(embeds->countOf(0), 0);
        QVERIFY(!held.expired());   // (a step that does it again has it)
        e.win.w.undoStack()->clear();
        QVERIFY(held.expired());
    }


    // ---- saving an object to an .ods whose picture is not a PNG

    void anOdsKeepsAnObjectWhosePictureIsNotAPngAsAPngAndOneWithNoPictureWithoutOne()
    {
        QTemporaryDir dir;
        QByteArray bmp;
        {
            QImage image(30, 20, QImage::Format_RGB32);
            image.fill(Qt::magenta);
            QBuffer buffer(&bmp);
            buffer.open(QIODevice::WriteOnly);
            QVERIFY(image.save(&buffer, "BMP"));
        }
        Spec drawable, plain;
        drawable.png = bmp;
        drawable.previewExt = QStringLiteral("bmp");
        drawable.previewType = QStringLiteral("image/bmp");
        drawable.bytes = "the one with a picture Qt reads";
        plain.png = QByteArray("EMF-LOOKING-BYTES-NO-ONE-CAN-DRAW");
        plain.previewExt = QStringLiteral("emf");
        plain.previewType = QStringLiteral("image/x-emf");
        plain.bytes = "the one whose picture can't be drawn";
        plain.fromCol = 8;
        plain.toCol = 11;
        EditWindow e;
        QVERIFY(ready(e, bookWith({drawable, plain})));
        QTRY_COMPARE(embedsOf(e)->countOf(0), 2);
        const QString ods = dir.filePath(QStringLiteral("copy.ods"));
        QSignalSpy exported(&e.win.w, &CrunchWindow::copyExported);
        QVERIFY(e.win.w.exportCopy(ods));
        QVERIFY(exported.wait(60000) && exported.last().value(1).toBool());
        const auto parts = unzipFile(ods);
        const QByteArray content = parts.value(QStringLiteral("content.xml"));
        QCOMPARE(content.count("<draw:object-ole"), 2);   // (neither is dropped)
        QCOMPARE(content.count("<draw:image"), 1);        // (the second has no picture to be given)
        QStringList kept;
        for (auto it = parts.begin(); it != parts.end(); ++it)
            if (it.key().startsWith(QStringLiteral("Object ")) && !it.key().contains(QLatin1Char('/'))) kept << QString::fromLatin1(it.value());
        kept.sort();
        QStringList wanted{QString::fromLatin1(drawable.bytes), QString::fromLatin1(plain.bytes)};
        wanted.sort();
        QCOMPARE(kept, wanted);
        const QStringList pictures = partsUnder(parts, QStringLiteral("Pictures/"));
        QCOMPARE(pictures.size(), 1);
        QVERIFY(pictures.first().endsWith(QStringLiteral(".png")));
        QVERIFY(parts.value(pictures.first()).startsWith("\x89PNG"));
    }


    // ---- an .ods's objects are objects as an .xlsx's are

    void anOdsObjectOfAnotherProgramOpensAsAnObjectAndIsKeptWhenThatIsSavedAgain()
    {
        QTemporaryDir dir;
        const QString range = savedFile(dir, QStringLiteral("range.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12"}, 2)));
        const QMap<QString, QByteArray> original = unzip(odsWithForeignObjects());
        const QByteArray blob = original.value(QStringLiteral("Object 1")), folder = original.value(QStringLiteral("Object 2/content.xml"));
        EditWindow e;
        QVERIFY(readyAs(e, odsWithForeignObjects(), QStringLiteral("objects.ods")));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        QTRY_COMPARE(embeds->countOf(0), 2);
        for (const int id : embeds->idsOf(0)) {
            QVERIFY(!embeds->objectOf(0, id)->isOurs);
            QVERIFY(embeds->canMove(0, id));
            QVERIFY(!embeds->objectOf(0, id)->previewPng.isEmpty());   // (the replacement image the file gave it)
        }
        // they are in the layer as objects, not as pictures of the drawing
        QCOMPARE(e.win.w.objects()->layer()->count(), 2);
        QCOMPARE(int(e.win.w.objects()->describe(0).size()), 0);
        // one of ours is put in beside them, and the .ods is written
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Range, range, QStringLiteral("A1:B2")));
        QCOMPARE(embeds->countOf(0), 3);
        const auto ours = embeds->objectOf(0, embeds->idsOf(0).back());
        const QString first = dir.filePath(QStringLiteral("first.ods"));
        {
            QSignalSpy exported(&e.win.w, &CrunchWindow::copyExported);
            QVERIFY(e.win.w.exportCopy(first));
            QVERIFY(exported.wait(60000) && exported.last().value(1).toBool());
        }
        const auto firstParts = unzipFile(first);
        QVERIFY(odsObjectBytes(firstParts).contains(blob));
        QVERIFY(odsObjectBytes(firstParts).contains(folder));
        QVERIFY(odsObjectBytes(firstParts).contains(ours->partBytes));
        // opened again: all three are objects, one of ours, and none is a picture
        QFile f(first);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray firstBytes = f.readAll();
        f.close();
        EditWindow again;
        QVERIFY(readyAs(again, firstBytes, QStringLiteral("again.ods")));
        crunch::SheetEmbeds *embedsAgain = embedsOf(again);
        QTRY_COMPARE(embedsAgain->countOf(0), 3);
        int mine = 0;
        for (const int id : embedsAgain->idsOf(0)) mine += embedsAgain->objectOf(0, id)->isOurs ? 1 : 0;
        QCOMPARE(mine, 1);
        QCOMPARE(again.win.w.objects()->layer()->count(), 3);
        QCOMPARE(int(again.win.w.objects()->describe(0).size()), 0);
        // one is taken away (the others are not touched) and the .ods is written again: what it holds of the others is as it was
        int theBinary = -1;
        for (const int id : embedsAgain->idsOf(0))
            if (embedsAgain->objectOf(0, id)->partBytes == blob) theBinary = id;
        QVERIFY(theBinary >= 0);
        again.win.w.objects()->layer()->select(theBinary);
        again.win.w.act(QStringLiteral("object.delete"))->trigger();
        QCOMPARE(embedsAgain->countOf(0), 2);
        const QString second = dir.filePath(QStringLiteral("second.ods"));
        {
            QSignalSpy exported(&again.win.w, &CrunchWindow::copyExported);
            QVERIFY(again.win.w.exportCopy(second));
            QVERIFY(exported.wait(60000) && exported.last().value(1).toBool());
        }
        const QByteArrayList left = odsObjectBytes(unzipFile(second));
        QVERIFY(!left.contains(blob));
        QVERIFY(left.contains(folder));
        QVERIFY(left.contains(ours->partBytes));
        QCOMPARE(left.size(), 2);
    }

    // ---- the edit in the app that made it

    void aDoubleClickStartsTheEditAndASaveIsOneStep()
    {
        QTemporaryDir dir, temp;
        const QString page = savedFile(dir, QStringLiteral("page.docx"), docxOf({QStringLiteral("Words")}));
        EditWindow e;
        QVERIFY(ready(e));
        jp::EmbeddedEdit *ed = e.win.w.objectEditor();
        ed->setTempRoot(temp.path());
        ed->setPollInterval(600000);
        QStringList opened;
        ed->setOpener([&](const QString &path, ole::Kind) {
            opened << path;
            return true;
        });
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        const int id = embeds->idsOf(0).front();
        const auto old = embeds->objectOf(0, id);
        // The double click on the object (the layer's activation, which a double click gives).
        Q_EMIT e.win.w.objects()->layer()->objectActivated(id);
        QTRY_COMPARE(opened.size(), 1);
        QSignalSpy saves(ed, &jp::EmbeddedEdit::objectSaved);
        const QByteArray changed = docxOf({QStringLiteral("Words"), QStringLiteral("and more words after them")});
        {
            QFile f(opened.first());
            QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
            f.write(changed);
        }
        const int steps = e.win.w.undoStack()->count();
        QTRY_VERIFY((ed->pollNow(), saves.count() >= 1));
        QTRY_COMPARE(e.win.w.undoStack()->count(), steps + 1);
        const auto now = embeds->objectOf(0, id);
        QCOMPARE(now->partBytes, changed);
        QVERIFY(now->previewPng != old->previewPng);
        QCOMPARE(embeds->countOf(0), 1);
        // One step undoes it.
        e.win.w.undoStack()->undo();
        QCOMPARE(embeds->objectOf(0, id)->partBytes, old->partBytes);
        e.win.w.undoStack()->redo();
        QCOMPARE(embeds->objectOf(0, id)->partBytes, changed);
        // (saved, it is the changed package that is in the file)
        QMap<QString, QByteArray> after;
        QVERIFY(saveAndRead(e, &after));
        const QStringList embedded = partsUnder(after, QStringLiteral("xl/embeddings/"));
        QCOMPARE(embedded.size(), 1);
        QCOMPARE(after.value(embedded.first()), changed);
    }

    void aSaveFromTheOtherAppLeavesTheCellEditGoingAndAPastedCopyEditsItself()
    {
        QTemporaryDir dir, temp;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12"}, 2)));
        EditWindow e;
        QVERIFY(ready(e));
        jp::EmbeddedEdit *ed = e.win.w.objectEditor();
        ed->setTempRoot(temp.path());
        ed->setPollInterval(600000);
        QStringList opened;
        ed->setOpener([&](const QString &path, ole::Kind) {
            opened << path;
            return true;
        });
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B2")));
        crunch::SheetEmbeds *embeds = embedsOf(e);
        const int first = embeds->idsOf(0).front();
        const auto old = embeds->objectOf(0, first);
        e.win.w.objects()->layer()->select(first);
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        const std::vector<int> ids = embeds->idsOf(0);
        QCOMPARE(int(ids.size()), 2);
        const int second = ids[0] == first ? ids[1] : ids[0];
        QVERIFY(e.win.w.editObject(0, second));
        QCOMPARE(opened.size(), 1);
        // A cell is being typed when the other app saves: the typing goes on.
        e.win.w.objects()->layer()->select(-1);
        e.select(QStringLiteral("D5"));
        QTest::keyClick(e.view(), Qt::Key_X);
        QVERIFY(e.win.w.isEditing());
        QSignalSpy saves(ed, &jp::EmbeddedEdit::objectSaved);
        const QByteArray changed = objects::rangePackage(gridOf({"Name", "Total", "North", "99", "West", "5"}, 2));
        {
            QFile f(opened.first());
            QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
            f.write(changed);
        }
        QTRY_VERIFY((ed->pollNow(), saves.count() >= 1));
        QCOMPARE(embeds->objectOf(0, second)->partBytes, changed);
        QCOMPARE(embeds->objectOf(0, first)->partBytes, old->partBytes);   // (the one that was not edited is as it was)
        QVERIFY(e.win.w.isEditing());
    }

    void aMissingProgramIsSaidInTheStatusBar()
    {
        QTemporaryDir dir, temp;
        const QString page = savedFile(dir, QStringLiteral("page.docx"), docxOf({QStringLiteral("Words")}));
        EditWindow e;
        QVERIFY(ready(e));
        e.win.w.objectEditor()->setTempRoot(temp.path());
        e.win.w.objectEditor()->setOpener([](const QString &, ole::Kind) { return false; });
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        QVERIFY(!e.win.w.editObject(0, embedsOf(e)->idsOf(0).front()));
        QVERIFY(e.win.w.statusText().contains(QStringLiteral("can't be opened")));
        QCOMPARE(e.win.w.objectEditor()->openCount(), 0);
    }

    // ---- the list in File > Info

    void theListShowsWordsAndConvertDropsThePackage()
    {
        QTemporaryDir dir;
        const QString page = savedFile(dir, QStringLiteral("page.docx"), docxOf({QStringLiteral("Words")}));
        EditWindow e;
        QVERIFY(ready(e));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        QCOMPARE(embedsOf(e)->countOf(0), 2);
        withDialog([&] { e.win.w.showObjects(); }, QStringLiteral("objectListDialog"), [&](QDialog *d) {
            auto *list = qobject_cast<objects::ObjectListDialog *>(d);
            QVERIFY(list);
            QCOMPARE(list->rowCount(), 2);
            for (int i = 0; i < 2; ++i) {
                QCOMPARE(list->textAt(i, 1), QStringLiteral("Document page"));
                QVERIFY(!list->textAt(i, 2).isEmpty());
            }
            list->select(0);
            d->findChild<QPushButton *>(QStringLiteral("objectConvert"))->click();
            QCOMPARE(list->rowCount(), 1);
        });
        // One is a picture with no package now; the other is still an object.
        QCOMPARE(embedsOf(e)->countOf(0), 1);
        QCOMPARE(int(e.win.w.objects()->describe(0).size()), 1);
        withDialog([&] { e.win.w.showObjects(); }, QStringLiteral("objectListDialog"), [&](QDialog *d) {
            d->findChild<QPushButton *>(QStringLiteral("objectDelete"))->click();
            QCOMPARE(qobject_cast<objects::ObjectListDialog *>(d)->rowCount(), 0);
        });
        QCOMPARE(embedsOf(e)->countOf(0), 0);
    }

    // ---- hostile files

    void aFileWithThousandsOfObjectsKeepsToTheCountLimit()
    {
        LimitsGuard guard;
        ole::Limits small = ole::limits();
        small.objects = 40;
        ole::setLimits(small);
        QVector<Spec> specs;
        for (int i = 0; i < 400; ++i) {
            Spec s;
            s.bytes = "object " + QByteArray::number(i);
            s.png = drawtest::pngBytes(8, 8, Qt::green);
            s.fromCol = i % 50;
            s.toCol = i % 50 + 2;
            specs.push_back(s);
        }
        EditWindow e;
        QVERIFY(ready(e, bookWith(specs)));
        QTRY_COMPARE(embedsOf(e)->countOf(0), 40);
        QCOMPARE(e.win.w.objects()->layer()->count(), 40);
        // (what the file holds past the limit is left as it is: a save keeps every part)
        QMap<QString, QByteArray> after;
        QVERIFY(saveAndRead(e, &after));
        QCOMPARE(partsUnder(after, QStringLiteral("xl/embeddings/")).size(), 400);
    }

    void anObjectWhosePartIsMissingIsNotAnObjectAndNothingBreaks()
    {
        Spec gone, fine;
        gone.write = false;
        gone.png = drawtest::pngBytes(8, 8, Qt::red);
        fine.png = drawtest::pngBytes(8, 8, Qt::blue);
        fine.fromCol = 8;
        fine.toCol = 11;
        EditWindow e;
        QVERIFY(ready(e, bookWith({gone, fine})));
        QTRY_COMPARE(embedsOf(e)->countOf(0), 1);
        QMap<QString, QByteArray> after;
        QVERIFY(saveAndRead(e, &after));
        QVERIFY(after.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("<oleObject "));
    }

    void aHugeDeclaredPictureIsNeverUnpackedPastTheRenderCap()
    {
        // (8000 by 8000 is within the picture's side cap, and would unpack to 256 MB)
        const QByteArray huge = hugePng(8000);
        QVERIFY(huge.size() < 1024 * 1024);
        QVERIFY(crunch::SheetEmbeds::previewImage(huge).isNull());
        QVERIFY(!crunch::SheetEmbeds::previewImage(drawtest::pngBytes(40, 30, Qt::red)).isNull());
        Spec spec;
        spec.png = huge;
        EditWindow e;
        QVERIFY(ready(e, bookWith({spec})));
        QTRY_COMPARE(embedsOf(e)->countOf(0), 1);
        e.win.w.objects()->layer()->prepareAll();   // (it is drawn as the box that says what it is, with nothing unpacked)
        QCOMPARE(e.win.w.objects()->layer()->pendingRenders(), 0);
        // A side past the cap leaves the picture out and keeps the object.
        Spec wide;
        wide.png = hugePng(9000);
        EditWindow f;
        QVERIFY(ready(f, bookWith({wide})));
        QTRY_COMPARE(embedsOf(f)->countOf(0), 1);
        QVERIFY(embedsOf(f)->objectOf(0, embedsOf(f)->idsOf(0).front())->previewPng.isEmpty());
    }

    void anAnchorFarOutsideTheSheetIsPlacedWithoutHangingOrCrashing()
    {
        Spec far;
        far.png = drawtest::pngBytes(8, 8, Qt::red);
        far.fromCol = 16000;
        far.toCol = 16383;
        far.fromRow = 1000000;
        far.toRow = 1048575;
        far.offsets = QStringLiteral("99999999999");
        EditWindow e;
        QVERIFY(ready(e, bookWith({far})));
        QTRY_COMPARE(embedsOf(e)->countOf(0), 1);
        e.win.w.objects()->layer()->prepareAll();
        const int id = embedsOf(e)->idsOf(0).front();
        const auto box = e.win.w.objects()->layer()->boxNow(id);
        QVERIFY(std::isfinite(box.x) && std::isfinite(box.w));
        QMap<QString, QByteArray> after;
        QVERIFY(saveAndRead(e, &after));
        QCOMPARE(partsUnder(after, QStringLiteral("xl/embeddings/")).size(), 1);
    }

    // ---- pictures for Jeff

    void screenshotsOfTheInsertMenuAndTheList()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        QTemporaryDir files;
        QStringList lines;
        for (int i = 0; i < 30; ++i) lines << QStringLiteral("Line %1 of the page that is shown as the object").arg(i + 1);
        const QString page = savedFile(files, QStringLiteral("page.docx"), docxOf(lines));
        const QString book = savedFile(files, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12", "South", "30"}, 2)));
        EditWindow e;
        QVERIFY(ready(e));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B3")));
        QVERIFY(e.win.w.moveObject(embedsOf(e)->idsOf(0).front(), cells(3, 2, 7, 8)));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        QVERIFY(e.win.w.moveObject(embedsOf(e)->idsOf(0).back(), cells(9, 2, 13, 14)));
        e.win.w.objects()->layer()->prepareAll();
        e.win.w.objects()->layer()->select(embedsOf(e)->idsOf(0).back());
        QVERIFY(e.win.w.grab().save(dir + QStringLiteral("/crunch-window-with-objects.png")));
        QMenu menu;
        for (const char *name : {"insert.object.range", "insert.object.chart", "insert.object.page"}) menu.addAction(e.win.w.act(QString::fromLatin1(name)));
        menu.show();
        QVERIFY(menu.grab().save(dir + QStringLiteral("/crunch-insert-object-menu.png")));
        withDialog([&] { e.win.w.showObjects(); }, QStringLiteral("objectListDialog"), [&](QDialog *d) { shotOf(d, QStringLiteral("crunch-objects-list")); });
    }
};

JP_TEST_CLASS(CrunchObjectTests)
#include "tst_crunch_objects.moc"
