// The editor every app puts over a shape that is typed in: its keys, its formats, bullets, copy and paste that keep the look, Undo, the input method, and the spelling
// marks. Each app's own test hosts it over a shape of its own (and runs the same helpers: shapetexttest.h).
#include "app/shapetextedit.h"
#include "../testrunner.h"
#include "shapetexttest.h"

#include <QInputMethodEvent>
#include <QShortcut>
#include <QSignalSpy>
#include <QWidget>

using namespace jp;

namespace {

std::unique_ptr<QTextDocument> story(const QString &text = QString())
{
    auto d = std::make_unique<QTextDocument>();
    d->setPlainText(text);
    return d;
}

} // namespace

class ShellShapeTextTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void typingAndFormats()
    {
        QWidget host;
        host.resize(400, 300);
        ShapeTextEdit e(&host);
        e.begin(story(), QRect(20, 20, 200, 100), ShapeTextLook());
        jptest::typeAndFormat(&e);
        jptest::verifyEditor(&e);
        QVERIFY(e.changed());
    }

    void wordsTurnedAQuarterAreLaidOutOnTheBoxsHeightAndMouseHitsFollowThem()
    {
        QWidget host;
        host.resize(400, 400);
        const QString text = QStringLiteral("Hello big world");
        ShapeTextLook look;
        look.insets = QMarginsF();
        {
            ShapeTextEdit e(&host);
            e.begin(story(text), QRect(0, 0, 100, 200), look);
            QCOMPARE(e.story()->textWidth(), 100.0);
        }
        // Turned clockwise: the line runs down the box, and its start is at the top (the first line at the right edge).
        look.quarterTurns = 1;
        ShapeTextEdit e(&host);
        e.begin(story(text), QRect(0, 0, 100, 200), look);
        QCOMPARE(e.story()->textWidth(), 200.0);
        QTest::mouseClick(e.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(98, 2));
        QCOMPARE(e.textCursor().position(), 0);
        QTest::mouseClick(e.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(98, 198));
        QCOMPARE(e.textCursor().position(), int(text.size()));
        // A drag from the top down selects from the start; the words are not typed across.
        QTest::mousePress(e.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(98, 2));
        QTest::mouseMove(e.viewport(), QPoint(98, 198));
        QTest::mouseRelease(e.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(98, 198));
        QCOMPARE(e.textCursor().selectedText(), text);
        e.take();
    }

    void wordsTurnedTheOtherWayStartAtTheBottom()
    {
        QWidget host;
        host.resize(400, 400);
        const QString text = QStringLiteral("Hello big world");
        ShapeTextLook look;
        look.insets = QMarginsF();
        look.quarterTurns = 3;
        ShapeTextEdit e(&host);
        e.begin(story(text), QRect(0, 0, 100, 200), look);
        QCOMPARE(e.story()->textWidth(), 200.0);
        QTest::mouseClick(e.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(2, 198));
        QCOMPARE(e.textCursor().position(), 0);
        QTest::mouseClick(e.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(2, 2));
        QCOMPARE(e.textCursor().position(), int(text.size()));
        e.take();
    }

    void escapeLeavesAndKeeps()
    {
        QWidget host;
        ShapeTextEdit e(&host);
        e.begin(story(QStringLiteral("abc")), QRect(0, 0, 200, 100), ShapeTextLook());
        QSignalSpy done(&e, &ShapeTextEdit::finished);
        QTest::keyClick(&e, Qt::Key_Escape);
        QCOMPARE(done.size(), 1);
        QCOMPARE(done.first().first().toBool(), true);
        auto s = e.take();
        QVERIFY(s);
        QCOMPARE(s->toPlainText(), QStringLiteral("abc"));
    }

    void keysOnASelectedShape()
    {
        QString typed;
        QKeyEvent f2(QEvent::KeyPress, Qt::Key_F2, Qt::NoModifier);
        QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        QKeyEvent ctrlEnter(QEvent::KeyPress, Qt::Key_Return, Qt::ControlModifier);
        QKeyEvent letter(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, QStringLiteral("a"));
        QKeyEvent ctrlA(QEvent::KeyPress, Qt::Key_A, Qt::ControlModifier, QStringLiteral("\x01"));
        QKeyEvent esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
        QCOMPARE(keyOnSelectedShape(&f2, &typed), SelectedShapeKey::Begin);
        QCOMPARE(keyOnSelectedShape(&enter, &typed), SelectedShapeKey::Begin);
        QCOMPARE(keyOnSelectedShape(&ctrlEnter, &typed), SelectedShapeKey::None);
        QCOMPARE(keyOnSelectedShape(&letter, &typed), SelectedShapeKey::BeginTyped);
        QCOMPARE(typed, QStringLiteral("a"));
        QCOMPARE(keyOnSelectedShape(&ctrlA, &typed), SelectedShapeKey::None);
        QCOMPARE(keyOnSelectedShape(&esc, &typed), SelectedShapeKey::None);
    }

    void typedKeyStartsTheEdit()
    {
        QWidget host;
        ShapeTextEdit e(&host);
        e.begin(story(QStringLiteral("old")), QRect(0, 0, 200, 100), ShapeTextLook(), QStringLiteral("x"));
        QCOMPARE(e.toPlainText(), QStringLiteral("oldx"));
        ShapeTextEdit r(&host);
        r.begin(story(QStringLiteral("old")), QRect(0, 0, 200, 100), ShapeTextLook(), QStringLiteral("x"), true);
        QCOMPARE(r.toPlainText(), QStringLiteral("x"));
    }

    void fontSteps()
    {
        QCOMPARE(nextFontSize(11, 1), 12.0);
        QCOMPARE(nextFontSize(11, -1), 10.0);
        QCOMPARE(nextFontSize(12, 1), 14.0);
        QCOMPARE(nextFontSize(6, -1), 5.0);
        QWidget host;
        ShapeTextEdit e(&host);
        e.begin(story(QStringLiteral("abc")), QRect(0, 0, 200, 100), ShapeTextLook());
        e.selectAll();
        QTest::keyClick(&e, Qt::Key_BracketRight, Qt::ControlModifier);
        QCOMPARE(e.story()->firstBlock().begin().fragment().charFormat().fontPointSize(), 12.0);
        QTest::keyClick(&e, Qt::Key_BracketLeft, Qt::ControlModifier);
        QCOMPARE(e.story()->firstBlock().begin().fragment().charFormat().fontPointSize(), 11.0);
    }

    void alignmentKeys()
    {
        QWidget host;
        ShapeTextEdit e(&host);
        e.begin(story(QStringLiteral("abc")), QRect(0, 0, 200, 100), ShapeTextLook());
        const std::pair<Qt::Key, Qt::Alignment> keys[] = {{Qt::Key_E, Qt::AlignHCenter}, {Qt::Key_R, Qt::AlignRight}, {Qt::Key_J, Qt::AlignJustify}, {Qt::Key_L, Qt::AlignLeft}};
        for (const auto &k : keys) {
            QTest::keyClick(&e, k.first, Qt::ControlModifier);
            QCOMPARE(e.story()->firstBlock().blockFormat().alignment() & Qt::AlignHorizontal_Mask, k.second);
        }
    }

    void theKeysOfTheWordsBeatTheWindowsShortcuts()
    {
        QWidget host;
        host.show();
        int fired = 0;
        QList<QShortcut *> shortcuts;
        for (const QKeySequence &k : {QKeySequence(Qt::CTRL | Qt::Key_E), QKeySequence(Qt::CTRL | Qt::Key_L), QKeySequence(Qt::CTRL | Qt::Key_R), QKeySequence(Qt::CTRL | Qt::Key_J), QKeySequence(Qt::Key_Escape)}) {
            auto *sc = new QShortcut(k, &host);
            connect(sc, &QShortcut::activated, this, [&fired] { ++fired; });
            shortcuts << sc;
        }
        ShapeTextEdit e(&host);
        e.begin(story(QStringLiteral("abc")), QRect(0, 0, 200, 100), ShapeTextLook());
        QSignalSpy done(&e, &ShapeTextEdit::finished);
        QTest::keyClick(&e, Qt::Key_E, Qt::ControlModifier);
        QCOMPARE(e.story()->firstBlock().blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
        QTest::keyClick(&e, Qt::Key_R, Qt::ControlModifier);
        QTest::keyClick(&e, Qt::Key_L, Qt::ControlModifier);
        QTest::keyClick(&e, Qt::Key_J, Qt::ControlModifier);
        QTest::keyClick(&e, Qt::Key_Escape);
        QCOMPARE(fired, 0);
        QCOMPARE(done.size(), 1);
    }

    void bullets()
    {
        QWidget host;
        ShapeTextEdit e(&host);
        e.begin(story(QStringLiteral("one")), QRect(0, 0, 200, 100), ShapeTextLook());
        QVERIFY(!e.hasBullets());
        e.setBullets(true);
        QVERIFY(e.hasBullets());
        QVERIFY(e.story()->firstBlock().textList());
        QTest::keyClick(&e, Qt::Key_End, Qt::ControlModifier);
        QTest::keyClick(&e, Qt::Key_Return);
        QTest::keyClicks(&e, QStringLiteral("two"));
        QCOMPARE(e.story()->blockCount(), 2);
        QVERIFY(e.story()->lastBlock().textList());
        e.setBullets(false);
        QVERIFY(!e.story()->lastBlock().textList());
    }

    void copyAndPasteKeepTheLook()
    {
        QWidget host;
        ShapeTextEdit a(&host), b(&host);
        a.begin(story(), QRect(0, 0, 200, 100), ShapeTextLook());
        jptest::typeAndFormat(&a);
        a.selectAll();
        a.copy();
        b.begin(story(), QRect(0, 0, 200, 100), ShapeTextLook());
        b.paste();
        jptest::verifyEditor(&b);
        // plain text, too
        QApplication::clipboard()->setText(QStringLiteral("plain"));
        b.selectAll();
        b.paste();
        QCOMPARE(b.toPlainText(), QStringLiteral("plain"));
    }

    void cutRemovesTheWords()
    {
        QWidget host;
        ShapeTextEdit e(&host);
        e.begin(story(QStringLiteral("abc def")), QRect(0, 0, 200, 100), ShapeTextLook());
        e.selectAll();
        e.cut();
        QCOMPARE(e.toPlainText(), QString());
        QCOMPARE(QApplication::clipboard()->text(), QStringLiteral("abc def"));
    }

    void undoInTheEditor()
    {
        QWidget host;
        ShapeTextEdit e(&host);
        e.begin(story(), QRect(0, 0, 200, 100), ShapeTextLook());
        QTest::keyClicks(&e, QStringLiteral("typed"));
        jptest::pressStandard(&e, QKeySequence::Undo);
        QCOMPARE(e.toPlainText(), QString());
        jptest::pressStandard(&e, QKeySequence::Redo);
        QCOMPARE(e.toPlainText(), QStringLiteral("typed"));
    }

    void inputMethodCommit()
    {
        QWidget host;
        ShapeTextEdit e(&host);
        e.begin(story(), QRect(0, 0, 200, 100), ShapeTextLook());
        QInputMethodEvent ev;
        ev.setCommitString(QStringLiteral("日本"));
        QApplication::sendEvent(&e, &ev);
        QCOMPARE(e.toPlainText(), QStringLiteral("日本"));
    }

    void spellingMarks()
    {
        QWidget host;
        ShapeTextEdit e(&host);
        e.setSpellCheck([](QTextDocument *d) {
            QVector<QPair<int, int>> out;
            const QString p = d->toPlainText();
            qsizetype from = 0;
            for (qsizetype at; (at = p.indexOf(QStringLiteral("teh"), from)) >= 0; from = at + 3) out << qMakePair(int(at), int(at) + 3);
            return out;
        });
        e.begin(story(QStringLiteral("teh cat")), QRect(0, 0, 200, 100), ShapeTextLook());
        QCOMPARE(e.spellMarks(), 1);
        QTest::keyClick(&e, Qt::Key_End, Qt::ControlModifier);
        QTest::keyClicks(&e, QStringLiteral(" teh"));
        QCOMPARE(e.spellMarks(), 2);
    }

    void anchoringMovesTheWords()
    {
        QWidget host;
        ShapeTextLook look;
        ShapeTextEdit top(&host), mid(&host), bottom(&host);
        look.anchor = Qt::AlignTop;
        top.begin(story(QStringLiteral("abc")), QRect(0, 0, 200, 200), look);
        look.anchor = Qt::AlignVCenter;
        mid.begin(story(QStringLiteral("abc")), QRect(0, 0, 200, 200), look);
        look.anchor = Qt::AlignBottom;
        bottom.begin(story(QStringLiteral("abc")), QRect(0, 0, 200, 200), look);
        auto at = [](ShapeTextEdit &e) { return e.viewport()->mapTo(&e, e.cursorRect().topLeft()).y(); };
        const int t = at(top), m = at(mid), b = at(bottom);
        QVERIFY(t < m);
        QVERIFY(m < b);
    }

    void zoomShowsLargerAndTheStoryKeepsItsPoints()
    {
        QWidget host;
        auto d = story(QStringLiteral("abc"));
        QTextCursor c(d.get());
        c.select(QTextCursor::Document);
        QTextCharFormat f;
        f.setFontPointSize(10);
        c.mergeCharFormat(f);
        ShapeTextLook look;
        look.zoom = 2;
        ShapeTextEdit e(&host);
        e.begin(std::move(d), QRect(0, 0, 200, 100), look);
        QCOMPARE(e.story()->firstBlock().begin().fragment().charFormat().fontPointSize(), 20.0);
        QCOMPARE(e.fontSizePt(), 10.0);
        e.setFontSizePt(14);
        QCOMPARE(e.story()->firstBlock().begin().fragment().charFormat().fontPointSize() > 0, true);
        auto back = e.take();
        QCOMPARE(back->firstBlock().begin().fragment().charFormat().fontPointSize(), 10.0);
    }

    // Typing into an empty paragraph whose end mark says 24 points, at 200% and 133%: it reads back as 24 points, the paragraph's mark too, and a size the story had keeps its exact digits.
    void anEmptyParagraphAndEverySizeComeBackExactlyFromAZoom()
    {
        for (double zoom : {2.0, 1.33, 1.37, 0.7}) {
            QWidget host;
            auto d = story();
            QTextCursor c(d.get());
            QTextCharFormat f;
            f.setFontPointSize(24);
            c.setBlockCharFormat(f);
            ShapeTextLook look;
            look.zoom = zoom;
            ShapeTextEdit e(&host);
            e.begin(std::move(d), QRect(0, 0, 300, 200), look);
            QCOMPARE(e.fontSizePt(), 24.0);
            QTest::keyClicks(&e, QStringLiteral("x"));
            auto back = e.take();
            QCOMPARE(back->firstBlock().begin().fragment().charFormat().fontPointSize(), 24.0);
            QCOMPARE(back->firstBlock().charFormat().fontPointSize(), 24.0);
        }
        QWidget host;
        auto d = story(QStringLiteral("abc"));
        QTextCursor c(d.get());
        c.select(QTextCursor::Document);
        QTextCharFormat f;
        f.setFontPointSize(10.35);
        c.mergeCharFormat(f);
        ShapeTextLook look;
        look.zoom = 1.37;
        ShapeTextEdit e(&host);
        e.begin(std::move(d), QRect(0, 0, 300, 200), look);
        auto back = e.take();
        QCOMPARE(back->firstBlock().begin().fragment().charFormat().fontPointSize(), 10.35);
    }

    void noWrapKeepsOneLine()
    {
        QWidget host;
        ShapeTextLook look;
        look.wrap = false;
        ShapeTextEdit e(&host);
        e.begin(story(QStringLiteral("a very long line that would wrap in a narrow frame")), QRect(0, 0, 60, 100), look);
        QCOMPARE(e.lineWrapMode(), QTextEdit::NoWrap);
        ShapeTextEdit w(&host);
        w.begin(story(QStringLiteral("x")), QRect(0, 0, 60, 100), ShapeTextLook());
        QCOMPARE(w.lineWrapMode(), QTextEdit::WidgetWidth);
    }
};

JP_TEST_CLASS(ShellShapeTextTests)
#include "tst_shell_shapetext.moc"
