// JeffWrite's Review tab: Track Changes Options, Lock Tracking, and Restrict Editing (the document's protection), and how a protected document is held.

#include "app/writewindow.h"

#include "app/homedialogs.h"
#include "app/pageview.h"
#include "app/settings.h"
#include "core/reviewops.h"

#include <QAction>
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
    return m_protectKind == QLatin1String("comments") && allowedWithComments(id);
}

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
    mk("review.lockTracking", tr("Lock Tracking…"), "lock", QKeySequence(), [this] { restrictEditing(true); });
    mk("review.restrictEditing", tr("Restrict Editing…"), "shield", QKeySequence(), [this] { restrictEditing(false); });
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
        if (!office::wml::documentProtectionAccepts(*now, password)) {
            statusBar()->showMessage(now->hasPassword() && password.isEmpty() ? tr("Type the password to stop the protection.") : tr("That is not the password."), 6000);
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
