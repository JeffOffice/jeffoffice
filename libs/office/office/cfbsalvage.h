#pragma once
// Open and Repair for the old binary formats: what can be read of a compound file (the container a .doc, .ppt, or .xls lives in) whose directory or tables of sectors are damaged,
// or that is cut short. The streams the caller asks for are found three ways, the first that gives one the caller's own check accepts:
//   1. the container read through its directory and tables: the stream is read from one directory entry, by the readings in the order of trust that cfbsalvage.cpp lists, and the
//      first that ends well by the caller's own check is kept (never the longest, and never bytes from sectors that belong to another stream or are free);
//   2. a container that reads as a whole, for what its directory does not lead to;
//   3. a scan of the whole file, a sector at a time, for a place a stream could start: the caller says what the first bytes look like and checks the rest by the stream's own records.
// A stream is only accepted when the caller's check does; nothing in the damaged file is believed more than it can be checked. Every step is bounded (see CfbSalvageLimits).

#include <QByteArray>
#include <QByteArrayView>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>

#include <functional>

namespace jp::office {

// One stream wanted from the file.
struct CfbWish {
    QString name;   // the stream's name, and what it is called in the container that is written back
    // True for the name of a directory entry that is this stream (empty: the same name, in any case).
    std::function<bool(const QString &)> nameMatches;
    bool required = true;
    bool anyOffset = false;    // a scan looks at every byte of the file, not every 64th (for a stream that is not in a container at all)
    // The bytes from a place the stream could start to the end of what could be read, and the streams already found: a cheap test, reading only a few of the bytes, of whether a stream
    // could start here, for a scan (empty: no scan is made for this stream).
    std::function<bool(QByteArrayView from, const QMap<QString, QByteArray> &found)> looksLike;
    // The bytes from a place the stream starts to the end of what could be read, and the streams already found (by name). The number of bytes of the stream that read
    // soundly, `bytes.size()` if its end can't be told; 0 if this is not the stream. At most CfbSalvageLimits::maxChecks calls are made for a scan (the places that look like it).
    std::function<qint64(QByteArrayView bytes, const QMap<QString, QByteArray> &found)> validBytes;
    // Whether a reading of the stream ends well, by the stream's own structure and never by its length (empty: the stream's tables and size are all there is to go by, and a stream
    // whose tables and size do not agree is read as far as they agree). `whole` is the number of bytes at which the stream ends (its own end marker, with whole records before it),
    // -1 if it does not end well in these bytes; `prefix` is the number of bytes from the start that are whole records (units the format can check), whether or not the stream ends well.
    struct End {
        qint64 whole = -1;
        qint64 prefix = 0;
    };
    std::function<End(QByteArrayView bytes, const QMap<QString, QByteArray> &found)> endsWell;
};

// What a salvage holds and looks at. Reading the container is bounded by the file (a file over maxFileBytes is refused); a stream is held to maxStreamBytes and the streams
// together to maxTotalBytes; at most maxWishes streams are asked for; the directory is read to maxDirSectors sectors; the chain of DIFAT sectors is followed to maxDifatSectors;
// a chain of sectors is followed once through (a sector met again ends it), and at most as many sectors as the stream's size says; a scan looks at each 64th byte of the file once
// for each stream wanted (so at most maxWishes times the file's size / 64 looks, each a few bytes) and calls the caller's full check at most maxChecks times for each; at most maxEntryTries directory entries are tried for a name; and the bytes the caller's full checks are given, over all the streams, come to at most 8 times the file's size and a megabyte. A mini sector other than 64 bytes (the format's only size) makes the mini stream unread.
struct CfbSalvageLimits {
    qint64 maxFileBytes = qint64(1) << 30;
    qint64 maxStreamBytes = qint64(512) << 20;
    qint64 maxTotalBytes = qint64(1) << 30;
    int maxWishes = 8;
    int maxDirSectors = 8192;
    int maxDifatSectors = 4096;
    int maxChecks = 64;
    int maxEntryTries = 8;   // the directory entries of one stream's name that are tried
};

struct CfbSalvage {
    enum class How { Whole, Directory, Scan };
    QMap<QString, QByteArray> streams;   // by the wish's name
    QMap<QString, How> how;
    QStringList directoryNames;          // the names of every stream the directory gave, if it could be read (so a caller can see a protected package)
    bool cutShort = false;               // a stream ran past the end of the file
    bool unverified = false;             // a stream's end could not be confirmed, so only what could be checked was kept
    bool usedFree = false;               // sectors the tables mark free were read, for want of anything else
    bool tablesGuessed = false;          // a chain of sectors had to be guessed
    bool directoryRead = false;
    // Where pieces of a stream were joined by guess (the sector after a link that could not be read was taken), as offsets in the stream: a caller leaves out what lies across one.
    QMap<QString, QVector<qint64>> joins;
    // Streams (by the wish's name) that were read short or damaged, as far as their tables went (a stream the caller has no check for is read by its tables alone), and streams the directory
    // lists that nothing could be read of: a caller says that part of the file could not be recovered.
    QStringList partial, lost;
    qint64 bytesBuilt = 0, bytesChecked = 0;   // the bytes the readings made and the bytes the caller's checks were given, for a test of the bound
    bool repaired = false;               // a stream was read other than as the directory and tables give it (its size and its tables did not agree): the container was not sound
    bool miniUnread = false;             // the container's mini stream was not read (its header gives a size of mini sector the format does not have)
    QStringList notes;                   // what was done, in words
};

// Looks for the wished streams in `file`. True if every required one was found (and so `out->streams` has it); false, with `error`, if not (`out` has what was found).
bool salvageCompoundFile(const QByteArray &file, const QVector<CfbWish> &wishes, CfbSalvage *out, QString *error, const CfbSalvageLimits &limits = CfbSalvageLimits());

// A compound file made of the streams that were found (the container the readers open).
QByteArray rebuildCompoundFile(const CfbSalvage &salvage);

} // namespace jp::office
