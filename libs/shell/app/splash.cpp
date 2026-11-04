#include "app/splash.h"

#include "core/appinfo.h"

#include <QApplication>
#include <QFileInfo>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>

#include <algorithm>

namespace jp {

namespace {
constexpr int kWidth = 440, kHeight = 250, kRadius = 12;
} // namespace

StartupSplash::StartupSplash()
{
    setAttribute(Qt::WA_DeleteOnClose);
    // Soft corners need a see-through window; an X server without a compositor would draw them black, so there the card
    // is square.
    m_round = QGuiApplication::platformName() != QLatin1String("xcb");
    if (m_round) setAttribute(Qt::WA_TranslucentBackground);
    const qreal dpr = QGuiApplication::primaryScreen() ? QGuiApplication::primaryScreen()->devicePixelRatio() : 1.0;
    QPixmap blank(QSize(kWidth, kHeight) * dpr);
    blank.setDevicePixelRatio(dpr);
    blank.fill(Qt::transparent);
    setPixmap(blank);
    setAccessibleName(appInfo().name);
    m_status = QCoreApplication::translate("Splash", "Starting…");
}

void StartupSplash::step(const QString &what, double done)
{
    m_status = what;
    m_done = std::clamp(done, 0.0, 1.0);
    setAccessibleDescription(what);
    repaint();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void StartupSplash::drawContents(QPainter *p)
{
    const AppInfo &app = appInfo();
    const QColor color = app.accent.isValid() ? app.accent : QColor(0x1A, 0x5F, 0xD6);
    const QRectF card(0, 0, kWidth, kHeight);
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(Qt::NoPen);
    p->setBrush(color);
    if (m_round) p->drawRoundedRect(card, kRadius, kRadius);
    else p->drawRect(card);

    const qreal pad = 22;
    // the tile: the app's letters on white
    const QRectF tile(pad, pad, 48, 48);
    p->setBrush(Qt::white);
    p->drawRoundedRect(tile, 12, 12);
    QFont f = QApplication::font();
    f.setPixelSize(19);
    f.setWeight(QFont::DemiBold);
    p->setFont(f);
    p->setPen(color);
    p->drawText(tile, Qt::AlignCenter, app.badge);

    // the suite's name, top right
    QFont small = QApplication::font();
    small.setPixelSize(12);
    p->setFont(small);
    p->setPen(QColor(255, 255, 255, 220));
    p->drawText(QRectF(pad, pad, kWidth - 2 * pad, 18), Qt::AlignRight | Qt::AlignTop, QStringLiteral("JeffOffice"));

    // the app's name
    QFont big = QApplication::font();
    big.setPixelSize(30);
    big.setWeight(QFont::Light);
    p->setFont(big);
    p->setPen(Qt::white);
    p->drawText(QRectF(pad, 120, kWidth - 2 * pad, 42), Qt::AlignLeft | Qt::AlignVCenter, app.name);

    // what it is doing, and the version
    p->setFont(small);
    p->setPen(QColor(255, 255, 255, 235));
    const QRectF line(pad, kHeight - pad - 30, kWidth - 2 * pad, 18);
    const QString version = QCoreApplication::applicationVersion();
    const int versionWidth = QFontMetrics(small).horizontalAdvance(version) + 12;
    p->drawText(line.adjusted(0, 0, -versionWidth, 0), Qt::AlignLeft | Qt::AlignVCenter,
                QFontMetrics(small).elidedText(m_status, Qt::ElideMiddle, int(line.width()) - versionWidth));
    p->drawText(line, Qt::AlignRight | Qt::AlignVCenter, version);

    // how far along
    const QRectF bar(pad, kHeight - pad - 5, kWidth - 2 * pad, 4);
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(255, 255, 255, 80));
    p->drawRoundedRect(bar, 2, 2);
    if (m_done > 0) {
        p->setBrush(Qt::white);
        p->drawRoundedRect(QRectF(bar.topLeft(), QSizeF(bar.width() * m_done, bar.height())), 2, 2);
    }
}

StartupSplash *showStartupSplash()
{
    const QString platform = QGuiApplication::platformName();
    if (platform == QLatin1String("offscreen") || platform == QLatin1String("minimal")) return nullptr;
    auto *s = new StartupSplash();
    s->show();
    s->step(s->status(), 0.1);
    return s;
}

QString openingStatus(const QString &path)
{
    return QCoreApplication::translate("Splash", "Opening %1…").arg(QFileInfo(path).fileName());
}

} // namespace jp
