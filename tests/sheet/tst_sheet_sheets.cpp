// Tests of the sheets of a workbook as a person changes them (Calculation::addSheet, setSheetNames, rewriteSheetReferences, and the sheet list of the workbook): a sheet added, copied,
// renamed, and deleted, every reference to it following (or being #REF!), the places of the other sheets never changing, and all of it put back by the undo calls. Every workbook is built here.
#include "sheethelpers.h"

#include "sheet/calculation.h"

#include <memory>

namespace {

QString cell(const QString &ref, const QString &value) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref, value); }
QString formula(const QString &ref, const QString &text, const QString &value = QStringLiteral("0"))
{
    return QStringLiteral("<c r=\"%1\"><f>%2</f><v>%3</v></c>").arg(ref, text.toHtmlEscaped(), value);
}
QString row(int n, const QString &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(n).arg(cells); }

//   Sheet1: A1 = 1, B1 = Sheet2!A1 + 1, C1 = SUM(Sheet2!A1:A2)
//   Sheet2: A1 = 10, A2 = Sheet1!A1 * 2
//   Sheet3: A1 = "x" is a number 7; B1 = 'Sheet 3'... (the name has no space here: Sheet3), C1 = Sheet1!A1 + Sheet3!A1
struct Book {
    Package package;
    Workbook book;
    std::unique_ptr<Calculation> calc;
    bool open()
    {
        const QByteArray one = worksheet(row(1, cell(QStringLiteral("A1"), QStringLiteral("1")) + formula(QStringLiteral("B1"), QStringLiteral("Sheet2!A1+1")) + formula(QStringLiteral("C1"), QStringLiteral("SUM(Sheet2!A1:A2)"))));
        const QByteArray two = worksheet(row(1, cell(QStringLiteral("A1"), QStringLiteral("10"))) + row(2, formula(QStringLiteral("A2"), QStringLiteral("Sheet1!A1*2"))));
        const QByteArray three = worksheet(row(1, cell(QStringLiteral("A1"), QStringLiteral("7")) + formula(QStringLiteral("C1"), QStringLiteral("Sheet1!A1+Sheet3!A1"))));
        QString why;
        if (!package.readLazy(buildXlsx({one, two, three}), &why) || !book.open(package, &why)) return false;
        calc.reset(new Calculation(book));
        return calc->load();
    }
    void work() { while (!calc->recalculate().finished) {} }
    QString value(int sheet, const QString &ref) const
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return calc->cell(sheet, r, c).debug();
    }
    QString text(int sheet, const QString &ref) const
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        const Cell *cellp = calc->sheet(sheet).cells.find(r, c);
        return cellp ? calc->sheet(sheet).formulaText(*cellp) : QStringLiteral("-");
    }
};

} // namespace

class SheetSheetsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The workbook's list of sheets: each has the name the file gave it, the tabs are in the file's order, and nothing is new or deleted.
    void theListBeginsAsTheFileHasIt()
    {
        Book b;
        QVERIFY(b.open());
        const WorkbookInfo &info = b.book.info();
        QCOMPARE(info.sheets.size(), 3);
        QCOMPARE(info.order, QVector<int>({0, 1, 2}));
        for (int i = 0; i < 3; ++i) {
            QCOMPARE(info.sheets[i].fileName, info.sheets[i].name);
            QVERIFY(!info.sheets[i].deleted && !info.sheets[i].isNew);
        }
        QCOMPARE(info.sheets[1].name, QStringLiteral("Sheet2"));
    }

    // Renaming: the formulas that name the sheet name the new name, every one of them works as before, and what puts it back puts back the texts and the values.
    void renamingASheetRewritesWhatNamesIt()
    {
        Book b;
        QVERIFY(b.open());
        b.work();
        QCOMPARE(b.value(0, QStringLiteral("B1")), QStringLiteral("n:11"));
        QCOMPARE(b.value(0, QStringLiteral("C1")), QStringLiteral("n:12"));
        b.calc->setSheetNames(1, QStringLiteral("Sheet2"), QStringLiteral("Data Sheet"));
        b.book.setSheetName(1, QStringLiteral("Data Sheet"));
        Calculation::SheetRewrite record;
        QVERIFY(b.calc->rewriteSheetReferences(QStringLiteral("Sheet2"), QStringLiteral("Data Sheet"), &record));
        b.work();
        QCOMPARE(b.text(0, QStringLiteral("B1")), QStringLiteral("'Data Sheet'!A1+1"));
        QCOMPARE(b.text(0, QStringLiteral("C1")), QStringLiteral("SUM('Data Sheet'!A1:A2)"));
        QCOMPARE(b.text(2, QStringLiteral("C1")), QStringLiteral("Sheet1!A1+Sheet3!A1"));   // (what names other sheets is as it was)
        QCOMPARE(b.value(0, QStringLiteral("B1")), QStringLiteral("n:11"));
        QCOMPARE(b.value(0, QStringLiteral("C1")), QStringLiteral("n:12"));
        QCOMPARE(b.calc->sheetIndex(QStringLiteral("data sheet")), 1);
        QCOMPARE(b.calc->sheetIndex(QStringLiteral("Sheet2")), -1);
        QCOMPARE(int(record.rewritten.size()), 2);
        // a formula typed afterwards names the sheet by its new name, and one that names the old name reads nothing
        QVERIFY(b.calc->setFormula(2, 2, 0, QStringLiteral("'Data Sheet'!A1*3")));
        b.work();
        QCOMPARE(b.value(2, QStringLiteral("A3")), QStringLiteral("n:30"));
        QVERIFY(b.calc->setFormula(2, 3, 0, QStringLiteral("Sheet2!A1")));
        b.work();
        QVERIFY(b.value(2, QStringLiteral("A4")).startsWith(QLatin1String("e:")));
        b.calc->clearCell(2, 2, 0);
        b.calc->clearCell(2, 3, 0);
        // back
        b.calc->setSheetNames(1, QStringLiteral("Data Sheet"), QStringLiteral("Sheet2"));
        b.book.setSheetName(1, QStringLiteral("Sheet2"));
        b.calc->undoSheetRewrite(record);
        b.work();
        QCOMPARE(b.text(0, QStringLiteral("B1")), QStringLiteral("Sheet2!A1+1"));
        QCOMPARE(b.text(0, QStringLiteral("C1")), QStringLiteral("SUM(Sheet2!A1:A2)"));
        QCOMPARE(b.value(0, QStringLiteral("B1")), QStringLiteral("n:11"));
        // a change to a cell the formulas read still reaches them (the graph was told again what they read)
        QVERIFY(b.calc->setValue(1, 0, 0, Value::number(20)));
        b.work();
        QCOMPARE(b.value(0, QStringLiteral("B1")), QStringLiteral("n:21"));
        QCOMPARE(b.value(0, QStringLiteral("C1")), QStringLiteral("n:22"));
        QCOMPARE(b.book.info().sheets[1].fileName, QStringLiteral("Sheet2"));
    }

    // Deleting: the formulas that read the sheet have #REF! for it (so they have the error), the other sheets keep their places and what they say, and Undo gives it all back.
    void deletingASheetMakesRefsOfWhatReadsIt()
    {
        Book b;
        QVERIFY(b.open());
        b.work();
        b.calc->setSheetNames(1, QStringLiteral("Sheet2"), QString());
        Calculation::SheetRewrite record;
        QVERIFY(b.calc->rewriteSheetReferences(QStringLiteral("Sheet2"), QString(), &record));
        b.book.setSheetDeleted(1, true);
        b.book.setSheetOrder({0, 2});
        b.work();
        QCOMPARE(b.text(0, QStringLiteral("B1")), QStringLiteral("#REF!+1"));
        QCOMPARE(b.text(0, QStringLiteral("C1")), QStringLiteral("SUM(#REF!)"));
        QVERIFY(b.value(0, QStringLiteral("B1")).startsWith(QLatin1String("e:")));
        QCOMPARE(b.value(0, QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(b.value(2, QStringLiteral("C1")), QStringLiteral("n:8"));   // (the third sheet is where it was: its number is 2)
        QCOMPARE(b.calc->sheetCount(), 3);
        QCOMPARE(b.book.info().order, QVector<int>({0, 2}));
        // a name nothing has now can be a new sheet's
        const int again = b.calc->addSheet(QStringLiteral("Sheet2"));
        b.book.addSheetEntry(QStringLiteral("Sheet2"));
        QCOMPARE(again, 3);
        QCOMPARE(b.calc->sheetIndex(QStringLiteral("Sheet2")), 3);
        QVERIFY(b.calc->setValue(3, 0, 0, Value::number(5)));
        QVERIFY(b.calc->setFormula(2, 4, 0, QStringLiteral("Sheet2!A1+1")));
        b.work();
        QCOMPARE(b.value(2, QStringLiteral("A5")), QStringLiteral("n:6"));
        b.calc->clearCell(2, 4, 0);
        b.calc->removeLastSheet();
        b.book.removeLastSheetEntry();
        QCOMPARE(b.calc->sheetCount(), 3);
        // Undo
        b.book.setSheetDeleted(1, false);
        b.book.setSheetOrder({0, 1, 2});
        b.calc->setSheetNames(1, QString(), QStringLiteral("Sheet2"));
        b.calc->undoSheetRewrite(record);
        b.work();
        QCOMPARE(b.text(0, QStringLiteral("B1")), QStringLiteral("Sheet2!A1+1"));
        QCOMPARE(b.value(0, QStringLiteral("B1")), QStringLiteral("n:11"));
        QCOMPARE(b.value(0, QStringLiteral("C1")), QStringLiteral("n:12"));
        QVERIFY(b.calc->setValue(1, 0, 0, Value::number(100)));
        b.work();
        QCOMPARE(b.value(0, QStringLiteral("B1")), QStringLiteral("n:101"));   // (the formulas read it again)
    }

    // A sheet added is last in place and its number is its own for good; one copied has the cells, formulas (that read their own sheet's cells), formats, sizes, merges, and rules of
    // the one it is a copy of, and works out on its own.
    void aSheetIsAddedAndCopied()
    {
        Book b;
        QVERIFY(b.open());
        b.work();
        const int empty = b.calc->addSheet(QStringLiteral("New"));
        const int entry = b.book.addSheetEntry(QStringLiteral("New"));
        QCOMPARE(empty, 3);
        QCOMPARE(entry, 3);
        QVERIFY(b.book.info().sheets[3].isNew);
        QVERIFY(b.book.info().sheets[3].fileName.isEmpty());
        b.book.setSheetOrder({0, 1, 3, 2});   // (inserted before the third)
        QCOMPARE(b.calc->sheetCount(), 4);
        QCOMPARE(b.calc->sheet(3).cells.cellCount(), qint64(0));
        // a formula on another sheet can read it
        QVERIFY(b.calc->setValue(3, 0, 0, Value::number(3)));
        QVERIFY(b.calc->setFormula(0, 5, 0, QStringLiteral("New!A1*2")));
        b.work();
        QCOMPARE(b.value(0, QStringLiteral("A6")), QStringLiteral("n:6"));
        b.calc->clearCell(0, 5, 0);
        // a copy of the second sheet, whose A2 reads Sheet1!A1 and whose A1 is a constant
        const int copy = b.calc->addSheet(QStringLiteral("Sheet2 (2)"), 1);
        b.book.addSheetEntry(QStringLiteral("Sheet2 (2)"));
        QCOMPARE(copy, 4);
        b.work();
        QCOMPARE(b.value(copy, QStringLiteral("A1")), QStringLiteral("n:10"));
        QCOMPARE(b.value(copy, QStringLiteral("A2")), QStringLiteral("n:2"));
        QCOMPARE(b.text(copy, QStringLiteral("A2")), QStringLiteral("Sheet1!A1*2"));
        // the copy is its own: a change to it does not change the sheet it came from
        QVERIFY(b.calc->setValue(copy, 0, 0, Value::number(99)));
        b.work();
        QCOMPARE(b.value(1, QStringLiteral("A1")), QStringLiteral("n:10"));
        QCOMPARE(b.value(0, QStringLiteral("B1")), QStringLiteral("n:11"));
        // and a change to what both read reaches both
        QVERIFY(b.calc->setValue(0, 0, 0, Value::number(4)));
        b.work();
        QCOMPARE(b.value(1, QStringLiteral("A2")), QStringLiteral("n:8"));
        QCOMPARE(b.value(copy, QStringLiteral("A2")), QStringLiteral("n:8"));
        // away again, last first
        b.calc->removeLastSheet();
        b.book.removeLastSheetEntry();
        b.calc->removeLastSheet();
        b.book.removeLastSheetEntry();
        QCOMPARE(b.calc->sheetCount(), 3);
        QCOMPARE(b.book.info().sheets.size(), 3);
        QCOMPARE(b.calc->sheetIndex(QStringLiteral("New")), -1);
        QVERIFY(b.calc->setValue(0, 0, 0, Value::number(6)));
        b.work();
        QCOMPARE(b.value(1, QStringLiteral("A2")), QStringLiteral("n:12"));
    }

    // A workbook with a formula over a block of sheets is not given to renaming or deleting (the meaning of such a formula depends on the order of the sheets).
    void aBlockOfSheetsIsNotRewritten()
    {
        Book b;
        QVERIFY(b.open());
        QVERIFY(!b.calc->hasThreeDReferences());
        QVERIFY(b.calc->setFormula(0, 7, 0, QStringLiteral("SUM(Sheet1:Sheet3!A1)")));
        b.work();
        QVERIFY(b.calc->hasThreeDReferences());
        Calculation::SheetRewrite record;
        QVERIFY(!b.calc->rewriteSheetReferences(QStringLiteral("Sheet2"), QStringLiteral("X"), &record));
        QVERIFY(record.texts.empty());
    }
};

JP_TEST_CLASS(SheetSheetsTests)
#include "tst_sheet_sheets.moc"
