// Opening presentation files in JeffDeck's window.

#include "app/deckcanvas.h"
#include "app/deckwindow.h"

#include "app/editor.h"
#include "app/notespane.h"
#include "app/outlineview.h"
#include "app/settings.h"
#include "app/telemetry.h"
#include "canvas/canvas.h"
#include "core/appinfo.h"
#include "io/pptximport.h"
#include "io/presentationfile.h"
#include "app/passworddialogs.h"
#include "io/crypto.h"
#include "office/package.h"
#include "office/repair.h"

#include <QApplication>
#include <QFile>
#include <QStatusBar>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFileInfo>
#include <QMessageBox>
#include "app/plaintext.h"

namespace jp {

QString DeckWindow::fileKind(const QString &path)
{
    static const QStringList kinds = {QStringLiteral("pptx"), QStringLiteral("potx"), QStringLiteral("ppsx"), QStringLiteral("ppt"),
                                      QStringLiteral("pot"), QStringLiteral("pps"), QStringLiteral("odp"), QStringLiteral("otp")};
    const QString ending = QFileInfo(path).suffix().toLower();
    return kinds.contains(ending) ? ending : QStringLiteral("other");
}

void DeckWindow::browseAndOpen(OpenHow how)
{
    const QString p = QFileDialog::getOpenFileName(this, openHowText(how), Settings::get().value("dirs/open", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString(),
                                                   tr("Presentations (*.pptx *.potx *.ppsx *.ppt *.pps *.pot *.odp *.otp)") + QStringLiteral(";;") + tr("All Files (*)"));
    if (p.isEmpty()) return;
    Settings::get().setValue("dirs/open", QFileInfo(p).absolutePath());
    openFile(p, how);
}

void DeckWindow::setReadOnly(bool on)
{
    AppWindow::setReadOnly(on);
    m_ed->setReadOnly(on);
    if (m_notes) m_notes->setReadOnly(on);
    if (m_outline) m_outline->setReadOnly(on);
    if (m_formatPane) m_formatPane->setEnabled(!on);   // (its boxes change the slide: a presentation open for reading only is not changed by them)
    // The commands that change the document are turned off (and refused if something still runs one).
    for (const QString &id : actionIds())
        if (QAction *a = act(id); a && !commandAllowedReadOnly(id)) a->setEnabled(!on);
    updateTitle();
}

bool DeckWindow::openFile(const QString &path, OpenHow how)
{
    QString notes;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    office::RepairReport repair;
    const bool repairing = how == OpenHow::Repair;
    std::unique_ptr<Document> doc;
    QString openedWith;   // the password a protected presentation was opened with
    bool wasProtected = false, cancelledAsking = false;
    if (repairing) {
        // Open and Repair: what can be read of a damaged package is made into a package that opens, and what was left out is said.
        QByteArray bytes;
        if (office::readWholeFile(path, &bytes, &notes)) {
            const QByteArray fixed = office::repairPackage(bytes, &notes, &repair);
            if (!fixed.isEmpty()) doc = deck::importPresentation(fixed, &notes);
        }
    } else {
        // A presentation protected with a password is a compound file holding the package: it is opened with the password, asked for as often as it takes (an old binary presentation is
        // a compound file too, and is read as one below).
        bool handled = false;
        QFile probe(path);
        const bool compound = probe.open(QIODevice::ReadOnly) && probe.read(8) == QByteArray::fromHex("D0CF11E0A1B11AE1");
        probe.close();
        QByteArray bytes;
        if (compound && office::readWholeFile(path, &bytes, &notes)) {
            QApplication::restoreOverrideCursor();
            const PackageUnlock unlocked = unlockProtectedPackage(this, QFileInfo(path).fileName(), &bytes, &openedWith, &wasProtected, &notes);
            QApplication::setOverrideCursor(Qt::WaitCursor);
            if (unlocked == PackageUnlock::WindowGone) {
                // (this window is deleted: nothing of it is touched from here)
                QApplication::restoreOverrideCursor();
                crypto::wipe(openedWith);
                return false;
            }
            if (unlocked == PackageUnlock::Cancelled) {
                cancelledAsking = true;
                handled = true;
            } else if (unlocked == PackageUnlock::Failed) {
                handled = true;
            } else if (bytes.startsWith("PK")) {
                // (a package: one with the password every program knows opens without a question, and is not one that asks for a password)
                doc = deck::importPresentation(bytes, &notes);
                handled = true;
            }
        }
        if (!handled) {
            notes.clear();
            doc = deck::importAnyPresentationFile(path, &notes);
        }
    }
    QApplication::restoreOverrideCursor();
    const QString name = QFileInfo(path).fileName();
    if (!doc) {
        crypto::wipe(openedWith);
        if (cancelledAsking) return false;
        const QString text = tr("%1 can't open \"%2\".\n%3").arg(appInfo().name, name, notes);
        if (m_notifier) m_notifier(tr("Open"), text);   // (a run with nobody at the window, a screenshot, says it instead of waiting on a box)
        else jp::plainWarning(this, tr("Open"), text);
        return false;
    }
    // A new presentation nobody has touched gives way; any other stays.
    m_notes->commit();
    if (m_outline) m_outline->commit();
    DeckWindow *target = this;
    if (m_ed->isModified() || !m_ed->filePath().isEmpty() || how == OpenHow::NewWindow) {
        target = new DeckWindow();
        target->setAttribute(Qt::WA_DeleteOnClose);
        target->show();
    }
    target->m_ed->endTextEdit();
    const bool copy = how == OpenHow::Copy || repairing;   // (a repaired presentation is not the file it was repaired from: Save asks for a name)
    target->setReadOnly(false);
    if (copy) {
        // The file's contents as a new presentation that has no file yet (Save asks where); the file is never written.
        target->m_ed->setDocument(std::move(doc));
        target->m_ed->setUntitledName((repairing ? tr("Repaired %1") : tr("Copy of %1")).arg(QFileInfo(path).completeBaseName()));
        target->m_ed->markUnsaved();
        // (a copy of a protected presentation is protected too)
        target->m_fileProtected = wasProtected;
        if (wasProtected) target->m_filePassword = openedWith;
        target->updateTitle();
    } else {
        target->m_ed->setDocument(std::move(doc), path);
        target->m_fileProtected = wasProtected;
        if (wasProtected) target->m_filePassword = openedWith;
        if (how == OpenHow::ReadOnly) target->setReadOnly(true);
        // A presentation marked as final opens for reading only (the Info page lets it be edited again).
        else if (how == OpenHow::Normal && target->markedAsFinal() && deck::presentationKindOf(path) != deck::PresentationKind::Template) {
            target->setReadOnly(true);
            target->statusBar()->showMessage(tr("Marked as final: this presentation is open for reading only. Open File > Info and choose Marked as Final to edit it."), 12000);
        }
    }
    crypto::wipe(openedWith);
    // The panes follow the new presentation (documentReplaced); its slides may be another size.
    target->m_canvas->zoomToFit(Canvas::Fit::WholePage);
    Settings::get().addRecentFile(path);
    telemetry::count(QStringLiteral("file.open.") + fileKind(path));
    hideFileScreen();
    if (repairing) {
        const QString text = tr("\"%1\" was opened as a repaired copy. Save it under a name of its own.\n\n%2").arg(name, office::repairSummary(repair));
        if (m_notifier) m_notifier(tr("Open and Repair"), text);
        else jp::plainInformation(target, tr("Open and Repair"), text);
    }
    if (!notes.isEmpty()) {
        const QString text = tr("Some of \"%1\" couldn't be read and was left out.\n\n%2").arg(name, notes);
        if (m_notifier) m_notifier(tr("Open"), text);
        else jp::plainWarning(target, tr("Open"), text);
    }
    return true;
}

} // namespace jp
