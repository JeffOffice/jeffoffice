#pragma once
// Hyperlinks: a web address, an email address, or a place in the document (a bookmark), over text. The link is a pair of tags round the text (w:hyperlink), as the file keeps it;
// an address outside the file is a relationship of the main part.

#include "core/fieldops.h"
#include "core/writedoc.h"

#include <QString>
#include <QVector>

class QTextCursor;
class QTextDocument;

namespace jp::write {

struct Hyperlink {
    QString address;      // outside the file ("https://...", "mailto:..."); empty for a link to a place
    QString anchor;       // the name of a bookmark in this document; empty for a link outside
    QString screenTip;
    QString text;         // what the link shows
    int start = -1, end = -1;   // the positions of its opening and closing tags in the story (the text is between them)
    bool valid() const { return start >= 0; }
};

// The address a person typed made into one a link can hold: an email address gets "mailto:", a web address without a scheme gets "https://". Empty where it is not an address.
QString normalizedAddress(const QString &typed);

// Whether a link to this address is followed (a web or email address; no link starts a program or opens a file).
bool mayFollow(const QString &address);

// Whether following a link to this address, shown as these words, is asked about first: a mail or telephone link, and one whose words look like another address than it goes to.
bool linkNeedsAsking(const QString &address, const QString &shown);

// The innermost link that has the position in it (between its tags, or at their edge), found in the paragraph; not valid where there is none.
Hyperlink hyperlinkAt(WriteDoc &doc, int position);

// A link over the selection (its text kept), or, with none selected, over `text` put in at the caret; the character style Hyperlink is given to its text. One step that undoes.
// False, with `problem` told as a sentence, where the address is empty or not one, or the place is no bookmark, or the caret is not in the text.
bool insertHyperlink(WriteDoc &doc, QTextCursor &c, const QString &address, const QString &anchor, const QString &screenTip, const QString &text, QString *problem = nullptr);
// Changes where the link at the position goes (and the words it shows when `text` is not empty), as one step that undoes.
bool changeHyperlink(WriteDoc &doc, int position, const QString &address, const QString &anchor, const QString &screenTip, const QString &text, QString *problem = nullptr);
// Takes the link at the position off its text (the text stays, as plain text).
bool removeHyperlink(WriteDoc &doc, int position);

// A stretch of the text that is a link, for a PDF: the words between a hyperlink's tags, or a cross-reference's result when it has the \\h switch (REF, PAGEREF, NOTEREF), as a table
// of contents' entries have. Positions in the story. A link outside the file has its address (a web or email address only); one to a place has where it goes, a position (-1: nowhere).
struct LinkSpan {
    int from = 0, to = 0;
    QString uri;
    int target = -1;
};
constexpr int kMostLinkSpans = 20000;
QVector<LinkSpan> linkSpans(WriteDoc &doc);
// The links of another story (a header's, a footer's, a note's, a text box's, in the form a page shows it): only the hyperlinks, whose addresses are those of the part `source` is of (null:
// the main part), and whose places are the bookmarks of the main text (`scan` is its scan). The positions are the story's own.
QVector<LinkSpan> linkSpansIn(WriteDoc &doc, QTextDocument *story, const QTextDocument *source, const StoryScan &scan);

} // namespace jp::write
