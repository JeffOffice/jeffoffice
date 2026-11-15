#pragma once
// "Set as Default for all new documents" in JeffWrite's dialogs: changes the Normal template, asks first before an unreadable one is replaced, and says why when the change can't be saved.

#include <QString>
#include <functional>

class QWidget;

namespace jp::write {

class WriteDoc;

// True when the template was changed. False when the person would not have the unreadable template replaced, or when it could not be saved (and the error is shown).
bool setNormalTemplate(QWidget *parent, const std::function<void(WriteDoc &)> &change);

// Stand-ins for the two boxes (a test sets them; the program never does): the answer to "Replace it?", and what is shown of an error. Set to empty functions, the boxes are shown.
void setNormalTemplatePrompts(const std::function<bool()> &askReplace, const std::function<void(const QString &)> &showError);

} // namespace jp::write
