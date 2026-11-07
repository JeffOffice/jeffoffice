// Tests of the first screen of a sheet that was saved scrolled far down: the rows above the view are cut down before the parser sees them, and the sheet that comes out is the one the parser
// makes from every row (the heights, the hidden rows, the cells below, and the formulas that share the text of a cell above), from a fraction of the bytes.
#include "sheethelpers.h"

#include "sheet/rowskip.h"

#include <QBuffer>
#include <QElapsedTimer>

namespace {

struct Spec {
    int rows = 20000;
    QString view = QStringLiteral("<sheetViews><sheetView workbookViewId=\"0\"><pane ySplit=\"2\" topLeftCell=\"A6001\" activePane=\"bottomLeft\" state=\"frozen\"/></sheetView></sheetViews>");
    QString format = QStringLiteral("<sheetFormatPr defaultRowHeight=\"15\"/>");
    bool noRowNumbers = false;       // some rows have none
    bool prefix = false;
};

// A sheet of rows: every 7th is tall, every 11th hidden; a number in A, a text in B, a formula in C that shares the text of the one in row 10 (the first cell) for the rows down to 9000, a formula
// of its own in D in every 5th, and a second group that begins in row 12000.
QByteArray makeSheet(const Spec &spec)
{
    QByteArray rows;
    rows.reserve(spec.rows * 120);
    for (int r = 1; r <= spec.rows; ++r) {
        QByteArray row = "<row";
        if (!(spec.noRowNumbers && r % 13 == 4)) row += " r=\"" + QByteArray::number(r) + "\"";
        if (r % 7 == 0) row += " ht=\"30\" customHeight=\"1\"";
        if (r % 11 == 0) row += " hidden=\"1\"";
        if (r % 17 == 0) row += " s=\"3\" customFormat=\"1\"";
        row += " spans=\"1:4\">";
        row += "<c r=\"A" + QByteArray::number(r) + "\"><v>" + QByteArray::number(r) + "</v></c>";
        row += "<c r=\"B" + QByteArray::number(r) + "\" t=\"inlineStr\"><is><t>row " + QByteArray::number(r) + "</t></is></c>";
        if (r == 10) row += "<c r=\"C10\"><f t=\"shared\" ref=\"C10:C9000\" si=\"0\">A10*2+$A$1</f><v>0</v></c>";
        else if (r > 10 && r <= 9000) row += "<c r=\"C" + QByteArray::number(r) + "\"><f t=\"shared\" si=\"0\"/><v>0</v></c>";
        else if (r == 12000) row += "<c r=\"C12000\"><f t=\"shared\" ref=\"C12000:C20000\" si=\"1\">A12000+1</f><v>0</v></c>";
        else if (r > 12000) row += "<c r=\"C" + QByteArray::number(r) + "\"><f t=\"shared\" si=\"1\"/><v>0</v></c>";
        if (r % 5 == 0) row += "<c r=\"D" + QByteArray::number(r) + "\"><f>SUM(A1:A" + QByteArray::number(r) + ")</f><v>1</v></c>";
        row += "</row>";
        rows += row;
    }
    QString head = QStringLiteral("<dimension ref=\"A1:D%1\"/>").arg(spec.rows) + spec.view + spec.format + QStringLiteral("<cols><col min=\"1\" max=\"1\" width=\"12\" customWidth=\"1\"/></cols>");
    return worksheet(QString::fromLatin1(rows), QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"A1:B1\"/></mergeCells>"), head);
}

bool readWith(const QByteArray &xml, bool skip, qint64 screenRows, Sheet *out, RowSkipDevice **device = nullptr)
{
    QByteArray copy = xml;
    QBuffer buffer(&copy);
    buffer.open(QIODevice::ReadOnly);
    ReadOptions o;
    o.screenRows = screenRows;
    o.skipRows = skip;
    Q_UNUSED(device);
    return readSheetPart(&buffer, o, out);
}

QString describe(const Sheet &s)
{
    QString out;
    s.cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
        out += QStringLiteral("%1,%2 t%3 n%4 s%5 f%6 %7\n").arg(row).arg(col).arg(int(c.type)).arg(c.number).arg(c.style).arg(int(c.flags)).arg(s.formulaText(c));
        if (c.type == CellType::InlineString || c.type == CellType::FormulaString) out += s.texts.value(c.textIndex()) + QLatin1Char('\n');
    });
    for (const RowSpec &r : s.rowSpecs) out += QStringLiteral("row %1 h%2 hid%3 ch%4 cf%5 s%6\n").arg(r.row).arg(r.height).arg(r.hidden).arg(r.customHeight).arg(r.customFormat).arg(r.style);
    for (const ColumnSpan &c : s.columns) out += QStringLiteral("col %1-%2 w%3\n").arg(c.first).arg(c.last).arg(c.width);
    const SheetView &v = s.view;
    out += QStringLiteral("view %1 %2 %3 %4 %5 %6 %7 %8\n").arg(v.frozen).arg(v.frozenRows).arg(v.frozenCols).arg(v.topRow).arg(v.leftCol).arg(v.paneRow).arg(v.paneCol).arg(v.activeRow);
    out += QStringLiteral("partial %1 extent %2x%3 merges %4 zero %5 err '%6'\n").arg(s.partial).arg(s.extentRows).arg(s.extentCols).arg(s.merges.size()).arg(s.format.zeroHeight).arg(s.error);
    return out;
}

// Rows of every kind the scanner of plain rows reads, and of every kind it hands to the parser.
QByteArray makeMixedSheet(int rows, const QByteArray &between = QByteArray())
{
    QByteArray body;
    for (int r = 1; r <= rows; ++r) {
        const QByteArray n = QByteArray::number(r);
        QByteArray row = "<row r=\"" + n + "\" spans=\"1:6\"";
        if (r % 7 == 0) row += " ht=\"30\" customHeight=\"1\"";
        if (r % 11 == 0) row += " hidden=\"true\"";
        row += ">";
        row += "<c r=\"A" + n + "\" s=\"1\"><v>" + n + ".5</v></c><c r=\"B" + n + "\" t=\"s\"><v>" + QByteArray::number(r % 3) + "</v></c>";
        row += "<c r=\"C" + n + "\"><f>A" + n + "*2</f><v>" + QByteArray::number(r * 2 + 1) + "</v></c>";
        if (r == 3) row += "<c r=\"D3\"><f t=\"shared\" ref=\"D3:D9\" si=\"0\">A3&gt;1</f><v>1</v></c>";
        else if (r > 3 && r <= 9) row += "<c r=\"D" + n + "\"><f t=\"shared\" si=\"0\"/><v>1</v></c>";
        if (r % 5 == 0) row += "<c r=\"E" + n + "\" t=\"b\"><v>1</v></c><c r=\"F" + n + "\" t=\"e\"><v>#DIV/0!</v></c>";
        if (r % 6 == 0) row += "<c r=\"G" + n + "\" t=\"str\"><f>\"x\"&amp;A" + n + "</f><v>x" + n + "</v></c>";
        if (r % 8 == 0) row += "<c r=\"H" + n + "\" s=\"2\"/>";
        if (r % 9 == 0) row += "<c><v>7</v></c><c t=\"s\"><v>1</v></c>";
        if (r % 13 == 0) row += "<c r=\"I" + n + "\" t=\"inlineStr\"><is><t>row " + n + "</t></is></c>";   // (given to the parser)
        if (r % 17 == 0) row += "<c r=\"J" + n + "\" t=\"str\"><v>caf\xC3\xA9 &amp; more</v></c>";   // (an entity and a letter past ASCII)
        if (r % 19 == 0) row += "<c r=\"K" + n + "\"><f t=\"array\" ref=\"K" + n + ":K" + n + "\">SUM(A1:A3)</f><v>6</v></c>";
        if (r % 23 == 0) row += "<c r=\"L" + n + "\"><v>1</v><f>1+1</f></c>";
        row += "</row>";
        if (r % 29 == 0) row = "<row r=\"" + n + "\" ht=\"20\" customHeight=\"1\"/>";
        if (r % 31 == 0) row = "<row r=\"" + n + "\"><c r=\"A" + n + "\" t=\"n\"><v> 12 </v></c></row>";
        body += row;
        if (r % 37 == 0) body += between;
    }
    return worksheet(QString::fromLatin1(body), QString(), QStringLiteral("<dimension ref=\"A1:L%1\"/>").arg(rows));
}

QString describeAll(const Sheet &s)
{
    return describe(s) + QStringLiteral("stats %1 %2 %3 %4 %5 %6 %7 %8\n").arg(s.formulaCells).arg(s.storedResults).arg(s.stats.longestFormula).arg(s.stats.expandedFormulaChars).arg(s.stats.heldBytes).arg(s.stats.sharedFormulas).arg(s.stats.rowSpecs).arg(s.stats.longestString);
}

bool readRows(const QByteArray &xml, bool fastRows, Sheet *out, qint64 stopAfterRow = -1)
{
    QByteArray copy = xml;
    QBuffer buffer(&copy);
    buffer.open(QIODevice::ReadOnly);
    ReadOptions o;
    o.fastRows = fastRows;
    o.stopAfterRow = stopAfterRow;
    return readSheetPart(&buffer, o, out);
}

} // namespace

class SheetRowSkipTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The sheet made from the rows cut down is the one the parser makes from every row: the same cells, heights, hidden rows, formats, the same text for the formulas that share the one in a
    // row above the view, and the same view; and the parser is given a small part of the bytes.
    void theSheetIsTheSameFromAFractionOfTheBytes()
    {
        const Spec spec;
        const QByteArray xml = makeSheet(spec);
        Sheet fast, slow;
        QVERIFY(readWith(xml, true, 1000, &fast) || !fast.error.isEmpty());
        QVERIFY(readWith(xml, false, 1000, &slow) || !slow.error.isEmpty());
        QCOMPARE(describe(fast), describe(slow));
        QVERIFY(fast.partial && slow.partial);
        QVERIFY(at(fast, QStringLiteral("C6001")) != nullptr);
        QCOMPARE(fast.formulaText(*at(fast, QStringLiteral("C6001"))), QStringLiteral("A6001*2+$A$1"));
        QVERIFY(at(fast, QStringLiteral("A100")) == nullptr);       // (above the view: not kept, in either)
        QVERIFY(at(fast, QStringLiteral("A1")) != nullptr);        // (the frozen rows are kept)
        // the parser was given a fraction of what it is given for every row
        QVERIFY2(fast.stats.xmlBytes * 5 < slow.stats.xmlBytes, qPrintable(QStringLiteral("%1 and %2").arg(fast.stats.xmlBytes).arg(slow.stats.xmlBytes)));
        // and the cells it kept are those of the screen and the frozen rows
        QCOMPARE(fast.cells.cellCount() > 0, true);
    }

    // Counted: the device says how many rows it gave as they were and how many it cut down or left out.
    void theDeviceCountsWhatItLeftOut()
    {
        const Spec spec;
        QByteArray xml = makeSheet(spec);
        QBuffer buffer(&xml);
        buffer.open(QIODevice::ReadOnly);
        ReadOptions o;
        o.screenRows = 1000;
        RowSkipDevice device(&buffer, o);
        Sheet s;
        readSheetPart(&device, ReadOptions(), &s);   // (reads it all: the device is the source)
        QVERIFY(device.skipping());
        // rows 3 to 6000 are above the view (the frozen rows 1 and 2 stay)
        QCOMPARE(device.rowsLeftOut(), qint64(6000 - 2));
        QVERIFY(device.rowsKept() >= 1000);
        QVERIFY(device.bytesLeftOut() > 6000 * 50);
    }

    // The same with other views: not frozen and scrolled; frozen far down with several frozen rows; a view at the top (nothing to leave out); no view at all; the rows kept hidden by default
    // (zeroHeight); rows with no number; a view after the cells.
    void otherViewsGiveTheSameSheet()
    {
        struct Case {
            const char *name;
            Spec spec;
        };
        QVector<Case> cases;
        {
            Spec s;
            s.view = QStringLiteral("<sheetViews><sheetView topLeftCell=\"A3001\" workbookViewId=\"0\"/></sheetViews>");
            cases.append({"scrolled", s});
        }
        {
            Spec s;
            s.view = QStringLiteral("<sheetViews><sheetView topLeftCell=\"B100\" workbookViewId=\"0\"><pane xSplit=\"1\" ySplit=\"5\" topLeftCell=\"B7001\" state=\"frozen\"/></sheetView></sheetViews>");
            cases.append({"frozen below the top", s});
        }
        {
            Spec s;
            s.view = QStringLiteral("<sheetViews><sheetView workbookViewId=\"0\"/></sheetViews>");
            cases.append({"at the top", s});
        }
        {
            Spec s;
            s.view = QString();
            cases.append({"no view", s});
        }
        {
            Spec s;
            s.format = QStringLiteral("<sheetFormatPr defaultRowHeight=\"15\" zeroHeight=\"1\"/>");
            s.rows = 8000;
            s.view = QStringLiteral("<sheetViews><sheetView topLeftCell=\"A4001\" workbookViewId=\"0\"/></sheetViews>");
            cases.append({"zero height", s});
        }
        {
            Spec s;
            s.noRowNumbers = true;
            cases.append({"rows with no number", s});
        }
        {
            Spec s;
            s.rows = 3000;
            s.view = QStringLiteral("<sheetViews><sheetView topLeftCell=\"A2001\" workbookViewId=\"0\"/></sheetViews>");
            cases.append({"a view in the last rows", s});
        }
        for (const Case &c : cases) {
            const QByteArray xml = makeSheet(c.spec);
            Sheet fast, slow;
            readWith(xml, true, 500, &fast);
            readWith(xml, false, 500, &slow);
            QVERIFY2(describe(fast) == describe(slow), c.name);
        }
        // a view after the cells is not looked at (the sheet begins at its first row), in both
        QByteArray late = makeSheet(Spec{3000, QString()});
        late.replace("</sheetData>", "</sheetData><sheetViews><sheetView topLeftCell=\"A2001\" workbookViewId=\"0\"/></sheetViews>");
        Sheet fast, slow;
        readWith(late, true, 500, &fast);
        readWith(late, false, 500, &slow);
        QCOMPARE(describe(fast), describe(slow));
    }

    // What it can't be sure of it passes on as it is: a part in UTF-16, elements with a prefix, an empty cells part, a part with no cells part, a row that is never closed.
    void whatItCannotBeSureOfIsPassedOn()
    {
        const Spec spec{2000, QStringLiteral("<sheetViews><sheetView topLeftCell=\"A1001\" workbookViewId=\"0\"/></sheetViews>")};
        const QByteArray xml = makeSheet(spec);
        // UTF-16
        const QString text = QString::fromUtf8(xml);
        QByteArray utf16("\xFF\xFE", 2);
        utf16.append(reinterpret_cast<const char *>(text.utf16()), text.size() * 2);
        Sheet a, b;
        readWith(utf16, true, 300, &a);
        readWith(utf16, false, 300, &b);
        QCOMPARE(describe(a), describe(b));
        // a prefix
        QByteArray prefixed = xml;
        prefixed.replace("<sheetData>", "<x:sheetData>").replace("</sheetData>", "</x:sheetData>");
        Sheet c, d;
        readWith(prefixed, true, 300, &c);
        readWith(prefixed, false, 300, &d);
        QCOMPARE(describe(c), describe(d));
        // an empty cells part
        const QByteArray empty = worksheet(QString(), QString(), QStringLiteral("<sheetViews><sheetView topLeftCell=\"A1001\" workbookViewId=\"0\"/></sheetViews>")).replace("<sheetData></sheetData>", "<sheetData/>");
        Sheet e, f;
        readWith(empty, true, 300, &e);
        readWith(empty, false, 300, &f);
        QCOMPARE(describe(e), describe(f));
        // no cells part
        Sheet g, h;
        readWith(QByteArray("<worksheet xmlns=\"x\"><sheetViews><sheetView topLeftCell=\"A9\"/></sheetViews></worksheet>"), true, 300, &g);
        readWith(QByteArray("<worksheet xmlns=\"x\"><sheetViews><sheetView topLeftCell=\"A9\"/></sheetViews></worksheet>"), false, 300, &h);
        QCOMPARE(describe(g), describe(h));
        // a row that is never closed, and a row with a number past the sheet, and text where a row goes: the same errors, or none, whether it cuts or not
        for (const char *bad : {"<row r=\"5\"><c r=\"A5\"><v>1</v>", "<row r=\"99999999\"><c r=\"A1\"><v>1</v></c></row>", "text<row r=\"5\"/>", "<row r=\"2\"><c r=\"A2\"><v>1</v></c></row><row r=\"7\"><c r=\"A7\"><f t=\"shared\" ref=\"A7:A9\" si=\"0\">1+1"}) {
            const QByteArray broken = (QStringLiteral("<?xml version=\"1.0\"?><worksheet xmlns=\"") + kMainNs + QStringLiteral("\"><sheetViews><sheetView topLeftCell=\"A4\"/></sheetViews><sheetData>")
                                       + QString::fromUtf8(bad) + QStringLiteral("</sheetData></worksheet>")).toUtf8();
            Sheet x, y;
            const bool ok1 = readWith(broken, true, 300, &x), ok2 = readWith(broken, false, 300, &y);
            QVERIFY2(ok1 == ok2, bad);
            QCOMPARE(x.cells.cellCount(), y.cells.cellCount());
        }
    }

    // A CDATA section or a comment that holds "</row>" in a row above the view is not the end of the row: the sheet is the one the parser makes from every row, not a torn one.
    void aClosingTagInASectionOfARowIsNotTheEndOfTheRow()
    {
        const Spec spec{3000, QStringLiteral("<sheetViews><sheetView topLeftCell=\"A2001\" workbookViewId=\"0\"/></sheetViews>")};
        QByteArray xml = makeSheet(spec);
        const QByteArray openTag = "<row r=\"5\" spans=\"1:4\">";
        QVERIFY(xml.contains(openTag));
        xml.replace(openTag, openTag + "<c r=\"Z5\" t=\"inlineStr\"><is><t><![CDATA[</row><row r=\"3\"><c r=\"A3\"><v>7</v></c>]]></t></is></c><!-- </row> -->");
        const QByteArray other = "<row r=\"2103\" spans=\"1:4\">";
        QVERIFY(xml.contains(other));
        xml.replace(other, other + "<!-- </row> --><c r=\"Z2103\"><v>1</v></c>");
        Sheet fast, slow;
        const bool ok1 = readWith(xml, true, 300, &fast), ok2 = readWith(xml, false, 300, &slow);
        QVERIFY2(ok1 && ok2, qPrintable(fast.error + QLatin1String(" / ") + slow.error));
        QCOMPARE(describe(fast), describe(slow));
        QVERIFY(at(fast, QStringLiteral("Z2103")) != nullptr);
        QVERIFY(at(fast, QStringLiteral("A2001")) != nullptr);
    }

    // Rows larger than what the device reads at a time, above the view and in it, with a shared formula's first cell far into one: the same sheet.
    void rowsLargerThanAReadAreHandled()
    {
        QByteArray rows;
        const QByteArray big(300000, 'x');
        for (int r = 1; r <= 30; ++r) {
            rows += "<row r=\"" + QByteArray::number(r) + "\" ht=\"20\" customHeight=\"1\"><c r=\"A" + QByteArray::number(r) + "\" t=\"inlineStr\"><is><t>" + (r % 3 == 0 ? big : QByteArray("small")) + "</t></is></c>";
            if (r == 4) rows += "<c r=\"B4\"><f t=\"shared\" ref=\"B4:B30\" si=\"0\">A4&amp;\"!\"</f><v>0</v></c>";
            else if (r > 4) rows += "<c r=\"B" + QByteArray::number(r) + "\"><f t=\"shared\" si=\"0\"/><v>0</v></c>";
            rows += "</row>";
        }
        const QByteArray xml = worksheet(QString::fromLatin1(rows), QString(), QStringLiteral("<sheetViews><sheetView topLeftCell=\"A20\" workbookViewId=\"0\"/></sheetViews>"));
        Sheet fast, slow;
        readWith(xml, true, 8, &fast);
        readWith(xml, false, 8, &slow);
        QCOMPARE(describe(fast), describe(slow));
        QVERIFY(at(fast, QStringLiteral("B20")) != nullptr);
        QCOMPARE(fast.formulaText(*at(fast, QStringLiteral("B20"))), QStringLiteral("A20&\"!\""));
        QVERIFY(fast.stats.xmlBytes < xml.size() / 3);
    }

    // A very large sheet far down: the part before the view is cut down in a fraction of the time the parser takes (counted: the bytes the parser is given).
    void aLargeSheetIsReadFromAFractionOfItsBytes()
    {
        Spec spec;
        spec.rows = 120000;
        spec.view = QStringLiteral("<sheetViews><sheetView topLeftCell=\"A100001\" workbookViewId=\"0\"/></sheetViews>");
        const QByteArray xml = makeSheet(spec);
        Sheet fast;
        QVERIFY(readWith(xml, true, 1000, &fast) || fast.partial);
        QVERIFY(at(fast, QStringLiteral("A100001")) != nullptr);
        QVERIFY(at(fast, QStringLiteral("A99999")) == nullptr);
        QVERIFY2(fast.stats.xmlBytes * 10 < xml.size(), qPrintable(QStringLiteral("%1 of %2").arg(fast.stats.xmlBytes).arg(xml.size())));
    }

    // Rows of plain cells are read by the scanner of rows and the others by the parser, and the sheet is the one the parser makes of every row.
    void plainRowsAreScannedAndTheSheetIsTheSame()
    {
        const QByteArray xml = makeMixedSheet(400);
        Sheet fast, slow;
        QVERIFY(readRows(xml, true, &fast));
        QVERIFY(readRows(xml, false, &slow));
        QCOMPARE(describeAll(fast), describeAll(slow));
        QCOMPARE(slow.stats.fastRows, qint64(0));
        // (the rows with an inline string, an entity, or a letter past ASCII are the parser's; the rest are the scanner's)
        QVERIFY2(fast.stats.fastRows > 250 && fast.stats.fastRows < 400, qPrintable(QString::number(fast.stats.fastRows)));
        QVERIFY(at(fast, QStringLiteral("A399")) != nullptr);
        QCOMPARE(fast.formulaText(*at(fast, QStringLiteral("D5"))), QStringLiteral("A5>1"));
    }

    // The scanner gives the parser the rest when the XML is not in the form it reads, and takes the rows up again after a row the parser reads.
    void theScannerHandsOverAndTakesBack()
    {
        Sheet a, b;
        // whitespace between the rows, a comment between rows, and a prefixed row: the scanner stops or the parser reads that row, and the sheet is the same
        for (const QByteArray &between : {QByteArray("\n  "), QByteArray("<!-- note -->"), QByteArray("<row r=\"900\" xmlns:q=\"urn:q\" q:z=\"1\"><c r=\"A900\"><v>9</v></c></row>")}) {
            const QByteArray xml = makeMixedSheet(200, between);
            QVERIFY(readRows(xml, true, &a));
            QVERIFY(readRows(xml, false, &b));
            QCOMPARE(describeAll(a), describeAll(b));
        }
        // a stop at a row (the first screen) is the same
        const QByteArray xml = makeMixedSheet(300);
        QVERIFY(readRows(xml, true, &a, 150));
        QVERIFY(readRows(xml, false, &b, 150));
        QVERIFY(a.partial && b.partial);
        QCOMPARE(describeAll(a), describeAll(b));
        QVERIFY(a.stats.fastRows > 100);
        // a part cut off in a row fails in the same words
        QByteArray cut = makeMixedSheet(100);
        cut.truncate(cut.indexOf("<row r=\"60\"") + 40);
        QVERIFY(!readRows(cut, true, &a));
        QVERIFY(!readRows(cut, false, &b));
        QCOMPARE(a.error, b.error);
        QVERIFY(!a.error.isEmpty());
        QCOMPARE(describeAll(a), describeAll(b));
    }

    // A formula over the limit is refused as it is when the parser reads it.
    void theScannerKeepsTheLimits()
    {
        QByteArray rows;
        for (int r = 1; r <= 5; ++r) rows += "<row r=\"" + QByteArray::number(r) + "\"><c r=\"A" + QByteArray::number(r) + "\"><f>" + QByteArray(r * 10, 'A') + "</f><v>1</v></c></row>";
        const QByteArray xml = worksheet(QString::fromLatin1(rows));
        Sheet a, b;
        for (bool fast : {true, false}) {
            QByteArray copy = xml;
            QBuffer buffer(&copy);
            buffer.open(QIODevice::ReadOnly);
            ReadOptions o;
            o.limits.formulaChars = 30;
            o.fastRows = fast;
            QVERIFY(!readSheetPart(&buffer, o, fast ? &a : &b));
        }
        QVERIFY(!a.error.isEmpty());
        QCOMPARE(a.error, b.error);
        QCOMPARE(describeAll(a), describeAll(b));
    }

    // A comment or an instruction between rows that have no number is read in the order of the file, not after the row that follows it.
    void rowsWithoutNumbersKeepTheirOrderAroundComments()
    {
        for (const char *between : {"<!-- x -->", "<?pi x?>"}) {
            const QByteArray xml = worksheet(QStringLiteral("<row><c><v>1</v></c></row>") + QLatin1String(between) + QStringLiteral("<row><c><v>2</v></c></row><row><c><v>3</v></c></row><row><c><v>4</v></c></row>"));
            Sheet a, b;
            QVERIFY(readRows(xml, true, &a));
            QVERIFY(readRows(xml, false, &b));
            QCOMPARE(describeAll(a), describeAll(b));
            QVERIFY(at(a, QStringLiteral("A3")) != nullptr);
            QCOMPARE(at(a, QStringLiteral("A3"))->number, 3.0);
            QCOMPARE(at(a, QStringLiteral("A2"))->number, 2.0);
        }
    }

    // A name that is no XML name (it begins with a digit) is for the parser, which refuses it.
    void namesThatAreNotNamesGoToTheParser()
    {
        for (const char *cell : {"<row r=\"1\"><c 1=\"x\" r=\"A1\"><v>1</v></c></row>", "<row 1=\"x\" r=\"1\"><c r=\"A1\"><v>1</v></c></row>"}) {
            const QByteArray xml = worksheet(QString::fromLatin1(cell));
            Sheet a, b;
            const bool fa = readRows(xml, true, &a), fb = readRows(xml, false, &b);
            QCOMPARE(fa, fb);
            QVERIFY(!fb);
            QCOMPARE(a.error, b.error);
            QCOMPARE(describeAll(a), describeAll(b));
        }
    }

    // A row of more cells than a sheet has columns is refused as the parser refuses it, at once and without holding the row.
    void aRowOfMillionsOfCellsIsRefusedAtOnce()
    {
        QByteArray row = "<row r=\"1\">";
        for (int i = 0; i < 2000000; ++i) row += "<c/>";
        row += "</row>";
        const QByteArray xml = worksheet(QString::fromLatin1(row));
        Sheet a, b;
        QElapsedTimer timer;
        timer.start();
        QVERIFY(!readRows(xml, true, &a));
        const qint64 fastMs = timer.elapsed();
        QVERIFY(!readRows(xml, false, &b));
        QCOMPARE(a.error, b.error);
        QVERIFY(!a.error.isEmpty());
        QCOMPARE(a.stats.fastRows, qint64(0));
        QVERIFY2(fastMs < 5000, qPrintable(QString::number(fastMs)));
    }

    // A string over the limit is refused as the parser refuses it.
    void theScannerKeepsTheStringLimit()
    {
        const QByteArray xml = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"str\"><v>short</v></c></row><row r=\"2\"><c r=\"A2\" t=\"str\"><v>a value that is far too long</v></c></row>"));
        Sheet a, b;
        for (bool fast : {true, false}) {
            QByteArray copy = xml;
            QBuffer buffer(&copy);
            buffer.open(QIODevice::ReadOnly);
            ReadOptions o;
            o.limits.stringChars = 10;
            o.fastRows = fast;
            QVERIFY(!readSheetPart(&buffer, o, fast ? &a : &b));
        }
        QVERIFY(!a.error.isEmpty());
        QCOMPARE(a.error, b.error);
        QCOMPARE(describeAll(a), describeAll(b));
    }
};

JP_TEST_CLASS(SheetRowSkipTests)
#include "tst_sheet_rowskip.moc"
