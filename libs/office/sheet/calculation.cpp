#include "sheet/calculation.h"

#include "calc/budget.h"
#include "calc/formula.h"
#include "calc/functions.h"
#include "calc/idframe.h"
#include "calc/refs.h"

#include <QDateTime>

#include <algorithm>
#include <chrono>
#include <iterator>
#include <cmath>
#include <tuple>

namespace jp::office::sheet {

using namespace jp::calc;

namespace {

// A number that says which blocks these are (in the order given): two lists with the same number are taken to be the same, as the graph takes them.
quint64 hashOfAreas(const std::vector<CellArea> &areas)
{
    quint64 h = 1469598103934665603ull ^ quint64(areas.size());
    auto mix = [&](quint64 v) {
        h ^= v + 0x9E3779B97F4A7C15ull + (h << 6) + (h >> 2);
        h *= 0xFF51AFD7ED558CCDull;
        h ^= h >> 32;
    };
    for (const CellArea &a : areas) {
        mix((quint64(quint32(a.sheet1)) << 32) | quint64(quint32(a.sheet2)));
        mix((quint64(a.row1) << 32) | quint64(a.row2));
        mix((quint64(a.col1) << 32) | quint64(a.col2));
    }
    return h ? h : 1;
}

// What a step that goes a chunk at a time does as it goes: it counts what it has done (Progress::jobItems), adds it to the work of the slice (so that a slice given a work budget is
// ended by it as by the clock), and after each chunk asks whether the slice is over. The first chunk is always done, so that a call gets on.
struct Chunker {
    static constexpr size_t kChunk = 256;
    qint64 *items, *spent;
    const std::function<bool()> &timeUp;
    size_t done = 0;
    // About to do `units` more things: false if a chunk is done and the slice is over.
    bool next(size_t units = 1)
    {
        if (done >= kChunk) {
            *spent += qint64(done);
            done = 0;
            if (timeUp()) return false;
        }
        done += units;
        *items += qint64(units);
        return true;
    }
    void finish()
    {
        *spent += qint64(done);
        done = 0;
    }
};

constexpr qint64 kMaxMembersAdded = 200000;        // the cells of arrays that the file has no cell for, which are put in (each costs the cells after it in its block of rows)

// (JC_ID_FRAME=1 in the environment turns it on for every calculation of the process: how the tools, and the whole of the tests, are run in the id frame)
std::atomic<bool> g_idFrameDefault{qEnvironmentVariableIntValue("JC_ID_FRAME") == 1};

// Whether a word of the text could be read as a cell past XFD (or past the last row) under the wide limits, and so cannot be held in ids as it is: a name "ABCD1". A word that a "(" (a function:
// ATAN2), a "[" (a table), or a "!" (a sheet) goes on from is none, as the parser reads it; nor is one in quotes.
bool hasWideLookingWord(const QString &text)
{
    const int n = int(text.size());
    int i = 0;
    while (i < n) {
        const QChar c = text[i];
        if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
            int j = i + 1;
            while (j < n && text[j] != c) ++j;   // (a quote doubled inside reads as two texts, which is the same)
            i = j + 1;
        } else if (c.isLetter() || c == QLatin1Char('$') || c == QLatin1Char('_')) {
            int j = i + 1;
            while (j < n && (text[j].isLetterOrNumber() || text[j] == QLatin1Char('$') || text[j] == QLatin1Char('_') || text[j] == QLatin1Char('.'))) ++j;
            const bool goesOn = j < n && (text[j] == QLatin1Char('(') || text[j] == QLatin1Char('[') || text[j] == QLatin1Char('!'));
            if (!goesOn && j - i <= 16 && nameLooksWide(text.mid(i, j - i))) return true;
            i = j;
        } else if (c.isDigit()) {
            int j = i + 1;
            while (j < n && (text[j].isLetterOrNumber() || text[j] == QLatin1Char('.'))) ++j;
            i = j;
        } else {
            ++i;
        }
    }
    return false;
}

} // namespace

bool Calculation::textHasWideLookingWord(const QString &text) { return hasWideLookingWord(text); }

void Calculation::setIdFrameDefault(bool on) { g_idFrameDefault = on; }
bool Calculation::idFrameDefault() { return g_idFrameDefault.load(std::memory_order_relaxed); }

Calculation::Calculation(const Workbook &book, const CalcOptions &options) : WorkbookSource(book, options.workbookBytes), m_options(options), m_graph(new DependencyGraph(options.maxGraphBytes))
{
    if (g_idFrameDefault.load(std::memory_order_relaxed) && !options.idFrameOff) m_options.idFrame = true;
    if (options.idFrameOff) m_options.idFrame = false;
    m_evaluator.seedRandom(quint64(QDateTime::currentMSecsSinceEpoch()) * 0x9E3779B97F4A7C15ull + 12345);
    if (options.workLimit > 0) m_evaluator.setWorkLimit(options.workLimit);
    m_iterate = book.info().iterate;
    m_iterCount = book.info().iterateCount;
    m_iterDelta = book.info().iterateDelta;
    m_ctx.dateSystem = book.info().dateSystem;
    if (options.recalcWorkLimit > 0) m_workLimit = options.recalcWorkLimit;
    if (options.maxHeldBytes > 0) m_heldLimit = options.maxHeldBytes;
}

Calculation::~Calculation() = default;

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// formulas as nodes

Calculation::Compiled &Calculation::compiled(int sheet, qint32 text, bool array)
{
    SheetState &st = m_state[size_t(sheet)];
    std::vector<Compiled> &slots = array ? st.array : st.plain;
    if (slots.size() <= size_t(text)) slots.resize(size_t(text) + 1);
    Compiled &slot = slots[size_t(text)];
    if (!slot.tried) {
        slot.tried = true;
        ParseOptions po;
        po.arrayFormula = array;
        po.limits = limitsOf(m_sheets[size_t(sheet)], text);
        const ParseResult pr = parseFormula(m_sheets[size_t(sheet)].formulaTexts[text], po);
        slot.program = pr.program;
        slot.error = pr.error;
        if (slot.program) {
            slot.bound = bindSheetNames(*slot.program, *this);
            // (what the names it uses stand for counts: a name that calls RAND makes the formula change by itself)
            const ProgramTraits t = traitsFollowingNames(*slot.program, slot.bound, sheet, *this, &m_nameTraits);
            slot.usesNames = t.usesNames;
            slot.dynamic = t.dynamic;
            slot.reaches = t.reachesBeyond;
            slot.changesByItself = t.changesByItself;
            slot.insensitive = t.blankInsensitive;
            slot.observesPosition = t.observesPosition;
            slot.iterative = t.iterative;
        }
        slot.traitsKnown = true;
    }
    return slot;
}

void Calculation::countTexts(int sheet)
{
    SheetState &st = m_state[size_t(sheet)];
    if (st.textsCounted) return;
    const Sheet &s = m_sheets[size_t(sheet)];
    st.textRefs.assign(size_t(s.formulaTexts.size()), 0);
    for (const Formula &f : s.formulas)
        if (f.text >= 0 && f.text < s.formulaTexts.size()) ++st.textRefs[size_t(f.text)];
    st.textsCounted = true;
}

QString Calculation::internKey(const QString &text, TextClass cls)
{
    // (the same characters may be a text in ids and a text in places: they are two texts, and a text is the whole of what follows the class and the bar, whatever characters it starts with)
    return QString(QChar(char16_t('0' + int(cls)))) + QLatin1Char('|') + text;
}

Calculation::TextClass Calculation::textClass(const Sheet &s, qint32 text) const
{
    if (!s.frame) return TextClass::Plain;
    if (s.positionalTexts.contains(text)) return TextClass::Positional;
    return s.wideTexts.contains(text) ? TextClass::Wide : TextClass::Plain;
}

// Whether the text has a range written back to front (C9:A9): a text that is shown as it was written until an edit writes it again, and then in order, which only the code that writes it again can
// say; it stays in places.
static bool hasBackToFrontRange(const QString &text)
{
    if (!text.contains(QLatin1Char(':'))) return false;
    std::vector<ReferenceAt> refs;
    if (!findReferences(text, ParseOptions(), &refs)) return false;
    for (const ReferenceAt &r : refs)
        if (r.ref.written) return true;
    return false;
}

Calculation::TextClass Calculation::classOfLoaded(const QString &text) const
{
    if (!m_options.idFrame) return TextClass::Plain;
    if (hasBackToFrontRange(text)) return TextClass::Positional;
    // (a text the file has has no id past the grid; what makes it stay in places is a reference through several sheets or a word that is a cell past XFD in the wide limits)
    if (hasWideLookingWord(text)) return TextClass::Positional;
    if (text.contains(QLatin1Char('!')) && text.contains(QLatin1Char(':')) && formulaHasThreeD(text)) return TextClass::Positional;
    return TextClass::Plain;
}

Calculation::Held Calculation::heldText(int sheet, const QString &typed, const jp::calc::Program *program)
{
    Held h;
    h.text = typed;
    if (!m_options.idFrame) return h;
    bool positional = hasWideLookingWord(typed) || hasBackToFrontRange(typed);
    if (!positional && program) {
        for (const RefLit &r : program->refs)
            if (r.sheet2 >= 0 && r.sheet2 != r.sheet1) positional = true;
    }
    if (!positional && typed.contains(QLatin1Char('!')) && typed.contains(QLatin1Char(':')) && formulaHasThreeD(typed)) positional = true;
    if (!positional) {
        QString held;
        bool wide = false;
        if (placesToIds(*this, sheet, typed, &held, &wide)) {
            if (held != typed) ++m_editStats.textsToIds;
            h.text = held;
            h.cls = wide ? TextClass::Wide : TextClass::Plain;
            return h;
        }
    }
    h.cls = TextClass::Positional;
    return h;
}

qint32 Calculation::internText(int sheet, const QString &text, TextClass cls)
{
    countTexts(sheet);
    SheetState &st = m_state[size_t(sheet)];
    Sheet &s = m_sheets[size_t(sheet)];
    const QString key = internKey(text, cls);
    const auto found = st.internOf.constFind(key);
    if (found != st.internOf.constEnd()) {
        ++st.textRefs[size_t(*found)];
        return *found;
    }
    qint32 index;
    if (!st.freeTexts.empty()) {
        index = st.freeTexts.back();
        st.freeTexts.pop_back();
        s.formulaTexts[index] = text;
        st.textRefs[size_t(index)] = 1;
    } else {
        index = qint32(s.formulaTexts.size());
        s.formulaTexts.append(text);
        st.textRefs.push_back(1);
    }
    if (size_t(index) < st.plain.size()) st.plain[size_t(index)] = Compiled();
    if (size_t(index) < st.array.size()) st.array[size_t(index)] = Compiled();
    forgetTextFacts(sheet, index);
    st.internOf.insert(key, index);
    s.positionalTexts.remove(index);
    s.wideTexts.remove(index);
    if (cls == TextClass::Positional) s.positionalTexts.insert(index);
    else if (cls == TextClass::Wide) s.wideTexts.insert(index);
    return index;
}

void Calculation::holdText(int sheet, qint32 text)
{
    if (text < 0) return;
    countTexts(sheet);
    ++m_state[size_t(sheet)].textRefs[size_t(text)];
}

void Calculation::releaseText(int sheet, qint32 text)
{
    if (text < 0) return;
    countTexts(sheet);
    SheetState &st = m_state[size_t(sheet)];
    Sheet &s = m_sheets[size_t(sheet)];
    if (size_t(text) >= st.textRefs.size() || st.textRefs[size_t(text)] == 0) return;
    if (--st.textRefs[size_t(text)] > 0) return;
    // (nothing has the text now: it and the program worked out for it are let go of, and the place is for the next text)
    const auto at = st.internOf.find(internKey(s.formulaTexts[text], textClass(s, text)));
    if (at != st.internOf.end() && *at == text) st.internOf.erase(at);
    s.formulaTexts[text] = QString();
    s.positionalTexts.remove(text);
    s.wideTexts.remove(text);
    if (size_t(text) < st.plain.size()) st.plain[size_t(text)] = Compiled();
    if (size_t(text) < st.array.size()) st.array[size_t(text)] = Compiled();
    forgetTextFacts(sheet, text);
    st.freeTexts.push_back(text);
}

void Calculation::forgetTextFacts(int sheet, qint32 text)
{
    // (what was found of a text is kept by its place, which is for another text now)
    if (size_t(sheet) < m_subtotalTexts.size() && size_t(text) < m_subtotalTexts[size_t(sheet)].size()) m_subtotalTexts[size_t(sheet)][size_t(text)] = -1;
}

qint32 Calculation::allocFormula(int sheet, const Formula &f)
{
    SheetState &st = m_state[size_t(sheet)];
    Sheet &s = m_sheets[size_t(sheet)];
    qint32 index;
    if (!st.freeFormulas.empty()) {
        index = st.freeFormulas.back();
        st.freeFormulas.pop_back();
        s.formulas[index] = f;
    } else {
        index = qint32(s.formulas.size());
        s.formulas.append(f);
    }
    if (st.nodeOf.size() <= size_t(index)) st.nodeOf.resize(size_t(index) + 1, kNone);
    st.nodeOf[size_t(index)] = kNone;
    return index;
}

void Calculation::releaseFormula(int sheet, qint32 formula)
{
    Sheet &s = m_sheets[size_t(sheet)];
    if (formula < 0 || formula >= s.formulas.size()) return;
    Formula &f = s.formulas[formula];
    if (f.text == kFreeFormula) return;   // (it is free already)
    if (f.text >= 0) releaseText(sheet, f.text);
    f = Formula();
    f.text = kFreeFormula;
    SheetState &st = m_state[size_t(sheet)];
    st.freeFormulas.push_back(formula);
    if (size_t(formula) < st.nodeOf.size()) st.nodeOf[size_t(formula)] = kNone;
}

void Calculation::retext(int sheet, Cell *cell, const QString &text)
{
    const Held held = heldText(sheet, text);
    Formula &f = m_sheets[size_t(sheet)].formulas[cell->formula];
    const qint32 old = f.text;
    f.text = internText(sheet, held.text, held.cls);
    f.dRow = f.dCol = 0;
    releaseText(sheet, old);
}

qint64 Calculation::formulaTextsInUse(int sheet) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return 0;
    return qint64(m_sheets[size_t(sheet)].formulaTexts.size()) - qint64(m_state[size_t(sheet)].freeTexts.size());
}

qint64 Calculation::formulaEntriesInUse(int sheet) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return 0;
    return qint64(m_sheets[size_t(sheet)].formulas.size()) - qint64(m_state[size_t(sheet)].freeFormulas.size());
}

qint64 Calculation::programsHeld(int sheet) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return 0;
    qint64 n = 0;
    for (const Compiled &c : m_state[size_t(sheet)].plain) n += c.program ? 1 : 0;
    for (const Compiled &c : m_state[size_t(sheet)].array) n += c.program ? 1 : 0;
    return n;
}

quint32 Calculation::addNode(int sheet, quint32 row, quint32 col, const Cell &cell, qint32 formulaIndex)
{
    const CellStore &cells = m_sheets[size_t(sheet)].cells;
    return addNodeById(sheet, cells.rowIdAt(row), cells.colIdAt(col), cell, formulaIndex);
}

quint32 Calculation::addNodeById(int sheet, quint32 rowId, quint32 colId, const Cell &cell, qint32 formulaIndex)
{
    quint32 id;
    if (!m_freeNodes.empty()) {
        id = m_freeNodes.back();
        m_freeNodes.pop_back();
        const quint16 listed = m_nodes[id].flags & AlwaysListed;   // (a gone node may still be on the list of those worked out each time)
        m_nodes[id] = Node();
        m_nodes[id].flags = listed;
        m_toldHash[id] = 0;
    } else {
        id = quint32(m_nodes.size());
        m_nodes.emplace_back();
        m_index.push_back(0);
        m_low.push_back(0);
        m_toldHash.push_back(0);
    }
    Node &n = m_nodes[id];
    n.sheet = sheet;
    n.formula = formulaIndex;
    n.rowId = rowId;
    n.colId = quint16(colId);
    SheetState &st = m_state[size_t(sheet)];
    if (st.nodeOf.size() <= size_t(formulaIndex)) st.nodeOf.resize(size_t(formulaIndex) + 1, kNone);
    st.nodeOf[size_t(formulaIndex)] = id;
    ++m_formulaCount;
    if (m_sheets[size_t(sheet)].frame && !inIds(m_sheets[size_t(sheet)], m_sheets[size_t(sheet)].formulas[formulaIndex].text)) m_positionalNodes.insert(id);
    classify(id, cell, m_sheets[size_t(sheet)].formulas[formulaIndex]);
    return id;
}

void Calculation::classify(quint32 id, const Cell &cell, const Formula &f)
{
    Node &n = m_nodes[id];
    const Sheet &s = m_sheets[size_t(n.sheet)];
    const bool array = cell.flags & ArrayFormula;
    n.flags = quint16((n.flags & AlwaysListed) | (array ? Array : 0));
    n.traits = 0;   // (a formula nobody classified can notice an empty line, and sees no place)
    char kept = 0;
    bool always = false, told = false;
    if (cell.flags & DataTable) {
        kept = 'D';
    } else if (f.text < 0 || f.text >= s.formulaTexts.size() || s.formulaTexts[f.text].trimmed().isEmpty()) {
        kept = 'N';
    } else {
        const Compiled &slot = compiled(n.sheet, f.text, array);
        if (!slot.program) {
            kept = 'P';
        } else {
            const Program &p = *slot.program;
            if (p.structured) m_structuredNodes.insert(id);
            if (isSubtotal(n.sheet, rowOf(n), colOf(n))) m_subtotalNodes.insert(id);
            // (a formula that sees where cells are, ROW() and COLUMN() with nothing in the brackets among them, is not told where it reads: an edit of lines looks at the places it saw)
            told = slot.usesNames || slot.dynamic || slot.reaches;
            n.traits = quint8((slot.insensitive ? Insensitive : 0) | (slot.observesPosition ? Observes : 0));
            always = slot.dynamic || slot.changesByItself;
            if (m_options.keepApart) {
                if (!p.unknownFunctions.isEmpty()) kept = 'U';
                else if (p.external) kept = 'E';
                else if (p.structured && !m_options.workOutTables) kept = 'T';
                else if (slot.changesByItself) kept = 'V';
                else if (slot.iterative) kept = 'R';
                else if (!cell.hasResult() && !m_options.workOutWhatHasNoResult) kept = 'N';
            }
        }
    }
    if (kept) {
        if (kept != 'U') m_special.push_back(id);
        n.flags |= Kept;
        n.kept = kept;
        if (m_options.keepApart) n.flags |= NodeTainted;
        keepStored(n, kept);
        n.state = Done;
        // A formula with a function the engine lacks keeps the result the file stored until something it reads changes: then it has #NAME?. It is told to the graph, so that an edit finds it.
        if (kept == 'U') {
            // (nothing is known of what a function the engine lacks reads: where cells are, or the text of a formula; it is sensitive, and an observer, and a formula that was written again is a change)
            n.traits = Observes;
            m_hasUnknown = true;
            registerNode(id, nullptr);
        }
        return;
    }
    if (always) {
        n.flags |= Always;
        listAlways(id);
    }
    if (told) {
        n.flags |= Told;
        m_special.push_back(id);
    }
    n.state = Dirty;
    ++m_notDone;
    m_dirty.push_back(id);
    registerNode(id, nullptr);
}

void Calculation::listAlways(quint32 id)
{
    Node &n = m_nodes[id];
    if (n.flags & AlwaysListed) return;
    n.flags |= AlwaysListed;
    m_always.push_back(id);
}

void Calculation::keepStored(Node &n, char cls)
{
    const quint32 id = quint32(&n - m_nodes.data());
    if (!m_options.keepApart && cls == 'P') {
        storeResult(n, id, Value::error(ErrorKind::Name));
        return;
    }
    const Sheet &s = m_sheets[size_t(n.sheet)];
    const Cell *c = s.cells.findById(n.rowId, n.colId);
    storeResult(n, id, c ? valueOf(s, *c) : Value(), false);   // (what the file stored is what the grid shows already, and is not told)
}

void Calculation::registerNode(quint32 id, const std::vector<CellArea> *told)
{
    Node &n = m_nodes[id];
    // (what a formula's own references name does not change; what it touched as it was worked out is told only if it is not what it touched the last time)
    quint64 toldHash = 0;
    if (told) {
        m_toldSet.assign(told->begin(), told->end());
        std::sort(m_toldSet.begin(), m_toldSet.end());
        m_toldSet.erase(std::unique(m_toldSet.begin(), m_toldSet.end()), m_toldSet.end());
        toldHash = hashOfAreas(m_toldSet);
        if (m_toldHash[id] == toldHash) return;
    }
    const Sheet &s = m_sheets[size_t(n.sheet)];
    const Formula &f = s.formulas[n.formula];
    const Compiled &slot = compiled(n.sheet, f.text, n.flags & Array);
    m_areas.clear();
    if (slot.program) resolveReferences(*slot.program, slot.bound, sheetCount(), n.sheet, f.dRow, f.dCol, &m_areas, inIds(s, f.text) ? this : nullptr);
    if (told) m_areas.insert(m_areas.end(), m_toldSet.begin(), m_toldSet.end());
    std::sort(m_areas.begin(), m_areas.end());
    m_areas.erase(std::unique(m_areas.begin(), m_areas.end()), m_areas.end());
    if (n.traits & Observes) {
        if (m_areas.empty() && !told && !(n.flags & (Told | Kept))) {
            // (it sees its own cell and nothing else: no list of places is kept for it, and an edit of lines asks the formulas at or past it)
            if (!(n.traits & OwnOnly)) {
                n.traits |= OwnOnly;
                ++m_ownOnly;
            }
            noteSeenPlace(n.sheet, rowOf(n), colOf(n));
            m_seen.erase(id);
        } else {
            if (n.traits & OwnOnly) {
                n.traits &= quint8(~OwnOnly);
                --m_ownOnly;
            }
            recordSeen(id, m_areas);
        }
    }
    ++m_registrations;
    // (a formula that reads a block of several sheets is not rewritten for a change to one of them, so the graph is told again what it reads when rows move on one)
    bool threeD = false;
    qint32 spanFirst = 1 << 30, spanLast = -1;
    for (const CellArea &a : m_areas)
        if (a.sheet1 != a.sheet2) {
            threeD = true;
            spanFirst = std::min(spanFirst, a.sheet1);
            spanLast = std::max(spanLast, a.sheet2);
        }
    if (threeD) {
        n.flags |= ThreeD;
        m_threeD.insert(id);
        m_threeDSpan[id] = {spanFirst, spanLast};
    } else if (n.flags & ThreeD) {
        n.flags &= quint16(~ThreeD);
        m_threeD.erase(id);
        m_threeDSpan.erase(id);
    }
    // (the graph holds ids: the blocks the text names, which are places, are turned into the ids of their lines, a sheet at a time, as a reader is told: see calc/idframe.h)
    m_idAreas.clear();
    for (const CellArea &a : m_areas) idAreasOn(a, true, &m_idAreas);
    std::sort(m_idAreas.begin(), m_idAreas.end());
    m_idAreas.erase(std::unique(m_idAreas.begin(), m_idAreas.end()), m_idAreas.end());
    const bool tracked = m_graph->setPrecedents(id, m_idAreas, n.traits & Insensitive);   // (marked, a formula that cannot notice an empty line is left out when the graph is asked for those that can)
    m_toldHash[id] = tracked ? toldHash : 0;   // (a formula the graph refused is told again next time)
    if (!tracked) {
        if (!(n.flags & Untracked)) {
            n.flags |= Untracked;
            ++m_untracked;
        }
        if (!(n.flags & Always)) {
            n.flags |= Always;
            listAlways(id);
        }
    }
}

// A block of places as the ids the graph is told it as (reading: a formula's block) or asked it as (a change), one area for each sheet of the block: each sheet has the order of its own lines.
void Calculation::idAreasOn(const CellArea &places, bool reading, std::vector<CellArea> *out) const
{
    const qint32 last = std::min<qint32>(places.sheet2, qint32(m_sheets.size()) - 1);
    for (qint32 sheet = std::max<qint32>(0, places.sheet1); sheet <= last; ++sheet) out->push_back(idAreaOnSheet(sheet, places, reading));
}

CellArea Calculation::idAreaOnSheet(int sheet, const CellArea &places, bool reading) const
{
    const CellStore &cells = m_sheets[size_t(sheet)].cells;
    CellArea a = idArea(cells.rows(), cells.cols(), places, reading);
    a.sheet1 = a.sheet2 = sheet;
    return a;
}

// The block a change in a formula's cell (and the cells of its array) is asked as. A line that was inserted has an id the graph has no interval for: it is asked as the original lines either side.
CellArea Calculation::idAreaOfNode(const Node &n) const
{
    if (n.flags & Array) return idAreaOnSheet(n.sheet, areaOf(n), false);
    const CellStore &cells = m_sheets[size_t(n.sheet)].cells;
    CellArea a;
    a.sheet1 = a.sheet2 = n.sheet;
    if (n.rowId < cells.rows().core().limits().firstNew) {
        a.row1 = a.row2 = n.rowId;
    } else {
        const quint32 at = cells.rowPlaceOf(n.rowId);
        const IdSpan r = idSpan(cells.rows().core(), at, at, kGridRows - 1, kMaxRowId, false);
        a.row1 = r.lo;
        a.row2 = r.hi;
    }
    if (n.colId < cells.cols().core().limits().firstNew) {
        a.col1 = a.col2 = n.colId;
    } else {
        const quint32 at = cells.colPlaceOf(n.colId);
        const IdSpan c = idSpan(cells.cols().core(), at, at, kGridCols - 1, kMaxColId, false);
        a.col1 = c.lo;
        a.col2 = c.hi;
    }
    return a;
}

bool Calculation::readersOf(const CellArea &places, std::vector<quint32> *out, const std::atomic<bool> *stop) const
{
    std::vector<CellArea> ids;
    idAreasOn(places, false, &ids);
    bool whole = true;
    for (const CellArea &a : ids) {
        if (a.single()) whole = m_graph->dependentsOfCell(a.sheet1, a.row1, a.col1, out, stop) && whole;
        else whole = m_graph->dependentsOfArea(a, out, stop) && whole;
        if (!whole) break;
    }
    return whole;
}

CellArea Calculation::areaOf(const Node &n) const
{
    CellArea a;
    a.sheet1 = a.sheet2 = n.sheet;
    a.row1 = a.row2 = rowOf(n);
    a.col1 = a.col2 = colOf(n);
    if (n.flags & Array) {
        const Sheet &s = m_sheets[size_t(n.sheet)];
        const qint32 range = s.formulas[n.formula].range;
        if (range >= 0 && range < s.ranges.size() && s.ranges[range].valid) {
            a.row2 = std::min<quint32>(s.ranges[range].row2, kGridRows - 1);
            a.col2 = std::min<quint32>(s.ranges[range].col2, kMaxColumns - 1);
        }
    }
    return a;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// values

Value Calculation::nodeValue(const Node &n, quint32 id) const
{
    switch (n.kind) {
    case KindNumber: return Value::number(n.number);
    case KindText: return Value::text(m_texts.value(n.text));
    case KindBool: return Value::boolean(n.number != 0);
    case KindError: return Value::error(ErrorKind(int(n.number)));
    case KindArray: return m_arrays.value(id).at(0, 0);
    default: return Value();
    }
}

// A slot of the texts that is free holds no string (the one it held is let go of, not kept for the slot's next use).
void Calculation::freeTextSlot(qint32 slot)
{
    m_texts[slot] = QString();
    m_freeTexts.push_back(slot);
}

qint64 Calculation::textSlotBytes() const
{
    qint64 bytes = 0;
    for (const QString &t : m_texts) bytes += qint64(t.capacity()) * 2;
    return bytes;
}

qint64 Calculation::heldByValue(const Value &v)
{
    if (v.isText()) return qint64(v.text().size()) * 2 + 64;
    if (v.isArray()) return qint64(v.rows()) * v.cols() * qint64(sizeof(Value)) + 96;
    return 0;
}

qint64 Calculation::heldBy(const Node &n, quint32 id) const
{
    if (n.kind == KindText && n.text >= 0) return qint64(m_texts.value(n.text).size()) * 2 + 64;
    if (n.kind == KindArray) return heldByValue(m_arrays.value(id));
    return 0;
}

// An array formula's cells are those of its range: the rest of the array it made is not held (a formula of one cell holds one value).
Value Calculation::fitToRange(const Node &n, const Value &v) const
{
    if (!(n.flags & Array) || !v.isArray()) return v;
    const CellArea a = areaOf(n);
    const int rows = int(std::min<qint64>(a.rows(), v.rows())), cols = int(std::min<qint64>(a.cols(), v.cols()));
    if (rows == v.rows() && cols == v.cols()) return v;
    if (rows == 1 && cols == 1) return v.at(0, 0);
    Value out = Value::array(rows, cols);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j) out.set(i, j, v.at(i, j));
    return out;
}

// Tells the window the value of a node's cell (and of the cells of its array) by their ids. `fill`: an array formula whose value is no array shows it in every cell of its range.
void Calculation::tellValue(const Node &n, const Value &v, bool fill) const
{
    auto one = [&](quint32 rowId, quint32 colId, const Value &x) {
        m_valuesTold.fetch_add(1, std::memory_order_relaxed);
        m_sink(n.sheet, rowId, colId, x);
    };
    // (a value that is no array is of the node's own cell, which the node names by ids: no place is looked up for it)
    const bool many = v.isArray() || (fill && (n.flags & Array) && !(n.flags & Kept));
    if (!many) {
        one(n.rowId, n.colId, v);
        return;
    }
    // (the cells of an array are the rows and columns that follow the first one in the order, whose ids are not those of the first plus one)
    const CellStore &cells = m_sheets[size_t(n.sheet)].cells;
    m_placeLookups.fetch_add(2, std::memory_order_relaxed);
    const quint32 row0 = rowOf(n), col0 = colOf(n);
    auto tell = [&](quint32 i, quint32 j, const Value &x) {
        if (i == 0 && j == 0) {
            one(n.rowId, n.colId, x);
            return;
        }
        const quint32 r = cells.rowIdAt(row0 + i), c = cells.colIdAt(col0 + j);
        if (r != CellStore::kNoLine && c != CellStore::kNoLine) one(r, c, x);
    };
    if (v.isArray()) {
        for (int i = 0; i < v.rows(); ++i)
            for (int j = 0; j < v.cols(); ++j) tell(quint32(i), quint32(j), v.at(i, j));
    } else {
        // (the cells of one that is kept apart are the file's, not its)
        const CellArea a = areaOf(n);
        if (a.rows() * a.cols() > 1 && a.rows() * a.cols() <= kMaxArrayCellsMarked) {
            for (qint64 i = 0; i < a.rows(); ++i)
                for (qint64 j = 0; j < a.cols(); ++j) tell(quint32(i), quint32(j), v);
        } else {
            tell(0, 0, v);
        }
    }
}

void Calculation::storeResult(Node &n, quint32 id, const Value &given, bool publish)
{
    const Value v = fitToRange(n, given);
    if (publish && m_sink) tellValue(n, v, true);
    const qint64 held = heldBy(n, id);
    struct Account {
        Calculation *c;
        Node &n;
        quint32 id;
        qint64 before;
        ~Account() { c->m_heldBytes += c->heldBy(n, id) - before; }
    } account{this, n, id, held};
    if (n.kind == KindText && n.text >= 0 && !v.isText()) {
        freeTextSlot(n.text);
        n.text = -1;
    }
    if (n.kind == KindArray && !v.isArray()) m_arrays.remove(id);
    n.flags |= Worked;
    switch (v.kind()) {
    case Value::Kind::Number: n.kind = KindNumber; n.number = v.number(); break;
    case Value::Kind::Bool: n.kind = KindBool; n.number = v.boolean() ? 1 : 0; break;
    case Value::Kind::Error: n.kind = KindError; n.number = double(int(v.errorKind())); break;
    case Value::Kind::Text:
        n.kind = KindText;
        if (n.text < 0) {
            if (!m_freeTexts.empty()) {
                n.text = m_freeTexts.back();
                m_freeTexts.pop_back();
            } else {
                n.text = qint32(m_texts.size());
                m_texts.append(QString());
            }
        }
        m_texts[n.text] = v.text();
        break;
    case Value::Kind::Array:
        n.kind = KindArray;
        m_arrays.insert(id, v);
        break;
    case Value::Kind::Empty: n.kind = KindEmpty; n.number = 0; break;
    }
}

bool Calculation::formulaValue(int sheet, quint32 row, quint32 col, const Cell &c, Value *out) const
{
    const SheetState &st = m_state[size_t(sheet)];
    const quint32 id = c.formula >= 0 && size_t(c.formula) < st.nodeOf.size() ? st.nodeOf[size_t(c.formula)] : kNone;
    if (id == kNone) {
        *out = Value();
        return true;
    }
    const Node &n = m_nodes[id];
    if (m_evaluating && n.state != Done) {
        m_pending.push_back(id);
        *out = Value();
        return false;
    }
    if (n.flags & NodeTainted) m_readTaint = true;
    if (n.flags & (Mismatch | Cascade)) m_readCascade = true;
    if (c.flags & ArrayMember) {
        const Value array = m_arrays.value(id);
        *out = array.isArray() ? array.at(int(row - rowOf(n)), int(col - colOf(n))) : nodeValue(n, id);
    } else {
        *out = nodeValue(n, id);
    }
    return true;
}

Value Calculation::cell(int sheet, quint32 row, quint32 col) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size()) || col >= kMaxColumns) return Value();
    const Sheet &s = m_sheets[size_t(sheet)];
    const Cell *c = s.cells.find(row, col);
    if (!c) return Value();
    if (c->hasFormula() || (c->flags & ArrayMember)) {
        Value v;
        formulaValue(sheet, row, col, *c, &v);
        return v;
    }
    if (c->flags & Tainted) m_readTaint = true;
    return valueOf(s, *c);
}

Value Calculation::shown(int sheet, quint32 row, quint32 col) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size()) || col >= kMaxColumns) return Value();
    const Sheet &s = m_sheets[size_t(sheet)];
    const Cell *c = s.cells.find(row, col);
    if (!c) return Value();
    if ((c->hasFormula() || (c->flags & ArrayMember)) && size_t(sheet) < m_state.size()) {
        const SheetState &st = m_state[size_t(sheet)];
        const quint32 id = c->formula >= 0 && size_t(c->formula) < st.nodeOf.size() ? st.nodeOf[size_t(c->formula)] : kNone;
        if (id != kNone && m_nodes[id].state != Done && !(m_nodes[id].flags & Worked)) return valueOf(s, *c);
        Value v;
        formulaValue(sheet, row, col, *c, &v);
        return v;
    }
    return valueOf(s, *c);
}

void Calculation::visit(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2, const std::function<bool(quint32, quint32, const Value &)> &f) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return;
    if (m_visitHook) m_visitHook();
    const Sheet &s = m_sheets[size_t(sheet)];
    CellStore::RangeWalk walk;
    s.cells.forEachInRange(row1, col1, row2, std::min<quint32>(col2, kMaxColumns - 1), [&](quint32 row, quint32 col, const Cell &c) {
        if (c.hasFormula() || (c.flags & ArrayMember)) {
            Value v;
            if (!formulaValue(sheet, row, col, c, &v) || v.isEmpty()) return true;
            return f(row, col, v);
        }
        if (c.type == CellType::Empty) return true;
        if (c.flags & Tainted) m_readTaint = true;
        return f(row, col, valueOf(s, c));
    }, &walk);
    jp::calc::budget::addWork((walk.compared + walk.stepped) / 4);   // (the rows walked over to find the cells are work too: a column of a sheet of many rows is read row by row)
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// loading

bool Calculation::load()
{
    static_assert(sizeof(Node) == 32, "a formula's node is 32 bytes");
    const bool whole = loadAll();
    const int sheets = int(m_sheets.size());
    m_state.assign(size_t(sheets), SheetState());
    m_nodes.clear();
    m_index.clear();
    m_low.clear();
    m_toldHash.clear();
    m_comp.clear();
    m_counter = 0;
    m_dirty.clear();
    m_always.clear();
    m_threeD.clear();
    m_threeDSpan.clear();
    m_positionalNodes.clear();
    std::vector<quint32> masters;   // array formulas, whose cells are then made members of them
    bool cancelled = false;
    for (int si = 0; si < sheets; ++si) frameSheet(si);
    for (int si = 0; si < sheets; ++si) {
        Sheet &s = m_sheets[size_t(si)];
        SheetState &st = m_state[size_t(si)];
        st.nodeOf.assign(size_t(s.formulas.size()), kNone);
        st.plain.resize(size_t(s.formulaTexts.size()));
        st.array.resize(size_t(s.formulaTexts.size()));
        quint32 rows = 0, cols = 0;
        qint64 visited = 0;
        s.cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
            if (cancelled) return;
            // (the flag is looked at every few thousand cells: a window that is closed does not wait for the formulas of a big sheet to be registered)
            if ((++visited & 4095) == 0 && m_cancel && m_cancel->load(std::memory_order_relaxed)) {
                cancelled = true;
                return;
            }
            if (c.type != CellType::Empty || c.hasFormula()) {
                rows = std::max(rows, row + 1);
                cols = std::max(cols, col + 1);
            }
            if (!c.hasFormula() || c.formula < 0 || c.formula >= s.formulas.size()) return;
            const quint32 id = addNode(si, row, col, c, c.formula);
            if (m_nodes[id].flags & Array) masters.push_back(id);
        });
        m_rows[size_t(si)] = rows;
        m_cols[size_t(si)] = cols;
        if (cancelled) return false;
    }
    // the cells of an array formula's range, but its first, are its: they read its value (a data table's, and those of an array that is kept apart, are the file's, marked)
    qint64 added = 0;
    for (const quint32 id : masters) {
        Node &n = m_nodes[id];
        Sheet &s = m_sheets[size_t(n.sheet)];
        const qint32 range = s.formulas[n.formula].range;
        if (range < 0 || range >= s.ranges.size() || !s.ranges[range].valid) continue;
        const CellRange r = s.ranges[range];
        if (r.cellCount() <= 1 || r.cellCount() > kMaxArrayCellsMarked) continue;
        for (quint32 row = r.row1; row <= r.row2; ++row)
            for (quint32 col = r.col1; col <= r.col2; ++col) {
                if (row == rowOf(n) && col == colOf(n)) continue;
                Cell member;
                if (const Cell *have = s.cells.find(row, col)) {
                    member = *have;
                } else {
                    if (++added > kMaxMembersAdded) continue;
                }
                if (member.hasFormula()) continue;
                if (n.flags & Kept) {
                    member.flags |= Tainted;
                } else {
                    member.flags |= ArrayMember;
                    member.formula = n.formula;
                }
                s.cells.set(row, col, member);
                m_rows[size_t(n.sheet)] = std::max(m_rows[size_t(n.sheet)], row + 1);
                m_cols[size_t(n.sheet)] = std::max(m_cols[size_t(n.sheet)], col + 1);
            }
    }
    // a data table's cells are the file's, marked
    for (int si = 0; si < sheets; ++si) {
        Sheet &s = m_sheets[size_t(si)];
        const SheetState &st = m_state[size_t(si)];
        for (size_t fi = 0; fi < st.nodeOf.size(); ++fi) {
            if (st.nodeOf[fi] == kNone || m_nodes[st.nodeOf[fi]].kept != 'D') continue;
            const qint32 range = s.formulas[qint32(fi)].range;
            if (range < 0 || range >= s.ranges.size() || !s.ranges[range].valid || s.ranges[range].cellCount() > kMaxArrayCellsMarked) continue;
            const CellRange r = s.ranges[range];
            for (quint32 row = r.row1; row <= r.row2; ++row)
                for (quint32 col = r.col1; col <= r.col2; ++col)
                    if (Cell *c = s.cells.findMutable(row, col)) c->flags |= Tainted;
        }
    }
    return whole;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// the sheets

// Registers the formulas of a sheet that is there (its cells are in place): a node for each, and the cells of its arrays and data tables marked as load() does.
// The sheet's formula texts are held in the id frame (CalcOptions::idFrame): it knows the workbook it is in, and which of its texts stay in places.
void Calculation::frameSheet(int si)
{
    if (!m_options.idFrame) return;
    Sheet &s = m_sheets[size_t(si)];
    s.frame = this;
    s.frameIndex = si;
    s.positionalTexts.clear();
    s.wideTexts.clear();
    for (qint32 t = 0; t < s.formulaTexts.size(); ++t)
        if (classOfLoaded(s.formulaTexts[t]) == TextClass::Positional) s.positionalTexts.insert(t);
}

void Calculation::registerSheet(int si)
{
    Sheet &s = m_sheets[size_t(si)];
    SheetState &st = m_state[size_t(si)];
    st.nodeOf.assign(size_t(s.formulas.size()), kNone);
    st.plain.resize(size_t(s.formulaTexts.size()));
    st.array.resize(size_t(s.formulaTexts.size()));
    quint32 rows = 0, cols = 0;
    std::vector<quint32> masters;
    s.cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
        if (c.type != CellType::Empty || c.hasFormula()) {
            rows = std::max(rows, row + 1);
            cols = std::max(cols, col + 1);
        }
        if (!c.hasFormula() || c.formula < 0 || c.formula >= s.formulas.size()) return;
        const quint32 id = addNode(si, row, col, c, c.formula);
        if (m_nodes[id].flags & Array) masters.push_back(id);
    });
    m_rows[size_t(si)] = rows;
    m_cols[size_t(si)] = cols;
    for (size_t fi = 0; fi < st.nodeOf.size(); ++fi) {
        if (st.nodeOf[fi] == kNone || m_nodes[st.nodeOf[fi]].kept != 'D') continue;
        const qint32 range = s.formulas[qint32(fi)].range;
        if (range < 0 || range >= s.ranges.size() || !s.ranges[range].valid || s.ranges[range].cellCount() > kMaxArrayCellsMarked) continue;
        const CellRange r = s.ranges[range];
        for (quint32 row = r.row1; row <= r.row2; ++row)
            for (quint32 col = r.col1; col <= r.col2; ++col)
                if (Cell *c = s.cells.findMutable(row, col)) c->flags |= Tainted;
    }
}

int Calculation::addSheet(const QString &name, int copyOf)
{
    abandonNow();
    const int index = int(m_sheets.size());
    Sheet content;
    std::vector<jp::calc::LineMapCore::Renumber> rowTable, colTable;
    if (copyOf >= 0 && copyOf < index) {
        content = m_sheets[size_t(copyOf)];
        content.view.tabSelected = false;
        content.partial = false;
        // (the copy has no history: its order is put back to one run, and the cells of rows and columns that were deleted in the sheet it is of are not held again)
        // (the tables say how the ids were renumbered, for the texts in ids: with the id frame off nothing asks for them)
        content.cells.rebaseLines(true, m_options.idFrame ? &rowTable : nullptr);
        content.cells.rebaseLines(false, m_options.idFrame ? &colTable : nullptr);
    }
    if (m_options.idFrame) {
        content.frame = this;
        content.frameIndex = index;
    }
    m_sheets.push_back(std::move(content));
    m_rows.push_back(0);
    m_cols.push_back(0);
    m_subtotalTexts.emplace_back();
    m_state.emplace_back();
    if (copyOf >= 0 && copyOf < index) {
        renumberTexts(index, true, rowTable, index);
        renumberTexts(index, false, colTable, index);
    }
    forgetRuleIndex();
    if (!name.isEmpty()) m_byName.insert(name.toCaseFolded(), index);
    registerSheet(index);
    if (!m_running) beginRecalculation();
    return index;
}

void Calculation::removeLastSheet()
{
    abandonNow();
    if (m_sheets.empty()) return;
    const int last = int(m_sheets.size()) - 1;
    for (quint32 id = 0; id < quint32(m_nodes.size()); ++id)
        if (!(m_nodes[id].flags & Dead) && m_nodes[id].sheet == last) dropNode(id);
    for (auto it = m_byName.begin(); it != m_byName.end();) {
        if (it.value() == last) it = m_byName.erase(it);
        else ++it;
    }
    m_sheets.pop_back();
    m_rows.pop_back();
    m_cols.pop_back();
    if (!m_subtotalTexts.empty()) m_subtotalTexts.pop_back();
    m_state.pop_back();
    forgetRuleIndex();
}

void Calculation::setSheetNames(int sheet, const QString &oldName, const QString &newName)
{
    if (!oldName.isEmpty()) {
        const auto it = m_byName.find(oldName.toCaseFolded());
        if (it != m_byName.end() && it.value() == sheet) m_byName.erase(it);
    }
    if (!newName.isEmpty()) m_byName.insert(newName.toCaseFolded(), sheet);
}

qint64 Calculation::SheetRewrite::bytes() const
{
    qint64 n = qint64(rewritten.size()) * qint64(sizeof(EditRecord::Place));
    for (const Text &t : texts) n += qint64(t.before.size()) * 2 + 24;
    for (const auto &p : names) n += qint64(p.first.size() + p.second.size()) * 2;
    return n;
}

// A formula text changed in place (the cells that have it have the new one): what is remembered of the characters, and the programs made of them, are let go.
void Calculation::setSlotText(int sheet, qint32 slot, const QString &text)
{
    Sheet &s = m_sheets[size_t(sheet)];
    SheetState &st = m_state[size_t(sheet)];
    const TextClass cls = textClass(s, slot);
    const auto at = st.internOf.find(internKey(s.formulaTexts[slot], cls));
    if (at != st.internOf.end() && *at == slot) st.internOf.erase(at);
    s.formulaTexts[slot] = text;
    if (!st.internOf.contains(internKey(text, cls)) && st.textsCounted) st.internOf.insert(internKey(text, cls), slot);
    if (size_t(slot) < st.plain.size()) st.plain[size_t(slot)] = Compiled();
    if (size_t(slot) < st.array.size()) st.array[size_t(slot)] = Compiled();
    forgetTextFacts(sheet, slot);
}

// A formula's node made again from the cell (its text changed): it is read again for what it reads, and worked out.
void Calculation::readdNode(quint32 id)
{
    if (m_nodes[id].flags & Dead) return;
    const int sheet = m_nodes[id].sheet;
    Sheet &s = m_sheets[size_t(sheet)];
    Cell *c = s.cells.findMutableById(m_nodes[id].rowId, m_nodes[id].colId);
    if (!c || !c->hasFormula() || c->formula < 0 || c->formula >= s.formulas.size()) return;
    const qint32 formula = c->formula;
    const quint32 rowId = m_nodes[id].rowId, colId = m_nodes[id].colId;
    dropNode(id);
    const quint32 now = addNodeById(sheet, rowId, colId, *c, formula);
    queueDependents(areaOf(m_nodes[now]));
}

bool Calculation::hasThreeDReferences() const
{
    if (!m_threeD.empty()) return true;
    for (auto it = m_names.constBegin(); it != m_names.constEnd(); ++it)
        if (formulaHasThreeD(it.value(), true)) return true;
    return false;
}

bool Calculation::rewriteSheetReferences(const QString &oldName, const QString &newName, SheetRewrite *record)
{
    if (hasThreeDReferences()) return false;
    abandonNow();
    BulkEdit bulk(this);
    auto rewrite = [&](const QString &text, bool definedName, bool *changed, const jp::calc::ParseLimits &limits = jp::calc::ParseLimits()) {
        return newName.isEmpty() ? deleteSheetInFormula(text, oldName, changed, definedName, limits) : renameSheetInFormula(text, oldName, newName, changed, definedName, limits);
    };
    std::vector<quint32> again;
    for (int si = 0; si < int(m_sheets.size()); ++si) {
        Sheet &s = m_sheets[size_t(si)];
        SheetState &st = m_state[size_t(si)];
        std::vector<char> changedSlot(size_t(s.formulaTexts.size()), 0);
        bool any = false;
        for (qint32 slot = 0; slot < s.formulaTexts.size(); ++slot) {
            const QString text = s.formulaTexts[slot];
            if (text.isEmpty()) continue;
            bool changed = false;
            const QString now = rewrite(text, false, &changed, limitsOf(s, slot));
            if (!changed) continue;
            record->texts.push_back({si, slot, text});
            setSlotText(si, slot, now);
            changedSlot[size_t(slot)] = 1;
            any = true;
        }
        if (any) {
            for (qint32 fi = 0; fi < s.formulas.size(); ++fi) {
                const Formula &f = s.formulas[fi];
                if (f.text < 0 || size_t(f.text) >= changedSlot.size() || !changedSlot[size_t(f.text)]) continue;
                if (size_t(fi) >= st.nodeOf.size() || st.nodeOf[size_t(fi)] == kNone) continue;
                const quint32 id = st.nodeOf[size_t(fi)];
                const Cell *c = s.cells.findById(m_nodes[id].rowId, m_nodes[id].colId);
                record->rewritten.push_back({si, rowOf(m_nodes[id]), colOf(m_nodes[id]), m_nodes[id].rowId, m_nodes[id].colId, c && (c->flags & DataTable)});
                again.push_back(id);
            }
        }
        // the rules of validation and of conditional formatting: their formulas
        bool rulesChanged = false;
        QVector<Validation> validations = s.validations;
        for (Validation &v : validations) {
            bool c1 = false, c2 = false;
            v.formula1 = rewrite(v.formula1, false, &c1);
            v.formula2 = rewrite(v.formula2, false, &c2);
            rulesChanged = rulesChanged || c1 || c2;
        }
        if (rulesChanged) {
            record->validations.push_back({si, s.validations});
            s.validations = validations;
            forgetRuleIndex();
        }
        bool cfChanged = false;
        QVector<CondRule> rules = s.conditionalFormats;
        for (CondRule &r : rules) {
            bool c1 = false, c2 = false;
            r.formula1 = rewrite(r.formula1, false, &c1);
            r.formula2 = rewrite(r.formula2, false, &c2);
            cfChanged = cfChanged || c1 || c2;
            for (CondValue &v : r.values)
                if (v.type == CondValue::Type::Formula) {
                    bool cv = false;
                    v.value = rewrite(v.value, false, &cv);
                    cfChanged = cfChanged || cv;
                }
        }
        if (cfChanged) {
            record->conditionalFormats.push_back({si, s.conditionalFormats});
            s.conditionalFormats = rules;
        }
    }
    bool namesChanged = false;
    for (auto it = m_names.begin(); it != m_names.end(); ++it) {
        bool changed = false;
        const QString now = rewrite(it.value(), true, &changed);
        if (!changed) continue;
        record->names.push_back({it.key(), it.value()});
        it.value() = now;
        namesChanged = true;
    }
    if (namesChanged) {
        m_nameTraits.clear();
        for (SheetState &state : m_state) {
            for (Compiled &slot : state.plain) slot.tried = false;
            for (Compiled &slot : state.array) slot.tried = false;
        }
        for (quint32 id = 0; id < quint32(m_nodes.size()); ++id) {
            const Node &n = m_nodes[id];
            if ((n.flags & Dead) || !(n.flags & Told)) continue;
            again.push_back(id);
        }
    }
    std::sort(again.begin(), again.end());
    again.erase(std::unique(again.begin(), again.end()), again.end());
    for (const quint32 id : again) readdNode(id);
    if (!m_running) beginRecalculation();
    return true;
}

void Calculation::undoSheetRewrite(const SheetRewrite &record)
{
    abandonNow();
    BulkEdit bulk(this);
    std::vector<quint32> again;
    for (auto it = record.texts.rbegin(); it != record.texts.rend(); ++it) setSlotText(it->sheet, it->slot, it->before);
    for (const auto &entry : record.validations)
        if (entry.first >= 0 && entry.first < int(m_sheets.size())) {
            m_sheets[size_t(entry.first)].validations = entry.second;
            forgetRuleIndex();
        }
    for (const auto &entry : record.conditionalFormats)
        if (entry.first >= 0 && entry.first < int(m_sheets.size())) m_sheets[size_t(entry.first)].conditionalFormats = entry.second;
    bool namesChanged = false;
    for (const auto &entry : record.names) {
        m_names[entry.first] = entry.second;
        namesChanged = true;
    }
    for (const EditRecord::Place &p : record.rewritten) {
        const quint32 id = nodeAt(p.sheet, p.row, p.col);
        if (id != kNone) again.push_back(id);
    }
    if (namesChanged) {
        m_nameTraits.clear();
        for (SheetState &state : m_state) {
            for (Compiled &slot : state.plain) slot.tried = false;
            for (Compiled &slot : state.array) slot.tried = false;
        }
        for (quint32 id = 0; id < quint32(m_nodes.size()); ++id) {
            const Node &n = m_nodes[id];
            if ((n.flags & Dead) || !(n.flags & Told)) continue;
            again.push_back(id);
        }
    }
    std::sort(again.begin(), again.end());
    again.erase(std::unique(again.begin(), again.end()), again.end());
    for (const quint32 id : again) readdNode(id);
    if (!m_running) beginRecalculation();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// recalculation

// A formula kept apart because it is in a circle (a window works circles out, or goes round them): when something it reads changes it is looked at again, as it may not be in a circle now.
bool Calculation::circleKeptForRetry(Node &n) const
{
    return m_options.goRoundCircles && (n.flags & Kept) && (n.kept == 'C' || n.kept == 'I') && n.state == Done;
}

void Calculation::markDirty(quint32 id, std::vector<CellArea> *changed)
{
    Node &n = m_nodes[id];
    n.state = Dirty;
    ++m_notDone;
    m_dirty.push_back(id);
    changed->push_back(idAreaOfNode(n));
}

// A formula with a function the engine lacks, found to read a cell that has changed: its result is #NAME? from now (what the file stored is no longer right, and the engine can't say what is), and
// what reads it is to be worked out again. One that has the error already is left (nothing it gives has changed).
void Calculation::giveName(quint32 id, std::vector<CellArea> *changed)
{
    Node &n = m_nodes[id];
    if (n.kind == KindError && int(n.number) == int(ErrorKind::Name)) return;
    storeResult(n, id, Value::error(ErrorKind::Name));
    changed->push_back(idAreaOfNode(n));
}

// Marks what reads the blocks of places given, and what reads those, to be worked out. (Inside, `changed` holds the ids of the lines, as the graph is asked.)
void Calculation::markDependents(std::vector<CellArea> *changed)
{
    {
        std::vector<CellArea> ids;
        for (const CellArea &a : *changed) idAreasOn(a, false, &ids);
        changed->swap(ids);
    }
    markDependentsOfIds(changed);
}

void Calculation::markDependentsOfIds(std::vector<CellArea> *changed)
{
    std::vector<quint32> found;
    while (!changed->empty()) {
        const CellArea a = changed->back();
        changed->pop_back();
        found.clear();
        if (a.single()) m_graph->dependentsOfCell(a.sheet1, a.row1, a.col1, &found);
        else m_graph->dependentsOfArea(a, &found);
        for (const quint32 id : found) {
            Node &n = m_nodes[id];
            if (n.flags & Dead) continue;
            if ((n.flags & Kept) && n.kept == 'U' && n.state == Done) {
                giveName(id, changed);
                continue;
            }
            if (circleKeptForRetry(n)) {
                n.flags &= quint16(~(Kept | NodeTainted | Circular));
                n.kept = 0;
            }
            if ((n.flags & Kept) || n.state != Done) continue;
            markDirty(id, changed);
        }
    }
}

void Calculation::beginRecalculation()
{
    m_running = true;
    m_workUsed = 0;
    m_now = m_options.now > 0 ? m_options.now : clockSerial(m_ctx.dateSystem);
    // (what the recalculation begins with is marked a chunk at a time by stepBegin: the formulas that change by themselves, and all that read them)
    m_begin = BeginJob();
    m_begin.active = true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// what an edit of whole lines asks

// A formula that can see where cells are keeps the places it saw: its own cell, and the first and last cell of each block it names (a block names its lines by ids; the places are what it saw).
void Calculation::recordSeen(quint32 id, const std::vector<CellArea> &blocks)
{
    constexpr size_t kMostPlaces = 128;           // the most places one formula keeps; past it, it is worked out again after every edit of lines
    constexpr qint64 kMostSeenPlaces = 1 << 22;  // and the most all formulas keep (a file with a million formulas of a hundred blocks each does not make a gigabyte of them)
    const Node &n = m_nodes[id];
    Seen &s = m_seen[id];
    m_seenPlaces -= qint64(s.at.size());
    s.at.clear();
    setSeenAlways(s, false);
    const CellStore &own = m_sheets[size_t(n.sheet)].cells;
    s.at.push_back({n.sheet, n.rowId, n.colId, own.rowPlaceOf(n.rowId), own.colPlaceOf(n.colId)});
    struct Count {
        qint64 &total;
        const Seen &s;
        ~Count() { total += qint64(s.at.size()); }
    } count{m_seenPlaces, s};
    auto bound = [&](qint32 sheet, quint32 row, quint32 col) { noteSeenPlace(sheet, row, col); };
    bound(n.sheet, own.rowPlaceOf(n.rowId), own.colPlaceOf(n.colId));
    if (m_seenPlaces > kMostSeenPlaces) {
        setSeenAlways(s, true);
        return;
    }
    auto corner = [&](qint32 sheet, quint32 row, quint32 col) {
        const CellStore &cells = m_sheets[size_t(sheet)].cells;
        const quint32 r = cells.rowIdAt(row), c = cells.colIdAt(col);
        if (r != CellStore::kNoLine && c != CellStore::kNoLine) {
            s.at.push_back({sheet, r, c, row, col});
            bound(sheet, row, col);
        }
    };
    for (const CellArea &a : blocks) {
        for (qint32 sheet = std::max<qint32>(0, a.sheet1); sheet <= a.sheet2 && sheet < qint32(m_sheets.size()); ++sheet) {
            if (s.at.size() + 2 > kMostPlaces) {
                setSeenAlways(s, true);
                return;
            }
            corner(sheet, a.row1, a.col1);
            if (a.row2 != a.row1 || a.col2 != a.col1) corner(sheet, a.row2, a.col2);
        }
    }
}

bool Calculation::seenDiffers(const Seen &s) const
{
    if (s.always) return true;
    for (const SeenPlace &p : s.at) {
        if (p.sheet < 0 || size_t(p.sheet) >= m_sheets.size()) return true;
        const CellStore &cells = m_sheets[size_t(p.sheet)].cells;
        if (cells.rowPlaceOf(p.rowId) != p.row || cells.colPlaceOf(p.colId) != p.col) return true;   // (a line that died has no place)
    }
    return false;
}

void Calculation::noteSeenPlace(int sheet, quint32 row, quint32 col)
{
    if (row == CellStore::kNoLine || col == CellStore::kNoLine) return;
    if (size_t(sheet) >= m_seenMaxRow.size()) {
        m_seenMaxRow.resize(m_sheets.size(), 0);
        m_seenMaxCol.resize(m_sheets.size(), 0);
    }
    m_seenMaxRow[size_t(sheet)] = std::max(m_seenMaxRow[size_t(sheet)], row + 1);
    m_seenMaxCol[size_t(sheet)] = std::max(m_seenMaxCol[size_t(sheet)], col + 1);
}

void Calculation::setSeenAlways(Seen &s, bool on)
{
    if (s.always == on) return;
    s.always = on;
    m_seenAlways += on ? 1 : -1;
}

bool Calculation::anObserverSawAnotherPlace(int sheet, bool rows, quint32 first, bool *own) const
{
    if (own) *own = false;
    // (no formula keeps a place at or past the edit, and none is worked out after every edit: nothing moved that any formula saw)
    if (sheet >= 0 && m_seenAlways == 0 && size_t(sheet) < m_seenMaxRow.size() && (rows ? m_seenMaxRow : m_seenMaxCol)[size_t(sheet)] <= first) return false;
    if (sheet >= 0 && m_ownOnly > 0) {
        for (quint32 id = 0; id < quint32(m_nodes.size()); ++id) {
            const Node &n = m_nodes[id];
            if (!(n.traits & OwnOnly) || (n.flags & Dead) || n.sheet != sheet) continue;
            ++m_observersLooked;
            if ((rows ? rowOf(n) : colOf(n)) >= first) {
                if (own) *own = true;
                break;
            }
        }
    }
    for (const auto &entry : m_seen) {
        ++m_observersLooked;
        if (!(m_nodes[entry.first].flags & Dead) && seenDiffers(entry.second)) return true;
    }
    return false;
}

// Whether the question names any formula (cheaply: the search ends at the first, and past a count of what it has looked at without finding one it says yes, for the worker to find out).
bool Calculation::questionHasAnAnswer(const Question &q) const
{
    constexpr int kLooks = 1024;
    int looked = 0;
    bool answer = false;
    auto live = [&](quint32 node) { return !(m_nodes[node].flags & Dead); };
    auto lines = [&](quint32 first, quint32 last, CellArea *a) {
        *a = CellArea();
        a->sheet1 = a->sheet2 = q.sheet;
        a->row1 = q.rows ? first : 0;
        a->row2 = q.rows ? last : kMaxRowId;
        a->col1 = q.rows ? 0 : first;
        a->col2 = q.rows ? kMaxColId : last;
    };
    // set 3: the lines that died are read
    for (const IdRun &r : q.dead) {
        CellArea a;
        lines(r.first, r.first + r.count - 1, &a);
        m_graph->forEachDependent(a, [&](quint32 node) {
            if (live(node)) answer = true;
            return !answer;
        });
        if (answer) return true;
    }
    for (const IdRun &r : q.deadBlank) {
        CellArea a;
        lines(r.first, r.first + r.count - 1, &a);
        m_graph->forEachDependent(a, [&](quint32 node) {
            if (live(node)) answer = true;
            return !answer;
        }, true);
        if (answer) return true;
    }
    if (q.deadPair) return true;
    // set 1: a block that holds the gap, of a formula that can notice an empty line. (For columns the blocks that hold both columns are not asked for alone: those that read the column before the gap are
    // looked at, which is more, and enough to say that something may be asked.)
    if (q.gapBefore != LineMapCore::kNone && q.gapAfter != LineMapCore::kNone) {
        auto visit = [&](quint32 node) {
            if (++looked > kLooks || (live(node) && !(m_nodes[node].traits & Insensitive))) answer = true;
            return !answer;
        };
        if (q.rows) {
            m_graph->forEachHoldingRows(q.sheet, q.gapBefore, q.gapAfter, visit, true);
        } else {
            CellArea a;
            lines(q.gapBefore, q.gapBefore, &a);
            m_graph->forEachDependent(a, visit, true);
        }
        if (answer) return true;
    }
    // set 2
    return q.observers || q.ownOnly;
}

// The formulas a question names, sets 3 and 1 (set 2 is looked at by the places the formulas saw): in ids, through the graph. False if a stop ended the search.
bool Calculation::buildAsked(const Question &q, std::vector<quint32> *out, const std::atomic<bool> *stop) const
{
    std::vector<quint32> found, other;
    auto lines = [&](quint32 first, quint32 last, CellArea *a) {
        *a = CellArea();
        a->sheet1 = a->sheet2 = q.sheet;
        a->row1 = q.rows ? first : 0;
        a->row2 = q.rows ? last : kMaxRowId;
        a->col1 = q.rows ? 0 : first;
        a->col2 = q.rows ? kMaxColId : last;
    };
    auto take = [&](bool onlySensitive) {
        for (const quint32 node : found) {
            const Node &n = m_nodes[node];
            if ((n.flags & Dead) || (onlySensitive && (n.traits & Insensitive))) continue;
            out->push_back(node);
        }
        found.clear();
    };
    // set 3: the readers of the lines that died or came back, from outside them (a formula in one of them is gone with it: the graph does not hold it any more)
    for (const IdRun &r : q.dead) {
        CellArea a;
        lines(r.first, r.first + r.count - 1, &a);
        if (!m_graph->dependentsOfArea(a, &found, stop)) return false;
        take(false);
    }
    for (const IdRun &r : q.deadBlank) {
        CellArea a;
        lines(r.first, r.first + r.count - 1, &a);
        if (!m_graph->dependentsOfArea(a, &found, stop, true)) return false;
        take(true);
    }
    if (q.deadPair) {
        // (inserted lines that held cells are gone or back: the blocks that held them hold the original lines either side)
        CellArea a;
        lines(q.gapBefore == LineMapCore::kNone ? 0 : q.gapBefore, q.gapAfter == LineMapCore::kNone ? (q.rows ? kMaxRowId : kMaxColId) : q.gapAfter, &a);
        if (!m_graph->dependentsOfArea(a, &found, stop)) return false;
        take(false);
    }
    // set 1: the formulas that can notice an empty line, of which a block holds the gap
    if (q.gapBefore != LineMapCore::kNone && q.gapAfter != LineMapCore::kNone) {
        if (q.rows) {
            if (!m_graph->dependentsHoldingRows(q.sheet, q.gapBefore, q.gapAfter, &found, stop, true)) return false;
        } else {
            // (a block holds both columns: it is among the readers of each; a formula that reads one in one block and the other in another is asked too, which is more and not less)
            CellArea a;
            lines(q.gapBefore, q.gapBefore, &a);
            if (!m_graph->dependentsOfArea(a, &found, stop, true)) return false;
            lines(q.gapAfter, q.gapAfter, &a);
            if (!m_graph->dependentsOfArea(a, &other, stop, true)) return false;
            std::sort(found.begin(), found.end());
            std::sort(other.begin(), other.end());
            std::vector<quint32> both;
            std::set_intersection(found.begin(), found.end(), other.begin(), other.end(), std::back_inserter(both));
            found.swap(both);
        }
        take(true);
    }
    return true;
}

// A formula a question names is worked out again (if it is done and not kept apart); what it was worked out as is let go of if it is still on its way to the window.
void Calculation::markAsked(quint32 node, const Question &q, std::vector<CellArea> *changed)
{
    Node &n = m_nodes[node];
    if (n.flags & Dead) return;
    if ((n.flags & Kept) && n.kept == 'U' && n.state == Done) {
        giveName(node, changed);
        ++m_formulasAsked;
        return;
    }
    if (circleKeptForRetry(n)) {
        n.flags &= quint16(~(Kept | NodeTainted | Circular));
        n.kept = 0;
    }
    if ((n.flags & Kept) || n.state != Done) return;
    markDirty(node, changed);
    ++m_formulasAsked;
    if (n.sheet == q.sheet) stampAsked(n, q.epoch);
}

// Marks the formulas that are worked out in every recalculation, and those that read them, to be worked out; a chunk at a time. True when all are marked.
bool Calculation::stepBegin(const std::function<bool()> &timeUp, qint64 *spent)
{
    BeginJob &j = m_begin;
    Chunker ck{&m_items, spent, timeUp};
    // (the list is kept to the formulas that are on it: one that is gone, or not volatile now, is left off)
    while (!j.dependents && j.alwaysAt < m_always.size()) {
        if (!ck.next()) return false;
        const quint32 id = m_always[j.alwaysAt++];
        Node &n = m_nodes[id];
        if ((n.flags & Dead) || !(n.flags & Always)) {
            n.flags &= quint16(~AlwaysListed);
            continue;
        }
        m_always[j.listed++] = id;
        if (m_onlySheet >= 0 && n.sheet != m_onlySheet) continue;   // (left for the next recalculation: it stays on the list)
        if ((n.flags & Kept) || n.state != Done) continue;
        markDirty(id, &j.changed);
    }
    if (!j.dependents) {
        m_always.resize(j.listed);
        j.dependents = true;
    }
    // what the edits of whole lines asked: the question at the front is turned into the formulas it names, and those are marked (they change in their turn, below); one question at a time, each a
    // chunk at a time, and a stop ends it where it is (it is made again whole when the calculation goes on)
    while (!m_questions.empty()) {
        const Question &q = m_questions.front();
        if (!j.built) {
            if (!ck.next()) return false;
            j.asked.clear();
            j.askedAt = 0;
            if (!buildAsked(q, &j.asked, m_ctx.cancel)) {
                j.asked.clear();
                return false;
            }
            j.observers.clear();
            j.observersAt = 0;
            if (q.observers) {
                j.observers.reserve(m_seen.size());
                for (const auto &entry : m_seen) j.observers.push_back(entry.first);
            }
            j.built = true;
            if (!ck.next(j.asked.size() + j.observers.size())) return false;
        }
        while (j.askedAt < j.asked.size()) {
            if (!ck.next()) return false;
            markAsked(j.asked[j.askedAt++], q, &j.changed);
        }
        // (the formulas that saw a place that is not where it was: the places are looked at again, and the formula is worked out again)
        while (j.observersAt < j.observers.size()) {
            if (!ck.next()) return false;
            const quint32 id = j.observers[j.observersAt++];
            const auto seen = m_seen.find(id);
            if (seen == m_seen.end() || (m_nodes[id].flags & Dead) || !seenDiffers(seen->second)) continue;
            const Node &n = m_nodes[id];
            const Sheet &sheet = m_sheets[size_t(n.sheet)];
            m_areas.clear();
            const Compiled &slot = compiled(n.sheet, sheet.formulas[n.formula].text, n.flags & Array);
            if (slot.program)
                resolveReferences(*slot.program, slot.bound, sheetCount(), n.sheet, sheet.formulas[n.formula].dRow, sheet.formulas[n.formula].dCol, &m_areas,
                                  inIds(sheet, sheet.formulas[n.formula].text) ? this : nullptr);
            recordSeen(id, m_areas);
            markAsked(id, q, &j.changed);
        }
        // (the formulas that see only their own cell: those of the sheet at or past the place, the node's own place being all they saw)
        while (q.ownOnly && j.ownAt < m_nodes.size()) {
            if (!ck.next()) return false;
            const quint32 id = quint32(j.ownAt++);
            const Node &n = m_nodes[id];
            if (!(n.traits & OwnOnly) || (n.flags & Dead) || n.sheet != q.sheet) continue;
            if ((q.rows ? rowOf(n) : colOf(n)) >= q.ownFirst) markAsked(id, q, &j.changed);
        }
        ++m_questionsAnswered;
        m_questions.pop_front();
        j.ownAt = 0;
        j.built = false;
        j.asked.clear();
        j.observers.clear();
    }
    // what reads those marked: each block that changed asks the graph, and what it finds is marked and changes in its turn
    for (;;) {
        while (j.foundAt < j.found.size()) {
            if (!ck.next()) return false;
            const quint32 id = j.found[j.foundAt++];
            Node &n = m_nodes[id];
            if (n.flags & Dead) continue;
            if (m_sheetAreaOwner >= 0 && n.sheet == m_sheetAreaOwner && j.queried == m_sheetArea) continue;   // (the sheet's own formulas were worked out whole)
            if (m_onlySheet >= 0 && n.sheet != m_onlySheet) {
                // (a formula of another sheet: it is not marked now; the block it reads is read again by the next recalculation)
                if (m_deferredAreas.empty() || !(m_deferredAreas.back() == j.queried)) m_deferredAreas.push_back(j.queried);
                continue;
            }
            if ((n.flags & Kept) && n.kept == 'U' && n.state == Done) {
                giveName(id, &j.changed);
                continue;
            }
            if (circleKeptForRetry(n)) {
                n.flags &= quint16(~(Kept | NodeTainted | Circular));
                n.kept = 0;
            }
            if ((n.flags & Kept) || n.state != Done) continue;
            markDirty(id, &j.changed);
        }
        if (j.changed.empty()) break;
        if (!ck.next()) return false;
        const CellArea a = j.changed.back();
        j.changed.pop_back();
        j.found.clear();
        j.foundAt = 0;
        j.queried = a;
        // (a stop ends the search where it is: it is made again, whole, when the calculation goes on; a cell that two hundred thousand formulas read is a search that long)
        const bool whole = a.single() ? m_graph->dependentsOfCell(a.sheet1, a.row1, a.col1, &j.found, m_ctx.cancel) : m_graph->dependentsOfArea(a, &j.found, m_ctx.cancel);
        if (!whole) {
            j.found.clear();
            j.changed.push_back(a);
            return false;
        }
        // (what the graph gave: a cell that very many formulas read is one query of that many, which is as small as the step can be made. The slice may be over after it: the formulas it found are
        // marked by the next, from where they are kept. The answer of this call is not to be dropped: it is the one place in the loop where a chunk ends, when the areas found have none.)
        if (!ck.next(j.found.size())) return false;
    }
    ck.finish();
    m_dirtyCursor = 0;
    m_begin = BeginJob();
    m_sheetAreaOwner = -1;
    return true;
}

void Calculation::tablesChanged()
{
    WorkbookSource::tablesChanged();
    abandonNow();
    BulkEdit bulk(this);
    // (the formulas that name a table's columns: each is registered again, which works it out and what reads it; the ones it finds are listed anew)
    std::vector<quint32> ids(m_structuredNodes.begin(), m_structuredNodes.end());
    m_structuredNodes.clear();
    // (a defined name that names a table's columns stands for other cells now: what uses a name is registered again, with what each name stands for looked at anew)
    bool namesOfTables = false;
    for (auto it = m_names.constBegin(); it != m_names.constEnd() && !namesOfTables; ++it) namesOfTables = it.value().contains(QLatin1Char('['));
    if (namesOfTables) {
        m_nameTraits.clear();
        for (SheetState &state : m_state) {
            for (Compiled &slot : state.plain) slot.tried = false;
            for (Compiled &slot : state.array) slot.tried = false;
        }
        for (quint32 id = 0; id < quint32(m_nodes.size()); ++id) {
            const Node &n = m_nodes[id];
            if (!(n.flags & Dead) && (n.flags & Told)) ids.push_back(id);
        }
    }
    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
    for (const quint32 id : ids)
        if (id < m_nodes.size()) readdNode(id);
    if (!m_running) beginRecalculation();
}

void Calculation::hiddenRowsAndSubtotals(int sheet)
{
    hiddenRowsChanged(sheet);
    abandonNow();
    BulkEdit bulk(this);
    std::vector<quint32> ids(m_subtotalNodes.begin(), m_subtotalNodes.end());
    m_subtotalNodes.clear();
    std::sort(ids.begin(), ids.end());
    for (const quint32 id : ids)
        if (id < m_nodes.size()) readdNode(id);
    if (!m_running) beginRecalculation();
}

void Calculation::invalidateAll()
{
    abandon();
    releaseDeferred(true);
    // (every formula is marked to be worked out, a chunk at a time by the next recalculation: asking costs nothing, and the count says how many are to be done)
    m_invalidate.active = true;
    m_invalidate.at = 0;
    m_invalidate.sheet = -1;
}

void Calculation::invalidateSheet(int sheet)
{
    abandon();
    m_onlySheet = sheet;
    // (what other sheets read of this one is read again by the next recalculation, if any formula there reads it: see releaseDeferred)
    CellArea whole;
    whole.sheet1 = whole.sheet2 = sheet;
    whole.row1 = 0;
    whole.row2 = kMaxRowId;
    whole.col1 = 0;
    whole.col2 = kMaxColId;
    m_sheetArea = whole;   // (in ids, as the graph is asked)
    m_sheetAreaPending = sheet;
    m_invalidate.active = true;
    m_invalidate.at = 0;
    m_invalidate.sheet = sheet;
}

bool Calculation::stepInvalidate(const std::function<bool()> &timeUp, qint64 *spent)
{
    Chunker ck{&m_items, spent, timeUp};
    while (m_invalidate.at < m_nodes.size()) {
        if (!ck.next()) return false;
        const quint32 id = quint32(m_invalidate.at++);
        Node &n = m_nodes[id];
        if (n.flags & Dead) continue;
        if (circleKeptForRetry(n)) {
            n.flags &= quint16(~(Kept | NodeTainted | Circular));
            n.kept = 0;
        }
        if ((n.flags & Kept) || n.state != Done || (m_invalidate.sheet >= 0 && n.sheet != m_invalidate.sheet)) continue;
        n.state = Dirty;
        ++m_notDone;
        m_dirty.push_back(id);
    }
    ck.finish();
    m_invalidate = InvalidateJob();
    return true;
}

void Calculation::abandon()
{
    if (!m_running && m_stack.empty() && m_job.phase == GroupJob::None && !m_refuse.active) return;
    // what is waiting or queued is to be worked out again, with what was not yet started: the lists of them are taken in hand (which costs nothing), and put right a chunk at a time
    // by stepAbandon, in the calls that follow; the formulas of a group that was being closed or woken, and not yet given back to the stack, are to be worked out again too
    // (a stop that was already in hand, and not yet put right, is finished first: its lists are the only record of formulas marked to be worked out that no other list holds, and taking
    // these in hand over it would lose them)
    if (m_abandon.active) {
        qint64 spent = 0;
        stepAbandon([] { return false; }, &spent);
    }
    AbandonJob &a = m_abandon;
    a = AbandonJob();
    a.active = true;
    if (m_job.phase != GroupJob::None) {
        a.openEnd = m_job.phase == GroupJob::Apply ? m_job.cursor : m_job.open.size();   // (Collect and Sort: all of them are still to be given back)
        a.open = std::move(m_job.open);
    }
    a.stack = std::move(m_stack);
    a.comp = std::move(m_comp);
    a.dirty = std::move(m_dirty);
    a.dirtyAt = std::min(m_dirtyCursor, a.dirty.size());
    m_stack.clear();
    m_comp.clear();
    m_dirty.clear();
    m_job = GroupJob();
    m_refuse = RefuseJob();
    // (a beginning that is part way goes on: the formulas it has marked are to be worked out, and what reads them is still to be marked; the marks begin from the start again only when
    // there is no beginning that is part way)
    if (!m_begin.active) m_begin = BeginJob();
    m_dirtyCursor = 0;
    m_running = m_begin.active;
}

bool Calculation::stepAbandon(const std::function<bool()> &timeUp, qint64 *spent)
{
    AbandonJob &a = m_abandon;
    Chunker ck{&m_items, spent, timeUp};
    // (in the order the lists were put right in when it was done at once: the formulas of the group, the stack, the group that waits, and those not started)
    while (a.openAt < a.openEnd) {
        if (!ck.next()) return false;
        const quint32 x = a.open[a.openAt++];
        Node &n = m_nodes[x];
        if (n.state == Done) {
            if (!(n.flags & Iterating)) continue;
            // (made ready to be gone round, and not given back: it is to be worked out again)
            n.flags &= quint16(~Iterating);
            ++m_notDone;
        }
        n.state = Dirty;
        n.flags &= quint16(~Retried);
        m_dirty.push_back(x);
    }
    while (a.stackAt < a.stack.size()) {
        if (!ck.next()) return false;
        const quint32 x = a.stack[a.stackAt++].node;
        Node &n = m_nodes[x];
        if (n.state == Waiting || n.state == Queued) {
            n.state = Dirty;
            n.flags &= quint16(~Retried);
            m_dirty.push_back(x);
        }
    }
    while (a.compAt < a.comp.size()) {
        if (!ck.next()) return false;
        const quint32 x = a.comp[a.compAt++];
        Node &n = m_nodes[x];
        m_index[x] = m_low[x] = 0;
        if (n.state == Suspended) {
            n.state = Dirty;
            n.flags &= quint16(~Retried);
            m_dirty.push_back(x);
        }
    }
    while (a.dirtyAt < a.dirty.size()) {
        if (!ck.next()) return false;
        ++m_abandonWork;
        m_dirty.push_back(a.dirty[a.dirtyAt++]);
    }
    ck.finish();
    m_abandon = AbandonJob();
    return true;
}

void Calculation::abandonNow()
{
    if (m_bulk > 0) return;
    abandon();
    if (m_abandon.active) {
        qint64 spent = 0;
        stepAbandon([] { return false; }, &spent);
    }
}

Value Calculation::runFormula(quint32 id, qint64 *work)
{
    Node &n = m_nodes[id];
    const Sheet &s = m_sheets[size_t(n.sheet)];
    const Formula &f = s.formulas[n.formula];
    const Compiled &slot = compiled(n.sheet, f.text, n.flags & Array);
    m_ctx.source = this;
    m_ctx.sheet = n.sheet;
    m_ctx.transition = m_sheets[size_t(n.sheet)].transitionEvaluation;
    // (the place is read by a formula that takes the cell it is in for its own, by implicit intersection, ROW(), and the like: every formula may)
    m_placeLookups.fetch_add(2, std::memory_order_relaxed);
    m_evaluations.fetch_add(1, std::memory_order_relaxed);
    m_ctx.row = rowOf(n);
    m_ctx.col = colOf(n);
    m_ctx.shiftRows = f.dRow;
    m_ctx.shiftCols = f.dCol;
    m_ctx.idFrame = inIds(s, f.text);
    m_ctx.now = m_now;
    m_touched.clear();
    m_ctx.touched = (n.flags & Told) ? &m_touched : nullptr;
    m_pending.clear();
    m_readTaint = false;
    m_readCascade = false;
    m_evaluating = true;
    const Value v = m_evaluator.evaluate(*slot.program, m_ctx);
    m_evaluating = false;
    *work = m_evaluator.work() + kArrayCellSteps * m_evaluator.peakArrayCells();
    return v;
}

// What a formula worked out is: its result, whether it read anything kept apart, and whether it is a mismatch with what the file stored (or reads one).
bool Calculation::finishNode(Node &n, quint32 id, const Value &v)
{
    bool refused = false;
    const EvalStatus status = m_evaluator.status();
    const int unsupported = m_evaluator.unsupportedKinds();
    if (m_options.keepApart && unsupported) {
        n.flags |= Kept | NodeTainted;
        n.kept = (unsupported & 1) ? 'E' : (unsupported & 2) ? 'T' : 'U';
        keepStored(n, n.kept);
    } else if (m_options.keepApart && status != EvalStatus::Ok) {
        n.flags |= Kept | NodeTainted;
        n.kept = 'L';
        keepStored(n, 'L');
    } else if (m_heldBytes - heldBy(n, id) + heldByValue(fitToRange(n, v)) > m_heldLimit) {
        // what the formulas hold would pass its bound: this one has the error (kept apart, the file's value), and the work goes on
        giveError(n, id);
        refused = true;
    } else {
        storeResult(n, id, v);
        n.flags &= quint16(~Circular);
        if (m_readTaint) n.flags |= NodeTainted;
        else n.flags &= quint16(~NodeTainted);
        n.flags &= quint16(~(Mismatch | Cascade));
        if (m_readCascade) n.flags |= Cascade;
        if (m_options.verify) {
            const Sheet &sh = m_sheets[size_t(n.sheet)];
            const Cell *stored = cellOf(n);
            if (stored && !m_options.verify(valueOf(sh, *stored), nodeValue(n, id))) n.flags |= Mismatch;
        }
    }
    n.flags &= quint16(~Retried);
    return refused;
}

// The error a formula has that is not worked out for want of steps or room: #VALUE! (kept apart, the file's value, of the class L).
void Calculation::giveError(Node &n, quint32 id)
{
    if (m_options.keepApart) {
        n.flags |= Kept | NodeTainted;
        n.kept = 'L';
        keepStored(n, 'L');
    } else {
        storeResult(n, id, Value::error(ErrorKind::Value));
        n.flags &= quint16(~(Circular | NodeTainted | Mismatch | Cascade));
    }
}

// A formula that is not worked out for want of steps: it has the error, and is done.
void Calculation::refuseNode(quint32 id, Progress &pr)
{
    Node &n = m_nodes[id];
    if ((n.flags & Dead) || n.state == Done) return;
    giveError(n, id);
    n.flags &= quint16(~Retried);
    n.state = Done;
    --m_notDone;
    ++pr.recalculated;
    ++pr.refused;
}

// The recalculation has taken the steps it may: every formula still to be worked out has the error, and the recalculation is over. It is done a chunk at a time (stepRefuse).
void Calculation::refuseTheRest(Progress &pr)
{
    Q_UNUSED(pr);
    m_refuse = RefuseJob();
    m_refuse.active = true;
}

bool Calculation::stepRefuse(Progress &pr, const std::function<bool()> &timeUp, qint64 *spent)
{
    Chunker ck{&m_items, spent, timeUp};
    RefuseJob &j = m_refuse;
    while (j.stackAt < m_stack.size()) {
        if (!ck.next()) return false;
        refuseNode(m_stack[j.stackAt++].node, pr);
    }
    while (j.compAt < m_comp.size()) {
        if (!ck.next()) return false;
        const quint32 x = m_comp[j.compAt++];
        refuseNode(x, pr);
        m_index[x] = m_low[x] = 0;
    }
    if (j.dirtyAt < m_dirtyCursor) j.dirtyAt = m_dirtyCursor;
    while (j.dirtyAt < m_dirty.size()) {
        if (!ck.next()) return false;
        refuseNode(m_dirty[j.dirtyAt++], pr);
    }
    ck.finish();
    m_stack.clear();
    m_comp.clear();
    m_running = false;
    m_dirty.clear();
    m_dirtyCursor = 0;
    pr.overBudget = true;
    pr.finished = true;
    m_refuse = RefuseJob();
    return true;
}

// A formula in a circle has the result 0 (kept apart, the file's value, which marks what reads it).
void Calculation::markCircular(quint32 id, Progress &pr)
{
    Node &c = m_nodes[id];
    if (m_options.keepApart) {
        // (the file's value of a cell in a circle is what the last calculation left, 0 or another, or what iterating gave: it is kept, and marks what reads it)
        c.flags |= Kept | NodeTainted | Circular;
        c.kept = m_iterate ? 'I' : 'C';
        keepStored(c, c.kept);
    } else {
        storeResult(c, id, Value::number(0));
        c.flags |= Circular;
        c.flags &= quint16(~NodeTainted);
    }
    c.flags &= quint16(~Retried);
    m_circle.insert(id);
    c.state = Done;
    --m_notDone;
    ++pr.recalculated;
    ++pr.circular;
}

// `root` waits for formulas that wait, directly or through others, for it: the formulas on the group's stack from it up are a group that waits for itself. Each was worked out
// with the formulas of the group not worked out (read as empty), so what one read may be a read it would not make with their values; a group is therefore given a second
// try, each formula worked out again, and one that waits again is in a circle: it has the result 0, as has every other formula of the group that waits again.
// Begins the job of that (a group may be a million formulas; stepGroup does it a chunk at a time, between which the clock is read).
void Calculation::closeGroup(quint32 root, quint32 parent, Progress &pr)
{
    Q_UNUSED(pr);
    m_job = GroupJob();
    m_job.phase = GroupJob::Collect;
    m_job.closing = true;
    m_job.root = root;
    m_job.parent = parent;
}

// `node` is worked out, and the formulas suspended since it began to wait were waiting for it or for a way round it: they are worked out again (a job, as closeGroup's).
void Calculation::leaveGroup(quint32 node, quint32 parent)
{
    m_job = GroupJob();
    m_job.phase = GroupJob::Collect;
    m_job.closing = false;
    m_job.root = node;
    m_job.parent = parent;
}

bool Calculation::stepGroup(Progress &pr, const std::function<bool()> &timeUp, qint64 *spent)
{
    GroupJob &j = m_job;
    Chunker ck{&m_items, spent, timeUp};
    if (j.phase == GroupJob::Collect) {
        // the formulas of the group, from the top of its stack down to the one that began it
        while (!m_comp.empty()) {
            if (!ck.next()) return false;
            const quint32 x = m_comp.back();
            m_comp.pop_back();
            m_index[x] = m_low[x] = 0;
            if (!j.closing && x == j.root) break;   // (the formula that is worked out is not woken)
            if (j.closing) ++pr.circleWork;
            const Node &c = m_nodes[x];
            if (c.state != Done) {
                j.open.push_back(x);
                if (j.closing && !(c.flags & Retried)) j.fresh = true;
            }
            if (j.closing && x == j.root) break;
        }
        j.phase = GroupJob::Sort;
        j.sortPass = 0;
        j.sortStage = 0;
        j.sortAt = 0;
    }
    if (j.phase == GroupJob::Sort) {
        // (three passes of a radix sort, each of two parts, counting the places and putting them where they go, and each of those in blocks: nothing is more than a chunk between two looks
        // at the clock)
        constexpr size_t kBlock = 128;
        const size_t n = j.open.size();
        if (n < 512) {
            std::sort(j.open.begin(), j.open.end());
            j.sortPass = 3;
        }
        while (j.sortPass < 3) {
            const int shift = 11 * j.sortPass;
            if (j.sortStage == 0) {
                if (j.sortAt == 0) j.sortCount.assign(2049, 0);
                while (j.sortAt < n) {
                    if (!ck.next(kBlock)) return false;
                    const size_t end = std::min(n, j.sortAt + kBlock);
                    for (; j.sortAt < end; ++j.sortAt) ++j.sortCount[((j.open[j.sortAt] >> shift) & 2047) + 1];
                }
                for (size_t i = 0; i < 2048; ++i) j.sortCount[i + 1] += j.sortCount[i];
                j.sortTmp.resize(n);
                j.sortStage = 1;
                j.sortAt = 0;
            }
            while (j.sortAt < n) {
                if (!ck.next(kBlock)) return false;
                const size_t end = std::min(n, j.sortAt + kBlock);
                for (; j.sortAt < end; ++j.sortAt) {
                    const quint32 x = j.open[j.sortAt];
                    j.sortTmp[j.sortCount[(x >> shift) & 2047]++] = x;
                }
            }
            j.open.swap(j.sortTmp);
            ++j.sortPass;
            j.sortStage = 0;
            j.sortAt = 0;
        }
        j.sortCount = std::vector<size_t>();
        j.sortTmp = std::vector<quint32>();
        j.cursor = j.open.size();
        j.phase = GroupJob::Apply;
    }
    // the formulas of a circle of a workbook that is to be gone round: each is worked out again and again, reading the latest values of the others
    if (j.phase == GroupJob::Apply && j.closing && !j.fresh && m_iterate && m_options.goRoundCircles) {
        j.phase = GroupJob::Iterate;
        j.stage = 0;
        j.round = 0;
        j.at = 0;
        j.change = 0;
    }
    if (j.phase == GroupJob::Iterate) {
        const size_t n = j.open.size();
        if (j.stage == 0) {
            if (j.at == 0) {
                j.start.assign(n, Value());
                j.fromFile.assign(n, 0);
            }
            while (j.at < n) {
                if (!ck.next()) return false;
                const size_t place = j.at;
                const quint32 id = j.open[j.at++];
                Node &x = m_nodes[id];
                if (x.state != Done) {
                    x.state = Done;
                    x.flags |= Iterating;
                    --m_notDone;
                }
                // (a formula never worked out begins from the result its file stored, as a program going on from where the file was left would)
                if (!(x.flags & Worked)) {
                    const Sheet &sheet = m_sheets[size_t(x.sheet)];
                    if (const Cell *stored = cellOf(x)) {
                        j.start[place] = valueOf(sheet, *stored);
                        j.fromFile[place] = 1;
                        storeResult(x, id, j.start[place], false);
                    }
                    x.flags &= quint16(~Worked);
                }
            }
            j.stage = 1;
            j.at = 0;
            j.round = 0;
            j.change = 0;
        }
        const quint32 rounds = quint32(std::clamp(m_iterCount, 1, kMaxIterations));
        while (j.stage == 1) {
            if (j.at >= n) {
                // a round is done: another, unless nothing changed by as much as it may, or the work there is to do is used
                ++j.round;
                const bool settled = j.change < m_iterDelta || j.change == 0;
                if (settled && j.round == 1 && std::find(j.fromFile.begin(), j.fromFile.end(), char(0)) == j.fromFile.end()) {
                    // the first round moved no value by as much as it may: the file's results stand, as they were stored
                    for (size_t k = 0; k < n; ++k) storeResult(m_nodes[j.open[k]], j.open[k], j.start[k], false);
                }
                j.at = 0;
                j.change = 0;
                if (settled || j.round >= rounds || m_workUsed >= m_workLimit) {
                    j.stage = 2;
                    break;
                }
                continue;
            }
            if (!ck.next()) return false;
            const quint32 id = j.open[j.at];
            Node &x = m_nodes[id];
            const Value was = x.kind == KindArray ? m_arrays.value(id) : nodeValue(x, id);
            qint64 work = 0;
            const Value v = runFormula(id, &work);
            if (m_evaluator.status() == EvalStatus::Cancelled) return false;   // (stopped in the middle of it: it is worked out again)
            ++j.at;
            *spent += work;
            pr.work += work;
            m_workUsed += work;
            ++pr.evaluated;
            if (x.flags & Told) registerNode(id, &m_touched);
            storeResult(x, id, v, false);
            if (was.isNumber() && v.isNumber()) j.change = std::max(j.change, std::fabs(was.number() - v.number()));
            else if (was.debug() != v.debug()) j.change = std::max(j.change, 1e308);
        }
        // the values the rounds came to are given, and the formulas are done
        while (j.at < n) {
            if (!ck.next()) return false;
            const quint32 id = j.open[j.at++];
            Node &x = m_nodes[id];
            if (m_sink) tellValue(x, x.kind == KindArray ? m_arrays.value(id) : nodeValue(x, id), false);
            x.flags &= quint16(~(Iterating | Circular | Retried | NodeTainted));
            ++pr.recalculated;
        }
        ck.finish();
        m_job = GroupJob();
        return true;
    }
    // the formulas of the group, each given back to the stack (the first of them is worked out first), or the result of a circle
    const bool circle = j.closing && !j.fresh;
    while (j.cursor > 0) {
        if (!ck.next()) return false;
        const quint32 x = j.open[--j.cursor];
        if (circle) {
            markCircular(x, pr);
        } else if (j.closing) {
            m_nodes[x].flags |= Retried;
            m_nodes[x].state = Queued;
            m_stack.push_back({x, j.parent});
        } else {
            m_nodes[x].state = Queued;
            m_stack.push_back({x, j.parent});
        }
    }
    ck.finish();
    m_job = GroupJob();
    return true;
}

bool Calculation::formulaPlace(quint32 node, int *sheet, quint32 *row, quint32 *col) const
{
    if (node >= m_nodes.size() || (m_nodes[node].flags & Dead)) return false;
    *sheet = m_nodes[node].sheet;
    *row = rowOf(m_nodes[node]);
    *col = colOf(m_nodes[node]);
    return true;
}

Calculation::Progress Calculation::recalculate(qint64 budget, const std::atomic<bool> *cancel, qint64 timeBudgetMicros)
{
    const qint64 before = m_items;
    m_onlyFresh = false;
    Progress pr = recalculateSlice(budget, cancel, timeBudgetMicros);
    pr.jobItems = m_items - before;
    return pr;
}

Calculation::Progress Calculation::recalculateFresh(qint64 budget, const std::atomic<bool> *cancel, qint64 timeBudgetMicros)
{
    if (m_fresh.empty()) {
        Progress done;
        done.finished = true;
        return done;
    }
    const qint64 before = m_items;
    m_onlyFresh = true;
    Progress pr = recalculateSlice(budget, cancel, timeBudgetMicros);
    pr.jobItems = m_items - before;
    return pr;
}

Calculation::Progress Calculation::recalculateSlice(qint64 budget, const std::atomic<bool> *cancel, qint64 timeBudgetMicros)
{
    Progress pr;
    qint64 spent = 0;
    const auto started = std::chrono::steady_clock::now();
    const std::atomic<bool> *stop = cancel ? cancel : m_cancel;
    m_ctx.cancel = stop;
    auto stopped = [&] { return stop && stop->load(std::memory_order_relaxed); };
    // The slice is over when its time is, or when the work it was given is done: the steps that go a chunk at a time add what they do to `spent`, as a formula's steps are added to it, so
    // that a slice with a work budget is as bounded as one with a time (and a test can count it).
    const std::function<bool()> timeUp = [&] {
        // (a stop asked for ends the step at its next chunk: a window that wants the calculation for an edit waits for a chunk, and not for the rest of the slice)
        if (stopped()) return true;
        if (budget > 0 && spent >= budget) return true;
        return timeBudgetMicros > 0 && std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - started).count() >= timeBudgetMicros;
    };
    // (what a stop left to put right, marking every formula to be worked out again, and the marks a recalculation begins with, are done a chunk at a time, in the slices)
    if (m_abandon.active) {
        if (!stepAbandon(timeUp, &spent)) return pr;
    }
    if (m_invalidate.active) {
        if (stopped()) {
            pr.cancelled = true;
            return pr;
        }
        if (!stepInvalidate(timeUp, &spent)) return pr;
    }
    if (!m_running) beginRecalculation();
    if (m_begin.active && !m_onlyFresh) {
        if (stopped()) {
            abandon();
            pr.cancelled = true;
            return pr;
        }
        if (!stepBegin(timeUp, &spent)) return pr;
    }
    for (;;) {
        if (stopped()) {
            abandon();
            pr.cancelled = true;
            return pr;
        }
        if (m_refuse.active) {
            stepRefuse(pr, timeUp, &spent);
            return pr;
        }
        if (m_job.phase != GroupJob::None) {
            if (!stepGroup(pr, timeUp, &spent)) return pr;
            if (budget > 0 && spent >= budget) return pr;
            continue;
        }
        if (m_stack.empty()) {
            quint32 root = kNone;
            while (m_dirtyCursor < m_dirty.size()) {
                const quint32 id = m_dirty[m_dirtyCursor++];
                const Node &n = m_nodes[id];
                if (n.state == Dirty && !(n.flags & Dead)) {
                    if ((m_onlySheet >= 0 && n.sheet != m_onlySheet) || (m_onlyFresh && !m_fresh.count(id))) {
                        m_deferredRoots.push_back(id);   // (it waits for the next recalculation)
                        continue;
                    }
                    root = id;
                    break;
                }
            }
            if (root == kNone) break;
            m_nodes[root].state = Queued;
            m_stack.push_back({root, kNone});
        }
        if (budget > 0 && spent >= budget) return pr;
        if (pr.evaluated > 0 && timeUp()) return pr;
        const Frame frame = m_stack.back();
        const quint32 id = frame.node;
        {
            const Node &top = m_nodes[id];
            if (top.state == Done || top.state == Suspended || (top.flags & Dead)) {
                m_stack.pop_back();
                continue;
            }
        }
        if (m_workUsed >= m_workLimit) {
            refuseTheRest(pr);
            continue;
        }
        qint64 work = 0;
        const Value v = runFormula(id, &work);
        if (m_evaluator.status() == EvalStatus::Cancelled) {
            // the formula was stopped in the middle: what it gave is of no worth, and it is to be worked out again
            abandon();
            pr.cancelled = true;
            return pr;
        }
        Node &n = m_nodes[id];
        spent += work;
        pr.work += work;
        m_workUsed += work;
        ++pr.evaluated;
        // (what it read through a name, INDIRECT, or OFFSET is told each time it is worked out, whether or not it can be finished: a circle it closes is broken by an edit to that)
        if (n.flags & Told) registerNode(id, &m_touched);
        if (m_pending.empty()) {
            if (finishNode(n, id, v)) ++pr.refused;
            n.state = Done;
            --m_notDone;
            ++pr.recalculated;
            m_stack.pop_back();
            if (m_index[id]) leaveGroup(id, frame.parent);
            continue;
        }
        // it read formulas that are not worked out: those that can be are worked out first, and it again after them
        std::sort(m_pending.begin(), m_pending.end());
        m_pending.erase(std::unique(m_pending.begin(), m_pending.end()), m_pending.end());
        if (!m_index[id]) {
            m_index[id] = m_low[id] = ++m_counter;
            m_comp.push_back(id);
        }
        const size_t before = m_stack.size();
        for (const quint32 y : m_pending) {
            ++pr.circleWork;
            Node &d = m_nodes[y];
            if (y == id || d.state == Waiting || d.state == Suspended) {
                // one it waits for already, or itself, or one suspended: it is in a group with the formulas from there up
                m_low[id] = std::min(m_low[id], m_index[y]);
            } else {
                if (d.state == Dirty) d.state = Queued;
                m_stack.push_back({y, id});
            }
        }
        if (m_stack.size() > before) {
            n.state = Waiting;
            pr.deepest = std::max<qint64>(pr.deepest, qint64(m_stack.size()));
            continue;
        }
        // it waits only for formulas on the way to it, or itself, or those suspended: it cannot be worked out until they are
        m_stack.pop_back();
        if (m_low[id] < m_index[id]) {
            n.state = Suspended;
            if (frame.parent != kNone) m_low[frame.parent] = std::min(m_low[frame.parent], m_low[id]);
            continue;
        }
        closeGroup(id, frame.parent, pr);
    }
    m_running = false;
    m_dirty.clear();
    m_dirtyCursor = 0;
    m_fresh.clear();
    m_onlyFresh = false;
    releaseDeferred();
    pr.finished = true;
    return pr;
}

// What a recalculation of one sheet left for the others is waiting again: the formulas that were to be started from, and the blocks whose readers were to be marked.
void Calculation::releaseDeferred(bool discardSheetArea)
{
    m_onlySheet = -1;
    const int pending = m_sheetAreaPending;
    m_sheetAreaPending = -1;
    m_sheetAreaOwner = -1;
    if (pending >= 0 && !discardSheetArea) {
        // (the sheet that was worked out is read by formulas of another: only then is anything left waiting for it)
        std::vector<quint32> readers;
        m_graph->dependentsOfArea(m_sheetArea, &readers);
        bool other = false;
        for (const quint32 id : readers)
            if (!(m_nodes[id].flags & Dead) && m_nodes[id].sheet != pending) {
                other = true;
                break;
            }
        if (other) {
            m_deferredAreas.push_back(m_sheetArea);
            m_sheetAreaOwner = pending;
        }
    }
    for (const quint32 id : m_deferredRoots)
        if (id < m_nodes.size() && m_nodes[id].state == Dirty && !(m_nodes[id].flags & Dead)) m_dirty.push_back(id);
    m_deferredRoots.clear();
    m_dirtyCursor = 0;
    const std::vector<CellArea> areas = std::move(m_deferredAreas);
    m_deferredAreas.clear();
    if (!areas.empty() && !m_running) beginRecalculation();
    for (const CellArea &a : areas) m_begin.changed.push_back(a);   // (these are ids already)
}

void Calculation::setIteration(bool on, int count, double delta)
{
    count = std::clamp(count, 1, kMaxIterations);
    if (!(delta >= 0)) delta = 0;
    const bool changed = on != m_iterate || count != m_iterCount || delta != m_iterDelta;
    m_iterate = on;
    m_iterCount = count;
    m_iterDelta = delta;
    if (changed) invalidateAll();   // (what is in a circle is worked out again)
}

const Calculation::Node &Calculation::circleNode(quint32 id) const
{
    ++m_circleWork;   // (every formula circularCells looks at is counted here)
    return m_nodes[id];
}

qint64 Calculation::circularCells(std::vector<CircularCell> *out, qint64 cap) const
{
    // (the formulas that were found in circles are kept as they are found, so that nothing looks at every formula; one that has stopped being in a circle is let go of when met)
    std::vector<CircularCell> all;
    for (auto it = m_circle.begin(); it != m_circle.end();) {
        const Node &n = circleNode(*it);
        if ((n.flags & (Dead | Circular)) != Circular) {
            it = m_circle.erase(it);
            continue;
        }
        if (n.state == Done) all.push_back({n.sheet, rowOf(n), colOf(n)});
        ++it;
    }
    const qint64 found = qint64(all.size());
    if (!out || cap <= 0 || found == 0) return found;
    // (in the order of the sheets and cells, so that the first of them are the first of the sheets)
    std::sort(all.begin(), all.end(), [](const CircularCell &a, const CircularCell &b) { return std::tie(a.sheet, a.row, a.col) < std::tie(b.sheet, b.row, b.col); });
    for (size_t i = 0; i < all.size() && qint64(i) < cap; ++i) out->push_back(all[i]);
    return found;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// edits

void Calculation::useExtent(int sheet, quint32 row, quint32 col)
{
    m_rows[size_t(sheet)] = std::max(m_rows[size_t(sheet)], row + 1);
    m_cols[size_t(sheet)] = std::max(m_cols[size_t(sheet)], col + 1);
}

void Calculation::dropNode(quint32 id)
{
    Node &n = m_nodes[id];
    if (n.flags & Dead) return;
    if (!(n.flags & Kept) && n.state != Done) --m_notDone;
    m_heldBytes -= heldBy(n, id);
    if (n.kind == KindText && n.text >= 0) freeTextSlot(n.text);
    m_arrays.remove(id);
    m_graph->clearPrecedents(id);
    if (!m_positionalNodes.empty()) m_positionalNodes.erase(id);
    if (n.flags & ThreeD) {
        m_threeD.erase(id);
        m_threeDSpan.erase(id);
    }
    if (n.traits & OwnOnly) --m_ownOnly;
    if (n.traits & Observes) {
        const auto seen = m_seen.find(id);
        if (seen != m_seen.end()) {
            m_seenPlaces -= qint64(seen->second.at.size());
            if (seen->second.always) --m_seenAlways;
            m_seen.erase(seen);
        }
    }
    SheetState &st = m_state[size_t(n.sheet)];
    if (size_t(n.formula) < st.nodeOf.size()) st.nodeOf[size_t(n.formula)] = kNone;
    n.flags = quint16(Dead | (n.flags & AlwaysListed));
    n.state = Done;
    n.kind = KindEmpty;
    n.text = -1;
    --m_formulaCount;
    m_freeNodes.push_back(id);
}

void Calculation::queueDependents(const CellArea &area)
{
    if (!m_running) beginRecalculation();
    idAreasOn(area, false, &m_begin.changed);   // (the places are turned into ids now: an edit before the worker gets to it changes none)
}

bool Calculation::setValue(int sheet, quint32 row, quint32 col, const Value &v, Marking marking, qint32 sharedString)
{
    if (sheet < 0 || sheet >= int(m_sheets.size()) || col >= kMaxColumns || row >= kGridRows || v.isArray()) return false;
    Sheet &s = m_sheets[size_t(sheet)];
    const Cell *found = s.cells.find(row, col);
    if (found && ((found->flags & (ArrayMember | ArrayFormula)))) return false;   // (a cell of an array is not edited alone)
    abandonNow();
    Cell c = found ? *found : Cell();
    if (found && found->hasFormula()) {
        const SheetState &st = m_state[size_t(sheet)];
        if (found->formula >= 0 && size_t(found->formula) < st.nodeOf.size() && st.nodeOf[size_t(found->formula)] != kNone) dropNode(st.nodeOf[size_t(found->formula)]);
        releaseFormula(sheet, found->formula);   // (it is a value now: nothing has the formula)
    }
    c.flags &= quint8(~(HasFormula | SharedFormula | DataTable | Tainted));
    c.formula = -1;
    switch (v.kind()) {
    case Value::Kind::Number: c.type = CellType::Number; c.number = v.number(); break;
    case Value::Kind::Bool: c.type = CellType::Bool; c.number = v.boolean() ? 1 : 0; break;
    case Value::Kind::Error: c.type = CellType::Error; c.number = double(int(v.errorKind())); break;
    case Value::Kind::Text:
        if (sharedString >= 0 && sharedString < m_book.sharedStrings().strings.size()) {
            c.type = CellType::SharedString;
            c.number = double(sharedString);
        } else {
            if (c.type == CellType::InlineString && c.textIndex() >= 0 && c.textIndex() < s.texts.size()) {
                s.texts[c.textIndex()] = v.text();
            } else {
                s.texts.append(v.text());
                c.number = double(s.texts.size() - 1);
            }
            c.type = CellType::InlineString;
        }
        break;
    default: c.type = CellType::Empty; c.number = 0; break;
    }
    if (c.type == CellType::Empty && c.style == 0) s.cells.erase(row, col);
    else s.cells.set(row, col, c);
    if (c.type != CellType::Empty) {
        useExtent(sheet, row, col);
        s.extentRows = std::max(s.extentRows, row + 1);
        s.extentCols = std::max<quint32>(s.extentCols, col + 1);
    }
    CellArea a;
    a.sheet1 = a.sheet2 = sheet;
    a.row1 = a.row2 = row;
    a.col1 = a.col2 = col;
    if (marking == Marking::Later) {
        queueDependents(a);
        return true;
    }
    std::vector<CellArea> changed;
    changed.push_back(a);
    markDependents(&changed);
    return true;
}

bool Calculation::setFormula(int sheet, quint32 row, quint32 col, const QString &formula, Marking marking, const Value *stored)
{
    if (sheet < 0 || sheet >= int(m_sheets.size()) || col >= kMaxColumns || row >= kGridRows) return false;
    ParseOptions po;
    const ParseResult parsed = parseFormula(formula, po);
    if (!parsed.ok()) return false;
    Sheet &s = m_sheets[size_t(sheet)];
    const Cell *found = s.cells.find(row, col);
    if (found && (found->flags & (ArrayMember | ArrayFormula))) return false;
    abandonNow();
    SheetState &st = m_state[size_t(sheet)];
    Cell c = found ? *found : Cell();
    if (found && found->hasFormula() && found->formula >= 0) {
        if (size_t(found->formula) < st.nodeOf.size() && st.nodeOf[size_t(found->formula)] != kNone) dropNode(st.nodeOf[size_t(found->formula)]);
        releaseFormula(sheet, found->formula);   // (the cell had a formula: nothing has it now)
    }
    Formula f;
    {
        const Held held = heldText(sheet, formula, parsed.program.get());
        f.text = internText(sheet, held.text, held.cls);
    }
    c.flags = quint8((c.flags & ~(SharedFormula | DataTable | Tainted)) | HasFormula);
    c.formula = allocFormula(sheet, f);
    c.type = CellType::Empty;
    c.number = 0;
    if (stored) {
        switch (stored->kind()) {
        case Value::Kind::Number: c.type = CellType::Number; c.number = stored->number(); break;
        case Value::Kind::Bool: c.type = CellType::Bool; c.number = stored->boolean() ? 1 : 0; break;
        case Value::Kind::Error: c.type = CellType::Error; c.number = double(int(stored->errorKind())); break;
        case Value::Kind::Text:
            s.texts.append(stored->text());
            c.type = CellType::FormulaString;
            c.number = double(s.texts.size() - 1);
            break;
        default: break;
        }
    }
    s.cells.set(row, col, c);
    useExtent(sheet, row, col);
    s.extentRows = std::max(s.extentRows, row + 1);
    s.extentCols = std::max<quint32>(s.extentCols, col + 1);
    const quint32 id = addNode(sheet, row, col, c, c.formula);
    m_fresh.insert(id);
    const CellArea a = areaOf(m_nodes[id]);
    // a formula typed with a function the engine lacks has no result to keep: it has #NAME?, which the recalculation gives it
    const bool needsName = m_nodes[id].kept == 'U' && !c.hasResult();
    if (marking == Marking::Later) {
        queueDependents(a);
        if (needsName) m_begin.found.push_back(id);
        return true;
    }
    std::vector<CellArea> changed;
    changed.push_back(a);
    if (needsName) giveName(id, &changed);
    markDependents(&changed);
    return true;
}

bool Calculation::setStyle(int sheet, quint32 row, quint32 col, quint32 style)
{
    if (sheet < 0 || sheet >= int(m_sheets.size()) || col >= kMaxColumns || row >= kGridRows) return false;
    Sheet &s = m_sheets[size_t(sheet)];
    Cell *found = s.cells.findMutable(row, col);
    if (!found) {
        if (style == 0) return true;
        Cell c;
        c.style = style;
        s.cells.set(row, col, c);
    } else if (found->style != style) {
        found->style = style;
        if (found->type == CellType::Empty && !found->hasFormula() && style == 0) s.cells.erase(row, col);
    }
    if (style != 0) {
        s.extentRows = std::max(s.extentRows, row + 1);
        s.extentCols = std::max<quint32>(s.extentCols, col + 1);
    }
    return true;
}

Calculation::CellContent Calculation::content(int sheet, quint32 row, quint32 col) const
{
    CellContent out;
    if (sheet < 0 || sheet >= int(m_sheets.size()) || col >= kMaxColumns || row >= kGridRows) return out;
    const Sheet &s = m_sheets[size_t(sheet)];
    const Cell *c = s.cells.find(row, col);
    if (!c) return out;
    out.exists = true;
    out.style = c->style;
    if (c->flags & (ArrayMember | ArrayFormula | DataTable)) out.editable = false;
    if (c->hasFormula()) {
        out.isFormula = true;
        out.formula = s.formulaText(*c);
        if ((c->flags & (ArrayFormula | DataTable)) && c->formula >= 0) {
            const qint32 range = s.formulas[c->formula].range;
            if (c->flags & DataTable) out.dataTable = true;
            else if (range >= 0 && range < s.ranges.size() && s.ranges[range].valid) out.arrayRange = s.ranges[range];
        }
        const quint32 id = nodeAt(sheet, row, col);
        if (id != kNone && (m_nodes[id].flags & Kept)) {
            out.hasStored = true;
            out.stored = nodeValue(m_nodes[id], id);
        }
        return out;
    }
    switch (c->type) {
    case CellType::Empty: break;
    case CellType::Number:
    case CellType::Date: out.value = Value::number(c->number); break;
    case CellType::SharedString:
        out.value = Value::text(m_book.sharedStrings().at(c->textIndex()));
        out.sharedString = c->textIndex();
        break;
    case CellType::InlineString:
    case CellType::FormulaString: out.value = Value::text(s.texts.value(c->textIndex())); break;
    case CellType::Bool: out.value = Value::boolean(c->number != 0); break;
    case CellType::Error: out.value = Value::error(ErrorKind(std::max(0, std::min(int(ErrorKind::Unknown), int(c->number))))); break;
    }
    return out;
}

bool Calculation::put(int sheet, quint32 row, quint32 col, const CellContent &c, Marking marking)
{
    if (!c.exists) {
        if (!setValue(sheet, row, col, Value(), marking)) return false;
        return setStyle(sheet, row, col, 0);
    }
    const bool ok = c.isFormula ? setFormula(sheet, row, col, c.formula, marking, c.hasStored ? &c.stored : nullptr) : setValue(sheet, row, col, c.value, marking, c.sharedString);
    return ok && setStyle(sheet, row, col, c.style);
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// about the formulas

qint64 Calculation::positionalFormulas() const { return qint64(m_positionalNodes.size()); }

qint64 Calculation::keptCount() const
{
    qint64 n = 0;
    for (const Node &node : m_nodes)
        if (!(node.flags & Dead) && (node.flags & Kept)) ++n;
    return n;
}

quint32 Calculation::nodeAt(int sheet, quint32 row, quint32 col) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return kNone;
    const Cell *c = m_sheets[size_t(sheet)].cells.find(row, col);
    if (!c || !c->hasFormula() || c->formula < 0) return kNone;
    const SheetState &st = m_state[size_t(sheet)];
    return size_t(c->formula) < st.nodeOf.size() ? st.nodeOf[size_t(c->formula)] : kNone;
}

bool Calculation::staleResult(int sheet, quint32 rowId, quint32 colId, quint32 epoch, Target *redo) const
{
    if (sheet >= 0 && sheet < int(m_state.size()) && int(epoch - m_state[size_t(sheet)].renumberEpoch) < 0) return true;   // (named by ids that were renumbered since)
    if (m_stampCount.load(std::memory_order_relaxed) == 0) return false;
    std::lock_guard<std::mutex> guard(m_stampLock);
    const auto at = m_stamps.find(stampKey(sheet, rowId, colId));
    if (at == m_stamps.end() || int(epoch - at->second.epoch) >= 0) return false;   // (epochs wrap: the difference says which is older)
    if (redo) *redo = at->second.redo;
    return true;
}

void Calculation::redirtyCell(int sheet, quint32 rowId, quint32 colId)
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return;
    const Cell *c = m_sheets[size_t(sheet)].cells.findById(rowId, colId);
    if (!c || !c->hasFormula() || c->formula < 0) return;
    const SheetState &st = m_state[size_t(sheet)];
    if (size_t(c->formula) >= st.nodeOf.size() || st.nodeOf[size_t(c->formula)] == kNone) return;
    redirty(st.nodeOf[size_t(c->formula)]);
    if (!m_running) beginRecalculation();
}

void Calculation::pruneStamps(const std::vector<quint32> &epochs)
{
    std::lock_guard<std::mutex> guard(m_stampLock);
    for (auto it = m_stamps.begin(); it != m_stamps.end();) {
        const int sheet = int(it->first >> 40);
        if (sheet < int(epochs.size()) && int(epochs[size_t(sheet)] - it->second.epoch) >= 0) it = m_stamps.erase(it);
        else ++it;
    }
    m_stampCount.store(m_stamps.size(), std::memory_order_relaxed);
}

void Calculation::stampCell(int sheet, quint32 rowId, quint32 colId, const Target &redo)
{
    if (sheet < 0 || sheet >= int(m_state.size()) || !resultsInFlight()) return;
    std::lock_guard<std::mutex> guard(m_stampLock);
    m_stamps[stampKey(sheet, rowId, colId)] = Stamp{m_state[size_t(sheet)].epoch, redo};
    m_stampCount.store(m_stamps.size(), std::memory_order_relaxed);
}

// A formula the worker's questions marked to be worked out again after an edit of lines: a result of it worked out before the edit is let go of (the formula is worked out again already, so no
// redo is asked). The stamp is of the edit that asked; one made by a later edit stays.
void Calculation::stampAsked(const Node &n, quint32 epoch)
{
    if (!resultsInFlight()) return;
    std::lock_guard<std::mutex> guard(m_stampLock);
    Stamp &s = m_stamps[stampKey(n.sheet, n.rowId, n.colId)];
    if (s.epoch == 0 || int(epoch - s.epoch) > 0) s = Stamp{epoch, Target()};
    m_stampCount.store(m_stamps.size(), std::memory_order_relaxed);
}

// A formula that is to be worked out again though nothing it reads was changed (its cell moved).
void Calculation::redirty(quint32 id)
{
    Node &n = m_nodes[id];
    if (n.flags & Dead) return;
    if (n.state == Done) {
        n.state = Dirty;
        ++m_notDone;
        m_dirty.push_back(id);
    }
}

bool Calculation::isCircular(int sheet, quint32 row, quint32 col) const
{
    const quint32 id = nodeAt(sheet, row, col);
    return id != kNone && (m_nodes[id].flags & Circular);
}

bool Calculation::isTainted(int sheet, quint32 row, quint32 col) const
{
    const quint32 id = nodeAt(sheet, row, col);
    return id != kNone && (m_nodes[id].flags & NodeTainted);
}

bool Calculation::isCascaded(int sheet, quint32 row, quint32 col) const
{
    const quint32 id = nodeAt(sheet, row, col);
    return id != kNone && (m_nodes[id].flags & Cascade);
}

char Calculation::keptClass(int sheet, quint32 row, quint32 col) const
{
    const quint32 id = nodeAt(sheet, row, col);
    return id != kNone ? m_nodes[id].kept : 0;
}

void Calculation::forEachFormulaText(const std::function<void(int, quint32, quint32, const QString &)> &f) const
{
    for (int si = 0; si < int(m_sheets.size()); ++si) {
        const Sheet &s = m_sheets[size_t(si)];
        s.cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
            if (!c.hasFormula() || (c.flags & (ArrayMember | ArrayFormula | DataTable))) return;
            f(si, row, col, s.formulaText(c));
        });
    }
}

void Calculation::forEachFormula(const std::function<void(const FormulaCell &)> &f) const
{
    for (int si = 0; si < int(m_sheets.size()); ++si) {
        const Sheet &s = m_sheets[size_t(si)];
        const SheetState &st = m_state[size_t(si)];
        s.cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
            if (!c.hasFormula() || c.formula < 0 || size_t(c.formula) >= st.nodeOf.size() || st.nodeOf[size_t(c.formula)] == kNone) return;
            const quint32 id = st.nodeOf[size_t(c.formula)];
            const Node &n = m_nodes[id];
            FormulaCell out;
            out.sheet = si;
            out.row = row;
            out.col = col;
            out.cell = &c;
            out.stored = valueOf(s, c);
            out.ours = nodeValue(n, id);
            out.kept = n.kept;
            out.tainted = n.flags & NodeTainted;
            out.circular = n.flags & Circular;
            out.mismatch = n.flags & Mismatch;
            out.cascaded = n.flags & Cascade;
            out.worked = n.state == Done;
            out.changesByItself = false;
            out.readsWhereToldOrNames = n.flags & Told;
            out.program = nullptr;
            const Formula &fm = s.formulas[n.formula];
            if (fm.text >= 0 && fm.text < s.formulaTexts.size()) {
                const std::vector<Compiled> &slots = (n.flags & Array) ? st.array : st.plain;
                if (size_t(fm.text) < slots.size() && slots[size_t(fm.text)].program) {
                    const Compiled &slot = slots[size_t(fm.text)];
                    out.changesByItself = slot.changesByItself;
                    out.program = slot.program.get();
                }
            }
            f(out);
        });
    }
}

} // namespace jp::office::sheet
