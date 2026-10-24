// Tests of what a sheet's rules of conditional formatting do to the cells that are shown (libs/shell/grid/condformat.h): each kind of rule (cell value, formula, top and bottom, above and
// below average, duplicates and unique values, text, dates, blanks, errors, color scales, data bars, icon sets), the order of their priority and stop-if-true, that what a rule needs of
// all its cells is worked out once and again only when a value in its cells changes, the formula of a rule that reads another sheet, and the look and the text a cell gets. Every sheet is
// built here, and the day the rules of dates read is given.
#include "../localepin.h"
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
    CondCell at(const QString &ref) const
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

    // What a rule needs of all its cells is worked out once and kept: asking for every cell of a rule over a thousand cells makes one pass over them, and a value changing in them changes
    // the figures by that one cell (no pass again), one that changes outside them changes nothing.
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
        QCOMPARE(b.grid.conditional().incrementalChanges(), qint64(0));
        // a value inside changes (the highest is no longer): the figures follow that cell, and are not made again
        quint32 row = 0, col = 0;
        parseCellRef(QStringLiteral("A1000"), &row, &col);
        const_cast<Cell *>(b.sheet.cells.find(row, col))->number = 5;
        b.grid.invalidateText(row, col);
        QVERIFY(!b.at(QStringLiteral("A1000")).any);
        QVERIFY(b.at(QStringLiteral("A999")).any);
        QCOMPARE(b.grid.conditional().passes(), qint64(1));
        QCOMPARE(b.grid.conditional().incrementalChanges(), qint64(1));
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

    // The cost: a sheet of a hundred thousand rows with two rules over all of them asks about the cells it is asked about only, and makes the figures once for both.
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
        QCOMPARE(b.grid.conditional().passes(), qint64(1));   // (the two rules have the same cells: one pass is theirs both)
        QCOMPARE(b.grid.conditional().tests(), qint64(0));
    }

    // ---- the limits of what a sheet's rules cost (hostile workbooks)

    // Sixty rules of duplicate values over the same cells have one count between them, not sixty: one pass, and the cells held once.
    void rulesOverTheSameCellsShareTheirFigures()
    {
        Book b;
        for (int i = 0; i < 20000; ++i) b.sheet.cells.add(Book::numberCell(quint32(i), 0, double(i % 1000)));
        const int d = b.dxfBoldItalic(true, false);
        for (int i = 0; i < 60; ++i) b.rule(CondRule::Type::DuplicateValues, QStringLiteral("A1:A20000"), d);
        b.build();
        for (int i = 0; i < 60; ++i) QVERIFY(b.at(QStringLiteral("A%1").arg(1 + i)).any);
        QCOMPARE(b.grid.conditional().passes(), qint64(1));
        QCOMPARE(b.grid.conditional().statCells(), qint64(20000));
        // a count is kept for what a rule of unique values needs too, and the rules still tell the cells apart
        b.rule(CondRule::Type::UniqueValues, QStringLiteral("A1:A20000"), d);
        b.sheet.conditionalFormats.last().priority = 1000;
        b.grid.resetConditional();
        QVERIFY(b.at(QStringLiteral("A1")).any);
        QCOMPARE(b.grid.conditional().passes(), qint64(2));   // (one more: the same cells, for the sixty-one rules)
    }

    // The cells that a sheet's rules keep figures for are limited for the sheet, not for each rule: a rule whose figures would go past the limit shows nothing, and says why once.
    void theFiguresOfASheetAreHeldForAtMostALimit()
    {
        const PinnedLocale english;   // (the limit is written with the marks of a language)
        Book b;
        for (int i = 0; i < 600; ++i)
            for (int c = 0; c < 3; ++c) b.sheet.cells.add(Book::numberCell(quint32(i), quint32(c), double((i * 7 + c) % 50)));
        const int d = b.dxfBoldItalic(true, false);
        b.rule(CondRule::Type::DuplicateValues, QStringLiteral("A1:A600"), d);
        b.rule(CondRule::Type::DuplicateValues, QStringLiteral("B1:B600"), d);
        b.rule(CondRule::Type::DuplicateValues, QStringLiteral("C1:C600"), d);
        b.build();
        CondFormats::Limits limits;
        limits.statCells = 1300;
        b.grid.conditional().setLimits(limits);
        QVERIFY(b.at(QStringLiteral("A1")).any);
        QVERIFY(b.at(QStringLiteral("B1")).any);
        QVERIFY(!b.at(QStringLiteral("C1")).any);   // (the third set would be 1,800 cells in all)
        QVERIFY(b.grid.conditional().statCells() <= 1300);
        const QString said = b.grid.conditional().takeNotice();
        QVERIFY2(said.contains(QStringLiteral("1,300")) || said.contains(QStringLiteral("1300")), qPrintable(said));
        QVERIFY(b.grid.conditional().takeNotice().isEmpty());   // (once)
    }

    // A pass over a lot of cells is not one turn of the program: the cells show nothing until the figures are made, which is done a slice at a time, and the ones asked for after it are right.
    void aBigPassIsMadeInSlicesAndTheCellsShowNothingUntilItIsDone()
    {
        Book b;
        for (int i = 0; i < 5000; ++i) b.sheet.cells.add(Book::numberCell(quint32(i), 0, double(i)));
        CondRule &top = b.rule(CondRule::Type::Top10, QStringLiteral("A1:A5000"), b.dxfBoldItalic(true, false));
        top.rank = 3;
        b.build();
        CondFormats::Limits limits;
        limits.syncCells = 1000;
        limits.sliceCells = 500;
        b.grid.conditional().setLimits(limits);
        QVERIFY(!b.at(QStringLiteral("A5000")).any);   // (the figures are not made: the cell shows nothing yet)
        QVERIFY(b.grid.conditional().wantsMore());
        QCOMPARE(b.grid.conditional().passes(), qint64(0));
        int steps = 0;
        while (b.grid.conditional().step(1)) ++steps;
        QVERIFY2(steps >= 6, qPrintable(QString::number(steps)));   // (5,000 cells in slices of 500, the first 1,000 of them taken at the first ask)
        QVERIFY(!b.grid.conditional().wantsMore());
        QCOMPARE(b.grid.conditional().passes(), qint64(1));
        QVERIFY(b.at(QStringLiteral("A5000")).any && b.at(QStringLiteral("A4998")).any && !b.at(QStringLiteral("A4997")).any);
    }

    // A value that changes changes the figures of the rules that have it by that one cell: the counts of a rule of duplicates, and the lowest and highest of a scale, with no pass over the cells.
    void aChangedValueChangesTheFiguresByThatCell()
    {
        Book b;
        for (int i = 0; i < 50000; ++i) b.sheet.cells.add(Book::numberCell(quint32(i), 0, double(i)));   // (none twice)
        const int d = b.dxfBoldItalic(true, false);
        b.rule(CondRule::Type::DuplicateValues, QStringLiteral("A1:A50000"), d);
        CondRule &scale = b.rule(CondRule::Type::ColorScale, QStringLiteral("A1:A50000"));
        scale.values = {{CondValue::Type::Min, QString(), true}, {CondValue::Type::Max, QString(), true}};
        scale.colors = {rgb(0xFF0000), rgb(0x00FF00)};
        b.build();
        QVERIFY(b.at(QStringLiteral("A10")).hasScale);
        QVERIFY(b.formatAt(QStringLiteral("A10")).isEmpty());   // (not a duplicate)
        QCOMPARE(b.at(QStringLiteral("A1")).scale, QRgb(0xFFFF0000));   // (the lowest: the red end)
        QCOMPARE(b.grid.conditional().passes(), qint64(1));
        // A10 becomes what A20 has
        const_cast<Cell *>(b.sheet.cells.find(9, 0))->number = 19;
        b.grid.invalidateText(9, 0);
        QCOMPARE(b.grid.conditional().incrementalChanges(), qint64(1));
        QCOMPARE(b.grid.conditional().passes(), qint64(1));
        QVERIFY(!b.formatAt(QStringLiteral("A10")).isEmpty() && !b.formatAt(QStringLiteral("A20")).isEmpty());
        // the highest is brought down below all of them: the lowest of the scale is that, the highest the next, and the cell that was the lowest is in the middle
        const_cast<Cell *>(b.sheet.cells.find(49999, 0))->number = -50000;
        b.grid.invalidateText(49999, 0);
        QCOMPARE(b.grid.conditional().passes(), qint64(1));
        QCOMPARE(b.grid.conditional().incrementalChanges(), qint64(2));
        QCOMPARE(b.at(QStringLiteral("A49999")).scale, QRgb(0xFF00FF00));   // (49,998 is the highest now: the green end)
        const QRgb middle = b.at(QStringLiteral("A1")).scale;
        QVERIFY2(((middle >> 16) & 0xFF) > 100 && ((middle >> 16) & 0xFF) < 160 && ((middle >> 8) & 0xFF) > 100 && ((middle >> 8) & 0xFF) < 160, qPrintable(QString::number(middle, 16)));
    }

    // The rules are found through an index of their ranges: a cell costs what the rules that have it cost, not all of them.
    void rulesAreFoundThroughAnIndexOfTheirRanges()
    {
        Book b;
        for (quint32 r = 0; r < 400; ++r) {
            CondRule rule;
            rule.type = CondRule::Type::CellIs;
            rule.op = CondRule::Operator::GreaterThan;
            rule.formula1 = QStringLiteral("0");
            rule.priority = int(r) + 1;
            for (quint32 c = 0; c < 250; ++c) {
                CellRange one;
                one.valid = true;
                one.row1 = one.row2 = r;
                one.col1 = one.col2 = c;
                rule.ranges.append(one);
            }
            b.sheet.conditionalFormats.append(rule);
        }
        b.build();
        b.grid.conditional().reset();
        qint64 before = b.grid.conditional().indexSteps();
        QVERIFY(b.grid.conditional().covers(5, 7));
        QVERIFY(b.grid.conditional().covers(399, 249));
        QVERIFY(!b.grid.conditional().covers(5, 300));
        QVERIFY(!b.grid.conditional().covers(400, 0));
        const qint64 built = b.grid.conditional().indexSteps();
        for (int i = 0; i < 1000; ++i) b.grid.conditional().covers(quint32(i % 400), quint32((i * 7) % 250));
        // (a hundred thousand ranges: looking at each for each of a thousand cells would be a hundred million)
        QVERIFY2(b.grid.conditional().indexSteps() - built < 1000 * 400, qPrintable(QString::number(b.grid.conditional().indexSteps() - built)));
        Q_UNUSED(before);
    }

    // The ranges of a sheet's rules that are looked at are limited, and the rules past the limit are not shown, with a sentence that says so, once.
    void theRangesOfASheetAreLookedAtForAtMostALimit()
    {
        Book b;
        b.num(QStringLiteral("A1"), 5);
        for (int r = 0; r < 10; ++r) {
            CondRule rule;
            rule.type = CondRule::Type::CellIs;
            rule.op = CondRule::Operator::GreaterThan;
            rule.formula1 = QStringLiteral("0");
            rule.dxf = b.dxfBoldItalic(true, false);
            rule.priority = r + 1;
            for (int k = 0; k < 20; ++k) rule.ranges.append(range(QStringLiteral("A%1").arg(r * 20 + k + 1)));
            b.sheet.conditionalFormats.append(rule);
        }
        b.build();
        CondFormats::Limits limits;
        limits.ranges = 50;
        b.grid.conditional().setLimits(limits);
        QVERIFY(b.at(QStringLiteral("A1")).any);          // (the first rule is among the first two)
        QVERIFY(!b.grid.conditional().covers(0 + 100, 0));   // (the sixth rule's cells: past the limit)
        const QString said = b.grid.conditional().takeNotice();
        QVERIFY2(said.contains(QStringLiteral("50")), qPrintable(said));
        QVERIFY(b.grid.conditional().takeNotice().isEmpty());
    }

    // What a rule's formula costs is spread over the paints: a paint works out as many as the steps it has allow, the cells left plain are asked about again, and after a few paints
    // they all have what a paint with no limit would give them.
    void aRulesFormulaIsWorkedOutOverSeveralPaints()
    {
        Book b;
        for (int i = 0; i < 300; ++i) b.sheet.cells.add(Book::numberCell(quint32(i), 0, double(i)));
        CondRule &r = b.rule(CondRule::Type::Expression, QStringLiteral("B1:B100"), b.dxfBoldItalic(true, false));
        r.formula1 = QStringLiteral("SUM($A$1:$A$300)>44000+ROW()");
        b.build();
        CondFormats::Limits limits;
        limits.paintWork = 2000;
        b.grid.conditional().setLimits(limits);
        int rounds = 0;
        QVector<bool> got(100, false);
        for (; rounds < 100; ++rounds) {
            b.grid.conditional().beginPaint();
            for (int i = 0; i < 100; ++i) got[i] = got[i] || b.at(QStringLiteral("B%1").arg(i + 1)).any;
            if (!b.grid.conditional().wantsMore()) break;
            b.grid.conditional().step(1000);
        }
        QVERIFY2(rounds >= 2 && rounds < 100, qPrintable(QString::number(rounds)));
        // SUM(A1:A300) = 44850: the rule holds for the rows under 850
        for (int i = 0; i < 100; ++i) QVERIFY(got[i]);
        QVERIFY(b.grid.conditional().tests() >= 100);
    }

    // A rule whose formula needs more steps than one cell may take does not hold there, and the person is told once that a limit stopped it.
    void aFormulaPastItsLimitIsSaidOnce()
    {
        Book b;
        for (int i = 0; i < 2000; ++i) b.sheet.cells.add(Book::numberCell(quint32(i), 0, double(i)));
        CondRule &r = b.rule(CondRule::Type::Expression, QStringLiteral("B1:B5"), b.dxfBoldItalic(true, false));
        r.formula1 = QStringLiteral("SUMPRODUCT(--($A$1:$A$2000>A1))>5");
        b.build();
        CondFormats::Limits limits;
        limits.testWork = 500;
        b.grid.conditional().setLimits(limits);
        QVERIFY(!b.at(QStringLiteral("B1")).any);
        const QString said = b.grid.conditional().takeNotice();
        QVERIFY2(said.contains(QStringLiteral("formula")), qPrintable(said));
        QVERIFY(b.at(QStringLiteral("B2")).any == false);
        QVERIFY(b.grid.conditional().takeNotice().isEmpty());
        // with room for it, the rule holds
        limits.testWork = 1000000;
        b.grid.conditional().setLimits(limits);
        QVERIFY(b.at(QStringLiteral("B1")).any);
        QVERIFY(b.grid.conditional().takeNotice().isEmpty());
    }
};

JP_TEST_CLASS(GridCondFormatTests)
#include "tst_grid_condformat.moc"
