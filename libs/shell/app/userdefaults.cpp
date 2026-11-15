#include "app/userdefaults.h"

#include "app/settings.h"

#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>

namespace jp {

namespace {
std::function<DefaultScope(const QString &)> g_standIn;
}

void setDefaultScopeStandIn(const std::function<DefaultScope(const QString &what)> &standIn) { g_standIn = standIn; }

DefaultScope askDefaultScope(QWidget *parent, const QString &what)
{
    if (g_standIn) return g_standIn(what);
    QMessageBox box(QMessageBox::Question, QCoreApplication::translate("Dialogs", "Set as Default"),
                    QCoreApplication::translate("Dialogs", "Make %1 the default for this document only, or for all new documents?").arg(what), QMessageBox::NoButton, parent);
    box.setTextFormat(Qt::PlainText);
    auto *thisOne = box.addButton(QCoreApplication::translate("Dialogs", "This Document Only"), QMessageBox::AcceptRole);
    thisOne->setObjectName(QStringLiteral("defaultThisDocument"));
    auto *all = box.addButton(QCoreApplication::translate("Dialogs", "All New Documents"), QMessageBox::AcceptRole);
    all->setObjectName(QStringLiteral("defaultAllNew"));
    box.addButton(QMessageBox::Cancel);
    box.setObjectName(QStringLiteral("setAsDefaultBox"));
    box.exec();
    if (box.clickedButton() == thisOne) return DefaultScope::ThisDocument;
    if (box.clickedButton() == all) return DefaultScope::AllNew;
    return DefaultScope::Cancel;
}

QString userDefaultsFolder()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/defaults");
    QDir().mkpath(dir);
    return dir;
}

QString userDefaultFile(const QString &name) { return userDefaultsFolder() + QLatin1Char('/') + name; }

QJsonObject userDefault(const QString &name)
{
    return QJsonDocument::fromJson(Settings::get().value(QStringLiteral("defaults/") + name).toString().toUtf8()).object();
}

void setUserDefault(const QString &name, const QJsonObject &choices)
{
    Settings::get().setValue(QStringLiteral("defaults/") + name, QString::fromUtf8(QJsonDocument(choices).toJson(QJsonDocument::Compact)));
}

void clearUserDefault(const QString &name) { Settings::get().setValue(QStringLiteral("defaults/") + name, QString()); }

} // namespace jp
