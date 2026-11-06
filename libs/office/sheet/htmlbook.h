#pragma once
// A web page read as a workbook: each table of the page is a sheet (Table1, Table2, ...), its cells read as a person would type them into a cell (a number, a date, a percent, or text, as
// a file of delimited values is read). Nothing the page names is fetched, and nothing is run. A page with no table is not a workbook. The tables are found by the reader of web pages the
// word processor uses (wml/wmlhtmlread.h); each is handed to the delimited-values reader as tab-separated text, so a cell is read in the same way in both.

#include "sheet/csv.h"

namespace jp::office::sheet {

// Whether the name ends as a web page does (.html, .htm, .xhtml; any case).
bool isHtmlName(const QString &path);

class HtmlBook : public ForeignBook {
public:
    // Reads the page's tables and makes the sheets' names and the formats a typed entry asks for. False, with a message, if there is no table to read.
    static std::shared_ptr<const HtmlBook> open(const QByteArray &page, const Limits &limits, const QLocale &locale, WorkbookInfo *info, SharedStrings *shared, Styles *styles, QString *error);
    bool readSheet(int index, const Limits &limits, const std::function<bool(qint64)> &progress, qint64 stopAfterRow, qint64 screenRows, Sheet *out) const override;
    QStringList skipped() const override { return m_skipped; }
    QString kind() const override { return QStringLiteral(".html"); }
    int tableCount() const { return int(m_tables.size()); }

private:
    HtmlBook() = default;
    std::vector<std::shared_ptr<const CsvBook>> m_tables;   // each table as the tab-separated text it was made into
    QStringList m_skipped;
};

} // namespace jp::office::sheet
