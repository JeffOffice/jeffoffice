// Make an Email: the presentation, or the presentation as a PDF, in a message file for an email program. The message is written by libs/shell/app/emailops.cpp.

#include "app/deckwindow.h"

#include "app/editor.h"
#include "app/emailops.h"
#include "app/filedialogs.h"
#include "app/notespane.h"
#include "app/pdfprotection.h"
#include "app/plaintext.h"
#include "app/privacy.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "io/crypto.h"
#include "io/deckoutput.h"
#include "io/presentationfile.h"

#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QSaveFile>
#include <QScopeGuard>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTemporaryDir>
#include <QVBoxLayout>

namespace jp {

bool DeckWindow::exportEmail(EmailWhat what, const PdfProtection &protect)
{
    if (!m_ed->doc() || m_ed->doc()->pages.isEmpty()) return false;
    m_notes->commit();
    m_ed->endTextEdit();
    const QString name = m_ed->displayName();
    QString err;
    QByteArray bytes;
    {
        // A folder of its own, so that nothing is left in the shared temporary folder (and, with a password, the only file made is the protected one).
        QTemporaryDir dir;
        QApplication::setOverrideCursor(Qt::WaitCursor);
        if (dir.isValid() && what == EmailWhat::Presentation) {
            const QString path = dir.filePath(QStringLiteral("presentation.pptx"));
            QStringList skipped;
            // (a presentation with a password stays locked with it; the password is never in the message)
            if (deck::saveAnyPresentationFile(*m_ed->doc(), path, &err, &skipped, Settings::get().value(QStringLiteral("save/embedFonts"), true).toBool(), m_fileProtected ? m_filePassword : QString(),
                                           privacy::removeOnSave())) {
                QFile f(path);
                if (f.open(QIODevice::ReadOnly)) bytes = f.readAll();
            }
        } else if (dir.isValid()) {
            const QString path = dir.filePath(QStringLiteral("presentation.pdf"));
            if (deck::exportPdf(*m_ed->doc(), path, deck::OutputOptions(), &err, &protect)) {
                QFile f(path);
                if (f.open(QIODevice::ReadOnly)) bytes = f.readAll();
            }
        }
        QApplication::restoreOverrideCursor();
    }
    if (bytes.isEmpty()) {
        jp::plainWarning(this, tr("Email"), tr("%1 couldn't make the message.\n%2").arg(appInfo().name, err));
        return false;
    }
    QVector<write::EmailPart> attachments;
    QString text;
    if (what == EmailWhat::Presentation) {
        attachments.push_back({name + QStringLiteral(".pptx"), QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation"), bytes, QString()});
        text = tr("The presentation \"%1\" is attached.").arg(name);
    } else {
        attachments.push_back({name + QStringLiteral(".pdf"), QStringLiteral("application/pdf"), bytes, QString()});
        text = tr("The presentation \"%1\" is attached as a PDF.").arg(name);
    }
    const QString title = properties().title;
    const QString subject = title.isEmpty() ? name : title;
    const QString file = m_ed->filePath();
    const QString dir = file.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(file).absolutePath();
    const QString filter = tr("Email Message (*.eml)");
    const QString suggested = dir + QLatin1Char('/') + name + QStringLiteral(".eml");
    QString path = m_pathChooser ? m_pathChooser(suggested, filter) : askSavePath(this, tr("Save Email"), suggested, filter);
    if (path.isEmpty()) return false;
    if (QFileInfo(path).suffix().compare(QLatin1String("eml"), Qt::CaseInsensitive) != 0) path += QStringLiteral(".eml");
    const QByteArray message = write::emailMessage(subject, text, attachments);
    QSaveFile out(path);
    if (!(out.open(QIODevice::WriteOnly) && out.write(message) == message.size() && out.commit())) {
        err = out.errorString();
        jp::plainWarning(this, tr("Email"), tr("%1 couldn't save \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
        return false;
    }
    statusBar()->showMessage(tr("Saved %1. Open it in your email program, add who it is for, and send it.").arg(QFileInfo(path).fileName()), 8000);
    return true;
}

bool DeckWindow::exportEmailAsk(const PdfProtection &fromPage)
{
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("emailDialog"));
    dialog.setWindowTitle(tr("Make an Email"));
    auto *layout = new QVBoxLayout(&dialog);
    auto *intro = new QLabel(tr("This makes a message file. Nothing is sent. Open the file in your email program, add who it is for, and send it."), &dialog);
    intro->setWordWrap(true);
    intro->setTextFormat(Qt::PlainText);
    layout->addWidget(intro);
    auto *what = new QComboBox(&dialog);
    what->setObjectName(QStringLiteral("emailWhat"));
    what->setAccessibleName(tr("What the message holds"));
    what->addItem(tr("The presentation, as a file"), int(EmailWhat::Presentation));
    what->addItem(tr("The presentation, as a PDF"), int(EmailWhat::Pdf));
    layout->addWidget(what);
    // A password for a PDF (the presentation as a file keeps the password of its own; see exportEmail). It starts as the Export page's, if one was set there.
    auto *protect = new PdfProtectionRow(&dialog);
    protect->setProtection(fromPage);
    layout->addWidget(protect);
    auto syncProtect = [what, protect] { protect->setEnabled(what->currentData().toInt() == int(EmailWhat::Pdf)); };
    connect(what, &QComboBox::currentIndexChanged, &dialog, syncProtect);
    syncProtect();
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted) return false;
    const EmailWhat chosen = static_cast<EmailWhat>(what->currentData().toInt());
    PdfProtection locked = chosen == EmailWhat::Pdf ? protect->protection() : PdfProtection();
    const auto wipe = qScopeGuard([&locked] { locked.wipe(); });
    return exportEmail(chosen, locked);
}

} // namespace jp
