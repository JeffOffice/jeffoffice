#pragma once
// What AutoCorrect does while typing, as the person set it: the switches and the lists of a language, kept in the app's settings and read for the text rules in
// text/autocorrect.h. Shared by the programs, so JeffWrite, JeffPub, and JeffDeck all follow the same options.

#include "core/export.h"
#include "text/autocorrect.h"

#include <QString>

namespace jp {

// The settings' AutoCorrect configuration for a language: the switches (proof/ac/...) and the three lists of the language's first part (proof/ac/replace/fr), each the
// program's own list until the person changes it.
JO_COMMON_EXPORT autotext::Config autoCorrectConfig(const QString &language);
// Keeps the lists (and the switches) of `config`; a list that is the program's own is not kept, so that a better one in a later version reaches those who never changed it.
JO_COMMON_EXPORT void saveAutoCorrectConfig(const autotext::Config &config);

} // namespace jp
