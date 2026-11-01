// Tests of the copies of a workbook in the old binary form (.xls) and in OpenDocument (.ods): a workbook of the XML form is built in memory, written as the copy, and read back
// by this program's own readers; what the copy holds is what the original held (values, formulas, formats, sizes, merged cells, names, panes), and what a copy cannot hold or a
// damaged model would make too large is cut and named. Nothing here reads a file made by another program.
#include "odshelpers.h"

#include "io/cfb.h"
#include "sheet/oldsave.h"
#include "sheet/xlscompile.h"

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
                           "<xf numFmtId=\"9\" fontId=\"0\" fillId=\"0\" borderId=\"0\" applyNumberFormat=\"1\"><protection locked=\"0\"/></xf>"));
        const QByteArray x = simple(row(1, num(QStringLiteral("A1"), 1.23456, 1) + num(QStringLiteral("B1"), 5, 2) + num(QStringLiteral("C1"), 45000, 3) + num(QStringLiteral("D1"), 0.25, 4)
                                              + num(QStringLiteral("E1"), 9, 0)),
                                    QString(), QString(), styles);
        QVERIFY2(c.make(x), qPrintable(c.why));
        const Sheet a = c.sheet(c.original, 0);
        for (const Workbook *wb : {&c.xls, &c.ods}) {
            const Sheet b = c.sheet(*wb, 0);
            for (const char *r : {"A1", "B1", "C1", "D1", "E1"}) {
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
                                                   "<pageSetup paperSize=\"9\" scale=\"80\" orientation=\"landscape\" firstPageNumber=\"3\" useFirstPageNumber=\"1\"/>"
                                                   "<headerFooter differentFirst=\"1\"><oddHeader>&amp;LLeft&amp;CPage &amp;P&amp;RRight</oddHeader><oddFooter>&amp;F</oddFooter><firstHeader>&amp;CFirst</firstHeader></headerFooter>"
                                                   "<rowBreaks count=\"1\" manualBreakCount=\"1\"><brk id=\"10\" max=\"16383\" man=\"1\"/></rowBreaks>"));
        QVERIFY2(c.make(x, false), qPrintable(c.why));
        const PageSetup &a = c.sheet(c.original, 0).page, b = c.sheet(c.xls, 0).page;
        QCOMPARE(b.paperSize, 9);
        QCOMPARE(b.scale, 80);
        QVERIFY(b.orientation == PageOrientation::Landscape);
        QCOMPARE(b.firstPageNumber, 3);
        QVERIFY(b.useFirstPageNumber);
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
        QVERIFY(c.ods.info().dateSystem == jp::calc::DateSystem::Date1904 || c.ods.info().dateSystem == jp::calc::DateSystem::Date1900);
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

    void anOdsCopyReadsBackInManyRows()
    {
        Copies c;
        QString rows;
        for (int i = 1; i <= 300; ++i) rows += row(i, num(QStringLiteral("A%1").arg(i), i) + (i % 50 == 0 ? str(QStringLiteral("C%1").arg(i), QStringLiteral("x")) : QString()));
        QVERIFY2(c.make(simple(rows)), qPrintable(c.why));
        const Sheet b = c.sheet(c.ods, 0);
        QCOMPARE(b.cells.cellCount(), qint64(306));
        QCOMPARE(val(c.ods, b, QStringLiteral("A300")), QStringLiteral("n:300"));
        QCOMPARE(val(c.ods, b, QStringLiteral("C250")), QStringLiteral("s:x"));
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
