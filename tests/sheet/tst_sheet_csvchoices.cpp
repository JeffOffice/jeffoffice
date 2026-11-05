// Tests of what a person chooses when a text file of values is opened: the separators (several, and several in a row as one), the quote, the code page, the row to begin at, and how each column
// is read (as typed, as text, as a date in an order, or left out), and the preview that is shown before the file is opened (bounded in rows, columns, characters, and bytes). Every file is
// built in memory.
#include "odshelpers.h"

#include "sheet/csv.h"
#include "sheet/xlscodepage.h"

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

    // The marks a file writes numbers with are chosen for it: the decimal mark, the thousands mark, and a minus sign after the number. A text that is no number of those marks stays text.
    void numbersAreReadWithTheMarksChosenForTheFile()
    {
        LocaleScope locale(QLocale::c());
        auto c = choices();
        c->separators = QStringLiteral(";");
        c->decimalMark = QLatin1Char(',');
        c->thousandsMark = QLatin1Char('.');
        c->trailingMinus = true;
        Workbook wb;
        QVERIFY(openWith("1.234,5;12,5-;-3;7;1.5;50%;0,25%;abc\n", c, &wb));
        const Sheet s = read(wb);
        QCOMPARE(shown(wb, s, QLatin1String("A1")), QStringLiteral("n:1234.5"));
        QCOMPARE(shown(wb, s, QLatin1String("B1")), QStringLiteral("n:-12.5"));
        QCOMPARE(shown(wb, s, QLatin1String("C1")), QStringLiteral("n:-3"));
        QCOMPARE(shown(wb, s, QLatin1String("D1")), QStringLiteral("n:7"));
        QCOMPARE(shown(wb, s, QLatin1String("E1")), QStringLiteral("i:1.5"));   // (a point in a number with a comma for its decimal mark is not a mark of this file)
        QCOMPARE(shown(wb, s, QLatin1String("F1")), QStringLiteral("n:0.5"));
        QVERIFY(qAbs(shown(wb, s, QLatin1String("G1")).mid(2).toDouble() - 0.0025) < 1e-15);
        QCOMPARE(shown(wb, s, QLatin1String("H1")), QStringLiteral("i:abc"));
        // with a point for the decimal mark and a comma for thousands, the other way round
        auto d = choices();
        d->separators = QStringLiteral(";");
        d->decimalMark = QLatin1Char('.');
        d->thousandsMark = QLatin1Char(',');
        Workbook wb2;
        QVERIFY(openWith("1,234.5;1.5;1,5\n", d, &wb2));
        const Sheet s2 = read(wb2);
        QCOMPARE(shown(wb2, s2, QLatin1String("A1")), QStringLiteral("n:1234.5"));
        QCOMPARE(shown(wb2, s2, QLatin1String("B1")), QStringLiteral("n:1.5"));
        QCOMPARE(shown(wb2, s2, QLatin1String("C1")), QStringLiteral("i:1,5"));
        // a trailing minus is only read when chosen
        auto e = choices();
        e->separators = QStringLiteral(";");
        Workbook wb3;
        QVERIFY(openWith("5-\n", e, &wb3));
        const Sheet s3 = read(wb3);
        QCOMPARE(shown(wb3, s3, QLatin1String("A1")), QStringLiteral("i:5-"));
    }

    // Columns of fixed width: each line is cut at the places chosen, with the spaces around each piece taken off; the last column takes what is left, and a short line has fewer fields.
    void aFileOfFixedColumnsIsCutAtTheChosenPlaces()
    {
        LocaleScope locale(QLocale::c());
        auto c = choices();
        c->fixedWidth = true;
        c->breaks = {6, 12};
        Workbook wb;
        QVERIFY(openWith("Name  Qty   Note here\nBob   12    ok\r\nAl    7\n\nCy    100   x y z\n", c, &wb));
        const Sheet s = read(wb);
        QCOMPARE(shown(wb, s, QLatin1String("A1")), QStringLiteral("i:Name"));
        QCOMPARE(shown(wb, s, QLatin1String("B1")), QStringLiteral("i:Qty"));
        QCOMPARE(shown(wb, s, QLatin1String("C1")), QStringLiteral("i:Note here"));
        QCOMPARE(shown(wb, s, QLatin1String("A2")), QStringLiteral("i:Bob"));
        QCOMPARE(shown(wb, s, QLatin1String("B2")), QStringLiteral("n:12"));
        QCOMPARE(shown(wb, s, QLatin1String("C2")), QStringLiteral("i:ok"));
        QCOMPARE(shown(wb, s, QLatin1String("B3")), QStringLiteral("n:7"));
        QCOMPARE(shown(wb, s, QLatin1String("C3")), QStringLiteral("none"));
        QCOMPARE(shown(wb, s, QLatin1String("A4")), QStringLiteral("none"));
        QCOMPARE(shown(wb, s, QLatin1String("A5")), QStringLiteral("i:Cy"));
        QCOMPARE(shown(wb, s, QLatin1String("B5")), QStringLiteral("n:100"));
        QCOMPARE(shown(wb, s, QLatin1String("C5")), QStringLiteral("i:x y z"));
        // the preview is cut the same way
        auto narrow = choices();
        narrow->fixedWidth = true;
        narrow->breaks = {4};
        const QVector<QStringList> rows = csvPreview(QByteArray("ab  cd\n"), QStringLiteral("x.txt"), *narrow, QLocale::c());
        QCOMPARE(rows.size(), 1);
        QCOMPARE(rows[0], (QStringList{QStringLiteral("ab"), QStringLiteral("cd")}));
    }

    // The values of the import window's own example land as numbers, and its preview keeps the text of the file as it is (never half converted).
    void theExampleOfTheImportWindowLandsAsNumbersAndThePreviewKeepsTheText()
    {
        LocaleScope locale(QLocale::c());
        auto c = choices();
        c->fixedWidth = true;
        c->breaks = {6, 12};
        c->decimalMark = QLatin1Char(',');
        c->thousandsMark = QLatin1Char('.');
        c->trailingMinus = true;
        const QByteArray file = "Name  Qty   Price\nBob   12    1.234,50\nAl    7     9,5-\n";
        Workbook wb;
        QVERIFY(openWith(file, c, &wb));
        const Sheet s = read(wb);
        QCOMPARE(shown(wb, s, QLatin1String("C2")), QStringLiteral("n:1234.5"));
        QCOMPARE(shown(wb, s, QLatin1String("C3")), QStringLiteral("n:-9.5"));
        const QVector<QStringList> rows = csvPreview(file, QStringLiteral("data.txt"), *c, QLocale::c());
        QCOMPARE(rows.value(1).value(2), QStringLiteral("1.234,50"));
        QCOMPARE(rows.value(2).value(2), QStringLiteral("9,5-"));
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

    // The ISO 8859 pages and the Macintosh page can be chosen as the encoding of a file, and a text goes back to the bytes of its page (a letter the page lacks is a question mark, counted).
    void theIsoAndMacintoshPagesAreRead()
    {
        LocaleScope locale(QLocale::c());
        const struct {
            int page;
            const char *bytes;
            QString text;
        } cases[] = {{28592, "\xB3\xF1", QString::fromUtf8("\xC5\x82\xC5\x84")},         // Latin-2: the Polish l with a stroke and n with an acute
                     {28595, "\xD6\xD0", QString::fromUtf8("\xD0\xB6\xD0\xB0")},         // Cyrillic: zhe, a
                     {28597, "\xE1\xE2", QString::fromUtf8("\xCE\xB1\xCE\xB2")},         // Greek: alpha, beta
                     {28599, "\xF0\xFD", QString::fromUtf8("\xC4\x9F\xC4\xB1")},         // Latin-5: g breve, dotless i
                     {28605, "\xA4\xBD", QString::fromUtf8("\xE2\x82\xAC\xC5\x93")},   // Latin-9: the euro sign, oe
                     {10000, "\x8E\x9F", QString::fromUtf8("\xC3\xA9\xC3\xBC")},         // Macintosh Roman: e acute, u diaeresis
                     {28591, "\xE9\xFC", QString::fromUtf8("\xC3\xA9\xC3\xBC")}};
        for (const auto &c : cases) {
            auto choice = choices();
            choice->separators = QStringLiteral(",");
            choice->encodingSet = true;
            choice->encoding = CsvEncoding::Legacy;
            choice->codePage = c.page;
            Workbook wb;
            QVERIFY(openWith(QByteArray(c.bytes) + ",1\n", choice, &wb));
            QCOMPARE(shown(wb, read(wb), QLatin1String("A1")), QStringLiteral("i:") + c.text);
            QByteArray back;
            qsizetype lost = -1;
            QVERIFY(xlsTextToBytes(c.page, c.text, &back, &lost));
            QCOMPARE(back, QByteArray(c.bytes));
            QCOMPARE(lost, qsizetype(0));
        }
        QByteArray out;
        qsizetype lost = 0;
        QVERIFY(xlsTextToBytes(1252, QString::fromUtf8("a\xE2\x82\xAC\xCE\xB1z"), &out, &lost));   // the euro sign is in 1252; alpha is not
        QCOMPARE(out, QByteArray("a\x80?z"));
        QCOMPARE(lost, qsizetype(1));
        QVERIFY(!xlsTextToBytes(1361, QStringLiteral("a"), &out, &lost) || !out.isEmpty());   // (a double-byte page is the system's: written when it has the converter)
    }

    // A double-byte page (Japanese here) can be written when the system has it: the characters go to the bytes of the page, and a letter the page lacks is a question mark.
    void aDoubleBytePageIsWrittenWhenTheSystemHasIt()
    {
        if (!xlsCanConvert(932, CodePageBackend::Qt)) QSKIP("this machine has no converter for the Japanese page");
        QByteArray out;
        qsizetype lost = 0;
        QVERIFY(xlsTextToBytes(932, QString::fromUtf8("\xE6\x97\xA5\xE6\x9C\xAC a"), &out, &lost));
        QCOMPARE(out, QByteArray("\x93\xFA\x96\x7B a"));
        QCOMPARE(lost, qsizetype(0));
        QVERIFY(xlsTextToBytes(932, QString::fromUtf8("\xCE\xB1\xF0\x9F\x98\x80"), &out, &lost));   // (a Greek letter is in the page; the face is not)
        QVERIFY(out.endsWith("?"));
        QCOMPARE(lost, qsizetype(1));
    }

    // How a file's lines end is told from its first line, and kept in the form.
    void theLineEndsOfAFileAreKept()
    {
        const QLocale c = QLocale::c();
        QCOMPARE(detectCsvForm("a,b\n1,2\n", QStringLiteral("x.csv"), c).lineEnd, QStringLiteral("\n"));
        QCOMPARE(detectCsvForm("a,b\r\n1,2\r\n", QStringLiteral("x.csv"), c).lineEnd, QStringLiteral("\r\n"));
        QCOMPARE(detectCsvForm("a,b\r1,2\r", QStringLiteral("x.csv"), c).lineEnd, QStringLiteral("\r"));
        QCOMPARE(detectCsvForm("a,b", QStringLiteral("x.csv"), c).lineEnd, QStringLiteral("\r\n"));
        QCOMPARE(detectCsvForm("sep=;\na;b\n", QStringLiteral("x.csv"), c).lineEnd, QStringLiteral("\n"));
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

    // A mark is dropped only if the file has it: an encoding chosen for a file that has none leaves its first letter where it is.
    void aMarkIsDroppedOnlyIfTheFileHasIt()
    {
        LocaleScope locale(QLocale::c());
        auto c = choices();
        c->separators = QStringLiteral(",");
        c->encodingSet = true;
        c->encoding = CsvEncoding::Utf8Bom;
        Workbook wb;
        QVERIFY(openWith("abc,def\n", c, &wb));
        QCOMPARE(shown(wb, read(wb), QLatin1String("A1")), QStringLiteral("i:abc"));
        // the same in the preview, and a file with the mark loses it
        c->separators = QStringLiteral(",");
        QCOMPARE(csvPreview("abc,def\n", QStringLiteral("a.csv"), *c, QLocale::c()).value(0).value(0), QStringLiteral("abc"));
        QCOMPARE(csvPreview("\xEF\xBB\xBF" "abc,def\n", QStringLiteral("a.csv"), *c, QLocale::c()).value(0).value(0), QStringLiteral("abc"));
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
