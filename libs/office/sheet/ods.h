#pragma once
// An OpenDocument spreadsheet (.ods, and the flat XML form .fods), read as a workbook. The package is a ZIP of parts: content.xml holds the sheets (each a `table:table`) and the
// styles the cells use, styles.xml the named styles and the defaults, settings.xml how each sheet was left on the screen (the cell it was on, frozen panes, the zoom). Everything is
// read as a stream of XML that passed the guard of office/xmlstream.h (UTF-8, no DOCTYPE, no piece larger than a bound, so no entity is ever defined or expanded) and nesting is
// bounded.
//
// Read: the sheets and whether each is hidden, cells of every value type (float, percentage, currency, date, time, boolean, string with its line breaks, spaces, and spans, and
// the error a formula gave), formulas (converted by odsformula.h, with the result the file stored), array formulas, data styles turned into number formats (numbers, percents,
// currency, dates, times, text, fractions, scientific, conditions and colors), cell styles with the named styles they follow (fonts, fills, borders, alignment, wrap, indent,
// protection), merged cells, repeated rows, columns, and cells, column widths and row heights, hidden and filtered rows and columns, frozen panes, the cell the sheet was left
// on, the zoom, the tab's color, and named ranges and expressions (the workbook's and a sheet's).
// Not read, and said so by skipped(): charts, pictures and other drawing objects, hyperlinks, comments, pivot tables, macros, conditional formats, and data validation. Changed, and said
// so by changed(): sheet names the .xlsx form does not allow (too long, a mark it does not allow, the same as another in capitals or small letters), which are made valid and unique with
// the formulas and names that use them, and formulas that could not be converted, which keep the value the file stored. A document that is
// encrypted is refused, as is one that is a text document or a presentation.
//
// Rows and cells that repeat ("number-rows-repeated" of a million) are never made one by one beyond what a sheet holds and the budget of a read: an empty cell that has the look of
// its column, and a run of empty cells that reaches the end of a row, make no cell.

#include "sheet/foreignbook.h"

#include <QByteArray>

#include <memory>

namespace jp::office::sheet {

// Whether the bytes of a ZIP are an OpenDocument package of any kind (a content.xml, a mimetype, and no [Content_Types].xml).
bool looksLikeOpenDocumentZip(const QByteArray &zip);
// Whether the first bytes of a file are the flat XML form of an OpenDocument document.
bool looksLikeFlatOpenDocument(const QByteArray &head);

// The deepest nesting of elements.
constexpr int kMaxOdsDepth = 200;
// Rows and cells a sheet may repeat at the largest: the sheet's own limits (the rows and columns of a sheet), and the work of making cells from a repeat is the cells it makes.

class OdsBook : public ForeignBook {
public:
    // Reads the package's styles, settings, and the sheets' names, and finds where each sheet is in content.xml. False, with a message, if the file is not a spreadsheet, is
    // encrypted, is damaged, or is past a limit.
    static std::shared_ptr<const OdsBook> open(const QByteArray &file, bool flat, const Limits &limits, WorkbookInfo *info, SharedStrings *shared, Styles *styles, QString *error);
    bool readSheet(int index, const Limits &limits, const std::function<bool(qint64)> &progress, qint64 stopAfterRow, qint64 screenRows, Sheet *out) const override;
    QStringList skipped() const override { return m_skipped; }
    QStringList changed() const override;
    QString kind() const override { return m_flat ? QStringLiteral(".fods") : QStringLiteral(".ods"); }
    ~OdsBook() override;

    struct Impl;

private:
    OdsBook();
    bool m_flat = false;
    QStringList m_skipped;
    std::shared_ptr<Impl> m;
};

} // namespace jp::office::sheet
