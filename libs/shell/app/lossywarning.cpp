#include "app/lossywarning.h"

#include "app/plaintext.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QSet>

namespace jp {

namespace {

constexpr int kMaxRemembered = 1000;   // (files the person said not to ask about: more than that are not kept)

QSet<QString> &notAgain()
{
    static QSet<QString> set;
    return set;
}

std::function<LossyChoice(const LossyAsk &, bool *)> &standIn()
{
    static std::function<LossyChoice(const LossyAsk &, bool *)> f;
    return f;
}

QString keyOf(const QString &path)
{
    const QFileInfo info(path);
    const QString canonical = info.canonicalFilePath();
    return canonical.isEmpty() ? info.absoluteFilePath() : canonical;
}

QString listText(const QStringList &lost)
{
    QStringList lines;
    for (int i = 0; i < lost.size() && i < kLossyMaxShown; ++i) lines << lost.at(i);
    if (lost.size() > kLossyMaxShown) lines << QCoreApplication::translate("jp::LossyWarning", "and %n more", nullptr, int(lost.size() - kLossyMaxShown));
    return lines.join(QLatin1Char('\n'));
}

} // namespace

QString lossyReport(const LossyAsk &ask)
{
    if (ask.lost.isEmpty()) return QCoreApplication::translate("jp::LossyWarning", "Nothing in this %1 is lost when it is saved as a %2 file.").arg(ask.what, ask.format);
    return QCoreApplication::translate("jp::LossyWarning", "A %1 file can't hold some of what this %2 has. If you save it that way, these parts are saved as they stand now or left out:\n\n%3")
        .arg(ask.format, ask.what, listText(ask.lost));
}

QString lossyOwnFormatPath(const QString &path, const QString &ownFormat)
{
    const QFileInfo info(path);
    const QString beside = info.absolutePath() + QLatin1Char('/') + info.completeBaseName();
    QString target = beside + ownFormat;
    for (int n = 2; QFileInfo::exists(target) && n < 10000; ++n) target = beside + QStringLiteral(" (%1)").arg(n) + ownFormat;
    return target;
}

bool lossyAskedNotAgain(const QString &path) { return notAgain().contains(keyOf(path)); }

void lossyForgetAll() { notAgain().clear(); }

void setLossyStandIn(const std::function<LossyChoice(const LossyAsk &, bool *)> &f) { standIn() = f; }

LossyChoice askLossy(QWidget *parent, const LossyAsk &ask)
{
    if (ask.lost.isEmpty()) return LossyChoice::SaveAnyway;
    if (lossyAskedNotAgain(ask.path)) return LossyChoice::SaveAnyway;
    bool dontAsk = false;
    LossyChoice choice = LossyChoice::Cancel;
    if (standIn()) {
        choice = standIn()(ask, &dontAsk);
    } else {
        QMessageBox box(QMessageBox::Warning, QCoreApplication::translate("jp::LossyWarning", "Save"), lossyReport(ask), QMessageBox::NoButton, parent);
        box.setObjectName(QStringLiteral("lossyWarning"));
        box.setTextFormat(Qt::PlainText);
        QPushButton *anyway = box.addButton(QCoreApplication::translate("jp::LossyWarning", "Save Anyway"), QMessageBox::AcceptRole);
        QPushButton *own = box.addButton(QCoreApplication::translate("jp::LossyWarning", "Save as %1").arg(ask.ownFormat), QMessageBox::ActionRole);
        QPushButton *cancel = box.addButton(QMessageBox::Cancel);
        auto *check = new QCheckBox(QCoreApplication::translate("jp::LossyWarning", "Don't ask again for this file"), &box);
        box.setCheckBox(check);
        box.setDefaultButton(own);
        box.setEscapeButton(cancel);
        box.exec();
        dontAsk = check->isChecked();
        choice = box.clickedButton() == anyway ? LossyChoice::SaveAnyway : box.clickedButton() == own ? LossyChoice::SaveOwnFormat : LossyChoice::Cancel;
    }
    if (choice == LossyChoice::SaveAnyway && dontAsk && notAgain().size() < kMaxRemembered) notAgain().insert(keyOf(ask.path));
    return choice;
}

} // namespace jp
