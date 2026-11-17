#pragma once
// Content controls (w:sdt round runs): a plain text box, a drop-down list, a date picker, and a check box, put in a paragraph, with a title, a tag, and locks. A control is a pair of atoms
// (its start with the properties, its end) round the text it holds, as a hyperlink is; what a file says that is not known here is kept as written, and a control nobody changes is written
// byte for byte as it was read.

#include "core/writedoc.h"

#include <QByteArray>
#include <QDate>
#include <QString>
#include <QVector>

class QTextBlock;
class QTextCursor;
class QTextDocument;

namespace jp::write {

enum class ControlKind { PlainText, DropDown, Date, CheckBox, Other };

struct ControlItem {
    QString display, value;
};

// What a control says of itself (w:sdtPr).
struct ControlProps {
    ControlKind kind = ControlKind::PlainText;
    QString title, tag;
    bool lockDelete = false;       // "The control cannot be deleted" (w:lock sdtLocked)
    bool lockContents = false;     // "The contents cannot be edited" (w:lock contentLocked)
    QVector<ControlItem> items;    // a drop-down list's
    QString dateFormat;            // a date picker's (as a person writes it: M/d/yyyy)
    QDate date;                    // the date it holds (invalid for none)
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
    QString text;                  // what it shows
    bool valid() const { return start >= 0; }
    // Whether a place in the text is inside it: after its first atom, up to its last.
    bool contains(int position) const { return position > start && position <= end; }
};

// The most each of these is read or made: items in a list, controls in a file, controls inside one another.
constexpr int kMostControlItems = 1000;
constexpr int kMostControls = 10000;
constexpr int kMostControlDepth = 8;

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

// A date written in a format of the picker (d, dd, ddd, dddd, M, MM, MMM, MMMM, yy, yyyy and the text round them).
QString formatControlDate(const QDate &date, const QString &format);

// The place of a drop-down list's choice that is shown (-1: the placeholder, or something not in the list).
int controlItemShown(const ContentControl &control);

// Whether an edit may be made: removing the text from `from` to `to` (when they differ) or putting text in at `from`. A control that can't be deleted keeps both its atoms, one whose contents
// are locked keeps what it holds, and the check box and the drop-down list hold only what is chosen for them. `typing`: text goes in (else it comes out).
bool contentControlsAllow(WriteDoc &doc, QTextDocument *story, int from, int to, bool typing);
// Whether what a person types at the caret takes the place of a placeholder (the caret is in a control that shows one, or the selection is it).
bool typingReplacesPlaceholder(WriteDoc &doc, const QTextCursor &c);
// Then the placeholder is selected, and the control no longer says it is showing one (call it inside the step that types).
void prepareTypingInControl(WriteDoc &doc, QTextCursor &c);
// Whether the caret is in a control of a kind known here (an Enter there makes no paragraph: the control's two ends stay in one).
bool caretInKnownControl(WriteDoc &doc, const QTextCursor &c);

} // namespace jp::write
