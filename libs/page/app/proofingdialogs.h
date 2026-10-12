#pragma once
// Spelling and the thesaurus, as dialog boxes that work on an Editor.

#include <QString>
#include <QStringList>

class QWidget;

namespace jp {

class Editor;

void spellingDialog(QWidget *p, Editor *ed);
void thesaurusDialog(QWidget *p, Editor *ed);
QStringList thesaurusLookup(const QString &word);

} // namespace jp
