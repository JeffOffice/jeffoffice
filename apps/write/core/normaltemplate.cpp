#include "core/normaltemplate.h"

#include "app/userdefaults.h"
#include "core/commentops.h"
#include "core/reviewops.h"
#include "core/writedoc.h"
#include "office/packageprops.h"

#include <QCoreApplication>
#include <QFile>
#include <QLockFile>
#include <QSaveFile>

namespace jp::write {

QString normalTemplatePath() { return userDefaultFile(QStringLiteral("Normal.docx")); }

namespace {

// The template as a document; null when there is no file, or it can't be used (too large, not readable, or not a document), and then `why` says so.
std::unique_ptr<WriteDoc> readNormalTemplate(NormalTemplateState *state)
{
    *state = NormalTemplateState::Missing;
    QFile f(normalTemplatePath());
    if (!f.exists()) return nullptr;
    *state = NormalTemplateState::Unreadable;
    if (f.size() > kMaxNormalTemplateBytes || !f.open(QIODevice::ReadOnly)) return nullptr;
    const QByteArray bytes = f.read(kMaxNormalTemplateBytes + 1);
    if (bytes.size() > kMaxNormalTemplateBytes) return nullptr;
    std::unique_ptr<WriteDoc> doc = WriteDoc::fromDocx(bytes);
    if (doc) *state = NormalTemplateState::Usable;
    return doc;
}

} // namespace

NormalTemplateState normalTemplateState()
{
    NormalTemplateState state;
    readNormalTemplate(&state);
    return state;
}

std::unique_ptr<WriteDoc> newBlankDocument(QString *note)
{
    NormalTemplateState state;
    if (std::unique_ptr<WriteDoc> doc = readNormalTemplate(&state)) {
        // As a template makes a document: what it holds stays; the person it was made by, the dates, the comments, and the changes it tracked do not.
        doc->setPackageProps(office::PackageProps());
        deleteAllComments(*doc);
        acceptAll(*doc);
        doc->setModified(false);
        return doc;
    }
    if (state == NormalTemplateState::Unreadable && note)
        *note = QCoreApplication::translate("jp::write::NormalTemplate", "The Normal template can't be read, so this document starts with the program's own settings.");
    return WriteDoc::blank();
}

bool changeNormalTemplate(const std::function<void(WriteDoc &)> &change, QString *error, bool replaceUnreadable)
{
    // The file is locked from the reading to the writing: a change made in another window meanwhile is waited for, and so is the one that follows this one.
    QLockFile lock(normalTemplatePath() + QStringLiteral(".lock"));
    lock.setStaleLockTime(30000);
    if (!lock.tryLock(10000)) {
        if (error) *error = QCoreApplication::translate("jp::write::NormalTemplate", "the Normal template is in use by another window");
        return false;
    }
    NormalTemplateState state;
    std::unique_ptr<WriteDoc> doc = readNormalTemplate(&state);
    if (state == NormalTemplateState::Unreadable && !replaceUnreadable) {
        if (error) *error = QCoreApplication::translate("jp::write::NormalTemplate", "the Normal template can't be read");
        return false;
    }
    if (!doc) doc = WriteDoc::blank();
    change(*doc);
    QString problem;
    const QByteArray bytes = doc->toDocx(&problem);
    if (bytes.isEmpty()) {
        if (error) *error = problem;
        return false;
    }
    QSaveFile out(normalTemplatePath());
    if (!out.open(QIODevice::WriteOnly) || out.write(bytes) != bytes.size() || !out.commit()) {
        if (error) *error = out.errorString();
        return false;
    }
    return true;
}

} // namespace jp::write
