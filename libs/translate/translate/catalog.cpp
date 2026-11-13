#include "catalog.h"
#include "languages.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

namespace jp {
namespace translate {

qint64 ModelPair::totalBytes() const
{
    qint64 n = 0;
    for (const ModelFile &f : files) n += f.size;
    return n;
}

namespace {

QString fileNameFor(const QString &role, const QString &fromTo)
{
    return role + QStringLiteral(".") + fromTo + QStringLiteral(".bin");
}

} // namespace

Catalog Catalog::fromJson(const QByteArray &json, QString *error, bool allowLocal)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return Catalog();
    };
    QJsonParseError parse;
    const QJsonDocument doc = QJsonDocument::fromJson(json, &parse);
    if (doc.isNull() || !doc.isObject()) return fail(QStringLiteral("not JSON: ") + parse.errorString());
    const QJsonObject root = doc.object();
    const QString base = root.value(QStringLiteral("baseUrl")).toString();
    const QJsonObject pairs = root.value(QStringLiteral("pairs")).toObject();
    if (pairs.isEmpty()) return fail(QStringLiteral("no pairs"));
    static const QRegularExpression hex(QStringLiteral("^[0-9a-f]{64}$"));
    static const QRegularExpression pairKey(QStringLiteral("^[a-z]{2,3}-[a-z]{2,3}$"));
    Catalog c;
    for (auto it = pairs.begin(); it != pairs.end(); ++it) {
        if (!pairKey.match(it.key()).hasMatch()) return fail(QStringLiteral("bad pair name ") + it.key());
        ModelPair p;
        p.from = it.key().section(QLatin1Char('-'), 0, 0);
        p.to = it.key().section(QLatin1Char('-'), 1, 1);
        const QJsonObject files = it.value().toObject().value(QStringLiteral("files")).toObject();
        bool hasModel = false, hasVocab = false;
        for (auto f = files.begin(); f != files.end(); ++f) {
            const QJsonObject o = f.value().toObject();
            ModelFile mf;
            mf.role = f.key();
            if (mf.role != QLatin1String("model") && mf.role != QLatin1String("shortlist") && mf.role != QLatin1String("vocab") && mf.role != QLatin1String("srcvocab") && mf.role != QLatin1String("trgvocab"))
                return fail(QStringLiteral("unknown file ") + it.key() + QLatin1Char('/') + mf.role);
            const QString address = o.value(QStringLiteral("url")).toString();
            mf.url = QUrl(address.contains(QLatin1String("://")) ? address : base + address, QUrl::StrictMode);
            const QString scheme = mf.url.scheme();
            const bool loopback = scheme == QLatin1String("http") && (mf.url.host() == QLatin1String("127.0.0.1") || mf.url.host() == QLatin1String("localhost"));
            if (!mf.url.isValid() || !(scheme == QLatin1String("https") || (allowLocal && (scheme == QLatin1String("file") || loopback))))
                return fail(QStringLiteral("the address of ") + it.key() + QLatin1Char('/') + mf.role + QStringLiteral(" is not a secure web address"));
            const QJsonValue sz = o.value(QStringLiteral("size"));
            if (!sz.isDouble() || sz.toDouble() <= 0 || sz.toDouble() > 4e9) return fail(QStringLiteral("the size of ") + it.key() + QLatin1Char('/') + mf.role + QStringLiteral(" is wrong"));
            mf.size = qint64(sz.toDouble());
            mf.sha256 = o.value(QStringLiteral("sha256")).toString().toLower();
            if (!hex.match(mf.sha256).hasMatch()) return fail(QStringLiteral("the SHA-256 of ") + it.key() + QLatin1Char('/') + mf.role + QStringLiteral(" is wrong"));
            mf.fileName = fileNameFor(mf.role, it.key());
            hasModel |= mf.role == QLatin1String("model");
            hasVocab |= mf.role == QLatin1String("vocab") || mf.role == QLatin1String("srcvocab");
            p.files.push_back(mf);
        }
        if (!hasModel || !hasVocab) return fail(QStringLiteral("the files of ") + it.key() + QStringLiteral(" are incomplete"));
        c.m_pairs.insert(it.key(), p);
    }
    return c;
}

const Catalog &Catalog::builtIn()
{
    static const Catalog c = [] {
        QFile f(QStringLiteral(":/translate/models.json"));
        if (!f.open(QIODevice::ReadOnly)) return Catalog();
        return fromJson(f.readAll());
    }();
    return c;
}

QVector<ModelPair> Catalog::route(const QString &from, const QString &to) const
{
    if (from == to) return {};
    if (hasPair(from, to)) return {pair(from, to)};
    const QString pivot = QString::fromLatin1(kPivot);
    if (from != pivot && to != pivot && hasPair(from, pivot) && hasPair(pivot, to)) return {pair(from, pivot), pair(pivot, to)};
    return {};
}

bool Catalog::canTranslate(const QString &from, const QString &to) const { return !route(from, to).isEmpty(); }

QStringList Catalog::languages() const
{
    QStringList out;
    const QString pivot = QString::fromLatin1(kPivot);
    for (const ModelPair &p : m_pairs) {
        const QString other = p.from == pivot ? p.to : p.to == pivot ? p.from : QString();
        if (!other.isEmpty() && !out.contains(other) && hasPair(pivot, other) && hasPair(other, pivot)) out << other;
    }
    return out;
}

} // namespace translate
} // namespace jp
