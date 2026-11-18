// Pack and Go: the document with its fonts and the files it links to, in a ZIP or a folder (the part every app shares is libs/shell/app/packandgo).
#include "app/writewindow.h"
#include "app/packandgo.h"
#include "app/privacy.h"
#include "core/writedoc.h"
#include "office/officecrypt.h"
#include "office/privacy.h"

#include <QApplication>

namespace jp {

PackReport WriteWindow::packAndGoTo(const QString &destination, PackMode mode, const PackAsk &ask, const PackLimits &limits)
{
    PackReport none;
    if (!m_doc || !commitShapeText()) {
        none.error = tr("The document couldn't be saved for Pack and Go.");
        return none;
    }
    BytesPackSource source(displayName() + QStringLiteral(".docx"), m_path, m_fileProtected ? m_filePassword : QString(), [this](QString *err, QStringList *notes) {
        QByteArray bytes = m_doc->toDocx(err);
        // (Options > Save: the names stay out of a file that is sent away)
        QStringList problems;
        if (privacy::removeOnSave() && !bytes.isEmpty()) bytes = office::removePersonalFromPackage(bytes, nullptr, &problems);
        if (!problems.isEmpty()) notes->push_back(privacy::notTakenOutMessage(problems));
        return bytes;
    });
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const PackReport report = jp::packAndGo(source, destination, mode, limits, ask);
    QApplication::restoreOverrideCursor();
    return report;
}

bool WriteWindow::packAndGo(bool toFolder)
{
    const QString destination = askPackAndGoDestination(this, displayName() + QStringLiteral(".docx"), toFolder);
    if (destination.isEmpty()) return false;
    return showPackAndGoReport(this, packAndGoTo(destination, toFolder ? PackMode::Folder : PackMode::Zip, packAndGoAsk(this)));
}

} // namespace jp
