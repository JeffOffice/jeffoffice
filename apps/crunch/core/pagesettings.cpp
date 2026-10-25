#include "core/pagesettings.h"

#include <algorithm>

namespace jp::crunch {

using jp::office::sheet::PageSetup;
using jp::office::sheet::StructEdit;

namespace {

// The most steps whose earlier setups are kept for Undo (the undo stack holds a thousand).
constexpr size_t kMaxKept = 4096;

bool saysSomething(const PageSetup &p) { return p.hasPrintArea() || p.titles.hasRows || p.titles.hasCols || !p.rowBreaks.isEmpty() || !p.colBreaks.isEmpty(); }

} // namespace

void PageSettings::open(const jp::office::sheet::Calculation &calc)
{
    m_sheets.clear();
    m_touched.clear();
    m_kept.clear();
    m_kinds.clear();
    m_zooms.clear();
    for (int i = 0; i < calc.sheetCount(); ++i) {
        m_sheets.push_back(calc.sheet(i).page);
        m_kinds.push_back(int(calc.sheet(i).view.kind));
        m_zooms.push_back(calc.sheet(i).view.zoom);
    }
    m_fileKinds = m_kinds;
}

void PageSettings::setView(int index, int kind, int zoom)
{
    if (index < 0 || index >= int(m_kinds.size())) return;
    m_kinds[size_t(index)] = std::clamp(kind, 0, 2);
    m_zooms[size_t(index)] = std::clamp(zoom, 10, 400);
}

const PageSetup &PageSettings::sheet(int index) const
{
    static const PageSetup none;
    return index >= 0 && index < int(m_sheets.size()) ? m_sheets[size_t(index)] : none;
}

void PageSettings::set(int index, const PageSetup &setup)
{
    if (index < 0 || index >= int(m_sheets.size())) return;
    // (the drawing the file names for the pictures is the file's, not a step's: a step that was made before a save has the one the file had then)
    const QString rel = m_sheets[size_t(index)].headerFooter.drawingRel;
    m_sheets[size_t(index)] = setup;
    m_sheets[size_t(index)].headerFooter.drawingRel = rel;
    m_touched.insert(index);
}

void PageSettings::followEdit(const StructEdit &edit, const QStringList &sheetNames, const void *token)
{
    Kept kept;
    kept.token = token;
    std::set<int> affected = {edit.sheet};
    if (edit.kind == StructEdit::Kind::Move) affected.insert(edit.toSheet);
    for (const int s : affected) {
        if (s < 0 || s >= int(m_sheets.size())) continue;
        kept.before.emplace_back(s, m_sheets[size_t(s)]);
        const PageSetup was = m_sheets[size_t(s)];
        jp::office::sheet::followStructEdit(&m_sheets[size_t(s)], edit, s, sheetNames);
        // (a sheet whose setup the edit changed, or that names cells, is written again with what it is now)
        if (m_sheets[size_t(s)] != was || saysSomething(m_sheets[size_t(s)])) m_touched.insert(s);
    }
    // a name on a sheet the edit is not on can name the cells that moved (a print area that reads another sheet is not one, but a title can only be of its own sheet): nothing more follows
    m_kept.push_back(std::move(kept));
    if (m_kept.size() > kMaxKept) m_kept.erase(m_kept.begin(), m_kept.begin() + std::ptrdiff_t(m_kept.size() - kMaxKept));
}

void PageSettings::undoEdit(const void *token)
{
    for (size_t i = m_kept.size(); i-- > 0;) {
        if (m_kept[i].token != token) continue;
        // (a sheet put back is written again: the file may have it as the step left it)
        for (const auto &[s, setup] : m_kept[i].before)
            if (s >= 0 && s < int(m_sheets.size())) {
                const QString rel = m_sheets[size_t(s)].headerFooter.drawingRel;
                m_sheets[size_t(s)] = setup;
                m_sheets[size_t(s)].headerFooter.drawingRel = rel;
                m_touched.insert(s);
            }
        m_kept.erase(m_kept.begin() + std::ptrdiff_t(i));
        return;
    }
}

void PageSettings::addSheet(const PageSetup &setup, bool touched)
{
    m_sheets.push_back(setup);
    m_kinds.push_back(0);
    m_zooms.push_back(100);
    m_fileKinds.push_back(0);
    if (touched) m_touched.insert(int(m_sheets.size()) - 1);
}

void PageSettings::removeLastSheet()
{
    if (m_sheets.empty()) return;
    const int last = int(m_sheets.size()) - 1;
    m_sheets.pop_back();
    if (!m_kinds.empty()) {
        m_kinds.pop_back();
        m_zooms.pop_back();
        m_fileKinds.pop_back();
    }
    m_touched.erase(last);
    for (Kept &k : m_kept) k.before.erase(std::remove_if(k.before.begin(), k.before.end(), [last](const auto &b) { return b.first == last; }), k.before.end());
}

void PageSettings::addToSave(jp::office::sheet::SaveChanges *changes, const QStringList &sheetNames, const QVector<int> &order) const
{
    for (size_t s = 0; s < m_kinds.size() && s < m_fileKinds.size(); ++s) {
        if (m_kinds[s] == m_fileKinds[s] || int(s) >= sheetNames.size() || sheetNames[int(s)].isEmpty()) continue;
        jp::office::sheet::SheetChanges &sc = changes->sheets[int(s)];
        sc.viewKind = m_kinds[s];
        sc.viewZoom = m_zooms[s];
    }
    for (const int s : m_touched) {
        if (s < 0 || s >= int(m_sheets.size()) || s >= sheetNames.size() || sheetNames[s].isEmpty()) continue;   // (a deleted sheet has no name, and none of its names are written)
        const int place = order.isEmpty() ? s : int(order.indexOf(s));   // (the names of a sheet say where it is among the tabs)
        if (place < 0) continue;
        jp::office::sheet::SheetChanges &sc = changes->sheets[s];
        sc.setPage = true;
        sc.page = m_sheets[size_t(s)];
        jp::office::sheet::PrintNameChange n;
        n.sheet = place;
        n.sheetName = sheetNames[s];
        n.setup = m_sheets[size_t(s)];
        changes->printNames.push_back(n);
    }
}

void PageSettings::setHeaderDrawing(int index, const QString &rel)
{
    if (index >= 0 && index < int(m_sheets.size())) m_sheets[size_t(index)].headerFooter.drawingRel = rel;
}

void PageSettings::saved()
{
    m_fileKinds = m_kinds;
    m_touched.clear();   // (what the steps kept stays: an Undo after a save puts the setups back, and they are written again)
}

} // namespace jp::crunch
