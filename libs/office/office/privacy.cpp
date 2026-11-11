#include "office/privacy.h"

#include "io/zip.h"
#include "io/zipindex.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QStringDecoder>
#include <QVector>
#include <optional>

namespace jp::office {

const char *const kAnonymousAuthor = "Author";

int removePersonalNames(PackageProps &p)
{
    int n = 0;
    for (QString *s : {&p.creator, &p.lastModifiedBy, &p.company, &p.manager})
        if (!s->isEmpty()) {
            s->clear();
            ++n;
        }
    return n;
}

namespace {

std::function<QByteArray(const QByteArray &)> &hook()
{
    static std::function<QByteArray(const QByteArray &)> h;
    return h;
}

constexpr qint64 kMostInflated = qint64(2) << 30;   // (the most the parts rewritten may come to, together)
const QString kEpoch = QStringLiteral("1970-01-01T00:00:00Z");

// ---- A part of XML as pieces: what is in a tag is told from what is in text, so that only real attributes and elements are looked at.

struct Seg {
    enum Kind { Text, Cdata, Comment, Pi, Decl, Start, End, Empty } kind = Text;
    QString s;
};

QString localName(const QString &qname)
{
    const int c = qname.indexOf(QLatin1Char(':'));
    return c < 0 ? qname : qname.mid(c + 1);
}

bool isSpace(QChar c) { return c == QLatin1Char(' ') || c == QLatin1Char('\t') || c == QLatin1Char('\n') || c == QLatin1Char('\r'); }

// The end of the tag that starts at `at` (the place of its '>'), quotes respected; -1 for none.
qsizetype tagEnd(const QString &x, qsizetype at)
{
    QChar quote;
    for (qsizetype i = at + 1; i < x.size(); ++i) {
        const QChar c = x[i];
        if (!quote.isNull()) {
            if (c == quote) quote = QChar();
        } else if (c == QLatin1Char('"') || c == QLatin1Char('\'')) quote = c;
        else if (c == QLatin1Char('>')) return i;
    }
    return -1;
}

bool tokenize(const QString &x, QVector<Seg> *out)
{
    qsizetype i = 0;
    const qsizetype n = x.size();
    auto take = [&](Seg::Kind k, qsizetype to) {
        out->push_back({k, x.mid(i, to - i)});
        i = to;
    };
    while (i < n) {
        if (x[i] != QLatin1Char('<')) {
            qsizetype j = x.indexOf(QLatin1Char('<'), i);
            take(Seg::Text, j < 0 ? n : j);
        } else if (x.mid(i, 4) == QLatin1String("<!--")) {
            const qsizetype j = x.indexOf(QLatin1String("-->"), i + 4);
            if (j < 0) return false;
            take(Seg::Comment, j + 3);
        } else if (x.mid(i, 9) == QLatin1String("<![CDATA[")) {
            const qsizetype j = x.indexOf(QLatin1String("]]>"), i + 9);
            if (j < 0) return false;
            take(Seg::Cdata, j + 3);
        } else if (x.mid(i, 2) == QLatin1String("<?")) {
            const qsizetype j = x.indexOf(QLatin1String("?>"), i + 2);
            if (j < 0) return false;
            take(Seg::Pi, j + 2);
        } else if (x.mid(i, 2) == QLatin1String("<!")) {
            const qsizetype j = tagEnd(x, i);
            if (j < 0) return false;
            take(Seg::Decl, j + 1);
        } else {
            const qsizetype j = tagEnd(x, i);
            if (j < 0) return false;
            const bool end = i + 1 < n && x[i + 1] == QLatin1Char('/');
            take(end ? Seg::End : x[j - 1] == QLatin1Char('/') ? Seg::Empty : Seg::Start, j + 1);
        }
    }
    return true;
}

QString tagName(const QString &tag)
{
    qsizetype i = tag.startsWith(QLatin1String("</")) ? 2 : 1;
    const qsizetype from = i;
    while (i < tag.size() && !isSpace(tag[i]) && tag[i] != QLatin1Char('/') && tag[i] != QLatin1Char('>')) ++i;
    return tag.mid(from, i - from);
}

QString decode(const QString &t)
{
    if (!t.contains(QLatin1Char('&'))) return t;
    QString r;
    for (qsizetype i = 0; i < t.size(); ++i) {
        if (t[i] != QLatin1Char('&')) {
            r += t[i];
            continue;
        }
        const qsizetype j = t.indexOf(QLatin1Char(';'), i);
        if (j < 0) {
            r += t[i];
            continue;
        }
        const QString e = t.mid(i + 1, j - i - 1);
        QString v;
        if (e == QLatin1String("amp")) v = QStringLiteral("&");
        else if (e == QLatin1String("lt")) v = QStringLiteral("<");
        else if (e == QLatin1String("gt")) v = QStringLiteral(">");
        else if (e == QLatin1String("quot")) v = QStringLiteral("\"");
        else if (e == QLatin1String("apos")) v = QStringLiteral("'");
        else if (e.startsWith(QLatin1String("#"))) {
            bool ok = false;
            const uint cp = e.startsWith(QLatin1String("#x")) ? e.mid(2).toUInt(&ok, 16) : e.mid(1).toUInt(&ok);
            if (ok && cp > 0 && cp <= 0x10FFFF) v = QString::fromUcs4(reinterpret_cast<const char32_t *>(&cp), 1);
        }
        if (v.isEmpty()) {
            r += t[i];
            continue;
        }
        r += v;
        i = j;
    }
    return r;
}

QString escapeText(const QString &t) { return QString(t).replace(QLatin1Char('&'), QLatin1String("&amp;")).replace(QLatin1Char('<'), QLatin1String("&lt;")).replace(QLatin1Char('>'), QLatin1String("&gt;")); }
QString escapeAttr(const QString &t) { return escapeText(t).replace(QLatin1Char('"'), QLatin1String("&quot;")); }

// The text of the pieces from `from` up to (not including) `to`: text decoded, CDATA as it is.
QString textOf(const QVector<Seg> &segs, int from, int to)
{
    QString r;
    for (int i = from; i < to; ++i) {
        if (segs[i].kind == Seg::Text) r += decode(segs[i].s);
        else if (segs[i].kind == Seg::Cdata) r += segs[i].s.mid(9, segs[i].s.size() - 12);
    }
    return r;
}

// ---- What each part has taken out of it.

struct AttrRule {
    QString element;   // the local name of the element it is on; empty for any
    QString attr;      // the name as written (a prefixed name is matched as written, one without a prefix matches that attribute with no prefix)
    QString with;
    bool remove = false;
};

struct TextRule {
    QString element;   // local name
    std::function<std::optional<QString>(const QString &)> change;   // the new text, or none to leave it
};

struct Rules {
    QVector<AttrRule> attrs;
    QVector<TextRule> texts;
    QSet<QString> drops;   // local names of elements taken out whole
    bool noteRuns = false;
    QVector<QByteArray> needles;   // a part that has none of these has nothing to take out
};

AttrRule setTo(const QString &element, const QString &attr, const QString &with) { return {element, attr, with, false}; }
AttrRule removed(const QString &element, const QString &attr) { return {element, attr, QString(), true}; }
TextRule emptied(const QString &element)
{
    return {element, [](const QString &old) -> std::optional<QString> {
                if (old.isEmpty()) return std::nullopt;
                return QString();
            }};
}
TextRule anonymized(const QString &element, bool keepThreadLinks)
{
    const QString anon = QString::fromLatin1(kAnonymousAuthor);
    return {element, [anon, keepThreadLinks](const QString &old) -> std::optional<QString> {
                if (old == anon || old.isEmpty() || (keepThreadLinks && old.trimmed().startsWith(QLatin1String("tc=")))) return std::nullopt;
                return anon;
            }};
}

bool hasRules(const QString &name, Rules *r)
{
    const QString n = name.toLower();
    const QString anon = QString::fromLatin1(kAnonymousAuthor);
    if (n == QLatin1String("docprops/core.xml")) {
        r->texts = {emptied(QStringLiteral("creator")), emptied(QStringLiteral("lastModifiedBy"))};
        r->needles = {"creator", "lastModifiedBy"};
    } else if (n == QLatin1String("docprops/app.xml")) {
        r->texts = {emptied(QStringLiteral("Company")), emptied(QStringLiteral("Manager"))};
        r->needles = {"Company", "Manager"};
    } else if (n == QLatin1String("word/people.xml")) {
        r->attrs = {setTo(QString(), QStringLiteral("w15:author"), anon)};
        r->drops = {QStringLiteral("presenceInfo")};
        r->needles = {"author", "presenceInfo"};
    } else if (n.startsWith(QLatin1String("word/")) && n.endsWith(QLatin1String(".xml"))) {
        r->attrs = {setTo(QString(), QStringLiteral("w:author"), anon), removed(QString(), QStringLiteral("w:initials")), removed(QString(), QStringLiteral("w:date")),
                    removed(QString(), QStringLiteral("w16cex:dateUtc")), removed(QString(), QStringLiteral("w16du:dateUtc"))};
        r->needles = {"w:author", "w:initials", "w:date", "dateUtc"};
    } else if (n == QLatin1String("ppt/commentauthors.xml")) {
        r->attrs = {setTo(QStringLiteral("cmAuthor"), QStringLiteral("name"), anon), setTo(QStringLiteral("cmAuthor"), QStringLiteral("initials"), QStringLiteral("A"))};
        r->needles = {"name", "initials"};
    } else if (n == QLatin1String("ppt/authors.xml")) {
        r->attrs = {setTo(QStringLiteral("author"), QStringLiteral("name"), anon), setTo(QStringLiteral("author"), QStringLiteral("initials"), QStringLiteral("A")),
                    setTo(QStringLiteral("author"), QStringLiteral("userId"), anon), setTo(QStringLiteral("author"), QStringLiteral("providerId"), QStringLiteral("None"))};
        r->needles = {"name", "userId", "providerId"};
    } else if (n.startsWith(QLatin1String("ppt/comments/")) && n.endsWith(QLatin1String(".xml"))) {
        r->attrs = {removed(QString(), QStringLiteral("dt")), setTo(QString(), QStringLiteral("created"), kEpoch)};
        r->needles = {"dt=", "created="};
    } else if (n.startsWith(QLatin1String("xl/comments")) && n.endsWith(QLatin1String(".xml"))) {
        r->texts = {anonymized(QStringLiteral("author"), true)};
        r->noteRuns = true;
        r->needles = {"author"};
    } else if (n.startsWith(QLatin1String("xl/threadedcomments/")) && n.endsWith(QLatin1String(".xml"))) {
        r->attrs = {removed(QStringLiteral("threadedComment"), QStringLiteral("dT"))};
        r->needles = {"dT="};
    } else if (n.startsWith(QLatin1String("xl/persons/")) && n.endsWith(QLatin1String(".xml"))) {
        r->attrs = {setTo(QStringLiteral("person"), QStringLiteral("displayName"), anon), setTo(QStringLiteral("person"), QStringLiteral("userId"), anon),
                    setTo(QStringLiteral("person"), QStringLiteral("providerId"), QStringLiteral("None"))};
        r->needles = {"displayName", "userId", "providerId"};
    } else if (n.startsWith(QLatin1String("xl/revisions/")) && n.endsWith(QLatin1String(".xml"))) {
        r->attrs = {setTo(QString(), QStringLiteral("userName"), anon), setTo(QString(), QStringLiteral("dateTime"), kEpoch), setTo(QStringLiteral("userinfo"), QStringLiteral("name"), anon)};
        r->needles = {"userName", "dateTime", "userinfo"};
    } else if (n == QLatin1String("xl/workbook.xml")) {
        r->attrs = {removed(QStringLiteral("fileSharing"), QStringLiteral("userName")), setTo(QStringLiteral("absPath"), QStringLiteral("url"), QString())};
        r->needles = {"fileSharing", "absPath"};
    } else if (n.startsWith(QLatin1String("xl/worksheets/")) && n.endsWith(QLatin1String(".xml"))) {
        r->attrs = {removed(QStringLiteral("scenario"), QStringLiteral("user"))};
        r->needles = {"<scenario ", ":scenario "};
    } else if (n == QLatin1String("meta.xml")) {
        r->texts = {emptied(QStringLiteral("initial-creator")), emptied(QStringLiteral("creator")), emptied(QStringLiteral("printed-by"))};
        r->needles = {"creator", "printed-by"};
    } else if (n == QLatin1String("content.xml") || n == QLatin1String("styles.xml")) {
        r->texts = {anonymized(QStringLiteral("creator"), false), emptied(QStringLiteral("creator-initials"))};
        r->drops = {QStringLiteral("date")};
        r->needles = {"creator", "dc:date"};
    } else {
        return false;
    }
    return true;
}

// The tag with the attributes the rules name changed; `changed` is set when it is not as it was.
QString editTag(const QString &tag, const Rules &rules, bool *changed)
{
    const QString element = localName(tagName(tag));
    QString out;
    qsizetype copied = 0, i = 1;
    while (i < tag.size() && !isSpace(tag[i]) && tag[i] != QLatin1Char('/') && tag[i] != QLatin1Char('>')) ++i;
    for (;;) {
        const qsizetype wsStart = i;
        while (i < tag.size() && isSpace(tag[i])) ++i;
        if (i >= tag.size() || tag[i] == QLatin1Char('/') || tag[i] == QLatin1Char('>')) break;
        const qsizetype nameStart = i;
        while (i < tag.size() && !isSpace(tag[i]) && tag[i] != QLatin1Char('=')) ++i;
        const QString name = tag.mid(nameStart, i - nameStart);
        while (i < tag.size() && isSpace(tag[i])) ++i;
        if (i >= tag.size() || tag[i] != QLatin1Char('=')) break;
        ++i;
        while (i < tag.size() && isSpace(tag[i])) ++i;
        if (i >= tag.size() || (tag[i] != QLatin1Char('"') && tag[i] != QLatin1Char('\''))) break;
        const QChar quote = tag[i];
        const qsizetype close = tag.indexOf(quote, i + 1);
        if (close < 0) break;
        const qsizetype end = close + 1;
        const QString value = decode(tag.mid(i + 1, close - i - 1));
        i = end;
        for (const AttrRule &r : rules.attrs) {
            if (r.attr != name || (!r.element.isEmpty() && r.element != element)) continue;
            if (r.remove) {
                out += tag.mid(copied, wsStart - copied);
                copied = end;
                *changed = true;
            } else if (value != r.with) {
                out += tag.mid(copied, nameStart - copied) + name + QLatin1String("=\"") + escapeAttr(r.with) + QLatin1Char('"');
                copied = end;
                *changed = true;
            }
            break;
        }
    }
    return out + tag.mid(copied);
}

int endOf(const QVector<Seg> &segs, int start, const QString &qname)
{
    int depth = 0;
    for (int j = start + 1; j < segs.size(); ++j) {
        if (segs[j].kind == Seg::Start && tagName(segs[j].s) == qname) ++depth;
        else if (segs[j].kind == Seg::End && tagName(segs[j].s) == qname) {
            if (depth == 0) return j;
            --depth;
        }
    }
    return -1;
}

QString attrValue(const QString &tag, const QString &name)
{
    Rules probe;
    probe.attrs = {};
    const QRegularExpression re(QStringLiteral("\\s%1\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)')").arg(QRegularExpression::escape(name)));
    const QRegularExpressionMatch m = re.match(tag);
    return m.hasMatch() ? decode(m.captured(1).isEmpty() ? m.captured(2) : m.captured(1)) : QString();
}

// A note of a sheet begins with a run that is the author's name and a colon: that run goes, when the author's name is taken out. (The authors are read before they are made anonymous.)
bool removeNoteNameRuns(QVector<Seg> *segs)
{
    QStringList authors;
    for (int i = 0; i < segs->size(); ++i)
        if ((*segs)[i].kind == Seg::Start && localName(tagName((*segs)[i].s)) == QLatin1String("author")) {
            const int j = endOf(*segs, i, tagName((*segs)[i].s));
            authors << (j < 0 ? QString() : textOf(*segs, i + 1, j));
        }
    bool changed = false;
    for (int i = 0; i < segs->size(); ++i) {
        const Seg &s = (*segs)[i];
        if (s.kind != Seg::Start || localName(tagName(s.s)) != QLatin1String("comment")) continue;
        const QString idText = attrValue(s.s, QStringLiteral("authorId"));
        bool ok = false;
        const int id = idText.toInt(&ok);
        if (!ok || id < 0 || id >= authors.size() || authors[id].isEmpty() || authors[id].trimmed().startsWith(QLatin1String("tc="))) continue;
        const int close = endOf(*segs, i, tagName(s.s));
        if (close < 0) continue;
        for (int r = i + 1; r < close; ++r) {
            if ((*segs)[r].kind != Seg::Start || localName(tagName((*segs)[r].s)) != QLatin1String("r")) continue;
            const int rEnd = endOf(*segs, r, tagName((*segs)[r].s));
            if (rEnd > 0 && textOf(*segs, r + 1, rEnd) == authors[id] + QLatin1Char(':')) {
                segs->erase(segs->begin() + r, segs->begin() + rEnd + 1);
                changed = true;
            }
            break;   // (the first run only)
        }
    }
    return changed;
}

// The part rewritten; false when it is not XML this can read. `changed` is whether anything was taken out.
bool rewrite(const QString &x, const Rules &rules, QString *result, bool *changed)
{
    QVector<Seg> segs;
    if (!tokenize(x, &segs)) return false;
    *changed = false;
    if (rules.noteRuns && removeNoteNameRuns(&segs)) *changed = true;
    for (int i = 0; i < segs.size(); ++i) {
        Seg &s = segs[i];
        if (s.kind != Seg::Start && s.kind != Seg::Empty) continue;
        const QString qname = tagName(s.s);
        const QString local = localName(qname);
        if (rules.drops.contains(local)) {
            const int j = s.kind == Seg::Empty ? i : endOf(segs, i, qname);
            if (j >= 0) {
                segs.erase(segs.begin() + i, segs.begin() + j + 1);
                *changed = true;
                --i;
                continue;
            }
        }
        if (!rules.attrs.isEmpty()) {
            bool edited = false;
            const QString tag = editTag(s.s, rules, &edited);
            if (edited) {
                s.s = tag;
                *changed = true;
            }
        }
        if (s.kind != Seg::Start) continue;
        for (const TextRule &t : rules.texts) {
            if (t.element != local) continue;
            const int j = endOf(segs, i, qname);
            if (j < 0) break;
            bool simple = true;
            for (int k = i + 1; k < j; ++k) simple = simple && (segs[k].kind == Seg::Text || segs[k].kind == Seg::Cdata);
            if (!simple) break;
            const std::optional<QString> now = t.change(textOf(segs, i + 1, j));
            if (now) {
                segs.erase(segs.begin() + i + 1, segs.begin() + j);
                if (!now->isEmpty()) segs.insert(segs.begin() + i + 1, Seg{Seg::Text, escapeText(*now)});
                *changed = true;
            }
            break;
        }
    }
    QString r;
    r.reserve(x.size());
    for (const Seg &s : std::as_const(segs)) r += s.s;
    *result = r;
    return true;
}

QString say(const char *text) { return QCoreApplication::translate("jp::office::Privacy", text); }

} // namespace

void setPackageScrubHookForTest(std::function<QByteArray(const QByteArray &)> h) { hook() = std::move(h); }

QByteArray removePersonalFromPackage(const QByteArray &zipIn, int *parts, QStringList *problems)
{
    if (parts) *parts = 0;
    const QByteArray zip = hook() ? hook()(zipIn) : zipIn;
    auto fail = [&](const QString &why) {
        if (problems) *problems << why;
    };
    QVector<ZipEntry> directory;
    if (!readZipDirectory(zip, &directory)) {
        fail(say("The file is not a package that could be read."));
        return zipIn;
    }
    const InflateCounter budget = std::make_shared<InflateBudget>();
    budget->limit = kMostInflated;
    ZipWriter out;
    int changed = 0;
    for (const ZipEntry &e : std::as_const(directory)) {
        Rules rules;
        if (!e.name.endsWith(QLatin1Char('/')) && hasRules(e.name, &rules)) {
            QByteArray data;
            ZipFailure why = ZipFailure::None;
            if (!inflateZipEntry(zip, e, &data, budget, &why)) {
                fail(say("A part (%1) could not be read.").arg(e.name));
            } else if (data.startsWith("\xFF\xFE") || data.startsWith("\xFE\xFF")) {
                fail(say("A part (%1) is not in a form that could be rewritten.").arg(e.name));
            } else {
                bool wanted = false;
                for (const QByteArray &needle : std::as_const(rules.needles)) wanted = wanted || data.contains(needle);
                if (wanted) {
                    QStringDecoder utf8(QStringDecoder::Utf8, QStringConverter::Flag::Stateless);
                    const QString before = utf8(data);
                    QString after;
                    bool did = false;
                    if (utf8.hasError()) fail(say("A part (%1) is not in a form that could be rewritten.").arg(e.name));
                    else if (!rewrite(before, rules, &after, &did)) fail(say("A part (%1) could not be read as XML.").arg(e.name));
                    else if (did) {
                        out.add(e.name, after.toUtf8(), true);
                        ++changed;
                        continue;
                    }
                }
            }
        }
        out.addRaw(e.name, e.method, e.crc, quint64(e.csize), quint64(e.usize), zip.constData() + e.dataAt, e.flags);
    }
    if (parts) *parts = changed;
    return out.finish();
}

} // namespace jp::office
