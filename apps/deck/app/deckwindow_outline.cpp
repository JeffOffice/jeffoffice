// JeffDeck's Slides from Outline: the slides a text, Markdown, rich text, or .docx outline makes (in this presentation after the current slide, or as a new
// one when the file is opened), and Save a Copy as a Markdown outline.

#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/editor.h"
#include "app/filedialogs.h"
#include "app/notespane.h"
#include "app/outlineview.h"
#include "app/settings.h"
#include "core/deck.h"
#include "io/outlineimport.h"
#include "core/appinfo.h"

#include "core/workthread.h"
#include "app/plaintext.h"
#include "app/telemetry.h"
#include "canvas/canvas.h"

#include <QAction>
#include <QApplication>
#include <QEventLoop>
#include <QFileDialog>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTimer>
#include <atomic>

namespace jp {

namespace {

// Reads one outline file on a thread of its own, and then, on the window's thread, hands what it made on. It is a child of the window: deleting it (the window's
// close and its destruction do) stops the read between lines and waits for the thread, and a result nobody is waiting for is never handed on.
class OutlineReadJob : public QObject {
public:
    using Done = std::function<void(bool ok, deck::Outline &outline, const QString &why)>;
    OutlineReadJob(QObject *parent, const QString &path, const std::function<void(const std::atomic<bool> &)> &hold, Done done)
        : QObject(parent), m_path(path), m_hold(hold), m_done(std::move(done))
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        m_worker = std::make_unique<WorkThread>([this] { run(true); });
        if (!m_worker->started()) {   // (no thread to be had: read here)
            m_worker.reset();
            run(false);
        }
    }
    ~OutlineReadJob() override
    {
        m_cancel.store(true);
        m_worker.reset();   // (waits for the thread: it has touched nothing of the window)
        QApplication::restoreOverrideCursor();
    }

private:
    void run(bool onThread)
    {
        if (onThread && m_hold) m_hold(m_cancel);
        m_ok = deck::readOutlineFile(m_path, &m_outline, &m_why, &m_cancel);
        QMetaObject::invokeMethod(this, [this] { finish(); }, Qt::QueuedConnection);
    }
    void finish()
    {
        const bool wanted = !m_cancel.load();
        const bool ok = m_ok;
        deck::Outline outline = std::move(m_outline);
        const QString why = m_why;
        Done done = std::move(m_done);
        delete this;   // (the thread has ended; what follows may show a box, and the window may close meanwhile)
        if (wanted && done) done(ok, outline, why);
    }

    QString m_path;
    std::function<void(const std::atomic<bool> &)> m_hold;
    Done m_done;
    std::atomic<bool> m_cancel{false};
    bool m_ok = false;
    deck::Outline m_outline;
    QString m_why;
    std::unique_ptr<WorkThread> m_worker;   // (last: the thread starts when it is made)
};

} // namespace

void DeckWindow::readOutlineAsync(const QString &path, std::function<void(bool ok, deck::Outline &outline, const QString &why)> done)
{
    m_outlineJobs.removeAll(QPointer<QObject>());
    m_outlineJobs << QPointer<QObject>(new OutlineReadJob(this, path, m_outlineHold, std::move(done)));
}

void DeckWindow::stopOutlineReads()
{
    const auto jobs = std::exchange(m_outlineJobs, {});
    for (const QPointer<QObject> &job : jobs) delete job.data();
}

void DeckWindow::createOutlineActions()
{
    mk("slide.fromOutline", tr("Slides from Outline…"), "list-tree", QKeySequence(), [this] {
        if (isReadOnly()) {
            refuseReadOnlyChange();
            return;
        }
        const QString dir = Settings::get().value(QStringLiteral("dirs/open"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        const QString path = QFileDialog::getOpenFileName(this, tr("Slides from Outline"), dir,
                                                          tr("Outlines (*.txt *.text *.md *.markdown *.rtf *.docx)") + QStringLiteral(";;") + tr("All Files (*)"));
        if (path.isEmpty()) return;
        Settings::get().setValue(QStringLiteral("dirs/open"), QFileInfo(path).absolutePath());
        insertOutlineFile(path);
    });
}

bool DeckWindow::insertOutlineFile(const QString &path)
{
    if (isReadOnly()) {
        refuseReadOnlyChange();
        return false;
    }
    readOutlineAsync(path, [this, path](bool ok, deck::Outline &outline, const QString &why) {
        if (!ok) {
            notice(tr("Slides from Outline"), tr("\"%1\" can't make slides.\n%2").arg(QFileInfo(path).fileName(), why), true);
            return;
        }
        if (isReadOnly()) {   // (it was made read-only while the file was read)
            refuseReadOnlyChange();
            return;
        }
        m_notes->commit();
        if (m_outline) m_outline->commit();
        m_ed->endTextEdit();
        m_ed->beginChange(tr("Slides from Outline"));
        const int first = m_ed->currentPage() + 1;
        const int made = deck::insertOutline(*m_ed->doc(), first, outline);
        m_ed->endChange();
        if (made > 0) m_ed->setCurrentPage(first);
        if (!outline.notes.isEmpty()) notice(tr("Slides from Outline"), tr("Some of \"%1\" was left out.\n\n%2").arg(QFileInfo(path).fileName(), outline.notes.join(QLatin1Char('\n'))), true);
        else statusBar()->showMessage(tr("Added %n slide(s) from \"%1\".", nullptr, made).arg(QFileInfo(path).fileName()), 6000);
    });
    return true;
}

void DeckWindow::finishOutlineOpen(const QString &path, OpenHow how, bool ok, deck::Outline &outline, const QString &why)
{
    const QString name = QFileInfo(path).fileName();
    std::unique_ptr<Document> doc;
    if (ok) doc = deck::presentationFromOutline(outline);
    if (!doc) {
        const QString text = tr("%1 can't open \"%2\".\n%3").arg(appInfo().name, name, why);
        if (m_notifier) m_notifier(tr("Open"), text);
        else jp::plainWarning(this, tr("Open"), text);
        return;
    }
    // A new presentation nobody has touched gives way; any other stays. The slides are a new presentation that has no file yet (Save asks where); the file is never written.
    m_notes->commit();
    if (m_outline) m_outline->commit();
    DeckWindow *target = this;
    if (m_ed->isModified() || !m_ed->filePath().isEmpty() || how == OpenHow::NewWindow) {
        target = new DeckWindow();
        target->setAttribute(Qt::WA_DeleteOnClose);
        target->show();
    }
    target->m_ed->endTextEdit();
    target->setReadOnly(false);
    target->m_ed->setDocument(std::move(doc));
    target->m_ed->setUntitledName(QFileInfo(path).completeBaseName());
    target->m_ed->markUnsaved();
    target->m_fileProtected = false;
    target->m_versions.reset(QString());
    target->updateTitle();
    target->m_canvas->zoomToFit(Canvas::Fit::WholePage);
    telemetry::count(QStringLiteral("file.open.") + fileKind(path));
    hideFileScreen();
    if (!outline.notes.isEmpty()) {
        const QString text = tr("Some of \"%1\" couldn't be read and was left out.\n\n%2").arg(name, outline.notes.join(QLatin1Char('\n')));
        if (m_notifier) m_notifier(tr("Open"), text);
        else jp::plainWarning(target, tr("Open"), text);
    }
}

bool DeckWindow::saveOutlineCopy(const QString &path)
{
    if (!m_ed->filePath().isEmpty() && sameFile(path, m_ed->filePath())) {
        notice(tr("Save"), tr("A copy can't be saved over \"%1\". Choose another name.").arg(QFileInfo(path).fileName()), true);
        return false;
    }
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    QStringList left;
    const QByteArray bytes = deck::outlineMarkdown(*m_ed->doc(), &left).toUtf8();
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size() || !f.commit()) {
        notice(tr("Save a Copy"), tr("%1 couldn't save \"%2\".").arg(appInfo().name, QFileInfo(path).fileName()), true);
        return false;
    }
    notice(tr("Save a Copy"), tr("Saved the titles and text of the slides in \"%1\". It does not hold:\n\n%2").arg(QFileInfo(path).fileName(), left.join(QLatin1Char('\n'))), false);
    return true;
}

} // namespace jp
