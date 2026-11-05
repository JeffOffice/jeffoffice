#pragma once
// Styles as a person works with them: the list the Styles pane shows, Create Style, Modify Style, and Update Style to Match Selection. Each change to the style sheet is one step of the
// history with the text it restyles (writedoc.h, putStyle), and a hostile document is held to bounds: names, how many styles the program makes, and chains that name themselves.

#include "core/writedoc.h"

#include <QString>
#include <optional>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>

namespace jp::write {

constexpr int kMostStyleNameLength = 255;   // longer names are refused when made, and cut off where they are listed
constexpr int kMostStylesMade = 20000;      // Create Style adds no style to a document that has this many
constexpr int kMostStylesListed = 5000;     // the Styles pane lists no more than this many

enum class StyleFilter { InUse, All, Recommended };
// How the list is ordered: Default is by name (by priority for Recommended); the others hold for any filter.
enum class StyleSort { Default, Alphabetical, AsRecommended, ByType, BasedOn };

struct ListedStyle {
    QString id, name;
    QString basedOn;            // the id of the style it is based on (empty for none)
    bool character = false;
    bool linked = false;        // a paragraph style with a character style of its own look (w:link)
    bool inUse = false;         // the text uses it (the default paragraph style always)
    bool recommended = false;
    bool inDocument = true;     // false: a built-in style the document does not have yet (it is added when used)
};

// The styles the Styles pane shows for a filter: In Use lists those the text uses, All every style the document has (and the built-in ones it lacks), and Recommended those a
// person is meant to see (not hidden, and either in the gallery, with a low priority, or in use). In Use and All are sorted by name, Recommended by priority.
// The styles the text uses and the style `caretStyle` (an id) are always among them, and the rest follow in the order of the file up to the most that are listed.
QVector<ListedStyle> listStyles(WriteDoc &doc, StyleFilter filter, const QString &caretStyle = QString(), StyleSort sort = StyleSort::Default);
// Whether a style is listed for people and where: `inGallery` (recommended: it is in the gallery), `hidden` (left out of every list), and `priority` (0 to 99, lower comes first in the
// recommended list; -1 leaves it as it is). One step of the history; nothing done when it is all as it was. False, with `problem` told, when it can't be done.
bool setStyleListing(WriteDoc &doc, const QString &id, bool inGallery, bool hidden, int priority = -1, QString *problem = nullptr);
// Whether a style is locked (w:locked): while the document limits formatting (Manage Styles; WriteDoc::formattingLimited), a locked style can't be given to text. Setting it is one step of the
// history, and nothing is done when it is as it was.
bool styleLocked(const WriteDoc &doc, const QString &id);
bool setStyleLocked(WriteDoc &doc, const QString &id, bool locked, QString *problem = nullptr);
// Writes the styles `ids` of the document (with the styles they are based on) to a template file, `path` (a .dotx), that Import Styles and a new document can take them from. False, with `problem`
// told, when a style can't be copied or the file can't be written.
bool exportStyles(const WriteDoc &doc, const QStringList &ids, const QString &path, QString *problem = nullptr);
// The style the document has under `id`, or else the built-in one it lacks (which is not added: it is added with the first change that is made to it, as part of that step).
std::optional<office::wml::Style> styleOfDocument(const WriteDoc &doc, const QString &id);

// Why a name can't be given to a style (empty, too long, or taken by another style: names are compared without regard to case), as a sentence; empty when it can. `exceptId` is the
// style being renamed.
QString styleNameProblem(const WriteDoc &doc, const QString &name, const QString &exceptId = QString());
// Why a style can't be based on another (it doesn't exist, it is of the other kind, or it is, or is based on, the style itself); empty when it can.
QString styleBasedOnProblem(const WriteDoc &doc, const QString &id, const QString &basedOn, const QString &type);

// The style a selection looks like, ready for Create Style: a paragraph style (or, with `character`, a character style) based on the style of the cursor's paragraph that says only what
// the paragraph and the characters say themselves.
office::wml::Style styleFromSelection(WriteDoc &doc, const QTextCursor &c, const QString &name, bool character = false);

// Creates the style `def` describes (its id is made from its name; its type is paragraph or character) as one step of the history, and with `apply` gives it to what the cursor selects.
// Returns the id; empty, with `problem` told, when it can't be made.
QString createStyle(WriteDoc &doc, QTextCursor &c, const office::wml::Style &def, QString *problem = nullptr, bool apply = false);
// Gives the style `id` the definition `def` (name, based on, next, the paragraph and font properties, whether it is in the gallery, whether it updates itself) as one step of the history
// with the restyling of the text: a paragraph or run whose look is what the old style gave it follows the new one, and what the text says itself stays. A definition that comes to the
// style as it is changes nothing (no step; the file is written as it was). False, with `problem` told, when it can't be done.
bool modifyStyle(WriteDoc &doc, QTextCursor &c, const QString &id, const office::wml::Style &def, QString *problem = nullptr);
// Deletes a style the person made, as one step of the history: the paragraphs that had it go back to the default paragraph style (the characters, to none) and take its look off, the styles
// based on it are based on what it was based on, and what the text says itself stays. False, with `problem` told, for a built-in style (those only go back to what they were) and one the
// document lacks.
bool deleteStyle(WriteDoc &doc, const QString &id, QString *problem = nullptr);
// Copies the styles `ids` of another document into this one as one step of the history, each with the styles it is based on (the bases first), and says how many were added. A style this
// document has already is left as it is unless `replace` is true and the style was asked for (not just a base): then it is replaced, and the text that has it follows. A style that is not a paragraph
// or character style, that the other document lacks, or whose name another style of this document has, is not copied; the other document's list numbering is not carried over.
int importStyles(WriteDoc &doc, const WriteDoc &from, const QStringList &ids, bool replace, QString *problem = nullptr);
// Table styles: creates the table style the choices describe (based on any table style; the id is made from the name), or changes style `id` to them (the styles under it stay; a style that
// the choices leave as it is changes nothing), each as one step of the history. A table style the document lacks that is built in is added with the step. Returns the id (empty,
// with `problem` told, when it can't be made), or whether it was done.
// Whether a table style's name may be changed: the file defines it (or it is none of ours); a built-in one the file lacks keeps its name, also after it is changed.
bool tableStyleNameIsEditable(const WriteDoc &doc, const QString &id);
QString createTableStyle(WriteDoc &doc, const office::wml::TableStyleSpec &spec, QString *problem = nullptr);
bool modifyTableStyle(WriteDoc &doc, const QString &id, const office::wml::TableStyleSpec &spec, QString *problem = nullptr);
// Update <style> to Match Selection: the style the cursor's paragraph has says what the selection says besides it.
bool updateStyleToSelection(WriteDoc &doc, QTextCursor &c, QString *problem = nullptr);

// ---- Copy Formatting
// The look of what is selected (or, with nothing selected, of the paragraph and the character at the caret): the characters' look, and the paragraph's when it is a paragraph that is
// copied (the selection takes in a paragraph mark, or there is no selection).
struct FormatBrush {
    bool valid = false;
    QTextCharFormat chars;
    bool hasParagraph = false;
    QTextBlockFormat paragraph;
};
FormatBrush copyFormat(WriteDoc &doc, const QTextCursor &c);
// Gives the selection the look the brush holds, as one step of the history: the characters' (a collapsed cursor takes the word it is in), and the paragraphs' look and style as well when
// the brush has a paragraph and the selection takes in a paragraph mark (or is collapsed). What the text says about its place in the file (a list, a section, a frame) stays. False
// when nothing was changed.
bool pasteFormat(WriteDoc &doc, QTextCursor &c, const FormatBrush &brush);
// Whether a property of a paragraph's format is the paragraph's own (a list, a section break, the XML kept beside it), which a style or a painted look does not change.
bool paragraphKeepsOwn(int key);

} // namespace jp::write
