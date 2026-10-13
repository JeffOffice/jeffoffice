// Saving presentations in JeffDeck's window: File > Save and Save As, and
// AutoRecover copies of work that isn't saved yet.

#include "app/deckcanvas.h"
#include "app/deckwindow.h"

#include "app/editor.h"
#include "app/filedialogs.h"
#include "app/icons.h"
#include "app/notespane.h"
#include "app/outlineview.h"
#include "app/recovery.h"
#include "app/settings.h"
#include "canvas/canvas.h"
#include "core/appinfo.h"
#include "io/pptxexport.h"
#include "io/pptximport.h"
#include "io/presentationfile.h"

#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QListWidget>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QStatusBar>
#include <QVBoxLayout>

namespace jp {

namespace {

// The endings JeffDeck writes (.pptx, .potx, .ppsx, .odp, .otp). A file with
// another (an old binary presentation) is read, never written over: it is saved
// under a new name.
bool isKnownEnding(const QString &path) { return deck::isWrittenEnding(path); }

} // namespace

void DeckWindow::createSaveActions()
{
    mk("file.save", tr("Save"), "save", QKeySequence::Save, [this] { save(); });
    mk("file.saveAs", tr("Save As"), "save-all", QKeySequence(Qt::Key_F12), [this] { saveAs(); });

    static quint64 s_windows = 0;
    m_serial = ++s_windows;
    m_recoverTimer.setInterval(std::max(1, Settings::get().autoRecoverMinutes()) * 60 * 1000);
    connect(&m_recoverTimer, &QTimer::timeout, this, &DeckWindow::autoRecover);
    m_recoverTimer.start();
    // A copy is only for work that isn't saved: it goes once the presentation
    // is saved, replaced, or closed.
    connect(m_ed, &Editor::documentReplaced, this, [this] {
        dropRecoveryCopy();
        m_recoveredFrom.clear();
        m_templatePath.clear();
    });
    connect(m_ed, &Editor::modifiedChanged, this, [this](bool modified) {
        if (!modified) dropRecoveryCopy();
    });
}

bool DeckWindow::save()
{
    const QString path = m_ed->filePath();
    // A presentation opened from a template is saved as a new presentation:
    // the template stays as it is, unless this window wrote it. So is one
    // opened from a file of a kind JeffDeck doesn't write.
    const bool isTemplate = deck::presentationKindOf(path) == deck::PresentationKind::Template && path != m_templatePath;
    if (path.isEmpty() || isTemplate || !isKnownEnding(path)) return saveAs();
    return saveTo(path);
}

bool DeckWindow::saveAs()
{
    const QString from = m_ed->filePath().isEmpty() ? m_recoveredFrom : m_ed->filePath();
    const QString dir = from.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(from).absolutePath();
    const QString name = from.isEmpty() ? m_ed->displayName() : QFileInfo(from).completeBaseName();
    // The dialog hands back the filter text it showed, so the choice is
    // recognized by comparing with these same (translated) texts.
    const QString pptxFilter = tr("Presentation (*.pptx)"), potxFilter = tr("Template (*.potx)"), ppsxFilter = tr("Show (*.ppsx)");
    const QString odpFilter = tr("OpenDocument Presentation (*.odp)"), otpFilter = tr("OpenDocument Presentation Template (*.otp)");
    const deck::PresentationKind was = deck::presentationKindOf(from);
    const bool ownTemplate = was == deck::PresentationKind::Template && from == m_templatePath;
    // A file of an open document kind is saved as one again; a template only when this window wrote it.
    const bool odf = deck::formatOfFileName(from) == deck::FileFormat::Odp;
    QString selected = was == deck::PresentationKind::Show ? ppsxFilter
                       : ownTemplate                       ? (odf ? otpFilter : potxFilter)
                       : odf                               ? odpFilter
                                                           : pptxFilter;
    auto endingOf = [&](const QString &filter) {
        return filter == ppsxFilter ? QStringLiteral(".ppsx") : filter == potxFilter ? QStringLiteral(".potx") : filter == odpFilter ? QStringLiteral(".odp") : filter == otpFilter ? QStringLiteral(".otp") : QStringLiteral(".pptx");
    };
    const QString suggested = dir + QLatin1Char('/') + name + endingOf(selected);
    QString path = m_pathChooser ? m_pathChooser(suggested, selected)
                                 : askSavePath(this, tr("Save As"), suggested,
                                               pptxFilter + QStringLiteral(";;") + potxFilter + QStringLiteral(";;") + ppsxFilter + QStringLiteral(";;") + odpFilter + QStringLiteral(";;") + otpFilter, &selected);
    if (path.isEmpty()) return false;
    if (!isKnownEnding(path)) path += endingOf(selected);
    return saveTo(path);
}

bool DeckWindow::saveTo(const QString &path)
{
    // What was typed in the notes or a slide's text goes into the file.
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    Document *doc = m_ed->doc();
    doc->props.modified = QDateTime::currentDateTime();
    if (doc->props.author.isEmpty()) doc->props.author = Settings::get().userName();
    QString err;
    QStringList skipped;
    if (!deck::saveAnyPresentationFile(*doc, path, &err, &skipped)) {
        QMessageBox::warning(this, tr("Save"), tr("%1 couldn't save \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
        return false;
    }
    m_ed->setFilePath(path);
    m_ed->markSaved();
    m_recoveredFrom.clear();
    m_templatePath = deck::presentationKindOf(path) == deck::PresentationKind::Template ? path : QString();
    Settings::get().addRecentFile(path);
    updateTitle();
    if (skipped.isEmpty()) statusBar()->showMessage(tr("Saved %1").arg(QFileInfo(path).fileName()), 4000);
    else if (deck::formatOfFileName(path) == deck::FileFormat::Odp) statusBar()->showMessage(tr("Saved %1. Left out, as this kind of file can't hold it: %2").arg(QFileInfo(path).fileName(), skipped.join(QLatin1Char(' '))), 15000);
    else statusBar()->showMessage(tr("Saved %1, but %n object(s) can't be saved in this kind of file yet.", nullptr, int(skipped.size())).arg(QFileInfo(path).fileName()), 8000);
    return true;
}

void DeckWindow::autoRecover()
{
    // Options may have changed the interval since the timer started.
    const int ms = std::max(1, Settings::get().autoRecoverMinutes()) * 60 * 1000;
    if (m_recoverTimer.interval() != ms) m_recoverTimer.start(ms);
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->flushTyping();
    if (!m_ed->isModified()) return;
    // Named for the presentation as it is now (Save As moves it).
    const QString copy = recovery::copyPath(m_ed->filePath(), m_ed->displayName(), m_serial);
    if (copy != m_recoveryCopy) dropRecoveryCopy();
    const QString source = m_ed->filePath().isEmpty() ? m_recoveredFrom : m_ed->filePath();
    const bool written = recovery::write([this](const QString &file, QString *error) { return deck::savePresentationFile(*m_ed->doc(), file, error); },
                                         copy, source, m_ed->displayName());
    if (written) m_recoveryCopy = copy;
}

void DeckWindow::dropRecoveryCopy()
{
    recovery::remove(m_recoveryCopy);
    m_recoveryCopy.clear();
}

void DeckWindow::offerRecovery(bool askIfNone)
{
    const QVector<recovery::Recovered> found = recovery::orphans();
    if (found.isEmpty()) {
        if (askIfNone) QMessageBox::information(this, tr("Recover Unsaved Work"), tr("There's no unsaved work to recover."));
        return;
    }
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Recover Unsaved Work"));
    auto *v = new QVBoxLayout(&dlg);
    auto *intro = new QLabel(tr("%1 didn't close normally, and saved copies of your unsaved work. Choose which to open; you can save each one where you like.").arg(appInfo().name), &dlg);
    intro->setWordWrap(true);
    v->addWidget(intro);
    auto *list = new QListWidget(&dlg);
    list->setAccessibleName(tr("Recovered presentations"));
    const QLocale loc;
    for (const auto &r : found) {
        const QString where = r.source.isEmpty() ? tr("never saved") : QDir::toNativeSeparators(r.source);
        auto *it = new QListWidgetItem(tr("%1\n%2, saved %3").arg(r.title, where, loc.toString(r.saved, QLocale::ShortFormat)), list);
        it->setFlags(it->flags() | Qt::ItemIsUserCheckable);
        it->setCheckState(Qt::Checked);
        it->setIcon(icon(QStringLiteral("life-buoy")));
    }
    list->setMinimumSize(480, 180);
    v->addWidget(list);
    auto *buttons = new QDialogButtonBox(&dlg);
    QPushButton *open = buttons->addButton(tr("Open"), QDialogButtonBox::AcceptRole);
    QPushButton *del = buttons->addButton(tr("Delete"), QDialogButtonBox::DestructiveRole);
    buttons->addButton(tr("Not Now"), QDialogButtonBox::RejectRole);
    v->addWidget(buttons);
    open->setDefault(true);
    int choice = 0;
    connect(open, &QPushButton::clicked, &dlg, [&] { choice = 1; dlg.accept(); });
    connect(del, &QPushButton::clicked, &dlg, [&] {
        if (QMessageBox::question(&dlg, tr("Delete"), tr("Delete the checked copies? Their unsaved work can't be brought back.")) != QMessageBox::Yes)
            return;
        choice = 2;
        dlg.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return;
    for (int i = 0; i < found.size(); ++i) {
        if (list->item(i)->checkState() != Qt::Checked) continue;
        const recovery::Recovered &r = found[i];
        if (choice == 2) {
            recovery::discard(r);
            continue;
        }
        QString err;
        if (!openRecovered(r, &err))
            QMessageBox::warning(this, tr("Recover Unsaved Work"), tr("%1 can't open the copy of \"%2\".\n%3").arg(appInfo().name, r.title, err));
    }
}

DeckWindow *DeckWindow::openRecovered(const recovery::Recovered &r, QString *error)
{
    QByteArray bytes;
    {
        // Closed before the old copy is discarded below: Windows can't delete an open file.
        QFile copy(r.file);
        if (!copy.open(QIODevice::ReadOnly)) {
            if (error) *error = copy.errorString();
            return nullptr;
        }
        bytes = copy.readAll();
    }
    std::unique_ptr<Document> doc = deck::importPresentation(bytes, error);
    if (!doc) return nullptr;
    DeckWindow *target = this;
    if (!m_ed->filePath().isEmpty() || m_ed->isModified()) {
        target = new DeckWindow();
        target->setAttribute(Qt::WA_DeleteOnClose);
        target->show();
    }
    target->m_ed->endTextEdit();
    target->m_ed->setDocument(std::move(doc));
    target->m_ed->setUntitledName(r.title);
    target->m_recoveredFrom = r.source;
    target->m_ed->markUnsaved();
    target->m_canvas->zoomToFit(Canvas::Fit::WholePage);
    target->updateTitle();
    // This run keeps its own copy before the old one goes.
    target->autoRecover();
    if (!target->m_recoveryCopy.isEmpty()) recovery::discard(r);
    return target;
}

} // namespace jp
