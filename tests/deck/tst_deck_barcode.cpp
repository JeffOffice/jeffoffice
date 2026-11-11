// JeffDeck's tests of Insert > Barcode: a barcode is an object on the slide (moved, sized, edited again, undone), and saved as a picture other programs show
// that JeffDeck opens as the same barcode.
#include "deckhelpers.h"
#include "../withdialog.h"
#include "core/barcodeitem.h"

namespace {

// Insert > Barcode with the dialog answered: a QR code of `text`.
void insertQr(DeckWindow &w, const QString &text)
{
    withDialog([&] { w.act(QStringLiteral("insert.barcode"))->trigger(); }, QStringLiteral("barcodeDialog"), [&](QDialog *d) {
        QCOMPARE(d->windowTitle(), QStringLiteral("Insert Barcode"));
        auto *type = d->findChildren<QComboBox *>().first();
        type->setCurrentIndex(int(barcode::Type::Qr));
        d->findChildren<QLineEdit *>().first()->setText(text);
    });
}

GroupItem *lastBarcode(Document &d)
{
    for (auto it = d.pages[0]->items.rbegin(); it != d.pages[0]->items.rend(); ++it)
        if (isBarcodeItem(it->get())) return static_cast<GroupItem *>(it->get());
    return nullptr;
}

} // namespace

class DeckBarcodeTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // Insert > Barcode puts a barcode in the middle of the slide as one step: Undo takes it away and Redo puts it back.
    void insertUndoAndRedo()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        const int before = int(ed->doc()->pages[0]->items.size());
        insertQr(w, QStringLiteral("https://jeffoffice.com/deck"));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);
        GroupItem *g = lastBarcode(*ed->doc());
        QVERIFY(g);
        QCOMPARE(g->barcode.value(QLatin1String("data")).toString(), QStringLiteral("https://jeffoffice.com/deck"));
        QCOMPARE(g->barcode.value(QLatin1String("type")).toInt(), int(barcode::Type::Qr));
        QVERIFY(g->altText.startsWith(QLatin1String("Barcode: QR code")));
        const QSizeF page = ed->doc()->pageSize();
        QVERIFY(qAbs(g->rect.center().x() - page.width() / 2) < 1 && qAbs(g->rect.center().y() - page.height() / 2) < 1);
        ed->undoStack()->undo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before);
        QVERIFY(!lastBarcode(*ed->doc()));
        ed->undoStack()->redo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);
        QVERIFY(lastBarcode(*ed->doc()));
    }

    // A picture of the window with a QR code on the slide, for the docs, when JP_SHOT_DIR names a folder.
    void aPictureOfTheWindowForTheDocs()
    {
        if (qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) return;
        DeckWindow w;
        w.resize(1300, 820);
        w.show();
        insertQr(w, QStringLiteral("https://jeffoffice.com"));
        QCoreApplication::processEvents();
        w.grab().save(qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/deck-qr.png"));
    }

    // With the barcode selected, Insert > Barcode edits it: the dialog starts from its data, and the new barcode takes its place, one step back.
    void editingReopensTheDialogWithItsData()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        insertQr(w, QStringLiteral("first"));
        GroupItem *g = lastBarcode(*ed->doc());
        QVERIFY(g);
        g->moveBy(-100, 40);
        const QString id = g->id;
        const QPointF where = g->rect.topLeft();
        ed->select(id);
        withDialog([&] { w.act(QStringLiteral("insert.barcode"))->trigger(); }, QStringLiteral("barcodeDialog"), [&](QDialog *d) {
            QCOMPARE(d->windowTitle(), QStringLiteral("Edit Barcode"));
            auto *line = d->findChildren<QLineEdit *>().first();
            QCOMPARE(line->text(), QStringLiteral("first"));
            line->setText(QStringLiteral("second"));
        });
        GroupItem *now = lastBarcode(*ed->doc());
        QVERIFY(now);
        QCOMPARE(now->id, id);
        QCOMPARE(now->rect.topLeft(), where);
        QCOMPARE(now->barcode.value(QLatin1String("data")).toString(), QStringLiteral("second"));
        ed->undoStack()->undo();
        QCOMPARE(lastBarcode(*ed->doc())->barcode.value(QLatin1String("data")).toString(), QStringLiteral("first"));
    }

    // Saved, a barcode is a picture of its own (the SVG, with a PNG for programs that don't draw SVG) in the slide's package, with the marker that
    // carries its data. Opened again, it is the same barcode, in the same place and at the size it was given, and can be edited.
    void aSavedBarcodeIsAPictureAndReopensAsABarcode()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        insertQr(w, QStringLiteral("https://jeffoffice.com/round-trip"));
        GroupItem *g = lastBarcode(*ed->doc());
        QVERIFY(g);
        // Moved and sized: 100 points wide.
        g->scaleInto(g->rect, QRectF(50, 60, 100, 100));
        g->rotation = 0;
        const QJsonObject settings = g->barcode;
        const QString description = g->altText;
        const int objects = int(ed->doc()->pages[0]->items.size());

        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*ed->doc(), &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(skipped.isEmpty(), qPrintable(skipped.join('\n')));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString slidePart = QStringLiteral("/ppt/slides/slide1.xml");
        const QString slide = part(e, slidePart);
        // Another program sees a picture, with the alternative text, and nothing it would draw as a pile of shapes.
        QVERIFY(slide.contains(QStringLiteral("<p:pic>")));
        QVERIFY(slide.contains(QStringLiteral("descr=\"Barcode: QR code https://jeffoffice.com/round-trip\"")));
        QVERIFY(slide.contains(QStringLiteral("jeffoffice.com/ns/barcode")));
        QString svgTarget, pngTarget;
        for (const QString &target : relationshipTargets(e, slidePart, QStringLiteral("/image"))) {
            if (target.endsWith(QLatin1String(".svg"))) svgTarget = target;
            if (target.endsWith(QLatin1String(".png"))) pngTarget = target;
        }
        QVERIFY(!svgTarget.isEmpty() && !pngTarget.isEmpty());
        QCOMPARE(e.pkg.contentType(svgTarget), QStringLiteral("image/svg+xml"));
        QCOMPARE(e.pkg.contentType(pngTarget), QStringLiteral("image/png"));
        QVERIFY(e.zip.value(svgTarget.mid(1)).contains("<svg"));
        QVERIFY(e.zip.value(pngTarget.mid(1)).startsWith("\x89PNG"));
        QVERIFY(slide.contains(QStringLiteral("<asvg:svgBlip ")));

        // Opened again: the same barcode.
        auto reopened = deck::importPresentation(bytes, &err);
        QVERIFY2(reopened, qPrintable(err));
        QCOMPARE(int(reopened->pages[0]->items.size()), objects);
        GroupItem *back = lastBarcode(*reopened);
        QVERIFY(back);
        QCOMPARE(back->barcode, settings);
        QCOMPARE(back->altText, description);
        QVERIFY(qAbs(back->rect.x() - 50) < 0.2 && qAbs(back->rect.y() - 60) < 0.2 && qAbs(back->rect.width() - 100) < 0.2 && qAbs(back->rect.height() - 100) < 0.2);
        // And saved a second time, it is the same file's worth of barcode.
        const QByteArray again = deck::exportPresentation(*reopened, &err);
        QVERIFY2(!again.isEmpty(), qPrintable(err));
        Exported e2;
        QVERIFY2(unpack(again, &e2, &err), qPrintable(err));
        QCOMPARE(part(e2, slidePart).count(QStringLiteral("jeffoffice.com/ns/barcode")), 1);
    }

    // In an open document presentation a barcode is a picture frame (a PNG) with the settings in an attribute of the frame; opened again, it is the same barcode.
    void inAnOpenDocumentFileItIsAPictureWithTheSettingsInTheFrame()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        insertQr(w, QStringLiteral("https://jeffoffice.com/odp"));
        GroupItem *g = lastBarcode(*ed->doc());
        QVERIFY(g);
        g->scaleInto(g->rect, QRectF(40, 50, 120, 120));
        const QJsonObject settings = g->barcode;
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportOdp(*ed->doc(), false, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> parts;
        QVERIFY(readZip(bytes, parts, &err));
        const QString content = QString::fromUtf8(parts.value(QStringLiteral("content.xml")));
        QVERIFY(content.contains(QStringLiteral("jo:barcode=")));
        QVERIFY(content.contains(QStringLiteral("<draw:image ")));
        bool png = false;
        for (auto it = parts.cbegin(); it != parts.cend(); ++it) png = png || (it.key().startsWith(QLatin1String("Pictures/")) && it.value().startsWith("\x89PNG"));
        QVERIFY(png);
        auto back = deck::importOdp(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        GroupItem *again = lastBarcode(*back);
        QVERIFY(again);
        QCOMPARE(again->barcode, settings);
        QVERIFY(qAbs(again->rect.x() - 40) < 0.5 && qAbs(again->rect.width() - 120) < 0.5);
    }

    // A marker that holds no barcode the encoder can draw (the data is gone) leaves the picture as an ordinary picture.
    void aMarkerWithNoBarcodeInItIsJustAPicture()
    {
        DeckWindow w;
        insertQr(w, QStringLiteral("x"));
        QString err;
        const QByteArray bytes = deck::exportPresentation(*w.editor()->doc(), &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(slide.contains(QStringLiteral("&quot;data&quot;:&quot;x&quot;")) || slide.contains(QStringLiteral("\"data\":\"x\"")));
        slide.replace(QStringLiteral("&quot;data&quot;:&quot;x&quot;"), QStringLiteral("&quot;data&quot;:&quot;&quot;"));
        e.zip[QStringLiteral("ppt/slides/slide1.xml")] = slide.toUtf8();
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        QByteArray tampered;
        {
            ZipWriter zip;
            for (auto it = e.zip.cbegin(); it != e.zip.cend(); ++it) zip.add(it.key(), it.value(), true);
            tampered = zip.finish();
        }
        auto d = deck::importPresentation(tampered, &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(!lastBarcode(*d));
        QVERIFY(firstPicture(*d));
    }
};

JP_TEST_CLASS(DeckBarcodeTests)
#include "tst_deck_barcode.moc"
