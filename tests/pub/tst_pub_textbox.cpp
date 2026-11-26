// Tests of the text box drawn with the mouse and of the way its words run (JeffPub): the box the drag makes, the tools that put it down, Text Direction (the three ways, one step each), the
// direction in a .pub file, and a click in turned words.
#include "../isolatesettings.h"
#include "pubhelpers.h"

#include "app/theme.h"
#include "app/highlightmenu.h"
#include <QMenu>
#include <QStatusBar>
#include "app/lossywarning.h"
#include <QScopeGuard>
#include "io/zip.h"
#include "render/renderer.h"

#include <QDir>
#include <QKeyEvent>
#include <QTemporaryDir>

using namespace jp;

namespace {

std::unique_ptr<Document> oneSlide() { return Document::blank(QSizeF(612, 792)); }

QPoint viewOf(MainWindow &w, QPointF page) { return w.canvas()->pageToView(page).toPoint(); }

void drag(MainWindow &w, QPointF from, QPointF to)
{
    QWidget *vp = w.canvas()->viewport();
    QTest::qWait(QApplication::doubleClickInterval() + 50);
    QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, viewOf(w, from));
    for (const QPointF at : {(from + to) / 2, to}) {
        const QPoint v = viewOf(w, at);
        QMouseEvent mv(QEvent::MouseMove, v, vp->mapToGlobal(v), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(vp, &mv);
    }
    QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, viewOf(w, to));
}

TextItem *textBoxOf(Editor *ed)
{
    for (const ItemPtr &it : ed->surfaceItems())
        if (it->type() == ItemType::Text && it->role.isEmpty()) return static_cast<TextItem *>(it.get());
    return nullptr;
}

TextItem *firstTextBox(const Document &d)
{
    for (const ItemPtr &it : d.pages[0]->items)
        if (it->type() == ItemType::Text && it->role.isEmpty()) return static_cast<TextItem *>(it.get());
    return nullptr;
}

QByteArray bytesOf(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

// A box of "AAAA BBBB" with the first word highlighted yellow, in a window of its own.
std::shared_ptr<TextItem> markedBox(MainWindow &w)
{
    Editor *ed = w.editor();
    auto box = std::static_pointer_cast<TextItem>(ed->newTextBox(QRectF(72, 72, 400, 200), QStringLiteral("AAAA BBBB")));
    ed->addItem(box);
    ed->beginTextEdit(box->id, 0);
    QTextCursor c = ed->cursor();
    c.setPosition(0);
    c.setPosition(4, QTextCursor::KeepAnchor);
    ed->setCursor(c);
    ed->chooseHighlight(ColorRef::rgb(QColor(Qt::yellow)));
    ed->endTextEdit();
    return box;
}

} // namespace

class PubTextBoxTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void aDragDrawsATextBoxOfTheDraggedSizeInOneStep()
    {
        MainWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        QVERIFY(w.act(QStringLiteral("ins.textbox")));
        w.act(QStringLiteral("ins.textbox"))->trigger();
        QCOMPARE(ed->tool(), Tool::Text);
        const int steps = ed->undoStack()->count();
        drag(w, QPointF(100, 100), QPointF(300, 220));
        ed->endTextEdit();
        TextItem *box = textBoxOf(ed);
        QVERIFY(box);
        QVERIFY2(std::abs(box->rect.left() - 100) < 8 && std::abs(box->rect.top() - 100) < 8 && std::abs(box->rect.width() - 200) < 8 && std::abs(box->rect.height() - 120) < 8, qPrintable(QStringLiteral("%1,%2 %3x%4").arg(box->rect.left()).arg(box->rect.top()).arg(box->rect.width()).arg(box->rect.height())));
        QCOMPARE(ed->tool(), Tool::Select);
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        ed->undoStack()->undo();
        QVERIFY(!textBoxOf(ed));
    }

    void aSmallDragGivesTheUsualSizeAndAnyOtherDragHasSidesOfAtLeast18Points()
    {
        MainWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        // (a click, and a drag of under 4 points: the usual size, from where the button went down)
        ed->setTool(Tool::Text);
        drag(w, QPointF(100, 100), QPointF(102, 101));
        ed->endTextEdit();
        TextItem *box = textBoxOf(ed);
        QVERIFY(box);
        QVERIFY2(std::abs(box->rect.width() - 144) < 3 && std::abs(box->rect.height() - 72) < 3, qPrintable(QStringLiteral("%1x%2").arg(box->rect.width()).arg(box->rect.height())));
        ed->undoStack()->undo();
        // (a drag of 10 points: a box that can hold a line)
        ed->setTool(Tool::Text);
        drag(w, QPointF(100, 100), QPointF(110, 110));
        ed->endTextEdit();
        box = textBoxOf(ed);
        QVERIFY(box);
        QVERIFY2(std::abs(box->rect.width() - 18) < 3 && std::abs(box->rect.height() - 18) < 3, qPrintable(QStringLiteral("%1x%2").arg(box->rect.width()).arg(box->rect.height())));
        QVERIFY(std::abs(box->rect.left() - 100) < 3);
    }

    // A document that becomes read-only ends the text edit: a key typed after is refused, and the next format command finds no caret (it once used a cursor into
    // text that the refused edit had rolled back).
    void aDocumentThatBecomesReadOnlyEndsTheTextEdit()
    {
        MainWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        ed->setDocument(oneSlide());
        auto box = std::static_pointer_cast<TextItem>(ed->newTextBox(QRectF(72, 72, 400, 200), QStringLiteral("alpha")));
        ed->addItem(box);
        ed->beginTextEdit(box->id, 0);
        QVERIFY(ed->isEditingText());
        ed->setReadOnly(true);
        QVERIFY(!ed->isEditingText());
        ed->typeText(QStringLiteral("zz"));
        ed->toggleBold();
        ed->setFontSize(30);
        QCOMPARE(ed->doc()->storyDoc(box->storyId)->toPlainText(), QStringLiteral("alpha"));
        QVERIFY(!ed->isEditingText());
        ed->setReadOnly(false);
    }

    void escapeAndAnotherToolPutTheTextBoxToolDown()
    {
        MainWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        w.act(QStringLiteral("ins.textbox"))->trigger();
        QCOMPARE(ed->tool(), Tool::Text);
        QCOMPARE(w.canvas()->viewport()->cursor().shape(), Qt::CrossCursor);
        w.canvas()->setFocus();
        QTest::keyClick(w.canvas(), Qt::Key_Escape);
        QCOMPARE(ed->tool(), Tool::Select);
        QVERIFY(!textBoxOf(ed));
        w.act(QStringLiteral("ins.textbox"))->trigger();
        ed->setTool(Tool::Table);
        QVERIFY(ed->tool() != Tool::Text);
        drag(w, QPointF(100, 100), QPointF(300, 220));
        QVERIFY(!textBoxOf(ed));   // (the box tool was put down: this drag made a table)
    }

    void textDirectionHasThreeChoicesAndEachIsOneStepThatUndoes()
    {
        MainWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        ed->addItem(ed->newTextBox(QRectF(100, 100, 200, 120), QStringLiteral("Hello turned")));
        TextItem *box = textBoxOf(ed);
        QVERIFY(box);
        const QString id = box->id;
        for (const char *name : {"tb.dir.horz", "tb.dir.r90", "tb.dir.r270"}) QVERIFY2(w.act(QString::fromLatin1(name)), name);
        auto now = [&] { return static_cast<TextItem *>(ed->doc()->item(id)); };
        const int steps = ed->undoStack()->count();
        w.act(QStringLiteral("tb.dir.r90"))->trigger();
        QVERIFY(now()->vertical && !now()->turnedBack);
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QTRY_VERIFY(w.act(QStringLiteral("tb.dir.r90"))->isChecked());
        w.act(QStringLiteral("tb.dir.r270"))->trigger();
        QVERIFY(now()->vertical && now()->turnedBack);
        QCOMPARE(ed->undoStack()->count(), steps + 2);
        QTRY_VERIFY(w.act(QStringLiteral("tb.dir.r270"))->isChecked());
        QTRY_VERIFY(!w.act(QStringLiteral("tb.dir.r90"))->isChecked());
        ed->undoStack()->undo();
        QVERIFY(now()->vertical && !now()->turnedBack);   // undo gives the old direction back
        ed->undoStack()->undo();
        QVERIFY(!now()->vertical);
    }

    void pickingADirectionDropsAVertValueThisProgramDoesNotKnow()
    {
        MainWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        ed->addItem(ed->newTextBox(QRectF(100, 100, 200, 120), QStringLiteral("Hello turned")));
        const QString id = textBoxOf(ed)->id;
        ed->change(QStringLiteral("Test"), [&] { textBoxOf(ed)->keptVert = QStringLiteral("wordArtVert"); });
        ed->select(id);
        QTRY_VERIFY(w.act(QStringLiteral("tb.dir.r90"))->isEnabled());
        w.act(QStringLiteral("tb.dir.r90"))->trigger();
        w.act(QStringLiteral("tb.dir.horz"))->trigger();
        QVERIFY(!static_cast<TextItem *>(ed->doc()->item(id))->vertical);
        QVERIFY(static_cast<TextItem *>(ed->doc()->item(id))->keptVert.isEmpty());
    }

    void aTurnedGrowToFitBoxGrowsInWidthAndKeepsItsHeight()
    {
        MainWindow w;
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        const QString words = QStringLiteral("One two three four five six seven eight nine ten eleven twelve thirteen fourteen fifteen sixteen seventeen eighteen nineteen twenty");
        QStringList ids;
        for (const QString &text : {QStringLiteral("One"), words}) {
            ed->addItem(ed->newTextBox(QRectF(100, 100, 60, 300), text));
            ids << ed->surfaceItems().back()->id;
        }
        for (const bool back : {false, true}) {
            double widths[2] = {0, 0};
            for (int k = 0; k < 2; ++k) {
                ed->change(QStringLiteral("Test"), [&] {
                    auto *t = static_cast<TextItem *>(ed->doc()->item(ids[k]));
                    t->vertical = true;
                    t->turnedBack = back;
                    t->rect = QRectF(100, 100, 60, 300);
                    ed->setTextFit(*t, 2);
                });
                const TextItem *t = static_cast<TextItem *>(ed->doc()->item(ids[k]));
                widths[k] = t->rect.width();
                QCOMPARE(t->rect.height(), 300.0);
            }
            QVERIFY2(widths[1] > widths[0] + 5, qPrintable(QString::number(widths[0]) + " " + QString::number(widths[1])));
        }
    }

    void aTurnedBoxIsSavedAndOpenedTurnedInAPubFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        for (const bool back : {false, true}) {
            MainWindow w;
            w.editor()->setDocument(oneSlide());
            Editor *ed = w.editor();
            ed->addItem(ed->newTextBox(QRectF(100, 100, 60, 300), QStringLiteral("Hello turned")));
            TextItem *box = textBoxOf(ed);
            QVERIFY(box);
            ed->change(QStringLiteral("Text Direction"), [&] {
                box->vertical = true;
                box->turnedBack = back;
            });
            const QString file = dir.filePath(QStringLiteral("turned%1.pub").arg(int(back)));
            QString err;
            QVERIFY2(jp::exportPublisher(*ed->doc(), file, &err), qPrintable(err));
            auto again = jp::importPublisherFile(file, &err);
            QVERIFY2(again, qPrintable(err));
            const TextItem *got = nullptr;
            for (const ItemPtr &it : again->pages[0]->items)
                if (it->type() == ItemType::Text) got = static_cast<const TextItem *>(it.get());
            QVERIFY(got);
            QVERIFY(got->vertical);
            QCOMPARE(got->turnedBack, back);
            QVERIFY(std::abs(got->rotation) < 0.5);   // (read turned, not as a box turned half round)
        }
    }

    void aTurnedBackBoxThatIsItselfRotatedKeepsBothInAPubFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        MainWindow w;
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        ed->addItem(ed->newTextBox(QRectF(100, 100, 60, 300), QStringLiteral("Hello turned")));
        TextItem *box = textBoxOf(ed);
        QVERIFY(box);
        ed->change(QStringLiteral("Text Direction"), [&] {
            box->setDirection(TextDirection::Rotate270);
            box->rotation = 30;
        });
        QString err;
        std::unique_ptr<Document> doc;
        for (int round = 0; round < 2; ++round) {   // (and again: what is read is written as it was)
            const QString file = dir.filePath(QStringLiteral("r%1.pub").arg(round));
            QVERIFY2(jp::exportPublisher(round ? *doc : *ed->doc(), file, &err), qPrintable(err));
            doc = jp::importPublisherFile(file, &err);
            QVERIFY2(doc, qPrintable(err));
            const TextItem *got = firstTextBox(*doc);
            QVERIFY(got);
            QVERIFY(got->vertical);
            QVERIFY(got->turnedBack);
            QVERIFY2(std::abs(got->rotation - 30) < 0.5, qPrintable(QString::number(got->rotation)));
        }
    }

    void aClickNearTheStartOfTurnedWordsPutsTheCaretAtTheStart()
    {
        MainWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        ed->addItem(ed->newTextBox(QRectF(300, 100, 100, 300), QStringLiteral("Hello turned words")));
        const QString id = textBoxOf(ed)->id;
        for (const bool back : {false, true}) {
            ed->change(QStringLiteral("Text Direction"), [&] {
                auto *box = static_cast<TextItem *>(ed->doc()->item(id));
                box->vertical = true;
                box->turnedBack = back;
            });
            ed->beginTextEdit(id, 10);
            const QRectF r = ed->doc()->item(id)->rect;
            // (turned 90 degrees the words begin at the top of the right side, turned 270 at the bottom of the left; `along` is how far from that end, in the first line)
            auto at = [&](double along) { return back ? QPointF(r.left() + 12, r.bottom() - along) : QPointF(r.right() - 12, r.top() + along); };
            double first = 0;
            for (double along = 1; along < 40 && first == 0; along += 0.5) {
                const auto hit = w.canvas()->hitTest(w.canvas()->pageToView(at(along)));
                if (hit.kind == Canvas::HitKind::Item && hit.id == id && hit.textInterior) first = along;
            }
            QVERIFY2(first > 0, back ? "270" : "90");
            const QString label = QStringLiteral("turned back %1").arg(back);
            QTest::qWait(QApplication::doubleClickInterval() + 50);
            QTest::mouseClick(w.canvas()->viewport(), Qt::LeftButton, Qt::NoModifier, viewOf(w, at(first + 2)));
            QVERIFY2(ed->isEditingText(), qPrintable(label));
            QVERIFY2(ed->cursor().position() <= 1, qPrintable(label + QStringLiteral(": caret at %1").arg(ed->cursor().position())));
            // (and far along the line it is far along the words)
            QTest::qWait(QApplication::doubleClickInterval() + 50);
            QTest::mouseClick(w.canvas()->viewport(), Qt::LeftButton, Qt::NoModifier, viewOf(w, at(150)));
            QVERIFY2(ed->cursor().position() >= 10, qPrintable(label + QStringLiteral(": caret at %1").arg(ed->cursor().position())));
            ed->endTextEdit();
        }
    }

    // A picture of a drawn text box whose words run turned, for a person to look at (JO_SHOT_DIR names the folder; nothing is made without it).
    void aDrawnTurnedTextBoxScreenshot()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        QDir().mkpath(dir);
        MainWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        w.act(QStringLiteral("ins.textbox"))->trigger();
        drag(w, QPointF(120, 100), QPointF(200, 330));
        ed->commitText(QStringLiteral("Words that run down the box"));
        ed->endTextEdit();
        ed->select(textBoxOf(ed)->id);
        w.act(QStringLiteral("tb.dir.r90"))->trigger();
        QApplication::processEvents();
        QVERIFY(w.grab().save(dir + QStringLiteral("/pub-turned-text-box.png")));
    }

    // The Text Highlight Color button (the same command and menu as JeffDeck's): it marks the selected words in one step of Undo, the menu's No Color
    // takes the mark off, and with nothing selected a color of the menu arms the highlighter, which Stop Highlighting ends.
    void theHighlightButtonMarksSelectedWordsAndArmsTheHighlighter()
    {
        MainWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        w.act(QStringLiteral("ins.textbox"))->trigger();
        drag(w, QPointF(100, 100), QPointF(300, 220));
        TextItem *box = textBoxOf(ed);
        QVERIFY(box);
        const QString id = box->id, storyId = box->storyId;
        QTextCursor(ed->doc()->storyDoc(storyId)).insertText(QStringLiteral("AAAA BBBB"));
        auto story = [&] { return ed->doc()->storyDoc(storyId); };
        auto markOf = [&](int at) {
            QTextCursor c(story());
            c.setPosition(at + 1);
            return c.charFormat().stringProperty(tp::HighlightRefP);
        };
        auto pick = [&](int from, int to) {
            if (!ed->isEditingText()) ed->beginTextEdit(id, from);
            QTextCursor c = ed->cursor();
            c.setPosition(from);
            c.setPosition(to, QTextCursor::KeepAnchor);
            ed->setCursor(c);
        };
        pick(0, 4);
        QAction *button = w.act(QStringLiteral("fmt.highlight"));
        QVERIFY(button);
        const int steps = ed->undoStack()->count();
        button->trigger();
        QCOMPARE(markOf(0), ColorRef::rgb(QColor(Qt::yellow)).toString());
        QVERIFY(markOf(5).isEmpty());
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QMenu *menu = highlightMenu(&w, ed, w.act(QStringLiteral("fmt.stopHighlighting")));
        pick(0, 4);
        for (QAction *a : menu->actions())
            if (a->text() == QStringLiteral("No Color")) a->trigger();
        QVERIFY(markOf(0).isEmpty());
        pick(2, 2);
        for (QAction *a : menu->actions())
            if (a->text() == QStringLiteral("Green")) a->trigger();
        QVERIFY(ed->highlighterArmed());
        QVERIFY(markOf(0).isEmpty());
        w.act(QStringLiteral("fmt.stopHighlighting"))->trigger();
        QVERIFY(!ed->highlighterArmed());
        delete menu;
        ed->endTextEdit();
    }

    // A second click on the button turns the highlighter off, Esc does too wherever focus is in the window, and so does a changed view.
    void aSecondClickEscAndAChangedViewEndTheHighlighter()
    {
        MainWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        QAction *button = w.act(QStringLiteral("fmt.highlight"));
        button->trigger();
        QVERIFY(ed->highlighterArmed());
        QVERIFY(button->isChecked());
        button->trigger();
        QVERIFY(!ed->highlighterArmed());
        QVERIFY(!button->isChecked());
        button->trigger();
        QVERIFY(ed->highlighterArmed());
        QTest::keyClick(w.statusBar(), Qt::Key_Escape);   // (not the canvas)
        QVERIFY(!ed->highlighterArmed());
        QVERIFY(!button->isChecked());
        QVERIFY(!ed->doc()->masters.empty());
        button->trigger();
        QVERIFY(ed->highlighterArmed());
        ed->setMasterView(ed->doc()->masters[0]->id);
        QVERIFY(!ed->highlighterArmed());
        ed->setMasterView(QString());
    }

    // No Color also takes off a mark that a paragraph's own format carries (a file saved before the mark went on the words only).
    void noColorTakesOffAMarkOfTheParagraphFormatToo()
    {
        MainWindow w;
        Editor *ed = w.editor();
        auto box = markedBox(w);
        QTextDocument *story = ed->doc()->storyDoc(box->storyId);
        {
            QTextCursor bc(story);
            QTextCharFormat bf = bc.blockCharFormat();
            bf.setProperty(tp::HighlightRefP, ColorRef::rgb(QColor(Qt::green)).toString());
            bc.setBlockCharFormat(bf);
        }
        QVERIFY(story->begin().charFormat().hasProperty(tp::HighlightRefP));
        ed->beginTextEdit(box->id, 0);
        QTextCursor c = ed->cursor();
        c.select(QTextCursor::Document);
        ed->setCursor(c);
        ed->chooseHighlight(ColorRef());
        QVERIFY(!story->begin().charFormat().hasProperty(tp::HighlightRefP));
        QTextCursor w1(story);
        w1.setPosition(2);
        QVERIFY(!w1.charFormat().hasProperty(tp::HighlightRefP));
        ed->endTextEdit();
    }

    // A .pub file has no place for a highlight: the save warning and Check Compatibility name it, and only when some text has one. EPUB keeps it as a
    // background, and the drawn page and the SVG show it.
    void everyFormatKeepsTheHighlightOrSaysItIsLost()
    {
        MainWindow w;
        Editor *ed = w.editor();
        auto box = std::static_pointer_cast<TextItem>(ed->newTextBox(QRectF(72, 72, 400, 200), QStringLiteral("AAAA BBBB")));
        ed->addItem(box);
        auto namesHighlight = [&] {
            for (const QString &line : w.pubLosses())
                if (line.contains(QStringLiteral("highlight"))) return true;
            return false;
        };
        QVERIFY(!namesHighlight());
        markedBox(w);
        QVERIFY(namesHighlight());
        QTemporaryDir dir;
        // The save to .pub asks first, and the box names the highlight.
        QString said;
        jp::setLossyStandIn([&](const jp::LossyAsk &a, bool *) {
            said = a.lost.join(QLatin1Char('\n'));
            return jp::LossyChoice::SaveAnyway;
        });
        const auto reset = qScopeGuard([] { jp::setLossyStandIn(nullptr); });
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("marked.pub")), true));
        QVERIFY2(said.contains(QStringLiteral("text highlight colors couldn't be saved to .pub"), Qt::CaseInsensitive), qPrintable(said));
        // EPUB
        QString err;
        const QString epubPath = dir.filePath(QStringLiteral("book.epub"));
        QVERIFY2(w.exportEpubTo(epubPath, QStringLiteral("Book"), QString(), false, &err), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY(jp::readZip(bytesOf(epubPath), zip, &err));
        QString text;
        for (auto it = zip.cbegin(); it != zip.cend(); ++it)
            if (it.key().endsWith(QStringLiteral(".xhtml"))) text += QString::fromUtf8(it.value());
        QVERIFY2(text.contains(QStringLiteral("<span style=\"background-color: #ffff00\">AAAA</span>")), qPrintable(text.left(600)));
        // The page drawn (the PDF, the images, and the HTML page are made from it) and the SVG.
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = ed->doc();
        ctx.cache = &cache;
        const QImage img = Renderer::renderToImage(ctx, 0, 1.0);
        int yellow = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x) {
                const QColor c = img.pixelColor(x, y);
                if (c.red() > 200 && c.green() > 200 && c.blue() < 90) ++yellow;
            }
        QVERIFY(yellow > 100);
        const QString svgPath = dir.filePath(QStringLiteral("page.svg"));
        QVERIFY(w.exportSvgTo(svgPath, &err));
        QVERIFY(QString::fromUtf8(bytesOf(svgPath)).contains(QStringLiteral("#ffff00")));
    }

    // Copy Formatting carries a highlight, and Clear All Formatting takes it off.
    void copyFormattingCarriesAHighlightAndClearAllFormattingTakesItOff()
    {
        MainWindow w;
        Editor *ed = w.editor();
        auto box = markedBox(w);
        QTextDocument *story = ed->doc()->storyDoc(box->storyId);
        auto markOf = [&](int at) {
            QTextCursor c(story);
            c.setPosition(at + 1);
            return c.charFormat().stringProperty(tp::HighlightRefP);
        };
        const QString yellow = ColorRef::rgb(QColor(Qt::yellow)).toString();
        ed->beginTextEdit(box->id, 0);
        QTextCursor c = ed->cursor();
        c.setPosition(0);
        c.setPosition(4, QTextCursor::KeepAnchor);
        ed->setCursor(c);
        ed->copyFormatting();
        c.setPosition(5);
        c.setPosition(9, QTextCursor::KeepAnchor);
        ed->setCursor(c);
        QVERIFY(markOf(5).isEmpty());
        ed->pasteFormatting();
        QCOMPARE(markOf(5), yellow);
        c.setPosition(0);
        c.setPosition(9, QTextCursor::KeepAnchor);
        ed->setCursor(c);
        ed->clearFormatting();
        QVERIFY(markOf(0).isEmpty());
        QVERIFY(markOf(5).isEmpty());
        ed->endTextEdit();
    }
};

JP_TEST_CLASS(PubTextBoxTests)
#include "tst_pub_textbox.moc"
