#pragma once
// The library of saved blocks: pieces of a document a person saved to insert again (a signature block, a paragraph of boilerplate, a letterhead), and short names that
// expand to text. One file in the person's data folder, shared by every app of the suite: each entry is a small document of the app that made it (`format` says which),
// kept as the bytes that app would save, so no new parser is needed and an app lists only the entries it can insert.
//
// The file is a ZIP: library.json (the version and each entry's properties) and e/<id>.bin (each entry's bytes). It is written whole to a new file and put in place
// (QSaveFile), under a lock file, and every change reads the file again under that lock and applies itself to what it finds there, so two windows, or two programs,
// that add and remove entries at the same time lose none of each other's changes. Nothing is written but by a call of an explicit person action: a document never adds
// an entry. The file is read within limits (entry count, bytes of an entry, bytes of all) and an entry that breaks one is left out, never the whole file.

#include "core/export.h"

#include <QByteArray>
#include <QDateTime>
#include <QJsonObject>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>
#include <atomic>
#include <memory>
#include <optional>

class QObject;

namespace jp {

class WorkThread;

enum class BlockInsert { Content = 0, Paragraph = 1, Page = 2 };   // where a block goes in: as it is, in a paragraph of its own, on a page of its own

struct BlockInfo {
    QString id;             // 32 lowercase hex digits, made when the entry is added
    QString name;
    QString gallery;        // kBlocksGallery, kShortcutsGallery, or the gallery of covers, tables, or text boxes (any other is kept and not shown)
    QString category;
    QString description;
    QString format;         // what the bytes are: "docx" for a word-processing file
    BlockInsert insertAs = BlockInsert::Content;
    qint64 created = 0;     // milliseconds since 1970
    qint64 size = 0;        // bytes of the entry
};

struct BlockEntry {
    BlockInfo info;
    QByteArray data;
    QJsonObject extra;   // fields of the entry this version does not know: kept when the file is written again
};

static const QString kBlocksGallery = QStringLiteral("saved");
static const QString kShortcutsGallery = QStringLiteral("shortcuts");
// The galleries of a person's own cover pages, tables, and text boxes: listed under the commands that put in the built-in ones.
static const QString kCoverPagesGallery = QStringLiteral("coverpages");
static const QString kTablesGallery = QStringLiteral("tables");
static const QString kTextBoxesGallery = QStringLiteral("textboxes");

class JO_COMMON_EXPORT BlockLibrary {
public:
    static constexpr int kVersion = 1;
    static constexpr int kMostEntries = 5000;
    static constexpr qint64 kMostEntryBytes = qint64(8) << 20;       // as much as a paste takes
    static constexpr qint64 kMostTotalBytes = qint64(256) << 20;     // of all the entries' bytes
    static constexpr qint64 kMostManifestBytes = qint64(4) << 20;
    static constexpr qint64 kMostFileBytes = kMostTotalBytes + kMostManifestBytes + (qint64(4) << 20);   // all the entries, the manifest, and the zip's own headers
    static constexpr int kMostAsideFiles = 9;                         // ".damaged", ".damaged-2", ... ".damaged-9": none is ever deleted
    static constexpr int kMostNameChars = 120;
    static constexpr int kMostTextChars = 1000;                      // a category or a description
    static constexpr qint64 kBackgroundBytes = qint64(2) << 20;      // a file this large is read on a thread of its own

    explicit BlockLibrary(const QString &path);
    ~BlockLibrary();
    BlockLibrary(const BlockLibrary &) = delete;
    BlockLibrary &operator=(const BlockLibrary &) = delete;

    // The person's library, in the folder every app of the suite shares; tests give `setSharedPath` a file of a folder of their own.
    static QString defaultPath();
    static BlockLibrary &shared();
    static void setSharedPath(const QString &path);

    const QString &path() const { return m_path; }

    // Reads the file (a missing file is an empty library). False, with `error`, for a file that is no library; the entries are then none.
    bool load(QString *error = nullptr);
    // As load, on a thread of its own when the file is large; `done(ok)` runs on the thread of `context` (at once, when the file is small). Only one at a time.
    void loadInBackground(QObject *context, const std::function<void(bool)> &done);
    bool loaded() const { return m_loaded; }
    bool loading() const { return m_thread != nullptr && !(m_state && m_state->abandoned.load()); }
    // For tests: `hook` runs on the loading thread after it has read the file, before the answer is handed over. And `step` runs in a write after each chunk of bytes (its argument counts from 1).
    // For tests: the limits are these (0: the usual one), so that a limit is reached with a few small blocks; call it with none to put them back.
    static void setLimitsForTest(int entries = 0, qint64 entryBytes = 0, qint64 totalBytes = 0, qint64 manifestBytes = 0);
    static void setLoaderHook(const std::function<void()> &hook);
    static void setWriteStepHook(const std::function<void(int)> &step);

    const QVector<BlockInfo> &list() const { return m_infos; }
    QVector<BlockInfo> list(const QString &gallery) const;
    // How many blocks a gallery has, at a cost that does not grow with the library (no list is made): what a caret that moves asks.
    int count(const QString &gallery) const { return m_galleryCounts.value(gallery); }
    // For tests: how many lists of one gallery were made since the program began.
    static qint64 galleryListsMadeForTest();
    std::optional<BlockInfo> find(const QString &id) const;
    std::optional<BlockInfo> findByName(const QString &gallery, const QString &name) const;   // the name without regard to case
    QByteArray data(const QString &id) const;

    // The changes. Each reads the file again under the lock, applies itself, writes, and then the library holds what the file now holds. An entry's id is made here when it has none
    // (or one that is taken); `replace` puts an entry of the same gallery and name in the place of the one that has it, else a taken name is refused.
    bool add(const BlockEntry &entry, QString *newId, QString *error, bool replace = false);
    bool update(const BlockInfo &info, QString *error);   // name, category, description, and how it is inserted; not the bytes
    bool remove(const QStringList &ids, QString *error);
    // Writes the entries named (all, when none) to a library file of their own, and takes in the entries of one: those whose gallery and name are taken are counted in `skipped`.
    bool exportTo(const QString &file, const QStringList &ids, QString *error) const;
    int importFrom(const QString &file, QString *error, int *skipped = nullptr);

    // A name a person may give: trimmed and cut to kMostNameChars, no line breaks.
    static QString cleanName(const QString &name);

private:
    // What a read found: a file that was read completely and is no library (`NotALibrary`) is the only one that may be put aside; one that could not be read, or is too large to read, is left as it is.
    enum class Problem { None, NotALibrary, Unreadable };
    struct Content {
        QVector<BlockEntry> entries;
        QJsonObject extra;    // fields of the manifest this version does not know
        bool newer = false;   // made by a later version: read for what it is, never written over
        Problem problem = Problem::None;
    };
    struct LoadState {
        std::atomic<bool> abandoned{false};   // the answer can never be delivered (the one who asked is gone)
    };
    static bool readFile(const QString &path, Content *out, QString *error);
    static bool readBytes(const QByteArray &zip, Content *out, QString *error);
    static bool writeFile(const QString &path, const QVector<BlockEntry> &entries, const QJsonObject &extra, QString *error);
    bool change(const std::function<bool(QVector<BlockEntry> &, QString *)> &apply, QString *error);
    void adopt(Content &&content);

    QString m_path;
    QVector<BlockEntry> m_entries;
    QVector<BlockInfo> m_infos;
    QHash<QString, int> m_galleryCounts;
    bool m_loaded = false;
    std::unique_ptr<WorkThread> m_thread;
    std::shared_ptr<std::atomic<bool>> m_alive;
    std::shared_ptr<LoadState> m_state;
    quint64 m_generation = 0;   // a load that was started before the last change or load is out of date
};

} // namespace jp
