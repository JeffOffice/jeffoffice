#include "io/blocklibrary.h"

#include "core/workthread.h"
#include "io/zip.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QMetaObject>
#include <QPointer>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QStandardPaths>
#include <QCoreApplication>
#include <algorithm>

namespace jp {

namespace {

QString g_sharedPath;

bool validId(const QString &id)
{
    static const QRegularExpression re(QStringLiteral("^[0-9a-f]{32}$"));
    return re.match(id).hasMatch();
}

QString newId()
{
    QString s;
    for (int i = 0; i < 4; ++i) s += QStringLiteral("%1").arg(QRandomGenerator::global()->generate(), 8, 16, QLatin1Char('0'));
    return s;
}

QString clipped(const QString &s, int most) { return s.left(most); }

QString entryFile(const QString &id) { return QStringLiteral("e/") + id + QStringLiteral(".bin"); }

} // namespace

BlockLibrary::BlockLibrary(const QString &path) : m_path(path) {}

BlockLibrary::~BlockLibrary()
{
    if (m_alive) m_alive->store(false);
    if (m_thread) m_thread->join();
}

QString BlockLibrary::defaultPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/JeffOffice/Saved Blocks/library.jblocks");
}

void BlockLibrary::setSharedPath(const QString &path) { g_sharedPath = path; }

BlockLibrary &BlockLibrary::shared()
{
    static std::unique_ptr<BlockLibrary> lib;
    static QString of;
    const QString want = g_sharedPath.isEmpty() ? defaultPath() : g_sharedPath;
    if (!lib || of != want) {
        lib = std::make_unique<BlockLibrary>(want);
        of = want;
    }
    return *lib;
}

QString BlockLibrary::cleanName(const QString &name)
{
    QString s = name;
    s.replace(QRegularExpression(QStringLiteral("[\\r\\n\\t]+")), QStringLiteral(" "));
    return s.trimmed().left(kMostNameChars).trimmed();
}

bool BlockLibrary::readBytes(const QByteArray &zip, Content *out, QString *error)
{
    out->entries.clear();
    QByteArray manifest;
    QHash<QString, QByteArray> files;
    bool sawManifest = false;
    QSet<QString> wanted;
    ZipScanStats stats;
    // (Only the manifest and the entry files are kept; the scan holds one entry's bytes at a time and stops at the limits.)
    scanZipEach(zip, [&](ZipScanEntry &e) {
        if (e.name == QLatin1String("library.json")) {
            if (e.whole && e.data.size() <= kMostManifestBytes) {
                manifest = e.data;
                sawManifest = true;
            }
        } else if (e.name.startsWith(QLatin1String("e/")) && e.name.endsWith(QLatin1String(".bin")) && e.whole && !e.data.isEmpty() && e.data.size() <= kMostEntryBytes) {
            files.insert(e.name, e.data);
        }
    }, kMostEntries * 2 + 8, kMostEntryBytes, kMostTotalBytes, &stats);
    if (!sawManifest) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "This is not a library of saved blocks.");
        return false;
    }
    QJsonParseError parse;
    const QJsonDocument doc = QJsonDocument::fromJson(manifest, &parse);
    if (parse.error != QJsonParseError::NoError || !doc.isObject()) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "This is not a library of saved blocks.");
        return false;
    }
    const QJsonObject root = doc.object();
    const int version = root.value(QStringLiteral("version")).toInt(0);
    if (version < 1) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "This is not a library of saved blocks.");
        return false;
    }
    out->newer = version > kVersion;
    QSet<QString> seen;
    qint64 total = 0;
    const QJsonArray list = root.value(QStringLiteral("entries")).toArray();
    for (const QJsonValue &v : list) {
        if (out->entries.size() >= kMostEntries) break;
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();
        BlockEntry e;
        e.info.id = o.value(QStringLiteral("id")).toString();
        if (!validId(e.info.id) || seen.contains(e.info.id)) continue;
        e.data = files.value(entryFile(e.info.id));
        if (e.data.isEmpty() || total + e.data.size() > kMostTotalBytes) continue;
        e.info.name = cleanName(o.value(QStringLiteral("name")).toString());
        if (e.info.name.isEmpty()) continue;
        e.info.gallery = clipped(o.value(QStringLiteral("gallery")).toString(), 40);
        e.info.category = clipped(o.value(QStringLiteral("category")).toString(), kMostTextChars);
        e.info.description = clipped(o.value(QStringLiteral("description")).toString(), kMostTextChars);
        e.info.format = clipped(o.value(QStringLiteral("format")).toString(), 16);
        const int how = o.value(QStringLiteral("insertAs")).toInt(0);
        e.info.insertAs = how == 1 ? BlockInsert::Paragraph : how == 2 ? BlockInsert::Page : BlockInsert::Content;
        e.info.created = qint64(o.value(QStringLiteral("created")).toDouble(0));
        e.info.size = e.data.size();
        total += e.data.size();
        seen.insert(e.info.id);
        out->entries.push_back(std::move(e));
    }
    return true;
}

bool BlockLibrary::readFile(const QString &path, Content *out, QString *error)
{
    out->entries.clear();
    out->newer = false;
    QFile f(path);
    if (!f.exists()) return true;
    if (f.size() > kMostTotalBytes + kMostManifestBytes || !f.open(QIODevice::ReadOnly)) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "The library of saved blocks could not be read.");
        return false;
    }
    return readBytes(f.readAll(), out, error);
}

bool BlockLibrary::writeFile(const QString &path, const QVector<BlockEntry> &entries, QString *error)
{
    QJsonArray list;
    ZipWriter zip;
    for (const BlockEntry &e : entries) {
        QJsonObject o;
        o.insert(QStringLiteral("id"), e.info.id);
        o.insert(QStringLiteral("name"), e.info.name);
        o.insert(QStringLiteral("gallery"), e.info.gallery);
        o.insert(QStringLiteral("category"), e.info.category);
        o.insert(QStringLiteral("description"), e.info.description);
        o.insert(QStringLiteral("format"), e.info.format);
        o.insert(QStringLiteral("insertAs"), int(e.info.insertAs));
        o.insert(QStringLiteral("created"), double(e.info.created));
        list.append(o);
        zip.add(entryFile(e.info.id), e.data, false);
    }
    QJsonObject root;
    root.insert(QStringLiteral("version"), kVersion);
    root.insert(QStringLiteral("entries"), list);
    zip.add(QStringLiteral("library.json"), QJsonDocument(root).toJson(QJsonDocument::Compact), true);
    const QByteArray bytes = zip.finish();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile out(path);
    if (!out.open(QIODevice::WriteOnly) || out.write(bytes) != bytes.size() || !out.commit()) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "The library of saved blocks could not be saved: %1").arg(out.errorString());
        return false;
    }
    return true;
}

void BlockLibrary::adopt(Content &&content)
{
    m_entries = std::move(content.entries);
    m_infos.clear();
    m_infos.reserve(m_entries.size());
    for (const BlockEntry &e : std::as_const(m_entries)) m_infos.push_back(e.info);
    m_loaded = true;
}

bool BlockLibrary::load(QString *error)
{
    Content c;
    const bool ok = readFile(m_path, &c, error);
    adopt(std::move(c));
    return ok;
}

void BlockLibrary::loadInBackground(QObject *context, const std::function<void(bool)> &done)
{
    if (m_thread) return;
    if (QFileInfo(m_path).size() < kBackgroundBytes) {
        const bool ok = load();
        if (done) done(ok);
        return;
    }
    auto alive = std::make_shared<std::atomic<bool>>(true);
    m_alive = alive;
    QPointer<QObject> where(context);
    const QString path = m_path;
    m_thread = std::make_unique<WorkThread>([this, alive, where, path, done] {
        auto content = std::make_shared<Content>();
        const bool ok = readFile(path, content.get(), nullptr);
        if (!where) return;
        QMetaObject::invokeMethod(where.data(), [this, alive, content, ok, done] {
            if (!alive->load()) return;
            adopt(std::move(*content));
            m_thread.reset();
            if (done) done(ok);
        }, Qt::QueuedConnection);
    });
}

QVector<BlockInfo> BlockLibrary::list(const QString &gallery) const
{
    QVector<BlockInfo> out;
    for (const BlockInfo &i : m_infos)
        if (i.gallery == gallery) out.push_back(i);
    return out;
}

std::optional<BlockInfo> BlockLibrary::find(const QString &id) const
{
    for (const BlockInfo &i : m_infos)
        if (i.id == id) return i;
    return std::nullopt;
}

std::optional<BlockInfo> BlockLibrary::findByName(const QString &gallery, const QString &name) const
{
    for (const BlockInfo &i : m_infos)
        if (i.gallery == gallery && i.name.compare(name, Qt::CaseInsensitive) == 0) return i;
    return std::nullopt;
}

QByteArray BlockLibrary::data(const QString &id) const
{
    for (const BlockEntry &e : m_entries)
        if (e.info.id == id) return e.data;
    return QByteArray();
}

// Reads the file again under the lock, applies the change to what it holds, writes it, and then holds that.
bool BlockLibrary::change(const std::function<bool(QVector<BlockEntry> &, QString *)> &apply, QString *error)
{
    if (m_thread) m_thread->join(), m_thread.reset();
    QDir().mkpath(QFileInfo(m_path).absolutePath());
    QLockFile lock(m_path + QStringLiteral(".lock"));
    lock.setStaleLockTime(30000);
    if (!lock.tryLock(10000)) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "Another window is changing the saved blocks. Try again in a moment.");
        return false;
    }
    Content c;
    QString problem;
    if (!readFile(m_path, &c, &problem)) {
        // (A file that is no library is put aside, not written over.)
        QFile::remove(m_path + QStringLiteral(".damaged"));
        if (!QFile::rename(m_path, m_path + QStringLiteral(".damaged"))) {
            if (error) *error = problem;
            return false;
        }
        c.entries.clear();
        c.newer = false;
    }
    if (c.newer) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "These saved blocks were made by a newer version of JeffOffice. Update the program to change them.");
        return false;
    }
    if (!apply(c.entries, error)) return false;
    if (!writeFile(m_path, c.entries, error)) return false;
    adopt(std::move(c));
    return true;
}

bool BlockLibrary::add(const BlockEntry &entry, QString *newIdOut, QString *error, bool replace)
{
    BlockEntry e = entry;
    e.info.name = cleanName(e.info.name);
    e.info.category = clipped(e.info.category, kMostTextChars);
    e.info.description = clipped(e.info.description, kMostTextChars);
    if (e.info.name.isEmpty()) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "A saved block needs a name.");
        return false;
    }
    if (e.data.isEmpty() || e.data.size() > kMostEntryBytes) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "This is too large to save as a block.");
        return false;
    }
    e.info.size = e.data.size();
    if (e.info.created == 0) e.info.created = QDateTime::currentMSecsSinceEpoch();
    return change([&](QVector<BlockEntry> &all, QString *err) {
        int same = -1;
        QSet<QString> ids;
        qint64 total = 0;
        for (int i = 0; i < all.size(); ++i) {
            ids.insert(all[i].info.id);
            total += all[i].data.size();
            if (all[i].info.gallery == e.info.gallery && all[i].info.name.compare(e.info.name, Qt::CaseInsensitive) == 0) same = i;
        }
        if (same >= 0) {
            if (!replace) {
                if (err) *err = QCoreApplication::translate("jp::BlockLibrary", "A block with this name is already saved.");
                return false;
            }
            total -= all[same].data.size();
            ids.remove(all[same].info.id);
            all.remove(same);
        }
        if (all.size() >= kMostEntries || total + e.data.size() > kMostTotalBytes) {
            if (err) *err = QCoreApplication::translate("jp::BlockLibrary", "There are as many saved blocks as the library holds. Delete some to save this one.");
            return false;
        }
        if (!validId(e.info.id) || ids.contains(e.info.id)) {
            do e.info.id = newId();
            while (ids.contains(e.info.id));
        }
        if (newIdOut) *newIdOut = e.info.id;
        all.push_back(e);
        return true;
    }, error);
}

bool BlockLibrary::update(const BlockInfo &info, QString *error)
{
    const QString name = cleanName(info.name);
    if (name.isEmpty()) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "A saved block needs a name.");
        return false;
    }
    return change([&](QVector<BlockEntry> &all, QString *err) {
        int at = -1;
        for (int i = 0; i < all.size(); ++i) {
            if (all[i].info.id == info.id) at = i;
            else if (all[i].info.gallery == info.gallery && all[i].info.name.compare(name, Qt::CaseInsensitive) == 0) {
                if (err) *err = QCoreApplication::translate("jp::BlockLibrary", "A block with this name is already saved.");
                return false;
            }
        }
        if (at < 0) {
            if (err) *err = QCoreApplication::translate("jp::BlockLibrary", "This block is no longer in the library.");
            return false;
        }
        BlockInfo &i = all[at].info;
        i.name = name;
        i.category = clipped(info.category, kMostTextChars);
        i.description = clipped(info.description, kMostTextChars);
        i.insertAs = info.insertAs;
        return true;
    }, error);
}

bool BlockLibrary::remove(const QStringList &ids, QString *error)
{
    const QSet<QString> gone(ids.begin(), ids.end());
    return change([&](QVector<BlockEntry> &all, QString *) {
        all.erase(std::remove_if(all.begin(), all.end(), [&](const BlockEntry &e) { return gone.contains(e.info.id); }), all.end());
        return true;
    }, error);
}

bool BlockLibrary::exportTo(const QString &file, const QStringList &ids, QString *error) const
{
    QVector<BlockEntry> out;
    const QSet<QString> want(ids.begin(), ids.end());
    for (const BlockEntry &e : m_entries)
        if (want.isEmpty() || want.contains(e.info.id)) out.push_back(e);
    return writeFile(file, out, error);
}

int BlockLibrary::importFrom(const QString &file, QString *error, int *skipped)
{
    Content theirs;
    if (!readFile(file, &theirs, error)) return -1;
    if (!QFileInfo::exists(file)) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "This is not a library of saved blocks.");
        return -1;
    }
    int taken = 0, added = 0;
    const bool ok = change([&](QVector<BlockEntry> &all, QString *err) {
        QSet<QString> ids;
        qint64 total = 0;
        for (const BlockEntry &e : all) {
            ids.insert(e.info.id);
            total += e.data.size();
        }
        for (BlockEntry &e : theirs.entries) {
            bool same = false;
            for (const BlockEntry &have : std::as_const(all))
                if (have.info.gallery == e.info.gallery && have.info.name.compare(e.info.name, Qt::CaseInsensitive) == 0) same = true;
            if (same) {
                ++taken;
                continue;
            }
            if (all.size() >= kMostEntries || total + e.data.size() > kMostTotalBytes) {
                if (err) *err = QCoreApplication::translate("jp::BlockLibrary", "There are as many saved blocks as the library holds. Delete some to save this one.");
                return added > 0;
            }
            if (ids.contains(e.info.id))
                do e.info.id = newId();
                while (ids.contains(e.info.id));
            ids.insert(e.info.id);
            total += e.data.size();
            all.push_back(e);
            ++added;
        }
        return true;
    }, error);
    if (skipped) *skipped = taken;
    return ok ? added : -1;
}

} // namespace jp
