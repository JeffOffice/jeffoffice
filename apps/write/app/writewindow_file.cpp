// Opening, saving, and recovering documents in JeffWrite's window.

#include "app/writewindow.h"

#include "app/editor.h"
#include "app/filedialogs.h"
#include "app/filetasks.h"
#include "app/icons.h"
#include "app/optionsdialog.h"
#include "app/pageview.h"
#include "app/recovery.h"
#include "app/ruler.h"
#include "app/settings.h"
#include "app/telemetry.h"
#include "app/writefilescreen.h"
#include "core/appinfo.h"
#include "app/passworddialogs.h"
#include "io/crypto.h"
#include "office/officecrypt.h"
#include "office/package.h"
#include "office/repair.h"
#include "odt/odttemplate.h"

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QListWidget>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStatusBar>
#include <QVBoxLayout>
#include "app/plaintext.h"

namespace jp {

namespace {

enum class UnlockResult { Opened, Cancelled, Failed };

// A file protected with a password becomes the package inside it: the password files protected only against changes by accident have is tried first, then the person is asked, as often as it
// takes. `bytes` is the package afterwards; `kept` the password that opened it; a file that is not protected is left as it is.
UnlockResult unlockProtected(QWidget *parent, const QString &fileName, QByteArray *bytes, QString *kept, bool *wasProtected, QString *error)
{
    QByteArray info, package, zip;
    if (!office::splitProtectedPackage(*bytes, &info, &package)) return UnlockResult::Opened;
    // (the file's own bytes are let go: the package inside is what is worked on, and a file of a gigabyte is not held twice more)
    *bytes = QByteArray();
    bytes->squeeze();
    auto decrypt = [&](const QString &password, office::CryptResult *result) {
        // On a thread of its own, with a note and a Cancel button: a key made with many rounds of hashing takes seconds.
        const bool finished = runWithCancel(parent, QCoreApplication::translate("jp::WriteWindow", "Checking the password…"), [&](const std::atomic<bool> *cancel) {
            *result = office::decryptPackage(info, package, password, office::kMaxOpenFileBytes, false, &zip, [cancel] { return cancel->load(); });
        });
        if (!finished) *result = office::CryptResult::Cancelled;
    };
    office::CryptResult r = office::CryptResult::Unsupported;
    decrypt(office::defaultPassword(), &r);
    if (r == office::CryptResult::Ok) {
        *bytes = zip;
        return UnlockResult::Opened;
    }
    bool wrong = false;
    for (;;) {
        QString password;
        if (!askOpenFilePassword(parent, fileName, wrong, &password)) return UnlockResult::Cancelled;
        decrypt(password, &r);
        if (r == office::CryptResult::Ok) {
            *bytes = zip;
            *kept = password;
            *wasProtected = true;
            return UnlockResult::Opened;
        }
        crypto::wipe(password);
        if (r == office::CryptResult::Cancelled) return UnlockResult::Cancelled;
        if (r == office::CryptResult::WrongPassword) {
            wrong = true;
            continue;
        }
        *error = r == office::CryptResult::Unsupported ? QCoreApplication::translate("jp::WriteWindow", "This file is protected in a way this program doesn't read.")
               : r == office::CryptResult::TooLarge    ? QCoreApplication::translate("jp::WriteWindow", "The file is too large to open.")
               : r == office::CryptResult::TooCostly   ? QCoreApplication::translate("jp::WriteWindow", "The password protection of this file asks for more work than this program allows.")
                                                       : QCoreApplication::translate("jp::WriteWindow", "The file is damaged.");
        return UnlockResult::Failed;
    }
}


// The endings JeffWrite writes. A file with another (an old binary document) is read, never written over: it is saved under a new name.
bool isWrittenEnding(const QString &path)
{
    const QString ending = QFileInfo(path).suffix().toLower();
    return ending == QLatin1String("docx") || ending == QLatin1String("dotx") || ending == QLatin1String("odt") || ending == QLatin1String("ott");
}

// The old binary kinds, which are read and never written over.
bool isOldBinary(const QString &path)
{
    const QString ending = QFileInfo(path).suffix().toLower();
    return ending == QLatin1String("doc") || ending == QLatin1String("dot") || ending == QLatin1String("rtf");
}

bool isRichText(const QString &path) { return QFileInfo(path).suffix().compare(QLatin1String("rtf"), Qt::CaseInsensitive) == 0; }

// A flat OpenDocument text (one XML file), which is read and never written over: it is saved as a packed .odt.
bool isFlatOdt(const QString &path) { return QFileInfo(path).suffix().compare(QLatin1String("fodt"), Qt::CaseInsensitive) == 0; }

QString fileKind(const QString &path)
{
    static const QStringList kinds = {QStringLiteral("docx"), QStringLiteral("dotx"), QStringLiteral("docm"), QStringLiteral("dotm"), QStringLiteral("doc"), QStringLiteral("dot"), QStringLiteral("rtf"), QStringLiteral("odt"), QStringLiteral("fodt"), QStringLiteral("ott")};
    const QString ending = QFileInfo(path).suffix().toLower();
    return kinds.contains(ending) ? ending : QStringLiteral("other");
}

} // namespace

void WriteWindow::createFileActions()
{
    mk("file.new", tr("New"), "file-plus", QKeySequence::New, [] {
        auto *w = new WriteWindow();
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
    });
    mk("file.open", tr("Open"), "folder-open", QKeySequence::Open, [this] { browseAndOpen(); });
    mk("file.close", tr("Close"), "circle-x", QKeySequence::Close, [this] { close(); });
    mk("file.save", tr("Save"), "save", QKeySequence::Save, [this] { save(); });
    mk("file.saveAs", tr("Save As"), "save-all", QKeySequence(Qt::Key_F12), [this] { saveAs(); });
    mk("file.print", tr("Print…"), "printer", QKeySequence::Print, [this] { showFileScreen(QStringLiteral("print")); });
    mk("file.exportPdf", tr("Export to PDF…"), "file-output", QKeySequence(), [this] { exportPdf(); });
    mk("file.options", tr("Options"), "settings", QKeySequence(), [this] {
        // The shared pages (General, Proofing); JeffWrite has no pages of its own yet.
        Editor stand;
        stand.view.spelling = m_view->spellingEnabled();   // (its "check spelling as you type" is the view's)
        if (optionsDialog(this, &stand, {})) {
            act("review.checkAsType")->setChecked(stand.view.spelling);
            m_view->setSpellingEnabled(stand.view.spelling);
            Settings::get().setValue(QStringLiteral("proof/asYouType"), stand.view.spelling);
        }
        // (The name and initials Options has may be new: they are who makes changes and comments from here on.)
        if (m_doc) m_doc->setReviewer(Settings::get().userName(), Settings::get().value(QStringLiteral("user/initials")).toString());
    });

    static quint64 s_windows = 0;
    m_serial = ++s_windows;
    m_recoverTimer.setInterval(std::max(1, Settings::get().autoRecoverMinutes()) * 60 * 1000);
    connect(&m_recoverTimer, &QTimer::timeout, this, &WriteWindow::autoRecover);
    m_recoverTimer.start();
}

void WriteWindow::browseAndOpen(OpenHow how)
{
    const QString p = QFileDialog::getOpenFileName(this, openHowText(how), Settings::get().value("dirs/open", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString(),
                                                   tr("Documents (*.docx *.docm *.dotx *.dotm *.doc *.dot *.rtf *.odt *.fodt *.ott)") + QStringLiteral(";;") + tr("Rich Text Format (*.rtf)") + QStringLiteral(";;") + tr("OpenDocument Text (*.odt *.fodt *.ott)") + QStringLiteral(";;") + tr("All Files (*)"));
    if (p.isEmpty()) return;
    Settings::get().setValue("dirs/open", QFileInfo(p).absolutePath());
    openFile(p, how);
}

void WriteWindow::setReadOnly(bool on)
{
    AppWindow::setReadOnly(on);
    m_view->setReadOnly(on);
    m_ruler->setEnabled(!on);
    updateTitle();
}

bool WriteWindow::openFile(const QString &path, OpenHow how)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QByteArray bytes;
    QString err;
    QStringList notes;
    std::unique_ptr<write::WriteDoc> doc;
    // (A file larger than the apps open is refused before it is read: it would all be held at once.)
    office::RepairReport repair;
    const bool repairing = how == OpenHow::Repair;
    QString openedWith;   // the password a protected file was opened with
    bool wasProtected = false, cancelledAsking = false;
    if (office::readWholeFile(path, &bytes, &err) && !bytes.isEmpty()) {
        // A file protected with a password is opened with it: asked for, as often as it takes.
        if (!repairing) {
            QApplication::restoreOverrideCursor();
            const UnlockResult unlocked = unlockProtected(this, QFileInfo(path).fileName(), &bytes, &openedWith, &wasProtected, &err);
            QApplication::setOverrideCursor(Qt::WaitCursor);
            if (unlocked == UnlockResult::Cancelled) {
                cancelledAsking = true;
                bytes.clear();
            } else if (unlocked == UnlockResult::Failed) {
                bytes.clear();
            }
        }
        // Open and Repair: what can be read of a damaged package is made into a package that opens, and what was left out is said.
        if (repairing) bytes = office::repairPackage(bytes, &err, &repair);
        if (!bytes.isEmpty()) doc = write::WriteDoc::fromFile(bytes, &err, &notes);
    }
    QApplication::restoreOverrideCursor();
    const QString name = QFileInfo(path).fileName();
    if (!doc) {
        crypto::wipe(openedWith);
        if (!cancelledAsking) jp::plainWarning(this, tr("Open"), tr("%1 can't open \"%2\".\n%3").arg(appInfo().name, name, err));
        return false;
    }
    // A new document nobody has touched gives way; any other stays.
    WriteWindow *target = this;
    if (isModified() || !m_path.isEmpty() || how == OpenHow::NewWindow) {
        target = new WriteWindow();
        target->setAttribute(Qt::WA_DeleteOnClose);
        target->show();
    }
    // A template opens as a new document made from it; so does a file opened as a copy.
    const QString ending = QFileInfo(path).suffix().toLower();
    // (opened read-only, the template is itself what is opened, for reading: nothing is made from it)
    const bool tmpl = (ending == QLatin1String("dotx") || ending == QLatin1String("dotm") || ending == QLatin1String("dot") || ending == QLatin1String("ott")) && how != OpenHow::ReadOnly;
    const bool copy = how == OpenHow::Copy || repairing;   // (a repaired document is not the file it was repaired from: Save asks for a name)
    const bool leftOut = doc->leftSomethingOut();
    if (copy) target->m_untitled = (repairing ? tr("Repaired %1") : tr("Copy of %1")).arg(QFileInfo(path).completeBaseName());
    target->setReadOnly(how == OpenHow::ReadOnly);
    target->setDocument(std::move(doc), tmpl || copy ? QString() : path);
    target->m_macrosNoted = false;
    target->m_personalInfoRemoved = false;
    target->m_recoveryNoted = false;
    target->m_fileProtected = wasProtected && !tmpl && !copy;
    crypto::wipe(target->m_filePassword);
    if (target->m_fileProtected) target->m_filePassword = openedWith;
    crypto::wipe(openedWith);
    // A document marked as final opens for reading only (the Info page lets it be edited again).
    if (how == OpenHow::Normal && !tmpl && target->m_doc->markedAsFinal()) {
        target->setReadOnly(true);
        target->statusBar()->showMessage(tr("Marked as final: this document is open for reading only. Open File > Info and choose Marked as Final to edit it."), 12000);
    }
    target->m_odtNoted.clear();
    target->m_protectedPath = leftOut && !tmpl && !copy && !isOldBinary(path) ? path : QString();
    if (tmpl || copy) target->m_doc->setModified(true);
    Settings::get().addRecentFile(path);
    telemetry::count(QStringLiteral("file.open.") + fileKind(path));
    hideFileScreen();
    if (repairing) target->notice(tr("Open and Repair"), tr("\"%1\" was opened as a repaired copy. Save it under a name of its own.\n\n%2").arg(name, office::repairSummary(repair)), repair.damaged());
    if (!notes.isEmpty()) target->notice(tr("Open"), tr("Some of \"%1\" couldn't be read and was left out.\n\n%2").arg(name, notes.join(QLatin1Char('\n'))), true);
    return true;
}

bool WriteWindow::save()
{
    // A document open for reading only is saved as a copy under a new name.
    if (isReadOnly()) {
        notice(tr("Save"), tr("This document is open for reading only, so it is saved under a new name."));
        return saveAs();
    }
    // A file that could not be read whole is not written over: the copy is saved under a name of its own.
    if (m_doc && m_doc->leftSomethingOut() && !m_protectedPath.isEmpty() && m_path == m_protectedPath) {
        notice(tr("Save"), tr("Part of this file couldn't be opened, so saving it here would lose that part: choose a new name for the copy."));
        return saveAs();
    }
    if (isRichText(m_path)) notice(tr("Save"), tr("This document came from an .rtf file, which %1 doesn't write, so it is saved as a new .docx file.").arg(appInfo().name));
    else if (isOldBinary(m_path)) notice(tr("Save"), tr("This document came from an older .doc file, which %1 doesn't write, so it is saved as a new .docx file.").arg(appInfo().name));
    else if (isFlatOdt(m_path)) notice(tr("Save"), tr("This document came from a flat .fodt file, which %1 doesn't write, so it is saved as a new .odt file.").arg(appInfo().name));
    if (m_path.isEmpty() || !isWrittenEnding(m_path)) return saveAs();
    return saveTo(m_path);
}

void WriteWindow::notice(const QString &title, const QString &text, bool warning)
{
    if (m_notifier) m_notifier(title, text);
    else if (warning) jp::plainWarning(this, title, text);
    else jp::plainInformation(this, title, text);
}

bool WriteWindow::saveAs()
{
    const QString from = m_path.isEmpty() ? m_recoveredFrom : m_path;
    const QString dir = from.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(from).absolutePath();
    // (a document open for reading only is offered a name of its own, not the file's)
    const QString name = isReadOnly() ? tr("%1 copy").arg(from.isEmpty() ? displayName() : QFileInfo(from).completeBaseName()) : from.isEmpty() ? displayName() : QFileInfo(from).completeBaseName();
    // The dialog hands back the filter text it showed, so the choice is recognized by comparing with these same (translated) texts.
    const QString docxFilter = tr("Document (*.docx)"), dotxFilter = tr("Template (*.dotx)"), odtFilter = tr("OpenDocument Text (*.odt)"), ottFilter = tr("OpenDocument Text Template (*.ott)");
    const QString fromEnding = QFileInfo(from).suffix().toLower();
    QString selected = fromEnding == QLatin1String("dotx") || fromEnding == QLatin1String("dot") ? dotxFilter : fromEnding == QLatin1String("ott") ? ottFilter : fromEnding == QLatin1String("odt") || fromEnding == QLatin1String("fodt") ? odtFilter : docxFilter;
    auto endingOf = [&](const QString &filter) { return filter == dotxFilter ? QStringLiteral(".dotx") : filter == odtFilter ? QStringLiteral(".odt") : filter == ottFilter ? QStringLiteral(".ott") : QStringLiteral(".docx"); };
    const QString suggested = dir + QLatin1Char('/') + name + endingOf(selected);
    QString path = m_pathChooser ? m_pathChooser(suggested, selected)
                                 : askSavePath(this, tr("Save As"), suggested, docxFilter + QStringLiteral(";;") + dotxFilter + QStringLiteral(";;") + odtFilter + QStringLiteral(";;") + ottFilter, &selected);
    if (path.isEmpty()) return false;
    if (!isWrittenEnding(path)) path += endingOf(selected);
    return saveTo(path);
}

bool WriteWindow::saveCopy()
{
    if (!m_doc) return false;
    const QString from = m_path.isEmpty() ? m_recoveredFrom : m_path;
    const QString dir = from.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(from).absolutePath();
    const QString name = tr("%1 copy").arg(from.isEmpty() ? displayName() : QFileInfo(from).completeBaseName());
    const QString docxFilter = tr("Document (*.docx)"), dotxFilter = tr("Template (*.dotx)"), odtFilter = tr("OpenDocument Text (*.odt)"), ottFilter = tr("OpenDocument Text Template (*.ott)");
    QString selected = docxFilter;
    auto endingOf = [&](const QString &filter) { return filter == dotxFilter ? QStringLiteral(".dotx") : filter == odtFilter ? QStringLiteral(".odt") : filter == ottFilter ? QStringLiteral(".ott") : QStringLiteral(".docx"); };
    const QString suggested = dir + QLatin1Char('/') + name + endingOf(selected);
    QString path = m_pathChooser ? m_pathChooser(suggested, selected)
                                 : askSavePath(this, tr("Save a Copy"), suggested, docxFilter + QStringLiteral(";;") + dotxFilter + QStringLiteral(";;") + odtFilter + QStringLiteral(";;") + ottFilter, &selected);
    if (path.isEmpty()) return false;
    if (!isWrittenEnding(path)) path += endingOf(selected);
    return saveTo(path, true);
}

bool WriteWindow::saveAsTemplate()
{
    if (!m_doc) return false;
    const QString path = askMyTemplatePath(this, displayName(), QStringLiteral(".dotx"));
    if (path.isEmpty()) return false;
    QString err;
    const QByteArray bytes = m_doc->toDocx(&err, true);
    bool ok = !bytes.isEmpty();
    if (ok) {
        QSaveFile out(path);
        ok = out.open(QIODevice::WriteOnly) && out.write(bytes) == bytes.size() && out.commit();
        if (!ok) err = out.errorString();
    }
    if (!ok) {
        jp::plainWarning(this, tr("Save as Template"), tr("%1 couldn't save \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
        return false;
    }
    saveTemplateThumbnail(path, m_view->pageImage(0, 240));
    statusBar()->showMessage(tr("Saved \"%1\" in My Templates.").arg(QFileInfo(path).completeBaseName()), 5000);
    return true;
}

bool WriteWindow::saveTo(const QString &path, bool asCopy)
{
    if (!m_doc) return false;
    // A document open for reading only is never written over its own file, and a copy is never saved over the file this window has open (it would be the same as saving).
    if (asCopy ? sameFile(path, m_path) : isReadOnly() && sameFile(path, m_path)) {
        notice(tr("Save"), asCopy ? tr("A copy can't be saved over \"%1\". Choose another name.").arg(QFileInfo(path).fileName()) : tr("This document is open for reading only. Choose another name for it."));
        return false;
    }
    // (The same file again, by another way to it: the name is compared as the system sees it.)
    if (m_doc->leftSomethingOut() && !m_protectedPath.isEmpty() && QFileInfo(path).absoluteFilePath() == QFileInfo(m_protectedPath).absoluteFilePath()) {
        notice(tr("Save"), tr("Part of this file couldn't be opened, so saving it here would lose that part: choose a new name for the copy."));
        return false;
    }
    if (m_doc->hadMacros() && !m_macrosNoted) {
        m_macrosNoted = true;
        const QString kind = QFileInfo(path).suffix().toLower() == QLatin1String("odt") || QFileInfo(path).suffix().toLower() == QLatin1String("ott") ? QStringLiteral(".odt") : QStringLiteral(".docx");
        notice(tr("Save"), tr("The macros in this document aren't saved, because a %1 file can't hold them.").arg(kind));
    }
    QString err;
    const bool tmpl = QFileInfo(path).suffix().compare(QLatin1String("dotx"), Qt::CaseInsensitive) == 0;
    const bool ott = QFileInfo(path).suffix().compare(QLatin1String("ott"), Qt::CaseInsensitive) == 0;
    const bool odt = ott || QFileInfo(path).suffix().compare(QLatin1String("odt"), Qt::CaseInsensitive) == 0;
    // A password protects a .docx file; an .odt file can't be protected yet, and is never written without the protection the person asked for.
    if (m_fileProtected && odt) {
        notice(tr("Save"), tr("This document is protected with a password, and a .odt file can't hold a password yet. Save it as a .docx file, or remove the password first (File > Info)."));
        return false;
    }
    // The mark that a document is final is a property a .odt file does not keep: said, so that the window and the file do not disagree.
    if (odt && m_doc->markedAsFinal()) {
        notice(tr("Save"), tr("This document is marked as final, and a .odt file can't hold that mark. The file is saved without it."));
        if (!asCopy) {
            office::PackageProps unmarked = m_doc->packageProps();
            unmarked.custom = office::withMarkAsFinal(unmarked.custom, false);
            m_doc->setPackageProps(unmarked);
        }
    }
    if (!backupBeforeSave(this, path)) return false;
    QStringList notKept;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QByteArray bytes = odt ? m_doc->toOdt(&err, &notKept) : m_doc->toDocx(&err, tmpl);
    if (ott && !bytes.isEmpty()) bytes = office::odt::odfAsTemplate(bytes);
    if (m_fileProtected && !bytes.isEmpty()) bytes = office::encryptPackage(bytes, m_filePassword);
    bool ok = !bytes.isEmpty();
    if (ok) {
        QSaveFile out(path);
        ok = out.open(QIODevice::WriteOnly) && out.write(bytes) == bytes.size() && out.commit();
        if (!ok) err = out.errorString();
    }
    QApplication::restoreOverrideCursor();
    if (!ok) {
        jp::plainWarning(this, tr("Save"), tr("%1 couldn't save \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
        return false;
    }
    if (odt) {
        // (Told once of each thing, not once of the first: a later save that leaves out something new says so.)
        QStringList fresh;
        for (const QString &what : std::as_const(notKept))
            if (!m_odtNoted.contains(what)) {
                m_odtNoted.insert(what);
                fresh << what;
            }
        if (!fresh.isEmpty()) notice(tr("Save"), tr("An OpenDocument text file can't hold some of what this document has, so those parts were saved as they stand now or left out:\n\n%1").arg(fresh.join(QLatin1Char('\n'))));
    }
    if (asCopy) {
        Settings::get().addRecentFile(path);
        telemetry::count(QStringLiteral("file.save.") + fileKind(path));
        statusBar()->showMessage(tr("Saved a copy as %1").arg(QFileInfo(path).fileName()), 4000);
        return true;
    }
    m_doc->setModified(false);
    m_path = path;
    m_doc->filePath = path;
    if (isReadOnly()) setReadOnly(false);
    m_recoveredFrom.clear();
    dropRecoveryCopy();
    Settings::get().addRecentFile(path);
    telemetry::count(QStringLiteral("file.save.") + fileKind(path));
    updateTitle();
    statusBar()->showMessage(tr("Saved %1").arg(QFileInfo(path).fileName()), 4000);
    return true;
}

// ---- Automatic recovery

void WriteWindow::autoRecover()
{
    // Options may have changed the interval since the timer started.
    const int ms = std::max(1, Settings::get().autoRecoverMinutes()) * 60 * 1000;
    if (m_recoverTimer.interval() != ms) m_recoverTimer.start(ms);
    if (!m_doc || !isModified()) return;
    // A document protected with a password is never written out as plain text: no copy is kept for it, and the person is told.
    if (m_fileProtected) {
        if (!m_recoveryNoted) {
            m_recoveryNoted = true;
            statusBar()->showMessage(tr("This document is protected with a password, so no recovery copy is kept of it. Save it to keep your work."), 10000);
        }
        return;
    }
    // Named for the document as it is now (Save As moves it).
    const QString copy = recovery::copyPath(m_path, displayName(), m_serial);
    if (copy != m_recoveryCopy) dropRecoveryCopy();
    const QString source = m_path.isEmpty() ? m_recoveredFrom : m_path;
    const bool written = recovery::write(
        [this](const QString &file, QString *error) {
            const QByteArray bytes = m_doc->toDocx(error);
            if (bytes.isEmpty()) return false;
            QSaveFile out(file);
            if (!out.open(QIODevice::WriteOnly) || out.write(bytes) != bytes.size() || !out.commit()) {
                if (error) *error = out.errorString();
                return false;
            }
            return true;
        },
        copy, source, displayName());
    if (written) m_recoveryCopy = copy;
}

void WriteWindow::dropRecoveryCopy()
{
    recovery::remove(m_recoveryCopy);
    m_recoveryCopy.clear();
}

void WriteWindow::offerRecovery(bool askIfNone)
{
    const QVector<recovery::Recovered> found = recovery::orphans();
    if (found.isEmpty()) {
        if (askIfNone) jp::plainInformation(this, tr("Recover Unsaved Work"), tr("There's no unsaved work to recover."));
        return;
    }
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Recover Unsaved Work"));
    auto *v = new QVBoxLayout(&dlg);
    auto *intro = plainLabel(new QLabel(tr("%1 didn't close normally, and saved copies of your unsaved work. Choose which to open; you can save each one where you like.").arg(appInfo().name), &dlg));
    intro->setWordWrap(true);
    v->addWidget(intro);
    auto *list = new QListWidget(&dlg);
    list->setAccessibleName(tr("Recovered documents"));
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
        if (jp::plainQuestion(&dlg, tr("Delete"), tr("Delete the checked copies? Their unsaved work can't be brought back.")) != QMessageBox::Yes) return;
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
        if (!openRecovered(r, &err)) jp::plainWarning(this, tr("Recover Unsaved Work"), tr("%1 can't open the copy of \"%2\".\n%3").arg(appInfo().name, r.title, err));
    }
}

WriteWindow *WriteWindow::openRecovered(const recovery::Recovered &r, QString *error)
{
    QByteArray bytes;
    // (Closed before the old copy is discarded below: Windows can't delete an open file.)
    if (!office::readWholeFile(r.file, &bytes, error)) return nullptr;
    std::unique_ptr<write::WriteDoc> doc = write::WriteDoc::fromDocx(bytes, error);
    if (!doc) return nullptr;
    WriteWindow *target = this;
    if (!m_path.isEmpty() || isModified()) {
        target = new WriteWindow();
        target->setAttribute(Qt::WA_DeleteOnClose);
        target->show();
    }
    target->m_untitled = r.title;
    target->setDocument(std::move(doc), QString());
    target->m_recoveredFrom = r.source;
    target->m_doc->setModified(true);
    target->updateTitle();
    // This run keeps its own copy before the old one goes.
    target->autoRecover();
    if (!target->m_recoveryCopy.isEmpty()) recovery::discard(r);
    return target;
}

} // namespace jp
