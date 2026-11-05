// Tests of the copies of a workbook in the old binary form (.xls) and in OpenDocument (.ods): a workbook of the XML form is built in memory, written as the copy, and read back
// by this program's own readers; what the copy holds is what the original held (values, formulas, formats, sizes, merged cells, names, panes), and what a copy cannot hold or a
// damaged model would make too large is cut and named. Nothing here reads a file made by another program.
#include "odshelpers.h"

#include "io/cfb.h"
#include "xlsvalidate.h"
#include "sheet/oldsave.h"
#include "sheet/oldsavecommon.h"
#include "sheet/xlscompile.h"
#include "sheet/xlsrecord.h"

#include <QRegularExpression>
#include <QSet>

namespace {

const QString kS = QStringLiteral("xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"");

QByteArray stylesWith(const QString &numFmts, const QString &fonts, const QString &fills, const QString &borders, const QString &xfs)
{
    return ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<styleSheet " + kS + ">" + (numFmts.isEmpty() ? QString() : "<numFmts>" + numFmts + "</numFmts>")
            + "<fonts>" + fonts + "</fonts><fills>" + fills + "</fills><borders>" + borders + "</borders><cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
            + "<cellXfs>" + xfs + "</cellXfs></styleSheet>").toUtf8();
}

QString num(const QString &ref, double v, int style = 0) { return QStringLiteral("<c r=\"%1\" s=\"%3\"><v>%2</v></c>").arg(ref, QString::number(v, 'g', 17)).arg(style); }
QString str(const QString &ref, const QString &s, int style = 0)
{
    return QStringLiteral("<c r=\"%1\" s=\"%3\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, s.toHtmlEscaped()).arg(style);
}
QString boolean(const QString &ref, bool v) { return QStringLiteral("<c r=\"%1\" t=\"b\"><v>%2</v></c>").arg(ref).arg(v ? 1 : 0); }
QString err(const QString &ref, const QString &e) { return QStringLiteral("<c r=\"%1\" t=\"e\"><v>%2</v></c>").arg(ref, e); }
QString fnum(const QString &ref, const QString &f, double v, int style = 0)
{
    return QStringLiteral("<c r=\"%1\" s=\"%4\"><f>%2</f><v>%3</v></c>").arg(ref, f.toHtmlEscaped(), QString::number(v, 'g', 17)).arg(style);
}
QString fstr(const QString &ref, const QString &f, const QString &v) { return QStringLiteral("<c r=\"%1\" t=\"str\"><f>%2</f><v>%3</v></c>").arg(ref, f.toHtmlEscaped(), v.toHtmlEscaped()); }
QString row(int n, const QString &cells, const QString &attrs = QString()) { return QStringLiteral("<row r=\"%1\"%3>%2</row>").arg(n).arg(cells, attrs.isEmpty() ? QString() : QLatin1Char(' ') + attrs); }

bool openXlsx(const QByteArray &bytes, Workbook *wb, QString *why)
{
    Package p;
    if (!p.readLazy(bytes, why)) return false;
    return wb->open(p, why);
}

// The cell as the original and the copy are compared: a text is a text whichever way it is kept.
QString val(const Workbook &wb, const Sheet &sheet, const QString &ref)
{
    QString s = show(sheet, wb.sharedStrings(), ref);
    if (s.startsWith(QLatin1String("i:"))) s.replace(0, 1, QLatin1String("s"));
    return s;
}

QString formula(const Sheet &sheet, const QString &ref)
{
    const Cell *c = at(sheet, ref);
    return c && c->hasFormula() ? sheet.formulaText(*c) : QString();
}

struct Copies {
    Workbook original, xls, ods;
    CopyReport xlsReport, odsReport;
    QByteArray xlsBytes, odsBytes;
    QString why;
    bool make(const QByteArray &xlsx, bool wantOds = true)
    {
        if (!openXlsx(xlsx, &original, &why)) return false;
        if (!saveXls(original, &xlsBytes, &xlsReport, &why)) return false;
        // (every copy any test makes is checked for the structure the format wants, apart from what the tests look for in it)
        const QString broken = xlstest::validateXls(xlsBytes);
        if (!broken.isEmpty()) {
            why = QStringLiteral("the .xls copy is malformed: ") + broken;
            return false;
        }
        if (!xls.openBinary(xlsBytes, &why)) return false;
        if (!wantOds) return true;
        if (!saveOds(original, &odsBytes, &odsReport, &why)) return false;
        return openBytes(odsBytes, QStringLiteral("copy.ods"), &ods, &why);
    }
    Sheet sheet(const Workbook &wb, int i) const
    {
        Sheet s;
        wb.readSheet(i, &s);
        return s;
    }
};

QByteArray simple(const QString &rows, const QString &after = QString(), const QString &head = QString(), const QByteArray &styles = QByteArray(), const QString &names = QString())
{
    return buildXlsx({worksheet(rows, after, head)}, QByteArray(), QString(), QString(), styles, QStringList(), names);
}

// The records of a copy's Workbook stream as they lie in the file: the id and the bytes of each (a CONTINUE record is a record of its own here).
struct RawRecord {
    quint16 id;
    QByteArray data;
};

QList<RawRecord> rawRecords(const QByteArray &xls, bool *wellFormed = nullptr)
{
    QList<RawRecord> out;
    jp::cfb::File f;
    QString why;
    if (!jp::cfb::read(xls, &f, &why)) {
        if (wellFormed) *wellFormed = false;
        return out;
    }
    const QByteArray d = f.stream(QStringLiteral("Workbook"));
    qsizetype at = 0;
    bool ok = true;
    while (at + 4 <= d.size()) {
        const quint16 id = quint8(d[at]) | (quint8(d[at + 1]) << 8), len = quint8(d[at + 2]) | (quint8(d[at + 3]) << 8);
        if (id == 0 && len == 0 && d.mid(at).count('\0') == d.size() - at) {   // (the zeros that fill the stream out to its smallest size)
            at = d.size();
            break;
        }
        if (len > 8224 || at + 4 + len > d.size()) {
            ok = false;
            break;
        }
        out.append({id, d.mid(at + 4, len)});
        at += 4 + len;
    }
    if (at != d.size()) ok = false;
    if (wellFormed) *wellFormed = ok;
    return out;
}

QString manyOnes(int n)
{
    QStringList parts;
    for (int i = 0; i < n; ++i) parts << QStringLiteral("1");
    return QStringLiteral("{") + parts.join(QLatin1Char(',')) + QStringLiteral("}");
}

} // namespace

class SheetOldSaveTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // ---- .xls

    void anXlsCopyHoldsValuesOfEveryKind()
    {
        Copies c;
        const QByteArray x = simple(row(1, num(QStringLiteral("A1"), 1.5) + str(QStringLiteral("B1"), QStringLiteral("héllo wörld")) + boolean(QStringLiteral("C1"), true)
                                              + err(QStringLiteral("D1"), QStringLiteral("#DIV/0!")) + num(QStringLiteral("E1"), -0.0001234) + num(QStringLiteral("F1"), 1e300))
                                    + row(2, num(QStringLiteral("A2"), 100) + num(QStringLiteral("B2"), 70000) + num(QStringLiteral("C2"), 1234567.25) + str(QStringLiteral("D2"), QStringLiteral("日本語"))
                                                 + num(QStringLiteral("E2"), 0.1)));
        QVERIFY2(c.make(x), qPrintable(c.why));
        QVERIFY(c.xls.isBinary());
        const Sheet a = c.sheet(c.original, 0), b = c.sheet(c.xls, 0);
        for (const char *ref : {"A1", "B1", "C1", "D1", "E1", "F1", "A2", "B2", "C2", "D2", "E2", "F2"})
            QCOMPARE(val(c.xls, b, QString::fromLatin1(ref)), val(c.original, a, QString::fromLatin1(ref)));
        QCOMPARE(c.xls.info().sheets[0].name, QStringLiteral("Sheet1"));
    }

    void anXlsCopyHoldsFormulasAndTheirResults()
    {
        Copies c;
        const QStringList formulas = {QStringLiteral("A1+B1*2"), QStringLiteral("SUM(A1:B2,C3)"), QStringLiteral("IF(A1>1,\"yes\",\"no\")"), QStringLiteral("-A1^2"),
                                      QStringLiteral("A1&\"x\"&B1"), QStringLiteral("AVERAGE(A:A)"), QStringLiteral("SUM(1:2)"), QStringLiteral("ROUND(A1,2)"),
                                      QStringLiteral("COUNTIF(A1:A9,\">1\")"), QStringLiteral("PI()"), QStringLiteral("A1%"), QStringLiteral("(A1+B1)*C1"),
                                      QStringLiteral("SUM({1,2;3,4})"), QStringLiteral("IF(A1,,2)"), QStringLiteral("$A$1+B$2+$C3"), QStringLiteral("Sheet2!A1+1"),
                                      QStringLiteral("'My Sheet'!$B$2*2"), QStringLiteral("TRUE"), QStringLiteral("#N/A"), QStringLiteral("MyName*2"), QStringLiteral("LEN(\"say \"\"hi\"\"\")"),
                                      QStringLiteral("EOMONTH(A1,1)"), QStringLiteral("NETWORKDAYS(A1,A1+10)+WEEKNUM(A1)"), QStringLiteral("YEARFRAC(A1,A1+365,1)")};
        QString rows;
        for (int i = 0; i < formulas.size(); ++i) rows += row(i + 1, num(QStringLiteral("A%1").arg(i + 1), 1) + fnum(QStringLiteral("B%1").arg(i + 1), formulas[i], 7.5));
        const QByteArray x = buildXlsx({worksheet(rows), worksheet(row(1, num(QStringLiteral("A1"), 2))), worksheet(row(2, num(QStringLiteral("B2"), 3)))}, QByteArray(), QString(), QString(),
                                       QByteArray(), QStringList{QStringLiteral("Sheet1"), QStringLiteral("Sheet2"), QStringLiteral("My Sheet")},
                                       QStringLiteral("<definedName name=\"MyName\">Sheet1!$A$1</definedName>"));
        QVERIFY2(c.make(x, false), qPrintable(c.why));
        const Sheet a = c.sheet(c.original, 0), b = c.sheet(c.xls, 0);
        for (int i = 0; i < formulas.size(); ++i) {
            const QString ref = QStringLiteral("B%1").arg(i + 1);
            QCOMPARE(formula(b, ref), formula(a, ref));
            QCOMPARE(val(c.xls, b, ref), val(c.original, a, ref));
        }
        QCOMPARE(c.xlsReport.formulasAsValues, qint64(0));
        QCOMPARE(c.xlsReport.formulas, qint64(formulas.size()));
    }

    // A range that runs to the last row or column of the new form runs to the last of the old; a reference to a sheet that is gone is the error.
    void aRangeToTheLastRowOrColumnIsCutAtTheOldFormsLast()
    {
        Copies c;
        const QStringList in = {QStringLiteral("SUM(Sheet2!$A:$XFD)"), QStringLiteral("SUM(A1:XFD1048576)"), QStringLiteral("INDEX(Sheet2!$1:$1048576,2,3)"), QStringLiteral("COUNT(B2:B1048576)"),
                                QStringLiteral("Sheet2!#REF!+1"), QStringLiteral("SUM(Sheet2!#REF!:Sheet2!#REF!)")};
        const QStringList out = {QStringLiteral("SUM(Sheet2!$A:$IV)"), QStringLiteral("SUM(A:IV)"), QStringLiteral("INDEX(Sheet2!A:IV,2,3)"), QStringLiteral("COUNT(B2:B65536)"),
                                 QStringLiteral("#REF!+1"), QString()};
        QString rows;
        for (int i = 0; i < in.size(); ++i) rows += row(i + 1, num(QStringLiteral("A%1").arg(i + 1), 1) + fnum(QStringLiteral("B%1").arg(i + 1), in[i], 5));
        const QByteArray x = buildXlsx({worksheet(rows), worksheet(row(1, num(QStringLiteral("A1"), 2)))});
        QVERIFY2(c.make(x, false), qPrintable(c.why));
        const Sheet b = c.sheet(c.xls, 0);
        for (int i = 0; i < in.size() - 1; ++i) QCOMPARE(formula(b, QStringLiteral("B%1").arg(i + 1)), out[i]);
        QVERIFY(c.xlsReport.formulasAsValues <= 1);
    }

    void aFormulaTheOldFormCannotHoldKeepsItsValue()
    {
        Copies c;
        const QByteArray x = simple(row(1, num(QStringLiteral("A1"), 3) + fnum(QStringLiteral("B1"), QStringLiteral("_xlfn.CONCAT(A1,A1)"), 33) + fnum(QStringLiteral("C1"), QStringLiteral("A1+XFD1"), 3)
                                              + fstr(QStringLiteral("D1"), QStringLiteral("TEXTJOIN(\",\",TRUE,A1)"), QStringLiteral("3"))));
        QVERIFY2(c.make(x, false), qPrintable(c.why));
        const Sheet b = c.sheet(c.xls, 0);
        QCOMPARE(val(c.xls, b, QStringLiteral("B1")), QStringLiteral("n:33"));
        QCOMPARE(val(c.xls, b, QStringLiteral("C1")), QStringLiteral("n:3"));
        QCOMPARE(val(c.xls, b, QStringLiteral("D1")), QStringLiteral("s:3"));
        QVERIFY(!b.cells.find(0, 1)->hasFormula());
        QCOMPARE(c.xlsReport.formulasAsValues, qint64(3));
        QVERIFY(c.xlsReport.notKept.join(QLatin1Char(' ')).contains(QLatin1String("formula")));
    }

    // Functions added for the first release and 1.0 are saved with the prefix a file gives them (_xlfn.) and come back the same from the .xlsx, the .xls copy, and the .ods copy.
    void theNewerFunctionsAreSavedAndReadBackInEveryForm()
    {
        const QStringList formulas = {QStringLiteral("_xlfn.NORM.DIST(1,0,1,TRUE)"),     QStringLiteral("_xlfn.T.DIST.2T(1.5,10)"),       QStringLiteral("_xlfn.BINOM.DIST(3,10,0.5,FALSE)"),
                                      QStringLiteral("CONVERT(1,\"mi\",\"m\")"),          QStringLiteral("BESSELJ(1.9,2)"),                 QStringLiteral("IMSUM(\"3+4i\",\"5-3i\")"),
                                      QStringLiteral("ROMAN(499)"),                       QStringLiteral("_xlfn.CEILING.PRECISE(4.3)"),    QStringLiteral("_xlfn.BITAND(13,25)"),
                                      QStringLiteral("_xlfn.NUMBERVALUE(\"1.5\")"),        QStringLiteral("_xlfn.WORKDAY.INTL(40909,5,11)"), QStringLiteral("COUPNUM(40909,41640,2,1)"),
                                      QStringLiteral("DB(1000,100,6,1,7)"),               QStringLiteral("_xlfn.SEQUENCE(2)"),             QStringLiteral("_xlfn.XLOOKUP(1,A1:A2,A1:A2)"),
                                      QStringLiteral("_xlfn.SORT(A1:A2)"),                QStringLiteral("_xlfn.SHEET(A1)"),               QStringLiteral("SUMX2MY2(A1:A2,A1:A2)")};
        Copies c;
        QString rows;
        for (int i = 0; i < formulas.size(); ++i) rows += row(i + 1, num(QStringLiteral("A%1").arg(i + 1), 1) + fnum(QStringLiteral("B%1").arg(i + 1), formulas[i], 7.5));
        QVERIFY2(c.make(simple(rows), true), qPrintable(c.why));
        const Sheet a = c.sheet(c.original, 0), b = c.sheet(c.xls, 0), o = c.sheet(c.ods, 0);
        qint64 keptInXls = 0;
        for (int i = 0; i < formulas.size(); ++i) {
            const QString ref = QStringLiteral("B%1").arg(i + 1);
            QVERIFY2(formula(a, ref) == formulas[i], qPrintable(ref + QLatin1Char(' ') + formula(a, ref)));
            // (the .ods form writes TRUE as TRUE() and reads a newer function's prefix in capitals)
            const QString back = formula(o, ref).toUpper().replace(QLatin1String("TRUE()"), QLatin1String("TRUE")).replace(QLatin1String("FALSE()"), QLatin1String("FALSE"));
            QVERIFY2(back == formulas[i].toUpper(), qPrintable(QStringLiteral("ods %1: %2").arg(ref, formula(o, ref))));
            // the old form holds the functions it has a number for; the others keep the value they came to (and are counted)
            const Cell *cell = b.cells.find(quint32(i), 1);
            if (cell && cell->hasFormula()) {
                ++keptInXls;
                QVERIFY2(formula(b, ref) == formulas[i], qPrintable(QStringLiteral("xls %1: %2").arg(ref, formula(b, ref))));
            } else {
                QCOMPARE(val(c.xls, b, ref), QStringLiteral("n:7.5"));
            }
        }
        QVERIFY2(keptInXls >= 6, qPrintable(QString::number(keptInXls)));
        QCOMPARE(c.xlsReport.formulasAsValues, qint64(formulas.size()) - keptInXls);
    }

    void anXlsCopyHoldsArrayFormulas()
    {
        Copies c;
        const QByteArray x = simple(row(1, num(QStringLiteral("A1"), 1) + num(QStringLiteral("B1"), 2) + QStringLiteral("<c r=\"C1\"><f t=\"array\" ref=\"C1:C2\">A1:A2*B1:B2</f><v>2</v></c>"))
                                    + row(2, num(QStringLiteral("A2"), 3) + num(QStringLiteral("B2"), 4) + QStringLiteral("<c r=\"C2\"><v>12</v></c>")));
        QVERIFY2(c.make(x, false), qPrintable(c.why));
        const Sheet a = c.sheet(c.original, 0), b = c.sheet(c.xls, 0);
        QVERIFY(at(b, QStringLiteral("C1"))->flags & ArrayFormula);
        QCOMPARE(formula(b, QStringLiteral("C1")), formula(a, QStringLiteral("C1")));
        QCOMPARE(val(c.xls, b, QStringLiteral("C1")), QStringLiteral("n:2"));
        QCOMPARE(val(c.xls, b, QStringLiteral("C2")), QStringLiteral("n:12"));
    }

    void anXlsCopyHoldsFormats()
    {
        Copies c;
        const QByteArray styles = stylesWith(
            QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"0.000\"/><numFmt numFmtId=\"165\" formatCode=\"#,##0 &quot;units&quot;\"/>"),
            QStringLiteral("<font><sz val=\"11\"/><name val=\"Calibri\"/></font><font><b/><i/><sz val=\"14\"/><color rgb=\"FFFF0000\"/><name val=\"Courier New\"/></font>"),
            QStringLiteral("<fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill><fill><patternFill patternType=\"solid\"><fgColor rgb=\"FF3366CC\"/></patternFill></fill>"),
            QStringLiteral("<border><left/><right/><top/><bottom/><diagonal/></border><border><left style=\"thin\"><color rgb=\"FF000000\"/></left><right/><top/><bottom style=\"double\"><color rgb=\"FF00AA00\"/></bottom><diagonal/></border>"),
            QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/>"
                           "<xf numFmtId=\"164\" fontId=\"1\" fillId=\"2\" borderId=\"1\" applyNumberFormat=\"1\" applyFont=\"1\" applyFill=\"1\" applyBorder=\"1\"><alignment horizontal=\"center\" vertical=\"top\" wrapText=\"1\" indent=\"1\"/></xf>"
                           "<xf numFmtId=\"165\" fontId=\"0\" fillId=\"0\" borderId=\"0\" applyNumberFormat=\"1\"/><xf numFmtId=\"14\" fontId=\"0\" fillId=\"0\" borderId=\"0\" applyNumberFormat=\"1\"/>"
                           "<xf numFmtId=\"9\" fontId=\"0\" fillId=\"0\" borderId=\"0\" applyNumberFormat=\"1\"><protection locked=\"0\"/></xf>"
                           "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"><protection locked=\"1\" hidden=\"1\"/></xf>"));
        const QByteArray x = simple(row(1, num(QStringLiteral("A1"), 1.23456, 1) + num(QStringLiteral("B1"), 5, 2) + num(QStringLiteral("C1"), 45000, 3) + num(QStringLiteral("D1"), 0.25, 4)
                                              + num(QStringLiteral("E1"), 9, 0) + num(QStringLiteral("F1"), 3, 5)),
                                    QString(), QString(), styles);
        QVERIFY2(c.make(x), qPrintable(c.why));
        const Sheet a = c.sheet(c.original, 0);
        for (const Workbook *wb : {&c.xls, &c.ods}) {
            const Sheet b = c.sheet(*wb, 0);
            for (const char *r : {"A1", "B1", "C1", "D1", "E1", "F1"}) {
                const QString ref = QString::fromLatin1(r);
                const CellFormat &fa = c.original.styles().format(at(a, ref)->style), &fb = wb->styles().format(at(b, ref)->style);
                QCOMPARE(wb->styles().formatCode(at(b, ref)->style, QLocale::c()), c.original.styles().formatCode(at(a, ref)->style, QLocale::c()));
                const FontSpec &ga = c.original.styles().font(fa.font), &gb = wb->styles().font(fb.font);
                QCOMPARE(gb.bold, ga.bold);
                QCOMPARE(gb.italic, ga.italic);
                QCOMPARE(gb.size, ga.size);
                QCOMPARE(wb->styles().fontNameOf(gb), c.original.styles().fontNameOf(ga));
                QCOMPARE(wb->styles().resolve(gb.color, 0), c.original.styles().resolve(ga.color, 0));
                QCOMPARE(fb.horizontal, fa.horizontal);
                QCOMPARE(fb.vertical, fa.vertical);
                QCOMPARE(fb.wrap, fa.wrap);
                QCOMPARE(fb.locked, fa.locked);
                const FillSpec &ha = c.original.styles().fill(fa.fill), &hb = wb->styles().fill(fb.fill);
                QCOMPARE(hb.pattern, ha.pattern);
                if (ha.pattern != PatternKind::None) QCOMPARE(wb->styles().resolve(hb.fg, 0), c.original.styles().resolve(ha.fg, 0));
                const BorderSpec &ia = c.original.styles().border(fa.border), &ib = wb->styles().border(fb.border);
                QCOMPARE(ib.left.style, ia.left.style);
                QCOMPARE(ib.bottom.style, ia.bottom.style);
                if (ia.bottom.style != BorderStyle::None) QCOMPARE(wb->styles().resolve(ib.bottom.color, 0), c.original.styles().resolve(ia.bottom.color, 0));
            }
        }
    }

    // A format with a literal among its digits (a ZIP+4 code, a telephone number) keeps the literal in an .ods copy: the text is embedded in the number, at a position.
    void aFormatWithTextAmongItsDigitsKeepsTheText()
    {
        Copies c;
        const QStringList codes = {QStringLiteral("00000\\-0000"), QStringLiteral("#####\\-####"), QStringLiteral("(000) 000\\-0000"), QStringLiteral("000\\-00\\-0000")};
        QString fmts, xfs = QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/>"), cells;
        for (int i = 0; i < codes.size(); ++i) {
            fmts += QStringLiteral("<numFmt numFmtId=\"%1\" formatCode=\"%2\"/>").arg(164 + i).arg(QString(codes[i]).replace(QLatin1Char('"'), QLatin1String("&quot;")));
            xfs += QStringLiteral("<xf numFmtId=\"%1\" fontId=\"0\" fillId=\"0\" borderId=\"0\" applyNumberFormat=\"1\"/>").arg(164 + i);
            cells += num(QStringLiteral("A%1").arg(i + 1), 123456789, i + 1);
        }
        const QByteArray styles = stylesWith(fmts, QStringLiteral("<font><sz val=\"11\"/><name val=\"Calibri\"/></font>"), QStringLiteral("<fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>"),
                                             QStringLiteral("<border><left/><right/><top/><bottom/><diagonal/></border>"), xfs);
        QString rows;
        for (int i = 0; i < codes.size(); ++i) rows += row(i + 1, num(QStringLiteral("A%1").arg(i + 1), 123456789, i + 1));
        QVERIFY2(c.make(simple(rows, QString(), QString(), styles)), qPrintable(c.why));
        const Sheet a = c.sheet(c.original, 0), b = c.sheet(c.ods, 0);
        for (int i = 0; i < codes.size(); ++i) {
            const QString ref = QStringLiteral("A%1").arg(i + 1);
            QString want = c.original.styles().formatCode(at(a, ref)->style, QLocale::c()), got = c.ods.styles().formatCode(at(b, ref)->style, QLocale::c());
            // (a run of # before the text stands for one place: the places left of the farthest text are not counted in the file)
            got.remove(QLatin1Char(0x5C)).remove(QLatin1Char(0x22));
            want.remove(QLatin1Char(0x5C)).remove(QLatin1Char(0x22));
            QCOMPARE(got.replace(QRegularExpression(QStringLiteral("#+")), QStringLiteral("#")), want.replace(QRegularExpression(QStringLiteral("#+")), QStringLiteral("#")));
        }
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(c.odsBytes, parts));
        QVERIFY2(QString::fromUtf8(parts.value(QStringLiteral("content.xml"))).contains(QLatin1String("<number:embedded-text number:position=\"4\">-</number:embedded-text>")),
                 qPrintable(QString::fromUtf8(parts.value(QStringLiteral("content.xml"))).left(3000)));
    }

    void anXlsCopyHoldsTheShapeOfTheSheet()
    {
        Copies c;
        const QString head = QStringLiteral("<sheetViews><sheetView workbookViewId=\"0\" zoomScale=\"150\"><pane xSplit=\"1\" ySplit=\"2\" topLeftCell=\"B3\" activePane=\"bottomRight\" state=\"frozen\"/></sheetView></sheetViews>"
                                            "<sheetFormatPr defaultRowHeight=\"15\"/><cols><col min=\"1\" max=\"2\" width=\"20.5\" customWidth=\"1\"/><col min=\"4\" max=\"4\" width=\"9\" hidden=\"1\" customWidth=\"1\"/></cols>");
        const QByteArray x = simple(row(1, num(QStringLiteral("A1"), 1) + num(QStringLiteral("C1"), 2), QStringLiteral("ht=\"30\" customHeight=\"1\""))
                                    + row(2, num(QStringLiteral("A2"), 3), QStringLiteral("hidden=\"1\"")) + row(4, num(QStringLiteral("A4"), 4)),
                                    QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"A4:C5\"/></mergeCells>"), head);
        QVERIFY2(c.make(x), qPrintable(c.why));
        const Sheet a = c.sheet(c.original, 0);
        for (const Workbook *wb : {&c.xls, &c.ods}) {
            const Sheet b = c.sheet(*wb, 0);
            QCOMPARE(b.merges.size(), qsizetype(1));
            QCOMPARE(rangeName(b.merges[0]), QStringLiteral("A4:C5"));
            QVERIFY(b.view.frozen);
            QCOMPARE(b.view.frozenCols, quint32(1));
            QCOMPARE(b.view.frozenRows, quint32(2));
            double w = -1;
            // (a width most columns have is the sheet's: the width of a column is its own, or the sheet's)
            for (const ColumnSpan &s : b.columns)
                if (s.first <= 0 && s.last >= 0) w = s.width;
            if (w < 0) w = b.format.defaultColWidth;
            QVERIFY2(std::abs(w - 20.5) < 0.3, qPrintable(QString::number(w)));
            bool hiddenD = false;
            for (const ColumnSpan &s : b.columns)
                if (s.first <= 3 && s.last >= 3 && s.hidden) hiddenD = true;
            QVERIFY(hiddenD);
            double h1 = -1;
            bool hidden2 = false;
            for (const RowSpec &s : b.rowSpecs) {
                if (s.row == 0) h1 = s.height;
                if (s.row == 1 && s.hidden) hidden2 = true;
            }
            QVERIFY2(std::abs(h1 - 30) < 0.2, qPrintable(QString::number(h1)));
            QVERIFY(hidden2);
            QCOMPARE(val(*wb, b, QStringLiteral("A4")), val(c.original, a, QStringLiteral("A4")));
        }
    }

    void anXlsCopyHoldsThePageSetup()
    {
        Copies c;
        const QByteArray x = simple(row(1, num(QStringLiteral("A1"), 1)),
                                    QStringLiteral("<printOptions horizontalCentered=\"1\" gridLines=\"1\"/><pageMargins left=\"0.5\" right=\"0.6\" top=\"0.8\" bottom=\"0.9\" header=\"0.2\" footer=\"0.25\"/>"
                                                   "<pageSetup paperSize=\"9\" scale=\"80\" orientation=\"landscape\" firstPageNumber=\"3\" useFirstPageNumber=\"1\" horizontalDpi=\"300\" verticalDpi=\"1200\"/>"
                                                   "<headerFooter differentFirst=\"1\"><oddHeader>&amp;LLeft&amp;CPage &amp;P&amp;RRight</oddHeader><oddFooter>&amp;F</oddFooter><firstHeader>&amp;CFirst</firstHeader></headerFooter>"
                                                   "<rowBreaks count=\"1\" manualBreakCount=\"1\"><brk id=\"10\" max=\"16383\" man=\"1\"/></rowBreaks>"));
        QVERIFY2(c.make(x, false), qPrintable(c.why));
        const PageSetup a = c.sheet(c.original, 0).page, b = c.sheet(c.xls, 0).page;
        QCOMPARE(b.paperSize, 9);
        QCOMPARE(b.scale, 80);
        QVERIFY(b.orientation == PageOrientation::Landscape);
        QCOMPARE(b.firstPageNumber, 3);
        QVERIFY(b.useFirstPageNumber);
        QCOMPARE(b.horizontalDpi, 300);   // the print quality
        QCOMPARE(b.verticalDpi, 1200);
        QVERIFY(b.centerHorizontally && b.gridLines && !b.centerVertically);
        QCOMPARE(b.margins.left, a.margins.left);
        QCOMPARE(b.margins.right, a.margins.right);
        QCOMPARE(b.margins.top, a.margins.top);
        QCOMPARE(b.margins.bottom, a.margins.bottom);
        QCOMPARE(b.margins.header, a.margins.header);
        QCOMPARE(b.margins.footer, a.margins.footer);
        QCOMPARE(b.headerFooter.oddHeader, QStringLiteral("&LLeft&CPage &P&RRight"));
        QCOMPARE(b.headerFooter.oddFooter, QStringLiteral("&F"));
        QCOMPARE(b.headerFooter.firstHeader, QStringLiteral("&CFirst"));
        QVERIFY(b.headerFooter.differentFirst);
        QCOMPARE(b.rowBreaks.size(), qsizetype(1));
        QCOMPARE(b.rowBreaks[0].before, quint32(10));
        QVERIFY(!c.xlsReport.notKept.join(QLatin1Char('|')).contains(QLatin1String("page setup")));
    }

    void anXlsCopyHoldsSheetsAndNames()
    {
        Copies c;
        const QByteArray x = buildXlsx({worksheet(row(1, num(QStringLiteral("A1"), 1))), worksheet(row(1, str(QStringLiteral("A1"), QStringLiteral("two")))), worksheet(row(1, num(QStringLiteral("A1"), 3)))},
                                       QByteArray(), QStringLiteral("date1904=\"1\""), QString(), QByteArray(), QStringList{QStringLiteral("First"), QStringLiteral("Second sheet"), QStringLiteral("Hidden")},
                                       QStringLiteral("<definedName name=\"Total\">First!$A$1:$A$3</definedName><definedName name=\"Local\" localSheetId=\"1\">'Second sheet'!$A$1</definedName>"
                                                      "<definedName name=\"_xlnm.Print_Area\" localSheetId=\"0\">First!$A$1:$C$9</definedName><definedName name=\"_xlnm._FilterDatabase\" localSheetId=\"0\" hidden=\"1\">First!$A$1:$C$9</definedName>"),
                                       QByteArray(), QString(), QList<int>{2});
        QVERIFY2(c.make(x), qPrintable(c.why));
        for (const Workbook *wb : {&c.xls, &c.ods}) {
            QCOMPARE(wb->info().sheets.size(), 3);
            QCOMPARE(wb->info().sheets[1].name, QStringLiteral("Second sheet"));
            QVERIFY(wb->info().sheets[2].state == SheetState::Hidden);
            QCOMPARE(val(*wb, c.sheet(*wb, 1), QStringLiteral("A1")), QStringLiteral("s:two"));
            QSet<QString> names;
            for (const DefinedName &d : wb->info().definedNames) names.insert(d.name + QLatin1Char('=') + QString(d.ref).remove(QLatin1Char('\'')) + QLatin1Char('@') + QString::number(d.localSheet));
            QVERIFY2(names.contains(QStringLiteral("Total=First!$A$1:$A$3@-1")), qPrintable(QStringList(names.values()).join(QLatin1Char(' '))));
            QVERIFY2(names.contains(QStringLiteral("Local=Second sheet!$A$1@1")), qPrintable(QStringList(names.values()).join(QLatin1Char(' '))));
        }
        // (the names the form numbers keep their numbers: the print area and the filter's range come back as themselves)
        QSet<QString> old;
        for (const DefinedName &d : c.xls.info().definedNames) old.insert(d.name);
        QVERIFY2(old.contains(QStringLiteral("_xlnm.Print_Area")) && old.contains(QStringLiteral("_xlnm._FilterDatabase")), qPrintable(QStringList(old.values()).join(QLatin1Char(' '))));
        QVERIFY(c.xls.info().dateSystem == jp::calc::DateSystem::Date1904);
        QVERIFY(c.ods.info().dateSystem == jp::calc::DateSystem::Date1904);
    }

    void aLongTextIsWrittenWholeAcrossContinuedRecords()
    {
        Copies c;
        QString rows;
        QStringList texts;
        for (int i = 0; i < 40; ++i) {
            QString t = QString(3000 + i * 111, QLatin1Char('a' + i % 26));
            if (i % 3 == 0) t += QString(5, QChar(0x4E2D));
            texts << t;
            rows += row(i + 1, str(QStringLiteral("A%1").arg(i + 1), t));
        }
        QVERIFY2(c.make(simple(rows), false), qPrintable(c.why));
        const Sheet b = c.sheet(c.xls, 0);
        for (int i = 0; i < texts.size(); ++i) QCOMPARE(val(c.xls, b, QStringLiteral("A%1").arg(i + 1)), QStringLiteral("s:") + texts[i]);
    }

    // ---- hostile models

    void aCellPastTheOldFormsGridIsCutAndNamed()
    {
        Copies c;
        const QByteArray x = simple(row(1, num(QStringLiteral("A1"), 1) + num(QStringLiteral("IW1"), 2)) + row(70000, num(QStringLiteral("A70000"), 3)) + row(2, num(QStringLiteral("IV2"), 4)));
        QVERIFY2(c.make(x, false), qPrintable(c.why));
        const Sheet b = c.sheet(c.xls, 0);
        QCOMPARE(val(c.xls, b, QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(val(c.xls, b, QStringLiteral("IV2")), QStringLiteral("n:4"));
        QCOMPARE(b.cells.cellCount(), qint64(2));
        QVERIFY(!c.xlsReport.trimmed.isEmpty());
        QVERIFY2(c.xlsReport.trimmed.join(QLatin1Char('|')).contains(QLatin1String("2 cells")), qPrintable(c.xlsReport.trimmed.join(QLatin1Char('|'))));
    }

    void aTextLongerThanTheFormatAllowsIsCutAndNamed()
    {
        Copies c;
        const QByteArray x = simple(row(1, str(QStringLiteral("A1"), QString(40000, QLatin1Char('x'))) + str(QStringLiteral("B1"), QStringLiteral("ok"))));
        QVERIFY2(c.make(x), qPrintable(c.why));
        const Sheet b = c.sheet(c.xls, 0), d = c.sheet(c.ods, 0);
        QCOMPARE(b.text(*at(b, QStringLiteral("A1")), c.xls.sharedStrings()).size(), 32767);
        QCOMPARE(d.text(*at(d, QStringLiteral("A1")), c.ods.sharedStrings()).size(), 32767);
        QCOMPARE(val(c.xls, b, QStringLiteral("B1")), QStringLiteral("s:ok"));
        QVERIFY(!c.xlsReport.trimmed.isEmpty());
        QVERIFY(!c.odsReport.trimmed.isEmpty());
    }

    void moreSheetsThanTheOldFormHoldsAreCutAndNamed()
    {
        Copies c;
        QVector<QByteArray> sheets;
        QStringList names;
        for (int i = 0; i < 300; ++i) {
            sheets << worksheet(row(1, num(QStringLiteral("A1"), i)));
            names << QStringLiteral("S%1").arg(i);
        }
        QVERIFY2(c.make(buildXlsx(sheets, QByteArray(), QString(), QString(), QByteArray(), names)), qPrintable(c.why));
        QCOMPARE(c.xls.info().sheets.size(), 255);
        QCOMPARE(c.ods.info().sheets.size(), 300);
        QVERIFY(!c.xlsReport.trimmed.isEmpty());
        QCOMPARE(val(c.xls, c.sheet(c.xls, 254), QStringLiteral("A1")), QStringLiteral("n:254"));
    }

    void aNumberOfFormatsPastTheLimitFallsBackAndIsNamed()
    {
        Copies c;
        QString fmts, xfs = QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/>");
        for (int i = 0; i < 300; ++i) {
            fmts += QStringLiteral("<numFmt numFmtId=\"%1\" formatCode=\"0.%2\"/>").arg(164 + i).arg(QString(1 + i % 7, QLatin1Char('0')) + QString::number(i) + QStringLiteral("\\x"));
            xfs += QStringLiteral("<xf numFmtId=\"%1\" fontId=\"0\" fillId=\"0\" borderId=\"0\" applyNumberFormat=\"1\"/>").arg(164 + i);
        }
        const QByteArray styles = stylesWith(fmts, QStringLiteral("<font><sz val=\"11\"/><name val=\"Calibri\"/></font>"),
                                             QStringLiteral("<fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>"),
                                             QStringLiteral("<border><left/><right/><top/><bottom/><diagonal/></border>"), xfs);
        QString r1, r2;
        for (int i = 0; i < 300; ++i) (i < 200 ? r1 : r2) += num(QStringLiteral("%1%2").arg(columnName(quint32(i % 200))).arg(1 + i / 200), i + 0.5, i + 1);
        QVERIFY2(c.make(simple(row(1, r1) + row(2, r2), QString(), QString(), styles)), qPrintable(c.why));
        const Sheet b = c.sheet(c.xls, 0);
        QCOMPARE(val(c.xls, b, QStringLiteral("A1")), QStringLiteral("n:0.5"));
        int custom = 0;
        for (auto it = c.xls.styles().numFmts.cbegin(); it != c.xls.styles().numFmts.cend(); ++it) ++custom;
        QVERIFY2(custom <= kMaxXlsNumberFormats, qPrintable(QString::number(custom)));
        QVERIFY(!c.xlsReport.trimmed.isEmpty());
        // (the .ods copy has them all)
        QVERIFY(c.sheet(c.ods, 0).cells.cellCount() == 300);
    }

    void moreFontsAndCellFormatsThanTheOldFormHoldsAreCutAndNamed()
    {
        Copies c;
        QString fonts = QStringLiteral("<font><sz val=\"11\"/><name val=\"Calibri\"/></font>");
        for (int i = 0; i < 700; ++i) fonts += QStringLiteral("<font><sz val=\"%1\"/><name val=\"Font %2\"/></font>").arg(8 + i % 40 + 0.25 * (i / 40)).arg(i);
        QString xfs = QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/>");
        for (int i = 0; i < 700; ++i)
            for (int indent = 0; indent < 15; ++indent) xfs += QStringLiteral("<xf numFmtId=\"0\" fontId=\"%1\" fillId=\"0\" borderId=\"0\" applyFont=\"1\" applyAlignment=\"1\"><alignment horizontal=\"left\" indent=\"%2\"/></xf>").arg(i + 1).arg(indent + 1);
        const QByteArray styles = stylesWith(QString(), fonts, QStringLiteral("<fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>"),
                                             QStringLiteral("<border><left/><right/><top/><bottom/><diagonal/></border>"), xfs);
        QString rows;
        for (int r = 0; r < 70; ++r) {
            QString cells;
            for (int k = 0; k < 200; ++k) {
                const int n = r * 200 + k;   // (700 fonts, each with 15 indents, one after another)
                cells += num(QStringLiteral("%1%2").arg(columnName(quint32(k))).arg(r + 1), k, 1 + (n * 11) % 10500);
            }
            rows += row(r + 1, cells);
        }
        QVERIFY2(c.make(simple(rows, QString(), QString(), styles), false), qPrintable(c.why));
        const Sheet b = c.sheet(c.xls, 0);
        QCOMPARE(b.cells.cellCount(), qint64(14000));
        QVERIFY2(c.xls.styles().formats.size() <= kMaxXlsFormats + 1, qPrintable(QString::number(c.xls.styles().formats.size())));
        QVERIFY2(c.xls.styles().fonts.size() <= kMaxXlsFonts + 2, qPrintable(QString::number(c.xls.styles().fonts.size())));
        const QString said = c.xlsReport.trimmed.join(QLatin1Char('|'));
        QVERIFY2(said.contains(QLatin1String("cell formats past")) && said.contains(QLatin1String("fonts past")), qPrintable(said));
        // (the cells keep their values whatever look they were cut to)
        QCOMPARE(val(c.xls, b, QStringLiteral("GR70")), QStringLiteral("n:199"));
    }

    // ---- .ods

    void anOdsCopyHoldsValuesAndFormulas()
    {
        Copies c;
        const QByteArray two = buildXlsx({worksheet(row(1, num(QStringLiteral("A1"), 1.5) + str(QStringLiteral("B1"), QStringLiteral("a <b> & \"c\"\nline two  spaced")) + boolean(QStringLiteral("C1"), false)
                                                         + err(QStringLiteral("D1"), QStringLiteral("#N/A")) + fnum(QStringLiteral("E1"), QStringLiteral("SUM(A1:A2)*2"), 7)
                                                         + fstr(QStringLiteral("F1"), QStringLiteral("B1&\"!\""), QStringLiteral("a!")) + fnum(QStringLiteral("G1"), QStringLiteral("IF(A1>1,\"y\",\"n\")"), 0)
                                                         + fnum(QStringLiteral("H1"), QStringLiteral("Sheet2!A1+1"), 4))
                                                     + row(2, num(QStringLiteral("A2"), 2) + fnum(QStringLiteral("B2"), QStringLiteral("MyName+A2"), 3))),
                                          worksheet(row(1, num(QStringLiteral("A1"), 3)))},
                                         QByteArray(), QString(), QString(), QByteArray(), QStringList(), QStringLiteral("<definedName name=\"MyName\">Sheet1!$A$1</definedName>"));
        QVERIFY2(c.make(two), qPrintable(c.why));
        QVERIFY(c.ods.isConverted());
        const Sheet a = c.sheet(c.original, 0), b = c.sheet(c.ods, 0);
        for (const char *r : {"A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1", "A2", "B2"}) {
            const QString ref = QString::fromLatin1(r);
            QCOMPARE(val(c.ods, b, ref), val(c.original, a, ref));
            QCOMPARE(formula(b, ref), formula(a, ref));
        }
    }

    void anOdsCopyHoldsArrayFormulas()
    {
        Copies c;
        const QByteArray x = simple(row(1, num(QStringLiteral("A1"), 1) + num(QStringLiteral("B1"), 2) + QStringLiteral("<c r=\"C1\"><f t=\"array\" ref=\"C1:C2\">A1:A2*B1:B2</f><v>2</v></c>"))
                                    + row(2, num(QStringLiteral("A2"), 3) + num(QStringLiteral("B2"), 4) + QStringLiteral("<c r=\"C2\"><v>12</v></c>")));
        QVERIFY2(c.make(x), qPrintable(c.why));
        const Sheet b = c.sheet(c.ods, 0);
        QVERIFY(at(b, QStringLiteral("C1"))->flags & ArrayFormula);
        QCOMPARE(val(c.ods, b, QStringLiteral("C2")), QStringLiteral("n:12"));
    }

    // The page setup and links are in the .ods in the form the other program reads: the page layout and master page, the breaks before rows, and a link around the text of its cell.
    void anOdsCopyHoldsThePageSetupAndLinks()
    {
        Copies c;
        const QByteArray x = simple(row(1, str(QStringLiteral("A1"), QStringLiteral("link here")) + num(QStringLiteral("B1"), 2)) + row(12, num(QStringLiteral("A12"), 3)),
                                    QStringLiteral("<hyperlinks><hyperlink ref=\"A1\" location=\"Sheet1!B1\" display=\"link here\"/></hyperlinks><printOptions gridLines=\"1\"/>"
                                                   "<pageMargins left=\"0.5\" right=\"0.6\" top=\"0.8\" bottom=\"0.9\" header=\"0.2\" footer=\"0.25\"/><pageSetup paperSize=\"9\" scale=\"80\" orientation=\"landscape\"/>"
                                                   "<headerFooter><oddHeader>&amp;LLeft&amp;CPage &amp;P&amp;RRight</oddHeader><oddFooter>&amp;A</oddFooter></headerFooter><rowBreaks count=\"1\" manualBreakCount=\"1\"><brk id=\"10\" max=\"16383\" man=\"1\"/></rowBreaks>"));
        QVERIFY2(c.make(x), qPrintable(c.why));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(c.odsBytes, parts));
        const QString styles = QString::fromUtf8(parts.value(QStringLiteral("styles.xml"))), content = QString::fromUtf8(parts.value(QStringLiteral("content.xml")));
        QVERIFY2(styles.contains(QLatin1String("style:print-orientation=\"landscape\"")) && styles.contains(QLatin1String("style:scale-to=\"80%\"")), qPrintable(styles.left(3000)));
        QVERIFY(styles.contains(QLatin1String("<style:region-left><text:p>Left</text:p></style:region-left>")));
        QVERIFY(styles.contains(QLatin1String("<text:page-number>")) && styles.contains(QLatin1String("<text:sheet-name/>")));
        QVERIFY(styles.contains(QLatin1String("style:print=\"charts drawings objects zero-values grid\"")));
        QVERIFY(content.contains(QLatin1String("fo:break-before=\"page\"")));
        QVERIFY2(content.contains(QLatin1String("<text:a xlink:href=\"#Sheet1.B1\" xlink:type=\"simple\">link here</text:a>")), qPrintable(content.mid(content.indexOf(QLatin1String("<table:table "))).left(1500)));
        QVERIFY(c.odsReport.notKept.join(QLatin1Char('|')).indexOf(QLatin1String("page setup")) < 0);
    }

    // A cell with no look of its own in a column that has one is plain, not the column's: the document says so for it.
    void aPlainCellInAStyledColumnStaysPlainInAnOdsCopy()
    {
        Copies c;
        const QByteArray styles = stylesWith(QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"0.000\"/>"), QStringLiteral("<font><sz val=\"11\"/><name val=\"Calibri\"/></font>"),
                                             QStringLiteral("<fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>"),
                                             QStringLiteral("<border><left/><right/><top/><bottom/><diagonal/></border>"),
                                             QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/><xf numFmtId=\"164\" fontId=\"0\" fillId=\"0\" borderId=\"0\" applyNumberFormat=\"1\"/>"));
        const QByteArray x = simple(row(1, num(QStringLiteral("A1"), 5, 0) + num(QStringLiteral("B1"), 6, 1)) + row(2, num(QStringLiteral("A2"), 7, 1)), QString(),
                                    QStringLiteral("<cols><col min=\"1\" max=\"1\" width=\"12\" style=\"1\" customWidth=\"1\"/></cols>"), styles);
        QVERIFY2(c.make(x), qPrintable(c.why));
        const Sheet b = c.sheet(c.ods, 0);
        QCOMPARE(c.ods.styles().formatCode(at(b, QStringLiteral("A1"))->style, QLocale::c()), QStringLiteral("General"));
        QCOMPARE(c.ods.styles().formatCode(at(b, QStringLiteral("A2"))->style, QLocale::c()), QStringLiteral("0.000"));
    }

    // A format of four sections keeps its fourth (the section for texts) even when it is empty.
    void aFormatWithAnEmptyTextSectionComesBackWithIt()
    {
        Copies c;
        const QByteArray styles = stylesWith(QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"0_);[Red]\\(0\\);;\"/>"), QStringLiteral("<font><sz val=\"11\"/><name val=\"Calibri\"/></font>"),
                                             QStringLiteral("<fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>"),
                                             QStringLiteral("<border><left/><right/><top/><bottom/><diagonal/></border>"),
                                             QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/><xf numFmtId=\"164\" fontId=\"0\" fillId=\"0\" borderId=\"0\" applyNumberFormat=\"1\"/>"));
        QVERIFY2(c.make(simple(row(1, num(QStringLiteral("A1"), 5, 1)), QString(), QString(), styles)), qPrintable(c.why));
        const Sheet b = c.sheet(c.ods, 0);
        const QString code = c.ods.styles().formatCode(at(b, QStringLiteral("A1"))->style, QLocale::c());
        QCOMPARE(code.count(QLatin1Char(';')), 3);
    }

    // Gaps between the rows and between the cells are written as repeats, not as the empty rows and cells they are, and read back to the same places.
    void anOdsCopyWritesGapsAsRepeatsAndReadsThemBack()
    {
        Copies c;
        QString rows;
        for (int i = 1; i <= 60; ++i) rows += row(i, num(QStringLiteral("A%1").arg(i), i));
        rows += row(5000, num(QStringLiteral("A5000"), 5000) + num(QStringLiteral("E5000"), 1) + num(QStringLiteral("XFD5000"), 2));
        rows += row(900000, str(QStringLiteral("C900000"), QStringLiteral("far")));
        QVERIFY2(c.make(simple(rows)), qPrintable(c.why));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(c.odsBytes, parts));
        const QString content = QString::fromUtf8(parts.value(QStringLiteral("content.xml")));
        QVERIFY2(content.contains(QLatin1String("table:number-rows-repeated=\"4939\"")), "the gap of rows is not a repeat");
        QVERIFY2(content.contains(QLatin1String("table:number-columns-repeated=\"16379\"")) || content.contains(QLatin1String("table:number-columns-repeated=\"3\"")), "the gaps of cells are not repeats");
        QVERIFY(content.size() < 100000);
        const Sheet b = c.sheet(c.ods, 0);
        QCOMPARE(b.cells.cellCount(), qint64(60 + 3 + 1));
        QCOMPARE(val(c.ods, b, QStringLiteral("A60")), QStringLiteral("n:60"));
        QCOMPARE(val(c.ods, b, QStringLiteral("XFD5000")), QStringLiteral("n:2"));
        QCOMPARE(val(c.ods, b, QStringLiteral("C900000")), QStringLiteral("s:far"));
    }

    // The most sheets the old form holds (255) are all written, and none is named as cut.
    void exactlyTheMostSheetsTheOldFormHoldsAreAllWritten()
    {
        Copies c;
        QVector<QByteArray> sheets;
        QStringList names;
        for (int i = 0; i < 255; ++i) {
            sheets << worksheet(row(1, num(QStringLiteral("A1"), i)));
            names << QStringLiteral("S%1").arg(i);
        }
        QVERIFY2(c.make(buildXlsx(sheets, QByteArray(), QString(), QString(), QByteArray(), names)), qPrintable(c.why));
        QCOMPARE(c.xls.info().sheets.size(), 255);
        QCOMPARE(val(c.xls, c.sheet(c.xls, 254), QStringLiteral("A1")), QStringLiteral("n:254"));
        QVERIFY2(c.xlsReport.trimmed.isEmpty(), qPrintable(c.xlsReport.trimmed.join(QLatin1Char('|'))));
    }

    // A workbook stream past about 6.8 MB needs the extra table of sector numbers in the container (the DIFAT): the copy of a sheet of some 420,000 numbers has it, and reads back.
    void aVeryLargeXlsCopyUsesTheContainersExtraSectorTable()
    {
        Copies c;
        QString rows;
        for (int r = 1; r <= 1700; ++r) {
            QString cells;
            for (int col = 0; col < 250; ++col) cells += num(columnName(quint32(col)) + QString::number(r), r * 0.37 + col * 1.0001);
            rows += row(r, cells);
        }
        QVERIFY2(c.make(simple(rows), false), qPrintable(c.why));
        QVERIFY2(c.xlsBytes.size() > 7200000, qPrintable(QString::number(c.xlsBytes.size())));
        // (the header of the container: the first sector of the extra table at 68, and how many sectors it has at 72)
        const quint32 difatSectors = quint8(c.xlsBytes[72]) | (quint8(c.xlsBytes[73]) << 8) | (quint8(c.xlsBytes[74]) << 16) | (quint32(quint8(c.xlsBytes[75])) << 24);
        QVERIFY2(difatSectors > 0, "the container has no extra sector table");
        const Sheet a = c.sheet(c.original, 0), b = c.sheet(c.xls, 0);
        QCOMPARE(b.cells.cellCount(), a.cells.cellCount());
        for (const char *ref : {"A1", "IA1700", "HZ850", "B1699"}) QCOMPARE(val(c.xls, b, QString::fromLatin1(ref)), val(c.original, a, QString::fromLatin1(ref)));
    }

    // ---- what the copy cannot hold

    void whatACopyLacksIsNamed()
    {
        Workbook wb;
        QString why;
        const QByteArray x = simple(row(1, num(QStringLiteral("A1"), 1)),
                                    QStringLiteral("<conditionalFormatting sqref=\"A1\"><cfRule type=\"cellIs\" dxfId=\"0\" priority=\"1\" operator=\"greaterThan\"><formula>0</formula></cfRule></conditionalFormatting>"
                                                   "<dataValidations count=\"1\"><dataValidation type=\"whole\" sqref=\"A1\"><formula1>1</formula1></dataValidation></dataValidations>"));
        QVERIFY2(openXlsx(x, &wb, &why), qPrintable(why));
        const QString xlsList = featuresNotInCopy(wb, CopyKind::Xls).join(QLatin1Char('|'));
        QVERIFY2(xlsList.contains(QLatin1String("conditional")), qPrintable(xlsList));
        QVERIFY2(xlsList.contains(QLatin1String("validation")), qPrintable(xlsList));
    }

    void aCopyIsNotMadeOfAWorkbookWithNoSheet()
    {
        Copies c;
        QVERIFY2(c.make(simple(QString())), qPrintable(c.why));
        QCOMPARE(c.xls.info().sheets.size(), 1);
        QCOMPARE(c.ods.info().sheets.size(), 1);
    }

    // ---- review fixes

    // A record is at most 8,224 bytes: a formula whose tokens do not fit is written as its value and named, never cut short.
    void aFormulaTooLongForARecordIsWrittenAsItsValueNotCut()
    {
        Copies c;
        const QByteArray x = simple(row(1, num(QStringLiteral("A1"), 2) + fnum(QStringLiteral("B1"), QStringLiteral("SUMPRODUCT(") + manyOnes(1000) + QLatin1Char(')'), 1000)
                                              + fnum(QStringLiteral("C1"), QStringLiteral("SUMPRODUCT(") + manyOnes(100) + QLatin1Char(')'), 100) + fnum(QStringLiteral("D1"), QStringLiteral("A1*3"), 6)));
        QVERIFY2(c.make(x, false), qPrintable(c.why));
        bool ok = false;
        rawRecords(c.xlsBytes, &ok);
        QVERIFY2(ok, "a record is longer than the format allows or the stream does not end where a record ends");
        const Sheet b = c.sheet(c.xls, 0);
        QCOMPARE(val(c.xls, b, QStringLiteral("B1")), QStringLiteral("n:1000"));
        QVERIFY(!b.cells.find(0, 1)->hasFormula());
        QVERIFY(b.cells.find(0, 2)->hasFormula());
        QVERIFY(b.cells.find(0, 3)->hasFormula());
        QCOMPARE(c.xlsReport.formulasAsValues, qint64(1));
        QVERIFY2(c.xlsReport.notKept.join(QLatin1Char(' ')).contains(QLatin1String("formula")), qPrintable(c.xlsReport.notKept.join(QLatin1Char('|'))));
    }

    // A name of 255 characters whose formula does not fit stands for #REF!, and is named in the report.
    void aNameWhoseRecordWouldBeTooLongStandsForRefAndIsNamed()
    {
        Copies c;
        const QString longName = QString(254, QChar(0x0151)) + QLatin1Char('x');   // 255 characters, all but one of them wide
        const QString names = QStringLiteral("<definedName name=\"%1\">SUMPRODUCT(%2)</definedName><definedName name=\"Fine\">Sheet1!$A$1</definedName>").arg(longName, manyOnes(870));
        QVERIFY2(c.make(simple(row(1, num(QStringLiteral("A1"), 1)), QString(), QString(), QByteArray(), names), false), qPrintable(c.why));
        bool ok = false;
        rawRecords(c.xlsBytes, &ok);
        QVERIFY2(ok, "a record is longer than the format allows");
        QString refOfLong, refOfFine;
        for (const DefinedName &d : c.xls.info().definedNames) {
            if (d.name == longName) refOfLong = d.ref;
            if (d.name == QLatin1String("Fine")) refOfFine = d.ref;
        }
        QCOMPARE(refOfLong, QStringLiteral("#REF!"));
        QCOMPARE(refOfFine, QStringLiteral("Sheet1!$A$1"));
        QVERIFY2(c.xlsReport.trimmed.join(QLatin1Char(' ')).contains(QLatin1String("name")), qPrintable(c.xlsReport.trimmed.join(QLatin1Char('|'))));
    }

    // The numbers the format gives built-in names, tested against the bytes: 6 the print area, 7 the print titles, 8 the macro recorder, 0x0C the sheet title, 0x0D the filter's range.
    void theBuiltInNamesCarryTheNumbersTheFormatGivesThem()
    {
        Copies c;
        const QByteArray x = simple(row(1, num(QStringLiteral("A1"), 1)), QString(), QString(), QByteArray(),
                                    QStringLiteral("<definedName name=\"_xlnm.Print_Area\" localSheetId=\"0\">Sheet1!$A$1:$C$9</definedName>"
                                                   "<definedName name=\"_xlnm.Print_Titles\" localSheetId=\"0\">Sheet1!$1:$2</definedName>"
                                                   "<definedName name=\"_xlnm.Recorder\" localSheetId=\"0\">Sheet1!$A$1</definedName>"
                                                   "<definedName name=\"_xlnm.Sheet_Title\" localSheetId=\"0\">Sheet1!$A$1</definedName>"
                                                   "<definedName name=\"_xlnm._FilterDatabase\" localSheetId=\"0\" hidden=\"1\">Sheet1!$A$1:$C$9</definedName>"));
        QVERIFY2(c.make(x, false), qPrintable(c.why));
        QList<int> codes;
        for (const RawRecord &r : rawRecords(c.xlsBytes))
            if (r.id == 0x0018 && (quint8(r.data[0]) & 0x20)) codes << quint8(r.data[15]);   // (the name's one character, after the 14 bytes of the head and the wide flag)
        std::sort(codes.begin(), codes.end());
        QCOMPARE(codes, (QList<int>{6, 7, 8, 0x0C, 0x0D}));
        QSet<QString> back;
        for (const DefinedName &d : c.xls.info().definedNames) back.insert(d.name);
        QVERIFY2(back.contains(QStringLiteral("_xlnm.Recorder")) && back.contains(QStringLiteral("_xlnm.Sheet_Title")) && back.contains(QStringLiteral("_xlnm._FilterDatabase")),
                 qPrintable(QStringList(back.values()).join(QLatin1Char(' '))));
    }

    // The words of the report: no "2nd", which reads wrong in other languages and for 255, and a sheet name the format does not cut for OpenDocument.
    void theReportOfCutSheetsHasNoOrdinalAndOdsKeepsLongSheetNames()
    {
        Copies c;
        QVector<QByteArray> sheets;
        QStringList names;
        for (int i = 0; i < 260; ++i) {
            sheets << worksheet(row(1, num(QStringLiteral("A1"), i)));
            names << QStringLiteral("S%1").arg(i);
        }
        names[0] = QStringLiteral("A sheet whose name is longer than thirty-one characters");
        QVERIFY2(c.make(buildXlsx(sheets, QByteArray(), QString(), QString(), QByteArray(), names)), qPrintable(c.why));
        const QString said = c.xlsReport.trimmed.join(QLatin1Char('|'));
        QVERIFY2(!said.contains(QLatin1String("nd or later")) && !said.contains(QLatin1String("2nd")), qPrintable(said));
        QVERIFY2(said.contains(QLatin1String("255")), qPrintable(said));
        QCOMPARE(c.xls.info().sheets[0].name, names[0].left(31));
        // (the copy holds the name whole; this program's reader shortens it to what the .xlsx form allows when it reads the file)
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(c.odsBytes, parts));
        QVERIFY(QString::fromUtf8(parts.value(QStringLiteral("content.xml"))).contains(QLatin1String("table:name=\"") + names[0] + QLatin1String("\"")));
    }

    // A long chain of the same operator is as wide as it is long, not as deep: 600 terms are written as a formula, and the result reads back as the same text.
    void aLongChainOfTermsKeepsItsFormula()
    {
        Copies c;
        QStringList terms, products;
        for (int i = 0; i < 600; ++i) terms << QStringLiteral("A1");
        for (int i = 0; i < 300; ++i) products << QStringLiteral("A1*2");
        const QString sum = terms.join(QLatin1Char('+')), mixed = products.join(QLatin1Char('-')), joined = QStringLiteral("\"a\"&") + QStringList(300, QStringLiteral("\"b\"")).join(QLatin1Char('&'));
        QVERIFY2(c.make(simple(row(1, num(QStringLiteral("A1"), 1) + fnum(QStringLiteral("B1"), sum, 600) + fnum(QStringLiteral("C1"), mixed, -300) + fstr(QStringLiteral("D1"), joined, QStringLiteral("a") + QString(300, QLatin1Char('b'))))), false), qPrintable(c.why));
        const Sheet a = c.sheet(c.original, 0), b = c.sheet(c.xls, 0);
        for (const char *ref : {"B1", "C1", "D1"}) {
            QVERIFY2(b.cells.find(0, ref[0] - 'A')->hasFormula(), ref);
            QCOMPARE(formula(b, QString::fromLatin1(ref)), formula(a, QString::fromLatin1(ref)));
        }
        QCOMPARE(c.xlsReport.formulasAsValues, qint64(0));
    }

    // The tokens of IF and CHOOSE as the format has them (the jump records after the condition and after each choice), and the class of a reference by what the function asks for:
    // a value for ABS, a reference for SUM, the condition of IF a value and its choices references. Bytes checked against the formulas of real files.
    void theTokensOfIfChooseAndTheClassesOfReferencesAreTheFormats()
    {
        XlsCompileContext cx;
        cx.sheetNames = {QStringLiteral("Sheet1")};
        cx.xti = [](int, int) { return 0; };
        auto hex = [&](const QString &f) {
            QByteArray rgce, extra;
            QString why;
            if (!xlsCompileFormula(f, cx, &rgce, &extra, &why)) return QStringLiteral("refused: ") + why;
            return QString::fromLatin1(rgce.toHex(' '));
        };
        QCOMPARE(hex(QStringLiteral("ABS(A1)")), QStringLiteral("44 00 00 00 c0 41 18 00"));
        QCOMPARE(hex(QStringLiteral("SUM(A1)")), QStringLiteral("24 00 00 00 c0 42 01 04 00"));
        QCOMPARE(hex(QStringLiteral("IF(A1>1,\"yes\",\"no\")")), QStringLiteral("44 00 00 00 c0 1e 01 00 0d 19 02 0a 00 17 03 00 79 65 73 19 08 0c 00 17 02 00 6e 6f 19 08 03 00 42 03 01 00"));
        QCOMPARE(hex(QStringLiteral("IF(A1,B1,C1)")), QStringLiteral("44 00 00 00 c0 19 02 09 00 24 00 00 01 c0 19 08 0c 00 24 00 00 02 c0 19 08 03 00 42 03 01 00"));
        QCOMPARE(hex(QStringLiteral("IF(A1,B1)")), QStringLiteral("44 00 00 00 c0 19 02 09 00 24 00 00 01 c0 19 08 03 00 42 02 01 00"));
        QCOMPARE(hex(QStringLiteral("CHOOSE(A1,\"a\",\"b\")")),
                 QStringLiteral("44 00 00 00 c0 19 04 02 00 06 00 0e 00 16 00 17 01 00 61 19 08 0b 00 17 01 00 62 19 08 03 00 42 03 64 00"));
    }

    // A save can be stopped in the middle of a sheet, not only between rows: a sheet of one very wide row, and one of a few hundred rows, are stopped after a few thousand cells.
    void aSaveIsStoppedInTheMiddleOfASheet()
    {
        auto colName = [](int i) {
            QString s;
            for (int n = i + 1; n > 0; n = (n - 1) / 26) s.prepend(QChar('A' + (n - 1) % 26));
            return s;
        };
        QString wide, tall;
        for (int c = 0; c < 20000; ++c) wide += num(colName(c) + QStringLiteral("1"), c);
        for (int r = 1; r <= 300; ++r) {
            QString cells;
            for (int c = 0; c < 256; ++c) cells += num(colName(c) + QString::number(r), r + c);
            tall += row(r, cells);
        }
        Workbook wb;
        QString why;
        QVERIFY2(openXlsx(simple(row(1, wide)), &wb, &why), qPrintable(why));
        Workbook wb2;
        QVERIFY2(openXlsx(simple(tall), &wb2, &why), qPrintable(why));
        // (stop at the sixth look: before, the looks were at the start, at each sheet, and at the first row, and so never got there)
        for (int which = 0; which < 2; ++which) {
            int looks = 0;
            QByteArray out;
            CopyReport report;
            const auto stop = [&] { return ++looks >= 6; };
            const bool ok = which == 0 ? saveOds(wb, &out, &report, &why, stop) : saveXls(wb2, &out, &report, &why, stop);
            QVERIFY2(!ok, which == 0 ? "the .ods save was not stopped" : "the .xls save was not stopped");
            QVERIFY(looks >= 6);
        }
    }

    // A small workbook's stream is padded to 4,096 bytes, so that it lies in whole sectors of the container and not in its small-stream area, where some programs do not look.
    void aSmallWorkbookStreamIsPaddedToTheSmallestRegularSize()
    {
        Copies c;
        QVERIFY2(c.make(simple(row(1, num(QStringLiteral("A1"), 1))), false), qPrintable(c.why));
        jp::cfb::File f;
        QString why;
        QVERIFY2(jp::cfb::read(c.xlsBytes, &f, &why), qPrintable(why));
        QVERIFY2(f.stream(QStringLiteral("Workbook")).size() >= 4096, qPrintable(QString::number(f.stream(QStringLiteral("Workbook")).size())));
        QCOMPARE(val(c.xls, c.sheet(c.xls, 0), QStringLiteral("A1")), QStringLiteral("n:1"));
    }

    // Finding the cells a merged range covers is a lookup, not a scan of all the ranges for each cell: 2,000 merged ranges side by side cost a few steps for each of 8,000 cells.
    void theCellsAMergedRangeCoversAreFoundByLookup()
    {
        Copies c;
        QString r1, r2, merges;
        for (int i = 0; i < 2000; ++i) {
            const QString a = columnName(quint32(2 * i)), b = columnName(quint32(2 * i + 1));
            r1 += num(a + QStringLiteral("1"), i) + num(b + QStringLiteral("1"), -i);
            r2 += num(a + QStringLiteral("2"), i) + num(b + QStringLiteral("2"), -i);
            merges += QStringLiteral("<mergeCell ref=\"%1:%2\"/>").arg(a + QStringLiteral("1"), b + QStringLiteral("2"));
        }
        const qint64 before = odsCoveredWorkForTest(), lookupsBefore = odsCoveredLookupsForTest();
        QVERIFY2(c.make(simple(row(1, r1) + row(2, r2), QStringLiteral("<mergeCells count=\"2000\">") + merges + QStringLiteral("</mergeCells>"))), qPrintable(c.why));
        const qint64 work = odsCoveredWorkForTest() - before;
        const qint64 lookups = odsCoveredLookupsForTest() - lookupsBefore;
        QVERIFY2(lookups >= 8000, qPrintable(QString::number(lookups)));   // (each of the 8,000 cells was asked about)
        QVERIFY2(work <= lookups * 12, qPrintable(QStringLiteral("%1 looks for %2 questions").arg(work).arg(lookups)));   // (2,000 stretches are halved to one in 11 looks; a scan would be a thousand)
        const Sheet b = c.sheet(c.ods, 0);
        QCOMPARE(b.merges.size(), 2000);
        QCOMPARE(val(c.ods, b, QStringLiteral("A1")), QStringLiteral("n:0"));
        QCOMPARE(val(c.ods, b, columnName(3998) + QStringLiteral("1")), QStringLiteral("n:1999"));   // (the first cell of the last range)
    }

    // The arrays an .ods copy writes are bounded in all as the .xls copy's are (2,000,000 cells), and one too large, or past the total, is written as its values and named.
    void arraysPastTheTotalAreWrittenAsValuesInAnOdsCopy()
    {
        Copies c;
        auto array = [](const QString &ref, const QString &first, double v) {
            return QStringLiteral("<c r=\"%1\"><f t=\"array\" ref=\"%2\">1+1</f><v>%3</v></c>").arg(first, ref).arg(v);
        };
        const QByteArray x = simple(row(1, array(QStringLiteral("A1:A1500000"), QStringLiteral("A1"), 2) + array(QStringLiteral("B1:B900000"), QStringLiteral("B1"), 2) + array(QStringLiteral("C1:C900000"), QStringLiteral("C1"), 2)
                                              + array(QStringLiteral("D1:D900000"), QStringLiteral("D1"), 2) + num(QStringLiteral("E1"), 1)));
        QVERIFY2(c.make(x), qPrintable(c.why));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(c.odsBytes, parts));
        const QString content = QString::fromUtf8(parts.value(QStringLiteral("content.xml")));
        QCOMPARE(content.count(QLatin1String("table:number-matrix-rows-spanned")), 2);   // (B and C, which fit; A is past the size one may have, and D past the total)
        QCOMPARE(c.odsReport.formulasAsValues, qint64(2));
    }

    // The cap on cells is applied as the cells are taken from the sheet, not after all of them are copied: a sheet of 1,000 cells with a cap of 100 gives the first 100 in order and counts the rest.
    void theCellsOfASheetAreCopiedOnlyUpToTheCap()
    {
        QString rows;
        for (int r = 1; r <= 100; ++r) {
            QString cells;
            for (int col = 0; col < 10; ++col) cells += num(columnName(quint32(col)) + QString::number(r), r * 100 + col);
            rows += row(r, cells);
        }
        Workbook wb;
        QString why;
        QVERIFY2(openXlsx(simple(rows), &wb, &why), qPrintable(why));
        Sheet sheet;
        wb.readSheet(0, &sheet);
        qint64 left = -1;
        const std::vector<PlacedCell> few = oldsave::sortedCells(sheet, 100, &left);
        QCOMPARE(qint64(few.size()), qint64(100));
        QCOMPARE(left, qint64(900));
        QCOMPARE(few.front().row, quint32(0));
        QCOMPARE(few.back().row, quint32(9));
        QCOMPARE(few.back().col, quint32(9));
        const std::vector<PlacedCell> all = oldsave::sortedCells(sheet, 5000, &left);
        QCOMPARE(qint64(all.size()), qint64(1000));
        QCOMPARE(left, qint64(0));
    }

    // The panes of a frozen sheet: a PANE of ten bytes, and a selection for each pane the split makes (one for a split of rows or of columns, two; for both, four). The copies of two workbooks
    // with frozen panes were refused as damaged by the program the format is named for, for a PANE of nine bytes.
    void aFrozenSheetHasTenBytePaneAndASelectionForEachPane()
    {
        const QString rows = row(1, num(QStringLiteral("A1"), 1) + num(QStringLiteral("B1"), 2)) + row(5, num(QStringLiteral("A5"), 3));
        const QStringList views = {QStringLiteral("<pane ySplit=\"3\" topLeftCell=\"A4\" activePane=\"bottomLeft\" state=\"frozen\"/>"),
                                   QStringLiteral("<pane xSplit=\"1\" topLeftCell=\"B1\" activePane=\"topRight\" state=\"frozen\"/>"),
                                   QStringLiteral("<pane xSplit=\"1\" ySplit=\"3\" topLeftCell=\"B4\" activePane=\"bottomRight\" state=\"frozen\"/>")};
        const int selections[] = {2, 2, 4};
        for (int i = 0; i < 3; ++i) {
            Copies c;
            QVERIFY2(c.make(simple(rows, QString(), QStringLiteral("<sheetViews><sheetView workbookViewId=\"0\">") + views[i] + QStringLiteral("</sheetView></sheetViews>")), false), qPrintable(c.why));
            int paneLength = 0, selectionCount = 0;
            for (const RawRecord &r : rawRecords(c.xlsBytes)) {
                if (r.id == 0x0041) paneLength = int(r.data.size());
                if (r.id == 0x001D) ++selectionCount;
            }
            QCOMPARE(paneLength, 10);
            QCOMPARE(selectionCount, selections[i]);
        }
    }

    // The structure check itself: a PANE of nine bytes, a selection missing, and a name whose tokens are an error value are what it names.
    void theStructureCheckNamesWhatIsWrong()
    {
        Copies c;
        QVERIFY2(c.make(simple(row(1, num(QStringLiteral("A1"), 1))), false), qPrintable(c.why));
        QVERIFY(xlstest::validateXls(c.xlsBytes).isEmpty());
        QString why;
        QVERIFY(!xlstest::validateTokens(QByteArray::fromHex("1E01001E0200"), -1, -1, &why));   // two values left over
        QVERIFY(!xlstest::validateTokens(QByteArray::fromHex("39000001000000450000020000c000c04202ff00"), -1, -1, &why));   // an add-in call given a range of the value class
        QVERIFY(xlstest::validateTokens(QByteArray::fromHex("39000001000000250000020000c000c04202ff00"), -1, -1, &why));
        QVERIFY(xlstest::validateTokens(QByteArray::fromHex("1E01001E020003"), -1, -1, &why));
        QVERIFY(!xlstest::validateTokens(QByteArray::fromHex("3B050000000000000000"), 2, -1, &why));   // a reference through an external sheet that is not in the list, and cut off
    }

    // Over a list of workbooks (the path of a file of names in XLS_VALIDATE_LIST, one to a line; skipped without it, and no file of the list is in the repository): the .xls copy of each is
    // checked for its structure. The count of failures is the bar: none.
    void theCopiesOfAListOfWorkbooksAreWellFormed()
    {
        const QByteArray list = qgetenv("XLS_VALIDATE_LIST");
        if (list.isEmpty()) QSKIP("XLS_VALIDATE_LIST names no list");
        QFile f(QString::fromLocal8Bit(list));
        QVERIFY(f.open(QIODevice::ReadOnly));
        qint64 checked = 0, bad = 0;
        QStringList firsts;
        const int step = std::max(1, qEnvironmentVariableIntValue("XLS_VALIDATE_STEP"));
        int n = 0;
        while (!f.atEnd()) {
            const QString path = QString::fromLocal8Bit(f.readLine()).trimmed();
            if (path.isEmpty() || (n++ % step) != 0) continue;
            Workbook wb;
            QString why;
            if (!wb.openFile(path, &why)) continue;
            QByteArray out;
            CopyReport report;
            if (!saveXls(wb, &out, &report, &why)) continue;
            ++checked;
            const QString broken = xlstest::validateXls(out);
            if (!broken.isEmpty()) {
                ++bad;
                if (firsts.size() < 10) firsts << path + QStringLiteral(": ") + broken;
            }
        }
        qInfo("%lld copies checked, %lld malformed", checked, bad);
        QVERIFY2(bad == 0, qPrintable(firsts.join(QLatin1Char('\n'))));
    }

    // The tokens of formulas as a real program wrote them in real .xls files (the formula's text, and the bytes of the file's own tokens, with the marks for spaces left out): the class of every
    // reference by what the function asks for (a value for a text function nested in another, a reference for a sum, an array in SUMPRODUCT and the operators inside it), the volatile mark, and the
    // jumps of a nested IF. A reference in the wrong class is worked out differently by that program: every formula of a sheet that took a text function's argument as a reference gave #VALUE!.
    void theTokensOfRealFormulasAreWrittenByteForByte()
    {
        XlsCompileContext cx;
        cx.sheetNames = {QStringLiteral("Sheet1")};
        cx.xti = [](int, int) { return 0; };
        struct Case {
            const char *text;
            const char *bytes;
        };
        const Case cases[] = {
            {"NORMSINV(I3)", "44 02 00 08 c0 41 28 01"},
            {"CONCATENATE(F166,\" \",F167)", "44 a5 00 05 c0 17 01 00 20 44 a6 00 05 c0 42 03 50 01"},
            {"CEILING(SUMPRODUCT(B138:B142*C41:C45),100)", "65 89 00 8d 00 01 c0 01 c0 65 28 00 2c 00 02 c0 02 c0 05 42 01 e4 00 1e 64 00 41 20 01"},
            {"NOW()", "19 01 00 00 41 4a 00"},
            {"HYPERLINK(CONCATENATE($H$4,F6),G6)", "44 03 00 07 00 44 05 00 05 c0 42 02 50 01 44 05 00 06 c0 42 02 67 01"},
            {"IF(C9>C27*$A$31,$A$33,IF(C9<C27*$A$32,$A$34,$A$35))",
             "44 08 00 02 c0 44 1a 00 02 c0 44 1e 00 00 00 05 0d 19 02 09 00 24 20 00 00 00 19 08 32 00 44 08 00 02 c0 44 1a 00 02 c0 44 1f 00 00 00 05 09 19 02 09 00 24 21 00 00 00 19 08 0c 00 "
             "24 22 00 00 00 19 08 03 00 22 03 01 00 19 08 03 00 42 03 01 00"},
            {"IF(INDIRECT(B2&\"!\"&CHAR(64+D2)&C2)>0,FALSE,TRUE)",
             "19 01 00 00 44 01 00 01 c0 17 01 00 21 08 1e 40 00 44 01 00 03 c0 03 41 6f 00 08 44 01 00 02 c0 08 42 01 94 00 1e 00 00 0d 19 02 06 00 1d 00 19 08 09 00 1d 01 19 08 03 00 42 03 01 00"},
        };
        for (const Case &c : cases) {
            QByteArray rgce, extra;
            QString why;
            QVERIFY2(xlsCompileFormula(QString::fromLatin1(c.text), cx, &rgce, &extra, &why), c.text);
            QCOMPARE(QString::fromLatin1(rgce.toHex(' ')), QString::fromLatin1(c.bytes));
        }
    }

    // A name or formula for a reference that is gone is the token for it (a reference through the list of external sheets in a name), not an error value: the program the format is named for
    // gave #VALUE! for a name whose tokens were an error value, and a copy with such a name was refused as damaged.
    void aReferenceThatIsGoneIsNotAnErrorValueInANameOrAFormula()
    {
        XlsCompileContext cx;
        cx.sheetNames = {QStringLiteral("Sheet1")};
        cx.xti = [](int, int) { return 3; };
        QByteArray rgce, extra;
        QString why;
        QVERIFY(xlsCompileFormula(QStringLiteral("#REF!"), cx, &rgce, &extra, &why));
        QCOMPARE(QString::fromLatin1(rgce.toHex(' ')), QStringLiteral("2a 00 00 00 00"));
        cx.isName = true;
        QVERIFY(xlsCompileFormula(QStringLiteral("#REF!"), cx, &rgce, &extra, &why));
        QCOMPARE(QString::fromLatin1(rgce.toHex(' ')), QStringLiteral("3c 03 00 00 00 00 00"));
        Copies c;
        QVERIFY2(c.make(simple(row(1, num(QStringLiteral("A1"), 1)), QString(), QString(), QByteArray(), QStringLiteral("<definedName name=\"Gone\">#REF!</definedName>")), false), qPrintable(c.why));
    }

    // Sheets whose names are the same in their first 31 characters (all the old form keeps): the names written are different (a number in brackets ends the later ones, as the shortened names
    // go), the order is kept, and a formula that names each sheet reads the sheet it named.
    void sheetsWithTheSameFirstCharactersKeepDistinctNamesAndTheirFormulas()
    {
        const QString stem = QString(31, QLatin1Char('s'));
        const QStringList names = {stem + QStringLiteral("A"), stem + QStringLiteral("B"), stem + QStringLiteral("C"), QStringLiteral("Other"), stem.toUpper() + QStringLiteral("D")};
        QVector<QByteArray> sheets;
        for (int i = 0; i < names.size(); ++i) {
            QString rows = row(1, num(QStringLiteral("A1"), 100 + i));
            if (i == 3) rows = row(1, fnum(QStringLiteral("A1"), QStringLiteral("'%1'!A1+'%2'!A1*10+'%3'!A1*100").arg(names[0], names[1], names[2]), 100 + 10 * 101 + 100 * 102) + fnum(QStringLiteral("B1"), QStringLiteral("'%1'!A1").arg(names[4]), 104));
            sheets << worksheet(rows);
        }
        Copies c;
        QVERIFY2(c.make(buildXlsx(sheets, QByteArray(), QString(), QString(), QByteArray(), names)), qPrintable(c.why));
        QStringList written;
        QSet<QString> lower;
        for (const SheetEntry &e : c.xls.info().sheets) {
            written << e.name;
            QVERIFY2(!lower.contains(e.name.toLower()), qPrintable(written.join(QLatin1Char('|'))));
            lower.insert(e.name.toLower());
            QVERIFY(e.name.size() <= 31);
        }
        QCOMPARE(written.size(), 5);
        QCOMPARE(written[3], QStringLiteral("Other"));
        const Sheet other = c.sheet(c.xls, 3);
        QCOMPARE(val(c.xls, other, QStringLiteral("A1")), val(c.original, c.sheet(c.original, 3), QStringLiteral("A1")));
        QCOMPARE(val(c.xls, other, QStringLiteral("B1")), QStringLiteral("n:104"));
        // the formulas name the sheets as written, and each reads its own
        const QString f = formula(other, QStringLiteral("A1"));
        QVERIFY2(f.contains(written[0]) && f.contains(written[1]) && f.contains(written[2]), qPrintable(f + QStringLiteral(" / ") + written.join(QLatin1Char('|'))));
        QCOMPARE(formula(other, QStringLiteral("B1")).contains(written[4]), true);
        // and the .ods copy keeps the names whole
        QCOMPARE(c.ods.info().sheets.size(), 5);
    }

    // The general rule for a record of more than 8,224 bytes: it goes on in CONTINUE records, in pieces of 8,224 at most, and the bytes put together are the bytes given.
    void aRecordOfMoreThanTheMostIsContinuedNotCut()
    {
        for (const int size : {0, 1, 8224, 8225, 16448, 16449, 20000, 50000}) {
            QByteArray data(size, 0);
            for (int i = 0; i < size; ++i) data[i] = char(i * 7 + i / 251);
            QByteArray out;
            xlsPutRecord(&out, 0x0018, data);
            QByteArray joined;
            int records = 0, at = 0;
            while (at < out.size()) {
                const int id = quint8(out[at]) | (quint8(out[at + 1]) << 8), len = quint8(out[at + 2]) | (quint8(out[at + 3]) << 8);
                QCOMPARE(id, records == 0 ? 0x0018 : 0x003C);
                QVERIFY(len <= 8224);
                if (records > 0) QVERIFY(len > 0);
                joined += out.mid(at + 4, len);
                at += 4 + len;
                ++records;
            }
            QCOMPARE(joined, data);
            QCOMPARE(records, std::max(1, (size + 8223) / 8224));
        }
    }

    // A cell with no color of its own has none in the copy, in both forms: not the color the reader would make of nothing, which is opaque white (and would draw the text white).
    void aCellWithNoFontColorHasNoneInTheCopies()
    {
        Copies c;
        QVERIFY2(c.make(simple(row(1, str(QStringLiteral("A1"), QStringLiteral("plain")) + num(QStringLiteral("B1"), 1)))), qPrintable(c.why));
        const Sheet a = c.sheet(c.original, 0);
        for (const Workbook *wb : {&c.original, &c.xls, &c.ods}) {
            const Sheet b = c.sheet(*wb, 0);
            for (const char *ref : {"A1", "B1"}) {
                const Cell *cell = at(b, QString::fromLatin1(ref));
                QVERIFY(cell);
                const FontSpec &font = wb->styles().font(wb->styles().format(cell->style).font);
                QVERIFY2(!font.color.isSet() || wb->styles().resolve(font.color, 0) != 0xFFFFFFFFu, qPrintable(QStringLiteral("the font color of %1 is %2").arg(QString::fromLatin1(ref)).arg(wb->styles().resolve(font.color, 0), 0, 16)));
                QCOMPARE(wb->styles().resolve(font.color, 0), c.original.styles().resolve(c.original.styles().font(c.original.styles().format(at(a, QString::fromLatin1(ref))->style).font).color, 0));
            }
        }
    }

    // The arguments of a function of the add-in set (XNPV, EDATE, and the rest) are written as references, as the real files write them (0x25 for a range, 0x24 for a cell): written as values, a
    // range gave #VALUE! in the program the format is named for, and every cell that depended on one, a data table's among them.
    void theArgumentsOfAnAddInFunctionAreReferences()
    {
        XlsCompileContext cx;
        cx.sheetNames = {QStringLiteral("Sheet1")};
        cx.xti = [](int, int) { return 0; };
        cx.addIn = [](const QString &, int *ixti, int *index) {
            *ixti = 0;
            *index = 1;
            return true;
        };
        QByteArray rgce, extra;
        QString why;
        QVERIFY(xlsCompileFormula(QStringLiteral("XNPV(0.1,A1:A3,B1:B3)"), cx, &rgce, &extra, &why));
        QCOMPARE(QString::fromLatin1(rgce.toHex(' ')), QStringLiteral("39 00 00 01 00 00 00 1f 9a 99 99 99 99 99 b9 3f 25 00 00 02 00 00 c0 00 c0 25 00 00 02 00 01 c0 01 c0 42 04 ff 00"));
        QVERIFY(xlsCompileFormula(QStringLiteral("EDATE(C3,2)"), cx, &rgce, &extra, &why));
        QCOMPARE(QString::fromLatin1(rgce.toHex(' ')), QStringLiteral("39 00 00 01 00 00 00 24 02 00 02 c0 1e 02 00 42 03 ff 00"));
    }

    // A formula that uses a name whose definition the old form cannot hold (a cell of another workbook, an error, a definition too long for a record) is written as its value: written as a
    // formula it would be worked out again by the program that opens the copy, to the error the name stands for (every cell that depends on it, a data table's among them, went wrong).
    void aFormulaThatUsesANameTheFormCannotHoldKeepsItsValue()
    {
        Copies c;
        const QString names = QStringLiteral("<definedName name=\"Outside\">[1]Other!$A$1</definedName><definedName name=\"Inside\">Sheet1!$A$1</definedName><definedName name=\"ViaOutside\">Outside*2</definedName>");
        const QByteArray x = simple(row(1, num(QStringLiteral("A1"), 5) + fnum(QStringLiteral("B1"), QStringLiteral("Outside*2"), 10) + fnum(QStringLiteral("C1"), QStringLiteral("Inside*2"), 10)
                                              + fnum(QStringLiteral("D1"), QStringLiteral("ViaOutside+1"), 11) + fnum(QStringLiteral("E1"), QStringLiteral("B1+C1"), 20)),
                                    QString(), QString(), QByteArray(), names);
        QVERIFY2(c.make(x, false), qPrintable(c.why));
        const Sheet b = c.sheet(c.xls, 0);
        QVERIFY(!b.cells.find(0, 1)->hasFormula());   // Outside*2
        QVERIFY(b.cells.find(0, 2)->hasFormula());    // Inside*2
        QVERIFY(!b.cells.find(0, 3)->hasFormula());   // ViaOutside+1
        QVERIFY(b.cells.find(0, 4)->hasFormula());    // B1+C1
        QCOMPARE(val(c.xls, b, QStringLiteral("B1")), QStringLiteral("n:10"));
        QCOMPARE(val(c.xls, b, QStringLiteral("D1")), QStringLiteral("n:11"));
    }

    void theFormulaCompilerRefusesWhatItCannotWrite()
    {
        XlsCompileContext cx;
        cx.sheetNames = {QStringLiteral("Sheet1")};
        cx.xti = [](int, int) { return 0; };
        QByteArray rgce, extra;
        QString why;
        QVERIFY(xlsCompileFormula(QStringLiteral("SUM(A1:B2)"), cx, &rgce, &extra, &why));
        QVERIFY(!xlsCompileFormula(QStringLiteral("SUM(A1:B2"), cx, &rgce, &extra, &why));
        QVERIFY(!xlsCompileFormula(QStringLiteral("NOSUCHFUNCTION(1)"), cx, &rgce, &extra, &why));
        QVERIFY(!xlsCompileFormula(QStringLiteral("A1+B65537"), cx, &rgce, &extra, &why));
        QVERIFY(!xlsCompileFormula(QStringLiteral("[1]Sheet1!A1"), cx, &rgce, &extra, &why));
        QVERIFY(!xlsCompileFormula(QString(QStringLiteral("(")).repeated(5000) + QStringLiteral("1") + QString(QStringLiteral(")")).repeated(5000), cx, &rgce, &extra, &why));
        QVERIFY(!xlsCompileFormula(QStringLiteral("1+") .repeated(6000) + QStringLiteral("1"), cx, &rgce, &extra, &why));
        QVERIFY(!xlsCompileFormula(QStringLiteral("LEN(\"") + QString(300, QLatin1Char('x')) + QStringLiteral("\")"), cx, &rgce, &extra, &why));
    }
};

JP_TEST_CLASS(SheetOldSaveTests)
#include "tst_sheet_oldsave.moc"
