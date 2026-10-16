#pragma once
// The table styles a presentation file defines (its ppt/tableStyles.xml, a:tblStyleLst, ECMA-376 Part 1, 20.1.4.2.27). A table that names one is
// drawn and edited with the file's definition of it, so they are kept in the document (appData "tableStyles", each a:tblStyle as standalone XML
// with its id) and written back as they were. Only what a hostile file can't multiply is kept: a number of styles, a size for each, and a size in all.

#include "core/document.h"
#include "office/drawingml_table.h"

#include <QDomElement>
#include <QString>
#include <QStringList>
#include <functional>

namespace jp::deck {

// Keeps the definitions of `list` (the root of the part) and its default style; `standalone` gives an element as XML that reads by itself.
// `notes` gets a line when some are too many or too large to keep. A null list keeps none.
void readTableStyleDefs(const QDomElement &list, const std::function<QString(const QDomElement &)> &standalone, Document &doc, QStringList *notes);

// The definitions kept, by the upper-case id a table names (for office::tableStyleEffect).
office::TableStyleDefs tableStyleDefsOf(const Document &doc);

// The tableStyles part: the definitions kept, in the order the file had them, and the default style.
QString tableStylesPartXml(const Document &doc);

} // namespace jp::deck
