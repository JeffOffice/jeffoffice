// Tests of the text box drawn with the mouse and of the way its words run (JeffPub): the box the drag makes, the tools that put it down, Text Direction (the three ways, one step each), the
// direction in a .pub file, and a click in turned words.
#include "../isolatesettings.h"
#include "pubhelpers.h"

#include "app/theme.h"

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
            QTest::mouseClick(w.canvas()->viewport(), Qt::LeftButton, Qt::NoModifier, viewOf(w, at(first + 0.3)));
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
};

JP_TEST_CLASS(PubTextBoxTests)
#include "tst_pub_textbox.moc"
