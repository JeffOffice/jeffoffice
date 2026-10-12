#include "core/comments.h"

#include "app/settings.h"

#include <QJsonArray>
#include <QJsonObject>

namespace jp::deck {

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
    return QJsonObject{{"id", c.id},     {"author", c.author}, {"initials", c.initials}, {"date", dateText(c.date)}, {"text", c.text},
                       {"x", c.at.x()},  {"y", c.at.y()},      {"replies", replies},     {"done", c.done}};
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

QString addComment(Page &slide, const CommentAuthor &who, const QString &text, const QPointF &at, const QDateTime &when)
{
    Comment c;
    c.id = newId("c");
    c.author = who.name;
    c.initials = who.initials.isEmpty() ? initialsFor(who.name) : who.initials;
    c.date = when.toUTC();
    c.text = commentText(text);
    c.at = at;
    QVector<Comment> list = commentsOf(slide);
    list.append(c);
    setComments(slide, list);
    return c.id;
}

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

} // namespace jp::deck
