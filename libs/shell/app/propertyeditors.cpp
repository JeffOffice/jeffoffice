#include "app/propertyeditors.h"

#include "app/icons.h"
#include "app/settings.h"
#include "app/widgets.h"

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QRadioButton>
#include <QShowEvent>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include "app/plaintext.h"

namespace jp {

namespace {

// ---- small helpers

class WheelGuard : public QObject {
public:
    explicit WheelGuard(QWidget *owner) : QObject(owner), m_owner(owner) {}

protected:
    bool eventFilter(QObject *, QEvent *e) override
    {
        if (e->type() != QEvent::Wheel) return false;
        QWidget *focus = QApplication::focusWidget();
        if (m_owner->hasFocus() || (focus && m_owner->isAncestorOf(focus))) return false;
        // The pane scrolls instead.
        e->ignore();
        if (QWidget *parent = m_owner->parentWidget()) QCoreApplication::sendEvent(parent, e);
        return true;
    }

private:
    QWidget *m_owner;
};

// A color well that opens its palette when pressed.
ColorButton *colorWell(const QString &name, const QString &label, bool allowNone, const QString &noneLabel, QWidget *parent)
{
    auto *b = new ColorButton(QString(), label, allowNone, noneLabel, parent);
    b->setObjectName(name);
    b->setPopupMode(QToolButton::InstantPopup);
    b->setWell(true);
    return b;
}

QComboBox *combo(const QString &name, const QStringList &items, QWidget *parent)
{
    auto *c = new QComboBox(parent);
    c->setObjectName(name);
    c->addItems(items);
    ignoreWheelUnlessFocused(c);
    return c;
}

NumberField *percentField(const QString &name, QWidget *parent, bool slider = true)
{
    auto *f = new NumberField(parent);
    f->setObjectName(name);
    f->setRange(0, 100);
    f->setDecimals(0);
    f->setSuffix(QStringLiteral("%"));
    f->setSlider(slider);
    return f;
}

// The stops of a gradient as it draws them: the two colors of one that lists none.
QVector<GradientStop> stopsOf(const Fill &f)
{
    if (!f.stops.isEmpty()) return f.stops;
    return {{0, f.color, f.transparency}, {1, f.color2.isNone() ? ColorRef::rgb(Qt::white) : f.color2, f.transparency}};
}

QColor colorAtStop(const QVector<GradientStop> &stops, double pos, const ColorScheme &s, double *transparency)
{
    QColor out = Qt::black;
    double t = 0;
    if (stops.isEmpty()) {
        if (transparency) *transparency = 0;
        return out;
    }
    if (pos <= stops.first().pos) {
        out = stops.first().color.resolve(s);
        t = stops.first().transparency;
    } else if (pos >= stops.last().pos) {
        out = stops.last().color.resolve(s);
        t = stops.last().transparency;
    } else {
        for (int i = 1; i < stops.size(); ++i)
            if (pos <= stops[i].pos) {
                const GradientStop &a = stops[i - 1], &b = stops[i];
                const double k = b.pos > a.pos ? (pos - a.pos) / (b.pos - a.pos) : 0;
                out = mix(a.color.resolve(s), b.color.resolve(s), k);
                t = a.transparency + (b.transparency - a.transparency) * k;
                break;
            }
    }
    if (transparency) *transparency = t;
    return out;
}

template <class T, class Get> bool allSame(const QVector<T> &items, Get get)
{
    for (int i = 1; i < items.size(); ++i)
        if (!(get(items[i]) == get(items[0]))) return false;
    return true;
}

// Shows a number from the objects: blank when they differ.
template <class T, class Get> void showNumber(NumberField *f, const QVector<T> &items, Get get, double scale = 1.0)
{
    if (items.isEmpty()) return;
    bool same = true;
    for (int i = 1; i < items.size(); ++i)
        if (std::abs(get(items[i]) - get(items[0])) > 1e-6) same = false;
    if (same) f->setValue(get(items[0]) * scale);
    else f->setMixed(true);
}

// Checks one radio button of a group, or none.
void checkOnly(QButtonGroup *group, int id)
{
    group->setExclusive(false);
    for (QAbstractButton *b : group->buttons()) b->setChecked(group->id(b) == id);
    group->setExclusive(true);
}

} // namespace

void ignoreWheelUnlessFocused(QWidget *w)
{
    auto *guard = new WheelGuard(w);
    w->installEventFilter(guard);
    for (QObject *child : w->children())
        if (auto *cw = qobject_cast<QWidget *>(child)) cw->installEventFilter(guard);
    w->setFocusPolicy(Qt::StrongFocus);
}

// ---------------- HoldGate ----------------

HoldGate::HoldGate(QWidget *target, Keys keys) : QObject(target), m_target(target), m_keys(keys)
{
    target->installEventFilter(this);
    for (QWidget *child : target->findChildren<QWidget *>()) child->installEventFilter(this);   // (a spin box's text: the keyboard is there)
}

void HoldGate::end()
{
    const bool was = held();
    m_pressed = m_keyStep = false;
    if (was) Q_EMIT released();
}

bool HoldGate::eventFilter(QObject *o, QEvent *e)
{
    switch (e->type()) {
    case QEvent::MouseButtonPress:
        if (o == m_target && static_cast<QMouseEvent *>(e)->button() == Qt::LeftButton) m_pressed = true;
        break;
    case QEvent::MouseButtonRelease:
        if (o == m_target && m_pressed) {
            m_pressed = false;
            Q_EMIT released();
        }
        break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
        const auto *k = static_cast<QKeyEvent *>(e);
        bool steps = k->key() == Qt::Key_Up || k->key() == Qt::Key_Down || k->key() == Qt::Key_PageUp || k->key() == Qt::Key_PageDown;
        if (m_keys == SliderKeys) steps = steps || k->key() == Qt::Key_Left || k->key() == Qt::Key_Right || k->key() == Qt::Key_Home || k->key() == Qt::Key_End;
        if (!steps || k->isAutoRepeat()) break;
        if (e->type() == QEvent::KeyPress) {
            m_keyStep = true;
        } else if (m_keyStep) {
            m_keyStep = false;
            Q_EMIT released();
        }
        break;
    }
    case QEvent::FocusOut:
        // The keyboard going elsewhere (Tab, a click, another window) ends a hold, whatever is let go of after.
        end();
        break;
    default: break;
    }
    return false;
}

// ---------------- NumberField ----------------

class NumberField::Spin : public QDoubleSpinBox {
public:
    explicit Spin(QWidget *parent) : QDoubleSpinBox(parent) {}
    bool measure = false, mixed = false;
    QString textFromValue(double v) const override
    {
        if (mixed) return QString();
        return measure ? Settings::get().format(v, 3) : QDoubleSpinBox::textFromValue(v);
    }
    double valueFromText(const QString &text) const override
    {
        if (!measure) return QDoubleSpinBox::valueFromText(text);
        double pt = 0;
        return Settings::get().parse(text, &pt) ? pt : value();
    }
    QValidator::State validate(QString &input, int &pos) const override
    {
        if (measure) {
            double pt;
            return Settings::get().parse(input, &pt) ? QValidator::Acceptable : QValidator::Intermediate;
        }
        if (mixed && input.isEmpty()) return QValidator::Acceptable;
        return QDoubleSpinBox::validate(input, pos);
    }
    void stepBy(int steps) override
    {
        mixed = false;
        QDoubleSpinBox::stepBy(steps);
    }
    QLineEdit *edit() const { return findChild<QLineEdit *>(); }
};

NumberField::NumberField(QWidget *parent) : QWidget(parent)
{
    auto *h = new QHBoxLayout(this);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(8);
    m_spin = new Spin(this);
    m_spin->setKeyboardTracking(false);
    m_spin->setAccelerated(true);
    m_spin->setMinimumWidth(78);
    m_spin->setMaximumWidth(110);
    h->addWidget(m_spin);
    h->setAlignment(m_spin, Qt::AlignLeft);
    m_spinGate = new HoldGate(m_spin, HoldGate::SpinKeys);
    connect(m_spinGate, &HoldGate::released, this, [this] { endHold(); });
    ignoreWheelUnlessFocused(m_spin);
    connect(m_spin, &QDoubleSpinBox::valueChanged, this, [this](double v) { spinChanged(v); });
    if (QLineEdit *le = m_spin->edit()) {
        connect(le, &QLineEdit::textEdited, this, [this] {
            m_typing = true;
            m_changedSinceTyping = false;
        });
    }
    connect(m_spin, &QAbstractSpinBox::editingFinished, this, [this] {
        // A number typed that is the one the box already held (a blank one) is a change too.
        if (m_typing && !m_changedSinceTyping && m_mixed) {
            m_mixed = false;
            m_spin->mixed = false;
            Q_EMIT edited(m_spin->value());
            Q_EMIT finished();
        }
        m_typing = false;
        blankIfMixed();
    });
    setFocusProxy(m_spin);
}

void NumberField::setRange(double lo, double hi)
{
    QSignalBlocker block(m_spin);
    m_spin->setRange(lo, hi);
    if (m_slider) m_slider->setRange(int(std::lround(lo * m_sliderScale)), int(std::lround(hi * m_sliderScale)));
}

void NumberField::setDecimals(int n)
{
    m_spin->setDecimals(n);
    m_sliderScale = int(std::pow(10.0, std::min(n, 3)));
    if (m_slider) m_slider->setRange(int(std::lround(m_spin->minimum() * m_sliderScale)), int(std::lround(m_spin->maximum() * m_sliderScale)));
}

void NumberField::setStep(double step) { m_spin->setSingleStep(step); }

void NumberField::setSuffix(const QString &suffix) { m_spin->setSuffix(suffix); }

void NumberField::setMeasure(bool on)
{
    m_spin->measure = on;
    m_spin->setDecimals(6);
    m_spin->setSingleStep(on ? 9 : m_spin->singleStep());
    m_spin->setSuffix(QString());
    m_spin->setValue(m_spin->value());
}

void NumberField::setSlider(bool on)
{
    if (on == (m_slider != nullptr)) return;
    if (!on) {
        delete m_slider;
        m_slider = nullptr;
        return;
    }
    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setMinimumWidth(80);
    m_sliderScale = int(std::pow(10.0, std::min(m_spin->decimals(), 3)));
    m_slider->setRange(int(std::lround(m_spin->minimum() * m_sliderScale)), int(std::lround(m_spin->maximum() * m_sliderScale)));
    m_slider->setValue(int(std::lround(m_spin->value() * m_sliderScale)));
    ignoreWheelUnlessFocused(m_slider);
    auto *h = static_cast<QHBoxLayout *>(layout());
    h->insertWidget(0, m_slider, 1);
    h->setAlignment(m_spin, Qt::Alignment());
    m_sliderGate = new HoldGate(m_slider, HoldGate::SliderKeys);
    connect(m_sliderGate, &HoldGate::released, this, [this] { endHold(); });
    connect(m_slider, &QSlider::valueChanged, this, [this](int i) {
        if (m_loading) return;
        const double v = double(i) / m_sliderScale;
        {
            QSignalBlocker block(m_spin);
            m_spin->mixed = false;
            m_spin->setValue(v);
        }
        m_mixed = false;
        m_unfinished = true;
        Q_EMIT edited(v);
        // (A click on the groove that goes on into a drag is all one hold, in every style.)
        if (!m_slider->isSliderDown() && !m_sliderGate->held()) endHold();
    });
    connect(m_slider, &QSlider::sliderReleased, this, [this] { endHold(); });
}

void NumberField::endHold()
{
    if (!m_unfinished) return;
    m_unfinished = false;
    Q_EMIT finished();
}

void NumberField::setAccessibleLabel(const QString &name)
{
    m_spin->setAccessibleName(name);
    if (m_slider) m_slider->setAccessibleName(name);
}

double NumberField::value() const { return m_spin->value(); }

QDoubleSpinBox *NumberField::box() const { return m_spin; }

bool NumberField::isBusy() const { return m_spinGate->held() || (m_sliderGate && m_sliderGate->held()) || m_typing || (m_slider && m_slider->isSliderDown()); }

void NumberField::setValue(double v)
{
    if (isBusy()) return;
    m_loading = true;
    {
        QSignalBlocker block(m_spin);
        m_spin->mixed = false;
        m_spin->setValue(v);
    }
    if (m_slider) {
        QSignalBlocker block(m_slider);
        m_slider->setValue(int(std::lround(v * m_sliderScale)));
    }
    m_mixed = false;
    m_loading = false;
}

void NumberField::setMixed(bool on)
{
    if (isBusy()) return;
    if (!on) {
        m_mixed = false;
        m_spin->mixed = false;
        setValue(m_spin->value());
        return;
    }
    m_mixed = true;
    m_spin->mixed = true;
    blankIfMixed();
}

void NumberField::blankIfMixed()
{
    if (!m_mixed) return;
    if (QLineEdit *le = m_spin->edit()) {
        QSignalBlocker block(le);
        le->clear();
    }
}

void NumberField::spinChanged(double v)
{
    if (m_loading) return;
    m_mixed = false;
    m_spin->mixed = false;
    m_changedSinceTyping = true;
    if (m_slider) {
        QSignalBlocker block(m_slider);
        m_slider->setValue(int(std::lround(v * m_sliderScale)));
    }
    m_unfinished = true;
    Q_EMIT edited(v);
    // A value typed, or a wheel turned, is finished at once; the arrows held down are many changes, finished when they are let go of.
    if (!m_spinGate->held()) endHold();
}

// The label beside the field (the one that has it as its buddy) names its box for screen readers, which read the box and not the field.
void NumberField::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    if (!m_spin->accessibleName().isEmpty() || !window()) return;
    for (const QLabel *label : window()->findChildren<QLabel *>())
        if (label->buddy() == this) {
            QString name = label->text();
            name.remove(QLatin1Char('&'));
            if (name.endsWith(QLatin1Char(':'))) name.chop(1);
            setAccessibleLabel(name);
            break;
        }
}

// ---------------- FontField ----------------

FontField::FontField(QWidget *parent) : QWidget(parent)
{
    auto *h = new QHBoxLayout(this);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(6);
    m_family = new FontCombo(this);
    m_family->setObjectName(QStringLiteral("fontFamily"));
    m_family->setMaximumWidth(QWIDGETSIZE_MAX);
    m_size = new SizeCombo(this);
    m_size->setObjectName(QStringLiteral("fontSize"));
    h->addWidget(m_family, 1);
    h->addWidget(m_size);
    ignoreWheelUnlessFocused(m_family);
    ignoreWheelUnlessFocused(m_size);
    connect(m_family, &FontCombo::familyChosen, this, [this](const QString &f) {
        Q_EMIT familyChosen(f);
        Q_EMIT finished();
    });
    connect(m_size, &SizeCombo::sizeChosen, this, [this](double pt) {
        Q_EMIT sizeChosen(pt);
        Q_EMIT finished();
    });
}

void FontField::setSchemeFonts(const QString &heading, const QString &body) { m_family->setSchemeFonts(heading, body); }

void FontField::setFamily(const QString &family) { m_family->setCurrentFamily(family); }

void FontField::setSize(double pt) { m_size->setSize(pt); }

// ---------------- TextField ----------------

TextField::TextField(QWidget *parent) : QPlainTextEdit(parent)
{
    setTabChangesFocus(true);
    setFocusPolicy(Qt::StrongFocus);
    connect(this, &QPlainTextEdit::textChanged, this, [this] {
        if (m_loading) return;
        m_dirty = true;
        m_mixed = false;
    });
}

void TextField::setSingleLine(bool on)
{
    m_single = on;
    setLineWrapMode(on ? QPlainTextEdit::NoWrap : QPlainTextEdit::WidgetWidth);
    setHorizontalScrollBarPolicy(on ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(on ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);
    updateGeometry();
}

QSize TextField::sizeHint() const
{
    const int lines = m_single ? 1 : 4;
    return QSize(180, fontMetrics().lineSpacing() * lines + 2 * (frameWidth() + 4) + 2);
}

QSize TextField::minimumSizeHint() const { return QSize(80, sizeHint().height()); }

void TextField::setText(const QString &text)
{
    if (m_dirty) return;
    m_loading = true;
    m_mixed = false;
    if (toPlainText() != text) setPlainText(text);
    m_loading = false;
}

void TextField::setMixed(bool on)
{
    if (m_dirty) return;
    m_loading = true;
    m_mixed = on;
    if (on) clear();
    m_loading = false;
}

void TextField::commit()
{
    if (!m_dirty) return;
    m_dirty = false;
    Q_EMIT edited(m_single ? toPlainText().simplified() : toPlainText());
    Q_EMIT finished();
}

void TextField::focusOutEvent(QFocusEvent *e)
{
    QPlainTextEdit::focusOutEvent(e);
    commit();
}

void TextField::keyPressEvent(QKeyEvent *e)
{
    const bool enter = e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter;
    if (enter && (m_single || (e->modifiers() & Qt::ControlModifier))) {
        commit();
        e->accept();
        return;
    }
    QPlainTextEdit::keyPressEvent(e);
}

// ---------------- GradientBar ----------------

GradientBar::GradientBar(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("gradientStops"));
    setFocusPolicy(Qt::StrongFocus);
    setAccessibleName(tr("Gradient stops"));
    setAccessibleDescription(tr("Left and Right arrows move the stop, Up and Down arrows choose another stop, Insert adds one, and Delete takes it away."));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_stops = {{0, ColorRef::rgb(Qt::black), 0}, {1, ColorRef::rgb(Qt::white), 0}};
}

void GradientBar::setScheme(const ColorScheme &s)
{
    m_scheme = s;
    update();
}

void GradientBar::setStops(const QVector<GradientStop> &stops)
{
    m_stops = stops;
    m_current = std::clamp(m_current, 0, std::max(0, int(m_stops.size()) - 1));
    update();
}

void GradientBar::setCurrent(int i)
{
    i = std::clamp(i, 0, std::max(0, int(m_stops.size()) - 1));
    if (i == m_current) return;
    m_current = i;
    update();
    Q_EMIT currentChanged(i);
}

QRectF GradientBar::barRect() const { return QRectF(12, 4, std::max(10, width() - 24), 20); }

QRectF GradientBar::markerRect(int i) const
{
    const QRectF bar = barRect();
    const double x = bar.left() + std::clamp(m_stops.value(i).pos, 0.0, 1.0) * bar.width();
    return QRectF(x - 6, 27, 12, 15);
}

double GradientBar::posAt(double x) const
{
    const QRectF bar = barRect();
    return std::clamp((x - bar.left()) / bar.width(), 0.0, 1.0);
}

int GradientBar::markerAt(const QPointF &p) const
{
    // The one chosen is on top; then the nearest.
    int best = -1;
    double bestDistance = 1e9;
    for (int i = 0; i < m_stops.size(); ++i) {
        const QRectF r = markerRect(i).adjusted(-3, -4, 3, 3);
        if (!r.contains(p)) continue;
        const double d = std::abs(p.x() - markerRect(i).center().x()) - (i == m_current ? 0.5 : 0);
        if (d < bestDistance) {
            bestDistance = d;
            best = i;
        }
    }
    return best;
}

void GradientBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF bar = barRect();
    // A checkerboard under the colors shows what is clear.
    p.save();
    p.setClipRect(bar);
    for (int y = 0; y < 2; ++y)
        for (int x = 0; x < int(bar.width() / 8) + 1; ++x) p.fillRect(QRectF(bar.left() + x * 8, bar.top() + y * 10, 8, 10), (x + y) % 2 ? QColor(200, 200, 200) : QColor(255, 255, 255));
    QLinearGradient g(bar.left(), 0, bar.right(), 0);
    for (const GradientStop &s : m_stops) {
        QColor c = s.color.resolve(m_scheme);
        c.setAlphaF(float(1 - s.transparency));
        g.setColorAt(std::clamp(s.pos, 0.0, 1.0), c);
    }
    p.fillRect(bar, g);
    p.restore();
    p.setPen(QPen(uiLine(), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(bar, 3, 3);
    for (int pass = 0; pass < 2; ++pass)
        for (int i = 0; i < m_stops.size(); ++i) {
            if ((i == m_current) != (pass == 1)) continue;
            const QRectF m = markerRect(i);
            QPolygonF shape;
            shape << QPointF(m.center().x(), m.top() - 2) << QPointF(m.right(), m.top() + 4) << m.bottomRight() << m.bottomLeft() << QPointF(m.left(), m.top() + 4);
            QColor c = m_stops[i].color.resolve(m_scheme);
            p.setPen(QPen(i == m_current ? uiAccent() : uiText(), i == m_current ? 2 : 1));
            p.setBrush(QColor(c.red(), c.green(), c.blue()));
            p.drawPolygon(shape);
        }
    if (hasFocus()) {
        p.setPen(QPen(focusRingColor(), 2));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 4, 4);
    }
}

void GradientBar::move(int i, double pos)
{
    if (i < 0 || i >= m_stops.size()) return;
    const double lo = i > 0 ? m_stops[i - 1].pos : 0.0, hi = i + 1 < m_stops.size() ? m_stops[i + 1].pos : 1.0;
    pos = std::clamp(pos, lo, hi);
    if (pos == m_stops[i].pos) return;
    m_stops[i].pos = pos;
    update();
    Q_EMIT stopsEdited(m_stops);
}

void GradientBar::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) return;
    setFocus(Qt::MouseFocusReason);
    const int i = markerAt(e->position());
    if (i < 0) return;
    setCurrent(i);
    m_dragging = true;
    m_moved = false;
}

void GradientBar::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_dragging) return;
    const double before = m_stops.value(m_current).pos;
    move(m_current, posAt(e->position().x()));
    if (m_stops.value(m_current).pos != before) m_moved = true;
}

void GradientBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton || !m_dragging) return;
    m_dragging = false;
    if (m_moved) Q_EMIT finished();
    m_moved = false;
}

void GradientBar::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton || markerAt(e->position()) >= 0) return;
    if (!barRect().adjusted(-4, -2, 4, 2).contains(e->position())) return;
    addStopAt(posAt(e->position().x()));
}

void GradientBar::addStopAt(double pos)
{
    double t = 0;
    const QColor c = colorAtStop(m_stops, pos, m_scheme, &t);
    int at = 0;
    while (at < m_stops.size() && m_stops[at].pos <= pos) ++at;
    m_stops.insert(at, GradientStop{pos, ColorRef::rgb(c), t});
    const bool moved = m_current != at;
    m_current = at;
    update();
    Q_EMIT stopsEdited(m_stops);
    if (moved) Q_EMIT currentChanged(at);
    Q_EMIT finished();
}

void GradientBar::removeCurrent()
{
    if (m_stops.size() <= 2) return;
    m_stops.remove(m_current);
    m_current = std::clamp(m_current, 0, int(m_stops.size()) - 1);
    update();
    Q_EMIT stopsEdited(m_stops);
    Q_EMIT currentChanged(m_current);
    Q_EMIT finished();
}

void GradientBar::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right: {
        const double step = (e->modifiers() & Qt::ShiftModifier) ? 0.1 : 0.01;
        const double before = m_stops.value(m_current).pos;
        move(m_current, before + (e->key() == Qt::Key_Right ? step : -step));
        if (m_stops.value(m_current).pos != before) m_keyHeld = true;
        return;
    }
    case Qt::Key_Up: setCurrent(m_current - 1); return;
    case Qt::Key_Down: setCurrent(m_current + 1); return;
    case Qt::Key_Delete:
    case Qt::Key_Backspace: removeCurrent(); return;
    case Qt::Key_Insert: {
        const int next = std::min(m_current + 1, int(m_stops.size()) - 1);
        addStopAt(next == m_current ? std::max(0.0, m_stops[m_current].pos - 0.1) : (m_stops[m_current].pos + m_stops[next].pos) / 2);
        return;
    }
    default: break;
    }
    QWidget::keyPressEvent(e);
}

// The keyboard leaving ends what a marker was being moved by (a release that goes to another widget would never end it).
void GradientBar::focusOutEvent(QFocusEvent *e)
{
    QWidget::focusOutEvent(e);
    const bool was = m_dragging || m_keyHeld;
    const bool moved = m_moved || m_keyHeld;
    m_dragging = m_keyHeld = m_moved = false;
    if (was && moved) Q_EMIT finished();
}

void GradientBar::keyReleaseEvent(QKeyEvent *e)
{
    if ((e->key() == Qt::Key_Left || e->key() == Qt::Key_Right) && !e->isAutoRepeat() && m_keyHeld) {
        m_keyHeld = false;
        Q_EMIT finished();
        return;
    }
    QWidget::keyReleaseEvent(e);
}

// ---------------- GradientEditor ----------------

GradientEditor::GradientEditor(QWidget *parent) : QWidget(parent)
{
    auto *form = new QFormLayout(this);
    form->setContentsMargins(0, 0, 0, 0);
    form->setLabelAlignment(Qt::AlignLeft);
    m_kind = combo(QStringLiteral("gradientType"), {tr("Linear"), tr("Radial"), tr("Rectangular"), tr("Path")}, this);
    form->addRow(tr("Type"), m_kind);
    m_angle = new NumberField(this);
    m_angle->setObjectName(QStringLiteral("gradientAngle"));
    m_angle->setRange(0, 359);
    m_angle->setDecimals(0);
    m_angle->setSuffix(QStringLiteral("°"));
    form->addRow(tr("Angle"), m_angle);
    m_bar = new GradientBar(this);
    form->addRow(m_bar);
    auto *buttons = new QHBoxLayout();
    m_add = new QToolButton(this);
    m_add->setObjectName(QStringLiteral("gradientAdd"));
    m_add->setText(tr("Add Stop"));
    m_add->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_add->setIcon(icon(QStringLiteral("plus")));
    m_remove = new QToolButton(this);
    m_remove->setObjectName(QStringLiteral("gradientRemove"));
    m_remove->setText(tr("Remove Stop"));
    m_remove->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_remove->setIcon(icon(QStringLiteral("minus")));
    buttons->addWidget(m_add);
    buttons->addWidget(m_remove);
    buttons->addStretch(1);
    form->addRow(buttons);
    m_color = colorWell(QStringLiteral("stopColor"), tr("Color"), false, QString(), this);
    form->addRow(tr("Color"), m_color);
    m_position = percentField(QStringLiteral("stopPosition"), this);
    form->addRow(tr("Position"), m_position);
    m_transparency = percentField(QStringLiteral("stopTransparency"), this);
    form->addRow(tr("Transparency"), m_transparency);

    connect(m_kind, &QComboBox::currentIndexChanged, this, [this](int k) {
        if (m_loading || k < 0) return;
        Q_EMIT changed([k](Fill &f) { f.gradType = Fill::GradType(k); });
        Q_EMIT finished();
        showAngle(k == int(Fill::Linear));
    });
    connect(m_angle, &NumberField::edited, this, [this](double v) { Q_EMIT changed([v](Fill &f) { f.angle = v; }); });
    connect(m_angle, &NumberField::finished, this, &GradientEditor::finished);
    connect(m_bar, &GradientBar::stopsEdited, this, [this](const QVector<GradientStop> &stops) { stopsEdited(stops); });
    connect(m_bar, &GradientBar::currentChanged, this, [this] { showStop(); });
    connect(m_bar, &GradientBar::finished, this, &GradientEditor::finished);
    connect(m_add, &QToolButton::clicked, this, [this] {
        const int i = m_bar->current(), n = int(m_shown.size());
        if (n == 0 || m_stops != Stops::Identical) return;
        const int next = i + 1 < n ? i + 1 : i - 1;
        const double pos = next >= 0 && next != i ? (m_shown[i].pos + m_shown[next].pos) / 2 : m_shown[i].pos;
        m_bar->addStopAt(pos);
    });
    connect(m_remove, &QToolButton::clicked, this, [this] {
        if (m_stops == Stops::Identical) m_bar->removeCurrent();
    });
    connect(m_color, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
        editStop([c](GradientStop &st) { st.color = c; }, true);
    });
    connect(m_position, &NumberField::edited, this, [this](double v) { editStop([v](GradientStop &st) { st.pos = std::clamp(v / 100, 0.0, 1.0); }, false); });
    connect(m_position, &NumberField::finished, this, &GradientEditor::finished);
    connect(m_transparency, &NumberField::edited, this, [this](double v) { editStop([v](GradientStop &st) { st.transparency = std::clamp(v / 100, 0.0, 1.0); }, false); });
    connect(m_transparency, &NumberField::finished, this, &GradientEditor::finished);
}

void GradientEditor::showAngle(bool linear) { static_cast<QFormLayout *>(layout())->setRowVisible(m_angle, linear); }

void GradientEditor::setScheme(const ColorScheme &s)
{
    m_scheme = s;
    m_bar->setScheme(s);
    m_color->setScheme(s);
}

namespace {

bool sameStops(const QVector<GradientStop> &a, const QVector<GradientStop> &b)
{
    if (a.size() != b.size()) return false;
    for (int i = 0; i < a.size(); ++i)
        if (std::abs(a[i].pos - b[i].pos) > 1e-9 || a[i].color != b[i].color || std::abs(a[i].transparency - b[i].transparency) > 1e-9) return false;
    return true;
}

// Changes one property of the stop at `index` in each fill that has it, and keeps the stops in order: the stop moves no further than its neighbors.
FillChange stopChange(int index, const std::function<void(GradientStop &)> &change)
{
    return [index, change](Fill &f) {
        QVector<GradientStop> stops = stopsOf(f);
        if (index < 0 || index >= stops.size()) return;
        change(stops[index]);
        stops[index].pos = std::clamp(stops[index].pos, index > 0 ? stops[index - 1].pos : 0.0, index + 1 < stops.size() ? stops[index + 1].pos : 1.0);
        f.stops = stops;
        f.color = stops.first().color;
        f.color2 = stops.last().color;
    };
}

} // namespace

void GradientEditor::setGradients(const QVector<Fill> &fills)
{
    if (fills.isEmpty()) return;
    m_loading = true;
    m_fills = fills;
    if (allSame(fills, [](const Fill &f) { return int(f.gradType); })) m_kind->setCurrentIndex(int(fills.first().gradType));
    else m_kind->setCurrentIndex(-1);
    const bool linear = m_kind->currentIndex() == int(Fill::Linear) || m_kind->currentIndex() < 0;
    showAngle(linear);
    showNumber(m_angle, fills, [](const Fill &f) { return f.angle; });
    // The stops: those of the first, always as they are now (a selection that changed, an undo, or Select All shows the new ones); what the others
    // have is the mode.
    const QVector<GradientStop> first = stopsOf(fills.first());
    m_stops = Stops::Identical;
    for (const Fill &f : fills) {
        const QVector<GradientStop> other = stopsOf(f);
        if (other.size() != first.size()) {
            m_stops = Stops::Different;
            break;
        }
        if (!sameStops(other, first)) m_stops = Stops::SameCount;
    }
    m_shown = first;
    m_bar->setStops(m_shown);
    m_bar->setEnabled(m_stops != Stops::Different);
    m_loading = false;
    showStop();
}

void GradientEditor::showStop()
{
    const int i = m_bar->current();
    const bool has = m_stops != Stops::Different && i >= 0 && i < m_shown.size();
    m_loading = true;
    m_color->setEnabled(has);
    m_position->setEnabled(has);
    m_transparency->setEnabled(has);
    m_add->setEnabled(m_stops == Stops::Identical);
    m_remove->setEnabled(m_stops == Stops::Identical && m_shown.size() > 2);
    if (has) {
        m_color->setScheme(m_scheme);
        // Each shows the stop's own property, or blank where the fills' stops differ in it.
        QVector<GradientStop> chosen;
        for (const Fill &f : std::as_const(m_fills)) {
            const QVector<GradientStop> stops = stopsOf(f);
            if (i < stops.size()) chosen << stops[i];
        }
        if (allSame(chosen, [](const GradientStop &st) { return st.color; })) m_color->setCurrent(m_shown[i].color);
        else m_color->setMixed(true);
        showNumber(m_position, chosen, [](const GradientStop &st) { return std::round(st.pos * 100); });
        showNumber(m_transparency, chosen, [](const GradientStop &st) { return std::round(st.transparency * 100); });
    }
    m_loading = false;
}

void GradientEditor::stopsEdited(const QVector<GradientStop> &stops)
{
    if (m_stops == Stops::Different) return;
    if (m_stops == Stops::SameCount) {
        // Stops can't be added or taken away from fills whose stops differ (the bar goes back); a marker moved moves that stop of each.
        if (stops.size() != m_shown.size()) {
            m_bar->setStops(m_shown);
            return;
        }
        const int i = std::clamp(m_bar->current(), 0, int(stops.size()) - 1);
        const double pos = stops[i].pos;
        m_shown = stops;
        showStop();
        Q_EMIT changed(stopChange(i, [pos](GradientStop &st) { st.pos = pos; }));
        return;
    }
    m_shown = stops;
    showStop();
    Q_EMIT changed([stops](Fill &f) {
        f.stops = stops;
        if (!stops.isEmpty()) {
            f.color = stops.first().color;
            f.color2 = stops.last().color;
        }
    });
}

void GradientEditor::editStop(const std::function<void(GradientStop &)> &change, bool last)
{
    const int i = m_bar->current();
    if (m_stops == Stops::Different || i < 0 || i >= m_shown.size()) return;
    if (m_stops == Stops::SameCount) {
        change(m_shown[i]);
        m_bar->setStops(m_shown);
        Q_EMIT changed(stopChange(i, change));
        if (last) Q_EMIT finished();
        return;
    }
    QVector<GradientStop> stops = m_shown;
    change(stops[i]);
    // A stop moved past another takes its place in the order; the bar follows the one that was chosen.
    const GradientStop moved = stops[i];
    std::stable_sort(stops.begin(), stops.end(), [](const GradientStop &a, const GradientStop &b) { return a.pos < b.pos; });
    int at = 0;
    for (int k = 0; k < stops.size(); ++k)
        if (stops[k].pos == moved.pos && stops[k].color == moved.color && stops[k].transparency == moved.transparency) at = k;
    m_bar->setStops(stops);
    m_bar->setCurrent(at);
    stopsEdited(stops);
    if (last) Q_EMIT finished();
}

// ---------------- FillEditor ----------------

namespace {

// The name of each of the 13 patterns, in the order of patternBrushes().
QString patternName(int i)
{
    static const char *const names[] = {"90%", "80%", "70%", "50%", "25%", "10%", "5%"};
    if (i < 7) return QStringLiteral("%1").arg(QLatin1String(names[i]));
    switch (i) {
    case 7: return QCoreApplication::translate("FillEditor", "Horizontal");
    case 8: return QCoreApplication::translate("FillEditor", "Vertical");
    case 9: return QCoreApplication::translate("FillEditor", "Cross");
    case 10: return QCoreApplication::translate("FillEditor", "Upward Diagonal");
    case 11: return QCoreApplication::translate("FillEditor", "Downward Diagonal");
    default: return QCoreApplication::translate("FillEditor", "Diagonal Cross");
    }
}

QIcon patternIcon(int pattern)
{
    Fill f;
    f.type = Fill::Pattern;
    f.pattern = pattern;
    f.color = ColorRef::rgb(Qt::black);
    f.color2 = ColorRef::rgb(Qt::white);
    QImage tile = f.patternTile(ColorScheme());
    QPixmap pm(28, 28);
    pm.fill(Qt::white);
    QPainter p(&pm);
    p.fillRect(pm.rect(), QBrush(tile));
    p.setPen(QColor(120, 120, 120));
    p.setBrush(Qt::NoBrush);
    p.drawRect(pm.rect().adjusted(0, 0, -1, -1));
    return QIcon(pm);
}

} // namespace

FillEditor::FillEditor(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("fillEditor"));
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(6);
    auto *group = new QButtonGroup(this);
    const struct { const char *name; QString text; } choices[5] = {{"fillNone", tr("No fill")}, {"fillSolid", tr("Solid fill")}, {"fillGradient", tr("Gradient fill")},
                                                                  {"fillPicture", tr("Picture or texture fill")}, {"fillPattern", tr("Pattern fill")}};
    for (int i = 0; i < 5; ++i) {
        m_radios[i] = new QRadioButton(choices[i].text, this);
        m_radios[i]->setObjectName(QLatin1String(choices[i].name));
        group->addButton(m_radios[i], i);
        v->addWidget(m_radios[i]);
        connect(m_radios[i], &QRadioButton::clicked, this, [this, i] { typeChosen(i == 4 ? Fill::Pattern : Fill::Type(i)); });
    }
    m_group = group;
    m_pages = new QStackedWidget(this);
    v->addWidget(m_pages);

    m_pages->addWidget(new QWidget(this));   // none

    auto *solid = new QWidget(this);
    auto *sf = new QFormLayout(solid);
    sf->setContentsMargins(0, 4, 0, 0);
    m_color = colorWell(QStringLiteral("fillColor"), tr("Fill color"), false, QString(), solid);
    sf->addRow(tr("Color"), m_color);
    m_transparency = percentField(QStringLiteral("fillTransparency"), solid);
    sf->addRow(tr("Transparency"), m_transparency);
    m_pages->addWidget(solid);

    m_gradient = new GradientEditor(this);
    m_gradient->setObjectName(QStringLiteral("fillGradientEditor"));
    m_gradient->layout()->setContentsMargins(0, 4, 0, 0);
    m_pages->addWidget(m_gradient);

    auto *picture = new QWidget(this);
    auto *pv = new QVBoxLayout(picture);
    pv->setContentsMargins(0, 4, 0, 0);
    auto *row = new QHBoxLayout();
    auto *insert = new QToolButton(picture);
    insert->setObjectName(QStringLiteral("pictureInsert"));
    insert->setText(tr("Insert…"));
    insert->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    insert->setIcon(icon(QStringLiteral("image-plus")));
    row->addWidget(insert);
    m_thumb = plainLabel(new QLabel(picture));
    m_thumb->setObjectName(QStringLiteral("pictureThumb"));
    m_thumb->setFixedSize(96, 64);
    m_thumb->setFrameShape(QFrame::StyledPanel);
    m_thumb->setAlignment(Qt::AlignCenter);
    row->addWidget(m_thumb);
    row->addStretch(1);
    pv->addLayout(row);
    m_tile = new QCheckBox(tr("Tile picture as texture"), picture);
    m_tile->setObjectName(QStringLiteral("pictureTile"));
    pv->addWidget(m_tile);
    auto *pf = new QFormLayout();
    m_tileScale = new NumberField(picture);
    m_tileScale->setObjectName(QStringLiteral("pictureScale"));
    m_tileScale->setRange(1, 1000);
    m_tileScale->setDecimals(0);
    m_tileScale->setSuffix(QStringLiteral("%"));
    pf->addRow(tr("Scale"), m_tileScale);
    m_pictureTransparency = percentField(QStringLiteral("pictureTransparency"), picture);
    pf->addRow(tr("Transparency"), m_pictureTransparency);
    pv->addLayout(pf);
    m_pages->addWidget(picture);

    auto *pattern = new QWidget(this);
    auto *pa = new QVBoxLayout(pattern);
    pa->setContentsMargins(0, 4, 0, 0);
    auto *grid = new QGridLayout();
    grid->setSpacing(2);
    auto *patterns = new QButtonGroup(this);
    for (int i = 0; i < 13; ++i) {
        auto *b = new QToolButton(pattern);
        b->setObjectName(QStringLiteral("pattern%1").arg(i));
        b->setCheckable(true);
        b->setIcon(patternIcon(i));
        b->setIconSize(QSize(28, 28));
        b->setToolTip(jp::plainToolTip(patternName(i)));
        b->setAccessibleName(patternName(i));
        patterns->addButton(b, i);
        grid->addWidget(b, i / 7, i % 7);
        m_patterns << b;
        connect(b, &QToolButton::clicked, this, [this, i] {
            if (m_loading) return;
            Q_EMIT changed([i](Fill &f) {
                if (f.type != Fill::Pattern) {
                    const ColorRef was = f.type == Fill::Gradient && !f.stops.isEmpty() ? f.stops.first().color : f.color;
                    f.type = Fill::Pattern;
                    f.color = was.isNone() ? ColorRef::scheme(Accent1) : was;
                    f.color2 = ColorRef::rgb(Qt::white);
                }
                f.pattern = i;
            });
            Q_EMIT finished();
        });
    }
    pa->addLayout(grid);
    auto *paf = new QFormLayout();
    m_fore = colorWell(QStringLiteral("patternFore"), tr("Foreground color"), false, QString(), pattern);
    m_back = colorWell(QStringLiteral("patternBack"), tr("Background color"), true, tr("No Color"), pattern);
    paf->addRow(tr("Foreground"), m_fore);
    paf->addRow(tr("Background"), m_back);
    pa->addLayout(paf);
    m_pages->addWidget(pattern);

    connect(m_color, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
        if (m_loading) return;
        Q_EMIT changed([c](Fill &f) { f.color = c; });
        Q_EMIT finished();
    });
    connect(m_transparency, &NumberField::edited, this, [this](double v) { Q_EMIT changed([v](Fill &f) { f.transparency = v / 100; }); });
    connect(m_transparency, &NumberField::finished, this, &FillEditor::finished);
    connect(m_pictureTransparency, &NumberField::edited, this, [this](double v) { Q_EMIT changed([v](Fill &f) { f.transparency = v / 100; }); });
    connect(m_pictureTransparency, &NumberField::finished, this, &FillEditor::finished);
    connect(m_gradient, &GradientEditor::changed, this, &FillEditor::changed);
    connect(m_gradient, &GradientEditor::finished, this, &FillEditor::finished);
    connect(insert, &QToolButton::clicked, this, &FillEditor::pictureRequested);
    connect(m_tile, &QCheckBox::clicked, this, [this](bool on) {
        Q_EMIT changed([on](Fill &f) { f.tile = on; });
        Q_EMIT finished();
    });
    connect(m_tileScale, &NumberField::edited, this, [this](double v) { Q_EMIT changed([v](Fill &f) { f.tileScale = v / 100; }); });
    connect(m_tileScale, &NumberField::finished, this, &FillEditor::finished);
    connect(m_fore, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
        Q_EMIT changed([c](Fill &f) { f.color = c; });
        Q_EMIT finished();
    });
    connect(m_back, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
        Q_EMIT changed([c](Fill &f) { f.color2 = c; });
        Q_EMIT finished();
    });
    setFills({});
}

void FillEditor::setScheme(const ColorScheme &s)
{
    m_scheme = s;
    for (ColorButton *b : {m_color, m_fore, m_back}) b->setScheme(s);
    m_gradient->setScheme(s);
}

void FillEditor::setFills(const QVector<Fill> &fills)
{
    m_fills = fills;
    showFills();
}

void FillEditor::showFills()
{
    setEnabled(!m_fills.isEmpty());
    m_loading = true;
    // The radio button of each: none, solid, gradient, picture or texture, pattern.
    auto typeOf = [](const Fill &f) { return f.isNone() ? 0 : f.type == Fill::Texture ? 3 : f.type == Fill::Pattern ? 4 : int(f.type); };
    int type = -1;
    if (!m_fills.isEmpty() && allSame(m_fills, typeOf)) type = typeOf(m_fills.first());
    checkOnly(m_group, type);
    m_pages->setVisible(type >= 0);
    if (type >= 0) m_pages->setCurrentIndex(type);
    switch (type) {
    case 1:
        if (allSame(m_fills, [](const Fill &f) { return f.color; })) m_color->setCurrent(m_fills.first().color);
        else m_color->setMixed(true);
        showNumber(m_transparency, m_fills, [](const Fill &f) { return f.transparency; }, 100);
        break;
    case 2: {
        QVector<Fill> g;
        for (const Fill &f : m_fills) g << f;
        m_gradient->setGradients(g);
        break;
    }
    case 3: {
        const bool tiled = allSame(m_fills, [](const Fill &f) { return f.type == Fill::Texture || f.tile; });
        m_tile->setTristate(false);
        m_tile->setChecked(tiled && (m_fills.first().type == Fill::Texture || m_fills.first().tile));
        m_tileScale->setEnabled(m_tile->isChecked());
        showNumber(m_tileScale, m_fills, [](const Fill &f) { return f.tileScale; }, 100);
        showNumber(m_pictureTransparency, m_fills, [](const Fill &f) { return f.transparency; }, 100);
        // The thumbnail is made once for each picture (the decoded picture is the same one while it is the same), not at every refresh.
        QPixmap pm;
        if (m_images && !m_fills.first().imageId.isEmpty()) {
            const QImage im = m_images(m_fills.first().imageId);
            if (!im.isNull() && im.cacheKey() == m_thumbnailOf && !m_thumbnail.isNull()) {
                pm = m_thumbnail;
            } else if (!im.isNull()) {
                pm = QPixmap::fromImage(im.scaled(m_thumb->size() - QSize(4, 4), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                m_thumbnail = pm;
                m_thumbnailOf = im.cacheKey();
            }
        }
        if (pm.cacheKey() != m_thumb->pixmap(Qt::ReturnByValue).cacheKey()) m_thumb->setPixmap(pm);
        break;
    }
    case 4: {
        const int pattern = allSame(m_fills, [](const Fill &f) { return f.pattern; }) ? m_fills.first().pattern : -1;
        for (int i = 0; i < m_patterns.size(); ++i) m_patterns[i]->setChecked(i == pattern);
        if (allSame(m_fills, [](const Fill &f) { return f.color; })) m_fore->setCurrent(m_fills.first().color);
        else m_fore->setMixed(true);
        if (allSame(m_fills, [](const Fill &f) { return f.color2; })) m_back->setCurrent(m_fills.first().color2);
        else m_back->setMixed(true);
        break;
    }
    default: break;
    }
    m_loading = false;
}

void FillEditor::typeChosen(Fill::Type type)
{
    if (m_loading) return;
    switch (type) {
    case Fill::NoFill:
        Q_EMIT changed([](Fill &f) { f = Fill(); });
        Q_EMIT finished();
        break;
    case Fill::Solid:
        Q_EMIT changed([](Fill &f) {
            if (f.type == Fill::Solid && !f.color.isNone()) return;
            ColorRef c = f.type == Fill::Gradient && !f.stops.isEmpty() ? f.stops.first().color : f.color;
            if (c.isNone()) c = ColorRef::scheme(Accent1);
            f = Fill::solid(c, f.type == Fill::Gradient ? 0 : f.transparency);
        });
        Q_EMIT finished();
        break;
    case Fill::Gradient: {
        const ColorScheme scheme = m_scheme;
        Q_EMIT changed([scheme](Fill &f) {
            if (f.type == Fill::Gradient) return;
            ColorRef c = f.color;
            if (c.isNone()) c = ColorRef::scheme(Accent1);
            const double t = f.type == Fill::Solid ? f.transparency : 0;
            Fill g;
            g.type = Fill::Gradient;
            g.gradType = Fill::Linear;
            g.angle = 90;
            // From the color to a lighter one of it.
            const ColorRef lighter = ColorRef::rgb(mix(c.resolve(scheme), Qt::white, 0.6));
            g.stops = {{0, c, t}, {1, lighter, t}};
            g.color = c;
            g.color2 = lighter;
            f = g;
        });
        Q_EMIT finished();
        break;
    }
    case Fill::Picture:
    case Fill::Texture: {
        bool already = true;
        for (const Fill &f : std::as_const(m_fills)) already = already && (f.type == Fill::Picture || f.type == Fill::Texture) && !f.imageId.isEmpty();
        if (!already) Q_EMIT pictureRequested();
        showFills();
        break;
    }
    case Fill::Pattern:
        Q_EMIT changed([](Fill &f) {
            if (f.type == Fill::Pattern) return;
            const ColorRef was = f.type == Fill::Gradient && !f.stops.isEmpty() ? f.stops.first().color : f.color;
            f = Fill();
            f.type = Fill::Pattern;
            f.pattern = 6;
            f.color = was.isNone() ? ColorRef::scheme(Accent1) : was;
            f.color2 = ColorRef::rgb(Qt::white);
        });
        Q_EMIT finished();
        break;
    }
}

// ---------------- LineEditor ----------------

LineEditor::LineEditor(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("lineEditor"));
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(6);
    auto *group = new QButtonGroup(this);
    const struct { const char *name; QString text; } choices[3] = {{"lineNone", tr("No line")}, {"lineSolid", tr("Solid line")}, {"lineGradient", tr("Gradient line")}};
    for (int i = 0; i < 3; ++i) {
        m_radios[i] = new QRadioButton(choices[i].text, this);
        m_radios[i]->setObjectName(QLatin1String(choices[i].name));
        group->addButton(m_radios[i], i);
        v->addWidget(m_radios[i]);
        connect(m_radios[i], &QRadioButton::clicked, this, [this, i] { typeChosen(i); });
    }
    m_group = group;
    m_details = new QWidget(this);
    auto *form = new QFormLayout(m_details);
    form->setContentsMargins(0, 4, 0, 0);
    m_form = form;
    m_color = colorWell(QStringLiteral("lineColor"), tr("Line color"), false, QString(), m_details);
    form->addRow(tr("Color"), m_color);
    m_transparency = percentField(QStringLiteral("lineTransparency"), m_details);
    form->addRow(tr("Transparency"), m_transparency);
    m_gradient = new GradientEditor(m_details);
    m_gradient->setObjectName(QStringLiteral("lineGradientEditor"));
    m_gradient->layout()->setContentsMargins(0, 0, 0, 0);
    form->addRow(m_gradient);
    m_width = new NumberField(m_details);
    m_width->setObjectName(QStringLiteral("lineWidth"));
    m_width->setRange(0, 1584);
    m_width->setDecimals(2);
    m_width->setStep(0.25);
    m_width->setSuffix(tr(" pt"));
    form->addRow(tr("Width"), m_width);
    m_compound = combo(QStringLiteral("lineCompound"), {tr("Single"), tr("Double"), tr("Thick Thin"), tr("Thin Thick"), tr("Triple")}, m_details);
    form->addRow(tr("Compound type"), m_compound);
    QStringList dashes;
    for (int d = 0; d <= int(Stroke::LongDashDotDot); ++d) dashes << dashName(Stroke::Dash(d));
    m_dash = combo(QStringLiteral("lineDash"), dashes, m_details);
    for (int d = 0; d < m_dash->count(); ++d) {
        Stroke s = Stroke::line(ColorRef::rgb(Qt::black), 1.5);
        s.dash = Stroke::Dash(d);
        QPixmap pm(64, 12);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.setPen(s.pen(ColorScheme()));
        p.drawLine(2, 6, 62, 6);
        m_dash->setItemIcon(d, QIcon(pm));
    }
    m_dash->setIconSize(QSize(64, 12));
    form->addRow(tr("Dash type"), m_dash);
    m_cap = combo(QStringLiteral("lineCap"), {tr("Flat"), tr("Round"), tr("Square")}, m_details);
    form->addRow(tr("Cap type"), m_cap);
    m_join = combo(QStringLiteral("lineJoin"), {tr("Round"), tr("Bevel"), tr("Miter")}, m_details);
    form->addRow(tr("Join type"), m_join);
    m_arrows = new QWidget(m_details);
    auto *af = new QFormLayout(m_arrows);
    af->setContentsMargins(0, 0, 0, 0);
    const QStringList types = {tr("None"), tr("Triangle"), tr("Open"), tr("Stealth"), tr("Diamond"), tr("Oval")};
    const QStringList sizes = {tr("Small"), tr("Medium"), tr("Large")};
    m_beginType = combo(QStringLiteral("lineBeginType"), types, m_arrows);
    m_beginSize = combo(QStringLiteral("lineBeginSize"), sizes, m_arrows);
    m_endType = combo(QStringLiteral("lineEndType"), types, m_arrows);
    m_endSize = combo(QStringLiteral("lineEndSize"), sizes, m_arrows);
    af->addRow(tr("Begin arrow type"), m_beginType);
    af->addRow(tr("Begin arrow size"), m_beginSize);
    af->addRow(tr("End arrow type"), m_endType);
    af->addRow(tr("End arrow size"), m_endSize);
    form->addRow(m_arrows);
    v->addWidget(m_details);

    auto edit = [this](const StrokeChange &change) {
        if (m_loading) return;
        Q_EMIT changed(change);
    };
    auto editAndFinish = [this](const StrokeChange &change) {
        if (m_loading) return;
        Q_EMIT changed(change);
        Q_EMIT finished();
    };
    connect(m_color, &ColorButton::colorPicked, this, [editAndFinish](const ColorRef &c) {
        editAndFinish([c](Stroke &s) { s.color = c; });
    });
    connect(m_transparency, &NumberField::edited, this, [edit](double t) { edit([t](Stroke &s) { s.transparency = t / 100; }); });
    connect(m_transparency, &NumberField::finished, this, &LineEditor::finished);
    connect(m_width, &NumberField::edited, this, [edit](double w) { edit([w](Stroke &s) { s.width = w; }); });
    connect(m_width, &NumberField::finished, this, &LineEditor::finished);
    connect(m_compound, &QComboBox::currentIndexChanged, this, [editAndFinish](int k) {
        if (k >= 0) editAndFinish([k](Stroke &s) { s.compound = Stroke::Compound(k); });
    });
    connect(m_dash, &QComboBox::currentIndexChanged, this, [editAndFinish](int k) {
        if (k >= 0) editAndFinish([k](Stroke &s) { s.dash = Stroke::Dash(k); });
    });
    connect(m_cap, &QComboBox::currentIndexChanged, this, [editAndFinish](int k) {
        if (k >= 0) editAndFinish([k](Stroke &s) { s.cap = k == 1 ? Qt::RoundCap : k == 2 ? Qt::SquareCap : Qt::FlatCap; });
    });
    connect(m_join, &QComboBox::currentIndexChanged, this, [editAndFinish](int k) {
        if (k >= 0) editAndFinish([k](Stroke &s) { s.join = k == 0 ? Qt::RoundJoin : k == 1 ? Qt::BevelJoin : Qt::MiterJoin; });
    });
    connect(m_beginType, &QComboBox::currentIndexChanged, this, [editAndFinish](int k) {
        if (k >= 0) editAndFinish([k](Stroke &s) { s.startArrow = Arrow(k); });
    });
    connect(m_beginSize, &QComboBox::currentIndexChanged, this, [editAndFinish](int k) {
        if (k >= 0) editAndFinish([k](Stroke &s) { s.startSize = k; });
    });
    connect(m_endType, &QComboBox::currentIndexChanged, this, [editAndFinish](int k) {
        if (k >= 0) editAndFinish([k](Stroke &s) { s.endArrow = Arrow(k); });
    });
    connect(m_endSize, &QComboBox::currentIndexChanged, this, [editAndFinish](int k) {
        if (k >= 0) editAndFinish([k](Stroke &s) { s.endSize = k; });
    });
    connect(m_gradient, &GradientEditor::changed, this, [this](const FillChange &change) {
        if (m_loading) return;
        Q_EMIT changed([change](Stroke &s) {
            if (!s.hasGradient()) return;
            change(s.gradient);
            if (!s.gradient.stops.isEmpty()) s.color = s.gradient.stops.first().color;
        });
    });
    connect(m_gradient, &GradientEditor::finished, this, [this] {
        if (!m_loading) Q_EMIT finished();
    });
    setStrokes({});
}

void LineEditor::setScheme(const ColorScheme &s)
{
    m_scheme = s;
    m_color->setScheme(s);
    m_gradient->setScheme(s);
}

void LineEditor::setArrowsShown(bool shown) { m_arrows->setVisible(shown); }

void LineEditor::setStrokes(const QVector<Stroke> &strokes)
{
    m_strokes = strokes;
    showStrokes();
}

void LineEditor::showStrokes()
{
    setEnabled(!m_strokes.isEmpty());
    m_loading = true;
    auto typeOf = [](const Stroke &s) { return s.isNone() ? 0 : s.hasGradient() ? 2 : 1; };
    int type = -1;
    if (!m_strokes.isEmpty() && allSame(m_strokes, typeOf)) type = typeOf(m_strokes.first());
    checkOnly(m_group, type);
    m_details->setVisible(type > 0);
    m_form->setRowVisible(m_color, type == 1);
    m_form->setRowVisible(m_transparency, type == 1);
    m_form->setRowVisible(m_gradient, type == 2);
    if (type > 0) {
        if (type == 1) {
            if (allSame(m_strokes, [](const Stroke &s) { return s.color; })) m_color->setCurrent(m_strokes.first().color);
            else m_color->setMixed(true);
            showNumber(m_transparency, m_strokes, [](const Stroke &s) { return s.transparency; }, 100);
        } else {
            QVector<Fill> g;
            for (const Stroke &s : m_strokes) g << s.gradient;
            m_gradient->setGradients(g);
        }
        showNumber(m_width, m_strokes, [](const Stroke &s) { return s.width; });
        auto choose = [&](QComboBox *box, auto get) {
            if (allSame(m_strokes, get)) box->setCurrentIndex(get(m_strokes.first()));
            else box->setCurrentIndex(-1);
        };
        choose(m_compound, [](const Stroke &s) { return int(s.compound); });
        choose(m_dash, [](const Stroke &s) { return int(s.dash); });
        choose(m_cap, [](const Stroke &s) { return s.cap == Qt::RoundCap ? 1 : s.cap == Qt::SquareCap ? 2 : 0; });
        choose(m_join, [](const Stroke &s) { return s.join == Qt::RoundJoin ? 0 : s.join == Qt::BevelJoin ? 1 : 2; });
        choose(m_beginType, [](const Stroke &s) { return int(s.startArrow); });
        choose(m_beginSize, [](const Stroke &s) { return std::clamp(s.startSize, 0, 2); });
        choose(m_endType, [](const Stroke &s) { return int(s.endArrow); });
        choose(m_endSize, [](const Stroke &s) { return std::clamp(s.endSize, 0, 2); });
    }
    m_loading = false;
}

void LineEditor::typeChosen(int type)
{
    if (m_loading) return;
    if (type == 0) {
        Q_EMIT changed([](Stroke &s) {
            s.color = ColorRef();
            s.gradient = Fill();
        });
    } else if (type == 1) {
        Q_EMIT changed([](Stroke &s) {
            s.gradient = Fill();
            if (s.color.isNone()) s.color = ColorRef::scheme(Main);
            if (s.width <= 0) s.width = 0.75;
        });
    } else {
        const ColorScheme scheme = m_scheme;
        Q_EMIT changed([scheme](Stroke &s) {
            if (s.hasGradient()) return;
            const ColorRef c = s.color.isNone() ? ColorRef::scheme(Accent1) : s.color;
            const ColorRef lighter = ColorRef::rgb(mix(c.resolve(scheme), Qt::white, 0.6));
            Fill g;
            g.type = Fill::Gradient;
            g.gradType = Fill::Linear;
            g.angle = 0;
            g.stops = {{0, c, 0}, {1, lighter, 0}};
            g.color = c;
            g.color2 = lighter;
            s.gradient = g;
            s.color = c;
            if (s.width <= 0) s.width = 0.75;
        });
    }
    Q_EMIT finished();
}

} // namespace jp
