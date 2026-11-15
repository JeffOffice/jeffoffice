// What every app's test of typing in a shape shares: the same keys and the same checks, run on the editor the app put over the shape (app/shapetextedit.h).
#pragma once

#include "app/shapetextedit.h"

#include <QApplication>
#include <QClipboard>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QtTest>

namespace jptest {

// Types "Hello big world", then makes "big" bold (Ctrl+B), 20 points, and red, and centers the paragraph (Ctrl+E).
inline void typeAndFormat(jp::ShapeTextEdit *e)
{
    QTest::keyClicks(e, QStringLiteral("Hello big world"));
    QCOMPARE(e->toPlainText(), QStringLiteral("Hello big world"));
    QTextCursor c = e->textCursor();
    c.setPosition(6);
    c.setPosition(9, QTextCursor::KeepAnchor);
    e->setTextCursor(c);
    QTest::keyClick(e, Qt::Key_B, Qt::ControlModifier);
    e->setFontSizePt(20);
    e->setColor(QColor(Qt::red));
    QTest::keyClick(e, Qt::Key_E, Qt::ControlModifier);
}

// Presses a standard key (Undo, Redo: the keys are not the same on every system).
inline void pressStandard(QWidget *w, QKeySequence::StandardKey key)
{
    const QKeyCombination k = QKeySequence(key)[0];
    QTest::keyClick(w, k.key(), k.keyboardModifiers());
}

// The story holds what typeAndFormat made.
inline void verifyFormatted(const QTextDocument *story)
{
    QVERIFY(story);
    QCOMPARE(story->toPlainText(), QStringLiteral("Hello big world"));
    QTextCursor c(const_cast<QTextDocument *>(story));
    c.setPosition(7);
    const QTextCharFormat big = c.charFormat();
    QVERIFY(big.font().bold());
    QCOMPARE(big.fontPointSize(), 20.0);
    QCOMPARE(big.foreground().color(), QColor(Qt::red));
    c.setPosition(2);
    QVERIFY(!c.charFormat().font().bold());
    QVERIFY(c.charFormat().foreground().color() != QColor(Qt::red));
    c.setPosition(13);
    QVERIFY(!c.charFormat().font().bold());
    QCOMPARE(story->firstBlock().blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
}

// The editor holds what typeAndFormat made.
inline void verifyEditor(jp::ShapeTextEdit *e) { verifyFormatted(e->story()); }

} // namespace jptest
