#pragma once
// The one model of review that every app shares: a list of changes (what someone else did to a document, as compared with this one) and of
// comments, each with a state that the person reviewing sets. JeffDeck's Compare fills it first (apps/deck/core/deckcompare.h); the Reviewing
// Pane lists it whatever app it came from. Header only and QtCore only: it holds no document, so it sits in libs/base beside the text
// properties that hold a tracked change's marks (tp::Revision), under every reader, JeffWrite's and JeffCrunch's included.
//
// How each app's own records map onto a Change (the model needs no change for them):
// - JeffDeck: container is the slide's page id, object the object's id (empty for the slide), path the property ("rect", "text", "fill", "line",
//   "font", "notes", "order", "comment").
// - JeffWrite: a Revision is a Change: Insertion is Added, Deletion is Deleted, a move is Moved, a changed property is Reformatted; a comment is
//   Commented. container names the story ("body", "footnote:3", "header:1"), object is the revision's or comment's id, position is the
//   offset in the story so that the list is in document order, data holds what is app-owned (the run properties, the paragraph, the id of the
//   revision a move pairs with). Write keeps its live revision ids and recomputes where each revision is whenever it builds the list: it never
//   relies on a stored range. `path` may carry the range "from-to" for a person or a script to read, but it is advisory: text typed above a
//   change moves it, and the list is rebuilt (or shifted with ChangeList::shiftAfter) and not trusted to still be right.
// - JeffCrunch: a thread (NoteInfo::thread) is a Commented change: container is the sheet's name, object the cell's address, path "thread", the
//   comments after the first are `replies`, and data keeps the resolved flag (a resolved thread is Accepted). Position is crunchPosition(sheet,
//   row, col), so that the list runs down the workbook and no two cells share a number.
// The pane shows kind, author, when, and summary; it does not need to know more, and filters by container, author, and state.

#include <QDateTime>
#include <QHash>
#include <QJsonArray>
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

// A reply in the thread of a change (a comment's replies; Crunch's comments after the first).
struct Reply {
    QString author;
    QDateTime when;              // UTC; invalid when unknown
    QString text;

    QJsonObject toJson() const
    {
        QJsonObject o;
        o[QStringLiteral("author")] = author;
        if (when.isValid()) o[QStringLiteral("when")] = when.toUTC().toString(Qt::ISODate);
        o[QStringLiteral("text")] = text;
        return o;
    }
    static Reply fromJson(const QJsonObject &o)
    {
        Reply r;
        r.author = o.value(QStringLiteral("author")).toString();
        r.when = QDateTime::fromString(o.value(QStringLiteral("when")).toString(), Qt::ISODate);
        r.text = o.value(QStringLiteral("text")).toString();
        return r;
    }
};

// Most replies read from one change in a file (the files are not trusted); the rest are left out.
constexpr int kMaxReplies = 1000;

// The sort key of a Crunch cell: sheet, then row, then column, so that no two cells have the same number and the order is the workbook's. A column
// is under 2^14 (16,384, the most a sheet has), a row under 2^26 (a sheet has 2^20), a sheet under 2^23: the three fields never touch. A value outside
// its range (a file is not trusted) is held to the nearest edge of it, 0 or the largest, so the key stays whole, never negative, and never another cell's.
constexpr int kCrunchMaxSheet = (1 << 23) - 1, kCrunchMaxRow = (1 << 26) - 1, kCrunchMaxCol = (1 << 14) - 1;
constexpr int crunchClamp(int v, int hi) { return v < 0 ? 0 : v > hi ? hi : v; }
constexpr qint64 crunchPosition(int sheet, int row, int col)
{
    return (qint64(crunchClamp(sheet, kCrunchMaxSheet)) << 40) | (qint64(crunchClamp(row, kCrunchMaxRow)) << 14) | qint64(crunchClamp(col, kCrunchMaxCol));
}

struct Change {
    QString id;                  // told apart within a ChangeList
    Kind kind = Kind::Edited;
    QString author;
    QDateTime when;              // UTC; invalid when unknown
    Target target;               // `path` is advisory: an app finds the change by its container and object, never by a stored range
    QString summary;             // one line for the pane, already translated
    State state = State::Pending;
    qint64 position = 0;         // document order: the list is sorted by it, and next/previous follow it (a slide key, an offset, crunchPosition)
    QVector<Reply> replies;      // the thread under the change, oldest first
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
        o[QStringLiteral("position")] = QString::number(position);   // a string: a number in JSON is a double, which keeps 53 bits
        if (!replies.isEmpty()) {
            QJsonArray a;
            for (const Reply &r : replies) a.append(r.toJson());
            o[QStringLiteral("replies")] = a;
        }
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
        const QJsonValue pos = o.value(QStringLiteral("position"));
        c.position = pos.isString() ? pos.toString().toLongLong() : qint64(pos.toDouble());
        const QJsonArray replies = o.value(QStringLiteral("replies")).toArray();
        for (int i = 0; i < replies.size() && i < kMaxReplies; ++i) c.replies << Reply::fromJson(replies[i].toObject());
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
    // The place of the change with this id: a hash made on the first call (and again when the list has grown or shrunk), so that a lookup is one
    // step however long the list. A caller that gives a change another id in place calls reindex(). The first change with an id is the one found.
    int indexOf(const QString &id) const
    {
        if (m_indexed != changes.size()) reindex();
        const auto at = m_index.constFind(id);
        return at != m_index.cend() && *at < changes.size() && changes[*at].id == id ? *at : -1;
    }
    void reindex() const
    {
        m_index.clear();
        m_index.reserve(changes.size());
        for (int i = 0; i < changes.size(); ++i)
            if (!m_index.contains(changes[i].id)) m_index.insert(changes[i].id, i);
        m_indexed = int(changes.size());
        ++m_rebuilds;
    }
    int indexRebuilds() const { return m_rebuilds; }   // how many times the hash was made, for tests
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
    // Text (or cells) came or went in `container` at `position`: `delta` more positions (negative: fewer) from there on. A change at or after `position`
    // moves by `delta`; after a removal (negative `delta`) a change inside what was removed is put at `position`. The order of the list is kept.
    // Only `position` moves: `path` is advisory and is not rewritten, and a change in another container is not touched.
    void shiftAfter(const QString &container, qint64 position, qint64 delta)
    {
        for (Change &c : changes) {
            if (c.target.container != container || c.position < position) continue;
            if (delta < 0 && c.position < position - delta) c.position = position;
            else c.position += delta;
        }
    }

    QJsonObject toJson() const
    {
        QJsonArray a;
        for (const Change &c : changes) a.append(c.toJson());
        QJsonObject o;
        o[QStringLiteral("changes")] = a;
        o[QStringLiteral("tooMany")] = tooMany;
        return o;
    }
    // Reads at most kMaxChanges; if the file has more, `tooMany` is set.
    static ChangeList fromJson(const QJsonObject &o)
    {
        ChangeList list;
        const QJsonArray a = o.value(QStringLiteral("changes")).toArray();
        list.tooMany = o.value(QStringLiteral("tooMany")).toBool() || a.size() > kMaxChanges;
        for (int i = 0; i < a.size() && i < kMaxChanges; ++i) list.changes << Change::fromJson(a[i].toObject());
        return list;
    }

private:
    mutable QHash<QString, int> m_index;
    mutable int m_indexed = -1;
    mutable int m_rebuilds = 0;
};

} // namespace jp::review
