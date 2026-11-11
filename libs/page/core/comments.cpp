#include "core/comments.h"

#include "app/settings.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <algorithm>
#include <atomic>
#include <cmath>

namespace jp::comments {

namespace {

const QString kKey = QStringLiteral("comments");

QString dateText(const QDateTime &t) { return t.isValid() ? t.toUTC().toString(Qt::ISODateWithMs) : QString(); }

QDateTime dateOf(const QJsonValue &v) { return QDateTime::fromString(v.toString(), Qt::ISODateWithMs); }

QJsonObject toJson(const CommentReply &r)
{
    return QJsonObject{{"id", r.id}, {"author", r.author}, {"initials", r.initials}, {"date", dateText(r.date)}, {"text", r.text}};
}

CommentReply replyFromJson(const QJsonObject &o)
{
    CommentReply r;
    r.id = o.value(QStringLiteral("id")).toString();
    r.author = o.value(QStringLiteral("author")).toString();
    r.initials = o.value(QStringLiteral("initials")).toString();
    r.date = dateOf(o.value(QStringLiteral("date")));
    r.text = o.value(QStringLiteral("text")).toString();
    return r;
}

QJsonObject toJson(const Comment &c)
{
    QJsonArray replies;
    for (const CommentReply &r : c.replies) replies.append(toJson(r));
    QJsonObject o{{"id", c.id},     {"author", c.author}, {"initials", c.initials}, {"date", dateText(c.date)}, {"text", c.text},
                  {"x", c.at.x()},  {"y", c.at.y()},      {"replies", replies},     {"done", c.done}};
    if (!c.itemId.isEmpty()) o[QStringLiteral("item")] = c.itemId;
    return o;
}

Comment fromJson(const QJsonObject &o)
{
    Comment c;
    c.id = o.value(QStringLiteral("id")).toString();
    c.author = o.value(QStringLiteral("author")).toString();
    c.initials = o.value(QStringLiteral("initials")).toString();
    c.date = dateOf(o.value(QStringLiteral("date")));
    c.text = o.value(QStringLiteral("text")).toString();
    c.at = QPointF(o.value(QStringLiteral("x")).toDouble(10), o.value(QStringLiteral("y")).toDouble(10));
    for (const QJsonValue &v : o.value(QStringLiteral("replies")).toArray()) c.replies.append(replyFromJson(v.toObject()));
    c.done = o.value(QStringLiteral("done")).toBool();
    c.itemId = o.value(QStringLiteral("item")).toString();
    return c;
}

// Runs `fn` on the list and stores the result.
template <class Fn>
bool edit(Page &slide, Fn fn)
{
    QVector<Comment> list = commentsOf(slide);
    if (!fn(list)) return false;
    setComments(slide, list);
    return true;
}

} // namespace

CommentAuthor commentAuthor()
{
    CommentAuthor who;
    who.name = Settings::get().value(QStringLiteral("user/name")).toString().trimmed();
    who.initials = Settings::get().value(QStringLiteral("user/initials")).toString().trimmed();
    if (who.initials.isEmpty()) who.initials = initialsFor(who.name);
    return who;
}

void setCommentAuthor(const CommentAuthor &who)
{
    const QString name = who.name.trimmed();
    Settings::get().setValue(QStringLiteral("user/name"), name);
    Settings::get().setValue(QStringLiteral("user/initials"), who.initials.trimmed().isEmpty() ? initialsFor(name) : who.initials.trimmed());
}

QString initialsFor(const QString &name)
{
    QString out;
    for (const QString &word : name.split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
        if (out.size() >= 3) break;
        if (word.front().isLetterOrNumber()) out += word.front().toUpper();
    }
    return out;
}

QString commentText(const QString &text)
{
    QString t = text;
    t.replace(QLatin1String("\r\n"), QLatin1String("\n")).replace(QLatin1Char('\r'), QLatin1Char('\n'));
    t.replace(QChar(QChar::LineSeparator), QLatin1Char('\n')).replace(QChar(QChar::ParagraphSeparator), QLatin1Char('\n'));
    return t.left(kMaxCommentText);
}

QVector<Comment> commentsOf(const Page &slide)
{
    QVector<Comment> out;
    for (const QJsonValue &v : slide.props.value(kKey).toArray()) out.append(fromJson(v.toObject()));
    return out;
}

void setComments(Page &slide, const QVector<Comment> &list)
{
    if (list.isEmpty()) {
        slide.props.remove(kKey);
        return;
    }
    QJsonArray a;
    for (const Comment &c : list) a.append(toJson(c));
    slide.props[kKey] = a;
}

int commentCount(const Page &slide) { return slide.props.value(kKey).toArray().size(); }

std::optional<Comment> findComment(const Page &slide, const QString &id)
{
    for (const QJsonValue &v : slide.props.value(kKey).toArray())
        if (v.toObject().value(QStringLiteral("id")).toString() == id) return fromJson(v.toObject());
    return std::nullopt;
}

QString addComment(Page &slide, const CommentAuthor &who, const QString &text, const QPointF &at, const QDateTime &when, const QString &itemId)
{
    Comment c;
    c.id = newId("c");
    c.author = who.name;
    c.initials = who.initials.isEmpty() ? initialsFor(who.name) : who.initials;
    c.date = when.toUTC();
    c.text = commentText(text);
    c.at = at;
    c.itemId = itemId;
    QVector<Comment> list = commentsOf(slide);
    list.append(c);
    setComments(slide, list);
    return c.id;
}

namespace {
std::atomic<int> g_builds{0}, g_scans{0};

// The objects of a page (those in groups too), by id.
QHash<QString, const Item *> itemsOf(const Page &page)
{
    ++g_scans;
    QHash<QString, const Item *> out;
    walkItems(page.items, [&](const ItemPtr &it) { out.insert(it->id, it.get()); });
    return out;
}

QPointF cornerOf(const Item &it)
{
    const QRectF r = it.bounds();
    return QPointF(r.right(), r.top());
}
} // namespace

QPointF commentPlace(const Page &page, const Comment &c)
{
    if (c.itemId.isEmpty()) return c.at;
    const auto items = itemsOf(page);
    const Item *it = items.value(c.itemId);
    return it ? cornerOf(*it) : c.at;
}

QVector<CommentMark> commentMarks(const Page &page)
{
    ++g_builds;
    const QVector<Comment> list = commentsOf(page);
    QVector<CommentMark> out;
    out.reserve(list.size());
    QHash<QString, const Item *> items;
    bool scanned = false;
    for (const Comment &c : list) {
        CommentMark m{c.id, c.author, c.initials, c.at, c.done, false};
        if (!c.itemId.isEmpty()) {
            if (!scanned) {
                items = itemsOf(page);
                scanned = true;
            }
            if (const Item *it = items.value(c.itemId)) {
                m.place = cornerOf(*it);
                m.attached = true;
            }
        }
        out.append(m);
    }
    return out;
}

int commentMarkBuilds() { return g_builds; }
int commentItemScans() { return g_scans; }

QString addReply(Page &slide, const QString &commentId, const CommentAuthor &who, const QString &text, const QDateTime &when)
{
    CommentReply r;
    r.id = newId("r");
    r.author = who.name;
    r.initials = who.initials.isEmpty() ? initialsFor(who.name) : who.initials;
    r.date = when.toUTC();
    r.text = commentText(text);
    const bool added = edit(slide, [&](QVector<Comment> &list) {
        for (Comment &c : list)
            if (c.id == commentId) {
                c.replies.append(r);
                return true;
            }
        return false;
    });
    return added ? r.id : QString();
}

bool editComment(Page &slide, const QString &id, const QString &text)
{
    const QString t = commentText(text);
    return edit(slide, [&](QVector<Comment> &list) {
        for (Comment &c : list) {
            if (c.id == id) {
                c.text = t;
                return true;
            }
            for (CommentReply &r : c.replies)
                if (r.id == id) {
                    r.text = t;
                    return true;
                }
        }
        return false;
    });
}

bool resolveComment(Page &slide, const QString &id, bool done)
{
    return edit(slide, [&](QVector<Comment> &list) {
        for (Comment &c : list)
            if (c.id == id) {
                c.done = done;
                return true;
            }
        return false;
    });
}

bool moveComment(Page &slide, const QString &id, const QPointF &to)
{
    return edit(slide, [&](QVector<Comment> &list) {
        for (Comment &c : list)
            if (c.id == id) {
                c.at = to;
                return true;
            }
        return false;
    });
}

bool deleteComment(Page &slide, const QString &id)
{
    return edit(slide, [&](QVector<Comment> &list) {
        for (int i = 0; i < list.size(); ++i) {
            if (list[i].id == id) {
                list.removeAt(i);
                return true;
            }
            for (int j = 0; j < list[i].replies.size(); ++j)
                if (list[i].replies[j].id == id) {
                    list[i].replies.removeAt(j);
                    return true;
                }
        }
        return false;
    });
}

int deleteComments(Page &slide)
{
    const int n = commentCount(slide);
    slide.props.remove(kKey);
    return n;
}

int deleteAllComments(Document &d)
{
    int n = 0;
    for (const auto &slide : d.pages) n += deleteComments(*slide);
    return n;
}

QVector<CommentRef> allComments(const Document &d)
{
    QVector<CommentRef> out;
    for (int i = 0; i < d.pages.size(); ++i)
        for (const QJsonValue &v : d.pages[i]->props.value(kKey).toArray()) out.append(CommentRef{i, v.toObject().value(QStringLiteral("id")).toString()});
    return out;
}

int limitComments(Document &d)
{
    const QSizeF size = d.pageSize();
    int cut = 0, comments = 0, replies = 0;
    qint64 text = 0;
    // A text, held to what one may hold and to what is left for the document.
    auto fit = [&](const QJsonObject &o, QString *out) {
        const qint64 room = kMaxTotalText - text;
        if (room <= 0) return false;
        *out = o.value(QStringLiteral("text")).toString().left(int(std::min<qint64>(kMaxCommentText, room)));
        text += out->size();
        return true;
    };
    for (const auto &page : d.pages) {
        const QJsonValue v = page->props.value(kKey);
        if (v.isUndefined()) continue;
        QSet<QString> used;
        QSet<QString> onPage;
        bool scanned = false;
        auto freshId = [&](const QString &id, const char *prefix) {
            const QString made = id.isEmpty() || used.contains(id) ? newId(prefix) : id;
            used.insert(made);
            return made;
        };
        // (a name and its initials are a line, not a page)
        auto tidy = [&](QJsonObject &o, const char *prefix) {
            o[QStringLiteral("id")] = freshId(o.value(QStringLiteral("id")).toString(), prefix);
            o[QStringLiteral("author")] = o.value(QStringLiteral("author")).toString().left(200);
            o[QStringLiteral("initials")] = o.value(QStringLiteral("initials")).toString().left(8);
        };
        QJsonArray kept;
        for (const QJsonValue &entry : v.toArray()) {
            QString body;
            if (!entry.isObject() || comments >= kMaxComments || !fit(entry.toObject(), &body)) {
                ++cut;
                continue;
            }
            ++comments;
            QJsonObject o = entry.toObject();
            tidy(o, "c");
            o[QStringLiteral("text")] = body;
            auto place = [&](const char *key, double limit) {
                double x = o.value(QLatin1String(key)).toDouble(10);
                if (!std::isfinite(x)) x = 10;
                o[QLatin1String(key)] = std::clamp(x, -kPlaceMargin, limit + kPlaceMargin);
            };
            place("x", size.width());
            place("y", size.height());
            const QString item = o.value(QStringLiteral("item")).toString();
            if (!item.isEmpty()) {
                if (!scanned) {
                    walkItems(page->items, [&](const ItemPtr &it) { onPage.insert(it->id); });
                    scanned = true;
                }
                if (!onPage.contains(item)) o.remove(QStringLiteral("item"));
            }
            QJsonArray kept2;
            for (const QJsonValue &r : o.value(QStringLiteral("replies")).toArray()) {
                QString replyText;
                if (!r.isObject() || kept2.size() >= kMaxReplies || replies >= kMaxTotalReplies || !fit(r.toObject(), &replyText)) {
                    ++cut;
                    continue;
                }
                ++replies;
                QJsonObject ro = r.toObject();
                tidy(ro, "r");
                ro[QStringLiteral("text")] = replyText;
                kept2.append(ro);
            }
            o[QStringLiteral("replies")] = kept2;
            kept.append(o);
        }
        if (kept.isEmpty()) page->props.remove(kKey);
        else page->props[kKey] = kept;
    }
    return cut;
}

NamesOutScope::NamesOutScope(Document &d, bool on) : m_doc(d), m_on(on)
{
    if (!on) return;
    for (const auto &page : d.pages) {
        m_kept.append(page->props.value(kKey).toArray());
        QVector<Comment> list = commentsOf(*page);
        if (list.isEmpty()) continue;
        QJsonArray blank;
        for (const QJsonValue &v : page->props.value(kKey).toArray()) {
            QJsonObject o = v.toObject();
            o[QStringLiteral("author")] = QString();
            o[QStringLiteral("initials")] = QString();
            QJsonArray rs;
            for (const QJsonValue &r : o.value(QStringLiteral("replies")).toArray()) {
                QJsonObject ro = r.toObject();
                ro[QStringLiteral("author")] = QString();
                ro[QStringLiteral("initials")] = QString();
                rs.append(ro);
            }
            o[QStringLiteral("replies")] = rs;
            blank.append(o);
        }
        page->props[kKey] = blank;
    }
}

NamesOutScope::~NamesOutScope()
{
    if (!m_on) return;
    for (int i = 0; i < m_doc.pages.size() && i < m_kept.size(); ++i) {
        if (m_kept[i].isEmpty()) m_doc.pages[i]->props.remove(kKey);
        else m_doc.pages[i]->props[kKey] = m_kept[i];
    }
}

void carryComments(const Document &from, Document &to)
{
    if (to.pages.isEmpty()) return;
    QVector<QVector<Comment>> onPage(to.pages.size());
    for (int i = 0; i < from.pages.size(); ++i) {
        QVector<Comment> list = commentsOf(*from.pages[i]);
        for (Comment &c : list) c.itemId.clear();
        onPage[std::min<int>(i, to.pages.size() - 1)] += list;
    }
    for (int i = 0; i < to.pages.size(); ++i) setComments(*to.pages[i], onPage[i]);
}

void copyComments(const Page &from, Page &to, const QHash<QString, QString> &copies)
{
    QVector<Comment> list = commentsOf(from);
    for (Comment &c : list) {
        c.id = newId("c");
        if (!c.itemId.isEmpty()) c.itemId = copies.value(c.itemId);
        for (CommentReply &r : c.replies) r.id = newId("r");
    }
    setComments(to, list);
}

} // namespace jp::comments
