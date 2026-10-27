#include "core/anchors.h"

#include <algorithm>

namespace jp::crunch {

using namespace jp::office::sheet;

namespace {

const std::map<quint64, NoteInfo> kNoNotes;
const std::vector<LinkSpec> kNoLinks;

bool single(const CellRange &r) { return r.row1 == r.row2 && r.col1 == r.col2; }

} // namespace

CellAnchors::SheetData &CellAnchors::sheetData(int sheet)
{
    if (sheet >= int(m_sheets.size())) m_sheets.resize(size_t(sheet) + 1);
    return m_sheets[size_t(sheet)];
}

void CellAnchors::open(std::vector<std::map<quint64, NoteInfo>> notes, std::vector<std::vector<LinkSpec>> links)
{
    m_sheets.clear();
    m_sheets.resize(std::max(notes.size(), links.size()));
    for (size_t i = 0; i < notes.size(); ++i) m_sheets[i].notes = std::move(notes[i]);
    for (size_t i = 0; i < links.size(); ++i) m_sheets[i].links = std::move(links[i]);
}

const std::map<quint64, NoteInfo> &CellAnchors::notes(int sheet) const { return sheet >= 0 && sheet < int(m_sheets.size()) ? m_sheets[size_t(sheet)].notes : kNoNotes; }

const NoteInfo *CellAnchors::noteAt(int sheet, quint32 row, quint32 col) const
{
    const auto &n = notes(sheet);
    const auto it = n.find(noteKey(row, col));
    return it == n.end() ? nullptr : &it->second;
}

const std::vector<LinkSpec> &CellAnchors::links(int sheet) const { return sheet >= 0 && sheet < int(m_sheets.size()) ? m_sheets[size_t(sheet)].links : kNoLinks; }

const LinkSpec *CellAnchors::linkAt(int sheet, quint32 row, quint32 col) const
{
    const auto &all = links(sheet);
    for (size_t i = all.size(); i-- > 0;)
        if (all[i].range.contains(row, col)) return &all[i];
    return nullptr;
}

std::vector<quint64> CellAnchors::noteCells(int sheet) const
{
    std::vector<quint64> out;
    for (const auto &n : notes(sheet)) out.push_back(n.first);
    return out;
}

void CellAnchors::apply(const SideEdit &edit, bool forward)
{
    SheetData &d = sheetData(edit.sheet);
    for (const SideEdit::Note &n : edit.notes) {
        const std::optional<NoteInfo> &now = forward ? n.after : n.before;
        const quint64 key = noteKey(n.row, n.col);
        if (now) d.notes[key] = *now;
        else d.notes.erase(key);
        d.noteTouched.insert(key);
    }
    const std::vector<LinkSpec> &out = forward ? edit.linksRemoved : edit.linksAdded;
    const std::vector<LinkSpec> &in = forward ? edit.linksAdded : edit.linksRemoved;
    for (const LinkSpec &l : out) {
        const auto at = std::find(d.links.begin(), d.links.end(), l);
        if (at != d.links.end()) d.links.erase(at);
        touch(d, l);
    }
    for (const LinkSpec &l : in) {
        d.links.push_back(l);
        touch(d, l);
    }
}

namespace {

// A link's place ("Sheet2!B5", "'My Sheet'!B5:C9") after the edit: the cells it names follow it (a place that is gone becomes "#REF!"); a name, or a place on another sheet, is as it was.
QString placeAfter(const QString &place, const QStringList &sheetNames, const StructEdit &edit)
{
    const int bang = int(place.lastIndexOf(QLatin1Char('!')));
    if (bang <= 0) return place;
    QString name = place.left(bang);
    if (name.size() >= 2 && name.startsWith(QLatin1Char('\'')) && name.endsWith(QLatin1Char('\''))) name = name.mid(1, name.size() - 2).replace(QStringLiteral("''"), QStringLiteral("'"));
    if (edit.sheet < 0 || edit.sheet >= sheetNames.size() || sheetNames[edit.sheet].compare(name, Qt::CaseInsensitive) != 0) return place;
    QString cells = place.mid(bang + 1);
    const bool dollars = cells.contains(QLatin1Char('$'));
    cells.remove(QLatin1Char('$'));
    CellRange range;
    if (!parseRangeRef(cells, &range)) return place;
    if (!adjustRange(edit, edit.sheet, &range)) return place.left(bang + 1) + QStringLiteral("#REF!");
    QString now = rangeName(range);
    if (dollars) {
        // (as written with its dollar signs: each of the two cells absolute)
        const QStringList parts = now.split(QLatin1Char(':'));
        QStringList marked;
        for (const QString &part : parts) {
            quint32 r = 0, c = 0;
            parseCellRef(part, &r, &c);
            marked << QLatin1Char('$') + columnName(c) + QLatin1Char('$') + QString::number(r + 1);
        }
        now = marked.join(QLatin1Char(':'));
    }
    return place.left(bang + 1) + now;
}

} // namespace

qint64 AnchorDelta::bytes() const
{
    return qint64(sizeof(*this)) + qint64(notes.size()) * qint64(sizeof(Note)) + qint64(touchedNotes.size()) * qint64(sizeof(TouchedNote)) + qint64(touchedLinks.size()) * qint64(sizeof(TouchedLink))
           + qint64(links.size()) * (qint64(sizeof(Link)) + 160);
}

std::shared_ptr<AnchorDelta> CellAnchors::followEdit(const StructEdit &edit, const QStringList &sheetNames)
{
    using K = StructEdit::Kind;
    if (edit.kind == K::Offset) return nullptr;
    std::set<int> sheets = {edit.sheet};
    if (edit.kind == K::Move) sheets.insert(edit.toSheet);
    auto delta = std::make_shared<AnchorDelta>();
    delta->saves = m_saves;
    struct Landing {
        int sheet;
        quint64 key;
        jp::office::sheet::NoteInfo note;
    };
    std::vector<Landing> landings;
    std::vector<std::pair<int, quint64>> touchedLandings;
    std::vector<std::pair<int, LinkSpec>> linkLandings;
    for (const int sheet : sheets) {
        if (sheet < 0 || sheet >= int(m_sheets.size())) continue;
        SheetData &d = m_sheets[size_t(sheet)];
        // the notes that move or go (only those are touched: the others stay where they are, in the map and in the delta's absence)
        struct Change {
            quint64 from;
            int toSheet;
            quint64 to;
            bool gone;
        };
        std::vector<Change> changes;
        for (const auto &n : d.notes) {
            int s = sheet;
            quint32 row = quint32(n.first >> 32), col = quint32(n.first & 0xFFFFFFFFu);
            const RefFate fate = adjustCell(edit, &s, &row, &col);
            if (fate == RefFate::Same) continue;
            changes.push_back({n.first, s, noteKey(row, col), fate == RefFate::Gone});
        }
        for (const Change &c : changes) {
            auto node = d.notes.extract(c.from);
            delta->notes.push_back({sheet, c.from, c.toSheet, c.to, c.gone, node.mapped()});
            if (!c.gone) landings.push_back({c.toSheet, c.to, std::move(node.mapped())});
        }
        // the cells marked as changed since the file was read or saved
        std::vector<Change> touchedChanges;
        for (const quint64 k : d.noteTouched) {
            int s = sheet;
            quint32 row = quint32(k >> 32), col = quint32(k & 0xFFFFFFFFu);
            const RefFate fate = adjustCell(edit, &s, &row, &col);
            if (fate == RefFate::Same) continue;
            touchedChanges.push_back({k, s, noteKey(row, col), fate == RefFate::Gone});
        }
        for (const Change &c : touchedChanges) {
            d.noteTouched.erase(c.from);
            delta->touchedNotes.push_back({sheet, c.from, c.toSheet, c.to, c.gone});
            if (!c.gone) touchedLandings.emplace_back(c.toSheet, c.to);
        }
        // the links: those that move are changed in place, and those that go are taken out (a link that goes to another sheet goes to the end of that sheet's)
        delta->linkCounts[sheet] = d.links.size();
        std::vector<LinkSpec> kept;
        for (size_t i = 0; i < d.links.size(); ++i) {
            LinkSpec &l = d.links[i];
            LinkSpec moved = l;
            bool alive = true;
            int toSheet = sheet;
            if (single(l.range) && edit.kind == K::Move) {
                quint32 row = l.range.row1, col = l.range.col1;
                const RefFate fate = adjustCell(edit, &toSheet, &row, &col);
                alive = fate != RefFate::Gone;
                moved.range.row1 = moved.range.row2 = row;
                moved.range.col1 = moved.range.col2 = col;
            } else {
                alive = adjustRange(edit, sheet, &moved.range);
            }
            if (alive && !moved.location.isEmpty()) moved.location = placeAfter(moved.location, sheetNames, edit);
            if (alive && toSheet == sheet && moved == l) {
                kept.push_back(std::move(l));
                continue;
            }
            AnchorDelta::Link change;
            change.sheet = sheet;
            change.index = i;
            change.before = l;
            change.gone = !alive;
            if (alive && toSheet == sheet) {
                kept.push_back(std::move(moved));
            } else if (alive) {
                change.crossed = true;
                change.toSheet = toSheet;
                linkLandings.emplace_back(toSheet, std::move(moved));
            } else {
                change.gone = true;
            }
            delta->links.push_back(std::move(change));
        }
        d.links = std::move(kept);   // (the links that stayed were moved into `kept`: it is the list, whether or not another link moved)
        // the ranges marked as changed
        std::vector<std::pair<RangeKey, RangeKey>> touchedMoves;
        std::vector<RangeKey> touchedGone;
        for (const RangeKey &k : d.linkTouched) {
            CellRange r;
            r.valid = true;
            std::tie(r.row1, r.col1, r.row2, r.col2) = k;
            bool alive;
            if (single(r) && edit.kind == K::Move) {
                int s = sheet;
                quint32 row = r.row1, col = r.col1;
                alive = adjustCell(edit, &s, &row, &col) != RefFate::Gone && s == sheet;
                r.row1 = r.row2 = row;
                r.col1 = r.col2 = col;
            } else {
                alive = adjustRange(edit, sheet, &r);
            }
            if (alive && keyOf(r) == k) continue;
            if (alive) touchedMoves.emplace_back(k, keyOf(r));
            else touchedGone.push_back(k);
        }
        for (const auto &m : touchedMoves) {
            d.linkTouched.erase(m.first);
            delta->touchedLinks.push_back({sheet, m.first, m.second, false});
        }
        for (const RangeKey &k : touchedGone) {
            d.linkTouched.erase(k);
            delta->touchedLinks.push_back({sheet, k, k, true});
        }
        for (const auto &m : touchedMoves) d.linkTouched.insert(m.second);
    }
    // what went to its new place, and what went to another sheet
    for (Landing &l : landings) {
        SheetData &d = sheetData(l.sheet);
        d.notes.insert_or_assign(l.key, std::move(l.note));
        d.noteTouched.insert(l.key);
    }
    for (const auto &t : touchedLandings) sheetData(t.first).noteTouched.insert(t.second);
    for (auto &l : linkLandings) {
        SheetData &d = sheetData(l.first);
        delta->linkCounts.emplace(l.first, d.links.size());
        d.links.push_back(std::move(l.second));
        touch(d, d.links.back());
    }
    return delta->empty() ? nullptr : delta;
}

void CellAnchors::undoEdit(const std::shared_ptr<AnchorDelta> &delta, const StructEdit &edit)
{
    if (!delta) return;
    const bool saved = delta->saves != m_saves;
    // the links that went to another sheet are taken off its end, last first
    for (auto it = delta->links.rbegin(); it != delta->links.rend(); ++it)
        if (it->crossed) {
            SheetData &d = sheetData(it->toSheet);
            if (!d.links.empty()) d.links.pop_back();
        }
    // the links are as they were: those that stayed in the order they have, and those that moved or went in the places they were in
    std::map<int, std::vector<const AnchorDelta::Link *>> bySheet;
    for (const AnchorDelta::Link &l : delta->links) bySheet[l.sheet].push_back(&l);
    for (auto &entry : bySheet) {
        SheetData &d = sheetData(entry.first);
        const size_t total = delta->linkCounts.count(entry.first) ? delta->linkCounts.at(entry.first) : d.links.size();
        std::vector<LinkSpec> restored;
        restored.reserve(total);
        size_t current = 0, next = 0;
        for (size_t i = 0; i < total; ++i) {
            if (next < entry.second.size() && entry.second[next]->index == i) {
                const AnchorDelta::Link &l = *entry.second[next++];
                restored.push_back(l.before);
                if (!l.gone && !l.crossed) ++current;   // (it was changed in place: its place now is passed over)
            } else if (current < d.links.size()) {
                restored.push_back(std::move(d.links[current++]));
            }
        }
        d.links = std::move(restored);
    }
    // the notes: those that moved or went, out of the places they went to (all of them first, so that none lands on another that has not moved yet), and back
    std::vector<std::pair<int, quint64>> out;
    for (const AnchorDelta::Note &n : delta->notes)
        if (!n.gone) out.emplace_back(n.toSheet, n.to);
    for (const auto &o : out)
        if (o.first >= 0 && o.first < int(m_sheets.size())) m_sheets[size_t(o.first)].notes.erase(o.second);
    for (const AnchorDelta::Note &n : delta->notes) sheetData(n.sheet).notes.insert_or_assign(n.from, n.note);
    // the cells marked as changed
    if (!saved) {
        std::vector<std::pair<int, quint64>> touchedOut;
        for (const AnchorDelta::TouchedNote &t : delta->touchedNotes)
            if (!t.gone) touchedOut.emplace_back(t.toSheet, t.to);
        for (const auto &o : touchedOut)
            if (o.first >= 0 && o.first < int(m_sheets.size())) m_sheets[size_t(o.first)].noteTouched.erase(o.second);
        for (const AnchorDelta::TouchedNote &t : delta->touchedNotes) sheetData(t.sheet).noteTouched.insert(t.from);
        for (const AnchorDelta::TouchedLink &t : delta->touchedLinks) {
            SheetData &d = sheetData(t.sheet);
            if (!t.gone) d.linkTouched.erase(t.to);
        }
        for (const AnchorDelta::TouchedLink &t : delta->touchedLinks) sheetData(t.sheet).linkTouched.insert(t.from);
    } else {
        // the file was written after the edit: it has the notes and links where the edit put them, and then where the undo of it puts them, and what is marked now stays marked where its
        // cells go back to
        const StructEdit back = edit.inverse();
        for (SheetData &d : m_sheets) {
            std::set<quint64> keys;
            for (const quint64 key : d.noteTouched) {
                int s = int(&d - m_sheets.data());
                quint32 row = quint32(key >> 32), col = quint32(key & 0xFFFFFFFFu);
                if (adjustCell(back, &s, &row, &col) != RefFate::Gone && s == int(&d - m_sheets.data())) keys.insert(noteKey(row, col));
            }
            d.noteTouched = std::move(keys);
            std::set<RangeKey> ranges;
            for (const RangeKey &key : d.linkTouched) {
                CellRange r;
                r.valid = true;
                std::tie(r.row1, r.col1, r.row2, r.col2) = key;
                if (adjustRange(back, int(&d - m_sheets.data()), &r)) ranges.insert(keyOf(r));
            }
            d.linkTouched = std::move(ranges);
        }
    }
}

void CellAnchors::permute(const CellPermute &p, bool forward)
{
    SheetData &d = sheetData(p.sheet);
    auto place = [&](quint32 *row, quint32 *col) {
        quint32 &line = p.columns ? *col : *row;
        const quint32 other = p.columns ? *row : *col;
        if (other < p.otherFirst || other > p.otherLast || line < p.first || line - p.first >= p.to.size()) return;
        if (forward) {
            line = p.to[line - p.first];
        } else {
            // (the line that went to this one)
            for (size_t i = 0; i < p.to.size(); ++i)
                if (p.to[i] == line) {
                    line = p.first + quint32(i);
                    break;
                }
        }
    };
    // (the file's notes and links go with their cells when it is written, as these do: nothing is touched by it, and what is touched goes with its cell)
    std::map<quint64, NoteInfo> notes;
    std::set<quint64> touched;
    for (const auto &n : d.notes) {
        quint32 row = quint32(n.first >> 32), col = quint32(n.first & 0xFFFFFFFFu);
        place(&row, &col);
        notes[noteKey(row, col)] = n.second;
    }
    for (const quint64 k : d.noteTouched) {
        quint32 row = quint32(k >> 32), col = quint32(k & 0xFFFFFFFFu);
        place(&row, &col);
        touched.insert(noteKey(row, col));
    }
    d.notes = std::move(notes);
    d.noteTouched = std::move(touched);
    std::set<RangeKey> linkTouched;
    for (const RangeKey &k : d.linkTouched) {
        CellRange r;
        r.valid = true;
        std::tie(r.row1, r.col1, r.row2, r.col2) = k;
        if (single(r)) {
            place(&r.row1, &r.col1);
            r.row2 = r.row1;
            r.col2 = r.col1;
        }
        linkTouched.insert(keyOf(r));
    }
    d.linkTouched = std::move(linkTouched);
    for (LinkSpec &l : d.links) {
        if (!single(l.range)) continue;
        quint32 row = l.range.row1, col = l.range.col1;
        place(&row, &col);
        l.range.row1 = l.range.row2 = row;
        l.range.col1 = l.range.col2 = col;
    }
}

void CellAnchors::addSheet(int copyOf)
{
    m_sheets.emplace_back();
    if (copyOf >= 0 && copyOf < int(m_sheets.size()) - 1) m_sheets.back().links = m_sheets[size_t(copyOf)].links;
}

void CellAnchors::removeLastSheet()
{
    if (!m_sheets.empty()) m_sheets.pop_back();
}

void CellAnchors::addToSave(SaveChanges *changes) const
{
    for (size_t i = 0; i < m_sheets.size(); ++i) {
        const SheetData &d = m_sheets[i];
        if (d.noteTouched.empty() && d.linkTouched.empty()) continue;
        SheetChanges &sc = changes->sheets[int(i)];
        for (const quint64 key : d.noteTouched) {
            NoteOp op;
            op.row = quint32(key >> 32);
            op.col = quint32(key & 0xFFFFFFFFu);
            const auto n = d.notes.find(key);
            if (n == d.notes.end()) {
                op.remove = true;
            } else {
                op.author = n->second.author;
                op.text = n->second.text;
                op.shown = n->second.shown;
                // (a note of a thread is kept by the file as it is)
                if (n->second.threaded) continue;
            }
            sc.noteOps.push_back(op);
        }
        for (const RangeKey &key : d.linkTouched) {
            CellRange r;
            r.valid = true;
            std::tie(r.row1, r.col1, r.row2, r.col2) = key;
            sc.unlinks.push_back(r);
            for (const LinkSpec &l : d.links)
                if (l.range == r) sc.links.push_back(l);
        }
    }
}

void CellAnchors::saved()
{
    ++m_saves;
    for (SheetData &d : m_sheets) {
        d.noteTouched.clear();
        d.linkTouched.clear();
    }
}

bool CellAnchors::touched() const
{
    for (const SheetData &d : m_sheets)
        if (!d.noteTouched.empty() || !d.linkTouched.empty()) return true;
    return false;
}

size_t CellAnchors::touchedNotes() const
{
    size_t n = 0;
    for (const SheetData &d : m_sheets) n += d.noteTouched.size();
    return n;
}

size_t CellAnchors::touchedLinks() const
{
    size_t n = 0;
    for (const SheetData &d : m_sheets) n += d.linkTouched.size();
    return n;
}

} // namespace jp::crunch
