// Tests of the index of the columns and rows that lookups search again and again (calc/lookupindex.h), and of the graph that a workbook is told when it is first needed: the
// answers of VLOOKUP, HLOOKUP, MATCH, and LOOKUP are what the cells give with the indexes off, and an edit, a clear, a formula, or an insert of rows inside a searched range changes
// the next answer. Every workbook is built in memory.
#include "sheethelpers.h"

#include "sheet/calculation.h"

#include <memory>

namespace {

QString num(const QString &ref, const QString &value) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref, value); }
QString txt(const QString &ref, const QString &text) { return QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, text.toHtmlEscaped()); }
QString boolean(const QString &ref, bool v) { return QStringLiteral("<c r=\"%1\" t=\"b\"><v>%2</v></c>").arg(ref).arg(v ? 1 : 0); }
QString err(const QString &ref, const QString &e) { return QStringLiteral("<c r=\"%1\" t=\"e\"><v>%2</v></c>").arg(ref, e); }
QString fx(const QString &ref, const QString &formula) { return QStringLiteral("<c r=\"%1\"><f>%2</f></c>").arg(ref, formula.toHtmlEscaped()); }
QString row(int n, const QString &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(n).arg(cells); }

struct Book {
    Package package;
    Workbook book;
    std::unique_ptr<Calculation> calc;
    bool open(const QByteArray &zip, bool indexes = true, bool lazy = true)
    {
        QString why;
        if (!package.readLazy(zip, &why) || !book.open(package, &why)) return false;
        CalcOptions options;
        options.lazyGraph = lazy;
        calc.reset(new Calculation(book, options));
        calc->useLookupIndexes(indexes);
        return calc->load();
    }
    static quint32 rowOf(const QString &ref)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return r;
    }
    static quint32 colOf(const QString &ref)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return c;
    }
    QString at(const QString &ref, int sheet = 0) const { return calc->cell(sheet, rowOf(ref), colOf(ref)).debug(); }
    void finish() { while (!calc->recalculate().finished) {} }
};

// A1:A40 the keys (numbers with repeats, texts in both cases, true and false, an error, a text with a character that is not ASCII, a text with a star, and gaps); B1:B40 100 to 4000.
using CellRows = QMap<int, QString>;

CellRows dataMap()
{
    CellRows m;
    auto set = [&](int r, const QString &cell) { m[r] += cell; };
    set(1, num("A1", "10"));
    set(2, txt("A2", "Bob"));
    set(3, num("A3", "20"));
    set(4, boolean("A4", true));
    set(5, txt("A5", "bob"));
    set(6, num("A6", "30"));
    set(7, err("A7", "#N/A"));
    // 8 is empty
    set(9, txt("A9", QString::fromUtf8("\xC3\x89" "clair")));
    set(10, txt("A10", "eclair"));
    set(11, num("A11", "40"));
    set(12, txt("A12", "a*b"));
    set(13, num("A13", "20"));
    set(14, boolean("A14", false));
    // 15 is empty
    set(16, num("A16", "60"));
    set(17, txt("A17", "axb"));
    set(18, txt("A18", "Zed"));
    for (int r = 19; r <= 40; ++r) set(r, num(QStringLiteral("A%1").arg(r), QString::number(100 + r)));
    for (int r = 1; r <= 40; ++r) set(r, num(QStringLiteral("B%1").arg(r), QString::number(r * 100)));
    return m;
}

// The map as the sheet's rows, each row's cells in column order (the data is in A and B, what is added to a row is to the right of them).
QString rowsOf(const CellRows &m)
{
    QString rows;
    for (auto it = m.constBegin(); it != m.constEnd(); ++it) rows += row(it.key(), it.value());
    return rows;
}

QString dataRows(int firstRowOfFormulas = 0, const QString &formulaCells = QString())
{
    CellRows m = dataMap();
    if (firstRowOfFormulas > 0) m[firstRowOfFormulas] += formulaCells;
    return rowsOf(m);
}

QString columnName(int zeroBased)
{
    QString name;
    int x = zeroBased;
    while (x >= 0) {
        name.prepend(QChar(u'A' + x % 26));
        x = x / 26 - 1;
    }
    return name;
}

} // namespace

class SheetLookupIndexTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Every lookup of the five kinds, on keys of each kind, gives what it gives with the indexes off, and the indexes were used.
    void indexedLookupsAnswerAsTheCellsDo()
    {
        const QStringList keys = {QStringLiteral("20"), QStringLiteral("30.5"), QStringLiteral("0"), QStringLiteral("\"BOB\""), QStringLiteral("\"bob\""), QStringLiteral("\"ECLAIR\""),
                                  QString::fromUtf8("\"\xC3\x89" "CLAIR\""), QStringLiteral("TRUE"), QStringLiteral("FALSE"), QStringLiteral("\"a*b\""), QStringLiteral("\"b*\""),
                                  QStringLiteral("\"a?b\""), QStringLiteral("\"*\""), QStringLiteral("\"zzz\""), QStringLiteral("\"Zed\""), QStringLiteral("114"), QStringLiteral("141"),
                                  QStringLiteral("100"), QStringLiteral("\"\""), QStringLiteral("#N/A")};
        QStringList cells;
        int n = 0;
        auto add = [&](const QString &formula) { cells << fx(QStringLiteral("D%1").arg(++n), formula); };
        for (const QString &k : keys) {
            add(QStringLiteral("VLOOKUP(%1,$A$1:$B$40,2,FALSE)").arg(k));
            add(QStringLiteral("VLOOKUP(%1,$A$1:$B$40,2,TRUE)").arg(k));
            add(QStringLiteral("MATCH(%1,$A$1:$A$40,0)").arg(k));
            add(QStringLiteral("MATCH(%1,$A$1:$A$40,1)").arg(k));
            add(QStringLiteral("MATCH(%1,$A$1:$A$40,-1)").arg(k));
            add(QStringLiteral("LOOKUP(%1,$A$1:$A$40,$B$1:$B$40)").arg(k));
            add(QStringLiteral("HLOOKUP(%1,$F$1:$AS$2,2,FALSE)").arg(k));
            add(QStringLiteral("HLOOKUP(%1,$F$1:$AS$2,2,TRUE)").arg(k));
            add(QStringLiteral("MATCH(%1,$F$1:$AS$1,0)").arg(k));
        }
        // the formulas down column D (from row 1, past the data's 40 rows), and a row of 40 cells in F1:AS1 with a row of numbers under it for HLOOKUP
        CellRows m = dataMap();
        for (int i = 0; i < cells.size(); ++i) m[i + 1] += cells[i];
        const char *kinds[] = {"n10", "sBob", "n20", "bT", "sbob", "n30", "e", "sx", "sEclair", "n40", "sa*b", "n20", "bF", "sy", "n60", "saxb", "sZed", "n119", "n120", "n121",
                               "n122", "n123", "n124", "n125", "n126", "n127", "n128", "n129", "n130", "n131", "n132", "n133", "n134", "n135", "n136", "n137", "n138", "n139", "n140", "n141"};
        for (int c = 0; c < 40; ++c) {
            const QString ref1 = columnName(5 + c) + QStringLiteral("1");
            const QString k = QString::fromLatin1(kinds[c]);
            if (k[0] == u'n') m[1] += num(ref1, k.mid(1));
            else if (k[0] == u'b') m[1] += boolean(ref1, k[1] == u'T');
            else if (k[0] == u'e') m[1] += err(ref1, "#N/A");
            else m[1] += txt(ref1, k.mid(1));
            m[2] += num(columnName(5 + c) + QStringLiteral("2"), QString::number((c + 1) * 7));
        }
        const QString sheet = rowsOf(m);
        const QByteArray zip = buildXlsx({worksheet(sheet)});
        Book on, off;
        QVERIFY(on.open(zip, true));
        QVERIFY(off.open(zip, false));
        on.finish();
        off.finish();
        int compared = 0;
        for (int r = 1; r <= cells.size(); ++r) {
            const QString ref = QStringLiteral("D%1").arg(r);
            QCOMPARE(on.at(ref), off.at(ref));
            ++compared;
        }
        QVERIFY(compared > 150);
        const LookupStats stats = on.calc->lookupStats();
        QVERIFY2(stats.builds >= 2, qPrintable(QStringLiteral("builds %1").arg(stats.builds)));
        QVERIFY2(stats.hits >= 100, qPrintable(QStringLiteral("hits %1").arg(stats.hits)));
        QCOMPARE(off.calc->lookupStats().builds, qint64(0));
    }

    // An edit, a clear, a formula, and rows inserted inside a searched range change the next lookup: the index of the range is dropped by the write.
    void aWriteInsideARangeChangesTheNextAnswer()
    {
        const QString formulas = fx("D1", "MATCH(777,$A$1:$A$40,0)") + fx("E1", "VLOOKUP(777,$A$1:$B$40,2,FALSE)") + fx("F1", "MATCH(777,$A$1:$A$40,1)") + fx("G1", "LOOKUP(777,$A$1:$A$40,$B$1:$B$40)");
        const QString more = row(41, fx("D41", "MATCH(40,$A$1:$A$40,0)") + fx("E41", "VLOOKUP(2,$A$1:$B$40,2,FALSE)"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(dataRows(1, formulas) + more)})));
        b.finish();
        QCOMPARE(b.at("D1"), QStringLiteral("e:#N/A"));
        QCOMPARE(b.at("E1"), QStringLiteral("e:#N/A"));
        QVERIFY(b.calc->lookupStats().builds >= 1);
        QVERIFY(b.calc->lookupStats().hits >= 1);
        // a number written into the range (A20, row 20)
        QVERIFY(b.calc->setValue(0, 19, 0, Value::number(777)));
        b.finish();
        QCOMPARE(b.at("D1"), QStringLiteral("n:20"));
        QCOMPARE(b.at("E1"), QStringLiteral("n:2000"));
        // searched in order, 777 is not before 100+19 .. and the halving meets it: the answer is what the same search gives with the cells read one by one
        Book plain;
        QVERIFY(plain.open(buildXlsx({worksheet(dataRows(1, formulas) + more)}), false));
        plain.finish();
        QVERIFY(plain.calc->setValue(0, 19, 0, Value::number(777)));
        plain.finish();
        for (const char *ref : {"D1", "E1", "F1", "G1"}) QCOMPARE(b.at(ref), plain.at(ref));
        // the cell cleared
        QVERIFY(b.calc->clearCell(0, 19, 0));
        QVERIFY(plain.calc->clearCell(0, 19, 0));
        b.finish();
        plain.finish();
        QCOMPARE(b.at("D1"), QStringLiteral("e:#N/A"));
        for (const char *ref : {"D1", "E1", "F1", "G1"}) QCOMPARE(b.at(ref), plain.at(ref));
        // a formula put in the range, which makes its value 777
        QVERIFY(b.calc->setFormula(0, 24, 0, QStringLiteral("700+77")));
        QVERIFY(plain.calc->setFormula(0, 24, 0, QStringLiteral("700+77")));
        b.finish();
        plain.finish();
        QCOMPARE(b.at("D1"), QStringLiteral("n:25"));
        for (const char *ref : {"D1", "E1", "F1", "G1"}) QCOMPARE(b.at(ref), plain.at(ref));
        // and the formula is a constant again, elsewhere
        QVERIFY(b.calc->setValue(0, 24, 0, Value::number(5)));
        QVERIFY(plain.calc->setValue(0, 24, 0, Value::number(5)));
        QVERIFY(b.calc->setValue(0, 30, 0, Value::number(777)));
        QVERIFY(plain.calc->setValue(0, 30, 0, Value::number(777)));
        b.finish();
        plain.finish();
        QCOMPARE(b.at("D1"), QStringLiteral("n:31"));
        for (const char *ref : {"D1", "E1", "F1", "G1"}) QCOMPARE(b.at(ref), plain.at(ref));
        // rows inserted above the range's first rows: the ranges follow, the answers are those of the places now
        Calculation::EditRecord rec;
        Calculation::EditRecord recPlain;
        QVERIFY(b.calc->applyEdit(StructEdit::insertRows(0, 2, 2), &rec));
        QVERIFY(plain.calc->applyEdit(StructEdit::insertRows(0, 2, 2), &recPlain));
        b.finish();
        plain.finish();
        QCOMPARE(b.at("D1"), QStringLiteral("n:33"));   // (the 777 is two places further down the range)
        QVERIFY2(b.at("D1") == plain.at("D1") && b.at("E1") == plain.at("E1") && b.at("F1") == plain.at("F1") && b.at("G1") == plain.at("G1"), "after the rows were inserted");
        // and a value written into the inserted rows
        QVERIFY(b.calc->setValue(0, 2, 0, Value::number(777)));
        QVERIFY(plain.calc->setValue(0, 2, 0, Value::number(777)));
        b.finish();
        plain.finish();
        QCOMPARE(b.at("D1"), QStringLiteral("n:3"));
        QCOMPARE(b.at("D1"), plain.at("D1"));
    }

    // A range with a formula in it is read from the cells, as before, and counted as refused.
    void aRangeWithAFormulaIsNotIndexed()
    {
        const QString formulas = fx("D1", "MATCH(40,$A$1:$A$40,0)") + fx("E1", "MATCH(999,$A$1:$A$40,0)") + fx("F1", "MATCH(998,$A$1:$A$40,0)") + fx("G1", "MATCH(997,$A$1:$A$40,0)")
                                 + fx("H1", "MATCH(996,$A$1:$A$40,0)");
        QString rows = dataRows(1, formulas);
        rows.replace(num("A11", "40"), fx("A11", "20+20"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        b.finish();
        QCOMPARE(b.at("D1"), QStringLiteral("n:11"));
        QCOMPARE(b.at("E1"), QStringLiteral("e:#N/A"));
        QCOMPARE(b.calc->lookupStats().hits, qint64(0));
        QVERIFY(b.calc->lookupStats().refused >= 1);
    }

    // More hot lines than the cache holds, searched round and round: the builds stay few (a line thrown out is not built again at once), and every answer is right.
    void theIndexesStayWithinTheirBounds()
    {
        // 70 columns of 40 numbers, each searched by three formulas that an edit of Z1 works out again
        CellRows m;
        for (int c = 0; c < 70; ++c)
            for (int r = 1; r <= 40; ++r) m[r] += num(columnName(c) + QString::number(r), QString::number(r * 10 + c));
        m[1] += num("CC1", "0");
        for (int k = 0; k < 3; ++k)
            for (int c = 0; c < 70; ++c) m[41 + k] += fx(columnName(70 + c) + QString::number(41 + k), QStringLiteral("MATCH(%1+0*$CC$1,%2$1:%2$40,0)").arg(38 * 10 + c).arg(columnName(c)));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rowsOf(m))})));
        b.finish();
        for (int round = 1; round <= 8; ++round) {
            QVERIFY(b.calc->setValue(0, 0, 80, Value::number(round)));
            b.finish();
        }
        const LookupStats stats = b.calc->lookupStats();
        QVERIFY2(stats.entries <= LookupCache::kMaxEntries, qPrintable(QStringLiteral("entries %1").arg(stats.entries)));
        QVERIFY(stats.bytes >= 0 && stats.bytes <= LookupCache::kMaxBytes);
        QVERIFY2(stats.builds <= 150, qPrintable(QStringLiteral("builds %1").arg(stats.builds)));
        for (int c = 0; c < 70; ++c) QCOMPARE(b.at(columnName(70 + c) + QStringLiteral("41")), QStringLiteral("n:38"));
    }

    // A table above the size a build pays at, searched with a key near its start: the answers are right and no index is built (the searches cost less than a build); with a key at its
    // end the index is built once.
    void aLargeRangeIsIndexedOnlyWhereItPays()
    {
        CellRows m;
        for (int r = 1; r <= 3000; ++r) m[r] += num(QStringLiteral("A%1").arg(r), QString::number(r)) + num(QStringLiteral("B%1").arg(r), QString::number(r * 2));
        for (int k = 0; k < 6; ++k) m[1 + k] += fx(QStringLiteral("D%1").arg(1 + k), QStringLiteral("VLOOKUP(%1+0*$F$1,$A$1:$B$3000,2,FALSE)").arg(3 + k)) + (k == 0 ? num("F1", "0") : QString());
        Book early;
        QVERIFY(early.open(buildXlsx({worksheet(rowsOf(m))})));
        early.finish();
        QCOMPARE(early.at("D1"), QStringLiteral("n:6"));
        QCOMPARE(early.at("D6"), QStringLiteral("n:16"));
        QCOMPARE(early.calc->lookupStats().builds, qint64(0));
        CellRows far;
        for (int r = 1; r <= 3000; ++r) far[r] += num(QStringLiteral("A%1").arg(r), QString::number(r)) + num(QStringLiteral("B%1").arg(r), QString::number(r * 2));
        for (int k = 0; k < 6; ++k) far[1 + k] += fx(QStringLiteral("D%1").arg(1 + k), QStringLiteral("VLOOKUP(%1+0*$F$1,$A$1:$B$3000,2,FALSE)").arg(2990 + k)) + (k == 0 ? num("F1", "0") : QString());
        Book late;
        QVERIFY(late.open(buildXlsx({worksheet(rowsOf(far))})));
        late.finish();
        QCOMPARE(late.at("D1"), QStringLiteral("n:5980"));
        QCOMPARE(late.calc->lookupStats().builds, qint64(1));
        QVERIFY(late.calc->lookupStats().hits >= 3);
    }

    // An edit of a cell outside the searched table (a number, or a format), twenty times: the table's index is built once.
    void editsOutsideTheTableKeepItsIndex()
    {
        CellRows m;
        for (int r = 1; r <= 3000; ++r) m[r] += num(QStringLiteral("A%1").arg(r), QString::number(r)) + num(QStringLiteral("B%1").arg(r), QString::number(r * 2));
        m[1] += fx("D1", "VLOOKUP($F$1,$A$1:$B$3000,2,FALSE)") + num("F1", "2990");
        m[2] += fx("D2", "MATCH($F$1,$A$1:$A$3000,0)");
        m[3] += fx("D3", "VLOOKUP($F$1+1,$A$1:$B$3000,2,FALSE)");
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rowsOf(m))})));
        b.finish();
        for (int i = 0; i < 20; ++i) {
            QVERIFY(b.calc->setValue(0, 0, 5, Value::number(2980 + i)));
            if (i % 2) QVERIFY(b.calc->setStyle(0, 4000 + i, 8, 0));
            b.finish();
            QCOMPARE(b.at("D2"), QStringLiteral("n:%1").arg(2980 + i));
            QCOMPARE(b.at("D1"), QStringLiteral("n:%1").arg((2980 + i) * 2));
        }
        QVERIFY2(b.calc->lookupStats().builds <= 1, qPrintable(QStringLiteral("builds %1").arg(b.calc->lookupStats().builds)));
        // a write inside the table drops it, and the answer is the new one
        QVERIFY(b.calc->setValue(0, 100, 0, Value::number(77777)));
        QVERIFY(b.calc->setValue(0, 0, 5, Value::number(77777)));
        b.finish();
        QCOMPARE(b.at("D2"), QStringLiteral("n:101"));
    }

    // Texts that look like numbers are not numbers to a lookup, with the index as without it; and a text against a number cell.
    void aTextThatLooksLikeANumberIsNotTheNumber()
    {
        CellRows m;
        for (int r = 1; r <= 40; ++r) m[r] += (r % 2 ? num(QStringLiteral("A%1").arg(r), QString::number(r)) : txt(QStringLiteral("A%1").arg(r), QString::number(r))) + num(QStringLiteral("B%1").arg(r), QString::number(r));
        QStringList f;
        for (int k = 0; k < 12; ++k) {
            const int n = k == 0 ? 3 : k == 1 ? 4 : 4 + k;
            const bool asText = k == 1 || (k > 1 && k % 2);
            f << fx(QStringLiteral("D%1").arg(1 + k), QStringLiteral("MATCH(%1,$A$1:$A$40,0)").arg(asText ? QStringLiteral("\"%1\"").arg(n) : QString::number(n)));
        }
        for (int k = 0; k < 12; ++k) m[1 + k] += f[k];
        const QByteArray zip = buildXlsx({worksheet(rowsOf(m))});
        Book on, off;
        QVERIFY(on.open(zip, true));
        QVERIFY(off.open(zip, false));
        on.finish();
        off.finish();
        for (int k = 1; k <= 12; ++k) QCOMPARE(on.at(QStringLiteral("D%1").arg(k)), off.at(QStringLiteral("D%1").arg(k)));
        QCOMPARE(on.at("D1"), QStringLiteral("n:3"));       // 3 is a number in A3
        QCOMPARE(on.at("D2"), QStringLiteral("n:4"));       // "4" is a text, and A4 holds one
        QCOMPARE(on.at("D3"), QStringLiteral("e:#N/A"));    // the number 6, and A6 holds the text "6"
        QCOMPARE(on.at("D4"), QStringLiteral("e:#N/A"));    // the text "7", and A7 holds the number 7
    }

    // The work an indexed search is counted at is what the scan's is, long texts that agree for hundreds of characters included.
    void anIndexedSearchTakesTheStepsOfTheScan()
    {
        const QString prefix = QString(240, QLatin1Char(0x78));
        CellRows m;
        for (int r = 1; r <= 60; ++r) m[r] += txt(QStringLiteral("A%1").arg(r), prefix + QStringLiteral("%1").arg(r, 4, 10, QLatin1Char('0'))) + num(QStringLiteral("B%1").arg(r), QString::number(r));
        for (int k = 0; k < 12; ++k) m[1 + k] += fx(QStringLiteral("D%1").arg(1 + k), QStringLiteral("MATCH(\"%1%2\",$A$1:$A$60,0)").arg(prefix).arg(40 + k, 4, 10, QLatin1Char('0')));
        const QByteArray zip = buildXlsx({worksheet(rowsOf(m))});
        Book on, off;
        QVERIFY(on.open(zip, true));
        QVERIFY(off.open(zip, false));
        on.finish();
        off.finish();
        QVERIFY2(on.calc->lookupStats().hits >= 1, qPrintable(QStringLiteral("builds %1 refused %2 scan %3").arg(on.calc->lookupStats().builds).arg(on.calc->lookupStats().refused).arg(on.calc->lookupStats().scanSteps)));
        QCOMPARE(on.at("D12"), off.at("D12"));
        const qint64 a = on.calc->workUsed(), b = off.calc->workUsed();
        QVERIFY2(qAbs(a - b) * 50 <= b, qPrintable(QStringLiteral("work %1 with the index, %2 without").arg(a).arg(b)));
    }

    // Undo and redo of an edit of rows, a paste, and a sort, each inside the searched range: the next lookup sees it.
    void undoPasteAndSortAreSeenByTheNextLookup()
    {
        const QString formulas = fx("D1", "MATCH(555,$A$1:$A$40,0)") + fx("E1", "MATCH(556,$A$1:$A$40,0)") + fx("F1", "MATCH(557,$A$1:$A$40,0)") + fx("G1", "MATCH(558,$A$1:$A$40,0)")
                                 + fx("H1", "MATCH(559,$A$1:$A$40,0)");
        const QByteArray zip = buildXlsx({worksheet(dataRows(1, formulas))});
        Book b, plain;
        QVERIFY(b.open(zip));
        QVERIFY(plain.open(zip, false));
        auto both = [&](auto &&f) {
            f(*b.calc);
            f(*plain.calc);
            b.finish();
            plain.finish();
            for (const char *ref : {"D1", "E1", "F1", "G1", "H1"}) QCOMPARE(b.at(ref), plain.at(ref));
        };
        b.finish();
        plain.finish();
        QVERIFY(b.calc->lookupStats().builds >= 1);
        // paste: values written over a block
        both([](Calculation &c) { for (int i = 0; i < 5; ++i) c.setValue(0, 30 + i, 0, Value::number(555 + i)); });
        QCOMPARE(b.at("D1"), QStringLiteral("n:31"));
        QCOMPARE(b.at("H1"), QStringLiteral("n:35"));
        // sort: the same values in the other order
        both([](Calculation &c) { for (int i = 0; i < 5; ++i) c.setValue(0, 30 + i, 0, Value::number(559 - i)); });
        QCOMPARE(b.at("D1"), QStringLiteral("n:35"));
        // an edit of rows, undone and done again
        Calculation::EditRecord rec, recPlain;
        QVERIFY(b.calc->applyEdit(StructEdit::insertRows(0, 5, 3), &rec));
        QVERIFY(plain.calc->applyEdit(StructEdit::insertRows(0, 5, 3), &recPlain));
        b.finish();
        plain.finish();
        QCOMPARE(b.at("D1"), QStringLiteral("n:38"));
        QCOMPARE(b.at("D1"), plain.at("D1"));
        QVERIFY(b.calc->undoEdit(rec));
        QVERIFY(plain.calc->undoEdit(recPlain));
        b.finish();
        plain.finish();
        QCOMPARE(b.at("D1"), QStringLiteral("n:35"));
        for (const char *ref : {"D1", "E1", "F1", "G1", "H1"}) QCOMPARE(b.at(ref), plain.at(ref));
        // delete rows, and undo it
        Calculation::EditRecord del, delPlain;
        QVERIFY(b.calc->applyEdit(StructEdit::deleteRows(0, 1, 2), &del));
        QVERIFY(plain.calc->applyEdit(StructEdit::deleteRows(0, 1, 2), &delPlain));
        b.finish();
        plain.finish();
        QCOMPARE(b.at("D1"), plain.at("D1"));
        QVERIFY(b.calc->undoEdit(del));
        QVERIFY(plain.calc->undoEdit(delPlain));
        b.finish();
        plain.finish();
        QCOMPARE(b.at("D1"), QStringLiteral("n:35"));
        // a cell put back whole
        const Calculation::CellContent was = b.calc->content(0, 34, 0);
        both([](Calculation &c) { c.setValue(0, 34, 0, Value::number(1)); });
        QCOMPARE(b.at("D1"), QStringLiteral("e:#N/A"));   // (the 555 was in that cell)
        QVERIFY(b.calc->put(0, 34, 0, was));
        QVERIFY(plain.calc->put(0, 34, 0, was));
        b.finish();
        plain.finish();
        for (const char *ref : {"D1", "E1", "F1", "G1", "H1"}) QCOMPARE(b.at(ref), plain.at(ref));
    }

    // A sheet renamed or deleted: a lookup that reads a range on it follows the rewrite, with the index as without it.
    void aSheetRenamedOrDeletedIsFollowed()
    {
        QString f = fx("D1", "MATCH(555,Sheet2!$A$1:$A$40,0)") + fx("E1", "MATCH(556,Sheet2!$A$1:$A$40,0)") + fx("F1", "MATCH(557,Sheet2!$A$1:$A$40,0)") + fx("G1", "MATCH(558,Sheet2!$A$1:$A$40,0)");
        CellRows other = dataMap();
        other[30] += QString();
        const QByteArray zip = buildXlsx({worksheet(row(1, f)), worksheet(rowsOf(other))});
        Book b, plain;
        QVERIFY(b.open(zip));
        QVERIFY(plain.open(zip, false));
        b.finish();
        plain.finish();
        QVERIFY(b.calc->setValue(1, 29, 0, Value::number(555)));
        QVERIFY(plain.calc->setValue(1, 29, 0, Value::number(555)));
        b.finish();
        plain.finish();
        QCOMPARE(b.at("D1"), QStringLiteral("n:30"));
        Calculation::SheetRewrite r1, r2;
        QVERIFY(b.calc->rewriteSheetReferences(QStringLiteral("Sheet2"), QString(), &r1));
        QVERIFY(plain.calc->rewriteSheetReferences(QStringLiteral("Sheet2"), QString(), &r2));
        b.finish();
        plain.finish();
        QCOMPARE(b.at("D1"), plain.at("D1"));
        QCOMPARE(b.at("D1"), QStringLiteral("e:#REF!"));
    }

    // A workbook whose graph has no room for its formulas refuses the edits of lines, whether or not the graph was told yet.
    void anEditOfLinesIsRefusedWhenTheGraphIsOverCapacity()
    {
        QString rows = row(1, num("A1", "1"));
        for (int i = 2; i <= 200; ++i) rows += row(i, fx(QStringLiteral("B%1").arg(i), QStringLiteral("A1+%1").arg(i)) + fx(QStringLiteral("C%1").arg(i), QStringLiteral("SUM(A$1:A%1)").arg(i)));
        const QByteArray zip = buildXlsx({worksheet(rows)});
        for (const bool lazy : {false, true}) {
            Package package;
            Workbook book;
            QString why;
            QVERIFY(package.readLazy(zip, &why) && book.open(package, &why));
            CalcOptions options;
            options.lazyGraph = lazy;
            options.maxGraphBytes = 8000;
            Calculation calc(book, options);
            QVERIFY(calc.load());
            Calculation::EditRecord rec;
            QVERIFY2(!calc.applyEdit(StructEdit::insertRows(0, 5, 1), &rec), lazy ? "lazy graph accepted the edit" : "eager graph accepted the edit");
            QVERIFY(calc.untrackedCount() > 0);
        }
    }

    // After an edit left to the recalculation, formulas the graph had no room for, those that change by themselves, and those in places all have the new value.
    void theFirstRecalculationAfterAnEditWorksOutWhatTheGraphLeftOut()
    {
        QString rows = row(1, num("A1", "1") + fx("B1", "SUM(Sheet1:Sheet2!A2)+A1"));
        for (int i = 2; i <= 150; ++i) rows += row(i, fx(QStringLiteral("C%1").arg(i), QStringLiteral("A1+%1").arg(i)) + fx(QStringLiteral("D%1").arg(i), QStringLiteral("OFFSET(A1,0,0)+%1").arg(i)));
        const QByteArray sheet2 = worksheet(row(2, num("A2", "10")));
        const QByteArray zip = buildXlsx({worksheet(rows), sheet2});
        for (const Calculation::Marking marking : {Calculation::Marking::Later, Calculation::Marking::Now}) {
            Package package;
            Workbook book;
            QString why;
            QVERIFY(package.readLazy(zip, &why) && book.open(package, &why));
            CalcOptions options;
            options.maxGraphBytes = 9000;
            Calculation calc(book, options);
            QVERIFY(calc.load());
            while (!calc.recalculate().finished) {}
            QVERIFY(calc.setValue(0, 0, 0, Value::number(100), marking));
            while (!calc.recalculate().finished) {}
            QVERIFY(calc.untrackedCount() > 0);
            QCOMPARE(calc.cell(0, 0, 1).debug(), QStringLiteral("n:110"));
            for (int i = 2; i <= 150; ++i) {
                QVERIFY2(calc.cell(0, quint32(i - 1), 2).debug() == QStringLiteral("n:%1").arg(100 + i), qPrintable(QStringLiteral("C%1 is %2").arg(i).arg(calc.cell(0, quint32(i - 1), 2).debug())));
                QVERIFY2(calc.cell(0, quint32(i - 1), 3).debug() == QStringLiteral("n:%1").arg(100 + i), qPrintable(QStringLiteral("D%1 is %2").arg(i).arg(calc.cell(0, quint32(i - 1), 3).debug())));
            }
        }
    }

    // A formula the graph was not told because lines moved first is worked out every time; once it is a value, or told, the count of the untracked is right.
    void theUntrackedCountFollowsAFormulaThatIsTold()
    {
        const QByteArray zip = buildXlsx({worksheet(row(1, num("A1", "1") + fx("B1", "SUM(Sheet1:Sheet2!A2)"))), worksheet(row(2, num("A2", "10")))});
        Book b;
        QVERIFY(b.open(zip));
        b.calc->pretendLinesMovedForTest();
        QCOMPARE(b.calc->untrackedCount(), qint64(1));
        QVERIFY(b.calc->setValue(0, 0, 1, Value::number(5)));
        QCOMPARE(b.calc->untrackedCount(), qint64(0));
        // told after all: a formula put in again is followed like the rest
        QVERIFY(b.calc->setFormula(0, 0, 1, QStringLiteral("SUM(Sheet1:Sheet2!A2)")));
        b.finish();
        QCOMPARE(b.calc->untrackedCount(), qint64(0));
        QCOMPARE(b.at("B1"), QStringLiteral("n:10"));
        QVERIFY(b.calc->setValue(1, 1, 0, Value::number(30)));
        b.finish();
        QCOMPARE(b.at("B1"), QStringLiteral("n:30"));
    }

    // A workbook that was just opened has told the graph nothing; the first need tells it all, and what it holds is what the eager way held.
    void theGraphIsToldOnTheFirstNeed()
    {
        QString rows = row(1, num("A1", "1"));
        for (int i = 2; i <= 50; ++i) rows += row(i, fx(QStringLiteral("B%1").arg(i), QStringLiteral("A1+%1").arg(i)) + fx(QStringLiteral("C%1").arg(i), QStringLiteral("SUM(A$1:A%1)").arg(i)));
        const QByteArray zip = buildXlsx({worksheet(rows)});
        Book lazy, eager;
        QVERIFY(lazy.open(zip, true, true));
        QVERIFY(eager.open(zip, true, false));
        QVERIFY(lazy.calc->graphPendingCount() >= 98);
        QCOMPARE(eager.calc->graphPendingCount(), qint64(0));
        lazy.finish();   // (the first calculation of what was opened needs no graph)
        QVERIFY(lazy.calc->graphPendingCount() >= 98);
        QCOMPARE(lazy.at("B10"), QStringLiteral("n:11"));
        QCOMPARE(lazy.calc->graph().entryCount(), eager.calc->graph().entryCount());
        QCOMPARE(lazy.calc->graphPendingCount(), qint64(0));
        // an edit before anything asked the graph: it is told first, and the readers are found
        Book edited;
        QVERIFY(edited.open(zip, true, true));
        edited.finish();
        QVERIFY(edited.calc->graphPendingCount() > 0);
        QVERIFY(edited.calc->setValue(0, 0, 0, Value::number(100)));
        edited.finish();
        QCOMPARE(edited.at("B10"), QStringLiteral("n:110"));
        QCOMPARE(edited.at("C50"), QStringLiteral("n:100"));
        QCOMPARE(edited.calc->graphPendingCount(), qint64(0));
    }

    // Rows inserted into a sheet before the graph was told, on a formula whose references are places (a reference through two sheets): it is still worked out after an edit it reads.
    void aFormulaInPlacesIsRightAfterRowsWereInsertedFirst()
    {
        const QByteArray sheet1 = worksheet(row(1, num("A1", "1") + fx("B1", "SUM(Sheet1:Sheet2!A2)")) + row(2, num("A2", "2")));
        const QByteArray sheet2 = worksheet(row(1, num("A1", "10")) + row(2, num("A2", "20")));
        const QByteArray zip = buildXlsx({sheet1, sheet2});
        Book b;
        QVERIFY(b.open(zip, true, true));
        b.finish();
        QCOMPARE(b.at("B1"), QStringLiteral("n:22"));
        Calculation::EditRecord rec;
        QVERIFY(b.calc->applyEdit(StructEdit::insertRows(1, 0, 1), &rec));   // a row at the top of Sheet2
        b.finish();
        QCOMPARE(b.at("B1"), QStringLiteral("n:12"));   // (the reference through two sheets stays at A2, which holds the 10 that was in A1)
        QVERIFY(b.calc->setValue(1, 1, 0, Value::number(500)));
        b.finish();
        QCOMPARE(b.at("B1"), QStringLiteral("n:502"));
        // the same with the graph told while the workbook was read
        Book eager;
        QVERIFY(eager.open(zip, true, false));
        eager.finish();
        QVERIFY(eager.calc->applyEdit(StructEdit::insertRows(1, 0, 1), &rec));
        eager.finish();
        QVERIFY(eager.calc->setValue(1, 1, 0, Value::number(500)));
        eager.finish();
        QCOMPARE(eager.at("B1"), b.at("B1"));
    }
};

JP_TEST_CLASS(SheetLookupIndexTests)
#include "tst_sheet_lookup_index.moc"
