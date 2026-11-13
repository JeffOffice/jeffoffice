#pragma once
// Options: the pages every app shares (General, with the suite's settings, and
// Proofing), then the app's own.

#include <QString>
#include <QVector>
#include <functional>

class QWidget;

namespace jp {

class Editor;

// A page of an app's own, after the shared ones. `build` makes the page's
// widget when the dialog opens; `apply` runs after OK, once the shared pages
// have been saved, and reads what `build` made.
struct OptionsPage {
    QString title;
    std::function<QWidget *()> build;
    std::function<void()> apply;
};

// Shows the dialog. False when it was cancelled. `grammar`: the app checks grammar as you type (JeffWrite), so the Proofing page has that choice; for an app that does not, it
// would be a box that does nothing.
bool optionsDialog(QWidget *p, Editor *ed, const QVector<OptionsPage> &appPages, bool grammar = false);
// "Grammar settings...", from the Proofing page: which kinds of issue the grammar check looks for (a checkbox for each kind; style is off until it is turned on). False when cancelled;
// otherwise the choice is saved (setGrammarKinds) and every open document is checked again. With `kinds`, the dialog starts from *kinds and puts the choice back there, and saves nothing (the
// Options dialog keeps the choice until its own OK, as it does for its other pages).
bool grammarSettingsDialog(QWidget *parent, quint32 *kinds = nullptr);

} // namespace jp
