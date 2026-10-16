// Tests of libs/office/wml: the tree the word processing parts are read into, and reading and writing a .docx.

#include "office/package.h"
#include "office/xml.h"
#include "wml/wml.h"
#include "wml/xmltree.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTest>

using namespace jp::office;
using namespace jp::office::wml;

namespace {

const char kW[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";

QByteArray xmlDoc(const QByteArray &body) { return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n") + body; }

// A package of a document whose body is `body` (the XML inside w:body), with the parts given.
struct Made {
    QByteArray body;
    QByteArray styles, numbering;
    QMap<QString, QByteArray> extra;   // more parts, by name, each related to the document (type: other)
    QByteArray build() const
    {
        PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"), xmlDoc(QByteArray("<w:document ") + kW + "><w:body>" + body + "</w:body></w:document>"),
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        if (!styles.isEmpty()) {
            w.addPart(QStringLiteral("/word/styles.xml"), xmlDoc(QByteArray("<w:styles ") + kW + ">" + styles + "</w:styles>"),
                      QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
            w.addRelationship(QStringLiteral("/word/document.xml"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"), QStringLiteral("/word/styles.xml"));
        }
        if (!numbering.isEmpty()) {
            w.addPart(QStringLiteral("/word/numbering.xml"), xmlDoc(QByteArray("<w:numbering ") + kW + ">" + numbering + "</w:numbering>"),
                      QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"));
            w.addRelationship(QStringLiteral("/word/document.xml"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering"), QStringLiteral("/word/numbering.xml"));
        }
        for (auto it = extra.cbegin(); it != extra.cend(); ++it) {
            w.addPart(it.key(), it.value(), it.key().endsWith(QLatin1String(".png")) ? QStringLiteral("image/png") : QStringLiteral("application/xml"));
            w.addRelationship(QStringLiteral("/word/document.xml"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/other"), it.key());
        }
        return w.finish();
    }
};

QByteArray documentXmlOf(const QByteArray &zip)
{
    Package p;
    p.read(zip);
    return p.part(QStringLiteral("/word/document.xml"));
}

int count(const QByteArray &hay, const QByteArray &needle)
{
    int n = 0;
    for (qsizetype at = hay.indexOf(needle); at >= 0; at = hay.indexOf(needle, at + 1)) ++n;
    return n;
}

} // namespace

class WmlTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void xmlTreeReadsElementsAndKeepsTheirBytes()
    {
        const QByteArray xml = "<?xml version=\"1.0\"?><a:r xmlns:a=\"urn:a\" xmlns=\"urn:d\"><a:x k='1&amp;2' a:j=\"v\"><y>t&lt;1</y>\n<![CDATA[<c>]]></a:x><!-- c --><a:z/></a:r>";
        XmlTree t;
        QVERIFY(t.parse(xml));
        const int root = t.root();
        QCOMPARE(t.localName(root), QByteArray("r"));
        const int x = t.firstChild(root);
        QCOMPARE(t.localName(x), QByteArray("x"));
        QCOMPARE(t.attr(x, "k", NsNone), QStringLiteral("1&2"));
        QCOMPARE(t.raw(x), QByteArray("<a:x k='1&amp;2' a:j=\"v\"><y>t&lt;1</y>\n<![CDATA[<c>]]></a:x>"));
        QCOMPARE(t.text(x), QStringLiteral("t<1\n<c>"));
        const int z = t.next(x);
        QVERIFY(t.isEmptyElement(z));
        QCOMPARE(t.raw(z), QByteArray("<a:z/>"));
        QCOMPARE(t.endTag(z), QByteArray());
        QCOMPARE(t.endTag(x), QByteArray("</a:x>"));
    }

    void xmlTreeMakesAFragmentWholeWhenAnInnerElementDeclaredItsPrefix()
    {
        const QByteArray xml = "<w:body xmlns:w=\"urn:w\"><w:p xmlns:q=\"urn:q\"><w:r q:a=\"1\"/><w:s xmlns:q=\"urn:own\"/></w:p></w:body>";
        XmlTree t;
        QVERIFY(t.parse(xml));
        const int p = t.firstChild(t.root());
        const int r = t.firstChild(p), s = t.next(r);
        // The prefix q was declared by the paragraph, outside the slice: it is declared on the slice again.
        QCOMPARE(t.raw(r), QByteArray("<w:r xmlns:q=\"urn:q\" q:a=\"1\"/>"));
        // A slice that declares the prefix itself is left as it is.
        QCOMPARE(t.raw(s), QByteArray("<w:s xmlns:q=\"urn:own\"/>"));
        // What stands under the root's own declarations needs nothing.
        QCOMPARE(t.raw(p), QByteArray("<w:p xmlns:q=\"urn:q\"><w:r q:a=\"1\"/><w:s xmlns:q=\"urn:own\"/></w:p>"));
    }

    void xmlTreeRefusesWhatIsNotWellFormedOrPassesALimit()
    {
        XmlTree t;
        QString why;
        QVERIFY(!t.parse("<!DOCTYPE a [<!ENTITY e \"x\">]><a>&e;</a>", &why));
        QVERIFY(why.contains(QLatin1String("DOCTYPE")));
        QVERIFY(!t.parse("<a><b></a></b>"));
        QVERIFY(!t.parse("<a>"));
        QVERIFY(!t.parse("<a/><b/>"));
        QVERIFY(!t.parse("<a>&nbsp;</a>"));
        QVERIFY(!t.parse("<p:a/>"));
        QVERIFY(!t.parse("text"));
        // Nested past the depth limit.
        QByteArray deep;
        for (int i = 0; i < kMaxXmlDepth + 5; ++i) deep += "<a>";
        for (int i = 0; i < kMaxXmlDepth + 5; ++i) deep += "</a>";
        QVERIFY(!t.parse(deep, &why));
        QVERIFY(why.contains(QLatin1String("deep")));
        // Too many attributes on one element.
        QByteArray many = "<a";
        for (int i = 0; i <= kMaxAttributesPerElement; ++i) many += " a" + QByteArray::number(i) + "=\"1\"";
        many += "/>";
        QVERIFY(!t.parse(many, &why));
        QVERIFY(why.contains(QLatin1String("attributes")));
        // An element count past the limit is refused before the memory goes.
        QVERIFY(t.parse("<a><b/><b/></a>"));
        QCOMPARE(t.count(), 3);
    }

    void readsParagraphsRunsAndTheirProperties()
    {
        Made m;
        m.body = R"(<w:p><w:pPr><w:pStyle w:val="Heading1"/><w:keepNext/><w:spacing w:before="240" w:after="60" w:line="276" w:lineRule="auto"/><w:ind w:left="720" w:hanging="360"/><w:jc w:val="center"/></w:pPr>)"
                 R"(<w:r><w:rPr><w:rFonts w:ascii="Georgia" w:hAnsi="Georgia"/><w:b/><w:i w:val="0"/><w:color w:val="FF0000"/><w:sz w:val="28"/><w:u w:val="double"/></w:rPr><w:t>Hello</w:t><w:tab/><w:t xml:space="preserve"> there &amp; all</w:t><w:br/><w:t>x</w:t></w:r></w:p>)"
                 R"(<w:sectPr><w:pgSz w:w="15840" w:h="12240" w:orient="landscape"/><w:pgMar w:top="720" w:right="1080" w:bottom="720" w:left="1080" w:header="360" w:footer="360" w:gutter="0"/><w:cols w:num="2" w:space="360"/></w:sectPr>)";
        Document d;
        QString err;
        QVERIFY2(readDocx(m.build(), &d, &err), qPrintable(err));
        QCOMPARE(d.body.size(), 1);
        const Paragraph &p = d.body[0].para;
        QCOMPARE(p.props.style, QStringLiteral("Heading1"));
        QCOMPARE(p.props.keepNext, std::optional<bool>(true));
        QCOMPARE(p.props.before, std::optional<int>(240));
        QCOMPARE(p.props.after, std::optional<int>(60));
        QCOMPARE(p.props.line, std::optional<int>(276));
        QCOMPARE(p.props.indLeft, std::optional<int>(720));
        QCOMPARE(p.props.indFirst, std::optional<int>(-360));
        QCOMPARE(p.props.jc, QStringLiteral("center"));
        QCOMPARE(p.content.size(), 1);
        const Inline &r = p.content[0];
        QCOMPARE(r.kind, Inline::Text);
        QCOMPARE(r.text, QStringLiteral("Hello\t there & all") + QChar(QChar::LineSeparator) + QStringLiteral("x"));
        QCOMPARE(r.props.fonts->ascii, QStringLiteral("Georgia"));
        QCOMPARE(r.props.bold, std::optional<bool>(true));
        QCOMPARE(r.props.italic, std::optional<bool>(false));
        QCOMPARE(r.props.color->val, QStringLiteral("FF0000"));
        QCOMPARE(r.props.size, std::optional<int>(28));
        QCOMPARE(r.props.underline->val, QStringLiteral("double"));
        QVERIFY(d.lastSectionStated);
        QCOMPARE(d.lastSection.pageW, 15840);
        QCOMPARE(d.lastSection.orient, QStringLiteral("landscape"));
        QCOMPARE(d.lastSection.left, 1080);
        QCOMPARE(d.lastSection.cols, 2);
        QCOMPARE(d.lastSection.colSpace, 360);
    }

    void sectionBreaksAreInTheParagraphThatEndsThem()
    {
        Made m;
        m.body = R"(<w:p><w:pPr><w:sectPr><w:type w:val="continuous"/><w:pgSz w:w="12240" w:h="15840"/></w:sectPr></w:pPr><w:r><w:t>one</w:t></w:r></w:p><w:p><w:r><w:t>two</w:t></w:r></w:p>)";
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        QVERIFY(d.body[0].para.props.section);
        QCOMPARE(d.body[0].para.props.section->type, QStringLiteral("continuous"));
        QVERIFY(!d.body[1].para.props.section);
    }

    void stylesInheritFromTheStylesTheyAreBasedOnAndTheDefaults()
    {
        Made m;
        m.body = R"(<w:p/>)";
        m.styles = R"(<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="Calibri"/><w:sz w:val="22"/><w:lang w:val="en-US"/></w:rPr></w:rPrDefault><w:pPrDefault><w:pPr><w:spacing w:after="160"/></w:pPr></w:pPrDefault></w:docDefaults>)"
                   R"(<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>)"
                   R"(<w:style w:type="paragraph" w:styleId="Heading1"><w:name w:val="heading 1"/><w:basedOn w:val="Normal"/><w:next w:val="Normal"/><w:pPr><w:keepNext/><w:spacing w:before="240"/></w:pPr><w:rPr><w:b/><w:sz w:val="32"/></w:rPr></w:style>)"
                   R"(<w:style w:type="paragraph" w:styleId="Heading2"><w:name w:val="heading 2"/><w:basedOn w:val="Heading1"/><w:rPr><w:sz w:val="26"/></w:rPr></w:style>)"
                   R"(<w:style w:type="paragraph" w:styleId="Loop1"><w:basedOn w:val="Loop2"/><w:rPr><w:i/></w:rPr></w:style><w:style w:type="paragraph" w:styleId="Loop2"><w:basedOn w:val="Loop1"/></w:style>)"
                   R"(<w:style w:type="character" w:styleId="Strong"><w:name w:val="Strong"/><w:rPr><w:b/></w:rPr></w:style>)";
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        const StyleSheet &s = d.styles;
        QCOMPARE(s.defaultId(QStringLiteral("paragraph")), QStringLiteral("Normal"));
        const ParaProps p = s.paragraph(QStringLiteral("Heading2"));
        QCOMPARE(p.keepNext, std::optional<bool>(true));
        QCOMPARE(p.before, std::optional<int>(240));
        QCOMPARE(p.after, std::optional<int>(160));   // (from the defaults)
        const RunProps r = s.run(QStringLiteral("Heading2"), QString());
        QCOMPARE(r.size, std::optional<int>(26));      // (the style's own, over Heading1's)
        QCOMPARE(r.bold, std::optional<bool>(true));   // (Heading1's)
        QCOMPARE(r.fonts->ascii, QStringLiteral("Calibri"));
        // A character style comes after the paragraph's.
        QCOMPARE(s.run(QStringLiteral("Normal"), QStringLiteral("Strong")).bold, std::optional<bool>(true));
        // A style that no file defines gives the defaults; styles that name each other don't loop.
        QCOMPARE(s.run(QStringLiteral("Missing"), QString()).size, std::optional<int>(22));
        QCOMPARE(s.run(QStringLiteral("Loop1"), QString()).italic, std::optional<bool>(true));
        // What a style says besides the properties that have a field (here, the language of the defaults) is the style's: it is not carried to
        // the paragraphs and runs that have the style, where it would be written again as theirs.
        QVERIFY(s.run(QStringLiteral("Heading2"), QString()).extra.isEmpty());
        QVERIFY(s.paragraph(QStringLiteral("Heading2")).extra.isEmpty());
    }

    void readsListDefinitions()
    {
        Made m;
        m.body = R"(<w:p><w:pPr><w:numPr><w:ilvl w:val="1"/><w:numId w:val="3"/></w:numPr></w:pPr></w:p>)";
        m.numbering = R"(<w:abstractNum w:abstractNumId="7"><w:multiLevelType w:val="multilevel"/><w:lvl w:ilvl="0"><w:start w:val="4"/><w:numFmt w:val="upperRoman"/><w:lvlText w:val="%1."/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="720" w:hanging="360"/></w:pPr></w:lvl>)"
                      R"(<w:lvl w:ilvl="1"><w:start w:val="1"/><w:numFmt w:val="bullet"/><w:lvlText w:val="o"/><w:rPr><w:rFonts w:ascii="Courier New"/></w:rPr></w:lvl></w:abstractNum>)"
                      R"(<w:num w:numId="3"><w:abstractNumId w:val="7"/><w:lvlOverride w:ilvl="0"><w:startOverride w:val="9"/></w:lvlOverride></w:num>)";
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        QCOMPARE(d.body[0].para.props.numId, std::optional<int>(3));
        QCOMPARE(d.body[0].para.props.ilvl, std::optional<int>(1));
        const AbstractNum *a = d.numbering.abstractOf(3);
        QVERIFY(a);
        QCOMPARE(a->levels[0].numFmt, QStringLiteral("upperRoman"));
        QCOMPARE(a->levels[0].start, 4);
        QCOMPARE(a->levels[0].pPr.indLeft, std::optional<int>(720));
        QCOMPARE(a->levels[0].pPr.indFirst, std::optional<int>(-360));
        QCOMPARE(a->levels[1].numFmt, QStringLiteral("bullet"));
        QVERIFY(!a->levels[2].present);
        QCOMPARE(d.numbering.num(3)->startOverrides.value(0), 9);
    }

    void roundTripKeepsPartsAndTheirRelationshipsAsTheyWere()
    {
        Made m;
        m.body = R"(<w:p><w:r><w:t>x</w:t></w:r></w:p>)";
        const QByteArray picture("\x89PNG\r\n\x1a\n\0\0\0\rIHDR\xff\xfe binary \x80\x81", 28);
        const QByteArray header = xmlDoc(QByteArray("<w:hdr ") + kW + "><w:p><w:r><w:t>Header &amp; text</w:t></w:r></w:p></w:hdr>");
        m.extra.insert(QStringLiteral("/word/media/image1.png"), picture);
        m.extra.insert(QStringLiteral("/word/header1.xml"), header);
        m.extra.insert(QStringLiteral("/customXml/item1.xml"), xmlDoc("<root xmlns=\"urn:x\"><item/></root>"));
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        QByteArray out = writeDocx(d);
        QVERIFY(!out.isEmpty());
        Package a, b;
        QVERIFY(a.read(m.build()));
        QVERIFY(b.read(out));
        for (const QString &name : {QStringLiteral("/word/media/image1.png"), QStringLiteral("/word/header1.xml"), QStringLiteral("/customXml/item1.xml")}) {
            QVERIFY2(b.hasPart(name), qPrintable(name));
            QCOMPARE(b.part(name), a.part(name));
            QCOMPARE(b.contentType(name), a.contentType(name));
        }
        // The links, with the Ids that the document's XML refers to them by.
        const QVector<Relationship> was = a.relationships(QStringLiteral("/word/document.xml")), now = b.relationships(QStringLiteral("/word/document.xml"));
        QCOMPARE(now.size(), was.size());
        for (const Relationship &r : was) {
            const Relationship n = b.relationship(QStringLiteral("/word/document.xml"), r.id);
            QCOMPARE(n.target, r.target);
            QCOMPARE(n.type, r.type);
        }
        QCOMPARE(b.mainPart(), QStringLiteral("/word/document.xml"));
        // Written again from what was read, it comes out the same.
        Document d2;
        QVERIFY(readDocx(out, &d2));
        QCOMPARE(writeDocx(d2), out);
    }

    void roundTripKeepsWhatItDoesNotUnderstandWhereItWas()
    {
        const QByteArray table = R"(<w:tbl><w:tblPr><w:tblW w:w="0" w:type="auto"/></w:tblPr><w:tblGrid><w:gridCol w:w="4680"/><w:gridCol w:w="4680"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>A</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>B</w:t></w:r></w:p></w:tc></w:tr></w:tbl>)";
        const QByteArray drawing = R"(<w:drawing><wp:inline xmlns:wp="urn:wp" distT="0"><wp:extent cx="914400" cy="914400"/></wp:inline></w:drawing>)";
        Made m;
        m.body = R"(<w:p w:rsidR="00A1"><w:pPr><w:pStyle w:val="X"/><w:suppressAutoHyphens/><w:contextualSpacing/><w:framePr w:w="10"/></w:pPr>)"
                 R"(<w:bookmarkStart w:id="0" w:name="_Toc1"/><w:r><w:rPr><w:rtl/><w:lang w:val="fr-FR"/><w:b/><w:rStyle w:val="Strong"/></w:rPr><w:t>Bonjour</w:t></w:r><w:bookmarkEnd w:id="0"/>)"
                 R"(<w:hyperlink r:id="rId9" w:history="1"><w:r><w:t>link</w:t></w:r></w:hyperlink>)"
                 R"(<w:r>)" + drawing + R"(</w:r><w:r><w:fldChar w:fldCharType="begin"/></w:r><w:r><w:instrText xml:space="preserve"> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType="separate"/></w:r><w:r><w:t>1</w:t></w:r><w:r><w:fldChar w:fldCharType="end"/></w:r></w:p>)" +
                 table + R"(<w:sdt><w:sdtPr><w:id w:val="5"/></w:sdtPr><w:sdtContent><w:p><w:r><w:t>inside</w:t></w:r></w:p></w:sdtContent></w:sdt>)" +
                 R"(<w:sectPr w:rsidR="00B2"><w:headerReference w:type="default" r:id="rId8"/><w:pgSz w:w="12240" w:h="15840"/><w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="720" w:footer="720" w:gutter="0"/><w:docGrid w:linePitch="360"/></w:sectPr>)";
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        // The table is a block of its own, shown as a stand-in; the paragraph holds the atoms.
        QCOMPARE(d.body.size(), 5);   // paragraph, table, sdt start, paragraph, sdt end
        QVERIFY(d.body[1].isRaw);
        QCOMPARE(d.body[1].raw, table);
        QVERIFY(d.body[1].display.contains(QLatin1String("2")));
        QCOMPARE(d.body[2].role, Inline::Open);
        QCOMPARE(d.body[4].role, Inline::Close);
        const QByteArray out = documentXmlOf(writeDocx(d));
        QVERIFY(out.contains(table));
        QVERIFY(out.contains(drawing));
        QVERIFY(out.contains(R"(<w:bookmarkStart w:id="0" w:name="_Toc1"/>)"));
        QVERIFY(out.contains(R"(<w:bookmarkEnd w:id="0"/>)"));
        QVERIFY(out.contains(R"(<w:hyperlink r:id="rId9" w:history="1">)"));
        QVERIFY(out.contains("</w:hyperlink>"));
        QVERIFY(out.contains(R"(<w:fldChar w:fldCharType="begin"/>)"));
        QVERIFY(out.contains(R"(<w:instrText xml:space="preserve"> PAGE </w:instrText>)"));
        QVERIFY(out.contains(R"(<w:sdt><w:sdtPr><w:id w:val="5"/></w:sdtPr><w:sdtContent>)"));
        QVERIFY(out.contains("</w:sdtContent></w:sdt>"));
        QVERIFY(out.contains(R"(<w:p w:rsidR="00A1">)"));
        QVERIFY(out.contains(R"(<w:sectPr w:rsidR="00B2">)"));
        QVERIFY(out.contains(R"(<w:headerReference w:type="default" r:id="rId8"/>)"));
        QVERIFY(out.contains(R"(<w:docGrid w:linePitch="360"/>)"));
        // Properties it has no field for come back, in the places the schema gives them.
        const int pStyle = out.indexOf("<w:pStyle"), framePr = out.indexOf("<w:framePr"), suppress = out.indexOf("<w:suppressAutoHyphens/>"), ctx = out.indexOf("<w:contextualSpacing/>");
        QVERIFY(pStyle >= 0 && pStyle < framePr && framePr < suppress && suppress < ctx);
        const int rStyle = out.indexOf("<w:rStyle"), b = out.indexOf("<w:b/>"), rtl = out.indexOf("<w:rtl/>"), lang = out.indexOf("<w:lang");
        QVERIFY(rStyle >= 0 && rStyle < b && b < rtl && rtl < lang);
        // And it reads back the same.
        Document again;
        QVERIFY(readDocx(writeDocx(d), &again));
        QCOMPARE(again.body.size(), d.body.size());
        QCOMPARE(again.body[1].raw, table);
        QCOMPARE(documentXmlOf(writeDocx(again)), out);
    }

    void aSliceKeepsThePrefixesAnInnerElementDeclared()
    {
        // The paragraph declares a prefix that its picture uses; the picture is kept with it.
        Made m;
        m.body = R"(<w:p xmlns:wp="urn:wp"><w:r><w:drawing><wp:inline/></w:drawing></w:r></w:p>)";
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        const QByteArray out = documentXmlOf(writeDocx(d));
        XmlTree t;
        QVERIFY2(t.parse(out), "the written document is well formed with its prefixes declared");
    }

    void piecesFromAnotherFileGetTheirPrefixesDeclared()
    {
        // A picture kept as XML came from a file whose root declared wp and a; put in a document whose root does not, the prefixes are declared.
        Document d = newDocument();
        Block b;
        Inline pic;
        pic.kind = Inline::RunAtom;
        pic.tag = QStringLiteral("drawing");
        pic.raw = "<w:drawing><wp:inline><wp:extent cx=\"1\" cy=\"1\"/><a:graphic><a:graphicData uri=\"x\"/></a:graphic></wp:inline></w:drawing>";
        b.para.content.push_back(pic);
        Inline mark;
        mark.kind = Inline::ParaAtom;
        mark.tag = QStringLiteral("x");
        mark.raw = "<w14:foo w14:val=\"1\"/>";
        b.para.content.push_back(mark);
        d.body = {b};
        const QByteArray out = documentXmlOf(writeDocx(d));
        XmlTree t;
        QString why;
        QVERIFY2(t.parse(out, &why), qPrintable(why));
        QVERIFY(out.contains("xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\""));
        QVERIFY(out.contains("xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\""));
        QVERIFY(out.contains("xmlns:w14="));
        QCOMPARE(count(out, "xmlns:wp="), 1);
        // A root that declares them already is left as it is.
        Document again;
        QVERIFY(readDocx(writeDocx(d), &again));
        QCOMPARE(count(documentXmlOf(writeDocx(again)), "xmlns:wp="), 1);
    }

    void unpairedPiecesAreLeftOut()
    {
        Document d;
        QVERIFY(readDocx(Made{R"(<w:p/>)", {}, {}, {}}.build(), &d));
        auto text = [](const QString &s) { Inline i; i.text = s; return i; };
        auto atom = [](Inline::Kind kind, const QByteArray &raw, const QString &tag, const QString &key, Inline::Role role = Inline::Single) {
            Inline i;
            i.kind = kind;
            i.raw = raw;
            i.tag = tag;
            i.key = key;
            i.role = role;
            return i;
        };
        Block b;
        b.para.content = {text(QStringLiteral("a")),
                          atom(Inline::ParaAtom, "<w:bookmarkStart w:id=\"1\" w:name=\"gone\"/>", QStringLiteral("bookmarkStart"), QStringLiteral("1")),   // no end
                          atom(Inline::ParaAtom, "<w:bookmarkEnd w:id=\"2\"/>", QStringLiteral("bookmarkEnd"), QStringLiteral("2")),                       // no start
                          atom(Inline::ParaAtom, "<w:bookmarkStart w:id=\"3\" w:name=\"kept\"/>", QStringLiteral("bookmarkStart"), QStringLiteral("3")),
                          atom(Inline::ParaAtom, "<w:bookmarkEnd w:id=\"3\"/>", QStringLiteral("bookmarkEnd"), QStringLiteral("3")),
                          atom(Inline::ParaAtom, "<w:hyperlink r:id=\"rId1\">", QStringLiteral("hyperlink"), QString(), Inline::Open),                 // never closed
                          text(QStringLiteral("b")),
                          atom(Inline::RunAtom, "<w:fldChar w:fldCharType=\"begin\"/>", QStringLiteral("fldChar"), QStringLiteral("begin")),             // the field never ends
                          atom(Inline::RunAtom, "<w:instrText>X</w:instrText>", QStringLiteral("instrText"), QString()),
                          atom(Inline::RunAtom, "<w:fldChar w:fldCharType=\"end\"/>", QStringLiteral("fldChar"), QStringLiteral("end")),                  // (ends the one above)
                          atom(Inline::RunAtom, "<w:fldChar w:fldCharType=\"end\"/>", QStringLiteral("fldChar"), QStringLiteral("end")),                  // an end with no field
                          atom(Inline::RunAtom, "<w:fldChar w:fldCharType=\"begin\"/>", QStringLiteral("fldChar"), QStringLiteral("begin")),             // never ended
                          atom(Inline::ParaAtom, "</w:hyperlink>", QStringLiteral("hyperlink"), QString(), Inline::Close)};                              // (a close of a different tag stack)
        d.body.push_back(b);
        const QByteArray out = documentXmlOf(writeDocx(d));
        QCOMPARE(count(out, "w:bookmarkStart"), 1);
        QVERIFY(out.contains("w:name=\"kept\""));
        QVERIFY(!out.contains("gone"));
        QCOMPARE(count(out, "<w:bookmarkEnd"), 1);
        QCOMPARE(count(out, "<w:fldChar"), 2);   // one field, begin and end
        QCOMPARE(count(out, "<w:instrText"), 1);
        // The hyperlink was opened and then closed after other pieces: its tags pair up, so both stay.
        QCOMPARE(count(out, "hyperlink"), 2);
    }

    void aNewDocumentIsWholeAndOpensAgain()
    {
        Document d = newDocument();
        QVERIFY(!d.styles.styles.isEmpty());
        QVERIFY(d.styles.find(QStringLiteral("Heading1")));
        const QByteArray zip = writeDocx(d);
        QVERIFY(!zip.isEmpty());
        Package p;
        QVERIFY(p.read(zip));
        QCOMPARE(p.mainPart(), QStringLiteral("/word/document.xml"));
        QVERIFY(p.hasPart(QStringLiteral("/word/styles.xml")));
        XmlTree t;
        for (const QString &name : p.partNames()) {
            if (name.endsWith(QLatin1String(".xml"))) QVERIFY2(t.parse(p.part(name)), qPrintable(name));
        }
        Document back;
        QString err;
        QVERIFY2(readDocx(zip, &back, &err), qPrintable(err));
        QCOMPARE(back.styles.styles.size(), d.styles.styles.size());
        QVERIFY(back.numbering.abstractOf(2));
        QCOMPARE(back.numbering.abstractOf(2)->levels[0].numFmt, QStringLiteral("decimal"));   // (list 2 is the numbered one, list 1 the bulleted one)
        QCOMPARE(back.numbering.abstractOf(1)->levels[1].text, QString::fromUtf8("◦"));
        QCOMPARE(back.styles.paragraph(QStringLiteral("Heading1")).outlineLvl, std::optional<int>(0));
        QVERIFY(back.hasTheme);
        QCOMPARE(back.defaultTabStop, 720);
    }

    void propertiesAreWrittenInTheSchemasOrder()
    {
        ParaProps p;
        p.jc = QStringLiteral("center");
        p.indLeft = 720;
        p.before = 120;
        p.numId = 2;
        p.ilvl = 1;
        p.keepNext = true;
        p.style = QStringLiteral("S");
        p.outlineLvl = 1;
        p.tabs = QVector<TabStop>{{1440, QStringLiteral("right"), QStringLiteral("dot")}};
        p.extra.push_back({paraChildRank("suppressLineNumbers"), "<w:suppressLineNumbers/>"});
        const QByteArray x = paraPropsXml(p);
        int last = -1;
        for (const char *name : {"<w:pStyle", "<w:keepNext", "<w:numPr", "<w:suppressLineNumbers", "<w:tabs", "<w:spacing", "<w:ind", "<w:jc", "<w:outlineLvl"}) {
            const int at = x.indexOf(name);
            QVERIFY2(at > last, name);
            last = at;
        }
        QVERIFY(x.indexOf("<w:ilvl") < x.indexOf("<w:numId"));
        RunProps r;
        r.size = 24;
        r.underline = Underline{QStringLiteral("single"), QString()};
        r.bold = true;
        r.fonts = Fonts{QStringLiteral("A"), QStringLiteral("A"), {}, {}, {}, {}, {}, {}};
        r.style = QStringLiteral("C");
        r.vertAlign = QStringLiteral("superscript");
        r.color = Color{QStringLiteral("00FF00"), {}, {}, {}};
        r.highlight = QStringLiteral("yellow");
        const QByteArray y = runPropsXml(r);
        last = -1;
        for (const char *name : {"<w:rStyle", "<w:rFonts", "<w:b/>", "<w:color", "<w:sz ", "<w:highlight", "<w:u ", "<w:vertAlign"}) {
            const int at = y.indexOf(name);
            QVERIFY2(at > last, name);
            last = at;
        }
        SectionProps s;
        s.type = QStringLiteral("continuous");
        s.titlePg = true;
        s.cols = 2;
        s.pageStart = 3;
        s.extra.push_back({sectionChildRank("headerReference"), "<w:headerReference w:type=\"first\" r:id=\"rId5\"/>"});
        s.extra.push_back({sectionChildRank("docGrid"), "<w:docGrid w:linePitch=\"360\"/>"});
        const QByteArray z = sectionPropsXml(s);
        last = -1;
        for (const char *name : {"<w:headerReference", "<w:type", "<w:pgSz", "<w:pgMar", "<w:pgNumType", "<w:cols", "<w:titlePg", "<w:docGrid"}) {
            const int at = z.indexOf(name);
            QVERIFY2(at > last, name);
            last = at;
        }
    }

    void differencesFromTheStyleAreWhatIsWrittenDirectly()
    {
        RunProps base;
        base.size = 22;
        base.bold = true;
        RunProps full = base;
        full.italic = false;       // (the default: not a difference)
        full.size = 22;
        RunProps d = full.differences(base);
        QVERIFY(!d.bold && !d.italic && !d.size);
        full.bold = false;         // turned off against a bold style: written, as off
        full.color = Color{QStringLiteral("FF0000"), {}, {}, {}};
        d = full.differences(base);
        QCOMPARE(d.bold, std::optional<bool>(false));
        QCOMPARE(d.color->val, QStringLiteral("FF0000"));
        ParaProps pb;
        pb.after = 160;
        ParaProps pf = pb;
        pf.jc = QStringLiteral("left");   // the default
        QVERIFY(pf.differences(pb).jc.isEmpty());
        pf.jc = QStringLiteral("center");
        pf.after = 0;
        const ParaProps pd = pf.differences(pb);
        QCOMPARE(pd.jc, QStringLiteral("center"));
        QCOMPARE(pd.after, std::optional<int>(0));
        QVERIFY(!pd.before);
    }

    void tabsAddAndClearAgainstTheStyles()
    {
        ParaProps base;
        base.tabs = QVector<TabStop>{{720, QStringLiteral("left"), QString()}, {1440, QStringLiteral("left"), QString()}};
        ParaProps top;
        top.tabs = QVector<TabStop>{{720, QStringLiteral("clear"), QString()}, {2880, QStringLiteral("right"), QStringLiteral("dot")}};
        ParaProps merged = base;
        merged.overlay(top);
        QCOMPARE(merged.tabs->size(), 2);
        QCOMPARE(merged.tabs->at(0).pos, 1440);
        QCOMPARE(merged.tabs->at(1).pos, 2880);
        const ParaProps d = merged.differences(base);
        QCOMPARE(d.tabs->size(), 2);
        QCOMPARE(d.tabs->at(0).pos, 2880);
        QCOMPARE(d.tabs->at(1).val, QStringLiteral("clear"));
    }

    void hostileDocumentsAreHeldToLimits()
    {
        QString err;
        Document d;
        // A document nested past the depth limit, one with a DOCTYPE, and a body that is not XML are refused with a message.
        QByteArray deep;
        for (int i = 0; i < kMaxXmlDepth + 10; ++i) deep += "<w:p>";
        for (int i = 0; i < kMaxXmlDepth + 10; ++i) deep += "</w:p>";
        QVERIFY(!readDocx(Made{deep, {}, {}, {}}.build(), &d, &err));
        QVERIFY(!err.isEmpty());
        {
            PackageWriter w;
            w.addPart(QStringLiteral("/word/document.xml"), "<!DOCTYPE d [<!ENTITY a \"aaaa\">]><w:document " + QByteArray(kW) + "><w:body><w:p><w:r><w:t>&a;</w:t></w:r></w:p></w:body></w:document>",
                      QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
            w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
            QVERIFY(!readDocx(w.finish(), &d, &err));
        }
        QVERIFY(!readDocx(QByteArray("not a zip"), &d, &err));
        // A file that is not a document in a package is refused too.
        {
            PackageWriter w;
            w.addPart(QStringLiteral("/word/document.xml"), xmlDoc("<a/>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
            w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
            QVERIFY(!readDocx(w.finish(), &d, &err));
        }
        // Two hundred thousand paragraphs, each with runs, read in a time a person would wait, and written again.
        QByteArray many;
        many.reserve(200000 * 80);
        for (int i = 0; i < 200000; ++i) many += "<w:p><w:r><w:rPr><w:b/></w:rPr><w:t>word</w:t></w:r></w:p>";
        QElapsedTimer timer;
        timer.start();
        QVERIFY(readDocx(Made{many, {}, {}, {}}.build(), &d, &err));
        QCOMPARE(d.body.size(), 200000);
        QVERIFY2(timer.elapsed() < 20000, "reading 200,000 paragraphs");
        QVERIFY(!writeDocx(d).isEmpty());
        // More elements than the limit allows are refused before the memory goes: 8,000,001 empty ones.
        QByteArray tooMany;
        tooMany.reserve(8000100 * 6);
        tooMany += "<w:p>";
        for (int i = 0; i < 8000001; ++i) tooMany += "<w:r/>";
        tooMany += "</w:p>";
        QVERIFY(!readDocx(Made{tooMany, {}, {}, {}}.build(), &d, &err));
        QVERIFY(err.contains(QLatin1String("elements")));
        tooMany.clear();
        // A body longer than the limit of paragraphs is read as far as the limit, with a note, not refused.
        QByteArray longBody;
        longBody.reserve((kMaxParagraphs + 10) * 12);
        for (int i = 0; i < kMaxParagraphs + 10; ++i) longBody += "<w:p/>";
        ReadNotes notes;
        QVERIFY(readDocx(Made{longBody, {}, {}, {}}.build(), &d, &err, &notes));
        QCOMPARE(d.body.size(), kMaxParagraphs);
        QCOMPARE(notes.notes.size(), 1);
        // Sizes a file states are held to what a page can have, and a style that is its own ancestor is no loop.
        Made sizes;
        sizes.body = R"(<w:p><w:pPr><w:ind w:left="99999999999"/></w:pPr><w:r><w:rPr><w:sz w:val="9999999"/></w:rPr><w:t>x</w:t></w:r></w:p><w:sectPr><w:pgSz w:w="2000000000" w:h="1"/><w:cols w:num="999999"/></w:sectPr>)";
        QVERIFY(readDocx(sizes.build(), &d, &err));
        QVERIFY(d.body[0].para.props.indLeft.value() <= 1000000);
        QVERIFY(d.body[0].para.content[0].props.size.value() <= 4000);
        QVERIFY(d.lastSection.pageW <= 63360 && d.lastSection.pageH >= 720);
        QVERIFY(d.lastSection.cols <= 45);
        // Containers nested a hundred deep are not read into past a small depth, and are still kept.
        QByteArray nest = "<w:p>";
        for (int i = 0; i < 100; ++i) nest += "<w:smartTag w:uri=\"u\" w:element=\"e\">";
        nest += "<w:r><w:t>deep</w:t></w:r>";
        for (int i = 0; i < 100; ++i) nest += "</w:smartTag>";
        nest += "</w:p>";
        QVERIFY(readDocx(Made{nest, {}, {}, {}}.build(), &d, &err));
        const QByteArray written = documentXmlOf(writeDocx(d));
        QCOMPARE(count(written, "<w:smartTag"), 100);
        QVERIFY(written.contains("deep"));
    }
};

QTEST_MAIN(WmlTests)
#include "tst_wml.moc"
