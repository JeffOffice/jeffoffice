// The comparison that is kept in a file (pptxreview.h).

#include "io/pptxreview.h"

#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <atomic>

namespace jp::deck {

namespace {

const QString kCompare = QStringLiteral("compare");
thread_local int t_openScopes = 0;
std::atomic<qint64> g_theirsLimit{-1}, g_partLimit{-1};

struct Entry {
    QString key;
    QByteArray bytes;
    QSet<const QObject *> owners;
};
struct Store {
    QMutex lock;
    QList<Entry> kept;   // oldest first
    int rawMade = 0;
};
Store &store()
{
    static Store s;
    return s;
}

// (the lock is held) the entries nobody uses are at most kReviewStoreMost: the oldest of them goes
void trimUnused(Store &s)
{
    int unused = 0;
    for (const Entry &e : std::as_const(s.kept))
        if (e.owners.isEmpty()) ++unused;
    for (int i = 0; i < s.kept.size() && unused > kReviewStoreMost;)
        if (s.kept[i].owners.isEmpty()) {
            s.kept.removeAt(i);
            --unused;
        } else
            ++i;
}

void release(const QString &key, const QObject *owner)
{
    Store &s = store();
    QMutexLocker lock(&s.lock);
    for (int i = 0; i < s.kept.size(); ++i)
        if (s.kept[i].key == key) {
            s.kept[i].owners.remove(owner);
            if (s.kept[i].owners.isEmpty()) s.kept.removeAt(i);
            return;
        }
}

QString keyOf(const QByteArray &bytes) { return QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha1).toHex()); }

} // namespace

ReviewOpenScope::ReviewOpenScope() { ++t_openScopes; }
ReviewOpenScope::~ReviewOpenScope() { --t_openScopes; }

ReviewCounters &reviewCounters()
{
    static ReviewCounters c;
    return c;
}

void setReviewLimitsForTest(qint64 theirsBytes, qint64 partBytes)
{
    g_theirsLimit = theirsBytes;
    g_partLimit = partBytes;
}
qint64 reviewTheirsLimit() { return g_theirsLimit >= 0 ? qint64(g_theirsLimit) : kMaxReviewTheirsBytes; }
qint64 reviewPartLimit() { return g_partLimit >= 0 ? qint64(g_partLimit) : kMaxReviewPartBytes; }

QString storeReviewTheirs(const QByteArray &bytes)
{
    if (bytes.isEmpty() || bytes.size() > reviewTheirsLimit()) return {};
    const QString key = keyOf(bytes);
    Store &s = store();
    QMutexLocker lock(&s.lock);
    for (int i = 0; i < s.kept.size(); ++i)
        if (s.kept[i].key == key) {
            s.kept.move(i, s.kept.size() - 1);
            return key;
        }
    s.kept.append({key, bytes, {}});
    trimUnused(s);
    return key;
}

void retainReviewTheirs(const QString &key, const QObject *owner)
{
    if (key.isEmpty() || !owner) return;
    Store &s = store();
    bool added = false;
    {
        QMutexLocker lock(&s.lock);
        for (Entry &e : s.kept)
            if (e.key == key) {
                added = !e.owners.contains(owner);
                e.owners.insert(owner);
            }
    }
    if (added) QObject::connect(owner, &QObject::destroyed, owner, [key, owner] { release(key, owner); }, Qt::DirectConnection);
}

QByteArray storedReviewTheirs(const QString &key)
{
    Store &s = store();
    QMutexLocker lock(&s.lock);
    for (const Entry &e : std::as_const(s.kept))
        if (e.key == key) return e.bytes;
    return {};
}

int storedReviewCount()
{
    Store &s = store();
    QMutexLocker lock(&s.lock);
    return int(s.kept.size());
}

void clearReviewStore()
{
    Store &s = store();
    QMutexLocker lock(&s.lock);
    s.kept.clear();
}

QString reviewPartName() { return QStringLiteral("/jo/review.json"); }
QString reviewOdfName() { return QStringLiteral("jo/review.json"); }

QByteArray reviewPartOf(const Document &doc)
{
    const QJsonObject c = doc.appData.value(kCompare).toObject();
    if (c.isEmpty()) return {};
    if (c.contains(QStringLiteral("raw"))) return storedReviewTheirs(c.value(QStringLiteral("raw")).toString());   // (not taken up by a window: as it was read)
    if (c.value(QStringLiteral("pending")).toInt() <= 0) return {};
    const QByteArray theirs = storedReviewTheirs(c.value(QStringLiteral("theirs")).toString());
    if (theirs.isEmpty()) return {};
    QJsonObject o;
    o[QStringLiteral("format")] = 1;
    o[QStringLiteral("theirs")] = QString::fromLatin1(theirs.toBase64());
    o[QStringLiteral("rejected")] = c.value(QStringLiteral("rejected")).toArray();
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

bool readReviewPart(const QByteArray &part, Document &doc)
{
    if (t_openScopes <= 0 || part.isEmpty() || part.size() > reviewPartLimit()) return false;
    Store &s = store();
    QString key;
    {
        QMutexLocker lock(&s.lock);
        key = QStringLiteral("raw:%1").arg(++s.rawMade);
        s.kept.append({key, part, {}});
        trimUnused(s);
    }
    QJsonObject c;
    c[QStringLiteral("raw")] = key;
    doc.appData[kCompare] = c;
    return true;
}

bool decodeReviewPart(Document &doc)
{
    const QJsonObject c = doc.appData.value(kCompare).toObject();
    const QString rawKey = c.value(QStringLiteral("raw")).toString();
    if (rawKey.isEmpty()) return !c.isEmpty();
    ++reviewCounters().decodes;
    const QByteArray part = storedReviewTheirs(rawKey);
    doc.appData.remove(kCompare);
    {
        Store &s = store();
        QMutexLocker lock(&s.lock);
        for (int i = 0; i < s.kept.size(); ++i)
            if (s.kept[i].key == rawKey) {
                s.kept.removeAt(i);
                break;
            }
    }
    if (part.isEmpty()) return false;
    const QJsonDocument j = QJsonDocument::fromJson(part);
    if (!j.isObject() || j.object().value(QStringLiteral("format")).toInt() != 1) return false;
    const QByteArray theirs = QByteArray::fromBase64(j.object().value(QStringLiteral("theirs")).toString().toLatin1());
    const QString key = storeReviewTheirs(theirs);
    if (key.isEmpty()) return false;
    QJsonArray rejected;
    const QJsonArray in = j.object().value(QStringLiteral("rejected")).toArray();
    for (int i = 0; i < in.size() && i < kMaxReviewKeys; ++i)
        if (in[i].isString()) rejected.append(in[i].toString().left(500));
    QJsonObject out;
    out[QStringLiteral("theirs")] = key;
    out[QStringLiteral("rejected")] = rejected;
    out[QStringLiteral("states")] = QJsonObject();
    out[QStringLiteral("pending")] = 1;   // (the changes are found again when the window has the other presentation)
    doc.appData[kCompare] = out;
    return true;
}

} // namespace jp::deck
