#pragma once
// The templates JeffWrite comes with: documents to start from (a letter, a report, a brief, a manuscript, a newsletter). They are made here, in code, as .docx files, so each is an
// ordinary document with styles of its own; nothing is read from outside the program.

#include <QString>
#include <QVector>

#include <memory>

namespace jp::write {

class WriteDoc;

struct BuiltInTemplate {
    QString id;         // never shown, never translated: how the template is asked for
    QString name;       // as shown
    QString category;   // the heading it is listed under
};

// Every template, in the order the New page shows them.
QVector<BuiltInTemplate> builtInTemplates();

// A new document made from the template; null for an id that names none.
std::unique_ptr<WriteDoc> builtInTemplateDocument(const QString &id);

} // namespace jp::write
