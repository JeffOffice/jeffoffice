// Tests of libs/office: DrawingML tables, and the numbers a file gave its shapes.
#include "officehelpers.h"

class OfficeDmlTableTests : public QObject {
    Q_OBJECT


private Q_SLOTS:

    void itemSourceIdIsKeptInJsonAndNotByCopies()
    {
        auto s = std::make_shared<ShapeItem>();
        QVERIFY(!s->toJson().contains(QStringLiteral("srcId")));
        s->sourceId = 42;
        QCOMPARE(s->toJson().value(QStringLiteral("srcId")).toInt(), 42);
        const ItemPtr back = jp::Item::fromJsonAny(s->toJson());
        QCOMPARE(back->sourceId, 42);
        // A copy is a new shape.
        QCOMPARE(s->clone()->sourceId, 0);
        auto t = std::make_shared<TextItem>();
        t->sourceId = 9;
        QCOMPARE(t->clone()->sourceId, 0);
        auto g = std::make_shared<GroupItem>();
        g->children = {s};
        g->sourceId = 3;
        const ItemPtr copy = g->clone();
        QCOMPARE(copy->sourceId, 0);
        QCOMPARE(static_cast<GroupItem *>(copy.get())->children[0]->sourceId, 0);
    }

    // A shape that holds no text keeps its text insets and anchor through its own data (undo, copies): they were written only for a shape
    // with a story, so a shape made with insets of its own (a drawing's, with the text to come) came back with the defaults.
    void shapesWithoutTextKeepTheirInsetsAndAnchorInJson()
    {
        auto s = std::make_shared<ShapeItem>();
        // As made, with the defaults: nothing more is written than before.
        QVERIFY(!s->toJson().contains(QStringLiteral("insets")));
        QVERIFY(!s->toJson().contains(QStringLiteral("valign")));
        s->insets = QMarginsF(1, 2, 3, 4);
        s->valign = VAlign::Bottom;
        QVERIFY(s->storyId.isEmpty());
        const ItemPtr back = jp::Item::fromJsonAny(s->toJson());
        QVERIFY(back && back->type() == ItemType::Shape);
        const auto &b = static_cast<const ShapeItem &>(*back);
        QCOMPARE(b.insets, QMarginsF(1, 2, 3, 4));
        QCOMPARE(int(b.valign), int(VAlign::Bottom));
        // An anchor alone, and with a story.
        auto top = std::make_shared<ShapeItem>();
        top->valign = VAlign::Top;
        QCOMPARE(int(static_cast<const ShapeItem &>(*jp::Item::fromJsonAny(top->toJson())).valign), int(VAlign::Top));
        auto withStory = std::make_shared<ShapeItem>();
        withStory->storyId = QStringLiteral("s1");
        withStory->insets = QMarginsF(5, 6, 7, 8);
        withStory->valign = VAlign::Top;
        const ItemPtr wBack = jp::Item::fromJsonAny(withStory->toJson());   // (held, so the item outlives the line)
        QVERIFY(wBack && wBack->type() == ItemType::Shape);
        const auto &w = static_cast<const ShapeItem &>(*wBack);
        QCOMPARE(w.insets, QMarginsF(5, 6, 7, 8));
        QCOMPARE(int(w.valign), int(VAlign::Top));
    }

    // A text frame, and a shape that holds text, that don't wrap their text keep that through their own data and copies; the default is
    // not written.
    void framesThatDoNotWrapKeepThatInJson()
    {
        auto t = std::make_shared<TextItem>();
        QVERIFY(!t->noWrap && !t->toJson().contains(QStringLiteral("noWrap")));
        t->noWrap = true;
        QVERIFY(static_cast<const TextItem &>(*jp::Item::fromJsonAny(t->toJson())).noWrap);
        QVERIFY(static_cast<const TextItem &>(*t->clone()).noWrap);
        auto text = std::make_shared<ShapeItem>();
        text->storyId = QStringLiteral("s2");
        QVERIFY(!text->noWrap && !text->toJson().contains(QStringLiteral("noWrap")));
        text->noWrap = true;
        QVERIFY(static_cast<const ShapeItem &>(*jp::Item::fromJsonAny(text->toJson())).noWrap);
        QVERIFY(static_cast<const ShapeItem &>(*text->clone()).noWrap);
    }

    void dmlReadKeepsTheNumbersShapesHad()
    {
        DmlEnv env;
        ItemPtr sp = readShape(env.element(dmlSp(dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"ellipse\"/>"), QString(), QString(), QString(), QString(), QString(), QStringLiteral("17"))), env.ctx);
        QVERIFY(sp);
        QCOMPARE(sp->sourceId, 17);
        ItemPtr text = readShape(env.element(dmlSp(dmlXfrm(0, 0, 10, 10), dmlBody(), QString(), QStringLiteral("txBox=\"1\""), QString(), QString(), QStringLiteral("23"))), env.ctx);
        QVERIFY(text);
        QCOMPARE(text->sourceId, 23);
        ItemPtr group = readShape(env.element(QStringLiteral("<p:grpSp><p:nvGrpSpPr><p:cNvPr id=\"30\" name=\"G\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr>%1</p:grpSpPr>%2</p:grpSp>")
                                                .arg(dmlXfrm(0, 0, 10, 10).replace(QStringLiteral("</a:xfrm>"), QStringLiteral("<a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"127000\" cy=\"127000\"/></a:xfrm>")),
                                                     dmlSp(dmlXfrm(0, 0, 5, 5) + QStringLiteral("<a:prstGeom prst=\"ellipse\"/>"), QString(), QString(), QString(), QString(), QString(), QStringLiteral("31")))), env.ctx);
        QVERIFY(group);
        QCOMPARE(group->sourceId, 30);
        QCOMPARE(static_cast<GroupItem *>(group.get())->children[0]->sourceId, 31);
    }

    void dmlBackgroundReadsItsFill()
    {
        DmlEnv env;
        env.useFormats();
        Fill f = readBackground(env.element(QStringLiteral("<p:bg><p:bgPr><a:solidFill><a:srgbClr val=\"102030\"/></a:solidFill><a:effectLst/></p:bgPr></p:bg>")), env.ctx);
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("102030"));
        f = readBackground(env.element(QStringLiteral("<p:bg><p:bgPr><a:gradFill><a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"FF0000\"/></a:gs><a:gs pos=\"100000\"><a:srgbClr val=\"0000FF\"/></a:gs></a:gsLst>"
                                                      "<a:lin ang=\"5400000\"/></a:gradFill></p:bgPr></p:bg>")), env.ctx);
        QVERIFY(f.type == Fill::Gradient);
        QVERIFY(readBackground(env.element(QStringLiteral("<p:bg><p:bgPr><a:noFill/></p:bgPr></p:bg>")), env.ctx).isNone());
        // A reference into the theme's background fills, with the color it gives.
        f = readBackground(env.element(QStringLiteral("<p:bg><p:bgRef idx=\"1001\"><a:schemeClr val=\"bg1\"/></p:bgRef></p:bg>")), env.ctx);
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("FEFEFE"));
        f = readBackground(env.element(QStringLiteral("<p:bg><p:bgRef idx=\"1003\"><a:srgbClr val=\"336699\"/></p:bgRef></p:bg>")), env.ctx);
        QVERIFY(f.type == Fill::Gradient && f.gradType == Fill::Radial);
        QCOMPARE(dmlHex(f.stops[0].color), QStringLiteral("336699"));
        // A reference of 1 to 3 is into the fill styles.
        f = readBackground(env.element(QStringLiteral("<p:bg><p:bgRef idx=\"2\"><a:schemeClr val=\"accent1\"/></p:bgRef></p:bg>")), env.ctx);
        QVERIFY(f.type == Fill::Gradient);
        // Nothing, no scheme to refer to, or a reference past the list is no fill.
        QVERIFY(readBackground(QDomElement(), env.ctx).isNone());
        QVERIFY(readBackground(env.element(QStringLiteral("<p:bg/>")), env.ctx).isNone());
        QVERIFY(readBackground(env.element(QStringLiteral("<p:bg><p:bgRef idx=\"1009\"><a:schemeClr val=\"bg1\"/></p:bgRef></p:bg>")), env.ctx).isNone());
        DmlEnv bare;
        QVERIFY(readBackground(bare.element(QStringLiteral("<p:bg><p:bgRef idx=\"1001\"><a:schemeClr val=\"bg1\"/></p:bgRef></p:bg>")), bare.ctx).isNone());
    }

    // ---- DrawingML tables

    // A 3 by 3 table: the first cell spans two columns and the first of the next row two rows; cells with
    // fills, borders, margins, and anchors of their own, and a bold text cell. No style, so what the cells
    // say is all there is.
    void dmlTableReadsStructureMergesFillsAndBorders()
    {
        DmlEnv env;
        const QString line = QStringLiteral("<a:solidFill><a:srgbClr val=\"%1\"/></a:solidFill>");
        const QString rows =
            QStringLiteral("<a:tr h=\"%1\">").arg(emu(30))
            + dmlTc(QStringLiteral("Total"), QStringLiteral("gridSpan=\"2\""),
                    QStringLiteral("<a:lnT><a:noFill/></a:lnT><a:lnB w=\"25400\">%1</a:lnB>").arg(line.arg(QStringLiteral("0000FF"))) + dmlSolid(QStringLiteral("FF0000")),
                    QStringLiteral("marL=\"127000\" marR=\"63500\" marT=\"38100\" marB=\"25400\" anchor=\"ctr\""), QStringLiteral("b=\"1\""))
            + dmlTc(QString(), QStringLiteral("hMerge=\"1\"")) + dmlTc(QStringLiteral("C0")) + QStringLiteral("</a:tr>")
            + QStringLiteral("<a:tr h=\"%1\">").arg(emu(30))
            + dmlTc(QStringLiteral("tall"), QStringLiteral("rowSpan=\"2\""), QString(), QStringLiteral("anchor=\"b\" vert=\"vert\""))
            + dmlTc(QStringLiteral("E1"))
            + dmlTc(QStringLiteral("F1"), QString(),
                    QStringLiteral("<a:lnR w=\"12700\">%1<a:prstDash val=\"dash\"/></a:lnR><a:lnTlToBr w=\"12700\">%2</a:lnTlToBr>").arg(line.arg(QStringLiteral("00FF00")), line.arg(QStringLiteral("808080"))))
            + QStringLiteral("</a:tr>")
            + QStringLiteral("<a:tr h=\"%1\">").arg(emu(30))
            + dmlTc(QString(), QStringLiteral("vMerge=\"1\"")) + dmlTc(QStringLiteral("H2")) + dmlTc(QStringLiteral("I2"), QString(), dmlSolid(QStringLiteral("00FF00")))
            + QStringLiteral("</a:tr>");
        const QString grid = QStringLiteral("<a:tblGrid><a:gridCol w=\"%1\"/><a:gridCol w=\"%2\"/><a:gridCol w=\"%3\"/></a:tblGrid>").arg(emu(100), emu(150), emu(50));
        auto t = dmlAs<TableItem>(readTable(env.element(dmlTableFrame(QStringLiteral("<a:tblPr/>"), grid, rows, 36, 72, 300, 90)), env.ctx));
        QVERIFY(t);
        QCOMPARE(t->rows, 3);
        QCOMPARE(t->cols, 3);
        QCOMPARE(int(t->cells.size()), 9);
        QVERIFY2(dmlRectIs(t->rect, 36, 72, 300, 90), qPrintable(dmlRectText(t->rect)));
        QCOMPARE(t->colW, (QVector<double>{100, 150, 50}));
        QCOMPARE(t->rowH, (QVector<double>{30, 30, 30}));
        QCOMPARE(t->name, QStringLiteral("Table 3"));
        QCOMPARE(t->altText, QStringLiteral("Figures"));
        QVERIFY(t->wrap.mode == jp::Wrap::None);
        QVERIFY(!t->header && !t->banded);
        QVERIFY(t->format.isEmpty());
        QVERIFY(t->fill.isNone() && t->stroke.isNone());

        // Merges: the cell that spans carries the span, the cells under it are covered.
        QCOMPARE(t->cell(0, 0).colSpan, 2);
        QCOMPARE(t->cell(0, 0).rowSpan, 1);
        QVERIFY(!t->cell(0, 0).covered && t->cell(0, 1).covered && !t->cell(0, 2).covered);
        QCOMPARE(t->cell(1, 0).rowSpan, 2);
        QCOMPARE(t->cell(1, 0).colSpan, 1);
        QVERIFY(!t->cell(1, 0).covered && t->cell(2, 0).covered && !t->cell(2, 1).covered);
        QVERIFY2(dmlRectIs(t->cellRect(0, 0), 0, 0, 250, 30), qPrintable(dmlRectText(t->cellRect(0, 0))));
        QVERIFY2(dmlRectIs(t->cellRect(1, 0), 0, 30, 100, 60), qPrintable(dmlRectText(t->cellRect(1, 0))));

        // Text: each cell has a story, the bold run is bold.
        const auto text = [&](int r, int c) { return env.ctx.storyDoc(t->cell(r, c).storyId)->toPlainText(); };
        QCOMPARE(text(0, 0), QStringLiteral("Total"));
        QCOMPARE(text(0, 2), QStringLiteral("C0"));
        QCOMPARE(text(2, 2), QStringLiteral("I2"));
        QVERIFY(text(0, 1).isEmpty());
        const auto weight = [&](int r, int c) { return env.ctx.storyDoc(t->cell(r, c).storyId)->begin().begin().fragment().charFormat().fontWeight(); };
        QCOMPARE(weight(0, 0), int(QFont::Bold));
        QVERIFY(weight(0, 2) < int(QFont::Bold));

        // Margins and anchors; a cell that says nothing has the usual ones.
        QCOMPARE(t->cell(0, 0).margins, QMarginsF(10, 3, 5, 2));
        QCOMPARE(t->cell(0, 2).margins, QMarginsF(7.2, 3.6, 7.2, 3.6));
        QVERIFY(t->cell(0, 0).valign == jp::VAlign::Middle);
        QVERIFY(t->cell(1, 0).valign == jp::VAlign::Bottom && t->cell(1, 0).vertical);
        QVERIFY(t->cell(0, 2).valign == jp::VAlign::Top && !t->cell(0, 2).vertical);
        QVERIFY(!t->cell(0, 0).hyphenate);

        // Fills: explicit ones, and none where there are none.
        QVERIFY(t->cell(0, 0).fill.type == Fill::Solid);
        QCOMPARE(dmlHex(t->cell(0, 0).fill.color), QStringLiteral("FF0000"));
        QCOMPARE(dmlHex(t->cell(2, 2).fill.color), QStringLiteral("00FF00"));
        QVERIFY(t->cell(0, 2).fill.isNone() && t->cell(1, 1).fill.isNone());

        // Borders: where they are, and none where they aren't or say so.
        QVERIFY(!t->cell(0, 0).border.bottom.isNone());
        QVERIFY(near(t->cell(0, 0).border.bottom.width, 2));
        QCOMPARE(dmlHex(t->cell(0, 0).border.bottom.color), QStringLiteral("0000FF"));
        QVERIFY(t->cell(0, 0).border.top.isNone() && t->cell(0, 0).border.left.isNone() && t->cell(0, 0).border.right.isNone());
        QVERIFY(near(t->cell(1, 2).border.right.width, 1));
        QCOMPARE(dmlHex(t->cell(1, 2).border.right.color), QStringLiteral("00FF00"));
        QVERIFY(t->cell(1, 2).border.right.dash == Stroke::DashLine);
        // A diagonal is a cell's diagonal line.
        QCOMPARE(t->cell(1, 2).diagonal, 1);
        QCOMPARE(dmlHex(t->cell(1, 2).diagonalLine.color), QStringLiteral("808080"));
        QCOMPARE(t->cell(1, 1).diagonal, 0);

        // The grid, not the frame, says how big the table is.
        auto big = dmlAs<TableItem>(readTable(env.element(dmlTableFrame(QStringLiteral("<a:tblPr firstRow=\"1\" bandRow=\"1\"/>"), grid, rows, 10, 20, 500, 500)), env.ctx));
        QVERIFY(big);
        QVERIFY2(dmlRectIs(big->rect, 10, 20, 300, 90), qPrintable(dmlRectText(big->rect)));
        QVERIFY(big->header && big->banded);
    }

    // The table styles part gives the cells their fills, borders, and text; what a cell says itself wins.
    void dmlTableAppliesItsStyleToTheCells()
    {
        const QString ln = QStringLiteral("<a:ln w=\"%1\"><a:solidFill><a:srgbClr val=\"%2\"/></a:solidFill></a:ln>");
        const QString id = QStringLiteral("{11111111-1111-1111-1111-111111111111}");
        const QString styles = QStringLiteral(
            "<a:tblStyleLst %1 def=\"%2\"><a:tblStyle styleId=\"%2\" styleName=\"Test Style\">"
            "<a:wholeTbl><a:tcTxStyle><a:fontRef idx=\"minor\"><a:prstClr val=\"black\"/></a:fontRef><a:srgbClr val=\"202020\"/></a:tcTxStyle>"
            "<a:tcStyle><a:tcBdr><a:left>%3</a:left><a:right>%3</a:right><a:top>%3</a:top><a:bottom>%3</a:bottom><a:insideH>%4</a:insideH><a:insideV>%5</a:insideV></a:tcBdr>"
            "<a:fill>%6</a:fill></a:tcStyle></a:wholeTbl>"
            "<a:band1H><a:tcStyle><a:tcBdr/><a:fill>%7</a:fill></a:tcStyle></a:band1H><a:band2H><a:tcStyle><a:tcBdr/></a:tcStyle></a:band2H>"
            "<a:lastRow><a:tcTxStyle b=\"on\"><a:fontRef idx=\"minor\"/><a:srgbClr val=\"FFFF00\"/></a:tcTxStyle><a:tcStyle><a:tcBdr><a:top>%8</a:top></a:tcBdr><a:fill>%9</a:fill></a:tcStyle></a:lastRow>"
            "<a:firstCol><a:tcTxStyle b=\"on\" i=\"on\"><a:fontRef idx=\"minor\"/><a:srgbClr val=\"FFFFFF\"/></a:tcTxStyle><a:tcStyle><a:tcBdr/></a:tcStyle></a:firstCol>"
            "<a:firstRow><a:tcTxStyle b=\"on\"><a:fontRef idx=\"minor\"/><a:srgbClr val=\"FFFFFF\"/></a:tcTxStyle><a:tcStyle><a:tcBdr><a:bottom>%10</a:bottom></a:tcBdr><a:fill>%11</a:fill></a:tcStyle></a:firstRow>"
            "</a:tblStyle></a:tblStyleLst>")
            .arg(dmlNamespaces(), id, ln.arg(12700).arg(QStringLiteral("FF0000")), ln.arg(6350).arg(QStringLiteral("00FF00")), ln.arg(6350).arg(QStringLiteral("00AA00")),
                 dmlSolid(QStringLiteral("EEEEEE")), dmlSolid(QStringLiteral("DDDDDD")), ln.arg(38100).arg(QStringLiteral("FFFF00")), dmlSolid(QStringLiteral("444444")),
                 ln.arg(38100).arg(QStringLiteral("0000FF")), dmlSolid(QStringLiteral("333333")));
        DmlEnv env;
        dmlUseTableStyles(env, styles.toUtf8());
        // Three columns, four rows; the cell at row 2, column 2 has a yellow fill and no bottom border of its own.
        QString rows;
        for (int r = 0; r < 4; ++r) {
            rows += QStringLiteral("<a:tr h=\"%1\">").arg(emu(20));
            for (int c = 0; c < 3; ++c)
                rows += dmlTc(QStringLiteral("r%1c%2").arg(r).arg(c), QString(), r == 2 && c == 2 ? dmlSolid(QStringLiteral("FFFF00")) + QStringLiteral("<a:lnB><a:noFill/></a:lnB>") : QString());
            rows += QStringLiteral("</a:tr>");
        }
        const QString grid = QStringLiteral("<a:tblGrid><a:gridCol w=\"%1\"/><a:gridCol w=\"%1\"/><a:gridCol w=\"%1\"/></a:tblGrid>").arg(emu(80));
        const QString tblPr = QStringLiteral("<a:tblPr firstRow=\"1\" bandRow=\"1\" firstCol=\"1\" lastRow=\"1\"><a:tableStyleId>%1</a:tableStyleId></a:tblPr>").arg(id);
        auto t = dmlAs<TableItem>(readTable(env.element(dmlTableFrame(tblPr, grid, rows, 0, 0, 240, 80)), env.ctx));
        QVERIFY(t);
        QCOMPARE(t->format, id);
        QVERIFY(t->header && t->banded);

        const auto fill = [&](int r, int c) { return dmlHex(t->cell(r, c).fill.color); };
        QCOMPARE(fill(0, 1), QStringLiteral("333333"));       // the header row
        QCOMPARE(fill(1, 1), QStringLiteral("DDDDDD"));       // the first band
        QCOMPARE(fill(2, 1), QStringLiteral("EEEEEE"));       // the second has the table's
        QCOMPARE(fill(3, 1), QStringLiteral("444444"));       // the last row
        QCOMPARE(fill(1, 0), QStringLiteral("DDDDDD"));       // the first column says nothing of fill
        QCOMPARE(fill(2, 2), QStringLiteral("FFFF00"));       // the cell's own

        const auto border = [&](const jp::Stroke &s) { return s.isNone() ? QStringLiteral("none") : QStringLiteral("%1 %2").arg(s.width).arg(dmlHex(s.color)); };
        // The outside of the table, the edges between cells, and the edge a region of the style has (on both sides of it).
        QCOMPARE(border(t->cell(0, 0).border.left), QStringLiteral("1 FF0000"));
        QCOMPARE(border(t->cell(2, 2).border.right), QStringLiteral("1 FF0000"));
        QCOMPARE(border(t->cell(0, 1).border.top), QStringLiteral("1 FF0000"));
        QCOMPARE(border(t->cell(3, 1).border.bottom), QStringLiteral("1 FF0000"));
        QCOMPARE(border(t->cell(2, 0).border.right), QStringLiteral("0.5 00AA00"));
        QCOMPARE(border(t->cell(2, 1).border.left), QStringLiteral("0.5 00AA00"));
        QCOMPARE(border(t->cell(1, 1).border.bottom), QStringLiteral("0.5 00FF00"));
        QCOMPARE(border(t->cell(2, 1).border.top), QStringLiteral("0.5 00FF00"));
        QCOMPARE(border(t->cell(0, 1).border.bottom), QStringLiteral("3 0000FF"));
        QCOMPARE(border(t->cell(1, 1).border.top), QStringLiteral("3 0000FF"));
        QCOMPARE(border(t->cell(3, 1).border.top), QStringLiteral("3 FFFF00"));
        QCOMPARE(border(t->cell(2, 1).border.bottom), QStringLiteral("3 FFFF00"));
        // A border a cell turns off is off on both sides.
        QCOMPARE(border(t->cell(2, 2).border.bottom), QStringLiteral("none"));
        QCOMPARE(border(t->cell(3, 2).border.top), QStringLiteral("none"));

        // The text: the style's color and weight, in the stories.
        const auto format = [&](int r, int c) { return env.ctx.storyDoc(t->cell(r, c).storyId)->begin().begin().fragment().charFormat(); };
        QCOMPARE(hexOf(format(0, 1).foreground().color()), QStringLiteral("FFFFFF"));
        QCOMPARE(format(0, 1).fontWeight(), int(QFont::Bold));
        QCOMPARE(hexOf(format(2, 1).foreground().color()), QStringLiteral("202020"));
        QVERIFY(format(2, 1).fontWeight() < int(QFont::Bold) && !format(2, 1).fontItalic());
        QCOMPARE(hexOf(format(3, 1).foreground().color()), QStringLiteral("FFFF00"));
        QCOMPARE(format(3, 1).fontWeight(), int(QFont::Bold));
        QVERIFY(format(1, 0).fontWeight() == int(QFont::Bold) && format(1, 0).fontItalic());
        QCOMPARE(hexOf(format(1, 0).foreground().color()), QStringLiteral("FFFFFF"));
        QVERIFY(format(1, 1).fontWeight() < int(QFont::Bold));

        // Written, the table has all it got from its style in the cells themselves: read where the style isn't
        // known, the cells are the same.
        DmlEnv back;
        auto u = dmlAs<TableItem>(readTable(back.element(dmlWriteTable(*t, env, 2)), back.ctx));
        QVERIFY(u);
        QCOMPARE(u->format, id);
        for (int r = 0; r < t->rows; ++r)
            for (int c = 0; c < t->cols; ++c) {
                const QByteArray at = QStringLiteral("row %1, column %2").arg(r).arg(c).toLatin1();
                QVERIFY2(u->cell(r, c).fill == t->cell(r, c).fill, at);
                const jp::CellBorder &a = t->cell(r, c).border, &b = u->cell(r, c).border;
                QVERIFY2(a.top == b.top && a.bottom == b.bottom && a.left == b.left && a.right == b.right, at);
            }
    }

    // A style the file doesn't define is approximated by the usual one (an accent header, banded rows in tints, white
    // borders); the two plain styles are what they say.
    void dmlTableApproximatesStylesItDoesNotHave()
    {
        const auto table = [&](DmlEnv &env, const QString &style) {
            const QString rows = QStringLiteral("<a:tr h=\"%1\">%2%2</a:tr><a:tr h=\"%1\">%2%2</a:tr><a:tr h=\"%1\">%2%2</a:tr>").arg(emu(20), dmlTc(QStringLiteral("x")));
            const QString grid = QStringLiteral("<a:tblGrid><a:gridCol w=\"%1\"/><a:gridCol w=\"%1\"/></a:tblGrid>").arg(emu(80));
            const QString tblPr = QStringLiteral("<a:tblPr firstRow=\"1\" bandRow=\"1\">%1</a:tblPr>").arg(style.isEmpty() ? QString() : QStringLiteral("<a:tableStyleId>%1</a:tableStyleId>").arg(style));
            return dmlAs<TableItem>(readTable(env.element(dmlTableFrame(tblPr, grid, rows, 0, 0, 160, 60)), env.ctx));
        };
        const QString medium = QStringLiteral("{5C22544A-7EE6-4342-B048-85BDC9FD1C3A}");
        // With no table styles part at all, and with one that has other styles, for the usual style's id and another's.
        for (int variant = 0; variant < 4; ++variant) {
            const QString id = variant % 2 ? QStringLiteral("{ABCDEF01-2345-6789-ABCD-EF0123456789}") : medium;
            DmlEnv env;
            if (variant >= 2)
                dmlUseTableStyles(env, QStringLiteral("<a:tblStyleLst %1 def=\"{22222222-2222-2222-2222-222222222222}\"><a:tblStyle styleId=\"{22222222-2222-2222-2222-222222222222}\" styleName=\"Other\">"
                                                      "<a:wholeTbl><a:tcStyle><a:tcBdr/></a:tcStyle></a:wholeTbl></a:tblStyle></a:tblStyleLst>").arg(dmlNamespaces()).toUtf8());
            auto t = table(env, id);
            QVERIFY(t);
            QCOMPARE(t->format, id);
            QCOMPARE(dmlHex(t->cell(0, 0).fill.color), QStringLiteral("4472C4"));      // accent 1
            QVERIFY(t->cell(1, 0).fill.type == Fill::Solid && t->cell(2, 0).fill.type == Fill::Solid);
            QVERIFY(t->cell(1, 0).fill.color != t->cell(2, 0).fill.color);             // banded
            QVERIFY(t->cell(1, 0).fill.color.rgbValue().lightness() > QColor(0x44, 0x72, 0xC4).lightness());
            QCOMPARE(dmlHex(t->cell(1, 0).border.left.color), QStringLiteral("FEFEFE"));   // the theme's light color
            QVERIFY(!t->cell(1, 0).border.bottom.isNone() && !t->cell(1, 1).border.left.isNone());
            const auto format = env.ctx.storyDoc(t->cell(0, 0).storyId)->begin().begin().fragment().charFormat();
            QCOMPARE(format.fontWeight(), int(QFont::Bold));
            QCOMPARE(hexOf(format.foreground().color()), QStringLiteral("FEFEFE"));
            QVERIFY(env.ctx.storyDoc(t->cell(1, 0).storyId)->begin().begin().fragment().charFormat().fontWeight() < int(QFont::Bold));
        }
        // "No Style, No Grid" draws nothing; "No Style, Table Grid" draws a thin line around every cell.
        DmlEnv env;
        auto plain = table(env, QStringLiteral("{2D5ABB26-0587-4C30-8999-92F81FD0307C}"));
        QVERIFY(plain);
        for (const auto &c : plain->cells) {
            QVERIFY(c.fill.isNone());
            QVERIFY(c.border.top.isNone() && c.border.left.isNone());
        }
        auto grid = table(env, QStringLiteral("{5940675A-B579-460E-94D1-54222C63F5DA}"));
        QVERIFY(grid);
        for (const auto &c : grid->cells) {
            QVERIFY(c.fill.isNone());
            QVERIFY(!c.border.top.isNone() && !c.border.bottom.isNone() && !c.border.left.isNone() && !c.border.right.isNone());
            QVERIFY(near(c.border.top.width, 1));
        }
        // No style at all: nothing from one.
        auto none = table(env, QString());
        QVERIFY(none);
        for (const auto &c : none->cells) QVERIFY(c.fill.isNone() && c.border.top.isNone());
    }

    // What isn't a table, or is too much of one, is not read; spans that go past the table stop at its edge.
    void dmlTableLeavesWhatItCannotRead()
    {
        DmlEnv env;
        const QString grid = QStringLiteral("<a:tblGrid><a:gridCol w=\"%1\"/><a:gridCol w=\"%1\"/></a:tblGrid>").arg(emu(50));
        const QString row = QStringLiteral("<a:tr h=\"%1\">%2%2</a:tr>").arg(emu(20), dmlTc(QStringLiteral("x")));
        QVERIFY(!readTable(QDomElement(), env.ctx));
        QVERIFY(!isTableFrame(QDomElement()));
        // A frame of something else, a table with no columns, one with no rows.
        const QString chart = QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"2\" name=\"Chart\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                                             "<p:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"100\" cy=\"100\"/></p:xfrm><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"/></a:graphic></p:graphicFrame>");
        QVERIFY(!isTableFrame(env.element(chart)));
        QVERIFY(!readTable(env.element(chart), env.ctx));
        QVERIFY(isTableFrame(env.element(dmlTableFrame(QString(), grid, row, 0, 0, 100, 20))));
        // The strict form of the standard names the table by another URI.
        QString strict = dmlTableFrame(QString(), grid, row, 0, 0, 100, 20);
        strict.replace(QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/table"), QStringLiteral("http://purl.oclc.org/ooxml/drawingml/table"));
        QVERIFY(readTable(env.element(strict), env.ctx));
        QVERIFY(!readTable(env.element(dmlTableFrame(QString(), QStringLiteral("<a:tblGrid/>"), row, 0, 0, 100, 20)), env.ctx));
        QVERIFY(!readTable(env.element(dmlTableFrame(QString(), grid, QString(), 0, 0, 100, 20)), env.ctx));
        // More columns than the table keeps.
        QString wide = QStringLiteral("<a:tblGrid>");
        for (int i = 0; i < 300; ++i) wide += QStringLiteral("<a:gridCol w=\"12700\"/>");
        QVERIFY(!readTable(env.element(dmlTableFrame(QString(), wide + QStringLiteral("</a:tblGrid>"), row, 0, 0, 100, 20)), env.ctx));
        // Spans past the edge, a span that covers another span's cell, and a row with too few or too many cells.
        const QString rows = QStringLiteral("<a:tr h=\"254000\">") + dmlTc(QStringLiteral("a"), QStringLiteral("gridSpan=\"99\" rowSpan=\"99\"")) + dmlTc(QStringLiteral("b")) + QStringLiteral("</a:tr>")
            + QStringLiteral("<a:tr h=\"254000\">") + dmlTc(QStringLiteral("c"), QStringLiteral("gridSpan=\"2\"")) + dmlTc(QStringLiteral("d"), QStringLiteral("rowSpan=\"2\" gridSpan=\"2\"")) + dmlTc(QStringLiteral("e")) + QStringLiteral("</a:tr>")
            + QStringLiteral("<a:tr h=\"254000\">") + dmlTc(QStringLiteral("f")) + QStringLiteral("</a:tr>");
        auto t = dmlAs<TableItem>(readTable(env.element(dmlTableFrame(QString(), grid, rows, 0, 0, 100, 60)), env.ctx));
        QVERIFY(t);
        QCOMPARE(t->rows, 3);
        QCOMPARE(t->cols, 2);
        QCOMPARE(t->cell(0, 0).colSpan, 2);
        QCOMPARE(t->cell(0, 0).rowSpan, 3);
        QVERIFY(t->cell(0, 1).covered && t->cell(1, 0).covered && t->cell(1, 1).covered && t->cell(2, 0).covered && t->cell(2, 1).covered);
        QCOMPARE(t->cell(1, 0).colSpan, 1);   // covered, so what it said of itself is gone
        QCOMPARE(int(t->cells.size()), 6);
        for (const auto &c : t->cells) QVERIFY(!c.storyId.isEmpty());
        // A row that has too few cells is filled out with empty ones.
        auto few = dmlAs<TableItem>(readTable(env.element(dmlTableFrame(QString(), grid, QStringLiteral("<a:tr h=\"254000\">") + dmlTc(QStringLiteral("a")) + QStringLiteral("</a:tr>"), 0, 0, 100, 20)), env.ctx));
        QVERIFY(few);
        QCOMPARE(few->cols, 2);
        QVERIFY(!few->cell(0, 1).covered && few->cell(0, 1).rowSpan == 1);
        QVERIFY(env.ctx.storyDoc(few->cell(0, 1).storyId)->toPlainText().isEmpty());
    }

    // Written and read again, a table is the table it was: its grid, merges, fills, borders, margins, anchors, and
    // text, and the style it names; and it is a frame with a table in it, as the presentation program writes one.
    void dmlTableWritesWhatItRead()
    {
        const QString ln = QStringLiteral("<a:solidFill><a:srgbClr val=\"%1\"/></a:solidFill>");
        const QString rows =
            QStringLiteral("<a:tr h=\"%1\">").arg(emu(30))
            + dmlTc(QStringLiteral("Total"), QStringLiteral("gridSpan=\"2\""), QStringLiteral("<a:lnT><a:noFill/></a:lnT><a:lnB w=\"25400\">%1</a:lnB>").arg(ln.arg(QStringLiteral("0000FF"))) + dmlSolid(QStringLiteral("FF0000")),
                    QStringLiteral("marL=\"127000\" marR=\"63500\" marT=\"38100\" marB=\"25400\" anchor=\"ctr\""), QStringLiteral("b=\"1\""))
            + dmlTc(QString(), QStringLiteral("hMerge=\"1\"")) + dmlTc(QStringLiteral("C0")) + QStringLiteral("</a:tr>")
            + QStringLiteral("<a:tr h=\"%1\">").arg(emu(25))
            + dmlTc(QStringLiteral("tall"), QStringLiteral("rowSpan=\"2\""), QString(), QStringLiteral("anchor=\"b\" vert=\"vert\""))
            + dmlTc(QStringLiteral("E1"), QString(), QStringLiteral("<a:noFill/>"))
            + dmlTc(QStringLiteral("F1"), QString(), QStringLiteral("<a:lnR w=\"12700\" cmpd=\"dbl\">%1<a:prstDash val=\"dash\"/></a:lnR><a:lnBlToTr w=\"12700\">%2</a:lnBlToTr>").arg(ln.arg(QStringLiteral("00FF00")), ln.arg(QStringLiteral("808080"))))
            + QStringLiteral("</a:tr>")
            + QStringLiteral("<a:tr h=\"%1\">").arg(emu(20))
            + dmlTc(QString(), QStringLiteral("vMerge=\"1\"")) + dmlTc(QStringLiteral("H2")) + dmlTc(QStringLiteral("I2"), QString(), dmlSolid(QStringLiteral("00FF00")))
            + QStringLiteral("</a:tr>");
        const QString grid = QStringLiteral("<a:tblGrid><a:gridCol w=\"%1\"/><a:gridCol w=\"%2\"/><a:gridCol w=\"%3\"/></a:tblGrid>").arg(emu(100), emu(150), emu(50));
        const QString tblPr = QStringLiteral("<a:tblPr firstRow=\"1\" bandRow=\"1\"><a:tableStyleId>{5C22544A-7EE6-4342-B048-85BDC9FD1C3A}</a:tableStyleId></a:tblPr>");
        DmlEnv env;
        auto t = dmlAs<TableItem>(readTable(env.element(dmlTableFrame(tblPr, grid, rows, 36, 72, 300, 75)), env.ctx));
        QVERIFY(t);
        t->role = QStringLiteral("ph:tbl:3");

        int next = 0;
        const QString xml = dmlWriteTable(*t, env, 7, &next);
        QCOMPARE(next, 8);
        QVERIFY2(xml.startsWith(QStringLiteral("<p:graphicFrame>")), qPrintable(xml));
        QVERIFY(xml.contains(QStringLiteral("<p:cNvPr id=\"7\"")));
        QVERIFY(xml.contains(QStringLiteral("<a:graphicFrameLocks noGrp=\"1\"/>")));
        QVERIFY(xml.contains(QStringLiteral("<p:ph type=\"tbl\" idx=\"3\"/>")));
        QVERIFY(xml.contains(QStringLiteral("<a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/table\">")));
        QVERIFY(xml.contains(QStringLiteral("<a:tblPr firstRow=\"1\" bandRow=\"1\">")));
        QVERIFY(xml.contains(QStringLiteral("<a:tableStyleId>{5C22544A-7EE6-4342-B048-85BDC9FD1C3A}</a:tableStyleId>")));
        QVERIFY(xml.contains(QStringLiteral("<a:gridCol w=\"1270000\"/><a:gridCol w=\"1905000\"/><a:gridCol w=\"635000\"/>")));
        QVERIFY(xml.contains(QStringLiteral("<a:tr h=\"381000\">")) && xml.contains(QStringLiteral("<a:tr h=\"317500\">")));
        QVERIFY(xml.contains(QStringLiteral("gridSpan=\"2\"")) && xml.contains(QStringLiteral("hMerge=\"1\"")));
        QVERIFY(xml.contains(QStringLiteral("rowSpan=\"2\"")) && xml.contains(QStringLiteral("vMerge=\"1\"")));
        QVERIFY(xml.contains(QStringLiteral("<a:off x=\"457200\" y=\"914400\"/><a:ext cx=\"3810000\" cy=\"952500\"/>")));

        DmlEnv back;
        auto u = dmlAs<TableItem>(readTable(back.element(xml), back.ctx));
        QVERIFY2(u, qPrintable(xml));
        QCOMPARE(u->rows, t->rows);
        QCOMPARE(u->cols, t->cols);
        QCOMPARE(u->colW, t->colW);
        QCOMPARE(u->rowH, t->rowH);
        QCOMPARE(u->rect, t->rect);
        QCOMPARE(u->name, t->name);
        QCOMPARE(u->altText, t->altText);
        QCOMPARE(u->format, t->format);
        QCOMPARE(u->header, t->header);
        QCOMPARE(u->banded, t->banded);
        for (int r = 0; r < t->rows; ++r)
            for (int c = 0; c < t->cols; ++c) {
                const QByteArray at = QStringLiteral("row %1, column %2").arg(r).arg(c).toLatin1();
                const jp::TableCell &a = t->cell(r, c), &b = u->cell(r, c);
                QVERIFY2(a.rowSpan == b.rowSpan && a.colSpan == b.colSpan && a.covered == b.covered, at);
                QVERIFY2(a.fill == b.fill, at);
                QVERIFY2(a.border.top == b.border.top && a.border.bottom == b.border.bottom && a.border.left == b.border.left && a.border.right == b.border.right, at);
                QVERIFY2(a.diagonal == b.diagonal && a.diagonalLine == b.diagonalLine, at);
                QVERIFY2(a.margins == b.margins && a.valign == b.valign && a.vertical == b.vertical, at);
            }
        // The style's look came back as the cells' own, so none of it is read from the style twice.
        QCOMPARE(dmlHex(u->cell(0, 2).fill.color), dmlHex(t->cell(0, 2).fill.color));
        QVERIFY(u->cell(1, 1).fill.isNone());
        // The text, once the drawing writer writes text.
        const auto texts = [](DmlEnv &e, const TableItem &item) {
            QStringList out;
            for (const auto &c : item.cells) out << e.ctx.storyDoc(c.storyId)->toPlainText();
            return out;
        };
        if (!dmlWriterWritesText()) QEXPECT_FAIL("", "drawing writer pending", Continue);
        QCOMPARE(texts(back, *u), texts(env, *t));
        if (!dmlWriterWritesText()) QEXPECT_FAIL("", "drawing writer pending", Continue);
        QCOMPARE(back.ctx.storyDoc(u->cell(0, 0).storyId)->begin().begin().fragment().charFormat().fontWeight(), int(QFont::Bold));

        // A table with nothing in it is nothing to write.
        TableItem empty;
        int used = 0;
        QVERIFY(dmlWriteTable(empty, env, 11, &used).isEmpty());
        QCOMPARE(used, 11);

        // A table made here, with a style of ours by name, names none; its role is kept only as a placeholder's.
        auto own = std::make_shared<TableItem>();
        own->rows = own->cols = 1;
        own->colW = {72};
        own->rowH = {36};
        own->cells.resize(1);
        own->format = QStringLiteral("Basic 1");
        own->header = false;
        own->banded = false;
        own->rect = QRectF(10, 20, 72, 36);
        const QString plain = dmlWriteTable(*own, env, 2);
        QVERIFY2(!plain.contains(QStringLiteral("tableStyleId")), qPrintable(plain));
        QVERIFY(!plain.contains(QStringLiteral("<p:ph")));
        QVERIFY(plain.contains(QStringLiteral("<a:tblPr/>")));
        QVERIFY(plain.contains(QStringLiteral("name=\"Table ")));
        DmlEnv again;
        auto v = dmlAs<TableItem>(readTable(again.element(plain), again.ctx));
        QVERIFY(v);
        QVERIFY(v->format.isEmpty() && !v->header && !v->banded);
        QVERIFY(v->cell(0, 0).fill.isNone() && v->cell(0, 0).border.top.isNone());
    }

private:

    // The presentation's table styles part, linked from its main part, as `env`'s package.
    static void dmlUseTableStyles(DmlEnv &env, const QByteArray &xml)
    {
        PackageWriter w;
        w.addPart(QStringLiteral("/ppt/presentation.xml"), "<p:presentation/>", kPresentationType);
        w.addPart(QStringLiteral("/ppt/tableStyles.xml"), xml, QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.tableStyles+xml"));
        w.addRelationship(QStringLiteral("/"), kRelBase + "officeDocument", QStringLiteral("/ppt/presentation.xml"));
        w.addRelationship(QStringLiteral("/ppt/presentation.xml"), kRelBase + "tableStyles", QStringLiteral("/ppt/tableStyles.xml"));
        QVERIFY(env.package.read(w.finish()));
        env.ctx.package = &env.package;
        env.ctx.part = QStringLiteral("/ppt/slides/slide1.xml");
    }

    // The table as the writer writes it, with its stories from `from`.
    static QString dmlWriteTable(const TableItem &item, DmlEnv &from, int firstId, int *nextId = nullptr)
    {
        DrawingWriteContext wctx;
        wctx.nextShapeId = firstId;
        wctx.storyDoc = [&from](const QString &id) -> const QTextDocument * { return from.ctx.storyDoc(id); };
        QByteArray xml;
        QXmlStreamWriter w(&xml);
        writeTable(w, item, wctx);
        if (nextId) *nextId = wctx.nextShapeId;
        return QString::fromUtf8(xml);
    }

    // Whether the drawing writer writes the text of a story yet.
    static bool dmlWriterWritesText()
    {
        QTextDocument doc;
        doc.setPlainText(QStringLiteral("x"));
        QByteArray xml;
        QXmlStreamWriter w(&xml);
        DrawingWriteContext ctx;
        writeParagraphs(w, &doc, ctx);
        return xml.contains("<a:t>");
    }
};

JP_TEST_CLASS(OfficeDmlTableTests)
#include "tst_office_dmltable.moc"
