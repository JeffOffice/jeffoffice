#pragma once
// The results the edit session keeps by cell for the saver (docs/jeffcrunch-rowids.md, round 7): a hash table of 24-byte slots (the key, a number or the place of a text, and the kind), not a tree of
// nodes holding whole values (about 106 bytes an entry: 2 million entries after one column delete at a million rows were 213 MB). It has the part of std::map's interface the session uses; an array is
// kept as its first element, which is what a file holds for it. No GUI.

#include "calc/value.h"

#include <QString>

#include <cstddef>
#include <iterator>
#include <utility>
#include <vector>

namespace jp::crunch {

class ResultMap {
public:
    using Pair = std::pair<quint64, jp::calc::Value>;

    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Pair;
        using difference_type = std::ptrdiff_t;
        using pointer = const Pair *;
        using reference = Pair;
        struct Arrow {
            Pair pair;
            const Pair *operator->() const { return &pair; }
        };
        iterator() = default;
        Pair operator*() const { return m_map->entry(m_at); }
        Arrow operator->() const { return Arrow{m_map->entry(m_at)}; }
        iterator &operator++()
        {
            ++m_at;
            m_map->skip(&m_at);
            return *this;
        }
        iterator operator++(int)
        {
            iterator was = *this;
            ++*this;
            return was;
        }
        friend bool operator==(const iterator &a, const iterator &b) { return a.m_at == b.m_at; }
        friend bool operator!=(const iterator &a, const iterator &b) { return a.m_at != b.m_at; }

    private:
        friend class ResultMap;
        iterator(const ResultMap *map, size_t at) : m_map(map), m_at(at) {}
        const ResultMap *m_map = nullptr;
        size_t m_at = 0;
    };

    size_t size() const { return m_used; }
    bool empty() const { return m_used == 0; }
    void clear()
    {
        std::vector<Slot>().swap(m_slots);
        std::vector<QString>().swap(m_texts);
        m_used = m_taken = 0;
    }
    void swap(ResultMap &o)
    {
        m_slots.swap(o.m_slots);
        m_texts.swap(o.m_texts);
        std::swap(m_used, o.m_used);
        std::swap(m_taken, o.m_taken);
    }
    iterator begin() const
    {
        size_t at = 0;
        skip(&at);
        return iterator(this, at);
    }
    iterator end() const { return iterator(this, m_slots.size()); }
    iterator find(quint64 key) const
    {
        const size_t at = slotOf(key);
        return at == kNone ? end() : iterator(this, at);
    }
    size_t count(quint64 key) const { return slotOf(key) == kNone ? 0 : 1; }
    // The value of the key (an empty value if there is none).
    jp::calc::Value value(quint64 key) const
    {
        const size_t at = slotOf(key);
        return at == kNone ? jp::calc::Value() : valueOf(m_slots[at]);
    }
    void set(quint64 key, const jp::calc::Value &v)
    {
        size_t at = slotOf(key);
        if (at == kNone) {
            if ((m_taken + 1) * 4 > m_slots.size() * 3) grow();
            at = place(key);
            ++m_used;
        }
        fill(&m_slots[at], v);
    }
    size_t erase(quint64 key)
    {
        const size_t at = slotOf(key);
        if (at == kNone) return 0;
        kill(at);
        return 1;
    }
    // Takes the entry out; the iterator is to the next.
    iterator erase(iterator it)
    {
        kill(it.m_at);
        ++it;
        return it;
    }

private:
    static constexpr quint64 kEmpty = ~quint64(0), kDead = ~quint64(0) - 1;   // (no key is either: a key has the sheet in its top bits)
    static constexpr size_t kNone = ~size_t(0);
    struct Slot {
        quint64 key = kEmpty;
        union {
            double num;
            quint32 text;
        };
        quint8 kind = 0, error = 0;
        Slot() : num(0) {}
    };
    static_assert(sizeof(Slot) == 24, "a slot is 24 bytes");

    static size_t hashOf(quint64 k)
    {
        k ^= k >> 33;
        k *= 0xff51afd7ed558ccdull;
        k ^= k >> 33;
        return size_t(k);
    }
    void skip(size_t *at) const
    {
        while (*at < m_slots.size() && m_slots[*at].key >= kDead) ++*at;
    }
    size_t slotOf(quint64 key) const
    {
        if (m_slots.empty()) return kNone;
        const size_t mask = m_slots.size() - 1;
        for (size_t i = hashOf(key) & mask;; i = (i + 1) & mask) {
            if (m_slots[i].key == key) return i;
            if (m_slots[i].key == kEmpty) return kNone;
        }
    }
    size_t place(quint64 key)
    {
        const size_t mask = m_slots.size() - 1;
        size_t i = hashOf(key) & mask;
        while (m_slots[i].key < kDead) i = (i + 1) & mask;   // (a free or dead slot)
        if (m_slots[i].key == kEmpty) ++m_taken;
        m_slots[i].key = key;
        return i;
    }
    void kill(size_t at)
    {
        m_slots[at].key = kDead;
        --m_used;
    }
    void grow()
    {
        std::vector<Slot> old;
        old.swap(m_slots);
        std::vector<QString> oldTexts;
        oldTexts.swap(m_texts);
        size_t cap = 16;
        while (cap * 3 < (m_used + 1) * 4 * 2) cap *= 2;   // (at most 3/8 full when rebuilt: tombstones are let go of)
        m_slots.assign(cap, Slot());
        m_taken = 0;
        for (const Slot &s : old) {
            if (s.key >= kDead) continue;
            Slot &n = m_slots[place(s.key)];
            n.kind = s.kind;
            n.error = s.error;
            if (s.kind == kText) {
                n.text = quint32(m_texts.size());
                m_texts.push_back(oldTexts[s.text]);
            } else {
                n.num = s.num;
            }
        }
    }
    static constexpr quint8 kText = 2;
    void fill(Slot *s, const jp::calc::Value &v0)
    {
        using K = jp::calc::Value::Kind;
        const jp::calc::Value v = v0.isArray() ? v0.at(0, 0) : v0;
        s->kind = quint8(v.kind());
        s->error = quint8(v.errorKind());
        if (v.kind() == K::Text) {
            s->text = quint32(m_texts.size());
            m_texts.push_back(v.text());
        } else {
            s->num = v.number();
        }
    }
    jp::calc::Value valueOf(const Slot &s) const
    {
        using K = jp::calc::Value::Kind;
        switch (K(s.kind)) {
        case K::Number: return jp::calc::Value::number(s.num);
        case K::Bool: return jp::calc::Value::boolean(s.num != 0);
        case K::Text: return jp::calc::Value::text(m_texts[s.text]);
        case K::Error: return jp::calc::Value::error(jp::calc::ErrorKind(s.error));
        default: return jp::calc::Value();
        }
    }
    Pair entry(size_t at) const { return {m_slots[at].key, valueOf(m_slots[at])}; }

    std::vector<Slot> m_slots;
    std::vector<QString> m_texts;
    size_t m_used = 0, m_taken = 0;   // entries, and slots that are not empty (entries and tombstones)
};

} // namespace jp::crunch
