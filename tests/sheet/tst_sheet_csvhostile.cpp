// Tests of the reader of delimited text on files made to do harm: lines without end, a million columns, quotes never closed, more rows than a sheet has, bytes that are nothing,
// and more cells than the budget. Each ends with what could be read and a message in words.
#include "odshelpers.h"

#include "sheet/csv.h"

#include <QRandomGenerator>

class SheetCsvHostileTests : public QObject {
    Q_OBJECT

    static bool openCsv(const QByteArray &bytes, Workbook *wb, QString *why, const Limits &limits = Limits()) { return openBytes(bytes, QStringLiteral("hostile.csv"), wb, why, limits); }

    QLocale m_before;

private Q_SLOTS:
    // The language is set for every test, whatever the system's is: the order in which delimiters are tried and the reading of numbers follow the decimal mark, and the answers must
    // not (the suite gives the same ones under C.UTF-8, en_US, and de_DE).
    void init()
    {
        m_before = QLocale();
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    }

    void cleanup() { QLocale::setDefault(m_before); }

    // A line of a million columns: the first 16,384 are read and the sheet says the rest were not.
    void aMillionColumns()
    {
        QByteArray line = "a";
        for (int i = 0; i < 1000000; ++i) line += ",";
        line += "z\n";
        Workbook wb;
        QString why;
        QVERIFY2(openCsv(line, &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(!wb.readSheet(0, &s));
        QVERIFY2(s.error.contains(QLatin1String("16,384")), qPrintable(s.error));
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("i:a"));
        QCOMPARE(s.extentCols, 1u);
    }

    // More rows than a sheet has: the first 1,048,576 are read, and the sheet says so.
    void moreRowsThanASheetHas()
    {
        QByteArray file;
        file.reserve(2200000);
        for (int i = 0; i < 1048578; ++i) file += "1\n";
        Workbook wb;
        QString why;
        QVERIFY2(openCsv(file, &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(!wb.readSheet(0, &s));
        QVERIFY2(s.error.contains(QLatin1String("1,048,576")), qPrintable(s.error));
        QCOMPARE(s.extentRows, 1048576u);
        QCOMPARE(s.cells.cellCount(), qint64(1048576));
    }

    // A field that is longer than a cell is cut, and a quote never closed makes the rest one cell (cut too), without the file's size in memory.
    void longFieldsAndQuotesNeverClosed()
    {
        Workbook wb;
        QString why;
        QVERIFY2(openCsv("a," + QByteArray(5 << 20, 'x') + "\nnext,row\n", &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(!wb.readSheet(0, &s));
        QVERIFY2(s.error.contains(QLatin1String("32,767")), qPrintable(s.error));
        const Cell *b1 = at(s, QStringLiteral("B1"));
        QVERIFY(b1);
        QCOMPARE(s.text(*b1, wb.sharedStrings()).size(), 32767);
        QCOMPARE(shown(wb, s, QStringLiteral("A2")), QStringLiteral("i:next"));
        Workbook open;
        QByteArray file = "a,\"never closed\n";
        for (int i = 0; i < 100000; ++i) file += "more,text,here\n";
        QVERIFY2(openCsv(file, &open, &why), qPrintable(why));
        QVERIFY(!open.readSheet(0, &s));
        QVERIFY2(s.error.contains(QLatin1String("never closed")), qPrintable(s.error));
        QCOMPARE(shown(open, s, QStringLiteral("A1")), QStringLiteral("i:a"));
        QCOMPARE(s.cells.cellCount(), qint64(2));
    }

    // More cells than the budget of a sheet: the read stops there, saying so, with the cells before it.
    void theBudgetOfCellsStopsTheRead()
    {
        QByteArray file;
        for (int i = 0; i < 2000; ++i) file += "1,2,3\n";
        Limits limits;
        limits.cellsPerSheet = 5000;
        Workbook wb;
        QString why;
        QVERIFY2(openCsv(file, &wb, &why, limits), qPrintable(why));
        Sheet s;
        QVERIFY(!wb.readSheet(0, &s));
        QVERIFY2(s.error.contains(QLatin1String("more cells")), qPrintable(s.error));
        QVERIFY(s.cells.cellCount() <= 5001);
        QVERIFY(s.cells.cellCount() >= 5000);
    }

    // Any bytes at all: the file is read or refused, and never takes more than its own size to do it. (Random bytes, some with the marks of encodings, some with quotes and delimiters.)
    void anyBytesAreHandled()
    {
        QRandomGenerator rng(12345);
        const char alphabet[] = "ab1,.;\t\"\r\n=-+'%$ \xC3\xA9\xFF\xFE\x00#TRUE";
        for (int round = 0; round < 300; ++round) {
            QByteArray bytes;
            const int n = int(rng.bounded(1, 2000));
            if (round % 5 == 1) bytes = "\xFF\xFE";
            if (round % 5 == 2) bytes = "\xEF\xBB\xBF";
            for (int i = 0; i < n; ++i) bytes += alphabet[rng.bounded(int(sizeof(alphabet)) - 1)];
            Workbook wb;
            QString why;
            if (!openCsv(bytes, &wb, &why)) {
                QVERIFY(!why.isEmpty());
                continue;
            }
            Sheet s;
            wb.readSheet(0, &s, nullptr, -1, 5);
            wb.readSheet(0, &s);
            QVERIFY(s.cells.cellCount() <= n);
        }
    }

    // A formula of any length is a formula or text, never a hang; a record of a million empty fields makes no cell.
    void hostileFormulasAndEmptiness()
    {
        QByteArray file = "=";
        file += QByteArray(100000, '(');
        file += "\n\"=SUM(";
        for (int i = 0; i < 5000; ++i) file += "A1,";
        file += "A1)\"\n";
        file += QByteArray(1000000, ',') + "\n";
        Workbook wb;
        QString why;
        QVERIFY2(openCsv(file, &wb, &why), qPrintable(why));
        Sheet s;
        wb.readSheet(0, &s);
        QVERIFY2(s.cells.cellCount() <= 3, qPrintable(QString::number(s.cells.cellCount())));
        QVERIFY(s.formulaCells <= 1);
    }
};

JP_TEST_CLASS(SheetCsvHostileTests)
#include "tst_sheet_csvhostile.moc"
