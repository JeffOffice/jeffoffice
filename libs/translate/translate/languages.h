#pragma once
// The languages the Translate pane offers, by the code Mozilla's translation models are named with (the two letters of the language; "zh" is Simplified Chinese).

#include <QString>
#include <QStringList>
#include <QVector>

namespace jp {
namespace translate {

struct Language {
    QString code;
    QString name;   // in the language of the program
};

// Every language offered, in the order the pickers show them (English first, then by name in the language of the program).
QVector<Language> languages();
// The name of the language in the language of the program; the code itself for one that is not offered.
QString languageName(const QString &code);
// The code of the language a proofing-language tag names ("en-US" is "en", "pt-BR" is "pt", "zh-CN" is "zh"); empty for one that is not offered.
QString languageOfTag(const QString &tag);
// The code the website path (LibreTranslate) knows the language by.
QString websiteCode(const QString &code);

constexpr const char *kPivot = "en";   // every pair that has no model of its own goes through English

} // namespace translate
} // namespace jp
