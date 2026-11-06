#include "tst_write.h"


// ---- endnotes: the most notes, in the window

// A document with the most endnotes the program holds (each cited): in the window Insert Endnote is off, the status bar says why, and Insert Footnote (another count) is on.
void WriteTests::aDocumentWithTheMostEndnotesTurnsInsertEndnoteOffInTheWindow()
{
    const int count = kMostNoteStories;
    QByteArray ends = "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote();
    QByteArray body;
    for (int i = 1; i <= count; ++i) {
        ends += "<w:endnote w:id=\"" + QByteArray::number(i) + "\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t> e" + QByteArray::number(i) + "</w:t></w:r></w:p></w:endnote>";
        body += "<w:p><w:r><w:t>q" + QByteArray::number(i) + "</w:t></w:r><w:r><w:endnoteReference w:id=\"" + QByteArray::number(i) + "\"/></w:r></w:p>";
    }
    ends += "</w:endnotes>";
    const QByteArray zip = makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), ends}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>");
    WriteWindow w;
    w.resize(1000, 900);
    w.show();
    w.setDocument(open(zip), QString());
    QTRY_VERIFY(!w.act("refs.endnote")->isEnabled());
    QTRY_VERIFY2(w.statusBar()->currentMessage().contains(QLocale().toString(count)), qPrintable(w.statusBar()->currentMessage()));
    QVERIFY(w.act("refs.footnote")->isEnabled());
    QVERIFY(w.act("refs.endnote")->statusTip().contains(QLocale().toString(count)));
    // Triggered all the same (a shortcut can be): nothing is made.
    const int blocks = w.document()->story()->blockCount();
    w.act("refs.endnote")->trigger();
    QCOMPARE(w.document()->story()->blockCount(), blocks);
}


// ---- endnotes: a keystroke and the references after it

// A keystroke before the references of a long text costs the same, whatever their number: the places of the references are not all moved on for each (the work is counted in the
// places that are read or written).
void WriteTests::aKeystrokeCostsAboutTheSameWhateverTheNumberOfEndnoteReferencesAfterIt()
{
    auto workPerKey = [&](int references) {
        QByteArray body = paragraphs(40, 8), notes = "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote();
        for (int i = 1; i <= references; ++i) {
            body += "<w:p><w:r><w:t>r" + QByteArray::number(i) + "</w:t></w:r><w:r><w:endnoteReference w:id=\"" + QByteArray::number(i) + "\"/></w:r></w:p>";
            notes += "<w:endnote w:id=\"" + QByteArray::number(i) + "\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t> n" + QByteArray::number(i) + "</w:t></w:r></w:p></w:endnote>";
        }
        notes += "</w:endnotes>";
        auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), notes}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>"));
        [&] { QVERIFY(doc); }();
        QTextCursor c(doc->story());
        c.setPosition(5);   // (before every reference)
        insertParagraph(*doc, c);   // (the first edits that can change the notes: the references are looked for once)
        deleteBack(*doc, c, false);
        resetEndnoteSequenceWorkForTest();
        for (int i = 0; i < 100; ++i) insertText(*doc, c, QStringLiteral("x"));
        for (int i = 0; i < 20; ++i) deleteBack(*doc, c, false);
        const bool followed = doc->endnoteSequenceMatchesStoryForTest();
        [&] { QVERIFY(followed); }();
        return double(endnoteSequenceWorkForTest()) / 120;
    };
    const double few = workPerKey(20), many = workPerKey(1000);
    QVERIFY2(many <= 60, qPrintable(QStringLiteral("%1 places read or written for a keystroke with 1000 references after it, %2 with 20").arg(many).arg(few)));
    QVERIFY2(many <= few + 40, qPrintable(QStringLiteral("%1 places for a keystroke with 1000 references after it, %2 with 20").arg(many).arg(few)));
}


// ---- endnotes: the caret across pages and copies

// An endnote that goes on over several pages: a click on any page puts the caret there (its page, and the place drawn), typing leaves it, Ctrl+End and Ctrl+Home go to the end and the
// start of the note's words, and Down from the first line to the last goes through every page that has the note.
void WriteTests::theCaretStaysOnThePageOfAnEndnoteThatContinuesOnAnotherPage()
{
    QByteArray words;
    for (int i = 0; i < 2600; ++i) words += " word" + QByteArray::number(i);
    const QByteArray ends = "<w:endnotes " + QByteArray(kNs) + "><w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:endnote><w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\">" +
                            words + "</w:t></w:r></w:p></w:endnote></w:endnotes>";
    const QByteArray body = "<w:p><w:r><w:t>Body text</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>";
    auto doc = open(makeDocxWithParts(body, pageSection(), {{QStringLiteral("/word/endnotes.xml"), ends}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>"));
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 1200);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    int from = 0, to = 0;
    QVERIFY(doc->endnoteWords(2, &from, &to));
    const int firstPage = doc->pager().pageOfPosition(from), lastPage = doc->pager().pageOfPosition(to);
    QVERIFY2(lastPage - firstPage >= 2, qPrintable(QString::number(lastPage - firstPage)));
    auto caretOnPage = [&](int page) {
        QRect caret;
        return view.caretRect(&caret) && view.pageRect(page).adjusted(-3, -3, 3, 3).contains(caret) && view.currentPage() == page;
    };
    auto scrollTo = [&](int page) {
        view.verticalScrollBar()->setValue(view.verticalScrollBar()->value() + view.pageRect(page).top());
        QCoreApplication::processEvents();
    };
    for (int page = firstPage; page <= lastPage; ++page) {
        scrollTo(page);
        // (Not at the same place as the click before, or the clicks go for a double and a triple click, which select a word and the paragraph.)
        QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, pagePoint(view, page, 200 + 40 * (page - firstPage), page == firstPage ? 600 : 300));
        const EndnoteSpan span = endnoteSpanAt(doc->story(), view.cursor().position());
        int noteId = 0;
        QVERIFY2(span.valid && span.editable && doc->endnoteIdOf(doc->story()->findBlock(view.cursor().position()), &noteId) && noteId == 2, qPrintable(QStringLiteral("a click on page %1: the caret is at %2").arg(page).arg(view.cursor().position())));
        QVERIFY2(caretOnPage(page), qPrintable(QStringLiteral("a click on page %1").arg(page)));
        QTest::keyClicks(&view, "x");
        QVERIFY2(caretOnPage(page), qPrintable(QStringLiteral("typing on page %1").arg(page)));
    }
    QTest::keyClick(&view, Qt::Key_End, Qt::ControlModifier);
    QVERIFY(doc->endnoteWords(2, &from, &to));
    QCOMPARE(view.cursor().position(), to);
    QVERIFY(caretOnPage(doc->pager().pageOfPosition(to)));
    QTest::keyClick(&view, Qt::Key_Home, Qt::ControlModifier);
    QCOMPARE(view.cursor().position(), from);
    QVERIFY(caretOnPage(doc->pager().pageOfPosition(from)));
    QSet<int> visited{view.currentPage()};
    for (int press = 0; press < 500; ++press) {
        QTest::keyClick(&view, Qt::Key_Down);
        const EndnoteSpan span = endnoteSpanAt(doc->story(), view.cursor().position());
        if (!(span.valid && span.editable)) break;
        visited.insert(view.currentPage());
        QRect caret;
        QVERIFY2(view.caretRect(&caret), qPrintable(QStringLiteral("Down, press %1").arg(press)));
    }
    QCOMPARE(int(visited.size()), doc->pager().pageOfPosition(to) - doc->pager().pageOfPosition(from) + 1);
}


// An endnote cited twice is set twice, and only the first copy is edited, though the second shows it: a click in the second copy is a click at the same place in the first, the note's own, and
// the view goes there; what is typed, taken out, or split there is in both copies, as one step with it; an undo gives both back; and a click in the first copy is in it, on the page clicked.
void WriteTests::aClickInALaterCopyOfAnEndnoteIsInTheFirstCopyAndTypingUpdatesBoth()
{
    QByteArray words;
    for (int i = 0; i < 1500; ++i) words += " word" + QByteArray::number(i);
    const QByteArray ends = "<w:endnotes " + QByteArray(kNs) + "><w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:endnote><w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\">" +
                            words + "</w:t></w:r></w:p></w:endnote></w:endnotes>";
    const QByteArray body = "<w:p><w:r><w:t>First</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p><w:p><w:r><w:t>Second</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>";
    auto doc = open(makeDocxWithParts(body, pageSection(), {{QStringLiteral("/word/endnotes.xml"), ends}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>"));
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 1200);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    // The words of each copy, as the paragraphs of the story that are its own.
    auto copyText = [&](bool first) {
        QStringList out;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) {
            const int serial = b.blockFormat().intProperty(prop::EndnoteNote);
            if (b.blockFormat().hasProperty(prop::EndnoteBlock) && (first ? serial > 0 : serial < 0)) out << b.text();
        }
        return out.join(QLatin1Char('\n'));
    };
    QTextBlock copyOne, copyTwo;
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) {
        const int serial = b.blockFormat().intProperty(prop::EndnoteNote);
        if (b.blockFormat().hasProperty(prop::EndnoteBlock) && serial > 0) copyOne = b;
        if (b.blockFormat().hasProperty(prop::EndnoteBlock) && serial < 0) copyTwo = b;
    }
    QVERIFY(copyOne.isValid() && copyTwo.isValid());
    QCOMPARE(copyText(true), copyText(false));
    const int copyOneLast = doc->pager().pageOfPosition(copyOne.position() + copyOne.length() - 1);
    const int copyTwoLast = doc->pager().pageOfPosition(copyTwo.position() + copyTwo.length() - 1);
    const int page = copyOneLast + 1;   // (a page that has the second copy's words only)
    QVERIFY2(page <= copyTwoLast, qPrintable(QStringLiteral("pages %1 to %2").arg(copyOneLast).arg(copyTwoLast)));
    // The caret is in the text; the view is then put at the page, and the top of its first line is clicked.
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, pagePoint(view, 0, 74, 80));
    QVERIFY(!endnoteSpanAt(doc->story(), view.cursor().position()).valid);
    view.verticalScrollBar()->setValue(view.verticalScrollBar()->value() + view.pageRect(page).top());
    QCoreApplication::processEvents();
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, pagePoint(view, page, 80, 75));
    const int pageStart = doc->pager().page(page).start.pos;
    const int expected = copyOne.position() + (pageStart - copyTwo.position());
    int id = 0;
    const int at = view.cursor().position();
    QVERIFY2(endnoteSpanAt(doc->story(), at).editable && doc->endnoteIdOf(doc->story()->findBlock(at), &id) && id == 2, qPrintable(QStringLiteral("the caret is at %1").arg(at)));
    QVERIFY2(std::abs(at - expected) <= 20, qPrintable(QStringLiteral("the caret is at %1, the same place of the first copy is %2").arg(at).arg(expected)));
    QRect caret;
    QVERIFY(view.caretRect(&caret) && view.viewport()->rect().contains(caret));   // (the view shows it, where it is: pages from the page that was clicked)
    QCOMPARE(view.currentPage(), doc->pager().pageOfPosition(at));
    // Typed, and taken out, and split: both copies say the same, in one step.
    QTest::keyClicks(&view, "ZZ ");
    QVERIFY(copyText(true).contains(QLatin1String("ZZ ")));
    QCOMPARE(copyText(false), copyText(true));
    QTest::keyClick(&view, Qt::Key_Backspace);
    QCOMPARE(copyText(false), copyText(true));
    QTest::keyClick(&view, Qt::Key_Return);
    QCOMPARE(copyText(false), copyText(true));
    QCOMPARE(copyText(true).count(QLatin1Char('\n')), 1);
    const QByteArray part = notesPartOf(doc->toDocx());
    QCOMPARE(int(part.count("<w:endnote w:id=\"2\"")), 1);
    QVERIFY(part.contains("ZZ"));
    view.undo();   // (the paragraph that was split)
    QCOMPARE(copyText(false), copyText(true));
    QCOMPARE(copyText(true).count(QLatin1Char('\n')), 0);
    while (doc->canUndo()) doc->undo();
    QCOMPARE(copyText(false), copyText(true));
    QVERIFY(!copyText(true).contains(QLatin1String("ZZ ")));
    // A click in the first copy is in it, on the page clicked.
    const int firstPage = doc->pager().pageOfPosition(copyOne.position());
    view.verticalScrollBar()->setValue(view.verticalScrollBar()->value() + view.pageRect(firstPage + 1).top());
    QCoreApplication::processEvents();
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, pagePoint(view, firstPage + 1, 200, 300));
    const EndnoteSpan span = endnoteSpanAt(doc->story(), view.cursor().position());
    QVERIFY(span.valid && span.editable);
    QCOMPARE(view.currentPage(), firstPage + 1);
}


// ---- notes: what an ordinary edit costs

// The same ordinary edits (typing, Enter, Backspace, Delete, bold, copy and paste, taking text out, a style) at the top of a text that has no note, a few, and hundreds, the references being
// after the places of the edits: counted in what the notes' bookkeeping looks at, there is none of it without notes, and with them it does not grow with their number.
void WriteTests::ordinaryEditsCostTheSameWorkWhateverTheNumberOfNotes()
{
    constexpr int edits = 300;
    auto workWith = [&](int notes) {
        QByteArray body = paragraphs(300, 8);
        QList<std::pair<int, QByteArray>> footnotes;
        QByteArray ends = "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote();
        for (int i = 1; i <= notes; ++i) {
            const QByteArray n = QByteArray::number(i);
            body += "<w:p><w:r><w:t>cites " + n + "</w:t></w:r><w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr><w:footnoteReference w:id=\"" + n + "\"/></w:r><w:r><w:endnoteReference w:id=\"" + n + "\"/></w:r></w:p>";
            footnotes << std::make_pair(i, plainRun(" note " + n));
            ends += "<w:endnote w:id=\"" + n + "\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t> end " + n + "</w:t></w:r></w:p></w:endnote>";
        }
        ends += "</w:endnotes>";
        const QByteArray zip = notes == 0 ? makeDocx(body, kStyles)
                                          : makeDocxWithParts(body, pageSection(), {{QStringLiteral("/word/footnotes.xml"), footnotesPart(footnotes)}, {QStringLiteral("/word/endnotes.xml"), ends}},
                                                              "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>", noteStyles());
        auto doc = open(zip);
        [&] { QVERIFY(doc); }();
        QTextCursor c(doc->story());
        c.setPosition(5);
        insertParagraph(*doc, c);   // (the first edits that can change the notes: the references are looked for once)
        deleteBack(*doc, c, false);
        resetNoteWorkForTest();
        QTextDocumentFragment held;
        for (int i = 0; i < edits; ++i) {
            const int at = 5 + (i * 211) % 1200;
            c.setPosition(at);
            switch (i % 8) {
            case 0: insertText(*doc, c, QStringLiteral("x")); break;
            case 1: insertParagraph(*doc, c); break;
            case 2: deleteBack(*doc, c, false); break;
            case 3: deleteForward(*doc, c, false); break;
            case 4: c.setPosition(at + 40, QTextCursor::KeepAnchor); toggleBold(*doc, c); break;
            case 5: c.setPosition(at + 30, QTextCursor::KeepAnchor); held = selectionFragment(c); c.setPosition(at + 50); pasteFragment(*doc, c, held); break;
            case 6: c.setPosition(at + 20, QTextCursor::KeepAnchor); removeSelection(*doc, c); break;
            default: c.setPosition(at + 60, QTextCursor::KeepAnchor); applyParagraphStyle(*doc, c, QStringLiteral("Heading2")); break;
            }
        }
        return noteWorkForTest();
    };
    const qint64 none = workWith(0), few = workWith(10), many = workWith(400);
    QVERIFY2(none == 0, qPrintable(QStringLiteral("%1 places looked at in a text with no note").arg(none)));
    QVERIFY2(many <= 40 * edits, qPrintable(QStringLiteral("%1 places looked at for %2 edits with 400 notes (%3 with 10)").arg(many).arg(edits).arg(few)));
    QVERIFY2(many <= few + 12 * edits, qPrintable(QStringLiteral("%1 places looked at for %2 edits with 400 notes, %3 with 10").arg(many).arg(edits).arg(few)));
}


// ---- endnotes: a keystroke in a note that has copies

// A keystroke in an endnote that is cited twice sets its other copy again, and that copy only: counted in what the notes' bookkeeping looks at and the endnote paragraphs it makes, it does not
// grow with the number of other endnotes (a keystroke that set all of them again would make as many paragraphs as the notes there are), and the copies say the same.
void WriteTests::aKeystrokeInAnEndnoteWithCopiesCostsWhatTheNoteIsNotWhatTheOthersAre()
{
    constexpr int keys = 40;
    auto workPerKey = [&](int others) {
        QByteArray body, ends = "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote();
        for (int i = 1; i <= others; ++i) {
            const QByteArray n = QByteArray::number(i);
            body += "<w:p><w:r><w:t>cites " + n + "</w:t></w:r><w:r><w:endnoteReference w:id=\"" + n + "\"/></w:r></w:p>";
            ends += "<w:endnote w:id=\"" + n + "\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t> end " + n + "</w:t></w:r></w:p></w:endnote>";
        }
        const QByteArray twice = QByteArray::number(others + 1);
        body += "<w:p><w:r><w:t>twice</w:t></w:r><w:r><w:endnoteReference w:id=\"" + twice + "\"/></w:r></w:p><w:p><w:r><w:t>again</w:t></w:r><w:r><w:endnoteReference w:id=\"" + twice + "\"/></w:r></w:p>";
        ends += "<w:endnote w:id=\"" + twice + "\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t> the note cited twice</w:t></w:r></w:p><w:p><w:r><w:t>and its second paragraph</w:t></w:r></w:p></w:endnote></w:endnotes>";
        auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), ends}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>"));
        [&] { QVERIFY(doc); }();
        int from = 0, to = 0;
        [&] { QVERIFY(doc->endnoteWords(others + 1, &from, &to)); }();
        QTextCursor c(doc->story());
        c.setPosition(to);
        insertText(*doc, c, QStringLiteral("x"));   // (the first edit: the references are looked for once)
        resetNoteWorkForTest();
        resetEndnoteParagraphsMadeForTest();
        for (int i = 0; i < keys; ++i) insertText(*doc, c, QStringLiteral("y"));
        const qint64 work = noteWorkForTest() + endnoteParagraphsMadeForTest();
        // The copies say the same, in the story and in what is saved.
        auto copy = [&](bool first) {
            QStringList out;
            for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) {
                const int serial = b.blockFormat().intProperty(prop::EndnoteNote);
                int found = 0;
                if (b.blockFormat().hasProperty(prop::EndnoteBlock) && (first ? serial > 0 : serial < 0) && doc->endnoteIdOf(b, &found) && found == others + 1) out << b.text();
            }
            return out.join(QLatin1Char('\n'));
        };
        const QString typed = QString(QLatin1Char('x')) + QString(keys, QLatin1Char('y'));
        [&] {
            QVERIFY2(copy(true).contains(typed), qPrintable(copy(true)));
            QCOMPARE(copy(false), copy(true));
            QVERIFY(notePartText(notesPartOf(doc->toDocx()), "endnote", others + 1).contains(typed));
        }();
        return double(work) / keys;
    };
    const double few = workPerKey(20), many = workPerKey(2500);
    QVERIFY2(many <= 60, qPrintable(QStringLiteral("%1 places and paragraphs for a keystroke with 2500 other endnotes, %2 with 20").arg(many).arg(few)));
    QVERIFY2(many <= few + 30, qPrintable(QStringLiteral("%1 places and paragraphs for a keystroke with 2500 other endnotes, %2 with 20").arg(many).arg(few)));
}


// The copies set again alone give the story that setting every note again gives: after typing, a paragraph made, one left empty, one taken out, in a note cited three times between others.
void WriteTests::theCopiesOfAnEditedEndnoteSetAgainAloneAreWhatSettingAllOfThemAgainGives()
{
    QByteArray body, ends = "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote();
    for (int i = 1; i <= 4; ++i) {
        const QByteArray n = QByteArray::number(i);
        body += "<w:p><w:r><w:t>cites " + n + "</w:t></w:r><w:r><w:endnoteReference w:id=\"" + n + "\"/></w:r></w:p><w:p><w:r><w:t>and the note cited more than once</w:t></w:r><w:r><w:endnoteReference w:id=\"9\"/></w:r></w:p>";
        ends += "<w:endnote w:id=\"" + n + "\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t> end " + n + "</w:t></w:r></w:p></w:endnote>";
    }
    ends += "<w:endnote w:id=\"9\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> the first paragraph of it</w:t></w:r></w:p><w:p/><w:p><w:r><w:rPr><w:b/></w:rPr><w:t>the last one, in bold</w:t></w:r></w:p></w:endnote></w:endnotes>";
    // (Three references to it are the three copies: the body above cites it four times, so drop the fourth.)
    const int cut = body.lastIndexOf("<w:p><w:r><w:t>and the note cited more than once");
    body.remove(cut, body.indexOf("</w:p>", cut) + 6 - cut);
    auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), ends}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>"));
    QVERIFY(doc);
    // The notes' paragraphs: their text, and every property of their paragraphs' and fragments' formats, with the copies' numbers by the order they come in.
    auto dump = [](WriteDoc &w) {
        auto props = [](const QTextFormat &f) {
            QString s;
            const QMap<int, QVariant> all = f.properties();
            for (auto it = all.cbegin(); it != all.cend(); ++it)
                s += QString::number(it.key()) + QLatin1Char('=') + (it.value().typeId() == QMetaType::QByteArray ? QString::fromLatin1(it.value().toByteArray().toHex()) : it.value().toString()) + QLatin1Char(';');
            return s;
        };
        QString out;
        QHash<int, int> order;
        for (QTextBlock b = w.story()->begin(); b.isValid(); b = b.next()) {
            if (!b.blockFormat().hasProperty(prop::EndnoteBlock)) continue;
            QTextBlockFormat bf = b.blockFormat();
            const int serial = bf.intProperty(prop::EndnoteNote);
            if (serial != 0) {
                if (!order.contains(std::abs(serial))) order.insert(std::abs(serial), order.size() + 1);
                bf.setProperty(prop::EndnoteNote, serial < 0 ? -order.value(-serial) : order.value(serial));
            }
            out += QStringLiteral("[%1|%2]").arg(b.text(), props(bf));
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) out += QStringLiteral("{%1|%2}").arg(it.fragment().text(), props(it.fragment().charFormat()));
            out += QLatin1Char('\n');
        }
        return out;
    };
    auto cursorAt = [&](bool end) {
        int from = 0, to = 0;
        [&] { QVERIFY(doc->endnoteWords(9, &from, &to)); }();
        QTextCursor c(doc->story());
        c.setPosition(end ? to : from + 3);
        return c;
    };
    resetEndnoteParagraphsMadeForTest();
    auto same = [&](const char *what) {
        // (The edit made no endnote paragraph by setting the notes: the copies were set again alone, which is what is looked at.)
        QVERIFY2(endnoteParagraphsMadeForTest() == 0, what);
        QVERIFY2(doc->endnoteSequenceMatchesStoryForTest(), what);   // (and the places of the references are followed through it)
        const QString alone = dump(*doc);
        doc->rebuildEndnoteTails();   // (every note set again)
        resetEndnoteParagraphsMadeForTest();
        const QString all = dump(*doc);
        QVERIFY2(alone == all, qPrintable(QStringLiteral("after %1:\n%2\nagainst setting them all again:\n%3").arg(QLatin1String(what), alone, all)));
    };
    {
        QSet<int> copies;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) {
            int id = 0;
            if (doc->endnoteIdOf(b, &id) && id == 9) copies.insert(b.blockFormat().intProperty(prop::EndnoteNote));
        }
        QCOMPARE(int(copies.size()), 3);
    }
    { QTextCursor c = cursorAt(true); insertText(*doc, c, QStringLiteral("ab")); }
    same("typing at the end");
    { QTextCursor c = cursorAt(true); insertParagraph(*doc, c); insertText(*doc, c, QStringLiteral("new")); }
    same("a paragraph made");
    { QTextCursor c = cursorAt(true); insertParagraph(*doc, c); }
    same("a paragraph left empty at the end");
    { QTextCursor c = cursorAt(false); insertText(*doc, c, QStringLiteral("z")); }
    same("typing inside the first paragraph");
    { QTextCursor c = cursorAt(false); deleteBack(*doc, c, false); deleteBack(*doc, c, false); }
    same("taking two out");
    { QTextCursor c = cursorAt(true); deleteBack(*doc, c, false); insertText(*doc, c, QStringLiteral("again")); }
    same("the empty paragraph taken out and typed in");
}


// Where notes are set at the end of their sections, a copy cannot be found by its number among the story's last paragraphs: an edit in the first copy sets the notes again, and the other copy says the same.
void WriteTests::aCopyOfAnEndnoteAtTheEndOfASectionStillShowsWhatTheFirstCopySays()
{
    const QByteArray pr = "<w:endnotePr><w:pos w:val=\"sectEnd\"/></w:endnotePr>";
    const QByteArray body = "<w:p><w:r><w:t>First body</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>"
                            "<w:p><w:pPr>" + sectionXml(6000, 6000, 720, pr) + "</w:pPr><w:r><w:t>Second body</w:t></w:r></w:p>"
                            "<w:p><w:r><w:t>Third body</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>";
    const QByteArray notes = "<w:endnotes " + QByteArray(kNs) + "><w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:t>Sources</w:t></w:r></w:p></w:endnote>"
                             "<w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Alpha body note</w:t></w:r></w:p></w:endnote></w:endnotes>";
    auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720, pr), {{QStringLiteral("/word/endnotes.xml"), notes}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>"));
    QVERIFY(doc);
    int from = 0, to = 0;
    QVERIFY(doc->endnoteWords(2, &from, &to));
    resetEndnoteParagraphsMadeForTest();
    QTextCursor c(doc->story());
    c.setPosition(to);
    insertText(*doc, c, QStringLiteral("XY"));
    QVERIFY2(endnoteParagraphsMadeForTest() > 0, "the notes were set again");
    int shown = 0;
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
        if (b.blockFormat().hasProperty(prop::EndnoteBlock) && b.text().contains(QLatin1String("Alpha body noteXY"))) ++shown;
    QCOMPARE(shown, 2);
    QCOMPARE(doc->sectionCount(), 2);
    QVERIFY(notePartText(notesPartOf(doc->toDocx()), "endnote", 2).contains(QLatin1String("Alpha body noteXY")));
}


// ---- endnotes: the undo steps of an edit in a note that has copies

// An edit in an endnote that has copies and the setting again of the copies are one step of the undo stack: one undo puts every copy back as it was (the first copy and the others
// together, never one without the other), and one redo does it again.
void WriteTests::undoAndRedoTakeEveryCopyOfAnEditedEndnoteTogether_data()
{
    QTest::addColumn<bool>("atSectionEnds");
    QTest::newRow("at the end of the story") << false;
    QTest::newRow("at the end of each section") << true;
}

void WriteTests::undoAndRedoTakeEveryCopyOfAnEditedEndnoteTogether()
{
    QFETCH(bool, atSectionEnds);
    const QByteArray pr = "<w:endnotePr><w:pos w:val=\"sectEnd\"/></w:endnotePr>";
    const QByteArray body = "<w:p><w:r><w:t>First body</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>"
                            "<w:p><w:pPr>" + (atSectionEnds ? sectionXml(6000, 6000, 720, pr) : sectionXml(6000, 6000, 720)) + "</w:pPr><w:r><w:t>Second body</w:t></w:r></w:p>"
                            "<w:p><w:r><w:t>Third body</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>";
    const QByteArray notes = "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote() + "<w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> shared</w:t></w:r></w:p></w:endnote></w:endnotes>";
    auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720, atSectionEnds ? pr : QByteArray()), {{QStringLiteral("/word/endnotes.xml"), notes}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>"));
    QVERIFY(doc);
    // The notes' paragraphs, copy by copy: the first copy's, then the shown ones'.
    auto copies = [&] {
        QStringList first, others;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) {
            const int serial = b.blockFormat().intProperty(prop::EndnoteNote);
            if (!b.blockFormat().hasProperty(prop::EndnoteBlock) || serial == 0) continue;
            (serial > 0 ? first : others) << b.text();
        }
        return std::make_pair(first.join(QLatin1Char('|')), others.join(QLatin1Char('|')));
    };
    auto state = [&] {
        const auto both = copies();
        return both.first + QStringLiteral(" // ") + both.second;
    };
    auto cursorAt = [&](int back) {
        int from = 0, to = 0;
        [&] { QVERIFY(doc->endnoteWords(2, &from, &to)); }();
        QTextCursor c(doc->story());
        c.setPosition(to - back);
        return c;
    };
    QStringList states{state()};
    auto step = [&](const QString &what, const std::function<void()> &edit) {
        edit();
        const auto both = copies();
        QVERIFY2(both.first == both.second, qPrintable(what + QStringLiteral(": ") + state()));   // (every copy says what the first says)
        states << state();
    };
    step(QStringLiteral("a character typed"), [&] { QTextCursor c = cursorAt(0); insertText(*doc, c, QStringLiteral("X")); });
    step(QStringLiteral("another typed"), [&] { QTextCursor c = cursorAt(0); insertText(*doc, c, QStringLiteral("Y")); });
    step(QStringLiteral("Backspace"), [&] { QTextCursor c = cursorAt(0); deleteBack(*doc, c, false); });
    step(QStringLiteral("Delete"), [&] { QTextCursor c = cursorAt(1); deleteForward(*doc, c, false); });
    step(QStringLiteral("Enter"), [&] { QTextCursor c = cursorAt(0); insertParagraph(*doc, c); });
    step(QStringLiteral("typed in the new paragraph"), [&] { QTextCursor c = cursorAt(0); insertText(*doc, c, QStringLiteral("Q")); });
    // Each undo puts back the story as it was before the edit, copies and all, and each redo as it was after.
    for (int i = int(states.size()) - 2; i >= 0; --i) {
        QVERIFY(doc->undo());
        QCOMPARE(state(), states[i]);
    }
    for (int i = 1; i < states.size(); ++i) {
        QVERIFY(doc->redo());
        QCOMPARE(state(), states[i]);
    }
}


// Typing where there are no copies is as it was: consecutive characters are one step of the undo stack.
void WriteTests::consecutiveCharactersTypedInAnEndnoteWithoutCopiesAreOneUndoStep()
{
    const QByteArray body = "<w:p><w:r><w:t>First</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>";
    const QByteArray notes = "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote() + "<w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> alone</w:t></w:r></w:p></w:endnote></w:endnotes>";
    auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), notes}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>"));
    QVERIFY(doc);
    int from = 0, to = 0;
    QVERIFY(doc->endnoteWords(2, &from, &to));
    QTextCursor c(doc->story());
    c.setPosition(to);
    for (const QString &typed : {QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")}) insertText(*doc, c, typed);
    QVERIFY(doc->story()->toPlainText().contains(QLatin1String(" alone" "abc")));
    QVERIFY(doc->undo());
    QVERIFY2(!doc->story()->toPlainText().contains(QLatin1String("alonea")), qPrintable(doc->story()->toPlainText()));
}


// ---- pictures, page setup, and the ruler (beta wave C)

QByteArray WriteTests::pngOf(int width, int height, int shade)
{
    QImage image(width, height, QImage::Format_RGB32);
    image.fill(QColor(shade & 255, (shade >> 8) & 255, 90));
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return bytes;
}


int WriteTests::picturesIn(const QTextDocument *story)
{
    int n = 0;
    for (int i = 0; i < story->characterCount() - 1; ++i) n += isPictureAt(story, i);
    return n;
}


// The first line of the first page's text, as the page lays it out round the objects it holds.
QRectF WriteTests::firstLineRect(WriteDoc &doc)
{
    const auto page = doc.pager().materialize(0);
    if (!page || page->boxes.isEmpty()) return {};
    const auto *item = dynamic_cast<const TextItem *>(page->doc->item(page->boxes[0]));
    if (!item) return {};
    LayoutCache cache;
    const LayoutCache::FrameLayout fl = cache.textFrame(*page->doc, *item, page->number, RenderOptions());
    if (!fl.layout) return {};
    const auto lines = fl.layout->lineInfo(0);
    return lines.isEmpty() ? QRectF() : lines.first().rect;
}


// The objects set at places on the first page: the rectangle of each picture.
QVector<QRectF> WriteTests::pictureItemsOfPage(WriteDoc &doc)
{
    QVector<QRectF> out;
    const auto page = doc.pager().materialize(0);
    if (!page) return out;
    for (const auto &it : page->doc->pages[0]->items)
        if (it->type() == ItemType::Picture && it->id.startsWith(QLatin1String("flow-object-"))) out << it->rect;
    return out;
}


QString WriteTests::longText() { return QString(QStringLiteral("The court held that the provision applies to every appeal and every judgment. ")).repeated(40); }


void WriteTests::aPictureInsertedFromAFileIsOneStepAndSavesAndOpensTheSame()
{
    auto doc = textDoc({"Before after"});
    QTextCursor c(doc->story());
    c.setPosition(7);
    const QByteArray png = pngOf(96, 48);
    QCOMPARE(insertPicture(*doc, c, png, QStringLiteral("photo")), PictureResult::Ok);
    QVERIFY(isPictureAt(doc->story(), 7));
    QCOMPARE(c.position(), 8);
    const PictureInfo info = pictureAt(*doc, doc->story(), 7);
    QVERIFY(info.valid && info.editable && !info.floating);
    QVERIFY(info.wrap == PictureWrap::Inline);
    QCOMPARE(info.width, 72.0);   // 96 pixels at 96 to the inch
    QCOMPARE(info.height, 36.0);
    QVERIFY(doc->isModified());
    // One Undo takes the picture away, one Redo puts it back.
    QVERIFY(doc->undo());
    QVERIFY(!isPictureAt(doc->story(), 7));
    QCOMPARE(plain(*doc), QStringLiteral("Before after"));
    QVERIFY(!doc->canUndo());
    QVERIFY(doc->redo());
    QVERIFY(isPictureAt(doc->story(), 7));
    const ItemPtr shown = inlineObjectOf(*doc);
    QVERIFY(shown);
    QVERIFY(shown->type() == ItemType::Picture);
    QCOMPARE(shown->rect.width(), 72.0);
    // Saved: the picture's file is a part of its own, the element well formed.
    const QByteArray saved = doc->toDocx();
    office::Package pkg;
    QVERIFY(pkg.read(saved));
    QCOMPARE(pkg.part(QStringLiteral("/word/media/image1.png")), png);
    QCOMPARE(pkg.contentType(QStringLiteral("/word/media/image1.png")), QStringLiteral("image/png"));
    const QByteArray docXml = documentXml(saved);
    wml::XmlTree tree;
    QVERIFY(tree.parse(docXml));
    QVERIFY(docXml.contains("<wp:inline"));
    QVERIFY(docXml.contains("cx=\"914400\""));
    // And opened again, it is the same.
    auto back = open(saved);
    QVERIFY(back);
    QVERIFY(isPictureAt(back->story(), 7));
    const PictureInfo again = pictureAt(*back, back->story(), 7);
    QCOMPARE(again.width, 72.0);
    QCOMPARE(again.height, 36.0);
    QCOMPARE(plain(*back), plain(*doc));
    // The same picture put in again is the same part of the file.
    QTextCursor d(doc->story());
    d.setPosition(0);
    QCOMPARE(insertPicture(*doc, d, png, QStringLiteral("again")), PictureResult::Ok);
    office::Package twice;
    QVERIFY(twice.read(doc->toDocx()));
    QVERIFY(twice.part(QStringLiteral("/word/media/image2.png")).isEmpty());
    QCOMPARE(picturesIn(doc->story()), 2);
}


void WriteTests::aPictureLargerThanTheColumnIsMadeToFitIt()
{
    auto doc = textDoc({"x"});
    QTextCursor c(doc->story());
    QCOMPARE(insertPicture(*doc, c, pngOf(3000, 1500)), PictureResult::Ok);   // 2250 by 1125 points as it asks
    const PictureInfo info = pictureAt(*doc, doc->story(), 0);
    const double column = doc->flow().section(0).columnWidth();
    QVERIFY2(info.width <= column + 0.01 && info.width > column - 1, qPrintable(QString::number(info.width)));
    QVERIFY(std::abs(info.width / info.height - 2.0) < 0.01);
}


void WriteTests::aPictureFromAnImageIsKeptAsPngAndSavesAndOpensTheSame()
{
    QImage image(30, 20, QImage::Format_ARGB32);
    image.fill(QColor(200, 30, 30, 255));
    image.setPixelColor(3, 4, QColor(0, 0, 0, 0));
    auto doc = WriteDoc::blank();
    QTextCursor c(doc->story());
    typeText(*doc, c, QStringLiteral("Paste here: "));
    QCOMPARE(insertPictureImage(*doc, c, image), PictureResult::Ok);
    const int at = 12;
    QVERIFY(isPictureAt(doc->story(), at));
    QCOMPARE(pictureAt(*doc, doc->story(), at).width, 22.5);
    const QByteArray saved = doc->toDocx();
    office::Package pkg;
    QVERIFY(pkg.read(saved));
    QImage kept;
    QVERIFY(kept.loadFromData(pkg.part(QStringLiteral("/word/media/image1.png"))));
    QCOMPARE(kept.size(), image.size());
    QCOMPARE(kept.pixelColor(3, 4).alpha(), 0);
    QCOMPARE(kept.pixelColor(5, 5), QColor(200, 30, 30));
    auto back = open(saved);
    QVERIFY(back);
    QVERIFY(isPictureAt(back->story(), at));
    QCOMPARE(plain(*back), plain(*doc));
    QVERIFY(doc->undo());
    QVERIFY(!isPictureAt(doc->story(), at));
}


void WriteTests::eachWrapOfAPictureShowsOnThePageIsOneStepAndSavesAndOpensTheSame()
{
    struct Case {
        PictureWrap wrap;
        const char *token;
    };
    const Case cases[] = {{PictureWrap::Square, "wrapSquare"}, {PictureWrap::Tight, "wrapTight"}, {PictureWrap::TopAndBottom, "wrapTopAndBottom"},
                          {PictureWrap::BehindText, "behindDoc=\"1\""}, {PictureWrap::InFrontOfText, "wrapNone"}};
    for (const Case &k : cases) {
        auto doc = textDoc({longText()});
        QTextCursor c(doc->story());
        QCOMPARE(insertPicture(*doc, c, pngOf(160, 160)), PictureResult::Ok);   // 120 points square, in line with the text
        QVERIFY(pictureItemsOfPage(*doc).isEmpty());
        QVERIFY2(firstLineRect(*doc).height() >= 119, qPrintable(QString::number(firstLineRect(*doc).height())));
        QVERIFY(setPictureWrap(*doc, 0, k.wrap));
        const PictureInfo info = pictureAt(*doc, doc->story(), 0);
        QVERIFY(info.floating && info.wrap == k.wrap);
        QVERIFY(!setPictureWrap(*doc, 0, k.wrap));   // (the wrap it has is no step)
        // On the page: a picture set at a place, and the text going round it as the wrap says.
        QCOMPARE(pictureItemsOfPage(*doc).size(), 1);
        const QRectF line = firstLineRect(*doc);
        if (k.wrap == PictureWrap::Square || k.wrap == PictureWrap::Tight) QVERIFY2(line.left() >= 118, qPrintable(QString::number(line.left())));
        else if (k.wrap == PictureWrap::TopAndBottom) QVERIFY2(line.top() >= 118, qPrintable(QString::number(line.top())));
        else QVERIFY2(line.left() < 2 && line.height() < 40, qPrintable(QString::number(line.left())));
        // One Undo, and the picture is in line again; one more takes the picture away.
        QVERIFY(doc->undo());
        QVERIFY(pictureAt(*doc, doc->story(), 0).wrap == PictureWrap::Inline);
        QVERIFY(pictureItemsOfPage(*doc).isEmpty());
        QVERIFY(doc->redo());
        QVERIFY(pictureAt(*doc, doc->story(), 0).wrap == k.wrap);
        // Saved, and opened again.
        const QByteArray saved = doc->toDocx();
        const QByteArray docXml = documentXml(saved);
        wml::XmlTree tree;
        QVERIFY(tree.parse(docXml));
        QVERIFY2(docXml.contains(k.token), k.token);
        auto back = open(saved);
        QVERIFY(back);
        const PictureInfo again = pictureAt(*back, back->story(), 0);
        QVERIFY2(again.floating && again.wrap == k.wrap, k.token);
        QCOMPARE(pictureItemsOfPage(*back).size(), 1);
        // Back in line.
        QVERIFY(setPictureWrap(*doc, 0, PictureWrap::Inline));
        QVERIFY(!pictureAt(*doc, doc->story(), 0).floating);
        QVERIFY(documentXml(doc->toDocx()).contains("<wp:inline"));
    }
}


void WriteTests::aPictureMarkedAsDecorativeHasNoDescriptionAndKeepsTheMarkWhenSaved()
{
    auto doc = textDoc({longText()});
    QTextCursor c(doc->story());
    QCOMPARE(insertPicture(*doc, c, pngOf(160, 160), QString(), PictureWrap::Inline, QStringLiteral("A square")), PictureResult::Ok);
    QVERIFY(!pictureAt(*doc, doc->story(), 0).decorative);
    QVERIFY(!setPictureDecorative(*doc, 0, false));   // (nothing to take off)
    QVERIFY(setPictureDecorative(*doc, 0, true));
    PictureInfo info = pictureAt(*doc, doc->story(), 0);
    QVERIFY(info.decorative && info.alt.isEmpty() && info.editable);
    QVERIFY(!setPictureDecorative(*doc, 0, true));
    // in the file as the newer format has it, inside the picture's properties, and read back
    const QByteArray saved = doc->toDocx();
    const QByteArray xml = documentXml(saved);
    QVERIFY2(xml.contains("{C183D7F6-B498-43B3-948B-1728B52AA6E4}") && xml.contains("<adec:decorative"), xml.constData());
    wml::XmlTree tree;
    QVERIFY(tree.parse(xml));
    auto back = open(saved);
    QVERIFY(back && pictureAt(*back, back->story(), 0).decorative);
    // one step undoes it; off takes the mark away and leaves nothing behind
    QVERIFY(doc->undo());
    QVERIFY(!pictureAt(*doc, doc->story(), 0).decorative);
    QCOMPARE(pictureAt(*doc, doc->story(), 0).alt, QStringLiteral("A square"));
    QVERIFY(doc->redo());
    QVERIFY(setPictureDecorative(*doc, 0, false));
    QVERIFY(!pictureAt(*doc, doc->story(), 0).decorative);
    QVERIFY(!documentXml(doc->toDocx()).contains("C183D7F6"));
    QVERIFY(!documentXml(doc->toDocx()).contains("extLst"));
}


void WriteTests::aPictureIsSizedPlacedAndDescribedInOneStepEach()
{
    auto doc = textDoc({longText()});
    QTextCursor c(doc->story());
    QCOMPARE(insertPicture(*doc, c, pngOf(160, 160)), PictureResult::Ok);
    // Size.
    QVERIFY(setPictureSize(*doc, 0, 50, 25));
    PictureInfo info = pictureAt(*doc, doc->story(), 0);
    QCOMPARE(info.width, 50.0);
    QCOMPARE(info.height, 25.0);
    QVERIFY(!setPictureSize(*doc, 0, 50, 25));
    QVERIFY(doc->undo());
    QCOMPARE(pictureAt(*doc, doc->story(), 0).width, 120.0);
    QVERIFY(doc->redo());
    QCOMPARE(inlineObjectOf(*doc)->rect.width(), 50.0);
    // A place on the page (a picture in line becomes one set at a place, the text going round it).
    QVERIFY(setPicturePlace(*doc, 0, 200, 300));
    info = pictureAt(*doc, doc->story(), 0);
    QVERIFY(info.floating && info.wrap == PictureWrap::Square);
    QVERIFY(info.placeIsOffset);
    QCOMPARE(info.left, 200.0);
    QCOMPARE(info.top, 300.0);
    QCOMPARE(info.width, 50.0);
    {
        const QVector<QRectF> items = pictureItemsOfPage(*doc);
        QCOMPARE(items.size(), 1);
        QVERIFY(std::abs(items[0].left() - 200) < 0.5 && std::abs(items[0].top() - 300) < 0.5);
        QCOMPARE(items[0].size(), QSizeF(50, 25));
    }
    QVERIFY(setPicturePlace(*doc, 0, 100, 150));
    QVERIFY(doc->undo());
    QCOMPARE(pictureAt(*doc, doc->story(), 0).left, 200.0);
    QVERIFY(doc->redo());
    // Size and place together are one step.
    QVERIFY(setPictureBox(*doc, 0, 80, 40, QPointF(90, 120)));
    QVERIFY(doc->undo());
    info = pictureAt(*doc, doc->story(), 0);
    QCOMPARE(info.width, 50.0);
    QCOMPARE(info.left, 100.0);
    QVERIFY(doc->redo());
    // Alt text.
    QVERIFY(setPictureAltText(*doc, 0, QStringLiteral("A red square with <angle brackets> & \"quotes\"")));
    QVERIFY(!setPictureAltText(*doc, 0, QStringLiteral("A red square with <angle brackets> & \"quotes\"")));
    QVERIFY(doc->undo());
    QVERIFY(pictureAt(*doc, doc->story(), 0).alt.isEmpty());
    QVERIFY(doc->redo());
    const QByteArray saved = doc->toDocx();
    wml::XmlTree tree;
    QVERIFY(tree.parse(documentXml(saved)));
    auto back = open(saved);
    QVERIFY(back);
    info = pictureAt(*back, back->story(), 0);
    QCOMPARE(info.alt, QStringLiteral("A red square with <angle brackets> & \"quotes\""));
    QCOMPARE(info.width, 80.0);
    QCOMPARE(info.height, 40.0);
    QCOMPARE(info.left, 90.0);
    QCOMPARE(info.top, 120.0);
    // What the alt text may not hold is left out, and it is as long as it may be.
    QVERIFY(setPictureAltText(*doc, 0, QString(QChar(1)) + QStringLiteral("ok") + QString(100000, QLatin1Char('x'))));
    const QString kept = pictureAt(*doc, doc->story(), 0).alt;
    QCOMPARE(kept.size(), kMostAltTextLength);
    QVERIFY(kept.startsWith(QStringLiteral(" ok")));
    wml::XmlTree again;
    QVERIFY(again.parse(documentXml(doc->toDocx())));
}


void WriteTests::aPictureInTheTextMovesToAnotherPlaceInOneStep()
{
    auto doc = textDoc({"ab cd ef"});
    QTextCursor c(doc->story());
    c.setPosition(1);
    QCOMPARE(insertPicture(*doc, c, pngOf(40, 40)), PictureResult::Ok);
    QVERIFY(isPictureAt(doc->story(), 1));
    QVERIFY(!movePictureInText(*doc, 1, 1));
    QVERIFY(!movePictureInText(*doc, 1, 2));
    QVERIFY(movePictureInText(*doc, 1, 7));
    QVERIFY(isPictureAt(doc->story(), 6));
    QCOMPARE(plain(*doc).remove(QChar::ObjectReplacementCharacter), QStringLiteral("ab cd ef"));
    QVERIFY(doc->undo());
    QVERIFY(isPictureAt(doc->story(), 1));
    QVERIFY(doc->redo());
    QVERIFY(isPictureAt(doc->story(), 6));
    QVERIFY(open(doc->toDocx()));
    QCOMPARE(picturesIn(doc->story()), 1);
}


quint32 WriteTests::crc32Of(const QByteArray &bytes)
{
    quint32 crc = 0xFFFFFFFFu;
    for (const char ch : bytes) {
        crc ^= quint8(ch);
        for (int k = 0; k < 8; ++k) crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}


void WriteTests::aPictureThatCannotBeReadOrIsTooLargeChangesNothing()
{
    auto doc = textDoc({"Some text"});
    const QByteArray before = doc->toDocx();
    QTextCursor c(doc->story());
    c.setPosition(4);
    QVERIFY(insertPicture(*doc, c, QByteArray("not a picture at all"), QStringLiteral("x")) == PictureResult::Unreadable);
    QVERIFY(insertPicture(*doc, c, QByteArray(), QStringLiteral("x")) == PictureResult::Unreadable);
    QVERIFY(insertPictureImage(*doc, c, QImage()) == PictureResult::Unreadable);
    // A picture whose header says it is 30001 pixels across.
    QByteArray wide = pngOf(2, 2);
    QVERIFY(wide.size() > 33);
    wide[16] = 0;
    wide[17] = 0;
    wide[18] = char(30001 >> 8);
    wide[19] = char(30001 & 255);
    const quint32 crc = crc32Of(wide.mid(12, 17));
    wide[29] = char(crc >> 24);
    wide[30] = char(crc >> 16);
    wide[31] = char(crc >> 8);
    wide[32] = char(crc);
    QVERIFY(insertPicture(*doc, c, wide, QStringLiteral("x")) == PictureResult::TooLarge);
    // More bytes than a picture may be.
    QVERIFY(insertPicture(*doc, c, QByteArray(int(kMostPictureBytes) + 1, 'x'), QStringLiteral("x")) == PictureResult::TooLarge);
    // Not into a story that is not the text's.
    QTextDocument other;
    QTextCursor oc(&other);
    QVERIFY(insertPicture(*doc, oc, pngOf(4, 4), QStringLiteral("x")) == PictureResult::Refused);
    QVERIFY(!doc->pictureNotice.isEmpty());
    QCOMPARE(plain(*doc), QStringLiteral("Some text"));
    QVERIFY(!doc->canUndo());
    QVERIFY(!doc->isModified());
    QCOMPARE(documentXml(doc->toDocx()), documentXml(before));
    office::Package pkg;
    QVERIFY(pkg.read(doc->toDocx()));
    for (const QString &name : pkg.partNames()) QVERIFY2(!name.contains(QLatin1String("/media/")), qPrintable(name));
    // Numbers that are no numbers, and sizes that are too much or too little.
    QCOMPARE(insertPicture(*doc, c, pngOf(8, 8)), PictureResult::Ok);
    QVERIFY(!setPictureSize(*doc, 4, std::nan(""), 10));
    QVERIFY(!setPicturePlace(*doc, 4, std::numeric_limits<double>::infinity(), 0));
    QVERIFY(!setPictureSize(*doc, 3, 10, 10));   // (no picture there)
    QVERIFY(!setPictureAltText(*doc, 99999, QStringLiteral("x")));
    QVERIFY(setPictureSize(*doc, 4, 1e12, 1e-9));
    PictureInfo info = pictureAt(*doc, doc->story(), 4);
    QCOMPARE(info.width, kMostPictureSize);
    QCOMPARE(info.height, kLeastPictureSize);
    QVERIFY(setPicturePlace(*doc, 4, -1e12, 1e12));
    info = pictureAt(*doc, doc->story(), 4);
    QCOMPARE(info.left, -kMostPictureOffset);
    QCOMPARE(info.top, kMostPictureOffset);
    QVERIFY(open(doc->toDocx()));
}


void WriteTests::aDocumentThatHoldsAsManyPicturesAsItCanShowRefusesMoreAndKeepsItsUndo()
{
    auto doc = textDoc({"x"});
    QTextCursor c(doc->story());
    int made = 0;
    PictureResult last = PictureResult::Ok;
    for (int i = 0; i < 2100 && last == PictureResult::Ok; ++i) {
        last = insertPicture(*doc, c, pngOf(1, 1, i + 1), QStringLiteral("p"));
        if (last == PictureResult::Ok) ++made;
    }
    QVERIFY(last == PictureResult::Refused);
    QVERIFY(made >= 1000 && made <= 2100);
    QVERIFY(!doc->pictureNotice.isEmpty());
    QCOMPARE(picturesIn(doc->story()), made);
    // The refused one left no part behind: as many parts as pictures.
    {
        office::Package pkg;
        QVERIFY(pkg.read(doc->toDocx()));
        int media = 0;
        for (const QString &name : pkg.partNames()) media += name.contains(QLatin1String("/media/"));
        QCOMPARE(media, made);
    }
    // Nothing of the history was let go: every picture can be taken back, and one brought again.
    int undone = 0;
    while (doc->canUndo() && undone < made + 5) undone += doc->undo() ? 1 : 0;
    QCOMPARE(undone, made);
    QCOMPARE(picturesIn(doc->story()), 0);
    QVERIFY(doc->redo());
    QCOMPARE(picturesIn(doc->story()), 1);
}


// ---- page setup

QByteArray WriteTests::sectionXml(int w, int h, int top, int right, int bottom, int left, const char *orient, const char *type)
{
    return QByteArray("<w:sectPr>") + (*type ? QByteArray("<w:type w:val=\"") + type + "\"/>" : QByteArray()) + "<w:pgSz w:w=\"" + QByteArray::number(w) + "\" w:h=\"" + QByteArray::number(h) +
           "\"" + (*orient ? QByteArray(" w:orient=\"") + orient + "\"" : QByteArray()) + "/><w:pgMar w:top=\"" + QByteArray::number(top) + "\" w:right=\"" + QByteArray::number(right) +
           "\" w:bottom=\"" + QByteArray::number(bottom) + "\" w:left=\"" + QByteArray::number(left) + "\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
}


// Three sections: Letter, then landscape Letter, then Letter again.
std::unique_ptr<WriteDoc> WriteTests::threeSections()
{
    QByteArray body;
    body += "<w:p><w:pPr>" + sectionXml(12240, 15840, 1440, 1440, 1440, 1440) + "</w:pPr><w:r><w:t>One</w:t></w:r></w:p>";
    body += "<w:p><w:pPr>" + sectionXml(15840, 12240, 1440, 1440, 1440, 1440, "landscape") + "</w:pPr><w:r><w:t>Two</w:t></w:r></w:p>";
    body += "<w:p><w:r><w:t>Three</w:t></w:r></w:p>";
    return open(makeDocx(body, kStyles, QByteArray(), sectionXml(12240, 15840, 1440, 1440, 1440, 1440)));
}


QVector<SectionSetup> WriteTests::setupsOf(const WriteDoc &doc)
{
    QVector<SectionSetup> out;
    for (int i = 0; i < doc.sectionCount(); ++i) out << pageSetupOf(doc.sectionProps(i), doc.differentOddEven());
    return out;
}


void WriteTests::formatPageNumbersSetsASectionsNumberingInOneStepAndSavesIt()
{
    auto doc = threeSections();
    QVERIFY(doc);
    QCOMPARE(pageNumberingOf(doc->sectionProps(1)), PageNumbering());
    PageNumbering n;
    n.format = QStringLiteral("lowerRoman");
    n.continueFromPrevious = false;
    n.startAt = 5;
    n.chapterStyle = 1;
    n.chapterSeparator = QStringLiteral("period");
    QCOMPARE(applyPageNumbering(*doc, 1, false, n), 1);
    QCOMPARE(pageNumberingOf(doc->sectionProps(1)), n);
    QCOMPARE(pageNumberingOf(doc->sectionProps(0)), PageNumbering());
    QCOMPARE(doc->flow().section(1).pageNumberFormat, QStringLiteral("roman"));
    QCOMPARE(doc->flow().section(1).firstPageNumber, std::optional<int>(5));
    QCOMPARE(doc->flow().section(1).chapterStyle, 1);
    QCOMPARE(doc->flow().section(1).chapterSeparator, QStringLiteral("."));
    QCOMPARE(applyPageNumbering(*doc, 1, false, n), 0);   // (nothing to change)
    // saved as the file format has it, and read back the same
    office::Package pack;
    QVERIFY(pack.read(doc->toDocx()));
    const QByteArray xml = pack.part(QStringLiteral("/word/document.xml"));
    QVERIFY2(xml.contains("<w:pgNumType w:fmt=\"lowerRoman\" w:start=\"5\" w:chapStyle=\"1\" w:chapSep=\"period\"/>"), xml.constData());
    auto back = open(doc->toDocx());
    QVERIFY(back);
    QCOMPARE(pageNumberingOf(back->sectionProps(1)), n);
    // Continue clears the start and None clears the chapter; one step undoes it
    n.continueFromPrevious = true;
    n.chapterStyle = 0;
    n.format = QStringLiteral("decimal");
    QCOMPARE(applyPageNumbering(*doc, 1, false, n), 1);
    QVERIFY(!doc->sectionProps(1).pageStart.has_value() && doc->sectionProps(1).pageFormat.isEmpty());
    QVERIFY(!QString::fromUtf8(doc->sectionProps(1).pgNumAttrs).contains(QLatin1String("chap")));
    QVERIFY(doc->undo());
    QCOMPARE(pageNumberingOf(doc->sectionProps(1)).format, QStringLiteral("lowerRoman"));
    // every section
    QCOMPARE(applyPageNumbering(*doc, 0, true, n), 1);   // (the other two already number as asked)
    QVERIFY(!pageNumberFormats().isEmpty());
}


void WriteTests::pageSetupChangesTheSectionOrTheWholeDocumentInOneStepAndSavesTheSame()
{
    auto doc = threeSections();
    QVERIFY(doc);
    QCOMPARE(doc->sectionCount(), 3);
    const QVector<SectionSetup> first = setupsOf(*doc);
    QVERIFY(first[1].landscape && !first[0].landscape);
    // This section's margins, and no other's.
    SectionSetup before = first[1], after = before;
    after.left = 36;
    after.top = 54;
    after.gutter = 18;
    QCOMPARE(applyPageSetup(*doc, 1, PageSetupScope::ThisSection, before, after), 1);
    QVector<SectionSetup> now = setupsOf(*doc);
    QCOMPARE(now[0], first[0]);
    QCOMPARE(now[2], first[2]);
    QCOMPARE(now[1].left, 36.0);
    QCOMPARE(now[1].top, 54.0);
    QCOMPARE(now[1].gutter, 18.0);
    QVERIFY(now[1].landscape);   // (the orientation was not asked to change)
    QCOMPARE(doc->flow().section(1).margins.left(), 54.0);   // (the gutter is added to the left margin)
    QVERIFY(doc->undo());
    QVERIFY(!doc->canUndo());   // (one step)
    QCOMPARE(setupsOf(*doc), first);
    QVERIFY(doc->redo());
    QCOMPARE(setupsOf(*doc)[1].left, 36.0);
    QVERIFY(doc->undo());
    // The whole document: the paper and the layout too, each section keeping what was not changed.
    before = first[0];
    after = before;
    after.paperWidth = 595.3;
    after.paperHeight = 841.9;
    after.right = 90;
    after.start = QStringLiteral("continuous");
    after.differentFirstPage = true;
    after.differentOddEven = true;
    after.header = 20;
    after.footer = 30;
    QVERIFY(applyPageSetup(*doc, 0, PageSetupScope::WholeDocument, before, after) >= 3);
    now = setupsOf(*doc);
    for (int i = 0; i < 3; ++i) {
        QVERIFY2(std::abs(now[i].paperWidth - 595.3) < 0.1 && std::abs(now[i].paperHeight - 841.9) < 0.1, qPrintable(QString::number(i)));
        QCOMPARE(now[i].right, 90.0);
        QVERIFY(now[i].start == QLatin1String("continuous"));
        QVERIFY(now[i].differentFirstPage && now[i].differentOddEven);
        QCOMPARE(now[i].header, 20.0);
        QCOMPARE(now[i].footer, 30.0);
    }
    QVERIFY(doc->differentOddEven());
    QCOMPARE(now[1].landscape, first[1].landscape);   // (and the section that was turned stays turned)
    QCOMPARE(now[0].left, first[0].left);
    const QVector<SectionSetup> changed = now;
    QVERIFY(doc->undo());
    QVERIFY(!doc->canUndo());
    QCOMPARE(setupsOf(*doc), first);
    QVERIFY(!doc->differentOddEven());
    QVERIFY(doc->redo());
    QCOMPARE(setupsOf(*doc), changed);
    // Saved, and opened again.
    const QByteArray saved = doc->toDocx();
    wml::XmlTree tree;
    QVERIFY(tree.parse(documentXml(saved)));
    auto back = open(saved);
    QVERIFY(back);
    QCOMPARE(back->sectionCount(), 3);
    QCOMPARE(setupsOf(*back), changed);
    // The pages follow: a page of the new paper.
    QVERIFY(std::abs(doc->flow().section(0).pageSize().width() - 595.3) < 0.1);
    QVERIFY(doc->pager().materialize(0));
}


void WriteTests::pageSetupTurnsAPageAndKeepsItsMarginsOnIt()
{
    auto doc = threeSections();
    const SectionSetup before = pageSetupOf(doc->sectionProps(0), false);
    SectionSetup after = before;
    after.landscape = true;
    after.left = before.top;   // (as the dialog turns the margins with the page)
    after.top = before.right;
    QVERIFY(applyPageSetup(*doc, 0, PageSetupScope::ThisSection, before, after) >= 1);
    const flow::Section s = doc->flow().section(0);
    QVERIFY(s.orientation == flow::Section::Orientation::Landscape);
    QCOMPARE(s.pageSize(), QSizeF(792, 612));
    QCOMPARE(s.paper, QSizeF(612, 792));
    auto back = open(doc->toDocx());
    QVERIFY(back);
    QVERIFY(back->flow().section(0).orientation == flow::Section::Orientation::Landscape);
    QVERIFY(documentXml(doc->toDocx()).contains("w:orient=\"landscape\""));
}


void WriteTests::pageSetupIsHeldToWhatAPageCanHold()
{
    SectionSetup bad;
    bad.paperWidth = 1e12;
    bad.paperHeight = std::nan("");
    bad.top = -50;
    bad.bottom = 1e12;
    bad.left = 1e12;
    bad.right = std::numeric_limits<double>::infinity();
    bad.gutter = 1e9;
    bad.header = 1e9;
    bad.footer = -1e9;
    bad.start = QStringLiteral("nonsense");
    const SectionSetup fixed = normalized(bad);
    QCOMPARE(fixed.paperWidth, kMostPaper);
    QCOMPARE(fixed.paperHeight, 792.0);
    QVERIFY(fixed.top >= 0 && fixed.bottom >= 0 && fixed.left >= 0 && fixed.right >= 0 && fixed.gutter >= 0);
    QVERIFY(fixed.left + fixed.right + fixed.gutter <= fixed.paperWidth - kLeastText + 0.01);
    QVERIFY(fixed.top + fixed.bottom <= fixed.paperHeight - kLeastText + 0.01);
    QVERIFY(fixed.header <= fixed.paperHeight / 2 && fixed.footer >= 0);
    QVERIFY(fixed.start == QLatin1String("nextPage"));
    // Applied to a document, the text keeps room and the pages are made.
    auto doc = threeSections();
    const SectionSetup before = pageSetupOf(doc->sectionProps(0), false);
    QVERIFY(applyPageSetup(*doc, 0, PageSetupScope::WholeDocument, before, bad) >= 1);
    for (int i = 0; i < doc->sectionCount(); ++i) {
        const flow::Section s = doc->flow().section(i);
        QVERIFY(s.columnWidth() >= 1);
        QVERIFY(s.pageSize().width() - s.margins.left() - s.margins.right() >= kLeastText - 0.5);
        QVERIFY(s.pageSize().height() - s.margins.top() - s.margins.bottom() >= kLeastText - 0.5);
    }
    QVERIFY(doc->pager().materialize(0));
    QVERIFY(open(doc->toDocx()));
}


void WriteTests::pageSetupOfTenThousandSectionsMakesEachOnceAndIsOneStep()
{
    QByteArray body;
    for (int i = 0; i < 10000; ++i) body += "<w:p><w:pPr>" + sectionXml(12240, 15840, 1440, 1440, 1440, 1440, "", "continuous") + "</w:pPr><w:r><w:t>s</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles, QByteArray(), sectionXml(12240, 15840, 1440, 1440, 1440, 1440)));
    QVERIFY(doc);
    QCOMPARE(doc->sectionCount(), 10001);
    const SectionSetup before = pageSetupOf(doc->sectionProps(0), false);
    SectionSetup after = before;
    after.left = 36;
    after.bottom = 54;
    const qint64 made = doc->sectionsMadeForTest();
    QVERIFY(applyPageSetup(*doc, 0, PageSetupScope::WholeDocument, before, after) >= 10001);
    // Each section was made again once (not once for each that changed, which is the square of their number).
    QVERIFY2(doc->sectionsMadeForTest() - made <= 10001 + 5, qPrintable(QString::number(doc->sectionsMadeForTest() - made)));
    QCOMPARE(pageSetupOf(doc->sectionProps(5000), false).left, 36.0);
    QCOMPARE(pageSetupOf(doc->sectionProps(10000), false).bottom, 54.0);
    QVERIFY(doc->undo());
    QVERIFY(!doc->canUndo());
    QCOMPARE(pageSetupOf(doc->sectionProps(5000), false).left, 72.0);
    QCOMPARE(pageSetupOf(doc->sectionProps(10000), false).bottom, 72.0);
}


// ---- the ruler's tab stops

void WriteTests::tabStopsAreAddedMovedAndRemovedInOneStepAndSaveTheSame()
{
    auto doc = textDoc({"One two", "Three four"});
    QTextCursor c(doc->story());
    c.setPosition(3);
    TabStopEntry left;
    left.position = 72;
    QVERIFY(addTabStop(*doc, c, left));
    QVERIFY(!addTabStop(*doc, c, left));   // (the same stop again is no step)
    QCOMPARE(tabStopsOf(doc->story()->begin().blockFormat()).size(), 1);
    QVERIFY(doc->undo());
    QVERIFY(tabStopsOf(doc->story()->begin().blockFormat()).isEmpty());
    QVERIFY(!doc->canUndo());
    QVERIFY(doc->redo());
    TabStopEntry right;
    right.position = 200;
    right.type = QTextOption::RightTab;
    right.leader = QLatin1Char('.');
    QVERIFY(addTabStop(*doc, c, right));
    QVERIFY(moveTabStop(*doc, c, 72, 100));
    QVector<TabStopEntry> stops = tabStopsOf(doc->story()->begin().blockFormat());
    QCOMPARE(stops.size(), 2);
    QCOMPARE(stops[0].position, 100.0);
    QVERIFY(stops[0].type == QTextOption::LeftTab);
    QCOMPARE(stops[1].position, 200.0);
    QVERIFY(stops[1].type == QTextOption::RightTab && stops[1].leader == QLatin1Char('.'));   // (the leader stays with its stop)
    QVERIFY(doc->undo());
    QCOMPARE(tabStopsOf(doc->story()->begin().blockFormat())[0].position, 72.0);
    QVERIFY(doc->redo());
    // The other paragraph has none; a selection over both gives both a stop.
    QVERIFY(tabStopsOf(doc->story()->begin().next().blockFormat()).isEmpty());
    c.setPosition(0);
    c.setPosition(doc->story()->characterCount() - 1, QTextCursor::KeepAnchor);
    TabStopEntry middle;
    middle.position = 150;
    middle.type = QTextOption::CenterTab;
    QVERIFY(addTabStop(*doc, c, middle));
    QCOMPARE(tabStopsOf(doc->story()->begin().next().blockFormat()).size(), 1);
    QCOMPARE(tabStopsOf(doc->story()->begin().blockFormat()).size(), 3);
    QVERIFY(doc->undo());
    QVERIFY(tabStopsOf(doc->story()->begin().next().blockFormat()).isEmpty());
    QVERIFY(doc->redo());
    QVERIFY(removeTabStop(*doc, c, 200));
    QVERIFY(!removeTabStop(*doc, c, 200));
    QCOMPARE(tabStopsOf(doc->story()->begin().blockFormat()).size(), 2);
    // Saved, and opened again.
    const QByteArray saved = doc->toDocx();
    const QByteArray docXml = documentXml(saved);
    QVERIFY2(docXml.contains("w:pos=\"2000\""), docXml.constData());
    QVERIFY(docXml.contains("w:val=\"center\""));
    auto back = open(saved);
    QVERIFY(back);
    QCOMPARE(tabStopsOf(back->story()->begin().blockFormat()), tabStopsOf(doc->story()->begin().blockFormat()));
    QCOMPARE(tabStopsOf(back->story()->begin().next().blockFormat()), tabStopsOf(doc->story()->begin().next().blockFormat()));
}


void WriteTests::tabStopsAreHeldToWhatAParagraphKeeps()
{
    auto doc = textDoc({"One"});
    QTextCursor c(doc->story());
    for (int i = 0; i < 100; ++i) {
        TabStopEntry t;
        t.position = 10 + i * 7;
        addTabStop(*doc, c, t);
    }
    QCOMPARE(tabStopsOf(doc->story()->begin().blockFormat()).size(), kMostTabStops);
    auto other = textDoc({"One"});
    QTextCursor d(other->story());
    TabStopEntry far;
    far.position = 1e12;
    QVERIFY(addTabStop(*other, d, far));
    TabStopEntry weird;
    weird.position = std::nan("");
    QVERIFY(addTabStop(*other, d, weird));
    TabStopEntry negative;
    negative.position = -40;
    negative.type = QTextOption::CenterTab;
    QVERIFY(addTabStop(*other, d, negative));
    const QVector<TabStopEntry> stops = tabStopsOf(other->story()->begin().blockFormat());
    QVERIFY(!stops.isEmpty());
    for (const TabStopEntry &s : stops) QVERIFY(s.position >= 0 && s.position <= kMostTabPosition);
    QVERIFY(open(other->toDocx()));
    QVERIFY(open(doc->toDocx()));
}


// ---- wave C review

// A picture dressed up as a hostile one: a drawing with a hyperlink on it, hidden, and children and attributes of the newer drawing format that the program does not know.
QByteArray WriteTests::decoratedPictureBody(bool anchored, bool withLink)
{
    const QByteArray ns = "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                          "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" xmlns:wp14=\"http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing\"";
    const QByteArray graphic = "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:pic><pic:nvPicPr><pic:cNvPr id=\"0\" name=\"x\"/><pic:cNvPicPr/></pic:nvPicPr>"
                               "<pic:blipFill><a:blip r:embed=\"rIdPic\"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill><pic:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1270000\" cy=\"635000\"/></a:xfrm>"
                               "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></pic:spPr></pic:pic></a:graphicData></a:graphic>";
    const QByteArray docPr = "<wp:docPr id=\"7\" name=\"Picture 7\" descr=\"old\" hidden=\"1\" title=\"T\">" + QByteArray(withLink ? "<a:hlinkClick r:id=\"rIdLnk\"/>" : "") + "</wp:docPr>";
    QByteArray d = "<w:r><w:drawing " + ns + ">";
    if (anchored)
        d += "<wp:anchor distT=\"0\" distB=\"0\" distL=\"114300\" distR=\"114300\" simplePos=\"0\" relativeHeight=\"5\" behindDoc=\"0\" locked=\"0\" layoutInCell=\"1\" allowOverlap=\"1\" wp14:anchorId=\"1234ABCD\">"
             "<wp:simplePos x=\"0\" y=\"0\"/><wp:positionH relativeFrom=\"column\"><wp:posOffset>0</wp:posOffset></wp:positionH><wp:positionV relativeFrom=\"paragraph\"><wp:posOffset>0</wp:posOffset></wp:positionV>"
             "<wp:extent cx=\"1270000\" cy=\"635000\"/><wp:effectExtent l=\"0\" t=\"0\" r=\"0\" b=\"0\"/><wp:wrapSquare wrapText=\"bothSides\"/>" + docPr +
             "<wp:cNvGraphicFramePr/>" + graphic + "<wp14:sizeRelH relativeFrom=\"margin\"><wp14:pctWidth>0</wp14:pctWidth></wp14:sizeRelH><wp14:sizeRelV relativeFrom=\"margin\"><wp14:pctHeight>0</wp14:pctHeight></wp14:sizeRelV></wp:anchor>";
    else
        d += "<wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"><wp:extent cx=\"1270000\" cy=\"635000\"/>" + docPr + "<wp:cNvGraphicFramePr/>" + graphic + "</wp:inline>";
    d += "</w:drawing></w:r>";
    return "<w:p><w:r><w:t xml:space=\"preserve\">Before </w:t></w:r>" + d + "<w:r><w:t xml:space=\"preserve\"> after</w:t></w:r></w:p>";
}


void WriteTests::editingAPictureKeepsWhatTheEditDoesNotChange()
{
    for (bool anchored : {true, false}) {
        auto doc = open(makeDocxWithPicture(decoratedPictureBody(anchored)));
        QVERIFY(doc);
        const int at = 7;
        QVERIFY(pictureAt(*doc, doc->story(), at).valid);
        QVERIFY(setPictureAltText(*doc, at, QStringLiteral("new words")));
        QVERIFY(setPictureSize(*doc, at, 50, 40));
        QVERIFY(setPictureWrap(*doc, at, PictureWrap::Tight));
        QVERIFY(setPictureWrap(*doc, at, PictureWrap::TopAndBottom));
        const QByteArray floating = documentXml(doc->toDocx());
        wml::XmlTree tree;
        QVERIFY(tree.parse(floating));
        for (const char *kept : {"<a:hlinkClick", "r:id=\"rIdLnk\"", "hidden=\"1\"", "title=\"T\"", "descr=\"new words\"", "wp14:sizeRelH", "wp14:sizeRelV", "wp14:anchorId=\"1234ABCD\"", "<wp:effectExtent"})
            if (anchored || !QByteArray(kept).contains("wp14") ) {
                if (!anchored && QByteArray(kept) == "<wp:effectExtent") continue;
                QVERIFY2(floating.contains(kept), kept);
            }
        QVERIFY(!floating.contains("descr=\"old\""));
        QVERIFY(floating.contains("wrapTopAndBottom"));
        QVERIFY(floating.contains("cx=\"635000\"") && floating.contains("cy=\"508000\""));   // 50 by 40 points
        QCOMPARE(pictureAt(*doc, doc->story(), at).width, 50.0);
        // In line again, the children only a floating drawing may have are left out (the others stay).
        QVERIFY(setPictureWrap(*doc, at, PictureWrap::Inline));
        const QByteArray inLine = documentXml(doc->toDocx());
        QVERIFY(tree.parse(inLine));
        QVERIFY(inLine.contains("<wp:inline"));
        QVERIFY(inLine.contains("<a:hlinkClick") && inLine.contains("hidden=\"1\"") && inLine.contains("descr=\"new words\""));
        QVERIFY(!inLine.contains("sizeRelH") && !inLine.contains("<wp:anchor"));
        auto back = open(doc->toDocx());
        QVERIFY(back);
        QCOMPARE(pictureAt(*back, back->story(), at).alt, QStringLiteral("new words"));
    }
}


// The ids of drawings: above the ids of every drawing the file has, in the headers and other parts too, and above those pasted.
void WriteTests::drawingIdsDoNotCollideWithThoseOfHeadersOrPastes()
{
    const QByteArray header = "<w:hdr " + QByteArray(kNs) + " xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\"><w:p><w:r><w:drawing><wp:inline><wp:extent cx=\"100\" cy=\"100\"/>"
                              "<wp:docPr id=\"1\" name=\"Header picture\"/></wp:inline></w:drawing></w:r></w:p></w:hdr>";
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
    auto doc = open(makeDocxWithParts(paragraphs(1), sect, {{QStringLiteral("/word/header1.xml"), header}}));
    QVERIFY(doc);
    QTextCursor c(doc->story());
    QCOMPARE(insertPicture(*doc, c, pngOf(8, 8), QStringLiteral("a")), PictureResult::Ok);
    auto idsOf = [](const QByteArray &xml) {
        QList<int> out;
        static const QRegularExpression re(QStringLiteral("<wp:docPr[^>]*? id=\"(\\d+)\""));
        auto it = re.globalMatch(QString::fromUtf8(xml));
        while (it.hasNext()) out << it.next().captured(1).toInt();
        return out;
    };
    const int first = idsOf(documentXml(doc->toDocx())).value(0);
    QVERIFY2(first > 1, qPrintable(QString::number(first)));
    // A picture pasted from another document, with an id of its own.
    auto other = open(makeDocxWithPicture(decoratedPictureBody(false, false)));   // (its drawing has the id 7)
    QTextCursor oc(other->story());
    oc.setPosition(7);
    oc.setPosition(8, QTextCursor::KeepAnchor);
    const Clip clip = copySelection(*other, oc);
    QTextCursor at(doc->story());
    at.setPosition(0);
    pasteClip(*doc, at, clip);
    QCOMPARE(picturesIn(doc->story()), 2);
    QTextCursor end(doc->story());
    end.movePosition(QTextCursor::End);
    QCOMPARE(insertPicture(*doc, end, pngOf(9, 9), QStringLiteral("b")), PictureResult::Ok);
    QList<int> all = idsOf(documentXml(doc->toDocx()));
    QCOMPARE(all.size(), 3);
    QSet<int> unique(all.cbegin(), all.cend());
    QCOMPARE(unique.size(), 3);
    QVERIFY(!unique.contains(1));
}


void WriteTests::altTextAndNamesKeepTheirSurrogatePairsAndDropLoneOnes()
{
    auto doc = textDoc({"x"});
    QTextCursor c(doc->story());
    const QString emoji = QString::fromUtf8("\xF0\x9F\x98\x80");   // U+1F600
    QCOMPARE(insertPicture(*doc, c, pngOf(8, 8), QStringLiteral("name ") + emoji + QStringLiteral(".png")), PictureResult::Ok);
    QString alt = QStringLiteral("a") + emoji + QStringLiteral("b") + QString(QChar(0xD800)) + QStringLiteral("c") + QString(QChar(0xDC00)) + QStringLiteral("d");
    QVERIFY(setPictureAltText(*doc, 0, alt));
    QCOMPARE(pictureAt(*doc, doc->story(), 0).alt, QStringLiteral("a") + emoji + QStringLiteral("bcd"));
    // A text that ends in the middle of a pair at the limit is cut before the pair.
    QVERIFY(setPictureAltText(*doc, 0, QString(kMostAltTextLength - 1, QLatin1Char('x')) + emoji + emoji));
    const QString cut = pictureAt(*doc, doc->story(), 0).alt;
    QCOMPARE(cut.size(), kMostAltTextLength - 1);
    QCOMPARE(cleanAltText(QString(kMostAltTextLength - 1, QLatin1Char('x')) + emoji).size(), kMostAltTextLength - 1);
    QCOMPARE(cleanAltText(QString(kMostAltTextLength - 2, QLatin1Char('x')) + emoji), QString(kMostAltTextLength - 2, QLatin1Char('x')) + emoji);
    const QByteArray saved = doc->toDocx();
    wml::XmlTree tree;
    QVERIFY(tree.parse(documentXml(saved)));
    QVERIFY(documentXml(saved).contains(("name " + emoji + ".png").toUtf8()));
    auto back = open(saved);
    QCOMPARE(pictureAt(*back, back->story(), 0).alt, cut);
}


// An undone or deleted insert leaves nothing of the picture in the file saved, and brings it back with the Redo or Undo that needs it.
void WriteTests::anUndoneOrDeletedPictureIsNotInTheSavedFile()
{
    auto mediaIn = [](const QByteArray &saved) {
        office::Package pkg;
        pkg.read(saved);
        int n = 0;
        for (const QString &name : pkg.partNames()) n += name.contains(QLatin1String("/media/"));
        int rels = 0;
        for (const office::Relationship &r : pkg.relationships(QStringLiteral("/word/document.xml"))) rels += r.type.endsWith(QLatin1String("/image"));
        return QPair<int, int>(n, rels);
    };
    auto doc = textDoc({"Some text"});
    QTextCursor c(doc->story());
    c.setPosition(4);
    QCOMPARE(insertPicture(*doc, c, pngOf(30, 30, 5), QStringLiteral("secret")), PictureResult::Ok);
    QCOMPARE(mediaIn(doc->toDocx()), (QPair<int, int>(1, 1)));
    QVERIFY(doc->undo());
    QCOMPARE(mediaIn(doc->toDocx()), (QPair<int, int>(0, 0)));
    QVERIFY(doc->redo());
    QCOMPARE(mediaIn(doc->toDocx()), (QPair<int, int>(1, 1)));
    // Deleted: the same.
    QTextCursor d(doc->story());
    d.setPosition(4);
    d.setPosition(5, QTextCursor::KeepAnchor);
    removeSelection(*doc, d);
    QVERIFY(!isPictureAt(doc->story(), 4));
    QCOMPARE(mediaIn(doc->toDocx()), (QPair<int, int>(0, 0)));
    QVERIFY(doc->undo());
    QVERIFY(isPictureAt(doc->story(), 4));
    QCOMPARE(mediaIn(doc->toDocx()), (QPair<int, int>(1, 1)));
    // A picture a file had stays whether or not anything shows it.
    auto file = open(makeDocxWithPicture("<w:p><w:r><w:t>no picture shown</w:t></w:r></w:p>"));
    QCOMPARE(mediaIn(file->toDocx()).first, 1);
}


// The bytes of a picture are held once, and a document holds no more than its total.
void WriteTests::aPictureIsHeldOnceAndTheDocumentsPicturesHaveATotal()
{
    auto doc = textDoc({"x"});
    QTextCursor c(doc->story());
    const QByteArray png = pngOf(64, 64, 9);
    QCOMPARE(insertPicture(*doc, c, png, QStringLiteral("p")), PictureResult::Ok);
    QVERIFY(!doc->flow().images.isEmpty());
    QString name;
    office::Package pkg;
    QVERIFY(pkg.read(doc->toDocx()));
    for (const QString &n : pkg.partNames())
        if (n.contains(QLatin1String("/media/"))) name = n;
    QVERIFY(!name.isEmpty());
    QVERIFY2(doc->partData(name).constData() == doc->flow().images.last().bytes.constData(), "the part and the drawn picture are two copies");
    QVERIFY(doc->mediaBytes() >= png.size());
    // The total.
    doc->mediaLimit = doc->mediaBytes() + png.size() * 2 + 10;
    int made = 0;
    PictureResult last = PictureResult::Ok;
    for (int i = 0; i < 10 && last == PictureResult::Ok; ++i) {
        last = insertPicture(*doc, c, pngOf(64, 64, 100 + i), QStringLiteral("q"));
        made += last == PictureResult::Ok;
    }
    QVERIFY(last == PictureResult::TooLarge);
    QVERIFY(made >= 1 && made <= 3);
    QVERIFY(!doc->pictureNotice.isEmpty());
    QVERIFY(doc->mediaBytes() <= doc->mediaLimit);
}


QByteArray WriteTests::svgWith(const QByteArray &inner) { return "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"40\" height=\"30\">" + inner + "</svg>"; }


void WriteTests::insertFormat(const char *format)
{
    if (!QImageReader::supportedImageFormats().contains(format)) QSKIP(qPrintable(QStringLiteral("no %1 codec on this machine").arg(QLatin1String(format))));
    auto doc = textDoc({"x"});
    QTextCursor c(doc->story());
    QImage image(20, 10, QImage::Format_RGB32);
    image.fill(QColor(10, 200, 30));
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    QVERIFY2(image.save(&buffer, format), format);
    QVERIFY2(insertPicture(*doc, c, bytes, QStringLiteral("f")) == PictureResult::Ok, format);
    QCOMPARE(picturesIn(doc->story()), 1);
    QVERIFY(open(doc->toDocx()));
}


void WriteTests::aTiffPictureIsInserted() { insertFormat("tiff"); }

void WriteTests::aWebpPictureIsInserted() { insertFormat("webp"); }

void WriteTests::aBmpPictureIsInserted() { insertFormat("bmp"); }


void WriteTests::anSvgPictureIsInsertedAsThePictureItDraws()
{
    if (!QImageReader::supportedImageFormats().contains("svg")) QSKIP("no SVG codec on this machine");
    auto doc = textDoc({"x"});
    QTextCursor c(doc->story());
    QVERIFY(insertPicture(*doc, c, svgWith("<rect width=\"40\" height=\"30\" fill=\"red\"/>"), QStringLiteral("s")) == PictureResult::Ok);
    office::Package pkg;
    QVERIFY(pkg.read(doc->toDocx()));
    bool png = false;
    for (const QString &n : pkg.partNames()) png = png || n.endsWith(QLatin1String(".png"));
    QVERIFY(png);   // (an SVG is kept as the picture it draws)
}


void WriteTests::anSvgThatReachesOutsideItselfOrIsHugeIsRefused()
{
    auto doc = textDoc({"x"});
    QTextCursor c(doc->story());
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QFile secret(dir.filePath(QStringLiteral("secret.png")));
    QVERIFY(secret.open(QIODevice::WriteOnly));
    secret.write(pngOf(10, 10, 77));
    secret.close();
    const QByteArray url = QUrl::fromLocalFile(secret.fileName()).toString().toUtf8();
    const QByteArray padding = "<!--" + QByteArray(6000, ' ') + "-->";   // (what comes first is longer than any look at the start of the file)
    for (const QByteArray &inner : {"<image xlink:href=\"" + url + "\" width=\"40\" height=\"30\"/>", "<image href='" + url + "' width=\"40\" height=\"30\"/>",
                                    QByteArray("<image xlink:href=\"secret.png\" width=\"40\" height=\"30\"/>"), "<image src=\"" + url + "\" width=\"40\" height=\"30\"/>",
                                    "<rect width=\"4\" height=\"4\" style=\"fill:url(" + url + ")\"/>", QByteArray("<style>@import url(secret.css);</style>"),
                                    padding + "<image xlink:href=\"" + url + "\" width=\"40\" height=\"30\"/>"}) {
        for (bool late : {false, true}) {
            const QByteArray svg = late ? "<?xml version=\"1.0\"?>" + padding + svgWith(inner) : svgWith(inner);
            QVERIFY2(insertPicture(*doc, c, svg, QStringLiteral("s")) == PictureResult::External, qPrintable(QString::fromLatin1(svg.left(200))));
        }
    }
    // An SVG whose first mention of itself is past the first bytes of the file is still looked through.
    QCOMPARE(picturesIn(doc->story()), 0);
    QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("none"))));
    // What it holds of itself is allowed: a reference to its own parts, and an image in the data it carries.
    const QByteArray own = svgWith("<defs><linearGradient id=\"g\"><stop offset=\"0\" stop-color=\"red\"/></linearGradient></defs><rect width=\"4\" height=\"4\" fill=\"url(#g)\"/>");
    if (QImageReader::supportedImageFormats().contains("svg")) QVERIFY(insertPicture(*doc, c, own, QStringLiteral("own")) == PictureResult::Ok);
    // Too large an SVG file.
    QVERIFY(insertPicture(*doc, c, svgWith(QByteArray(6 << 20, ' ')), QStringLiteral("big")) == PictureResult::TooLarge);
}


void WriteTests::aPictureFileThatIsNotAFileOrTooLongIsNotReadToTheEnd()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QByteArray bytes;
    const QString good = dir.filePath(QStringLiteral("good.png"));
    QFile f(good);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(pngOf(8, 8));
    f.close();
    QVERIFY(readPictureFile(good, &bytes) == PictureResult::Ok);
    QVERIFY(!bytes.isEmpty());
    bytes.clear();
    QVERIFY(readPictureFile(dir.filePath(QStringLiteral("missing.png")), &bytes) != PictureResult::Ok);
    QVERIFY(readPictureFile(dir.path(), &bytes) != PictureResult::Ok);
    QVERIFY(bytes.isEmpty());
    // A file longer than a picture may be is refused after reading no more than the limit and a byte.
    const QString big = dir.filePath(QStringLiteral("big.png"));
    QFile b(big);
    QVERIFY(b.open(QIODevice::WriteOnly));
    QVERIFY(b.resize(kMostPictureBytes + 10));
    b.close();
    QVERIFY(readPictureFile(big, &bytes) == PictureResult::TooLarge);
    QVERIFY(bytes.isEmpty());
#ifdef Q_OS_UNIX
    const QString fifo = dir.filePath(QStringLiteral("pipe.png"));
    QVERIFY(::mkfifo(QFile::encodeName(fifo).constData(), 0600) == 0);
    QVERIFY(readPictureFile(fifo, &bytes) != PictureResult::Ok);   // (not opened: it would wait for a writer)
    const QString zero = dir.filePath(QStringLiteral("zero.png"));
    if (QFileInfo::exists(QStringLiteral("/dev/zero")) && QFile::link(QStringLiteral("/dev/zero"), zero)) {
        QVERIFY(readPictureFile(zero, &bytes) != PictureResult::Ok);
        QVERIFY(bytes.isEmpty());
    }
#else
    QSKIP("no named pipes on this system");
#endif
}


void WriteTests::thePaperIsNoLargerThanTwentyTwoInches()
{
    QCOMPARE(kMostPaper, 1584.0);
    SectionSetup s;
    s.paperWidth = 1e9;
    s.paperHeight = 5000;
    const SectionSetup fixed = normalized(s);
    QCOMPARE(fixed.paperWidth, 1584.0);
    QCOMPARE(fixed.paperHeight, 1584.0);
}


void WriteTests::aSizeThatKeepsItsRatioIsHeldToTheLimitsAsAWhole()
{
    double w = 1e9, h = 5e8;
    clampKeepingRatio(&w, &h);
    QCOMPARE(w, kMostPictureSize);
    QCOMPARE(h, kMostPictureSize / 2);
    w = 3000;
    h = 1500;
    clampKeepingRatio(&w, &h);
    QCOMPARE(w, kMostPictureSize);
    QCOMPARE(h, kMostPictureSize / 2);
    w = 0.01;
    h = 0.02;
    clampKeepingRatio(&w, &h);
    QVERIFY(w >= kLeastPictureSize - 1e-9 && h >= kLeastPictureSize - 1e-9);
    QVERIFY(std::abs(h / w - 2.0) < 1e-6);
    w = 100;
    h = 50;
    clampKeepingRatio(&w, &h);
    QCOMPARE(w, 100.0);
    QCOMPARE(h, 50.0);
}


void WriteTests::tabStopsAFileHasPastTheLimitAreKept()
{
    QByteArray tabs = "<w:tabs>";
    for (int i = 0; i < 100; ++i) tabs += "<w:tab w:val=\"left\" w:pos=\"" + QByteArray::number(100 + i * 100) + "\"/>";
    tabs += "</w:tabs>";
    auto doc = open(makeDocx("<w:p><w:pPr>" + tabs + "</w:pPr><w:r><w:t>text</w:t></w:r></w:p>", kStyles));
    QVERIFY(doc);
    QCOMPARE(tabStopsOf(doc->story()->begin().blockFormat()).size(), 100);
    QTextCursor c(doc->story());
    QVERIFY(moveTabStop(*doc, c, 5, 5.5));
    QCOMPARE(tabStopsOf(doc->story()->begin().blockFormat()).size(), 100);
    TabStopEntry more;
    more.position = 700.5;
    QVERIFY(!addTabStop(*doc, c, more));   // (no more are added past the limit)
    QCOMPARE(tabStopsOf(doc->story()->begin().blockFormat()).size(), 100);
    QVERIFY(removeTabStop(*doc, c, 105));
    QCOMPARE(tabStopsOf(doc->story()->begin().blockFormat()).size(), 99);
    QCOMPARE(documentXml(doc->toDocx()).count("<w:tab "), 99);
}


// The children of a section are written in the order the schema gives, whatever the dialog changed.
void WriteTests::aSectionChangedByPageSetupIsWrittenInSchemaOrder()
{
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" "
                            "w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/><w:pgBorders w:offsetFrom=\"page\"><w:top w:val=\"single\" w:sz=\"4\" w:space=\"24\" w:color=\"auto\"/></w:pgBorders>"
                            "<w:lnNumType w:countBy=\"1\"/><w:cols w:space=\"720\"/><w:vAlign w:val=\"center\"/><w:docGrid w:linePitch=\"360\"/></w:sectPr>";
    const QByteArray header = headerPart("hdr", {"<w:r><w:t>h</w:t></w:r>"});
    auto doc = open(makeDocxWithParts(paragraphs(2), sect, {{QStringLiteral("/word/header1.xml"), header}}));
    QVERIFY(doc);
    const SectionSetup before = pageSetupOf(doc->sectionProps(0), false);
    SectionSetup after = before;
    after.landscape = true;
    after.left = 30;
    after.gutter = 12;
    after.header = 20;
    after.start = QStringLiteral("continuous");
    after.differentFirstPage = true;
    QVERIFY(applyPageSetup(*doc, 0, PageSetupScope::ThisSection, before, after) >= 1);
    const QByteArray xml = documentXml(doc->toDocx());
    wml::XmlTree tree;
    QVERIFY(tree.parse(xml));
    const int at = xml.lastIndexOf("<w:sectPr");
    QVERIFY(at >= 0);
    const QByteArray body = xml.mid(at, xml.indexOf("</w:sectPr>", at) - at);
    static const QStringList order = {"headerReference", "footerReference", "footnotePr", "endnotePr", "type", "pgSz", "pgMar", "paperSrc", "pgBorders", "lnNumType", "pgNumType", "cols", "formProt", "vAlign",
                                      "noEndnote", "titlePg", "textDirection", "bidi", "rtlGutter", "docGrid", "printerSettings"};
    int last = -1, seen = 0;
    static const QRegularExpression child(QStringLiteral("<w:(\\w+)"));
    auto it = child.globalMatch(QString::fromUtf8(body));
    int depth = 0;
    QStringList names;
    while (it.hasNext()) names << it.next().captured(1);
    for (const QString &name : names) {
        const int rank = int(order.indexOf(name));
        if (rank < 0) continue;   // (the children of a child: a border's side)
        ++seen;
        QVERIFY2(rank >= last, qPrintable(QStringLiteral("%1 after %2: %3").arg(name, order.value(last), names.join(QLatin1Char(' ')))));
        last = rank;
    }
    Q_UNUSED(depth);
    QVERIFY(seen >= 8);
    QVERIFY(names.contains(QStringLiteral("type")) && names.contains(QStringLiteral("titlePg")) && names.contains(QStringLiteral("pgBorders")) && names.contains(QStringLiteral("vAlign")));
}


// Thirty thousand moves and wraps of one picture: none is refused, the records of the picture's drawn form are what the text holds and the history can bring back, and Undo
// goes all the way back to the file as it was.
void WriteTests::thirtyThousandEditsOfOnePictureAreNeverRefusedAndItsRecordsAreBounded()
{
    auto doc = textDoc({longText()});
    const QByteArray original = documentXml(doc->toDocx());
    QTextCursor c(doc->story());
    QCOMPARE(insertPicture(*doc, c, pngOf(40, 40)), PictureResult::Ok);
    QVERIFY(setPictureWrap(*doc, 0, PictureWrap::Square));
    const PictureWrap wraps[] = {PictureWrap::Tight, PictureWrap::TopAndBottom, PictureWrap::BehindText, PictureWrap::InFrontOfText, PictureWrap::Square};
    int steps = 2, wrap = 0, refused = 0;
    for (int i = 0; i < 30000; ++i) {
        const bool ok = i % 5 == 4 ? setPictureWrap(*doc, 0, wraps[wrap++ % 5]) : setPicturePlace(*doc, 0, 10 + i * 0.1, 20 + (i % 100) * 0.5);
        if (ok) ++steps;
        else ++refused;
    }
    QCOMPARE(refused, 0);
    QVERIFY2(doc->pictureNotice.isEmpty(), qPrintable(doc->pictureNotice));
    // What the document holds of the drawn forms: no more than the steps (the text holds one).
    QVERIFY2(doc->objectSlotsForTest() <= steps + 50, qPrintable(QString::number(doc->objectSlotsForTest())));
    int undone = 0;
    while (doc->canUndo()) undone += doc->undo() ? 1 : 0;
    QCOMPARE(undone, steps);
    QCOMPARE(documentXml(doc->toDocx()), original);
    // A new edit puts the steps that were undone out of reach, and the forms they held are let go.
    QTextCursor start(doc->story());
    QCOMPARE(insertPicture(*doc, start, pngOf(40, 40, 3)), PictureResult::Ok);
    QVERIFY(setPictureWrap(*doc, 0, PictureWrap::Square));
    doc->collectObjectRecordsForTest();
    QVERIFY2(doc->liveObjectRecordsForTest() <= 20, qPrintable(QString::number(doc->liveObjectRecordsForTest())));
    // And the freed places are used again: five thousand more edits make no new places (the places are what the first thirty thousand needed, not more).
    const int places = doc->objectSlotsForTest();
    for (int i = 0; i < 5000; ++i) QVERIFY(setPicturePlace(*doc, 0, 400 + i * 0.1, 50));
    QVERIFY2(doc->objectSlotsForTest() <= places + 5, qPrintable(QStringLiteral("%1 places, were %2").arg(doc->objectSlotsForTest()).arg(places)));
    // The same drawings again are the forms made for them: wrapping back and forth between two looks makes no more than the two.
    const int before = doc->liveObjectRecordsForTest();
    for (int i = 0; i < 1000; ++i) QVERIFY(setPictureWrap(*doc, 0, i % 2 ? PictureWrap::Square : PictureWrap::Tight));
    QVERIFY2(doc->liveObjectRecordsForTest() - before <= 4, qPrintable(QString::number(doc->liveObjectRecordsForTest() - before)));
    QVERIFY(setPicturePlace(*doc, 0, 100, 100));
    QVERIFY(doc->undo());
    QCOMPARE(pictureItemsOfPage(*doc).size(), 1);
}


// A copy of a picture does not lean on the drawn forms the document may let go: they are made again for what is pasted.
void WriteTests::aCopiedFloatingPictureIsPastedIntactAfterTheFormsItHadAreFreed()
{
    auto doc = textDoc({longText(), QStringLiteral("second paragraph")});
    QTextCursor c(doc->story());
    QCOMPARE(insertPicture(*doc, c, pngOf(40, 40)), PictureResult::Ok);
    QVERIFY(setPicturePlace(*doc, 0, 100, 100));
    QTextCursor sel(doc->story());
    sel.setPosition(0);
    sel.setPosition(1, QTextCursor::KeepAnchor);
    const Clip clip = copySelection(*doc, sel);
    // The step that made the picture's form is taken back and put out of reach by a new edit; the places it held are used by others.
    QVERIFY(doc->undo());
    QVERIFY(setPictureWrap(*doc, 0, PictureWrap::Square));
    for (int i = 0; i < 20; ++i) QVERIFY(setPicturePlace(*doc, 0, 200 + i, 300));
    doc->collectObjectRecordsForTest();
    QTextCursor at(doc->story());
    at.setPosition(doc->story()->findBlockByNumber(1).position());
    pasteClip(*doc, at, clip);
    QCOMPARE(picturesIn(doc->story()), 2);
    const int pasted = doc->story()->findBlockByNumber(1).position();
    QVERIFY(isPictureAt(doc->story(), pasted));
    const PictureInfo info = pictureAt(*doc, doc->story(), pasted);
    QVERIFY(info.valid && info.floating);
    QCOMPARE(info.left, 100.0);
    QCOMPARE(info.top, 100.0);
    QCOMPARE(pictureItemsOfPage(*doc).size(), 2);
    auto back = open(doc->toDocx());
    QVERIFY(back);
    QCOMPARE(picturesIn(back->story()), 2);
    QCOMPARE(pictureItemsOfPage(*back).size(), 2);
}


// ---- wave C, the second review

QByteArray WriteTests::pictureBodyWithId(int id)
{
    return "<w:p><w:r><w:t xml:space=\"preserve\">Pic </w:t></w:r><w:r><w:drawing xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" "
           "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><wp:inline><wp:extent cx=\"635000\" cy=\"635000\"/>"
           "<wp:docPr id=\"" + QByteArray::number(id) + "\" name=\"P\"/><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:pic><pic:nvPicPr>"
           "<pic:cNvPr id=\"0\" name=\"x\"/><pic:cNvPicPr/></pic:nvPicPr><pic:blipFill><a:blip r:embed=\"rIdPic\"/></pic:blipFill><pic:spPr/></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing></w:r></w:p>";
}


QList<int> WriteTests::drawingIdsOf(const QByteArray &xml)
{
    QList<int> out;
    static const QRegularExpression re(QStringLiteral("<wp:docPr[^>]*? id=\"(\\d+)\""));
    auto it = re.globalMatch(QString::fromUtf8(xml));
    while (it.hasNext()) out << it.next().captured(1).toInt();
    return out;
}


void WriteTests::everyPastedPictureHasADrawingIdOfItsOwn()
{
    auto doc = open(makeDocxWithPicture(pictureBodyWithId(1)));
    auto other = open(makeDocxWithPicture(pictureBodyWithId(1)));
    QVERIFY(doc && other);
    QTextCursor sel(doc->story());
    sel.setPosition(4);
    sel.setPosition(5, QTextCursor::KeepAnchor);
    const Clip same = copySelection(*doc, sel);
    QTextCursor sel2(other->story());
    sel2.setPosition(4);
    sel2.setPosition(5, QTextCursor::KeepAnchor);
    const Clip foreign = copySelection(*other, sel2);
    QTextCursor at(doc->story());
    at.movePosition(QTextCursor::End);
    pasteClip(*doc, at, same);
    pasteClip(*doc, at, same);
    pasteClip(*doc, at, foreign);
    QCOMPARE(picturesIn(doc->story()), 4);
    const QList<int> ids = drawingIdsOf(documentXml(doc->toDocx()));
    QCOMPARE(ids.size(), 4);
    QCOMPARE(QSet<int>(ids.cbegin(), ids.cend()).size(), 4);
    // A new picture is above them all, and the undone paste leaves the numbers as they are.
    QVERIFY(doc->undo());
    QTextCursor end(doc->story());
    end.movePosition(QTextCursor::End);
    QCOMPARE(insertPicture(*doc, end, pngOf(9, 9), QStringLiteral("n")), PictureResult::Ok);
    const QList<int> more = drawingIdsOf(documentXml(doc->toDocx()));
    QCOMPARE(QSet<int>(more.cbegin(), more.cend()).size(), more.size());
}


void WriteTests::theDrawingIdsOfTheNotesAreCountedToo()
{
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + " xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\"><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                             "<w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:drawing><wp:inline><wp:extent cx=\"100\" cy=\"100\"/><wp:docPr id=\"50\" name=\"In a note\"/></wp:inline></w:drawing></w:r></w:p></w:footnote></w:footnotes>";
    const QByteArray body = "<w:p><w:r><w:t>text</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>";
    auto doc = open(makeDocxWithParts(body, pageSection(), {{QStringLiteral("/word/footnotes.xml"), notes}}));
    QVERIFY(doc);
    QTextCursor c(doc->story());
    QCOMPARE(insertPicture(*doc, c, pngOf(8, 8), QStringLiteral("a")), PictureResult::Ok);
    const QList<int> ids = drawingIdsOf(documentXml(doc->toDocx()));
    QCOMPARE(ids.size(), 1);
    QVERIFY2(ids[0] > 50, qPrintable(QString::number(ids[0])));
}


void WriteTests::aPastedPictureIsHeldToTheTotalOfTheDocumentsPictures()
{
    auto doc = open(makeDocxWithPicture("<w:p><w:r><w:t>Target.</w:t></w:r></w:p>"));
    auto other = textDoc({"Other."});
    QTextCursor oc(other->story());
    QCOMPARE(insertPicture(*other, oc, pngOf(50, 50, 9), QStringLiteral("o")), PictureResult::Ok);
    QTextCursor sel(other->story());
    sel.setPosition(0);
    sel.setPosition(1, QTextCursor::KeepAnchor);
    const Clip clip = copySelection(*other, sel);
    QCOMPARE(clip.links.size(), 1);
    // (The picture of the other document is not the target's: it would be added.)
    QTextCursor c(doc->story());
    QCOMPARE(insertPicture(*doc, c, pngOf(33, 33, 4), QStringLiteral("first")), PictureResult::Ok);
    doc->mediaLimit = doc->mediaBytes() + 5;
    const qint64 held = doc->mediaBytes();
    QTextCursor at(doc->story());
    at.movePosition(QTextCursor::End);
    pasteClip(*doc, at, clip);
    QCOMPARE(doc->mediaBytes(), held);
    QCOMPARE(picturesIn(doc->story()), 1);
    QVERIFY(!doc->pictureNotice.isEmpty());
    // With room, it goes in.
    doc->mediaLimit = kMostMediaBytes;
    pasteClip(*doc, at, clip);
    QCOMPARE(picturesIn(doc->story()), 2);
}


void WriteTests::tabStopsAParagraphHasBeyondWhatTheRulerShowsAreNeverDropped()
{
    auto doc = textDoc({"text"});
    QTextCursor c(doc->story());
    QVector<TabStopEntry> many;
    for (int i = 0; i < 5000; ++i) {
        TabStopEntry t;
        t.position = 5 + i * 0.25;
        many << t;
    }
    setTabStops(*doc, c, many);
    QCOMPARE(tabStopsOf(doc->story()->begin().blockFormat()).size(), 5000);
    TabStopEntry more;
    more.position = 3000.5;
    QVERIFY(!addTabStop(*doc, c, more));
    QVERIFY(moveTabStop(*doc, c, 5, 5.1));
    QCOMPARE(tabStopsOf(doc->story()->begin().blockFormat()).size(), 5000);
    QVERIFY(removeTabStop(*doc, c, 5.1));
    QCOMPARE(tabStopsOf(doc->story()->begin().blockFormat()).size(), 4999);
    QVERIFY(doc->undo());
    QCOMPARE(tabStopsOf(doc->story()->begin().blockFormat()).size(), 5000);
}


// Left and Right move over a picture the same way whichever way its form was made (made for the drawing, or the one made before for the same drawing), and over a picture in
// line as over a letter.
void WriteTests::theArrowKeysMovePastAPictureWhicheverWayItsFormWasMade()
{
    auto walk = [](WriteDoc &doc) {
        QList<int> where;
        QTextCursor k(doc.story());
        k.setPosition(3);
        for (int i = 0; i < 2; ++i) {
            moveCursor(k, QTextCursor::NextCharacter, false);
            where << k.position();
        }
        for (int i = 0; i < 2; ++i) {
            moveCursor(k, QTextCursor::PreviousCharacter, false);
            where << k.position();
        }
        return where;
    };
    for (PictureWrap wrap : {PictureWrap::Inline, PictureWrap::Square}) {
        auto doc = textDoc({QStringLiteral("ab cd")});
        QTextCursor c(doc->story());
        c.setPosition(3);
        QCOMPARE(insertPicture(*doc, c, pngOf(40, 40), QStringLiteral("p"), wrap), PictureResult::Ok);
        const QList<int> made = walk(*doc);
        if (wrap == PictureWrap::Inline) QCOMPARE(made, (QList<int>{4, 5, 4, 3}));   // (as over a letter)
        if (wrap != PictureWrap::Inline) {
            QVERIFY(setPictureWrap(*doc, 3, PictureWrap::Tight));
            QVERIFY(setPictureWrap(*doc, 3, PictureWrap::Square));   // (the form made before for it)
        } else {
            QVERIFY(setPicturePlace(*doc, 3, 100, 100));
            QVERIFY(doc->undo());
            QVERIFY(doc->redo());
            QVERIFY(doc->undo());
        }
        const QList<int> before = walk(*doc);
        QCOMPARE(before, made);
        auto fresh = open(doc->toDocx());   // (the form made for the drawing as a file opens it)
        QCOMPARE(walk(*fresh), made);
        const QList<QTextCharFormat> a = atomFormats(*doc, QStringLiteral("drawing")), b = atomFormats(*fresh, QStringLiteral("drawing"));
        QCOMPARE(a.size(), 1);
        QCOMPARE(b.size(), 1);
        QCOMPARE(a[0].stringProperty(tp::Field), b[0].stringProperty(tp::Field));
    }
}


// ---- wave D: styles, Copy Formatting, Go To, and dragging text (core/styleops.h, core/gotoops.h, moveSelectedText)

QTextCharFormat WriteTests::styCharAt(WriteDoc &doc, int position)
{
    QTextCursor t(doc.story());
    t.setPosition(position + 1);
    return t.charFormat();
}

QString WriteTests::styBlockStyle(WriteDoc &doc, int position) { return doc.story()->findBlock(position).blockFormat().stringProperty(tp::StyleName); }

wml::Style WriteTests::styStyle(WriteDoc &doc, const QString &id)
{
    const wml::Style *s = doc.styleSheet().find(id);
    return s ? *s : wml::Style();
}

QByteArray WriteTests::styCustomStyle(const QString &id, const QString &name, const QByteArray &inside, const QByteArray &type)
{
    return "<w:style w:type=\"" + type + "\" w:customStyle=\"1\" w:styleId=\"" + id.toUtf8() + "\"><w:name w:val=\"" + name.toUtf8() + "\"/>" + inside + "</w:style>";
}

QStringList WriteTests::styIds(const QVector<ListedStyle> &list)
{
    QStringList out;
    for (const ListedStyle &s : list) out << s.id;
    return out;
}


void WriteTests::theStylesPaneListsTheStylesAndFiltersThemByUseAndByRecommendation()
{
    const QByteArray styles = kStyles + styCustomStyle(QStringLiteral("Pull"), QStringLiteral("Pull quote"), "<w:basedOn w:val=\"Normal\"/><w:uiPriority w:val=\"20\"/><w:qFormat/>") +
                              styCustomStyle(QStringLiteral("Stealth"), QStringLiteral("Stealth note"), "<w:basedOn w:val=\"Normal\"/><w:semiHidden/>") +
                              styCustomStyle(QStringLiteral("Quiet"), QStringLiteral("quiet"), "<w:semiHidden/><w:unhideWhenUsed/><w:uiPriority w:val=\"30\"/>", "character");
    const QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Title</w:t></w:r></w:p>"
                            "<w:p><w:r><w:t xml:space=\"preserve\">Plain </w:t></w:r><w:r><w:rPr><w:rStyle w:val=\"Strong\"/></w:rPr><w:t>strong</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, styles));
    QVERIFY(doc);
    const QVector<ListedStyle> inUse = listStyles(*doc, StyleFilter::InUse);
    const QStringList inUseIds = styIds(inUse);
    QCOMPARE(QSet<QString>(inUseIds.begin(), inUseIds.end()), (QSet<QString>{QStringLiteral("Normal"), QStringLiteral("Heading1"), QStringLiteral("Strong")}));
    for (const ListedStyle &s : inUse) QVERIFY(s.inUse && s.inDocument);
    // (sorted by name, without regard to case)
    for (int i = 1; i < inUse.size(); ++i) QVERIFY(QString::compare(inUse[i - 1].name, inUse[i].name, Qt::CaseInsensitive) <= 0);
    const QVector<ListedStyle> all = listStyles(*doc, StyleFilter::All);
    const QStringList allIds = styIds(all);
    for (const char *id : {"Normal", "Heading1", "Strong", "Pull", "Stealth", "Quiet", "Title"}) QVERIFY2(allIds.contains(QLatin1String(id)), id);
    for (const ListedStyle &s : all) {
        if (s.id == QLatin1String("Title")) QVERIFY(!s.inDocument);   // (a built-in style the document lacks: added when it is used)
        if (s.id == QLatin1String("Quiet")) QVERIFY(s.character && s.name == QLatin1String("Quiet"));
        if (s.id == QLatin1String("Heading1")) QCOMPARE(s.name, QStringLiteral("Heading 1"));
    }
    const QStringList recommended = styIds(listStyles(*doc, StyleFilter::Recommended));
    for (const char *id : {"Normal", "Heading1", "Strong", "Pull", "Title"}) QVERIFY2(recommended.contains(QLatin1String(id)), id);
    QVERIFY(!recommended.contains(QStringLiteral("Stealth")));
    QVERIFY(!recommended.contains(QStringLiteral("Quiet")));
    // (by priority: the default style first, then Heading 1 (9) before the Pull quote (20))
    QCOMPARE(recommended.first(), QStringLiteral("Normal"));
    QVERIFY(recommended.indexOf(QStringLiteral("Heading1")) < recommended.indexOf(QStringLiteral("Pull")));
    // A style half hidden until it is used is shown, and is in use, once it is.
    QTextCursor c(doc->story());
    c.setPosition(0);
    c.setPosition(3, QTextCursor::KeepAnchor);
    applyCharacterStyle(*doc, c, QStringLiteral("Quiet"));
    QVERIFY(styIds(listStyles(*doc, StyleFilter::InUse)).contains(QStringLiteral("Quiet")));
    QVERIFY(styIds(listStyles(*doc, StyleFilter::Recommended)).contains(QStringLiteral("Quiet")));
    QVERIFY(doc->undo());
    QVERIFY(!styIds(listStyles(*doc, StyleFilter::InUse)).contains(QStringLiteral("Quiet")));
}


void WriteTests::aStyleMadeFromTheSelectionIsOneStepAndSavesAndOpensTheSame()
{
    auto doc = textDoc({QStringLiteral("First paragraph"), QStringLiteral("Second")});
    QTextCursor c(doc->story());
    c.setPosition(0);
    c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    toggleBold(*doc, c);
    setAlignment(*doc, c, Qt::AlignHCenter);
    const wml::Style def = styleFromSelection(*doc, c, QStringLiteral("Pull quote"));
    QCOMPARE(def.basedOn, QStringLiteral("Normal"));
    QCOMPARE(def.rPr.bold, std::optional<bool>(true));
    QCOMPARE(def.pPr.jc, QStringLiteral("center"));
    QVERIFY(def.customStyle && def.qFormat);
    const quint64 before = doc->nextStepSerial();
    const QString styleBefore = styBlockStyle(*doc, 0);
    QString problem;
    const QString id = createStyle(*doc, c, def, &problem, true);
    QVERIFY2(!id.isEmpty(), qPrintable(problem));
    QCOMPARE(id, QStringLiteral("Pullquote"));
    QCOMPARE(doc->nextStepSerial(), before + 1);   // (one step: the style and its being given to the paragraph)
    QCOMPARE(styBlockStyle(*doc, 0), id);
    QVERIFY(doc->styleSheet().find(id));
    // One Undo takes the style away and leaves the paragraph as it was (bold and centered by hand).
    QVERIFY(doc->undo());
    QVERIFY(!doc->styleSheet().find(id));
    QCOMPARE(styBlockStyle(*doc, 0), styleBefore);
    QCOMPARE(doc->story()->begin().blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
    QVERIFY(styCharAt(*doc, 2).fontWeight() >= QFont::DemiBold);
    QVERIFY(doc->redo());
    QVERIFY(doc->styleSheet().find(id));
    QCOMPARE(styBlockStyle(*doc, 0), id);
    // Saved, the paragraph says the style and nothing of its own; opened again, it is the same.
    const QByteArray saved = doc->toDocx();
    const QByteArray body = documentXml(saved);
    QVERIFY(body.contains("<w:pStyle w:val=\"Pullquote\"/>"));
    QVERIFY(!body.contains("<w:jc "));
    QVERIFY(!body.contains("<w:b/>"));
    auto back = open(saved);
    QVERIFY(back);
    const wml::Style made = styStyle(*back, id);
    QCOMPARE(made.name, QStringLiteral("Pull quote"));
    QCOMPARE(made.basedOn, QStringLiteral("Normal"));
    QVERIFY(made.customStyle && made.qFormat);
    QCOMPARE(made.pPr.jc, QStringLiteral("center"));
    QCOMPARE(made.rPr.bold, std::optional<bool>(true));
    QCOMPARE(styBlockStyle(*back, 0), id);
    QCOMPARE(back->story()->begin().blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
    QVERIFY(styCharAt(*back, 2).fontWeight() >= QFont::DemiBold);
    QCOMPARE(plain(*back), plain(*doc));
    // A character style from a selection is based on nothing and is given to the characters.
    QTextCursor word(doc->story());
    word.setPosition(18);
    word.setPosition(22, QTextCursor::KeepAnchor);
    toggleItalic(*doc, word);
    const wml::Style chr = styleFromSelection(*doc, word, QStringLiteral("Slanted"), true);
    QCOMPARE(chr.type, QStringLiteral("character"));
    QVERIFY(chr.basedOn.isEmpty());
    QCOMPARE(chr.rPr.italic, std::optional<bool>(true));
    const QString cid = createStyle(*doc, word, chr, &problem, true);
    QVERIFY2(!cid.isEmpty(), qPrintable(problem));
    QCOMPARE(styCharAt(*doc, 19).stringProperty(tp::CharStyle), cid);
}
