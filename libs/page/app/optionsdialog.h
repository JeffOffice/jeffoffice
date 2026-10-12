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

// Shows the dialog. False when it was cancelled.
bool optionsDialog(QWidget *p, Editor *ed, const QVector<OptionsPage> &appPages);

} // namespace jp
