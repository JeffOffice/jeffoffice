#pragma once
// Writing Rich Text Format files (.rtf) from the document the .docx reader makes (libs/office/wml): the character and paragraph looks, the styles, lists, tables, sections with their page
// setup, headers and footers, links, and pictures (PNG, JPEG, and EMF). The file follows the Rich Text Format specification (version 1.9): a header with the font, color, style, and list
// tables, then the text, every character outside ASCII written as a signed 16-bit \u number with a "?" for programs that don't read it. What the format has no place for here (notes,
// comments, tracked changes, equations, drawings that are not pictures) is left out, and `notKept` says which, one line for each kind.

#include "wml/wml.h"

namespace jp::office::rtf {

// The document as an .rtf file; empty, with the reason in `error`, if it can't be written.
QByteArray writeRtf(const wml::Document &doc, QStringList *notKept = nullptr, QString *error = nullptr);

} // namespace jp::office::rtf
