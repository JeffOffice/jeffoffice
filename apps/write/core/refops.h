#pragma once
// Bookmarks, cross-references, and the places a link in the text goes to: what the Bookmark and Cross-reference dialogs and Ctrl+click work with (the fields are core/fieldops.h's).

#include "core/fieldops.h"

namespace jp::write {

constexpr int kMostBookmarkNameLength = 40;
constexpr int kMostCrossReferenceTargets = 5000;

// ---- bookmarks
struct BookmarkItem {
    QString name;
    int start = 0, end = 0;
    bool hidden = false;
};
// The bookmarks of the text, sorted by name (without regard to case) or by where they begin; the hidden ones (their names begin with an underscore) only with `hidden`.
QVector<BookmarkItem> bookmarkList(WriteDoc &doc, bool hidden = false, bool byLocation = false);
// Why a name can't be a bookmark's (empty: it can): it must begin with a letter, hold only letters, digits, and underscores, and be 40 characters at most. (A hidden name is made by the
// program.)
QString bookmarkNameProblem(const QString &name);
// Puts the bookmark at the selection (or the caret: an empty place); one of the name that is already there moves. One step of the history. False, with the reason, where it can't be.
bool addBookmark(WriteDoc &doc, const QTextCursor &c, const QString &name, QString *problem = nullptr, bool hidden = false);   // (`hidden`: the program's own, a name that begins with an underscore)
// Takes the bookmark away (its marks, not the text); false if there is none of the name.
bool deleteBookmark(WriteDoc &doc, const QString &name);
// Where a bookmark begins (-1 for none).
int bookmarkStart(WriteDoc &doc, const QString &name);

// ---- cross-references
enum class RefKind { Heading, Bookmark, Footnote, Endnote, NumberedItem, Caption };
// Something a cross-reference can point to: a line the dialog lists, and the place it is.
struct RefTarget {
    QString label;            // the text the dialog lists
    int position = 0;         // the paragraph (or the note's reference, or the bookmark's start)
    QString bookmark;         // the bookmark that holds it, or the one that is made for it (a hidden _Ref name)
    bool newBookmark = false;
    int from = 0, to = 0;     // the text the bookmark covers when it is made
    QString number;           // the paragraph's number as a list gives it (empty if none)
};
// The captions' labels the text has (the identifiers of its SEQ fields, "Figure" and "Table" among them).
QStringList captionLabels(WriteDoc &doc);
// The targets of a kind in the order of the text (at most kMostCrossReferenceTargets). `label` names the caption label for Caption.
QVector<RefTarget> crossReferenceTargets(WriteDoc &doc, RefKind kind, const QString &label = QString());

// What the reference shows.
enum class RefWhat { Text, Number, NumberNoContext, NumberFullContext, Page, AboveBelow, FootnoteNumber, FootnoteNumberFormatted, LabelAndNumber, CaptionText };
// The code of the field a cross-reference makes (REF, PAGEREF, or NOTEREF with its switches); empty where the kind has no such reference.
QString crossReferenceCode(const RefTarget &target, RefKind kind, RefWhat what, bool hyperlink);
// Puts the reference at the caret, making the bookmark if the target has none, as one step. False, with the reason, where it can't be.
bool insertCrossReference(WriteDoc &doc, QTextCursor &c, const RefTarget &target, RefKind kind, RefWhat what, bool hyperlink, QString *problem = nullptr);

// Counted work, for tests: the bookmarks looked at to find a target's own bookmark, since the count was reset.
qint64 crossReferenceProbesForTest();
void resetCrossReferenceProbesForTest();

// ---- following links
// The place in the text that the link at a position goes to: a hyperlink to a bookmark, or a field with \h (REF, PAGEREF, NOTEREF) or a TOC entry. -1 where the position is in none.
int linkTargetAt(WriteDoc &doc, int position);

// ---- notes
// The next (or previous) footnote or endnote reference after (before) the position; -1 for none.
int nextNoteReference(WriteDoc &doc, int from, bool forward, bool endnote);
// The Id of the footnote whose reference is at the position (or just before it); -1 where there is none.
int footnoteReferenceAt(WriteDoc &doc, int position);

} // namespace jp::write
