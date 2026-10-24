// Tests of the reader of delimited text added after its review: a text that begins with the letters of a package is still text, quotes where a field begins hide the delimiter whatever
// the line, a day and a month without a year stay text in every form and language, and true and false are read in the person's language.
#include "odshelpers.h"

#include "calc/dates.h"
#include "sheet/csv.h"

namespace {

struct LocaleScope {
    QLocale old;
    explicit LocaleScope(const QLocale &l) : old() { QLocale::setDefault(l); }
    ~LocaleScope() { QLocale::setDefault(old); }
};

} // namespace

class SheetCsvReviewTests : public QObject {
    Q_OBJECT

    static bool openCsv(const QByteArray &bytes, Workbook *wb, QString *why, const QString &name = QStringLiteral("data.csv")) { return openBytes(bytes, name, wb, why); }
    static QString at1(const Workbook &wb, const Sheet &s, const char *ref) { return shown(wb, s, QLatin1String(ref)); }

private Q_SLOTS:
    // Item 5: text that begins with the two letters a package begins with is text; only the signature of a package makes it one.
    void textThatBeginsWithPkIsText()
    {
        LocaleScope locale(QLocale::c());
        Workbook wb;
        QString why;
        QVERIFY2(openCsv("PK,Name\n1,2\n", &wb, &why), qPrintable(why));
        QVERIFY(wb.isConverted());
        QCOMPARE(wb.convertedKind(), QStringLiteral(".csv"));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(at1(wb, s, "A1"), QStringLiteral("i:PK"));
        QCOMPARE(at1(wb, s, "B2"), QStringLiteral("n:2"));
        Workbook tsv;
        QVERIFY2(openBytes("PK\t1\n", QStringLiteral("x.tsv"), &tsv, &why), qPrintable(why));
        Workbook txt;
        QVERIFY2(openBytes("PK\x03 not a package, only text\n", QStringLiteral("x.txt"), &txt, &why), qPrintable(why));
        // a package with the name of a text file is still a package
        const QByteArray xlsx = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>5</v></c></row>"))});
        Workbook x;
        QVERIFY2(openCsv(xlsx, &x, &why), qPrintable(why));
        QVERIFY(!x.isConverted());
    }

    // Item 6: a quote where a field begins hides the delimiter in it, in the first lines too: the delimiter is found as the fields are.
    void quotesWhereAFieldBeginsAreHonored()
    {
        const QLocale c = QLocale::c();
        QCOMPARE(detectCsvForm("a;\"x,y,z,w\";b\na;\"x,y,z,w\";b\na;\"x,y,z,w\";b\n", QStringLiteral("x.csv"), c).delimiter, QLatin1Char(';'));
        QCOMPARE(detectCsvForm("a,\"x;y;z;w\",b\n1,\"x;y;z;w\",2\n", QStringLiteral("x.csv"), c).delimiter, QLatin1Char(','));
        QCOMPARE(detectCsvForm("a\t\"x,y,z,w\"\tb\n1\t\"x,y,z,w\"\t2\n", QStringLiteral("x.csv"), c).delimiter, QLatin1Char('\t'));
        QCOMPARE(detectCsvForm("a|\"x;y,z\"|b\n", QStringLiteral("x.csv"), c).delimiter, QLatin1Char('|'));
        // a quote that does not begin a field is a character, as it is when the file is read
        QCOMPARE(detectCsvForm("5\" pipe;a;b\n6\" pipe;c;d\n", QStringLiteral("x.csv"), c).delimiter, QLatin1Char(';'));
        LocaleScope locale(QLocale::c());
        Workbook wb;
        QString why;
        QVERIFY2(openCsv("a;\"x,y,z,w\";b\n1;\"p,q,r,s\";2\n", &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(at1(wb, s, "B1"), QStringLiteral("i:x,y,z,w"));
        QCOMPARE(at1(wb, s, "C2"), QStringLiteral("n:2"));
    }

    // Item 7: a day and a month without a year stay text in every form, order, and language; a date with a year is a date in the person's order.
    void aDayAndMonthWithoutAYearStayText()
    {
        double march15 = 0, jan5 = 0, oct7 = 0;
        QVERIFY(jp::calc::dateToSerial(2024, 3, 15, jp::calc::DateSystem::Date1900, &march15));
        QVERIFY(jp::calc::dateToSerial(2024, 5, 1, jp::calc::DateSystem::Date1900, &jan5));
        QVERIFY(jp::calc::dateToSerial(2024, 3, 1, jp::calc::DateSystem::Date1900, &oct7));
        {
            LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
            Workbook wb;
            QString why;
            QVERIFY2(openCsv("1 2,1-2,3/4,1.5,12/25,March 1,1 March,Sep-01,Dec 25,3/15/2024,March 1 2024,1 March 2024,Mar-2024,1/2/03,2024-03-15,1/5/2024 10:30\n", &wb, &why), qPrintable(why));
            Sheet s;
            QVERIFY(wb.readSheet(0, &s));
            QCOMPARE(at1(wb, s, "A1"), QStringLiteral("i:1 2"));
            QCOMPARE(at1(wb, s, "B1"), QStringLiteral("i:1-2"));
            QCOMPARE(at1(wb, s, "C1"), QStringLiteral("i:3/4"));
            QCOMPARE(at1(wb, s, "D1"), QStringLiteral("n:1.5"));   // (a number here)
            QCOMPARE(at1(wb, s, "E1"), QStringLiteral("i:12/25"));
            QCOMPARE(at1(wb, s, "F1"), QStringLiteral("i:March 1"));
            QCOMPARE(at1(wb, s, "G1"), QStringLiteral("i:1 March"));
            QCOMPARE(at1(wb, s, "H1"), QStringLiteral("i:Sep-01"));
            QCOMPARE(at1(wb, s, "I1"), QStringLiteral("i:Dec 25"));
            QCOMPARE(at1(wb, s, "J1"), QStringLiteral("n:") + QString::number(march15, 'g', 17));
            QCOMPARE(at1(wb, s, "K1"), QStringLiteral("n:") + QString::number(oct7, 'g', 17));
            QCOMPARE(at1(wb, s, "L1"), QStringLiteral("n:") + QString::number(oct7, 'g', 17));
            QVERIFY2(at1(wb, s, "M1").startsWith(QLatin1String("n:")), qPrintable(at1(wb, s, "M1")));
            QVERIFY2(at1(wb, s, "N1").startsWith(QLatin1String("n:")), qPrintable(at1(wb, s, "N1")));
            QVERIFY2(at1(wb, s, "O1").startsWith(QLatin1String("n:")), qPrintable(at1(wb, s, "O1")));
            QVERIFY2(at1(wb, s, "P1").startsWith(QLatin1String("n:")), qPrintable(at1(wb, s, "P1")));   // (a date with a time)
        }
        {
            LocaleScope locale(QLocale(QLocale::German, QLocale::Germany));
            Workbook wb;
            QString why;
            QVERIFY2(openCsv("1.5;12.5;3.4;7.10;1,5;15.03.2024;1.5.2024;1.3.;5. März;März 2024;5-3\n", &wb, &why), qPrintable(why));
            Sheet s;
            QVERIFY(wb.readSheet(0, &s));
            QCOMPARE(at1(wb, s, "A1"), QStringLiteral("i:1.5"));
            QCOMPARE(at1(wb, s, "B1"), QStringLiteral("i:12.5"));
            QCOMPARE(at1(wb, s, "C1"), QStringLiteral("i:3.4"));
            QCOMPARE(at1(wb, s, "D1"), QStringLiteral("i:7.10"));
            QCOMPARE(at1(wb, s, "E1"), QStringLiteral("n:1.5"));
            QCOMPARE(at1(wb, s, "F1"), QStringLiteral("n:") + QString::number(march15, 'g', 17));
            QCOMPARE(at1(wb, s, "G1"), QStringLiteral("n:") + QString::number(jan5, 'g', 17));
            QCOMPARE(at1(wb, s, "H1"), QStringLiteral("i:1.3."));
            QCOMPARE(at1(wb, s, "I1"), QStringLiteral("i:5. März"));
            QVERIFY2(at1(wb, s, "J1").startsWith(QLatin1String("n:")), qPrintable(at1(wb, s, "J1")));
            QCOMPARE(at1(wb, s, "K1"), QStringLiteral("i:5-3"));
        }
    }

    // Item 8: true and false are read in the person's language (and in English), as the common spreadsheet program reads them, in any case.
    void booleansAreReadInThePersonsLanguage()
    {
        struct Case {
            QLocale::Language language;
            QLocale::Country country;
            const char *yes;
            const char *no;
        };
        const Case cases[] = {{QLocale::German, QLocale::Germany, "WAHR", "FALSCH"},       {QLocale::French, QLocale::France, "VRAI", "FAUX"},
                              {QLocale::Spanish, QLocale::Spain, "VERDADERO", "FALSO"},    {QLocale::Italian, QLocale::Italy, "VERO", "FALSO"},
                              {QLocale::Dutch, QLocale::Netherlands, "WAAR", "ONWAAR"},    {QLocale::Portuguese, QLocale::Brazil, "VERDADEIRO", "FALSO"},
                              {QLocale::Swedish, QLocale::Sweden, "SANT", "FALSKT"},       {QLocale::Danish, QLocale::Denmark, "SAND", "FALSK"},
                              {QLocale::NorwegianBokmal, QLocale::Norway, "SANN", "USANN"},      {QLocale::Finnish, QLocale::Finland, "TOSI", "EPÄTOSI"},
                              {QLocale::Polish, QLocale::Poland, "PRAWDA", "FAŁSZ"},       {QLocale::Czech, QLocale::CzechRepublic, "PRAVDA", "NEPRAVDA"},
                              {QLocale::Hungarian, QLocale::Hungary, "IGAZ", "HAMIS"},     {QLocale::Russian, QLocale::Russia, "ИСТИНА", "ЛОЖЬ"},
                              {QLocale::Turkish, QLocale::Turkey, "DOĞRU", "YANLIŞ"},      {QLocale::Greek, QLocale::Greece, "ΑΛΗΘΕΣ", "ΨΕΥΔΕΣ"}};
        for (const Case &c : cases) {
            LocaleScope locale(QLocale(c.language, c.country));
            const QString yes = QString::fromUtf8(c.yes), no = QString::fromUtf8(c.no);
            const QString line = yes + QStringLiteral(";") + no.toLower() + QStringLiteral(";") + yes.toLower().left(1).toUpper() + yes.toLower().mid(1) + QStringLiteral(";TRUE;false;x\n");
            Workbook wb;
            QString why;
            QVERIFY2(openCsv(line.toUtf8(), &wb, &why), qPrintable(why));
            Sheet s;
            QVERIFY(wb.readSheet(0, &s));
            QCOMPARE(at1(wb, s, "A1"), QStringLiteral("b:1"));
            QCOMPARE(at1(wb, s, "B1"), QStringLiteral("b:0"));
            QCOMPARE(at1(wb, s, "C1"), QStringLiteral("b:1"));
            QCOMPARE(at1(wb, s, "D1"), QStringLiteral("b:1"));
            QCOMPARE(at1(wb, s, "E1"), QStringLiteral("b:0"));
            QCOMPARE(at1(wb, s, "F1"), QStringLiteral("i:x"));
        }
        // in English they are texts
        LocaleScope english(QLocale(QLocale::English, QLocale::UnitedStates));
        Workbook wb;
        QString why;
        QVERIFY2(openCsv("WAHR,VRAI,true\n", &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(at1(wb, s, "A1"), QStringLiteral("i:WAHR"));
        QCOMPARE(at1(wb, s, "B1"), QStringLiteral("i:VRAI"));
        QCOMPARE(at1(wb, s, "C1"), QStringLiteral("b:1"));
    }
};

JP_TEST_CLASS(SheetCsvReviewTests)
#include "tst_sheet_csvreview.moc"
