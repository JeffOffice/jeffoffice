#include "io/pdftransparency.h"

#include "io/qtpdf.h"

#include <QCoreApplication>
#include <QStringList>
#include <functional>
#include <QHash>
#include <QRegularExpression>
#include <QSet>

namespace jp {

namespace {

constexpr int kMostPages = 100000;
constexpr int kMostObjects = 2000000;
constexpr int kMostDepth = 8;       // a form inside a form inside a pattern...
constexpr int kMostVisits = 200000; // streams read for all the pages together

bool seeThroughState(const QString &dict)
{
    static const QRegularExpression re(QStringLiteral("/(?:ca|CA)\\s+(-?[0-9.]+)"));
    auto it = re.globalMatch(dict);
    while (it.hasNext())
        if (it.next().captured(1).toDouble() < 0.9995) return true;
    return false;
}

bool hasSoftMask(const QString &dict)
{
    static const QRegularExpression re(QStringLiteral("/SMask\\s*(?!/None)(\\d+ 0 R|<<)"));
    return re.match(dict).hasMatch();
}

// A blend mode other than Normal (or Compatible, its old name), as a name or as the list of names a reader tries in turn.
bool hasBlendMode(const QString &dict)
{
    static const QRegularExpression re(QStringLiteral("/BM\\s*(/[A-Za-z]+|\\[[^\\]]*\\])"));
    static const QRegularExpression name(QStringLiteral("/([A-Za-z]+)"));
    auto it = re.globalMatch(dict);
    while (it.hasNext()) {
        auto names = name.globalMatch(it.next().captured(1));
        while (names.hasNext()) {
            const QString n = names.next().captured(1);
            if (n != QLatin1String("Normal") && n != QLatin1String("Compatible")) return true;
        }
    }
    return false;
}

bool isTransparencyGroup(const QString &dict)
{
    static const QRegularExpression re(QStringLiteral("/Group\\s*<<[^>]*?/S\\s*/Transparency"));
    return re.match(dict).hasMatch();
}

bool seeThroughDict(const QString &dict) { return seeThroughState(dict) || hasSoftMask(dict) || hasBlendMode(dict) || isTransparencyGroup(dict); }

struct Scan {
    const QtPdf &pdf;
    QHash<int, const QtPdf::Obj *> byId;
    int visits = 0;
    bool unreadable = false;

    explicit Scan(const QtPdf &p) : pdf(p) {}

    const QtPdf::Obj *get(int id) const { return id > 0 ? byId.value(id, nullptr) : nullptr; }

    // A dictionary as text, with the dictionary a "/Group N 0 R" points to put where it is (a group is kept as an object of its own as well).
    QString dictText(const QtPdf::Obj &o) const
    {
        QString text = QString::fromLatin1(QtPdf::dictOf(o.body));
        const int group = QtPdf::ref(text.toLatin1(), "/Group");
        if (const QtPdf::Obj *g = get(group)) text += QStringLiteral(" /Group ") + QString::fromLatin1(QtPdf::dictOf(g->body));
        return text;
    }

    // The names a resource dictionary gives to objects ("/Name N 0 R"), looking into the dictionaries it points to ("/ExtGState N 0 R"), a few levels down.
    void namesIn(const QString &text, QHash<QString, int> &names, int depth, bool &bad) const
    {
        static const QRegularExpression re(QStringLiteral("/([A-Za-z0-9_.#-]+)\\s+(\\d+) 0 R"));
        auto it = re.globalMatch(text);
        while (it.hasNext()) {
            const auto m = it.next();
            const int id = m.captured(2).toInt();
            names.insert(m.captured(1), id);
            const QString key = m.captured(1);
            if (depth > 0 && (key == QLatin1String("ExtGState") || key == QLatin1String("XObject") || key == QLatin1String("Pattern") || key == QLatin1String("Resources"))) {
                if (const QtPdf::Obj *o = get(id)) {
                    const QString inner = QString::fromLatin1(QtPdf::dictOf(o->body));
                    bad |= seeThroughDict(inner);
                    namesIn(inner, names, depth - 1, bad);
                }
            }
        }
    }

    QVector<int> contentsOf(const QString &dict) const
    {
        QVector<int> ids;
        const qsizetype at = dict.indexOf(QLatin1String("/Contents"));
        if (at < 0) return ids;
        qsizetype i = at + 9;
        while (i < dict.size() && dict[i].isSpace()) ++i;
        static const QRegularExpression one(QStringLiteral("(\\d+) 0 R"));
        if (i < dict.size() && dict[i] == QLatin1Char('[')) {
            const qsizetype close = dict.indexOf(QLatin1Char(']'), i);
            auto it = one.globalMatch(dict.mid(i, close < 0 ? -1 : close - i));
            while (it.hasNext()) ids << it.next().captured(1).toInt();
        } else {
            const auto m = one.match(dict, i);
            if (m.hasMatch() && m.capturedStart() == i) {
                const int id = m.captured(1).toInt();
                // (an object that is a list of streams)
                const QtPdf::Obj *o = get(id);
                if (o && !QtPdf::isStream(o->body) && o->body.trimmed().startsWith('[')) {
                    auto it = one.globalMatch(QString::fromLatin1(o->body));
                    while (it.hasNext()) ids << it.next().captured(1).toInt();
                } else {
                    ids << id;
                }
            }
        }
        return ids;
    }

    // The resources a page, form, or pattern names (its own, else its parents'), as text.
    QString resourcesOf(const QtPdf::Obj &o, const QString &dict) const
    {
        QString cur = dict;
        for (int up = 0; up < kMostDepth; ++up) {
            if (cur.contains(QLatin1String("/Resources"))) {
                const int res = QtPdf::ref(cur.toLatin1(), "/Resources");
                if (const QtPdf::Obj *r = get(res)) return QString::fromLatin1(QtPdf::dictOf(r->body));
                return cur;
            }
            const QtPdf::Obj *parent = get(QtPdf::ref(cur.toLatin1(), "/Parent"));
            if (!parent) break;
            cur = QString::fromLatin1(QtPdf::dictOf(parent->body));
        }
        Q_UNUSED(o)
        return dict;
    }

    // Whether what a drawing (its resources `res` and its content `stream`) uses is see-through, in a form or a pattern it draws with too.
    bool uses(const QString &res, const QByteArray &stream, int depth, QSet<int> &seen)
    {
        bool bad = false;
        QHash<QString, int> names;
        namesIn(res, names, 2, bad);
        if (bad || seeThroughDict(res)) return true;
        static const QRegularExpression use(QStringLiteral("/([A-Za-z0-9_.#-]+)\\s+(?:gs|Do|scn|SCN|sh)\\b"));
        auto it = use.globalMatch(QString::fromLatin1(stream));
        while (it.hasNext()) {
            const int id = names.value(it.next().captured(1), 0);
            const QtPdf::Obj *o = get(id);
            if (!o) continue;
            const QString dict = dictText(*o);
            if (seeThroughDict(dict)) return true;
            // (a pattern or a shading keeps its graphics states in its own dictionary)
            if (dict.contains(QLatin1String("/ExtGState"))) {
                QHash<QString, int> inner;
                bool innerBad = false;
                namesIn(dict, inner, 2, innerBad);
                if (innerBad) return true;
                for (auto at = inner.cbegin(); at != inner.cend(); ++at)
                    if (const QtPdf::Obj *state = get(at.value()); state && seeThroughDict(dictText(*state))) return true;
            }
            if (!QtPdf::isStream(o->body) || seen.contains(id)) continue;
            // A form, or a tiling pattern: drawn with its own resources.
            const bool drawing = dict.contains(QLatin1String("/Form")) || dict.contains(QRegularExpression(QStringLiteral("/PatternType\\s*1")));
            if (!drawing) continue;
            if (depth >= kMostDepth || ++visits > kMostVisits) {
                unreadable = true;
                return false;
            }
            seen.insert(id);
            bool ok = false;
            const QByteArray inner = pdf.streamData(*o, &ok);
            if (!ok) {
                unreadable = true;
                return false;
            }
            if (uses(resourcesOf(*o, dict), inner, depth + 1, seen)) return true;
            if (unreadable) return false;
        }
        return false;
    }
};

std::function<QByteArray(const QByteArray &)> g_probe;

} // namespace

void setPdfTransparencyProbeForTest(std::function<QByteArray(const QByteArray &)> probe) { g_probe = std::move(probe); }

PdfTransparency pdfTransparency(const QByteArray &given)
{
    const QByteArray bytes = g_probe ? g_probe(given) : given;
    PdfTransparency out;
    QtPdf pdf;
    if (!pdf.loadBytes(bytes) || pdf.objects.size() > kMostObjects) {
        out.unreadable = true;
        return out;
    }
    Scan scan(pdf);
    QVector<const QtPdf::Obj *> pages;
    static const QRegularExpression isPage(QStringLiteral("/Type\\s*/Page(?![a-zA-Z])"));
    for (const QtPdf::Obj &o : pdf.objects) {
        scan.byId.insert(o.id, &o);
        const QString dict = QString::fromLatin1(QtPdf::dictOf(o.body));
        if (seeThroughDict(dict)) out.found = true;
        if (isPage.match(dict).hasMatch() && pages.size() < kMostPages) pages << &o;
    }
    if (pages.isEmpty()) {
        out.unreadable = true;
        return out;
    }
    // The pages in the order of the page tree (the objects are in the order they were written, which is not the pages').
    QVector<const QtPdf::Obj *> ordered;
    {
        QSet<int> seen;
        std::function<void(int, int)> walk = [&](int id, int depth) {
            const QtPdf::Obj *o = scan.get(id);
            if (!o || depth > kMostDepth || seen.contains(id) || ordered.size() >= kMostPages) return;
            seen.insert(id);
            const QString dict = QString::fromLatin1(QtPdf::dictOf(o->body));
            if (isPage.match(dict).hasMatch()) {
                ordered << o;
                return;
            }
            const qsizetype kids = dict.indexOf(QLatin1String("/Kids"));
            if (kids < 0) return;
            const qsizetype close = dict.indexOf(QLatin1Char(']'), kids);
            static const QRegularExpression one(QStringLiteral("(\\d+) 0 R"));
            auto it = one.globalMatch(dict.mid(kids, close < 0 ? -1 : close - kids));
            while (it.hasNext()) walk(it.next().captured(1).toInt(), depth + 1);
        };
        const int root = QtPdf::ref(pdf.trailer, "/Root");
        const QtPdf::Obj *catalog = scan.get(root);
        if (catalog) walk(QtPdf::ref(QtPdf::dictOf(catalog->body), "/Pages"), 0);
        if (ordered.size() == pages.size()) pages = ordered;
    }
    int page = 0;
    for (const QtPdf::Obj *o : std::as_const(pages)) {
        ++page;
        const QString dict = scan.dictText(*o);
        bool bad = seeThroughDict(dict);
        if (!bad) {
            QByteArray stream;
            bool ok = true;
            for (int id : scan.contentsOf(dict)) {
                const QtPdf::Obj *c = scan.get(id);
                if (!c) continue;
                if (++scan.visits > kMostVisits) {
                    out.unreadable = true;
                    break;
                }
                bool read = false;
                stream += pdf.streamData(*c, &read) + '\n';
                ok &= read;
            }
            if (!ok) out.unreadable = true;
            QSet<int> seen;
            if (!out.unreadable) bad = scan.uses(scan.resourcesOf(*o, dict), stream, 0, seen);
        }
        if (scan.unreadable) out.unreadable = true;
        if (bad) {
            out.found = true;
            out.pages << page;
        }
        if (out.unreadable) break;
    }
    return out;
}

QString pdfTransparencyRefusal(const PdfTransparency &t, const QString &standard, const std::function<QString(int pdfPage)> &name)
{
    if (t.unreadable)
        return QCoreApplication::translate("PdfTransparency", "The PDF could not be checked for see-through parts, so it can't be a %1 file. Turn %1 off to export it.").arg(standard);
    QStringList named;
    for (int page : t.pages) {
        if (named.size() >= 8) break;
        const QString label = name ? name(page) : QString();
        named << (label.isEmpty() ? QCoreApplication::translate("PdfTransparency", "PDF page %1").arg(page) : label);
    }
    if (named.isEmpty())
        return QCoreApplication::translate("PdfTransparency", "A %1 file can't have see-through parts, and some could not be made solid. Turn %1 off to export it.").arg(standard);
    if (t.pages.size() > named.size()) named << QCoreApplication::translate("PdfTransparency", "more");
    return QCoreApplication::translate("PdfTransparency", "A %1 file can't have see-through parts, and some could not be made solid (%2). Turn %1 off to export it.")
        .arg(standard, named.join(QStringLiteral(", ")));
}

} // namespace jp
