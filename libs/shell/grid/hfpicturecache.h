#pragma once
// The pictures of headers and footers decoded for drawing, kept for the whole process within a budget of bytes (the decoded pictures of every open workbook together), the one used longest ago
// let go of first. A picture is made either at once on the thread that asks (a print or an export, which wait for it anyway), or on a worker thread while the asker is given nothing and told it is
// being made (a view of the pages, which must not stop for a decode).

#include <QByteArray>
#include <QHash>
#include <QImage>
#include <QMutex>

#include <set>
#include <utility>

namespace jp::grid {

class HeaderPictureCache {
public:
    // One for the process, never destroyed (a worker may still be finishing when the program ends).
    static HeaderPictureCache &instance();
    // The decoded picture (at most 2,400 pixels on a side; null for one that can't be read, or is too large to decode). With `wait` false and the picture not made yet, it is begun on a worker
    // and a null image comes back with `*pending` set; with `wait` true it is made here.
    QImage get(const QByteArray &image, bool wait, bool *pending);
    void setBudgetBytes(qint64 bytes);
    qint64 budgetBytes() const;
    qint64 bytes() const;                 // what the kept pictures take
    int count() const;
    int pendingCount() const;             // pictures being made on workers
    int decodesHere() const;              // pictures made on the thread that asked for them, since the process began (counted, for tests)
    void waitForWorkers();
    void clear();

    // The most pictures that are being made at once.
    static constexpr int kMaxPending = 16;
    static constexpr qint64 kDefaultBudgetBytes = qint64(96) << 20;

private:
    using Key = std::pair<const char *, qsizetype>;
    struct Entry {
        QByteArray pin;    // (holds the bytes the key points into, so that the pointer is not given to another picture)
        QImage image;
        quint64 used = 0;
    };
    QImage make(const QByteArray &image) const;
    void insert(const Key &key, const QByteArray &pin, const QImage &image);
    void trim();

    mutable QMutex m_mutex;
    QHash<Key, Entry> m_entries;
    std::set<Key> m_pending;
    qint64 m_bytes = 0, m_budget = kDefaultBudgetBytes;
    quint64 m_tick = 0;
    int m_here = 0;
};

} // namespace jp::grid

