// JeffDeck's tests of the Text Highlight Color command (the menu and button, the armed highlighter, what is kept in each file format, and how it is drawn).
#include "app/highlightmenu.h"
#include "../minitoolbartest.h"
#include "deckhelpers.h"
#include "app/theme.h"
#include "text/textengine.h"

#include <QMenu>
#include <QTemporaryDir>

class DeckHighlightTests : public QObject {
    Q_OBJECT

    static TextItem *title(DeckWindow &w) { return static_cast<TextItem *>(w.editor()->doc()->pages[0]->items[0].get()); }

    // The highlight of the run that holds `word` in a story (empty when it has none).
    static QString highlightOf(const QTextDocument *story, const QString &word)
    {
        const int at = story->toPlainText().indexOf(word);
        if (at < 0) return QStringLiteral("(missing)");
        QTextCursor c(const_cast<QTextDocument *>(story));
        c.setPosition(at + 1);
        return c.charFormat().stringProperty(tp::HighlightRefP);
    }

    static QAction *menuAction(QMenu *m, const QString &text)
    {
        for (QAction *a : m->actions())
            if (a->text() == text) return a;
        return nullptr;
    }

    static int count(const QImage &img, const std::function<bool(const QColor &)> &test, QPointF *centroid = nullptr)
    {
        int n = 0;
        double sx = 0, sy = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (test(img.pixelColor(x, y))) {
                    ++n;
                    sx += x;
                    sy += y;
                }
        if (centroid && n) *centroid = QPointF(sx / n, sy / n);
        return n;
    }
    static bool yellowish(const QColor &c) { return c.red() > 200 && c.green() > 200 && c.blue() < 90; }
    static bool bluish(const QColor &c) { return c.blue() > 150 && c.red() < 90 && c.green() < 90; }

    static QImage slide(Document &doc)
    {
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = &doc;
        ctx.cache = &cache;
        return Renderer::renderToImage(ctx, 0, 1.0);
    }
    static void bigLetters(QTextDocument *story)
    {
        QTextCursor c(story);
        c.select(QTextCursor::Document);
        QTextCharFormat f;
        f.setFontPointSize(60);
        c.mergeCharFormat(f);
    }

    // A slide whose title reads "AAAA BBBB" with one word highlighted yellow.
    static QImage drawn(DeckWindow &w, const QString &word, const std::function<void(TextItem *)> &setUp = nullptr, const QString &text = QStringLiteral("AAAA BBBB"))
    {
        Editor *ed = w.editor();
        TextItem *t = title(w);
        QTextDocument *story = ed->doc()->storyDoc(t->storyId);
        story->clear();
        QTextCursor(story).insertText(text);
        bigLetters(story);
        if (setUp) setUp(t);
        if (!word.isEmpty()) {
            const int at = story->toPlainText().indexOf(word);
            QTextCursor c(story);
            c.setPosition(at);
            c.setPosition(at + word.size(), QTextCursor::KeepAnchor);
            QTextCharFormat f;
            f.setProperty(tp::HighlightRefP, ColorRef::rgb(QColor(Qt::yellow)).toString());
            c.mergeCharFormat(f);
        }
        return slide(*ed->doc());
    }

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // The button marks the selected words with the last color in one step of Undo, the menu picks another, No Color takes it off, and with
    // nothing selected the button arms the highlighter instead.
    void theButtonAndMenuMarkTheSelectedWordsInOneStep()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        editTitle(w, QStringLiteral("AAAA BBBB"), 0, 4);
        auto story = [&] { return ed->doc()->storyDoc(title(w)->storyId); };   // (Undo gives the text a new document)
        QAction *button = w.act(QStringLiteral("fmt.highlight"));
        QVERIFY(button);
        QCOMPARE(button->text(), QStringLiteral("Text Highlight Color"));
        auto pick = [&](int from, int to) {   // (Undo ends the editing of the text, so each pick begins it again when it must)
            if (!ed->isEditingText()) ed->beginTextEdit(title(w)->id, from);
            QTextCursor c = ed->cursor();
            c.setPosition(from);
            c.setPosition(to, QTextCursor::KeepAnchor);
            ed->setCursor(c);
        };
        const int steps = ed->undoStack()->count();
        button->trigger();
        QCOMPARE(highlightOf(story(), QStringLiteral("AAAA")), ColorRef::rgb(QColor(Qt::yellow)).toString());
        QVERIFY(highlightOf(story(), QStringLiteral("BBBB")).isEmpty());
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QVERIFY(!ed->highlighterArmed());
        ed->undoStack()->undo();
        QVERIFY(highlightOf(story(), QStringLiteral("AAAA")).isEmpty());
        // The menu: sixteen colors, No Color, Stop Highlighting; a color of it goes on at once and is the one the button uses next.
        QMenu *menu = highlightMenu(&w, ed, w.act(QStringLiteral("fmt.stopHighlighting")));
        QCOMPARE(menu->actions().size(), 16 + 1 + 1 + 1);
        QAction *pink = menuAction(menu, QStringLiteral("Pink"));
        QVERIFY(pink);
        pick(0, 4);
        pink->trigger();
        QCOMPARE(ed->undoStack()->count(), steps + 1);   // (the one undone step is gone, this is the next)
        const QString pinkRef = ColorRef::rgb(QColor(Qt::magenta)).toString();
        QCOMPARE(highlightOf(story(), QStringLiteral("AAAA")), pinkRef);
        QCOMPARE(ed->lastHighlight().toString(), pinkRef);
        QVERIFY(menuAction(menu, QStringLiteral("White")));
        // The word's other half: the button now uses pink.
        pick(5, 9);
        button->trigger();
        QCOMPARE(highlightOf(story(), QStringLiteral("BBBB")), pinkRef);
        // No Color, on the whole text.
        pick(0, 9);
        QAction *none = menuAction(menu, QStringLiteral("No Color"));
        QVERIFY(none);
        none->trigger();
        QVERIFY(highlightOf(story(), QStringLiteral("AAAA")).isEmpty() && highlightOf(story(), QStringLiteral("BBBB")).isEmpty());
        // Nothing selected: the highlighter is armed (and Stop Highlighting is there to stop it).
        pick(2, 2);
        QAction *stop = w.act(QStringLiteral("fmt.stopHighlighting"));
        QVERIFY(!stop->isEnabled());
        button->trigger();
        QVERIFY(ed->highlighterArmed());
        QVERIFY(stop->isEnabled());
        QVERIFY(highlightOf(story(), QStringLiteral("AAAA")).isEmpty());
        stop->trigger();
        QVERIFY(!ed->highlighterArmed() && !stop->isEnabled());
        delete menu;
    }

    // With an object selected and no text being edited, all of its words are marked.
    void aSelectedObjectsWordsAreAllMarked()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        QTextDocument *story = editTitle(w, QStringLiteral("AAAA BBBB"), 0, 0);
        ed->endTextEdit();
        ed->select(title(w)->id);
        QVERIFY(!ed->isEditingText());
        w.act(QStringLiteral("fmt.highlight"))->trigger();
        const QString yellow = ColorRef::rgb(QColor(Qt::yellow)).toString();
        QCOMPARE(highlightOf(story, QStringLiteral("AAAA")), yellow);
        QCOMPARE(highlightOf(story, QStringLiteral("BBBB")), yellow);
        QVERIFY(!ed->highlighterArmed());
    }

    // The armed highlighter marks each run of words selected with a real drag, goes on until Esc, and Esc is the only thing that stops it.
    void theArmedHighlighterMarksWhatTheMouseSelectsUntilEsc()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        const auto *box = title(w);
        QTextCursor(ed->doc()->storyDoc(box->storyId)).insertText(minitest::kText);
        minitest::Rig<DeckWindow> rig(w, box->id, box->storyId, 0);
        QTextDocument *story = ed->doc()->storyDoc(box->storyId);
        ed->beginTextEdit(box->id, 0);
        QTextCursor c = ed->cursor();
        c.setPosition(0);
        ed->setCursor(c);
        QMenu *menu = highlightMenu(&w, ed, w.act(QStringLiteral("fmt.stopHighlighting")));
        menuAction(menu, QStringLiteral("Turquoise"))->trigger();
        QVERIFY(ed->highlighterArmed());
        auto drag = [&](int from, int to) {
            QTest::mousePress(rig.view, Qt::LeftButton, Qt::NoModifier, rig.inside(from));
            QTest::mouseMove(rig.view, rig.at((from + to) / 2));
            QTest::mouseMove(rig.view, rig.inside(to));
            QTest::mouseRelease(rig.view, Qt::LeftButton, Qt::NoModifier, rig.inside(to));
        };
        const QString cyan = ColorRef::rgb(QColor(Qt::cyan)).toString();
        drag(5, 10);   // "words"
        QVERIFY(ed->highlighterArmed());
        QTRY_COMPARE(highlightOf(story, QStringLiteral("words")), cyan);
        QVERIFY(highlightOf(story, QStringLiteral("Some")).isEmpty());
        drag(14, 18);   // "pick"
        QTRY_COMPARE(highlightOf(story, QStringLiteral("pick")), cyan);
        QVERIFY(ed->highlighterArmed());
        QTest::keyClick(rig.view, Qt::Key_Escape);
        QVERIFY(!ed->highlighterArmed());
        drag(19, 22);   // "out"
        QTRY_VERIFY(ed->cursor().hasSelection());
        QVERIFY(highlightOf(story, QStringLiteral("out")).isEmpty());
        QVERIFY(!(QGuiApplication::mouseButtons() & Qt::LeftButton));
        delete menu;
        ed->endTextEdit();
    }

    // Clear All Formatting takes it off; Copy Formatting and Paste Formatting carry it.
    void clearFormattingTakesItOffAndCopyFormattingCarriesIt()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        QTextDocument *story = editTitle(w, QStringLiteral("AAAA BBBB"), 0, 4);
        const QString yellow = ColorRef::rgb(QColor(Qt::yellow)).toString();
        w.act(QStringLiteral("fmt.highlight"))->trigger();
        QCOMPARE(highlightOf(story, QStringLiteral("AAAA")), yellow);
        // Copy the look of the marked word, paste it on the other.
        ed->copyFormatting();
        QTextCursor c = ed->cursor();
        c.setPosition(5);
        c.setPosition(9, QTextCursor::KeepAnchor);
        ed->setCursor(c);
        QVERIFY(highlightOf(story, QStringLiteral("BBBB")).isEmpty());
        ed->pasteFormatting();
        QCOMPARE(highlightOf(story, QStringLiteral("BBBB")), yellow);
        // Clear All Formatting on the whole text.
        c.setPosition(0);
        c.setPosition(9, QTextCursor::KeepAnchor);
        ed->setCursor(c);
        QTRY_VERIFY(w.act(QStringLiteral("fmt.clear"))->isEnabled());
        w.act(QStringLiteral("fmt.clear"))->trigger();
        QVERIFY(highlightOf(story, QStringLiteral("AAAA")).isEmpty());
        QVERIFY(highlightOf(story, QStringLiteral("BBBB")).isEmpty());
    }

    // The color is kept as the file has it, in a .pptx and an .odp saved and opened through the program's own paths: a color of the standard
    // palette and one outside it, the second word left alone.
    void theColorIsSavedAndOpenedInEveryFormat()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QColor odd(0x12, 0xAB, 0x89);
        for (const QString &ending : {QStringLiteral("pptx"), QStringLiteral("odp")}) {
            const QString path = dir.filePath(QStringLiteral("marked.") + ending);
            {
                DeckWindow w;
                QTextDocument *story = editTitle(w, QStringLiteral("AAAA BBBB CCCC"), 0, 4);
                w.act(QStringLiteral("fmt.highlight"))->trigger();
                QTextCursor c = w.editor()->cursor();
                c.setPosition(5);
                c.setPosition(9, QTextCursor::KeepAnchor);
                w.editor()->setCursor(c);
                w.editor()->setTextHighlight(ColorRef::rgb(odd));
                QCOMPARE(highlightOf(story, QStringLiteral("BBBB")), ColorRef::rgb(odd).toString());
                w.editor()->endTextEdit();
                QVERIFY2(w.saveTo(path), qPrintable(path));
                w.editor()->markSaved();
            }
            DeckWindow again;
            QVERIFY2(again.openFile(path), qPrintable(path));
            const auto *read = static_cast<const TextItem *>(again.editor()->doc()->pages[0]->items[0].get());
            const QTextDocument *story = again.editor()->doc()->storyDoc(read->storyId);
            QVERIFY(story);
            auto resolved = [&](const QString &word) { return highlightOf(story, word).isEmpty() ? QColor() : ColorRef::fromString(highlightOf(story, word)).resolve(again.editor()->doc()->colors); };
            QCOMPARE(resolved(QStringLiteral("AAAA")), QColor(Qt::yellow));
            QCOMPARE(resolved(QStringLiteral("BBBB")), odd);
            QVERIFY2(!resolved(QStringLiteral("CCCC")).isValid(), qPrintable(ending));
            again.editor()->markSaved();
        }
    }

    // The mark is drawn behind the words on the slide (the picture every thumbnail, the show, the printer, and the PDF are made from), the PDF has it,
    // and a text outline and a shadow are still drawn over it.
    void theMarkIsDrawnBehindTheWordsAndEffectsStayOnTop()
    {
        DeckWindow w;
        const QImage plain = drawn(w, QString());
        const QImage marked = drawn(w, QStringLiteral("BBBB"));
        QPointF where;
        QCOMPARE(count(plain, yellowish), 0);
        QVERIFY2(count(marked, yellowish, &where) > 200, qPrintable(QString::number(count(marked, yellowish))));
        // The PDF made from the same slide has the yellow too.
        QString err;
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("o.pdf"));
        QVERIFY2(deck::exportPdf(*w.editor()->doc(), path, deck::OutputOptions(), &err), qPrintable(err));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const PdfDocument pdf(f.readAll());
        const QSize px = pdf.pageSize(0).toSize();
        QImage page(px, QImage::Format_RGB32);
        page.fill(Qt::white);
        {
            QPainter on(&page);
            on.drawImage(0, 0, pdf.render(0, px));
        }
        QVERIFY(count(page, yellowish) > 200);
        // An outline in blue and a shadow: drawn over the mark (the blue letters are as many as without the mark, within a margin).
        auto blueOutline = [&](bool mark) {
            Editor *ed = w.editor();
            QTextDocument *story = ed->doc()->storyDoc(title(w)->storyId);
            story->clear();
            QTextCursor(story).insertText(QStringLiteral("BBBB"));
            bigLetters(story);
            QTextCursor c(story);
            c.select(QTextCursor::Document);
            QTextCharFormat fmt;
            fmt.setProperty(tp::OutlineRef, ColorRef::rgb(QColor(0, 0, 255)).toString());
            fmt.setProperty(tp::OutlineWidth, 2.0);
            if (mark) fmt.setProperty(tp::HighlightRefP, ColorRef::rgb(QColor(Qt::yellow)).toString());
            c.mergeCharFormat(fmt);
            return slide(*ed->doc());
        };
        const int blueWithout = count(blueOutline(false), bluish);
        const QImage both = blueOutline(true);
        QVERIFY(blueWithout > 20);
        QVERIFY2(count(both, bluish) * 10 > blueWithout * 7, qPrintable(QString::number(count(both, bluish)) + QStringLiteral(" of ") + QString::number(blueWithout)));
        QVERIFY(count(both, yellowish) > 100);
    }

    // The words that are marked are the right ones in right-to-left text and in text turned a quarter (the first word of the line is on the right, or
    // at the top or bottom).
    void theRightWordsAreMarkedInRightToLeftAndTurnedText()
    {
        DeckWindow w;
        QPointF first, second;
        // Left to right: the first word is on the left.
        QVERIFY(count(drawn(w, QStringLiteral("AAAA")), yellowish, &first) > 100);
        QVERIFY(count(drawn(w, QStringLiteral("BBBB")), yellowish, &second) > 100);
        QVERIFY(first.x() < second.x());
        // Right to left: the first word is on the right.
        const QString hebrewFirst = QString::fromUtf8("\xd7\x90\xd7\x90\xd7\x90\xd7\x90"), hebrewSecond = QString::fromUtf8("\xd7\x91\xd7\x91\xd7\x91\xd7\x91");
        const QString hebrew = hebrewFirst + QLatin1Char(' ') + hebrewSecond;
        const std::function<void(TextItem *)> setRtl = [&](TextItem *t) {
            QTextDocument *story = w.editor()->doc()->storyDoc(t->storyId);
            QTextBlockFormat bf;
            bf.setLayoutDirection(Qt::RightToLeft);
            QTextCursor c(story);
            c.select(QTextCursor::Document);
            c.mergeBlockFormat(bf);
        };
        QVERIFY(count(drawn(w, hebrewFirst, setRtl, hebrew), yellowish, &first) > 100);
        QVERIFY(count(drawn(w, hebrewSecond, setRtl, hebrew), yellowish, &second) > 100);
        QVERIFY2(first.x() > second.x(), qPrintable(QString::number(first.x()) + QStringLiteral(" ") + QString::number(second.x())));
        // Turned a quarter clockwise the line runs down the box: the first word is above the second; turned the other way, below.
        const std::function<void(TextItem *)> down = [&](TextItem *t) {
            t->setDirection(TextDirection::Rotate90);
            QTextCursor c(w.editor()->doc()->storyDoc(t->storyId));   // (small letters: both words fit on one line down the box)
            c.select(QTextCursor::Document);
            QTextCharFormat small;
            small.setFontPointSize(16);
            c.mergeCharFormat(small);
        };
        const std::function<void(TextItem *)> up = [&](TextItem *t) {
            t->setDirection(TextDirection::Rotate270);
            QTextCursor c(w.editor()->doc()->storyDoc(t->storyId));   // (small letters: both words fit on one line down the box)
            c.select(QTextCursor::Document);
            QTextCharFormat small;
            small.setFontPointSize(16);
            c.mergeCharFormat(small);
        };
        QVERIFY(count(drawn(w, QStringLiteral("AAAA"), down), yellowish, &first) > 100);
        QVERIFY(count(drawn(w, QStringLiteral("BBBB"), down), yellowish, &second) > 100);
        QVERIFY2(first.y() < second.y(), qPrintable(QString::number(first.y()) + QStringLiteral(" ") + QString::number(second.y())));
        QVERIFY(count(drawn(w, QStringLiteral("AAAA"), up), yellowish, &first) > 100);
        QVERIFY(count(drawn(w, QStringLiteral("BBBB"), up), yellowish, &second) > 100);
        QVERIFY2(first.y() > second.y(), qPrintable(QString::number(first.y()) + QStringLiteral(" ") + QString::number(second.y())));
    }

    // Pictures for people to look at, made only when JD_SHOT_DIR names a folder: the Home tab with a highlighted title, and the color menu.
    void screenshotsOfTheCommand()
    {
        const QString dir = qEnvironmentVariable("JD_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JD_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        DeckWindow w;
        w.resize(1300, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        QTextDocument *story = editTitle(w, QStringLiteral("Quarterly results"), 0, 9);
        Q_UNUSED(story)
        w.act(QStringLiteral("fmt.highlight"))->trigger();
        ed->endTextEdit();
        QTRY_VERIFY(w.act(QStringLiteral("fmt.highlight"))->isEnabled());
        QVERIFY(w.grab().save(dir + QStringLiteral("/deck-highlight-window.png")));
        QMenu *menu = highlightMenu(&w, ed, w.act(QStringLiteral("fmt.stopHighlighting")));
        menu->move(100, 100);
        menu->show();
        QVERIFY(QTest::qWaitForWindowExposed(menu));
        QVERIFY(menu->grab().save(dir + QStringLiteral("/deck-highlight-menu.png")));
        delete menu;
    }
};

JP_TEST_CLASS(DeckHighlightTests)
#include "tst_deck_highlight.moc"
