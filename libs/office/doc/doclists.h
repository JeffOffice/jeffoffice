#pragma once
// The lists of an old binary document (the list definitions and the instances of them that paragraphs refer to), written as the
// numbering part of a .docx. Internal to libs/office/doc.

#include "doc/docstyles.h"

namespace jp::office::doc {

struct Lists {
    int instances = 0;        // how many instances (list numbers a paragraph can have)
    QByteArray xml;           // the numbering part (empty if there are no lists)
};
// Reads the list definitions (PlfLst) and the instances (PlfLfo) from the table stream. `plfLst` runs on past the size the header gives for it (`lcbLst`), as the levels of the lists
// are appended to it (section 2.9.201), but the definitions themselves are read only as far as that size; the same for `plfLfo`, whose overrides of levels (kept after the list of
// the instances) are read past the size `lcbLfo`, as the files of the real programs have them (section 2.9.200).
Lists readLists(const Span &plfLst, qint64 lcbLst, const Span &plfLfo, qint64 lcbLfo, const FontNames &fonts, const StyleSheet &styles);

} // namespace jp::office::doc
