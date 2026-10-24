// Tests of what a sheet's rules of conditional formatting do to the cells that are shown (libs/shell/grid/condformat.h): each kind of rule (cell value, formula, top and bottom, above and
// below average, duplicates and unique values, text, dates, blanks, errors, color scales, data bars, icon sets), the order of their priority and stop-if-true, that what a rule needs of
// all its cells is worked out once and again only when a value in its cells changes, the formula of a rule that reads another sheet, and the look and the text a cell gets. Every sheet is
// built here, and the day the rules of dates read is given.
#include "core/fonts.h"
#include "sheethelpers.h"

#include "grid/gridsheet.h"

#include <QDate>

#include <memory>

using namespace jp::grid;

namespace {

constexpr QRgb kRed = 0xFFFF0000;
constexpr QRgb kBlue = 0xFF0000FF;

ColorSpec rgb(quint32 v) { return ColorSpec::fromRgb(v); }

CellRange range(const QString &ref)
{
    CellRange r;
    parseRangeRef(ref, &r);
    return r;
}

// A sheet in the making: its cells, its styles (a font, a fill, a border, a format, and the differential formats the rules use), its rules, and the grid that shows it.
struct Book {
    Sheet sheet;
    Styles styles;
    SharedStrings strings;
    GridSheet grid;
    Sheet second;   // a second sheet, for a rule that reads it

    Book()
    {
        FontSpec f;
        f.name = QStringLiteral("Calibri");
        f.size = 11;
        styles.fonts = {f};
        styles.fills = {FillSpec()};
        styles.borders = {BorderSpec()};
        styles.formats = {CellFormat()};
        styles.cellXfs = {0};
    }
    static Cell numberCell(quint32 row, quint32 col, double v)
    {
        Cell c;
        c.row = row;
        c.col = quint16(col);
        c.type = CellType::Number;
        c.number = v;
        return c;
    }
    void put(Sheet *s, const QString &ref, double v)
    {
        quint32 row = 0, col = 0;
        parseCellRef(ref, &row, &col);
        s->cells.add(numberCell(row, col, v));
    }
    void num(const QString &ref, double v) { put(&sheet, ref, v); }
    void text(const QString &ref, const QString &t)
    {
        quint32 row = 0, col = 0;
        parseCellRef(ref, &row, &col);
        Cell c;
        c.row = row;
        c.col = quint16(col);
        c.type = CellType::InlineString;
        c.number = sheet.texts.size();
        sheet.texts.append(t);
        sheet.cells.add(c);
    }
    void error(const QString &ref)
    {
        quint32 row = 0, col = 0;
        parseCellRef(ref, &row, &col);
        Cell c;
        c.row = row;
        c.col = quint16(col);
        c.type = CellType::Error;
        c.number = double(int(jp::calc::ErrorKind::Div0));
        sheet.cells.add(c);
    }
    // A differential format with a fill (in the background color, where a differential format keeps its color), and the font color if given.
    int dxfFill(quint32 fill, quint32 fontColor = 0xFFFFFFFF)
    {
        DifferentialFormat d;
        d.hasFill = true;
        d.fill.pattern = PatternKind::Solid;
        d.fill.bg = rgb(fill);
        if (fontColor != 0xFFFFFFFF) d.fontColor = rgb(fontColor);
        styles.dxfs.append(d);
        return int(styles.dxfs.size() - 1);
    }
    int dxfBoldItalic(bool bold, bool italic)
    {
        DifferentialFormat d;
        if (bold) d.bold = true;
        if (italic) d.italic = true;
        styles.dxfs.append(d);
        return int(styles.dxfs.size() - 1);
    }
    int dxfNumber(const QString &code)
    {
        DifferentialFormat d;
        d.numberFormat = code;
        styles.dxfs.append(d);
        return int(styles.dxfs.size() - 1);
    }
    CondRule &rule(CondRule::Type type, const QString &where, int dxf = -1)
    {
        CondRule r;
        r.type = type;
        r.ranges.append(range(where));
        r.dxf = dxf;
        r.priority = int(sheet.conditionalFormats.size()) + 1;
        sheet.conditionalFormats.append(r);
        return sheet.conditionalFormats.last();
    }
    CondRule &cellIs(const QString &where, CondRule::Operator op, const QString &f1, int dxf, const QString &f2 = QString())
    {
        CondRule &r = rule(CondRule::Type::CellIs, where, dxf);
        r.op = op;
        r.formula1 = f1;
        r.formula2 = f2;
        return r;
    }
    void build()
    {
        sheet.cells.finish();
        second.cells.finish();
        GridData d;
        d.styles = &styles;
        d.strings = &strings;
        d.sheet = &sheet;
        d.sheets = {&sheet, &second};
        d.sheetNames = {QStringLiteral("Sheet1"), QStringLiteral("Data")};
        grid.setData(d);
        grid.setToday(todayNumber());
    }
    static double todayNumber() { return double(QDate(1899, 12, 30).daysTo(QDate(2026, 10, 24))); }
    static double serial(int y, int m, int d) { return double(QDate(1899, 12, 30).daysTo(QDate(y, m, d))); }
    const CondCell &at(const QString &ref) const
    {
        quint32 row = 0, col = 0;
        parseCellRef(ref, &row, &col);
        return grid.conditional().at(row, col);
    }
    const DifferentialFormat &formatAt(const QString &ref) const { return grid.conditional().format(at(ref).format); }
};

} // namespace

class GridCondFormatTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // A cell-value rule colors the cells that pass its test, and the first rule's format wins where two say the same thing (the later one adds what is left): the cell over 3 is bold and
    // yellow, the one between 2 and 4 is italic and (where only that rule holds it) green. With stop-if-true the later rule says nothing to the cells the first holds.
    void priorityAndStopIfTrue()
    {
        Book b;
        for (int i = 1; i <= 5; ++i) b.num(QStringLiteral("A%1").arg(i), i);
        const int yellowBold = b.dxfFill(0xFFFF00, 0xC00000), greenItalic = b.dxfFill(0x00FF00);
        b.styles.dxfs[yellowBold].bold = true;
        b.styles.dxfs[greenItalic].italic = true;
        b.cellIs(QStringLiteral("A1:A5"), CondRule::Operator::GreaterThan, QStringLiteral("3"), yellowBold);
        b.cellIs(QStringLiteral("A1:A5"), CondRule::Operator::Between, QStringLiteral("2"), greenItalic, QStringLiteral("4"));
        b.build();
        QVERIFY(!b.at(QStringLiteral("A1")).any);
        QCOMPARE(b.at(QStringLiteral("A1")).format, -1);
        // 3 is in the second rule only: green and italic
        QVERIFY(b.at(QStringLiteral("A3")).any);
        QVERIFY(b.formatAt(QStringLiteral("A3")).fill.bg == rgb(0x00FF00));
        QVERIFY(b.formatAt(QStringLiteral("A3")).italic && *b.formatAt(QStringLiteral("A3")).italic);
        QVERIFY(!b.formatAt(QStringLiteral("A3")).bold);
        // 4 is in both: the first rule's fill, font color, and bold, and the second's italic
        const DifferentialFormat &four = b.formatAt(QStringLiteral("A4"));
        QVERIFY(four.fill.bg == rgb(0xFFFF00) && four.fontColor == rgb(0xC00000));
        QVERIFY(four.bold && *four.bold && four.italic && *four.italic);
        // 5 is in the first only
        QVERIFY(b.formatAt(QStringLiteral("A5")).fill.bg == rgb(0xFFFF00));
        QVERIFY(!b.formatAt(QStringLiteral("A5")).italic);
        // stop if true on the first
        b.sheet.conditionalFormats[0].stopIfTrue = true;
        b.grid.resetConditional();
        QVERIFY(!b.formatAt(QStringLiteral("A4")).italic);
        QVERIFY(b.formatAt(QStringLiteral("A4")).fill.bg == rgb(0xFFFF00));
        QVERIFY(b.formatAt(QStringLiteral("A3")).italic);   // (the first rule does not hold 3, so the second still speaks)
        QVERIFY(b.grid.conditional().covers(0, 0) && !b.grid.conditional().covers(0, 1) && !b.grid.conditional().covers(5, 0));
    }

    // The operators of a cell-value rule, with the value text and the cell's own kind: a number does not pass a test against a text, and equal to and not equal to are exact.
    void cellValueOperators()
    {
        Book b;
        for (int i = 1; i <= 5; ++i) b.num(QStringLiteral("A%1").arg(i), i * 10);
        const int d = b.dxfBoldItalic(true, false);
        b.cellIs(QStringLiteral("A1:A5"), CondRule::Operator::Equal, QStringLiteral("30"), d);
        b.cellIs(QStringLiteral("B1:B1"), CondRule::Operator::Equal, QStringLiteral("30"), d);
        b.cellIs(QStringLiteral("C1:C5"), CondRule::Operator::NotBetween, QStringLiteral("20"), d, QStringLiteral("40"));
        b.num(QStringLiteral("C1"), 10);
        b.num(QStringLiteral("C2"), 20);
        b.num(QStringLiteral("C3"), 30);
        b.num(QStringLiteral("C4"), 40);
        b.num(QStringLiteral("C5"), 50);
        b.build();
        QVERIFY(b.at(QStringLiteral("A3")).any && !b.at(QStringLiteral("A2")).any && !b.at(QStringLiteral("A4")).any);
        QVERIFY(!b.at(QStringLiteral("B1")).any);   // (an empty cell is not 30)
        QVERIFY(b.at(QStringLiteral("C1")).any && !b.at(QStringLiteral("C2")).any && !b.at(QStringLiteral("C3")).any && !b.at(QStringLiteral("C4")).any && b.at(QStringLiteral("C5")).any);
    }

    // A formula rule is written for the first cell of its range and read in each cell as a formula copied there: relative references move with the cell, absolute ones stay.
    void formulaRulesMoveWithTheirCell()
    {
        Book b;
        for (int i = 1; i <= 4; ++i) {
            b.num(QStringLiteral("A%1").arg(i), i);
            b.num(QStringLiteral("B%1").arg(i), 5 - i);
        }
        const int d = b.dxfFill(0xFF0000);
        CondRule &r = b.rule(CondRule::Type::Expression, QStringLiteral("B1:B4"), d);
        r.formula1 = QStringLiteral("$A1>B1");   // (A1 against B1 in the first row; A3 against B3 in the third)
        CondRule &fixed = b.rule(CondRule::Type::Expression, QStringLiteral("C1:C4"), d);
        fixed.formula1 = QStringLiteral("$A$1=1");
        b.build();
        // A: 1 2 3 4; B: 4 3 2 1 -> A>B in rows 3 and 4
        QVERIFY(!b.at(QStringLiteral("B1")).any && !b.at(QStringLiteral("B2")).any && b.at(QStringLiteral("B3")).any && b.at(QStringLiteral("B4")).any);
        for (const char *ref : {"C1", "C2", "C3", "C4"}) QVERIFY2(b.at(QString::fromLatin1(ref)).any, ref);
    }

    // Top and bottom: the N highest or lowest, or a percent of the cells (at least one), counting ties as the cell they tie with.
    void topAndBottom()
    {
        Book b;
        const double values[] = {5, 1, 9, 7, 3, 8};
        for (int i = 0; i < 6; ++i) b.num(QStringLiteral("A%1").arg(i + 1), values[i]);
        const int d = b.dxfBoldItalic(true, false);
        CondRule &top = b.rule(CondRule::Type::Top10, QStringLiteral("A1:A6"), d);
        top.rank = 2;
        b.build();
        auto marked = [&] {
            QString s;
            for (int i = 1; i <= 6; ++i) s += b.at(QStringLiteral("A%1").arg(i)).any ? QLatin1Char('x') : QLatin1Char('.');
            return s;
        };
        QCOMPARE(marked(), QStringLiteral("..x..x"));   // 9 and 8
        b.sheet.conditionalFormats[0].bottom = true;
        b.grid.resetConditional();
        QCOMPARE(marked(), QStringLiteral(".x..x."));   // 1 and 3
        b.sheet.conditionalFormats[0].percent = true;
        b.sheet.conditionalFormats[0].rank = 10;   // 10 percent of 6 cells is less than one: one
        b.grid.resetConditional();
        QCOMPARE(marked(), QStringLiteral(".x...."));
        b.sheet.conditionalFormats[0].bottom = false;
        b.sheet.conditionalFormats[0].rank = 50;   // the top three
        b.grid.resetConditional();
        QCOMPARE(marked(), QStringLiteral("..xx.x"));
    }

    // Above and below the average (the average is of the numbers in the rule's cells; equal to it counts only when the rule says), and by standard deviations.
    void aboveAndBelowAverage()
    {
        Book b;
        const double values[] = {5, 1, 9, 7, 3, 8};   // the average is 5.5; the sample deviation is 3.08
        for (int i = 0; i < 6; ++i) b.num(QStringLiteral("A%1").arg(i + 1), values[i]);
        b.text(QStringLiteral("A7"), QStringLiteral("not a number"));
        const int d = b.dxfBoldItalic(true, false);
        CondRule &r = b.rule(CondRule::Type::AboveAverage, QStringLiteral("A1:A7"), d);
        b.build();
        auto marked = [&] {
            QString s;
            for (int i = 1; i <= 7; ++i) s += b.at(QStringLiteral("A%1").arg(i)).any ? QLatin1Char('x') : QLatin1Char('.');
            return s;
        };
        QCOMPARE(marked(), QStringLiteral("..xx.x."));
        r.above = false;
        b.grid.resetConditional();
        QCOMPARE(marked(), QStringLiteral("xx..x.."));
        b.sheet.conditionalFormats[0].above = true;
        b.sheet.conditionalFormats[0].stdDev = 1;   // over 5.5 + 3.08
        b.grid.resetConditional();
        QCOMPARE(marked(), QStringLiteral("..x...."));
        // equal to the average counts when the rule says so
        Book c;
        c.num(QStringLiteral("A1"), 1);
        c.num(QStringLiteral("A2"), 2);
        c.num(QStringLiteral("A3"), 3);
        CondRule &e = c.rule(CondRule::Type::AboveAverage, QStringLiteral("A1:A3"), c.dxfBoldItalic(true, false));
        c.build();
        QVERIFY(!c.at(QStringLiteral("A2")).any && c.at(QStringLiteral("A3")).any);
        e.equalAverage = true;
        c.grid.resetConditional();
        QVERIFY(c.at(QStringLiteral("A2")).any && c.at(QStringLiteral("A3")).any && !c.at(QStringLiteral("A1")).any);
    }

    // Duplicates and unique values: numbers by value, texts without regard to case; empty cells are neither.
    void duplicatesAndUnique()
    {
        Book b;
        b.num(QStringLiteral("A1"), 1);
        b.num(QStringLiteral("A2"), 2);
        b.num(QStringLiteral("A3"), 2);
        b.num(QStringLiteral("A4"), 3);
        b.text(QStringLiteral("A5"), QStringLiteral("a"));
        b.text(QStringLiteral("A6"), QStringLiteral("A"));
        b.text(QStringLiteral("A8"), QStringLiteral("x"));
        const int d = b.dxfBoldItalic(true, false);
        b.rule(CondRule::Type::DuplicateValues, QStringLiteral("A1:A8"), d);
        b.build();
        auto marked = [&] {
            QString s;
            for (int i = 1; i <= 8; ++i) s += b.at(QStringLiteral("A%1").arg(i)).any ? QLatin1Char('x') : QLatin1Char('.');
            return s;
        };
        QCOMPARE(marked(), QStringLiteral(".xx.xx.."));
        b.sheet.conditionalFormats[0].type = CondRule::Type::UniqueValues;
        b.grid.resetConditional();
        QCOMPARE(marked(), QStringLiteral("x..x...x"));
    }

    // Rules about text, blanks, and errors.
    void textBlanksAndErrors()
    {
        Book b;
        b.text(QStringLiteral("A1"), QStringLiteral("Banana"));
        b.text(QStringLiteral("A2"), QStringLiteral("band"));
        b.text(QStringLiteral("A3"), QStringLiteral("cab"));
        b.num(QStringLiteral("A4"), 5);
        b.error(QStringLiteral("A5"));
        // A6 is empty
        const int d = b.dxfBoldItalic(true, false);
        auto marks = [&] {
            QString s;
            for (int i = 1; i <= 6; ++i) s += b.at(QStringLiteral("A%1").arg(i)).any ? QLatin1Char('x') : QLatin1Char('.');
            return s;
        };
        CondRule &r = b.rule(CondRule::Type::ContainsText, QStringLiteral("A1:A6"), d);
        r.text = QStringLiteral("AN");
        b.build();
        QCOMPARE(marks(), QStringLiteral("xx...."));   // (without regard to case)
        auto set = [&](CondRule::Type t, const QString &text = QString()) {
            b.sheet.conditionalFormats[0].type = t;
            b.sheet.conditionalFormats[0].text = text;
            b.grid.resetConditional();
        };
        set(CondRule::Type::NotContainsText, QStringLiteral("an"));
        QCOMPARE(marks(), QStringLiteral("..xxxx"));
        set(CondRule::Type::BeginsWith, QStringLiteral("ba"));
        QCOMPARE(marks(), QStringLiteral("xx...."));
        set(CondRule::Type::EndsWith, QStringLiteral("AB"));
        QCOMPARE(marks(), QStringLiteral("..x..."));
        set(CondRule::Type::ContainsBlanks);
        QCOMPARE(marks(), QStringLiteral(".....x"));
        set(CondRule::Type::NotContainsBlanks);
        QCOMPARE(marks(), QStringLiteral("xxxx.."));
        set(CondRule::Type::ContainsErrors);
        QCOMPARE(marks(), QStringLiteral("....x."));
        set(CondRule::Type::NotContainsErrors);
        QCOMPARE(marks(), QStringLiteral("xxxx.x"));
    }

    // Dates occurring: the day the rules read is given (today is 2026-10-24, a Saturday), so these hold on any day the tests are run.
    void datesOccurring()
    {
        Book b;
        const struct {
            const char *ref;
            int y, m, d;
        } dates[] = {{"A1", 2026, 10, 24}, {"A2", 2026, 10, 23}, {"A3", 2026, 10, 25}, {"A4", 2026, 10, 18}, {"A5", 2026, 10, 17}, {"A6", 2026, 10, 1}, {"A7", 2026, 9, 30}, {"A8", 2026, 11, 1}};
        for (const auto &x : dates) b.num(QString::fromLatin1(x.ref), Book::serial(x.y, x.m, x.d));
        const int d = b.dxfBoldItalic(true, false);
        CondRule &r = b.rule(CondRule::Type::TimePeriod, QStringLiteral("A1:A8"), d);
        r.timePeriod = QStringLiteral("today");
        b.build();
        auto marks = [&] {
            QString s;
            for (int i = 1; i <= 8; ++i) s += b.at(QStringLiteral("A%1").arg(i)).any ? QLatin1Char('x') : QLatin1Char('.');
            return s;
        };
        auto period = [&](const char *name) {
            b.sheet.conditionalFormats[0].timePeriod = QString::fromLatin1(name);
            b.grid.resetConditional();
            return marks();
        };
        QCOMPARE(marks(), QStringLiteral("x......."));
        QCOMPARE(period("yesterday"), QStringLiteral(".x......"));
        QCOMPARE(period("tomorrow"), QStringLiteral("..x....."));
        QCOMPARE(period("last7Days"), QStringLiteral("xx.x...."));   // (the 18th is six days back; the 17th is seven)
        QCOMPARE(period("thisMonth"), QStringLiteral("xxxxxx.."));
        QCOMPARE(period("lastMonth"), QStringLiteral("......x."));
        QCOMPARE(period("nextMonth"), QStringLiteral(".......x"));
        // the week begins on Sunday: Sunday the 18th through Saturday the 24th is this week
        QCOMPARE(period("thisWeek"), QStringLiteral("xx.x...."));
        QCOMPARE(period("lastWeek"), QStringLiteral("....x..."));
        QCOMPARE(period("nextWeek"), QStringLiteral("..x....."));   // (Sunday the 25th to Saturday the 31st)
    }

    // A color scale: the lowest value has the first color, the highest the last, and those between a mix; a cell that is not a number has none.
    void colorScales()
    {
        Book b;
        b.num(QStringLiteral("A1"), 0);
        b.num(QStringLiteral("A2"), 50);
        b.num(QStringLiteral("A3"), 100);
        b.num(QStringLiteral("A4"), 25);
        b.text(QStringLiteral("A5"), QStringLiteral("text"));
        CondRule &r = b.rule(CondRule::Type::ColorScale, QStringLiteral("A1:A5"));
        r.values = {{CondValue::Type::Min, QString(), true}, {CondValue::Type::Max, QString(), true}};
        r.colors = {rgb(0xFF0000), rgb(0x0000FF)};
        b.build();
        QVERIFY(b.at(QStringLiteral("A1")).hasScale && b.at(QStringLiteral("A1")).scale == kRed);
        QVERIFY(b.at(QStringLiteral("A3")).scale == kBlue);
        QCOMPARE(b.at(QStringLiteral("A2")).scale, QRgb(0xFF800080));   // (halfway: 127.5 rounds up)
        QCOMPARE(b.at(QStringLiteral("A4")).scale, QRgb(0xFFBF0040));
        QVERIFY(!b.at(QStringLiteral("A5")).hasScale && !b.at(QStringLiteral("A5")).any);
        // three colors, the middle at the 50th percentile: the values 0 50 100 25 have the median at 37.5
        r.values = {{CondValue::Type::Min, QString(), true}, {CondValue::Type::Percentile, QStringLiteral("50"), true}, {CondValue::Type::Max, QString(), true}};
        r.colors = {rgb(0xFF0000), rgb(0xFFFF00), rgb(0x00FF00)};
        b.grid.resetConditional();
        QCOMPARE(b.at(QStringLiteral("A1")).scale, kRed);
        QCOMPARE(b.at(QStringLiteral("A3")).scale, QRgb(0xFF00FF00));
        // 50 is past the middle (37.5) by a fifth of the way to 100 (62.5): between yellow and green
        const QRgb at50 = b.at(QStringLiteral("A2")).scale;
        QVERIFY(qRed(at50) < 255 && qRed(at50) > 0 && qGreen(at50) == 255);
        // a number as a stop, and a value below the first stop has the first color
        r.values = {{CondValue::Type::Number, QStringLiteral("40"), true}, {CondValue::Type::Number, QStringLiteral("60"), true}};
        r.colors = {rgb(0xFF0000), rgb(0x0000FF)};
        b.grid.resetConditional();
        QCOMPARE(b.at(QStringLiteral("A1")).scale, kRed);
        QCOMPARE(b.at(QStringLiteral("A3")).scale, kBlue);
        QCOMPARE(b.at(QStringLiteral("A2")).scale, QRgb(0xFF800080));
    }

    // Data bars: from the shortest length (10 percent of the cell) to the longest (90) across the values; with negative values the axis is where zero is and negative bars reach to
    // the left in their own color; and a bar can leave the value out.
    void dataBars()
    {
        Book b;
        b.num(QStringLiteral("A1"), 0);
        b.num(QStringLiteral("A2"), 50);
        b.num(QStringLiteral("A3"), 100);
        CondRule &r = b.rule(CondRule::Type::DataBar, QStringLiteral("A1:A3"));
        r.values = {{CondValue::Type::Min, QString(), true}, {CondValue::Type::Max, QString(), true}};
        r.colors = {rgb(0x638EC6)};
        b.build();
        QVERIFY(b.at(QStringLiteral("A1")).hasBar && !b.at(QStringLiteral("A1")).hideValue);
        QCOMPARE(b.at(QStringLiteral("A1")).bar.from, 0.0);
        QVERIFY(qAbs(b.at(QStringLiteral("A1")).bar.to - 0.1) < 1e-9);
        QVERIFY(qAbs(b.at(QStringLiteral("A2")).bar.to - 0.5) < 1e-9);
        QVERIFY(qAbs(b.at(QStringLiteral("A3")).bar.to - 0.9) < 1e-9);
        QCOMPARE(b.at(QStringLiteral("A3")).bar.fill, QRgb(0xFF638EC6));
        QVERIFY(!b.at(QStringLiteral("A3")).bar.hasAxis);
        r.showValue = false;
        b.grid.resetConditional();
        QVERIFY(b.at(QStringLiteral("A2")).hideValue);
        // negative values
        Book n;
        n.num(QStringLiteral("A1"), -50);
        n.num(QStringLiteral("A2"), 0);
        n.num(QStringLiteral("A3"), 25);
        n.num(QStringLiteral("A4"), 50);
        CondRule &nr = n.rule(CondRule::Type::DataBar, QStringLiteral("A1:A4"));
        nr.values = {{CondValue::Type::Min, QString(), true}, {CondValue::Type::Max, QString(), true}};
        nr.colors = {rgb(0x638EC6)};
        nr.negativeColor = rgb(0xFF0000);
        nr.axisColor = rgb(0x000000);
        nr.border = true;
        nr.gradient = false;
        n.build();
        const CondBar &low = n.at(QStringLiteral("A1")).bar, &quarter = n.at(QStringLiteral("A3")).bar, &high = n.at(QStringLiteral("A4")).bar;
        QVERIFY(low.hasAxis && qAbs(low.axisAt - 0.5) < 1e-9);
        QVERIFY(low.negative && qAbs(low.to - 0.5) < 1e-9 && qAbs(low.from) < 1e-9);
        QCOMPARE(low.fill, kRed);
        QVERIFY(!quarter.negative && qAbs(quarter.from - 0.5) < 1e-9 && qAbs(quarter.to - 0.75) < 1e-9);
        QVERIFY(qAbs(high.to - 1.0) < 1e-9 && !high.gradient && high.hasBorder);
        QCOMPARE(high.axis, QRgb(0xFF000000));
        // a bar for the right-to-left direction is mirrored
        nr.rightToLeft = true;
        n.grid.resetConditional();
        QVERIFY(qAbs(n.at(QStringLiteral("A4")).bar.from - 0.0) < 1e-9 && qAbs(n.at(QStringLiteral("A4")).bar.to - 0.5) < 1e-9);
    }

    // Icon sets: the icon of a value is the highest whose value it reaches (or passes, when the rule says "greater than"), reversed when the rule says so; the value can be left out.
    void iconSets()
    {
        QCOMPARE(iconSetCount(QStringLiteral("3Arrows")), 3);
        QCOMPARE(iconSetCount(QStringLiteral("4Rating")), 4);
        QCOMPARE(iconSetCount(QStringLiteral("5Quarters")), 5);
        QCOMPARE(iconSetCount(QStringLiteral("nonsense")), 0);
        Book c;
        const double values[] = {0, 33, 40, 67, 100};
        for (int i = 0; i < 5; ++i) c.num(QStringLiteral("A%1").arg(i + 1), values[i]);
        CondRule &r = c.rule(CondRule::Type::IconSet, QStringLiteral("A1:A5"));
        r.iconSet = QStringLiteral("3TrafficLights1");
        r.values = {{CondValue::Type::Percent, QStringLiteral("0"), true}, {CondValue::Type::Percent, QStringLiteral("33"), true}, {CondValue::Type::Percent, QStringLiteral("67"), true}};
        c.build();
        auto icons = [&] {
            QString s;
            for (int i = 1; i <= 5; ++i) s += QString::number(c.at(QStringLiteral("A%1").arg(i)).icon);
            return s;
        };
        QCOMPARE(icons(), QStringLiteral("01122"));   // 33 reaches the 33 percent point; 67 reaches 67
        QCOMPARE(c.at(QStringLiteral("A1")).iconSet, 0);
        QCOMPARE(c.grid.conditional().iconSetName(c.at(QStringLiteral("A1")).iconSet), QStringLiteral("3TrafficLights1"));
        r.values[1].gte = false;   // over 33, not at it
        c.grid.resetConditional();
        QCOMPARE(icons(), QStringLiteral("00122"));
        r.reverse = true;
        c.grid.resetConditional();
        QCOMPARE(icons(), QStringLiteral("22100"));
        r.showValue = false;
        c.grid.resetConditional();
        QVERIFY(c.at(QStringLiteral("A3")).hideValue);
    }

    // What a rule needs of all its cells is worked out once and kept: asking for every cell of a rule over a thousand cells makes one pass over them, and a value changing in them makes
    // the next ask pass again, one that changes outside them does not.
    void figuresAreWorkedOutOnceAndAgainWhenAValueChanges()
    {
        Book b;
        for (int i = 1; i <= 1000; ++i) b.num(QStringLiteral("A%1").arg(i), i);
        b.num(QStringLiteral("C1"), 7);
        const int d = b.dxfBoldItalic(true, false);
        CondRule &top = b.rule(CondRule::Type::Top10, QStringLiteral("A1:A1000"), d);
        top.rank = 1;
        b.build();
        for (int i = 1; i <= 1000; ++i) b.at(QStringLiteral("A%1").arg(i));
        QCOMPARE(b.grid.conditional().passes(), qint64(1));
        QVERIFY(b.at(QStringLiteral("A1000")).any && !b.at(QStringLiteral("A999")).any);
        // a value outside the rule's cells changes: the results are asked again, but the figures stay
        b.grid.invalidateText(0, 2);
        b.at(QStringLiteral("A1000"));
        QCOMPARE(b.grid.conditional().passes(), qint64(1));
        // a value inside changes (the highest is no longer): the figures are made again
        quint32 row = 0, col = 0;
        parseCellRef(QStringLiteral("A1000"), &row, &col);
        const_cast<Cell *>(b.sheet.cells.find(row, col))->number = 5;
        b.grid.invalidateText(row, col);
        QVERIFY(!b.at(QStringLiteral("A1000")).any);
        QVERIFY(b.at(QStringLiteral("A999")).any);
        QCOMPARE(b.grid.conditional().passes(), qint64(2));
    }

    // A rule whose formula reads another sheet sees that sheet's cells.
    void aFormulaCanReadAnotherSheet()
    {
        Book b;
        b.num(QStringLiteral("A1"), 1);
        b.num(QStringLiteral("A2"), 2);
        b.put(&b.second, QStringLiteral("A1"), 10);
        b.put(&b.second, QStringLiteral("A2"), 0);
        CondRule &r = b.rule(CondRule::Type::Expression, QStringLiteral("A1:A2"), b.dxfBoldItalic(true, false));
        r.formula1 = QStringLiteral("Data!A1>5");
        b.build();
        QVERIFY(b.at(QStringLiteral("A1")).any);
        r.formula1 = QStringLiteral("Data!A1>A1+8");   // (relative: Data!A2 > A2 + 8 in the second row, which does not hold)
        b.grid.resetConditional();
        QVERIFY(b.at(QStringLiteral("A1")).any && !b.at(QStringLiteral("A2")).any);
    }

    // The look a cell gets: the cell's own with the differential format's changes over it, kept (the same object) for the same format; and the cell's text in the number format a rule gives.
    void theLookAndTheTextOfACell()
    {
        Book b;
        b.num(QStringLiteral("A1"), 0.5);
        b.num(QStringLiteral("A2"), 0.25);
        const int fill = b.dxfFill(0xFFEB9C, 0x9C5700);
        const int percent = b.dxfNumber(QStringLiteral("0%"));
        b.cellIs(QStringLiteral("A1:A2"), CondRule::Operator::GreaterThan, QStringLiteral("0.3"), fill);
        b.cellIs(QStringLiteral("A1:A2"), CondRule::Operator::GreaterThan, QStringLiteral("0.3"), percent);
        b.styles.dxfs[fill].bold = true;
        b.build();
        quint32 row1 = 0, row2 = 1;
        const CellLook &plain = b.grid.lookOf(row2, 0, 0);
        QVERIFY(plain.pattern == PatternKind::None && !plain.bold);
        QCOMPARE(&plain, &b.grid.looks().look(0));
        const CellLook &ruled = b.grid.lookOf(row1, 0, 0);
        QVERIFY(ruled.pattern == PatternKind::Solid && ruled.fg == 0xFFFFEB9C && ruled.bold);
        QCOMPARE(ruled.textColor, QRgb(0xFF9C5700));
        QCOMPARE(&b.grid.lookOf(row1, 0, 0), &ruled);
        // the number format of the second rule
        QCOMPARE(b.grid.textOf(*b.sheet.cells.find(0, 0)), QStringLiteral("50%"));
        QCOMPARE(b.grid.textOf(*b.sheet.cells.find(1, 0)), QStringLiteral("0.25"));
        // new rules (a rule edited) let the looks go
        b.sheet.conditionalFormats.clear();
        b.grid.resetConditional();
        b.grid.invalidateTexts();
        QVERIFY(!b.grid.hasConditionalFormats());
        QCOMPARE(&b.grid.lookOf(row1, 0, 0), &b.grid.looks().look(0));
        QCOMPARE(b.grid.textOf(*b.sheet.cells.find(0, 0)), QStringLiteral("0.5"));
    }

    // The cost: a sheet of a hundred thousand rows with a rule over all of them asks about the cells it is asked about only, and makes the figures once.
    void aBigSheetIsWorkedOutForWhatIsAskedAbout()
    {
        Book b;
        for (int i = 0; i < 100000; ++i) b.sheet.cells.add(Book::numberCell(quint32(i), 0, double((i * 7919) % 100003)));
        CondRule &r = b.rule(CondRule::Type::DataBar, QStringLiteral("A1:A100000"));
        r.values = {{CondValue::Type::Min, QString(), true}, {CondValue::Type::Max, QString(), true}};
        r.colors = {rgb(0x638EC6)};
        b.rule(CondRule::Type::ColorScale, QStringLiteral("A1:A100000"));
        b.sheet.conditionalFormats.last().values = {{CondValue::Type::Min, QString(), true}, {CondValue::Type::Max, QString(), true}};
        b.sheet.conditionalFormats.last().colors = {rgb(0xFF0000), rgb(0x00FF00)};
        b.build();
        for (quint32 row = 50000; row < 50040; ++row) b.grid.conditional().at(row, 0);
        QCOMPARE(b.grid.conditional().passes(), qint64(2));   // (one pass for each rule)
        QCOMPARE(b.grid.conditional().tests(), qint64(0));
    }
};

JP_TEST_CLASS(GridCondFormatTests)
#include "tst_grid_condformat.moc"
