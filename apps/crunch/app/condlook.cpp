#include "app/condlook.h"

#include "app/condpresets.h"
#include "calc/numfmt.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QPainter>
#include <QPen>

namespace jp {

using namespace jp::office::sheet;

quint32 condLookRgb(const ColorSpec &color, const Styles *styles, quint32 automatic)
{
    if (styles) return styles->resolve(color, automatic) & 0xFFFFFF;
    return color.kind == ColorSpec::Kind::Rgb ? color.rgb : automatic;
}

FormatView viewOfCondLook(const DifferentialFormat &d, const Styles *styles)
{
    FormatView v;
    v.numberFormat = d.numberFormat.value_or(QStringLiteral("General"));
    v.bold = d.bold.value_or(false);
    v.italic = d.italic.value_or(false);
    v.strike = d.strike.value_or(false);
    v.underline = d.underline.value_or(Underline::None);
    if (d.fontColor.isSet() && d.fontColor.kind != ColorSpec::Kind::Auto) v.fontColor = QColor::fromRgb(condLookRgb(d.fontColor, styles, 0x000000));
    if (d.hasFill) {
        const FillSpec &f = d.fill;
        if (f.pattern == PatternKind::None || f.pattern == PatternKind::Solid) {
            const ColorSpec &c = f.bg.isSet() ? f.bg : f.fg;   // (a rule's solid fill keeps its color in the background color)
            if (c.isSet()) {
                v.pattern = PatternKind::Solid;
                v.fillColor = QColor::fromRgb(condLookRgb(c, styles, 0xFFFFFF));
            }
        } else {
            v.pattern = f.pattern;
            if (f.fg.isSet() && f.fg.kind != ColorSpec::Kind::Indexed) v.patternColor = QColor::fromRgb(condLookRgb(f.fg, styles, 0x000000));
            if (f.bg.isSet() && f.bg.kind != ColorSpec::Kind::Indexed) v.fillColor = QColor::fromRgb(condLookRgb(f.bg, styles, 0xFFFFFF));
        }
    }
    if (d.hasBorder) {
        BorderSpec b = d.border;
        for (BorderSide *s : {&b.left, &b.right, &b.top, &b.bottom})
            if (s->style != BorderStyle::None) s->color = ColorSpec::fromRgb(condLookRgb(s->color, styles, 0x000000));
        b.diagonal = BorderSide();
        b.diagonalUp = b.diagonalDown = false;
        v.border = b;
    }
    return v;
}

DifferentialFormat condLookAfter(const DifferentialFormat &look, const FormatCellsDialog::Result &answer)
{
    DifferentialFormat d = look;
    const FormatChange c = answer.wholeChange();
    // (a style put back to what a cell has anyway is taken out of the look, not written as an "off")
    auto flag = [](std::optional<bool> *to, const std::optional<bool> &now) {
        if (!now) return;
        if (*now) *to = true;
        else to->reset();
    };
    flag(&d.bold, c.bold);
    flag(&d.italic, c.italic);
    flag(&d.strike, c.strike);
    if (c.underline) {
        if (*c.underline == Underline::None) d.underline.reset();
        else d.underline = c.underline;
    }
    if (c.fontColor) d.fontColor = *c.fontColor;
    if (c.numberFormat) {
        if (c.numberFormat->compare(QLatin1String("General"), Qt::CaseInsensitive) == 0) d.numberFormat.reset();
        else d.numberFormat = *c.numberFormat;
    }
    if (c.fill) {
        FillSpec f = *c.fill;
        if (f.pattern == PatternKind::None) {
            d.hasFill = false;
            d.fill = FillSpec();
        } else {
            if (f.pattern == PatternKind::Solid) f.bg = f.fg;   // (a rule's solid fill keeps its color in the background color, and the foreground says the same)
            d.hasFill = true;
            d.fill = f;
        }
    }
    if (c.left || c.right || c.top || c.bottom) {
        BorderSpec b = d.hasBorder ? d.border : BorderSpec();
        if (c.left) b.left = *c.left;
        if (c.right) b.right = *c.right;
        if (c.top) b.top = *c.top;
        if (c.bottom) b.bottom = *c.bottom;
        const bool any = b.left.style != BorderStyle::None || b.right.style != BorderStyle::None || b.top.style != BorderStyle::None || b.bottom.style != BorderStyle::None ||
                         b.diagonal.style != BorderStyle::None;
        d.hasBorder = any;
        d.border = any ? b : BorderSpec();
    }
    return d;
}

bool askCondLook(QWidget *parent, const CondLookContext &context, const DifferentialFormat &start, DifferentialFormat *out)
{
    if (context.asker) return context.asker(start, out);
    FormatCellsDialog::Input in = context.base;
    in.view = viewOfCondLook(start, context.styles);
    in.ruleLook = true;
    in.oneCell = true;
    FormatCellsDialog dialog(in, parent);
    if (dialog.exec() != QDialog::Accepted) return false;
    *out = condLookAfter(start, dialog.result());
    return true;
}

bool sameCondLook(const DifferentialFormat &a, const DifferentialFormat &b)
{
    return sameDifferentialLook(a, b);   // (as it is written: a solid fill keeps its color in one of the two places, and a file read back has it in both)
}

CondLookBox::CondLookBox(bool editing, const DifferentialFormat &current, QWidget *parent) : QWidget(parent), m_editing(editing), m_current(current)
{
    auto *row = new QHBoxLayout(this);
    row->setContentsMargins(0, 0, 0, 0);
    m_list = new QComboBox(this);
    m_list->setObjectName(QStringLiteral("condPreset"));
    if (editing) {
        // (a look the file has that none of the ready-made ones is: it is the rule's Custom Format)
        bool ready = current.isEmpty();
        for (int i = 0; i < condpresets::lookCount() && !ready; ++i) ready = sameCondLook(condpresets::lookOf(i), current);
        m_list->addItem(ready ? QCoreApplication::translate("jp::CondLookBox", "Keep the current look") : QCoreApplication::translate("jp::CondLookBox", "Custom Format"), -1);
    }
    for (int i = 0; i < condpresets::lookCount(); ++i) m_list->addItem(condpresets::lookName(i), i);
    m_list->addItem(QCoreApplication::translate("jp::CondLookBox", "Custom Format..."), -2);
    m_sample = new CondLookSample(this);
    row->addWidget(m_list, 1);
    row->addWidget(m_sample);
    connect(m_list, &QComboBox::currentIndexChanged, this, [this] { changed(); });
    connect(m_list, &QComboBox::activated, this, [this](int at) {
        if (!m_building && at == m_before && m_list->itemData(at).toInt() == -2) changed(true);   // (the same item picked again: the person's look is changed)
    });
    m_building = false;
    m_before = 0;
    changed();
}

void CondLookBox::setContext(const CondLookContext &context)
{
    m_context = context;
    m_sample->setLook(shown(), m_context.styles);
}

DifferentialFormat CondLookBox::shown() const
{
    const int data = m_list->currentData().toInt();
    if (data == -1) return m_current;
    if (data == -2) return m_custom;
    return condpresets::lookOf(std::max(0, data));
}

void CondLookBox::setChoice(const Choice &choice)
{
    const QSignalBlocker block(m_list);
    if (choice.custom) {
        m_custom = choice.look;
        m_list->setCurrentIndex(m_list->findData(-2));
    } else if (choice.keep && m_editing) {
        m_list->setCurrentIndex(0);
    } else {
        m_list->setCurrentIndex(std::max(0, m_list->findData(choice.preset)));
    }
    m_before = m_list->currentIndex();
    m_sample->setLook(shown(), m_context.styles);
}

CondLookBox::Choice CondLookBox::choice() const
{
    Choice c;
    const int data = m_list->currentData().toInt();
    c.keep = data == -1;
    c.custom = data == -2;
    c.preset = std::max(0, data);
    if (c.custom) c.look = m_custom;
    return c;
}

void CondLookBox::changed(bool again)
{
    if (m_building) return;
    if (m_list->currentData().toInt() == -2) {
        // (the box starts from the look that was shown, and a cancelled box leaves the list where it was)
        const int was = again ? m_list->currentIndex() : m_before;
        const int wasData = m_list->itemData(was).toInt();
        const DifferentialFormat start = wasData == -2 ? m_custom : wasData == -1 ? m_current : condpresets::lookOf(std::max(0, wasData));
        DifferentialFormat made;
        if (askCondLook(window(), m_context, start, &made)) {
            m_custom = made;
        } else {
            const QSignalBlocker block(m_list);
            m_list->setCurrentIndex(was);
        }
    }
    m_before = m_list->currentIndex();
    m_sample->setLook(shown(), m_context.styles);
}

CondLookSample::CondLookSample(QWidget *parent) : QWidget(parent), m_text(QCoreApplication::translate("jp::CondLookSample", "AaBbCcYyZz"))
{
    setObjectName(QStringLiteral("condLookSample"));
    setMinimumSize(110, 28);
}

void CondLookSample::setLook(const DifferentialFormat &look, const Styles *styles)
{
    m_look = look;
    m_styles = styles;
    m_text = QCoreApplication::translate("jp::CondLookSample", "AaBbCcYyZz");
    if (look.numberFormat) {
        const jp::calc::NumberFormat code(*look.numberFormat);
        if (code.isValid() && !code.isGeneral()) m_text = code.format(1234.5).text;
    }
    update();
}

void CondLookSample::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    const QRect box = rect().adjusted(0, 0, -1, -1);
    p.fillRect(rect(), palette().color(QPalette::Base));
    const DifferentialFormat &d = m_look;
    if (d.hasFill) {
        const FillSpec &f = d.fill;
        if (f.pattern == PatternKind::None || f.pattern == PatternKind::Solid) {
            const ColorSpec &c = f.bg.isSet() ? f.bg : f.fg;
            if (c.isSet()) p.fillRect(rect(), QColor::fromRgb(condLookRgb(c, m_styles, 0xFFFFFF)));
        } else {
            p.fillRect(rect(), QColor::fromRgb(condLookRgb(f.bg, m_styles, 0xFFFFFF)));
            QColor ink = QColor::fromRgb(condLookRgb(f.fg, m_styles, 0x000000));
            ink.setAlpha(110);   // (the pattern is lightened, so that the text stays readable in the sample)
            p.fillRect(rect(), QBrush(ink, Qt::Dense6Pattern));
        }
    }
    QFont font = p.font();
    font.setBold(d.bold.value_or(false));
    font.setItalic(d.italic.value_or(false));
    font.setStrikeOut(d.strike.value_or(false));
    font.setUnderline(d.underline.value_or(Underline::None) != Underline::None);
    p.setFont(font);
    p.setPen(d.fontColor.isSet() && d.fontColor.kind != ColorSpec::Kind::Auto ? QColor::fromRgb(condLookRgb(d.fontColor, m_styles, 0x000000)) : palette().color(QPalette::Text));
    p.drawText(rect().adjusted(6, 0, -6, 0), Qt::AlignVCenter | Qt::AlignLeft, m_text);
    if (d.hasBorder) {
        auto line = [&](const BorderSide &s, QPoint a, QPoint b) {
            if (s.style == BorderStyle::None) return;
            QPen pen(QColor::fromRgb(condLookRgb(s.color, m_styles, 0x000000)));
            pen.setWidth(1);
            if (s.style == BorderStyle::Dotted || s.style == BorderStyle::Hair) pen.setStyle(Qt::DotLine);
            else if (s.style == BorderStyle::Dashed) pen.setStyle(Qt::DashLine);
            else if (s.style == BorderStyle::DashDot) pen.setStyle(Qt::DashDotLine);
            else if (s.style == BorderStyle::DashDotDot) pen.setStyle(Qt::DashDotDotLine);
            p.setPen(pen);
            p.drawLine(a, b);
        };
        line(d.border.left, box.topLeft(), box.bottomLeft());
        line(d.border.right, box.topRight(), box.bottomRight());
        line(d.border.top, box.topLeft(), box.topRight());
        line(d.border.bottom, box.bottomLeft(), box.bottomRight());
    }
}

} // namespace jp
