// The Layout and Design dialogs: More Columns, Line Numbers, and Page Borders (the work on the document is core/layoutops.h).

#include "app/writedialogs.h"

#include "app/widgets.h"
#include "core/layoutops.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>
#include <algorithm>
#include <memory>

namespace jp::write {

namespace {


MeasureSpin *measure(QWidget *parent, const char *name, double points)
{
    auto *s = new MeasureSpin(parent);
    s->setObjectName(QString::fromLatin1(name));
    s->setPoints(points);
    s->setValue(points);
    return s;
}

QComboBox *scopeBox(QWidget *parent)
{
    auto *scope = new QComboBox(parent);
    scope->setObjectName(QStringLiteral("scope"));
    scope->addItem(QCoreApplication::translate("jp::write::LayoutDialogs", "This section"));
    scope->addItem(QCoreApplication::translate("jp::write::LayoutDialogs", "Whole document"));
    return scope;
}

} // namespace

bool columnsDialog(QWidget *parent, WriteDoc *doc, int section)
{
    if (!doc) return false;
    section = std::clamp(section, 0, doc->sectionCount() - 1);
    const ColumnSetup before = columnSetupOf(doc->sectionProps(section));
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("columnsDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::LayoutDialogs", "Columns"));
    auto *v = new QVBoxLayout(&dlg);
    auto *presets = new QHBoxLayout;
    v->addLayout(presets);
    auto *count = new QSpinBox(&dlg);
    count->setObjectName(QStringLiteral("count"));
    count->setRange(1, kMostColumnsSet);
    count->setValue(before.count);
    auto *equal = new QCheckBox(QCoreApplication::translate("jp::write::LayoutDialogs", "Equal column width"), &dlg);
    equal->setObjectName(QStringLiteral("equal"));
    equal->setChecked(before.equal);
    auto *line = new QCheckBox(QCoreApplication::translate("jp::write::LayoutDialogs", "Line between"), &dlg);
    line->setObjectName(QStringLiteral("line"));
    line->setChecked(before.line);
    auto *form = new QFormLayout;
    form->addRow(QCoreApplication::translate("jp::write::LayoutDialogs", "Number of columns:"), count);
    form->addRow(line);
    form->addRow(equal);
    v->addLayout(form);
    // One row for each column: its width, and the space after it (the equal ones share the first row's space).
    auto *grid = new QGridLayout;
    grid->addWidget(new QLabel(QCoreApplication::translate("jp::write::LayoutDialogs", "Column"), &dlg), 0, 0);
    grid->addWidget(new QLabel(QCoreApplication::translate("jp::write::LayoutDialogs", "Width"), &dlg), 0, 1);
    grid->addWidget(new QLabel(QCoreApplication::translate("jp::write::LayoutDialogs", "Spacing"), &dlg), 0, 2);
    QVector<MeasureSpin *> widths, spaces;
    auto *rows = new QGroupBox(&dlg);
    rows->setLayout(grid);
    for (int i = 0; i < kMostColumnsSet; ++i) {
        auto *label = new QLabel(QString::number(i + 1), rows);
        auto *w = measure(rows, ("width" + std::to_string(i + 1)).c_str(), i < before.widths.size() ? before.widths[i] : 100);
        auto *s = measure(rows, ("space" + std::to_string(i + 1)).c_str(), i < before.spaces.size() ? before.spaces[i] : before.space);
        grid->addWidget(label, i + 1, 0);
        grid->addWidget(w, i + 1, 1);
        grid->addWidget(s, i + 1, 2);
        widths << w;
        spaces << s;
    }
    v->addWidget(rows);
    auto *scope = scopeBox(&dlg);
    auto *scopeForm = new QFormLayout;
    scopeForm->addRow(QCoreApplication::translate("jp::write::LayoutDialogs", "Apply to:"), scope);
    v->addLayout(scopeForm);
    // The rows shown are the columns there are; for equal columns only the spacing is free, the first row's, which all the others copy.
    auto refresh = [=] {
        const bool eq = equal->isChecked();
        for (int i = 0; i < kMostColumnsSet; ++i) {
            const bool shown = i < count->value();
            const QList<QWidget *> cells = {widths[i], spaces[i]};
            for (QWidget *w : cells) w->setVisible(shown);
            grid->itemAtPosition(i + 1, 0)->widget()->setVisible(shown);
            widths[i]->setEnabled(!eq);
            spaces[i]->setEnabled(eq ? i == 0 : i + 1 < count->value());
        }
        line->setEnabled(count->value() > 1);
    };
    auto shareSpace = [=] {
        if (!equal->isChecked()) return;
        for (int i = 1; i < kMostColumnsSet; ++i) spaces[i]->setValue(spaces[0]->value());
    };
    QObject::connect(count, &QSpinBox::valueChanged, &dlg, refresh);
    QObject::connect(equal, &QCheckBox::toggled, &dlg, [=](bool) { refresh(); shareSpace(); });
    QObject::connect(spaces[0], &QDoubleSpinBox::valueChanged, &dlg, shareSpace);
    // The presets: One, Two, Three, Left (a narrow column and a wide one), and Right.
    auto preset = [=, &dlg](const QString &name, int n, double first, double second) {
        auto *b = new QPushButton(name, &dlg);
        b->setObjectName(QStringLiteral("preset") + name);
        presets->addWidget(b);
        QObject::connect(b, &QPushButton::clicked, &dlg, [=] {
            const QSignalBlocker block(count);
            count->setValue(n);
            const double text = std::max(1.0, 468.0 - spaces[0]->value() * (n - 1));
            if (first > 0) {
                equal->setChecked(false);
                widths[0]->setValue(text * first);
                widths[1]->setValue(text * second);
                spaces[0]->setValue(spaces[0]->value());
            } else {
                equal->setChecked(true);
            }
            refresh();
        });
    };
    preset(QCoreApplication::translate("jp::write::LayoutDialogs", "One"), 1, 0, 0);
    preset(QCoreApplication::translate("jp::write::LayoutDialogs", "Two"), 2, 0, 0);
    preset(QCoreApplication::translate("jp::write::LayoutDialogs", "Three"), 3, 0, 0);
    preset(QCoreApplication::translate("jp::write::LayoutDialogs", "Left"), 2, 1.0 / 3, 2.0 / 3);
    preset(QCoreApplication::translate("jp::write::LayoutDialogs", "Right"), 2, 2.0 / 3, 1.0 / 3);
    refresh();
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(bb);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return false;
    ColumnSetup after;
    after.count = count->value();
    after.equal = equal->isChecked();
    after.space = spaces[0]->value();
    after.line = line->isChecked();
    for (int i = 0; i < after.count; ++i) {
        after.widths << widths[i]->value();
        after.spaces << spaces[i]->value();
    }
    after = normalized(after);
    return applyToSections(*doc, section, scope->currentIndex() == 1, [after](office::wml::SectionProps *p) { setColumnSetup(p, after); }) > 0;
}

bool lineNumbersDialog(QWidget *parent, WriteDoc *doc, int section)
{
    if (!doc) return false;
    section = std::clamp(section, 0, doc->sectionCount() - 1);
    const LineNumberSetup before = lineNumbersOf(doc->sectionProps(section));
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("lineNumbersDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::LayoutDialogs", "Line Numbers"));
    auto *v = new QVBoxLayout(&dlg);
    auto *on = new QCheckBox(QCoreApplication::translate("jp::write::LayoutDialogs", "Add line numbering"), &dlg);
    on->setObjectName(QStringLiteral("on"));
    on->setChecked(before.countBy > 0);
    v->addWidget(on);
    auto *form = new QFormLayout;
    auto *start = new QSpinBox(&dlg);
    start->setObjectName(QStringLiteral("start"));
    start->setRange(1, 32768);
    start->setValue(before.start);
    auto *by = new QSpinBox(&dlg);
    by->setObjectName(QStringLiteral("countBy"));
    by->setRange(1, 1000);
    by->setValue(std::max(1, before.countBy));
    auto *distance = measure(&dlg, "distance", before.distance);
    auto *restart = new QComboBox(&dlg);
    restart->setObjectName(QStringLiteral("restart"));
    restart->addItem(QCoreApplication::translate("jp::write::LayoutDialogs", "Restart each page"));
    restart->addItem(QCoreApplication::translate("jp::write::LayoutDialogs", "Restart each section"));
    restart->addItem(QCoreApplication::translate("jp::write::LayoutDialogs", "Continuous"));
    restart->setCurrentIndex(int(before.restart));
    form->addRow(QCoreApplication::translate("jp::write::LayoutDialogs", "Start at:"), start);
    form->addRow(QCoreApplication::translate("jp::write::LayoutDialogs", "From text:"), distance);
    form->addRow(QCoreApplication::translate("jp::write::LayoutDialogs", "Count by:"), by);
    form->addRow(QCoreApplication::translate("jp::write::LayoutDialogs", "Numbering:"), restart);
    auto *scope = scopeBox(&dlg);
    form->addRow(QCoreApplication::translate("jp::write::LayoutDialogs", "Apply to:"), scope);
    v->addLayout(form);
    auto enable = [=] {
        for (QWidget *w : {static_cast<QWidget *>(start), static_cast<QWidget *>(by), static_cast<QWidget *>(distance), static_cast<QWidget *>(restart)}) w->setEnabled(on->isChecked());
    };
    QObject::connect(on, &QCheckBox::toggled, &dlg, enable);
    enable();
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(bb);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return false;
    LineNumberSetup after;
    after.countBy = on->isChecked() ? by->value() : 0;
    after.start = start->value();
    after.distance = distance->value();
    after.restart = LineNumberSetup::Restart(restart->currentIndex());
    return applyToSections(*doc, section, scope->currentIndex() == 1, [after](office::wml::SectionProps *p) { setLineNumbers(p, after); }) > 0;
}

bool pageBordersDialog(QWidget *parent, WriteDoc *doc, int section)
{
    if (!doc) return false;
    section = std::clamp(section, 0, doc->sectionCount() - 1);
    const PageBorderSetup before = pageBordersOf(doc->sectionProps(section));
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("pageBordersDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::LayoutDialogs", "Page Borders"));
    auto *v = new QVBoxLayout(&dlg);
    auto *sides = new QGroupBox(QCoreApplication::translate("jp::write::LayoutDialogs", "Setting"), &dlg);
    auto *sv = new QHBoxLayout(sides);
    auto side = [&](const char *name, const QString &text, bool on) {
        auto *c = new QCheckBox(text, sides);
        c->setObjectName(QString::fromLatin1(name));
        c->setChecked(on);
        sv->addWidget(c);
        return c;
    };
    QCheckBox *top = side("top", QCoreApplication::translate("jp::write::LayoutDialogs", "Top"), before.top), *left = side("left", QCoreApplication::translate("jp::write::LayoutDialogs", "Left"), before.left), *bottom = side("bottom", QCoreApplication::translate("jp::write::LayoutDialogs", "Bottom"), before.bottom),
              *right = side("right", QCoreApplication::translate("jp::write::LayoutDialogs", "Right"), before.right);
    auto *all = new QPushButton(QCoreApplication::translate("jp::write::LayoutDialogs", "Box"), sides);
    all->setObjectName(QStringLiteral("box"));
    auto *none = new QPushButton(QCoreApplication::translate("jp::write::LayoutDialogs", "None"), sides);
    none->setObjectName(QStringLiteral("none"));
    sv->addWidget(all);
    sv->addWidget(none);
    QObject::connect(all, &QPushButton::clicked, &dlg, [=] { for (QCheckBox *c : {top, left, bottom, right}) c->setChecked(true); });
    QObject::connect(none, &QPushButton::clicked, &dlg, [=] { for (QCheckBox *c : {top, left, bottom, right}) c->setChecked(false); });
    v->addWidget(sides);
    auto *form = new QFormLayout;
    auto *style = new QComboBox(&dlg);
    style->setObjectName(QStringLiteral("style"));
    const QList<QPair<QString, QString>> styles = {{QCoreApplication::translate("jp::write::LayoutDialogs", "Single line"), QStringLiteral("single")}, {QCoreApplication::translate("jp::write::LayoutDialogs", "Thick line"), QStringLiteral("thick")}, {QCoreApplication::translate("jp::write::LayoutDialogs", "Double line"), QStringLiteral("double")},
                                                   {QCoreApplication::translate("jp::write::LayoutDialogs", "Dotted"), QStringLiteral("dotted")}, {QCoreApplication::translate("jp::write::LayoutDialogs", "Dashed"), QStringLiteral("dashed")}};
    for (const auto &s : styles) style->addItem(s.first, s.second);
    style->setCurrentIndex(std::max(0, style->findData(before.style)));
    auto *width = new QDoubleSpinBox(&dlg);
    width->setObjectName(QStringLiteral("width"));
    width->setRange(0.25, 12);
    width->setSingleStep(0.25);
    width->setSuffix(QStringLiteral(" pt"));
    width->setValue(before.width);
    QColor chosen = before.color;
    auto *color = new QPushButton(&dlg);
    color->setObjectName(QStringLiteral("color"));
    auto paintColor = [=](const QColor &c) { color->setStyleSheet(QStringLiteral("background-color: %1; min-width: 48px;").arg(c.name())); };
    paintColor(chosen);
    auto picked = std::make_shared<QColor>(chosen);
    QObject::connect(color, &QPushButton::clicked, &dlg, [=, &dlg] {
        const QColor c = QColorDialog::getColor(*picked, &dlg);
        if (!c.isValid()) return;
        *picked = c;
        paintColor(c);
    });
    auto *space = new QSpinBox(&dlg);
    space->setObjectName(QStringLiteral("space"));
    space->setRange(0, 31);
    space->setSuffix(QStringLiteral(" pt"));
    space->setValue(int(before.space));
    auto *from = new QComboBox(&dlg);
    from->setObjectName(QStringLiteral("measureFrom"));
    from->addItem(QCoreApplication::translate("jp::write::LayoutDialogs", "Text"));
    from->addItem(QCoreApplication::translate("jp::write::LayoutDialogs", "Edge of page"));
    from->setCurrentIndex(before.fromPage ? 1 : 0);
    auto *show = new QComboBox(&dlg);
    show->setObjectName(QStringLiteral("show"));
    show->addItem(QCoreApplication::translate("jp::write::LayoutDialogs", "Whole document"), QStringLiteral("all"));
    show->addItem(QCoreApplication::translate("jp::write::LayoutDialogs", "This section, first page only"), QStringLiteral("firstPage"));
    show->addItem(QCoreApplication::translate("jp::write::LayoutDialogs", "This section, all except first page"), QStringLiteral("notFirstPage"));
    show->setCurrentIndex(std::max(0, show->findData(before.display)));
    form->addRow(QCoreApplication::translate("jp::write::LayoutDialogs", "Style:"), style);
    form->addRow(QCoreApplication::translate("jp::write::LayoutDialogs", "Width:"), width);
    form->addRow(QCoreApplication::translate("jp::write::LayoutDialogs", "Color:"), color);
    form->addRow(QCoreApplication::translate("jp::write::LayoutDialogs", "Distance:"), space);
    form->addRow(QCoreApplication::translate("jp::write::LayoutDialogs", "Measure from:"), from);
    form->addRow(QCoreApplication::translate("jp::write::LayoutDialogs", "Pages:"), show);
    auto *scope = scopeBox(&dlg);
    form->addRow(QCoreApplication::translate("jp::write::LayoutDialogs", "Apply to:"), scope);
    v->addLayout(form);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(bb);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return false;
    PageBorderSetup after;
    after.top = top->isChecked();
    after.left = left->isChecked();
    after.bottom = bottom->isChecked();
    after.right = right->isChecked();
    after.style = style->currentData().toString();
    after.width = width->value();
    after.color = *picked;
    after.space = space->value();
    after.fromPage = from->currentIndex() == 1;
    after.display = show->currentData().toString();
    return applyToSections(*doc, section, scope->currentIndex() == 1, [after](office::wml::SectionProps *p) { setPageBorders(p, after); }) > 0;
}

} // namespace jp::write
