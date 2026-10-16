// JeffDeck's tests of tables and picture cropping.
#include "deckhelpers.h"

class DeckTableTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

private:
    // The colors of the presentation's theme, as a table style takes them.
    static office::Theme tableThemeOf(const Document &d)
    {
        office::Theme theme;
        const deck::DeckTheme t = deck::themeOf(d);
        for (int i = 0; i < office::Theme::ColorCount; ++i) theme.colors[i] = t.colors[i];
        return theme;
    }

    // The whole document as an undo step keeps it: equal before and after an undo when the undo took everything back.
    static QByteArray documentJson(const Editor *ed) { return QJsonDocument(ed->doc()->toJson()).toJson(QJsonDocument::Compact); }

    static PictureItem *thePicture(Editor *ed)
    {
        const auto &items = ed->doc()->pages[ed->currentPage()]->items;
        for (auto it = items.rbegin(); it != items.rend(); ++it)
            if ((*it)->type() == ItemType::Picture) return static_cast<PictureItem *>(it->get());
        return nullptr;
    }

    // A table on the window's slide, a letter and a number in each cell (A1, A2, ... B1), selected as an object.
    static void tableOnSlide(DeckWindow &w, int rows, int cols)
    {
        w.insertTable(rows, cols);
        TableItem *t = theTable(w.editor());
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c)
                QTextCursor(w.editor()->doc()->storyDoc(t->cell(r, c).storyId)).insertText(QStringLiteral("%1%2").arg(QChar('A' + r)).arg(c + 1));   // in the cell's own look
        w.editor()->endTextEdit();
        w.editor()->select(t->id);
    }

    static QString cellText(Editor *ed, int r, int c) { return ed->doc()->storyDoc(theTable(ed)->cell(r, c).storyId)->toPlainText(); }

    static bool cellIsBold(Editor *ed, int r, int c)
    {
        QTextCursor cur(ed->doc()->storyDoc(theTable(ed)->cell(r, c).storyId));
        cur.select(QTextCursor::Document);
        return cur.charFormat().fontWeight() >= QFont::DemiBold;
    }

    // The presentation as a saved .pptx (or .odp) gives it back; null when it isn't written.
    static std::unique_ptr<Document> savedAndRead(const Document &d, bool odp)
    {
        QString err;
        const QByteArray bytes = odp ? deck::exportOdp(d, false, &err) : deck::exportPresentation(d, &err);
        return bytes.isEmpty() ? nullptr : odp ? deck::importOdp(bytes, &err) : deck::importPresentation(bytes, &err);
    }

    static const TableItem *firstTable(const Document &d)
    {
        for (const auto &p : d.pages)
            for (const auto &it : p->items)
                if (it->type() == ItemType::Table) return static_cast<const TableItem *>(it.get());
        return nullptr;
    }

    static QString savedSlideXml(const Document &d)
    {
        QMap<QString, QByteArray> zip;
        QString err;
        if (!readZip(deck::exportPresentation(d, &err), zip, &err)) return QString();
        return QString::fromUtf8(zip.value(QStringLiteral("ppt/slides/slide1.xml")));
    }

private Q_SLOTS:
    // Insert Above, Below, Left, and Right put a row or column beside the one the cursor is in; Delete takes the rows or columns of the
    // selected cells away, keeps one of each, and Delete Table removes the table. Each is one step that undoes.
    void tableToolsInsertAndDeleteRowsAndColumns()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        tableOnSlide(w, 3, 4);
        const QString id = theTable(ed)->id;
        ed->selectCells(id, 1, 1, 1, 1);
        QVERIFY(command(w, "tbl.insAbove"));
        QCOMPARE(theTable(ed)->rows, 4);
        QCOMPARE(cellText(ed, 1, 0), QString());   // the new row is row 1; B moved down
        QCOMPARE(cellText(ed, 2, 0), QStringLiteral("B1"));
        ed->undo();
        QCOMPARE(theTable(ed)->rows, 3);
        ed->selectCells(id, 1, 1, 1, 1);
        QVERIFY(command(w, "tbl.insBelow"));
        QCOMPARE(theTable(ed)->rows, 4);
        QCOMPARE(cellText(ed, 1, 0), QStringLiteral("B1"));
        QCOMPARE(cellText(ed, 2, 0), QString());
        ed->undo();
        ed->selectCells(id, 1, 1, 1, 1);
        QVERIFY(command(w, "tbl.insLeft"));
        QCOMPARE(theTable(ed)->cols, 5);
        QCOMPARE(cellText(ed, 0, 1), QString());
        QCOMPARE(cellText(ed, 0, 2), QStringLiteral("A2"));
        ed->undo();
        ed->selectCells(id, 1, 1, 1, 1);
        QVERIFY(command(w, "tbl.insRight"));
        QCOMPARE(theTable(ed)->cols, 5);
        QCOMPARE(cellText(ed, 0, 1), QStringLiteral("A2"));
        QCOMPARE(cellText(ed, 0, 2), QString());
        ed->undo();
        QCOMPARE(theTable(ed)->cols, 4);
        ed->selectCells(id, 1, 1, 1, 2);
        QVERIFY(command(w, "tbl.delRow"));
        QCOMPARE(theTable(ed)->rows, 2);
        QCOMPARE(cellText(ed, 1, 0), QStringLiteral("C1"));
        ed->undo();
        QCOMPARE(theTable(ed)->rows, 3);
        ed->selectCells(id, 0, 1, 2, 2);
        QVERIFY(command(w, "tbl.delCol"));
        QCOMPARE(theTable(ed)->cols, 2);
        QCOMPARE(cellText(ed, 0, 1), QStringLiteral("A4"));
        ed->undo();
        QCOMPARE(theTable(ed)->cols, 4);
        // A table keeps a row and a column: all the rows can't go.
        ed->selectCells(id, 0, 0, 2, 0);
        QVERIFY(command(w, "tbl.delRow"));
        QCOMPARE(theTable(ed)->rows, 3);
        const int items = int(ed->doc()->pages[0]->items.size());
        ed->select(id);
        QVERIFY(command(w, "tbl.delTable"));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), items - 1);
        ed->undo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), items);
    }

    // Merge Cells joins the selected cells into one with all their words, Split Cells makes them cells again, each as one step; the merged
    // cell is saved as the formats write one and read back as it was.
    void tableToolsMergeAndSplitCells()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        tableOnSlide(w, 3, 4);
        const QString id = theTable(ed)->id;
        ed->selectCells(id, 0, 0, 1, 1);
        QVERIFY(command(w, "tbl.merge"));
        {
            const TableItem *t = theTable(ed);
            QCOMPARE(t->cell(0, 0).rowSpan, 2);
            QCOMPARE(t->cell(0, 0).colSpan, 2);
            QVERIFY(t->cell(0, 1).covered && t->cell(1, 0).covered && t->cell(1, 1).covered);
            QVERIFY(!t->cell(0, 2).covered && !t->cell(2, 0).covered);
            QCOMPARE(cellText(ed, 0, 0).split(QLatin1Char('\n')), (QStringList{QStringLiteral("A1"), QStringLiteral("A2"), QStringLiteral("B1"), QStringLiteral("B2")}));
        }
        for (bool odp : {false, true}) {
            auto back = savedAndRead(*ed->doc(), odp);
            QVERIFY(back);
            const TableItem *bt = firstTable(*back);
            QVERIFY(bt);
            QCOMPARE(bt->cell(0, 0).rowSpan, 2);
            QCOMPARE(bt->cell(0, 0).colSpan, 2);
            QVERIFY(bt->cell(0, 1).covered && bt->cell(1, 0).covered && bt->cell(1, 1).covered);
            QVERIFY(!bt->cell(0, 2).covered && !bt->cell(2, 0).covered);
            QVERIFY(back->storyDoc(bt->cell(0, 0).storyId)->toPlainText().contains(QStringLiteral("B2")));
        }
        const QString slide = savedSlideXml(*ed->doc());
        QVERIFY(slide.contains(QStringLiteral("gridSpan=\"2\"")));
        QVERIFY(slide.contains(QStringLiteral("rowSpan=\"2\"")));
        QVERIFY(slide.contains(QStringLiteral("hMerge=\"1\"")));
        QVERIFY(slide.contains(QStringLiteral("vMerge=\"1\"")));
        ed->undo();
        QCOMPARE(theTable(ed)->cell(0, 0).rowSpan, 1);
        QVERIFY(!theTable(ed)->cell(1, 1).covered);
        ed->redo();
        QVERIFY(theTable(ed)->cell(1, 1).covered);
        ed->selectCells(id, 0, 0, 0, 0);
        QVERIFY(command(w, "tbl.split"));
        QCOMPARE(theTable(ed)->cell(0, 0).rowSpan, 1);
        QCOMPARE(theTable(ed)->cell(0, 0).colSpan, 1);
        QVERIFY(!theTable(ed)->cell(0, 1).covered && !theTable(ed)->cell(1, 1).covered);
        // The words stay in the first cell, the others are empty; and every cell is as the table's style has it.
        QCOMPARE(cellText(ed, 0, 0).split(QLatin1Char('\n')).size(), 4);
        QCOMPARE(cellText(ed, 0, 1), QString());
        QCOMPARE(cellText(ed, 1, 1), QString());
        {
            office::TableStyleEffect fx;
            QVERIFY(office::tableStyleEffect(theTable(ed)->format, *theTable(ed), tableThemeOf(*ed->doc()), &fx));
            for (int r = 0; r < theTable(ed)->rows; ++r)
                for (int c = 0; c < theTable(ed)->cols; ++c)
                    QVERIFY2(theTable(ed)->cell(r, c).fill == fx.fill[size_t(r) * theTable(ed)->cols + c], qPrintable(QStringLiteral("%1,%2").arg(r).arg(c)));
        }
        ed->undo();
        QCOMPARE(theTable(ed)->cell(0, 0).colSpan, 2);
        // One cell on its own joins the cell on its right.
        ed->selectCells(id, 2, 0, 2, 0);
        QVERIFY(command(w, "tbl.merge"));
        QCOMPARE(theTable(ed)->cell(2, 0).colSpan, 2);
    }

    // Select Row, Column, and Table choose cells; Distribute makes the rows or columns the same size, and Row Height and Column Width set
    // the size of those of the selected cells.
    void tableToolsSelectAndSizeCells()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        tableOnSlide(w, 3, 4);
        TableItem *t = theTable(ed);
        const QString id = t->id;
        ed->beginChange(QStringLiteral("Size"));
        const double first = t->rowH[0];   // as tall as its words need (the font here or on any machine)
        t->rowH = {first, first + 30, first + 10};
        t->colW = {50, 100, 150, 200};
        t->syncRect();
        ed->endChange();
        ed->selectCells(id, 1, 2, 1, 2);
        QVERIFY(command(w, "tbl.selectRow"));
        QCOMPARE(ed->cellBlock().range, (CellRange{1, 0, 1, 3}));
        ed->selectCells(id, 1, 2, 1, 2);
        QVERIFY(command(w, "tbl.selectCol"));
        QCOMPARE(ed->cellBlock().range, (CellRange{0, 2, 2, 2}));
        QVERIFY(command(w, "tbl.selectTable"));
        QVERIFY(!ed->hasCellBlock());
        QCOMPARE(ed->selection(), QStringList{id});
        t = theTable(ed);
        const double height = t->rect.height(), width = t->rect.width();
        QVERIFY(command(w, "tbl.distributeRows"));
        QVERIFY(std::abs(theTable(ed)->rowH[0] - height / 3) < 0.01 && std::abs(theTable(ed)->rowH[2] - height / 3) < 0.01);
        QVERIFY(command(w, "tbl.distributeCols"));
        QVERIFY(std::abs(theTable(ed)->colW[0] - width / 4) < 0.01 && std::abs(theTable(ed)->colW[3] - width / 4) < 0.01);
        ed->undo();
        QCOMPARE(theTable(ed)->colW[0], 50.0);
        ed->undo();
        QCOMPARE(theTable(ed)->rowH[1], first + 30);
        // Typed sizes: the height of the rows, and the width of the columns, of the selected cells.
        ed->selectCells(id, 1, 1, 2, 2);
        setTableRowHeight(ed, first + 40);
        QCOMPARE(theTable(ed)->rowH[1], first + 40);
        QCOMPARE(theTable(ed)->rowH[2], first + 40);
        QCOMPARE(theTable(ed)->rowH[0], first);
        setTableColumnWidth(ed, 90);
        QCOMPARE(theTable(ed)->colW[1], 90.0);
        QCOMPARE(theTable(ed)->colW[2], 90.0);
        QCOMPARE(theTable(ed)->colW[3], 200.0);
        QCOMPARE(theTable(ed)->rect.width(), 50.0 + 90 + 90 + 200);
        ed->undo();
        QCOMPARE(theTable(ed)->colW[2], 150.0);
    }

    // Shading, Borders, alignment, text direction, and cell margins act on the selected cells; saved, each is written as the formats have it.
    void tableToolsBordersShadingAndAlignment()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        tableOnSlide(w, 3, 3);
        const QString id = theTable(ed)->id;
        ed->beginChange(QStringLiteral("Plain"));
        for (auto &cell : theTable(ed)->cells) {
            cell.fill = Fill();
            cell.border = CellBorder();
        }
        ed->endChange();
        ed->selectCells(id, 0, 0, 1, 1);
        QVERIFY(command(w, "border.outside"));
        const TableItem *t = theTable(ed);
        QVERIFY(!t->cell(0, 0).border.top.isNone() && !t->cell(0, 0).border.left.isNone());
        QVERIFY(t->cell(0, 0).border.bottom.isNone() && t->cell(0, 0).border.right.isNone());
        QVERIFY(!t->cell(1, 1).border.bottom.isNone() && !t->cell(1, 1).border.right.isNone());
        QVERIFY(t->cell(2, 2).border.top.isNone());
        QVERIFY(command(w, "border.insideH"));
        QVERIFY(!theTable(ed)->cell(0, 0).border.bottom.isNone() && !theTable(ed)->cell(1, 0).border.top.isNone());
        QVERIFY(theTable(ed)->cell(0, 0).border.right.isNone());
        QVERIFY(command(w, "border.insideV"));
        QVERIFY(!theTable(ed)->cell(0, 0).border.right.isNone());
        QVERIFY(command(w, "border.none"));
        QVERIFY(theTable(ed)->cell(0, 0).border.top.isNone() && theTable(ed)->cell(1, 1).border.right.isNone());
        ed->undo();
        QVERIFY(!theTable(ed)->cell(0, 0).border.right.isNone());
        ed->selectCells(id, 0, 0, 1, 1);
        QVERIFY(command(w, "border.all"));
        QVERIFY(!theTable(ed)->cell(1, 1).border.left.isNone());
        QVERIFY(theTable(ed)->cell(2, 2).border.left.isNone());
        // Shading: a color for the cells chosen.
        ed->selectCells(id, 2, 0, 2, 2);
        w.setCellShading(ColorRef::rgb(QColor(0x12, 0x34, 0x56)));
        QCOMPARE(theTable(ed)->cell(2, 1).fill.color.rgbValue(), QColor(0x12, 0x34, 0x56));
        QVERIFY(theTable(ed)->cell(1, 1).fill.isNone());
        // Alignment (up, middle, down), text direction, and margins, for the same cells.
        QVERIFY(command(w, "tbl.alignMiddle"));
        QCOMPARE(int(theTable(ed)->cell(2, 0).valign), int(VAlign::Middle));
        QCOMPARE(int(theTable(ed)->cell(0, 0).valign), int(VAlign::Top));
        QVERIFY(command(w, "tbl.alignBottom"));
        QCOMPARE(int(theTable(ed)->cell(2, 2).valign), int(VAlign::Bottom));
        QVERIFY(command(w, "tbl.direction"));
        QVERIFY(theTable(ed)->cell(2, 1).vertical && !theTable(ed)->cell(1, 1).vertical);
        QVERIFY(command(w, "tbl.marginWide"));
        QCOMPARE(theTable(ed)->cell(2, 1).margins, QMarginsF(14.4, 7.2, 14.4, 7.2));
        QVERIFY(theTable(ed)->cell(0, 1).margins != QMarginsF(14.4, 7.2, 14.4, 7.2));
        for (bool odp : {false, true}) {
            auto back = savedAndRead(*ed->doc(), odp);
            QVERIFY(back);
            const TableItem *bt = firstTable(*back);
            QVERIFY(bt);
            const char *format = odp ? "odp" : "pptx";
            QVERIFY2(!bt->cell(0, 0).border.top.isNone(), format);
            QVERIFY2(!bt->cell(1, 1).border.left.isNone(), format);
            QVERIFY2(bt->cell(2, 2).border.bottom.isNone() && bt->cell(2, 2).border.right.isNone(), format);   // a cell outside the ones that were lined
            QCOMPARE(bt->cell(2, 1).fill.color.rgbValue(), QColor(0x12, 0x34, 0x56));
            QCOMPARE(int(bt->cell(2, 2).valign), int(VAlign::Bottom));
            QVERIFY2(bt->cell(2, 1).vertical, format);
        }
        const QString slide = savedSlideXml(*ed->doc());
        QVERIFY(QRegularExpression(QStringLiteral("<a:tcPr[^>]*anchor=\"b\"")).match(slide).hasMatch());
        QVERIFY(QRegularExpression(QStringLiteral("<a:tcPr[^>]*vert=\"vert\"")).match(slide).hasMatch());
        QVERIFY(slide.contains(QStringLiteral("marL=\"182880\"")));   // 14.4 points
        QVERIFY(slide.contains(QStringLiteral("<a:lnL")) && slide.contains(QStringLiteral("123456")));
    }

    // A new table has the program's usual style (Medium Style 2, Accent 1), and the Table Design tab's boxes turn its parts on and off:
    // each is one step that undoes, what the person shaded by hand stays, and the saved file names the style and the parts.
    void tableStyleOptionsAndTheStylesGallery()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        tableOnSlide(w, 4, 3);
        const QString id = theTable(ed)->id;
        const QString medium = QStringLiteral("{5C22544A-7EE6-4342-B048-85BDC9FD1C3A}");
        QCOMPARE(theTable(ed)->format, medium);
        QVERIFY(theTable(ed)->header && theTable(ed)->banded);
        QVERIFY(!theTable(ed)->cell(0, 0).fill.isNone());
        QVERIFY(!theTable(ed)->cell(1, 0).fill.isNone());
        QVERIFY(!theTable(ed)->cell(2, 0).fill.isNone());
        // The header row's words are bold, the others' plain; the bands alternate.
        QVERIFY(cellIsBold(ed, 0, 1));
        QVERIFY(!cellIsBold(ed, 1, 1));
        const Fill band1 = theTable(ed)->cell(1, 0).fill, band2 = theTable(ed)->cell(2, 0).fill;
        QVERIFY(!(band1 == band2));
        // Banded Rows off: the rows below the header are all one fill.
        QVERIFY(command(w, "tbl.optBanded"));
        QVERIFY(!theTable(ed)->banded);
        QVERIFY(theTable(ed)->cell(1, 0).fill == theTable(ed)->cell(2, 0).fill);
        ed->undo();
        QVERIFY(theTable(ed)->banded);
        QVERIFY(theTable(ed)->cell(1, 0).fill == band1 && theTable(ed)->cell(2, 0).fill == band2);
        // Total Row on: the last row is like the header. First Column, Last Column, Banded Columns, and Header Row likewise.
        QVERIFY(command(w, "tbl.optTotal"));
        QVERIFY(theTable(ed)->totalRow);
        QVERIFY(theTable(ed)->cell(3, 1).fill == theTable(ed)->cell(0, 1).fill);
        QVERIFY(cellIsBold(ed, 3, 1));
        QVERIFY(command(w, "tbl.optFirstCol"));
        QVERIFY(theTable(ed)->firstCol && theTable(ed)->cell(1, 0).fill == theTable(ed)->cell(0, 1).fill);
        QVERIFY(command(w, "tbl.optLastCol"));
        QVERIFY(theTable(ed)->lastCol && theTable(ed)->cell(1, 2).fill == theTable(ed)->cell(0, 1).fill);
        QVERIFY(command(w, "tbl.optBandedCols"));
        QVERIFY(theTable(ed)->bandedCols);
        QVERIFY(command(w, "tbl.optHeader"));
        QVERIFY(!theTable(ed)->header);
        QVERIFY(!cellIsBold(ed, 0, 1));
        QVERIFY(!(theTable(ed)->cell(0, 1).fill == theTable(ed)->cell(3, 1).fill));
        QVERIFY(command(w, "tbl.optHeader"));
        QVERIFY(theTable(ed)->header && cellIsBold(ed, 0, 1));
        // A cell shaded by hand keeps its shade when a part is turned off.
        ed->selectCells(id, 2, 1, 2, 1);
        w.setCellShading(ColorRef::rgb(QColor(250, 200, 0)));
        ed->select(id);
        QVERIFY(command(w, "tbl.optBandedCols"));
        QVERIFY(!theTable(ed)->bandedCols);
        QCOMPARE(theTable(ed)->cell(2, 1).fill.color.rgbValue(), QColor(250, 200, 0));
        QVERIFY(command(w, "tbl.optBandedCols"));
        // The gallery: a style gives every cell its look (the shading by hand too), and is saved by its id with the parts that show.
        const QString light1 = QStringLiteral("{9D7B26C5-4107-4FEC-AEDC-1716B250A1EF}");
        w.applyTableStyle(light1);
        QCOMPARE(theTable(ed)->format, light1);
        QVERIFY(theTable(ed)->cell(2, 1).fill.color.rgbValue() != QColor(250, 200, 0));
        QVERIFY(theTable(ed)->cell(0, 0).fill.isNone() || theTable(ed)->cell(0, 0).fill.transparency > 0);
        ed->undo();
        QCOMPARE(theTable(ed)->format, medium);
        QCOMPARE(theTable(ed)->cell(2, 1).fill.color.rgbValue(), QColor(250, 200, 0));
        w.applyTableStyle(QStringLiteral("{93296810-A885-4BE3-A3E7-6D5BEEA58F35}"));   // Medium Style 2, Accent 6
        QCOMPARE(theTable(ed)->cell(0, 0).fill.color.rgbValue(), deck::themeOf(*ed->doc()).colors[9]);
        const QString slide = savedSlideXml(*ed->doc());
        QVERIFY(slide.contains(QStringLiteral("<a:tableStyleId>{93296810-A885-4BE3-A3E7-6D5BEEA58F35}</a:tableStyleId>")));
        for (const char *attribute : {"firstRow", "lastRow", "firstCol", "lastCol", "bandRow", "bandCol"})
            QVERIFY2(slide.contains(QStringLiteral("%1=\"1\"").arg(QLatin1String(attribute))), attribute);
        auto back = savedAndRead(*ed->doc(), false);
        QVERIFY(back);
        const TableItem *bt = firstTable(*back);
        QVERIFY(bt);
        QCOMPARE(bt->format, theTable(ed)->format);
        QVERIFY(bt->header && bt->totalRow && bt->firstCol && bt->lastCol && bt->banded && bt->bandedCols);
        QVERIFY(bt->cell(0, 0).fill == theTable(ed)->cell(0, 0).fill);
        // A style the program doesn't have by this id is kept as it was named.
        const QString unknown = QStringLiteral("{11111111-2222-3333-4444-555555555555}");
        ed->beginChange(QStringLiteral("Rename"));
        theTable(ed)->format = unknown;
        ed->endChange();
        back = savedAndRead(*ed->doc(), false);
        QVERIFY(back);
        QCOMPARE(firstTable(*back)->format, unknown);
    }

    // The style goes on through rows and columns that come and go: the bands still alternate, a new row has the look its place gives it, the
    // header row stays the first row, and the words of a dark style stay light in the new cells.
    void tableStyleGoesOnThroughNewAndDeletedRowsAndColumns()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        tableOnSlide(w, 4, 3);
        const QString id = theTable(ed)->id;
        const Fill head = theTable(ed)->cell(0, 1).fill, band1 = theTable(ed)->cell(1, 0).fill, band2 = theTable(ed)->cell(2, 0).fill;
        QVERIFY(!(band1 == band2) && !(head == band1));
        // A row below the last: the bands go on (rows 1 and 3 are one band, row 2 the other, so row 4 is the second).
        ed->selectCells(id, 3, 0, 3, 0);
        QVERIFY(command(w, "tbl.insBelow"));
        QCOMPARE(theTable(ed)->rows, 5);
        QVERIFY(theTable(ed)->cell(4, 1).fill == band2);
        QVERIFY(theTable(ed)->cell(3, 1).fill == band1);
        // A row in the middle: every row below it takes the band its place now gives it.
        ed->selectCells(id, 1, 0, 1, 0);
        QVERIFY(command(w, "tbl.insAbove"));
        QCOMPARE(theTable(ed)->rows, 6);
        for (int r = 1; r < 6; ++r) QVERIFY2(theTable(ed)->cell(r, 2).fill == (r % 2 ? band1 : band2), qPrintable(QString::number(r)));
        QVERIFY(theTable(ed)->cell(0, 1).fill == head);
        // Taking a row away does the same; and a row above the first is the header, the old one a row of the bands.
        ed->selectCells(id, 2, 0, 2, 0);
        QVERIFY(command(w, "tbl.delRow"));
        for (int r = 1; r < 5; ++r) QVERIFY2(theTable(ed)->cell(r, 2).fill == (r % 2 ? band1 : band2), qPrintable(QString::number(r)));
        ed->selectCells(id, 0, 0, 0, 0);
        QVERIFY(command(w, "tbl.insAbove"));
        QVERIFY(theTable(ed)->cell(0, 1).fill == head && cellIsBold(ed, 0, 1));
        QVERIFY(theTable(ed)->cell(1, 1).fill == band1);
        QVERIFY(!cellIsBold(ed, 1, 1));   // "A2", the old header's words, are a body row's now
        QCOMPARE(cellText(ed, 1, 1), QStringLiteral("A2"));
        // Tab in the last cell adds a row, with the band its place gives it.
        const int rows = theTable(ed)->rows;
        ed->beginTextEdit(id, 0, rows - 1, theTable(ed)->cols - 1);
        QTest::keyClick(w.canvas(), Qt::Key_Tab);
        QCOMPARE(theTable(ed)->rows, rows + 1);
        QVERIFY(theTable(ed)->cell(rows, 0).fill == ((rows) % 2 ? band1 : band2));
        // A dark style: the words of a new row and column are light, whatever the table copied from its neighbors.
        ed->endTextEdit();
        ed->select(id);
        w.applyTableStyle(QStringLiteral("{E8034E78-7F5D-4C2E-B375-FC64B27BC917}"));   // Dark Style 1
        ed->selectCells(id, 1, 1, 1, 1);
        QVERIFY(command(w, "tbl.insRight"));
        QTextCursor cur(ed->doc()->storyDoc(theTable(ed)->cell(2, 2).storyId));
        QVERIFY(cur.charFormat().stringProperty(tp::ColorRefP).size() > 0);
        QVERIFY(QColor(cur.charFormat().stringProperty(tp::ColorRefP)).lightness() > 128);
        // And undo takes the new column and its look back in one step.
        ed->undo();
        QCOMPARE(theTable(ed)->cols, 3);
    }

    // Dragging the line between two columns of a table moves it: the column on its left widens, and the table's width stays.
    void tableColumnBorderDragResizesTheColumns()
    {
        DeckWindow w;
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        tableOnSlide(w, 3, 3);
        const TableItem *t = theTable(ed);
        const QRectF r = t->rect;
        const double x = r.left() + t->colW[0];
        const QPoint a = w.canvas()->pageToView(QPointF(x, r.center().y())).toPoint(), b = w.canvas()->pageToView(QPointF(x + 40, r.center().y())).toPoint();
        const double before = theTable(ed)->colW[0], width = theTable(ed)->rect.width();
        QWidget *vp = w.canvas()->viewport();
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, a);
        QTest::mouseMove(vp, a + QPoint(5, 0));
        QTest::mouseMove(vp, b);
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, b);
        QVERIFY(theTable(ed)->colW[0] > before + 20);
        QVERIFY(std::abs(theTable(ed)->rect.width() - width) < 0.01);
        ed->undo();
        QCOMPARE(theTable(ed)->colW[0], before);
    }

    // Crop to Shape offers closed shapes only, and only those a presentation file has by name (a shape without one would be saved as a rectangle).
    void everyCropShapeIsSavedByItsName()
    {
        DeckWindow w;
        auto *button = w.ribbon()->findChild<GalleryButton *>(QStringLiteral("cropToShapeButton"));
        QVERIFY(button);
        const QVector<GalleryItem> items = button->items();
        QVERIFY(items.size() > 50);
        QStringList ids;
        for (const GalleryItem &item : items) {
            QVERIFY2(!office::presetForShape(item.id).isEmpty(), qPrintable(item.id));
            const ShapeDef *shape = shapeDef(item.id);
            QVERIFY2(shape && !shape->open, qPrintable(item.id));
            ids << item.id;
        }
        QVERIFY(ids.contains(QStringLiteral("ellipse")) && ids.contains(QStringLiteral("rect")) && ids.contains(QStringLiteral("roundRect")));
    }

    // Typing, or choosing another object, while a crop is going on is not part of the crop: the crop ends as a step of its own, then the other
    // thing is a step, and Undo takes back the last of them only.
    void editsDuringACropAreSeparateSteps()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        Editor *ed = w.editor();
        PictureItem *pic = pictureOnSlide(w, dir);
        QVERIFY(pic);
        const QString id = pic->id;
        const QString title = ed->doc()->pages[0]->items.front()->id;
        const QRectF frame = pic->rect;
        const int steps = ed->undoStack()->index();
        // Typing in a text box.
        ed->select(id);
        QVERIFY(command(w, "pic.crop"));
        ed->beginCropChange(QStringLiteral("Crop"));
        thePicture(ed)->rect.setRight(thePicture(ed)->rect.right() - 30);
        ed->endChange();
        ed->beginTextEdit(title);
        QVERIFY(ed->cropItem.isEmpty());
        ed->typeText(QStringLiteral("ZQ"));
        ed->flushTyping();
        QCOMPARE(ed->undoStack()->index(), steps + 2);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Typing"));
        ed->endTextEdit();
        ed->undo();   // takes back the typing only
        QCOMPARE(thePicture(ed)->rect.width(), frame.width() - 30);
        QVERIFY(ed->doc()->storyDoc(static_cast<TextItem *>(ed->doc()->item(title))->storyId)->toPlainText().isEmpty());
        ed->undo();   // and then the crop
        QCOMPARE(thePicture(ed)->rect, frame);
        // Shift+click on another object.
        ed->select(id);
        QVERIFY(command(w, "pic.crop"));
        ed->beginCropChange(QStringLiteral("Crop"));
        thePicture(ed)->rect.setRight(thePicture(ed)->rect.right() - 30);
        ed->endChange();
        const int before = ed->undoStack()->index();
        ed->toggleSelect(title);
        QVERIFY(ed->cropItem.isEmpty());
        QCOMPARE(ed->undoStack()->index(), before + 1);
        QCOMPARE(thePicture(ed)->rect.width(), frame.width() - 30);
    }

    // The slides pane's moving a slide while a crop is going on is a step of its own, labeled for what it is, and the crop is a step before it.
    void aSlideMoveDuringACropIsItsOwnStep()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        Editor *ed = w.editor();
        PictureItem *pic = pictureOnSlide(w, dir);
        QVERIFY(pic);
        const QString id = pic->id;
        w.newSlide();
        ed->setCurrentPage(0);
        ed->select(id);
        QVERIFY(command(w, "pic.crop"));
        ed->beginCropChange(QStringLiteral("Crop"));
        thePicture(ed)->rect.setRight(thePicture(ed)->rect.right() - 30);
        ed->endChange();
        const int beforeMove = ed->undoStack()->index();
        ed->movePage(0, 1);
        QVERIFY(ed->cropItem.isEmpty());
        QCOMPARE(ed->undoStack()->index(), beforeMove + 2);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Move Page"));
        ed->undo();
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Crop"));
    }

    // Going to another slide while cropping ends the crop on the slide it was made on: redone, it is on that slide with the picture selected.
    void aCropRedoesOnItsOwnSlide()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        ed->setCurrentPage(0);
        PictureItem *pic = pictureOnSlide(w, dir);
        QVERIFY(pic);
        const QString id = pic->id;
        const QRectF frame = pic->rect;
        QVERIFY(command(w, "pic.crop"));
        ed->beginCropChange(QStringLiteral("Crop"));
        thePicture(ed)->rect.setRight(thePicture(ed)->rect.right() - 30);
        ed->endChange();
        ed->setCurrentPage(1);
        QVERIFY(ed->cropItem.isEmpty());
        ed->undo();
        QCOMPARE(ed->currentPage(), 0);
        QCOMPARE(thePicture(ed)->rect, frame);
        ed->redo();
        QCOMPARE(ed->currentPage(), 0);
        QCOMPARE(ed->selection(), QStringList{id});
        QCOMPARE(thePicture(ed)->rect.width(), frame.width() - 30);
    }

    // A crop still going on when the window closes (or saves) is a change: it is asked about, and what is written has it.
    void aCropInProgressIsAChangeToSave()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        Editor *ed = w.editor();
        PictureItem *pic = pictureOnSlide(w, dir);
        QVERIFY(pic);
        const double width = pic->rect.width();
        ed->markSaved();
        QVERIFY(!ed->isModified());
        QVERIFY(command(w, "pic.crop"));
        ed->beginCropChange(QStringLiteral("Crop"));
        thePicture(ed)->rect.setRight(thePicture(ed)->rect.right() - 30);
        ed->endChange();
        QVERIFY(ed->isModified());
        bool asked = false;
        QTimer::singleShot(0, &w, [&] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                asked = true;
                box->button(QMessageBox::Cancel)->click();
            }
        });
        w.show();
        QVERIFY(!w.close());   // asked, and told to keep the window
        QVERIFY(asked);
        QVERIFY(ed->cropItem.isEmpty());   // the crop is over, and kept
        QCOMPARE(thePicture(ed)->rect.width(), width - 30);
        QVERIFY(ed->isModified());
        // Saved in the middle of a crop, the file has the crop and the window is no longer modified.
        QVERIFY(command(w, "pic.crop"));
        ed->beginCropChange(QStringLiteral("Crop"));
        thePicture(ed)->rect.setRight(thePicture(ed)->rect.right() - 10);
        ed->endChange();
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("cropped.pptx"))));
        Settings::get().removeRecentFile(dir.filePath(QStringLiteral("cropped.pptx")));   // the file goes with the folder, and the list of recent ones doesn't keep it
        QVERIFY(!ed->isModified());
        QString err;
        auto back = deck::importPresentationFile(dir.filePath(QStringLiteral("cropped.pptx")), &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY(std::abs(firstPicture(*back)->rect.width() - (width - 40)) < 0.1);
    }

    // A style the file defines is the one a table is drawn and edited with, and is written back: a first column the file makes orange goes when
    // First Column is turned off, and the file's version of the style stays in the saved file.
    void tableStylesTheFileDefinesAreKeptAndUsed()
    {
        const QString id = QStringLiteral("{85BE263C-DBD7-4A20-BB59-AAB30ACAA65A}");   // a style this suite also has (Medium Style 3, Accent 2), but not like this
        const QString definition = QStringLiteral(
            "<a:tblStyleLst xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" def=\"%1\"><a:tblStyle styleId=\"%1\" styleName=\"The file's own\">"
            "<a:wholeTbl><a:tcTxStyle><a:fontRef idx=\"minor\"/><a:srgbClr val=\"000000\"/></a:tcTxStyle><a:tcStyle><a:tcBdr/><a:fill><a:solidFill><a:srgbClr val=\"FFEEDD\"/></a:solidFill></a:fill></a:tcStyle></a:wholeTbl>"
            "<a:firstCol><a:tcTxStyle b=\"on\"><a:fontRef idx=\"minor\"/><a:srgbClr val=\"000000\"/></a:tcTxStyle><a:tcStyle><a:tcBdr/><a:fill><a:solidFill><a:srgbClr val=\"FF8800\"/></a:solidFill></a:fill></a:tcStyle></a:firstCol>"
            "<a:firstRow><a:tcTxStyle b=\"on\"><a:fontRef idx=\"minor\"/><a:srgbClr val=\"FFFFFF\"/></a:tcTxStyle><a:tcStyle><a:tcBdr/><a:fill><a:solidFill><a:srgbClr val=\"112233\"/></a:solidFill></a:fill></a:tcStyle></a:firstRow>"
            "</a:tblStyle></a:tblStyleLst>").arg(id);
        // A file as the other program writes one: the cells say nothing of their own, and the style is defined in the file.
        QString err;
        std::unique_ptr<Document> d;
        {
            DeckWindow maker;
            tableOnSlide(maker, 3, 3);
            maker.applyTableStyle(id);
            d = deck::importPresentation(deck::exportPresentation(*maker.editor()->doc(), &err), &err);
            QVERIFY(d);
        }
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(deck::exportPresentation(*d, &err), zip, &err));
        QString slide = QString::fromUtf8(zip.value(QStringLiteral("ppt/slides/slide1.xml")));
        slide.replace(QRegularExpression(QStringLiteral("<a:tcPr[^>]*>.*?</a:tcPr>")), QStringLiteral("<a:tcPr/>"));
        slide.replace(QStringLiteral("<a:tableStyleId>%1</a:tableStyleId>").arg(QStringLiteral("{85BE263C-DBD7-4A20-BB59-AAB30ACAA65A}")), QStringLiteral("<a:tableStyleId>%1</a:tableStyleId>").arg(id));
        if (!slide.contains(QStringLiteral("firstCol=\"1\""))) slide.replace(QStringLiteral("<a:tblPr "), QStringLiteral("<a:tblPr firstCol=\"1\" "));
        zip[QStringLiteral("ppt/slides/slide1.xml")] = slide.toUtf8();
        zip[QStringLiteral("ppt/tableStyles.xml")] = definition.toUtf8();
        ZipWriter z;
        for (auto it = zip.cbegin(); it != zip.cend(); ++it) z.add(it.key(), it.value(), true);
        const QByteArray file = z.finish();
        auto opened = deck::importPresentation(file, &err);
        QVERIFY2(opened, qPrintable(err));
        QCOMPARE(firstTable(*opened)->cell(1, 0).fill.color.rgbValue(), QColor(0xFF, 0x88, 0x00));   // the file's, read
        QCOMPARE(firstTable(*opened)->cell(1, 1).fill.color.rgbValue(), QColor(0xFF, 0xEE, 0xDD));
        // The window opens it, and First Column off makes the column what the file's style says the whole table is.
        DeckWindow w;
        Editor *ed = w.editor();
        ed->setDocument(std::move(opened));
        ed->select(theTable(ed)->id);
        QVERIFY(command(w, "tbl.optFirstCol"));
        QVERIFY(!theTable(ed)->firstCol);
        QCOMPARE(theTable(ed)->cell(1, 0).fill.color.rgbValue(), QColor(0xFF, 0xEE, 0xDD));
        QCOMPARE(theTable(ed)->cell(1, 1).fill.color.rgbValue(), QColor(0xFF, 0xEE, 0xDD));
        QCOMPARE(theTable(ed)->cell(0, 1).fill.color.rgbValue(), QColor(0x11, 0x22, 0x33));
        QVERIFY(command(w, "tbl.optFirstCol"));
        QCOMPARE(theTable(ed)->cell(1, 0).fill.color.rgbValue(), QColor(0xFF, 0x88, 0x00));
        // Saved, the file's definition goes back, and the file reads the same again.
        QMap<QString, QByteArray> out;
        QVERIFY(readZip(deck::exportPresentation(*ed->doc(), &err), out, &err));
        const QString styles = QString::fromUtf8(out.value(QStringLiteral("ppt/tableStyles.xml")));
        QVERIFY(styles.contains(QStringLiteral("FF8800")) && styles.contains(QStringLiteral("FFEEDD")) && styles.contains(id));
        QVERIFY(styles.contains(QStringLiteral("The file's own")));
        auto again = savedAndRead(*ed->doc(), false);
        QVERIFY(again);
        QCOMPARE(firstTable(*again)->cell(1, 0).fill.color.rgbValue(), QColor(0xFF, 0x88, 0x00));
        // A table of a presentation with no styles of its own is written as before: the default, and none defined.
        QVERIFY(readZip(deck::exportPresentation(*deck::newPresentation(), &err), out, &err));
        QVERIFY(!QString::fromUtf8(out.value(QStringLiteral("ppt/tableStyles.xml"))).contains(QStringLiteral("tblStyle ")));
    }

    // Every table command is one step that takes everything back: after Undo the document is what it was, and after Redo what it was after the
    // command (the table here has a style, merged cells, text, and a block of cells selected).
    void tableCommandsUndoAndRedoExactly()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        tableOnSlide(w, 4, 4);
        const QString id = theTable(ed)->id;
        ed->selectCells(id, 0, 0, 1, 1);
        QVERIFY(command(w, "tbl.merge"));
        struct Case { const char *command; int r0, c0, r1, c1; };
        const Case cases[] = {{"tbl.insAbove", 2, 2, 2, 2}, {"tbl.insBelow", 3, 1, 3, 1}, {"tbl.insLeft", 2, 2, 2, 2}, {"tbl.insRight", 1, 3, 1, 3},
                              {"tbl.delRow", 3, 0, 3, 3}, {"tbl.delCol", 0, 3, 3, 3}, {"tbl.split", 0, 0, 0, 0}, {"tbl.merge", 2, 0, 3, 1},
                              {"tbl.distributeRows", 2, 2, 2, 2}, {"tbl.distributeCols", 2, 2, 2, 2}, {"border.all", 2, 2, 3, 3}, {"border.none", 2, 2, 3, 3},
                              {"tbl.alignMiddle", 2, 2, 3, 3}, {"tbl.direction", 2, 2, 3, 3}, {"tbl.marginWide", 2, 2, 3, 3}, {"tbl.optBanded", 2, 2, 2, 2},
                              {"tbl.optTotal", 2, 2, 2, 2}, {"tbl.optFirstCol", 2, 2, 2, 2}, {"tbl.optHeader", 2, 2, 2, 2}};
        for (const Case &c : cases) {
            if (QLatin1String(c.command).startsWith(QLatin1String("tbl.distribute"))) {   // made unequal first, as there is something to distribute
                ed->change(QStringLiteral("Size"), [&] {
                    theTable(ed)->colW[0] += 10;
                    theTable(ed)->rowH[0] += 10;
                    theTable(ed)->syncRect();
                });
            }
            ed->selectCells(id, c.r0, c.c0, c.r1, c.c1);
            const QByteArray before = documentJson(ed);
            const int steps = ed->undoStack()->index();
            QVERIFY(command(w, c.command));
            QVERIFY2(ed->undoStack()->index() == steps + 1, c.command);
            const QByteArray after = documentJson(ed);
            QVERIFY2(after != before, c.command);
            ed->undo();
            QVERIFY2(documentJson(ed) == before, c.command);
            ed->redo();
            QVERIFY2(documentJson(ed) == after, c.command);
        }
        w.applyTableStyle(QStringLiteral("{E8034E78-7F5D-4C2E-B375-FC64B27BC917}"));
        const QByteArray styled = documentJson(ed);
        ed->undo();
        QVERIFY(documentJson(ed) != styled);
        ed->redo();
        QVERIFY(documentJson(ed) == styled);
    }

    // A table a file damages (spans far larger than the table, every flag on) is read, styled, and edited without harm: the
    // spans stop at the table's edge, and no command reaches past a row or a column.
    void hostileTablesAreHeldToTheirGrid()
    {
        DeckWindow maker;
        tableOnSlide(maker, 3, 3);
        QString err;
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(deck::exportPresentation(*maker.editor()->doc(), &err), zip, &err));
        QString slide = QString::fromUtf8(zip.value(QStringLiteral("ppt/slides/slide1.xml")));
        slide.replace(QRegularExpression(QStringLiteral("<a:tc>")), QStringLiteral("<a:tc gridSpan=\"1000000000\" rowSpan=\"2000000000\">"));
        slide.replace(QStringLiteral("<a:tblPr "), QStringLiteral("<a:tblPr lastRow=\"1\" firstCol=\"1\" lastCol=\"1\" bandCol=\"1\" "));
        zip[QStringLiteral("ppt/slides/slide1.xml")] = slide.toUtf8();
        ZipWriter z;
        for (auto it = zip.cbegin(); it != zip.cend(); ++it) z.add(it.key(), it.value(), true);
        auto d = deck::importPresentation(z.finish(), &err);
        QVERIFY2(d, qPrintable(err));
        const TableItem *read = firstTable(*d);
        QVERIFY(read);
        for (int r = 0; r < read->rows; ++r)
            for (int c = 0; c < read->cols; ++c) {
                QVERIFY(r + read->cell(r, c).rowSpan <= read->rows);
                QVERIFY(c + read->cell(r, c).colSpan <= read->cols);
            }
        DeckWindow w;
        Editor *ed = w.editor();
        ed->setDocument(std::move(d));
        ed->select(theTable(ed)->id);
        w.applyTableStyle(QStringLiteral("{9D7B26C5-4107-4FEC-AEDC-1716B250A1EF}"));
        for (const char *name : {"tbl.optBanded", "tbl.optTotal", "tbl.insBelow", "tbl.insRight", "tbl.split", "tbl.distributeRows", "border.all"}) {
            ed->select(theTable(ed)->id);
            QVERIFY(command(w, name));
        }
        QVERIFY(theTable(ed)->rows >= 3 && theTable(ed)->cols >= 3);
        QVERIFY(!savedSlideXml(*ed->doc()).isEmpty());
        auto saved = savedAndRead(*ed->doc(), true);
        QVERIFY(saved);
    }

    // A crop of a picture that is turned and flipped keeps the picture where it is on the slide, and is saved with its turn and flips; a picture
    // that is fitted (room round it, a negative a:srcRect) is saved and read back the same.
    void pictureCropOfATurnedPictureAndANegativeSrcRect()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        Editor *ed = w.editor();
        PictureItem *pic = pictureOnSlide(w, dir);
        QVERIFY(pic);
        ed->beginChange(QStringLiteral("Turn"));
        pic->rotation = 30;
        pic->flipH = true;
        ed->endChange();
        const QPointF whereTheTopLeftIs = thePicture(ed)->transform().map(thePicture(ed)->imgRect.topLeft());
        const QPointF whereTheBottomRightIs = thePicture(ed)->transform().map(thePicture(ed)->imgRect.bottomRight());
        QVERIFY(command(w, "pic.aspect.1x1"));
        QCOMPARE(thePicture(ed)->rect.size(), QSizeF(75, 75));
        QVERIFY((thePicture(ed)->transform().map(thePicture(ed)->imgRect.topLeft()) - whereTheTopLeftIs).manhattanLength() < 0.001);
        QVERIFY((thePicture(ed)->transform().map(thePicture(ed)->imgRect.bottomRight()) - whereTheBottomRightIs).manhattanLength() < 0.001);
        QString slide = savedSlideXml(*ed->doc());
        QVERIFY(slide.contains(QStringLiteral("rot=\"1800000\"")));
        QVERIFY(slide.contains(QStringLiteral("flipH=\"1\"")));
        QVERIFY(slide.contains(QStringLiteral("<a:srcRect")));
        auto back = savedAndRead(*ed->doc(), false);
        QVERIFY(back);
        const PictureItem *p = firstPicture(*back);
        QVERIFY(p);
        QVERIFY(p->flipH);
        QVERIFY(std::abs(p->rotation - 30) < 0.01);
        QVERIFY((p->transform().map(p->imgRect.topLeft()) - whereTheTopLeftIs).manhattanLength() < 0.05);
        QVERIFY((p->transform().map(p->imgRect.bottomRight()) - whereTheBottomRightIs).manhattanLength() < 0.05);
        // Fit: a picture 150 by 75 in a frame 100 by 100 has room above and below, which the file says with negative cuts.
        ed->beginChange(QStringLiteral("Frame"));
        thePicture(ed)->rotation = 0;
        thePicture(ed)->flipH = false;
        thePicture(ed)->rect = QRectF(thePicture(ed)->rect.topLeft(), QSizeF(100, 100));
        thePicture(ed)->imgRect = QRectF(0, 0, 100, 100);
        ed->endChange();
        QVERIFY(command(w, "pic.fit"));
        const QRectF fitted = thePicture(ed)->imgRect;
        QCOMPARE(fitted.size(), QSizeF(100, 50));
        slide = savedSlideXml(*ed->doc());
        QVERIFY(slide.contains(QStringLiteral("t=\"-50000\"")) && slide.contains(QStringLiteral("b=\"-50000\"")));   // each is half the height of the picture
        for (bool odp : {false, true}) {
            back = savedAndRead(*ed->doc(), odp);
            QVERIFY(back);
            p = firstPicture(*back);
            QVERIFY(p);
            QVERIFY2(std::abs(p->imgRect.height() - fitted.height()) < 0.5 && std::abs(p->imgRect.width() - fitted.width()) < 0.5, odp ? "odp" : "pptx");
            QVERIFY2(std::abs(p->imgRect.top() - fitted.top()) < 0.5, odp ? "odp" : "pptx");
        }
    }

    // A style is given to every cell, the ones a merged cell covers too: what a split uncovers is as the style has it, whether the style was
    // chosen, or a row came, while the cells were merged.
    void aStyleReachesTheCellsAMergeCovers()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        tableOnSlide(w, 4, 4);
        const QString id = theTable(ed)->id;
        auto allAsTheStyleHasThem = [&](const char *what) {
            office::TableStyleEffect fx;
            if (!office::tableStyleEffect(theTable(ed)->format, *theTable(ed), tableThemeOf(*ed->doc()), &fx)) return false;
            for (int r = 0; r < theTable(ed)->rows; ++r)
                for (int c = 0; c < theTable(ed)->cols; ++c)
                    if (!(theTable(ed)->cell(r, c).fill == fx.fill[size_t(r) * theTable(ed)->cols + c])) {
                        qWarning("%s: cell %d,%d", what, r, c);
                        return false;
                    }
            return true;
        };
        ed->selectCells(id, 0, 0, 1, 1);
        QVERIFY(command(w, "tbl.merge"));
        w.applyTableStyle(QStringLiteral("{E8034E78-7F5D-4C2E-B375-FC64B27BC917}"));   // Dark Style 1
        ed->selectCells(id, 0, 0, 0, 0);
        QVERIFY(command(w, "tbl.split"));
        QVERIFY(allAsTheStyleHasThem("chosen while merged"));
        // A row that comes while cells are merged, and an option turned off.
        ed->selectCells(id, 2, 2, 3, 3);
        QVERIFY(command(w, "tbl.merge"));
        ed->selectCells(id, 0, 0, 0, 0);
        QVERIFY(command(w, "tbl.insAbove"));
        ed->select(id);
        QVERIFY(command(w, "tbl.optBanded"));
        ed->selectCells(id, 3, 2, 3, 2);
        QVERIFY(command(w, "tbl.split"));
        QVERIFY(allAsTheStyleHasThem("row and option while merged"));
    }

    // Every style in the gallery is one the reader and the tab know: Light, Medium, and Dark, plain and in the six accent colors, with the
    // theme's colors in them.
    void tableStylesAreDrawnFromTheTheme()
    {
        const QVector<office::TableStyleInfo> &styles = office::tableStyleList();
        QVERIFY(styles.size() >= 60);
        QSet<QString> ids;
        office::Theme theme;
        for (int i = 0; i < office::Theme::ColorCount; ++i) theme.colors[i] = QColor::fromHsv(i * 30, 200, 200);
        TableItem t;
        t.rows = 4;
        t.cols = 3;
        t.header = t.banded = true;
        for (const auto &s : styles) {
            QVERIFY2(!ids.contains(s.id), qPrintable(s.id));
            ids.insert(s.id);
            QVERIFY2(!office::tableStyleXml(s.id).isEmpty(), qPrintable(s.name));
            office::TableStyleEffect fx;
            QVERIFY2(office::tableStyleEffect(s.id, t, theme, &fx), qPrintable(s.name));
            QCOMPARE(int(fx.fill.size()), 12);
        }
        // Medium Style 2 - Accent 3 has the third accent in its header row and tints of it in the bands.
        office::TableStyleEffect fx;
        QVERIFY(office::tableStyleEffect(QStringLiteral("{F5AB1C69-6EDB-4FF4-983F-18BD219EF322}"), t, theme, &fx));
        QCOMPARE(fx.fill[0].color.rgbValue().rgb(), theme.colors[office::Theme::Accent3].rgb());
        QVERIFY(fx.fill[3].color.rgbValue().rgb() != theme.colors[office::Theme::Accent3].rgb());   // a band
        QVERIFY(!(fx.fill[3] == fx.fill[6]));
        QVERIFY(!office::tableStyleEffect(QStringLiteral("{11111111-2222-3333-4444-555555555555}"), t, theme, &fx));
    }

private:
    // A red picture 200 by 100, put on the slide of a window.
    static PictureItem *pictureOnSlide(DeckWindow &w, const QTemporaryDir &dir)
    {
        QImage image(200, 100, QImage::Format_RGB32);
        image.fill(Qt::red);
        const QString path = dir.filePath(QStringLiteral("red.png"));
        if (!image.save(path) || !w.insertPicture(path)) return nullptr;
        return thePicture(w.editor());
    }

private Q_SLOTS:
    // The Table Design and Table Layout tabs show while a table or a cell of one is selected, the Picture Format tab while a picture is; their
    // boxes, gallery, and sizes act on the table.
    void tableAndPictureTabsFollowTheSelection()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        Editor *ed = w.editor();
        auto shown = [&](const QString &name) {
            for (int i = 0; i < w.ribbon()->tabCount(); ++i)
                if (w.ribbon()->tabName(i) == name) return w.ribbon()->tabVisible(i);
            return false;
        };
        QVERIFY(!shown(QStringLiteral("Table Design")) && !shown(QStringLiteral("Table Layout")) && !shown(QStringLiteral("Picture Format")));
        tableOnSlide(w, 3, 3);
        const QString id = theTable(ed)->id;
        QVERIFY(shown(QStringLiteral("Table Design")) && shown(QStringLiteral("Table Layout")));
        QVERIFY(!shown(QStringLiteral("Picture Format")));
        ed->clearSelection();
        QVERIFY(!shown(QStringLiteral("Table Design")));
        ed->beginTextEdit(id, 0, 1, 1);   // typing in a cell is enough
        QVERIFY(shown(QStringLiteral("Table Layout")));
        ed->endTextEdit();
        ed->select(id);
        // The boxes are the commands'.
        auto *banded = w.ribbon()->findChild<QCheckBox *>(QStringLiteral("tbl.optBanded"));
        QVERIFY(banded);
        QVERIFY(banded->isChecked());
        banded->click();
        QVERIFY(!theTable(ed)->banded);
        QVERIFY(!banded->isChecked());
        // The gallery lists the styles and applies one.
        auto *gallery = w.ribbon()->findChild<Gallery *>(QStringLiteral("tableStyles"));
        QVERIFY(gallery);
        Q_EMIT gallery->activated(QStringLiteral("{3B4B98B0-60AC-42C2-AFA5-B58CD77FA1E5}"));
        QCOMPARE(theTable(ed)->format, QStringLiteral("{3B4B98B0-60AC-42C2-AFA5-B58CD77FA1E5}"));
        // The row height and column width show those of the cell chosen, and typing one changes the rows or columns chosen.
        ed->selectCells(id, 2, 1, 2, 1);
        auto *height = w.ribbon()->findChild<MeasureSpin *>(QStringLiteral("tableRowHeight"));
        auto *width = w.ribbon()->findChild<MeasureSpin *>(QStringLiteral("tableColumnWidth"));
        QVERIFY(height && width);
        QVERIFY(std::abs(height->value() - theTable(ed)->rowH[2]) < 0.5);
        QVERIFY(std::abs(width->value() - theTable(ed)->colW[1]) < 0.5);
        height->setValue(90);
        width->setValue(120);
        QCOMPARE(theTable(ed)->rowH[2], 90.0);
        QCOMPARE(theTable(ed)->rowH[1] == 90.0, false);
        QCOMPARE(theTable(ed)->colW[1], 120.0);
        ed->clearSelection();
        PictureItem *pic = pictureOnSlide(w, dir);
        QVERIFY(pic);
        QVERIFY(shown(QStringLiteral("Picture Format")));
        QVERIFY(!shown(QStringLiteral("Table Design")));
        ed->clearSelection();
        QVERIFY(!shown(QStringLiteral("Picture Format")));
    }

    // A picture is cropped by its handles: the crop starts with the Crop button, the hidden part of the picture is kept, and the whole crop
    // is one step that undoes. Enter ends it, or a click away; Esc takes it back.
    void pictureCropIsOneStepWithEnterAndEsc()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        Editor *ed = w.editor();
        PictureItem *pic = pictureOnSlide(w, dir);
        QVERIFY(pic);
        const QString id = pic->id;
        const QRectF frame = pic->rect, whole = pic->imgRect;
        const int steps = ed->undoStack()->count();
        QVERIFY(command(w, "pic.crop"));
        QCOMPARE(ed->cropItem, id);
        // Two drags of two handles, as the canvas makes them.
        for (int drag = 0; drag < 2; ++drag) {
            PictureItem *p = thePicture(ed);
            ed->beginCropChange(QStringLiteral("Crop"));
            if (drag == 0) {
                p->rect.setLeft(p->rect.left() + 30);
                p->imgRect.translate(-30, 0);
            } else {
                p->rect.setBottom(p->rect.bottom() - 10);
            }
            ed->endChange();
        }
        QCOMPARE(ed->undoStack()->count(), steps);   // nothing is a step until the crop is done
        QTest::keyClick(w.canvas(), Qt::Key_Return);
        QVERIFY(ed->cropItem.isEmpty());
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Crop"));
        QCOMPARE(thePicture(ed)->rect.width(), frame.width() - 30);
        QCOMPARE(thePicture(ed)->rect.height(), frame.height() - 10);
        QCOMPARE(thePicture(ed)->imgRect.size(), whole.size());   // the picture itself keeps its size
        ed->undo();
        QCOMPARE(thePicture(ed)->rect, frame);
        QCOMPARE(thePicture(ed)->imgRect, whole);
        ed->redo();
        QCOMPARE(thePicture(ed)->rect.width(), frame.width() - 30);
        // Esc: the crop that was begun is taken back whole, and leaves no step.
        const QRectF cropped = thePicture(ed)->rect;
        const int stepsNow = ed->undoStack()->count();
        ed->select(id);
        QVERIFY(command(w, "pic.crop"));
        ed->beginCropChange(QStringLiteral("Crop"));
        thePicture(ed)->rect.setRight(thePicture(ed)->rect.right() - 40);
        ed->endChange();
        QTest::keyClick(w.canvas(), Qt::Key_Escape);
        QVERIFY(ed->cropItem.isEmpty());
        QCOMPARE(thePicture(ed)->rect, cropped);
        QCOMPARE(ed->undoStack()->count(), stepsNow);
        QCOMPARE(ed->selection(), QStringList{id});
        // Choosing something else ends it too, keeping the crop.
        QVERIFY(command(w, "pic.crop"));
        ed->beginCropChange(QStringLiteral("Crop"));
        thePicture(ed)->rect.setRight(thePicture(ed)->rect.right() - 20);
        ed->endChange();
        ed->clearSelection();
        QVERIFY(ed->cropItem.isEmpty());
        QCOMPARE(thePicture(ed)->rect.width(), cropped.width() - 20);
        QCOMPARE(ed->undoStack()->count(), stepsNow + 1);
        ed->undo();   // the crop is one step; undo is not stuck inside it
        QCOMPARE(thePicture(ed)->rect, cropped);
        // Undo while cropping takes the crop back; another command ends the crop, keeping it, and then does its own work.
        ed->select(id);
        QVERIFY(command(w, "pic.crop"));
        ed->beginCropChange(QStringLiteral("Crop"));
        thePicture(ed)->rect.setRight(thePicture(ed)->rect.right() - 25);
        ed->endChange();
        QTRY_VERIFY(w.act(QStringLiteral("edit.undo"))->isEnabled());   // the window's commands follow the editor a moment later
        QVERIFY(command(w, "edit.undo"));
        QVERIFY(ed->cropItem.isEmpty());
        QCOMPARE(thePicture(ed)->rect, cropped);
        const int before = ed->undoStack()->index();   // steps taken (a step that was undone is in the way of the next one, and is dropped)
        QVERIFY(command(w, "pic.crop"));
        ed->beginCropChange(QStringLiteral("Crop"));
        thePicture(ed)->rect.setRight(thePicture(ed)->rect.right() - 25);
        ed->endChange();
        QVERIFY(command(w, "arr.flipH"));
        QVERIFY(ed->cropItem.isEmpty());
        QVERIFY(thePicture(ed)->flipH);
        QCOMPARE(ed->undoStack()->index(), before + 2);
        ed->undo();
        QVERIFY(!thePicture(ed)->flipH);
        QCOMPARE(thePicture(ed)->rect.width(), cropped.width() - 25);
    }

    // The same with the mouse: a handle of the frame is dragged in, and a drag inside moves the picture in its frame; Enter ends the crop, which
    // is one step.
    void pictureCropByDraggingTheHandles()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        PictureItem *pic = pictureOnSlide(w, dir);
        QVERIFY(pic);
        const QRectF frame = pic->rect, whole = pic->imgRect;
        const QPointF whereTheTopLeftIs = pic->transform().map(whole.topLeft());
        Canvas *canvas = w.canvas();
        QWidget *vp = canvas->viewport();
        const int steps = ed->undoStack()->count();
        QVERIFY(command(w, "pic.crop"));
        auto drag = [&](const QPointF &from, const QPointF &to) {
            const QPoint a = canvas->pageToView(from).toPoint(), b = canvas->pageToView(to).toPoint();
            QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, a);
            QTest::mouseMove(vp, a + (b - a) / 3);
            QTest::mouseMove(vp, a + (b - a) * 2 / 3);
            QTest::mouseMove(vp, b);
            QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, b);
        };
        // The right edge in by 30 points, then the top edge down by 15.
        drag(QPointF(frame.right(), frame.center().y()), QPointF(frame.right() - 30, frame.center().y()));
        QVERIFY(std::abs(thePicture(ed)->rect.width() - (frame.width() - 30)) < 2);
        drag(QPointF(thePicture(ed)->rect.center().x(), frame.top()), QPointF(thePicture(ed)->rect.center().x(), frame.top() + 15));
        QVERIFY(std::abs(thePicture(ed)->rect.height() - (frame.height() - 15)) < 2);
        // The picture stayed where it was on the slide while the frame closed in.
        QVERIFY((thePicture(ed)->transform().map(thePicture(ed)->imgRect.topLeft()) - whereTheTopLeftIs).manhattanLength() < 0.01);
        QCOMPARE(thePicture(ed)->imgRect.size(), whole.size());
        // A drag inside the frame moves the picture in it.
        const QPointF inside = thePicture(ed)->rect.center();
        drag(inside, inside + QPointF(-6, 0));
        QVERIFY(thePicture(ed)->transform().map(thePicture(ed)->imgRect.topLeft()).x() < whereTheTopLeftIs.x() - 3);
        QCOMPARE(ed->cropItem, thePicture(ed)->id);
        QCOMPARE(ed->undoStack()->count(), steps);   // still one crop in progress
        QTest::keyClick(canvas, Qt::Key_Enter);
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QVERIFY(ed->cropItem.isEmpty());
        ed->undo();
        QCOMPARE(thePicture(ed)->rect, frame);
        QCOMPARE(thePicture(ed)->imgRect, whole);
        // A click on the empty slide ends a crop too.
        ed->select(thePicture(ed)->id);
        QVERIFY(command(w, "pic.crop"));
        drag(QPointF(frame.right(), frame.center().y()), QPointF(frame.right() - 20, frame.center().y()));
        const QPoint away = canvas->pageToView(QPointF(8, 8)).toPoint();
        QTest::mouseClick(vp, Qt::LeftButton, Qt::NoModifier, away);
        QVERIFY(ed->cropItem.isEmpty());
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QVERIFY(std::abs(thePicture(ed)->rect.width() - (frame.width() - 20)) < 2);
    }

    // Aspect Ratio crops the frame to the ratio around its middle, the picture staying where it is on the slide; Fill and Fit make the
    // picture cover or fit in the frame; Clear Crop shows all of it; Crop to Shape trims it to a shape. Each is one step.
    void pictureCropAspectRatioFillFitAndShape()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        Editor *ed = w.editor();
        PictureItem *pic = pictureOnSlide(w, dir);
        QVERIFY(pic);
        const QRectF frame = pic->rect;   // 150 by 75: the 200 by 100 pixels at the screen's 96 to the inch
        QCOMPARE(frame.size(), QSizeF(150, 75));
        const QPointF middle = frame.center();
        const QPointF whereTheTopLeftIs = pic->transform().map(pic->imgRect.topLeft());
        QVERIFY(command(w, "pic.aspect.1x1"));
        QCOMPARE(thePicture(ed)->rect.size(), QSizeF(75, 75));
        QCOMPARE(thePicture(ed)->rect.center(), middle);
        QCOMPARE(thePicture(ed)->transform().map(thePicture(ed)->imgRect.topLeft()), whereTheTopLeftIs);
        QCOMPARE(thePicture(ed)->imgRect.size(), QSizeF(150, 75));
        ed->undo();
        QCOMPARE(thePicture(ed)->rect, frame);
        QVERIFY(command(w, "pic.aspect.16x9"));
        QVERIFY(std::abs(thePicture(ed)->rect.width() / thePicture(ed)->rect.height() - 16.0 / 9) < 1e-6);
        QCOMPARE(thePicture(ed)->rect.height(), 75.0);   // the frame is wider than 16:9: its height stays and the width goes down
        ed->undo();
        QVERIFY(command(w, "pic.aspect.9x16"));
        QVERIFY(std::abs(thePicture(ed)->rect.width() / thePicture(ed)->rect.height() - 9.0 / 16) < 1e-6);
        QCOMPARE(thePicture(ed)->rect.height(), 75.0);
        // Fill: the picture covers the frame (a crop from the middle); Fit: it is all in the frame.
        ed->undo();
        ed->beginChange(QStringLiteral("Frame"));
        thePicture(ed)->rect = QRectF(frame.topLeft(), QSizeF(100, 100));
        thePicture(ed)->imgRect = QRectF(0, 0, 100, 100);
        ed->endChange();
        QVERIFY(command(w, "pic.fill"));
        QCOMPARE(thePicture(ed)->imgRect.height(), 100.0);
        QCOMPARE(thePicture(ed)->imgRect.width(), 200.0);
        QCOMPARE(thePicture(ed)->imgRect.center(), QPointF(50, 50));
        QVERIFY(command(w, "pic.fit"));
        QCOMPARE(thePicture(ed)->imgRect.width(), 100.0);
        QCOMPARE(thePicture(ed)->imgRect.height(), 50.0);
        QVERIFY(command(w, "pic.fill"));
        QVERIFY(command(w, "pic.clearCrop"));
        QCOMPARE(thePicture(ed)->rect.size(), QSizeF(200, 100));
        QCOMPARE(thePicture(ed)->imgRect, QRectF(0, 0, 200, 100));
        // Crop to Shape.
        w.cropPictureToShape(QStringLiteral("ellipse"));
        QCOMPARE(thePicture(ed)->maskShape, QStringLiteral("ellipse"));
        ed->undo();
        QCOMPARE(thePicture(ed)->maskShape, QStringLiteral("rect"));
        w.cropPictureToShape(QStringLiteral("ellipse"));
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Crop to Shape"));
    }

    // A crop is saved as the formats have it and read back the same: a:srcRect on the picture's blip and the shape's geometry in a .pptx,
    // fo:clip in an .odp (which draws a picture cropped to a shape into the picture itself).
    void pictureCropIsSavedAndReadBack()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        Editor *ed = w.editor();
        PictureItem *pic = pictureOnSlide(w, dir);
        QVERIFY(pic);
        // Trimmed 40 off the left, 10 off the top, and 20 off the bottom of a picture 200 by 100, and to an ellipse.
        ed->beginChange(QStringLiteral("Crop"));
        pic->rect = QRectF(pic->rect.left() + 40, pic->rect.top() + 10, 160, 70);
        pic->imgRect = QRectF(-40, -10, 200, 100);
        ed->endChange();
        w.cropPictureToShape(QStringLiteral("ellipse"));
        const QRectF frame = thePicture(ed)->rect, whole = thePicture(ed)->imgRect;
        const QString slide = savedSlideXml(*ed->doc());
        QVERIFY(slide.contains(QStringLiteral("<a:srcRect l=\"20000\" t=\"10000\" b=\"20000\"/>")));
        QVERIFY(slide.contains(QStringLiteral("<a:prstGeom prst=\"ellipse\">")));
        auto back = savedAndRead(*ed->doc(), false);
        QVERIFY(back);
        const PictureItem *p = firstPicture(*back);
        QVERIFY(p);
        QCOMPARE(p->maskShape, QStringLiteral("ellipse"));
        QCOMPARE(p->rect.size(), frame.size());
        QVERIFY(std::abs(p->imgRect.left() - whole.left()) < 0.01 && std::abs(p->imgRect.top() - whole.top()) < 0.01);
        QVERIFY(std::abs(p->imgRect.width() - whole.width()) < 0.01 && std::abs(p->imgRect.height() - whole.height()) < 0.01);
        // The .odp: the crop of a picture with the usual shape.
        ed->beginChange(QStringLiteral("Square"));
        thePicture(ed)->maskShape = QStringLiteral("rect");
        ed->endChange();
        back = savedAndRead(*ed->doc(), true);
        QVERIFY(back);
        p = firstPicture(*back);
        QVERIFY(p);
        QVERIFY(std::abs(p->rect.width() - frame.width()) < 0.1 && std::abs(p->rect.height() - frame.height()) < 0.1);
        QVERIFY(std::abs(p->imgRect.left() - whole.left()) < 0.2 && std::abs(p->imgRect.top() - whole.top()) < 0.2);
        QVERIFY(std::abs(p->imgRect.width() - whole.width()) < 0.2 && std::abs(p->imgRect.height() - whole.height()) < 0.2);
        // Cropped to a shape, an .odp keeps what the slide shows: a picture that is round at the corners shows nothing there.
        ed->beginChange(QStringLiteral("Round"));
        thePicture(ed)->maskShape = QStringLiteral("ellipse");
        ed->endChange();
        back = savedAndRead(*ed->doc(), true);
        QVERIFY(back);
        QVERIFY(firstPicture(*back));
    }
};

JP_TEST_CLASS(DeckTableTests)
#include "tst_deck_tables.moc"
