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
void WriteTests::picturesOfTheBorderPainterTheMoveHandleAndShowNotes()
{
    const QString dir = qEnvironmentVariable("JW_TAIL_SHOTS");
    if (dir.isEmpty()) QSKIP("JW_TAIL_SHOTS is not set");
    QDir().mkpath(dir);
    jp::applyUiTheme(1);
    TailWin win;
    win.w.resize(1200, 820);
    win.w.view()->paginateAll();
    win.w.view()->setCaretWithoutFocus(true);
    win.w.ribbon()->showTab(win.w.ribbon()->tab(QStringLiteral("Table Design")));
    const QHash<int, QRect> cells = win.cells();
    const QRect ra = cells.value(positionOf(*win.w.document(), QStringLiteral("a")));
    win.w.act(QStringLiteral("table.paintBorder"))->trigger();
    win.w.view()->setBorderPen(flow::TableLine{flow::TableLine::Style::Double, 2.25, QColor(0xC0, 0x39, 0x2B)});
    QWidget *vp = win.w.view()->viewport();
    const int x = ra.right() + 1;
    QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, QPoint(x, ra.top() + 4));
    for (int i = 1; i <= 5; ++i) QTest::mouseMove(vp, QPoint(x, ra.top() + 4 + (ra.height() - 8) * i / 10));
    win.w.grab().save(QDir(dir).filePath(QStringLiteral("border-painter-dragging.png")));
    QTest::mouseMove(vp, QPoint(x, ra.bottom() - 4));
    QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, QPoint(x, ra.bottom() - 4));
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
