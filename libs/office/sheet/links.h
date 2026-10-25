#pragma once
// Links on cells: a cell (or a block of cells) that goes to a web address, an e-mail address, a file, or a place in the workbook when it is followed. The file keeps them in the sheet's
// `hyperlinks` (a place in the workbook in the element itself, an address outside it in a relationship of the sheet). Reading them is part of reading the sheet (sheetreader); writing is
// part of saving a sheet (xlsxsave). This is what they are, and what following one may do.

#include "sheet/cellref.h"

#include <QString>

namespace jp::office::sheet {

// The most links one sheet is read with, and the most characters of an address, a place, a shown text, and a screen tip kept (a link with more is not read).
constexpr qint64 kMaxLinks = 20000;
constexpr int kMaxLinkTarget = 8192;
constexpr int kMaxLinkText = 1024;
// The most bytes (counting two for each character, and 64 for each link) the links of all the sheets of a workbook hold; a link that would go past it is not read.
constexpr qint64 kMaxLinkBytesPerBook = qint64(32) << 20;

struct LinkSpec {
    CellRange range;
    QString target;     // an address outside the workbook, as the file writes it ("https://example.org/", "mailto:a@example.org", a file's name); empty for a place in the workbook
    QString location;   // a place in the workbook ("Sheet2!B5", or a defined name); empty for an address outside it
    QString display;    // the text the file says to show
    QString tip;        // the screen tip
    QString relId;      // for a link read from a file: the relationship that names its address (the sheet resolves it; not kept past that)
    bool outside() const { return !target.isEmpty(); }
    bool operator==(const LinkSpec &o) const { return range == o.range && target == o.target && location == o.location && display == o.display && tip == o.tip; }
};

enum class LinkKind : quint8 {
    Web,      // http or https
    Mail,     // mailto
    File,     // a file (file:, a path, or a name with no scheme)
    Place,    // a place in this workbook
    Other,    // another scheme: it is asked about before it is followed
    Unsafe,   // a scheme that runs a script or carries a page in itself: never followed
};

LinkKind linkKind(const LinkSpec &link);
// Whether a file or path a link names is a program, a script, or a shortcut (a name ending .exe, .bat, .cmd, .ps1, .vbs, .js, .msi, .scr, .lnk, and the like): a link to one is never opened.
// Looked at the way a system would take the name: a query or a fragment is left off, a %-escape read, and spaces, dots, and quotes at the end ignored.
bool namesAProgram(const QString &target);
// The text cut in the middle (a "..." between its start and its end) if it is longer than `most` characters, so that a long address is shown with both ends.
QString elidedMiddle(const QString &text, int most);
LinkKind targetKind(const QString &target);

// A web address as a person typed it, made one a link can hold: spaces at the ends taken off, "mailto:" put before one that looks like an e-mail address and has no scheme, and
// "https://" before any other that has none. Empty if nothing was typed.
QString webAddressFromTyped(const QString &typed);

} // namespace jp::office::sheet
