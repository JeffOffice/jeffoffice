// Tests of the reader of delimited text files (.csv, .tsv, .txt): the delimiter and the encoding that are found, quoted fields, line breaks, each kind of value read in the person's
// locale, the text that a number would damage, formulas, the limits of a sheet, the first rows read at once, and the workbook written from it. Every file is built in memory.
#include "odshelpers.h"

#include "calc/dates.h"
#include "sheet/csv.h"
#include "sheet/xlscodepage.h"
#include "sheet/xlsxbase.h"

namespace {

// The default locale of the process for the length of a test (the files are read in it).
struct LocaleScope {
    QLocale old;
    explicit LocaleScope(const QLocale &l) : old()
    {
        QLocale::setDefault(l);
    }
    ~LocaleScope() { QLocale::setDefault(old); }
};

QByteArray utf16(const QString &text, bool little)
{
    QByteArray out = little ? QByteArray("\xFF\xFE", 2) : QByteArray("\xFE\xFF", 2);
    for (const QChar c : text) {
        const ushort u = c.unicode();
        if (little) {
            out.append(char(u & 0xFF));
            out.append(char(u >> 8));
        } else {
            out.append(char(u >> 8));
            out.append(char(u & 0xFF));
        }
    }
    return out;
}

} // namespace

class SheetCsvTests : public QObject {
    Q_OBJECT

    static bool openCsv(const QByteArray &bytes, Workbook *wb, QString *why = nullptr, const QString &name = QStringLiteral("data.csv"), const Limits &limits = Limits())
    {
        QString message;
        const bool ok = openBytes(bytes, name, wb, why ? why : &message, limits);
        return ok;
    }
    static Sheet readSheet(const Workbook &wb)
    {
        Sheet s;
        wb.readSheet(0, &s);
        return s;
    }
    static QString at1(const Workbook &wb, const Sheet &s, const char *ref) { return shown(wb, s, QLatin1String(ref)); }

private Q_SLOTS:
    // A file of values is a workbook of one sheet, named for the file, saved as .xlsx like any other that was read from another form.
    void aFileIsOneSheetNamedForIt()
    {
        LocaleScope locale(QLocale::c());
        Workbook wb;
        QString why;
        QVERIFY2(openCsv("a,b\n1,2\n", &wb, &why, QStringLiteral("my data.csv")), qPrintable(why));
        QVERIFY(wb.isConverted());
        QVERIFY(!wb.isBinary());
        QCOMPARE(wb.convertedKind(), QStringLiteral(".csv"));
        QCOMPARE(wb.info().sheets.size(), 1);
        QCOMPARE(wb.info().sheets[0].name, QStringLiteral("my data"));
        QVERIFY(wb.skippedFeatures().isEmpty());
        Workbook odd;
        QVERIFY(openCsv("x\n", &odd, &why, QStringLiteral("a[b]:c*d?e.csv")));
        QCOMPARE(odd.info().sheets[0].name, QStringLiteral("a_b__c_d_e"));
        Workbook tsv, txt;
        QVERIFY(openCsv("a\tb\n", &tsv, &why, QStringLiteral("t.tsv")));
        QVERIFY(openCsv("a;b\n", &txt, &why, QStringLiteral("t.TXT")));
        QCOMPARE(at1(tsv, readSheet(tsv), "B1"), QStringLiteral("i:b"));
        QCOMPARE(at1(txt, readSheet(txt), "B1"), QStringLiteral("i:b"));
    }

    // The delimiter is the one that splits the first lines alike: comma, semicolon, tab, or bar; quotes hide a delimiter; a first line `sep=;` says it; one column needs none.
    void theDelimiterIsFound()
    {
        LocaleScope locale(QLocale::c());
        struct Case {
            const char *file;
            const char *b1;
            QChar delimiter;
        };
        const Case cases[] = {{"a,b,c\n1,2,3\n4,5,6\n", "i:b", QLatin1Char(',')},
                              {"a;b;c\n1;2;3\n4;5;6\n", "i:b", QLatin1Char(';')},
                              {"a\tb\tc\n1\t2\t3\n4\t5\t6\n", "i:b", QLatin1Char('\t')},
                              {"a|b|c\n1|2|3\n4|5|6\n", "i:b", QLatin1Char('|')},
                              {"\"x,y\";b;c\n\"1,5\";2;3\n\"4,5\";5;6\n", "i:b", QLatin1Char(';')},
                              {"sep=;\na,b;c\n1,5;2\n", "i:c", QLatin1Char(';')},
                              {"only\nthree\nlines\n", "none", QLatin1Char(',')}};
        for (const Case &c : cases) {
            Workbook wb;
            QString why;
            QVERIFY2(openCsv(c.file, &wb, &why), qPrintable(why));
            const Sheet s = readSheet(wb);
            QCOMPARE(at1(wb, s, "B1"), QString::fromLatin1(c.b1)) ;
            const auto detected = detectCsvForm(QByteArray(c.file), QStringLiteral("x.csv"), QLocale::c());
            QCOMPARE(detected.delimiter, c.delimiter);
        }
        // the `sep=` line is not data
        Workbook sep;
        QString why;
        QVERIFY(openCsv("sep=;\nname;age\nann;31\n", &sep, &why));
        const Sheet s = readSheet(sep);
        QCOMPARE(at1(sep, s, "A1"), QStringLiteral("i:name"));
        QCOMPARE(at1(sep, s, "B2"), QStringLiteral("n:31"));
        // a language that writes the decimal comma lists with the semicolon: a tie between them goes to its own
        QCOMPARE(detectCsvForm("a,b;c\n1,2;3\n", QStringLiteral("x.csv"), QLocale(QLocale::German, QLocale::Germany)).delimiter, QLatin1Char(';'));
    }

    // UTF-8 with and without the byte order mark, UTF-16 of either order, and the code page of the person's language for what is not Unicode.
    void encodingsAreRead()
    {
        LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
        const QString text = QStringLiteral("name,city\nZoë,Zürich\n日本,€5\n");
        auto check = [&](const QByteArray &bytes, CsvEncoding encoding) {
            Workbook wb;
            QString why;
            QVERIFY2(openCsv(bytes, &wb, &why), qPrintable(why));
            const Sheet s = readSheet(wb);
            QCOMPARE(at1(wb, s, "A2"), QStringLiteral("i:Zoë"));
            QCOMPARE(at1(wb, s, "B2"), QStringLiteral("i:Zürich"));
            QCOMPARE(at1(wb, s, "A3"), QStringLiteral("i:日本"));
            QCOMPARE(at1(wb, s, "B3"), QStringLiteral("i:€5"));
            QCOMPARE(detectCsvForm(bytes, QStringLiteral("x.csv"), QLocale()).encoding, encoding);
        };
        check(text.toUtf8(), CsvEncoding::Utf8);
        check(QByteArray("\xEF\xBB\xBF") + text.toUtf8(), CsvEncoding::Utf8Bom);
        check(utf16(text, true), CsvEncoding::Utf16Le);
        check(utf16(text, false), CsvEncoding::Utf16Be);
        // a file that is not UTF-8 is in the language's code page: Windows-1252 for English, and the others' own
        Workbook legacy;
        QString why;
        QVERIFY2(openCsv("caf\xE9,na\xEFve\n", &legacy, &why), qPrintable(why));
        QCOMPARE(at1(legacy, readSheet(legacy), "A1"), QStringLiteral("i:café"));
        QCOMPARE(at1(legacy, readSheet(legacy), "B1"), QStringLiteral("i:naïve"));
        QCOMPARE(detectCsvForm("caf\xE9\n", QStringLiteral("x.csv"), QLocale()).encoding, CsvEncoding::Legacy);
        QCOMPARE(legacyCodePageFor(QLocale(QLocale::German, QLocale::Germany)), 1252);
        QCOMPARE(legacyCodePageFor(QLocale(QLocale::Russian, QLocale::Russia)), 1251);
        QCOMPARE(legacyCodePageFor(QLocale(QLocale::Greek, QLocale::Greece)), 1253);
        QCOMPARE(legacyCodePageFor(QLocale(QLocale::Japanese, QLocale::Japan)), 932);
        QCOMPARE(legacyCodePageFor(QLocale(QLocale::Polish, QLocale::Poland)), 1250);
        {
            LocaleScope russian(QLocale(QLocale::Russian, QLocale::Russia));
            Workbook cyrillic;
            QVERIFY2(openCsv("\xCF\xF0\xE8\xE2\xE5\xF2\n", &cyrillic, &why), qPrintable(why));   // (Windows-1251)
            QCOMPARE(at1(cyrillic, readSheet(cyrillic), "A1"), QStringLiteral("i:Привет"));
        }
        if (xlsCanConvert(932, CodePageBackend::Automatic)) {
            LocaleScope japanese(QLocale(QLocale::Japanese, QLocale::Japan));
            Workbook jp;
            QVERIFY2(openCsv("\x82\xA0,\x82\xA2\n", &jp, &why), qPrintable(why));   // (Shift JIS)
            QCOMPARE(at1(jp, readSheet(jp), "A1"), QStringLiteral("i:あ"));
            QCOMPARE(at1(jp, readSheet(jp), "B1"), QStringLiteral("i:い"));
        }
    }

    // Whether a file is UTF-8 is told from its first 32 MB (a file of a gigabyte is not read to its end for that): one that is UTF-8 so far is, and a byte that is not is found there.
    void theEncodingIsToldFromTheFirstPart()
    {
        const QLocale english(QLocale::English, QLocale::UnitedStates);
        QByteArray big(40 << 20, 'a');
        for (qsizetype i = 15; i < big.size(); i += 16) big[i] = '\n';
        QByteArray late = big;
        late[late.size() - 5] = char(0xE9);
        QCOMPARE(detectCsvForm(late, QStringLiteral("x.csv"), english).encoding, CsvEncoding::Utf8);
        QByteArray early = big;
        early[1000] = char(0xE9);
        QCOMPARE(detectCsvForm(early, QStringLiteral("x.csv"), english).encoding, CsvEncoding::Legacy);
        // (a character that straddles the end of the part looked at is not a bad byte)
        QByteArray straddle = big;
        straddle.replace((32 << 20) - 1, 2, "\xC3\xA9");
        QCOMPARE(detectCsvForm(straddle, QStringLiteral("x.csv"), english).encoding, CsvEncoding::Utf8);
    }

    // Fields in quotes hold the delimiter, quotes (written twice), and line breaks; a quote inside an unquoted field is a character; what follows a closing quote is kept.
    void quotedFieldsKeepTheirText()
    {
        LocaleScope locale(QLocale::c());
        Workbook wb;
        QString why;
        QVERIFY2(openCsv("\"a,b\",\"say \"\"hi\"\"\",\"line1\nline2\",\"crlf\r\ninside\"\nit's \"fine\",\"quoted\"tail,\"\",x\n", &wb, &why), qPrintable(why));
        const Sheet s = readSheet(wb);
        QCOMPARE(at1(wb, s, "A1"), QStringLiteral("i:a,b"));
        QCOMPARE(at1(wb, s, "B1"), QStringLiteral("i:say \"hi\""));
        QCOMPARE(at1(wb, s, "C1"), QStringLiteral("i:line1\nline2"));
        QCOMPARE(at1(wb, s, "D1"), QStringLiteral("i:crlf\ninside"));
        QCOMPARE(at1(wb, s, "A2"), QStringLiteral("i:it's \"fine\""));
        QCOMPARE(at1(wb, s, "B2"), QStringLiteral("i:quotedtail"));
        QCOMPARE(at1(wb, s, "C2"), QStringLiteral("none"));   // (an empty quoted field is empty)
        QCOMPARE(at1(wb, s, "D2"), QStringLiteral("i:x"));
    }

    // A record ends at a line feed, a carriage return, or both; the last needs no end; a blank line is an empty row; a line ending the file is not another row.
    void recordsEndAtAnyLineBreak()
    {
        LocaleScope locale(QLocale::c());
        for (const char *file : {"a,1\r\nb,2\r\nc,3", "a,1\nb,2\nc,3\n", "a,1\rb,2\rc,3\r", "a,1\r\nb,2\nc,3\r\n"}) {
            Workbook wb;
            QString why;
            QVERIFY2(openCsv(file, &wb, &why), qPrintable(why));
            const Sheet s = readSheet(wb);
            QCOMPARE(at1(wb, s, "A3"), QStringLiteral("i:c"));
            QCOMPARE(at1(wb, s, "B3"), QStringLiteral("n:3"));
            QCOMPARE(s.extentRows, 3u);
        }
        Workbook blank;
        QString why;
        QVERIFY(openCsv("a,1\n\n\nb,2\n", &blank, &why));
        const Sheet s = readSheet(blank);
        QCOMPARE(at1(blank, s, "A4"), QStringLiteral("i:b"));
        QCOMPARE(s.extentRows, 4u);
        Workbook trailing;
        QVERIFY(openCsv("a,b,\n1,2,\n", &trailing, &why));
        QCOMPARE(readSheet(trailing).extentCols, 2u);   // (empty fields are no cells)
    }

    // Numbers, percents, money, dates, times, true and false, and errors are read as they are typed in the person's locale: the same file in English and in German.
    void valuesAreReadInThePersonsLocale()
    {
        double march15 = 0;
        QVERIFY(jp::calc::dateToSerial(2024, 3, 15, jp::calc::DateSystem::Date1900, &march15));
        {
            LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
            Workbook wb;
            QString why;
            QVERIFY2(openCsv("1234.5,\"1,234.5\",12%,$5.25,3/15/2024,10:30,TRUE,false,#N/A,1e3,-7\n", &wb, &why), qPrintable(why));
            const Sheet s = readSheet(wb);
            QCOMPARE(at1(wb, s, "A1"), QStringLiteral("n:1234.5"));
            QCOMPARE(at1(wb, s, "B1"), QStringLiteral("n:1234.5"));
            QCOMPARE(wb.styles().formatCode(at(s, QStringLiteral("B1"))->style), QStringLiteral("#,##0.00"));
            QCOMPARE(at1(wb, s, "C1"), QStringLiteral("n:0.12"));
            QCOMPARE(wb.styles().formatCode(at(s, QStringLiteral("C1"))->style), QStringLiteral("0%"));
            QCOMPARE(at1(wb, s, "D1"), QStringLiteral("n:5.25"));
            QCOMPARE(wb.styles().formatCode(at(s, QStringLiteral("D1"))->style), QStringLiteral("\"$\"#,##0.00"));
            QCOMPARE(at1(wb, s, "E1"), QStringLiteral("n:") + QString::number(march15, 'g', 17));
            QCOMPARE(wb.styles().formatCode(at(s, QStringLiteral("E1"))->style, QLocale(QLocale::English, QLocale::UnitedStates)), QStringLiteral("m/d/yyyy"));
            QCOMPARE(at1(wb, s, "F1"), QStringLiteral("n:0.4375"));
            QCOMPARE(wb.styles().formatCode(at(s, QStringLiteral("F1"))->style), QStringLiteral("h:mm"));
            QCOMPARE(at1(wb, s, "G1"), QStringLiteral("b:1"));
            QCOMPARE(at1(wb, s, "H1"), QStringLiteral("b:0"));
            QCOMPARE(at1(wb, s, "I1"), QStringLiteral("e:#N/A"));
            QCOMPARE(at1(wb, s, "J1"), QStringLiteral("n:1000"));
            QCOMPARE(at1(wb, s, "K1"), QStringLiteral("n:-7"));
        }
        {
            LocaleScope locale(QLocale(QLocale::German, QLocale::Germany));
            Workbook wb;
            QString why;
            QVERIFY2(openCsv("1234,5;\"1.234,5\";12%;5,25 €;15.03.2024;10:30;WAHR;1,5\n", &wb, &why), qPrintable(why));
            const Sheet s = readSheet(wb);
            QCOMPARE(at1(wb, s, "A1"), QStringLiteral("n:1234.5"));
            QCOMPARE(at1(wb, s, "B1"), QStringLiteral("n:1234.5"));
            QCOMPARE(at1(wb, s, "C1"), QStringLiteral("n:0.12"));
            QCOMPARE(at1(wb, s, "D1"), QStringLiteral("n:5.25"));
            QVERIFY2(wb.styles().formatCode(at(s, QStringLiteral("D1"))->style).contains(QStringLiteral("€")), qPrintable(wb.styles().formatCode(at(s, QStringLiteral("D1"))->style)));
            QCOMPARE(at1(wb, s, "E1"), QStringLiteral("n:") + QString::number(march15, 'g', 17));
            QCOMPARE(at1(wb, s, "F1"), QStringLiteral("n:0.4375"));
            QCOMPARE(at1(wb, s, "H1"), QStringLiteral("n:1.5"));
        }
    }

    // Text that a number would damage stays text: digits with a zero in front, and more than fifteen digits. Other numbers are numbers.
    void textThatANumberWouldDamageStaysText()
    {
        LocaleScope locale(QLocale::c());
        Workbook wb;
        QString why;
        QVERIFY2(openCsv("007,-007,00,0123456,1234567890123456,123456789012345,0,0.5,10,\"00\"\n", &wb, &why), qPrintable(why));
        const Sheet s = readSheet(wb);
        QCOMPARE(at1(wb, s, "A1"), QStringLiteral("i:007"));
        QCOMPARE(at1(wb, s, "B1"), QStringLiteral("i:-007"));
        QCOMPARE(at1(wb, s, "C1"), QStringLiteral("i:00"));
        QCOMPARE(at1(wb, s, "D1"), QStringLiteral("i:0123456"));
        QCOMPARE(at1(wb, s, "E1"), QStringLiteral("i:1234567890123456"));
        QCOMPARE(at1(wb, s, "F1"), QStringLiteral("n:123456789012345"));
        QCOMPARE(at1(wb, s, "G1"), QStringLiteral("n:0"));
        QCOMPARE(at1(wb, s, "H1"), QStringLiteral("n:0.5"));
        QCOMPARE(at1(wb, s, "I1"), QStringLiteral("n:10"));
        QCOMPARE(at1(wb, s, "J1"), QStringLiteral("i:00"));
    }

    // A text that only looks like a date stays text: two small numbers with a dash or a slash, a month's name joined to a number. Real dates are dates.
    void textThatOnlyLooksLikeADateStaysText()
    {
        LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
        Workbook wb;
        QString why;
        QVERIFY2(openCsv("1-2,3/4,MARCH1,Sep2,12-25,March 1,3/15/2024,2024-03-15,1-2-2024,Mar-2024,May\n", &wb, &why), qPrintable(why));
        const Sheet s = readSheet(wb);
        QCOMPARE(at1(wb, s, "A1"), QStringLiteral("i:1-2"));
        QCOMPARE(at1(wb, s, "B1"), QStringLiteral("i:3/4"));
        QCOMPARE(at1(wb, s, "C1"), QStringLiteral("i:MARCH1"));
        QCOMPARE(at1(wb, s, "D1"), QStringLiteral("i:Sep2"));
        QCOMPARE(at1(wb, s, "E1"), QStringLiteral("i:12-25"));
        QVERIFY2(at1(wb, s, "F1").startsWith(QLatin1String("n:")), qPrintable(at1(wb, s, "F1")));   // (March 1)
        double march15 = 0;
        QVERIFY(jp::calc::dateToSerial(2024, 3, 15, jp::calc::DateSystem::Date1900, &march15));
        QCOMPARE(at1(wb, s, "G1"), QStringLiteral("n:") + QString::number(march15, 'g', 17));
        QCOMPARE(at1(wb, s, "H1"), QStringLiteral("n:") + QString::number(march15, 'g', 17));
        QVERIFY2(at1(wb, s, "I1").startsWith(QLatin1String("n:")), qPrintable(at1(wb, s, "I1")));
        QVERIFY2(at1(wb, s, "J1").startsWith(QLatin1String("n:")), qPrintable(at1(wb, s, "J1")));   // (a month and a year)
        QCOMPARE(at1(wb, s, "K1"), QStringLiteral("i:May"));
    }

    // A field that begins with = is a formula (kept with no result, which the window works out); one that is not a formula, a lone =, a sign and more, and an apostrophe are text.
    void formulasAndApostrophes()
    {
        LocaleScope locale(QLocale::c());
        Workbook wb;
        QString why;
        QVERIFY2(openCsv("1,2,=SUM(A1:B1),=,=1+,-hello,+49 30 1234,'quoted,=A1*2,'-5\n", &wb, &why), qPrintable(why));
        const Sheet s = readSheet(wb);
        QCOMPARE(at1(wb, s, "C1"), QStringLiteral("-"));
        QVERIFY(at(s, QStringLiteral("C1"))->hasFormula());
        QCOMPARE(formulaOf(s, QStringLiteral("C1")), QStringLiteral("SUM(A1:B1)"));
        QCOMPARE(at1(wb, s, "D1"), QStringLiteral("i:="));
        QCOMPARE(at1(wb, s, "E1"), QStringLiteral("i:=1+"));
        QCOMPARE(at1(wb, s, "F1"), QStringLiteral("i:-hello"));
        QCOMPARE(at1(wb, s, "G1"), QStringLiteral("i:+49 30 1234"));
        QCOMPARE(at1(wb, s, "H1"), QStringLiteral("i:'quoted"));
        QCOMPARE(formulaOf(s, QStringLiteral("I1")), QStringLiteral("A1*2"));
        QCOMPARE(at1(wb, s, "J1"), QStringLiteral("i:'-5"));
        QCOMPARE(s.formulaCells, qint64(2));
        QCOMPARE(s.storedResults, qint64(0));
    }

    // The first rows are read at once and the sheet says it is cut short; the whole file is read after.
    void theFirstScreenStopsEarly()
    {
        LocaleScope locale(QLocale::c());
        QByteArray file;
        for (int i = 0; i < 100; ++i) file += QByteArray::number(i) + ",x\n";
        Workbook wb;
        QString why;
        QVERIFY(openCsv(file, &wb, &why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s, nullptr, -1, 10));
        QVERIFY(s.partial);
        QCOMPARE(s.extentRows, 10u);
        QVERIFY(wb.readSheet(0, &s, nullptr, 4));
        QVERIFY(s.partial);
        QCOMPARE(s.extentRows, 5u);
        QVERIFY(wb.readSheet(0, &s));
        QVERIFY(!s.partial);
        QCOMPARE(s.extentRows, 100u);
        QVERIFY(!wb.readSheet(1, &s));
    }

    // A file larger than the pieces it is read in has the same cells: fields, multi-byte characters, and quoted fields that straddle a piece's end.
    void longFilesAreReadInPieces()
    {
        LocaleScope locale(QLocale::c());
        for (int variant = 0; variant < 3; ++variant) {
            QString text;
            const int rows = 150000;
            for (int i = 0; i < rows; ++i) text += QStringLiteral("é%1,\"q\nü %2\",%3\n").arg(i).arg(i).arg(i * 3);
            QByteArray bytes = variant == 0 ? text.toUtf8() : variant == 1 ? utf16(text, true) : utf16(text, false);
            QVERIFY(bytes.size() > (2 << 20));
            Workbook wb;
            QString why;
            QVERIFY2(openCsv(bytes, &wb, &why), qPrintable(why));
            Sheet s;
            QVERIFY(wb.readSheet(0, &s));
            QVERIFY(s.error.isEmpty());
            QCOMPARE(s.extentRows, quint32(rows));
            for (int i : {0, 1, 12345, 70000, 149999}) {
                QCOMPARE(at1(wb, s, qPrintable(QStringLiteral("A%1").arg(i + 1))), QStringLiteral("i:é%1").arg(i));
                QCOMPARE(at1(wb, s, qPrintable(QStringLiteral("B%1").arg(i + 1))), QStringLiteral("i:q\nü %1").arg(i));
                QCOMPARE(at1(wb, s, qPrintable(QStringLiteral("C%1").arg(i + 1))), QStringLiteral("n:%1").arg(i * 3));
            }
        }
    }

    // A file that is not text is refused in words, and an empty one is an empty sheet; a file with the name of a text file that is a workbook of another kind is that kind.
    void whatIsNotTextIsRefused()
    {
        Workbook wb;
        QString why;
        // (a file with nothing in it is a sheet with nothing in it, which is what it was saved from)
        QVERIFY2(openCsv(QByteArray(), &wb, &why), qPrintable(why));
        QCOMPARE(wb.info().sheets.size(), 1);
        QCOMPARE(readSheet(wb).cells.cellCount(), qint64(0));
        QVERIFY(openCsv("\n\n", &wb, &why));
        QCOMPARE(readSheet(wb).cells.cellCount(), qint64(0));
        QVERIFY(!openCsv(QByteArray("a,b\0c,d\n", 8), &wb, &why));
        QVERIFY2(why.contains(QLatin1String("text file")), qPrintable(why));
        // an .xlsx with the name .csv
        const QByteArray xlsx = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>5</v></c></row>"))});
        Workbook x;
        QVERIFY2(openCsv(xlsx, &x, &why), qPrintable(why));
        QVERIFY(!x.isConverted());
        // a text file named for something else is not opened as values
        QVERIFY(!openBytes("a,b\n", QStringLiteral("data.dat"), &wb, &why));
    }

    // Written as the .xlsx the other readers write: the values, the formats of a percent and a date, a formula, and a text with a line break read back the same.
    void aFileIsWrittenAsXlsx()
    {
        LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
        Workbook wb;
        QString why;
        QVERIFY2(openCsv("12%,3/15/2024,=A1*2,\"two\nlines\",007\n", &wb, &why), qPrintable(why));
        QByteArray xlsx;
        QVERIFY2(writeBaseXlsx(wb, &xlsx, &why), qPrintable(why));
        Workbook back;
        QVERIFY2(openBytes(xlsx, QStringLiteral("back.xlsx"), &back, &why), qPrintable(why));
        QVERIFY(!back.isConverted());
        const Sheet a = readSheet(wb), b = readSheet(back);
        QCOMPARE(back.styles().formatCode(at(b, QStringLiteral("A1"))->style), QStringLiteral("0%"));
        QCOMPARE(at1(back, b, "A1"), QStringLiteral("n:0.12"));
        QCOMPARE(at1(back, b, "B1"), at1(wb, a, "B1"));
        QCOMPARE(formulaOf(b, QStringLiteral("C1")), QStringLiteral("A1*2"));
        QCOMPARE(at1(back, b, "D1").mid(1), QStringLiteral(":two\nlines"));
        QCOMPARE(at1(back, b, "E1").mid(1), QStringLiteral(":007"));
    }
};

JP_TEST_CLASS(SheetCsvTests)
#include "tst_sheet_csv.moc"
