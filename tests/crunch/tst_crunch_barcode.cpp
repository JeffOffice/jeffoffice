// Tests of Insert > Barcode in JeffCrunch: a barcode is an object over the sheet (a picture that is moved, sized, edited again, and undone like any), saved as a picture other programs
// show (an SVG with a PNG beside it) that carries the marker JeffCrunch opens as the same barcode.
#include "../sheet/drawinghelpers.h"
#include "../withdialog.h"
#include "editwindow.h"
#include "savedok.h"

#include "app/crunchinfo.h"
#include "app/barcodedialog.h"
#include "core/barcodeclip.h"
#include "core/barcodeitem.h"
#include "office/barcodemarker.h"
#include "sheet/xlsxsave.h"
#include "core/fonts.h"
#include "core/sheetobjects.h"
#include "io/zip.h"
#include "office/package.h"

#ifdef __GLIBC__
#include <malloc.h>
#endif
#include <QComboBox>
#include <QElapsedTimer>
#include <QFile>
#include <QLineEdit>
#include <QSignalSpy>
#include <QUndoStack>

#include "../testrunner.h"

using namespace crunchtest;
using namespace drawtest;
using namespace jp::office::sheet;

namespace {

QByteArray plainBook() { return buildXlsx({dataSheet()}); }

QMap<QString, QByteArray> partsOf(const QString &path)
{
    QMap<QString, QByteArray> parts;
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) jp::readZip(f.readAll(), parts);
    return parts;
}

} // namespace

class CrunchBarcodeTests : public QObject {
    Q_OBJECT

    static bool save(EditWindow &e)
    {
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        return e.win.w.save() && savedOk(saved);
    }
    // Insert > Barcode with the dialog answered: a QR code of `text`.
    static void insertQr(EditWindow &e, const QString &text)
    {
        withDialog([&] { e.win.w.act(QStringLiteral("insert.barcode"))->trigger(); }, QStringLiteral("barcodeDialog"), [&](QDialog *d) {
            QCOMPARE(d->windowTitle(), QStringLiteral("Insert Barcode"));
            d->findChildren<QComboBox *>().first()->setCurrentIndex(int(jp::barcode::Type::Qr));
            d->findChildren<QLineEdit *>().first()->setText(text);
        });
    }

    // A Code 128 barcode made from the text of A2 ("Q1"), linked to that cell; its id.
    static int linkedBarcode(EditWindow &e)
    {
        jp::office::sheet::CellRange range;
        range.valid = true;
        range.row1 = range.row2 = 1;
        range.col1 = range.col2 = 0;
        const QJsonObject look{{"type", int(jp::barcode::Type::Code128)}, {"data", "sample"}, {"text", true}};
        const CrunchWindow::BarcodeColumnResult r = e.win.w.insertBarcodesFromCells(range, look);
        if (r.made != 1) return -1;
        return e.win.w.objects()->idsOf(0).front();
    }
    // The data of the one barcode of the sheet that is not taken away (empty for none).
    static QString shownData(EditWindow &e)
    {
        const std::vector<int> ids = e.win.w.objects()->idsOf(0);
        return ids.size() == 1 ? e.win.w.objects()->barcodeOf(0, ids.front()).value(QLatin1String("data")).toString() : QStringLiteral("(%1 objects)").arg(ids.size());
    }
    // The marker of the one barcode a saved file holds: its settings, and whether the picture in the file is the one the marker was written with.
    static bool savedMarker(const QString &path, QJsonObject *settings, bool *pictureMatches)
    {
        const QMap<QString, QByteArray> parts = partsOf(path);
        const QByteArray drawing = parts.value(QStringLiteral("xl/drawings/drawing1.xml"));
        const qsizetype uri = drawing.indexOf("{5B6C1E0A"), from = uri < 0 ? -1 : drawing.lastIndexOf('<', uri), to = drawing.indexOf("</a:ext>", from);
        if (from < 0 || to < 0) {
            qWarning("no marker in: %s", drawing.left(3000).constData());
            return false;
        }
        const jp::office::BarcodeMarker m = jp::office::readBarcodeMarkerXml(drawing.mid(from, to + 8 - from));
        if (m.isEmpty()) return false;
        *settings = m.settings;
        *pictureMatches = false;
        for (auto it = parts.begin(); it != parts.end(); ++it)
            if (it.key().startsWith(QLatin1String("xl/media/")) && it.key().endsWith(QLatin1String(".png"))) *pictureMatches = jp::office::barcodeHash(it.value()) == m.pngHash;
        return true;
    }
    // A workbook whose A column holds T0, T1, ... and whose drawing holds a barcode picture at each row (linked to that row's cell, or plain pictures without a marker), far down the sheet.
    static QByteArray manyBarcodes(int n, bool linked)
    {
        crunch::ObjectClip clip;
        const QJsonObject first{{"type", int(jp::barcode::Type::Code128)}, {"data", "T0"}, {"text", true}};
        if (!crunch::barcodeClip(first, &clip)) return {};
        QString rows, anchors;
        for (int i = 0; i < n; ++i) {
            rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\" t=\"inlineStr\"><is><t>T%2</t></is></c></row>").arg(i + 1).arg(i);
            QJsonObject settings = first;
            settings.insert(QStringLiteral("data"), QStringLiteral("T%1").arg(i));
            settings.insert(QStringLiteral("cell"), QStringLiteral("Sheet1!$A$%1").arg(i + 1));
            const QString marker = linked ? QString::fromUtf8("<a:extLst>" + jp::office::barcodeMarkerExtXml(settings, jp::office::barcodeHash(clip.media[0].bytes), jp::office::barcodeHash(clip.media[1].bytes)) + "</a:extLst>") : QString();
            const QString pic = QStringLiteral("<xdr:pic><xdr:nvPicPr><xdr:cNvPr id=\"%1\" name=\"Picture %1\" descr=\"x\">%2</xdr:cNvPr><xdr:cNvPicPr/></xdr:nvPicPr><xdr:blipFill><a:blip r:embed=\"rIdP\">"
                                               "<a:extLst><a:ext uri=\"{96DAC541-7B7A-43D3-8B79-37D633B846F1}\"><asvg:svgBlip xmlns:asvg=\"http://schemas.microsoft.com/office/drawing/2016/SVG/main\" r:embed=\"rIdS\"/></a:ext></a:extLst></a:blip>"
                                               "<a:stretch><a:fillRect/></a:stretch></xdr:blipFill><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"952500\" cy=\"190500\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></xdr:spPr></xdr:pic>")
                                    .arg(i + 2).arg(marker);
            anchors += twoCell(3, 100 + i, 5, 101 + i, pic);
        }
        const QByteArray drawing = drawingPart(anchors);
        QVector<DrawingTarget> targets{imageTarget(QStringLiteral("rIdP"), 1, clip.media[0].bytes), {QStringLiteral("rIdS"), QStringLiteral("xl/media/image2.svg"), clip.media[1].bytes, QStringLiteral("image/svg+xml"), QStringLiteral("image")}};
        return bookWithDrawing({sheettest::worksheet(rows)}, drawing, targets);
    }
    static qint64 residentKb()
    {
#ifdef __GLIBC__
        malloc_trim(0);   // (what was freed is given back, so that the figure is what is held)
#endif
        QFile f(QStringLiteral("/proc/self/status"));
        if (!f.open(QIODevice::ReadOnly)) return 0;
        for (const QByteArray &line : f.readAll().split('\n'))
            if (line.startsWith("VmRSS:")) return line.mid(6).trimmed().split(' ').value(0).toLongLong();
        return 0;
    }

    static qint64 darkPixels(const QImage &img)
    {
        qint64 n = 0;
        const QImage g = img.convertToFormat(QImage::Format_ARGB32);
        for (int y = 0; y < g.height(); ++y)
            for (int x = 0; x < g.width(); ++x) {
                const QRgb c = g.pixel(x, y);
                if (qAlpha(c) > 128 && qGray(c) < 100) ++n;
            }
        return n;
    }
    // What print, print preview, and PDF export draw of the barcode of the sheet (the layer's own drawing of it, as the printed objects have it), at its size on the page.
    static QImage printedBarcode(EditWindow &e)
    {
        const std::vector<jp::grid::LayerObject> objects = e.win.w.objects()->printObjects(0);
        if (objects.size() != 1 || !objects.front().render) return {};
        const jp::grid::ObjectBox box = e.win.w.objects()->layer()->boxNow(objects.front().id);
        jp::grid::RenderRequest q;
        q.pixels = QSize(int(box.w * 2), int(box.h * 2));
        q.zoom = 2;
        q.units = QSizeF(box.w, box.h);
        q.inner = QRectF(QPointF(0, 0), q.units);
        return objects.front().render(q);
    }

    static void letGo()
    {
        for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
    }

private Q_SLOTS:
    void initTestCase()
    {
        setAppInfo(crunchAppInfo());
        jp::initCore();
    }

    // Insert > Barcode puts a picture object on the sheet as one step: Undo takes it off, and Redo puts it back.
    void insertUndoAndRedo()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        QVERIFY(e.win.w.act(QStringLiteral("insert.barcode")));
        crunch::SheetObjects *o = e.win.w.objects();
        const int steps = e.win.w.undoStack()->count();
        insertQr(e, QStringLiteral("https://jeffoffice.com/crunch"));
        QCOMPARE(o->layer()->count(), 1);
        const int id = o->layer()->selectedId();
        QCOMPARE(o->kindOf(0, id), ObjectKind::Picture);
        QCOMPARE(o->barcodeOf(0, id).value(QLatin1String("data")).toString(), QStringLiteral("https://jeffoffice.com/crunch"));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        const jp::grid::ObjectBox box = o->layer()->boxNow(id);
        QVERIFY(box.w > 50 && qAbs(box.w - box.h) < 0.5);
        e.win.w.undoStack()->undo();
        QCOMPARE(int(o->idsOf(0).size()), 0);
        e.win.w.undoStack()->redo();
        QCOMPARE(int(o->idsOf(0).size()), 1);
        QVERIFY(!o->barcodeOf(0, o->idsOf(0).front()).isEmpty());
    }

    // Barcodes for a column: a range of cells makes a barcode for each cell, with the cell's own text, as one step of Undo.
    void aColumnOfCellsMakesABarcodeForEach()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        const int steps = e.win.w.undoStack()->count();
        jp::office::sheet::CellRange range;
        range.valid = true;
        range.row1 = 1;
        range.row2 = 3;
        range.col1 = range.col2 = 0;   // A2:A4: Q1, Q2, Q3
        const QJsonObject look{{"type", int(jp::barcode::Type::Code128)}, {"data", "sample"}, {"text", true}};
        const CrunchWindow::BarcodeColumnResult r = e.win.w.insertBarcodesFromCells(range, look);
        QCOMPARE(r.made, 3);
        QVERIFY(r.problems.isEmpty());
        QCOMPARE(int(o->idsOf(0).size()), 3);
        QStringList data;
        for (int id : o->idsOf(0)) data << o->barcodeOf(0, id).value(QLatin1String("data")).toString();
        QCOMPARE(data, (QStringList{"Q1", "Q2", "Q3"}));
        // Each is drawn apart from the others, one under the next.
        QSet<qint64> tops;
        for (int id : o->idsOf(0)) tops.insert(qint64(o->layer()->boxNow(id).y));
        QCOMPARE(int(tops.size()), 3);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        e.win.w.undoStack()->undo();
        QCOMPARE(int(o->idsOf(0).size()), 0);
        e.win.w.undoStack()->redo();
        QCOMPARE(int(o->idsOf(0).size()), 3);
        // Saved and opened again, the three are still barcodes.
        QVERIFY(save(e));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 3);
        for (int id : second.w.objects()->idsOf(0)) QVERIFY(!second.w.objects()->barcodeOf(0, id).isEmpty());
    }

    // A value the code can't hold gets a message naming its cell, and no barcode; the count made at once is limited.
    void aValueTheCodeCannotHoldIsReportedNotDrawn()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        jp::office::sheet::CellRange range;
        range.valid = true;
        range.row1 = 1;
        range.row2 = 3;
        range.col1 = range.col2 = 0;
        const int steps = e.win.w.undoStack()->count();
        const QJsonObject ean{{"type", int(jp::barcode::Type::Ean8)}, {"data", "12345670"}};
        const CrunchWindow::BarcodeColumnResult bad = e.win.w.insertBarcodesFromCells(range, ean);
        QCOMPARE(bad.made, 0);
        QCOMPARE(int(bad.problems.size()), 3);
        QVERIFY2(bad.problems.first().startsWith(QLatin1String("A2")), qPrintable(bad.problems.first()));
        QCOMPARE(int(o->idsOf(0).size()), 0);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        // More cells than the limit: nothing is made.
        const QJsonObject code128{{"type", int(jp::barcode::Type::Code128)}, {"data", "x"}};
        const CrunchWindow::BarcodeColumnResult over = e.win.w.insertBarcodesFromCells(range, code128, 2);
        QVERIFY(over.tooMany);
        QCOMPARE(over.made, 0);
        QCOMPARE(int(o->idsOf(0).size()), 0);
    }

    // Ctrl+A on a sheet with a few cells in it: only the cells the sheet uses are looked at, however large the selection is.
    void aHugeSelectionLooksOnlyAtTheUsedCells()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        jp::office::sheet::CellRange all;
        all.valid = true;
        all.row1 = all.col1 = 0;
        all.row2 = 1048575;
        all.col2 = 16383;
        const QJsonObject look{{"type", int(jp::barcode::Type::Code128)}, {"data", "x"}};
        QElapsedTimer timer;
        timer.start();
        const CrunchWindow::BarcodeColumnResult r = e.win.w.insertBarcodesFromCells(all, look);
        QVERIFY2(r.visited <= 100, qPrintable(QString::number(r.visited)));
        QVERIFY(timer.elapsed() < 20000);
    }

    // A picture of the window with a QR code on the sheet, for the docs, when JP_SHOT_DIR names a folder.
    void aPictureOfTheWindowForTheDocs()
    {
        if (qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) return;
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        e.win.w.resize(1300, 820);
        e.win.w.show();
        insertQr(e, QStringLiteral("https://jeffoffice.com"));
        // (The object is drawn on a worker: the picture is taken once the bars are on the screen.)
        QImage shot;
        QTRY_VERIFY2([&] {
            shot = e.win.w.grab().toImage();
            int black = 0;
            for (int y = 200; y < shot.height() - 60; ++y)
                for (int x = 400; x < shot.width() - 100; ++x) black += (shot.pixel(x, y) & 0xFFFFFF) == 0 ? 1 : 0;
            return black > 300;
        }(), "the barcode was not drawn");
        shot.save(qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/crunch-qr.png"));
    }

    // With the barcode selected, Insert > Barcode edits it: the dialog starts from its data, the new barcode is where the old one was, and one step undoes it.
    void editingReopensTheDialogWithItsData()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        insertQr(e, QStringLiteral("first"));
        o->layer()->select(o->idsOf(0).front());
        const jp::grid::ObjectBox was = o->layer()->boxNow(o->layer()->selectedId());
        const int steps = e.win.w.undoStack()->count();
        withDialog([&] { e.win.w.act(QStringLiteral("insert.barcode"))->trigger(); }, QStringLiteral("barcodeDialog"), [&](QDialog *d) {
            QCOMPARE(d->windowTitle(), QStringLiteral("Edit Barcode"));
            auto *line = d->findChildren<QLineEdit *>().first();
            QCOMPARE(line->text(), QStringLiteral("first"));
            line->setText(QStringLiteral("second"));
        });
        QCOMPARE(int(o->idsOf(0).size()), 1);
        QCOMPARE(int(o->idsOf(0).size()), 1);
        const int id = o->layer()->selectedId();
        QCOMPARE(o->barcodeOf(0, id).value(QLatin1String("data")).toString(), QStringLiteral("second"));
        const jp::grid::ObjectBox now = o->layer()->boxNow(id);
        QVERIFY(qAbs(now.x - was.x) < 0.5 && qAbs(now.y - was.y) < 0.5);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        e.win.w.undoStack()->undo();
        QCOMPARE(int(o->idsOf(0).size()), 1);
        QCOMPARE(o->barcodeOf(0, o->idsOf(0).front()).value(QLatin1String("data")).toString(), QStringLiteral("first"));
    }

    // Saved, the barcode is a picture of the drawing (an SVG with a PNG, each with its content type), with the marker; another program shows the picture. Opened again, it is the same
    // barcode, and saved a second time it is still one.
    void aSavedBarcodeIsAPictureAndReopensAsABarcode()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        insertQr(e, QStringLiteral("https://jeffoffice.com/round-trip"));
        const QJsonObject settings = o->barcodeOf(0, o->idsOf(0).front());
        QVERIFY(!settings.isEmpty());
        QVERIFY(save(e));
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        const QByteArray drawing = parts.value(QStringLiteral("xl/drawings/drawing1.xml"));
        QVERIFY2(drawing.contains("<xdr:pic"), drawing.left(1500).constData());
        QVERIFY(drawing.contains("jeffoffice.com/ns/barcode"));
        QVERIFY(drawing.contains("<asvg:svgBlip "));
        QVERIFY(drawing.contains("descr=\"Barcode: QR code https://jeffoffice.com/round-trip\""));
        QString svg, png;
        for (auto it = parts.begin(); it != parts.end(); ++it) {
            if (it.key().startsWith(QLatin1String("xl/media/")) && it.key().endsWith(QLatin1String(".svg"))) svg = it.key();
            if (it.key().startsWith(QLatin1String("xl/media/")) && it.key().endsWith(QLatin1String(".png"))) png = it.key();
        }
        QVERIFY(!svg.isEmpty() && !png.isEmpty());
        QVERIFY(parts.value(svg).contains("<svg"));
        QVERIFY(parts.value(png).startsWith("\x89PNG"));
        jp::office::Package pkg;
        QVERIFY(pkg.read(QByteArray([&] {
            QFile f(e.path);
            f.open(QIODevice::ReadOnly);
            return f.readAll();
        }())));
        QCOMPARE(pkg.contentType(QLatin1Char('/') + svg), QStringLiteral("image/svg+xml"));
        QCOMPARE(pkg.contentType(QLatin1Char('/') + png), QStringLiteral("image/png"));
        int images = 0;
        for (const jp::office::Relationship &r : pkg.relationships(QStringLiteral("/xl/drawings/drawing1.xml")))
            if (r.type.endsWith(QLatin1String("/image")) && (r.target == QLatin1Char('/') + svg || r.target == QLatin1Char('/') + png)) ++images;
        QCOMPARE(images, 2);

        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 1);
        crunch::SheetObjects *again = second.w.objects();
        QCOMPARE(again->barcodeOf(0, again->idsOf(0).front()), settings);
    }

    // A barcode made from a cell follows it: the cell changes, and the barcode is made again from its text in the same place, at the same size, as a step after the cell's. Undo gives the
    // old barcode back.
    void aCellEditUpdatesItsBarcode()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        const int first = linkedBarcode(e);
        QVERIFY(first >= 0);
        QCOMPARE(shownData(e), QStringLiteral("Q1"));
        QCOMPARE(o->barcodeOf(0, first).value(QLatin1String("cell")).toString(), QStringLiteral("Sheet1!$A$2"));
        const jp::grid::ObjectBox was = o->layer()->boxNow(first);
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("NEW-42")).ok);
        QTRY_COMPARE(shownData(e), QStringLiteral("NEW-42"));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);   // (the barcode is part of the step that changed the cell)
        const int now = o->idsOf(0).front();
        QVERIFY(now != first);
        const jp::grid::ObjectBox box = o->layer()->boxNow(now);
        QVERIFY(qAbs(box.x - was.x) < 0.5 && qAbs(box.y - was.y) < 0.5 && qAbs(box.w - was.w) < 0.5 && qAbs(box.h - was.h) < 0.5);
        QCOMPARE(o->barcodeOf(0, now).value(QLatin1String("cell")).toString(), QStringLiteral("Sheet1!$A$2"));
        // One Undo takes the cell and the new barcode back: the old barcode is there again, with the cell's old text. Redo does both again, and makes no step.
        e.win.w.undoStack()->undo();
        QCOMPARE(shownData(e), QStringLiteral("Q1"));
        QCOMPARE(o->idsOf(0).front(), first);
        QCOMPARE(e.win.w.session()->neutralText(0, 1, 0), QStringLiteral("Q1"));
        letGo();
        QCOMPARE(shownData(e), QStringLiteral("Q1"));
        QVERIFY(e.win.w.undoStack()->canRedo());
        e.win.w.undoStack()->redo();
        QCOMPARE(shownData(e), QStringLiteral("NEW-42"));
        QCOMPARE(e.win.w.session()->neutralText(0, 1, 0), QStringLiteral("NEW-42"));
        letGo();
        QCOMPARE(shownData(e), QStringLiteral("NEW-42"));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QVERIFY(!e.win.w.undoStack()->canRedo());
    }

    // Rows inserted above the cell move the link with it, and a barcode follows the cell at its new place.
    void aRowInsertedAboveKeepsTheLink()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(linkedBarcode(e) >= 0);
        QVERIFY(e.win.w.session()->insertRows(0, 0, 2).ok);
        letGo();
        QCOMPARE(shownData(e), QStringLiteral("Q1"));
        QCOMPARE(o->barcodeLinkOf(0, o->idsOf(0).front()), QStringLiteral("Sheet1!$A$4"));
        QVERIFY(e.win.w.session()->type(0, 3, 0, QStringLiteral("MOVED")).ok);
        QTRY_COMPARE(shownData(e), QStringLiteral("MOVED"));
        // (the cell it had before is another now: typing there changes nothing)
        QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("OTHER")).ok);
        letGo();
        QCOMPARE(shownData(e), QStringLiteral("MOVED"));
    }

    // The cell's row deleted: the barcode keeps its last value, and nothing breaks (here, or when the file is saved and opened).
    void aDeletedCellLeavesTheBarcodeAsItWas()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(linkedBarcode(e) >= 0);
        QVERIFY(e.win.w.session()->deleteRows(0, 1, 1).ok);
        letGo();
        QCOMPARE(shownData(e), QStringLiteral("Q1"));
        QVERIFY(o->linkedBarcodes().size() == 1 && !o->linkedBarcodes().front().found);
        QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("Another")).ok);
        letGo();
        QCOMPARE(shownData(e), QStringLiteral("Q1"));
        QVERIFY(save(e));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 1);
        QVERIFY(!second.w.objects()->barcodeOf(0, second.w.objects()->idsOf(0).front()).isEmpty());
    }

    // Saved and opened again, the barcode is still linked: the picture in the file is of the text the cell had when it was saved (the marker is honored only for the picture that was written), the
    // link follows rows inserted before the save, and a change of the cell in the reopened workbook updates it; saved once more, the file has that.
    void theLinkSurvivesSaveAndReopen()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        QVERIFY(linkedBarcode(e) >= 0);
        QVERIFY(e.win.w.session()->insertRows(0, 0, 1).ok);
        QVERIFY(e.win.w.session()->type(0, 2, 0, QStringLiteral("SAVED-1")).ok);
        QTRY_COMPARE(shownData(e), QStringLiteral("SAVED-1"));
        QVERIFY(save(e));
        EditWindow again;
        QVERIFY(again.open([&] {
            QFile f(e.path);
            f.open(QIODevice::ReadOnly);
            return f.readAll();
        }()));
        QVERIFY(again.settle());
        QTRY_VERIFY(again.win.w.objects()->shownSheet() == 0 && again.win.w.objects()->layer()->count() == 1);
        crunch::SheetObjects *o = again.win.w.objects();
        QCOMPARE(shownData(again), QStringLiteral("SAVED-1"));
        QCOMPARE(o->barcodeLinkOf(0, o->idsOf(0).front()), QStringLiteral("Sheet1!$A$3"));
        // A cell changed in this workbook: the save writes the barcode from it, whether or not the window has done it yet.
        QVERIFY(again.win.w.session()->type(0, 2, 0, QStringLiteral("SAVED-2")).ok);
        {
            jp::office::sheet::SaveChanges changes;
            o->addToSave(&changes);
            QVERIFY(!changes.objectBarcodeMarkers.empty());
            QCOMPARE(int(changes.producedParts.size()), 2);
        }
        QTRY_COMPARE(shownData(again), QStringLiteral("SAVED-2"));
        QVERIFY(save(again));
        EditWindow third;
        QVERIFY(third.open([&] {
            QFile f(again.path);
            f.open(QIODevice::ReadOnly);
            return f.readAll();
        }()));
        QVERIFY(third.settle());
        QTRY_VERIFY(third.win.w.objects()->shownSheet() == 0 && third.win.w.objects()->layer()->count() == 1);
        QCOMPARE(shownData(third), QStringLiteral("SAVED-2"));
        QVERIFY(third.win.w.session()->type(0, 2, 0, QStringLiteral("SAVED-3")).ok);
        QTRY_COMPARE(shownData(third), QStringLiteral("SAVED-3"));
    }

    // A cell that holds an error leaves the barcode as it was (in the window and in the saved file); the next good text makes it again.
    void anErrorInTheCellLeavesTheBarcodeAsItWas()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        QVERIFY(linkedBarcode(e) >= 0);
        QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("=1/0")).ok);
        QCOMPARE(e.win.w.session()->linkText(0, 1, 0), QString());
        letGo();
        QCOMPARE(shownData(e), QStringLiteral("Q1"));
        QVERIFY(save(e));
        QJsonObject marker;
        bool matches = false;
        QVERIFY(savedMarker(e.path, &marker, &matches));
        QCOMPARE(marker.value(QLatin1String("data")).toString(), QStringLiteral("Q1"));
        QVERIFY(matches);
        QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("GOOD")).ok);
        QTRY_COMPARE(shownData(e), QStringLiteral("GOOD"));
    }

    // A text the code can't hold is tried once: the barcode keeps what it had, the message is told once, and no more tries are made until the cell's text is another.
    void aTextTheCodeCannotHoldIsNotTriedAgainUntilTheCellChanges()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("12345670")).ok);
        jp::office::sheet::CellRange range;
        range.valid = true;
        range.row1 = range.row2 = 1;
        range.col1 = range.col2 = 0;
        QCOMPARE(e.win.w.insertBarcodesFromCells(range, QJsonObject{{"type", int(jp::barcode::Type::Ean8)}, {"data", "12345670"}}).made, 1);
        letGo();
        const qint64 base = e.win.w.barcodeTriesForTest();
        QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("ABCDEFGH")).ok);
        QTRY_COMPARE(e.win.w.barcodeTriesForTest(), base + 1);
        QCOMPARE(shownData(e), QStringLiteral("12345670"));
        QCOMPARE(e.win.w.barcodeFailuresToldForTest(), qint64(1));
        for (int i = 0; i < 5; ++i) {
            QVERIFY(e.win.w.session()->type(0, 3, 2, QString::number(i)).ok);   // (other cells change)
            letGo();
        }
        QCOMPARE(e.win.w.barcodeTriesForTest(), base + 1);
        QCOMPARE(e.win.w.barcodeFailuresToldForTest(), qint64(1));   // (told once)
        // The cell's text changes to one it can hold: made again.
        QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("96385074")).ok);
        QTRY_COMPARE(shownData(e), QStringLiteral("96385074"));
    }

    // A change made while the cell is being edited (or another large change is going on) is not dropped: it is done when that is over.
    void aSyncThatCannotRunYetIsTriedAgain()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        QVERIFY(linkedBarcode(e) >= 0);
        e.select(QStringLiteral("C5"));
        QTest::keyClick(e.view(), Qt::Key_F2);
        QVERIFY(e.win.w.isEditing());
        QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("LATER")).ok);
        letGo();
        QCOMPARE(shownData(e), QStringLiteral("Q1"));
        QTest::keyClick(e.win.w.cellEditor(), Qt::Key_Escape);
        QVERIFY(!e.win.w.isEditing());
        QTRY_COMPARE(shownData(e), QStringLiteral("LATER"));
    }

    // A replaced barcode gives its slot back: after many edits of one linked cell the workbook still counts one live object, the barcode still updates, and the memory held stays bounded.
    void manyEditsOfOneLinkedCellKeepOneSlot()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(linkedBarcode(e) >= 0);
        QCOMPARE(o->liveObjects(), 1);
        const qint64 before = residentKb();
        QElapsedTimer clock;
        clock.start();
        for (int i = 0; i < 5000; ++i) {
            QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("V%1").arg(i)).ok);
            letGo();
            if (shownData(e) != QStringLiteral("V%1").arg(i)) QTRY_COMPARE(shownData(e), QStringLiteral("V%1").arg(i));
        }
        qInfo("5000 edits: %lld ms, %lld MB more, Undo holds %lld MB", clock.elapsed(), (residentKb() - before) / 1024, e.win.w.session()->undoBytes() >> 20);
        QCOMPARE(o->liveObjects(), 1);
        QCOMPARE(shownData(e), QStringLiteral("V4999"));
        // What Undo keeps of the swapped-out barcodes is counted against the limit of the steps (128 MB), and the oldest steps are forgotten past it with their barcodes let go: the memory held stays
        // within a few times that, however many edits are made. The newest edit still undoes.
        QVERIFY2(e.win.w.session()->undoBytes() <= e.win.w.session()->undoByteLimit() + (1 << 20), qPrintable(QString::number(e.win.w.session()->undoBytes())));
        QVERIFY2((residentKb() - before) / 1024 < 450, qPrintable(QString::number((residentKb() - before) / 1024)));
        e.win.w.undoStack()->undo();
        QCOMPARE(shownData(e), QStringLiteral("V4998"));
        QCOMPARE(o->liveObjects(), 1);
    }

    // What one swap keeps for Undo, measured: the barcodes swapped out are kept (here with the limit lifted, so that none is let go), and the memory per swap is what heldBytesOf counts.
    void aSwappedOutBarcodeKeepsAboutWhatIsCounted()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        QVERIFY(linkedBarcode(e) >= 0);
        e.win.w.session()->setUndoByteLimit(Q_INT64_C(1) << 40);
        for (int i = 0; i < 20; ++i) {   // (to get past what the first ones allocate once)
            QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("W%1").arg(i)).ok);
            QTRY_COMPARE(shownData(e), QStringLiteral("W%1").arg(i));
        }
        const qint64 before = residentKb(), steps = e.win.w.session()->undoBytes();
        for (int i = 0; i < 500; ++i) {
            QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("X%1").arg(i)).ok);
            letGo();
            if (shownData(e) != QStringLiteral("X%1").arg(i)) QTRY_COMPARE(shownData(e), QStringLiteral("X%1").arg(i));
        }
        const qint64 perSwapKb = (residentKb() - before) / 500, countedKb = (e.win.w.session()->undoBytes() - steps) / 500 / 1024;
        qInfo("one swap keeps %lld KB; Undo counts %lld KB; images kept %d, decoded %lld KB", perSwapKb, countedKb, e.win.w.objects()->imagesKept(), e.win.w.objects()->imageBytesDecoded() / 1024);
        QVERIFY2(countedKb >= perSwapKb, qPrintable(QStringLiteral("%1 counted, %2 held").arg(countedKb).arg(perSwapKb)));
        // With a small limit the oldest steps are forgotten and their barcodes let go: 500 more swaps hold a fraction of what 500 held.
        e.win.w.session()->setUndoByteLimit(Q_INT64_C(16) << 20);
        const qint64 held = residentKb();
        for (int i = 0; i < 500; ++i) {
            QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("Y%1").arg(i)).ok);
            letGo();
            if (shownData(e) != QStringLiteral("Y%1").arg(i)) QTRY_COMPARE(shownData(e), QStringLiteral("Y%1").arg(i));
        }
        qInfo("under a 16 MB limit 500 more swaps hold %lld MB more (without it %lld MB); Undo holds %lld MB", (residentKb() - held) / 1024, perSwapKb * 500 / 1024, e.win.w.session()->undoBytes() >> 20);
        if (held > 0)   // (resident memory is read from /proc: Linux only; elsewhere residentKb() is 0 and there is nothing to compare)
            QVERIFY2(residentKb() - held < perSwapKb * 500 / 4, qPrintable(QString::number(residentKb() - held)));
    }

    // Ten thousand pictures with a link marker (a document honors 2,000 of them as barcodes, the rest are plain pictures) and a change of all their cells: the swaps are done a few at a time (no turn
    // of the window is long), as part of the step of the fill, within a time and a memory bound.
    void aFillOfTenThousandLinkedCellsIsDoneInShortTurns()
    {
        EditWindow e;
        QVERIFY(e.open(manyBarcodes(10000, true)));
        QVERIFY(e.settle());
        QTRY_VERIFY_WITH_TIMEOUT(e.win.w.objects()->shownSheet() == 0 && e.win.w.objects()->listed(0), 60000);
        crunch::SheetObjects *o = e.win.w.objects();
        QCOMPARE(int(o->linkedBarcodes().size()), 2000);
        QVector<QStringList> table;
        for (int i = 0; i < 10000; ++i) table << QStringList{QStringLiteral("U%1").arg(i)};
        jp::office::sheet::CellRange target;
        target.valid = true;
        target.row1 = 0;
        target.row2 = 9999;
        target.col1 = target.col2 = 0;
        const int steps = e.win.w.undoStack()->count(), liveBefore = o->liveObjects();
        const qint64 before = residentKb();
        QElapsedTimer clock;
        clock.start();
        QVERIFY(e.win.w.session()->pasteText(0, target, table, {}).ok);
        QTRY_VERIFY_WITH_TIMEOUT(e.win.w.barcodeTriesForTest() >= 1000 && e.win.w.barcodeFailuresToldForTest() == 1, 300000);
        letGo();
        qInfo("1000 swaps: %lld ms, longest turn %lld ms, %lld MB more", clock.elapsed(), e.win.w.longestBarcodeTurnForTest(), (residentKb() - before) / 1024);
        // (a checked build, with the memory and undefined-behavior checkers, runs slower and sets JP_TEST_TIME_SCALE)
        QVERIFY2(e.win.w.longestBarcodeTurnForTest() < 200 * std::max(1, qEnvironmentVariableIntValue("JP_TEST_TIME_SCALE")), qPrintable(QString::number(e.win.w.longestBarcodeTurnForTest())));
        QVERIFY(clock.elapsed() < 240000);
        QVERIFY2((residentKb() - before) / 1024 < 3000, qPrintable(QString::number((residentKb() - before) / 1024)));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);   // (the swaps are part of the step of the fill)
        QCOMPARE(int(o->linkedBarcodes().size()), 2000);
        // (a workbook holds as many new objects as one save writes, 1,000: the other 1,000 wait for a save, and the window said so once; a save writes them from the cells in any case)
        int stale = 0;
        for (const crunch::SheetObjects::LinkedBarcode &l : o->linkedBarcodes())
            if (!l.settings.value(QLatin1String("data")).toString().startsWith(QLatin1Char('U'))) ++stale;
        QVERIFY2(stale >= 900 && stale <= 1100, qPrintable(QString::number(stale)));
        QCOMPARE(e.win.w.barcodeFailuresToldForTest(), qint64(1));
        // (the swaps gave their slots back: without that the count would be 1,000 more; the scan above may have made a few more pictures of the file)
        QVERIFY2(o->liveObjects() <= liveBefore + 50, qPrintable(QString::number(o->liveObjects() - liveBefore)));
    }

    // A workbook whose pictures hold no link pays nothing for the feature: an edit or a save makes none of them.
    void picturesWithNoLinkAreNotMadeByAnEditOrASave()
    {
        EditWindow e;
        QVERIFY(e.open(manyBarcodes(300, false)));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        letGo();
        const int made = o->objectsMade();
        QVERIFY(made < 300);   // (the pictures far down the sheet are not made until they are drawn)
        QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("edit")).ok);
        letGo();
        QCOMPARE(o->objectsMade(), made);
        QVERIFY(o->linkedBarcodes().empty());
        QCOMPARE(o->objectsMade(), made);
        jp::office::sheet::SaveChanges changes;
        o->addToSave(&changes);
        QCOMPARE(o->objectsMade(), made);
        QVERIFY(save(e));
        QCOMPARE(o->objectsMade(), made);
    }

    // A linked barcode copied to another sheet follows the same cell; a renamed sheet keeps the link; and in the saved file the marker (not only the item shown) has the cell's text and
    // reference, and its picture is the one the marker was written with, through save, reopen, edit, save, and reopen.
    void theMarkerInTheFileFollowsTheCellThroughRenamesCopiesAndSaves()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({dataSheet(), dataSheet()})));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        const int first = linkedBarcode(e);
        QVERIFY(first >= 0);
        // copied to the other sheet
        crunch::ObjectClip clip;
        QVERIFY(o->copyElement(0, first, &clip));
        o->show(1);
        const int copy = o->makeElement(1, clip, o->anchorOf(0, first));
        QVERIFY(copy >= 0);
        crunch::ObjectEdit put;
        put.sheet = 1;
        put.id = copy;
        put.before = o->stateOf(1, copy);
        put.after = put.before;
        put.after.deleted = false;
        QVERIFY(e.win.w.session()->changeObjects({put}, QStringLiteral("Paste")).ok);
        QCOMPARE(o->barcodeLinkOf(1, copy), QStringLiteral("Sheet1!$A$2"));
        // a renamed sheet
        QVERIFY(e.win.w.session()->renameSheet(0, QStringLiteral("Sales Q1")).ok);
        QCOMPARE(o->barcodeLinkOf(0, o->idsOf(0).front()), QStringLiteral("'Sales Q1'!$A$2"));
        QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("ONE")).ok);
        QTRY_COMPARE(shownData(e), QStringLiteral("ONE"));
        QTRY_COMPARE(o->barcodeOf(1, o->idsOf(1).front()).value(QLatin1String("data")).toString(), QStringLiteral("ONE"));
        QVERIFY(save(e));
        // (the drawing of the first sheet: its marker holds the text and the renamed sheet, and the picture is the marker's)
        QJsonObject marker;
        bool matches = false;
        QVERIFY(savedMarker(e.path, &marker, &matches));
        QCOMPARE(marker.value(QLatin1String("data")).toString(), QStringLiteral("ONE"));
        QCOMPARE(marker.value(QLatin1String("cell")).toString(), QStringLiteral("'Sales Q1'!$A$2"));
        QVERIFY(matches);
        auto reopen = [&](EditWindow &w, const QString &from) {
            QFile f(from);
            f.open(QIODevice::ReadOnly);
            QVERIFY(w.open(f.readAll()));
            QVERIFY(w.settle());
            QTRY_VERIFY(w.win.w.objects()->shownSheet() == 0 && w.win.w.objects()->layer()->count() == 1);
        };
        EditWindow again;
        reopen(again, e.path);
        QCOMPARE(shownData(again), QStringLiteral("ONE"));
        QVERIFY(again.win.w.session()->insertRows(0, 0, 3).ok);
        QVERIFY(again.win.w.session()->type(0, 4, 0, QStringLiteral("TWO")).ok);
        QTRY_COMPARE(shownData(again), QStringLiteral("TWO"));
        QVERIFY(save(again));
        QVERIFY(savedMarker(again.path, &marker, &matches));
        QCOMPARE(marker.value(QLatin1String("data")).toString(), QStringLiteral("TWO"));
        QCOMPARE(marker.value(QLatin1String("cell")).toString(), QStringLiteral("'Sales Q1'!$A$5"));
        QVERIFY(matches);
        EditWindow third;
        reopen(third, again.path);
        QCOMPARE(shownData(third), QStringLiteral("TWO"));
        QCOMPARE(third.win.w.objects()->barcodeLinkOf(0, third.win.w.objects()->idsOf(0).front()), QStringLiteral("'Sales Q1'!$A$5"));
    }

    // The barcode dialog of every app keeps the keys of a barcode's settings that it does not show, such as the cell a barcode is linked to.
    void theDialogKeepsKeysItDoesNotKnow()
    {
        QJsonObject was{{"type", int(jp::barcode::Type::Qr)}, {"data", "old"}, {"cell", "Sheet1!$A$2"}, {"futureKey", 7}};
        QJsonObject out;
        bool ok = false;
        withDialog([&] { ok = jp::askBarcode(nullptr, was, &out); }, QStringLiteral("barcodeDialog"), [&](QDialog *d) { d->findChildren<QLineEdit *>().first()->setText(QStringLiteral("new")); });
        QVERIFY(ok);
        QCOMPARE(out.value(QLatin1String("data")).toString(), QStringLiteral("new"));
        QCOMPARE(out.value(QLatin1String("cell")).toString(), QStringLiteral("Sheet1!$A$2"));
        QCOMPARE(out.value(QLatin1String("futureKey")).toInt(), 7);
    }

    // After a cell edit the user sees the bars of the new value, at the object's size, at once (the picture kept in the file is made at a save): the sheet view shows them, the printed
    // objects (print, print preview, and PDF export draw those) hold them, a copy of the barcode has its real picture, and so has the saved file.
    void aSwappedBarcodeShowsItsBarsAtOnceAndEveryOutputHasTheRealPicture()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        const QImage empty = e.view()->grab().toImage();
        const int first = linkedBarcode(e);
        QVERIFY(first >= 0);
        QImage before;
        QTRY_VERIFY2(darkPixels(before = e.view()->grab().toImage()) > darkPixels(empty) + 400, "the barcode is drawn");
        const QImage printedBefore = printedBarcode(e);
        QVERIFY(darkPixels(printedBefore) > 400);
        const jp::grid::ObjectBox box = o->layer()->boxNow(first);
        QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("NEW-42-LONGER")).ok);
        QTRY_COMPARE(shownData(e), QStringLiteral("NEW-42-LONGER"));
        const int id = o->idsOf(0).front();
        const jp::grid::ObjectBox now = o->layer()->boxNow(id);
        QVERIFY(qAbs(now.w - box.w) < 0.5 && qAbs(now.h - box.h) < 0.5);   // (the size it had)
        // the view
        QImage after;
        auto viewShowsTheNewBars = [&] {
            after = e.view()->grab().toImage();
            if (after.size() != before.size()) return false;
            qint64 differing = 0;
            int left = after.width(), right = 0;
            for (int y = 0; y < after.height(); ++y)
                for (int x = 0; x < after.width(); ++x)
                    if (before.pixel(x, y) != after.pixel(x, y)) {
                        ++differing;
                        left = std::min(left, x);
                        right = std::max(right, x);
                    }
            return differing > 1500 && right - left > 80 && darkPixels(after) > darkPixels(empty) + 400;
        };
        QTRY_VERIFY2(viewShowsTheNewBars(), "the view shows the bars of the new value");
        // print, print preview, and PDF export
        const QImage printed = printedBarcode(e);
        QVERIFY(printed.width() > 100 && darkPixels(printed) > 400);
        QVERIFY(printed != printedBefore);
        // copy: the barcode as a clip has its real picture
        crunch::ObjectClip clip;
        QVERIFY(o->copyElement(0, id, &clip));
        QImage copied;
        for (const crunch::ClipMedia &m : std::as_const(clip.media))
            if (m.extension == QLatin1String("png")) copied.loadFromData(m.bytes);
        QVERIFY2(copied.width() > 100 && copied.height() > 30, "copy has the real picture");
        QCOMPARE(copied.format() == QImage::Format_Invalid, false);
        // saved
        QVERIFY(save(e));
        QImage saved;
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        for (auto it = parts.cbegin(); it != parts.cend(); ++it)
            if (it.key().startsWith(QLatin1String("xl/media/")) && it.key().endsWith(QLatin1String(".png"))) saved.loadFromData(it.value());
        QVERIFY2(saved.width() > 100 && saved.height() > 30, "the file has the real picture");
    }
};

JP_TEST_CLASS(CrunchBarcodeTests)
#include "tst_crunch_barcode.moc"
