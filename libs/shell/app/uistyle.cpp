#include "app/uistyle.h"

#include "app/icons.h"
#include "app/i18n.h"
#include "app/theme.h"
#include "app/plaintext.h"
#include "core/appinfo.h"

#include <QApplication>
#include <QBoxLayout>
#include <QDialog>
#include <QEvent>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QProxyStyle>
#include <QPushButton>
#include <QStyleFactory>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <memory>

namespace jp {

QString upperCase(const QString &text)
{
    const QString language = currentUiLanguage();
    return (language.isEmpty() ? QLocale() : QLocale(language)).toUpper(text);
}

namespace {
// Runs `apply` now and again whenever the look changes (light, dark, contrast), so that a color kept in a palette follows the theme.
void followTheme(QWidget *w, const std::function<void()> &apply)
{
    apply();
    QObject::connect(UiTheme::instance(), &UiTheme::changed, w, apply);
}

void repolish(QWidget *w)
{
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}

QFont headingFont(QFont f)
{
    f.setPixelSize(11);
    f.setWeight(QFont::Medium);
    return f;
}

QLabel *smallLabel(const QString &text, QWidget *parent)
{
    auto *l = new QLabel(text, parent);
    l->setTextFormat(Qt::PlainText);
    QFont f = l->font();
    f.setPixelSize(11);
    l->setFont(f);
    followTheme(l, [l] {
        QPalette p = l->palette();
        p.setColor(QPalette::WindowText, uiMuted());
        l->setPalette(p);
    });
    l->setProperty("smallLabel", true);
    return l;
}
} // namespace

QLabel *sectionHeading(const QString &text, QWidget *parent)
{
    auto *l = new QLabel(upperCase(text), parent);
    l->setTextFormat(Qt::PlainText);
    l->setFont(headingFont(l->font()));
    followTheme(l, [l] {
        QPalette p = l->palette();
        p.setColor(QPalette::WindowText, uiMuted());
        l->setPalette(p);
    });
    l->setProperty("sectionHeading", true);
    l->setAccessibleName(text);
    return l;
}

Section::Section(const QString &heading, QWidget *parent) : QGroupBox(upperCase(heading), parent), m_heading(heading)
{
    setProperty("section", true);
    setAccessibleName(heading);
    setFlat(true);
}

QSize Section::minimumSizeHint() const
{
    QSize hint = QGroupBox::minimumSizeHint();
    hint.setWidth(std::max(hint.width(), fontMetrics().horizontalAdvance(title()) + (isCheckable() ? 30 : 4)));
    return hint;
}

QSize Section::sizeHint() const
{
    QSize hint = QGroupBox::sizeHint();
    hint.setWidth(std::max(hint.width(), minimumSizeHint().width()));
    return hint;
}

void styleForm(QFormLayout *form)
{
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(8);
}

QWidget *twoUp(const QString &labelA, QWidget *a, const QString &labelB, QWidget *b, QWidget *parent)
{
    auto *w = new QWidget(parent);
    auto *g = new QGridLayout(w);
    g->setContentsMargins(0, 0, 0, 0);
    g->setHorizontalSpacing(12);
    g->setVerticalSpacing(3);
    g->setColumnStretch(0, 1);
    g->setColumnStretch(1, 1);
    QLabel *la = smallLabel(labelA, w), *lb = smallLabel(labelB, w);
    la->setBuddy(a);
    lb->setBuddy(b);
    g->addWidget(la, 0, 0);
    g->addWidget(lb, 0, 1);
    g->addWidget(a, 1, 0);
    g->addWidget(b, 1, 1);
    return w;
}

QWidget *labeled(const QString &label, QWidget *field, QWidget *parent)
{
    auto *w = new QWidget(parent);
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(3);
    QLabel *l = smallLabel(label, w);
    l->setBuddy(field);
    v->addWidget(l);
    v->addWidget(field);
    return w;
}

QWidget *checkPair(QWidget *a, QWidget *b, QWidget *parent)
{
    auto *w = new QWidget(parent);
    auto *h = new QHBoxLayout(w);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(16);
    h->addWidget(a);
    h->addWidget(b);
    h->addStretch(1);
    return w;
}

QPushButton *linkButton(const QString &text, QWidget *parent)
{
    auto *b = new QPushButton(text, parent);
    b->setProperty("link", true);
    b->setFlat(true);
    b->setCursor(Qt::PointingHandCursor);
    b->setAutoDefault(false);
    followTheme(b, [b] {
        QPalette p = b->palette();
        p.setColor(QPalette::ButtonText, uiAccent());
        b->setPalette(p);
    });
    return b;
}

void softPanel(QFrame *f)
{
    f->setFrameShape(QFrame::NoFrame);
    f->setProperty("previewPanel", true);
    f->setAutoFillBackground(true);
    followTheme(f, [f] {
        QPalette p = f->palette();
        p.setColor(QPalette::Window, uiPanel());
        f->setPalette(p);
    });
}

QFrame *previewPanel(QWidget *parent)
{
    auto *f = new QFrame(parent);
    softPanel(f);
    return f;
}

QLabel *noteLabel(const QString &text, QWidget *parent)
{
    auto *l = new QLabel(text, parent);
    l->setTextFormat(Qt::PlainText);
    l->setWordWrap(true);
    QFont f = l->font();
    f.setPixelSize(12);
    l->setFont(f);
    followTheme(l, [l] {
        QPalette p = l->palette();
        p.setColor(QPalette::WindowText, uiMuted());
        l->setPalette(p);
    });
    l->setProperty("note", true);
    return l;
}

// ---- buttons ----
namespace {
// No icons on buttons.
class ButtonStyle : public QProxyStyle {
public:
    ButtonStyle() : QProxyStyle(QStyleFactory::create(QStringLiteral("Fusion"))) {}
    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *ret) const override
    {
        if (hint == SH_DialogButtonBox_ButtonsHaveIcons) return 0;
        return QProxyStyle::styleHint(hint, option, widget, ret);
    }
};

// Where a button sits among the ones at the right, in reading order: the helpers first, Cancel before the primary button, the primary last.
int rank(QDialogButtonBox::ButtonRole role)
{
    switch (role) {
    case QDialogButtonBox::HelpRole: return 0;
    case QDialogButtonBox::ResetRole: return 1;
    case QDialogButtonBox::ActionRole: return 2;
    case QDialogButtonBox::ApplyRole: return 3;
    case QDialogButtonBox::NoRole: return 5;
    case QDialogButtonBox::YesRole: return 6;
    case QDialogButtonBox::AcceptRole: return 7;
    case QDialogButtonBox::RejectRole: return 8;
    default: return 2;
    }
}

// The button Enter triggers, which is the one that gets the accent: the box's own default button (Qt makes the first accept button the default when the box is shown); none when
// there is none, so that the accent is never on a button Enter does not press.
QPushButton *chooseDefault(QDialogButtonBox *box)
{
    for (QAbstractButton *b : box->buttons())
        if (auto *pb = qobject_cast<QPushButton *>(b); pb && pb->isDefault()) return pb;
    return nullptr;
}

// Puts the buttons in the order of the style: a destructive choice alone at the left, then a stretch, then the rest in reading order with the primary last.
void arrange(QDialogButtonBox *box)
{
    if (box->property("jpArranging").toBool()) return;
    auto *lay = qobject_cast<QBoxLayout *>(box->layout());
    if (!lay || box->orientation() != Qt::Horizontal) return;
    box->setProperty("jpArranging", true);
    // a destructive choice never becomes the default by taking the focus (Enter would throw something away)
    for (QAbstractButton *b : box->buttons())
        if (auto *pb = qobject_cast<QPushButton *>(b); pb && box->buttonRole(b) == QDialogButtonBox::DestructiveRole) {
            pb->setAutoDefault(false);
            if (pb->isDefault()) pb->setDefault(false);
        }
    // who is the primary: the one Enter triggers
    QPushButton *primary = chooseDefault(box);
    for (QAbstractButton *b : box->buttons())
        if (b->property("primary").toBool() != (b == primary) && qobject_cast<QPushButton *>(b)) {
            b->setProperty("primary", b == primary);
            repolish(b);
        }
    // (the one that comes last: the primary, and before the default is known the first accept or yes button)
    QAbstractButton *last = primary;
    if (!last)
        for (QAbstractButton *b : box->buttons())
            if (box->buttonRole(b) == QDialogButtonBox::AcceptRole || box->buttonRole(b) == QDialogButtonBox::YesRole) { last = b; break; }
    QList<QAbstractButton *> left, right;
    for (QAbstractButton *b : box->buttons()) (box->buttonRole(b) == QDialogButtonBox::DestructiveRole && b != last ? left : right).append(b);
    std::stable_sort(right.begin(), right.end(), [&](QAbstractButton *x, QAbstractButton *y) {
        const int rx = x == last ? 100 : rank(box->buttonRole(x)), ry = y == last ? 100 : rank(box->buttonRole(y));
        return rx < ry;
    });
    QList<QWidget *> want;
    for (QAbstractButton *b : left) want << b;
    want << nullptr;
    for (QAbstractButton *b : right) want << b;
    QList<QWidget *> have;
    for (int i = 0; i < lay->count(); ++i) have << lay->itemAt(i)->widget();
    // (a layout that has spacers of its own, or none, is made again whole; one already in order is left alone)
    if (have != want) {
        while (QLayoutItem *item = lay->takeAt(0)) {
            delete item;   // (the layout item only; a button stays)
        }
        for (QAbstractButton *b : left) lay->addWidget(b);
        lay->addStretch(1);
        for (QAbstractButton *b : right) lay->addWidget(b);
    }
    // no icons on buttons (a Qt layout of the macOS kind takes them off; the others leave them)
    for (QAbstractButton *b : box->buttons())
        if (!b->icon().isNull()) b->setIcon(QIcon());
    // Tab follows the picture
    QList<QAbstractButton *> order = left + right;
    box->setFocusProxy(nullptr);   // (Qt's own layout gives the box the first button as its proxy, which a tab order set on the buttons would not follow)
    for (int i = 0; i + 1 < order.size(); ++i) QWidget::setTabOrder(order[i], order[i + 1]);
    box->setProperty("jpArranging", false);
}

// Watches a button box for everything that makes Qt lay its buttons out again (a button added, a style change, a layout request) and for the default button changing hands
// (focus moving among the buttons), and puts the order and the accent right again.
class BoxWatcher : public QObject {
public:
    explicit BoxWatcher(QDialogButtonBox *box) : QObject(box), m_box(box)
    {
        box->installEventFilter(this);
        watchButtons();
    }
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (o == m_box) {
            switch (e->type()) {
            case QEvent::Show: arrange(m_box); later(); break;
            case QEvent::ChildAdded: case QEvent::ChildRemoved: case QEvent::StyleChange: case QEvent::LayoutRequest: case QEvent::EnabledChange: later(); break;
            default: break;
            }
        } else if (e->type() == QEvent::FocusIn || e->type() == QEvent::FocusOut) {
            later();
        }
        return false;
    }

private:
    void watchButtons()
    {
        for (QAbstractButton *b : m_box->buttons())
            if (!b->property("jpWatched").toBool()) {
                b->setProperty("jpWatched", true);
                b->installEventFilter(this);
            }
    }
    void later()
    {
        if (m_pending) return;
        m_pending = true;
        QMetaObject::invokeMethod(this, [this] {
            m_pending = false;
            watchButtons();
            arrange(m_box);
            // (Qt may lay the buttons out again after this, from a call it queued itself: once more a moment later, which puts the order right again)
            if (!m_again) {
                m_again = true;
                QTimer::singleShot(40, this, [this] {
                    m_again = false;
                    watchButtons();
                    arrange(m_box);
                });
            }
        }, Qt::QueuedConnection);
    }
    QDialogButtonBox *m_box;
    bool m_pending = false;
    bool m_again = false;
};
} // namespace

void arrangeButtons(QDialogButtonBox *box)
{
    if (!box->property("jpButtonStyle").toBool()) {
        box->setProperty("jpButtonStyle", true);
        // (one style for every box, owned by the application: a style owned by the box was deleted with the box's children, before a
        // button added later, and that button's focus-out on the way went through the freed style. Nov 16: JeffDeck's Go To dialog
        // crashed so in a test, and could on a system that clears focus late. ButtonStyle keeps no state of its own box.)
        static QPointer<ButtonStyle> shared;
        if (!shared) {
            shared = new ButtonStyle;
            shared->setParent(qApp);
        }
        box->setStyle(shared);
        new BoxWatcher(box);
    }
    arrange(box);
}

ButtonBox::ButtonBox(QWidget *parent) : QDialogButtonBox(parent) { arrangeButtons(this); }
ButtonBox::ButtonBox(StandardButtons buttons, QWidget *parent) : QDialogButtonBox(buttons, parent) { arrangeButtons(this); }
ButtonBox::ButtonBox(StandardButtons buttons, Qt::Orientation orientation, QWidget *parent) : QDialogButtonBox(buttons, orientation, parent) { arrangeButtons(this); }

QPushButton *ButtonBox::addDestructive(const QString &text)
{
    QPushButton *b = addButton(text, DestructiveRole);
    b->setAutoDefault(false);
    return b;
}

bool ButtonBox::event(QEvent *e) { return QDialogButtonBox::event(e); }

namespace {
// The program's own boxes do it by themselves; every other button box (a dialog written before the style, a Qt one) gets it when it is shown.
class ButtonBoxFilter : public QObject {
public:
    using QObject::QObject;
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() == QEvent::Show && o->isWidgetType()) {
            if (auto *box = qobject_cast<QDialogButtonBox *>(o); box && !box->property("jpButtonStyle").toBool()) arrangeButtons(box);
            if (auto *dlg = qobject_cast<QDialog *>(o); dlg && !dlg->property("jpDialogStyled").toBool() && !qobject_cast<QMessageBox *>(dlg) &&
                                                      !QByteArrayView(dlg->metaObject()->className()).startsWith("Q")) {
                dlg->setProperty("jpDialogStyled", true);
                // one label column and rows 8 apart in every form of the dialog
                for (QFormLayout *f : dlg->findChildren<QFormLayout *>()) styleForm(f);
            }
        }
        return false;
    }
};
} // namespace

void installButtonBoxStyle()
{
    static bool done = false;
    if (done || !qApp) return;
    done = true;
    qApp->installEventFilter(new ButtonBoxFilter(qApp));
}

// ---- message boxes ----
QPixmap boxSign(QMessageBox::Icon kind, int size, qreal pixelRatio)
{
    QPixmap pm(QSize(size, size) * pixelRatio);
    pm.setDevicePixelRatio(pixelRatio);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    const double s = size;
    p.scale(s / 32.0, s / 32.0);   // drawn on a 32 by 32 grid
    const bool red = kind == QMessageBox::Warning || kind == QMessageBox::Critical;
    // (blue for Information and Question whatever the app's own color is: the sign says what kind of box it is)
    const QColor blue = uiHighContrast() ? uiAccent() : uiDark() ? QColor(0x7F, 0xB2, 0xFF) : QColor(0x1A, 0x5F, 0xD6);
    const QColor color = red ? uiDanger() : blue;
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    if (kind == QMessageBox::Warning) {
        QPainterPath tri;
        tri.moveTo(16, 3);
        tri.lineTo(30, 28);
        tri.lineTo(2, 28);
        tri.closeSubpath();
        p.setPen(QPen(color, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawPath(tri);
    } else {
        p.drawEllipse(QRectF(2, 2, 28, 28));
    }
    const QColor mark = uiHighContrast() ? QGuiApplication::palette().color(QPalette::Window) : uiDark() ? QColor(0x10, 0x14, 0x18) : QColor(Qt::white);   // (3:1 or better on the sign)
    QPen pen(mark, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setBrush(Qt::NoBrush);
    p.setPen(pen);
    switch (kind) {
    case QMessageBox::Warning:
        p.drawLine(QPointF(16, 11), QPointF(16, 19.5));
        p.setPen(Qt::NoPen);
        p.setBrush(mark);
        p.drawEllipse(QPointF(16, 23.5), 1.7, 1.7);
        break;
    case QMessageBox::Critical:
        p.drawLine(QPointF(11, 11), QPointF(21, 21));
        p.drawLine(QPointF(21, 11), QPointF(11, 21));
        break;
    case QMessageBox::Information:
        p.drawLine(QPointF(16, 14.5), QPointF(16, 23));
        p.setPen(Qt::NoPen);
        p.setBrush(mark);
        p.drawEllipse(QPointF(16, 9.5), 1.8, 1.8);
        break;
    default: {   // Question (and a box with no kind)
        QPainterPath q;
        q.moveTo(11.5, 12.5);
        q.arcTo(QRectF(11.5, 8.5, 9, 8), 180, -230);
        q.lineTo(16, 19);
        p.drawPath(q);
        p.setPen(Qt::NoPen);
        p.setBrush(mark);
        p.drawEllipse(QPointF(16, 23), 1.8, 1.8);
        break;
    }
    }
    return pm;
}

void MessageBox::splitText(const QString &text, QString *lead, QString *rest)
{
    // The first sentence: up to a full stop, question mark, or exclamation mark that is outside quotes, is followed by a space and then not by a lower-case letter (a file name such
    // as "plan. xlsx" or "e.g. this" does not end one), or up to a line break.
    qsizetype cut = -1;
    bool quoted = false;
    for (qsizetype i = 0; i < text.size(); ++i) {
        const QChar c = text.at(i);
        if (c == QLatin1Char('"')) quoted = !quoted;
        else if (c == QChar(0x201C)) quoted = true;
        else if (c == QChar(0x201D)) quoted = false;
        if (quoted) continue;
        if (c == QLatin1Char('\n')) { cut = i; break; }
        if ((c == QLatin1Char('.') || c == QLatin1Char('?') || c == QLatin1Char('!')) && i + 1 < text.size() && text.at(i + 1).isSpace()) {
            qsizetype j = i + 1;
            while (j < text.size() && text.at(j).isSpace()) ++j;
            if (j < text.size() && text.at(j).isLower()) continue;
            cut = i + 1;
            break;
        }
    }
    if (cut < 0) {
        *lead = text.trimmed();
        rest->clear();
        return;
    }
    *lead = text.left(cut).trimmed();
    *rest = text.mid(cut).trimmed();
}

MessageBox::MessageBox(QWidget *parent) : QMessageBox(parent) {}
MessageBox::MessageBox(Icon icon, const QString &title, const QString &text, StandardButtons buttons, QWidget *parent) : QMessageBox(icon, title, text, buttons, parent) {}

void MessageBox::showEvent(QShowEvent *e)
{
    restyle();
    QMessageBox::showEvent(e);
}

void MessageBox::restyle()
{
    if (m_styled) return;
    m_styled = true;
    auto *grid = qobject_cast<QGridLayout *>(layout());
    auto *textLabel = findChild<QLabel *>(QStringLiteral("qt_msgbox_label"));
    auto *iconLabel = findChild<QLabel *>(QStringLiteral("qt_msgboxex_icon_label"));
    auto *infoLabel = findChild<QLabel *>(QStringLiteral("qt_msgbox_informativelabel"));
    if (grid && textLabel) {
        int row = 0, col = 0, rowSpan = 1, colSpan = 1;
        grid->getItemPosition(grid->indexOf(textLabel), &row, &col, &rowSpan, &colSpan);
        QString lead, rest;
        const QString info = informativeText().trimmed();
        if (info.isEmpty()) splitText(text(), &lead, &rest);   // (a caller that gives two parts is taken at its word)
        else { lead = text().trimmed(); rest = info; }
        grid->setContentsMargins(20, 20, 20, 16);
        grid->setHorizontalSpacing(6);
        grid->setVerticalSpacing(16);
        auto *box = new QWidget(this);
        auto *v = new QVBoxLayout(box);
        v->setContentsMargins(0, 0, 0, 0);
        v->setSpacing(6);
        auto *head = plainLabel(new QLabel(lead, box));
        if (textFormat() != Qt::PlainText) head->setTextFormat(textFormat());
        head->setWordWrap(true);
        QFont hf = head->font();
        hf.setPixelSize(15);
        hf.setWeight(QFont::DemiBold);
        head->setFont(hf);
        head->setMinimumWidth(340);
        head->setMaximumWidth(460);
        head->setTextInteractionFlags(Qt::TextSelectableByMouse);
        v->addWidget(head);
        if (!rest.isEmpty()) {
            auto *detail = plainLabel(new QLabel(rest, box));
            detail->setTextFormat(head->textFormat());
            detail->setWordWrap(true);
            QFont df = detail->font();
            df.setPixelSize(13);
            detail->setFont(df);
            QPalette dp = detail->palette();
            dp.setColor(QPalette::WindowText, uiMuted());
            detail->setPalette(dp);
            detail->setMaximumWidth(460);
            detail->setTextInteractionFlags(Qt::TextSelectableByMouse);
            v->addWidget(detail);
        }
        v->addStretch(1);
        textLabel->hide();
        if (infoLabel) infoLabel->hide();
        grid->addWidget(box, row, col, rowSpan, colSpan);
        box->show();
    }
    if (iconLabel && grid) {
        int row = 0, col = 0, rowSpan = 1, colSpan = 1;
        grid->getItemPosition(grid->indexOf(iconLabel), &row, &col, &rowSpan, &colSpan);
        auto *sign = new QLabel(this);
        sign->setObjectName(QStringLiteral("jpBoxSign"));
        sign->setPixmap(boxSign(icon(), 32, devicePixelRatioF()));
        sign->setFixedSize(32, 32);
        sign->setAccessibleName(QString());
        iconLabel->hide();
        grid->addWidget(sign, row, col, rowSpan, colSpan, Qt::AlignTop | Qt::AlignHCenter);
        sign->show();
    }
    if (buttons().isEmpty()) addButton(QMessageBox::Ok);   // (the one Qt adds itself when it is shown: styled with the others)
    if (auto *bb = findChild<QDialogButtonBox *>()) {
        arrangeButtons(bb);
    }
}

QString saveChangesText(const QString &name) { return QCoreApplication::translate("SaveChanges", "Save changes to \"%1\"?").arg(name); }
QString saveChangesDetail() { return QCoreApplication::translate("SaveChanges", "If you don't save, your changes since the last save are lost."); }

MessageBox *saveChangesBox(QWidget *parent, const QString &name)
{
    auto *box = new MessageBox(QMessageBox::Warning, appInfo().name, saveChangesText(name), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, parent);
    box->setTextFormat(Qt::PlainText);
    box->setInformativeText(saveChangesDetail());
    box->button(QMessageBox::Discard)->setText(QCoreApplication::translate("SaveChanges", "Don't Save"));
    box->setDefaultButton(QMessageBox::Save);
    return box;
}

QMessageBox::StandardButton askSaveChanges(QWidget *parent, const QString &name)
{
    std::unique_ptr<MessageBox> box(saveChangesBox(parent, name));
    box->exec();
    const auto answer = QMessageBox::StandardButton(box->standardButton(box->clickedButton()));
    return answer == QMessageBox::NoButton ? QMessageBox::Cancel : answer;
}

} // namespace jp
