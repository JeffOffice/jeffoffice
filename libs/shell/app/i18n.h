#pragma once
// Translations of the program's own text: the apps' (one set for the suite,
// in i18n/jeffoffice_<language>.qm of the data folder) and Qt's for its standard
// dialogs.

#include <QString>
#include <QStringList>

namespace jp {

// The languages the apps have a translation for (BCP 47 names, "de", "pt_BR"),
// besides American English, which its text is written in. The English variants
// ("en_GB", "en_CA", "en_AU") are in it: they hold only the texts they spell
// differently, and show the American ones for the rest. So does Spanish for
// Spain ("es_ES") over the neutral Spanish ("es").
QStringList availableUiLanguages();

// Languages with a translation file but not translated yet: not offered in Options and not picked from the system's languages, until they
// are taken off this list (kept in step with HELD in tools/translate/translate.py, which a test checks). A test may set it.
QStringList heldUiLanguages();
void setHeldUiLanguages(const QStringList &languages);

// A language's name as its speakers write it, with the country when the
// language comes in more than one ("Português (Brasil)").
QString uiLanguageName(const QString &language);

// The language to use from the ones a person prefers, best first ("de_DE", "en_US"),
// and the ones the apps have; empty for American English. A region the apps have no
// file for but that writes its English as one of theirs ("en_NZ") gets that one.
QString pickUiLanguage(const QStringList &wanted, const QStringList &have);

// Loads the translation for the language chosen in Options (ui/language),
// or the system's first language the apps have, at startup. Returns the
// language in use ("en" when none applies).
QString installTranslations();

// The language installTranslations() chose ("en" until it has run).
QString currentUiLanguage();

} // namespace jp
