#pragma once
// OpenDocument text (.odt, and the flat XML form .fodt), turned into the package of a .docx and back, so that the word processor's one document model reads and writes both.
//
// Read: toDocxPackage() makes the parts of a .docx from the package (content.xml, styles.xml with its master pages, and meta.xml): paragraphs, headings, spans, the paragraph and
// text properties of automatic and named styles (a style's parent is the style it is based on), lists and their numbering, tables (merged cells, column widths, borders,
// backgrounds, nested tables), sections and columns, page layouts, headers and footers (first, left, and right), footnotes and endnotes, fields (page number, page count, title,
// author, file name, cross-references, sequences, bookmarks, and the table of contents), pictures and text boxes, tabs, line and page breaks, links, and comments. Changes a person
// tracked are kept as the text stands now. A soft page break is ignored.
// Write: fromDocxPackage() makes the package of an .odt from a .docx package: mimetype first and stored, META-INF/manifest.xml, content.xml, styles.xml, meta.xml, and Pictures/.
//
// What a file may hold is limited as the other readers' are: the package's size and part count (office/package.h), what one part may hold and how deep it nests (wml/xmltree.h), a style's
// chain of parents, a repeat count of a column, row, or cell, and the number of the tables', lists', and pictures' parts one document may make.

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace jp::office::odt {

// Whether the bytes are an OpenDocument text document: the ZIP form whose mimetype names a text document or its template, or the flat XML form.
bool looksLikeOdt(const QByteArray &bytes);
// Whether the bytes begin the flat XML form of an OpenDocument text document.
bool looksLikeFlatOdt(const QByteArray &head);

// The .docx package that an OpenDocument text is; empty, with the reason, if the file is not one that can be read. `notes` get what was left out.
QByteArray toDocxPackage(const QByteArray &file, QString *error = nullptr, QStringList *notes = nullptr);
// The .odt package that a .docx package is; empty, with the reason, if it can't be made. `notes` get what the format has no place for.
QByteArray fromDocxPackage(const QByteArray &docx, QString *error = nullptr, QStringList *notes = nullptr);

// The most parts of one kind (lists, tables' styles, pictures, notes) a document makes, so a file cannot multiply them.
constexpr int kMaxMadeParts = 20000;
// The deepest nesting of tables and lists, and of styles' parents.
constexpr int kMaxNesting = 60;
constexpr int kMaxStyleChain = 32;
// The most columns a table has, and the most rows and cells one repeat count makes.
constexpr int kMaxColumns = 1000;
constexpr int kMaxRepeat = 1000;
constexpr qint64 kMaxTableCells = 200000;

} // namespace jp::office::odt
