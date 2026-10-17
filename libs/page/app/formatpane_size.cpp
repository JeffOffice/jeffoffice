// The Size & Properties page of the Format pane: size, place, turn, and what a text box or shape does with its text; and the alt text of
// every object.
#include "app/formatpane_p.h"
#include "app/paneframe.h"

#include <QFormLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <cmath>

namespace jp::format {

namespace {

using std::nullopt;

// The size a picture has before it is changed: its pixels at 96 to the inch.
QSizeF nativeSize(FormatPane *pane, const PictureItem &p)
{
    const QSize px = pane->editor()->doc()->imageSize(p.imageId);
    return QSizeF(px.width() * 0.75, px.height() * 0.75);
}

bool takesRotation(const Item &it)
{
    return it.type() == ItemType::Text || it.type() == ItemType::Picture || it.type() == ItemType::Shape || it.type() == ItemType::TextArt;
}

// Objects with text of their own that the text options reach: text boxes and shapes.
bool takesTextOptions(const Item &it) { return it.type() == ItemType::Text || it.type() == ItemType::Shape; }

NumberField *measure(QWidget *parent, const char *name, double lo, double hi)
{
    auto *f = new NumberField(parent);
    f->setObjectName(QLatin1String(name));
    f->setMeasure(true);
    f->setRange(lo, hi);
    return f;
}

NumberField *plain(QWidget *parent, const char *name, double lo, double hi, int decimals, const QString &suffix, double step = 1)
{
    auto *f = new NumberField(parent);
    f->setObjectName(QLatin1String(name));
    f->setRange(lo, hi);
    f->setDecimals(decimals);
    f->setStep(step);
    f->setSuffix(suffix);
    return f;
}

QComboBox *choices(QWidget *parent, const char *name, const QStringList &items)
{
    auto *c = new QComboBox(parent);
    c->setObjectName(QLatin1String(name));
    c->addItems(items);
    ignoreWheelUnlessFocused(c);
    return c;
}

} // namespace

void buildSizePage(FormatPane *pane, PanePage *page)
{
    FormatSession *s = pane->session();
    Editor *ed = pane->editor();

    // ---- size
    PaneSection *size = page->addSection(FormatPane::tr("Size"));
    {
        QWidget *w = size->bodyWidget();
        auto *form = new QFormLayout();
        form->setContentsMargins(0, 0, 0, 0);
        auto *height = measure(w, "sizeHeight", 0, 4000);
        auto *width = measure(w, "sizeWidth", 0, 4000);
        auto *rotation = plain(w, "sizeRotation", 0, 359.9, 1, QStringLiteral("°"));
        auto *scaleHeight = plain(w, "scaleHeight", 1, 10000, 0, QStringLiteral("%"));
        auto *scaleWidth = plain(w, "scaleWidth", 1, 10000, 0, QStringLiteral("%"));
        auto *lock = new QCheckBox(FormatPane::tr("Lock aspect ratio"), w);
        lock->setObjectName(QStringLiteral("lockAspect"));
        form->addRow(FormatPane::tr("Height"), height);
        form->addRow(FormatPane::tr("Width"), width);
        form->addRow(FormatPane::tr("Rotation"), rotation);
        form->addRow(FormatPane::tr("Scale height"), scaleHeight);
        form->addRow(FormatPane::tr("Scale width"), scaleWidth);
        size->body()->addLayout(form);
        size->body()->addWidget(lock);
        const QString label = FormatPane::tr("Size");
        bindNumber(pane, height, label, [](const Item &it) -> std::optional<double> { return it.rect.height(); },
                   [pane](Item &it, double v) {
                       QSizeF sz = it.rect.size();
                       if (formatops::keepsProportions(it) && sz.height() > 0) sz.setWidth(sz.width() * v / sz.height());
                       sz.setHeight(v);
                       formatops::resizeFrame(pane->editor(), it, sz);
                   }, Scope::Selected, false);
        bindNumber(pane, width, label, [](const Item &it) -> std::optional<double> { return it.rect.width(); },
                   [pane](Item &it, double v) {
                       QSizeF sz = it.rect.size();
                       if (formatops::keepsProportions(it) && sz.width() > 0) sz.setHeight(sz.height() * v / sz.width());
                       sz.setWidth(v);
                       formatops::resizeFrame(pane->editor(), it, sz);
                   }, Scope::Selected, false);
        bindNumber(pane, rotation, FormatPane::tr("Rotation"), [](const Item &it) -> std::optional<double> {
            return takesRotation(it) ? std::optional<double>(std::fmod(std::fmod(it.rotation, 360.0) + 360.0, 360.0)) : nullopt;
        }, [](Item &it, double v) { it.rotation = std::fmod(std::fmod(v, 360.0) + 360.0, 360.0); }, Scope::Selected, false);
        // A picture is scaled against the size it has in its file; anything else, against the size it has now (so the boxes say 100).
        auto scaleOf = [pane](const Item &it, bool vertical) -> std::optional<double> {
            if (it.type() != ItemType::Picture) return 100.0;
            const auto &p = static_cast<const PictureItem &>(it);
            const QSizeF native = nativeSize(pane, p);
            if (native.isEmpty() || p.imgRect.isEmpty()) return 100.0;
            return vertical ? p.imgRect.height() / native.height() * 100 : p.imgRect.width() / native.width() * 100;
        };
        auto scaleTo = [pane, scaleOf](Item &it, double v, bool vertical) {
            const double now = scaleOf(it, vertical).value_or(100.0);
            if (now <= 0) return;
            const double k = v / now;
            QSizeF sz = it.rect.size();
            if (vertical) sz.setHeight(sz.height() * k);
            else sz.setWidth(sz.width() * k);
            if (formatops::keepsProportions(it)) sz = vertical ? QSizeF(it.rect.width() * k, sz.height()) : QSizeF(sz.width(), it.rect.height() * k);
            formatops::resizeFrame(pane->editor(), it, sz);
        };
        bindNumber(pane, scaleHeight, label, [scaleOf](const Item &it) { return scaleOf(it, true); }, [scaleTo](Item &it, double v) { scaleTo(it, v, true); }, Scope::Selected, false);
        bindNumber(pane, scaleWidth, label, [scaleOf](const Item &it) { return scaleOf(it, false); }, [scaleTo](Item &it, double v) { scaleTo(it, v, false); }, Scope::Selected, false);
        bindCheck(pane, lock, FormatPane::tr("Lock Aspect Ratio"), [](const Item &it) -> std::optional<bool> { return formatops::keepsProportions(it); },
                  [](Item &it, bool v) {
                      if (it.type() == ItemType::Table) static_cast<TableItem &>(it).lockSize = v;
                      else it.lockAspect = v;
                  });
    }

    // ---- position
    PaneSection *position = page->addSection(FormatPane::tr("Position"));
    {
        QWidget *w = position->bodyWidget();
        auto *form = new QFormLayout();
        form->setContentsMargins(0, 0, 0, 0);
        auto *x = measure(w, "positionX", -4000, 8000);
        auto *y = measure(w, "positionY", -4000, 8000);
        form->addRow(FormatPane::tr("Horizontal position"), x);
        form->addRow(FormatPane::tr("Vertical position"), y);
        auto *from = new QLabel(FormatPane::tr("From the top left corner"), w);
        form->addRow(from);
        position->body()->addLayout(form);
        const QString label = FormatPane::tr("Position");
        bindNumber(pane, x, label, [](const Item &it) -> std::optional<double> { return it.rect.x(); },
                   [pane](Item &it, double v) {
                       formatops::moveFrame(pane->editor(), it, v - it.rect.x(), 0);
                   }, Scope::Selected, false);
        bindNumber(pane, y, label, [](const Item &it) -> std::optional<double> { return it.rect.y(); },
                   [pane](Item &it, double v) {
                       formatops::moveFrame(pane->editor(), it, 0, v - it.rect.y());
                   }, Scope::Selected, false);
    }

    // ---- text box
    PaneSection *text = page->addSection(FormatPane::tr("Text Box"));
    {
        QWidget *w = text->bodyWidget();
        auto *form = new QFormLayout();
        form->setContentsMargins(0, 0, 0, 0);
        auto *align = choices(w, "textAlign", {FormatPane::tr("Top"), FormatPane::tr("Top Centered"), FormatPane::tr("Middle"), FormatPane::tr("Middle Centered"),
                                               FormatPane::tr("Bottom"), FormatPane::tr("Bottom Centered")});
        auto *direction = choices(w, "textDirection", {FormatPane::tr("Horizontal"), FormatPane::tr("Rotate all text 90°")});
        auto *autofit = choices(w, "textAutofit", {FormatPane::tr("Do not Autofit"), FormatPane::tr("Shrink text on overflow"), FormatPane::tr("Resize shape to fit text")});
        form->addRow(FormatPane::tr("Vertical alignment"), align);
        form->addRow(FormatPane::tr("Text direction"), direction);
        form->addRow(FormatPane::tr("Autofit"), autofit);
        auto *left = measure(w, "marginLeft", 0, 500), *right = measure(w, "marginRight", 0, 500);
        auto *top = measure(w, "marginTop", 0, 500), *bottom = measure(w, "marginBottom", 0, 500);
        form->addRow(FormatPane::tr("Left margin"), left);
        form->addRow(FormatPane::tr("Right margin"), right);
        form->addRow(FormatPane::tr("Top margin"), top);
        form->addRow(FormatPane::tr("Bottom margin"), bottom);
        auto *wrap = new QCheckBox(FormatPane::tr("Wrap text in shape"), w);
        wrap->setObjectName(QStringLiteral("textWrap"));
        auto *columns = plain(w, "textColumns", 1, 16, 0, QString());
        auto *spacing = measure(w, "columnSpacing", 0, 500);
        text->body()->addLayout(form);
        text->body()->addWidget(wrap);
        auto *cform = new QFormLayout();
        cform->setContentsMargins(0, 0, 0, 0);
        cform->addRow(FormatPane::tr("Columns"), columns);
        cform->addRow(FormatPane::tr("Spacing"), spacing);
        text->body()->addLayout(cform);

        const QString label = FormatPane::tr("Text Box");
        bindChoice(pane, align, label, [](const Item &it) -> std::optional<int> {
            if (it.type() == ItemType::Text) {
                const auto &t = static_cast<const TextItem &>(it);
                return int(t.valign) * 2 + (t.centerBlock ? 1 : 0);
            }
            if (it.type() == ItemType::Shape) {
                const auto &t = static_cast<const ShapeItem &>(it);
                return int(t.valign) * 2 + (t.centerBlock ? 1 : 0);
            }
            return nullopt;
        }, [](Item &it, int i) {
            const VAlign v = VAlign(i / 2);
            const bool centered = i % 2;
            if (it.type() == ItemType::Text) {
                auto &t = static_cast<TextItem &>(it);
                t.valign = v;
                t.centerBlock = centered;
            } else if (it.type() == ItemType::Shape) {
                auto &t = static_cast<ShapeItem &>(it);
                t.valign = v;
                t.centerBlock = centered;
            }
        });
        // Text turned a quarter is a text box's; a shape's text is laid out across it.
        bindChoice(pane, direction, label, [](const Item &it) -> std::optional<int> {
            return it.type() == ItemType::Text ? std::optional<int>(static_cast<const TextItem &>(it).vertical ? 1 : 0) : nullopt;
        }, [](Item &it, int i) { static_cast<TextItem &>(it).vertical = i == 1; });
        // Shrinking text to fit and growing the box to fit are a text box's; a shape's text keeps its size.
        bindChoice(pane, autofit, label, [](const Item &it) -> std::optional<int> {
            if (it.type() == ItemType::Text) {
                const auto a = static_cast<const TextItem &>(it).autofit;
                return a == TextItem::GrowBox ? 2 : a == TextItem::NoAutofit ? 0 : 1;
            }
            return it.type() == ItemType::Shape ? std::optional<int>(static_cast<const ShapeItem &>(it).shrinks ? 1 : 0) : nullopt;
        }, [ed](Item &it, int i) {
            if (it.type() == ItemType::Text) {
                auto &t = static_cast<TextItem &>(it);
                t.autofit = i == 2 ? TextItem::GrowBox : i == 1 ? TextItem::ShrinkOnOverflow : TextItem::NoAutofit;
                t.fitAsStored = false;
                t.fitScale = 1.0;
                t.fitLineCut = 0;
                if (i == 2) ed->autoGrowText(&t);
            } else if (it.type() == ItemType::Shape && i == 0) {
                static_cast<ShapeItem &>(it).shrinks = false;
            }
        });
        // (What a shape can't do is turned off in the list.)
        s->enroll([=] {
            bool shapes = false;
            for (const Item *it : s->items()) shapes = shapes || it->type() == ItemType::Shape;
            if (auto *model = qobject_cast<QStandardItemModel *>(autofit->model()))
                for (int i = 1; i < 3; ++i)
                    if (QStandardItem *item = model->item(i)) item->setEnabled(!shapes);
        });
        auto insetsOf = [](const Item &it) -> std::optional<QMarginsF> {
            if (it.type() == ItemType::Text) return static_cast<const TextItem &>(it).insets;
            if (it.type() == ItemType::Shape) return static_cast<const ShapeItem &>(it).insets;
            return nullopt;
        };
        auto setInsets = [](Item &it, const std::function<void(QMarginsF &)> &change) {
            if (it.type() == ItemType::Text) change(static_cast<TextItem &>(it).insets);
            else if (it.type() == ItemType::Shape) change(static_cast<ShapeItem &>(it).insets);
        };
        bindNumber(pane, left, label, [=](const Item &it) -> std::optional<double> { const auto m = insetsOf(it); return m ? std::optional<double>(m->left()) : nullopt; },
                   [=](Item &it, double v) { setInsets(it, [v](QMarginsF &m) { m.setLeft(v); }); });
        bindNumber(pane, right, label, [=](const Item &it) -> std::optional<double> { const auto m = insetsOf(it); return m ? std::optional<double>(m->right()) : nullopt; },
                   [=](Item &it, double v) { setInsets(it, [v](QMarginsF &m) { m.setRight(v); }); });
        bindNumber(pane, top, label, [=](const Item &it) -> std::optional<double> { const auto m = insetsOf(it); return m ? std::optional<double>(m->top()) : nullopt; },
                   [=](Item &it, double v) { setInsets(it, [v](QMarginsF &m) { m.setTop(v); }); });
        bindNumber(pane, bottom, label, [=](const Item &it) -> std::optional<double> { const auto m = insetsOf(it); return m ? std::optional<double>(m->bottom()) : nullopt; },
                   [=](Item &it, double v) { setInsets(it, [v](QMarginsF &m) { m.setBottom(v); }); });
        bindCheck(pane, wrap, label, [](const Item &it) -> std::optional<bool> {
            if (it.type() == ItemType::Text) return !static_cast<const TextItem &>(it).noWrap;
            if (it.type() == ItemType::Shape) return !static_cast<const ShapeItem &>(it).noWrap;
            return nullopt;
        }, [](Item &it, bool on) {
            if (it.type() == ItemType::Text) static_cast<TextItem &>(it).noWrap = !on;
            else if (it.type() == ItemType::Shape) static_cast<ShapeItem &>(it).noWrap = !on;
        });
        bindNumber(pane, columns, label, [](const Item &it) -> std::optional<double> {
            if (it.type() == ItemType::Text) return static_cast<const TextItem &>(it).columns;
            if (it.type() == ItemType::Shape) return static_cast<const ShapeItem &>(it).columns;
            return nullopt;
        }, [](Item &it, double v) {
            const int n = std::clamp(int(std::lround(v)), 1, 16);
            if (it.type() == ItemType::Text) {
                auto &t = static_cast<TextItem &>(it);
                if (t.columns <= 1 && n > 1 && t.columnGap <= 0) t.columnGap = 18;
                t.columns = n;
            } else if (it.type() == ItemType::Shape) {
                auto &t = static_cast<ShapeItem &>(it);
                if (t.columns <= 1 && n > 1 && t.columnGap <= 0) t.columnGap = 18;
                t.columns = n;
            }
        });
        bindNumber(pane, spacing, label, [](const Item &it) -> std::optional<double> {
            if (it.type() == ItemType::Text && static_cast<const TextItem &>(it).columns > 1) return static_cast<const TextItem &>(it).columnGap;
            if (it.type() == ItemType::Shape && static_cast<const ShapeItem &>(it).columns > 1) return static_cast<const ShapeItem &>(it).columnGap;
            return nullopt;
        }, [](Item &it, double v) {
            if (it.type() == ItemType::Text) static_cast<TextItem &>(it).columnGap = v;
            else if (it.type() == ItemType::Shape) static_cast<ShapeItem &>(it).columnGap = v;
        });
        s->enroll([=] {
            bool any = false;
            for (const Item *it : s->items()) any = any || takesTextOptions(*it);
            text->setVisible(any);
            bool texts = false;
            for (const Item *it : s->items()) texts = texts || it->type() == ItemType::Text;
            direction->setEnabled(texts);
            if (!texts) {   // (a shape's text runs across it)
                direction->setProperty("loading", true);
                direction->setCurrentIndex(0);
                direction->setProperty("loading", false);
            }
        });
    }

    // ---- alt text
    PaneSection *alt = page->addSection(FormatPane::tr("Alt Text"), false);
    {
        QWidget *w = alt->bodyWidget();
        auto *title = new TextField(w);
        title->setObjectName(QStringLiteral("altTitle"));
        title->setSingleLine(true);
        title->setAccessibleName(FormatPane::tr("Title"));
        auto *description = new TextField(w);
        description->setObjectName(QStringLiteral("altDescription"));
        description->setAccessibleName(FormatPane::tr("Description"));
        auto *decorative = new QCheckBox(FormatPane::tr("Mark as decorative"), w);
        decorative->setObjectName(QStringLiteral("altDecorative"));
        auto *titleLabel = new QLabel(FormatPane::tr("Title"), w);
        titleLabel->setBuddy(title);
        auto *descriptionLabel = new QLabel(FormatPane::tr("Description"), w);
        descriptionLabel->setBuddy(description);
        alt->body()->addWidget(titleLabel);
        alt->body()->addWidget(title);
        alt->body()->addWidget(descriptionLabel);
        alt->body()->addWidget(description);
        alt->body()->addWidget(decorative);
        const QString label = FormatPane::tr("Alt Text");
        auto commit = [=](TextField *field, std::function<void(Item &, const QString &)> set) {
            QObject::connect(field, &TextField::edited, pane, [=](const QString &t) {
                s->apply(label, [=](Item &it) { set(it, t); }, Scope::Selected);
            });
        };
        commit(title, [](Item &it, const QString &t) { it.altTitle = t; });
        commit(description, [](Item &it, const QString &t) { it.altText = t; });
        bindCheck(pane, decorative, label, [](const Item &it) -> std::optional<bool> { return it.decorative; }, [](Item &it, bool v) { it.decorative = v; });
        s->enroll([=] {
            const QVector<Item *> items = s->items();
            bool allDecorative = !items.isEmpty();
            auto show = [&](TextField *field, auto get) {
                bool same = true;
                for (const Item *it : items) same = same && get(*it) == get(*items.first());
                field->setPlaceholderText(same ? QString() : FormatPane::tr("Different on each object"));
                if (!items.isEmpty() && same) field->setText(get(*items.first()));
                else field->setMixed(!items.isEmpty());
            };
            for (const Item *it : items) allDecorative = allDecorative && it->decorative;
            show(title, [](const Item &it) { return it.altTitle; });
            show(description, [](const Item &it) { return it.altText; });
            title->setEnabled(!items.isEmpty() && !allDecorative);
            description->setEnabled(!items.isEmpty() && !allDecorative);
        });
    }
}

} // namespace jp::format
