#pragma once
// The changes that Compare found and nobody has decided yet, kept in the file (.pptx and .odp) so that closing and opening the presentation keeps them.
// What is kept is the other presentation (the file as it was read, whole) and the keys of the changes that were rejected; the changes themselves are found
// again against it when the presentation opens (apps/deck/core/deckcompare.h), so a change that was accepted is simply no longer a difference. The part is
// named /jo/review.json (jo/review.json in an .odp): other programs do not know it and leave it alone.
//
// In the document the comparison is appData "compare": {"theirs": the hash of the other presentation's bytes, "states": {change id: "accepted" or "rejected"},
// "rejected": [keys of the rejected changes]}. appData is saved and undone with the document, so an undo brings the states back with the slides. The bytes
// themselves are not in the document (they would be copied into every step of the history): they are in a small store here, by hash.
//
// Limits (a file is hostile): the part is read only up to kMaxReviewPartBytes, the copy of the other presentation up to kMaxReviewTheirsBytes, and at most
// kMaxReviewKeys keys; what is over is left out and the comparison is made again from what was kept.

#include "core/document.h"

#include <QByteArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace jp::deck {

constexpr qint64 kMaxReviewTheirsBytes = qint64(128) << 20;
constexpr qint64 kMaxReviewPartBytes = kMaxReviewTheirsBytes * 4 / 3 + (1 << 20);   // base64 and a little more
constexpr int kMaxReviewKeys = 10000;
constexpr int kReviewStoreMost = 4;   // the other presentations kept in memory at once (the oldest goes)

// Keeps `bytes` in the store and returns the key it is found by (empty for none, or more than kMaxReviewTheirsBytes).
QString storeReviewTheirs(const QByteArray &bytes);
QByteArray storedReviewTheirs(const QString &key);
int storedReviewCount();   // for tests
void clearReviewStore();   // for tests

// The part to write for `doc`: empty when it has no pending comparison (or the other presentation is no longer in the store).
QByteArray reviewPartOf(const Document &doc);
// Takes a part read from a file into `doc` (appData "compare"); false when it is not a usable one.
bool readReviewPart(const QByteArray &part, Document &doc);

// The part's name in a package of the newer kind and in one of the open document kind.
QString reviewPartName();
QString reviewOdfName();

} // namespace jp::deck
