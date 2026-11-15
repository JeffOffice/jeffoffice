#include "app/normaltemplateui.h"

#include "app/plaintext.h"
#include "core/normaltemplate.h"

#include <QCoreApplication>

namespace jp::write {

namespace {
std::function<bool()> g_askReplace;
std::function<void(const QString &)> g_showError;
} // namespace

void setNormalTemplatePrompts(const std::function<bool()> &askReplace, const std::function<void(const QString &)> &showError)
{
    g_askReplace = askReplace;
    g_showError = showError;
}

bool setNormalTemplate(QWidget *parent, const std::function<void(WriteDoc &)> &change)
{
    bool replace = false;
    if (normalTemplateState() == NormalTemplateState::Unreadable) {
        replace = g_askReplace ? g_askReplace()
                               : plainQuestion(parent, QCoreApplication::translate("jp::write::NormalTemplate", "Set as Default"),
                                               QCoreApplication::translate("jp::write::NormalTemplate", "Normal template can't be read. Replace it with a new one?")) == QMessageBox::Yes;
        if (!replace) return false;
    }
    QString error;
    if (changeNormalTemplate(change, &error, replace)) return true;
    const QString text = QCoreApplication::translate("jp::write::NormalTemplate", "couldn't save the default: %1").arg(error);
    if (g_showError) g_showError(text);
    else plainWarning(parent, QCoreApplication::translate("jp::write::NormalTemplate", "Set as Default"), text);
    return false;
}

} // namespace jp::write
