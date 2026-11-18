// Tests of the text box drawn with the mouse and of the way its words run (JeffDeck): the box the drag makes, the tools that put it down, Text Direction (the three ways, one step each), the
// direction in a .pptx and an .odp, a value in a file that this program does not know, and a click in turned words.
#include "../isolatesettings.h"
#include "deckhelpers.h"
#include "io/presentationfile.h"
#include "io/zip.h"

#include "app/theme.h"

#include <QDir>
#include <QKeyEvent>
#include <QTemporaryDir>

using namespace jp;

namespace {

std::unique_ptr<Document> oneSlide()
{
    auto d = deck::newPresentation();
    deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("blank")));
    return d;
}

QPoint viewOf(DeckWindow &w, QPointF page) { return w.canvas()->pageToView(page).toPoint(); }

void drag(DeckWindow &w, QPointF from, QPointF to)
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

class DeckTextBoxTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void aDragDrawsATextBoxOfTheDraggedSizeInOneStep()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        QVERIFY(w.act(QStringLiteral("insert.textBox")));
        w.act(QStringLiteral("insert.textBox"))->trigger();
        QCOMPARE(ed->tool(), Tool::Text);
        const int steps = ed->undoStack()->count();
        drag(w, QPointF(100, 100), QPointF(300, 220));
        ed->endTextEdit();
        TextItem *box = textBoxOf(ed);
        QVERIFY(box);
        QVERIFY2(std::abs(box->rect.left() - 100) < 3 && std::abs(box->rect.top() - 100) < 3 && std::abs(box->rect.width() - 200) < 3 && std::abs(box->rect.height() - 120) < 3, qPrintable(QStringLiteral("%1,%2 %3x%4").arg(box->rect.left()).arg(box->rect.top()).arg(box->rect.width()).arg(box->rect.height())));
        QCOMPARE(ed->tool(), Tool::Select);
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        ed->undoStack()->undo();
        QVERIFY(!textBoxOf(ed));
    }

    void aSmallDragGivesTheUsualSizeAndAnyOtherDragHasSidesOfAtLeast18Points()
    {
        DeckWindow w;
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
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        w.act(QStringLiteral("insert.textBox"))->trigger();
        QCOMPARE(ed->tool(), Tool::Text);
        QCOMPARE(w.canvas()->viewport()->cursor().shape(), Qt::CrossCursor);
        w.canvas()->setFocus();
        QTest::keyClick(w.canvas(), Qt::Key_Escape);
        QCOMPARE(ed->tool(), Tool::Select);
        QVERIFY(!textBoxOf(ed));
        w.act(QStringLiteral("insert.textBox"))->trigger();
        ed->setTool(Tool::Table);
        QVERIFY(ed->tool() != Tool::Text);
        drag(w, QPointF(100, 100), QPointF(300, 220));
        QVERIFY(!textBoxOf(ed));   // (the box tool was put down: this drag made a table)
    }

    void textDirectionHasThreeChoicesAndEachIsOneStepThatUndoes()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        ed->addItem(ed->newTextBox(QRectF(100, 100, 200, 120), QStringLiteral("Hello turned")));
        TextItem *box = textBoxOf(ed);
        QVERIFY(box);
        const QString id = box->id;
        for (const char *name : {"tb.dirHorizontal", "tb.dirVertical", "tb.dirRotate270"}) QVERIFY2(w.act(QString::fromLatin1(name)), name);
        auto now = [&] { return static_cast<TextItem *>(ed->doc()->item(id)); };
        const int steps = ed->undoStack()->count();
        QTRY_VERIFY(w.act(QStringLiteral("tb.dirVertical"))->isEnabled());   // (the ribbon catches up with the selection)
        w.act(QStringLiteral("tb.dirVertical"))->trigger();
        QVERIFY(now()->vertical && !now()->turnedBack);
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QTRY_VERIFY(w.act(QStringLiteral("tb.dirVertical"))->isChecked());
        w.act(QStringLiteral("tb.dirRotate270"))->trigger();
        QVERIFY(now()->vertical && now()->turnedBack);
        QCOMPARE(ed->undoStack()->count(), steps + 2);
        QTRY_VERIFY(w.act(QStringLiteral("tb.dirRotate270"))->isChecked());
        QTRY_VERIFY(!w.act(QStringLiteral("tb.dirVertical"))->isChecked());
        ed->undoStack()->undo();
        QVERIFY(now()->vertical && !now()->turnedBack);   // undo gives the old direction back
        ed->undoStack()->undo();
        QVERIFY(!now()->vertical);
    }

    void pickingADirectionDropsAVertValueThisProgramDoesNotKnow()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        ed->addItem(ed->newTextBox(QRectF(100, 100, 200, 120), QStringLiteral("Hello turned")));
        const QString id = textBoxOf(ed)->id;
        ed->change(QStringLiteral("Test"), [&] { textBoxOf(ed)->keptVert = QStringLiteral("wordArtVert"); });
        ed->select(id);
        QTRY_VERIFY(w.act(QStringLiteral("tb.dirVertical"))->isEnabled());
        w.act(QStringLiteral("tb.dirVertical"))->trigger();
        w.act(QStringLiteral("tb.dirHorizontal"))->trigger();
        QVERIFY(!static_cast<TextItem *>(ed->doc()->item(id))->vertical);
        const QString file = dir.filePath(QStringLiteral("a.pptx"));
        QString err;
        QVERIFY2(deck::saveAnyPresentationFile(*ed->doc(), file, &err), qPrintable(err));
        QMap<QString, QByteArray> parts;
        QVERIFY(readZip(bytesOf(file), parts));
        const QByteArray slide = parts.value(QStringLiteral("ppt/slides/slide1.xml"));
        QVERIFY(!slide.contains("wordArtVert"));
        QVERIFY(!slide.contains("vert=\"vert"));
    }

    void aTurnedGrowToFitBoxGrowsInWidthAndKeepsItsHeight()
    {
        DeckWindow w;
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

    void aTurnedBoxIsSavedAndOpenedTurnedInPptxAndOdp()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        for (const bool back : {false, true}) {
            for (const char *ending : {"pptx", "odp"}) {
                DeckWindow w;
                w.editor()->setDocument(oneSlide());
                Editor *ed = w.editor();
                ed->addItem(ed->newTextBox(QRectF(100, 100, 200, 120), QStringLiteral("Hello turned")));
                TextItem *box = textBoxOf(ed);
                QVERIFY(box);
                ed->change(QStringLiteral("Text Direction"), [&] {
                    box->vertical = true;
                    box->turnedBack = back;
                });
                const QString file = dir.filePath(QStringLiteral("turned%1.%2").arg(int(back)).arg(QLatin1String(ending)));
                QString err;
                QVERIFY2(deck::saveAnyPresentationFile(*ed->doc(), file, &err), qPrintable(err));
                auto again = deck::importAnyPresentationFile(file, &err);
                QVERIFY2(again, qPrintable(err));
                const TextItem *got = firstTextBox(*again);
                QVERIFY2(got, ending);
                QVERIFY2(got->vertical, ending);
                QCOMPARE(got->turnedBack, back);
            }
        }
        // (the .odp says it with the writing mode: 90 is the standard value, 270 the extension's)
        DeckWindow w;
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        ed->addItem(ed->newTextBox(QRectF(100, 100, 200, 120), QStringLiteral("Hello turned")));
        TextItem *box = textBoxOf(ed);
        ed->change(QStringLiteral("Text Direction"), [&] {
            box->vertical = true;
            box->turnedBack = true;
        });
        const QString odp = dir.filePath(QStringLiteral("mode.odp"));
        QVERIFY(deck::saveAnyPresentationFile(*ed->doc(), odp));
        QMap<QString, QByteArray> parts;
        QVERIFY(readZip(bytesOf(odp), parts));
        QVERIFY(parts.value(QStringLiteral("content.xml")).contains("loext:writing-mode=\"bt-lr\""));
        QVERIFY(!parts.value(QStringLiteral("content.xml")).contains("jo:turned-back"));
    }

    void aVertValueThisProgramDoesNotKnowReadsAcrossAndIsKeptUntilTheDirectionChanges()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto base = oneSlide();
        DeckWindow w;
        w.editor()->setDocument(std::move(base));
        Editor *ed = w.editor();
        ed->addItem(ed->newTextBox(QRectF(100, 100, 200, 120), QStringLiteral("Hello turned")));
        ed->change(QStringLiteral("Text Direction"), [&] { textBoxOf(ed)->vertical = true; });
        const QString file = dir.filePath(QStringLiteral("a.pptx"));
        QString err;
        QVERIFY2(deck::saveAnyPresentationFile(*ed->doc(), file, &err), qPrintable(err));
        QMap<QString, QByteArray> parts;
        QVERIFY(readZip(bytesOf(file), parts));
        const QString slide = QStringLiteral("ppt/slides/slide1.xml");
        QVERIFY(parts.value(slide).contains("vert=\"vert\""));
        auto withVert = [&](const QByteArray &value) {
            ZipWriter zip;
            for (auto it = parts.cbegin(); it != parts.cend(); ++it) {
                QByteArray data = it.value();
                if (it.key() == slide) data.replace("vert=\"vert\"", "vert=\"" + value + "\"");
                zip.add(it.key(), data, true);
            }
            const QString path = dir.filePath(QStringLiteral("b%1.pptx").arg(value.size()));
            QFile f(path);
            if (!f.open(QIODevice::WriteOnly)) return QString();
            f.write(zip.finish());
            return path;
        };
        for (const QByteArray &value : {QByteArray("wordArtVert"), QByteArray("junk"), QByteArray(100000, 'x')}) {
            auto doc = deck::importAnyPresentationFile(withVert(value), &err);
            QVERIFY2(doc, qPrintable(err));
            TextItem *box = firstTextBox(*doc);
            QVERIFY(box);
            QVERIFY2(!box->vertical, value.constData());   // (read across)
            const QString out = dir.filePath(QStringLiteral("c.pptx"));
            QVERIFY2(deck::saveAnyPresentationFile(*doc, out, &err), qPrintable(err));
            QMap<QString, QByteArray> saved;
            QVERIFY(readZip(bytesOf(out), saved));
            if (value.size() < 100) QVERIFY2(saved.value(slide).contains("vert=\"" + value + "\""), value.constData());   // (kept as it was)
            else QVERIFY(saved.value(slide).size() < 50000);   // (a value that long is not kept)
            // (set to a direction of ours, it is written as that)
            box->vertical = true;
            QVERIFY(deck::saveAnyPresentationFile(*doc, out, &err));
            QVERIFY(readZip(bytesOf(out), saved));
            QVERIFY(saved.value(slide).contains("vert=\"vert\""));
        }
    }

    void aClickNearTheStartOfTurnedWordsPutsTheCaretAtTheStart()
    {
        DeckWindow w;
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
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(oneSlide());
        Editor *ed = w.editor();
        w.act(QStringLiteral("insert.textBox"))->trigger();
        drag(w, QPointF(120, 100), QPointF(200, 330));
        ed->commitText(QStringLiteral("Words that run down the box"));
        ed->endTextEdit();
        ed->select(textBoxOf(ed)->id);
        QTRY_VERIFY(w.act(QStringLiteral("tb.dirVertical"))->isEnabled());
        w.act(QStringLiteral("tb.dirVertical"))->trigger();
        QApplication::processEvents();
        QVERIFY(w.grab().save(dir + QStringLiteral("/deck-turned-text-box.png")));
    }
};

JP_TEST_CLASS(DeckTextBoxTests)
#include "tst_deck_textbox.moc"
