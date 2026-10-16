// Opening, saving, and recovering documents in JeffWrite's window.

#include "app/writewindow.h"

#include "app/editor.h"
#include "app/filedialogs.h"
#include "app/icons.h"
#include "app/optionsdialog.h"
#include "app/pageview.h"
#include "app/recovery.h"
#include "app/settings.h"
#include "app/telemetry.h"
#include "app/writefilescreen.h"
#include "core/appinfo.h"

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

namespace jp {

namespace {

// The endings JeffWrite writes. A file with another (an old binary document) is read, never written over: it is saved under a new name.
bool isWrittenEnding(const QString &path)
{
    const QString ending = QFileInfo(path).suffix().toLower();
    return ending == QLatin1String("docx") || ending == QLatin1String("dotx");
}

QString fileKind(const QString &path)
{
    static const QStringList kinds = {QStringLiteral("docx"), QStringLiteral("dotx"), QStringLiteral("docm"), QStringLiteral("dotm")};
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
    mk("file.open", tr("Open"), "folder-open", QKeySequence::Open, [this] {
        const QString p = QFileDialog::getOpenFileName(this, tr("Open Document"), Settings::get().value("dirs/open", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString(),
                                                       tr("Documents (*.docx *.docm *.dotx *.dotm)") + QStringLiteral(";;") + tr("All Files (*)"));
        if (p.isEmpty()) return;
        Settings::get().setValue("dirs/open", QFileInfo(p).absolutePath());
        openFile(p);
    });
    mk("file.close", tr("Close"), "circle-x", QKeySequence::Close, [this] { close(); });
    mk("file.save", tr("Save"), "save", QKeySequence::Save, [this] { save(); });
    mk("file.saveAs", tr("Save As"), "save-all", QKeySequence(Qt::Key_F12), [this] { saveAs(); });
    mk("file.print", tr("Print…"), "printer", QKeySequence::Print, [this] { showFileScreen(QStringLiteral("print")); });
    mk("file.exportPdf", tr("Export to PDF…"), "file-output", QKeySequence(), [this] { exportPdf(); });
    mk("file.options", tr("Options"), "settings", QKeySequence(), [this] {
        // The shared pages (General, Proofing); JeffWrite has no pages of its own yet.
        Editor stand;
        optionsDialog(this, &stand, {});
    });

    static quint64 s_windows = 0;
    m_serial = ++s_windows;
    m_recoverTimer.setInterval(std::max(1, Settings::get().autoRecoverMinutes()) * 60 * 1000);
    connect(&m_recoverTimer, &QTimer::timeout, this, &WriteWindow::autoRecover);
    m_recoverTimer.start();
}

bool WriteWindow::openFile(const QString &path)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QByteArray bytes;
    QString err;
    QStringList notes;
    std::unique_ptr<write::WriteDoc> doc;
    {
        QFile f(path);
        if (f.open(QIODevice::ReadOnly)) bytes = f.readAll();
        else err = f.errorString();
        if (!bytes.isEmpty()) doc = write::WriteDoc::fromDocx(bytes, &err, &notes);
    }
    QApplication::restoreOverrideCursor();
    const QString name = QFileInfo(path).fileName();
    if (!doc) {
        QMessageBox::warning(this, tr("Open"), tr("%1 can't open \"%2\".\n%3").arg(appInfo().name, name, err));
        return false;
    }
    // A new document nobody has touched gives way; any other stays.
    WriteWindow *target = this;
    if (isModified() || !m_path.isEmpty()) {
        target = new WriteWindow();
        target->setAttribute(Qt::WA_DeleteOnClose);
        target->show();
    }
    // A template opens as a new document made from it.
    const QString ending = QFileInfo(path).suffix().toLower();
    const bool tmpl = ending == QLatin1String("dotx") || ending == QLatin1String("dotm");
    target->setDocument(std::move(doc), tmpl ? QString() : path);
    if (tmpl) target->m_doc->setModified(true);
    Settings::get().addRecentFile(path);
    telemetry::count(QStringLiteral("file.open.") + fileKind(path));
    hideFileScreen();
    if (!notes.isEmpty()) QMessageBox::warning(target, tr("Open"), tr("Some of \"%1\" couldn't be read and was left out.\n\n%2").arg(name, notes.join(QLatin1Char('\n'))));
    return true;
}

bool WriteWindow::save()
{
    if (m_path.isEmpty() || !isWrittenEnding(m_path)) return saveAs();
    return saveTo(m_path);
}

bool WriteWindow::saveAs()
{
    const QString from = m_path.isEmpty() ? m_recoveredFrom : m_path;
    const QString dir = from.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(from).absolutePath();
    const QString name = from.isEmpty() ? displayName() : QFileInfo(from).completeBaseName();
    // The dialog hands back the filter text it showed, so the choice is recognized by comparing with these same (translated) texts.
    const QString docxFilter = tr("Document (*.docx)"), dotxFilter = tr("Template (*.dotx)");
    QString selected = QFileInfo(from).suffix().compare(QLatin1String("dotx"), Qt::CaseInsensitive) == 0 ? dotxFilter : docxFilter;
    auto endingOf = [&](const QString &filter) { return filter == dotxFilter ? QStringLiteral(".dotx") : QStringLiteral(".docx"); };
    const QString suggested = dir + QLatin1Char('/') + name + endingOf(selected);
    QString path = m_pathChooser ? m_pathChooser(suggested, selected)
                                 : askSavePath(this, tr("Save As"), suggested, docxFilter + QStringLiteral(";;") + dotxFilter, &selected);
    if (path.isEmpty()) return false;
    if (!isWrittenEnding(path)) path += endingOf(selected);
    return saveTo(path);
}

bool WriteWindow::saveTo(const QString &path)
{
    if (!m_doc) return false;
    QString err;
    const bool tmpl = QFileInfo(path).suffix().compare(QLatin1String("dotx"), Qt::CaseInsensitive) == 0;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const QByteArray bytes = m_doc->toDocx(&err, tmpl);
    bool ok = !bytes.isEmpty();
    if (ok) {
        QSaveFile out(path);
        ok = out.open(QIODevice::WriteOnly) && out.write(bytes) == bytes.size() && out.commit();
        if (!ok) err = out.errorString();
    }
    QApplication::restoreOverrideCursor();
    if (!ok) {
        QMessageBox::warning(this, tr("Save"), tr("%1 couldn't save \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
        return false;
    }
    m_doc->setModified(false);
    m_path = path;
    m_doc->filePath = path;
    m_recoveredFrom.clear();
    dropRecoveryCopy();
    Settings::get().addRecentFile(path);
    telemetry::count(QStringLiteral("file.save.") + fileKind(path));
    updateTitle();
    statusBar()->showMessage(tr("Saved %1").arg(QFileInfo(path).fileName()), 4000);
    return true;
}

// ---- AutoRecover

void WriteWindow::autoRecover()
{
    // Options may have changed the interval since the timer started.
    const int ms = std::max(1, Settings::get().autoRecoverMinutes()) * 60 * 1000;
    if (m_recoverTimer.interval() != ms) m_recoverTimer.start(ms);
    if (!m_doc || !isModified()) return;
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
        if (QMessageBox::question(&dlg, tr("Delete"), tr("Delete the checked copies? Their unsaved work can't be brought back.")) != QMessageBox::Yes) return;
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
        if (!openRecovered(r, &err)) QMessageBox::warning(this, tr("Recover Unsaved Work"), tr("%1 can't open the copy of \"%2\".\n%3").arg(appInfo().name, r.title, err));
    }
}

WriteWindow *WriteWindow::openRecovered(const recovery::Recovered &r, QString *error)
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
