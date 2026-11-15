#include "core/normaltemplate.h"

#include "app/userdefaults.h"
#include "core/writedoc.h"

#include <QFile>
#include <QSaveFile>

namespace jp::write {

QString normalTemplatePath() { return userDefaultFile(QStringLiteral("Normal.docx")); }

namespace {
// The template as a document; null when there is no file, or it can't be read.
std::unique_ptr<WriteDoc> readNormalTemplate()
{
    QFile f(normalTemplatePath());
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) return nullptr;
    return WriteDoc::fromDocx(f.readAll());
}
} // namespace

std::unique_ptr<WriteDoc> newBlankDocument()
{
    if (std::unique_ptr<WriteDoc> doc = readNormalTemplate()) return doc;
    return WriteDoc::blank();
}

bool changeNormalTemplate(const std::function<void(WriteDoc &)> &change, QString *error)
{
    std::unique_ptr<WriteDoc> doc = readNormalTemplate();
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
