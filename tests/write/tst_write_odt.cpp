// Tests of JeffWrite opening and saving OpenDocument text (.odt): the window's File > Open and Save As, and the document's reader and writer. Every file is built here from a few lines of XML.

#include "../isolatesettings.h"
#include "../stablesaves.h"
#include "app/pageview.h"
#include "app/settings.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/fieldops.h"
#include "core/fonts.h"
#include "core/pictureops.h"
#include "core/rulerops.h"
#include "flow/flowdocument.h"
#include "core/styleops.h"
#include "core/listpresets.h"
#include "core/writeprops.h"
#include "core/tableops.h"
#include "core/writedoc.h"
#include "odt/odtread.h"
#include "io/zip.h"
#include "odt/odt.h"
#include "office/package.h"
#include "office/mathobject.h"
#include "wml/wmlmathedit.h"
#include "wml/wmlmathpath.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include <QBuffer>
#include <QImage>
#include <QRegularExpression>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

using namespace jp;
using namespace jp::write;

namespace {

// A document of another program with a graphic (an inline picture whose blip names a PNG and holds `svg` in its svgBlip).
QByteArray foreignGraphicDocx(const QByteArray &svg)
{
    QByteArray png;
    {
        QImage image(40, 40, QImage::Format_RGB32);
        image.fill(Qt::red);
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
    }
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    const QByteArray run = "<w:r><w:drawing><wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"><wp:extent cx=\"1270000\" cy=\"1270000\"/><wp:docPr id=\"1\" name=\"g\"/><wp:cNvGraphicFramePr/>"
                           "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:pic xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">"
                           "<pic:nvPicPr><pic:cNvPr id=\"0\" name=\"g\"/><pic:cNvPicPr/></pic:nvPicPr><pic:blipFill><a:blip r:embed=\"rIdP\"><a:extLst><a:ext uri=\"{96DAC541-7B7A-43D3-8B79-37D633B846F1}\">"
                           "<asvg:svgBlip xmlns:asvg=\"http://schemas.microsoft.com/office/drawing/2016/SVG/main\" r:embed=\"rIdS\"/></a:ext></a:extLst></a:blip><a:stretch><a:fillRect/></a:stretch>"
                           "</pic:blipFill><pic:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1270000\" cy=\"1270000\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></pic:spPr></pic:pic>"
                           "</a:graphicData></a:graphic></wp:inline></w:drawing></w:r>";
    w.addPart(QStringLiteral("/word/document.xml"),
              QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" "
                         "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" "
                         "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"><w:body><w:p><w:r><w:t xml:space=\"preserve\">Before </w:t></w:r>") + run + "</w:p></w:body></w:document>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addPart(QStringLiteral("/word/media/image1.png"), png, QStringLiteral("image/png"));
    w.addPart(QStringLiteral("/word/media/image2.svg"), svg, QStringLiteral("image/svg+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdP"), rel + QStringLiteral("image"), QStringLiteral("media/image1.png"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdS"), rel + QStringLiteral("image"), QStringLiteral("media/image2.svg"));
    return w.finish();
}

// The pictures of the .odt a document of another program is saved as: the SVG of its graphic, or its PNG.
QStringList odtPicturesOf(const QByteArray &docx)
{
    QString err;
    auto doc = WriteDoc::fromDocx(docx, &err);
    if (!doc) return {};
    const QByteArray odt = doc->toOdt(&err);
    QMap<QString, QByteArray> parts;
    QStringList out;
    if (odt.isEmpty() || !jp::readZip(odt, parts)) return out;
    for (auto it = parts.constBegin(); it != parts.constEnd(); ++it)
        if (it.key().startsWith(QLatin1String("Pictures/"))) out << it.key().section(QLatin1Char('.'), -1);
    return out;
}

const char kNs[] =
    " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""
    " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\""
    " xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\""
    " xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"";

QByteArray makeOdt(const QByteArray &body, const QByteArray &autoStyles = QByteArray(), const QByteArray &commonStyles = QByteArray())
{
    jp::ZipWriter z;
    z.add(QStringLiteral("mimetype"), QByteArray("application/vnd.oasis.opendocument.text"), false);
    z.add(QStringLiteral("content.xml"),
          QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-content") + kNs + " office:version=\"1.2\"><office:automatic-styles>" + autoStyles +
              "</office:automatic-styles><office:body><office:text>" + body + "</office:text></office:body></office:document-content>",
          true);
    z.add(QStringLiteral("styles.xml"),
          QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-styles") + kNs + " office:version=\"1.2\"><office:styles>" + commonStyles + "<style:style style:name=\"Standard\" style:family=\"paragraph\"/>"
              "<style:style style:name=\"Heading_20_1\" style:display-name=\"Heading 1\" style:family=\"paragraph\" style:default-outline-level=\"1\"><style:text-properties fo:font-size=\"20pt\"/></style:style></office:styles>"
              "<office:automatic-styles/><office:master-styles/></office:document-styles>",
          true);
    return z.finish();
}

const QByteArray kBody =
    "<text:h text:style-name=\"Heading_20_1\" text:outline-level=\"1\">A heading</text:h><text:p>Plain <text:span text:style-name=\"T1\">bold words</text:span> and more.</text:p>"
    "<text:list text:style-name=\"L1\"><text:list-item><text:p>first item</text:p></text:list-item><text:list-item><text:p>second item</text:p></text:list-item></text:list>"
    "<table:table><table:table-column table:number-columns-repeated=\"2\"/><table:table-row><table:table-cell><text:p>cell a</text:p></table:table-cell><table:table-cell><text:p>cell b</text:p></table:table-cell></table:table-row></table:table>"
    "<text:p>Last paragraph.</text:p>";
const QByteArray kAuto = "<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties fo:font-weight=\"bold\"/></style:style>"
                         "<text:list-style style:name=\"L1\"><text:list-level-style-number text:level=\"1\" style:num-format=\"1\" style:num-suffix=\".\"/></text:list-style>";

bool hasBold(const WriteDoc &d, const QString &text)
{
    for (QTextBlock b = d.story()->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it)
            if (it.fragment().text().contains(text) && it.fragment().charFormat().fontWeight() > 500) return true;
    return false;
}

QString writeFile(const QString &path, const QByteArray &bytes)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size()) return QString();
    return path;
}

QByteArray readAll(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

} // namespace

class WriteOdtTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    // The run's settings are its own, and none reached the person's (checked at the end).
    void cleanupTestCase() { QVERIFY(jptest::settingsStayInTheRunsFolder()); }

    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        initCore();
    }

    // An icon's line color, which the document names by the theme's color and the changes to it (luminance, tint), is written as the color those make; the theme part is read once for all.
    void anIconsThemeColorAndItsChangesAreWrittenAsTheColorTheyMake()
    {
        auto doc = WriteDoc::blank();
        QVERIFY(doc->theme());
        QTextCursor c(doc->story());
        QCOMPARE(insertIcon(*doc, c, QStringLiteral("star")), PictureResult::Ok);
        QByteArray m_stroke;
        auto strokeOf = [&](const QByteArray &colorXml) {
            QMap<QString, QByteArray> parts;
            QVERIFY(readZip(doc->toDocx(), parts));
            QByteArray x = parts.value(QStringLiteral("word/document.xml"));
            if (x.isEmpty()) x = parts.value(QStringLiteral("/word/document.xml"));
            const QByteArray plain = "<a:schemeClr val=\"tx1\"/>";
            QVERIFY(x.contains(plain));
            x.replace(plain, colorXml);
            const QString key = parts.contains(QStringLiteral("word/document.xml")) ? QStringLiteral("word/document.xml") : QStringLiteral("/word/document.xml");
            parts[key] = x;
            ZipWriter z;
            for (auto it = parts.constBegin(); it != parts.constEnd(); ++it) z.add(it.key(), it.value(), true);
            auto changed = WriteDoc::fromDocx(z.finish());
            QVERIFY(changed);
            const QByteArray content = odtPart(changed->toOdt(), QStringLiteral("content.xml"));
            const qsizetype at = content.indexOf("svg:stroke-color=\"#");
            QVERIFY(at >= 0);
            m_stroke = content.mid(at + 19, 6).toUpper();
        };
        strokeOf("<a:schemeClr val=\"tx1\"/>");
        QCOMPARE(m_stroke, doc->theme()->colors[office::Theme::Dark1].name(QColor::HexRgb).mid(1).toUpper().toLatin1());
        strokeOf("<a:schemeClr val=\"tx1\"><a:lumMod val=\"50000\"/><a:lumOff val=\"50000\"/></a:schemeClr>");   // (black, half way to white)
        QCOMPARE(m_stroke, QByteArray("7790B3"));   // (the theme's dark 1, with its luminance halfway to white)
        strokeOf("<a:schemeClr val=\"tx1\"><a:tint val=\"50000\"/></a:schemeClr>");   // (black, half way to white)
        QCOMPARE(m_stroke, QByteArray("8F949B"));   // (the same, half way to white)
        strokeOf("<a:schemeClr val=\"tx1\"><a:shade val=\"50000\"/></a:schemeClr>");   // (half as bright: toward black)
        {
            const QColor base = doc->theme()->colors[office::Theme::Dark1], got(QStringLiteral("#") + QString::fromLatin1(m_stroke));
            QVERIFY(got.isValid());
            QVERIFY2(qAbs(got.red() - base.red() / 2) <= 1 && qAbs(got.green() - base.green() / 2) <= 1 && qAbs(got.blue() - base.blue() / 2) <= 1, m_stroke.constData());
        }
    }

    void anOdtOpensWithItsTextAndLooks()
    {
        QString err;
        QStringList notes;
        auto doc = WriteDoc::fromFile(makeOdt(kBody, kAuto), &err, &notes);
        QVERIFY2(doc, qPrintable(err));
        QVERIFY(notes.isEmpty());
        const QString text = doc->story()->toPlainText();
        for (const char *w : {"A heading", "Plain bold words and more.", "first item", "second item", "cell a", "cell b", "Last paragraph."}) QVERIFY2(text.contains(QLatin1String(w)), w);
        QVERIFY(hasBold(*doc, QStringLiteral("bold words")));
        QVERIFY(!hasBold(*doc, QStringLiteral("Plain")));
    }

    // The distance of the default tab stops, character scaling, suppressed line numbers, and bar stops come in from an .odt, are drawn (the first two) or kept as the document's, and go out again.
    void theDefaultTabsScalingLineNumbersAndBarStopsOfAnOdtSurviveAnOpenAndASave()
    {
        const QByteArray auto_ = "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties xmlns:jo=\"urn:jeffoffice:odf-extension\" text:number-lines=\"false\" jo:bar-tabs=\"1440\"/></style:style>"
                                 "<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties style:text-scale=\"200%\"/></style:style>";
        const QByteArray body = "<text:p text:style-name=\"P1\">a<text:span text:style-name=\"T1\">wide</text:span></text:p><text:p>plain</text:p>";
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt(body, auto_, "<style:default-style style:family=\"paragraph\"><style:paragraph-properties style:tab-stop-distance=\"0.25in\"/></style:default-style>"), &err);
        QVERIFY2(doc, qPrintable(err));
        auto check = [](const WriteDoc &d) {
            QCOMPARE(d.defaultTabStop(), 360);
            const QTextBlock first = d.story()->begin();
            QVERIFY(first.blockFormat().boolProperty(flow::fp::SuppressLineNumbers));
            QVERIFY(!first.next().blockFormat().boolProperty(flow::fp::SuppressLineNumbers));
            const QVector<TabStopEntry> stops = tabStopsOf(first.blockFormat());
            QCOMPARE(stops.size(), 1);
            QVERIFY(stops[0].bar);
            QCOMPARE(stops[0].position, 72.0);
            bool scaled = false;
            for (auto it = first.begin(); !it.atEnd(); ++it) scaled = scaled || (it.fragment().text() == QLatin1String("wide") && it.fragment().charFormat().fontStretch() == 200);
            QVERIFY(scaled);
        };
        check(*doc);
        const QByteArray odt = doc->toOdt();
        const QByteArray content = odtPart(odt, QStringLiteral("content.xml")), styles = odtPart(odt, QStringLiteral("styles.xml"));
        QVERIFY2(styles.contains("style:tab-stop-distance=\"0.2500in\""), styles.constData());
        QVERIFY2(content.contains("text:number-lines=\"false\"") && content.contains("jo:bar-tabs=\"1440\"") && content.contains("style:text-scale=\"200%\""), content.constData());
        auto again = WriteDoc::fromFile(odt, &err);
        QVERIFY2(again, qPrintable(err));
        check(*again);
        // The .docx the document makes says the same things.
        const QByteArray docx = doc->toDocx();
        office::Package pkg;
        QVERIFY(pkg.read(docx));
        const QByteArray x = pkg.part(QStringLiteral("/word/document.xml")), s = pkg.part(QStringLiteral("/word/settings.xml"));
        QVERIFY2(x.contains("<w:suppressLineNumbers/>") && x.contains("<w:tab w:val=\"bar\" w:pos=\"1440\"/>") && x.contains("<w:w w:val=\"200\"/>"), x.constData());
        QVERIFY2(s.contains("<w:defaultTabStop w:val=\"360\"/>"), s.constData());
    }

    // A paragraph keeps up to 64 bar stops: a file with more (a .docx or an .odt) is read with the first 64, and both formats are written with those, the same ones.
    void aParagraphKeepsAtMost64BarStopsInEveryFormat()
    {
        QByteArray tabs = "<w:tabs>";
        QByteArray places;
        for (int i = 0; i < 70; ++i) {
            tabs += "<w:tab w:val=\"bar\" w:pos=\"" + QByteArray::number(100 + i * 50) + "\"/>";
            places += (i ? " " : "") + QByteArray::number(100 + i * 50);
        }
        tabs += "</w:tabs>";
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body><w:p><w:pPr>" + tabs + "</w:pPr><w:r><w:t>x</w:t></w:r></w:p></w:body></w:document>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        QString err;
        auto fromDocx = WriteDoc::fromDocx(w.finish(), &err);
        QVERIFY2(fromDocx, qPrintable(err));
        const QByteArray body = "<text:p text:style-name=\"P1\">x</text:p>";
        auto fromOdt = WriteDoc::fromFile(makeOdt(body, "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties xmlns:jo=\"urn:jeffoffice:odf-extension\" jo:bar-tabs=\"" + places + "\"/></style:style>"), &err);
        QVERIFY2(fromOdt, qPrintable(err));
        for (WriteDoc *d : {fromDocx.get(), fromOdt.get()}) {
            QCOMPARE(int(tabStopsOf(d->story()->begin().blockFormat()).size()), 64);
            office::Package pkg;
            QVERIFY(pkg.read(d->toDocx()));
            QCOMPARE(int(pkg.part(QStringLiteral("/word/document.xml")).count("w:val=\"bar\"")), 64);
            const QByteArray odt = d->toOdt();
            const QByteArray content = odtPart(odt, QStringLiteral("content.xml"));
            const qsizetype at = content.indexOf("jo:bar-tabs=\"");
            QVERIFY(at >= 0);
            QCOMPARE(int(content.mid(at + 13, content.indexOf('"', at + 13) - at - 13).split(' ').size()), 64);
            auto back = WriteDoc::fromFile(odt, &err);
            QVERIFY2(back, qPrintable(err));
            QCOMPARE(int(tabStopsOf(back->story()->begin().blockFormat()).size()), 64);
        }
        // Both read the same 64.
        QCOMPARE(tabStopsOf(fromDocx->story()->begin().blockFormat()), tabStopsOf(fromOdt->story()->begin().blockFormat()));
    }

    // Heavy dashed underlines of an .odt (bold width) are the engine's heavy kinds (4, 12) in the text, and stay so through an .rtf.
    void heavyDashedUnderlinesOfAnOdtAreHeavyKindsAndSurviveRtf()
    {
        struct Row { const char *style, *word; QTextCharFormat::UnderlineStyle line; int kind; };
        const Row rows[] = {{"dash", "aaa", QTextCharFormat::DashUnderline, 4}, {"long-dash", "bbb", QTextCharFormat::DashUnderline, 12}, {"dot-dash", "ccc", QTextCharFormat::DashDotLine, 4},
                            {"dot-dot-dash", "ddd", QTextCharFormat::DashDotDotLine, 4}};
        QByteArray body, styles;
        for (int i = 0; i < 4; ++i) {
            body += QByteArray("<text:p><text:span text:style-name=\"T") + QByteArray::number(i) + "\">" + rows[i].word + "</text:span></text:p>";
            styles += QByteArray("<style:style style:name=\"T") + QByteArray::number(i) + "\" style:family=\"text\"><style:text-properties style:text-underline-style=\"" + rows[i].style +
                      "\" style:text-underline-type=\"single\" style:text-underline-width=\"bold\"/></style:style>";
        }
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt(body, styles), &err);
        QVERIFY2(doc, qPrintable(err));
        auto check = [&](WriteDoc *d, const char *what) {
            for (const Row &r : rows) {
                bool seen = false;
                for (auto b = d->story()->begin(); b.isValid(); b = b.next())
                    for (auto it = b.begin(); !it.atEnd(); ++it)
                        if (it.fragment().text().contains(QLatin1String(r.word))) {
                            seen = true;
                            QVERIFY2(it.fragment().charFormat().underlineStyle() == r.line, what);
                            QVERIFY2(it.fragment().charFormat().intProperty(tp::UnderlineKind) == r.kind, qPrintable(QStringLiteral("%1 %2").arg(QLatin1String(what), QLatin1String(r.word))));
                        }
                QVERIFY2(seen, r.word);
            }
        };
        check(doc.get(), "odt");
        auto back = WriteDoc::fromFile(doc->toRtf(&err), &err);
        QVERIFY2(back, qPrintable(err));
        check(back.get(), "rtf");
    }

    // Character scaling goes into an .rtf (\charscalex) and comes back from it.
    void aScaledRunIsKeptInAnRtf()
    {
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt("<text:p>a<text:span text:style-name=\"T1\">wide</text:span></text:p>", "<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties style:text-scale=\"200%\"/></style:style>"), &err);
        QVERIFY2(doc, qPrintable(err));
        const QByteArray rtf = doc->toRtf(&err);
        QVERIFY2(rtf.contains("\\charscalex200"), rtf.constData());
        auto back = WriteDoc::fromFile(rtf, &err);
        QVERIFY2(back, qPrintable(err));
        int scale = 0;
        for (auto it = back->story()->begin().begin(); !it.atEnd(); ++it)
            if (it.fragment().text().contains(QLatin1String("wide"))) scale = it.fragment().charFormat().fontStretch();
        QCOMPARE(scale, 200);
    }

    // The markup of a paragraph's look is made once for each style, not for each paragraph: five thousand paragraphs of two styles make some looks, not thousands. (Counted.)
    void theLooksOfParagraphsAreMadeOnceForEachStyle()
    {
        QByteArray body;
        for (int i = 0; i < 5000; ++i) body += i % 2 ? "<text:p text:style-name=\"Standard\">Plain <text:span text:style-name=\"T1\">bold</text:span> words.</text:p>" : "<text:h text:style-name=\"Heading_20_1\" text:outline-level=\"1\">Heading</text:h>";
        const qint64 before = jp::office::odt::Reader::looksMadeForTest();
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt(body, kAuto), &err, nullptr);
        QVERIFY2(doc, qPrintable(err));
        const qint64 made = jp::office::odt::Reader::looksMadeForTest() - before;
        QVERIFY2(made <= 40, qPrintable(QStringLiteral("%1 looks made for 5000 paragraphs").arg(made)));
    }

    void fileOpenFindsAnOdtByItsContent()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        // (The name says nothing: the mimetype does.)
        const QString path = writeFile(dir.filePath(QStringLiteral("letter.dat")), makeOdt(kBody, kAuto));
        WriteWindow w;
        QStringList notices;
        w.setNotifier([&](const QString &, const QString &text) { notices << text; });
        QVERIFY(w.openFile(path));
        QVERIFY(w.document()->story()->toPlainText().contains(QLatin1String("second item")));
        QVERIFY(notices.isEmpty());
        QVERIFY(!w.isModified());
        QVERIFY(w.document()->pager().pageCount() >= 1);
        // A damaged one is refused with a reason.
        QString err;
        QVERIFY(!WriteDoc::fromFile(makeOdt(kBody, kAuto).left(300), &err));
        QVERIFY(!err.isEmpty());
    }

    void saveAsOffersAnOdtAndWritesOne()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = writeFile(dir.filePath(QStringLiteral("letter.odt")), makeOdt(kBody, kAuto));
        WriteWindow a;
        QStringList said;
        a.setNotifier([&](const QString &, const QString &text) { said << text; });
        QVERIFY(a.openFile(path));
        QString suggested, filter;
        a.setSavePathChooser([&](const QString &s, const QString &f) {
            suggested = s;
            filter = f;
            return dir.filePath(QStringLiteral("copy.odt"));
        });
        QVERIFY2(a.saveAs(), qPrintable(said.join(QLatin1Char('\n'))));
        // The dialog started on the kind the file is, with the suggested name in that ending.
        QVERIFY2(suggested.endsWith(QLatin1String("letter.odt")), qPrintable(suggested));
        QVERIFY2(filter.contains(QLatin1String("*.odt")), qPrintable(filter));
        QVERIFY(a.filePath().endsWith(QLatin1String("copy.odt")));
        QVERIFY(!a.isModified());
        QByteArray bytes;
        {
            // Closed before the next save: Windows refuses to replace a file another handle holds open.
            QFile f(a.filePath());
            QVERIFY(f.open(QIODevice::ReadOnly));
            bytes = f.readAll();
        }
        QVERIFY(office::odt::looksLikeOdt(bytes));
        QVERIFY(bytes.startsWith("PK"));
        // It opens again with the same text and looks, and Save writes an .odt again (it is a kind JeffWrite writes).
        WriteWindow b;
        QVERIFY(b.openFile(a.filePath()));
        QCOMPARE(b.document()->story()->toPlainText(), a.document()->story()->toPlainText());
        QVERIFY(hasBold(*b.document(), QStringLiteral("bold words")));
        const qint64 before = QFileInfo(a.filePath()).size();
        QVERIFY2(a.save(), qPrintable(said.join(QLatin1Char('\n'))));
        QCOMPARE(QFileInfo(a.filePath()).size(), before);
        QFile again(a.filePath());
        QVERIFY(again.open(QIODevice::ReadOnly));
        QVERIFY(office::odt::looksLikeOdt(again.readAll()));
    }

    // Book fold is kept in an .odt as a setting of its own (the standard has none for it), comes back, and goes on through another save.
    void bookFoldSurvivesSavingAsOdtAndOpeningAgain()
    {
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt(kBody, kAuto), &err);
        QVERIFY(doc);
        QVERIFY(!doc->bookFold());
        doc->setBookFold(true, 12);
        QStringList notes;
        const QByteArray odt = doc->toOdt(&err, &notes);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        QVERIFY(odtPart(odt, QStringLiteral("settings.xml")).contains("JeffOfficeBookFold"));
        auto back = WriteDoc::fromFile(odt, &err);
        QVERIFY(back);
        QVERIFY(back->bookFold());
        QCOMPARE(back->bookFoldSheets(), 12);
        auto again = WriteDoc::fromFile(back->toOdt(&err, &notes), &err);
        QVERIFY(again && again->bookFold());
        QCOMPARE(again->bookFoldSheets(), 12);
        // Turned off, it is gone from the file.
        back->setBookFold(false, 0);
        const QByteArray plain = back->toOdt(&err, &notes);
        QVERIFY(!odtPart(plain, QStringLiteral("settings.xml")).contains("JeffOfficeBookFold\" config:type=\"boolean\">true"));
        auto off = WriteDoc::fromFile(plain, &err);
        QVERIFY(off && !off->bookFold());
    }

    void footnotesBelowTheTextSurviveSavingAsOdtAndOpeningAgain()
    {
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt(kBody, kAuto), &err);
        QVERIFY(doc);
        QCOMPARE(doc->noteSettings(false).position, QString());
        WriteDoc::NoteSettings s;
        s.position = QStringLiteral("beneathText");
        doc->setNoteSettings(false, s);
        QStringList notes;
        const QByteArray odt = doc->toOdt(&err, &notes);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        const QByteArray styles = odtPart(odt, QStringLiteral("styles.xml"));
        QVERIFY2(styles.contains("text:notes-configuration") && styles.contains("text:footnotes-position=\"text\""), styles.constData());
        auto back = WriteDoc::fromFile(odt, &err);
        QVERIFY(back);
        QCOMPARE(back->noteSettings(false).position, QStringLiteral("beneathText"));
        // At the foot of the page there is no such element.
        s.position = QStringLiteral("pageBottom");
        back->setNoteSettings(false, s);
        QVERIFY(!odtPart(back->toOdt(&err, &notes), QStringLiteral("styles.xml")).contains("text:footnotes-position"));
    }

    void aFootnoteMarkThatWasTypedIsTheCitationsLabelInAnOdtAndTheColumnsAreToldAsLost()
    {
        QString err;
        const QByteArray body = "<text:p text:style-name=\"Standard\">Cited<text:note text:id=\"ftn1\" text:note-class=\"footnote\"><text:note-citation text:label=\"\u2020\u2020\">\u2020\u2020</text:note-citation>"
                                "<text:note-body><text:p text:style-name=\"Standard\">NOTEWORDS</text:p></text:note-body></text:note> after</text:p>";
        auto doc = WriteDoc::fromFile(makeOdt(body, QByteArray()), &err);
        QVERIFY2(doc, qPrintable(err));
        const QString mark = QString(2, QChar(0x2020));
        QVERIFY2(doc->story()->toPlainText().contains(mark + QStringLiteral(" after")), qPrintable(doc->story()->toPlainText()));
        QVERIFY(doc->editableFootnote(1)->toPlainText().startsWith(mark));
        QStringList notes;
        const QByteArray odt = doc->toOdt(&err, &notes);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        const QString content = QString::fromUtf8(odtPart(odt, QStringLiteral("content.xml")));
        QVERIFY2(content.contains(QStringLiteral("text:label=\"%1\">%1</text:note-citation>").arg(mark)), qPrintable(content));
        // (The mark is the citation's alone: not again in the text after the note, nor at the start of the note.)
        QCOMPARE(content.count(mark), 2);
        QVERIFY2(content.contains(QStringLiteral("<text:note-body><text:p text:style-name=\"Standard\">NOTEWORDS")), qPrintable(content));
        auto back = WriteDoc::fromFile(odt, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY(back->story()->toPlainText().contains(mark + QStringLiteral(" after")));
        QVERIFY(back->editableFootnote(1)->toPlainText().startsWith(mark));
        QCOMPARE(back->editableFootnote(1)->toPlainText().count(mark), 1);
        // Footnotes in columns are not kept by the format: the person is told.
        auto noted = [&](int columns) {
            WriteDoc::NoteSettings s;
            s.columns = columns;
            back->setNoteSettings(false, s);
            QStringList lost;
            back->toOdt(&err, &lost);
            return lost.filter(QStringLiteral("footnote columns")).size();
        };
        QCOMPARE(noted(3), 1);
        QCOMPARE(noted(1), 0);
    }

    void anOdtMadeFromADocxKeepsItsHeadingListAndTable()
    {
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt(kBody, kAuto), &err);
        QVERIFY(doc);
        QStringList notes;
        const QByteArray odt = doc->toOdt(&err, &notes);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        auto back = WriteDoc::fromFile(odt, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->story()->toPlainText(), doc->story()->toPlainText());
        QVERIFY(hasBold(*back, QStringLiteral("bold words")));
        // The same document's .docx is the same too: what is written is what was read.
        const QByteArray docx1 = doc->toDocx(), docx2 = back->toDocx();
        office::Package p1, p2;
        QVERIFY(p1.read(docx1) && p2.read(docx2));
        QCOMPARE(p1.part(QStringLiteral("/word/document.xml")).count("<w:tc>"), p2.part(QStringLiteral("/word/document.xml")).count("<w:tc>"));
        QCOMPARE(p1.part(QStringLiteral("/word/document.xml")).count("<w:numId"), p2.part(QStringLiteral("/word/document.xml")).count("<w:numId"));
    }

    // A shape with no words is a draw:custom-shape that names its preset (as other programs write it), read into the shape the file format has and written again.
    void aShapeWithoutTextIsReadFromAnOdtAndSavedBackAsACustomShape()
    {
        QString err;
        QStringList notes;
        const QByteArray styles = "<style:style style:name=\"gr1\" style:family=\"graphic\"><style:graphic-properties draw:fill=\"solid\" draw:fill-color=\"#3366ff\" draw:stroke=\"solid\" "
                                  "svg:stroke-color=\"#112233\" svg:stroke-width=\"0.03in\" style:wrap=\"none\" style:horizontal-pos=\"from-left\" style:vertical-pos=\"from-top\" "
                                  "style:horizontal-rel=\"page\" style:vertical-rel=\"page\"/></style:style>";
        const QByteArray body = "<text:p>Before<draw:custom-shape draw:style-name=\"gr1\" draw:name=\"Shape 1\" text:anchor-type=\"paragraph\" svg:width=\"2in\" svg:height=\"1in\" svg:x=\"1in\" svg:y=\"2in\">"
                                "<draw:enhanced-geometry svg:viewBox=\"0 0 21600 21600\" draw:type=\"ooxml-star5\"/></draw:custom-shape> after</text:p>";
        auto doc = WriteDoc::fromFile(makeOdt(body, styles), &err, &notes);
        QVERIFY2(doc, qPrintable(err));
        QVERIFY2(notes.isEmpty(), qPrintable(notes.join(QLatin1Char('|'))));
        int at = -1;
        for (int i = 0; i < doc->story()->characterCount(); ++i)
            if (isDrawnObjectAt(doc->story(), i)) at = i;
        QVERIFY(at >= 0);
        PictureInfo info = pictureAt(*doc, doc->story(), at);
        QCOMPARE(info.preset, QStringLiteral("star5"));
        QCOMPARE(info.fillColor, QStringLiteral("3366FF"));
        QCOMPARE(info.lineColor, QStringLiteral("112233"));
        QVERIFY(std::abs(info.width - 144) < 0.5 && std::abs(info.height - 72) < 0.5);
        QVERIFY(info.floating);
        QVERIFY(std::abs(info.left - 72) < 0.5 && std::abs(info.top - 144) < 0.5);
        // Saved as an .odt it is a custom shape that names the preset, and read again it is the same.
        QStringList saveNotes;
        const QByteArray odt = doc->toOdt(&err, &saveNotes);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        QVERIFY2(saveNotes.isEmpty(), qPrintable(saveNotes.join(QLatin1Char('|'))));
        QByteArray content;
        for (const ZipScanEntry &e : scanZip(odt))
            if (e.name == QLatin1String("content.xml")) content = e.data;
        QVERIFY(content.contains("<draw:custom-shape") && content.contains("ooxml-star5"));
        auto back = WriteDoc::fromFile(odt, &err, &notes);
        QVERIFY2(back, qPrintable(err));
        int again = -1;
        for (int i = 0; i < back->story()->characterCount(); ++i)
            if (isDrawnObjectAt(back->story(), i)) again = i;
        QVERIFY(again >= 0);
        const PictureInfo b = pictureAt(*back, back->story(), again);
        QCOMPARE(b.preset, info.preset);
        QCOMPARE(b.fillColor, info.fillColor);
        QCOMPARE(b.lineColor, info.lineColor);
        QVERIFY(std::abs(b.width - info.width) < 0.5 && std::abs(b.height - info.height) < 0.5);
        QVERIFY(std::abs(b.left - info.left) < 0.5 && std::abs(b.top - info.top) < 0.5);
        QCOMPARE(back->story()->toPlainText().remove(QChar::ObjectReplacementCharacter), QStringLiteral("Before after"));
    }

    void aShapeMadeInTheWindowIsSavedAsAnOdtAndReadBack()
    {
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt("<text:p>Hello world</text:p>"), &err);
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(5);
        ShapeLook look;
        look.fill = QStringLiteral("FF8800");
        look.line = QStringLiteral("112233");
        look.weight = 2;
        QCOMPARE(insertShape(*doc, c, QStringLiteral("ellipse"), QSizeF(90, 80), look), PictureResult::Ok);
        QVERIFY(setPicturePlace(*doc, 5, 150, 200));
        QStringList notes;
        const QByteArray odt = doc->toOdt(&err, &notes);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        QVERIFY2(notes.isEmpty(), qPrintable(notes.join(QLatin1Char('|'))));
        auto back = WriteDoc::fromFile(odt, &err);
        QVERIFY2(back, qPrintable(err));
        int at = -1;
        for (int i = 0; i < back->story()->characterCount(); ++i)
            if (isDrawnObjectAt(back->story(), i)) at = i;
        QVERIFY(at >= 0);
        const PictureInfo info = pictureAt(*back, back->story(), at);
        QCOMPARE(info.preset, QStringLiteral("ellipse"));
        QCOMPARE(info.fillColor, QStringLiteral("FF8800"));
        QVERIFY(std::abs(info.width - 90) < 0.5 && std::abs(info.height - 80) < 0.5);
        QVERIFY(std::abs(info.left - 150) < 0.5 && std::abs(info.top - 200) < 0.5);
    }

    // A shape's turn, flips, adjustments, line width, dash, and arrowheads are saved in an .odt and come back as they were: a line drawn up and to the left still points that way.
    void aShapesTurnFlipsAdjustmentsAndLineAreKeptByAnOdt()
    {
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt("<text:p>Hello world</text:p>"), &err);
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(5);
        QCOMPARE(insertShapeAt(*doc, c, QStringLiteral("straightConnector1"), QPointF(300, 200), QPointF(200, 120)), PictureResult::Ok);
        QVERIFY(setShapeLine(*doc, 5, QStringLiteral("CC0000"), 3, QStringLiteral("dash")));
        QTextCursor d(doc->story());
        d.setPosition(1);
        QCOMPARE(insertShape(*doc, d, QStringLiteral("roundRect"), QSizeF(120, 60)), PictureResult::Ok);
        QVERIFY(setPicturePlace(*doc, 1, 150, 300));
        QVERIFY(setShapeAdjustment(*doc, 1, 0, QPointF(30, 0)));
        QVERIFY(setShapeRotation(*doc, 1, 30));
        QStringList notes;
        const QByteArray odt = doc->toOdt(&err, &notes);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        QVERIFY2(notes.isEmpty(), qPrintable(notes.join(QLatin1Char('|'))));
        auto back = WriteDoc::fromFile(odt, &err);
        QVERIFY2(back, qPrintable(err));
        int line = -1, box = -1;
        for (int i = 0; i < back->story()->characterCount(); ++i) {
            if (!isDrawnObjectAt(back->story(), i)) continue;
            (pictureAt(*back, back->story(), i).preset == QLatin1String("roundRect") ? box : line) = i;
        }
        QVERIFY(line >= 0 && box >= 0);
        const QByteArray xml = QByteArray(); Q_UNUSED(xml);
        const PictureInfo l = pictureAt(*back, back->story(), line), b = pictureAt(*back, back->story(), box);
        QCOMPARE(int(l.kind), int(DrawnKind::Line));
        QVERIFY(std::abs(l.width - 100) < 0.5 && std::abs(l.height - 80) < 0.5);
        QCOMPARE(l.lineColor, QStringLiteral("CC0000"));
        QVERIFY(std::abs(l.lineWeight - 3) < 0.2);
        QCOMPARE(l.lineDash, QStringLiteral("dash"));
        QVERIFY(std::abs(b.rotation - 30) < 0.05);
        QVERIFY(std::abs(shapeAdjustHandles(*back, box)[0].at.x() - 30) < 0.2);
        // The saved .docx of what came back still has the flips and the arrowhead of the line.
        office::Package pkg;
        QVERIFY(pkg.read(back->toDocx()));
        const QByteArray body = pkg.part(QStringLiteral("/word/document.xml"));
        QVERIFY2(body.contains("flipH=\"1\"") && body.contains("flipV=\"1\""), "a line drawn up and to the left points the same way");
        QVERIFY(body.contains("<a:tailEnd type=\"triangle\"/>"));
    }

    void aShapeThatTheLibraryDoesNotHaveIsLeftOutOfAnOdtAndTheNoticeSaysSo()
    {
        QString err;
        QStringList readNotes;
        const QByteArray styles = "<style:style style:name=\"gr1\" style:family=\"graphic\"><style:graphic-properties draw:fill=\"solid\" draw:fill-color=\"#3366ff\"/></style:style>";
        const QByteArray body = "<text:p>Before<draw:custom-shape draw:style-name=\"gr1\" text:anchor-type=\"paragraph\" svg:width=\"2in\" svg:height=\"1in\" svg:x=\"1in\" svg:y=\"2in\">"
                                "<draw:enhanced-geometry svg:viewBox=\"0 0 21600 21600\" draw:type=\"ooxml-madeUpShape\"/></draw:custom-shape> after</text:p>";
        auto doc = WriteDoc::fromFile(makeOdt(body, styles), &err, &readNotes);
        QVERIFY2(doc, qPrintable(err));
        QStringList notes;
        const QByteArray odt = doc->toOdt(&err, &notes);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        QVERIFY2(!notes.isEmpty(), "the shape that was left out is reported");
        bool found = false;
        for (const ZipScanEntry &e : scanZip(odt))
            if (e.name == QLatin1String("content.xml")) found = e.data.contains("madeUpShape") || e.data.contains("custom-shape");
        QVERIFY(!found);
    }

    void theMacroNoticeNamesTheFormatSavedTo()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  "<?xml version=\"1.0\"?><w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body><w:p><w:r><w:t>with macros</w:t></w:r></w:p></w:body></w:document>",
                  QStringLiteral("application/vnd.ms-word.document.macroEnabled.main+xml"));
        w.addPart(QStringLiteral("/word/vbaProject.bin"), QByteArray("VBA-project"), QStringLiteral("application/vnd.ms-office.vbaProject"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdVba"), QStringLiteral("http://schemas.microsoft.com/office/2006/relationships/vbaProject"), QStringLiteral("/word/vbaProject.bin"));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = writeFile(dir.filePath(QStringLiteral("macros.docm")), w.finish());
        WriteWindow win;
        QStringList said;
        win.setNotifier([&](const QString &, const QString &text) { said << text; });
        QVERIFY(win.openFile(path));
        QVERIFY(win.document()->hadMacros());
        QVERIFY(win.saveTo(dir.filePath(QStringLiteral("plain.odt"))));
        QVERIFY(!said.isEmpty());
        QVERIFY2(said[0].contains(QLatin1String("macros")) && said[0].contains(QLatin1String(".odt")) && !said[0].contains(QLatin1String(".docx")), qPrintable(said[0]));
    }

    // ---- Rich Text Format
    static QByteArray formattedDocx()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  "<?xml version=\"1.0\"?><w:document " + ns + "><w:body>"
                  "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Big Title</w:t></w:r></w:p>"
                  "<w:p><w:pPr><w:jc w:val=\"center\"/><w:spacing w:before=\"120\" w:after=\"240\"/><w:ind w:left=\"720\"/></w:pPr>"
                  "<w:r><w:rPr><w:b/><w:i/><w:u w:val=\"single\"/></w:rPr><w:t>Bold caf\xC3\xA9 \xE2\x82\xAC</w:t></w:r>"
                  "<w:r><w:rPr><w:color w:val=\"FF0000\"/><w:sz w:val=\"28\"/><w:rFonts w:ascii=\"Courier New\" w:hAnsi=\"Courier New\"/><w:highlight w:val=\"yellow\"/></w:rPr><w:t xml:space=\"preserve\"> red {braces} \\ slash</w:t></w:r>"
                  "<w:r><w:tab/><w:t>after tab</w:t></w:r>"
                  "<w:hyperlink r:id=\"rIdLink\"><w:r><w:t>a link</w:t></w:r></w:hyperlink></w:p>"
                  "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>first item</w:t></w:r></w:p>"
                  "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>second item</w:t></w:r></w:p>"
                  "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/><w:tblBorders><w:top w:val=\"single\" w:sz=\"8\" w:color=\"000000\"/><w:left w:val=\"single\" w:sz=\"8\" w:color=\"000000\"/>"
                  "<w:bottom w:val=\"single\" w:sz=\"8\" w:color=\"000000\"/><w:right w:val=\"single\" w:sz=\"8\" w:color=\"000000\"/><w:insideH w:val=\"single\" w:sz=\"8\" w:color=\"000000\"/>"
                  "<w:insideV w:val=\"single\" w:sz=\"8\" w:color=\"000000\"/></w:tblBorders></w:tblPr><w:tblGrid><w:gridCol w:w=\"3000\"/><w:gridCol w:w=\"3000\"/></w:tblGrid>"
                  "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"6000\" w:type=\"dxa\"/><w:gridSpan w:val=\"2\"/><w:shd w:val=\"clear\" w:fill=\"DDEEFF\"/></w:tcPr><w:p><w:r><w:t>joined</w:t></w:r></w:p></w:tc></w:tr>"
                  "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>left cell</w:t></w:r></w:p></w:tc>"
                  "<w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>right cell</w:t></w:r></w:p></w:tc></w:tr></w:tbl>"
                  "<w:p><w:pPr><w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdHead\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr></w:pPr>"
                  "<w:r><w:t>end of one</w:t></w:r></w:p>"
                  "<w:p><w:r><w:t>second section</w:t></w:r></w:p>"
                  "<w:sectPr><w:pgSz w:w=\"15840\" w:h=\"12240\" w:orient=\"landscape\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/>"
                  "<w:pgNumType w:fmt=\"lowerRoman\" w:start=\"3\"/></w:sectPr></w:body></w:document>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addPart(QStringLiteral("/word/styles.xml"),
                  "<?xml version=\"1.0\"?><w:styles " + ns + "><w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii=\"Georgia\" w:hAnsi=\"Georgia\"/><w:sz w:val=\"22\"/></w:rPr></w:rPrDefault></w:docDefaults>"
                  "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                  "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/><w:qFormat/>"
                  "<w:pPr><w:keepNext/><w:outlineLvl w:val=\"0\"/></w:pPr><w:rPr><w:b/><w:sz w:val=\"36\"/></w:rPr></w:style></w:styles>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
        w.addPart(QStringLiteral("/word/numbering.xml"),
                  "<?xml version=\"1.0\"?><w:numbering " + ns + "><w:abstractNum w:abstractNumId=\"0\"><w:multiLevelType w:val=\"hybridMultilevel\"/><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/>"
                  "<w:numFmt w:val=\"upperRoman\"/><w:lvlText w:val=\"%1)\"/><w:lvlJc w:val=\"left\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr></w:lvl></w:abstractNum>"
                  "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num></w:numbering>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"));
        w.addPart(QStringLiteral("/word/header1.xml"), "<?xml version=\"1.0\"?><w:hdr " + ns + "><w:p><w:r><w:t>Running head</w:t></w:r></w:p></w:hdr>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdStyles"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdNum"), rel + QStringLiteral("numbering"), QStringLiteral("/word/numbering.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdHead"), rel + QStringLiteral("header"), QStringLiteral("/word/header1.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdLink"), rel + QStringLiteral("hyperlink"), QStringLiteral("https://example.com/page"), true);
        return w.finish();
    }

    void aDocumentIsSavedAsRichTextThatReadsBackWithItsLooks()
    {
        auto doc = write::WriteDoc::fromDocx(formattedDocx());
        QVERIFY(doc);
        QStringList notKept;
        QString err;
        const QByteArray rtf = doc->toRtf(&err, &notKept);
        QVERIFY2(!rtf.isEmpty(), qPrintable(err));
        QVERIFY(notKept.isEmpty());
        // the file as the format has it: the header words and tables, in order, braces balanced, and every character outside ASCII a \u number
        QVERIFY(rtf.startsWith("{\\rtf1\\ansi"));
        for (const char *word : {"\\fonttbl", "\\colortbl", "\\stylesheet", "\\listtable", "\\listoverridetable", "\\trowd", "\\cellx", "\\row", "\\cell", "\\sectd", "\\sect", "\\headerr", "\\fldinst HYPERLINK", "\\lndscpsxn", "\\pgnlcrm"})
            QVERIFY2(rtf.contains(word), word);
        QVERIFY2(rtf.contains("caf\\u233?") && rtf.contains("\\u8364?") && rtf.contains("{braces}") == false && rtf.contains("\\{braces\\}") && rtf.contains("\\\\ slash"), rtf.constData());
        int depth = 0;
        bool escaped = false;
        for (const char c : rtf) {
            if (escaped) {
                escaped = false;
                continue;
            }
            if (c == '\\') escaped = true;
            else if (c == '{') ++depth;
            else if (c == '}') QVERIFY(--depth >= 0);
        }
        QCOMPARE(depth, 0);
        for (const char c : rtf) QVERIFY(uchar(c) < 0x80);
        // read back: the words, the looks, the list, the table, the link, the sections
        auto back = write::WriteDoc::fromRtf(rtf, &err);
        QVERIFY2(back, qPrintable(err));
        const QString plain = back->story()->toPlainText();
        for (const QString &words : {QStringLiteral("Big Title"), QStringLiteral("café €"), QStringLiteral("red {braces} \\ slash"), QStringLiteral("after tab"), QStringLiteral("a link"),
                                     QStringLiteral("first item"), QStringLiteral("joined"), QStringLiteral("right cell"), QStringLiteral("second section")})
            QVERIFY2(plain.contains(words), qPrintable(words));
        office::Package p;
        QVERIFY(p.read(back->toDocx()));
        const QString xml = QString::fromUtf8(p.part(QStringLiteral("/word/document.xml")));
        if (qEnvironmentVariableIsSet("JW_DEBUG_RTF")) { QFile f(QStringLiteral("/tmp/w-ctx-home/out.rtf")); f.open(QIODevice::WriteOnly); f.write(rtf); QFile g(QStringLiteral("/tmp/w-ctx-home/out.xml")); g.open(QIODevice::WriteOnly); g.write(xml.toUtf8()); }
        for (const QString &needle : {QStringLiteral("<w:b/>"), QStringLiteral("<w:i/>"), QStringLiteral("w:val=\"FF0000\""), QStringLiteral("w:val=\"center\""), QStringLiteral("w:val=\"yellow\""), QStringLiteral("w:sz w:val=\"28\""),
                                      QStringLiteral("Courier New"), QStringLiteral("<w:tbl>"), QStringLiteral("w:gridSpan"), QStringLiteral("DDEEFF"), QStringLiteral("<w:numPr>"), QStringLiteral("w:orient=\"landscape\""),
                                      QStringLiteral("lowerRoman"), QStringLiteral("w:left=\"720\""), QStringLiteral("w:before=\"120\""), QStringLiteral("w:after=\"240\"")})
            QVERIFY2(xml.contains(needle), qPrintable(needle));
        QCOMPARE(back->sectionCount(), 2);
        QVERIFY(QString::fromUtf8(p.part(QStringLiteral("/word/numbering.xml"))).contains(QLatin1String("upperRoman")));
        QVERIFY(p.part(QStringLiteral("/word/document.xml")).contains("w:hyperlink") || xml.contains(QLatin1String("HYPERLINK")));
        // the heading style kept its name and look
        const QString styles = QString::fromUtf8(p.part(QStringLiteral("/word/styles.xml")));
        QVERIFY2(styles.contains(QLatin1String("heading 1"), Qt::CaseInsensitive), qPrintable(styles.left(400)));
        // the header of the first section
        bool header = false;
        for (int i = 0; i < 6 && !header; ++i) header = p.part(QStringLiteral("/word/header%1.xml").arg(i)).contains("Running head");
        QVERIFY(header);
        // and what was read is written again as the same words
        const QByteArray again = back->toRtf();
        auto third = write::WriteDoc::fromRtf(again, &err);
        QVERIFY(third);
        QCOMPARE(third->story()->toPlainText(), plain);
    }

    void aPictureIsSavedInRichTextAsAnImageThatReadsBack()
    {
        auto doc = write::WriteDoc::blank();
        QImage image(40, 20, QImage::Format_RGB32);
        image.fill(Qt::red);
        QByteArray png;
        QBuffer buffer(&png);
        QVERIFY(buffer.open(QIODevice::WriteOnly) && image.save(&buffer, "PNG"));
        QTextCursor c(doc->story());
        QCOMPARE(write::insertPicture(*doc, c, png), write::PictureResult::Ok);
        QStringList notKept;
        const QByteArray rtf = doc->toRtf(nullptr, &notKept);
        QVERIFY2(rtf.contains("\\pngblip") && rtf.contains(png.left(8).toHex()), rtf.left(600).constData());
        QVERIFY(notKept.isEmpty());
        auto back = write::WriteDoc::fromRtf(rtf);
        QVERIFY(back);
        office::Package p;
        QVERIFY(p.read(back->toDocx()));
        bool found = false;
        for (const QString &name : p.partNames()) found = found || (name.contains(QLatin1String("media/")) && p.part(name) == png);
        QVERIFY(found);
    }

    void richTextSaysWhatItCannotHold()
    {
        QStringList told;
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  "<?xml version=\"1.0\"?><w:document " + ns + "><w:body><w:p><w:r><w:t>note here</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p></w:body></w:document>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        auto doc = write::WriteDoc::fromDocx(w.finish());
        QVERIFY(doc);
        QStringList notKept;
        const QByteArray rtf = doc->toRtf(nullptr, &notKept);
        QVERIFY(rtf.contains("note here"));
        QVERIFY2(!notKept.isEmpty() && notKept.join(QLatin1Char(' ')).contains(QLatin1String("footnote")), qPrintable(notKept.join(QLatin1Char('|'))));
    }

    void aMacroFileSavedAsDocmKeepsItsProjectUntouched()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QString vbaRel = QStringLiteral("http://schemas.microsoft.com/office/2006/relationships/vbaProject");
        const QByteArray project = QByteArray("VBA-project \x00\x01\x02\xFF bytes that must come back as they went", 56);
        for (const bool tmpl : {false, true}) {
            office::PackageWriter w;
            w.addPart(QStringLiteral("/word/document.xml"),
                      "<?xml version=\"1.0\"?><w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body><w:p><w:r><w:t>with macros</w:t></w:r></w:p></w:body></w:document>",
                      tmpl ? QStringLiteral("application/vnd.ms-word.template.macroEnabledTemplate.main+xml") : QStringLiteral("application/vnd.ms-word.document.macroEnabled.main+xml"));
            w.addPart(QStringLiteral("/word/vbaProject.bin"), project, QStringLiteral("application/vnd.ms-office.vbaProject"));
            w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdVba"), vbaRel, QStringLiteral("/word/vbaProject.bin"));
            QTemporaryDir dir;
            QVERIFY(dir.isValid());
            const QString ext = tmpl ? QStringLiteral("dotm") : QStringLiteral("docm");
            const QString path = writeFile(dir.filePath(QStringLiteral("macros.") + ext), w.finish());
            WriteWindow win;
            QStringList said;
            win.setNotifier([&](const QString &, const QString &text) { said << text; });
            QVERIFY(win.openFile(path, tmpl ? OpenHow::ReadOnly : OpenHow::Normal));
            QVERIFY(win.document()->hadMacros());
            const QString out = dir.filePath(QStringLiteral("saved.") + ext);
            QVERIFY(win.saveTo(out));
            QVERIFY2(said.isEmpty(), qPrintable(said.join(QLatin1Char('|'))));
            office::Package p;
            QVERIFY(p.read(readAll(out)));
            QCOMPARE(p.part(QStringLiteral("/word/vbaProject.bin")), project);
            QVERIFY(p.contentType(QStringLiteral("/word/document.xml")).contains(tmpl ? QLatin1String("macroEnabledTemplate.main+xml") : QLatin1String("macroEnabled.main+xml")));
            QCOMPARE(p.contentType(QStringLiteral("/word/vbaProject.bin")), QStringLiteral("application/vnd.ms-office.vbaProject"));
            QVERIFY(!p.relationshipsOfType(QStringLiteral("/word/document.xml"), QStringLiteral("/vbaProject")).isEmpty());
            // Reopened, it still has them; and saved as a .docx they are left out, said so.
            WriteWindow again;
            QStringList told;
            again.setNotifier([&](const QString &, const QString &text) { told << text; });
            QVERIFY(again.openFile(out, OpenHow::ReadOnly));
            QVERIFY(again.document()->hadMacros());
            if (tmpl) continue;
            QVERIFY(again.saveTo(dir.filePath(QStringLiteral("plain.docx")), true));
            office::Package q;
            QVERIFY(q.read(readAll(dir.filePath(QStringLiteral("plain.docx")))));
            QVERIFY(q.part(QStringLiteral("/word/vbaProject.bin")).isEmpty());
            QVERIFY(!told.isEmpty() && told[0].contains(QLatin1String(".docx")));
        }
    }

    void saveOnAFlatOdtExplainsAndOffersAnOdt()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        // (The file is read by its content; its name says it is a flat one.)
        const QString path = writeFile(dir.filePath(QStringLiteral("letter.fodt")), makeOdt(kBody, kAuto));
        WriteWindow w;
        QStringList said;
        w.setNotifier([&](const QString &, const QString &text) { said << text; });
        QVERIFY(w.openFile(path));
        QString suggested, filter;
        w.setSavePathChooser([&](const QString &s, const QString &f) {
            suggested = s;
            filter = f;
            return dir.filePath(QStringLiteral("copy.odt"));
        });
        QVERIFY(w.save());
        QVERIFY2(!said.isEmpty() && said[0].contains(QLatin1String(".fodt")) && said[0].contains(QLatin1String(".odt file")), qPrintable(said.join(QLatin1Char('|'))));
        QVERIFY2(suggested.endsWith(QLatin1String("letter.odt")), qPrintable(suggested));
        QVERIFY2(filter.contains(QLatin1String("*.odt")), qPrintable(filter));
        QVERIFY(w.filePath().endsWith(QLatin1String("copy.odt")));
    }

    void theNotKeptNoteComesAgainForWhatIsNew()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        office::PackageWriter p;
        p.addPart(QStringLiteral("/word/document.xml"),
                  "<?xml version=\"1.0\"?><w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"><w:body>"
                  "<w:p><w:r><w:t>first</w:t></w:r><w:r><w:object/></w:r></w:p><w:p><w:r><w:t>second</w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText> FORMTEXT </w:instrText></w:r>"
                  "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>typed</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r></w:p></w:body></w:document>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        p.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = writeFile(dir.filePath(QStringLiteral("two.docx")), p.finish());
        WriteWindow w;
        QStringList said;
        w.setNotifier([&](const QString &, const QString &text) { said << text; });
        QVERIFY(w.openFile(path));
        // The form field goes first (so the first save has only the object to say), and comes back.
        QTextCursor c(w.document()->story());
        c.setPosition(w.document()->story()->findBlockByNumber(1).position() + 6);
        c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QVERIFY(c.hasSelection());
        removeSelection(*w.document(), c);
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("one.odt"))));
        QCOMPARE(said.size(), 1);
        QVERIFY2(said[0].contains(QLatin1String("embedded objects")) && !said[0].contains(QLatin1String("form fields")), qPrintable(said[0]));
        QVERIFY(w.document()->undo());
        said.clear();
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("two.odt"))));
        QCOMPARE(said.size(), 1);
        QVERIFY2(said[0].contains(QLatin1String("form fields")) && !said[0].contains(QLatin1String("embedded objects")), qPrintable(said[0]));
        said.clear();
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("three.odt"))));
        QVERIFY(said.isEmpty());
    }

    // ---- equations as formula objects

    static QByteArray equationsDocx(int count, const QByteArray &math = "<m:oMath><m:f><m:num><m:r><m:t>a</m:t></m:r></m:num><m:den><m:r><m:t>b</m:t></m:r></m:den></m:f><m:r><m:t>+x</m:t></m:r></m:oMath>")
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        QByteArray body;
        for (int i = 0; i < count; ++i) body += "<w:p><w:r><w:t>eq </w:t></w:r>" + math + "</w:p>";
        office::PackageWriter p;
        p.addPart(QStringLiteral("/word/document.xml"),
                  "<?xml version=\"1.0\"?><w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"><w:body>" + body +
                      "</w:body></w:document>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        p.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        return p.finish();
    }

    // An .odt of one paragraph per frame, with the parts the frames name.
    static QByteArray formulaOdt(const QStringList &frames, const QMap<QString, QByteArray> &parts)
    {
        QByteArray body;
        for (const QString &f : frames) body += "<text:p>" + f.toUtf8() + "</text:p>";
        jp::ZipWriter z;
        z.add(QStringLiteral("mimetype"), QByteArray("application/vnd.oasis.opendocument.text"), false);
        z.add(QStringLiteral("content.xml"),
              QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-content") + kNs + " office:version=\"1.2\"><office:automatic-styles/><office:body><office:text>" + body +
                  "</office:text></office:body></office:document-content>",
              true);
        z.add(QStringLiteral("styles.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-styles") + kNs + " office:version=\"1.2\"><office:styles/><office:automatic-styles/><office:master-styles/></office:document-styles>", true);
        for (auto it = parts.cbegin(); it != parts.cend(); ++it) z.add(it.key(), it.value(), true);
        return z.finish();
    }

    static QStringList mathsOf(const QByteArray &docx)
    {
        office::Package p;
        QString err;
        if (!p.read(docx, &err)) return {};
        const QByteArray xml = p.part(QStringLiteral("/word/document.xml"));
        QStringList out;
        static const QRegularExpression math(QStringLiteral("<m:oMath[ >].*?</m:oMath>"), QRegularExpression::DotMatchesEverythingOption);
        const QString text = QString::fromUtf8(xml);
        for (auto it = math.globalMatch(text); it.hasNext();) out << office::wml::equationLine(office::wml::equationMarkupDeclared(it.next().captured().toUtf8()));
        return out;
    }

    static const QByteArray hello()
    {
        return "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mfrac><mi>p</mi><mi>q</mi></mfrac></math>";
    }

    static QString frameOf(const QString &object, const QString &picture = QString())
    {
        return QStringLiteral("<draw:frame svg:width=\"1in\" svg:height=\"0.4in\" text:anchor-type=\"as-char\"><draw:object xlink:href=\"./%1\"/>%2</draw:frame>")
            .arg(object, picture.isEmpty() ? QString() : QStringLiteral("<draw:image xlink:href=\"%1\"/>").arg(picture));
    }

    static QByteArray tinyPng()
    {
        QByteArray png;
        QImage image(4, 4, QImage::Format_RGB32);
        image.fill(Qt::blue);
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        return png;
    }

    // An equation is saved as a formula object (its MathML in a part of its own, with a picture), opens as the same equation, and saving it again makes the same parts.
    void anEquationIsAFormulaObjectInAnOdt()
    {
        const QByteArray docx = equationsDocx(2);
        QString err;
        QStringList notes;
        const QByteArray odt = office::odt::fromDocxPackage(docx, &err, &notes);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(odt, parts));
        QVERIFY(parts.value(QStringLiteral("content.xml")).contains("<draw:object"));
        QVERIFY(parts.value(QStringLiteral("Object 1/content.xml")).contains("MathML"));
        QVERIFY(parts.value(QStringLiteral("Object 2/content.xml")).contains("<mfrac"));
        QVERIFY(parts.value(QStringLiteral("ObjectReplacements/Object 1.png")).startsWith("\x89PNG"));
        QVERIFY(parts.value(QStringLiteral("META-INF/manifest.xml")).contains("application/vnd.oasis.opendocument.formula"));
        QVERIFY2(notes.join(QLatin1Char('|')).isEmpty(), qPrintable(notes.join(QLatin1Char('|'))));
        const QStringList before = mathsOf(docx);
        QCOMPARE(before.size(), 2);
        QVERIFY(!before[0].isEmpty());
        const QByteArray back = office::odt::toDocxPackage(odt, &err, &notes);
        QVERIFY2(!back.isEmpty(), qPrintable(err));
        QCOMPARE(mathsOf(back), before);
        // Saved again: the same parts (the sub-documents of the first are not kept beside the second's).
        const QByteArray again = office::odt::fromDocxPackage(back, &err, &notes);
        QMap<QString, QByteArray> second;
        QVERIFY(jp::readZip(again, second));
        QCOMPARE(QStringList(second.keys()), QStringList(parts.keys()));
    }

    // A formula object written by hand (the other suite's own form) is shown as an equation.
    void aHandWrittenFormulaObjectIsAnEquation()
    {
        QString err;
        QStringList notes;
        const QByteArray odt = formulaOdt({frameOf(QStringLiteral("Object 1"), QStringLiteral("ObjectReplacements/Object 1"))},
                                          {{QStringLiteral("Object 1/content.xml"), hello()}, {QStringLiteral("ObjectReplacements/Object 1"), tinyPng()}});
        const QByteArray docx = office::odt::toDocxPackage(odt, &err, &notes);
        QVERIFY2(!docx.isEmpty(), qPrintable(err));
        const QStringList maths = mathsOf(docx);
        QCOMPARE(maths.size(), 1);
        QCOMPARE(maths[0], office::wml::equationLine(office::wml::equationMarkupDeclared(office::mathObjectToOmml(hello()))));
        QVERIFY(!maths[0].isEmpty());
    }

    // A document with a script, a link, or a root that is not MathML is not read as an equation: the frame is its picture.
    void aFormulaObjectThatRunsOrLinksOrIsNotMathIsItsPicture()
    {
        const QByteArray script = "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mi>x</mi><script>alert(1)</script></math>";
        const QByteArray link = "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mi href=\"http://example.com/\">x</mi></math>";
        const QByteArray xlink = "<math xmlns=\"http://www.w3.org/1998/Math/MathML\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"><mrow xlink:href=\"http://example.com/\"><mi>x</mi></mrow></math>";
        const QByteArray other = "<svg xmlns=\"http://www.w3.org/2000/svg\"><mi>x</mi></svg>";
        for (const QByteArray &bad : {script, link, xlink, other}) {
            QString err;
            const QByteArray odt = formulaOdt({frameOf(QStringLiteral("Object 1"), QStringLiteral("ObjectReplacements/Object 1.png"))},
                                              {{QStringLiteral("Object 1/content.xml"), bad}, {QStringLiteral("ObjectReplacements/Object 1.png"), tinyPng()}});
            const QByteArray docx = office::odt::toDocxPackage(odt, &err);
            QVERIFY2(!docx.isEmpty(), qPrintable(err));
            QVERIFY2(mathsOf(docx).isEmpty(), bad.constData());
            office::Package p;
            QVERIFY(p.read(docx, &err));
            QVERIFY2(p.part(QStringLiteral("/word/document.xml")).contains("<pic:pic"), bad.constData());
        }
    }

    // At most 2,000 formula objects are read (and written), and 16 MB of their documents.
    void formulaObjectsAreHeldToTheirLimits()
    {
        QString err;
        // Read: 2,001 frames of one object.
        QStringList frames;
        for (int i = 0; i < 2000 + 1; ++i) frames << frameOf(QStringLiteral("Object 1"));
        QStringList notes;
        const QByteArray odt = formulaOdt(frames, {{QStringLiteral("Object 1/content.xml"), hello()}});
        const QByteArray docx = office::odt::toDocxPackage(odt, &err, &notes);
        QVERIFY2(!docx.isEmpty(), qPrintable(err));
        QCOMPARE(mathsOf(docx).size(), 2000);
        QVERIFY2(notes.join(QLatin1Char('|')).contains(QLatin1String("equations past")), qPrintable(notes.join(QLatin1Char('|'))));
        // Written: 2,001 equations make 2,000 objects, the last is kept as text.
        QStringList wnotes;
        const QByteArray written = office::odt::fromDocxPackage(equationsDocx(2000 + 1), &err, &wnotes);
        QVERIFY2(!written.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(written, parts));
        int objects = 0;
        for (const QString &k : parts.keys())
            if (k.startsWith(QLatin1String("Object ")) && k.endsWith(QLatin1String("/content.xml"))) ++objects;
        QCOMPARE(objects, 2000);
        QVERIFY2(wnotes.join(QLatin1Char('|')).contains(QLatin1String("plain text")), qPrintable(wnotes.join(QLatin1Char('|'))));
        // Read: documents of about a megabyte each (the comment is padding): 16 fit in 16 MB, the 17th and on are left.
        QByteArray big = hello();
        big.insert(big.size() - 7, "<!--" + QByteArray(999000, 'x') + "-->");
        QStringList many;
        for (int i = 0; i < 20; ++i) many << frameOf(QStringLiteral("Object 1"));
        const QByteArray heavy = formulaOdt(many, {{QStringLiteral("Object 1/content.xml"), big}});
        const QByteArray heavyDocx = office::odt::toDocxPackage(heavy, &err);
        QVERIFY2(!heavyDocx.isEmpty(), qPrintable(err));
        QCOMPARE(mathsOf(heavyDocx).size(), int((16 << 20) / big.size()));
    }

    // A chart made by another suite declares the math namespace on its root: it is read once as a chart (not inflated again as a formula), however many frames name it.
    void aChartThatDeclaresMathIsInflatedOnceNoMatterTheFrames()
    {
        const QByteArray chart = "<?xml version=\"1.0\"?><office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\" "
                                 "xmlns:math=\"http://www.w3.org/1998/Math/MathML\"><office:body><office:chart><chart:chart chart:class=\"chart:bar\"/></office:chart></office:body></office:document-content>";
        QStringList frames;
        for (int i = 0; i < 500; ++i) frames << frameOf(QStringLiteral("Object 1"));
        const QByteArray odt = formulaOdt(frames, {{QStringLiteral("Object 1/content.xml"), chart}});
        const qint64 before = office::mathObjectTestHooks().objectReads.load();
        QString err;
        office::odt::toDocxPackage(odt, &err);
        QCOMPARE(office::mathObjectTestHooks().objectReads.load() - before, qint64(1));
    }

    // A formula object that is refused is inflated once, not again for each frame that names it.
    void aRefusedFormulaObjectIsInflatedOnce()
    {
        const QByteArray bad = "<?xml version=\"1.0\"?><math xmlns=\"http://www.w3.org/1998/Math/MathML\"><script>x</script></math>";
        QStringList frames;
        for (int i = 0; i < 1000; ++i) frames << frameOf(QStringLiteral("Object 1"));
        const QByteArray odt = formulaOdt(frames, {{QStringLiteral("Object 1/content.xml"), bad}});
        const qint64 before = office::mathObjectTestHooks().objectReads.load();
        QString err;
        office::odt::toDocxPackage(odt, &err);
        QCOMPARE(office::mathObjectTestHooks().objectReads.load() - before, qint64(1));
    }

    // The count and the bytes are looked at before a document is inflated: 2,500 objects of their own inflate 2,000, and 20 of a megabyte inflate 16.
    void formulaObjectsPastTheLimitsAreNotInflated()
    {
        QStringList frames;
        QMap<QString, QByteArray> parts;
        for (int i = 1; i <= 2500; ++i) {
            frames << frameOf(QStringLiteral("Object %1").arg(i));
            parts.insert(QStringLiteral("Object %1/content.xml").arg(i), hello());
        }
        qint64 before = office::mathObjectTestHooks().objectReads.load();
        QString err;
        const QByteArray docx = office::odt::toDocxPackage(formulaOdt(frames, parts), &err);
        QCOMPARE(office::mathObjectTestHooks().objectReads.load() - before, qint64(2000));
        QCOMPARE(mathsOf(docx).size(), 2000);
        QByteArray big = hello();
        big.insert(big.size() - 7, "<!--" + QByteArray(999000, 'x') + "-->");
        frames.clear();
        parts.clear();
        for (int i = 1; i <= 20; ++i) {
            frames << frameOf(QStringLiteral("Object %1").arg(i));
            parts.insert(QStringLiteral("Object %1/content.xml").arg(i), big);
        }
        before = office::mathObjectTestHooks().objectReads.load();
        office::odt::toDocxPackage(formulaOdt(frames, parts), &err);
        QCOMPARE(office::mathObjectTestHooks().objectReads.load() - before, qint64((16 << 20) / big.size()));
    }

    static QByteArray frameAndPictureOfFirstEquation(const QByteArray &math, QSizeF *frame, QImage *picture)
    {
        QString err;
        const QByteArray odt = office::odt::fromDocxPackage(equationsDocx(1, math), &err);
        const QString content = QString::fromUtf8(odtPart(odt, QStringLiteral("content.xml")));
        static const QRegularExpression size(QStringLiteral("svg:width=\"([0-9.]+)in\" svg:height=\"([0-9.]+)in\""));
        const QRegularExpressionMatch m = size.match(content);
        *frame = m.hasMatch() ? QSizeF(m.captured(1).toDouble(), m.captured(2).toDouble()) : QSizeF();
        *picture = QImage::fromData(odtPart(odt, QStringLiteral("ObjectReplacements/Object 1.png")));
        return odt;
    }

    static QByteArray runOf(const QByteArray &props, int count)
    {
        QByteArray text;
        for (int i = 0; i < count; ++i) text += "a+b";
        return "<m:oMath><m:r><w:rPr>" + props + "</w:rPr><m:t>" + text + "</m:t></m:r></m:oMath>";
    }

    // The frame and the picture of a saved equation follow the size of its letters.
    void anEquationsPreviewFollowsItsRunsSize()
    {
        auto run = runOf;
        QSizeF small, large;
        QImage smallPicture, largePicture;
        frameAndPictureOfFirstEquation(run("<w:sz w:val=\"24\"/>", 1), &small, &smallPicture);
        frameAndPictureOfFirstEquation(run("<w:sz w:val=\"48\"/>", 1), &large, &largePicture);
        QVERIFY(small.width() > 0 && large.width() > 0);
        QVERIFY2(large.width() > 1.8 * small.width() && large.width() < 2.2 * small.width(), qPrintable(QString::number(large.width() / small.width())));
        QVERIFY(largePicture.width() > 1.6 * smallPicture.width());
    }

    // ... and its color.
    void anEquationsPreviewIsDrawnInItsRunsColor()
    {
        auto run = runOf;
        QSizeF redSize;
        QImage red;
        frameAndPictureOfFirstEquation(run("<w:color w:val=\"FF0000\"/>", 1), &redSize, &red);
        QVERIFY(!red.isNull());
        int redPixels = 0, otherPixels = 0;
        for (int y = 0; y < red.height(); ++y)
            for (int x = 0; x < red.width(); ++x) {
                const QColor c = red.pixelColor(x, y);
                if (c.alpha() < 200) continue;
                (c.red() > 200 && c.green() < 60 && c.blue() < 60 ? redPixels : otherPixels)++;
            }
        QVERIFY(redPixels > 20);
        QCOMPARE(otherPixels, 0);
    }

    // One too wide for a picture (at most 4,000 pixels) keeps its proportions.
    void aWideEquationsPictureKeepsItsProportions()
    {
        auto run = runOf;
        QSizeF wide;
        QImage widePicture;
        frameAndPictureOfFirstEquation(run("<w:sz w:val=\"24\"/>", 800), &wide, &widePicture);
        QVERIFY(!widePicture.isNull());
        QVERIFY(widePicture.width() <= 4000 && widePicture.width() >= 3000);
        // (the picture's height is whole pixels: the proportions agree to about a pixel of it)
        const double seen = double(widePicture.width()) / widePicture.height(), asked = wide.width() / wide.height();
        QVERIFY2(std::abs(seen / asked - 1) < 1.5 / widePicture.height() + 0.01, qPrintable(QString::number(seen) + " vs " + QString::number(asked)));
    }

    static QByteArray odtPart(const QByteArray &odt, const QString &name)
    {
        QMap<QString, QByteArray> parts;
        return jp::readZip(odt, parts) ? parts.value(name) : QByteArray();
    }

    // A formula in a table cell is saved as the field's expression and comes back as the same formula, with the result it showed.
    void aFormulaInATableCellSurvivesSavingAsOdt()
    {
        const QByteArray body = "<table:table><table:table-column table:number-columns-repeated=\"2\"/><table:table-row><table:table-cell><text:p>a</text:p></table:table-cell><table:table-cell><text:p>5</text:p></table:table-cell></table:table-row>"
                                "<table:table-row><table:table-cell><text:p>b</text:p></table:table-cell><table:table-cell><text:p>7</text:p></table:table-cell></table:table-row>"
                                "<table:table-row><table:table-cell><text:p>Total</text:p></table:table-cell><table:table-cell><text:p/></table:table-cell></table:table-row></table:table><text:p>after</text:p>";
        auto doc = WriteDoc::fromFile(makeOdt(body));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
            if (b.text().isEmpty() && cellKeyOf(b)) c.setPosition(b.position());
        QVERIFY(insertField(*doc, c, QStringLiteral("= SUM(ABOVE) \\# \"0.00\"")));
        updateFields(*doc);
        auto docxText = [](WriteDoc &d) {
            office::Package pk;
            pk.read(d.toDocx());
            return pk.part(QStringLiteral("/word/document.xml"));
        };
        QVERIFY2(docxText(*doc).contains(">12.00<"), docxText(*doc).right(1500).constData());
        const QByteArray odt = doc->toOdt();
        QVERIFY(!odt.isEmpty());
        const QByteArray content = odtPart(odt, QStringLiteral("content.xml"));
        QVERIFY2(content.contains("text:expression") && content.contains("of:=SUM(ABOVE)"), content.right(1500).constData());
        auto back = WriteDoc::fromFile(odt);
        QVERIFY(back);
        const QByteArray x = docxText(*back);
        QVERIFY2(x.contains("SUM(ABOVE)") && x.contains(">12.00<"), x.right(1500).constData());
        // (and it can be updated again)
        updateFields(*back);
        QVERIFY2(docxText(*back).contains("SUM(ABOVE)") && docxText(*back).contains(">12.00<"), docxText(*back).right(1500).constData());   // (the number format came back too)
        QVERIFY(docxText(*back).contains("0.00"));
    }

    // What a table style gives the cells (the header row's fill, the bands) is written on the cells, so the table looks the same when the file is opened again.
    void aTableStylesFillsAreKeptOnTheCellsWhenSavingAsOdt()
    {
        auto doc = WriteDoc::fromFile(makeOdt("<table:table><table:table-column table:number-columns-repeated=\"2\"/>"
                                              "<table:table-row><table:table-cell><text:p>h1</text:p></table:table-cell><table:table-cell><text:p>h2</text:p></table:table-cell></table:table-row>"
                                              "<table:table-row><table:table-cell><text:p>x</text:p></table:table-cell><table:table-cell><text:p>y</text:p></table:table-cell></table:table-row>"
                                              "<table:table-row><table:table-cell><text:p>z</text:p></table:table-cell><table:table-cell><text:p>w</text:p></table:table-cell></table:table-row></table:table><text:p>after</text:p>"));
        QVERIFY(doc);
        office::wml::TableStyleSpec spec;
        spec.name = QStringLiteral("Red Head");
        spec.headerFill = QStringLiteral("FF0000");
        spec.bandedRowFill = QStringLiteral("00FF00");
        QString problem;
        const QString id = createTableStyle(*doc, spec, &problem);
        QVERIFY2(!id.isEmpty(), qPrintable(problem));
        QTextCursor c(doc->story());
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
            if (b.text() == QLatin1String("h1")) c.setPosition(b.position());
        setTableStyle(*doc, c, id);
        const QByteArray odt = doc->toOdt();
        const QByteArray content = odtPart(odt, QStringLiteral("content.xml")).toLower();
        QVERIFY(content.contains("fo:background-color=\"#ff0000\""));   // (a style the two header cells share)
        QVERIFY(content.contains("fo:background-color=\"#00ff00\""));       // (the banded row)
        auto back = WriteDoc::fromFile(odt);
        QVERIFY(back);
        office::Package p;
        QVERIFY(p.read(back->toDocx()));
        QCOMPARE(int(p.part(QStringLiteral("/word/document.xml")).toUpper().count("W:FILL=\"FF0000\"")), 2);
    }

    // The text looks a table style gives the header row (bold, a color) are written on its cells' words, and the other rows' words have none.
    void aTableStylesTextLooksAreKeptOnTheWordsWhenSavingAsOdt()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QByteArray ns = " xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
        auto cell = [](const char *word) { return QByteArray("<w:tc><w:tcPr><w:tcW w:w=\"2400\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>") + word + "</w:t></w:r></w:p></w:tc>"; };
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  "<?xml version=\"1.0\"?><w:document" + ns + "><w:body><w:tbl><w:tblPr><w:tblStyle w:val=\"BoldHead\"/><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2400\"/><w:gridCol w:w=\"2400\"/></w:tblGrid>"
                  "<w:tr>" + cell("h1") + cell("h2") + "</w:tr><w:tr>" + cell("x") + cell("y") + "</w:tr></w:tbl><w:p><w:r><w:t>after</w:t></w:r></w:p></w:body></w:document>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addPart(QStringLiteral("/word/styles.xml"),
                  "<?xml version=\"1.0\"?><w:styles" + ns + "><w:style w:type=\"table\" w:styleId=\"BoldHead\"><w:name w:val=\"Bold Head\"/><w:tblPr/>"
                  "<w:tblStylePr w:type=\"firstRow\"><w:rPr><w:b/><w:color w:val=\"0000FF\"/></w:rPr></w:tblStylePr></w:style></w:styles>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
        auto doc = WriteDoc::fromFile(w.finish());
        QVERIFY(doc);
        auto back = WriteDoc::fromFile(doc->toOdt());
        QVERIFY(back);
        office::Package p;
        QVERIFY(p.read(back->toDocx()));
        const QByteArray x = p.part(QStringLiteral("/word/document.xml"));
        auto runOf = [&](const char *word) {
            const qsizetype at = x.indexOf(QByteArray(">") + word + "<");
            const qsizetype start = std::max(x.lastIndexOf("<w:r>", at), x.lastIndexOf("<w:r ", at));
            return x.mid(int(start), int(at - start));
        };
        for (const char *head : {"h1", "h2"}) {
            QVERIFY2(runOf(head).contains("<w:b/>") || runOf(head).contains("<w:b "), runOf(head).constData());
            QVERIFY2(runOf(head).contains("0000FF"), runOf(head).constData());
        }
        for (const char *body : {"x", "y"}) QVERIFY2(!runOf(body).contains("<w:b/>") && !runOf(body).contains("0000FF"), runOf(body).constData());
    }

    // Not a test of its own: the work of a process that the next function starts.
    // The font features of a run are kept in an .odt as the list of features its style holds (fo:font-feature-settings) and come back as they were; as in CSS, the common
    // ligatures (liga) are standard and contextual both, so "liga" 0 turns both off.
    void theFontFeaturesOfARunAreKeptInAnOdt()
    {
        struct Case {
            const char *list;
            int ligatures, form, spacing, set, alternates;
        };
        const Case cases[] = {
            {"&quot;liga&quot; 0", 1, 0, 0, 0, 0},                                                          // (none)
            {"&quot;liga&quot; 1, &quot;clig&quot; 0", 17, 0, 0, 0, 0},                                    // (standard only)
            {"&quot;liga&quot; 1, &quot;dlig&quot; 1", 27, 0, 0, 0, 0},                                    // (standard, contextual, and discretionary)
            {"&quot;hlig&quot; 1, &quot;dlig&quot; 1, &quot;liga&quot; 0", 28, 0, 0, 0, 0},              // (historical and discretionary)
            {"&quot;lnum&quot; 1, &quot;tnum&quot;", 0, 1, 2, 0, 0},
            {"&quot;onum&quot; 1, &quot;pnum&quot; 1, &quot;ss07&quot; 1, &quot;calt&quot; 0", 0, 2, 1, 7, 2},
            {"&quot;calt&quot; 1", 0, 0, 0, 0, 1},
        };
        auto choices = [](WriteDoc &doc, QList<int> *out) {
            for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
                for (auto it = b.begin(); !it.atEnd(); ++it) {
                    if (!it.fragment().text().contains(QLatin1String("word"))) continue;
                    const QTextCharFormat f = it.fragment().charFormat();
                    *out = {f.intProperty(tp::Ligatures), f.intProperty(tp::NumberStyle), f.intProperty(tp::NumberSpacing), f.intProperty(tp::StylisticSet), f.intProperty(tp::ContextualAlts)};
                    return true;
                }
            return false;
        };
        for (const Case &c : cases) {
            const QByteArray styles = QByteArray("<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties fo:font-feature-settings=\"") + c.list + "\"/></style:style>";
            QString err;
            auto doc = WriteDoc::fromFile(makeOdt("<text:p>a <text:span text:style-name=\"T1\">word</text:span> b</text:p>", styles), &err);
            QVERIFY2(doc, c.list);
            const QList<int> want{c.ligatures, c.form, c.spacing, c.set, c.alternates};
            QList<int> got;
            QVERIFY(choices(*doc, &got));
            QCOMPARE(got, want);
            QStringList notes;
            const QByteArray odt = doc->toOdt(&err, &notes);
            QVERIFY(!odt.isEmpty());
            QVERIFY2(odtPart(odt, QStringLiteral("content.xml")).contains("font-feature-settings"), c.list);
            auto back = WriteDoc::fromFile(odt, &err);
            QVERIFY(back);
            QList<int> again;
            QVERIFY(choices(*back, &again));
            QCOMPARE(again, want);
        }
    }

    void savesAnOdtForTheChecks()
    {
        QString out;
        if (!stable::childOutput(&out)) return;
        auto doc = WriteDoc::fromFile(makeOdt(kBody, kAuto));
        QVERIFY(doc);
        const QByteArray odt = doc->toOdt();
        QVERIFY(!odt.isEmpty());
        QFile f(out);
        QVERIFY(f.open(QIODevice::WriteOnly));
        QCOMPARE(f.write(odt), odt.size());
    }

    // A tight wrap's outline is the frame's contour polygon in an .odt, and a picture's side and room round it are its wrap and margins; all of it is read back the same.
    void aWrapOutlineAndItsRoomAreSavedAsTheFramesContourAndMargins()
    {
        auto doc = WriteDoc::blank();
        QImage image(160, 160, QImage::Format_RGB32);
        image.fill(Qt::red);
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        QTextCursor c(doc->story());
        QCOMPARE(insertPicture(*doc, c, png), PictureResult::Ok);
        QVERIFY(setPictureWrap(*doc, 0, PictureWrap::Tight));
        QPolygonF triangle;
        triangle << QPointF(0.5, 0) << QPointF(1, 1) << QPointF(0, 1);
        QVERIFY(setPictureWrapOutline(*doc, 0, triangle));
        QVERIFY(setPictureWrapSide(*doc, 0, PictureWrapSide::Right));
        QVERIFY(setPictureWrapDistances(*doc, 0, 3, 4, 5, 6));
        QString err;
        QStringList notes;
        const QByteArray odt = doc->toOdt(&err, &notes);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        const QByteArray content = odtPart(odt, QStringLiteral("content.xml"));
        QVERIFY(content.contains("<draw:contour-polygon"));
        QVERIFY(content.contains("draw:points=\"10800,0 21600,21600 0,21600\""));
        QVERIFY(content.contains("style:wrap-contour=\"true\""));
        auto back = WriteDoc::fromFile(odt, &err);
        QVERIFY2(back, qPrintable(err));
        const PictureInfo info = pictureAt(*back, back->story(), 0);
        QVERIFY(info.wrap == PictureWrap::Tight);
        QCOMPARE(info.wrapOutline.size(), 3);
        QVERIFY(std::abs(info.wrapOutline[0].x() - 0.5) < 0.0001 && std::abs(info.wrapOutline[2].y() - 1.0) < 0.0001);
        QCOMPARE(info.wrapSide, PictureWrapSide::Right);
        QVERIFY(std::abs(info.distLeft - 5.0) < 0.1 && std::abs(info.distTop - 3.0) < 0.1);
    }

    // A shape's and a text box's outline is the contour polygon of its frame too, and is read back as it was.
    void aTextBoxKeepsItsWrapOutlineInAnOdtAndAShapeLeavesTheEditedPointsOut()
    {
        for (const bool textBox : {false, true}) {
            auto doc = WriteDoc::blank();
            QTextCursor c(doc->story());
            if (textBox) {
                TextBoxDesign design = textBoxDesigns().first();
                design.size = QSizeF(160, 160);
                QCOMPARE(insertTextBox(*doc, c, design), PictureResult::Ok);
            } else {
                QCOMPARE(insertShape(*doc, c, QStringLiteral("rect"), QSizeF(160, 160)), PictureResult::Ok);
            }
            QVERIFY(setPictureWrap(*doc, 0, PictureWrap::Tight));
            QPolygonF triangle;
            triangle << QPointF(0.5, 0) << QPointF(1, 1) << QPointF(0, 1);
            QVERIFY(setPictureWrapOutline(*doc, 0, triangle));
            QVERIFY(setPictureWrapDistances(*doc, 0, 3, 4, 5, 6));
            QString err;
            const QByteArray odt = doc->toOdt(&err);
            QVERIFY2(!odt.isEmpty(), qPrintable(err));
            const QByteArray content = odtPart(odt, QStringLiteral("content.xml"));
            QStringList notes;
            doc->toOdt(&err, &notes);
            auto back = WriteDoc::fromFile(odt, &err);
            QVERIFY2(back, qPrintable(err));
            const PictureInfo info = pictureAt(*back, back->story(), 0);
            QCOMPARE(info.kind, textBox ? DrawnKind::TextBox : DrawnKind::Shape);
            if (textBox) {
                // (A text box is a frame, which may hold the contour polygon.)
                QVERIFY(content.contains("draw:points=\"10800,0 21600,21600 0,21600\""));
                QVERIFY(info.wrap == PictureWrap::Tight);
                QCOMPARE(info.wrapOutline.size(), 3);
                QVERIFY(std::abs(info.wrapOutline[0].x() - 0.5) < 0.0001 && std::abs(info.wrapOutline[2].y() - 1.0) < 0.0001);
            } else {
                // A shape is not a frame: the polygon is not written inside it (other programs would drop or refuse it), the wrap follows the shape's own outline, and the points are named as left out.
                QVERIFY(!content.contains("draw:contour-polygon"));
                QVERIFY(content.contains("style:wrap-contour=\"true\""));
                QVERIFY2(notes.join('|').contains("edited wrap points on shapes"), qPrintable(notes.join('|')));
                QVERIFY(info.wrap == PictureWrap::Square);
            }
            QVERIFY(std::abs(info.distLeft - 5.0) < 0.1 && std::abs(info.distTop - 3.0) < 0.1);
        }
    }

    // A list style (a numbering style in the .docx model) is a text:list-style of the document's styles in an .odt, which the lists of its paragraphs name; read back it is a list style again, whose
    // change reaches every paragraph that has it.
    void aListStyleIsKeptInTheOdtAndFollowsItsChanges()
    {
        auto doc = WriteDoc::blank();
        QTextCursor typing(doc->story());
        insertText(*doc, typing, QStringLiteral("One"));
        insertParagraph(*doc, typing);
        insertText(*doc, typing, QStringLiteral("Two"));
        insertParagraph(*doc, typing);
        insertText(*doc, typing, QStringLiteral("Three"));
        QTextCursor all(doc->story());
        all.select(QTextCursor::Document);
        const QString id = saveListStyle(*doc, QStringLiteral("Meeting Notes"), multilevelPresets().at(1).def);
        QVERIFY(!id.isEmpty());
        applyListStyle(*doc, all, id);
        const QByteArray odt = doc->toOdt();
        const QByteArray styles = odtPart(odt, QStringLiteral("styles.xml")), content = odtPart(odt, QStringLiteral("content.xml"));
        QVERIFY2(styles.contains("<text:list-style style:name=\"Meeting_20_Notes\" style:display-name=\"Meeting Notes\">"), styles.constData());
        QVERIFY2(content.contains("<text:list text:style-name=\"Meeting_20_Notes\""), content.constData());
        QVERIFY(!content.contains("<text:list-style"));   // (the paragraphs' lists name the style: no list style of their own)
        QString err;
        auto again = WriteDoc::fromFile(odt, &err);
        QVERIFY2(again, qPrintable(err));
        const QVector<WriteDoc::ListStyleEntry> entries = again->listStyleEntries();
        QCOMPARE(entries.size(), 1);
        QCOMPARE(entries[0].name, QStringLiteral("Meeting Notes"));
        for (int i = 0; i < 3; ++i) {
            const QTextBlock b = again->story()->findBlockByNumber(i);
            QCOMPARE(again->numbering().definitionOf(b.blockFormat().intProperty(prop::NumId)), entries[0].abstractId);
        }
        office::wml::AbstractNum changed = multilevelPresets().at(2).def;
        changed.levels[0].text = QStringLiteral("Item %1:");
        QVERIFY(modifyListStyle(*again, entries[0].id, changed));
        for (int i = 0; i < 3; ++i) {
            const QTextBlock b = again->story()->findBlockByNumber(i);
            QCOMPARE(again->numbering().levelOf(b.blockFormat().intProperty(prop::NumId), 0)->text, QStringLiteral("Item %1:"));
        }
        // Saved again, it says the new text, still in one list style.
        const QByteArray second = odtPart(again->toOdt(), QStringLiteral("styles.xml"));
        QCOMPARE(second.count("<text:list-style"), 1);
        QVERIFY(second.contains("text:prefix=\"Item \"") || second.contains("style:num-prefix=\"Item \""));
        // The .docx of the file read has the numbering style, and the style's link.
        const QByteArray docx = again->toDocx();
        office::Package pkg;
        QVERIFY(pkg.read(docx));
        QVERIFY(pkg.part(QStringLiteral("/word/styles.xml")).contains("w:type=\"numbering\""));
        QVERIFY(pkg.part(QStringLiteral("/word/numbering.xml")).contains("<w:styleLink"));
    }

    // A list style whose name is one the styles part keeps for itself (Normal, here) is given another id, and the paragraph style Normal stays the paragraph style.
    void aListStyleNamedNormalDoesNotTakeTheIdOfAStyle()
    {
        const QByteArray common = "<text:list-style style:name=\"Normal\"><text:list-level-style-number text:level=\"1\" style:num-format=\"I\" style:num-suffix=\".\"/></text:list-style>"
                                  "<text:list-style style:name=\"Hyperlink\"><text:list-level-style-number text:level=\"1\" style:num-format=\"a\" style:num-suffix=\")\"/></text:list-style>";
        const QByteArray body = "<text:p>plain</text:p><text:list text:style-name=\"Normal\"><text:list-item><text:p>one</text:p></text:list-item></text:list>";
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt(body, QByteArray(), common), &err);
        QVERIFY2(doc, qPrintable(err));
        const QVector<WriteDoc::ListStyleEntry> entries = doc->listStyleEntries();
        QCOMPARE(entries.size(), 2);
        QSet<QString> ids;
        for (const WriteDoc::ListStyleEntry &e : entries) {
            QVERIFY(e.id != QLatin1String("Normal") && e.id != QLatin1String("Hyperlink"));
            ids.insert(e.id);
        }
        QCOMPARE(ids.size(), 2);
        QCOMPARE(doc->defaultParagraphStyle(), QStringLiteral("Normal"));
        QCOMPARE(doc->styleSheet().find(QStringLiteral("Normal"))->type, QStringLiteral("paragraph"));
        QCOMPARE(doc->styleSheet().find(QStringLiteral("Hyperlink"))->type, QStringLiteral("character"));
        QCOMPARE(doc->story()->findBlockByNumber(1).textList() != nullptr, true);
    }

    // A list style named like a paragraph style the file has (Heading 1) does not take the style's id.
    void aListStyleNamedLikeAParagraphStyleDoesNotTakeItsId()
    {
        const QByteArray common = "<text:list-style style:name=\"Heading_20_1\" style:display-name=\"Heading 1\"><text:list-level-style-number text:level=\"1\" style:num-format=\"I\"/></text:list-style>";
        const QByteArray body = "<text:h text:style-name=\"Heading_20_1\" text:outline-level=\"1\">A heading</text:h><text:list text:style-name=\"Heading_20_1\"><text:list-item><text:p>one</text:p></text:list-item></text:list>";
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt(body, QByteArray(), common), &err);
        QVERIFY2(doc, qPrintable(err));
        const QVector<WriteDoc::ListStyleEntry> entries = doc->listStyleEntries();
        QCOMPARE(entries.size(), 1);
        QVERIFY(entries[0].id != QLatin1String("Heading1"));
        QCOMPARE(doc->styleSheet().find(QStringLiteral("Heading1"))->type, QStringLiteral("paragraph"));
        QCOMPARE(doc->styleSheet().find(entries[0].id)->type, QStringLiteral("numbering"));
    }

    // A list style named L1 does not take the name of the list a paragraph makes for itself in the file's text (L1, L2...): every list style of the file has a name of its own.
    void aListStyleNamedLikeAnAutomaticListKeepsItsOwnName()
    {
        auto doc = WriteDoc::blank();
        QTextCursor typing(doc->story());
        for (const char *t : {"One", "Two", "Three"}) {
            insertText(*doc, typing, QString::fromLatin1(t));
            if (QLatin1String(t) != QLatin1String("Three")) insertParagraph(*doc, typing);
        }
        const QString id = saveListStyle(*doc, QStringLiteral("L1"), multilevelPresets().at(1).def);
        QVERIFY(!id.isEmpty());
        QTextCursor first(doc->story());
        QVERIFY(applyListStyle(*doc, first, id));
        // A list of the paragraph's own (its own look, no style) in the third paragraph.
        QTextCursor third(doc->story());
        third.setPosition(doc->story()->findBlockByNumber(2).position());
        applyListLevel(*doc, third, numberLevel(QStringLiteral("upperRoman"), QStringLiteral("%1.")));
        const QByteArray odt = doc->toOdt();
        const QByteArray styles = odtPart(odt, QStringLiteral("styles.xml")), content = odtPart(odt, QStringLiteral("content.xml"));
        QVERIFY2(styles.contains("<text:list-style style:name=\"L1_list\" style:display-name=\"L1\">"), styles.constData());
        QVERIFY2(content.contains("<text:list text:style-name=\"L1_list\""), content.constData());
        QVERIFY2(content.contains("<text:list-style style:name=\"L"), content.constData());
        QSet<QByteArray> names;
        for (const QByteArray &part : {styles, content})
            for (qsizetype at = part.indexOf("<text:list-style style:name=\""); at >= 0; at = part.indexOf("<text:list-style style:name=\"", at + 1)) {
                const qsizetype from = at + 29, to = part.indexOf('"', from);
                const QByteArray name = part.mid(from, to - from);
                QVERIFY2(!names.contains(name), name.constData());
                names.insert(name);
            }
        QString err;
        auto again = WriteDoc::fromFile(odt, &err);
        QVERIFY2(again, qPrintable(err));
        QCOMPARE(again->listStyleEntries().size(), 1);
        QCOMPARE(again->listStyleEntries()[0].name, QStringLiteral("L1"));
        QCOMPARE(again->numbering().levelOf(again->story()->findBlockByNumber(2).blockFormat().intProperty(prop::NumId), 0)->numFmt, QStringLiteral("upperRoman"));
    }

    // Lists of one list style that go on from an earlier list of it (A, B, A) say which list they continue: the third goes on from the first, not from B.
    void listsOfAListStyleThatInterleaveEachContinueTheirOwn()
    {
        auto doc = WriteDoc::blank();
        QTextCursor typing(doc->story());
        for (int i = 0; i < 5; ++i) {
            insertText(*doc, typing, QStringLiteral("p%1").arg(i));
            if (i < 4) insertParagraph(*doc, typing);
        }
        const QString id = saveListStyle(*doc, QStringLiteral("Steps"), multilevelPresets().at(1).def);
        for (int at : {0, 2}) {
            QTextCursor c(doc->story());
            c.setPosition(doc->story()->findBlockByNumber(at).position());
            QVERIFY(applyListStyle(*doc, c, id));
        }
        const int a = doc->story()->findBlockByNumber(0).blockFormat().intProperty(prop::NumId), b = doc->story()->findBlockByNumber(2).blockFormat().intProperty(prop::NumId);
        QVERIFY(a != b);
        // The fifth paragraph is of A's list again (a list of the same numbering id, after B's).
        QTextCursor fifth(doc->story());
        fifth.setPosition(doc->story()->findBlockByNumber(4).position());
        QTextBlockFormat bf = fifth.blockFormat();
        bf.setProperty(prop::NumId, a);
        bf.setProperty(prop::NumLevel, 0);
        fifth.setBlockFormat(bf);
        const QByteArray content = odtPart(doc->toOdt(), QStringLiteral("content.xml"));
        const QList<QByteArray> lists = content.split('<');
        QList<QByteArray> opens;
        for (const QByteArray &l : lists)
            if (l.startsWith("text:list ")) opens << l;
        QCOMPARE(opens.size(), 3);
        QVERIFY2(opens[0].contains("xml:id=\"list1\"") && !opens[0].contains("continue"), opens[0].constData());
        QVERIFY2(opens[1].contains("xml:id=\"list2\"") && !opens[1].contains("continue"), opens[1].constData());
        QVERIFY2(opens[2].contains("text:continue-list=\"list1\""), opens[2].constData());
    }

    // A file with 1,500 list styles has 1,000 of them as list styles; the rest still number the lists that name them.
    void aFileOfThousandsOfOdtListStylesIsReadUpToTheLimit()
    {
        QByteArray common;
        for (int i = 0; i < 1500; ++i)
            common += "<text:list-style style:name=\"LS" + QByteArray::number(i) + "\"><text:list-level-style-number text:level=\"1\" style:num-format=\"A\" style:num-suffix=\")\"/></text:list-style>";
        const QByteArray body = "<text:list text:style-name=\"LS5\"><text:list-item><text:p>five</text:p></text:list-item></text:list>"
                                "<text:list text:style-name=\"LS1400\"><text:list-item><text:p>far</text:p></text:list-item></text:list>";
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt(body, QByteArray(), common), &err);
        QVERIFY2(doc, qPrintable(err));
        const QVector<WriteDoc::ListStyleEntry> entries = doc->listStyleEntries();
        QCOMPARE(entries.size(), office::wml::kMostListStyles);
        QCOMPARE(doc->story()->findBlockByNumber(1).textList() != nullptr, true);
        const QTextBlock far = doc->story()->findBlockByNumber(1);
        QCOMPARE(doc->numbering().levelOf(far.blockFormat().intProperty(prop::NumId), 0)->numFmt, QStringLiteral("upperLetter"));
        const QTextBlock five = doc->story()->findBlockByNumber(0);
        QCOMPARE(doc->numbering().definitionOf(five.blockFormat().intProperty(prop::NumId)), [&] {
            for (const WriteDoc::ListStyleEntry &e : entries)
                if (e.name == QLatin1String("LS5")) return e.abstractId;
            return -1;
        }());
    }

    // A graphic (an SVG picture) stays an SVG image in an .odt (the PNG beside it in the other format is left out), and a graphic turned into shapes is an ODF group of paths, one for each shape.
    void aGraphicStaysAnSvgInAnOdtAndItsShapesAreDrawShapes()
    {
        const QByteArray svg = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"48\" height=\"48\" viewBox=\"0 0 48 48\"><rect x=\"2\" y=\"2\" width=\"20\" height=\"20\" fill=\"#ff0000\"/>"
                               "<circle cx=\"36\" cy=\"12\" r=\"8\" fill=\"#00ff00\"/><path d=\"M4 30 L22 30 L13 46 Z\" fill=\"#0000ff\"/></svg>";
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt("<text:p>Words</text:p>"), &err);
        QVERIFY2(doc, qPrintable(err));
        QTextCursor c(doc->story());
        c.setPosition(2);
        QCOMPARE(insertPicture(*doc, c, svg, QStringLiteral("g"), PictureWrap::Square), PictureResult::Ok);
        QVERIFY(pictureAt(*doc, doc->story(), 2).svg);
        QByteArray odt = doc->toOdt(&err);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(odt, parts));
        QStringList images;
        for (auto it = parts.constBegin(); it != parts.constEnd(); ++it)
            if (it.key().startsWith(QLatin1String("Pictures/"))) images << it.key();
        QCOMPARE(images.size(), 1);
        QVERIFY(images[0].endsWith(QLatin1String(".svg")));
        QVERIFY(parts.value(images[0]).contains("<svg"));
        QVERIFY(odtPart(odt, QStringLiteral("content.xml")).contains("<draw:image xlink:href=\"" + images[0].toUtf8() + "\""));
        QVERIFY(odtPart(odt, QStringLiteral("META-INF/manifest.xml")).contains("image/svg+xml"));
        // Read again, the picture is the SVG still (and is a graphic).
        auto again = WriteDoc::fromFile(odt, &err);
        QVERIFY2(again, qPrintable(err));
        QVERIFY(pictureAt(*again, again->story(), 2).svg);
        QVERIFY(graphicSvgAt(*again, 2).contains("<svg"));
        // Converted to shapes, it is a group of three paths in the .odt.
        QCOMPARE(int(convertGraphicToShapes(*doc, 2).result), int(GraphicConvert::Done));
        odt = doc->toOdt(&err);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        const QByteArray content = odtPart(odt, QStringLiteral("content.xml"));
        QCOMPARE(content.count("<draw:g "), 1);
        QCOMPARE(content.count("<draw:path "), 3);
        QVERIFY(!content.contains("<draw:image"));
        QVERIFY(content.toLower().contains("draw:fill-color=\"#ff0000\"") && content.toLower().contains("draw:fill-color=\"#0000ff\""));
    }

    // An SVG that another program put in a .docx beside a picture is taken into the .odt only if it passes what an SVG put in a document does (no references outside it or pictures in it, within the
    // size, and no more to draw than the limit); else the .odt keeps the PNG.
    void aForeignSvgIsKeptInAnOdtOnlyWhereAGraphicMayBe()
    {
        const QByteArray ok = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"48\" height=\"48\" viewBox=\"0 0 48 48\"><rect width=\"20\" height=\"20\" fill=\"#ff0000\"/></svg>";
        QCOMPARE(odtPicturesOf(foreignGraphicDocx(ok)), QStringList{QStringLiteral("svg")});
        auto inside = [&](const QByteArray &part) { return QByteArray(ok).replace("<rect", part + "<rect"); };
        // Outside itself: an address, a file, a picture as data, a definition of its own.
        QCOMPARE(odtPicturesOf(foreignGraphicDocx(inside("<image href=\"http://example.com/x.png\" width=\"4\" height=\"4\"/>"))), QStringList{QStringLiteral("png")});
        QCOMPARE(odtPicturesOf(foreignGraphicDocx(inside("<image href=\"data:image/png;base64,iVBORw0KGgo=\" width=\"4\" height=\"4\"/>"))), QStringList{QStringLiteral("png")});
        QCOMPARE(odtPicturesOf(foreignGraphicDocx(inside("<use href=\"other.svg#a\"/>"))), QStringList{QStringLiteral("png")});
        QCOMPARE(odtPicturesOf(foreignGraphicDocx("<!DOCTYPE svg [<!ENTITY a \"b\">]>" + ok)), QStringList{QStringLiteral("png")});
        // Too large (a byte over 4 MB).
        QByteArray large = ok;
        large.insert(large.size() - 6, "<!--" + QByteArray(int((qint64(4) << 20) + 1 - large.size() - 7), 'x') + "-->");
        QCOMPARE(large.size(), int((qint64(4) << 20) + 1));
        QCOMPARE(odtPicturesOf(foreignGraphicDocx(large)), QStringList{QStringLiteral("png")});
        // Too much to draw: a group that <use>s the one before it ten times, nine deep.
        QByteArray bomb = "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"48\" height=\"48\" viewBox=\"0 0 48 48\"><defs><g id=\"g0\"><rect width=\"1\" height=\"1\"/></g>";
        for (int i = 1; i <= 9; ++i) {
            bomb += "<g id=\"g" + QByteArray::number(i) + "\">";
            for (int k = 0; k < 10; ++k) bomb += "<use xlink:href=\"#g" + QByteArray::number(i - 1) + "\" x=\"" + QByteArray::number(k) + "\"/>";
            bomb += "</g>";
        }
        bomb += "</defs><use xlink:href=\"#g9\"/></svg>";
        QCOMPARE(odtPicturesOf(foreignGraphicDocx(bomb)), QStringList{QStringLiteral("png")});
    }

    // The file is the same bytes each time it is saved, in whatever process.
    void aSavedOdtIsTheSameBytesInEachProcess() { JP_VERIFY_SAME_SAVES("savesAnOdtForTheChecks"); }
};

QTEST_MAIN(WriteOdtTests)
#include "tst_write_odt.moc"
