// The comparison that is kept in a file (pptxreview.h).

#include "io/pptxreview.h"

#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>
#include <QMutex>
#include <QMutexLocker>

namespace jp::deck {

namespace {

const QString kCompare = QStringLiteral("compare");

struct Store {
    QMutex lock;
    QList<QPair<QString, QByteArray>> kept;   // oldest first
};
Store &store()
{
    static Store s;
    return s;
}

} // namespace

QString storeReviewTheirs(const QByteArray &bytes)
{
    if (bytes.isEmpty() || bytes.size() > kMaxReviewTheirsBytes) return {};
    const QString key = QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha1).toHex());
    Store &s = store();
    QMutexLocker lock(&s.lock);
    for (int i = 0; i < s.kept.size(); ++i)
        if (s.kept[i].first == key) {
            s.kept.move(i, s.kept.size() - 1);
            return key;
        }
    s.kept.append({key, bytes});
    while (s.kept.size() > kReviewStoreMost) s.kept.removeFirst();
    return key;
}

QByteArray storedReviewTheirs(const QString &key)
{
    Store &s = store();
    QMutexLocker lock(&s.lock);
    for (const auto &k : std::as_const(s.kept))
        if (k.first == key) return k.second;
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
    if (part.isEmpty() || part.size() > kMaxReviewPartBytes) return false;
    const QJsonDocument j = QJsonDocument::fromJson(part);
    if (!j.isObject() || j.object().value(QStringLiteral("format")).toInt() != 1) return false;
    const QByteArray theirs = QByteArray::fromBase64(j.object().value(QStringLiteral("theirs")).toString().toLatin1());
    const QString key = storeReviewTheirs(theirs);
    if (key.isEmpty()) return false;
    QJsonArray rejected;
    const QJsonArray in = j.object().value(QStringLiteral("rejected")).toArray();
    for (int i = 0; i < in.size() && i < kMaxReviewKeys; ++i)
        if (in[i].isString()) rejected.append(in[i].toString().left(500));
    QJsonObject c;
    c[QStringLiteral("theirs")] = key;
    c[QStringLiteral("rejected")] = rejected;
    c[QStringLiteral("states")] = QJsonObject();
    doc.appData[kCompare] = c;
    return true;
}

} // namespace jp::deck
