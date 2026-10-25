// Tests of what a workbook says about how its cells look and how its sheets are laid out: the fonts, fills, borders, and cell formats of the styles part, the
// colors they name (RGB, a theme color with a tint, an indexed one), and a sheet's columns, rows, merged ranges, view (frozen panes, gridlines, zoom), and tab
// color. Every workbook is built here, in memory.
#include "sheethelpers.h"

#include "sheet/styles.h"

#include <cmath>

namespace {

bool readStylesXml(const QByteArray &xml, Styles *out, QString *error = nullptr, const Limits &limits = Limits())
{
    QByteArray copy = xml;
    QBuffer buffer(&copy);
    buffer.open(QIODevice::ReadOnly);
    return readStyles(&buffer, limits, out, error);
}

QByteArray stylesDoc(const QString &inside) { return ("<?xml version=\"1.0\"?>\n<styleSheet xmlns=\"" + kMainNs + "\">" + inside + "</styleSheet>").toUtf8(); }

QString row(int r, const QString &cells, const QString &attributes = QString())
{
    return QStringLiteral("<row r=\"%1\"%3>%2</row>").arg(r).arg(cells, attributes.isEmpty() ? QString() : QStringLiteral(" ") + attributes);
}

QString cell(const QString &ref, int value) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref).arg(value); }

// The channels of an RGB, to compare colors that two ways of rounding bring a unit apart.
void compareColor(quint32 got, quint32 want, int tolerance, const char *what)
{
    for (int shift : {16, 8, 0}) {
        const int a = int((got >> shift) & 0xFF), b = int((want >> shift) & 0xFF);
        if (std::abs(a - b) > tolerance) QFAIL(qPrintable(QStringLiteral("%1: %2 is not %3").arg(QLatin1String(what), QString::number(got, 16), QString::number(want, 16))));
    }
}

} // namespace

class SheetLookTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The fonts, fills, and borders the styles list, and the cell formats that choose among them and align the text.
    void fontsFillsBordersAndCellFormatsAreRead()
    {
        Styles s;
        QString why;
        QVERIFY2(readStylesXml(stylesDoc(QStringLiteral(
                                   "<fonts count=\"3\">"
                                   "<font><sz val=\"11\"/><color theme=\"1\"/><name val=\"Calibri\"/><family val=\"2\"/></font>"
                                   "<font><b/><i/><strike/><u val=\"double\"/><sz val=\"14.5\"/><color rgb=\"FFFF0000\"/><name val=\"Arial\"/></font>"
                                   "<font><u/><vertAlign val=\"superscript\"/><sz val=\"9\"/><color indexed=\"10\"/><name val=\"Cambria\"/></font>"
                                   "</fonts>"
                                   "<fills count=\"4\">"
                                   "<fill><patternFill patternType=\"none\"/></fill>"
                                   "<fill><patternFill patternType=\"gray125\"/></fill>"
                                   "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FF00FF00\"/><bgColor indexed=\"64\"/></patternFill></fill>"
                                   "<fill><patternFill patternType=\"darkGrid\"><fgColor theme=\"4\" tint=\"0.5\"/><bgColor rgb=\"FFFFFF00\"/></patternFill></fill>"
                                   "</fills>"
                                   "<borders count=\"2\">"
                                   "<border><left/><right/><top/><bottom/><diagonal/></border>"
                                   "<border diagonalUp=\"1\"><left style=\"thin\"><color auto=\"1\"/></left><right style=\"mediumDashDot\"><color rgb=\"FF0000FF\"/></right>"
                                   "<top style=\"double\"/><bottom style=\"hair\"><color indexed=\"12\"/></bottom><diagonal style=\"dotted\"/></border>"
                                   "</borders>"
                                   "<cellXfs count=\"3\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/>"
                                   "<xf numFmtId=\"164\" fontId=\"1\" fillId=\"2\" borderId=\"1\"><alignment horizontal=\"center\" vertical=\"top\" wrapText=\"1\" indent=\"2\" textRotation=\"45\" shrinkToFit=\"1\"/></xf>"
                                   "<xf fontId=\"2\" fillId=\"3\" borderId=\"1\"><alignment horizontal=\"centerContinuous\" vertical=\"center\" textRotation=\"255\"/></xf>"
                                   "</cellXfs>")),
                               &s, &why),
                 qPrintable(why));
        QCOMPARE(s.fonts.size(), 3);
        QCOMPARE(s.fonts[0].name, QStringLiteral("Calibri"));
        QCOMPARE(s.fonts[0].size, 11.0);
        QVERIFY(!s.fonts[0].bold);
        QVERIFY(s.fonts[0].color.kind == ColorSpec::Kind::Theme);
        QCOMPARE(s.fonts[0].color.index, 1);
        QCOMPARE(s.fonts[1].name, QStringLiteral("Arial"));
        QCOMPARE(s.fonts[1].size, 14.5);
        QVERIFY(s.fonts[1].bold && s.fonts[1].italic && s.fonts[1].strike);
        QVERIFY(s.fonts[1].underline == Underline::Double);
        QVERIFY(s.fonts[1].color.kind == ColorSpec::Kind::Rgb);
        QCOMPARE(s.fonts[1].color.rgb, 0xFF0000u);
        QVERIFY(s.fonts[2].underline == Underline::Single);
        QCOMPARE(int(s.fonts[2].vertAlign), 1);
        QVERIFY(s.fonts[2].color.kind == ColorSpec::Kind::Indexed);
        QCOMPARE(s.fonts[2].color.index, 10);
        QCOMPARE(s.fills.size(), 4);
        QVERIFY(s.fills[0].pattern == PatternKind::None);
        QVERIFY(s.fills[1].pattern == PatternKind::Gray125);
        QVERIFY(s.fills[2].pattern == PatternKind::Solid);
        QCOMPARE(s.fills[2].fg.rgb, 0x00FF00u);
        QVERIFY(s.fills[2].bg.kind == ColorSpec::Kind::Indexed);
        QVERIFY(s.fills[3].pattern == PatternKind::DarkGrid);
        QVERIFY(s.fills[3].fg.kind == ColorSpec::Kind::Theme);
        QCOMPARE(s.fills[3].fg.index, 4);
        QCOMPARE(s.fills[3].fg.tint, 0.5);
        QCOMPARE(s.fills[3].bg.rgb, 0xFFFF00u);
        QCOMPARE(s.borders.size(), 2);
        QVERIFY(s.borders[1].left.style == BorderStyle::Thin);
        QVERIFY(s.borders[1].left.color.kind == ColorSpec::Kind::Auto);
        QVERIFY(s.borders[1].right.style == BorderStyle::MediumDashDot);
        QCOMPARE(s.borders[1].right.color.rgb, 0x0000FFu);
        QVERIFY(s.borders[1].top.style == BorderStyle::Double);
        QVERIFY(s.borders[1].bottom.style == BorderStyle::Hair);
        QVERIFY(s.borders[1].diagonal.style == BorderStyle::Dotted);
        QVERIFY(s.borders[1].diagonalUp && !s.borders[1].diagonalDown);
        QCOMPARE(s.formats.size(), 3);
        QCOMPARE(s.formats[1].numFmtId, 164);
        QCOMPARE(s.formats[1].font, 1u);
        QCOMPARE(s.formats[1].fill, 2u);
        QCOMPARE(s.formats[1].border, 1u);
        QVERIFY(s.formats[1].horizontal == HAlign::Center);
        QVERIFY(s.formats[1].vertical == VAlign::Top);
        QVERIFY(s.formats[1].wrap && s.formats[1].shrink);
        QCOMPARE(int(s.formats[1].indent), 2);
        QCOMPARE(int(s.formats[1].rotation), 45);
        QVERIFY(s.formats[2].horizontal == HAlign::CenterContinuous);
        QVERIFY(s.formats[2].vertical == VAlign::Center);
        QCOMPARE(int(s.formats[2].rotation), 255);
        // (a format with no alignment: text at the bottom, as the format says)
        QVERIFY(s.formats[0].horizontal == HAlign::General && s.formats[0].vertical == VAlign::Bottom);
        // a place the file doesn't have is the plain look
        QCOMPARE(s.font(99).size, 11.0);
        QVERIFY(s.fill(99).pattern == PatternKind::None);
        QVERIFY(s.border(99).left.style == BorderStyle::None);
        QVERIFY(s.format(99).horizontal == HAlign::General);
    }

    // A color is as the file gives it, or a place in the theme (with a tint that lightens or darkens it) or in the palette.
    void colorsAreResolved()
    {
        Styles s;
        s.theme[0] = 0xFFFFFF;
        s.theme[1] = 0x000000;
        s.theme[4] = 0x336699;
        ColorSpec rgb;
        rgb.kind = ColorSpec::Kind::Rgb;
        rgb.rgb = 0x123456;
        QCOMPARE(s.resolve(rgb, 0xABCDEF), 0x123456u);
        QCOMPARE(s.resolve(ColorSpec(), 0xABCDEF), 0xABCDEFu);
        ColorSpec automatic;
        automatic.kind = ColorSpec::Kind::Auto;
        QCOMPARE(s.resolve(automatic, 0xABCDEF), 0xABCDEFu);
        auto theme = [&](int index, double tint) {
            ColorSpec c;
            c.kind = ColorSpec::Kind::Theme;
            c.index = index;
            c.tint = tint;
            return s.resolve(c, 0);
        };
        QCOMPARE(theme(4, 0), 0x336699u);
        // white a twentieth darker, black half lighter
        compareColor(theme(0, -0.05), 0xF2F2F2, 1, "white, darker 5%");
        compareColor(theme(1, 0.5), 0x7F7F7F, 1, "black, lighter 50%");
        // a color's lightness is moved a half of the way to white (lighter) or to black (darker): 0x336699 has lightness 0.4
        compareColor(theme(4, 0.5), 0x8CB3D9, 2, "accent, lighter 50%");
        compareColor(theme(4, -0.5), 0x1A334D, 2, "accent, darker 50%");
        // a place past the twelve is the caller's color
        QCOMPARE(theme(40, 0), 0u);
        auto indexed = [&](int index) {
            ColorSpec c;
            c.kind = ColorSpec::Kind::Indexed;
            c.index = index;
            return s.resolve(c, 0xABCDEF);
        };
        QCOMPARE(indexed(10), 0xFF0000u);
        QCOMPARE(indexed(12), 0x0000FFu);
        QCOMPARE(indexed(22), 0xC0C0C0u);
        QCOMPARE(indexed(2), 0xFF0000u);
        // 64 and 65 stand for the system's colors: whatever the text or the cell has when it names none
        QCOMPARE(indexed(64), 0xABCDEFu);
        QCOMPARE(indexed(65), 0xABCDEFu);
        QCOMPARE(Styles::defaultIndexed(8), 0x000000u);
        QCOMPARE(Styles::defaultIndexed(9), 0xFFFFFFu);
        QCOMPARE(Styles::defaultIndexed(63), 0x333333u);
        QCOMPARE(Styles::defaultIndexed(64), 0u);
    }

    // A palette the file defines replaces the usual one place by place.
    void aPaletteOfTheFileReplacesTheUsualOne()
    {
        Styles s;
        QVERIFY(readStylesXml(stylesDoc(QStringLiteral("<colors><indexedColors><rgbColor rgb=\"FF010203\"/><rgbColor rgb=\"FF040506\"/></indexedColors></colors>")), &s));
        QCOMPARE(s.indexed.size(), 2);
        ColorSpec c;
        c.kind = ColorSpec::Kind::Indexed;
        c.index = 1;
        QCOMPARE(s.resolve(c, 0), 0x040506u);
        c.index = 10;   // the places the file doesn't list keep the usual color
        QCOMPARE(s.resolve(c, 0), 0xFF0000u);
    }

    // A styles part with more fonts, fills, borders, or cell formats than any real file keeps what came before the limit and says so.
    void stylesAreBounded()
    {
        const QByteArray fonts = stylesDoc(QStringLiteral("<fonts><font><sz val=\"1\"/></font><font><sz val=\"2\"/></font><font><sz val=\"3\"/></font></fonts>"));
        Limits few;
        few.fonts = 2;
        Styles s;
        QString why;
        QVERIFY(!readStylesXml(fonts, &s, &why, few));
        QVERIFY2(why.contains(QLatin1String("fonts")), qPrintable(why));
        QCOMPARE(s.fonts.size(), 2);
        few = Limits();
        few.fills = 1;
        QVERIFY(!readStylesXml(stylesDoc(QStringLiteral("<fills><fill/><fill/></fills>")), &s, &why, few));
        QVERIFY2(why.contains(QLatin1String("fills")), qPrintable(why));
        few = Limits();
        few.borders = 1;
        QVERIFY(!readStylesXml(stylesDoc(QStringLiteral("<borders><border/><border/></borders>")), &s, &why, few));
        QVERIFY2(why.contains(QLatin1String("borders")), qPrintable(why));
        // a million fonts' worth of text is not read into the styles either
        QString many;
        for (int i = 0; i < 2000; ++i) many += QStringLiteral("<font><sz val=\"%1\"/><name val=\"Arial\"/></font>").arg(8 + i % 40);
        QVERIFY(readStylesXml(stylesDoc("<fonts>" + many + "</fonts>"), &s));
        QCOMPARE(s.fonts.size(), 2000);
        // numbers that are not numbers are not read as sizes
        QVERIFY(readStylesXml(stylesDoc(QStringLiteral("<fonts><font><sz val=\"nan\"/></font><font><sz val=\"1e9\"/></font></fonts>")), &s));
        QVERIFY(std::isfinite(s.fonts[0].size) && s.fonts[0].size > 0 && s.fonts[0].size < 1000);
        QVERIFY(s.fonts[1].size > 0 && s.fonts[1].size <= 409.5);
    }

    // The sizes of rows and columns that have none of their own, the runs of columns, and the rows that are not as the rest.
    void columnsRowsAndTheirDefaultsAreRead()
    {
        Sheet sheet;
        QVERIFY(readSheetXml(worksheet(row(2, cell("A2", 1), QStringLiteral("ht=\"30\" customHeight=\"1\"")) + row(3, QString(), QStringLiteral("hidden=\"1\"")) +
                                           row(5, QString(), QStringLiteral("s=\"2\" customFormat=\"1\" ht=\"18.75\"")) + row(7, cell("A7", 3), QStringLiteral("ht=\"18.75\"")),
                                       QString(),
                                       QStringLiteral("<sheetFormatPr defaultRowHeight=\"18.75\" defaultColWidth=\"11.5\" baseColWidth=\"9\" customHeight=\"1\"/>"
                                                      "<cols><col min=\"1\" max=\"2\" width=\"12.7109375\" customWidth=\"1\" style=\"3\"/><col min=\"4\" max=\"4\" width=\"0\" hidden=\"1\"/>"
                                                      "<col min=\"6\" max=\"20000\" width=\"9\"/></cols>")),
                             &sheet));
        QCOMPARE(sheet.format.defaultRowHeight, 18.75);
        QCOMPARE(sheet.format.defaultColWidth, 11.5);
        QCOMPARE(sheet.format.baseColWidth, 9);
        QVERIFY(!sheet.format.zeroHeight && sheet.format.customHeight);
        QCOMPARE(sheet.columns.size(), 3);
        QCOMPARE(sheet.columns[0].first, 0u);
        QCOMPARE(sheet.columns[0].last, 1u);
        QCOMPARE(sheet.columns[0].width, 12.7109375);
        QVERIFY(sheet.columns[0].customWidth && !sheet.columns[0].hidden);
        QCOMPARE(sheet.columns[0].style, 3u);
        QCOMPARE(sheet.columns[1].first, 3u);
        QVERIFY(sheet.columns[1].hidden);
        // a run past the last column ends at it
        QCOMPARE(sheet.columns[2].last, kMaxColumns - 1);
        // the rows that are as the default (a height that is the sheet's own, and nothing else) are not kept; the others are, with their places from 0
        QCOMPARE(sheet.rowSpecs.size(), 3);
        QCOMPARE(sheet.rowSpecs[0].row, 1u);
        QCOMPARE(sheet.rowSpecs[0].height, 30.0);
        QVERIFY(sheet.rowSpecs[0].customHeight);
        QCOMPARE(sheet.rowSpecs[1].row, 2u);
        QVERIFY(sheet.rowSpecs[1].hidden);
        QCOMPARE(sheet.rowSpecs[2].row, 4u);
        QVERIFY(sheet.rowSpecs[2].customFormat);
        QCOMPARE(sheet.rowSpecs[2].style, 2u);
        // the cell of row 7 was read, whatever became of its row
        QVERIFY(at(sheet, "A7"));
    }

    // A row of 4000 points or a column of a thousand characters is the most the format allows; a size that is not a number is none.
    void rowAndColumnSizesAreBounded()
    {
        Sheet sheet;
        QVERIFY(readSheetXml(worksheet(row(1, QString(), QStringLiteral("ht=\"4000\" customHeight=\"1\"")) + row(2, QString(), QStringLiteral("ht=\"0\" customHeight=\"1\"")) +
                                           row(3, QString(), QStringLiteral("ht=\"nan\" customHeight=\"1\"")) + row(4, QString(), QStringLiteral("ht=\"-5\" customHeight=\"1\"")),
                                       QString(),
                                       QStringLiteral("<cols><col min=\"1\" max=\"1\" width=\"1000\"/><col min=\"2\" max=\"2\" width=\"1e9\"/><col min=\"3\" max=\"3\" width=\"-4\"/>"
                                                      "<col min=\"5\" max=\"4\" width=\"9\"/><col min=\"0\" max=\"1\" width=\"9\"/></cols>")),
                             &sheet));
        QCOMPARE(sheet.rowSpecs.size(), 2);
        QCOMPARE(sheet.rowSpecs[0].height, 409.5);
        QCOMPARE(sheet.rowSpecs[1].height, 0.0);
        QCOMPARE(sheet.columns.size(), 2);
        QCOMPARE(sheet.columns[0].width, 255.0);
        QCOMPARE(sheet.columns[1].width, 255.0);
        // (a run that ends before it begins, and one that begins at 0, are not runs of the sheet)
    }

    // The merged ranges, in the file's order; a "range" of one cell is no merge, and an address that is none is not read.
    void mergedRangesAreRead()
    {
        Sheet sheet;
        QVERIFY(readSheetXml(worksheet(cell("A1", 1), QStringLiteral("<mergeCells count=\"4\"><mergeCell ref=\"A1:B2\"/><mergeCell ref=\"D4\"/><mergeCell ref=\"X\"/><mergeCell ref=\"E2:G9\"/></mergeCells>")), &sheet));
        QCOMPARE(sheet.mergeCount, qint64(4));
        QCOMPARE(sheet.merges.size(), 2);
        QCOMPARE(sheet.merges[0].row1, 0u);
        QCOMPARE(sheet.merges[0].col1, 0u);
        QCOMPARE(sheet.merges[0].row2, 1u);
        QCOMPARE(sheet.merges[0].col2, 1u);
        QCOMPARE(sheet.merges[1].row1, 1u);
        QCOMPARE(sheet.merges[1].col1, 4u);
        QCOMPARE(sheet.merges[1].row2, 8u);
        QCOMPARE(sheet.merges[1].col2, 6u);
        // more than the limit are refused, and what came before stays
        ReadOptions few;
        few.limits.merges = 2;
        QString many;
        for (int i = 0; i < 5; ++i) many += QStringLiteral("<mergeCell ref=\"A%1:B%1\"/>").arg(i * 2 + 1);
        QVERIFY(!readSheetXml(worksheet(cell("A1", 1), "<mergeCells>" + many + "</mergeCells>"), &sheet, few));
        QVERIFY2(sheet.error.contains(QLatin1String("merged")), qPrintable(sheet.error));
        QCOMPARE(sheet.merges.size(), 2);
    }

    // How the sheet was left on the screen: gridlines, zoom, the first rows and columns held in place, where it was scrolled to, and the cell chosen.
    void theViewIsRead()
    {
        Sheet sheet;
        QVERIFY(readSheetXml(worksheet(cell("A1", 1), QString(),
                                       QStringLiteral("<sheetPr><tabColor rgb=\"FFC00000\"/></sheetPr><sheetViews><sheetView showGridLines=\"0\" showRowColHeaders=\"0\" showZeros=\"0\" rightToLeft=\"1\" "
                                                      "tabSelected=\"1\" zoomScale=\"150\" topLeftCell=\"B3\"><pane xSplit=\"2\" ySplit=\"1\" topLeftCell=\"D5\" activePane=\"bottomRight\" state=\"frozen\"/>"
                                                      "<selection pane=\"topRight\" activeCell=\"A1\" sqref=\"A1\"/><selection pane=\"bottomRight\" activeCell=\"E7\" sqref=\"E7:F9\"/></sheetView></sheetViews>")),
                             &sheet));
        const SheetView &v = sheet.view;
        QVERIFY(!v.showGridLines && !v.showHeadings && !v.showZeros && v.rightToLeft && v.tabSelected);
        QCOMPARE(v.zoom, 150);
        QVERIFY(v.frozen);
        QCOMPARE(v.frozenCols, 2u);
        QCOMPARE(v.frozenRows, 1u);
        QCOMPARE(v.topRow, 2u);
        QCOMPARE(v.leftCol, 1u);
        QCOMPARE(v.paneRow, 4u);
        QCOMPARE(v.paneCol, 3u);
        // the cell chosen is that of the pane that has the cursor
        QCOMPARE(v.activeRow, 6u);
        QCOMPARE(v.activeCol, 4u);
        QVERIFY(sheet.tabColor.kind == ColorSpec::Kind::Rgb);
        QCOMPARE(sheet.tabColor.rgb, 0xC00000u);
    }

    // The kind of view a sheet was left in: the cells, the page breaks, or the pages; a name that is not one is the cells.
    void theKindOfViewIsRead()
    {
        auto kindOf = [](const char *attr) {
            Sheet sheet;
            if (!readSheetXml(worksheet(cell("A1", 1), QString(), QStringLiteral("<sheetViews><sheetView %1 workbookViewId=\"0\"/></sheetViews>").arg(QLatin1String(attr))), &sheet)) return -1;
            return int(sheet.view.kind);
        };
        QCOMPARE(kindOf(""), int(SheetView::Kind::Normal));
        QCOMPARE(kindOf("view=\"normal\""), int(SheetView::Kind::Normal));
        QCOMPARE(kindOf("view=\"pageBreakPreview\" zoomScale=\"60\""), int(SheetView::Kind::PageBreaks));
        QCOMPARE(kindOf("view=\"pageLayout\""), int(SheetView::Kind::PageLayout));
        QCOMPARE(kindOf("view=\"other\""), int(SheetView::Kind::Normal));
    }

    // The zoom of the cells as they are is read apart from the zoom of the view the sheet was left in: its own attribute, else the zoom of the sheet when it was left in that view, else 100.
    void theNormalZoomIsReadApartFromTheViewsZoom()
    {
        auto zooms = [](const char *attr) {
            Sheet sheet;
            if (!readSheetXml(worksheet(cell("A1", 1), QString(), QStringLiteral("<sheetViews><sheetView %1 workbookViewId=\"0\"/></sheetViews>").arg(QLatin1String(attr))), &sheet)) return QStringLiteral("unread");
            return QStringLiteral("%1/%2").arg(sheet.view.zoom).arg(sheet.view.normalZoom);
        };
        QCOMPARE(zooms(""), QStringLiteral("100/100"));
        QCOMPARE(zooms("zoomScale=\"80\""), QStringLiteral("80/80"));
        QCOMPARE(zooms("view=\"pageBreakPreview\" zoomScale=\"60\" zoomScaleNormal=\"130\""), QStringLiteral("60/130"));
        QCOMPARE(zooms("view=\"pageLayout\" zoomScale=\"70\""), QStringLiteral("70/100"));
        QCOMPARE(zooms("zoomScale=\"90\" zoomScaleNormal=\"125\""), QStringLiteral("90/125"));
    }

    // A split that is not frozen is not kept, and the view of a sheet with none is the plain one; a zoom outside what the program zooms to is brought in.
    void aSplitIsNotAFreezeAndTheZoomIsBounded()
    {
        Sheet sheet;
        QVERIFY(readSheetXml(worksheet(cell("A1", 1), QString(),
                                       QStringLiteral("<sheetViews><sheetView zoomScale=\"4000\"><pane xSplit=\"1500\" ySplit=\"600\" state=\"split\"/></sheetView></sheetViews>")), &sheet));
        QVERIFY(!sheet.view.frozen);
        QCOMPARE(sheet.view.frozenCols, 0u);
        QCOMPARE(sheet.view.zoom, 400);
        QVERIFY(readSheetXml(worksheet(cell("A1", 1), QString(), QStringLiteral("<sheetViews><sheetView zoomScale=\"1\"><pane ySplit=\"2\" state=\"frozenSplit\"/></sheetView></sheetViews>")), &sheet));
        QVERIFY(sheet.view.frozen);
        QCOMPARE(sheet.view.frozenRows, 2u);
        QCOMPARE(sheet.view.frozenCols, 0u);
        QCOMPARE(sheet.view.zoom, 10);
        QVERIFY(readSheetXml(worksheet(cell("A1", 1)), &sheet));
        QVERIFY(sheet.view.showGridLines && sheet.view.showHeadings && !sheet.view.frozen);
        QCOMPARE(sheet.view.zoom, 100);
        // frozen panes more than the sheet has rows or columns are not kept
        QVERIFY(readSheetXml(worksheet(cell("A1", 1), QString(), QStringLiteral("<sheetViews><sheetView><pane xSplit=\"99999\" ySplit=\"99999999\" state=\"frozen\"/></sheetView></sheetViews>")), &sheet));
        QVERIFY(!sheet.view.frozen || (sheet.view.frozenCols < kMaxColumns && sheet.view.frozenRows < kMaxRows));
    }

    // A read that stops at a row has the cells before it, and says it is part of the sheet: nothing after the rows (the merged ranges) is read.
    void aReadStopsAtARow()
    {
        QString rows;
        for (int r = 1; r <= 10; ++r) rows += row(r, cell(QStringLiteral("A%1").arg(r), r));
        const QByteArray xml = worksheet(rows, QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"A1:B2\"/></mergeCells>"),
                                         QStringLiteral("<sheetFormatPr defaultRowHeight=\"21\"/><cols><col min=\"1\" max=\"1\" width=\"20\"/></cols>"));
        Sheet sheet;
        ReadOptions options;
        options.stopAfterRow = 3;   // rows 1 to 4
        QVERIFY2(readSheetXml(xml, &sheet, options), qPrintable(sheet.error));
        QVERIFY(sheet.partial);
        QVERIFY(sheet.error.isEmpty());
        QCOMPARE(sheet.cells.cellCount(), qint64(4));
        QVERIFY(at(sheet, "A4"));
        QVERIFY(!at(sheet, "A5"));
        QCOMPARE(sheet.format.defaultRowHeight, 21.0);
        QCOMPARE(sheet.columns.size(), 1);
        QCOMPARE(sheet.merges.size(), 0);
        // asked for more rows than the sheet has, it is read whole
        options.stopAfterRow = 50;
        QVERIFY(readSheetXml(xml, &sheet, options));
        QVERIFY(!sheet.partial);
        QCOMPARE(sheet.cells.cellCount(), qint64(10));
        QCOMPARE(sheet.merges.size(), 1);
    }

    // The rows and columns the cells reach: cells with a value, a formula, or a format of their own count; plain empty cells do not. A row element with nothing in it is no cell.
    void theReachOfTheCellsIsKept()
    {
        Sheet sheet;
        QVERIFY(readSheetXml(worksheet(row(3, cell("C3", 1)) + row(9, QStringLiteral("<c r=\"B9\"/><c r=\"F9\"/>")) + row(12, QStringLiteral("<c r=\"D12\" s=\"2\"/>")) +
                                           row(20, QStringLiteral("<c r=\"E20\"><f>A1</f></c>")) + row(30, QString())),
                             &sheet));
        QCOMPARE(sheet.extentRows, 20u);
        QCOMPARE(sheet.extentCols, 5u);
        QVERIFY(readSheetXml(worksheet(QString()), &sheet));
        QCOMPARE(sheet.extentRows, 0u);
        QCOMPARE(sheet.extentCols, 0u);
        QVERIFY(readSheetXml(worksheet(row(5, QStringLiteral("<c r=\"B5\" s=\"1\"/>"))), &sheet));
        QCOMPARE(sheet.extentRows, 5u);
        QCOMPARE(sheet.extentCols, 2u);
    }

    // A sheet that hides the rows that have no element keeps every row that has one, so that the rows the file shows can be told from those it hides.
    void aSheetThatHidesUnlistedRowsKeepsTheListedOnes()
    {
        Sheet sheet;
        QVERIFY(readSheetXml(worksheet(row(1, cell("A1", 1)) + row(2, cell("A2", 1), QStringLiteral("ht=\"30\" customHeight=\"1\"")) + row(4, cell("A4", 1)), QString(),
                                       QStringLiteral("<sheetFormatPr defaultRowHeight=\"15\" zeroHeight=\"1\"/>")),
                             &sheet));
        QCOMPARE(sheet.rowSpecs.size(), 3);
        QCOMPARE(sheet.rowSpecs[0].row, 0u);
        QVERIFY(!sheet.rowSpecs[0].hidden);
        QCOMPARE(sheet.rowSpecs[2].row, 3u);
        // without it, rows as the default are not kept
        QVERIFY(readSheetXml(worksheet(row(1, cell("A1", 1))), &sheet));
        QCOMPARE(sheet.rowSpecs.size(), 0);
    }

    // The theme part gives the colors the styles name by place, and the workbook says which sheet was open.
    void theThemeAndTheOpenSheetComeWithTheWorkbook()
    {
        const QByteArray theme = QByteArray(
            "<?xml version=\"1.0\"?>\n<a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" name=\"T\"><a:themeElements><a:clrScheme name=\"C\">"
             "<a:dk1><a:srgbClr val=\"111111\"/></a:dk1><a:lt1><a:srgbClr val=\"EEEEEE\"/></a:lt1><a:dk2><a:srgbClr val=\"222222\"/></a:dk2><a:lt2><a:srgbClr val=\"DDDDDD\"/></a:lt2>"
             "<a:accent1><a:srgbClr val=\"AA0000\"/></a:accent1><a:accent2><a:srgbClr val=\"00AA00\"/></a:accent2><a:accent3><a:srgbClr val=\"0000AA\"/></a:accent3>"
             "<a:accent4><a:srgbClr val=\"AAAA00\"/></a:accent4><a:accent5><a:srgbClr val=\"00AAAA\"/></a:accent5><a:accent6><a:srgbClr val=\"AA00AA\"/></a:accent6>"
             "<a:hlink><a:srgbClr val=\"0000FF\"/></a:hlink><a:folHlink><a:srgbClr val=\"800080\"/></a:folHlink></a:clrScheme>"
             "<a:fontScheme name=\"F\"><a:majorFont><a:latin typeface=\"Arial\"/></a:majorFont><a:minorFont><a:latin typeface=\"Arial\"/></a:minorFont></a:fontScheme></a:themeElements></a:theme>");
        const QByteArray xlsx = buildXlsx({worksheet(cell("A1", 1)), worksheet(QString())}, QByteArray(), QString(), QString(),
                                          stylesXml(QString(), QStringLiteral("<xf/>")), QStringList(), QString(), theme, QStringLiteral("<workbookView activeTab=\"1\" firstSheet=\"1\"/>"));
        Package package;
        QString why;
        QVERIFY2(package.readLazy(xlsx, &why), qPrintable(why));
        Workbook book;
        QVERIFY2(book.open(package, &why), qPrintable(why));
        QCOMPARE(book.info().activeTab, 1);
        QCOMPARE(book.info().firstSheet, 1);
        // the styles name the theme's colors in the order light 1, dark 1, light 2, dark 2, then the accents
        QCOMPARE(book.styles().theme[0], 0xEEEEEEu);
        QCOMPARE(book.styles().theme[1], 0x111111u);
        QCOMPARE(book.styles().theme[2], 0xDDDDDDu);
        QCOMPARE(book.styles().theme[3], 0x222222u);
        QCOMPARE(book.styles().theme[4], 0xAA0000u);
        QCOMPARE(book.styles().theme[9], 0xAA00AAu);
        QCOMPARE(book.styles().theme[10], 0x0000FFu);
        QCOMPARE(book.styles().theme[11], 0x800080u);
        // a workbook with no theme part has the usual colors
        const QByteArray plain = buildXlsx({worksheet(cell("A1", 1))});
        Package p2;
        QVERIFY(p2.readLazy(plain, &why));
        Workbook b2;
        QVERIFY(b2.open(p2, &why));
        QCOMPARE(b2.info().activeTab, 0);
        QCOMPARE(b2.styles().theme[0], 0xFFFFFFu);
        QCOMPARE(b2.styles().theme[1], 0x000000u);
    }

    // A font's name is kept to its limit (31 characters in the format, 255 kept): a styles part of a hundred thousand fonts with names of a million characters does not hold them.
    void aFontsNameIsKeptToItsLimit()
    {
        Styles s;
        const QString huge(1000000, QLatin1Char('N'));
        QString why;
        QVERIFY2(readStylesXml(stylesDoc(QStringLiteral("<fonts count=\"2\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font><font><sz val=\"11\"/><name val=\"%1\"/></font></fonts>"
                                                          "<cellXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"1\"/></cellXfs>").arg(huge)), &s, &why), qPrintable(why));
        QCOMPARE(s.fonts.size(), 2);
        QCOMPARE(s.fonts[0].name, QStringLiteral("Calibri"));
        QCOMPARE(s.fonts[1].name.size(), kMaxFontNameChars);
        QCOMPARE(s.fonts[1].name, huge.left(kMaxFontNameChars));
    }

    // A read for the screen keeps what the view shows: from the row the view begins at (not the first rows, which a view scrolled far has left), and the frozen rows, whether they begin
    // at the top of the sheet or below it; the rows between, and above, keep no cells (but the read goes by them as quick as it can, and can be stopped).
    void aReadForTheScreenKeepsWhatTheViewShows()
    {
        QString rows;
        for (int r = 1; r <= 5000; ++r) rows += row(r, QStringLiteral("<c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><v>7</v></c>").arg(r), r == 2500 ? QStringLiteral("ht=\"40\" customHeight=\"1\"") : QString());
        auto readWith = [&](const QString &views, Sheet *out) {
            ReadOptions o;
            o.screenRows = 100;
            return readSheetXml(worksheet(rows, QString(), views), out, o);
        };
        auto has = [](const Sheet &s, const char *ref) { return at(s, QLatin1String(ref)) != nullptr; };
        // scrolled to row 3001
        Sheet scrolled;
        QVERIFY(readWith(QStringLiteral("<sheetViews><sheetView workbookViewId=\"0\" topLeftCell=\"A3001\"/></sheetViews>"), &scrolled));
        QVERIFY(scrolled.partial);
        QVERIFY(has(scrolled, "A3001") && has(scrolled, "B3100"));
        QVERIFY2(!has(scrolled, "A3101") && !has(scrolled, "A1") && !has(scrolled, "A3000"), "cells outside what the view shows were kept");
        QCOMPARE(scrolled.view.topRow, quint32(3000));
        // the height of a row above it is kept: where the view begins depends on it
        QVERIFY(scrolled.rowSpecs.size() == 1 && scrolled.rowSpecs[0].row == 2499);
        // two rows frozen, the part that scrolls begun at row 2001
        Sheet frozen;
        QVERIFY(readWith(QStringLiteral("<sheetViews><sheetView workbookViewId=\"0\"><pane ySplit=\"2\" topLeftCell=\"A2001\" activePane=\"bottomLeft\" state=\"frozen\"/></sheetView></sheetViews>"), &frozen));
        QVERIFY(has(frozen, "A1") && has(frozen, "A2") && has(frozen, "A2001") && has(frozen, "A2100"));
        QVERIFY2(!has(frozen, "A3") && !has(frozen, "A2000") && !has(frozen, "A2101"), "cells outside what the view shows were kept");
        // frozen rows that begin below the first (rows 11 and 12), then the part that scrolls from row 2001
        Sheet below;
        QVERIFY(readWith(QStringLiteral("<sheetViews><sheetView workbookViewId=\"0\" topLeftCell=\"A11\"><pane ySplit=\"2\" topLeftCell=\"A2001\" activePane=\"bottomLeft\" state=\"frozen\"/></sheetView></sheetViews>"), &below));
        QVERIFY(has(below, "A11") && has(below, "A12") && has(below, "A2001") && has(below, "A2100"));
        QVERIFY2(!has(below, "A1") && !has(below, "A10") && !has(below, "A13") && !has(below, "A2000"), "cells outside what the view shows were kept");
        // a view that says nothing begins at the first row
        Sheet plain;
        QVERIFY(readWith(QString(), &plain));
        QVERIFY(has(plain, "A1") && has(plain, "A100") && !has(plain, "A101"));
        // and without the option the whole sheet is read
        Sheet whole;
        QVERIFY(readSheetXml(worksheet(rows, QString(), QStringLiteral("<sheetViews><sheetView workbookViewId=\"0\" topLeftCell=\"A3001\"/></sheetViews>")), &whole));
        QVERIFY(has(whole, "A1") && has(whole, "A5000") && !whole.partial);
    }
};

JP_TEST_CLASS(SheetLookTests)
#include "tst_sheet_look.moc"
