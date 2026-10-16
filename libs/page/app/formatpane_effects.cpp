// The Effects page of the Format pane: shadow, reflection, glow, and soft edges.
#include "app/formatpane_p.h"
#include "app/formatpresets.h"
#include "app/paneframe.h"

#include <QFormLayout>
#include <QVBoxLayout>

namespace jp::format {

namespace {

using std::nullopt;

NumberField *field(QWidget *parent, const char *name, double lo, double hi, int decimals, const QString &suffix, bool slider, double step = 1)
{
    auto *f = new NumberField(parent);
    f->setObjectName(QLatin1String(name));
    f->setRange(lo, hi);
    f->setDecimals(decimals);
    f->setStep(step);
    f->setSuffix(suffix);
    f->setSlider(slider);
    return f;
}

// A box of presets: choosing one applies it and leaves the box as it was (it shows "Presets" until another is chosen).
QComboBox *presets(FormatPane *pane, QWidget *parent, const char *name, const QStringList &names, const std::function<void(Effects &, int)> &apply)
{
    auto *box = new QComboBox(parent);
    box->setObjectName(QLatin1String(name));
    box->addItems(names);
    box->setPlaceholderText(FormatPane::tr("Presets"));
    box->setCurrentIndex(-1);
    ignoreWheelUnlessFocused(box);
    FormatSession *s = pane->session();
    QObject::connect(box, &QComboBox::currentIndexChanged, pane, [=](int i) {
        if (i < 0) return;
        s->apply(FormatPane::tr("Effects"), [=](Item &it) { apply(it.fx, i); }, Scope::Leaves);
        const QSignalBlocker block(box);
        box->setCurrentIndex(-1);
    });
    return box;
}

} // namespace

void buildEffectsPage(FormatPane *pane, PanePage *page)
{
    FormatSession *s = pane->session();
    using namespace formatpresets;
    const QString P = QStringLiteral(" pt"), Pct = QStringLiteral("%");

    // ---- shadow
    PaneSection *shadow = page->addSection(FormatPane::tr("Shadow"));
    {
        auto *form = new QFormLayout();
        form->setContentsMargins(0, 0, 0, 0);
        QWidget *w = shadow->bodyWidget();
        auto *on = new QCheckBox(FormatPane::tr("Show shadow"), w);
        on->setObjectName(QStringLiteral("shadowOn"));
        shadow->body()->addWidget(on);
        QStringList names;
        for (int i = 0; i < shadowCount(); ++i) names << shadowName(i);
        form->addRow(FormatPane::tr("Presets"), presets(pane, w, "shadowPreset", names, [](Effects &fx, int i) { applyShadow(fx, i); }));
        auto *color = new ColorButton(QString(), FormatPane::tr("Shadow color"), false, QString(), w);
        color->setObjectName(QStringLiteral("shadowColor"));
        color->setPopupMode(QToolButton::InstantPopup);
        color->setWell(true);
        form->addRow(FormatPane::tr("Color"), color);
        auto *transparency = field(w, "shadowTransparency", 0, 100, 0, Pct, true);
        auto *size = field(w, "shadowSize", 1, 200, 0, Pct, true);
        auto *blur = field(w, "shadowBlur", 0, 100, 1, P, true, 0.5);
        auto *angle = field(w, "shadowAngle", 0, 359, 0, QStringLiteral("°"), false);
        auto *distance = field(w, "shadowDistance", 0, 200, 1, P, true, 0.5);
        form->addRow(FormatPane::tr("Transparency"), transparency);
        form->addRow(FormatPane::tr("Size"), size);
        form->addRow(FormatPane::tr("Blur"), blur);
        form->addRow(FormatPane::tr("Angle"), angle);
        form->addRow(FormatPane::tr("Distance"), distance);
        shadow->body()->addLayout(form);
        const QString label = FormatPane::tr("Shadow");
        bindCheck(pane, on, label, [](const Item &it) -> std::optional<bool> { return it.fx.shadow.on; }, [](Item &it, bool v) { it.fx.shadow.on = v; }, Scope::Leaves);
        auto when = [](auto get) { return [get](const Item &it) -> std::optional<double> { return it.fx.shadow.on ? std::optional<double>(get(it.fx.shadow)) : nullopt; }; };
        bindColor(pane, color, label, [](const Item &it) -> std::optional<ColorRef> { return it.fx.shadow.on ? std::optional<ColorRef>(it.fx.shadow.color) : nullopt; },
                  [](Item &it, const ColorRef &c) { it.fx.shadow.color = c; }, Scope::Leaves);
        bindNumber(pane, transparency, label, when([](const ShadowFx &x) { return x.transparency * 100; }), [](Item &it, double v) { it.fx.shadow.transparency = v / 100; }, Scope::Leaves);
        bindNumber(pane, size, label, when([](const ShadowFx &x) { return x.size; }), [](Item &it, double v) { it.fx.shadow.size = v; }, Scope::Leaves);
        bindNumber(pane, blur, label, when([](const ShadowFx &x) { return x.blur; }), [](Item &it, double v) { it.fx.shadow.blur = v; }, Scope::Leaves);
        bindNumber(pane, angle, label, when([](const ShadowFx &x) { return x.angle; }), [](Item &it, double v) { it.fx.shadow.angle = v; }, Scope::Leaves);
        bindNumber(pane, distance, label, when([](const ShadowFx &x) { return x.distance; }), [](Item &it, double v) { it.fx.shadow.distance = v; }, Scope::Leaves);
    }

    // ---- reflection
    PaneSection *reflection = page->addSection(FormatPane::tr("Reflection"), false);
    {
        auto *form = new QFormLayout();
        form->setContentsMargins(0, 0, 0, 0);
        QWidget *w = reflection->bodyWidget();
        auto *on = new QCheckBox(FormatPane::tr("Show reflection"), w);
        on->setObjectName(QStringLiteral("reflectionOn"));
        reflection->body()->addWidget(on);
        QStringList names;
        for (int i = 0; i < reflectionCount(); ++i) names << reflectionName(i);
        form->addRow(FormatPane::tr("Presets"), presets(pane, w, "reflectionPreset", names, [](Effects &fx, int i) { applyReflection(fx, i); }));
        auto *transparency = field(w, "reflectionTransparency", 0, 100, 0, Pct, true);
        auto *size = field(w, "reflectionSize", 0, 100, 0, Pct, true);
        auto *distance = field(w, "reflectionDistance", 0, 200, 1, P, true, 0.5);
        auto *blur = field(w, "reflectionBlur", 0, 100, 1, P, true, 0.5);
        form->addRow(FormatPane::tr("Transparency"), transparency);
        form->addRow(FormatPane::tr("Size"), size);
        form->addRow(FormatPane::tr("Distance"), distance);
        form->addRow(FormatPane::tr("Blur"), blur);
        reflection->body()->addLayout(form);
        const QString label = FormatPane::tr("Reflection");
        bindCheck(pane, on, label, [](const Item &it) -> std::optional<bool> { return it.fx.reflection.on; }, [](Item &it, bool v) { it.fx.reflection.on = v; }, Scope::Leaves);
        auto when = [](auto get) { return [get](const Item &it) -> std::optional<double> { return it.fx.reflection.on ? std::optional<double>(get(it.fx.reflection)) : nullopt; }; };
        bindNumber(pane, transparency, label, when([](const ReflectionFx &x) { return x.transparency * 100; }), [](Item &it, double v) { it.fx.reflection.transparency = v / 100; }, Scope::Leaves);
        bindNumber(pane, size, label, when([](const ReflectionFx &x) { return x.size * 100; }), [](Item &it, double v) { it.fx.reflection.size = v / 100; }, Scope::Leaves);
        bindNumber(pane, distance, label, when([](const ReflectionFx &x) { return x.distance; }), [](Item &it, double v) { it.fx.reflection.distance = v; }, Scope::Leaves);
        bindNumber(pane, blur, label, when([](const ReflectionFx &x) { return x.blur; }), [](Item &it, double v) { it.fx.reflection.blur = v; }, Scope::Leaves);
    }

    // ---- glow
    PaneSection *glow = page->addSection(FormatPane::tr("Glow"), false);
    {
        auto *form = new QFormLayout();
        form->setContentsMargins(0, 0, 0, 0);
        QWidget *w = glow->bodyWidget();
        auto *on = new QCheckBox(FormatPane::tr("Show glow"), w);
        on->setObjectName(QStringLiteral("glowOn"));
        glow->body()->addWidget(on);
        QStringList names;
        for (int i = 0; i < glowCount(); ++i) names << glowName(i);
        form->addRow(FormatPane::tr("Presets"), presets(pane, w, "glowPreset", names, [](Effects &fx, int i) { applyGlow(fx, i, ColorRef::scheme(Accent1)); }));
        auto *color = new ColorButton(QString(), FormatPane::tr("Glow color"), false, QString(), w);
        color->setObjectName(QStringLiteral("glowColor"));
        color->setPopupMode(QToolButton::InstantPopup);
        color->setWell(true);
        form->addRow(FormatPane::tr("Color"), color);
        auto *size = field(w, "glowSize", 0, 150, 1, P, true, 0.5);
        auto *transparency = field(w, "glowTransparency", 0, 100, 0, Pct, true);
        form->addRow(FormatPane::tr("Size"), size);
        form->addRow(FormatPane::tr("Transparency"), transparency);
        glow->body()->addLayout(form);
        const QString label = FormatPane::tr("Glow");
        bindCheck(pane, on, label, [](const Item &it) -> std::optional<bool> { return it.fx.glow.on; }, [](Item &it, bool v) { it.fx.glow.on = v; }, Scope::Leaves);
        auto when = [](auto get) { return [get](const Item &it) -> std::optional<double> { return it.fx.glow.on ? std::optional<double>(get(it.fx.glow)) : nullopt; }; };
        bindColor(pane, color, label, [](const Item &it) -> std::optional<ColorRef> { return it.fx.glow.on ? std::optional<ColorRef>(it.fx.glow.color) : nullopt; },
                  [](Item &it, const ColorRef &c) { it.fx.glow.color = c; }, Scope::Leaves);
        bindNumber(pane, size, label, when([](const GlowFx &x) { return x.size; }), [](Item &it, double v) { it.fx.glow.size = v; }, Scope::Leaves);
        bindNumber(pane, transparency, label, when([](const GlowFx &x) { return x.transparency * 100; }), [](Item &it, double v) { it.fx.glow.transparency = v / 100; }, Scope::Leaves);
    }

    // ---- soft edges
    PaneSection *soft = page->addSection(FormatPane::tr("Soft Edges"), false);
    {
        auto *form = new QFormLayout();
        form->setContentsMargins(0, 0, 0, 0);
        QWidget *w = soft->bodyWidget();
        QStringList names;
        for (int i = 0; i < softEdgeCount(); ++i) names << softEdgeName(i);
        form->addRow(FormatPane::tr("Presets"), presets(pane, w, "softEdgePreset", names, [](Effects &fx, int i) { applySoftEdge(fx, i); }));
        auto *size = field(w, "softEdgeSize", 0, 100, 1, P, true, 0.5);
        form->addRow(FormatPane::tr("Size"), size);
        soft->body()->addLayout(form);
        bindNumber(pane, size, FormatPane::tr("Soft Edges"), [](const Item &it) -> std::optional<double> { return it.fx.softEdge; }, [](Item &it, double v) { it.fx.softEdge = v; }, Scope::Leaves);
    }

    // A line is drawn with its shadow and no other effect.
    s->enroll([=] {
        bool others = false;
        for (const Item *it : s->leaves()) others = others || it->type() != ItemType::Line;
        reflection->setVisible(others);
        glow->setVisible(others);
        soft->setVisible(others);
    });
}

} // namespace jp::format
