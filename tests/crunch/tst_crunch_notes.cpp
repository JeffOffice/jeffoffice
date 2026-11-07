// Tests of the notes of cells in the window: the mark on a cell that has one, the note that appears when the pointer rests on it, New, Edit, Delete, and Show or Hide as steps of Undo,
// the notes following rows, columns, and a sort, and the file keeping all of them (and a conversation of comments as it was). Every workbook is built here.
#include "app/anchordialogs.h"
#include "app/threadspane.h"
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "savedok.h"
#include "sheet/notes.h"
#include "sheet/threads.h"
#include "../localepin.h"

#include <QAction>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;
using jp::office::sheet::NoteInfo;
using jp::office::sheet::noteKey;

namespace {

const char *kComments =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<comments xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><authors><author>Ann</author><author>Bob</author>"
    "<author>tc={11111111-2222-3333-4444-555555555555}</author></authors><commentList>"
    "<comment ref=\"A1\" authorId=\"0\" shapeId=\"0\"><text><r><rPr><b/><sz val=\"9\"/></rPr><t>Ann:</t></r><r><rPr><sz val=\"9\"/></rPr><t xml:space=\"preserve\">\ncheck the total</t></r></text></comment>"
    "<comment ref=\"C3\" authorId=\"1\" shapeId=\"0\"><text><t>Bob says hi</t></text></comment>"
    "<comment ref=\"D2\" authorId=\"2\" shapeId=\"0\"><text><t>[Threaded comment] fallback</t></text></comment></commentList></comments>";

const char *kVml =
    "<xml xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:x=\"urn:schemas-microsoft-com:office:excel\">"
    "<v:shapetype id=\"_x0000_t202\" coordsize=\"21600,21600\" o:spt=\"202\" path=\"m,l,21600r21600,l21600,xe\"><v:stroke joinstyle=\"miter\"/><v:path gradientshapeok=\"t\" o:connecttype=\"rect\"/></v:shapetype>"
    "<v:shape id=\"_x0000_s1025\" type=\"#_x0000_t202\" style=\"position:absolute;width:108pt;height:59pt;z-index:1;visibility:hidden\"><x:ClientData ObjectType=\"Note\"><x:Anchor>1, 15, 0, 2, 3, 15, 3, 16</x:Anchor>"
    "<x:Row>0</x:Row><x:Column>0</x:Column></x:ClientData></v:shape>"
    "<v:shape id=\"_x0000_s1026\" type=\"#_x0000_t202\" style=\"position:absolute;width:108pt;height:59pt;z-index:2;visibility:visible\"><x:ClientData ObjectType=\"Note\"><x:Anchor>3, 15, 1, 2, 5, 15, 4, 16</x:Anchor>"
    "<x:Row>2</x:Row><x:Column>2</x:Column><x:Visible/></x:ClientData></v:shape>"
    "<v:shape id=\"_x0000_s1027\" type=\"#_x0000_t202\" style=\"position:absolute;width:108pt;height:59pt;z-index:3;visibility:hidden\"><x:ClientData ObjectType=\"Note\"><x:Anchor>2, 15, 3, 2, 4, 15, 6, 16</x:Anchor>"
    "<x:Row>1</x:Row><x:Column>3</x:Column></x:ClientData></v:shape></xml>";

const char *kThreads =
    "<ThreadedComments xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\"><threadedComment ref=\"D2\" dT=\"2024-01-01T10:00:00.00\" personId=\"{AAAA}\" id=\"{11111111-2222-3333-4444-555555555555}\">"
    "<text>Is the date right?</text></threadedComment></ThreadedComments>";

// editBook() (A1:D3) with notes on A1 (by Ann), C3 (by Bob, shown), and D2 (a conversation of comments).
QByteArray bookWithNotes(const QByteArray &extraRows = QByteArray(), const QByteArray &threadXml = QByteArray(kThreads))
{
    QVector<ExtraPart> parts;
    parts.push_back({QStringLiteral("xl/comments1.xml"), QByteArray(kComments), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml"), 0, QStringLiteral("rId11"),
                     QStringLiteral("comments"), QStringLiteral("../comments1.xml")});
    parts.push_back({QStringLiteral("xl/drawings/vmlDrawing1.vml"), QByteArray(kVml), QStringLiteral("application/vnd.openxmlformats-officedocument.vmlDrawing"), 0, QStringLiteral("rId12"),
                     QStringLiteral("vmlDrawing"), QStringLiteral("../drawings/vmlDrawing1.vml")});
    parts.push_back({QStringLiteral("xl/threadedComments/threadedComment1.xml"), threadXml, QStringLiteral("application/vnd.ms-excel.threadedcomments+xml"), 0, QStringLiteral("rId13"),
                     QStringLiteral("threadedComment"), QStringLiteral("../threadedComments/threadedComment1.xml")});
    QMap<QString, QByteArray> all;
    jp::readZip(withParts(editBook(extraRows), parts), all);
    QByteArray sheetRels = all.value(QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels"));
    sheetRels.replace("http://schemas.openxmlformats.org/officeDocument/2006/relationships/threadedComment", "http://schemas.microsoft.com/office/2017/10/relationships/threadedComment");
    all[QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")] = sheetRels;
    QByteArray sheet = all.value(QStringLiteral("xl/worksheets/sheet1.xml"));
    sheet.replace("</worksheet>", "<legacyDrawing r:id=\"rId12\"/></worksheet>");
    all[QStringLiteral("xl/worksheets/sheet1.xml")] = sheet;
    jp::ZipWriter z;
    for (auto it = all.begin(); it != all.end(); ++it) z.add(it.key(), it.value(), true);
    return z.finish();
}

QPoint centerOf(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return e.view()->cellRectAt(r, c).center();
}

QString savedNoteText(const QString &path, const QString &ref)
{
    Package p;
    QString why;
    if (!p.readLazyFile(path, &why)) return why;
    Workbook b;
    if (!b.open(p, &why)) return why;
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    const auto notes = jp::office::sheet::readSheetNoteInfos(b, 0);
    const auto it = notes.find(noteKey(r, c));
    return it == notes.end() ? QStringLiteral("-") : it->second.text;
}

} // namespace

class CrunchNotesTests : public QObject {
    Q_OBJECT

    QLocale m_oldLocale;

    // Makes the window answer the note dialog: with this text (and the request kept for the test to look at).
    static void answerNotesWith(EditWindow &e, const QString &text, NoteRequest *seen = nullptr)
    {
        e.win.w.anchorHooks().noteAsker = [text, seen](NoteRequest *r) {
            if (seen) *seen = *r;
            if (!r->readOnly) r->text = text;
            return true;
        };
    }

private Q_SLOTS:
    void initTestCase()
    {
        static QTemporaryDir settings;
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settings.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
        setAppInfo(crunchAppInfo());
        initCore();
        m_oldLocale = QLocale();
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    }

    void cleanupTestCase() { QLocale::setDefault(m_oldLocale); }

    void cellsThatHaveNotesAreMarkedAndTheNoteAppearsWhenThePointerRests()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        QCOMPARE(e.view()->noteCellCount(), size_t(3));
        const crunch::CellAnchors &a = e.win.w.session()->anchors();
        QCOMPARE(a.noteAt(0, 0, 0)->author, QStringLiteral("Ann"));
        QCOMPARE(a.noteAt(0, 2, 2)->text, QStringLiteral("Bob says hi"));
        QVERIFY(a.noteAt(0, 2, 2)->shown);
        QVERIFY(a.noteAt(0, 1, 3)->threaded);
        QCOMPARE(a.noteAt(0, 1, 3)->text, QStringLiteral("Is the date right?"));
        // the pointer rests on A1: its note appears after a moment, with its words, and goes when the pointer does
        QTest::mouseMove(e.view()->viewport(), centerOf(e, QStringLiteral("A1")));
        auto popup = [&]() -> QLabel * { return e.win.w.findChild<QLabel *>(QStringLiteral("notePopup")); };
        QTRY_VERIFY(popup() && popup()->isVisible());
        QCOMPARE(popup()->text(), QStringLiteral("Ann:\ncheck the total"));
        QTest::mouseMove(e.view()->viewport(), centerOf(e, QStringLiteral("B3")));
        QTRY_VERIFY(!popup()->isVisible());
        // a note that is shown all the time is not shown twice
        QTest::mouseMove(e.view()->viewport(), centerOf(e, QStringLiteral("C3")));
        QTest::qWait(600);
        QVERIFY(!popup()->isVisible());
    }

    void aNoteCanBeMadeEditedAndDeletedAndEachIsAStepOfUndo()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("B2"));
        QCOMPARE(e.win.w.act(QStringLiteral("notes.edit"))->text(), QStringLiteral("New Note"));
        NoteRequest seen;
        answerNotesWith(e, QStringLiteral("first words"), &seen);
        e.win.w.act(QStringLiteral("notes.edit"))->trigger();
        QCOMPARE(seen.where, QStringLiteral("B2"));
        QVERIFY(seen.text.isEmpty());
        QVERIFY2(e.win.told.isEmpty(), qPrintable(e.win.told.join(QLatin1Char('\n'))));
        QCOMPARE(s->anchors().noteAt(0, 1, 1)->text, QStringLiteral("first words"));
        QCOMPARE(e.view()->noteCellCount(), size_t(1));
        QCOMPARE(e.win.w.act(QStringLiteral("notes.edit"))->text(), QStringLiteral("Edit Note"));
        // edited: the author stays the first person's
        answerNotesWith(e, QStringLiteral("second words"), &seen);
        e.win.w.act(QStringLiteral("notes.edit"))->trigger();
        QCOMPARE(seen.text, QStringLiteral("first words"));
        QCOMPARE(s->anchors().noteAt(0, 1, 1)->text, QStringLiteral("second words"));
        // undone and redone
        e.win.w.undoStack()->undo();
        QCOMPARE(s->anchors().noteAt(0, 1, 1)->text, QStringLiteral("first words"));
        e.win.w.undoStack()->undo();
        QVERIFY(!s->anchors().noteAt(0, 1, 1));
        QCOMPARE(e.view()->noteCellCount(), size_t(0));
        e.win.w.undoStack()->redo();
        e.win.w.undoStack()->redo();
        QCOMPARE(s->anchors().noteAt(0, 1, 1)->text, QStringLiteral("second words"));
        // deleted
        QVERIFY(e.win.w.act(QStringLiteral("notes.delete"))->isEnabled());
        e.win.w.act(QStringLiteral("notes.delete"))->trigger();
        QVERIFY(!s->anchors().noteAt(0, 1, 1));
        QCOMPARE(e.view()->noteCellCount(), size_t(0));
        QVERIFY(!e.win.w.act(QStringLiteral("notes.delete"))->isEnabled());
        e.win.w.undoStack()->undo();
        QCOMPARE(s->anchors().noteAt(0, 1, 1)->text, QStringLiteral("second words"));
        // emptied words take the note away
        answerNotesWith(e, QString());
        e.win.w.act(QStringLiteral("notes.edit"))->trigger();
        QVERIFY(!s->anchors().noteAt(0, 1, 1));
    }

    // Clear > Clear Comments and Notes takes the notes off the selected cells and nothing else; it is dimmed where there is no note.
    void clearCommentsAndNotesTakesOnlyTheNotes()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        crunch::EditSession *s = e.win.w.session();
        QAction *clear = e.win.w.act(QStringLiteral("edit.clear.notes"));
        QVERIFY(clear);
        e.select(QStringLiteral("B1"));
        QVERIFY(!clear->isEnabled());
        e.view()->selectRange(0, 0, 2, 2);                   // A1:C3
        QVERIFY(clear->isEnabled());
        const QString before = shown(e, QStringLiteral("A1:C3"));
        const int steps = e.win.w.undoStack()->count();
        clear->trigger();
        QVERIFY(!s->anchors().noteAt(0, 0, 0));
        QVERIFY(!s->anchors().noteAt(0, 2, 2));
        QVERIFY(s->anchors().noteAt(0, 1, 3));            // (D2 was outside the selection)
        QCOMPARE(shown(e, QStringLiteral("A1:C3")), before);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QVERIFY(!clear->isEnabled());
        e.win.w.undoStack()->undo();
        QVERIFY(s->anchors().noteAt(0, 0, 0));
    }

    void aNoteOfAConversationIsShownAndNotChanged()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        e.select(QStringLiteral("D2"));
        NoteRequest seen;
        answerNotesWith(e, QStringLiteral("overwritten"), &seen);
        e.win.w.act(QStringLiteral("notes.edit"))->trigger();
        QVERIFY(seen.readOnly);
        QCOMPARE(seen.text, QStringLiteral("Is the date right?"));
        QCOMPARE(e.win.w.session()->anchors().noteAt(0, 1, 3)->text, QStringLiteral("Is the date right?"));
        const int steps = e.win.w.undoStack()->count();
        e.win.w.act(QStringLiteral("notes.delete"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(e.win.w.session()->anchors().noteAt(0, 1, 3));
        QVERIFY(!e.win.told.isEmpty());   // (said why)
        // and the session itself refuses to change it, whoever asks
        const auto changed = e.win.w.session()->setNote(0, 1, 3, QStringLiteral("overwritten"), QStringLiteral("me"), false, QStringLiteral("Edit Note"));
        QVERIFY(!changed.ok);
        QVERIFY2(changed.problem.contains(QStringLiteral("conversation")), qPrintable(changed.problem));
        QCOMPARE(e.win.w.session()->anchors().noteAt(0, 1, 3)->text, QStringLiteral("Is the date right?"));
    }

    void aThreadIsStartedRepliedToResolvedEditedAndDeletedEachAsAStepOfUndo()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        crunch::EditSession *s = e.win.w.session();
        QUndoStack *u = e.win.w.undoStack();
        const int steps = u->count();
        QVERIFY(s->newComment(0, 0, 1, QStringLiteral("Is B1 right?"), QStringLiteral("Ann Lee"), QStringLiteral("New Comment")).ok);
        QCOMPARE(u->count(), steps + 1);
        const NoteInfo *n = s->anchors().noteAt(0, 0, 1);
        QVERIFY(n && n->threaded && n->thread);
        QCOMPARE(n->thread->size(), size_t(1));
        QCOMPARE(n->thread->at(0).author, QStringLiteral("Ann Lee"));
        QVERIFY(!n->thread->at(0).id.isEmpty() && !n->thread->at(0).personId.isEmpty() && !n->thread->at(0).time.isEmpty());
        // not on a cell that has a note or a thread, and not with no words
        QVERIFY(!s->newComment(0, 0, 0, QStringLiteral("x"), QStringLiteral("Ann Lee"), QStringLiteral("New Comment")).ok);
        QVERIFY(!s->newComment(0, 1, 3, QStringLiteral("x"), QStringLiteral("Ann Lee"), QStringLiteral("New Comment")).ok);
        QVERIFY(!s->newComment(0, 3, 3, QString(), QStringLiteral("Ann Lee"), QStringLiteral("New Comment")).ok);
        QCOMPARE(u->count(), steps + 1);
        // a reply by the same person keeps the person; the reply names its parent
        QVERIFY(s->replyToComment(0, 0, 1, QStringLiteral("Yes."), QStringLiteral("Ann Lee"), QStringLiteral("Reply")).ok);
        n = s->anchors().noteAt(0, 0, 1);
        QCOMPARE(n->thread->size(), size_t(2));
        QCOMPARE(n->thread->at(1).parentId, n->thread->at(0).id);
        QCOMPARE(n->thread->at(1).personId, n->thread->at(0).personId);
        QVERIFY(!s->replyToComment(0, 3, 3, QStringLiteral("x"), QStringLiteral("Ann Lee"), QStringLiteral("Reply")).ok);   // (no thread there)
        QVERIFY(!s->replyToComment(0, 0, 0, QStringLiteral("x"), QStringLiteral("Ann Lee"), QStringLiteral("Reply")).ok);   // (a plain note)
        // resolve and reopen
        QVERIFY(s->resolveThread(0, 0, 1, true, QStringLiteral("Resolve Thread")).ok);
        QVERIFY(jp::office::sheet::threadResolved(*s->anchors().noteAt(0, 0, 1)->thread));
        const int afterResolve = u->count();
        QVERIFY(s->resolveThread(0, 0, 1, true, QStringLiteral("Resolve Thread")).ok);
        QCOMPARE(u->count(), afterResolve);   // (already so: no step)
        QVERIFY(s->resolveThread(0, 0, 1, false, QStringLiteral("Reopen Thread")).ok);
        QVERIFY(!jp::office::sheet::threadResolved(*s->anchors().noteAt(0, 0, 1)->thread));
        // edit the reply's words
        QVERIFY(s->editComment(0, 0, 1, 1, QStringLiteral("Yes, it is."), QStringLiteral("Edit Comment")).ok);
        QCOMPARE(s->anchors().noteAt(0, 0, 1)->thread->at(1).text, QStringLiteral("Yes, it is."));
        QVERIFY(s->anchors().noteAt(0, 0, 1)->text.contains(QStringLiteral("Yes, it is.")));
        // delete the reply, and then the first comment, which takes the thread
        QVERIFY(s->deleteComment(0, 0, 1, 1, QStringLiteral("Delete Comment")).ok);
        QCOMPARE(s->anchors().noteAt(0, 0, 1)->thread->size(), size_t(1));
        QVERIFY(s->deleteComment(0, 0, 1, 0, QStringLiteral("Delete Comment")).ok);
        QVERIFY(!s->anchors().noteAt(0, 0, 1));
        // Undo takes each back, in turn
        u->undo();
        QCOMPARE(s->anchors().noteAt(0, 0, 1)->thread->size(), size_t(1));
        u->undo();
        QCOMPARE(s->anchors().noteAt(0, 0, 1)->thread->size(), size_t(2));
        u->undo();
        QCOMPARE(s->anchors().noteAt(0, 0, 1)->thread->at(1).text, QStringLiteral("Yes."));
        u->undo();   // reopen
        QVERIFY(jp::office::sheet::threadResolved(*s->anchors().noteAt(0, 0, 1)->thread));
        u->undo();   // resolve
        QVERIFY(!jp::office::sheet::threadResolved(*s->anchors().noteAt(0, 0, 1)->thread));
        u->redo();
        QVERIFY(jp::office::sheet::threadResolved(*s->anchors().noteAt(0, 0, 1)->thread));
        while (u->index() > steps) u->undo();
        QVERIFY(!s->anchors().noteAt(0, 0, 1));
    }

    void aReplyToAThreadFromTheFileKeepsItsPeopleAndIsSavedAndReadBack()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->replyToComment(0, 1, 3, QStringLiteral("It is."), QStringLiteral("Ann Lee"), QStringLiteral("Reply")).ok);
        QVERIFY(s->newComment(0, 2, 0, QStringLiteral("A3?"), QStringLiteral("Ann Lee"), QStringLiteral("New Comment")).ok);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const crunch::CellAnchors &a = second.w.session()->anchors();
        const NoteInfo *d2 = a.noteAt(0, 1, 3);
        QVERIFY(d2 && d2->thread);
        QCOMPARE(d2->thread->size(), size_t(2));
        QCOMPARE(d2->thread->at(0).text, QStringLiteral("Is the date right?"));
        QCOMPARE(d2->thread->at(0).personId, QStringLiteral("{AAAA}"));
        QCOMPARE(d2->thread->at(1).text, QStringLiteral("It is."));
        QCOMPARE(d2->thread->at(1).author, QStringLiteral("Ann Lee"));
        const NoteInfo *a3 = a.noteAt(0, 2, 0);
        QVERIFY(a3 && a3->thread && a3->thread->size() == 1);
        QCOMPARE(a3->thread->at(0).text, QStringLiteral("A3?"));
        // the notes the person did not touch are as they were
        QCOMPARE(a.noteAt(0, 0, 0)->text, QStringLiteral("Ann:\ncheck the total"));
        QCOMPARE(a.noteAt(0, 2, 2)->text, QStringLiteral("Bob says hi"));
    }

    void notesBecomeCommentsAndCommentsBecomeNotesAndEachIsOneStepOfUndo()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        crunch::EditSession *s = e.win.w.session();
        QUndoStack *u = e.win.w.undoStack();
        const int steps = u->count();
        const std::vector<std::pair<quint32, quint32>> cells = {{0, 0}, {2, 2}, {1, 3}};   // (A1 and C3 are notes; D2 is a thread already)
        QVERIFY(s->convertNotesToComments(0, cells, QStringLiteral("Me"), QStringLiteral("Convert Notes to Comments")).ok);
        QCOMPARE(u->count(), steps + 1);
        const NoteInfo *a1 = s->anchors().noteAt(0, 0, 0);
        QVERIFY(a1 && a1->threaded && a1->thread && a1->thread->size() == 1);
        QCOMPARE(a1->thread->at(0).text, QStringLiteral("Ann:\ncheck the total"));
        QCOMPARE(a1->thread->at(0).author, QStringLiteral("Ann"));
        QVERIFY(s->anchors().noteAt(0, 2, 2)->threaded);
        QCOMPARE(s->anchors().noteAt(0, 1, 3)->thread->size(), size_t(1));
        QVERIFY(s->convertCommentsToNotes(0, cells, QStringLiteral("Convert Comments to Notes")).ok);
        a1 = s->anchors().noteAt(0, 0, 0);
        QVERIFY(a1 && !a1->threaded && !a1->thread);
        QCOMPARE(a1->author, QStringLiteral("Ann"));
        QVERIFY(!s->anchors().noteAt(0, 1, 3)->threaded);
        QCOMPARE(s->anchors().noteAt(0, 1, 3)->text, QStringLiteral("Is the date right?"));
        u->undo();
        QVERIFY(s->anchors().noteAt(0, 0, 0)->threaded);
        QVERIFY(s->anchors().noteAt(0, 1, 3)->threaded);
        u->undo();
        QVERIFY(!s->anchors().noteAt(0, 0, 0)->threaded);
        QVERIFY(s->anchors().noteAt(0, 1, 3)->threaded);
    }

    void threadsAreDeletedWholeAndSurviveSortsAndRowsInTheFile()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->newComment(0, 0, 1, QStringLiteral("B1"), QStringLiteral("Me"), QStringLiteral("New Comment")).ok);
        QVERIFY(s->insertRows(0, 0, 2).ok);
        QVERIFY(s->anchors().noteAt(0, 2, 1) && s->anchors().noteAt(0, 2, 1)->threaded);
        QVERIFY(s->deleteThreads(0, {{3, 3}, {2, 1}}, QStringLiteral("Delete Comments")).ok);
        QVERIFY(!s->anchors().noteAt(0, 2, 1));
        QVERIFY(!s->anchors().noteAt(0, 3, 3));
        QVERIFY(s->anchors().noteAt(0, 2, 0));   // (the notes stay)
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QVERIFY(!second.w.session()->anchors().noteAt(0, 3, 3));
        QVERIFY(!second.w.session()->anchors().noteAt(0, 2, 1));
        QCOMPARE(second.w.session()->anchors().notes(0).size(), size_t(2));
    }

    void theCommentCommandsAreEnabledWhereTheyCanDoSomething()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        auto on = [&](const char *id) { return e.win.w.act(QString::fromLatin1(id))->isEnabled(); };
        e.select(QStringLiteral("B1"));
        QVERIFY(on("comments.new"));
        QVERIFY(!on("comments.reply") && !on("comments.resolve") && !on("comments.delete"));
        QVERIFY(on("comments.prev") && on("comments.next") && on("comments.show"));
        QVERIFY(on("comments.convert") && on("comments.revert"));   // (A1 and C3 are notes, D2 is a thread)
        e.select(QStringLiteral("D2"));
        QVERIFY(!on("comments.new"));
        QVERIFY(on("comments.reply") && on("comments.resolve") && on("comments.delete"));
        QCOMPARE(e.win.w.act(QStringLiteral("comments.resolve"))->text(), QStringLiteral("Resolve Thread"));
        QVERIFY(e.win.w.session()->resolveThread(0, 1, 3, true, QStringLiteral("Resolve Thread")).ok);
        QCOMPARE(e.win.w.act(QStringLiteral("comments.resolve"))->text(), QStringLiteral("Reopen Thread"));
        e.select(QStringLiteral("A1"));   // a plain note: not a thread to reply to
        QVERIFY(!on("comments.new") && !on("comments.reply") && !on("comments.delete"));
    }

    void newCommentAsksForTheWordsAndTheCommentsPaneListsTheThreads()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        e.select(QStringLiteral("B1"));
        NoteRequest seen;
        answerNotesWith(e, QStringLiteral("Check B1"), &seen);
        const int steps = e.win.w.undoStack()->count();
        e.win.w.act(QStringLiteral("comments.new"))->trigger();
        QVERIFY(seen.title.contains(QStringLiteral("B1")));
        const NoteInfo *n = e.win.w.session()->anchors().noteAt(0, 0, 1);
        QVERIFY(n && n->thread && n->thread->size() == 1);
        QCOMPARE(n->thread->at(0).text, QStringLiteral("Check B1"));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        // the pane opens with it and lists both threads (B1 and D2) in the order of the cells
        CommentThreadsPane *pane = e.win.w.commentsPane();
        QVERIFY(pane);
        QCOMPARE(pane->cardCount(), 2);
        QVERIFY(e.win.w.act(QStringLiteral("comments.show"))->isChecked());
        QCOMPARE(pane->findChildren<QFrame *>(QStringLiteral("commentCard")).value(0)->property("cell").toString(), QStringLiteral("B1"));
        // Show Comments again closes it
        e.win.w.act(QStringLiteral("comments.show"))->trigger();
        QVERIFY(!e.win.w.act(QStringLiteral("comments.show"))->isChecked());
        // Undo takes the thread away, and the pane follows
        e.win.w.act(QStringLiteral("comments.show"))->trigger();
        e.win.w.undoStack()->undo();
        QTRY_COMPARE(pane->cardCount(), 1);
    }

    void replyResolveAndDeleteWorkFromThePaneAndEachIsAStepOfUndo()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        e.win.w.act(QStringLiteral("comments.show"))->trigger();
        CommentThreadsPane *pane = e.win.w.commentsPane();
        QVERIFY(pane && pane->cardCount() == 1);
        auto button = [&](const char *name) { return pane->findChild<QPushButton *>(QString::fromLatin1(name)); };
        const int steps = e.win.w.undoStack()->count();
        answerNotesWith(e, QStringLiteral("It is."));
        QVERIFY(button("commentReply"));
        button("commentReply")->click();
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(s->anchors().noteAt(0, 1, 3)->thread->size(), size_t(2));
        QCOMPARE(s->anchors().noteAt(0, 1, 3)->thread->at(1).text, QStringLiteral("It is."));
        QTRY_VERIFY(pane->findChildren<QLabel *>(QStringLiteral("commentText")).size() == 2);
        QCOMPARE(button("commentResolve")->text(), QStringLiteral("Resolve"));
        button("commentResolve")->click();
        QVERIFY(jp::office::sheet::threadResolved(*s->anchors().noteAt(0, 1, 3)->thread));
        QTRY_COMPARE(button("commentResolve")->text(), QStringLiteral("Reopen"));
        button("commentResolve")->click();
        QVERIFY(!jp::office::sheet::threadResolved(*s->anchors().noteAt(0, 1, 3)->thread));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 3);
        QTRY_VERIFY(button("commentDelete"));
        button("commentDelete")->click();
        QVERIFY(!s->anchors().noteAt(0, 1, 3));
        QTRY_COMPARE(pane->cardCount(), 0);
        QVERIFY(pane->findChild<QLabel *>(QStringLiteral("commentsEmpty")));
        e.win.w.undoStack()->undo();
        QVERIFY(s->anchors().noteAt(0, 1, 3));
    }

    void thePopupShowsTheWholeThreadWithItsRepliesAndWhetherItIsResolved()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->replyToComment(0, 1, 3, QStringLiteral("It is."), QStringLiteral("Ann Lee"), QStringLiteral("Reply")).ok);
        QVERIFY(s->resolveThread(0, 1, 3, true, QStringLiteral("Resolve Thread")).ok);
        QTest::mouseMove(e.view()->viewport(), centerOf(e, QStringLiteral("B3")));   // (the pointer was on D2 in the test before: a move there from elsewhere is the one that is noticed)
        QTest::mouseMove(e.view()->viewport(), centerOf(e, QStringLiteral("D2")));
        auto popup = [&]() -> QLabel * { return e.win.w.findChild<QLabel *>(QStringLiteral("notePopup")); };
        QTRY_VERIFY(popup() && popup()->isVisible());
        const QString text = popup()->text();
        QVERIFY2(text.startsWith(QStringLiteral("Resolved\n")), qPrintable(text));
        QVERIFY(text.contains(QStringLiteral("Is the date right?")));
        QVERIFY(text.contains(QStringLiteral("Ann Lee")));
        QVERIFY(text.contains(QStringLiteral("  It is.")));   // (the reply is set in)
    }

    void previousAndNextCommentGoFromThreadToThreadAndRoundTheSheet()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->newComment(0, 2, 0, QStringLiteral("A3"), QStringLiteral("Me"), QStringLiteral("New Comment")).ok);
        e.select(QStringLiteral("A1"));
        e.win.w.act(QStringLiteral("comments.next"))->trigger();
        QCOMPARE(e.selected(), QStringLiteral("D2"));   // (A1 and C3 are plain notes: only threads count)
        e.win.w.act(QStringLiteral("comments.next"))->trigger();
        QCOMPARE(e.selected(), QStringLiteral("A3"));
        e.win.w.act(QStringLiteral("comments.next"))->trigger();
        QCOMPARE(e.selected(), QStringLiteral("D2"));
        e.win.w.act(QStringLiteral("comments.prev"))->trigger();
        QCOMPARE(e.selected(), QStringLiteral("A3"));
        e.select(QStringLiteral("D2"));
        e.win.w.act(QStringLiteral("comments.prev"))->trigger();
        QCOMPARE(e.selected(), QStringLiteral("A3"));
    }

    void convertCommandsTurnTheSheetsNotesIntoCommentsAndBackAsOneStepEach()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        e.win.w.setConfirmAsker([](const QString &, const QString &) { return true; });
        crunch::EditSession *s = e.win.w.session();
        const int steps = e.win.w.undoStack()->count();
        e.win.w.act(QStringLiteral("comments.convert"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QVERIFY(s->anchors().noteAt(0, 0, 0)->thread && s->anchors().noteAt(0, 2, 2)->thread);
        QVERIFY(!e.win.w.act(QStringLiteral("comments.convert"))->isEnabled());   // (nothing left to convert)
        e.win.w.act(QStringLiteral("comments.revert"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps + 2);
        QVERIFY(!s->anchors().noteAt(0, 0, 0)->thread && !s->anchors().noteAt(0, 1, 3)->thread);
        QVERIFY(!e.win.w.act(QStringLiteral("comments.revert"))->isEnabled());
        // the file keeps it all: three notes, none a thread
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QCOMPARE(second.w.session()->anchors().notes(0).size(), size_t(3));
        QVERIFY(!second.w.session()->anchors().noteAt(0, 1, 3)->threaded);
        QCOMPARE(second.w.session()->anchors().noteAt(0, 1, 3)->text, QStringLiteral("Is the date right?"));
    }

    void aThreadOfThousandsOfRepliesWithLoopsAndMissingPeopleOpensShowsAndIsLeftAlone()
    {
        QByteArray threads = "<ThreadedComments xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\">";
        for (int i = 0; i < 4000; ++i)
            threads += "<threadedComment ref=\"D2\" dT=\"2024-01-01T10:00:00.00\" personId=\"{NOBODY}\" id=\"{R" + QByteArray::number(i) + "}\" parentId=\"{R" + QByteArray::number((i + 1) % 4000) + "}\"><text>reply "
                       + QByteArray::number(i) + "</text></threadedComment>";
        threads += "</ThreadedComments>";
        EditWindow e;
        QVERIFY(e.open(bookWithNotes(QByteArray(), threads)));
        crunch::EditSession *s = e.win.w.session();
        const NoteInfo *n = s->anchors().noteAt(0, 1, 3);
        QVERIFY(n && n->thread && n->threadCut);
        QCOMPARE(n->thread->size(), size_t(jp::office::sheet::kMaxThreadComments));
        // shown: the popup and the pane draw a few of them, and say how many more there are
        QTest::mouseMove(e.view()->viewport(), centerOf(e, QStringLiteral("B3")));   // (the pointer was on D2 in the test before: a move there from elsewhere is the one that is noticed)
        QTest::mouseMove(e.view()->viewport(), centerOf(e, QStringLiteral("D2")));
        auto popup = [&]() -> QLabel * { return e.win.w.findChild<QLabel *>(QStringLiteral("notePopup")); };
        QTRY_VERIFY(popup() && popup()->isVisible());
        QVERIFY(popup()->text().contains(QStringLiteral("and 460 more")));
        e.win.w.act(QStringLiteral("comments.show"))->trigger();
        QCOMPARE(e.win.w.commentsPane()->cardCount(), 1);
        // not changed: a reply, a resolve, and an edit are refused with a reason; the cell can still be cleared
        QVERIFY(!s->replyToComment(0, 1, 3, QStringLiteral("x"), QStringLiteral("Me"), QStringLiteral("Reply")).ok);
        QVERIFY(!s->resolveThread(0, 1, 3, true, QStringLiteral("Resolve Thread")).ok);
        QVERIFY(!s->editComment(0, 1, 3, 0, QStringLiteral("x"), QStringLiteral("Edit")).ok);
        e.select(QStringLiteral("D2"));
        QVERIFY(!e.win.w.act(QStringLiteral("comments.reply"))->isEnabled());
        QVERIFY(!e.win.w.act(QStringLiteral("comments.resolve"))->isEnabled());
        // saving something else leaves the thread's part as it was
        QVERIFY(s->setNote(0, 0, 0, QStringLiteral("changed"), QStringLiteral("Ann"), false, QStringLiteral("Edit Note")).ok);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QVERIFY(second.w.session()->anchors().noteAt(0, 1, 3)->threadCut);
    }

    void showAndHideChangeWhetherANoteIsShownAndCanBeUndone()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("A1"));
        QVERIFY(!s->anchors().noteAt(0, 0, 0)->shown);
        QVERIFY(!e.win.w.act(QStringLiteral("notes.showHide"))->isChecked());
        e.win.w.act(QStringLiteral("notes.showHide"))->trigger();
        QVERIFY(s->anchors().noteAt(0, 0, 0)->shown);
        QVERIFY(e.win.w.act(QStringLiteral("notes.showHide"))->isChecked());
        e.win.w.undoStack()->undo();
        QVERIFY(!s->anchors().noteAt(0, 0, 0)->shown);
        // all of them
        QCOMPARE(e.win.w.act(QStringLiteral("notes.showAll"))->text(), QStringLiteral("Show Every Note"));
        e.win.w.act(QStringLiteral("notes.showAll"))->trigger();
        for (const auto &n : s->anchors().notes(0)) QVERIFY(n.second.shown);
        QCOMPARE(e.win.w.act(QStringLiteral("notes.showAll"))->text(), QStringLiteral("Hide Every Note"));
        e.win.w.act(QStringLiteral("notes.showAll"))->trigger();
        for (const auto &n : s->anchors().notes(0)) QVERIFY(!n.second.shown);
    }

    void notesGoWithTheirCellsWhenRowsAndColumnsAreInsertedAndDeletedAndComeBackWithUndo()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->insertRows(0, 0, 2).ok);
        QVERIFY(s->anchors().noteAt(0, 2, 0));            // A1 -> A3
        QVERIFY(s->anchors().noteAt(0, 4, 2));            // C3 -> C5
        QVERIFY(!s->anchors().noteAt(0, 0, 0));
        QVERIFY(s->insertColumns(0, 1, 1).ok);
        QVERIFY(s->anchors().noteAt(0, 4, 3));            // C5 -> D5
        QVERIFY(s->anchors().noteAt(0, 2, 0));
        QVERIFY(s->deleteRows(0, 2, 1).ok);               // the row of A3
        QVERIFY(!s->anchors().noteAt(0, 2, 0));
        QCOMPARE(s->anchors().notes(0).size(), size_t(2));
        QCOMPARE(e.view()->noteCellCount(), size_t(2));
        e.win.w.undoStack()->undo();
        QVERIFY(s->anchors().noteAt(0, 2, 0));
        QCOMPARE(s->anchors().noteAt(0, 2, 0)->text, QStringLiteral("Ann:\ncheck the total"));
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        QVERIFY(s->anchors().noteAt(0, 0, 0));
        QVERIFY(s->anchors().noteAt(0, 2, 2));
        QCOMPARE(s->anchors().notes(0).size(), size_t(3));
        // and again
        e.win.w.undoStack()->redo();
        QVERIFY(s->anchors().noteAt(0, 2, 0));
        QCOMPARE(e.view()->noteCellCount(), size_t(3));
    }

    // Round 6: notes and links go with their cells through edits of rows and columns, and come back whole with Undo and Redo, over and over; what the steps hold is the notes and links they moved or
    // took away, and no cell (counted: the bytes of the steps stay under what a few cells' copies would take).
    void notesAndLinksComeBackWholeWithUndoAndRedoOfRowsAndColumns()
    {
        // (and 100,000 rows below with a height of their own each: what the steps hold is what the edits changed)
        QString tall;
        for (int r = 6; r <= 100000; ++r) tall += QStringLiteral("<row r=\"%1\" ht=\"%2\" customHeight=\"1\"><c r=\"F%1\"><v>%1</v></c></row>").arg(r).arg(15 + r % 5);
        EditWindow e;
        QVERIFY(e.open(bookWithNotes(tall.toUtf8())));
        crunch::EditSession *s = e.win.w.session();
        jp::office::sheet::LinkSpec link;
        link.range.valid = true;
        link.range.row1 = link.range.row2 = 2;
        link.range.col1 = link.range.col2 = 0;
        link.location = QStringLiteral("Sheet1!B1");
        link.tip = QStringLiteral("to B1");
        QVERIFY(s->setLink(0, link.range, link, QStringLiteral("Making a link")).ok);   // on A3
        QVERIFY(e.settle());
        const qint64 base = s->undoBytes();
        auto state = [&] {
            QStringList out;
            for (const auto &n : s->anchors().notes(0))
                out << QStringLiteral("note %1: %2|%3|%4|%5").arg(n.first).arg(n.second.author, n.second.text).arg(n.second.shown).arg(n.second.threaded);
            for (const auto &l : s->anchors().links(0)) out << QStringLiteral("link %1:%2 %3 %4 %5").arg(l.range.row1).arg(l.range.col1).arg(l.target, l.location, l.tip);
            return out.join(QLatin1Char('\n'));
        };
        const QString start = state();
        QVERIFY(s->insertRows(0, 0, 2).ok);
        QVERIFY(s->insertColumns(0, 1, 1).ok);
        QVERIFY(s->deleteRows(0, 4, 1).ok);   // (the row of A3 is row 5 now; its link and its note are on cells that go)
        QVERIFY(s->deleteColumns(0, 3, 1).ok);
        QVERIFY(e.settle());
        const QString done = state();
        QVERIFY(done != start);
        QVERIFY2(s->undoBytes() - base < 24 * 1024, qPrintable(QString::number(s->undoBytes() - base)));
        for (int round = 0; round < 3; ++round) {
            for (int i = 0; i < 4; ++i) e.win.w.undoStack()->undo();
            QCOMPARE(state(), start);
            for (int i = 0; i < 4; ++i) e.win.w.undoStack()->redo();
            QCOMPARE(state(), done);
        }
        QVERIFY(e.settle());
        QVERIFY2(s->undoBytes() - base < 24 * 1024, qPrintable(QString::number(s->undoBytes() - base)));
    }

    void notesFollowASort()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        crunch::EditSession *s = e.win.w.session();
        // A1:A3 holds 1, 10, 0.25; sorted from the largest, the 10 goes first, the 1 second, and the 0.25 stays last: the note on A1 (the 1) goes to A2
        jp::crunch::SortSpec spec;
        jp::crunch::SortLevel level;
        level.key = 0;
        level.descending = true;
        spec.levels.append(level);
        jp::office::sheet::CellRange range;
        jp::office::sheet::parseRangeRef(QStringLiteral("A1:C3"), &range);
        QVERIFY(s->sortRange(0, range, spec).ok);
        QCOMPARE(s->displayText(0, 0, 0), QStringLiteral("10"));
        QVERIFY(s->anchors().noteAt(0, 1, 0));
        QVERIFY(!s->anchors().noteAt(0, 0, 0));
        QVERIFY(s->anchors().noteAt(0, 2, 2));   // (the note on C3 is on the last row, which stays)
        e.win.w.undoStack()->undo();
        QVERIFY(s->anchors().noteAt(0, 0, 0));
        QVERIFY(!s->anchors().noteAt(0, 1, 0));
    }

    void theFileKeepsNewEditedAndDeletedNotesAndTheOnesNotChanged()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        crunch::EditSession *s = e.win.w.session();
        // rows inserted above (the file's notes move), a note made, one edited, one deleted, one shown
        QVERIFY(s->insertRows(0, 0, 1).ok);
        e.select(QStringLiteral("B5"));
        answerNotesWith(e, QStringLiteral("made here"));
        e.win.w.act(QStringLiteral("notes.edit"))->trigger();
        e.select(QStringLiteral("C4"));   // (C3, moved)
        e.win.w.act(QStringLiteral("notes.showHide"))->trigger();
        e.select(QStringLiteral("A2"));   // (A1, moved)
        answerNotesWith(e, QStringLiteral("Ann:\nchecked"));
        e.win.w.act(QStringLiteral("notes.edit"))->trigger();
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QCOMPARE(savedNoteText(e.path, QStringLiteral("A2")), QStringLiteral("Ann:\nchecked"));
        QCOMPARE(savedNoteText(e.path, QStringLiteral("B5")), QStringLiteral("made here"));
        QCOMPARE(savedNoteText(e.path, QStringLiteral("C4")), QStringLiteral("Bob says hi"));
        QCOMPARE(savedNoteText(e.path, QStringLiteral("D3")), QStringLiteral("Is the date right?"));   // (the conversation, moved with its cell)
        QCOMPARE(savedNoteText(e.path, QStringLiteral("A1")), QStringLiteral("-"));
        // the person who wrote the edited note is still its author, and the one who made the other is new
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const crunch::CellAnchors &a = second.w.session()->anchors();
        QCOMPARE(a.noteAt(0, 1, 0)->author, QStringLiteral("Ann"));
        QVERIFY(a.noteAt(0, 4, 1) && a.noteAt(0, 4, 1)->author != QStringLiteral("Ann") && a.noteAt(0, 4, 1)->author != QStringLiteral("Bob"));
        QVERIFY(a.noteAt(0, 3, 2) && !a.noteAt(0, 3, 2)->shown);   // (C3 was shown in the file, and the person hid it)
        // (the file's conversation is still one)
        QVERIFY(a.noteAt(0, 2, 3)->threaded);
    }

    void afterASaveEditsAndUndoStillSaveRight()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("B2"));
        answerNotesWith(e, QStringLiteral("one"));
        e.win.w.act(QStringLiteral("notes.edit"))->trigger();
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QCOMPARE(savedNoteText(e.path, QStringLiteral("B2")), QStringLiteral("one"));
        // a row inserted above, the note edited in its new place, then both undone and the file saved
        QVERIFY(s->insertRows(0, 0, 1).ok);
        e.select(QStringLiteral("B3"));
        answerNotesWith(e, QStringLiteral("two"));
        e.win.w.act(QStringLiteral("notes.edit"))->trigger();
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        QVERIFY(s->anchors().noteAt(0, 1, 1));
        QCOMPARE(s->anchors().noteAt(0, 1, 1)->text, QStringLiteral("one"));
        QSignalSpy saved2(&e.win.w, &CrunchWindow::saved);
        e.win.w.undoStack()->redo();   // (the row again, so the file has something to change)
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved2, 20000), "the second save failed");
        QCOMPARE(savedNoteText(e.path, QStringLiteral("B3")), QStringLiteral("one"));
        QCOMPARE(savedNoteText(e.path, QStringLiteral("B2")), QStringLiteral("-"));
    }

    // A sheet added since the last save takes notes and links like any other, and they are in the file after the save.
    void aSheetAddedSinceTheLastSaveTakesNotesAndLinks()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));
        crunch::EditSession *s = e.win.w.session();
        int made = -1;
        QVERIFY(s->insertSheet(1, QStringLiteral("Fresh"), &made).ok);
        QVERIFY(made >= 0);
        QVERIFY(s->setNote(made, 1, 1, QStringLiteral("on a new sheet"), QStringLiteral("Zed"), false, QStringLiteral("Note")).ok);
        jp::office::sheet::LinkSpec link;
        link.range = jp::office::sheet::CellRange{2, 2, 2, 2, true};
        link.target = QStringLiteral("https://example.org/new");
        QVERIFY(s->setLink(made, link.range, link, QStringLiteral("Link")).ok);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const crunch::CellAnchors &a = second.w.session()->anchors();
        QVERIFY(a.noteAt(made, 1, 1));
        QCOMPARE(a.noteAt(made, 1, 1)->text, QStringLiteral("on a new sheet"));
        QCOMPARE(a.links(made).size(), size_t(1));
    }

    // What a note says is shown as the words it is: markup in it (a picture to fetch, say) is text, in the note that shows beside the cell, in the one that is shown all the time, and in the
    // tips of the lists of the search.
    void aNoteFromAFileIsShownAsPlainText()
    {
        QByteArray comments = QByteArray(kComments);
        comments.replace("Bob says hi", "&lt;img src=&quot;file:///etc/hostname&quot; width=&quot;400&quot;&gt;&lt;b&gt;bold&lt;/b&gt;");
        comments.replace("<t xml:space=\"preserve\">\ncheck the total</t>", "<t>&lt;img src=&quot;file:///etc/passwd&quot;&gt;</t>");
        ExtraPart part;
        part.name = QStringLiteral("xl/comments1.xml");
        part.data = comments;
        part.contentType = QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml");
        part.sheet = 0;
        part.relId = QStringLiteral("rIdC");
        part.relType = QStringLiteral("comments");
        part.relTarget = QStringLiteral("../comments1.xml");
        EditWindow e;
        QVERIFY(e.open(withParts(editBook(), {part})));
        QTest::mouseMove(e.view()->viewport(), centerOf(e, QStringLiteral("A1")));
        auto popup = [&]() -> QLabel * { return e.win.w.findChild<QLabel *>(QStringLiteral("notePopup")); };
        QTRY_VERIFY(popup() && popup()->isVisible());
        QCOMPARE(popup()->textFormat(), Qt::PlainText);
        QVERIFY(popup()->text().contains(QStringLiteral("<img src=\"file:///etc/passwd\">")));
        // its size is that of the words, not of a picture
        QVERIFY(popup()->width() < 400 && popup()->height() < 200);
    }

    // A note that is shown all the time is laid out once for the zoom it is drawn at, and kept: drawing again does not lay it out again.
    // What a step of Undo holds for an insert or a delete is what the edit moved or took away, and counts in the undo budget: an edit past every note holds next to nothing, one above them all
    // holds a record for each; and Undo and Redo bring every note back to where it was, through many edits.
    void anEditHoldsOnlyWhatItMovedAndUndoAndRedoPutItBack()
    {
        QByteArray comments = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<comments xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><authors><author>Ann</author></authors><commentList>";
        for (int r = 1; r <= 4000; ++r) comments += "<comment ref=\"E" + QByteArray::number(r) + "\" authorId=\"0\" shapeId=\"0\"><text><t>n" + QByteArray::number(r) + "</t></text></comment>";
        comments += "</commentList></comments>";
        ExtraPart part;
        part.name = QStringLiteral("xl/comments1.xml");
        part.data = comments;
        part.contentType = QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml");
        part.sheet = 0;
        part.relId = QStringLiteral("rIdC");
        part.relType = QStringLiteral("comments");
        part.relTarget = QStringLiteral("../comments1.xml");
        EditWindow e;
        QVERIFY(e.open(withParts(editBook(), {part})));
        crunch::EditSession *s = e.win.w.session();
        auto snapshot = [&] {
            std::map<quint64, QString> m;
            for (const auto &n : s->anchors().notes(0)) m[n.first] = n.second.text;
            return m;
        };
        const auto original = snapshot();
        QCOMPARE(original.size(), size_t(4000));
        const qint64 base = s->undoBytes();
        QVERIFY(s->insertRows(0, 5000, 2).ok);                 // (below every note: it moves none)
        const qint64 past = s->undoBytes() - base;
        QVERIFY(s->insertRows(0, 0, 2).ok);                    // (above them all: it moves every one)
        const qint64 above = s->undoBytes() - base - past;
        QVERIFY2(above > past + 4000 * 40, qPrintable(QString::number(past) + " " + QString::number(above)));
        QVERIFY2(past < 20000, qPrintable(QString::number(past)));
        const auto afterTwo = snapshot();
        QVERIFY(s->deleteRows(0, 100, 50).ok);
        QVERIFY(s->insertColumns(0, 2, 3).ok);                 // (E moves to H)
        QVERIFY(s->deleteColumns(0, 7, 1).ok);                 // (and the column the notes are in goes: with every note)
        QCOMPARE(s->anchors().notes(0).size(), size_t(0));
        const auto finalState = snapshot();
        for (int i = 0; i < 5; ++i) e.win.w.undoStack()->undo();
        QVERIFY(snapshot() == original);
        for (int i = 0; i < 5; ++i) e.win.w.undoStack()->redo();
        QVERIFY(snapshot() == finalState);
        for (int i = 0; i < 3; ++i) e.win.w.undoStack()->undo();   // (the column deleted, the columns inserted, the rows deleted)
        QVERIFY(snapshot() == afterTwo);
        QCOMPARE(s->anchors().notes(0).size(), size_t(4000));
        e.win.w.undoStack()->redo();                                 // (the 50 rows deleted took 50 notes)
        QCOMPARE(s->anchors().notes(0).size(), size_t(4000 - 50));
        e.win.w.undoStack()->undo();
        QVERIFY(snapshot() == afterTwo);
    }

    void aShownNoteIsLaidOutOnceAndNotAtEveryPaint()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithNotes()));   // (C3's note is shown)
        e.win.w.grab();
        const qint64 first = e.view()->shownNoteLayouts();
        QCOMPARE(first, qint64(1));
        for (int i = 0; i < 5; ++i) {
            e.view()->viewport()->repaint();
            e.win.w.grab();
        }
        QCOMPARE(e.view()->shownNoteLayouts(), first);
        // another zoom lays it out for that zoom, once
        e.view()->setZoom(1.5);
        e.win.w.grab();
        e.win.w.grab();
        QCOMPARE(e.view()->shownNoteLayouts(), first + 1);
        // and a note of a very long text is cut where the one beside the cell is (what is laid out is bounded)
        EditWindow longOne;
        QVERIFY(longOne.open(bookWithNotes()));
        longOne.select(QStringLiteral("A1"));
        answerNotesWith(longOne, QString(500000, QLatin1Char('x')));
        longOne.win.w.act(QStringLiteral("notes.edit"))->trigger();
        longOne.win.w.act(QStringLiteral("notes.showHide"))->trigger();
        QVERIFY(longOne.win.w.session()->anchors().noteAt(0, 0, 0)->shown);
        QElapsedTimer t;
        t.start();
        longOne.win.w.grab();
        const qint64 once = longOne.view()->shownNoteLayouts();
        QVERIFY(once <= 3);   // (C3's, and the long one's)
        longOne.win.w.grab();
        QCOMPARE(longOne.view()->shownNoteLayouts(), once);
        QVERIFY(t.elapsed() < 20000);
    }

    void aSheetWithMoreNotesThanAreKeptShowsThemAndLeavesThemAlone()
    {
        // 20,050 notes: the first 20,000 are kept, and the sheet's notes are not changed (the rest could not be followed through the rows and columns put in)
        QByteArray comments = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<comments xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><authors><author>Ann</author></authors><commentList>";
        for (int r = 1; r <= 20050; ++r) comments += "<comment ref=\"E" + QByteArray::number(r) + "\" authorId=\"0\" shapeId=\"0\"><text><t>n" + QByteArray::number(r) + "</t></text></comment>";
        comments += "</commentList></comments>";
        ExtraPart part;
        part.name = QStringLiteral("xl/comments1.xml");
        part.data = comments;
        part.contentType = QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml");
        part.sheet = 0;
        part.relId = QStringLiteral("rIdC");
        part.relType = QStringLiteral("comments");
        part.relTarget = QStringLiteral("../comments1.xml");
        EditWindow e;
        QVERIFY(e.open(withParts(editBook(), {part})));
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(s->anchors().notes(0).size(), size_t(20000));
        QVERIFY(s->anchors().truncated(0));
        QString why;
        QVERIFY(!s->canEditNotes(0, &why));
        QVERIFY2(why.contains(QStringLiteral("20000")) || why.contains(QStringLiteral("20,000")), qPrintable(why));
        QVERIFY(!s->setNote(0, 0, 0, QStringLiteral("x"), QStringLiteral("me"), false, QStringLiteral("New Note")).ok);
        QVERIFY(s->anchors().noteAt(0, 0, 4));   // (shown)
        // the file's notes are not touched by a save of something else
        QVERIFY(s->type(0, 5, 0, QStringLiteral("7")).ok);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QCOMPARE(savedNoteText(e.path, QStringLiteral("E20050")), QStringLiteral("n20050"));
    }

    void notesCannotBeMadeOnAWorkbookFromATextFileAndTheReasonIsGiven()
    {
        EditWindow e;
        QVERIFY(e.open("a,b\n1,2\n", QStringLiteral("data.csv")));
        answerNotesWith(e, QStringLiteral("x"));
        e.select(QStringLiteral("A1"));
        e.win.w.act(QStringLiteral("notes.edit"))->trigger();
        QVERIFY(e.win.w.session()->anchors().notes(0).empty());
    }
};

JP_TEST_CLASS(CrunchNotesTests)
#include "tst_crunch_notes.moc"
