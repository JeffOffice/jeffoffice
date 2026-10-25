#include "app/otherformats.h"

#include "app/oldformat.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace jp::crunch {

bool isCsvName(const QString &path) { return QFileInfo(path).suffix().compare(QLatin1String("csv"), Qt::CaseInsensitive) == 0; }

QString openFilter() { return QCoreApplication::translate("jp::crunch::OtherFormats", "Workbooks (*.xlsx *.xlsm *.xls *.ods *.fods *.csv *.tsv *.txt)"); }

QString csvFilter() { return QCoreApplication::translate("jp::crunch::OtherFormats", "Text files of values (*.csv)"); }

namespace {

// The list separator the system keeps for the person, where the system keeps one (Windows has it among the settings of the region; the other systems keep none), or a null character.
QChar systemListSeparator(const QLocale &locale)
{
#ifdef Q_OS_WIN
    wchar_t buffer[16] = {};
    const QString name = locale.name().replace(QLatin1Char('_'), QLatin1Char('-'));
    const bool theirs = locale == QLocale::system();
    const int n = theirs ? GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SLIST, buffer, 16)
                         : GetLocaleInfoEx(reinterpret_cast<LPCWSTR>(name.utf16()), LOCALE_SLIST, buffer, 16);
    // (the sign alone: a separator of more than one character is no separator of a file of values)
    if (n == 2 && (buffer[0] == L',' || buffer[0] == L';')) return QChar(ushort(buffer[0]));
    if (n == 2 && buffer[0] == L'\t') return QLatin1Char('\t');
#else
    Q_UNUSED(locale)
#endif
    return QChar();
}

} // namespace

QChar csvSeparatorFor(const QLocale &locale)
{
    const QChar system = systemListSeparator(locale);
    if (!system.isNull()) return system;
    return locale.decimalPoint() == QLatin1String(",") ? QLatin1Char(';') : QLatin1Char(',');
}

QString saveAsExplanation(const QString &fileName, const QString &kind, const QStringList &notRead, const QStringList &changed)
{
    const QString k = kind.toLower();
    if (k == QLatin1String(".xls") || k.isEmpty()) return saveAsExplanation(fileName, notRead);
    QString text;
    if (k == QLatin1String(".ods") || k == QLatin1String(".fods")) {
        text = QCoreApplication::translate("jp::crunch::OtherFormats", "\"%1\" is an OpenDocument spreadsheet (%2), and this program saves workbooks only as .xlsx. It will ask you for a new name to save a copy under. "
                  "The original file will not be changed.")
                   .arg(fileName, k);
    } else {
        text = QCoreApplication::translate("jp::crunch::OtherFormats", "\"%1\" is a text file of values (%2), and this program saves workbooks as .xlsx, which keeps the formulas, the formats, and more than one sheet. It will ask you for a "
                  "new name to save a copy under. The original file will not be changed. To write only the values of this sheet as a .csv file, choose that kind of file where you give the name.")
                   .arg(fileName, k);
    }
    if (!notRead.isEmpty()) text += QLatin1Char(' ') + QCoreApplication::translate("jp::crunch::OtherFormats", "These parts of the file will not be in the copy: %1.").arg(notRead.join(QStringLiteral(", ")));
    if (!changed.isEmpty()) text += QLatin1Char(' ') + QCoreApplication::translate("jp::crunch::OtherFormats", "These are changed in the copy: %1.").arg(changed.join(QStringLiteral(", ")));
    return text;
}

QString csvExportNote(const QString &sheetName, QChar separator, bool partial)
{
    QString text = QCoreApplication::translate("jp::crunch::OtherFormats",
                                               "Only the values of the sheet \"%1\" are saved in a .csv file, as the cells show them, with \"%2\" between them. The other sheets, the formulas, the formats, and the settings are not. "
                                               "Save the workbook as .xlsx to keep them.")
                       .arg(sheetName, QString(separator));
    if (partial)
        text += QLatin1Char(' ') + QCoreApplication::translate("jp::crunch::OtherFormats", "This sheet was read only in part, so the file has only the part that was read.");
    return text;
}

namespace {

// Where a path really is: the file as it is (links followed), or for a file that is not there yet, its folder as it is with the file's name.
QString placeOf(const QString &path)
{
    const QFileInfo info(path);
    const QString real = info.canonicalFilePath();
    if (!real.isEmpty()) return real;
    const QString folder = QFileInfo(info.absolutePath()).canonicalFilePath();
    return (folder.isEmpty() ? QDir::cleanPath(info.absolutePath()) : folder) + QLatin1Char('/') + info.fileName();
}

} // namespace

bool sameFile(const QString &a, const QString &b)
{
    if (a.isEmpty() || b.isEmpty()) return false;
    // (the names of files are the same in capitals and small letters on the systems whose files are)
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    constexpr Qt::CaseSensitivity kCase = Qt::CaseInsensitive;
#else
    constexpr Qt::CaseSensitivity kCase = Qt::CaseSensitive;
#endif
    return QFileInfo(a) == QFileInfo(b) || placeOf(a).compare(placeOf(b), kCase) == 0;
}

QString withPickedEnding(const QString &chosen, const QString &pickedFilter)
{
    if (chosen.isEmpty() || pickedFilter != csvFilter()) return chosen;
    return QFileInfo(chosen).suffix().isEmpty() ? chosen + QLatin1String(".csv") : chosen;
}

} // namespace jp::crunch
