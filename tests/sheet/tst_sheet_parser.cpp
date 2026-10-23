// Tests of the formula parser (calc/formula.h): what a formula as a file stores it parses to, shown as the program's postfix text (Program::dump), the order
// of the operators, the references of every kind, array constants, the prefixes of newer functions, what is refused, and the bounds on hostile text.
// Every expected text below is written from the language's own rules (the operators group from the left, a minus sign binds tighter than "^", and so on), never
// from what the parser printed.
#include "calchelpers.h"

#include <QRandomGenerator>

#include <algorithm>

using namespace jp::calc;

namespace {

struct Case {
    const char *formula;
    const char *dump;
};

QString dump(const QString &formula, const ParseOptions &options = ParseOptions())
{
    const ParseResult r = parseFormula(formula, options);
    return r.ok() ? r.program->dump() : QStringLiteral("!") + r.error;
}

ParseResult parsed(const QString &formula, const ParseOptions &options = ParseOptions()) { return parseFormula(formula, options); }

// The program of a formula that parses; a program with nothing in it for one that doesn't, which the checks then find wrong.
Program progOf(const QString &formula, const ParseOptions &options = ParseOptions())
{
    const ParseResult r = parseFormula(formula, options);
    return r.ok() ? *r.program : Program();
}

#define CHECK_CASES(cases, options)                                                                                                                         \
    for (const auto &c : cases) {                                                                                                                           \
        const QString got = dump(QString::fromUtf8(c.formula), options);                                                                                    \
        QVERIFY2(got == QString::fromUtf8(c.dump), qPrintable(QString::fromUtf8(c.formula) + QStringLiteral(" parsed to [") + got + QStringLiteral("], expected [") \
                                                                   + QString::fromUtf8(c.dump) + QLatin1Char(']')));                                      \
    }

} // namespace

class SheetParserTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void literalsParseToTheirValues()
    {
        const Case cases[] = {
            {"1", "1"}, {"1.5", "1.5"}, {".5", "0.5"}, {"1E5", "100000"}, {"1.5e-3", "0.0015"}, {"1E+3", "1000"}, {"007", "7"},
            {"\"abc\"", "\"abc\""}, {"\"\"", "\"\""}, {"\"a\"\"b\"", "\"a\"\"b\""}, {"\"two\nlines\"", "\"two\nlines\""},
            {"TRUE", "TRUE"}, {"false", "FALSE"}, {"True", "TRUE"},
            {"#DIV/0!", "#DIV/0!"}, {"#N/A", "#N/A"}, {"#NAME?", "#NAME?"}, {"#NULL!", "#NULL!"}, {"#NUM!", "#NUM!"}, {"#REF!", "#REF!"}, {"#VALUE!", "#VALUE!"},
            {"#n/a", "#N/A"},
        };
        CHECK_CASES(cases, ParseOptions());
    }

    void operatorsGroupFromTheLeftAndBindInTheirOrder()
    {
        const Case cases[] = {
            {"1+2*3", "1 2 3 * +"},
            {"(1+2)*3", "1 2 + 3 *"},
            {"-2^2", "2 neg 2 ^"},                  // a minus sign in front binds tighter than "^": (-2)^2
            {"2^-2", "2 2 neg ^"},
            {"1-2-3", "1 2 - 3 -"},                 // (1-2)-3, not 1-(2-3)
            {"2^3^2", "2 3 ^ 2 ^"},                 // (2^3)^2 = 64: "^" groups from the left
            {"2^-3^2", "2 3 neg ^ 2 ^"},
            {"\"a\"&1+1", "\"a\" 1 1 + &"},         // "+" binds tighter than "&"
            {"1+2=3&4", "1 2 + 3 4 & ="},           // "&" binds tighter than "="
            {"1&2&3", "1 2 & 3 &"},
            {"1*2/3*4", "1 2 * 3 / 4 *"},
            {"10-2*3+1", "10 2 3 * - 1 +"},
            {"5%", "5 pct"},
            {"-5%", "5 neg pct"},                   // the minus sign first, then the percent
            {"2*3%", "2 3 pct *"},
            {"A1^2%", "A1 2 pct ^"},
            {"-A1^2", "A1 neg 2 ^"},
            {"--1", "1 neg neg"},
            {"+1", "1"},                            // a plus sign in front changes nothing
            {"-+-1", "1 neg neg"},
            {"1--1", "1 1 neg -"},
            {"1+-2", "1 2 neg +"},
            {"1<2", "1 2 <"}, {"1<=2", "1 2 <="}, {"1>2", "1 2 >"}, {"1>=2", "1 2 >="}, {"1<>2", "1 2 <>"}, {"1=2", "1 2 ="},
            {"1=2=3", "1 2 = 3 ="},
            {"1+2<3*4", "1 2 + 3 4 * <"},
            {"@A1:A3", "A1:A3 @"},
            {"A1#", "A1 #"},
        };
        CHECK_CASES(cases, ParseOptions());
    }

    void referencesOfEveryKind()
    {
        const Case cases[] = {
            {"A1", "A1"}, {"$A$1", "$A$1"}, {"a1", "A1"}, {"$a1", "$A1"}, {"A$1", "A$1"}, {"XFD1048576", "XFD1048576"},
            {"A1:B2", "A1:B2"}, {"B2:A1", "A1:B2"}, {"$B$2:a1", "A1:$B$2"}, {"A1:A1", "A1:A1"},
            {"A:A", "A:A"}, {"$A:$C", "$A:$C"}, {"c:a", "A:C"}, {"1:1", "1:1"}, {"$1:$3", "$1:$3"}, {"3:1", "1:3"},
            {"Sheet2!A1", "Sheet2!A1"}, {"sheet2!a1", "sheet2!A1"}, {"'Sheet2'!A1", "Sheet2!A1"}, {"'My Sheet'!A1:B2", "'My Sheet'!A1:B2"},
            {"'It''s'!A1", "'It''s'!A1"}, {"Sheet2!A:A", "Sheet2!A:A"}, {"Sheet2!1:2", "Sheet2!1:2"}, {"Sheet2!A1:B2", "Sheet2!A1:B2"},
            {"Sheet1:Sheet3!A1", "Sheet1:Sheet3!A1"}, {"'Sheet 1:Sheet 3'!B2", "'Sheet 1:Sheet 3'!B2"}, {"Sheet1:Sheet3!A1:B2", "Sheet1:Sheet3!A1:B2"},
            {"'A!B'!A1", "'A!B'!A1"}, {"'2020'!A1", "'2020'!A1"}, {"Caf\xc3\xa9!A1", "Caf\xc3\xa9!A1"}, {"Sheet.1!A1", "Sheet.1!A1"},
            {"SUM(A1:A3)", "A1:A3 SUM/1"},
            // a word followed by "(" is a call, not a cell
            {"LOG10(100)", "100 LOG10/1"}, {"ATAN2(1,1)", "1 1 ATAN2/2"},
            // (and without the "(" they are cells: LOG is a column, as 16,384 columns reach past it)
            {"LOG10", "LOG10"}, {"ATAN2", "name:ATAN2"},
        };
        CHECK_CASES(cases, ParseOptions());
    }

    void namesAreNotCells()
    {
        const Case cases[] = {
            {"TaxRate", "name:TaxRate"}, {"Sheet1!TaxRate", "name:Sheet1!TaxRate"}, {"'My Sheet'!Total", "name:'My Sheet'!Total"}, {"_xlnm.Print_Area", "name:_xlnm.Print_Area"},
            {"A1B", "name:A1B"}, {"XFE1", "name:XFE1"}, {"A1048577", "name:A1048577"}, {"R1C1", "name:R1C1"}, {"my.name", "name:my.name"}, {"Q1_total", "name:Q1_total"},
            {"AB", "name:AB"}, {"TaxRate*A1", "name:TaxRate A1 *"},
        };
        CHECK_CASES(cases, ParseOptions());
    }

    void callsAndTheirPrefixes()
    {
        const Case cases[] = {
            {"SUM(1,2)", "1 2 SUM/2"}, {"PI()", "PI/0"}, {"sum(a1:a3)", "A1:A3 SUM/1"}, {"SUM(ABS(-1),2)", "1 neg ABS/1 2 SUM/2"},
            {"SUM( A1 , B1 )", "A1 B1 SUM/2"},
            // arguments left out
            {"CONCATENATE(,\"a\")", "_ \"a\" CONCATENATE/2"}, {"SUM(,)", "_ _ SUM/2"}, {"SUM(1,)", "1 _ SUM/2"},
            // IF, IFERROR, and IFNA work out only the argument they need: a jump past the other
            {"IF(A1,1,2)", "A1 ifjump 4 1 jump 5 2"},
            {"IFERROR(A1,0)", "A1 tryjump 3 0"},
            {"_xlfn.IFNA(A1,0)", "A1 trynajump 3 0"},
            {"CHOOSE(A1,1,2)", "A1 choose 8 jump 4 jump 6 1 jump 8 2 jump 8"},
            // the prefixes newer files write are not part of the name
            {"_xlfn.STDEV.S(A1:A3)", "A1:A3 STDEV.S/1"}, {"_xlfn._xlws.SORT(A1)", "A1 SORT/1?"}, {"_xlws.FILTER(A1:A3,B1:B3)", "A1:A3 B1:B3 FILTER/2?"},
            {"_XLFN.IFNA(A1,0)", "A1 trynajump 3 0"},
            // (a function newer than the first release, written without the prefix, is some other function)
            {"IFNA(A1,0)", "A1 0 IFNA/2?"},
            // a function that is not in the registry
            {"FOO(1)", "1 FOO/1?"}, {"Foo.Bar(1,2)", "1 2 FOO.BAR/2?"},
            // a single value of a reference
            {"_xlfn.SINGLE(A1:A3)", "A1:A3 @"},
            {"A1:INDEX(B1:B5,2)", "A1 B1:B5 2 INDEX/2 :"}, {"INDEX(A1:A5,1):B2", "A1:A5 1 INDEX/2 B2 :"}, {"OFFSET(A1,1,1):C3", "A1 1 1 OFFSET/3 C3 :"},
        };
        CHECK_CASES(cases, ParseOptions());
        QCOMPARE(progOf("FOO(1)+BAR(2)+foo(3)").unknownFunctions, QStringList({"FOO", "BAR"}));
        QVERIFY(!progOf("FOO(1)").supported());
        QVERIFY(progOf("SUM(1)").supported());
    }

    void callsWithTheWrongNumberOfArgumentsAreRefused()
    {
        for (const char *f : {"ABS()", "ABS(1,2)", "PI(1)", "IF()", "IF(1,2,3,4)", "SUM()", "LEFT()", "IFERROR(1)", "NOT(1,2)", "CHOOSE(1)"})
            QVERIFY2(!parsed(QString::fromLatin1(f)).ok(), f);
    }

    void arrayConstants()
    {
        const Case cases[] = {
            {"{1,2;3,4}", "{1,2;3,4}"}, {"{1;2;3}", "{1;2;3}"}, {"{1,2,3}", "{1,2,3}"}, {"{\"a\",TRUE,#N/A}", "{\"a\",TRUE,#N/A}"}, {"{-1,+2,-1.5}", "{-1,2,-1.5}"},
            {"{1,2}+1", "{1,2} 1 +"}, {"SUM({1,2;3,4})", "{1,2;3,4} SUM/1"}, {"{ 1 , 2 ; 3 , 4 }", "{1,2;3,4}"},
        };
        CHECK_CASES(cases, ParseOptions());
        const Program arr = progOf("{1,2;3,4}");
        QCOMPARE(arr.arrays.size(), size_t(1));
        QCOMPARE(arr.arrays[0].rows(), 2);
        QCOMPARE(arr.arrays[0].cols(), 2);
        QCOMPARE(arr.arrays[0].at(1, 0).debug(), QStringLiteral("n:3"));
        for (const char *f : {"{1,2;3}", "{}", "{1,}", "{A1}", "{1,2", "{1,SUM(2)}", "{{1}}", "{1;}"})
            QVERIFY2(!parsed(QString::fromLatin1(f)).ok(), f);
    }

    void spacesAreInsignificantExceptBetweenReferences()
    {
        const Case cases[] = {
            {" 1 + 2 ", "1 2 +"}, {"1 +\n2", "1 2 +"}, {"A1 + B1", "A1 B1 +"}, {"A1 B1", "A1 B1 isect"}, {"SUM(A1:B2 B1:C3)", "A1:B2 B1:C3 isect SUM/1"},
            {"A1:B2 B1:C3 C1:D4", "A1:B2 B1:C3 isect C1:D4 isect"}, {"(A1:A2,B1:B2)", "A1:A2 B1:B2 ,"}, {"SUM((A1:A2,B1:B2),C1)", "A1:A2 B1:B2 , C1 SUM/2"},
            {"SUM(A1:A2,B1:B2)", "A1:A2 B1:B2 SUM/2"}, {"A1 :B2", "A1 B2 :"},
        };
        CHECK_CASES(cases, ParseOptions());
    }

    void tableReferencesAreParsedAndKept()
    {
        const Case cases[] = {
            {"Table1[Col]", "table:Table1[Col]"}, {"Table1[[#This Row],[Col]]", "table:Table1[[#This Row],[Col]]"}, {"[@Col]", "table:[@Col]"}, {"Table1[#All]", "table:Table1[#All]"},
            {"SUM(Table1[Sales])*2", "table:Table1[Sales] SUM/1 2 *"}, {"Table1[[Col1]:[Col2]]", "table:Table1[[Col1]:[Col2]]"}, {"Table1[@[My Col]]", "table:Table1[@[My Col]]"},
            {"Table1[Qty ('000)]", "table:Table1[Qty ('000)]"}, {"Table1[]", "table:Table1[]"},
        };
        CHECK_CASES(cases, ParseOptions());
        QVERIFY(progOf("SUM(Table1[Sales])").structured);
        QVERIFY(!progOf("SUM(Table1[Sales])").supported());
        QVERIFY(!progOf("SUM(A1:A3)").structured);
    }

    void referencesToOtherWorkbooksAreParsedAndKept()
    {
        const Case cases[] = {
            {"[1]Sheet1!A1", "ext:[1]Sheet1!A1"}, {"'[1]Sheet 1'!A1:B2", "ext:'[1]Sheet 1'!A1:B2"}, {"SUM([2]Data!A1:A5)", "ext:[2]Data!A1:A5 SUM/1"}, {"[1]!MyName", "ext:[1]!MyName"},
            {"'C:\\dir\\[b.xlsx]Sheet1'!A1", "ext:'C:\\dir\\[b.xlsx]Sheet1'!A1"}, {"[1]Sheet1!A1+1", "ext:[1]Sheet1!A1 1 +"},
        };
        CHECK_CASES(cases, ParseOptions());
        QVERIFY(progOf("[1]Sheet1!A1").external);
        QVERIFY(!progOf("[1]Sheet1!A1").supported());
        QVERIFY(!progOf("Sheet1!A1").external);
    }

    void r1c1ReferencesAreMadeRelativeToTheirCell()
    {
        ParseOptions at;
        at.r1c1 = true;
        at.row = 4;   // E5
        at.col = 4;
        const Case cases[] = {
            {"R[-1]C[-1]", "D4"}, {"RC", "E5"}, {"R1C1", "$A$1"}, {"R[1]C", "E6"}, {"RC[2]", "G5"}, {"R2C3:R4C5", "$C$2:$E$4"}, {"SUM(R[-1]C:R[-3]C)", "E2:E4 SUM/1"},
            {"R1C1+R[1]C[1]", "$A$1 F6 +"}, {"Sheet2!R1C1", "Sheet2!$A$1"}, {"R[-1]C1", "$A4"}, {"R4C[-1]", "D$4"},
        };
        CHECK_CASES(cases, at);
        QVERIFY(!parsed("R[-9]C", at).ok());   // above the first row
        // A1 text is not R1C1 text and the other way round.
        QCOMPARE(dump(QStringLiteral("A1"), at), QStringLiteral("name:A1"));
    }

    void aDefinedNameMayBeAUnionAtTheTopLevel()
    {
        ParseOptions name;
        name.definedName = true;
        QCOMPARE(dump(QStringLiteral("Sheet1!$A$1:$B$2,Sheet1!$D$1:$E$2"), name), QStringLiteral("Sheet1!$A$1:$B$2 Sheet1!$D$1:$E$2 ,"));
        QCOMPARE(dump(QStringLiteral("A1,B1,C1"), name), QStringLiteral("A1 B1 , C1 ,"));
        QVERIFY(!parsed("A1,B1").ok());
    }

    void theFlagsSayWhatAFormulaUses()
    {
        for (const char *f : {"NOW()", "TODAY()+1", "RAND()", "RANDBETWEEN(1,2)", "OFFSET(A1,1,1)", "INDIRECT(\"A1\")", "SUM(OFFSET(A1,0,0,2,2))", "IF(A1,NOW(),0)"})
            QVERIFY2(progOf(QString::fromLatin1(f)).isVolatile, f);
        for (const char *f : {"SUM(A1)", "IF(A1,1,2)", "A1+B1", "ROW()", "VLOOKUP(1,A1:B2,2,FALSE)"})
            QVERIFY2(!progOf(QString::fromLatin1(f)).isVolatile, f);
        QVERIFY(functionIndex("NOW") >= 0 && (functionAt(functionIndex("NOW")).flags & kFnVolatile));
        QVERIFY(functionIndex("SUM") >= 0 && !(functionAt(functionIndex("SUM")).flags & kFnVolatile));
        const Program counted = progOf("SUM(1)+SUM(2)+ABS(1)");
        QCOMPARE(counted.calls.size(), size_t(2));
        int sums = 0, abs = 0;
        for (const auto &c : counted.calls) {
            if (c.first == functionIndex("SUM")) sums = c.second;
            if (c.first == functionIndex("ABS")) abs = c.second;
        }
        QCOMPARE(sums, 2);
        QCOMPARE(abs, 1);
    }

    void theRegistryKnowsItsFunctions()
    {
        QVERIFY(findFunction("sum") != nullptr);
        QVERIFY(findFunction("_xlfn.IFNA") != nullptr);
        QVERIFY(findFunction("_xlws.FILTER") == nullptr);
        QVERIFY(findFunction("NoSuchFunction") == nullptr);
        QCOMPARE(functionIndex("NoSuchFunction"), -1);
        const FunctionInfo *sum = findFunction("SUM");
        QVERIFY(sum != nullptr);
        QCOMPARE(QString::fromLatin1(sum->name), QStringLiteral("SUM"));
        QCOMPARE(int(sum->minArgs), 1);
        QCOMPARE(int(sum->maxArgs), 255);
        QCOMPARE(&functionAt(functionIndex("SUM")), sum);
        // every registered function has a name in upper case, an implementation, and a letter for its parameters
        for (int i = 0; i < functionCount(); ++i) {
            const FunctionInfo &f = functionAt(i);
            const QString name = QString::fromLatin1(f.name);
            QVERIFY2(name == name.toUpper() && f.impl, qPrintable(name));
            // a letter for each parameter, the last standing for more when the function takes more arguments than it has letters
            QVERIFY2((f.maxArgs == 0) == (strlen(f.params) == 0) && (f.maxArgs == 255 || int(strlen(f.params)) <= int(f.maxArgs)), qPrintable(name));
            QVERIFY2(f.minArgs <= f.maxArgs, qPrintable(name));
            QCOMPARE(functionIndex(name), i);
        }
    }

    void textThatIsNoFormulaIsRefused()
    {
        for (const char *f : {"", " ", "1+", "(1", "1)", "SUM(1,", "\"abc", "*1", "1**2", "SUM(1;2)", "{1,2", "#FOO", "#N/A!", "A1:", "'abc", "%", "=1+1", "1+*2", "SUM(,", ")", "A1,B1",
                              "1 +", "SUM(1)(2)", "A1 !B1", "Sheet1!", "!A1", "$", "$$A1", "A$$1", "[", "Table1[", "1:", ":1", "Sheet1!A1:"})
            QVERIFY2(!parsed(QString::fromLatin1(f)).ok(), f);
        const ParseResult r = parsed("1+");
        QVERIFY(!r.error.isEmpty());
        QCOMPARE(r.errorPos, 2);
    }

    void hostileTextIsBounded()
    {
        auto repeat = [](const QString &piece, int n) { return piece.repeated(n); };
        // nesting
        QVERIFY(parsed(repeat("(", 100) + "1" + repeat(")", 100)).ok());
        QVERIFY(!parsed(repeat("(", 200) + "1" + repeat(")", 200)).ok());
        QVERIFY(parsed(repeat("ABS(", 100) + "1" + repeat(")", 100)).ok());
        QVERIFY(!parsed(repeat("ABS(", 200) + "1" + repeat(")", 200)).ok());
        QVERIFY(!parsed(repeat("IF(1,", 5000) + "1").ok());
        QVERIFY(!parsed(repeat("{", 5000)).ok());
        QVERIFY(!parsed(repeat("(", 30000)).ok());
        QVERIFY(!parsed(repeat("-", 30000) + "(").ok());
        // a chain of any length is flat, not nested
        QVERIFY(parsed(repeat("1+", 10000) + "1").ok());
        QVERIFY(parsed(repeat("-", 10000) + "1").ok());
        QVERIFY(parsed("1" + repeat("%", 10000)).ok());
        QVERIFY(parsed(repeat("A1&", 8000) + "A1").ok());
        // length
        QVERIFY(parsed("\"" + repeat("x", 32000) + "\"").ok());
        QVERIFY(!parsed("\"" + repeat("x", 40000) + "\"").ok());
        QVERIFY(!parsed(repeat("1+", 20000) + "1").ok());
        // arguments
        QVERIFY(parsed("SUM(" + repeat("1,", 999) + "1)").ok());
        QVERIFY(!parsed("SUM(" + repeat("1,", 1000) + "1)").ok());
        QVERIFY(!parsed("SUM(" + repeat(",", 5000) + ")").ok());
        // arrays
        QVERIFY(parsed("{" + repeat("1,", 15000) + "1}").ok());
        QVERIFY(!parsed("{" + repeat("1;", 20000) + "1}").ok());
        // long words, quoted names, brackets, numbers
        QVERIFY(parsed(repeat("a", 900)).ok());     // a name (the format allows 255 characters)
        QVERIFY(!parsed(repeat("a", 30000)).ok());
        QVERIFY(!parsed("'" + repeat("a", 30000)).ok());
        QVERIFY(!parsed("Table1[" + repeat("[", 30000)).ok());
        QVERIFY(parsed(repeat("9", 300)).ok());
        QVERIFY(!parsed(repeat("9", 400)).ok());    // past the largest number
        QVERIFY(!parsed(repeat("9", 30000)).ok());
        QVERIFY(!parsed("1E" + repeat("9", 30000) + "+").ok());
    }

    void randomTextNeverCrashesOrLoops()
    {
        static const char *const pieces[] = {"(", ")", "+", "-", "*", "/", "^", "&", "=", "<>", "<", ">=", ":", ",", ";", "!", "'", "\"", "{", "}", "[", "]", "#", "%", "@", " ", "$", ".",
                                             "0", "1", "12", "3.5", "1E5", "A", "B2", "$C$3", "AB", "R", "C", "R1C1", "Sheet1", "Sheet2!", "'x y'!", "SUM", "IF(", "SUM(", "ABS(", "IFERROR(",
                                             "CHOOSE(", "TRUE", "#N/A", "#REF!", "Table1[", "[1]", "_xlfn.", "A1:B2", "1:2", "A:B", "\n", "\xc3\xa9", "\\", "?", "~"};
        QRandomGenerator rng(20261022);
        int accepted = 0;
        for (int i = 0; i < 40000; ++i) {
            QString text;
            const int parts = 1 + int(rng.bounded(14));
            for (int k = 0; k < parts; ++k) text += QString::fromUtf8(pieces[rng.bounded(quint32(sizeof(pieces) / sizeof(pieces[0])))]);
            const ParseResult r = parseFormula(text);
            if (r.ok()) {
                ++accepted;
                QVERIFY2(!r.program->dump().isEmpty() || r.program->code.empty(), qPrintable(text));
            } else {
                QVERIFY2(!r.error.isEmpty(), qPrintable(text));
                QVERIFY2(r.errorPos >= 0 && r.errorPos <= text.size(), qPrintable(text));
            }
        }
        // and in R1C1, and as arrays
        ParseOptions r1;
        r1.r1c1 = true;
        r1.arrayFormula = true;
        for (int i = 0; i < 10000; ++i) {
            QString text;
            const int parts = 1 + int(rng.bounded(14));
            for (int k = 0; k < parts; ++k) text += QString::fromUtf8(pieces[rng.bounded(quint32(sizeof(pieces) / sizeof(pieces[0])))]);
            parseFormula(text, r1);
        }
        QVERIFY(accepted > 1000);   // the pieces make formulas often enough for the test to mean something
    }
};

JP_TEST_CLASS(SheetParserTests)
#include "tst_sheet_parser.moc"
