#pragma once
// Pack and Go (File > Export), the same in every app: the file is saved with what it needs on another computer, in one folder or one ZIP: the fonts it uses that come with the program
// (never one whose license forbids it, which the report names), and the files it links to (pictures, movies, sounds, linked documents), with each link rewritten to point inside the package.
// An app gives a PackAndGoSource: its fonts, its links, and how it saves a copy with new link targets. The Office Open XML apps (JeffWrite, JeffDeck, JeffCrunch) share OpcPackSource, which finds
// and rewrites the links of a saved package itself, so an app gives only the bytes of its file.
//
// Bounds (a file is a stranger's: its links are too): a link is followed only to a file of a kind Pack and Go takes (by its name), never to another computer, a device, or a program; a relative
// link may not climb out of the document's folder; the files are copied flat, so a link to the document itself or a loop of documents that link to each other is never followed; inside the package
// every name is "linked/<plain name>" or "fonts/<plain name>", never a path from the file; and the number and size of the files are capped, asking before the first cap and never passing the second.

#include "core/export.h"

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>
#include <utility>

class QWidget;
class QVBoxLayout;

namespace jp {

enum class PackKind { Picture, Movie, Sound, Document, Workbook, Object };

// One link of a document to a file: the text the document has (a path, relative to its folder or absolute, or a file: address).
struct PackLink {
    QString target;
    PackKind kind = PackKind::Picture;
};

struct PackLimits {
    // Over these the person is asked before anything is saved.
    int askFiles = 200;
    qint64 askBytes = qint64(256) << 20;
    // Never passed: the files after these are left out, and the report says so.
    int mostFiles = 2000;
    qint64 mostBytes = qint64(1) << 30;
};

class JO_COMMON_EXPORT PackAndGoSource {
public:
    virtual ~PackAndGoSource() = default;
    // The file name the copy in the package has, with its extension ("Budget.xlsx").
    virtual QString documentName() const = 0;
    // Where the document is saved now (empty when it is not): a link to it is not followed, and the folder is where relative links are counted from.
    virtual QString documentPath() const { return QString(); }
    // The families of the fonts the document uses.
    virtual QStringList fontFamilies() = 0;
    // The files the document links to.
    virtual QVector<PackLink> links() = 0;
    // What the person is told besides (a problem making the copy that does not stop it).
    virtual QStringList notes() const { return QStringList(); }
    // The document as it is saved, with each link in `moved` (the target as links() gave it, then the target to write) changed. False, with `error`, when it can't be made.
    virtual bool saveCopy(const QHash<QString, QString> &moved, QByteArray *bytes, QString *error) = 0;
};

// One part of a package: its name and its bytes.
struct PackPart {
    QString name;
    QByteArray data;
};

// A document that is an Office Open XML package (a ZIP). `savedBytes` is the file as the app saves it, with no password; `password` is the one the copy is locked with (empty: none).
// Links are found in the relationships of every part that point outside the package (pictures, movies, sounds, linked objects, linked workbooks), and in the INCLUDEPICTURE and INCLUDETEXT
// fields. The fonts are those the parts name (runs, styles, cells, shapes, charts, and the theme's two).
class JO_COMMON_EXPORT OpcPackSource : public PackAndGoSource {
public:
    QStringList fontFamilies() override;
    QVector<PackLink> links() override;
    bool saveCopy(const QHash<QString, QString> &moved, QByteArray *bytes, QString *error) override;
    // The links and families of a package, for a test.
    static QVector<PackLink> linksOf(const QByteArray &package, QString *error = nullptr);
    static QStringList fontsOf(const QByteArray &package);
    static QByteArray withLinks(const QByteArray &package, const QHash<QString, QString> &moved, QString *error = nullptr);

protected:
    virtual QByteArray savedBytes(QString *error) = 0;
    virtual QString password() const { return QString(); }

private:
    bool have(QString *error);
    QByteArray m_bytes;
    QVector<PackPart> m_parts;
    bool m_have = false;
};

// An OpcPackSource that gets its bytes from a function the app gives (a lambda over its own save): `save` is called once, and may add what to tell the person to `notes`.
class JO_COMMON_EXPORT BytesPackSource : public OpcPackSource {
public:
    using Save = std::function<QByteArray(QString *error, QStringList *notes)>;
    BytesPackSource(const QString &documentName, const QString &documentPath, const QString &password, Save save)
        : m_name(documentName), m_path(documentPath), m_password(password), m_save(std::move(save)) {}
    QString documentName() const override { return m_name; }
    QString documentPath() const override { return m_path; }
    QStringList notes() const override { return m_notes; }

protected:
    QByteArray savedBytes(QString *error) override { return m_save ? m_save(error, &m_notes) : QByteArray(); }
    QString password() const override { return m_password; }

private:
    QString m_name, m_path, m_password;
    Save m_save;
    QStringList m_notes;
};

// What a font file's license lets Pack and Go do, and the files it takes.
struct PackFontFile {
    QString name;
    QByteArray data;
};
struct PackedFonts {
    QVector<PackFontFile> files;
    QStringList leftOut;   // families (as the file names them) whose license does not let the file travel
};
// The files of the families that are in the font folders (the program's own by default), each once. The first folder that has any of them is the one used.
JO_COMMON_EXPORT PackedFonts packFonts(const QStringList &families, const QStringList &dirs = QStringList());
// "These fonts were not packed, because their licenses do not allow it: A, B." (empty when none).
JO_COMMON_EXPORT QString fontsLeftOutText(const QStringList &families);
// A ZIP written to a file all or nothing; false, with the system's words in `error`, when it can't be.
JO_COMMON_EXPORT bool writeZipFile(const QString &path, const QByteArray &bytes, QString *error);

enum class PackMode { Zip, Folder };

struct PackItem {
    enum Status { Packed, Missing, Refused };
    QString target;       // as the document has it
    QString file;         // the file found (empty when none)
    QString packedAs;     // "linked/<name>" when packed
    Status status = Missing;
    QString why;          // for a link not packed: in words
    qint64 size = 0;
};

struct PackReport {
    bool ok = false;
    bool cancelled = false;   // the person said no to the question about size
    QString error;
    QString path;             // the ZIP, or the folder, that was made
    QVector<PackItem> items;
    QStringList fontsPacked;  // file names
    QStringList fontsLeftOut;
    QStringList notes;
    int packed() const;
    qint64 packedBytes() const;
    // What to tell the person, in words.
    QString text() const;
};

// Asked with a question when the package is over the first cap: true to go on.
using PackAsk = std::function<bool(const QString &question)>;

// Makes the package. Zip: `destination` is the file; Folder: it is the folder the package's own folder (named for the document) is made in. Nothing is written when the person says no or
// the copy can't be made. `fontDirs` for a test (empty: the program's).
JO_COMMON_EXPORT PackReport packAndGo(PackAndGoSource &source, const QString &destination, PackMode mode, const PackLimits &limits = PackLimits(), const PackAsk &ask = PackAsk(),
                                      const QStringList &fontDirs = QStringList());

// The file a link names, or empty with the reason in `why`. `baseFolder` is the document's folder (empty: it is not saved), `documentPath` the document's own file.
JO_COMMON_EXPORT QString resolvePackLink(const QString &target, PackKind kind, const QString &baseFolder, const QString &documentPath, QString *why, bool *missing = nullptr);

// The two cards of a File > Export page, the same words in every app: each is told whether it is the folder one when it is clicked.
JO_COMMON_EXPORT void addPackAndGoCards(QVBoxLayout *layout, QWidget *page, const std::function<void(bool toFolder)> &run);
// The steps of the command every app runs from its card: where to save (empty when the person gave it up), the question about size, and what was packed or why not (true when a package was made;
// a package the person said no to is not shown).
JO_COMMON_EXPORT QString askPackAndGoDestination(QWidget *parent, const QString &documentName, bool toFolder);
JO_COMMON_EXPORT PackAsk packAndGoAsk(QWidget *parent);
JO_COMMON_EXPORT bool showPackAndGoReport(QWidget *parent, const PackReport &report);

} // namespace jp
