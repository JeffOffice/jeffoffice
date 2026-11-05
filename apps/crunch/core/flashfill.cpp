#include "core/flashfill.h"

#include <QChar>

#include <algorithm>

namespace jp::crunch {

namespace {

using Piece = FlashFillPattern::Piece;

QStringList wordsOf(const QString &text)
{
    QStringList out;
    QString cur;
    for (const QChar c : text) {
        if (c.isLetterOrNumber()) {
            cur += c;
        } else if (!cur.isEmpty()) {
            out << cur;
            cur.clear();
        }
    }
    if (!cur.isEmpty()) out << cur;
    return out;
}

QString withCase(const QString &s, Piece::Case how)
{
    switch (how) {
    case Piece::Upper: return s.toUpper();
    case Piece::Lower: return s.toLower();
    case Piece::Capitalized: return s.isEmpty() ? s : s.left(1).toUpper() + s.mid(1).toLower();
    case Piece::AsIs: break;
    }
    return s;
}

// What a (non-constant) piece reads from the neighbor cells; null if the cell lacks it.
QString valueOf(const Piece &p, const QStringList &sources)
{
    if (p.source < 0 || p.source >= sources.size()) return QString();
    const QString text = sources[p.source].left(kFlashFillMaxLength);
    QString s;
    switch (p.kind) {
    case Piece::Whole: s = text; break;
    case Piece::WordFromStart:
    case Piece::WordFromEnd: {
        const QStringList w = wordsOf(text);
        const int i = p.kind == Piece::WordFromStart ? p.count : int(w.size()) - 1 - p.count;
        if (i < 0 || i >= w.size()) return QString();
        s = w[i];
        break;
    }
    case Piece::Initial: {
        const QStringList w = wordsOf(text);
        const int i = p.count >= 0 ? p.count : int(w.size()) + p.count;
        if (i < 0 || i >= w.size()) return QString();
        s = w[i].left(1);
        break;
    }
    case Piece::Left:
        if (text.size() < p.count) return QString();
        s = text.left(p.count);
        break;
    case Piece::Right:
        if (text.size() < p.count) return QString();
        s = text.right(p.count);
        break;
    case Piece::Constant: return p.text;
    }
    if (s.isEmpty()) return QString();
    return withCase(s, p.letterCase);
}

struct Search {
    const QVector<FlashFillExample> &ex;
    QVector<Piece> atoms;
    QVector<Piece> chosen;
    qint64 budget = 400000;
    bool anyConstant = false;   // a constant may hold letters and digits; if not, only spaces and marks

    // whether `piece` fits every example at its position; moves the positions on if so
    bool fits(const Piece &piece, const QVector<int> &pos, QVector<int> *next) const
    {
        next->resize(pos.size());
        for (int k = 0; k < ex.size(); ++k) {
            const QString v = valueOf(piece, ex[k].sources);
            if (v.isNull() || v.isEmpty()) return false;
            if (!QStringView(ex[k].output).mid(pos[k]).startsWith(v)) return false;
            (*next)[k] = pos[k] + int(v.size());
        }
        return true;
    }

    bool run(const QVector<int> &pos, int left, bool lastWasConstant)
    {
        bool done = true;
        for (int k = 0; k < ex.size(); ++k) done = done && pos[k] == ex[k].output.size();
        if (done) {
            // (text typed in alone says nothing about the other rows)
            for (const Piece &p : std::as_const(chosen))
                if (p.kind != Piece::Constant) return true;
            return false;
        }
        if (left == 0 || --budget < 0) return false;
        bool anyDone = false;
        for (int k = 0; k < ex.size(); ++k) anyDone = anyDone || pos[k] == ex[k].output.size();
        if (anyDone) return false;   // one example is finished and another is not
        QVector<int> next;
        for (const Piece &a : std::as_const(atoms)) {
            if (!fits(a, pos, &next)) continue;
            chosen << a;
            if (run(next, left - 1, false)) return true;
            chosen.removeLast();
            if (budget < 0) return false;
        }
        if (!lastWasConstant) {
            // a constant: the next letters of the first example, the same in all of them
            const QString &first = ex[0].output;
            for (int len = 1; pos[0] + len <= first.size() && len <= 24; ++len) {
                const QString c = first.mid(pos[0], len);
                bool all = true;
                for (int k = 0; k < ex.size() && all; ++k) all = ex[k].output.mid(pos[k], len) == c;
                if (!all) break;
                if (!anyConstant && c.back().isLetterOrNumber()) break;
                Piece p;
                p.kind = Piece::Constant;
                p.text = c;
                QVector<int> after(pos);
                for (int &v : after) v += len;
                chosen << p;
                if (run(after, left - 1, true)) return true;
                chosen.removeLast();
                if (budget < 0) return false;
            }
        }
        return false;
    }
};

QVector<Piece> atomsFor(const QVector<FlashFillExample> &examples)
{
    int sources = 0;
    for (const FlashFillExample &e : examples) sources = std::max(sources, int(e.sources.size()));
    sources = std::min(sources, kFlashFillMaxSources);
    QVector<Piece> out;
    const Piece::Case cases[] = {Piece::AsIs, Piece::Upper, Piece::Lower, Piece::Capitalized};
    for (int s = 0; s < sources; ++s) {
        auto add = [&](Piece::Kind kind, int count, bool withCases) {
            for (const Piece::Case c : cases) {
                if (!withCases && c != Piece::AsIs) break;
                Piece p;
                p.kind = kind;
                p.letterCase = c;
                p.source = s;
                p.count = count;
                out << p;
            }
        };
        add(Piece::Whole, 0, true);
        for (int i = 0; i < 4; ++i) add(Piece::WordFromStart, i, true);
        for (int i = 0; i < 3; ++i) add(Piece::WordFromEnd, i, true);
        for (int i = 0; i < 3; ++i) add(Piece::Initial, i, true);
        add(Piece::Initial, -1, true);
        for (int n = 1; n <= 8; ++n) add(Piece::Left, n, true);
        for (int n = 1; n <= 8; ++n) add(Piece::Right, n, true);
    }
    return out;
}

} // namespace

QString FlashFillPattern::apply(const QStringList &sources) const
{
    QString out;
    for (const Piece &p : m_pieces) {
        const QString v = valueOf(p, sources);
        if (v.isNull()) return QString();
        out += v;
    }
    return out;
}

FlashFillPattern learnFlashFill(const QVector<FlashFillExample> &given)
{
    FlashFillPattern none;
    QVector<FlashFillExample> examples;
    for (const FlashFillExample &e : given) {
        if (e.output.isEmpty() || e.output.size() > kFlashFillMaxLength) continue;
        if (examples.size() >= kFlashFillMaxExamples) break;
        examples << e;
    }
    if (examples.isEmpty()) return none;
    // fewest steps first; the first try reads whole words, and only if that finds nothing is a number of letters counted from an end allowed
    const QVector<Piece> all = atomsFor(examples);
    QVector<Piece> byWords;
    for (const Piece &p : all)
        if (p.kind != Piece::Left && p.kind != Piece::Right) byWords << p;
    for (const bool anyConstant : {false, true})
        for (const QVector<Piece> &atoms : {byWords, all}) {
            Search search{examples, atoms, {}, 400000, anyConstant};
            for (int pieces = 1; pieces <= kFlashFillMaxPieces; ++pieces) {
                search.chosen.clear();
                search.budget = 400000;
                if (search.run(QVector<int>(examples.size(), 0), pieces, false)) {
                    FlashFillPattern out;
                    out.m_pieces = search.chosen;
                    return out;
                }
            }
        }
    return none;
}

} // namespace jp::crunch
