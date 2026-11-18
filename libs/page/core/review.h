#pragma once
// The one model of review that every app shares: a list of changes (what someone else did to a document, as compared with this one) and of
// comments, each with a state that the person reviewing sets. JeffDeck's Compare fills it first (apps/deck/core/deckcompare.h); the Reviewing
// Pane lists it whatever app it came from. Header only: it holds no document, so it sits under every app.
//
// How each app's own records map onto a Change (the model needs no change for them):
// - JeffDeck: container is the slide's page id, object the object's id (empty for the slide), path the property ("rect", "text", "fill", "line",
//   "font", "notes", "order", "comment").
// - JeffWrite: a Revision is a Change: Insertion is Added, Deletion is Deleted, a move is Moved, a changed property is Reformatted; a comment is
//   Commented. container names the story ("body", "footnote:3", "header:1"), object is the revision's or comment's id, path is the character
//   range "from-to" in that story, position is the offset in the body so that the list is in document order, data holds what is app-owned
//   (the run properties, the paragraph, the id of the revision a move pairs with).
// - JeffCrunch: a thread (NoteInfo::thread) is a Commented change: container is the sheet's name, object the cell's address, path "thread", and
//   data keeps the number of comments and the resolved flag (a resolved thread is Accepted). Position is sheet index times a million plus row
//   times a thousand plus column, so that the list runs down the workbook.
// The pane shows kind, author, when, and summary; it does not need to know more, and filters by container, author, and state.

#include <QDateTime>
#include <QJsonObject>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

namespace jp::review {

enum class Kind { Added, Deleted, Moved, Edited, Reformatted, Commented };
// Pending until someone decides; Accepted keeps the other side's version, Rejected keeps this document's.
enum class State { Pending, Accepted, Rejected };

// Stable words for a file or a script (never shown): the pane's own words come from translate().
inline QString kindKey(Kind k)
{
    switch (k) {
    case Kind::Added: return QStringLiteral("added");
    case Kind::Deleted: return QStringLiteral("deleted");
    case Kind::Moved: return QStringLiteral("moved");
    case Kind::Edited: return QStringLiteral("edited");
    case Kind::Reformatted: return QStringLiteral("reformatted");
    case Kind::Commented: return QStringLiteral("commented");
    }
    return QStringLiteral("edited");
}
inline Kind kindFromKey(const QString &s, bool *ok = nullptr)
{
    static const Kind all[] = {Kind::Added, Kind::Deleted, Kind::Moved, Kind::Edited, Kind::Reformatted, Kind::Commented};
    for (Kind k : all)
        if (kindKey(k) == s) { if (ok) *ok = true; return k; }
    if (ok) *ok = false;
    return Kind::Edited;
}
inline QString stateKey(State s) { return s == State::Accepted ? QStringLiteral("accepted") : s == State::Rejected ? QStringLiteral("rejected") : QStringLiteral("pending"); }
inline State stateFromKey(const QString &s)
{
    return s == QLatin1String("accepted") ? State::Accepted : s == QLatin1String("rejected") ? State::Rejected : State::Pending;
}

// Where a change is: the unit that holds it (a slide, a story, a sheet), the thing in it (an object, a revision, a cell), and what of that thing
// changed. Any may be empty.
struct Target {
    QString container, object, path;
    bool operator==(const Target &o) const { return container == o.container && object == o.object && path == o.path; }
};

struct Change {
    QString id;                  // told apart within a ChangeList
    Kind kind = Kind::Edited;
    QString author;
    QDateTime when;              // UTC; invalid when unknown
    Target target;
    QString summary;             // one line for the pane, already translated
    State state = State::Pending;
    qint64 position = 0;         // document order: the list is sorted by it, and next/previous follow it
    QJsonObject data;            // what the app that made the change needs to apply or undo it; the pane never reads it

    QJsonObject toJson() const
    {
        QJsonObject o;
        o[QStringLiteral("id")] = id;
        o[QStringLiteral("kind")] = kindKey(kind);
        o[QStringLiteral("author")] = author;
        if (when.isValid()) o[QStringLiteral("when")] = when.toUTC().toString(Qt::ISODate);
        o[QStringLiteral("container")] = target.container;
        o[QStringLiteral("object")] = target.object;
        o[QStringLiteral("path")] = target.path;
        o[QStringLiteral("summary")] = summary;
        o[QStringLiteral("state")] = stateKey(state);
        o[QStringLiteral("position")] = double(position);
        if (!data.isEmpty()) o[QStringLiteral("data")] = data;
        return o;
    }
    static Change fromJson(const QJsonObject &o)
    {
        Change c;
        c.id = o.value(QStringLiteral("id")).toString();
        c.kind = kindFromKey(o.value(QStringLiteral("kind")).toString());
        c.author = o.value(QStringLiteral("author")).toString();
        c.when = QDateTime::fromString(o.value(QStringLiteral("when")).toString(), Qt::ISODate);
        c.target = {o.value(QStringLiteral("container")).toString(), o.value(QStringLiteral("object")).toString(), o.value(QStringLiteral("path")).toString()};
        c.summary = o.value(QStringLiteral("summary")).toString();
        c.state = stateFromKey(o.value(QStringLiteral("state")).toString());
        c.position = qint64(o.value(QStringLiteral("position")).toDouble());
        c.data = o.value(QStringLiteral("data")).toObject();
        return c;
    }
};

// The most changes a list holds (a file can make more than anyone reads); the rest are left out and `tooMany` says so.
constexpr int kMaxChanges = 10000;

struct ChangeList {
    QVector<Change> changes;
    bool tooMany = false;        // the comparison stopped at a limit: the list is not all there is

    int size() const { return int(changes.size()); }
    bool isEmpty() const { return changes.isEmpty(); }
    int count(State s) const
    {
        int n = 0;
        for (const Change &c : changes) n += c.state == s;
        return n;
    }
    int pending() const { return count(State::Pending); }
    int indexOf(const QString &id) const
    {
        for (int i = 0; i < changes.size(); ++i)
            if (changes[i].id == id) return i;
        return -1;
    }
    int count(Kind k) const
    {
        int n = 0;
        for (const Change &c : changes) n += c.kind == k;
        return n;
    }
    // The indices of the changes in `container` and by `author` (an empty one matches all), in list order.
    QVector<int> filter(const QString &container = QString(), const QString &author = QString(), bool onlyPending = false) const
    {
        QVector<int> out;
        for (int i = 0; i < changes.size(); ++i) {
            const Change &c = changes[i];
            if ((container.isEmpty() || c.target.container == container) && (author.isEmpty() || c.author == author) && (!onlyPending || c.state == State::Pending)) out << i;
        }
        return out;
    }
    QStringList authors() const
    {
        QStringList out;
        QSet<QString> seen;
        for (const Change &c : changes)
            if (!c.author.isEmpty() && !seen.contains(c.author)) { seen.insert(c.author); out << c.author; }
        return out;
    }
    // The next (or previous) pending change after list index `from` (-1: from the start), wrapping round; -1 when none is pending.
    int step(int from, bool forward = true) const
    {
        const int n = size();
        const int base = from < 0 && !forward ? 0 : from;
        for (int k = 1; k <= n; ++k) {
            const int i = forward ? ((base + k) % n + n) % n : ((base - k) % n + n) % n;
            if (changes[i].state == State::Pending) return i;
        }
        return -1;
    }
};

} // namespace jp::review
