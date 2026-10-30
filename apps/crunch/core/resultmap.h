#pragma once
// The results the edit session keeps by cell for the saver (docs/jeffcrunch-rowids.md, round 7): a hash table with linear probing, in three parallel arrays (the key, 8 bytes; a number or the place of a
// text, 8; the kind and the error, 1), 17 bytes a slot, at most 3/4 full and rebuilt to half full, so between 23 and 45 bytes an entry, not a tree of nodes holding whole values (about 106 bytes an entry).
// It has the part of std::map's interface the session uses; an array is kept as its first element, which is what a file holds for it. A text lives in a list beside the table, and the place of a
// text that is overwritten or taken out is used again. No GUI.

#include "calc/value.h"

#include <QString>
#include <QtGlobal>

#include <cstddef>
#include <cstring>
#include <iterator>
#include <utility>
#include <vector>

namespace jp::crunch {

class ResultMap {
public:
    using Pair = std::pair<quint64, jp::calc::Value>;
    // Keys that are not kept: the two values the table uses for a slot that is free or has been (a key has the sheet in its top bits, and is never one of them).
    static constexpr quint64 kEmptyKey = ~quint64(0), kDeadKey = ~quint64(0) - 1;

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
    // The places in the list of texts (used or free), and the bytes the table holds: for a test that sees that texts that come and go do not pile up.
    size_t textPlaces() const { return m_texts.size(); }
    size_t slotCount() const { return m_keys.size(); }
    void clear()
    {
        std::vector<quint64>().swap(m_keys);
        std::vector<quint64>().swap(m_bits);
        std::vector<quint8>().swap(m_meta);
        std::vector<QString>().swap(m_texts);
        std::vector<quint32>().swap(m_freeTexts);
        m_used = m_taken = 0;
    }
    void swap(ResultMap &o)
    {
        m_keys.swap(o.m_keys);
        m_bits.swap(o.m_bits);
        m_meta.swap(o.m_meta);
        m_texts.swap(o.m_texts);
        m_freeTexts.swap(o.m_freeTexts);
        std::swap(m_used, o.m_used);
        std::swap(m_taken, o.m_taken);
    }
    iterator begin() const
    {
        size_t at = 0;
        skip(&at);
        return iterator(this, at);
    }
    iterator end() const { return iterator(this, m_keys.size()); }
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
        return at == kNone ? jp::calc::Value() : valueAt(at);
    }
    // Keeps the value for the key. The two keys the table uses for itself are refused (false).
    bool set(quint64 key, const jp::calc::Value &v)
    {
        if (!isKey(key)) return false;
        size_t at = slotOf(key);
        if (at == kNone) {
            if ((m_taken + 1) * 4 > m_keys.size() * 3) grow();
            at = place(key);
            ++m_used;
        } else {
            freeText(at);
        }
        fill(at, v);
        return true;
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
    static constexpr size_t kNone = ~size_t(0);
    static constexpr quint8 kText = 2;   // (jp::calc::Value::Kind::Text)
    static bool isKey(quint64 key) { return key < kDeadKey; }

    static size_t hashOf(quint64 k)
    {
        k ^= k >> 33;
        k *= 0xff51afd7ed558ccdull;
        k ^= k >> 33;
        k *= 0xc4ceb9fe1a85ec53ull;
        k ^= k >> 33;
        return size_t(k);
    }
    void skip(size_t *at) const
    {
        while (*at < m_keys.size() && !isKey(m_keys[*at])) ++*at;
    }
    size_t slotOf(quint64 key) const
    {
        if (m_keys.empty() || !isKey(key)) return kNone;
        const size_t mask = m_keys.size() - 1;
        for (size_t i = hashOf(key) & mask;; i = (i + 1) & mask) {
            if (m_keys[i] == key) return i;
            if (m_keys[i] == kEmptyKey) return kNone;
        }
    }
    size_t place(quint64 key)
    {
        const size_t mask = m_keys.size() - 1;
        size_t i = hashOf(key) & mask;
        while (isKey(m_keys[i])) i = (i + 1) & mask;   // (a free or dead slot)
        if (m_keys[i] == kEmptyKey) ++m_taken;
        m_keys[i] = key;
        return i;
    }
    void kill(size_t at)
    {
        freeText(at);
        m_keys[at] = kDeadKey;
        --m_used;
    }
    void freeText(size_t at)
    {
        if ((m_meta[at] & 7) != kText) return;
        const quint32 place = quint32(m_bits[at]);
        m_texts[place] = QString();
        m_freeTexts.push_back(place);
        m_meta[at] = 0;
    }
    void grow()
    {
        std::vector<quint64> keys, bits;
        std::vector<quint8> meta;
        keys.swap(m_keys);
        bits.swap(m_bits);
        meta.swap(m_meta);
        std::vector<QString> texts;
        texts.swap(m_texts);
        m_freeTexts.clear();
        size_t cap = 16;
        while (cap < (m_used + 1) * 2) cap *= 2;   // (half full or less when rebuilt; tombstones are let go of)
        m_keys.assign(cap, kEmptyKey);
        m_bits.assign(cap, 0);
        m_meta.assign(cap, 0);
        m_taken = 0;
        for (size_t i = 0; i < keys.size(); ++i) {
            if (!isKey(keys[i])) continue;
            const size_t n = place(keys[i]);
            m_meta[n] = meta[i];
            if ((meta[i] & 7) == kText) {
                m_bits[n] = quint32(m_texts.size());
                m_texts.push_back(std::move(texts[quint32(bits[i])]));
            } else {
                m_bits[n] = bits[i];
            }
        }
    }
    void fill(size_t at, const jp::calc::Value &v0)
    {
        using K = jp::calc::Value::Kind;
        const jp::calc::Value v = v0.isArray() ? v0.at(0, 0) : v0;
        m_meta[at] = quint8(quint8(v.kind()) | (quint8(v.errorKind()) << 3));
        if (v.kind() == K::Text) {
            quint32 place;
            if (!m_freeTexts.empty()) {
                place = m_freeTexts.back();
                m_freeTexts.pop_back();
                m_texts[place] = v.text();
            } else {
                place = quint32(m_texts.size());
                m_texts.push_back(v.text());
            }
            m_bits[at] = place;
        } else {
            const double d = v.number();
            quint64 b;
            std::memcpy(&b, &d, sizeof b);
            m_bits[at] = b;
        }
    }
    jp::calc::Value valueAt(size_t at) const
    {
        using K = jp::calc::Value::Kind;
        double d = 0;
        if ((m_meta[at] & 7) != kText) std::memcpy(&d, &m_bits[at], sizeof d);
        switch (K(m_meta[at] & 7)) {
        case K::Number: return jp::calc::Value::number(d);
        case K::Bool: return jp::calc::Value::boolean(d != 0);
        case K::Text: return jp::calc::Value::text(m_texts[quint32(m_bits[at])]);
        case K::Error: return jp::calc::Value::error(jp::calc::ErrorKind(m_meta[at] >> 3));
        default: return jp::calc::Value();
        }
    }
    Pair entry(size_t at) const { return {m_keys[at], valueAt(at)}; }

    std::vector<quint64> m_keys, m_bits;
    std::vector<quint8> m_meta;       // the kind (3 bits) and the error (4 bits)
    std::vector<QString> m_texts;
    std::vector<quint32> m_freeTexts;
    size_t m_used = 0, m_taken = 0;   // entries, and slots that are not empty (entries and tombstones)
};

} // namespace jp::crunch
