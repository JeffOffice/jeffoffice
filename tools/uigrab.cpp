// Developer tool: saves pictures of the shared dialog pieces (message boxes, buttons) as PNGs, for looking at the style (docs/design/STYLE.md).
//   jo_uigrab <folder> [light|dark]
#include "app/icons.h"
#include "app/lossywarning.h"
#include "app/pdfprotection.h"
#include "app/plaintext.h"
#include "app/theme.h"
#include "app/uistyle.h"
#include "../tests/isolatesettings.h"

#include <QApplication>
#include <QDialog>
#include <QDir>
#include <functional>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    if (argc < 2) return 2;
    const QString dir = QString::fromLocal8Bit(argv[1]);
    const bool dark = argc > 2 && QByteArray(argv[2]) == "dark";
    QDir().mkpath(dir);
    jp::applyUiTheme(dark ? 2 : 1);
    const QString tag = dark ? QStringLiteral("-dark") : QString();
    auto grab = [&](QMessageBox &box, const QString &name) {
        box.setTextFormat(Qt::PlainText);
        box.show();
        app.processEvents();
        box.grab().save(QDir(dir).filePath(name + tag + QStringLiteral(".png")));
    };
    {
        jp::MessageBox b(QMessageBox::Warning, QStringLiteral("JeffWrite"), QStringLiteral("The file could not be saved. Check that the folder can be written to, then try again."), QMessageBox::Ok);
        grab(b, QStringLiteral("box-warning"));
    }
    {
        jp::MessageBox b(QMessageBox::Critical, QStringLiteral("JeffWrite"), QStringLiteral("This file is damaged and cannot be opened."), QMessageBox::Ok);
        grab(b, QStringLiteral("box-critical"));
    }
    {
        jp::MessageBox b(QMessageBox::Information, QStringLiteral("JeffCrunch"), QStringLiteral("Nothing was found. Try a different word."), QMessageBox::Ok);
        grab(b, QStringLiteral("box-information"));
    }
    {
        jp::MessageBox b(QMessageBox::Question, QStringLiteral("JeffCrunch"), QStringLiteral("Replace the existing data? The cells below will be overwritten."), QMessageBox::Yes | QMessageBox::No, nullptr);
        b.setDefaultButton(QMessageBox::Yes);
        grab(b, QStringLiteral("box-question"));
    }
    {
        std::unique_ptr<jp::MessageBox> b(jp::saveChangesBox(nullptr, QStringLiteral("Budget 2027.jcrunch")));
        grab(*b, QStringLiteral("box-save-changes"));
    }
    // boxes the program opens by itself: found while they are open, pictured, and closed
    auto openAndGrab = [&](const QString &name, const std::function<void()> &open) {
        QTimer poll;
        QObject::connect(&poll, &QTimer::timeout, [&] {
            QWidget *m = QApplication::activeModalWidget();
            if (!m) return;
            poll.stop();
            QTimer::singleShot(250, [&, m] {
                m->grab().save(QDir(dir).filePath(name + tag + QStringLiteral(".png")));
                if (auto *d = qobject_cast<QDialog *>(m)) d->reject();
            });
        });
        poll.start(20);
        open();
    };
    openAndGrab(QStringLiteral("lossy-save"), [&] {
        jp::LossyAsk ask;
        ask.path = QStringLiteral("/tmp/talk.odp");
        ask.what = QStringLiteral("presentation");
        ask.format = QStringLiteral(".odp");
        ask.ownFormat = QStringLiteral(".pptx");
        ask.lost = {QStringLiteral("animations that repeat"), QStringLiteral("sound on a slide"), QStringLiteral("the fonts inside the file")};
        jp::askLossy(nullptr, ask);
    });
    openAndGrab(QStringLiteral("pdf-protection"), [&] {
        jp::PdfProtectionRow row;
        QPushButton *b = row.findChild<QPushButton *>();
        if (b) b->click();
    });
    return 0;
}
