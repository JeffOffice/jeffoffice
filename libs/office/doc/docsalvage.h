#pragma once
// Open and Repair of a .doc whose container is damaged or cut short (libs/office/office/cfbsalvage.h finds the streams; this says what a document's streams look like): the document
// stream is found by its header, the stream of tables by the place in it the header names for the pieces of the text, and whatever of the data stream (the pictures) can be read is kept.
// When the tables are lost, the text is found in the document stream by its own look (a run of letters as long as the header says the text is) and kept as plain paragraphs. The
// result is a compound file the reader opens as any other; `notes` says what was done and what was left out. A document protected with a password is refused.

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace jp::office::doc {

// The text looked for is searched for in the first kSalvageTextSearchBytes of the document stream, at every byte.
constexpr qint64 kSalvageTextSearchBytes = 65536;

QByteArray salvageDoc(const QByteArray &file, QStringList *notes, QString *error);

} // namespace jp::office::doc
