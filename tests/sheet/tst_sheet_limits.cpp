// Tests of the hostile files: every count, size, and depth a file controls is bounded, and each bound has a file that passes it; and of reading in pieces,
// with the work counted (bytes taken from the part, cells held) and never the time.
#include "sheethelpers.h"

#include "office/xml.h"
#include "office/xmlstream.h"

#include <QXmlStreamReader>

#include <cstring>

namespace {

QString cellRow(int count, const QString &prefix = QString())
{
    QString row = QStringLiteral("<row r=\"1\">");
    for (int i = 0; i < count; ++i) row += QStringLiteral("<c r=\"%1\"><v>1</v></c>").arg(prefix + columnName(quint32(i)) + "1");
    return row + "</row>";
}

bool readSharedXml(const QByteArray &xml, const Limits &limits, SharedStrings *out, QString *error)
{
    QByteArray copy = xml;
    QBuffer buffer(&copy);
    buffer.open(QIODevice::ReadOnly);
    return readSharedStrings(&buffer, limits, out, error);
}

// A worksheet that is made as it is read: `rows` rows of five cells (a number, a shared string, a formula with its result, an inline string, and a number),
// handed out in whatever pieces are asked for, so that however large the sheet, none of it is ever in memory here. It counts what it has handed out.
class GeneratedSheet : public QIODevice {
public:
    explicit GeneratedSheet(qint64 rows) : m_rows(rows)
    {
        m_pending = ("<?xml version=\"1.0\"?><worksheet xmlns=\"" + kMainNs + "\"><sheetData>").toUtf8();
        open(QIODevice::ReadOnly);
    }
    bool isSequential() const override { return true; }
    qint64 served() const { return m_served; }
    // The XML of row r.
    static QByteArray rowXml(qint64 r)
    {
        const QByteArray n = QByteArray::number(r);
        return "<row r=\"" + n + "\"><c r=\"A" + n + "\"><v>" + n + "</v></c><c r=\"B" + n + "\" t=\"s\"><v>" + QByteArray::number(r % 100) + "</v></c><c r=\"C" + n + "\"><f>A" + n
            + "*2</f><v>" + QByteArray::number(r * 2) + "</v></c><c r=\"D" + n + "\" t=\"inlineStr\"><is><t>row " + n + "</t></is></c><c r=\"E" + n + "\"><v>" + QByteArray::number(r * 0.5)
            + "</v></c></row>";
    }

protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        while (m_pending.size() < maxlen && !m_finished) {
            if (m_next > m_rows) {
                m_pending += "</sheetData></worksheet>";
                m_finished = true;
            } else {
                m_pending += rowXml(m_next++);
            }
            if (m_pending.size() >= 1) break;
        }
        const qint64 n = std::min<qint64>(maxlen, m_pending.size());
        std::memcpy(data, m_pending.constData(), size_t(n));
        m_pending.remove(0, int(n));
        m_served += n;
        return n;
    }
    qint64 writeData(const char *, qint64) override { return -1; }

private:
    qint64 m_rows, m_next = 1, m_served = 0;
    QByteArray m_pending;
    bool m_finished = false;
};

} // namespace

class SheetLimitTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // ---- the bounds: a file at each passes, and a file past each is refused with what came before it kept

    void cellsPerSheetAreBounded()
    {
        Limits limits;
        limits.cellsPerSheet = 100;
        ReadOptions options;
        options.limits = limits;
        Sheet ok;
        QVERIFY2(readSheetXml(worksheet(cellRow(100)), &ok, options), qPrintable(ok.error));
        QCOMPARE(ok.cells.cellCount(), qint64(100));
        Sheet over;
        QVERIFY(!readSheetXml(worksheet(cellRow(101)), &over, options));
        QVERIFY2(over.error.contains(QStringLiteral("cells")), qPrintable(over.error));
        QCOMPARE(over.cells.cellCount(), qint64(100));
        QVERIFY(over.cells.find(0, 99));
        // The real bound is far above any real workbook, and a sheet of 5,000 rows of 20 cells is nowhere near it.
        QVERIFY(kMaxCellsPerSheet >= 10 * 5000000);
    }

    // What a sheet takes to hold is bounded as a whole, so that cells that each carry a string cannot fill the memory under every other limit.
    void whatASheetTakesToHoldIsBounded()
    {
        ReadOptions options;
        options.limits.sheetBytes = 150000;
        const auto row = [](int i) {
            QString cells;
            for (int c = 0; c < 20; ++c) cells += QStringLiteral("<c r=\"%1%2\" t=\"inlineStr\"><is><t>%3</t></is></c>").arg(columnName(quint32(c))).arg(i + 1).arg(QString(100, 'x'));
            return QStringLiteral("<row r=\"%1\">").arg(i + 1) + cells + "</row>";
        };
        // 20 cells of 100 characters is 20 * (32 for the cell + 32 for the slot of its text + 240 for the block that holds it) = 6,080 bytes a row.
        QString rows;
        for (int i = 0; i < 24; ++i) rows += row(i);
        Sheet ok;
        QVERIFY2(readSheetXml(worksheet(rows), &ok, options), qPrintable(ok.error));
        QCOMPARE(ok.cells.cellCount(), qint64(480));
        QCOMPARE(ok.stats.heldBytes, qint64(24 * 6080));
        rows += row(24) + row(25);
        Sheet over;
        QVERIFY(!readSheetXml(worksheet(rows), &over, options));
        QVERIFY2(over.error.contains(QStringLiteral("memory")), qPrintable(over.error));
        QVERIFY(over.cells.cellCount() >= 480 && over.cells.cellCount() < 520);
        // Formulas count too.
        ReadOptions formulas;
        formulas.limits.sheetBytes = 5000;
        QString cells;
        for (int i = 0; i < 100; ++i) cells += QStringLiteral("<c r=\"A%1\"><f>%2</f><v>1</v></c>").arg(i + 1).arg(QString(50, '1'));
        Sheet many;
        QVERIFY(!readSheetXml(worksheet("<row r=\"1\">" + cells + "</row>"), &many, formulas));
        QVERIFY(kMaxSheetBytes >= 10 * qint64(207576864));   // the most counted for a sheet of the practice workbooks
        QVERIFY(kMaxSheetBytes <= 2500000000);                // so that with the file, the shared strings, and the parser, a read stays within 4 GB
        QVERIFY(kMaxSharedStrings >= 10 * 66608 && kMaxSharedStringChars >= 10 * 547609);
    }

    void theLengthOfAStringIsBounded()
    {
        ReadOptions options;
        options.limits.stringChars = 10;
        const auto sheetWith = [](const QString &text) { return worksheet("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>" + text + "</t></is></c><c r=\"B1\"><v>1</v></c></row>"); };
        Sheet ok;
        QVERIFY2(readSheetXml(sheetWith(QString(10, 'x')), &ok, options), qPrintable(ok.error));
        Sheet over;
        QVERIFY(!readSheetXml(sheetWith(QString(11, 'x')), &over, options));
        QVERIFY2(over.error.contains(QStringLiteral("string")), qPrintable(over.error));
        // A rich string counts all its runs; a text of a formula's result counts too.
        Sheet runs;
        QVERIFY(!readSheetXml(worksheet("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><r><t>aaaaaa</t></r><r><t>bbbbbb</t></r></is></c></row>"), &runs, options));
        Sheet result;
        QVERIFY(!readSheetXml(worksheet("<row r=\"1\"><c r=\"A1\" t=\"str\"><f>1</f><v>" + QString(11, 'y') + "</v></c></row>"), &result, options));
        // The shared strings: the bound too, and what came before the long one is kept.
        Limits limits;
        limits.stringChars = 10;
        SharedStrings strings;
        QString why;
        QVERIFY(readSharedXml(sharedStringsXml({"<t>" + QString(10, 'x') + "</t>"}), limits, &strings, &why));
        QVERIFY(!readSharedXml(sharedStringsXml({QStringLiteral("<t>short</t>"), "<t>" + QString(11, 'x') + "</t>", QStringLiteral("<t>after</t>")}), limits, &strings, &why));
        QVERIFY(why.contains(QStringLiteral("string")));
        QCOMPARE(strings.strings.size(), 1);
        QCOMPARE(strings.at(0), QStringLiteral("short"));
        // At the real bound.
        Sheet atBound, pastBound;
        QVERIFY(readSheetXml(sheetWith(QString(int(kMaxStringChars), 'x')), &atBound));
        QVERIFY(!readSheetXml(sheetWith(QString(int(kMaxStringChars) + 1, 'x')), &pastBound));
        QVERIFY(kMaxStringChars >= 10 * 32767);
    }

    void theLengthOfAFormulaIsBounded()
    {
        ReadOptions options;
        options.limits.formulaChars = 20;
        const auto sheetWith = [](const QString &f) { return worksheet("<row r=\"1\"><c r=\"A1\"><f>" + f + "</f><v>1</v></c><c r=\"B1\"><v>1</v></c></row>"); };
        Sheet ok;
        QVERIFY2(readSheetXml(sheetWith(QString("A1+") + QString(17, '1')), &ok, options), qPrintable(ok.error));
        QCOMPARE(ok.stats.longestFormula, qint64(20));
        Sheet over;
        QVERIFY(!readSheetXml(sheetWith(QString("A1+") + QString(18, '1')), &over, options));
        QVERIFY2(over.error.contains(QStringLiteral("formula")), qPrintable(over.error));
        // A shared formula's first cell is the same.
        Sheet shared;
        QVERIFY(!readSheetXml(worksheet("<row r=\"1\"><c r=\"A1\"><f t=\"shared\" ref=\"A1:A2\" si=\"0\">" + QString(30, '1') + "</f></c></row>"), &shared, options));
        Sheet atBound, pastBound;
        QVERIFY(readSheetXml(sheetWith(QString(int(kMaxFormulaChars), '1')), &atBound));
        QVERIFY(!readSheetXml(sheetWith(QString(int(kMaxFormulaChars) + 1, '1')), &pastBound));
        QVERIFY(kMaxFormulaChars >= 10 * 8192);
    }

    void sharedFormulasAreBounded()
    {
        const auto master = [](int si, const QString &ref) { return QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><f t=\"shared\" ref=\"%2\" si=\"%3\">B1+1</f></c></row>").arg(si + 1).arg(ref).arg(si); };
        // How many there are.
        ReadOptions count;
        count.limits.sharedFormulas = 2;
        Sheet ok, over;
        QVERIFY2(readSheetXml(worksheet(master(0, "A1:A2") + master(1, "A2:A3")), &ok, count), qPrintable(ok.error));
        QVERIFY(!readSheetXml(worksheet(master(0, "A1:A2") + master(1, "A2:A3") + master(2, "A3:A4")), &over, count));
        QVERIFY2(over.error.contains(QStringLiteral("shared")), qPrintable(over.error));
        QCOMPARE(over.cells.cellCount(), qint64(2));
        // How many cells one says it covers: the whole grid is refused at the real bound.
        ReadOptions range;
        range.limits.sharedRangeCells = 100;
        Sheet inRange, outRange;
        QVERIFY2(readSheetXml(worksheet(master(0, "A1:A100")), &inRange, range), qPrintable(inRange.error));
        QVERIFY(!readSheetXml(worksheet(master(0, "A1:A101")), &outRange, range));
        QVERIFY2(outRange.error.contains(QStringLiteral("range")), qPrintable(outRange.error));
        Sheet grid;
        QVERIFY(!readSheetXml(worksheet(master(0, "A1:XFD1048576")), &grid));
        QVERIFY(kMaxSharedRangeCells >= 10 * 5000000);
        // The most formula characters expanded in one sheet found is 8,054,139: the limit is about ten times that, no more.
        QVERIFY(kMaxExpandedFormulaChars >= 10 * qint64(8054139) && kMaxExpandedFormulaChars <= 200000000);
        // The characters of every formula asked for, a shared formula counted once for each cell: 20 characters in 3 cells is 60.
        ReadOptions expanded;
        expanded.limits.expandedFormulaChars = 59;
        const QString text = QStringLiteral("SUM(A1:A9)+SUM(B1:B9)");   // 21 characters
        const QString rows = "<row r=\"1\"><c r=\"C1\"><f t=\"shared\" ref=\"C1:C3\" si=\"0\">" + text.left(20) + "</f></c></row>"
                             "<row r=\"2\"><c r=\"C2\"><f t=\"shared\" si=\"0\"/></c></row><row r=\"3\"><c r=\"C3\"><f t=\"shared\" si=\"0\"/></c></row>";
        Sheet fits, tooMuch;
        expanded.limits.expandedFormulaChars = 60;
        QVERIFY2(readSheetXml(worksheet(rows), &fits, expanded), qPrintable(fits.error));
        QCOMPARE(fits.stats.expandedFormulaChars, qint64(60));
        expanded.limits.expandedFormulaChars = 59;
        QVERIFY(!readSheetXml(worksheet(rows), &tooMuch, expanded));
        QVERIFY2(tooMuch.error.contains(QStringLiteral("formula")), qPrintable(tooMuch.error));
    }

    void sheetsInAWorkbookAreBounded()
    {
        const auto workbookWith = [](int sheets) {
            QString list;
            for (int i = 0; i < sheets; ++i) list += QStringLiteral("<sheet name=\"S%1\" sheetId=\"%1\" r:id=\"rId%1\"/>").arg(i + 1);
            return ("<workbook xmlns=\"" + kMainNs + "\" xmlns:r=\"" + kRelNs + "\"><sheets>" + list + "</sheets></workbook>").toUtf8();
        };
        const auto read = [](const QByteArray &xml, WorkbookInfo *info, QString *why) {
            QByteArray copy = xml;
            QBuffer buffer(&copy);
            buffer.open(QIODevice::ReadOnly);
            return readWorkbookPart(&buffer, Limits(), info, why);
        };
        WorkbookInfo atBound, over;
        QString why;
        QVERIFY2(read(workbookWith(kMaxSheets), &atBound, &why), qPrintable(why));
        QCOMPARE(atBound.sheets.size(), kMaxSheets);
        QVERIFY(!read(workbookWith(kMaxSheets + 1), &over, &why));
        QVERIFY2(why.contains(QStringLiteral("sheets")), qPrintable(why));
        QVERIFY(over.sheets.size() <= kMaxSheets);
        QVERIFY(kMaxSheets >= 10 * 100);
        // A name of a sheet is bounded too.
        WorkbookInfo longName;
        QVERIFY(!read(("<workbook xmlns=\"" + kMainNs + "\"><sheets><sheet name=\"" + QString(kMaxSheetNameChars + 1, 'n') + "\" sheetId=\"1\"/></sheets></workbook>").toUtf8(), &longName, &why));
    }

    void sharedStringsAreBounded()
    {
        Limits limits;
        limits.sharedStrings = 3;
        SharedStrings ok, over;
        QString why;
        QVERIFY(readSharedXml(sharedStringsXml({"<t>a</t>", "<t>b</t>", "<t>c</t>"}), limits, &ok, &why));
        QCOMPARE(ok.strings.size(), 3);
        QVERIFY(!readSharedXml(sharedStringsXml({"<t>a</t>", "<t>b</t>", "<t>c</t>", "<t>d</t>"}), limits, &over, &why));
        QCOMPARE(over.strings.size(), 3);
        QVERIFY2(why.contains(QStringLiteral("strings")), qPrintable(why));
        Limits chars;
        chars.sharedStringChars = 10;
        SharedStrings a, b;
        QVERIFY(readSharedXml(sharedStringsXml({"<t>12345</t>", "<t>67890</t>"}), chars, &a, &why));
        QVERIFY(!readSharedXml(sharedStringsXml({"<t>12345</t>", "<t>678901</t>"}), chars, &b, &why));
        QCOMPARE(b.strings.size(), 1);
        QVERIFY(why.contains(QStringLiteral("characters")));
    }

    void rowsAndColumnsAreInsideTheGrid()
    {
        // An address of a place past the last row or column (or too large to be a number) refuses the sheet, and what came before it stays.
        for (const char *bad : {"<c r=\"XFE1\"><v>1</v></c>", "<c r=\"A1048577\"><v>1</v></c>", "<c r=\"A99999999999999999999\"><v>1</v></c>", "<c r=\"ZZZZ1\"><v>1</v></c>",
                                "<c r=\"$XFE$1\"><v>1</v></c>", "<c r=\"A0\"><v>1</v></c>"}) {
            Sheet s;
            QVERIFY2(!readSheetXml(worksheet("<row r=\"1\"><c r=\"A1\"><v>0</v></c>" + QString::fromLatin1(bad) + "</row>"), &s), bad);
            QVERIFY2(!s.error.isEmpty(), bad);
            QCOMPARE(s.cells.cellCount(), qint64(1));
        }
        for (const char *bad : {"<row r=\"1048577\"><c><v>1</v></c></row>", "<row r=\"0\"><c><v>1</v></c></row>", "<row r=\"-5\"><c><v>1</v></c></row>"}) {
            Sheet s;
            QVERIFY2(!readSheetXml(worksheet(QString::fromLatin1(bad)), &s), bad);
        }
        // Text that is no address at all (some writers number cells their own way) is as none: the cell follows the one before it, and so does a row.
        Sheet own;
        QVERIFY2(readSheetXml(worksheet("<row r=\"x\"><c r=\"1_1\"><v>1</v></c><c r=\"\"><v>2</v></c><c r=\"A\"><v>3</v></c><c r=\"11\"><v>4</v></c></row><row r=\"3\"><c r=\"B2\"><v>5</v></c></row>"),
                                 &own), qPrintable(own.error));
        SharedStrings none;
        QCOMPARE(show(own, none, "A1"), QStringLiteral("n:1"));
        QCOMPARE(show(own, none, "B1"), QStringLiteral("n:2"));
        QCOMPARE(show(own, none, "C1"), QStringLiteral("n:3"));
        QCOMPARE(show(own, none, "D1"), QStringLiteral("n:4"));
        QCOMPARE(show(own, none, "B2"), QStringLiteral("n:5"));
        // A row that fills the last column has no room for one more cell with no address.
        Sheet s;
        QString full = "<row r=\"1\"><c r=\"XFD1\"><v>1</v></c><c><v>2</v></c></row>";
        QVERIFY(!readSheetXml(worksheet(full), &s));
        // The corners are inside.
        Sheet corners;
        QVERIFY2(readSheetXml(worksheet("<row r=\"1048576\"><c r=\"XFD1048576\"><v>1</v></c></row><row r=\"1\"><c r=\"A1\"><v>2</v></c></row>"), &corners), qPrintable(corners.error));
        QCOMPARE(corners.cells.cellCount(), qint64(2));
    }

    // XML nested deeper than any sheet is, an element with thousands of attributes (which take a parser time in proportion to their square), and a DOCTYPE.
    void hostileXmlIsRefused()
    {
        QString deep;
        for (int i = 0; i < 300; ++i) deep += "<x>";
        for (int i = 0; i < 300; ++i) deep += "</x>";
        Sheet nested;
        QVERIFY(!readSheetXml(worksheet("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>", "<extLst>" + deep + "</extLst>"), &nested));
        QVERIFY2(nested.error.contains(QStringLiteral("deep")), qPrintable(nested.error));
        QCOMPARE(nested.cells.cellCount(), qint64(1));
        // The deepest a sheet goes is nowhere near: root, sheetData, row, cell, formula.
        QString twoHundred;
        for (int i = 0; i < 200; ++i) twoHundred += "<x>";
        for (int i = 0; i < 200; ++i) twoHundred += "</x>";
        Sheet fine;
        QVERIFY2(readSheetXml(worksheet("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>", "<extLst>" + twoHundred + "</extLst>"), &fine), qPrintable(fine.error));
        // 100,000 attributes: refused from the bytes, without a parser ever seeing the element.
        QString attributes;
        for (int i = 0; i < 100000; ++i) attributes += QStringLiteral(" a%1=\"\"").arg(i);
        Sheet crowded;
        QVERIFY(!readSheetXml(worksheet("<row r=\"1\"><c r=\"A1\"" + attributes + "><v>1</v></c></row>"), &crowded));
        QVERIFY2(crowded.error.contains(QStringLiteral("attributes")), qPrintable(crowded.error));
        // The work is what the parser is given: the first thousand attributes of 1.3 MB of them, never the rest.
        QVERIFY2(crowded.stats.xmlBytes > 0 && crowded.stats.xmlBytes < 20000, qPrintable(QString::number(crowded.stats.xmlBytes)));
        Sheet crowdedRoot;
        QVERIFY(!readSheetXml("<worksheet" + attributes.toUtf8() + "><sheetData/></worksheet>", &crowdedRoot));
        // 1,000 attributes are allowed, in a quoted value too.
        QString allowed;
        for (int i = 0; i < 900; ++i) allowed += QStringLiteral(" a%1=\"x=y\"").arg(i);
        Sheet fineAttributes;
        QVERIFY2(readSheetXml(worksheet("<row r=\"1\"><c r=\"A1\"" + allowed + "><v>1</v></c></row>"), &fineAttributes), qPrintable(fineAttributes.error));
        // A comment, CDATA, or a quoted value full of "=" and ">" doesn't count as attributes.
        Sheet comments;
        QVERIFY2(readSheetXml(worksheet("<!-- " + QString(2000, '=') + " --><row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t><![CDATA[" + QString(2000, '=') + "]]></t></is></c></row>"), &comments),
                 qPrintable(comments.error));
        // A DOCTYPE (which is how an entity is defined) is refused.
        Sheet doctype;
        QVERIFY(!readSheetXml("<?xml version=\"1.0\"?><!DOCTYPE worksheet [<!ENTITY x \"y\">]><worksheet><sheetData><row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>&x;</t></is></c></row></sheetData></worksheet>",
                              &doctype));
        QVERIFY2(doctype.error.contains(QStringLiteral("DOCTYPE")), qPrintable(doctype.error));
        // Not XML, and cut off: what came before is kept.
        Sheet cut;
        QByteArray xml = worksheet("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c></row><row r=\"2\"><c r=\"A2\"><v>3</v></c></row>");
        xml.truncate(xml.indexOf("<row r=\"2\""));
        xml += "<row r=\"2\"><c r=\"A2\"><v>3</v>";
        QVERIFY(!readSheetXml(xml, &cut));
        QVERIFY(!cut.error.isEmpty());
        QCOMPARE(cut.cells.cellCount(), qint64(2));
        Sheet junk;
        QVERIFY(!readSheetXml("PK\x03\x04 this is not xml at all", &junk));
        Sheet empty;
        QVERIFY(!readSheetXml(QByteArray(), &empty));
    }

    // XML that is not UTF-8 is refused before it is parsed: the parser would take bytes such as 22 01 (one letter of UTF-16) for what they are in UTF-8, a
    // quote, and the guard that counts attributes would count wrong. Nothing but the first bytes is ever passed on.
    void xmlThatIsNotUtf8IsRefusedBeforeItIsParsed()
    {
        QString attributes;
        for (int i = 0; i < 20000; ++i) attributes += QStringLiteral(" a\u0122%1=\"v\u0122\"").arg(i);
        const QString xml = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-16\"?><worksheet xmlns=\"") + kMainNs + "\"><sheetData><row r=\"1\"><c r=\"A1\"" + attributes
            + "><v>1</v></c></row></sheetData></worksheet>";
        QStringEncoder little(QStringConverter::Utf16LE, QStringConverter::Flag::Default), big(QStringConverter::Utf16BE, QStringConverter::Flag::Default);
        const QByteArray withMark = QByteArray("\xff\xfe", 2) + QByteArray(little(xml));
        const QByteArray bigNoMark = QByteArray(big(xml));
        const QByteArray littleNoMark = QByteArray(little(xml));
        for (const QByteArray &bytes : {withMark, bigNoMark, littleNoMark}) {
            Sheet s;
            QVERIFY2(!readSheetXml(bytes, &s), bytes.left(8).toHex().constData());
            QVERIFY2(s.error.contains(QStringLiteral("UTF-8")), qPrintable(s.error));
            QVERIFY2(s.stats.xmlBytes < 4096, qPrintable(QString::number(s.stats.xmlBytes)));
            QCOMPARE(s.cells.cellCount(), qint64(0));
        }
        // The same letters in UTF-8 are read.
        Sheet utf8;
        QByteArray plain = worksheet("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>\u0122\u0122</t></is></c></row>");
        QVERIFY2(readSheetXml("\xef\xbb\xbf" + plain, &utf8), qPrintable(utf8.error));
        QCOMPARE(show(utf8, SharedStrings(), "A1"), QStringLiteral("i:\u0122\u0122"));
    }

    // Each kind of piece of XML has the most bytes it may have, checked before the parser has it: a text, an attribute value, a comment, a CDATA section, a
    // processing instruction, a name. A file of 200 MB of one would otherwise be held whole.
    void aSinglePieceOfXmlIsBounded()
    {
        const int size = int(kMaxXmlTokenBytes) * 4;
        const QByteArray big(size, 'x');
        const QByteArray row = "<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>";
        const auto sheetOf = [&](const QByteArray &before, const QByteArray &token, const QByteArray &after) {
            return ("<?xml version=\"1.0\"?><worksheet xmlns=\"" + kMainNs + "\"><sheetData>").toUtf8() + before + token + after + "</sheetData></worksheet>";
        };
        const QVector<QPair<QString, QByteArray>> cases = {
            {QStringLiteral("text"), sheetOf("<row r=\"1\"><c r=\"A1\"><v>", big, "</v></c></row>")},
            {QStringLiteral("inline text"), sheetOf("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>", big, "</t></is></c></row>")},
            {QStringLiteral("attribute value"), sheetOf(row + "<x a=\"", big, "\"/>")},
            {QStringLiteral("single-quoted value"), sheetOf(row + "<x a='", big, "'/>")},
            {QStringLiteral("comment"), sheetOf(row + "<!--", big, "-->")},
            {QStringLiteral("cdata"), sheetOf(row + "<![CDATA[", big, "]]>")},
            {QStringLiteral("processing instruction"), sheetOf(row + "<?pi ", big, "?>")},
            {QStringLiteral("element name"), sheetOf(row + "<", big, "/>")},
            {QStringLiteral("attribute name"), sheetOf(row + "<x ", big, "=\"1\"/>")},
            {QStringLiteral("text between elements"), sheetOf(row, big, "")},
            {QStringLiteral("many small attributes"), sheetOf(row + "<x", QByteArray(" a=\"" + QByteArray(2000, 'y') + "\"").repeated(900), "/>")},
        };
        for (const auto &c : cases) {
            Sheet s;
            QVERIFY2(!readSheetXml(c.second, &s), qPrintable(c.first));
            // (A name of more than 4 KiB characters is refused by the parser itself, which says the XML is damaged.)
            QVERIFY2(s.error.contains(QStringLiteral("longer")) || (c.first.contains(QStringLiteral("name")) && s.error.contains(QStringLiteral("damaged"))), qPrintable(c.first + ": " + s.error));
            // What the parser was given stops at the bound, and not a byte of the rest is read.
            QVERIFY2(s.stats.xmlBytes <= kMaxXmlTokenBytes + 70000, qPrintable(c.first + ": " + QString::number(s.stats.xmlBytes)));
            QVERIFY2(s.stats.largestToken <= kMaxXmlTokenBytes, qPrintable(c.first));
        }
        // A piece as large as the bound is passed (a string of 32,767 characters is 98,301 bytes at the most).
        Sheet fits;
        QVERIFY2(readSheetXml(sheetOf("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>", QByteArray(int(kMaxStringChars), 'x'), "</t></is></c></row>"), &fits), qPrintable(fits.error));
        QVERIFY(fits.stats.largestToken >= kMaxStringChars);
    }

    // What the guard refuses, it refuses whatever the parser would have made of it: forms of comment and declaration that the standard doesn't have (and
    // that parsers disagree on) are refused as damaged, and the forms it has are followed to where they end.
    void theGuardAndTheParserAgreeOnWhereTagsAre()
    {
        QString attributes;
        for (int i = 0; i < 40000; ++i) attributes += QStringLiteral(" a%1=\"1\"").arg(i);
        const auto sheetWith = [&](const QString &token) {
            return ("<?xml version=\"1.0\"?><worksheet xmlns=\"" + kMainNs + "\"><sheetData><row r=\"1\"><c r=\"A1\"><v>1</v></c></row>" + token + "<x" + attributes + "/></sheetData></worksheet>").toUtf8();
        };
        // Forms the standard has: the element after them is the crowded one, and is refused as that.
        for (const char *token : {"<!---->", "<!-- a - b -->", "<?pi >?>", "<?pi ?>", "<![CDATA[x]]]>", "<!-- <x a=\"> -->", "<?pi \"?>"}) {
            Sheet s;
            QVERIFY2(!readSheetXml(sheetWith(QString::fromLatin1(token)), &s), token);
            QVERIFY2(s.error.contains(QStringLiteral("attributes")), qPrintable(QString::fromLatin1(token) + ": " + s.error));
        }
        // Forms it doesn't have: refused as damaged, before anything after them is passed on.
        for (const char *token : {"<!--->", "<!-->", "<!-- a -- b -->", "<!--- x --->", "<!-- x --!>", "<!x>", "<![CDAT[x]]>", "<", "< x/>", "<x <y/>>", "<x a=\"<\"/>", "<?>", "<!>"}) {
            Sheet s;
            QVERIFY2(!readSheetXml(sheetWith(QString::fromLatin1(token)), &s), token);
            QVERIFY2(s.error.contains(QStringLiteral("damaged")) || s.error.contains(QStringLiteral("DOCTYPE")), qPrintable(QString::fromLatin1(token) + ": " + s.error));
            QVERIFY2(s.stats.xmlBytes < 4000, qPrintable(QString::fromLatin1(token) + ": " + QString::number(s.stats.xmlBytes)));
        }
    }

    // Whatever comes before an element with more attributes than any file has (random pieces of what looks like XML), the parser is never given more attributes
    // than the limit on one element.
    void theParserNeverSeesACrowdedElement()
    {
        static const char *const pieces[] = {"<", ">", "</", "/>", "<!--", "-->", "--", "-", "<![CDATA[", "]]>", "]", "<?", "?>", "?", "\"", "'", "=", " ", "a", "<x", "<c ", "<!", "!",
                                             "[", "&", ";", "\n", "<x a=\"", "<x a='", "<!DOCTYPE", "<?xml ", "\xc4\xa2", "x=\"1\" ", "/", ":"};
        const int count = int(sizeof(pieces) / sizeof(pieces[0]));
        const int scale = std::max(1, qEnvironmentVariableIntValue("JP_FUZZ") / 20);
        QRandomGenerator rng(20261023);
        QByteArray crowdDouble = "<z", crowdSingle = "<z";
        for (int i = 0; i < 1500; ++i) {
            crowdDouble += " b" + QByteArray::number(i) + "=\"1\"";
            crowdSingle += " b" + QByteArray::number(i) + "='1'";
        }
        crowdDouble += "/>";
        crowdSingle += "/>";
        qint64 most = 0, whole = 0;
        for (int round = 0; round < 6000 * scale; ++round) {
            QByteArray xml = "<?xml version=\"1.0\"?><r>";
            for (int n = int(rng.bounded(10)); n > 0; --n) xml += pieces[rng.bounded(count)];
            xml += rng.bounded(2) ? crowdDouble : crowdSingle;
            xml += "</r>";
            QBuffer source(&xml);
            source.open(QIODevice::ReadOnly);
            XmlGuardDevice guard(&source);
            QXmlStreamReader reader(&guard);
            qint64 inElement = 0;
            while (!reader.atEnd())
                if (reader.readNext() == QXmlStreamReader::StartElement) inElement = std::max<qint64>(inElement, reader.attributes().size() + reader.namespaceDeclarations().size());
            most = std::max(most, inElement);
            if (guard.bytesPassed() == xml.size()) ++whole;
            QVERIFY2(inElement <= kMaxAttributesPerElement, qPrintable(QString("round %1: %2 attributes after %3").arg(round).arg(inElement).arg(QString::fromLatin1(xml.left(80)))));
        }
        // Some pieces make no difference and some end the document, but no round is the same.
        QVERIFY(whole < 6000 * scale);
    }

    // A read of a sheet that is nothing but merged ranges, or an extension list, can be stopped: the check is made as the XML is read, not only as cells are.
    void aReadOfNothingButMergedRangesCanBeStopped()
    {
        QString merges;
        for (int i = 0; i < 200000; ++i) merges += QStringLiteral("<mergeCell ref=\"A1:B2\"/>");
        const QByteArray xml = worksheet(QString(), "<mergeCells count=\"200000\">" + merges + "</mergeCells>");
        ReadOptions options;
        qint64 calls = 0;
        options.progress = [&](qint64) {
            ++calls;
            return false;
        };
        Sheet s;
        QVERIFY(!readSheetXml(xml, &s, options));
        QVERIFY2(s.error.contains(QStringLiteral("stopped")), qPrintable(s.error));
        QCOMPARE(calls, qint64(1));
        // Of 5 MB of XML, not a tenth was read.
        QVERIFY2(s.stats.xmlBytes < xml.size() / 10, qPrintable(QString::number(s.stats.xmlBytes)));
        QVERIFY(s.mergeCount < 20000);
        // An extension list is the same.
        QString extensions;
        for (int i = 0; i < 200000; ++i) extensions += QStringLiteral("<e a=\"1\"/>");
        const QByteArray other = worksheet(QString(), "<extLst>" + extensions + "</extLst>");
        calls = 0;
        Sheet t;
        QVERIFY(!readSheetXml(other, &t, options));
        QCOMPARE(calls, qint64(1));
        QVERIFY(t.stats.xmlBytes < other.size() / 10);
    }

    // What the sheet takes is counted at what it costs: a cell with an inline string, 96 bytes at the least (the cell, the slot for its text, and the text's own
    // block); a cell of a shared formula whose first cell has not come, 140 (and the formula, and its entry in the list of those waiting).
    void whatASheetTakesIsCountedAtWhatItCosts()
    {
        const auto rowsOf = [](int count, const std::function<QString(int)> &cell) {
            QString rows;
            for (int i = 0; i < count; ++i) {
                if (i % 1000 == 0) rows += (i ? QStringLiteral("</row>") : QString()) + QStringLiteral("<row r=\"%1\">").arg(i / 1000 + 1);
                rows += cell(i);
            }
            return worksheet(rows + "</row>");
        };
        Sheet inlineStrings;
        QVERIFY2(readSheetXml(rowsOf(3000, [](int) { return QStringLiteral("<c t=\"inlineStr\"><is><t>a1</t></is></c>"); }), &inlineStrings), qPrintable(inlineStrings.error));
        QVERIFY2(inlineStrings.stats.heldBytes >= 3000 * 96, qPrintable(QString::number(inlineStrings.stats.heldBytes)));
        Sheet waiting;
        QVERIFY2(readSheetXml(rowsOf(5000, [](int i) { return QStringLiteral("<c><f t=\"shared\" si=\"%1\"/></c>").arg(i); }), &waiting), qPrintable(waiting.error));
        QVERIFY2(waiting.stats.heldBytes >= 5000 * 140, qPrintable(QString::number(waiting.stats.heldBytes)));
        // So a sheet of them comes to the budget at the number of cells that cost that.
        ReadOptions budget;
        budget.limits.sheetBytes = 5000 * 140 - 1;
        Sheet over;
        QVERIFY(!readSheetXml(rowsOf(5000, [](int i) { return QStringLiteral("<c><f t=\"shared\" si=\"%1\"/></c>").arg(i); }), &over, budget));
        QVERIFY2(over.error.contains(QStringLiteral("memory")), qPrintable(over.error));
        // How many different numbers wait for a first cell is bounded as the number of shared formulas is.
        ReadOptions few;
        few.limits.sharedFormulas = 1000;
        Sheet many;
        QVERIFY(!readSheetXml(rowsOf(1500, [](int i) { return QStringLiteral("<c><f t=\"shared\" si=\"%1\"/></c>").arg(i); }), &many, few));
        QVERIFY2(many.error.contains(QStringLiteral("shared")), qPrintable(many.error));
        Sheet allowed;
        QVERIFY2(readSheetXml(rowsOf(900, [](int i) { return QStringLiteral("<c><f t=\"shared\" si=\"%1\"/></c>").arg(i); }), &allowed, few), qPrintable(allowed.error));
    }

    // Files damaged at random (a byte changed, a piece cut out, doubled, or cut off, a quote or a bracket put in) are read or refused, whatever is left of them,
    // and never loop or crash; when a read says it was whole, it left no error. JP_FUZZ=n runs n times as many.
    void damagedFilesAreReadOrRefused()
    {
        const int scale = std::max(1, qEnvironmentVariableIntValue("JP_FUZZ") / 20);
        QByteArray sheetXml = worksheet(
            "<row r=\"1\" spans=\"1:5\"><c r=\"A1\" t=\"s\"><v>0</v></c><c r=\"B1\" t=\"inlineStr\"><is><r><t>ab</t></r><rPh><t>x</t></rPh></is></c><c r=\"C1\" t=\"d\"><v>2024-03-15T12:00:00</v></c>"
            "<c r=\"D1\" t=\"e\"><v>#REF!</v></c><c r=\"E1\" t=\"b\"><v>1</v></c></row>"
            "<row r=\"2\"><c r=\"A2\"><f t=\"shared\" ref=\"A2:A4\" si=\"0\">B1+$C$1*SUM(D:D)</f><v>1</v></c><c r=\"B2\" t=\"str\"><f>\"x\"&amp;A2</f><v>x1</v></c>"
            "<c r=\"C2\"><f t=\"array\" ref=\"C2:D3\">TRANSPOSE(A1:B2)</f><v>3</v></c><c r=\"E2\"><f t=\"dataTable\" ref=\"E2:E3\" r1=\"A1\"/></c></row>"
            "<row r=\"3\"><c r=\"A3\"><f t=\"shared\" si=\"0\"/></c><c r=\"B3\" s=\"7\"/></row><row r=\"4\"><c r=\"A4\"><f t=\"shared\" si=\"0\"/></c></row>",
            "<mergeCells count=\"1\"><mergeCell ref=\"A1:B2\"/></mergeCells>", "<dimension ref=\"A1:E4\"/>");
        const QByteArray stringsXml = sharedStringsXml({QStringLiteral("<t>one</t>"), QStringLiteral("<r><t>t</t></r><r><t>wo</t></r><rPh><t>x</t></rPh>")});
        const QByteArray book = buildXlsx({sheetXml, sheetXml}, stringsXml, QStringLiteral("date1904=\"1\""), QStringLiteral("calcMode=\"manual\""));
        QRandomGenerator rng(20261022);
        const auto damage = [&](QByteArray bytes) {
            for (int n = 1 + int(rng.bounded(4)); n > 0 && !bytes.isEmpty(); --n) {
                const int at = int(rng.bounded(quint32(bytes.size())));
                const int len = 1 + int(rng.bounded(40));
                switch (rng.bounded(7)) {
                case 0: bytes[at] = char(rng.bounded(256)); break;
                case 1: bytes.remove(at, len); break;
                case 2: bytes.insert(at, bytes.mid(int(rng.bounded(quint32(bytes.size()))), len)); break;
                case 3: bytes.truncate(at); break;
                case 4: bytes.insert(at, "<\"'&>[#$:!("[rng.bounded(10)]); break;
                case 5: bytes.insert(at, QByteArray(1 + int(rng.bounded(3000)), '9')); break;
                default: bytes[at] = char(bytes[at] ^ (1 << rng.bounded(8))); break;
                }
            }
            return bytes;
        };
        ReadOptions options;
        options.limits.cellsPerSheet = 10000;
        options.limits.stringChars = 5000;
        options.limits.formulaChars = 5000;
        qint64 whole = 0, refused = 0;
        for (int round = 0; round < 400 * scale; ++round) {
            Sheet s;
            const bool ok = readSheetXml(damage(sheetXml), &s, options);
            QCOMPARE(ok, s.error.isEmpty());
            QVERIFY(s.cells.cellCount() <= 10000);
            // What was read can be used: every formula and text it holds is reachable.
            SharedStrings none;
            s.cells.forEach([&](const Cell &c) {
                s.formulaText(c);
                s.text(c, none);
                s.formulaRange(c);
            });
            (ok ? whole : refused)++;
            SharedStrings strings;
            QByteArray copy = damage(stringsXml);
            QBuffer buffer(&copy);
            buffer.open(QIODevice::ReadOnly);
            QString why;
            const bool stringsOk = readSharedStrings(&buffer, options.limits, &strings, &why);
            QCOMPARE(stringsOk, why.isEmpty());
            // The same through a ZIP that is damaged too (a part inflates wrongly, a directory lies).
            Package package;
            if (!package.readLazy(damage(book))) continue;
            Workbook workbook;
            if (!workbook.open(package, nullptr, options.limits)) continue;
            for (int i = 0; i < workbook.info().sheets.size(); ++i) {
                Sheet inBook;
                const bool read = workbook.readSheet(i, &inBook);
                QCOMPARE(read, inBook.error.isEmpty());
            }
        }
        // Both happen: some damage is survived, and some is refused.
        QVERIFY2(whole > 0 && refused > 0, qPrintable(QString("%1 whole, %2 refused").arg(whole).arg(refused)));
    }

    // A caller that says stop stops the read, after the cells read so far.
    void aReadCanBeStopped()
    {
        ReadOptions options;
        qint64 calls = 0;
        options.progress = [&](qint64 cells) {
            ++calls;
            return cells < 8192;
        };
        Sheet s;
        GeneratedSheet device(10000);
        QVERIFY(!readSheetPart(&device, options, &s));
        QVERIFY2(s.error.contains(QStringLiteral("stopped")), qPrintable(s.error));
        QVERIFY(calls >= 2);
        QVERIFY(s.cells.cellCount() >= 8192 && s.cells.cellCount() < 8192 + 4096 + 5);
    }

    // ---- reading in pieces

    // A sheet of a million cells is read from a device that hands it out as it is asked, and never holds more than a piece: at every report, what was taken
    // from the device is no more than the XML of the cells read so far, and what the cells take is 24 bytes each and no more.
    void aLargeSheetIsReadInPieces()
    {
        const qint64 rows = 100000;
        GeneratedSheet device(rows);
        qint64 xmlTotal = 0;
        for (qint64 r = 1; r <= rows; ++r) xmlTotal += GeneratedSheet::rowXml(r).size();
        const qint64 perCell = xmlTotal / (rows * 5) + 1;
        qint64 reports = 0, largestAhead = 0;
        ReadOptions options;
        options.progress = [&](qint64 cells) {
            ++reports;
            largestAhead = std::max(largestAhead, device.served() - cells * perCell);
            return true;
        };
        Sheet sheet;
        QVERIFY2(readSheetPart(&device, options, &sheet), qPrintable(sheet.error));
        QCOMPARE(sheet.cells.cellCount(), rows * 5);
        QVERIFY(device.served() > xmlTotal);   // all of it, and the wrapper around the rows
        QVERIFY2(reports >= rows * 5 / 4096 - 1, qPrintable(QString::number(reports)));
        // Never more than a few pieces ahead of what it had turned into cells (a piece is 16,384 bytes or less).
        QVERIFY2(largestAhead < 256 * 1024, qPrintable(QString::number(largestAhead)));
        QVERIFY2(sheet.cells.memoryBytes() <= rows * 5 * 28, qPrintable(QString::number(sheet.cells.memoryBytes())));
        QCOMPARE(sheet.formulaCells, rows);
        QCOMPARE(sheet.storedResults, rows);
        const Cell *last = sheet.cells.find(quint32(rows - 1), 2);
        QVERIFY(last);
        QCOMPARE(sheet.formulaText(*last), QStringLiteral("A%1*2").arg(rows));
        QCOMPARE(sheet.stats.xmlBytes, device.served());
        // What the parser held at once is what it was given at once: no piece of XML larger than a row, and no read larger than a few pieces.
        QVERIFY2(sheet.stats.largestToken > 0 && sheet.stats.largestToken < 1000, qPrintable(QString::number(sheet.stats.largestToken)));
        QVERIFY2(sheet.stats.largestRead > 0 && sheet.stats.largestRead <= 65536, qPrintable(QString::number(sheet.stats.largestRead)));
    }

    // The same through a package that inflates as it is read: the bytes inflated so far stay within a few pieces of what the cells read so far come to, and
    // the part is checked at the end (its CRC).
    void aLargeSheetInAPackageIsInflatedInPieces()
    {
        const qint64 rows = 20000;
        QByteArray xml = "<?xml version=\"1.0\"?><worksheet xmlns=\"" + kMainNs.toUtf8() + "\"><sheetData>";
        for (qint64 r = 1; r <= rows; ++r) xml += GeneratedSheet::rowXml(r);
        xml += "</sheetData></worksheet>";
        const QByteArray zip = buildXlsx({xml}, sharedStringsXml({QStringLiteral("<t>x</t>")}));
        Package package;
        QVERIFY(package.readLazy(zip));
        Workbook book;
        QString why;
        QVERIFY2(book.open(package, &why), qPrintable(why));
        const qint64 base = book.package().inflatedBytes();
        const qint64 perCell = xml.size() / (rows * 5) + 1;
        qint64 largestAhead = 0;
        Sheet sheet;
        QVERIFY2(book.readSheet(0, &sheet, [&](qint64 cells) {
                     largestAhead = std::max(largestAhead, book.package().inflatedBytes() - base - cells * perCell);
                     return true;
                 }),
                 qPrintable(sheet.error));
        QCOMPARE(sheet.cells.cellCount(), rows * 5);
        QCOMPARE(book.package().inflatedBytes() - base, qint64(xml.size()));
        QVERIFY2(largestAhead < 256 * 1024, qPrintable(QString::number(largestAhead)));
        QVERIFY2(sheet.stats.largestToken > 0 && sheet.stats.largestToken < 1000, qPrintable(QString::number(sheet.stats.largestToken)));
        QVERIFY2(sheet.stats.largestRead > 0 && sheet.stats.largestRead <= 65536, qPrintable(QString::number(sheet.stats.largestRead)));
        // A part damaged at its end (its CRC) is not read as whole: the cells come, and the sheet says it is damaged.
        QByteArray damaged = zip;
        for (qsizetype at = damaged.indexOf(QByteArray("PK\x01\x02", 4)); at >= 0; at = damaged.indexOf(QByteArray("PK\x01\x02", 4), at + 4)) {
            const quint16 nameLength = qFromLittleEndian<quint16>(damaged.constData() + at + 28);
            if (QString::fromUtf8(damaged.mid(at + 46, nameLength)) == QLatin1String("xl/worksheets/sheet1.xml")) qToLittleEndian<quint32>(0xdeadbeefu, damaged.data() + at + 16);
        }
        Package badPackage;
        QVERIFY(badPackage.readLazy(damaged));
        Workbook badBook;
        QVERIFY(badBook.open(badPackage, &why));
        Sheet badSheet;
        QVERIFY(!badBook.readSheet(0, &badSheet));
        QVERIFY2(badSheet.error.contains(QStringLiteral("damaged")), qPrintable(badSheet.error));
        QVERIFY(badSheet.cells.cellCount() > 0);
    }
};

JP_TEST_CLASS(SheetLimitTests)
#include "tst_sheet_limits.moc"
