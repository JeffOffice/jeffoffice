// JeffPub's tests of tables: editing, dividing cells, and saving them.
#include "pubhelpers.h"

class PubTableTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    // With the whole table selected, Insert Below adds at the bottom (it
    // added above the first row) and Delete Rows asks for a row (it deleted
    // the first); Cell Margins changes table cells (it did nothing).
    void tableCommandsWithTheTableSelected()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto t = std::static_pointer_cast<jp::TableItem>(ed->newTable(QRectF(72, 72, 288, 72), 2, 2));
        ed->addItem(t);
        ed->doc()->storyDoc(t->cell(0, 0).storyId)->setPlainText(QStringLiteral("top"));
        ed->select(t->id);
        w.act(QStringLiteral("tbl.insBelow"))->trigger();
        QCOMPARE(t->rows, 3);
        QCOMPARE(ed->doc()->storyDoc(t->cell(0, 0).storyId)->toPlainText(), QStringLiteral("top"));
        w.act(QStringLiteral("tbl.insRight"))->trigger();
        QCOMPARE(t->cols, 3);
        QCOMPARE(ed->doc()->storyDoc(t->cell(0, 0).storyId)->toPlainText(), QStringLiteral("top"));
        w.act(QStringLiteral("tbl.delRow"))->trigger();
        w.act(QStringLiteral("tbl.delCol"))->trigger();
        QCOMPARE(t->rows, 3);
        QCOMPARE(t->cols, 3);
        w.act(QStringLiteral("tbmargin.Wide"))->trigger();
        for (const auto &c : t->cells) QCOMPARE(c.margins, QMarginsF(14.4, 14.4, 14.4, 14.4));
        ed->beginTextEdit(t->id, 0, 1, 1);
        w.act(QStringLiteral("tbmargin.None"))->trigger();
        QCOMPARE(t->cell(1, 1).margins, QMarginsF());
        QCOMPARE(t->cell(0, 0).margins, QMarginsF(14.4, 14.4, 14.4, 14.4));
        // In a cell, Delete Rows deletes that row.
        w.act(QStringLiteral("tbl.delRow"))->trigger();
        QCOMPARE(t->rows, 2);
    }

    // Several table cells at once, as in Publisher: dragging across cells, or
    // Shift+click, selects a block, shaded the way selected text is; a block
    // that touches part of a merged cell takes the whole cell.
    void tableCellsSelectedAsABlock()
    {
        // The rectangle between two cells grows over every merged cell it touches.
        jp::TableItem model;
        model.rows = 4;
        model.cols = 4;
        model.colW = QVector<double>(4, 50);
        model.rowH = QVector<double>(4, 20);
        model.cells.resize(16);
        model.cell(1, 1).rowSpan = 2;
        model.cell(1, 1).colSpan = 2;
        for (auto [r, c] : {std::pair{1, 2}, {2, 1}, {2, 2}}) model.cell(r, c).covered = true;
        QCOMPARE(model.cellsBetween(0, 0, 1, 1), (jp::CellRange{0, 0, 2, 2}));
        QCOMPARE(model.cellsBetween(2, 2, 3, 3), (jp::CellRange{1, 1, 3, 3}));   // given in any order
        QCOMPARE(model.cellsBetween(3, 3, 2, 0), (jp::CellRange{1, 0, 3, 3}));
        QCOMPARE(model.cellsBetween(0, 0, 0, 3), (jp::CellRange{0, 0, 0, 3}));
        int row = -1, col = -1;
        model.cellAt(QPointF(120, 50), &row, &col);   // under the merged cell
        QCOMPARE(std::make_pair(row, col), std::make_pair(1, 1));
        model.cellAt(QPointF(-30, 500), &row, &col);   // outside: the nearest cell
        QCOMPARE(std::make_pair(row, col), std::make_pair(3, 0));

        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        jp::Editor *ed = w.editor();
        auto made = std::static_pointer_cast<jp::TableItem>(ed->newTable(QRectF(72, 72, 360, 160), 4, 4));
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        jp::Canvas *cv = w.canvas();
        QTest::qWait(50);
        auto at = [&](int r, int c) { return cv->pageToView(table()->transform().map(table()->cellRect(r, c).center())).toPoint(); };
        auto block = [&] {
            const jp::CellRange b = ed->cellBlock().range;
            return QVector<int>{b.r0, b.c0, b.r1, b.c1};
        };
        auto drag = [&](const QPoint &from, const QPoint &to) {
            QTest::mousePress(cv->viewport(), Qt::LeftButton, Qt::NoModifier, from);
            for (int k = 1; k <= 4; ++k) {
                const QPoint p = from + (to - from) * k / 4;
                QMouseEvent mv(QEvent::MouseMove, QPointF(p), cv->viewport()->mapToGlobal(QPointF(p)), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(cv->viewport(), &mv);
            }
            QTest::mouseRelease(cv->viewport(), Qt::LeftButton, Qt::NoModifier, to);
        };

        // A click, or a drag inside one cell, is text editing as before.
        QTest::mouseClick(cv->viewport(), Qt::LeftButton, Qt::NoModifier, at(3, 3));
        QVERIFY(ed->isEditingText());
        QVERIFY(!ed->hasCellBlock());
        drag(at(3, 3) - QPoint(8, 0), at(3, 3) + QPoint(8, 0));
        QVERIFY(ed->isEditingText());
        QVERIFY(!ed->hasCellBlock());
        const QImage plain = cv->viewport()->grab().toImage();

        // Dragging across cells selects the block: the table stays the selected object.
        drag(at(0, 0), at(1, 2));
        QVERIFY(!ed->isEditingText());
        QCOMPARE(ed->selection(), QStringList{id});
        QCOMPARE(block(), (QVector<int>{0, 0, 1, 2}));
        // Shaded as selected text is, in the cells of the block only.
        const QImage shaded = cv->viewport()->grab().toImage();
        for (auto [r, c] : {std::pair{0, 0}, {0, 2}, {1, 1}})
            QVERIFY2(shaded.pixelColor(at(r, c)) != plain.pixelColor(at(r, c)), qPrintable(QStringLiteral("%1,%2").arg(r).arg(c)));
        for (auto [r, c] : {std::pair{2, 0}, {0, 3}, {2, 2}, {1, 3}})
            QVERIFY2(shaded.pixelColor(at(r, c)) == plain.pixelColor(at(r, c)), qPrintable(QStringLiteral("%1,%2").arg(r).arg(c)));
        // A drag up and to the left, shaded where it is and nowhere else.
        drag(at(2, 2), at(1, 1));
        QCOMPARE(block(), (QVector<int>{1, 1, 2, 2}));
        const QImage lower = cv->viewport()->grab().toImage();
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) lower.save(qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/table-cells.png"));
        for (auto [r, c] : {std::pair{1, 1}, {2, 2}, {1, 2}})
            QVERIFY2(lower.pixelColor(at(r, c)) != plain.pixelColor(at(r, c)), qPrintable(QStringLiteral("%1,%2").arg(r).arg(c)));
        for (auto [r, c] : {std::pair{0, 0}, {0, 1}, {3, 1}, {2, 3}})
            QVERIFY2(lower.pixelColor(at(r, c)) == plain.pixelColor(at(r, c)), qPrintable(QStringLiteral("%1,%2").arg(r).arg(c)));
        drag(at(2, 2), at(2, 2) + QPoint(3, 0));
        QVERIFY(ed->isEditingText());   // not far enough to leave the cell

        // Shift+click extends from the cell the cursor is in, then from where the block began.
        QTest::mouseClick(cv->viewport(), Qt::LeftButton, Qt::NoModifier, at(1, 1));
        QTest::mouseClick(cv->viewport(), Qt::LeftButton, Qt::ShiftModifier, at(2, 3));
        QVERIFY(!ed->isEditingText());
        QCOMPARE(block(), (QVector<int>{1, 1, 2, 3}));
        QTest::mouseClick(cv->viewport(), Qt::LeftButton, Qt::ShiftModifier, at(0, 0));
        QCOMPARE(block(), (QVector<int>{0, 0, 1, 1}));
        QTest::mouseClick(cv->viewport(), Qt::LeftButton, Qt::ShiftModifier, at(3, 1));
        QCOMPARE(block(), (QVector<int>{1, 1, 3, 1}));
        // A click in a cell ends the block and puts the cursor there.
        QTest::mouseClick(cv->viewport(), Qt::LeftButton, Qt::NoModifier, at(2, 2));
        QVERIFY(ed->isEditingText());
        QVERIFY(!ed->hasCellBlock());
        QCOMPARE(std::make_pair(ed->textTarget().row, ed->textTarget().col), std::make_pair(2, 2));

        // A block that touches part of a merged cell grows to hold all of it.
        ed->change(QStringLiteral("Merge"), [&] {
            table()->cell(1, 1).rowSpan = 2;
            table()->cell(1, 1).colSpan = 2;
            for (auto [r, c] : {std::pair{1, 2}, {2, 1}, {2, 2}}) table()->cell(r, c).covered = true;
        });
        drag(at(0, 0), at(1, 1));
        QCOMPARE(block(), (QVector<int>{0, 0, 2, 2}));
        ed->selectCells(id, 2, 0, 2, 1);   // the second is under the merged cell
        QCOMPARE(block(), (QVector<int>{1, 0, 2, 2}));
        QTest::mouseClick(cv->viewport(), Qt::LeftButton, Qt::ShiftModifier, at(3, 3));
        QCOMPARE(block(), (QVector<int>{1, 0, 3, 3}));
        // Esc puts the block away and leaves the table selected; Enter types in the cell the block began with.
        QTest::keyClick(cv, Qt::Key_Escape);
        QVERIFY(!ed->hasCellBlock());
        QCOMPARE(ed->selection(), QStringList{id});
        ed->selectCells(id, 3, 2, 1, 1);
        QTest::keyClick(cv, Qt::Key_Return);
        QVERIFY(ed->isEditingText());
        QCOMPARE(std::make_pair(ed->textTarget().row, ed->textTarget().col), std::make_pair(3, 2));
        // Choosing another object, or selecting the table again, puts the block away.
        ed->selectCells(id, 0, 0, 1, 0);
        ed->select(id);
        QVERIFY(!ed->hasCellBlock());
        ed->selectCells(id, 0, 0, 1, 0);
        ed->clearSelection();
        QVERIFY(!ed->hasCellBlock());
    }

    // Table Layout > Select has Select Row and Select Column beside Select
    // Cell and Select Table, reached by the KeyTips of the Select button: a
    // row or column of the cell the cursor is in, or every row or column a
    // block covers (merged cells whole).
    void tableSelectRowAndColumn()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        jp::Editor *ed = w.editor();
        auto made = std::static_pointer_cast<jp::TableItem>(ed->newTable(QRectF(72, 72, 360, 160), 4, 3));
        ed->addItem(made);
        const QString id = made->id;
        auto block = [&] {
            const jp::CellRange b = ed->cellBlock().range;
            return QVector<int>{b.r0, b.c0, b.r1, b.c1};
        };
        const int steps = ed->undoStack()->index();
        ed->beginTextEdit(id, 0, 1, 2);
        w.act(QStringLiteral("tbl.selectRow"))->trigger();
        QVERIFY(!ed->isEditingText());
        QCOMPARE(ed->selection(), QStringList{id});
        QCOMPARE(block(), (QVector<int>{1, 0, 1, 2}));
        ed->beginTextEdit(id, 0, 1, 2);
        w.act(QStringLiteral("tbl.selectCol"))->trigger();
        QCOMPARE(block(), (QVector<int>{0, 2, 3, 2}));
        // From a block: every row, or every column, that it covers.
        ed->selectCells(id, 1, 0, 2, 1);
        w.act(QStringLiteral("tbl.selectRow"))->trigger();
        QCOMPARE(block(), (QVector<int>{1, 0, 2, 2}));
        QCOMPARE(ed->cellBlock().anchorRow, 1);
        ed->selectCells(id, 1, 0, 2, 1);
        w.act(QStringLiteral("tbl.selectCol"))->trigger();
        QCOMPARE(block(), (QVector<int>{0, 0, 3, 1}));
        // A merged cell in the row comes whole.
        auto *t = static_cast<jp::TableItem *>(ed->doc()->item(id));
        t->cell(2, 1).rowSpan = 2;
        t->cell(3, 1).covered = true;
        ed->beginTextEdit(id, 0, 2, 0);
        w.act(QStringLiteral("tbl.selectRow"))->trigger();
        QCOMPARE(block(), (QVector<int>{2, 0, 3, 2}));
        // With the whole table selected there is no cell to take a row from.
        ed->select(id);
        w.act(QStringLiteral("tbl.selectRow"))->trigger();
        QVERIFY(!ed->hasCellBlock());
        w.act(QStringLiteral("tbl.selectCol"))->trigger();
        QVERIFY(!ed->hasCellBlock());
        // Selecting is not an edit: nothing to undo.
        QCOMPARE(ed->undoStack()->index(), steps);
        // Select Cell from a block goes to its first cell, all its text selected.
        ed->doc()->storyDoc(t->cell(1, 0).storyId)->setPlainText(QStringLiteral("hello"));
        ed->selectCells(id, 1, 0, 2, 1);
        w.act(QStringLiteral("tbl.selectCell"))->trigger();
        QVERIFY(ed->isEditingText());
        QCOMPARE(std::make_pair(ed->textTarget().row, ed->textTarget().col), std::make_pair(1, 0));
        QCOMPARE(ed->cursor().selectedText(), QStringLiteral("hello"));

        // The Select button's menu lists all four, and its KeyTips open it.
        auto *r = w.findChild<jp::Ribbon *>();
        QVERIFY(r);
        ed->select(id);
        const QString described = r->describe(true);
        const QRegularExpression line(QStringLiteral("ribbon\\.tableSelect.*menu=\\[([^\\]]*)\\].*keytip=(\\w+)"));
        const QRegularExpressionMatch m = line.match(described);
        QVERIFY2(m.hasMatch(), "the Select button");
        const QStringList entries = m.captured(1).split(QLatin1Char(','), Qt::SkipEmptyParts);
        QCOMPARE(entries.size(), 4);
        for (const char *cmd : {"tbl.selectCell", "tbl.selectRow", "tbl.selectCol", "tbl.selectTable"})
            QVERIFY2(m.captured(1).contains(QLatin1String(cmd)), cmd);
        QCOMPARE(m.captured(2), QStringLiteral("SL"));
        QStringList shown;
        QTimer::singleShot(300, [&] {
            if (auto *menu = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
                for (QAction *a : menu->actions()) shown << a->objectName();
                menu->close();
            }
        });
        QTest::keyPress(&w, Qt::Key_Alt);
        QTest::keyRelease(&w, Qt::Key_Alt);
        for (Qt::Key k : {Qt::Key_J, Qt::Key_L, Qt::Key_S, Qt::Key_L}) QTest::keyClick(&w, k);
        QCOMPARE(shown, (QStringList{"tbl.selectCell", "tbl.selectRow", "tbl.selectCol", "tbl.selectTable"}));
    }

    // Every cell-formatting command applies to every cell in the block, as
    // one undo step: fill, borders, diagonals, alignment, cell margins, and
    // the text's font, size, bold, italic, underline, color, and alignment.
    void tableBlockFormatsEveryCell()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        jp::Editor *ed = w.editor();
        auto made = std::static_pointer_cast<jp::TableItem>(ed->newTable(QRectF(72, 72, 360, 160), 4, 4));
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c) ed->doc()->storyDoc(table()->cell(r, c).storyId)->setPlainText(QStringLiteral("cell %1,%2").arg(r).arg(c));
        for (auto &cl : table()->cells) {   // plain cells, so every command has something to change
            cl.fill = jp::Fill();
            cl.border = jp::CellBorder();
        }
        auto inBlock = [](int r, int c) { return r >= 1 && r <= 2 && c >= 1 && c <= 2; };
        auto story = [&](int r, int c) { return ed->doc()->storyDoc(table()->cell(r, c).storyId); };
        auto charFormat = [&](int r, int c) {
            QTextCursor cur(story(r, c));
            cur.setPosition(3);
            return cur.charFormat();
        };
        auto cellJson = [&](int r, int c) {
            const jp::TableCell &cl = table()->cell(r, c);
            return QJsonObject{{"fill", cl.fill.toJson()}, {"top", cl.border.top.toJson()}, {"bottom", cl.border.bottom.toJson()}, {"left", cl.border.left.toJson()},
                               {"right", cl.border.right.toJson()}, {"diag", cl.diagonal}, {"valign", int(cl.valign)},
                               {"margins", QJsonArray{cl.margins.left(), cl.margins.top(), cl.margins.right(), cl.margins.bottom()}}};
        };
        auto cellsJson = [&] {
            QVector<QJsonObject> all;
            for (int r = 0; r < 4; ++r)
                for (int c = 0; c < 4; ++c) all << cellJson(r, c);
            return all;
        };
        auto textFormats = [&] {
            QVector<QString> all;
            for (int r = 0; r < 4; ++r)
                for (int c = 0; c < 4; ++c) {
                    const QTextCharFormat f = charFormat(r, c);
                    all << QStringLiteral("%1 %2 %3 %4 %5 %6 %7").arg(f.fontWeight()).arg(f.fontItalic()).arg(int(f.underlineStyle())).arg(f.fontFamilies().toStringList().join(QLatin1Char('+')))
                               .arg(f.fontPointSize()).arg(f.stringProperty(jp::tp::ColorRefP), QString::number(int(story(r, c)->firstBlock().blockFormat().alignment())));
                }
            return all;
        };
        // Runs one command on the block: it changes exactly the block's cells, in
        // one undo step, and the block stays selected, also after undoing it.
        // `prepare` first puts every cell in the state the command should change.
        auto command = [&](const char *what, const std::function<void()> &run, const std::function<bool(int, int)> &changed, const std::function<void(jp::TableCell &)> &prepare = nullptr) {
            if (prepare)
                for (auto &cl : table()->cells) prepare(cl);
            ed->selectCells(id, 1, 1, 2, 2);
            w.refreshUi();   // the ribbon catches up: text commands are on for selected cells
            const int steps = ed->undoStack()->index();
            const auto cellsBefore = cellsJson();
            const auto textBefore = textFormats();
            run();
            QVERIFY2(ed->undoStack()->index() == steps + 1, what);
            QVERIFY2(ed->hasCellBlock(), what);
            const auto cellsAfter = cellsJson();
            const auto textAfter = textFormats();
            for (int r = 0; r < 4; ++r)
                for (int c = 0; c < 4; ++c) {
                    const int k = r * 4 + c;
                    const bool did = cellsAfter[k] != cellsBefore[k] || textAfter[k] != textBefore[k];
                    QVERIFY2(did == inBlock(r, c), qPrintable(QStringLiteral("%1 at %2,%3").arg(QLatin1String(what)).arg(r).arg(c)));
                    if (inBlock(r, c)) QVERIFY2(changed(r, c), qPrintable(QStringLiteral("%1 at %2,%3").arg(QLatin1String(what)).arg(r).arg(c)));
                }
            ed->undo();
            QVERIFY2(cellsJson() == cellsBefore && textFormats() == textBefore, what);
            QVERIFY2(ed->hasCellBlock(), what);
        };
        auto cellFill = [&]() -> jp::ColorButton * {
            for (auto *b : w.findChildren<jp::ColorButton *>())
                if (b->toolTip() == QStringLiteral("Cell Fill")) return b;
            return nullptr;
        };
        QVERIFY(cellFill());

        const jp::ColorRef red = jp::ColorRef::fromString(QStringLiteral("#ff0000"));
        command("fill", [&] { Q_EMIT cellFill()->colorPicked(red); }, [&](int r, int c) { return table()->cell(r, c).fill == jp::Fill::solid(red); });
        command("fill removed", [&] { Q_EMIT cellFill()->colorPicked(jp::ColorRef()); }, [&](int r, int c) { return table()->cell(r, c).fill.isNone(); },
                [&](jp::TableCell &cl) { cl.fill = jp::Fill::solid(red); });
        command("diagonal down", [&] { w.act(QStringLiteral("tbl.diagDown"))->trigger(); }, [&](int r, int c) { return table()->cell(r, c).diagonal == 1; });
        command("diagonal up", [&] { w.act(QStringLiteral("tbl.diagUp"))->trigger(); }, [&](int r, int c) { return table()->cell(r, c).diagonal == 2; });
        command("align bottom", [&] { w.act(QStringLiteral("valign.2"))->trigger(); }, [&](int r, int c) { return table()->cell(r, c).valign == jp::VAlign::Bottom; });
        command("align middle", [&] { w.act(QStringLiteral("valign.1"))->trigger(); }, [&](int r, int c) { return table()->cell(r, c).valign == jp::VAlign::Middle; });
        command("margins", [&] { w.act(QStringLiteral("tbmargin.Wide"))->trigger(); }, [&](int r, int c) { return table()->cell(r, c).margins == QMarginsF(14.4, 14.4, 14.4, 14.4); });
        command("all borders", [&] { w.act(QStringLiteral("border.all"))->trigger(); }, [&](int r, int c) {
            const jp::CellBorder &b = table()->cell(r, c).border;
            return !b.top.isNone() && !b.bottom.isNone() && !b.left.isNone() && !b.right.isNone();
        });
        command("no borders", [&] { w.act(QStringLiteral("border.none"))->trigger(); }, [&](int r, int c) {
            const jp::CellBorder &b = table()->cell(r, c).border;
            return b.top.isNone() && b.bottom.isNone() && b.left.isNone() && b.right.isNone();
        }, [](jp::TableCell &cl) { cl.border.top = cl.border.bottom = cl.border.left = cl.border.right = jp::Stroke::line(jp::ColorRef::fromString(QStringLiteral("#000000"))); });
        for (auto &cl : table()->cells) cl.border = jp::CellBorder();
        // Outside Borders draws round the block, not round each cell in it.
        ed->selectCells(id, 1, 1, 2, 2);
        const int plain = ed->undoStack()->index();
        w.act(QStringLiteral("border.none"))->trigger();
        w.act(QStringLiteral("border.outside"))->trigger();
        QVERIFY(!table()->cell(1, 1).border.top.isNone() && !table()->cell(1, 1).border.left.isNone());
        QVERIFY(table()->cell(1, 1).border.right.isNone() && table()->cell(1, 1).border.bottom.isNone());
        QVERIFY(!table()->cell(2, 2).border.bottom.isNone() && !table()->cell(2, 2).border.right.isNone());
        QVERIFY(table()->cell(2, 2).border.left.isNone() && table()->cell(2, 2).border.top.isNone());
        ed->undoStack()->setIndex(plain);

        // The text of every cell in the block, whole.
        command("bold", [&] { w.act(QStringLiteral("fmt.bold"))->trigger(); }, [&](int r, int c) { return charFormat(r, c).fontWeight() >= QFont::DemiBold; });
        command("italic", [&] { w.act(QStringLiteral("fmt.italic"))->trigger(); }, [&](int r, int c) { return charFormat(r, c).fontItalic(); });
        command("underline", [&] { w.act(QStringLiteral("fmt.underline"))->trigger(); }, [&](int r, int c) { return charFormat(r, c).underlineStyle() != QTextCharFormat::NoUnderline; });
        command("font", [&] { ed->setFontFamily(QStringLiteral("DejaVu Serif")); }, [&](int r, int c) { return charFormat(r, c).fontFamilies().toStringList() == QStringList{"DejaVu Serif"}; });
        command("size", [&] { ed->setFontSize(23); }, [&](int r, int c) { return charFormat(r, c).fontPointSize() == 23.0; });
        command("color", [&] { ed->setTextColor(red); }, [&](int r, int c) { return charFormat(r, c).stringProperty(jp::tp::ColorRefP) == red.toString(); });
        command("center", [&] { w.act(QStringLiteral("para.center"))->trigger(); }, [&](int r, int c) { return story(r, c)->firstBlock().blockFormat().alignment().testFlag(Qt::AlignHCenter); });
        command("right", [&] { w.act(QStringLiteral("para.right"))->trigger(); }, [&](int r, int c) { return story(r, c)->firstBlock().blockFormat().alignment().testFlag(Qt::AlignRight); });
        // All the text of a cell, not just its start.
        ed->doc()->storyDoc(table()->cell(1, 1).storyId)->setPlainText(QStringLiteral("one\ntwo"));
        ed->selectCells(id, 1, 1, 2, 2);
        w.refreshUi();
        w.act(QStringLiteral("fmt.bold"))->trigger();
        QTextCursor last(story(1, 1));
        last.movePosition(QTextCursor::End);
        QVERIFY(last.charFormat().fontWeight() >= QFont::DemiBold);
        // The Home tab's boxes show the block's first cell.
        ed->selectCells(id, 1, 1, 1, 1);
        ed->setFontSize(31);
        ed->selectCells(id, 1, 1, 2, 2);
        QCOMPARE(ed->currentCharFormat().fontPointSize(), 31.0);
        ed->selectCells(id, 2, 2, 3, 3);
        QVERIFY(ed->currentCharFormat().fontPointSize() != 31.0);

        // With the cursor in one cell, a cell command still takes only that cell.
        for (auto &cl : table()->cells) cl.fill = jp::Fill();
        ed->beginTextEdit(id, 0, 3, 3);
        Q_EMIT cellFill()->colorPicked(red);
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c) QCOMPARE(table()->cell(r, c).fill == jp::Fill::solid(red), r == 3 && c == 3);
        // A merged cell in the block is bordered as the one cell it is: its far edges, not its inner ones.
        ed->endTextEdit();
        for (auto &cl : table()->cells) cl.border = jp::CellBorder();
        table()->cell(1, 1).rowSpan = 2;
        table()->cell(1, 1).colSpan = 2;
        for (auto [r, c] : {std::pair{1, 2}, {2, 1}, {2, 2}}) table()->cell(r, c).covered = true;
        ed->selectCells(id, 0, 0, 2, 2);
        w.act(QStringLiteral("border.outside"))->trigger();
        const jp::CellBorder &merged = table()->cell(1, 1).border;
        QVERIFY(merged.right.isNone() == false && merged.bottom.isNone() == false);
        QVERIFY(merged.top.isNone() && merged.left.isNone());
        QVERIFY(!table()->cell(0, 0).border.top.isNone() && table()->cell(0, 0).border.bottom.isNone());
    }

    // Merge Cells merges the block; Delete Rows and Delete Columns remove
    // every row and column it covers, merged cells kept consistent; the
    // Insert commands go beside the block.
    void tableBlockMergesAndDeletes()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto made = std::static_pointer_cast<jp::TableItem>(ed->newTable(QRectF(72, 72, 400, 200), 4, 4));
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        auto text = [&](int r, int c) { return ed->doc()->storyDoc(table()->cell(r, c).storyId)->toPlainText(); };
        auto fill = [&] {
            for (int r = 0; r < table()->rows; ++r)
                for (int c = 0; c < table()->cols; ++c) ed->doc()->storyDoc(table()->cell(r, c).storyId)->setPlainText(QStringLiteral("%1%2").arg(r).arg(c));
        };
        fill();

        // Merge the block: one cell with the text of all, in reading order.
        ed->selectCells(id, 1, 1, 2, 2);
        int steps = ed->undoStack()->index();
        w.act(QStringLiteral("tbl.merge"))->trigger();
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        QCOMPARE(table()->cell(1, 1).rowSpan, 2);
        QCOMPARE(table()->cell(1, 1).colSpan, 2);
        for (auto [r, c] : {std::pair{1, 2}, {2, 1}, {2, 2}}) QVERIFY(table()->cell(r, c).covered);
        QVERIFY(!table()->cell(1, 1).covered);
        QCOMPARE(text(1, 1), QStringLiteral("11\n12\n21\n22"));
        QVERIFY(text(2, 2).isEmpty());   // the text moved
        QVERIFY(table()->cell(0, 0).rowSpan == 1 && !table()->cell(0, 1).covered && !table()->cell(3, 3).covered);
        ed->undo();
        QCOMPARE(table()->cell(1, 1).rowSpan, 1);
        QVERIFY(!table()->cell(2, 2).covered);
        QCOMPARE(text(2, 2), QStringLiteral("22"));

        // A row of cells merges into one; a block over a merged cell takes all of it.
        ed->selectCells(id, 0, 0, 0, 3);
        w.act(QStringLiteral("tbl.merge"))->trigger();
        QCOMPARE(table()->cell(0, 0).colSpan, 4);
        QCOMPARE(text(0, 0), QStringLiteral("00\n01\n02\n03"));
        ed->selectCells(id, 0, 1, 1, 1);   // touches the merged row
        w.act(QStringLiteral("tbl.merge"))->trigger();
        QCOMPARE(table()->cell(0, 0).rowSpan, 2);
        QCOMPARE(table()->cell(0, 0).colSpan, 4);
        QCOMPARE(text(0, 0), QStringLiteral("00\n01\n02\n03\n10\n11\n12\n13"));
        // Split Cells puts a merged cell in a block back.
        ed->selectCells(id, 0, 0, 0, 0);
        w.act(QStringLiteral("tbl.split"))->trigger();
        QCOMPARE(table()->cell(0, 0).colSpan, 1);
        QVERIFY(!table()->cell(1, 3).covered);
        ed->undo();
        ed->undo();
        ed->undo();
        QCOMPARE(table()->cell(0, 0).colSpan, 1);

        // Delete Rows: every row the block covers, in one step.
        ed->selectCells(id, 1, 0, 2, 1);
        steps = ed->undoStack()->index();
        w.act(QStringLiteral("tbl.delRow"))->trigger();
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        QCOMPARE(table()->rows, 2);
        QCOMPARE(table()->rowH.size(), 2);
        QCOMPARE(table()->cells.size(), 8);
        QCOMPARE(text(0, 0), QStringLiteral("00"));
        QCOMPARE(text(1, 3), QStringLiteral("33"));
        QCOMPARE(table()->rect.height(), table()->rowH[0] + table()->rowH[1]);
        ed->undo();
        QCOMPARE(table()->rows, 4);
        QCOMPARE(text(2, 3), QStringLiteral("23"));

        // Delete Columns likewise, the table keeping its width.
        const double width = table()->rect.width();
        ed->selectCells(id, 0, 1, 1, 2);
        steps = ed->undoStack()->index();
        w.act(QStringLiteral("tbl.delCol"))->trigger();
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        QCOMPARE(table()->cols, 2);
        QCOMPARE(table()->colW.size(), 2);
        QCOMPARE(text(2, 0), QStringLiteral("20"));
        QCOMPARE(text(2, 1), QStringLiteral("23"));
        QVERIFY(std::abs(table()->rect.width() - width) < 0.001);
        ed->undo();
        QCOMPARE(table()->cols, 4);

        // A table keeps at least one row and one column.
        ed->selectCells(id, 0, 0, 3, 3);
        steps = ed->undoStack()->index();
        w.act(QStringLiteral("tbl.delRow"))->trigger();
        w.act(QStringLiteral("tbl.delCol"))->trigger();
        QCOMPARE(table()->rows, 4);
        QCOMPARE(table()->cols, 4);
        QCOMPARE(ed->undoStack()->index(), steps);

        // Insert goes above or below, left or right, of the whole block.
        ed->selectCells(id, 1, 1, 2, 2);
        w.act(QStringLiteral("tbl.insAbove"))->trigger();
        QCOMPARE(table()->rows, 5);
        QCOMPARE(text(2, 0), QStringLiteral("10"));
        ed->undo();
        ed->selectCells(id, 1, 1, 2, 2);
        w.act(QStringLiteral("tbl.insBelow"))->trigger();
        QCOMPARE(table()->rows, 5);
        QCOMPARE(text(2, 0), QStringLiteral("20"));
        QCOMPARE(text(3, 0), QString());
        QCOMPARE(text(4, 0), QStringLiteral("30"));
        ed->undo();
        ed->selectCells(id, 1, 1, 2, 2);
        w.act(QStringLiteral("tbl.insLeft"))->trigger();
        QCOMPARE(table()->cols, 5);
        QCOMPARE(text(1, 2), QStringLiteral("11"));
        ed->undo();
        ed->selectCells(id, 1, 1, 2, 2);
        w.act(QStringLiteral("tbl.insRight"))->trigger();
        QCOMPARE(table()->cols, 5);
        QCOMPARE(text(1, 2), QStringLiteral("12"));
        QCOMPARE(text(1, 3), QString());
        QCOMPARE(text(1, 4), QStringLiteral("13"));
        ed->undo();

        // Delete or Backspace with a block clears the cells' text, in one step, and keeps the table.
        ed->selectCells(id, 1, 1, 2, 2);
        steps = ed->undoStack()->index();
        QTest::keyClick(w.canvas(), Qt::Key_Delete);
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        QVERIFY(ed->doc()->item(id));
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c) QCOMPARE(text(r, c).isEmpty(), r >= 1 && r <= 2 && c >= 1 && c <= 2);
        QVERIFY(ed->hasCellBlock());
        ed->undo();
        QCOMPARE(text(1, 1), QStringLiteral("11"));

        // A merged cell reaching into the deleted rows shrinks; one whose first row goes starts in the next.
        ed->change(QStringLiteral("Merge"), [&] {
            table()->cell(1, 3).rowSpan = 3;
            table()->cell(2, 3).covered = table()->cell(3, 3).covered = true;
        });
        ed->selectCells(id, 0, 0, 1, 0);   // rows 0 and 1; the merged cell began in row 1
        w.act(QStringLiteral("tbl.delRow"))->trigger();
        QCOMPARE(table()->rows, 2);
        QVERIFY(!table()->cell(0, 3).covered);
        QCOMPARE(table()->cell(0, 3).rowSpan, 2);
        QVERIFY(table()->cell(1, 3).covered);
        QCOMPARE(text(0, 3), QStringLiteral("13"));   // it keeps its text
        ed->undo();
        QCOMPARE(table()->rows, 4);
        ed->selectCells(id, 2, 0, 2, 0);   // a row inside it
        w.act(QStringLiteral("tbl.delRow"))->trigger();
        QCOMPARE(table()->rows, 3);
        QCOMPARE(table()->cell(1, 3).rowSpan, 2);
        QVERIFY(table()->cell(2, 3).covered);
        ed->undo();
        ed->selectCells(id, 1, 3, 3, 3);   // all its rows
        w.act(QStringLiteral("tbl.delRow"))->trigger();
        QCOMPARE(table()->rows, 1);
        QVERIFY(!table()->cell(0, 3).covered);
        ed->undo();
        // The columns of a merged cell go together.
        ed->change(QStringLiteral("Merge"), [&] {
            table()->cell(0, 1).colSpan = 2;
            table()->cell(0, 2).covered = true;
        });
        ed->selectCells(id, 0, 2, 0, 2);   // grows to columns 1 and 2
        w.act(QStringLiteral("tbl.delCol"))->trigger();
        QCOMPARE(table()->cols, 2);
        ed->undo();
        QCOMPARE(table()->cols, 4);
    }

    // A row or column inserted through a merged cell makes that cell bigger,
    // and the new cells under it are covered (the new row had two live cells
    // inside the merge, and the rows below it covered cells with nothing over
    // them). Then edits in any order, undone and redone, keep the grid whole.
    void tableEditsKeepMergedCellsWhole()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto made = std::static_pointer_cast<jp::TableItem>(ed->newTable(QRectF(72, 72, 300, 150), 3, 3));
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        // What is wrong with the grid, if anything: every cell lies under exactly one
        // live cell's span, and spans stay inside the table.
        auto problem = [&]() -> QString {
            const jp::TableItem *t = table();
            if (!t) return QStringLiteral("the table is gone");
            if (t->rows < 1 || t->cols < 1 || t->cells.size() != t->rows * t->cols || t->colW.size() != t->cols || t->rowH.size() != t->rows)
                return QStringLiteral("sizes: %1x%2, %3 cells, %4 widths, %5 heights").arg(t->rows).arg(t->cols).arg(t->cells.size()).arg(t->colW.size()).arg(t->rowH.size());
            QVector<int> under(t->cells.size(), 0);
            for (int r = 0; r < t->rows; ++r)
                for (int c = 0; c < t->cols; ++c) {
                    const jp::TableCell &cl = t->cell(r, c);
                    if (!ed->doc()->storyDoc(cl.storyId)) return QStringLiteral("(%1,%2) has no text").arg(r).arg(c);
                    if (cl.covered) continue;
                    if (cl.rowSpan < 1 || cl.colSpan < 1 || r + cl.rowSpan > t->rows || c + cl.colSpan > t->cols)
                        return QStringLiteral("(%1,%2) spans %3x%4 in a %5x%6 table").arg(r).arg(c).arg(cl.rowSpan).arg(cl.colSpan).arg(t->rows).arg(t->cols);
                    for (int rr = r; rr < r + cl.rowSpan; ++rr)
                        for (int cc = c; cc < c + cl.colSpan; ++cc) {
                            ++under[rr * t->cols + cc];
                            if ((rr != r || cc != c) && !t->cell(rr, cc).covered) return QStringLiteral("(%1,%2) is live under the cell at (%3,%4)").arg(rr).arg(cc).arg(r).arg(c);
                        }
                }
            for (int i = 0; i < under.size(); ++i)
                if (under[i] != 1) return QStringLiteral("(%1,%2) is under %3 cells").arg(i / t->cols).arg(i % t->cols).arg(under[i]);
            return QString();
        };
        auto mergeBlock = [&] {
            ed->selectCells(id, 0, 0, 1, 1);
            w.act(QStringLiteral("tbl.merge"))->trigger();
            QCOMPARE(table()->cell(0, 0).rowSpan, 2);
        };

        // Insert Below with the cursor beside a merged cell: the merged cell reaches the new row.
        mergeBlock();
        ed->beginTextEdit(id, 0, 0, 2);
        w.act(QStringLiteral("tbl.insBelow"))->trigger();
        QCOMPARE(table()->rows, 4);
        QCOMPARE(table()->cell(0, 0).rowSpan, 3);
        QCOMPARE(table()->cell(0, 0).colSpan, 2);
        QVERIFY(table()->cell(2, 0).covered && table()->cell(2, 1).covered);
        QVERIFY(!table()->cell(2, 2).covered);
        QVERIFY(!table()->cell(3, 0).covered && table()->cell(3, 0).rowSpan == 1);
        QCOMPARE(problem(), QString());
        ed->undo();
        QCOMPARE(table()->rows, 3);
        QCOMPARE(table()->cell(0, 0).rowSpan, 2);
        QCOMPARE(problem(), QString());
        // Above the merge's first row it is only moved down; above its second row it grows.
        ed->beginTextEdit(id, 0, 0, 2);
        w.act(QStringLiteral("tbl.insAbove"))->trigger();
        QCOMPARE(table()->cell(0, 0).rowSpan, 1);
        QCOMPARE(table()->cell(1, 0).rowSpan, 2);
        QCOMPARE(problem(), QString());
        ed->undo();
        ed->beginTextEdit(id, 0, 1, 2);
        w.act(QStringLiteral("tbl.insAbove"))->trigger();
        QCOMPARE(table()->rows, 4);
        QCOMPARE(table()->cell(0, 0).rowSpan, 3);
        QVERIFY(table()->cell(1, 0).covered && table()->cell(1, 1).covered && !table()->cell(1, 2).covered);
        QCOMPARE(problem(), QString());
        ed->undo();
        // The same for columns.
        ed->beginTextEdit(id, 0, 2, 0);
        w.act(QStringLiteral("tbl.insRight"))->trigger();
        QCOMPARE(table()->cols, 4);
        QCOMPARE(table()->cell(0, 0).colSpan, 3);
        QCOMPARE(table()->cell(0, 0).rowSpan, 2);
        QVERIFY(table()->cell(0, 2).covered && table()->cell(1, 2).covered && !table()->cell(2, 2).covered);
        QCOMPARE(problem(), QString());
        ed->undo();
        ed->beginTextEdit(id, 0, 2, 1);
        w.act(QStringLiteral("tbl.insLeft"))->trigger();
        QCOMPARE(table()->cols, 4);
        QCOMPARE(table()->cell(0, 0).colSpan, 3);
        QVERIFY(table()->cell(0, 1).covered && table()->cell(1, 1).covered && !table()->cell(2, 1).covered);
        QCOMPARE(problem(), QString());
        ed->undo();
        QCOMPARE(problem(), QString());

        // Edits in any order (fixed seeds), undone and redone, never break the grid.
        const char *actions[] = {"tbl.merge", "tbl.split", "tbl.delRow", "tbl.delCol", "tbl.insAbove", "tbl.insBelow", "tbl.insLeft", "tbl.insRight", "edit.undo", "edit.redo"};
        int insertsInMerges = 0;   // inserts made while the table had merged cells
        for (const uint seed : {1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u}) {
            QRandomGenerator rng(seed);
            ed->undoStack()->clear();   // so Undo never goes back past this seed's first edit
            for (int step = 0; step < 150; ++step) {
                const jp::TableItem *t = table();
                const int roll = rng.bounded(100);
                QString what;
                if (roll < 40) {
                    ed->selectCells(id, rng.bounded(t->rows), rng.bounded(t->cols), rng.bounded(t->rows), rng.bounded(t->cols));
                    what = QStringLiteral("select");
                } else if (roll < 50) {
                    ed->beginTextEdit(id, 0, rng.bounded(t->rows), rng.bounded(t->cols));
                    what = QStringLiteral("cursor");
                } else {
                    const QString a = QString::fromLatin1(actions[rng.bounded(int(sizeof actions / sizeof *actions))]);
                    if (a.startsWith(QLatin1String("tbl.ins")) && (t->rows > 7 || t->cols > 7)) continue;   // keep it small
                    if (a == QLatin1String("edit.undo") && !ed->undoStack()->canUndo()) continue;
                    if (a.startsWith(QLatin1String("tbl.ins")) && std::any_of(t->cells.cbegin(), t->cells.cend(), [](const jp::TableCell &c) { return c.covered; })) ++insertsInMerges;
                    w.act(a)->trigger();
                    what = a;
                }
                QVERIFY2(problem().isEmpty(), qPrintable(QStringLiteral("seed %1, step %2 (%3): %4").arg(seed).arg(step).arg(what, problem())));
                if (ed->hasCellBlock()) {
                    const jp::CellRange b = ed->cellBlock().range;
                    QVERIFY2(b.r1 < table()->rows && b.c1 < table()->cols, qPrintable(QStringLiteral("seed %1, step %2 (%3): the selected cells are outside the table").arg(seed).arg(step).arg(what)));
                }
            }
        }
        QVERIFY2(insertsInMerges >= 30, qPrintable(QStringLiteral("only %1 inserts were made in a merged table").arg(insertsInMerges)));   // the seeds do reach the case
    }

    // Align Top, Middle, and Bottom set every cell when only the table is
    // selected (as Text Direction and Cell Margins do; they did nothing), and
    // just the selected cells otherwise.
    void tableVerticalAlignmentWithTheTableSelected()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto made = std::static_pointer_cast<jp::TableItem>(ed->newTable(QRectF(72, 72, 300, 150), 3, 3));
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        ed->select(id);
        QVERIFY(!ed->hasCellBlock());
        w.act(QStringLiteral("valign.1"))->trigger();
        for (const auto &c : table()->cells) QCOMPARE(c.valign, jp::VAlign::Middle);
        ed->undo();
        for (const auto &c : table()->cells) QCOMPARE(c.valign, jp::VAlign::Top);
        ed->selectCells(id, 1, 1, 1, 2);
        w.act(QStringLiteral("valign.2"))->trigger();
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c) QCOMPARE(table()->cell(r, c).valign, r == 1 && c >= 1 ? jp::VAlign::Bottom : jp::VAlign::Top);
    }

    // Table Layout > Size has Height and Width boxes for the whole table (and
    // none for a row or a column), Grow to Fit Text as a check box, all with
    // KeyTips and names. Typing a width scales the columns in proportion,
    // typing a height scales the rows; with Lock aspect ratio the other
    // follows. Each is one undo step.
    void tableSizeBoxes()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        jp::Editor *ed = w.editor();
        auto made = std::static_pointer_cast<jp::TableItem>(ed->newTable(QRectF(72, 72, 400, 400), 4, 4));
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        auto near = [](const QVector<double> &got, const QVector<double> &want) {
            if (got.size() != want.size()) return false;
            for (int i = 0; i < got.size(); ++i)
                if (std::abs(got[i] - want[i]) > 1e-6) return false;
            return true;
        };
        ed->change(QStringLiteral("Sizes"), [&] {
            table()->colW = {100, 50, 150, 100};
            table()->rowH = {40, 120, 80, 160};
            table()->syncRect();
        });
        auto *r = w.findChild<jp::Ribbon *>();
        QVERIFY(r && r->tab(QStringLiteral("Table Layout")));
        jp::RibbonGroup *size = nullptr;
        for (jp::RibbonGroup *g : r->tab(QStringLiteral("Table Layout"))->groups())
            if (g->title() == QStringLiteral("Size")) size = g;
        QVERIFY(size);
        // The table's own two boxes, and no box for a row's height or a column's width.
        QCOMPARE(r->tab(QStringLiteral("Table Layout"))->findChildren<jp::MeasureSpin *>().size(), 2);
        jp::MeasureSpin *widthBox = nullptr, *heightBox = nullptr;
        for (auto *s : size->findChildren<jp::MeasureSpin *>()) (s->toolTip() == QStringLiteral("Table Width") ? widthBox : heightBox) = s;
        QVERIFY(widthBox && heightBox);
        auto *grow = size->findChild<QCheckBox *>();
        QVERIFY(grow);
        QCOMPARE(grow->text(), QStringLiteral("Grow to Fit Text"));
        for (QWidget *c : {static_cast<QWidget *>(widthBox), static_cast<QWidget *>(heightBox), static_cast<QWidget *>(grow)})
            QVERIFY2(!jp::keytip(c).isEmpty(), qPrintable(c->toolTip()));
        QCOMPARE(widthBox->accessibleName(), QStringLiteral("Table Width"));
        QCOMPARE(heightBox->accessibleName(), QStringLiteral("Table Height"));

        ed->select(id);
        w.refreshUi();
        QCOMPARE(widthBox->value(), 400.0);
        QCOMPARE(heightBox->value(), 400.0);
        QVERIFY(grow->isChecked());

        // A width: the columns scale in proportion, the rows stay.
        int steps = ed->undoStack()->index();
        widthBox->setValue(200);
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        QVERIFY(near(table()->colW, {50, 25, 75, 50}));
        QVERIFY(near(table()->rowH, {40, 120, 80, 160}));
        QCOMPARE(table()->rect.size(), QSizeF(200, 400));
        ed->undo();
        QVERIFY(near(table()->colW, {100, 50, 150, 100}));
        // A height: the rows scale; it works while typing in a cell, too.
        ed->beginTextEdit(id, 0, 2, 1);
        w.refreshUi();
        steps = ed->undoStack()->index();
        heightBox->setValue(240);
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        QVERIFY(near(table()->rowH, {24, 72, 48, 96}));
        QVERIFY(near(table()->colW, {100, 50, 150, 100}));
        QCOMPARE(table()->rect.size(), QSizeF(400, 240));
        ed->undo();
        QVERIFY(near(table()->rowH, {40, 120, 80, 160}));
        // Locked, the other size follows, and the shown sizes follow the table.
        ed->change(QStringLiteral("Lock"), [&] { table()->lockSize = true; });
        ed->select(id);
        w.refreshUi();   // the boxes show the table again
        steps = ed->undoStack()->index();
        widthBox->setValue(200);
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        QVERIFY(near(table()->colW, {50, 25, 75, 50}));
        QVERIFY(near(table()->rowH, {20, 60, 40, 80}));
        w.refreshUi();
        QCOMPARE(heightBox->value(), 200.0);
        // A row too short for its text grows (Grow to Fit Text), whatever was typed.
        ed->change(QStringLiteral("Text"), [&] {
            ed->doc()->storyDoc(table()->cell(0, 0).storyId)->setPlainText(QStringLiteral("one\ntwo\nthree\nfour"));
            table()->lockSize = false;
        });
        w.refreshUi();
        heightBox->setValue(40);
        QVERIFY2(table()->rowH[0] > 20.0, qPrintable(QString::number(table()->rowH[0])));   // 4 lines of text do not fit in 4 points
        // Grow to Fit Text is a check box that follows the table, and one step.
        ed->select(id);
        w.refreshUi();
        QVERIFY(grow->isChecked());
        steps = ed->undoStack()->index();
        grow->click();
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        QVERIFY(!table()->growToFit);
        QVERIFY(!grow->isChecked());
        ed->undo();
        w.refreshUi();
        QVERIFY(table()->growToFit);
        QVERIFY(grow->isChecked());
    }

    // Format Table: the Alignment group's launcher and the table's right-click
    // menu open it, with the shape tabs and Cell Properties (vertical
    // alignment, the four cell margins, and turning the text), which act on
    // the selected cells, the cell the cursor is in, or all of them. Lock
    // aspect ratio stays with the table.
    void tableFormatTable()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        jp::Editor *ed = w.editor();
        auto made = std::static_pointer_cast<jp::TableItem>(ed->newTable(QRectF(72, 72, 400, 200), 4, 4));
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        auto *r = w.findChild<jp::Ribbon *>();
        jp::RibbonGroup *align = nullptr;
        for (jp::RibbonGroup *g : r->tab(QStringLiteral("Table Layout"))->groups())
            if (g->title() == QStringLiteral("Alignment")) align = g;
        QVERIFY(align && align->launcher());
        QVERIFY(!jp::keytip(align->launcher()).isEmpty());
        ed->select(id);
        r->showTab(r->tab(QStringLiteral("Table Layout")));
        w.refreshUi();
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) w.grab(QRect(0, 0, 1400, 170)).save(qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/table-layout.png"));

        // The launcher: the dialog is Format Table, on Cell Properties.
        QString title, shown;
        QStringList tabNames;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            title = d->windowTitle();
            if (auto *tabs = d->findChild<QTabWidget *>()) {
                shown = tabs->tabText(tabs->currentIndex());
                for (int i = 0; i < tabs->count(); ++i) tabNames << tabs->tabText(i);
            }
            d->reject();
        });
        align->launcher()->click();
        QCOMPARE(title, QStringLiteral("Format Table"));
        QCOMPARE(shown, QStringLiteral("Cell Properties"));
        QCOMPARE(tabNames, (QStringList{"Colors and Lines", "Size", "Layout", "Cell Properties", "Alt Text"}));

        // Cell Properties on a block: it shows the first cell's, and changes the block's cells only.
        ed->change(QStringLiteral("Set up"), [&] {
            table()->cell(1, 1).valign = jp::VAlign::Bottom;
            table()->cell(1, 1).margins = QMarginsF(5, 6, 7, 8);
        });
        double firstTop = 0;
        int firstAlign = -1;
        auto setCells = [&](int valign, double left, double right, double top, double bottom, bool turned, bool lock, bool report) {
            QTimer::singleShot(0, [=, &firstTop, &firstAlign] {
                auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!d) return;
                auto *va = d->findChild<QComboBox *>(QStringLiteral("cellValign"));
                auto spin = [&](const char *name) { return d->findChild<jp::MeasureSpin *>(QString::fromLatin1(name)); };
                auto *rotate = d->findChild<QCheckBox *>(QStringLiteral("cellRotate"));
                auto *lockBox = d->findChild<QCheckBox *>(QStringLiteral("lockAspect"));
                if (!va || !spin("cellMarginLeft") || !spin("cellMarginRight") || !spin("cellMarginTop") || !spin("cellMarginBottom") || !rotate || !lockBox) {
                    d->reject();   // a missing control is a failure the checks below report
                    return;
                }
                if (report) {
                    firstTop = spin("cellMarginTop")->value();
                    firstAlign = va->currentIndex();
                }
                va->setCurrentIndex(valign);
                spin("cellMarginLeft")->setValue(left);
                spin("cellMarginRight")->setValue(right);
                spin("cellMarginTop")->setValue(top);
                spin("cellMarginBottom")->setValue(bottom);
                rotate->setChecked(turned);
                lockBox->setChecked(lock);
                d->accept();
            });
        };
        ed->selectCells(id, 1, 1, 2, 2);
        int steps = ed->undoStack()->index();
        setCells(1, 10, 11, 12, 13, true, true, true);
        w.act(QStringLiteral("tbl.format"))->trigger();
        QCOMPARE(firstTop, 6.0);
        QCOMPARE(firstAlign, 2);
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        for (int rr = 0; rr < 4; ++rr)
            for (int cc = 0; cc < 4; ++cc) {
                const jp::TableCell &c = table()->cell(rr, cc);
                const bool in = rr >= 1 && rr <= 2 && cc >= 1 && cc <= 2;
                QCOMPARE(c.margins == QMarginsF(10, 12, 11, 13), in);
                QCOMPARE(c.valign == jp::VAlign::Middle, in);
                QCOMPARE(c.vertical, in);
            }
        QVERIFY(table()->lockSize);
        QVERIFY(ed->hasCellBlock());   // the block stays selected
        ed->undo();
        QVERIFY(!table()->lockSize);
        QCOMPARE(table()->cell(2, 2).margins, QMarginsF(2.88, 2.88, 2.88, 2.88));
        // The cell the cursor is in; and with only the table selected, every cell.
        ed->beginTextEdit(id, 0, 3, 3);
        steps = ed->undoStack()->index();
        setCells(2, 3, 3, 3, 3, true, false, false);
        w.act(QStringLiteral("tbl.format"))->trigger();
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        for (int rr = 0; rr < 4; ++rr)
            for (int cc = 0; cc < 4; ++cc) QCOMPARE(table()->cell(rr, cc).valign == jp::VAlign::Bottom && table()->cell(rr, cc).vertical, rr == 3 && cc == 3);
        ed->undo();
        ed->select(id);
        setCells(1, 4, 4, 4, 4, true, false, false);
        w.act(QStringLiteral("tbl.format"))->trigger();
        for (const auto &c : table()->cells) QVERIFY(c.valign == jp::VAlign::Middle && c.vertical && c.margins == QMarginsF(4, 4, 4, 4));

        // The right-click menu offers Format Table for a table, selected or typed in; other objects keep Format Object.
        jp::Canvas *cv = w.canvas();
        auto menuFor = [&](const QPointF &pagePoint) {
            QStringList ids;
            QTimer::singleShot(100, [&] {
                if (auto *m = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
                    for (QAction *a : m->actions()) ids << a->objectName();
                    m->close();
                }
            });
            const QPoint at = cv->pageToView(pagePoint).toPoint();
            QContextMenuEvent ce(QContextMenuEvent::Mouse, at, cv->viewport()->mapToGlobal(at));
            QApplication::sendEvent(cv->viewport(), &ce);
            return ids;
        };
        ed->select(id);
        QStringList items = menuFor(QPointF(100, 100));
        QVERIFY2(items.contains(QStringLiteral("tbl.format")) && !items.contains(QStringLiteral("obj.format")), qPrintable(items.join(QLatin1Char(' '))));
        ed->beginTextEdit(id, 0, 1, 1);
        items = menuFor(QPointF(100, 100));
        QVERIFY2(items.contains(QStringLiteral("tbl.format")) && !items.contains(QStringLiteral("obj.format")), qPrintable(items.join(QLatin1Char(' '))));
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 400, 200, 60), QStringLiteral("text")));
        ed->addItem(box);
        items = menuFor(QPointF(100, 420));
        QVERIFY2(items.contains(QStringLiteral("obj.format")) && !items.contains(QStringLiteral("tbl.format")), qPrintable(items.join(QLatin1Char(' '))));
    }

    // Lock aspect ratio (Format Table > Size) keeps the table's proportions
    // when a corner handle is dragged.
    void tableLockAspectRatioKeepsProportions()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        jp::Editor *ed = w.editor();
        auto made = std::static_pointer_cast<jp::TableItem>(ed->newTable(QRectF(72, 72, 360, 160), 4, 4));
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        jp::Canvas *cv = w.canvas();
        auto cornerDrag = [&](const QPoint &by) {
            ed->select(id);
            const QPoint from = cv->pageToView(table()->rect.bottomRight()).toPoint();
            QTest::mousePress(cv->viewport(), Qt::LeftButton, Qt::NoModifier, from);
            for (int k = 1; k <= 4; ++k) {
                const QPoint p = from + by * k / 4;
                QMouseEvent mv(QEvent::MouseMove, QPointF(p), cv->viewport()->mapToGlobal(QPointF(p)), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(cv->viewport(), &mv);
            }
            QTest::mouseRelease(cv->viewport(), Qt::LeftButton, Qt::NoModifier, from + by);
        };
        const double ratio = 360.0 / 160.0;
        cornerDrag(QPoint(80, 10));
        QVERIFY2(std::abs(table()->rect.width() / table()->rect.height() - ratio) > 0.2, "unlocked, a corner drag changes the proportions");
        ed->undo();
        QCOMPARE(table()->rect.size(), QSizeF(360, 160));
        ed->change(QStringLiteral("Lock"), [&] { table()->lockSize = true; });
        cornerDrag(QPoint(80, 10));
        QVERIFY2(table()->rect.width() > 400, qPrintable(QString::number(table()->rect.width())));
        QVERIFY2(std::abs(table()->rect.width() / table()->rect.height() - ratio) < 0.01, qPrintable(QString::number(table()->rect.width() / table()->rect.height())));
        // The rows and columns scaled along.
        double total = 0;
        for (double c : table()->colW) total += c;
        QVERIFY(std::abs(total - table()->rect.width()) < 0.01);
    }

    // Text Direction, Hyphenation, and the alignment buttons' check marks work
    // on the selected cells, as the other cell commands do (they took the
    // cell the cursor was in, or every cell).
    void tableTextDirectionAndHyphenationFollowTheBlock()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto made = std::static_pointer_cast<jp::TableItem>(ed->newTable(QRectF(72, 72, 400, 200), 4, 4));
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        auto inBlock = [](int r, int c) { return r >= 1 && r <= 2 && c >= 1 && c <= 2; };
        ed->selectCells(id, 1, 1, 2, 2);
        w.refreshUi();
        int steps = ed->undoStack()->index();
        w.act(QStringLiteral("tb.direction"))->trigger();
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c) QCOMPARE(table()->cell(r, c).vertical, inBlock(r, c));
        QVERIFY(ed->hasCellBlock());
        w.refreshUi();
        QVERIFY(w.act(QStringLiteral("tb.direction"))->isChecked());   // every cell of the block is turned
        ed->selectCells(id, 0, 0, 1, 1);
        w.refreshUi();
        QVERIFY(!w.act(QStringLiteral("tb.direction"))->isChecked());   // this block is not
        ed->selectCells(id, 1, 1, 2, 2);
        w.act(QStringLiteral("tb.direction"))->trigger();   // turned back
        for (const auto &c : table()->cells) QVERIFY(!c.vertical);
        // Hyphenation: the dialog shows the block's first cell and sets every cell of the table (its text is one story).
        ed->change(QStringLiteral("Set up"), [&] { table()->cell(1, 1).hyphenate = false; });
        bool shownOn = true;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            for (auto *box : d->findChildren<QCheckBox *>()) {
                shownOn = box->isChecked();
                box->setChecked(false);
                break;
            }
            d->accept();
        });
        ed->selectCells(id, 1, 1, 2, 2);
        steps = ed->undoStack()->index();
        w.act(QStringLiteral("rev.hyphenation"))->trigger();
        QVERIFY(!shownOn);   // (1,1) is not hyphenated, and the dialog showed it
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        for (const auto &c : table()->cells) QVERIFY(!c.hyphenate);
        QVERIFY(ed->hasCellBlock());
        // The vertical-alignment buttons are checked for the selected cells.
        ed->change(QStringLiteral("Align"), [&] {
            for (int r = 1; r <= 2; ++r)
                for (int c = 1; c <= 2; ++c) table()->cell(r, c).valign = jp::VAlign::Middle;
        });
        ed->selectCells(id, 1, 1, 2, 2);
        w.refreshUi();
        QVERIFY(w.act(QStringLiteral("valign.1"))->isChecked() && !w.act(QStringLiteral("valign.0"))->isChecked() && !w.act(QStringLiteral("valign.2"))->isChecked());
        ed->selectCells(id, 0, 1, 2, 2);   // mixed: none is checked
        w.refreshUi();
        QVERIFY(!w.act(QStringLiteral("valign.0"))->isChecked() && !w.act(QStringLiteral("valign.1"))->isChecked() && !w.act(QStringLiteral("valign.2"))->isChecked());
    }

    // A table with no rows or no columns (a crafted file) has no cell to look
    // up; asking for one reads nothing.
    // A damaged file's span as large as an int holds is cut to the table: the
    // cell covers the rows there are (the sum overflowed and gave it none).
    void hugeCellSpansStayInTheTable()
    {
        QJsonObject cell{{"story", "s"}, {"rs", 2147483647}, {"cs", 2147483647}, {"margins", QJsonArray{0, 0, 0, 0}}};
        QJsonObject o{{"type", "table"}, {"rows", 2}, {"cols", 2}, {"colW", QJsonArray{50, 60}}, {"rowH", QJsonArray{20, 30}},
                      {"cells", QJsonArray{cell, QJsonObject{{"story", "a"}}, QJsonObject{{"story", "b"}}, QJsonObject{{"story", "c"}}}}};
        jp::TableItem t;
        t.fromJson(o);
        QCOMPARE(t.cell(0, 0).rowSpan, 2);
        QCOMPARE(t.cell(0, 0).colSpan, 2);
        QCOMPARE(t.cellRect(0, 0), QRectF(0, 0, 110, 50));
        t.cell(1, 1).rowSpan = std::numeric_limits<int>::max();   // whatever a cell says, the sum stays in the table
        QCOMPARE(t.cellRect(1, 1), QRectF(50, 20, 60, 30));
    }

    void emptyTableHasNoCellToFind()
    {
        for (const auto &size : {std::pair<int, int>{0, 0}, {0, 3}, {2, 0}}) {
            jp::TableItem table;
            table.rows = size.first;
            table.cols = size.second;
            table.colW = QVector<double>(size.second, 100.0);
            table.rowH = QVector<double>(size.first, 20.0);
            table.cells.resize(size.first * size.second);
            int row = 5, col = 5;
            table.cellAt(QPointF(10, 10), &row, &col);
            QCOMPARE(row, 0);
            QCOMPARE(col, 0);
            QVERIFY(!table.cellsBetween(0, 0, 1, 1).valid());
        }
        jp::TableItem real;
        real.rows = real.cols = 2;
        real.colW = {100, 100};
        real.rowH = {20, 20};
        real.cells.resize(4);
        int row = 0, col = 0;
        real.cellAt(QPointF(150, 30), &row, &col);
        QCOMPARE(row, 1);
        QCOMPARE(col, 1);
    }

    // Insert > Table of Contents: the Heading 1-3 paragraphs in reading
    // order with the page each starts on (one pushed to page 2 by the text
    // before it), entries indented by level with a dot leader to the page
    // number; Update follows renamed headings; one undo takes it all back.
    void tableOfContents()
    {
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        doc->addPage(1, doc->pages[0]->masterId);
        auto head = [&](QTextDocument *sd, const QString &styleName, const QString &text) {
            QTextCursor c(sd);
            c.movePosition(QTextCursor::End);
            if (!sd->toPlainText().isEmpty()) c.insertBlock();
            QTextBlockFormat bf;
            bf.setProperty(jp::tp::StyleName, styleName);
            c.setBlockFormat(bf);
            c.insertText(text);
        };
        // Page 1: a box linked to one on page 2; enough text to push the
        // last heading over.
        auto a = std::make_shared<jp::TextItem>();
        a->rect = QRectF(72, 360, 468, 200);
        a->storyId = doc->createStory();
        auto b2 = std::make_shared<jp::TextItem>();
        b2->rect = QRectF(72, 72, 468, 600);
        b2->storyId = a->storyId;
        a->nextId = b2->id;
        doc->pages[0]->items.push_back(a);
        doc->pages[1]->items.push_back(b2);
        QTextDocument *sd = doc->storyDoc(a->storyId);
        head(sd, QStringLiteral("Heading 1"), QStringLiteral("Introduction"));
        for (int i = 0; i < 14; ++i) head(sd, QStringLiteral("Normal"), QStringLiteral("Body text that fills the first box. ").repeated(3));
        head(sd, QStringLiteral("Heading 2"), QStringLiteral("Background"));
        head(sd, QStringLiteral("Heading 3"), QStringLiteral("Earlier work"));
        head(sd, QStringLiteral("Heading 1"), QStringLiteral("Results"));
        w.editor()->setDocument(std::move(doc));
        jp::Editor *ed = w.editor();
        const QVector<jp::TocEntry> entries = jp::tableOfContentsEntries(*ed->doc());
        QCOMPARE(entries.size(), 4);
        QCOMPARE(entries[0].text, QStringLiteral("Introduction"));
        QCOMPARE(entries[0].page, 1);
        QCOMPARE(entries[1].level, 2);
        QCOMPARE(entries[3].text, QStringLiteral("Results"));
        QCOMPARE(entries[3].page, 2);
        // Insert from page 1, which has text: on a new page 2, so the
        // later headings move to page 3.
        ed->setCurrentPage(0);
        jp::insertTableOfContents(ed);
        QCOMPARE(int(ed->doc()->pages.size()), 3);
        QCOMPARE(ed->currentPage(), 1);
        QCOMPARE(int(ed->doc()->pages[1]->items.size()), 1);
        auto *toc = dynamic_cast<jp::TextItem *>(ed->doc()->pages[1]->items.front().get());
        QVERIFY(toc);
        QTextDocument *td = ed->doc()->storyDoc(toc->storyId);
        const QStringList lines = td->toPlainText().split(QChar::ParagraphSeparator).join('\n').split('\n');
        QCOMPARE(lines.value(0), QStringLiteral("Contents"));
        QCOMPARE(lines.value(1), QStringLiteral("Introduction\t1"));
        QCOMPARE(lines.value(4), QStringLiteral("Results\t3"));
        const QTextBlock second = td->begin().next().next();   // Background, level 2
        QCOMPARE(second.blockFormat().intProperty(jp::tp::TocLevel), 2);
        QCOMPARE(second.blockFormat().leftMargin(), 18.0);
        QCOMPARE(second.blockFormat().stringProperty(jp::tp::TabLeaders), QStringLiteral("."));
        QCOMPARE(second.blockFormat().tabPositions().value(0).type, QTextOption::RightTab);
        // Renamed, then updated.
        {
            QTextCursor c(sd->findBlockByNumber(17));
            c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            c.insertText(QStringLiteral("Findings"));
        }
        QCOMPARE(jp::updateTablesOfContents(ed), 1);
        QVERIFY(td->toPlainText().contains(QStringLiteral("Findings\t3")));
        QVERIFY(!td->toPlainText().contains(QStringLiteral("Results")));
        // One undo per command: the update, then the whole insert.
        ed->undo();
        ed->undo();
        QCOMPARE(int(ed->doc()->pages.size()), 2);
        // At the cursor, before the text there: the table on its own
        // paragraphs, the text after it.
        ed->beginTextEdit(a->id, 0);
        jp::insertTableOfContents(ed);
        QTextDocument *story = ed->doc()->storyDoc(a->storyId);
        QCOMPARE(story->begin().text(), QStringLiteral("Contents"));
        bool introAfter = false;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
            if (!b.blockFormat().hasProperty(jp::tp::TocLevel)) {
                introAfter = b.text() == QStringLiteral("Introduction");
                break;
            }
        QVERIFY(introAfter);
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            ed->endTextEdit();
            w.resize(1200, 900);
            w.show();
            QTest::qWait(100);
            w.grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/toc.png");
        }
    }

    // A cell's Text Direction is kept in a .pub file, a merged cell's too.
    void pubTableCellTextDirectionKept()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TableItem>();
        t->rows = 3;
        t->cols = 2;
        t->colW = {120, 120};
        t->rowH = {90, 60, 80};
        t->rect = QRectF(72, 144, 0, 0);
        t->syncRect();
        t->cells.resize(6);
        for (auto &c : t->cells) c.storyId = doc->createStory(QStringLiteral("Region"));
        t->cell(0, 1).vertical = true;
        t->cell(2, 0).colSpan = 2;
        t->cell(2, 0).vertical = true;
        t->cell(2, 1).covered = true;
        doc->pages[0]->items.push_back(t);

        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("cell-direction.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/cell-direction.pub";
            QFile::remove(out);
            QFile::copy(path, out);
        }
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        const jp::TableItem *bt = nullptr;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Table) bt = static_cast<const jp::TableItem *>(it.get());
        });
        QVERIFY(bt);
        QCOMPARE(bt->rows, 3);
        QCOMPARE(bt->cols, 2);
        QVERIFY(!bt->cell(0, 0).vertical);
        QVERIFY(bt->cell(0, 1).vertical);
        QVERIFY(!bt->cell(1, 0).vertical);
        QVERIFY(!bt->cell(1, 1).vertical);
        QCOMPARE(bt->cell(2, 0).colSpan, 2);
        QVERIFY(bt->cell(2, 0).vertical);
    }

    // Publisher hyphenates a table's text as one story, so a .pub file keeps
    // one setting for the whole table: off when any cell is set off, and the
    // first zone that is not the default.
    void pubTableHyphenationKept()
    {
        auto reopened = [&](const QString &name, const std::function<void(jp::TableItem &)> &set, QString *err) {
            auto doc = jp::Document::blank(QSizeF(612, 792));
            auto t = std::make_shared<jp::TableItem>();
            t->rows = 2;
            t->cols = 2;
            t->colW = {70, 70};
            t->rowH = {60, 60};
            t->rect = QRectF(72, 144, 0, 0);
            t->syncRect();
            t->cells.resize(4);
            for (auto &c : t->cells) c.storyId = doc->createStory(QStringLiteral("Internationalization characteristics"));
            set(*t);
            doc->pages[0]->items.push_back(t);
            QTemporaryDir dir;
            const QString path = dir.filePath(name + QStringLiteral(".pub"));
            if (!jp::exportPublisher(*doc, path, err)) return std::unique_ptr<jp::Document>();
            if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
                const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/" + name + ".pub";
                QFile::remove(out);
                QFile::copy(path, out);
            }
            return jp::importPublisherFile(path, err);
        };
        auto tableOf = [](const jp::Document &d) {
            const jp::TableItem *bt = nullptr;
            jp::walkItems(d.pages[0]->items, [&](const jp::ItemPtr &it) {
                if (it->type() == jp::ItemType::Table) bt = static_cast<const jp::TableItem *>(it.get());
            });
            return bt;
        };
        // Verifies every cell of the table against one hyphenation setting.
        auto allCells = [](const jp::TableItem *bt, bool hyphenate, double zone) {
            for (const jp::TableCell &c : bt->cells)
                if (c.hyphenate != hyphenate || std::abs(c.hyphenZone - zone) > 0.01) return false;
            return true;
        };
        QString err;

        auto plain = reopened(QStringLiteral("hyphenation-default"), [](jp::TableItem &) {}, &err);
        QVERIFY2(plain, qPrintable(err));
        QVERIFY(tableOf(*plain) && allCells(tableOf(*plain), true, 18));

        auto off = reopened(QStringLiteral("hyphenation-off"), [](jp::TableItem &t) {
            for (auto &c : t.cells) { c.hyphenate = false; c.hyphenZone = 30; }
        }, &err);
        QVERIFY2(off, qPrintable(err));
        QVERIFY(tableOf(*off) && allCells(tableOf(*off), false, 30));

        auto oneOff = reopened(QStringLiteral("hyphenation-one-off"), [](jp::TableItem &t) { t.cell(1, 0).hyphenate = false; }, &err);
        QVERIFY2(oneOff, qPrintable(err));
        QVERIFY(tableOf(*oneOff) && allCells(tableOf(*oneOff), false, 18));

        auto zone = reopened(QStringLiteral("hyphenation-zone"), [](jp::TableItem &t) { t.cell(0, 1).hyphenZone = 36; }, &err);
        QVERIFY2(zone, qPrintable(err));
        QVERIFY(tableOf(*zone) && allCells(tableOf(*zone), true, 36));
    }

    // A cell's vertical alignment (top, middle, or bottom) is kept in a .pub
    // file, alongside its Text Direction, and a merged cell's too.
    void pubTableCellVerticalAlignKept()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TableItem>();
        t->rows = 3;
        t->cols = 3;
        t->colW = {100, 100, 100};
        t->rowH = {90, 60, 80};
        t->rect = QRectF(72, 144, 0, 0);
        t->syncRect();
        t->cells.resize(9);
        for (auto &c : t->cells) c.storyId = doc->createStory(QStringLiteral("Region"));
        t->cell(0, 0).valign = jp::VAlign::Middle;
        t->cell(0, 1).valign = jp::VAlign::Bottom;
        t->cell(0, 1).vertical = true;
        t->cell(1, 2).valign = jp::VAlign::Middle;
        t->cell(2, 0).colSpan = 2;
        t->cell(2, 0).valign = jp::VAlign::Bottom;
        t->cell(2, 1).covered = true;
        doc->pages[0]->items.push_back(t);

        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("cell-valign.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/cell-valign.pub";
            QFile::remove(out);
            QFile::copy(path, out);
        }
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        const jp::TableItem *bt = nullptr;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Table) bt = static_cast<const jp::TableItem *>(it.get());
        });
        QVERIFY(bt);
        QCOMPARE(bt->rows, 3);
        QCOMPARE(bt->cols, 3);
        QCOMPARE(int(bt->cell(0, 0).valign), int(jp::VAlign::Middle));
        QCOMPARE(int(bt->cell(0, 1).valign), int(jp::VAlign::Bottom));
        QVERIFY(bt->cell(0, 1).vertical);
        QVERIFY(!bt->cell(0, 0).vertical);
        QCOMPARE(int(bt->cell(0, 2).valign), int(jp::VAlign::Top));
        QCOMPARE(int(bt->cell(1, 0).valign), int(jp::VAlign::Top));
        QCOMPARE(int(bt->cell(1, 1).valign), int(jp::VAlign::Top));
        QCOMPARE(int(bt->cell(1, 2).valign), int(jp::VAlign::Middle));
        QCOMPARE(bt->cell(2, 0).colSpan, 2);
        QCOMPARE(int(bt->cell(2, 0).valign), int(jp::VAlign::Bottom));
        QCOMPARE(int(bt->cell(2, 2).valign), int(jp::VAlign::Top));
    }

    // A cell divided by a diagonal is kept in a .pub file: its direction, the
    // line along it (or none, when Publisher prints no line), and the text.
    void pubTableCellDiagonalsKept()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TableItem>();
        t->rows = 3;
        t->cols = 3;
        t->colW = {100, 100, 100};
        t->rowH = {90, 60, 80};
        t->rect = QRectF(72, 144, 0, 0);
        t->syncRect();
        t->cells.resize(9);
        const QStringList words = {"Alpha", "Bravo", "Charlie", "Delta", "Echo", "Foxtrot", "Golf", "Hotel", "India"};
        for (int i = 0; i < 9; ++i) {
            t->cells[i].storyId = doc->createStory(QStringLiteral("Region"));
            doc->storyDoc(t->cells[i].storyId)->setPlainText(words[i]);
        }
        t->cell(0, 0).diagonal = 1;
        t->cell(0, 0).diagonalLine = jp::Stroke::line(jp::ColorRef::rgb(Qt::red), 3);
        t->cell(0, 1).diagonal = 2;
        t->cell(0, 1).diagonalLine = jp::Stroke::line(jp::ColorRef::rgb(QColor(0, 128, 0)), 1.5);
        t->cell(0, 1).valign = jp::VAlign::Bottom;
        t->cell(1, 0).diagonal = 1;   // divided, with no line
        t->cell(1, 2).diagonal = 2;
        t->cell(1, 2).diagonalLine = jp::Stroke::line(jp::ColorRef::rgb(Qt::blue), 0.75);
        t->cell(2, 0).colSpan = 2;
        t->cell(2, 1).covered = true;
        doc->pages[0]->items.push_back(t);

        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("cell-diagonals.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/cell-diagonals.pub";
            QFile::remove(out);
            QFile::copy(path, out);
        }
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        const jp::TableItem *bt = nullptr;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Table) bt = static_cast<const jp::TableItem *>(it.get());
        });
        QVERIFY(bt);
        QCOMPARE(bt->rows, 3);
        QCOMPARE(bt->cols, 3);
        auto text = [&](int r, int c) { return back->storyDoc(bt->cell(r, c).storyId)->toPlainText(); };
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c) {
                if (bt->cell(r, c).covered) continue;
                QCOMPARE(text(r, c), words[r * 3 + c]);
            }
        QCOMPARE(bt->cell(0, 0).diagonal, 1);
        QCOMPARE(bt->cell(0, 1).diagonal, 2);
        QCOMPARE(bt->cell(1, 0).diagonal, 1);
        QCOMPARE(bt->cell(1, 2).diagonal, 2);
        QCOMPARE(bt->cell(0, 2).diagonal, 0);
        QCOMPARE(bt->cell(1, 1).diagonal, 0);
        QCOMPARE(bt->cell(2, 0).diagonal, 0);
        QCOMPARE(bt->cell(2, 2).diagonal, 0);
        QCOMPARE(bt->cell(2, 0).colSpan, 2);
        QCOMPARE(int(bt->cell(0, 1).valign), int(jp::VAlign::Bottom));
        const jp::Stroke red = bt->cell(0, 0).diagonalLine, green = bt->cell(0, 1).diagonalLine, blue = bt->cell(1, 2).diagonalLine;
        QVERIFY(!red.isNone() && std::abs(red.width - 3) < 0.05);
        QCOMPARE(red.color.resolve(back->colors), QColor(Qt::red));
        QVERIFY(!green.isNone() && std::abs(green.width - 1.5) < 0.05);
        QCOMPARE(green.color.resolve(back->colors), QColor(0, 128, 0));
        QVERIFY(!blue.isNone() && std::abs(blue.width - 0.75) < 0.05);
        QCOMPARE(blue.color.resolve(back->colors), QColor(Qt::blue));
        QVERIFY(bt->cell(1, 0).diagonalLine.isNone());
        QVERIFY(bt->cell(0, 2).diagonalLine.isNone());
    }

    // Each half of a divided cell has its own text, and a .pub file keeps them apart.
    void pubTableCellHalvesKeepTheirText()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TableItem>();
        t->rows = 2;
        t->cols = 2;
        t->colW = {150, 150};
        t->rowH = {90, 70};
        t->rect = QRectF(72, 144, 0, 0);
        t->syncRect();
        t->cells.resize(4);
        const QStringList first = {"Alpha", "Bravo", QString(), "Delta"}, second = {"Alpha two", "Bravo two\nand more", "Charlie two", QString()};
        for (int i = 0; i < 4; ++i) {
            t->cells[i].storyId = doc->createStory(first[i]);
            if (i < 3) {
                t->cells[i].diagonal = i == 1 ? 2 : 1;
                t->cells[i].storyId2 = doc->createStory(second[i]);
            }
        }
        doc->pages[0]->items.push_back(t);

        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("cell-halves.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/cell-halves.pub";
            QFile::remove(out);
            QFile::copy(path, out);
        }
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        const jp::TableItem *bt = nullptr;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Table) bt = static_cast<const jp::TableItem *>(it.get());
        });
        QVERIFY(bt);
        auto text = [&](const QString &story) { const QTextDocument *d = back->storyDoc(story); return d ? d->toPlainText() : QStringLiteral("(none)"); };
        for (int i = 0; i < 4; ++i) {
            const jp::TableCell &c = bt->cells[i];
            QCOMPARE(text(c.storyId), first[i]);
            if (i < 3) {
                QCOMPARE(c.diagonal, i == 1 ? 2 : 1);
                QCOMPARE(text(c.storyId2), second[i]);
            } else {
                QCOMPARE(c.diagonal, 0);
                QVERIFY(c.storyId2.isEmpty());
            }
        }
    }

    // A damaged file's cell division stays 0, 1, or 2: other numbers made the program treat a cell as divided while the
    // .pub writer, which knows only 1 and 2, left out its second half and that half's text.
    void tableDiagonalFromFileStaysInRange()
    {
        QJsonArray cells;
        for (int d : {0, 1, 2, 3, 99, -1, 2147483647}) {
            QJsonObject c;
            c[QStringLiteral("diag")] = d;
            cells.append(c);
        }
        QJsonObject o;
        o[QStringLiteral("rows")] = 1;
        o[QStringLiteral("cols")] = 7;
        o[QStringLiteral("cells")] = cells;
        jp::TableItem t;
        t.fromJson(o);
        QCOMPARE(t.cells.size(), 7);
        const int expected[] = {0, 1, 2, 2, 2, 0, 2};
        for (int i = 0; i < 7; ++i) QCOMPARE(t.cells[i].diagonal, expected[i]);
    }

    // A cell's Text Direction is read from the low byte of its 06, as its vertical alignment (07) and division (05) are:
    // stray bits above it (seen in 07 of a first cell) don't turn the cell upright.
    void pubTableCellTextDirectionReadsItsLowByte()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TableItem>();
        t->rows = 2;
        t->cols = 2;
        t->colW = {100, 100};
        t->rowH = {40, 40};
        t->rect = QRectF(36, 100, 0, 0);
        t->syncRect();
        t->cells.resize(4);
        for (jp::TableCell &c : t->cells) c.storyId = doc->createStory(QStringLiteral("Cell"));
        t->cell(0, 1).vertical = true;
        doc->pages[0]->items.push_back(t);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("direction.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        QFile in(path);
        QVERIFY(in.open(QIODevice::ReadOnly));
        jp::cfb::File file;
        QVERIFY(jp::cfb::read(in.readAll(), &file));
        in.close();
        QByteArray contents = file.stream(QStringLiteral("Contents"));
        // The one cell with a 06 (= 2, a u32) has stray bits above it.
        const QByteArray field("\x06\x20\x02\x00\x00\x00\x0a\x20", 8);
        QCOMPARE(contents.count(field), 1);
        contents.replace(contents.indexOf(field), 6, QByteArray("\x06\x20\x02\x6c\x00\x00", 6));
        QVERIFY(file.setStream(QStringLiteral("Contents"), contents));
        QFile out(path);
        QVERIFY(out.open(QIODevice::WriteOnly));
        out.write(jp::cfb::write(file));
        out.close();
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        const jp::TableItem *bt = nullptr;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Table) bt = static_cast<const jp::TableItem *>(it.get());
        });
        QVERIFY(bt);
        QVERIFY(bt->cell(0, 1).vertical);
        QVERIFY(!bt->cell(0, 0).vertical);
    }

    // One table that disagrees with itself (rows that its size entries don't match, a list of cells that isn't
    // there, or no row count) is left out of a .pub file, and the rest of the file opens.
    void pubTableThatDisagreesWithItselfIsLeftOut()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto box = std::make_shared<jp::TextItem>();
        box->rect = QRectF(36, 36, 300, 40);
        box->storyId = doc->createStory(QStringLiteral("Before the table"));
        doc->pages[0]->items.push_back(box);
        auto t = std::make_shared<jp::TableItem>();
        t->rows = 2;
        t->cols = 2;
        t->colW = {100, 100};
        t->rowH = {40, 40};
        t->rect = QRectF(36, 100, 0, 0);
        t->syncRect();
        t->cells.resize(4);
        for (jp::TableCell &c : t->cells) c.storyId = doc->createStory(QStringLiteral("Cell"));
        doc->pages[0]->items.push_back(t);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("table.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        QFile in(path);
        QVERIFY(in.open(QIODevice::ReadOnly));
        jp::cfb::File file;
        QVERIFY(jp::cfb::read(in.readAll(), &file));
        in.close();
        const QByteArray contents = file.stream(QStringLiteral("Contents"));
        // The table's chunk: 66 rows, 67 columns, 68 and 69 its size, then 6b, the cell list's number.
        const QByteArray fields("\x66\x20\x02\x00\x00\x00\x67\x20\x02\x00\x00\x00", 12);
        QCOMPARE(contents.count(fields), 1);
        const qsizetype at = contents.indexOf(fields);
        QCOMPARE(contents.mid(at + 24, 2), QByteArray("\x6b\x70", 2));
        auto reopened = [&](const std::function<void(QByteArray &)> &damage, bool *tableKept) {
            QByteArray c = contents;
            damage(c);
            jp::cfb::File copy = file;
            if (!copy.setStream(QStringLiteral("Contents"), c)) return false;
            const QString damaged = dir.filePath(QStringLiteral("damaged.pub"));
            QFile out(damaged);
            if (!out.open(QIODevice::WriteOnly)) return false;
            out.write(jp::cfb::write(copy));
            out.close();
            auto back = jp::importPublisherFile(damaged, &err);
            if (!back) return false;
            bool text = false;
            *tableKept = false;
            jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
                if (it->type() == jp::ItemType::Table) *tableKept = true;
                if (it->type() == jp::ItemType::Text)
                    text = back->storyDoc(static_cast<const jp::TextItem *>(it.get())->storyId)->toPlainText() == QStringLiteral("Before the table");
            });
            return text;
        };
        bool tableKept = true;
        QVERIFY2(reopened([](QByteArray &) {}, &tableKept) && tableKept, "the undamaged file");
        QVERIFY2(reopened([&](QByteArray &c) { c[at + 2] = 3; }, &tableKept), "three rows, two size entries");
        QVERIFY(!tableKept);
        QVERIFY2(reopened([&](QByteArray &c) { qToLittleEndian<quint32>(0x7ffffff0, c.data() + at + 26); }, &tableKept), "no cell list");
        QVERIFY(!tableKept);
        QVERIFY2(reopened([&](QByteArray &c) { c[at] = 0x6e; }, &tableKept), "no row count");
        QVERIFY(!tableKept);
    }

    // Every cell of a table is made when a .pub file is read, so a table over 256 rows or 256 columns (twice
    // Publisher's largest, and the .jpub reader's limit) is left out, and the rest of the file opens.
    void pubTableOverTheLimitIsLeftOut()
    {
        auto read = [&](int rows, int cols, bool *textKept) {
            auto doc = jp::Document::blank(QSizeF(612, 792));
            auto box = std::make_shared<jp::TextItem>();
            box->rect = QRectF(36, 36, 300, 40);
            box->storyId = doc->createStory(QStringLiteral("Before the table"));
            doc->pages[0]->items.push_back(box);
            auto t = std::make_shared<jp::TableItem>();
            t->rows = rows;
            t->cols = cols;
            t->colW = QVector<double>(cols, 2.0);
            t->rowH = QVector<double>(rows, 2.0);
            t->rect = QRectF(36, 100, 0, 0);
            t->syncRect();
            t->cells.resize(rows * cols);
            doc->pages[0]->items.push_back(t);
            QTemporaryDir dir;
            const QString path = dir.filePath(QStringLiteral("table-size.pub"));
            QString err;
            *textKept = false;
            if (!jp::exportPublisher(*doc, path, &err)) return QSize(-2, -2);
            auto back = jp::importPublisherFile(path, &err);
            if (!back) return QSize(-1, -1);
            QSize size(0, 0);
            jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
                if (it->type() == jp::ItemType::Table) {
                    auto *tb = static_cast<const jp::TableItem *>(it.get());
                    size = QSize(tb->cols, tb->rows);
                }
                if (it->type() == jp::ItemType::Text)
                    *textKept = back->storyDoc(static_cast<const jp::TextItem *>(it.get())->storyId)->toPlainText() == QStringLiteral("Before the table");
            });
            return size;
        };
        bool kept = false;
        QCOMPARE(read(2, 256, &kept), QSize(256, 2));
        QVERIFY(kept);
        QCOMPARE(read(256, 2, &kept), QSize(2, 256));
        QVERIFY(kept);
        QCOMPARE(read(2, 257, &kept), QSize(0, 0));
        QVERIFY(kept);
        QCOMPARE(read(257, 2, &kept), QSize(0, 0));
        QVERIFY(kept);
    }

    // The cell list's count is 16 bits, so a table of 65,536 cells (256 x 256), or fewer cells with many divided
    // ones (each divided cell is two entries), would wrap it to a small number. The cells come first, then as
    // many second halves as fit; the save says what it left out.
    void pubTableCellCountDoesNotWrap()
    {
        auto written = [&](int rows, int cols, int diagonal, QString *note) {
            auto doc = jp::Document::blank(QSizeF(612, 792));
            auto t = std::make_shared<jp::TableItem>();
            t->rows = rows;
            t->cols = cols;
            t->colW = QVector<double>(cols, 2.0);
            t->rowH = QVector<double>(rows, 2.0);
            t->rect = QRectF(36, 36, 0, 0);
            t->syncRect();
            t->cells.resize(rows * cols);
            for (jp::TableCell &c : t->cells) c.diagonal = diagonal;
            doc->pages[0]->items.push_back(t);
            QTemporaryDir dir;
            const QString path = dir.filePath(QStringLiteral("many-cells.pub"));
            if (!jp::exportPublisher(*doc, path, note)) return -2;
            // The cell list is the one list of records that holds the field 0e = 114300 (one count, a u16, then the list).
            const QByteArray c = jp::cfb::readStream(path, QStringLiteral("Contents"));
            int count = -1;
            for (qsizetype at = c.indexOf(QByteArray("\x02\xa0", 2)); at >= 4; at = c.indexOf(QByteArray("\x02\xa0", 2), at + 1)) {
                if (c.mid(at - 4, 2) != QByteArray("\x01\x18", 2)) continue;
                const qsizetype length = qFromLittleEndian<quint32>(c.constData() + at + 2);
                if (!c.mid(at + 6, length).contains(QByteArray("\x0e\x20\x7c\xbe\x01\x00", 6))) continue;
                count = qFromLittleEndian<quint16>(c.constData() + at - 2);
                break;
            }
            return count;
        };
        QString note;
        // 256 x 256: the last cell has no room.
        QCOMPARE(written(256, 256, 0, &note), 65535);
        QVERIFY2(note.contains(QStringLiteral("1 table cell")), qPrintable(note));
        // 182 x 182, all divided: 33,124 cells, and room for 32,411 of the 33,124 second halves.
        QCOMPARE(written(182, 182, 1, &note), 65535);
        QVERIFY2(note.contains(QStringLiteral("713 divided table cell")), qPrintable(note));
        // Under the limit, nothing is left out.
        QCOMPARE(written(100, 100, 1, &note), 20000);
        QVERIFY2(note.isEmpty(), qPrintable(note));
    }

    // A .pub file holds 1,000 table formatting shapes (borders, shadings, and diagonals, in all of a publication's
    // tables). A table with more keeps its diagonals, which are fewer and lose more, and loses borders; the save says how many.
    void pubTableDiagonalsComeBeforeBordersPastTheLimit()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TableItem>();
        t->rows = 40;
        t->cols = 12;
        t->colW = QVector<double>(12, 40.0);
        t->rowH = QVector<double>(40, 14.0);
        t->rect = QRectF(36, 36, 0, 0);
        t->syncRect();
        t->cells.resize(480);
        const jp::Stroke black = jp::Stroke::line(jp::ColorRef::rgb(Qt::black), 0.75);
        for (jp::TableCell &c : t->cells) c.border.top = c.border.bottom = c.border.left = c.border.right = black;
        for (int col = 0; col < 12; ++col) {
            t->cell(39, col).diagonal = 1;
            t->cell(39, col).diagonalLine = jp::Stroke::line(jp::ColorRef::rgb(Qt::red), 1);
        }
        doc->pages[0]->items.push_back(t);

        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("many-borders.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        // 41 x 12 rules across and 40 x 13 down, and 12 diagonals, make 1,024 shapes; 1,000 fit.
        QVERIFY2(err.contains(QStringLiteral("24 table")), qPrintable(err));
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        const jp::TableItem *bt = nullptr;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Table) bt = static_cast<const jp::TableItem *>(it.get());
        });
        QVERIFY(bt);
        QCOMPARE(bt->cells.size(), 480);
        int diagonals = 0, sides = 0;
        for (const jp::TableCell &c : bt->cells) {
            if (c.diagonal == 1 && !c.diagonalLine.isNone()) ++diagonals;
            for (const jp::Stroke &s : {c.border.top, c.border.bottom, c.border.left, c.border.right}) sides += !s.isNone();
        }
        QCOMPARE(diagonals, 12);
        QVERIFY2(sides > 1850 && sides < 1920, qPrintable(QString::number(sides)));
    }

    // Dividing a cell gives it a second, empty half; typing in a half goes to that half; taking the division
    // away puts the second half's text after the first's, as in Publisher; and Undo brings the halves back.
    void tableDivideCellKeepsTextInHalves()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto made = ed->newTable(QRectF(72, 72, 300, 100), 1, 2);
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        auto text = [&](const QString &story) { const QTextDocument *d = ed->doc()->storyDoc(story); return d ? d->toPlainText() : QStringLiteral("(none)"); };
        ed->doc()->storyDoc(table()->cell(0, 0).storyId)->setPlainText(QStringLiteral("One"));
        ed->selectCells(id, 0, 0, 0, 0);
        w.act(QStringLiteral("tbl.diagDown"))->trigger();
        QCOMPARE(table()->cell(0, 0).diagonal, 1);
        QVERIFY(!table()->cell(0, 0).storyId2.isEmpty());
        QCOMPARE(text(table()->cell(0, 0).storyId), QStringLiteral("One"));
        QCOMPARE(text(table()->cell(0, 0).storyId2), QString());
        QVERIFY(table()->cell(0, 1).storyId2.isEmpty());
        // The cell's row doesn't grow for text its halves can't hold.
        const double rowHeight = table()->rowH[0];
        ed->doc()->storyDoc(table()->cell(0, 0).storyId2)->setPlainText(QStringLiteral("a long text, which would take the whole cell and more, in a narrow half ").repeated(12));
        ed->fitTableRows(table());
        QCOMPARE(table()->rowH[0], rowHeight);
        ed->doc()->storyDoc(table()->cell(0, 0).storyId2)->setPlainText(QString());
        // Typing in the second half.
        ed->beginTextEdit(id, -1, 0, 0, 1);
        QCOMPARE(ed->textTarget().half, 1);
        QCOMPARE(ed->textTarget().storyId, table()->cell(0, 0).storyId2);
        ed->insertText(QStringLiteral("Two"));
        QCOMPARE(text(table()->cell(0, 0).storyId2), QStringLiteral("Two"));
        QCOMPARE(text(table()->cell(0, 0).storyId), QStringLiteral("One"));
        // Changing the direction keeps both texts.
        ed->selectCells(id, 0, 0, 0, 0);
        w.act(QStringLiteral("tbl.diagUp"))->trigger();
        QCOMPARE(table()->cell(0, 0).diagonal, 2);
        QCOMPARE(text(table()->cell(0, 0).storyId), QStringLiteral("One"));
        QCOMPARE(text(table()->cell(0, 0).storyId2), QStringLiteral("Two"));
        // Taking the division away joins them.
        w.act(QStringLiteral("tbl.diagNone"))->trigger();
        QCOMPARE(table()->cell(0, 0).diagonal, 0);
        QVERIFY(table()->cell(0, 0).storyId2.isEmpty());
        QCOMPARE(text(table()->cell(0, 0).storyId), QStringLiteral("OneTwo"));
        ed->undo();
        QCOMPARE(table()->cell(0, 0).diagonal, 2);
        QCOMPARE(text(table()->cell(0, 0).storyId), QStringLiteral("One"));
        QCOMPARE(text(table()->cell(0, 0).storyId2), QStringLiteral("Two"));
    }

    // A divided cell is two places to type: a click goes to the half it is in, Tab and Shift+Tab visit the
    // first half, the second, and then the next cell, and Right at the end of the first half goes on to the second.
    void tableDividedCellEditing()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        jp::Editor *ed = w.editor();
        auto made = ed->newTable(QRectF(72, 72, 400, 150), 1, 2);
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        ed->doc()->storyDoc(table()->cell(0, 0).storyId)->setPlainText(QStringLiteral("First"));
        ed->doc()->storyDoc(table()->cell(0, 1).storyId)->setPlainText(QStringLiteral("Next"));
        ed->selectCells(id, 0, 0, 0, 0);
        w.act(QStringLiteral("tbl.diagUp"))->trigger();   // "/": the first half is the upper left
        ed->doc()->storyDoc(table()->cell(0, 0).storyId2)->setPlainText(QStringLiteral("Second"));
        ed->endTextEdit();
        auto *cv = w.findChild<jp::Canvas *>();
        const QRectF cell = table()->cellRect(0, 0).translated(table()->rect.topLeft());
        auto click = [&](double u, double v) {
            QTest::mouseClick(cv->viewport(), Qt::LeftButton, {}, cv->pageToView(QPointF(cell.left() + u * cell.width(), cell.top() + v * cell.height())).toPoint());
        };
        click(0.1, 0.1);
        QVERIFY(ed->isEditingText());
        QCOMPARE(ed->textTarget().half, 0);
        QCOMPARE(ed->textTarget().storyId, table()->cell(0, 0).storyId);
        click(0.9, 0.9);
        QCOMPARE(ed->textTarget().half, 1);
        QCOMPARE(ed->textTarget().storyId, table()->cell(0, 0).storyId2);
        click(0.1, 0.1);
        QCOMPARE(ed->textTarget().half, 0);
        // Tab goes to the second half, then the next cell; Shift+Tab back.
        QTest::keyClick(cv, Qt::Key_Tab);
        QCOMPARE(ed->textTarget().half, 1);
        QCOMPARE(ed->textTarget().col, 0);
        QTest::keyClick(cv, Qt::Key_Tab);
        QCOMPARE(ed->textTarget().col, 1);
        QCOMPARE(ed->textTarget().half, 0);
        QTest::keyClick(cv, Qt::Key_Backtab, Qt::ShiftModifier);
        QCOMPARE(ed->textTarget().col, 0);
        QCOMPARE(ed->textTarget().half, 1);
        QTest::keyClick(cv, Qt::Key_Backtab, Qt::ShiftModifier);
        QCOMPARE(ed->textTarget().half, 0);
        // Right at the end of the first half goes to the start of the second, Left back to the end of the first.
        ed->beginTextEdit(id, -1, 0, 0, 0);
        QTest::keyClick(cv, Qt::Key_Right);
        QCOMPARE(ed->textTarget().half, 1);
        QCOMPARE(ed->cursor().position(), 0);
        QTest::keyClick(cv, Qt::Key_Left);
        QCOMPARE(ed->textTarget().half, 0);
        QCOMPARE(ed->cursor().position(), ed->editDoc()->characterCount() - 1);
        QTest::mouseClick(cv->viewport(), Qt::LeftButton, {}, QPoint(5, 5));   // the mouse in use again: no focus ring in the tests after
    }

    // A half's text wraps around the diagonal: the first half stays on its side of it, the second on its own.
    void tableDividedCellWrapsAroundTheDiagonal()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        const double width = 300, height = 200;
        for (int diagonal = 1; diagonal <= 2; ++diagonal) {
            auto t = std::make_shared<jp::TableItem>();
            t->rows = 1;
            t->cols = 1;
            t->colW = {width};
            t->rowH = {height};
            t->rect = QRectF(72, 72, 0, 0);
            t->syncRect();
            t->cells.resize(1);
            jp::TableCell &c = t->cell(0, 0);
            c.diagonal = diagonal;
            c.diagonalLine = jp::Stroke::none();
            const QString words = QStringLiteral("The quick brown fox jumps over the lazy dog and keeps running through the long green field until evening comes. ").repeated(2);
            auto story = [&](const QString &color) {
                const QString id = doc->createStory(words);
                QTextCursor cur(doc->storyDoc(id));
                cur.select(QTextCursor::Document);
                QTextCharFormat cf;
                cf.setProperty(jp::tp::ColorRefP, color);
                cur.mergeCharFormat(cf);
                return id;
            };
            c.storyId = story(QStringLiteral("#FF0000"));
            c.storyId2 = story(QStringLiteral("#0000FF"));
            doc->pages[0]->items.clear();
            doc->pages[0]->items.push_back(t);
            jp::LayoutCache cache;
            jp::PaintContext ctx;
            ctx.doc = doc.get();
            ctx.cache = &cache;
            ctx.opt.output = true;
            const QImage img = jp::Renderer::renderToImage(ctx, 0, 1.0);
            // The diagonal runs from the top left (1) or from the bottom left (2); the first half is on side 0.
            auto sideOf = [&](double x, double y) {
                const double u = (x - 72) / width, v = (y - 72) / height;
                return diagonal == 1 ? (v > u ? 0 : 1) : (u + v < 1 ? 0 : 1);
            };
            int reds = 0, blues = 0, wrongReds = 0, wrongBlues = 0;
            for (int y = 72; y < 72 + int(height); ++y)
                for (int x = 72; x < 72 + int(width); ++x) {
                    const QColor px = img.pixelColor(x, y);
                    const bool red = px.red() > 180 && px.green() < 90 && px.blue() < 90, blue = px.blue() > 180 && px.red() < 90 && px.green() < 90;
                    // A pixel a point or so over the line is the edge of a letter.
                    const double u = (x - 72) / width, v = (y - 72) / height;
                    const double edge = std::abs(diagonal == 1 ? v - u : u + v - 1) * std::min(width, height);
                    if (red) {
                        ++reds;
                        if (sideOf(x, y) != 0 && edge > 2) ++wrongReds;
                    }
                    if (blue) {
                        ++blues;
                        if (sideOf(x, y) != 1 && edge > 2) ++wrongBlues;
                    }
                }
            QVERIFY2(reds > 300 && blues > 300, qPrintable(QStringLiteral("diagonal %1: %2 and %3 pixels").arg(diagonal).arg(reds).arg(blues)));
            QVERIFY2(wrongReds == 0 && wrongBlues == 0, qPrintable(QStringLiteral("diagonal %1: %2 red and %3 blue pixels on the wrong side").arg(diagonal).arg(wrongReds).arg(wrongBlues)));
        }
    }

    // An older file's divided cell has one story, which stays in the first half; typing in the second half
    // makes its story.
    void tableDividedCellFromOlderFile()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto made = ed->newTable(QRectF(72, 72, 300, 100), 1, 1);
        ed->addItem(made);
        const QString id = made->id;
        QJsonObject json = static_cast<jp::TableItem *>(ed->doc()->item(id))->toJson();
        QJsonArray cells = json["cells"].toArray();
        QJsonObject co = cells[0].toObject();
        co["diag"] = 1;
        co.remove("diagLine");
        co.remove("story2");
        cells[0] = co;
        json["cells"] = cells;
        jp::TableItem old;
        old.fromJson(json);
        QCOMPARE(old.cell(0, 0).diagonal, 1);
        QVERIFY(old.cell(0, 0).storyId2.isEmpty());
        QVERIFY(old.cell(0, 0).divided());
        auto *t = static_cast<jp::TableItem *>(ed->doc()->item(id));
        t->cell(0, 0).diagonal = 1;
        t->cell(0, 0).diagonalLine = old.cell(0, 0).diagonalLine;
        ed->beginTextEdit(id, -1, 0, 0, 1);
        t = static_cast<jp::TableItem *>(ed->doc()->item(id));
        QCOMPARE(ed->textTarget().half, 1);
        QVERIFY(!t->cell(0, 0).storyId2.isEmpty());
        QCOMPARE(ed->textTarget().storyId, t->cell(0, 0).storyId2);
    }

    // Taking a division away, merging cells, or splitting them changes how much room the words have: the row
    // grows to hold them, in the same Undo step.
    void tableDivisionAndMergeRefitRows()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        const QString words = QStringLiteral("The quick brown fox jumps over the lazy dog. ").repeated(8);
        {
            auto made = ed->newTable(QRectF(72, 72, 300, 60), 1, 1);
            ed->addItem(made);
            const QString id = made->id;
            auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
            ed->selectCells(id, 0, 0, 0, 0);
            w.act(QStringLiteral("tbl.diagDown"))->trigger();
            ed->doc()->storyDoc(table()->cell(0, 0).storyId)->setPlainText(words);
            ed->doc()->storyDoc(table()->cell(0, 0).storyId2)->setPlainText(words);
            ed->fitTableRows(table());
            QCOMPARE(table()->rowH[0], 60.0);   // the halves hold what fits
            w.act(QStringLiteral("tbl.diagNone"))->trigger();
            const double joined = table()->rowH[0];
            QVERIFY2(joined > 100, qPrintable(QString::number(joined)));
            ed->fitTableRows(table());
            QCOMPARE(table()->rowH[0], joined);   // it already holds the text
            ed->undo();
            QCOMPARE(table()->cell(0, 0).diagonal, 1);
            QCOMPARE(table()->rowH[0], 60.0);
        }
        auto made = ed->newTable(QRectF(72, 300, 300, 60), 1, 2);
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        ed->doc()->storyDoc(table()->cell(0, 0).storyId)->setPlainText(words);
        ed->doc()->storyDoc(table()->cell(0, 1).storyId)->setPlainText(words);
        ed->selectCells(id, 0, 0, 0, 1);
        w.act(QStringLiteral("tbl.merge"))->trigger();
        const double merged = table()->rowH[0];
        QVERIFY2(merged > 100, qPrintable(QString::number(merged)));
        ed->fitTableRows(table());
        QCOMPARE(table()->rowH[0], merged);
        // Splitting them gives the words less room.
        w.act(QStringLiteral("tbl.split"))->trigger();
        const double split = table()->rowH[0];
        QVERIFY2(split > merged + 20, qPrintable(QString::number(split)));
        ed->fitTableRows(table());
        QCOMPARE(table()->rowH[0], split);
        ed->undo();
        QCOMPARE(table()->rowH[0], merged);
        ed->undo();
        QCOMPARE(table()->cell(0, 0).colSpan, 1);
        QCOMPARE(table()->rowH[0], 60.0);
    }

    // Taking a division away leaves no second story behind: Word Count and File > Info add up every story, and a
    // file saves them all. Undo and Redo bring it back and take it away again.
    void tableJoiningHalvesRemovesTheSecondStory()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto made = ed->newTable(QRectF(72, 72, 300, 100), 1, 2);
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        auto text = [&](const QString &story) { const QTextDocument *d = ed->doc()->storyDoc(story); return d ? d->toPlainText() : QStringLiteral("(none)"); };
        auto holding = [&](const QString &needle) {
            int n = 0;
            for (auto it = ed->doc()->stories.cbegin(); it != ed->doc()->stories.cend(); ++it)
                if ((*it)->doc->toPlainText().contains(needle)) ++n;
            return n;
        };
        auto stories = [&] { return int(ed->doc()->stories.size()); };
        ed->doc()->storyDoc(table()->cell(0, 0).storyId)->setPlainText(QStringLiteral("One"));
        ed->selectCells(id, 0, 0, 0, 0);
        w.act(QStringLiteral("tbl.diagDown"))->trigger();
        ed->doc()->storyDoc(table()->cell(0, 0).storyId2)->setPlainText(QStringLiteral("Two"));
        const int divided = stories();
        w.act(QStringLiteral("tbl.diagNone"))->trigger();
        QCOMPARE(text(table()->cell(0, 0).storyId), QStringLiteral("OneTwo"));
        QCOMPARE(stories(), divided - 1);
        QCOMPARE(holding(QStringLiteral("Two")), 1);
        QCOMPARE(int(ed->doc()->storiesInUse().size()), stories());
        QString err;
        auto back = jp::publicationFromBytes(jp::publicationBytes(*ed->doc(), QImage()), &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->stories.size()), stories());
        ed->undo();
        QCOMPARE(stories(), divided);
        QCOMPARE(text(table()->cell(0, 0).storyId), QStringLiteral("One"));
        QCOMPARE(text(table()->cell(0, 0).storyId2), QStringLiteral("Two"));
        ed->redo();
        QCOMPARE(stories(), divided - 1);
        QCOMPARE(text(table()->cell(0, 0).storyId), QStringLiteral("OneTwo"));
        QCOMPARE(holding(QStringLiteral("Two")), 1);
        // Merging joins the halves of the cells it takes in, in the same way.
        ed->doc()->storyDoc(table()->cell(0, 1).storyId)->setPlainText(QStringLiteral("B"));
        ed->selectCells(id, 0, 1, 0, 1);
        w.act(QStringLiteral("tbl.diagUp"))->trigger();
        ed->doc()->storyDoc(table()->cell(0, 1).storyId2)->setPlainText(QStringLiteral("Three"));
        const int before = stories();
        ed->selectCells(id, 0, 0, 0, 1);
        w.act(QStringLiteral("tbl.merge"))->trigger();
        QCOMPARE(table()->cell(0, 0).colSpan, 2);
        QCOMPARE(stories(), before - 1);
        QCOMPARE(holding(QStringLiteral("Three")), 1);
        QCOMPARE(text(table()->cell(0, 0).storyId), QStringLiteral("OneTwo\nBThree"));
        QCOMPARE(int(ed->doc()->storiesInUse().size()), stories());
        ed->undo();
        QCOMPARE(stories(), before);
        QCOMPARE(text(table()->cell(0, 1).storyId2), QStringLiteral("Three"));
    }

    // Row and column commands, Merge Cells, Split Cells, and No Division drop the stories nothing uses any more, as
    // deleting an object does, and keep the ones the table still has.
    void tableStructureEditsLeaveNoUnusedStories()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto made = ed->newTable(QRectF(72, 72, 300, 150), 3, 3);
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        auto unused = [&] { return int(ed->doc()->stories.size()) - int(ed->doc()->storiesInUse().size()); };
        auto allThere = [&] {
            for (const auto &c : table()->cells) {
                if (!ed->doc()->storyDoc(c.storyId)) return false;
                if (c.divided() && !c.storyId2.isEmpty() && !ed->doc()->storyDoc(c.storyId2)) return false;
            }
            return true;
        };
        ed->selectCells(id, 0, 0, 2, 2);
        w.act(QStringLiteral("tbl.diagDown"))->trigger();
        QCOMPARE(unused(), 0);
        const int full = int(ed->doc()->stories.size());
        ed->selectCells(id, 1, 0, 1, 0);
        w.act(QStringLiteral("tbl.delRow"))->trigger();
        QCOMPARE(table()->rows, 2);
        QCOMPARE(unused(), 0);
        QCOMPARE(int(ed->doc()->stories.size()), full - 6);
        QVERIFY(allThere());
        ed->selectCells(id, 0, 0, 0, 0);
        w.act(QStringLiteral("tbl.delCol"))->trigger();
        QCOMPARE(table()->cols, 2);
        QCOMPARE(unused(), 0);
        QVERIFY(allThere());
        ed->undo();
        QCOMPARE(table()->cols, 3);
        QVERIFY(allThere());
        ed->redo();
        // New cells have their stories, and the table keeps them when something else is done to it.
        ed->selectCells(id, 0, 0, 0, 0);
        w.act(QStringLiteral("tbl.insBelow"))->trigger();
        QCOMPARE(table()->rows, 3);
        QCOMPARE(unused(), 0);
        QVERIFY(allThere());
        ed->selectCells(id, 1, 0, 1, 1);
        w.act(QStringLiteral("tbl.merge"))->trigger();
        QCOMPARE(unused(), 0);
        QVERIFY(allThere());
        w.act(QStringLiteral("tbl.split"))->trigger();
        QCOMPARE(unused(), 0);
        QVERIFY(allThere());
        ed->selectCells(id, 0, 0, 2, 1);
        w.act(QStringLiteral("tbl.diagNone"))->trigger();
        QCOMPARE(unused(), 0);
        QVERIFY(allThere());
    }

    // Visiting the second half of a divided cell that has no story yet (an older file's) makes one without an
    // Undo step: Tab through such cells does not make the publication ask to be saved or lose Redo.
    void tableVisitingAnOlderDividedCellsSecondHalfIsNoEdit()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto made = ed->newTable(QRectF(72, 72, 400, 100), 1, 3);
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        auto text = [&](const QString &story) { const QTextDocument *d = ed->doc()->storyDoc(story); return d ? d->toPlainText() : QStringLiteral("(none)"); };
        for (auto &c : table()->cells) {
            c.diagonal = 1;
            c.diagonalLine = jp::Stroke::line(jp::ColorRef::rgb(Qt::black), 0.75);
        }
        ed->change(QStringLiteral("Test"), [&] { ed->surfaceItems().push_back(ed->newTextBox(QRectF(72, 300, 100, 40))); });
        ed->undo();
        QVERIFY(ed->undoStack()->canRedo());
        ed->undoStack()->setClean();
        const int steps = ed->undoStack()->count(), index = ed->undoStack()->index();
        for (int c = 0; c < 3; ++c) {
            ed->beginTextEdit(id, -1, 0, c, 1);
            QVERIFY(!table()->cell(0, c).storyId2.isEmpty());
            QCOMPARE(ed->textTarget().storyId, table()->cell(0, c).storyId2);
        }
        QCOMPARE(ed->undoStack()->count(), steps);
        QVERIFY(ed->undoStack()->isClean());
        QVERIFY(ed->undoStack()->canRedo());
        // Typing there is one step, and Undo and Redo go through it with the story in place.
        ed->beginTextEdit(id, -1, 0, 1, 1);
        ed->insertText(QStringLiteral("Two"));
        ed->flushTyping();
        QCOMPARE(text(table()->cell(0, 1).storyId2), QStringLiteral("Two"));
        QCOMPARE(ed->undoStack()->index(), index + 1);
        ed->undo();
        QVERIFY(!table()->cell(0, 1).storyId2.isEmpty());
        QCOMPARE(text(table()->cell(0, 1).storyId2), QString());
        ed->redo();
        QCOMPARE(text(table()->cell(0, 1).storyId2), QStringLiteral("Two"));
    }

    // A cell already divided keeps its line when the direction changes: a line set to red and thick stays so, a
    // division with no line (a .pub file's can be) is not given one, and a cell not yet divided gets the default.
    void tableChangingTheDivisionKeepsItsLine()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto made = ed->newTable(QRectF(72, 72, 300, 100), 1, 3);
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        table()->cell(0, 0).diagonal = 1;
        table()->cell(0, 0).diagonalLine = jp::Stroke::line(jp::ColorRef::rgb(Qt::red), 3);
        table()->cell(0, 0).storyId2 = ed->doc()->createStory();
        table()->cell(0, 1).diagonal = 1;
        table()->cell(0, 1).diagonalLine = jp::Stroke::none();
        table()->cell(0, 1).storyId2 = ed->doc()->createStory();
        ed->selectCells(id, 0, 0, 0, 2);
        w.act(QStringLiteral("tbl.diagUp"))->trigger();
        for (int c = 0; c < 3; ++c) QCOMPARE(table()->cell(0, c).diagonal, 2);
        QCOMPARE(table()->cell(0, 0).diagonalLine.width, 3.0);
        QCOMPARE(table()->cell(0, 0).diagonalLine.color.toString(), jp::ColorRef::rgb(Qt::red).toString());
        QVERIFY(table()->cell(0, 1).diagonalLine.isNone());
        QVERIFY(!table()->cell(0, 2).diagonalLine.isNone());
        QCOMPARE(table()->cell(0, 2).diagonalLine.width, 0.75);
        // Taking the division away and dividing again starts over.
        w.act(QStringLiteral("tbl.diagNone"))->trigger();
        w.act(QStringLiteral("tbl.diagDown"))->trigger();
        QCOMPARE(table()->cell(0, 0).diagonalLine.width, 0.75);
        QVERIFY(!table()->cell(0, 1).diagonalLine.isNone());
    }

    // With text selected, an arrow key that would go on to the other half of a divided cell collapses the
    // selection first, as it does anywhere else; the next press goes on.
    void tableDividedCellArrowKeysCollapseASelectionFirst()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        jp::Editor *ed = w.editor();
        auto made = ed->newTable(QRectF(72, 72, 400, 150), 1, 1);
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        ed->doc()->storyDoc(table()->cell(0, 0).storyId)->setPlainText(QStringLiteral("First half"));
        ed->selectCells(id, 0, 0, 0, 0);
        w.act(QStringLiteral("tbl.diagDown"))->trigger();
        ed->doc()->storyDoc(table()->cell(0, 0).storyId2)->setPlainText(QStringLiteral("Second"));
        ed->endTextEdit();
        auto *cv = w.findChild<jp::Canvas *>();
        // All of the half selected, from the start to the end (forward) or the end to the start (backward).
        auto selectAll = [&](int half, bool forward) {
            ed->beginTextEdit(id, -1, 0, 0, half);
            const int last = ed->editDoc()->characterCount() - 1;
            QTextCursor c = ed->cursor();
            c.setPosition(forward ? 0 : last);
            c.setPosition(forward ? last : 0, QTextCursor::KeepAnchor);
            ed->setCursor(c);
            return ed->cursor().hasSelection();
        };
        QVERIFY(selectAll(0, true));
        QTest::keyClick(cv, Qt::Key_Right);
        QCOMPARE(ed->textTarget().half, 0);
        QVERIFY(!ed->cursor().hasSelection());
        QCOMPARE(ed->cursor().position(), ed->editDoc()->characterCount() - 1);
        QTest::keyClick(cv, Qt::Key_Right);
        QCOMPARE(ed->textTarget().half, 1);
        QVERIFY(selectAll(1, false));
        QTest::keyClick(cv, Qt::Key_Left);
        QCOMPARE(ed->textTarget().half, 1);
        QVERIFY(!ed->cursor().hasSelection());
        QCOMPARE(ed->cursor().position(), 0);
        QTest::keyClick(cv, Qt::Key_Left);
        QCOMPARE(ed->textTarget().half, 0);
        QVERIFY(selectAll(0, true));
        QTest::keyClick(cv, Qt::Key_Down);
        QCOMPARE(ed->textTarget().half, 0);
        QVERIFY(!ed->cursor().hasSelection());
        QTest::keyClick(cv, Qt::Key_Down);
        QCOMPARE(ed->textTarget().half, 1);
        QVERIFY(selectAll(1, false));
        QTest::keyClick(cv, Qt::Key_Up);
        QCOMPARE(ed->textTarget().half, 1);
        QVERIFY(!ed->cursor().hasSelection());
        QTest::keyClick(cv, Qt::Key_Up);
        QCOMPARE(ed->textTarget().half, 0);
        QTest::mouseClick(cv->viewport(), Qt::LeftButton, {}, QPoint(5, 5));   // the mouse in use again: no focus ring in the tests after
    }

    // Right-clicking in another cell while typing in a table leaves the caret where it is (it once tried to put it
    // at that cell's position in this cell's text); in the cell being typed in, it puts the caret at the click.
    void tableRightClickInAnotherCellWhileEditing()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        jp::Editor *ed = w.editor();
        auto made = ed->newTable(QRectF(72, 72, 400, 100), 1, 2);
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        ed->doc()->storyDoc(table()->cell(0, 0).storyId)->setPlainText(QStringLiteral("aa"));
        ed->doc()->storyDoc(table()->cell(0, 1).storyId)->setPlainText(QStringLiteral("a very long second cell text that goes on and on"));
        ed->beginTextEdit(id, 0, 0, 0, 0);
        auto *cv = w.findChild<jp::Canvas *>();
        auto rightClick = [&](int col, double dx) {
            const QRectF cell = table()->cellRect(0, col).translated(table()->rect.topLeft());
            QTest::mouseClick(cv->viewport(), Qt::RightButton, {}, cv->pageToView(QPointF(cell.left() + dx, cell.top() + 10)).toPoint());
        };
        QTest::failOnWarning(QRegularExpression(QStringLiteral("out of range")));
        rightClick(1, 60);
        QCOMPARE(ed->textTarget().col, 0);
        QCOMPARE(ed->cursor().position(), 0);
        rightClick(0, 12);   // past the two letters
        QCOMPARE(ed->textTarget().col, 0);
        QCOMPARE(ed->cursor().position(), 2);
        QTest::mouseClick(cv->viewport(), Qt::LeftButton, {}, QPoint(5, 5));
    }

    // Extra Content names a table by the first words in it, and a divided cell's second half counts.
    void extraContentTableLabelUsesSecondHalfText()
    {
        jp::MainWindow w;
        w.show();
        jp::Editor *ed = w.editor();
        auto made = ed->newTable(QRectF(72, 72, 300, 100), 1, 2);
        auto *t = static_cast<jp::TableItem *>(made.get());
        t->cell(0, 1).diagonal = 1;
        t->cell(0, 1).storyId2 = ed->doc()->createStory(QStringLiteral("Only in the second half"));
        ed->doc()->extra.push_back(made);
        w.showTaskPane(QStringLiteral("extra"));
        auto *list = w.findChild<jp::TaskPane *>()->findChild<QListWidget *>(QStringLiteral("extraContentList"));
        QVERIFY(list);
        QCOMPARE(list->count(), 1);
        QVERIFY2(list->item(0)->text().contains(QStringLiteral("Only in the second half")), qPrintable(list->item(0)->text()));
    }

    // The line along a cell's diagonal is saved in JeffPub files (none stays
    // none), the Diagonals commands draw one, and an older file's diagonal
    // keeps the line it was drawn with.
    void tableDiagonalLine()
    {
        jp::TableItem t;
        t.rows = 1;
        t.cols = 3;
        t.colW = {50, 50, 50};
        t.rowH = {40};
        t.cells.resize(3);
        t.cell(0, 0).diagonal = 1;
        t.cell(0, 0).diagonalLine = jp::Stroke::line(jp::ColorRef::rgb(Qt::red), 2);
        t.cell(0, 1).diagonal = 2;
        QJsonObject json = t.toJson();
        jp::TableItem u;
        u.fromJson(json);
        QCOMPARE(u.cell(0, 0).diagonalLine.width, 2.0);
        QCOMPARE(u.cell(0, 0).diagonalLine.color, jp::ColorRef::rgb(Qt::red));
        QVERIFY(u.cell(0, 1).diagonalLine.isNone());
        QCOMPARE(u.cell(0, 1).diagonal, 2);
        // Older files have no line: the top border, or a thin line.
        QJsonArray cells = json["cells"].toArray();
        for (int i = 0; i < cells.size(); ++i) {
            QJsonObject co = cells[i].toObject();
            co.remove("diagLine");
            cells[i] = co;
        }
        json["cells"] = cells;
        jp::TableItem v;
        v.fromJson(json);
        QVERIFY(!v.cell(0, 0).diagonalLine.isNone() && !v.cell(0, 1).diagonalLine.isNone());
        QVERIFY(v.cell(0, 2).diagonalLine.isNone());

        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto made = ed->newTable(QRectF(72, 72, 200, 100), 2, 2);
        ed->addItem(made);
        const QString id = made->id;
        auto table = [&] { return static_cast<jp::TableItem *>(ed->doc()->item(id)); };
        ed->selectCells(id, 0, 0, 0, 0);
        w.act(QStringLiteral("tbl.diagDown"))->trigger();
        QCOMPARE(table()->cell(0, 0).diagonal, 1);
        QVERIFY(!table()->cell(0, 0).diagonalLine.isNone());
        QVERIFY(table()->cell(1, 1).diagonalLine.isNone());
        w.act(QStringLiteral("tbl.diagNone"))->trigger();
        QCOMPARE(table()->cell(0, 0).diagonal, 0);
        QVERIFY(table()->cell(0, 0).diagonalLine.isNone());
    }

    // Table rules keep their width when read on a system that writes
    // decimals with a comma.
    void tableBordersAnyLocale()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TableItem>();
        t->rows = 2;
        t->cols = 2;
        t->colW = {100, 150};
        t->rowH = {30, 40};
        t->rect = QRectF(72, 144, 0, 0);
        t->syncRect();
        t->cells.resize(4);
        for (int i = 0; i < 4; ++i) t->cells[i].storyId = doc->createStory(QStringLiteral("cell"));
        for (int c = 0; c < 2; ++c) t->cell(0, c).border.bottom = jp::Stroke::line(jp::ColorRef::rgb(Qt::black), 1.5);
        doc->pages[0]->items.push_back(t);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("rules.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        const std::string before = setlocale(LC_NUMERIC, nullptr);
        const char *comma = nullptr;
        for (const char *l : {"de_DE.UTF-8", "de_DE.utf8", "de_AT.utf8", "fr_FR.UTF-8", "fr_FR.utf8"})
            if (setlocale(LC_NUMERIC, l)) { comma = l; break; }
        if (!comma) QSKIP("no comma-decimal locale on this system");
        auto back = jp::importPublisherFile(path, &err);
        setlocale(LC_NUMERIC, before.c_str());
        QVERIFY2(back, qPrintable(err));
        double widest = 0;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() != jp::ItemType::Table) return;
            for (const jp::TableCell &c : static_cast<const jp::TableItem *>(it.get())->cells)
                for (const jp::Stroke *st : {&c.border.left, &c.border.right, &c.border.top, &c.border.bottom})
                    if (!st->isNone()) widest = std::max(widest, st->width);
        });
        QVERIFY2(std::abs(widest - 1.5) < 0.05, qPrintable(QString::number(widest)));
    }

    // A table cell's Text Direction and hyphenation are saved with the table;
    // a cell that keeps the defaults saves nothing more than it did.
    void tableCellDirectionAndHyphenationSave()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TableItem>();
        t->rows = 1;
        t->cols = 3;
        t->colW = {60, 60, 60};
        t->rowH = {120};
        t->rect = QRectF(72, 72, 0, 0);
        t->syncRect();
        t->cells.resize(3);
        for (auto &c : t->cells) c.storyId = doc->createStory(QStringLiteral("cell"));
        t->cell(0, 1).vertical = true;
        t->cell(0, 2).hyphenate = false;
        t->cell(0, 2).hyphenZone = 30;
        doc->pages[0]->items.push_back(t);
        QString err;
        auto again = jp::publicationFromBytes(jp::publicationBytes(*doc, QImage()), &err);
        QVERIFY2(again, qPrintable(err));
        const auto *back = static_cast<const jp::TableItem *>(again->pages[0]->items[0].get());
        QCOMPARE(int(back->cells.size()), 3);
        QVERIFY(!back->cell(0, 0).vertical && back->cell(0, 0).hyphenate && back->cell(0, 0).hyphenZone == 18.0);
        QVERIFY(back->cell(0, 1).vertical && back->cell(0, 1).hyphenate);
        QVERIFY(!back->cell(0, 2).vertical && !back->cell(0, 2).hyphenate && back->cell(0, 2).hyphenZone == 30.0);
        const QJsonObject plain = t->toJson()["cells"].toArray()[0].toObject();
        for (const char *key : {"vertical", "hyph", "hyphZone"}) QVERIFY2(!plain.contains(QLatin1String(key)), key);
    }

    // A cell's Text Direction turns its text 90 degrees as a text box's does:
    // the lines run down a tall, narrow cell, and it is drawn just as the
    // vertical text box is.
    void tableCellTextDirectionLaysOutAndDraws()
    {
        const QString text = QStringLiteral("Region name and notes");
        auto tableDoc = [&](bool vertical) {
            auto doc = jp::Document::blank(QSizeF(612, 792));
            auto t = std::make_shared<jp::TableItem>();
            t->rows = 1;
            t->cols = 1;
            t->colW = {40};
            t->rowH = {200};
            t->rect = QRectF(72, 72, 0, 0);
            t->syncRect();
            t->stroke = jp::Stroke::none();
            t->cells.resize(1);
            t->cells[0].storyId = doc->createStory(text);
            t->cells[0].vertical = vertical;
            doc->pages[0]->items.push_back(t);
            return doc;
        };
        auto lines = [](jp::Document &doc) {
            jp::LayoutCache cache;
            jp::PaintContext ctx;
            ctx.doc = &doc;
            ctx.cache = &cache;
            QPointF origin;
            const auto *lay = jp::Renderer::cellLayout(ctx, *static_cast<jp::TableItem *>(doc.pages[0]->items[0].get()), 0, 0, &origin);
            return lay ? int(lay->lineInfo(0).size()) : -1;
        };
        auto across = tableDoc(false), down = tableDoc(true);
        QVERIFY2(lines(*across) > 1, "a 40-point cell wraps the text");
        QCOMPARE(lines(*down), 1);   // the same cell turned: 200 points to run along
        auto box = jp::Document::blank(QSizeF(612, 792));
        auto spine = std::make_shared<jp::TextItem>();
        spine->rect = QRectF(72, 72, 40, 200);
        spine->vertical = true;
        spine->stroke = jp::Stroke::none();
        spine->storyId = box->createStory(text);
        box->pages[0]->items.push_back(spine);
        const QImage cell = jp::renderPlate(*down, 0, 3, 72), asBox = jp::renderPlate(*box, 0, 3, 72);
        QRect ink;
        for (int y = 0; y < cell.height(); ++y)
            for (int x = 0; x < cell.width(); ++x)
                if (qGray(cell.pixel(x, y)) < 128) ink |= QRect(x, y, 1, 1);
        QVERIFY2(QRect(72, 72, 40, 200).contains(ink) && ink.height() > 3 * ink.width(), qPrintable(QStringLiteral("%1,%2 %3x%4").arg(ink.x()).arg(ink.y()).arg(ink.width()).arg(ink.height())));
        QVERIFY2(cell == asBox, "a turned cell draws as a turned text box");
    }

    // A cell hyphenates, or not, as its setting says: in the layout (a word
    // breaks at the end of the first line, with its hyphen) and in the
    // drawing; the zone is the cell's too.
    void tableCellHyphenationLaysOutAndDraws()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TableItem>();
        t->rows = 1;
        t->cols = 1;
        t->colW = {110};
        t->rowH = {100};
        t->rect = QRectF(72, 72, 0, 0);
        t->syncRect();
        t->stroke = jp::Stroke::none();
        t->cells.resize(1);
        t->cells[0].storyId = doc->createStory(QStringLiteral("We met at the international conference today."));
        doc->pages[0]->items.push_back(t);
        auto firstLine = [&](bool hyphenate, double zone) {
            t->cells[0].hyphenate = hyphenate;
            t->cells[0].hyphenZone = zone;
            jp::LayoutCache cache;
            jp::PaintContext ctx;
            ctx.doc = doc.get();
            ctx.cache = &cache;
            QPointF origin;
            const auto *lay = jp::Renderer::cellLayout(ctx, *t, 0, 0, &origin);
            return lay ? lay->lineInfo(0).value(0).text : QString();
        };
        QString on = firstLine(true, 0), off = firstLine(false, 0), far = firstLine(true, 200);
        QVERIFY2(on.endsWith(QChar(0x00AD)), qPrintable(on));   // the line ends in a hyphen
        for (QString *line : {&on, &off, &far}) line->remove(QChar(0x00AD));
        QVERIFY2(on.contains(QStringLiteral("inter")), qPrintable(on));
        QVERIFY2(!off.contains(QStringLiteral("inter")), qPrintable(off));
        QVERIFY2(!far.contains(QStringLiteral("inter")), qPrintable(far));   // moving the word whole leaves less than the zone
        // Drawn: the first line holds the word's start and its hyphen.
        auto topInk = [&](bool hyphenate) {
            t->cells[0].hyphenate = hyphenate;
            t->cells[0].hyphenZone = 0;
            const QImage k = jp::renderPlate(*doc, 0, 3, 72);
            int n = 0;
            for (int y = 72; y < 90; ++y)
                for (int x = 72; x < 182; ++x) n += qGray(k.pixel(x, y)) < 128;
            return n;
        };
        const int hyphenated = topInk(true), whole = topInk(false);
        QVERIFY2(hyphenated > whole, qPrintable(QStringLiteral("%1 vs %2").arg(hyphenated).arg(whole)));
    }

    // Typing in a cell turned on its side: the caret lies across the lines,
    // and a click at a letter puts the caret there.
    void tableCellEditsOnItsSide()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        jp::Editor *ed = w.editor();
        auto t = std::make_shared<jp::TableItem>();
        t->rows = 1;
        t->cols = 2;
        t->colW = {40, 120};
        t->rowH = {200};
        t->rect = QRectF(72, 72, 0, 0);
        t->syncRect();
        t->stroke = jp::Stroke::none();
        t->cells.resize(2);
        for (auto &c : t->cells) c.storyId = ed->doc()->createStory(QStringLiteral("Region name"));
        t->cell(0, 0).vertical = true;
        ed->addItem(t);
        jp::Canvas *cv = w.canvas();
        QTest::qWait(50);
        ed->beginTextEdit(t->id, 3, 0, 0);
        QRectF caret = cv->caretViewRect();
        QVERIFY2(caret.width() > caret.height(), "the caret in turned text is a level stroke");
        ed->beginTextEdit(t->id, 3, 0, 1);
        caret = cv->caretViewRect();
        QVERIFY2(caret.height() > caret.width(), "and an upright one in plain text");
        // Each place between letters is found again from where it is drawn:
        // the text reads down the cell, from its top.
        jp::PaintContext ctx;
        ctx.doc = ed->doc();
        ctx.cache = &ed->cache();
        QPointF origin;
        const auto *lay = jp::Renderer::cellLayout(ctx, *t, 0, 0, &origin);
        QVERIFY(lay);
        double lastY = -1;
        for (int pos : {0, 4, 7, 11}) {
            int frame;
            QRectF r;
            QVERIFY(lay->caretRect(pos, &frame, &r));
            const QPointF page = t->rect.topLeft() + QPointF(t->cellRect(0, 0).width() - r.center().y(), r.x());
            QCOMPARE(cv->textPosAt(t->id, page, 0, 0), pos);
            QVERIFY(page.y() > lastY);
            lastY = page.y();
        }
    }

    // Table Layout > Alignment has Text Direction, for the cell the cursor is
    // in or, with the table selected, every cell; new rows follow their
    // neighbors.
    void tableTextDirectionButton()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto made = ed->newTable(QRectF(72, 72, 288, 72), 2, 2);
        ed->addItem(made);
        const QString id = made->id;
        auto turned = [&] {
            QString s;
            for (const auto &c : static_cast<jp::TableItem *>(ed->doc()->item(id))->cells) s += c.vertical ? 'v' : '-';
            return s;
        };
        auto direction = [&] { return w.act(QStringLiteral("tb.direction")); };
        ed->beginTextEdit(id, 0, 1, 0);
        direction()->trigger();
        QCOMPARE(turned(), QStringLiteral("--v-"));   // row by row: the cell the cursor is in
        QTRY_VERIFY(direction()->isChecked());   // the ribbon catches up on its next refresh
        ed->beginTextEdit(id, 0, 0, 1);
        QTRY_VERIFY2(!direction()->isChecked(), "the button follows the cell");
        direction()->trigger();
        QCOMPARE(turned(), QStringLiteral("-vv-"));
        direction()->trigger();
        QCOMPARE(turned(), QStringLiteral("--v-"));
        // A row added next to a turned cell is turned too.
        ed->beginTextEdit(id, 0, 1, 0);
        w.act(QStringLiteral("tbl.insBelow"))->trigger();
        QCOMPARE(turned(), QStringLiteral("--v-v-"));
        ed->undo();
        QCOMPARE(turned(), QStringLiteral("--v-"));
        // The whole table: every cell turns, and turns back when all are turned.
        ed->endTextEdit();
        ed->select(id);
        direction()->trigger();
        QCOMPARE(turned(), QStringLiteral("vvvv"));
        QTRY_VERIFY(direction()->isChecked());   // the ribbon catches up on its next refresh
        direction()->trigger();
        QCOMPARE(turned(), QStringLiteral("----"));
        ed->undo();
        QCOMPARE(turned(), QStringLiteral("vvvv"));
        // Its place on the ribbon: Table Layout > Alignment, with KeyTips.
        auto *r = w.findChild<jp::Ribbon *>();
        QVERIFY(r);
        const QString d = r->describe(true);
        const QString layout = d.mid(d.indexOf(QStringLiteral("\ntab Table Layout")) + 1).section(QStringLiteral("\ntab "), 0, 0);
        QVERIFY(!layout.isEmpty());
        bool direct = false, hyphen = false;
        for (const QString &line : layout.split(QLatin1Char('\n'))) {
            direct = direct || (line.contains(QStringLiteral("tb.direction")) && line.endsWith(QStringLiteral(" keytip=TD")));
            hyphen = hyphen || (line.contains(QStringLiteral("ribbon.hyphenation")) && line.endsWith(QStringLiteral(" keytip=HY")));
        }
        QVERIFY2(direct && hyphen, qPrintable(layout));
    }

    // The Hyphenation command works in a table. A table's text is one story, so
    // it sets every cell, whether the cursor is in a cell, some cells are
    // selected, or the table is.
    void tableHyphenationCommand()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto made = ed->newTable(QRectF(72, 72, 288, 72), 2, 2);
        ed->addItem(made);
        const QString id = made->id;
        auto *t = static_cast<jp::TableItem *>(ed->doc()->item(id));
        for (auto &c : t->cells) ed->doc()->storyDoc(c.storyId)->setPlainText(QStringLiteral("international conference"));
        auto settings = [&] {
            QString s;
            for (const auto &c : static_cast<jp::TableItem *>(ed->doc()->item(id))->cells) s += c.hyphenate ? 'h' : '-';
            return s;
        };
        auto hyphenated = [&] {
            QString s;
            for (const auto &c : static_cast<jp::TableItem *>(ed->doc()->item(id))->cells)
                s += ed->doc()->storyDoc(c.storyId)->toPlainText().contains(QChar(0x00AD)) ? 'o' : '-';
            return s;
        };
        // The dialog: automatic hyphenation on or off, and optional hyphens added.
        auto command = [&](bool automatic, bool addHyphens) {
            QTimer::singleShot(0, &w, [&, automatic, addHyphens] {
                auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!dlg) return;
                auto *check = dlg->findChild<QCheckBox *>();
                if (!check) {
                    dlg->reject();   // the "click in a text box" notice
                    return;
                }
                check->setChecked(automatic);
                if (addHyphens)
                    for (auto *b : dlg->findChildren<QPushButton *>())
                        if (b->text() == QLatin1String("Add Optional Hyphens")) b->click();
                dlg->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
            });
            w.act(QStringLiteral("rev.hyphenation"))->trigger();
        };
        QCOMPARE(settings(), QStringLiteral("hhhh"));
        ed->beginTextEdit(id, 0, 1, 1);
        command(false, true);
        QCOMPARE(settings(), QStringLiteral("----"));   // every cell, not only the cell the cursor is in
        QCOMPARE(hyphenated(), QStringLiteral("---o"));   // optional hyphens go in that cell's text
        ed->undo();
        ed->undo();
        QCOMPARE(settings(), QStringLiteral("hhhh"));
        ed->endTextEdit();
        ed->selectCells(id, 0, 0, 0, 1);
        command(false, false);
        QCOMPARE(settings(), QStringLiteral("----"));   // selected cells: still the whole table
        command(true, false);
        QCOMPARE(settings(), QStringLiteral("hhhh"));
        ed->select(id);
        command(false, true);
        QCOMPARE(settings(), QStringLiteral("----"));
        QCOMPARE(hyphenated(), QStringLiteral("oooo"));
        command(true, false);
        QCOMPARE(settings(), QStringLiteral("hhhh"));
    }
};

JP_TEST_CLASS(PubTableTests)
#include "tst_pub_tables.moc"
