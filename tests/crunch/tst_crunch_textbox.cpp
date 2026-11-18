// Tests of the text box drawn with the mouse in JeffCrunch and of the way its words run: the box a drag makes (the size rule every app shares, one step of Undo), the cross pointer, Esc and any
// other command putting the tool down, Text Direction (three choices, one step each), the direction in a saved workbook, a value this program does not know, and a click in turned words.
#include "../sheet/drawinghelpers.h"
#include "../testrunner.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "savedok.h"

#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/shapetextedit.h"
#include "core/fonts.h"
#include "core/sheetobjects.h"
#include "io/zip.h"

#include "app/theme.h"

#include <QApplication>
#include <QDir>
#include <QSignalSpy>
#include <QUndoStack>

using namespace crunchtest;
using namespace jp;
using namespace drawtest;

namespace {

QMap<QString, QByteArray> partsOf(const QString &path)
{
    QMap<QString, QByteArray> parts;
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) jp::readZip(f.readAll(), parts);
    return parts;
}

}

class CrunchTextBoxTests : public QObject {
    Q_OBJECT

    static void drag(EditWindow &e, const QPoint &from, const QPoint &to, Qt::KeyboardModifiers modifiers = Qt::NoModifier)
    {
        QWidget *vp = e.view()->viewport();
        QTest::mousePress(vp, Qt::LeftButton, modifiers, from);
        QTest::mouseMove(vp, (from + to) / 2);
        QTest::mouseMove(vp, to);
        QTest::mouseRelease(vp, Qt::LeftButton, modifiers, to);
    }
    // The object that is on the sheet (a change makes the object again and takes the old one away; Undo has the old one back).
    static int liveId(EditWindow &e)
    {
        const grid::ObjectLayer *layer = e.win.w.objects()->layer();
        for (int i = 0; i < layer->count(); ++i)
            if (!layer->isDeleted(layer->object(i).id)) return layer->object(i).id;
        return -1;
    }
    static QString elementOf(EditWindow &e, int id)
    {
        crunch::ObjectClip clip;
        QString why;
        return e.win.w.objects()->copyElement(0, id, &clip, &why) ? QString::fromUtf8(clip.elementXml) : QString();
    }
    // The editor over the picked shape (opened with F2 on the grid), or none.
    static ShapeTextEdit *editShape(EditWindow &e)
    {
        QTest::keyClick(e.view()->viewport(), Qt::Key_F2);
        return e.win.w.shapeEditor();
    }
    static bool open(EditWindow &e)
    {
        e.win.w.resize(1100, 700);
        if (!e.open(editBook()) || !e.settle()) return false;
        e.view()->setZoom(1.0);
        return true;
    }

private Q_SLOTS:
    void initTestCase()
    {
        setAppInfo(crunchAppInfo());
        jp::initCore();
    }

    void aDragDrawsATextBoxOfTheDraggedSizeInOneStep()
    {
        EditWindow e;
        QVERIFY(open(e));
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(e.win.w.act(QStringLiteral("insert.textBox.draw")));
        e.win.w.act(QStringLiteral("insert.textBox.draw"))->trigger();
        QVERIFY(o->layer()->boxArmed());
        const int steps = e.win.w.session()->undoStack()->count();
        const QPoint from = e.view()->cellRect(4, 2).center();
        drag(e, from, from + QPoint(200, 120));
        QVERIFY(e.settle());
        QCOMPARE(o->layer()->count(), 1);
        QVERIFY(!o->layer()->boxArmed());
        const grid::ObjectBox box = o->layer()->boxNow(o->layer()->selectedId());
        QVERIFY2(std::abs(box.w - 200) < 3 && std::abs(box.h - 120) < 3, qPrintable(QStringLiteral("%1x%2").arg(box.w).arg(box.h)));
        QVERIFY(elementOf(e, liveId(e)).contains(QStringLiteral("txBox=\"1\"")));
        QCOMPARE(e.win.w.session()->undoStack()->count(), steps + 1);
        e.win.w.session()->undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(o->layer()->count() - int(o->layer()->isDeleted(0)), 0);
    }

    void aSmallDragGivesTheUsualSizeAndAnyOtherDragHasSidesOfAtLeast18Points()
    {
        EditWindow e;
        QVERIFY(open(e));
        crunch::SheetObjects *o = e.win.w.objects();
        const QPoint from = e.view()->cellRect(4, 2).center();
        // (a click, and a drag of under 4 points: the usual size every app has, 144 by 72 points, 192 by 96 pixels at 100%)
        e.win.w.act(QStringLiteral("insert.textBox.draw"))->trigger();
        drag(e, from, from + QPoint(2, 1));
        QVERIFY(e.settle());
        grid::ObjectBox box = o->layer()->boxNow(o->layer()->selectedId());
        QVERIFY2(std::abs(box.w - 192) < 3 && std::abs(box.h - 96) < 3, qPrintable(QStringLiteral("%1x%2").arg(box.w).arg(box.h)));
        // (a drag of 10 points, 13 pixels: a box that can hold a line, 18 points or 24 pixels a side)
        e.win.w.act(QStringLiteral("insert.textBox.draw"))->trigger();
        drag(e, from + QPoint(400, 0), from + QPoint(413, 13));
        QVERIFY(e.settle());
        box = o->layer()->boxNow(o->layer()->selectedId());
        QVERIFY2(std::abs(box.w - 24) < 3 && std::abs(box.h - 24) < 3, qPrintable(QStringLiteral("%1x%2").arg(box.w).arg(box.h)));
    }

    void theDrawnBoxPreviewIsTheSizeTheBoxGetsEvenWhenTheDragIsHuge()
    {
        EditWindow e;
        QVERIFY(open(e));
        e.view()->setZoom(0.1);
        crunch::SheetObjects *o = e.win.w.objects();
        QWidget *vp = e.view()->viewport();
        for (const QPoint by : {QPoint(120, 60), QPoint(700, 400)}) {   // (6000 by 3000 units is past the most a box can be, 4000 by 2000)
            e.win.w.act(QStringLiteral("insert.textBox.draw"))->trigger();
            const QPoint from = e.view()->cellRect(1, 1).center();
            QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, from);
            QTest::mouseMove(vp, from + by / 2);
            QTest::mouseMove(vp, from + by);
            const QRectF preview = o->layer()->boxPreview();
            QVERIFY(preview.width() > 0);
            QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, from + by);
            QVERIFY(e.settle());
            const grid::ObjectBox box = o->layer()->boxNow(o->layer()->selectedId());
            QVERIFY2(std::abs(box.w - preview.width()) < 3 && std::abs(box.h - preview.height()) < 3, qPrintable(QStringLiteral("%1x%2 shown, %3x%4 made").arg(preview.width()).arg(preview.height()).arg(box.w).arg(box.h)));
            if (by.x() > 500) QVERIFY2(preview.width() <= 4000.5 && preview.height() <= 2000.5, qPrintable(QStringLiteral("%1x%2").arg(preview.width()).arg(preview.height())));
        }
    }

    void theArmedToolShowsACrossAndEscAndAnyOtherCommandPutItDown()
    {
        EditWindow e;
        QVERIFY(open(e));
        crunch::SheetObjects *o = e.win.w.objects();
        e.win.w.act(QStringLiteral("insert.textBox.draw"))->trigger();
        QVERIFY(o->layer()->boxArmed());
        QTest::mouseMove(e.view()->viewport(), e.view()->cellRect(5, 3).center());
        QTRY_COMPARE(e.view()->viewport()->cursor().shape(), Qt::CrossCursor);
        QTest::keyClick(e.view(), Qt::Key_Escape);
        QVERIFY(!o->layer()->boxArmed());
        const QPoint from = e.view()->cellRect(4, 2).center();
        drag(e, from, from + QPoint(200, 120));
        QCOMPARE(o->layer()->count(), 0);   // (nothing was drawn)
        e.win.w.act(QStringLiteral("insert.textBox.draw"))->trigger();
        QVERIFY(o->layer()->boxArmed());
        e.win.w.act(QStringLiteral("insert.shape.rect"))->trigger();   // another tool
        QVERIFY(!o->layer()->boxArmed());
        QCOMPARE(o->layer()->count(), 1);
    }

    void textDirectionHasThreeChoicesAndEachIsOneStepThatUndoesAndIsSaved()
    {
        EditWindow e;
        QVERIFY(open(e));
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(e.win.w.insertTextBox());
        QVERIFY(e.settle());
        const int id = o->layer()->selectedId();
        for (const char *name : {"object.dir.horz", "object.dir.r90", "object.dir.r270"}) QVERIFY2(e.win.w.act(QString::fromLatin1(name)), name);
        QTRY_VERIFY(e.win.w.act(QStringLiteral("object.dir.r90"))->isEnabled());
        QTRY_VERIFY(e.win.w.act(QStringLiteral("object.dir.horz"))->isChecked());
        QUndoStack *stack = e.win.w.session()->undoStack();
        const int steps = stack->count();
        e.win.w.act(QStringLiteral("object.dir.r90"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(elementOf(e, liveId(e)).contains(QStringLiteral("vert=\"vert\"")));
        QCOMPARE(stack->count(), steps + 1);
        QTRY_VERIFY(e.win.w.act(QStringLiteral("object.dir.r90"))->isChecked());
        e.win.w.act(QStringLiteral("object.dir.r270"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(elementOf(e, liveId(e)).contains(QStringLiteral("vert=\"vert270\"")));
        QCOMPARE(stack->count(), steps + 2);
        QTRY_VERIFY(e.win.w.act(QStringLiteral("object.dir.r270"))->isChecked());
        // (the saved file says it, and opens turned)
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save() && savedOk(saved));
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        QVERIFY(parts.value(QStringLiteral("xl/drawings/drawing1.xml")).contains("vert=\"vert270\""));
        EditWindow again;
        QVERIFY(again.open(parts.isEmpty() ? QByteArray() : [&] { QFile f(e.path); f.open(QIODevice::ReadOnly); return f.readAll(); }()));
        QVERIFY(again.settle());
        QTRY_COMPARE(again.win.w.objects()->layer()->count(), 1);
        QVERIFY(elementOf(again, liveId(again)).contains(QStringLiteral("vert=\"vert270\"")));
        // (undo gives the old direction back)
        stack->undo();
        QVERIFY(e.settle());
        QVERIFY(elementOf(e, liveId(e)).contains(QStringLiteral("vert=\"vert\"")));
        stack->undo();
        QVERIFY(e.settle());
        QVERIFY(!elementOf(e, liveId(e)).contains(QStringLiteral("vert=")));
        (void)id;
    }

    // A direction this program does not know (a value that is none of ours, or one 100,000 characters long) reads as across and is kept in the file until a direction is chosen.
    void aDirectionThisProgramDoesNotKnowReadsAcrossAndIsKeptUntilOneIsPicked()
    {
        for (const QString &value : {QStringLiteral("junk"), QStringLiteral("wordArtVert"), QString(100000, QLatin1Char('x'))}) {
            const QString sp = QStringLiteral("<xdr:sp macro=\"\" textlink=\"\"><xdr:nvSpPr><xdr:cNvPr id=\"2\" name=\"Box\"/><xdr:cNvSpPr txBox=\"1\"/></xdr:nvSpPr><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/>"
                                              "<a:ext cx=\"1905000\" cy=\"476250\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom><a:noFill/></xdr:spPr><xdr:txBody><a:bodyPr vert=\"%1\" wrap=\"square\"/><a:lstStyle/>"
                                              "<a:p><a:r><a:rPr lang=\"en-US\"/><a:t>Words</a:t></a:r></a:p></xdr:txBody></xdr:sp>").arg(value);
            EditWindow e;
            QVERIFY(e.open(bookWithDrawing({worksheet(QString())}, drawingPart(twoCell(1, 1, 4, 5, sp)), {})));
            QVERIFY(e.settle());
            crunch::SheetObjects *o = e.win.w.objects();
            QTRY_COMPARE(o->layer()->count(), 1);
            o->layer()->select(o->layer()->object(0).id);
            QTRY_VERIFY(e.win.w.act(QStringLiteral("object.dir.horz"))->isEnabled());
            QTRY_VERIFY(e.win.w.act(QStringLiteral("object.dir.horz"))->isChecked());   // (read across)
            QUndoStack *stack = e.win.w.session()->undoStack();
            // (an edit of the same object that is not about the direction: the object is made again, and the file's value stays in it)
            QVERIFY(e.win.w.setObjectFill(QColor(255, 200, 0), false));
            QVERIFY(e.settle());
            QVERIFY(elementOf(e, liveId(e)).contains(QString(QStringLiteral("vert=\"%1\"")).arg(value)));
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            QVERIFY2(e.win.w.save() && savedOk(saved), qPrintable(QString::number(value.size())));
            QVERIFY(partsOf(e.path).value(QStringLiteral("xl/drawings/drawing1.xml")).contains(QString(QStringLiteral("vert=\"%1\"")).arg(value).toUtf8()));
            // (across picked, though it reads across already: the value goes, as in the other apps, in one step; picked again, nothing)
            int steps = stack->count();
            e.win.w.act(QStringLiteral("object.dir.horz"))->trigger();
            QVERIFY(e.settle());
            QCOMPARE(stack->count(), steps + 1);
            QVERIFY(!elementOf(e, liveId(e)).contains(QStringLiteral("vert=")));
            e.win.w.act(QStringLiteral("object.dir.horz"))->trigger();
            QVERIFY(e.settle());
            QCOMPARE(stack->count(), steps + 1);
            steps = stack->count();
            // (a direction picked replaces it, and across again leaves none)
            e.win.w.act(QStringLiteral("object.dir.r90"))->trigger();
            QVERIFY(e.settle());
            QCOMPARE(stack->count(), steps + 1);
            QVERIFY(elementOf(e, liveId(e)).contains(QStringLiteral("vert=\"vert\"")));
            QVERIFY(!elementOf(e, liveId(e)).contains(QString(QStringLiteral("vert=\"%1\"")).arg(value)));
            QSignalSpy saved90(&e.win.w, &CrunchWindow::saved);
            QVERIFY(e.win.w.save() && savedOk(saved90));
            QVERIFY(partsOf(e.path).value(QStringLiteral("xl/drawings/drawing1.xml")).contains("vert=\"vert\""));
            e.win.w.act(QStringLiteral("object.dir.horz"))->trigger();
            QVERIFY(e.settle());
            QVERIFY(!elementOf(e, liveId(e)).contains(QStringLiteral("vert=")));
            QSignalSpy savedAgain(&e.win.w, &CrunchWindow::saved);
            QVERIFY(e.win.w.save() && savedOk(savedAgain));
            QVERIFY(!partsOf(e.path).value(QStringLiteral("xl/drawings/drawing1.xml")).contains("vert="));
        }
    }

    void theEditorOfATurnedBoxIsTurnedAndAClickNearTheStartOfTheWordsPutsTheCaretAtTheStart()
    {
        for (const bool back : {false, true}) {
            EditWindow e;
            QVERIFY(open(e));
            crunch::SheetObjects *o = e.win.w.objects();
            e.win.w.act(QStringLiteral("insert.textBox.draw"))->trigger();
            const QPoint from = e.view()->cellRect(4, 2).center();
            drag(e, from, from + QPoint(100, 300));
            QVERIFY(e.settle());
            QVERIFY(e.win.w.setTextBoxDirection(back ? TextDirection::Rotate270 : TextDirection::Rotate90));
            QVERIFY(e.settle());
            const int id = o->layer()->selectedId();
            QVERIFY(id >= 0);
            ShapeTextEdit *edit = editShape(e);
            QVERIFY(edit);
            edit->setPlainText(QStringLiteral("Hello turned words"));
            QTextCursor c = edit->textCursor();
            c.setPosition(10);
            edit->setTextCursor(c);
            // (the words begin at the top of the right side, or, turned 270 degrees, the bottom of the left: a click a few pixels in from there)
            const QRect frame = edit->rect();
            const QPoint at = back ? QPoint(14, frame.height() - 8) : QPoint(frame.width() - 14, 8);
            QTest::mouseClick(edit->viewport(), Qt::LeftButton, Qt::NoModifier, at);
            QCOMPARE(edit->textCursor().position(), 0);
            QTest::keyClick(edit, Qt::Key_Escape);
        }
    }

    // A picture of a drawn text box whose words run turned, for a person to look at (JO_SHOT_DIR names the folder; nothing is made without it).
    void aDrawnTurnedTextBoxScreenshot()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        QDir().mkpath(dir);
        EditWindow e;
        QVERIFY(open(e));
        e.win.w.act(QStringLiteral("insert.textBox.draw"))->trigger();
        const QPoint from = e.view()->cellRect(4, 2).center();
        drag(e, from, from + QPoint(90, 230));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.setTextBoxDirection(TextDirection::Rotate90));
        QVERIFY(e.settle());
        ShapeTextEdit *edit = editShape(e);
        QVERIFY(edit);
        edit->insertPlainText(QStringLiteral("Words that run down the box"));
        QTest::keyClick(edit, Qt::Key_Escape);
        QVERIFY(e.settle());
        e.win.w.objects()->layer()->prepareAll();
        QApplication::processEvents();
        QVERIFY(e.win.w.grab().save(dir + QStringLiteral("/crunch-turned-text-box.png")));
    }
};

JP_TEST_CLASS(CrunchTextBoxTests)
#include "tst_crunch_textbox.moc"
