// JeffWrite's Review tab: Track Changes Options, Lock Tracking, and Restrict Editing (the document's protection), and how a protected document is held.

#include "app/writewindow.h"

#include "app/homedialogs.h"
#include "app/pageview.h"
#include "app/settings.h"
#include "core/reviewops.h"

#include "core/appinfo.h"
#include "office/package.h"

#include <QAction>
#include <QApplication>
#include <QEventLoop>
#include <QFileInfo>
#include <QProgressDialog>
#include <QThread>
#include <QTimer>
#include <atomic>
#include <QStatusBar>

namespace jp {

namespace {

// The commands a document protected for comments only still lets be used, besides those a read-only document has.
bool allowedWithComments(const QString &id)
{
    static const QSet<QString> ids = {QStringLiteral("review.newComment"), QStringLiteral("review.deleteComment"), QStringLiteral("review.replyComment"), QStringLiteral("review.resolveComment"),
                                      QStringLiteral("review.show.comments"), QStringLiteral("review.deleteAllComments")};
    return ids.contains(id);
}

} // namespace

bool WriteWindow::protectionAllows(const QString &id) const
{
    // The boxes that stop a protection are always there to be used in a window it holds read only.
    if (m_readOnlyByProtection && (id == QLatin1String("review.restrictEditing") || id == QLatin1String("review.lockTracking"))) return true;
    return m_protectKind == QLatin1String("comments") && allowedWithComments(id);
}

namespace {

// The check of a password against the protection's hash, which takes a moment for a file that asks for many rounds: done in a thread of its own, with a box that can be canceled when it takes long.
office::wml::PasswordCheck checkWithProgress(QWidget *parent, const office::wml::DocumentProtection &protection, const QString &password)
{
    std::atomic<bool> cancel{false};
    office::wml::PasswordCheck result = office::wml::PasswordCheck::Canceled;
    QThread *worker = QThread::create([&] { result = office::wml::checkDocumentPassword(protection, password, &cancel); });
    QProgressDialog box(QCoreApplication::translate("jp::WriteWindow", "Checking the password…"), QCoreApplication::translate("jp::WriteWindow", "Cancel"), 0, 0, parent);
    box.setWindowModality(Qt::WindowModal);
    box.setMinimumDuration(300);
    QObject::connect(&box, &QProgressDialog::canceled, &box, [&] { cancel = true; });
    QEventLoop loop;
    QTimer poll;
    QObject::connect(&poll, &QTimer::timeout, &loop, [&] {
        if (worker->isFinished()) loop.quit();
    });
    worker->start();
    poll.start(15);
    loop.exec();
    worker->wait();
    delete worker;
    return result;
}

} // namespace

void WriteWindow::createProtectActions()
{
    mk("review.trackOptions", tr("Change Tracking Options…"), "settings", QKeySequence(), [this] {
        QString name = Settings::get().userName(), initials = Settings::get().value(QStringLiteral("user/initials")).toString();
        if (!write::trackOptionsDialog(this, &name, &initials)) return;
        Settings::get().setValue(QStringLiteral("user/name"), name);
        Settings::get().setValue(QStringLiteral("user/initials"), initials);
        if (m_doc) m_doc->setReviewer(Settings::get().userName(), initials);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("review.compare", tr("Compare…"), "git-compare-arrows", QKeySequence(), [this] {
        if (!m_doc) return;
        QString path;
        write::CompareOptions options;
        options.author = Settings::get().userName();
        if (!write::compareDialog(this, &path, &options)) return;
        compareWith(path, options);
    });
    mk("review.lockTracking", tr("Lock Tracking…"), "lock", QKeySequence(), [this] { restrictEditing(true); });
    mk("review.restrictEditing", tr("Restrict Editing…"), "shield", QKeySequence(), [this] { restrictEditing(false); });
}

int WriteWindow::compareWith(const QString &revisedPath, const write::CompareOptions &options)
{
    if (!m_doc) return -1;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QByteArray bytes;
    QString err;
    std::unique_ptr<write::WriteDoc> revised;
    if (office::readWholeFile(revisedPath, &bytes, &err) && !bytes.isEmpty()) revised = write::WriteDoc::fromFile(bytes, &err, nullptr);
    std::unique_ptr<write::WriteDoc> result;
    int changes = 0;
    bool canceled = false;
    if (revised) {
        QProgressDialog box(tr("Comparing…"), tr("Cancel"), 0, 100, this);
        box.setWindowModality(Qt::WindowModal);
        box.setMinimumDuration(400);
        box.setAutoClose(false);
        box.setAutoReset(false);
        result = write::compareDocuments(*m_doc, *revised, options, &changes, &err, [&](int done, int total) {
            box.setValue(total > 0 ? std::min(99, done * 100 / total) : 0);
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);
            if (box.wasCanceled()) canceled = true;
            return !canceled;
        });
        box.reset();
    }
    QApplication::restoreOverrideCursor();
    if (canceled) {
        statusBar()->showMessage(tr("Compare was canceled."), 5000);
        return -1;
    }
    if (!result) {
        notice(tr("Compare"), tr("\"%1\" can't be compared.\n%2").arg(QFileInfo(revisedPath).fileName(), err), true);
        return -1;
    }
    auto *target = new WriteWindow();
    target->setAttribute(Qt::WA_DeleteOnClose);
    target->show();
    target->m_untitled = tr("Compare of %1 and %2").arg(displayName(), QFileInfo(revisedPath).completeBaseName());
    target->setDocument(std::move(result), QString());
    target->m_doc->setModified(true);
    target->statusBar()->showMessage(changes == 1 ? tr("1 change found.") : tr("%1 changes found.").arg(changes), 8000);
    return changes;
}

void WriteWindow::restrictEditing(bool lockTracking)
{
    if (!m_doc) return;
    const std::optional<office::wml::DocumentProtection> now = m_doc->protection();
    const bool enforced = now && now->enforced;
    QString edit, password;
    const int answer = write::restrictEditingDialog(this, enforced, now ? now->edit : QString(), now && now->hasPassword(), lockTracking, &edit, &password);
    if (answer == 1) {
        m_doc->setProtection(office::wml::makeDocumentProtection(edit, password));
        statusBar()->showMessage(tr("Protection is on."), 5000);
    } else if (answer == 2) {
        const office::wml::PasswordCheck check = now->hasPassword() && password.isEmpty() ? office::wml::PasswordCheck::Wrong : checkWithProgress(this, *now, password);
        if (check == office::wml::PasswordCheck::Canceled) return;
        if (check == office::wml::PasswordCheck::Unsupported) {
            statusBar()->showMessage(tr("This document's password uses a form %1 can't check, so the protection can't be stopped here.").arg(appInfo().name), 9000);
            return;
        }
        if (check == office::wml::PasswordCheck::Wrong) {
            statusBar()->showMessage(password.isEmpty() ? tr("Type the password to stop the protection.") : tr("That is not the password."), 6000);
            return;
        }
        m_doc->setProtection(std::nullopt);
        statusBar()->showMessage(tr("Protection is off."), 5000);
    } else {
        return;
    }
    applyProtection();
    m_view->setFocus(Qt::OtherFocusReason);
}

// A protected document is held as its protection says: read only, only comments, or with tracking locked on.
void WriteWindow::applyProtection()
{
    const std::optional<office::wml::DocumentProtection> p = m_doc ? m_doc->protection() : std::nullopt;
    const QString kind = p && p->enforced ? p->edit : QString();
    const bool holds = kind == QLatin1String("readOnly") || kind == QLatin1String("comments") || kind == QLatin1String("forms");
    m_protectKind = holds || kind == QLatin1String("trackedChanges") ? kind : QString();
    if (holds && !isReadOnly()) {
        m_readOnlyByProtection = true;
        setReadOnly(true);
    } else if (!holds && m_readOnlyByProtection) {
        m_readOnlyByProtection = false;
        setReadOnly(false);
    }
    if (kind == QLatin1String("trackedChanges") && m_doc && !m_doc->trackChanges()) m_doc->setTrackChanges(true);
    refreshReviewUi();
    if (kind != m_titledKind) updateTitle();
    m_titledKind = kind;
}

} // namespace jp
