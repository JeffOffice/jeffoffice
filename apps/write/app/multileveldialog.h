#pragma once
// Define New Multilevel List: the nine levels of a list definition, each with its number style, the text round its number (which may name the levels above it), where it starts, what restarts it,
// its alignment, number position and text indent, what follows the number, legal style, and a paragraph style it is linked to.

#include "wml/wml.h"

#include <QPair>
#include <QStringList>
#include <QWidget>
#include <functional>

namespace jp::write {

// What the box says of saving the definition as a named list style: `save` and `name` come back as chosen. With `editing` the box is for a list style that exists (its name is shown, and
// kept), and there is nothing to choose. `check` tells why a name can't be used (empty when it can); the box stays open, and says so, until it can.
struct ListStyleChoice {
    bool save = false, editing = false;
    QString name;
    std::function<QString(const QString &)> check;
};

// `styles` are the paragraph styles that a level can be linked to (id and name). False if canceled; `definition` is then as it was.
bool multilevelListDialog(QWidget *parent, office::wml::AbstractNum *definition, const QList<QPair<QString, QString>> &styles, ListStyleChoice *listStyle = nullptr);

} // namespace jp::write
