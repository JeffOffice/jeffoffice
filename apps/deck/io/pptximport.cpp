#include "io/pptximport.h"

#include "core/appinfo.h"
#include "core/comments.h"
#include "core/deck.h"
#include "core/embeddedfonts.h"
#include "core/sections.h"
#include "io/pptxdiagram.h"
#include "io/pptxfonts.h"
#include "io/pptxshow.h"
#include "io/pptxtablestyles.h"
#include "io/pptxtransition.h"
#include "io/presentationfile.h"
#include "office/chart.h"
#include "office/drawingml.h"
#include "office/drawingml_table.h"
#include "office/package.h"
#include "office/theme.h"
#include "office/xml.h"
#include "render/renderer.h"
#include "text/textengine.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QTextDocument>
#include <QTextStream>
#include <QTimeZone>
#include <climits>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <utility>

namespace jp::deck {

namespace {

const QLatin1String kMc("http://schemas.openxmlformats.org/markup-compatibility/2006");
// The threading extension of the older comments, the newer comments, and the extension that marks a comment done.
const QLatin1String kP15("http://schemas.microsoft.com/office/powerpoint/2012/main");
const QLatin1String kP188("http://schemas.microsoft.com/office/powerpoint/2018/8/main");
const QLatin1String kJoComments("http://jeffoffice.com/ns/deck/comments");
// The extension that lists the sections, and the namespace its elements are in.
const QLatin1String kSectionExt("{521415D9-36F7-43E2-AB2F-B90AF26B5E84}");
const QLatin1String kSectionNs("http://schemas.microsoft.com/office/powerpoint/2010/main");

// What a file may hold: more is left unread, with a note. The sample decks have at most 43 slides, 24 layouts, and 2 slide masters (a presentation
// converted from another format, 11), so these are 232, 41, and 18 times as many.
constexpr int kMaxSlides = 10000, kMaxLayouts = 1000, kMaxMasters = 200;
// A theme part is a few kilobytes: the largest in the sample decks is 8,723 bytes, and this is 30 times that.
constexpr qint64 kMaxThemeBytes = 256 << 10;
// What the whole file may keep, however it spreads that over slides, layouts, and masters, or lists the same part again: every read of it draws from one
// DrawingBudget. The sample decks' largest hold 507 objects, 397 text boxes and shapes with text, 156 table cells, 790 paragraphs, 666 runs, and 18,542
// letters, so these are 394, 76, 320, 380, 750, and 1,080 times as many. A file that uses up any one of them (measured with files that do, the time of
// reading their XML included) takes at most 10 seconds and 0.6 GB, and one that uses up several at once 14 seconds and 1.3 GB.
constexpr qint64 kMaxObjects = 200000, kMaxStories = 30000, kMaxCells = 50000, kMaxParagraphs = 300000, kMaxRuns = 500000, kMaxLetters = 20000000;
// What one page (a slide, a layout, or a master) may keep of it, so that each can be drawn in a moment: the sample decks' fullest slides hold
// 49 objects, 49 shapes with text, and 2,056 letters, so these are 102, 20, and 49 times as many (the cells, paragraphs, and runs as many
// again as the file's, in proportion).
constexpr qint64 kMaxPageObjects = 5000, kMaxPageStories = 1000, kMaxPageCells = 2000, kMaxPageParagraphs = 10000, kMaxPageRuns = 20000, kMaxPageLetters = 100000;

// The notes said of a file: more than this are counted, not listed.
constexpr int kMaxNotes = 20;
// A slide may be 56 inches across at most (the standard's limit), and is a point at least.
constexpr qint64 kMinSlideEmu = 12700, kMaxSlideEmu = 51206400;
// A chart keeps the parts it relates to (its workbook, a drawing laid over it...), down to this depth, at most so many, of at most this much in all.
constexpr int kMaxChartParts = 32, kMaxChartDepth = 4;
constexpr qint64 kMaxChartBytes = 64 << 20;
// A diagram keeps its parts as a chart does, down to the pictures in its drawing; all the diagrams of a file keep no more than this between them.
constexpr qint64 kMaxDiagramBytes = 256 << 20;
// The diagrams a file may have drawn (each is read and set as a slide is, and kept as an SVG), and what their drawings come to: the sample has ten,
// one on each slide, and real presentations a few dozen, with drawings of a few kilobytes (pictures in them are kept at most 1280 pixels across),
// so these are 100 times as many, and 25 times that much with a picture on every other shape.
constexpr int kMaxDiagrams = 1000;
constexpr qint64 kMaxDiagramSvgBytes = 256 << 20;
// The extension that holds a shape's math (a14:m) in a shape's text, which a newer program writes with a picture of the shape beside it.
const QLatin1String kA14("http://schemas.microsoft.com/office/drawing/2010/main");


QDomElement pChild(const QDomElement &e, const char *name) { return office::child(e, office::ns::P, QLatin1String(name)); }
QDomElement aChild(const QDomElement &e, const char *name) { return office::child(e, office::ns::A, QLatin1String(name)); }

bool isOneOf(const QString &name, std::initializer_list<const char *> names)
{
    for (const char *n : names)
        if (name == QLatin1String(n)) return true;
    return false;
}

// The name of a part for a note: its file name.
QString shortName(const QString &part) { return part.mid(part.lastIndexOf(QLatin1Char('/')) + 1); }

// What a placeholder says about itself (p:ph).
struct PhRef {
    QDomElement shape;
    QString type = QStringLiteral("obj");
    qint64 idx = 0;
};

bool placeholderOf(const QDomElement &shape, PhRef *out)
{
    for (QDomElement nv = shape.firstChildElement(); !nv.isNull(); nv = nv.nextSiblingElement()) {
        if (!nv.localName().startsWith(QLatin1String("nv"))) continue;
        const QDomElement ph = pChild(pChild(nv, "nvPr"), "ph");
        if (ph.isNull()) return false;
        out->shape = shape;
        out->type = office::attr(ph, QStringLiteral("type"), QStringLiteral("obj"));
        out->idx = std::max<qint64>(0, office::parseInt(office::attr(ph, QStringLiteral("idx")), 0));
        return true;
    }
    return false;
}

// Placeholder types that take each other's places. `wide`: every kind of
// content takes the body's, as a slide master has no other place for it.
QString family(const QString &type, bool wide)
{
    if (type == QLatin1String("title") || type == QLatin1String("ctrTitle")) return QStringLiteral("title");
    if (isOneOf(type, {"body", "subTitle", "obj"})) return QStringLiteral("body");
    if (wide && isOneOf(type, {"pic", "chart", "tbl", "clipArt", "dgm", "media"})) return QStringLiteral("body");
    return type;
}

// The placeholders of a layout or master, and the one that `want` stands on:
// on the same index and of the same kind, else of the same type, else of the
// same kind (the first of each, in the order they are in). A slide's
// placeholder goes by index to its layout's (`byIdx`); a layout's has only a
// type to go by to its master's. Each is found by lookup, so that slides of
// thousands of placeholders over a layout of thousands don't compare each
// with each.
class PhList {
public:
    PhList() = default;
    explicit PhList(QVector<PhRef> list) : m_list(std::move(list))
    {
        for (int i = 0; i < int(m_list.size()); ++i) {
            const PhRef &p = m_list[i];
            if (!m_byType.contains(p.type)) m_byType.insert(p.type, i);
            for (int w = 0; w < 2; ++w) {
                const QString kind = family(p.type, w != 0);
                if (!m_byKind[w].contains(kind)) m_byKind[w].insert(kind, i);
                if (!m_byIdx[w].contains({p.idx, kind})) m_byIdx[w].insert({p.idx, kind}, i);
            }
        }
    }
    const PhRef *find(const PhRef &want, bool byIdx, bool wide) const
    {
        const int w = wide ? 1 : 0;
        const QString kind = family(want.type, wide);
        if (byIdx)
            if (const auto it = m_byIdx[w].constFind({want.idx, kind}); it != m_byIdx[w].cend()) return &m_list[it.value()];
        if (const auto it = m_byType.constFind(want.type); it != m_byType.cend()) return &m_list[it.value()];
        if (const auto it = m_byKind[w].constFind(kind); it != m_byKind[w].cend()) return &m_list[it.value()];
        return nullptr;
    }

private:
    QVector<PhRef> m_list;
    QHash<QPair<qint64, QString>, int> m_byIdx[2];   // by index and kind, for a placeholder asking without `wide` and with
    QHash<QString, int> m_byType;
    QHash<QString, int> m_byKind[2];
};

QString roleOf(const PhRef &ph)
{
    return ph.idx > INT_MAX ? QStringLiteral("ph:%1:%2").arg(ph.type).arg(ph.idx) : placeholderRole(ph.type, int(ph.idx));
}

// The kinds of placeholder that hold text, and so are text boxes.
bool holdsText(const QString &type) { return !isOneOf(type, {"sldImg", "tbl", "chart", "dgm", "media", "clipArt"}); }

// The picture a graphic frame holds for programs that can't show what it
// really is: an embedded object's preview.
QDomElement previewPicture(const QDomElement &e, int depth = 0)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        if (office::isElement(c, office::ns::P, QStringLiteral("pic"))) return c;
        if (depth < 8)
            if (const QDomElement found = previewPicture(c, depth + 1); !found.isNull()) return found;
    }
    return QDomElement();
}

QDomElement listStyleOf(const QDomElement &shape) { return aChild(pChild(shape, "txBody"), "lstStyle"); }

QString escaped(const QString &text, bool attribute)
{
    QString out;
    out.reserve(text.size());
    for (const QChar c : text) {
        if (c == QLatin1Char('&')) out += QLatin1String("&amp;");
        else if (c == QLatin1Char('<')) out += QLatin1String("&lt;");
        else if (c == QLatin1Char('>')) out += QLatin1String("&gt;");
        else if (attribute && c == QLatin1Char('"')) out += QLatin1String("&quot;");
        else if (attribute && c == QLatin1Char('\n')) out += QLatin1String("&#10;");
        else if (attribute && c == QLatin1Char('\t')) out += QLatin1String("&#9;");
        else out += c;
    }
    return out;
}

QString qualified(const QString &prefix, const QString &local) { return prefix.isEmpty() ? local : prefix + QLatin1Char(':') + local; }

// Every prefix the element and what is in it use, with its namespace.
void collectPrefixes(const QDomElement &e, QHash<QString, QString> *known, int depth = 0)
{
    if (depth > 256) return;
    if (!e.prefix().isEmpty()) known->insert(e.prefix(), e.namespaceURI());
    const QDomNamedNodeMap attrs = e.attributes();
    for (int i = 0; i < attrs.length(); ++i) {
        const QDomAttr a = attrs.item(i).toAttr();
        if (!a.prefix().isEmpty()) known->insert(a.prefix(), a.namespaceURI());
    }
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) collectPrefixes(c, known, depth + 1);
}

// Writes the element and what is in it, declaring each namespace where it is
// first used (`scope`: the prefixes declared so far, "" the default). The
// prefixes markup compatibility names in attribute values (mc:Choice's
// Requires, mc:Ignorable) are declared on that element too, from `known`:
// readers look them up there, and a file whose Requires names an undeclared
// prefix is refused whole.
void writeElement(const QDomElement &e, QString *out, QHash<QString, QString> *scope, const QHash<QString, QString> &known)
{
    QVector<QPair<QString, QString>> declared;
    auto need = [&](const QString &prefix, const QString &uri) {
        const auto it = scope->constFind(prefix);
        const QString current = it == scope->cend() ? QString() : it.value();
        if (current == uri) return;
        for (const auto &d : declared)
            if (d.first == prefix) return;
        declared << qMakePair(prefix, uri);
    };
    need(e.prefix(), e.namespaceURI());
    const QDomNamedNodeMap attrs = e.attributes();
    for (int i = 0; i < attrs.length(); ++i) {
        const QDomAttr a = attrs.item(i).toAttr();
        if (!a.prefix().isEmpty()) need(a.prefix(), a.namespaceURI());
        const QString name = a.localName();
        if (name == QLatin1String("Requires") || name == QLatin1String("Ignorable") || name == QLatin1String("ProcessContent") || name == QLatin1String("MustUnderstand"))
            for (const QString &listed : a.value().split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
                const QString prefix = listed.section(QLatin1Char(':'), 0, 0);   // ProcessContent names prefix:element
                if (known.contains(prefix)) need(prefix, known.value(prefix));
            }
    }
    *out += QLatin1Char('<') + qualified(e.prefix(), e.localName());
    struct Saved {
        QString prefix, uri;
        bool had;
    };
    QVector<Saved> saved;
    for (const auto &d : declared) {
        saved << Saved{d.first, scope->value(d.first), scope->contains(d.first)};
        *out += d.first.isEmpty() ? QStringLiteral(" xmlns=\"%1\"").arg(escaped(d.second, true)) : QStringLiteral(" xmlns:%1=\"%2\"").arg(d.first, escaped(d.second, true));
        scope->insert(d.first, d.second);
    }
    for (int i = 0; i < attrs.length(); ++i) {
        const QDomAttr a = attrs.item(i).toAttr();
        *out += QLatin1Char(' ') + qualified(a.prefix(), a.localName()) + QLatin1String("=\"") + escaped(a.value(), true) + QLatin1Char('"');
    }
    if (!e.hasChildNodes()) {
        *out += QLatin1String("/>");
    } else {
        *out += QLatin1Char('>');
        for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
            if (n.isElement()) writeElement(n.toElement(), out, scope, known);
            else if (n.isCharacterData() && !n.isComment()) *out += escaped(n.toCharacterData().data(), false);
        }
        *out += QLatin1String("</") + qualified(e.prefix(), e.localName()) + QLatin1Char('>');
    }
    // The namespaces go back to what they were outside the element.
    for (const Saved &v : saved) {
        if (v.had) scope->insert(v.prefix, v.uri);
        else scope->remove(v.prefix);
    }
}

// The element as XML that can be read by itself: every namespace it uses is
// declared in it (the DOM does not keep the declarations of the elements around it).
QString standaloneXml(const QDomElement &e)
{
    QString out;
    QHash<QString, QString> scope, known;
    collectPrefixes(e, &known);
    writeElement(e, &out, &scope, known);
    return out;
}

// The name the presentation program gives a slide size.
QString sizeNameOf(const QString &type, qint64 cx, qint64 cy)
{
    struct Named { const char *type, *name; };
    static const Named names[] = {
        {"screen4x3", "On-screen Show (4:3)"}, {"screen16x9", "On-screen Show (16:9)"}, {"screen16x10", "On-screen Show (16:10)"},
        {"letter", "Letter Paper"}, {"A4", "A4 Paper"}, {"35mm", "35mm Slides"}, {"overhead", "Overhead"}, {"banner", "Banner"},
        {"ledger", "Ledger Paper"}, {"A3", "A3 Paper"}, {"B4ISO", "B4 (ISO) Paper"}, {"B5ISO", "B5 (ISO) Paper"},
        {"B4JIS", "B4 (JIS) Paper"}, {"B5JIS", "B5 (JIS) Paper"}, {"hagakiCard", "Hagaki Card"}};
    for (const Named &n : names)
        if (type == QLatin1String(n.type)) return QString::fromLatin1(n.name);
    // Widescreen is what files written without a type say for 13.333 by 7.5 inches.
    return type.isEmpty() && cx == 12192000 && cy == 6858000 ? QStringLiteral("Widescreen") : QStringLiteral("Custom");
}

// What a file may hold of comments: more is left unread, with a note.
constexpr int kMaxComments = 1000, kMaxReplies = 1000, kMaxAllComments = 20000, kMaxCommentAuthors = 10000;

// Reads the comments of a slide into its props (core/comments.h). The file has
// them in one of two forms, both read:
//  - The older form, ECMA-376 Part 1 (19.4). The presentation relates to a part of
//    comment authors (relationship type ".../commentAuthors"; p:cmAuthorLst holds
//    p:cmAuthor with id, name, initials), and each slide to a part of its comments
//    (".../comments"; p:cmLst holds p:cm with authorId, dt, and idx, then p:pos with x
//    and y in 576ths of an inch, then p:text). A reply is a p:cm too; the threading
//    extension of [MS-PPTX] (p:ext uri {C676402C-5697-4E1C-873F-D02D1690AC5C}) has
//    p15:threadingInfo, whose p15:parentCm gives the authorId and idx of the comment it
//    answers and whose timeZoneBias is the author's offset from UTC in minutes (UTC is
//    the time plus the bias), which the dt needs when it names no zone. The standard has
//    no mark for a comment that is done; the extension JeffDeck writes for it is read.
//  - The newer form, [MS-PPTX] (namespace .../powerpoint/2018/8/main). The presentation
//    relates to a part of authors (".../2018/10/relationships/authors"; p188:authorLst
//    holds p188:author with a GUID id, name, initials), and each slide to a part of
//    comments (".../2018/10/relationships/comments", also ending "/comments"; p188:cmLst
//    holds p188:cm with id, authorId, created, and status, then what the comment is
//    anchored to, p188:pos in EMU for a place on the slide, p188:replyLst of p188:reply,
//    and p188:txBody, a DrawingML text body). A comment anchored to a shape names it in
//    an spMk element with the shape's number.
class CommentReader {
public:
    CommentReader(const office::Package &pkg, const QString &mainPart, std::function<void(const QString &)> note)
        : m_pkg(pkg), m_main(mainPart), m_note(std::move(note)) {}
    void read(const QString &slidePart, int number, Page &page, const QSizeF &slide);

private:
    struct Person {
        QString name, initials;
    };
    struct Raw {
        qint64 author = -1, idx = -1;
        QString text;
        QDateTime date;
        QPointF at;
        bool placed = false, done = false, hasParent = false;
        qint64 parentAuthor = -1, parentIdx = -1;
    };
    void loadAuthors();
    bool parse(const QString &part, int number, QDomDocument *dom, QDomElement *root);
    void readOlder(const QDomElement &root, QVector<deck::Comment> *out, const QSizeF &slide);
    void readNewer(const QDomElement &root, QVector<deck::Comment> *out, const Page &page, const QSizeF &slide);
    QPointF unplaced(const QSizeF &slide);
    bool room();
    Person olderAuthor(qint64 id) const;
    Person newerAuthor(const QString &id) const;
    static QPointF clamped(QPointF at, const QSizeF &slide);

    const office::Package &m_pkg;
    QString m_main;
    std::function<void(const QString &)> m_note;
    bool m_authorsRead = false;
    QSet<QString> m_readParts;   // the comment parts read so far, by lower-cased name: one named again is read once
    QHash<qint64, Person> m_older;
    QHash<QString, Person> m_newer;   // by lower-cased id
    int m_total = 0, m_slide = 0, m_unplaced = 0;
    bool m_totalSaid = false;
};

// The farthest a time zone is from UTC, in minutes.
constexpr qint64 kMaxZoneBias = 24 * 60;

// 2013-05-10T10:21:50.343 (the author's own time, with the offset from UTC in
// `bias` when the file gave it), or with a zone: Z, +02:00. UTC; invalid for text that isn't a time.
QDateTime commentDate(QString text, std::optional<qint64> bias)
{
    text = text.trimmed();
    static const QRegularExpression fraction(QStringLiteral("(\\.\\d{3})\\d+"));
    static const QRegularExpression zone(QStringLiteral("T.*(Z|[+-]\\d\\d(:?\\d\\d)?)$"));
    text.replace(fraction, QStringLiteral("\\1"));
    const QDateTime t = QDateTime::fromString(text, Qt::ISODateWithMs);
    if (!t.isValid()) return QDateTime();
    if (zone.match(text).hasMatch()) return t.toUTC();
    const QDateTime own(t.date(), t.time(), QTimeZone::UTC);
    // A zone is within a day of UTC; the file's number may be any, and times it by 60.
    return bias ? own.addSecs(std::clamp<qint64>(*bias, -kMaxZoneBias, kMaxZoneBias) * 60) : own;
}

// The text of a DrawingML text body: a line for each paragraph.
QString commentBody(const QDomElement &body)
{
    QStringList lines;
    for (const QDomElement &p : office::children(body, office::ns::A, QStringLiteral("p"))) {
        QString line;
        for (const QDomElement &r : office::children(p)) {
            if (office::isElement(r, office::ns::A, QStringLiteral("br"))) line += QLatin1Char('\n');
            else if (office::isElement(r, office::ns::A, QStringLiteral("r")) || office::isElement(r, office::ns::A, QStringLiteral("fld"))) line += aChild(r, "t").text();
        }
        lines << line;
    }
    return lines.join(QLatin1Char('\n'));
}

// The first element called `local` below `e` (any namespace), down to a depth.
QDomElement findNamed(const QDomElement &e, const QString &local, int depth = 0)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        if (c.localName() == local) return c;
        if (depth < 6)
            if (const QDomElement found = findNamed(c, local, depth + 1); !found.isNull()) return found;
    }
    return QDomElement();
}

QPointF CommentReader::clamped(QPointF at, const QSizeF &slide)
{
    return QPointF(std::clamp(at.x(), 0.0, std::max(0.0, slide.width())), std::clamp(at.y(), 0.0, std::max(0.0, slide.height())));
}

// A comment with no place of its own stands at the corner, clear of the others.
QPointF CommentReader::unplaced(const QSizeF &slide)
{
    const int k = m_unplaced++;
    return clamped(QPointF(10 + 22 * (k % 30), 10 + 22 * (k / 30)), slide);
}

// Whether the file may have another comment or reply read (it holds only so many in all); says so once when not.
bool CommentReader::room()
{
    if (m_total < kMaxAllComments) {
        ++m_total;
        return true;
    }
    if (!m_totalSaid) m_note(QCoreApplication::translate("PptxImport", "Only the first %1 comments and replies of the file are read.").arg(kMaxAllComments));
    m_totalSaid = true;
    return false;
}

CommentReader::Person CommentReader::olderAuthor(qint64 id) const
{
    return m_older.contains(id) ? m_older.value(id) : Person{QCoreApplication::translate("PptxImport", "Unknown author"), QString()};
}

CommentReader::Person CommentReader::newerAuthor(const QString &id) const
{
    return m_newer.contains(id.toLower()) ? m_newer.value(id.toLower()) : Person{QCoreApplication::translate("PptxImport", "Unknown author"), QString()};
}

void CommentReader::loadAuthors()
{
    if (m_authorsRead) return;
    m_authorsRead = true;
    QSet<QString> seen;   // a part of authors the presentation names twice is read once
    auto people = [&](const QString &suffix, const char *rootName, const QString &rootNs, const char *personName, auto add) {
        for (const office::Relationship &rel : m_pkg.relationshipsOfType(m_main, suffix)) {
            if (rel.external || seen.contains(rel.target.toLower())) continue;
            seen.insert(rel.target.toLower());
            QDomDocument dom;
            QDomElement root;
            if (!parse(rel.target, 0, &dom, &root) || !office::isElement(root, rootNs, QLatin1String(rootName))) continue;
            int n = 0;
            for (const QDomElement &a : office::children(root, rootNs, QLatin1String(personName))) {
                if (++n > kMaxCommentAuthors) {
                    m_note(QCoreApplication::translate("PptxImport", "Only the first %1 comment authors are read.").arg(kMaxCommentAuthors));
                    break;
                }
                add(a, Person{office::attr(a, QStringLiteral("name")).left(256), office::attr(a, QStringLiteral("initials")).left(16)});
            }
        }
    };
    people(QStringLiteral("/commentAuthors"), "cmAuthorLst", office::ns::P, "cmAuthor", [&](const QDomElement &a, const Person &p) {
        bool ok = false;
        const qint64 id = office::parseInt(office::attr(a, QStringLiteral("id")), -1, &ok);
        if (ok && !m_older.contains(id)) m_older.insert(id, p);
    });
    people(QStringLiteral("/authors"), "authorLst", kP188, "author", [&](const QDomElement &a, const Person &p) {
        const QString id = office::attr(a, QStringLiteral("id")).toLower();
        if (!id.isEmpty() && !m_newer.contains(id)) m_newer.insert(id, p);
    });
}

bool CommentReader::parse(const QString &part, int number, QDomDocument *dom, QDomElement *root)
{
    QString why;
    const QByteArray xml = m_pkg.part(part);
    if (xml.isEmpty()) why = QCoreApplication::translate("PptxImport", "the part is missing from the file");
    else if (office::parseXml(xml, dom, &why)) {
        *root = dom->documentElement();
        return true;
    }
    m_note(number ? QCoreApplication::translate("PptxImport", "The comments of slide %1 can't be read: %2").arg(number).arg(why) : QCoreApplication::translate("PptxImport", "The comment authors can't be read: %1").arg(why));
    return false;
}

void CommentReader::read(const QString &slidePart, int number, Page &page, const QSizeF &slide)
{
    const QVector<office::Relationship> rels = m_pkg.relationshipsOfType(slidePart, QStringLiteral("/comments"));
    if (rels.isEmpty()) return;
    loadAuthors();
    m_slide = number;
    m_unplaced = 0;
    QVector<deck::Comment> out;
    for (const office::Relationship &rel : rels) {
        if (rel.external) continue;
        // The comments of one slide are the part of that slide: another that names it has none of its own.
        if (m_readParts.contains(rel.target.toLower())) {
            m_note(QCoreApplication::translate("PptxImport", "The comments of slide %1 were left out: the part %2 is read once.").arg(number).arg(shortName(rel.target)));
            continue;
        }
        m_readParts.insert(rel.target.toLower());
        QDomDocument dom;
        QDomElement root;
        if (!parse(rel.target, number, &dom, &root)) continue;
        if (office::isElement(root, office::ns::P, QStringLiteral("cmLst"))) readOlder(root, &out, slide);
        else if (office::isElement(root, kP188, QStringLiteral("cmLst"))) readNewer(root, &out, page, slide);
        else m_note(QCoreApplication::translate("PptxImport", "The comments of slide %1 can't be read: %2").arg(number).arg(QCoreApplication::translate("PptxImport", "the part is not what the file says it is")));
    }
    deck::setComments(page, out);
}

void CommentReader::readOlder(const QDomElement &root, QVector<deck::Comment> *out, const QSizeF &slide)
{
    QVector<Raw> raws;
    for (const QDomElement &cm : office::children(root, office::ns::P, QStringLiteral("cm"))) {
        if (raws.size() >= kMaxComments) {
            m_note(QCoreApplication::translate("PptxImport", "Only the first %1 comments of slide %2 are read.").arg(kMaxComments).arg(m_slide));
            break;
        }
        if (!room()) break;
        Raw r;
        r.author = office::parseInt(office::attr(cm, QStringLiteral("authorId")), -1);
        r.idx = office::parseInt(office::attr(cm, QStringLiteral("idx")), -1);
        r.text = deck::commentText(pChild(cm, "text").text());
        std::optional<qint64> bias;
        for (const QDomElement &ext : office::children(pChild(cm, "extLst"), office::ns::P, QStringLiteral("ext")))
            for (const QDomElement &e : office::children(ext)) {
                if (office::isElement(e, kP15, QStringLiteral("threadingInfo"))) {
                    bool ok = false;
                    const qint64 b = office::parseInt(office::attr(e, QStringLiteral("timeZoneBias")), 0, &ok);
                    if (ok) bias = b;
                    const QDomElement parent = office::child(e, kP15, QStringLiteral("parentCm"));
                    if (!parent.isNull()) {
                        r.hasParent = true;
                        r.parentAuthor = office::parseInt(office::attr(parent, QStringLiteral("authorId")), -1);
                        r.parentIdx = office::parseInt(office::attr(parent, QStringLiteral("idx")), -1);
                    }
                } else if (office::isElement(e, kJoComments, QStringLiteral("done"))) {
                    r.done = office::parseBool(office::attr(e, QStringLiteral("val")), true);
                }
            }
        r.date = commentDate(office::attr(cm, QStringLiteral("dt")), bias);
        const QDomElement pos = pChild(cm, "pos");
        bool okX = false, okY = false;
        const qint64 x = office::parseInt(office::attr(pos, QStringLiteral("x")), 0, &okX), y = office::parseInt(office::attr(pos, QStringLiteral("y")), 0, &okY);
        r.placed = okX && okY;
        r.at = r.placed ? clamped(QPointF(x / 8.0, y / 8.0), slide) : QPointF();   // 576ths of an inch, 8 to the point
        raws << r;
    }

    // Each reply names its comment by author and index; one whose comment is missing, or
    // that would answer itself or go round in a circle, is a comment of its own.
    const int n = int(raws.size());
    QHash<QPair<qint64, qint64>, int> byKey;
    for (int i = 0; i < n; ++i)
        if (!byKey.contains({raws[i].author, raws[i].idx})) byKey.insert({raws[i].author, raws[i].idx}, i);
    QVector<int> parent(n, -1);
    for (int i = 0; i < n; ++i)
        if (raws[i].hasParent) {
            const int p = byKey.value({raws[i].parentAuthor, raws[i].parentIdx}, -1);
            parent[i] = p == i ? -1 : p;
        }
    for (int i = 0; i < n; ++i) {
        int j = parent[i], steps = 0;
        while (j >= 0 && j != i && steps++ < n) j = parent[j];
        if (j == i) parent[i] = -1;
    }
    auto rootOf = [&](int i) {
        for (int steps = 0; parent[i] >= 0 && steps < n; ++steps) i = parent[i];
        return i;
    };
    const qsizetype first = out->size();
    QHash<int, qsizetype> place;   // raw -> its comment in `out`
    for (int i = 0; i < n; ++i) {
        if (parent[i] >= 0) continue;
        const Person who = olderAuthor(raws[i].author);
        deck::Comment c;
        c.id = newId("c");
        c.author = who.name;
        c.initials = who.initials;
        c.date = raws[i].date;
        c.text = raws[i].text;
        c.at = raws[i].placed ? raws[i].at : unplaced(slide);
        c.done = raws[i].done;
        place.insert(i, out->size());
        out->append(c);
    }
    bool saidReplies = false;
    for (int i = 0; i < n; ++i) {
        if (parent[i] < 0) continue;
        deck::Comment &c = (*out)[place.value(rootOf(i), first)];
        if (c.replies.size() >= kMaxReplies) {
            if (!saidReplies) m_note(QCoreApplication::translate("PptxImport", "Only the first %1 replies to a comment on slide %2 are read.").arg(kMaxReplies).arg(m_slide));
            saidReplies = true;
            continue;
        }
        const Person who = olderAuthor(raws[i].author);
        c.replies.append(deck::CommentReply{newId("r"), who.name, who.initials, raws[i].date, raws[i].text});
    }
}

void CommentReader::readNewer(const QDomElement &root, QVector<deck::Comment> *out, const Page &page, const QSizeF &slide)
{
    std::optional<QHash<int, QPointF>> cornerOf;
    int count = 0;
    for (const QDomElement &cm : office::children(root, kP188, QStringLiteral("cm"))) {
        if (++count > kMaxComments) {
            m_note(QCoreApplication::translate("PptxImport", "Only the first %1 comments of slide %2 are read.").arg(kMaxComments).arg(m_slide));
            break;
        }
        if (!room()) break;
        const Person who = newerAuthor(office::attr(cm, QStringLiteral("authorId")));
        deck::Comment c;
        c.id = newId("c");
        c.author = who.name;
        c.initials = who.initials;
        c.date = commentDate(office::attr(cm, QStringLiteral("created")), std::nullopt);
        c.text = deck::commentText(commentBody(office::child(cm, kP188, QStringLiteral("txBody"))));
        const QString status = office::attr(cm, QStringLiteral("status"));
        c.done = status == QLatin1String("resolved") || status == QLatin1String("closed");
        // A place on the slide (EMU), else the top right of the shape it is anchored to, else the corner.
        const QDomElement pos = office::child(cm, kP188, QStringLiteral("pos"));
        bool okX = false, okY = false;
        const qint64 x = office::parseInt(office::attr(pos, QStringLiteral("x")), 0, &okX), y = office::parseInt(office::attr(pos, QStringLiteral("y")), 0, &okY);
        bool placed = okX && okY;
        if (placed) {
            c.at = clamped(QPointF(office::emuToPoints(x), office::emuToPoints(y)), slide);
        } else if (const QDomElement shape = findNamed(cm, QStringLiteral("spMk")); !shape.isNull()) {
            // The first shape of each number, found once for all the comments of the part.
            if (!cornerOf) {
                cornerOf.emplace();
                walkItems(page.items, [&](const ItemPtr &it) {
                    if (!cornerOf->contains(it->sourceId)) cornerOf->insert(it->sourceId, QPointF(it->rect.right(), it->rect.top()));
                });
            }
            const qint64 id = office::parseInt(office::attr(shape, QStringLiteral("id")), -1);
            if (id >= INT_MIN && id <= INT_MAX && cornerOf->contains(int(id))) {
                c.at = clamped(cornerOf->value(int(id)), slide);
                placed = true;
            }
        }
        if (!placed) c.at = unplaced(slide);
        bool saidReplies = false;
        for (const QDomElement &reply : office::children(office::child(cm, kP188, QStringLiteral("replyLst")), kP188, QStringLiteral("reply"))) {
            if (c.replies.size() >= kMaxReplies) {
                if (!saidReplies) m_note(QCoreApplication::translate("PptxImport", "Only the first %1 replies to a comment on slide %2 are read.").arg(kMaxReplies).arg(m_slide));
                saidReplies = true;
                break;
            }
            if (!room()) break;
            const Person by = newerAuthor(office::attr(reply, QStringLiteral("authorId")));
            c.replies.append(deck::CommentReply{newId("r"), by.name, by.initials, commentDate(office::attr(reply, QStringLiteral("created")), std::nullopt),
                                                deck::commentText(commentBody(office::child(reply, kP188, QStringLiteral("txBody"))))});
        }
        out->append(c);
    }
}

class Importer {
public:
    Importer(const office::Package &pkg, Document &doc) : m_pkg(pkg), m_doc(doc)
    {
        using B = office::DrawingBudget;
        m_budget.left[B::Objects] = kMaxObjects;
        m_budget.left[B::Stories] = kMaxStories;
        m_budget.left[B::Cells] = kMaxCells;
        m_budget.left[B::Paragraphs] = kMaxParagraphs;
        m_budget.left[B::Runs] = kMaxRuns;
        m_budget.left[B::Letters] = kMaxLetters;
    }
    bool run(const QString &mainPart, QString *error);
    // The fonts the file carried, once, for EmbeddedFonts to take.
    QVector<EmbeddedFace> takeFonts() { return std::exchange(m_fonts, QVector<EmbeddedFace>()); }

private:
    enum class Kind { Master, Layout, Slide };
    // A theme part as read, once however many slide masters name it.
    struct ThemeCtx {
        office::Theme theme;
        QDomElement formats;
        QString base64;       // the part, as a master keeps it
        QString why;          // what is wrong with it, if it can't be used
        bool ok = false;
    };
    struct MasterCtx {
        std::shared_ptr<MasterPage> page;
        QString part;
        QString themePart;    // its pictures' relationships are the theme part's
        office::Theme theme;
        QDomElement formats;
        office::ColorMap colors;
        QDomElement titleStyle, bodyStyle, otherStyle;
        PhList placeholders;
    };
    struct LayoutCtx {
        std::shared_ptr<MasterPage> page;
        QString part;
        MasterCtx *master = nullptr;
        office::ColorMap colors;
        PhList placeholders;
    };
    // The page being read: where its items go and what it stands on.
    struct Scope {
        Kind kind = Kind::Slide;
        PageBase *page = nullptr;
        office::DrawingContext ctx;
        const MasterCtx *master = nullptr;
        const LayoutCtx *layout = nullptr;
        QJsonArray unread, charts;
    };

    QDomDocument &newDom()
    {
        m_doms.emplace_back();
        return m_doms.back();
    }
    void note(const QString &text);
    bool parsePart(const QString &part, const QString &root, const char *rootNs, QDomDocument *dom, QDomElement *rootElement, QString *why);
    office::DrawingContext contextFor(const MasterCtx &master, const office::ColorMap &colors, const QString &part);
    std::shared_ptr<const ThemeCtx> themeOf(const QString &part);
    void sayWhatRanOut();
    office::ColorMap colorsOf(const QDomElement &clrMapOvr, const office::ColorMap &inherited);
    QString addImage(const QByteArray &bytes, const QString &format);

    void readSize(const QDomElement &presentation);
    void readSections(const QDomElement &presentation, const QHash<qint64, int> &pageOfSlideId);
    void readShowSettings(const QDomElement &presentation, const QHash<QString, int> &pageOfRel);
    MasterCtx *readMaster(const QString &part);
    LayoutCtx *readLayout(const QString &part, MasterCtx &master);
    bool readSlide(int number, const QString &part);
    void readNotes(const QString &slidePart, Page &page, const MasterCtx &master);
    void readComments(const QString &slidePart, int number, Page &page);

    PhList placeholdersIn(const QDomElement &tree);
    void readTree(const QDomElement &tree, Scope &sc);
    // readTree for a whole page: within what the file has left, held to what one page may keep (kMaxPageObjects...).
    void readPageTree(const QDomElement &tree, Scope &sc);
    void readChild(const QDomElement &c, Scope &sc);
    void readAlternate(const QDomElement &alternate, Scope &sc);
    ItemPtr readOne(const QDomElement &shape, Scope &sc);
    bool readFramePicture(const QDomElement &frame, Scope &sc);
    bool readFrameTable(const QDomElement &frame, Scope &sc);
    bool readFrameChart(const QDomElement &frame, Scope &sc);
    bool readFrameDiagram(const QDomElement &frame, Scope &sc);
    bool readEquation(const QDomElement &alternate, Scope &sc);
    QString drawingLook(const Scope &sc, const QString &part, const QRectF &rect) const;
    bool keepChartPart(const QString &part, QJsonObject *parts, QSet<QString> *seen, qint64 *bytes, int depth);
    QDomElement tableStyles();
    void fitRows(TableItem &t);
    void keepUnread(const QDomElement &whole, const QDomElement &frame, Scope &sc);
    QDomElement withEmptyText(const QDomElement &shape);
    QDomElement textStyleFor(const MasterCtx &m, const QString &type) const;

    const office::Package &m_pkg;
    Document &m_doc;
    std::deque<QDomDocument> m_doms;
    QStringList m_notes;
    QVector<EmbeddedFace> m_fonts;
    int m_extraNotes = 0;
    bool m_layoutLimitSaid = false;
    QDomElement m_defaultStyle;
    office::DrawingBudget m_budget;               // what the whole file may keep (kMaxObjects...), drawn from by every read of it
    bool m_pageFull = false;                      // a page held more than one may keep
    // The charts drawn so far, by part, size, and look (readFrameChart), and the parts kept behind each chart part.
    struct ChartDrawing {
        QString imageId;
        bool coarser = false;
    };
    QHash<QString, ChartDrawing> m_chartDrawings;
    QHash<QString, std::optional<QJsonObject>> m_chartParts;
    qint64 m_diagramBytes = 0;                    // what the diagrams' kept parts come to, in all
    int m_diagramsDrawn = 0;                      // the diagrams drawn so far, and what their drawings come to
    qint64 m_diagramSvgBytes = 0;
    bool m_diagramsRanOut = false;
    QHash<QString, QString> m_images;             // the pictures and movies kept, by part and link, for every shape that uses one
    std::vector<std::unique_ptr<MasterCtx>> m_masters;
    std::vector<std::unique_ptr<LayoutCtx>> m_layouts;
    QHash<QString, MasterCtx *> m_masterByPart;   // by lower-cased part name; null for one that couldn't be read
    QHash<QString, LayoutCtx *> m_layoutByPart;   // by lower-cased part name; null for one that couldn't be read
    QHash<QString, std::shared_ptr<const ThemeCtx>> m_themes;   // by lower-cased part name
    QHash<QString, int> m_slideByPart;            // by lower-cased part name: the number of the slide that has it
    QSet<QString> m_notesParts;                   // the notes parts read, by lower-cased name
    QHash<QString, QDomElement> m_notesStyles;    // by notes master part
    int m_layoutCount = 0;
    bool m_footers = false;
    QString m_mainPart;
    std::unique_ptr<CommentReader> m_comments;
    bool m_tableStylesRead = false;
    QDomElement m_tableStyles;                    // the root of the table styles part, once read; null if there is none
};

void Importer::note(const QString &text)
{
    if (m_notes.size() < kMaxNotes) m_notes << text;
    else ++m_extraNotes;
}

// A note for each kind of thing the file held more of than it may.
void Importer::sayWhatRanOut()
{
    using B = office::DrawingBudget;
    if (m_budget.ranOut[B::Objects]) note(QCoreApplication::translate("PptxImport", "The file holds more than %1 objects in all; the rest are left out.").arg(kMaxObjects));
    if (m_budget.ranOut[B::Stories]) note(QCoreApplication::translate("PptxImport", "The file holds more than %1 shapes with text in all; the text of the rest is left out.").arg(kMaxStories));
    if (m_budget.ranOut[B::Cells]) note(QCoreApplication::translate("PptxImport", "The file holds more than %1 table cells in all; the tables past them are kept as written, not shown.").arg(kMaxCells));
    if (m_budget.ranOut[B::Paragraphs]) note(QCoreApplication::translate("PptxImport", "The file holds more than %1 paragraphs of text in all; the rest are left out.").arg(kMaxParagraphs));
    if (m_budget.ranOut[B::Runs]) note(QCoreApplication::translate("PptxImport", "The file holds more than %1 runs of text in all; the rest are left out.").arg(kMaxRuns));
    if (m_budget.ranOut[B::Letters]) note(QCoreApplication::translate("PptxImport", "The file holds more than %1 letters of text in all; the rest are left out.").arg(kMaxLetters));
    if (m_budget.longBody) note(QCoreApplication::translate("PptxImport", "Some text is longer than a text box is read to, so the rest of it is left out."));
    if (m_diagramsRanOut)
        note(QCoreApplication::translate("PptxImport", "The file holds more than %1 diagrams, or diagrams that are drawn from more than %2 MB in all; the rest are kept as written, not shown.").arg(kMaxDiagrams).arg(kMaxDiagramSvgBytes >> 20));
    if (m_pageFull)
        note(QCoreApplication::translate("PptxImport", "A slide is read to %1 objects, %2 shapes with text, and %3 letters; some slides here hold more, and the rest of them is left out.")
                 .arg(kMaxPageObjects)
                 .arg(kMaxPageStories)
                 .arg(kMaxPageLetters));
}

bool Importer::parsePart(const QString &part, const QString &root, const char *rootNs, QDomDocument *dom, QDomElement *rootElement, QString *why)
{
    const QByteArray xml = m_pkg.part(part);
    if (xml.isEmpty()) {
        *why = QCoreApplication::translate("PptxImport", "the part is missing from the file");
        return false;
    }
    if (!office::parseXml(xml, dom, why)) return false;
    *rootElement = dom->documentElement();
    if (!office::isElement(*rootElement, QLatin1String(rootNs), root)) {
        *why = QCoreApplication::translate("PptxImport", "the part is not what the file says it is");
        return false;
    }
    return true;
}

office::ColorMap Importer::colorsOf(const QDomElement &clrMapOvr, const office::ColorMap &inherited)
{
    const QDomElement over = aChild(clrMapOvr, "overrideClrMapping");
    return over.isNull() ? inherited : office::readColorMap(over);
}

office::DrawingContext Importer::contextFor(const MasterCtx &master, const office::ColorMap &colors, const QString &part)
{
    office::DrawingContext ctx;
    ctx.package = &m_pkg;
    ctx.part = part;
    ctx.theme = master.theme;
    ctx.colors = colors;
    ctx.formats = master.formats;
    ctx.themePart = master.themePart;
    ctx.images = &m_images;
    ctx.budget = &m_budget;
    ctx.addImage = [this](const QByteArray &bytes, const QString &format) { return addImage(bytes, format); };
    ctx.addMedia = [this](const QByteArray &bytes, const QString &contentType, const QString &extension) {
        return m_doc.addMedia(bytes, contentType, extension);
    };
    ctx.newStory = [this] { return m_doc.createStory(); };
    ctx.storyDoc = [this](const QString &id) { return m_doc.storyDoc(id); };
    return ctx;
}

QString Importer::addImage(const QByteArray &bytes, const QString &format)
{
    return m_doc.addImage(bytes, format);
}

void Importer::readSize(const QDomElement &presentation)
{
    const QDomElement sz = pChild(presentation, "sldSz");
    const qint64 cx = office::parseInt(office::attr(sz, QStringLiteral("cx"))), cy = office::parseInt(office::attr(sz, QStringLiteral("cy")));
    QSizeF size(kSlideWidth, kSlideHeight);
    QString name = QStringLiteral("Widescreen");
    if (!sz.isNull()) {
        if (cx >= kMinSlideEmu && cx <= kMaxSlideEmu && cy >= kMinSlideEmu && cy <= kMaxSlideEmu) {
            size = QSizeF(office::emuToPoints(cx), office::emuToPoints(cy));
            name = sizeNameOf(office::attr(sz, QStringLiteral("type")), cx, cy);
        } else {
            note(QCoreApplication::translate("PptxImport", "The slide size in the file can't be used, so the slides are 13.333 by 7.5 inches."));
        }
    }
    m_doc.setup.size = m_doc.setup.sheet = size;
    m_doc.setup.sizeName = name;
    m_doc.setup.margins = QMarginsF(0, 0, 0, 0);
    m_doc.setup.firstPageNumber = int(std::clamp<qint64>(office::parseInt(office::attr(presentation, QStringLiteral("firstSlideNum")), 1), 0, 9999));
    const QDomElement notesSz = pChild(presentation, "notesSz");
    const qint64 nx = office::parseInt(office::attr(notesSz, QStringLiteral("cx"))), ny = office::parseInt(office::attr(notesSz, QStringLiteral("cy")));
    if (nx > 0 && ny > 0)
        m_doc.appData[QStringLiteral("notesSize")] = QJsonObject{{"w", office::emuToPoints(nx)}, {"h", office::emuToPoints(ny)}};
}

// The sections the presentation lists in its extensions, each with the slides it names: kept as how many slides each holds, in slide order. A slide
// that none names joins the section before it, and those before the first section's slides are a "Default Section" made for them; a section can't
// come before an earlier one's slides.
void Importer::readSections(const QDomElement &presentation, const QHash<qint64, int> &pageOfSlideId)
{
    QDomElement list;
    for (const QDomElement &ext : office::children(pChild(presentation, "extLst"), office::ns::P, QStringLiteral("ext")))
        if (office::attr(ext, QStringLiteral("uri")).compare(kSectionExt, Qt::CaseInsensitive) == 0) {
            list = office::child(ext, kSectionNs, QStringLiteral("sectionLst"));
            break;
        }
    QVector<Section> sections;
    QVector<int> sectionOfPage(int(m_doc.pages.size()), -1);
    for (const QDomElement &s : office::children(list, kSectionNs, QStringLiteral("section"))) {
        QString id = office::attr(s, QStringLiteral("id")), name = office::attr(s, QStringLiteral("name"));
        sections << Section{id.isEmpty() ? newSectionId() : id, name.isEmpty() ? untitledSectionName() : name, 0};
        for (const QDomElement &slide : office::children(office::child(s, kSectionNs, QStringLiteral("sldIdLst")), kSectionNs, QStringLiteral("sldId"))) {
            const int page = pageOfSlideId.value(office::parseInt(office::attr(slide, QStringLiteral("id")), -1), -1);
            if (page >= 0 && sectionOfPage[page] < 0) sectionOfPage[page] = int(sections.size()) - 1;
        }
    }
    if (sections.isEmpty()) return;
    if (!sectionOfPage.isEmpty() && sectionOfPage.first() < 0) {
        sections.prepend(Section{newSectionId(), defaultSectionName(), 0});
        for (int &section : sectionOfPage)
            if (section >= 0) ++section;
    }
    int current = 0;
    for (int section : std::as_const(sectionOfPage)) {
        current = std::max(current, section);
        sections[current].count++;
    }
    setSections(m_doc, sections);
}

// The custom slide shows, and the slide show's settings from the presentation properties (ppt/presProps.xml).
void Importer::readShowSettings(const QDomElement &presentation, const QHash<QString, int> &pageOfRel)
{
    QStringList notes;
    readCustomShows(presentation, pageOfRel, m_doc, &notes);
    const QVector<office::Relationship> rels = m_pkg.relationshipsOfType(m_mainPart, QStringLiteral("/presProps"));
    if (!rels.isEmpty() && !rels[0].external) {
        QDomDocument &dom = newDom();
        QDomElement root;
        QString why;
        if (parsePart(rels[0].target, QStringLiteral("presentationPr"), "http://schemas.openxmlformats.org/presentationml/2006/main", &dom, &root, &why))
            readShowProperties(root, m_masters.empty() ? office::defaultTheme() : m_masters.front()->theme, [](const QDomElement &e) { return standaloneXml(e); }, m_doc, &notes);
        else
            notes << QCoreApplication::translate("PptxImport", "The slide show settings can't be read (%1), so the defaults are used.").arg(why);
    }
    for (const QString &text : std::as_const(notes)) note(text);
}

PhList Importer::placeholdersIn(const QDomElement &tree)
{
    QVector<PhRef> out;
    for (QDomElement c = tree.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        PhRef ph;
        if (isOneOf(c.localName(), {"sp", "pic", "graphicFrame"}) && placeholderOf(c, &ph)) out << ph;
    }
    return PhList(std::move(out));
}

QDomElement Importer::textStyleFor(const MasterCtx &m, const QString &type) const
{
    if (type == QLatin1String("title") || type == QLatin1String("ctrTitle")) return m.titleStyle;
    if (family(type, true) == QLatin1String("body")) return m.bodyStyle;
    return m.otherStyle;
}

// A theme part, read the first time a master names it and shared by every master that does.
std::shared_ptr<const Importer::ThemeCtx> Importer::themeOf(const QString &part)
{
    const QString key = part.toLower();
    if (const std::shared_ptr<const ThemeCtx> known = m_themes.value(key)) return known;
    auto t = std::make_shared<ThemeCtx>();
    t->theme = office::defaultTheme();
    const QByteArray bytes = m_pkg.part(part);
    if (bytes.size() > kMaxThemeBytes) {
        t->why = QCoreApplication::translate("PptxImport", "it is larger than %1 KB").arg(kMaxThemeBytes >> 10);
    } else {
        QDomDocument &dom = newDom();
        office::Theme theme;
        if (office::readTheme(bytes, &theme, &t->why) && office::parseXml(bytes, &dom, &t->why)) {
            t->theme = theme;
            t->formats = aChild(aChild(dom.documentElement(), "themeElements"), "fmtScheme");
            t->base64 = QString::fromLatin1(bytes.toBase64());
            t->ok = true;
        }
    }
    m_themes.insert(key, t);
    return t;
}

Importer::MasterCtx *Importer::readMaster(const QString &part)
{
    QDomDocument &dom = newDom();
    QDomElement root;
    QString why;
    if (!parsePart(part, QStringLiteral("sldMaster"), "http://schemas.openxmlformats.org/presentationml/2006/main", &dom, &root, &why)) {
        note(QCoreApplication::translate("PptxImport", "The slide master %1 was left out: %2").arg(shortName(part), why));
        return nullptr;
    }
    auto m = std::make_unique<MasterCtx>();
    m->part = part;
    m->theme = office::defaultTheme();
    const QVector<office::Relationship> themes = m_pkg.relationshipsOfType(part, QStringLiteral("/theme"));
    QString themeBase64;   // the theme part, as the master keeps it; empty if it has none that can be read
    if (themes.isEmpty() || themes[0].external) {
        note(QCoreApplication::translate("PptxImport", "The slide master %1 has no theme, so JeffDeck's colors and fonts stand in.").arg(shortName(part)));
    } else if (const std::shared_ptr<const ThemeCtx> theme = themeOf(themes[0].target); !theme->ok) {
        note(QCoreApplication::translate("PptxImport", "The theme of slide master %1 can't be read (%2), so JeffDeck's colors and fonts stand in.").arg(shortName(part), theme->why));
    } else {
        m->theme = theme->theme;
        m->formats = theme->formats;
        m->themePart = themes[0].target;
        themeBase64 = theme->base64;
    }
    m->colors = office::readColorMap(pChild(root, "clrMap"));
    const QDomElement styles = pChild(root, "txStyles");
    m->titleStyle = pChild(styles, "titleStyle");
    m->bodyStyle = pChild(styles, "bodyStyle");
    m->otherStyle = pChild(styles, "otherStyle");

    const int n = int(m_masters.size()) + 1;
    auto page = std::make_shared<MasterPage>();
    page->id = QStringLiteral("SM%1").arg(n);
    const QDomElement cSld = pChild(root, "cSld");
    page->name = office::attr(cSld, QStringLiteral("name"));
    if (page->name.isEmpty()) page->name = m->theme.name.isEmpty() ? QCoreApplication::translate("PptxImport", "Slide Master") : QCoreApplication::translate("PptxImport", "%1 Slide Master").arg(m->theme.name);
    page->abbr = n == 1 ? QStringLiteral("SM") : QStringLiteral("SM%1").arg(n);
    page->props[QStringLiteral("kind")] = QStringLiteral("master");
    if (!office::parseBool(office::attr(root, QStringLiteral("preserve")), true)) page->props[QStringLiteral("preserve")] = false;
    if (n > 1 && !themeBase64.isEmpty()) page->props[QStringLiteral("themeXml")] = themeBase64;
    m->page = page;

    Scope sc;
    sc.kind = Kind::Master;
    sc.page = page.get();
    sc.master = m.get();
    sc.ctx = contextFor(*m, m->colors, part);
    if (const QDomElement bg = pChild(cSld, "bg"); !bg.isNull()) page->background = office::readBackground(bg, sc.ctx);
    sc.ctx.background = page->background;
    const QDomElement tree = pChild(cSld, "spTree");
    m->placeholders = placeholdersIn(tree);
    readPageTree(tree, sc);

    MasterCtx *raw = m.get();
    m_masters.push_back(std::move(m));
    m_doc.masters << page;
    if (m_masters.size() == 1) {
        // The first master's theme is the presentation's.
        DeckTheme t = defaultTheme();
        t.name = raw->theme.name.isEmpty() ? QCoreApplication::translate("PptxImport", "Theme") : raw->theme.name;
        for (int i = 0; i < 12; ++i) t.colors[i] = raw->theme.colors[i];
        if (!raw->theme.majorLatin.isEmpty()) t.headingFont = raw->theme.majorLatin;
        if (!raw->theme.minorLatin.isEmpty()) t.bodyFont = raw->theme.minorLatin;
        setTheme(m_doc, t);
        m_doc.fonts = {t.name, t.headingFont, t.bodyFont};
        if (!themeBase64.isEmpty()) m_doc.appData[QStringLiteral("themeXml")] = themeBase64;
    }

    // Its layouts, in the order it lists them, right after it.
    for (const QDomElement &id : office::children(pChild(root, "sldLayoutIdLst"), office::ns::P, QStringLiteral("sldLayoutId"))) {
        const office::Relationship rel = m_pkg.relationship(part, office::attrNs(id, office::ns::R, QStringLiteral("id")));
        if (rel.id.isEmpty() || rel.external) continue;
        readLayout(rel.target, *raw);
    }
    return raw;
}

Importer::LayoutCtx *Importer::readLayout(const QString &part, MasterCtx &master)
{
    const QString key = part.toLower();
    if (m_layoutByPart.contains(key)) return m_layoutByPart.value(key);
    if (m_layoutCount >= kMaxLayouts) {
        if (!m_layoutLimitSaid) note(QCoreApplication::translate("PptxImport", "Only the first %1 layouts are read.").arg(kMaxLayouts));
        m_layoutLimitSaid = true;
        return nullptr;
    }
    m_layoutByPart.insert(key, nullptr);
    QDomDocument &dom = newDom();
    QDomElement root;
    QString why;
    if (!parsePart(part, QStringLiteral("sldLayout"), "http://schemas.openxmlformats.org/presentationml/2006/main", &dom, &root, &why)) {
        note(QCoreApplication::translate("PptxImport", "The layout %1 was left out: %2").arg(shortName(part), why));
        return nullptr;
    }
    // A layout stands on the master that lists it, unless its own link names another that was read.
    MasterCtx *base = &master;
    const QVector<office::Relationship> own = m_pkg.relationshipsOfType(part, QStringLiteral("/slideMaster"));
    if (!own.isEmpty())
        for (const auto &m : m_masters)
            if (m->part.compare(own[0].target, Qt::CaseInsensitive) == 0) base = m.get();

    auto l = std::make_unique<LayoutCtx>();
    l->part = part;
    l->master = base;
    l->colors = colorsOf(pChild(root, "clrMapOvr"), base->colors);
    const int n = ++m_layoutCount;
    auto page = std::make_shared<MasterPage>();
    page->id = QStringLiteral("L%1").arg(n);
    const QDomElement cSld = pChild(root, "cSld");
    page->name = office::attr(cSld, QStringLiteral("name"));
    if (page->name.isEmpty()) page->name = QCoreApplication::translate("PptxImport", "Layout %1").arg(n);
    page->abbr = QString::number(n);
    page->baseId = base->page->id;
    page->showMasterItems = office::parseBool(office::attr(root, QStringLiteral("showMasterSp")), true);
    page->props[QStringLiteral("kind")] = QStringLiteral("layout");
    if (!office::parseBool(office::attr(root, QStringLiteral("preserve")), true)) page->props[QStringLiteral("preserve")] = false;
    page->props[QStringLiteral("type")] = office::attr(root, QStringLiteral("type"), QStringLiteral("cust"));
    l->page = page;

    Scope sc;
    sc.kind = Kind::Layout;
    sc.page = page.get();
    sc.master = base;
    sc.ctx = contextFor(*base, l->colors, part);
    if (const QDomElement bg = pChild(cSld, "bg"); !bg.isNull()) page->background = office::readBackground(bg, sc.ctx);
    sc.ctx.background = !page->background.isNone() ? page->background : base->page->background;
    const QDomElement tree = pChild(cSld, "spTree");
    l->placeholders = placeholdersIn(tree);
    readPageTree(tree, sc);

    LayoutCtx *raw = l.get();
    m_layouts.push_back(std::move(l));
    m_layoutByPart.insert(key, raw);
    m_doc.masters << page;
    return raw;
}

void Importer::readPageTree(const QDomElement &tree, Scope &sc)
{
    using B = office::DrawingBudget;
    static constexpr qint64 caps[B::KindCount] = {kMaxPageObjects, kMaxPageStories, kMaxPageCells, kMaxPageParagraphs, kMaxPageRuns, kMaxPageLetters};
    // The page reads from the smaller of what the file has left and what a page may keep; the file pays for what the page took.
    qint64 fileLeft[B::KindCount], given[B::KindCount];
    bool fileRanOut[B::KindCount];
    for (int k = 0; k < B::KindCount; ++k) {
        fileLeft[k] = m_budget.left[k];
        fileRanOut[k] = m_budget.ranOut[k];
        given[k] = std::min(fileLeft[k], caps[k]);
        m_budget.left[k] = given[k];
        m_budget.ranOut[k] = false;
    }
    readTree(tree, sc);
    for (int k = 0; k < B::KindCount; ++k) {
        if (m_budget.ranOut[k]) {
            if (caps[k] < fileLeft[k]) m_pageFull = true;
            else fileRanOut[k] = true;
        }
        m_budget.left[k] = fileLeft[k] - (given[k] - m_budget.left[k]);
        m_budget.ranOut[k] = fileRanOut[k];
    }
}

void Importer::readTree(const QDomElement &tree, Scope &sc)
{
    for (QDomElement c = tree.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        // Past the file's objects, what is left of the tree has nothing to read.
        if (m_budget.left[office::DrawingBudget::Objects] <= 0 && isOneOf(c.localName(), {"sp", "pic", "grpSp", "cxnSp", "graphicFrame", "contentPart", "AlternateContent"})) {
            m_budget.ranOut[office::DrawingBudget::Objects] = true;
            break;
        }
        readChild(c, sc);
    }
    if (!sc.unread.isEmpty()) sc.page->props[QStringLiteral("unread")] = sc.unread;
    if (!sc.charts.isEmpty()) sc.page->props[QStringLiteral("charts")] = sc.charts;
}

void Importer::readChild(const QDomElement &c, Scope &sc)
{
    const QString name = c.localName();
    if (office::isElement(c, kMc, QStringLiteral("AlternateContent"))) {
        readAlternate(c, sc);
    } else if (name == QLatin1String("graphicFrame") || name == QLatin1String("contentPart")) {
        if (!m_budget.take(office::DrawingBudget::Objects)) return;
        if (!readFrameTable(c, sc) && !readFrameChart(c, sc) && !readFrameDiagram(c, sc) && !readFramePicture(c, sc)) keepUnread(c, c, sc);
    } else if (isOneOf(name, {"sp", "pic", "grpSp", "cxnSp"})) {
        if (ItemPtr it = readOne(c, sc)) sc.page->items.push_back(it);
    }
}

// Alternatives: what the reader can use of the choices, else a picture in the
// fallback, else the whole is kept as written.
void Importer::readAlternate(const QDomElement &alternate, Scope &sc)
{
    if (!m_budget.take(office::DrawingBudget::Objects)) return;
    if (readEquation(alternate, sc)) return;
    QDomElement fallback, frame;
    bool read = false;
    for (QDomElement alt = alternate.firstChildElement(); !alt.isNull() && !read; alt = alt.nextSiblingElement()) {
        if (office::isElement(alt, kMc, QStringLiteral("Fallback"))) {
            if (fallback.isNull()) fallback = alt;
            continue;
        }
        if (!office::isElement(alt, kMc, QStringLiteral("Choice"))) continue;
        for (QDomElement e = alt.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (isOneOf(e.localName(), {"graphicFrame", "contentPart"})) {
                if (readFrameTable(e, sc) || readFrameChart(e, sc) || readFrameDiagram(e, sc) || readFramePicture(e, sc)) read = true;
                else if (frame.isNull()) frame = e;
            } else if (isOneOf(e.localName(), {"sp", "pic", "grpSp", "cxnSp"})) {
                read = true;
                if (ItemPtr it = readOne(e, sc)) sc.page->items.push_back(it);
            }
        }
    }
    if (read) return;
    for (QDomElement e = fallback.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        if (e.localName() != QLatin1String("pic")) continue;
        if (ItemPtr it = readOne(e, sc)) {
            sc.page->items.push_back(it);
            read = true;
        }
    }
    if (!read) keepUnread(alternate, frame.isNull() ? alternate : frame, sc);
}

// The table styles part (a:tblStyleLst), read once; null if the presentation has none.
QDomElement Importer::tableStyles()
{
    if (m_tableStylesRead) return m_tableStyles;
    m_tableStylesRead = true;
    const QVector<office::Relationship> rels = m_pkg.relationshipsOfType(m_mainPart, QStringLiteral("/tableStyles"));
    if (rels.isEmpty() || rels[0].external) return m_tableStyles;
    QDomDocument &dom = newDom();
    QString why;
    if (!parsePart(rels[0].target, QStringLiteral("tblStyleLst"), "http://schemas.openxmlformats.org/drawingml/2006/main", &dom, &m_tableStyles, &why)) {
        m_tableStyles = QDomElement();
        note(QCoreApplication::translate("PptxImport", "The table styles of the presentation can't be read (%1), so its tables take the usual style.").arg(why));
    }
    return m_tableStyles;
}

// The number a file gave a frame (0 if it gave none that can be used), which an animation aims at it by.
static int frameNumber(const QDomElement &frame)
{
    const qint64 n = office::parseInt(office::attr(pChild(pChild(frame, "nvGraphicFramePr"), "cNvPr"), QStringLiteral("id")), 0);
    return n > 0 && n <= INT_MAX ? int(n) : 0;
}

// A frame that holds a table: the table, in the place of the frame.
bool Importer::readFrameTable(const QDomElement &frame, Scope &sc)
{
    if (!office::isTableFrame(frame)) return false;
    office::TextStyleStack styles;
    if (!m_defaultStyle.isNull()) styles << m_defaultStyle;
    ItemPtr it = office::readTable(frame, sc.ctx, styles, tableStyles());
    if (!it) return false;
    fitRows(static_cast<TableItem &>(*it));
    it->sourceId = frameNumber(frame);
    PhRef ph;
    if (placeholderOf(frame, &ph)) it->role = roleOf(ph);
    sc.page->items.push_back(it);
    return true;
}

// The height a row has in the file is the least it can be: it grows to hold the
// text of its cells (as Editor::fitTableRows does as text is typed), and the
// table with it. Cells that span rows, or turn their text, don't make it grow.
void Importer::fitRows(TableItem &t)
{
    if (!t.growToFit) return;
    LayoutEnv env = Renderer::layoutEnv(m_doc);
    bool grew = false;
    for (int r = 0; r < t.rows; ++r) {
        double need = 0;
        for (int c = 0; c < t.cols; ++c) {
            const TableCell &cell = t.cell(r, c);
            const QTextDocument *story = cell.covered || cell.rowSpan > 1 || cell.vertical || cell.divided() ? nullptr : m_doc.storyDoc(cell.storyId);
            if (!story) continue;
            FrameSpec spec;
            spec.size = QSizeF(t.cellRect(r, c).width(), 1e5);
            spec.insets = cell.margins;
            spec.hyphenate = cell.hyphenate;
            spec.hyphenZone = cell.hyphenZone;
            StoryLayout layout;
            layout.build(story, {spec}, env);
            need = std::max(need, layout.usedHeight(0));
        }
        if (need > t.rowH[r]) {
            t.rowH[r] = need;
            grew = true;
        }
    }
    if (grew) t.syncRect();
}

// A part and what it relates to, down the chain of relationships, for writing back: by part name, its content type, its bytes (base64), and its
// links (id, type, and the part or address they go to). False if the chain is too long or big, or leads to a part the file doesn't have.
bool Importer::keepChartPart(const QString &part, QJsonObject *parts, QSet<QString> *seen, qint64 *bytes, int depth)
{
    if (seen->contains(part.toLower())) return true;
    if (depth > kMaxChartDepth || seen->size() >= kMaxChartParts || !m_pkg.hasPart(part)) return false;
    seen->insert(part.toLower());
    const QByteArray data = m_pkg.part(part);
    *bytes += data.size();
    if (*bytes > kMaxChartBytes) return false;
    QJsonArray links;
    for (const office::Relationship &r : m_pkg.relationships(part)) {
        QJsonObject link{{"id", r.id}, {"type", r.type}, {"target", r.target}};
        if (r.external) link.insert(QStringLiteral("external"), true);
        else if (!keepChartPart(r.target, parts, seen, bytes, depth + 1)) return false;
        links.append(link);
    }
    parts->insert(part, QJsonObject{{"type", m_pkg.contentType(part)}, {"data", QString::fromLatin1(data.toBase64())}, {"rels", links}});
    return true;
}

// What a drawing of a part looks like where it is drawn: the part and the size, the theme's colors and fonts, and how the slide maps the colors.
QString Importer::drawingLook(const Scope &sc, const QString &part, const QRectF &rect) const
{
    QString look = part.toLower() + QLatin1Char('|') + QString::number(rect.width()) + QLatin1Char('x') + QString::number(rect.height());
    for (const QColor &c : sc.ctx.theme.colors) look += QLatin1Char('|') + c.name(QColor::HexArgb);
    look += QLatin1Char('|') + sc.ctx.theme.majorLatin + QLatin1Char('|') + sc.ctx.theme.minorLatin + QLatin1Char('|') + sc.ctx.themePart.toLower();
    const office::ColorMap &map = sc.ctx.colors;
    for (const auto slot : {map.bg1, map.tx1, map.bg2, map.tx2, map.accent[0], map.accent[1], map.accent[2], map.accent[3], map.accent[4], map.accent[5], map.hlink, map.folHlink})
        look += QLatin1Char('|') + QString::number(int(slot));
    return look;
}

// A frame that holds a chart: a picture of it (drawn from the numbers the chart part stores) in the frame's place. The parts of the chart, and the
// frame as written, are kept in the page's "charts" with the picture's id, so saving writes the chart itself again.
bool Importer::readFrameChart(const QDomElement &frame, Scope &sc)
{
    if (!office::isChartFrame(frame)) return false;
    const office::Relationship rel = m_pkg.relationship(sc.ctx.part, office::chartFrameRelId(frame));
    if (rel.id.isEmpty() || rel.external || !m_pkg.hasPart(rel.target)) return false;
    const QDomElement cNvPr = pChild(pChild(frame, "nvGraphicFramePr"), "cNvPr");
    if (office::parseBool(office::attr(cNvPr, QStringLiteral("hidden")))) return false;
    const QDomElement xfrm = pChild(frame, "xfrm"), off = aChild(xfrm, "off"), ext = aChild(xfrm, "ext");
    auto points = [](const QDomElement &e, const char *name) { return office::emuToPoints(office::parseInt(office::attr(e, QLatin1String(name)))); };
    // A frame is no larger than a slide can be (a chart is drawn that large, not as large as a file says it is).
    const QRectF rect(points(off, "x"), points(off, "y"), std::clamp(points(ext, "cx"), 0.0, office::kChartMaxSide), std::clamp(points(ext, "cy"), 0.0, office::kChartMaxSide));
    if (rect.width() < 1 || rect.height() < 1) return false;
    // A chart that many frames show is read and drawn once for each size and look.
    const QString look = drawingLook(sc, rel.target, rect);
    auto drawn = m_chartDrawings.constFind(look);
    if (drawn == m_chartDrawings.cend()) {
        office::Chart chart;
        ChartDrawing d;
        if (office::readChart(m_pkg.part(rel.target), sc.ctx.theme, sc.ctx.colors, &chart)) d.imageId = addImage(office::chartSvg(chart, rect.size(), &d.coarser), QStringLiteral("svg"));
        drawn = m_chartDrawings.insert(look, d);
    }
    if (drawn->imageId.isEmpty()) return false;

    auto pic = std::make_shared<PictureItem>();
    pic->name = office::attr(cNvPr, QStringLiteral("name"));
    pic->altText = office::attr(cNvPr, QStringLiteral("descr"));
    pic->rect = rect;
    pic->imgRect = QRectF(QPointF(0, 0), rect.size());
    pic->wrap.mode = Wrap::None;
    pic->imageId = drawn->imageId;
    pic->sourceId = frameNumber(frame);
    if (drawn->coarser) note(QCoreApplication::translate("PptxImport", "The chart \"%1\" has more points than a picture of it can show, so its picture is drawn from some of them.").arg(pic->name));
    PhRef ph;
    if (placeholderOf(frame, &ph)) pic->role = roleOf(ph);
    sc.page->items.push_back(pic);

    // The parts behind it are kept once too, and shared by the frames that show it.
    auto kept = m_chartParts.constFind(rel.target.toLower());
    if (kept == m_chartParts.cend()) {
        QJsonObject parts;
        QSet<QString> seen;
        qint64 bytes = 0;
        const bool ok = keepChartPart(rel.target, &parts, &seen, &bytes, 0);
        kept = m_chartParts.insert(rel.target.toLower(), ok ? std::optional<QJsonObject>(parts) : std::nullopt);
    }
    if (*kept)
        sc.charts.append(QJsonObject{{"item", pic->id}, {"main", rel.target}, {"frameXml", standaloneXml(frame)}, {"parts", **kept}});
    else
        note(QCoreApplication::translate("PptxImport", "The parts behind the chart \"%1\" are too many or too large to keep, so it is saved as a picture of the chart.").arg(pic->name));
    return true;
}

// A frame that holds a diagram (SmartArt): a picture of its drawing in the frame's place. The drawing is the shapes the presentation program drew the
// diagram with, set as slide shapes are and kept as an SVG (a picture that is printed as vectors, and drawn on screen at three pixels to the point).
// What the diagram is made of is kept in the page's "charts", with the picture's id: the frame as written, the slide's links to its four parts and
// its drawing, and the parts down to the pictures in the drawing, so saving writes the diagram itself again. A frame with no drawing (an older
// file's) is left to be kept as it is.
bool Importer::readFrameDiagram(const QDomElement &frame, Scope &sc)
{
    if (!isDiagramFrame(frame)) return false;
    const QDomElement cNvPr = pChild(pChild(frame, "nvGraphicFramePr"), "cNvPr");
    if (office::parseBool(office::attr(cNvPr, QStringLiteral("hidden")))) return false;
    const DiagramLinks ids = diagramFrameLinks(frame);
    if (!ids.complete()) return false;
    // The four parts the frame names, which the file has, and the drawing the data part names.
    struct Named {
        QString id;
        const char *suffix;
    };
    QJsonArray links;
    QStringList targets;
    for (const Named &n : {Named{ids.data, "/diagramData"}, Named{ids.layout, "/diagramLayout"}, Named{ids.style, "/diagramQuickStyle"}, Named{ids.colors, "/diagramColors"}}) {
        const office::Relationship r = m_pkg.relationship(sc.ctx.part, n.id);
        if (r.id.isEmpty() || r.external || !m_pkg.hasPart(r.target) || !r.type.endsWith(QLatin1String(n.suffix))) return false;
        links.append(QJsonObject{{"id", r.id}, {"type", r.type}, {"target", r.target}});
        targets << r.target;
    }
    const QString drawingId = diagramDrawingLinkId(m_pkg.part(targets[0]));
    const office::Relationship drawing = m_pkg.relationship(sc.ctx.part, drawingId);
    if (drawingId.isEmpty() || drawing.id.isEmpty() || drawing.external || !m_pkg.hasPart(drawing.target) || !drawing.type.endsWith(QLatin1String("/diagramDrawing"))) return false;
    links.append(QJsonObject{{"id", drawing.id}, {"type", drawing.type}, {"target", drawing.target}});
    targets << drawing.target;

    const QDomElement xfrm = pChild(frame, "xfrm"), off = aChild(xfrm, "off"), ext = aChild(xfrm, "ext");
    auto points = [](const QDomElement &e, const char *name) { return office::emuToPoints(office::parseInt(office::attr(e, QLatin1String(name)))); };
    // A frame is no larger than a slide can be (a diagram is drawn that large, not as large as a file says it is).
    const QRectF rect(points(off, "x"), points(off, "y"), std::clamp(points(ext, "cx"), 0.0, office::kChartMaxSide), std::clamp(points(ext, "cy"), 0.0, office::kChartMaxSide));
    if (rect.width() < 1 || rect.height() < 1) return false;
    // A diagram that many frames show is read and drawn once for each size and look.
    const QString look = drawingLook(sc, drawing.target, rect);
    auto drawn = m_chartDrawings.constFind(look);
    if (drawn == m_chartDrawings.cend() && m_diagramsDrawn >= kMaxDiagrams) {
        m_diagramsRanOut = true;
        return false;
    }
    if (drawn == m_chartDrawings.cend()) {
        ++m_diagramsDrawn;
        ChartDrawing d;
        QDomDocument dom;
        QDomElement root;
        QString why;
        if (parsePart(drawing.target, QStringLiteral("drawing"), "http://schemas.microsoft.com/office/drawing/2008/diagram", &dom, &root, &why)) {
            const QDomElement tree = office::child(root, kDiagramDrawingNs, QStringLiteral("spTree"));
            if (!tree.isNull()) {
                office::DrawingContext ctx = sc.ctx;
                ctx.part = drawing.target;
                office::TextStyleStack styles;
                if (!m_defaultStyle.isNull()) styles << m_defaultStyle;
                int leftOut = 0;
                const QByteArray svg = diagramSvg(tree, ctx, styles, rect.size(), m_doc, &leftOut);
                m_doc.imagesLeftOut += leftOut;
                if (m_diagramSvgBytes + svg.size() > kMaxDiagramSvgBytes) m_diagramsRanOut = true;
                else if (!(d.imageId = addImage(svg, QStringLiteral("svg"))).isEmpty()) m_diagramSvgBytes += svg.size();
            }
        }
        drawn = m_chartDrawings.insert(look, d);
    }
    if (drawn->imageId.isEmpty()) return false;

    auto pic = std::make_shared<PictureItem>();
    pic->name = office::attr(cNvPr, QStringLiteral("name"));
    pic->altText = office::attr(cNvPr, QStringLiteral("descr"));
    pic->rect = rect;
    pic->imgRect = QRectF(QPointF(0, 0), rect.size());
    pic->wrap.mode = Wrap::None;
    pic->imageId = drawn->imageId;
    PhRef ph;
    if (placeholderOf(frame, &ph)) pic->role = roleOf(ph);
    sc.page->items.push_back(pic);

    // What it is made of, kept once for the frames that show it, and within what the file's diagrams may keep in all.
    const QString key = targets.join(QLatin1Char('|')).toLower();
    auto kept = m_chartParts.constFind(key);
    if (kept == m_chartParts.cend()) {
        QJsonObject parts;
        QSet<QString> seen;
        qint64 bytes = 0;
        bool ok = true;
        for (const QString &target : std::as_const(targets)) ok = ok && keepChartPart(target, &parts, &seen, &bytes, 0);
        ok = ok && m_diagramBytes + bytes <= kMaxDiagramBytes;
        if (ok) m_diagramBytes += bytes;
        kept = m_chartParts.insert(key, ok ? std::optional<QJsonObject>(parts) : std::nullopt);
    }
    if (*kept)
        sc.charts.append(QJsonObject{{"item", pic->id}, {"kind", "diagram"}, {"frameXml", standaloneXml(frame)}, {"links", links}, {"parts", **kept},
                                     {"x", rect.x()}, {"y", rect.y()}, {"w", rect.width()}, {"h", rect.height()}, {"name", pic->name}, {"descr", pic->altText}});
    else
        note(QCoreApplication::translate("PptxImport", "The parts behind the diagram \"%1\" are too many or too large to keep, so it is saved as a picture of the diagram.").arg(pic->name));
    return true;
}

// Whether a shape's text holds math: a14:m among the runs of a paragraph.
bool holdsMath(const QDomElement &shape)
{
    for (QDomElement body = shape.firstChildElement(); !body.isNull(); body = body.nextSiblingElement()) {
        if (body.localName() != QLatin1String("txBody")) continue;
        for (QDomElement p = body.firstChildElement(); !p.isNull(); p = p.nextSiblingElement()) {
            if (p.localName() != QLatin1String("p")) continue;
            for (QDomElement r = p.firstChildElement(); !r.isNull(); r = r.nextSiblingElement())
                if (r.localName() == QLatin1String("m") && r.namespaceURI() == kA14) return true;
        }
    }
    return false;
}

// A picture in the place of a shape the picture fills, stretched over its frame; null for a shape with no picture fill.
ItemPtr pictureFilling(const Item &shape)
{
    if (shape.type() == ItemType::Picture) return nullptr;
    if (shape.fill.type != Fill::Picture || shape.fill.imageId.isEmpty()) return nullptr;
    auto pic = std::make_shared<PictureItem>();
    pic->name = shape.name;
    pic->altText = shape.altText;
    pic->rect = shape.rect;
    pic->rotation = shape.rotation;
    pic->flipH = shape.flipH;
    pic->flipV = shape.flipV;
    pic->sourceId = shape.sourceId;
    pic->stroke = shape.stroke;
    pic->fx = shape.fx;
    pic->wrap.mode = Wrap::None;
    pic->imageId = shape.fill.imageId;
    pic->blipEffects = shape.fill.blipEffects;
    pic->imgRect = QRectF(QPointF(0, 0), shape.rect.size());
    return pic;
}

// An equation: a shape whose text holds math (a14:m), which a newer program writes as a pair of alternatives, the shape with its math for the programs
// that know it, and for the others, in the fallback, a picture of the shape (a picture, or a shape filled with it). A text box with other text
// around an equation is written so too, the picture of the whole box. The picture is what the equation looks like here; the whole pair is kept in
// the page's "charts", with the picture's id, so saving writes the equation itself again. A pair with no picture to show is read as any other.
bool Importer::readEquation(const QDomElement &alternate, Scope &sc)
{
    bool math = false;
    QDomElement fallback;
    for (QDomElement alt = alternate.firstChildElement(); !alt.isNull(); alt = alt.nextSiblingElement()) {
        if (office::isElement(alt, kMc, QStringLiteral("Fallback"))) {
            if (fallback.isNull()) fallback = alt;
            continue;
        }
        if (!office::isElement(alt, kMc, QStringLiteral("Choice"))) continue;
        for (QDomElement e = alt.firstChildElement(); !e.isNull() && !math; e = e.nextSiblingElement())
            if (e.localName() == QLatin1String("sp") && holdsMath(e)) math = true;
    }
    if (!math || fallback.isNull()) return false;
    for (QDomElement e = fallback.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        if (!isOneOf(e.localName(), {"pic", "sp"})) continue;
        ItemPtr it = readOne(e, sc);
        if (!it) continue;
        ItemPtr pic = it->type() == ItemType::Picture ? it : pictureFilling(*it);
        // A shape's story is of no use now it is a picture.
        if (it != pic) {
            if (it->type() == ItemType::Text) m_doc.removeStory(static_cast<const TextItem &>(*it).storyId);
            if (it->type() == ItemType::Shape) m_doc.removeStory(static_cast<const ShapeItem &>(*it).storyId);
        }
        if (!pic || static_cast<const PictureItem &>(*pic).imageId.isEmpty()) continue;
        sc.page->items.push_back(pic);
        sc.charts.append(QJsonObject{{"item", pic->id}, {"kind", "equation"}, {"xml", standaloneXml(alternate)},
                                     {"x", pic->rect.x()}, {"y", pic->rect.y()}, {"w", pic->rect.width()}, {"h", pic->rect.height()},
                                     {"rotation", pic->rotation}, {"flipH", pic->flipH}, {"flipV", pic->flipV}});
        return true;
    }
    return false;
}

// What a frame shows for itself, when it holds a picture of it.
bool Importer::readFramePicture(const QDomElement &frame, Scope &sc)
{
    const QDomElement pic = previewPicture(aChild(frame, "graphic"));
    if (pic.isNull()) return false;
    ItemPtr it = readOne(pic, sc);
    if (!it) return false;
    sc.page->items.push_back(it);
    return true;
}

void Importer::keepUnread(const QDomElement &whole, const QDomElement &frame, Scope &sc)
{
    QDomElement xfrm = pChild(frame, "xfrm");
    if (xfrm.isNull()) xfrm = aChild(frame, "xfrm");
    const QDomElement off = aChild(xfrm, "off"), ext = aChild(xfrm, "ext");
    auto points = [](const QDomElement &e, const char *name) { return office::emuToPoints(office::parseInt(office::attr(e, QLatin1String(name)))); };
    sc.unread.append(QJsonObject{{"xml", standaloneXml(whole)},
                                 {"x", points(off, "x")},
                                 {"y", points(off, "y")},
                                 {"w", points(ext, "cx")},
                                 {"h", points(ext, "cy")}});
}

// A placeholder that has no text body yet, with one that is empty, so it can
// be typed in.
QDomElement Importer::withEmptyText(const QDomElement &shape)
{
    QDomDocument dom = shape.ownerDocument();
    QDomElement copy = shape.cloneNode(true).toElement();
    const bool strict = shape.namespaceURI() == office::ns::PStrict;
    const QString p = strict ? office::ns::PStrict : office::ns::P, a = strict ? office::ns::AStrict : office::ns::A;
    QDomElement body = dom.createElementNS(p, QStringLiteral("p:txBody"));
    body.appendChild(dom.createElementNS(a, QStringLiteral("a:bodyPr")));
    body.appendChild(dom.createElementNS(a, QStringLiteral("a:lstStyle")));
    body.appendChild(dom.createElementNS(a, QStringLiteral("a:p")));
    copy.appendChild(body);
    return copy;
}

ItemPtr Importer::readOne(const QDomElement &shape, Scope &sc)
{
    office::TextStyleStack styles;
    if (!m_defaultStyle.isNull()) styles << m_defaultStyle;
    PhRef ph;
    if (!placeholderOf(shape, &ph)) return office::readShape(shape, sc.ctx, styles);

    // Where it gets its place and its text formatting from: its layout's
    // placeholder, and that one's (else its own) on the master.
    const PhRef *layoutPh = nullptr, *masterPh = nullptr;
    if (sc.kind == Kind::Slide && sc.layout) layoutPh = sc.layout->placeholders.find(ph, true, false);
    if (sc.kind != Kind::Master && sc.master) masterPh = sc.master->placeholders.find(layoutPh ? *layoutPh : ph, false, true);
    QVector<QDomElement> inherits;
    if (layoutPh) inherits << layoutPh->shape;
    if (masterPh) inherits << masterPh->shape;
    if (sc.master) {
        if (const QDomElement base = textStyleFor(*sc.master, ph.type); !base.isNull()) styles << base;
    }
    for (const PhRef *from : {masterPh, layoutPh})
        if (from)
            if (const QDomElement lst = listStyleOf(from->shape); !lst.isNull()) styles << lst;
    if (const QDomElement lst = listStyleOf(shape); !lst.isNull()) styles << lst;

    // What it inherits that names other parts (a layout's picture fill) is named by its own part's relationships.
    office::DrawingContext ctx = sc.ctx;
    if (layoutPh && sc.layout) ctx.inheritedParts.append({layoutPh->shape.ownerDocument(), sc.layout->part});
    if (masterPh && sc.master) ctx.inheritedParts.append({masterPh->shape.ownerDocument(), sc.master->part});
    const bool text = shape.localName() == QLatin1String("sp") && holdsText(ph.type) && pChild(shape, "txBody").isNull();
    ItemPtr it = office::readShape(text ? withEmptyText(shape) : shape, ctx, styles, inherits);
    if (!it) return nullptr;
    it->role = roleOf(ph);
    if (sc.kind == Kind::Slide) {
        if (isOneOf(ph.type, {"dt", "ftr", "sldNum"})) m_footers = true;
        if (it->type() == ItemType::Text) {
            auto *t = static_cast<TextItem *>(it.get());
            const QTextDocument *story = m_doc.storyDoc(t->storyId);
            if (story && story->isEmpty()) t->prompt = promptFor(ph.type);
        }
    }
    return it;
}

bool Importer::readSlide(int number, const QString &part)
{
    QDomDocument dom;
    QDomElement root;
    QString why;
    if (!parsePart(part, QStringLiteral("sld"), "http://schemas.openxmlformats.org/presentationml/2006/main", &dom, &root, &why)) {
        note(QCoreApplication::translate("PptxImport", "Slide %1 was left out: %2").arg(number).arg(why));
        return false;
    }
    // The layout it stands on, and through it the slide master. One that no
    // master lists is read now, on the master its own link names.
    LayoutCtx *layout = nullptr;
    const QVector<office::Relationship> layoutRels = m_pkg.relationshipsOfType(part, QStringLiteral("/slideLayout"));
    if (!layoutRels.isEmpty() && !layoutRels[0].external) {
        const QString target = layoutRels[0].target;
        layout = m_layoutByPart.value(target.toLower());
        if (!layout && !m_layoutByPart.contains(target.toLower())) {
            MasterCtx *base = m_masters.front().get();
            const QVector<office::Relationship> masterRels = m_pkg.relationshipsOfType(target, QStringLiteral("/slideMaster"));
            if (!masterRels.isEmpty())
                for (const auto &m : m_masters)
                    if (m->part.compare(masterRels[0].target, Qt::CaseInsensitive) == 0) base = m.get();
            layout = readLayout(target, *base);
        }
    }
    const MasterCtx &master = layout ? *layout->master : *m_masters.front();

    auto page = std::make_shared<Page>();
    page->masterId = layout ? layout->page->id : master.page->id;
    page->hidden = !office::parseBool(office::attr(root, QStringLiteral("show")), true);
    page->showMasterItems = office::parseBool(office::attr(root, QStringLiteral("showMasterSp")), true);

    Scope sc;
    sc.kind = Kind::Slide;
    sc.page = page.get();
    sc.master = &master;
    sc.layout = layout;
    sc.ctx = contextFor(master, colorsOf(pChild(root, "clrMapOvr"), layout ? layout->colors : master.colors), part);
    const QDomElement cSld = pChild(root, "cSld");
    if (const QDomElement bg = pChild(cSld, "bg"); !bg.isNull()) page->background = office::readBackground(bg, sc.ctx);
    sc.ctx.background = !page->background.isNone() ? page->background
                      : layout && !layout->page->background.isNone() ? layout->page->background
                                                                      : master.page->background;
    readPageTree(pChild(cSld, "spTree"), sc);

    // How the slide changes to the next, and what moves on it, as written.
    for (const QDomElement &c : office::children(root)) {
        if (office::isElement(c, office::ns::P, QStringLiteral("transition"))) {
            page->props[QStringLiteral("transitionXml")] = standaloneXml(c);
        } else if (office::isElement(c, office::ns::P, QStringLiteral("timing"))) {
            page->props[QStringLiteral("timingXml")] = standaloneXml(c);
        } else if (office::isElement(c, kMc, QStringLiteral("AlternateContent"))) {
            // A transition with another form for programs that don't know the first.
            for (const QDomElement &alt : office::children(c))
                if (!pChild(alt, "transition").isNull()) page->props[QStringLiteral("transitionXml")] = standaloneXml(c);
        }
    }
    // The show plays it, and the Transitions tab shows it; saved again unchanged, the XML read is written back.
    deck::setTransition(*page, deck::transitionFromXml(page->props.value(QStringLiteral("transitionXml")).toString()));
    for (const auto &it : page->items) {
        const QString type = placeholderType(*it);
        if ((type == QLatin1String("title") || type == QLatin1String("ctrTitle")) && it->type() == ItemType::Text) {
            const QTextDocument *story = m_doc.storyDoc(static_cast<const TextItem *>(it.get())->storyId);
            if (story) page->title = story->toPlainText().section(QChar(QChar::LineSeparator), 0, 0).section(QLatin1Char('\n'), 0, 0).trimmed();
            break;
        }
    }
    readNotes(part, *page, master);
    readComments(part, number, *page);
    m_doc.pages << page;
    return true;
}

void Importer::readNotes(const QString &slidePart, Page &page, const MasterCtx &master)
{
    page.notesStoryId = m_doc.createStory();
    const QVector<office::Relationship> rels = m_pkg.relationshipsOfType(slidePart, QStringLiteral("/notesSlide"));
    if (rels.isEmpty() || rels[0].external) return;
    const QString part = rels[0].target;
    // A notes part belongs to one slide: another that names it has no notes of its own.
    if (m_notesParts.contains(part.toLower())) {
        note(QCoreApplication::translate("PptxImport", "The notes of %1 were left out: the part %2 is read once.").arg(shortName(slidePart), shortName(part)));
        return;
    }
    m_notesParts.insert(part.toLower());
    QDomDocument dom;
    QDomElement root;
    QString why;
    if (!parsePart(part, QStringLiteral("notes"), "http://schemas.openxmlformats.org/presentationml/2006/main", &dom, &root, &why)) {
        note(QCoreApplication::translate("PptxImport", "The notes of %1 can't be read: %2").arg(shortName(slidePart), why));
        return;
    }
    // The text of the notes placeholder, in the notes master's style.
    office::TextStyleStack styles;
    if (!m_defaultStyle.isNull()) styles << m_defaultStyle;
    const QVector<office::Relationship> masters = m_pkg.relationshipsOfType(part, QStringLiteral("/notesMaster"));
    if (!masters.isEmpty() && !masters[0].external) {
        const QString key = masters[0].target.toLower();
        if (!m_notesStyles.contains(key)) {
            QDomDocument &masterDom = newDom();
            QDomElement masterRoot;
            QString masterWhy;
            m_notesStyles.insert(key, parsePart(masters[0].target, QStringLiteral("notesMaster"), "http://schemas.openxmlformats.org/presentationml/2006/main", &masterDom, &masterRoot, &masterWhy)
                                          ? pChild(masterRoot, "notesStyle") : QDomElement());
        }
        if (const QDomElement style = m_notesStyles.value(key); !style.isNull()) styles << style;
    }
    const QDomElement tree = pChild(pChild(root, "cSld"), "spTree");
    for (const QDomElement &sp : office::children(tree, office::ns::P, QStringLiteral("sp"))) {
        PhRef ph;
        if (!placeholderOf(sp, &ph) || ph.type != QLatin1String("body")) continue;
        const QDomElement body = pChild(sp, "txBody");
        QTextDocument *story = m_doc.storyDoc(page.notesStoryId);
        if (body.isNull() || !story) break;
        if (const QDomElement lst = listStyleOf(sp); !lst.isNull()) styles << lst;
        office::readTextBody(body, story, contextFor(master, master.colors, part), styles);
        break;
    }
}

void Importer::readComments(const QString &slidePart, int number, Page &page)
{
    if (!m_comments) m_comments = std::make_unique<CommentReader>(m_pkg, m_mainPart, [this](const QString &text) { note(text); });
    m_comments->read(slidePart, number, page, m_doc.pageSize());
}

bool Importer::run(const QString &mainPart, QString *error)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    QDomDocument &dom = newDom();
    QDomElement root;
    QString why;
    if (!parsePart(mainPart, QStringLiteral("presentation"), "http://schemas.openxmlformats.org/presentationml/2006/main", &dom, &root, &why))
        return fail(QCoreApplication::translate("PptxImport", "The file has no presentation in it: %1").arg(why));
    m_doc.masters.clear();
    m_mainPart = mainPart;
    readSize(root);
    m_defaultStyle = pChild(root, "defaultTextStyle");
    QStringList fontNotes;
    m_fonts = readEmbeddedFonts(m_pkg, mainPart, root, &fontNotes);
    for (const QString &text : std::as_const(fontNotes)) note(text);

    for (const QDomElement &id : office::children(pChild(root, "sldMasterIdLst"), office::ns::P, QStringLiteral("sldMasterId"))) {
        const office::Relationship rel = m_pkg.relationship(mainPart, office::attrNs(id, office::ns::R, QStringLiteral("id")));
        if (rel.id.isEmpty() || rel.external) continue;
        // A master the presentation lists again is read once.
        if (m_masterByPart.contains(rel.target.toLower())) {
            note(QCoreApplication::translate("PptxImport", "The slide master %1 is listed more than once; it is read once.").arg(shortName(rel.target)));
            continue;
        }
        if (m_masters.size() >= kMaxMasters) {
            note(QCoreApplication::translate("PptxImport", "Only the first %1 slide masters are read.").arg(kMaxMasters));
            break;
        }
        m_masterByPart.insert(rel.target.toLower(), readMaster(rel.target));
    }
    if (m_masters.empty()) return fail(QCoreApplication::translate("PptxImport", "The presentation has no slide master that can be read.") + QLatin1Char('\n') + m_notes.join(QLatin1Char('\n')));

    QHash<qint64, int> pageOfSlideId;
    QHash<QString, int> pageOfRel;   // for the custom shows, which name slides by relationship
    int number = 0;
    for (const QDomElement &id : office::children(pChild(root, "sldIdLst"), office::ns::P, QStringLiteral("sldId"))) {
        if (++number > kMaxSlides) {
            note(QCoreApplication::translate("PptxImport", "Only the first %1 slides are read.").arg(kMaxSlides));
            break;
        }
        const office::Relationship rel = m_pkg.relationship(mainPart, office::attrNs(id, office::ns::R, QStringLiteral("id")));
        if (rel.id.isEmpty() || rel.external) {
            note(QCoreApplication::translate("PptxImport", "Slide %1 was left out: the file names no part for it.").arg(number));
            continue;
        }
        // A slide the presentation lists again is read once.
        if (const int first = m_slideByPart.value(rel.target.toLower(), 0)) {
            note(QCoreApplication::translate("PptxImport", "Slide %1 was left out: it is slide %2 listed again.").arg(number).arg(first));
            continue;
        }
        m_slideByPart.insert(rel.target.toLower(), number);
        if (readSlide(number, rel.target)) {
            pageOfSlideId.insert(office::parseInt(office::attr(id, QStringLiteral("id")), -1), int(m_doc.pages.size()) - 1);
            pageOfRel.insert(rel.id, int(m_doc.pages.size()) - 1);
        }
    }
    if (!ensureASlide(m_doc)) return fail(QCoreApplication::translate("PptxImport", "The presentation has no slides."));
    readSections(root, pageOfSlideId);
    readShowSettings(root, pageOfRel);
    {
        QStringList notes;
        readTableStyleDefs(tableStyles(), [](const QDomElement &e) { return standaloneXml(e); }, m_doc, &notes);
        for (const QString &text : std::as_const(notes)) note(text);
    }
    m_doc.appData[QStringLiteral("footers")] = m_footers;
    sayWhatRanOut();
    for (const QString &text : leftOutNotes(m_doc.imagesLeftOut, m_pkg.skippedCount())) note(text);
    if (error) {
        QStringList lines = m_notes;
        if (m_extraNotes) lines << QCoreApplication::translate("PptxImport", "And %1 more.").arg(m_extraNotes);
        *error = lines.join(QLatin1Char('\n'));
    }
    return true;
}

std::unique_ptr<Document> importPackage(const office::Package &pkg, QString *error)
{
    auto doc = std::make_unique<Document>();
    doc->slideText = true;
    doc->slidePictures = true;
    Importer importer(pkg, *doc);
    if (!importer.run(pkg.mainPart(), error)) return nullptr;
    // The presentation's own fonts are available for as long as it is open (see core/embeddedfonts.h).
    EmbeddedFonts::use(doc.get(), importer.takeFonts());
    doc->fromFile = true;   // what the file names outside itself is read only from beside it
    return doc;
}

// An older presentation is a compound file, as a package protected with a password is.
bool looksLikeCompoundFile(const QByteArray &bytes) { return bytes.startsWith(QByteArray::fromHex("D0CF11E0A1B11AE1")); }

QString olderFormat()
{
    return QCoreApplication::translate("PptxImport", "This file is in an older format, or is protected with a password, which %1 can't open yet.").arg(appInfo().name);
}

} // namespace

std::unique_ptr<Document> importPresentation(const QByteArray &bytes, QString *error)
{
    if (looksLikeCompoundFile(bytes)) {
        if (error) *error = olderFormat();
        return nullptr;
    }
    office::Package pkg;
    if (!pkg.read(bytes, error)) return nullptr;
    return importPackage(pkg, error);
}

std::unique_ptr<Document> importPresentationFile(const QString &path, QString *error)
{
    office::Package pkg;
    if (!pkg.readFile(path, error)) {
        // The compound file's magic is the first thing in the file.
        QFile f(path);
        if (f.open(QIODevice::ReadOnly) && looksLikeCompoundFile(f.read(8)) && error) *error = olderFormat();
        return nullptr;
    }
    return importPackage(pkg, error);
}

} // namespace jp::deck
