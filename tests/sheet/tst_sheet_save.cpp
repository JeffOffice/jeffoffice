// Tests of saving a workbook as the old file with what changed put in: the ZIP entries copied as they are, the sheet's XML with changed cells put in their places and the rest
// untouched, the shared strings and styles with additions at their ends, and the calculation chain left out. Every workbook is built in memory.
#include "sheethelpers.h"

#include "io/zipindex.h"
#include "sheet/xlsxsave.h"
#include "sheet/xmlscan.h"

#include <QDir>
#include <QSaveFile>
#include <QTemporaryDir>

using namespace jp;

namespace {

class BufferSink : public PartSink {
public:
    QByteArray data;
    qint64 writes = 0;
    void write(const char *d, qsizetype n) override
    {
        data.append(d, n);
        ++writes;
    }
};

QByteArray patch(const QByteArray &xml, const SheetChanges &changes, bool hasShared = true, QString *error = nullptr, bool *ok = nullptr)
{
    QByteArray copy = xml;
    QBuffer in(&copy);
    in.open(QIODevice::ReadOnly);
    BufferSink out;
    const bool done = patchSheet(&in, &out, changes, hasShared, error);
    if (ok) *ok = done;
    return out.data;
}

SavedCell number(quint32 row, quint16 col, double v, quint32 style = 0)
{
    SavedCell c;
    c.row = row;
    c.col = col;
    c.style = style;
    c.value.kind = SavedValue::Kind::Number;
    c.value.number = v;
    return c;
}

SavedCell text(quint32 row, quint16 col, const QString &t, qint32 shared = -1, quint32 style = 0)
{
    SavedCell c;
    c.row = row;
    c.col = col;
    c.style = style;
    c.value.kind = SavedValue::Kind::Text;
    c.value.text = t;
    c.value.sharedString = shared;
    return c;
}

SavedCell formula(quint32 row, quint16 col, const QString &f, double result, quint32 style = 0)
{
    SavedCell c = number(row, col, result, style);
    c.isFormula = true;
    c.formula = f;
    return c;
}

SavedCell blank(quint32 row, quint16 col, quint32 style = 0)
{
    SavedCell c;
    c.row = row;
    c.col = col;
    c.style = style;
    return c;
}

SavedResult resultNumber(quint32 row, quint16 col, double v)
{
    SavedResult r;
    r.row = row;
    r.col = col;
    r.value.kind = SavedValue::Kind::Number;
    r.value.number = v;
    return r;
}

SheetChanges changesOf(std::vector<SavedCell> cells, std::vector<SavedResult> results = {})
{
    SheetChanges c;
    c.cells = std::move(cells);
    c.results = std::move(results);
    return c;
}

// The entries of a ZIP, by name: what they inflate to.
QMap<QString, QByteArray> entries(const QByteArray &zip)
{
    QMap<QString, QByteArray> out;
    QString why;
    if (!readZip(zip, out, &why)) qWarning() << why;
    return out;
}

// The bytes an entry takes in the ZIP as they are (compressed): what a copy keeps.
QMap<QString, QByteArray> rawEntries(const QByteArray &zip)
{
    QMap<QString, QByteArray> out;
    QVector<ZipEntry> dir;
    if (!readZipDirectory(zip, &dir)) return out;
    for (const ZipEntry &e : dir) out.insert(e.name, zip.mid(int(e.dataAt), int(e.csize)));
    return out;
}

QStringList names(const QByteArray &zip)
{
    QStringList out;
    QVector<ZipEntry> dir;
    if (readZipDirectory(zip, &dir))
        for (const ZipEntry &e : dir) out << e.name;
    return out;
}

// A workbook opened from the bytes, for a save.
struct Opened {
    QByteArray bytes;
    Package package;
    Workbook book;
    bool ok = false;
    QString why;
};

std::unique_ptr<Opened> open(const QByteArray &bytes)
{
    auto o = std::make_unique<Opened>();
    o->bytes = bytes;
    o->ok = o->package.readLazy(o->bytes, &o->why) && o->book.open(o->package, &o->why);
    return o;
}

// A package like a real workbook's: two sheets, shared strings, styles, a theme, a calculation chain, and a part nothing here reads.
QByteArray realistic()
{
    PackageWriter w;
    const QString relBase = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    w.addPart(QStringLiteral("/xl/workbook.xml"), QByteArray(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    w.addRelationship(QStringLiteral("/"), relBase + "officeDocument", QStringLiteral("/xl/workbook.xml"));
    const QByteArray sheet1 = worksheet(QStringLiteral("<row r=\"1\" spans=\"1:3\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f>A1+1</f><v>2</v></c><c r=\"C1\" t=\"s\"><v>0</v></c></row>"
                                                       "<row r=\"2\"><c r=\"A2\"><v>10</v></c><c r=\"B2\"><f>A2*2</f><v>20</v></c></row>"),
                                        QStringLiteral("<conditionalFormatting sqref=\"A1:A2\"><cfRule type=\"cellIs\" dxfId=\"0\" priority=\"1\" operator=\"greaterThan\"><formula>5</formula></cfRule></conditionalFormatting>"),
                                        QStringLiteral("<dimension ref=\"A1:C2\"/>"));
    const QByteArray sheet2 = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>7</v></c></row>"));
    w.addPart(QStringLiteral("/xl/worksheets/sheet1.xml"), sheet1, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
    const QString id1 = w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + "worksheet", QStringLiteral("/xl/worksheets/sheet1.xml"));
    w.addPart(QStringLiteral("/xl/worksheets/sheet2.xml"), sheet2, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
    const QString id2 = w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + "worksheet", QStringLiteral("/xl/worksheets/sheet2.xml"));
    w.addPart(QStringLiteral("/xl/sharedStrings.xml"), sharedStringsXml({QStringLiteral("<t>hello</t>")}), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"));
    w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + "sharedStrings", QStringLiteral("/xl/sharedStrings.xml"));
    w.addPart(QStringLiteral("/xl/styles.xml"),
              stylesXml(QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"0.0&quot; kg&quot;\"/>"), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"164\" applyNumberFormat=\"1\"><alignment horizontal=\"center\"/></xf>")),
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"));
    w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + "styles", QStringLiteral("/xl/styles.xml"));
    w.addPart(QStringLiteral("/xl/calcChain.xml"), QByteArray("<calcChain xmlns=\"" + kMainNs.toUtf8() + "\"><c r=\"B1\" i=\"1\"/><c r=\"B2\" i=\"1\"/></calcChain>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.calcChain+xml"));
    w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + "calcChain", QStringLiteral("/xl/calcChain.xml"));
    w.addPart(QStringLiteral("/xl/theme/theme1.xml"), QByteArray("<a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"/>"), QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"));
    w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + "theme", QStringLiteral("/xl/theme/theme1.xml"));
    w.addPart(QStringLiteral("/xl/vbaProject.bin"), QByteArray("VBA\x01\x02\x03 not really a project", 30), QStringLiteral("application/vnd.ms-office.vbaProject"));
    const QByteArray workbook = ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<workbook xmlns=\"" + kMainNs + "\" xmlns:r=\"" + kRelNs + "\"><sheets><sheet name=\"One\" sheetId=\"1\" r:id=\""
                                 + id1 + "\"/><sheet name=\"Two\" sheetId=\"2\" r:id=\"" + id2 + "\"/></sheets><calcPr calcId=\"191029\"/></workbook>").toUtf8();
    w.addPart(QStringLiteral("/xl/workbook.xml"), workbook, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    return w.finish();
}

} // namespace

class SheetSaveTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // ---- the ZIP

    // An entry copied from another ZIP keeps the other's compressed bytes, CRC, and sizes: what it holds is the same to the last bit, and nothing is deflated again.
    void aZipEntryCanBeCopiedAsItIs()
    {
        ZipWriter first;
        QByteArray big;
        for (int i = 0; i < 4000; ++i) big += "row " + QByteArray::number(i) + " of the sheet\n";
        first.add(QStringLiteral("a/big.xml"), big, true);
        first.add(QStringLiteral("small.txt"), QByteArray("tiny"), false);
        const QByteArray zip = first.finish();
        QVector<ZipEntry> dir;
        QVERIFY(readZipDirectory(zip, &dir));
        QCOMPARE(dir.size(), 2);
        QCOMPARE(int(dir[0].method), 8);
        ZipWriter second;
        for (const ZipEntry &e : dir) second.addRaw(e.name, e.method, e.crc, quint32(e.csize), quint32(e.usize), zip.constData() + e.dataAt);
        const QByteArray copy = second.finish();
        QCOMPARE(entries(copy), entries(zip));
        QCOMPARE(rawEntries(copy), rawEntries(zip));
    }

    // An entry written in pieces is read back whole, with its sizes and CRC right; the pieces may be of any size, and the entry may be stored.
    void aZipEntryCanBeWrittenInPieces()
    {
        QByteArray all;
        for (int i = 0; i < 300000; ++i) all += "<c r=\"A" + QByteArray::number(i) + "\"><v>" + QByteArray::number(i * 7) + "</v></c>";
        for (const bool compress : {true, false}) {
            ZipWriter zip;
            zip.add(QStringLiteral("before.txt"), QByteArray("first"), false);
            zip.beginStream(QStringLiteral("xl/worksheets/sheet1.xml"), compress);
            for (qsizetype at = 0; at < all.size();) {
                const qsizetype n = std::min<qsizetype>(all.size() - at, 1 + (at / 7) % 70000);
                zip.write(all.constData() + at, n);
                at += n;
            }
            zip.endStream();
            zip.add(QStringLiteral("after.txt"), QByteArray("last"), false);
            const QByteArray bytes = zip.finish();
            const QMap<QString, QByteArray> read = entries(bytes);
            QCOMPARE(read.size(), 3);
            QCOMPARE(read.value(QStringLiteral("xl/worksheets/sheet1.xml")), all);
            QCOMPARE(read.value(QStringLiteral("after.txt")), QByteArray("last"));
            QCOMPARE(names(bytes), QStringList({"before.txt", "xl/worksheets/sheet1.xml", "after.txt"}));
            if (compress) {
                QVector<ZipEntry> dir;
                QVERIFY(readZipDirectory(bytes, &dir));
                QCOMPARE(int(dir[1].method), 8);
                QVERIFY(dir[1].csize < all.size() / 2);
            }
        }
    }

    // ---- the scanner

    void theScannerFindsTagsAndAttributes()
    {
        const QByteArray xml = "<?xml version=\"1.0\"?><!-- <c r=\"A1\"> --><x:a b='1 > 0' c=\"d\"><![CDATA[<c/>]]><x:e/></x:a>";
        xmlscan::Tag t;
        QVERIFY(xmlscan::nextTag(xml, 0, &t));
        QCOMPARE(QByteArray(t.name), QByteArray("x:a"));
        QCOMPARE(QByteArray(t.local), QByteArray("a"));
        QVERIFY(!t.selfClosing && !t.closing);
        const QByteArray tag = xml.mid(t.start, t.end - t.start);
        QCOMPARE(tag, QByteArray("<x:a b='1 > 0' c=\"d\">"));
        bool found = false;
        QCOMPARE(QByteArray(xmlscan::attribute(tag, "b", &found)), QByteArray("1 > 0"));
        QVERIFY(found);
        QCOMPARE(QByteArray(xmlscan::attribute(tag, "c", &found)), QByteArray("d"));
        xmlscan::attribute(tag, "z", &found);
        QVERIFY(!found);
        QVERIFY(xmlscan::nextTag(xml, t.end, &t));
        QCOMPARE(QByteArray(t.name), QByteArray("x:e"));
        QVERIFY(t.selfClosing);
        QVERIFY(xmlscan::nextTag(xml, t.end, &t));
        QVERIFY(t.closing);
        QVERIFY(!xmlscan::nextTag(xml, t.end, &t));
        bool cut = false;
        QVERIFY(!xmlscan::nextTag(QByteArray("<a><b c=\"d"), 3, &t, &cut));
        QVERIFY(cut);
        quint32 row = 0, col = 0;
        QVERIFY(xmlscan::parseAddress("AB12", &row, &col));
        QCOMPARE(row, 11u);
        QCOMPARE(col, 27u);
        QVERIFY(!xmlscan::parseAddress("A0", &row, &col));
        QVERIFY(!xmlscan::parseAddress("XFE1", &row, &col));
    }

    // Text goes into XML as it can be read back: the characters XML cannot hold are escaped as the format does, and what looks like such an escape is protected.
    void textIsEscapedAndReadBack()
    {
        const QString nasty = QString::fromUtf8("a & b < c > d \"q\" \x01 _x0041_ _x005F_ end") + QChar(0x0D) + QStringLiteral("x\ttab\nnl");
        const QByteArray esc = xmlscan::escapeText(nasty);
        QVERIFY(!esc.contains('\x01'));
        QVERIFY(!esc.contains('\r'));
        QVERIFY(esc.contains("_x0001_"));
        QVERIFY(esc.contains("_x000D_"));
        QVERIFY(esc.contains("_x005F_x0041_"));
        // through the sheet reader: the same text comes back
        const QByteArray xml = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>") + QString::fromUtf8(esc) + QStringLiteral("</t></is></c></row>"));
        Sheet sheet;
        QVERIFY(readSheetXml(xml, &sheet));
        const Cell *c = at(sheet, QStringLiteral("A1"));
        QVERIFY(c);
        QCOMPARE(sheet.texts.value(c->textIndex()), nasty);
        QCOMPARE(xmlscan::unescape("a &amp; b &lt;&#65;&#x42;&gt; &quot;&apos;"), QStringLiteral("a & b <AB> \"'"));
    }

    // ---- a sheet

    void anEditedCellIsPutInItsPlace()
    {
        const QByteArray xml = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\" s=\"2\"><v>2</v></c><c r=\"C1\"><v>3</v></c></row><row r=\"2\"><c r=\"A2\"><v>4</v></c></row>"));
        QString why;
        bool ok = false;
        const QByteArray out = patch(xml, changesOf({number(0, 1, 2.5, 7)}), true, &why, &ok);
        QVERIFY2(ok, qPrintable(why));
        QCOMPARE(out, worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\" s=\"7\"><v>2.5</v></c><c r=\"C1\"><v>3</v></c></row><row r=\"2\"><c r=\"A2\"><v>4</v></c></row>")));
    }

    // A cell that is new goes where its column puts it, in a row that is there, and a row that is new goes where its number puts it; a row after the last goes at the end.
    void newCellsAndRowsComeInOrder()
    {
        const QByteArray xml = worksheet(QStringLiteral("<row r=\"2\" spans=\"2:3\"><c r=\"B2\"><v>1</v></c><c r=\"D2\"><v>2</v></c></row><row r=\"5\"><c r=\"A5\"><v>9</v></c></row>"));
        const QByteArray out = patch(xml, changesOf({number(0, 0, 1), number(1, 0, 10), number(1, 2, 11), number(1, 4, 12), number(3, 1, 20), number(6, 2, 30)}));
        QCOMPARE(out, worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"
                                               "<row r=\"2\" spans=\"1:5\"><c r=\"A2\"><v>10</v></c><c r=\"B2\"><v>1</v></c><c r=\"C2\"><v>11</v></c><c r=\"D2\"><v>2</v></c><c r=\"E2\"><v>12</v></c></row>"
                                               "<row r=\"4\"><c r=\"B4\"><v>20</v></c></row>"
                                               "<row r=\"5\"><c r=\"A5\"><v>9</v></c></row>"
                                               "<row r=\"7\"><c r=\"C7\"><v>30</v></c></row>")));
        // an empty sheetData, with no content, or closing itself
        for (const QString &empty : {QStringLiteral("<sheetData></sheetData>"), QStringLiteral("<sheetData/>"), QStringLiteral("<sheetData />")}) {
            const QByteArray bare = ("<?xml version=\"1.0\"?><worksheet xmlns=\"" + kMainNs + "\">" + empty + "<pageMargins left=\"1\"/></worksheet>").toUtf8();
            const QByteArray got = patch(bare, changesOf({number(2, 1, 5)}));
            QVERIFY2(got.contains("<row r=\"3\"><c r=\"B3\"><v>5</v></c></row>"), got.constData());
            QVERIFY(got.contains("</sheetData><pageMargins left=\"1\"/></worksheet>"));
        }
    }

    // What the sheet says outside its cells is not touched: not a byte of it. The dimension grows to take in cells put outside it.
    void theRestOfTheSheetIsLeftAsItWas()
    {
        const QString after = QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"A1:B1\"/></mergeCells>\n  <conditionalFormatting sqref=\"A1\"><cfRule type=\"expression\" priority=\"1\"><formula>A1&gt;1</formula></cfRule></conditionalFormatting>"
                                             "<pageMargins left=\"0.7\" right=\"0.7\" top=\"0.75\" bottom=\"0.75\" header=\"0.3\" footer=\"0.3\"/><drawing r:id=\"rId1\"/><extLst><ext uri=\"x\"/></extLst>");
        const QString head = QStringLiteral("<sheetPr><tabColor rgb=\"FFFF0000\"/></sheetPr><dimension ref=\"A1:B2\"/><sheetViews><sheetView workbookViewId=\"0\"/></sheetViews><cols><col min=\"1\" max=\"2\" width=\"12\"/></cols>");
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>\n<row r=\"2\"><c r=\"B2\"><v>1</v></c></row>");
        const QByteArray xml = worksheet(rows, after, head);
        QCOMPARE(patch(xml, SheetChanges()), xml);
        const QByteArray out = patch(xml, changesOf({number(1, 1, 5), number(4, 3, 1)}));
        QCOMPARE(out, worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>\n<row r=\"2\"><c r=\"B2\"><v>5</v></c></row><row r=\"5\"><c r=\"D5\"><v>1</v></c></row>"), after,
                                QString(head).replace(QLatin1String("A1:B2"), QLatin1String("A1:D5"))));
    }

    // A cell that is cleared goes (unless it has a format of its own, which stays); and what a cell holds is written as the file writes it.
    void cellsAreWrittenAsTheFileWritesThem()
    {
        const QByteArray xml = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\" s=\"3\"><v>2</v></c><c r=\"C1\"><v>3</v></c></row>"));
        SavedCell textInShared = text(0, 3, QStringLiteral("shared"), 4);
        SavedCell textInline = text(0, 4, QStringLiteral(" padded & <odd> "));
        SavedCell yes = blank(0, 5);
        yes.value.kind = SavedValue::Kind::Bool;
        yes.value.number = 1;
        SavedCell err = blank(0, 6);
        err.value.kind = SavedValue::Kind::Error;
        err.value.number = double(int(CellError::NA));
        const QByteArray out = patch(xml, changesOf({blank(0, 0), blank(0, 1, 3), number(0, 2, 0.1), textInShared, textInline, yes, err}));
        QCOMPARE(out, worksheet(QStringLiteral("<row r=\"1\"><c r=\"B1\" s=\"3\"/><c r=\"C1\"><v>0.1</v></c><c r=\"D1\" t=\"s\"><v>4</v></c>"
                                               "<c r=\"E1\" t=\"inlineStr\"><is><t xml:space=\"preserve\"> padded &amp; &lt;odd&gt; </t></is></c><c r=\"F1\" t=\"b\"><v>1</v></c><c r=\"G1\" t=\"e\"><v>#N/A</v></c></row>")));
        // with no shared strings in the file, a text is written in the cell
        const QByteArray inlineOut = patch(xml, changesOf({textInShared}), false);
        QVERIFY(inlineOut.contains("<c r=\"D1\" t=\"inlineStr\"><is><t>shared</t></is></c>"));
        // numbers are written so that they read back as the same double
        for (const double v : {0.1, 1.0 / 3, 1e300, -2.5e-10, 123456789012345678.0, 5.0, 0.30000000000000004}) {
            const QByteArray n = patch(xml, changesOf({number(0, 0, v)}));
            Sheet sheet;
            QVERIFY(readSheetXml(n, &sheet));
            QCOMPARE(at(sheet, QStringLiteral("A1"))->number, v);
        }
    }

    void formulasAreWrittenWithTheirResults()
    {
        const QByteArray xml = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"));
        SavedCell str = formula(0, 2, QStringLiteral("A1&\"x<\""), 0);
        str.value.kind = SavedValue::Kind::Text;
        str.value.text = QStringLiteral("1x<");
        SavedCell noResult = formula(0, 3, QStringLiteral("=SUM(A1:A3)"), 0);
        noResult.hasResult = false;
        const QByteArray out = patch(xml, changesOf({formula(0, 1, QStringLiteral("A1+1"), 2, 5), str, noResult}));
        QCOMPARE(out, worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\" s=\"5\"><f>A1+1</f><v>2</v></c><c r=\"C1\" t=\"str\"><f>A1&amp;\"x&lt;\"</f><v>1x&lt;</v></c>"
                                               "<c r=\"D1\"><f>SUM(A1:A3)</f></c></row>")));
        Sheet sheet;
        QVERIFY(readSheetXml(out, &sheet));
        QCOMPARE(sheet.formulaText(*at(sheet, QStringLiteral("C1"))), QStringLiteral("A1&\"x<\""));
    }

    // A formula whose result changed keeps its formula element as it was (shared, array, with its attributes) and has the new value and type.
    void aResultChangesWhatTheFormulaCellHolds()
    {
        const QByteArray xml = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" s=\"4\" t=\"str\"><f t=\"shared\" ref=\"A1:A3\" si=\"0\">B1&amp;\"\"</f><v>old</v></c></row>"
                                                       "<row r=\"2\"><c r=\"A2\" t=\"e\"><f t=\"shared\" si=\"0\"/><v>#N/A</v></c><c r=\"B2\"><f>1+1</f><v>2</v></c></row>"));
        SavedResult r1;
        r1.row = 0;
        r1.col = 0;
        r1.value.kind = SavedValue::Kind::Number;
        r1.value.number = 5;
        SavedResult r2;
        r2.row = 1;
        r2.col = 0;
        r2.value.kind = SavedValue::Kind::Text;
        r2.value.text = QStringLiteral("new & <old>");
        const QByteArray out = patch(xml, changesOf({}, {r1, r2, resultNumber(1, 1, 3)}));
        QCOMPARE(out, worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" s=\"4\"><f t=\"shared\" ref=\"A1:A3\" si=\"0\">B1&amp;\"\"</f><v>5</v></c></row>"
                                               "<row r=\"2\"><c r=\"A2\" t=\"str\"><f t=\"shared\" si=\"0\"/><v>new &amp; &lt;old&gt;</v></c><c r=\"B2\"><f>1+1</f><v>3</v></c></row>")));
    }

    // The first cell of a shared formula is changed: the cells that shared it keep their formulas, each written out.
    void aSharedFormulaSurvivesItsFirstCellBeingChanged()
    {
        QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f t=\"shared\" ref=\"B1:B4\" si=\"3\">A1*2+$A$1</f><v>3</v></c></row>");
        for (int r = 2; r <= 4; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f t=\"shared\" si=\"3\"/><v>%2</v></c></row>").arg(r).arg(r * 2 + 1);
        rows += QStringLiteral("<row r=\"5\"><c r=\"B5\"><f t=\"shared\" si=\"9\"/><v>0</v></c></row>");
        const QByteArray xml = worksheet(rows);
        const QByteArray out = patch(xml, changesOf({number(0, 1, 99)}));
        Sheet sheet;
        QVERIFY(readSheetXml(out, &sheet));
        QCOMPARE(show(sheet, SharedStrings(), QStringLiteral("B1")), QStringLiteral("n:99"));
        QVERIFY(!at(sheet, QStringLiteral("B1"))->hasFormula());
        QCOMPARE(sheet.formulaText(*at(sheet, QStringLiteral("B2"))), QStringLiteral("A2*2+$A$1"));
        QCOMPARE(sheet.formulaText(*at(sheet, QStringLiteral("B3"))), QStringLiteral("A3*2+$A$1"));
        QCOMPARE(sheet.formulaText(*at(sheet, QStringLiteral("B4"))), QStringLiteral("A4*2+$A$1"));
        // (a group that was not touched stays a shared formula)
        QVERIFY(out.contains("<c r=\"B5\"><f t=\"shared\" si=\"9\"/><v>0</v></c>"));
        // a cell that shares the formula and whose result changed has the formula written out, with the new result
        const QByteArray withResult = patch(xml, changesOf({number(0, 1, 99)}, {resultNumber(2, 1, 1234)}));
        Sheet sheet2;
        QVERIFY(readSheetXml(withResult, &sheet2));
        QCOMPARE(sheet2.formulaText(*at(sheet2, QStringLiteral("B3"))), QStringLiteral("A3*2+$A$1"));
        QCOMPARE(show(sheet2, SharedStrings(), QStringLiteral("B3")), QStringLiteral("n:1234"));
        QCOMPARE(sheet2.formulaText(*at(sheet2, QStringLiteral("B4"))), QStringLiteral("A4*2+$A$1"));
        // and a cell of the group changed, not the first: the others stay shared
        const QByteArray other = patch(xml, changesOf({number(2, 1, 99)}));
        QVERIFY(other.contains("<f t=\"shared\" ref=\"B1:B4\" si=\"3\">A1*2+$A$1</f>"));
        QVERIFY(other.contains("<c r=\"B2\"><f t=\"shared\" si=\"3\"/><v>5</v></c>"));
    }

    // A sheet whose elements have a prefix (as some programs write them) is changed in the same way.
    void aPrefixedSheetIsChanged()
    {
        const QByteArray xml = ("<?xml version=\"1.0\"?><x:worksheet xmlns:x=\"" + kMainNs + "\"><x:dimension ref=\"A1\"/><x:sheetData><x:row r=\"1\"><x:c r=\"A1\"><x:v>1</x:v></x:c></x:row></x:sheetData></x:worksheet>").toUtf8();
        const QByteArray out = patch(xml, changesOf({number(0, 1, 2), number(1, 0, 3)}));
        QVERIFY2(out.contains("<x:row r=\"1\"><x:c r=\"A1\"><x:v>1</x:v></x:c><x:c r=\"B1\"><x:v>2</x:v></x:c></x:row><x:row r=\"2\"><x:c r=\"A2\"><x:v>3</x:v></x:c></x:row></x:sheetData>"), out.constData());
        QVERIFY(out.contains("<x:dimension ref=\"A1:B2\"/>"));
        Sheet sheet;
        QVERIFY(readSheetXml(out, &sheet));
        QCOMPARE(show(sheet, SharedStrings(), QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(show(sheet, SharedStrings(), QStringLiteral("B1")), QStringLiteral("n:2"));
        QCOMPARE(show(sheet, SharedStrings(), QStringLiteral("A2")), QStringLiteral("n:3"));
    }

    // A sheet far larger than the pieces it is read in: the changes at the ends and in the middle are in their places, the rest is as it was, and the output is the same
    // whatever the size of the pieces the reader gives.
    void aLargeSheetIsPatchedInPieces()
    {
        QByteArray rows;
        for (int r = 1; r <= 60000; ++r) rows += "<row r=\"" + QByteArray::number(r) + "\"><c r=\"A" + QByteArray::number(r) + "\"><v>" + QByteArray::number(r) + "</v></c><c r=\"B" + QByteArray::number(r) + "\" t=\"inlineStr\"><is><t>row number " + QByteArray::number(r) + " of the sheet</t></is></c></row>";
        const QByteArray xml = worksheet(QString::fromLatin1(rows), QStringLiteral("<pageMargins left=\"1\"/>"));
        QVERIFY(xml.size() > 3 * (1 << 20));
        SheetChanges changes = changesOf({number(0, 0, -1), number(29999, 0, -2), number(30000, 5, -3), number(59999, 0, -4), number(60005, 0, -5)}, {});
        QString why;
        bool ok = false;
        const QByteArray out = patch(xml, changes, true, &why, &ok);
        QVERIFY2(ok, qPrintable(why));
        QByteArray expected = xml;
        auto swap = [&](const QByteArray &from, const QByteArray &to) {
            const qsizetype at = expected.indexOf(from);
            QVERIFY(at >= 0);
            expected.replace(at, from.size(), to);
        };
        swap("<c r=\"A1\"><v>1</v></c>", "<c r=\"A1\"><v>-1</v></c>");
        swap("<c r=\"A30000\"><v>30000</v></c>", "<c r=\"A30000\"><v>-2</v></c>");
        swap("<c r=\"B30001\" t=\"inlineStr\"><is><t>row number 30001 of the sheet</t></is></c></row>", "<c r=\"B30001\" t=\"inlineStr\"><is><t>row number 30001 of the sheet</t></is></c><c r=\"F30001\"><v>-3</v></c></row>");
        swap("<c r=\"A60000\"><v>60000</v></c>", "<c r=\"A60000\"><v>-4</v></c>");
        swap("</sheetData>", "<row r=\"60006\"><c r=\"A60006\"><v>-5</v></c></row></sheetData>");
        QCOMPARE(out.size(), expected.size());
        QVERIFY(out == expected);
    }

    // A closing tag that is not a tag (inside a CDATA section or a comment, in a cell or between cells) does not end the cell or the row: the edits to the cells after it are made, and
    // what is in the section is kept as it was.
    void aClosingTagInACdataSectionOrACommentDoesNotHideTheRestOfTheRow()
    {
        const QByteArray rows = "<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t><![CDATA[x</c>y]]></t></is></c><!-- </c> and </row> --><c r=\"B1\"><v>2</v></c><c r=\"C1\"><v>3</v></c></row>"
                                "<row r=\"2\"><c r=\"A2\" t=\"inlineStr\"><is><t><![CDATA[</row><row r=\"9\">]]></t></is></c><c r=\"B2\"><v>5</v></c></row>"
                                "<row r=\"3\"><c r=\"A3\"><v>1</v></c><!--</row>--></row>"
                                "<row r=\"4\"><c r=\"A4\"><v>1</v></c></row>";
        const QByteArray xml = worksheet(QString::fromLatin1(rows));
        QString why;
        bool ok = false;
        const QByteArray out = patch(xml, changesOf({number(0, 1, 20), number(0, 2, 30), number(1, 1, 50), number(2, 0, 7), number(3, 0, 9)}), true, &why, &ok);
        QVERIFY2(ok, qPrintable(why));
        QByteArray expected = xml;
        auto swap = [&](const QByteArray &from, const QByteArray &to) {
            const qsizetype at = expected.indexOf(from);
            QVERIFY2(at >= 0, from.constData());
            expected.replace(at, from.size(), to);
        };
        // (a comment between the cells of a row that is written again is not kept; what is inside a cell is)
        swap("<!-- </c> and </row> -->", "");
        swap("<!--</row>-->", "");
        swap("<c r=\"B1\"><v>2</v></c>", "<c r=\"B1\"><v>20</v></c>");
        swap("<c r=\"C1\"><v>3</v></c>", "<c r=\"C1\"><v>30</v></c>");
        swap("<c r=\"B2\"><v>5</v></c>", "<c r=\"B2\"><v>50</v></c>");
        swap("<c r=\"A3\"><v>1</v></c>", "<c r=\"A3\"><v>7</v></c>");
        swap("<c r=\"A4\"><v>1</v></c>", "<c r=\"A4\"><v>9</v></c>");
        QVERIFY2(out == expected, qPrintable(QString::fromLatin1(out)));
        // and the sheet reads back with the edits and the text of the sections
        Sheet sheet;
        QVERIFY2(readSheetXml(out, &sheet), qPrintable(sheet.error));
        SharedStrings none;
        QCOMPARE(show(sheet, none, QStringLiteral("B1")), QStringLiteral("n:20"));
        QCOMPARE(show(sheet, none, QStringLiteral("C1")), QStringLiteral("n:30"));
        QCOMPARE(show(sheet, none, QStringLiteral("A1")), QStringLiteral("i:x</c>y"));
        QCOMPARE(show(sheet, none, QStringLiteral("B2")), QStringLiteral("n:50"));
        QCOMPARE(show(sheet, none, QStringLiteral("A2")), QStringLiteral("i:</row><row r=\"9\">"));
    }

    // A part that is cut off, or has no cells part, is refused with a message, not written half.
    void aSheetThatCannotBePatchedIsRefused()
    {
        QString why;
        bool ok = true;
        patch(QByteArray("<worksheet><dimension ref=\"A1\"/></worksheet>"), changesOf({number(0, 0, 1)}), true, &why, &ok);
        QVERIFY(!ok);
        QVERIFY(!why.isEmpty());
        ok = true;
        why.clear();
        patch(QByteArray("<worksheet><sheetData><row r=\"1\"><c r=\"A1\"><v>1</v>"), changesOf({number(0, 0, 1)}), true, &why, &ok);
        QVERIFY(!ok);
        QVERIFY(!why.isEmpty());
    }

    // ---- the other parts

    void sharedStringsGetNewStringsAtTheEnd()
    {
        const QByteArray xml = sharedStringsXml({QStringLiteral("<t>one</t>"), QStringLiteral("<r><t>t</t></r><r><t>wo</t></r>")});
        QByteArray out;
        QVERIFY(patchSharedStrings(xml, {QStringLiteral("three & <four>"), QStringLiteral(" five ")}, &out));
        QVERIFY(out.contains("count=\"4\" uniqueCount=\"4\""));
        QVERIFY(out.startsWith(xml.left(xml.indexOf("count=")))); // (what came before the counts is the same)
        QVERIFY(out.contains("<si><t>one</t></si><si><r><t>t</t></r><r><t>wo</t></r></si><si><t>three &amp; &lt;four&gt;</t></si><si><t xml:space=\"preserve\"> five </t></si></sst>"));
        SharedStrings read;
        QBuffer in(&out);
        in.open(QIODevice::ReadOnly);
        QVERIFY(readSharedStrings(&in, Limits(), &read));
        QCOMPARE(read.strings, QVector<QString>({"one", "two", "three & <four>", " five "}));
    }

    // Strings with runs of formatting are the old file's bytes in the new one: a change to an unrelated cell leaves the shared strings part as it was (the same compressed bytes), and one
    // that adds a string leaves every <si> of the file as it was, with the new one after them. (This documents what saving does; it held before the test was written.)
    void richTextStringsAreTheOldBytesInTheSavedFile()
    {
        const QStringList items = {
            QStringLiteral("<r><rPr><b/><sz val=\"11\"/><rFont val=\"Calibri\"/></rPr><t>bold</t></r><r><t xml:space=\"preserve\"> and plain</t></r>"),
            QStringLiteral("<t>plain</t>"),
            QStringLiteral("<r><rPr><i/><color rgb=\"FFFF0000\"/></rPr><t>red </t></r><r><rPr><u/></rPr><t>under</t></r><rPh sb=\"0\" eb=\"1\"><t>x</t></rPh><phoneticPr fontId=\"1\"/>")};
        const QByteArray bytes = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"s\"><v>0</v></c><c r=\"B1\"><v>1</v></c></row><row r=\"2\"><c r=\"A2\" t=\"s\"><v>2</v></c></row>"))},
                                          sharedStringsXml(items));
        auto o = open(bytes);
        QVERIFY2(o->ok, qPrintable(o->why));
        const QByteArray stringsBefore = entries(bytes).value(QStringLiteral("xl/sharedStrings.xml"));
        QString why;
        // an unrelated number changes: the part is copied as it is
        {
            SaveChanges changes;
            changes.sheets[0] = changesOf({number(0, 1, 7)});
            QByteArray out;
            SaveReport report;
            QVERIFY2(saveXlsx(o->book, changes, &out, &why, &report), qPrintable(why));
            QCOMPARE(entries(out).value(QStringLiteral("xl/sharedStrings.xml")), stringsBefore);
            QCOMPARE(rawEntries(out).value(QStringLiteral("xl/sharedStrings.xml")), rawEntries(bytes).value(QStringLiteral("xl/sharedStrings.xml")));
            QVERIFY(!report.changedParts.contains(QStringLiteral("xl/sharedStrings.xml")));
        }
        // a string is added: the ones there are, each from <si> to </si>, are in the new part as they were, in the same order, and the new one follows them
        {
            SaveChanges changes;
            changes.sheets[0] = changesOf({text(3, 0, QStringLiteral("typed"), 3)});
            changes.newSharedStrings = {QStringLiteral("typed")};
            changes.sharedStringsBefore = 3;
            QByteArray out;
            QVERIFY2(saveXlsx(o->book, changes, &out, &why), qPrintable(why));
            const QByteArray after = entries(out).value(QStringLiteral("xl/sharedStrings.xml"));
            QByteArray expected;
            for (const QString &item : items) expected += "<si>" + item.toUtf8() + "</si>";
            QVERIFY2(after.contains(expected + "<si><t>typed</t></si></sst>"), after.constData());
            QCOMPARE(after.count("<si>"), 4);
            // read again with the runs asked for, the formatting is where it was
            auto again = std::make_unique<Opened>();
            again->bytes = out;
            Limits withRuns;
            withRuns.richText = true;
            QVERIFY2(again->package.readLazy(again->bytes, &why) && again->book.open(again->package, &why, withRuns), qPrintable(why));
            const SharedStrings &sh = again->book.sharedStrings();
            QCOMPARE(sh.strings, QVector<QString>({"bold and plain", "plain", "red under", "typed"}));
            QCOMPARE(sh.rich.size(), 2);
            QCOMPARE(sh.rich.value(0).size(), 2);
            QCOMPARE(sh.rich.value(2).size(), 2);
            QVERIFY(sh.richFonts[int(sh.rich.value(0)[0].font)].bold);
        }
    }

    // An inline string with runs of formatting that is not changed survives byte for byte in a row where another cell is (this documents what saving does too).
    void anUnchangedInlineStringWithRunsSurvivesInAnEditedRow()
    {
        const QString inlineCell = QStringLiteral("<c r=\"A1\" t=\"inlineStr\"><is><r><rPr><b/></rPr><t>bold</t></r><r><t xml:space=\"preserve\"> plain</t></r></is></c>");
        const QString otherInline = QStringLiteral("<c r=\"C1\" s=\"0\" t=\"inlineStr\"><is><t>x</t></is></c>");
        const QString rows = QStringLiteral("<row r=\"1\" spans=\"1:3\">") + inlineCell + QStringLiteral("<c r=\"B1\"><v>1</v></c>") + otherInline + QStringLiteral("</row><row r=\"2\"><c r=\"A2\"><v>2</v></c></row>");
        const QByteArray bytes = buildXlsx({worksheet(rows)});
        auto o = open(bytes);
        QVERIFY2(o->ok, qPrintable(o->why));
        SaveChanges changes;
        changes.sheets[0] = changesOf({number(0, 1, 9)});
        QByteArray out;
        QString why;
        QVERIFY2(saveXlsx(o->book, changes, &out, &why), qPrintable(why));
        const QByteArray sheet = entries(out).value(QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY2(sheet.contains(inlineCell.toUtf8()), sheet.constData());
        QVERIFY2(sheet.contains(otherInline.toUtf8()), sheet.constData());
        QVERIFY2(sheet.contains("<c r=\"B1\"><v>9</v></c>"), sheet.constData());
        QVERIFY(sheet.contains("<c r=\"A2\"><v>2</v></c>"));
        // read again, the text of the inline string is its runs one after another
        auto again = open(out);
        QVERIFY2(again->ok, qPrintable(again->why));
        Sheet read;
        QVERIFY(again->book.readSheet(0, &read));
        QCOMPARE(show(read, again->book.sharedStrings(), QStringLiteral("A1")), QStringLiteral("i:bold plain"));
        QCOMPARE(show(read, again->book.sharedStrings(), QStringLiteral("B1")), QStringLiteral("n:9"));
    }

    void stylesGetNewFormatsAtTheEnd()
    {
        const QByteArray xml = stylesXml(QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"0.0&quot; kg&quot;\"/>"),
                                         QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"164\" applyNumberFormat=\"1\"><alignment horizontal=\"center\"/></xf>"));
        SaveChanges changes;
        changes.newNumberFormats.append({165, QStringLiteral("0.0%\"")});
        changes.newCellFormats.append({1, 9});      // a copy of the centered one with percent
        changes.newCellFormats.append({2, 165});    // and of that one
        changes.cellFormatsBefore = 2;
        QByteArray out;
        QString why;
        QVERIFY2(patchStyles(xml, changes, &out, &why), qPrintable(why));
        QVERIFY(out.contains("<numFmts count=\"2\">"));
        QVERIFY(out.contains("<cellXfs count=\"4\">"));
        Styles read;
        QBuffer in(&out);
        in.open(QIODevice::ReadOnly);
        QVERIFY(readStyles(&in, Limits(), &read));
        QCOMPARE(read.formats.size(), 4);
        QCOMPARE(read.formats[2].numFmtId, 9);
        QCOMPARE(read.formats[3].numFmtId, 165);
        QVERIFY(read.formats[2].horizontal == HAlign::Center);
        QVERIFY(read.formats[3].horizontal == HAlign::Center);
        QCOMPARE(read.numFmts.value(165), QStringLiteral("0.0%\""));
        QCOMPARE(read.numFmts.value(164), QStringLiteral("0.0\" kg\""));
        // a stylesheet with no numFmts gets them put in
        const QByteArray bare = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/>"));
        SaveChanges only;
        only.newNumberFormats.append({164, QStringLiteral("0.00")});
        only.cellFormatsBefore = 1;
        QVERIFY(patchStyles(bare, only, &out, &why));
        QVERIFY(out.contains("<styleSheet xmlns=\"" + kMainNs.toUtf8() + "\"><numFmts count=\"1\"><numFmt numFmtId=\"164\" formatCode=\"0.00\"/></numFmts><cellStyleXfs"));
    }

    // Fonts, fills, borders, and cell formats that a person's formatting made are put at the ends of their lists, each cell format a copy of the one it was made from with the places of its
    // font, fill, and border, its alignment and protection, and what the file said of it besides (the style it follows) kept.
    void stylesGetNewFontsFillsBordersAndWholeFormats()
    {
        const QString lists = QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/><scheme val=\"minor\"/></font></fonts>"
                                             "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                                             "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders><cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>");
        const QByteArray xml = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" xfId=\"0\" customAttribute=\"keep\"><alignment horizontal=\"center\" justifyLastLine=\"1\"/></xf>"), lists);
        SaveChanges changes;
        FontSpec font;
        font.name = QStringLiteral("Georgia");
        font.size = 14;
        font.bold = true;
        font.underline = Underline::Double;
        font.color = ColorSpec::fromRgb(0xC00000);
        changes.newFonts.append(font);
        changes.fontsBefore = 1;
        FillSpec fill;
        fill.pattern = PatternKind::Solid;
        fill.fg = ColorSpec::fromRgb(0xFFFF00);
        changes.newFills.append(fill);
        changes.fillsBefore = 2;
        BorderSpec border;
        border.bottom.style = BorderStyle::Double;
        border.bottom.color = ColorSpec::fromRgb(0x0000FF);
        changes.newBorders.append(border);
        changes.bordersBefore = 1;
        CellFormat one;
        one.font = 1;
        one.fill = 2;
        one.border = 1;
        one.horizontal = HAlign::Right;
        one.vertical = VAlign::Top;
        one.wrap = true;
        one.indent = 3;
        one.rotation = 90;
        one.readingOrder = 2;
        one.locked = false;
        one.formulaHidden = true;
        changes.newCellFormats.append({0, 0, true, one});
        CellFormat two = one;
        two.horizontal = HAlign::General;
        two.wrap = false;
        two.indent = 0;
        two.rotation = 0;
        two.readingOrder = 0;
        two.locked = true;
        two.formulaHidden = false;
        two.vertical = VAlign::Bottom;
        changes.newCellFormats.append({1, 0, true, two});   // a copy of the first, made plain again
        changes.cellFormatsBefore = 1;
        CellStyleName good;
        good.name = QStringLiteral("Good");
        good.builtin = 26;
        CellFormat goodFormat;
        goodFormat.font = 1;
        goodFormat.fill = 2;
        changes.newStyles.append({goodFormat, good});
        changes.styleXfsBefore = 1;
        QByteArray out;
        QString why;
        QVERIFY2(patchStyles(xml, changes, &out, &why), qPrintable(why));
        QVERIFY(out.contains("<fonts count=\"2\">"));
        QVERIFY(out.contains("<fills count=\"3\">"));
        QVERIFY(out.contains("<borders count=\"2\">"));
        QVERIFY(out.contains("<cellXfs count=\"3\">"));
        QVERIFY(out.contains("<cellStyleXfs count=\"2\">"));
        QVERIFY(out.contains("<cellStyles count=\"2\">"));
        QVERIFY2(out.contains("customAttribute=\"keep\""), out.constData());   // (what the file said of the format besides stays)
        Styles read;
        QBuffer in(&out);
        in.open(QIODevice::ReadOnly);
        QVERIFY(readStyles(&in, Limits(), &read));
        QCOMPARE(read.fonts.size(), 2);
        QVERIFY(read.fonts[1] == font);
        QCOMPARE(read.fonts[0].scheme, QStringLiteral("minor"));
        QCOMPARE(read.fonts[0].family, 2);
        QCOMPARE(read.fills.size(), 3);
        QVERIFY(read.fills[2] == fill);
        QCOMPARE(read.borders.size(), 2);
        QVERIFY(read.borders[1] == border);
        QCOMPARE(read.formats.size(), 3);
        const CellFormat &a = read.formats[1], &b = read.formats[2];
        QCOMPARE(a.font, 1u);
        QCOMPARE(a.fill, 2u);
        QCOMPARE(a.border, 1u);
        QVERIFY(a.horizontal == HAlign::Right && a.vertical == VAlign::Top && a.wrap && a.indent == 3 && a.rotation == 90 && a.readingOrder == 2 && !a.locked && a.formulaHidden);
        QVERIFY(b.horizontal == HAlign::General && b.vertical == VAlign::Bottom && !b.wrap && b.indent == 0 && b.rotation == 0 && b.readingOrder == 0 && b.locked && !b.formulaHidden);
        QCOMPARE(b.font, 1u);
        QCOMPARE(read.styleXfs.size(), 2);
        QCOMPARE(read.styleXfs[1].fill, 2u);
        QCOMPARE(read.cellStyles.size(), 2);
        QCOMPARE(read.cellStyles[1].name, QStringLiteral("Good"));
        QCOMPARE(read.cellStyles[1].builtin, 26);
        QCOMPARE(read.cellStyles[1].xfId, 1u);
        QVERIFY2(out.contains("justifyLastLine=\"1\""), "an alignment attribute the program does not know stays on the format that was copied from the first");
    }

    // ---- the whole file

    void aFileWithNothingChangedIsTheSameFile()
    {
        const QByteArray bytes = realistic();
        auto o = open(bytes);
        QVERIFY2(o->ok, qPrintable(o->why));
        QByteArray out;
        QString why;
        SaveReport report;
        QVERIFY2(saveXlsx(o->book, SaveChanges(), &out, &why, &report), qPrintable(why));
        QCOMPARE(names(out), names(bytes));
        QCOMPARE(entries(out), entries(bytes));
        QCOMPARE(rawEntries(out), rawEntries(bytes));
        QCOMPARE(report.partsChanged, 0);
        QCOMPARE(report.partsCopied, int(names(bytes).size()));
    }

    // One cell changes: its sheet is rewritten, the shared strings and styles that gained something are, and every other part is the old file's, byte for byte (compressed bytes too).
    void onlyWhatChangedIsWrittenAnew()
    {
        const QByteArray bytes = realistic();
        auto o = open(bytes);
        QVERIFY2(o->ok, qPrintable(o->why));
        SaveChanges changes;
        changes.sheets[0] = changesOf({number(0, 0, 5), text(2, 0, QStringLiteral("typed"), 1)}, {resultNumber(0, 1, 6), resultNumber(1, 1, 20)});
        changes.newSharedStrings = {QStringLiteral("typed")};
        changes.sharedStringsBefore = 1;
        changes.newNumberFormats.append({165, QStringLiteral("0.0%")});
        changes.newCellFormats.append({1, 165});
        changes.cellFormatsBefore = 2;
        QByteArray out;
        QString why;
        SaveReport report;
        QVERIFY2(saveXlsx(o->book, changes, &out, &why, &report), qPrintable(why));
        const auto before = entries(bytes), after = entries(out);
        const auto rawBefore = rawEntries(bytes), rawAfter = rawEntries(out);
        // every part but the changed ones, and the three that name the calculation chain, is the same
        const QSet<QString> changed = {"xl/worksheets/sheet1.xml", "xl/sharedStrings.xml", "xl/styles.xml", "[Content_Types].xml", "xl/_rels/workbook.xml.rels"};
        for (auto it = rawBefore.constBegin(); it != rawBefore.constEnd(); ++it) {
            if (it.key() == QLatin1String("xl/calcChain.xml")) {
                QVERIFY(!rawAfter.contains(it.key()));
                continue;
            }
            QVERIFY2(rawAfter.contains(it.key()), qPrintable(it.key()));
            if (changed.contains(it.key())) QVERIFY2(after.value(it.key()) != before.value(it.key()), qPrintable(it.key()));
            else QVERIFY2(rawAfter.value(it.key()) == it.value() && after.value(it.key()) == before.value(it.key()), qPrintable(it.key()));
        }
        QCOMPARE(rawAfter.size(), rawBefore.size() - 1);
        QVERIFY(report.calcChainDropped);
        QCOMPARE(report.sheetsRewritten, 1);
        // the new file opens: the cells, strings, and formats are as they were put
        auto again = open(out);
        QVERIFY2(again->ok, qPrintable(again->why));
        Sheet sheet;
        QVERIFY(again->book.readSheet(0, &sheet));
        QCOMPARE(show(sheet, again->book.sharedStrings(), QStringLiteral("A1")), QStringLiteral("n:5"));
        QCOMPARE(show(sheet, again->book.sharedStrings(), QStringLiteral("B1")), QStringLiteral("n:6"));   // (a formula's new result)
        QCOMPARE(sheet.formulaText(*at(sheet, QStringLiteral("B1"))), QStringLiteral("A1+1"));
        QCOMPARE(show(sheet, again->book.sharedStrings(), QStringLiteral("A3")), QStringLiteral("s:typed"));
        QCOMPARE(again->book.sharedStrings().strings.size(), 2);
        QCOMPARE(again->book.styles().formats.size(), 3);
        QCOMPARE(again->book.styles().formats[2].numFmtId, 165);
        QVERIFY(!again->package.hasPart(QStringLiteral("/xl/calcChain.xml")));
        QVERIFY(again->package.relationshipsOfType(QStringLiteral("/xl/workbook.xml"), QStringLiteral("/calcChain")).isEmpty());
        // (the sheet's conditional formatting is still in it)
        QVERIFY(after.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("<conditionalFormatting sqref=\"A1:A2\">"));
    }

    // Only results changed, no cell: the chain stays (it names the formulas, and none came or went).
    void theCalculationChainStaysWhenOnlyResultsChange()
    {
        auto o = open(realistic());
        QVERIFY(o->ok);
        SaveChanges changes;
        changes.sheets[0] = changesOf({}, {resultNumber(0, 1, 6)});
        QByteArray out;
        SaveReport report;
        QVERIFY(saveXlsx(o->book, changes, &out, nullptr, &report));
        QVERIFY(!report.calcChainDropped);
        QVERIFY(names(out).contains(QStringLiteral("xl/calcChain.xml")));
        QCOMPARE(report.partsChanged, 1);
    }

    void theOtherProgramIsAskedToCalculateWhenAsked()
    {
        auto o = open(realistic());
        QVERIFY(o->ok);
        SaveChanges changes;
        changes.fullCalcOnLoad = true;
        QByteArray out;
        QVERIFY(saveXlsx(o->book, changes, &out));
        auto again = open(out);
        QVERIFY(again->ok);
        QVERIFY(again->book.info().fullCalcOnLoad);
        // a workbook part with no calcPr gets one
        QByteArray xml = "<workbook xmlns=\"x\"><sheets><sheet name=\"a\" sheetId=\"1\"/></sheets><definedNames><definedName name=\"n\">A1</definedName></definedNames><oleSize ref=\"A1\"/></workbook>", patched;
        QVERIFY(setFullCalcOnLoad(xml, &patched));
        QVERIFY(patched.contains("</definedNames><calcPr fullCalcOnLoad=\"1\"/><oleSize"));
    }

    // The new file replaces the old only when it is whole: a failed write leaves the old file as it was.
    void theNewFileReplacesTheOldOnlyWhenWhole()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("book.xlsx"));
        QFile old(path);
        QVERIFY(old.open(QIODevice::WriteOnly));
        old.write("the old file");
        old.close();
        QString why;
        QVERIFY2(writeFileAtomically(path, QByteArray("the new file"), &why), qPrintable(why));
        QFile now(path);
        QVERIFY(now.open(QIODevice::ReadOnly));
        QCOMPARE(now.readAll(), QByteArray("the new file"));
        now.close();
        // no stray file is left beside it
        QCOMPARE(QDir(dir.path()).entryList(QDir::Files).size(), 1);
        // a place that cannot be written to: false, with a message, and nothing made
        QVERIFY(!writeFileAtomically(dir.filePath(QStringLiteral("no/such/folder/book.xlsx")), QByteArray("x"), &why));
        QVERIFY(!why.isEmpty());
        // a file that is there is replaced whole, however long it was (and a longer one by a shorter)
        QVERIFY2(writeFileAtomically(path, QByteArray("short"), &why), qPrintable(why));
        QVERIFY(now.open(QIODevice::ReadOnly));
        QCOMPARE(now.readAll(), QByteArray("short"));
        now.close();
        QVERIFY2(writeFileAtomically(path, QByteArray(300000, 'x'), &why), qPrintable(why));
        QCOMPARE(QFileInfo(path).size(), qint64(300000));
        // a file that is there and cannot be written (read-only): false, with a message, the file as it was, and no stray file
        QVERIFY(QFile::setPermissions(path, QFileDevice::ReadOwner));
        if (!QFileInfo(path).isWritable()) {
            why.clear();
            QVERIFY(!writeFileAtomically(path, QByteArray("y"), &why));
            QVERIFY(!why.isEmpty());
            QCOMPARE(QFileInfo(path).size(), qint64(300000));
            QCOMPARE(QDir(dir.path()).entryList(QDir::Files).size(), 1);
        }
        QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    }
};

JP_TEST_CLASS(SheetSaveTests)
#include "tst_sheet_save.moc"
