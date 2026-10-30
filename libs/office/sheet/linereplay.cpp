#include "sheet/linereplay.h"

#include "calc/linemap.h"
#include "sheet/formulaframe.h"
#include "sheet/sheetmodel.h"

#include <QHash>

#include <algorithm>

namespace jp::office::sheet {

using jp::calc::ColMap;
using jp::calc::LineEdit;
using jp::calc::LineLimits;
using jp::calc::LineMapCore;
using jp::calc::LineStatus;
using jp::calc::RefLit;
using jp::calc::RowMap;

namespace {

constexpr uint32_t kNone = LineMapCore::kNone;

// The same lookups as CellStore's: the places of the lines a reference names by the ids at its ends (an end that is dead is the line next to where it was).
bool placesOf(const LineMapCore &map, bool identity, quint32 startId, quint32 endId, quint32 *first, quint32 *last)
{
    if (identity) {
        if (startId >= map.size() || endId >= map.size()) return false;
        *first = std::min(startId, endId);
        *last = std::max(startId, endId);
        return true;
    }
    const uint32_t a = map.posOf(startId), b = map.posOf(endId);
    if (a != kNone && b != kNone) {
        *first = std::min(a, b);
        *last = std::max(a, b);
        return true;
    }
    return map.resolveRange(startId, endId, first, last);
}

bool linesOfTheFile(const LineMapCore &core, bool identity)
{
    if (identity) return true;
    const std::vector<jp::calc::IdRun> live = core.liveRuns();
    if (live.size() > 1 || (live.size() == 1 && live[0].first != 0)) return false;
    for (const jp::calc::LineDead &d : core.deadRecords())
        if (d.first < core.limits().firstNew) return false;
    return true;
}

// The edit as the window's cells make it (CellStore::insertLines and deleteLines): an insert that would not fit pushes everything from its place off the end.
bool make(LineMapCore &map, bool insert, quint32 at, quint32 count)
{
    if (count == 0 || at >= map.size()) return true;   // (the window refuses it: nothing was done)
    const jp::calc::LineLimits &limits = map.limits();
    if (uint64_t(map.runCount()) + map.deadCount() + 8 > limits.maxRuns) return false;
    if (uint64_t(map.nextId()) + 2 * uint64_t(count) > limits.idLimit) return false;
    if (insert && uint64_t(at) + count > limits.capacity) {
        insert = false;
        count = limits.capacity - at;
    }
    LineEdit e, refill;
    if (insert) return map.insert(at, count, &e) == LineStatus::Ok;
    count = std::min(count, map.size() - at);
    if (map.erase(at, count, &e) != LineStatus::Ok) return false;
    return map.insert(map.size(), count, &refill) == LineStatus::Ok;
}

class NoCells : public FrameSheets {
public:
    NoCells(const QHash<QString, int> &byName, int count) : m_byName(byName), m_count(count) {}
    int frameSheetCount() const override { return m_count; }
    int frameSheetIndex(const QString &name) const override { return m_byName.value(name.toCaseFolded(), -1); }
    const CellStore &frameCells(int) const override
    {
        static const CellStore none;   // (mapReferences asks for the sheets' names only)
        return none;
    }

private:
    const QHash<QString, int> &m_byName;
    int m_count;
};

} // namespace

struct LineReplay::Impl {
    struct Lines {
        RowMap rows{LineLimits::rows(), LineLimits::rows().capacity};
        ColMap cols{LineLimits::cols(), LineLimits::cols().capacity};
        bool rowsEdited = false, colsEdited = false;   // (an edit was made: the map is not the identity)
        bool moved = false;                            // the lines are not in the file's order
    };
    std::vector<std::unique_ptr<Lines>> sheets;
    QStringList names;
    QHash<QString, int> byName;
    bool ok = true;
    bool any = false;

    const Lines *of(int sheet) const { return sheet >= 0 && sheet < int(sheets.size()) ? sheets[size_t(sheet)].get() : nullptr; }

    MapResult map(int sheet, RefLit *lit) const
    {
        const Lines *L = of(sheet);
        if (!L || !L->moved) return MapResult::Same;
        const bool wholeCols = lit->flags & jp::calc::kRefWholeCols, wholeRows = lit->flags & jp::calc::kRefWholeRows;
        // (an end that was deleted is the line next to where it was; a reference with nothing left of it is gone)
        if (!wholeCols) {
            quint32 s0, e0, a = 0, b = 0;
            bool sa, ea;
            jp::calc::refEnds(*lit, true, &s0, &e0, &sa, &ea);
            if (!placesOf(L->rows.core(), !L->rowsEdited, s0, e0, &a, &b)) return MapResult::Gone;
            jp::calc::setRefEnds(lit, true, a, b, sa, ea);
        }
        if (!wholeRows) {
            quint32 s0, e0, a = 0, b = 0;
            bool sa, ea;
            jp::calc::refEnds(*lit, false, &s0, &e0, &sa, &ea);
            if (!placesOf(L->cols.core(), !L->colsEdited, s0, e0, &a, &b)) return MapResult::Gone;
            jp::calc::setRefEnds(lit, false, a, b, sa, ea);
        }
        return MapResult::Changed;
    }
};

LineReplay::LineReplay(const std::vector<StructEdit> &edits, const QStringList &sheetNames) : d(new Impl)
{
    d->names = sheetNames;
    for (int i = 0; i < sheetNames.size(); ++i)
        if (!sheetNames[i].isEmpty() && !d->byName.contains(sheetNames[i].toCaseFolded())) d->byName.insert(sheetNames[i].toCaseFolded(), i);
    d->sheets.resize(size_t(sheetNames.size()));
    for (const StructEdit &e : edits) {
        if (!e.changesRows() && !e.changesColumns()) continue;
        if (e.sheet < 0 || e.sheet >= sheetNames.size()) continue;
        std::unique_ptr<Impl::Lines> &slot = d->sheets[size_t(e.sheet)];
        if (!slot) slot = std::make_unique<Impl::Lines>();
        const bool rows = e.changesRows();
        const bool insert = e.kind == StructEdit::Kind::InsertRows || e.kind == StructEdit::Kind::InsertColumns;
        LineMapCore &core = rows ? slot->rows.core() : slot->cols.core();
        if (!make(core, insert, e.first, e.count)) {
            d->ok = false;
            return;
        }
        (rows ? slot->rowsEdited : slot->colsEdited) = true;
    }
    for (std::unique_ptr<Impl::Lines> &slot : d->sheets) {
        if (!slot) continue;
        slot->moved = !linesOfTheFile(slot->rows.core(), !slot->rowsEdited) || !linesOfTheFile(slot->cols.core(), !slot->colsEdited);
        d->any = d->any || slot->moved;
    }
}

LineReplay::~LineReplay() = default;

bool LineReplay::ok() const { return d->ok; }

bool LineReplay::movesAnything() const { return d->any; }

bool LineReplay::moves(int sheet) const
{
    const Impl::Lines *L = d->of(sheet);
    return L && L->moved;
}

QStringList LineReplay::movedSheetNames() const
{
    QStringList out;
    for (int i = 0; i < int(d->sheets.size()); ++i)
        if (d->sheets[size_t(i)] && d->sheets[size_t(i)]->moved) out << d->names[i];
    return out;
}

QString LineReplay::rewrite(const QString &text, int ownSheet) const
{
    if (!d->any || text.isEmpty()) return text;
    NoCells frame(d->byName, int(d->sheets.size()));
    QString out;
    if (!mapReferences(frame, ownSheet, text, jp::calc::ParseLimits::grid(), [this](int sheet, RefLit *lit) { return d->map(sheet, lit); }, &out)) return text;
    return out;
}

bool LineReplay::rowNow(int sheet, quint32 row, quint32 *now) const
{
    const Impl::Lines *L = d->of(sheet);
    if (!L || !L->rowsEdited) {
        *now = row;
        return true;
    }
    const uint32_t p = L->rows.core().posOf(row);
    *now = p;
    return p != kNone;
}

bool LineReplay::columnNow(int sheet, quint32 col, quint32 *now) const
{
    const Impl::Lines *L = d->of(sheet);
    if (!L || !L->colsEdited) {
        *now = col;
        return true;
    }
    const uint32_t p = L->cols.core().posOf(col);
    *now = p;
    return p != kNone;
}

} // namespace jp::office::sheet
