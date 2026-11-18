// Tests of the shared pieces of objects between apps (libs/shell/app/objects.h): the clipboard form, the text of copied cells, and the kinds' words.
// Every file is made here in code.
#include "app/objects.h"
#include "core/fonts.h"
#include "io/zip.h"
#include "office/package.h"
#include "../testrunner.h"

#include <QElapsedTimer>
#include <QMimeData>
#include <QRandomGenerator>
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

// A package with one more part of `bytes` (a package that is small to hold and large once unpacked, when the part is plain).
QByteArray withPart(const QByteArray &zip, const QString &name, const QByteArray &bytes, bool compress)
{
    QMap<QString, QByteArray> parts;
    readZip(zip, parts);
    ZipWriter zw;
    if (parts.contains(QStringLiteral("[Content_Types].xml"))) zw.add(QStringLiteral("[Content_Types].xml"), parts.value(QStringLiteral("[Content_Types].xml")), true);
    for (auto it = parts.cbegin(); it != parts.cend(); ++it)
        if (it.key() != QLatin1String("[Content_Types].xml")) zw.add(it.key(), it.value(), true);
    zw.add(name, bytes, compress);
    return zw.finish();
}

QByteArray padding(qint64 size) { return QByteArray(int(size), ' '); }

QByteArray chartPackage()
{
    const auto chart = chartObjectFromPart(QByteArrayLiteral("<c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"><c:chart><c:plotArea/></c:chart></c:chartSpace>"));
    return chart ? chart->partBytes : QByteArray();
}

} // namespace

class PubSharedObjectTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { initCore(); }

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

    void aSiblingProgramIsLookedForWhereThePlatformKeepsIt()
    {
        QCOMPARE(siblingPaths(QStringLiteral("/opt/jo"), QStringLiteral("JeffWrite"), Platform::Linux), (QStringList{QStringLiteral("/opt/jo/JeffWrite"), QStringLiteral("/opt/jo/jeffwrite")}));
        QCOMPARE(siblingPaths(QStringLiteral("C:/Program Files/JeffOffice"), QStringLiteral("JeffCrunch"), Platform::Windows), (QStringList{QStringLiteral("C:/Program Files/JeffOffice/JeffCrunch.exe")}));
        // (An app of the Mac is a bundle: its program is three folders in, and the other app's bundle is beside this one's.)
        QCOMPARE(siblingPaths(QStringLiteral("/Applications/JeffDeck.app/Contents/MacOS"), QStringLiteral("JeffWrite"), Platform::Mac),
                 (QStringList{QStringLiteral("/Applications/JeffWrite.app/Contents/MacOS/JeffWrite")}));
    }

    void aCopyKeepsTheOtherFormatsLazyAndWhole()
    {
        struct Counting : QMimeData {
            explicit Counting(int *asked) : pngAsked(asked) {}
            int *pngAsked;
            QStringList formats() const override { return QStringList{QStringLiteral("text/plain"), QStringLiteral("image/png")}; }
            bool hasFormat(const QString &f) const override { return f == QLatin1String("text/plain") || f == QLatin1String("image/png"); }
            QVariant retrieveData(const QString &f, QMetaType) const override
            {
                if (f == QLatin1String("image/png")) {
                    ++*pngAsked;
                    return QByteArray("png bytes");
                }
                return f == QLatin1String("text/plain") ? QVariant(QByteArray("words")) : QVariant();
            }
        };
        int pngAsked = 0;
        QMimeData *mime = withObject(new Counting(&pngAsked), []() -> std::optional<ole::EmbeddedObject> { return std::nullopt; });
        QCOMPARE(pngAsked, 0);   // (a copy does not ask for a picture to be encoded)
        QCOMPARE(mime->text(), QStringLiteral("words"));
        QCOMPARE(pngAsked, 0);
        QVERIFY(mime->hasFormat(QStringLiteral("image/png")));
        QVERIFY(hasObject(mime));
        QCOMPARE(mime->data(QStringLiteral("image/png")), QByteArray("png bytes"));
        QCOMPARE(pngAsked, 1);
        delete mime;
        // An image on the copy is still an image on the wrapper.
        auto *withImage = new QMimeData;
        QImage image(40, 30, QImage::Format_RGB32);
        image.fill(Qt::red);
        withImage->setImageData(image);
        QMimeData *wrapped = withObject(withImage, []() -> std::optional<ole::EmbeddedObject> { return std::nullopt; });
        QVERIFY(wrapped->hasImage());
        QCOMPARE(qvariant_cast<QImage>(wrapped->imageData()).size(), image.size());
        delete wrapped;
    }

    void aPackageOfAFileCannotBlowUpWhatIsDrawnFromIt()
    {
        QElapsedTimer clock;
        clock.start();
        // A workbook, a chart's workbook, and a document: each valid, small to hold, and 100 MB once unpacked (more than a picture of a range, chart, or page ever needs).
        const QByteArray book = withPart(rangePackage(twoByTwo()), QStringLiteral("xl/padding.xml"), padding(100 << 20), true);
        const QByteArray chart = withPart(chartPackage(), QStringLiteral("xl/padding.xml"), padding(100 << 20), true);
        const QByteArray docx = withPart(docxOf({QStringLiteral("Words")}), QStringLiteral("word/padding.xml"), padding(100 << 20), true);
        QVERIFY(book.size() < (1 << 20) && chart.size() < (1 << 20) && docx.size() < (1 << 20));
        Grid grid;
        QString why;
        QVERIFY(!readRange(book, QString(), &grid, &why));
        QVERIFY(sheetPicture(chart, 4000000, 3000000, 2).isNull());
        QVERIFY(!chartObject(chart, &why));
        QCOMPARE(pageCountOf(docx), 0);
        QVERIFY(pagePicture(docx, 4000000, 3000000, 2).isNull());
        QVERIFY(!pageObject(docx, 1, &why));
        QVERIFY(!why.isEmpty());
        // (an object read back from the clipboard may hold that much, as a file's may, but no picture is drawn from it)
        ole::EmbeddedObject big = *rangeObject(twoByTwo());
        big.partBytes = book;
        const auto back = decode(encode(big));
        QVERIFY(!back || !readRange(back->partBytes, QString(), &grid, &why));
        QVERIFY(clock.elapsed() < 20000);   // (a loose sanity bound only: the limits above are what refuse)
    }

    void aDocumentOverTheCapIsNotLaidOutForAPicture()
    {
        // A document of 17 MB (an entry that does not deflate), small enough to unpack: refused for its size alone.
        QByteArray noise(17 << 20, 0);
        QRandomGenerator rng(7);
        rng.fillRange(reinterpret_cast<quint32 *>(noise.data()), noise.size() / 4);
        const QByteArray docx = withPart(docxOf({QStringLiteral("Words")}), QStringLiteral("word/media/noise.bin"), noise, false);
        QVERIFY(docx.size() > (16 << 20));
        QCOMPARE(pageCountOf(docx), 0);
        QString why;
        QVERIFY(!pageObject(docx, 1, &why));
        QVERIFY(why.contains(QStringLiteral("large")));
        // The same document with less in it is read.
        const QByteArray small = withPart(docxOf({QStringLiteral("Words")}), QStringLiteral("word/media/noise.bin"), noise.left(1 << 20), false);
        QCOMPARE(pageCountOf(small), 1);
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
