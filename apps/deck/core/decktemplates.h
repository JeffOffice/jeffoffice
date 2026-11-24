#pragma once
// The presentations JeffDeck has ready for the New page: a pitch, a status update, a lesson, a photo album, and two minimal looks. Each is made here, in code (a theme of its own: colors,
// fonts, a slide master and layouts, then three to five sample slides with prompts to write over); nothing is read from outside the program.

#include "core/deck.h"

#include <QString>
#include <QVector>

#include <memory>

namespace jp::deck {

struct BuiltInTemplate {
    QString id;            // never shown, never translated: how the template is asked for
    QString name;          // as shown
    QString description;   // one sentence, as the tile's tooltip
};

// Every template, in the order the New page shows them.
QVector<BuiltInTemplate> builtInTemplates();

// A new presentation made from the template; null for an id that names none.
std::unique_ptr<Document> builtInTemplateDocument(const QString &id);

} // namespace jp::deck
