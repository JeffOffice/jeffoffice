// Developer tool: saves pictures of the shared dialog pieces (message boxes, buttons) as PNGs, for looking at the style (docs/design/STYLE.md).
//   jo_uigrab <folder> [light|dark]
#include "app/icons.h"
#include "app/plaintext.h"
#include "app/theme.h"
#include "app/uistyle.h"
#include "../tests/isolatesettings.h"

#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QPushButton>

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
    return 0;
}
