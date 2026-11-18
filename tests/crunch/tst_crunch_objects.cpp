// Tests of JeffCrunch's embedded objects: a range, chart, or page of one of our apps is held as an object of the sheet (its package and its picture, not a picture), and an object another
// program made is kept as the file has it. Insert > Object, Paste Special as an object, copy and paste, move and size, delete, Undo and Redo, the edit in the app that made it, saving to
// .xlsx and .ods and opening again, the list in File > Info, and hostile files. Every file is made here in code; no other program is started (the edit's opener is a stand-in that notes the
// path).
#include "../sheet/drawinghelpers.h"
#include "../withdialog.h"
#include "app/crunchwindow.h"
#include "app/objects.h"
#include "app/objectsdialog.h"
#include "app/theme.h"
#include "core/fonts.h"
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

QStringList partsUnder(const QMap<QString, QByteArray> &parts, const QString &folder)
{
    QStringList out;
    for (auto it = parts.begin(); it != parts.end(); ++it)
        if (it.key().startsWith(folder)) out << it.key();
    return out;
}

// What one object of a built workbook is: its program, its package's bytes (not written if `write` is false), its picture, and where it is (two cells).
struct Spec {
    QString progId = QStringLiteral("Foo.Widget.1");
    QByteArray bytes = "foreign package bytes";
    bool write = true;
    QByteArray png;
    int fromCol = 1, fromRow = 1, toCol = 4, toRow = 6;
    QString offsets = QStringLiteral("0");
};

// A one-sheet workbook (A1 holds 1) with the objects of the specs in its `oleObjects`, each with a part and a picture of its own.
QByteArray bookWith(const QVector<Spec> &specs)
{
    QString children;
    QVector<sheettest::ExtraPart> parts;
    for (int i = 0; i < specs.size(); ++i) {
        const Spec &s = specs[i];
        const int n = i + 1;
        const QString anchor = QStringLiteral("<anchor moveWithCells=\"1\"><xdr:from><xdr:col>%1</xdr:col><xdr:colOff>%5</xdr:colOff><xdr:row>%2</xdr:row><xdr:rowOff>%5</xdr:rowOff></xdr:from><xdr:to><xdr:col>%3</xdr:col>"
                                              "<xdr:colOff>%5</xdr:colOff><xdr:row>%4</xdr:row><xdr:rowOff>%5</xdr:rowOff></xdr:to></anchor>")
                                   .arg(s.fromCol).arg(s.fromRow).arg(s.toCol).arg(s.toRow).arg(s.offsets);
        const QString pic = s.png.isEmpty() ? QString() : QStringLiteral(" r:id=\"rIdP%1\"").arg(n);
        children += QStringLiteral("<oleObject xmlns:xdr=\"%1\" progId=\"%2\" shapeId=\"%3\" r:id=\"rIdO%4\"><objectPr defaultSize=\"0\"%5>%6</objectPr></oleObject>")
                        .arg(drawtest::kXdr, s.progId).arg(1024 + n).arg(n).arg(pic, anchor);
        parts.append({s.write ? QStringLiteral("xl/embeddings/oleObject%1.bin").arg(n) : QStringLiteral("xl/unused/r%1.bin").arg(n), s.write ? s.bytes : QByteArray("x"), QString(), 0,
                      QStringLiteral("rIdO%1").arg(n), QStringLiteral("oleObject"), QStringLiteral("/xl/embeddings/oleObject%1.bin").arg(n)});
        if (!s.png.isEmpty())
            parts.append({QStringLiteral("xl/media/p%1.png").arg(n), s.png, QStringLiteral("image/png"), 0, QStringLiteral("rIdP%1").arg(n), QStringLiteral("image"),
                          QStringLiteral("/xl/media/p%1.png").arg(n)});
    }
    const QByteArray sheet = sheettest::worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), children.isEmpty() ? QString() : QStringLiteral("<oleObjects>") + children + QStringLiteral("</oleObjects>"));
    return sheettest::withParts(sheettest::buildXlsx({sheet}), parts);
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
        QVERIFY(!embeds->canMove(0, id));
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
        QCOMPARE(read->previewPng, made->previewPng);
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
