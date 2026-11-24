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
#include <QStorageInfo>
#include <QStandardPaths>
#include <QCoreApplication>
#include <algorithm>

namespace jp {

namespace {

QString g_sharedPath;
std::function<void()> g_loaderHook;
std::function<void(int)> g_writeStep;

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

BlockLibrary::BlockLibrary(const QString &path) : m_path(path), m_alive(std::make_shared<std::atomic<bool>>(true)) {}

BlockLibrary::~BlockLibrary()
{
    m_alive->store(false);
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
    out->extra = QJsonObject();
    out->newer = false;
    out->problem = Problem::NotALibrary;
    const QString notLibrary = QCoreApplication::translate("jp::BlockLibrary", "This is not a library of saved blocks.");
    QHash<QString, QByteArray> files;
    bool sawManifest = false, manifestTooLarge = false, parsed = false;
    QJsonObject root;
    QSet<QString> wanted;
    ZipScanStats stats;
    // (Only the manifest and the entry files are kept; the scan holds one entry's bytes at a time and stops at the limits. This version writes the manifest first, so the entries
    // it does not list are not kept; an older file has it last, and keeps them all until the manifest comes.)
    scanZipEach(zip, [&](ZipScanEntry &e) {
        if (e.name == QLatin1String("library.json")) {
            if (e.whole && e.data.size() <= kMostManifestBytes) {
                sawManifest = true;
                QJsonParseError parse;
                const QJsonDocument doc = QJsonDocument::fromJson(e.data, &parse);
                if (parse.error == QJsonParseError::NoError && doc.isObject()) {
                    parsed = true;
                    root = doc.object();
                    for (const QJsonValue &v : root.value(QStringLiteral("entries")).toArray())
                        if (v.isObject()) wanted.insert(entryFile(v.toObject().value(QStringLiteral("id")).toString()));
                }
            } else if (e.whole || e.problem.contains(QLatin1String("too large"))) {
                manifestTooLarge = true;
            }
        } else if (e.name.startsWith(QLatin1String("e/")) && e.name.endsWith(QLatin1String(".bin")) && e.whole && !e.data.isEmpty() && e.data.size() <= kMostEntryBytes) {
            if (!sawManifest || wanted.contains(e.name)) files.insert(e.name, e.data);
        }
    }, kMostEntries * 2 + 8, kMostEntryBytes, kMostTotalBytes + kMostManifestBytes, &stats);
    if (manifestTooLarge) {
        out->problem = Problem::Unreadable;
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "The list of saved blocks is too large to read.");
        return false;
    }
    if (!sawManifest || !parsed) {
        if (error) *error = notLibrary;
        return false;
    }
    const int version = root.value(QStringLiteral("version")).toInt(0);
    if (version < 1) {
        if (error) *error = notLibrary;
        return false;
    }
    out->problem = Problem::None;
    out->newer = version > kVersion;
    for (auto it = root.begin(); it != root.end(); ++it)
        if (it.key() != QLatin1String("version") && it.key() != QLatin1String("entries")) out->extra.insert(it.key(), it.value());
    static const QSet<QString> known = {QStringLiteral("id"), QStringLiteral("name"), QStringLiteral("gallery"), QStringLiteral("category"), QStringLiteral("description"),
                                        QStringLiteral("format"), QStringLiteral("insertAs"), QStringLiteral("created")};
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
        for (auto it = o.begin(); it != o.end(); ++it)
            if (!known.contains(it.key())) e.extra.insert(it.key(), it.value());
        total += e.data.size();
        seen.insert(e.info.id);
        out->entries.push_back(std::move(e));
    }
    return true;
}

bool BlockLibrary::readFile(const QString &path, Content *out, QString *error)
{
    out->entries.clear();
    out->extra = QJsonObject();
    out->newer = false;
    out->problem = Problem::None;
    QFile f(path);
    if (!f.exists()) return true;
    // (A file that cannot be read whole is no proof that it is no library: it is left as it is.)
    out->problem = Problem::Unreadable;
    if (f.size() > kMostFileBytes) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "The library of saved blocks is too large to read.");
        return false;
    }
    if (!f.open(QIODevice::ReadOnly)) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "The library of saved blocks could not be read: %1").arg(f.errorString());
        return false;
    }
    const qint64 expected = f.size();
    const QByteArray bytes = f.readAll();
    if (f.error() != QFileDevice::NoError || bytes.size() != expected) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "The library of saved blocks could not be read: %1").arg(f.errorString());
        return false;
    }
    return readBytes(bytes, out, error);
}

bool BlockLibrary::writeFile(const QString &path, const QVector<BlockEntry> &entries, const QJsonObject &extra, QString *error)
{
    // (Nothing is written that the reader would refuse.)
    qint64 total = 0;
    for (const BlockEntry &e : entries) total += e.data.size();
    if (entries.size() > kMostEntries || total > kMostTotalBytes) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "There are as many saved blocks as the library holds. Delete some to save this one.");
        return false;
    }
    QJsonArray list;
    for (const BlockEntry &e : entries) {
        QJsonObject o = e.extra;
        o.insert(QStringLiteral("id"), e.info.id);
        o.insert(QStringLiteral("name"), e.info.name);
        o.insert(QStringLiteral("gallery"), e.info.gallery);
        o.insert(QStringLiteral("category"), e.info.category);
        o.insert(QStringLiteral("description"), e.info.description);
        o.insert(QStringLiteral("format"), e.info.format);
        o.insert(QStringLiteral("insertAs"), int(e.info.insertAs));
        o.insert(QStringLiteral("created"), double(e.info.created));
        list.append(o);
    }
    QJsonObject root = extra;
    root.insert(QStringLiteral("version"), kVersion);
    root.insert(QStringLiteral("entries"), list);
    const QByteArray manifest = QJsonDocument(root).toJson(QJsonDocument::Compact);
    if (manifest.size() > kMostManifestBytes) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "The names and descriptions of the saved blocks have used up the room kept for them. Delete some blocks to save this one.");
        return false;
    }
    ZipWriter zip;
    zip.add(QStringLiteral("library.json"), manifest, true);   // (first: a reader knows which entries to keep)
    for (const BlockEntry &e : entries) zip.add(entryFile(e.info.id), e.data, false);
    const QByteArray bytes = zip.finish();
    const QString folder = QFileInfo(path).absolutePath();
    QDir().mkpath(folder);
    QSaveFile out(path);
    bool ok = out.open(QIODevice::WriteOnly);
    if (ok) {
        const qint64 chunk = qint64(1) << 20;
        int step = 0;
        for (qint64 at = 0; ok && at < bytes.size(); at += chunk) {
            const qint64 n = std::min(chunk, bytes.size() - at);
            ok = out.write(bytes.constData() + at, n) == n;
            if (ok && g_writeStep) g_writeStep(++step);
        }
    }
    if (!ok || !out.commit()) {
        if (error) {
            if (QStorageInfo(folder).bytesAvailable() < bytes.size())
                *error = QCoreApplication::translate("jp::BlockLibrary", "There is no room on the disk to save the blocks.");
            else
                *error = QCoreApplication::translate("jp::BlockLibrary", "The library of saved blocks could not be saved: %1").arg(out.errorString());
        }
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

void BlockLibrary::setLoaderHook(const std::function<void()> &hook) { g_loaderHook = hook; }
void BlockLibrary::setWriteStepHook(const std::function<void(int)> &step) { g_writeStep = step; }

bool BlockLibrary::load(QString *error)
{
    ++m_generation;
    Content c;
    const bool ok = readFile(m_path, &c, error);
    adopt(std::move(c));
    return ok;
}

namespace {

// Held by the event that hands a finished load over: when the event is run, the load was delivered; when it is thrown away unrun (the one who asked is gone), the load is abandoned.
struct Delivery {
    explicit Delivery(std::shared_ptr<std::atomic<bool>> flag) : abandoned(std::move(flag)) {}
    ~Delivery()
    {
        if (!delivered) abandoned->store(true);
    }
    std::shared_ptr<std::atomic<bool>> abandoned;
    bool delivered = false;
};

} // namespace

void BlockLibrary::loadInBackground(QObject *context, const std::function<void(bool)> &done)
{
    if (m_thread) {
        if (!(m_state && m_state->abandoned.load())) return;   // (still on its way)
        m_thread->join();
        m_thread.reset();
    }
    if (QFileInfo(m_path).size() < kBackgroundBytes) {
        const bool ok = load();
        if (done) done(ok);
        return;
    }
    const quint64 generation = ++m_generation;
    auto state = std::make_shared<LoadState>();
    m_state = state;
    auto alive = m_alive;
    QPointer<QObject> where(context);
    const QString path = m_path;
    m_thread = std::make_unique<WorkThread>([this, alive, state, generation, where, path, done] {
        auto content = std::make_shared<Content>();
        const bool ok = readFile(path, content.get(), nullptr);
        if (g_loaderHook) g_loaderHook();
        if (!where) {
            state->abandoned.store(true);
            return;
        }
        auto flag = std::shared_ptr<std::atomic<bool>>(state, &state->abandoned);
        auto delivery = std::make_shared<Delivery>(flag);
        QMetaObject::invokeMethod(where.data(), [this, alive, generation, content, ok, done, delivery] {
            delivery->delivered = true;
            if (!alive->load()) return;
            if (generation == m_generation) {   // (a change made since the load started has put newer content in place)
                adopt(std::move(*content));
                m_thread.reset();
            }
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
    ++m_generation;   // (a load that is still on its way is out of date)
    QDir().mkpath(QFileInfo(m_path).absolutePath());
    QLockFile lock(m_path + QStringLiteral(".lock"));
    lock.setStaleLockTime(30000);
    if (!lock.tryLock(10000)) {
        if (error) {
            if (lock.error() == QLockFile::PermissionError)
                *error = QCoreApplication::translate("jp::BlockLibrary", "The saved blocks cannot be changed: this computer does not let the program write in their folder.");
            else
                *error = QCoreApplication::translate("jp::BlockLibrary", "Another window is changing the saved blocks. Try again in a moment.");
        }
        return false;
    }
    Content c;
    QString problem;
    if (!readFile(m_path, &c, &problem)) {
        // (Only a file that was read completely and is no library is put aside, never written over; one that could not be read is left as it is, and the change is refused.)
        if (c.problem != Problem::NotALibrary) {
            if (error) *error = problem;
            return false;
        }
        QString aside;
        for (int n = 1; n <= kMostAsideFiles && aside.isEmpty(); ++n) {
            const QString candidate = m_path + QStringLiteral(".damaged") + (n > 1 ? QStringLiteral("-%1").arg(n) : QString());
            if (!QFileInfo::exists(candidate) && !QFileInfo(candidate).isSymLink()) aside = candidate;
        }
        if (aside.isEmpty()) {
            if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "The saved blocks file is damaged, and the folder already holds earlier damaged copies of it. Remove some of the files that end in .damaged from the folder %1 to go on.").arg(QDir::toNativeSeparators(QFileInfo(m_path).absolutePath()));
            return false;
        }
        if (!QFile::rename(m_path, aside)) {
            if (error) *error = problem;
            return false;
        }
        c = Content();
    }
    if (c.newer) {
        if (error) *error = QCoreApplication::translate("jp::BlockLibrary", "These saved blocks were made by a newer version of JeffOffice. Update the program to change them.");
        return false;
    }
    if (!apply(c.entries, error)) return false;
    if (!writeFile(m_path, c.entries, c.extra, error)) return false;
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
    return writeFile(file, out, QJsonObject(), error);
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
                if (err) *err = QCoreApplication::translate("jp::BlockLibrary", "There is no room in the library for the rest of these blocks.");
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
