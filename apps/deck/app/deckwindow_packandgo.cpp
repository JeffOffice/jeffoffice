// Pack and Go: the presentation with its fonts and the movies, sounds, and pictures it links to, in a ZIP or a folder (the part every app shares is libs/shell/app/packandgo).
#include "app/deckwindow.h"
#include "app/editor.h"
#include "app/notespane.h"
#include "app/packandgo.h"
#include "app/privacy.h"
#include "app/settings.h"
#include "io/presentationfile.h"

#include <QApplication>
#include <QFile>
#include <QTemporaryDir>

namespace jp {

PackReport DeckWindow::packAndGoTo(const QString &destination, PackMode mode, const PackAsk &ask, const PackLimits &limits)
{
    PackReport none;
    if (!m_ed->doc() || m_ed->doc()->pages.isEmpty()) {
        none.error = tr("There is no presentation to pack.");
        return none;
    }
    m_notes->commit();
    m_ed->endTextEdit();
    BytesPackSource source(m_ed->displayName() + QStringLiteral(".pptx"), m_ed->filePath(), m_fileProtected ? m_filePassword : QString(), [this](QString *err, QStringList *notes) {
        QByteArray bytes;
        // A folder of its own, so that nothing is left in the shared temporary folder (the password is put on the copy by Pack and Go, after its links are moved).
        QTemporaryDir dir;
        if (!dir.isValid()) return bytes;
        const QString path = dir.filePath(QStringLiteral("presentation.pptx"));
        QStringList skipped, namesProblems;
        if (deck::saveAnyPresentationFile(*m_ed->doc(), path, err, &skipped, Settings::get().value(QStringLiteral("save/embedFonts"), true).toBool(), QString(), privacy::removeOnSave(),
                                          &namesProblems)) {
            QFile f(path);
            if (f.open(QIODevice::ReadOnly)) bytes = f.readAll();
        }
        if (!namesProblems.isEmpty()) notes->push_back(privacy::notTakenOutMessage(namesProblems));
        if (hasMacros()) notes->push_back(tr("The macros in this presentation aren't saved, because a .pptx file can't hold them."));
        return bytes;
    });
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const PackReport report = jp::packAndGo(source, destination, mode, limits, ask);
    QApplication::restoreOverrideCursor();
    return report;
}

bool DeckWindow::packAndGo(bool toFolder)
{
    const QString destination = askPackAndGoDestination(this, m_ed->displayName() + QStringLiteral(".pptx"), toFolder);
    if (destination.isEmpty()) return false;
    return showPackAndGoReport(this, packAndGoTo(destination, toFolder ? PackMode::Folder : PackMode::Zip, packAndGoAsk(this)));
}

} // namespace jp
