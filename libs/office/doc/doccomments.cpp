// The comments of an old binary document: who made each, where in the text it is, and what it says, written as the comments part of a .docx package.

#include "doc/docconvert.h"

#include <QCoreApplication>
#include <QHash>
#include <QSet>
#include <algorithm>

namespace jp::office::doc {

namespace {

// The authors' names: XSTs (a length in characters, then the characters) one after another, at most `limit`, each of at most 255 characters (section 2.5.6 puts 55 at most).
QStringList readOwners(const Span &s, int limit)
{
    QStringList out;
    qint64 o = 0;
    while (out.size() < limit && o + 2 <= s.size()) {
        const int cch = s.u16(o);
        o += 2;
        if (cch > 255 || !s.fits(o, 2 * qint64(cch))) break;
        QString name;
        for (int k = 0; k < cch; ++k) name += QChar(s.u16(o + 2 * k));
        out << name;
        o += 2 * qint64(cch);
    }
    return out;
}

} // namespace

int Converter::commentAt(qint64 cp) const
{
    auto it = std::lower_bound(comments.begin(), comments.end(), cp, [](const CommentInfo &c, qint64 v) { return c.reference < v; });
    return it != comments.end() && it->reference == cp ? int(it - comments.begin()) : -1;
}

void Converter::readComments()
{
    if (fib.ccpAtn <= 0) return;
    // Section 2.8.7: a 30-byte record for each comment (initials, the author's place in the list of authors, and the tag of the bookmark that holds its range); section 2.8.8: the
    // comment story's positions, one more than the comments and the last ignored.
    const Plc ref(tableRange(Fib::PlcfandRef), 30), txt(tableRange(Fib::PlcfandTxt), 0);
    const int count = std::min({ref.n, txt.n - 1, kMaxComments + 1});
    if (count <= 0) return;
    const QStringList owners = readOwners(tableRange(Fib::GrpXstAtnOwners), kMaxCommentAuthors);
    // The dates (section 2.9.5, 2.9.6: 18 bytes for each comment, the date first, in the files of 2002 and later).
    const Span extra = tableRange(Fib::AtrdExtra);
    // The ranges the comments cover (section 2.9.277, 2.9.9): the tags of the bookmarks, in the order of the starts and ends.
    struct Range {
        qint64 from, to;
    };
    QHash<quint32, Range> ranges;
    {
        const Span names = tableRange(Fib::SttbfAtnBkmk);
        const Plc starts(tableRange(Fib::PlcfAtnBkf), 4), ends(tableRange(Fib::PlcfAtnBkl), 0);
        const int cData = names.u16(0) == 0xFFFF ? names.u16(2) : 0, cbExtra = names.u16(4);
        qint64 o = 6;
        for (int i = 0; i < cData && i < starts.n && i < 0x3FFC && cbExtra >= 10; ++i) {
            const int cch = names.u16(o);
            if (!names.fits(o, 2 + 2 * qint64(cch) + cbExtra)) break;
            const qint64 e = o + 2 + 2 * qint64(cch);
            o = e + cbExtra;
            const int endIndex = starts.record(i).u16(0);
            // (The last position of a table of ends is not an end, section 2.8.10; and an end before its start, which the same section rules out, is no range: the comment has its point.)
            if (endIndex >= ends.n) continue;
            const qint64 a = starts.cp(i), b = ends.cp(endIndex);
            if (a < 0 || a >= fib.ccpText || b < a) continue;
            ranges.insert(names.u32(e + 2), {a, std::min<qint64>(b, fib.ccpText)});
        }
    }
    const QString &T = pieces.text();
    QVector<CommentInfo> found;
    QVector<Range> where;
    bool tooMany = false, tooLong = false;
    qint64 previous = -1;
    for (int i = 0; i < count; ++i) {
        ++commentsDeclared;
        const qint64 cp = ref.cp(i);
        // (Each comment is cited by the character 0x05, once; the references follow one another.)
        if (cp < 0 || cp >= fib.ccpText || cp >= T.size() || T[cp] != QChar(0x05) || cp <= previous) continue;
        if (found.size() >= kMaxComments) {
            tooMany = true;
            --commentsDeclared;
            break;
        }
        previous = cp;
        const Span r = ref.record(i);
        CommentInfo c;
        c.reference = cp;
        const int cchInitials = std::min<int>(r.u16(0), 9);
        for (int k = 0; k < cchInitials; ++k) c.initials += QChar(r.u16(2 + 2 * k));
        const int owner = r.u16(20);
        if (owner < owners.size()) c.author = owners[owner];
        if (extra.fits(qint64(i) * 18, 18)) c.date = extra.u32(qint64(i) * 18);
        c.textFrom = std::clamp<qint64>(txt.cp(i), 0, fib.ccpAtn);
        c.textTo = std::clamp<qint64>(txt.cp(i + 1), c.textFrom, fib.ccpAtn);
        if (c.textTo - c.textFrom > kMaxCommentChars) {
            c.textTo = c.textFrom + kMaxCommentChars;
            tooLong = true;
        }
        found.push_back(c);
        const auto it = ranges.constFind(r.u32(26));
        where.push_back(it != ranges.constEnd() ? it.value() : Range{cp, cp});
    }
    if (tooMany) left(QStringLiteral("commentsMany"), QCoreApplication::translate("jp::office::doc", "The document has more comments than this program can hold, and the last ones were left out."));
    if (tooLong) left(QStringLiteral("commentsLong"), QCoreApplication::translate("jp::office::doc", "Comments longer than this program can hold were cut short."));
    comments = found;
    // The marks of the ranges, among the bookmarks' (a range that ends where another begins ends first).
    QVector<BookmarkEvent> marks;
    for (int i = 0; i < where.size(); ++i) {
        if (where[i].from == where[i].to) continue;
        const QByteArray id = QByteArray::number(i);
        marks.push_back({where[i].from, "<w:commentRangeStart w:id=\"" + id + "\"/>"});
        marks.push_back({where[i].to, "<w:commentRangeEnd w:id=\"" + id + "\"/>"});
    }
    if (marks.isEmpty()) return;
    for (const BookmarkEvent &m : marks) bookmarkEvents.push_back(m);
    std::stable_sort(bookmarkEvents.begin(), bookmarkEvents.end(), [](const BookmarkEvent &x, const BookmarkEvent &y) { return x.cp < y.cp; });
}

QByteArray Converter::commentsXml(const QVector<int> &ids, QVector<int> *written)
{
    QByteArray x = QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<w:comments ") + kNamespaces + ">";
    const qint64 base = fib.commentStart();
    for (const int i : ids) {
        const CommentInfo &c = comments[i];
        QByteArray body = c.textTo > c.textFrom ? story(base + c.textFrom, base + c.textTo, Kind::Comment, QStringLiteral("/word/comments.xml")) : QByteArray();
        if (truncated) break;   // (this one is not whole, and it and the rest are not written)
        if (body.isEmpty()) body = "<w:p/>";
        QByteArray one = "<w:comment" + revisionAttributes(i, c.author, c.date);
        if (!c.initials.isEmpty()) one += " w:initials=\"" + xmlEscape(c.initials) + "\"";
        one += ">" + body + "</w:comment>";
        if (!spend(one)) break;
        x += one;
        written->push_back(i);
    }
    x += "</w:comments>";
    return x;
}

QByteArray Converter::finishComments(QByteArray *body)
{
    if (comments.isEmpty() && fib.ccpAtn <= 0) return QByteArray();
    // The marks of comments in the main text: where each is, what it is, and which comment it is of.
    struct Mark {
        qsizetype at, len;
        int kind, id;   // 0 a reference, 1 the start of a range, 2 its end
    };
    QVector<Mark> marks;
    static const QByteArray tag("<w:comment");
    static const struct { const char *name; int kind; } kinds[] = {{"Reference w:id=\"", 0}, {"RangeStart w:id=\"", 1}, {"RangeEnd w:id=\"", 2}};
    for (qsizetype at = 0; (at = body->indexOf(tag, at)) >= 0;) {
        const qsizetype from = at + tag.size();
        at = from;
        for (const auto &k : kinds) {
            const qsizetype n = qsizetype(strlen(k.name));
            if (body->mid(from, n) != k.name) continue;
            const qsizetype close = body->indexOf("/>", from + n);
            if (close < 0) break;
            bool ok = false;
            const int id = body->mid(from + n, close - (from + n) - 1).toInt(&ok);
            if (ok) marks.push_back({from - tag.size(), close + 2 - (from - tag.size()), k.kind, id});
            break;
        }
    }
    // The comments to write: those the body cites, in the order it does.
    QSet<int> cited;
    for (const Mark &m : std::as_const(marks))
        if (m.kind == 0 && m.id >= 0 && m.id < comments.size()) cited.insert(m.id);
    QVector<int> ids(cited.begin(), cited.end());
    std::sort(ids.begin(), ids.end());
    // (What they are made of has a budget of its own, so the text above does not give way to them; its counts are kept for the work done.)
    const qint64 bytes = outputBytes, elements = outputElements;
    const int madeParagraphs = paragraphs, madeCells = tableCells;
    const bool cut = truncated;
    outputBytes = outputElements = 0;
    paragraphs = tableCells = 0;
    truncated = false;
    inComments = true;
    QVector<int> written;
    const QByteArray x = commentsXml(ids, &written);
    inComments = false;
    outputBytes += bytes;
    outputElements += elements;
    paragraphs = madeParagraphs;
    tableCells = madeCells;
    truncated = cut;
    const QSet<int> kept(written.begin(), written.end());
    // Out of the body go the marks of the comments not written, the starts of ranges with no end, and the ends with no start.
    QSet<int> started, ended;
    for (const Mark &m : std::as_const(marks)) {
        if (m.kind == 1 && kept.contains(m.id)) started.insert(m.id);
        if (m.kind == 2 && kept.contains(m.id) && started.contains(m.id)) ended.insert(m.id);
    }
    QByteArray cleaned;
    qsizetype from = 0;
    bool changed = false;
    for (const Mark &m : std::as_const(marks)) {
        const bool keep = kept.contains(m.id) && (m.kind == 0 || (m.kind == 1 ? ended.contains(m.id) : started.contains(m.id)));
        if (keep) continue;
        if (!changed) cleaned.reserve(body->size());
        changed = true;
        cleaned.append(body->constData() + from, m.at - from);
        from = m.at + m.len;
    }
    if (changed) {
        cleaned.append(body->constData() + from, body->size() - from);
        *body = cleaned;
    }
    const int lost = commentsDeclared - int(written.size());
    if (written.isEmpty() && (commentsDeclared > 0 || fib.ccpAtn > 0))
        left(QStringLiteral("commentsNone"), QCoreApplication::translate("jp::office::doc", "The comments in the document could not be read, and were left out."));
    else if (lost > 0)
        left(QStringLiteral("commentsLost"), QCoreApplication::translate("jp::office::doc", "%n comment(s) could not be read and were left out.", nullptr, lost));
    return written.isEmpty() ? QByteArray() : x;
}

} // namespace jp::office::doc
