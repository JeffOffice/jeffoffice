#pragma once
// Minimal ZIP reader/writer for the .jpub container (stored entries) and
// EPUB books (text compressed).

#include <QByteArray>
#include <QMap>
#include <QVector>
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
    QByteArray finish();
    ~ZipWriter();
    ZipWriter() = default;
    ZipWriter(const ZipWriter &) = delete;
    ZipWriter &operator=(const ZipWriter &) = delete;
private:
    struct Entry { QString name; quint32 crc; quint32 size; quint32 csize; quint16 method; quint32 offset; };
    struct Stream;
    QByteArray m_out;
    QVector<Entry> m_entries;
    Stream *m_stream = nullptr;
};

// Returns entries by name; empty map and false if the data is not a readable
// zip. Deflated entries are inflated (up to 1 GB each, checked by CRC).
bool readZip(const QByteArray &zip, QMap<QString, QByteArray> &out, QString *error = nullptr);
// One stored entry read straight from a file (the directory, then just that
// entry), without loading the rest; empty if it isn't there.
QByteArray readZipEntry(const QString &path, const QString &name);

quint32 crc32(const QByteArray &data);

} // namespace jp
