// JeffWrite's border painter, the table's move handle, and Show Notes (apps/write/app/writewindow_tables.cpp, pageview_input.cpp, writewindow_fields.cpp;
// the geometry of the painter is libs/flow/flow/borderpaint.h).
#include "tst_write.h"
#include "flow/borderpaint.h"
#include "app/theme.h"

#include <QAction>
#include <QMenu>
#include <QTimer>
#include <QTemporaryDir>

namespace {

QByteArray cellXml(const char *text)
{
    return QByteArray("<w:tc><w:p><w:r><w:t>") + text + "</w:t></w:r></w:p></w:tc>";
}

// "Before", a table of 2 by 2 cells (a b / c d, 100 points each), and two paragraphs after it.
QByteArray tableBody()
{
    return "<w:p><w:r><w:t>Before</w:t></w:r></w:p><w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid>"
           "<w:tr>" + cellXml("a") + cellXml("b") + "</w:tr><w:tr>" + cellXml("c") + cellXml("d") + "</w:tr></w:tbl>"
           "<w:p><w:r><w:t>After one</w:t></w:r></w:p><w:p><w:r><w:t>After two</w:t></w:r></w:p>";
}

int positionOf(WriteDoc &doc, const QString &text)
{
    return doc.story()->toPlainText().indexOf(text);
}

// The borders the cell with this text has, as written (empty: none).
QByteArray bordersOf(QByteArray docxXml, const char *text)
{
    docxXml.replace(" xml:space=\"preserve\"", "");
    const int at = docxXml.indexOf(QByteArray("<w:t>") + text + "</w:t>");
    if (at < 0) return "(no such cell)";
    const int start = docxXml.lastIndexOf("<w:tc>", at);
    const QByteArray head = docxXml.mid(start, at - start);
    const int b = head.indexOf("<w:tcBorders>");
    return b < 0 ? QByteArray() : head.mid(b, head.indexOf("</w:tcBorders>", b) - b);
}

// The order the paragraph and cell words come in the document's file.
QStringList wordsInOrder(QByteArray docxXml)
{
    docxXml.replace(" xml:space=\"preserve\"", "");
    QStringList out;
    int at = 0;
    while ((at = docxXml.indexOf("<w:t>", at)) >= 0) {
        const int end = docxXml.indexOf("</w:t>", at);
        out << QString::fromUtf8(docxXml.mid(at + 5, end - at - 5));
        at = end;
    }
    return out;
}

struct TailWin {
    WriteWindow w;
    explicit TailWin(const QByteArray &body = tableBody())
    {
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(body)), QString());
        QTextCursor c(w.document()->story());
        c.setPosition(positionOf(*w.document(), QStringLiteral("a")) );
        w.view()->setCursor(c);
        w.refreshForTest();
    }
    QByteArray xml() { return documentXml(w.document()->toDocx()); }
    // The cells' boxes in the view (waits for the page to be laid out).
    QHash<int, QRect> cells()
    {
        QVector<QPair<int, QRect>> found;
        for (int i = 0; i < 200 && found.isEmpty(); ++i) {
            w.view()->cellBoxesInView(&found);
            if (found.isEmpty()) QCoreApplication::processEvents();
        }
        QHash<int, QRect> out;
        for (const auto &c : std::as_const(found)) out.insert(c.first, c.second);
        return out;
    }
};

void drag(QWidget *vp, const QPoint &from, const QPoint &to, int steps = 8)
{
    QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, from);
    for (int i = 1; i <= steps; ++i) QTest::mouseMove(vp, from + (to - from) * i / steps);
    QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, to);
}

} // namespace

void WriteTests::thePathOfABorderPainterPaintsTheSidesItRunsAlong()
{
    using namespace jp::flow;
    std::vector<PaintCell> cells = {{QRectF(0, 0, 100, 50), 1}, {QRectF(100, 0, 100, 50), 2}, {QRectF(0, 50, 100, 50), 3}, {QRectF(100, 50, 100, 50), 4}};
    auto has = [](const std::vector<PaintedSide> &v, int key, CellSide side) {
        return std::any_of(v.begin(), v.end(), [&](const PaintedSide &s) { return s.key == key && s.side == side; });
    };
    // Down the line between the two upper cells, over most of it: the side of each that touches it.
    auto down = sidesUnderPath(cells, {{100, 4}, {100, 25}, {100, 46}}, 3);
    QCOMPARE(int(down.size()), 2);
    QVERIFY(has(down, 1, CellSide::Right));
    QVERIFY(has(down, 2, CellSide::Left));
    // Over less than half of it, nothing.
    QCOMPARE(int(sidesUnderPath(cells, {{100, 4}, {100, 20}}, 3).size()), 0);
    // Across it, a stroke that crosses the line paints nothing.
    QCOMPARE(int(sidesUnderPath(cells, {{80, 25}, {120, 25}}, 3).size()), 0);
    // A stroke through the middle of a cell, along neither of its lines, paints nothing.
    QCOMPARE(int(sidesUnderPath(cells, {{20, 25}, {80, 25}}, 3).size()), 0);
    // The outside edge of the table, and the line that is between two rows, over the width of both columns: the sides above and below it.
    auto across = sidesUnderPath(cells, {{2, 50}, {100, 50}, {198, 50}}, 3);
    QCOMPARE(int(across.size()), 4);
    QVERIFY(has(across, 1, CellSide::Bottom));
    QVERIFY(has(across, 3, CellSide::Top));
    QVERIFY(has(across, 2, CellSide::Bottom));
    QVERIFY(has(across, 4, CellSide::Top));
    auto outside = sidesUnderPath(cells, {{2, 0}, {198, 0}}, 3);
    QCOMPARE(int(outside.size()), 2);
    QVERIFY(has(outside, 1, CellSide::Top));
    QVERIFY(has(outside, 2, CellSide::Top));
}

void WriteTests::aBorderPainterPathIsCutAtTheMostPiecesItLooksAt()
{
    using namespace jp::flow;
    // A path of many points, one point apart, along a line; the only cell is far along it, past where the path is cut.
    std::vector<PaintCell> cells = {{QRectF(5000, 0, 100, 50), 1}};
    std::vector<QPointF> path;
    for (int x = 0; x <= 5200; ++x) path.push_back({double(x), 0});
    qint64 work = 0;
    QCOMPARE(int(sidesUnderPath(cells, path, 3, &work).size()), 0);
    QVERIFY(work <= qint64(kMostPaintPieces) * 4);   // (the pieces looked at, for each side of the cell)
    // The same path, short enough, does paint it.
    path.assign(5120 - 4096 > 0 ? 0 : 0, QPointF());
    for (int x = 4990; x <= 5200; ++x) path.push_back({double(x), 0});
    QCOMPARE(int(sidesUnderPath(cells, path, 3).size()), 1);
}

void WriteTests::aScribbleOverAHugeTableIsBoundedByACountedNumberOfLooks()
{
    using namespace jp::flow;
    // 200 columns by 252 rows of cells ten points square: 50,400 cells, and a path of 4,096 points that crosses the whole table again and again.
    std::vector<PaintCell> cells;
    for (int r = 0; r < 252; ++r)
        for (int c = 0; c < 200; ++c) cells.push_back({QRectF(c * 10, r * 10, 10, 10), r * 200 + c});
    std::vector<QPointF> scribble;
    for (int i = 0; i <= kMostPaintPieces; ++i) scribble.push_back({double((i * 37) % 2000), double((i * 53) % 2520)});
    qint64 work = 0;
    sidesUnderPath(cells, scribble, 3, &work);
    QVERIFY2(work <= kMostPaintComparisons && work > kMostPaintComparisons - 8, qPrintable(QString::number(work)));   // (the path is cut where the looking is spent)
    // A line of the size of a drag along a few cells takes a few steps, and finds the sides.
    qint64 small = 0;
    const auto sides = sidesUnderPath(cells, {{100, 4}, {100, 26}, {100, 36}}, 3, &small);
    QVERIFY(small > 0 && small < 500);
    QVERIFY(std::any_of(sides.begin(), sides.end(), [](const PaintedSide &s) { return s.key == 10 && s.side == CellSide::Left; }));
}

void WriteTests::aBorderPainterDragThatStartsOutsideATableSaysSoOnce()
{
    TailWin win;
    win.w.view()->paginateAll();
    WriteDoc *doc = win.w.document();
    QWidget *vp = win.w.view()->viewport();
    win.w.act(QStringLiteral("table.paintBorder"))->trigger();
    QRect before;
    QVERIFY(win.w.view()->positionRect(positionOf(*doc, QStringLiteral("Before")), &before));
    QSignalSpy said(win.w.view(), &PageView::notice);
    const quint64 serial = doc->nextStepSerial();
    drag(vp, before.center(), before.center() + QPoint(60, 0));
    QCOMPARE(said.size(), 1);
    QVERIFY2(said.first().first().toString().contains(QStringLiteral("Drag along the lines of a table")), qPrintable(said.first().first().toString()));
    QCOMPARE(doc->nextStepSerial(), serial);
}

void WriteTests::paintingBordersPutsALineOnBothCellsInOneStepAndTheFileKeepsIt()
{
    auto doc = open(makeDocx(tableBody()));
    const int a = positionOf(*doc, QStringLiteral("a")), b = positionOf(*doc, QStringLiteral("b"));
    flow::TableLine line{flow::TableLine::Style::Double, 1.5, QColor(0xC0, 0x39, 0x2B)};
    const QString before = plain(*doc);
    QVERIFY(paintBorderEdges(*doc, {{a, BorderSide::Right}, {b, BorderSide::Left}}, line));
    QByteArray xml = documentXml(doc->toDocx());
    QVERIFY2(bordersOf(xml, "a").contains("<w:right w:val=\"double\" w:sz=\"12\""), bordersOf(xml, "a").constData());
    QVERIFY(bordersOf(xml, "a").contains("C0392B"));
    QVERIFY(bordersOf(xml, "b").contains("<w:left w:val=\"double\""));
    QVERIFY(bordersOf(xml, "c").isEmpty());
    QVERIFY(bordersOf(xml, "d").isEmpty());
    QCOMPARE(plain(*doc), before);
    // One undo takes the whole painting back.
    doc->story()->undo();
    xml = documentXml(doc->toDocx());
    QVERIFY(bordersOf(xml, "a").isEmpty());
    QVERIFY(bordersOf(xml, "b").isEmpty());
    // A line that is not visible takes a border off: the cell says none for the side.
    QVERIFY(paintBorderEdges(*doc, {{a, BorderSide::Right}}, line));
    QVERIFY(paintBorderEdges(*doc, {{a, BorderSide::Right}}, flow::TableLine()));
    QVERIFY2(bordersOf(documentXml(doc->toDocx()), "a").contains("<w:right w:val=\"nil\"/>"), bordersOf(documentXml(doc->toDocx()), "a").constData());
    // Edges of no table, or none at all, change nothing.
    const quint64 serial = doc->nextStepSerial();
    QVERIFY(!paintBorderEdges(*doc, {}, line));
    QCOMPARE(doc->nextStepSerial(), serial);
}

void WriteTests::draggingTheBorderPainterAlongALinePutsTheBorderOnBothSides()
{
    TailWin win;
    win.w.view()->paginateAll();
    const QHash<int, QRect> cells = win.cells();
    QCOMPARE(cells.size(), 4);
    const int a = positionOf(*win.w.document(), QStringLiteral("a")), b = positionOf(*win.w.document(), QStringLiteral("b"));
    QVERIFY(cells.contains(a) && cells.contains(b));
    QWidget *vp = win.w.view()->viewport();
    win.w.act(QStringLiteral("table.paintBorder"))->trigger();
    QCOMPARE(int(win.w.view()->tableTool()), int(PageView::TableTool::PaintBorder));
    QVERIFY(win.w.act(QStringLiteral("table.paintBorder"))->isChecked());
    // Down the line between a and b, from near the top of the row to near its bottom.
    const QRect ra = cells.value(a);
    const int x = ra.right() + 1;
    const QString before = plain(*win.w.document());
    drag(vp, QPoint(x, ra.top() + ra.height() / 10), QPoint(x, ra.bottom() - ra.height() / 10));
    QByteArray xml = win.xml();
    QVERIFY2(bordersOf(xml, "a").contains("<w:right w:val=\"single\""), bordersOf(xml, "a").constData());
    QVERIFY(bordersOf(xml, "b").contains("<w:left w:val=\"single\""));
    QVERIFY(bordersOf(xml, "c").isEmpty());
    QVERIFY(bordersOf(xml, "d").isEmpty());
    QCOMPARE(plain(*win.w.document()), before);
    // The tool stays on for the next drag; Esc puts it down.
    QCOMPARE(int(win.w.view()->tableTool()), int(PageView::TableTool::PaintBorder));
    // One undo.
    win.w.document()->story()->undo();
    QVERIFY(bordersOf(win.xml(), "a").isEmpty());
    // A drag across a cell's middle paints nothing.
    const quint64 serial = win.w.document()->nextStepSerial();
    drag(vp, QPoint(ra.left() + 8, ra.center().y()), QPoint(ra.right() - 8, ra.center().y()));
    QCOMPARE(win.w.document()->nextStepSerial(), serial);
    QTest::keyClick(vp, Qt::Key_Escape);
    QCOMPARE(int(win.w.view()->tableTool()), int(PageView::TableTool::None));
    QVERIFY(!win.w.act(QStringLiteral("table.paintBorder"))->isChecked());
}

void WriteTests::theBorderPainterWithNoBorderAsItsStyleErasesTheLinesItIsDraggedAlong()
{
    TailWin win;
    win.w.view()->paginateAll();
    QTextCursor c = win.w.view()->cursor();
    setBorders(*win.w.document(), c, BorderSide::All, flow::TableLine{flow::TableLine::Style::Single, 1, QColor()});
    win.w.view()->refresh();
    QVERIFY(bordersOf(win.xml(), "b").contains("<w:left w:val=\"single\""));
    const QHash<int, QRect> cells = win.cells();
    const int a = positionOf(*win.w.document(), QStringLiteral("a"));
    const QRect ra = cells.value(a);
    // The pen's style menu has No Border; the command takes it as the style.
    win.w.act(QStringLiteral("table.paintBorder"))->trigger();
    win.w.view()->setBorderPen(flow::TableLine());
    drag(win.w.view()->viewport(), QPoint(ra.left() + 8, ra.bottom()), QPoint(ra.right() - 8, ra.bottom()));
    const QByteArray xml = win.xml();
    QVERIFY2(bordersOf(xml, "a").contains("<w:bottom w:val=\"nil\"/>"), bordersOf(xml, "a").constData());
    QVERIFY(bordersOf(xml, "c").contains("<w:top w:val=\"nil\"/>"));
    QVERIFY(bordersOf(xml, "a").contains("<w:top w:val=\"single\""));   // (the other sides stay)
}

void WriteTests::paintedBordersSurviveSaveAndOpenInEveryFormat()
{
    TailWin win;
    win.w.view()->paginateAll();
    QTextCursor c = win.w.view()->cursor();
    const int a = positionOf(*win.w.document(), QStringLiteral("a")), b = positionOf(*win.w.document(), QStringLiteral("b"));
    QVERIFY(paintBorderEdges(*win.w.document(), {{a, BorderSide::Right}, {b, BorderSide::Left}}, flow::TableLine{flow::TableLine::Style::Dashed, 2, QColor(0x2B, 0x57, 0xB5)}));
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    for (const QString &name : {QStringLiteral("painted.docx"), QStringLiteral("painted.odt"), QStringLiteral("painted.rtf")}) {
        const QString path = dir.filePath(name);
        QVERIFY2(win.w.saveTo(path, true), qPrintable(name));
        WriteWindow back;
        back.resize(900, 700);
        QVERIFY2(back.openFile(path), qPrintable(name));
        QVERIFY(back.document());
        const QByteArray xml = documentXml(back.document()->toDocx());
        const QByteArray left = bordersOf(xml, "a"), right = bordersOf(xml, "b");
        // (each format keeps the side, the dashes, and the color; what the file cannot say is stated in the Help topic for tables)
        QVERIFY2(left.contains("<w:right w:val=\"dashed\"") && left.contains("2B57B5"), qPrintable(name + QStringLiteral(": ") + QString::fromLatin1(left)));
        QVERIFY2(right.contains("<w:left w:val=\"dashed\""), qPrintable(name + QStringLiteral(": ") + QString::fromLatin1(right)));
        QVERIFY2(!bordersOf(xml, "d").contains("dashed"), qPrintable(name));
    }
}

void WriteTests::paintingBordersWithTrackChangesOnIsNotRecordedAsTheOtherBorderCommandsAreNot()
{
    auto viaCommand = open(makeDocx(tableBody()));
    auto viaPainter = open(makeDocx(tableBody()));
    viaCommand->setTrackChanges(true);
    viaPainter->setTrackChanges(true);
    QTextCursor c(viaCommand->story());
    c.setPosition(positionOf(*viaCommand, QStringLiteral("a")));
    setBorders(*viaCommand, c, BorderSide::Right, flow::TableLine{flow::TableLine::Style::Single, 0.5, QColor()});
    QVERIFY(paintBorderEdges(*viaPainter, {{positionOf(*viaPainter, QStringLiteral("a")), BorderSide::Right}}, flow::TableLine{flow::TableLine::Style::Single, 0.5, QColor()}));
    const QByteArray one = documentXml(viaCommand->toDocx()), two = documentXml(viaPainter->toDocx());
    // Neither records the change (the same as each other), and the painted one has its line.
    QCOMPARE(one.contains("tcPrChange"), two.contains("tcPrChange"));
    QVERIFY(!two.contains("<w:ins ") && !two.contains("<w:del "));
    QVERIFY(bordersOf(two, "a").contains("<w:right w:val=\"single\""));
}

void WriteTests::theBorderPainterIsInTheTableDesignTabAndTheCellMenu()
{
    TailWin win;
    QVERIFY(win.w.act(QStringLiteral("table.paintBorder")));
    QCOMPARE(win.w.act(QStringLiteral("table.paintBorder"))->text(), QStringLiteral("Border Painter"));
    QVERIFY(win.w.ribbon()->tab(QStringLiteral("Table Design")));
    win.w.view()->paginateAll();
    QStringList ids;
    QTimer::singleShot(150, [&] {
        if (auto *m = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
            std::function<void(QMenu *)> collect = [&](QMenu *menu) {
                for (QAction *action : menu->actions()) {
                    if (action->menu()) collect(action->menu());
                    ids << action->objectName();
                }
            };
            collect(m);
            m->close();
        }
    });
    Q_EMIT win.w.view()->contextMenuWanted(win.w.view()->mapToGlobal(QPoint(300, 300)));
    QVERIFY2(ids.contains(QStringLiteral("table.paintBorder")), qPrintable(ids.join(QLatin1Char('|'))));
    QVERIFY(ids.contains(QStringLiteral("table.moveUp")) && ids.contains(QStringLiteral("table.moveDown")));
}

void WriteTests::aTableHandleAtTheCornerMovesTheTableWhenPulledAndSelectsItWhenClicked()
{
    TailWin win;
    win.w.view()->paginateAll();
    QRect handle;
    for (int i = 0; i < 200 && !win.w.view()->moveHandleInView(&handle); ++i) QCoreApplication::processEvents();
    QVERIFY(win.w.view()->moveHandleInView(&handle));
    const QHash<int, QRect> cells = win.cells();
    const int a = positionOf(*win.w.document(), QStringLiteral("a"));
    QVERIFY(handle.right() <= cells.value(a).left() + 1 && handle.bottom() <= cells.value(a).top() + 1);   // (at the upper left corner, outside the cells)
    QWidget *vp = win.w.view()->viewport();
    // A click selects the whole table.
    QTest::mouseClick(vp, Qt::LeftButton, Qt::NoModifier, handle.center());
    QTextCursor sel = win.w.view()->cursor();
    QVERIFY(sel.hasSelection());
    QVERIFY(sel.selectionStart() <= a);
    QVERIFY(sel.selectionEnd() > positionOf(*win.w.document(), QStringLiteral("d")));
    QCOMPARE(wordsInOrder(win.xml()), (QStringList{"Before", "a", "b", "c", "d", "After one", "After two"}));
    // Pulled to the paragraph "After two": the table goes before it, as one step.
    QTextCursor c(win.w.document()->story());
    c.setPosition(positionOf(*win.w.document(), QStringLiteral("After two")) + 3);
    QRect where;
    QVERIFY(win.w.view()->positionRect(c.position(), &where));
    win.w.view()->setCursor(QTextCursor(win.w.document()->story()));
    QTextCursor inA(win.w.document()->story());
    inA.setPosition(a);
    win.w.view()->setCursor(inA);
    QVERIFY(win.w.view()->moveHandleInView(&handle));
    drag(vp, handle.center(), where.center());
    QCOMPARE(wordsInOrder(win.xml()), (QStringList{"Before", "After one", "a", "b", "c", "d", "After two"}));
    QVERIFY(win.w.view()->cursor().hasSelection());
    win.w.document()->story()->undo();
    QCOMPARE(wordsInOrder(win.xml()), (QStringList{"Before", "a", "b", "c", "d", "After one", "After two"}));
    // Let go inside the table: nothing moves.
    inA.setPosition(a);
    win.w.view()->setCursor(inA);
    QVERIFY(win.w.view()->moveHandleInView(&handle));
    const QRect rd = win.cells().value(positionOf(*win.w.document(), QStringLiteral("d")));
    drag(vp, handle.center(), rd.center());
    QCOMPARE(wordsInOrder(win.xml()), (QStringList{"Before", "a", "b", "c", "d", "After one", "After two"}));
    // Esc during a pull lets it go.
    QVERIFY(win.w.view()->moveHandleInView(&handle));
    QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, handle.center());
    QTest::mouseMove(vp, where.center());
    QTest::mouseMove(vp, where.center() + QPoint(0, 2));
    QTest::keyClick(vp, Qt::Key_Escape);
    QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, where.center());
    QCOMPARE(wordsInOrder(win.xml()), (QStringList{"Before", "a", "b", "c", "d", "After one", "After two"}));
}

void WriteTests::movingATableKeepsItsCellsAndBordersThroughEveryFormat()
{
    TailWin win;
    win.w.view()->paginateAll();
    WriteDoc *doc = win.w.document();
    const int a = positionOf(*doc, QStringLiteral("a")), b = positionOf(*doc, QStringLiteral("b"));
    QVERIFY(paintBorderEdges(*doc, {{a, BorderSide::Right}, {b, BorderSide::Left}}, flow::TableLine{flow::TableLine::Style::Double, 1.5, QColor()}));
    QTextCursor c = win.w.view()->cursor();
    QVERIFY(moveTable(*doc, c, positionOf(*doc, QStringLiteral("a")), positionOf(*doc, QStringLiteral("After two"))));
    QCOMPARE(wordsInOrder(win.xml()), (QStringList{"Before", "After one", "a", "b", "c", "d", "After two"}));
    QVERIFY(bordersOf(win.xml(), "a").contains("<w:right w:val=\"double\""));
    QTemporaryDir dir;
    for (const QString &name : {QStringLiteral("moved.docx"), QStringLiteral("moved.odt"), QStringLiteral("moved.rtf")}) {
        const QString path = dir.filePath(name);
        QVERIFY2(win.w.saveTo(path, true), qPrintable(name));
        WriteWindow back;
        QVERIFY2(back.openFile(path), qPrintable(name));
        const QByteArray xml = documentXml(back.document()->toDocx());
        QCOMPARE(wordsInOrder(xml), (QStringList{"Before", "After one", "a", "b", "c", "d", "After two"}));
        QVERIFY2(bordersOf(xml, "a").contains("<w:right w:val=\"double\""), qPrintable(name));
    }
}

void WriteTests::moveTableUpAndDownGoPastOneParagraphEach()
{
    auto doc = open(makeDocx(tableBody()));
    QTextCursor c(doc->story());
    c.setPosition(positionOf(*doc, QStringLiteral("a")));
    QVERIFY(moveTableByParagraph(*doc, c, true));
    QCOMPARE(wordsInOrder(documentXml(doc->toDocx())), (QStringList{"a", "b", "c", "d", "Before", "After one", "After two"}));
    c.setPosition(positionOf(*doc, QStringLiteral("a")));
    QVERIFY(!moveTableByParagraph(*doc, c, true));   // (nothing above it)
    QVERIFY(!doc->tableNotice.isEmpty());
    QVERIFY(moveTableByParagraph(*doc, c, false));
    QCOMPARE(wordsInOrder(documentXml(doc->toDocx())), (QStringList{"Before", "a", "b", "c", "d", "After one", "After two"}));
    c.setPosition(positionOf(*doc, QStringLiteral("a")));
    QVERIFY(moveTableByParagraph(*doc, c, false));
    QCOMPARE(wordsInOrder(documentXml(doc->toDocx())), (QStringList{"Before", "After one", "a", "b", "c", "d", "After two"}));
    doc->story()->undo();
    QCOMPARE(wordsInOrder(documentXml(doc->toDocx())), (QStringList{"Before", "a", "b", "c", "d", "After one", "After two"}));
    // The window's commands are the same, in the table's tab and menu.
    TailWin win;
    win.w.act(QStringLiteral("table.moveDown"))->trigger();
    QCOMPARE(wordsInOrder(win.xml()), (QStringList{"Before", "After one", "a", "b", "c", "d", "After two"}));
    win.w.act(QStringLiteral("table.moveUp"))->trigger();
    QCOMPARE(wordsInOrder(win.xml()), (QStringList{"Before", "a", "b", "c", "d", "After one", "After two"}));
}

namespace {

// The paragraphs that are not in a table, each as the file writes it (the tables of these fixtures are not nested).
QList<QByteArray> paragraphsOutsideTables(QByteArray x)
{
    int at = 0;
    while ((at = x.indexOf("<w:tbl>")) >= 0) x.remove(at, x.indexOf("</w:tbl>", at) + 8 - at);
    QList<QByteArray> out;
    for (at = 0; (at = x.indexOf("<w:p", at)) >= 0;) {
        if (x[at + 4] != '>' && x[at + 4] != ' ') {
            ++at;
            continue;
        }
        const int end = x.indexOf("</w:p>", at) + 6;
        out << x.mid(at, end - at);
        at = end;
    }
    return out;
}

// Paragraphs whose marks carry run formatting and more, around a table: what a move must not touch.
QByteArray markedBody()
{
    return "<w:p><w:pPr><w:jc w:val=\"center\"/><w:rPr><w:b/></w:rPr></w:pPr><w:r><w:t>Before</w:t></w:r></w:p>"
           "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid>"
           "<w:tr>" + cellXml("a") + cellXml("b") + "</w:tr></w:tbl>"
           "<w:p><w:pPr><w:keepNext/><w:spacing w:after=\"240\"/><w:rPr><w:b/><w:i/><w:color w:val=\"FF0000\"/></w:rPr></w:pPr><w:r><w:t>After one</w:t></w:r></w:p>"
           "<w:p><w:pPr><w:rPr><w:b/></w:rPr></w:pPr><w:r><w:t>After two</w:t></w:r></w:p>";
}

QByteArray oneCellTable(const char *text)
{
    return "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr>" + cellXml(text) + "</w:tr></w:tbl>";
}

QByteArray para(const char *text)
{
    return QByteArray("<w:p><w:r><w:t>") + text + "</w:t></w:r></w:p>";
}

// What a move up or down does for the table with this text: whether the window would offer it, whether it was done, and what it said.
struct Step {
    bool offered = false, done = false;
    QString said;
};

Step stepOf(const QByteArray &body, const char *inTableWith, bool up)
{
    auto doc = open(makeDocx(body));
    QTextCursor c(doc->story());
    c.setPosition(positionOf(*doc, QString::fromLatin1(inTableWith)));
    Step s;
    s.offered = canMoveTableByParagraph(*doc, c, up);
    s.done = moveTableByParagraph(*doc, c, up);
    s.said = doc->tableNotice;
    return s;
}

QByteArray savedAndReopened(WriteWindow &w, const QString &path)
{
    if (!w.saveTo(path, true)) return "(not saved)";
    WriteWindow back;
    if (!back.openFile(path)) return "(not opened)";
    return documentXml(back.document()->toDocx());
}

} // namespace

void WriteTests::movingATableLeavesEveryOtherParagraphAsItWasSavedUpAndDown()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    for (const bool up : {true, false}) {
        for (const QString &ext : {QStringLiteral("docx"), QStringLiteral("odt"), QStringLiteral("rtf")}) {
            const QString tag = QStringLiteral("%1 %2").arg(up ? QStringLiteral("up") : QStringLiteral("down"), ext);
            TailWin still(markedBody());
            const QList<QByteArray> before = paragraphsOutsideTables(savedAndReopened(still.w, dir.filePath(QStringLiteral("still.") + ext)));
            TailWin moved(markedBody());
            QTextCursor c = moved.w.view()->cursor();
            QVERIFY2(moveTableByParagraph(*moved.w.document(), c, up), qPrintable(tag));
            const QList<QByteArray> after = paragraphsOutsideTables(savedAndReopened(moved.w, dir.filePath(QStringLiteral("moved.") + ext)));
            QCOMPARE(after.size(), before.size());
            for (int i = 0; i < before.size(); ++i) QVERIFY2(after[i] == before[i], qPrintable(tag + QStringLiteral(": ") + QString::fromLatin1(before[i]) + QStringLiteral(" became ") + QString::fromLatin1(after[i])));
            if (ext == QLatin1String("docx")) QVERIFY2(before[1].contains("<w:b/>") && before[1].contains("<w:i/>") && before[1].contains("FF0000"), before[1].constData());   // (the file keeps the marks at all)
        }
    }
}

void WriteTests::aTableIsOfferedAMoveOnlyWhereTheMoveIsDoneAndAFailedMoveSaysWhy()
{
    // (body, text in the table, direction, offered, said when it is not)
    struct Case {
        QByteArray body;
        const char *table;
        bool up, offered;
        const char *said;   // a part of the notice when the move is refused
    };
    const Case cases[] = {
        {oneCellTable("t") + para("p") + para("q"), "t", true, false, "no paragraph above"},
        {oneCellTable("t") + para("p") + para("q"), "t", false, true, ""},
        {para("p") + oneCellTable("t") + para("q"), "t", true, true, ""},
        {para("p") + oneCellTable("t") + para("q"), "t", false, false, "last paragraph"},
        {para("p") + oneCellTable("t") + para("q") + para("r"), "t", false, true, ""},
        // table, one paragraph, table: the first cannot go down and the second cannot go up, and each says what is true
        {oneCellTable("t") + para("p") + oneCellTable("u") + para("q"), "t", false, false, "against another table"},
        {oneCellTable("t") + para("p") + oneCellTable("u") + para("q"), "u", true, false, "against another table"},
        {oneCellTable("t") + para("p") + oneCellTable("u") + para("q") + para("r"), "u", false, true, ""},
        {para("o") + oneCellTable("t") + para("p") + oneCellTable("u") + para("q"), "t", true, true, ""},
    };
    int n = 0;
    for (const Case &c : cases) {
        const Step s = stepOf(c.body, c.table, c.up);
        const QString why = QStringLiteral("case %1").arg(n++);
        QVERIFY2(s.offered == c.offered, qPrintable(why + QStringLiteral(" offered")));
        QVERIFY2(s.done == c.offered, qPrintable(why + QStringLiteral(" done: ") + s.said));
        if (!c.offered) QVERIFY2(s.said.contains(QString::fromLatin1(c.said)), qPrintable(why + QStringLiteral(": ") + s.said));
    }
    // The window's command is off in the same places.
    TailWin win(oneCellTable("t") + para("p") + oneCellTable("u") + para("q"));
    QTextCursor in(win.w.document()->story());
    in.setPosition(positionOf(*win.w.document(), QStringLiteral("t")));
    win.w.view()->setCursor(in);
    win.w.refreshForTest();
    QVERIFY(!win.w.act(QStringLiteral("table.moveDown"))->isEnabled());
    QVERIFY(!win.w.act(QStringLiteral("table.moveUp"))->isEnabled());
}

void WriteTests::aTableIsNotPutRightAgainstAnotherTable()
{
    auto doc = open(makeDocx(oneCellTable("t") + para("p") + oneCellTable("u") + para("q") + para("r")));
    const QStringList was = wordsInOrder(documentXml(doc->toDocx()));
    const quint64 serial = doc->nextStepSerial();
    QTextCursor c(doc->story());
    // Dropped where the paragraph after the other table is: the two would meet.
    QVERIFY(!moveTable(*doc, c, positionOf(*doc, QStringLiteral("t")), positionOf(*doc, QStringLiteral("q"))));
    QVERIFY2(doc->tableNotice.contains(QStringLiteral("against another table")), qPrintable(doc->tableNotice));
    // The second table dropped where the paragraph after the first one is: the same.
    QVERIFY(!moveTable(*doc, c, positionOf(*doc, QStringLiteral("u")), positionOf(*doc, QStringLiteral("p"))));
    QVERIFY(doc->tableNotice.contains(QStringLiteral("against another table")));
    QCOMPARE(wordsInOrder(documentXml(doc->toDocx())), was);
    QCOMPARE(doc->nextStepSerial(), serial);
    // Dropped where a paragraph is between them, it goes.
    QVERIFY(moveTable(*doc, c, positionOf(*doc, QStringLiteral("t")), positionOf(*doc, QStringLiteral("r"))));
    QCOMPARE(wordsInOrder(documentXml(doc->toDocx())), (QStringList{"p", "u", "q", "t", "r"}));
}

void WriteTests::aTablePulledAndLetGoInAnotherTableMovesNothingAndSaysSo()
{
    TailWin win(oneCellTable("t") + para("p") + oneCellTable("u") + para("q"));
    win.w.view()->paginateAll();
    WriteDoc *doc = win.w.document();
    const QStringList was = wordsInOrder(win.xml());
    QRect handle;
    for (int i = 0; i < 200 && !win.w.view()->moveHandleInView(&handle); ++i) QCoreApplication::processEvents();
    QVERIFY(win.w.view()->moveHandleInView(&handle));
    QRect inU;
    QVERIFY(win.w.view()->positionRect(positionOf(*doc, QStringLiteral("u")), &inU));
    QSignalSpy said(win.w.view(), &PageView::notice);
    const quint64 serial = doc->nextStepSerial();
    drag(win.w.view()->viewport(), handle.center(), inU.center() + QPoint(2, 0));
    QCOMPARE(wordsInOrder(win.xml()), was);
    QCOMPARE(doc->nextStepSerial(), serial);
    QCOMPARE(said.size(), 1);
    QVERIFY2(said.first().first().toString().contains(QStringLiteral("between paragraphs")), qPrintable(said.first().first().toString()));
}

void WriteTests::movingATableWithTrackChangesOnRecordsNothingAsDeleteTableDoesNot()
{
    auto moved = open(makeDocx(tableBody()));
    auto deleted = open(makeDocx(tableBody()));
    moved->setTrackChanges(true);
    deleted->setTrackChanges(true);
    QTextCursor c(moved->story());
    QVERIFY(moveTable(*moved, c, positionOf(*moved, QStringLiteral("a")), positionOf(*moved, QStringLiteral("After two"))));
    QTextCursor d(deleted->story());
    d.setPosition(positionOf(*deleted, QStringLiteral("a")));
    deleteTable(*deleted, d);
    const QByteArray x = documentXml(moved->toDocx()), y = documentXml(deleted->toDocx());
    // What Delete Table records, the move records (nothing is marked as moved), and the table is whole: each cell once, in the new place, with its row and cells.
    QCOMPARE(x.contains("<w:ins "), y.contains("<w:ins "));
    QCOMPARE(x.contains("<w:del "), y.contains("<w:del "));
    QCOMPARE(x.contains("<w:moveFrom"), false);
    QCOMPARE(wordsInOrder(x), (QStringList{"Before", "After one", "a", "b", "c", "d", "After two"}));
    QCOMPARE(x.count("<w:tc>"), 4);
    QCOMPARE(x.count("<w:tr>") + x.count("<w:tr "), 2);
    QVERIFY(!y.contains("<w:t>a</w:t>") || y.contains("<w:del "));
}

void WriteTests::aTableMoveIsOneLookAtItsNeighborsNotAWalkOfTheTable()
{
    QByteArray rows;
    for (int i = 0; i < 3000; ++i) rows += "<w:tr>" + cellXml("cell") + "</w:tr>";
    const QByteArray body = para("Before") + "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid>" + rows + "</w:tbl>" + para("After one") + para("After two");
    auto doc = open(makeDocx(body));
    QTextCursor c(doc->story());
    c.setPosition(positionOf(*doc, QStringLiteral("cell")) + 5000);
    QVERIFY(inTable(doc->story()->findBlock(c.position())));
    QVERIFY(canMoveTableByParagraph(*doc, c, true));   // (the first look finds the table; the rest are what is counted)
    const qint64 looks = tableMoveLooksForTest(), blocks = tableMoveBlocksLookedAtForTest();
    QVERIFY(canMoveTableByParagraph(*doc, c, false));
    QCOMPARE(tableMoveLooksForTest() - looks, qint64(1));
    QVERIFY2(tableMoveBlocksLookedAtForTest() - blocks <= 8, qPrintable(QString::number(tableMoveBlocksLookedAtForTest() - blocks)));
    // The window asks once for both commands.
    TailWin win(body);
    QTextCursor in(win.w.document()->story());
    in.setPosition(positionOf(*win.w.document(), QStringLiteral("cell")) + 5000);
    win.w.view()->setCursor(in);
    win.w.refreshForTest();
    const qint64 looksBefore = tableMoveLooksForTest(), blocksBefore = tableMoveBlocksLookedAtForTest();
    win.w.refreshForTest();
    QCOMPARE(tableMoveLooksForTest() - looksBefore, qint64(1));
    QVERIFY2(tableMoveBlocksLookedAtForTest() - blocksBefore <= 8, qPrintable(QString::number(tableMoveBlocksLookedAtForTest() - blocksBefore)));
    QVERIFY(win.w.act(QStringLiteral("table.moveUp"))->isEnabled() && win.w.act(QStringLiteral("table.moveDown"))->isEnabled());
}

void WriteTests::lettingGoOfAPullOnLosingTheFocusDoesNotMoveTheTableOrPaintBorders()
{
    TailWin win;
    win.w.view()->paginateAll();
    WriteDoc *doc = win.w.document();
    QWidget *vp = win.w.view()->viewport();
    QRect handle, where;
    for (int i = 0; i < 200 && !win.w.view()->moveHandleInView(&handle); ++i) QCoreApplication::processEvents();
    QVERIFY(win.w.view()->moveHandleInView(&handle));
    QVERIFY(win.w.view()->positionRect(positionOf(*doc, QStringLiteral("After two")), &where));
    const QStringList was = wordsInOrder(win.xml());
    QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, handle.center());
    for (int i = 1; i <= 6; ++i) QTest::mouseMove(vp, handle.center() + (where.center() - handle.center()) * i / 6);
    QFocusEvent out(QEvent::FocusOut, Qt::ActiveWindowFocusReason);
    QApplication::sendEvent(win.w.view(), &out);
    QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, where.center());
    QCOMPARE(wordsInOrder(win.xml()), was);
    // The border painter's drag the same.
    const QRect ra = win.cells().value(positionOf(*doc, QStringLiteral("a")));
    win.w.act(QStringLiteral("table.paintBorder"))->trigger();
    const quint64 serial = doc->nextStepSerial();
    const int x = ra.right() + 1;
    QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, QPoint(x, ra.top() + 4));
    for (int i = 1; i <= 8; ++i) QTest::mouseMove(vp, QPoint(x, ra.top() + 4 + (ra.height() - 8) * i / 8));
    QApplication::sendEvent(win.w.view(), &out);
    QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, QPoint(x, ra.bottom() - 4));
    QCOMPARE(doc->nextStepSerial(), serial);
}

void WriteTests::theMoveHandleIsNotShownWhereTheDocumentCannotBeChanged()
{
    TailWin win;
    win.w.view()->paginateAll();
    QRect handle;
    for (int i = 0; i < 200 && !win.w.view()->moveHandleInView(&handle); ++i) QCoreApplication::processEvents();
    QVERIFY(win.w.view()->moveHandleInView(&handle));
    // (read-only, and protected: both are the view held for reading)
    win.w.setReadOnly(true);
    QVERIFY(!win.w.view()->moveHandleInView(&handle));
    const QStringList was = wordsInOrder(win.xml());
    QTest::mousePress(win.w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(40, 40));
    QTest::mouseRelease(win.w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(40, 40));
    QCOMPARE(wordsInOrder(win.xml()), was);
    win.w.setReadOnly(false);
    QTextCursor inA(win.w.document()->story());
    inA.setPosition(positionOf(*win.w.document(), QStringLiteral("a")));
    win.w.view()->setCursor(inA);
    QVERIFY(win.w.view()->moveHandleInView(&handle));
}

void WriteTests::aTableMovesInAColumnSectionAndNotInAHeaderWithNothingLost()
{
    // A section of two columns that ends at "After two", and one after it: the table goes down past a paragraph, then past the one that ends the section.
    const QByteArray cols = "<w:p><w:pPr><w:sectPr><w:type w:val=\"continuous\"/><w:cols w:num=\"2\" w:space=\"720\"/></w:sectPr></w:pPr><w:r><w:t>After two</w:t></w:r></w:p>";
    auto doc = open(makeDocx(para("Before") + oneCellTable("t") + para("After one") + cols + para("After three")));
    QTextCursor c(doc->story());
    c.setPosition(positionOf(*doc, QStringLiteral("t")));
    QVERIFY(moveTableByParagraph(*doc, c, false));
    QVERIFY(moveTableByParagraph(*doc, c, false));
    QByteArray x = documentXml(doc->toDocx());
    QCOMPARE(wordsInOrder(x), (QStringList{"Before", "After one", "After two", "t", "After three"}));
    QCOMPARE(x.count("<w:sectPr"), 2);
    const QList<QByteArray> ps = paragraphsOutsideTables(x);
    QVERIFY2(ps[2].contains("<w:cols w:num=\"2\"") && ps[2].contains("After two"), ps[2].constData());   // (the end of the section is still on its paragraph)
    doc->story()->undo();
    doc->story()->undo();
    QCOMPARE(wordsInOrder(documentXml(doc->toDocx())), (QStringList{"Before", "t", "After one", "After two", "After three"}));
    // A table in a header: the commands are off, and nothing of the text is touched, even by a call with the header's caret.
    const QByteArray header = "<w:hdr " + QByteArray(detail_tst_write::kNs) + ">" + oneCellTable("hcell") + "<w:p><w:r><w:t>hend</w:t></w:r></w:p></w:hdr>";
    WriteWindow w;
    w.resize(1100, 800);
    w.show();
    w.setDocument(open(makeDocxWithParts(para("x") + oneCellTable("t") + para("y") + para("z"), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), header}})), QString());
    w.view()->paginateAll();
    QVERIFY(w.view()->enterHeaderFooter(0, false, true));
    QTextCursor hc(w.document()->headerFooterStory(0, false, 0));
    hc.setPosition(1);
    w.view()->setCursor(hc);
    w.refreshForTest();
    QVERIFY(!w.act(QStringLiteral("table.moveUp"))->isEnabled());
    QVERIFY(!w.act(QStringLiteral("table.moveDown"))->isEnabled());
    const QByteArray bodyBefore = documentXml(w.document()->toDocx());
    QTextCursor viaCall = w.view()->cursor();
    QVERIFY(!canMoveTableByParagraph(*w.document(), viaCall, false));
    QVERIFY(!moveTableByParagraph(*w.document(), viaCall, false));
    QVERIFY(!w.document()->tableNotice.isEmpty());
    QCOMPARE(documentXml(w.document()->toDocx()), bodyBefore);
}

void WriteTests::showNotesGoesFromAMarkToItsNoteAndBackAndFromTheTextToTheNearestMark()
{
    WriteWindow w;
    w.resize(1300, 1000);
    w.show();
    w.setDocument(open(threeNotesDocx()), QString());
    WriteDoc *doc = w.document();
    w.view()->paginateAll();
    QAction *show = w.act(QStringLiteral("refs.showNotes"));
    QVERIFY(show);
    QCOMPARE(show->text(), QStringLiteral("Show Notes"));
    const QVector<int> refs = doc->footnoteReferences(3);
    QVERIFY(!refs.isEmpty());
    QTextCursor c(doc->story());
    c.setPosition(refs.first() + 1);
    w.view()->setCursor(c);
    w.refreshForTest();
    QVERIFY(show->isEnabled());
    show->trigger();
    QVERIFY(w.view()->inNote());
    QCOMPARE(w.view()->noteEdit().id, 3);
    show->trigger();
    QVERIFY(!w.view()->inNote());
    QCOMPARE(w.view()->cursor().position(), refs.first() + 1);
    // The caret in the words before the first mark: the nearest mark after it.
    c.setPosition(1);
    w.view()->setCursor(c);
    show->trigger();
    QVERIFY(w.view()->inNote());
    QCOMPARE(w.view()->noteEdit().id, 2);
    w.view()->leaveNote(true);
    // A document with no footnotes: the command is off.
    TailWin none;
    none.w.refreshForTest();
    QVERIFY(!none.w.act(QStringLiteral("refs.showNotes"))->isEnabled());
}

// A developer's hook, not a test: JW_TAIL_SHOTS names a folder for pictures of the border painter, the move handle, and Show Notes at work.
namespace {

// The pen the painted-border tests use: red, thick enough to be found in a picture of the page.
flow::TableLine redPen() { return flow::TableLine{flow::TableLine::Style::Single, 4.5, QColor(255, 0, 0)}; }

// The columns of a picture where red is drawn between two rows, as the first and the last (-1: none).
struct RedColumns {
    int first = -1, last = -1;
    double middle() const { return (first + last) / 2.0; }
};

RedColumns redColumns(const QImage &img, int y0, int y1, int x0 = 0, int x1 = 1 << 20)
{
    RedColumns out;
    for (int y = std::max(0, y0); y < std::min(img.height(), y1); ++y)
        for (int x = std::max(0, x0); x < std::min(img.width(), x1); ++x) {
            const QColor c = img.pixelColor(x, y);
            if (c.red() < 200 || c.green() > 90 || c.blue() > 90) continue;
            if (out.first < 0 || x < out.first) out.first = x;
            out.last = std::max(out.last, x);
        }
    return out;
}

// The page's first picture, as large as the page is in the view (so that a point of the view is a point of the picture, less the page's corner).
QImage pageAsInView(PageView *view) { return view->pageImage(0, view->pageRect(0).width()); }

// A table of two columns, "WWWWWWWW" and "i", marked right to left (the file format's visual-order property) or not.
QByteArray twoCellTable(bool rtl)
{
    return QByteArray("<w:tbl><w:tblPr>") + (rtl ? "<w:bidiVisual/>" : "") + "<w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid>"
           "<w:tr>" + cellXml("WWWWWWWW") + cellXml("i") + "</w:tr></w:tbl><w:p><w:r><w:t>End</w:t></w:r></w:p>";
}

} // namespace

// Measured on Nov 22: our layout does not read the table's right-to-left mark (it is read and written, never used): the cells are in the same places as in the same table without it,
// and a "left" border is drawn at the physical left of its cell, a "right" one at the physical right. The painter stores the side that is drawn where the pointer went, and this holds that
// down by the picture: if the layout ever mirrors a table, the sides would have to be swapped, and this fails first.
void WriteTests::aBorderPaintedInARightToLeftTableIsStoredOnTheSideThatIsDrawnWhereTheDragWent()
{
    jp::initCore();
    QList<double> leftDrawn, rightDrawn;
    for (const bool rtl : {false, true}) {
        const QString why = rtl ? QStringLiteral("right to left") : QStringLiteral("left to right");
        TailWin win(twoCellTable(rtl));
        win.w.view()->paginateAll();
        const QHash<int, QRect> cells = win.cells();
        QCOMPARE(cells.size(), 2);
        const int a = positionOf(*win.w.document(), QStringLiteral("W")), b = positionOf(*win.w.document(), QStringLiteral("i"));
        const QRect ra = cells.value(a), rb = cells.value(b);
        QVERIFY2(ra.right() <= rb.left() + 1, qPrintable(why + QStringLiteral(": the first cell is not at the left")));   // (cells are not mirrored)
        win.w.act(QStringLiteral("table.paintBorder"))->trigger();
        win.w.view()->setBorderPen(redPen());
        QWidget *vp = win.w.view()->viewport();
        const QPoint corner = win.w.view()->pageRect(0).topLeft();
        // Down the table's left edge, from the first cell's top to its bottom: the first cell's left side.
        drag(vp, QPoint(ra.left(), ra.top() + 2), QPoint(ra.left(), ra.bottom() - 2));
        QByteArray xml = win.xml();
        QVERIFY2(bordersOf(xml, "WWWWWWWW").contains("<w:left w:val=\"single\""), qPrintable(why + QStringLiteral(": ") + bordersOf(xml, "WWWWWWWW")));
        QVERIFY2(!bordersOf(xml, "WWWWWWWW").contains("<w:right"), qPrintable(why));
        QVERIFY2(bordersOf(xml, "i").isEmpty(), qPrintable(why));
        win.w.view()->refresh();
        RedColumns red = redColumns(pageAsInView(win.w.view()), ra.top() + 2 - corner.y(), ra.bottom() - 2 - corner.y());
        QVERIFY2(red.first >= 0, qPrintable(why + QStringLiteral(": no line drawn")));
        QVERIFY2(std::abs(red.middle() - (ra.left() - corner.x())) <= 4, qPrintable(why + QStringLiteral(": drawn at ") + QString::number(red.middle()) + QStringLiteral(", dragged at ") + QString::number(ra.left() - corner.x())));
        leftDrawn << red.middle() - (ra.left() - corner.x());
        // Down the line between the two cells: the first cell's right side and the second's left side, drawn where the line between them is.
        win.w.document()->story()->undo();
        const int between = ra.right() + 1;
        drag(vp, QPoint(between, ra.top() + 2), QPoint(between, ra.bottom() - 2));
        xml = win.xml();
        QVERIFY2(bordersOf(xml, "WWWWWWWW").contains("<w:right w:val=\"single\""), qPrintable(why + QStringLiteral(": ") + bordersOf(xml, "WWWWWWWW")));
        QVERIFY2(bordersOf(xml, "i").contains("<w:left w:val=\"single\""), qPrintable(why));
        QVERIFY2(!bordersOf(xml, "WWWWWWWW").contains("<w:left"), qPrintable(why));
        win.w.view()->refresh();
        red = redColumns(pageAsInView(win.w.view()), ra.top() + 2 - corner.y(), ra.bottom() - 2 - corner.y());
        QVERIFY2(red.first >= 0 && std::abs(red.middle() - (between - corner.x())) <= 4, qPrintable(why + QStringLiteral(": drawn at ") + QString::number(red.middle()) + QStringLiteral(", dragged at ") + QString::number(between - corner.x())));
        rightDrawn << red.middle() - (between - corner.x());
    }
    // The same table with and without the mark is drawn the same.
    QVERIFY(std::abs(leftDrawn[0] - leftDrawn[1]) < 1.5);
    QVERIFY(std::abs(rightDrawn[0] - rightDrawn[1]) < 1.5);
}

namespace {

// An outer table of two cells; the first holds a table of two cells ("n1", "n2"), then a paragraph ("tail"); the second is "o2".
QByteArray nestedTableBody()
{
    return "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"4000\"/><w:gridCol w:w=\"3000\"/></w:tblGrid><w:tr><w:tc>"
           "<w:tbl><w:tblPr><w:tblW w:w=\"3600\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"1800\"/><w:gridCol w:w=\"1800\"/></w:tblGrid><w:tr>" + cellXml("n1") + cellXml("n2") + "</w:tr></w:tbl>"
           "<w:p><w:r><w:t>tail</w:t></w:r></w:p></w:tc>" + cellXml("o2") + "</w:tr></w:tbl><w:p><w:r><w:t>End</w:t></w:r></w:p>";
}

} // namespace

// A table inside a cell is not a fragment of its own among the page's tables (the page lists the outermost tables, and each has its nested ones whole in its cells' contents):
// the drag looks for the innermost table that has the start of the drag in it, and so finds the nested one.
void WriteTests::aDragInsideANestedTablePaintsTheNestedTablesCellsAndNotTheOuterOnes()
{
    TailWin win(nestedTableBody());
    win.w.view()->paginateAll();
    PageView *view = win.w.view();
    const int n1 = positionOf(*win.w.document(), QStringLiteral("n1")), n2 = positionOf(*win.w.document(), QStringLiteral("n2"));
    QTextCursor c(win.w.document()->story());
    c.setPosition(n1);
    view->setCursor(c);
    win.w.refreshForTest();
    QRect first, second;
    QVERIFY(view->positionRect(n1, &first));
    QVERIFY(view->positionRect(n2, &second));
    // The line between the two nested cells is a cell margin (5.4 points, 7.2 pixels at the view's scale) before the second one's text, as tall as the text's line.
    const int x = second.left() - 7;
    win.w.act(QStringLiteral("table.paintBorder"))->trigger();
    QWidget *vp = view->viewport();
    const quint64 serial = win.w.document()->nextStepSerial();
    drag(vp, QPoint(x, first.top() - 1), QPoint(x, first.bottom() + 1));
    QVERIFY2(win.w.document()->nextStepSerial() != serial, "nothing was painted");
    const QByteArray xml = win.xml();
    QVERIFY2(bordersOf(xml, "n1").contains("<w:right w:val=\"single\""), qPrintable(bordersOf(xml, "n1")));
    QVERIFY2(bordersOf(xml, "n2").contains("<w:left w:val=\"single\""), qPrintable(bordersOf(xml, "n2")));
    QVERIFY(!bordersOf(xml, "n1").contains("<w:left") && !bordersOf(xml, "n2").contains("<w:right"));
    QCOMPARE(int(xml.count("<w:tcBorders>")), 2);   // (no outer cell has any)
    // A drag along the outer table's line between its cells (right of the nested table's cells) goes to the outer table.
    win.w.document()->story()->undo();
    QCOMPARE(int(win.xml().count("<w:tcBorders>")), 0);
    QRect tail, outerRight;
    QVERIFY(view->positionRect(positionOf(*win.w.document(), QStringLiteral("tail")), &tail));
    QVERIFY(view->positionRect(positionOf(*win.w.document(), QStringLiteral("o2")), &outerRight));
    const int between = outerRight.left() - 7;   // (the outer line: before the outer second cell's text, the same margin)
    drag(vp, QPoint(between, first.top()), QPoint(between, tail.bottom()));
    const QByteArray outer = win.xml();
    QVERIFY2(outer.count("<w:tcBorders>") >= 1, "nothing was painted on the outer table");
    QVERIFY2(bordersOf(outer, "o2").contains("<w:left w:val=\"single\""), qPrintable(bordersOf(outer, "o2")));
    QVERIFY(bordersOf(outer, "n1").isEmpty() && bordersOf(outer, "n2").isEmpty());
}

namespace {

// A column of one cell joined down over three rows ("M") beside three cells ("x1", "x2", "x3"), each row 30 points tall.
QByteArray mergedTableBody()
{
    auto row = [](const char *joined, const char *text) {
        return QByteArray("<w:tr><w:trPr><w:trHeight w:val=\"600\"/></w:trPr><w:tc><w:tcPr>") + joined + "</w:tcPr><w:p/></w:tc>" + cellXml(text) + "</w:tr>";
    };
    return "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid>" +
           QByteArray("<w:tr><w:trPr><w:trHeight w:val=\"600\"/></w:trPr><w:tc><w:tcPr><w:vMerge w:val=\"restart\"/></w:tcPr><w:p><w:r><w:t>M</w:t></w:r></w:p></w:tc>") + cellXml("x1") + "</w:tr>" +
           row("<w:vMerge/>", "x2") + row("<w:vMerge/>", "x3") + "</w:tbl><w:p><w:r><w:t>End</w:t></w:r></w:p>";
}

} // namespace

// What a drag along a joined cell's side does today (pinned, not designed): the joined cell's side is one side, so a drag that runs along at least half of its height puts the line on all of
// it, and one that runs along less puts it on none of it, while each unjoined cell beside it is painted by what the drag covers of its own side. Erasing along part of it therefore
// takes the line off the cells beside the part, and leaves the joined cell's own line, which is drawn there still.
void WriteTests::paintingAlongAPartOfAMergedCellsSideIsAllOrNothingByHalfAndErasingPartLeavesTheLine()
{
    jp::initCore();
    auto setup = [](TailWin &win, QHash<int, QRect> *cells, int *m, int *x1, int *x2, int *x3) {
        win.w.view()->paginateAll();
        *cells = win.cells();
        *m = positionOf(*win.w.document(), QStringLiteral("M"));
        *x1 = positionOf(*win.w.document(), QStringLiteral("x1"));
        *x2 = positionOf(*win.w.document(), QStringLiteral("x2"));
        *x3 = positionOf(*win.w.document(), QStringLiteral("x3"));
    };
    QHash<int, QRect> cells;
    int m = 0, p1 = 0, p2 = 0, p3 = 0;
    {
        // Two rows of three (more than half): the joined cell has its whole right side painted, the third cell beside it has none.
        TailWin win(mergedTableBody());
        setup(win, &cells, &m, &p1, &p2, &p3);
        QCOMPARE(cells.size(), 4);
        const QRect rm = cells.value(m), r1 = cells.value(p1), r2 = cells.value(p2);
        QVERIFY(rm.height() >= 2 * r1.height());
        win.w.act(QStringLiteral("table.paintBorder"))->trigger();
        drag(win.w.view()->viewport(), QPoint(rm.right() + 1, r1.top() + 2), QPoint(rm.right() + 1, r2.bottom() - 2));
        const QByteArray xml = win.xml();
        QVERIFY2(bordersOf(xml, "M").contains("<w:right w:val=\"single\""), qPrintable(bordersOf(xml, "M")));
        QVERIFY2(bordersOf(xml, "x1").contains("<w:left w:val=\"single\""), qPrintable(bordersOf(xml, "x1")));
        QVERIFY(bordersOf(xml, "x2").contains("<w:left w:val=\"single\""));
        QVERIFY(bordersOf(xml, "x3").isEmpty());
        QCOMPARE(int(xml.count("<w:tcBorders>")), 3);
    }
    {
        // One row of three (less than half): the joined cell has nothing, the cell beside the row has its left.
        TailWin win(mergedTableBody());
        setup(win, &cells, &m, &p1, &p2, &p3);
        const QRect rm = cells.value(m), r1 = cells.value(p1);
        win.w.act(QStringLiteral("table.paintBorder"))->trigger();
        drag(win.w.view()->viewport(), QPoint(rm.right() + 1, r1.top() + 2), QPoint(rm.right() + 1, r1.bottom() - 2));
        const QByteArray xml = win.xml();
        QVERIFY2(bordersOf(xml, "M").isEmpty(), qPrintable(bordersOf(xml, "M")));
        QVERIFY2(bordersOf(xml, "x1").contains("<w:left w:val=\"single\""), qPrintable(bordersOf(xml, "x1")));
        QCOMPARE(int(xml.count("<w:tcBorders>")), 1);
    }
    {
        // Every side is red, then the first row's part of the line between the columns is erased: the line comes off the cell beside it and stays on the joined cell, and is still drawn there.
        TailWin win(mergedTableBody());
        QTextCursor all = win.w.view()->cursor();
        selectInTable(*win.w.document(), all, TableUnit::Table);
        setBorders(*win.w.document(), all, BorderSide::All, redPen());
        win.w.view()->refresh();
        setup(win, &cells, &m, &p1, &p2, &p3);
        const QRect rm = cells.value(m), r1 = cells.value(p1), r3 = cells.value(p3);
        const QPoint corner = win.w.view()->pageRect(0).topLeft();
        const int x = rm.right() + 1;
        // Whether red is drawn at the line between the columns, at the height of the row.
        auto lineAt = [&](const QRect &row) {
            win.w.view()->refresh();
            const int y = row.center().y() - corner.y();
            return redColumns(pageAsInView(win.w.view()), y - 2, y + 2, x - corner.x() - 5, x - corner.x() + 6).first >= 0;
        };
        QVERIFY(lineAt(r1) && lineAt(r3));
        win.w.act(QStringLiteral("table.paintBorder"))->trigger();
        win.w.view()->setBorderPen(flow::TableLine());
        drag(win.w.view()->viewport(), QPoint(x, r1.top() + 2), QPoint(x, r1.bottom() - 2));
        const QByteArray xml = win.xml();
        QVERIFY2(bordersOf(xml, "x1").contains("<w:left w:val=\"nil\"/>"), qPrintable(bordersOf(xml, "x1")));
        QVERIFY2(bordersOf(xml, "M").contains("<w:right w:val=\"single\""), qPrintable(bordersOf(xml, "M")));   // (the joined cell keeps its right side)
        QVERIFY2(bordersOf(xml, "x2").contains("<w:left w:val=\"single\""), qPrintable(bordersOf(xml, "x2")));
        QVERIFY2(lineAt(r1), "the line the joined cell keeps is drawn in the first row still");
        // Erased along all of it, every one of them comes off.
        drag(win.w.view()->viewport(), QPoint(x, rm.top() + 2), QPoint(x, rm.bottom() - 2));
        const QByteArray gone = win.xml();
        QVERIFY2(bordersOf(gone, "M").contains("<w:right w:val=\"nil\"/>") && bordersOf(gone, "x2").contains("<w:left w:val=\"nil\"/>") && bordersOf(gone, "x3").contains("<w:left w:val=\"nil\"/>"), qPrintable(bordersOf(gone, "M")));
        QVERIFY(!lineAt(r1) && !lineAt(r3));
    }
}

// The cell that is split across pages: its end on a page that it goes on from is where the page ends, not a side of the cell; the top of a part that goes on from the page before is not its top.
void WriteTests::aCellSplitAcrossAPageGetsNoSideAtTheFootAndNoneAtTheTopOfTheNextPart()
{
    QByteArray lines;
    for (int i = 0; i < 130; ++i) lines += "<w:p><w:r><w:t>line</w:t></w:r></w:p>";
    TailWin tall("<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"4000\"/></w:tblGrid><w:tr><w:tc>" + lines + "</w:tc></w:tr></w:tbl><w:p><w:r><w:t>End</w:t></w:r></w:p>");
    tall.w.view()->paginateAll();
    PageView *view = tall.w.view();
    QWidget *vp = view->viewport();
    const QString story = tall.w.document()->story()->toPlainText();
    // The cell's box on the page that has the paragraph `nth` (0 based) of the lines.
    auto boxAt = [&](int nth) {
        int at = -1;
        for (int i = 0; i <= nth; ++i) at = story.indexOf(QStringLiteral("line"), at + 1);
        QTextCursor c(tall.w.document()->story());
        c.setPosition(at);
        view->setCursor(c);
        tall.w.refreshForTest();
        const QHash<int, QRect> cells = tall.cells();
        return cells.size() == 1 ? cells.constBegin().value() : QRect();
    };
    tall.w.act(QStringLiteral("table.paintBorder"))->trigger();
    QSignalSpy said(view, &PageView::notice);
    // `side` of the box (0 top, 1 bottom) dragged along; what it painted ("top", "bottom", or "" for nothing), then taken back.
    auto along = [&](const QRect &box, int side) {
        const quint64 serial = tall.w.document()->nextStepSerial();
        const int y = side == 0 ? box.top() : box.bottom();
        drag(vp, QPoint(box.left() + 10, y), QPoint(box.right() - 10, y));
        const bool painted = tall.w.document()->nextStepSerial() != serial;
        if (painted) tall.w.document()->story()->undo();
        return painted ? (side == 0 ? QByteArray("top") : QByteArray("bottom")) : QByteArray();
    };
    // The first part: its top is the cell's, its bottom is the page's foot.
    const QRect first = boxAt(2);
    QVERIFY(!first.isNull());
    QCOMPARE(along(first, 0), QByteArray("top"));
    said.clear();
    QCOMPARE(along(first, 1), QByteArray());
    QCOMPARE(said.size(), 1);   // (it is a drag along no line)
    // A part in the middle has neither.
    const QRect middle = boxAt(60);
    QVERIFY(!middle.isNull());
    QCOMPARE(along(middle, 0), QByteArray());
    QCOMPARE(along(middle, 1), QByteArray());
    // The last part: not its top, but its bottom is the cell's own.
    const QRect last = boxAt(125);
    QVERIFY(!last.isNull());
    QCOMPARE(along(last, 0), QByteArray());
    QCOMPARE(along(last, 1), QByteArray("bottom"));
    QVERIFY(!tall.xml().contains("<w:tcBorders>"));
}

// The geometry alone: a cell that goes on below the page's foot has no bottom, one that goes on from the page above has no top; the other sides are found.
void WriteTests::theGeometryGivesNoSideToTheEndOfACellThatContinues()
{
    using namespace jp::flow;
    auto sides = [](bool continued, bool continues, const std::vector<QPointF> &path) {
        PaintCell cell{QRectF(0, 0, 100, 50), 7, continued, continues};
        return sidesUnderPath({cell}, path, 3);
    };
    const std::vector<QPointF> bottom = {{2, 50}, {98, 50}}, top = {{2, 0}, {98, 0}};
    QCOMPARE(int(sides(false, false, bottom).size()), 1);
    QCOMPARE(int(sides(false, true, bottom).size()), 0);
    QCOMPARE(int(sides(true, false, bottom).size()), 1);
    QCOMPARE(int(sides(false, false, top).size()), 1);
    QCOMPARE(int(sides(true, false, top).size()), 0);
    QCOMPARE(int(sides(false, true, top).size()), 1);
    // A part that has both (a middle part) has neither, and still has its left.
    QCOMPARE(int(sides(true, true, top).size() + sides(true, true, bottom).size()), 0);
    QCOMPARE(int(sides(true, true, {{0, 2}, {0, 48}}).size()), 1);
}

void WriteTests::picturesOfTheBorderPainterTheMoveHandleAndShowNotes()
{
    const QString dir = qEnvironmentVariable("JW_TAIL_SHOTS");
    if (dir.isEmpty()) QSKIP("JW_TAIL_SHOTS is not set");
    QDir().mkpath(dir);
    jp::applyUiTheme(1);
    // (a table of three rows by three columns, each row 45 points tall, so that a drag along a line has some length to be seen)
    QByteArray rows;
    for (const char *row : {"abc", "def", "ghi"}) {
        rows += "<w:tr><w:trPr><w:trHeight w:val=\"900\"/></w:trPr>";
        for (const char *ch = row; *ch; ++ch) rows += cellXml(QByteArray(1, *ch).constData());
        rows += "</w:tr>";
    }
    TailWin win("<w:p><w:r><w:t>Before</w:t></w:r></w:p><w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2400\"/><w:gridCol w:w=\"2400\"/><w:gridCol w:w=\"2400\"/></w:tblGrid>" + rows +
                "</w:tbl><w:p><w:r><w:t>After one</w:t></w:r></w:p><w:p><w:r><w:t>After two</w:t></w:r></w:p>");
    win.w.resize(1200, 820);
    {
        // (the table has visible borders, so that the painting and the move can be seen against them)
        QTextCursor all = win.w.view()->cursor();
        selectInTable(*win.w.document(), all, TableUnit::Table);
        setBorders(*win.w.document(), all, BorderSide::All, flow::TableLine{flow::TableLine::Style::Single, 0.75, QColor(0x55, 0x55, 0x55)});
        win.w.view()->refresh();
    }
    win.w.view()->paginateAll();
    win.w.view()->setCaretWithoutFocus(true);
    win.w.ribbon()->showTab(win.w.ribbon()->tab(QStringLiteral("Table Design")));
    const QHash<int, QRect> cells = win.cells();
    const QRect ra = cells.value(positionOf(*win.w.document(), QStringLiteral("a")));
    win.w.act(QStringLiteral("table.paintBorder"))->trigger();
    win.w.view()->setBorderPen(flow::TableLine{flow::TableLine::Style::Double, 2.25, QColor(0xC0, 0x39, 0x2B)});
    QWidget *vp = win.w.view()->viewport();
    const int x = ra.right() + 1;
    const QRect rg = cells.value(positionOf(*win.w.document(), QStringLiteral("g")));
    const int top = ra.top() + 4, bottom = rg.bottom() - 4;   // (the line between the first and the second column, down the table's three rows)
    QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, QPoint(x, top));
    for (int i = 1; i <= 6; ++i) QTest::mouseMove(vp, QPoint(x, top + (bottom - top) * i / 10));
    win.w.grab().save(QDir(dir).filePath(QStringLiteral("border-painter-dragging.png")));
    for (int i = 7; i <= 10; ++i) QTest::mouseMove(vp, QPoint(x, top + (bottom - top) * i / 10));
    QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, QPoint(x, bottom));
    QTest::keyClick(vp, Qt::Key_Escape);
    win.w.view()->refresh();
    win.w.grab().save(QDir(dir).filePath(QStringLiteral("border-painter-done.png")));
    // The table pulled by its handle, with the line where it would go.
    QRect handle;
    QVERIFY(win.w.view()->moveHandleInView(&handle));
    QRect where;
    QVERIFY(win.w.view()->positionRect(positionOf(*win.w.document(), QStringLiteral("After two")), &where));
    QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, handle.center());
    for (int i = 1; i <= 6; ++i) QTest::mouseMove(vp, handle.center() + (where.center() - handle.center()) * i / 6);
    win.w.grab().save(QDir(dir).filePath(QStringLiteral("move-handle-pulling.png")));
    QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, where.center());
    win.w.grab().save(QDir(dir).filePath(QStringLiteral("move-handle-done.png")));
    // Show Notes: from a mark into its note.
    WriteWindow notes;
    notes.resize(1200, 820);
    notes.show();
    notes.setDocument(open(threeNotesDocx()), QString());
    notes.view()->paginateAll();
    notes.view()->setCaretWithoutFocus(true);
    notes.ribbon()->showTab(notes.ribbon()->tab(QStringLiteral("References")));
    QTextCursor c(notes.document()->story());
    c.setPosition(notes.document()->footnoteReferences(3).first() + 1);
    notes.view()->setCursor(c);
    notes.refreshForTest();
    notes.act(QStringLiteral("refs.showNotes"))->trigger();
    notes.grab().save(QDir(dir).filePath(QStringLiteral("show-notes.png")));
}
