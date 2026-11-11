#include "app/privacy.h"

#include "app/settings.h"

#include <QCheckBox>
#include <QCoreApplication>

namespace jp::privacy {

bool removeOnSave()
{
    Settings &s = Settings::get();
    const QVariant v = s.value(QStringLiteral("privacy/removeOnSave"));
    if (v.isValid()) return v.toBool();
    return s.value(QStringLiteral("crunch/removePersonalOnSave"), false).toBool();
}

void setRemoveOnSave(bool on) { Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), on); }

QString optionText() { return QCoreApplication::translate("jp::Privacy", "Remove personal information from this file when saving"); }

QString optionDetail()
{
    return QCoreApplication::translate("jp::Privacy", "Takes out the author, the person who saved the file last, the company, the manager, and the names on comments and tracked changes. What the file says stays as it is.");
}

QCheckBox *makeOption(QWidget *parent, const QString &objectName)
{
    auto *box = new QCheckBox(optionText(), parent);
    box->setObjectName(objectName);
    box->setChecked(removeOnSave());
    return box;
}

} // namespace jp::privacy
