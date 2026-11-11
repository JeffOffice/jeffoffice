#pragma once
// For tests of "remove personal information": a hook for office::setPackageScrubHookForTest that damages one part of the package (a byte of its data is changed, so that it can't be inflated or
// does not match its checksum), and what pulls the attachment out of an .eml file the apps make.

#include <QApplication>
#include <QByteArray>
#include <QMessageBox>
#include <QTimer>
#include <QRegularExpression>
#include <functional>

namespace jptest {

// While it lives, any message box that opens is closed (checked every 20 ms): for a save that fails and says so in a box.
struct DismissBoxes {
    DismissBoxes()
    {
        QObject::connect(&timer, &QTimer::timeout, [] {
            for (QWidget *top : QApplication::topLevelWidgets())
                if (auto *box = qobject_cast<QMessageBox *>(top); box && box->isVisible()) box->accept();
        });
        timer.start(20);
    }
    QTimer timer;
};

inline std::function<QByteArray(const QByteArray &)> damagePart(const QByteArray &name)
{
    return [name](const QByteArray &zip) {
        QByteArray damaged = zip;
        const qsizetype at = damaged.indexOf(name);
        if (at >= 0 && at + name.size() + 4 < damaged.size()) damaged[at + name.size() + 3] = char(damaged[at + name.size() + 3] ^ 0x55);
        return damaged;
    };
}

// The bytes of the first attachment whose file name ends with `ending` in a message the apps make (base64, in lines).
inline QByteArray attachmentOf(const QByteArray &eml, const QByteArray &ending)
{
    const qsizetype name = eml.indexOf(ending + "\"");
    if (name < 0) return {};
    qsizetype body = eml.indexOf("\r\n\r\n", name);
    qsizetype gap = 4;
    if (body < 0) {
        body = eml.indexOf("\n\n", name);
        gap = 2;
    }
    if (body < 0) return {};
    body += gap;
    qsizetype end = eml.indexOf("\n--", body);
    if (end < 0) end = eml.size();
    return QByteArray::fromBase64(eml.mid(body, end - body), QByteArray::Base64Encoding | QByteArray::IgnoreBase64DecodingErrors);
}

} // namespace jptest
