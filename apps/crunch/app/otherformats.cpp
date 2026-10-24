#include "app/otherformats.h"

#include "app/oldformat.h"

#include <QCoreApplication>
#include <QFileInfo>

namespace jp::crunch {

namespace {
QString tr(const char *text) { return QCoreApplication::translate("jp::crunch::OtherFormats", text); }
} // namespace

bool isCsvName(const QString &path) { return QFileInfo(path).suffix().compare(QLatin1String("csv"), Qt::CaseInsensitive) == 0; }

QString openFilter() { return tr("Workbooks (*.xlsx *.xlsm *.xls *.ods *.fods *.csv *.tsv *.txt)"); }

QString csvFilter() { return tr("Text files of values (*.csv)"); }

QChar csvSeparatorFor(const QLocale &locale) { return locale.decimalPoint() == QLatin1String(",") ? QLatin1Char(';') : QLatin1Char(','); }

QString saveAsExplanation(const QString &fileName, const QString &kind, const QStringList &notRead)
{
    const QString k = kind.toLower();
    if (k == QLatin1String(".xls") || k.isEmpty()) return saveAsExplanation(fileName, notRead);
    QString text;
    if (k == QLatin1String(".ods") || k == QLatin1String(".fods")) {
        text = tr("\"%1\" is an OpenDocument spreadsheet (%2), and this program saves workbooks only as .xlsx. It will ask you for a new name to save a copy under. "
                  "The original file will not be changed.")
                   .arg(fileName, k);
    } else {
        text = tr("\"%1\" is a text file of values (%2), and this program saves workbooks as .xlsx, which keeps the formulas, the formats, and more than one sheet. It will ask you for a "
                  "new name to save a copy under. The original file will not be changed. To write only the values of this sheet as a .csv file, choose that kind of file where you give the name.")
                   .arg(fileName, k);
    }
    if (!notRead.isEmpty()) text += QLatin1Char(' ') + tr("These parts of the file will not be in the copy: %1.").arg(notRead.join(QStringLiteral(", ")));
    return text;
}

QString csvExportNote(const QString &sheetName, QChar separator)
{
    return tr("Only the values of the sheet \"%1\" are saved in a .csv file, as the cells show them, with \"%2\" between them. The other sheets, the formulas, the formats, and the settings are not. "
              "Save the workbook as .xlsx to keep them.")
        .arg(sheetName, QString(separator));
}

} // namespace jp::crunch
