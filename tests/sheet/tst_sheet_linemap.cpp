// Tests of the order of a sheet's rows and columns as ids (calc/linemap.h): the id at a position and the position of an id; inserting and erasing; the memory of dead ids and the
// answers for a range whose end was deleted; Undo and Redo as inverse operations; the cache of runs for a range; the caps and the rebase; and the reads made while the map is held.
// The map is checked against a plain vector of ids that is changed the obvious way. What an operation cost is counted (searches, runs visited, entries moved), never timed, and
// shown not to depend on the number of lines.
#include "calc/linemap.h"

#include "../testrunner.h"

#include <QRandomGenerator>
#include <QtTest>

#include <atomic>
#include <map>
#include <set>
#include <thread>

using namespace jp::calc;

namespace {

constexpr uint32_t kNone = LineMapCore::kNone;

// ------------------------------------------------------------------------------------------ the plain model

struct Step {
    LineEdit e;
    std::vector<uint32_t> ids, pushed;
};

struct Ref {
    LineLimits lim;
    std::vector<uint32_t> ids;                                   // the live ids in order of position
    std::map<uint32_t, std::pair<uint32_t, uint32_t>> dead;      // dead id -> (live id before, live id after) when it died
    uint32_t next;

    Ref(const LineLimits &l, uint32_t live) : lim(l), next(l.firstNew)
    {
        for (uint32_t i = 0; i < std::min({live, l.capacity, l.firstNew}); ++i) ids.push_back(i);
    }
    void kill(uint32_t at, uint32_t n)
    {
        const uint32_t before = at > 0 ? ids[at - 1] : kNone, after = at + n < ids.size() ? ids[at + n] : kNone;
        for (uint32_t i = 0; i < n; ++i) dead[ids[at + i]] = {before, after};
        ids.erase(ids.begin() + at, ids.begin() + at + n);
    }
    void revive(uint32_t at, const std::vector<uint32_t> &v)
    {
        for (uint32_t id : v) dead.erase(id);
        ids.insert(ids.begin() + at, v.begin(), v.end());
    }
    uint32_t pushOff(uint32_t count) const { return ids.size() + count > lim.capacity ? uint32_t(ids.size() + count - lim.capacity) : 0; }
    bool canInsert(uint32_t at, uint32_t count) const { return count > 0 && at <= ids.size() && at + uint64_t(count) <= lim.capacity; }
    Step insert(uint32_t at, uint32_t count)
    {
        Step s;
        const uint32_t over = pushOff(count);
        const uint32_t pushedAt = uint32_t(ids.size()) - over;
        for (uint32_t i = 0; i < over; ++i) s.pushed.push_back(ids[pushedAt + i]);
        if (over) kill(pushedAt, over);
        for (uint32_t i = 0; i < count; ++i) s.ids.push_back(next + i);
        revive(at, s.ids);
        next += count;
        return s;
    }
    Step erase(uint32_t at, uint32_t count)
    {
        Step s;
        s.ids.assign(ids.begin() + at, ids.begin() + at + count);
        kill(at, count);
        return s;
    }
    // (the same moves the map makes for Undo, Redo, and dropping a step)
    void undo(const Step &s)
    {
        if (s.e.kind == LineEdit::Kind::Insert) {
            kill(s.e.at, uint32_t(s.ids.size()));
            if (!s.pushed.empty()) revive(s.e.pushedAt, s.pushed);
        } else {
            revive(s.e.at, s.ids);
        }
    }
    void redo(const Step &s)
    {
        if (s.e.kind == LineEdit::Kind::Insert) {
            if (!s.pushed.empty()) kill(s.e.pushedAt, uint32_t(s.pushed.size()));
            revive(s.e.at, s.ids);
        } else {
            kill(s.e.at, uint32_t(s.ids.size()));
        }
    }
    void forget(const Step &s)
    {
        if (s.e.kind != LineEdit::Kind::Insert) return;
        for (uint32_t id : s.ids) dead.erase(id);
        if (s.ids.back() + 1 == next) next = s.ids.front();
    }
    uint32_t resolve(uint32_t id, bool wantAfter) const
    {
        for (int hops = 0; id != kNone && hops < 100000; ++hops) {
            if (std::find(ids.begin(), ids.end(), id) != ids.end()) return id;
            const auto it = dead.find(id);
            if (it == dead.end()) return kNone;
            id = wantAfter ? it->second.second : it->second.first;
        }
        return kNone;
    }
};

// Everything the map answers, against the model. An empty string if they agree.
QString differences(const LineMapCore &m, const Ref &r, QRandomGenerator &rng)
{
    if (m.size() != r.ids.size()) return QStringLiteral("size %1 against %2").arg(m.size()).arg(r.ids.size());
    if (m.nextId() != r.next) return QStringLiteral("next id %1 against %2").arg(m.nextId()).arg(r.next);
    if (!m.selfCheck()) return QStringLiteral("the map's own bookkeeping is wrong");
    LineHint hint;
    for (uint32_t p = 0; p < r.ids.size(); ++p) {
        if (m.idAt(p) != r.ids[p]) return QStringLiteral("id at %1: %2 against %3").arg(p).arg(m.idAt(p)).arg(r.ids[p]);
        if (m.idAt(p, &hint) != r.ids[p]) return QStringLiteral("hinted id at %1").arg(p);
        if (m.posOf(r.ids[p], &hint) != p) return QStringLiteral("hinted position of %1").arg(r.ids[p]);
    }
    if (m.idAt(uint32_t(r.ids.size())) != kNone) return QStringLiteral("an id past the end");
    // the original lines next to a position: the last at or before it, the first at or after it
    {
        uint32_t before = kNone;
        std::vector<uint32_t> after(r.ids.size() + 1, kNone);
        for (size_t p = r.ids.size(); p-- > 0;) after[p] = r.ids[p] < r.lim.firstNew ? r.ids[p] : after[p + 1];
        for (uint32_t p = 0; p < r.ids.size(); ++p) {
            if (r.ids[p] < r.lim.firstNew) before = r.ids[p];
            if (m.origBefore(p) != before) return QStringLiteral("origBefore(%1): %2 against %3").arg(p).arg(m.origBefore(p)).arg(before);
            if (m.origAfter(p) != after[p]) return QStringLiteral("origAfter(%1): %2 against %3").arg(p).arg(m.origAfter(p)).arg(after[p]);
        }
    }
    std::map<uint32_t, uint32_t> position;
    for (uint32_t p = 0; p < r.ids.size(); ++p) position[r.ids[p]] = p;
    for (uint32_t id = 0; id < r.next + 3; ++id) {
        const auto it = position.find(id);
        const uint32_t want = it == position.end() ? kNone : it->second;
        if (m.posOf(id) != want) return QStringLiteral("position of %1: %2 against %3").arg(id).arg(m.posOf(id)).arg(want);
        if (m.isLive(id) != (want != kNone)) return QStringLiteral("isLive %1").arg(id);
    }
    // the dead records: the same ids, the same neighbors
    size_t covered = 0;
    for (const LineDead &d : m.deadRecords()) {
        for (uint32_t i = 0; i < d.count; ++i) {
            const auto it = r.dead.find(d.first + i);
            if (it == r.dead.end()) return QStringLiteral("a record for %1, which the model has live or unknown").arg(d.first + i);
            if (it->second.first != d.before || it->second.second != d.after) return QStringLiteral("record of %1: neighbors differ").arg(d.first + i);
            ++covered;
        }
    }
    if (covered != r.dead.size()) return QStringLiteral("records cover %1 ids, the model has %2").arg(covered).arg(r.dead.size());
    for (const auto &kv : r.dead) {
        if (m.resolveStart(kv.first) != r.resolve(kv.first, true)) return QStringLiteral("resolveStart(%1)").arg(kv.first);
        if (m.resolveEnd(kv.first) != r.resolve(kv.first, false)) return QStringLiteral("resolveEnd(%1)").arg(kv.first);
    }
    if (!r.ids.empty()) {
        for (int k = 0; k < 4; ++k) {
            const uint32_t a = rng.bounded(uint32_t(r.ids.size())), b = a + rng.bounded(uint32_t(r.ids.size()) - a);
            std::vector<IdRun> runs;
            m.walk(a, b, &runs);
            std::vector<uint32_t> got;
            for (const IdRun &x : runs)
                for (uint32_t i = 0; i < x.count; ++i) got.push_back(x.first + i);
            if (got != std::vector<uint32_t>(r.ids.begin() + a, r.ids.begin() + b + 1)) return QStringLiteral("walk %1..%2").arg(a).arg(b);
            const LineMapCore::Runs cached = m.runsFor(a, b);
            if (!cached || *cached != runs) return QStringLiteral("cached walk %1..%2").arg(a).arg(b);
        }
    }
    return QString();
}

struct Snapshot {
    uint64_t version, state;
    uint32_t next, size;
    std::vector<IdRun> runs;
    std::vector<LineDead> dead;
    bool operator==(const Snapshot &o) const { return version == o.version && state == o.state && next == o.next && size == o.size && runs == o.runs && dead == o.dead; }
};
Snapshot snap(const LineMapCore &m) { return {m.version(), m.state(), m.nextId(), m.size(), m.liveRuns(), m.deadRecords()}; }

// A map with about `runs` runs after random single edits (no model: for the cost tests); `steps` gets every edit, in order.
void growRuns(LineMapCore &m, size_t runs, QRandomGenerator &rng, std::vector<LineEdit> *steps = nullptr)
{
    while (m.runCount() < runs) {
        LineEdit e;
        const uint32_t n = 1 + rng.bounded(3);
        const bool erase = m.size() > 10 && rng.bounded(2) == 0;
        const LineStatus s = erase ? m.erase(rng.bounded(m.size() - n), n, &e) : m.insert(rng.bounded(m.size() + 1 - 0), n, &e);
        if (s != LineStatus::Ok) continue;
        if (steps) steps->push_back(std::move(e));
    }
}

uint32_t log2ceil(size_t n)
{
    uint32_t b = 0;
    while ((size_t(1) << b) < n) ++b;
    return b;
}

} // namespace

class SheetLineMapTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Ids are positions until the first edit, and a sheet with no edit has no runs and holds no memory.
    void aMapWithNoEditHoldsNothingAndIdsAreThePositions()
    {
        RowMap m(LineLimits::rows(), 1048576);
        QVERIFY(m.isIdentity());
        QCOMPARE(m.runCount(), size_t(0));
        QCOMPARE(m.core().allocatedBytes(), size_t(0));
        QCOMPARE(m.idAt(RowPos(0)).value(), 0u);
        QCOMPARE(m.idAt(RowPos(1048575)).value(), 1048575u);
        QVERIFY(m.idAt(RowPos(1048576)).isNone());
        QCOMPARE(m.posOf(RowId(777)).value(), 777u);
        QVERIFY(m.posOf(RowId(1048576)).isNone());
        LineHint h;
        LineWork w;
        for (uint32_t p = 0; p < 1000; ++p) QCOMPARE(m.idAt(RowPos(p), &h, &w).value(), p);
        QCOMPARE(w.comparisons, uint64_t(0));
        const auto runs = m.walk(RowPos(100), RowPos(900));
        QCOMPARE(runs.size(), size_t(1));
        QCOMPARE(runs[0].first.value(), 100u);
        QCOMPARE(runs[0].count, 801u);
        QCOMPARE(m.runsFor(RowPos(5), RowPos(9))->size(), size_t(1));
        QCOMPARE(m.core().allocatedBytes(), size_t(0));   // (the cache is not counted: it holds nothing the map needs)
        QVERIFY(m.core().selfCheck());
        // the first edit makes the one run, and then the edit's own
        RowMap::Edit e;
        QCOMPARE(m.insert(RowPos(10), 1, &e), LineStatus::Ok);
        QVERIFY(!m.isIdentity());
        QCOMPARE(m.runCount(), size_t(3));
        QCOMPARE(m.idAt(RowPos(9)).value(), 9u);
        QCOMPARE(m.idAt(RowPos(10)).value(), 1u << 20);
        QCOMPARE(m.idAt(RowPos(11)).value(), 10u);
    }

    // Different kinds of number do not mix: nothing converts, and a bare number is not an id.
    void idsAndPositionsAreTypesOfTheirOwn()
    {
        static_assert(!std::is_convertible_v<RowId, RowPos>);
        static_assert(!std::is_convertible_v<RowPos, RowId>);
        static_assert(!std::is_convertible_v<ColId, ColPos>);
        static_assert(!std::is_convertible_v<RowId, ColId>);
        static_assert(!std::is_convertible_v<uint32_t, RowPos>);
        static_assert(!std::is_convertible_v<ColPos, uint32_t>);
        static_assert(!std::is_constructible_v<ColPos, ColId>);
        static_assert(!std::is_same_v<RowMap, ColMap>);
        QVERIFY(RowId().isNone());
        QVERIFY(ColPos::none().isNone());
        QVERIFY(RowId(5) < RowId(6));
        ColMap c(LineLimits::cols(), 16384);
        ColMap::Edit e;
        QCOMPARE(c.insert(ColPos(2), 3, &e), LineStatus::Ok);
        QCOMPARE(c.idAt(ColPos(2)).value(), 16384u);
        QCOMPARE(c.posOf(ColId(16386)).value(), 4u);
        QCOMPARE(e.at().value(), 2u);
        QCOMPARE(e.count(), 3u);
        QCOMPARE(e.runs().size(), size_t(1));
        QCOMPARE(e.runs()[0].first.value(), 16384u);
        QVERIFY(e.isInsert());
        QCOMPARE(e.pushed().size(), size_t(1));   // (all 16,384 columns were live: the last three were pushed off)
    }

    // Insert and erase move the ids after them and no others; Undo puts the same ids in the same places.
    void insertEraseUndoRedoOnASmallMap()
    {
        LineLimits lim;
        lim.capacity = 1000;
        lim.firstNew = 1000;
        lim.idLimit = 5000;
        RowMap m(lim, 10);   // ids 0..9
        RowMap::Edit ins, del;
        QCOMPARE(m.insert(RowPos(3), 2, &ins), LineStatus::Ok);   // 0 1 2 N N 3 4 ...
        QCOMPARE(m.size(), 12u);
        QCOMPARE(m.idAt(RowPos(2)).value(), 2u);
        QCOMPARE(m.idAt(RowPos(3)).value(), 1000u);
        QCOMPARE(m.idAt(RowPos(4)).value(), 1001u);
        QCOMPARE(m.idAt(RowPos(5)).value(), 3u);
        QCOMPARE(m.posOf(RowId(3)).value(), 5u);
        QCOMPARE(m.erase(RowPos(1), 4, &del), LineStatus::Ok);    // erases 1 2 N N
        QCOMPARE(m.size(), 8u);
        QCOMPARE(m.idAt(RowPos(1)).value(), 3u);
        QVERIFY(m.posOf(RowId(2)).isNone());
        QVERIFY(m.posOf(RowId(1000)).isNone());
        QVERIFY(!m.isLive(RowId(1001)));
        QCOMPARE(m.undo(del), LineStatus::Ok);
        QCOMPARE(m.size(), 12u);
        QCOMPARE(m.idAt(RowPos(2)).value(), 2u);
        QCOMPARE(m.idAt(RowPos(3)).value(), 1000u);
        QCOMPARE(m.undo(ins), LineStatus::Ok);
        QCOMPARE(m.size(), 10u);
        for (uint32_t p = 0; p < 10; ++p) QCOMPARE(m.idAt(RowPos(p)).value(), p);
        QCOMPARE(m.nextId().value(), 1002u);   // (Undo and Redo use no ids)
        QCOMPARE(m.redo(ins), LineStatus::Ok);
        QCOMPARE(m.idAt(RowPos(3)).value(), 1000u);
        QCOMPARE(m.redo(del), LineStatus::Ok);
        QCOMPARE(m.size(), 8u);
        QCOMPARE(m.nextId().value(), 1002u);
        QVERIFY(m.core().selfCheck());
        // refused edits change nothing
        const Snapshot before = snap(m.core());
        QCOMPARE(m.erase(RowPos(7), 2, nullptr), LineStatus::OutOfRange);
        QCOMPARE(m.erase(RowPos(8), 1, nullptr), LineStatus::OutOfRange);
        QCOMPARE(m.erase(RowPos(0), 0, nullptr), LineStatus::OutOfRange);
        QCOMPARE(m.insert(RowPos(9), 1, nullptr), LineStatus::OutOfRange);
        QCOMPARE(m.insert(RowPos(0), 0, nullptr), LineStatus::OutOfRange);
        QCOMPARE(m.insert(RowPos(990), 11, nullptr), LineStatus::OutOfRange);
        QVERIFY(snap(m.core()) == before);
    }

    // The sequence from the design: delete a, delete the next, Undo, Redo, Undo twice, Redo twice, and insert between the live neighbors of the deleted rows.
    void theNamedSequenceOfDeadEnds()
    {
        LineLimits lim;
        lim.capacity = 100;
        lim.firstNew = 100;
        lim.idLimit = 1000;
        RowMap m(lim, 6);   // ids 0..5; the formula reads a..d = 1..4
        const RowId a(1), b(2), c(3), d(4);
        RowMap::Edit e1, e2;
        QCOMPARE(m.erase(RowPos(1), 1, &e1), LineStatus::Ok);   // delete a
        QCOMPARE(m.resolveStart(a).value(), 2u);                 // the start is now b
        QCOMPARE(m.erase(RowPos(1), 1, &e2), LineStatus::Ok);   // delete b (the row now at 1)
        QCOMPARE(m.resolveStart(a).value(), 3u);                 // follows on to c
        QCOMPARE(m.resolveStart(b).value(), 3u);
        QCOMPARE(m.undo(e2), LineStatus::Ok);
        QCOMPARE(m.resolveStart(a).value(), 2u);                 // b is live again: the start is b
        QCOMPARE(m.redo(e2), LineStatus::Ok);
        QCOMPARE(m.resolveStart(a).value(), 3u);                 // a dies again with the same record
        QCOMPARE(m.undo(e2), LineStatus::Ok);
        QCOMPARE(m.undo(e1), LineStatus::Ok);
        QCOMPARE(m.resolveStart(a).value(), 1u);
        QVERIFY(m.core().deadRecords().empty());
        QCOMPARE(m.redo(e1), LineStatus::Ok);
        QCOMPARE(m.redo(e2), LineStatus::Ok);
        QCOMPARE(m.resolveStart(a).value(), 3u);
        RowMap::PosRange range;
        QVERIFY(m.resolveRange(a, d, &range));
        QCOMPARE(range.first.value(), 1u);                       // (c is at 1 now: 0, c, d, 5)
        QCOMPARE(range.last.value(), 2u);
        // the new row goes between the live neighbors of the deleted rows (id 0 and c): outside the range, before its start
        RowMap::Edit ins;
        QCOMPARE(m.insert(RowPos(1), 1, &ins), LineStatus::Ok);
        QCOMPARE(m.posOf(RowId(100)).value(), 1u);
        QVERIFY(m.resolveRange(a, d, &range));
        QCOMPARE(m.idAt(range.first).value(), 3u);
        QCOMPARE(range.first.value(), 2u);
        QCOMPARE(range.last.value(), 3u);
        QCOMPARE(m.resolveStart(a).value(), 3u);
        // the end: the live row before the deleted rows, by what they were when deleted
        QCOMPARE(m.erase(RowPos(3), 1, nullptr), LineStatus::Ok);   // delete d
        QCOMPARE(m.resolveEnd(d).value(), 3u);
        // a single cell whose row is dead has no position
        QVERIFY(m.posOf(a).isNone());
        // a range with every row deleted is no range; a start after the end is none
        QCOMPARE(m.erase(RowPos(2), 1, nullptr), LineStatus::Ok);   // delete c
        QVERIFY(!m.resolveRange(a, d, &range));
        QVERIFY(m.resolveStart(RowId(9999)).isNone());              // (an id never made)
    }

    // The invariant, one statement at a time (4.4).
    void theInvariantOfTheDeadEndMemory()
    {
        LineLimits lim;
        lim.capacity = 200;
        lim.firstNew = 200;
        lim.idLimit = 5000;
        RowMap m(lim, 20);
        // (2) a step is undone or redone only in the state it left: out of order, it is refused and nothing changes
        RowMap::Edit e1, e2, e3;
        QCOMPARE(m.erase(RowPos(5), 3, &e1), LineStatus::Ok);
        QCOMPARE(m.erase(RowPos(2), 2, &e2), LineStatus::Ok);
        QCOMPARE(m.insert(RowPos(0), 1, &e3), LineStatus::Ok);
        Snapshot s = snap(m.core());
        QCOMPARE(m.undo(e1), LineStatus::StaleStep);
        QCOMPARE(m.undo(e2), LineStatus::StaleStep);
        QCOMPARE(m.redo(e3), LineStatus::StaleStep);   // (it is applied)
        QVERIFY(snap(m.core()) == s);
        QCOMPARE(m.undo(e3), LineStatus::Ok);
        QCOMPARE(m.undo(e3), LineStatus::StaleStep);   // (and not twice)
        QCOMPARE(m.redo(e2), LineStatus::StaleStep);   // (it is applied; e3 is the next to redo)
        // (4) records are never edited: what the dead resolve to does not depend on what is inserted or erased next to them afterwards
        QVERIFY(m.core().forget(e3.raw()));   // (a new edit is coming: the redo of e3 is dropped)
        const std::vector<LineDead> records = m.core().deadRecords();
        const uint32_t startOfE1 = m.resolveStart(RowId(5)).value();
        RowMap::Edit x;
        QCOMPARE(m.insert(RowPos(3), 4, &x), LineStatus::Ok);
        QCOMPARE(m.insert(RowPos(3), 1, nullptr), LineStatus::Ok);
        QCOMPARE(m.erase(RowPos(10), 2, nullptr), LineStatus::Ok);
        QCOMPARE(m.resolveStart(RowId(5)).value(), startOfE1);
        const std::vector<LineDead> now = m.core().deadRecords();
        for (const LineDead &r : records) QVERIFY(std::find(now.begin(), now.end(), r) != now.end());
        // (3) a chain of dead ends ends, at a live row, one hop for each row deleted in turn
        RowMap n(lim, 200);
        for (int i = 0; i < 150; ++i) QCOMPARE(n.erase(RowPos(1), 1, nullptr), LineStatus::Ok);   // ids 1..150, one at a time
        LineWork w;
        QCOMPARE(n.resolveStart(RowId(1), &w).value(), 151u);
        QCOMPARE(w.hops, uint64_t(150));
        QCOMPARE(n.resolveEnd(RowId(150), &w).value(), 0u);
        QVERIFY(n.core().selfCheck());
        // (1) an id is made once: ids that were made are never made again while anything may name them
        RowMap k(lim, 5);
        std::set<uint32_t> seen;
        for (int i = 0; i < 60; ++i) {
            RowMap::Edit e;
            QCOMPARE(k.insert(RowPos(uint32_t(i) % k.size()), 2, &e), LineStatus::Ok);
            for (const auto &run : e.runs())
                for (uint32_t j = 0; j < run.count; ++j) QVERIFY(seen.insert(run.first.value() + j).second);
            if (i % 3 == 2) QCOMPARE(k.undo(e), LineStatus::Ok);   // undone and not dropped: its ids stay made
        }
    }

    // A whole row or column is the first to the last live position, whatever is inserted at the ends (4.4).
    void aWholeLineIsTheFirstToTheLastLivePosition()
    {
        LineLimits lim;
        lim.capacity = 1000;
        lim.firstNew = 1000;
        lim.idLimit = 5000;
        RowMap m(lim, 10);
        RowMap::PosRange r;
        QVERIFY(m.wholeRange(&r));
        QCOMPARE(r.first.value(), 0u);
        QCOMPARE(r.last.value(), 9u);
        QCOMPARE(m.insert(RowPos(0), 2, nullptr), LineStatus::Ok);   // at the top
        QCOMPARE(m.insert(RowPos(12), 1, nullptr), LineStatus::Ok);   // past the last original row
        QCOMPARE(m.insert(RowPos(9), 1, nullptr), LineStatus::Ok);    // at the last original row
        QVERIFY(m.wholeRange(&r));
        QCOMPARE(r.first.value(), 0u);
        QCOMPARE(r.last.value(), 13u);
        const auto runs = m.walk(r.first, r.last);
        uint32_t n = 0;
        std::set<uint32_t> ids;
        for (const auto &run : runs)
            for (uint32_t i = 0; i < run.count; ++i, ++n) ids.insert(run.first.value() + i);
        QCOMPARE(n, 14u);
        QVERIFY(ids.count(1000) && ids.count(1001) && ids.count(1002) && ids.count(1003));   // the inserted rows are inside
        QCOMPARE(m.erase(RowPos(0), 14, nullptr), LineStatus::Ok);
        QVERIFY(!m.wholeRange(&r));
    }

    // At the sheet's end an insert pushes the last lines off, and Undo brings them back.
    void anInsertOnAFullSheetPushesTheLastLinesOff()
    {
        LineLimits lim;
        lim.capacity = 100;
        lim.firstNew = 128;
        lim.idLimit = 1000;
        RowMap m(lim, 100);
        RowMap::Edit e;
        QCOMPARE(m.pushOff(3), 3u);
        QCOMPARE(m.insert(RowPos(10), 3, &e), LineStatus::Ok);
        QCOMPARE(m.size(), 100u);
        QCOMPARE(m.idAt(RowPos(10)).value(), 128u);
        QCOMPARE(m.idAt(RowPos(13)).value(), 10u);
        QCOMPARE(m.idAt(RowPos(99)).value(), 96u);
        QVERIFY(m.posOf(RowId(97)).isNone());
        QCOMPARE(m.resolveStart(RowId(98)).value(), kNone);   // (nothing follows them)
        QCOMPARE(m.resolveEnd(RowId(98)).value(), 96u);
        QCOMPARE(e.pushed().size(), size_t(1));
        QCOMPARE(m.undo(e), LineStatus::Ok);
        QCOMPARE(m.size(), 100u);
        for (uint32_t p = 0; p < 100; ++p) QCOMPARE(m.idAt(RowPos(p)).value(), p);
        QCOMPARE(m.redo(e), LineStatus::Ok);
        QCOMPARE(m.idAt(RowPos(99)).value(), 96u);
        QCOMPARE(m.insert(RowPos(98), 3, nullptr), LineStatus::OutOfRange);   // (the new lines themselves would be past the end)
        QVERIFY(m.core().selfCheck());
    }

    // Thousands of random sequences, with Undo and Redo, steps tried out of order, steps dropped from both ends, and new edits after Undo, against the plain vector of ids.
    void randomSequencesAgreeWithAPlainVectorOfIds()
    {
        int checks = 0, stale = 0, pushedOff = 0, forgets = 0, refusedInsert = 0, refusedForgets = 0;
        for (uint32_t seed = 1; seed <= 2500; ++seed) {
            QRandomGenerator rng(seed);
            LineLimits lim;
            lim.capacity = 40 + rng.bounded(60);
            lim.firstNew = 128;
            lim.idLimit = 128 + 6000;
            lim.maxRuns = 1000000;
            const uint32_t live = 1 + rng.bounded(lim.capacity);
            LineMapCore m(lim, live);
            Ref ref(lim, live);
            std::vector<Step> undoStack, redoStack;
            auto drop = [&](std::vector<Step> &stack) {   // (newest first)
                while (!stack.empty()) {
                    Step &s = stack.back();
                    if (!m.forget(s.e)) ++refusedForgets;
                    ref.forget(s);
                    stack.pop_back();
                    ++forgets;
                }
            };
            const int ops = 40 + int(rng.bounded(40));
            for (int op = 0; op < ops; ++op) {
                const int kind = int(rng.bounded(100));
                const Snapshot before = snap(m);
                QString what;
                if (kind < 28) {   // insert
                    const uint32_t at = rng.bounded(uint32_t(ref.ids.size()) + 1), n = 1 + rng.bounded(4);
                    Step s;
                    // (a new edit drops the redo steps before it takes ids, so that a dropped insert holding the highest ids gives them back)
                    const bool couldBeOk = ref.canInsert(at, n);
                    if (couldBeOk) drop(redoStack);
                    const bool should = couldBeOk && uint64_t(ref.next) + n <= lim.idLimit;
                    const Snapshot pre = snap(m);
                    const LineStatus st = m.insert(at, n, &s.e);
                    QVERIFY2((st == LineStatus::Ok) == should, qPrintable(QStringLiteral("seed %1 insert %2 x%3: status %4, expected ok=%5").arg(seed).arg(at).arg(n).arg(int(st)).arg(int(should))));
                    if (st == LineStatus::Ok) {
                        const uint32_t over = ref.pushOff(n);
                        pushedOff += over ? 1 : 0;
                        Step r = ref.insert(at, n);
                        s.ids = r.ids;
                        s.pushed = r.pushed;
                        undoStack.push_back(std::move(s));
                    } else {
                        QVERIFY(snap(m) == pre);
                        ++refusedInsert;
                    }
                    what = QStringLiteral("insert %1 x%2").arg(at).arg(n);
                } else if (kind < 52) {   // erase
                    if (ref.ids.empty()) continue;
                    const uint32_t at = rng.bounded(uint32_t(ref.ids.size())), n = 1 + rng.bounded(std::min<uint32_t>(6, uint32_t(ref.ids.size()) - at));
                    Step s;
                    drop(redoStack);
                    QCOMPARE(m.erase(at, n, &s.e), LineStatus::Ok);
                    Step r = ref.erase(at, n);
                    s.ids = r.ids;
                    undoStack.push_back(std::move(s));
                    what = QStringLiteral("erase %1 x%2").arg(at).arg(n);
                } else if (kind < 70) {   // undo
                    if (undoStack.empty()) continue;
                    Step s = std::move(undoStack.back());
                    undoStack.pop_back();
                    QCOMPARE(m.undo(s.e), LineStatus::Ok);
                    ref.undo(s);
                    redoStack.push_back(std::move(s));
                    what = QStringLiteral("undo");
                } else if (kind < 82) {   // redo
                    if (redoStack.empty()) continue;
                    Step s = std::move(redoStack.back());
                    redoStack.pop_back();
                    QCOMPARE(m.redo(s.e), LineStatus::Ok);
                    ref.redo(s);
                    undoStack.push_back(std::move(s));
                    what = QStringLiteral("redo");
                } else if (kind < 90) {   // a step out of order, or the wrong way, is refused and nothing changes
                    std::vector<std::pair<Step *, bool>> tries;   // (step, undo?)
                    for (size_t i = 0; i + 1 < undoStack.size(); ++i) tries.push_back({&undoStack[i], true});
                    for (size_t i = 0; i + 1 < redoStack.size(); ++i) tries.push_back({&redoStack[i], false});
                    if (!undoStack.empty()) tries.push_back({&undoStack.back(), false});
                    if (!redoStack.empty()) tries.push_back({&redoStack.back(), true});
                    if (tries.empty()) continue;
                    auto t = tries[rng.bounded(uint32_t(tries.size()))];
                    const LineStatus st = t.second ? m.undo(t.first->e) : m.redo(t.first->e);
                    QVERIFY2(st == LineStatus::StaleStep, qPrintable(QStringLiteral("seed %1: an out-of-order step was accepted").arg(seed)));
                    QVERIFY(snap(m) == before);
                    ++stale;
                    what = QStringLiteral("stale");
                } else if (kind < 95) {   // the oldest undo step is trimmed: nothing is revived, the records stay
                    if (undoStack.empty()) continue;
                    undoStack.erase(undoStack.begin());
                    what = QStringLiteral("trim");
                } else {   // the redo steps are dropped without a new edit
                    drop(redoStack);
                    what = QStringLiteral("drop redo");
                }
                const QString diff = differences(m, ref, rng);
                QVERIFY2(diff.isEmpty(), qPrintable(QStringLiteral("seed %1 op %2 (%3): %4").arg(seed).arg(op).arg(what, diff)));
                ++checks;
            }
        }
        QCOMPARE(refusedForgets, 0);   // (every dropped step was one that was undone)
        QVERIFY(checks > 100000);
        QVERIFY(stale > 1000);
        QVERIFY(pushedOff > 500);
        QVERIFY(forgets > 1000);
        QVERIFY(refusedInsert > 50);
    }

    // The same, with ids that run out and runs that are capped: refused edits leave the map as it was, and the rest still agree with the model.
    void randomSequencesWithTightCapsStillAgree()
    {
        int refused = 0, ok = 0;
        for (uint32_t seed = 1; seed <= 600; ++seed) {
            QRandomGenerator rng(seed * 7919);
            LineLimits lim;
            lim.capacity = 60;
            lim.firstNew = 64;
            lim.idLimit = 64 + 12 + rng.bounded(40);
            lim.maxRuns = 6 + rng.bounded(10);
            lim.rebaseRuns = 100000;
            LineMapCore m(lim, 30);
            Ref ref(lim, 30);
            std::vector<Step> undoStack;
            for (int op = 0; op < 60; ++op) {
                const Snapshot before = snap(m);
                const int kind = int(rng.bounded(10));
                if (kind < 4) {
                    const uint32_t at = rng.bounded(uint32_t(ref.ids.size()) + 1), n = 1 + rng.bounded(3);
                    Step s;
                    const LineStatus st = m.insert(at, n, &s.e);
                    if (st == LineStatus::Ok) {
                        Step r = ref.insert(at, n);
                        s.ids = r.ids;
                        s.pushed = r.pushed;
                        undoStack.push_back(std::move(s));
                        ++ok;
                    } else {
                        QVERIFY(st == LineStatus::OutOfRange || st == LineStatus::TooManyRuns || st == LineStatus::IdsExhausted);
                        QVERIFY(snap(m) == before);
                        ++refused;
                    }
                } else if (kind < 7) {
                    if (ref.ids.empty()) continue;
                    const uint32_t at = rng.bounded(uint32_t(ref.ids.size())), n = 1 + rng.bounded(std::min<uint32_t>(3, uint32_t(ref.ids.size()) - at));
                    Step s;
                    const LineStatus st = m.erase(at, n, &s.e);
                    if (st == LineStatus::Ok) {
                        s.ids = ref.erase(at, n).ids;
                        undoStack.push_back(std::move(s));
                        ++ok;
                    } else {
                        QCOMPARE(st, LineStatus::TooManyRuns);
                        QVERIFY(snap(m) == before);
                        ++refused;
                    }
                } else if (!undoStack.empty()) {   // undo (and drop: the step is not redone here)
                    Step s = std::move(undoStack.back());
                    undoStack.pop_back();
                    QCOMPARE(m.undo(s.e), LineStatus::Ok);
                    ref.undo(s);
                    QVERIFY(m.forget(s.e));
                    ref.forget(s);
                }
                const QString diff = differences(m, ref, rng);
                QVERIFY2(diff.isEmpty(), qPrintable(QStringLiteral("seed %1 op %2: %3").arg(seed).arg(op).arg(diff)));
            }
        }
        QVERIFY(refused > 100);
        QVERIFY(ok > 1000);
    }

    // A lookup is a search over the runs: its probes grow with the logarithm of the runs and not with the lines, at 1, 1,000, and 20,000 runs, in a thousand lines and in a million.
    void aLookupCostsTheLogarithmOfTheRunsAndNotTheLines()
    {
        for (uint32_t lines : {5000u, 1000000u}) {
            for (size_t runs : {size_t(3), size_t(1000), size_t(20000)}) {
                if (lines == 5000 && runs > 1000) continue;   // (there are not so many runs in so few lines)
                QRandomGenerator rng(uint32_t(runs) + lines);
                LineMapCore m(LineLimits::rows(), lines);
                growRuns(m, runs, rng);
                QVERIFY(m.runCount() >= runs);
                const uint32_t bound = log2ceil(m.runCount()) + 1;
                uint64_t most = 0, mostId = 0;
                for (int i = 0; i < 3000; ++i) {
                    const uint32_t p = rng.bounded(m.size());
                    LineWork w;
                    const uint32_t id = m.idAt(p, nullptr, &w);
                    most = std::max(most, w.comparisons);
                    LineWork w2;
                    QCOMPARE(m.posOf(id, nullptr, &w2), p);
                    mostId = std::max(mostId, w2.comparisons);
                }
                QVERIFY2(most <= bound, qPrintable(QStringLiteral("%1 runs, %2 lines: %3 probes for an id at a position").arg(m.runCount()).arg(lines).arg(most)));
                QVERIFY2(mostId <= bound, qPrintable(QStringLiteral("%1 runs, %2 lines: %3 probes for a position").arg(m.runCount()).arg(lines).arg(mostId)));
                if (runs >= 1000) QVERIFY(most >= log2ceil(m.runCount()) - 2);   // (it is a search: it is not free)
            }
        }
    }

    // A reader that walks down the lines in order, with its hint, searches once, however many runs there are.
    void aHintMakesSequentialReadsConstant()
    {
        QRandomGenerator rng(11);
        LineMapCore m(LineLimits::rows(), 1000000);
        growRuns(m, 20000, rng);
        LineHint h;
        LineWork w;
        const uint32_t reads = 200000;
        for (uint32_t p = 0; p < reads; ++p) QVERIFY(m.idAt(p, &h, &w) != kNone);
        QVERIFY2(w.comparisons <= uint64_t(reads) * 3 / 2 + 64, qPrintable(QString::number(w.comparisons)));
        // and by id, down the runs in order of id
        LineHint hid;
        LineWork wid;
        for (uint32_t id = 0; id < 100000; ++id) m.posOf(id, &hid, &wid);
        QVERIFY2(wid.comparisons <= uint64_t(100000) * 3 / 2 + 64, qPrintable(QString::number(wid.comparisons)));
        // a hint from before an edit is never trusted: the answers are the same as without it
        LineHint stale;
        const uint32_t idBefore = m.idAt(5000, &stale);
        QCOMPARE(m.insert(0, 3, nullptr), LineStatus::Ok);
        for (uint32_t p : {4990u, 5000u, 5003u, 5010u, 9000u}) QCOMPARE(m.idAt(p, &stale), m.idAt(p));
        QCOMPARE(m.posOf(idBefore, &stale), m.posOf(idBefore));
        QCOMPARE(m.posOf(idBefore), 5003u);
    }

    // An edit moves entries in proportion to the runs, not to the lines: the same at 1,000 lines and at 1,000,000; an edit at the end moves almost none.
    void anEditMovesEntriesInProportionToTheRunsNotTheLines()
    {
        for (size_t runs : {size_t(10), size_t(1000), size_t(20000)}) {
            std::vector<uint64_t> atTop, atEnd;
            for (uint32_t lines : {5000u, 1000000u}) {
                if (lines == 5000 && runs > 1000) continue;
                QRandomGenerator rng(uint32_t(runs) + 0u);
                LineMapCore m(LineLimits::rows(), lines);
                growRuns(m, runs, rng);
                const size_t r = m.runCount();
                QCOMPARE(m.insert(0, 1, nullptr), LineStatus::Ok);
                atTop.push_back(m.lastWork().entriesMoved);
                QVERIFY2(m.lastWork().entriesMoved <= 12 * r + 64, qPrintable(QStringLiteral("%1 runs: %2 entries moved by an insert at the top").arg(r).arg(m.lastWork().entriesMoved)));
                QVERIFY(m.lastWork().entriesMoved >= r);   // (the running counts after it all change)
                QCOMPARE(m.insert(m.size(), 1, nullptr), LineStatus::Ok);
                atEnd.push_back(m.lastWork().entriesMoved);
                QVERIFY2(m.lastWork().entriesMoved <= 16, qPrintable(QStringLiteral("%1 runs: %2 entries moved by an insert at the end").arg(r).arg(m.lastWork().entriesMoved)));
                QCOMPARE(m.erase(m.size() / 2, 1, nullptr), LineStatus::Ok);
                QVERIFY(m.lastWork().entriesMoved <= 14 * r + 64);
            }
            if (runs == 1000) {   // the same order for the same runs: lines do not enter into it
                QVERIFY(atTop[1] <= atTop[0] * 2 && atTop[0] <= atTop[1] * 2);
            }
        }
        // erasing half of a million lines that lie in a handful of runs touches a handful of entries
        LineMapCore m(LineLimits::rows(), 1000000);
        QCOMPARE(m.insert(300000, 2, nullptr), LineStatus::Ok);
        QCOMPARE(m.insert(700000, 2, nullptr), LineStatus::Ok);
        LineEdit e;
        QCOMPARE(m.erase(250000, 500000, &e), LineStatus::Ok);
        QVERIFY2(m.lastWork().entriesMoved <= 120, qPrintable(QString::number(m.lastWork().entriesMoved)));
        QCOMPARE(m.size(), 500004u);
        QCOMPARE(m.undo(e), LineStatus::Ok);
        QVERIFY2(m.lastWork().entriesMoved <= 120, qPrintable(QString::number(m.lastWork().entriesMoved)));
        QCOMPARE(m.size(), 1000004u);
        QCOMPARE(m.runCount(), size_t(5));
    }

    // A walk of a range visits the runs the range covers (and one search to find the first), whatever the lines.
    void aWalkVisitsTheRunsItCovers()
    {
        QRandomGenerator rng(5);
        LineMapCore m(LineLimits::rows(), 1000000);
        growRuns(m, 20000, rng);
        for (int k = 0; k < 300; ++k) {
            const uint32_t a = rng.bounded(m.size() - 1), b = a + rng.bounded(std::min<uint32_t>(m.size() - a, 20000));
            std::vector<IdRun> runs;
            LineWork w;
            m.walk(a, b, &runs, &w);
            QCOMPARE(w.runsVisited, uint64_t(runs.size()));
            QVERIFY(w.comparisons <= log2ceil(m.runCount()) + 1);
            uint64_t lines = 0;
            for (const IdRun &x : runs) lines += x.count;
            QCOMPARE(lines, uint64_t(b - a + 1));
            // the number of runs it covers is what a count of the live runs between says
            const uint32_t firstRun = [&] { LineHint h; m.idAt(a, &h); return h.pos; }();
            const uint32_t lastRun = [&] { LineHint h; m.idAt(b, &h); return h.pos; }();
            QCOMPARE(uint32_t(runs.size()), lastRun - firstRun + 1);
        }
        // a whole line is the same walk
        std::vector<IdRun> all;
        LineWork w;
        m.walk(0, m.size() - 1, &all, &w);
        QCOMPARE(w.runsVisited, uint64_t(m.runCount()));
        QCOMPARE(all.size(), m.runCount());
    }

    // The cache of runs for a range: a range read a million times is walked once; an edit empties it; a reader keeps what it was given; it is bounded.
    void theRunCacheWalksARangeOncePerVersion()
    {
        QRandomGenerator rng(3);
        LineMapCore m(LineLimits::rows(), 1000000);
        growRuns(m, 20000, rng);
        const LineMapCore::Runs first = m.runsFor(1000, 900000);
        QCOMPARE(m.cacheMisses(), uint64_t(1));
        for (int i = 0; i < 1000000; ++i)
            if (m.runsFor(1000, 900000).get() != first.get()) QFAIL("a different object for the same range");
        QCOMPARE(m.cacheMisses(), uint64_t(1));
        QCOMPARE(m.cacheHits(), uint64_t(1000000));
        std::vector<IdRun> direct;
        m.walk(1000, 900000, &direct);
        QVERIFY(*first == direct);
        QVERIFY(first->size() > 10000);
        // another range is another walk; a change of the map is a new version and the walk is made again
        m.runsFor(5, 6);
        QCOMPARE(m.cacheMisses(), uint64_t(2));
        const std::vector<IdRun> kept = *first;
        QCOMPARE(m.insert(10, 1, nullptr), LineStatus::Ok);
        const LineMapCore::Runs again = m.runsFor(1000, 900000);
        QCOMPARE(m.cacheMisses(), uint64_t(3));
        QVERIFY(again.get() != first.get());
        QVERIFY(*first == kept);   // (what a reader holds does not change under it)
        std::vector<IdRun> now;
        m.walk(1000, 900000, &now);
        QVERIFY(*again == now);
        // bounded: 20,000 different ranges do not stay
        for (uint32_t i = 0; i < 20000; ++i) m.runsFor(i, i + 100);
        QVERIFY(m.cacheEntries() <= 8192);
        QVERIFY(*again == now);
        // an Undo is a change too
        LineEdit e;
        QCOMPARE(m.erase(20, 3, &e), LineStatus::Ok);
        const LineMapCore::Runs a = m.runsFor(1000, 2000);
        QCOMPARE(m.undo(e), LineStatus::Ok);
        const LineMapCore::Runs b = m.runsFor(1000, 2000);
        QVERIFY(a.get() != b.get());
        std::vector<IdRun> w2;
        m.walk(1000, 2000, &w2);
        QVERIFY(*b == w2);
    }

    // A long session: a million lines in 20,000 runs. Everything still agrees, every step is undone back to the first state in one run, and redone to the same state.
    void aLongSessionOfAMillionLinesAndTwentyThousandRuns()
    {
        QRandomGenerator rng(2026);
        LineMapCore m(LineLimits::rows(), 1000000);
        std::vector<LineEdit> steps;
        growRuns(m, 20000, rng, &steps);
        QVERIFY(m.runCount() >= 20000);
        QVERIFY(m.selfCheck());
        // positions and ids round-trip, every live id is where the order says, and the runs add up to the lines
        LineHint h1, h2;
        for (int i = 0; i < 20000; ++i) {
            const uint32_t p = rng.bounded(m.size());
            const uint32_t id = m.idAt(p, &h1);
            QCOMPARE(m.posOf(id, &h2), p);
        }
        uint64_t total = 0;
        for (const IdRun &x : m.liveRuns()) total += x.count;
        QCOMPARE(total, uint64_t(m.size()));
        const Snapshot full = snap(m);
        // (the memory is the runs: 24 bytes each, and the dead records)
        QCOMPARE(m.runBytes(), m.runCount() * 24);
        QVERIFY(m.allocatedBytes() <= m.runCount() * 24 * 2 + m.deadCount() * 16 * 2);
        for (size_t i = steps.size(); i-- > 0;) QCOMPARE(m.undo(steps[i]), LineStatus::Ok);
        QCOMPARE(m.size(), 1000000u);
        QCOMPARE(m.runCount(), size_t(1));
        size_t inserts = 0;
        for (const LineEdit &e : steps) inserts += e.kind == LineEdit::Kind::Insert ? e.runs.size() : 0;
        QCOMPARE(m.deadCount(), inserts);   // (an insert that is undone is remembered as dead, for its Redo; an erase that is undone leaves no record)
        QCOMPARE(m.idAt(777777), 777777u);
        QVERIFY(m.selfCheck());
        for (size_t i = 0; i < steps.size(); ++i) QCOMPARE(m.redo(steps[i]), LineStatus::Ok);
        QCOMPARE(m.liveRuns(), full.runs);
        QCOMPARE(m.deadRecords(), full.dead);
        QCOMPARE(m.size(), full.size);
        QVERIFY(m.selfCheck());
    }

    // Inserting and undoing in a loop uses no ids; typing rows one after another makes one run; Undo and Redo of one edit make no runs.
    void loopsOfEditsDoNotUseIdsOrGrowTheMap()
    {
        LineMapCore m(LineLimits::rows(), 5000);
        for (int i = 0; i < 2000; ++i) {
            LineEdit e;
            QCOMPARE(m.insert(100, 3, &e), LineStatus::Ok);
            QCOMPARE(m.undo(e), LineStatus::Ok);
            QVERIFY(m.forget(e));   // (a new edit after Undo drops it)
        }
        QCOMPARE(m.nextId(), 1u << 20);
        QCOMPARE(m.runCount(), size_t(1));   // (the two halves of the first run are one again)
        QCOMPARE(m.deadCount(), size_t(0));
        LineEdit e;
        QCOMPARE(m.insert(100, 3, &e), LineStatus::Ok);
        for (int i = 0; i < 2000; ++i) {
            QCOMPARE(m.undo(e), LineStatus::Ok);
            QCOMPARE(m.redo(e), LineStatus::Ok);
        }
        QCOMPARE(m.nextId(), (1u << 20) + 3);
        QCOMPARE(m.runCount(), size_t(3));
        QCOMPARE(m.deadCount(), size_t(0));
        // rows typed one under the other are one run
        LineMapCore t(LineLimits::rows(), 1000);
        for (uint32_t i = 0; i < 500; ++i) QCOMPARE(t.insert(1000 + i, 1, nullptr), LineStatus::Ok);
        QCOMPARE(t.runCount(), size_t(2));
        // an insert undone is not given back if a later one holds higher ids; a step that is not the highest leaves the counter alone
        LineMapCore u(LineLimits::rows(), 100);
        LineEdit e1, e2;
        QCOMPARE(u.insert(1, 2, &e1), LineStatus::Ok);
        QCOMPARE(u.insert(5, 2, &e2), LineStatus::Ok);
        QCOMPARE(u.undo(e2), LineStatus::Ok);
        QCOMPARE(u.undo(e1), LineStatus::Ok);
        QVERIFY(u.forget(e1));   // (not the highest: the counter stays)
        QCOMPARE(u.nextId(), (1u << 20) + 4);
        QVERIFY(u.forget(e2));   // (the highest: its ids come back; the earlier step's were let go of while not the highest, and stay lost)
        QCOMPARE(u.nextId(), (1u << 20) + 2);
        // a step that is applied is not forgotten, and a forgotten step is never undone or redone
        LineEdit e3;
        QCOMPARE(u.insert(1, 1, &e3), LineStatus::Ok);
        QVERIFY(!u.forget(e3));
        QCOMPARE(u.undo(e3), LineStatus::Ok);
        QVERIFY(u.forget(e3));
        QCOMPARE(u.redo(e3), LineStatus::StaleStep);
        QCOMPARE(u.undo(e3), LineStatus::StaleStep);
    }

    // The caps refuse with a reason, change nothing, and are named in words; the real caps are the design's.
    void theCapsRefuseWithAReasonAndChangeNothing()
    {
        QCOMPARE(LineLimits::rows().maxRuns, 1000000u);
        QCOMPARE(LineLimits::rows().idLimit - LineLimits::rows().firstNew, (1u << 24) - (1u << 20));   // about 15.7 million rows in a session
        QCOMPARE(LineLimits::cols().idLimit - LineLimits::cols().firstNew, 49152u);
        QCOMPARE(LineLimits::cols().capacity, 16384u);
        const QString words = lineStatusText(LineStatus::TooManyRuns);
        QCOMPARE(words, QStringLiteral("This sheet has been edited too many times. Save it and open it again."));
        QCOMPARE(lineStatusText(LineStatus::IdsExhausted), words);
        QVERIFY(lineStatusText(LineStatus::Ok).isEmpty());
        // runs
        LineLimits lim;
        lim.capacity = 1000;
        lim.firstNew = 1000;
        lim.idLimit = 100000;
        lim.maxRuns = 8;
        LineMapCore m(lim, 100);
        int ok = 0;
        LineStatus last = LineStatus::Ok;
        for (uint32_t i = 0; i < 20 && last == LineStatus::Ok; ++i) {
            const Snapshot before = snap(m);
            last = m.insert(i * 4 + 1, 1, nullptr);
            if (last == LineStatus::Ok) ++ok;
            else QVERIFY(snap(m) == before);
        }
        QCOMPARE(last, LineStatus::TooManyRuns);
        QVERIFY(ok >= 2 && ok <= 4);
        QVERIFY(m.runCount() <= lim.maxRuns);
        // the dead records are capped as well
        LineLimits dl = lim;
        dl.maxRuns = 6;
        LineMapCore d(dl, 100);
        LineStatus s = LineStatus::Ok;
        for (int i = 0; i < 20 && s == LineStatus::Ok; ++i) s = d.erase(10, 1, nullptr);
        QCOMPARE(s, LineStatus::TooManyRuns);
        QVERIFY(d.deadCount() <= dl.maxRuns);
        // ids: columns have 49,152 spare ids; inserting at the last column pushes the one before off each time, and the 49,153rd is refused
        ColMap cols(LineLimits::cols(), 16384);
        uint32_t made = 0;
        LineStatus cs = LineStatus::Ok;
        while ((cs = cols.insert(ColPos(16383), 1, nullptr)) == LineStatus::Ok) ++made;
        QCOMPARE(made, 49152u);
        QCOMPARE(cs, LineStatus::IdsExhausted);
        QCOMPARE(cols.nextId().value(), 1u << 16);
        QCOMPARE(cols.size(), 16384u);
        QVERIFY(cols.needsRebase());
        QVERIFY(cols.core().selfCheck());
        // and a refused insert changes nothing
        QCOMPARE(cols.insert(ColPos(0), 1, nullptr), LineStatus::IdsExhausted);
    }

    // The rebase: the ids become the positions, in one pass, with a table of what each run was; every older step is then refused; it is refused if the map would still be over a cap.
    void aRebaseRenumbersTheIdsToTheirPositions()
    {
        for (uint32_t seed = 1; seed <= 200; ++seed) {
            QRandomGenerator rng(seed);
            LineLimits lim;
            lim.capacity = 200;
            lim.firstNew = 256;
            lim.idLimit = 256 + 1000;
            LineMapCore m(lim, 50 + rng.bounded(100));
            std::vector<LineEdit> steps;
            for (int i = 0; i < 60; ++i) {
                LineEdit e;
                const LineStatus st = rng.bounded(2) && m.size() > 3 ? m.erase(rng.bounded(m.size() - 2), 1 + rng.bounded(2), &e) : m.insert(rng.bounded(m.size() + 1), 1 + rng.bounded(3), &e);
                if (st == LineStatus::Ok) steps.push_back(e);
            }
            std::vector<uint32_t> before;   // the ids in order of position
            for (uint32_t p = 0; p < m.size(); ++p) before.push_back(m.idAt(p));
            std::vector<LineMapCore::Renumber> table;
            QCOMPARE(m.rebase(&table), LineStatus::Ok);
            QVERIFY(m.isIdentity());
            QCOMPARE(m.runCount(), size_t(0));
            QCOMPARE(m.deadCount(), size_t(0));
            QCOMPARE(m.allocatedBytes(), size_t(0));
            QCOMPARE(m.nextId(), lim.firstNew);
            QCOMPARE(m.size(), uint32_t(before.size()));
            for (uint32_t p = 0; p < m.size(); ++p) QCOMPARE(m.idAt(p), p);
            QVERIFY(!m.needsRebase());
            // the table says where each old id went: to its old position
            std::map<uint32_t, uint32_t> renamed;
            for (const auto &t : table)
                for (uint32_t i = 0; i < t.count; ++i) renamed[t.oldFirst + i] = t.newFirst + i;
            QCOMPARE(renamed.size(), before.size());
            for (uint32_t p = 0; p < before.size(); ++p) QCOMPARE(renamed[before[p]], p);
            for (LineEdit &e : steps) {
                QCOMPARE(m.undo(e), LineStatus::StaleStep);
                QCOMPARE(m.redo(e), LineStatus::StaleStep);
            }
            // and it goes on from there
            QCOMPARE(m.insert(0, 1, nullptr), LineStatus::Ok);
            QCOMPARE(m.idAt(0), lim.firstNew);
            QVERIFY(m.selfCheck());
        }
        // wanted at four fifths of the spare ids, or at the run count; refused if it would leave the map over a cap
        LineLimits lim;
        lim.capacity = 100;
        lim.firstNew = 100;
        lim.idLimit = 200;
        LineMapCore m(lim, 50);
        for (int i = 0; i < 79; ++i) {
            QVERIFY(!m.needsRebase());
            QCOMPARE(m.insert(0, 1, nullptr), LineStatus::Ok);
            QCOMPARE(m.erase(0, 1, nullptr), LineStatus::Ok);
        }
        QVERIFY(!m.needsRebase());   // (79 of 100 spare ids used)
        QCOMPARE(m.insert(0, 1, nullptr), LineStatus::Ok);
        QVERIFY(m.needsRebase());    // (80)
        lim.rebaseRuns = 1;
        LineMapCore tight(lim, 50);
        QCOMPARE(tight.insert(3, 1, nullptr), LineStatus::Ok);
        const Snapshot s = snap(tight);
        QCOMPARE(tight.rebase(nullptr), LineStatus::StillOverCap);
        QVERIFY(snap(tight) == s);
        QVERIFY(!tight.isIdentity());
        QVERIFY(!lineStatusText(LineStatus::StillOverCap).isEmpty());
        // a map that was never edited has nothing to renumber
        LineMapCore fresh(lim, 10);
        std::vector<LineMapCore::Renumber> t;
        QCOMPARE(fresh.rebase(&t), LineStatus::Ok);
        QVERIFY(t.empty());
    }

    // Readers on many threads at once (the cache is locked; hints are their own), and reads made while the window thread changes the map only under a hold that waits for them.
    void readersAreSafeWhileTheMapIsChangedUnderAHold()
    {
        QRandomGenerator rng(8);
        LineMapCore m(LineLimits::rows(), 100000);
        growRuns(m, 400, rng);
        // 1. no writer: four readers share the map and the cache of runs; every request is counted once
        {
            std::atomic<uint64_t> requests{0}, wrong{0};
            const uint64_t hits0 = m.cacheHits(), misses0 = m.cacheMisses();
            std::vector<std::thread> threads;
            for (int t = 0; t < 4; ++t)
                threads.emplace_back([&, t] {
                    QRandomGenerator local(100 + t);
                    LineHint h;
                    for (int i = 0; i < 20000; ++i) {
                        const uint32_t a = local.bounded(8) * 1000, b = a + 5000;
                        const LineMapCore::Runs r = m.runsFor(a, b);
                        ++requests;
                        uint64_t n = 0;
                        for (const IdRun &x : *r) n += x.count;
                        if (n != 5001) ++wrong;
                        const uint32_t p = local.bounded(m.size());
                        if (m.posOf(m.idAt(p, &h), &h) != p) ++wrong;
                    }
                });
            for (auto &t : threads) t.join();
            QCOMPARE(wrong.load(), uint64_t(0));
            QCOMPARE((m.cacheHits() - hits0) + (m.cacheMisses() - misses0), requests.load());
            QCOMPARE(m.cacheMisses() - misses0, uint64_t(8));   // (eight ranges, each walked once)
        }
        // 2. readers that look at a flag between reads, and a writer that raises it, waits for all of them to stop, changes the map, and lowers it
        {
            std::atomic<bool> hold{false}, stop{false};
            std::atomic<int> parked[3] = {{0}, {0}, {0}};
            std::atomic<uint64_t> reads{0}, torn{0}, wrong{0};
            const int readers = 3;
            std::vector<std::thread> threads;
            for (int t = 0; t < readers; ++t)
                threads.emplace_back([&, t] {
                    QRandomGenerator local(500 + t);
                    LineHint h;
                    while (!stop) {
                        if (hold) {
                            parked[t] = 1;
                            while (hold && !stop) std::this_thread::yield();
                            parked[t] = 0;
                            continue;   // (and look at the flag again before reading)
                        }
                        const uint64_t v0 = m.version();
                        const uint32_t size = m.size();
                        const uint32_t a = local.bounded(size / 2), b = a + local.bounded(size / 2);
                        const LineMapCore::Runs runs = m.runsFor(a, b);
                        uint64_t n = 0;
                        for (const IdRun &x : *runs) n += x.count;
                        if (n != uint64_t(b) - a + 1) ++wrong;
                        for (int i = 0; i < 20; ++i) {
                            const uint32_t p = local.bounded(size);
                            if (m.posOf(m.idAt(p, &h), &h) != p) ++wrong;
                        }
                        if (m.version() != v0 || m.size() != size) ++torn;   // (a change in the middle of a read)
                        ++reads;
                    }
                });
            std::vector<LineEdit> steps;
            for (int i = 0; i < 150; ++i) {
                const uint64_t seen = reads.load();
                while (reads.load() < seen + 20) std::this_thread::yield();   // (let them read in between)
                hold = true;
                while (parked[0] + parked[1] + parked[2] < readers) std::this_thread::yield();
                LineEdit e;
                const uint32_t n = 1 + rng.bounded(3);
                const LineStatus st = i % 3 == 0 && !steps.empty() ? m.undo(steps.back()) : (rng.bounded(2) ? m.insert(rng.bounded(m.size()), n, &e) : m.erase(rng.bounded(m.size() - 4), n, &e));
                if (i % 3 == 0 && !steps.empty()) steps.pop_back();
                else if (st == LineStatus::Ok) steps.push_back(e);
                hold = false;
            }
            stop = true;
            for (auto &t : threads) t.join();
            QCOMPARE(torn.load(), uint64_t(0));
            QCOMPARE(wrong.load(), uint64_t(0));
            QVERIFY(reads.load() > 150 * 20);
            QVERIFY(m.selfCheck());
        }
    }
};

JP_TEST_CLASS(SheetLineMapTests)
#include "tst_sheet_linemap.moc"
