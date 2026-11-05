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
    explicit VersionStore(const QString &documentPath);
    // Where this file's versions are kept.
    QString folder() const { return m_folder; }
    // Newest first.
    QVector<DocVersion> list() const;
    // Keeps `docx` as a version. False, with the reason, when it can't be written.
    bool add(const QString &name, const QByteArray &docx, QString *error = nullptr);
    // The bytes of a version; empty if there is none by that file name.
    QByteArray read(const QString &file) const;
    bool remove(const QString &file);
    // Takes every version of the file away (and the folder).
    void removeAll();

private:
    QVector<DocVersion> load() const;
    bool store(const QVector<DocVersion> &all, QString *error) const;
    QString m_folder;
};

} // namespace jp::write
