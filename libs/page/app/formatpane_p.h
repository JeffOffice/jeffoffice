#pragma once
// What the pages of the Format pane share (formatpane_*.cpp): which objects take what, and the pages themselves.

#include "app/editor.h"
#include "app/formatops.h"
#include "app/formatpane.h"
#include "app/propertyeditors.h"
#include "app/widgets.h"
#include "core/items.h"

#include <QCheckBox>
#include <QComboBox>
#include <functional>
#include <optional>

namespace jp {

class PanePage;

namespace format {

using formatops::takesFill;
using formatops::takesLine;

using Scope = FormatSession::Scope;

// What a control shows of a property that the objects it applies to may or may not have: whether any has it, and whether they agree.
template <class T> struct Shown {
    bool any = false, mixed = false;
    T value{};
};

template <class T, class Get> Shown<T> shownOf(const QVector<Item *> &items, Get &&get)
{
    Shown<T> out;
    for (const Item *it : items) {
        const std::optional<T> v = get(*it);
        if (!v) continue;
        if (!out.any) {
            out.any = true;
            out.value = *v;
        } else if (!(std::abs(double(*v) - double(out.value)) < 1e-6)) {
            out.mixed = true;
        }
    }
    return out;
}

// A number the objects share: the field shows it (blank when they differ, off when none has it), and what is entered is set on each that has it.
// `get` gives an object's value (nothing for an object without the property), `set` puts one there.
inline void bindNumber(FormatPane *pane, NumberField *field, const QString &label, const std::function<std::optional<double>(const Item &)> &get,
                       const std::function<void(Item &, double)> &set, Scope scope = Scope::Selected, bool includeLocked = true)
{
    FormatSession *s = pane->session();
    QObject::connect(field, &NumberField::edited, pane, [=](double v) {
        s->apply(label, [=](Item &it) {
            if (get(it)) set(it, v);
        }, scope, false, includeLocked);
    });
    QObject::connect(field, &NumberField::finished, pane, [s] { s->finish(); });
    s->enroll([=] {
        const Shown<double> shown = shownOf<double>(s->targets(scope), get);
        field->setEnabled(shown.any);
        if (!shown.any) return;
        if (shown.mixed) field->setMixed(true);
        else field->setValue(shown.value);
    });
}

// A choice among a few (a combo box whose items are 0, 1, 2...): the box shows it (blank when they differ), and a choice is set on each.
inline void bindChoice(FormatPane *pane, QComboBox *box, const QString &label, const std::function<std::optional<int>(const Item &)> &get,
                       const std::function<void(Item &, int)> &set, Scope scope = Scope::Selected)
{
    FormatSession *s = pane->session();
    QObject::connect(box, &QComboBox::currentIndexChanged, pane, [=](int i) {
        if (i < 0 || box->property("loading").toBool()) return;
        s->apply(label, [=](Item &it) {
            if (get(it)) set(it, i);
        }, scope);
    });
    s->enroll([=] {
        const Shown<int> shown = shownOf<int>(s->targets(scope), get);
        box->setEnabled(shown.any);
        box->setProperty("loading", true);
        box->setCurrentIndex(shown.any && !shown.mixed ? shown.value : -1);
        box->setProperty("loading", false);
    });
}

// A check box: checked when all the objects have it on, empty when none, and partly checked when they differ; a click makes it all on (or all
// off, when it was).
inline void bindCheck(FormatPane *pane, QCheckBox *box, const QString &label, const std::function<std::optional<bool>(const Item &)> &get,
                      const std::function<void(Item &, bool)> &set, Scope scope = Scope::Selected, bool includeLocked = true)
{
    FormatSession *s = pane->session();
    box->setTristate(true);
    QObject::connect(box, &QCheckBox::clicked, pane, [=] {
        if (box->property("loading").toBool()) return;
        const bool on = box->property("shown").toInt() != int(Qt::Checked);
        box->setProperty("loading", true);
        box->setCheckState(on ? Qt::Checked : Qt::Unchecked);
        box->setProperty("loading", false);
        s->apply(label, [=](Item &it) {
            if (get(it)) set(it, on);
        }, scope, true, includeLocked);
    });
    s->enroll([=] {
        const Shown<bool> shown = shownOf<bool>(s->targets(scope), [&](const Item &it) { return get(it); });
        box->setEnabled(shown.any);
        const Qt::CheckState state = !shown.any ? Qt::Unchecked : shown.mixed ? Qt::PartiallyChecked : shown.value ? Qt::Checked : Qt::Unchecked;
        box->setProperty("loading", true);
        box->setCheckState(state);
        box->setProperty("shown", int(state));
        box->setProperty("loading", false);
    });
}

// A color: the well shows it (blank when they differ), and a pick is set on each.
inline void bindColor(FormatPane *pane, ColorButton *well, const QString &label, const std::function<std::optional<ColorRef>(const Item &)> &get,
                      const std::function<void(Item &, const ColorRef &)> &set, Scope scope = Scope::Selected)
{
    FormatSession *s = pane->session();
    QObject::connect(well, &ColorButton::colorPicked, pane, [=](const ColorRef &c) {
        s->apply(label, [=](Item &it) {
            if (get(it)) set(it, c);
        }, scope);
    });
    s->enroll([=] {
        bool any = false, mixed = false;
        ColorRef first;
        for (const Item *it : s->targets(scope)) {
            const std::optional<ColorRef> c = get(*it);
            if (!c) continue;
            if (!any) {
                any = true;
                first = *c;
            } else if (*c != first) {
                mixed = true;
            }
        }
        well->setEnabled(any);
        well->setScheme(pane->editor()->doc()->colors);
        if (!any) return;
        if (mixed) well->setMixed(true);
        else well->setCurrent(first);
    });
}

template <class F> bool anyTakes(const QVector<Item *> &items, F &&takes)
{
    for (const Item *it : items)
        if (takes(*it)) return true;
    return false;
}

// Builds the controls of each page (each in the formatpane_*.cpp file named for it).
void buildPages(FormatPane *pane, PanePage *fill, PanePage *effects, PanePage *size, PanePage *picture);
void buildFillLinePage(FormatPane *pane, PanePage *page);
void buildEffectsPage(FormatPane *pane, PanePage *page);
void buildSizePage(FormatPane *pane, PanePage *page);
void buildPicturePage(FormatPane *pane, PanePage *page);

} // namespace format
} // namespace jp
