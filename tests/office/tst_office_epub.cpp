// Tests of libs/office/epub: the package every app's EPUB is made in.
#include "officehelpers.h"
#include "epub/epub.h"
#include "io/zip.h"
#include <QXmlStreamReader>

using namespace jp;
using namespace jp::office;

class OfficeEpubTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    // The type is the first entry and stored; the container names the package; the package lists every file and the spine every chapter.
    void aBookIsAZipWithItsTypeFirstAndItsPartsListed()
    {
        epub::Book book;
        book.title = QStringLiteral("T & U");
        book.author = QStringLiteral("Pat");
        book.language = QStringLiteral("en-US");
        book.identifier = QStringLiteral("urn:uuid:1");
        book.chapters << epub::Chapter{QStringLiteral("One"), QStringLiteral("<h1 id=\"h1\">One</h1>\n")} << epub::Chapter{QString(), QStringLiteral("<p>x</p>\n")};
        book.resources << epub::Resource{QStringLiteral("images/pic1.png"), QStringLiteral("image/png"), QByteArray("png")};
        const QByteArray bytes = epub::build(book);
        QCOMPARE(bytes.mid(30, 28), QByteArray("mimetypeapplication/epub+zip"));
        QCOMPARE(int(bytes[8]), 0);
        QMap<QString, QByteArray> z;
        QVERIFY(readZip(bytes, z));
        QVERIFY(z.value(QStringLiteral("META-INF/container.xml")).contains("full-path=\"OEBPS/content.opf\""));
        const QString opf = QString::fromUtf8(z.value(QStringLiteral("OEBPS/content.opf")));
        QVERIFY(opf.contains(QStringLiteral("<dc:title>T &amp; U</dc:title>")) && opf.contains(QStringLiteral("<dc:identifier id=\"bookid\">urn:uuid:1</dc:identifier>")));
        QVERIFY(opf.contains(QStringLiteral("<itemref idref=\"c2\"/>")) && opf.contains(QStringLiteral("href=\"images/pic1.png\"")));
        QVERIFY(opf.contains(QStringLiteral("<spine>")));   // (no NCX unless asked)
        QVERIFY(!z.contains(QStringLiteral("OEBPS/toc.ncx")));
        QVERIFY(QString::fromUtf8(z.value(QStringLiteral("OEBPS/chapter2.xhtml"))).contains(QStringLiteral("<title>Part 2</title>")));
        QVERIFY(QString::fromUtf8(z.value(QStringLiteral("OEBPS/nav.xhtml"))).contains(QStringLiteral("<a href=\"chapter1.xhtml\">T &amp; U</a>")));   // (no contents given: the book itself)
    }

    // The contents are one list however the headings begin (a Heading 2 before the first Heading 1), and the list of landmarks is hidden.
    void theContentsAreOneListAndTheLandmarksAreHidden()
    {
        epub::Book book;
        book.title = QStringLiteral("T");
        book.language = QStringLiteral("en");
        book.chapters << epub::Chapter{QString(), QString()};
        book.nav << epub::NavEntry{2, QStringLiteral("B"), QStringLiteral("chapter1.xhtml#h1")} << epub::NavEntry{1, QStringLiteral("A"), QStringLiteral("chapter1.xhtml#h2")}
                 << epub::NavEntry{3, QStringLiteral("C"), QStringLiteral("chapter1.xhtml#h3")};
        QMap<QString, QByteArray> z;
        QVERIFY(readZip(epub::build(book), z));
        QXmlStreamReader r(z.value(QStringLiteral("OEBPS/nav.xhtml")));
        int depth = 0, toc = 0, topLists = 0;
        bool inToc = false;
        while (!r.atEnd()) {
            r.readNext();
            if (r.isStartElement()) {
                ++depth;
                if (r.name() == QLatin1String("nav")) inToc = r.attributes().value(QStringLiteral("http://www.idpf.org/2007/ops"), QStringLiteral("type")) == QLatin1String("toc");
                if (inToc && r.name() == QLatin1String("ol") && depth == 4) ++topLists;   // html, body, nav, ol
                if (inToc) toc += r.name() == QLatin1String("a");
            } else if (r.isEndElement()) {
                --depth;
            }
        }
        QVERIFY2(!r.hasError(), qPrintable(r.errorString()));
        QCOMPARE(topLists, 1);
        QCOMPARE(toc, 3);
        QVERIFY(QString::fromUtf8(z.value(QStringLiteral("OEBPS/nav.xhtml"))).contains(QStringLiteral("<nav epub:type=\"landmarks\" hidden=\"hidden\">")));
    }

    // The contents nest by level in the navigation document and, when asked, in an NCX that the spine names; a level past the deepest is cut to it.
    void theContentsAreANavDocumentAndAnNcxForOlderReaders()
    {
        epub::Book book;
        book.title = QStringLiteral("T");
        book.language = QStringLiteral("en");
        book.ncx = true;
        book.chapters << epub::Chapter{QString(), QString()};
        book.nav << epub::NavEntry{1, QStringLiteral("A"), QStringLiteral("chapter1.xhtml")} << epub::NavEntry{2, QStringLiteral("B"), QStringLiteral("chapter1.xhtml#h2")}
                 << epub::NavEntry{99, QStringLiteral("C"), QStringLiteral("chapter1.xhtml#h3")} << epub::NavEntry{1, QStringLiteral("D"), QStringLiteral("chapter1.xhtml#h4")};
        QMap<QString, QByteArray> z;
        QVERIFY(readZip(epub::build(book), z));
        const QString nav = QString::fromUtf8(z.value(QStringLiteral("OEBPS/nav.xhtml")));
        QVERIFY2(nav.contains(QStringLiteral("<li><a href=\"chapter1.xhtml\">A</a><ol>\n<li><a href=\"chapter1.xhtml#h2\">B</a><ol>\n<li><a href=\"chapter1.xhtml#h3\">C</a></li>")), qPrintable(nav));
        const QString ncx = QString::fromUtf8(z.value(QStringLiteral("OEBPS/toc.ncx")));
        QVERIFY(ncx.contains(QStringLiteral("<navPoint id=\"n1\" playOrder=\"1\"><navLabel><text>A</text></navLabel><content src=\"chapter1.xhtml\"/>")));
        QVERIFY(ncx.contains(QStringLiteral("playOrder=\"4\"><navLabel><text>D</text>")) && ncx.contains(QStringLiteral("name=\"dtb:depth\" content=\"3\"")));
        const QString opf = QString::fromUtf8(z.value(QStringLiteral("OEBPS/content.opf")));
        QVERIFY(opf.contains(QStringLiteral("<spine toc=\"ncx\">")) && opf.contains(QStringLiteral("media-type=\"application/x-dtbncx+xml\"")));
        QXmlStreamReader r(z.value(QStringLiteral("OEBPS/toc.ncx")));
        while (!r.atEnd()) r.readNext();
        QVERIFY2(!r.hasError(), qPrintable(r.errorString()));
    }
};

JP_TEST_CLASS(OfficeEpubTests)
#include "tst_office_epub.moc"
