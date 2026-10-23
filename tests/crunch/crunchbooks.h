// The workbooks JeffCrunch's tests open, built here in memory: a small one that looks like a real one (a budget, with a frozen header, merged cells, fills, borders, number formats, a
// hidden sheet, and formulas whose stored results are wrong on purpose), and a large one made of numbers and text.
#pragma once

#include "../sheet/sheethelpers.h"

#include <QFile>
#include <QTemporaryDir>

namespace crunchtest {

inline QByteArray budgetStyles()
{
    return stylesXml(
        QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"&quot;$&quot;#,##0.00\"/><numFmt numFmtId=\"165\" formatCode=\"0.0%\"/><numFmt numFmtId=\"166\" formatCode=\"mmm d, yyyy\"/>"),
        QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/>"
                       "<xf numFmtId=\"0\" fontId=\"1\" fillId=\"2\" borderId=\"0\"><alignment horizontal=\"center\" vertical=\"center\"/></xf>"
                       "<xf numFmtId=\"0\" fontId=\"2\" fillId=\"3\" borderId=\"1\"><alignment horizontal=\"center\"/></xf>"
                       "<xf numFmtId=\"164\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/>"
                       "<xf numFmtId=\"165\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/>"
                       "<xf numFmtId=\"166\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/>"
                       "<xf numFmtId=\"164\" fontId=\"2\" fillId=\"3\" borderId=\"2\"/>"
                       "<xf numFmtId=\"0\" fontId=\"3\" fillId=\"4\" borderId=\"0\"><alignment wrapText=\"1\" vertical=\"top\"/></xf>"
                       "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"><alignment horizontal=\"right\"/></xf>"),
        QStringLiteral("<fonts count=\"4\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font>"
                       "<font><b/><sz val=\"14\"/><color rgb=\"FFFFFFFF\"/><name val=\"Calibri\"/></font>"
                       "<font><b/><sz val=\"11\"/><name val=\"Calibri\"/></font>"
                       "<font><i/><sz val=\"10\"/><color rgb=\"FF808080\"/><name val=\"Calibri\"/></font></fonts>"
                       "<fills count=\"5\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>"
                       "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FF13773A\"/></patternFill></fill>"
                       "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FFE2F0E6\"/></patternFill></fill>"
                       "<fill><patternFill patternType=\"lightGrid\"><fgColor rgb=\"FFB0B0B0\"/></patternFill></fill></fills>"
                       "<borders count=\"3\"><border><left/><right/><top/><bottom/><diagonal/></border>"
                       "<border><left/><right/><top/><bottom style=\"medium\"><color rgb=\"FF13773A\"/></bottom><diagonal/></border>"
                       "<border><left style=\"thin\"><color auto=\"1\"/></left><right style=\"thin\"><color auto=\"1\"/></right><top style=\"thin\"><color auto=\"1\"/></top>"
                       "<bottom style=\"double\"><color auto=\"1\"/></bottom><diagonal/></border></borders>"));
}

// The sheet "Budget": a title over A1:G1 (merged), headers (a frozen row and column), six categories by six months, totals whose stored results are 0 (the calculation corrects
// them), a percent that is a formula, a date, a number too wide for its cell, and wrapped notes in a merged block.
inline QByteArray budgetSheet()
{
    auto text = [](const QString &ref, const QString &t, int style) {
        return QStringLiteral("<c r=\"%1\" s=\"%3\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, t.toHtmlEscaped()).arg(style);
    };
    auto num = [](const QString &ref, double v, int style) { return QStringLiteral("<c r=\"%1\" s=\"%3\"><v>%2</v></c>").arg(ref).arg(v, 0, 'g', 15).arg(style); };
    QString rows = QStringLiteral("<row r=\"1\" ht=\"30\" customHeight=\"1\">") + text("A1", "Household Budget 2026", 1) + QStringLiteral("</row>");
    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun"};
    QString head = text("A2", "Category", 2);
    for (int j = 0; j < 6; ++j) head += text(QStringLiteral("%1%2").arg(QChar('B' + j)).arg(2), QString::fromLatin1(months[j]), 2);
    rows += QStringLiteral("<row r=\"2\">") + head + QStringLiteral("</row>");
    const char *cats[] = {"Rent", "Groceries", "Utilities", "Transport", "Fun", "A very long category name that spills over"};
    for (int i = 0; i < 6; ++i) {
        QString cells = text(QStringLiteral("A%1").arg(3 + i), QString::fromLatin1(cats[i]), 0);
        for (int j = 0; j < 6; ++j) cells += num(QStringLiteral("%1%2").arg(QChar('B' + j)).arg(3 + i), 100 + i * 37 + j * 11.5, 3);
        rows += QStringLiteral("<row r=\"%1\">%2</row>").arg(3 + i).arg(cells);
    }
    QString total = text("A9", "Total", 6);
    for (int j = 0; j < 6; ++j) total += QStringLiteral("<c r=\"%1\" s=\"6\"><f>SUM(%1%2:%1%3)</f><v>0</v></c>").arg(QChar('B' + j)).arg(3).arg(8);
    rows += QStringLiteral("<row r=\"9\">") + total + QStringLiteral("</row>");
    rows += QStringLiteral("<row r=\"11\">") + text("A11", "Growth", 0) + QStringLiteral("<c r=\"B11\" s=\"4\"><f>C9/B9-1</f><v>0.5</v></c>") + num("C11", 46100, 5) + num("D11", 12345678901.5, 8)
        + QStringLiteral("</row>");
    rows += QStringLiteral("<row r=\"13\" ht=\"48\" customHeight=\"1\">") + text("A13", "Notes: amounts are estimates, wrap text keeps long notes inside their cell.", 7) + QStringLiteral("</row>");
    const QString head2 = QStringLiteral("<sheetPr><tabColor rgb=\"FF13773A\"/></sheetPr><dimension ref=\"A1:G13\"/><sheetViews><sheetView workbookViewId=\"0\">"
                                         "<pane xSplit=\"1\" ySplit=\"2\" topLeftCell=\"B3\" activePane=\"bottomRight\" state=\"frozen\"/><selection pane=\"bottomRight\" activeCell=\"B3\" sqref=\"B3\"/>"
                                         "</sheetView></sheetViews><sheetFormatPr defaultRowHeight=\"15\"/><cols><col min=\"1\" max=\"1\" width=\"22.7109375\" customWidth=\"1\"/>"
                                         "<col min=\"2\" max=\"7\" width=\"12.7109375\" customWidth=\"1\"/></cols>");
    return worksheet(rows, QStringLiteral("<mergeCells count=\"2\"><mergeCell ref=\"A1:G1\"/><mergeCell ref=\"A13:D13\"/></mergeCells>"), head2);
}

// A workbook of three sheets (the second hidden) with the budget in the first.
inline QByteArray budgetWorkbook()
{
    const QByteArray other = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>other</t></is></c><c r=\"B1\"><v>7</v></c></row>"
                                                      "<row r=\"2\"><c r=\"A2\"><v>3</v></c></row>"));
    return buildXlsx({budgetSheet(), other, other}, QByteArray(), QString(), QString(), budgetStyles(), QStringList({"Budget", "Hidden", "Notes"}), QString(), QByteArray(), QString(), {1});
}

// The file in a folder that goes away with the object.
struct Folder {
    QTemporaryDir dir;
    QString write(const QString &name, const QByteArray &bytes)
    {
        const QString path = dir.filePath(name);
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly)) return QString();
        f.write(bytes);
        return path;
    }
};

} // namespace crunchtest
