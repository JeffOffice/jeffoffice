#include "app/passworddialogs.h"

#include "app/plaintext.h"
#include "io/crypto.h"
#include "office/officecrypt.h"
#include "office/package.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QPointer>
#include <QProgressDialog>
#include <QThread>
#include <QTimer>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace jp {

namespace {

std::function<NewPasswordChoice(bool, QString *)> g_newStandIn;
std::function<bool(const QString &, bool, QString *)> g_openStandIn;


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
    QString password() const { return m_password->text(); }
    bool removed() const { return m_removed; }
    void clear()
    {
        m_password->clear();
        m_again->clear();
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

} // namespace


NewPasswordChoice askNewFilePassword(QWidget *parent, bool hasPassword, QString *password)
{
    if (g_newStandIn) return g_newStandIn(hasPassword, password);
    NewPasswordDialog dialog(hasPassword, parent);
    if (dialog.exec() != QDialog::Accepted) return NewPasswordChoice::Cancelled;
    const bool removed = dialog.removed();
    if (!removed) *password = dialog.password();
    dialog.clear();
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
    if (dialog.exec() != QDialog::Accepted) return false;
    *password = edit->text();
    edit->clear();
    return true;
}

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
