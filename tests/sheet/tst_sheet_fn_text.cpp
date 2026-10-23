// Tests of the text functions: cutting, finding, joining, changing case, and turning values to text and text to values (TEXT uses the number formats of
// calc/numfmt.h). Each expected value is written from the function's definition and the rules of the language, not from what the code gave.
#include "calchelpers.h"

using namespace jp::calc;

namespace {

struct E {
    const char *formula;
    const char *value;
};

#define CHECK_EVALS(cases, grid, settings)                                                                                                                  \
    for (const auto &c : cases) {                                                                                                                           \
        const QString got = ev(QString::fromUtf8(c.formula), grid, settings);                                                                               \
        QVERIFY2(got == QString::fromUtf8(c.value), qPrintable(QString::fromUtf8(c.formula) + QStringLiteral(" gave [") + got + QStringLiteral("], expected [") \
                                                                   + QString::fromUtf8(c.value) + QLatin1Char(']')));                                      \
    }

} // namespace

class SheetFnTextTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void cuttingText()
    {
        Grid g;
        g.n("A1", 12345).s("A2", "hello").b("A3", true);
        const E cases[] = {
            {"LEN(\"hello\")", "n:5"},
            {"LEN(123)", "n:3"},
            {"LEN(\"\")", "n:0"},
            {"LEN(A1)", "n:5"},
            {"LEN(A9)", "n:0"},
            {"LEN(A3)", "n:4"},
            {"LEN(1/3)", "n:17"},                   // 0.333333333333333
            {"LEFT(\"hello\",2)", "s:he"},
            {"LEFT(\"hello\")", "s:h"},
            {"LEFT(\"hello\",0)", "s:"},
            {"LEFT(\"hi\",10)", "s:hi"},
            {"LEFT(\"x\",-1)", "e:#VALUE!"},
            {"LEFT(12345,2)", "s:12"},
            {"LEFT(A1,3)", "s:123"},
            {"LEFT(\"hello\",2.9)", "s:he"},
            {"LEFT(\"hello\",\"2\")", "s:he"},
            {"LEFT(#N/A)", "e:#N/A"},
            {"RIGHT(\"hello\",2)", "s:lo"},
            {"RIGHT(\"hello\")", "s:o"},
            {"RIGHT(\"hello\",0)", "s:"},
            {"RIGHT(\"hi\",10)", "s:hi"},
            {"RIGHT(12345,2)", "s:45"},
            {"RIGHT(\"x\",-1)", "e:#VALUE!"},
            {"MID(\"hello\",2,3)", "s:ell"},
            {"MID(\"hello\",0,1)", "e:#VALUE!"},
            {"MID(\"hello\",10,2)", "s:"},
            {"MID(\"hello\",4,100)", "s:lo"},
            {"MID(\"hello\",2,0)", "s:"},
            {"MID(\"hello\",2,-1)", "e:#VALUE!"},
            {"MID(12345,2,2)", "s:23"},
            {"MID(A2,1,1)", "s:h"},
            {"REPLACE(\"abcdef\",2,3,\"X\")", "s:aXef"},
            {"REPLACE(\"abc\",5,1,\"X\")", "s:abcX"},
            {"REPLACE(\"abc\",1,0,\"X\")", "s:Xabc"},
            {"TRIM(\"  a   b  \")", "s:a b"},
            {"TRIM(\"a\")", "s:a"},
            {"TRIM(123)", "s:123"},
            {"CLEAN(\"a\")", "s:a"},
            {"REPT(\"ab\",3)", "s:ababab"},
            {"REPT(\"ab\",0)", "s:"},
            {"REPT(\"ab\",-1)", "e:#VALUE!"},
            {"REPT(5,2)", "s:55"},
            {"SUBSTITUTE(\"aaa\",\"a\",\"b\",2)", "s:aba"},
            {"SUBSTITUTE(\"hello\",\"l\",\"L\")", "s:heLLo"},
            {"SUBSTITUTE(\"hello\",\"x\",\"y\")", "s:hello"},
            {"SUBSTITUTE(\"hello\",\"\",\"y\")", "s:hello"},
            {"SUBSTITUTE(\"Hello\",\"h\",\"j\")", "s:Hello"},   // case matters
            {"SUBSTITUTE(\"a-b-c\",\"-\",\"\")", "s:abc"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void findingText()
    {
        const E cases[] = {
            {"FIND(\"l\",\"hello\")", "n:3"},
            {"FIND(\"L\",\"hello\")", "e:#VALUE!"},
            {"FIND(\"l\",\"hello\",4)", "n:4"},
            {"FIND(\"l\",\"hello\",5)", "e:#VALUE!"},
            {"FIND(\"\",\"abc\")", "n:1"},
            {"FIND(\"x\",\"abc\")", "e:#VALUE!"},
            {"FIND(\"a\",\"abc\",0)", "e:#VALUE!"},
            {"FIND(\"a\",\"abc\",5)", "e:#VALUE!"},
            {"FIND(\"b\",\"abc\",2)", "n:2"},
            {"FIND(\"?\",\"a?b\")", "n:2"},        // no wildcards in FIND
            {"FIND(2,12345)", "n:2"},
            {"SEARCH(\"L\",\"hello\")", "n:3"},     // SEARCH ignores case
            {"SEARCH(\"l*o\",\"hello\")", "n:3"},
            {"SEARCH(\"?e\",\"hello\")", "n:1"},
            {"SEARCH(\"~?\",\"a?b\")", "n:2"},
            {"SEARCH(\"x\",\"abc\")", "e:#VALUE!"},
            {"SEARCH(\"b\",\"abcb\",3)", "n:4"},
            {"SEARCH(\"\",\"abc\")", "n:1"},
            {"ISNUMBER(FIND(\"l\",\"hello\"))", "b:1"},
            {"ISNUMBER(FIND(\"z\",\"hello\"))", "b:0"},
            {"EXACT(\"a\",\"A\")", "b:0"},
            {"EXACT(\"a\",\"a\")", "b:1"},
            {"EXACT(1,\"1\")", "b:1"},
            {"EXACT(\"\",\"\")", "b:1"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }

    void joiningAndChangingCase()
    {
        Grid g;
        g.s("A1", "x").n("A2", 2.5);
        const E cases[] = {
            {"CONCATENATE(\"a\",1,TRUE)", "s:a1TRUE"},
            {"CONCATENATE(\"a\")", "s:a"},
            // CONCAT, newer than the first release, is a function of its own only with its prefix: a file that writes CONCAT without one means some other function
            {"CONCAT(\"a\")", "!unsupported: CONCAT"},
            {"CONCATENATE(A1,A2,A9)", "s:x2.5"},
            {"CONCATENATE(\"a\",#N/A)", "e:#N/A"},
            {"CONCATENATE(,\"a\")", "s:a"},
            {"_xlfn.CONCAT(\"a\",\"b\",1)", "s:ab1"},
            {"_xlfn.CONCAT(A1:A2)", "s:x2.5"},
            {"UPPER(\"abC\")", "s:ABC"},
            {"LOWER(\"AbC\")", "s:abc"},
            {"PROPER(\"hello wORLD\")", "s:Hello World"},
            {"PROPER(\"o'neil-smith 2nd\")", "s:O'Neil-Smith 2Nd"},
            {"UPPER(1.5)", "s:1.5"},
            {"UPPER(TRUE)", "s:TRUE"},
            {"LOWER(A9)", "s:"},
            {"HYPERLINK(\"http://example.com\",\"click\")", "s:click"},
            {"HYPERLINK(\"http://example.com\")", "s:http://example.com"},
            {"T(\"x\")", "s:x"},
            {"T(1)", "s:"},
            {"T(A1)", "s:x"},
            {"T(TRUE)", "s:"},
            {"CHAR(65)", "s:A"},
            {"CHAR(97)", "s:a"},
            {"CHAR(0)", "e:#VALUE!"},
            {"CHAR(256)", "e:#VALUE!"},
            {"CODE(\"A\")", "n:65"},
            {"CODE(\"abc\")", "n:97"},
            {"CODE(\"\")", "e:#VALUE!"},
            {"_xlfn.TEXTJOIN(\", \",TRUE,\"a\",\"\",\"b\")", "s:a, b"},
            {"_xlfn.TEXTJOIN(\"-\",FALSE,\"a\",\"\",\"b\")", "s:a--b"},
        };
        CHECK_EVALS(cases, g, Settings());
    }

    void valuesAsTextAndTextAsValues()
    {
        const E cases[] = {
            {"VALUE(\"12\")", "n:12"},
            {"VALUE(\" 12.5 \")", "n:12.5"},
            {"VALUE(\"1,234\")", "n:1234"},
            {"VALUE(\"abc\")", "e:#VALUE!"},
            {"VALUE(\"\")", "e:#VALUE!"},
            {"VALUE(\"10%\")", "n:0.1"},
            {"VALUE(\"3/15/2024\")", "n:45366"},
            {"VALUE(\"12:00\")", "n:0.5"},
            {"VALUE(5)", "n:5"},
            {"VALUE(TRUE)", "e:#VALUE!"},
            {"VALUE(\"$1,000.50\")", "n:1000.5"},
            {"VALUE(\"1E3\")", "n:1000"},
            {"N(\"x\")", "n:0"},
            {"N(5)", "n:5"},
            {"N(TRUE)", "n:1"},
            {"TEXT(1234.5,\"#,##0.00\")", "s:1,234.50"},
            {"TEXT(0.5,\"0%\")", "s:50%"},
            {"TEXT(45366,\"yyyy-mm-dd\")", "s:2024-03-15"},
            {"TEXT(45366,\"mmmm d, yyyy\")", "s:March 15, 2024"},
            {"TEXT(45366.75,\"h:mm AM/PM\")", "s:6:00 PM"},
            {"TEXT(5,\"00\")", "s:05"},
            {"TEXT(-1.5,\"0.0\")", "s:-1.5"},
            {"TEXT(1234,\"General\")", "s:1234"},
            {"TEXT(\"abc\",\"@\")", "s:abc"},
            {"TEXT(\"12\",\"0.00\")", "s:12.00"},
            {"TEXT(1,\"[\")", "e:#VALUE!"},
            {"TEXT(TRUE,\"0\")", "s:TRUE"},
            {"TEXT(A9,\"0\")", "s:0"},
            {"TEXT(1.005,\"0.00\")", "s:1.01"},
            {"FIXED(1234.567,1)", "s:1,234.6"},
            {"FIXED(1234.567,1,TRUE)", "s:1234.6"},
            {"FIXED(1234.567)", "s:1,234.57"},
            {"FIXED(-1234.567,0)", "s:-1,235"},
            {"FIXED(1234.567,-2)", "s:1,200"},
            {"DOLLAR(1234.5)", "s:$1,234.50"},
            {"DOLLAR(-1234.5,0)", "s:($1,235)"},
            {"DOLLAR(0.5,1)", "s:$0.5"},
        };
        CHECK_EVALS(cases, Grid(), Settings());
    }
};

JP_TEST_CLASS(SheetFnTextTests)
#include "tst_sheet_fn_text.moc"
