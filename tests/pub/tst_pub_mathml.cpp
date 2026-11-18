// The two-way conversion of equations between the math markup of a .docx (m:oMath) and MathML, the form an .odt or .odp keeps them in: what each structure becomes, what a file written by another program
// reads as, what is kept that has no match, and the limits on a hostile part (nesting, size, element count, DOCTYPE and entities).
#include "pubhelpers.h"
#include "mathml/mathml.h"
#include "wml/wmlmathedit.h"
#include <QXmlStreamReader>

namespace {

using namespace jp::office;

const char kM[] = "xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"";
const char kMml[] = "xmlns=\"http://www.w3.org/1998/Math/MathML\"";

QByteArray omml(const QByteArray &inner) { return QByteArray("<m:oMath ") + kM + ">" + inner + "</m:oMath>"; }
QByteArray mml(const QByteArray &inner) { return QByteArray("<math ") + kMml + ">" + inner + "</math>"; }
QByteArray run(const char *text) { return QByteArray("<m:r><m:t>") + text + "</m:t></m:r>"; }

// The local names of the elements, in order.
QStringList names(const QByteArray &xml)
{
    QStringList out;
    QXmlStreamReader r(xml);
    while (!r.atEnd())
        if (r.readNext() == QXmlStreamReader::StartElement) out << r.name().toString();
    return out;
}

QByteArray toMathml(const QByteArray &markup, bool *ok = nullptr)
{
    bool good = false;
    const QByteArray out = mathml::mathmlFromOmml(markup, &good);
    if (ok) *ok = good;
    return out;
}

QByteArray toOmml(const QByteArray &xml, bool *ok = nullptr)
{
    bool good = false;
    const QByteArray out = mathml::ommlFromMathml(xml, &good);
    if (ok) *ok = good;
    return out;
}

} // namespace

class PubMathmlTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Every equation of the gallery gives the same line after going to MathML and back, and again after a second trip (so the MathML a file keeps does not drift when it is saved again).
    void everyGalleryEquationKeepsItsLineBothWays()
    {
        const auto gallery = wml::equationGallery();
        QVERIFY(gallery.size() >= 12);
        for (const auto &design : gallery) {
            const QByteArray first = wml::equationMarkup(design.linear);
            QVERIFY(!first.isEmpty());
            bool ok = false;
            const QByteArray m1 = toMathml(first, &ok);
            QVERIFY2(ok && m1.startsWith("<math "), qPrintable(design.linear));
            const QByteArray second = toOmml(m1, &ok);
            QVERIFY2(ok, qPrintable(design.linear));
            QCOMPARE(wml::equationLine(second), wml::equationLine(first));
            int at = 0;
            while (at < first.size() && at < second.size() && first[at] == second[at]) ++at;
            QVERIFY2(second == first, qPrintable(design.linear + QStringLiteral(" differs at ") + QString::number(at) + QStringLiteral(": ") + QString::fromUtf8(first.mid(at - 20, 90)) + QStringLiteral(" | ") + QString::fromUtf8(second.mid(at - 20, 90))));
            QVERIFY2(!wml::equationLine(first).isEmpty(), qPrintable(design.linear));
            const QByteArray m2 = toMathml(second, &ok);
            QVERIFY(ok);
            QCOMPARE(m2, m1);
            QCOMPARE(wml::equationLine(toOmml(m2)), wml::equationLine(first));
        }
    }

    // The structures of the gallery come out as the MathML elements that stand for them.
    void theGalleryUsesTheStandardElements()
    {
        QSet<QString> seen;
        for (const auto &design : wml::equationGallery())
            for (const QString &n : names(toMathml(wml::equationMarkup(design.linear)))) seen.insert(n);
        for (const char *want : {"math", "mrow", "mi", "mn", "mo", "mfrac", "msup", "msub", "msqrt", "munderover", "msubsup", "mtable", "mtr", "mtd", "mover"})
            QVERIFY2(seen.contains(QLatin1String(want)), want);
        QVERIFY(!seen.contains(QStringLiteral("annotation")));
    }

    // A part as the other program's .odt keeps it: a semantics element with the StarMath annotation, stretchy operators. The annotation is left out and the structure names are the markup's.
    void aPartWrittenByAnotherProgramReadsIntoTheMarkupStructures()
    {
        const QByteArray xml =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<math xmlns=\"http://www.w3.org/1998/Math/MathML\" display=\"block\"><semantics><mrow><mi>x</mi><mo stretchy=\"false\">=</mo>"
            "<mfrac><mrow><mo stretchy=\"false\">-</mo><mi>b</mi><mo>&#xB1;</mo><msqrt><mrow><msup><mi>b</mi><mn>2</mn></msup><mo stretchy=\"false\">-</mo><mn>4</mn><mi>a</mi><mi>c</mi></mrow></msqrt></mrow>"
            "<mrow><mn>2</mn><mi>a</mi></mrow></mfrac></mrow><annotation encoding=\"StarMath 5.0\">x = {-b plusminus sqrt {b^2 - 4ac}} over {2a}</annotation></semantics></math>";
        bool ok = false;
        const QByteArray out = toOmml(xml, &ok);
        QVERIFY(ok);
        const QStringList n = names(out);
        QCOMPARE(n.first(), QStringLiteral("oMath"));
        for (const char *want : {"f", "num", "den", "rad", "radPr", "degHide", "e", "sSup", "sup", "r", "t"})
            QVERIFY2(n.contains(QLatin1String(want)), want);
        QVERIFY(!n.contains(QStringLiteral("annotation")) && !n.contains(QStringLiteral("semantics")));
        QVERIFY(!out.contains("StarMath") && !out.contains("plusminus"));
        QCOMPARE(wml::equationLine(out), wml::equationLine(wml::equationMarkup(QStringLiteral("x=(-b\\pm\\sqrt(b^2-4ac))/(2a)"))));
        // Written back, a standard MathML 3 part: no semantics, no annotation, the namespace declared.
        const QByteArray back = toMathml(out);
        QVERIFY(back.startsWith("<math xmlns=\"http://www.w3.org/1998/Math/MathML\""));
        QVERIFY(!back.contains("semantics") && !back.contains("annotation"));
    }

    // The part of an .odt is read with a prefix on its names (math:math) as well as without.
    void aPrefixedPartReads()
    {
        const QByteArray xml = "<math:math xmlns:math=\"http://www.w3.org/1998/Math/MathML\"><math:msup><math:mi>x</math:mi><math:mn>2</math:mn></math:msup></math:math>";
        bool ok = false;
        const QByteArray out = toOmml(xml, &ok);
        QVERIFY(ok);
        QVERIFY(names(out).contains(QStringLiteral("sSup")));
        QCOMPARE(wml::equationLine(out), wml::equationLine(wml::equationMarkup(QStringLiteral("x^2"))));
    }

    void anNaryOperatorKeepsItsSymbolLimitsAndPlace()
    {
        const QByteArray sum = omml("<m:nary><m:naryPr><m:chr m:val=\"\xE2\x88\x91\"/><m:limLoc m:val=\"undOvr\"/></m:naryPr><m:sub>" + run("i") + "</m:sub><m:sup>" + run("n") + "</m:sup><m:e>" + run("x") + "</m:e></m:nary>");
        const QByteArray m = toMathml(sum);
        QVERIFY(m.contains("<munderover>") && m.contains("\xE2\x88\x91"));
        QByteArray back = toOmml(m);
        QVERIFY(back.contains("<m:chr m:val=\"\xE2\x88\x91\"/>") && back.contains("<m:limLoc m:val=\"undOvr\"/>") && !back.contains("subHide"));
        QCOMPARE(names(back).count(QStringLiteral("nary")), 1);
        QCOMPARE(wml::equationLine(back), wml::equationLine(sum));

        const QByteArray integral = omml("<m:nary><m:naryPr><m:chr m:val=\"\xE2\x88\xAB\"/><m:limLoc m:val=\"subSup\"/><m:supHide m:val=\"1\"/></m:naryPr><m:sub>" + run("0") + "</m:sub><m:sup/><m:e>" + run("f") + "</m:e></m:nary>");
        const QByteArray mi = toMathml(integral);
        QVERIFY(mi.contains("<msub>") && !mi.contains("<msubsup>"));
        back = toOmml(mi);
        QVERIFY(back.contains("<m:limLoc m:val=\"subSup\"/>") && back.contains("<m:supHide m:val=\"1\"/>") && !back.contains("<m:subHide"));
        QCOMPARE(wml::equationLine(back), wml::equationLine(integral));
        // What follows the operator in a line stays outside it.
        const QByteArray line = toOmml(mml("<mrow><munderover><mo>\xE2\x88\x91</mo><mi>i</mi><mi>n</mi></munderover><mi>x</mi><mo>+</mo><mn>1</mn></mrow>"));
        QVERIFY(line.indexOf("</m:nary>") < line.indexOf("<m:t>+</m:t>"));
    }

    void fencesSeparatorsAndMatricesKeepTheirShape()
    {
        const QByteArray d = omml("<m:d><m:dPr><m:begChr m:val=\"[\"/><m:sepChr m:val=\";\"/><m:endChr m:val=\"]\"/></m:dPr><m:e>" + run("a") + "</m:e><m:e>" + run("b") + "</m:e></m:d>");
        const QByteArray m = toMathml(d);
        QVERIFY(m.contains("fence=\"true\"") && m.contains("separator=\"true\""));
        const QByteArray back = toOmml(m);
        QVERIFY(back.contains("<m:begChr m:val=\"[\"/>") && back.contains("<m:sepChr m:val=\";\"/>") && back.contains("<m:endChr m:val=\"]\"/>"));
        QCOMPARE(names(back).count(QStringLiteral("e")), 2);
        QCOMPARE(wml::equationLine(back), wml::equationLine(d));
        // A fenced list as another program writes it.
        const QByteArray fen = toOmml(mml("<mfenced open=\"{\" close=\"}\"><mi>a</mi><mi>b</mi><mi>c</mi></mfenced>"));
        QVERIFY(fen.contains("<m:begChr m:val=\"{\"/>") && fen.contains("<m:sepChr m:val=\",\"/>"));
        QCOMPARE(names(fen).count(QStringLiteral("e")), 3);
        // A pair of plain brackets round a group is not a fence: it stays runs.
        QVERIFY(!names(toOmml(mml("<mrow><mo>(</mo><mi>x</mi><mo>)</mo></mrow>"))).contains(QStringLiteral("d")));

        const QByteArray matrix = wml::equationMarkup(QStringLiteral("\\pmatrix{a & b \\\\ c & d}"));
        const QByteArray mm = toMathml(matrix);
        QCOMPARE(names(mm).count(QStringLiteral("mtr")), 2);
        QCOMPARE(names(mm).count(QStringLiteral("mtd")), 4);
        const QByteArray mb = toOmml(mm);
        QCOMPARE(names(mb).count(QStringLiteral("mr")), 2);
        QVERIFY(mb.contains("<m:count m:val=\"2\"/>"));
        QCOMPARE(wml::equationLine(mb), wml::equationLine(matrix));
    }

    void accentsBarsAndRootsKeepTheirKind()
    {
        const QByteArray acc = wml::equationMarkup(QStringLiteral("\\hat{x}"));
        const QByteArray m = toMathml(acc);
        QVERIFY(m.contains("<mover accent=\"true\">"));
        QVERIFY(toOmml(m).contains("<m:chr m:val=\"\xCC\x82\"/>"));
        QCOMPARE(wml::equationLine(toOmml(m)), wml::equationLine(acc));
        const QByteArray bar = wml::equationMarkup(QStringLiteral("\\bar{x}"));
        QCOMPARE(wml::equationLine(toOmml(toMathml(bar))), wml::equationLine(bar));
        // A bar over the operand (not an accent) is the markup's bar.
        const QByteArray overline = omml("<m:bar><m:barPr><m:pos m:val=\"top\"/></m:barPr><m:e>" + run("x") + "</m:e></m:bar>");
        QVERIFY(names(toOmml(toMathml(overline))).contains(QStringLiteral("bar")));
        QCOMPARE(wml::equationLine(toOmml(toMathml(overline))), wml::equationLine(overline));
        QVERIFY(names(toOmml(mml("<mover><mi>x</mi><mo>\xC2\xAF</mo></mover>"))).contains(QStringLiteral("bar")));
        // An index root and a square root are told apart; a script on the left is a prescript.
        const QByteArray root = toOmml(mml("<mroot><mi>x</mi><mn>3</mn></mroot>"));
        QVERIFY(names(root).contains(QStringLiteral("deg")) && !root.contains("degHide"));
        QVERIFY(toOmml(mml("<msqrt><mi>x</mi></msqrt>")).contains("<m:degHide m:val=\"1\"/>"));
        QVERIFY(names(toOmml(mml("<mmultiscripts><mi>X</mi><none/><none/><mprescripts/><mn>1</mn><mn>2</mn></mmultiscripts>"))).contains(QStringLiteral("sPre")));
        QVERIFY(toMathml(omml("<m:rad><m:radPr><m:degHide m:val=\"1\"/></m:radPr><m:deg/><m:e>" + run("x") + "</m:e></m:rad>")).contains("<msqrt>"));
        // A fraction with no bar.
        QVERIFY(toOmml(mml("<mfrac linethickness=\"0\"><mi>a</mi><mi>b</mi></mfrac>")).contains("<m:type m:val=\"noBar\"/>"));
        QVERIFY(toMathml(omml("<m:f><m:fPr><m:type m:val=\"noBar\"/></m:fPr><m:num>" + run("a") + "</m:num><m:den>" + run("b") + "</m:den></m:f>")).contains("linethickness=\"0\""));
        // A function name and its argument.
        const QByteArray fn = omml("<m:func><m:fName><m:r><m:rPr><m:sty m:val=\"p\"/></m:rPr><m:t>sin</m:t></m:r></m:fName><m:e>" + run("x") + "</m:e></m:func>");
        QVERIFY(names(toOmml(toMathml(fn))).contains(QStringLiteral("func")));
        QCOMPARE(wml::equationLine(toOmml(toMathml(fn))), wml::equationLine(fn));
    }

    void normalTextAndStylesKeepTheirKind()
    {
        const QByteArray nor = omml("<m:r><m:rPr><m:nor m:val=\"1\"/></m:rPr><m:t>if x</m:t></m:r>");
        const QByteArray m = toMathml(nor);
        QVERIFY(m.contains("<mtext>if x</mtext>"));
        QVERIFY(toOmml(m).contains("<m:nor m:val=\"1\"/>") && toOmml(m).contains("if x"));
        const QByteArray bold = toMathml(omml("<m:r><m:rPr><m:sty m:val=\"b\"/></m:rPr><m:t>x</m:t></m:r>"));
        QVERIFY(bold.contains("<mi mathvariant=\"bold\">x</mi>"));
        QVERIFY(toOmml(bold).contains("<m:sty m:val=\"b\"/>"));
        const QByteArray script = toOmml(mml("<mi mathvariant=\"double-struck\">R</mi>"));
        QVERIFY(script.contains("<m:scr m:val=\"double-struck\"/>"));
        // A name of several letters is upright, one letter is not; digits, and decimals, are numbers; the rest are operators.
        const QByteArray word = toOmml(mml("<mi>sin</mi><mn>3.5</mn><mo>+</mo>"));
        QVERIFY(word.contains("<m:sty m:val=\"p\"/>") && word.contains("<m:t>sin</m:t>") && word.contains("<m:t>3.5</m:t>") && word.contains("<m:t>+</m:t>"));
        QVERIFY(!toOmml(mml("<mi>x</mi>")).contains("<m:sty"));
        const QByteArray tokens = toMathml(omml(run("a1+2")));
        QVERIFY(tokens.contains("<mi>a</mi><mn>1</mn><mo>+</mo><mn>2</mn>"));
    }

    // What has no match is kept as the text of a run, from either side, and an annotation or a script is not shown as text.
    void whatCannotBeMappedIsKeptAsText()
    {
        const QByteArray out = toOmml(mml("<mrow><mi>a</mi><foo><bar>kept words</bar></foo></mrow>"));
        QVERIFY(out.contains("<m:t>kept words</m:t>"));
        QVERIFY(toMathml(omml("<m:mystery><m:r><m:t>zz</m:t></m:r></m:mystery>")).contains("zz"));
        QVERIFY(!toOmml(mml("<mi>a</mi><annotation>hidden</annotation>")).contains("hidden"));
    }

    void markupIsEscaped()
    {
        const QByteArray out = toOmml(mml("<mo>&lt;</mo><mi>a</mi><mo>&amp;</mo>"));
        QVERIFY(out.contains("<m:t>&lt;</m:t>") && out.contains("<m:t>&amp;</m:t>"));
        QVERIFY(toMathml(omml("<m:r><m:t>&lt;&amp;&gt;</m:t></m:r>")).contains("&lt;"));
        bool ok = false;
        toOmml(toMathml(omml(run("a&amp;b"))), &ok);
        QVERIFY(ok);
    }

    // The nesting limit: exactly kMostDepth levels (the root counts) are read, one more is refused; both ways.
    void nestingIsLimitedToSixtyFourLevels()
    {
        QCOMPARE(mathml::kMostDepth, 64);
        auto nested = [](const char *open, const char *close, int levels, const QByteArray &leaf) {
            QByteArray s;
            for (int i = 0; i < levels; ++i) s += open;
            s += leaf;
            for (int i = 0; i < levels; ++i) s += close;
            return s;
        };
        // math + 62 mrow + mi = 64 deep
        bool ok = false;
        toOmml(mml(nested("<mrow>", "</mrow>", 62, "<mi>x</mi>")), &ok);
        QVERIFY(ok);
        toOmml(mml(nested("<mrow>", "</mrow>", 63, "<mi>x</mi>")), &ok);   // 65 deep
        QVERIFY(!ok);
        // oMath + 61 e + r + t = 64 deep
        toMathml(omml(nested("<m:e>", "</m:e>", 61, run("x"))), &ok);
        QVERIFY(ok);
        toMathml(omml(nested("<m:e>", "</m:e>", 62, run("x"))), &ok);
        QVERIFY(!ok);
    }

    // What the editor makes nests two levels for each fraction: a continued fraction 20 deep is read, both ways, and so is the nesting another program writes (wrappers in wrappers).
    void deepEquationsFromTheEditorAndAnotherProgramConvert()
    {
        QString linear = QStringLiteral("1");
        for (int i = 0; i < 20; ++i) linear = QStringLiteral("1+1/(") + linear + QLatin1Char(')');
        const QByteArray markup = wml::equationMarkup(linear);
        QVERIFY(!markup.isEmpty());
        bool ok = false;
        const QByteArray m = toMathml(markup, &ok);
        QVERIFY(ok);
        QCOMPARE(m.count("<mfrac>"), 20);
        const QByteArray back = toOmml(m, &ok);
        QVERIFY(ok);
        QCOMPARE(wml::equationLine(back), wml::equationLine(markup));
        QByteArray part = "<mi>x</mi>";
        for (int i = 0; i < 10; ++i) part = "<mrow><mstyle mathvariant=\"normal\"><msup>" + part + "<mn>2</mn></msup></mstyle></mrow>";
        const QByteArray deep = toOmml(mml(part), &ok);
        QVERIFY(ok);
        QCOMPARE(names(deep).count(QStringLiteral("sSup")), 10);
    }

    void elementsAreLimitedToFourThousand()
    {
        QCOMPARE(mathml::kMostElements, 4000);
        bool ok = false;
        QByteArray body;
        for (int i = 0; i < 3999; ++i) body += "<mi>x</mi>";
        QVERIFY(!toOmml(mml(body), &ok).isEmpty());
        QVERIFY(ok);   // math + 3999 = 4000
        toOmml(mml(body + "<mi>x</mi>"), &ok);
        QVERIFY(!ok);
        QByteArray inner;
        for (int i = 0; i < 3999; ++i) inner += "<m:e/>";
        toMathml(omml(inner), &ok);
        QVERIFY(ok);
        toMathml(omml(inner + "<m:e/>"), &ok);
        QVERIFY(!ok);
    }

    void sizeIsLimitedToOneMegabyte()
    {
        QCOMPARE(mathml::kMostBytes, 1 << 20);
        QByteArray part = mml("<mi>x</mi>");
        part += QByteArray(mathml::kMostBytes - part.size(), ' ');
        QCOMPARE(part.size(), 1 << 20);
        bool ok = false;
        QVERIFY(!toOmml(part, &ok).isEmpty());
        QVERIFY(ok);
        toOmml(part + " ", &ok);
        QVERIFY(!ok);
        QByteArray markup = omml(run("x"));
        markup += QByteArray(mathml::kMostBytes - markup.size(), ' ');
        toMathml(markup, &ok);
        QVERIFY(ok);
        toMathml(markup + " ", &ok);
        QVERIFY(!ok);
    }

    // No DOCTYPE, so no entity is ever defined: a harmless one is refused as well as an external one and a bomb.
    void aDoctypeIsRefused()
    {
        QString error;
        bool ok = true;
        QByteArray out = mathml::ommlFromMathml("<!DOCTYPE math [<!ENTITY a \"x\">]><math><mi>&a;</mi></math>", &ok, &error);
        QVERIFY(!ok && out.isEmpty());
        QVERIFY2(error.contains(QLatin1String("DOCTYPE")), qPrintable(error));
        ok = true;
        out = mathml::ommlFromMathml("<!DOCTYPE math SYSTEM \"http://127.0.0.1:9/x.dtd\"><math><mi>x</mi></math>", &ok, &error);
        QVERIFY(!ok && out.isEmpty());
        ok = true;
        out = mathml::mathmlFromOmml("<!DOCTYPE m [<!ENTITY a \"x\">]><m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"><m:r><m:t>&a;</m:t></m:r></m:oMath>", &ok, &error);
        QVERIFY(!ok && out.isEmpty());
        QVERIFY2(error.contains(QLatin1String("DOCTYPE")), qPrintable(error));
    }

    void aBillionLaughsFileIsRefusedAtOnce()
    {
        QByteArray xml = "<?xml version=\"1.0\"?><!DOCTYPE lolz [<!ENTITY lol \"lol\">";
        xml += "<!ENTITY lol1 \"&lol;&lol;&lol;&lol;&lol;&lol;&lol;&lol;&lol;&lol;\">";
        for (int i = 2; i <= 9; ++i)
            xml += "<!ENTITY lol" + QByteArray::number(i) + " \"" + QByteArray("&lol" + QByteArray::number(i - 1) + ";").repeated(10) + "\">";
        xml += "]><math><mtext>&lol9;</mtext></math>";
        QString error;
        bool ok = true;
        const QByteArray out = mathml::ommlFromMathml(xml, &ok, &error);
        QVERIFY(!ok && out.isEmpty());
        QVERIFY2(error.contains(QLatin1String("DOCTYPE")), qPrintable(error));
    }

    // One megabyte of blanks was 25 MB of output. A run of blanks is one space of that width and reads back as that many.
    void aRunOfBlanksIsOneSpace()
    {
        bool ok = false;
        const QByteArray big = toMathml(omml("<m:r><m:t xml:space=\"preserve\">" + QByteArray(900000, ' ') + "</m:t></m:r>"), &ok);
        QVERIFY(ok);
        QVERIFY2(big.size() < 1000, qPrintable(QString::number(big.size())));
        QCOMPARE(names(big).count(QStringLiteral("mspace")), 1);
        const QByteArray five = toOmml(toMathml(omml("<m:r><m:t xml:space=\"preserve\">a     b</m:t></m:r>")));
        QVERIFY(five.contains("<m:t>a</m:t>") && five.contains("<m:t>b</m:t>"));
        QVERIFY(five.contains("<m:t xml:space=\"preserve\">     </m:t>"));
    }

    // A long run of one operator character is one operator; mixed ones cannot be shortened and are refused past the output limit, in elements, exactly at it.
    void outputIsLimitedAndLongOperatorRunsAreOne()
    {
        bool ok = false;
        const QByteArray plus = toMathml(omml(run(QByteArray(100000, '+').constData())), &ok);
        QVERIFY(ok);
        QCOMPARE(names(plus).count(QStringLiteral("mo")), 1);
        QCOMPARE(toOmml(plus).count('+'), 100000);
        QCOMPARE(mathml::kMostOutputElements, 20000);
        // math + mrow + 19998 letters = 20000 elements
        toMathml(omml(run(QByteArray(19998, 'x').constData())), &ok);
        QVERIFY(ok);
        QString error;
        mathml::mathmlFromOmml(omml(run(QByteArray(19999, 'x').constData())), &ok, &error);
        QVERIFY(!ok);
        QVERIFY2(error.contains(QLatin1String("output")), qPrintable(error));
        QByteArray mixed;
        for (int i = 0; i < 100000; ++i) mixed += "+-";
        toMathml(omml(run(mixed.constData())), &ok);
        QVERIFY(!ok);
        // The other way the output cannot pass the limit from 4,000 elements (5 at the most for each): the worst case is read.
        QByteArray body;
        for (int i = 0; i < 3999; ++i) body += "<mi mathvariant=\"bold-script\">x</mi>";
        toOmml(mml(body), &ok);
        QVERIFY(ok);
    }

    // `(a)+(b)` written with fences is two groups, not one that runs from the first fence to the last.
    void twoFencedGroupsInARowStayTwo()
    {
        const QByteArray two = toOmml(mml("<mrow><mo fence=\"true\" stretchy=\"true\">(</mo><mi>a</mi><mo fence=\"true\" stretchy=\"true\">)</mo><mo>+</mo>"
                                          "<mo fence=\"true\" stretchy=\"true\">(</mo><mi>b</mi><mo fence=\"true\" stretchy=\"true\">)</mo></mrow>"));
        QCOMPARE(names(two).count(QStringLiteral("d")), 0);
        QVERIFY(wml::equationLine(two).contains(QLatin1String("+")));
        // A group in a group is one; a separator of an inner group does not cut the outer.
        const QByteArray nested = toOmml(mml("<mrow><mo fence=\"true\">(</mo><mi>a</mi><mo separator=\"true\">,</mo><mo fence=\"true\">[</mo><mi>b</mi><mo separator=\"true\">,</mo><mi>c</mi>"
                                             "<mo fence=\"true\">]</mo><mo fence=\"true\">)</mo></mrow>"));
        QCOMPARE(names(nested).count(QStringLiteral("e")), 2);
        QCOMPARE(names(nested).count(QStringLiteral("d")), 1);
    }

    void aPartInTheWrongNamespaceIsRefused()
    {
        bool ok = true;
        toOmml("<evil:math xmlns:evil=\"http://example.com/x\"><evil:mi>x</evil:mi></evil:math>", &ok);
        QVERIFY(!ok);
        toOmml("<math xmlns=\"http://example.com/x\"><mi>x</mi></math>", &ok);
        QVERIFY(!ok);
        toMathml("<m:oMath xmlns:m=\"http://example.com/x\"><m:r><m:t>x</m:t></m:r></m:oMath>", &ok);
        QVERIFY(!ok);
        toMathml("<oMath><r><t>x</t></r></oMath>", &ok);
        QVERIFY(!ok);
        // A formula part with no namespace written is read; an element of another namespace inside one in MathML's is kept as text, not mapped.
        QVERIFY(!toOmml("<math><mi>x</mi></math>", &ok).isEmpty());
        QVERIFY(ok);
        const QByteArray foreign = toOmml(mml("<mrow><x:mfrac xmlns:x=\"http://example.com/x\"><mi>a</mi><mi>b</mi></x:mfrac></mrow>"));
        QVERIFY(!names(foreign).contains(QStringLiteral("f")));
        // An attribute in another namespace is not the attribute.
        const QByteArray attr = toOmml(mml("<mi x:mathvariant=\"bold\" xmlns:x=\"http://example.com/x\">y</mi>"));
        QVERIFY(!attr.contains("<m:sty"));
    }

    // A part shaped as another program writes it: semantics, the StarMath annotation, nested groups, styles on mstyle, fences with stretchy, upright names, quoted strings, wide spaces.
    void aPartShapedLikeAnotherProgramsReads()
    {
        const QByteArray xml = "<math xmlns=\"http://www.w3.org/1998/Math/MathML\" display=\"block\"><semantics><mrow><mstyle mathvariant=\"normal\"><mrow><mi mathvariant=\"normal\">sin</mi><mo>&#x2061;</mo>"
                               "<mrow><mo fence=\"true\" stretchy=\"true\">(</mo><mrow><mi>x</mi><mspace width=\"0.278em\"/><mtext>if&#xA0;a  b</mtext></mrow><mo fence=\"true\" stretchy=\"true\">)</mo></mrow></mrow></mstyle>"
                               "<mspace width=\"0.1em\"/><mstyle fontweight=\"bold\"><mi>y</mi></mstyle><mstyle fontstyle=\"italic\"><mi>zz</mi></mstyle><ms>q</ms></mrow>"
                               "<annotation encoding=\"StarMath 5.0\">sin (x ~ \"if a b\") </annotation></semantics></math>";
        bool ok = false;
        const QByteArray out = toOmml(xml, &ok);
        QVERIFY(ok);
        QVERIFY(!out.contains("StarMath") && !out.contains("semantics"));
        const QStringList n = names(out);
        QVERIFY(n.contains(QStringLiteral("func")) && n.contains(QStringLiteral("d")));
        QVERIFY(out.contains("<m:t>sin</m:t>"));
        QVERIFY(out.contains("<m:sty m:val=\"p\"/>"));
        QVERIFY2(out.contains("if\xC2\xA0" "a  b"), "inner blanks and a no-break space are kept");
        QVERIFY(out.contains("<m:sty m:val=\"b\"/></m:rPr><m:t>y</m:t>"));
        QVERIFY(out.contains("<m:sty m:val=\"i\"/></m:rPr><m:t>zz</m:t>"));
        QVERIFY(out.contains("<m:t>&quot;q&quot;</m:t>") || out.contains("<m:t>\"q\"</m:t>"));
        // (a space under 0.2em is none; the one of 0.278em is a blank)
        QCOMPARE(out.count("<m:t xml:space=\"preserve\"> </m:t>"), 1);
    }

    void mstyleMspaceMsAndMencloseAreRead()
    {
        // mstyle: its attributes reach the tokens in it, and a token's own win.
        const QByteArray styled = toOmml(mml("<mstyle mathvariant=\"bold\"><mi>a</mi><mi mathvariant=\"italic\">b</mi></mstyle>"));
        QVERIFY(styled.contains("<m:sty m:val=\"b\"/></m:rPr><m:t>a</m:t>") && styled.contains("<m:sty m:val=\"i\"/></m:rPr><m:t>b</m:t>"));
        // mspace: 0.2em is a blank; narrower, and a width with no unit, are none; a wide one is several, never more than 32.
        QCOMPARE(toOmml(mml("<mi>a</mi><mspace width=\"0.2em\"/><mi>b</mi>")).count("preserve"), 1);
        QCOMPARE(toOmml(mml("<mi>a</mi><mspace width=\"0.19em\"/><mi>b</mi>")).count("preserve"), 0);
        QCOMPARE(toOmml(mml("<mi>a</mi><mspace width=\"3\"/><mi>b</mi>")).count("preserve"), 0);
        QVERIFY(toOmml(mml("<mspace width=\"1000000em\"/>")).contains("<m:t xml:space=\"preserve\">" + QByteArray(32, ' ') + "</m:t>"));
        // ms keeps its quotes, and its own if it names them.
        QVERIFY(toOmml(mml("<ms lquote=\"'\" rquote=\"'\">s</ms>")).contains("<m:t>'s'</m:t>"));
        // menclose: a box is the markup's border box, and comes back as one; another kind is looked through.
        const QByteArray box = toOmml(mml("<menclose notation=\"box\"><mi>x</mi></menclose>"));
        QVERIFY(names(box).contains(QStringLiteral("borderBox")));
        QVERIFY(toMathml(box).contains("<menclose notation=\"box\">"));
        QVERIFY(!names(toOmml(mml("<menclose notation=\"top\"><mi>x</mi></menclose>"))).contains(QStringLiteral("borderBox")));
    }

    // A limit written as a function (the name, the sign for applying it, its operand) is the markup's function with a limit as its name, both ways.
    void aLimitAsAFunctionNameIsAFunction()
    {
        const QByteArray fn = toOmml(mml("<mrow><munder><mi>lim</mi><mrow><mi>n</mi><mo>&#x2192;</mo><mi>k</mi></mrow></munder><mo>&#x2061;</mo><mi>f</mi></mrow>"));
        QVERIFY(names(fn).indexOf(QStringLiteral("func")) >= 0);
        QVERIFY(names(fn).indexOf(QStringLiteral("limLow")) > names(fn).indexOf(QStringLiteral("fName")));
        const QByteArray back = toMathml(fn);
        QVERIFY(back.contains("<munder>") && back.contains("\xE2\x81\xA1"));
        QCOMPARE(toOmml(back), fn);
    }

    // What the first version lost on the way: the equation array, group character, border box, phantom, an empty separator, an empty operator symbol.
    void shapesThatLostTheirKindKeepIt()
    {
        const QByteArray arr = omml("<m:eqArr><m:eqArrPr/><m:e>" + run("a") + "</m:e><m:e>" + run("b") + "</m:e></m:eqArr>");
        QVERIFY(toMathml(arr).contains("class=\"eqArr\""));
        QCOMPARE(names(toOmml(toMathml(arr))).count(QStringLiteral("e")), 2);
        QVERIFY(names(toOmml(toMathml(arr))).contains(QStringLiteral("eqArr")) && !names(toOmml(toMathml(arr))).contains(QStringLiteral("mr")));

        const QByteArray group = omml("<m:groupChr><m:groupChrPr><m:chr m:val=\"~\"/><m:pos m:val=\"top\"/></m:groupChrPr><m:e>" + run("x") + "</m:e></m:groupChr>");
        const QByteArray gm = toMathml(group);
        QVERIFY(gm.contains("<mover class=\"groupChr\">"));
        const QByteArray gb = toOmml(gm);
        QVERIFY(names(gb).contains(QStringLiteral("groupChr")) && gb.contains("<m:chr m:val=\"~\"/>") && gb.contains("<m:pos m:val=\"top\"/>"));

        const QByteArray box = omml("<m:borderBox><m:borderBoxPr/><m:e>" + run("x") + "</m:e></m:borderBox>");
        QVERIFY(toMathml(box).contains("<menclose notation=\"box\">"));
        QVERIFY(names(toOmml(toMathml(box))).contains(QStringLiteral("borderBox")));

        const QByteArray hidden = omml("<m:phant><m:phantPr><m:show m:val=\"0\"/><m:zeroWid m:val=\"1\"/></m:phantPr><m:e>" + run("x") + "</m:e></m:phant>");
        const QByteArray hm = toMathml(hidden);
        QVERIFY(hm.contains("<mphantom>") && hm.contains("width=\"0\""));
        const QByteArray hb = toOmml(hm);
        QVERIFY(hb.contains("<m:show m:val=\"0\"/>") && hb.contains("<m:zeroWid m:val=\"1\"/>") && !hb.contains("zeroAsc"));
        const QByteArray shown = omml("<m:phant><m:phantPr><m:zeroAsc m:val=\"1\"/></m:phantPr><m:e>" + run("x") + "</m:e></m:phant>");
        const QByteArray sb = toOmml(toMathml(shown));
        QVERIFY(sb.contains("<m:zeroAsc m:val=\"1\"/>") && !sb.contains("<m:show"));
        QVERIFY(names(toOmml(mml("<mphantom><mi>x</mi></mphantom>"))).contains(QStringLiteral("phant")));

        const QByteArray list = omml("<m:d><m:dPr><m:begChr m:val=\"(\"/><m:sepChr m:val=\"\"/><m:endChr m:val=\")\"/></m:dPr><m:e>" + run("a") + "</m:e><m:e>" + run("b") + "</m:e></m:d>");
        const QByteArray lb = toOmml(toMathml(list));
        QCOMPARE(names(lb).count(QStringLiteral("e")), 2);
        QVERIFY(lb.contains("<m:sepChr m:val=\"\"/>"));

        const QByteArray emptyChr = omml("<m:nary><m:naryPr><m:chr m:val=\"\"/></m:naryPr><m:sub/><m:sup/><m:e>" + run("f") + "</m:e></m:nary>");
        QVERIFY(toMathml(emptyChr).contains("\xE2\x88\xAB"));
    }

    void notXmlOrNotMathIsRefused()
    {
        bool ok = true;
        toOmml("<math><mi>x</mrow></math>", &ok);
        QVERIFY(!ok);
        toOmml("<html><mi>x</mi></html>", &ok);
        QVERIFY(!ok);
        toOmml(QByteArray(), &ok);
        QVERIFY(!ok);
        toMathml("<m:oMath", &ok);
        QVERIFY(!ok);
        toOmml(mml("<mi>x</mi>"), &ok);
        QVERIFY(ok);
    }
};

JP_TEST_CLASS(PubMathmlTests)
#include "tst_pub_mathml.moc"
