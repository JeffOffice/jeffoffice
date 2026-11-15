#include "app/uistyle.h"

#include "app/icons.h"
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
#include <QProxyStyle>
#include <QPushButton>
#include <QStyleFactory>
#include <QVBoxLayout>

#include <memory>

namespace jp {

namespace {
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
    QPalette p = l->palette();
    p.setColor(QPalette::WindowText, uiMuted());
    l->setPalette(p);
    l->setProperty("smallLabel", true);
    return l;
}
} // namespace

QLabel *sectionHeading(const QString &text, QWidget *parent)
{
    auto *l = new QLabel(QLocale().toUpper(text), parent);
    l->setTextFormat(Qt::PlainText);
    l->setFont(headingFont(l->font()));
    QPalette p = l->palette();
    p.setColor(QPalette::WindowText, uiMuted());
    l->setPalette(p);
    l->setProperty("sectionHeading", true);
    l->setAccessibleName(text);
    return l;
}

Section::Section(const QString &heading, QWidget *parent) : QGroupBox(QLocale().toUpper(heading), parent), m_heading(heading)
{
    setProperty("section", true);
    setAccessibleName(heading);
    setFlat(true);
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
    QPalette p = b->palette();
    p.setColor(QPalette::ButtonText, uiAccent());
    b->setPalette(p);
    return b;
}

void softPanel(QFrame *f)
{
    f->setFrameShape(QFrame::NoFrame);
    f->setProperty("previewPanel", true);
    f->setAutoFillBackground(true);
    QPalette p = f->palette();
    p.setColor(QPalette::Window, uiPanel());
    f->setPalette(p);
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
    QPalette p = l->palette();
    p.setColor(QPalette::WindowText, uiMuted());
    l->setPalette(p);
    l->setProperty("note", true);
    return l;
}

// ---- buttons ----
namespace {
// The layout of the macOS: Help and Reset at the left, then the rest at the right with Cancel before the accept button; and no icons on buttons.
class ButtonStyle : public QProxyStyle {
public:
    ButtonStyle() : QProxyStyle(QStyleFactory::create(QStringLiteral("Fusion"))) {}
    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *ret) const override
    {
        if (hint == SH_DialogButtonLayout) return QDialogButtonBox::MacLayout;
        if (hint == SH_DialogButtonBox_ButtonsHaveIcons) return 0;
        return QProxyStyle::styleHint(hint, option, widget, ret);
    }
};

QPushButton *theDefault(QDialogButtonBox *box)
{
    QPushButton *accept = nullptr;
    for (QAbstractButton *b : box->buttons()) {
        auto *pb = qobject_cast<QPushButton *>(b);
        if (!pb) continue;
        if (pb->isDefault()) return pb;
        const auto role = box->buttonRole(b);
        if (!accept && (role == QDialogButtonBox::AcceptRole || role == QDialogButtonBox::YesRole)) accept = pb;
    }
    return accept;
}

void moveDestructiveLeft(QDialogButtonBox *box)
{
    auto *lay = qobject_cast<QBoxLayout *>(box->layout());
    if (!lay || box->orientation() != Qt::Horizontal) return;
    int at = 0;
    for (QAbstractButton *b : box->buttons()) {
        const auto role = box->buttonRole(b);
        if (role != QDialogButtonBox::DestructiveRole && role != QDialogButtonBox::ResetRole) continue;
        if (lay->indexOf(b) == at) { ++at; continue; }
        lay->removeWidget(b);
        lay->insertWidget(at++, b);
    }
}

void markPrimary(QDialogButtonBox *box)
{
    for (QAbstractButton *b : box->buttons()) {
        if (b->property("primary").toBool()) return;   // the dialog chose its own
    }
    if (QPushButton *def = theDefault(box)) {
        def->setProperty("primary", true);
        repolish(def);
    }
}
} // namespace

void arrangeButtons(QDialogButtonBox *box)
{
    if (!box->property("jpButtonStyle").toBool()) {
        box->setProperty("jpButtonStyle", true);
        auto *style = new ButtonStyle;
        style->setParent(box);
        box->setStyle(style);
    }
    moveDestructiveLeft(box);
    markPrimary(box);
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

bool ButtonBox::event(QEvent *e)
{
    const bool done = QDialogButtonBox::event(e);
    if (e->type() == QEvent::Show || e->type() == QEvent::StyleChange) arrangeButtons(this);
    return done;
}

namespace {
// The program's own boxes do it by themselves; every other button box (a dialog written before the style, a Qt one) gets it when it is shown.
class ButtonBoxFilter : public QObject {
public:
    using QObject::QObject;
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() == QEvent::Show && o->isWidgetType()) {
            if (auto *dlg = qobject_cast<QDialog *>(o); dlg && !dlg->property("jpDialogStyled").toBool() && !qobject_cast<QMessageBox *>(dlg) &&
                                                      !QByteArrayView(dlg->metaObject()->className()).startsWith("Q")) {
                dlg->setProperty("jpDialogStyled", true);
                // one label column and rows 8 apart in every form of the dialog
                for (QFormLayout *f : dlg->findChildren<QFormLayout *>()) styleForm(f);
            }
            if (auto *box = qobject_cast<QDialogButtonBox *>(o); box && !dynamic_cast<ButtonBox *>(box) && !box->property("jpFiltered").toBool()) {
                box->setProperty("jpFiltered", true);
                QMetaObject::invokeMethod(box, [box] { arrangeButtons(box); }, Qt::QueuedConnection);
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
    const QColor color = red ? uiDanger() : uiAccent();
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
    const QColor mark = uiHighContrast() ? QGuiApplication::palette().color(QPalette::Window) : QColor(Qt::white);
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
    // The first sentence: up to a full stop, question mark, or exclamation mark followed by a space, or up to a line break.
    qsizetype cut = -1;
    for (qsizetype i = 0; i < text.size(); ++i) {
        const QChar c = text.at(i);
        if (c == QLatin1Char('\n')) { cut = i; break; }
        if ((c == QLatin1Char('.') || c == QLatin1Char('?') || c == QLatin1Char('!')) && i + 1 < text.size() && text.at(i + 1).isSpace()) { cut = i + 1; break; }
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
        splitText(text(), &lead, &rest);
        const QString info = informativeText().trimmed();
        if (!info.isEmpty()) rest = rest.isEmpty() ? info : rest + QLatin1Char('\n') + info;
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
    if (auto *bb = findChild<QDialogButtonBox *>()) {
        bb->setProperty("jpFiltered", true);
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
