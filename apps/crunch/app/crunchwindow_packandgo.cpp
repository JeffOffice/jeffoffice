// Pack and Go: the workbook with its fonts and the pictures and workbooks it links to, in a ZIP or a folder (the part every app shares is libs/shell/app/packandgo).
#include "app/crunchwindow.h"
#include "app/packandgo.h"
#include "app/privacy.h"
#include "core/sheetobjects.h"
#include "office/officecrypt.h"
#include "office/privacy.h"
#include "sheet/xlsxsave.h"

#include <QApplication>
#include <QFileInfo>

namespace jp {

PackReport CrunchWindow::packAndGoTo(const QString &destination, PackMode mode, const PackAsk &ask, const PackLimits &limits)
{
    PackReport none;
    if (!m_session || !m_loaded || !m_loaded->book || m_saving || !commitEdit()) {
        none.error = tr("There is no workbook to pack.");
        return none;
    }
    if (m_loading || !m_haveCalc) {
        none.error = tr("The workbook is still being read. Pack it when the status bar says Ready.");
        return none;
    }
    const QString name = m_path.isEmpty() ? displayName() : QFileInfo(m_path).completeBaseName();
    const QString password = m_fileProtected ? (m_filePassword.isEmpty() ? office::defaultPassword() : m_filePassword) : QString();
    BytesPackSource source(name + QStringLiteral(".xlsx"), m_path, password, [this](QString *err, QStringList *notes) {
        QByteArray bytes;
        syncPanes();
        crunch::EditSession::Snapshot snapshot;
        if (m_session->takeSnapshot(&snapshot, err, false)) {
            if (m_objects) m_objects->addToSave(&snapshot.changes);
            office::sheet::SaveReport report;
            if (office::sheet::saveXlsx(*m_loaded->book, snapshot.changes, &bytes, err, &report)) {
                QStringList problems;
                if (privacy::removeOnSave()) bytes = office::removePersonalFromPackage(bytes, nullptr, &problems);
                if (!problems.isEmpty()) notes->push_back(privacy::notTakenOutMessage(problems));
            } else {
                bytes.clear();
            }
        }
        return bytes;
    });
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const PackReport report = jp::packAndGo(source, destination, mode, limits, ask);
    QApplication::restoreOverrideCursor();
    return report;
}

bool CrunchWindow::packAndGo(bool toFolder)
{
    const QString name = (m_path.isEmpty() ? displayName() : QFileInfo(m_path).completeBaseName()) + QStringLiteral(".xlsx");
    const QString destination = askPackAndGoDestination(this, name, toFolder);
    if (destination.isEmpty()) return false;
    return showPackAndGoReport(this, packAndGoTo(destination, toFolder ? PackMode::Folder : PackMode::Zip, packAndGoAsk(this)));
}

} // namespace jp
