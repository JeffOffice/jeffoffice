#pragma once
// The lists of an old binary document (the list definitions and the instances of them that paragraphs refer to), written as the
// numbering part of a .docx. Internal to libs/office/doc.

#include "doc/docstyles.h"

namespace jp::office::doc {

struct Lists {
    int instances = 0;        // how many instances (list numbers a paragraph can have)
    QByteArray xml;           // the numbering part (empty if there are no lists)
};
// Reads the list definitions (PlfLst) and the instances (PlfLfo) from the table stream.
Lists readLists(const Span &plfLst, const Span &plfLfo, const FontNames &fonts, const StyleSheet &styles);

} // namespace jp::office::doc
