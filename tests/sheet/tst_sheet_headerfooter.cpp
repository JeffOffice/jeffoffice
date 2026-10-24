// Tests of the header and footer codes: the three parts, the fields, the fonts and styles, the ampersand, and the bounds on a string that a file controls.
#include "sheethelpers.h"

#include "sheet/headerfooter.h"

namespace {

QString textOf(const HfLine &line)
{
    QString t;
    for (const HfRun &r : line) t += r.field == HfField::None ? r.text : QStringLiteral("<%1>").arg(int(r.field));
    return t;
}

} // namespace

class SheetHeaderFooterTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The three parts, by &L, &C, and &R; text before any part code is the center's.
    void theThreePartsAreSplit()
    {
        const HfText t = parseHeaderFooter(QStringLiteral("&LLeft text&CCenter&RRight text"));
        QCOMPARE(t.left.lines.size(), 1);
        QCOMPARE(textOf(t.left.lines[0]), QStringLiteral("Left text"));
        QCOMPARE(textOf(t.center.lines[0]), QStringLiteral("Center"));
        QCOMPARE(textOf(t.right.lines[0]), QStringLiteral("Right text"));
        const HfText plain = parseHeaderFooter(QStringLiteral("Just this"));
        QVERIFY(plain.left.isEmpty() && plain.right.isEmpty());
        QCOMPARE(textOf(plain.center.lines[0]), QStringLiteral("Just this"));
        QVERIFY(parseHeaderFooter(QString()).isEmpty());
        // a part named twice goes on after what it had
        const HfText twice = parseHeaderFooter(QStringLiteral("&Lone&Rtwo&Lthree"));
        QCOMPARE(textOf(twice.left.lines[0]), QStringLiteral("onethree"));
    }

    // The fields are runs of their own, between the text.
    void theFieldsAreRuns()
    {
        const HfText t = parseHeaderFooter(QStringLiteral("&CPage &P of &N on &D at &T: &F in &Z, sheet &A &G"));
        const HfLine &line = t.center.lines[0];
        QCOMPARE(textOf(line), QStringLiteral("Page <1> of <2> on <3> at <4>: <5> in <6>, sheet <7> <8>"));
        QVERIFY(t.hasPicture);
        QVERIFY(line[1].field == HfField::Page);
        QVERIFY(line[3].field == HfField::Pages);
    }

    // && is an ampersand, and a code with no meaning, or an & at the end, is passed over.
    void anAmpersandAndCodesWithNoMeaning()
    {
        QCOMPARE(textOf(parseHeaderFooter(QStringLiteral("&CR&&D & more")).center.lines[0]), QStringLiteral("R&D  more"));
        QCOMPARE(textOf(parseHeaderFooter(QStringLiteral("&Ca&qb&")).center.lines[0]), QStringLiteral("ab"));
        QCOMPARE(escapeHeaderFooterText(QStringLiteral("Q&A & you")), QStringLiteral("Q&&A && you"));
    }

    // Fonts, sizes, styles, and colors apply to the text after the code; the toggles switch back; each part begins plain.
    void fontsAndStylesApplyToWhatFollows()
    {
        const HfText t = parseHeaderFooter(QStringLiteral("&L&\"Arial,Bold Italic\"&14Big&\"-,Regular\" plain&B bold&B off&U under&E double&S strike&X up&Y down&KFF0000 red&C center"));
        const HfLine &l = t.left.lines[0];
        QCOMPARE(l[0].text, QStringLiteral("Big"));
        QCOMPARE(l[0].font, QStringLiteral("Arial"));
        QVERIFY(l[0].bold && l[0].italic);
        QCOMPARE(l[0].size, 14.0);
        QCOMPARE(l[1].text, QStringLiteral(" plain"));
        QVERIFY(l[1].font.isEmpty());
        QVERIFY(!l[1].bold && !l[1].italic);
        QCOMPARE(l[1].size, 14.0);   // (the size stays until another)
        QCOMPARE(l[2].text, QStringLiteral(" bold"));
        QVERIFY(l[2].bold);
        QCOMPARE(l[3].text, QStringLiteral(" off"));
        QVERIFY(!l[3].bold);
        QVERIFY(l[4].underline == HfUnderline::Single);
        QVERIFY(l[5].underline == HfUnderline::Double);
        QVERIFY(l[6].strike);
        QVERIFY(l[7].superscript);
        QVERIFY(l[8].subscript);
        QVERIFY(l[9].hasColor);
        QCOMPARE(l[9].color, QColor(255, 0, 0));
        // the center begins with none of it
        const HfRun &c = t.center.lines[0][0];
        QVERIFY(!c.bold && !c.strike && !c.hasColor && c.size == 0 && c.font.isEmpty());
    }

    // A line break ends a line; a theme color is not read as a color; a size can be followed by digits only if a code comes between.
    void linesAndColors()
    {
        const HfText t = parseHeaderFooter(QStringLiteral("&CLine one\nLine two\r\nLine three&K01+034 x"));
        QCOMPARE(t.center.lines.size(), 3);
        QCOMPARE(textOf(t.center.lines[1]), QStringLiteral("Line two"));
        QVERIFY(!t.center.lines[2][0].hasColor);
        QCOMPARE(textOf(t.center.lines[2]), QStringLiteral("Line three x"));
    }

    // Bounds: the string, the codes, the runs, and the lines.
    void aStringThatIsTooLargeIsBounded()
    {
        QString codes;
        for (int i = 0; i < 5000; ++i) codes += QStringLiteral("&Bx");
        const HfText many = parseHeaderFooter(QStringLiteral("&C") + codes);
        QVERIFY(many.center.lines[0].size() <= kMaxHeaderFooterRuns);
        QString lines(200, QLatin1Char('\n'));
        const HfText manyLines = parseHeaderFooter(QStringLiteral("&Ca") + lines + QStringLiteral("b"));
        QVERIFY(manyLines.center.lines.size() <= kMaxHeaderFooterLines);
        const HfText longText = parseHeaderFooter(QStringLiteral("&C") + QString(kMaxHeaderFooterChars * 4, QLatin1Char('x')));
        qsizetype chars = 0;
        for (const HfRun &r : longText.center.lines[0]) chars += r.text.size();
        QVERIFY(chars <= kMaxHeaderFooterChars);
        // a size is held to what a font can be
        const HfText size = parseHeaderFooter(QStringLiteral("&C&99999x&0y"));
        QVERIFY(size.center.lines[0][0].size <= 409);
    }

    // The fields as a page shows them.
    void fieldsAreWorkedOutForThePage()
    {
        HfContext c;
        c.page = 3;
        c.pages = 12;
        c.now = QDateTime(QDate(2026, 10, 24), QTime(14, 5, 0));
        c.file = QStringLiteral("Budget.xlsx");
        c.path = QStringLiteral("/home/me/Documents/");
        c.sheet = QStringLiteral("Q3");
        c.locale = QLocale(QLocale::English, QLocale::UnitedStates);
        QCOMPARE(hfFieldText(HfField::Page, c), QStringLiteral("3"));
        QCOMPARE(hfFieldText(HfField::Pages, c), QStringLiteral("12"));
        QCOMPARE(hfFieldText(HfField::Date, c), c.locale.toString(c.now.date(), QLocale::ShortFormat));
        QCOMPARE(hfFieldText(HfField::Time, c), c.locale.toString(c.now.time(), QLocale::ShortFormat));
        QCOMPARE(hfFieldText(HfField::File, c), QStringLiteral("Budget.xlsx"));
        QCOMPARE(hfFieldText(HfField::Path, c), QStringLiteral("/home/me/Documents/"));
        QCOMPARE(hfFieldText(HfField::Sheet, c), QStringLiteral("Q3"));
        QVERIFY(hfFieldText(HfField::Picture, c).isEmpty());
        QCOMPARE(headerFooterPreview(QStringLiteral("&LSheet &A&CPage &P of &N"), c), QStringLiteral("Sheet Q3 | Page 3 of 12"));
    }

    // The editor's three boxes make the string the file keeps, and take it apart again.
    void thePartsAreComposedAndSplit()
    {
        QCOMPARE(composeHeaderFooter(QStringLiteral("a"), QString(), QStringLiteral("&P")), QStringLiteral("&La&R&P"));
        QCOMPARE(composeHeaderFooter(QString(), QString(), QString()), QString());
        QString l, c, r;
        splitHeaderFooter(QStringLiteral("&La&&b&C&B&D&Rend"), &l, &c, &r);
        QCOMPARE(l, QStringLiteral("a&&b"));
        QCOMPARE(c, QStringLiteral("&B&D"));
        QCOMPARE(r, QStringLiteral("end"));
        splitHeaderFooter(QStringLiteral("only center"), &l, &c, &r);
        QVERIFY(l.isEmpty() && r.isEmpty());
        QCOMPARE(c, QStringLiteral("only center"));
        // composing what was split gives the same parts
        splitHeaderFooter(composeHeaderFooter(QStringLiteral("x"), QStringLiteral("y"), QStringLiteral("z")), &l, &c, &r);
        QCOMPARE(l + c + r, QStringLiteral("xyz"));
    }
};

JP_TEST_CLASS(SheetHeaderFooterTests)
#include "tst_sheet_headerfooter.moc"
