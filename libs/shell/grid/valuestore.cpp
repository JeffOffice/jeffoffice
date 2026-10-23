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
        if (c.slots.empty()) c.slots.assign(64, Slot());
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
