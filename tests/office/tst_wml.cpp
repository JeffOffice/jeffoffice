// Tests of libs/office/wml: the tree the word processing parts are read into, and reading and writing a .docx.

#include "../timing.h"
#include "office/package.h"
#include "office/xml.h"
#include "wml/wml.h"
#include "wml/xmltree.h"

#include "core/fonts.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
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

    // A symbol (w:sym) of a symbol font that this machine lacks shows as the picture it is: a degree sign from Symbol, an arrow from Wingdings, a banner's block from
    // Webdings; one of any other font, or a symbol that has no known picture, shows a box. The element is kept as written.
    void aSymbolOfAMissingSymbolFontShowsItsPicture()
    {
        for (const char *family : {"Symbol", "Wingdings", "Webdings"})
            if (QFontDatabase::hasFamily(QString::fromLatin1(family))) QSKIP("a symbol font is installed here: its symbols are not made into pictures");
        Made m;
        m.body = R"(<w:p><w:r><w:sym w:font="Symbol" w:char="F0B0"/></w:r><w:r><w:sym w:font="Wingdings" w:char="F0EA"/></w:r><w:r><w:sym w:font="Webdings" w:char="F067"/></w:r>)"
                 R"(<w:r><w:sym w:font="No Such Face" w:char="F0B0"/></w:r><w:r><w:sym w:font="Wingdings" w:char="F021"/></w:r></w:p>)";
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        QCOMPARE(d.body.size(), 1);
        QStringList shown;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("sym")) shown << in.display;
        QCOMPARE(shown, (QStringList{QString(QChar(0x00B0)), QString(QChar(0x2B07)), QString(QChar(0x2588)), QString(QChar(0x25A1)), QString(QChar(0x25A1))}));
        QVERIFY(d.body[0].para.content[0].raw.contains("F0B0"));
    }

    // Where the symbol font is installed, the symbol is not made into a picture: it is the character the file has (U+F0B0, in the font's own range), which the font draws.
    void aSymbolOfAnInstalledSymbolFontIsTheFilesOwnCharacter()
    {
        QString path;
        for (const QString &dir : jp::bundledFontDirs())
            if (QFileInfo::exists(dir + QStringLiteral("/Oswald-Regular.ttf"))) path = dir + QStringLiteral("/Oswald-Regular.ttf");
        if (path.isEmpty()) QSKIP("the bundled fonts are not beside the test");
        if (QFontDatabase::hasFamily(QStringLiteral("Symbol"))) QSKIP("Symbol is installed here already");
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QByteArray font = f.readAll();
        // (Oswald, renamed Symbol: six letters for six.)
        font.replace(QByteArray("\x00" "O\x00s\x00w\x00" "a\x00l\x00" "d", 12), QByteArray("\x00" "S\x00y\x00m\x00" "b\x00o\x00l", 12));
        font.replace(QByteArray("Oswald"), QByteArray("Symbol"));
        const int id = QFontDatabase::addApplicationFontFromData(font);
        QVERIFY(id >= 0);
        jp::fontsChanged();
        QString shown;
        const bool installed = QFontDatabase::hasFamily(QStringLiteral("Symbol"));
        if (installed) {
            Made m;
            m.body = R"(<w:p><w:r><w:sym w:font="Symbol" w:char="F0B0"/></w:r></w:p>)";
            Document d;
            QVERIFY(readDocx(m.build(), &d));
            for (const Inline &in : d.body[0].para.content)
                if (in.kind == Inline::RunAtom && in.tag == QLatin1String("sym")) shown = in.display;
        }
        QFontDatabase::removeApplicationFont(id);
        jp::fontsChanged();
        QVERIFY(installed);
        QCOMPARE(shown, QString(QChar(0xF0B0)));
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

    // Text that is not marked to keep its blanks has none at its ends (a space at the end of one run and the start of the next is no space between them); text that is
    // marked keeps every one, and the blanks inside are the text's own either way.
    void textWithoutPreserveLosesTheBlanksAtItsEnds()
    {
        Made m;
        m.body = R"(<w:p><w:r><w:t>Before </w:t></w:r><w:r><w:t> inside  twice </w:t></w:r><w:r><w:t xml:space="preserve"> kept </w:t></w:r><w:r><w:t> </w:t></w:r><w:r><w:t>After</w:t></w:r></w:p>)";
        Document d;
        QString err;
        QVERIFY2(readDocx(m.build(), &d, &err), qPrintable(err));
        QCOMPARE(d.body.size(), 1);
        QString all;
        for (const Inline &in : d.body[0].para.content) all += in.text;
        QCOMPARE(all, QStringLiteral("Beforeinside  twice kept After"));
    }

    // Ruby text is shown with its base text where the pages cannot set it above: the base, and the ruby after it in brackets (an atom that is kept as it is written).
    void rubyShowsItsBaseTextAndItsRuby()
    {
        Made m;
        m.body = R"(<w:p><w:r><w:ruby><w:rubyPr><w:rubyAlign w:val="distributeSpace"/></w:rubyPr><w:rt><w:r><w:t>とうきょう</w:t></w:r></w:rt><w:rubyBase><w:r><w:t>東京</w:t></w:r></w:rubyBase></w:ruby></w:r></w:p>)";
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        QString shown;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("ruby")) shown = in.display;
        QCOMPARE(shown, QString::fromUtf8("東京(とうきょう)"));
        QVERIFY(documentXmlOf(writeDocx(d)).contains("<w:ruby>"));
    }

    // The ruby of a file that is written with its elements on lines of their own is the text of its runs, as the text of runs is read (the blanks between the elements are not
    // text).
    void rubyOfAnIndentedFileHasNoIndentationInIt()
    {
        Made m;
        m.body = "<w:p><w:r><w:ruby>\n  <w:rubyPr><w:rubyAlign w:val=\"center\"/></w:rubyPr>\n  <w:rt>\n    <w:r>\n      <w:t>\xe3\x81\xa8\xe3\x81\x86</w:t>\n    </w:r>\n  </w:rt>\n  <w:rubyBase>\n    <w:r>\n      <w:t>\xe6\x9d\xb1\xe4\xba\xac</w:t>\n    </w:r>\n  </w:rubyBase>\n</w:ruby></w:r></w:p>";
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        QString shown;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("ruby")) shown = in.display;
        QCOMPARE(shown, QString::fromUtf8("\xe6\x9d\xb1\xe4\xba\xac(\xe3\x81\xa8\xe3\x81\x86)"));
    }

    // An equation (m:oMath, m:oMathPara) shows every character it has, in order, as a line of text on the baseline: a fraction is its numerator and then its denominator, a script comes
    // after its base, a delimiter's characters round what is in it, an n-ary operator is followed by its limits and then what it works on, and a matrix goes row by row. The
    // element itself is kept as it was written.
    void anEquationShowsItsCharactersInOrderAsALine()
    {
        const QByteArray ns = "xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"";
        auto run = [](const QByteArray &text) { return "<m:r><m:t>" + text + "</m:t></m:r>"; };
        auto arg = [&](const char *name, const QByteArray &text) { return QByteArray("<m:") + name + ">" + run(text) + "</m:" + name + ">"; };
        auto shown = [&](const QByteArray &inner, bool para = false) {
            Made m;
            m.body = "<w:p>" + QByteArray(para ? "<m:oMathPara " : "<m:oMath ") + ns + ">" + inner + (para ? "</m:oMathPara>" : "</m:oMath>") + "</w:p>";
            Document d;
            if (!readDocx(m.build(), &d)) return QStringLiteral("READ FAILED");
            QString out = QStringLiteral("NO EQUATION");
            for (const Inline &in : d.body[0].para.content)
                if (in.kind == Inline::ParaAtom && (in.tag == QLatin1String("oMath") || in.tag == QLatin1String("oMathPara"))) out = in.display;
            if (!documentXmlOf(writeDocx(d)).contains(m.body.mid(5, m.body.size() - 11))) out = QStringLiteral("NOT KEPT: ") + out;
            return out;
        };
        QCOMPARE(shown("<m:f>" + arg("num", "a") + arg("den", "b") + "</m:f>"), QStringLiteral("a/b"));
        QCOMPARE(shown("<m:f>" + arg("num", "x+1") + arg("den", "y") + "</m:f>"), QStringLiteral("(x + 1)/y"));
        QCOMPARE(shown("<m:sSup>" + arg("e", "x") + arg("sup", "2") + "</m:sSup>"), QStringLiteral("x2"));
        QCOMPARE(shown("<m:sSub>" + arg("e", "x") + arg("sub", "i") + "</m:sSub>"), QStringLiteral("xi"));
        QCOMPARE(shown("<m:sSubSup>" + arg("e", "x") + arg("sub", "i") + arg("sup", "n") + "</m:sSubSup>"), QStringLiteral("xin"));
        QCOMPARE(shown("<m:sPre>" + arg("sub", "1") + arg("sup", "2") + arg("e", "C") + "</m:sPre>"), QStringLiteral("12C"));
        QCOMPARE(shown("<m:rad><m:radPr><m:degHide m:val=\"1\"/></m:radPr><m:deg/>" + arg("e", "x+1") + "</m:rad>"), QStringLiteral("\u221a(x + 1)"));
        QCOMPARE(shown("<m:rad>" + arg("deg", "3") + arg("e", "x") + "</m:rad>"), QStringLiteral("3\u221ax"));
        QCOMPARE(shown("<m:nary><m:naryPr><m:chr m:val=\"\u2211\"/></m:naryPr>" + arg("sub", "i=1") + arg("sup", "n") + arg("e", "x") + "</m:nary>"), QStringLiteral("\u2211 (i = 1) n x"));
        QCOMPARE(shown("<m:nary><m:naryPr><m:subHide m:val=\"1\"/><m:supHide m:val=\"1\"/></m:naryPr><m:sub/><m:sup/>" + arg("e", "f(x)dx") + "</m:nary>"), QStringLiteral("\u222b f(x)dx"));
        QCOMPARE(shown("<m:d><m:dPr><m:begChr m:val=\"[\"/><m:sepChr m:val=\",\"/><m:endChr m:val=\"]\"/></m:dPr>" + arg("e", "a") + arg("e", "b") + "</m:d>"), QStringLiteral("[a,b]"));
        QCOMPARE(shown("<m:d>" + arg("e", "a") + "</m:d>"), QStringLiteral("(a)"));
        QCOMPARE(shown("<m:func>" + arg("fName", "sin") + arg("e", "x") + "</m:func>"), QStringLiteral("sin x"));
        QCOMPARE(shown("<m:limLow>" + arg("e", "lim") + arg("lim", "x\u21920") + "</m:limLow>"), QStringLiteral("lim x \u2192 0"));
        QCOMPARE(shown("<m:m><m:mr>" + arg("e", "a") + arg("e", "b") + "</m:mr><m:mr>" + arg("e", "c") + arg("e", "d") + "</m:mr></m:m>"), QStringLiteral("a b; c d"));
        // An equation of its own paragraph holds equations that are lines of their own.
        QCOMPARE(shown("<m:oMath>" + run("a") + "</m:oMath><m:oMath>" + run("b=c") + "</m:oMath>", true), QStringLiteral("a\u2028b = c"));
        // Operators have blanks round them where they work on two things, and none where they sign one.
        QCOMPARE(shown(run("a+b=-c")), QStringLiteral("a + b = -c"));
        // A run's own text of normal letters (w:t in an m:r) counts as well.
        QCOMPARE(shown("<m:r><m:rPr><m:nor/></m:rPr><w:t>if</w:t></m:r>" + run("x")), QStringLiteral("ifx"));
    }

    // The characters of the mathematical alphanumeric block (U+1D400 to U+1D7FF) are shown as the letters and digits they are styles of, and how most of an equation's letters are
    // set (italic, bold, both, or neither) is kept with it.
    void anEquationsStyledLettersAreShownAsTheirLetters()
    {
        const QByteArray ns = "xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"";
        auto read = [&](const QByteArray &text, QString *style) {
            Made m;
            m.body = "<w:p><m:oMath " + ns + "><m:r><m:t>" + text + "</m:t></m:r></m:oMath></w:p>";
            Document d;
            if (!readDocx(m.build(), &d)) return QStringLiteral("READ FAILED");
            for (const Inline &in : d.body[0].para.content)
                if (in.kind == Inline::ParaAtom && in.tag == QLatin1String("oMath")) {
                    *style = in.key;
                    return in.display;
                }
            return QStringLiteral("NO EQUATION");
        };
        QString style;
        QCOMPARE(read(QString::fromUtf8("\xf0\x9d\x91\xa5=\xf0\x9d\x9b\xbc+\xf0\x9d\x9f\x91").toUtf8(), &style), QStringLiteral("x = \u03b1 + 3"));   // (math italic x, italic alpha, bold digit 3)
        QCOMPARE(style, QStringLiteral("i"));
        QCOMPARE(read(QString::fromUtf8("\xf0\x9d\x90\x80\xf0\x9d\x90\x81").toUtf8(), &style), QStringLiteral("AB"));   // (bold A and B)
        QCOMPARE(style, QStringLiteral("b"));
        QCOMPARE(read("xy", &style), QStringLiteral("xy"));   // (letters a run has no style for are italic, as an equation's are)
        QCOMPARE(style, QStringLiteral("i"));
        QCOMPARE(read("12", &style), QStringLiteral("12"));
        QCOMPARE(style, QString());
    }

    // A mathematical alphanumeric in a paragraph's text (as a run in Cambria Math has it) is kept as the text it is, written back as it was, and shown as its letter: an atom that holds the
    // characters and shows the letters, its style in its key (italic i, bold b, bold italic ib; d for double-struck, which is shown as it is: a face has it).
    void aStyledLetterInTextIsKeptAndShownAsItsLetter()
    {
        Made m;
        m.body = QString::fromUtf8("<w:p><w:r><w:rPr><w:b/></w:rPr><w:t>a \xf0\x9d\x91\x86 b \xf0\x9d\x90\x80\xf0\x9d\x90\x81 c \xf0\x9d\x94\xb8</w:t></w:r></w:p>").toUtf8();
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        QStringList parts;
        for (const Inline &in : d.body[0].para.content) {
            if (in.kind == Inline::Text) parts << QStringLiteral("T:") + in.text;
            else if (in.kind == Inline::RunAtom && in.tag == QLatin1String("t")) parts << QStringLiteral("A:") + in.display + QLatin1Char('/') + in.key;
        }
        QCOMPARE(parts, (QStringList{QStringLiteral("T:a "), QStringLiteral("A:S/i"), QStringLiteral("T: b "), QStringLiteral("A:AB/b"), QStringLiteral("T: c "), QString::fromUtf8("A:\xf0\x9d\x94\xb8/d")}));
        const QByteArray written = documentXmlOf(writeDocx(d));
        QVERIFY(written.contains("\xf0\x9d\x91\x86") && written.contains("\xf0\x9d\x90\x80\xf0\x9d\x90\x81") && written.contains("\xf0\x9d\x94\xb8"));
        Document again;
        QVERIFY(readDocx(writeDocx(d), &again));
        QCOMPARE(documentXmlOf(writeDocx(again)), written);
    }

    // What a file controls about an equation is bounded: how deep it nests (what is deeper is left out of the line), and how much text the line holds.
    void anEquationsDepthAndSizeAreBounded()
    {
        const QByteArray ns = "xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"";
        QByteArray nested = "<m:r><m:t>x</m:t></m:r>";
        for (int i = 0; i < 200; ++i) nested = "<m:sSup><m:e>" + nested + "</m:e><m:sup><m:r><m:t>y</m:t></m:r></m:sup></m:sSup>";
        Made m;
        m.body = "<w:p><m:oMath " + ns + ">" + nested + "</m:oMath></w:p><w:p><m:oMath " + ns + "><m:r><m:t>" + QByteArray(100000, 'z') + "</m:t></m:r></m:oMath></w:p>";
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        QVERIFY(d.body.size() >= 2);
        for (int i = 0; i < 2; ++i) {
            QString shown;
            for (const Inline &in : d.body[i].para.content)
                if (in.kind == Inline::ParaAtom && in.tag == QLatin1String("oMath")) shown = in.display;
            QVERIFY2(!shown.isEmpty() && shown.size() <= kMaxEquationChars, qPrintable(QStringLiteral("equation %1 shows %2 characters").arg(i).arg(shown.size())));
        }
        QVERIFY(documentXmlOf(writeDocx(d)).contains(nested.left(200)));   // (kept whole, whatever was drawn)
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

    // A section names its headers and footers by relationship Id, a slot each for the first page, the ordinary pages, and the even pages.
    void sectionsNameTheirHeadersAndFooters()
    {
        Made m;
        m.body = R"(<w:p><w:pPr><w:sectPr><w:headerReference w:type="default" r:id="rId11"/><w:headerReference w:type="first" r:id="rId12"/><w:footerReference w:type="even" r:id="rId13"/><w:footerReference w:type="default" r:id="rId14"/><w:titlePg/><w:pgSz w:w="12240" w:h="15840"/></w:sectPr></w:pPr><w:r><w:t>one</w:t></w:r></w:p><w:p><w:r><w:t>two</w:t></w:r></w:p>)";
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        const HeaderRefs refs = headerRefsOf(*d.body[0].para.props.section);
        QCOMPARE(refs.header[0], QStringLiteral("rId11"));
        QCOMPARE(refs.header[1], QStringLiteral("rId12"));
        QCOMPARE(refs.header[2], QString());
        QCOMPARE(refs.footer[0], QStringLiteral("rId14"));
        QCOMPARE(refs.footer[1], QString());
        QCOMPARE(refs.footer[2], QStringLiteral("rId13"));
        QVERIFY(refs.any());
        // (A section that names none, and one that was written with the references kept.)
        QVERIFY(!headerRefsOf(d.lastSection).any());
        QVERIFY(d.body[0].para.props.section->titlePg);
        const QByteArray out = documentXmlOf(writeDocx(d));
        QCOMPARE(count(out, "headerReference"), 2);
        QCOMPARE(count(out, "footerReference"), 2);
    }

    // The paragraphs of a header, footer, or note part are read as the body's are.
    void readsTheParagraphsOfAHeaderPart()
    {
        const QByteArray part = xmlDoc(QByteArray("<w:hdr ") + kW + R"(><w:p><w:pPr><w:pStyle w:val="Header"/><w:jc w:val="center"/></w:pPr><w:r><w:t>Top</w:t></w:r></w:p><w:tbl><w:tr><w:tc><w:p/></w:tc></w:tr></w:tbl><w:p><w:r><w:t>Bottom</w:t></w:r></w:p></w:hdr>)");
        QVector<Block> blocks;
        QVERIFY(readBlocks(part, &blocks));
        QCOMPARE(int(blocks.size()), 3);
        QVERIFY(!blocks[0].isRaw);
        QCOMPARE(blocks[0].para.props.style, QStringLiteral("Header"));
        QCOMPARE(blocks[0].para.content.first().text, QStringLiteral("Top"));
        QVERIFY(blocks[1].isRaw);
        QCOMPARE(blocks[1].tag, QStringLiteral("tbl"));
        QCOMPARE(blocks[2].para.content.first().text, QStringLiteral("Bottom"));
        QVERIFY(!readBlocks("<w:hdr", &blocks));
    }

    // The drawn objects of a run: the pictures, rules, boxes, and text boxes of VML, with where they are set and how text goes round them; and the
    // pictures of DrawingML.
    void readsTheShapesOfARun()
    {
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
                              "xmlns:w10=\"urn:schemas-microsoft-com:office:word\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                              "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                              "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"";
        // A picture in the line.
        QVector<Shape> shapes = readShapes("<w:pict " + ns + "><v:shapetype id='t75'/><v:shape id='s1' type='#t75' style='width:54pt;height:36pt'><v:imagedata r:id='rId4' o:title=''/></v:shape></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].kind, Shape::Picture);
        QVERIFY(!shapes[0].floating);
        QCOMPARE(shapes[0].embed, QStringLiteral("rId4"));
        QCOMPARE(shapes[0].width, 54.0);
        QCOMPARE(shapes[0].height, 36.0);
        // A picture set at a place, with the text going round it.
        shapes = readShapes("<w:pict " + ns + "><v:shape id='s2' style='position:absolute;margin-left:228.7pt;margin-top:61.25pt;width:198pt;height:142.5pt;z-index:251654144;mso-position-horizontal-relative:margin;mso-position-vertical-relative:page'><v:imagedata r:id='rId8'/><w10:wrap type='tight'/></v:shape></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(shapes[0].floating);
        QCOMPARE(shapes[0].left, 228.7);
        QCOMPARE(shapes[0].top, 61.25);
        QCOMPARE(shapes[0].hRel, Shape::Margin);
        QCOMPARE(shapes[0].vRel, Shape::Page);
        QCOMPARE(shapes[0].wrap, Shape::Tight);
        QVERIFY(!shapes[0].behind);
        // A picture with a negative z-index and a wrap that keeps text off is not behind the text (the text goes round it); one with no wrap is.
        shapes = readShapes("<w:pict " + ns + "><v:shape id='s8' style='position:absolute;margin-left:180pt;margin-top:5.1pt;width:309.7pt;height:220.5pt;z-index:-251657728'><v:imagedata r:id='rId8'/><w10:wrap type='tight'/></v:shape></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].wrap, Shape::Tight);
        QVERIFY(!shapes[0].behind);
        shapes = readShapes("<w:pict " + ns + "><v:shape id='s9' style='position:absolute;margin-left:180pt;margin-top:5.1pt;width:309.7pt;height:220.5pt;z-index:-251657728'><v:imagedata r:id='rId8'/></v:shape></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].wrap, Shape::WrapNone);
        QVERIFY(shapes[0].behind);
        shapes = readShapes("<w:pict " + ns + "><v:shape id='s9b' style='position:absolute;margin-left:180pt;margin-top:5.1pt;width:309.7pt;height:220.5pt;z-index:-251657728'><v:imagedata r:id='rId8'/><w10:wrap type='topAndBottom'/></v:shape></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].wrap, Shape::TopBottom);
        QVERIFY(shapes[0].behind);
        // A shape filled with a picture stretched over it is what it is, with the picture as its fill: a box keeps its outline, and a text box its text (a tiled fill is not read).
        shapes = readShapes("<w:pict " + ns + "><v:rect id='s6' style='position:absolute;margin-left:-40pt;margin-top:1.8pt;width:81pt;height:45pt;z-index:251655168' stroked='f'><v:fill r:id='rId1' o:title='logo' type='frame'/></v:rect></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].kind, Shape::Rect);
        QCOMPARE(shapes[0].fillEmbed, QStringLiteral("rId1"));
        QVERIFY(shapes[0].floating && !shapes[0].filled && !shapes[0].stroked);
        shapes = readShapes("<w:pict " + ns + "><v:rect id='s6b' style='position:absolute;width:81pt;height:45pt' strokecolor='#0000FF' strokeweight='2pt'><v:fill r:id='rId1' type='frame'/></v:rect></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].kind, Shape::Rect);
        QCOMPARE(shapes[0].fillEmbed, QStringLiteral("rId1"));
        QVERIFY(!shapes[0].filled && shapes[0].stroked);
        QCOMPARE(shapes[0].strokeColor, QStringLiteral("0000FF"));
        QCOMPARE(shapes[0].strokeWeight, 2.0);
        shapes = readShapes("<w:pict " + ns + "><v:rect id='s6c' style='position:absolute;width:81pt;height:45pt'><v:fill r:id='rId1' type='frame'/><v:textbox><w:txbxContent><w:p><w:r><w:t>Over a picture</w:t></w:r></w:p></w:txbxContent></v:textbox></v:rect></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].kind, Shape::TextBox);
        QVERIFY(shapes[0].textBox.contains("Over a picture"));
        QCOMPARE(shapes[0].fillEmbed, QStringLiteral("rId1"));
        shapes = readShapes("<w:pict " + ns + "><v:rect id='s7' style='position:absolute;width:81pt;height:45pt' fillcolor='red'><v:fill r:id='rId1' type='tile'/></v:rect></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].kind, Shape::Rect);
        QVERIFY(shapes[0].fillEmbed.isEmpty());
        // A rule drawn as a filled rectangle behind the text, and a line from point to point.
        shapes = readShapes("<w:pict " + ns + "><v:rect id='s3' style='position:absolute;margin-left:1.5pt;margin-top:-2.25pt;width:543.75pt;height:1.4pt;z-index:-251657216' fillcolor='black' stroked='f' strokeweight='0'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].kind, Shape::Rect);
        QVERIFY(shapes[0].behind && shapes[0].filled && !shapes[0].stroked);
        QCOMPARE(shapes[0].fillColor, QStringLiteral("000000"));
        QCOMPARE(shapes[0].top, -2.25);
        shapes = readShapes("<w:pict " + ns + "><v:line id='s4' style='position:absolute;left:0;text-align:left;z-index:5' from='-4.95pt,18.2pt' to='481.05pt,18.2pt' strokeweight='1.25pt'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].kind, Shape::Line);
        QCOMPARE(shapes[0].x1, -4.95);
        QCOMPARE(shapes[0].x2, 481.05);
        QCOMPARE(shapes[0].y1, 18.2);
        QCOMPARE(shapes[0].strokeWeight, 1.25);
        QVERIFY(shapes[0].stroked);
        // A text box: its content as written, its insets, and no fill or line when it says so.
        shapes = readShapes("<w:pict " + ns + "><v:shape id='s5' type='#t202' style='position:absolute;margin-left:171.75pt;margin-top:.6pt;width:206.35pt;height:18pt' filled='f' stroked='f'><v:textbox inset='0,0,0,0'><w:txbxContent><w:p><w:r><w:t>Inside</w:t></w:r></w:p></w:txbxContent></v:textbox></v:shape></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].kind, Shape::TextBox);
        QVERIFY(!shapes[0].filled && !shapes[0].stroked);
        QVERIFY(shapes[0].textBox.contains("Inside"));
        QCOMPARE(shapes[0].inset[0], 0.0);
        // A picture of DrawingML, set behind the text at a place from the page's edge.
        shapes = readShapes("<w:drawing " + ns + "><wp:anchor behindDoc='1' distT='0' distB='0' distL='114300' distR='114300'><wp:positionH relativeFrom='page'><wp:posOffset>127000</wp:posOffset></wp:positionH><wp:positionV relativeFrom='paragraph'><wp:align>top</wp:align></wp:positionV><wp:extent cx='2540000' cy='1270000'/><wp:wrapSquare wrapText='bothSides'/><wp:docPr id='1' name='p' descr='a logo'/><a:graphic><a:graphicData><pic:pic><pic:blipFill><a:blip r:embed='rId7'/></pic:blipFill></pic:pic></a:graphicData></a:graphic></wp:anchor></w:drawing>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(shapes[0].floating && !shapes[0].behind);   // (a square wrap casts a wall whatever the order: the text goes round it)
        QCOMPARE(shapes[0].embed, QStringLiteral("rId7"));
        QCOMPARE(shapes[0].width, 200.0);
        QCOMPARE(shapes[0].height, 100.0);
        QCOMPARE(shapes[0].left, 10.0);
        QCOMPARE(shapes[0].hRel, Shape::Page);
        QCOMPARE(shapes[0].vAlign, Shape::Start);
        QCOMPARE(shapes[0].wrap, Shape::Square);
        QCOMPARE(shapes[0].gapLeft, 9.0);
        QCOMPARE(shapes[0].alt, QStringLiteral("a logo"));
        QCOMPARE(shapes[0].z, 0);
        // Behind the text with the other wraps: tight is a wall as square is; one that keeps text off by its top and bottom, or none at all, stays behind the text.
        auto behindWith = [&](const QByteArray &wrap) {
            const QVector<Shape> one = readShapes("<w:drawing " + ns + "><wp:anchor behindDoc='1' distT='0' distB='0' distL='0' distR='0'><wp:positionH relativeFrom='page'><wp:posOffset>0</wp:posOffset></wp:positionH><wp:positionV relativeFrom='page'><wp:posOffset>0</wp:posOffset></wp:positionV><wp:extent cx='254000' cy='127000'/>" + wrap +
                                                  "<wp:docPr id='1' name='p'/><a:graphic><a:graphicData><pic:pic><pic:blipFill><a:blip r:embed='rId7'/></pic:blipFill></pic:pic></a:graphicData></a:graphic></wp:anchor></w:drawing>");
            return one.size() == 1 ? int(one[0].behind) : -1;
        };
        QCOMPARE(behindWith("<wp:wrapTight wrapText='bothSides'/>"), 0);
        QCOMPARE(behindWith("<wp:wrapTopAndBottom/>"), 1);
        QCOMPARE(behindWith("<wp:wrapNone/>"), 1);
        // Its height among the objects.
        shapes = readShapes("<w:drawing " + ns + "><wp:anchor behindDoc='0' relativeHeight='251658240' distT='0' distB='0' distL='0' distR='0'><wp:positionH relativeFrom='page'><wp:posOffset>0</wp:posOffset></wp:positionH><wp:positionV relativeFrom='page'><wp:posOffset>0</wp:posOffset></wp:positionV><wp:extent cx='254000' cy='127000'/><wp:wrapNone/><wp:docPr id='1' name='p'/><a:graphic><a:graphicData><pic:pic><pic:blipFill><a:blip r:embed='rId7'/></pic:blipFill></pic:pic></a:graphicData></a:graphic></wp:anchor></w:drawing>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].z, 251658240);
        // A group's members, in its units: a coordinate space of 1000 by 500 over 100 by 50 points.
        shapes = readShapes("<w:pict " + ns + "><v:group id='g' style='position:absolute;margin-left:10pt;margin-top:20pt;width:100pt;height:50pt' coordsize='1000,500' coordorigin='0,0'><v:rect id='m' style='position:absolute;left:100;top:50;width:200;height:100' fillcolor='red'/></v:group></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(std::abs(shapes[0].left - 20) < 0.01 && std::abs(shapes[0].top - 25) < 0.01 && std::abs(shapes[0].width - 20) < 0.01 && std::abs(shapes[0].height - 10) < 0.01);
        QVERIFY(readShapes("not xml").isEmpty());
    }

    // A group inside a group: each shape of it is read once (the members of the outer group are not read again for each group in it), in the units of the groups it is in: the
    // outer group's coordinate space is 1000 by 500 over 100 by 50 points, the inner one's 40 by 20 over 400 by 200 of the outer's units.
    void readsEachShapeOfAGroupInAGroupOnce()
    {
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:v=\"urn:schemas-microsoft-com:vml\"";
        const QVector<Shape> shapes = readShapes("<w:pict " + ns + "><v:group id='g' style='position:absolute;margin-left:10pt;margin-top:20pt;width:100pt;height:50pt' coordsize='1000,500' coordorigin='0,0'>"
                                                 "<v:rect id='a' style='position:absolute;left:100;top:50;width:200;height:100' fillcolor='red'/>"
                                                 "<v:group id='n' style='position:absolute;left:400;top:200;width:400;height:200' coordsize='40,20' coordorigin='0,0'>"
                                                 "<v:rect id='b' style='position:absolute;left:10;top:5;width:20;height:10' fillcolor='blue'/></v:group>"
                                                 "<v:rect id='c' style='position:absolute;left:600;top:50;width:100;height:100' fillcolor='green'/></v:group></w:pict>");
        QCOMPARE(int(shapes.size()), 3);
        QVERIFY(std::abs(shapes[0].left - 20) < 0.01 && std::abs(shapes[0].top - 25) < 0.01 && std::abs(shapes[0].width - 20) < 0.01 && std::abs(shapes[0].height - 10) < 0.01);
        QVERIFY(std::abs(shapes[1].left - 60) < 0.01 && std::abs(shapes[1].top - 45) < 0.01 && std::abs(shapes[1].width - 20) < 0.01 && std::abs(shapes[1].height - 10) < 0.01);
        QVERIFY(std::abs(shapes[2].left - 70) < 0.01 && std::abs(shapes[2].top - 25) < 0.01);
    }

    // A line's turn and mirror are in its ends (a line has no frame of its own to turn about at draw time): a quarter turn
    // clockwise of a level segment about its middle makes a plumb one; a mirror of a steep one swaps its ends.
    void readsALineTurned()
    {
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:v=\"urn:schemas-microsoft-com:vml\"";
        QVector<Shape> shapes = readShapes("<w:pict " + ns + "><v:line id='l1' style='position:absolute;rotation:90' from='10pt,20pt' to='110pt,20pt'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].kind, Shape::Line);
        QVERIFY(std::abs(shapes[0].x1 - 60) < 0.01 && std::abs(shapes[0].y1 - (-30)) < 0.01);
        QVERIFY(std::abs(shapes[0].x2 - 60) < 0.01 && std::abs(shapes[0].y2 - 70) < 0.01);
        QCOMPARE(shapes[0].rotation, 0.0);
        // A half turn swaps the ends.
        shapes = readShapes("<w:pict " + ns + "><v:line id='l2' style='position:absolute;rotation:180' from='10pt,20pt' to='110pt,40pt'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(std::abs(shapes[0].x1 - 110) < 0.01 && std::abs(shapes[0].y1 - 40) < 0.01);
        QVERIFY(std::abs(shapes[0].x2 - 10) < 0.01 && std::abs(shapes[0].y2 - 20) < 0.01);
        // A mirror across the segment's middle, no turn.
        shapes = readShapes("<w:pict " + ns + "><v:line id='l3' style='position:absolute;flip:x' from='10pt,20pt' to='110pt,40pt'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(std::abs(shapes[0].x1 - 110) < 0.01 && std::abs(shapes[0].y1 - 20) < 0.01);
        QVERIFY(std::abs(shapes[0].x2 - 10) < 0.01 && std::abs(shapes[0].y2 - 40) < 0.01);
        QVERIFY(!shapes[0].flipH && !shapes[0].flipV);
        // A mirror and a quarter turn after it: mirrored ends, then turned.
        shapes = readShapes("<w:pict " + ns + "><v:line id='l4' style='position:absolute;rotation:90;flip:x' from='1472,14543' to='1787,14543'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(std::abs(shapes[0].x1 - 1629.5) < 0.01 && std::abs(shapes[0].y1 - 14700.5) < 0.01);
        QVERIFY(std::abs(shapes[0].x2 - 1629.5) < 0.01 && std::abs(shapes[0].y2 - 14385.5) < 0.01);
        // None of it for a shape that is not a line: that one keeps its turn.
        shapes = readShapes("<w:pict " + ns + "><v:rect id='r1' style='position:absolute;left:0;top:0;width:10pt;height:20pt;rotation:90'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].rotation, 90.0);
        // A line in a group: its ends carry the group's place, so the group's offset is not added to the line again beside them.
        shapes = readShapes("<w:pict " + ns + "><v:group id='g' style='position:absolute;margin-left:100pt;margin-top:50pt;width:20pt;height:20pt' coordsize='400,400' coordorigin='1000,1000'><v:line id='m' from='1000,1000' to='1400,1000'/></v:group></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].kind, Shape::Line);
        QVERIFY(std::abs(shapes[0].x1 - 100) < 0.01 && std::abs(shapes[0].y1 - 50) < 0.01);
        QVERIFY(std::abs(shapes[0].x2 - 120) < 0.01 && std::abs(shapes[0].y2 - 50) < 0.01);
        QCOMPARE(shapes[0].left, 0.0);
        QCOMPARE(shapes[0].top, 0.0);
    }

    // A picture's cut edges (VML's crop attributes in sixty-five-thousandths, DrawingML's a:srcRect in thousandths of a percent), the color it makes clear
    // (chromakey), and its border (the sides' weights are in eighths of a point).
    void readsAPicturesCutEdgesClearColorAndBorder()
    {
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
                              "xmlns:w10=\"urn:schemas-microsoft-com:office:word\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                              "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                              "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"";
        QVector<Shape> shapes = readShapes("<w:pict " + ns + "><v:shape id='s1' type='#_x0000_t75' style='width:77.25pt;height:1in'><v:imagedata r:id='rId5' croptop='16384f' cropbottom='32768f' "
                                           "cropleft='10%' cropright='0.125' chromakey='black'/><w10:bordertop type='single' width='12'/><w10:borderleft type='single' width='12'/></v:shape></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].kind, Shape::Picture);
        QVERIFY(std::abs(shapes[0].cropT - 0.25) < 1e-9);
        QVERIFY(std::abs(shapes[0].cropB - 0.5) < 1e-9);
        QVERIFY(std::abs(shapes[0].cropL - 0.1) < 1e-9);
        QVERIFY(std::abs(shapes[0].cropR - 0.125) < 1e-9);
        QCOMPARE(shapes[0].clearColor, QStringLiteral("000000"));
        QVERIFY(std::abs(shapes[0].borderWeight - 1.5) < 1e-9);
        // The other form of a picture (r:pict) is kept beside the picture's own (r:id); a picture that has only that one is that one.
        shapes = readShapes("<w:pict " + ns + "><v:shape id='sp' type='#_x0000_t75' style='width:54pt;height:36pt'><v:imagedata r:id='rId4' r:pict='rId5'/></v:shape></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].embed, QStringLiteral("rId4"));
        QCOMPARE(shapes[0].pictEmbed, QStringLiteral("rId5"));
        shapes = readShapes("<w:pict " + ns + "><v:shape id='sq' type='#_x0000_t75' style='width:54pt;height:36pt'><v:imagedata r:pict='rId6'/></v:shape></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].embed, QStringLiteral("rId6"));
        QCOMPARE(shapes[0].pictEmbed, QStringLiteral("rId6"));
        // A negative cut is room round the picture; a picture that says none is whole, with no clear color or border.
        shapes = readShapes("<w:pict " + ns + "><v:shape id='s2' type='#_x0000_t75' style='width:54pt;height:36pt'><v:imagedata r:id='rId5' croptop='-3277f' chromakey='#FFFFFF'/></v:shape></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(std::abs(shapes[0].cropT + 0.05) < 1e-4);
        QCOMPARE(shapes[0].cropL, 0.0);
        QCOMPARE(shapes[0].clearColor, QStringLiteral("FFFFFF"));
        shapes = readShapes("<w:pict " + ns + "><v:shape id='s3a' type='#_x0000_t75' style='width:54pt;height:36pt'><v:imagedata r:id='rId5' croptop='8192f' chromakey='white'/><w10:bordertop type='single' width='8'/></v:shape>"
                            "<v:shape id='s3' type='#_x0000_t75' style='width:54pt;height:36pt'><v:imagedata r:id='rId5'/></v:shape></w:pict>");
        QCOMPARE(int(shapes.size()), 2);
        QVERIFY(shapes[0].cropT > 0.1 && shapes[0].clearColor == QStringLiteral("FFFFFF") && shapes[0].borderWeight == 1.0);
        QCOMPARE(shapes[1].cropT + shapes[1].cropB + shapes[1].cropL + shapes[1].cropR, 0.0);
        QVERIFY(shapes[1].clearColor.isEmpty());
        QCOMPARE(shapes[1].borderWeight, 0.0);
        // A picture of DrawingML: the cut edges are of its blip fill.
        shapes = readShapes("<w:drawing " + ns + "><wp:inline><wp:extent cx='2540000' cy='1270000'/><wp:docPr id='1' name='p'/><a:graphic><a:graphicData><pic:pic><pic:blipFill><a:blip r:embed='rId7'/>"
                            "<a:srcRect l='10000' t='20000' r='5000' b='-2500'/><a:stretch><a:fillRect/></a:stretch></pic:blipFill></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(std::abs(shapes[0].cropL - 0.1) < 1e-9 && std::abs(shapes[0].cropT - 0.2) < 1e-9);
        QVERIFY(std::abs(shapes[0].cropR - 0.05) < 1e-9 && std::abs(shapes[0].cropB + 0.025) < 1e-9);
        // (A cut that leaves nothing of the picture is read as none: it would draw nothing, and the file's frame is the one it means.)
        shapes = readShapes("<w:pict " + ns + "><v:shape id='s4' type='#_x0000_t75' style='width:54pt;height:36pt'><v:imagedata r:id='rId5' croptop='45000f' cropbottom='45000f'/></v:shape></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].cropT + shapes[0].cropB, 0.0);
    }

    // The number of the drawing format's shape that a VML object is (from its type, or the o:spt of the shape type in the same element), text on a path
    // (a banner's lettering: the text, its font, and the shape's color and shadow), and a horizontal rule (its share of the text's width).
    void readsAShapeTypeTextOnAPathAndAHorizontalRule()
    {
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
                              "xmlns:w10=\"urn:schemas-microsoft-com:office:word\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
        QVector<Shape> shapes = readShapes("<w:pict " + ns + "><v:shape id='a' type='#_x0000_t19' style='position:absolute;margin-left:412.2pt;margin-top:-7.2pt;width:27pt;height:54pt' adj='-5898241'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].shapeType, 19);
        // The shape type of the same element says the number itself.
        shapes = readShapes("<w:pict " + ns + "><v:shapetype id='mine' coordsize='21600,21600' o:spt='85' adj='1800'/><v:shape id='b' type='#mine' style='width:25.2pt;height:54pt'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].shapeType, 85);
        shapes = readShapes("<w:pict " + ns + "><v:rect id='c' style='width:10pt;height:10pt'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].shapeType, 0);
        // Text on a path.
        shapes = readShapes("<w:pict " + ns + "><v:shape id='w' type='#_x0000_t136' style='position:absolute;margin-left:14.4pt;margin-top:0;width:121.4pt;height:32.4pt' fillcolor='black' strokeweight='1.5pt'>"
                            "<v:shadow on='t' color='#969696' opacity='52429f' offset='1pt,2pt'/><v:textpath style='font-family:&quot;Arial Black&quot;;font-style:italic;v-text-kern:t' trim='t' fitpath='t' string='LTIMP'/></v:shape></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(shapes[0].shapeType, 136);
        QCOMPARE(shapes[0].pathText, QStringLiteral("LTIMP"));
        QCOMPARE(shapes[0].pathFont, QStringLiteral("Arial Black"));
        QVERIFY(shapes[0].pathItalic && !shapes[0].pathBold);
        QCOMPARE(shapes[0].fillColor, QStringLiteral("000000"));
        QCOMPARE(shapes[0].strokeWeight, 1.5);
        QVERIFY(shapes[0].shadow);
        QCOMPARE(shapes[0].shadowColor, QStringLiteral("969696"));
        QVERIFY(std::abs(shapes[0].shadowX - 1) < 1e-9 && std::abs(shapes[0].shadowY - 2) < 1e-9);
        // A text path that is off is not one, beside one that is (so a reader that read none would fail on the first); a bold one is bold; no shadow is none.
        shapes = readShapes("<w:pict " + ns + "><v:shape id='x1' type='#_x0000_t136' style='width:100pt;height:20pt'><v:shadow on='t' offset='3pt'/><v:textpath style='font-family:Arial;font-weight:bold' string='shown'/></v:shape>"
                            "<v:shape id='x2' type='#_x0000_t136' style='width:100pt;height:20pt'><v:textpath on='f' style='font-family:Arial;font-weight:bold' string='off'/></v:shape></w:pict>");
        QCOMPARE(int(shapes.size()), 2);
        QCOMPARE(shapes[0].pathText, QStringLiteral("shown"));
        QVERIFY(shapes[0].pathBold && !shapes[0].pathItalic && shapes[0].shadow);
        QVERIFY(shapes[1].pathText.isEmpty() && !shapes[1].shadow);
        // A horizontal rule: its share of the text's width in tenths of a percent (960 is 96 percent); none given and no width is all of it; a width and no
        // share is that width.
        shapes = readShapes("<w:pict " + ns + "><v:rect id='h1' style='width:110.25pt;height:1pt;flip:y' o:hrpct='960' o:hralign='center' o:hrstd='t' o:hrnoshade='t' o:hr='t' fillcolor='black' stroked='f'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(shapes[0].rule && !shapes[0].floating);
        QCOMPARE(shapes[0].rulePercent, 96.0);
        QCOMPARE(shapes[0].hAlign, Shape::Center);
        QCOMPARE(shapes[0].height, 1.0);
        shapes = readShapes("<w:pict " + ns + "><v:rect id='h2' style='width:0;height:2.25pt' o:hralign='center' o:hrstd='t' o:hr='t' fillcolor='gray' stroked='f'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(shapes[0].rule);
        QCOMPARE(shapes[0].rulePercent, 100.0);
        QCOMPARE(shapes[0].fillColor, QStringLiteral("808080"));
        shapes = readShapes("<w:pict " + ns + "><v:rect id='h3' style='width:200pt;height:1pt' o:hr='t' o:hrpct='0'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(shapes[0].rule);
        QCOMPARE(shapes[0].rulePercent, 0.0);
        QCOMPARE(shapes[0].width, 200.0);
        // A rectangle that is not a rule, beside one that is.
        shapes = readShapes("<w:pict " + ns + "><v:rect id='h4' style='width:200pt;height:1pt'/><v:rect id='h5' style='width:200pt;height:1pt' o:hr='t'/></w:pict>");
        QCOMPARE(int(shapes.size()), 2);
        QVERIFY(!shapes[0].rule && shapes[1].rule);
    }

    // The sides a picture's border is set on (each with its weight; the others have none), the color it is drawn in ("this" is the picture's own line color), and a shape's own
    // numbers (a list in which an empty place or a word is not a number).
    void readsAPicturesBorderSidesAndColorAndAShapesNumbers()
    {
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
                              "xmlns:w10=\"urn:schemas-microsoft-com:office:word\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
        QVector<Shape> shapes = readShapes("<w:pict " + ns + "><v:shape id='b' type='#_x0000_t75' style='width:54pt;height:36pt' o:bordertopcolor='#FF0000' o:borderleftcolor='this'><v:imagedata r:id='rId4'/>"
                                           "<w10:bordertop type='single' width='12'/><w10:borderbottom type='double' width='24'/><w10:borderleft type='none' width='40'/></v:shape></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(std::abs(shapes[0].borderSides[0] - 1.5) < 1e-9);   // top
        QCOMPARE(shapes[0].borderSides[1], 0.0);                    // left: none
        QVERIFY(std::abs(shapes[0].borderSides[2] - 3.0) < 1e-9);   // bottom
        QCOMPARE(shapes[0].borderSides[3], 0.0);                    // right: not said
        QVERIFY(std::abs(shapes[0].borderWeight - 3.0) < 1e-9);
        QCOMPARE(shapes[0].borderColor, QStringLiteral("FF0000"));
        // "this" is the shape's line color; none said is black.
        shapes = readShapes("<w:pict " + ns + "><v:shape id='b' type='#_x0000_t75' style='width:54pt;height:36pt' strokecolor='#0000FF' o:bordertopcolor='this'><v:imagedata r:id='rId4'/><w10:bordertop type='single' width='8'/></v:shape></w:pict>");
        QCOMPARE(shapes[0].borderColor, QStringLiteral("0000FF"));
        shapes = readShapes("<w:pict " + ns + "><v:shape id='b' type='#_x0000_t75' style='width:54pt;height:36pt'><v:imagedata r:id='rId4'/><w10:bordertop type='single' width='8'/></v:shape></w:pict>");
        QCOMPARE(shapes[0].borderColor, QStringLiteral("000000"));
        // The numbers of a shape.
        shapes = readShapes("<w:pict " + ns + "><v:shape id='a' type='#_x0000_t19' style='width:27pt;height:54pt' adj='-5898241,,3,word,1e999'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(int(shapes[0].adjust.size()), 5);
        QCOMPARE(shapes[0].adjust[0], -5898241.0);
        QVERIFY(std::isnan(shapes[0].adjust[1]) && std::isnan(shapes[0].adjust[3]) && std::isnan(shapes[0].adjust[4]));
        QCOMPARE(shapes[0].adjust[2], 3.0);
    }

    // Numbers a file may give that are not numbers, or are beyond any page, are no cut, no border, no share past all the width, and no number of a shape's: everything read is
    // finite and in the range a page could hold.
    void readsNumbersThatAreNotNumbersOrAreTooLarge()
    {
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
                              "xmlns:w10=\"urn:schemas-microsoft-com:office:word\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                              "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                              "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"";
        auto finite = [](const Shape &s) {
            for (const double v : {s.cropL, s.cropT, s.cropR, s.cropB, s.borderWeight, s.borderSides[0], s.borderSides[1], s.borderSides[2], s.borderSides[3], s.rulePercent, s.shadowX, s.shadowY, s.width, s.height, s.left, s.top})
                if (!std::isfinite(v)) return false;
            return true;
        };
        QVector<Shape> shapes = readShapes("<w:pict " + ns + "><v:shape id='s' type='#_x0000_t75' style='width:54pt;height:36pt'><v:imagedata r:id='rId4' croptop='nan' cropbottom='1e999f' cropleft='-1e999' cropright='inf%'/>"
                                           "<w10:bordertop type='single' width='nan'/><w10:borderleft type='single' width='1e308'/><w10:borderbottom type='single' width='-5'/></v:shape></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(finite(shapes[0]));
        for (const double cut : {shapes[0].cropL, shapes[0].cropT, shapes[0].cropR, shapes[0].cropB}) QVERIFY(cut >= -1 && cut <= 1);
        QVERIFY(shapes[0].cropT == 0.0 && shapes[0].cropR == 0.0);   // (words that are not numbers are no cut)
        QVERIFY(shapes[0].cropL + shapes[0].cropR < 0.98 && shapes[0].cropT + shapes[0].cropB < 0.98);
        QVERIFY(shapes[0].borderSides[0] == 0.0 && shapes[0].borderSides[1] == 100.0 && shapes[0].borderSides[2] == 0.0);   // (nan: none; too much: 100 points; below none: none)
        shapes = readShapes("<w:pict " + ns + "><v:rect id='r' style='width:1e999pt;height:nan' o:hr='t' o:hrpct='1e999' o:hralign='center' fillcolor='black'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(finite(shapes[0]));
        QVERIFY(shapes[0].rulePercent >= 0 && shapes[0].rulePercent <= 100);
        shapes = readShapes("<w:pict " + ns + "><v:rect id='r' style='width:50pt;height:1pt' o:hr='t' o:hrpct='nan'/></w:pict>");
        QVERIFY(finite(shapes[0]) && shapes[0].rulePercent == 0.0);
        shapes = readShapes("<w:drawing " + ns + "><wp:inline><wp:extent cx='2540000' cy='1270000'/><wp:docPr id='1' name='p'/><a:graphic><a:graphicData><pic:pic><pic:blipFill><a:blip r:embed='rId7'/>"
                            "<a:srcRect l='nan' t='1e999' r='-99999999' b='abc'/></pic:blipFill></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(finite(shapes[0]));
        QVERIFY(shapes[0].cropL + shapes[0].cropR < 0.98 && shapes[0].cropT + shapes[0].cropB < 0.98);
        shapes = readShapes("<w:pict " + ns + "><v:shape id='a' type='#_x0000_t19' style='width:27pt;height:54pt' adj='1e999,-1e999,nan'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        for (const double v : shapes[0].adjust) QVERIFY(std::isnan(v) || std::abs(v) <= 1e9);
    }

    // The side the text goes on beside a wrap (DrawingML's wrapText, VML's side), and the room the wrap itself says (its own distances come before the object's).
    void readsWhichSideTheTextGoesOnAndTheRoomRound()
    {
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
                              "xmlns:w10=\"urn:schemas-microsoft-com:office:word\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                              "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                              "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"";
        auto drawing = [&](const QByteArray &wrap, const QByteArray &anchorAttributes = "distT='127000' distB='127000' distL='127000' distR='127000'") {
            const QByteArray xml = "<w:drawing " + ns + "><wp:anchor " + anchorAttributes + "><wp:positionH relativeFrom='column'><wp:posOffset>0</wp:posOffset></wp:positionH><wp:positionV relativeFrom='paragraph'><wp:posOffset>0</wp:posOffset></wp:positionV>"
                                   "<wp:extent cx='2540000' cy='1270000'/>" + wrap + "<wp:docPr id='1' name='p'/><a:graphic><a:graphicData><pic:pic><pic:blipFill><a:blip r:embed='rId7'/></pic:blipFill></pic:pic></a:graphicData></a:graphic></wp:anchor></w:drawing>";
            return readShapes(xml);
        };
        // Both sides is the default; left, right, and largest are read.
        QCOMPARE(drawing("<wp:wrapSquare wrapText='bothSides'/>")[0].side, Shape::SideBoth);
        QCOMPARE(drawing("<wp:wrapSquare/>")[0].side, Shape::SideBoth);
        QCOMPARE(drawing("<wp:wrapSquare wrapText='left'/>")[0].side, Shape::SideLeft);
        QCOMPARE(drawing("<wp:wrapSquare wrapText='right'/>")[0].side, Shape::SideRight);
        QCOMPARE(drawing("<wp:wrapTight wrapText='largest'><wp:wrapPolygon edited='0'><wp:start x='0' y='0'/></wp:wrapPolygon></wp:wrapTight>")[0].side, Shape::SideLargest);
        QCOMPARE(drawing("<wp:wrapThrough wrapText='right'><wp:wrapPolygon edited='0'><wp:start x='0' y='0'/></wp:wrapPolygon></wp:wrapThrough>")[0].side, Shape::SideRight);
        // The room: the object's distances are 10 points each; the wrap's own, where it has them, come first (3, 1, 4, 2 points: left, top, right, bottom).
        QVector<Shape> shapes = drawing("<wp:wrapSquare wrapText='bothSides' distT='12700' distB='25400' distL='38100' distR='50800'/>");
        QCOMPARE(shapes[0].gapLeft, 3.0);
        QCOMPARE(shapes[0].gapTop, 1.0);
        QCOMPARE(shapes[0].gapRight, 4.0);
        QCOMPARE(shapes[0].gapBottom, 2.0);
        // Tight has only the sides of its own; top and bottom stay the object's.
        shapes = drawing("<wp:wrapTight wrapText='bothSides' distL='38100' distR='50800'><wp:wrapPolygon edited='0'><wp:start x='0' y='0'/></wp:wrapPolygon></wp:wrapTight>");
        QCOMPARE(shapes[0].wrap, Shape::Tight);
        QCOMPARE(shapes[0].gapLeft, 3.0);
        QCOMPARE(shapes[0].gapRight, 4.0);
        QCOMPARE(shapes[0].gapTop, 10.0);
        QCOMPARE(shapes[0].gapBottom, 10.0);
        // Top and bottom: its own top and bottom.
        shapes = drawing("<wp:wrapTopAndBottom distT='12700' distB='25400'/>");
        QCOMPARE(shapes[0].wrap, Shape::TopBottom);
        QCOMPARE(shapes[0].gapTop, 1.0);
        QCOMPARE(shapes[0].gapBottom, 2.0);
        QCOMPARE(shapes[0].gapLeft, 10.0);
        // VML: the side of w10:wrap.
        auto vml = [&](const QByteArray &wrap) {
            return readShapes("<w:pict " + ns + "><v:shape id='s' style='position:absolute;margin-left:0;margin-top:0;width:54pt;height:36pt'><v:imagedata r:id='rId4'/>" + wrap + "</v:shape></w:pict>");
        };
        QCOMPARE(vml("<w10:wrap type='square'/>")[0].side, Shape::SideBoth);
        QCOMPARE(vml("<w10:wrap type='square' side='left'/>")[0].side, Shape::SideLeft);
        QCOMPARE(vml("<w10:wrap type='square' side='right'/>")[0].side, Shape::SideRight);
        QCOMPARE(vml("<w10:wrap type='tight' side='largest'/>")[0].side, Shape::SideLargest);
        QCOMPARE(vml("<w10:wrap type='tight' side='largest'/>")[0].wrap, Shape::Tight);
    }

    // What an object's position is measured from, including the strips of the page beside the margins.
    void readsTheStripsOfThePageThatPositionsAreMeasuredFrom()
    {
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
                              "xmlns:w10=\"urn:schemas-microsoft-com:office:word\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                              "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                              "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"";
        auto drawing = [&](const char *h, const char *v) {
            const QByteArray xml = "<w:drawing " + ns + "><wp:anchor distT='0' distB='0' distL='0' distR='0'><wp:positionH relativeFrom='" + QByteArray(h) + "'><wp:posOffset>12700</wp:posOffset></wp:positionH>"
                                   "<wp:positionV relativeFrom='" + QByteArray(v) + "'><wp:posOffset>25400</wp:posOffset></wp:positionV><wp:extent cx='254000' cy='127000'/><wp:wrapNone/>"
                                   "<wp:docPr id='1' name='p'/><a:graphic><a:graphicData><pic:pic><pic:blipFill><a:blip r:embed='rId7'/></pic:blipFill></pic:pic></a:graphicData></a:graphic></wp:anchor></w:drawing>";
            return readShapes(xml);
        };
        using R = Shape::Rel;
        struct Case { const char *h, *v; R hRel, vRel; };
        for (const Case &c : {Case{"margin", "margin", R::Margin, R::Margin}, Case{"page", "page", R::Page, R::Page}, Case{"column", "paragraph", R::Column, R::Paragraph},
                              Case{"character", "line", R::Character, R::LineOf}, Case{"leftMargin", "topMargin", R::LeftMargin, R::TopMargin},
                              Case{"rightMargin", "bottomMargin", R::RightMargin, R::BottomMargin}, Case{"insideMargin", "insideMargin", R::InsideMargin, R::InsideMargin},
                              Case{"outsideMargin", "outsideMargin", R::OutsideMargin, R::OutsideMargin}}) {
            const QVector<Shape> shapes = drawing(c.h, c.v);
            QCOMPARE(int(shapes.size()), 1);
            QVERIFY2(shapes[0].hRel == c.hRel, c.h);
            QVERIFY2(shapes[0].vRel == c.vRel, c.v);
        }
        // VML names them as it does.
        auto vml = [&](const char *h, const char *v) {
            return readShapes("<w:pict " + ns + "><v:rect id='s' style='position:absolute;margin-left:0;margin-top:0;width:54pt;height:36pt;mso-position-horizontal-relative:" + QByteArray(h) + ";mso-position-vertical-relative:" + QByteArray(v) + "'/></w:pict>");
        };
        struct VCase { const char *h, *v; R hRel, vRel; };
        for (const VCase &c : {VCase{"margin", "margin", R::Margin, R::Margin}, VCase{"page", "page", R::Page, R::Page}, VCase{"text", "text", R::Column, R::Column},
                               VCase{"char", "line", R::Character, R::LineOf}, VCase{"left-margin-area", "top-margin-area", R::LeftMargin, R::TopMargin},
                               VCase{"right-margin-area", "bottom-margin-area", R::RightMargin, R::BottomMargin}, VCase{"inner-margin-area", "inner-margin-area", R::InsideMargin, R::InsideMargin},
                               VCase{"outer-margin-area", "outer-margin-area", R::OutsideMargin, R::OutsideMargin}}) {
            const QVector<Shape> shapes = vml(c.h, c.v);
            QCOMPARE(int(shapes.size()), 1);
            QVERIFY2(shapes[0].hRel == c.hRel, c.h);
            QVERIFY2(shapes[0].vRel == c.vRel, c.v);
        }
    }

    // The numbers of a shape that are past what a page holds (or are not numbers) are brought into a range a page could be in.
    void shapeCoordinatesStayInRange()
    {
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
                              "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\"";
        auto sane = [](const Shape &s) {
            for (double v : {s.left, s.top, s.width, s.height, s.x1, s.y1, s.x2, s.y2, s.rotation})
                if (!std::isfinite(v) || std::abs(v) > 1e6) return false;
            return true;
        };
        QVector<Shape> shapes = readShapes("<w:pict " + ns + "><v:rect id='a' style='position:absolute;margin-left:1e999pt;margin-top:-1e999pt;width:1e999pt;height:1e300pt;rotation:1e999' fillcolor='red'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY2(sane(shapes[0]), qPrintable(QStringLiteral("left %1 top %2 width %3 height %4 rotation %5").arg(shapes[0].left).arg(shapes[0].top).arg(shapes[0].width).arg(shapes[0].height).arg(shapes[0].rotation)));
        shapes = readShapes("<w:pict " + ns + "><v:line id='b' style='position:absolute;left:0' from='1e999pt,-1e999pt' to='1e999pt,1e300pt'/></w:pict>");
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(sane(shapes[0]));
        shapes = readShapes("<w:pict " + ns + "><v:group id='g' style='position:absolute;margin-left:10pt;margin-top:20pt;width:100pt;height:50pt' coordsize='1e999,0' coordorigin='1e999,1e999'><v:rect id='m' style='position:absolute;left:100;top:50;width:200;height:100' fillcolor='red'/></v:group></w:pict>");
        for (const Shape &s : shapes) QVERIFY(sane(s));
    }

    // The notes of a footnotes part come with their Ids, types, and paragraphs.
    void readsTheNotesOfAPart()
    {
        const QByteArray part = xmlDoc(QByteArray("<w:footnotes ") + kW + R"(><w:footnote w:type="separator" w:id="-1"><w:p><w:r><w:separator/></w:r></w:p></w:footnote><w:footnote w:id="2"><w:p><w:pPr><w:pStyle w:val="FootnoteText"/></w:pPr><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space="preserve"> note two</w:t></w:r></w:p><w:p><w:r><w:t>second paragraph</w:t></w:r></w:p></w:footnote></w:footnotes>)");
        QVector<NotePart> notes;
        QVERIFY(readNotes(part, &notes));
        QCOMPARE(int(notes.size()), 2);
        QCOMPARE(notes[0].id, -1);
        QCOMPARE(notes[0].type, QStringLiteral("separator"));
        QCOMPARE(notes[1].id, 2);
        QVERIFY(notes[1].type.isEmpty());
        QCOMPARE(int(notes[1].blocks.size()), 2);
        QCOMPARE(notes[1].blocks[0].para.content.first().tag, QStringLiteral("footnoteRef"));
        QCOMPARE(notes[1].blocks[1].para.content.first().text, QStringLiteral("second paragraph"));
        QVERIFY(!readNotes("<w:footnotes", &notes));
        // A part with more notes than the limit has the rest left out.
        QByteArray many = QByteArray("<w:footnotes ") + kW + ">";
        for (int i = 0; i < kMaxNotes + 50; ++i) many += "<w:footnote w:id=\"" + QByteArray::number(i + 1) + "\"><w:p/></w:footnote>";
        many += "</w:footnotes>";
        QVector<NotePart> capped;
        QVERIFY(readNotes(many, &capped));
        QCOMPARE(int(capped.size()), kMaxNotes);
    }

    // The fonts of a font table come with their alternates and kinds; a part of thousands of fonts is read in step with its size, and one that is
    // not XML (unclosed elements) is refused.
    void readsTheFontsOfAFontTable()
    {
        const QByteArray part = xmlDoc(QByteArray("<w:fonts ") + kW + R"(><w:font w:name="Palatino"><w:altName w:val="Book Antiqua"/><w:panose1 w:val="02040502050505030304"/><w:charset w:val="00"/><w:family w:val="roman"/><w:pitch w:val="variable"/></w:font><w:font w:name="Optima"><w:family w:val="auto"/></w:font><w:font w:name="Plain"/><w:font w:name="Dingbats"><w:charset w:val="02"/><w:family w:val="decorative"/><w:notTrueType/><w:pitch w:val="fixed"/></w:font></w:fonts>)");
        QVector<FontEntry> fonts;
        QVERIFY(readFonts(part, &fonts));
        QCOMPARE(int(fonts.size()), 4);
        QCOMPARE(fonts[0].name, QStringLiteral("Palatino"));
        QCOMPARE(fonts[0].alternate, QStringLiteral("Book Antiqua"));
        QCOMPARE(fonts[0].family, QStringLiteral("roman"));
        // (Its pitch, its PANOSE digits, and its character set, as written.)
        QCOMPARE(fonts[0].pitch, QStringLiteral("variable"));
        QCOMPARE(fonts[0].panose, QStringLiteral("02040502050505030304"));
        QCOMPARE(fonts[0].charset, QStringLiteral("00"));
        QCOMPARE(fonts[1].family, QStringLiteral("auto"));
        QVERIFY(fonts[1].alternate.isEmpty());
        QVERIFY(fonts[1].pitch.isEmpty() && fonts[1].panose.isEmpty() && fonts[1].charset.isEmpty());
        QVERIFY(fonts[2].family.isEmpty());
        QCOMPARE(fonts[3].charset, QStringLiteral("02"));
        QCOMPARE(fonts[3].pitch, QStringLiteral("fixed"));
        auto seconds = [&](int count) {
            QByteArray many = QByteArray("<w:fonts ") + kW + ">";
            for (int i = 0; i < count; ++i) many += "<w:font ";
            bool accepted = false;
            const double best = bestNanoseconds([&] {
                QVector<FontEntry> none;
                if (readFonts(many, &none)) accepted = true;
            }) / 1e9;
            return accepted ? -1.0 : best;
        };
        const double small = seconds(5000), large = seconds(20000);
        QVERIFY(small >= 0 && large >= 0);
        // (Four times the openers: four times the time, or sixteen when it is the square; and never more than a moment.)
        QVERIFY2(large < 0.1 + small * 8, qPrintable(QStringLiteral("5000 openers %1 s, 20000 openers %2 s").arg(small).arg(large)));
        // More fonts than the limit have the rest left out.
        QByteArray huge = QByteArray("<w:fonts ") + kW + ">";
        for (int i = 0; i < kMaxFonts + 50; ++i) huge += "<w:font w:name=\"F" + QByteArray::number(i) + "\"/>";
        huge += "</w:fonts>";
        QVector<FontEntry> capped;
        QVERIFY(readFonts(huge, &capped));
        QCOMPARE(int(capped.size()), kMaxFonts);
    }

    // What the settings say of the numbering of the footnotes is read from the footnotes' own element: an endnote setting of the same name is not it.
    void readsHowFootnoteNumbersRestart()
    {
        auto settings = [](const QByteArray &inner) { return xmlDoc(QByteArray("<w:settings ") + kW + ">" + inner + "</w:settings>"); };
        QCOMPARE(noteNumberRestart(settings("<w:footnotePr><w:numRestart w:val=\"eachSect\"/></w:footnotePr>"), "footnotePr"), QStringLiteral("eachSect"));
        QCOMPARE(noteNumberRestart(settings("<w:footnotePr><w:numFmt w:val=\"decimal\"/></w:footnotePr><w:endnotePr><w:numRestart w:val=\"eachSect\"/></w:endnotePr>"), "footnotePr"), QString());
        QCOMPARE(noteNumberRestart(settings("<w:footnotePr><w:numFmt w:val=\"decimal\"/></w:footnotePr><w:endnotePr><w:numRestart w:val=\"eachSect\"/></w:endnotePr>"), "endnotePr"), QStringLiteral("eachSect"));
        QCOMPARE(noteNumberRestart(settings("<w:footnotePr><w:numRestart w:val=\"continuous\"/></w:footnotePr>"), "footnotePr"), QStringLiteral("continuous"));
        QCOMPARE(noteNumberRestart(settings(""), "footnotePr"), QString());
        QCOMPARE(noteNumberRestart("not xml", "footnotePr"), QString());
    }

    // How the endnotes are numbered is read from the endnotes' own element of the settings (the format and the first number), or of a section's properties as it keeps them.
    void readsHowNotesAreNumbered()
    {
        auto settings = [](const QByteArray &inner) { return xmlDoc(QByteArray("<w:settings ") + kW + ">" + inner + "</w:settings>"); };
        const NoteNumbering both = readNoteNumbering(settings("<w:footnotePr><w:numFmt w:val=\"chicago\"/></w:footnotePr><w:endnotePr><w:numFmt w:val=\"upperLetter\"/><w:numStart w:val=\"4\"/></w:endnotePr>"), "endnotePr");
        QCOMPARE(both.format, QStringLiteral("upperLetter"));
        QCOMPARE(both.start, 4);
        QVERIFY(readNoteNumbering(settings(""), "endnotePr").format.isEmpty());
        QCOMPARE(readNoteNumbering(settings("<w:endnotePr><w:numStart w:val=\"-3\"/></w:endnotePr>"), "endnotePr").start, 0);
        QCOMPARE(readNoteNumbering("not xml", "endnotePr").start, 0);
        SectionProps section;
        section.extra.push_back({1, "<w:footnotePr><w:numFmt w:val=\"decimal\"/></w:footnotePr>"});
        QVERIFY(noteNumberingOf(section, "endnotePr").format.isEmpty());   // (the footnotes' is not the endnotes')
        section.extra.push_back({2, "<w:endnotePr><w:numFmt w:val=\"lowerLetter\"/><w:numStart w:val=\"9\"/></w:endnotePr>"});
        const NoteNumbering own = noteNumberingOf(section, "endnotePr");
        QCOMPARE(own.format, QStringLiteral("lowerLetter"));
        QCOMPARE(own.start, 9);
    }

    // The compatibility options that the layout reads come from one reading of the settings part (a part may be very large, and was read once for each option): each is
    // on when it is stated in w:compat and not turned off, an option outside w:compat is none, and a part that is not XML has none.
    void readsTheCompatibilityOptionsInOneReading()
    {
        auto settings = [](const QByteArray &inner) { return xmlDoc(QByteArray("<w:settings ") + kW + "><w:compat>" + inner + "</w:compat></w:settings>"); };
        const CompatOptions all = readCompatOptions(settings("<w:useFELayout/><w:doNotWrapTextWithPunct w:val=\"1\"/><w:doNotUseHTMLParagraphAutoSpacing/><w:noTabHangInd/>"
                                                             "<w:doNotUseIndentAsNumberingTabStop/><w:splitPgBreakAndParaMark w:val=\"true\"/><w:suppressTopSpacingWP/><w:noLeading/>"));
        QVERIFY(all.useFELayout && all.doNotWrapTextWithPunct && all.doNotUseHTMLParagraphAutoSpacing && all.noTabHangInd);
        QVERIFY(all.doNotUseIndentAsNumberingTabStop && all.splitPgBreakAndParaMark && all.suppressTopSpacingWP && all.noLeading);
        const CompatOptions off = readCompatOptions(settings("<w:suppressTopSpacingWP w:val=\"0\"/><w:noLeading w:val=\"false\"/><w:noTabHangInd w:val=\"off\"/><w:useFELayout/>"));
        QVERIFY(!off.suppressTopSpacingWP && !off.noLeading && !off.noTabHangInd && off.useFELayout);
        const CompatOptions outside = readCompatOptions(xmlDoc(QByteArray("<w:settings ") + kW + "><w:noLeading/><w:suppressTopSpacingWP/></w:settings>"));
        QVERIFY(!outside.noLeading && !outside.suppressTopSpacingWP);
        const CompatOptions bad = readCompatOptions("not xml");
        QVERIFY(!bad.noLeading && !bad.useFELayout && !bad.splitPgBreakAndParaMark);
    }

    // A compatibility option of the settings counts when it is stated and not turned off.
    void readsACompatibilityOptionByItsValue()
    {
        auto settings = [](const QByteArray &inner) { return xmlDoc(QByteArray("<w:settings ") + kW + "><w:compat>" + inner + "</w:compat></w:settings>"); };
        QVERIFY(compatOption(settings("<w:doNotWrapTextWithPunct/>"), "doNotWrapTextWithPunct"));
        QVERIFY(compatOption(settings("<w:doNotWrapTextWithPunct w:val=\"1\"/>"), "doNotWrapTextWithPunct"));
        QVERIFY(compatOption(settings("<w:doNotWrapTextWithPunct w:val=\"true\"/>"), "doNotWrapTextWithPunct"));
        QVERIFY(!compatOption(settings("<w:doNotWrapTextWithPunct w:val=\"0\"/>"), "doNotWrapTextWithPunct"));
        QVERIFY(!compatOption(settings("<w:doNotWrapTextWithPunct w:val=\"false\"/>"), "doNotWrapTextWithPunct"));
        QVERIFY(!compatOption(settings("<w:doNotWrapTextWithPunct w:val=\"off\"/>"), "doNotWrapTextWithPunct"));
        QVERIFY(!compatOption(settings("<w:somethingElse/>"), "doNotWrapTextWithPunct"));
        QVERIFY(!compatOption("not xml", "doNotWrapTextWithPunct"));
        // (Only in w:compat: an element of that name elsewhere, or in a comment, is not the option.)
        QVERIFY(!compatOption(xmlDoc(QByteArray("<w:settings ") + kW + "><w:doNotWrapTextWithPunct/></w:settings>"), "doNotWrapTextWithPunct"));
    }

    // A paragraph's borders and shading are read into their sides and colors, and a side a style states that the paragraph doesn't is the style's;
    // the elements are kept as written beside that, and written back.
    void paragraphBordersAndShadingAreReadAndInherited()
    {
        Made m;
        m.styles = R"(<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>)"
                   R"(<w:style w:type="paragraph" w:styleId="Rule"><w:name w:val="Rule"/><w:basedOn w:val="Normal"/><w:pPr><w:pBdr><w:bottom w:val="single" w:sz="12" w:space="2" w:color="FF0000"/><w:between w:val="dotted" w:sz="4" w:space="1"/></w:pBdr><w:shd w:val="clear" w:color="auto" w:fill="DDDDDD"/></w:pPr></w:style>)";
        m.body = R"(<w:p><w:pPr><w:pStyle w:val="Rule"/><w:pBdr><w:top w:val="double" w:sz="6" w:space="3" w:color="auto"/><w:bottom w:val="nil"/></w:pBdr></w:pPr><w:r><w:t>one</w:t></w:r></w:p><w:p><w:pPr><w:pStyle w:val="Rule"/></w:pPr><w:r><w:t>two</w:t></w:r></w:p>)";
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        const ParaProps &own = d.body[0].para.props;
        QVERIFY(own.borders && own.borders->top && own.borders->bottom);
        QCOMPARE(own.borders->top->val, QStringLiteral("double"));
        QCOMPARE(own.borders->top->size, 6);
        QCOMPARE(own.borders->top->space, 3);
        QVERIFY(own.borders->top->visible());
        QVERIFY(!own.borders->bottom->visible());   // (nil says there is none)
        QVERIFY(!own.shading);
        // What the paragraph has in all: its own top, its own "no bottom" over the style's, the style's between and shading.
        ParaProps all = d.styles.paragraph(QStringLiteral("Rule"));
        QVERIFY(all.borders && all.borders->bottom && all.borders->bottom->visible() && all.shading);
        QCOMPARE(all.borders->bottom->color, QStringLiteral("FF0000"));
        all.overlay(own);
        QVERIFY(all.borders->top->visible() && !all.borders->bottom->visible());
        QVERIFY(all.borders->between && all.borders->between->visible());
        QCOMPARE(all.shading->fill, QStringLiteral("DDDDDD"));
        // Written back as they were.
        const QByteArray out = documentXmlOf(writeDocx(d));
        QCOMPARE(count(out, "<w:pBdr>"), 1);
        QVERIFY(out.contains("w:val=\"double\" w:sz=\"6\" w:space=\"3\""));
    }

    // Automatic space before and after (a paragraph from a web page) is read apart from the points beside it, and a paragraph that gives its own space
    // overrides its style's automatic one; written back as it was.
    void automaticSpacingIsReadAndOverridden()
    {
        Made m;
        m.styles = R"(<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>)"
                   R"(<w:style w:type="paragraph" w:styleId="Web"><w:name w:val="Web"/><w:basedOn w:val="Normal"/><w:pPr><w:spacing w:before="100" w:beforeAutospacing="1" w:after="100" w:afterAutospacing="1"/></w:pPr></w:style>)";
        m.body = R"(<w:p><w:pPr><w:pStyle w:val="Web"/><w:spacing w:before="0"/></w:pPr><w:r><w:t>own</w:t></w:r></w:p><w:p><w:pPr><w:pStyle w:val="Web"/></w:pPr><w:r><w:t>style's</w:t></w:r></w:p>)";
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        const ParaProps web = d.styles.paragraph(QStringLiteral("Web"));
        QVERIFY(web.beforeAuto.value_or(false) && web.afterAuto.value_or(false));
        QCOMPARE(web.before.value_or(-1), 100);
        ParaProps own = d.styles.paragraph(QStringLiteral("Web"));
        own.overlay(d.body[0].para.props);
        QVERIFY(!own.beforeAuto.value_or(true));   // (its own before, with no automatic, ends the style's)
        QVERIFY(own.afterAuto.value_or(false));
        QVERIFY(!d.body[1].para.props.beforeAuto);
        const QByteArray out = documentXmlOf(writeDocx(d));
        QVERIFY(out.contains("w:before=\"0\""));
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

    // The toggles (bold, italic, capitals, small capitals, strike-through, hidden) of a character style flip those of the paragraph style: Strong over a heading that is bold
    // is not bold, and Emphasis over text that is not italic is. The styles a style is based on do not flip: each says over the one under it. A character style that says
    // "off" flips nothing, and a double strike and "no proofing" are not toggles.
    void aCharacterStyleFlipsTheTogglesOfTheParagraphStyle()
    {
        Made m;
        m.body = R"(<w:p/>)";
        m.styles = R"(<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>)"
                   R"(<w:style w:type="paragraph" w:styleId="Head"><w:basedOn w:val="Normal"/><w:rPr><w:b/><w:smallCaps/><w:dstrike/></w:rPr></w:style>)"
                   R"(<w:style w:type="paragraph" w:styleId="SubHead"><w:basedOn w:val="Head"/><w:rPr><w:b/><w:i/></w:rPr></w:style>)"
                   R"(<w:style w:type="character" w:styleId="Strong"><w:rPr><w:b/><w:dstrike/></w:rPr></w:style>)"
                   R"(<w:style w:type="character" w:styleId="StrongToo"><w:basedOn w:val="Strong"/><w:rPr><w:b/><w:smallCaps/><w:i/></w:rPr></w:style>)"
                   R"(<w:style w:type="character" w:styleId="Plain"><w:rPr><w:b w:val="0"/><w:smallCaps w:val="false"/></w:rPr></w:style>)";
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        const StyleSheet &s = d.styles;
        auto on = [&](const char *para, const char *chr, std::optional<bool> RunProps::*f) { return (s.run(QString::fromLatin1(para), QString::fromLatin1(chr)).*f).value_or(false); };
        QVERIFY(on("Head", "", &RunProps::bold));
        QVERIFY(!on("Head", "Strong", &RunProps::bold));        // (flipped)
        QVERIFY(on("Normal", "Strong", &RunProps::bold));       // (flipped from not bold)
        QVERIFY(on("SubHead", "", &RunProps::bold));            // (a style based on a bold one that says bold again is bold)
        QVERIFY(!on("SubHead", "Strong", &RunProps::bold));
        QVERIFY(!on("Head", "StrongToo", &RunProps::bold));     // (its own and Strong's are one value, flipping once)
        QVERIFY(!on("Head", "StrongToo", &RunProps::smallCaps));
        QVERIFY(on("Head", "StrongToo", &RunProps::italic));
        QVERIFY(!on("SubHead", "StrongToo", &RunProps::italic));
        QVERIFY(on("Head", "Plain", &RunProps::bold));          // (an "off" flips nothing)
        QVERIFY(on("Head", "Plain", &RunProps::smallCaps));
        QVERIFY(on("Head", "Strong", &RunProps::dstrike));      // (not a toggle: the character style says it over the paragraph's)
        QVERIFY(on("Normal", "Strong", &RunProps::dstrike));
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

    // A table is the blocks of its parts: an Open block of the table with its properties and grid as written, one for each row and each cell with the cell's paragraphs
    // after it, and a Close block of each; what the table says is read into the Open blocks' marks, and what is written is the bytes.
    void aTableIsReadAsItsPartsAndWrittenBackAsItWas()
    {
        const QByteArray nested = R"(<w:tbl><w:tblPr><w:tblW w:w="1000" w:type="dxa"/></w:tblPr><w:tblGrid><w:gridCol w:w="1000"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>deep</w:t></w:r></w:p></w:tc></w:tr></w:tbl>)";
        const QByteArray table =
            R"(<w:tbl><w:tblPr><w:tblStyle w:val="Grid"/><w:tblW w:w="5000" w:type="pct"/><w:jc w:val="center"/><w:tblInd w:w="115" w:type="dxa"/>)"
            R"(<w:tblBorders><w:top w:val="single" w:sz="8" w:space="0" w:color="FF0000"/><w:left w:val="double" w:sz="4" w:space="0" w:color="auto"/><w:insideH w:val="dotted" w:sz="2" w:space="0" w:color="0000FF"/></w:tblBorders>)"
            R"(<w:tblLayout w:type="fixed"/><w:tblCellMar><w:top w:w="10" w:type="dxa"/><w:left w:w="100" w:type="dxa"/></w:tblCellMar><w:tblLook w:val="04A0" w:firstRow="1" w:lastRow="0" w:firstColumn="1" w:lastColumn="0" w:noHBand="0" w:noVBand="1"/></w:tblPr>)"
            R"(<w:tblGrid><w:gridCol w:w="2000"/><w:gridCol w:w="3000"/><w:gridCol w:w="4000"/></w:tblGrid>)"
            R"(<w:tr w:rsidR="00AA"><w:trPr><w:cantSplit/><w:trHeight w:val="500" w:hRule="exact"/><w:tblHeader/></w:trPr>)"
            R"(<w:tc><w:tcPr><w:tcW w:w="5000" w:type="dxa"/><w:gridSpan w:val="2"/><w:vMerge w:val="restart"/><w:tcBorders><w:bottom w:val="single" w:sz="12" w:space="0" w:color="00FF00"/></w:tcBorders><w:shd w:val="clear" w:color="auto" w:fill="FFFF00"/><w:vAlign w:val="center"/></w:tcPr><w:p><w:r><w:t>A</w:t></w:r></w:p>)" + nested + R"(<w:p/></w:tc>)"
            R"(<w:tc><w:tcPr><w:tcW w:w="4000" w:type="dxa"/><w:tcMar><w:left w:w="50" w:type="dxa"/></w:tcMar><w:noWrap/></w:tcPr><w:p><w:r><w:t>B</w:t></w:r></w:p><w:p><w:r><w:t>B2</w:t></w:r></w:p></w:tc></w:tr>)"
            R"(<w:bookmarkStart w:id="3" w:name="between"/><w:bookmarkEnd w:id="3"/>)"
            R"(<w:tr><w:trPr><w:gridBefore w:val="1"/><w:wBefore w:w="2000" w:type="dxa"/><w:hidden/></w:trPr><w:tc><w:tcPr><w:tcW w:w="3000" w:type="dxa"/><w:vMerge/></w:tcPr><w:p/></w:tc><w:tc><w:p><w:r><w:t>C</w:t></w:r></w:p></w:tc></w:tr></w:tbl>)";
        Made m;
        m.body = "<w:p><w:r><w:t>before</w:t></w:r></w:p>" + table + R"(<w:p><w:r><w:t>after</w:t></w:r></w:p>)";
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        // before, the table, after.
        QCOMPARE(d.body.size(), 31);   // before; the table's start; row 1 (start, cell A: start, a paragraph, the nested table's 7, a paragraph, end; cell B: start, two paragraphs, end; end);
                                       // the two kept bookmark pieces; row 2 (start, two cells of three, end); the table's end; after
        int tables = 0, rows = 0, cells = 0, raws = 0;
        const TableMark *first = nullptr, *firstRow = nullptr, *firstCell = nullptr, *secondCell = nullptr, *vContinue = nullptr, *secondRow = nullptr;
        int rowNumber = 0, cellNumber = 0;
        for (const Block &b : d.body) {
            if (!b.isRaw) continue;
            if (b.role == Inline::Open && b.table) {
                tables += b.tag == QLatin1String("tbl");
                rows += b.tag == QLatin1String("tr");
                cells += b.tag == QLatin1String("tc");
                if (b.tag == QLatin1String("tbl") && !first) first = b.table.get();
                if (b.tag == QLatin1String("tr")) {
                    ++rowNumber;   // (the file's second row comes after the nested table's)
                    if (rowNumber == 1) firstRow = b.table.get();
                    else if (rowNumber == 3) secondRow = b.table.get();
                }
                if (b.tag == QLatin1String("tc")) {
                    ++cellNumber;   // (A, the nested table's, B, then the second row's two)
                    if (cellNumber == 1) firstCell = b.table.get();
                    else if (cellNumber == 3) secondCell = b.table.get();
                    else if (cellNumber == 4) vContinue = b.table.get();
                }
            } else if (b.role == Inline::Single) {
                ++raws;
            }
        }
        QCOMPARE(tables, 2);   // (one of them in a cell)
        QCOMPARE(rows, 3);
        QCOMPARE(cells, 5);
        QCOMPARE(raws, 2);     // (the bookmark between the rows, and its end)
        QVERIFY(first && firstRow && firstCell && secondCell && vContinue && secondRow);
        QCOMPARE(vContinue->vMerge, QStringLiteral("continue"));
        QCOMPARE(first->grid, (QVector<int>{2000, 3000, 4000}));
        QCOMPARE(first->style, QStringLiteral("Grid"));
        QCOMPARE(first->width.type, QStringLiteral("pct"));
        QCOMPARE(first->width.w, 5000);
        QCOMPARE(first->jc, QStringLiteral("center"));
        QCOMPARE(first->indent.w, 115);
        QCOMPARE(first->layout, QStringLiteral("fixed"));
        QCOMPARE(first->top.val, QStringLiteral("single"));
        QCOMPARE(first->top.size, 8);
        QCOMPARE(first->top.color, QStringLiteral("FF0000"));
        QCOMPARE(first->left.val, QStringLiteral("double"));
        QCOMPARE(first->insideH.val, QStringLiteral("dotted"));
        QVERIFY(!first->bottom.stated());
        QVERIFY(first->marTop && first->marTop->w == 10 && first->marLeft && first->marLeft->w == 100 && !first->marRight);
        QCOMPARE(first->look, 0x04A0);   // (the first row and column, no column bands)
        QVERIFY(firstRow->cantSplit && firstRow->header);
        QCOMPARE(firstRow->height, 500);
        QCOMPARE(firstRow->heightRule, QStringLiteral("exact"));
        QCOMPARE(secondRow->gridBefore, 1);
        QVERIFY(secondRow->hidden && !firstRow->hidden);   // (a row that says it is hidden is)
        QCOMPARE(secondRow->widthBefore.w, 2000);
        QCOMPARE(firstCell->span, 2);
        QCOMPARE(firstCell->vMerge, QStringLiteral("restart"));
        QCOMPARE(firstCell->cellWidth.w, 5000);
        QCOMPARE(firstCell->bottom.color, QStringLiteral("00FF00"));
        QCOMPARE(firstCell->shdFill, QStringLiteral("FFFF00"));
        QCOMPARE(firstCell->vAlign, QStringLiteral("center"));
        QVERIFY(secondCell->noWrap && secondCell->cMarLeft && secondCell->cMarLeft->w == 50);
        // Written, the table is what it was.
        QVERIFY(documentXmlOf(writeDocx(d)).contains(table));
        Document again;
        QVERIFY(readDocx(writeDocx(d), &again));
        QCOMPARE(writeDocx(again), writeDocx(d));
    }

    // A table is written as it was read while its blocks are as they were; one whose text, formatting, or structure changed is written from its blocks (with the
    // nested table in it, which did not change, still as it was).
    void aTableThatChangedIsWrittenFromItsBlocksAndOneThatDidNotAsItWas()
    {
        const QByteArray inner = R"(<w:tbl><w:tblGrid><w:gridCol w:w="1000"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>inner</w:t></w:r></w:p></w:tc></w:tr></w:tbl>)";
        // (Spaces and new lines between the elements, runs that a writer would join, and a text without xml:space: all kept while nothing changes.)
        const QByteArray table = "<w:tbl>\n  <w:tblGrid><w:gridCol w:w=\"3000\"/></w:tblGrid>\n  <w:tr>\n    <w:tc><w:p><w:r><w:t>one</w:t></w:r><w:r><w:t>two</w:t></w:r></w:p>" + inner +
                                 "<w:p/></w:tc>\n  </w:tr>\n</w:tbl>";
        Made m;
        m.body = "<w:p><w:r><w:t>x</w:t></w:r></w:p>" + table;
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        QVERIFY(documentXmlOf(writeDocx(d)).contains(table));
        // The paragraph in the outer cell is changed: that table is written from its blocks, and the inner one as it was.
        int changed = -1;
        for (int i = 0; i < d.body.size(); ++i)
            if (!d.body[i].isRaw && !d.body[i].para.content.isEmpty() && d.body[i].para.content[0].text == QLatin1String("one")) changed = i;
        QVERIFY(changed > 0);
        d.body[changed].para.content[0].text = QStringLiteral("uno");
        const QByteArray out = documentXmlOf(writeDocx(d));
        QVERIFY(!out.contains(table));
        QVERIFY(out.contains("uno"));
        QVERIFY(out.contains(inner));
        QVERIFY(out.contains("<w:tc>"));
        // And a damaged block structure (a row that is not closed) leaves nothing half written: the pieces without partners are not written.
        Document d2;
        QVERIFY(readDocx(m.build(), &d2));
        for (int i = d2.body.size() - 1; i >= 0; --i)
            if (d2.body[i].isRaw && d2.body[i].role == Inline::Close && d2.body[i].tag == QLatin1String("tr")) {
                d2.body.removeAt(i);
                break;
            }
        XmlTree t;
        QVERIFY2(t.parse(documentXmlOf(writeDocx(d2))), "the document is well formed");
    }

    // Rows that a content control holds (nested controls too), and a cell that one holds, are the table's rows and cells: read as such, with the controls kept open and closed
    // between them; and written as they were while nothing changes, and as nested as they were when something does.
    void rowsAndCellsInContentControlsAreTheTablesOwn()
    {
        const QByteArray rowControls = R"(<w:sdt><w:sdtPr><w:id w:val="1"/></w:sdtPr><w:sdtContent><w:sdt><w:sdtPr><w:id w:val="2"/></w:sdtPr><w:sdtContent>)"
                                       R"(<w:tr><w:sdt><w:sdtPr><w:id w:val="3"/></w:sdtPr><w:sdtContent><w:tc><w:p><w:r><w:t>Choose an item.</w:t></w:r></w:p></w:tc></w:sdtContent></w:sdt>)"
                                       R"(<w:tc><w:p><w:r><w:t>Here is a sample</w:t></w:r></w:p></w:tc></w:tr></w:sdtContent></w:sdt></w:sdtContent></w:sdt>)";
        const QByteArray table = R"(<w:tbl><w:tblGrid><w:gridCol w:w="3000"/><w:gridCol w:w="3000"/></w:tblGrid>)"
                                 R"(<w:tr><w:tc><w:p><w:r><w:t>Note</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>Details</w:t></w:r></w:p></w:tc></w:tr>)" + rowControls + "</w:tbl>";
        Made m;
        m.body = "<w:p><w:r><w:t>before</w:t></w:r></w:p>" + table;
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        QStringList texts, tags;
        for (const Block &b : d.body) {
            if (!b.isRaw) {
                QString text;
                for (const Inline &in : b.para.content) text += in.text;
                texts << text;
            } else {
                tags << QString(b.role == Inline::Open ? "+" : b.role == Inline::Close ? "-" : "=") + b.tag;
            }
        }
        QCOMPARE(texts, (QStringList{QStringLiteral("before"), QStringLiteral("Note"), QStringLiteral("Details"), QStringLiteral("Choose an item."), QStringLiteral("Here is a sample")}));
        QCOMPARE(tags, (QStringList{"+tbl", "+tr", "+tc", "-tc", "+tc", "-tc", "-tr", "+sdt", "+sdt", "+tr", "+sdt", "+tc", "-tc", "-sdt", "+tc", "-tc", "-tr", "-sdt", "-sdt", "-tbl"}));
        QVERIFY(documentXmlOf(writeDocx(d)).contains(table));
        // One of the cells' text is changed: the table is written from its blocks, with the controls where they were.
        for (Block &b : d.body)
            if (!b.isRaw && !b.para.content.isEmpty() && b.para.content[0].text == QLatin1String("Here is a sample")) b.para.content[0].text = QStringLiteral("Changed");
        const QByteArray out = documentXmlOf(writeDocx(d));
        QVERIFY(!out.contains(table));
        QVERIFY(out.contains(">Changed<"));
        XmlTree t;
        QVERIFY2(t.parse(out), "the document is well formed");
        QVERIFY(out.contains("<w:sdtContent><w:sdt><w:sdtPr><w:id w:val=\"2\"/></w:sdtPr><w:sdtContent><w:tr>"));
        QVERIFY(out.contains("<w:tr><w:sdt><w:sdtPr><w:id w:val=\"3\"/></w:sdtPr><w:sdtContent><w:tc>"));
        QVERIFY(out.contains("</w:tc></w:sdtContent></w:sdt><w:tc>"));
    }

    // Content controls nested without end do not make a table that is read as one (a limit stands in for the table, which is kept whole).
    void contentControlsRoundRowsAreBounded()
    {
        QByteArray open, close;
        for (int i = 0; i < 60; ++i) {   // (more than the controls are followed to, fewer than the XML is allowed to be nested)
            open += R"(<w:sdt><w:sdtPr><w:id w:val="9"/></w:sdtPr><w:sdtContent>)";
            close += "</w:sdtContent></w:sdt>";
        }
        Made m;
        m.body = R"(<w:tbl><w:tblGrid><w:gridCol w:w="3000"/></w:tblGrid>)" + open + R"(<w:tr><w:tc><w:p><w:r><w:t>deep</w:t></w:r></w:p></w:tc></w:tr>)" + close + "</w:tbl>";
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        QVERIFY(documentXmlOf(writeDocx(d)).contains(m.body));
        XmlTree t;
        QVERIFY2(t.parse(documentXmlOf(writeDocx(d))), "the document is well formed");
    }

    // A table style is what it says over the styles it is based on: borders and margins side by side, paragraph and run properties over one another, and the parts
    // of it for the first row and the like each with its own.
    void aTableStyleIsReadWithTheStylesItIsBasedOn()
    {
        Made m;
        m.body = "<w:p/>";
        m.styles =
            R"(<w:style w:type="table" w:default="1" w:styleId="TableNormal"><w:name w:val="Normal Table"/><w:tblPr><w:tblInd w:w="0" w:type="dxa"/><w:tblCellMar><w:top w:w="0" w:type="dxa"/><w:left w:w="108" w:type="dxa"/><w:bottom w:w="0" w:type="dxa"/><w:right w:w="108" w:type="dxa"/></w:tblCellMar></w:tblPr></w:style>)"
            R"(<w:style w:type="table" w:styleId="Grid"><w:name w:val="Grid"/><w:basedOn w:val="TableNormal"/><w:pPr><w:spacing w:after="0" w:line="240" w:lineRule="auto"/></w:pPr>)"
            R"(<w:tblPr><w:tblBorders><w:top w:val="single" w:sz="4" w:space="0" w:color="auto"/><w:left w:val="single" w:sz="4" w:space="0" w:color="auto"/><w:insideH w:val="single" w:sz="4" w:space="0" w:color="auto"/></w:tblBorders></w:tblPr></w:style>)"
            R"(<w:style w:type="table" w:styleId="Fancy"><w:name w:val="Fancy"/><w:basedOn w:val="Grid"/><w:rPr><w:b/></w:rPr><w:pPr><w:jc w:val="center"/></w:pPr>)"
            R"(<w:tblPr><w:tblStyleRowBandSize w:val="2"/><w:tblBorders><w:top w:val="double" w:sz="8" w:space="0" w:color="FF0000"/></w:tblBorders></w:tblPr><w:tcPr><w:vAlign w:val="center"/></w:tcPr>)"
            R"(<w:tblStylePr w:type="firstRow"><w:rPr><w:color w:val="FF0000"/></w:rPr><w:tcPr><w:tcBorders><w:bottom w:val="double" w:sz="4" w:space="0" w:color="auto"/></w:tcBorders><w:shd w:val="clear" w:color="auto" w:fill="000000"/></w:tcPr></w:tblStylePr>)"
            R"(<w:tblStylePr w:type="band1Horz"><w:tcPr><w:shd w:val="clear" w:color="auto" w:fill="EEEEEE"/></w:tcPr></w:tblStylePr></w:style>)";
        Document d;
        QVERIFY(readDocx(m.build(), &d));
        const TableStyleDef &fancy = d.styles.tableStyle(QStringLiteral("Fancy"));
        QCOMPARE(fancy.base.tbl.top.val, QStringLiteral("double"));       // (its own over Grid's)
        QCOMPARE(fancy.base.tbl.top.color, QStringLiteral("FF0000"));
        QCOMPARE(fancy.base.tbl.left.val, QStringLiteral("single"));      // (Grid's)
        QCOMPARE(fancy.base.tbl.insideH.val, QStringLiteral("single"));
        QVERIFY(fancy.base.tbl.marLeft && fancy.base.tbl.marLeft->w == 108 && fancy.base.tbl.marRight && fancy.base.tbl.marTop && fancy.base.tbl.marTop->w == 0);   // (TableNormal's)
        QVERIFY(fancy.base.pPr && fancy.base.pPr->after && *fancy.base.pPr->after == 0 && fancy.base.pPr->jc == QLatin1String("center"));
        QVERIFY(fancy.base.rPr && fancy.base.rPr->bold && *fancy.base.rPr->bold);
        QCOMPARE(fancy.base.cell.vAlign, QStringLiteral("center"));
        QVERIFY(fancy.rowBand && *fancy.rowBand == 2 && !fancy.colBand);
        QVERIFY(fancy.parts.contains(QStringLiteral("firstRow")) && fancy.parts.contains(QStringLiteral("band1Horz")));
        QCOMPARE(fancy.parts[QStringLiteral("firstRow")].cell.shdFill, QStringLiteral("000000"));
        QCOMPARE(fancy.parts[QStringLiteral("firstRow")].cell.bottom.val, QStringLiteral("double"));
        QCOMPARE(fancy.parts[QStringLiteral("firstRow")].rPr->color->val, QStringLiteral("FF0000"));
        QCOMPARE(fancy.parts[QStringLiteral("band1Horz")].cell.shdFill, QStringLiteral("EEEEEE"));
        // The table style the file calls the default, and one it has none of.
        QCOMPARE(d.styles.defaultId(QStringLiteral("table")), QStringLiteral("TableNormal"));
        QVERIFY(d.styles.tableStyle(QStringLiteral("None")).parts.isEmpty() && !d.styles.tableStyle(QStringLiteral("None")).base.tbl.top.stated());
        // A paragraph in a cell has the style's properties under its own style's.
        TableLayer layer;
        layer.p = *fancy.base.pPr;
        layer.r = *fancy.base.rPr;
        layer.key = QStringLiteral("fancy");
        QVERIFY(!d.styles.paragraph(QStringLiteral("Normal")).after);
        const ParaProps in = d.styles.paragraph(QStringLiteral("Normal"), &layer);
        QVERIFY(in.after && *in.after == 0 && in.jc == QLatin1String("center"));
        QVERIFY(d.styles.run(QStringLiteral("Normal"), QString(), &layer).bold.value_or(false));
        QVERIFY(!d.styles.run(QStringLiteral("Normal"), QString()).bold.value_or(false));
    }

    // A cell with no paragraph in a damaged file is given one, and a table past a limit (columns, depth) is kept whole as XML and shown as a stand-in, as before.
    void tablesPastTheLimitsAreKeptWholeAndCellsWithNothingGetAParagraph()
    {
        Document d;
        QVERIFY(readDocx(Made{R"(<w:tbl><w:tblGrid><w:gridCol w:w="2000"/></w:tblGrid><w:tr><w:tc/></w:tr></w:tbl>)", {}, {}, {}}.build(), &d));
        int paragraphs = 0;
        for (const Block &b : d.body) paragraphs += !b.isRaw;
        QCOMPARE(paragraphs, 1);
        // Too many columns in a row: the table is one stand-in block, written as it was.
        QByteArray wide = "<w:tbl><w:tr>";
        for (int i = 0; i < kMaxTableColumns + 1; ++i) wide += "<w:tc><w:p/></w:tc>";
        wide += "</w:tr></w:tbl>";
        QVERIFY(readDocx(Made{wide, {}, {}, {}}.build(), &d));
        QCOMPARE(d.body.size(), 1);
        QVERIFY(d.body[0].isRaw && d.body[0].role == Inline::Single && !d.body[0].display.isEmpty());
        QCOMPARE(d.body[0].raw, wide);
        // A table with no row is kept whole too.
        QVERIFY(readDocx(Made{"<w:tbl><w:tblPr/></w:tbl>", {}, {}, {}}.build(), &d));
        QCOMPARE(d.body.size(), 1);
        QVERIFY(d.body[0].isRaw && !d.body[0].display.isEmpty());
        // Nested more deeply than the limit: the tables down to it are read, the one below is a stand-in in a cell.
        QByteArray deep;
        for (int i = 0; i < kMaxTableDepth + 3; ++i) deep += "<w:tbl><w:tr><w:tc><w:p><w:r><w:t>level " + QByteArray::number(i) + "</w:t></w:r></w:p>";
        for (int i = 0; i < kMaxTableDepth + 3; ++i) deep += "</w:tc></w:tr></w:tbl><w:p/>";
        QVERIFY(readDocx(Made{deep, {}, {}, {}}.build(), &d));
        int open = 0, standIns = 0;
        for (const Block &b : d.body) {
            open += b.isRaw && b.role == Inline::Open && b.tag == QLatin1String("tbl");
            standIns += b.isRaw && b.role == Inline::Single && !b.display.isEmpty();
        }
        QCOMPARE(open, kMaxTableDepth);
        QCOMPARE(standIns, 1);
        QVERIFY(documentXmlOf(writeDocx(d)).contains("level " + QByteArray::number(kMaxTableDepth + 2) + "<"));
    }

    // A document lays out at most kMaxTableCells cells in its tables: the table that would be past that is a stand-in, kept as it was, and the file's reader says that
    // a table was kept so (with how many), and says nothing when all are shown as tables.
    void tablesPastTheDocumentsCellsAreKeptWholeAndSaidSo()
    {
        auto grid = [](int rows, int cols) {
            QByteArray t = "<w:tbl><w:tblGrid>";
            for (int c = 0; c < cols; ++c) t += "<w:gridCol w:w=\"1000\"/>";
            t += "</w:tblGrid>";
            for (int r = 0; r < rows; ++r) {
                t += "<w:tr>";
                for (int c = 0; c < cols; ++c) t += "<w:tc><w:p/></w:tc>";
                t += "</w:tr>";
            }
            return t + "</w:tbl><w:p/>";
        };
        const int rows = kMaxTableCells / 10 * 6 / 10;   // (each of two tables holds three fifths of the cells the document may: the second is past them)
        const QByteArray first = grid(rows, 10), second = grid(rows, 10);
        Document d;
        ReadNotes notes;
        QVERIFY(readDocx(Made{first + second, {}, {}, {}}.build(), &d, nullptr, &notes));
        int open = 0, standIns = 0;
        for (const Block &b : d.body) {
            open += b.isRaw && b.role == Inline::Open && b.tag == QLatin1String("tbl");
            standIns += b.isRaw && b.role == Inline::Single && !b.display.isEmpty();
        }
        QCOMPARE(open, 1);
        QCOMPARE(standIns, 1);
        QCOMPARE(notes.notes.size(), 1);
        QVERIFY2(notes.notes[0].contains(QLatin1String("table")), qPrintable(notes.notes.join(QLatin1Char('|'))));
        QVERIFY(documentXmlOf(writeDocx(d)).contains(second.left(second.size() - 6)));   // (kept as it was: the table, not the paragraph after it)
        ReadNotes none;
        QVERIFY(readDocx(Made{first, {}, {}, {}}.build(), &d, nullptr, &none));
        QVERIFY2(none.notes.isEmpty(), qPrintable(none.notes.join(QLatin1Char('|'))));
    }

    void theCompatibilityModeIsReadFromTheSettings()
    {
        PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"), xmlDoc(QByteArray("<w:document ") + kW + "><w:body><w:p/></w:body></w:document>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addPart(QStringLiteral("/word/settings.xml"), xmlDoc(QByteArray("<w:settings ") + kW + "><w:compat><w:compatSetting w:name=\"compatibilityMode\" w:uri=\"http://schemas.microsoft.com/office/word\" w:val=\"15\"/></w:compat></w:settings>"),
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings"), QStringLiteral("/word/settings.xml"));
        Document d;
        QVERIFY(readDocx(w.finish(), &d));
        QCOMPARE(d.compatMode, 15);
        QVERIFY(readDocx(Made{"<w:p/>", {}, {}, {}}.build(), &d));
        QCOMPARE(d.compatMode, 12);
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
        // The table is the blocks of its parts; the paragraph holds the atoms.
        QCOMPARE(d.body.size(), 14);   // paragraph, table (start, row start, two cells of start, paragraph, end, then row end and table end), sdt start, paragraph, sdt end
        QVERIFY(d.body[1].isRaw && d.body[1].tag == QLatin1String("tbl") && d.body[1].role == Inline::Open);
        QCOMPARE(d.body[11].role, Inline::Open);
        QCOMPARE(d.body[13].role, Inline::Close);
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
        QVERIFY(again.body[1].isRaw && again.body[1].tag == QLatin1String("tbl"));
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
        // A document nested past the depth limit is not refused: what is below the limit is made plain paragraphs, and a note says so (the limit
        // itself stays: the XML reader never goes that deep). One with a DOCTYPE and a body that is not XML are refused with a message.
        QByteArray deep;
        for (int i = 0; i < kMaxXmlDepth + 10; ++i) deep += "<w:p>";
        for (int i = 0; i < kMaxXmlDepth + 10; ++i) deep += "</w:p>";
        ReadNotes deepNotes;
        QVERIFY(readDocx(Made{deep, {}, {}, {}}.build(), &d, &err, &deepNotes));
        QVERIFY(deepNotes.leftOut && deepNotes.notes.size() == 1);
        QByteArray tables;
        for (int i = 0; i < 400; ++i) tables += "<w:tbl><w:tr><w:tc><w:p><w:r><w:t>level " + QByteArray::number(i) + "</w:t></w:r></w:p>";
        for (int i = 0; i < 400; ++i) tables += "</w:tc></w:tr></w:tbl><w:p/>";
        ReadNotes tableNotes;
        QVERIFY(readDocx(Made{tables, {}, {}, {}}.build(), &d, &err, &tableNotes));
        QVERIFY(tableNotes.leftOut);
        QVERIFY(documentXmlOf(writeDocx(d)).contains("level 399"));
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
        // Two hundred thousand paragraphs, each with runs, read in time in proportion to their number (ten times the paragraphs of twenty
        // thousand, about ten times as long: compared with each other, not with a clock), and written again.
        auto readTime = [&](int count, Document *into) {
            QByteArray many;
            many.reserve(count * 80);
            for (int i = 0; i < count; ++i) many += "<w:p><w:r><w:rPr><w:b/></w:rPr><w:t>word</w:t></w:r></w:p>";
            const QByteArray zip = Made{many, {}, {}, {}}.build();
            bool ok = false;
            const qint64 ns = bestNanoseconds([&] { ok = readDocx(zip, into, &err); });
            return ok ? ns / 1e6 : 1e9;
        };
        Document small;
        readTime(20000, &small);   // (the first of anything is slow)
        const double few = std::max(readTime(20000, &small), 10.0), lots = readTime(200000, &d);
        QCOMPARE(d.body.size(), 200000);
        QVERIFY2(lots < 25 * few, qPrintable(QStringLiteral("20,000 paragraphs %1 ms, 200,000 paragraphs %2 ms").arg(few).arg(lots)));
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
        QVERIFY(notes.leftOut);   // (a document with its end left out is not to be written over the file it came from)
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
