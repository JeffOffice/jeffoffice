#include "app/oldformat.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

namespace jp::crunch {

namespace {
QString tr(const char *text) { return QCoreApplication::translate("jp::crunch::OldFormat", text); }
} // namespace

bool isOlderFormatName(const QString &path) { return QFileInfo(path).suffix().compare(QLatin1String("xls"), Qt::CaseInsensitive) == 0; }

QString suggestedSaveName(const QString &openedPath)
{
    const QFileInfo info(openedPath);
    const QString folder = info.absolutePath();
    const QString base = info.completeBaseName();
    QString name = QDir(folder).filePath(base + QLatin1String(".xlsx"));
    for (int n = 2; n < 1000 && (QFileInfo::exists(name) || QDir::cleanPath(name) == QDir::cleanPath(info.absoluteFilePath())); ++n)
        name = QDir(folder).filePath(QStringLiteral("%1 (%2).xlsx").arg(base).arg(n));
    return name;
}

QString withWorkbookEnding(const QString &chosen)
{
    if (chosen.isEmpty()) return chosen;
    const QFileInfo info(chosen);
    const QString suffix = info.suffix().toLower();
    if (suffix == QLatin1String("xlsx") || suffix == QLatin1String("xlsm")) return chosen;
    if (suffix == QLatin1String("xls")) return chosen + QLatin1Char('x');
    return chosen + QLatin1String(".xlsx");
}

QString saveAsExplanation(const QString &fileName)
{
    return tr("\"%1\" is in an older file format (.xls), and this program saves workbooks only as .xlsx. It will ask you for a new name to save a copy under. "
              "The original file will not be changed.").arg(fileName);
}

QString notReadNotice(const QStringList &notRead)
{
    if (notRead.isEmpty()) return QString();
    return tr("This workbook has parts this program doesn't read yet: %1. They are not shown, and a copy saved from here will not have them.").arg(notRead.join(QStringLiteral(", ")));
}

QString askForSaveName(QWidget *parent, const QString &openedPath)
{
    QMessageBox box(QMessageBox::Information, tr("Save As"), saveAsExplanation(QFileInfo(openedPath).fileName()), QMessageBox::Ok | QMessageBox::Cancel, parent);
    if (box.exec() != QMessageBox::Ok) return QString();
    const QString chosen = QFileDialog::getSaveFileName(parent, tr("Save Workbook As"), suggestedSaveName(openedPath), tr("Workbooks (*.xlsx)"));
    return withWorkbookEnding(chosen);
}

} // namespace jp::crunch
