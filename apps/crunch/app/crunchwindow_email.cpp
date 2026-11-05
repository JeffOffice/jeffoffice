// Make an Email: the workbook, or the workbook as a PDF, in a message file for an email program. The message is written by libs/shell/app/emailops.cpp.

#include "app/crunchwindow.h"

#include "app/emailops.h"
#include "app/filedialogs.h"
#include "app/plaintext.h"
#include "core/appinfo.h"
#include "core/printjob.h"
#include "core/sheetobjects.h"
#include "io/crypto.h"
#include "office/officecrypt.h"
#include "sheet/xlsxsave.h"

#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTemporaryDir>
#include <QVBoxLayout>

namespace jp {

bool CrunchWindow::exportEmail(EmailWhat what)
{
    if (!m_session || !m_loaded || !m_loaded->book || m_saving || !commitEdit()) return false;
    if (m_loading || !m_haveCalc) {
        notice(tr("Email"), tr("The workbook is still being read. Make the message when the status bar says Ready."), true);
        return false;
    }
    const QString name = m_path.isEmpty() ? displayName() : QFileInfo(m_path).completeBaseName();
    QString err;
    QByteArray bytes;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (what == EmailWhat::Workbook) {
        syncPanes();
        crunch::EditSession::Snapshot snapshot;
        if (m_session->takeSnapshot(&snapshot, &err, false)) {
            if (m_objects) m_objects->addToSave(&snapshot.changes);
            office::sheet::SaveReport report;
            if (office::sheet::saveXlsx(*m_loaded->book, snapshot.changes, &bytes, &err, &report)) {
                // (a workbook with a password is sent with it)
                if (m_fileProtected) bytes = office::encryptPackage(bytes, m_filePassword.isEmpty() ? office::defaultPassword() : m_filePassword);
            } else {
                bytes.clear();
            }
        }
    } else {
        QTemporaryDir dir;
        crunch::PrintJobOptions options = m_printChoices->job;
        options.what = crunch::PrintWhat::Workbook;
        const QString path = dir.filePath(QStringLiteral("workbook.pdf"));
        if (dir.isValid() && exportPdfTo(path, options, QString(), &err)) {
            QFile f(path);
            if (f.open(QIODevice::ReadOnly)) bytes = f.readAll();
        }
    }
    QApplication::restoreOverrideCursor();
    if (bytes.isEmpty()) {
        jp::plainWarning(this, tr("Email"), tr("%1 couldn't make the message.\n%2").arg(appInfo().name, err));
        return false;
    }
    QVector<write::EmailPart> attachments;
    QString text;
    if (what == EmailWhat::Workbook) {
        attachments.push_back({name + QStringLiteral(".xlsx"), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"), bytes, QString()});
        text = tr("The workbook \"%1\" is attached.").arg(name);
    } else {
        attachments.push_back({name + QStringLiteral(".pdf"), QStringLiteral("application/pdf"), bytes, QString()});
        text = tr("The workbook \"%1\" is attached as a PDF.").arg(name);
    }
    const office::PackageProps props = properties();
    const QString subject = props.title.isEmpty() ? name : props.title;
    const QString dir = m_path.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(m_path).absolutePath();
    const QString filter = tr("Email Message (*.eml)");
    const QString suggested = dir + QLatin1Char('/') + name + QStringLiteral(".eml");
    QString path = askForSavePath(tr("Save Email"), suggested, filter);
    if (path.isEmpty()) return false;
    if (QFileInfo(path).suffix().compare(QLatin1String("eml"), Qt::CaseInsensitive) != 0) path += QStringLiteral(".eml");
    const QByteArray message = write::emailMessage(subject, text, attachments);
    QSaveFile file(path);
    if (!(file.open(QIODevice::WriteOnly) && file.write(message) == message.size() && file.commit())) {
        err = file.errorString();
        jp::plainWarning(this, tr("Email"), tr("%1 couldn't save \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
        return false;
    }
    statusBar()->showMessage(tr("Saved %1. Open it in your email program, add who it is for, and send it.").arg(QFileInfo(path).fileName()), 8000);
    return true;
}

bool CrunchWindow::exportEmailAsk()
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
    what->addItem(tr("The workbook, as a file"), int(EmailWhat::Workbook));
    what->addItem(tr("The workbook, as a PDF"), int(EmailWhat::Pdf));
    layout->addWidget(what);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted) return false;
    return exportEmail(static_cast<EmailWhat>(what->currentData().toInt()));
}

} // namespace jp
