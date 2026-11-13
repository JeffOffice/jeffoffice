#pragma once
// The compressed form the spelling dictionaries and the thesaurus have in an install (tools/dictpack.cpp makes them at build time from
// the plain files of resources/dict; the repository keeps the plain files).
//   A packed file ("name.dic.jz"): "JZD1", the size of the plain file (4 bytes, little endian), and the zlib stream of it.
//   The packed thesaurus ("th_en_US_v2.dat.jz"): "JZT1", then the entries in blocks of about 64 KB, each its own zlib stream, with a table
//   of the blocks first, so a lookup reads and inflates one block and the whole thesaurus is never in memory.
// Inflating is bounded: the plain size a file claims is checked against a cap before anything is allocated, and the stream must end
// at exactly that size, so a damaged or hostile file makes the call fail (nothing is returned) and never grows memory.

#include <QByteArray>
#include <QCache>
#include <QMutex>
#include <QString>
#include <QVector>

namespace jp::dictpack {

constexpr qint64 kMaxFile = 64 << 20;    // the most a packed dictionary may inflate to (the largest shipped is 9 MB)
constexpr qint64 kMaxBlock = 4 << 20;    // the most a block of the packed thesaurus may inflate to (they are about 64 KB)
constexpr qint64 kMaxEntry = 1 << 20;    // the most one thesaurus entry may hold
constexpr const char *kSuffix = ".jz";   // what a packed file's name adds

// `plain` as a packed file; empty when zlib fails (or `plain` is over the cap).
QByteArray pack(const QByteArray &plain, int level = 9);
// The plain bytes of a packed file; false (and `out` empty) when it is not one, is damaged or cut short, or inflates to more than `cap`.
bool unpack(const QByteArray &packed, QByteArray &out, qint64 cap = kMaxFile);

// The thesaurus (a MyThes .dat: an encoding line, then "word|count" and `count` lines of "(pos)|synonym|..." for each word) as the
// packed thesaurus. Each word's entry (its lines, as the file has them) is kept under the word lowercased, the last of equal ones
// winning; empty when the file is not a thesaurus or an entry is over kMaxEntry.
QByteArray packThesaurus(const QByteArray &dat, int level = 9);

// A packed thesaurus, from a file or from memory. entry() is the lines of one word's entry (the key is the word lowercased, as UTF-8),
// or empty when the word has none or its block is damaged. Safe to call from several threads.
class Thesaurus {
public:
    bool open(const QString &path);
    bool openMemory(const QByteArray &packed);
    void close();
    bool isOpen() const { return !m_blocks.isEmpty(); }
    QByteArray entry(const QByteArray &key) const;
    int blockCount() const { return int(m_blocks.size()); }

private:
    struct Block {
        QByteArray firstKey;
        quint32 rawSize = 0, compSize = 0;
        qint64 offset = 0;   // in the file, or in m_mem
    };
    bool parse(const QByteArray &head, qint64 fileSize);
    QByteArray readBlock(int index) const;   // the inflated block, from the cache or the file
    QString m_path;
    QByteArray m_mem;
    QVector<Block> m_blocks;
    mutable QMutex m_mutex;
    mutable QCache<int, QByteArray> m_cache{1 << 20};
};

} // namespace jp::dictpack
