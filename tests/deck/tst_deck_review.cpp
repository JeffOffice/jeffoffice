// JeffDeck's tests of spelling, comments, the options, and telemetry.
#include "deckhelpers.h"

class DeckReviewTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // Review > Spelling (F7) goes through the slides to a misspelled word and
    // selects it; Thesaurus is Shift+F7.
    void spellingChecksTheSlides()
    {
        DeckWindow w;
        QCOMPARE(w.act(QStringLiteral("review.spelling"))->shortcut(), QKeySequence(Qt::Key_F7));
        QCOMPARE(w.act(QStringLiteral("review.thesaurus"))->shortcut(), QKeySequence(Qt::SHIFT | Qt::Key_F7));
        Editor *ed = w.editor();
        auto *title = static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get());
        QTextCursor(ed->doc()->storyDoc(title->storyId)).insertText(QStringLiteral("A mispelled word"));
        QString shown, selected;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            for (QLabel *l : d->findChildren<QLabel *>())
                if (l->text().startsWith(QLatin1String("<b>"))) shown = l->text();
            selected = ed->cursor().selectedText();
            d->reject();
        });
        w.act(QStringLiteral("review.spelling"))->trigger();
        QVERIFY2(shown.contains(QStringLiteral("mispelled")), qPrintable(shown));
        QCOMPARE(selected, QStringLiteral("mispelled"));
        QVERIFY(ed->isEditingText());
    }

    // Word Count counts the words on the slides, not the prompts of the layouts.
    void wordCountCountsTheSlidesWords()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        QTextCursor(ed->doc()->storyDoc(static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get())->storyId)).insertText(QStringLiteral("One two three"));
        QTextCursor(ed->doc()->storyDoc(static_cast<TextItem *>(ed->doc()->pages[0]->items[1].get())->storyId)).insertText(QStringLiteral("Four five"));
        QString shown;
        QTimer::singleShot(0, [&] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                shown = box->text();
                box->close();
            }
        });
        w.act(QStringLiteral("review.wordCount"))->trigger();
        QVERIFY2(shown.contains(QStringLiteral("Words: 5")), qPrintable(shown));
    }

    // Insert > Symbol puts the character chosen in the dialog into the
    // placeholder being typed in; Date & Time puts in the format chosen.
    void symbolAndDateGoIntoThePlaceholderBeingEdited()
    {
        DeckWindow w;
        QTextDocument *story = editTitle(w, QStringLiteral("ab"), 1, 1);
        QString title;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            title = d->windowTitle();
            QTableWidget *grid = nullptr;
            for (QTableWidget *t : d->findChildren<QTableWidget *>())
                if (t->rowCount() > 1) grid = t;
            QPushButton *insert = nullptr;
            for (QPushButton *b : d->findChildren<QPushButton *>())
                if (b->text() == QLatin1String("Insert")) insert = b;
            if (grid && insert) {
                grid->setCurrentCell(2, 1);   // Basic Latin from U+0020: 33 on is "A"
                insert->click();
            }
            d->accept();
        });
        w.act(QStringLiteral("insert.symbol"))->trigger();
        QCOMPARE(title, QStringLiteral("Symbol"));
        QCOMPARE(story->toPlainText(), QStringLiteral("aAb"));
        // Date & Time: the first format in the list.
        QString shown;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            if (auto *list = d->findChild<QListWidget *>()) shown = list->currentItem()->text();
            d->accept();
        });
        w.act(QStringLiteral("insert.dateTime"))->trigger();
        QVERIFY(!shown.isEmpty());
        QCOMPARE(story->toPlainText(), QStringLiteral("aA") + shown + QStringLiteral("b"));
    }

    // Insert > Link (Ctrl+K) makes the selected text a link to the address typed.
    void hyperlinkLinksTheSelectedText()
    {
        DeckWindow w;
        QCOMPARE(w.act(QStringLiteral("insert.link"))->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_K));
        QTextDocument *story = editTitle(w, QStringLiteral("Visit our site"), 6, 9);
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            for (QLabel *l : d->findChildren<QLabel *>())
                if (l->text() == QLatin1String("Address:") && l->buddy()) static_cast<QLineEdit *>(l->buddy())->setText(QStringLiteral("example.com"));
            d->accept();
        });
        w.act(QStringLiteral("insert.link"))->trigger();
        QTextCursor c(story);
        c.setPosition(7);
        QCOMPARE(c.charFormat().anchorHref(), QStringLiteral("https://example.com"));
        c.setPosition(2);
        QVERIFY(c.charFormat().anchorHref().isEmpty());
    }

    // The ribbon has a Review tab after Slide Show, with the proofing commands,
    // and the Insert tab has the symbol, link, and date commands, each KeyTip its own.
    void reviewTabAndInsertDialogs()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QStringList tabs;
        for (int i = 0; i < w.ribbon()->tabCount(); ++i) tabs << w.ribbon()->tabName(i);
        QCOMPARE(tabs.indexOf(QStringLiteral("Review")), tabs.indexOf(QStringLiteral("Slide Show")) + 1);
        for (const char *id : {"review.spelling", "review.thesaurus", "review.wordCount", "insert.symbol", "insert.link", "insert.dateTime", "fmt.fontDialog", "para.dialog", "fmt.spacingDialog",
                               "para.bulletsDialog", "para.numberingDialog", "file.options"})
            QVERIFY2(w.act(QString::fromLatin1(id)), id);
        for (const QString &name : {QStringLiteral("Home"), QStringLiteral("Insert"), QStringLiteral("Review")}) {
            QVERIFY2(w.ribbon()->tab(name), qPrintable(name));
            QStringList seen;
            for (const QWidget *c : w.ribbon()->tab(name)->findChildren<QWidget *>()) {
                const QString k = keytip(c);
                if (k.isEmpty()) continue;
                QVERIFY2(!seen.contains(k), qPrintable(name + ": two controls have the KeyTip " + k));
                seen << k;
            }
        }
        // Each is in its tab.
        auto has = [&](const QString &tab, const QString &id) {
            for (const QToolButton *b : w.ribbon()->tab(tab)->findChildren<QToolButton *>())
                if (b->defaultAction() == w.act(id)) return true;
            return false;
        };
        QVERIFY(has(QStringLiteral("Review"), QStringLiteral("review.spelling")));
        QVERIFY(has(QStringLiteral("Review"), QStringLiteral("review.thesaurus")));
        QVERIFY(has(QStringLiteral("Review"), QStringLiteral("review.wordCount")));
        QVERIFY(has(QStringLiteral("Insert"), QStringLiteral("insert.symbol")));
        QVERIFY(has(QStringLiteral("Insert"), QStringLiteral("insert.link")));
        QVERIFY(has(QStringLiteral("Insert"), QStringLiteral("insert.dateTime")));
    }

    // A slide's comments live in its page props, so undo covers them: add,
    // reply, edit, resolve, move, and delete; the author comes from File > Options.
    void commentModelKeepsCommentsInThePageProps()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        Page &slide = *d->pages[0];
        QVERIFY(deck::commentsOf(slide).isEmpty());
        QVERIFY(!slide.props.contains(QStringLiteral("comments")));
        const deck::CommentAuthor pat{QStringLiteral("Pat Doe"), QStringLiteral("PD")}, lee{QStringLiteral("Lee Roy"), QString()};
        const QDateTime noon(QDate(2026, 10, 12), QTime(12, 0, 5, 125), QTimeZone::UTC);
        const QString first = deck::addComment(slide, pat, QStringLiteral("Check\r\nthe numbers"), QPointF(100.5, 40.25), noon);
        QVERIFY(!first.isEmpty());
        // As the props hold it.
        const QJsonObject o = slide.props.value(QStringLiteral("comments")).toArray().at(0).toObject();
        QCOMPARE(o.value(QStringLiteral("id")).toString(), first);
        QCOMPARE(o.value(QStringLiteral("author")).toString(), QStringLiteral("Pat Doe"));
        QCOMPARE(o.value(QStringLiteral("initials")).toString(), QStringLiteral("PD"));
        QCOMPARE(o.value(QStringLiteral("date")).toString(), QStringLiteral("2026-10-12T12:00:05.125Z"));
        QCOMPARE(o.value(QStringLiteral("text")).toString(), QStringLiteral("Check\nthe numbers"));
        QCOMPARE(o.value(QStringLiteral("x")).toDouble(), 100.5);
        QCOMPARE(o.value(QStringLiteral("y")).toDouble(), 40.25);
        QVERIFY(o.value(QStringLiteral("replies")).isArray() && o.value(QStringLiteral("replies")).toArray().isEmpty());
        QCOMPARE(o.value(QStringLiteral("done")).toBool(true), false);

        // Replies belong to a comment; initials left out are made from the name.
        QVERIFY(deck::addReply(slide, QStringLiteral("nope"), pat, QStringLiteral("x")).isEmpty());
        const QString answer = deck::addReply(slide, first, lee, QStringLiteral("Agreed"), noon.addSecs(90));
        QVERIFY(!answer.isEmpty() && answer != first);
        const QString second = deck::addComment(slide, lee, QStringLiteral("Second"), QPointF(300, 200), noon.addSecs(120));
        QVector<deck::Comment> list = deck::commentsOf(slide);
        QCOMPARE(int(list.size()), 2);
        QCOMPARE(list[0].id, first);
        QCOMPARE(list[1].id, second);
        QCOMPARE(int(list[0].replies.size()), 1);
        QCOMPARE(list[0].replies[0].author, QStringLiteral("Lee Roy"));
        QCOMPARE(list[0].replies[0].initials, QStringLiteral("LR"));
        QCOMPARE(list[0].replies[0].date, noon.addSecs(90));
        QCOMPARE(list[0].replies[0].text, QStringLiteral("Agreed"));
        QCOMPARE(list[0].date, noon);
        QCOMPARE(list[0].at, QPointF(100.5, 40.25));
        QCOMPARE(deck::commentCount(slide), 2);
        QVERIFY(deck::findComment(slide, second));
        QVERIFY(!deck::findComment(slide, answer));   // a reply is not a comment
        QVERIFY(!deck::findComment(slide, QStringLiteral("nope")));

        // Edit (a comment or a reply), resolve and reopen, and move.
        QVERIFY(deck::editComment(slide, first, QStringLiteral("Check again")));
        QVERIFY(deck::editComment(slide, answer, QStringLiteral("Agreed, doing it")));
        QVERIFY(!deck::editComment(slide, QStringLiteral("nope"), QStringLiteral("x")));
        QVERIFY(deck::resolveComment(slide, first));
        QVERIFY(!deck::resolveComment(slide, QStringLiteral("nope")));
        QVERIFY(deck::moveComment(slide, second, QPointF(12, 34)));
        list = deck::commentsOf(slide);
        QCOMPARE(list[0].text, QStringLiteral("Check again"));
        QCOMPARE(list[0].replies[0].text, QStringLiteral("Agreed, doing it"));
        QVERIFY(list[0].done && !list[1].done);
        QCOMPARE(list[1].at, QPointF(12, 34));
        QVERIFY(deck::resolveComment(slide, first, false));
        QVERIFY(!deck::commentsOf(slide)[0].done);

        // Delete a reply, then a comment with its replies.
        QVERIFY(deck::deleteComment(slide, answer));
        QVERIFY(deck::commentsOf(slide)[0].replies.isEmpty());
        QVERIFY(!deck::deleteComment(slide, answer));
        deck::addReply(slide, first, lee, QStringLiteral("Again"));
        QVERIFY(deck::deleteComment(slide, first));
        QCOMPARE(int(deck::commentsOf(slide).size()), 1);
        QCOMPARE(deck::commentsOf(slide)[0].id, second);

        // All of a slide's, and all of the presentation's; with none the key is gone.
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        deck::addComment(*d->pages[1], pat, QStringLiteral("On the second slide"), QPointF(10, 10));
        deck::addComment(*d->pages[1], pat, QStringLiteral("Another"), QPointF(30, 10));
        const QVector<deck::CommentRef> all = deck::allComments(*d);
        QCOMPARE(int(all.size()), 3);
        QCOMPARE(all[0].slide, 0);
        QCOMPARE(all[0].id, second);
        QCOMPARE(all[1].slide, 1);
        QCOMPARE(all[2].slide, 1);
        QCOMPARE(deck::deleteComments(*d->pages[1]), 2);
        QVERIFY(!d->pages[1]->props.contains(QStringLiteral("comments")));
        deck::addComment(*d->pages[1], pat, QStringLiteral("Again"), QPointF(10, 10));
        QCOMPARE(deck::deleteAllComments(*d), 2);
        QVERIFY(deck::allComments(*d).isEmpty());
        QVERIFY(!d->pages[0]->props.contains(QStringLiteral("comments")));

        // A comment can't hold more than a page of text.
        deck::addComment(slide, pat, QString(deck::kMaxCommentText + 500, QLatin1Char('x')), QPointF(1, 1));
        QCOMPARE(int(deck::commentsOf(slide)[0].text.size()), int(deck::kMaxCommentText));

        QCOMPARE(deck::initialsFor(QStringLiteral("Pat Doe")), QStringLiteral("PD"));
        QCOMPARE(deck::initialsFor(QStringLiteral("  mary jane watson parker ")), QStringLiteral("MJW"));
        QCOMPARE(deck::initialsFor(QString()), QString());

        // The author is the user of File > Options; initials left empty come from the name.
        Settings &st = Settings::get();
        st.setValue(QStringLiteral("user/name"), QString());
        st.setValue(QStringLiteral("user/initials"), QString());
        QVERIFY(!deck::commentAuthor().known());
        deck::setCommentAuthor({QStringLiteral(" Pat Doe "), QString()});
        QCOMPARE(st.value(QStringLiteral("user/name")).toString(), QStringLiteral("Pat Doe"));
        QCOMPARE(st.value(QStringLiteral("user/initials")).toString(), QStringLiteral("PD"));
        QVERIFY(deck::commentAuthor().known());
        QCOMPARE(deck::commentAuthor().initials, QStringLiteral("PD"));
        deck::setCommentAuthor({QStringLiteral("Lee Roy"), QStringLiteral("LRY")});
        QCOMPARE(deck::commentAuthor().name, QStringLiteral("Lee Roy"));
        QCOMPARE(deck::commentAuthor().initials, QStringLiteral("LRY"));
        st.setValue(QStringLiteral("user/name"), QString());
        st.setValue(QStringLiteral("user/initials"), QString());
    }

    // The older form: the authors in one part of the presentation, each slide's
    // comments in a part of its own, a reply naming the comment it answers in
    // an extension. Times are the author's own, with their offset from UTC.
    void importReadsLegacyComments()
    {
        QString err;
        auto d = deck::importPresentation(pack(withLegacyComments()), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        QCOMPARE(int(d->pages.size()), 2);
        QVERIFY(deck::commentsOf(*d->pages[1]).isEmpty());
        const QVector<deck::Comment> list = deck::commentsOf(*d->pages[0]);
        QCOMPARE(int(list.size()), 2);   // the file has three, one of them an answer
        const deck::Comment &c = list[0];
        QCOMPARE(c.author, QStringLiteral("Pat Doe"));
        QCOMPARE(c.initials, QStringLiteral("PD"));
        QCOMPARE(c.text, QStringLiteral("Check the numbers\non this slide"));
        // 10:15:30.250 where UTC minus local is 420 minutes.
        QCOMPARE(c.date, QDateTime(QDate(2026, 10, 12), QTime(17, 15, 30, 250), QTimeZone::UTC));
        // Positions are 576ths of an inch: 5760 is 10 inches, 720 points.
        QCOMPARE(c.at, QPointF(720, 144));
        QVERIFY(!c.done);
        QCOMPARE(int(c.replies.size()), 1);
        QCOMPARE(c.replies[0].author, QStringLiteral("Lee Roy"));
        QCOMPARE(c.replies[0].initials, QStringLiteral("LR"));
        QCOMPARE(c.replies[0].text, QStringLiteral("Done, thanks"));
        QCOMPARE(c.replies[0].date, QDateTime(QDate(2026, 10, 12), QTime(17, 20, 0, 0), QTimeZone::UTC));
        QVERIFY(!c.replies[0].id.isEmpty() && c.replies[0].id != c.id);
        // A time that says its zone is used as it is; one with no offset known is taken as UTC.
        QCOMPARE(list[1].author, QStringLiteral("Lee Roy"));
        QCOMPARE(list[1].text, QStringLiteral("Second thread"));
        QCOMPARE(list[1].date, QDateTime(QDate(2026, 10, 12), QTime(11, 0, 0, 0), QTimeZone::UTC));
        QCOMPARE(list[1].at, QPointF(100, 50));
        QVERIFY(list[1].replies.isEmpty());
        QVERIFY(list[1].id != c.id);
        // The extension that says a comment is done.
        Parts p = withLegacyComments();
        QString xml = QString::fromUtf8(p.data["/ppt/comments/comment1.xml"].xml);
        xml.replace(QStringLiteral("<p:text>Second thread</p:text>"),
                    QStringLiteral("<p:text>Second thread</p:text><p:extLst><p:ext uri=\"{7C0E3E5B-5A0B-4B52-9D6E-2F1A8E4C9B21}\">"
                                   "<jo:done xmlns:jo=\"http://jeffoffice.com/ns/deck/comments\" val=\"1\"/></p:ext></p:extLst>"));
        p.data["/ppt/comments/comment1.xml"].xml = xml.toUtf8();
        d = deck::importPresentation(pack(p), &err);
        QVERIFY(d);
        QVERIFY(deck::commentsOf(*d->pages[0])[1].done);
        QVERIFY(!deck::commentsOf(*d->pages[0])[0].done);

        // The strict form of the standard names the same parts.
        Parts s = withLegacyComments();
        for (auto it = s.data.begin(); it != s.data.end(); ++it) {
            if (!it.key().endsWith(QLatin1String(".xml"))) continue;
            QString text = QString::fromUtf8(it.value().xml);
            text.replace(QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/main"), QStringLiteral("http://purl.oclc.org/ooxml/drawingml/main"));
            text.replace(QStringLiteral("http://schemas.openxmlformats.org/presentationml/2006/main"), QStringLiteral("http://purl.oclc.org/ooxml/presentationml/main"));
            text.replace(QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships"), QStringLiteral("http://purl.oclc.org/ooxml/officeDocument/relationships"));
            it.value().xml = text.toUtf8();
        }
        d = deck::importPresentation(pack(s), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(deck::commentsOf(*d->pages[0]).size()), 2);
        QCOMPARE(int(deck::commentsOf(*d->pages[0])[0].replies.size()), 1);
    }

    // The newer form: authors by GUID, comments that are threads with replies
    // inside, a status, and a place on the slide or the shape they stand on.
    void importReadsModernComments()
    {
        QString err;
        auto d = deck::importPresentation(pack(withModernComments()), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        QVERIFY(deck::commentsOf(*d->pages[0]).isEmpty());
        const QVector<deck::Comment> list = deck::commentsOf(*d->pages[1]);
        QCOMPARE(int(list.size()), 3);
        const deck::Comment &c = list[0];
        QCOMPARE(c.author, QStringLiteral("Pat Doe"));
        QCOMPARE(c.initials, QStringLiteral("PD"));
        QCOMPARE(c.text, QStringLiteral("Line one\nLine two"));
        QCOMPARE(c.date, QDateTime(QDate(2026, 10, 12), QTime(17, 15, 30, 250), QTimeZone::UTC));
        QVERIFY(c.done);   // status "resolved"
        // The place is in EMU: 9144000 is 720 points.
        QCOMPARE(c.at, QPointF(720, 108));
        QCOMPARE(int(c.replies.size()), 1);
        QCOMPARE(c.replies[0].author, QStringLiteral("Lee Roy"));
        QCOMPARE(c.replies[0].initials, QStringLiteral("LR"));
        QCOMPARE(c.replies[0].text, QStringLiteral("On it"));
        QCOMPARE(c.replies[0].date, QDateTime(QDate(2026, 10, 12), QTime(17, 20, 0, 0), QTimeZone::UTC));
        // One with no status is open; one on a shape stands at the shape's top right (the title, shape 2).
        QCOMPARE(list[1].text, QStringLiteral("Anchored"));
        QVERIFY(!list[1].done);
        const Item *title = d->pages[1]->items[0].get();
        QCOMPARE(title->sourceId, 2);
        QCOMPARE(list[1].at, QPointF(title->rect.right(), title->rect.top()));
        // One that says nowhere stands at the corner, clear of the others.
        QCOMPARE(list[2].text, QStringLiteral("Nowhere"));
        QVERIFY(list[2].at.x() >= 0 && list[2].at.x() < 100 && list[2].at.y() >= 0 && list[2].at.y() < 100);
        QVERIFY(list[2].at != list[1].at && list[2].at != c.at);
    }

    // A comment file is a stranger's: what doesn't make sense is left out or
    // mended, never fatal, and the slides are read all the same.
    void importSurvivesHostileComments()
    {
        QString err;
        // A comments part that isn't XML, or isn't comments at all.
        for (const char *junk : {"<<< not xml", "<p:sld xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"/>"}) {
            Parts p = withLegacyComments();
            p.data["/ppt/comments/comment1.xml"].xml = junk;
            err.clear();
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(d->pages.size()), 2);
            QVERIFY(deck::commentsOf(*d->pages[0]).isEmpty());
            QVERIFY2(err.contains(QStringLiteral("comments")), qPrintable(err));
        }
        // No authors part, or an author the comments don't name: the comments stay.
        {
            Parts p = withLegacyComments();
            p.data.remove(QStringLiteral("/ppt/commentAuthors.xml"));
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            const QVector<deck::Comment> list = deck::commentsOf(*d->pages[0]);
            QCOMPARE(int(list.size()), 2);
            QVERIFY(!list[0].author.isEmpty());
            QCOMPARE(list[0].text, QStringLiteral("Check the numbers\non this slide"));
        }
        // A reply to itself, to a comment that isn't there, and two that answer each other: all stay, none loops.
        {
            Parts p = withLegacyComments();
            QString body;
            body += legacyComment(0, 1, QStringLiteral("2026-10-12T10:00:00"), 80, 80, QStringLiteral("Self"), 0, QStringLiteral("0:1"));
            body += legacyComment(0, 2, QStringLiteral("2026-10-12T10:00:00"), 80, 80, QStringLiteral("Orphan"), 0, QStringLiteral("5:9"));
            body += legacyComment(1, 1, QStringLiteral("2026-10-12T10:00:00"), 80, 80, QStringLiteral("Ping"), 0, QStringLiteral("1:2"));
            body += legacyComment(1, 2, QStringLiteral("2026-10-12T10:00:00"), 80, 80, QStringLiteral("Pong"), 0, QStringLiteral("1:1"));
            p.data["/ppt/comments/comment1.xml"].xml = legacyCommentList(body).toUtf8();
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            const QVector<deck::Comment> list = deck::commentsOf(*d->pages[0]);
            int comments = 0, replies = 0;
            for (const deck::Comment &c : list) {
                ++comments;
                replies += int(c.replies.size());
            }
            QCOMPARE(comments + replies, 4);
            QVERIFY(comments >= 3);
        }
        // Dates and places that aren't numbers, and text of any length.
        {
            Parts p = withLegacyComments();
            QString body = legacyComment(0, 1, QStringLiteral("last Tuesday"), 0, 0, QStringLiteral("No date"));
            body.replace(QStringLiteral("<p:pos x=\"0\" y=\"0\"/>"), QStringLiteral("<p:pos x=\"many\" y=\"-4000000000\"/>"));
            body += legacyComment(0, 2, QStringLiteral("2026-10-12T10:00:00"), 0, 0, QString(deck::kMaxCommentText + 1000, QLatin1Char('y')));
            body += QStringLiteral("<p:cm authorId=\"0\" idx=\"3\"><p:text>No place</p:text></p:cm><p:cm><p:text>No author</p:text></p:cm>");
            p.data["/ppt/comments/comment1.xml"].xml = legacyCommentList(body).toUtf8();
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            const QVector<deck::Comment> list = deck::commentsOf(*d->pages[0]);
            QCOMPARE(int(list.size()), 4);
            QVERIFY(!list[0].date.isValid());
            QCOMPARE(list[0].text, QStringLiteral("No date"));
            QVERIFY(list[0].at.x() >= 0 && list[0].at.x() <= 960 && list[0].at.y() >= 0 && list[0].at.y() <= 540);
            QCOMPARE(int(list[1].text.size()), int(deck::kMaxCommentText));
            QCOMPARE(list[2].text, QStringLiteral("No place"));
        }
        // More than a slide can hold are counted, not read.
        {
            Parts p = withLegacyComments();
            QString body;
            for (int i = 0; i < 1100; ++i) body += legacyComment(0, i + 1, QStringLiteral("2026-10-12T10:00:00"), 80, 80, QStringLiteral("C%1").arg(i));
            p.data["/ppt/comments/comment1.xml"].xml = legacyCommentList(body).toUtf8();
            err.clear();
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(deck::commentsOf(*d->pages[0]).size()), 1000);
            QVERIFY2(err.contains(QStringLiteral("1000")), qPrintable(err));
        }
        // Authors by the thousand are not all read either; the first ones are.
        {
            Parts p = withLegacyComments();
            QString authors;
            for (int i = 0; i < 12000; ++i) authors += QStringLiteral("<p:cmAuthor id=\"%1\" name=\"A%1\" initials=\"A\" lastIdx=\"1\" clrIdx=\"0\"/>").arg(i);
            p.data["/ppt/commentAuthors.xml"].xml = (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:cmAuthorLst ") + nsDecl() + QStringLiteral(">") + authors + QStringLiteral("</p:cmAuthorLst>")).toUtf8();
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(deck::commentsOf(*d->pages[0])[0].author, QStringLiteral("A0"));
        }
        // The newer form: a part that isn't XML, replies without end, text in odd places.
        {
            Parts p = withModernComments();
            p.data["/ppt/comments/modernComment_101_5.xml"].xml = "<<< not xml";
            err.clear();
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(d->pages.size()), 2);
            QVERIFY(deck::commentsOf(*d->pages[1]).isEmpty());
            QVERIFY2(err.contains(QStringLiteral("comments")), qPrintable(err));
            Parts q = withModernComments();
            QString xml = QString::fromUtf8(q.data["/ppt/comments/modernComment_101_5.xml"].xml);
            QString replies;
            for (int i = 0; i < 1100; ++i)
                replies += QStringLiteral("<p188:reply id=\"{%1}\" authorId=\"{22222222-2222-4222-8222-222222222222}\" created=\"bad\"><p188:txBody><a:p><a:r><a:t>r%1</a:t></a:r></a:p></p188:txBody></p188:reply>").arg(i);
            xml.replace(QStringLiteral("<p188:replyLst>"), QStringLiteral("<p188:replyLst>") + replies);
            q.data["/ppt/comments/modernComment_101_5.xml"].xml = xml.toUtf8();
            err.clear();
            d = deck::importPresentation(pack(q), &err);
            QVERIFY(d);
            const QVector<deck::Comment> list = deck::commentsOf(*d->pages[1]);
            QCOMPARE(int(list.size()), 3);
            QCOMPARE(int(list[0].replies.size()), 1000);
            QVERIFY2(err.contains(QStringLiteral("1000")), qPrintable(err));
        }
    }

    // A presentation with no comments has no comment parts; one with comments
    // has the authors once and a part of comments for each slide that has some,
    // in the older form that every version of the program opens.
    void exportWritesCommentsAndReadsThemBack()
    {
        {
            auto plain = deck::newPresentation();
            const QByteArray bytes = deck::exportPresentation(*plain);
            QVERIFY(!bytes.isEmpty());
            Exported e;
            QString why;
            QVERIFY2(unpack(bytes, &e, &why), qPrintable(why));
            for (const QString &n : e.pkg.partNames()) QVERIFY2(!n.contains(QStringLiteral("omment")), qPrintable(n));
            QVERIFY(e.pkg.relationshipsOfType(QStringLiteral("/ppt/presentation.xml"), QStringLiteral("/commentAuthors")).isEmpty());
            QVERIFY(!e.zip.value(QStringLiteral("[Content_Types].xml")).contains("omment"));
        }

        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        const deck::CommentAuthor pat{QStringLiteral("Pat Doe"), QStringLiteral("PD")}, lee{QStringLiteral("Lee Roy"), QStringLiteral("LR")}, odd{QStringLiteral("A <B> & \"C\""), QStringLiteral("A&B")};
        const QDateTime t(QDate(2026, 10, 12), QTime(14, 3, 5, 123), QTimeZone::UTC);
        const QString c1 = deck::addComment(*d->pages[0], pat, QStringLiteral("Check the numbers\non this slide"), QPointF(100.5, 40.25), t);
        deck::addReply(*d->pages[0], c1, lee, QStringLiteral("Done, thanks"), t.addSecs(300));
        deck::addReply(*d->pages[0], c1, pat, QStringLiteral("Great"), t.addSecs(360));
        const QString c2 = deck::addComment(*d->pages[0], lee, QStringLiteral("Second & <last>"), QPointF(700, 300), t.addDays(1));
        deck::resolveComment(*d->pages[0], c2);
        deck::addComment(*d->pages[2], odd, QStringLiteral("Third slide"), QPointF(10, 10), t.addDays(2));

        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QString why;
        QVERIFY2(unpack(bytes, &e, &why), qPrintable(why));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString pres = QStringLiteral("/ppt/presentation.xml");
        QCOMPARE(relationshipTargets(e, pres, QStringLiteral("/commentAuthors")), QStringList{QStringLiteral("/ppt/commentAuthors.xml")});
        QCOMPARE(e.pkg.contentType(QStringLiteral("/ppt/commentAuthors.xml")), QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.commentAuthors+xml"));
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/comments")), QStringList{QStringLiteral("/ppt/comments/comment1.xml")});
        QVERIFY(relationshipTargets(e, QStringLiteral("/ppt/slides/slide2.xml"), QStringLiteral("/comments")).isEmpty());
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/slides/slide3.xml"), QStringLiteral("/comments")), QStringList{QStringLiteral("/ppt/comments/comment2.xml")});
        for (const char *n : {"/ppt/comments/comment1.xml", "/ppt/comments/comment2.xml"})
            QCOMPARE(e.pkg.contentType(QLatin1String(n)), QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.comments+xml"));
        QCOMPARE(int(e.pkg.partNames().filter(QRegularExpression(QStringLiteral("^/ppt/comments/"))).size()), 2);
        // The relationship types are the standard's.
        QCOMPARE(e.pkg.relationshipsOfType(pres, QStringLiteral("/commentAuthors")).at(0).type, QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/commentAuthors"));
        QCOMPARE(e.pkg.relationshipsOfType(QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/comments")).at(0).type, QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/comments"));

        // The authors, once each, with the last index they used; a comment names its author and its index.
        QDomDocument authors;
        const QDomElement list = rootOf(e.zip.value(QStringLiteral("ppt/commentAuthors.xml")), &authors);
        QCOMPARE(list.localName(), QStringLiteral("cmAuthorLst"));
        const QVector<QDomElement> people = office::children(list, office::ns::P, QStringLiteral("cmAuthor"));
        QCOMPARE(int(people.size()), 3);
        QCOMPARE(people[0].attribute(QStringLiteral("name")), QStringLiteral("Pat Doe"));
        QCOMPARE(people[0].attribute(QStringLiteral("initials")), QStringLiteral("PD"));
        QCOMPARE(people[0].attribute(QStringLiteral("id")), QStringLiteral("0"));
        QCOMPARE(people[0].attribute(QStringLiteral("lastIdx")), QStringLiteral("2"));   // the comment and its second reply
        QCOMPARE(people[1].attribute(QStringLiteral("name")), QStringLiteral("Lee Roy"));
        QCOMPARE(people[1].attribute(QStringLiteral("lastIdx")), QStringLiteral("2"));
        QCOMPARE(people[2].attribute(QStringLiteral("name")), QStringLiteral("A <B> & \"C\""));
        QVERIFY(people[2].hasAttribute(QStringLiteral("clrIdx")));
        QDomDocument comments;
        const QDomElement cmLst = rootOf(e.zip.value(QStringLiteral("ppt/comments/comment1.xml")), &comments);
        QCOMPARE(cmLst.localName(), QStringLiteral("cmLst"));
        const QVector<QDomElement> cms = office::children(cmLst, office::ns::P, QStringLiteral("cm"));
        QCOMPARE(int(cms.size()), 4);   // two comments and the two replies to the first
        QSet<QString> seen;
        for (const QDomElement &cm : cms) {
            QStringList names;
            for (const QDomElement &c : office::children(cm)) names << c.localName();
            QVERIFY2(names.size() >= 2 && names[0] == QLatin1String("pos") && names[1] == QLatin1String("text") && (names.size() == 2 || names[2] == QLatin1String("extLst")), qPrintable(names.join(QLatin1Char(','))));
            const QString key = cm.attribute(QStringLiteral("authorId")) + QLatin1Char(':') + cm.attribute(QStringLiteral("idx"));
            QVERIFY2(!seen.contains(key), qPrintable(key));
            seen << key;
            QVERIFY(cm.hasAttribute(QStringLiteral("dt")));
        }
        // Positions in 576ths of an inch (8 to the point).
        const QDomElement pos = office::child(cms[0], office::ns::P, QStringLiteral("pos"));
        QCOMPARE(pos.attribute(QStringLiteral("x")), QStringLiteral("804"));
        QCOMPARE(pos.attribute(QStringLiteral("y")), QStringLiteral("322"));
        // A reply says which comment it answers.
        const QString xml = QString::fromUtf8(e.zip.value(QStringLiteral("ppt/comments/comment1.xml")));
        QCOMPARE(int(xml.count(QStringLiteral("<p15:parentCm authorId=\"0\" idx=\"1\"/>"))), 2);

        // Read back, it is what was written: authors, dates to the millisecond, places, replies, and the done mark.
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        QCOMPARE(int(back->pages.size()), 3);
        for (int i = 0; i < 3; ++i) {
            const QVector<deck::Comment> want = deck::commentsOf(*d->pages[i]), got = deck::commentsOf(*back->pages[i]);
            QCOMPARE(int(got.size()), int(want.size()));
            for (int k = 0; k < want.size(); ++k) {
                QCOMPARE(got[k].author, want[k].author);
                QCOMPARE(got[k].initials, want[k].initials);
                QCOMPARE(got[k].date, want[k].date);
                QCOMPARE(got[k].text, want[k].text);
                QCOMPARE(got[k].at, want[k].at);
                QCOMPARE(got[k].done, want[k].done);
                QCOMPARE(int(got[k].replies.size()), int(want[k].replies.size()));
                for (int r = 0; r < want[k].replies.size(); ++r) {
                    QCOMPARE(got[k].replies[r].author, want[k].replies[r].author);
                    QCOMPARE(got[k].replies[r].initials, want[k].replies[r].initials);
                    QCOMPARE(got[k].replies[r].date, want[k].replies[r].date);
                    QCOMPARE(got[k].replies[r].text, want[k].replies[r].text);
                }
            }
        }
        QVERIFY(deck::commentsOf(*back->pages[0])[1].done);
        QVERIFY(!deck::commentsOf(*back->pages[0])[0].done);
        QVERIFY(deck::commentsOf(*back->pages[1]).isEmpty());
        // Written again, the same file's comments come out the same.
        QCOMPARE(deck::exportPresentation(*back).isEmpty(), false);
    }

    // The Review tab has a Comments group: New Comment, Delete (with its menu),
    // Previous, Next, and Show Comments.
    void reviewTabHasTheCommentsGroup()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        for (const char *id : {"comment.new", "comment.delete", "comment.deleteSlide", "comment.deleteAll", "comment.prev", "comment.next", "comment.show"})
            QVERIFY2(w.act(QString::fromLatin1(id)), id);
        QVERIFY(w.act(QStringLiteral("comment.show"))->isCheckable());
        QVERIFY(!w.act(QStringLiteral("comment.show"))->isChecked());
        QVERIFY(w.commentsPane());
        QVERIFY(w.commentsPane()->isHidden());
        auto button = [&](const QString &id) -> QToolButton * {
            for (QToolButton *b : w.ribbon()->tab(QStringLiteral("Review"))->findChildren<QToolButton *>())
                if (b->defaultAction() == w.act(id)) return b;
            return nullptr;
        };
        for (const char *id : {"comment.new", "comment.delete", "comment.prev", "comment.next", "comment.show"})
            QVERIFY2(button(QString::fromLatin1(id)), id);
        // Delete has a menu with the other two.
        QToolButton *del = button(QStringLiteral("comment.delete"));
        QVERIFY(del->menu());
        QVERIFY(del->menu()->actions().contains(w.act(QStringLiteral("comment.deleteSlide"))));
        QVERIFY(del->menu()->actions().contains(w.act(QStringLiteral("comment.deleteAll"))));
        // With no comments there is nothing to delete or visit; New Comment and Show Comments are always there.
        QVERIFY(w.act(QStringLiteral("comment.new"))->isEnabled());
        QVERIFY(w.act(QStringLiteral("comment.show"))->isEnabled());
        for (const char *id : {"comment.delete", "comment.deleteSlide", "comment.deleteAll", "comment.prev", "comment.next"})
            QVERIFY2(!w.act(QString::fromLatin1(id))->isEnabled(), id);
        // The KeyTips of the tab stay their own.
        QStringList seen;
        for (const QWidget *c : w.ribbon()->tab(QStringLiteral("Review"))->findChildren<QWidget *>()) {
            const QString k = keytip(c);
            if (k.isEmpty()) continue;
            QVERIFY2(!seen.contains(k), qPrintable("two controls have the KeyTip " + k));
            seen << k;
        }
        // Show Comments opens and closes the pane.
        w.resize(1280, 800);
        w.show();
        w.act(QStringLiteral("comment.show"))->trigger();
        QVERIFY(!w.commentsPane()->isHidden());
        QVERIFY(w.canvas()->commentsShown());
        w.act(QStringLiteral("comment.show"))->trigger();
        QVERIFY(w.commentsPane()->isHidden());
        QVERIFY(!w.canvas()->commentsShown());
    }

    // The Comments pane: New Comment opens a box to type in, Post makes the
    // comment as one undo step, and each card replies, resolves, edits, and deletes.
    void commentsPaneAddsRepliesResolvesEditsAndDeletes()
    {
        Settings &st = Settings::get();
        st.setValue(QStringLiteral("user/name"), QStringLiteral("Pat Doe"));
        st.setValue(QStringLiteral("user/initials"), QStringLiteral("PD"));
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        Editor *ed = w.editor();
        CommentsPane *pane = w.commentsPane();
        QVERIFY(pane->isHidden());
        auto slide = [&]() -> Page & { return *ed->doc()->pages[ed->currentPage()]; };
        auto card = [&](int i = 0) { return pane->findChildren<QFrame *>(QStringLiteral("commentCard")).value(i); };

        w.act(QStringLiteral("comment.new"))->trigger();
        QVERIFY(!pane->isHidden());
        QVERIFY(w.act(QStringLiteral("comment.show"))->isChecked());
        auto *draft = pane->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"));
        QVERIFY(draft && draft->isVisible());
        QCOMPARE(w.focusWidget(), draft);
        QCOMPARE(int(pane->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 0);
        auto *post = pane->findChild<QPushButton *>(QStringLiteral("commentPost"));
        QVERIFY(post);
        QVERIFY(!post->isEnabled());   // nothing typed
        draft->setPlainText(QStringLiteral("Check the title"));
        QVERIFY(post->isEnabled());
        QCOMPARE(deck::commentCount(slide()), 0);   // not until Post
        post->click();
        QCOMPARE(deck::commentCount(slide()), 1);
        QCOMPARE(ed->undoStack()->count(), 1);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("New Comment"));
        deck::Comment c = deck::commentsOf(slide())[0];
        QCOMPARE(c.author, QStringLiteral("Pat Doe"));
        QCOMPARE(c.initials, QStringLiteral("PD"));
        QCOMPARE(c.text, QStringLiteral("Check the title"));
        QVERIFY(c.date.isValid() && qAbs(c.date.secsTo(QDateTime::currentDateTimeUtc())) < 60);
        QVERIFY(c.at.x() >= 0 && c.at.x() < 960 && c.at.y() >= 0 && c.at.y() < 540);
        QVERIFY(!draft->isVisible());
        QCOMPARE(int(pane->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 1);
        QCOMPARE(pane->currentComment(), c.id);   // the new comment is the one open
        QVERIFY(w.act(QStringLiteral("comment.delete"))->isEnabled());
        QVERIFY(card()->findChild<QLabel *>(QStringLiteral("commentText"))->text().contains(QStringLiteral("Check the title")));
        QVERIFY(card()->findChild<QLabel *>(QStringLiteral("commentAuthor"))->text().contains(QStringLiteral("Pat Doe")));

        // A reply.
        auto *reply = card()->findChild<QLineEdit *>(QStringLiteral("commentReplyEdit"));
        auto *replyPost = card()->findChild<QAbstractButton *>(QStringLiteral("commentReplyPost"));
        QVERIFY(reply && replyPost);
        QVERIFY(!replyPost->isEnabled());
        reply->setText(QStringLiteral("Agreed"));
        QVERIFY(replyPost->isEnabled());
        replyPost->click();
        QCOMPARE(int(deck::commentsOf(slide())[0].replies.size()), 1);
        QCOMPARE(deck::commentsOf(slide())[0].replies[0].text, QStringLiteral("Agreed"));
        QCOMPARE(deck::commentsOf(slide())[0].replies[0].author, QStringLiteral("Pat Doe"));
        QCOMPARE(ed->undoStack()->count(), 2);
        QCOMPARE(int(card()->findChildren<QLabel *>(QStringLiteral("commentReplyText")).size()), 1);
        // Enter in the box replies too.
        card()->findChild<QLineEdit *>(QStringLiteral("commentReplyEdit"))->setText(QStringLiteral("Second answer"));
        QTest::keyClick(card()->findChild<QLineEdit *>(QStringLiteral("commentReplyEdit")), Qt::Key_Return);
        QCOMPARE(int(deck::commentsOf(slide())[0].replies.size()), 2);

        // Resolve and reopen.
        auto *resolve = card()->findChild<QAbstractButton *>(QStringLiteral("commentResolve"));
        QVERIFY(resolve);
        resolve->click();
        QVERIFY(deck::commentsOf(slide())[0].done);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Resolve Comment"));
        resolve = card()->findChild<QAbstractButton *>(QStringLiteral("commentResolve"));
        QVERIFY(resolve->text().contains(QStringLiteral("Reopen")));
        resolve->click();
        QVERIFY(!deck::commentsOf(slide())[0].done);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Reopen Comment"));

        // Edit the text.
        card()->findChild<QAbstractButton *>(QStringLiteral("commentEdit"))->click();
        auto *editor = card()->findChild<QPlainTextEdit *>(QStringLiteral("commentEditor"));
        QVERIFY(editor);
        QCOMPARE(editor->toPlainText(), QStringLiteral("Check the title"));
        editor->setPlainText(QStringLiteral("Check the heading"));
        card()->findChild<QAbstractButton *>(QStringLiteral("commentSave"))->click();
        QCOMPARE(deck::commentsOf(slide())[0].text, QStringLiteral("Check the heading"));
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Edit Comment"));
        QVERIFY(!card()->findChild<QPlainTextEdit *>(QStringLiteral("commentEditor")));
        // Cancel leaves it be.
        const int steps = ed->undoStack()->count();
        card()->findChild<QAbstractButton *>(QStringLiteral("commentEdit"))->click();
        card()->findChild<QPlainTextEdit *>(QStringLiteral("commentEditor"))->setPlainText(QStringLiteral("Never mind"));
        card()->findChild<QAbstractButton *>(QStringLiteral("commentCancel"))->click();
        QCOMPARE(ed->undoStack()->count(), steps);
        QCOMPARE(deck::commentsOf(slide())[0].text, QStringLiteral("Check the heading"));

        // A reply goes by itself.
        auto replyDeletes = card()->findChildren<QAbstractButton *>(QStringLiteral("commentReplyDelete"));
        QCOMPARE(int(replyDeletes.size()), 2);
        replyDeletes[0]->click();
        QCOMPARE(int(deck::commentsOf(slide())[0].replies.size()), 1);
        QCOMPARE(deck::commentsOf(slide())[0].replies[0].text, QStringLiteral("Second answer"));

        // Delete takes the comment and its replies; one undo brings it all back, and redo takes it again.
        card()->findChild<QAbstractButton *>(QStringLiteral("commentDelete"))->click();
        QCOMPARE(deck::commentCount(slide()), 0);
        QCOMPARE(int(pane->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 0);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Delete Comment"));
        QVERIFY(!w.act(QStringLiteral("comment.delete"))->isEnabled());
        ed->undo();
        QCOMPARE(deck::commentCount(slide()), 1);
        QCOMPARE(int(deck::commentsOf(slide())[0].replies.size()), 1);
        QTRY_COMPARE(int(pane->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 1);
        ed->redo();
        QCOMPARE(deck::commentCount(slide()), 0);
        QTRY_COMPARE(int(pane->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 0);
        // Undoing all the way takes the first comment away again.
        while (ed->undoStack()->canUndo()) ed->undo();
        QCOMPARE(deck::commentCount(slide()), 0);

        // Cancel in the new comment's box makes nothing.
        w.act(QStringLiteral("comment.new"))->trigger();
        pane->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"))->setPlainText(QStringLiteral("Never mind"));
        pane->findChild<QPushButton *>(QStringLiteral("commentDraftCancel"))->click();
        QVERIFY(!pane->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"))->isVisible());
        QCOMPARE(deck::commentCount(slide()), 0);
        QVERIFY(!ed->undoStack()->canUndo());
        st.setValue(QStringLiteral("user/name"), QString());
        st.setValue(QStringLiteral("user/initials"), QString());
    }

    // The pane asks who is writing the first time, when File > Options has no
    // name, and keeps the answer; saying no makes no comment.
    void commentsPaneAsksWhoIsWritingOnce()
    {
        Settings &st = Settings::get();
        st.setValue(QStringLiteral("user/name"), QString());
        st.setValue(QStringLiteral("user/initials"), QString());
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        // Declined: nothing opens, nothing is kept.
        bool asked = false;
        QTimer::singleShot(0, [&] {
            auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!dlg) return;
            asked = dlg->findChild<QLineEdit *>(QStringLiteral("commentAuthorName")) && dlg->findChild<QLineEdit *>(QStringLiteral("commentAuthorInitials"));
            dlg->reject();
        });
        w.act(QStringLiteral("comment.new"))->trigger();
        QVERIFY(asked);
        QVERIFY(!deck::commentAuthor().known());
        QVERIFY(!w.commentsPane()->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft")) || !w.commentsPane()->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"))->isVisible());
        // Answered: kept in the settings, and the box to type in opens.
        asked = false;
        QTimer::singleShot(0, [&] {
            auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!dlg) return;
            asked = true;
            dlg->findChild<QLineEdit *>(QStringLiteral("commentAuthorName"))->setText(QStringLiteral("Pat Doe"));
            dlg->accept();
        });
        w.act(QStringLiteral("comment.new"))->trigger();
        QVERIFY(asked);
        QCOMPARE(st.value(QStringLiteral("user/name")).toString(), QStringLiteral("Pat Doe"));
        QCOMPARE(st.value(QStringLiteral("user/initials")).toString(), QStringLiteral("PD"));
        auto *draft = w.commentsPane()->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"));
        QVERIFY(draft && draft->isVisible());
        draft->setPlainText(QStringLiteral("Hello"));
        w.commentsPane()->findChild<QPushButton *>(QStringLiteral("commentPost"))->click();
        QCOMPARE(deck::commentsOf(*w.editor()->doc()->pages[0])[0].initials, QStringLiteral("PD"));
        // Not again.
        bool askedAgain = false;
        QTimer watch;   // lets go of a dialog that would otherwise hold the test
        watch.setSingleShot(true);
        connect(&watch, &QTimer::timeout, [&] {
            if (auto *dlg = QApplication::activeModalWidget()) {
                askedAgain = true;
                dlg->close();
            }
        });
        watch.start(0);
        w.act(QStringLiteral("comment.new"))->trigger();
        QVERIFY(!askedAgain);
        QVERIFY(w.commentsPane()->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"))->isVisible());
        st.setValue(QStringLiteral("user/name"), QString());
        st.setValue(QStringLiteral("user/initials"), QString());
    }

    // Each comment has a marker on the slide, where its place is: it shows only
    // while comments are shown, a click opens the comment in the pane, and
    // dragging it moves the comment as one step that undoes.
    void commentMarkersShowOnTheSlideAndMove()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        Editor *ed = w.editor();
        DeckCanvas *cv = w.canvas();
        const deck::CommentAuthor pat{QStringLiteral("Pat Doe"), QStringLiteral("PD")};
        ed->beginChange(QStringLiteral("Comments"));
        const QString first = deck::addComment(*ed->doc()->pages[0], pat, QStringLiteral("One"), QPointF(600, 100));
        const QString second = deck::addComment(*ed->doc()->pages[0], pat, QStringLiteral("Two"), QPointF(300, 400));
        ed->endChange();
        ed->undoStack()->clear();
        QVERIFY(!cv->commentsShown());
        auto markerPoint = [&](const QPointF &page) { return (cv->pageToView(page) + QPointF(6, 6)).toPoint(); };
        QTRY_VERIFY(cv->viewport()->isVisible());
        QVERIFY(cv->commentAt(markerPoint(QPointF(600, 100))).isEmpty());   // hidden
        const QImage hidden = cv->viewport()->grab().toImage();

        w.act(QStringLiteral("comment.show"))->trigger();
        QVERIFY(cv->commentsShown());
        QCOMPARE(cv->commentAt(markerPoint(QPointF(600, 100))), first);
        QCOMPARE(cv->commentAt(markerPoint(QPointF(300, 400))), second);
        QVERIFY(cv->commentAt((cv->pageToView(QPointF(600, 100)) - QPointF(8, 8)).toPoint()).isEmpty());
        QVERIFY(cv->commentAt(markerPoint(QPointF(450, 250))).isEmpty());
        // The markers are drawn: the pixels under them differ from the slide without them.
        const QImage shown = cv->viewport()->grab().toImage();
        for (const QPointF &page : {QPointF(600, 100), QPointF(300, 400)}) {
            int changed = 0;
            const QPoint at = cv->pageToView(page).toPoint();
            for (int dy = 2; dy < 14; ++dy)
                for (int dx = 2; dx < 18; ++dx) changed += shown.pixelColor(at.x() + dx, at.y() + dy) != hidden.pixelColor(at.x() + dx, at.y() + dy);
            QVERIFY2(changed > 60, qPrintable(QString::number(changed)));
        }
        // Another slide has none of these.
        deck::addSlide(*ed->doc(), *deck::layoutOfType(*ed->doc(), QStringLiteral("obj")));
        ed->setCurrentPage(1);
        QVERIFY(cv->commentAt(markerPoint(QPointF(600, 100))).isEmpty());
        ed->setCurrentPage(0);
        ed->undoStack()->clear();

        // A click opens the pane on the comment.
        w.act(QStringLiteral("comment.show"))->trigger();
        QVERIFY(w.commentsPane()->isHidden());
        w.act(QStringLiteral("comment.show"))->trigger();
        QVERIFY(w.commentsPane()->currentComment().isEmpty());
        QTest::mouseClick(cv->viewport(), Qt::LeftButton, Qt::NoModifier, markerPoint(QPointF(300, 400)));
        QVERIFY(!w.commentsPane()->isHidden());
        QCOMPARE(w.commentsPane()->currentComment(), second);
        QCOMPARE(cv->activeComment(), second);
        QVERIFY(w.act(QStringLiteral("comment.delete"))->isEnabled());
        QCOMPARE(ed->undoStack()->count(), 0);   // a click moves nothing
        QCOMPARE(deck::commentsOf(*ed->doc()->pages[0])[1].at, QPointF(300, 400));

        // A drag moves it, in one step.
        const QPoint from = markerPoint(QPointF(300, 400)), to = from + QPoint(90, -60);
        QTest::mousePress(cv->viewport(), Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(cv->viewport(), from + QPoint(20, -10));
        QTest::mouseMove(cv->viewport(), from + QPoint(60, -40));
        QTest::mouseMove(cv->viewport(), to);
        QCOMPARE(ed->undoStack()->count(), 0);   // not until it lets go
        QTest::mouseRelease(cv->viewport(), Qt::LeftButton, Qt::NoModifier, to);
        QCOMPARE(ed->undoStack()->count(), 1);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Move Comment"));
        const QPointF moved = deck::commentsOf(*ed->doc()->pages[0])[1].at;
        QVERIFY2(qAbs(moved.x() - (300 + 90 / cv->ppp())) < 1.5 && qAbs(moved.y() - (400 - 60 / cv->ppp())) < 1.5, qPrintable(QString::number(moved.x()) + "," + QString::number(moved.y())));
        QCOMPARE(deck::commentsOf(*ed->doc()->pages[0])[0].at, QPointF(600, 100));   // the other stayed
        QCOMPARE(cv->commentAt(markerPoint(moved)), second);
        QCOMPARE(w.commentsPane()->currentComment(), second);
        ed->undo();
        QCOMPARE(deck::commentsOf(*ed->doc()->pages[0])[1].at, QPointF(300, 400));
        // The markers don't take the slide's own clicks away where they aren't.
        ed->clearSelection();
        const QPointF titleAt = ed->doc()->pages[0]->items[0]->rect.center();
        QTest::mouseClick(cv->viewport(), Qt::LeftButton, Qt::NoModifier, cv->pageToView(titleAt).toPoint());
        QCOMPARE(ed->selection(), QStringList{ed->doc()->pages[0]->items[0]->id});
        (void)first;
    }

    // Previous and Next go from comment to comment through the presentation, in
    // slide order, opening each one in the pane on its slide; the Delete menu
    // clears the slide's comments or all of them as one step.
    void commentCommandsVisitAndDelete()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        Editor *ed = w.editor();
        for (int i = 0; i < 2; ++i) deck::addSlide(*ed->doc(), *deck::layoutOfType(*ed->doc(), QStringLiteral("obj")));
        const deck::CommentAuthor pat{QStringLiteral("Pat Doe"), QStringLiteral("PD")};
        ed->beginChange(QStringLiteral("Comments"));
        const QString a = deck::addComment(*ed->doc()->pages[0], pat, QStringLiteral("A"), QPointF(10, 10));
        const QString b = deck::addComment(*ed->doc()->pages[0], pat, QStringLiteral("B"), QPointF(40, 10));
        const QString c = deck::addComment(*ed->doc()->pages[2], pat, QStringLiteral("C"), QPointF(10, 10));
        ed->endChange();
        ed->undoStack()->clear();
        ed->setCurrentPage(0);
        QVERIFY(w.act(QStringLiteral("comment.next"))->isEnabled() && w.act(QStringLiteral("comment.prev"))->isEnabled());
        QVERIFY(w.act(QStringLiteral("comment.deleteAll"))->isEnabled() && w.act(QStringLiteral("comment.deleteSlide"))->isEnabled());
        QVERIFY(!w.act(QStringLiteral("comment.delete"))->isEnabled());   // none is open yet
        auto open = [&] { return w.commentsPane()->currentComment(); };

        w.act(QStringLiteral("comment.next"))->trigger();   // none was open: the first
        QVERIFY(!w.commentsPane()->isHidden());
        QVERIFY(w.canvas()->commentsShown());
        QCOMPARE(open(), a);
        w.act(QStringLiteral("comment.next"))->trigger();
        QCOMPARE(open(), b);
        w.act(QStringLiteral("comment.next"))->trigger();   // over to the third slide
        QCOMPARE(open(), c);
        QCOMPARE(ed->currentPage(), 2);
        w.act(QStringLiteral("comment.next"))->trigger();   // and round to the first
        QCOMPARE(open(), a);
        QCOMPARE(ed->currentPage(), 0);
        w.act(QStringLiteral("comment.prev"))->trigger();   // and back
        QCOMPARE(open(), c);
        QCOMPARE(ed->currentPage(), 2);
        w.act(QStringLiteral("comment.prev"))->trigger();
        QCOMPARE(open(), b);
        QCOMPARE(ed->currentPage(), 0);

        // Delete takes the open comment.
        QVERIFY(w.act(QStringLiteral("comment.delete"))->isEnabled());
        w.act(QStringLiteral("comment.delete"))->trigger();
        QCOMPARE(int(deck::commentsOf(*ed->doc()->pages[0]).size()), 1);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Delete Comment"));
        QVERIFY(open().isEmpty());
        QVERIFY(!w.act(QStringLiteral("comment.delete"))->isEnabled());
        ed->undo();
        QCOMPARE(int(deck::commentsOf(*ed->doc()->pages[0]).size()), 2);
        // All of the slide's.
        ed->undoStack()->clear();
        w.act(QStringLiteral("comment.deleteSlide"))->trigger();
        QVERIFY(deck::commentsOf(*ed->doc()->pages[0]).isEmpty());
        QCOMPARE(int(deck::commentsOf(*ed->doc()->pages[2]).size()), 1);
        QCOMPARE(ed->undoStack()->count(), 1);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Delete Comments"));
        ed->undo();
        QCOMPARE(int(deck::commentsOf(*ed->doc()->pages[0]).size()), 2);
        // All of the presentation's, in one step.
        ed->undoStack()->clear();
        w.act(QStringLiteral("comment.deleteAll"))->trigger();
        QVERIFY(deck::allComments(*ed->doc()).isEmpty());
        QCOMPARE(ed->undoStack()->count(), 1);
        QVERIFY(!w.act(QStringLiteral("comment.deleteAll"))->isEnabled());
        QVERIFY(!w.act(QStringLiteral("comment.next"))->isEnabled());
        ed->undo();
        QCOMPARE(int(deck::allComments(*ed->doc()).size()), 3);
        QVERIFY(w.act(QStringLiteral("comment.next"))->isEnabled());
    }

    // A presentation opened with comments in it shows them, and the comments
    // go into the file the window saves.
    void commentsOpenAndSaveWithThePresentation()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("talk.pptx"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(pack(withLegacyComments()));
        f.close();
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        QVERIFY(w.openFile(path));
        QVERIFY(!w.commentsPane()->isHidden());   // the file has comments: they show
        QVERIFY(w.canvas()->commentsShown());
        QTRY_COMPARE(int(w.commentsPane()->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 2);
        w.editor()->setCurrentPage(1);
        QTRY_COMPARE(int(w.commentsPane()->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 0);
        const QString out = dir.filePath(QStringLiteral("again.pptx"));
        QVERIFY(w.saveTo(out));
        QString err;
        auto back = deck::importPresentationFile(out, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(deck::commentsOf(*back->pages[0]).size()), 2);
        QCOMPARE(int(deck::commentsOf(*back->pages[0])[0].replies.size()), 1);
        w.editor()->markSaved();
    }


    // File > Options is the shared dialog: its General tab has the user name
    // but not the update check or the statistics (JeffDeck's preview does
    // neither; JeffPub's Options keep them for the suite), Proofing follows,
    // and JeffDeck's View page sets the ruler and snapping for every open window.
    void optionsHasTheGeneralTabAndTheViewPage()
    {
        DeckWindow w;
        QVERIFY(w.act(QStringLiteral("file.options")));
        QStringList tabNames;
        QString title, shown, userName;
        QStringList online;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            title = d->windowTitle();
            if (auto *tabs = d->findChild<QTabWidget *>()) {
                shown = tabs->tabText(tabs->currentIndex());
                for (int i = 0; i < tabs->count(); ++i) tabNames << tabs->tabText(i);
            }
            for (QLabel *l : d->findChildren<QLabel *>())
                if (l->text() == QLatin1String("User name:") && l->buddy()) userName = static_cast<QLineEdit *>(l->buddy())->text();
            for (const char *name : {"updates", "stats", "betaUpdates"})
                if (auto *box = d->findChild<QCheckBox *>(QLatin1String(name)); box && box->isVisibleTo(d)) online << box->text();
            d->reject();
        });
        w.act(QStringLiteral("file.options"))->trigger();
        QCOMPARE(title, QStringLiteral("JeffDeck Options"));
        QCOMPARE(tabNames, (QStringList{"General", "Proofing", "View", "Save"}));
        QCOMPARE(shown, QStringLiteral("General"));
        QCOMPARE(userName, Settings::get().userName());
        // JeffDeck checks for updates and sends statistics (when chosen) like JeffPub, so the choices are here, in its own words.
        QVERIFY(deckAppInfo().online);
        QCOMPARE(online, (QStringList{QStringLiteral("Check for updates when JeffDeck starts"), QStringLiteral("Send anonymous usage statistics"),
                                      QStringLiteral("Get beta versions (early builds for testing)")}));
        // The View page: the ruler and snapping, in this window now and in new ones.
        QVERIFY(!w.editor()->view.rulers);
        QVERIFY(w.editor()->view.snapObjects);
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            if (auto *box = d->findChild<QCheckBox *>(QStringLiteral("showRuler"))) box->setChecked(true);
            if (auto *box = d->findChild<QCheckBox *>(QStringLiteral("snapObjects"))) box->setChecked(false);
            d->accept();
        });
        w.act(QStringLiteral("file.options"))->trigger();
        QVERIFY(w.editor()->view.rulers);
        QVERIFY(!w.canvas()->hRuler()->isHidden());
        QVERIFY(!w.editor()->view.snapObjects);
        {
            DeckWindow another;
            QVERIFY(another.editor()->view.rulers);
            QVERIFY(!another.editor()->view.snapObjects);
        }
        Settings::get().setValue(QStringLiteral("view/rulers"), false);
        Settings::get().setValue(QStringLiteral("view/snapObjects"), true);
    }

    // A ping and a crash report from JeffDeck say so, and carry nothing else JeffPub's don't.
    void deckPingsAndCrashReportsNameJeffDeck()
    {
        telemetry::setEnabled(true);
        const QJsonObject p = telemetry::payload();
        QCOMPARE(p[QStringLiteral("app")].toString(), QStringLiteral("jeffdeck"));
        QStringList keys = p.keys();
        keys.sort();
        QCOMPARE(keys, (QStringList{"app", "counts", "install", "lang", "launches", "os", "osVersion", "version"}));
        QJsonObject report;
        report[QStringLiteral("version")] = QStringLiteral(JP_VERSION);
        report[QStringLiteral("when")] = QDateTime::currentDateTimeUtc().date().toString(Qt::ISODate);
        report[QStringLiteral("kind")] = QStringLiteral("signal");
        report[QStringLiteral("code")] = 11;
        report[QStringLiteral("frames")] = QJsonArray{QJsonObject{{QStringLiteral("module"), QStringLiteral("JeffDeck.exe")}, {QStringLiteral("offset"), 6699}}};
        const QJsonObject sent = telemetry::crashPayload(report);
        keys = sent.keys();
        keys.sort();
        QCOMPARE(keys, (QStringList{"app", "code", "frames", "install", "kind", "os", "version", "when"}));
        QCOMPARE(sent[QStringLiteral("app")].toString(), QStringLiteral("jeffdeck"));
        QCOMPARE(sent[QStringLiteral("install")].toString(), p[QStringLiteral("install")].toString());
        telemetry::setEnabled(false);
        QVERIFY(telemetry::crashPayload(report).isEmpty());   // not sent when sharing is off (and the report is deleted either way)
    }

    // The choice to share and the install id are the suite's: made in JeffPub, they hold here, and JeffDeck doesn't ask. The counts
    // are JeffDeck's own, and JeffPub's never reach its ping.
    void deckSharesTheChoiceAndTheInstallButKeepsItsOwnCounts()
    {
        QSettings suite(QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice")), pub(QStringLiteral("JeffOffice"), QStringLiteral("JeffPub")),
            deck(QStringLiteral("JeffOffice"), QStringLiteral("JeffDeck"));
        QVERIFY(!telemetry::enabled());
        suite.setValue(QStringLiteral("telemetry/enabled"), true);
        suite.setValue(QStringLiteral("telemetry/install"), QStringLiteral("0f8fad5b-d9cb-469f-a165-70867728950e"));
        suite.sync();
        pub.setValue(QStringLiteral("telemetry/counts"), QVariantMap{{QStringLiteral("file.open.pub"), 4}});
        pub.setValue(QStringLiteral("telemetry/launches"), 6);
        pub.sync();
        Settings::get().sync();
        QVERIFY(telemetry::decided());   // so JeffDeck's first start has no question to ask
        QVERIFY(telemetry::enabled());
        QJsonObject p = telemetry::payload();
        QCOMPARE(p[QStringLiteral("install")].toString(), QStringLiteral("0f8fad5b-d9cb-469f-a165-70867728950e"));
        QVERIFY(p[QStringLiteral("counts")].toObject().isEmpty());
        QCOMPARE(p[QStringLiteral("launches")].toInt(), 0);
        telemetry::count(QStringLiteral("file.open.pptx"));
        Settings::get().sync();
        QCOMPARE(telemetry::payload()[QStringLiteral("counts")].toObject().keys(), QStringList{QStringLiteral("file.open.pptx")});
        deck.sync();
        QCOMPARE(deck.value(QStringLiteral("telemetry/counts")).toMap().value(QStringLiteral("file.open.pptx")).toInt(), 1);
        QVERIFY(!deck.contains(QStringLiteral("telemetry/install")));   // the suite's, not its own
        pub.sync();
        QCOMPARE(pub.value(QStringLiteral("telemetry/counts")).toMap().keys(), QStringList{QStringLiteral("file.open.pub")});   // JeffPub's, untouched
        // Turned off here, it is off for the suite, and its id is gone.
        telemetry::setEnabled(false);
        Settings::get().sync();
        suite.sync();
        QVERIFY(!suite.value(QStringLiteral("telemetry/enabled")).toBool());
        QVERIFY(suite.value(QStringLiteral("telemetry/install")).toString().isEmpty());
        QVERIFY(telemetry::decided());
        pub.remove(QStringLiteral("telemetry"));
        pub.sync();
    }

    // JeffDeck counts each command by its id, and a file opened or saved by its kind, never by its name.
    void deckCountsCommandsAndFileKindsButNeverNames()
    {
        telemetry::setEnabled(false);
        {
            DeckWindow quiet;
            quiet.act(QStringLiteral("view.gridlines"))->trigger();
            quiet.act(QStringLiteral("view.gridlines"))->trigger();
            QVERIFY(telemetry::payload()[QStringLiteral("counts")].toObject().isEmpty());   // sharing is off: nothing counted
        }
        telemetry::setEnabled(true);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString secret = QStringLiteral("quarterly-numbers-for-Pat");
        const QString pptx = dir.filePath(secret + QStringLiteral(".pptx"));
        {
            DeckWindow w;
            w.act(QStringLiteral("view.gridlines"))->trigger();
            w.act(QStringLiteral("view.gridlines"))->trigger();
            QVERIFY(w.saveTo(pptx));
            QVERIFY(w.saveTo(dir.filePath(secret + QStringLiteral(".POTX"))));
            QVERIFY(w.saveTo(dir.filePath(secret + QStringLiteral(".ppsx"))));
            QVERIFY(w.saveTo(dir.filePath(secret + QStringLiteral(".dat"))));   // an ending JeffDeck has no kind for
            answerDialogs({QMessageBox::Ok});
            QVERIFY(!w.saveTo(dir.filePath(QStringLiteral("missing/") + secret + QStringLiteral(".pptx"))));   // a save that failed is not counted
        }
        // Read by what a file is and not what it is called; counted by the ending, among the kinds JeffDeck knows.
        for (const QString &name : {secret + QStringLiteral(".pptx"), secret + QStringLiteral(".odp"), secret + QStringLiteral(".PPT"), secret + QStringLiteral(".dat")}) {
            const QString copy = dir.filePath(QStringLiteral("copy-") + name);
            QVERIFY(QFile::copy(pptx, copy));
            DeckWindow w;
            QVERIFY(w.openFile(copy));
        }
        const QJsonObject counts = telemetry::payload()[QStringLiteral("counts")].toObject();
        QCOMPARE(counts[QStringLiteral("view.gridlines")].toInt(), 2);
        QCOMPARE(counts[QStringLiteral("file.save.pptx")].toInt(), 1);
        QCOMPARE(counts[QStringLiteral("file.save.potx")].toInt(), 1);
        QCOMPARE(counts[QStringLiteral("file.save.ppsx")].toInt(), 1);
        QCOMPARE(counts[QStringLiteral("file.save.other")].toInt(), 1);
        QCOMPARE(counts[QStringLiteral("file.open.pptx")].toInt(), 1);
        QCOMPARE(counts[QStringLiteral("file.open.odp")].toInt(), 1);
        QCOMPARE(counts[QStringLiteral("file.open.ppt")].toInt(), 1);
        QCOMPARE(counts[QStringLiteral("file.open.other")].toInt(), 1);
        QCOMPARE(counts.size(), 9);
        for (const QString &k : counts.keys()) {
            QVERIFY2(QRegularExpression(QStringLiteral("^[a-z0-9][a-z0-9._-]{0,47}$")).match(k).hasMatch(), qPrintable(k));   // what the collector takes
            QVERIFY2(!k.contains(QStringLiteral("quarterly"), Qt::CaseInsensitive) && !k.contains(QStringLiteral("pat")) && !k.contains(QStringLiteral("dat")), qPrintable(k));
        }
        telemetry::setEnabled(false);
    }

    // JeffDeck says which version it offers once a day with JeffPub: the note is the suite's, written under JeffDeck's name.
    void deckNotesWhatItOffered()
    {
        for (const char *k : {"offered", "offeredOn", "offeredBy"}) Settings::get().setValue(QStringLiteral("updates/") + QString::fromLatin1(k), QVariant());
        Updater::noteOffered(QStringLiteral("v0.8.0"));
        QSettings suite(QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice"));
        QCOMPARE(suite.value(QStringLiteral("updates/offeredBy")).toString(), QStringLiteral("jeffdeck"));
        QVERIFY(!Updater::offeredByAnotherAppToday(QStringLiteral("v0.8.0")));
        Settings::get().setValue(QStringLiteral("updates/offeredBy"), QStringLiteral("jeffpub"));   // JeffPub offered it first today
        QVERIFY(Updater::offeredByAnotherAppToday(QStringLiteral("v0.8.0")));
        for (const char *k : {"offered", "offeredOn", "offeredBy"}) Settings::get().setValue(QStringLiteral("updates/") + QString::fromLatin1(k), QVariant());
    }

private:
    // The sample presentation with comments in the newer form on its second slide.
    static Parts withModernComments()
    {
        Parts p = samplePptx();
        const QString head = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>");
        const QString modern = nsDecl() + QStringLiteral(" xmlns:p188=\"http://schemas.microsoft.com/office/powerpoint/2018/8/main\"");
        const QString pat = QStringLiteral("{11111111-1111-4111-8111-111111111111}"), lee = QStringLiteral("{22222222-2222-4222-8222-222222222222}");
        p.data["/ppt/authors.xml"] = {QStringLiteral("application/vnd.ms-powerpoint.authors+xml"),
            (head + QStringLiteral("<p188:authorLst ") + modern + QStringLiteral(">"
             "<p188:author id=\"%1\" name=\"Pat Doe\" initials=\"PD\" userId=\"S::pat@example.com::1\" providerId=\"AD\"/>"
             "<p188:author id=\"%2\" name=\"Lee Roy\" initials=\"LR\" userId=\"lee\" providerId=\"None\"/></p188:authorLst>").arg(pat, lee)).toUtf8()};
        p.rels << Rel{"/ppt/presentation.xml", "http://schemas.microsoft.com/office/2018/10/relationships/authors", "/ppt/authors.xml"};
        auto body = [](const QString &text) {
            return QStringLiteral("<p188:txBody><a:bodyPr/><a:lstStyle/>%1</p188:txBody>").arg(text);
        };
        auto line = [](const QString &text) { return QStringLiteral("<a:p><a:r><a:rPr lang=\"en-US\"/><a:t>%1</a:t></a:r></a:p>").arg(text); };
        const QString slideMark = QStringLiteral("<pc:docMk/><pc:sldMk cId=\"123456\" sldId=\"257\"/>");
        const QString pc = QStringLiteral("xmlns:pc=\"http://schemas.microsoft.com/office/powerpoint/2013/main/command\"");
        QString xml = head + QStringLiteral("<p188:cmLst ") + modern + QStringLiteral(">");
        xml += QStringLiteral("<p188:cm id=\"{AAAAAAAA-0000-4000-8000-000000000001}\" authorId=\"%1\" created=\"2026-10-12T17:15:30.250\" status=\"resolved\">"
                              "<pc:sldMkLst %2>%3</pc:sldMkLst><p188:pos x=\"9144000\" y=\"1371600\"/>"
                              "<p188:replyLst><p188:reply id=\"{BBBBBBBB-0000-4000-8000-000000000001}\" authorId=\"%4\" created=\"2026-10-12T17:20:00.000\">%5</p188:reply></p188:replyLst>%6</p188:cm>")
                   .arg(pat, pc, slideMark, lee, body(line(QStringLiteral("On it"))), body(line(QStringLiteral("Line one")) + line(QStringLiteral("Line two"))));
        xml += QStringLiteral("<p188:cm id=\"{AAAAAAAA-0000-4000-8000-000000000002}\" authorId=\"%1\" created=\"2026-10-12T18:00:00.000\">"
                              "<ac:deMkLst xmlns:ac=\"http://schemas.microsoft.com/office/drawing/2013/main/command\" %2>%3<ac:spMk id=\"2\" creationId=\"{CCCCCCCC-0000-4000-8000-000000000001}\"/></ac:deMkLst>%4</p188:cm>")
                   .arg(lee, pc, slideMark, body(line(QStringLiteral("Anchored"))));
        xml += QStringLiteral("<p188:cm id=\"{AAAAAAAA-0000-4000-8000-000000000003}\" authorId=\"%1\" created=\"2026-10-12T18:30:00.000\"><pc:sldMkLst %2>%3</pc:sldMkLst>%4</p188:cm>")
                   .arg(pat, pc, slideMark, body(line(QStringLiteral("Nowhere"))));
        xml += QStringLiteral("</p188:cmLst>");
        p.data["/ppt/comments/modernComment_101_5.xml"] = {QStringLiteral("application/vnd.ms-powerpoint.comments+xml"), xml.toUtf8()};
        p.rels << Rel{"/ppt/slides/slide2.xml", "http://schemas.microsoft.com/office/2018/10/relationships/comments", "/ppt/comments/modernComment_101_5.xml"};
        return p;
    }
};

JP_TEST_CLASS(DeckReviewTests)
#include "tst_deck_review.moc"
