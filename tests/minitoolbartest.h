#pragma once
// The mini toolbar's checks, the same in JeffDeck and JeffPub: each app's window test sets up one text box holding
// "Some words to pick out of the line" (the words start `lead` characters in) and hands its window over.
#include "../tests/withdialog.h"
#include "app/editor.h"
#include "app/minitoolbar.h"
#include "app/widgets.h"
#include "canvas/canvas.h"
#include "text/textprops.h"
#include <QScrollBar>
#include <QSlider>
#include <QToolButton>
#include <QtTest>

namespace minitest {

inline const QString kText = QStringLiteral("Some words to pick out of the line");

template <class Win> struct Rig {
    Win &w;
    QString boxId, storyId;
    int lead = 0;
    jp::Editor *ed = nullptr;
    jp::Canvas *canvas = nullptr;
    QWidget *view = nullptr;
    QFrame *mini = nullptr;

    Rig(Win &window, const QString &box, const QString &story, int leadChars) : w(window), boxId(box), storyId(story), lead(leadChars)
    {
        ed = w.editor();
        canvas = w.canvas();
        view = canvas->viewport();
        mini = view->findChild<QFrame *>(QStringLiteral("miniToolbar"));
    }
    QPoint at(int pos) const { return view->mapFromGlobal(canvas->caretScreenRect(pos).center()); }
    // A point a third of the way into the character after `pos`. Both hit-test styles (nearest boundary, or the start of
    // the character under the point) give `pos` there; the exact boundary can land on the space's side (macOS, Nov 17).
    QPoint inside(int pos) const
    {
        const QPoint a = at(pos), b = at(pos + 1);
        return a + (b - a) / 3;
    }
    // A drag over characters `from` to `to` of the box's text, with the mouse.
    void select(int from = 5, int to = 10)
    {
        ed->beginTextEdit(boxId, 0);
        QTest::mousePress(view, Qt::LeftButton, Qt::NoModifier, inside(from + lead));
        QTest::mouseMove(view, at((from + to) / 2 + lead));
        QVERIFY(!mini->isVisible());   // (not while the mouse is still down)
        QTest::mouseMove(view, inside(to + lead));
        QTest::mouseRelease(view, Qt::LeftButton, Qt::NoModifier, inside(to + lead));
        QTRY_VERIFY2(mini->isVisible(), qPrintable(QStringLiteral("drag %1 to %2 at %3,%4 to %5,%6: the selection is %7 to %8").arg(from).arg(to).arg(at(from + lead).x()).arg(at(from + lead).y()).arg(at(to + lead).x()).arg(at(to + lead).y()).arg(ed->cursor().anchor()).arg(ed->cursor().position())));
    }
    QToolButton *button(const char *id) const
    {
        for (QToolButton *b : mini->findChildren<QToolButton *>())
            if (b->defaultAction() == w.act(QString::fromLatin1(id))) return b;
        return nullptr;
    }
    QTextCursor probe(int pos) const
    {
        QTextCursor c(ed->doc()->storyDoc(storyId));
        c.setPosition(pos + lead);
        c.setPosition(pos + lead + 1, QTextCursor::KeepAnchor);
        return c;
    }
};


#define MINI_SELECT(rig, ...) do { (rig).select(__VA_ARGS__); if (QTest::currentTestFailed()) return; } while (false)

template <class Win> void floats(Win &w, Rig<Win> &r, const QString &shot, const char *boldId = "fmt.bold")
{
    QVERIFY(r.mini);
    QVERIFY(!r.mini->isVisible());
    MINI_SELECT(r);
    QCOMPARE(r.ed->cursor().selectedText(), QStringLiteral("words"));
    const QRect firstLine = QRect(r.view->mapFromGlobal(r.canvas->caretScreenRect(5 + r.lead).topLeft()), r.canvas->caretScreenRect(5 + r.lead).size());
    QVERIFY2(r.mini->geometry().bottom() <= firstLine.top() + 2 || r.mini->geometry().top() > firstLine.bottom(), "above the words, or below them when there is no room above");
    QVERIFY(r.view->rect().contains(r.mini->geometry()));
    // The buttons show the selection's state at once, not after the ribbon's delayed refresh.
    QToolButton *bold = r.button(boldId);
    QVERIFY(bold && bold->isEnabled());
    auto *font = r.mini->template findChild<jp::FontCombo *>();
    QVERIFY(font && font->isEnabled() && !font->currentText().isEmpty());
    QVERIFY(r.mini->template findChild<jp::ColorButton *>());
    QVERIFY(r.mini->template findChildren<QToolButton *>().size() >= 6);
    shotOf(&w, shot);
    QVERIFY(!QApplication::focusWidget() || !r.mini->isAncestorOf(QApplication::focusWidget()));
    // The state is the selection's: bold words show Bold pressed the moment the bar comes.
    QVERIFY(!bold->isChecked());
    r.ed->toggleBold();
    r.ed->clearSelection();
    MINI_SELECT(r);
    QVERIFY(r.button(boldId)->isChecked());
    // A new selection while the bar is up: Bold shows the new words' state at once (plain words first, then the bold ones).
    MINI_SELECT(r, 20, 24);
    QVERIFY(!r.button(boldId)->isChecked());
    QTextCursor boldWords = r.ed->cursor();
    boldWords.setPosition(5 + r.lead);
    boldWords.setPosition(10 + r.lead, QTextCursor::KeepAnchor);
    r.ed->setCursor(boldWords);
    QVERIFY(r.mini->isVisible());
    QTest::mouseRelease(r.view, Qt::LeftButton, Qt::NoModifier, r.at(10 + r.lead));
    QCoreApplication::sendPostedEvents(nullptr, QEvent::MetaCall);
    QVERIFY(r.button(boldId)->isChecked());
    // A click that leaves no selection brings none.
    r.ed->beginTextEdit(r.boxId, 0);
    QTest::mouseClick(r.view, Qt::LeftButton, Qt::NoModifier, r.at(3 + r.lead));
    QTRY_VERIFY(r.ed->isEditingText() && !r.ed->cursor().hasSelection());
    QCoreApplication::sendPostedEvents(nullptr, QEvent::MetaCall);
    QVERIFY(!r.mini->isVisible());
}

// Each control of the bar changes the selected words as the ribbon's does, in one step of Undo.
template <class Win> void buttonsChangeTheSelection(Win &, Rig<Win> &r, const char *rightId)
{
    QVERIFY(r.mini);
    auto once = [&](const char *what, const std::function<void()> &act, const std::function<bool()> &changed) {
        MINI_SELECT(r);
        const int steps = r.ed->undoStack()->index();
        act();
        QVERIFY2(changed(), what);
        QVERIFY2(r.ed->undoStack()->index() == steps + 1, what);
        r.ed->undo();
        QVERIFY2(!changed(), what);
    };
    auto *font = r.mini->template findChild<jp::FontCombo *>();
    auto *color = r.mini->template findChild<jp::ColorButton *>();
    QVERIFY(font && color);
    // (the bar must be up for its controls to exist as the user sees them: select first, then look)
    MINI_SELECT(r);
    once("font", [&] { Q_EMIT font->familyChosen(QStringLiteral("Courier New")); }, [&] { return r.probe(6).charFormat().fontFamilies().toStringList().value(0) == QStringLiteral("Courier New"); });
    QTextCharFormat base;
    once("theme font", [&] { base = r.probe(6).charFormat(); Q_EMIT font->familyChosen(QStringLiteral("+Heading")); }, [&] { return r.probe(6).charFormat() != base; });
    once("color", [&] { Q_EMIT color->colorPicked(jp::ColorRef::rgb(QColor(0x12, 0x34, 0x56))); }, [&] { return r.probe(6).charFormat().stringProperty(jp::tp::ColorRefP) == jp::ColorRef::rgb(QColor(0x12, 0x34, 0x56)).toString(); });
    once("alignment", [&] { r.button(rightId)->click(); }, [&] { return (r.probe(6).blockFormat().alignment() & Qt::AlignHorizontal_Mask) == Qt::AlignRight; });
    once("bullets", [&] { r.button("para.bullets")->click(); }, [&] { return r.probe(6).block().textList() != nullptr; });
    double before = 0;
    once("size", [&] { before = r.probe(6).charFormat().fontPointSize(); r.button("fmt.grow")->click(); }, [&] { return r.probe(6).charFormat().fontPointSize() > before || (before == 0 && r.probe(6).charFormat().fontPointSize() > 0); });
}

// The bar never floats detached: it goes (or follows) when the view or the words move.
template <class Win> void goesWhenTheWordsMove(Win &w, Rig<Win> &r, const char *rightId)
{
    QVERIFY(r.mini);
    // Keys the bar handles itself: with the editor's signals held, only the bar's own filter can hide it.
    for (Qt::Key key : {Qt::Key_X, Qt::Key_Escape, Qt::Key_Backspace}) {
        MINI_SELECT(r);
        const QSignalBlocker hold(r.ed);
        QTest::keyClick(r.canvas, key);
        QVERIFY2(!r.mini->isVisible(), qPrintable(QString::number(key)));
    }
    // A real scroll range, and a real change of the value.
    r.ed->setCurrentPage(r.ed->currentPage());
    r.canvas->setZoom(1.0);
    QScrollBar *bar = nullptr;
    QTRY_VERIFY((bar = r.canvas->verticalScrollBar()->maximum() > 0 ? r.canvas->verticalScrollBar() : (r.canvas->horizontalScrollBar()->maximum() > 0 ? r.canvas->horizontalScrollBar() : nullptr)));
    MINI_SELECT(r);
    const int was = bar->value();
    bar->setValue(was == bar->maximum() ? was - 1 : was + 1);
    QVERIFY(bar->value() != was);
    QVERIFY(!r.mini->isVisible());
    // The wheel.
    MINI_SELECT(r);
    QWheelEvent wheel(QPointF(20, 20), QPointF(r.view->mapToGlobal(QPoint(20, 20))), QPoint(), QPoint(0, -120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(r.view, &wheel);
    QVERIFY(!r.mini->isVisible());
    // The zoom slider, and a zoom from the canvas.
    auto *slider = w.template findChild<QSlider *>();
    QVERIFY(slider);
    MINI_SELECT(r);
    slider->setValue(slider->value() + 5);
    QTRY_VERIFY(!r.mini->isVisible());
    MINI_SELECT(r);
    r.canvas->setZoom(r.canvas->zoom() * 1.25);
    QVERIFY(!r.mini->isVisible());
    // A resize.
    MINI_SELECT(r);
    w.resize(w.width() - 120, w.height() - 80);
    QTRY_VERIFY(!r.mini->isVisible());
    // Text that moves the words takes the bar along (or it goes): after bold or a bullet it is still by the selection.
    MINI_SELECT(r);
    r.button(rightId)->click();
    QTRY_VERIFY(r.mini->isVisible());
    const QRect now = r.mini->geometry();
    const QRect line(r.view->mapFromGlobal(r.canvas->caretScreenRect(5 + r.lead).topLeft()), r.canvas->caretScreenRect(5 + r.lead).size());
    QVERIFY2(qAbs(now.left() - line.left()) < now.width() && (now.bottom() <= line.top() + 2 || now.top() > line.bottom()), "still by the words");
    // Undo that takes the words away, a box that is deleted, and another page.
    r.ed->beginTextEdit(r.boxId, 0);
    QTextCursor at0 = r.ed->cursor();
    at0.setPosition(r.lead);
    r.ed->setCursor(at0);
    r.ed->insertText(QStringLiteral("plus more "));   // (the words the undo will take away)
    QTRY_VERIFY(!r.canvas->caretScreenRect(8 + r.lead).isNull() && r.view->rect().contains(r.at(8 + r.lead)));
    MINI_SELECT(r, 2, 6);
    r.ed->undo();
    QTRY_VERIFY(!r.mini->isVisible());
    QVERIFY(!r.ed->doc()->storyDoc(r.storyId)->toPlainText().contains(QStringLiteral("plus")));
    MINI_SELECT(r);
    r.ed->deleteItems({r.boxId});
    QTRY_VERIFY(!r.mini->isVisible());
}

template <class Win> void goesWithAnotherPage(Rig<Win> &r)
{
    QVERIFY(r.mini);
    r.ed->insertPages(0, 1, false, false);
    r.ed->setCurrentPage(0);
    MINI_SELECT(r);
    r.ed->setCurrentPage(1);
    QTRY_VERIFY(!r.mini->isVisible());
}

} // namespace minitest
