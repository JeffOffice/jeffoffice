// Tests of the shared pieces of objects between apps (libs/shell/app/objects.h): the picture that carries an object, the clipboard form, the text of copied cells, and the kinds' words.
// Every file is made here in code.
#include "app/objects.h"
#include "core/fonts.h"
#include "io/zip.h"
#include "office/package.h"
#include "../testrunner.h"

#include <QBuffer>
#include <QMimeData>
#include <QtTest>

using namespace jp;
using namespace jp::objects;
namespace ole = jp::office::ole;

namespace {

objects::Grid twoByTwo()
{
    objects::Grid grid;
    for (int r = 0; r < 2; ++r) {
        QVector<objects::CellValue> row;
        for (int c = 0; c < 2; ++c) {
            objects::CellValue v;
            v.text = QString::number(r * 2 + c + 1);
            v.number = true;
            v.value = r * 2 + c + 1;
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
              QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body>") + body + "</w:body></w:document>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    return w.finish();
}

} // namespace

class PubSharedObjectTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { initCore(); }

    void aCarriedObjectComesBackWithItsToken()
    {
        const auto object = rangeObject(twoByTwo());
        QVERIFY(object);
        const QByteArray png = carryObject(*object, QStringLiteral("token-1"));
        QVERIFY(!png.isEmpty());
        QVERIFY(QImage::fromData(png, "PNG").width() > 0);   // (still a picture to a program that does not know the object)
        const auto back = carriedObject(png);
        QVERIFY(back);
        QCOMPARE(back->token, QStringLiteral("token-1"));
        QCOMPARE(back->object.partBytes, object->partBytes);
        QCOMPARE(back->object.previewPng, object->previewPng);
        QVERIFY(back->object.isOurs);
        // A plain picture carries nothing.
        QVERIFY(!carriedObject(object->previewPng));
    }

    void aDamagedCarrierIsNotAnObject()
    {
        const auto object = rangeObject(twoByTwo());
        QVERIFY(object);
        const QByteArray png = carryObject(*object, QStringLiteral("t"));
        QByteArray bad = png;
        bad[60] = char(bad[60] ^ 0x55);   // (inside the chunk: its checksum fails)
        QVERIFY(!carriedObject(bad));
        QVERIFY(!carriedObject(png.left(png.size() / 2)));
        QVERIFY(!carriedObject(QByteArray("not a picture at all, nothing like one")));
        // (a package that is not a safe package is kept but is not one of ours to open: the chunk holds bytes of no zip)
        ole::EmbeddedObject junk = *object;
        junk.partBytes = QByteArray(200, 'x');
        const auto kept = carriedObject(carryObject(junk, QStringLiteral("t")));
        QVERIFY(!kept || !kept->object.isOurs);
    }

    void theClipboardFormCarriesTheObjectAndIsMadeWhenAsked()
    {
        int made = 0;
        auto *mime = new QMimeData;
        mime->setText(QStringLiteral("words"));
        mime = withObject(mime, [&made]() -> std::optional<ole::EmbeddedObject> {
            ++made;
            return rangeObject(twoByTwo());
        });
        QCOMPARE(made, 0);   // (a copy alone does not make it)
        QVERIFY(hasObject(mime));
        QCOMPARE(mime->text(), QStringLiteral("words"));
        const auto object = objectOf(mime);
        QVERIFY(object);
        QCOMPARE(made, 1);
        QCOMPARE(kindWords(*object), QStringLiteral("Spreadsheet range"));
        QVERIFY(!decode(QByteArray("JOOBJ1 and more")));
        delete mime;
    }

    void aPagePictureShowsTheWordsOfItsPage()
    {
        QStringList many;
        for (int i = 0; i < 120; ++i) many << QStringLiteral("Line %1 of a long document that fills several pages").arg(i);
        const QByteArray docx = docxOf(many);
        const int pages = pageCountOf(docx);
        QVERIFY(pages >= 2);
        const auto first = pageObject(docx, 1);
        const auto second = pageObject(docx, 2);
        QVERIFY(first && second);
        QVERIFY(!pageObject(docx, pages + 1));
        auto inkOf = [](const QByteArray &png) {
            const QImage image = QImage::fromData(png, "PNG").convertToFormat(QImage::Format_RGB32);
            int dark = 0;
            for (int y = 2; y < image.height() - 2; ++y)
                for (int x = 2; x < image.width() - 2; ++x) dark += qGray(image.pixel(x, y)) < 100;
            return dark;
        };
        QVERIFY(inkOf(first->previewPng) > 50);    // (the words are drawn)
        QVERIFY(inkOf(second->previewPng) > 50);
        QVERIFY(first->previewPng != second->previewPng);   // (the page asked for is the page shown)
    }

    void copiedTextBecomesAGridWithinTheCaps()
    {
        const Grid small = gridFromText(QStringLiteral("a\t\"b c\"\t3\n1\t2\t3\n"));
        QCOMPARE(small.size(), 2);
        QCOMPARE(small[0].size(), 3);
        QCOMPARE(small[0][1].text, QStringLiteral("b c"));
        QVERIFY(small[0][2].number);
        QVERIFY(!small[0][0].number);
        QString big;
        for (int r = 0; r < kMostRows + 50; ++r) {
            for (int c = 0; c < kMostColumns + 10; ++c) big += (c ? QStringLiteral("\t") : QString()) + QStringLiteral("x");
            big += QLatin1Char('\n');
        }
        const Grid capped = gridFromText(big);
        QCOMPARE(capped.size(), int(kMostRows));
        QCOMPARE(capped[0].size(), int(kMostColumns));
    }

    void aRangeFromAWorkbookReadsTheCellsAsTheyShow()
    {
        const auto object = rangeObject(gridFromText(QStringLiteral("name\ttotal\nnorth\t12\nsouth\t30\n")));
        QVERIFY(object);
        Grid grid;
        QString why;
        QVERIFY(readRange(object->partBytes, QStringLiteral("B2:B3"), &grid, &why));
        QCOMPARE(grid.size(), 2);
        QCOMPARE(grid[1][0].text, QStringLiteral("30"));
        QVERIFY(!readRange(object->partBytes, QStringLiteral("nonsense"), &grid, &why));
        QVERIFY(!why.isEmpty());
        QVERIFY(!readRange(QByteArray("not a workbook"), QString(), &grid, &why));
    }
};

JP_TEST_CLASS(PubSharedObjectTests)
#include "tst_pub_sharedobjects.moc"
