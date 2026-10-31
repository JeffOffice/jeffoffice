#pragma once
// A ZIP read without inflating it: the directory first (what each entry is,
// where its bytes lie, and how large it says it is), then any one entry
// inflated whole or as a stream, when it is asked for. zip.h's readZip
// inflates every entry at once; this is for the packages whose parts may be
// far larger than the rest of the file needs (a spreadsheet's sheets).
//
// A directory that can't be read, an entry whose local header isn't one or that
// is cut short by the end of the file, and a method other than stored or
// deflated are refused, as readZip refuses all but the first. An entry that
// inflates to other than the size the directory gives, or whose CRC is wrong
// (a stored entry's too), is damage, found when it is inflated.

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

// The most one entry may inflate to; more is a decompression bomb. At least ten times
// the largest that real workbooks have (a sheet of 151 MB).
constexpr qint64 kMaxZipEntryBytes = qint64(2) << 30;

// Why an entry could not be given.
enum class ZipFailure { None, Damaged, TooLarge, OverBudget };

// Every entry, in the order of the directory. False if the directory can't
// be read or an entry isn't inside the data or isn't stored or deflated.
// Nothing is inflated.
bool readZipDirectory(const QByteArray &zip, QVector<ZipEntry> *out);
// Why readZipDirectory cannot read the zip (None if it can): no end-of-directory record (a copy cut short), the zip64 form, a way of packing other than stored and deflate, or a directory or
// entry that is broken (a header that is not one, data past the end of the file).
enum class ZipFault { None, NoEnd, Zip64, UnsupportedMethod, Damaged };
ZipFault zipFault(const QByteArray &zip);

// What all the entries of a package may come to when inflated, over its whole life:
// each byte that comes out of an inflater is counted, and no entry is begun that
// would take the total past `limit` (when it is not negative). So no list of parts, and
// no part asked for again and again, can multiply the work.
struct InflateBudget {
    std::atomic<qint64> inflated{0};
    qint64 limit = -1;
};
using InflateCounter = std::shared_ptr<InflateBudget>;

// The entry's bytes; false if it is damaged (a stream that ends early or runs on, a size
// other than the directory's, a wrong CRC), larger than kMaxZipEntryBytes, or would take
// the budget past its limit; `why` says which. An entry that claims more than a deflate
// stream of its size could make is damaged without anything being reserved for it.
bool inflateZipEntry(const QByteArray &zip, const ZipEntry &entry, QByteArray *out, const InflateCounter &budget = nullptr, ZipFailure *why = nullptr);

// The entry as a stream, inflated as it is read, in whatever pieces the reader
// asks for: nothing but the reader's own piece is ever held, so a part of any size
// costs no memory. It is sequential. When the stream ends it has been checked as
// inflateZipEntry checks it; if it is damaged, or is past a limit, the read fails (-1)
// and failed() says so.
class ZipEntryDevice : public QIODevice {
public:
    using Failure = ZipFailure;
    ZipEntryDevice(const QByteArray &zip, const ZipEntry &entry, const InflateCounter &budget = nullptr);
    ~ZipEntryDevice() override;
    bool isSequential() const override { return true; }
    bool failed() const { return m_failure != Failure::None; }
    Failure failure() const { return m_failure; }

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *, qint64) override { return -1; }

private:
    struct Inflater;
    bool fail(Failure why, const QString &message);

    QByteArray m_zip;   // shares the file's bytes
    ZipEntry m_entry;
    InflateCounter m_budget;
    std::unique_ptr<Inflater> m_z;
    qint64 m_out = 0;        // bytes given so far
    quint32 m_crc = 0;
    Failure m_failure = Failure::None;
    bool m_done = false;
};

} // namespace jp
