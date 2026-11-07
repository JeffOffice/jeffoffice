#pragma once
// Minimal ZIP reader/writer for the .jpub container (stored entries) and
// EPUB books (text compressed).

#include <QByteArray>
#include <QMap>
#include <QVector>
#include <functional>
#include <QString>

namespace jp {

class ZipWriter {
public:
    // `compress` deflates the entry when that makes it smaller.
    void add(const QString &name, const QByteArray &data, bool compress = false);
    // An entry as another ZIP has it: its bytes (stored, method 0, or deflated, method 8) are put in as they are, not inflated and deflated again, so what it holds
    // is the same to the last bit; `crc`, `csize` (the bytes given), and `usize` (what they inflate to) are the other ZIP's directory's.
    void addRaw(const QString &name, quint16 method, quint32 crc, quint32 csize, quint32 usize, const char *bytes);
    // An entry written in pieces, for a part too large to hold whole: begin it, write the pieces, end it. A piece is deflated as it comes (level 6, a good deal faster than the
    // 9 that add() uses), and the sizes and CRC are put in the entry's header when it ends. Nothing else is added in between.
    void beginStream(const QString &name, bool compress = true);
    void write(const char *data, qsizetype size);
    void endStream();
    // How many bytes the ZIP is so far (what a streamed entry has written is in it).
    qint64 size() const { return m_out.size(); }
    // Every entry's record is written in the long form (the one that has room for sizes and places past 4 GiB), as a ZIP that needs it has them; for a test of the readers.
    void useLongRecords(bool on) { m_long = on; }
    QByteArray finish();
    ~ZipWriter();
    ZipWriter() = default;
    ZipWriter(const ZipWriter &) = delete;
    ZipWriter &operator=(const ZipWriter &) = delete;
private:
    struct Entry { QString name; quint32 crc; quint64 size; quint64 csize; quint16 method; quint64 offset; };
    struct Stream;
    QByteArray m_out;
    QVector<Entry> m_entries;
    Stream *m_stream = nullptr;
    bool m_long = false;
};

// Returns entries by name; empty map and false if the data is not a readable
// zip. Deflated entries are inflated (up to 1 GB each, checked by CRC).
bool readZip(const QByteArray &zip, QMap<QString, QByteArray> &out, QString *error = nullptr, qint64 maxInflated = qint64(1) << 31);
// One stored entry read straight from a file (the directory, then just that
// entry), without loading the rest; empty if it isn't there.
QByteArray readZipEntry(const QString &path, const QString &name);

// One entry found in a damaged ZIP by looking for the entries' own headers (the directory at the end of the file may be gone or wrong). `data` is what could be inflated: all of it
// when `whole` (the entry ended where it should, with the size and CRC the header gives, if it gives them), else the start of the entry up to where it broke.
struct ZipScanEntry {
    QString name;
    QByteArray data;
    bool whole = false;
    QString problem;   // when not whole: what was wrong ("cut short", "damaged", "stored with no size")
};
// Every entry that can be found, in the order the file has them. The work is bounded: at most `maxEntries` entries, `maxEntryBytes` inflated from any one, and `maxTotalBytes` from all.
// An entry that passes `maxEntryBytes` (or what is left of `maxTotalBytes`) is not kept: its `data` is empty and its `problem` is "too large".
QVector<ZipScanEntry> scanZip(const QByteArray &zip, int maxEntries = 10000, qint64 maxEntryBytes = qint64(512) << 20, qint64 maxTotalBytes = qint64(2) << 30);

// What a scan left out for want of its limits (so the caller can say so).
struct ZipScanStats {
    bool entriesLeftOut = false;   // there were entries after the `maxEntries` taken
    bool sizeLeftOut = false;      // there were entries after the point where `maxTotalBytes` was used up
};
// The same scan, one entry at a time: `each` is handed each entry as it is found, and the scan keeps none, so a caller that holds only what it needs holds one entry's bytes at a time. The
// order, the limits, and what is found are those of scanZip (and the same every time for the same bytes). `stats`, when given, says what was left out.
void scanZipEach(const QByteArray &zip, const std::function<void(ZipScanEntry &)> &each, int maxEntries = 10000, qint64 maxEntryBytes = qint64(512) << 20, qint64 maxTotalBytes = qint64(2) << 30,
                 ZipScanStats *stats = nullptr);

quint32 crc32(const QByteArray &data);

} // namespace jp
