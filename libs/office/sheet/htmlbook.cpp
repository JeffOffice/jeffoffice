#include "sheet/htmlbook.h"

#include "wml/wmlhtmlread.h"

#include <QCoreApplication>

namespace jp::office::sheet {

bool isHtmlName(const QString &path)
{
    const QString lower = path.toLower();
    return lower.endsWith(QLatin1String(".html")) || lower.endsWith(QLatin1String(".htm")) || lower.endsWith(QLatin1String(".xhtml"));
}

namespace {

// A table's rows as tab-separated text that the reader of delimited values takes back as the same cells: every cell is quoted, a quote is written twice, and a cell that would be read as a formula
// is held as text.
QByteArray tabSeparated(const wml::HtmlTable &table)
{
    QString out;
    for (const QStringList &row : table.rows) {
        for (int c = 0; c < row.size(); ++c) {
            if (c) out += QLatin1Char('\t');
            QString text = row[c];
            if (text.startsWith(QLatin1Char('='))) text.prepend(QLatin1Char('\''));
            if (text.isEmpty()) continue;
            text.replace(QLatin1Char('"'), QStringLiteral("\"\""));
            out += QLatin1Char('"') + text + QLatin1Char('"');
        }
        out += QLatin1Char('\n');
    }
    return out.toUtf8();
}

} // namespace

std::shared_ptr<const HtmlBook> HtmlBook::open(const QByteArray &page, const Limits &limits, const QLocale &locale, WorkbookInfo *info, SharedStrings *shared, Styles *styles, QString *error)
{
    bool tooMany = false;
    QString why;
    const QVector<wml::HtmlTable> tables = wml::readHtmlTables(page, &tooMany, &why);
    if (tables.isEmpty()) {
        if (error) *error = !why.isEmpty() ? why : QCoreApplication::translate("jp::office::sheet::Html", "This web page has no table in it, so there is nothing to open as a workbook.");
        return nullptr;
    }
    std::shared_ptr<HtmlBook> book(new HtmlBook);
    // (the tables are read as tab-separated, in UTF-8: whatever the page's own encoding was, it is text by now)
    Limits mine = limits;
    auto choices = std::make_shared<CsvImport>();
    choices->separators = QStringLiteral("\t");
    choices->encodingSet = true;
    choices->encoding = CsvEncoding::Utf8;
    mine.csv = choices;
    WorkbookInfo bookInfo;
    for (int i = 0; i < tables.size(); ++i) {
        // (the first table's reader fills the workbook's strings and formats; the others make the same ones, which are put aside)
        WorkbookInfo extraInfo;
        SharedStrings extraShared;
        Styles extraStyles;
        QString problem;
        const auto csv = CsvBook::open(tabSeparated(tables[i]), QStringLiteral("table.tsv"), mine, locale, i == 0 ? &bookInfo : &extraInfo, i == 0 ? shared : &extraShared, i == 0 ? styles : &extraStyles, &problem);
        if (!csv) {
            if (error) *error = problem;
            return nullptr;
        }
        book->m_tables.push_back(csv);
    }
    *info = bookInfo;
    info->sheets.clear();
    for (int i = 0; i < tables.size(); ++i) {
        SheetEntry sheet;
        sheet.name = QStringLiteral("Table%1").arg(i + 1);
        sheet.sheetId = i + 1;
        info->sheets.append(sheet);
    }
    book->m_skipped << QCoreApplication::translate("jp::office::sheet::Html", "the page's text outside the tables, its pictures, and its looks");
    if (tooMany) book->m_skipped << QCoreApplication::translate("jp::office::sheet::Html", "the tables after the first %1").arg(wml::kMostHtmlTables);
    return book;
}

bool HtmlBook::readSheet(int index, const Limits &limits, const std::function<bool(qint64)> &progress, qint64 stopAfterRow, qint64 screenRows, Sheet *out) const
{
    if (index < 0 || index >= int(m_tables.size())) {
        *out = Sheet();
        out->error = QCoreApplication::translate("jp::office::sheet::Html", "There is no such sheet.");
        return false;
    }
    const bool ok = m_tables[size_t(index)]->readSheet(0, limits, progress, stopAfterRow, screenRows, out);
    out->view.tabSelected = index == 0;
    return ok;
}

bool Workbook::openHtml(const QByteArray &file, const QString &path, QString *error, const Limits &limits)
{
    Q_UNUSED(path)
    *this = Workbook();
    m_limits = limits;
    const auto book = HtmlBook::open(file, limits, QLocale(), &m_info, &m_shared, &m_styles, error);
    if (!book) {
        *this = Workbook();
        return false;
    }
    m_foreign = book;
    nameBaseParts();
    return true;
}

} // namespace jp::office::sheet
