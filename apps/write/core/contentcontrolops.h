#pragma once
// Content controls (w:sdt round runs): a plain text box, a drop-down list, a date picker, and a check box, put in a paragraph, with a title, a tag, and locks. A control is a pair of atoms
// (its start with the properties, its end) round the text it holds, as a hyperlink is; what a file says that is not known here is kept as written, and a control nobody changes is written
// byte for byte as it was read.

#include "core/writedoc.h"
#include "core/pictureops.h"

#include <QByteArray>
#include <QDate>
#include <QSet>
#include <QString>
#include <QVector>

class QTextBlock;
class QTextCursor;
class QTextDocument;

namespace jp::office::wml {
struct Block;
}

namespace jp::write {

enum class ControlKind { PlainText, DropDown, Date, CheckBox, RichText, Picture, ComboBox, RepeatingSection, Other };

struct ControlItem {
    QString display, value;
};

// What a control says of itself (w:sdtPr).
struct ControlProps {
    ControlKind kind = ControlKind::PlainText;
    QString title, tag;
    bool lockDelete = false;       // "The control cannot be deleted" (w:lock sdtLocked)
    bool lockContents = false;     // "The contents cannot be edited" (w:lock contentLocked)
    QVector<ControlItem> items;    // a drop-down list's, or a combo box's
    QString dateFormat;            // a date picker's (as a person writes it: M/d/yyyy)
    QDate date;                    // the date it holds (invalid for none)
    QString lid;                   // the language its date is written in (w:lid, "en-US"); empty: the program's
    bool sectionItem = false;      // it is an item of a repeating section (w15:repeatingSectionItem)
    bool fixedItems = false;       // a repeating section's: items may not be added or removed (w15:doNotAllowInsertDeleteSection)
    bool checked = false;          // a check box's
    QString lastValue;             // a drop-down list's: the value of the item chosen
    bool operator==(const ControlProps &o) const;
};

// A control in a paragraph of the text: `start` is the place of its first atom, `end` the place of its last; what it holds is between them.
struct ContentControl {
    ControlProps props;
    int start = -1, end = -1;
    bool placeholder = false;      // it shows the words that stand for what is not yet put in
    bool editable = false;         // its properties can be changed here (it was read whole, and is of a kind known here)
    int depth = 0;                 // how many controls hold it
    bool block = false;            // it holds whole paragraphs (its two ends are kept pieces before the first and after the last, as the table of contents' wrapper is); start and end are then the first and last paragraph's
    int openIndex = -1;            // a block control: which of the first paragraph's kept pieces opens it
    QString text;                  // what it shows
    bool valid() const { return start >= 0; }
    // Whether a place in the text is inside it: after its first atom, up to its last (a block control: from its first paragraph's start to its last paragraph's end).
    bool contains(int position) const { return block ? position >= start && position <= end : position > start && position <= end; }
};

// The most each of these is read or made: items in a list, controls in a file, controls inside one another.
constexpr int kMostControlItems = 1000;
constexpr int kMostControls = 10000;
constexpr int kMostControlDepth = 8;
constexpr int kMostRepeatingItems = 1000;   // items in one repeating section

// What the start of a control (the XML its first atom keeps) says. `ok` says it was read whole; a start that can't be read (not well formed, or in a namespace prefix other than the usual)
// has a kind of Other and is left as it is.
ControlProps controlPropsOf(const QByteArray &startXml, bool *ok = nullptr, bool *placeholder = nullptr);

// The controls in the text, in the order of their starts (at most kMostControls; those inside others deeper than kMostControlDepth are not counted).
QVector<ContentControl> contentControls(WriteDoc &doc);
// The innermost control that holds the place (in the story given, the text by default), not valid where there is none.
ContentControl contentControlAt(WriteDoc &doc, int position, QTextDocument *story = nullptr);
// The controls of one paragraph.
QVector<ContentControl> contentControlsOf(const QTextBlock &block);

// Puts a control of the kind at the caret, showing its placeholder words (a plain text control over a selection holds the selection; the others go after it): one step that undoes. False, with
// `problem`, where the caret is not in the text, the file already has the most controls, or the controls hold each other to the greatest depth.
bool insertContentControl(WriteDoc &doc, QTextCursor &c, ControlKind kind, QString *problem = nullptr);

// Changes what the control that begins at `start` says of itself (title, tag, locks, the items of a drop-down list, a date's format). One step. False where there is no such control, it can't
// be edited, or the kind differs.
bool setContentControlProps(WriteDoc &doc, int start, const ControlProps &props, QString *problem = nullptr);
// Puts the item (a number in its list) in the drop-down list.
bool chooseControlItem(WriteDoc &doc, int start, int index);
// Puts the date in the date picker, written in its format (an invalid date takes it out: the placeholder shows again).
bool setControlDate(WriteDoc &doc, int start, const QDate &date);
// Flips a check box (or sets it, with `checked`).
bool toggleControlCheck(WriteDoc &doc, int start);
bool setControlChecked(WriteDoc &doc, int start, bool checked);
// Selects everything the control holds.
void selectControlContents(QTextCursor &c, const ContentControl &control);

// ---- picture controls (a placeholder that takes a picture; a picture in its place keeps the size the first had)
// Puts the picture file's bytes in the picture control that begins at `start` (the one step that undoes), fitted in the size the control had (two inches square for the first), kept in
// proportion; the result says what a refused picture was (the picture limits are pictureops.h's).
PictureResult setControlPicture(WriteDoc &doc, int start, const QByteArray &bytes, const QString &name = QString());
// Takes the picture out: the placeholder shows again. False where there is none to take out or the contents are locked.
bool clearControlPicture(WriteDoc &doc, int start);

// ---- block controls: a control round whole paragraphs (and the lists, tables, and controls in them); `block` is set, `start` and `end` are the places of the first and last paragraph
// Every block control of the story, and the innermost one that holds the place (not valid where there is none).
QVector<ContentControl> blockControls(WriteDoc &doc, QTextDocument *story = nullptr);
ContentControl blockControlAt(WriteDoc &doc, int position, QTextDocument *story = nullptr);
// The control the caret is in: the innermost one in the words, else the innermost one round the paragraphs.
ContentControl controlAtCaret(WriteDoc &doc, const QTextCursor &c);
// Puts a rich text control round the paragraphs the selection touches (a repeating section, with one item holding them): refused where they hold only part of a table or another control.
bool insertBlockControl(WriteDoc &doc, QTextCursor &c, ControlKind kind, QString *problem = nullptr);
bool setBlockControlProps(WriteDoc &doc, const ContentControl &ctl, const ControlProps &props, QString *problem = nullptr);
// The repeating item that holds the place and, in `section`, the section it is in (not valid where there is none).
ContentControl repeatingItemAt(WriteDoc &doc, int position, ContentControl *section = nullptr, QTextDocument *story = nullptr);
// Adds an item after the last of the section that holds the place, a copy of the last one with every control in it showing its placeholder (the one step that undoes); removes the item that
// holds the place (a section keeps one). False, with `problem`, where it can't.
bool addRepeatingItem(WriteDoc &doc, int position, QString *problem = nullptr);
bool removeRepeatingItem(WriteDoc &doc, int position, QString *problem = nullptr);

// A date written in a format of the picker (d, dd, ddd, dddd, M, MM, MMM, MMMM, yy, yyyy and the text round them).
QString formatControlDate(const QDate &date, const QString &format, const QString &lid = QString());

// The place of a drop-down list's choice that is shown (-1: the placeholder, or something not in the list).
int controlItemShown(const ContentControl &control);

// Whether the range holds one end of a control and not the other (an edit of it would leave half a control).
bool splitsContentControl(QTextDocument *story, int from, int to);
// A text control that has been emptied shows its placeholder again (the caret is in it). False where there is none to do.
bool restoreEmptyPlaceholder(WriteDoc &doc, int position);
// For tests: how many times the start of a control has been read (what is read once is kept).
int controlParsesForTest();

// Whether an edit may be made: removing the text from `from` to `to` (when they differ) or putting text in at `from`. A control that can't be deleted keeps both its atoms, one whose contents
// are locked keeps what it holds, and the check box and the drop-down list hold only what is chosen for them. `typing`: text goes in (else it comes out).
bool contentControlsAllow(WriteDoc &doc, QTextDocument *story, int from, int to, bool typing);
// The same for a story alone (what has no document to ask).
bool contentControlsAllowIn(QTextDocument *story, int from, int to, bool typing);
// The places of a story that content controls keep from being changed (the contents of those that can't be edited, and what only a choice fills in), as sorted, separate ranges [first, end); with
// `withEdges` the paragraph breaks at the edges of a control round paragraphs are in them. The places of a change (a format, an accepted or rejected change) are checked against them.
QVector<std::pair<int, int>> lockedContentRanges(QTextDocument *story, bool withEdges);
// While alive, a story that has no content control that keeps its contents is not looked through for them at each edit (the work of one edit would be the whole text): for a batch of edits that put in plain
// text and no control. (When the story has a locked control to begin with, nothing is held, and each edit looks as before.)
struct NoLockedContent {
    explicit NoLockedContent(QTextDocument *s);
    ~NoLockedContent();
    NoLockedContent(const NoLockedContent &) = delete;
    NoLockedContent &operator=(const NoLockedContent &) = delete;
    QTextDocument *story;
    bool on = false;
};
bool overlapsRanges(const QVector<std::pair<int, int>> &ranges, int from, int to);
// For tests: how many times the places have been worked out, since the program began.
int lockedRangeScansForTest();
// Whether the characters from `from` to `to` may be given another look (not where the contents can't be edited).
bool contentControlsAllowFormat(QTextDocument *story, int from, int to);
// A paragraph `a` joined to the one after it by Backspace or Delete, where the break is the edge of a control round paragraphs: endJoinAtEdge, after the join, puts the paragraph made on the side of the
// caret (`inSecond`: the caret was in the second paragraph, so the joined paragraph is inside the control that starts there, or outside the one that ends in the first), unless that would leave a
// control with no paragraph: then it is the control's (beginJoinAtEdge says false where both would). Locks are checked by contentControlsAllow, in the join itself.
struct EdgeJoin {
    QTextDocument *story = nullptr;
    int position = 0, blocks = 0, moved = 0;
};
bool beginJoinAtEdge(WriteDoc &doc, const QTextBlock &a, bool inSecond, EdgeJoin *join);
void endJoinAtEdge(const EdgeJoin &join);
// For the paragraphs a change takes out (Accept All, Reject All): the starts and ends of controls a paragraph keeps, when it keeps nothing else (`*only`), so that they can be carried to the
// paragraph before it; put after that paragraph's own by appendKeptAfter.
QVector<office::wml::Block> controlPiecesOf(const QTextBlock &b, bool *only);
void appendKeptAfter(const QTextBlock &b, const QVector<office::wml::Block> &pieces);
// A control round paragraphs that no paragraph is left in (its start straight before its end, once the paragraphs a change took out are gone) goes, unless it was empty before (`wasEmpty`,
// found by emptyControlStarts before the change: a control a form leaves empty is not the change's to take). How many went.
QSet<QByteArray> emptyControlStarts(QTextDocument *story);
int dropEmptyControls(QTextDocument *story, const QSet<QByteArray> &wasEmpty);
// Whether what a person types at the caret takes the place of a placeholder (the caret is in a control that shows one, or the selection is it).
bool typingReplacesPlaceholder(WriteDoc &doc, const QTextCursor &c);
// Then the placeholder is selected, and the control no longer says it is showing one (call it inside the step that types).
void prepareTypingInControl(WriteDoc &doc, QTextCursor &c);
// Whether the caret is in a control of a kind known here that holds one paragraph (an Enter there makes no paragraph: the control's two ends stay in one; a rich text control is split by it).
bool caretInKnownControl(WriteDoc &doc, const QTextCursor &c);

} // namespace jp::write
