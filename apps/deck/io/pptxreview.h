#pragma once
// The changes that Compare found and nobody has decided yet, kept in the file (.pptx and .odp) so that closing and opening the presentation keeps them.
// What is kept is the other presentation (the file as it was read, whole) and the keys of the changes that were rejected; the changes themselves are found
// again against it when the presentation opens (apps/deck/core/deckcompare.h), so a change that was accepted is simply no longer a difference. The part is
// named /jo/review.json (jo/review.json in an .odp): other programs do not know it and leave it alone.
//
// In the document the comparison is appData "compare": {"theirs": the hash of the other presentation's bytes, "states": {change id: "accepted" or "rejected"},
// "rejected": [keys of the rejected changes], "pending": how many changes are not decided}. appData is saved and undone with the document, so an undo brings the states back with the slides. The bytes
// themselves are not in the document (they would be copied into every step of the history): they are in a small store here, by hash.
//
// Limits (a file is hostile): the part is read only up to kMaxReviewPartBytes, the copy of the other presentation up to kMaxReviewTheirsBytes, and at most
// kMaxReviewKeys keys; what is over is left out and the comparison is made again from what was kept.

#include "core/document.h"

#include <QByteArray>
#include <QObject>
#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace jp::deck {

constexpr qint64 kMaxReviewTheirsBytes = qint64(128) << 20;
constexpr qint64 kMaxReviewPartBytes = kMaxReviewTheirsBytes * 4 / 3 + (1 << 20);   // base64 and a little more
constexpr int kMaxReviewKeys = 10000;
constexpr int kReviewStoreMost = 4;   // the other presentations (or parts not yet taken up by a window) kept in memory with no window using them

// The part is read only while a window opens its own file (a ReviewOpenScope is alive): a presentation read for any other reason (the other one of a
// comparison, slides taken from a file, the command-line tool) ignores it. It is not decoded then either: the bytes are kept as they came, size-checked,
// and decodeReviewPart() makes the other presentation of them when the window first needs the comparison.
class ReviewOpenScope {
public:
    ReviewOpenScope();
    ~ReviewOpenScope();
    ReviewOpenScope(const ReviewOpenScope &) = delete;
    ReviewOpenScope &operator=(const ReviewOpenScope &) = delete;
};

// Work done and limits in force, for tests.
struct ReviewCounters {
    int decodes = 0;       // parts decoded (inflated, parsed, base64 read, hashed, stored)
    int imports = 0;       // times a window read the other presentation to find the changes again
    int markRebuilds = 0;  // times a window made the marks of the slide again
};
ReviewCounters &reviewCounters();
void setReviewLimitsForTest(qint64 theirsBytes, qint64 partBytes);   // negative: the constants above
qint64 reviewTheirsLimit();
qint64 reviewPartLimit();

// Keeps `bytes` in the store and returns the key it is found by (empty for none, or more than the limit). An entry nobody uses is one of at most
// kReviewStoreMost kept; an entry a window uses (retainReviewTheirs) is kept as long as that window lives.
QString storeReviewTheirs(const QByteArray &bytes);
QByteArray storedReviewTheirs(const QString &key);
// `owner` (a window) uses the entry until it is destroyed; then the entry goes if no other owner has it.
void retainReviewTheirs(const QString &key, const QObject *owner);
// `owner` stops using the entry now (a window that starts another comparison lets go of its earlier copy); the entry goes if no other owner has it.
void releaseReviewTheirs(const QString &key, const QObject *owner);
int storedReviewCount();   // for tests
void clearReviewStore();   // for tests

// The part to write for `doc`: empty when it has no change pending (or the other presentation is no longer in the store). A part read from a file and not yet
// decoded is written as it was read.
QByteArray reviewPartOf(const Document &doc);
// Takes a part read from a file into `doc` (appData "compare"), undecoded; false when it is not a usable one or no ReviewOpenScope is alive.
bool readReviewPart(const QByteArray &part, Document &doc);
// Makes the comparison of a part kept by readReviewPart (appData "compare": theirs, rejected); false, and the comparison gone from `doc`, when it is not usable.
bool decodeReviewPart(Document &doc);

// The part's name in a package of the newer kind and in one of the open document kind.
QString reviewPartName();
QString reviewOdfName();

} // namespace jp::deck
