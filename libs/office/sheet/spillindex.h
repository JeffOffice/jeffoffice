// The blocks that dynamic arrays fill, kept as rectangles and not as cells (the spill index). A formula that spills holds its result with its own cell; the cells of its block are not in the
// cell store, and a read of the store (CellStore::find, forEachInRange, forEach) answers for them from this index: a cell of a block that has no cell of its own is given as a member of the
// formula (ArrayMember with SpillMember, `formula` the formula's place in the sheet's list), and one that has a cell (a format, or a value a person typed) is that cell. The calculation worker changes
// only its own table (SpillPublisher::mine) and publishes an immutable copy of it at the end of a slice; every other thread reads the copy it is given (SpillPublisher::view), and a copy of a
// store (a snapshot) keeps the one it was made with. The blocks of one sheet never overlap: a formula whose block would meet another's is blocked, as one whose block meets a cell with content.
#pragma once

#include <QtGlobal>

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace jp::office::sheet {

class MutationGuard;

// Whether a calculation that loads from now on keeps the cells a formula spills into as blocks in this index (true) or as cells in the store (false, the default). A trial switch: the environment
// variable JO_SPILL_INDEX=1 turns it on for a process; a test sets it with SpillIndexSwitch.
bool spillIndexDefault();
void setSpillIndexDefault(bool on);
struct SpillIndexSwitch {
    bool before;
    explicit SpillIndexSwitch(bool on) : before(spillIndexDefault()) { setSpillIndexDefault(on); }
    ~SpillIndexSwitch() { setSpillIndexDefault(before); }
};

struct SpillRect {
    quint32 row1 = 0, col1 = 0, row2 = 0, col2 = 0;
    qint32 formula = -1;   // the formula's place in Sheet::formulas; its cell is the first of the block
    bool contains(quint32 row, quint32 col) const { return row >= row1 && row <= row2 && col >= col1 && col <= col2; }
    bool meets(quint32 r1, quint32 c1, quint32 r2, quint32 c2) const { return row1 <= r2 && r1 <= row2 && col1 <= c2 && c1 <= col2; }
};

// A set of rectangles, found by the places they cover: a grid of buckets of kBlockRows rows by kBlockCols columns, each holding the rectangles that touch it. Blocks of one sheet never overlap, so a bucket
// holds few (at most kBlockRows * kBlockCols / 2), however many blocks there are or where: 500 one-column blocks in a header row, or 200 columns of 5,000 rows, look at 8 or fewer to answer `at`.
// A rectangle that would be in more than kMaxBuckets buckets is refused (a block is at most kMaxSpillCells cells, which is 3,200 buckets at the worst, a single column), and so is one that would take
// the table past kMaxTableBuckets in all (the table is copied each time it is published: 16,000 blocks of the most a block can be would be a gigabyte).
class SpillTable {
public:
    static constexpr quint32 kBlockRows = 64;
    static constexpr quint32 kBlockCols = 8;
    static constexpr qint64 kMaxBuckets = 4096;
    static constexpr qint64 kMaxTableBuckets = 1 << 18;   // (about 25 MB, and 83 blocks of the most a block can be)

    bool empty() const { return m_byFormula.empty(); }
    size_t size() const { return m_byFormula.size(); }
    quint32 lastRow() const { return m_lastRow; }   // the last row and column of any block (never less: a block let go of does not lower them)
    quint32 lastCol() const { return m_lastCol; }
    // The block that holds the place, or null. `probes`, if given, is added the number of blocks looked at.
    const SpillRect *at(quint32 row, quint32 col, qint64 *probes = nullptr) const;
    const SpillRect *ofFormula(qint32 formula) const;
    // Puts the formula's block (taking away the one it had). False, and the formula has no block, when the rectangle is in more than kMaxBuckets buckets, or the table would be in more than kMaxTableBuckets.
    bool set(const SpillRect &r);
    void erase(qint32 formula);
    void clear() { *this = SpillTable(); }
    // Whether a block of another formula than `except` meets the rectangle (stops at the first).
    bool meetsOther(quint32 r1, quint32 c1, quint32 r2, quint32 c2, qint32 except, qint64 *probes = nullptr) const;
    // Calls f(rect) once for each block that meets the rectangle; f may give false to stop.
    template <class F>
    void forEachMeeting(quint32 r1, quint32 c1, quint32 r2, quint32 c2, F f, qint64 *probes = nullptr) const
    {
        const quint32 rb1 = r1 / kBlockRows, rb2 = r2 / kBlockRows, cb1 = c1 / kBlockCols, cb2 = c2 / kBlockCols;
        bool go = true;
        auto visit = [&](quint32 rb, quint32 cb, const std::vector<qint32> &list) {
            for (const qint32 i : list) {
                if (probes) ++*probes;
                const SpillRect &r = m_rects[size_t(i)];
                if (!r.meets(r1, c1, r2, c2)) continue;
                if (std::max(r.row1 / kBlockRows, rb1) != rb || std::max(r.col1 / kBlockCols, cb1) != cb) continue;   // (met in the first bucket of the range that it is in only)
                if (!call(f, r)) {
                    go = false;
                    return;
                }
            }
        };
        const qint64 span = (qint64(rb2) - rb1 + 1) * (qint64(cb2) - cb1 + 1);
        if (span > qint64(m_buckets.size())) {
            for (const auto &[key, list] : m_buckets) {
                const quint32 rb = quint32(key >> 32), cb = quint32(key & 0xFFFFFFFFu);
                if (rb >= rb1 && rb <= rb2 && cb >= cb1 && cb <= cb2) visit(rb, cb, list);
                if (!go) return;
            }
        } else {
            for (quint32 rb = rb1; rb <= rb2 && go; ++rb)
                for (quint32 cb = cb1; cb <= cb2 && go; ++cb) {
                    const auto at = m_buckets.find(key(rb, cb));
                    if (at != m_buckets.end()) visit(rb, cb, at->second);
                }
        }
    }
    // The blocks, in no order.
    template <class F>
    void forEach(F f) const
    {
        for (const auto &[formula, i] : m_byFormula) f(m_rects[size_t(i)]);
    }

private:
    static quint64 key(quint32 rb, quint32 cb) { return (quint64(rb) << 32) | cb; }
    template <class F>
    static bool call(F &f, const SpillRect &r)
    {
        if constexpr (std::is_void_v<decltype(f(r))>) {
            f(r);
            return true;
        } else {
            return f(r);
        }
    }
    std::vector<SpillRect> m_rects;
    std::vector<qint32> m_free;
    std::unordered_map<qint32, qint32> m_byFormula;
    std::unordered_map<quint64, std::vector<qint32>> m_buckets;
    quint32 m_lastRow = 0, m_lastCol = 0;
    qint64 m_bucketsInAll = 0;   // (the places of rectangles in buckets, all the table's)
};

// What a reader has of a sheet's blocks: the table (null for none) and what keeps it alive.
struct SpillView {
    const SpillTable *table = nullptr;
    std::shared_ptr<const SpillTable> keep;
    explicit operator bool() const { return table != nullptr; }
    // A copy that no thread changes (for a snapshot of the store).
    std::shared_ptr<const SpillTable> frozen() const;
};

// The table of one sheet: the worker's own (`mine`, changed by the thread that works formulas out, or by one that has it held back) and the copy the others read.
class SpillPublisher {
public:
    explicit SpillPublisher(const MutationGuard *guard) : m_guard(guard) {}
    SpillTable &mine() { return m_mine; }
    // Makes the worker's own table what `table` holds (none: empty), for a store that is assigned another.
    void follow(const SpillTable *table);
    const SpillTable &mine() const { return m_mine; }
    // Makes what `mine` holds the copy the others read, if it changed since.
    void publish();
    void noteChanged()
    {
        m_changed = true;
        m_live.store(true, std::memory_order_relaxed);
    }
    // What the calling thread reads: the worker's own table on the worker's thread, or when no worker serves; the published copy on any other.
    SpillView view() const;
    std::shared_ptr<const SpillTable> published() const;
    qint64 publications() const { return m_publications.load(std::memory_order_relaxed); }

private:
    const MutationGuard *m_guard;
    SpillTable m_mine;
    bool m_changed = false;
    mutable std::mutex m_lock;
    std::shared_ptr<const SpillTable> m_published;
    std::atomic<bool> m_anyPublished{false};
    std::atomic<bool> m_live{false};   // (a block was ever put: before that no thread has anything to read)
    std::atomic<qint64> m_publications{0};
};

} // namespace jp::office::sheet
