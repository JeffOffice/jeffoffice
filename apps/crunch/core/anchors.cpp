#include "core/anchors.h"

#include <algorithm>

namespace jp::crunch {

using namespace jp::office::sheet;

namespace {

const std::map<quint64, NoteInfo> kNoNotes;
const std::vector<LinkSpec> kNoLinks;

// The most edits kept to take back (each step of Undo holds one).
constexpr size_t kMaxKept = 1000;

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
    m_kept.clear();
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

void CellAnchors::followEdit(const StructEdit &edit, const QStringList &sheetNames, const void *token)
{
    using K = StructEdit::Kind;
    if (edit.kind == K::Offset) return;
    // the sheets the edit may change something of
    std::set<int> sheets = {edit.sheet};
    if (edit.kind == K::Move) sheets.insert(edit.toSheet);
    Kept kept;
    kept.token = token;
    kept.saves = m_saves;
    // Everything on the sheet that is not where it was, or not there, is rebuilt: the notes by cell, the links by range.
    struct MovedNote {
        int sheet;
        quint32 row, col;
        NoteInfo note;
        bool touched;
    };
    std::vector<MovedNote> movedNotes;
    std::vector<std::pair<int, LinkSpec>> movedLinks;
    std::vector<std::pair<int, RangeKey>> movedLinkTouched;
    for (const int sheet : sheets) {
        if (sheet < 0) continue;
        SheetData &d = sheetData(sheet);
        // notes
        std::map<quint64, NoteInfo> notes;
        std::set<quint64> touched;
        for (const auto &n : d.notes) {
            int s = sheet;
            quint32 row = quint32(n.first >> 32), col = quint32(n.first & 0xFFFFFFFFu);
            const RefFate fate = adjustCell(edit, &s, &row, &col);
            const bool isTouched = d.noteTouched.count(n.first) != 0;
            if (fate == RefFate::Gone) continue;
            if (s == sheet) {
                notes[noteKey(row, col)] = n.second;
                if (isTouched) touched.insert(noteKey(row, col));
            } else {
                movedNotes.push_back({s, row, col, n.second, isTouched});
            }
        }
        // (a note written over by cells moved onto it is gone, and its place may hold one that moved there: the moved ones win)
        std::set<quint64> touchedOnly;
        for (const quint64 k : d.noteTouched) {
            int s = sheet;
            quint32 row = quint32(k >> 32), col = quint32(k & 0xFFFFFFFFu);
            const RefFate fate = adjustCell(edit, &s, &row, &col);
            if (fate == RefFate::Gone) continue;
            if (s == sheet) touchedOnly.insert(noteKey(row, col));
        }
        // links
        std::vector<LinkSpec> links;
        for (const LinkSpec &l : d.links) {
            LinkSpec moved = l;
            bool alive = true;
            if (single(l.range) && edit.kind == K::Move) {
                int s = sheet;
                quint32 row = l.range.row1, col = l.range.col1;
                const RefFate fate = adjustCell(edit, &s, &row, &col);
                if (fate == RefFate::Gone) alive = false;
                else {
                    moved.range.row1 = moved.range.row2 = row;
                    moved.range.col1 = moved.range.col2 = col;
                    if (s != sheet) {
                        movedLinks.emplace_back(s, moved);
                        continue;
                    }
                }
            } else {
                alive = adjustRange(edit, sheet, &moved.range);
            }
            if (!alive) continue;
            if (!moved.location.isEmpty()) moved.location = adjustFormula(moved.location, sheet, sheetNames, edit);
            links.push_back(moved);
        }
        std::set<RangeKey> linkTouched;
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
            if (alive) linkTouched.insert(keyOf(r));
        }
        if ((!d.notes.empty() || !d.links.empty() || !d.noteTouched.empty() || !d.linkTouched.empty()) && !kept.before.count(sheet)) kept.before[sheet] = d;
        d.notes = std::move(notes);
        d.noteTouched = std::move(touched);
        for (const quint64 k : touchedOnly) d.noteTouched.insert(k);
        d.links = std::move(links);
        d.linkTouched = std::move(linkTouched);
    }
    // what went to another sheet (a block of cells moved there)
    for (MovedNote &m : movedNotes) {
        SheetData &d = sheetData(m.sheet);
        if (!kept.before.count(m.sheet)) kept.before[m.sheet] = d;
        d.notes[noteKey(m.row, m.col)] = m.note;
        d.noteTouched.insert(noteKey(m.row, m.col));
    }
    for (auto &m : movedLinks) {
        SheetData &d = sheetData(m.first);
        if (!kept.before.count(m.first)) kept.before[m.first] = d;
        d.links.push_back(m.second);
        touch(d, m.second);
    }
    // (what the sheets held before is kept in full: a step is taken back to exactly it)
    if (!kept.before.empty()) {
        if (m_kept.size() >= kMaxKept) m_kept.erase(m_kept.begin());
        m_kept.push_back(std::move(kept));
    }
}

void CellAnchors::undoEdit(const void *token, const StructEdit &edit)
{
    for (size_t i = m_kept.size(); i-- > 0;) {
        if (m_kept[i].token != token) continue;
        Kept k = std::move(m_kept[i]);
        m_kept.erase(m_kept.begin() + qsizetype(i));
        const bool saved = k.saves != m_saves;
        for (auto &entry : k.before) {
            SheetData &now = sheetData(entry.first);
            if (saved) {
                // the file was written after the edit: it has the notes and links where the edit put them, and then where the undo of it puts them; what is touched now stays touched
                // where the cells go back to
                SheetData restored = std::move(entry.second);
                restored.noteTouched.clear();
                restored.linkTouched.clear();
                const StructEdit back = edit.inverse();
                for (const quint64 key : now.noteTouched) {
                    int s = entry.first;
                    quint32 row = quint32(key >> 32), col = quint32(key & 0xFFFFFFFFu);
                    if (adjustCell(back, &s, &row, &col) != RefFate::Gone && s == entry.first) restored.noteTouched.insert(noteKey(row, col));
                }
                for (const RangeKey &key : now.linkTouched) {
                    CellRange r;
                    r.valid = true;
                    std::tie(r.row1, r.col1, r.row2, r.col2) = key;
                    if (adjustRange(back, entry.first, &r)) restored.linkTouched.insert(keyOf(r));
                }
                now = std::move(restored);
            } else {
                now = std::move(entry.second);
            }
        }
        return;
    }
    // (a step that moved nothing of ours: the edits that follow it are taken back the way they went)
    (void)edit;
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
