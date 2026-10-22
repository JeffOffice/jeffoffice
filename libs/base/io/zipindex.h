#pragma once
// A ZIP read without inflating it: the directory first (what each entry is,
// where its bytes lie, and how large it says it is), then any one entry
// inflated whole or as a stream, when it is asked for. zip.h's readZip
// inflates every entry at once; this is for the packages whose parts may be
// far larger than the rest of the file needs (a spreadsheet's sheets).
//
// The refusals are readZip's: a directory that can't be read, an entry
// cut short by the end of the file, a method other than stored or deflated.
// An entry that inflates to other than the size the directory gives, or
// whose CRC is wrong, is damage, found when it is inflated.

#include <QByteArray>
#include <QIODevice>
#include <QString>
#include <QVector>

#include <atomic>
#include <memory>

namespace jp {

// One entry of the directory.
struct ZipEntry {
    QString name;
    quint16 method = 0;   // 0 stored, 8 deflated
    quint32 crc = 0;
    qint64 csize = 0;     // bytes in the file
    qint64 usize = 0;     // bytes once inflated, as the directory says (a stored entry is csize)
    qint64 dataAt = 0;    // where its bytes start in the file
};

// The most one entry may inflate to; more is a decompression bomb. As readZip.
constexpr qint64 kMaxZipEntryBytes = qint64(1) << 30;

// Every entry, in the order of the directory. False if the directory can't
// be read or an entry isn't inside the data or isn't stored or deflated.
// Nothing is inflated.
bool readZipDirectory(const QByteArray &zip, QVector<ZipEntry> *out);

// What the work of inflating comes to, for a caller that wants to count it:
// every byte that came out of an inflater.
using InflateCounter = std::shared_ptr<std::atomic<qint64>>;

// The entry's bytes; false if it is damaged (a stream that ends early or runs
// on, a size other than the directory's, a wrong CRC; a stored entry is
// taken as it is) or larger than kMaxZipEntryBytes.
bool inflateZipEntry(const QByteArray &zip, const ZipEntry &entry, QByteArray *out, const InflateCounter &counter = nullptr);

// The entry as a stream, inflated as it is read, in whatever pieces the reader
// asks for: nothing but the reader's own piece is ever held. It is
// sequential. When the stream ends it has been checked as inflateZipEntry
// checks it; if it is damaged the read fails (-1) and failed() says so.
class ZipEntryDevice : public QIODevice {
public:
    ZipEntryDevice(const QByteArray &zip, const ZipEntry &entry, const InflateCounter &counter = nullptr);
    ~ZipEntryDevice() override;
    bool isSequential() const override { return true; }
    bool failed() const { return m_failed; }

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *, qint64) override { return -1; }

private:
    struct Inflater;
    bool fail(const QString &why);

    QByteArray m_zip;   // shares the file's bytes
    ZipEntry m_entry;
    InflateCounter m_counter;
    std::unique_ptr<Inflater> m_z;
    qint64 m_out = 0;        // bytes given so far
    quint32 m_crc = 0;
    bool m_done = false, m_failed = false;
};

} // namespace jp
