// What the window does for a workbook that came from the older binary file format (.xls): it is saved under a new name, as .xlsx.

#include "app/crunchwindow.h"

#include "app/oldformat.h"
#include "core/loader.h"

#include <QFileInfo>

namespace jp {

QString CrunchWindow::pathToSave()
{
    if (m_path.isEmpty() || !m_loaded || !m_loaded->book) return QString();
    if (!m_loaded->book->isBinary() && !crunch::isOlderFormatName(m_path)) return m_path;
    const QString name = m_saveNameAsker ? m_saveNameAsker(m_path) : crunch::askForSaveName(this, m_path);
    // (whatever was answered, it ends as a workbook of the new form does, and is not the file the workbook was read from)
    const QString path = crunch::withWorkbookEnding(name);
    if (path.isEmpty() || QFileInfo(path).absoluteFilePath() == QFileInfo(m_path).absoluteFilePath()) return QString();
    return path;
}

} // namespace jp
