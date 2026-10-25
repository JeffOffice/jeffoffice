#include "sheet/textmatch.h"

#include <algorithm>

namespace jp::office::sheet {

TextMatcher::TextMatcher(const TextPattern &pattern) : m_pattern(pattern)
{
    const QString &t = m_pattern.text;
    if (t.isEmpty() || t.size() > kMaxPatternChars) return;
    m_valid = true;
    m_pieces.append(Piece());
    for (qsizetype i = 0; i < t.size(); ++i) {
        const QChar c = t.at(i);
        if (m_pattern.wildcards && c == QLatin1Char('*')) {
            // (runs of them are one)
            if (m_tokens.isEmpty() || m_tokens.back().kind != Kind::Many) {
                m_tokens.append({Kind::Many, QChar()});
                m_pieces.append(Piece{qsizetype(m_tokens.size()), 0});
            }
            continue;
        }
        Token tok{Kind::Char, c};
        if (m_pattern.wildcards && c == QLatin1Char('?')) {
            tok.kind = Kind::One;
        } else if (m_pattern.wildcards && c == QLatin1Char('~') && i + 1 < t.size()) {
            const QChar next = t.at(i + 1);
            if (next == QLatin1Char('?') || next == QLatin1Char('*') || next == QLatin1Char('~')) {
                tok.ch = next;
                ++i;
            }
        }
        m_tokens.append(tok);
        ++m_pieces.back().size;
    }
}

bool TextMatcher::sameChar(QChar a, QChar b) const
{
    if (a == b) return true;
    return !m_pattern.matchCase && a.toCaseFolded() == b.toCaseFolded();
}

bool TextMatcher::pieceAt(const Piece &p, QStringView text, qsizetype at, qint64 *steps) const
{
    if (at < 0 || at + p.size > text.size()) return false;
    for (qsizetype i = 0; i < p.size; ++i) {
        ++*steps;
        const Token &tok = m_tokens[int(p.first + i)];
        if (tok.kind == Kind::Char && !sameChar(tok.ch, text.at(at + i))) return false;
    }
    return true;
}

// The first place at or after `from` where the piece fits and ends at or before `limit`; -1 for none (or past the steps).
qsizetype TextMatcher::findPiece(const Piece &p, QStringView text, qsizetype from, qsizetype limit, qint64 *steps) const
{
    for (qsizetype at = from; at + p.size <= limit; ++at) {
        if (*steps > kMaxMatchSteps) return -1;
        if (pieceAt(p, text, at, steps)) return at;
        ++*steps;
    }
    return -1;
}

// The last place where the piece fits, begins at or after `from`, and ends at or before `limit`.
qsizetype TextMatcher::findPieceLast(const Piece &p, QStringView text, qsizetype from, qsizetype limit, qint64 *steps) const
{
    for (qsizetype at = limit - p.size; at >= from; --at) {
        if (*steps > kMaxMatchSteps) return -1;
        if (pieceAt(p, text, at, steps)) return at;
        ++*steps;
    }
    return -1;
}

bool TextMatcher::find(QStringView text, qsizetype from, qsizetype *start, qsizetype *length, qint64 *stepsOut) const
{
    if (!m_valid || text.isEmpty() || from < 0) return false;
    if (text.size() > kMaxMatchChars) text = text.left(kMaxMatchChars);
    const qsizetype n = text.size();
    qint64 steps = 0;
    bool found = false;
    qsizetype s = 0, len = 0;
    const qsizetype k = m_pieces.size() - 1;   // the runs of `*`
    const Piece &first = m_pieces.front(), &last = m_pieces.back();
    if (m_pattern.wholeCell) {
        if (from == 0) {
            if (k == 0) {
                found = first.size == n && pieceAt(first, text, 0, &steps);
            } else if (first.size + last.size <= n && pieceAt(first, text, 0, &steps) && pieceAt(last, text, n - last.size, &steps)) {
                qsizetype pos = first.size;
                found = true;
                for (qsizetype i = 1; i < k && found; ++i) {
                    const qsizetype at = findPiece(m_pieces[int(i)], text, pos, n - last.size, &steps);
                    if (at < 0) found = false;
                    else pos = at + m_pieces[int(i)].size;
                }
            }
            if (found) {
                s = 0;
                len = n;
            }
        }
    } else if (k == 0) {
        const qsizetype at = findPiece(first, text, from, n, &steps);
        if (at >= 0) {
            found = true;
            s = at;
            len = first.size;
        }
    } else {
        qsizetype pos = from;
        bool ok = true;
        if (first.size > 0) {
            const qsizetype at = findPiece(first, text, from, n, &steps);
            if (at < 0) ok = false;
            else {
                s = at;
                pos = at + first.size;
            }
        } else {
            s = from;
        }
        for (qsizetype i = 1; i < k && ok; ++i) {
            const qsizetype at = findPiece(m_pieces[int(i)], text, pos, n, &steps);
            if (at < 0) ok = false;
            else pos = at + m_pieces[int(i)].size;
        }
        qsizetype end = n;
        if (ok && last.size > 0) {
            const qsizetype at = findPieceLast(last, text, pos, n, &steps);
            if (at < 0) ok = false;
            else end = at + last.size;
        }
        if (ok && s <= n && end >= s) {
            found = true;
            len = end - s;
        }
    }
    if (stepsOut) *stepsOut += steps;
    if (!found || steps > kMaxMatchSteps) return false;
    *start = s;
    *length = len;
    return true;
}

bool TextMatcher::matches(QStringView text, qint64 *steps) const
{
    qsizetype s = 0, len = 0;
    return find(text, 0, &s, &len, steps);
}

QString TextMatcher::replaced(const QString &text, const QString &with, int *count) const
{
    if (count) *count = 0;
    if (!m_valid || text.isEmpty()) return text;
    QString out;
    qsizetype from = 0, copied = 0;
    int made = 0;
    const QStringView view(text);
    const qsizetype limit = std::min(text.size(), kMaxMatchChars);
    while (from <= limit) {
        qsizetype s = 0, len = 0;
        if (!find(view.left(limit), from, &s, &len)) break;
        if (len == 0) break;
        out += view.mid(copied, s - copied);
        out += with;
        copied = s + len;
        from = copied;
        ++made;
        if (m_pattern.wholeCell) break;
    }
    if (made == 0) return text;
    out += view.mid(copied);
    if (count) *count = made;
    return out;
}

} // namespace jp::office::sheet
