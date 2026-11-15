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

constexpr int kMaxRemembered = 1000;   // (files the person said not to ask about: past that the oldest is forgotten)

// The files, oldest first; the set is for the look-up.
QList<QString> &order()
{
    static QList<QString> list;
    return list;
}

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

// The same file has the same key before it is saved (it does not exist yet) and after: the folder is made canonical (a link, a short name, /var on a Mac) and the file's name is added as written.
// Names are not told apart by case where the system doesn't.
QString keyOf(const QString &path)
{
    const QFileInfo info(path);
    const QString dir = QFileInfo(info.absolutePath()).canonicalFilePath();
    QString key = (dir.isEmpty() ? info.absolutePath() : dir) + QLatin1Char('/') + info.fileName();
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    key = key.toCaseFolded();
#endif
    return key;
}

QString listText(const QStringList &lost)
{
    QStringList lines;
    for (int i = 0; i < lost.size() && i < kLossyMaxShown; ++i) lines << lost.at(i);
    if (lost.size() > kLossyMaxShown) lines << QCoreApplication::translate("jp::LossyWarning", "and %1 more").arg(lost.size() - kLossyMaxShown);
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

void lossyForgetAll()
{
    notAgain().clear();
    order().clear();
}

bool lossyStandInSet() { return bool(standIn()); }

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
        jp::MessageBox box(QMessageBox::Warning, QCoreApplication::translate("jp::LossyWarning", "Save"), lossyReport(ask), QMessageBox::NoButton, parent);
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
    if (choice == LossyChoice::SaveAnyway && dontAsk) {
        const QString key = keyOf(ask.path);
        if (!notAgain().contains(key)) {
            notAgain().insert(key);
            order().append(key);
            while (order().size() > kMaxRemembered) notAgain().remove(order().takeFirst());
        }
    }
    return choice;
}

QStringList LossyCache::get(const QString &key, const std::function<QStringList()> &compute)
{
    if (m_valid && m_at == m_rev && m_key == key) return m_lost;
    m_lost = compute();
    m_key = key;
    m_at = m_rev;
    m_valid = true;
    return m_lost;
}

} // namespace jp
