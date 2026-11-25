#include "office/packageprops.h"
#include "core/inspectops.h"
#include "core/mergeops.h"

#include "core/commentops.h"
#include "core/fieldops.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QRegularExpression>
#include <QStringConverter>
#include <QXmlStreamReader>
#include <atomic>
#include <functional>
#include <climits>
#include <cstring>
#include <QTextBlock>
#include <QTextCursor>

namespace jp::write {

namespace {

bool hiddenRun(const QTextCharFormat &f) { return f.boolProperty(tp::HiddenText) && !f.hasProperty(prop::TrackedDeleted) && !f.hasProperty(prop::Atom); }

// The runs of hidden text in a story as (position, length), in order.
QVector<QPair<int, int>> hiddenRuns(const QTextDocument *story)
{
    QVector<QPair<int, int>> runs;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            if (!hiddenRun(fr.charFormat())) continue;
            if (!runs.isEmpty() && runs.last().first + runs.last().second == fr.position()) runs.last().second += fr.length();
            else runs.push_back({fr.position(), fr.length()});
        }
    return runs;
}

// Links to other files in kept XML (the pictures and objects of the text, kept as the XML they came in, and the parts the program keeps whole: the comments, and so on). Each part names its links by
// Ids of its own relationships. Every pass looks through the bytes once, and what is looked through is counted (externalLinkBytesScannedForTest).
constexpr qint64 kMostLinkBytes = qint64(1) << 26;   // a part larger than this is not looked through: it is said to be unchecked
constexpr int kMostLinkEdits = 100000;               // the most links one Remove All takes out of kept XML; the rest wait for the next
std::atomic<qint64> s_linkBytes{0};
std::atomic<int> s_mostLinkEdits{0};
std::atomic<qint64> s_linkSteps{0};
std::atomic<qint64> s_linkTokens{0};

struct LinkIds {
    QSet<QString> files;    // to addresses outside the file that are not web addresses
    QSet<QString> inside;   // to parts of the file
    QSet<QString> every;
};

bool pictureOrObjectKind(const QString &type)
{
    for (const char *end : {"/image", "/oleObject", "/package", "/video", "/audio", "/media"})
        if (type.endsWith(QLatin1String(end))) return true;
    return false;
}

// The Ids of a part's relationships; a hyperlink is not a link to a file, and in a part kept whole only a picture's or an object's kind is (the attached template and the like are another matter).
LinkIds linkIdsOf(const QVector<office::Relationship> &rels, bool kindsOnly)
{
    LinkIds ids;
    for (const office::Relationship &r : rels) {
        ids.every.insert(r.id);
        if (!r.external) ids.inside.insert(r.id);
        else if (!r.type.endsWith(QLatin1String("/hyperlink")) && (!kindsOnly || pictureOrObjectKind(r.type))) ids.files.insert(r.id);
    }
    return ids;
}

// Whether the bytes of a picture or object have a quoted value that is one of the Ids (the attribute that names it, r:link or r:id, is the format's).
bool namesAnId(const QByteArray &bytes, const QSet<QString> &ids)
{
    for (qsizetype at = 0; at < bytes.size(); ++at) {
        const char quote = bytes[at];
        if (quote != '"' && quote != '\'') continue;
        const qsizetype end = bytes.indexOf(quote, at + 1);
        if (end < 0) break;
        if (end - at - 1 <= 64 && ids.contains(QString::fromLatin1(bytes.mid(at + 1, end - at - 1)))) return true;
        at = end;
    }
    return false;
}

bool mightLink(const QByteArray &bytes, const LinkIds &ids) { return bytes.contains(":link") || bytes.contains(":href") || (!ids.files.isEmpty() && namesAnId(bytes, ids.files)); }

// One attribute that names a link to a file: where it is (with the blank before it), the Id, whether it is inside the content of a text box, and whether it can be left out alone (it is a link beside
// the picture's own data: another attribute of the same element names a part of the file).
struct Ref {
    qsizetype from = 0, to = 0;
    QString id;
    bool inBox = false, removable = false;
};
struct Scan {
    QVector<Ref> refs;
    int unresolved = 0;   // links by an Id the part has no relationship for
    bool broken = false;  // the bytes stop making sense as markup: what follows was not looked at
    qint64 steps = 0;     // the attributes looked at
};

Scan scanRefs(const QByteArray &x, const LinkIds &ids, const QHash<QByteArray, QByteArray> *outer = nullptr)
{
    Scan out;
    s_linkBytes += x.size();
    const qsizetype n = x.size();
    const char *d = x.constData();
    auto has = [&](qsizetype p, const char *s) {
        const qsizetype l = qsizetype(strlen(s));
        return p + l <= n && memcmp(d + p, s, size_t(l)) == 0;
    };
    auto space = [](char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; };
    auto localOf = [&](qsizetype from, qsizetype to) {
        qsizetype c = to;
        while (c > from && d[c - 1] != ':') --c;
        return QByteArrayView(d + c, to - c);
    };
    struct Attr {
        qsizetype with, end;
        QByteArrayView name, local, value;
        bool rel = true;   // (not a prefix this part binds to some other namespace than the relationships')
    };
    QVector<Attr> attrs;
    // (the namespace prefixes in scope: what each is bound to, and what the declarations of an element put back when it ends)
    struct Undo {
        QByteArray prefix, old;
        bool had;
    };
    QHash<QByteArray, QByteArray> bound = outer ? *outer : QHash<QByteArray, QByteArray>();
    QVector<Undo> log;
    QVector<qsizetype> frames;
    auto rollBack = [&](qsizetype to) {
        while (log.size() > to) {
            const Undo u = log.takeLast();
            if (u.had) bound.insert(u.prefix, u.old);
            else bound.remove(u.prefix);
        }
    };
    qint64 steps = 0;
    int box = 0;
    qsizetype i = 0;
    while (i < n) {
        const qsizetype lt = x.indexOf('<', i);
        if (lt < 0) break;
        auto skipTo = [&](const char *close, qsizetype from) {
            const qsizetype e = x.indexOf(close, from);
            if (e < 0) out.broken = true;
            else i = e + qsizetype(strlen(close));
            return e >= 0;
        };
        if (has(lt, "<!--")) { if (!skipTo("-->", lt + 4)) break; continue; }
        if (has(lt, "<![CDATA[")) { if (!skipTo("]]>", lt + 9)) break; continue; }
        if (has(lt, "<?")) { if (!skipTo("?>", lt + 2)) break; continue; }
        if (has(lt, "<!")) { if (!skipTo(">", lt + 2)) break; continue; }
        if (lt + 1 >= n) { out.broken = true; break; }
        if (d[lt + 1] == '/') {
            const qsizetype e = x.indexOf('>', lt);
            if (e < 0) { out.broken = true; break; }
            qsizetype ne = lt + 2;
            while (ne < e && !space(d[ne])) ++ne;
            if (localOf(lt + 2, ne) == QByteArrayView("txbxContent") && box > 0) --box;
            if (!frames.isEmpty()) rollBack(frames.takeLast());
            i = e + 1;
            continue;
        }
        qsizetype p = lt + 1;
        while (p < n && !space(d[p]) && d[p] != '/' && d[p] != '>') ++p;
        const QByteArrayView tag = localOf(lt + 1, p);
        attrs.clear();
        bool self = false, ended = false;
        while (p < n) {
            const qsizetype with = p;
            while (p < n && space(d[p])) ++p;
            if (p >= n) break;
            if (d[p] == '>') { ++p; ended = true; break; }
            if (d[p] == '/') {
                if (p + 1 < n && d[p + 1] == '>') { self = true; p += 2; ended = true; break; }
                ++p;
                continue;
            }
            const qsizetype an = p;
            while (p < n && !space(d[p]) && d[p] != '=' && d[p] != '>' && d[p] != '/') ++p;
            if (p == an) { ++p; continue; }
            const qsizetype ae = p;
            while (p < n && space(d[p])) ++p;
            if (p < n && d[p] == '=') {
                ++p;
                while (p < n && space(d[p])) ++p;
                if (p >= n || (d[p] != '"' && d[p] != '\'')) break;
                const qsizetype vf = p + 1, close = x.indexOf(d[p], vf);
                if (close < 0) break;
                attrs.push_back({with, close + 1, QByteArrayView(d + an, ae - an), localOf(an, ae), QByteArrayView(d + vf, close - vf)});
                p = close + 1;
            }
        }
        if (!ended) { out.broken = true; break; }
        i = p;
        if (tag == QByteArrayView("txbxContent") && !self) ++box;
        // (one pass over the element's attributes: the namespaces it declares, and whether a part of the file is named beside a link)
        const qsizetype frame = log.size();
        for (const Attr &a : std::as_const(attrs)) {
            ++steps;
            if (a.name.startsWith("xmlns") && (a.name.size() == 5 || a.name[5] == ':')) {
                const QByteArray prefix = a.name.size() == 5 ? QByteArray() : QByteArray(a.name.sliced(6));
                const auto had = bound.constFind(prefix);
                log.push_back({prefix, had == bound.cend() ? QByteArray() : *had, had != bound.cend()});
                bound.insert(prefix, QByteArray(a.value));
            }
        }
        bool beside = false;
        for (Attr &a : attrs) {
            // (a prefix that is bound to another namespace than the relationships' names no relationship)
            if (const qsizetype colon = a.name.indexOf(':'); colon > 0) {
                const auto to = bound.constFind(QByteArray(a.name.first(colon)));
                a.rel = to == bound.cend() || to->endsWith("/officeDocument/2006/relationships") || to->endsWith("/ooxml/officeDocument/relationships");
            }
            if (a.rel && !beside && (a.local == QByteArrayView("embed") || a.local == QByteArrayView("id") || a.local == QByteArrayView("pict")) && a.value.size() <= 64)
                beside = ids.inside.contains(QString::fromLatin1(a.value));
        }
        for (const Attr &a : std::as_const(attrs)) {
            if (!a.rel || a.value.isEmpty() || a.value.size() > 64) continue;
            const QString id = QString::fromLatin1(a.value);
            const bool linkish = a.name.contains(':') && (a.local == QByteArrayView("link") || a.local == QByteArrayView("href"));
            if (ids.files.contains(id)) {
                Ref r;
                r.from = a.with;
                r.to = a.end;
                r.id = id;
                r.inBox = box > 0;
                r.removable = linkish && beside;
                out.refs.push_back(r);
            } else if (linkish && !ids.every.contains(id)) {
                ++out.unresolved;
            }
        }
        if (self) rollBack(frame);
        else frames.push_back(frame);
    }
    out.steps = steps;
    s_linkSteps += steps;
    return out;
}

// The namespace prefixes the first element of a part declares (what a piece of the part, looked through alone, takes its prefixes from).
QHash<QByteArray, QByteArray> rootNamespaces(const QByteArray &part)
{
    QHash<QByteArray, QByteArray> out;
    qsizetype at = 0;
    for (;;) {
        at = part.indexOf('<', at);
        if (at < 0 || at + 1 >= part.size()) return out;
        if (part[at + 1] != '?' && part[at + 1] != '!') break;
        ++at;
    }
    const QByteArray tag = part.mid(at, std::min<qsizetype>(part.indexOf('>', at) - at, 1 << 16));
    static const QRegularExpression declaration(QStringLiteral("\\sxmlns(?::([A-Za-z0-9_.-]+))?\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)')"));
    QRegularExpressionMatchIterator it = declaration.globalMatch(QString::fromLatin1(tag));
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        out.insert(m.captured(1).toLatin1(), (m.captured(2).isEmpty() ? m.captured(3) : m.captured(2)).toLatin1());
    }
    return out;
}

// The bytes without the first `limit` of the links that can be left out alone (those that `pick` takes), and how many went and which Ids.
QByteArray withoutLinks(const QByteArray &x, const QVector<Ref> &refs, int limit, int *done, QSet<QString> *named)
{
    QByteArray out;
    out.reserve(x.size());
    qsizetype at = 0;
    int n = 0;
    for (const Ref &r : refs) {
        if (!r.removable || n >= limit) continue;
        out.append(x.constData() + at, r.from - at);
        at = r.to;
        ++n;
        if (named) named->insert(r.id);
    }
    out.append(x.constData() + at, x.size() - at);
    *done = n;
    return out;
}

// Whether kept XML (or, as a `fragment`, one element or more, with no root of their own) is well formed.
bool wellFormed(const QByteArray &x, bool fragment, bool utf8Text = false)
{
    s_linkBytes += x.size();
    // (a document type with entities is how a few bytes become gigabytes of text: such a part is never edited, and its links are kept and said)
    if (x.contains("<!ENTITY") || x.contains("<!DOCTYPE")) return false;
    QXmlStreamReader r;
    r.setNamespaceProcessing(false);
    if (fragment) r.addData("<x>");
    // (text that was read out of another encoding is given as text, so a declaration of that encoding is not held against its bytes)
    if (utf8Text) r.addData(QString::fromUtf8(x));
    else r.addData(x);
    if (fragment) r.addData("</x>");
    qint64 tokens = 0;
    while (!r.atEnd()) {
        r.readNext();
        ++tokens;
    }
    s_linkTokens += tokens;
    return !r.hasError();
}

// What a piece of kept XML holds of links to files. `whole`: a link at its own level (not inside a text box's content), so what holds it is taken out whole. Else: `editable` links inside it
// that go by leaving one attribute out, `applied` of them with the limit given, `kept` links that stay (they have no data of the picture's own beside them, or the edited bytes would not be well
// formed, or they were not well formed to begin with), and `unresolved` links by an Id the part has no relationship for. `edited` is the bytes with the first `applied` links out, `named` their Ids.
struct Judged {
    bool whole = false;
    int editable = 0, applied = 0, kept = 0, unresolved = 0;
    QByteArray edited;
    QSet<QString> named, every;
};

Judged judgeXml(const QByteArray &xml, const LinkIds &ids, bool fragment, bool allNested, int limit, bool utf8Text = false)
{
    Judged j;
    const Scan s = scanRefs(xml, ids);
    j.unresolved = s.unresolved + (s.broken ? 1 : 0);
    int removable = 0;
    for (const Ref &r : s.refs) {
        if (!allNested && !r.inBox) j.whole = true;
        j.every.insert(r.id);
        removable += r.removable;
    }
    if (j.whole) return j;
    j.kept = int(s.refs.size());
    if (removable == 0 || s.broken) return j;
    const int take = std::min(removable, std::max(limit, 0));
    if (take == 0) {
        j.editable = removable;
        j.kept -= removable;
        return j;
    }
    int done = 0;
    QSet<QString> named;
    const QByteArray edited = withoutLinks(xml, s.refs, take, &done, &named);
    if (!wellFormed(edited, fragment, utf8Text)) return j;
    j.editable = removable;
    j.kept -= removable;
    j.applied = done;
    j.edited = edited;
    j.named = named;
    return j;
}

// What one story holds: the atoms (pictures, objects, text boxes) and the kept blocks, with the edits that take a link out of them.
struct StoryPlan {
    QVector<QPair<int, int>> whole;   // (position, length) of the atoms that are taken out whole
    struct AtomEdit {
        int position, length;
        QTextCharFormat before;
        office::wml::Inline made;
        int links;   // how many links it takes out
    };
    QVector<AtomEdit> atoms;
    struct BlockEdit {
        int position, key;
        QByteArray packed;
        int links;
    };
    QVector<BlockEdit> blocks;
    QSet<QPair<bool, QString>> named;    // the Ids (with whether they are the endnotes') the atoms taken out whole name, and those the edits left out
    QSet<QPair<bool, QString>> seen;     // every Id a link found in the story names (taken out, kept, or waiting for the limit)
    int editable = 0, applied = 0, kept = 0, unresolved = 0, fields = 0;
};

const QRegularExpression &fileFieldPattern()
{
    static const QRegularExpression field(QStringLiteral("(?:w:instr=\"|<w:instrText[^>]*>)\\s*(?:&quot;)?\\s*(?:INCLUDEPICTURE|INCLUDETEXT|LINK|DDEAUTO|DDE)\\b"), QRegularExpression::CaseInsensitiveOption);
    return field;
}

// `limit`: how many more links may be edited out of kept XML; the count of those edited now is taken off it.
StoryPlan planStory(const WriteDoc &doc, QTextDocument *story, int *limit)
{
    using namespace office::wml;
    StoryPlan plan;
    const bool ofBody = story == doc.story();
    const LinkIds body = linkIdsOf(doc.storyRelationships(story), false);
    const LinkIds notes = ofBody ? linkIdsOf(doc.storyRelationships(story, true), false) : LinkIds();
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        const bool endnote = ofBody && doc.endnoteIdOf(b);
        const LinkIds &ids = endnote ? notes : body;
        auto add = [&](const Judged &j) {
            plan.kept += j.kept;
            plan.editable += j.editable;
            plan.applied += j.applied;
            plan.unresolved += j.unresolved;
            *limit -= j.applied;
            for (const QString &id : j.named) plan.named.insert({endnote, id});
            for (const QString &id : j.every) plan.seen.insert({endnote, id});
        };
        // (the elements kept between paragraphs: all of what they hold is inside, so a link in one goes only by an attribute)
        const QTextBlockFormat bf = b.blockFormat();
        for (const int key : {int(prop::RawBefore), int(prop::RawBlock), int(prop::RawAfter)}) {
            if (!bf.hasProperty(key)) continue;
            const QByteArray packed = storedBytes(bf, key);
            if (!mightLink(packed, ids)) continue;
            QVector<Block> blocks = unpackBlocks(packed);
            bool changed = false;
            int links = 0;
            auto one = [&](QByteArray *raw) {
                if (raw->isEmpty() || !mightLink(*raw, ids)) return;
                const Judged j = judgeXml(*raw, ids, true, true, *limit);
                add(j);
                if (j.applied > 0) {
                    *raw = j.edited;
                    changed = true;
                    links += j.applied;
                }
            };
            for (Block &blk : blocks) {
                if (blk.isRaw) one(&blk.raw);
                for (Inline &a : blk.para.content)
                    if (a.kind != Inline::Text) one(&a.raw);
            }
            if (changed) plan.blocks.push_back({b.position(), key, packBlocks(blocks), links});
        }
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            const QTextCharFormat cf = fr.charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const QByteArray packed = storedBytes(cf, prop::Atom);
            bool whole = false;
            if (mightLink(packed, ids)) {
                Inline a = unpackAtom(packed);
                const Judged j = judgeXml(a.raw, ids, true, false, *limit);
                if (j.whole) {
                    whole = true;
                    plan.unresolved += j.unresolved;
                    plan.whole.push_back({fr.position(), fr.length()});
                    for (const QString &id : j.every) {
                        plan.named.insert({endnote, id});
                        plan.seen.insert({endnote, id});
                    }
                } else {
                    add(j);
                    if (j.applied > 0) {
                        a.raw = j.edited;
                        plan.atoms.push_back({fr.position(), fr.length(), cf, a, j.applied});
                    }
                }
            }
            // (a field in a text box, kept whole, is neither counted nor taken out: the row says so)
            if (!whole && packed.contains("instr") && fileFieldPattern().match(QString::fromLatin1(packed.left(1 << 20))).hasMatch()) ++plan.fields;
        }
    }
    return plan;
}

int kindOfOutside(const QString &type, const QString &partType)
{
    const QString t = type.toLower();
    if (t.endsWith(QLatin1String("/attachedtemplate"))) return KindTemplate;
    if (t.contains(QLatin1String("mailmerge"))) return KindMailMerge;
    if (t.endsWith(QLatin1String("/frame")) || t.endsWith(QLatin1String("/frameset"))) return KindFrames;
    if (t.endsWith(QLatin1String("/subdocument"))) return KindSubDocument;
    if (t.contains(QLatin1String("stylesheet"))) return KindStyleSheet;
    if (t.endsWith(QLatin1String("/video")) || t.endsWith(QLatin1String("/audio")) || t.endsWith(QLatin1String("/media"))) return KindMovieOrSound;
    if (partType.contains(QLatin1String("chart"), Qt::CaseInsensitive)) return KindChartData;
    return 0;
}

// The elements of a settings part that attach a template (<attachedTemplate r:id="..."/>) by one of these Ids, without the first `limit` of them: how many there are (`found`), how many went
// (`done`), and which Ids. The template's own styles are in the styles part, which is not touched.
QByteArray withoutTemplateElements(const QByteArray &x, const QSet<QString> &ids, int limit, int *found, int *done, QSet<QString> *named, QSet<QString> *seen)
{
    QByteArray out;
    qsizetype at = 0, from = 0;
    *found = *done = 0;
    for (;;) {
        const qsizetype word = x.indexOf("attachedTemplate", from);
        if (word < 0) break;
        from = word + 16;
        const qsizetype start = x.lastIndexOf('<', word), end = x.indexOf('>', word);
        if (start < 0 || end < 0 || x[end - 1] != '/' || start < at) continue;
        bool plain = true;   // (the name, with an optional prefix, follows the "<" at once)
        for (qsizetype k = start + 1; k < word; ++k)
            if (!(isalnum(uchar(x[k])) || x[k] == '_' || x[k] == '-' || x[k] == '.' || x[k] == ':')) plain = false;
        if (!plain || x.mid(word + 16, end - word - 16).contains('<')) continue;
        static const QRegularExpression idAttribute(QStringLiteral("\\s(?:[A-Za-z0-9_.-]+:)?id\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)')"));
        const QRegularExpressionMatch m = idAttribute.match(QString::fromLatin1(x.mid(word, end - word)));
        if (!m.hasMatch()) continue;
        const QString id = m.captured(1).isEmpty() ? m.captured(2) : m.captured(1);
        if (!ids.contains(id)) continue;
        ++*found;
        seen->insert(id);
        if (*done >= limit) continue;
        out.append(x.constData() + at, start - at);
        at = end + 1;
        ++*done;
        named->insert(id);
    }
    out.append(x.constData() + at, x.size() - at);
    return out;
}

// What a part kept whole holds.
struct PartPlan {
    QString name, contentType;
    QVector<office::Relationship> rels;
    Judged judged;
    int fields = 0, leftOut = 0;   // (links in the part's relationships that no attribute of it was found to name)
    int kinds = 0;                 // (the kinds of link kept that the row names)
    bool tooLarge = false;
    QByteArray encodedBack;   // the bytes to put in the file, in the encoding the part came in (when it was read out of another)
    bool reencode = false;
};

// A kept part's text as UTF-8 for the look through it: the part's own bytes when they are one byte to a letter, the text of a part in UTF-16 (a byte-order mark, or the bytes of "<?" with a
// zero beside each), and nothing for another encoding that has zeros in its first bytes (UTF-32), which is said to be not looked through. `back` puts the edited text into the first encoding again.
struct KeptText {
    QByteArray utf8;
    bool readable = true, converted = false, mark = false;   // (a byte-order mark is not part of the text: it is put back first)
    QStringConverter::Encoding encoding = QStringConverter::Utf8;
    QByteArray back(const QByteArray &edited) const
    {
        QStringEncoder enc(encoding, QStringConverter::Flag::Stateless);
        return QByteArray(enc(QString(mark ? QString(QChar(0xFEFF)) : QString()) + QString::fromUtf8(edited)));
    }
};

KeptText keptText(const QByteArray &data)
{
    KeptText t;
    const auto b = [&](int i) { return i < data.size() ? uchar(data[i]) : uchar(0xFF); };
    if (data.size() >= 4 && ((b(0) == 0xFF && b(1) == 0xFE && b(2) == 0 && b(3) == 0) || (b(0) == 0 && b(1) == 0 && b(2) == 0xFE && b(3) == 0xFF))) {
        t.readable = false;
        return t;
    }
    if (data.size() >= 2 && b(0) == 0xFF && b(1) == 0xFE) t.encoding = QStringConverter::Utf16LE, t.converted = true;
    else if (data.size() >= 2 && b(0) == 0xFE && b(1) == 0xFF) t.encoding = QStringConverter::Utf16BE, t.converted = true;
    else if (data.size() >= 4 && b(0) == '<' && b(1) == 0 && b(2) != 0 && b(3) == 0) t.encoding = QStringConverter::Utf16LE, t.converted = true;
    else if (data.size() >= 4 && b(0) == 0 && b(1) == '<' && b(2) == 0 && b(3) != 0) t.encoding = QStringConverter::Utf16BE, t.converted = true;
    else if (data.left(4).contains('\0')) {
        t.readable = false;
        return t;
    }
    if (!t.converted) {
        t.utf8 = data;
        return t;
    }
    QStringDecoder dec(t.encoding, QStringConverter::Flag::Stateless);
    QString text = dec(data);
    t.mark = (b(0) == 0xFF && b(1) == 0xFE) || (b(0) == 0xFE && b(1) == 0xFF);
    if (text.startsWith(QChar(0xFEFF))) text.remove(0, 1);
    if (dec.hasError()) {
        t.readable = false;
        return t;
    }
    t.utf8 = text.toUtf8();
    return t;
}

QVector<PartPlan> planParts(const WriteDoc &doc, int *limit)
{
    QVector<PartPlan> out;
    for (const WriteDoc::KeptXmlPart &part : doc.keptXmlParts()) {
        PartPlan plan;
        plan.name = part.name;
        plan.contentType = part.contentType;
        plan.rels = part.relationships;
        if (part.data.size() > kMostLinkBytes) {
            plan.tooLarge = true;
            plan.judged.unresolved = 1;
            out.push_back(plan);
            continue;
        }
        const KeptText text = keptText(part.data);
        if (!text.readable) {
            plan.judged.unresolved = 1;
            out.push_back(plan);
            continue;
        }
        const LinkIds ids = linkIdsOf(part.relationships, true);
        // (the attached template goes by its element, and the file's link to it with the save)
        QByteArray work = text.utf8;
        QSet<QString> templateIds, templateNamed, templateSeen;
        for (const office::Relationship &r : part.relationships)
            if (r.external && r.type.endsWith(QLatin1String("/attachedTemplate"), Qt::CaseInsensitive)) templateIds.insert(r.id);
        int templateFound = 0, templateDone = 0;
        if (!templateIds.isEmpty() && work.contains("attachedTemplate")) {
            const QByteArray edited = withoutTemplateElements(work, templateIds, std::max(*limit, 0), &templateFound, &templateDone, &templateNamed, &templateSeen);
            if (templateDone > 0 && wellFormed(edited, false, text.converted)) {
                work = edited;
                for (const QString &id : std::as_const(templateIds))
                    if (namesAnId(work, {id})) templateNamed.remove(id);   // (still named by another attribute: its link stays)
            } else {
                if (templateDone > 0) {   // (the edit would not be well formed: the link is kept, and said)
                    templateFound = 0;
                    templateSeen.clear();
                }
                templateDone = 0;
                templateNamed.clear();
            }
        }
        if (mightLink(work, ids)) plan.judged = judgeXml(work, ids, false, true, *limit - templateDone, text.converted);
        plan.judged.editable += templateFound;
        if (templateDone > 0) {
            plan.judged.applied += templateDone;
            plan.judged.named += templateNamed;
            if (plan.judged.edited.isEmpty()) plan.judged.edited = work;
        }
        *limit -= plan.judged.applied;
        if (plan.judged.applied > 0 && text.converted) {
            plan.reencode = true;
            plan.encodedBack = text.back(plan.judged.edited);
        }
        // (the part's own list of relationships is the ground truth for whether it points outside the file: an address that no attribute found was seen to name is said, not passed)
        for (const office::Relationship &r : part.relationships) {
            if (!r.external || r.type.endsWith(QLatin1String("/hyperlink")) || plan.judged.named.contains(r.id)) continue;
            if (templateSeen.contains(r.id)) continue;
            plan.kinds |= kindOfOutside(r.type, part.contentType);
            if (!plan.judged.every.contains(r.id)) ++plan.leftOut;
        }
        if (text.utf8.contains("connectString")) plan.kinds |= KindMailMerge;   // (a data source named in the settings' own words, with no relationship)
        if (text.utf8.contains("instr")) {
            QRegularExpressionMatchIterator it = fileFieldPattern().globalMatch(QString::fromUtf8(text.utf8));
            while (it.hasNext()) {
                it.next();
                ++plan.fields;
            }
        }
        if (plan.judged.editable || plan.judged.kept || plan.judged.unresolved || plan.fields || plan.leftOut || plan.kinds) out.push_back(plan);
    }
    return out;
}

// The first word of a field the text holds as a group (a field in a header, a footer, or the footnotes keeps its runs whole, so it has no name of its own): the word of its instruction.
QString groupFieldWord(const QByteArray &raw)
{
    // (the instruction may be split over runs: the pieces of the first field's instruction are joined before its first word is read)
    static const QRegularExpression attribute(QStringLiteral("w:instr=\"([^\"]*)\""));
    static const QRegularExpression piece(QStringLiteral("<w:instrText[^>]*>([^<]*)</w:instrText>"));
    static const QRegularExpression word(QStringLiteral("^\\s*(?:&quot;)?\\s*([A-Za-z]+)"));
    const QString text = QString::fromUtf8(raw.left(1 << 16));
    QString instruction;
    const QRegularExpressionMatch a = attribute.match(text), p = piece.match(text);
    if (a.hasMatch() && (!p.hasMatch() || a.capturedStart() < p.capturedStart())) {
        instruction = a.captured(1);
    } else {
        QRegularExpressionMatchIterator it = piece.globalMatch(text);
        int depth = 0;
        static const QRegularExpression fldChar(QStringLiteral("<w:fldChar[^>]*w:fldCharType=\"(begin|end)\""));
        // (only the pieces before a second field begins or the first one ends)
        QRegularExpressionMatchIterator marks = fldChar.globalMatch(text);
        qsizetype stop = text.size();
        while (marks.hasNext()) {
            const QRegularExpressionMatch m = marks.next();
            if (m.captured(1) == QLatin1String("begin") && ++depth == 2) { stop = m.capturedStart(); break; }
            if (m.captured(1) == QLatin1String("end")) { stop = m.capturedStart(); break; }
        }
        while (it.hasNext()) {
            const QRegularExpressionMatch m = it.next();
            if (m.capturedStart() >= stop) break;
            instruction += m.captured(1);
        }
    }
    const QRegularExpressionMatch m = word.match(instruction);
    return m.hasMatch() ? m.captured(1).toUpper() : QString();
}

bool isFileLinkField(const FieldPlace &f)
{
    static const QSet<QString> names = {QStringLiteral("INCLUDEPICTURE"), QStringLiteral("INCLUDETEXT"), QStringLiteral("LINK"), QStringLiteral("DDE"), QStringLiteral("DDEAUTO")};
    if (f.name.isEmpty() && f.form == FieldPlace::Group) return names.contains(groupFieldWord(f.groupRaw));
    return names.contains(f.name);
}

} // namespace

int personalInfoCount(WriteDoc &doc)
{
    const office::PackageProps p = doc.packageProps();
    int n = 0;
    for (const QString &s : {p.creator, p.lastModifiedBy, p.company, p.manager})
        if (!s.isEmpty()) ++n;
    return n;
}

int removePersonalInfo(WriteDoc &doc)
{
    office::PackageProps p = doc.packageProps();
    const int n = personalInfoCount(doc);
    if (n == 0) return 0;
    p.creator.clear();
    p.lastModifiedBy.clear();
    p.company.clear();
    p.manager.clear();
    doc.setPackageProps(p);
    return n;
}

// Every story that holds text a person reads: the body, and the headers, footers, and footnotes (made editable if they were not).
static QVector<QTextDocument *> allStories(WriteDoc &doc)
{
    QVector<QTextDocument *> stories = {doc.story()};
    for (QTextDocument *s : doc.storiesWithChanges(true))
        if (s && !stories.contains(s)) stories.push_back(s);
    return stories;
}

int hiddenTextCount(WriteDoc &doc)
{
    int n = 0;
    for (const QTextDocument *story : allStories(doc)) n += int(hiddenRuns(story).size());
    return n;
}

int removeHiddenText(WriteDoc &doc)
{
    int n = 0;
    for (QTextDocument *story : allStories(doc)) {
        const QVector<QPair<int, int>> runs = hiddenRuns(story);
        if (runs.isEmpty()) continue;
        QTextCursor c(story);
        c.beginEditBlock();
        for (qsizetype i = runs.size() - 1; i >= 0; --i) {
            c.setPosition(runs[i].first);
            c.setPosition(runs[i].first + runs[i].second, QTextCursor::KeepAnchor);
            c.removeSelectedText();
        }
        c.endEditBlock();
        n += int(runs.size());
    }
    return n;
}

int headerFooterCount(WriteDoc &doc)
{
    int n = 0;
    QSet<const QTextDocument *> seen;
    for (int s = 0; s < doc.sectionCount(); ++s)
        for (int footer = 0; footer < 2; ++footer)
            for (int slot = 0; slot < 3; ++slot) {
                if (!doc.hasOwnHeaderFooter(s, footer, slot)) continue;
                const QTextDocument *story = doc.headerFooterStory(s, footer, slot);
                if (!story || seen.contains(story)) continue;
                seen.insert(story);
                if (!story->toPlainText().trimmed().isEmpty() || story->characterCount() > 1) ++n;
            }
    return n;
}

int clearHeadersAndFooters(WriteDoc &doc)
{
    int n = 0;
    QSet<const QTextDocument *> seen;
    for (int s = 0; s < doc.sectionCount(); ++s)
        for (int footer = 0; footer < 2; ++footer)
            for (int slot = 0; slot < 3; ++slot) {
                if (!doc.hasOwnHeaderFooter(s, footer, slot)) continue;
                QTextDocument *story = doc.editableHeaderFooter(s, footer, slot);
                if (!story || seen.contains(story)) continue;
                seen.insert(story);
                if (story->characterCount() <= 1) continue;
                QTextCursor c(story);
                c.select(QTextCursor::Document);
                c.removeSelectedText();
                ++n;
            }
    return n;
}

int customXmlCount(WriteDoc &doc)
{
    static const QRegularExpression item(QStringLiteral("^/customXml/item[0-9]*\\.xml$"), QRegularExpression::CaseInsensitiveOption);
    int n = 0;
    for (const QString &name : doc.packagePartNames())
        if (item.match(name).hasMatch()) ++n;
    return n;
}

int removeCustomXml(WriteDoc &doc)
{
    const int n = customXmlCount(doc);
    for (const QString &name : doc.packagePartNames())
        if (name.startsWith(QLatin1String("/customXml/"), Qt::CaseInsensitive)) doc.removePackagePart(name);
    if (n > 0) doc.setModified(true);
    return n;
}

int mergeExclusionCount(WriteDoc &doc)
{
    const mail::State *state = mail::stateIfAny(doc);
    return state ? int(state->exclusions.keys.size() + state->exclusions.places.size()) : 0;
}

int removeMergeExclusions(WriteDoc &doc)
{
    const int n = mergeExclusionCount(doc);
    if (n == 0) return 0;
    mail::State &state = mail::stateOf(doc);
    state.exclusions = MergeExclusions();
    state.exclusionsNote.clear();
    state.exclusionsUnread.clear();
    state.changed = true;   // (written again without the part)
    doc.setModified(true);
    return n;
}

qint64 externalLinkBytesScannedForTest() { return s_linkBytes; }
qint64 externalLinkAttributeStepsForTest() { return s_linkSteps; }
qint64 externalLinkXmlTokensForTest() { return s_linkTokens; }
void resetExternalLinkBytesScannedForTest() { s_linkBytes = 0; s_linkSteps = 0; s_linkTokens = 0; }
void setMostExternalLinkEditsForTest(int most) { s_mostLinkEdits = most; }

namespace {
int editLimit() { return s_mostLinkEdits > 0 ? int(s_mostLinkEdits) : kMostLinkEdits; }
}

ExternalLinkScan scanExternalLinks(WriteDoc &doc)
{
    ExternalLinkScan out;
    int limit = editLimit(), editable = 0, applied = 0;
    for (QTextDocument *story : allStories(doc)) {
        const StoryPlan plan = planStory(doc, story, &limit);
        for (const auto &run : plan.whole) out.count += run.second;
        out.count += plan.editable;
        editable += plan.editable;
        applied += plan.applied;
        out.unread.unresolved += plan.unresolved;
        out.unread.unreachable += plan.kept + plan.fields;   // (a link or a field kept inside a text box)
        for (const FieldPlace &f : scanStory(story, false).fields) {
            if (isFileLinkField(f)) ++out.count;
            else if (f.name.isEmpty() && f.form == FieldPlace::Group && fileFieldPattern().match(QString::fromUtf8(f.groupRaw.left(1 << 16))).hasMatch()) ++out.unread.unreachable;   // (a link kept inside another field)
        }
        // (the story's own relationships are the ground truth: an address no link found names stays in the file, and is said; one the text let go of goes with the save)
        for (const bool endnote : {false, true}) {
            if (endnote && story != doc.story()) break;
            for (const office::Relationship &r : doc.storyRelationships(story, endnote)) {
                if (!r.external || r.type.endsWith(QLatin1String("/hyperlink")) || plan.seen.contains({endnote, r.id}) || doc.storyLinkForgotten(story, r.id, endnote)) continue;
                out.unread.kinds |= kindOfOutside(r.type, QString());
                ++out.unread.unreachable;
            }
        }
    }
    // (the parts the program keeps whole: the comments, the drawings, and so on)
    for (const PartPlan &p : planParts(doc, &limit)) {
        out.count += p.judged.editable;
        editable += p.judged.editable;
        applied += p.judged.applied;
        out.unread.unresolved += p.judged.unresolved;
        out.unread.unreachable += p.judged.kept + p.fields + p.leftOut;
        out.unread.kinds |= p.kinds;
    }
    out.unread.pastLimit = editable - applied;   // (what one Remove All leaves for the next)
    return out;
}

int externalLinkCount(WriteDoc &doc) { return scanExternalLinks(doc).count; }

ExternalLinkUnread externalLinkUnread(WriteDoc &doc) { return scanExternalLinks(doc).unread; }

int removeExternalLinks(WriteDoc &doc)
{
    int n = 0, limit = editLimit();
    const WriteDoc::StepGroup group(doc);
    for (QTextDocument *story : allStories(doc)) {
        // (the fields first: each becomes its result; the last in the story goes first, so the places of the rest do not move; the count of fields found is the most turns)
        const int most = int(scanStory(story, false).fields.size());
        for (int turn = 0; turn < most; ++turn) {
            const StoryScan scan = scanStory(story, false);
            int last = -1;
            for (int i = 0; i < scan.fields.size(); ++i)
                if (isFileLinkField(scan.fields[i]) && (last < 0 || scan.fields[i].begin > scan.fields[last].begin)) last = i;
            if (last < 0) break;
            QTextCursor c(story);
            c.setPosition(scan.fields[last].begin);
            c.setPosition(std::min(scan.fields[last].end + 1, story->characterCount() - 1), QTextCursor::KeepAnchor);
            if (unlinkFieldsIn(doc, story, c) == 0) break;
            ++n;
        }
        const StoryPlan plan = planStory(doc, story, &limit);
        QTextCursor c(story);
        c.beginEditBlock();
        // (a link inside a text box goes by the attribute that names it: the edits keep the length of the text, so they go before the atoms taken out whole)
        QSet<QPair<bool, QString>> edited;
        bool drawnChanged = false;
        for (const StoryPlan::AtomEdit &e : plan.atoms) {
            QTextCharFormat f = e.before;
            const bool drawn = f.intProperty(flow::fp::AnchorId) > 0;
            f.clearProperty(flow::fp::AnchorId);
            f.clearProperty(tp::InlineObject);
            setStored(f, prop::Atom, packAtom(e.made));
            if (drawn && !doc.makeDrawnFormat(e.made, &f)) continue;
            c.setPosition(e.position);
            c.setPosition(e.position + e.length, QTextCursor::KeepAnchor);
            c.setCharFormat(f);
            drawnChanged = true;
            n += e.links;
        }
        for (const StoryPlan::BlockEdit &e : plan.blocks) {
            c.setPosition(e.position);
            QTextBlockFormat f = c.blockFormat();
            setStored(f, e.key, e.packed);
            c.setBlockFormat(f);
            n += e.links;
        }
        for (qsizetype i = plan.whole.size() - 1; i >= 0; --i) {
            c.setPosition(plan.whole[i].first);
            c.setPosition(plan.whole[i].first + plan.whole[i].second, QTextCursor::KeepAnchor);
            c.removeSelectedText();
            n += plan.whole[i].second;
        }
        c.endEditBlock();
        if (drawnChanged) doc.keepObjectsForTheHistory();
        // (the file's own link to the address goes when nothing names it: see WriteDoc::forgetExternalLink)
        for (const auto &[endnote, id] : std::as_const(plan.named)) doc.forgetStoryLink(story, id, endnote);
    }
    // (the parts the program keeps whole are edited in their bytes; that is no step of the history, as taking out the custom XML is none)
    const QString comments = doc.mainRelatedPart(QStringLiteral("comments"));
    for (const PartPlan &p : planParts(doc, &limit)) {
        if (p.judged.applied == 0) continue;
        if (!comments.isEmpty() && p.name.compare(comments, Qt::CaseInsensitive) == 0) {
            const LinkIds ids = linkIdsOf(p.rels, true);
            // (the comments in the order of the part, each with what is left of the limit: the same links go from every copy of them)
            int left = p.judged.applied;
            const QHash<QByteArray, QByteArray> root = rootNamespaces(doc.partData(p.name));
            editKeptCommentXml(doc, [&](const QByteArray &xml) {
                if (left <= 0) return xml;
                int done = 0;
                const QByteArray edited = withoutLinks(xml, scanRefs(xml, ids, &root).refs, left, &done, nullptr);
                left -= done;
                return edited;
            });
        }
        doc.putPackagePart(p.name, p.contentType, p.reencode ? p.encodedBack : p.judged.edited);
        // (an address goes from the relationships when no attribute of the part names it any more)
        const Scan left = scanRefs(p.judged.edited, linkIdsOf(p.rels, true));
        QSet<QString> gone = p.judged.named;
        for (const Ref &r : left.refs) gone.remove(r.id);
        doc.dropPartLinks(p.name, gone);
        n += p.judged.applied;
    }
    if (n > 0) doc.setModified(true);
    return n;
}

} // namespace jp::write
