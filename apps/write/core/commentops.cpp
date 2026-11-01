// Comments: reading them from the parts of a file and the marks in the text, making them, and writing the parts back.

#include "core/commentops.h"

#include "core/editops.h"
#include "core/reviewops.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "text/textprops.h"
#include "wml/wml.h"
#include "wml/xmltree.h"

#include <QDataStream>
#include <QMap>
#include <QSet>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <algorithm>

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

namespace {

constexpr char kCommentsRel[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/comments";
constexpr char kCommentsType[] = "application/vnd.openxmlformats-officedocument.wordprocessingml.comments+xml";
constexpr char kExtendedRel[] = "http://schemas.microsoft.com/office/2011/relationships/commentsExtended";
constexpr char kExtendedType[] = "application/vnd.openxmlformats-officedocument.wordprocessingml.commentsExtended+xml";
constexpr char kIdsRel[] = "http://schemas.microsoft.com/office/2016/09/relationships/commentsIds";
constexpr char kIdsType[] = "application/vnd.openxmlformats-officedocument.wordprocessingml.commentsIds+xml";
constexpr char kPeopleRel[] = "http://schemas.microsoft.com/office/2011/relationships/people";
constexpr char kPeopleType[] = "application/vnd.openxmlformats-officedocument.wordprocessingml.people+xml";
constexpr char kW14[] = "http://schemas.microsoft.com/office/word/2010/wordml";
constexpr char kW15[] = "http://schemas.microsoft.com/office/word/2012/wordml";
constexpr char kW16cid[] = "http://schemas.microsoft.com/office/word/2016/wordml/cid";
constexpr char kMc[] = "http://schemas.openxmlformats.org/markup-compatibility/2006";
constexpr char kWNs[] = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
constexpr char kXmlHead[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n";

// ---- the parts of a file

struct PartComment {
    QString author, initials, paraId;
    QDateTime date;
    QString text;
    QByteArray xml;   // the element as written
    int order = 0;
};

// What the file's comment parts say, read once.
struct CommentSource {
    bool read = false, opened = false;
    QSet<int> idsAtOpen;   // the comments the text referred to when the document was opened
    QMap<int, PartComment> byId;
    QHash<QString, QString> parentOfParaId;   // a comment's last paragraph's id -> the id of the one it replies to
    QHash<QString, bool> doneOfParaId;
    QHash<QString, QByteArray> extensionOfParaId;   // the commentEx elements as written
    QHash<QString, QByteArray> idsOfParaId;          // the commentId elements as written
    QVector<int> order;
    QByteArray commentsRoot, extensionRoot, idsRoot, peopleRoot;   // the roots' start tags as written
    QByteArray peopleBytes;
    QStringList people;
    QString commentsName, extensionName, idsName, peopleName;
    // The comments the text has, for the text as it was last looked at (the story's revision).
    int madeAtRevision = -1;
    QVector<Comment> made;
};

QByteArray startTagOfRoot(const QByteArray &xml, QByteArray *closing = nullptr)
{
    qsizetype at = 0;
    for (;;) {
        at = xml.indexOf('<', at);
        if (at < 0 || at + 1 >= xml.size()) return QByteArray();
        if (xml[at + 1] == '?') {
            at = xml.indexOf("?>", at);
            if (at < 0) return QByteArray();
            at += 2;
            continue;
        }
        if (xml[at + 1] == '!') {
            at = xml.indexOf("-->", at);
            if (at < 0) return QByteArray();
            at += 3;
            continue;
        }
        break;
    }
    const qsizetype end = xml.indexOf('>', at);
    if (end < 0) return QByteArray();
    QByteArray tag = xml.mid(at, end - at + 1);
    qsizetype e = 1;
    while (e < tag.size() && tag[e] != ' ' && tag[e] != '/' && tag[e] != '>' && tag[e] != '\r' && tag[e] != '\n' && tag[e] != '\t') ++e;
    if (closing) *closing = "</" + tag.mid(1, e - 1) + ">";
    if (tag.endsWith("/>")) {
        tag.chop(2);
        tag = tag.trimmed() + ">";
    }
    return tag;
}

// A root's start tag that declares the prefix (and, for the comments, ignores the extension namespace the way a reader that doesn't know it should).
QByteArray declaring(QByteArray tag, const char *prefix, const char *uri)
{
    if (tag.contains(QByteArray(" xmlns:") + prefix + "=")) return tag;
    const qsizetype end = tag.lastIndexOf('>');
    if (end < 0) return tag;
    return tag.left(end) + " xmlns:" + prefix + "=\"" + uri + "\"" + tag.mid(end);
}

QByteArray ignoring(QByteArray tag, const char *prefix)
{
    tag = declaring(tag, "mc", kMc);
    const qsizetype at = tag.indexOf("mc:Ignorable=\"");
    if (at < 0) {
        const qsizetype end = tag.lastIndexOf('>');
        return tag.left(end) + " mc:Ignorable=\"" + prefix + "\"" + tag.mid(end);
    }
    const qsizetype from = at + 14, to = tag.indexOf('"', from);
    if (to < 0) return tag;
    const QList<QByteArray> have = tag.mid(from, to - from).split(' ');
    if (have.contains(prefix)) return tag;
    return tag.left(to) + " " + prefix + tag.mid(to);
}

QDateTime parseDate(const QString &s)
{
    if (s.isEmpty()) return QDateTime();
    QDateTime d = QDateTime::fromString(s, Qt::ISODate);
    if (d.isValid() && d.timeSpec() == Qt::LocalTime) d.setTimeSpec(Qt::UTC);
    return d;
}

// The text of a paragraph element: its words, a tab, and a line break.
QString paragraphText(const XmlTree &t, int p)
{
    QString out;
    QVector<int> stack;
    for (int c = t.firstChild(p); c >= 0; c = t.next(c)) stack.push_front(c);
    int guard = 0;
    while (!stack.isEmpty() && ++guard < 200000) {
        const int n = stack.takeFirst();
        if (t.ns(n) != NsW) continue;
        if (t.is(n, "t")) out += t.text(n);
        else if (t.is(n, "tab")) out += QLatin1Char('\t');
        else if (t.is(n, "br") || t.is(n, "cr")) out += QLatin1Char('\n');
        else if (t.is(n, "pPr") || t.is(n, "rPr")) continue;
        else {
            QVector<int> kids;
            for (int c = t.firstChild(n); c >= 0; c = t.next(c)) kids.push_back(c);
            for (int i = int(kids.size()) - 1; i >= 0; --i) stack.push_front(kids[i]);
        }
    }
    return out;
}

void readSource(WriteDoc &doc, CommentSource *src)
{
    src->read = true;
    src->commentsName = doc.mainRelatedPart(QStringLiteral("comments"));
    src->extensionName = doc.mainRelatedPart(QStringLiteral("commentsExtended"));
    src->idsName = doc.mainRelatedPart(QStringLiteral("commentsIds"));
    src->peopleName = doc.mainRelatedPart(QStringLiteral("people"));
    const QByteArray comments = src->commentsName.isEmpty() ? QByteArray() : doc.partData(src->commentsName);
    src->commentsRoot = startTagOfRoot(comments);
    if (!comments.isEmpty()) {
        XmlTree t;
        if (t.parse(comments, nullptr)) {
            int order = 0;
            for (int c = t.firstChild(t.root()); c >= 0 && src->byId.size() < kMostComments; c = t.next(c)) {
                if (!t.is(c, "comment")) continue;
                bool ok = false;
                const int id = t.attr(c, "id").toInt(&ok);
                if (!ok || src->byId.contains(id)) continue;
                PartComment pc;
                pc.author = t.attr(c, "author");
                pc.initials = t.attr(c, "initials");
                pc.date = parseDate(t.attr(c, "date"));
                pc.xml = t.raw(c);
                pc.order = order++;
                QStringList paragraphs;
                for (int p = t.firstChild(c); p >= 0; p = t.next(p)) {
                    if (!t.is(p, "p")) continue;
                    paragraphs << paragraphText(t, p);
                    const QString pid = t.attrAny(p, "paraId");
                    if (!pid.isEmpty()) pc.paraId = pid;
                }
                pc.text = paragraphs.join(QLatin1Char('\n'));
                src->byId.insert(id, pc);
                src->order.push_back(id);
            }
        }
    }
    const QByteArray ext = src->extensionName.isEmpty() ? QByteArray() : doc.partData(src->extensionName);
    src->extensionRoot = startTagOfRoot(ext);
    if (!ext.isEmpty()) {
        XmlTree t;
        if (t.parse(ext, nullptr))
            for (int c = t.firstChild(t.root()); c >= 0; c = t.next(c)) {
                if (!t.is(c, "commentEx", t.ns(c))) continue;
                const QString pid = t.attrAny(c, "paraId");
                if (pid.isEmpty()) continue;
                src->parentOfParaId.insert(pid, t.attrAny(c, "paraIdParent"));
                const QString done = t.attrAny(c, "done");
                src->doneOfParaId.insert(pid, done == QLatin1String("1") || done == QLatin1String("true"));
                src->extensionOfParaId.insert(pid, t.raw(c));
            }
    }
    const QByteArray ids = src->idsName.isEmpty() ? QByteArray() : doc.partData(src->idsName);
    src->idsRoot = startTagOfRoot(ids);
    if (!ids.isEmpty()) {
        XmlTree t;
        if (t.parse(ids, nullptr))
            for (int c = t.firstChild(t.root()); c >= 0; c = t.next(c)) {
                const QString pid = t.attrAny(c, "paraId");
                if (!pid.isEmpty()) src->idsOfParaId.insert(pid, t.raw(c));
            }
    }
    src->peopleBytes = src->peopleName.isEmpty() ? QByteArray() : doc.partData(src->peopleName);
    src->peopleRoot = startTagOfRoot(src->peopleBytes);
    if (!src->peopleBytes.isEmpty()) {
        XmlTree t;
        if (t.parse(src->peopleBytes, nullptr))
            for (int c = t.firstChild(t.root()); c >= 0; c = t.next(c))
                if (!t.attrAny(c, "author").isEmpty()) src->people << t.attrAny(c, "author");
    }
}

// ---- the marks in the text

struct Anchor {
    int id = -1;
    int start = -1, end = -1, reference = -1;
    QByteArray look;   // what the mark of the reference holds, if anything
};

int idOfRaw(const QByteArray &raw)
{
    const qsizetype at = raw.indexOf(":id=\"");
    if (at < 0) return -1;
    const qsizetype to = raw.indexOf('"', at + 5);
    bool ok = false;
    const int v = raw.mid(at + 5, to - at - 5).toInt(&ok);
    return ok ? v : -1;
}

QVector<Anchor> anchorsOf(const QTextDocument *story, QVector<int> *order = nullptr)
{
    QMap<int, Anchor> byId;
    QVector<int> seen;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        if (b.blockFormat().hasProperty(prop::EndnoteBlock)) continue;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const QTextCharFormat cf = f.charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const Inline a = unpackAtom(storedBytes(cf, prop::Atom));
            const bool start = a.kind == Inline::ParaAtom && a.tag == QLatin1String("commentRangeStart");
            const bool end = a.kind == Inline::ParaAtom && a.tag == QLatin1String("commentRangeEnd");
            const bool ref = a.kind == Inline::RunAtom && a.tag == QLatin1String("commentReference");
            if (!start && !end && !ref) continue;
            const int id = start || end ? (a.key.isEmpty() ? idOfRaw(a.raw) : a.key.toInt()) : idOfRaw(a.raw);
            if (id < 0) continue;
            for (int k = 0; k < int(f.length()) && byId.size() < kMostComments + 1; ++k) {
                Anchor &an = byId[id];
                an.id = id;
                if (start && an.start < 0) an.start = f.position() + k;
                else if (end && an.end < 0) an.end = f.position() + k;
                else if (ref && an.reference < 0) {
                    an.reference = f.position() + k;
                    an.look = a.look;
                    seen.push_back(id);
                }
            }
        }
    }
    if (order) *order = seen;
    QVector<Anchor> out;
    for (const int id : std::as_const(seen)) out.push_back(byId.value(id));
    return out;
}

// ---- what a mark of a reference holds

QByteArray packComment(const Comment &c)
{
    QByteArray out;
    QDataStream s(&out, QIODevice::WriteOnly);
    s.setVersion(QDataStream::Qt_6_0);
    s << qint8(1) << c.author << c.initials << qint64(c.date.isValid() ? c.date.toMSecsSinceEpoch() : -1) << c.text << c.paraId << qint32(c.parent) << c.done;
    return out;
}

bool unpackComment(const QByteArray &packed, Comment *c)
{
    if (packed.isEmpty()) return false;
    QDataStream s(packed);
    s.setVersion(QDataStream::Qt_6_0);
    qint8 version = 0;
    qint64 when = -1;
    qint32 parent = -1;
    s >> version >> c->author >> c->initials >> when >> c->text >> c->paraId >> parent >> c->done;
    if (s.status() != QDataStream::Ok || version != 1) return false;
    c->date = when < 0 ? QDateTime() : QDateTime::fromMSecsSinceEpoch(when, Qt::UTC);
    c->parent = parent;
    c->changed = true;
    return true;
}

CommentSource &sourceOf(WriteDoc &doc)
{
    if (!doc.commentCache) doc.commentCache = std::make_shared<CommentSource>();
    CommentSource &src = *static_cast<CommentSource *>(doc.commentCache.get());
    if (!src.read) {
        readSource(doc, &src);
        if (!src.opened) {
            // (Not noted when the document was made: noted the first time they are asked for.)
            for (const Anchor &a : anchorsOf(doc.story())) src.idsAtOpen.insert(a.id);
            src.opened = true;
        }
    }
    return src;
}

QString newParaId(const QSet<QString> &taken, int seed)
{
    quint32 v = (quint32(seed) * 2654435761u) & 0x7FFFFFFFu;
    for (;; ++v) {
        v &= 0x7FFFFFFFu;
        if (v == 0) v = 1;
        const QString id = QStringLiteral("%1").arg(v, 8, 16, QLatin1Char('0')).toUpper();
        if (!taken.contains(id)) return id;
    }
}

QSet<QString> paraIdsIn(WriteDoc &doc)
{
    CommentSource &src = sourceOf(doc);
    QSet<QString> out;
    for (const PartComment &pc : std::as_const(src.byId)) out.insert(pc.paraId);
    for (const Anchor &a : anchorsOf(doc.story())) {
        Comment c;
        if (unpackComment(a.look, &c)) out.insert(c.paraId);
    }
    return out;
}

} // namespace

void rememberCommentsAtOpen(WriteDoc &doc)
{
    if (!doc.commentCache) doc.commentCache = std::make_shared<CommentSource>();
    CommentSource &src = *static_cast<CommentSource *>(doc.commentCache.get());
    for (const Anchor &a : anchorsOf(doc.story())) src.idsAtOpen.insert(a.id);
    src.opened = true;
}

QString WriteDoc::mainRelatedPart(const QString &relationshipTypeEnd) const
{
    for (const Relationship &r : m_wml.mainRelationships)
        if (!r.external && (r.type.endsWith(QLatin1Char('/') + relationshipTypeEnd))) return r.target;
    return QString();
}

QVector<Comment> comments(WriteDoc &doc)
{
    CommentSource &src = sourceOf(doc);
    if (src.madeAtRevision == doc.story()->revision()) return src.made;
    QVector<Comment> out;
    QVector<int> order;
    const QVector<Anchor> anchors = anchorsOf(doc.story(), &order);
    QHash<QString, int> idOfPara;
    for (auto it = src.byId.cbegin(); it != src.byId.cend(); ++it)
        if (!it->paraId.isEmpty()) idOfPara.insert(it->paraId, it.key());
    for (const Anchor &a : anchors) {
        if (out.size() >= kMostComments) break;
        Comment c;
        c.id = a.id;
        c.from = a.start >= 0 ? a.start : a.reference;
        c.to = a.end >= 0 ? a.end : c.from;
        if (c.to < c.from) c.to = c.from;
        c.reference = a.reference;
        if (!unpackComment(a.look, &c)) {
            const auto it = src.byId.constFind(a.id);
            if (it == src.byId.constEnd()) continue;   // (a reference to a comment the file lacks)
            c.author = it->author;
            c.initials = it->initials;
            c.date = it->date;
            c.text = it->text;
            c.paraId = it->paraId;
            const QString parentPara = src.parentOfParaId.value(it->paraId);
            c.parent = parentPara.isEmpty() ? -1 : idOfPara.value(parentPara, -1);
            c.done = src.doneOfParaId.value(it->paraId, false);
        }
        c.id = a.id;
        out.push_back(c);
    }
    // A reply to a comment that is not in the text, or one that makes a loop of replies, is a comment of its own.
    QHash<int, int> parentOf;
    for (const Comment &c : std::as_const(out)) parentOf.insert(c.id, c.parent);
    for (Comment &c : out) {
        if (c.parent < 0) continue;
        if (!parentOf.contains(c.parent) || c.parent == c.id) {
            c.parent = -1;
            parentOf[c.id] = -1;
            continue;
        }
        QSet<int> seen;
        int at = c.id;
        bool loop = false;
        for (int steps = 0; steps < 100000; ++steps) {
            if (seen.contains(at)) { loop = true; break; }
            seen.insert(at);
            const int p = parentOf.value(at, -1);
            if (p < 0 || !parentOf.contains(p)) break;
            at = p;
        }
        if (loop) {
            c.parent = -1;
            parentOf[c.id] = -1;
        }
    }
    src.made = out;
    src.madeAtRevision = doc.story()->revision();
    return out;
}

std::optional<Comment> commentById(WriteDoc &doc, int id)
{
    for (const Comment &c : comments(doc))
        if (c.id == id) return c;
    return std::nullopt;
}

std::optional<Comment> commentAt(WriteDoc &doc, int position)
{
    std::optional<Comment> best;
    for (const Comment &c : comments(doc)) {
        if (c.parent >= 0) continue;
        const bool inside = (position >= c.from && position <= c.to) || position == c.reference || position == c.reference + 1;
        if (!inside) continue;
        if (!best || (c.to - c.from) < (best->to - best->from)) best = c;
    }
    return best;
}

std::optional<Comment> nextComment(WriteDoc &doc, int position)
{
    std::optional<Comment> best;
    for (const Comment &c : comments(doc))
        if (c.parent < 0 && c.from > position && (!best || c.from < best->from)) best = c;
    return best;
}

std::optional<Comment> previousComment(WriteDoc &doc, int position)
{
    std::optional<Comment> best;
    for (const Comment &c : comments(doc))
        if (c.parent < 0 && c.from < position && (!best || c.from > best->from)) best = c;
    return best;
}

QVector<Comment> repliesTo(WriteDoc &doc, int id)
{
    QVector<Comment> out;
    for (const Comment &c : comments(doc))
        if (c.parent == id) out.push_back(c);
    return out;
}

// ---- making comments

namespace {

QTextCharFormat markFormatLike(const QTextCharFormat &like)
{
    QTextCharFormat f = withRevisionLook(like, false, 0, 0);
    f.clearProperty(tp::RevisionFormat);
    f.clearProperty(prop::RawRPr);
    f.clearProperty(tp::InlineObject);
    f.clearProperty(flow::fp::AnchorId);
    f.clearProperty(tp::Field);
    return f;
}

QTextCharFormat commentMark(const QTextCharFormat &like, const Inline &atom)
{
    QTextCharFormat f = markFormatLike(like);
    setStored(f, prop::Atom, packAtom(atom));
    f.setProperty(tp::Field, QStringLiteral("raw:"));
    return f;
}

Inline rangeAtom(int id, bool start)
{
    Inline a;
    a.kind = Inline::ParaAtom;
    a.role = Inline::Single;
    a.tag = start ? QStringLiteral("commentRangeStart") : QStringLiteral("commentRangeEnd");
    a.key = QString::number(id);
    a.raw = QByteArray("<w:") + (start ? "commentRangeStart" : "commentRangeEnd") + " w:id=\"" + QByteArray::number(id) + "\"/>";
    return a;
}

Inline referenceAtom(const Comment &c)
{
    Inline a;
    a.kind = Inline::RunAtom;
    a.role = Inline::Single;
    a.tag = QStringLiteral("commentReference");
    a.raw = "<w:commentReference w:id=\"" + QByteArray::number(c.id) + "\"/>";
    a.look = packComment(c);
    return a;
}

// The format of the character before the place (the one after it at the start of the story).
QTextCharFormat formatAt(QTextDocument *d, int position)
{
    QTextCursor q(d);
    q.setPosition(std::max(1, std::min(position, d->characterCount() - 1)));
    return q.charFormat();
}

Comment newComment(WriteDoc &doc, const QString &text, int parent)
{
    Comment c;
    c.id = doc.nextRevisionId();
    c.author = doc.reviewer();
    c.initials = doc.reviewerInitials();
    if (c.initials.isEmpty()) {
        const QStringList words = c.author.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        for (const QString &w : words) c.initials += w.at(0).toUpper();
        c.initials = c.initials.left(4);
    }
    c.date = QDateTime::fromSecsSinceEpoch(QDateTime::currentSecsSinceEpoch(), Qt::UTC);
    c.text = text;
    c.parent = parent;
    c.changed = true;
    c.paraId = newParaId(paraIdsIn(doc), c.id);
    return c;
}

} // namespace

int addComment(WriteDoc &doc, QTextCursor &c, const QString &text)
{
    QTextDocument *d = c.document();
    if (d != doc.story() || !intoBody(c)) return -1;
    const int from = c.selectionStart(), to = c.selectionEnd();
    if (isLocked(d->findBlock(from)) || isLocked(d->findBlock(to))) return -1;
    sourceOf(doc);
    const UntrackedEdit untracked;
    EditScope scope(doc, d);
    const Comment cm = newComment(doc, text, -1);
    QTextCursor t(d);
    t.beginEditBlock();
    const QTextCharFormat like = formatAt(d, to);
    t.setPosition(to);
    t.insertText(QString(QChar::ObjectReplacementCharacter), commentMark(like, referenceAtom(cm)));
    t.setPosition(to);
    t.insertText(QString(QChar::ObjectReplacementCharacter), commentMark(like, rangeAtom(cm.id, false)));
    t.setPosition(from);
    t.insertText(QString(QChar::ObjectReplacementCharacter), commentMark(like, rangeAtom(cm.id, true)));
    t.endEditBlock();
    c.setPosition(to + 3);
    return cm.id;
}

int replyToComment(WriteDoc &doc, int parentId, const QString &text)
{
    const std::optional<Comment> parent = commentById(doc, parentId);
    if (!parent) return -1;
    QTextDocument *d = doc.story();
    const UntrackedEdit untracked;
    EditScope scope(doc, d);
    Comment cm = newComment(doc, text, parentId);
    // The reply covers what the comment does: its marks are put just after the comment's own.
    QVector<Anchor> anchors = anchorsOf(d);
    const Anchor *pa = nullptr;
    for (const Anchor &a : anchors)
        if (a.id == parentId) pa = &a;
    if (!pa || pa->reference < 0) return -1;
    const Anchor anchor = *pa;
    const QTextCharFormat like = formatAt(d, anchor.reference + 1);
    QTextCursor t(d);
    t.beginEditBlock();
    t.setPosition(anchor.reference + 1);
    t.insertText(QString(QChar::ObjectReplacementCharacter), commentMark(like, referenceAtom(cm)));
    if (anchor.end >= 0) {
        t.setPosition(anchor.end + 1);
        t.insertText(QString(QChar::ObjectReplacementCharacter), commentMark(like, rangeAtom(cm.id, false)));
    }
    if (anchor.start >= 0) {
        t.setPosition(anchor.start + 1);
        t.insertText(QString(QChar::ObjectReplacementCharacter), commentMark(like, rangeAtom(cm.id, true)));
    }
    t.endEditBlock();
    return cm.id;
}

namespace {

// Rewrites what the mark of a comment's reference holds.
void putComment(QTextDocument *d, const Comment &c, int reference)
{
    QTextCursor t(d);
    t.setPosition(reference);
    t.setPosition(reference + 1, QTextCursor::KeepAnchor);
    QTextCharFormat f = t.charFormat();
    // (The format of the character at `reference`: the selection's own.)
    Inline a = unpackAtom(storedBytes(f, prop::Atom));
    Comment stored = c;
    stored.changed = true;
    a.look = packComment(stored);
    setStored(f, prop::Atom, packAtom(a));
    t.setCharFormat(f);
}

// The comments of a thread: the comment and everything that replies to it, down.
QVector<Comment> threadOf(const QVector<Comment> &all, int id)
{
    QVector<Comment> out;
    QSet<int> in;
    for (const Comment &c : all)
        if (c.id == id) {
            out.push_back(c);
            in.insert(id);
        }
    for (int round = 0; round < 1000; ++round) {
        bool grew = false;
        for (const Comment &c : all)
            if (!in.contains(c.id) && in.contains(c.parent)) {
                out.push_back(c);
                in.insert(c.id);
                grew = true;
            }
        if (!grew) break;
    }
    return out;
}

} // namespace

bool setCommentDone(WriteDoc &doc, int id, bool done)
{
    const QVector<Comment> all = comments(doc);
    const QVector<Comment> thread = threadOf(all, id);
    if (thread.isEmpty()) return false;
    const UntrackedEdit untracked;
    EditScope scope(doc, doc.story());
    QTextCursor edit(doc.story());
    edit.beginEditBlock();
    // From the last reference to the first, so that the places hold (a mark's rewriting changes no length).
    for (Comment c : thread) {
        if (c.reference < 0) continue;
        c.done = done;
        putComment(doc.story(), c, c.reference);
    }
    edit.endEditBlock();
    return true;
}

bool setCommentText(WriteDoc &doc, int id, const QString &text)
{
    const std::optional<Comment> found = commentById(doc, id);
    if (!found || found->reference < 0) return false;
    Comment c = *found;
    c.text = text;
    const UntrackedEdit untracked;
    EditScope scope(doc, doc.story());
    QTextCursor edit(doc.story());
    edit.beginEditBlock();
    putComment(doc.story(), c, c.reference);
    edit.endEditBlock();
    return true;
}

int deleteComment(WriteDoc &doc, int id)
{
    const QVector<Comment> all = comments(doc);
    const QVector<Comment> thread = threadOf(all, id);
    if (thread.isEmpty()) return 0;
    QSet<int> ids;
    for (const Comment &c : thread) ids.insert(c.id);
    QTextDocument *d = doc.story();
    // The places of the marks of the thread, from the last to the first.
    QVector<int> places;
    for (QTextBlock b = d->begin(); b.isValid(); b = b.next()) {
        if (b.blockFormat().hasProperty(prop::EndnoteBlock)) continue;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const QTextCharFormat cf = f.charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const Inline a = unpackAtom(storedBytes(cf, prop::Atom));
            const bool mark = (a.kind == Inline::ParaAtom && (a.tag == QLatin1String("commentRangeStart") || a.tag == QLatin1String("commentRangeEnd"))) ||
                              (a.kind == Inline::RunAtom && a.tag == QLatin1String("commentReference"));
            if (!mark) continue;
            const int markId = a.kind == Inline::ParaAtom ? (a.key.isEmpty() ? idOfRaw(a.raw) : a.key.toInt()) : idOfRaw(a.raw);
            if (!ids.contains(markId)) continue;
            for (int k = 0; k < int(f.length()); ++k) places.push_back(f.position() + k);
        }
    }
    std::sort(places.begin(), places.end());
    const UntrackedEdit untracked;
    EditScope scope(doc, d);
    QTextCursor t(d);
    t.beginEditBlock();
    for (int i = int(places.size()) - 1; i >= 0; --i) {
        t.setPosition(places[i]);
        t.setPosition(places[i] + 1, QTextCursor::KeepAnchor);
        t.removeSelectedText();
    }
    t.endEditBlock();
    return int(thread.size());
}

int deleteAllComments(WriteDoc &doc)
{
    int n = 0;
    for (;;) {
        const QVector<Comment> all = comments(doc);
        const Comment *root = nullptr;
        for (const Comment &c : all)
            if (c.parent < 0) {
                root = &c;
                break;
            }
        if (!root) break;
        const int done = deleteComment(doc, root->id);
        if (done <= 0) break;
        n += done;
    }
    return n;
}

// ---- writing the parts

namespace {

QByteArray commentXml(const Comment &c, bool commentStyles)
{
    QByteArray out = "<w:comment w:id=\"" + QByteArray::number(c.id) + "\" w:author=\"" + escapeXml(c.author, true) + "\"";
    if (c.date.isValid()) out += " w:date=\"" + c.date.toUTC().toString(QStringLiteral("yyyy-MM-ddTHH:mm:ssZ")).toLatin1() + "\"";
    out += " w:initials=\"" + escapeXml(c.initials, true) + "\">";
    const QStringList paragraphs = c.text.split(QLatin1Char('\n'));
    for (int i = 0; i < paragraphs.size(); ++i) {
        const bool last = i + 1 == paragraphs.size();
        out += "<w:p";
        if (last && !c.paraId.isEmpty()) out += " w14:paraId=\"" + c.paraId.toLatin1() + "\" w14:textId=\"77777777\"";
        out += ">";
        if (commentStyles) out += "<w:pPr><w:pStyle w:val=\"CommentText\"/></w:pPr>";
        if (i == 0) out += commentStyles ? "<w:r><w:rPr><w:rStyle w:val=\"CommentReference\"/></w:rPr><w:annotationRef/></w:r>" : "<w:r><w:annotationRef/></w:r>";
        if (!paragraphs[i].isEmpty()) out += "<w:r><w:t xml:space=\"preserve\">" + escapeXml(paragraphs[i]) + "</w:t></w:r>";
        out += "</w:p>";
    }
    return out + "</w:comment>";
}

// An element as written, with a paragraph id put on its last paragraph when it has none.
QByteArray withParaId(QByteArray xml, const QString &paraId)
{
    const qsizetype at = xml.lastIndexOf("<w:p>");
    const qsizetype at2 = xml.lastIndexOf("<w:p ");
    const qsizetype p = std::max(at, at2);
    if (p < 0) return xml;
    const qsizetype end = xml.indexOf('>', p);
    if (end < 0 || xml.mid(p, end - p).contains("paraId=")) return xml;
    return xml.left(p + 4) + " w14:paraId=\"" + paraId.toLatin1() + "\" w14:textId=\"77777777\"" + xml.mid(p + 4);
}

void putPart(office::wml::Document *out, const QString &name, const QByteArray &data, const char *type, const char *rel, QString *nameInOut)
{
    for (KeptPart &k : out->parts)
        if (k.name.compare(name, Qt::CaseInsensitive) == 0 && !name.isEmpty()) {
            k.data = data;
            return;
        }
    KeptPart kept;
    kept.name = name;
    kept.contentType = QString::fromLatin1(type);
    kept.data = data;
    out->parts.push_back(kept);
    Relationship r;
    r.id = nextRelationshipId(out->mainRelationships);
    r.type = QString::fromLatin1(rel);
    r.target = name;
    out->mainRelationships.push_back(r);
    if (nameInOut) *nameInOut = name;
}

QString freshPartName(const office::wml::Document &out, const QString &base)
{
    QString name = QStringLiteral("/word/%1.xml").arg(base);
    auto taken = [&](const QString &n) {
        for (const KeptPart &k : out.parts)
            if (k.name.compare(n, Qt::CaseInsensitive) == 0) return true;
        return n.compare(out.mainName, Qt::CaseInsensitive) == 0;
    };
    for (int n = 2; taken(name); ++n) name = QStringLiteral("/word/%1%2.xml").arg(base).arg(n);
    return name;
}

} // namespace

void WriteDoc::writeCommentParts(office::wml::Document *out)
{
    CommentSource &src = sourceOf(*this);
    const QVector<Comment> now = comments(*this);
    QSet<int> nowIds;
    bool dirty = false;
    for (const Comment &c : now) {
        nowIds.insert(c.id);
        dirty = dirty || c.changed;
    }
    if (!dirty && nowIds != src.idsAtOpen) dirty = true;
    if (!dirty) return;
    const bool styles = styleSheet().find(QStringLiteral("CommentText")) && styleSheet().find(QStringLiteral("CommentReference"));
    QSet<QString> taken;
    for (const PartComment &pc : std::as_const(src.byId)) taken.insert(pc.paraId);
    // The comments that are written, in order: the file's own (as written, unless changed), those it has no mark of (kept as they were), then the new ones.
    struct Written { int id; QByteArray xml; QString paraId; QString parentParaId; bool done; bool known; QString author; };
    QVector<Written> written;
    QHash<int, Comment> byId;
    for (const Comment &c : now) byId.insert(c.id, c);
    QHash<int, QString> paraOfId;
    for (const Comment &c : now)
        if (!c.paraId.isEmpty()) paraOfId.insert(c.id, c.paraId);
    auto add = [&](const Comment &c, const QByteArray &original, bool regenerate) {
        Written w;
        w.id = c.id;
        w.paraId = c.paraId;
        w.author = c.author;
        w.parentParaId = c.parent >= 0 ? paraOfId.value(c.parent) : QString();
        w.done = c.done;
        w.known = true;
        if (!original.isEmpty() && !regenerate && !c.paraId.isEmpty()) {
            w.xml = original;
        } else {
            Comment g = c;
            if (g.paraId.isEmpty()) {
                g.paraId = newParaId(taken, g.id);
                taken.insert(g.paraId);
                w.paraId = g.paraId;
            }
            w.xml = original.isEmpty() || regenerate ? commentXml(g, styles) : withParaId(original, g.paraId);
        }
        written.push_back(w);
    };
    QSet<int> done;
    for (const int id : std::as_const(src.order)) {
        const PartComment &pc = src.byId[id];
        if (byId.contains(id)) {
            Comment c = byId.value(id);
            if (c.paraId.isEmpty()) c.paraId = pc.paraId;
            add(c, pc.xml, c.text != pc.text || c.author != pc.author);
            done.insert(id);
        } else if (!src.idsAtOpen.contains(id)) {
            Written w;   // (no mark of it in the text, and none when the file was opened: kept)
            w.id = id;
            w.xml = pc.xml;
            w.paraId = pc.paraId;
            w.parentParaId = src.parentOfParaId.value(pc.paraId);
            w.done = src.doneOfParaId.value(pc.paraId, false);
            w.known = false;
            w.author = pc.author;
            written.push_back(w);
        }
    }
    for (const Comment &c : now)
        if (!done.contains(c.id)) add(c, QByteArray(), true);
    // comments.xml
    QByteArray root = src.commentsRoot;
    if (root.isEmpty()) root = QByteArray("<w:comments xmlns:w=\"") + kWNs + "\">";
    root = ignoring(declaring(root, "w14", kW14), "w14");
    QByteArray part = kXmlHead + root;
    for (const Written &w : std::as_const(written)) part += w.xml;
    part += "</w:comments>";
    QByteArray closing;
    startTagOfRoot(src.commentsRoot.isEmpty() ? root : src.commentsRoot, &closing);
    if (!closing.isEmpty() && closing != "</w:comments>") {
        part.chop(int(qstrlen("</w:comments>")));
        part += closing;
    }
    if (written.isEmpty() && src.commentsName.isEmpty()) return;
    QString commentsName = src.commentsName.isEmpty() ? freshPartName(*out, QStringLiteral("comments")) : src.commentsName;
    putPart(out, commentsName, part, kCommentsType, kCommentsRel, nullptr);
    // commentsExtended.xml: which comments reply to which, and which are done.
    bool wantExtension = !src.extensionName.isEmpty();
    for (const Written &w : std::as_const(written)) wantExtension = wantExtension || (w.known && (w.done || !w.parentParaId.isEmpty()));
    if (wantExtension) {
        QByteArray eroot = src.extensionRoot;
        if (eroot.isEmpty()) eroot = QByteArray("<w15:commentsEx xmlns:w15=\"") + kW15 + "\">";
        eroot = ignoring(declaring(eroot, "w15", kW15), "w15");
        QByteArray ep = kXmlHead + eroot;
        QSet<QString> wrote;
        for (const Written &w : std::as_const(written)) {
            if (w.paraId.isEmpty()) continue;
            wrote.insert(w.paraId);
            if (!w.known && src.extensionOfParaId.contains(w.paraId)) {
                ep += src.extensionOfParaId.value(w.paraId);
                continue;
            }
            ep += "<w15:commentEx w15:paraId=\"" + w.paraId.toLatin1() + "\"";
            if (!w.parentParaId.isEmpty()) ep += " w15:paraIdParent=\"" + w.parentParaId.toLatin1() + "\"";
            ep += QByteArray(" w15:done=\"") + (w.done ? "1" : "0") + "\"/>";
        }
        QByteArray eclosing;
        startTagOfRoot(eroot, &eclosing);
        ep += eclosing.isEmpty() ? QByteArray("</w15:commentsEx>") : eclosing;
        QString extensionName = src.extensionName.isEmpty() ? freshPartName(*out, QStringLiteral("commentsExtended")) : src.extensionName;
        putPart(out, extensionName, ep, kExtendedType, kExtendedRel, nullptr);
    }
    // commentsIds.xml (kept for the comments that stay, with an entry for each new one) and people.xml (with the authors of the comments).
    if (!src.idsName.isEmpty()) {
        QByteArray iroot = src.idsRoot;
        QByteArray ip = kXmlHead + iroot;
        QSet<QString> have;
        for (const Written &w : std::as_const(written)) {
            if (w.paraId.isEmpty()) continue;
            have.insert(w.paraId);
            if (src.idsOfParaId.contains(w.paraId)) ip += src.idsOfParaId.value(w.paraId);
            else ip += "<w16cid:commentId w16cid:paraId=\"" + w.paraId.toLatin1() + "\" w16cid:durableId=\"" + newParaId(have, w.id).toLatin1() + "\"/>";
        }
        QByteArray iclosing;
        startTagOfRoot(iroot, &iclosing);
        ip += iclosing;
        putPart(out, src.idsName, ip, kIdsType, kIdsRel, nullptr);
    }
    if (!src.peopleName.isEmpty() || wantExtension) {
        QStringList authors = src.people;
        bool added = false;
        QByteArray extra;
        for (const Written &w : std::as_const(written)) {
            if (!w.known || w.author.isEmpty() || authors.contains(w.author)) continue;
            authors << w.author;
            added = true;
            extra += "<w15:person w15:author=\"" + escapeXml(w.author, true) + "\"><w15:presenceInfo w15:providerId=\"None\" w15:userId=\"" + escapeXml(w.author, true) + "\"/></w15:person>";
        }
        if (added || src.peopleName.isEmpty()) {
            QByteArray proot = src.peopleRoot;
            if (proot.isEmpty()) proot = QByteArray("<w15:people xmlns:w15=\"") + kW15 + "\">";
            QByteArray pclosing;
            startTagOfRoot(proot, &pclosing);
            if (pclosing.isEmpty()) pclosing = "</w15:people>";
            QByteArray pp;
            if (!src.peopleBytes.isEmpty()) {
                const qsizetype at = src.peopleBytes.lastIndexOf(pclosing);
                pp = at >= 0 ? src.peopleBytes.left(at) + extra + pclosing : src.peopleBytes;
            } else {
                pp = kXmlHead + proot + extra + pclosing;
            }
            QString peopleName = src.peopleName.isEmpty() ? freshPartName(*out, QStringLiteral("people")) : src.peopleName;
            putPart(out, peopleName, pp, kPeopleType, kPeopleRel, nullptr);
        }
    }
}

} // namespace jp::write
