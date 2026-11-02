#pragma once
// Fields of the text: finding them, updating their results (F9), unlinking, locking, and the cross-references, bookmarks, and tables of contents built on them. A field of the text
// is a begin mark, its code, a separator, its result, and an end mark, each mark a character of the story that keeps its piece of the file (or a short form of one character that the
// program makes, kFieldGroupTag); the result is the text between the separator and the end. An update changes only the text of results (and a mark's dirty flag), in one step of the
// history, and never as a tracked change: the original program does not track one either.

#include "core/fieldcode.h"
#include "core/writedoc.h"

#include <QDateTime>
#include <QHash>
#include <QString>
#include <QTextCursor>
#include <QVector>
#include <functional>

namespace jp::write {

constexpr int kMostFields = 100000;        // the fields of a story that are looked at (a hostile file can have more)
constexpr int kMostFieldDepth = 16;        // how deep fields are nested in each other, in the code or in the result
constexpr int kMostTocEntries = 20000;     // the lines a table of contents is made of
constexpr int kMostBookmarks = 100000;     // the bookmarks looked at

// A piece of a field's code: the words of one instruction mark, or the place where a field nested in the code stands (its result is the words).
struct CodePiece {
    QString text;
    int field = -1;
};

// A field of the text, by the places of its marks (positions in the story).
struct FieldPlace {
    enum Form { Complex, Simple, Group };   // fldChar marks; one w:fldSimple element; one character that holds all the runs of a field (a field inserted, or in a header)
    Form form = Complex;
    int begin = -1, separate = -1, end = -1;   // the marks' positions; a group's are all one place, a simple field's begin and separate are one
    int parent = -1;                           // the field this one stands in
    bool inCode = false;                       // in the parent's code (else in its result)
    int depth = 0;
    QVector<CodePiece> code;
    QString name;                              // the code's first word, upper case
    bool locked = false, dirty = false;
    bool live = false;                         // a group the pages answer (PAGE, NUMPAGES, SECTIONPAGES, STYLEREF): it needs no update
    QString display;                           // a group's result
    QByteArray groupRaw;                       // a group's runs
};

struct BookmarkSpan {
    QString name, id;
    int start = -1, end = -1;                  // the marks' positions (the text is between them)
    bool hidden = false;                       // a name that begins with an underscore
};

// What a pass over a story finds: its fields in the order they begin, its bookmarks, and its text (U+FFFC for each mark).
struct StoryScan {
    QVector<FieldPlace> fields;
    QVector<BookmarkSpan> bookmarks;
    QHash<QString, int> bookmarkByName;       // (lower case) -> the first of the name
    QString plain;
    bool truncated = false;
    long maxBookmarkId = 0;                   // the highest id of any bookmark mark in the story, those past the most that are kept too
    // The text of the result a field has, marks left out.
    QString resultOf(int field) const;
    // The code of a field with the nested fields' results (those in `override` in place of the ones the story has).
    QString instructionOf(int field, const QHash<int, QString> &override = QHash<int, QString>()) const;
};
StoryScan scanStory(QTextDocument *story, bool bookmarks = true);

// The number a list gives the paragraph, without trailing periods ("2.1"); empty for a paragraph that is not numbered.
QString paragraphNumberOf(const QTextBlock &block);
// The same as the list shows it ("2.1.", with its period).
QString paragraphLabelOf(const QTextBlock &block);

// The properties the document's parts give the fields (the author, the title, the dates, the counts).
fields::Properties documentProperties(WriteDoc &doc);

struct FieldChange {
    QString name, instruction, before, after;
    int position = 0;
    enum Outcome { Unchanged, Changed, Kept, Locked } outcome = Unchanged;   // Kept: the engine does not know the field, or the document can't say
};

struct UpdateOptions {
    int from = 0, to = -1;                       // the part of the text whose fields are updated (a field that touches it is); to < 0: to the end
    bool apply = true;                           // false: only work the results out (for a tool that compares them)
    bool pageNumbersOnly = false;                // a table of contents: its page numbers, not the lines
    QString tocLeader;                           // the tab leader of a table's page numbers when it is made (dot, hyphen, underscore, none); empty: the one the table has, else dot
    QDateTime now;                               // the moment automatic dates show (invalid: now)
    std::function<bool(const QString &)> skip;   // fields whose name this says are left as they are
};

struct UpdateReport {
    int found = 0, changed = 0, unchanged = 0, kept = 0, locked = 0;
    bool tocTruncated = false;                   // a table of contents had more lines than kMostTocEntries and was cut there
    QVector<FieldChange> changes;                // every field looked at, with what it was and would be (only when `keepChanges`)
};

// Updates the fields of the text in the part the options name, as one step of the history. The page-dependent ones are worked out from the pages as laid out now (the pages are laid
// out first, which can take a while in a long document).
UpdateReport updateFields(WriteDoc &doc, const UpdateOptions &options = UpdateOptions(), bool keepChanges = false);
// The same for the field the caret is in, or those a selection touches; for F9.
UpdateReport updateFieldsAt(WriteDoc &doc, const QTextCursor &c);

// Each field the selection touches (the one the caret is in) becomes the text of its result. Returns how many.
int unlinkFields(WriteDoc &doc, const QTextCursor &c);
// Locks (or unlocks) the fields the selection touches (Ctrl+F11, Ctrl+Shift+F11). Returns how many were changed.
int lockFields(WriteDoc &doc, const QTextCursor &c, bool lock);
// Shows the codes of the field at the caret (Shift+F9), or of every field (Alt+F9, with `all`), in braces in place of the results, or shows the results again. A view only: the file is
// written as it was. Returns how many fields were changed.
int toggleFieldCodes(WriteDoc &doc, const QTextCursor &c, bool all);

// Counted work, for tests: the paragraphs looked at to find the paragraph of a style, and the checks of results made again, since the counts were reset.
qint64 styleRefParagraphVisitsForTest();
qint64 resultOverlapChecksForTest();
// How many times the engine was asked for a result since the counts were reset (a bound on what a pass over hostile fields costs).
qint64 fieldComputationsForTest();
// The bytes of a field mark with its lock (`name` fldLock) or dirty flag set or cleared; a mark that is not whole is returned as it is.
QByteArray fieldFlagForTest(const QByteArray &raw, const char *name, bool on);
void resetFieldWorkForTest();

// The innermost field of the text that holds the position (the caret), -1 for none.
int fieldContaining(const StoryScan &scan, int position);

// A field, as one character, inserted at the caret (or in place of the selection): its code and the result the engine works out (the text given when it can't). Returns false where it
// can't be put (a header, a note, a table of a kind the program does not edit). `code` is the field's instruction, without the blanks round it.
bool insertField(WriteDoc &doc, QTextCursor &c, const QString &code, const QString &fallbackResult = QString());

} // namespace jp::write
