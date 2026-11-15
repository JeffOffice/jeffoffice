// What every app's test of typing in a shape shares: the same keys and the same checks, run on the editor the app put over the shape (app/shapetextedit.h).
#pragma once

#include "app/editor.h"
#include "app/shapetextedit.h"
#include "core/color.h"
#include "text/textprops.h"

#include <QApplication>
#include <QDir>
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

// The color of a run: the editor's own (a brush), or the page apps' (a color of their theme, written as text).
inline QColor runColor(const QTextCharFormat &f)
{
    if (f.hasProperty(jp::tp::ColorRefP)) return QColor(jp::ColorRef::fromString(f.stringProperty(jp::tp::ColorRefP)).toString());   // ("#RRGGBB" for a color of its own)
    return f.foreground().style() == Qt::NoBrush ? QColor(Qt::black) : f.foreground().color();
}

// A picture of a window while the words are typed (when the folder is named in JO_SHAPETEXT_SHOTS): for looking at what the editor looks like in each app.
inline void shapeShot(QWidget *window, const QString &app)
{
    const QString dir = qEnvironmentVariable("JO_SHAPETEXT_SHOTS");
    if (dir.isEmpty()) return;
    QDir().mkpath(dir);
    window->grab().save(dir + QLatin1Char('/') + app + QStringLiteral("-typing-in-a-shape.png"));
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
    QCOMPARE(runColor(big), QColor(Qt::red));
    c.setPosition(2);
    QVERIFY(!c.charFormat().font().bold());
    QVERIFY(runColor(c.charFormat()) != QColor(Qt::red));
    c.setPosition(13);
    QVERIFY(!c.charFormat().font().bold());
    QCOMPARE(story->firstBlock().blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
}

// The same on the page of JeffPub or JeffDeck: the words are typed on the canvas, in the shape the editor is editing, with the same keys (Ctrl+B and Ctrl+E), and the size and the color
// set as the ribbon sets them.
inline void typeAndFormatOnPage(jp::Editor *ed, QWidget *canvas)
{
    QTest::keyClicks(canvas, QStringLiteral("Hello big world"));
    QVERIFY(ed->editDoc());
    QCOMPARE(ed->editDoc()->toPlainText(), QStringLiteral("Hello big world"));
    QTextCursor c = ed->cursor();
    c.setPosition(6);
    c.setPosition(9, QTextCursor::KeepAnchor);
    ed->setCursor(c);
    QTest::keyClick(canvas, Qt::Key_B, Qt::ControlModifier);
    ed->setFontSize(20);
    ed->setTextColor(jp::ColorRef::rgb(QColor(Qt::red)));
    QTest::keyClick(canvas, Qt::Key_E, Qt::ControlModifier);
}

// The editor holds what typeAndFormat made.
inline void verifyEditor(jp::ShapeTextEdit *e) { verifyFormatted(e->story()); }

} // namespace jptest
