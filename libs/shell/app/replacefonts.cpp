#include "app/replacefonts.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QFormLayout>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QSet>
#include <algorithm>

namespace jp {

QString ReplaceFontsResult::message() const
{
    if (uses <= 0) return QString();
    return QCoreApplication::translate("jp::ReplaceFonts", "Replaced %1 with %2 (places changed: %3).").arg(from, to).arg(uses);
}

QVector<FontUse> fontUsesOf(const QMap<QString, int> &families)
{
    QVector<FontUse> out;
    for (auto it = families.constBegin(); it != families.constEnd(); ++it) out.push_back({it.key(), QString(), it.value(), false});
    std::stable_sort(out.begin(), out.end(), [](const FontUse &a, const FontUse &b) { return a.uses > b.uses; });
    return out;
}

ReplaceFontsResult askReplaceFonts(QWidget *parent, const ReplaceFontsHost &host)
{
    ReplaceFontsResult result;
    if (!host.fontsUsed || !host.replace) return result;
    const QVector<FontUse> used = host.fontsUsed();
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("replaceFontsDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::ReplaceFonts", "Replace Fonts"));
    auto *form = new QFormLayout(&dlg);
    auto *intro = new QLabel(QCoreApplication::translate("jp::ReplaceFonts", "Choose a font this document uses, and the font to use in its place everywhere."), &dlg);
    intro->setTextFormat(Qt::PlainText);
    intro->setWordWrap(true);
    form->addRow(intro);
    QSet<QString> installed;
    for (const QString &family : QFontDatabase::families()) installed.insert(family.toLower());
    auto *from = new QComboBox(&dlg);
    from->setObjectName(QStringLiteral("replaceFontsFrom"));
    QStringList names;
    for (const FontUse &u : used) {
        QString text = u.shown.isEmpty() ? u.key : u.shown;
        names << text;
        if (!u.theme && !installed.contains(u.key.toLower())) text += QCoreApplication::translate("jp::ReplaceFonts", " (not installed)");
        from->addItem(QCoreApplication::translate("jp::ReplaceFonts", "%1 (used in %2)").arg(text).arg(u.uses), u.key);
    }
    form->addRow(QCoreApplication::translate("jp::ReplaceFonts", "Replace:"), from);
    auto *to = new QComboBox(&dlg);
    to->setObjectName(QStringLiteral("replaceFontsTo"));
    to->setEditable(true);
    to->setInsertPolicy(QComboBox::NoInsert);
    to->addItems(QFontDatabase::families());
    to->setCurrentIndex(-1);
    to->setPlaceholderText(QCoreApplication::translate("jp::ReplaceFonts", "Choose a font"));
    form->addRow(QCoreApplication::translate("jp::ReplaceFonts", "With:"), to);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    auto *ok = buttons->button(QDialogButtonBox::Ok);
    ok->setText(QCoreApplication::translate("jp::ReplaceFonts", "Replace"));
    ok->setObjectName(QStringLiteral("replaceFontsReplace"));
    form->addRow(buttons);
    auto check = [&] {
        const QString target = to->currentText().trimmed();
        ok->setEnabled(from->currentIndex() >= 0 && !target.isEmpty() && target.compare(from->currentData().toString(), Qt::CaseInsensitive) != 0);
    };
    QObject::connect(from, &QComboBox::currentIndexChanged, &dlg, check);
    QObject::connect(to, &QComboBox::currentTextChanged, &dlg, check);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    check();
    if (dlg.exec() != QDialog::Accepted || from->currentIndex() < 0) return result;
    const int at = from->currentIndex();
    const QString key = from->currentData().toString(), target = to->currentText().trimmed();
    if (target.isEmpty()) return result;
    result.uses = host.replace(key, target);
    result.from = names.value(at);
    result.to = target;
    return result;
}

} // namespace jp
