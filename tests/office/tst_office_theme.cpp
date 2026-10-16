// Tests of libs/office: the theme and DrawingML colors.
#include "officehelpers.h"

namespace {

// Whether each channel is within `tolerance` of the color written as hex.
bool nearTo(const QColor &c, const QString &hex, int tolerance)
{
    const QColor want(QLatin1Char('#') + hex);
    return c.isValid() && qAbs(c.red() - want.red()) <= tolerance && qAbs(c.green() - want.green()) <= tolerance
        && qAbs(c.blue() - want.blue()) <= tolerance;
}

QString srgbClr(const QString &hex, const QStringList &list = {})
{
    return QStringLiteral("<a:srgbClr val=\"%1\">%2</a:srgbClr>").arg(hex, transforms(list));
}

} // namespace

class OfficeThemeTests : public QObject {
    Q_OBJECT


private Q_SLOTS:
    // ---- the theme

    void themeColorNames()
    {
        const QStringList names = {"dk1", "lt1", "dk2", "lt2", "accent1", "accent2", "accent3", "accent4", "accent5", "accent6", "hlink", "folHlink"};
        QCOMPARE(names.size(), int(Theme::ColorCount));
        for (int i = 0; i < Theme::ColorCount; ++i) {
            QCOMPARE(themeColorName(Theme::Color(i)), names[i]);
            Theme::Color back = Theme::ColorCount;
            QVERIFY(themeColorFromName(names[i], &back));
            QCOMPARE(int(back), i);
        }
        Theme::Color slot = Theme::Accent3;
        for (const char *bad : {"", "bg1", "tx1", "accent7", "Accent1", "phClr"}) {
            QVERIFY2(!themeColorFromName(QString::fromLatin1(bad), &slot), bad);
            QCOMPARE(int(slot), int(Theme::Accent3));   // untouched
        }
    }

    void themeReadsColorsAndFonts()
    {
        Theme t;
        QString err;
        const QByteArray xml = handWrittenTheme();
        QVERIFY2(readTheme(xml, &t, &err), qPrintable(err));
        QCOMPARE(t.name, QStringLiteral("Test Theme"));
        QCOMPARE(t.colorSchemeName, QStringLiteral("Test Colors"));
        QCOMPARE(t.fontSchemeName, QStringLiteral("Test Fonts"));
        for (int i = 0; i < Theme::ColorCount; ++i) {
            QVERIFY2(t.colors[i].isValid(), qPrintable(themeColorName(Theme::Color(i))));
            QCOMPARE(t.colors[i].name(QColor::HexRgb), kHandWrittenColors[i].name(QColor::HexRgb));   // the sysClr two by their lastClr
            QCOMPARE(t.colors[i].alpha(), 255);
        }
        QCOMPARE(t.majorLatin, QStringLiteral("Georgia"));
        QCOMPARE(t.majorEastAsian, QStringLiteral("Noto Serif CJK JP"));
        QCOMPARE(t.majorComplex, QStringLiteral("Arial"));
        QCOMPARE(t.minorLatin, QStringLiteral("Verdana"));
        QCOMPARE(t.minorEastAsian, QString());
        QCOMPARE(t.minorComplex, QStringLiteral("Tahoma"));
        QCOMPARE(t.xml, xml);
    }

    void themeReadsStrictNamespace()
    {
        Theme t;
        QString err;
        QVERIFY2(readTheme(handWrittenTheme(QString(ns::AStrict)), &t, &err), qPrintable(err));
        QCOMPARE(t.name, QStringLiteral("Test Theme"));
        QCOMPARE(t.colors[Theme::Accent4].name(QColor::HexRgb), QStringLiteral("#ffc000"));
        QCOMPARE(t.minorLatin, QStringLiteral("Verdana"));
    }

    void themeFillsInWhatIsMissing()
    {
        // Only some colors, no fonts: the colors it lacks are the default's.
        const QByteArray xml = QStringLiteral(
            "<a:theme xmlns:a=\"%1\" name=\"Small\"><a:themeElements><a:clrScheme name=\"S\">"
            "<a:accent1><a:srgbClr val=\"123456\"/></a:accent1>"
            "<a:accent2><a:srgbClr val=\"nonsense\"/></a:accent2>"
            "<a:hlink/>"
            "</a:clrScheme></a:themeElements></a:theme>").arg(ns::A).toUtf8();
        Theme t;
        QString err;
        QVERIFY2(readTheme(xml, &t, &err), qPrintable(err));
        const Theme d = defaultTheme();
        QCOMPARE(t.name, QStringLiteral("Small"));
        QCOMPARE(t.colors[Theme::Accent1].name(QColor::HexRgb), QStringLiteral("#123456"));
        QCOMPARE(t.colors[Theme::Accent2], d.colors[Theme::Accent2]);   // unreadable
        QCOMPARE(t.colors[Theme::Hyperlink], d.colors[Theme::Hyperlink]);   // empty
        QCOMPARE(t.colors[Theme::Dark1], d.colors[Theme::Dark1]);   // absent
        QCOMPARE(t.fontSchemeName, QString());
        QCOMPARE(t.majorLatin, QString());
        QCOMPARE(t.minorLatin, QString());
    }

    void themeUsesSystemColorNamesWithoutLastClr()
    {
        const QByteArray xml = QStringLiteral(
            "<a:theme xmlns:a=\"%1\"><a:themeElements><a:clrScheme name=\"S\">"
            "<a:dk1><a:sysClr val=\"windowText\"/></a:dk1><a:lt1><a:sysClr val=\"window\"/></a:lt1>"
            "</a:clrScheme></a:themeElements></a:theme>").arg(ns::A).toUtf8();
        Theme t;
        QVERIFY(readTheme(xml, &t));
        QCOMPARE(t.colors[Theme::Dark1].name(QColor::HexRgb), QStringLiteral("#000000"));
        QCOMPARE(t.colors[Theme::Light1].name(QColor::HexRgb), QStringLiteral("#ffffff"));
    }

    void themeRejectsWhatIsNotATheme_data()
    {
        QTest::addColumn<QByteArray>("xml");
        QTest::newRow("empty") << QByteArray();
        QTest::newRow("not XML") << QByteArray("a theme, in words");
        QTest::newRow("another root") << QStringLiteral("<a:other xmlns:a=\"%1\"/>").arg(ns::A).toUtf8();
        QTest::newRow("a theme in another namespace") << QByteArray("<a:theme xmlns:a=\"urn:other\"><a:themeElements/></a:theme>");
        QTest::newRow("no theme elements") << QStringLiteral("<a:theme xmlns:a=\"%1\"/>").arg(ns::A).toUtf8();
        QTest::newRow("no color scheme") << QStringLiteral("<a:theme xmlns:a=\"%1\"><a:themeElements><a:fontScheme name=\"f\"/></a:themeElements></a:theme>").arg(ns::A).toUtf8();
        QTest::newRow("DOCTYPE") << QByteArray("<!DOCTYPE a:theme [<!ENTITY e \"x\">]>") + handWrittenTheme();
        QByteArray inner = handWrittenTheme();
        inner.insert(inner.indexOf("?>") + 2, "\n<!DOCTYPE a:theme [<!ENTITY e \"x\">]>");
        QTest::newRow("DOCTYPE after the declaration") << inner;
    }

    void themeRejectsWhatIsNotATheme()
    {
        QFETCH(QByteArray, xml);
        Theme t = defaultTheme();
        QString err;
        QVERIFY(!readTheme(xml, &t, &err));
        QVERIFY(!err.isEmpty());
        QCOMPARE(t.name, defaultTheme().name);   // left as it was
    }

    void themeWritesWhatItRead()
    {
        // A part read is written back byte for byte, whatever it holds that
        // the struct doesn't.
        QByteArray xml = handWrittenTheme();
        Theme t;
        QVERIFY(readTheme(xml, &t));
        QCOMPARE(writeTheme(t), xml);
        // Until the xml is cleared, a changed field is not written.
        t.colors[Theme::Accent1] = QColor(Qt::red);
        QCOMPARE(writeTheme(t), xml);
        t.xml.clear();
        Theme back;
        QVERIFY(readTheme(writeTheme(t), &back));
        QCOMPARE(back.colors[Theme::Accent1].name(QColor::HexRgb), QStringLiteral("#ff0000"));
    }

    void themeWritesFromFields()
    {
        Theme t;
        t.name = QStringLiteral("Fine & \"Dandy\" <1>");
        t.colorSchemeName = QStringLiteral("Colors > Others");
        t.fontSchemeName = QStringLiteral("Fonts 'n' Things");
        for (int i = 0; i < Theme::ColorCount; ++i) t.colors[i] = kHandWrittenColors[i];
        t.majorLatin = QStringLiteral("Georgia");
        t.minorLatin = QStringLiteral("Verdana");
        t.majorEastAsian = QStringLiteral("Noto Serif CJK JP");
        t.minorEastAsian = QString();
        t.majorComplex = QStringLiteral("Arial & Co");
        t.minorComplex = QStringLiteral("Tahoma");
        const QByteArray xml = writeTheme(t);
        QVERIFY(!xml.isEmpty());

        Theme back;
        QString err;
        QVERIFY2(readTheme(xml, &back, &err), qPrintable(err));
        QCOMPARE(back.name, t.name);
        QCOMPARE(back.colorSchemeName, t.colorSchemeName);
        QCOMPARE(back.fontSchemeName, t.fontSchemeName);
        for (int i = 0; i < Theme::ColorCount; ++i) QCOMPARE(back.colors[i].name(QColor::HexRgb), t.colors[i].name(QColor::HexRgb));
        QCOMPARE(back.majorLatin, t.majorLatin);
        QCOMPARE(back.minorLatin, t.minorLatin);
        QCOMPARE(back.majorEastAsian, t.majorEastAsian);
        QCOMPARE(back.minorEastAsian, t.minorEastAsian);
        QCOMPARE(back.majorComplex, t.majorComplex);
        QCOMPARE(back.minorComplex, t.minorComplex);
        QCOMPARE(back.xml, xml);
    }

    void themeWrittenHasACompleteStructure()
    {
        // The parts the schema requires: the three schemes in order, and in
        // the format scheme three of each style.
        QDomDocument doc;
        QString err;
        QVERIFY2(parseXml(writeTheme(defaultTheme()), &doc, &err), qPrintable(err));
        const QDomElement root = doc.documentElement();
        QVERIFY(isElement(root, ns::A, QStringLiteral("theme")));
        const QDomElement elements = child(root, ns::A, QStringLiteral("themeElements"));
        QVERIFY(!elements.isNull());
        const QVector<QDomElement> schemes = jp::office::children(elements);
        QCOMPARE(schemes.size(), 3);
        QVERIFY(isElement(schemes[0], ns::A, QStringLiteral("clrScheme")));
        QVERIFY(isElement(schemes[1], ns::A, QStringLiteral("fontScheme")));
        QVERIFY(isElement(schemes[2], ns::A, QStringLiteral("fmtScheme")));
        // Twelve colors, in the schema's order, each a plain RGB.
        const QStringList slots = {"dk1", "lt1", "dk2", "lt2", "accent1", "accent2", "accent3", "accent4", "accent5", "accent6", "hlink", "folHlink"};
        const QVector<QDomElement> colors = jp::office::children(schemes[0]);
        QCOMPARE(colors.size(), 12);
        for (int i = 0; i < 12; ++i) {
            QVERIFY(isElement(colors[i], ns::A, slots[i]));
            QVERIFY(!child(colors[i], ns::A, QStringLiteral("srgbClr")).isNull());
        }
        for (const QString &font : {QStringLiteral("majorFont"), QStringLiteral("minorFont")}) {
            const QDomElement f = child(schemes[1], ns::A, font);
            QVERIFY(!f.isNull());
            // latin, ea, and cs are all required, even when empty.
            QVERIFY(!child(f, ns::A, QStringLiteral("latin")).isNull());
            QVERIFY(!child(f, ns::A, QStringLiteral("ea")).isNull());
            QVERIFY(!child(f, ns::A, QStringLiteral("cs")).isNull());
        }
        const QVector<QDomElement> styles = jp::office::children(schemes[2]);
        QCOMPARE(styles.size(), 4);
        const QStringList lists = {"fillStyleLst", "lnStyleLst", "effectStyleLst", "bgFillStyleLst"};
        for (int i = 0; i < 4; ++i) {
            QVERIFY(isElement(styles[i], ns::A, lists[i]));
            QCOMPARE(jp::office::children(styles[i]).size(), 3);
        }
        QVERIFY(!child(root, ns::A, QStringLiteral("objectDefaults")).isNull());
        QVERIFY(!child(root, ns::A, QStringLiteral("extraClrSchemeLst")).isNull());
    }

    void themeWritesUnsetColorsAsDefault()
    {
        Theme t;
        t.name = QStringLiteral("Bare");
        Theme back;
        QVERIFY(readTheme(writeTheme(t), &back));
        const Theme d = defaultTheme();
        for (int i = 0; i < Theme::ColorCount; ++i) QCOMPARE(back.colors[i].name(QColor::HexRgb), d.colors[i].name(QColor::HexRgb));
    }

    void themeDefault()
    {
        const Theme d = defaultTheme();
        QCOMPARE(d.name, QStringLiteral("JeffOffice"));
        QCOMPARE(d.colorSchemeName, QStringLiteral("JeffOffice"));
        QCOMPARE(d.fontSchemeName, QStringLiteral("JeffOffice"));
        QCOMPARE(d.majorLatin, QStringLiteral("Carlito"));
        QCOMPARE(d.minorLatin, QStringLiteral("Carlito"));
        QVERIFY(d.xml.isEmpty());
        const char *expected[Theme::ColorCount] = {"#1f2937", "#ffffff", "#24364f", "#e8ecf2", "#1d55c4", "#b6420f",
                                                   "#13773a", "#9e1f63", "#0a6c8c", "#5e2ca5", "#1d55c4", "#5e2ca5"};
        for (int i = 0; i < Theme::ColorCount; ++i) {
            QCOMPARE(d.colors[i].name(QColor::HexRgb), QString::fromLatin1(expected[i]));
            QCOMPARE(d.colors[i].alpha(), 255);
        }
        // It makes a part and reads it back as it was.
        Theme back;
        QString err;
        QVERIFY2(readTheme(writeTheme(d), &back, &err), qPrintable(err));
        QCOMPARE(back.name, d.name);
        QCOMPARE(back.majorLatin, d.majorLatin);
        for (int i = 0; i < Theme::ColorCount; ++i) QCOMPARE(back.colors[i], d.colors[i]);
    }

    // ---- DrawingML colors

    void colorReadsRgb()
    {
        QCOMPARE(hexOf(colorOf(srgbClr(QStringLiteral("FF8000")))), QStringLiteral("FF8000"));
        QCOMPARE(hexOf(colorOf(srgbClr(QStringLiteral("0a1b2c")))), QStringLiteral("0A1B2C"));
        QCOMPARE(colorOf(srgbClr(QStringLiteral("FF8000"))).alpha(), 255);
        for (const char *bad : {"", "GG0000", "FF80", "#FF8000", "FF800000", "red", "0x1234", " FFFFF", "+FFFFF", "FFFFF ", "-FFFFF", "FF 000"})
            QVERIFY2(!colorOf(srgbClr(QString::fromLatin1(bad))).isValid(), bad);
    }

    void colorNeedsAColorChoice()
    {
        QVERIFY(!colorOf(QString()).isValid());
        QVERIFY(!colorOf(QStringLiteral("<a:noFill/>")).isValid());
        QVERIFY(!colorOf(QStringLiteral("<a:srgbClr/>")).isValid());
        QVERIFY(!readColor(QDomElement(), colorTestTheme(), ColorMap()).isValid());
        // The first color choice is the color, whatever else is there.
        QCOMPARE(hexOf(colorOf(QStringLiteral("<a:extLst/><a:srgbClr val=\"FF0000\"/><a:srgbClr val=\"00FF00\"/>"))), QStringLiteral("FF0000"));
        // A color in some other namespace is none.
        QVERIFY(!colorOf(QStringLiteral("<x:srgbClr xmlns:x=\"urn:other\" val=\"FF0000\"/>")).isValid());
    }

    void colorReadsStrictNamespace()
    {
        QCOMPARE(hexOf(colorOf(srgbClr(QStringLiteral("FF0000")), QColor(), ColorMap(), colorTestTheme(), QString(ns::AStrict))), QStringLiteral("FF0000"));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("accent1"), {"lumMod 75000"}), QColor(), ColorMap(), colorTestTheme(), QString(ns::AStrict))),
                 QStringLiteral("2F5597"));
    }

    void colorReadsSchemeColors()
    {
        const struct { const char *name; Theme::Color slot; } names[] = {
            {"dk1", Theme::Dark1}, {"lt1", Theme::Light1}, {"dk2", Theme::Dark2}, {"lt2", Theme::Light2},
            {"accent1", Theme::Accent1}, {"accent2", Theme::Accent2}, {"accent3", Theme::Accent3}, {"accent4", Theme::Accent4},
            {"accent5", Theme::Accent5}, {"accent6", Theme::Accent6}, {"hlink", Theme::Hyperlink}, {"folHlink", Theme::FollowedHyperlink},
            // Through the usual map.
            {"bg1", Theme::Light1}, {"tx1", Theme::Dark1}, {"bg2", Theme::Light2}, {"tx2", Theme::Dark2}};
        for (const auto &n : names)
            QCOMPARE(hexOf(colorOf(schemeClr(QString::fromLatin1(n.name)))), hexOf(kHandWrittenColors[n.slot]));
        for (const char *bad : {"", "foo", "Accent1", "accent7", "phClr", "window"})
            QVERIFY2(!colorOf(schemeClr(QString::fromLatin1(bad))).isValid(), bad);
        // A theme with a color unset gives none.
        QVERIFY(!colorOf(schemeClr(QStringLiteral("accent1")), QColor(), ColorMap(), Theme()).isValid());
    }

    void colorMapChoosesThemeColors()
    {
        const ColorMap usual;
        Theme::Color slot = Theme::ColorCount;
        QVERIFY(usual.resolve(QStringLiteral("bg1"), &slot));
        QCOMPARE(int(slot), int(Theme::Light1));
        QVERIFY(usual.resolve(QStringLiteral("tx2"), &slot));
        QCOMPARE(int(slot), int(Theme::Dark2));
        QVERIFY(usual.resolve(QStringLiteral("accent5"), &slot));
        QCOMPARE(int(slot), int(Theme::Accent5));
        QVERIFY(usual.resolve(QStringLiteral("dk2"), &slot));
        QCOMPARE(int(slot), int(Theme::Dark2));
        QVERIFY(!usual.resolve(QStringLiteral("phClr"), &slot));
        QVERIFY(!usual.resolve(QStringLiteral("nonsense"), &slot));

        // A dark slide: its background is dark 1, its text light 1, and two accents swap.
        QDomDocument doc;
        QVERIFY(parseXml(QStringLiteral("<p:clrMap xmlns:p=\"%1\" bg1=\"dk1\" tx1=\"lt1\" bg2=\"dk2\" tx2=\"lt2\" accent1=\"accent2\" accent2=\"accent1\" "
                                        "accent3=\"accent3\" accent4=\"accent4\" accent5=\"accent5\" accent6=\"accent6\" hlink=\"hlink\" folHlink=\"folHlink\"/>")
                             .arg(ns::P).toUtf8(), &doc));
        const ColorMap dark = readColorMap(doc.documentElement());
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("bg1")), QColor(), dark)), hexOf(kHandWrittenColors[Theme::Dark1]));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("tx1")), QColor(), dark)), hexOf(kHandWrittenColors[Theme::Light1]));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("bg2")), QColor(), dark)), hexOf(kHandWrittenColors[Theme::Dark2]));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("tx2")), QColor(), dark)), hexOf(kHandWrittenColors[Theme::Light2]));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("accent1")), QColor(), dark)), hexOf(kHandWrittenColors[Theme::Accent2]));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("accent2")), QColor(), dark)), hexOf(kHandWrittenColors[Theme::Accent1]));
        // The theme's own names don't go through the map.
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("dk1")), QColor(), dark)), hexOf(kHandWrittenColors[Theme::Dark1]));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("accent3")), QColor(), dark)), hexOf(kHandWrittenColors[Theme::Accent3]));
    }

    void colorMapDefaultsWhatIsMissing()
    {
        QDomDocument doc;
        QVERIFY(parseXml(QStringLiteral("<a:overrideClrMapping xmlns:a=\"%1\" bg1=\"dk1\" tx1=\"nonsense\" bg2=\"bg1\" accent4=\"accent6\"/>")
                             .arg(ns::A).toUtf8(), &doc));
        const ColorMap m = readColorMap(doc.documentElement());
        QCOMPARE(int(m.bg1), int(Theme::Dark1));
        QCOMPARE(int(m.tx1), int(Theme::Dark1));        // not a theme color: the usual
        QCOMPARE(int(m.bg2), int(Theme::Light2));       // "bg1" is not one either
        QCOMPARE(int(m.tx2), int(Theme::Dark2));        // absent
        QCOMPARE(int(m.accent[3]), int(Theme::Accent6));
        QCOMPARE(int(m.accent[0]), int(Theme::Accent1));
        QCOMPARE(int(m.hlink), int(Theme::Hyperlink));
        QCOMPARE(int(m.folHlink), int(Theme::FollowedHyperlink));
        // No element: the usual map.
        const ColorMap none = readColorMap(QDomElement());
        QCOMPARE(int(none.bg1), int(Theme::Light1));
        QCOMPARE(int(none.tx1), int(Theme::Dark1));
        QCOMPARE(int(none.accent[5]), int(Theme::Accent6));
    }

    void colorUsesThePlaceholderColor()
    {
        const QColor red(255, 0, 0);
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("phClr")), red)), QStringLiteral("FF0000"));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("phClr"), {"lumMod 40000"}), red)), QStringLiteral("660000"));
        QVERIFY(!colorOf(schemeClr(QStringLiteral("phClr"))).isValid());
        // It is for "phClr" only.
        QCOMPARE(hexOf(colorOf(srgbClr(QStringLiteral("00FF00")), red)), QStringLiteral("00FF00"));
        QCOMPARE(hexOf(colorOf(schemeClr(QStringLiteral("accent1")), red)), hexOf(kHandWrittenColors[Theme::Accent1]));
        // The placeholder's own alpha goes along, and alpha can be set over it.
        const QColor half(255, 0, 0, 128);
        QCOMPARE(colorOf(schemeClr(QStringLiteral("phClr")), half).alpha(), 128);
        QCOMPARE(colorOf(schemeClr(QStringLiteral("phClr"), {"lumMod 40000"}), half).alpha(), 128);
        QCOMPARE(colorOf(schemeClr(QStringLiteral("phClr"), {"alpha 100000"}), half).alpha(), 255);
    }

    void colorAppliesTransforms_data()
    {
        QTest::addColumn<QString>("xml");
        QTest::addColumn<QString>("hex");
        QTest::addColumn<int>("tolerance");
        const QString accent1 = QStringLiteral("accent1");   // 4472C4
        // Luminance, saturation, and hue, in HSL.
        QTest::newRow("lumMod 60 lumOff 40") << schemeClr(accent1, {"lumMod 60000", "lumOff 40000"}) << "8FAADC" << 0;
        QTest::newRow("lumMod 75") << schemeClr(accent1, {"lumMod 75000"}) << "2F5597" << 0;
        QTest::newRow("lumMod 50") << schemeClr(accent1, {"lumMod 50000"}) << "203864" << 0;
        QTest::newRow("lumMod 20 lumOff 80") << schemeClr(accent1, {"lumMod 20000", "lumOff 80000"}) << "DAE3F3" << 0;
        QTest::newRow("lumMod 40 lumOff 60") << schemeClr(accent1, {"lumMod 40000", "lumOff 60000"}) << "B4C7E7" << 0;
        QTest::newRow("lumMod 200 stops at white") << schemeClr(accent1, {"lumMod 200000"}) << "FFFFFF" << 0;
        QTest::newRow("lumOff -80 stops at black") << schemeClr(accent1, {"lumOff -80000"}) << "000000" << 0;
        QTest::newRow("lum 50") << schemeClr(accent1, {"lum 50000"}) << "3D6DC2" << 0;
        QTest::newRow("lumMod 50 of a gray") << srgbClr(QStringLiteral("808080"), {"lumMod 50000"}) << "404040" << 0;
        QTest::newRow("satMod 50") << schemeClr(accent1, {"satMod 50000"}) << "647BA4" << 0;
        QTest::newRow("satOff 20") << schemeClr(accent1, {"satOff 20000"}) << "2B6BDD" << 0;
        QTest::newRow("satMod 300 stops at full") << schemeClr(accent1, {"satMod 300000"}) << "0961FF" << 0;
        QTest::newRow("sat 0 is a gray") << schemeClr(accent1, {"sat 0"}) << "848484" << 0;
        QTest::newRow("hueOff 60 degrees") << schemeClr(accent1, {"hueOff 3600000"}) << "9644C4" << 0;
        QTest::newRow("hueOff -60 degrees") << schemeClr(accent1, {"hueOff -3600000"}) << "44C496" << 0;
        QTest::newRow("hueOff a full turn") << schemeClr(accent1, {"hueOff 21600000"}) << "4472C4" << 0;
        QTest::newRow("hueMod 50") << schemeClr(accent1, {"hueMod 50000"}) << "5BC444" << 0;
        QTest::newRow("hue 0") << schemeClr(accent1, {"hue 0"}) << "C44444" << 0;
        QTest::newRow("comp") << schemeClr(accent1, {"comp"}) << "C49644" << 0;
        QTest::newRow("comp of red") << srgbClr(QStringLiteral("FF0000"), {"comp"}) << "00FFFF" << 0;
        QTest::newRow("inv") << schemeClr(accent1, {"inv"}) << "BB8D3B" << 0;
        QTest::newRow("inv of white") << srgbClr(QStringLiteral("FFFFFF"), {"inv"}) << "000000" << 0;
        QTest::newRow("gray") << schemeClr(accent1, {"gray"}) << "6E6E6E" << 0;
        QTest::newRow("gray of red") << srgbClr(QStringLiteral("FF0000"), {"gray"}) << "4C4C4C" << 0;
        QTest::newRow("gray of white") << srgbClr(QStringLiteral("FFFFFF"), {"gray"}) << "FFFFFF" << 0;
        // The channels, in sRGB.
        QTest::newRow("red 50") << schemeClr(accent1, {"red 50000"}) << "8072C4" << 0;
        QTest::newRow("redMod 50") << schemeClr(accent1, {"redMod 50000"}) << "2272C4" << 0;
        QTest::newRow("greenOff 10") << schemeClr(accent1, {"greenOff 10000"}) << "448CC4" << 0;
        QTest::newRow("blueMod 150 stops at full") << schemeClr(accent1, {"blueMod 150000"}) << "4472FF" << 0;
        QTest::newRow("redOff -100 stops at none") << schemeClr(accent1, {"redOff -100000"}) << "0072C4" << 0;
        // Shade and tint, in linear RGB.
        QTest::newRow("shade 50 of white") << srgbClr(QStringLiteral("FFFFFF"), {"shade 50000"}) << "BCBCBC" << 1;
        QTest::newRow("tint 50 of black") << srgbClr(QStringLiteral("000000"), {"tint 50000"}) << "BCBCBC" << 1;
        QTest::newRow("shade 50") << schemeClr(accent1, {"shade 50000"}) << "2F528F" << 1;
        QTest::newRow("tint 40") << schemeClr(accent1, {"tint 40000"}) << "CFD5EA" << 1;
        QTest::newRow("shade 25 of white") << srgbClr(QStringLiteral("FFFFFF"), {"shade 25000"}) << "898989" << 1;
        QTest::newRow("tint 80") << srgbClr(QStringLiteral("FF8000"), {"tint 80000"}) << "FFA47C" << 1;
        QTest::newRow("shade 100 keeps it") << schemeClr(accent1, {"shade 100000"}) << "4472C4" << 1;
        QTest::newRow("tint 100 keeps it") << schemeClr(accent1, {"tint 100000"}) << "4472C4" << 1;
        QTest::newRow("tint 0 is white") << schemeClr(accent1, {"tint 0"}) << "FFFFFF" << 0;
        QTest::newRow("shade 0 is black") << schemeClr(accent1, {"shade 0"}) << "000000" << 0;
        // In the order they are written.
        QTest::newRow("lumOff 20 then lumMod 50") << schemeClr(accent1, {"lumOff 20000", "lumMod 50000"}) << "2C4E8B" << 0;
        QTest::newRow("lumMod 50 then lumOff 20") << schemeClr(accent1, {"lumMod 50000", "lumOff 20000"}) << "3864B2" << 0;
        // What is not understood is passed over.
        QTest::newRow("gamma and invGamma") << schemeClr(accent1, {"gamma", "invGamma"}) << "4472C4" << 0;
        QTest::newRow("a transform of the future") << schemeClr(accent1, {"bogus 5", "lumMod 75000"}) << "2F5597" << 0;
        QTest::newRow("a transform with no value") << schemeClr(accent1, {"lumMod", "lumOff"}) << "4472C4" << 0;
        QTest::newRow("a transform with a bad value") << schemeClr(accent1, {"lumMod many"}) << "4472C4" << 0;
        QTest::newRow("alpha leaves the color") << schemeClr(accent1, {"alpha 50000"}) << "4472C4" << 0;
    }

    void colorAppliesTransforms()
    {
        QFETCH(QString, xml);
        QFETCH(QString, hex);
        QFETCH(int, tolerance);
        const QColor c = colorOf(xml);
        QVERIFY2(nearTo(c, hex, tolerance), qPrintable(QStringLiteral("%1 is not %2 (within %3)").arg(hexOf(c), hex).arg(tolerance)));
    }

    void colorAppliesAlpha()
    {
        auto alphaOf = [](const QStringList &list) { return colorOf(srgbClr(QStringLiteral("FF0000"), list)).alpha(); };
        QCOMPARE(alphaOf({}), 255);
        QVERIFY(qAbs(alphaOf({"alpha 50000"}) - 128) <= 1);
        QCOMPARE(alphaOf({"alpha 0"}), 0);
        QCOMPARE(alphaOf({"alpha 100000"}), 255);
        QCOMPARE(alphaOf({"alpha 25000"}), 64);
        QCOMPARE(alphaOf({"alpha 150000"}), 255);                   // stops at opaque
        QCOMPARE(alphaOf({"alpha 50000", "alphaMod 50000"}), 64);
        QCOMPARE(alphaOf({"alphaMod 50000"}), 128);                 // of opaque
        QCOMPARE(alphaOf({"alphaMod 0"}), 0);
        QCOMPARE(alphaOf({"alpha 100000", "alphaOff -25000"}), 191);
        QCOMPARE(alphaOf({"alphaOff -50000"}), 128);
        QCOMPARE(alphaOf({"alpha 80000", "alphaOff 50000"}), 255);  // stops at opaque
        QCOMPARE(alphaOf({"alphaOff -200000"}), 0);                 // stops at clear
        // The color under it is the color.
        QCOMPARE(hexOf(colorOf(srgbClr(QStringLiteral("FF0000"), {"alpha 50000"}))), QStringLiteral("FF0000"));
        // Other transforms keep the alpha.
        const QColor c = colorOf(srgbClr(QStringLiteral("FF0000"), {"alpha 50000", "lumMod 40000"}));
        QCOMPARE(hexOf(c), QStringLiteral("660000"));
        QVERIFY(qAbs(c.alpha() - 128) <= 1);
    }

    void colorReadsSystemColors()
    {
        // The value it had when saved wins over the name.
        QCOMPARE(hexOf(colorOf(QStringLiteral("<a:sysClr val=\"windowText\" lastClr=\"F0F0F0\"/>"))), QStringLiteral("F0F0F0"));
        QCOMPARE(hexOf(colorOf(QStringLiteral("<a:sysClr val=\"window\" lastClr=\"101010\"/>"))), QStringLiteral("101010"));
        // Without it, the name.
        QCOMPARE(hexOf(colorOf(QStringLiteral("<a:sysClr val=\"window\"/>"))), QStringLiteral("FFFFFF"));
        QCOMPARE(hexOf(colorOf(QStringLiteral("<a:sysClr val=\"windowText\"/>"))), QStringLiteral("000000"));
        QVERIFY(!colorOf(QStringLiteral("<a:sysClr val=\"menu\"/>")).isValid());
        QVERIFY(!colorOf(QStringLiteral("<a:sysClr/>")).isValid());
        // A bad lastClr is passed over for the name.
        QCOMPARE(hexOf(colorOf(QStringLiteral("<a:sysClr val=\"window\" lastClr=\"zz\"/>"))), QStringLiteral("FFFFFF"));
        QCOMPARE(hexOf(colorOf(QStringLiteral("<a:sysClr val=\"window\" lastClr=\"808080\"><a:lumMod val=\"50000\"/></a:sysClr>"))), QStringLiteral("404040"));
    }

    void colorReadsPresetColors()
    {
        const struct { const char *name, *hex; } known[] = {
            {"black", "000000"}, {"white", "FFFFFF"}, {"red", "FF0000"}, {"blue", "0000FF"}, {"lime", "00FF00"}, {"gray", "808080"},
            {"green", "008000"}, {"aliceBlue", "F0F8FF"}, {"yellowGreen", "9ACD32"}, {"dimGray", "696969"},
            // dk, lt, and med for dark, light, and medium.
            {"dkBlue", "00008B"}, {"dkGray", "A9A9A9"}, {"dkOliveGreen", "556B2F"}, {"dkOrange", "FF8C00"}, {"dkSlateGray", "2F4F4F"},
            {"ltGray", "D3D3D3"}, {"ltGoldenrodYellow", "FAFAD2"}, {"ltSlateGray", "778899"}, {"medAquamarine", "66CDAA"},
            {"medVioletRed", "C71585"}, {"ltGrey", "D3D3D3"}, {"dkGrey", "A9A9A9"}};
        for (const auto &k : known)
            QCOMPARE(hexOf(colorOf(QStringLiteral("<a:prstClr val=\"%1\"/>").arg(QString::fromLatin1(k.name)))), QString::fromLatin1(k.hex));
        // Every name the schema lists.
        const QStringList all = QStringLiteral(
            "aliceBlue antiqueWhite aqua aquamarine azure beige bisque black blanchedAlmond blue blueViolet brown burlyWood cadetBlue "
            "chartreuse chocolate coral cornflowerBlue cornsilk crimson cyan dkBlue dkCyan dkGoldenrod dkGray dkGreen dkKhaki dkMagenta "
            "dkOliveGreen dkOrange dkOrchid dkRed dkSalmon dkSeaGreen dkSlateBlue dkSlateGray dkTurquoise dkViolet deepPink deepSkyBlue "
            "dimGray dodgerBlue firebrick floralWhite forestGreen fuchsia gainsboro ghostWhite gold goldenrod gray green greenYellow "
            "honeydew hotPink indianRed indigo ivory khaki lavender lavenderBlush lawnGreen lemonChiffon ltBlue ltCoral ltCyan "
            "ltGoldenrodYellow ltGray ltGreen ltPink ltSalmon ltSeaGreen ltSkyBlue ltSlateGray ltSteelBlue ltYellow lime limeGreen linen "
            "magenta maroon medAquamarine medBlue medOrchid medPurple medSeaGreen medSlateBlue medSpringGreen medTurquoise medVioletRed "
            "midnightBlue mintCream mistyRose moccasin navajoWhite navy oldLace olive oliveDrab orange orangeRed orchid paleGoldenrod "
            "paleGreen paleTurquoise paleVioletRed papayaWhip peachPuff peru pink plum powderBlue purple red rosyBrown royalBlue "
            "saddleBrown salmon sandyBrown seaGreen seaShell sienna silver skyBlue slateBlue slateGray snow springGreen steelBlue tan "
            "teal thistle tomato turquoise violet wheat white whiteSmoke yellow yellowGreen "
            "dkGrey dimGrey dkSlateGrey grey ltGrey ltSlateGrey slateGrey").split(QLatin1Char(' '));
        QCOMPARE(all.size(), 147);
        for (const QString &name : all)
            QVERIFY2(colorOf(QStringLiteral("<a:prstClr val=\"%1\"/>").arg(name)).isValid(), qPrintable(name));
        for (const char *bad : {"", "nonsense", "dkNonsense", "transparent", "#FF0000", "re d", "red1"})
            QVERIFY2(!colorOf(QStringLiteral("<a:prstClr val=\"%1\"/>").arg(QString::fromLatin1(bad))).isValid(), bad);
        QVERIFY(!colorOf(QStringLiteral("<a:prstClr/>")).isValid());
    }

    void colorReadsHslAndScrgb()
    {
        auto hsl = [](const char *hue, const char *sat, const char *lum) {
            return colorOf(QStringLiteral("<a:hslClr hue=\"%1\" sat=\"%2\" lum=\"%3\"/>").arg(QString::fromLatin1(hue), QString::fromLatin1(sat), QString::fromLatin1(lum)));
        };
        QCOMPARE(hexOf(hsl("0", "100000", "50000")), QStringLiteral("FF0000"));
        QCOMPARE(hexOf(hsl("7200000", "50000", "25000")), QStringLiteral("206020"));      // 120 degrees
        QCOMPARE(hexOf(hsl("14400000", "100000", "50000")), QStringLiteral("0000FF"));    // 240 degrees
        QCOMPARE(hexOf(hsl("0", "0", "100000")), QStringLiteral("FFFFFF"));
        QCOMPARE(hexOf(hsl("0", "0", "0")), QStringLiteral("000000"));
        QVERIFY(!colorOf(QStringLiteral("<a:hslClr hue=\"0\" sat=\"100000\"/>")).isValid());   // lum missing

        auto scrgb = [](const char *r, const char *g, const char *b) {
            return colorOf(QStringLiteral("<a:scrgbClr r=\"%1\" g=\"%2\" b=\"%3\"/>").arg(QString::fromLatin1(r), QString::fromLatin1(g), QString::fromLatin1(b)));
        };
        QCOMPARE(hexOf(scrgb("100000", "0", "0")), QStringLiteral("FF0000"));
        QCOMPARE(hexOf(scrgb("0", "0", "0")), QStringLiteral("000000"));
        QCOMPARE(hexOf(scrgb("100000", "100000", "100000")), QStringLiteral("FFFFFF"));
        // The values are linear light, as scRGB is: half is a lighter sRGB gray.
        QVERIFY(nearTo(scrgb("50000", "50000", "50000"), QStringLiteral("BCBCBC"), 1));
        QVERIFY(nearTo(scrgb("25000", "50000", "75000"), QStringLiteral("89BCE1"), 1));
        QVERIFY(!colorOf(QStringLiteral("<a:scrgbClr r=\"x\" g=\"0\" b=\"0\"/>")).isValid());
    }

    void themeReadsAnyColorInASlot()
    {
        QByteArray xml = handWrittenTheme();
        xml.replace("<a:accent1><a:srgbClr val=\"4472C4\"/></a:accent1>", "<a:accent1><a:prstClr val=\"dkBlue\"/></a:accent1>");
        xml.replace("<a:accent2><a:srgbClr val=\"ED7D31\"/></a:accent2>", "<a:accent2><a:hslClr hue=\"0\" sat=\"100000\" lum=\"50000\"/></a:accent2>");
        xml.replace("<a:accent3><a:srgbClr val=\"A5A5A5\"/></a:accent3>", "<a:accent3><a:srgbClr val=\"808080\"><a:lumMod val=\"50000\"/></a:srgbClr></a:accent3>");
        Theme t;
        QString err;
        QVERIFY2(readTheme(xml, &t, &err), qPrintable(err));
        QCOMPARE(hexOf(t.colors[Theme::Accent1]), QStringLiteral("00008B"));
        QCOMPARE(hexOf(t.colors[Theme::Accent2]), QStringLiteral("FF0000"));
        QCOMPARE(hexOf(t.colors[Theme::Accent3]), QStringLiteral("404040"));
        QCOMPARE(hexOf(t.colors[Theme::Accent4]), QStringLiteral("FFC000"));   // the rest as before
    }

    void themeAndColorsSurviveRandomDamage()
    {
        const QByteArray good = handWrittenTheme();
        const QByteArray color = QStringLiteral("<a:solidFill xmlns:a=\"%1\"><a:schemeClr val=\"accent1\"><a:lumMod val=\"60000\"/><a:hueOff val=\"3600000\"/>"
                                                "<a:alpha val=\"50000\"/><a:tint val=\"40000\"/></a:schemeClr></a:solidFill>").arg(ns::A).toUtf8();
        QRandomGenerator gen(77);
        for (int i = 0; i < 500; ++i) {
            for (const QByteArray *source : {&good, &color}) {
                QByteArray bad = *source;
                for (int k = 0, n = 1 + int(gen.bounded(3)); k < n; ++k) bad[int(gen.bounded(quint32(bad.size())))] = char(gen.bounded(256u));
                Theme t;
                readTheme(bad, &t);
                QDomDocument doc;
                if (parseXml(bad, &doc)) readColor(doc.documentElement(), colorTestTheme(), ColorMap(), QColor(Qt::red));
            }
        }
    }
};

JP_TEST_CLASS(OfficeThemeTests)
#include "tst_office_theme.moc"
