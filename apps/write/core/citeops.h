#pragma once
// Citations and bibliographies: the sources a document cites (a custom XML part in the file's own bibliography format, kept beside the document's other parts), the CITATION field that
// shows a citation in the style the sources ask for (APA, MLA, Chicago, Harvard, or IEEE), and the BIBLIOGRAPHY field whose result is the list of the sources cited, worked out as the
// other tables are (core/fieldops.h calls the functions that make the text). The master list that documents share is a file in the program's data folder.

#include "core/fieldcode.h"
#include "core/fieldops.h"
#include "core/tocops.h"

#include <QMap>

namespace jp::write {

constexpr int kMostSources = 5000;

struct Person {
    QString last, first, middle;
};

struct Source {
    QString tag;                       // the name a citation gives it ("Smi20")
    QString type = QStringLiteral("Book");
    QVector<Person> authors;
    QString corporateAuthor;           // an author that is no person (used where there are no people)
    QMap<QString, QString> fields;     // the other single values, by the element names of the format: Title, Year, Publisher, City, JournalName, Pages, Volume, Issue, URL...
    QByteArray extra;                  // the elements this program does not edit (editors, translators...), as written, kept as they were
    QByteArray authorExtra;            // the same inside the element of the authors (editors, translators...)
    bool operator==(const Source &o) const { return tag == o.tag && type == o.type && corporateAuthor == o.corporateAuthor && fields == o.fields && extra == o.extra && authorExtra == o.authorExtra && authorsSame(o); }
    bool authorsSame(const Source &o) const
    {
        if (authors.size() != o.authors.size()) return false;
        for (int i = 0; i < authors.size(); ++i)
            if (authors[i].last != o.authors[i].last || authors[i].first != o.authors[i].first || authors[i].middle != o.authors[i].middle) return false;
        return true;
    }
};

// The kinds of source the format has, as it names them ("Book", "JournalArticle"...) and as they are shown.
QStringList sourceTypes();
QString sourceTypeLabel(const QString &type);
// The single values each kind asks for (the element names, in the order they are asked).
QStringList sourceFieldsFor(const QString &type);
QString sourceFieldLabel(const QString &field);

// The styles: the names the file keeps ("APA", "MLA", "Chicago", "Harvard", "IEEE"), and the one a part without a style has.
QStringList citationStyles();
QString defaultCitationStyle();

// Reads and writes the part's XML. The style is the one the part names (empty: none). Reading is held to the limits of a part (kMostSources sources).
QVector<Source> parseSources(const QByteArray &xml, QString *style = nullptr, bool *truncated = nullptr);
QByteArray sourcesToXml(const QVector<Source> &sources, const QString &style);

// The document's sources and style, and setting them (written with the file).
QVector<Source> documentSources(WriteDoc &doc);
QString documentCitationStyle(WriteDoc &doc);
void setDocumentSources(WriteDoc &doc, const QVector<Source> &sources);
// Changes the style the sources are shown in, and makes the citations and the bibliographies of the text again, as one step.
void setDocumentCitationStyle(WriteDoc &doc, const QString &style);

// The master list: the sources that every document of the person can take from (a file in the program's data folder; `path` empty: that file).
QString masterSourcesPath();
QVector<Source> masterSources(const QString &path = QString());
bool setMasterSources(const QVector<Source> &sources, const QString &path = QString());

// A tag for a new source that none of `taken` has: the first letters of the first author's last name (or of the title), and the last two digits of the year ("Smi20").
QString newSourceTag(const Source &source, const QVector<Source> &taken);
// What a source shows in the lists: the authors, the year, and the title.
QString sourceSummary(const Source &source);

// The text a citation of the source with this code shows in this style (the code's switches: \p pages, \n without the author, \y without the year, \t with the title). `number` is the place
// the source has in the order of citation (for the numbered styles).
QString citationText(const Source &source, const QString &style, const fields::Code &code, int number);
// What a CITATION field with this tag shows in the document (nothing where there is no source by the tag), for the field engine.
std::optional<QString> citationFieldText(WriteDoc &doc, const StoryScan &scan, const QString &tag, const fields::Code &code);
// The paragraphs of the bibliography: the sources the text cites, in the style, each in the style "Bibliography".
QVector<GenLine> bibliographyLines(WriteDoc &doc, const StoryScan &scan, int field, const fields::Code &code);
QString noBibliography();
// The entry of one source in the bibliography, as pieces (the titles of books and journals are italic).
QVector<GenRun> bibliographyEntry(const Source &source, const QString &style, int number);

// Puts a citation of the source at the caret (after the selection), as one step. `pages` is the page or pages the citation names. False where it can't be put or there is no such source.
struct CitationOptions {
    QString pages;
    bool suppressAuthor = false, suppressYear = false, suppressTitle = true;
};
bool insertCitation(WriteDoc &doc, QTextCursor &c, const QString &tag, const CitationOptions &options = CitationOptions());
// How the bibliography gallery heads its list.
enum class BibliographyKind { Bibliography, References, WorksCited };
// Puts the heading and a bibliography at the caret, as one step (the heading in the style Heading 1: "Bibliography", "References", or "Works Cited"). False where it can't be put.
bool insertBibliography(WriteDoc &doc, QTextCursor &c, BibliographyKind kind);

} // namespace jp::write
