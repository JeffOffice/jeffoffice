#include "runmap.h"

#include <QFont>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextFragment>

#include <algorithm>

namespace jp {
namespace translate {

namespace {

bool isFixedChar(QChar c) { return c == QChar::ObjectReplacementCharacter || c == QChar::LineSeparator || c == QLatin1Char('\t'); }

struct Flags {
    bool bold = false, italic = false, underline = false;
    bool operator==(const Flags &o) const { return bold == o.bold && italic == o.italic && underline == o.underline; }
};

Flags flagsOf(const QTextCharFormat &f)
{
    Flags g;
    g.bold = f.fontWeight() >= QFont::DemiBold || f.font().bold();
    g.italic = f.fontItalic();
    g.underline = f.fontUnderline();
    return g;
}

QTextCharFormat withFlags(QTextCharFormat f, const Flags &g)
{
    f.setFontWeight(g.bold ? QFont::Bold : QFont::Normal);
    f.setFontItalic(g.italic);
    f.setFontUnderline(g.underline);
    return f;
}

QString escaped(const QString &s)
{
    QString out;
    out.reserve(s.size());
    for (const QChar c : s) {
        if (c == QLatin1Char('&')) out += QLatin1String("&amp;");
        else if (c == QLatin1Char('<')) out += QLatin1String("&lt;");
        else if (c == QLatin1Char('>')) out += QLatin1String("&gt;");
        else out += c;
    }
    return out;
}

} // namespace

bool Source::isEmpty() const
{
    for (const Paragraph &p : paragraphs)
        for (const Run &r : p.runs)
            if (!r.fixed)
                for (const QChar c : r.text)
                    if (c.isLetterOrNumber()) return false;
    return true;
}

QString Source::plainText() const
{
    QStringList lines;
    for (const Paragraph &p : paragraphs) {
        QString t;
        for (const Run &r : p.runs) {
            if (r.skipped) continue;
            QString part = r.text;
            part.remove(QChar::ObjectReplacementCharacter).replace(QChar::LineSeparator, QLatin1Char('\n'));
            t += part;
        }
        lines << t;
    }
    return lines.join(QLatin1Char('\n'));
}

namespace {

// Where each block and each stretch of one look begins in [a, b], and the look.
QVector<QPair<int, QTextFormat>> marksOf(QTextDocument *doc, int a, int b)
{
    QVector<QPair<int, QTextFormat>> marks;
    for (QTextBlock block = doc->findBlock(a); block.isValid() && block.position() <= b; block = block.next()) {
        marks.push_back({block.position(), block.blockFormat()});
        for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (frag.isValid()) marks.push_back({frag.position(), frag.charFormat()});
        }
    }
    return marks;
}

} // namespace

bool Source::isCurrent() const
{
    if (!document) return false;
    if (first < 0 || last > document->characterCount() - 1 || first > last) return false;
    QTextCursor c(document);
    c.setPosition(first);
    c.setPosition(last, QTextCursor::KeepAnchor);
    if (c.selectedText() != snapshot) return false;
    return marksOf(document, first, last) == marks;
}

Source readSource(const QTextCursor &cursor, const SkipFn &skip)
{
    Source s;
    if (cursor.isNull()) return s;   // (a cursor whose document has gone has none to ask)
    QTextDocument *doc = cursor.document();
    if (!doc) return s;
    s.document = doc;
    int a = cursor.hasSelection() ? cursor.selectionStart() : cursor.block().position();
    int b = cursor.hasSelection() ? cursor.selectionEnd() : cursor.block().position() + cursor.block().length() - 1;
    s.first = a;
    s.last = b;
    if (b - a > kMostSourceChars) {
        s.tooLong = true;
        return s;
    }
    QTextCursor all(doc);
    all.setPosition(a);
    all.setPosition(b, QTextCursor::KeepAnchor);
    s.snapshot = all.selectedText();
    s.marks = marksOf(doc, a, b);
    for (QTextBlock block = doc->findBlock(a); block.isValid() && block.position() <= b; block = block.next()) {
        Paragraph p;
        p.blockFormat = block.blockFormat();
        p.start = std::max(a, block.position());
        p.end = std::min(b, block.position() + block.length() - 1);
        if (p.end < p.start) p.end = p.start;
        for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid()) continue;
            const int from = std::max(p.start, frag.position());
            const int to = std::min(p.end, frag.position() + frag.length());
            if (to <= from) continue;
            const QString text = frag.text().mid(from - frag.position(), to - from);
            if (skip && skip(frag.charFormat())) {
                Run r;
                r.text = text;
                r.format = frag.charFormat();
                r.fixed = true;
                r.skipped = true;
                r.start = from;
                p.runs.push_back(r);
                continue;
            }
            // Cut at the characters that are kept as they are.
            int i = 0;
            while (i < text.size()) {
                int j = i;
                const bool fixed = isFixedChar(text.at(i));
                while (j < text.size() && isFixedChar(text.at(j)) == fixed) ++j;
                Run r;
                r.text = text.mid(i, j - i);
                r.format = frag.charFormat();
                r.fixed = fixed;
                r.start = from + i;
                p.runs.push_back(r);
                i = j;
            }
        }
        s.paragraphs.push_back(p);
    }
    return s;
}

QVector<Piece> pieces(const Source &source)
{
    QVector<Piece> out;
    for (int pi = 0; pi < source.paragraphs.size(); ++pi) {
        const QVector<Run> &runs = source.paragraphs[pi].runs;
        int i = 0;
        while (i < runs.size()) {
            if (runs[i].fixed) {
                ++i;
                continue;
            }
            int j = i;
            bool letters = false;
            while (j < runs.size() && !runs[j].fixed) {
                for (const QChar c : runs[j].text)
                    if (c.isLetterOrNumber()) letters = true;
                ++j;
            }
            if (letters) out.push_back({pi, i, j - 1});
            i = j;
        }
    }
    return out;
}

namespace {

// The text of the runs, trimmed, with the white space at its ends and what the first and last run of it are.
void trimmedBounds(const QVector<Run> &runs, int first, int last, int *lead, int *trail)
{
    // The count of white space characters at the start and at the end of the joined text.
    QString all;
    for (int i = first; i <= last; ++i) all += runs[i].text;
    int l = 0, t = 0;
    while (l < all.size() && all.at(l).isSpace()) ++l;
    while (t < all.size() - l && all.at(all.size() - 1 - t).isSpace()) ++t;
    *lead = l;
    *trail = t;
}

} // namespace

QString pieceHtml(const Source &source, const Piece &piece)
{
    const QVector<Run> &runs = source.paragraphs[piece.paragraph].runs;
    int lead = 0, trail = 0;
    trimmedBounds(runs, piece.first, piece.last, &lead, &trail);
    int total = 0;
    for (int i = piece.first; i <= piece.last; ++i) total += runs[i].text.size();
    QString html;
    Flags open;
    int at = 0;
    auto setFlags = [&](const Flags &want) {
        // Close in the reverse order of opening, then open: a simple scheme that never crosses tags.
        if (open.underline && !want.underline) html += QLatin1String("</u>");
        if (open.italic && !want.italic) html += QLatin1String("</i>");
        if (open.bold && !want.bold) html += QLatin1String("</b>");
        if (!open.bold && want.bold) html += QLatin1String("<b>");
        if (!open.italic && want.italic) html += QLatin1String("<i>");
        if (!open.underline && want.underline) html += QLatin1String("<u>");
        open = want;
    };
    // (a tag that closes and opens again between runs would be wrong for bold open over italic: close all, then open all)
    auto change = [&](const Flags &want) {
        if (want == open) return;
        const Flags none;
        setFlags(none);
        setFlags(want);
    };
    for (int i = piece.first; i <= piece.last; ++i) {
        const QString &t = runs[i].text;
        const int from = std::max(lead, at), to = std::min(total - trail, int(at + t.size()));
        at += t.size();
        if (to <= from) continue;
        change(flagsOf(runs[i].format));
        html += escaped(t.mid(from - (at - t.size()), to - from));
    }
    setFlags(Flags());
    return html;
}

namespace {

QString decodeEntity(const QString &name)
{
    if (name == QLatin1String("amp")) return QStringLiteral("&");
    if (name == QLatin1String("lt")) return QStringLiteral("<");
    if (name == QLatin1String("gt")) return QStringLiteral(">");
    if (name == QLatin1String("quot")) return QStringLiteral("\"");
    if (name == QLatin1String("apos")) return QStringLiteral("'");
    if (name == QLatin1String("nbsp")) return QString(QChar(0x00A0));
    if (name.startsWith(QLatin1Char('#'))) {
        bool ok = false;
        const uint v = name.startsWith(QLatin1String("#x")) || name.startsWith(QLatin1String("#X")) ? name.mid(2).toUInt(&ok, 16) : name.mid(1).toUInt(&ok, 10);
        // (a number that is half of a pair of surrogates, or none of Unicode, is not a character: the entity stays as written)
        if (ok && v > 0 && v <= 0x10FFFF && !(v >= 0xD800 && v <= 0xDFFF)) return QString::fromUcs4(reinterpret_cast<const char32_t *>(&v), 1);
    }
    return QString();
}

} // namespace

QVector<Run> runsFromHtml(const QString &html, const QVector<Run> &original)
{
    QVector<Run> out;
    int bold = 0, italic = 0, under = 0;
    QString text;
    // The format for given flags: the first run of the original that has them, else the first run's own with the flags set.
    QTextCharFormat base;
    for (const Run &r : original)
        if (!r.fixed) {
            base = r.format;
            break;
        }
    auto formatFor = [&](const Flags &g) {
        for (const Run &r : original)
            if (!r.fixed && flagsOf(r.format) == g) return r.format;
        return withFlags(base, g);
    };
    auto flush = [&]() {
        if (text.isEmpty()) return;
        Flags g;
        g.bold = bold > 0;
        g.italic = italic > 0;
        g.underline = under > 0;
        const QTextCharFormat f = formatFor(g);
        if (!out.isEmpty() && out.last().format == f) out.last().text += text;
        else {
            Run r;
            r.text = text;
            r.format = f;
            out.push_back(r);
        }
        text.clear();
    };
    int i = 0;
    while (i < html.size()) {
        const QChar c = html.at(i);
        if (c == QLatin1Char('<')) {
            const int close = html.indexOf(QLatin1Char('>'), i);
            if (close < 0) {   // (a bare < that is not a tag)
                text += c;
                ++i;
                continue;
            }
            QString tag = html.mid(i + 1, close - i - 1).trimmed().toLower();
            const bool closing = tag.startsWith(QLatin1Char('/'));
            if (closing) tag = tag.mid(1).trimmed();
            const QString name = tag.section(QRegularExpression(QStringLiteral("[\\s/]")), 0, 0);
            const int delta = closing ? -1 : 1;
            if (name == QLatin1String("b") || name == QLatin1String("strong") || name == QLatin1String("i") || name == QLatin1String("em") || name == QLatin1String("u")) {
                flush();
                int &counter = (name == QLatin1String("b") || name == QLatin1String("strong")) ? bold : (name == QLatin1String("u") ? under : italic);
                counter = std::max(0, counter + delta);
            } else if (name == QLatin1String("br") && !closing) {
                text += QLatin1Char(' ');
            }
            i = close + 1;
            continue;
        }
        if (c == QLatin1Char('&')) {
            const int semi = html.indexOf(QLatin1Char(';'), i);
            if (semi > i && semi - i <= 10) {
                const QString decoded = decodeEntity(html.mid(i + 1, semi - i - 1));
                if (!decoded.isEmpty()) {
                    text += decoded;
                    i = semi + 1;
                    continue;
                }
            }
        }
        text += c;
        ++i;
    }
    flush();
    return out;
}


QVector<Chunk> splitForTranslation(const QString &html, int most)
{
    QVector<Chunk> out;
    if (html.size() <= most) {
        out.push_back({html, QString()});
        return out;
    }
    struct Cut {
        int at = -1;           // the place in `html` the chunk ends at
        QStringList stack;     // the tags open there
    };
    const int n = html.size();
    int start = 0;
    QStringList stack;   // the tags open at `start`
    while (start < n) {
        QStringList now = stack;
        Cut sentence, space;
        int i = start, size = 0;
        bool full = false;
        while (i < n) {
            // One token: a tag, an entity, or a letter (with its low surrogate).
            int j = i + 1;
            const QChar c = html.at(i);
            if (c == QLatin1Char('<')) {
                const int close = html.indexOf(QLatin1Char('>'), i);
                if (close > i && close - i < 40) {
                    j = close + 1;
                    QString tag = html.mid(i + 1, close - i - 1).trimmed().toLower();
                    const bool closing = tag.startsWith(QLatin1Char('/'));
                    if (closing) tag = tag.mid(1);
                    if (closing) {
                        if (!now.isEmpty() && now.last() == tag) now.removeLast();
                    } else if (!tag.endsWith(QLatin1Char('/'))) {
                        now.push_back(tag);
                    }
                }
            } else if (c == QLatin1Char('&')) {
                const int semi = html.indexOf(QLatin1Char(';'), i);
                if (semi > i && semi - i <= 10) j = semi + 1;
            } else if (c.isHighSurrogate() && j < n && html.at(j).isLowSurrogate()) {
                ++j;
            }
            if (size + (j - i) > most && size > 0) {
                full = true;
                break;
            }
            size += j - i;
            const bool wasSpace = c.isSpace();
            i = j;
            // A place to cut: after the white space that follows the end of a sentence, or after any white space.
            if (wasSpace) {
                const QChar before = i >= 2 ? html.at(i - 2) : QChar();
                const bool end = before == QLatin1Char('.') || before == QLatin1Char('!') || before == QLatin1Char('?') || before == QChar(0x3002) || before == QChar(0xFF01) || before == QChar(0xFF1F)
                    || before == QLatin1Char('>') || c == QLatin1Char('\n');
                // (a tag before the space may close a sentence's last words: look behind it for the stop)
                bool stop = end && before != QLatin1Char('>');
                if (before == QLatin1Char('>')) {
                    int k = i - 2;
                    while (k >= start && html.at(k) == QLatin1Char('>')) {
                        const int open = html.lastIndexOf(QLatin1Char('<'), k);
                        if (open < start) break;
                        k = open - 1;
                    }
                    const QChar real = k >= start ? html.at(k) : QChar();
                    stop = real == QLatin1Char('.') || real == QLatin1Char('!') || real == QLatin1Char('?');
                }
                if (c == QLatin1Char('\n')) stop = true;
                if (stop) sentence = {i, now};
                space = {i, now};
            }
        }
        if (!full) {
            QString opening;
            for (const QString &t : std::as_const(stack)) opening += QLatin1Char('<') + t + QLatin1Char('>');
            out.push_back({opening + html.mid(start), QString()});
            break;
        }
        Cut cut;
        if (sentence.at > start + most / 4) cut = sentence;
        else if (space.at > start) cut = space;
        else cut = {i, now};
        // (a cut that leaves a tag half-way is not possible: tokens are whole)
        QString body = html.mid(start, cut.at - start);
        int k = body.size();
        while (k > 0 && body.at(k - 1).isSpace()) --k;
        Chunk ch;
        ch.trailing = body.mid(k);
        body.truncate(k);
        QString opening;
        for (const QString &t : std::as_const(stack)) opening += QLatin1Char('<') + t + QLatin1Char('>');
        QString closing;
        for (int t = cut.stack.size() - 1; t >= 0; --t) closing += QLatin1String("</") + cut.stack.at(t) + QLatin1Char('>');
        ch.html = opening + body + closing;
        if (!body.isEmpty()) out.push_back(ch);
        else if (!ch.trailing.isEmpty() && !out.isEmpty()) out.last().trailing += ch.trailing;
        stack = cut.stack;
        start = cut.at;
    }
    return out;
}

Result rebuild(const Source &source, const QVector<Piece> &list, const QStringList &translated)
{
    Result r;
    r.paragraphs = source.paragraphs;
    // Each piece from the last to the first, so the runs before it keep their numbers.
    for (int k = list.size() - 1; k >= 0; --k) {
        if (k >= translated.size() || translated[k].trimmed().isEmpty()) continue;
        const Piece &p = list[k];
        const QVector<Run> &all = source.paragraphs[p.paragraph].runs;
        const QVector<Run> originals = all.mid(p.first, p.last - p.first + 1);
        int lead = 0, trail = 0;
        trimmedBounds(all, p.first, p.last, &lead, &trail);
        QString joined;
        for (const Run &o : originals) joined += o.text;
        QVector<Run> made = runsFromHtml(translated[k], originals);
        if (made.isEmpty()) continue;
        // The white space the piece had at its ends goes back.
        if (lead > 0) made.first().text.prepend(joined.left(lead));
        if (trail > 0) made.last().text.append(joined.right(trail));
        Replacement rep;
        rep.start = originals.first().start;
        rep.end = originals.last().start + originals.last().text.size();
        rep.runs = made;
        r.replacements.prepend(rep);
        QVector<Run> &runs = r.paragraphs[p.paragraph].runs;
        runs.remove(p.first, p.last - p.first + 1);
        for (int m = 0; m < made.size(); ++m) runs.insert(p.first + m, made[m]);
    }
    return r;
}

QString Result::plainText() const
{
    QStringList lines;
    for (const Paragraph &p : paragraphs) {
        QString t;
        for (const Run &r : p.runs)
            if (!r.skipped) t += r.text;
        t.remove(QChar::ObjectReplacementCharacter).replace(QChar::LineSeparator, QLatin1Char('\n'));
        lines << t;
    }
    return lines.join(QLatin1Char('\n'));
}

QString Result::html() const
{
    QString out;
    for (const Paragraph &p : paragraphs) {
        out += QLatin1String("<p>");
        for (const Run &r : p.runs) {
            if (r.skipped) continue;
            const Flags g = flagsOf(r.format);
            QString part = r.text;
            part.replace(QLatin1Char('&'), QLatin1String("&amp;")).replace(QLatin1Char('<'), QLatin1String("&lt;")).replace(QLatin1Char('>'), QLatin1String("&gt;"));
            part.replace(QChar::LineSeparator, QLatin1String("<br>")).remove(QChar::ObjectReplacementCharacter);
            if (g.bold) part = QLatin1String("<b>") + part + QLatin1String("</b>");
            if (g.italic) part = QLatin1String("<i>") + part + QLatin1String("</i>");
            if (g.underline) part = QLatin1String("<u>") + part + QLatin1String("</u>");
            out += part;
        }
        out += QLatin1String("</p>");
    }
    return out;
}

EditHooks::~EditHooks() = default;

void EditHooks::replaceSelection(QTextCursor &c, const QVector<Run> &runs)
{
    c.beginEditBlock();
    c.removeSelectedText();
    for (const Run &r : runs) c.insertText(r.text, r.format);
    c.endEditBlock();
}

void EditHooks::insertParagraphAfter(QTextCursor &c, const QTextBlockFormat &like, const QVector<Run> &runs)
{
    c.beginEditBlock();
    c.movePosition(QTextCursor::EndOfBlock);
    QTextCharFormat charFormat = runs.isEmpty() ? QTextCharFormat() : runs.first().format;
    c.insertBlock(like, charFormat);
    for (const Run &r : runs) c.insertText(r.text, r.format);
    c.endEditBlock();
}

bool applyReplace(const Source &source, const Result &result, EditHooks &hooks)
{
    if (!source.isCurrent()) return false;
    QTextCursor c(source.document);
    c.beginEditBlock();
    // From the last to the first: what comes before keeps its place.
    for (int k = result.replacements.size() - 1; k >= 0; --k) {
        const Replacement &r = result.replacements[k];
        c.setPosition(r.start);
        c.setPosition(r.end, QTextCursor::KeepAnchor);
        QVector<Run> runs = r.runs;
        for (Run &run : runs) run.format = hooks.languageFormat(run.format, result.language);
        hooks.replaceSelection(c, runs);
    }
    c.endEditBlock();
    return true;
}

bool applyInsertBelow(const Source &source, const Result &result, EditHooks &hooks)
{
    if (!source.isCurrent()) return false;
    QTextCursor c(source.document);
    c.setPosition(source.last);
    c.movePosition(QTextCursor::EndOfBlock);
    c.beginEditBlock();
    for (const Paragraph &p : result.paragraphs) {
        QVector<Run> runs;
        for (Run r : p.runs) {
            if (r.skipped) continue;
            r.text.remove(QChar::ObjectReplacementCharacter);
            r.format = hooks.languageFormat(r.format, result.language);
            if (!r.text.isEmpty()) runs.push_back(r);
        }
        hooks.insertParagraphAfter(c, p.blockFormat, runs);
    }
    c.endEditBlock();
    return true;
}

} // namespace translate
} // namespace jp
