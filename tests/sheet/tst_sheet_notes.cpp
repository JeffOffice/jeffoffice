// Tests of the notes, the links, and the protection of a sheet as the file has them: read from a workbook built here, changed, saved by saveXlsx, and read again. Every workbook is built in
// memory (never a real-world file).
#include "sheethelpers.h"

#include "drawinghelpers.h"
#include "io/zip.h"
#include "sheet/notes.h"
#include "sheet/notesave.h"
#include "sheet/protection.h"
#include "sheet/xlsxsave.h"

using namespace jp;

namespace {

const QString kContentComments = QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml");
const QString kContentVml = QStringLiteral("application/vnd.openxmlformats-officedocument.vmlDrawing");

QMap<QString, QByteArray> entriesOf(const QByteArray &zip)
{
    QMap<QString, QByteArray> parts;
    jp::readZip(zip, parts);
    return parts;
}

QByteArray zipOf(const QMap<QString, QByteArray> &parts)
{
    jp::ZipWriter z;
    for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
    return z.finish();
}

// The cell comments of the notes part used in these tests: Ann's note on A1 (formatted text), Bob's on C3, and a comment that belongs to a thread on B5.
const char *kComments =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<comments xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><authors><author>Ann</author><author>Bob</author>"
    "<author>tc={11111111-2222-3333-4444-555555555555}</author></authors><commentList>"
    "<comment ref=\"A1\" authorId=\"0\" shapeId=\"0\"><text><r><rPr><b/><sz val=\"9\"/><rFont val=\"Tahoma\"/></rPr><t>Ann:</t></r><r><rPr><sz val=\"9\"/><rFont val=\"Tahoma\"/></rPr>"
    "<t xml:space=\"preserve\">\ncheck the total</t></r><rPh sb=\"0\" eb=\"1\"><t>PHONETIC</t></rPh></text></comment>"
    "<comment ref=\"C3\" authorId=\"1\" shapeId=\"0\"><text><t>Bob says hi</t></text></comment>"
    "<comment ref=\"B5\" authorId=\"2\" shapeId=\"0\"><text><t>[Threaded comment] fallback words</t></text></comment></commentList></comments>";

const char *kVml =
    "<xml xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:x=\"urn:schemas-microsoft-com:office:excel\">"
    "<o:shapelayout v:ext=\"edit\"><o:idmap v:ext=\"edit\" data=\"1\"/></o:shapelayout>"
    "<v:shapetype id=\"_x0000_t202\" coordsize=\"21600,21600\" o:spt=\"202\" path=\"m,l,21600r21600,l21600,xe\"><v:stroke joinstyle=\"miter\"/><v:path gradientshapeok=\"t\" o:connecttype=\"rect\"/></v:shapetype>"
    "<v:shape id=\"_x0000_s1025\" type=\"#_x0000_t202\" style=\"position:absolute;margin-left:80pt;margin-top:2pt;width:108pt;height:59pt;z-index:1;visibility:hidden\" fillcolor=\"#ffffe1\">"
    "<x:ClientData ObjectType=\"Note\"><x:MoveWithCells/><x:SizeWithCells/><x:Anchor>1, 15, 0, 2, 3, 15, 3, 16</x:Anchor><x:AutoFill>False</x:AutoFill><x:Row>0</x:Row><x:Column>0</x:Column></x:ClientData></v:shape>"
    "<v:shape id=\"_x0000_s1026\" type=\"#_x0000_t202\" style=\"position:absolute;width:108pt;height:59pt;z-index:2;visibility:visible\" fillcolor=\"#ffffe1\">"
    "<x:ClientData ObjectType=\"Note\"><x:Anchor>3, 15, 1, 2, 5, 15, 4, 16</x:Anchor><x:Row>2</x:Row><x:Column>2</x:Column><x:Visible/></x:ClientData></v:shape>"
    "<v:shape id=\"_x0000_s1027\" type=\"#_x0000_t202\" style=\"position:absolute;width:108pt;height:59pt;z-index:3;visibility:hidden\" fillcolor=\"#ffffe1\">"
    "<x:ClientData ObjectType=\"Note\"><x:Anchor>2, 15, 3, 2, 4, 15, 6, 16</x:Anchor><x:Row>4</x:Row><x:Column>1</x:Column></x:ClientData></v:shape></xml>";

const char *kThreads =
    "<ThreadedComments xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\"><threadedComment ref=\"B5\" dT=\"2024-01-01T10:00:00.00\" personId=\"{AAAAAAAA-0000-0000-0000-000000000001}\" id=\"{11111111-2222-3333-4444-555555555555}\">"
    "<text>Is this figure final?</text></threadedComment><threadedComment ref=\"B5\" dT=\"2024-01-01T11:00:00.00\" personId=\"{AAAAAAAA-0000-0000-0000-000000000002}\" id=\"{66666666-2222-3333-4444-555555555555}\" parentId=\"{11111111-2222-3333-4444-555555555555}\">"
    "<text>Yes, final.</text></threadedComment></ThreadedComments>";
const char *kPersons =
    "<personList xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\"><person displayName=\"Dee\" id=\"{AAAAAAAA-0000-0000-0000-000000000001}\" userId=\"dee\" providerId=\"None\"/>"
    "<person displayName=\"Eli\" id=\"{AAAAAAAA-0000-0000-0000-000000000002}\" userId=\"eli\" providerId=\"None\"/></personList>";

const QString kRows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c></row><row r=\"3\"><c r=\"C3\"><v>3</v></c></row><row r=\"5\"><c r=\"B5\"><v>5</v></c></row>");

QByteArray plainBook(const QString &after = QString())
{
    return buildXlsx({worksheet(kRows, after)});
}

// A workbook whose first sheet has the three notes (the one on B5 from a thread).
QByteArray notesBook(bool threads = true, const QByteArray &threadXml = QByteArray(kThreads))
{
    QVector<ExtraPart> parts;
    parts.push_back({QStringLiteral("xl/comments1.xml"), QByteArray(kComments), kContentComments, 0, QStringLiteral("rId1"), QStringLiteral("comments"), QStringLiteral("../comments1.xml")});
    parts.push_back({QStringLiteral("xl/drawings/vmlDrawing1.vml"), QByteArray(kVml), kContentVml, 0, QStringLiteral("rId2"), QStringLiteral("vmlDrawing"), QStringLiteral("../drawings/vmlDrawing1.vml")});
    if (threads) {
        parts.push_back({QStringLiteral("xl/threadedComments/threadedComment1.xml"), threadXml, QStringLiteral("application/vnd.ms-excel.threadedcomments+xml"), 0, QStringLiteral("rId3"),
                         QStringLiteral("threadedComment"), QStringLiteral("../threadedComments/threadedComment1.xml")});
    }
    QByteArray book = withParts(buildXlsx({worksheet(kRows, QStringLiteral("<legacyDrawing r:id=\"rId2\"/>"))}), parts);
    if (threads) {
        // (the persons are the workbook's, and the threads have a relationship type of their own)
        QMap<QString, QByteArray> all = entriesOf(book);
        all[QStringLiteral("xl/persons/person.xml")] = QByteArray(kPersons);
        QByteArray rels = all.value(QStringLiteral("xl/_rels/workbook.xml.rels"));
        rels.replace("</Relationships>", "<Relationship Id=\"rIdP\" Type=\"http://schemas.microsoft.com/office/2017/10/relationships/person\" Target=\"persons/person.xml\"/></Relationships>");
        all[QStringLiteral("xl/_rels/workbook.xml.rels")] = rels;
        QByteArray sheetRels = all.value(QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels"));
        sheetRels.replace("/relationships/threadedComment\"", "/relationships/threadedComment\"");
        sheetRels.replace("http://schemas.openxmlformats.org/officeDocument/2006/relationships/threadedComment", "http://schemas.microsoft.com/office/2017/10/relationships/threadedComment");
        all[QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")] = sheetRels;
        book = zipOf(all);
    }
    return book;
}

struct Opened {
    QByteArray bytes;
    Package package;
    Workbook book;
    bool ok = false;
    QString why;
};

std::unique_ptr<Opened> open(const QByteArray &bytes)
{
    auto o = std::make_unique<Opened>();
    o->bytes = bytes;
    o->ok = o->package.readLazy(o->bytes, &o->why) && o->book.open(o->package, &o->why);
    return o;
}

// The workbook saved with the changes, and opened again.
std::unique_ptr<Opened> saved(Opened &from, const SaveChanges &changes, QString *why)
{
    QByteArray out;
    if (!saveXlsx(from.book, changes, &out, why)) return nullptr;
    auto o = open(out);
    return o->ok ? std::move(o) : nullptr;
}

NoteOp put(quint32 row, quint32 col, const QString &author, const QString &text, bool shown = false)
{
    NoteOp op;
    op.row = row;
    op.col = col;
    op.author = author;
    op.text = text;
    op.shown = shown;
    return op;
}

NoteOp drop(quint32 row, quint32 col)
{
    NoteOp op;
    op.row = row;
    op.col = col;
    op.remove = true;
    return op;
}

ThreadComment comment(const QString &id, const QString &parent, const QString &person, const QString &name, const QString &text, bool done = false)
{
    ThreadComment c;
    c.id = id;
    c.parentId = parent;
    c.personId = person;
    c.author = name;
    c.userId = name.toLower();
    c.providerId = QStringLiteral("None");
    c.time = QStringLiteral("2026-11-07T09:30:00.00");
    c.text = text;
    c.done = done;
    return c;
}

// The change that puts a thread on a cell: the older note that stands for it, and the thread.
NoteOp putThread(quint32 row, quint32 col, const ThreadList &list)
{
    NoteOp op = put(row, col, legacyThreadAuthor(list), legacyThreadText(list));
    op.thread = std::make_shared<const ThreadList>(list);
    return op;
}

// The external relationships a sheet's part has, by id: its target.
QMap<QString, QString> hyperlinkRels(const Package &p, const QString &sheetPart)
{
    QMap<QString, QString> out;
    for (const Relationship &r : p.relationshipsOfType(sheetPart, QStringLiteral("/hyperlink")))
        if (r.external) out.insert(r.id, r.target);
    return out;
}

} // namespace

class SheetNotesTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // ---- reading

    void notesAreReadWithAuthorsShownFlagAndThreads()
    {
        auto o = open(notesBook());
        QVERIFY2(o->ok, qPrintable(o->why));
        const auto notes = readSheetNoteInfos(o->book, 0);
        QCOMPARE(notes.size(), size_t(3));
        const NoteInfo &a = notes.at(noteKey(0, 0));
        QCOMPARE(a.author, QStringLiteral("Ann"));
        QCOMPARE(a.text, QStringLiteral("Ann:\ncheck the total"));   // (the phonetic run is not text)
        QVERIFY(!a.shown);
        QVERIFY(!a.threaded);
        const NoteInfo &c = notes.at(noteKey(2, 2));
        QCOMPARE(c.author, QStringLiteral("Bob"));
        QVERIFY(c.shown);
        const NoteInfo &t = notes.at(noteKey(4, 1));
        QVERIFY(t.threaded);
        QCOMPARE(t.text, QStringLiteral("Dee: Is this figure final?\n\nEli: Yes, final."));
        QVERIFY(t.author.isEmpty());
        // Find's reader still gives just the texts
        QCOMPARE(readSheetNotes(o->book, 0).at(noteKey(2, 2)), QStringLiteral("Bob says hi"));
    }

    // A character a file can't hold as it is (a carriage return, a control character, an underscore before an x and four hex digits) is written _xHHHH_ and is read as the character, and a text
    // that has one is written so that it reads back as it was.
    void charactersTheFileWritesAsEscapesAreReadAndWrittenBack()
    {
        QByteArray comments = QByteArray(kComments);
        comments.replace("<t>Bob says hi</t>", "<t xml:space=\"preserve\">line one_x000D_\nline two _x0007_ and _x005F_x0041_ stay</t>");
        QVector<ExtraPart> parts;
        parts.push_back({QStringLiteral("xl/comments1.xml"), comments, kContentComments, 0, QStringLiteral("rId1"), QStringLiteral("comments"), QStringLiteral("../comments1.xml")});
        auto o = open(withParts(plainBook(), parts));
        QVERIFY2(o->ok, qPrintable(o->why));
        const QString expected = QStringLiteral("line one\r\nline two \a and _x0041_ stay");
        QCOMPARE(readSheetNoteInfos(o->book, 0).at(noteKey(2, 2)).text, expected);
        // put again (with what was read), saved, and read: the same
        SaveChanges changes;
        changes.sheets[0].noteOps = {put(2, 2, QStringLiteral("Bob"), expected)};
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        QCOMPARE(readSheetNoteInfos(again->book, 0).at(noteKey(2, 2)).text, expected);
        QVERIFY(!entriesOf(again->bytes).value(QStringLiteral("xl/comments1.xml")).contains('\r'));
    }

    // A comments part that two sheets name is read once for the workbook, not once for each sheet (a hostile file can name one part from a thousand sheets).
    void aCommentsPartTwoSheetsNameIsReadOnce()
    {
        QVector<ExtraPart> parts;
        parts.push_back({QStringLiteral("xl/comments1.xml"), QByteArray(kComments), kContentComments, 0, QStringLiteral("rId1"), QStringLiteral("comments"), QStringLiteral("../comments1.xml")});
        parts.push_back({QStringLiteral("xl/dummy.txt"), QByteArray("x"), QString(), 1, QStringLiteral("rId1"), QStringLiteral("comments"), QStringLiteral("../comments1.xml")});
        auto o = open(withParts(buildXlsx({worksheet(kRows), worksheet(kRows)}), parts));
        QVERIFY2(o->ok, qPrintable(o->why));
        NoteReadContext shared;
        const qint64 before = noteReads();
        const auto a = readSheetNoteInfos(o->book, 0, nullptr, kMaxNotes, &shared);
        const auto b = readSheetNoteInfos(o->book, 1, nullptr, kMaxNotes, &shared);
        QCOMPARE(noteReads() - before, qint64(1));
        QCOMPARE(a.size(), size_t(3));
        QCOMPARE(b.size(), size_t(3));
        // without it each sheet reads it
        const qint64 mid = noteReads();
        readSheetNoteInfos(o->book, 0);
        readSheetNoteInfos(o->book, 1);
        QCOMPARE(noteReads() - mid, qint64(2));
    }

    // A person's name in a thread is cut, and what is read of names and notes is counted for the workbook.
    void aHugeNameOfAPersonIsCutAndTheCharactersAreCounted()
    {
        QMap<QString, QByteArray> all = entriesOf(notesBook());
        all[QStringLiteral("xl/persons/person.xml")] = "<personList xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\"><person displayName=\"" + QByteArray(2000000, 'x')
                                                       + "\" id=\"{AAAAAAAA-0000-0000-0000-000000000001}\"/><person displayName=\"Eli\" id=\"{AAAAAAAA-0000-0000-0000-000000000002}\"/></personList>";
        auto o = open(zipOf(all));
        QVERIFY2(o->ok, qPrintable(o->why));
        NoteReadContext context;
        const auto notes = readSheetNoteInfos(o->book, 0, nullptr, kMaxNotes, &context);
        const QString text = notes.at(noteKey(4, 1)).text;
        QVERIFY2(text.size() < kMaxPersonChars + 100, qPrintable(QString::number(text.size())));
        QVERIFY(text.startsWith(QString(kMaxPersonChars, QLatin1Char('x')) + QStringLiteral(": Is this")));
        QVERIFY(context.chars > 0 && context.chars < 10000);   // (the long name is counted as cut)
    }

    void aSheetWithNoNotesHasNone()
    {
        auto o = open(plainBook());
        QVERIFY(o->ok);
        QVERIFY(readSheetNoteInfos(o->book, 0).empty());
        QVERIFY(notesDrawingPart(o->package, QStringLiteral("/xl/worksheets/sheet1.xml")).isEmpty());
    }

    // ---- writing

    void changedNotesLeaveTheOthersAsTheFileHasThem()
    {
        const std::vector<NoteOp> ops = {drop(0, 0), put(2, 2, QStringLiteral("Cy"), QStringLiteral("New words & <more>")), put(3, 3, QStringLiteral("Bob"), QStringLiteral("fresh"), true)};
        QByteArray xml;
        QString why;
        QVERIFY2(applyNoteOpsToComments(QByteArray(kComments), ops, &xml, &why), qPrintable(why));
        // the threaded comment, as it was
        const QByteArray threadedElement = "<comment ref=\"B5\" authorId=\"2\" shapeId=\"0\"><text><t>[Threaded comment] fallback words</t></text></comment>";
        QVERIFY(xml.contains(threadedElement));
        QVERIFY(!xml.contains("Ann:"));
        QVERIFY(xml.contains("<author>Cy</author>"));
        QVERIFY(xml.contains("New words &amp; &lt;more&gt;"));
        // the ones the file has stay in their places (the edited one where it was); a note made for a cell the file has none for goes after them
        const qsizetype c3 = xml.indexOf("ref=\"C3\""), d4 = xml.indexOf("ref=\"D4\""), b5 = xml.indexOf("ref=\"B5\"");
        QVERIFY(c3 > 0 && b5 > c3 && d4 > b5);
        QCOMPARE(xml.count("<comment "), 3);
        // Bob (already an author) is not added twice
        QCOMPARE(xml.count("<author>Bob</author>"), 1);
    }

    // The cells of a file's comments are in no order we can count on (the order a program wrote them in, row by row or column by column): a change to one is made to that one wherever it is.
    void commentsInNoParticularOrderAreChangedByTheirCells()
    {
        const QByteArray comments =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?><comments xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><authors><author>Ann</author></authors><commentList>"
            "<comment ref=\"A1\" authorId=\"0\"><text><t>one</t></text></comment><comment ref=\"A2\" authorId=\"0\"><text><t>two</t></text></comment>"
            "<comment ref=\"B1\" authorId=\"0\"><text><t>three</t></text></comment></commentList></comments>";
        QByteArray edited, removed;
        QString why;
        QVERIFY2(applyNoteOpsToComments(comments, {put(0, 1, QStringLiteral("Ann"), QStringLiteral("THREE"))}, &edited, &why), qPrintable(why));
        QCOMPARE(edited.count("<comment "), 3);
        QCOMPARE(edited.count("ref=\"B1\""), 1);
        QVERIFY(edited.contains("THREE") && !edited.contains(">three<") && edited.contains(">one<") && edited.contains(">two<"));
        QVERIFY(edited.indexOf("ref=\"A2\"") < edited.indexOf("ref=\"B1\""));   // (where it was)
        QVERIFY2(applyNoteOpsToComments(comments, {drop(0, 1)}, &removed, &why), qPrintable(why));
        QCOMPARE(removed.count("<comment "), 2);
        QVERIFY(!removed.contains("B1") && removed.contains(">one<") && removed.contains(">two<"));
        // and a note put on a cell the file has none for goes after them
        QByteArray added;
        QVERIFY2(applyNoteOpsToComments(comments, {put(0, 2, QStringLiteral("Ann"), QStringLiteral("four"))}, &added, &why), qPrintable(why));
        QCOMPARE(added.count("<comment "), 4);
        QVERIFY(added.indexOf("ref=\"C1\"") > added.indexOf("ref=\"B1\""));
    }

    void notesAreAddedToAFileThatHasNone()
    {
        auto o = open(plainBook());
        QVERIFY(o->ok);
        SaveChanges changes;
        changes.sheets[0].noteOps = {put(0, 1, QStringLiteral("Jo"), QStringLiteral("first")), put(4, 2, QStringLiteral("Jo"), QStringLiteral("line one\nline two"), true)};
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        const auto notes = readSheetNoteInfos(again->book, 0);
        QCOMPARE(notes.size(), size_t(2));
        QCOMPARE(notes.at(noteKey(0, 1)).text, QStringLiteral("first"));
        QCOMPARE(notes.at(noteKey(0, 1)).author, QStringLiteral("Jo"));
        QVERIFY(!notes.at(noteKey(0, 1)).shown);
        QCOMPARE(notes.at(noteKey(4, 2)).text, QStringLiteral("line one\nline two"));
        QVERIFY(notes.at(noteKey(4, 2)).shown);
        // the sheet names the drawing, and the parts and types are there
        const QMap<QString, QByteArray> parts = entriesOf(again->bytes);
        QVERIFY(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("<legacyDrawing"));
        QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains("comments1.xml"));
        QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains("vmlDrawing1.vml"));
        QVERIFY(!notesDrawingPart(again->package, QStringLiteral("/xl/worksheets/sheet1.xml")).isEmpty());
    }

    // A sheet given a note and a header picture in one save has two legacy drawings, and they are two parts: the file has one name for one part (the program that reads it asks to repair a file in
    // which two drawings take the same name, and a part named twice in the types).
    void aNoteAndAHeaderPictureAddedTogetherGetDrawingsOfTheirOwn()
    {
        auto o = open(plainBook());
        QVERIFY(o->ok);
        SaveChanges changes;
        changes.sheets[0].noteOps = {put(0, 1, QStringLiteral("Jo"), QStringLiteral("first"))};
        changes.sheets[0].setPage = true;
        HeaderPicture pic;
        pic.place = QStringLiteral("CH");
        pic.image = drawtest::pngBytes(8, 4, Qt::red);
        pic.extension = QStringLiteral("png");
        pic.width = 40;
        pic.height = 20;
        changes.sheets[0].page.headerFooter.oddHeader = QStringLiteral("&C&G");
        changes.sheets[0].page.headerFooter.pictures.append(pic);
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        const QVector<Relationship> drawings = again->package.relationshipsOfType(QStringLiteral("/xl/worksheets/sheet1.xml"), QStringLiteral("/vmlDrawing"));
        QCOMPARE(drawings.size(), 2);
        QVERIFY(drawings[0].target.compare(drawings[1].target, Qt::CaseInsensitive) != 0);
        const QByteArray types = entriesOf(again->bytes).value(QStringLiteral("[Content_Types].xml"));
        QCOMPARE(types.count("vmlDrawing1.vml"), 1);
        QCOMPARE(types.count("vmlDrawing2.vml"), 1);
        // both are in the notes' and the picture's own readers' hands
        QCOMPARE(readSheetNoteInfos(again->book, 0).size(), size_t(1));
        Sheet sheet;
        QVERIFY(again->book.readSheet(0, &sheet));
        QCOMPARE(sheet.page.headerFooter.pictures.size(), 1);
    }

    void editsAndDeletesToTheFilesNotesAreSavedAndTheThreadIsKept()
    {
        auto o = open(notesBook());
        QVERIFY(o->ok);
        SaveChanges changes;
        changes.sheets[0].noteOps = {drop(0, 0), put(2, 2, QStringLiteral("Bob"), QStringLiteral("edited"), false), put(6, 0, QStringLiteral("Ann"), QStringLiteral("another"))};
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        const auto notes = readSheetNoteInfos(again->book, 0);
        QCOMPARE(notes.size(), size_t(3));
        QVERIFY(notes.count(noteKey(0, 0)) == 0);
        QCOMPARE(notes.at(noteKey(2, 2)).text, QStringLiteral("edited"));
        QVERIFY(!notes.at(noteKey(2, 2)).shown);   // (it was shown; the change hides it)
        QCOMPARE(notes.at(noteKey(6, 0)).text, QStringLiteral("another"));
        // the thread: its part is as it was, and its note is still shown from it, and not to be changed
        QCOMPARE(entriesOf(again->bytes).value(QStringLiteral("xl/threadedComments/threadedComment1.xml")), QByteArray(kThreads));
        QVERIFY(notes.at(noteKey(4, 1)).threaded);
        QCOMPARE(notes.at(noteKey(4, 1)).text, QStringLiteral("Dee: Is this figure final?\n\nEli: Yes, final."));
        // the shapes: one for each note, the new one made
        const QByteArray vml = entriesOf(again->bytes).value(QStringLiteral("xl/drawings/vmlDrawing1.vml"));
        QCOMPARE(vml.count("ObjectType=\"Note\""), 3);
        QVERIFY(vml.contains("<x:Row>6</x:Row>"));
        QVERIFY(vml.contains("id=\"_x0000_s1027\""));   // (the one the file had is as it was)
    }

    void threadsAreReadWithPeopleTimesRepliesAndResolved()
    {
        QByteArray threads = kThreads;
        threads.replace("id=\"{11111111-2222-3333-4444-555555555555}\">", "id=\"{11111111-2222-3333-4444-555555555555}\" done=\"1\">");
        auto o = open(notesBook(true, threads));
        QVERIFY(o->ok);
        const auto notes = readSheetNoteInfos(o->book, 0);
        const NoteInfo &n = notes.at(noteKey(4, 1));
        QVERIFY(n.threaded);
        QVERIFY(n.thread);
        QCOMPARE(n.thread->size(), size_t(2));
        const ThreadComment &first = n.thread->at(0), &reply = n.thread->at(1);
        QCOMPARE(first.author, QStringLiteral("Dee"));
        QCOMPARE(first.userId, QStringLiteral("dee"));
        QCOMPARE(first.time, QStringLiteral("2024-01-01T10:00:00.00"));
        QVERIFY(first.parentId.isEmpty());
        QVERIFY(first.done);
        QCOMPARE(reply.author, QStringLiteral("Eli"));
        QCOMPARE(reply.parentId, first.id);
        QCOMPARE(reply.text, QStringLiteral("Yes, final."));
        QVERIFY(threadResolved(*n.thread));
        QCOMPARE(threadDepth(*n.thread, 1), 1);
    }

    void aThreadWithNoOlderNoteIsStillRead()
    {
        QByteArray comments = kComments;
        comments.replace("<comment ref=\"B5\" authorId=\"2\" shapeId=\"0\"><text><t>[Threaded comment] fallback words</t></text></comment>", "");
        QMap<QString, QByteArray> all = entriesOf(notesBook());
        all[QStringLiteral("xl/comments1.xml")] = comments;
        auto o = open(zipOf(all));
        QVERIFY(o->ok);
        const auto notes = readSheetNoteInfos(o->book, 0);
        QVERIFY(notes.count(noteKey(4, 1)));
        QVERIFY(notes.at(noteKey(4, 1)).thread);
        QCOMPARE(notes.at(noteKey(4, 1)).thread->size(), size_t(2));
    }

    void hostileThreadsAreCappedAndEndInsteadOfHanging()
    {
        // thousands of replies, a person the list does not have, a parent that is itself, and two that name each other
        QByteArray threads = "<ThreadedComments xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\">";
        threads += "<threadedComment ref=\"B5\" dT=\"2024-01-01T10:00:00.00\" personId=\"{GONE}\" id=\"{R0}\" parentId=\"{R0}\"><text>loop</text></threadedComment>";
        for (int i = 1; i < 3000; ++i)
            threads += "<threadedComment ref=\"B5\" dT=\"2024-01-01T10:00:00.00\" personId=\"{GONE}\" id=\"{R" + QByteArray::number(i) + "}\" parentId=\"{R" + QByteArray::number(i % 2 ? i + 1 : i - 1) + "}\"><text>x</text></threadedComment>";
        threads += "</ThreadedComments>";
        auto o = open(notesBook(true, threads));
        QVERIFY(o->ok);
        const auto notes = readSheetNoteInfos(o->book, 0);
        const NoteInfo &n = notes.at(noteKey(4, 1));
        QVERIFY(n.thread);
        QCOMPARE(n.thread->size(), size_t(kMaxThreadComments));
        QVERIFY(n.threadCut);
        QVERIFY(n.thread->at(0).author.isEmpty());
        const std::vector<int> depths = threadDepths(*n.thread);
        QCOMPARE(depths.size(), n.thread->size());
        for (size_t i = 0; i < n.thread->size(); i += 7) {
            QVERIFY(depths[i] <= 32);
            QCOMPARE(depths[i], threadDepth(*n.thread, i));
        }
    }

    void aThreadIsSavedWithItsOlderNoteAndItsPeople()
    {
        auto o = open(notesBook());
        QVERIFY(o->ok);
        const auto before = readSheetNoteInfos(o->book, 0);
        QVERIFY(before.at(noteKey(4, 1)).thread);
        ThreadList list = *before.at(noteKey(4, 1)).thread;
        list.push_back(comment(QStringLiteral("{77777777-0000-0000-0000-000000000001}"), list[0].id, QStringLiteral("{BBBBBBBB-0000-0000-0000-000000000009}"), QStringLiteral("Fay"), QStringLiteral("Thanks <both> & done")));
        list[0].done = true;
        SaveChanges changes;
        changes.sheets[0].noteOps = {putThread(4, 1, list)};
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        const NoteInfo n = readSheetNoteInfos(again->book, 0).at(noteKey(4, 1));
        QVERIFY(n.threaded);
        QVERIFY(n.thread);
        QCOMPARE(*n.thread, list);
        const auto parts = entriesOf(again->bytes);
        QVERIFY(parts.value(QStringLiteral("xl/persons/person.xml")).contains("displayName=\"Fay\""));
        QVERIFY(parts.value(QStringLiteral("xl/persons/person.xml")).contains("displayName=\"Dee\""));
        const QByteArray older = parts.value(QStringLiteral("xl/comments1.xml"));
        QVERIFY(older.contains("tc={11111111-2222-3333-4444-555555555555}"));
        QVERIFY(older.contains("Reply:"));
        QCOMPARE(older.count("ref=\"B5\""), 1);
        QVERIFY(parts.value(QStringLiteral("xl/threadedComments/threadedComment1.xml")).contains("done=\"1\""));
        // the notes beside it are as they were
        QCOMPARE(readSheetNoteInfos(again->book, 0).at(noteKey(0, 0)).text, readSheetNoteInfos(o->book, 0).at(noteKey(0, 0)).text);
    }

    void aThreadOnASheetThatHadNoneMakesItsPartsAndTheirLinks()
    {
        auto o = open(plainBook());
        QVERIFY(o->ok);
        const ThreadList list = {comment(QStringLiteral("{A0000000-0000-0000-0000-000000000001}"), QString(), QStringLiteral("{C0000000-0000-0000-0000-000000000001}"), QStringLiteral("Gus"), QStringLiteral("First"))};
        SaveChanges changes;
        changes.sheets[0].noteOps = {putThread(0, 0, list)};
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        const auto parts = entriesOf(again->bytes);
        QVERIFY(parts.contains(QStringLiteral("xl/threadedComments/threadedComment1.xml")));
        QVERIFY(parts.contains(QStringLiteral("xl/persons/person.xml")));
        const QByteArray types = parts.value(QStringLiteral("[Content_Types].xml"));
        QVERIFY(types.contains("application/vnd.ms-excel.threadedcomments+xml"));
        QVERIFY(types.contains("application/vnd.ms-excel.person+xml"));
        QVERIFY(parts.value(QStringLiteral("xl/_rels/workbook.xml.rels")).contains("relationships/person\""));
        QVERIFY(parts.value(QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")).contains("relationships/threadedComment\""));
        const NoteInfo n = readSheetNoteInfos(again->book, 0).at(noteKey(0, 0));
        QVERIFY(n.threaded);
        QCOMPARE(*n.thread, list);
    }

    void aThreadTurnedIntoANoteLeavesItsPartAndKeepsTheOthers()
    {
        auto o = open(notesBook());
        QVERIFY(o->ok);
        SaveChanges changes;
        changes.sheets[0].noteOps = {put(4, 1, QStringLiteral("Dee"), QStringLiteral("plain now"))};
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        const auto notes = readSheetNoteInfos(again->book, 0);
        QVERIFY(!notes.at(noteKey(4, 1)).threaded);
        QCOMPARE(notes.at(noteKey(4, 1)).text, QStringLiteral("plain now"));
        QVERIFY(!entriesOf(again->bytes).value(QStringLiteral("xl/threadedComments/threadedComment1.xml")).contains("ref=\"B5\""));
    }

    void aThreadTakenAwayLeavesNeitherThreadNorNote()
    {
        auto o = open(notesBook());
        QVERIFY(o->ok);
        SaveChanges changes;
        changes.sheets[0].noteOps = {drop(4, 1)};
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        QVERIFY(readSheetNoteInfos(again->book, 0).count(noteKey(4, 1)) == 0);
        QVERIFY(!entriesOf(again->bytes).value(QStringLiteral("xl/threadedComments/threadedComment1.xml")).contains("ref=\"B5\""));
    }

    void whoIsMentionedInACommentIsKeptWhenItsThreadIsSavedAgain()
    {
        QByteArray threads = kThreads;
        threads.replace("<text>Is this figure final?</text>", "<text>@Eli is this figure final?</text><mentions><mention mentionpersonId=\"{AAAAAAAA-0000-0000-0000-000000000002}\" mentionId=\"{99999999-0000-0000-0000-000000000001}\" startIndex=\"0\" length=\"4\"/></mentions>");
        auto o = open(notesBook(true, threads));
        QVERIFY(o->ok);
        const auto before = readSheetNoteInfos(o->book, 0);
        QVERIFY(before.at(noteKey(4, 1)).thread);
        ThreadList list = *before.at(noteKey(4, 1)).thread;
        QVERIFY(list[0].extra.contains(QStringLiteral("mentionpersonId")));
        QCOMPARE(list[0].text, QStringLiteral("@Eli is this figure final?"));
        list.push_back(comment(QStringLiteral("{77777777-0000-0000-0000-000000000002}"), list[0].id, list[0].personId, list[0].author, QStringLiteral("Thanks")));
        SaveChanges changes;
        changes.sheets[0].noteOps = {putThread(4, 1, list)};
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        QVERIFY2(entriesOf(again->bytes).value(QStringLiteral("xl/threadedComments/threadedComment1.xml")).contains("mentionpersonId"), entriesOf(again->bytes).value(QStringLiteral("xl/threadedComments/threadedComment1.xml")).constData());
        const NoteInfo n = readSheetNoteInfos(again->book, 0).at(noteKey(4, 1));
        QVERIFY(n.thread);
        QCOMPARE(*n.thread, list);
    }

    void aCommentThatHasNoTimeOrPersonInTheFileIsWrittenWithoutThemAndAPersonKeepsOnlyWhatItHad()
    {
        // (a thread from a program that writes neither a time nor a user id: nothing is made up for it)
        QByteArray threads = "<ThreadedComments xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\"><threadedComment ref=\"B5\" personId=\"{AAAAAAAA-0000-0000-0000-000000000003}\" id=\"{11111111-2222-3333-4444-555555555555}\" done=\"0\">"
                             "<text xml:space=\"preserve\">no time\n</text></threadedComment></ThreadedComments>";
        QByteArray persons = "<personList xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\"><person displayName=\"Gil\" id=\"{AAAAAAAA-0000-0000-0000-000000000003}\"/></personList>";
        QMap<QString, QByteArray> all = entriesOf(notesBook(true, threads));
        all[QStringLiteral("xl/persons/person.xml")] = persons;
        auto o = open(zipOf(all));
        QVERIFY(o->ok);
        const auto before = readSheetNoteInfos(o->book, 0);
        QVERIFY(before.at(noteKey(4, 1)).thread);
        ThreadList list = *before.at(noteKey(4, 1)).thread;
        QVERIFY(list[0].time.isEmpty() && list[0].userId.isEmpty());
        list.push_back(comment(QStringLiteral("{77777777-0000-0000-0000-000000000003}"), list[0].id, list[0].personId, list[0].author, QStringLiteral("reply")));
        SaveChanges changes;
        changes.sheets[0].noteOps = {putThread(4, 1, list)};
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        const auto parts = entriesOf(again->bytes);
        const QByteArray written = parts.value(QStringLiteral("xl/threadedComments/threadedComment1.xml"));
        QVERIFY2(!written.contains("dT=\"\""), written.constData());
        QVERIFY(written.contains("id=\"{11111111-2222-3333-4444-555555555555}\"") && !written.contains("personId=\"\""));
        QCOMPARE(parts.value(QStringLiteral("xl/persons/person.xml")), persons);   // (the person was in the list: the list is as it was)
        // a person who is new has the ids it was given
        list.push_back(comment(QStringLiteral("{77777777-0000-0000-0000-000000000004}"), list[0].id, QStringLiteral("{C0000000-0000-0000-0000-000000000009}"), QStringLiteral("Hal"), QStringLiteral("hi")));
        list.back().userId.clear();
        list.back().providerId.clear();
        changes.sheets[0].noteOps = {putThread(4, 1, list)};
        auto third = saved(*o, changes, &why);
        QVERIFY2(third, qPrintable(why));
        const QByteArray people = entriesOf(third->bytes).value(QStringLiteral("xl/persons/person.xml"));
        QVERIFY(people.contains("displayName=\"Hal\"") && !people.contains("userId=\"\"") && !people.contains("providerId=\"\""));
    }

    void bigExtrasAreChargedToTheBooksBudgetSoThePerBookTotalHolds()
    {
        // 25 cells of 480 comments, each with some 3,300 characters of mentions: the extras alone are more than the book may read
        QByteArray extra = "<mentions>";
        for (int i = 0; i < 22; ++i) extra += "<mention mentionpersonId=\"{AAAAAAAA-0000-0000-0000-000000000001}\" mentionId=\"{99999999-0000-0000-0000-00000000" + QByteArray::number(1000 + i) + "}\" startIndex=\"0\" length=\"1\"/>";
        extra += "</mentions>";
        QByteArray threads = "<ThreadedComments xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\">";
        for (int cell = 1; cell <= 25; ++cell)
            for (int i = 0; i < 480; ++i)
                threads += "<threadedComment ref=\"A" + QByteArray::number(cell) + "\" id=\"{C" + QByteArray::number(cell) + "-" + QByteArray::number(i) + "}\"><text>x</text>" + extra + "</threadedComment>";
        threads += "</ThreadedComments>";
        auto o = open(notesBook(true, threads));
        QVERIFY(o->ok);
        NoteReadContext ctx;
        bool truncated = false;
        const auto notes = readSheetNoteInfos(o->book, 0, &truncated, kMaxNotes, &ctx);
        qint64 comments = 0;
        for (const auto &n : notes)
            if (n.second.thread) comments += qint64(n.second.thread->size());
        QVERIFY(truncated);
        QVERIFY(ctx.exhausted);
        QVERIFY(comments < 25 * 480);
        QVERIFY2(ctx.chars <= kMaxNoteCharsPerBook + 5000, qPrintable(QString::number(ctx.chars)));
    }

    void aThreadPartThatTwentyFiveSheetsNameIsParsedOnce()
    {
        QVector<QByteArray> sheets;
        for (int i = 0; i < 25; ++i) sheets.push_back(worksheet(kRows));
        QVector<ExtraPart> parts;
        parts.push_back({QStringLiteral("xl/threadedComments/threadedComment1.xml"), QByteArray(kThreads), QStringLiteral("application/vnd.ms-excel.threadedcomments+xml"), -1, QString(), QString(), QString()});
        QMap<QString, QByteArray> all = entriesOf(withParts(buildXlsx(sheets), parts));
        for (int i = 1; i <= 25; ++i)
            all[QStringLiteral("xl/worksheets/_rels/sheet%1.xml.rels").arg(i)] =
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"><Relationship Id=\"rId1\" Type=\"http://schemas.microsoft.com/office/2017/10/relationships/threadedComment\" Target=\"../threadedComments/threadedComment1.xml\"/></Relationships>";
        auto o = open(zipOf(all));
        QVERIFY(o->ok);
        NoteReadContext ctx;
        const qint64 before = threadPartReads();
        for (int i = 0; i < 25; ++i) {
            const auto notes = readSheetNoteInfos(o->book, i, nullptr, kMaxNotes, &ctx);
            QVERIFY2(notes.count(noteKey(4, 1)) && notes.at(noteKey(4, 1)).thread && notes.at(noteKey(4, 1)).thread->size() == 2, qPrintable(QString::number(i)));
        }
        QCOMPARE(threadPartReads() - before, qint64(1));
    }

    void aPersonThatTheListLacksIsNotAddedAsABlankOne()
    {
        const ThreadList list = {comment(QStringLiteral("{A0000000-0000-0000-0000-000000000001}"), QString(), QStringLiteral("{GONE}"), QString(), QStringLiteral("whose?"))};
        SaveChanges changes;
        changes.sheets[0].noteOps = {putThread(0, 0, list)};
        QString why;
        // a workbook with no list of people is not given one for a person it has no name for
        auto plain = open(plainBook());
        QVERIFY(plain->ok);
        auto fresh = saved(*plain, changes, &why);
        QVERIFY2(fresh, qPrintable(why));
        QVERIFY(!entriesOf(fresh->bytes).contains(QStringLiteral("xl/persons/person.xml")));
        // and one that has a list keeps it as it is
        auto o = open(notesBook());
        QVERIFY(o->ok);
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        QCOMPARE(entriesOf(again->bytes).value(QStringLiteral("xl/persons/person.xml")), QByteArray(kPersons));
    }

    void theOlderNoteForAThreadIsCappedAndTheThreadsWordsStayUnderTheLimit()
    {
        ThreadList list;
        for (int i = 0; i < 3; ++i) list.push_back(comment(QStringLiteral("{B%1}").arg(i), i ? list[0].id : QString(), QStringLiteral("{P}"), QStringLiteral("Ann"), QString(30000, QLatin1Char('x'))));
        const QString legacy = legacyThreadText(list);
        QVERIFY2(legacy.size() <= 32767, qPrintable(QString::number(legacy.size())));
        QVERIFY(legacy.endsWith(QChar(0x2026)));
        // a thread of the most comments, each of the most characters: its words and its older note are held to the limits
        ThreadList big;
        for (int i = 0; i < 300; ++i) big.push_back(comment(QStringLiteral("{C%1}").arg(i), i ? big[0].id : QString(), QStringLiteral("{P}"), QStringLiteral("Ann"), QString(kMaxThreadCommentChars, QLatin1Char('y'))));
        auto o = open(plainBook());
        QVERIFY(o->ok);
        SaveChanges changes;
        changes.sheets[0].noteOps = {putThread(0, 0, big)};
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        const NoteInfo n = readSheetNoteInfos(again->book, 0).at(noteKey(0, 0));
        QVERIFY(n.thread);
        QCOMPARE(n.thread->size(), big.size());
        QVERIFY2(n.text.size() <= kMaxNoteChars, qPrintable(QString::number(n.text.size())));
        QVERIFY(entriesOf(again->bytes).value(QStringLiteral("xl/comments1.xml")).size() < 100000);
    }

    void aThreadPartWithAPrefixedRootIsChangedThroughTheWriter()
    {
        QByteArray threads = "<tc:ThreadedComments xmlns:tc=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\"><tc:threadedComment ref=\"B5\" dT=\"2024-01-01T10:00:00.00\" personId=\"{AAAAAAAA-0000-0000-0000-000000000001}\" id=\"{11111111-2222-3333-4444-555555555555}\">"
                             "<tc:text>Is this figure final?</tc:text></tc:threadedComment></tc:ThreadedComments>";
        auto o = open(notesBook(true, threads));
        QVERIFY(o->ok);
        const auto before = readSheetNoteInfos(o->book, 0);
        QVERIFY(before.at(noteKey(4, 1)).thread);
        ThreadList list = *before.at(noteKey(4, 1)).thread;
        list.push_back(comment(QStringLiteral("{77777777-0000-0000-0000-000000000005}"), list[0].id, list[0].personId, list[0].author, QStringLiteral("Yes")));
        SaveChanges changes;
        changes.sheets[0].noteOps = {putThread(4, 1, list)};
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        const QByteArray part = entriesOf(again->bytes).value(QStringLiteral("xl/threadedComments/threadedComment1.xml"));
        QVERIFY2(part.contains("<tc:threadedComment ") && !part.contains("<threadedComment "), part.constData());
        const NoteInfo n = readSheetNoteInfos(again->book, 0).at(noteKey(4, 1));
        QVERIFY(n.thread);
        QCOMPARE(*n.thread, list);
    }

    void theCapsOnCommentsInABookAndOnAnExtraAreHit()
    {
        // 1,000 cells of 401 comments: 401,000, one more thousand than a book is read with
        QByteArray threads = "<ThreadedComments xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\">";
        for (int cell = 1; cell <= 1000; ++cell)
            for (int i = 0; i < 401; ++i) threads += "<threadedComment ref=\"A" + QByteArray::number(cell) + "\" id=\"{" + QByteArray::number(i) + "}\"><text>x</text></threadedComment>";
        threads += "</ThreadedComments>";
        auto o = open(notesBook(true, threads));
        QVERIFY(o->ok);
        bool truncated = false;
        const auto notes = readSheetNoteInfos(o->book, 0, &truncated);
        qint64 comments = 0;
        for (const auto &n : notes)
            if (n.second.thread) comments += qint64(n.second.thread->size());
        QVERIFY(truncated);
        QCOMPARE(comments, kMaxThreadCommentsPerBook);
        // an extra of more than 4,000 characters is let go of, and one of fewer is kept
        auto mentions = [](int count) {
            QByteArray x = "<mentions>";
            for (int i = 0; i < count; ++i) x += "<mention mentionpersonId=\"{AAAAAAAA-0000-0000-0000-000000000001}\" mentionId=\"{" + QByteArray::number(i) + "}\" startIndex=\"0\" length=\"1\"/>";
            return x + "</mentions>";
        };
        QByteArray two = "<ThreadedComments xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\"><threadedComment ref=\"B5\" id=\"{S}\"><text>small</text>" + mentions(3)
                         + "</threadedComment><threadedComment ref=\"B5\" id=\"{L}\"><text>large</text>" + mentions(60) + "</threadedComment></ThreadedComments>";
        auto p = open(notesBook(true, two));
        QVERIFY(p->ok);
        const NoteInfo n = readSheetNoteInfos(p->book, 0).at(noteKey(4, 1));
        QVERIFY(n.thread && n.thread->size() == 2);
        QVERIFY(!n.thread->at(0).extra.isEmpty());
        QVERIFY(n.thread->at(1).extra.isEmpty());
    }

    void twoThreadPartsOnOneSheetAreChangedTogetherAndNoCommentIsTwice()
    {
        QMap<QString, QByteArray> all = entriesOf(notesBook());
        all[QStringLiteral("xl/threadedComments/threadedComment2.xml")] =
            "<ThreadedComments xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\"><threadedComment ref=\"C3\" dT=\"2024-02-01T10:00:00.00\" personId=\"{AAAAAAAA-0000-0000-0000-000000000001}\" id=\"{22222222-2222-3333-4444-555555555555}\"><text>Second part</text></threadedComment></ThreadedComments>";
        QByteArray types = all.value(QStringLiteral("[Content_Types].xml"));
        types.replace("</Types>", "<Override PartName=\"/xl/threadedComments/threadedComment2.xml\" ContentType=\"application/vnd.ms-excel.threadedcomments+xml\"/></Types>");
        all[QStringLiteral("[Content_Types].xml")] = types;
        QByteArray rels = all.value(QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels"));
        rels.replace("</Relationships>", "<Relationship Id=\"rId9\" Type=\"http://schemas.microsoft.com/office/2017/10/relationships/threadedComment\" Target=\"../threadedComments/threadedComment2.xml\"/></Relationships>");
        all[QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")] = rels;
        auto o = open(zipOf(all));
        QVERIFY(o->ok);
        const auto before = readSheetNoteInfos(o->book, 0);
        QVERIFY(before.at(noteKey(2, 2)).thread && before.at(noteKey(4, 1)).thread);
        ThreadList first = *before.at(noteKey(4, 1)).thread, second = *before.at(noteKey(2, 2)).thread;
        first.push_back(comment(QStringLiteral("{77777777-0000-0000-0000-000000000006}"), first[0].id, first[0].personId, first[0].author, QStringLiteral("one")));
        second.push_back(comment(QStringLiteral("{77777777-0000-0000-0000-000000000007}"), second[0].id, second[0].personId, second[0].author, QStringLiteral("two")));
        SaveChanges changes;
        changes.sheets[0].noteOps = {putThread(4, 1, first), putThread(2, 2, second)};
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        const auto notes = readSheetNoteInfos(again->book, 0);
        QCOMPARE(*notes.at(noteKey(4, 1)).thread, first);
        QCOMPARE(*notes.at(noteKey(2, 2)).thread, second);
    }

    void aNotesChangeLeavesUnrelatedPartsByteForByte()
    {
        auto o = open(notesBook());
        QVERIFY(o->ok);
        SaveChanges changes;
        changes.sheets[0].noteOps = {put(1, 1, QStringLiteral("Ann"), QStringLiteral("x"))};
        QByteArray out;
        QString why;
        QVERIFY2(saveXlsx(o->book, changes, &out, &why), qPrintable(why));
        const auto before = entriesOf(o->bytes), after = entriesOf(out);
        for (const QString &name : {QStringLiteral("xl/workbook.xml"), QStringLiteral("xl/threadedComments/threadedComment1.xml"), QStringLiteral("xl/persons/person.xml"), QStringLiteral("xl/styles.xml")})
            if (before.contains(name)) QCOMPARE(after.value(name), before.value(name));
    }

    void tooManyChangesToNotesAreRefusedWithAMessage()
    {
        std::vector<NoteOp> ops;
        for (qint64 i = 0; i <= kMaxNoteOps; ++i) ops.push_back(put(quint32(i), 0, QStringLiteral("a"), QStringLiteral("t")));
        QByteArray out;
        QString why;
        QVERIFY(!applyNoteOpsToComments(QByteArray(kComments), ops, &out, &why));
        QVERIFY(!why.isEmpty());
        why.clear();
        QVERIFY(!applyNoteOpsToDrawing(QByteArray(kVml), ops, &out, &why));
        QVERIFY(!why.isEmpty());
    }

    void aDamagedNotesPartIsRefusedNotWrittenOver()
    {
        QByteArray out;
        QString why;
        QVERIFY(!applyNoteOpsToComments(QByteArray("<comments><authors><author>A</author></authors><commentList><comment ref=\"A1\" authorId=\"0\"><text><t>cut"), {put(0, 0, QStringLiteral("A"), QStringLiteral("x"))}, &out, &why));
        QVERIFY(!why.isEmpty());
    }

    void shownCanBeSwitchedInTheDrawing()
    {
        QByteArray out;
        QString why;
        // A1 (hidden) shown; C3 (shown) hidden; B5 left
        QVERIFY2(applyNoteOpsToDrawing(QByteArray(kVml), {put(0, 0, QStringLiteral("Ann"), QStringLiteral("t"), true), put(2, 2, QStringLiteral("Bob"), QStringLiteral("t"), false)}, &out, &why), qPrintable(why));
        const auto shapes = noteShapes(out);
        QVERIFY(shapes.at(noteKey(0, 0)));
        QVERIFY(!shapes.at(noteKey(2, 2)));
        QVERIFY(!shapes.at(noteKey(4, 1)));
        QCOMPARE(out.count("<x:Visible/>"), 1);
    }

    // ---- links

    void linksAreReadWithTheirAddressesAndPlaces()
    {
        QByteArray book = withParts(plainBook(QStringLiteral("<hyperlinks><hyperlink ref=\"A1\" r:id=\"rId7\" tooltip=\"Go there\" display=\"Home\"/><hyperlink ref=\"B1:C2\" location=\"Sheet1!C3\"/>"
                                                             "<hyperlink ref=\"not a ref\" location=\"X!A1\"/></hyperlinks>")),
                                    {});
        QMap<QString, QByteArray> all = entriesOf(book);
        all[QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")] = "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"><Relationship Id=\"rId7\" "
                                                                     "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink\" Target=\"https://example.org/a?b=1&amp;c=2\" TargetMode=\"External\"/></Relationships>";
        auto o = open(zipOf(all));
        QVERIFY2(o->ok, qPrintable(o->why));
        Sheet sheet;
        QVERIFY(o->book.readSheet(0, &sheet));
        QCOMPARE(sheet.links.size(), 2);
        QCOMPARE(sheet.links[0].target, QStringLiteral("https://example.org/a?b=1&c=2"));
        QCOMPARE(sheet.links[0].tip, QStringLiteral("Go there"));
        QCOMPARE(sheet.links[0].display, QStringLiteral("Home"));
        QVERIFY(sheet.links[0].range.contains(0, 0));
        QVERIFY(!sheet.links[1].outside());
        QCOMPARE(sheet.links[1].location, QStringLiteral("Sheet1!C3"));
        QCOMPARE(rangeName(sheet.links[1].range), QStringLiteral("B1:C2"));
    }

    // The links of all the sheets of a workbook are held to one budget of bytes, so that many sheets with many long addresses can't hold more than a workbook should.
    void theLinksOfAllTheSheetsShareOneBudget()
    {
        QString links = QStringLiteral("<hyperlinks>");
        for (int i = 1; i <= 50; ++i) links += QStringLiteral("<hyperlink ref=\"A%1\" location=\"Sheet1!%2\" tooltip=\"%3\"/>").arg(i).arg(QString(100, QLatin1Char('Z')), QString(100, QLatin1Char('t')));
        links += QStringLiteral("</hyperlinks>");
        Package package;
        const QByteArray bytes = buildXlsx({worksheet(kRows, links), worksheet(kRows, links)});
        QString why;
        QVERIFY(package.readLazy(bytes, &why));
        Limits limits;
        limits.linkBytes = 25000;   // (a link here is about 450 bytes: 50 of them are 22,500)
        Workbook book;
        QVERIFY2(book.open(package, &why, limits), qPrintable(why));
        Sheet a, b;
        QVERIFY(book.readSheet(0, &a));
        QCOMPARE(a.links.size(), 50);
        QVERIFY(book.readSheet(1, &b));
        QVERIFY2(b.links.size() < 50 && b.links.size() > 0, qPrintable(QString::number(b.links.size())));
        // a sheet read again is not counted twice
        Sheet again;
        QVERIFY(book.readSheet(0, &again));
        QVERIFY2(again.links.size() >= 40, qPrintable(QString::number(again.links.size())));
    }

    void linksAreSavedTakenOutAndPutIn()
    {
        QMap<QString, QByteArray> all = entriesOf(withParts(plainBook(QStringLiteral("<hyperlinks><hyperlink ref=\"A1\" r:id=\"rId7\"/><hyperlink ref=\"B1\" location=\"Sheet1!C3\"/></hyperlinks><pageMargins left=\"0.7\" right=\"0.7\" top=\"0.75\" bottom=\"0.75\" header=\"0.3\" footer=\"0.3\"/>")), {}));
        all[QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")] = "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"><Relationship Id=\"rId7\" "
                                                                     "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink\" Target=\"https://example.org/a\" TargetMode=\"External\"/></Relationships>";
        auto o = open(zipOf(all));
        QVERIFY2(o->ok, qPrintable(o->why));
        SaveChanges changes;
        SheetChanges &sc = changes.sheets[0];
        CellRange b1;
        QVERIFY(parseRangeRef(QStringLiteral("B1"), &b1));
        sc.unlinks = {b1};
        LinkSpec same, mail, place;
        parseRangeRef(QStringLiteral("D4"), &same.range);
        same.target = QStringLiteral("https://example.org/a");   // (an address the sheet already has a relationship for)
        parseRangeRef(QStringLiteral("D5"), &mail.range);
        mail.target = QStringLiteral("mailto:a@example.org?subject=hi");
        mail.tip = QStringLiteral("Write to A");
        parseRangeRef(QStringLiteral("E1:E3"), &place.range);
        place.location = QStringLiteral("Sheet1!A1");
        sc.links = {same, mail, place};
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        Sheet sheet;
        QVERIFY(again->book.readSheet(0, &sheet));
        QCOMPARE(sheet.links.size(), 4);
        QCOMPARE(sheet.links[0].target, QStringLiteral("https://example.org/a"));   // A1, as it was
        QCOMPARE(rangeName(sheet.links[1].range), QStringLiteral("D4"));
        QCOMPARE(sheet.links[1].target, QStringLiteral("https://example.org/a"));
        QCOMPARE(sheet.links[2].target, QStringLiteral("mailto:a@example.org?subject=hi"));
        QCOMPARE(sheet.links[2].tip, QStringLiteral("Write to A"));
        QCOMPARE(sheet.links[3].location, QStringLiteral("Sheet1!A1"));
        // B1's link is gone, and the element is before the page margins, as the format has it
        const QByteArray xml = entriesOf(again->bytes).value(QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY(!xml.contains("ref=\"B1\""));
        QVERIFY(xml.indexOf("<hyperlinks>") < xml.indexOf("<pageMargins"));
        // the address both links name is one relationship, and the mail address another
        QCOMPARE(hyperlinkRels(again->package, QStringLiteral("/xl/worksheets/sheet1.xml")).size(), 2);
    }

    void takingOutTheLastLinkTakesOutTheElement()
    {
        QMap<QString, QByteArray> all = entriesOf(plainBook(QStringLiteral("<hyperlinks><hyperlink ref=\"B1\" location=\"Sheet1!C3\"/></hyperlinks>")));
        auto o = open(zipOf(all));
        QVERIFY(o->ok);
        SaveChanges changes;
        CellRange b1;
        parseRangeRef(QStringLiteral("B1"), &b1);
        changes.sheets[0].unlinks = {b1};
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        QVERIFY(!entriesOf(again->bytes).value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("hyperlinks"));
    }

    void linkKindsSayWhatMayBeOpened()
    {
        QCOMPARE(targetKind(QStringLiteral("https://example.org")), LinkKind::Web);
        QCOMPARE(targetKind(QStringLiteral("HTTP://example.org")), LinkKind::Web);
        QCOMPARE(targetKind(QStringLiteral("mailto:a@b.c")), LinkKind::Mail);
        QCOMPARE(targetKind(QStringLiteral("file:///tmp/a.txt")), LinkKind::File);
        QCOMPARE(targetKind(QStringLiteral("C:\\Users\\a\\b.xlsx")), LinkKind::File);
        QCOMPARE(targetKind(QStringLiteral("..\\other.xlsx")), LinkKind::File);
        QCOMPARE(targetKind(QStringLiteral("ftp://example.org/x")), LinkKind::Other);
        QCOMPARE(targetKind(QStringLiteral("tel:+15551234")), LinkKind::Other);
        QCOMPARE(targetKind(QStringLiteral("javascript:alert(1)")), LinkKind::Unsafe);
        QCOMPARE(targetKind(QStringLiteral("  JavaScript:alert(1)")), LinkKind::Unsafe);
        QCOMPARE(targetKind(QStringLiteral("vbscript:x")), LinkKind::Unsafe);
        QCOMPARE(targetKind(QStringLiteral("data:text/html,<b>")), LinkKind::Unsafe);
        // a program, a script, or a shortcut is never opened, however the name is dressed
        for (const char *name : {"C:\\Tools\\run.EXE", "file:///tmp/a.sh", "report.pdf.exe", "x.exe?arg=1", "x.exe ", "x.exe...", "x.ex%65", "\\\\host\\share\\setup.msi", "a.lnk", "b.ps1#frag", "c.vbs", "d.js", "e.scr", "f.cmd",
                                 "g.com", "h.bat\"", "doc.txt:evil.exe", "k.hta", "m.jar"})
            QVERIFY2(targetKind(QString::fromLatin1(name)) == LinkKind::Unsafe, name);
        QCOMPARE(targetKind(QStringLiteral("data/report.pdf")), LinkKind::File);
        QCOMPARE(targetKind(QStringLiteral("notes.exe.txt")), LinkKind::File);
        QCOMPARE(targetKind(QStringLiteral("C:\\Data\\book.xlsx")), LinkKind::File);
        // a long address is shown with both ends
        const QString longOne = QStringLiteral("https://example.org/") + QString(500, QLatin1Char('q')) + QStringLiteral("/end.pdf");
        const QString shortOne = elidedMiddle(longOne, 100);
        QCOMPARE(shortOne.size(), 100);
        QVERIFY(shortOne.startsWith(QStringLiteral("https://example.org/")) && shortOne.endsWith(QStringLiteral("/end.pdf")) && shortOne.contains(QStringLiteral("...")));
        QCOMPARE(elidedMiddle(QStringLiteral("short"), 100), QStringLiteral("short"));
        LinkSpec place;
        place.location = QStringLiteral("Sheet2!A1");
        QCOMPARE(linkKind(place), LinkKind::Place);
        QCOMPARE(webAddressFromTyped(QStringLiteral("  www.example.org ")), QStringLiteral("https://www.example.org"));
        QCOMPARE(webAddressFromTyped(QStringLiteral("a@example.org")), QStringLiteral("mailto:a@example.org"));
        QCOMPARE(webAddressFromTyped(QStringLiteral("http://x.org")), QStringLiteral("http://x.org"));
        QVERIFY(webAddressFromTyped(QStringLiteral("   ")).isEmpty());
    }

    // ---- protection

    void aSheetsProtectionIsReadWithWhatItAllows()
    {
        auto o = open(buildXlsx({worksheet(QString(), QStringLiteral("<sheetProtection algorithmName=\"SHA-512\" hashValue=\"AAAA\" saltValue=\"BBBB\" spinCount=\"100000\" sheet=\"1\" objects=\"1\" scenarios=\"1\" "
                                                                      "formatCells=\"0\" insertRows=\"0\" sort=\"0\" selectLockedCells=\"1\"/>"))}));
        QVERIFY(o->ok);
        Sheet sheet;
        QVERIFY(o->book.readSheet(0, &sheet));
        const SheetProtection &p = sheet.protection;
        QVERIFY(p.enabled);
        QVERIFY(sheet.isProtected);
        QVERIFY(p.allowFormatCells && p.allowInsertRows && p.allowSort);
        QVERIFY(!p.allowFormatColumns && !p.allowDeleteRows && !p.allowFilter && !p.allowInsertLinks);
        QVERIFY(!p.allowSelectLocked);
        QVERIFY(p.allowSelectUnlocked);
        QCOMPARE(p.password.algorithm, QStringLiteral("SHA-512"));
        QCOMPARE(p.password.hash, QStringLiteral("AAAA"));
        QCOMPARE(p.password.salt, QStringLiteral("BBBB"));
        QCOMPARE(p.password.spin, qint64(100000));
    }

    void aSheetWithOnlyTheSheetAttributeAllowsNothingButSelecting()
    {
        auto o = open(buildXlsx({worksheet(QString(), QStringLiteral("<sheetProtection sheet=\"1\"/>"))}));
        Sheet sheet;
        QVERIFY(o->book.readSheet(0, &sheet));
        const SheetProtection &p = sheet.protection;
        QVERIFY(p.enabled && p.allowSelectLocked && p.allowSelectUnlocked);
        QVERIFY(!p.allowFormatCells && !p.allowInsertRows && !p.allowDeleteColumns && !p.allowSort && !p.allowFilter && !p.allowPivot);
        QVERIFY(!p.password.isSet());
    }

    void protectionIsSavedAndReadBackWithItsHash()
    {
        auto o = open(plainBook(QStringLiteral("<sheetProtection sheet=\"1\" password=\"CF1A\"/><mergeCells count=\"1\"><mergeCell ref=\"A1:B1\"/></mergeCells>")));
        QVERIFY(o->ok);
        SheetProtection p;
        p.enabled = true;
        p.allowSort = true;
        p.allowFormatRows = true;
        p.allowSelectUnlocked = false;
        PasswordHash h;
        h.algorithm = QStringLiteral("SHA-512");
        h.hash = QStringLiteral("dE0/gpjHqvlLhLV/djxJ4l2pulkbVaLyRxlEONXVgsBucIoSd+xMxowHOOhChksR4N0TVLKM07EZmWHrP1Mpbw==");
        h.salt = QStringLiteral("MDEyMzQ1Njc4OWFiY2RlZg==");
        h.spin = 5;
        p.password = h;
        SaveChanges changes;
        changes.sheets[0].setProtection = true;
        changes.sheets[0].protection = p;
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        Sheet sheet;
        QVERIFY(again->book.readSheet(0, &sheet));
        QVERIFY(sheet.protection == p);
        QCOMPARE(entriesOf(again->bytes).value(QStringLiteral("xl/worksheets/sheet1.xml")).count("<sheetProtection"), 1);
        // before the merged ranges, as the format has it
        const QByteArray xml = entriesOf(again->bytes).value(QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY(xml.indexOf("<sheetProtection") < xml.indexOf("<mergeCells"));
        QVERIFY(!xml.contains("password=\"CF1A\""));
        // taking the protection off takes the element away
        SaveChanges off;
        off.sheets[0].setProtection = true;
        auto cleared = saved(*again, off, &why);
        QVERIFY2(cleared, qPrintable(why));
        QVERIFY(!entriesOf(cleared->bytes).value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("sheetProtection"));
        Sheet none;
        QVERIFY(cleared->book.readSheet(0, &none));
        QVERIFY(!none.isProtected);
    }

    void aSheetGetsProtectionWhereTheFileHadNone()
    {
        auto o = open(plainBook(QStringLiteral("<pageMargins left=\"0.7\" right=\"0.7\" top=\"0.75\" bottom=\"0.75\" header=\"0.3\" footer=\"0.3\"/>")));
        SaveChanges changes;
        changes.sheets[0].setProtection = true;
        changes.sheets[0].protection.enabled = true;
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        const QByteArray xml = entriesOf(again->bytes).value(QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY(xml.indexOf("</sheetData>") < xml.indexOf("<sheetProtection"));
        QVERIFY(xml.indexOf("<sheetProtection") < xml.indexOf("<pageMargins"));
        Sheet sheet;
        QVERIFY(again->book.readSheet(0, &sheet));
        QVERIFY(sheet.isProtected);
    }

    void theOlderPasswordFormIsCheckedAndNeverWritten()
    {
        // known values of the older 16-bit form
        QCOMPARE(legacyPasswordHash(QStringLiteral("password")), QStringLiteral("83AF"));
        QCOMPARE(legacyPasswordHash(QStringLiteral("test")), QStringLiteral("CBEB"));
        PasswordHash h;
        h.legacy = QStringLiteral("83AF");
        QCOMPARE(checkPassword(h, QStringLiteral("password")), PasswordResult::Match);
        QCOMPARE(checkPassword(h, QStringLiteral("Password")), PasswordResult::Wrong);
        QCOMPARE(checkPassword(h, QString()), PasswordResult::Wrong);
        // only the first 15 characters count, as in the format, and a very long password shifts nothing past a place a number has
        QCOMPARE(legacyPasswordHash(QStringLiteral("0123456789abcdefghijklmnopqrstuvwxyz")), legacyPasswordHash(QStringLiteral("0123456789abcde")));
        QCOMPARE(checkPassword(h, QString(100000, QChar(0xFFFF))), PasswordResult::Wrong);
        PasswordHash longOne;
        longOne.legacy = legacyPasswordHash(QStringLiteral("0123456789abcdeXYZ"));
        QCOMPARE(checkPassword(longOne, QStringLiteral("0123456789abcdeQQQ")), PasswordResult::Match);
        // no password at all matches anything (nothing is asked)
        QCOMPARE(checkPassword(PasswordHash(), QStringLiteral("x")), PasswordResult::Match);
    }

    void theStrongPasswordFormMatchesAnIndependentlyMadeHash()
    {
        PasswordHash h;
        h.algorithm = QStringLiteral("SHA-512");
        h.salt = QStringLiteral("MDEyMzQ1Njc4OWFiY2RlZg==");
        h.hash = QStringLiteral("dE0/gpjHqvlLhLV/djxJ4l2pulkbVaLyRxlEONXVgsBucIoSd+xMxowHOOhChksR4N0TVLKM07EZmWHrP1Mpbw==");
        h.spin = 5;
        QCOMPARE(checkPassword(h, QStringLiteral("secret")), PasswordResult::Match);
        QCOMPARE(checkPassword(h, QStringLiteral("Secret")), PasswordResult::Wrong);
        // no repeats: the first hash only
        h.spin = 0;
        h.hash = QStringLiteral("7LSV15kk27nFgOiiFDbh+3tDXLPDtmWJ99Bmy24qQ5LWJPmLgcwIUTgfc1a+U0PZoz+ysO20rqXFjTXBsSrRWw==");
        QCOMPARE(checkPassword(h, QStringLiteral("secret")), PasswordResult::Match);
        // other algorithms, and letters outside ASCII
        PasswordHash s;
        s.algorithm = QStringLiteral("SHA-256");
        s.salt = QStringLiteral("MDEyMzQ1Njc4OWFiY2RlZg==");
        s.hash = QStringLiteral("4m+ywdJ+o3DOMoDNVF0Mp8vPCETqH660IGwmodbXkUk=");
        s.spin = 3;
        QCOMPARE(checkPassword(s, QString::fromUtf8("P\xc3\xa4ssw\xc3\xb6rd")), PasswordResult::Match);
    }

    void aNewPasswordIsHashedWithARandomSaltAndNeverKeptAsText()
    {
        PasswordHash a, b;
        QVERIFY(makePasswordHash(QStringLiteral("hunter2"), &a));
        QVERIFY(makePasswordHash(QStringLiteral("hunter2"), &b));
        QCOMPARE(a.algorithm, QStringLiteral("SHA-512"));
        QCOMPARE(a.spin, qint64(kNewSpinCount));
        QVERIFY(a.legacy.isEmpty());
        QVERIFY(a.salt != b.salt);   // (a salt of its own each time)
        QVERIFY(a.hash != b.hash);
        QVERIFY(!a.hash.contains(QStringLiteral("hunter2")) && !a.salt.contains(QStringLiteral("hunter2")));
        QCOMPARE(QByteArray::fromBase64(a.salt.toLatin1()).size(), 16);
        QCOMPARE(checkPassword(a, QStringLiteral("hunter2")), PasswordResult::Match);
        QCOMPARE(checkPassword(a, QStringLiteral("hunter3")), PasswordResult::Wrong);
        QVERIFY(!sheetProtectionElement([&] {
                    SheetProtection p;
                    p.enabled = true;
                    p.password = a;
                    return p;
                }(), QByteArray()).contains("hunter2"));
    }

    void aFileThatAsksForTooManyRepeatsIsRefusedNotComputed()
    {
        PasswordHash h;
        h.algorithm = QStringLiteral("SHA-512");
        h.salt = QStringLiteral("MDEyMzQ1Njc4OWFiY2RlZg==");
        h.hash = QStringLiteral("AAAA");
        h.spin = kMaxSpinCount + 1;
        QCOMPARE(checkPassword(h, QStringLiteral("x")), PasswordResult::TooSlow);
        h.spin = -1;
        QCOMPARE(checkPassword(h, QStringLiteral("x")), PasswordResult::TooSlow);
        h.spin = 5;
        h.algorithm = QStringLiteral("WHIRLPOOL-9000");
        QCOMPARE(checkPassword(h, QStringLiteral("x")), PasswordResult::Unsupported);
        // a repeat count the limit allows can be stopped
        h.algorithm = QStringLiteral("SHA-512");
        h.spin = kMaxSpinCount;
        std::atomic<bool> stop{true};
        QCOMPARE(checkPassword(h, QStringLiteral("x"), &stop), PasswordResult::Stopped);
    }

    void aWorkbooksStructureProtectionIsReadAndSaved()
    {
        QMap<QString, QByteArray> all = entriesOf(plainBook());
        QByteArray wb = all.value(QStringLiteral("xl/workbook.xml"));
        wb.replace("<sheets>", "<workbookProtection workbookPassword=\"CF1A\" lockStructure=\"1\"/><sheets>");
        all[QStringLiteral("xl/workbook.xml")] = wb;
        auto o = open(zipOf(all));
        QVERIFY2(o->ok, qPrintable(o->why));
        QVERIFY(o->book.info().protection.lockStructure);
        QVERIFY(!o->book.info().protection.lockWindows);
        QCOMPARE(o->book.info().protection.password.legacy, QStringLiteral("CF1A"));
        // changed: windows locked too, a strong password
        SaveChanges changes;
        changes.setWorkbookProtection = true;
        changes.workbookProtection.lockStructure = true;
        changes.workbookProtection.lockWindows = true;
        QVERIFY(makePasswordHash(QStringLiteral("pw"), &changes.workbookProtection.password));
        QString why;
        auto again = saved(*o, changes, &why);
        QVERIFY2(again, qPrintable(why));
        QVERIFY(again->book.info().protection == changes.workbookProtection);
        const QByteArray xml = entriesOf(again->bytes).value(QStringLiteral("xl/workbook.xml"));
        QCOMPARE(xml.count("<workbookProtection"), 1);
        QVERIFY(!xml.contains("workbookPassword"));
        // and taken off
        SaveChanges off;
        off.setWorkbookProtection = true;
        auto cleared = saved(*again, off, &why);
        QVERIFY2(cleared, qPrintable(why));
        QVERIFY(!cleared->book.info().protection.any());
        QVERIFY(!entriesOf(cleared->bytes).value(QStringLiteral("xl/workbook.xml")).contains("workbookProtection"));
    }
};

JP_TEST_CLASS(SheetNotesTests)
#include "tst_sheet_notes.moc"
