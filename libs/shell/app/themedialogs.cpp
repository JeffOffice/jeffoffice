#include "app/themedialogs.h"

#include "app/plaintext.h"
#include "app/widgets.h"
#include "core/fontfiles.h"

#include <QActionGroup>
#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

namespace jp {

using office::Theme;
using office::ThemeColors;
using office::ThemeFonts;

namespace {

// Whether two sets are the same scheme: the ten colors of the text, backgrounds, and accents (the two hyperlink colors are the document's to keep readable, so they may differ).
bool sameScheme(const ThemeColors &a, const ThemeColors &b)
{
    for (int i = 0; i < Theme::Hyperlink; ++i)
        if (a.colors[i] != b.colors[i]) return false;
    return true;
}

bool hasName(const QStringList &names, const QString &name)
{
    for (const QString &n : names)
        if (n.compare(name, Qt::CaseInsensitive) == 0) return true;
    return false;
}

QStringList namesOf(const QVector<ThemeColors> &sets)
{
    QStringList out;
    for (const ThemeColors &c : sets) out << c.name;
    return out;
}

QStringList namesOfFonts(const QVector<ThemeFonts> &sets)
{
    QStringList out;
    for (const ThemeFonts &c : sets) out << c.name;
    return out;
}

} // namespace

ThemePreview::ThemePreview(QWidget *parent) : QWidget(parent)
{
    m_colors = office::builtinThemeColors().first();
    setObjectName(QStringLiteral("themePreview"));
}

void ThemePreview::setColors(const ThemeColors &colors)
{
    m_colors = colors;
    update();
}

void ThemePreview::setFonts(const QString &heading, const QString &body)
{
    m_heading = heading;
    m_body = body;
    update();
}

void ThemePreview::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRect r = rect().adjusted(1, 1, -1, -1);
    p.fillRect(r, m_colors.colors[Theme::Light1]);
    p.setPen(QColor(0, 0, 0, 60));
    p.drawRect(r);
    // The sample text: a heading in the first dark color, the body in the second, and a link.
    QFont h = m_heading.isEmpty() ? font() : QFont(m_heading), b = m_body.isEmpty() ? font() : QFont(m_body);
    h.setPointSizeF(15);
    h.setBold(true);
    b.setPointSizeF(10);
    const int swatch = std::max(8, (r.width() - 16 - 11 * 4) / 12);
    const int top = r.bottom() - swatch - 8;
    p.setFont(h);
    p.setPen(m_colors.colors[Theme::Dark1]);
    p.drawText(QRect(r.left() + 8, r.top() + 4, r.width() - 16, 24), Qt::AlignLeft | Qt::AlignVCenter, QCoreApplication::translate("jp::ThemeDialogs", "Heading"));
    p.setFont(b);
    p.setPen(m_colors.colors[Theme::Dark2]);
    p.drawText(QRect(r.left() + 8, r.top() + 28, r.width() - 16, 18), Qt::AlignLeft | Qt::AlignVCenter, QCoreApplication::translate("jp::ThemeDialogs", "Body text"));
    p.setPen(m_colors.colors[Theme::Hyperlink]);
    QFont link = b;
    link.setUnderline(true);
    p.setFont(link);
    p.drawText(QRect(r.left() + 80, r.top() + 28, r.width() - 88, 18), Qt::AlignLeft | Qt::AlignVCenter, QCoreApplication::translate("jp::ThemeDialogs", "A link"));
    for (int i = 0; i < Theme::ColorCount; ++i) {
        const QRect s(r.left() + 8 + i * (swatch + 4), top, swatch, swatch);
        p.setPen(QColor(0, 0, 0, 80));
        p.setBrush(m_colors.colors[i]);
        p.drawRect(s);
    }
}

QString newThemeSetName(const QStringList &taken)
{
    for (int i = 1; i < 10000; ++i) {
        const QString name = QCoreApplication::translate("jp::ThemeDialogs", "Custom %1").arg(i);
        if (!hasName(taken, name)) return name;
    }
    return QCoreApplication::translate("jp::ThemeDialogs", "Custom");
}

QIcon themeColorsIcon(const ThemeColors &set)
{
    const int size = 16;
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    static const int slots[6] = {Theme::Accent1, Theme::Accent2, Theme::Accent3, Theme::Dark2, Theme::Light2, Theme::Accent4};
    for (int i = 0; i < 6; ++i) {
        p.setPen(QPen(QColor(0, 0, 0, 70), 1));
        p.setBrush(set.colors[slots[i]]);
        p.drawRect(QRect((i % 3) * 5, 1 + (i / 3) * 7, 5, 6));
    }
    return QIcon(pix);
}

// ---------------- Create New Theme Colors ----------------
ThemeColorsDialog::ThemeColorsDialog(const ThemeColors &start, QWidget *parent) : QDialog(parent), m_colors(start)
{
    setObjectName(QStringLiteral("themeColorsDialog"));
    setWindowTitle(QCoreApplication::translate("jp::ThemeDialogs", "Create New Theme Colors"));
    auto *layout = new QVBoxLayout(this);
    auto *grid = new QGridLayout();
    for (int i = 0; i < Theme::ColorCount; ++i) {
        auto *label = plainLabel(new QLabel(QCoreApplication::translate("jp::ThemeDialogs", "%1:").arg(office::themeSlotLabel(i)), this));
        auto *b = new QPushButton(this);
        b->setObjectName(QStringLiteral("themeSlot%1").arg(i));
        b->setFixedSize(90, 26);
        b->setAccessibleName(office::themeSlotLabel(i));
        m_buttons[i] = b;
        paintButton(i);
        connect(b, &QPushButton::clicked, this, [this, i] {
            const QColor c = colorsDialog(this, m_colors.colors[i], office::themeSlotLabel(i));
            if (c.isValid()) setSlot(i, c);
        });
        grid->addWidget(label, i % 6, (i / 6) * 2);
        grid->addWidget(b, i % 6, (i / 6) * 2 + 1);
    }
    layout->addLayout(grid);
    m_preview = new ThemePreview(this);
    m_preview->setColors(m_colors);
    layout->addWidget(m_preview);
    auto *form = new QFormLayout();
    m_name = new QLineEdit(this);
    m_name->setMaxLength(office::kMaxThemeNameLength);
    QStringList taken = namesOf(office::customThemeColors());
    for (const ThemeColors &c : office::builtinThemeColors()) taken << c.name;
    m_name->setText(hasName(taken, start.name) || start.name.isEmpty() ? newThemeSetName(taken) : start.name);
    form->addRow(QCoreApplication::translate("jp::ThemeDialogs", "Name:"), m_name);
    layout->addLayout(form);
    auto *box = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(box);
}

void ThemeColorsDialog::paintButton(int slot)
{
    m_buttons[slot]->setStyleSheet(QStringLiteral("background:%1; border:1px solid #888;").arg(m_colors.colors[slot].name()));
}

void ThemeColorsDialog::setSlot(int slot, const QColor &color)
{
    if (slot < 0 || slot >= Theme::ColorCount || !color.isValid()) return;
    m_colors.colors[slot] = color;
    paintButton(slot);
    m_preview->setColors(m_colors);
}

ThemeColors ThemeColorsDialog::result() const
{
    ThemeColors c = m_colors;
    c.name = m_name->text().trimmed();
    return c;
}

void ThemeColorsDialog::accept()
{
    const ThemeColors c = result();
    if (c.name.isEmpty()) {
        m_name->setFocus();
        return;
    }
    // (a built-in name is no name for a set of one's own)
    for (const ThemeColors &b : office::builtinThemeColors())
        if (b.name.compare(c.name, Qt::CaseInsensitive) == 0) {
            m_name->setFocus();
            m_name->selectAll();
            return;
        }
    if (!office::saveCustomThemeColors(c)) {
        jp::plainWarning(this, windowTitle(), QCoreApplication::translate("jp::ThemeDialogs", "The colors could not be saved."));
        return;
    }
    QDialog::accept();
}

// ---------------- Create New Theme Fonts ----------------
ThemeFontsDialog::ThemeFontsDialog(const ThemeFonts &start, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("themeFontsDialog"));
    setWindowTitle(QCoreApplication::translate("jp::ThemeDialogs", "Create New Theme Fonts"));
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();
    m_heading = new QComboBox(this);
    m_body = new QComboBox(this);
    m_heading->setObjectName(QStringLiteral("themeHeadingFont"));
    m_body->setObjectName(QStringLiteral("themeBodyFont"));
    const QStringList families = visibleFontFamilies();
    for (QComboBox *box : {m_heading, m_body}) {
        box->setEditable(false);
        box->addItems(families);
    }
    auto choose = [&](QComboBox *box, const QString &family) {
        int at = box->findText(family);
        if (at < 0 && !family.isEmpty()) {
            box->addItem(family);
            at = box->count() - 1;
        }
        box->setCurrentIndex(at);
    };
    choose(m_heading, start.heading);
    choose(m_body, start.body);
    m_name = new QLineEdit(this);
    m_name->setMaxLength(office::kMaxThemeNameLength);
    QStringList taken = namesOfFonts(office::customThemeFonts());
    for (const ThemeFonts &f : office::builtinThemeFonts()) taken << f.name;
    m_name->setText(hasName(taken, start.name) || start.name.isEmpty() ? newThemeSetName(taken) : start.name);
    form->addRow(QCoreApplication::translate("jp::ThemeDialogs", "Heading font:"), m_heading);
    form->addRow(QCoreApplication::translate("jp::ThemeDialogs", "Body font:"), m_body);
    form->addRow(QCoreApplication::translate("jp::ThemeDialogs", "Name:"), m_name);
    layout->addLayout(form);
    m_preview = new ThemePreview(this);
    layout->addWidget(m_preview);
    connect(m_heading, &QComboBox::currentTextChanged, this, [this] { updatePreview(); });
    connect(m_body, &QComboBox::currentTextChanged, this, [this] { updatePreview(); });
    updatePreview();
    auto *box = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(box);
}

void ThemeFontsDialog::updatePreview() { m_preview->setFonts(m_heading->currentText(), m_body->currentText()); }

ThemeFonts ThemeFontsDialog::result() const { return {m_name->text().trimmed(), m_heading->currentText(), m_body->currentText()}; }

void ThemeFontsDialog::accept()
{
    const ThemeFonts f = result();
    if (f.name.isEmpty() || f.heading.isEmpty() || f.body.isEmpty()) {
        m_name->setFocus();
        return;
    }
    for (const ThemeFonts &b : office::builtinThemeFonts())
        if (b.name.compare(f.name, Qt::CaseInsensitive) == 0) {
            m_name->setFocus();
            m_name->selectAll();
            return;
        }
    if (!office::saveCustomThemeFonts(f)) {
        jp::plainWarning(this, windowTitle(), QCoreApplication::translate("jp::ThemeDialogs", "The fonts could not be saved."));
        return;
    }
    QDialog::accept();
}

// ---------------- the menus ----------------
void addThemeColorsEntries(QMenu *menu, const QVector<ThemeColors> &builtins, const ThemeColors &current, const ThemeMenuHooks &hooks)
{
    auto *group = new QActionGroup(menu);
    auto add = [&](const ThemeColors &set) {
        QAction *a = menu->addAction(themeColorsIcon(set), set.name);
        a->setCheckable(true);
        a->setChecked(sameScheme(set, current));
        group->addAction(a);
        QObject::connect(a, &QAction::triggered, menu, [hooks, set] { hooks.chooseColors(set); });
    };
    const QVector<ThemeColors> custom = office::customThemeColors();
    if (!custom.isEmpty()) {
        menu->addSection(QCoreApplication::translate("jp::ThemeDialogs", "Custom"));
        for (const ThemeColors &set : custom) add(set);
    }
    menu->addSection(QCoreApplication::translate("jp::ThemeDialogs", "Built-In"));
    for (const ThemeColors &set : builtins) add(set);
    menu->addSeparator();
    QAction *create = menu->addAction(QCoreApplication::translate("jp::ThemeDialogs", "Create New Theme Colors…"));
    create->setObjectName(QStringLiteral("createThemeColors"));
    QObject::connect(create, &QAction::triggered, menu, [hooks, current] {
        ThemeColorsDialog dlg(current, hooks.dialogParent ? hooks.dialogParent : QApplication::activeWindow());
        if (dlg.exec() == QDialog::Accepted) hooks.chooseColors(dlg.result());
    });
    if (!custom.isEmpty()) {
        QMenu *remove = menu->addMenu(QCoreApplication::translate("jp::ThemeDialogs", "Delete Custom Colors"));
        for (const ThemeColors &set : custom) {
            const QString name = set.name;
            QObject::connect(remove->addAction(themeColorsIcon(set), name), &QAction::triggered, menu, [name] { office::removeCustomThemeColors(name); });
        }
    }
    if (hooks.reset) {
        QAction *reset = menu->addAction(QCoreApplication::translate("jp::ThemeDialogs", "Reset to the Template's Theme"));
        reset->setObjectName(QStringLiteral("resetThemeColors"));
        QObject::connect(reset, &QAction::triggered, menu, [hooks] { hooks.reset(); });
    }
}

void addThemeFontsEntries(QMenu *menu, const QVector<ThemeFonts> &builtins, const ThemeFonts &current, const ThemeMenuHooks &hooks)
{
    auto *group = new QActionGroup(menu);
    auto add = [&](const ThemeFonts &set) {
        QAction *a = menu->addAction(QCoreApplication::translate("jp::ThemeDialogs", "%1  (%2 / %3)").arg(set.name, set.heading, set.body));
        a->setCheckable(true);
        a->setChecked(set == current);
        group->addAction(a);
        QObject::connect(a, &QAction::triggered, menu, [hooks, set] { hooks.chooseFonts(set); });
    };
    const QVector<ThemeFonts> custom = office::customThemeFonts();
    if (!custom.isEmpty()) {
        menu->addSection(QCoreApplication::translate("jp::ThemeDialogs", "Custom"));
        for (const ThemeFonts &set : custom) add(set);
    }
    menu->addSection(QCoreApplication::translate("jp::ThemeDialogs", "Built-In"));
    for (const ThemeFonts &set : builtins) add(set);
    menu->addSeparator();
    QAction *create = menu->addAction(QCoreApplication::translate("jp::ThemeDialogs", "Create New Theme Fonts…"));
    create->setObjectName(QStringLiteral("createThemeFonts"));
    QObject::connect(create, &QAction::triggered, menu, [hooks, current] {
        ThemeFontsDialog dlg(current, hooks.dialogParent ? hooks.dialogParent : QApplication::activeWindow());
        if (dlg.exec() == QDialog::Accepted) hooks.chooseFonts(dlg.result());
    });
    if (!custom.isEmpty()) {
        QMenu *remove = menu->addMenu(QCoreApplication::translate("jp::ThemeDialogs", "Delete Custom Fonts"));
        for (const ThemeFonts &set : custom) {
            const QString name = set.name;
            QObject::connect(remove->addAction(name), &QAction::triggered, menu, [name] { office::removeCustomThemeFonts(name); });
        }
    }
    if (hooks.reset) {
        QAction *reset = menu->addAction(QCoreApplication::translate("jp::ThemeDialogs", "Reset to the Template's Theme"));
        reset->setObjectName(QStringLiteral("resetThemeFonts"));
        QObject::connect(reset, &QAction::triggered, menu, [hooks] { hooks.reset(); });
    }
}

} // namespace jp
