#include "app/filescreenparts.h"

#include "core/appinfo.h"

#include <QCoreApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "app/plaintext.h"

namespace jp {

QString cardStyle()
{
    return QStringLiteral("#jpCard{background:%1; border:1px solid %2; border-radius:12px;}").arg(uiCard().name(), uiLine().name());
}

QLabel *heading(const QString &t, QWidget *parent, double scale)
{
    auto *l = plainLabel(new QLabel(t, parent));
    QFont f = l->font();
    f.setPointSizeF(f.pointSizeF() * scale);
    f.setWeight(QFont::DemiBold);
    l->setFont(f);
    l->setContentsMargins(0, 0, 0, 6);
    return l;
}

// A page drawn as paper: soft layered shadow and a hairline edge.
QPixmap paperPixmap(const QImage &page, const QSize &box, qreal dpr)
{
    QPixmap pm(box * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    const QSizeF avail = QSizeF(box) - QSizeF(16, 18);
    QSizeF sz = QSizeF(page.size()) / page.devicePixelRatio();
    sz.scale(avail, Qt::KeepAspectRatio);
    const QRectF r(QPointF((box.width() - sz.width()) / 2.0, (box.height() - sz.height()) / 2.0 - 2), sz);
    for (int i = 6; i >= 1; --i) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 7));
        p.drawRoundedRect(r.adjusted(-i * 0.6, -i * 0.3 + 1, i * 0.6, i * 0.9 + 1), 2 + i * 0.5, 2 + i * 0.5);
    }
    p.drawImage(r, page);
    p.setPen(QPen(QColor(0, 0, 0, 38), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(r.adjusted(0.5, 0.5, -0.5, -0.5));
    return pm;
}

// A rounded white (or dark) panel used for grouped content.
QFrame *cardFrame(QWidget *parent)
{
    auto *f = new QFrame(parent);
    f->setObjectName(QStringLiteral("jpCard"));
    f->setStyleSheet(cardStyle());
    return f;
}

QLabel *mutedLabel(const QString &t, QWidget *parent, double alpha)
{
    auto *l = new QLabel(t, parent);
    QPalette pl = l->palette();
    QColor c = uiText();
    c.setAlphaF(float(alpha));
    pl.setColor(QPalette::WindowText, c);
    l->setPalette(pl);
    return l;
}

QLabel *sectionLabel(const QString &t, QWidget *parent)
{
    auto *l = new QLabel(t.toUpper(), parent);
    QFont f = l->font();
    f.setPointSizeF(f.pointSizeF() * 0.78);
    f.setWeight(QFont::DemiBold);
    f.setLetterSpacing(QFont::PercentageSpacing, 108);
    l->setFont(f);
    QPalette pl = l->palette();
    QColor c = uiText();
    c.setAlphaF(0.55f);
    pl.setColor(QPalette::WindowText, c);
    l->setPalette(pl);
    l->setContentsMargins(24, 14, 12, 4);
    return l;
}

namespace {
// Keeps a word-wrapped label as tall as its text at the width it gets. The
// About cards are capped at 820 pixels, but Qt works out a card's height for
// the page's full width, where text wraps less: in a wide window the cards
// came out short and cut off their last lines.
class FitHeight : public QObject {
public:
    using QObject::QObject;
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() == QEvent::Resize)
            if (auto *l = qobject_cast<QLabel *>(o)) {
                const int h = l->heightForWidth(l->width());
                if (h > 0 && h != l->minimumHeight()) l->setMinimumHeight(h);
            }
        return false;
    }
};
} // namespace

void fitHeight(QLabel *l) { l->installEventFilter(new FitHeight(l)); }

// One app of the suite: a rounded tile in the app's color holding its two
// letters, its name in bold, what it makes, and where it stands.
// The columns are the widths given, so the rows line up.
static QWidget *suiteRow(QWidget *parent, const QString &name, const QString &letters, const QColor &color, const QString &what, const QString &state,
                         bool muted, const int (&widths)[3], const QString &accessible)
{
    auto *row = new QWidget(parent);
    row->setAccessibleName(accessible);
    auto *h = new QHBoxLayout(row);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(12);
    auto *tile = new QLabel(row);
    const qreal dpr = parent->devicePixelRatioF();
    QPixmap pm(QSize(28, 28) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawRoundedRect(QRectF(0, 0, 28, 28), 28 * 0.24, 28 * 0.24);
    QFont tf = parent->font();
    tf.setWeight(QFont::DemiBold);
    tf.setPixelSize(12);
    p.setFont(tf);
    p.setPen(Qt::white);
    p.drawText(QRectF(0, 0, 28, 28), Qt::AlignCenter, letters);
    p.end();
    tile->setPixmap(pm);
    h->addWidget(tile);
    auto *nameLabel = plainLabel(new QLabel(name, row));
    QFont nf = nameLabel->font();
    nf.setBold(true);
    nameLabel->setFont(nf);
    nameLabel->setFixedWidth(widths[0]);
    h->addWidget(nameLabel);
    auto *whatLabel = plainLabel(new QLabel(what, row));
    // A narrow window clips this column, not the state; the stretches give it
    // all the room it can use before the spare goes to the end of the row.
    whatLabel->setMaximumWidth(widths[1]);
    whatLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    h->addWidget(whatLabel, 1000);
    QLabel *stateLabel = muted ? mutedLabel(state, row) : plainLabel(new QLabel(state, row));
    stateLabel->setFixedWidth(widths[2]);
    h->addWidget(stateLabel);
    h->addStretch(1);
    return row;
}

QPixmap appBadge(const QWidget *on, int size, int pixelSize, qreal radius)
{
    const qreal dpr = on->devicePixelRatioF();
    QPixmap pm(QSize(size, size) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(uiAccent());
    p.drawRoundedRect(QRectF(0, 0, size, size), radius, radius);
    QFont bf = on->font();
    bf.setBold(true);
    bf.setPixelSize(pixelSize);
    p.setFont(bf);
    p.setPen(Qt::white);
    p.drawText(QRectF(0, 0, size, size), Qt::AlignCenter, appInfo().badge);
    p.end();
    return pm;
}

QWidget *suiteCard(QWidget *parent, const QWidget *screen)
{
    auto *suite = cardFrame(parent);
    suite->setMaximumWidth(820);
    auto *suv = new QVBoxLayout(suite);
    suv->setContentsMargins(20, 16, 20, 16);
    suv->setSpacing(10);
    auto *logo = plainLabel(new QLabel(suite));
    {
        const qreal dpr = screen->devicePixelRatioF();
        const QImage lockup(uiDark() ? QStringLiteral(":/brand/jeffoffice-lockup-on-dark.png") : QStringLiteral(":/brand/jeffoffice-lockup.png"));
        QPixmap pm = QPixmap::fromImage(lockup.scaledToHeight(qRound(40 * dpr), Qt::SmoothTransformation));
        pm.setDevicePixelRatio(dpr);
        logo->setPixmap(pm);
        logo->setAccessibleName(QStringLiteral("JeffOffice"));
    }
    suv->addWidget(logo);
    auto *suh = new QLabel(QCoreApplication::translate("jp::FileScreen", "JeffOffice apps"), suite);
    QFont suf = suh->font();
    suf.setWeight(QFont::DemiBold);
    suh->setFont(suf);
    suv->addWidget(suh);
    // The app running says so; one that is out says it is available, or a preview.
    enum Stage { Soon, Preview, Out };
    struct App { const char *name, *letters, *color; QString what; Stage stage; };
    const App apps[] = {
        {"JeffPub", "JP", "#9E1F63", QCoreApplication::translate("jp::FileScreen", "Publications"), Out},
        {"JeffDeck", "JD", "#B6420F", QCoreApplication::translate("jp::FileScreen", "Presentations"), Out},
        {"JeffWrite", "JW", "#1D55C4", QCoreApplication::translate("jp::FileScreen", "Documents"), Preview},
        {"JeffCrunch", "JC", "#13773A", QCoreApplication::translate("jp::FileScreen", "Spreadsheets"), Preview},
        {"JeffMail", "JM", "#0A6C8C", QCoreApplication::translate("jp::FileScreen", "Email and calendar"), Soon},
        {"JeffBase", "JB", "#5E2CA5", QCoreApplication::translate("jp::FileScreen", "Databases"), Soon},
    };
    const QString inUse = QCoreApplication::translate("jp::FileScreen", "You're using it"), soon = QCoreApplication::translate("jp::FileScreen", "Coming soon"),
                  available = QCoreApplication::translate("jp::FileScreen", "Available"), preview = QCoreApplication::translate("jp::FileScreen", "Preview");
    auto here = [](const App &a) { return QLatin1String(a.name) == appInfo().name; };
    auto stateOf = [&](const App &a) { return here(a) ? inUse : a.stage == Out ? available : a.stage == Preview ? preview : soon; };
    auto line = [](const QString &what) { return QStringLiteral("— %1").arg(what); };
    QFont bold = screen->font();
    bold.setBold(true);
    const QFontMetrics plain(screen->font()), strong(bold);
    int widths[3] = {0, 0, 0};
    for (const App &a : apps) {
        widths[0] = qMax(widths[0], strong.horizontalAdvance(QLatin1String(a.name)));
        widths[1] = qMax(widths[1], plain.horizontalAdvance(line(a.what)));
        widths[2] = qMax(widths[2], plain.horizontalAdvance(stateOf(a)));
    }
    widths[0] += 4;
    widths[1] += 16;   // and room before the state
    widths[2] += 4;
    for (const App &a : apps) {
        const QString state = stateOf(a);
        suv->addWidget(suiteRow(suite, QLatin1String(a.name), QLatin1String(a.letters), QColor(QLatin1String(a.color)), line(a.what), state,
                                !here(a), widths, QCoreApplication::translate("jp::FileScreen", "%1, %2, %3").arg(QLatin1String(a.name), a.what, state)));
    }
    return suite;
}

} // namespace jp
