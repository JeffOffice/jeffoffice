// Tests of the saver's own rewriting of formulas (docs/jeffcrunch-rowids.md, round 7): the edits made to rows and columns are replayed on the order of the file's lines, and the formulas of the file
// get the text the edits give them without the workbook listing them. The workbooks are built in memory.
#include "sheethelpers.h"

#include "sheet/linereplay.h"
#include "sheet/refshift.h"
#include "sheet/structsave.h"
#include "sheet/xlsxsave.h"

using namespace jp::office::sheet;
using namespace sheettest;

namespace {

// (a generator of its own: the same numbers on every machine)
struct Random {
    quint64 state;
    explicit Random(quint64 seed) : state(seed * 2862933555777941757ull + 3037000493ull) {}
    quint32 next(quint32 below)
    {
        state = state * 6364136223846793005ull + 1442695040888963407ull;
        return quint32((state >> 33) % below);
    }
};

struct Saved {
    QMap<QString, QByteArray> parts;
    QString error;
};

Saved saveWith(const QByteArray &xlsx, const std::vector<StructEdit> &edits, bool rewrite)
{
    Saved out;
    Package package;
    if (!package.readLazy(xlsx, &out.error)) return out;
    Workbook book;
    if (!book.open(package, &out.error)) return out;
    SaveChanges changes;
    changes.edits = edits;
    changes.rewriteFormulas = rewrite;
    QByteArray bytes;
    if (!saveXlsx(book, changes, &bytes, &out.error, nullptr)) return out;
    jp::readZip(bytes, out.parts);
    return out;
}

// The element of a cell, `<c ...>...</c>`, in a sheet's XML ("" if there is none).
QString cellElement(const QString &xml, const QString &ref)
{
    const int at = xml.indexOf(QStringLiteral("<c r=\"%1\"").arg(ref));
    if (at < 0) return {};
    const int close = xml.indexOf(QStringLiteral("</c>"), at);
    const int selfClose = xml.indexOf(QStringLiteral("/>"), at);
    if (close < 0 || (selfClose >= 0 && selfClose < xml.indexOf(QLatin1Char('>'), at))) return xml.mid(at, selfClose + 2 - at);
    return xml.mid(at, close + 4 - at);
}

// The text of the formula in the cell, as the file writes it (unescaped): "" if the cell has none.
QString formulaIn(const QString &xml, const QString &ref)
{
    const QString cell = cellElement(xml, ref);
    const int open = cell.indexOf(QStringLiteral("<f"));
    if (open < 0) return {};
    const int gt = cell.indexOf(QLatin1Char('>'), open);
    if (gt < 0 || cell[gt - 1] == QLatin1Char('/')) return {};
    const int close = cell.indexOf(QStringLiteral("</f>"), gt);
    QString text = cell.mid(gt + 1, close - gt - 1);
    text.replace(QStringLiteral("&lt;"), QStringLiteral("<")).replace(QStringLiteral("&gt;"), QStringLiteral(">")).replace(QStringLiteral("&amp;"), QStringLiteral("&"));
    return text;
}

// What the edits one after another make of a formula (the rules the workbook's own rewriting uses).
QString oneByOne(QString text, int ownSheet, const QStringList &names, const std::vector<StructEdit> &edits)
{
    for (const StructEdit &e : edits) text = adjustFormula(text, ownSheet, names, e);
    return text;
}

} // namespace

class SheetLineReplayTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A formula's text after the edits is the text the edits one after another make of it, for formulas of every kind of reference and lists of rows and columns inserted and deleted on two sheets.
    void aFormulaIsRewrittenAsTheEditsOneByOneRewriteIt()
    {
        const QStringList names = {QStringLiteral("Data"), QStringLiteral("Calc")};
        const QStringList formulas = {
            QStringLiteral("A1"), QStringLiteral("$B$2"), QStringLiteral("B$3*$C4"), QStringLiteral("SUM(A1:A10)"), QStringLiteral("SUM(A$2:C$6)"), QStringLiteral("SUM(B:B)"), QStringLiteral("SUM(3:5)"),
            QStringLiteral("Calc!C3+Data!A5"), QStringLiteral("SUM('Calc'!A1:B3)"), QStringLiteral("IF(A5>0,A6,A7)"), QStringLiteral("A2:C4"), QStringLiteral("Data!A1:A4"), QStringLiteral("SUM(Calc!B2:D9)*2"),
            QStringLiteral("A10+B10+C10-D10"), QStringLiteral("COUNT(A1:Z1)"), QStringLiteral("SUM(A5:A5)"), QStringLiteral("Calc!A:C"), QStringLiteral("SUM(Data!2:4)"), QStringLiteral("MAX(C2:C30,Calc!E5)"),
        };
        for (quint32 seed = 1; seed <= 400; ++seed) {
            Random r(seed);
            std::vector<StructEdit> edits;
            const int n = 1 + int(r.next(7));
            for (int i = 0; i < n; ++i) {
                const int sheet = int(r.next(2));
                const quint32 count = 1 + r.next(3);
                switch (r.next(4)) {
                case 0: edits.push_back(StructEdit::insertRows(sheet, r.next(14), count)); break;
                case 1: edits.push_back(StructEdit::deleteRows(sheet, r.next(14), count)); break;
                case 2: edits.push_back(StructEdit::insertColumns(sheet, r.next(6), count)); break;
                default: edits.push_back(StructEdit::deleteColumns(sheet, r.next(6), count)); break;
                }
            }
            const LineReplay replay(edits, names);
            QVERIFY(replay.ok());
            for (const QString &f : formulas)
                for (int own = 0; own < 2; ++own) {
                    const QString want = oneByOne(f, own, names, edits);
                    const QString got = replay.rewrite(f, own);
                    QVERIFY2(got == want, qPrintable(QStringLiteral("seed %1, %2 on sheet %3: replay '%4', one by one '%5'").arg(seed).arg(f).arg(own).arg(got, want)));
                }
        }
    }

    // Rows put in and taken out again leave every row that can hold anything where the file had it (the lines pushed off the end of the sheet by the insert are not back): no formula is rewritten.
    void rowsPutInAndTakenOutAgainMoveNothing()
    {
        const QStringList names = {QStringLiteral("Data")};
        const LineReplay replay({StructEdit::insertRows(0, 3, 2), StructEdit::deleteRows(0, 3, 2)}, names);
        QVERIFY(replay.ok());
        QCOMPARE(replay.rewrite(QStringLiteral("SUM(A1:A9)"), 0), QStringLiteral("SUM(A1:A9)"));
        quint32 now = 0;
        QVERIFY(replay.rowNow(0, 3, &now));
        QCOMPARE(now, 3u);
        const LineReplay moved({StructEdit::insertRows(0, 3, 2)}, names);
        QVERIFY(moved.movesAnything());
        QCOMPARE(moved.movedSheetNames(), QStringList{QStringLiteral("Data")});
        QVERIFY(moved.rowNow(0, 3, &now));
        QCOMPARE(now, 5u);
        QVERIFY(moved.rowNow(0, 2, &now));
        QCOMPARE(now, 2u);
        const LineReplay gone({StructEdit::deleteRows(0, 3, 2)}, names);
        QVERIFY(!gone.rowNow(0, 4, &now));
        QVERIFY(gone.rowNow(0, 5, &now));
        QCOMPARE(now, 3u);
    }

    // A reference through several sheets is left as it is, a name that is not a cell is not touched, and a sheet of another workbook is not a sheet of this one.
    void whatIsNotACellOfASheetThatMovedIsLeftAlone()
    {
        const QStringList names = {QStringLiteral("Data"), QStringLiteral("Calc")};
        const LineReplay replay({StructEdit::insertRows(0, 0, 1)}, names);
        QCOMPARE(replay.rewrite(QStringLiteral("SUM(Data:Calc!A1)"), 1), QStringLiteral("SUM(Data:Calc!A1)"));
        QCOMPARE(replay.rewrite(QStringLiteral("MyName+Calc!A1"), 0), QStringLiteral("MyName+Calc!A1"));
        QCOMPARE(replay.rewrite(QStringLiteral("[2]Data!A1+Other!A1"), 1), QStringLiteral("[2]Data!A1+Other!A1"));
        QCOMPARE(replay.rewrite(QStringLiteral("Data!A1"), 1), QStringLiteral("Data!A2"));
        QCOMPARE(replay.rewrite(QStringLiteral("DATA!a1"), 1), QStringLiteral("DATA!A2"));
    }

    // ---- the saver

    // A sheet's rows, one formula of each kind: a plain one, a volatile one, a shared family, an array, and a data table; and a sheet that reads the first.
    static QByteArray book()
    {
        const QString rows = QStringLiteral(
            "<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f t=\"shared\" ref=\"B1:B6\" si=\"0\">A1*2</f><v>2</v></c><c r=\"C1\"><f ca=\"1\">NOW()+A5</f><v>45000</v></c><c r=\"D1\"><f>SUM(A1:A6)</f><v>21</v></c></row>"
            "<row r=\"2\"><c r=\"A2\"><v>2</v></c><c r=\"B2\"><f t=\"shared\" si=\"0\"/><v>4</v></c></row>"
            "<row r=\"3\"><c r=\"A3\"><v>3</v></c><c r=\"B3\"><f t=\"shared\" si=\"0\"/><v>6</v></c><c r=\"C3\"><f t=\"array\" ref=\"C3:C4\">A3:A4*2</f><v>6</v></c></row>"
            "<row r=\"4\"><c r=\"A4\"><v>4</v></c><c r=\"B4\"><f t=\"shared\" si=\"0\"/><v>8</v></c><c r=\"C4\"><v>8</v></c></row>"
            "<row r=\"5\"><c r=\"A5\"><v>5</v></c><c r=\"B5\"><f t=\"shared\" si=\"0\"/><v>10</v></c></row>"
            "<row r=\"6\"><c r=\"A6\"><v>6</v></c><c r=\"B6\"><f t=\"shared\" si=\"0\"/><v>12</v></c><c r=\"F6\"><f t=\"dataTable\" ref=\"F6:F7\" r1=\"A6\"/><v>5</v></c></row>"
            "<row r=\"7\"><c r=\"F7\"><v>8</v></c></row>");
        const QString reader = QStringLiteral("<row r=\"1\"><c r=\"A1\"><f>Sheet1!A4+Sheet1!B5</f><v>18</v></c><c r=\"B1\"><f>SUM(Sheet1!A1:A6)</f><v>21</v></c></row><row r=\"2\"><c r=\"A2\"><f>A1*2</f><v>36</v></c></row>");
        const QString alone = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>3</v></c><c r=\"B1\"><f>A1+1</f><v>4</v></c></row>");
        return buildXlsx({worksheet(rows), worksheet(reader), worksheet(alone)});
    }

    // A row inserted inside a shared family: the members above the row read what they read, those below read the rows that moved (the family is cut where the row is put in); the formulas that read
    // cells of the other sheet follow, and a formula that reads nothing that moved keeps the bytes of its cell (its volatile mark, its result).
    void aSharedFamilyCutByAnInsertIsWrittenWithTheTextOfEachMember()
    {
        const Saved s = saveWith(book(), {StructEdit::insertRows(0, 3, 1)}, true);   // before row 4
        QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
        const QString one = QString::fromUtf8(s.parts.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        for (const char *cell : {"B1", "B2", "B3"}) QVERIFY2(formulaIn(one, QLatin1String(cell)) == QStringLiteral("A%1*2").arg(QLatin1String(cell).mid(1)), qPrintable(one));
        QCOMPARE(formulaIn(one, QStringLiteral("B5")), QStringLiteral("A5*2"));   // (was B4: its row moved, and so did the row it reads)
        QCOMPARE(formulaIn(one, QStringLiteral("B6")), QStringLiteral("A6*2"));
        QCOMPARE(formulaIn(one, QStringLiteral("B7")), QStringLiteral("A7*2"));
        QCOMPARE(formulaIn(one, QStringLiteral("D1")), QStringLiteral("SUM(A1:A7)"));   // (a range with a row put in it grows)
        QCOMPARE(formulaIn(one, QStringLiteral("C1")), QStringLiteral("NOW()+A6"));
        QVERIFY2(cellElement(one, QStringLiteral("C1")).contains(QStringLiteral("<f ca=\"1\">")) && cellElement(one, QStringLiteral("C1")).contains(QStringLiteral("<v>45000</v>")), qPrintable(one));
        QVERIFY2(cellElement(one, QStringLiteral("D1")).contains(QStringLiteral("<v>21</v>")), qPrintable(one));   // (the result the file had stays: the worker's changes are patched in by the window)
        QVERIFY2(cellElement(one, QStringLiteral("C3")).contains(QStringLiteral("t=\"array\" ref=\"C3:C5\"")), qPrintable(one));   // (the row put in is inside the array)
        QCOMPARE(formulaIn(one, QStringLiteral("C3")), QStringLiteral("A3:A5*2"));
        QVERIFY2(cellElement(one, QStringLiteral("F7")).contains(QStringLiteral("t=\"dataTable\" ref=\"F7:F8\"")), qPrintable(one));
        QVERIFY(!one.contains(QStringLiteral("TABLE(")));
        const QString two = QString::fromUtf8(s.parts.value(QStringLiteral("xl/worksheets/sheet2.xml")));
        QCOMPARE(formulaIn(two, QStringLiteral("A1")), QStringLiteral("Sheet1!A5+Sheet1!B6"));   // (a formula of another sheet reads the cells that moved)
        QCOMPARE(formulaIn(two, QStringLiteral("B1")), QStringLiteral("SUM(Sheet1!A1:A7)"));
        QCOMPARE(formulaIn(two, QStringLiteral("A2")), QStringLiteral("A1*2"));
        // the sheet that reads nothing that moved is the file's, byte for byte
        QMap<QString, QByteArray> original;
        jp::readZip(book(), original);
        QCOMPARE(s.parts.value(QStringLiteral("xl/worksheets/sheet3.xml")), original.value(QStringLiteral("xl/worksheets/sheet3.xml")));
    }

    // The master's row of a family deleted: the members that are left have the formula, each reading its own row.
    void aFamilyWhoseMastersRowIsDeletedKeepsItsMembers()
    {
        const Saved s = saveWith(book(), {StructEdit::deleteRows(0, 0, 1)}, true);   // the first row, where B1 holds the text
        QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
        const QString one = QString::fromUtf8(s.parts.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        QVERIFY2(!one.contains(QStringLiteral("shared")), qPrintable(one));
        for (int row = 1; row <= 5; ++row) QCOMPARE(formulaIn(one, QStringLiteral("B%1").arg(row)), QStringLiteral("A%1*2").arg(row));
        QCOMPARE(formulaIn(one, QStringLiteral("D1")), QString());   // (the formula was in the row that is gone)
        const QString two = QString::fromUtf8(s.parts.value(QStringLiteral("xl/worksheets/sheet2.xml")));
        QCOMPARE(formulaIn(two, QStringLiteral("A1")), QStringLiteral("Sheet1!A3+Sheet1!B4"));
        QCOMPARE(formulaIn(two, QStringLiteral("B1")), QStringLiteral("SUM(Sheet1!A1:A5)"));
    }

    // A family that moves as one (a row put in above it, a column in front of it): every member reads what it read, in the new places, whatever the number of the edits.
    void aFamilyThatMovesAsOneKeepsItsFormula()
    {
        const Saved s = saveWith(book(), {StructEdit::insertRows(0, 0, 2), StructEdit::insertColumns(0, 0, 1), StructEdit::insertRows(0, 0, 1)}, true);
        QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
        const QString one = QString::fromUtf8(s.parts.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        for (int row = 1; row <= 6; ++row) QCOMPARE(formulaIn(one, QStringLiteral("C%1").arg(row + 3)), QStringLiteral("B%1*2").arg(row + 3));
        QCOMPARE(formulaIn(one, QStringLiteral("E4")), QStringLiteral("SUM(B4:B9)"));
        QCOMPARE(formulaIn(one, QStringLiteral("D4")), QStringLiteral("NOW()+B8"));
    }

    // A column deleted: a formula that read it has #REF!, a range that lost a column shrinks, and every other formula keeps its text.
    void aDeletedColumnLeavesRefAndShrinksRanges()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c><c r=\"C1\"><v>3</v></c><c r=\"D1\"><f>B1+C1</f><v>5</v></c><c r=\"E1\"><f>SUM(A1:C1)</f><v>6</v></c><c r=\"F1\"><f>A1</f><v>1</v></c></row>");
        const Saved s = saveWith(buildXlsx({worksheet(rows)}), {StructEdit::deleteColumns(0, 1, 1)}, true);
        QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
        const QString one = QString::fromUtf8(s.parts.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        QCOMPARE(formulaIn(one, QStringLiteral("C1")), QStringLiteral("#REF!+B1"));
        QCOMPARE(formulaIn(one, QStringLiteral("D1")), QStringLiteral("SUM(A1:B1)"));
        QCOMPARE(formulaIn(one, QStringLiteral("E1")), QStringLiteral("A1"));
        QVERIFY2(cellElement(one, QStringLiteral("E1")).contains(QStringLiteral("<v>1</v>")), qPrintable(one));
    }

    // Without the flag the saver does what it did: no formula is given another text (the workbook lists the ones that changed), and a shared formula is written out.
    void withoutTheFlagNoFormulaIsRewritten()
    {
        const Saved s = saveWith(book(), {StructEdit::insertRows(0, 3, 1)}, false);
        QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
        const QString one = QString::fromUtf8(s.parts.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        QCOMPARE(formulaIn(one, QStringLiteral("D1")), QStringLiteral("SUM(A1:A6)"));
        QCOMPARE(formulaIn(one, QStringLiteral("B5")), QStringLiteral("A4*2"));
        const QString two = QString::fromUtf8(s.parts.value(QStringLiteral("xl/worksheets/sheet2.xml")));
        QCOMPARE(formulaIn(two, QStringLiteral("A1")), QStringLiteral("Sheet1!A4+Sheet1!B5"));
    }

    // A sheet that names a sheet that moved but reads nothing that moved is the file's, byte for byte (its shared formula stays shared); one that reads a row that moved is written.
    void aSheetThatNamesAMovedSheetButReadsNothingThatMovedIsCopied()
    {
        QMap<QString, QByteArray> original;
        jp::readZip(book(), original);
        const Saved far = saveWith(book(), {StructEdit::insertRows(0, 30, 1)}, true);   // below every row the other sheet reads
        QVERIFY2(far.error.isEmpty(), qPrintable(far.error));
        QCOMPARE(far.parts.value(QStringLiteral("xl/worksheets/sheet2.xml")), original.value(QStringLiteral("xl/worksheets/sheet2.xml")));
        QVERIFY(far.parts.value(QStringLiteral("xl/worksheets/sheet1.xml")) != original.value(QStringLiteral("xl/worksheets/sheet1.xml")));   // (the sheet edited is renumbered)
        const Saved near = saveWith(book(), {StructEdit::insertRows(0, 0, 1)}, true);
        QVERIFY2(near.error.isEmpty(), qPrintable(near.error));
        QVERIFY(near.parts.value(QStringLiteral("xl/worksheets/sheet2.xml")) != original.value(QStringLiteral("xl/worksheets/sheet2.xml")));
        QCOMPARE(formulaIn(QString::fromUtf8(near.parts.value(QStringLiteral("xl/worksheets/sheet2.xml"))), QStringLiteral("A1")), QStringLiteral("Sheet1!A5+Sheet1!B6"));
    }

    // A sheet that names no sheet that moved is not read for its cells: the part is the file's, though the sheet it reads from is edited, and a name that is not plain text is looked for everywhere.
    void aSheetThatNamesNoMovedSheetIsCopied()
    {
        QMap<QString, QByteArray> original;
        jp::readZip(book(), original);
        const Saved s = saveWith(book(), {StructEdit::insertRows(2, 0, 1)}, true);   // the third sheet
        QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
        QCOMPARE(s.parts.value(QStringLiteral("xl/worksheets/sheet1.xml")), original.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        QCOMPARE(s.parts.value(QStringLiteral("xl/worksheets/sheet2.xml")), original.value(QStringLiteral("xl/worksheets/sheet2.xml")));
        QVERIFY(formulaIn(QString::fromUtf8(s.parts.value(QStringLiteral("xl/worksheets/sheet3.xml"))), QStringLiteral("B2")) == QStringLiteral("A2+1"));
    }
};

JP_TEST_CLASS(SheetLineReplayTests)
#include "tst_sheet_linereplay.moc"
