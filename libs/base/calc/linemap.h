#pragma once
// The order of a sheet's rows (or columns) as ids: which id is at which position, and the other way, kept as runs of consecutive ids (design: docs/jeffcrunch-rowids.md, 4.2).
// A line (a row or a column) gets an id when the sheet is opened (its position in the file) or when it is inserted (the next id), and keeps it until the workbook is closed;
// a deleted line leaves the order but its id is remembered with the lines that were on either side of it at that moment (the "dead-end memory"), so that a range whose end
// was deleted can still be resolved, and Undo can put the same ids back. Nothing here knows about cells or formulas.
//
// Costs, in runs R (a few dozen after a session of edits, thousands after a long one), never in lines: an id at a position and the position of an id O(log R), or O(1) with a
// reader's hint; a walk of a range O(log R + the runs it covers); an insert or an erase moves O(R) entries of 12 bytes (a run is 24 bytes: one entry in the order of position and
// one in the order of id). The map is changed in place, never built again. A sheet that has had no edit has no runs and allocates nothing (identity: id equals position).
//
// Threads: the map changes only on the window's thread while readers are held off (the workbook's hold); every const function may be called from any number of threads at once
// (a reader's hint is its own, passed in; the cache of runs for ranges is locked). A hint is valid only for the version it saw; the map checks it against what it holds now.
//
// Ids and positions are different types (RowId, RowPos, ColId, ColPos) with no conversion between them or from a bare number: an id is a plausible number and a mix-up would be silent.
// No GUI; uses only the standard library and QtCore (for the words of a refusal).

#include <QString>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

namespace jp::calc {

// A number that is an id or a position of one kind of line. Explicit to make, explicit to read, and the kinds do not mix.
template <class Tag>
class LineIndex {
public:
    static constexpr uint32_t kNoneValue = 0xFFFFFFFFu;
    constexpr LineIndex() : m_v(kNoneValue) {}
    explicit constexpr LineIndex(uint32_t v) : m_v(v) {}
    static constexpr LineIndex none() { return LineIndex(); }
    constexpr bool isNone() const { return m_v == kNoneValue; }
    constexpr uint32_t value() const { return m_v; }
    friend constexpr bool operator==(LineIndex a, LineIndex b) { return a.m_v == b.m_v; }
    friend constexpr bool operator!=(LineIndex a, LineIndex b) { return a.m_v != b.m_v; }
    friend constexpr bool operator<(LineIndex a, LineIndex b) { return a.m_v < b.m_v; }
    friend constexpr bool operator<=(LineIndex a, LineIndex b) { return a.m_v <= b.m_v; }
    friend constexpr bool operator>(LineIndex a, LineIndex b) { return a.m_v > b.m_v; }
    friend constexpr bool operator>=(LineIndex a, LineIndex b) { return a.m_v >= b.m_v; }

private:
    uint32_t m_v;
};

struct RowIdTag {};
struct RowPosTag {};
struct ColIdTag {};
struct ColPosTag {};
using RowId = LineIndex<RowIdTag>;
using RowPos = LineIndex<RowPosTag>;
using ColId = LineIndex<ColIdTag>;
using ColPos = LineIndex<ColPosTag>;

// Consecutive ids, `count` of them from `first`.
struct IdRun {
    uint32_t first = 0;
    uint32_t count = 0;
    friend bool operator==(const IdRun &a, const IdRun &b) { return a.first == b.first && a.count == b.count; }
};

// The caps of one kind of line. A sheet's live lines never pass `capacity`; ids from `firstNew` up go to inserted lines and stop at `idLimit`; at `maxRuns` (live runs, and
// dead records, each) an edit is refused; `rebaseRuns` and four fifths of the spare ids are where a rebase is wanted (4.16).
struct LineLimits {
    uint32_t capacity = 1048576;
    uint32_t firstNew = 1u << 20;
    uint32_t idLimit = 1u << 24;
    uint32_t maxRuns = 1000000;
    uint32_t rebaseRuns = 100000;
    static LineLimits rows() { return {1048576u, 1u << 20, 1u << 24, 1000000u, 100000u}; }   // (a row id is 24 bits in the session's cells)
    static LineLimits cols() { return {16384u, 1u << 14, 1u << 16, 1000000u, 100000u}; }     // (a column id is 16 bits)
};

enum class LineStatus : uint8_t {
    Ok,
    OutOfRange,     // positions or a count that name lines the map does not have, or an insert that would put new lines past the sheet's end
    TooManyRuns,    // the map holds as many runs or dead records as it may
    IdsExhausted,   // there are not enough unused ids
    StaleStep,      // an undo or a redo that is not the next one in last-in-first-out order (or was dropped, or is from before a rebase)
    StillOverCap,   // a rebase would leave the map over a cap
};
// The words for a person ("This sheet has been edited too many times. Save it and open it again." for the caps); empty for Ok.
QString lineStatusText(LineStatus s);

// Work done, counted instead of timed, so that a test can say what a call cost. A reader passes one in; the map keeps one for its last change (lastWork()).
struct LineWork {
    uint64_t comparisons = 0;   // probes of a search, and checks of a hint
    uint64_t runsVisited = 0;   // runs a walk looked at
    uint64_t entriesMoved = 0;  // entries of the map moved or rewritten by a change
    uint64_t hops = 0;          // steps along a chain of dead records
};

// A reader's memory of the run it used last: valid for the version it saw, used only if it still holds the line wanted.
struct LineHint {
    uint64_t version = ~uint64_t(0);
    uint32_t pos = 0;   // run (in the order of position) the last position lookup ended in
    uint32_t id = 0;    // entry (in the order of id) the last id lookup ended in
};

// What one insert or erase did, enough to undo it and redo it; kept by the caller as the step's record (a few runs, never a copy of any cell).
struct LineEdit {
    enum class Kind : uint8_t { Insert, Erase };
    Kind kind = Kind::Insert;
    uint32_t at = 0;                   // position of the first line inserted or erased
    std::vector<IdRun> runs;           // the ids inserted (one run), or erased (the pieces, in order of position)
    uint32_t pushedAt = 0;             // an insert that pushes lines off the end of the sheet: where they were,
    std::vector<IdRun> pushed;         //   and which (they die with the insert and come back with its Undo)
    uint64_t before = 0, after = 0;    // the state of the order before the edit and after it: a step is undone only in the state it left
    bool forgotten = false;
};

// A dead record, as the order remembers it: the ids, and the live ids on either side of them when they died (kNone for none).
struct LineDead {
    uint32_t first, count, before, after;
    friend bool operator==(const LineDead &a, const LineDead &b) { return a.first == b.first && a.count == b.count && a.before == b.before && a.after == b.after; }
};

// The core, on bare numbers; use LineMap below, which has the typed interface.
class LineMapCore {
public:
    static constexpr uint32_t kNone = 0xFFFFFFFFu;
    using Runs = std::shared_ptr<const std::vector<IdRun>>;
    struct Renumber {
        uint32_t oldFirst, count, newFirst;   // after a rebase the live ids [oldFirst, +count) are [newFirst, +count)
    };

    // `live` lines (ids 0 .. live - 1, in order), at most limits.capacity and at most limits.firstNew.
    LineMapCore(const LineLimits &limits, uint32_t live);
    LineMapCore(const LineMapCore &other);
    LineMapCore &operator=(const LineMapCore &other);
    LineMapCore(LineMapCore &&) noexcept;
    LineMapCore &operator=(LineMapCore &&) noexcept;
    ~LineMapCore();

    uint32_t size() const { return m_live; }
    bool isIdentity() const { return m_identity; }
    size_t runCount() const { return m_byPos.size(); }
    size_t deadCount() const { return m_dead.size(); }
    uint64_t version() const { return m_version; }       // up by one at every change of the order or of the records
    uint64_t state() const { return m_state; }           // names the state of the order along the history of edits (see LineEdit::before)
    uint32_t nextId() const { return m_next; }
    const LineLimits &limits() const { return m_limits; }
    size_t runBytes() const { return m_byPos.size() * 2 * sizeof(PosRun); }    // 24 per run
    size_t allocatedBytes() const { return m_byPos.capacity() * sizeof(PosRun) + m_byId.capacity() * sizeof(IdEntry) + m_dead.capacity() * sizeof(LineDead); }
    bool needsRebase() const;
    const LineWork &lastWork() const { return m_work; }

    // The id at a position (kNone past the end) and the position of an id (kNone if it is not live).
    uint32_t idAt(uint32_t pos, LineHint *hint = nullptr, LineWork *work = nullptr) const;
    uint32_t posOf(uint32_t id, LineHint *hint = nullptr, LineWork *work = nullptr) const;
    bool isLive(uint32_t id) const { return posOf(id) != kNone; }
    // Whether the id is of a line that was deleted and that a step could still bring back (a record of it is held).
    bool holdsDead(uint32_t id) const { return deadFor(id, nullptr) != nullptr; }

    LineStatus insert(uint32_t at, uint32_t count, LineEdit *edit);
    LineStatus erase(uint32_t at, uint32_t count, LineEdit *edit);
    LineStatus undo(LineEdit &edit);
    LineStatus redo(LineEdit &edit);
    // Lets go of a step that will never be undone or redone (the redo steps a new edit drops), newest first. An insert that was undone gives its ids back if they are the
    // highest ever taken; returns false (and does nothing) for an insert that is still applied. The step is stale afterward.
    bool forget(LineEdit &edit);
    // Whether an insert of `count` at `at` would push lines off the end (and how many).
    uint32_t pushOff(uint32_t count) const { return m_live + uint64_t(count) > m_limits.capacity ? uint32_t(m_live + uint64_t(count) - m_limits.capacity) : 0; }

    // Dead references (4.4): an id that is not live is resolved to the live id that followed it, or preceded it, when it died; kNone if there is none.
    uint32_t resolveStart(uint32_t id, LineWork *work = nullptr) const;
    uint32_t resolveEnd(uint32_t id, LineWork *work = nullptr) const;
    // A range given by the ids at its ends, as positions; false if it is not a range any more (an end gone, or the start after the end).
    bool resolveRange(uint32_t startId, uint32_t endId, uint32_t *first, uint32_t *last, LineWork *work = nullptr) const;
    // A whole row or column: the first and the last live position (false if there is none).
    bool wholeRange(uint32_t *first, uint32_t *last) const;
    // The original lines (the ids below limits.firstNew, which are in the order of position whatever is inserted among them) next to a position: the id of the last original line at a live
    // position up to `pos`, and of the first at one from `pos` on; kNone if there is none. The graph tells a reader of an inserted line as the original line at that end, and asks about a
    // change in an inserted line as the original lines on either side of it (docs/jeffcrunch-rowids.md, 4.6). Cost: a search, and a step for each run of inserted lines passed over.
    uint32_t origBefore(uint32_t pos) const;
    uint32_t origAfter(uint32_t pos) const;

    // The id runs that the positions first..last cover, in order; `out` is cleared first.
    void walk(uint32_t first, uint32_t last, std::vector<IdRun> *out, LineWork *work = nullptr) const;
    // The same, from a cache kept for the version of the map (a range read by a million formulas is walked once); shared, so it outlives the cache.
    Runs runsFor(uint32_t first, uint32_t last) const;
    uint64_t cacheHits() const;
    uint64_t cacheMisses() const;
    size_t cacheEntries() const;

    // The records of the lines at these ids are let go of (nothing can name them, or bring them back, any more: the caller has dropped the step that could). Ids already live are left alone.
    void dropDead(const std::vector<IdRun> &runs);
    // Back to identity (no map at all) if the order is that already: no records of dead lines, and the live lines are the ids from 0, in order. True if it is identity now.
    bool tryIdentity();

    // Renumbers the live ids to their positions and forgets the dead ones (4.16): the caller renumbers whatever holds ids with `table`. Every older step is stale after it.
    LineStatus rebase(std::vector<Renumber> *table);

    // Checks the map's own bookkeeping (counts, order, links, the dead records); for tests.
    bool selfCheck() const;
    std::vector<LineDead> deadRecords() const { return m_dead; }
    std::vector<IdRun> liveRuns() const;

private:
    struct PosRun {
        uint32_t first, count, pre;   // pre: lines before this run
    };
    struct IdEntry {
        uint32_t first, count, pre;   // the same run as in m_byPos, found by its ids (pre is kept equal to the run's)
    };
    struct Cache;
    void materialize();
    size_t runAtPos(uint32_t pos, LineWork *w) const;
    size_t entryAtId(uint32_t id, LineWork *w) const;
    const LineDead *deadFor(uint32_t id, LineWork *w) const;
    size_t splitAt(uint32_t pos);
    void eraseRuns(size_t r0, size_t r1);
    void insertRuns(size_t r, const std::vector<IdRun> &runs);
    void tryMerge(size_t r);
    void kill(uint32_t at, const std::vector<IdRun> &pieces);
    void revive(uint32_t at, const std::vector<IdRun> &pieces);
    void pieces(uint32_t at, uint32_t count, std::vector<IdRun> *out) const;
    bool overSoftCap(size_t runs, uint64_t idsUsed) const;
    uint32_t resolve(uint32_t id, bool wantAfter, LineWork *w) const;

    LineLimits m_limits;
    uint32_t m_live = 0;
    bool m_identity = true;
    std::vector<PosRun> m_byPos;
    std::vector<IdEntry> m_byId;
    std::vector<LineDead> m_dead;   // sorted by first
    uint32_t m_next = 0;
    uint64_t m_version = 0;
    uint64_t m_state = 1;
    uint64_t m_stateSeq = 1;
    LineWork m_work;
    std::unique_ptr<Cache> m_cache;
};

// The typed map: LineMap<RowId, RowPos> for a sheet's rows, LineMap<ColId, ColPos> for its columns.
template <class Id, class Pos>
class LineMap {
public:
    struct Run {
        Id first;
        uint32_t count;
    };
    struct PosRange {
        Pos first, last;
    };
    // A step's record, with its positions and ids typed.
    class Edit {
    public:
        bool isInsert() const { return m_e.kind == LineEdit::Kind::Insert; }
        Pos at() const { return Pos(m_e.at); }
        std::vector<Run> runs() const { return typed(m_e.runs); }
        std::vector<Run> pushed() const { return typed(m_e.pushed); }
        uint32_t count() const
        {
            uint32_t n = 0;
            for (const IdRun &r : m_e.runs) n += r.count;
            return n;
        }
        LineEdit &raw() { return m_e; }
        const LineEdit &raw() const { return m_e; }

    private:
        static std::vector<Run> typed(const std::vector<IdRun> &v)
        {
            std::vector<Run> out;
            out.reserve(v.size());
            for (const IdRun &r : v) out.push_back({Id(r.first), r.count});
            return out;
        }
        LineEdit m_e;
    };

    LineMap(const LineLimits &limits, uint32_t live) : m_core(limits, live) {}

    uint32_t size() const { return m_core.size(); }
    bool isIdentity() const { return m_core.isIdentity(); }
    size_t runCount() const { return m_core.runCount(); }
    uint64_t version() const { return m_core.version(); }
    uint64_t state() const { return m_core.state(); }
    Id nextId() const { return Id(m_core.nextId()); }
    bool needsRebase() const { return m_core.needsRebase(); }
    const LineWork &lastWork() const { return m_core.lastWork(); }
    LineMapCore &core() { return m_core; }
    const LineMapCore &core() const { return m_core; }

    Id idAt(Pos p, LineHint *hint = nullptr, LineWork *w = nullptr) const { return Id(m_core.idAt(p.value(), hint, w)); }
    Pos posOf(Id id, LineHint *hint = nullptr, LineWork *w = nullptr) const { return Pos(m_core.posOf(id.value(), hint, w)); }
    bool isLive(Id id) const { return m_core.isLive(id.value()); }

    LineStatus insert(Pos at, uint32_t count, Edit *edit) { return m_core.insert(at.value(), count, edit ? &edit->raw() : nullptr); }
    LineStatus erase(Pos at, uint32_t count, Edit *edit) { return m_core.erase(at.value(), count, edit ? &edit->raw() : nullptr); }
    LineStatus undo(Edit &edit) { return m_core.undo(edit.raw()); }
    LineStatus redo(Edit &edit) { return m_core.redo(edit.raw()); }
    bool forget(Edit &edit) { return m_core.forget(edit.raw()); }
    uint32_t pushOff(uint32_t count) const { return m_core.pushOff(count); }

    Id resolveStart(Id id, LineWork *w = nullptr) const { return Id(m_core.resolveStart(id.value(), w)); }
    Id resolveEnd(Id id, LineWork *w = nullptr) const { return Id(m_core.resolveEnd(id.value(), w)); }
    bool resolveRange(Id startId, Id endId, PosRange *out, LineWork *w = nullptr) const
    {
        uint32_t a, b;
        if (!m_core.resolveRange(startId.value(), endId.value(), &a, &b, w)) return false;
        if (out) *out = {Pos(a), Pos(b)};
        return true;
    }
    bool wholeRange(PosRange *out) const
    {
        uint32_t a, b;
        if (!m_core.wholeRange(&a, &b)) return false;
        if (out) *out = {Pos(a), Pos(b)};
        return true;
    }
    Id origBefore(Pos p) const { return Id(m_core.origBefore(p.value())); }
    Id origAfter(Pos p) const { return Id(m_core.origAfter(p.value())); }

    std::vector<Run> walk(Pos first, Pos last, LineWork *w = nullptr) const
    {
        std::vector<IdRun> raw;
        m_core.walk(first.value(), last.value(), &raw, w);
        std::vector<Run> out;
        out.reserve(raw.size());
        for (const IdRun &r : raw) out.push_back({Id(r.first), r.count});
        return out;
    }
    // The cached walk, as the bare runs (first id, count) the cell store's range walk is built on.
    LineMapCore::Runs runsFor(Pos first, Pos last) const { return m_core.runsFor(first.value(), last.value()); }

    LineStatus rebase(std::vector<LineMapCore::Renumber> *table) { return m_core.rebase(table); }

private:
    LineMapCore m_core;
};

using RowMap = LineMap<RowId, RowPos>;
using ColMap = LineMap<ColId, ColPos>;

static_assert(!std::is_convertible_v<RowId, RowPos> && !std::is_convertible_v<RowPos, RowId> && !std::is_convertible_v<ColId, ColPos> && !std::is_convertible_v<RowId, ColId>, "ids and positions do not convert");
static_assert(!std::is_convertible_v<uint32_t, RowId> && !std::is_convertible_v<RowPos, uint32_t> && !std::is_constructible_v<RowPos, RowId>, "nor do bare numbers");

} // namespace jp::calc
