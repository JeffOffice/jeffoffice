#include "app/versionstore.h"

#include "io/crypto.h"
#include "office/officecrypt.h"
#include "office/package.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

namespace jp::write {

namespace {

constexpr qint64 kLargestIndex = 4 << 20;   // (the list is read from a file the person could have changed: not past this)

bool plainName(const QString &file, const QString &extension)
{
    return !file.isEmpty() && file.size() < 80 && !file.contains(QLatin1Char('/')) && !file.contains(QLatin1Char('\\')) && !file.startsWith(QLatin1Char('.')) && file.endsWith(extension);
}

} // namespace

VersionStore::VersionStore(const QString &documentPath, const QString &extension) : m_extension(extension)
{
    const QFileInfo info(documentPath);
    const QString canonical = info.canonicalFilePath().isEmpty() ? info.absoluteFilePath() : info.canonicalFilePath();
    const QString id = QString::fromLatin1(QCryptographicHash::hash(canonical.toUtf8(), QCryptographicHash::Sha1).toHex());
    m_folder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/Versions/") + id;
}

QVector<DocVersion> VersionStore::load() const
{
    QVector<DocVersion> out;
    QFile f(m_folder + QStringLiteral("/versions.json"));
    if (!f.open(QIODevice::ReadOnly) || f.size() > kLargestIndex) return out;
    const QJsonArray rows = QJsonDocument::fromJson(f.readAll()).array();
    for (const QJsonValue &v : rows) {
        const QJsonObject o = v.toObject();
        DocVersion d;
        d.file = o.value(QStringLiteral("file")).toString();
        d.name = o.value(QStringLiteral("name")).toString();
        d.saved = QDateTime::fromString(o.value(QStringLiteral("saved")).toString(), Qt::ISODate);
        if (!plainName(d.file, m_extension)) continue;
        const QFileInfo copy(m_folder + QLatin1Char('/') + d.file);
        if (!copy.isFile()) continue;   // (a copy taken away by hand is not listed)
        d.size = copy.size();
        out.push_back(d);
        if (out.size() >= kMostVersions * 2) break;
    }
    return out;
}

bool VersionStore::store(const QVector<DocVersion> &all, QString *error) const
{
    QJsonArray rows;
    for (const DocVersion &d : all)
        rows.push_back(QJsonObject{{QStringLiteral("file"), d.file}, {QStringLiteral("name"), d.name}, {QStringLiteral("saved"), d.saved.toString(Qt::ISODate)}});
    QSaveFile f(m_folder + QStringLiteral("/versions.json"));
    if (!f.open(QIODevice::WriteOnly) || f.write(QJsonDocument(rows).toJson(QJsonDocument::Compact)) < 0 || !f.commit()) {
        if (error) *error = f.errorString();
        return false;
    }
    return true;
}

QVector<DocVersion> VersionStore::list() const
{
    QVector<DocVersion> all = load();
    std::reverse(all.begin(), all.end());   // (kept oldest first, shown newest first)
    return all;
}

bool VersionStore::add(const QString &name, const QByteArray &docx, QString *error)
{
    if (docx.isEmpty()) {
        if (error) *error = QCoreApplication::translate("VersionStore", "There is nothing to keep.");
        return false;
    }
    if (!QDir().mkpath(m_folder)) {
        if (error) *error = QCoreApplication::translate("VersionStore", "The folder for versions can't be made.");
        return false;
    }
    QVector<DocVersion> all = load();
    DocVersion d;
    d.saved = QDateTime::currentDateTime();
    // (a name that no copy has: the time to the millisecond, and a count if two come at once)
    QString base = d.saved.toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"));
    d.file = base + m_extension;
    for (int n = 2; QFileInfo::exists(m_folder + QLatin1Char('/') + d.file); ++n) d.file = base + QLatin1Char('-') + QString::number(n) + m_extension;
    d.name = name.simplified().left(120);
    QSaveFile f(m_folder + QLatin1Char('/') + d.file);
    if (!f.open(QIODevice::WriteOnly) || f.write(docx) != docx.size() || !f.commit()) {
        if (error) *error = f.errorString();
        return false;
    }
    d.size = docx.size();
    all.push_back(d);
    while (all.size() > kMostVersions) {
        QFile::remove(m_folder + QLatin1Char('/') + all.first().file);
        all.removeFirst();
    }
    return store(all, error);
}

QByteArray VersionStore::read(const QString &file) const
{
    if (!plainName(file, m_extension)) return {};
    bool listed = false;
    for (const DocVersion &d : load()) listed = listed || d.file == file;
    if (!listed) return {};
    QFile f(m_folder + QLatin1Char('/') + file);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

bool VersionStore::remove(const QString &file)
{
    if (!plainName(file, m_extension)) return false;
    QVector<DocVersion> all = load();
    bool found = false;
    for (int i = 0; i < all.size(); ++i)
        if (all[i].file == file) {
            all.removeAt(i);
            found = true;
            break;
        }
    if (!found) return false;
    QFile::remove(m_folder + QLatin1Char('/') + file);
    return store(all, nullptr);
}

bool VersionStore::replace(const QString &file, const QByteArray &bytes)
{
    if (bytes.isEmpty() || !plainName(file, m_extension)) return false;
    bool listed = false;
    for (const DocVersion &d : load()) listed = listed || d.file == file;
    if (!listed) return false;
    QSaveFile f(m_folder + QLatin1Char('/') + file);
    return f.open(QIODevice::WriteOnly) && f.write(bytes) == bytes.size() && f.commit();
}

void VersionStore::removeAll()
{
    QDir(m_folder).removeRecursively();
}

bool versionPackage(const QByteArray &stored, const QString &password, QByteArray *package)
{
    QByteArray info, encrypted;
    if (!office::splitProtectedPackage(stored, &info, &encrypted)) {
        *package = stored;
        return !stored.isEmpty();
    }
    return office::decryptPackage(info, encrypted, password, office::kMaxOpenFileBytes, false, package) == office::CryptResult::Ok;
}

bool VersionKey::open(const QByteArray &stored, const QString &filePassword, QByteArray *package) const
{
    return versionPackage(stored, kept, package) || versionPackage(stored, filePassword, package);
}

int VersionKey::follow(const QString &documentPath, const QString &extension, const QString &filePassword)
{
    int failed = 0;
    if (kept != filePassword) {
        VersionStore store(documentPath, extension);
        for (const DocVersion &v : store.list()) {
            QByteArray package;
            if (!open(store.read(v.file), filePassword, &package)) {
                ++failed;
                continue;
            }
            const QByteArray now = filePassword.isEmpty() ? package : office::encryptPackage(package, filePassword);
            if (now.isEmpty() || !store.replace(v.file, now)) ++failed;
        }
    }
    reset(filePassword);
    return failed;
}

void VersionKey::reset(const QString &password)
{
    crypto::wipe(kept);
    kept = password;
}

} // namespace jp::write
