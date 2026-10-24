#pragma once
// A text file of delimited values (.csv, .tsv, .txt), read as a workbook of one sheet. The file is taken as it is found:
//  - its encoding is UTF-8 (with or without the byte order mark) or UTF-16 (with the mark, either byte order); a file that is not valid UTF-8 is in the code page of the person's
//    language (Windows-1252 unless the language is one with its own: Japanese, Chinese, Korean, the Cyrillic, Greek, Turkish, Hebrew, Arabic, Baltic, Vietnamese, Thai, and
//    Central European ones);
//  - its delimiter is the comma, semicolon, tab, or vertical bar that splits the first lines into the same number of fields most often (a first line `sep=;` says it);
//  - fields in quotes may hold the delimiter, line breaks, and quotes (a quote written twice); a quote that is never closed makes the rest of the file one field;
//  - each field is read as a person would type it into a cell, in the person's locale (read through readEntry, entry.h): a number (1.234,5 in German), a percent, an amount of money,
//    a date or time, TRUE or FALSE, an error; a field that begins with "=" is a formula; anything else is text, as is a field that begins with an apostrophe (kept as it is).
//    Two choices keep what a number would lose: a field of digits only that begins with a zero (a postal code, an identifier: 007) and one of more than 15 digits (a card or an
//    account number) stay text, which the common spreadsheet program turns into a number and so drops the zeros and the digits after the 15th.
// The sheet limits are enforced: what is past the last row or column of a sheet, a field longer than a cell holds, is not read, and the sheet says so (Sheet::error).
// The file is read in pieces as it is scanned, so a file far larger than the memory its cells take is read in the memory its cells take, and the first rows can be had at once.

#include "sheet/foreignbook.h"

#include <QByteArray>
#include <QHash>
#include <QLocale>
#include <QSet>
#include <QString>

#include <memory>

namespace jp::office::sheet {

enum class CsvEncoding { Utf8, Utf8Bom, Utf16Le, Utf16Be, Legacy };

// What the first bytes of a file say about its form.
struct CsvForm {
    CsvEncoding encoding = CsvEncoding::Utf8;
    QChar delimiter = QLatin1Char(',');
    bool sepLine = false;      // the first line is `sep=x`: it names the delimiter and holds no data
    int codePage = 1252;       // for Legacy
};

// Whether the file name ends as a file of delimited values does (.csv, .tsv, .txt, any case).
bool isDelimitedName(const QString &path);

// The form of a file from its bytes (as much as is needed) and its name (.tsv says tab when the lines don't).
CsvForm detectCsvForm(const QByteArray &bytes, const QString &fileName, const QLocale &locale);

// The code page of the person's language, for a file that is not Unicode.
int legacyCodePageFor(const QLocale &locale);

class CsvBook : public ForeignBook {
public:
    // The most characters of one field (a cell holds 32,767).
    static constexpr int kMaxFieldChars = 32767;
    // Reads the file's form and makes the sheet's name and the formats the typed entry asks for. False, with a message, if the file is empty or holds no text this can read.
    static std::shared_ptr<const CsvBook> open(const QByteArray &file, const QString &fileName, const Limits &limits, const QLocale &locale, WorkbookInfo *info,
                                               SharedStrings *shared, Styles *styles, QString *error);
    bool readSheet(int index, const Limits &limits, const std::function<bool(qint64)> &progress, qint64 stopAfterRow, qint64 screenRows, Sheet *out) const override;
    QStringList skipped() const override { return QStringList(); }
    QString kind() const override { return QStringLiteral(".csv"); }
    const CsvForm &form() const { return m_form; }

private:
    CsvBook() = default;
    QByteArray m_data;
    CsvForm m_form;
    QLocale m_locale;
    qint64 m_skip = 0;                  // bytes before the data: the byte order mark and the `sep=` line
    QHash<QString, quint32> m_formats;  // the cell format of each number format code a typed entry asks for
    QSet<QString> m_words;              // the words a number, a date, or a time may have in it
};

} // namespace jp::office::sheet
