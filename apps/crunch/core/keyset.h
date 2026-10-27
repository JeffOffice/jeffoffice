#pragma once
// A set of 64-bit keys (the marks of cells the session keeps: by sheet, row id, and column id), held flat: one table of keys with linear probing, so that putting in a hundred thousand
// at once (the formulas an edit of rows wrote again) is a few passes over memory and no allocation for each. The key that is all ones is not held (the keys of cells are far from it).

#include <QtGlobal>

#include <algorithm>
#include <vector>

namespace jp::crunch {

class KeySet {
public:
    static constexpr quint64 kEmpty = ~quint64(0);

    size_t size() const { return m_size; }
    bool empty() const { return m_size == 0; }
    void clear()
    {
        std::vector<quint64>().swap(m_slots);
        m_size = 0;
    }
    void swap(KeySet &o)
    {
        m_slots.swap(o.m_slots);
        std::swap(m_size, o.m_size);
    }
    // Makes room for `n` keys in all.
    void reserve(size_t n)
    {
        size_t cap = std::max<size_t>(16, m_slots.size());
        while (n * 5 > cap * 3) cap *= 2;
        if (cap != m_slots.size()) rehash(cap);
    }
    // True if the key was not there.
    bool insert(quint64 key)
    {
        if (key == kEmpty) return false;
        reserve(m_size + 1);
        const size_t mask = m_slots.size() - 1;
        size_t at = size_t(mix(key)) & mask;
        while (m_slots[at] != kEmpty) {
            if (m_slots[at] == key) return false;
            at = (at + 1) & mask;
        }
        m_slots[at] = key;
        ++m_size;
        return true;
    }
    size_t count(quint64 key) const { return contains(key) ? 1 : 0; }
    bool contains(quint64 key) const
    {
        if (m_slots.empty() || key == kEmpty) return false;
        const size_t mask = m_slots.size() - 1;
        for (size_t at = size_t(mix(key)) & mask; m_slots[at] != kEmpty; at = (at + 1) & mask)
            if (m_slots[at] == key) return true;
        return false;
    }
    // True if the key was there.
    bool erase(quint64 key)
    {
        if (m_slots.empty() || key == kEmpty) return false;
        const size_t mask = m_slots.size() - 1;
        size_t at = size_t(mix(key)) & mask;
        while (m_slots[at] != key) {
            if (m_slots[at] == kEmpty) return false;
            at = (at + 1) & mask;
        }
        // (the keys after it that were put there because it was in the way are moved up: no mark is left in the table)
        size_t hole = at;
        for (size_t j = (at + 1) & mask; m_slots[j] != kEmpty; j = (j + 1) & mask) {
            const size_t ideal = size_t(mix(m_slots[j])) & mask;
            const bool between = hole <= j ? (ideal > hole && ideal <= j) : (ideal > hole || ideal <= j);
            if (between) continue;
            m_slots[hole] = m_slots[j];
            hole = j;
        }
        m_slots[hole] = kEmpty;
        --m_size;
        return true;
    }
    // The keys, in no order (a copy: the set may change as they are looked at).
    std::vector<quint64> keys() const
    {
        std::vector<quint64> out;
        out.reserve(m_size);
        for (const quint64 k : m_slots)
            if (k != kEmpty) out.push_back(k);
        return out;
    }
    template <class F>
    void forEach(F f) const
    {
        for (const quint64 k : m_slots)
            if (k != kEmpty) f(k);
    }

private:
    static quint64 mix(quint64 x)
    {
        x ^= x >> 33;
        x *= 0xFF51AFD7ED558CCDull;
        x ^= x >> 33;
        x *= 0xC4CEB9FE1A85EC53ull;
        x ^= x >> 33;
        return x;
    }
    void rehash(size_t cap)
    {
        std::vector<quint64> now(cap, kEmpty);
        const size_t mask = cap - 1;
        for (const quint64 k : m_slots) {
            if (k == kEmpty) continue;
            size_t at = size_t(mix(k)) & mask;
            while (now[at] != kEmpty) at = (at + 1) & mask;
            now[at] = k;
        }
        m_slots.swap(now);
    }
    std::vector<quint64> m_slots;
    size_t m_size = 0;
};

} // namespace jp::crunch
