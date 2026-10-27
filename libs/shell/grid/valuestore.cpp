#include "grid/valuestore.h"

#include <algorithm>
#include <array>

namespace jp::grid {

namespace {

constexpr quint64 kEmptyKey = ~quint64(0);
constexpr int kChunkBits = 10;   // 1,024 small tables to a sheet

quint64 mix(quint64 x)
{
    x ^= x >> 33;
    x *= 0xFF51AFD7ED558CCDull;
    x ^= x >> 33;
    x *= 0xC4CEB9FE1A85EC53ull;
    x ^= x >> 33;
    return x;
}

quint64 keyOf(quint32 row, quint32 col) { return (quint64(row) << 16) | (col & 0xFFFF); }

} // namespace

struct ValueStore::Slot {
    quint64 key = kEmptyKey;
    double number = 0;
    qint32 text = -1;
    quint8 kind = 0;   // jp::calc::Value::Kind
    quint8 error = 0;
};

struct ValueStore::Chunk {
    std::vector<Slot> slots;
    size_t used = 0;
};

struct ValueStore::Table {
    std::array<Chunk, size_t(1) << kChunkBits> chunks;
};

ValueStore::ValueStore() = default;
ValueStore::~ValueStore() = default;

void ValueStore::clear()
{
    m_sheets.clear();
    m_texts.clear();
    m_freeTexts.clear();
    m_count = 0;
}

ValueStore::Table &ValueStore::table(int sheet)
{
    if (size_t(sheet) >= m_sheets.size()) m_sheets.resize(size_t(sheet) + 1);
    if (!m_sheets[size_t(sheet)]) m_sheets[size_t(sheet)] = std::make_unique<Table>();
    return *m_sheets[size_t(sheet)];
}

void ValueStore::apply(const std::vector<Item> &items)
{
    using jp::calc::Value;
    for (const Item &it : items) {
        if (it.sheet < 0 || it.col > 0xFFFF) continue;
        Table &t = table(it.sheet);
        const quint64 key = keyOf(it.row, it.col);
        const quint64 h = mix(key);
        Chunk &c = t.chunks[size_t(h >> (64 - kChunkBits))];
        if (c.slots.empty()) c.slots.assign(16, Slot());
        // (grown while it is less than three-fifths full: the table of one chunk, which holds a thousandth of the sheet's values)
        if ((c.used + 1) * 5 > c.slots.size() * 3) {
            std::vector<Slot> bigger(c.slots.size() * 2);
            const size_t mask = bigger.size() - 1;
            for (const Slot &s : c.slots) {
                if (s.key == kEmptyKey) continue;
                size_t at = size_t(mix(s.key)) & mask;
                while (bigger[at].key != kEmptyKey) at = (at + 1) & mask;
                bigger[at] = s;
            }
            c.slots.swap(bigger);
        }
        const size_t mask = c.slots.size() - 1;
        size_t at = size_t(h) & mask;
        while (c.slots[at].key != kEmptyKey && c.slots[at].key != key) at = (at + 1) & mask;
        Slot &s = c.slots[at];
        if (s.key == kEmptyKey) {
            s.key = key;
            ++c.used;
            ++m_count;
        } else if (s.text >= 0) {
            // (the text it held is let go of)
            m_texts[size_t(s.text)] = QString();
            m_freeTexts.push_back(s.text);
            s.text = -1;
        }
        const Value &v = it.value.isArray() ? it.value.at(0, 0) : it.value;
        s.kind = quint8(v.kind());
        s.number = 0;
        s.error = 0;
        switch (v.kind()) {
        case Value::Kind::Number: s.number = v.number(); break;
        case Value::Kind::Bool: s.number = v.boolean() ? 1 : 0; break;
        case Value::Kind::Error: s.error = quint8(v.errorKind()); break;
        case Value::Kind::Text:
            if (!m_freeTexts.empty()) {
                s.text = m_freeTexts.back();
                m_freeTexts.pop_back();
                m_texts[size_t(s.text)] = v.text();
            } else {
                s.text = qint32(m_texts.size());
                m_texts.push_back(v.text());
            }
            break;
        default: break;
        }
    }
}

bool ValueStore::erase(int sheet, quint32 row, quint32 col)
{
    if (sheet < 0 || size_t(sheet) >= m_sheets.size() || !m_sheets[size_t(sheet)] || col > 0xFFFF) return false;
    Table &t = *m_sheets[size_t(sheet)];
    const quint64 key = keyOf(row, col);
    const quint64 h = mix(key);
    Chunk &c = t.chunks[size_t(h >> (64 - kChunkBits))];
    if (c.slots.empty()) return false;
    const size_t mask = c.slots.size() - 1;
    size_t i = size_t(h) & mask;
    while (c.slots[i].key != key) {
        if (c.slots[i].key == kEmptyKey) return false;
        i = (i + 1) & mask;
    }
    if (c.slots[i].text >= 0) {
        m_texts[size_t(c.slots[i].text)] = QString();
        m_freeTexts.push_back(c.slots[i].text);
    }
    // (the places after it that were put there because it was in the way are moved up: no mark is left in the table)
    size_t hole = i;
    for (size_t j = (i + 1) & mask; c.slots[j].key != kEmptyKey; j = (j + 1) & mask) {
        const size_t ideal = size_t(mix(c.slots[j].key)) & mask;
        const bool between = hole <= j ? (ideal > hole && ideal <= j) : (ideal > hole || ideal <= j);
        if (between) continue;
        c.slots[hole] = c.slots[j];
        hole = j;
    }
    c.slots[hole] = Slot();
    --c.used;
    --m_count;
    return true;
}

void ValueStore::dropSheet(int sheet)
{
    if (sheet < 0 || size_t(sheet) >= m_sheets.size() || !m_sheets[size_t(sheet)]) return;
    for (Chunk &c : m_sheets[size_t(sheet)]->chunks)
        for (Slot &s : c.slots) {
            if (s.key == kEmptyKey) continue;
            if (s.text >= 0) {
                m_texts[size_t(s.text)] = QString();
                m_freeTexts.push_back(s.text);
            }
            --m_count;
        }
    m_sheets[size_t(sheet)].reset();
}

void ValueStore::renumber(int sheet, bool rows, const std::vector<Renumbering> &table)
{
    if (sheet < 0 || size_t(sheet) >= m_sheets.size() || !m_sheets[size_t(sheet)]) return;
    auto renumbered = [&](quint32 id) -> quint32 {
        auto it = std::upper_bound(table.begin(), table.end(), id, [](quint32 v, const Renumbering &r) { return v < r.from; });
        if (it == table.begin() || id - (it - 1)->from >= (it - 1)->count) return ~quint32(0);
        return (it - 1)->to + (id - (it - 1)->from);
    };
    // each value is taken up before any is put down (the new ids are among the old)
    struct Held {
        quint32 row, col;
        Slot slot;
        QString text;
    };
    std::vector<Held> held;
    for (Chunk &c : m_sheets[size_t(sheet)]->chunks)
        for (const Slot &s : c.slots) {
            if (s.key == kEmptyKey) continue;
            Held h{quint32(s.key >> 16), quint32(s.key & 0xFFFF), s, QString()};
            if (s.text >= 0) h.text = m_texts[size_t(s.text)];
            held.push_back(std::move(h));
        }
    dropSheet(sheet);
    std::vector<Item> items;
    items.reserve(held.size());
    for (const Held &h : held) {
        Item it;
        it.sheet = sheet;
        it.row = h.row;
        it.col = h.col;
        quint32 &id = rows ? it.row : it.col;
        id = renumbered(id);
        if (id == ~quint32(0)) continue;
        using jp::calc::Value;
        switch (Value::Kind(h.slot.kind)) {
        case Value::Kind::Number: it.value = Value::number(h.slot.number); break;
        case Value::Kind::Bool: it.value = Value::boolean(h.slot.number != 0); break;
        case Value::Kind::Error: it.value = Value::error(jp::calc::ErrorKind(h.slot.error)); break;
        case Value::Kind::Text: it.value = Value::text(h.text); break;
        default: it.value = Value(); break;
        }
        items.push_back(std::move(it));
    }
    apply(items);
}

void ValueStore::erase(int sheet, const std::vector<std::pair<quint32, quint32>> &cells)
{
    for (const auto &cell : cells) erase(sheet, cell.first, cell.second);
}

void ValueStore::relocate(int sheet, const std::vector<Move> &moves)
{
    m_relocated += qint64(moves.size());
    // each value is taken up before any is put down (a cell may move to where another was)
    std::vector<Item> items;
    items.reserve(moves.size());
    for (const Move &m : moves) {
        Item it;
        it.sheet = sheet;
        it.row = m.toRow;
        it.col = m.toCol;
        if (find(sheet, m.fromRow, m.fromCol, &it.value)) items.push_back(std::move(it));
    }
    for (const Move &m : moves) erase(sheet, m.fromRow, m.fromCol);
    apply(items);
}

bool ValueStore::find(int sheet, quint32 row, quint32 col, jp::calc::Value *out) const
{
    using jp::calc::Value;
    if (sheet < 0 || size_t(sheet) >= m_sheets.size() || !m_sheets[size_t(sheet)] || col > 0xFFFF) return false;
    const quint64 key = keyOf(row, col);
    const quint64 h = mix(key);
    const Chunk &c = m_sheets[size_t(sheet)]->chunks[size_t(h >> (64 - kChunkBits))];
    if (c.slots.empty()) return false;
    const size_t mask = c.slots.size() - 1;
    for (size_t at = size_t(h) & mask;; at = (at + 1) & mask) {
        const Slot &s = c.slots[at];
        if (s.key == kEmptyKey) return false;
        if (s.key != key) continue;
        switch (Value::Kind(s.kind)) {
        case Value::Kind::Number: *out = Value::number(s.number); break;
        case Value::Kind::Bool: *out = Value::boolean(s.number != 0); break;
        case Value::Kind::Error: *out = Value::error(jp::calc::ErrorKind(s.error)); break;
        case Value::Kind::Text: *out = Value::text(m_texts[size_t(s.text)]); break;
        default: *out = Value(); break;
        }
        return true;
    }
}

} // namespace jp::grid
