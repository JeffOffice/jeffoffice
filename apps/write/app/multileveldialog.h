#pragma once
// Define New Multilevel List: the nine levels of a list definition, each with its number style, the text round its number (which may name the levels above it), where it starts, what restarts it,
// its alignment, number position and text indent, what follows the number, legal style, and a paragraph style it is linked to.

#include "wml/wml.h"

#include <QPair>
#include <QStringList>
#include <QWidget>

namespace jp::write {

// `styles` are the paragraph styles that a level can be linked to (id and name). False if canceled; `definition` is then as it was.
bool multilevelListDialog(QWidget *parent, office::wml::AbstractNum *definition, const QList<QPair<QString, QString>> &styles);

} // namespace jp::write
