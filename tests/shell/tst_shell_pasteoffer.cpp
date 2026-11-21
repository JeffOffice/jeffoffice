// The Paste Options button every app shares (libs/shell/app/pasteoffer.h): where it sits, the key that opens it, its menu, and what happens to it when the window it is in goes.
#include "app/pasteoffer.h"
#include "../testrunner.h"

#include <QAction>
#include <QApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QSignalSpy>
#include <QToolButton>
#include <QtTest>

using namespace jp;

namespace {

void key(QWidget *to, QEvent::Type type, int k)
{
    QKeyEvent e(type, k, Qt::NoModifier);
    QApplication::sendEvent(to, &e);
}

void closePopup()
{
    if (QWidget *p = QApplication::activePopupWidget()) p->close();
    QTRY_VERIFY(!QApplication::activePopupWidget());
}

// A host the button floats over, and a widget that has the keyboard.
struct Bench {
    QWidget host;
    QWidget *focus = new QWidget(&host);
    int asked = 0, ran = 0;
    PasteOffer *offer = nullptr;
    Bench()
    {
        host.resize(300, 200);
        focus->setFocusPolicy(Qt::StrongFocus);
        offer = new PasteOffer(&host, focus, [this] {
            ++asked;
            return QVector<PasteChoice>{{QStringLiteral("First"), QStringLiteral("pasteOption0"), [this] { ++ran; }, false},
                                        {QStringLiteral("Second"), QStringLiteral("pasteOption1"), [this] { ran += 10; }, true}};
        });
        offer->installOn(focus);
        host.show();
        QVERIFY(QTest::qWaitForWindowExposed(&host));
    }
};

} // namespace

class ShellPasteOfferTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    // The button sits below and to the right of what was pasted, stays inside the host, and goes when there is nothing to sit by.
    void theButtonSitsByTheAnchorAndGoesWithoutOne()
    {
        Bench b;
        QVERIFY(!b.offer->isShown());
        b.offer->showAt(QRect(10, 10, 20, 20));
        QToolButton *button = b.offer->button();
        QVERIFY(button && button->isVisible());
        QCOMPARE(button->objectName(), QStringLiteral("pasteOptionsButton"));
        QCOMPARE(button->x(), QRect(10, 10, 20, 20).right() + 6);
        QCOMPARE(button->y(), QRect(10, 10, 20, 20).bottom() + 4);
        b.offer->showAt(QRect(290, 190, 5, 5));
        QVERIFY(button->geometry().right() < b.host.width() && button->geometry().bottom() < b.host.height());
        b.offer->showAt(QRect());
        QVERIFY(!b.offer->isShown());
        b.offer->showAt(QRect(10, 10, 20, 20));
        b.offer->hide();
        QVERIFY(!button->isVisible());
    }

    // The key tapped by itself opens the menu; a key, a shortcut, a click, or the wheel between its press and release is no tap, and with no button there is nothing to open.
    void aTapOfTheKeyOpensTheMenuAndAnythingBetweenIsNoTap()
    {
        Bench b;
        QSignalSpy opened(b.offer, &PasteOffer::opened);
        const int ctrl = PasteOffer::key();
        b.offer->showAt(QRect(10, 10, 20, 20));
        auto tap = [&](const std::function<void()> &between) {
            const int before = opened.count();
            key(b.focus, QEvent::KeyPress, ctrl);
            if (between) between();
            key(b.focus, QEvent::KeyRelease, ctrl);
            return opened.count() - before;
        };
        QCOMPARE(tap(nullptr), 1);
        QTRY_VERIFY(QApplication::activePopupWidget());
        closePopup();
        QCOMPARE(tap([&] { key(b.focus, QEvent::KeyPress, Qt::Key_Shift); key(b.focus, QEvent::KeyRelease, Qt::Key_Shift); }), 0);
        QCOMPARE(tap([&] { QKeyEvent e(QEvent::ShortcutOverride, Qt::Key_V, Qt::ControlModifier); QApplication::sendEvent(b.focus, &e); }), 0);
        QCOMPARE(tap([&] { QMouseEvent e(QEvent::MouseButtonPress, QPointF(5, 5), QPointF(5, 5), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier); QApplication::sendEvent(b.focus, &e); }), 0);
        b.offer->hide();
        QCOMPARE(tap(nullptr), 0);
        QVERIFY(!QApplication::activePopupWidget());
    }

    // Esc says so while the button is shown, and only then.
    void escapeIsToldOnlyWhileTheButtonIsShown()
    {
        Bench b;
        QSignalSpy escaped(b.offer, &PasteOffer::escaped);
        key(b.focus, QEvent::KeyPress, Qt::Key_Escape);
        QCOMPARE(escaped.count(), 0);
        b.offer->showAt(QRect(10, 10, 20, 20));
        key(b.focus, QEvent::KeyPress, Qt::Key_Escape);
        QCOMPARE(escaped.count(), 1);
    }

    // The choices are asked when the menu opens (not before), each is an action of its own name, a choice runs, and the keyboard goes back to the widget that had it.
    void theMenuHoldsTheChoicesAskedWhenItOpensAndAChoiceRuns()
    {
        Bench b;
        b.offer->showAt(QRect(10, 10, 20, 20));
        QCOMPARE(b.asked, 0);
        QMenu *menu = b.offer->button()->menu();
        QVERIFY(b.offer->openMenu());
        QTRY_VERIFY(menu->isVisible());
        QCOMPARE(b.asked, 1);
        QStringList names;
        int separators = 0;
        for (QAction *a : menu->actions()) {
            if (a->isSeparator()) ++separators;
            else names << a->objectName() + QLatin1Char('=') + a->text();
        }
        QCOMPARE(names, (QStringList{"pasteOption0=First", "pasteOption1=Second"}));
        QCOMPARE(separators, 1);
        QAction *second = menu->findChild<QAction *>(QStringLiteral("pasteOption1"));
        QVERIFY(second);
        b.focus->clearFocus();
        QVERIFY(!b.focus->hasFocus());
        second->trigger();
        QCOMPARE(b.ran, 10);
        QTRY_VERIFY(b.focus->hasFocus());
        closePopup();
        // opened again, the choices are asked again
        QVERIFY(b.offer->openMenu());
        QTRY_VERIFY(menu->isVisible());
        QCOMPARE(b.asked, 2);
        closePopup();
    }

    // A window that is closing is not called back by its button: the offer takes its button away with it, and what the menu held does not run.
    void aClosingWindowIsNotCalledBack()
    {
        auto *b = new Bench;
        QPointer<QToolButton> button;
        b->offer->showAt(QRect(10, 10, 20, 20));
        button = b->offer->button();
        QVERIFY(button);
        QMenu *menu = button->menu();
        QVERIFY(b->offer->openMenu());
        QTRY_VERIFY(menu->isVisible());
        int asked = b->asked;
        QPointer<PasteOffer> offer = b->offer;
        delete b->offer;
        QVERIFY(offer.isNull());
        QVERIFY(button.isNull());
        QCOMPARE(b->asked, asked);
        QVERIFY(!QApplication::activePopupWidget());
        // the keys of the widget that had the keyboard do nothing now
        key(b->focus, QEvent::KeyPress, PasteOffer::key());
        key(b->focus, QEvent::KeyRelease, PasteOffer::key());
        QCOMPARE(b->asked, asked);
        delete b;
    }

    // The offer goes without cutting what others hung on the button and its menu: the style sheet's own list of widgets (it hears of a widget going from
    // its destroyed signal) was cut by a wildcard disconnect, and the next change of the interface theme read a widget that was gone.
    void whatOthersHungOnTheButtonAndItsMenuSurvivesTheOffer()
    {
        Bench b;
        b.offer->showAt(QRect(10, 10, 20, 20));
        QToolButton *button = b.offer->button();
        QMenu *menu = button->menu();
        int buttonGone = 0, menuGone = 0;
        QObject sink;
        connect(button, &QObject::destroyed, &sink, [&buttonGone] { ++buttonGone; });
        connect(menu, &QObject::destroyed, &sink, [&menuGone] { ++menuGone; });
        delete b.offer;
        b.offer = nullptr;
        QCOMPARE(buttonGone, 1);
        QCOMPARE(menuGone, 1);
    }
};

JP_TEST_CLASS(ShellPasteOfferTests)
#include "tst_shell_pasteoffer.moc"
