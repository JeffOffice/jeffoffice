// Opening presentation files in JeffDeck's window.

#include "app/deckcanvas.h"
#include "app/deckwindow.h"

#include "app/editor.h"
#include "app/notespane.h"
#include "app/outlineview.h"
#include "app/settings.h"
#include "canvas/canvas.h"
#include "core/appinfo.h"
#include "io/presentationfile.h"

#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>

namespace jp {

bool DeckWindow::openFile(const QString &path)
{
    QString notes;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    std::unique_ptr<Document> doc = deck::importAnyPresentationFile(path, &notes);
    QApplication::restoreOverrideCursor();
    const QString name = QFileInfo(path).fileName();
    if (!doc) {
        QMessageBox::warning(this, tr("Open"), tr("%1 can't open \"%2\".\n%3").arg(appInfo().name, name, notes));
        return false;
    }
    // A new presentation nobody has touched gives way; any other stays.
    m_notes->commit();
    if (m_outline) m_outline->commit();
    DeckWindow *target = this;
    if (m_ed->isModified() || !m_ed->filePath().isEmpty()) {
        target = new DeckWindow();
        target->setAttribute(Qt::WA_DeleteOnClose);
        target->show();
    }
    target->m_ed->endTextEdit();
    target->m_ed->setDocument(std::move(doc), path);
    // The panes follow the new presentation (documentReplaced); its slides may be another size.
    target->m_canvas->zoomToFit(Canvas::Fit::WholePage);
    Settings::get().addRecentFile(path);
    hideFileScreen();
    if (!notes.isEmpty())
        QMessageBox::warning(target, tr("Open"), tr("Some of \"%1\" couldn't be read and was left out.\n\n%2").arg(name, notes));
    return true;
}

} // namespace jp
