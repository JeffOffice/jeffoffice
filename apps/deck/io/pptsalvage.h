#pragma once
// Open and Repair of an old binary presentation (.ppt) whose container is damaged or cut short. libs/office/office/cfbsalvage.h finds the streams (the presentation's stream by the
// start of its document record, the stream that names the current edit by its own header, and the pictures if the directory leads to them); this makes them into a presentation the
// reader opens. When the edit the file ends with is gone (a file cut short loses the end of the stream, which is where the list of edits is), the last edit that is whole is used; when
// there is none, the list of where each slide and master is kept is made again from the records of the stream, matching the slides the document's list names to the slide records in the order
// they are stored. Slides that can be read open and the rest are left out, as the reader says. A presentation protected with a password is refused.

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace jp::deck {

// The records of the stream are looked at once, and at most kSalvageMaxRecords of them (at the top of the stream: the records inside them are the reader's, with its own limits).
constexpr int kSalvageMaxRecords = 1000000;

// The edits at the end of the stream that are tried, newest first, each by reading the whole presentation again.
constexpr int kSalvageMaxEditTries = 8;
// How many times the presentation was read to see whether it opens, in the last salvage (for a test of the limit).
int salvageOpensForTest();

// The most records any one walk of a stream has gone through since the program began (for a test of the limit).
qint64 salvageRecordsWalkedForTest();

// The repaired file (a compound file the reader opens), empty with `error` if nothing can be read. `notes` says in words what was done.
QByteArray salvagePresentation(const QByteArray &file, QStringList *notes, QString *error);

} // namespace jp::deck
