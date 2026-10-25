// Tests of the named cell styles: the built-in ones the program knows (Good, Heading 1, Accent2, Percent), that a workbook that lacks one is given it when it is first used (and only if its
// file has the lists to put it in), that one the file has is used as the file has it, that a style applied gives the cell the style's look and the cell format follows it, and that it is all
// saved and read again.
#include "sheethelpers.h"

#include "sheet/builtinstyles.h"
#include "sheet/xlsxsave.h"

#include <memory>

namespace {

const QString kLists = QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/><scheme val=\"minor\"/></font></fonts>"
                                      "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                                      "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                                      "<cellStyles count=\"2\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/><cellStyle name=\"Mine\" xfId=\"0\"/></cellStyles>");

struct Open {
    Package package;
    Workbook book;
    bool open(const QByteArray &zip)
    {
        QString why;
        return package.readLazy(zip, &why) && book.open(package, &why);
    }
};

QByteArray plainBook(const QString &lists = kLists)
{
    const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"), lists);
    return buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"))}, QByteArray(), QString(), QString(), styles);
}


// A theme that names only its fonts (its colors are the usual ones): the typefaces of headings and of body text.
QByteArray themeWithFonts(const QString &major, const QString &minor)
{
    return QStringLiteral("<a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"><a:themeElements><a:clrScheme name=\"Test\"/><a:fontScheme name=\"Test\">"
                          "<a:majorFont><a:latin typeface=\"%1\"/></a:majorFont><a:minorFont><a:latin typeface=\"%2\"/></a:minorFont></a:fontScheme></a:themeElements></a:theme>")
        .arg(major, minor)
        .toUtf8();
}

QByteArray themedBook(const QByteArray &theme)
{
    const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"), kLists);
    return buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"))}, QByteArray(), QString(), QString(), styles, QStringList(), QString(), theme);
}
} // namespace

class SheetStyleTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Every built-in style resolves by its name (in any case) and has the number the file gives it, once; the names come in the gallery's order, the good, bad, and neutral first.
    void theBuiltInStylesAreKnown()
    {
        const QStringList names = builtinStyleNames();
        QVERIFY(names.size() >= 47);
        QCOMPARE(names.first(), QStringLiteral("Normal"));
        QVERIFY(names.indexOf(QStringLiteral("Good")) < names.indexOf(QStringLiteral("Calculation")));
        QVERIFY(names.indexOf(QStringLiteral("Calculation")) < names.indexOf(QStringLiteral("Title")));
        QVERIFY(names.indexOf(QStringLiteral("Title")) < names.indexOf(QStringLiteral("20% - Accent1")));
        QVERIFY(names.indexOf(QStringLiteral("Accent6")) < names.indexOf(QStringLiteral("Percent")));
        QSet<qint32> ids;
        for (const QString &n : names) {
            BuiltinStyle s;
            QVERIFY2(builtinStyle(n.toUpper(), &s), qPrintable(n));
            QCOMPARE(s.name, n);
            QVERIFY2(!ids.contains(s.builtinId), qPrintable(n));
            ids.insert(s.builtinId);
        }
        BuiltinStyle good, heading, accent, percent;
        QVERIFY(builtinStyle(QStringLiteral("good"), &good) && builtinStyle(QStringLiteral("Heading 1"), &heading) && builtinStyle(QStringLiteral("60% - Accent3"), &accent) && builtinStyle(QStringLiteral("Percent"), &percent));
        QCOMPARE(good.builtinId, 26);
        QCOMPARE(good.fontColor, ColorSpec::fromRgb(0x006100));
        QCOMPARE(good.fill.fg, ColorSpec::fromRgb(0xC6EFCE));
        QVERIFY(heading.bold && heading.size == 15 && heading.border.bottom.style == BorderStyle::Thick);
        QCOMPARE(accent.builtinId, 40);
        QCOMPARE(accent.fill.fg.index, 6);   // (accent 3 is the sixth color of the theme)
        QCOMPARE(percent.numFmtId, 9);
        BuiltinStyle none;
        QVERIFY(!builtinStyle(QStringLiteral("No Such Style"), &none));
    }

    // A workbook is given a built-in style it lacks when it asks for it: the style format has the look, the font is the workbook's own with the style's changes, and asking again gives the
    // same place. A style the file has is the file's; a name that is not a style gives none.
    void aMissingStyleIsMadeOnceAndAStyleTheFileHasIsUsed()
    {
        Open o;
        QVERIFY(o.open(plainBook()));
        Workbook &w = o.book;
        QCOMPARE(w.styleNamed(QStringLiteral("Normal")), 0);
        QCOMPARE(w.styleNamed(QStringLiteral("mine")), 0);       // (the file's own style, by its name)
        QCOMPARE(w.styleNamed(QStringLiteral("Nonsense")), -1);
        QCOMPARE(w.addedStyles().size(), 0);
        const qint32 good = w.styleNamed(QStringLiteral("Good"));
        QCOMPARE(good, 1);
        QCOMPARE(w.styleNamed(QStringLiteral("Good")), good);
        QCOMPARE(w.addedStyles().size(), 1);
        QCOMPARE(w.addedStyles()[0].name.builtin, 26);
        const CellFormat sf = w.styles().styleXfs[good];   // (a copy: the list grows below)
        QVERIFY(w.styles().fills[int(sf.fill)].fg == ColorSpec::fromRgb(0xC6EFCE));
        QVERIFY(w.styles().fonts[int(sf.font)].color == ColorSpec::fromRgb(0x006100));
        QCOMPARE(w.styles().fonts[int(sf.font)].name, QStringLiteral("Calibri"));   // (the workbook's own face and size)
        QCOMPARE(w.styles().fonts[int(sf.font)].size, 11.0);
        QCOMPARE(w.styles().fonts[int(sf.font)].scheme, QStringLiteral("minor"));
        const qint32 title = w.styleNamed(QStringLiteral("Title"));
        QCOMPARE(w.styles().fonts[int(w.styles().styleXfs[title].font)].size, 18.0);
        QCOMPARE(w.styles().fonts[int(w.styles().styleXfs[title].font)].scheme, QStringLiteral("major"));
        QCOMPARE(w.styles().styleXfs[w.styleNamed(QStringLiteral("Percent"))].numFmtId, 9);
        // a cell format with the style: its look and its place among the styles; the same format again is the same place
        FormatChange change;
        change.namedStyle = quint32(good);
        const quint32 cell = w.formatWith(0, change);
        QVERIFY(cell != 0);
        QCOMPARE(w.styles().formats[int(cell)].xfId, quint32(good));
        QCOMPARE(w.styles().formats[int(cell)].fill, sf.fill);
        QCOMPARE(w.formatWith(0, change), cell);
        QCOMPARE(w.formatWith(cell, change), cell);
    }

    // The typefaces are the workbook's, not the program's: a title is set in the heading face the workbook's theme names, a font that only says "minor" is the theme's body face, and a
    // workbook with no theme keeps its own usual face for a title (no name of a face is built into the program).
    void headingsAndBodyTextTakeTheTypefacesOfTheWorkbooksTheme()
    {
        Open o;
        QVERIFY(o.open(themedBook(themeWithFonts(QStringLiteral("Cambria"), QStringLiteral("Candara")))));
        Workbook &w = o.book;
        QCOMPARE(w.styles().majorFont, QStringLiteral("Cambria"));
        QCOMPARE(w.styles().minorFont, QStringLiteral("Candara"));
        const qint32 title = w.styleNamed(QStringLiteral("Title"));
        QVERIFY(title >= 0);
        const FontSpec titleFont = w.styles().fonts[int(w.styles().styleXfs[title].font)];
        QCOMPARE(titleFont.name, QStringLiteral("Cambria"));
        QCOMPARE(titleFont.scheme, QStringLiteral("major"));
        QCOMPARE(titleFont.size, 18.0);
        QCOMPARE(w.styles().fontNameOf(titleFont), QStringLiteral("Cambria"));
        // a font with a name is that name; one with only the theme's say is the theme's
        FontSpec own;
        own.name = QStringLiteral("Georgia");
        QCOMPARE(w.styles().fontNameOf(own), QStringLiteral("Georgia"));
        FontSpec byTheme;
        byTheme.scheme = QStringLiteral("minor");
        QCOMPARE(w.styles().fontNameOf(byTheme), QStringLiteral("Candara"));
        byTheme.scheme = QStringLiteral("major");
        QCOMPARE(w.styles().fontNameOf(byTheme), QStringLiteral("Cambria"));
        // no theme: the title is in the workbook's own face, and nothing built in is named
        Open plain;
        QVERIFY(plain.open(plainBook()));
        const qint32 plainTitle = plain.book.styleNamed(QStringLiteral("Title"));
        QVERIFY(plainTitle >= 0);
        const FontSpec plainFont = plain.book.styles().fonts[int(plain.book.styles().styleXfs[plainTitle].font)];
        QCOMPARE(plainFont.name, plain.book.styles().fonts[0].name);
        QVERIFY(plain.book.styles().majorFont.isEmpty() && plain.book.styles().minorFont.isEmpty());
        FontSpec nameless;
        QCOMPARE(plain.book.styles().fontNameOf(nameless), plain.book.styles().fonts[0].name);   // (the workbook's usual face)
    }

    // A file with no place for styles (no list of cell styles) is given none: the person's workbook is never made to be saved in a form this program does not write.
    void aFileWithNoListOfStylesIsNotGivenOne()
    {
        const QString lists = QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts>"
                                             "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                                             "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>");
        Open o;
        QVERIFY(o.open(plainBook(lists)));
        QCOMPARE(o.book.styleNamed(QStringLiteral("Good")), -1);
        QCOMPARE(o.book.addedStyles().size(), 0);
    }

    // The style made is in the styles part saved: its format among the style formats and its name with the number the file gives it, and the cell format that follows it names it.
    void aStyleMadeIsSavedAndRead()
    {
        Open o;
        QVERIFY(o.open(plainBook()));
        Workbook &w = o.book;
        const qint32 style = w.styleNamed(QStringLiteral("Bad"));
        FormatChange change;
        change.namedStyle = quint32(style);
        w.formatWith(0, change);
        SaveChanges changes;
        for (const Workbook::AddedStyle &s : w.addedStyles()) changes.newStyles.append({s.format, s.name});
        changes.styleXfsBefore = w.styleXfsInFile();
        for (const FontSpec &f : w.addedFonts()) changes.newFonts.append(f);
        changes.fontsBefore = w.fontsInFile();
        for (const FillSpec &f : w.addedFills()) changes.newFills.append(f);
        changes.fillsBefore = w.fillsInFile();
        for (const Workbook::AddedCellFormat &f : w.addedCellFormats()) changes.newCellFormats.append({f.base, f.format.numFmtId, true, f.format});
        changes.cellFormatsBefore = w.cellFormatsInFile();
        QByteArray part;
        const QByteArray original = o.package.part(w.stylesPart());
        QString why;
        QVERIFY2(patchStyles(original, changes, &part, &why), qPrintable(why));
        QVERIFY(part.contains("name=\"Bad\""));
        QVERIFY(part.contains("builtinId=\"27\""));
        Styles read;
        QBuffer in(&part);
        in.open(QIODevice::ReadOnly);
        QVERIFY(readStyles(&in, Limits(), &read));
        QCOMPARE(read.styleXfs.size(), 2);
        int found = -1;
        for (const CellStyleName &n : read.cellStyles)
            if (n.name == QStringLiteral("Bad")) found = int(n.xfId);
        QCOMPARE(found, 1);
        QCOMPARE(read.cellStyles.last().builtin, 27);
        QVERIFY(read.fills[int(read.styleXfs[1].fill)].fg == ColorSpec::fromRgb(0xFFC7CE));
        QCOMPARE(read.formats.size(), 2);
        QCOMPARE(read.formats[1].xfId, 1u);
        QCOMPARE(read.formats[1].fill, read.styleXfs[1].fill);
    }
};

JP_TEST_CLASS(SheetStyleTests)
#include "tst_sheet_styles.moc"
