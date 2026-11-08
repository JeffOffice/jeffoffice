#include "app/passworddialogs.h"

#include "app/plaintext.h"
#include "io/crypto.h"
#include "office/officecrypt.h"
#include "office/package.h"
#include "render/pdfpage.h"

#include <memory>

#include <QCoreApplication>
#include <QEventLoop>
#include <QPointer>
#include <QProgressDialog>
#include <QScopeGuard>
#include <QThread>
#include <QTimer>
#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace jp {

namespace {

std::function<NewPasswordChoice(bool, QString *)> g_newStandIn;
std::function<bool(const QString &, bool, QString *)> g_openStandIn;
std::function<NewPasswordChoice(bool, PdfProtection *)> g_pdfStandIn;

// A copy of what a box holds that shares nothing with the box, for the caller to wipe.
QString taken(const QLineEdit *edit)
{
    const QString text = edit->text();
    return QString(text.constData(), text.size());
}

// Overwrites what a box holds (with spaces of the same length, so that the box's own buffer is written over), then empties it.
void zero(QLineEdit *edit)
{
    edit->setText(QString(edit->text().size(), QLatin1Char(' ')));
    edit->clear();
}


class NewPasswordDialog : public QDialog {
    Q_DECLARE_TR_FUNCTIONS(NewPasswordDialog)

public:
    NewPasswordDialog(bool hasPassword, QWidget *parent) : QDialog(parent)
    {
        setObjectName(QStringLiteral("newPasswordDialog"));
        setWindowTitle(tr("Encrypt with Password"));
        auto *layout = new QVBoxLayout(this);
        auto *intro = new QLabel(hasPassword ? tr("This file is protected with a password. Type a new one to change it, or remove the password.")
                                             : tr("Anyone who opens this file will have to type the password. The password protects the file when you save it."), this);
        intro->setWordWrap(true);
        layout->addWidget(intro);
        auto *form = new QFormLayout;
        m_password = new QLineEdit(this);
        m_password->setObjectName(QStringLiteral("newPassword"));
        m_password->setAccessibleName(tr("Password"));
        m_password->setEchoMode(QLineEdit::Password);
        form->addRow(tr("Password:"), m_password);
        m_again = new QLineEdit(this);
        m_again->setObjectName(QStringLiteral("newPasswordAgain"));
        m_again->setAccessibleName(tr("Password again"));
        m_again->setEchoMode(QLineEdit::Password);
        form->addRow(tr("Type it again:"), m_again);
        layout->addLayout(form);
        auto *warn = new QLabel(tr("A password can't be found again if it is forgotten. Keep it somewhere safe."), this);
        warn->setWordWrap(true);
        layout->addWidget(warn);
        m_problem = plainLabel(new QLabel(this));
        m_problem->setObjectName(QStringLiteral("newPasswordProblem"));
        m_problem->setWordWrap(true);
        layout->addWidget(m_problem);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        buttons->button(QDialogButtonBox::Ok)->setText(tr("Protect"));
        if (hasPassword) {
            QPushButton *remove = buttons->addButton(tr("Remove Password"), QDialogButtonBox::DestructiveRole);
            connect(remove, &QPushButton::clicked, this, [this] {
                m_removed = true;
                QDialog::accept();
            });
        }
        connect(buttons, &QDialogButtonBox::accepted, this, &NewPasswordDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttons);
    }
    QString password() const { return taken(m_password); }
    bool removed() const { return m_removed; }
    void clear()
    {
        zero(m_password);
        zero(m_again);
    }

private:
    void accept() override
    {
        if (m_password->text().isEmpty()) {
            m_problem->setText(tr("Type a password, or cancel."));
            return;
        }
        if (m_password->text() != m_again->text()) {
            m_problem->setText(tr("The two passwords are not the same."));
            return;
        }
        QDialog::accept();
    }
    QLineEdit *m_password = nullptr, *m_again = nullptr;
    QLabel *m_problem = nullptr;
    bool m_removed = false;
};

class PdfProtectionDialog : public QDialog {
    Q_DECLARE_TR_FUNCTIONS(PdfProtectionDialog)

public:
    PdfProtectionDialog(bool hasPassword, const PdfProtection &now, QWidget *parent) : QDialog(parent)
    {
        setObjectName(QStringLiteral("pdfProtectionDialog"));
        setWindowTitle(tr("Encrypt with a Password"));
        auto *layout = new QVBoxLayout(this);
        auto *intro = new QLabel(tr("Anyone who opens this PDF will have to type the password. The password is used for this PDF only, and is not kept."), this);
        intro->setWordWrap(true);
        layout->addWidget(intro);
        auto *form = new QFormLayout;
        m_open = passwordBox(QStringLiteral("pdfPassword"), tr("Password"));
        form->addRow(tr("Password:"), m_open);
        m_openAgain = passwordBox(QStringLiteral("pdfPasswordAgain"), tr("Confirm password"));
        form->addRow(tr("Confirm:"), m_openAgain);
        layout->addLayout(form);

        m_restrict = new QGroupBox(tr("Restrict printing, copying, and editing"), this);
        m_restrict->setObjectName(QStringLiteral("pdfRestrict"));
        m_restrict->setCheckable(true);
        m_restrict->setChecked(now.limited);
        auto *inner = new QVBoxLayout(m_restrict);
        auto *innerForm = new QFormLayout;
        m_owner = passwordBox(QStringLiteral("pdfOwnerPassword"), tr("Password for the limits"));
        innerForm->addRow(tr("Password for the limits:"), m_owner);
        m_ownerAgain = passwordBox(QStringLiteral("pdfOwnerPasswordAgain"), tr("Confirm password for the limits"));
        innerForm->addRow(tr("Confirm:"), m_ownerAgain);
        inner->addLayout(innerForm);
        m_print = new QCheckBox(tr("Allow printing"), m_restrict);
        m_print->setObjectName(QStringLiteral("pdfAllowPrint"));
        m_print->setChecked(now.limited ? now.allowPrint : true);
        m_copy = new QCheckBox(tr("Allow copying text and pictures"), m_restrict);
        m_copy->setObjectName(QStringLiteral("pdfAllowCopy"));
        m_copy->setChecked(now.limited ? now.allowCopy : false);
        m_change = new QCheckBox(tr("Allow changing the document"), m_restrict);
        m_change->setObjectName(QStringLiteral("pdfAllowChange"));
        m_change->setChecked(now.limited ? now.allowChange : false);
        for (QCheckBox *box : {m_print, m_copy, m_change}) inner->addWidget(box);
        auto *limitNote = new QLabel(tr("These limits are a request. Most PDF readers honor them, but they do not stop anyone who wants to get around them. Only the password that opens the PDF really protects it."), m_restrict);
        limitNote->setWordWrap(true);
        inner->addWidget(limitNote);
        layout->addWidget(m_restrict);

        auto *warn = new QLabel(tr("A password can't be found again if it is forgotten. Keep it somewhere safe."), this);
        warn->setWordWrap(true);
        layout->addWidget(warn);
        m_problem = plainLabel(new QLabel(this));
        m_problem->setObjectName(QStringLiteral("pdfProtectionProblem"));
        m_problem->setWordWrap(true);
        layout->addWidget(m_problem);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        buttons->button(QDialogButtonBox::Ok)->setText(tr("Encrypt"));
        if (hasPassword) {
            QPushButton *remove = buttons->addButton(tr("Remove Password"), QDialogButtonBox::DestructiveRole);
            connect(remove, &QPushButton::clicked, this, [this] {
                m_removed = true;
                QDialog::accept();
            });
        }
        connect(buttons, &QDialogButtonBox::accepted, this, &PdfProtectionDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttons);
    }
    bool removed() const { return m_removed; }
    PdfProtection protection() const
    {
        PdfProtection p;
        p.openPassword = taken(m_open);
        p.limited = m_restrict->isChecked();
        if (p.limited) {
            p.ownerPassword = taken(m_owner);
            p.allowPrint = m_print->isChecked();
            p.allowCopy = m_copy->isChecked();
            p.allowChange = m_change->isChecked();
        }
        return p;
    }
    void clear()
    {
        for (QLineEdit *e : {m_open, m_openAgain, m_owner, m_ownerAgain}) zero(e);
    }

private:
    QLineEdit *passwordBox(const QString &name, const QString &accessibleName)
    {
        auto *edit = new QLineEdit(this);
        edit->setObjectName(name);
        edit->setAccessibleName(accessibleName);
        edit->setEchoMode(QLineEdit::Password);
        return edit;
    }
    void accept() override
    {
        // Compared as they will be written (normalized, and no longer than a PDF password can be), so that "e" and an accent typed apart are the same as "é".
        QString open, openAgain, owner, ownerAgain;
        const auto wipeAll = qScopeGuard([&] {
            for (QString *t : {&open, &openAgain, &owner, &ownerAgain}) crypto::wipe(*t);
        });
        if (m_open->text().isEmpty()) {
            m_problem->setText(tr("Type a password, or cancel."));
            return;
        }
        PdfPasswordProblem problem = normalizePdfPassword(m_open->text(), &open);
        if (problem == PdfPasswordProblem::None) problem = normalizePdfPassword(m_openAgain->text(), &openAgain);
        if (problem != PdfPasswordProblem::None) {
            m_problem->setText(pdfPasswordProblemText(problem));
            return;
        }
        if (open != openAgain) {
            m_problem->setText(tr("The two passwords are not the same."));
            return;
        }
        if (m_restrict->isChecked()) {
            if (m_owner->text().isEmpty()) {
                m_problem->setText(tr("Type a password for the limits, or turn the limits off."));
                return;
            }
            problem = normalizePdfPassword(m_owner->text(), &owner);
            if (problem == PdfPasswordProblem::None) problem = normalizePdfPassword(m_ownerAgain->text(), &ownerAgain);
            if (problem != PdfPasswordProblem::None) {
                m_problem->setText(pdfPasswordProblemText(problem));
                return;
            }
            if (owner != ownerAgain) {
                m_problem->setText(tr("The two passwords for the limits are not the same."));
                return;
            }
            if (owner == open) {
                m_problem->setText(tr("The password for the limits must not be the one that opens the PDF."));
                return;
            }
        }
        QDialog::accept();
    }
    QLineEdit *m_open = nullptr, *m_openAgain = nullptr, *m_owner = nullptr, *m_ownerAgain = nullptr;
    QGroupBox *m_restrict = nullptr;
    QCheckBox *m_print = nullptr, *m_copy = nullptr, *m_change = nullptr;
    QLabel *m_problem = nullptr;
    bool m_removed = false;
};

} // namespace


NewPasswordChoice askNewFilePassword(QWidget *parent, bool hasPassword, QString *password)
{
    if (g_newStandIn) return g_newStandIn(hasPassword, password);
    NewPasswordDialog dialog(hasPassword, parent);
    const bool accepted = dialog.exec() == QDialog::Accepted;
    const bool removed = accepted && dialog.removed();
    if (accepted && !removed) *password = dialog.password();
    dialog.clear();   // (on cancel too)
    if (!accepted) return NewPasswordChoice::Cancelled;
    return removed ? NewPasswordChoice::Removed : NewPasswordChoice::Set;
}

bool askOpenFilePassword(QWidget *parent, const QString &fileName, bool wrongBefore, QString *password)
{
    if (g_openStandIn) return g_openStandIn(fileName, wrongBefore, password);
    QDialog dialog(parent);
    dialog.setObjectName(QStringLiteral("openPasswordDialog"));
    dialog.setWindowTitle(QCoreApplication::translate("jp::PasswordDialogs", "Password"));
    auto *layout = new QVBoxLayout(&dialog);
    auto *intro = plainLabel(new QLabel(wrongBefore ? QCoreApplication::translate("jp::PasswordDialogs", "That password is not right. Type the password for \"%1\" again.").arg(fileName) : QCoreApplication::translate("jp::PasswordDialogs", "\"%1\" is protected with a password. Type it to open the file.").arg(fileName), &dialog));
    intro->setWordWrap(true);
    layout->addWidget(intro);
    auto *edit = new QLineEdit(&dialog);
    edit->setObjectName(QStringLiteral("openPassword"));
    edit->setAccessibleName(QCoreApplication::translate("jp::PasswordDialogs", "Password"));
    edit->setEchoMode(QLineEdit::Password);
    layout->addWidget(edit);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    const bool accepted = dialog.exec() == QDialog::Accepted;
    if (accepted) *password = taken(edit);
    zero(edit);   // (on cancel too)
    return accepted;
}

NewPasswordChoice askPdfProtection(QWidget *parent, bool hasPassword, PdfProtection *protection)
{
    if (g_pdfStandIn) return g_pdfStandIn(hasPassword, protection);
    PdfProtectionDialog dialog(hasPassword, *protection, parent);
    const bool accepted = dialog.exec() == QDialog::Accepted;
    const bool removed = accepted && dialog.removed();
    if (accepted && !removed) *protection = dialog.protection();
    dialog.clear();   // (on cancel too)
    if (!accepted) return NewPasswordChoice::Cancelled;
    return removed ? NewPasswordChoice::Removed : NewPasswordChoice::Set;
}

PdfUnlock unlockPdf(QWidget *parent, const QString &fileName, const QByteArray &bytes, QString *password)
{
    password->clear();
    bool wrongBefore = false;
    while (true) {
        // The password as the standard has it (normalized and cut at 127 bytes), so that one typed with the accent apart opens a file made with it joined; and, for a file
        // made by a program that did not normalize, as it was typed.
        QString tried = pdfOpeningPassword(*password);
        auto pdf = std::make_unique<PdfDocument>(bytes, tried);
        if (!pdf->isValid() && pdf->needsPassword() && !password->isEmpty() && tried != *password) {
            auto asTyped = std::make_unique<PdfDocument>(bytes, *password);
            if (asTyped->isValid()) {
                pdf = std::move(asTyped);
                crypto::wipe(tried);
                tried = QString(password->constData(), password->size());
            }
        }
        if (pdf->isValid()) {
            crypto::wipe(*password);
            *password = tried;
            return PdfUnlock::Opened;
        }
        crypto::wipe(tried);
        const bool wanted = pdf->needsPassword();
        // (the one that was wrong is wiped before the next is asked for)
        crypto::wipe(*password);
        if (!wanted) return PdfUnlock::Unreadable;
        // (a wrong password is asked after again, once for each answer the person gives)
        if (!askOpenFilePassword(parent, fileName, wrongBefore, password)) {
            crypto::wipe(*password);
            return PdfUnlock::Cancelled;
        }
        wrongBefore = true;
    }
}

void setPdfProtectionStandIn(const std::function<NewPasswordChoice(bool, PdfProtection *)> &standIn) { g_pdfStandIn = standIn; }
void setNewPasswordStandIn(const std::function<NewPasswordChoice(bool, QString *)> &standIn) { g_newStandIn = standIn; }
void setOpenPasswordStandIn(const std::function<bool(const QString &, bool, QString *)> &standIn) { g_openStandIn = standIn; }

namespace {
std::atomic<bool> *g_running = nullptr;
}

bool runWithCancel(QWidget *parent, const QString &text, const std::function<void(const std::atomic<bool> *cancel)> &work)
{
    // (one at a time: a second call, from an event handled meanwhile, would take the place of the first's flag)
    if (g_running) return false;
    std::atomic<bool> stop{false};
    g_running = &stop;
    // (the window takes no input until the note is up, and may be closed and deleted meanwhile)
    const QPointer<QWidget> guard(parent);
    const QPointer<QWidget> window(parent ? parent->window() : nullptr);
    const bool windowWasEnabled = window && window->isEnabled();
    if (window) window->setEnabled(false);
    QThread *thread = QThread::create([&] { work(&stop); });
    QEventLoop loop;
    QObject::connect(thread, &QThread::finished, &loop, &QEventLoop::quit);
    QPointer<QProgressDialog> note;
    QTimer later;
    later.setSingleShot(true);
    QObject::connect(&later, &QTimer::timeout, [&] {
        if (parent && !guard) return;
        if (window && windowWasEnabled) window->setEnabled(true);   // (the note is modal from here on)
        note = new QProgressDialog(text, QCoreApplication::translate("jp::PasswordDialogs", "Cancel"), 0, 0, parent);
        note->setObjectName(QStringLiteral("workNote"));
        note->setWindowModality(Qt::ApplicationModal);
        note->setAutoClose(false);
        note->setAutoReset(false);
        QObject::connect(note.data(), &QProgressDialog::canceled, [&] { stop = true; });
        note->show();
    });
    later.start(300);
    thread->start();
    // (the note is modal once it shows, so the window takes no input from then on, and its Cancel button takes the click)
    loop.exec();
    thread->wait();
    delete thread;
    if (note) delete note.data();
    if (window && windowWasEnabled) window->setEnabled(true);
    g_running = nullptr;
    return !stop.load();
}

void cancelRunningForTest()
{
    if (g_running) *g_running = true;
}

PackageUnlock unlockProtectedPackage(QWidget *parent, const QString &fileName, QByteArray *bytes, QString *kept, bool *wasProtected, QString *error)
{
    QByteArray info, package, zip;
    if (!office::splitProtectedPackage(*bytes, &info, &package)) return PackageUnlock::Opened;
    // (the file's own bytes are let go: the package inside is what is worked on, and a file of a gigabyte is not held twice more)
    *bytes = QByteArray();
    bytes->squeeze();
    // (the window may be closed, from its title bar, while a password is checked: nothing is asked of it or done to it after)
    const QPointer<QWidget> alive(parent);
    auto decrypt = [&](const QString &password, office::CryptResult *result) {
        const bool finished = runWithCancel(parent, QCoreApplication::translate("jp::PasswordDialogs", "Checking the password…"), [&](const std::atomic<bool> *cancel) {
            *result = office::decryptPackage(info, package, password, office::kMaxOpenFileBytes, false, &zip, [cancel] { return cancel->load(); });
        });
        if (!finished) *result = office::CryptResult::Cancelled;
    };
    office::CryptResult r = office::CryptResult::Unsupported;
    decrypt(office::defaultPassword(), &r);
    if (parent && !alive) return PackageUnlock::WindowGone;
    if (r == office::CryptResult::Ok) {
        *bytes = zip;
        return PackageUnlock::Opened;
    }
    bool wrong = false;
    for (;;) {
        QString password;
        if (!askOpenFilePassword(parent, fileName, wrong, &password)) return PackageUnlock::Cancelled;
        decrypt(password, &r);
        if (parent && !alive) {
            crypto::wipe(password);
            return PackageUnlock::WindowGone;
        }
        if (r == office::CryptResult::Ok) {
            *bytes = zip;
            *kept = password;
            *wasProtected = true;
            return PackageUnlock::Opened;
        }
        crypto::wipe(password);
        if (r == office::CryptResult::Cancelled) return PackageUnlock::Cancelled;
        if (r == office::CryptResult::WrongPassword) {
            wrong = true;
            continue;
        }
        *error = r == office::CryptResult::Unsupported ? QCoreApplication::translate("jp::PasswordDialogs", "This file is protected in a way this program doesn't read.")
               : r == office::CryptResult::TooLarge    ? QCoreApplication::translate("jp::PasswordDialogs", "The file is too large to open.")
               : r == office::CryptResult::TooCostly   ? QCoreApplication::translate("jp::PasswordDialogs", "The password protection of this file asks for more work than this program allows.")
                                                       : QCoreApplication::translate("jp::PasswordDialogs", "The file is damaged.");
        return PackageUnlock::Failed;
    }
}

} // namespace jp
