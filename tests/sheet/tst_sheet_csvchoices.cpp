// Tests of what a person chooses when a text file of values is opened: the separators (several, and several in a row as one), the quote, the code page, the row to begin at, and how each column
// is read (as typed, as text, as a date in an order, or left out), and the preview that is shown before the file is opened (bounded in rows, columns, characters, and bytes). Every file is
// built in memory.
#include "odshelpers.h"

#include "sheet/csv.h"

namespace {

struct LocaleScope {
    QLocale old;
    explicit LocaleScope(const QLocale &l) : old() { QLocale::setDefault(l); }
    ~LocaleScope() { QLocale::setDefault(old); }
};

std::shared_ptr<CsvImport> choices() { return std::make_shared<CsvImport>(); }

} // namespace

class SheetCsvChoicesTests : public QObject {
    Q_OBJECT

    static bool openWith(const QByteArray &bytes, const std::shared_ptr<CsvImport> &c, Workbook *wb, QString *why = nullptr)
    {
        Limits limits;
        limits.csv = c;
        QString message;
        return openBytes(bytes, QStringLiteral("data.csv"), wb, why ? why : &message, limits);
    }
    static Sheet read(const Workbook &wb)
    {
        Sheet s;
        wb.readSheet(0, &s);
        return s;
    }

private Q_SLOTS:
    void severalSeparatorsAndAQuoteTheFileDoesNotUseAreAsChosen()
    {
        LocaleScope locale(QLocale::c());
        auto c = choices();
        c->separators = QStringLiteral(";|");
        c->quote = QLatin1Char('\'');
        Workbook wb;
        QVERIFY(openWith("a;b|c\n1;'x;y'|3\n", c, &wb));
        const Sheet s = read(wb);
        QCOMPARE(shown(wb, s, QLatin1String("C1")), QStringLiteral("i:c"));
        QCOMPARE(shown(wb, s, QLatin1String("B2")), QStringLiteral("i:x;y"));
        QCOMPARE(shown(wb, s, QLatin1String("C2")), QStringLiteral("n:3"));
        // with no quote at all, a quote mark is text
        auto none = choices();
        none->separators = QStringLiteral(",");
        none->quote = QChar();
        Workbook wb2;
        QVERIFY(openWith("\"a,b\",c\n", none, &wb2));
        const Sheet s2 = read(wb2);
        QCOMPARE(shown(wb2, s2, QLatin1String("A1")), QStringLiteral("i:\"a"));
        QCOMPARE(shown(wb2, s2, QLatin1String("B1")), QStringLiteral("i:b\""));
        QCOMPARE(shown(wb2, s2, QLatin1String("C1")), QStringLiteral("i:c"));
    }

    void separatorsInARowCountAsOneWhenAsked()
    {
        LocaleScope locale(QLocale::c());
        auto c = choices();
        c->separators = QStringLiteral(" ");
        Workbook plain;
        QVERIFY(openWith("a   b\n", c, &plain));
        const Sheet s = read(plain);
        QCOMPARE(shown(plain, s, QLatin1String("B1")), QStringLiteral("none"));   // (an empty field between)
        QCOMPARE(shown(plain, s, QLatin1String("D1")), QStringLiteral("i:b"));
        c->mergeSeparators = true;
        Workbook merged;
        QVERIFY(openWith("a   b\nc d\n", c, &merged));
        const Sheet m = read(merged);
        QCOMPARE(shown(merged, m, QLatin1String("A1")), QStringLiteral("i:a"));
        QCOMPARE(shown(merged, m, QLatin1String("B1")), QStringLiteral("i:b"));
        QCOMPARE(shown(merged, m, QLatin1String("B2")), QStringLiteral("i:d"));
        QVERIFY(!at(m, QStringLiteral("C1")));
    }

    void theRowToBeginAtLeavesTheRowsBeforeItOut()
    {
        LocaleScope locale(QLocale::c());
        auto c = choices();
        c->separators = QStringLiteral(",");
        c->firstRow = 2;
        Workbook wb;
        QVERIFY(openWith("Report\nmade on a Monday\nname,qty\nbolt,5\n", c, &wb));
        const Sheet s = read(wb);
        QCOMPARE(shown(wb, s, QLatin1String("A1")), QStringLiteral("i:name"));
        QCOMPARE(shown(wb, s, QLatin1String("B2")), QStringLiteral("n:5"));
        QCOMPARE(s.extentRows, quint32(2));
        // only the first rows asked for are read when the window shows its first screen, counted from the row it begins at
        Sheet first;
        QVERIFY(wb.readSheet(0, &first, nullptr, -1, 1));
        QCOMPARE(first.extentRows, quint32(1));
    }

    void eachColumnIsReadAsChosen()
    {
        LocaleScope locale(QLocale::c());
        auto c = choices();
        c->separators = QStringLiteral(",");
        c->columns = {CsvColumn{CsvColumn::Kind::Text, QString()}, CsvColumn{CsvColumn::Kind::Date, QStringLiteral("DMY")}, CsvColumn{CsvColumn::Kind::Skip, QString()},
                      CsvColumn{CsvColumn::Kind::Date, QStringLiteral("MDY")}};
        Workbook wb;
        QVERIFY(openWith("1.50,31/01/2024,gone,01/31/2024\n7,31/31/2024,gone,2024-01-31\n", c, &wb));
        const Sheet s = read(wb);
        // column A as text; B a date; the third left out, so the fourth is the third
        QCOMPARE(show(s, wb.sharedStrings(), QStringLiteral("A1")), QStringLiteral("i:1.50"));
        QCOMPARE(show(s, wb.sharedStrings(), QStringLiteral("A2")), QStringLiteral("i:7"));
        const QString first = show(s, wb.sharedStrings(), QStringLiteral("B1"));
        QVERIFY2(first.startsWith(QStringLiteral("n:")), qPrintable(first));
        QCOMPARE(show(s, wb.sharedStrings(), QStringLiteral("C1")), first);   // (the same day, written the other way round)
        QCOMPARE(shown(wb, s, QLatin1String("B1")), QStringLiteral("n:45322"));
        QCOMPARE(wb.styles().formatCode(at(s, QStringLiteral("B1"))->style, QLocale::c()), QStringLiteral("yyyy-mm-dd"));   // (shown as a date)
        QCOMPARE(show(s, wb.sharedStrings(), QStringLiteral("B2")), QStringLiteral("i:31/31/2024"));   // (not a date in that order: the text stays)
        QVERIFY(!at(s, QStringLiteral("D1")));
        QCOMPARE(s.extentCols, quint32(3));
    }

    void aCodePageCanBeChosenForAFileThatSaysNothing()
    {
        LocaleScope locale(QLocale::c());
        auto c = choices();
        c->separators = QStringLiteral(",");
        c->encodingSet = true;
        c->encoding = CsvEncoding::Legacy;
        c->codePage = 1251;
        Workbook wb;
        QVERIFY(openWith("\xCF\xF0\xE8\xE2\xE5\xF2,1\n", c, &wb));
        const Sheet s = read(wb);
        QCOMPARE(shown(wb, s, QLatin1String("A1")), QStringLiteral("i:") + QString::fromUtf8("\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82"));
        // a mark at the front of a file read as UTF-8 is not text
        auto utf8 = choices();
        utf8->encodingSet = true;
        utf8->encoding = CsvEncoding::Utf8;
        Workbook wb2;
        QVERIFY(openWith("\xEF\xBB\xBF" "a,b\n", utf8, &wb2));
        QCOMPARE(shown(wb2, read(wb2), QLatin1String("A1")), QStringLiteral("i:a"));
    }

    void aListOfColumnChoicesThatIsTooLongIsCut()
    {
        LocaleScope locale(QLocale::c());
        auto c = choices();
        c->separators = QStringLiteral(",");
        for (int i = 0; i < 100000; ++i) c->columns.append(CsvColumn());
        c->firstRow = -5;
        Workbook wb;
        QVERIFY(openWith("a,b\n", c, &wb));
        QCOMPARE(shown(wb, read(wb), QLatin1String("B1")), QStringLiteral("i:b"));
    }

    // ---- the preview

    void thePreviewShowsTheFirstRowsAsTheChoicesWouldReadThem()
    {
        LocaleScope locale(QLocale::c());
        CsvImport c;
        c.separators = QStringLiteral(";");
        c.firstRow = 1;
        c.columns = {CsvColumn(), CsvColumn{CsvColumn::Kind::Date, QStringLiteral("DMY")}};
        int columns = 0;
        const QVector<QStringList> rows = csvPreview("title\na;05/03/2024;x\nb;15/12/2023;y\n", QStringLiteral("a.csv"), c, QLocale::c(), nullptr, &columns);
        QCOMPARE(columns, 3);
        QCOMPARE(rows.size(), 2);
        QCOMPARE(rows[0], (QStringList{"a", "2024-03-05", "x"}));
        QCOMPARE(rows[1], (QStringList{"b", "2023-12-15", "y"}));
    }

    void thePreviewIsBounded()
    {
        LocaleScope locale(QLocale::c());
        // 5,000 rows of 300 columns, one field of 5,000 characters
        QByteArray big;
        for (int r = 0; r < 5000; ++r) {
            QByteArray line;
            for (int c = 0; c < 300; ++c) line += (r == 3 && c == 2 ? QByteArray(5000, 'z') : QByteArray::number(c)) + ",";
            big += line + "\n";
        }
        int columns = 0;
        const QVector<QStringList> rows = csvPreview(big, QStringLiteral("big.csv"), CsvImport(), QLocale::c(), nullptr, &columns);
        QCOMPARE(rows.size(), kCsvPreviewRows);
        QCOMPARE(columns, kCsvPreviewColumns);
        QCOMPARE(rows[3][2].size(), kCsvPreviewFieldChars);
        for (const QStringList &r : rows) QVERIFY(r.size() <= kCsvPreviewColumns);
        // a file that is one line of megabytes: the first megabyte at most is looked at, and the field is cut
        const QByteArray line(5 << 20, 'a');
        const QVector<QStringList> one = csvPreview(line, QStringLiteral("one.csv"), CsvImport(), QLocale::c(), nullptr, &columns);
        QCOMPARE(one.size(), 1);
        QCOMPARE(one[0][0].size(), kCsvPreviewFieldChars);
        // a binary file is no text: nothing to show
        QVERIFY(csvPreview(QByteArray("ab\0cd", 5), QStringLiteral("x.csv"), CsvImport(), QLocale::c(), nullptr, &columns).isEmpty());
    }
};

JP_TEST_CLASS(SheetCsvChoicesTests)
#include "tst_sheet_csvchoices.moc"
