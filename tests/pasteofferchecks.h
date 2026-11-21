#pragma once
// The Paste Options button's Ctrl tap, tried through the real widgets of an app (the grid, the canvas): the key goes down, something else happens, the key comes up, and the menu is up or not.
#include "app/pasteoffer.h"

#include <QApplication>
#include <QWheelEvent>
#include <QtTest>

namespace jp::pastecheck {

inline void closeMenus()
{
    while (QWidget *p = QApplication::activePopupWidget()) {
        p->close();
        QCoreApplication::processEvents();
    }
}

// Ctrl goes down on `focus`, `between` is done, Ctrl comes up: whether the menu came up (it is closed again after).
template <class F>
bool menuAfterCtrl(QWidget *focus, F between)
{
    closeMenus();
    QTest::keyEvent(QTest::Press, focus, Qt::Key(PasteOffer::key()), Qt::NoModifier);
    between();
    QTest::keyEvent(QTest::Release, focus, Qt::Key(PasteOffer::key()), Qt::NoModifier);
    QCoreApplication::processEvents();
    const bool up = QApplication::activePopupWidget() != nullptr;
    closeMenus();
    return up;
}

// A turn of the mouse wheel with Ctrl held (it zooms), on the viewport where a wheel goes.
inline void ctrlWheel(QWidget *viewport)
{
    const QPoint at = viewport->rect().center();
    QWheelEvent e(QPointF(at), QPointF(viewport->mapToGlobal(at)), QPoint(), QPoint(0, 120), Qt::NoButton, Qt::ControlModifier, Qt::NoScrollPhase, false);
    QCoreApplication::sendEvent(viewport, &e);
}

} // namespace jp::pastecheck
