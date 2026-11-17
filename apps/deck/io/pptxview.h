#pragma once
// The view properties part of a .pptx (ppt/viewProps.xml): the grid spacing, whether guides show and what snaps, and the guides, as core/gridview.h
// keeps them in the presentation.

#include "core/document.h"

#include <QDomElement>
#include <QString>

namespace jp::deck {

// What the file says about the grid and guides goes into the presentation. Read within limits: the guide list's elements counted (a file can hold
// millions), positions put inside the slide.
void readViewProperties(const QDomElement &viewPr, Document &doc);

// The whole part. A presentation with no grid or guides settings gets the part JeffDeck has always written.
QString viewPropertiesXml(const Document &doc, const QString &namespaces);

} // namespace jp::deck
