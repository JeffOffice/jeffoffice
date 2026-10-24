#pragma once
// A scanner for the tags of an XML part held in bytes, for changing a part without reading it into a tree: it finds where a tag starts and ends, what its attributes say,
// and gives the bytes of text and attributes as they are written. Saving a workbook uses it to change a few cells of a sheet, a few strings of the shared strings, or one format
// of the styles, and to keep every other byte as it was. It does not check that the XML is well formed; a part it cannot make sense of is left to the caller to refuse.

#include <QByteArray>
#include <QByteArrayView>
#include <QString>

namespace jp::office::sheet::xmlscan {

struct Tag {
    qsizetype start = 0;          // the "<"
    qsizetype end = 0;            // after the ">"
    bool closing = false;         // "</name>"
    bool selfClosing = false;     // "<name .../>"
    QByteArrayView name;          // as written, with any prefix ("x:row")
    QByteArrayView local;         // without the prefix ("row")
};

// The next tag of an element (an open tag, a close tag, or one that closes itself) that begins at or after `from`; comments, CDATA sections, processing instructions, and the
// declaration are passed over. False at the end of the data, or if a tag is cut off by it (`cutOff`, if given, says which).
bool nextTag(QByteArrayView data, qsizetype from, Tag *tag, bool *cutOff = nullptr);

// The index after the ">" that ends the tag beginning at `start` (a "<"), the quotes of its attributes respected; -1 if the data ends first.
qsizetype tagEnd(QByteArrayView data, qsizetype start);

// The value of the attribute `name` in the tag's bytes, as written (not unescaped), without its quotes; `found` says whether there was one. A prefix is part of the name asked for
// ("xml:space").
QByteArrayView attribute(QByteArrayView tag, QByteArrayView name, bool *found = nullptr);
// (the view is of the bytes of the tag: a tag that is a temporary would be let go of before the view is read, so it is not taken)
QByteArrayView attribute(QByteArray &&tag, QByteArrayView name, bool *found = nullptr) = delete;
// Where the value of the attribute is in the tag's bytes ([start, end), inside the quotes); false if the tag has none.
bool attributeValue(QByteArrayView tag, QByteArrayView name, qsizetype *start, qsizetype *end);

// The prefix of the name ("x:" for "x:row", "" for "row") and the name without it.
QByteArrayView prefixOf(QByteArrayView name);
QByteArrayView localName(QByteArrayView name);

// Text as it is written in a text node or in an attribute: & < > (and " in an attribute) as entities, and the characters XML cannot hold (the controls other than tab and line feed,
// a carriage return, and the two noncharacters) as _xHHHH_, with a "_x" that is followed by four hex digits and "_" written _x005F_x so that the reading does not take it for one.
QByteArray escapeText(const QString &text);
QByteArray escapeAttribute(const QString &text);
// An attribute or text as written, with the entities (&amp; &lt; &gt; &quot; &apos; &#N; &#xN;) read.
QString unescape(QByteArrayView text);

// The number of a column as a file writes it in "r" ("AB12" -> column 27 (from 0), row 11): false if the text is no cell address.
bool parseAddress(QByteArrayView text, quint32 *row, quint32 *col);

} // namespace jp::office::sheet::xmlscan
