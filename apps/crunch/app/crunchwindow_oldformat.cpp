// What the window does for a workbook that came from the older binary file format (.xls): it is saved under a new name, as .xlsx, and the file it came from is never written to.

#include "app/crunchwindow.h"

#include "app/oldformat.h"
#include "app/otherformats.h"
#include "core/loader.h"

#include <QFileDialog>
#include <QFileInfo>

namespace jp {

bool CrunchWindow::savesAsANewFile() const
{
    return m_loaded && m_loaded->book && m_loaded->book->isConverted() && !m_loaded->book->package().isLazy();
}

QString CrunchWindow::pathToSave()
{
    if (m_path.isEmpty() || !m_loaded || !m_loaded->book) return QString();
    if (!savesAsANewFile()) return m_path;
    // (the person is told first, in plain words, and then asked for the name: the file dialog starts with the old name and the ending .xlsx)
    notice(tr("Save"), crunch::saveAsExplanation(QFileInfo(m_path).fileName(), m_loaded->book->convertedKind(), m_loaded->book->skippedFeatures()), false);
    const QString start = crunch::suggestedSaveName(m_path);
    const QString filter = tr("Workbooks (*.xlsx)") + QStringLiteral(";;") + crunch::csvFilter();
    const QString chosen = m_savePicker ? m_savePicker(start, filter) : QFileDialog::getSaveFileName(this, tr("Save Workbook As"), start, filter);
    if (chosen.isEmpty()) return QString();
    // (whatever was answered, it ends as a workbook of the new form does, and it is not the file the workbook was read from)
    // (a name ending in .csv is the values of the sheet in view, written as a .csv: see exportCsv)
    const QString path = crunch::isCsvName(chosen) ? chosen : crunch::withWorkbookEnding(chosen);
    if (QFileInfo(path).absoluteFilePath() == QFileInfo(m_path).absoluteFilePath() || QFileInfo(chosen).absoluteFilePath() == QFileInfo(m_path).absoluteFilePath()) {
        notice(tr("Save"), crunch::originalNotWrittenNotice(QFileInfo(m_path).fileName()));
        return QString();
    }
    return path;
}

} // namespace jp
