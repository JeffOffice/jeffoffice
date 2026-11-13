#pragma once
// The bundled spelling and hyphenation dictionaries, LibreOffice's, one
// folder per language under resources/dict. Text carries its language as a
// BCP-47 tag ("en-US", "pt-BR"); text without one is US English.
// In an install each dictionary file is compressed ("en_US.dic.jz", see
// text/dictpack.h) and inflated in memory when it is first used; the plain
// files of the source tree are read as they are.

#include <QString>
#include <QStringList>
#include <QVector>

namespace jp::dict {

struct Language {
    QString code;   // "es-MX"
    QString name;   // "Spanish (Mexico)"
};

// The languages JeffPub can check and hyphenate, in menu order.
const QVector<Language> &languages();
// The display name of a tag ("pt-BR" -> "Portuguese (Brazil)"), or the tag.
QString languageName(const QString &tag);

// The bundled language for text marked `tag`: the same language and region,
// else that language's usual dictionary ("es-AR" -> "es-MX", "en-NZ" ->
// "en-GB"); empty when the language has none. An empty tag is US English.
QString match(const QString &tag);

// The folder holding the language folders, or empty when it's missing: the data folder's dict (jp::dataFolder), else beside the
// program, in ../share/jeffoffice, or in ../Resources (older installs), else the source tree's resources/dict.
QString root();
// Reads the dictionaries from `dir` (laid out as resources/dict) and not from the folder root() would find; an empty string goes back
// to the search. Forgets every checker and the thesaurus already loaded. For tests.
void setRootOverride(const QString &dir);
// How many languages have their checker kept (at most the languages of the list, whatever tags files carry; for tests).
int loadedCount();

// Spelling, by a code from match(). A language whose dictionary is missing
// accepts every word; hasSpelling() says whether it's there.
bool hasSpelling(const QString &code);
bool spell(const QString &code, const QString &word);
QStringList suggest(const QString &code, const QString &word);

// The language's hyphenation patterns (a LibreOffice hyph_*.dic), or empty. It is the plain file's name even where only the
// compressed file exists: read it with readFile().
QString hyphenationFile(const QString &code);

// A dictionary file's bytes: the compressed file beside `path` (its name and ".jz") inflated, else the plain file. Empty when
// there is neither or the compressed one is damaged, cut short, or inflates to too much (a plain file is never read in its place).
QByteArray readFile(const QString &path);
// Whether the file or its compressed form is there.
bool fileExists(const QString &path);

// The English thesaurus's entry for a word (lowercased, as UTF-8): the lines of the word's entry in the MyThes file ("word|count",
// then "(pos)|synonym|..." for each), or empty when the word has none or there is no thesaurus. It reads one block of the
// compressed file, not the whole thesaurus.
QByteArray thesaurusEntry(const QByteArray &lowerWord);
// Whether the thesaurus is there and can be read.
bool hasThesaurus();

// `path` as Hunspell can open it. It takes 8-bit names, which on Windows are
// in the system's code page, and JeffPub installs under the user's own
// folder; when the user name doesn't fit that code page, the short (8.3)
// form of the path does.
QByteArray hunspellFileName(const QString &path);

} // namespace jp::dict
