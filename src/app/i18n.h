#pragma once
// Translations of the program's own text: JeffPub's (built in under
// :/i18n/jeffpub_<language>.qm) and Qt's for its standard dialogs.

#include <QString>
#include <QStringList>

namespace jp {

// The languages JeffPub has a translation for (BCP 47 names, "de", "pt_BR"),
// besides English, which its text is written in.
QStringList availableUiLanguages();

// A language's name as its speakers write it, with the country when the
// language comes in more than one ("Português (Brasil)").
QString uiLanguageName(const QString &language);

// Loads the translation for the language chosen in Options (ui/language),
// or the system's first language JeffPub has, at startup. Returns the
// language in use ("en" when none applies).
QString installTranslations();

// The language installTranslations() chose ("en" until it has run).
QString currentUiLanguage();

} // namespace jp
