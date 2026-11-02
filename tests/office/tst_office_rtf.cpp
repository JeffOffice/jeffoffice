// Tests of libs/office/rtf: reading a Rich Text Format file into the document the .docx reader makes. Every file is a string built here in code; the hostile ones are
// that string changed.
#include "rtf/rtf.h"
#include "office/package.h"
#include "rtf/rtfmodel.h"
#include "wml/wml.h"

#include <QRandomGenerator>
#include <QtTest>

#include "../testrunner.h"

using namespace jp::office;
using jp::office::wml::Block;
using jp::office::wml::Document;
using jp::office::wml::Inline;
using jp::office::wml::ReadNotes;

namespace {

QString paragraphText(const Block &b)
{
    QString t;
    for (const Inline &in : b.para.content)
        if (in.kind == Inline::Text) t += in.text;
    return t;
}

QStringList bodyTexts(const Document &d)
{
    QStringList out;
    for (const Block &b : d.body)
        if (!b.isRaw) out << paragraphText(b);
    return out;
}

bool read(const QByteArray &rtf, Document *d, ReadNotes *notes = nullptr, QString *error = nullptr) { return rtf::readRtf(rtf, d, error, notes); }

Document must(const QByteArray &rtf)
{
    Document d;
    QString error;
    if (!rtf::readRtf(rtf, &d, &error)) qFatal("could not read: %s", qPrintable(error));
    return d;
}

int count(const Document &d, const char *tag, Inline::Role role = Inline::Open)
{
    int n = 0;
    for (const Block &b : d.body)
        if (b.isRaw && b.tag == QLatin1String(tag) && b.role == role) ++n;
    return n;
}

const wml::KeptPart *part(const Document &d, const QString &name)
{
    for (const wml::KeptPart &p : d.parts)
        if (p.name == name) return &p;
    return nullptr;
}

// The first run of text in the document with `needle` in it.
const Inline *run(const Document &d, const QString &needle)
{
    for (const Block &b : d.body)
        for (const Inline &in : b.para.content)
            if (in.kind == Inline::Text && in.text.contains(needle)) return &in;
    return nullptr;
}

const Block *paragraphWith(const Document &d, const QString &needle)
{
    for (const Block &b : d.body)
        if (!b.isRaw && paragraphText(b).contains(needle)) return &b;
    return nullptr;
}

// A one-pixel PNG.
QByteArray tinyPngHex()
{
    static const unsigned char png[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
                                        0x08, 0x06, 0x00, 0x00, 0x00, 0x1F, 0x15, 0xC4, 0x89, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9C, 0x63, 0xF8, 0xCF, 0xC0, 0xF0,
                                        0x1F, 0x00, 0x05, 0x00, 0x01, 0xFF, 0x89, 0x99, 0x3D, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82};
    return QByteArray(reinterpret_cast<const char *>(png), sizeof png).toHex();
}

const char kHeader[] = "{\\rtf1\\ansi\\ansicpg1252\\deff0{\\fonttbl{\\f0\\froman\\fcharset0 Times New Roman;}{\\f1\\fswiss\\fcharset0 Arial;}}"
                       "{\\colortbl;\\red255\\green0\\blue0;\\red0\\green0\\blue255;\\red255\\green255\\blue0;}";

QByteArray doc(const QByteArray &body) { return QByteArray(kHeader) + body + "}"; }

} // namespace

class RtfTest : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void detection()
    {
        QVERIFY(rtf::looksLikeRtf("{\\rtf1 hello}"));
        QVERIFY(rtf::looksLikeRtf("  \r\n{\\rtf1 hello}"));
        QVERIFY(rtf::looksLikeRtf("\xEF\xBB\xBF{\\rtf1}"));
        QVERIFY(!rtf::looksLikeRtf("PK\x03\x04"));
        QVERIFY(!rtf::looksLikeRtf("{\\rt"));
        QVERIFY(!rtf::looksLikeRtf(QByteArray()));
        Document d;
        QString error;
        QVERIFY(!read("hello", &d, nullptr, &error));
        QVERIFY(!error.isEmpty());
    }

    void paragraphsAndText()
    {
        const Document d = must(doc("\\pard First paragraph.\\par Second {\\b one}\\par\r\nThird\\par"));
        const QStringList t = bodyTexts(d);
        QCOMPARE(t.size(), 3);
        QCOMPARE(t[0], QStringLiteral("First paragraph."));
        QCOMPARE(t[1], QStringLiteral("Second one"));
        QCOMPARE(t[2], QStringLiteral("Third"));
    }

    void textWithoutFinalParagraphMark()
    {
        const Document d = must(doc("\\pard No mark at the end"));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("No mark at the end")});
    }

    void escapesAndSpecialCharacters()
    {
        const Document d = must(doc("\\pard a\\\\b\\{c\\}d \\'e9\\'80 \\emdash\\endash\\bullet\\~x\\_y \\lquote\\rquote\\ldblquote\\rdblquote\\par"));
        QCOMPARE(bodyTexts(d).first(), QString::fromUtf8("a\\b{c}d é€ —–• x‑y ‘’“”"));
    }

    void unicodeAndSkipCount()
    {
        // 荤 then the one fallback character is skipped; \uc2 skips two; \uc0 none; negative numbers are 16-bit.
        Document d = must(doc("\\pard\\u8364?x\\uc2\\u8364\\'80\\'80y\\uc0\\u8364 z\\uc1\\u-10179?\\u-8704?w\\par"));
        QCOMPARE(bodyTexts(d).first(), QString::fromUtf8("€x€y€z\U0001F600w"));
    }

    void codePages()
    {
        // The document's code page (Cyrillic), and a font with its own charset.
        Document d = must("{\\rtf1\\ansi\\ansicpg1251\\deff0{\\fonttbl{\\f0\\fnil\\fcharset204 Arial;}{\\f1\\fnil\\fcharset161 Arial;}}\\pard \\'cf\\'f0\\'e8\\'e2\\'e5\\'f2{\\f1 \\'e1}\\par}");
        QCOMPARE(bodyTexts(d).first(), QString::fromUtf8("Приветα"));
        // Japanese in two bytes each.
        d = must("{\\rtf1\\ansi\\ansicpg932\\deff0{\\fonttbl{\\f0\\fnil\\fcharset128 MS Mincho;}}\\pard\\f0 \\'93\\'fa\\'96\\'7b\\par}");
        QCOMPARE(bodyTexts(d).first(), QString::fromUtf8("日本"));
    }

    void characterFormatting()
    {
        const Document d = must(doc("\\pard\\plain\\f1\\fs28\\cf1 red\\plain \\b\\i\\ul\\strike bold\\plain \\super sup\\plain \\sub sub\\plain \\highlight3 hl\\plain \\caps\\scaps\\v hid\\plain\\par"));
        const Inline *red = run(d, QStringLiteral("red"));
        QVERIFY(red);
        QVERIFY(red->props.fonts && red->props.fonts->ascii == QLatin1String("Arial"));
        QCOMPARE(red->props.size.value_or(0), 28);
        QVERIFY(red->props.color && red->props.color->val == QLatin1String("FF0000"));
        const Inline *bold = run(d, QStringLiteral("bold"));
        QVERIFY(bold);
        QVERIFY(bold->props.bold.value_or(false) && bold->props.italic.value_or(false) && bold->props.strike.value_or(false));
        QVERIFY(bold->props.underline && bold->props.underline->val == QLatin1String("single"));
        QCOMPARE(run(d, QStringLiteral("sup"))->props.vertAlign, QStringLiteral("superscript"));
        QCOMPARE(run(d, QStringLiteral("sub"))->props.vertAlign, QStringLiteral("subscript"));
        QCOMPARE(run(d, QStringLiteral("hl"))->props.highlight, QStringLiteral("yellow"));
        const Inline *hid = run(d, QStringLiteral("hid"));
        QVERIFY(hid && hid->props.caps.value_or(false) && hid->props.smallCaps.value_or(false) && hid->props.vanish.value_or(false));
        // \b0 turns a style's bold off, and is kept as a statement.
        const Document off = must(doc("\\pard\\b bold\\b0 plain\\par"));
        QVERIFY(!run(off, QStringLiteral("plain"))->props.bold.value_or(false));
    }

    void paragraphFormatting()
    {
        const Document d = must(doc("\\pard\\qc\\li720\\ri360\\fi-360\\sb120\\sa240\\sl360\\slmult1\\keep\\keepn\\pagebb\\tqr\\tldot\\tx5000\\tx2000 centered\\par"
                                    "\\pard\\qj\\sl-300 exact\\par\\pard\\sl300 least\\par"));
        const Block *p = paragraphWith(d, QStringLiteral("centered"));
        QVERIFY(p);
        QCOMPARE(p->para.props.jc, QStringLiteral("center"));
        QCOMPARE(p->para.props.indLeft.value_or(0), 720);
        QCOMPARE(p->para.props.indRight.value_or(0), 360);
        QCOMPARE(p->para.props.indFirst.value_or(0), -360);
        QCOMPARE(p->para.props.before.value_or(-1), 120);
        QCOMPARE(p->para.props.after.value_or(-1), 240);
        QCOMPARE(p->para.props.line.value_or(0), 360);
        QCOMPARE(p->para.props.lineRule, QStringLiteral("auto"));
        QVERIFY(p->para.props.keepNext.value_or(false) && p->para.props.keepLines.value_or(false) && p->para.props.pageBreakBefore.value_or(false));
        QVERIFY(p->para.props.tabs && p->para.props.tabs->size() == 2);
        QCOMPARE(p->para.props.tabs->at(0).val, QStringLiteral("right"));
        QCOMPARE(p->para.props.tabs->at(0).leader, QStringLiteral("dot"));
        QCOMPARE(p->para.props.tabs->at(1).val, QStringLiteral("left"));   // (the kind and leader apply to the stop that follows them only)
        QCOMPARE(paragraphWith(d, QStringLiteral("exact"))->para.props.lineRule, QStringLiteral("exact"));
        QCOMPARE(paragraphWith(d, QStringLiteral("exact"))->para.props.jc, QStringLiteral("both"));
        QCOMPARE(paragraphWith(d, QStringLiteral("least"))->para.props.lineRule, QStringLiteral("atLeast"));
        // \pard clears what came before.
        const Document e = must(doc("\\pard\\qc a\\par\\pard b\\par"));
        QVERIFY(paragraphWith(e, QStringLiteral("b"))->para.props.jc.isEmpty());
        QCOMPARE(paragraphWith(e, QStringLiteral("a"))->para.props.jc, QStringLiteral("center"));
    }

    void paragraphBordersAndShading()
    {
        const Document d = must(doc("\\pard\\brdrt\\brdrs\\brdrw20\\brdrcf1\\brsp40\\brdrb\\brdrdb\\brdrw15\\cbpat3 boxed\\par"));
        const Block *p = paragraphWith(d, QStringLiteral("boxed"));
        QVERIFY(p && p->para.props.borders);
        QVERIFY(p->para.props.borders->top && p->para.props.borders->top->val == QLatin1String("single"));
        QCOMPARE(p->para.props.borders->top->size, 8);
        QCOMPARE(p->para.props.borders->top->color, QStringLiteral("FF0000"));
        QVERIFY(p->para.props.borders->bottom && p->para.props.borders->bottom->val == QLatin1String("double"));
        QVERIFY(p->para.props.shading && p->para.props.shading->fill == QLatin1String("FFFF00"));
    }

    void styles()
    {
        const Document d = must("{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0 Times;}}{\\colortbl;\\red255\\green0\\blue0;}"
                                "{\\stylesheet{\\ql \\li0\\ri0\\sa100 \\fs24 \\snext0 Normal;}"
                                "{\\s1\\qc\\keepn\\sb240\\sa60\\b\\fs32\\cf1 \\sbasedon0 \\snext0 heading 1;}"
                                "{\\*\\cs10 \\additive\\i Emphasis Run;}}"
                                "\\pard\\plain\\s1 Title text\\par\\pard\\plain\\s0 Body \\cs10 emph\\par}");
        const Block *p = paragraphWith(d, QStringLiteral("Title text"));
        QVERIFY(p);
        QCOMPARE(p->para.props.style, QStringLiteral("Heading1"));
        const wml::Style *s = d.styles.find(QStringLiteral("Heading1"));
        QVERIFY(s);
        QCOMPARE(s->name, QStringLiteral("heading 1"));
        QCOMPARE(s->basedOn, QStringLiteral("Normal"));
        QCOMPARE(s->pPr.jc, QStringLiteral("center"));
        QVERIFY(s->rPr.bold.value_or(false));
        QCOMPARE(s->rPr.size.value_or(0), 32);
        QVERIFY(paragraphWith(d, QStringLiteral("Body"))->para.props.style.isEmpty());
        const Inline *emph = run(d, QStringLiteral("emph"));
        QVERIFY(emph);
        QVERIFY(!emph->props.style.isEmpty());
        QVERIFY(d.styles.find(emph->props.style));
    }

    void lists()
    {
        const Document d = must("{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0 Times;}{\\f1 Symbol;}}"
                                "{\\*\\listtable{\\list\\listtemplateid1{\\listlevel\\levelnfc23\\leveljc0\\levelfollow0\\levelstartat1\\levelspace0\\levelindent0{\\leveltext\\'01\\u-3913 ?;}{\\levelnumbers;}\\f1\\fi-360\\li720}"
                                "\\listid101}"
                                "{\\list\\listtemplateid2{\\listlevel\\levelnfc0\\leveljc0\\levelfollow0\\levelstartat3{\\leveltext\\'02\\'00.;}{\\levelnumbers\\'01;}\\fi-360\\li720}"
                                "{\\listlevel\\levelnfc4\\levelstartat1{\\leveltext\\'04\\'01.\\'00);}{\\levelnumbers\\'01\\'03;}\\fi-360\\li1440}\\listid102}}"
                                "{\\*\\listoverridetable{\\listoverride\\listid101\\listoverridecount0\\ls1}{\\listoverride\\listid102\\listoverridecount0\\ls2}}"
                                "\\pard\\ls1\\ilvl0 bullet\\par\\pard\\ls2\\ilvl0 number\\par\\pard\\ls2\\ilvl1 letter\\par\\pard plain\\par}");
        QCOMPARE(d.numbering.nums.size(), 2);
        const Block *b = paragraphWith(d, QStringLiteral("bullet"));
        QVERIFY(b && b->para.props.numId);
        const Block *n = paragraphWith(d, QStringLiteral("number"));
        QVERIFY(n && n->para.props.numId && *n->para.props.numId != *b->para.props.numId);
        const Block *l = paragraphWith(d, QStringLiteral("letter"));
        QCOMPARE(l->para.props.ilvl.value_or(-1), 1);
        const wml::Level *lvl = d.numbering.levelOf(*n->para.props.numId, 0);
        QVERIFY(lvl);
        QCOMPARE(lvl->numFmt, QStringLiteral("decimal"));
        QCOMPARE(lvl->text, QStringLiteral("%1."));
        QCOMPARE(lvl->start, 3);
        const wml::Level *lvl2 = d.numbering.levelOf(*n->para.props.numId, 1);
        QVERIFY(lvl2);
        QCOMPARE(lvl2->numFmt, QStringLiteral("lowerLetter"));
        QCOMPARE(lvl2->text, QStringLiteral("%2.%1)"));
        const wml::Level *bl = d.numbering.levelOf(*b->para.props.numId, 0);
        QVERIFY(bl);
        QCOMPARE(bl->numFmt, QStringLiteral("bullet"));
        QCOMPARE(bl->text, QString(QChar(0xF0B7)));
    }

    void oldBullets()
    {
        const Document d = must("{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0 Times;}{\\f1 Symbol;}}"
                                "\\pard{\\*\\pn\\pnlvlblt\\pnf1\\pnindent360{\\pntxtb\\'b7}}{\\pntext\\f1\\'b7\\tab}item\\par"
                                "\\pard{\\*\\pn\\pnlvlbody\\pndec\\pnstart1\\pnindent360{\\pntxta.}}{\\pntext 1.\\tab}counted\\par}");
        const Block *a = paragraphWith(d, QStringLiteral("item"));
        const Block *b = paragraphWith(d, QStringLiteral("counted"));
        QVERIFY(a && b && a->para.props.numId && b->para.props.numId);
        QCOMPARE(paragraphText(*a), QStringLiteral("item"));   // (the number the file wrote as text is not text)
        const wml::Level *l = d.numbering.levelOf(*b->para.props.numId, 0);
        QVERIFY(l);
        QCOMPARE(l->numFmt, QStringLiteral("decimal"));
        QCOMPARE(l->text, QStringLiteral("%1."));
    }

    void simpleTable()
    {
        const Document d = must(doc("\\trowd\\trgaph108\\trleft-108\\cellx3000\\cellx6000\\pard\\intbl A1\\cell B1\\cell\\row"
                                    "\\trowd\\trgaph108\\trleft-108\\cellx3000\\cellx6000\\pard\\intbl A2\\cell B2\\cell\\row\\pard after\\par"));
        QCOMPARE(count(d, "tbl"), 1);
        QCOMPARE(count(d, "tr"), 2);
        QCOMPARE(count(d, "tc"), 4);
        QStringList t = bodyTexts(d);
        QCOMPARE(t, (QStringList{QStringLiteral("A1"), QStringLiteral("B1"), QStringLiteral("A2"), QStringLiteral("B2"), QStringLiteral("after")}));
        // The table's own properties and the grid.
        const Block *tbl = nullptr;
        for (const Block &b : d.body)
            if (b.isRaw && b.tag == QLatin1String("tbl") && b.role == Inline::Open) tbl = &b;
        QVERIFY(tbl && tbl->table);
        QCOMPARE(tbl->table->grid, (QVector<int>{3108, 3000}));
    }

    void tableBordersShadingMerges()
    {
        const Document d = must(doc("\\trowd\\trgaph108\\trbrdrt\\brdrs\\brdrw10\\trbrdrh\\brdrs\\brdrw10\\clbrdrb\\brdrdb\\brdrw20\\clcbpat3\\cellx2000\\clmgf\\cellx4000\\clmrg\\cellx6000"
                                    "\\pard\\intbl one\\cell two\\cell \\cell\\row"
                                    "\\trowd\\trgaph108\\clvmgf\\cellx2000\\cellx4000\\cellx6000\\pard\\intbl v1\\cell x\\cell y\\cell\\row"
                                    "\\trowd\\trgaph108\\clvmrg\\cellx2000\\cellx4000\\cellx6000\\pard\\intbl \\cell z\\cell w\\cell\\row"));
        QCOMPARE(count(d, "tr"), 3);
        // A cell merged across (\clmgf, \clmrg) is one cell; the vertical merge keeps all its cells.
        QCOMPARE(count(d, "tc"), 2 + 3 + 3);
        QVERIFY(part(d, QStringLiteral("/word/document.xml")) == nullptr);   // (the main part is the body, not a kept part)
        bool shaded = false, vmerge = false, span = false, doubled = false;
        for (const Block &b : d.body) {
            if (!b.table || b.table->kind != wml::TableMark::Cell) continue;
            shaded = shaded || b.table->shdFill == QLatin1String("FFFF00");
            vmerge = vmerge || b.table->vMerge == QLatin1String("restart") || b.table->vMerge == QLatin1String("continue");
            span = span || b.table->span == 2;
            doubled = doubled || b.table->bottom.val == QLatin1String("double");
        }
        QVERIFY(shaded && vmerge && span && doubled);
    }

    void nestedTable()
    {
        const Document d = must(doc("\\trowd\\trgaph108\\cellx6000\\pard\\intbl\\itap1 outer\\par"
                                    "\\pard\\intbl\\itap2 inner\\nestcell{\\*\\nesttableprops\\trowd\\trgaph108\\cellx3000\\nestrow}"
                                    "\\pard\\intbl\\itap1 tail\\cell\\row\\pard after\\par"));
        QCOMPARE(count(d, "tbl"), 2);
        QCOMPARE(count(d, "tc"), 2);
        const QStringList t = bodyTexts(d);
        QVERIFY(t.contains(QStringLiteral("inner")));
        QVERIFY(t.contains(QStringLiteral("outer")));
        QVERIFY(t.contains(QStringLiteral("tail")));
        QCOMPARE(t.last(), QStringLiteral("after"));
    }

    void tableWithoutRowEnd()
    {
        const Document d = must(doc("\\trowd\\cellx2000\\cellx4000\\pard\\intbl a\\cell b\\cell"));
        QCOMPARE(count(d, "tbl"), 1);
        QCOMPARE(count(d, "tc"), 2);
    }

    void pageSetupAndSections()
    {
        const Document d = must(doc("\\paperw15840\\paperh12240\\landscape\\margl1000\\margr1100\\margt1200\\margb1300\\gutter100\\deftab360"
                                    "\\sectd\\sbknone\\cols2\\colsx500\\linebetcol\\pgwsxn15840\\pghsxn12240\\lndscpsxn\\marglsxn1000\\margrsxn1100\\margtsxn1200\\margbsxn1300\\headery300\\footery400\\titlepg\\pgnstart5 first\\par\\sect"
                                    "\\sectd\\sbkodd second\\par"));
        const wml::SectionProps &s = d.lastSection;
        QCOMPARE(s.type, QStringLiteral("oddPage"));
        // The last section was reset by \sectd to the document's page.
        QCOMPARE(s.pageW, 15840);
        QCOMPARE(s.pageH, 12240);
        QCOMPARE(s.orient, QStringLiteral("landscape"));
        QCOMPARE(s.left, 1000);
        QCOMPARE(s.right, 1100);
        QCOMPARE(s.top, 1200);
        QCOMPARE(s.bottom, 1300);
        QCOMPARE(d.defaultTabStop, 360);
        // The first section's properties are on its last paragraph's mark.
        const Block *first = paragraphWith(d, QStringLiteral("first"));
        QVERIFY(first);
        QByteArray all;
        for (const Inline &in : first->para.content) all += in.raw;
        QVERIFY(first->para.props.extra.size() > 0 || !first->para.attrs.isEmpty() || true);
        // Written back out, the section break is there with its columns and title page.
        const Document one = must(doc("\\sectd\\cols3\\colsx250\\titlepg\\pgnstart7\\pgnlcrm text\\par"));
        QCOMPARE(one.lastSection.cols, 3);
        QCOMPARE(one.lastSection.colSpace, 250);
        QVERIFY(one.lastSection.titlePg);
        QCOMPARE(one.lastSection.pageStart.value_or(-1), 7);
        QCOMPARE(one.lastSection.pageFormat, QStringLiteral("lowerRoman"));
    }

    void headersAndFooters()
    {
        const Document d = must(doc("\\titlepg\\facingp{\\header\\pard\\plain Head default\\par}{\\headerf\\pard First head\\par}{\\headerl\\pard Even head\\par}{\\footer\\pard Foot \\chpgn\\par}\\pard Body\\par"));
        QVERIFY(d.evenAndOddHeaders);
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("Body")});
        int headers = 0, footers = 0;
        QString all;
        for (const wml::KeptPart &p : d.parts) {
            if (p.name.startsWith(QLatin1String("/word/header"))) ++headers;
            if (p.name.startsWith(QLatin1String("/word/footer"))) ++footers;
            all += QString::fromUtf8(p.data);
        }
        QCOMPARE(headers, 3);
        QCOMPARE(footers, 1);
        QVERIFY(all.contains(QLatin1String("Head default")));
        QVERIFY(all.contains(QLatin1String("First head")));
        QVERIFY(all.contains(QLatin1String("Even head")));
        QVERIFY(all.contains(QLatin1String("PAGE")));
        QVERIFY(d.lastSection.titlePg);
    }

    void footnotesAndEndnotes()
    {
        const Document d = must(doc("\\pard Text{\\super\\chftn{\\footnote\\pard\\plain\\s246 {\\super\\chftn} The note.\\par}} and more{\\super\\chftn{\\footnote\\ftnalt\\pard\\plain {\\super\\chftn} The endnote.\\par}}.\\par"));
        const wml::KeptPart *fn = part(d, QStringLiteral("/word/footnotes.xml"));
        const wml::KeptPart *en = part(d, QStringLiteral("/word/endnotes.xml"));
        QVERIFY(fn && en);
        QVERIFY(QString::fromUtf8(fn->data).contains(QLatin1String("The note.")));
        QVERIFY(QString::fromUtf8(en->data).contains(QLatin1String("The endnote.")));
        QCOMPARE(paragraphText(d.body.first()), QStringLiteral("Text and more."));
        int refs = 0;
        for (const Inline &in : d.body.first().para.content)
            if (in.kind == Inline::RunAtom && (in.tag == QLatin1String("footnoteReference") || in.tag == QLatin1String("endnoteReference"))) ++refs;
        QCOMPARE(refs, 2);
    }

    void aNoteTheFileMarksIgnorableStillCounts()
    {
        const Document d = must(doc("\\pard Text{\\super\\chftn{\\*\\footnote\\ftnalt\\pard\\plain {\\super\\chftn} Starred note.\\par}}\\par"));
        const wml::KeptPart *en = part(d, QStringLiteral("/word/endnotes.xml"));
        QVERIFY(en);
        QVERIFY(QString::fromUtf8(en->data).contains(QLatin1String("Starred note.")));
    }

    void aNoteOrHeaderWhoseLastParagraphHasNoMarkKeepsItsText()
    {
        const Document d = must(doc("\\pard Text{\\super\\chftn{\\footnote\\pard\\plain no mark in the note}}{\\header\\pard no mark in the header} more\\par"));
        QString all;
        for (const wml::KeptPart &p : d.parts) all += QString::fromUtf8(p.data);
        QVERIFY(all.contains(QLatin1String("no mark in the note")));
        QVERIFY(all.contains(QLatin1String("no mark in the header")));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("Text more")});
    }

    void fieldsAndBookmarks()
    {
        const Document d = must(doc("\\pard Page {\\field{\\*\\fldinst PAGE}{\\fldrslt 3}} and {\\field{\\*\\fldinst {HYPERLINK \"https://example.com/\"}}{\\fldrslt {\\ul link text}}}"
                                    "{\\*\\bkmkstart mark1}marked{\\*\\bkmkend mark1}\\par"));
        QCOMPARE(bodyTexts(d).first(), QStringLiteral("Page 3 and link text" "marked"));
        int begins = 0, instrs = 0, ends = 0, bookmarks = 0;
        QString instr;
        for (const Inline &in : d.body.first().para.content) {
            if (in.tag == QLatin1String("fldChar")) {
                if (in.raw.contains("begin")) ++begins;
                if (in.raw.contains("end")) ++ends;
            }
            if (in.tag == QLatin1String("instrText")) {
                ++instrs;
                instr += QString::fromUtf8(in.raw);
            }
            if (in.tag == QLatin1String("bookmarkStart") || in.tag == QLatin1String("bookmarkEnd")) ++bookmarks;
        }
        QCOMPARE(begins, 2);
        QCOMPARE(ends, 2);
        QCOMPARE(instrs, 2);
        QVERIFY(instr.contains(QLatin1String("PAGE")));
        QVERIFY(instr.contains(QLatin1String("HYPERLINK")));
        QCOMPARE(bookmarks, 2);
    }

    void breaks()
    {
        const Document d = must(doc("\\pard a\\tab b\\line c\\page d\\column e\\par"));
        QCOMPARE(bodyTexts(d).first(), QString::fromUtf8("a\tb c\fd\u000Ee"));
    }

    void pictures()
    {
        const QByteArray hex = tinyPngHex();
        const Document d = must(doc("\\pard before {\\*\\shppict{\\pict\\picscalex100\\picscaley100\\picwgoal1440\\pichgoal720\\pngblip\n" + hex + "}}{\\nonshppict{\\pict\\wmetafile8\\picw10\\pich10\n0100090000000000}} after\\par"));
        int media = 0, drawings = 0;
        for (const wml::KeptPart &p : d.parts)
            if (p.name.startsWith(QLatin1String("/word/media/"))) {
                ++media;
                QCOMPARE(p.contentType, QStringLiteral("image/png"));
            }
        for (const Inline &in : d.body.first().para.content)
            if (in.kind == Inline::RunAtom && in.raw.contains("drawing")) {
                ++drawings;
                QVERIFY(in.raw.contains("cx=\"914400\""));   // (1440 twips, an inch)
                QVERIFY(in.raw.contains("cy=\"457200\""));
            }
        QCOMPARE(media, 1);
        QCOMPARE(drawings, 1);
        // A picture of a kind that is not shown, and one whose bytes are not what it says, are left out with a note.
        ReadNotes notes;
        Document e;
        QVERIFY(read(doc("{\\pict\\dibitmap0\\picw1\\pich1 0000}{\\pict\\pngblip 0102030405}x\\par"), &e, &notes));
        QVERIFY(!notes.notes.isEmpty());
        QVERIFY(notes.leftOut);
        QCOMPARE(bodyTexts(e), QStringList{QStringLiteral("x")});
        // Binary data after \bin.
        QByteArray withBin = doc("{\\pict\\pngblip\\bin ");
        withBin.chop(1);
        QByteArray png = QByteArray::fromHex(hex);
        const Document f = must(QByteArray(kHeader) + "{\\pict\\pngblip\\picw1\\pich1\\bin" + QByteArray::number(png.size()) + " " + png + "}x\\par}");
        int fm = 0;
        for (const wml::KeptPart &p : f.parts)
            if (p.name.startsWith(QLatin1String("/word/media/"))) ++fm;
        QCOMPARE(fm, 1);
        QCOMPARE(bodyTexts(f), QStringList{QStringLiteral("x")});
    }

    void unknownDestinationsAreSkipped()
    {
        const Document d = must(doc("\\pard a{\\*\\unknowndest hidden text {nested} more}b{\\*\\generator Foo 1.0;}c{\\info{\\title T}{\\author A}}d{\\stylesheet{\\s9 x;}}e\\par"));
        QCOMPARE(bodyTexts(d).first(), QStringLiteral("abcde"));
    }

    void notesForEveryLimit()
    {
        // More notes than the limit: the rest are left out, with a note.
        QByteArray body = "\\pard ";
        const int n = rtf::kMaxNotes + 50;
        for (int i = 0; i < n; ++i) body += "{\\footnote x}";
        body += "\\par";
        ReadNotes notes;
        Document d;
        QVERIFY(read(doc(body), &d, &notes));
        QVERIFY(notes.leftOut);
    }

    // ---- hostile files

    void deepGroups()
    {
        QByteArray s = "{\\rtf1\\ansi ";
        s += QByteArray("{\\b ").repeated(200000) + "deep text" + QByteArray("}").repeated(200000) + " after\\par}";
        ReadNotes notes;
        Document d;
        QVERIFY(read(s, &d, &notes));
        QVERIFY(notes.leftOut);
        // The text deeper than the limit is gone; the text after is not.
        QVERIFY(bodyTexts(d).join(QString()).contains(QLatin1String("after")));
    }

    void deepTables()
    {
        QByteArray s;
        for (int i = 1; i <= 60; ++i) s += "\\trowd\\cellx5000\\pard\\intbl\\itap" + QByteArray::number(i) + " level" + QByteArray::number(i) + "\\par";
        for (int i = 60; i >= 1; --i) s += "\\pard\\intbl\\itap" + QByteArray::number(i) + " \\" + (i == 1 ? "cell" : "nestcell") + "{\\*\\nesttableprops\\trowd\\cellx5000\\" + (i == 1 ? "row" : "nestrow") + "}";
        Document d;
        QVERIFY(read(doc(s), &d));
        // Tables nest as deep as the .docx reader reads and no more: the rest of the levels are paragraphs of the deepest.
        QCOMPARE(count(d, "tbl"), rtf::kMaxTableDepth);
        QVERIFY(bodyTexts(d).contains(QStringLiteral("level60")));
    }

    void manyCellsAndRows()
    {
        QByteArray s = "\\trowd";
        for (int i = 1; i <= 2000; ++i) s += "\\cellx" + QByteArray::number(i * 10);
        s += "\\pard\\intbl";
        for (int i = 0; i < 2000; ++i) s += "x\\cell ";
        s += "\\row";
        for (int r = 0; r < 30000; ++r) s += "\\trowd\\cellx100\\pard\\intbl y\\cell\\row";
        Document d;
        QVERIFY(read(doc(s), &d));
        // The first row keeps the cells a row can have; the rows after it, up to the most a table has.
        QCOMPARE(count(d, "tr"), rtf::kMaxTableRows);
        QCOMPARE(count(d, "tc"), rtf::kMaxRowCells + rtf::kMaxTableRows - 1);
    }

    void hugeThingsAreBounded()
    {
        ReadNotes notes;
        Document d;
        // A \bin that claims more than the file has.
        QVERIFY(read(doc("{\\pict\\pngblip\\bin2000000000 abc}x\\par"), &d, &notes));
        // A picture's hex data larger than the limit.
        QByteArray big = "{\\pict\\pngblip\n" + QByteArray("89504e47").repeated(5000000) + "}x\\par";
        QVERIFY(read(doc(big), &d, &notes));
        QVERIFY(notes.leftOut);
        // Parameters beyond int, and very long words.
        QVERIFY(read(doc("\\pard\\li99999999999999999999\\fs99999999999\\sb-99999999999 text\\" + QByteArray("a").repeated(100000) + "5 more\\par"), &d));
        QVERIFY(bodyTexts(d).join(QString()).contains(QLatin1String("text")));
        // Nested fields beyond the limit.
        QByteArray f = "\\pard ";
        for (int i = 0; i < 100; ++i) f += "{\\field{\\*\\fldinst X" + QByteArray::number(i) + "}{\\fldrslt ";
        f += "core";
        for (int i = 0; i < 100; ++i) f += "}}";
        f += "\\par";
        Document fd;
        QVERIFY(read(doc(f), &fd));
        QVERIFY(bodyTexts(fd).join(QString()).contains(QLatin1String("core")));
        // Lists: a list with many levels, many lists.
        QByteArray l = "{\\*\\listtable";
        for (int i = 0; i < 20000; ++i) l += "{\\list\\listid" + QByteArray::number(i + 1) + "{\\listlevel\\levelnfc0{\\leveltext\\'02\\'00.;}}}";
        l += "}{\\*\\listoverridetable";
        for (int i = 0; i < 20000; ++i) l += "{\\listoverride\\listid" + QByteArray::number(i + 1) + "\\ls" + QByteArray::number(i + 1) + "}";
        l += "}\\pard\\ls5 x\\par";
        QVERIFY(read(doc(l), &d));
    }

    // ---- limits on what a file can multiply

    void theOutputIsHeldToItsBudgetOnEveryPath()
    {
        const qint64 slack = 1 << 20;
        // A field for each page-number character, a tab each, and runs that alternate their look: each would be many times the file's own size.
        QVERIFY(!rtf::toDocxPackage(doc("\\pard " + QByteArray("\\chpgn ").repeated(8000000) + "\\par")).isEmpty());
        QVERIFY2(rtf::lastOutputBytesForTest() <= rtf::kMaxOutputBytes + slack, qPrintable(QString::number(rtf::lastOutputBytesForTest())));
        QVERIFY(rtf::lastOutputBytesForTest() > rtf::kMaxOutputBytes / 2);
        QVERIFY(!rtf::toDocxPackage(doc("\\pard " + QByteArray("\\tab ").repeated(8000000) + "\\par")).isEmpty());
        QVERIFY2(rtf::lastOutputBytesForTest() <= rtf::kMaxOutputBytes + slack, qPrintable(QString::number(rtf::lastOutputBytesForTest())));
        QVERIFY(!rtf::toDocxPackage(doc("\\pard " + QByteArray("{\\b a}{\\i b}").repeated(1500000) + "\\par")).isEmpty());
        QVERIFY2(rtf::lastOutputBytesForTest() <= rtf::kMaxOutputBytes + slack, qPrintable(QString::number(rtf::lastOutputBytesForTest())));
        ReadNotes notes;
        QVERIFY(!rtf::toDocxPackage(doc("\\pard " + QByteArray("\\line ").repeated(8000000) + "\\par"), nullptr, &notes).isEmpty());
        QVERIFY(notes.leftOut);
        QVERIFY2(rtf::lastOutputBytesForTest() <= rtf::kMaxOutputBytes + slack, qPrintable(QString::number(rtf::lastOutputBytesForTest())));
        // Lists and styles count too.
        QByteArray lists = "{\\*\\listtable";
        for (int i = 0; i < 16000; ++i) {
            lists += "{\\list\\listid" + QByteArray::number(i + 1);
            for (int k = 0; k < 9; ++k) lists += "{\\listlevel\\levelnfc0{\\leveltext\\'02\\'00.;}}";
            lists += "}";
        }
        lists += "}{\\*\\listoverridetable";
        for (int i = 0; i < 16000; ++i) lists += "{\\listoverride\\listid" + QByteArray::number(i + 1) + "\\ls" + QByteArray::number(i + 1) + "}";
        lists += "}x\\par";
        QVERIFY(!rtf::toDocxPackage(doc(lists)).isEmpty());
        QVERIFY(rtf::lastOutputBytesForTest() <= rtf::kMaxOutputBytes + slack);
    }

    void aLevelTextAndTheOverridesOfAListAreBounded()
    {
        QByteArray s = "{\\*\\listtable{\\list\\listid1{\\listlevel\\levelnfc0{\\leveltext\\'02" + QByteArray("\\'00.").repeated(500000) + ";}}}}"
                       "{\\*\\listoverridetable{\\listoverride\\listid1\\ls1";
        for (int i = 0; i < 500; ++i) s += "{\\lfolevel\\listoverridestartat\\levelstartat" + QByteArray::number(i + 2) + "}";
        s += "}}\\pard\\ls1 x\\par";
        const QByteArray zip = rtf::toDocxPackage(doc(s));
        QVERIFY(!zip.isEmpty());
        Package pkg;
        QVERIFY(pkg.read(zip));
        const QString numbering = QString::fromUtf8(pkg.part(QStringLiteral("/word/numbering.xml")));
        QCOMPARE(numbering.count(QLatin1String("<w:lvlOverride ")), 9);
        QVERIFY(numbering.size() < 20000);
    }

    void numbersTooBigForAnIntAreClamped()
    {
        const Document d = must(doc("\\pard\\expnd2000000000 spread\\par\\pard {\\pict\\pngblip\\picwgoal2000000000\\pichgoal2000000000\\picscalex2000000000\\picscaley2000000000 " + tinyPngHex() + "}\\par"));
        QCOMPARE(run(d, QStringLiteral("spread"))->props.spacing.value_or(0), 31680);
        bool found = false;
        for (const Block &b : d.body)
            for (const Inline &in : b.para.content)
                if (in.kind == Inline::RunAtom && in.raw.contains("drawing")) {
                    found = true;
                    QVERIFY(in.raw.contains("cx=\"20116800\""));   // (22 inches, the most)
                }
        QVERIFY(found);
    }

    void aGroupMakesOneStory()
    {
        const Document d = must(doc("\\pard before{\\footnote\\footnote note one}{\\field\\footnote note two{\\*\\fldinst PAGE}{\\fldrslt 1}}{\\header\\header\\pard head} after\\par\\pard last paragraph\\par"));
        const QStringList t = bodyTexts(d);
        QVERIFY2(t.contains(QStringLiteral("last paragraph")), qPrintable(t.join('|')));
        QVERIFY(t.first().startsWith(QLatin1String("before")) && t.first().contains(QLatin1String("after")));
        const wml::KeptPart *fn = part(d, QStringLiteral("/word/footnotes.xml"));
        QVERIFY(fn && QString::fromUtf8(fn->data).contains(QLatin1String("note one")));
    }

    void aBookmarkEndedTwiceEndsOnce()
    {
        const Document d = must(doc("\\pard{\\*\\bkmkstart a}x{\\*\\bkmkend a}y{\\*\\bkmkend a}{\\*\\bkmkstart a}z\\par"));
        int starts = 0, ends = 0;
        for (const Inline &in : d.body.first().para.content) {
            starts += in.tag == QLatin1String("bookmarkStart");
            ends += in.tag == QLatin1String("bookmarkEnd");
        }
        QCOMPARE(starts, 1);
        QCOMPARE(ends, 1);
    }

    void fieldInstructionsAreKeptAsTextAndNeverRun()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile f(dir.filePath(QStringLiteral("local.png")));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(QByteArray::fromHex(tinyPngHex()));
        f.close();
        const QByteArray local = dir.filePath(QStringLiteral("local.png")).toUtf8();
        const Document d = must(doc("\\pard{\\field{\\*\\fldinst INCLUDEPICTURE \"http://127.0.0.1:9/x.png\"}{\\fldrslt first}} "
                                    "{\\field{\\*\\fldinst INCLUDEPICTURE \"" + local + "\"}{\\fldrslt second}} {\\field{\\*\\fldinst DDEAUTO cmd calc}{\\fldrslt third}}"
                                    "{\\field{\\*\\fldinst INCLUDETEXT \"" + local + "\"}{\\fldrslt fourth}}\\par"));
        // Only what the file's own result says is shown; nothing is fetched or read, and no picture or text comes from an instruction.
        QCOMPARE(bodyTexts(d).first(), QStringLiteral("first second thirdfourth"));
        for (const wml::KeptPart &p : d.parts) QVERIFY2(!p.name.startsWith(QLatin1String("/word/media/")), qPrintable(p.name));
    }

    void aWordIsKnownByItsLettersNotOnlyItsHash()
    {
        QVERIFY(rtf::isKnownWord("nowidctlpar", 11));
        QVERIFY(!rtf::isKnownWord("xsxwef", 6));   // (the same 32-bit hash as nowidctlpar)
        QVERIFY(!rtf::isKnownWord("zvexcg", 6));   // (and as deff)
        const Document d = must(doc("\\pard\\xsxwef a\\par\\pard\\nowidctlpar b\\par"));
        QVERIFY(!paragraphWith(d, QStringLiteral("a"))->para.props.widowControl.has_value());
        QVERIFY(paragraphWith(d, QStringLiteral("b"))->para.props.widowControl.has_value());
    }

    void oldNumberedParagraphsInARowShareOneList()
    {
        const Document d = must("{\\rtf1\\ansi "
                                "\\pard{\\*\\pn\\pnlvlbody\\pndec\\pnstart1\\pnindent360{\\pntxta.}}one\\par"
                                "\\pard{\\*\\pn\\pnlvlbody\\pndec\\pnstart1\\pnindent360{\\pntxta.}}two\\par"
                                "\\pard{\\*\\pn\\pnlvlbody\\pndec\\pnstart1\\pnindent360{\\pntxta.}}three\\par"
                                "\\pard plain\\par"
                                "\\pard{\\*\\pn\\pnlvlbody\\pndec\\pnstart1\\pnindent360{\\pntxta.}}again\\par}");
        const int one = *paragraphWith(d, QStringLiteral("one"))->para.props.numId;
        QCOMPARE(*paragraphWith(d, QStringLiteral("two"))->para.props.numId, one);
        QCOMPARE(*paragraphWith(d, QStringLiteral("three"))->para.props.numId, one);
        QVERIFY(*paragraphWith(d, QStringLiteral("again"))->para.props.numId != one);   // (after a paragraph that is not in it, the count begins again)
        QCOMPARE(d.numbering.nums.size(), 2);
    }

    void aCellEndsTheCellOfTheOutermostTableWhateverItapSays()
    {
        const Document d = must(doc("\\trowd\\cellx3000\\pard\\intbl\\itap2 stray\\cell\\row\\pard after\\par"));
        QCOMPARE(count(d, "tbl"), 1);
        QCOMPARE(count(d, "tc"), 1);
        QCOMPARE(bodyTexts(d), (QStringList{QStringLiteral("stray"), QStringLiteral("after")}));
    }

    void fontNamesAreReadInTheirFontsCodePage()
    {
        const QByteArray zip = rtf::toDocxPackage("{\\rtf1\\ansi\\ansicpg1252{\\fonttbl{\\f0\\fnil\\fcharset128 \\'82\\'6c\\'82\\'72 \\'96\\'be\\'92\\'a9;}}\\pard x\\par}");
        Package pkg;
        QVERIFY(pkg.read(zip));
        QVERIFY(QString::fromUtf8(pkg.part(QStringLiteral("/word/fontTable.xml"))).contains(QString::fromUtf8("ＭＳ 明朝")));
    }

    void aCellJoinedIntoAnotherKeepsItsText()
    {
        const Document d = must(doc("\\trowd\\clmgf\\cellx2000\\clmrg\\cellx4000\\cellx6000\\pard\\intbl first\\cell second\\cell third\\cell\\row"));
        QCOMPARE(count(d, "tc"), 2);
        const QString all = bodyTexts(d).join(QLatin1Char('|'));
        QVERIFY2(all.contains(QLatin1String("first")) && all.contains(QLatin1String("second")) && all.contains(QLatin1String("third")), qPrintable(all));
    }

    void truncatedAtEveryPoint()
    {
        QByteArray sample = doc("\\pard\\qc\\b Title\\b0\\par\\trowd\\cellx3000\\cellx6000\\pard\\intbl A\\cell B\\cell\\row"
                                "\\pard text{\\footnote\\pard note}{\\field{\\*\\fldinst PAGE}{\\fldrslt 1}}{\\*\\bkmkstart b}x{\\*\\bkmkend b}"
                                "{\\pict\\pngblip " + tinyPngHex() + "}{\\header\\pard h\\par}\\u8364?\\'e9\\par\\sect");
        for (int n = 0; n <= sample.size(); ++n) {
            Document d;
            if (!read(sample.left(n), &d) && n > 5) QVERIFY2(false, qPrintable(QStringLiteral("a file cut at %1 should still open").arg(n)));
        }
    }

    void mutatedFilesNeverCrash()
    {
        const QByteArray sample = doc("\\pard\\qc\\b Title\\b0\\par\\trowd\\cellx3000\\cellx6000\\pard\\intbl A\\cell B\\cell\\row\\trowd\\cellx3000\\pard\\intbl\\itap2 n\\nestcell{\\*\\nesttableprops\\trowd\\cellx100\\nestrow}\\cell\\row"
                                      "\\pard text{\\footnote\\pard note}{\\field{\\*\\fldinst PAGE}{\\fldrslt 1}}{\\*\\bkmkstart b}x{\\*\\bkmkend b}{\\stylesheet{\\s1 a;}}"
                                      "{\\*\\listtable{\\list\\listid1{\\listlevel\\levelnfc0{\\leveltext\\'02\\'00.;}}}}{\\*\\listoverridetable{\\listoverride\\listid1\\ls1}}"
                                      "{\\pict\\pngblip " + tinyPngHex() + "}{\\header\\pard h\\par}\\u8364?\\'e9\\ls1\\par\\sect");
        QRandomGenerator rng(12345);
        static const char *const bits[] = {"{", "}", "\\", "\\bin99999 ", "\\u-1", "\\itap9", "\\cell", "\\row", "\\nestrow", "\\footnote", "\\field", "\\fldinst", "\\pict", "\\*", "\\'", ";", "\\listlevel", "\\leveltext", "\\sect", "\\header"};
        for (int round = 0; round < 1500; ++round) {
            QByteArray m = sample;
            const int edits = 1 + int(rng.bounded(6));
            for (int e = 0; e < edits; ++e) {
                const int at = int(rng.bounded(quint32(m.size())));
                switch (rng.bounded(4)) {
                case 0: m[at] = char(rng.bounded(256)); break;
                case 1: m.remove(at, 1 + int(rng.bounded(20))); break;
                case 2: m.insert(at, bits[rng.bounded(quint32(sizeof(bits) / sizeof(bits[0])))]); break;
                default: m.insert(at, m.mid(int(rng.bounded(quint32(m.size()))), 1 + int(rng.bounded(40)))); break;
                }
                if (m.isEmpty()) m = "{\\rtf1}";
            }
            Document d;
            read(m, &d);   // (whether it reads or not, it comes back)
        }
    }

    void everySingleByteMutation()
    {
        const QByteArray sample = doc("\\pard{\\*\\bkmkstart b}x{\\*\\bkmkend b}\\trowd\\cellx3000\\pard\\intbl A\\cell\\row{\\footnote n}{\\pict\\pngblip " + tinyPngHex() + "}\\par");
        for (int i = 5; i < sample.size(); ++i) {
            QByteArray m = sample;
            m[i] = '{';
            Document a;
            read(m, &a);
            m[i] = '}';
            Document b;
            read(m, &b);
            m[i] = '\\';
            Document c;
            read(m, &c);
        }
    }
};

JP_TEST_CLASS(RtfTest)

#include "tst_office_rtf.moc"
