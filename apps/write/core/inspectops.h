#pragma once
// What Inspect Document finds that a person may not want to share, and takes out: personal information in the properties, hidden text, the text of headers and footers, and custom XML
// parts. (Comments and tracked changes have their own operations: commentops.h and reviewops.h.) Each count is of what is there now; each removal is one step of the history where the text
// changes, and says how many it took out.

#include "core/writedoc.h"

namespace jp::write {

// The properties that name a person or a company: the author, the person who last saved, the company, and the manager.
int personalInfoCount(WriteDoc &doc);
int removePersonalInfo(WriteDoc &doc);

// Runs of hidden text in the body (text a tracked deletion hides is not counted: it is a change, not hidden text).
int hiddenTextCount(WriteDoc &doc);
int removeHiddenText(WriteDoc &doc);

// The headers and footers that have text in them.
int headerFooterCount(WriteDoc &doc);
int clearHeadersAndFooters(WriteDoc &doc);

// The parts of custom XML the file carries (data for other programs).
int customXmlCount(WriteDoc &doc);
int removeCustomXml(WriteDoc &doc);

// Links of the text to other files, not web addresses: a picture or an object that is only linked (the file it shows is not in the document), and a field that takes its result from
// another file (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE, and DDEAUTO). The count is of those in the body, the headers and footers, and the footnotes. Removing turns each such field into its
// result as plain text, takes out each linked picture or object (the file it showed is not touched), and lets the file's link to that address go when the document is saved.
int externalLinkCount(WriteDoc &doc);
// What the count could not look at, or could not take out: `unresolved` pictures and objects that name a link by an Id their part has no relationship for (the part, or its list of
// relationships, is missing, or has no such Id), so where they go can't be told; `unreachable` fields that take from another file in the comments and in text boxes, which the program keeps as
// they are and does not take apart.
struct ExternalLinkUnread {
    int unresolved = 0;
    int unreachable = 0;
};
ExternalLinkUnread externalLinkUnread(WriteDoc &doc);
int removeExternalLinks(WriteDoc &doc);

} // namespace jp::write
