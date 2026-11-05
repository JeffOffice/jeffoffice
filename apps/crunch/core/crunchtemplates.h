#pragma once
// The workbooks JeffCrunch has ready for the New page: a budget, an invoice, and so on. Each is made here, in code, as an .xlsx package (original layouts, written for JeffOffice); a workbook
// made from one is a copy with no file of its own (see CrunchFileScreen::buildNew).

#include <QByteArray>
#include <QImage>
#include <QString>
#include <QVector>

namespace jp::crunch {

struct TemplateInfo {
    QString id;            // "budget"
    QString name;          // "Monthly Budget" (in the person's language)
    QString category;      // "Finance"
    QString description;   // one sentence
};

// All of them, in the order the New page shows them.
QVector<TemplateInfo> builtinTemplates();
// The .xlsx package of one; empty for a name there is none of.
QByteArray builtinTemplateBytes(const QString &id);

// A small picture of the top of one template, `width` pixels across (null for a name there is none of).
QImage builtinTemplatePreview(const QString &id, int width);

} // namespace jp::crunch
