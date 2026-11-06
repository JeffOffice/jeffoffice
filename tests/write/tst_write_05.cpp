#include "tst_write.h"

QStringList WriteTests::endnoteReferenceNumbers(WriteDoc &doc)
{
    QStringList out;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (!cf.hasProperty(prop::Atom) || !storedBytes(cf, prop::Atom).contains("endnoteReference")) continue;
            const wml::Inline atom = unpackAtom(storedBytes(cf, prop::Atom));
            for (int k = 0; k < it.fragment().length(); ++k) out << atom.key + QLatin1Char(':') + cf.stringProperty(tp::Field).mid(4);
        }
    return out;
}

const QByteArray &WriteTests::betaNote()
{
    static const QByteArray s = "<w:endnote w:id=\"3\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Beta body note</w:t></w:r></w:p></w:endnote>";
    return s;
}

const QByteArray &WriteTests::sourcesNote()
{
    static const QByteArray s = "<w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:t>Sources</w:t></w:r></w:p></w:endnote>";
    return s;
}


// The words of an endnote are edited where the pages set them: typing in them is the note's, the body and the other notes and the separator are as they were, and what is
// saved has the one note rewritten and the rest as the file had it; an undo gives the file's own bytes back.
void WriteTests::theWordsOfAnEndnoteAreEditedWhereTheyStand()
{
    const QByteArray zip = notedDocx();
    auto doc = open(zip);
    QVERIFY(doc);
    QTextDocument *story = doc->story();
    const QString text = savedBody(*doc);
    const QTextBlock alpha = story->findBlockByNumber(3);
    QCOMPARE(alpha.text().mid(1), QStringLiteral(" Alpha body note brwon"));
    int id = 0;
    QVERIFY(doc->endnoteIdOf(alpha, &id));
    QCOMPARE(id, 2);
    QVERIFY(!doc->endnoteIdOf(story->findBlockByNumber(2)));   // (the separator is no note's)
    QVERIFY(!doc->endnoteIdOf(story->findBlockByNumber(1)));
    QTextCursor c(story);
    c.setPosition(alpha.position() + 2);
    c.setPosition(alpha.position() + 7, QTextCursor::KeepAnchor);   // "Alpha"
    insertText(*doc, c, QStringLiteral("ZZZ"));
    insertText(*doc, c, QStringLiteral("!"));
    QCOMPARE(story->findBlockByNumber(3).text().mid(1), QStringLiteral(" ZZZ! body note brwon"));
    QCOMPARE(story->findBlockByNumber(4).text().mid(1), QStringLiteral(" Beta body note"));
    QCOMPARE(story->findBlockByNumber(2).text(), QStringLiteral("Sources"));
    QCOMPARE(savedBody(*doc), text);
    const QByteArray saved = doc->toDocx();
    const QByteArray part = notesPartOf(saved);
    QVERIFY2(part.contains("ZZZ!") && !part.contains("Alpha"), part.constData());
    QVERIFY2(part.contains(betaNote()) && part.contains(sourcesNote()), part.constData());
    QVERIFY(rt::strictProblems(saved, zip).isEmpty());
    // The words are the story's, in the one history: an undo takes them back, and the part is the file's own.
    while (doc->canUndo()) doc->undo();
    QCOMPARE(story->findBlockByNumber(3).text().mid(1), QStringLiteral(" Alpha body note brwon"));
    QCOMPARE(notesPartOf(doc->toDocx()), notesPartOf(zip));
    // Saved and read again, the note says what was typed.
    doc->redo();
    auto again = open(doc->toDocx());
    QVERIFY(again);
    QVERIFY(again->story()->findBlockByNumber(3).text().contains(QLatin1String("ZZZ!")));
}


// No edit takes an endnote's number, or reaches the separator, or another note: the caret is held after the number, a selection stops at the end of the note it began in, and Backspace
// and Delete at a note's edges join it to nothing.
void WriteTests::noEditReachesTheSeparatorTheNumberOrAnotherEndnote()
{
    const QByteArray zip = notedDocx();
    auto doc = open(zip);
    QVERIFY(doc);
    QTextDocument *story = doc->story();
    const QString mark(QChar::ObjectReplacementCharacter);
    auto block = [&](int n) { return story->findBlockByNumber(n); };
    // The separator: nothing is typed, made, or deleted there.
    const QString before = story->toPlainText();
    QTextCursor sep(story);
    sep.setPosition(block(2).position() + 3);
    insertText(*doc, sep, QStringLiteral("ZZZ"));
    insertParagraph(*doc, sep);
    deleteBack(*doc, sep, false);
    deleteForward(*doc, sep, false);
    QCOMPARE(story->toPlainText(), before);
    // The number: typing before it goes after it; Backspace just after it does nothing; so does the word-wise one.
    QTextCursor c(story);
    c.setPosition(block(3).position());
    insertText(*doc, c, QStringLiteral("X"));
    QCOMPARE(block(3).text(), mark + QStringLiteral("X Alpha body note brwon"));
    c.setPosition(block(3).position() + 1);
    deleteBack(*doc, c, false);
    deleteBack(*doc, c, true);
    QCOMPARE(block(3).text(), mark + QStringLiteral("X Alpha body note brwon"));
    // A selection from inside the first note to inside the second stops at the end of the first: the second note is whole.
    QTextCursor across(story);
    across.setPosition(block(3).position() + 10);
    across.setPosition(block(4).position() + 6, QTextCursor::KeepAnchor);
    removeSelection(*doc, across);
    QCOMPARE(block(3).text(), mark + QStringLiteral("X Alpha b"));
    QCOMPARE(block(4).text(), mark + QStringLiteral(" Beta body note"));
    QCOMPARE(story->blockCount(), 5);
    // The same from the second note back into the first, and from a note into the separator: held to the note it began in.
    QTextCursor back(story);
    back.setPosition(block(4).position() + 6);
    back.setPosition(block(3).position() + 3, QTextCursor::KeepAnchor);
    removeSelection(*doc, back);
    QCOMPARE(block(3).text(), mark + QStringLiteral("X Alpha b"));
    QVERIFY(block(4).text().startsWith(mark));
    QCOMPARE(story->blockCount(), 5);
    // Backspace at the start of the second note's words, and Delete at the end of the first's, join neither.
    QTextCursor edge(story);
    edge.setPosition(block(4).position() + 1);
    deleteBack(*doc, edge, false);
    edge.setPosition(block(3).position() + block(3).length() - 1);
    deleteForward(*doc, edge, false);
    QCOMPARE(story->blockCount(), 5);
    QVERIFY(block(3).text().endsWith(QLatin1String("Alpha b")));
    // Every note's number is written as the file had it, and the separator is as it was.
    const QByteArray part = notesPartOf(doc->toDocx());
    QCOMPARE(int(part.count("<w:endnoteRef/>")), 2);
    QVERIFY(part.contains(sourcesNote()));
    QVERIFY(part.contains("body note"));
    // A selection of all the words of a note, deleted, leaves the number and the paragraph: the note is empty and still there.
    QTextCursor all(story);
    all.setPosition(block(3).position());
    all.setPosition(block(3).position() + block(3).length() - 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, all);
    QCOMPARE(block(3).text(), mark);
    QCOMPARE(story->blockCount(), 5);
}


// A note's words are edited as the text is: characters and paragraphs get their looks, Enter makes a paragraph that is the note's, the clipboard works inside a note, and what
// is copied out of a note and pasted in the text is the text's.
void WriteTests::anEndnoteIsEditedAsTheTextIs()
{
    const QByteArray zip = notedDocx();
    auto doc = open(zip);
    QVERIFY(doc);
    QTextDocument *story = doc->story();
    auto block = [&](int n) { return story->findBlockByNumber(n); };
    QTextCursor c(story);
    c.setPosition(block(3).position() + 2);
    c.setPosition(block(3).position() + 7, QTextCursor::KeepAnchor);
    toggleBold(*doc, c);
    QVERIFY(allSelected(c, [](const QTextCharFormat &f) { return f.fontWeight() >= QFont::DemiBold; }));
    setAlignment(*doc, c, Qt::AlignRight);
    QCOMPARE(int(block(3).blockFormat().alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignRight));
    QCOMPARE(int(block(4).blockFormat().alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignLeft));
    // Enter in the middle of the first note: a second paragraph, the note's own.
    c.setPosition(block(3).position() + 8);
    insertParagraph(*doc, c);
    QCOMPARE(story->blockCount(), 6);
    int id = 0;
    QVERIFY(doc->endnoteIdOf(block(4), &id) && id == 2);
    QVERIFY(block(4).blockFormat().hasProperty(prop::EndnoteBlock));
    QCOMPARE(tailOf(*doc), 4);
    typeText(*doc, c, QStringLiteral("more"));
    QVERIFY(block(4).text().startsWith(QLatin1String("more")));
    // Backspace at the start of the second paragraph joins it to the first, as in the text.
    c.setPosition(block(4).position());
    deleteBack(*doc, c, false);
    QCOMPARE(story->blockCount(), 5);
    // The clipboard in the note: some words copied and pasted at the end of the note.
    c.setPosition(block(3).position() + 2);
    c.setPosition(block(3).position() + 4, QTextCursor::KeepAnchor);
    const QString words = c.selectedText();
    const QTextDocumentFragment held = selectionFragment(c);
    QCOMPARE(plainTextOf(held), words);
    c.setPosition(block(3).position() + block(3).length() - 1);
    pasteFragment(*doc, c, held);
    QVERIFY(block(3).text().endsWith(words));
    QCOMPARE(tailOf(*doc), 3);
    // Words of a note pasted in the text are the text's.
    QTextCursor in(story);
    in.setPosition(block(0).position() + 5);
    pasteFragment(*doc, in, held);
    QCOMPARE(tailOf(*doc), 3);
    QVERIFY(savedBody(*doc).contains(words));
    const QByteArray part = notesPartOf(doc->toDocx());
    QVERIFY2(part.contains("<w:jc w:val=\"right\"/>") && part.contains("<w:b/>"), part.constData());
    // Taken back, the part is the file's bytes.
    while (doc->canUndo()) doc->undo();
    QCOMPARE(notesPartOf(doc->toDocx()), notesPartOf(zip));
    QCOMPARE(tailOf(*doc), 3);
}


// Insert > Endnote puts a reference in the text and a new note among the endnotes, in the order of the references, and numbers the others anew (the references and the notes' marks);
// the caret is in the new note's words. An undo takes the note, the reference, and the new numbers back; the file has the note once, and the others as they were.
void WriteTests::insertEndnoteMakesAReferenceAndANoteAndNumbersTheOthersAnew()
{
    const QByteArray zip = notedDocx();
    auto doc = open(zip);
    QVERIFY(doc);
    QTextDocument *story = doc->story();
    QCOMPARE(endnoteMarks(*doc), (QStringList{"2:1", "3:2"}));
    QCOMPARE(endnoteReferenceNumbers(*doc), (QStringList{"2:1", "3:2"}));
    QTextCursor c(story);
    c.setPosition(5);   // (after "First")
    QVERIFY(insertEndnote(*doc, c));
    QCOMPARE(c.document(), story);
    int id = 0;
    QVERIFY(doc->endnoteIdOf(c.block(), &id));
    QCOMPARE(id, 4);
    QCOMPARE(c.position(), c.block().position() + c.block().length() - 1);   // (at the end of the note's words)
    QCOMPARE(c.block().text(), QString(QChar::ObjectReplacementCharacter) + QStringLiteral(" "));
    // The notes are in the order of their references, and each reference and each note shows its number.
    QCOMPARE(endnoteMarks(*doc), (QStringList{"4:1", "2:2", "3:3"}));
    QCOMPARE(endnoteReferenceNumbers(*doc), (QStringList{"4:1", "2:2", "3:3"}));
    QCOMPARE(doc->endnoteReferences(4), (QVector<int>{5}));
    QCOMPARE(tailOf(*doc), 4);
    typeText(*doc, c, QStringLiteral("A new endnote."));
    const QByteArray saved = doc->toDocx();
    const QByteArray part = notesPartOf(saved);
    QVERIFY2(part.contains("<w:endnote w:id=\"4\">") && part.contains("A new endnote."), part.constData());
    QVERIFY(part.contains(betaNote()) && part.contains(sourcesNote()));
    QCOMPARE(int(documentXml(saved).count("<w:endnoteReference w:id=\"4\"/>")), 1);
    QCOMPARE(savedBody(*doc), QStringLiteral("First body paragraph|Second body paragraph"));
    QVERIFY(rt::strictProblems(saved, zip).isEmpty());
    // Undone (the words, then the note with its reference): as it was, to the byte.
    doc->undo();
    doc->undo();
    QCOMPARE(endnoteMarks(*doc), (QStringList{"2:1", "3:2"}));
    QCOMPARE(endnoteReferenceNumbers(*doc), (QStringList{"2:1", "3:2"}));
    QCOMPARE(tailOf(*doc), 3);
    QVERIFY(!doc->canUndo());
    QCOMPARE(notesPartOf(doc->toDocx()), notesPartOf(zip));
    QVERIFY(!doc->isModified());
    // Done again, both are back, with the words.
    doc->redo();
    doc->redo();
    QCOMPARE(endnoteMarks(*doc), (QStringList{"4:1", "2:2", "3:3"}));
    QVERIFY(notesPartOf(doc->toDocx()).contains("A new endnote."));
}


// A reference taken out of the text takes its endnote with it, in the text and in the file, and the others are numbered anew; an undo brings both back, to the byte.
void WriteTests::deletingAnEndnoteReferenceDeletesItsNoteAndUndoBringsBothBack()
{
    const QByteArray zip = notedDocx();
    auto doc = open(zip);
    QVERIFY(doc);
    QTextDocument *story = doc->story();
    const QVector<int> refs = doc->endnoteReferences(2);
    QCOMPARE(int(refs.size()), 1);
    QTextCursor c(story);
    c.setPosition(refs[0]);
    c.setPosition(refs[0] + 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, c);
    QVERIFY(doc->endnoteReferences(2).isEmpty());
    QCOMPARE(endnoteMarks(*doc), (QStringList{"3:1"}));
    QCOMPARE(endnoteReferenceNumbers(*doc), (QStringList{"3:1"}));
    QCOMPARE(tailOf(*doc), 2);
    QByteArray part = notesPartOf(doc->toDocx());
    QVERIFY2(!part.contains("Alpha") && part.contains(betaNote()) && part.contains(sourcesNote()), part.constData());
    QVERIFY(!documentXml(doc->toDocx()).contains("endnoteReference w:id=\"2\""));
    doc->undo();
    QCOMPARE(endnoteMarks(*doc), (QStringList{"2:1", "3:2"}));
    QCOMPARE(endnoteReferenceNumbers(*doc), (QStringList{"2:1", "3:2"}));
    QCOMPARE(tailOf(*doc), 3);
    QCOMPARE(notesPartOf(doc->toDocx()), notesPartOf(zip));
    doc->redo();
    QVERIFY(!notesPartOf(doc->toDocx()).contains("Alpha"));
    doc->undo();
    // Backspace after a reference, and Delete before one, are the same; so is the whole text selected and deleted (no notes then, and all of them back with an undo).
    QTextCursor key(story);
    key.setPosition(doc->endnoteReferences(3).first() + 1);
    deleteBack(*doc, key, false);
    QVERIFY(doc->endnoteReferences(3).isEmpty());
    QVERIFY(!notesPartOf(doc->toDocx()).contains("Beta"));
    doc->undo();
    key.select(QTextCursor::Document);
    removeSelection(*doc, key);
    QCOMPARE(tailOf(*doc), 0);
    QVERIFY(!notesPartOf(doc->toDocx()).contains("Alpha"));
    doc->undo();
    QCOMPARE(tailOf(*doc), 3);
    QCOMPARE(notesPartOf(doc->toDocx()), notesPartOf(zip));
}


// Cutting a reference and pasting it moves its note (with what had been written in it); copying one and pasting it makes a note of its own with the same words.
void WriteTests::cuttingAndPastingAnEndnoteReferenceMovesItsNote()
{
    auto doc = open(notedDocx());
    QVERIFY(doc);
    QTextDocument *story = doc->story();
    // Words typed in the first note, then its reference cut and pasted after the second's: the notes are in the other order, and the first says what was typed.
    QTextCursor c(story);
    c.setPosition(story->findBlockByNumber(3).position() + story->findBlockByNumber(3).length() - 1);
    insertText(*doc, c, QStringLiteral(" edited"));
    const int at = doc->endnoteReferences(2).first();
    QTextCursor cut(story);
    cut.setPosition(at);
    cut.setPosition(at + 1, QTextCursor::KeepAnchor);
    const QTextDocumentFragment held = selectionFragment(cut);
    removeSelection(*doc, cut);
    QVERIFY(doc->endnoteReferences(2).isEmpty());
    QCOMPARE(endnoteMarks(*doc), (QStringList{"3:1"}));
    QTextCursor into(story);
    into.setPosition(story->findBlockByNumber(1).position() + story->findBlockByNumber(1).length() - 1);
    pasteFragment(*doc, into, held);
    QCOMPARE(endnoteMarks(*doc), (QStringList{"3:1", "2:2"}));
    QCOMPARE(endnoteReferenceNumbers(*doc), (QStringList{"3:1", "2:2"}));
    QVERIFY(endnoteParagraphs(*doc, 2).first().text().endsWith(QLatin1String("brwon edited")));
    QByteArray part = notesPartOf(doc->toDocx());
    QCOMPARE(int(part.count("Alpha")), 1);
    QVERIFY(part.contains("brwon edited"));
    // Copied: a note of its own, with the words.
    QTextCursor copy(story);
    const int first = doc->endnoteReferences(3).first();
    copy.setPosition(first);
    copy.setPosition(first + 1, QTextCursor::KeepAnchor);
    const QTextDocumentFragment again = selectionFragment(copy);
    QTextCursor start(story);
    start.setPosition(0);
    pasteFragment(*doc, start, again);
    QCOMPARE(int(doc->endnoteReferences(3).size()), 1);
    QCOMPARE(int(doc->endnoteReferences(4).size()), 1);
    QCOMPARE(endnoteMarks(*doc), (QStringList{"4:1", "3:2", "2:3"}));
    QVERIFY(endnoteParagraphs(*doc, 4).first().text().endsWith(QLatin1String("Beta body note")));
    part = notesPartOf(doc->toDocx());
    QCOMPARE(int(part.count("Beta body note")), 2);
    QVERIFY(documentXml(doc->toDocx()).contains("<w:endnoteReference w:id=\"4\"/>"));
    doc->undo();
    QVERIFY(doc->endnoteReferences(4).isEmpty());
    QCOMPARE(endnoteMarks(*doc), (QStringList{"3:1", "2:2"}));
}


// Where the notes are set at the end of each section, an edit in a note is the note's (the section's break stays on its last paragraph), and a new endnote is set at the end of its section.
void WriteTests::endnotesAtTheEndOfASectionAreEditedAndMadeThere()
{
    const QByteArray pr = "<w:endnotePr><w:pos w:val=\"sectEnd\"/></w:endnotePr>";
    const QByteArray zip = sectionNotedDocx(pr, pr);
    auto doc = open(zip);
    QVERIFY(doc);
    QTextDocument *story = doc->story();
    auto breaks = [&] {
        QList<bool> out;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) out << b.blockFormat().boolProperty(flow::fp::SectionBreakAfter);
        return out;
    };
    const QList<bool> was = breaks();
    QTextCursor c(story);
    const QTextBlock alpha = story->findBlockByNumber(3);
    c.setPosition(alpha.position() + 2);
    insertText(*doc, c, QStringLiteral("ZZZ"));
    QCOMPARE(story->findBlockByNumber(3).text().mid(1, 4), QStringLiteral(" ZZZ"));
    QCOMPARE(breaks(), was);
    QCOMPARE(savedBody(*doc), QStringLiteral("First body|Second body|Third body"));
    QVERIFY(notesPartOf(doc->toDocx()).contains("ZZZ"));
    QCOMPARE(doc->sectionCount(), 2);
    // A new endnote at the end of the first section's text: it is set with that section's notes, after the first of them (its reference is after the first's), and the break moves to the new last.
    QTextCursor end(story);
    end.setPosition(story->findBlockByNumber(1).position() + story->findBlockByNumber(1).length() - 1);
    QVERIFY(insertEndnote(*doc, end));
    QCOMPARE(doc->sectionCount(), 2);
    int id = 0;
    QVERIFY(doc->endnoteIdOf(end.block(), &id));
    QCOMPARE(id, 4);
    QCOMPARE(endnoteMarks(*doc), (QStringList{"2:1", "4:2", "3:3"}));
    QVERIFY(story->findBlockByNumber(4).blockFormat().boolProperty(flow::fp::SectionBreakAfter));   // (the break is on the last note of the section)
    QVERIFY(!story->findBlockByNumber(3).blockFormat().boolProperty(flow::fp::SectionBreakAfter));
    QCOMPARE(savedBody(*doc), QStringLiteral("First body|Second body|Third body"));
    QCOMPARE(documentXml(doc->toDocx()).count("<w:sectPr"), 2);
    QVERIFY(notesPartOf(doc->toDocx()).contains("<w:endnote w:id=\"4\">"));
    doc->undo();
    QCOMPARE(breaks(), was);
    QCOMPARE(endnoteMarks(*doc), (QStringList{"2:1", "3:2"}));
}


// A document with no endnotes gets an endnotes part when one is inserted: the part with its separators, its content type and relationship, the settings that name them, and the styles;
// it reads back; and one put in and taken back leaves the file as it was.
void WriteTests::aNewEndnotesPartIsValidAndReadsBack()
{
    const QByteArray original = makeDocxWithParts(paragraphs(3, 20), pageSection(), {}, QByteArray(), kStyles);
    auto doc = open(original);
    QVERIFY(doc);
    QTextCursor c(doc->story());
    c.setPosition(doc->story()->findBlockByNumber(1).position() + 4);
    QVERIFY(insertEndnote(*doc, c));
    typeText(*doc, c, QStringLiteral("A new endnote."));
    const QByteArray saved = doc->toDocx();
    QStringList problems = rt::strictProblems(saved, original);
    QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QLatin1Char('\n'))));
    QVERIFY(rt::danglingReferences(saved).isEmpty());
    office::Package pkg;
    QVERIFY(pkg.read(saved));
    QVERIFY(pkg.hasPart(QStringLiteral("/word/endnotes.xml")));
    QCOMPARE(pkg.contentType(QStringLiteral("/word/endnotes.xml")), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.endnotes+xml"));
    QCOMPARE(int(pkg.relationshipsOfType(pkg.mainPart(), QStringLiteral("/endnotes")).size()), 1);
    QVector<wml::NotePart> notes;
    QVERIFY(wml::readNotes(pkg.part(QStringLiteral("/word/endnotes.xml")), &notes));
    QCOMPARE(int(notes.size()), 3);
    QCOMPARE(notes[0].type, QStringLiteral("separator"));
    QCOMPARE(notes[1].type, QStringLiteral("continuationSeparator"));
    QCOMPARE(notes[2].id, 1);
    const QString settingsName = pkg.relationshipsOfType(pkg.mainPart(), QStringLiteral("/settings")).first().target;
    QVERIFY(pkg.part(settingsName).contains("<w:endnotePr><w:endnote w:id=\"-1\"/><w:endnote w:id=\"0\"/></w:endnotePr>"));
    const QByteArray styles = pkg.part(QStringLiteral("/word/styles.xml"));
    QVERIFY(styles.contains("w:styleId=\"EndnoteText\"") && styles.contains("w:styleId=\"EndnoteReference\""));
    QVERIFY(documentXml(saved).contains("<w:endnoteReference w:id=\"1\"/>"));
    auto back = open(saved);
    QVERIFY(back);
    QCOMPARE(back->citedEndnotes(), (QVector<int>{1}));
    QVERIFY(endnoteParagraphs(*back, 1).first().text().endsWith(QLatin1String("A new endnote.")));
    QCOMPARE(notesPartOf(back->toDocx()), pkg.part(QStringLiteral("/word/endnotes.xml")));
    QCOMPARE(documentXml(back->toDocx()), documentXml(saved));
    // Put in and taken back: no part, no relationship, no styles.
    auto other = open(original);
    QVERIFY(other);
    const QByteArray before = other->toDocx();
    QTextCursor d(other->story());
    d.setPosition(4);
    QVERIFY(insertEndnote(*other, d));
    typeText(*other, d, QStringLiteral("typed"));
    while (other->canUndo()) other->undo();
    QVERIFY(!other->isModified());
    const QByteArray after = other->toDocx();
    office::Package a, b;
    QVERIFY(a.read(before));
    QVERIFY(b.read(after));
    QCOMPARE(b.partNames(), a.partNames());
    QCOMPARE(b.part(QStringLiteral("/word/styles.xml")), a.part(QStringLiteral("/word/styles.xml")));
    QCOMPARE(documentXml(after), documentXml(before));
    QVERIFY(!b.hasPart(QStringLiteral("/word/endnotes.xml")));
}


// ---- footnotes: hostile files

// A file of thousands of notes (most of them cited from nowhere): opening it costs what it did, an edit costs no more than in a file of three, no note is made past the most the
// program has, and the part keeps every note, the ones not edited as they were.
void WriteTests::thousandsOfFootnotesAreEditedWithinBounds()
{
    constexpr int count = 5200;
    QByteArray body;
    QList<std::pair<int, QByteArray>> notes;
    for (int i = 1; i <= count; ++i) notes << std::make_pair(i, plainRun(" note " + QByteArray::number(i)));
    for (int i = 1; i <= 40; ++i) body += citing("Paragraph " + QByteArray::number(i), i);
    const QByteArray zip = notesDocx(body, footnotesPart(notes));
    resetNoteStoriesMadeForTest();
    auto doc = open(zip);
    QVERIFY(doc);
    QVERIFY(doc->pager().pageCount() >= 1);
    QVERIFY2(noteStoriesMadeForTest() <= 60, qPrintable(QString::number(noteStoriesMadeForTest())));   // (the notes of the pages laid out, not the part's thousands)
    QTextDocument *first = doc->editableFootnote(1);
    QVERIFY(first);
    const qint64 madeBefore = noteStoriesMadeForTest();
    typeAtEnd(*doc, first, QStringLiteral(" edited"));
    QCOMPARE(noteStoriesMadeForTest(), madeBefore);   // (typing in a note makes no other story)
    QTextCursor c(doc->story());
    c.setPosition(2);
    QVERIFY(doc->roomForFootnote());   // (forty are cited: the part's other notes are no count)
    QTextDocument *made = insertFootnote(*doc, c);
    QVERIFY(made && made != first);
    QCOMPARE(doc->undo(), doc->story());   // (the insertion is the last step: undone, the part is as it was)
    const QByteArray saved = doc->toDocx();
    QVERIFY(rt::strictProblems(saved, zip).isEmpty());
    const QByteArray part = partOf(saved, QStringLiteral("/word/footnotes.xml"));
    QCOMPARE(int(part.count("<w:footnoteRef/>")), count);
    QVERIFY(part.contains(" note 1 edited"));
    QVERIFY(part.contains("<w:footnote w:id=\"5150\"><w:p><w:pPr><w:pStyle w:val=\"FootnoteText\"/></w:pPr><w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> note 5150</w:t></w:r></w:p></w:footnote>"));
}


// A reference to a note the file lacks is kept and does nothing; a new note takes an Id that no reference uses.
void WriteTests::aReferenceToAMissingNoteIsKeptAndNoNewNoteTakesItsId()
{
    const QByteArray body = citing("First", 2) + citing("Second", 9);
    auto doc = open(notesDocx(body, footnotesPart({{2, plainRun(" two")}})));
    QVERIFY(doc);
    QCOMPARE(int(notesOnPage(*doc, 0).size()), 1);
    QTextCursor c(doc->story());
    c.setPosition(2);
    QTextDocument *note = insertFootnote(*doc, c);
    QVERIFY(note);
    int id = 0;
    QVERIFY(doc->footnoteIdOf(note, &id));
    QCOMPARE(id, 10);
    // (The reference to the missing note comes out and goes back without a note appearing or the file changing.)
    QTextCursor gone(doc->story());
    const QTextBlock second = doc->story()->findBlockByNumber(1);
    const int at = second.position() + second.text().indexOf(QChar::ObjectReplacementCharacter);
    QVERIFY(at >= second.position());
    gone.setPosition(at);
    gone.setPosition(at + 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, gone);
    const QByteArray part = partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml"));
    QVERIFY(part.contains("w:id=\"2\"") && part.contains("w:id=\"10\""));
    doc->undo();
    QVERIFY(documentXml(doc->toDocx()).contains("footnoteReference w:id=\"9\""));
    QVERIFY(rt::strictProblems(doc->toDocx(), QByteArray()).isEmpty());
}


// A note of thousands of paragraphs and a table: the story holds the most the page shows, the rest is kept in it, and an edit of the first paragraph writes every one.
void WriteTests::aHugeFootnoteKeepsEveryParagraphWhenItIsEdited()
{
    constexpr int paragraphsInNote = 1500;
    QByteArray inner = "<w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>first</w:t></w:r></w:p>";
    for (int i = 0; i < paragraphsInNote; ++i) inner += "<w:p><w:r><w:t>p" + QByteArray::number(i) + "</w:t></w:r></w:p>";
    inner += "<w:tbl><w:tblPr><w:tblW w:w=\"4000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>cellA</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>cellB</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>last</w:t></w:r></w:p>";
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote><w:footnote w:id=\"2\">" + inner + "</w:footnote></w:footnotes>";
    const QByteArray zip = notesDocx(citing("A paragraph", 2) + paragraphs(2, 20), notes);
    auto doc = open(zip);
    QVERIFY(doc);
    QTextDocument *note = doc->editableFootnote(2);
    QVERIFY(note);
    QVERIFY2(note->blockCount() <= 410, qPrintable(QString::number(note->blockCount())));
    QTextCursor c(note);
    c.setPosition(noteTextStart(note));
    insertText(*doc, c, QStringLiteral(">"));
    const QByteArray saved = doc->toDocx();
    const QByteArray part = partOf(saved, QStringLiteral("/word/footnotes.xml"));
    QCOMPARE(int(part.count("<w:p>")) + int(part.count("<w:p ")), paragraphsInNote + 5);   // (the separator's, the first, the 1500, the two cells', and the last)
    QVERIFY(part.contains("&gt;first"));
    QVERIFY(part.contains("p1499") && part.contains("cellA") && part.contains("cellB") && part.contains("last"));
    QVERIFY(part.indexOf("p1499") < part.indexOf("<w:tbl>") && part.indexOf("</w:tbl>") < part.indexOf("last"));
    QVERIFY(rt::strictProblems(saved, zip).isEmpty());
}


// A note cited from a header (and nowhere in the text) is the program's to keep: it is not shown on the pages, deleting its reference there takes nothing from the file, and the note edits
// and saves. A note cited in a table's cell is shown and edited, and a note that holds a table keeps it.
void WriteTests::notesInAHeaderAndInATableAreKeptAndEdited()
{
    const QByteArray header = headerPart("hdr", {"<w:r><w:t>Head</w:t></w:r><w:r><w:footnoteReference w:id=\"7\"/></w:r>"});
    const QByteArray cell = "<w:tbl><w:tblPr><w:tblW w:w=\"5000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"5000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t xml:space=\"preserve\">In a cell </w:t></w:r>"
                            "<w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr><w:footnoteReference w:id=\"2\"/></w:r></w:p></w:tc></w:tr></w:tbl><w:p/>";
    const QByteArray tableNote = "<w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>with a table</w:t></w:r></w:p><w:tbl><w:tblPr><w:tblW w:w=\"3000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"1500\"/><w:gridCol w:w=\"1500\"/></w:tblGrid>"
                                 "<w:tr><w:tc><w:p><w:r><w:t>c1</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>c2</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>after</w:t></w:r></w:p>";
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote><w:footnote w:id=\"2\">" + tableNote +
                             "</w:footnote><w:footnote w:id=\"7\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>in the header</w:t></w:r></w:p></w:footnote></w:footnotes>";
    const QByteArray zip = makeDocxWithParts(cell + paragraphs(2, 20), sectionNaming({{"default", "rIdheader1"}}),
                                             {{QStringLiteral("/word/header1.xml"), header}, {QStringLiteral("/word/footnotes.xml"), notes}}, QByteArray(), noteStyles());
    auto doc = open(zip);
    QVERIFY(doc);
    QCOMPARE(int(notesOnPage(*doc, 0).size()), 1);   // (the cell's; the header's is not set at the foot)
    QTextDocument *headed = doc->editableFootnote(7);
    QVERIFY(headed);
    typeAtEnd(*doc, headed, QStringLiteral("!"));
    QTextDocument *tabled = doc->editableFootnote(2);
    QVERIFY(tabled);
    QTextCursor c(tabled);
    c.setPosition(noteTextStart(tabled) + 2);
    insertText(*doc, c, QStringLiteral("X"));
    // The header's reference taken out: nothing of the file goes.
    QTextDocument *head = doc->editableHeaderFooter(0, false, WriteDoc::kDefaultSlot);
    QVERIFY(head);
    QTextCursor hc(head);
    hc.select(QTextCursor::Document);
    removeSelection(*doc, hc);
    const QByteArray saved = doc->toDocx();
    const QByteArray part = partOf(saved, QStringLiteral("/word/footnotes.xml"));
    QVERIFY2(part.contains("in the header!"), part.constData());
    QVERIFY(part.contains("c1") && part.contains("c2") && part.contains("after") && part.contains("<w:tbl>"));
    QVERIFY(part.contains("wiXth a table"));
    QVERIFY(rt::strictProblems(saved, zip).isEmpty());
    // A footnote inserted in the cell is shown and written.
    auto doc2 = open(zip);
    QVERIFY(doc2);
    QTextCursor in(doc2->story());
    in.setPosition(3);
    QTextDocument *made = insertFootnote(*doc2, in);
    QVERIFY(made);
    typeAtEnd(*doc2, made, QStringLiteral("cell note"));
    QVERIFY(partOf(doc2->toDocx(), QStringLiteral("/word/footnotes.xml")).contains("cell note"));
    QVERIFY(layoutDiffersFromFresh(*doc2).isEmpty());
}


// A footnotes part that is not XML is left as it is: no note is made over it, and it is written back untouched.
void WriteTests::anUnreadableFootnotesPartIsLeftAsItIsAndNoNoteIsMadeInIt()
{
    const QByteArray junk = "<w:footnotes not xml at all";
    auto doc = open(notesDocx(citing("A paragraph", 2) + paragraphs(2, 20), junk));
    QVERIFY(doc);
    QTextCursor c(doc->story());
    c.setPosition(3);
    QVERIFY(!insertFootnote(*doc, c));
    QVERIFY(!doc->editableFootnote(2));
    QCOMPARE(partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")), xml(junk));
    QVERIFY(!doc->canUndo());
}


// Two notes with one Id: the first is the note, and is the one that is edited and written; the other is kept as it was.
void WriteTests::twoNotesWithOneIdAreOnlyTheFirstEditedAndTheOtherIsKept()
{
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>first</w:t></w:r></w:p></w:footnote>"
                             "<w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>second</w:t></w:r></w:p></w:footnote></w:footnotes>";
    auto doc = open(notesDocx(citing("A paragraph", 2) + paragraphs(2, 20), notes));
    QVERIFY(doc);
    QTextDocument *note = doc->editableFootnote(2);
    QVERIFY(note);
    QVERIFY(note->toPlainText().endsWith(QStringLiteral("first")));
    typeAtEnd(*doc, note, QStringLiteral("!"));
    const QByteArray part = partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml"));
    QVERIFY2(part.contains("first!") && part.contains("<w:t>second</w:t>"), part.constData());
    QCOMPARE(int(part.count("<w:footnote ")), 2);
    // The reference taken out: the first goes, the second (which no reference can name) stays.
    QTextCursor c(doc->story());
    const int at = doc->footnoteReferences(2).first();
    c.setPosition(at);
    c.setPosition(at + 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, c);
    const QByteArray after = partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml"));
    QVERIFY2(!after.contains("first") && after.contains("<w:t>second</w:t>"), after.constData());
    QVERIFY(rt::strictProblems(doc->toDocx(), QByteArray()).isEmpty());
}


// ---- endnotes: hostile files

// A file of thousands of endnotes (most of them cited from nowhere): opening it costs what it did, an edit costs no more than in a file of three, no endnote is made past the most the
// program has, and the part keeps every note, the ones not edited as they were.
void WriteTests::thousandsOfEndnotesAreEditedWithinBounds()
{
    constexpr int count = 5200;
    QByteArray notes = "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote();
    for (int i = 1; i <= count; ++i) notes += "<w:endnote w:id=\"" + QByteArray::number(i) + "\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> note " + QByteArray::number(i) + "</w:t></w:r></w:p></w:endnote>";
    notes += "</w:endnotes>";
    QByteArray body;
    for (int i = 1; i <= 40; ++i) body += "<w:p><w:r><w:t>Paragraph " + QByteArray::number(i) + "</w:t></w:r><w:r><w:endnoteReference w:id=\"" + QByteArray::number(i) + "\"/></w:r></w:p>";
    const QByteArray zip = makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), notes}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>");
    resetEndnoteParagraphsMadeForTest();
    auto doc = open(zip);
    QVERIFY(doc);
    QVERIFY(doc->pager().pageCount() >= 1);
    QVERIFY2(endnoteParagraphsMadeForTest() <= 200, qPrintable(QString::number(endnoteParagraphsMadeForTest())));   // (the forty notes the text cites, not the part's thousands)
    QCOMPARE(tailOf(*doc), 41);
    int from = 0, to = 0;
    QVERIFY(doc->endnoteWords(1, &from, &to));
    const qint64 madeBefore = endnoteParagraphsMadeForTest();
    QTextCursor c(doc->story());
    c.setPosition(to);
    for (const QChar ch : QStringLiteral(" edited")) insertText(*doc, c, QString(ch));
    QCOMPARE(endnoteParagraphsMadeForTest(), madeBefore);   // (typing in a note sets no note again)
    QTextCursor at(doc->story());
    at.setPosition(2);
    QVERIFY(doc->roomForEndnote());   // (forty are cited: the part's other notes are no count)
    QVERIFY(insertEndnote(*doc, at));
    doc->undo();   // (taken back at once: the part is as it was)
    const QByteArray saved = doc->toDocx();
    QVERIFY(rt::strictProblems(saved, zip).isEmpty());
    const QByteArray part = notesPartOf(saved);
    QCOMPARE(int(part.count("<w:endnoteRef/>")), count);
    QVERIFY(part.contains(" note 1 edited"));
    QVERIFY(part.contains("<w:endnote w:id=\"5150\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> note 5150</w:t></w:r></w:p></w:endnote>"));
    QVERIFY(part.contains("<w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> note 2</w:t></w:r></w:p></w:endnote>"));
}


// A reference to an endnote the file lacks is kept and does nothing; a new note takes an Id that no reference uses.
void WriteTests::aReferenceToAMissingEndnoteIsKeptAndNoNewNoteTakesItsId()
{
    const QByteArray body = "<w:p><w:r><w:t>First</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p><w:p><w:r><w:t>Second</w:t></w:r><w:r><w:endnoteReference w:id=\"9\"/></w:r></w:p>";
    const QByteArray notes = "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote() + "<w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t> two</w:t></w:r></w:p></w:endnote></w:endnotes>";
    const QByteArray zip = makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), notes}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>");
    auto doc = open(zip);
    QVERIFY(doc);
    QCOMPARE(endnoteMarks(*doc), (QStringList{"2:1"}));
    QTextCursor c(doc->story());
    c.setPosition(2);
    QVERIFY(insertEndnote(*doc, c));
    int id = 0;
    QVERIFY(doc->endnoteIdOf(c.block(), &id));
    QCOMPARE(id, 10);
    // (The reference to the missing note comes out and goes back without a note appearing or the file changing.)
    const QByteArray before = notesPartOf(doc->toDocx());
    QTextBlock second = doc->story()->findBlockByNumber(1);
    const int at = second.position() + second.text().indexOf(QChar::ObjectReplacementCharacter);
    QVERIFY(at >= second.position());
    QTextCursor gone(doc->story());
    gone.setPosition(at);
    gone.setPosition(at + 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, gone);
    QCOMPARE(notesPartOf(doc->toDocx()), before);
    doc->undo();
    QVERIFY(documentXml(doc->toDocx()).contains("endnoteReference w:id=\"9\""));
    QVERIFY(rt::strictProblems(doc->toDocx(), QByteArray()).isEmpty());
}


// A note cited twice is set twice: the first copy's words are edited and written, and the second is shown and left alone; the reference of the first taken out, the second is the note's
// and is edited (what was written in the first is in it).
void WriteTests::anEndnoteCitedTwiceIsEditedInItsFirstCopyOnly()
{
    const QByteArray body = "<w:p><w:r><w:t>First</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p><w:p><w:r><w:t>Second</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>";
    const QByteArray notes = "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote() + "<w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> shared</w:t></w:r></w:p></w:endnote></w:endnotes>";
    const QByteArray zip = makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), notes}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>");
    auto doc = open(zip);
    QVERIFY(doc);
    QTextDocument *story = doc->story();
    QCOMPARE(endnoteMarks(*doc), (QStringList{"2:1", "2:2"}));
    QCOMPARE(tailOf(*doc), 3);
    const QTextBlock first = story->findBlockByNumber(3), second = story->findBlockByNumber(4);
    QVERIFY(first.blockFormat().intProperty(prop::EndnoteNote) > 0 && second.blockFormat().intProperty(prop::EndnoteNote) < 0);
    // The second copy: nothing is typed in it. The first: typed in, and written once.
    QTextCursor c(story);
    c.setPosition(second.position() + 3);
    insertText(*doc, c, QStringLiteral("ZZZ"));
    QCOMPARE(story->findBlockByNumber(4).text().mid(1), QStringLiteral(" shared"));
    c.setPosition(first.position() + first.length() - 1);
    insertText(*doc, c, QStringLiteral("!"));
    QCOMPARE(story->findBlockByNumber(3).text().mid(1), QStringLiteral(" shared!"));
    QByteArray part = notesPartOf(doc->toDocx());
    QCOMPARE(int(part.count("<w:endnote w:id=\"2\">")), 1);
    QVERIFY(part.contains("shared!"));
    // The first reference taken out: the note goes on from the second, with the words that were written, and the copy is the first of its note now.
    QTextCursor gone(story);
    const int at = doc->endnoteReferences(2).first();
    gone.setPosition(at);
    gone.setPosition(at + 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, gone);
    QCOMPARE(endnoteMarks(*doc), (QStringList{"2:1"}));
    const QTextBlock now = story->findBlockByNumber(3);
    QCOMPARE(now.text().mid(1), QStringLiteral(" shared!"));
    QVERIFY(now.blockFormat().intProperty(prop::EndnoteNote) > 0);
    c.setPosition(now.position() + now.length() - 1);
    insertText(*doc, c, QStringLiteral("?"));
    QVERIFY(notesPartOf(doc->toDocx()).contains("shared!?"));
    QVERIFY(rt::strictProblems(doc->toDocx(), zip).isEmpty());
}


// An endnote of thousands of paragraphs and a table: all of it is set (as many as the pages set), an edit of one paragraph writes every one, and the table stays where it was.
void WriteTests::aHugeEndnoteKeepsEveryParagraphWhenItIsEdited()
{
    constexpr int paragraphsInNote = 1500;
    QByteArray inner = "<w:p><w:r><w:endnoteRef/></w:r><w:r><w:t>first</w:t></w:r></w:p>";
    for (int i = 0; i < paragraphsInNote; ++i) inner += "<w:p><w:r><w:t>p" + QByteArray::number(i) + "</w:t></w:r></w:p>";
    inner += "<w:tbl><w:tblPr><w:tblW w:w=\"4000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>cellA</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>cellB</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>last</w:t></w:r></w:p>";
    const QByteArray notes = "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote() + "<w:endnote w:id=\"2\">" + inner + "</w:endnote></w:endnotes>";
    const QByteArray zip = makeDocxWithParts("<w:p><w:r><w:t>A paragraph</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>", sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), notes}});
    auto doc = open(zip);
    QVERIFY(doc);
    int from = 0, to = 0;
    QVERIFY(doc->endnoteWords(2, &from, &to));
    QTextCursor c(doc->story());
    c.setPosition(from);
    insertText(*doc, c, QStringLiteral(">"));
    const QByteArray saved = doc->toDocx();
    const QByteArray part = notesPartOf(saved);
    QCOMPARE(int(part.count("<w:p>")) + int(part.count("<w:p ")), paragraphsInNote + 5);
    QVERIFY2(part.contains("&gt;") && part.indexOf("&gt;") < part.indexOf("first"), "the typed character");
    QVERIFY2(part.contains("p1499"), "the last of the paragraphs");
    QVERIFY2(part.contains("cellA") && part.contains("cellB"), "the table's cells");
    QVERIFY2(part.contains("last"), "the paragraph after the table");
    QVERIFY(part.indexOf("p1499") < part.indexOf("<w:tbl>") && part.indexOf("</w:tbl>") < part.indexOf("last"));
    QVERIFY(rt::strictProblems(saved, zip).isEmpty());
}


// An endnotes part that is not XML is left as it is: no endnote is made over it; and a reference to a note in a table's cell, and one made there, work.
void WriteTests::anUnreadableEndnotesPartIsLeftAsItIsAndANoteInATableCellWorks()
{
    const QByteArray junk = "<w:endnotes not xml at all";
    auto doc = open(makeDocxWithParts("<w:p><w:r><w:t>A paragraph</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>", sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), junk}}));
    QVERIFY(doc);
    QTextCursor c(doc->story());
    c.setPosition(3);
    QVERIFY(!insertEndnote(*doc, c));
    QCOMPARE(partOf(doc->toDocx(), QStringLiteral("/word/endnotes.xml")), xml(junk));
    QVERIFY(!doc->canUndo());
    // A table's cell: an endnote is made in it and set at the end of the document.
    const QByteArray cell = "<w:tbl><w:tblPr><w:tblW w:w=\"5000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"5000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t xml:space=\"preserve\">In a cell </w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>after</w:t></w:r></w:p>";
    const QByteArray zip = makeDocxWithParts(cell, sectionXml(6000, 6000, 720), {});
    auto other = open(zip);
    QVERIFY(other);
    QTextCursor in(other->story());
    in.setPosition(5);
    QVERIFY(insertEndnote(*other, in));
    typeText(*other, in, QStringLiteral("cell note"));
    const QByteArray saved = other->toDocx();
    QVERIFY(notesPartOf(saved).contains("cell note"));
    QVERIFY(rt::strictProblems(saved, zip).isEmpty());
    auto back = open(saved);
    QVERIFY(back);
    QCOMPARE(back->citedEndnotes(), (QVector<int>{1}));
}


// ---- endnotes: in the window

// Clicking in an endnote's words puts the caret there (the notes are on the pages after the text); typing is the note's; Up from its first line and Down from its last, and Esc, leave it
// for the text just after its reference; the keys that move inside the note stay in it.
void WriteTests::clickingInAnEndnotePutsTheCaretInItsWordsAndTheKeysLeaveIt()
{
    auto doc = open(notedDocx());
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 900);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    QTextDocument *story = doc->story();
    int from = 0, to = 0;
    QVERIFY(doc->endnoteWords(2, &from, &to));
    // (Where the caret is drawn when it is in the first note, which is where the click goes.)
    QTextCursor in(story);
    in.setPosition(from + 6);
    view.setCursor(in);
    QRect caret;
    QVERIFY(view.caretRect(&caret));
    QTextCursor start(story);
    start.setPosition(0);
    view.setCursor(start);
    QVERIFY(view.cursor().position() == 0);
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, caret.center());
    const EndnoteSpan span = endnoteSpanAt(story, view.cursor().position());
    QVERIFY2(span.valid && span.editable && span.textFrom == from, qPrintable(QString::number(view.cursor().position())));
    QVERIFY(view.cursor().position() >= from && view.cursor().position() <= to);
    QTest::keyClicks(&view, "xyz");
    QVERIFY(story->findBlockByNumber(3).text().contains(QLatin1String("xyz")));
    QCOMPARE(savedBody(*doc), QStringLiteral("First body paragraph|Second body paragraph"));
    // Inside the note the keys stay: Home goes to the start of its words, and Left from there does not leave; End and Right at its end do not go on into the next note.
    QTest::keyClick(&view, Qt::Key_Home);
    QCOMPARE(view.cursor().position(), from);
    QTest::keyClick(&view, Qt::Key_Left);
    QCOMPARE(view.cursor().position(), from);
    QTest::keyClick(&view, Qt::Key_End);
    QTest::keyClick(&view, Qt::Key_Right);
    QTest::keyClick(&view, Qt::Key_Right);
    const EndnoteSpan same = endnoteSpanAt(story, view.cursor().position());
    QVERIFY(same.valid && same.serial == span.serial);
    // Esc leaves for the text, after the note's reference.
    const int after = doc->endnoteReferences(2).first() + 1;
    QTest::keyClick(&view, Qt::Key_Escape);
    QCOMPARE(view.cursor().position(), after);
    // Up from the first line of a note leaves it; Down from the last does too.
    view.setCursor(in);
    QTest::keyClick(&view, Qt::Key_Up);
    QCOMPARE(view.cursor().position(), after);
    QVERIFY(doc->endnoteWords(3, &from, &to));
    QTextCursor last(story);
    last.setPosition(to);
    view.setCursor(last);
    QTest::keyClick(&view, Qt::Key_Down);
    QCOMPARE(view.cursor().position(), doc->endnoteReferences(3).first() + 1);
    // From the text, Ctrl+End still stops where the text does.
    QTest::keyClick(&view, Qt::Key_End, Qt::ControlModifier);
    QCOMPARE(view.cursor().position(), editableEnd(story));
}


// Insert Endnote on the References tab and Ctrl+Alt+D: the note is made and the caret is in it, after the text of the document.
void WriteTests::insertEndnoteIsOnTheReferencesTabAndMovesTheCaretIntoTheNote()
{
    WriteWindow w;
    w.resize(1300, 1000);
    w.show();
    QVERIFY(QTest::qWaitForWindowActive(&w));
    QVERIFY(w.act("refs.endnote"));
    QCOMPARE(w.act("refs.endnote")->shortcut(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_D));
    w.setDocument(open(makeDocxWithParts(paragraphs(4, 40), pageSection(), {}, QByteArray(), kStyles)), QString());
    WriteDoc *doc = w.document();
    w.view()->paginateAll();
    w.view()->cursor().setPosition(doc->story()->findBlockByNumber(1).position() + 5);
    QVERIFY(w.act("refs.endnote")->isEnabled());
    w.act("refs.endnote")->trigger();
    QCOMPARE(doc->citedEndnotes(), (QVector<int>{1}));
    int id = 0;
    QVERIFY(doc->endnoteIdOf(w.view()->cursor().block(), &id));
    QCOMPARE(id, 1);
    QTest::keyClicks(w.view(), "An endnote typed in the window");
    QVERIFY(partOf(doc->toDocx(), QStringLiteral("/word/endnotes.xml")).contains("An endnote typed in the window"));
    // The key combination does it too, from the text; not in a header (the command is gray there) and not in a note (it is not one's to make).
    w.view()->cursor().setPosition(doc->story()->findBlockByNumber(0).position() + 3);
    QTest::keyClick(w.view(), Qt::Key_D, Qt::ControlModifier | Qt::AltModifier);
    QCOMPARE(doc->citedEndnotes(), (QVector<int>{1, 2}));
    QVERIFY(doc->endnoteIdOf(w.view()->cursor().block(), &id) && id == 2);
    w.act("refs.endnote")->trigger();   // (the caret is in a note: nothing is made)
    QCOMPARE(doc->citedEndnotes(), (QVector<int>{1, 2}));
    QVERIFY(w.view()->enterHeaderFooter(0, false, true));
    QTRY_VERIFY(!w.act("refs.endnote")->isEnabled());
}


// The Find and Replace pane and the Spelling pane look through the text: opening them with a footnote open leaves it.
void WriteTests::findAndSpellingWithAFootnoteOpenLeaveIt()
{
    WriteWindow w;
    w.resize(1300, 1000);
    w.show();
    w.setDocument(open(threeNotesDocx()), QString());
    w.view()->paginateAll();
    QVERIFY(w.view()->enterFootnote(3));
    w.showFindPane(false);
    w.findPane()->findBox()->setText(QStringLiteral("zzzz"));
    w.findPane()->findNext();
    QVERIFY(!w.view()->inNote());
    QVERIFY(w.view()->enterFootnote(2));
    w.showSpelling();
    QVERIFY(!w.view()->inNote());
}


// ---- what is written

// The runs of a field keep their own properties (the marks and the code are not the result's): only a look the person gave the field is given to all of its runs.
void WriteTests::theRunsOfAFieldKeepTheirOwnProperties()
{
    const QByteArray field = "<w:r><w:rPr><w:i/></w:rPr><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:rPr><w:i/></w:rPr><w:instrText xml:space=\"preserve\"> FILENAME </w:instrText></w:r>"
                             "<w:r><w:rPr><w:i/></w:rPr><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:rPr><w:b/></w:rPr><w:t>name.docx</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
    auto doc = open(makeDocxWithParts(paragraphs(2, 20), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>x</w:t></w:r>" + field})}}));
    QVERIFY(doc);
    QTextDocument *story = doc->editableHeaderFooter(0, false, 0);
    QVERIFY(story);
    QTextCursor c(story);
    insertText(*doc, c, QStringLiteral(">"));
    const QByteArray out = partOf(doc->toDocx(), QStringLiteral("/word/header1.xml"));
    QVERIFY2(out.contains("<w:r><w:rPr><w:i/></w:rPr><w:fldChar w:fldCharType=\"begin\"/></w:r>"), out.constData());
    QVERIFY2(out.contains("<w:r><w:rPr><w:i/></w:rPr><w:instrText xml:space=\"preserve\"> FILENAME </w:instrText></w:r>"), out.constData());
    QVERIFY2(out.contains("<w:r><w:rPr><w:i/></w:rPr><w:fldChar w:fldCharType=\"separate\"/></w:r>"), out.constData());
    QVERIFY2(out.contains("<w:r><w:rPr><w:b/></w:rPr><w:t xml:space=\"preserve\">name.docx</w:t></w:r>"), out.constData());
    QVERIFY2(out.contains("<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>"), out.constData());
    // A look given to the field is given to the runs of it.
    QTextCursor all(story);
    all.select(QTextCursor::Document);
    toggleUnderline(*doc, all);
    const QByteArray styled = partOf(doc->toDocx(), QStringLiteral("/word/header1.xml"));
    QVERIFY2(styled.count("<w:u w:val=\"single\"/>") >= 6, styled.constData());
}


// After an edit that is all undone, the part is written as the file had it (not made again from the story), and what making a header added to the styles is not written.
void WriteTests::anEditThatIsUndoneLeavesTheFileAsItWas()
{
    // A header part with bytes that a rewrite would change.
    const QByteArray odd = "<w:hdr " + QByteArray(kNs) + " xmlns:x=\"urn:x\"><!-- kept --><w:p  w:rsidR=\"00A1\"   ><w:pPr><x:odd a=\"1\"/></w:pPr><w:r ><w:t>ODD  bytes</w:t></w:r></w:p>\n</w:hdr>";
    const QByteArray zip = makeDocxWithParts(paragraphs(3, 20), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), odd}});
    auto doc = open(zip);
    QVERIFY(doc);
    typeAtEnd(*doc, doc->editableHeaderFooter(0, false, 0), QStringLiteral("abc"));
    QVERIFY(doc->isModified());
    QVERIFY(partOf(doc->toDocx(), QStringLiteral("/word/header1.xml")) != xml(odd));
    while (doc->canUndo()) doc->undo();
    QVERIFY(!doc->isModified());
    QCOMPARE(partOf(doc->toDocx(), QStringLiteral("/word/header1.xml")), xml(odd));
    // Again: edited, and written again from the story.
    while (doc->canRedo()) doc->redo();
    QVERIFY(partOf(doc->toDocx(), QStringLiteral("/word/header1.xml")).contains("ODD  bytesabc"));
    // A header made, typed in, and all of it undone: no part, and the styles as they were.
    const QByteArray plainZip = makeDocxWithParts(paragraphs(3, 20), QByteArray(), {});
    auto made = open(plainZip);
    QVERIFY(made);
    QVERIFY(made->createHeaderFooter(0, false, 0));
    typeAtEnd(*made, made->headerFooterStory(0, false, 0), QStringLiteral("Top"));
    QVERIFY(partOf(made->toDocx(), QStringLiteral("/word/styles.xml")).contains("w:styleId=\"Header\""));
    while (made->canUndo()) made->undo();
    const QByteArray saved = made->toDocx();
    office::Package pkg;
    QVERIFY(pkg.read(saved));
    for (const QString &n : pkg.partNames()) QVERIFY2(!n.startsWith(QLatin1String("/word/header")), qPrintable(n));
    QCOMPARE(pkg.part(QStringLiteral("/word/styles.xml")), partOf(plainZip, QStringLiteral("/word/styles.xml")));
    QCOMPARE(documentXml(saved), documentXml(open(plainZip)->toDocx()));   // (as the file saves with nothing done to it)
}


// A copy that Link to Previous off makes has the ids of its own: its bookmarks, drawings, and comment ranges are not the ones of the part it was copied from.
void WriteTests::aCopyMadeByUnlinkingHasIdsOfItsOwn()
{
    const QByteArray inner = "<w:p><w:bookmarkStart w:id=\"4\" w:name=\"hdrmark\"/><w:r><w:t>text</w:t></w:r><w:bookmarkEnd w:id=\"4\"/><w:commentRangeStart w:id=\"3\"/><w:r><w:t>c</w:t></w:r><w:commentRangeEnd w:id=\"3\"/>"
                             "<w:r><w:drawing><wp:inline><wp:extent cx=\"100\" cy=\"100\"/><wp:docPr id=\"7\" name=\"Logo\"/></wp:inline></w:drawing></w:r></w:p>";
    const QByteArray header = "<w:hdr " + QByteArray(kNs) + " xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\">" + inner + "</w:hdr>";
    const QByteArray body = "<w:p><w:pPr>" + sectionNaming({{"default", "rIdheader1"}}) + "</w:pPr><w:r><w:t>one</w:t></w:r></w:p><w:p><w:r><w:t>two</w:t></w:r></w:p>";
    auto doc = open(makeDocxWithParts(body, sectionNaming({}), {{QStringLiteral("/word/header1.xml"), header}}));
    QVERIFY(doc);
    QVERIFY(doc->setLinkedToPrevious(1, false, 0, false));
    const QByteArray saved = doc->toDocx();
    office::Package pkg;
    QVERIFY(pkg.read(saved));
    const QByteArray first = pkg.part(QStringLiteral("/word/header1.xml")), second = pkg.part(QStringLiteral("/word/header2.xml"));
    QVERIFY(!second.isEmpty());
    auto idsOf = [](const QByteArray &xmlPart, const char *element, const char *attribute) {
        QSet<QString> ids;
        const QRegularExpression re(QStringLiteral("<%1\\b[^>]*\\b%2=\"([^\"]*)\"").arg(QLatin1String(element), QLatin1String(attribute)));
        for (auto m = re.globalMatch(QString::fromUtf8(xmlPart)); m.hasNext();) ids.insert(m.next().captured(1));
        return ids;
    };
    for (const auto &what : {std::make_pair("w:bookmarkStart", "w:id"), std::make_pair("w:bookmarkEnd", "w:id"), std::make_pair("w:bookmarkStart", "w:name"), std::make_pair("wp:docPr", "id"),
                             std::make_pair("w:commentRangeStart", "w:id"), std::make_pair("w:commentRangeEnd", "w:id")}) {
        const QSet<QString> a = idsOf(first, what.first, what.second), b = idsOf(second, what.first, what.second);
        QVERIFY2(!a.isEmpty() && !b.isEmpty(), what.first);
        QVERIFY2(!a.intersects(b), qPrintable(QStringLiteral("%1 %2 is the same in both parts: %3").arg(QLatin1String(what.first), QLatin1String(what.second), QStringList(QList<QString>(a.begin(), a.end())).join(QLatin1Char(',')))));
    }
    // The pairs are still pairs.
    QCOMPARE(idsOf(second, "w:bookmarkStart", "w:id"), idsOf(second, "w:bookmarkEnd", "w:id"));
    QCOMPARE(idsOf(second, "w:commentRangeStart", "w:id"), idsOf(second, "w:commentRangeEnd", "w:id"));
}


// A part that no section names any more takes the pictures only it used with it; a picture another part uses stays.
void WriteTests::aDroppedPartTakesItsOwnPicturesWithIt()
{
    QImage image(4, 4, QImage::Format_RGB32);
    image.fill(Qt::blue);
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    auto drawing = [](const char *id) { return QByteArray("<w:r><w:pict><v:shape id=\"s\" style=\"width:20pt;height:20pt\"><v:imagedata r:id=\"") + id + "\"/></v:shape></w:pict></w:r>"; };
    const QByteArray vml = " xmlns:v=\"urn:schemas-microsoft-com:vml\"";
    office::PackageWriter w;
    const QString rel = QString::fromLatin1(kRelNs);
    const QByteArray body = "<w:p><w:pPr>" + sectionNaming({{"default", "rIdheader1"}}) + "</w:pPr><w:r><w:t>one</w:t></w:r></w:p><w:p><w:r><w:t>two</w:t></w:r></w:p>";
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body + sectionNaming({{"default", "rIdheader2"}}) + "</w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addPart(QStringLiteral("/word/header1.xml"), xml("<w:hdr " + QByteArray(kNs) + vml + "><w:p>" + drawing("rId9") + "</w:p></w:hdr>"), QString::fromLatin1(kHeaderType));
    w.addPart(QStringLiteral("/word/header2.xml"), xml("<w:hdr " + QByteArray(kNs) + vml + "><w:p><w:r><w:t>own</w:t></w:r>" + drawing("rId8") + drawing("rId9") + "</w:p></w:hdr>"), QString::fromLatin1(kHeaderType));
    w.addPart(QStringLiteral("/word/media/shared.png"), png, QStringLiteral("image/png"));
    w.addPart(QStringLiteral("/word/media/only2.png"), png + "2", QStringLiteral("image/png"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdheader1"), rel + QStringLiteral("header"), QStringLiteral("/word/header1.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdheader2"), rel + QStringLiteral("header"), QStringLiteral("/word/header2.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/header1.xml"), QStringLiteral("rId9"), rel + QStringLiteral("image"), QStringLiteral("/word/media/shared.png"));
    w.addRelationshipWithId(QStringLiteral("/word/header2.xml"), QStringLiteral("rId8"), rel + QStringLiteral("image"), QStringLiteral("/word/media/only2.png"));
    w.addRelationshipWithId(QStringLiteral("/word/header2.xml"), QStringLiteral("rId9"), rel + QStringLiteral("image"), QStringLiteral("/word/media/shared.png"));
    auto doc = open(w.finish());
    QVERIFY(doc);
    QCOMPARE(doc->sectionCount(), 2);
    QVERIFY(doc->hasOwnHeaderFooter(1, false, 0));
    QVERIFY(doc->setLinkedToPrevious(1, false, 0, true));
    const QByteArray saved = doc->toDocx();
    office::Package pkg;
    QVERIFY(pkg.read(saved));
    QVERIFY(!pkg.hasPart(QStringLiteral("/word/header2.xml")));
    QVERIFY2(!pkg.hasPart(QStringLiteral("/word/media/only2.png")), "the picture only the dropped part used is still in the file");
    QVERIFY(pkg.hasPart(QStringLiteral("/word/media/shared.png")));
    QVERIFY(pkg.relationships(QStringLiteral("/word/header2.xml")).isEmpty());
}


// A settings part that is one empty element takes the setting.
void WriteTests::aSelfClosingSettingsPartTakesTheOddEvenSetting()
{
    const QByteArray settings = "<w:settings xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"/>";
    const QByteArray on = wml::settingsWithEvenAndOddHeaders(xml(settings), true);
    QVERIFY2(on.contains("<w:evenAndOddHeaders/>") && on.endsWith("</w:settings>"), on.constData());
    QXmlStreamReader reader(on);
    while (!reader.atEnd()) reader.readNext();
    QVERIFY2(!reader.hasError(), qPrintable(reader.errorString()));
    QCOMPARE(wml::settingsWithEvenAndOddHeaders(xml(settings), false), xml(settings));
    // In a file: the toggle is kept when it is saved.
    office::PackageWriter w;
    const QString rel = QString::fromLatin1(kRelNs);
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + paragraphs(2, 10) + "</w:body></w:document>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addPart(QStringLiteral("/word/settings.xml"), xml(settings), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdset"), rel + QStringLiteral("settings"), QStringLiteral("/word/settings.xml"));
    auto doc = open(w.finish());
    QVERIFY(doc);
    doc->setDifferentOddEven(true);
    const QByteArray saved = doc->toDocx();
    QVERIFY2(partOf(saved, QStringLiteral("/word/settings.xml")).contains("evenAndOddHeaders"), partOf(saved, QStringLiteral("/word/settings.xml")).constData());
    auto again = open(saved);
    QVERIFY(again);
    QVERIFY(again->differentOddEven());
}


// ---- entering a header

// A double click in the margin of a page whose section has no header shows an empty header to edit, and makes nothing in the document until something is typed there: leaving
// it as it is leaves the document as it was; typing makes the part, and an undo takes the typing and then the header away.
void WriteTests::aDoubleClickInAMarginWithNoHeaderMakesNothingUntilSomethingIsTyped()
{
    const QByteArray zip = makeDocxWithParts(paragraphs(3, 20), QByteArray(), {});
    auto doc = open(zip);
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 1200);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    auto at = [&](double x, double y) {
        const QRect r = view.pageRect(0);
        return QPoint(r.left() + int(x * 96 / 72 * view.zoom()), r.top() + int(y * 96 / 72 * view.zoom()));
    };
    const QByteArray untouched = documentXml(open(zip)->toDocx());   // (what the file saves as with nothing done to it)
    auto fileIsAsItWas = [&] {
        const QByteArray saved = doc->toDocx();
        office::Package pkg;
        if (!pkg.read(saved)) return false;
        for (const QString &n : pkg.partNames())
            if (n.startsWith(QLatin1String("/word/header")) || n.startsWith(QLatin1String("/word/footer"))) return false;
        return pkg.part(QStringLiteral("/word/styles.xml")) == partOf(zip, QStringLiteral("/word/styles.xml")) && documentXml(saved) == untouched;
    };
    QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, at(300, 20));
    QVERIFY(view.inHeaderFooter());
    QVERIFY(doc->headerFooterStory(0, false, 0));   // (an empty one is there to type in)
    QCOMPARE(view.cursor().document(), doc->headerFooterStory(0, false, 0));
    QVERIFY2(!doc->isModified(), "entering made the document modified");
    QVERIFY(!doc->canUndo());
    QVERIFY(!doc->hasOwnHeaderFooter(0, false, 0));
    QVERIFY(fileIsAsItWas());
    QRect caret;
    QVERIFY(view.caretRect(&caret));
    QTest::keyClick(&view, Qt::Key_Escape);
    QVERIFY(!view.inHeaderFooter());
    QVERIFY2(!doc->headerFooterStory(0, false, 0), "an empty header that nobody typed in is still there");
    QVERIFY(!doc->isModified());
    QVERIFY(!doc->canUndo());
    QVERIFY(fileIsAsItWas());
    // Entered again, and typed in.
    QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, at(300, 20));
    QVERIFY(view.inHeaderFooter());
    QTest::keyClicks(&view, QStringLiteral("Top"));
    QVERIFY(doc->isModified());
    QVERIFY(doc->hasOwnHeaderFooter(0, false, 0));
    QCOMPARE(doc->headerFooterStory(0, false, 0)->toPlainText(), QStringLiteral("Top"));
    const QByteArray saved = doc->toDocx();
    QVERIFY(partOf(saved, QStringLiteral("/word/header1.xml")).contains(">Top<"));
    QVERIFY(partOf(saved, QStringLiteral("/word/styles.xml")).contains("w:styleId=\"Header\""));
    view.undo();   // the typing
    QCOMPARE(doc->headerFooterStory(0, false, 0)->toPlainText(), QString());
    QVERIFY(doc->hasOwnHeaderFooter(0, false, 0));
    view.undo();   // the header
    QVERIFY(!doc->hasOwnHeaderFooter(0, false, 0));
    QVERIFY(!doc->isModified());
    QTRY_VERIFY(!view.inHeaderFooter());
    QVERIFY(fileIsAsItWas());
    // Insert > Header (the command) makes it at once.
    QVERIFY(view.enterHeaderFooterAtCaret(false));
    QVERIFY(doc->hasOwnHeaderFooter(0, false, 0));
    QVERIFY(doc->isModified());
}


// The blank page that a section starting on an odd page leaves has no header or footer, so there is nothing to edit on it: a double click in its margin does not enter one.
void WriteTests::aBlankPageOfAnOddPageSectionIsNotEntered()
{
    const QByteArray one = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>" + QByteArray(kPage) + "</w:sectPr>";
    const QByteArray two = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:type w:val=\"oddPage\"/>" + QByteArray(kPage) + "</w:sectPr>";
    const QByteArray body = "<w:p><w:pPr>" + one + "</w:pPr><w:r><w:t>one</w:t></w:r></w:p><w:p><w:r><w:t>two</w:t></w:r></w:p>";
    auto doc = open(makeDocxWithParts(body, two, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>HEAD</w:t></w:r>"})}}));
    QVERIFY(doc);
    QCOMPARE(doc->pager().pageCount(), 3);
    QVERIFY(doc->pager().page(1).start.blank);
    PageView view;
    view.resize(1000, 1200);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    QVERIFY(!view.enterHeaderFooter(1, false));
    QVERIFY(!view.inHeaderFooter());
    const QRect r = view.pageRect(1);
    QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(r.left() + 300, r.top() + 26));
    QVERIFY(!view.inHeaderFooter());
    // The pages with text are entered as before.
    QVERIFY(view.enterHeaderFooter(0, false));
    QVERIFY(view.enterHeaderFooter(2, true));
    QRect caret;
    QTRY_VERIFY(view.caretRect(&caret));
}


// The file's font table says what a font it names is set in where the machine lacks it: its alternate (when the machine has that), a symbol font for a symbol
// character set, otherwise by its PANOSE class (a sans face in Arial, a serif one in Times New Roman) or its kind, a serif face or a fixed-width one.
void WriteTests::theFontTableGivesTheStandIns()
{
    const QByteArray table = "<w:fonts " + QByteArray(kNs) + "><w:font w:name=\"Palatino\"><w:altName w:val=\"Book Antiqua\"/><w:family w:val=\"roman\"/></w:font>"
                             "<w:font w:name=\"Old Serif\"><w:family w:val=\"roman\"/></w:font><w:font w:name=\"Typewriter\"><w:family w:val=\"modern\"/></w:font>"
                             "<w:font w:name=\"Sans\"><w:family w:val=\"swiss\"/></w:font>"
                             "<w:font w:name=\"Dingbats\"><w:charset w:val=\"02\"/><w:family w:val=\"decorative\"/></w:font>"
                             "<w:font w:name=\"Unicode Sans\"><w:panose1 w:val=\"020B0604020202020204\"/><w:family w:val=\"roman\"/></w:font>"
                             "<w:font w:name=\"Ghost\"><w:altName w:val=\"No Such Alternate\"/><w:family w:val=\"roman\"/></w:font>"
                             "<w:font w:name=\"Fixed\"><w:family w:val=\"auto\"/><w:pitch w:val=\"fixed\"/></w:font>"
                             // (A fixed-pitch font is set in Courier New whatever its PANOSE says of its strokes or its kind says: a sans one with fixed pitch, and a
                             // serif one that Courier New's own PANOSE, whose proportion digit 09 is monospaced, describes; and a fixed one of kind swiss.)
                             "<w:font w:name=\"Mono Sans\"><w:panose1 w:val=\"020B0509000000000004\"/><w:family w:val=\"swiss\"/><w:pitch w:val=\"fixed\"/></w:font>"
                             "<w:font w:name=\"Typewriter Two\"><w:panose1 w:val=\"02070309020205020404\"/><w:family w:val=\"roman\"/><w:pitch w:val=\"variable\"/></w:font>"
                             "<w:font w:name=\"Fixed Swiss\"><w:family w:val=\"swiss\"/><w:pitch w:val=\"fixed\"/></w:font>"
                             // (A PANOSE that is short, or not hexadecimal, says nothing: the kind decides.)
                             "<w:font w:name=\"Short Panose\"><w:panose1 w:val=\"020B\"/><w:family w:val=\"roman\"/></w:font>"
                             "<w:font w:name=\"Odd Panose\"><w:panose1 w:val=\"02ZZ0604020202020204\"/><w:family w:val=\"swiss\"/></w:font></w:fonts>";
    auto doc = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {{QStringLiteral("/word/fontTable.xml"), table}}));
    QVERIFY(doc);
    const auto &standIns = doc->flow().env.standIns;
    QCOMPARE(standIns.value(QStringLiteral("palatino")), QStringLiteral("Book Antiqua"));
    QCOMPARE(standIns.value(QStringLiteral("old serif")), QStringLiteral("Times New Roman"));
    QCOMPARE(standIns.value(QStringLiteral("typewriter")), QStringLiteral("Courier New"));
    QVERIFY(!standIns.contains(QStringLiteral("sans")));
    QCOMPARE(standIns.value(QStringLiteral("dingbats")), QStringLiteral("Wingdings"));
    QCOMPARE(standIns.value(QStringLiteral("unicode sans")), QStringLiteral("Arial"));      // (its PANOSE says sans, whatever its kind says)
    QCOMPARE(standIns.value(QStringLiteral("ghost")), QStringLiteral("Times New Roman"));   // (an alternate the machine lacks is not one)
    QCOMPARE(standIns.value(QStringLiteral("fixed")), QStringLiteral("Courier New"));
    QCOMPARE(standIns.value(QStringLiteral("mono sans")), QStringLiteral("Courier New"));
    QCOMPARE(standIns.value(QStringLiteral("typewriter two")), QStringLiteral("Courier New"));
    QCOMPARE(standIns.value(QStringLiteral("fixed swiss")), QStringLiteral("Courier New"));
    QCOMPARE(standIns.value(QStringLiteral("short panose")), QStringLiteral("Times New Roman"));
    QVERIFY(!standIns.contains(QStringLiteral("odd panose")));
}


// The file's compatibility options that change how lines are set reach the layout as fields of their own, the flow document's and each page's (not in the table
// of stand-ins): w:suppressTopSpacingWP gives lines of whole points, w:noLeading drops the leading. The practice documents that carry w:suppressTopSpacingWP are
// all of the 2003 programs' compatibility mode (11); a file in the 2010 programs' mode (14) or the 2013 programs' (15) has no whole-point lines.
void WriteTests::compatibilityOptionsOfLineSpacingReachTheLayout()
{
    const QByteArray old = "<w:compat><w:suppressTopSpacingWP/><w:noLeading/></w:compat>";
    auto inMode = [](int mode) {
        return QByteArray("<w:compat><w:suppressTopSpacingWP/><w:noLeading/><w:compatSetting w:name=\"compatibilityMode\" w:uri=\"http://schemas.microsoft.com/office/word\" w:val=\"") +
               QByteArray::number(mode) + "\"/></w:compat>";
    };
    struct Seen { bool wholeFlow, noLeadingFlow, wholePage, noLeadingPage, tableEmpty, wrapFlow, wrapPage; double shrinkFlow, shrinkPage; };
    auto seen = [&](const QByteArray &settings) {
        auto doc = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {}, settings));
        Seen s{};
        if (!doc) return s;
        const auto &env = doc->flow().env;
        s.wholeFlow = env.wordWholePointLines;
        s.noLeadingFlow = env.wordNoLeading;
        s.wrapFlow = env.wordWrapTrailSpaces;
        s.tableEmpty = env.standIns.isEmpty();
        s.shrinkFlow = env.wordSpaceShrink;
        if (const auto page = doc->pager().materialize(0)) {
            const LayoutEnv pageEnv = Renderer::layoutEnv(*page->doc);
            s.wholePage = pageEnv.wordWholePointLines;
            s.noLeadingPage = pageEnv.wordNoLeading;
            s.wrapPage = pageEnv.wordWrapTrailSpaces;
            s.shrinkPage = pageEnv.wordSpaceShrink;
        }
        return s;
    };
    const Seen a = seen(old);
    QVERIFY(a.wholeFlow && a.noLeadingFlow && a.wholePage && a.noLeadingPage);
    QVERIFY2(a.tableEmpty, "the options are not entries of the table of stand-ins");
    QVERIFY(seen(inMode(11)).wholeFlow && seen(inMode(12)).wholeFlow && seen(inMode(12)).wholePage);   // (the 2003 and the 2007 programs')
    const Seen d = seen(inMode(14));
    QVERIFY2(!d.wholeFlow && !d.wholePage, "a file of mode 14 has no whole-point lines");
    const Seen b = seen(inMode(15));
    QVERIFY2(!b.wholeFlow && !b.wholePage, "a file of mode 15 has no whole-point lines");
    QVERIFY(b.noLeadingFlow && b.noLeadingPage);
    const Seen c = seen("<w:compat/>");
    QVERIFY(!c.wholeFlow && !c.noLeadingFlow && !c.wholePage && !c.noLeadingPage && c.tableEmpty);
    // w:wpJustification (the narrowing of the spaces of a justified line) is the same in both layouts: the share the flow's environment has is the one a page is drawn with,
    // and a file without the option has none in either.
    const Seen j = seen("<w:compat><w:wpJustification/></w:compat>");
    QVERIFY2(j.shrinkFlow > 0 && j.shrinkPage == j.shrinkFlow, qPrintable(QString::number(j.shrinkFlow) + " and " + QString::number(j.shrinkPage)));
    QVERIFY(c.shrinkFlow == 0 && c.shrinkPage == 0 && a.shrinkFlow == 0 && a.shrinkPage == 0);
    // w:wrapTrailSpaces reaches both layouts too.
    const Seen w = seen("<w:compat><w:wrapTrailSpaces/></w:compat>");
    QVERIFY(w.wrapFlow && w.wrapPage && !c.wrapFlow && !c.wrapPage && !a.wrapFlow);
    Document made;
    made.wordSpaceShrink = 0.25;
    QCOMPARE(Renderer::layoutEnv(made).wordSpaceShrink, 0.25);
}


// A font table of thousands of fonts that say nothing is read in step with its size.
void WriteTests::aLongFontTableCostsInStepWithItsSize()
{
    auto steps = [&](int count) {
        QByteArray table = "<w:fonts " + QByteArray(kNs) + ">";
        for (int i = 0; i < count; ++i) table += "<w:font w:name=\"F" + QByteArray::number(i) + "\"/>";
        table += "</w:fonts>";
        const QByteArray zip = makeDocxWithParts(paragraphs(1, 5), QByteArray(), {{QStringLiteral("/word/fontTable.xml"), table}});
        const qint64 before = office::wml::treeStepsForTest();
        auto doc = open(zip);
        return doc ? office::wml::treeStepsForTest() - before : qint64(-1);
    };
    const qint64 small = steps(5000), large = steps(20000);
    // (Four times the fonts: about four times the steps along the table; the square would be sixteen.)
    QVERIFY2(small > 0 && large < 8 * small, qPrintable(QStringLiteral("5000 fonts %1 steps, 20000 fonts %2").arg(small).arg(large)));
}


// A file whose compatibility options say to suppress the space before after a page break has none at the top of the page that a break at the start of a paragraph opens.
void WriteTests::theCompatibilityOptionThatSuppressesTheSpaceBeforeAfterABreakIsKept()
{
    auto top = [&](const QByteArray &settings) {
        const QByteArray body = "<w:p><w:pPr><w:spacing w:before=\"400\" w:after=\"0\"/></w:pPr><w:r><w:br w:type=\"page\"/></w:r><w:r><w:t>Second page</w:t></w:r></w:p>";
        auto doc = open(makeDocxWithParts(body, QByteArray(), {}, settings));
        if (!doc || doc->pager().pageCount() != 2) return -1.0;
        return doc->pager().lines(1)[0].rect.top();
    };
    QVERIFY2(std::abs(top("<w:zoom w:percent=\"100\"/>") - 20) < 0.01, qPrintable(QString::number(top("<w:zoom w:percent=\"100\"/>"))));
    const double suppressed = top("<w:compat><w:suppressSpBfAfterPgBrk/></w:compat>");
    QVERIFY2(std::abs(suppressed) < 0.01, qPrintable(QString::number(suppressed)));
}


// The notes' numbers restart in each section when the settings say so of the footnotes (not of the endnotes).
void WriteTests::footnoteNumbersRestartWhenTheSettingsSaySo()
{
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>Note.</w:t></w:r></w:p></w:footnote></w:footnotes>";
    auto restarts = [&](const QByteArray &settings) {
        auto doc = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {{QStringLiteral("/word/footnotes.xml"), notes}}, settings));
        return doc && doc->flow().restartNotesInSections;
    };
    QVERIFY(restarts("<w:footnotePr><w:numRestart w:val=\"eachSect\"/></w:footnotePr>"));
    QVERIFY(!restarts("<w:footnotePr><w:numRestart w:val=\"continuous\"/></w:footnotePr>"));
    QVERIFY(!restarts("<w:footnotePr><w:numFmt w:val=\"decimal\"/></w:footnotePr><w:endnotePr><w:numRestart w:val=\"eachSect\"/></w:endnotePr>"));
    QVERIFY(!restarts(QByteArray()));
}


// A section's page number format, as the file names it and as the pages count with it, each way: dashes too.
void WriteTests::pageNumberFormatsMapBothWays()
{
    const QPair<QString, QString> pairs[] = {{"lowerRoman", "roman"}, {"upperRoman", "ROMAN"}, {"lowerLetter", "alpha"}, {"upperLetter", "ALPHA"}, {"numberInDash", "dash"}, {"decimal", ""}};
    for (const auto &pair : pairs) {
        office::wml::SectionProps props;
        props.pageFormat = pair.first;
        flow::Section section = flowSectionOf(props, false);
        QCOMPARE(section.pageNumberFormat, pair.second);
        // (Back: a section changed to the format has the file's name for it.)
        office::wml::SectionProps blank;
        section.pageNumberFormat = pair.second;
        flow::Section plain = flowSectionOf(blank, false);
        if (pair.second != plain.pageNumberFormat) {
            updateSectionProps(&blank, section);
            QCOMPARE(blank.pageFormat, pair.first == QLatin1String("decimal") ? QString() : pair.first);
        }
    }
    // The pages show it: the page's own number is in dashes.
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgNumType w:fmt=\"numberInDash\" w:start=\"7\"/></w:sectPr>";
    auto doc = open(makeDocxWithParts(paragraphs(1, 5), sect, {}));
    QVERIFY(doc);
    QCOMPARE(doc->flow().sections[0].pageNumberFormat, QStringLiteral("dash"));
    const auto page = doc->pager().materialize(0);
    QVERIFY(page);
    FieldContext ctx;
    ctx.doc = page->doc.get();
    ctx.pageNumber = 1;
    QCOMPARE(ctx.resolve(QStringLiteral("page")), QStringLiteral("- 7 -"));
    QVERIFY(documentXml(doc->toDocx()).contains("numberInDash"));
}


// Punctuation hangs past the margin only in a file that has the East Asian layout on, and not where it also says no punctuation hangs; each option
// is read by its value: stated off, it is off.
void WriteTests::punctuationHangsOnlyWithTheEastAsianLayout()
{
    auto hangs = [&](const QByteArray &option) {
        auto doc = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {}, "<w:compat>" + option + "</w:compat>"));
        return doc && !doc->flow().env.wordNoHang;
    };
    QVERIFY(!hangs(""));
    QVERIFY(!hangs("<w:doNotWrapTextWithPunct/>"));
    QVERIFY(!hangs("<w:useFELayout w:val=\"0\"/>"));
    QVERIFY(hangs("<w:useFELayout/>"));
    QVERIFY(hangs("<w:useFELayout w:val=\"1\"/>"));
    QVERIFY(!hangs("<w:useFELayout/><w:doNotWrapTextWithPunct/>"));
    QVERIFY(!hangs("<w:useFELayout/><w:doNotWrapTextWithPunct w:val=\"1\"/>"));
    QVERIFY(hangs("<w:useFELayout/><w:doNotWrapTextWithPunct w:val=\"0\"/>"));
    QVERIFY(hangs("<w:useFELayout/><w:doNotWrapTextWithPunct w:val=\"false\"/>"));
}


// The mark after a page break goes to the next page in a file that says so (splitPgBreakAndParaMark) and in the files of the recent programs (compatibility
// mode 15); an older file's mark stays where the break is.
void WriteTests::theMarkAfterABreakFollowsTheFilesOption()
{
    auto follows = [&](const QByteArray &compat) {
        auto doc = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {}, "<w:compat>" + compat + "</w:compat>"));
        return doc && doc->flow().markFollowsBreak;
    };
    QVERIFY(!follows(""));
    QVERIFY(follows("<w:splitPgBreakAndParaMark/>"));
    QVERIFY(!follows("<w:splitPgBreakAndParaMark w:val=\"0\"/>"));
    QVERIFY(!follows("<w:compatSetting w:name=\"compatibilityMode\" w:uri=\"http://schemas.microsoft.com/office/word\" w:val=\"14\"/>"));
    QVERIFY(follows("<w:compatSetting w:name=\"compatibilityMode\" w:uri=\"http://schemas.microsoft.com/office/word\" w:val=\"15\"/>"));
}


// What a tracked change deleted is not in the lines: a paragraph whose mark was deleted and that shows nothing else takes no room, nor do the
// breaks and tabs of deleted runs; one whose mark was deleted but that shows text is one paragraph with the next. The file is saved with the changes as they were.
void WriteTests::whatATrackedChangeDeletedTakesNoRoom()
{
    const QByteArray rev = " w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"";
    auto para = [](const char *text) { return QByteArray("<w:p><w:r><w:t>") + text + "</w:t></w:r></w:p>"; };
    const QByteArray deletedMark = "<w:p><w:pPr><w:rPr><w:del" + rev + "/></w:rPr></w:pPr></w:p>";
    const QByteArray deletedBreak = "<w:p><w:r><w:t>two</w:t></w:r><w:del" + rev + "><w:r><w:tab/><w:br/><w:delText>x</w:delText></w:r></w:del></w:p>";
    const QByteArray keptText = "<w:p><w:pPr><w:rPr><w:del" + rev + "/></w:rPr></w:pPr><w:r><w:t>text</w:t></w:r></w:p>";
    auto lines = [&](const QByteArray &body) {
        auto doc = open(makeDocx(body));
        return doc ? doc->pager().lines(0) : QVector<StoryLayout::LineBrief>();
    };
    const auto plain = lines(para("one") + para("two") + para("three"));
    QCOMPARE(plain.size(), 3);
    const auto edited = lines(para("one") + deletedMark + deletedBreak + para("three"));
    QCOMPARE(edited.size(), plain.size());
    for (int i = 0; i < plain.size(); ++i) {
        QVERIFY2(std::abs(edited[i].rect.top() - plain[i].rect.top()) < 0.01, qPrintable(QStringLiteral("line %1 is at %2, not %3").arg(i).arg(edited[i].rect.top()).arg(plain[i].rect.top())));
        QVERIFY(std::abs(edited[i].rect.height() - plain[i].rect.height()) < 0.01);
    }
    QCOMPARE(lines(para("one") + keptText + para("three")).size(), 2);   // ("one", and "text" and "three" together)
    // Saved after an edit: the deleted parts are as they were, and nothing is made hidden.
    auto doc = open(makeDocx(para("one") + deletedMark + deletedBreak + para("three")));
    QVERIFY(doc);
    QTextCursor c(doc->story());
    c.setPosition(1);
    insertText(*doc, c, QStringLiteral("x"));
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(!x.contains("vanish"), x.constData());
    QVERIFY(x.contains("<w:del"));
    QVERIFY(x.contains("<w:br/>"));
    // Text typed beside deleted content is not deleted.
    QTextCursor at(doc->story());
    at.setPosition(doc->story()->findBlockByNumber(2).position() + 3);   // (after "two", before what was deleted)
    insertText(*doc, at, QStringLiteral("!"));
    QVERIFY(!at.charFormat().boolProperty(tp::HiddenText));
    QVERIFY(doc->pager().lines(0).size() == 3);
}


// What a file says of hidden text on what a tracked change deleted is what is saved: a w:vanish on a deleted run or on a deleted paragraph mark stays, and
// text that is hidden only because it is deleted is not made hidden in the file (nor, in a style that hides text, shown by a w:vanish of value 0).
void WriteTests::whatAFileSaysOfHiddenTextOnDeletedTextIsSavedAsItWas()
{
    const QByteArray rev = " w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"";
    auto savedAfterTyping = [&](const QByteArray &body, const QByteArray &styles) {
        auto doc = open(makeDocx(body, styles));
        if (!doc) return QByteArray();
        QTextCursor c(doc->story());
        c.setPosition(1);
        insertText(*doc, c, QStringLiteral("x"));
        return documentXml(doc->toDocx());
    };
    // A file that hides the deleted run and the deleted mark itself.
    const QByteArray vanished = "<w:p><w:r><w:t>one</w:t></w:r><w:del" + rev + "><w:r><w:rPr><w:vanish/></w:rPr><w:delText>gone</w:delText></w:r></w:del></w:p>"
                                "<w:p><w:pPr><w:rPr><w:vanish/><w:del" + rev + "/></w:rPr></w:pPr><w:del" + rev + "><w:r><w:delText>mark</w:delText></w:r></w:del></w:p>"
                                "<w:p><w:r><w:t>tail</w:t></w:r></w:p>";
    QByteArray x = savedAfterTyping(vanished, QByteArray());
    QVERIFY2(x.count("<w:vanish/>") == 2, x.constData());
    QVERIFY2(x.contains("gone</w:delText>") && x.contains("mark</w:delText>"), x.constData());
    // A deleted run in a style that hides text, which says nothing of it: nothing is added.
    const QByteArray styles = "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                              "<w:style w:type=\"paragraph\" w:styleId=\"Hid\"><w:name w:val=\"Hid\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:vanish/></w:rPr></w:style>";
    const QByteArray inHidden = "<w:p><w:r><w:t>one</w:t></w:r></w:p><w:p><w:pPr><w:pStyle w:val=\"Hid\"/></w:pPr><w:del" + rev + "><w:r><w:delText>gone</w:delText></w:r></w:del></w:p>";
    x = savedAfterTyping(inHidden, styles);
    QVERIFY2(!x.contains("vanish w:val") && x.count("vanish") == 0 && x.contains("gone</w:delText>"), x.constData());
    // One that says it is not hidden stays so.
    const QByteArray shown = "<w:p><w:r><w:t>one</w:t></w:r></w:p><w:p><w:pPr><w:pStyle w:val=\"Hid\"/></w:pPr><w:del" + rev + "><w:r><w:rPr><w:vanish w:val=\"0\"/></w:rPr><w:delText>gone</w:delText></w:r></w:del></w:p>";
    x = savedAfterTyping(shown, styles);
    QVERIFY2(x.contains("<w:vanish w:val=\"0\"/>"), x.constData());
}


// The round-trip check sees what a save did to hidden text and to deleted text.
void WriteTests::theRoundTripCheckSeesLostHiddenAndDeletedText()
{
    const QByteArray rev = " w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"";
    auto file = [&](const QByteArray &runProps, const QByteArray &markProps, const QByteArray &deleted) {
        return makeDocx("<w:p><w:pPr><w:rPr>" + markProps + "</w:rPr></w:pPr><w:del" + rev + "><w:r><w:rPr>" + runProps + "</w:rPr><w:delText>" + deleted + "</w:delText></w:r></w:del></w:p>");
    };
    const QByteArray base = file("<w:vanish/>", "<w:vanish/>", "gone");
    QVERIFY(rt::lossProblems(base, base).isEmpty());
    QVERIFY(!rt::lossProblems(base, file(QByteArray(), "<w:vanish/>", "gone")).isEmpty());   // (the run's w:vanish lost)
    QVERIFY(!rt::lossProblems(base, file("<w:vanish/>", QByteArray(), "gone")).isEmpty());   // (the mark's)
    QVERIFY(!rt::lossProblems(base, file("<w:vanish w:val=\"0\"/>", "<w:vanish/>", "gone")).isEmpty());   // (changed)
    QVERIFY(!rt::lossProblems(base, file("<w:vanish/>", "<w:vanish/>", "gone!")).isEmpty());   // (the deleted text changed)
}


// Text typed beside what a tracked change deleted is never inside the deletion in the file: typed before it or just inside its start, it goes before it;
// typed after its content, just inside its end, or after it, it goes after it.
void WriteTests::textTypedBesideADeletionIsOutsideIt()
{
    const QByteArray rev = " w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"";
    const QByteArray body = "<w:p><w:r><w:t>one</w:t></w:r><w:del" + rev + "><w:r><w:delText>gone</w:delText></w:r></w:del><w:r><w:t>tail</w:t></w:r></w:p>";
    // (In the story: "one" 0 to 2, the deletion's start 3, its text 4 to 7, its end 8, "tail" from 9.)
    for (int at = 3; at <= 9; ++at) {
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(at);
        insertText(*doc, c, QStringLiteral("ZZ"));
        const QByteArray x = documentXml(doc->toDocx());
        const int open = x.indexOf("<w:del "), close = x.indexOf("</w:del>"), typed = x.indexOf("ZZ");
        QVERIFY2(open >= 0 && close > open && typed >= 0, x.constData());
        QVERIFY2(typed < open || typed > close, qPrintable(QStringLiteral("typed at %1 is inside the deletion: %2").arg(at).arg(QString::fromUtf8(x))));
        QVERIFY2((at <= 4) == (typed < open), qPrintable(QStringLiteral("typed at %1 is on the wrong side: %2").arg(at).arg(QString::fromUtf8(x))));
        QVERIFY(x.contains("gone</w:delText>"));
    }
}


// Editing acts on what shows: Backspace at the start of a paragraph after one a tracked change deleted joins it with the paragraph that shows before, Delete at
// the end of one before such a paragraph does the same, and what is deleted stays, as paragraphs of its own: no paragraph that has text keeps the deleted mark.
void WriteTests::editingJoinsTheParagraphsThatShowAcrossADeletedOne()
{
    const QByteArray rev = " w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"", mark = " w:id=\"9\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"";
    const QByteArray body = "<w:p><w:r><w:t>one</w:t></w:r></w:p><w:p><w:pPr><w:rPr><w:del" + mark + "/></w:rPr></w:pPr><w:del" + rev + "><w:r><w:delText>gone</w:delText></w:r></w:del></w:p>"
                            "<w:p><w:r><w:t>two</w:t></w:r></w:p>";
    // The paragraphs of the saved file that have text, and whether the deleted one is still there.
    auto check = [&](WriteDoc &doc, const QString &what) {
        const QByteArray x = documentXml(doc.toDocx());
        QVERIFY2(x.contains("gone</w:delText>") && x.contains("w:id=\"9\""), qPrintable(what + QLatin1String(": the deletion is gone: ") + QString::fromUtf8(x)));
        QVector<QByteArray> withText;
        for (int from = x.indexOf("<w:p>"); from >= 0; from = x.indexOf("<w:p>", from + 1)) {
            const QByteArray para = x.mid(from, x.indexOf("</w:p>", from) - from);
            if (para.contains("<w:t>") || para.contains("<w:t ")) withText << para;
        }
        QCOMPARE(withText.size(), 1);   // (one, and two, are one paragraph)
        QVERIFY2(!withText[0].contains("w:id=\"9\"") && withText[0].contains("one") && withText[0].contains("two"), qPrintable(what + QLatin1String(": ") + QString::fromUtf8(withText[0])));
        QCOMPARE(doc.pager().lines(0).size(), 1);
    };
    {
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->findBlockByNumber(2).position());
        deleteBack(*doc, c, false);
        check(*doc, QStringLiteral("Backspace"));
    }
    {
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->findBlockByNumber(0).position() + 3);
        deleteForward(*doc, c, false);
        check(*doc, QStringLiteral("Delete"));
    }
}


// A selection that begins or ends in a paragraph that shows nothing because a tracked change deleted it begins or ends where the paragraphs that show do:
// what survives is never left in the deleted paragraph.
void WriteTests::aSelectionAtADeletedParagraphLeavesNoTextInIt()
{
    const QByteArray rev = " w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"", mark = " w:id=\"9\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"";
    const QByteArray body = "<w:p><w:r><w:t>one</w:t></w:r></w:p><w:p><w:pPr><w:rPr><w:del" + mark + "/></w:rPr></w:pPr><w:del" + rev + "><w:r><w:delText>gone</w:delText></w:r></w:del></w:p>"
                            "<w:p><w:r><w:t>two</w:t></w:r></w:p>";
    auto textParagraphsKeepNoDeletedMark = [&](WriteDoc &doc, const QString &what) {
        const QByteArray x = documentXml(doc.toDocx());
        for (int from = x.indexOf("<w:p>"); from >= 0; from = x.indexOf("<w:p>", from + 1)) {
            const QByteArray para = x.mid(from, x.indexOf("</w:p>", from) - from);
            if ((para.contains("<w:t>") || para.contains("<w:t ")) && para.contains("w:id=\"9\"")) return QStringLiteral("%1: text in the deleted paragraph: %2").arg(what, QString::fromUtf8(para));
        }
        return QString();
    };
    auto run = [&](const char *name, std::function<void(QTextDocument *, QTextCursor &)> select, const QString &expected) {
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        select(doc->story(), c);
        deleteBack(*doc, c, false);
        QVERIFY2(textParagraphsKeepNoDeletedMark(*doc, QString::fromLatin1(name)).isEmpty(), qPrintable(textParagraphsKeepNoDeletedMark(*doc, QString::fromLatin1(name))));
        QString shown;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) shown += b.text().remove(QChar::ObjectReplacementCharacter) + QLatin1Char('|');
        QCOMPARE(shown, expected);
    };
    // From the start of the deleted paragraph to inside the next: the next loses its letters; the deleted one is not what was selected.
    run("from the deleted paragraph", [](QTextDocument *d, QTextCursor &c) {
        c.setPosition(d->findBlockByNumber(1).position());
        c.setPosition(d->findBlockByNumber(2).position() + 1, QTextCursor::KeepAnchor);
    }, QStringLiteral("one|gone|wo|"));
    // From inside the paragraph before to the start of the deleted one: only the letters go.
    run("to the deleted paragraph", [](QTextDocument *d, QTextCursor &c) {
        c.setPosition(d->findBlockByNumber(0).position() + 1);
        c.setPosition(d->findBlockByNumber(1).position(), QTextCursor::KeepAnchor);
    }, QStringLiteral("o|gone|two|"));
}


// A run that asks for ligatures (w14:ligatures) has them in the layout, by the value it states; the file keeps the element as it was.
void WriteTests::aRunThatAsksForLigaturesIsReadSo()
{
    const QByteArray ns = " xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\"";
    auto ligaturesOf = [&](const char *val) {
        const QByteArray body = "<w:p><w:r><w:rPr><w14:ligatures" + ns + " w14:val=\"" + val + "\"/></w:rPr><w:t>office</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body));
        if (!doc) return -2;
        const QTextCharFormat f = doc->story()->begin().begin().fragment().charFormat();
        if (!f.hasProperty(tp::Ligatures)) return -1;
        QTextCursor c(doc->story());
        insertText(*doc, c, QStringLiteral("x"));
        if (!documentXml(doc->toDocx()).contains(QByteArray("w14:val=\"") + val + "\"")) return -3;
        return f.intProperty(tp::Ligatures);
    };
    QCOMPARE(ligaturesOf("none"), 1);
    QCOMPARE(ligaturesOf("standard"), 0);
    QCOMPARE(ligaturesOf("standardContextual"), 0);
    QCOMPARE(ligaturesOf("all"), 2);
    QCOMPARE(ligaturesOf("discretional"), 2);
    auto doc = open(makeDocx("<w:p><w:r><w:t>office</w:t></w:r></w:p>"));
    QVERIFY(doc);
    QVERIFY(!doc->story()->begin().begin().fragment().charFormat().hasProperty(tp::Ligatures));
}


// A paragraph that turns overflowPunct off, itself or by its style, does not let punctuation hang; the file keeps the element as it was.
void WriteTests::overflowPunctOffIsReadFromTheParagraphAndItsStyle()
{
    const QByteArray styles = "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                              "<w:style w:type=\"paragraph\" w:styleId=\"Tight\"><w:name w:val=\"Tight\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:overflowPunct w:val=\"0\"/></w:pPr></w:style>";
    auto noHang = [&](const QByteArray &props, int block) {
        auto doc = open(makeDocx("<w:p><w:pPr>" + props + "</w:pPr><w:r><w:t>one</w:t></w:r></w:p><w:p><w:r><w:t>two</w:t></w:r></w:p>", styles));
        return doc && doc->story()->findBlockByNumber(block).blockFormat().boolProperty(tp::NoHangPunct);
    };
    QVERIFY(noHang("<w:overflowPunct w:val=\"0\"/>", 0));
    QVERIFY(!noHang("<w:overflowPunct/>", 0));
    QVERIFY(!noHang("", 0));
    QVERIFY(noHang("<w:pStyle w:val=\"Tight\"/>", 0));
    QVERIFY(!noHang("<w:pStyle w:val=\"Tight\"/><w:overflowPunct/>", 0));   // (the paragraph says it does over its style)
    QVERIFY(!noHang("<w:overflowPunct w:val=\"0\"/>", 1));               // (the next paragraph is not the one)
    auto doc = open(makeDocx("<w:p><w:pPr><w:overflowPunct w:val=\"0\"/></w:pPr><w:r><w:t>one</w:t></w:r></w:p>", styles));
    QVERIFY(doc);
    QTextCursor c(doc->story());
    insertText(*doc, c, QStringLiteral("x"));
    QVERIFY(documentXml(doc->toDocx()).contains("<w:overflowPunct w:val=\"0\"/>"));
}


// Clearing the formatting of text keeps what a tracked change deleted deleted: its tabs and breaks do not come back into the lines, and the file keeps
// them as they were.
void WriteTests::clearingFormattingKeepsDeletedTextDeleted()
{
    const QByteArray rev = " w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"";
    const QByteArray body = "<w:p><w:r><w:t>one</w:t></w:r></w:p><w:p><w:r><w:t>two</w:t></w:r><w:del" + rev + "><w:r><w:tab/><w:br/><w:delText>x</w:delText></w:r></w:del></w:p>"
                            "<w:p><w:r><w:t>three</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body));
    QVERIFY(doc);
    const auto before = doc->pager().lines(0);
    QCOMPARE(before.size(), 3);
    QTextCursor c(doc->story());
    c.select(QTextCursor::Document);
    clearFormatting(*doc, c);
    const auto after = doc->pager().lines(0);
    QCOMPARE(after.size(), 3);
    for (int i = 0; i < 3; ++i) QVERIFY(std::abs(after[i].rect.top() - before[i].rect.top()) < 0.01);
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(x.contains("<w:del") && x.contains("x</w:delText>") && x.contains("<w:br/>") && !x.contains("vanish"), x.constData());
}


// An endnote that a tracked change deleted is no endnote in the numbering: the one after it shows the number the first would.
void WriteTests::aDeletedEndnoteReferenceTakesNoNumber()
{
    const QByteArray rev = " w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"";
    const QByteArray body = "<w:p><w:r><w:t>a</w:t></w:r><w:del" + rev + "><w:r><w:endnoteReference w:id=\"1\"/></w:r></w:del><w:r><w:t>b</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>";
    auto doc = open(makeDocx(body));
    QVERIFY(doc);
    QStringList shown;
    for (QTextBlock::iterator it = doc->story()->begin().begin(); !it.atEnd(); ++it) {
        const QTextCharFormat cf = it.fragment().charFormat();
        if (cf.hasProperty(prop::Atom) && unpackAtom(storedBytes(cf, prop::Atom)).tag == QLatin1String("endnoteReference")) shown << cf.stringProperty(tp::Field);
    }
    QCOMPARE(shown, QStringList({QStringLiteral("raw:i"), QStringLiteral("raw:i")}));   // (the deleted one is "i" too, as nothing shows it)
}


// The numbers of the endnote references are in the format the settings give (w:endnotePr), lower case roman where they give none, as the other program's are.
void WriteTests::endnoteReferencesShowTheFormatTheSettingsGive()
{
    const QByteArray body = "<w:p><w:r><w:t>a</w:t></w:r><w:r><w:endnoteReference w:id=\"1\"/></w:r><w:r><w:t>b</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>";
    auto shownWith = [&](const QByteArray &settings) {
        auto doc = open(makeDocxWithParts(body, QByteArray(), {}, settings));
        QStringList shown;
        if (!doc) return shown;
        for (QTextBlock::iterator it = doc->story()->begin().begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (cf.hasProperty(prop::Atom) && unpackAtom(storedBytes(cf, prop::Atom)).tag == QLatin1String("endnoteReference")) shown << cf.stringProperty(tp::Field);
        }
        return shown;
    };
    QCOMPARE(shownWith(QByteArray()), QStringList({QStringLiteral("raw:i"), QStringLiteral("raw:ii")}));
    QCOMPARE(shownWith("<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>"), QStringList({QStringLiteral("raw:1"), QStringLiteral("raw:2")}));
    QCOMPARE(shownWith("<w:endnotePr><w:numFmt w:val=\"upperLetter\"/></w:endnotePr>"), QStringList({QStringLiteral("raw:A"), QStringLiteral("raw:B")}));
    // (The footnotes' format is not the endnotes'.)
    QCOMPARE(shownWith("<w:footnotePr><w:numFmt w:val=\"decimal\"/></w:footnotePr>"), QStringList({QStringLiteral("raw:i"), QStringLiteral("raw:ii")}));
}


// The endnotes' marks are numbered as the settings say and as a section says, and from the number the settings give: chicago marks (a star, a dagger, a double dagger, a section
// sign, then two of each), none, or the number the first begins with; a section's own format is for the references in it.
void WriteTests::endnoteMarksFollowTheFormatsTheNumberAndTheSection()
{
    const QByteArray refs = "<w:r><w:endnoteReference w:id=\"1\"/></w:r><w:r><w:t>b</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r><w:r><w:endnoteReference w:id=\"3\"/></w:r>"
                            "<w:r><w:endnoteReference w:id=\"4\"/></w:r><w:r><w:endnoteReference w:id=\"5\"/></w:r>";
    auto shown = [&](const QByteArray &body, const QByteArray &settings) {
        auto doc = open(makeDocxWithParts(body, QByteArray(), {}, settings));
        QStringList out;
        if (!doc) return out;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const QTextCharFormat cf = it.fragment().charFormat();
                if (cf.hasProperty(prop::Atom) && unpackAtom(storedBytes(cf, prop::Atom)).tag == QLatin1String("endnoteReference")) out << cf.stringProperty(tp::Field).mid(4);
            }
        return out;
    };
    const QByteArray body = "<w:p>" + refs + "</w:p>";
    QCOMPARE(shown(body, "<w:endnotePr><w:numFmt w:val=\"chicago\"/></w:endnotePr>"),
             (QStringList{QStringLiteral("*"), QString(QChar(0x2020)), QString(QChar(0x2021)), QString(QChar(0x00A7)), QStringLiteral("**")}));
    QCOMPARE(shown(body, "<w:endnotePr><w:numFmt w:val=\"none\"/></w:endnotePr>"), (QStringList{QString(), QString(), QString(), QString(), QString()}));
    QCOMPARE(shown(body, "<w:endnotePr><w:numFmt w:val=\"decimal\"/><w:numStart w:val=\"7\"/></w:endnotePr>"), (QStringList{QStringLiteral("7"), QStringLiteral("8"), QStringLiteral("9"), QStringLiteral("10"), QStringLiteral("11")}));
    // The first paragraph ends a section that says upper case roman numerals; the settings' decimal numbers are the last section's.
    const QByteArray two = "<w:p><w:pPr><w:sectPr><w:endnotePr><w:numFmt w:val=\"upperRoman\"/></w:endnotePr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr></w:pPr><w:r><w:endnoteReference w:id=\"1\"/></w:r></w:p>"
                           "<w:p><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>";
    QCOMPARE(shown(two, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>"), (QStringList{QStringLiteral("I"), QStringLiteral("2")}));
}


// Reading the settings for the notes' numbering is once for the document (the endnotes' and the footnotes', two reads), not for each story made (the notes' stories, the headers', and the footers' are made as pages
// need them: 3,000 footnotes were 5 seconds of reading the same part again).
void WriteTests::theEndnotesNumberingIsReadOnceForTheDocument()
{
    QByteArray body, notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>";
    for (int i = 2; i < 22; ++i) {
        body += "<w:p><w:r><w:t>Cited</w:t></w:r><w:r><w:footnoteReference w:id=\"" + QByteArray::number(i) + "\"/></w:r></w:p>";
        notes += "<w:footnote w:id=\"" + QByteArray::number(i) + "\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> note</w:t></w:r></w:p></w:footnote>";
    }
    notes += "</w:footnotes>";
    const qint64 before = office::wml::noteNumberingReadsForTest();
    auto doc = open(makeDocxWithParts(body, QByteArray(), {{QStringLiteral("/word/footnotes.xml"), notes}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>"));
    QVERIFY(doc);
    for (int k = 0; k < doc->pager().pageCount(); ++k) QVERIFY(doc->pager().materialize(k));   // (the notes' stories are made as the pages are)
    const qint64 reads = office::wml::noteNumberingReadsForTest() - before;
    QVERIFY2(reads == 2, qPrintable(QStringLiteral("the settings were read %1 times").arg(reads)));
}


// Words are hyphenated where the settings ask (w:autoHyphenation, by its value), for the zone they give (360 twentieths of a point where they give none); a paragraph that says
// suppressAutoHyphens, or whose style does, is not, and the mark is written back as it was read.
void WriteTests::autoHyphenationFollowsTheSettingsAndTheParagraphsMayRefuseIt()
{
    const QByteArray plain = "<w:p><w:r><w:t>Alpha</w:t></w:r></w:p>";
    auto flowOf = [&](const QByteArray &settings) { return open(makeDocxWithParts(plain, QByteArray(), {}, settings)); };
    auto none = flowOf(QByteArray());
    QVERIFY(none && !none->flow().hyphenate);
    auto on = flowOf("<w:autoHyphenation/>");
    QVERIFY(on && on->flow().hyphenate);
    QCOMPARE(on->flow().hyphenZone, 18.0);
    auto zone = flowOf("<w:autoHyphenation w:val=\"true\"/><w:hyphenationZone w:val=\"720\"/>");
    QVERIFY(zone && zone->flow().hyphenate);
    QCOMPARE(zone->flow().hyphenZone, 36.0);
    auto off = flowOf("<w:autoHyphenation w:val=\"0\"/>");
    QVERIFY(off && !off->flow().hyphenate);
    const QByteArray styles = kStyles + "<w:style w:type=\"paragraph\" w:styleId=\"Tight\"><w:name w:val=\"Tight\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:suppressAutoHyphens/></w:pPr></w:style>";
    const QByteArray body = "<w:p><w:r><w:t>none</w:t></w:r></w:p>"
                            "<w:p><w:pPr><w:suppressAutoHyphens/></w:pPr><w:r><w:t>own</w:t></w:r></w:p>"
                            "<w:p><w:pPr><w:pStyle w:val=\"Tight\"/></w:pPr><w:r><w:t>style</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, styles));
    QVERIFY(doc);
    QList<bool> refused;
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) refused << b.blockFormat().boolProperty(tp::NoAutoHyphen);
    QCOMPARE(refused, (QList<bool>{false, true, true}));
    const QByteArray x = documentXml(doc->toDocx());
    QCOMPARE(int(x.count("<w:suppressAutoHyphens/>")), 1);   // (the paragraph's own: the style's is the style's)
}


// With automatic hyphenation on, a word that does not fit the rest of its line breaks at one of its points (the lines of a narrow column take more of each word, and there are fewer
// of them); a paragraph that refuses it (suppressAutoHyphens) is as it is with it off; and a string of letters longer than any word is not looked up, whatever the file makes of it.
void WriteTests::hyphenatedWordsBreakAtTheirPointsAndALongStringIsNotLookedUp()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"3600\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"720\" w:bottom=\"1440\" w:left=\"720\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    QByteArray text;
    for (int i = 0; i < 12; ++i) text += i % 2 ? "interpretation " : "reasonable ";
    auto linesOf = [&](const QByteArray &settings, const QByteArray &props, const QByteArray &words) {
        auto doc = open(makeDocxWithParts("<w:p><w:pPr>" + props + "</w:pPr><w:r><w:t xml:space=\"preserve\">" + words + "</w:t></w:r></w:p>", sect, {}, settings));
        return doc ? int(doc->pager().lines(0).size()) : -1;
    };
    const int off = linesOf(QByteArray(), QByteArray(), text), on = linesOf("<w:autoHyphenation/>", QByteArray(), text);
    QVERIFY2(off > 0 && on > 0 && on < off, qPrintable(QStringLiteral("%1 lines with hyphenation, %2 without").arg(on).arg(off)));
    QCOMPARE(linesOf("<w:autoHyphenation/>", "<w:suppressAutoHyphens/>", text), off);
    QCOMPARE(linesOf("<w:autoHyphenation w:val=\"false\"/>", QByteArray(), text), off);
    // The same words with a string of twenty thousand letters among them have as many letters looked up as without it, and the few more of the word after it.
    auto lookups = [&](const QByteArray &words) {
        const qint64 before = StoryLayout::hyphenationLettersLookedUpForTest();
        linesOf("<w:autoHyphenation/>", QByteArray(), words);
        return StoryLayout::hyphenationLettersLookedUpForTest() - before;
    };
    const qint64 plainLookups = lookups(text), withString = lookups(text + QByteArray(20000, 'q') + " " + text.left(10));
    QVERIFY2(plainLookups > 0, "the words are looked up");
    QVERIFY2(withString <= plainLookups * 2, qPrintable(QStringLiteral("%1 lookups with the string, %2 without").arg(withString).arg(plainLookups)));
}


// A file that says its hanging indents add no tab stop (noTabHangInd) is read so, by its value.
void WriteTests::hangingIndentsAreNoTabStopsWhereTheFileSaysSo()
{
    auto noStop = [&](const QByteArray &option) {
        auto doc = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {}, "<w:compat>" + option + "</w:compat>"));
        return doc && doc->flow().env.wordNoHangStop;
    };
    QVERIFY(!noStop(""));
    QVERIFY(noStop("<w:noTabHangInd/>"));
    QVERIFY(!noStop("<w:noTabHangInd w:val=\"0\"/>"));
}


// A file that does not use the web page's way of spacing paragraphs has the space after one and the space before the next added; it is the larger of the
// two otherwise.
void WriteTests::paragraphSpaceAddsInAFileThatTurnsTheWebWayOff()
{
    auto sums = [&](const QByteArray &option) {
        auto doc = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {}, "<w:compat>" + option + "</w:compat>"));
        return doc && doc->flow().env.wordSpaceSums;
    };
    QVERIFY(!sums(""));
    QVERIFY(sums("<w:doNotUseHTMLParagraphAutoSpacing/>"));
    QVERIFY(sums("<w:doNotUseHTMLParagraphAutoSpacing w:val=\"1\"/>"));
    QVERIFY(!sums("<w:doNotUseHTMLParagraphAutoSpacing w:val=\"0\"/>"));
    // The pages the paginator makes carry it, so that a page's text is spaced as the paginator spaced it.
    auto doc = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {}, "<w:compat><w:doNotUseHTMLParagraphAutoSpacing/></w:compat>"));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    QVERIFY(page);
    QVERIFY(page->doc->wordSpaceSums);
}


// The web page's automatic spacing is 14 points before and after a paragraph, the larger of the two between paragraphs; a file that turns that way off
// (doNotUseHTMLParagraphAutoSpacing) has the spacing the paragraph states (w:before and w:after, 5 points here), and the two add.
void WriteTests::automaticSpacingIsTheStatedSpacingInAFileThatTurnsTheWebWayOff()
{
    auto para = [](const char *text) {
        return QByteArray("<w:p><w:pPr><w:spacing w:before=\"100\" w:beforeAutospacing=\"1\" w:after=\"100\" w:afterAutospacing=\"1\"/></w:pPr><w:r><w:t>") + text + "</w:t></w:r></w:p>";
    };
    auto gap = [&](const QByteArray &option) {
        auto doc = open(makeDocxWithParts(para("one") + para("two") + para("three"), QByteArray(), {}, "<w:compat>" + option + "</w:compat>"));
        if (!doc) return -1.0;
        const auto lines = doc->pager().lines(0);
        if (lines.size() < 3) return -1.0;
        return lines[1].rect.top() - lines[0].rect.bottom();   // (what is between one paragraph's last line and the next's first)
    };
    const double web = gap("");
    const double fixed = gap("<w:doNotUseHTMLParagraphAutoSpacing/>");
    QVERIFY2(std::abs(web - 14.0) < 0.5, qPrintable(QStringLiteral("the web way: %1 points between paragraphs, not 14").arg(web)));
    QVERIFY2(std::abs(fixed - 10.0) < 0.5, qPrintable(QStringLiteral("the fixed way: %1 points between paragraphs (5 after and 5 before), not 10").arg(fixed)));
}


// Where a file that turns the web page's way of spacing off asks for automatic spacing and states none, the automatic spacing is the fixed 5 points (100 twentieths)
// a word processor writes with it.
void WriteTests::automaticSpacingIsFivePointsWhereAFileStatesNone()
{
    auto para = [](const char *text) {
        return QByteArray("<w:p><w:pPr><w:spacing w:beforeAutospacing=\"1\" w:afterAutospacing=\"1\"/></w:pPr><w:r><w:t>") + text + "</w:t></w:r></w:p>";
    };
    // (Styles that give no spacing of their own: none is stated anywhere.)
    const QByteArray styles = "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>";
    auto doc = open(makeDocxWithParts(para("one") + para("two") + para("three"), QByteArray(), {}, "<w:compat><w:doNotUseHTMLParagraphAutoSpacing/></w:compat>", styles));
    QVERIFY(doc);
    const auto lines = doc->pager().lines(0);
    QVERIFY(lines.size() >= 3);
    const double gap = lines[1].rect.top() - lines[0].rect.bottom();
    QVERIFY2(std::abs(gap - 10.0) < 0.5, qPrintable(QStringLiteral("%1 points between paragraphs that ask for automatic spacing and state none, not 10").arg(gap)));
}


// The text of a page that a paragraph of many pages (a break between each) has in its box is that page's own, not the rest of the paragraph's run of one format: the
// work of making each page is its own text's, not the paragraph's after it.
void WriteTests::aPageOfALongParagraphCopiesOnlyItsOwnText()
{
    QByteArray runs;
    for (int i = 0; i < 2600; ++i) runs += "<w:r><w:t>page " + QByteArray::number(i) + "</w:t></w:r><w:r><w:br w:type=\"page\"/></w:r>";
    auto doc = open(makeDocx("<w:p>" + runs + "</w:p>"));
    QVERIFY(doc);
    QVERIFY(doc->pager().pageCount() >= 2600);
    for (int k : {3, 1300, 2500}) {
        const auto page = doc->pager().materialize(k);
        QVERIFY(page);
        const auto story = page->doc->stories.constFind(QStringLiteral("flow-story-%1").arg(k));
        QVERIFY(story != page->doc->stories.constEnd());
        const int chars = story.value()->doc->characterCount();
        QVERIFY2(chars < 100, qPrintable(QStringLiteral("page %1 has %2 characters of the paragraph in its box").arg(k).arg(chars)));
    }
}


// A raised symbol of the symbol font (the mark of a note as an older program wrote it) reaches above the text of its line: the line is taller by as much as the
// symbol font's ascent, two thirds of the size, raised a third, takes it above the text's (about a point for 12-point text); text raised that is of the text's own
// font makes the line no taller.
void WriteTests::aRaisedSymbolMakesItsLineTallerByWhatItReachesAbove()
{
    const QByteArray font = "<w:rFonts w:ascii=\"Times New Roman\" w:hAnsi=\"Times New Roman\"/><w:sz w:val=\"24\"/>";
    auto height = [&](const QByteArray &middle) {
        const QByteArray body = "<w:p><w:r><w:rPr>" + font + "</w:rPr><w:t>coord</w:t></w:r>" + middle + "<w:r><w:rPr>" + font + "</w:rPr><w:t>inating agency</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body));
        if (!doc) return -1.0;
        const auto lines = doc->pager().lines(0);
        return lines.isEmpty() ? -1.0 : lines[0].rect.height();
    };
    const double plain = height(QByteArray());
    const double symbol = height("<w:r><w:rPr>" + font + "<w:vertAlign w:val=\"superscript\"/></w:rPr><w:sym w:font=\"Symbol\" w:char=\"F020\"/></w:r>");
    const double text = height("<w:r><w:rPr>" + font + "<w:vertAlign w:val=\"superscript\"/></w:rPr><w:t>2</w:t></w:r>");
    QVERIFY(plain > 10);
    QVERIFY2(symbol - plain > 0.6 && symbol - plain < 1.3, qPrintable(QStringLiteral("a raised symbol made the line %1 points taller than without it, not about 0.95").arg(symbol - plain)));
    QVERIFY2(std::abs(text - plain) < 0.3, qPrintable(QStringLiteral("a raised digit made the line %1 points taller").arg(text - plain)));
}


// Spacing a person sets on paragraphs that have automatic spacing is the spacing they have: the automatic gap goes, on the page and in the file.
void WriteTests::spacingSetOnAutomaticSpacingReplacesIt()
{
    auto para = [](const char *text) { return QByteArray("<w:p><w:pPr><w:spacing w:beforeAutospacing=\"1\" w:afterAutospacing=\"1\"/></w:pPr><w:r><w:t>") + text + "</w:t></w:r></w:p>"; };
    // (A file whose styles give spacing of their own, so that none set on a paragraph is written.)
    const QByteArray styles = "<w:docDefaults><w:pPrDefault><w:pPr><w:spacing w:before=\"100\" w:after=\"200\"/></w:pPr></w:pPrDefault></w:docDefaults>"
                              "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>";
    auto doc = open(makeDocx(para("one") + para("two") + para("three"), styles));
    QVERIFY(doc);
    auto pitch = [&] {
        const auto lines = doc->pager().lines(0);
        return lines.size() >= 2 ? lines[1].rect.top() - lines[0].rect.top() : -1.0;
    };
    const double automatic = pitch();
    QTextCursor c(doc->story());
    c.select(QTextCursor::Document);
    setSpacing(*doc, c, 0, 0);
    const double none = pitch();
    QVERIFY2(automatic - none > 10, qPrintable(QStringLiteral("automatic %1, set to none %2").arg(automatic).arg(none)));
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(!x.contains("Autospacing"), x.constData());
    QVERIFY2(x.contains("w:before=\"0\"") && x.contains("w:after=\"0\""), x.constData());
    // (Spacing before alone, as the paragraph fields of the ribbon and the dialog set it, takes the automatic spacing before and keeps the one after.)
    auto again = open(makeDocx(para("one") + para("two"), styles));
    QTextCursor c2(again->story());
    c2.select(QTextCursor::Document);
    changeBlockFormat(*again, c2, [](QTextBlockFormat &bf) { setSpaceBefore(bf, 3); });
    const QByteArray y = documentXml(again->toDocx());
    QVERIFY2(!y.contains("beforeAutospacing") && y.contains("afterAutospacing=\"1\""), y.constData());
    QVERIFY2(y.contains("w:before=\"60\""), y.constData());
}


// A header's last paragraph's space after is part of the header: the text starts below it.
void WriteTests::aHeadersSpaceAfterTakesRoomOnThePage()
{
    auto top = [&](int after) {
        const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>"
                                "<w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"720\" w:right=\"1440\" w:bottom=\"720\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        const QByteArray head = "<w:pPr><w:spacing w:after=\"" + QByteArray::number(after) + "\"/></w:pPr><w:r><w:t>HEAD</w:t></w:r>";
        auto doc = open(makeDocxWithParts(paragraphs(3), sect, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {head})}}));
        return doc ? doc->pager().page(0).content.top() : -1.0;
    };
    const double plain = top(0), spaced = top(400);   // (twenty points)
    QVERIFY(plain > 0 && spaced > 0);
    QVERIFY2(std::abs(spaced - plain - 20) < 0.01, qPrintable(QStringLiteral("body top %1 against %2").arg(spaced).arg(plain)));
}


// A paragraph whose mark a tracked change deleted is joined to the next: an empty paragraph after one that shows text is the rest of it and takes no room,
// but after one that shows nothing (the deleted paragraph itself takes none) it is a blank line as it was.
void WriteTests::aParagraphAfterOneWithADeletedMarkThatShowsNothingTakesNoRoom()
{
    const QByteArray deleted = "<w:rPr><w:del w:id=\"7\" w:author=\"a\"/></w:rPr>";
    auto para = [&](const char *text, bool markDeleted) {
        return "<w:p><w:pPr>" + QByteArray(markDeleted ? deleted : QByteArray()) + "</w:pPr>" + (*text ? "<w:r><w:t>" + QByteArray(text) + "</w:t></w:r>" : QByteArray()) + "</w:p>";
    };
    auto gap = [&](const QByteArray &body) {
        auto doc = open(makeDocx(body, kStyles));
        const auto lines = doc ? doc->pager().lines(0) : QVector<StoryLayout::LineBrief>();
        return lines.size() >= 2 ? lines.last().rect.top() - lines.first().rect.top() : -1.0;
    };
    const double normal = gap(para("alpha", false) + para("", false) + para("gamma", false));   // (the text, a blank line, the text)
    const double joined = gap(para("alpha", true) + para("", false) + para("gamma", false));   // (the blank line is the rest of "alpha")
    QVERIFY2(normal > 0 && joined > 0, "no lines");
    QVERIFY2(std::abs(joined * 2 - normal) < 0.01, qPrintable(QStringLiteral("a line less: %1 against %2").arg(joined).arg(normal)));
    const double nothing = gap(para("alpha", false) + para("", true) + para("", false) + para("gamma", false));   // (alpha, one blank line, gamma)
    QVERIFY2(std::abs(nothing - normal) < 0.01, qPrintable(QStringLiteral("a deleted empty paragraph is no line: %1 against %2").arg(nothing).arg(normal)));
    // The lines of the text the person sees: the rest of "alpha" is no line of its own, where a deleted empty paragraph is no line either way.
    auto lineCount = [&](const QByteArray &body) {
        auto doc = open(makeDocx(body, kStyles));
        return doc ? int(doc->pager().lines(0).size()) : -1;
    };
    QCOMPARE(lineCount(para("alpha", false) + para("", false) + para("gamma", false)), 3);
    QCOMPARE(lineCount(para("alpha", false) + para("", true) + para("", false) + para("gamma", false)), 3);
    QCOMPARE(lineCount(para("alpha", true) + para("", false) + para("gamma", false)), 2);
}


// A paragraph whose mark a tracked change deleted is one paragraph with the next as the text stands: its text and the next's are laid out together, with the properties of the
// next (the mark that is left), whatever the first has. Pages, and the pages as drawn, are those of the one paragraph the two make; a number is counted once for them; a tab that
// survives in the first goes on into the next's line; and editing it changes the text only, the file keeps both paragraphs and the deletion.
void WriteTests::aParagraphWhoseMarkWasDeletedIsOneParagraphWithTheNext()
{
    const QByteArray deleted = "<w:rPr><w:del w:id=\"7\" w:author=\"a\"/></w:rPr>";
    auto run = [](const QByteArray &text) { return "<w:r><w:t xml:space=\"preserve\">" + text + "</w:t></w:r>"; };
    auto para = [&](const QByteArray &props, const QByteArray &content, bool markDeleted) {
        return "<w:p><w:pPr>" + props + (markDeleted ? deleted : QByteArray()) + "</w:pPr>" + content + "</w:p>";
    };
    auto words = [](int from, int count) {
        static const char *const w[] = {"court", "rule", "appeal", "section", "evidence", "judgment", "provision", "reasonable", "interpretation", "the", "of", "and"};
        QByteArray out;
        for (int i = from; i < from + count; ++i) out += (i > from ? " " : "") + QByteArray(w[(i * 7 + i / 5) % 12]);
        return out;
    };
    const QByteArray roomy = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray small = "<w:sectPr><w:pgSz w:w=\"6000\" w:h=\"4800\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray first = "<w:jc w:val=\"center\"/><w:spacing w:before=\"400\" w:after=\"600\"/>";   // (the first's properties are not the paragraph's)
    const QByteArray next = "<w:ind w:left=\"720\" w:firstLine=\"0\"/><w:jc w:val=\"both\"/>";
    auto doc = [&](const QByteArray &body, const QByteArray &sect) { return open(makeDocx(body, kStyles, QByteArray(), sect)); };

    // One line, as the one paragraph: where the lines are, what they hold, and their indent (the next's).
    {
        auto joined = doc(para(first, run("alpha beta"), true) + para(next, run(" gamma delta"), false), roomy);
        auto single = doc(para(next, run("alpha beta") + run(" gamma delta"), false), roomy);
        QVERIFY(joined && single);
        const auto a = joined->pager().lines(0), b = single->pager().lines(0);
        QCOMPARE(a.size(), 1);
        QCOMPARE(b.size(), 1);
        QCOMPARE(a[0].pos, 0);
        QCOMPARE(a[0].end, joined->story()->characterCount() - 1);
        QCOMPARE(a[0].rect, b[0].rect);
        QVERIFY2(std::abs(a[0].rect.left() - 36) < 0.01, qPrintable(QStringLiteral("the line begins at %1: the next paragraph's indent is 36 points").arg(a[0].rect.left())));
    }
    // Pages: five hundred words in two paragraphs of a page that holds a dozen lines are the pages (lines, positions, and pictures) of one paragraph of them.
    {
        auto joined = doc(para(first, run(words(0, 240)), true) + para(next, run(" " + words(240, 260)), false) + para(QByteArray(), run("After."), false), small);
        auto single = doc(para(next, run(words(0, 240)) + run(" " + words(240, 260)), false) + para(QByteArray(), run("After."), false), small);
        QVERIFY(joined && single);
        const int pages = joined->pager().pageCount();
        QVERIFY2(pages >= 5, qPrintable(QString::number(pages)));
        QCOMPARE(pages, single->pager().pageCount());
        PageView viewJoined, viewSingle;
        viewJoined.resize(1000, 700);
        viewSingle.resize(1000, 700);
        viewJoined.setDocument(joined.get());
        viewSingle.setDocument(single.get());
        // (The characters of the one paragraph are the two's, less the mark between them.)
        const int cut = joined->story()->findBlockByNumber(1).position();
        auto unjoined = [&](int position) { return position >= cut ? position - 1 : position; };
        for (int i = 0; i < pages; ++i) {
            const auto a = joined->pager().lines(i), b = single->pager().lines(i);
            QVERIFY2(a.size() == b.size(), qPrintable(QStringLiteral("page %1 has %2 lines, one paragraph's page %3").arg(i + 1).arg(a.size()).arg(b.size())));
            for (int k = 0; k < a.size(); ++k) {
                QVERIFY2(a[k].rect == b[k].rect && unjoined(a[k].pos) == b[k].pos && unjoined(a[k].end) == b[k].end,
                         qPrintable(QStringLiteral("page %1 line %2 is %3,%4 (%5 to %6), the one paragraph's %7,%8 (%9 to %10)").arg(i + 1).arg(k).arg(a[k].rect.left()).arg(a[k].rect.top()).arg(a[k].pos).arg(a[k].end)
                                        .arg(b[k].rect.left()).arg(b[k].rect.top()).arg(b[k].pos).arg(b[k].end)));
            }
            QVERIFY2(viewJoined.pageImage(i, 400) == viewSingle.pageImage(i, 400), qPrintable(QStringLiteral("page %1 is drawn differently").arg(i + 1)));
        }
    }
    // A number is counted once for the two (an item of the list, then the two, then another): 1., 2., 3.
    {
        const QByteArray level = "<w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr></w:lvl>";
        const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\">" + level + "</w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
        const QByteArray item = "<w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr>";
        auto numbered = open(makeDocx(para(item, run("one"), false) + para(item, run("two and"), true) + para(item, run(" three"), false) + para(item, run("four"), false), kStyles, numbering, roomy));
        QVERIFY(numbered);
        QStringList shown;
        for (QTextBlock b = numbered->story()->begin(); b.isValid(); b = b.next())
            if (b.textList() && !b.charFormat().boolProperty(tp::HiddenText))
                shown << (b.blockFormat().stringProperty(tp::BulletChar).isEmpty() ? b.textList()->itemText(b) : b.blockFormat().stringProperty(tp::BulletChar));
        QCOMPARE(shown, (QStringList{QStringLiteral("1."), QStringLiteral("2."), QStringLiteral("3.")}));
        QCOMPARE(int(numbered->pager().lines(0).size()), 3);
    }
    // A tab that survives in a paragraph that shows nothing else goes on into the next's line (it is no line of its own), and the next paragraph's own deleted tab is none.
    {
        const QByteArray stop = "<w:tabs><w:tab w:val=\"left\" w:pos=\"360\"/></w:tabs>";
        auto tabbed = doc(para(stop, "<w:r><w:tab/></w:r>", true) + para(stop, "<w:del w:id=\"8\" w:author=\"a\"><w:r><w:tab/></w:r></w:del>" + run("text"), false), roomy);
        QVERIFY(tabbed);
        QCOMPARE(int(tabbed->pager().lines(0).size()), 1);
    }
    // Editing: typing at the end of the first and at the start of the next changes the text; the file has both paragraphs, with the deletion, and the lines are still one.
    {
        auto edited = doc(para(first, run("alpha"), true) + para(next, run("omega"), false), roomy);
        QVERIFY(edited);
        QTextCursor c(edited->story());
        c.setPosition(5);
        insertText(*edited, c, QStringLiteral("1"));
        c.setPosition(edited->story()->findBlockByNumber(1).position());
        insertText(*edited, c, QStringLiteral("2"));
        QCOMPARE(edited->story()->toPlainText(), QStringLiteral("alpha1\n2omega"));
        QCOMPARE(int(edited->pager().lines(0).size()), 1);
        const QByteArray x = documentXml(edited->toDocx());
        QCOMPARE(int(x.count("<w:p>")), 2);
        QVERIFY2(x.contains("<w:del") && x.contains("alpha1") && x.contains("2omega"), x.constData());
    }
    // A run of thousands of paragraphs with deleted marks is cut into paragraphs of a bounded length, and every word is on a page.
    {
        QByteArray body;
        const int count = 4000;
        for (int i = 0; i < count; ++i) body += para(QByteArray(), run("w" + QByteArray::number(i) + " "), true);
        body += para(QByteArray(), run("end"), false);
        auto many = doc(body, small);
        QVERIFY(many);
        const int pages = many->pager().pageCount();
        QVERIFY(pages > 3);
        // (Every word is on a page once, the pages' lines one after another.)
        const QString whole = many->story()->toPlainText();
        QString shown;
        int last = 0;
        for (int i = 0; i < pages; ++i)
            for (const auto &l : many->pager().lines(i)) {
                QVERIFY2(l.pos >= last, qPrintable(QStringLiteral("a line at %1 begins before the one before it ends, at %2").arg(l.pos).arg(last)));
                shown += whole.mid(l.pos, l.end - l.pos);
                last = l.end;
            }
        QCOMPARE(last, many->story()->characterCount() - 1);
        QCOMPARE(shown.remove(QLatin1Char('\n')), QString(whole).remove(QLatin1Char('\n')));
    }
}
