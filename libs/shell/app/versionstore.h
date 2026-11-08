#pragma once
// Version history without a cloud service: named copies of a document kept on this computer, in a folder of the program's own data that belongs to the file's path. Nothing is sent
// anywhere. (A copy is a whole .docx; at most kMostVersions are kept for a file, and the oldest goes when there are more.)

#include <QByteArray>
#include <QDateTime>
#include <QString>
#include <QVector>

namespace jp::write {

constexpr int kMostVersions = 50;

struct DocVersion {
    QString file;     // the copy's file name in the folder (plain: never a path)
    QString name;     // what the person called it
    QDateTime saved;
    qint64 size = 0;
};

class VersionStore {
public:
    // The versions of the document that is the file `documentPath`.
    // `extension` is the ending of the copies' files: the kind of file the document is saved as (".docx" if none is given).
    explicit VersionStore(const QString &documentPath, const QString &extension = QStringLiteral(".docx"));
    // Where this file's versions are kept.
    QString folder() const { return m_folder; }
    // Newest first.
    QVector<DocVersion> list() const;
    // Keeps `docx` as a version. False, with the reason, when it can't be written.
    bool add(const QString &name, const QByteArray &docx, QString *error = nullptr);
    // The bytes of a version; empty if there is none by that file name.
    QByteArray read(const QString &file) const;
    bool remove(const QString &file);
    // Puts `bytes` in the place of the version's (it keeps its name and time). False if there is no such version or it can't be written.
    bool replace(const QString &file, const QByteArray &bytes);
    // Takes every version of the file away (and the folder).
    void removeAll();

private:
    QVector<DocVersion> load() const;
    bool store(const QVector<DocVersion> &all, QString *error) const;
    QString m_folder;
    QString m_extension;
};

// The package a kept version holds: a version of a file that asks for a password was kept protected, and is opened here with `password`. A version that is not protected is as it is.
// False when the password is not the one (or the version is damaged).
bool versionPackage(const QByteArray &stored, const QString &password, QByteArray *package);

// Gives every version kept for the file the protection the file has now: each is opened with `oldPassword` (a version in the clear is as it is) and protected with `newPassword`
// (empty: not protected). The number of versions that could not be changed (the old password is not theirs) is returned.
int reprotectVersions(const QString &documentPath, const QString &extension, const QString &oldPassword, const QString &newPassword);

} // namespace jp::write
