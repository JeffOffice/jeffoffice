#pragma once
// Hyperlinks: a web address, an email address, or a place in the document (a bookmark), over text. The link is a pair of tags round the text (w:hyperlink), as the file keeps it;
// an address outside the file is a relationship of the main part.

#include "core/writedoc.h"

#include <QString>

class QTextCursor;

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

// The innermost link that has the position in it (between its tags, or at their edge), found in the paragraph; not valid where there is none.
Hyperlink hyperlinkAt(WriteDoc &doc, int position);

// A link over the selection (its text kept), or, with none selected, over `text` put in at the caret; the character style Hyperlink is given to its text. One step that undoes.
// False, with `problem` told as a sentence, where the address is empty or not one, or the place is no bookmark, or the caret is not in the text.
bool insertHyperlink(WriteDoc &doc, QTextCursor &c, const QString &address, const QString &anchor, const QString &screenTip, const QString &text, QString *problem = nullptr);
// Changes where the link at the position goes (and the words it shows when `text` is not empty), as one step that undoes.
bool changeHyperlink(WriteDoc &doc, int position, const QString &address, const QString &anchor, const QString &screenTip, const QString &text, QString *problem = nullptr);
// Takes the link at the position off its text (the text stays, as plain text).
bool removeHyperlink(WriteDoc &doc, int position);

} // namespace jp::write
