#include "app/guidesdialog.h"
#include "app/uistyle.h"
#include "app/editor.h"
#include "app/settings.h"
#include "app/widgets.h"
#include "core/document.h"
#include <QCheckBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <algorithm>
#include "app/plaintext.h"

namespace jp {

namespace {

struct Dlg {
    QDialog d;
    QVBoxLayout *v;
    QDialogButtonBox *bb;
    explicit Dlg(QWidget *p, const QString &title, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok | QDialogButtonBox::Cancel) : d(p)
    {
        d.setWindowTitle(title);
        v = new QVBoxLayout(&d);
        bb = new jp::ButtonBox(buttons, &d);
        QObject::connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
        QObject::connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    }
    bool exec()
    {
        v->addWidget(bb);
        return d.exec() == QDialog::Accepted;
    }
};

MeasureSpin *measure(double pt, QWidget *p, double min = 0, double max = 72 * 240)
{
    auto *m = new MeasureSpin(p);
    m->setRange(min, max);
    m->setValue(pt);
    m->setFixedWidth(110);
    return m;
}

QDoubleSpinBox *points(double v, QWidget *p, double min = 0, double max = 1000, const QString &suffix = QStringLiteral(" pt"))
{
    auto *s = new DecimalSpin(p);
    s->setRange(min, max);
    s->setValue(v);
    s->setSuffix(suffix);
    return s;
}

} // namespace

// ---------------- Grid and Baseline Guides ----------------
void gridGuidesDialog(QWidget *p, Editor *ed, int startTab)
{
    Document *doc = ed->doc();
    if (doc->pages.isEmpty() || doc->masters.isEmpty()) return;
    MasterPage *mp = ed->masterView().isEmpty() ? doc->masterFor(*doc->pages[ed->currentPage()]) : doc->master(ed->masterView());
    if (!mp) mp = doc->masters.first().get();
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Layout Guides"));
    auto *tabs = new QTabWidget(&dlg.d);
    auto *g = new QWidget();
    auto *gf = new QFormLayout(g);
    auto *cols = new QSpinBox(g), *rows = new QSpinBox(g);
    cols->setRange(1, 63);
    rows->setRange(1, 63);
    cols->setValue(mp->grid.cols);
    rows->setValue(mp->grid.rows);
    auto *cg = measure(mp->grid.colGap, g), *rg = measure(mp->grid.rowGap, g);
    auto *center = new QCheckBox(QCoreApplication::translate("Dialogs", "Add center guide between columns and rows"), g);
    center->setChecked(mp->grid.centerGuide);
    gf->addRow(QCoreApplication::translate("Dialogs", "Columns"), cols);
    gf->addRow(QCoreApplication::translate("Dialogs", "Column spacing"), cg);
    gf->addRow(QCoreApplication::translate("Dialogs", "Rows"), rows);
    gf->addRow(QCoreApplication::translate("Dialogs", "Row spacing"), rg);
    gf->addRow(center);
    tabs->addTab(g, QCoreApplication::translate("Dialogs", "Grid Guides"));
    auto *b = new QWidget();
    auto *bf = new QFormLayout(b);
    auto *spacing = points(mp->grid.baseline, b, 1, 200);
    auto *offset = points(mp->grid.baselineOffset, b, 0, 200);
    bf->addRow(QCoreApplication::translate("Dialogs", "Horizontal baseline spacing"), spacing);
    bf->addRow(QCoreApplication::translate("Dialogs", "Offset"), offset);
    tabs->addTab(b, QCoreApplication::translate("Dialogs", "Baseline Guides"));
    auto *mg = new QWidget();
    auto *mgf = new QFormLayout(mg);
    auto *twoPage = new QCheckBox(QCoreApplication::translate("Dialogs", "Two-page master"), mg);
    twoPage->setChecked(mp->twoPage);
    mgf->addRow(twoPage);
    const QMarginsF m = doc->setup.margins;
    auto *mt = measure(m.top(), mg), *mb = measure(m.bottom(), mg), *ml = measure(m.left(), mg), *mr = measure(m.right(), mg);
    mgf->addRow(QCoreApplication::translate("Dialogs", "Top"), mt);
    mgf->addRow(QCoreApplication::translate("Dialogs", "Bottom"), mb);
    mgf->addRow(QCoreApplication::translate("Dialogs", "Left"), ml);
    mgf->addRow(QCoreApplication::translate("Dialogs", "Right"), mr);
    // A two-page master's side guides are inside and outside.
    auto sideNames = [=](bool two) {
        if (auto *l = qobject_cast<QLabel *>(mgf->labelForField(ml))) l->setText(two ? QCoreApplication::translate("Dialogs", "Inside") : QCoreApplication::translate("Dialogs", "Left"));
        if (auto *l = qobject_cast<QLabel *>(mgf->labelForField(mr))) l->setText(two ? QCoreApplication::translate("Dialogs", "Outside") : QCoreApplication::translate("Dialogs", "Right"));
    };
    sideNames(mp->twoPage);
    QObject::connect(twoPage, &QCheckBox::toggled, mg, sideNames);
    tabs->insertTab(0, mg, QCoreApplication::translate("Dialogs", "Margin Guides"));
    tabs->setCurrentIndex(std::clamp(startTab, 0, tabs->count() - 1));
    dlg.v->addWidget(tabs);
    if (!dlg.exec()) return;
    ed->change(QCoreApplication::translate("Dialogs", "Layout Guides"), [&] {
        mp->grid.cols = cols->value();
        mp->grid.rows = rows->value();
        mp->grid.colGap = cg->value();
        mp->grid.rowGap = rg->value();
        mp->grid.centerGuide = center->isChecked();
        mp->grid.baseline = spacing->value();
        mp->grid.baselineOffset = offset->value();
        mp->twoPage = twoPage->isChecked();
        doc->setup.margins = QMarginsF(ml->value(), mt->value(), mr->value(), mb->value());
    });
}

// ---------------- Grid and Guides (a presentation's) ----------------
bool gridSettingsDialog(QWidget *p, GridSettings &s, const QSizeF &page)
{
    Dlg dlg(p, QCoreApplication::translate("GuidesDialog", "Grid and Guides"));
    dlg.d.setObjectName(QStringLiteral("gridGuidesDialog"));
    auto *form = new QFormLayout();
    auto *spacing = measure(std::max(kMinGridSpacing, s.spacing), &dlg.d, kMinGridSpacing, 72 * 10);
    spacing->setObjectName(QStringLiteral("gridSpacing"));
    form->addRow(QCoreApplication::translate("GuidesDialog", "Grid spacing"), spacing);
    auto check = [&](const QString &name, const QString &text, bool on) {
        auto *c = new QCheckBox(text, &dlg.d);
        c->setObjectName(name);
        c->setChecked(on);
        form->addRow(c);
        return c;
    };
    auto *showGrid = check(QStringLiteral("showGrid"), QCoreApplication::translate("GuidesDialog", "Show grid on the slide"), s.showGrid);
    auto *snapGrid = check(QStringLiteral("snapGrid"), QCoreApplication::translate("GuidesDialog", "Snap objects to the grid"), s.snapGrid);
    auto *snapObjects = check(QStringLiteral("snapObjects"), QCoreApplication::translate("GuidesDialog", "Snap objects to other objects"), s.snapObjects);
    auto *showGuides = check(QStringLiteral("showGuides"), QCoreApplication::translate("GuidesDialog", "Show guides on the slide"), s.showGuides);
    dlg.v->addLayout(form);

    // The guides: how many there are, a button for each kind to add one (at the middle, or the next free place), and one to take them all away.
    RulerGuides guides = s.guides;
    auto *count = plainLabel(new QLabel(&dlg.d));
    count->setObjectName(QStringLiteral("guideCount"));
    auto *addV = new QPushButton(QCoreApplication::translate("GuidesDialog", "Add Vertical Guide"), &dlg.d);
    addV->setObjectName(QStringLiteral("addVertical"));
    auto *addH = new QPushButton(QCoreApplication::translate("GuidesDialog", "Add Horizontal Guide"), &dlg.d);
    addH->setObjectName(QStringLiteral("addHorizontal"));
    auto *clear = new QPushButton(QCoreApplication::translate("GuidesDialog", "Remove All Guides"), &dlg.d);
    clear->setObjectName(QStringLiteral("removeGuides"));
    auto refresh = [&]() {
        count->setText(QCoreApplication::translate("GuidesDialog", "%n vertical guide(s)", nullptr, int(guides.v.size())) + QStringLiteral(", ") +
                       QCoreApplication::translate("GuidesDialog", "%n horizontal guide(s)", nullptr, int(guides.h.size())));
        addV->setEnabled(guides.v.size() < kMaxGuidesPerAxis);
        addH->setEnabled(guides.h.size() < kMaxGuidesPerAxis);
        clear->setEnabled(!guides.v.isEmpty() || !guides.h.isEmpty());
    };
    QObject::connect(addV, &QPushButton::clicked, &dlg.d, [&, refresh]() {
        guides.v << RulerGuides::freeSpot(guides.v, page.width() / 2, std::max(kMinGridSpacing, spacing->value()), page.width());
        showGuides->setChecked(true);
        refresh();
    });
    QObject::connect(addH, &QPushButton::clicked, &dlg.d, [&, refresh]() {
        guides.h << RulerGuides::freeSpot(guides.h, page.height() / 2, std::max(kMinGridSpacing, spacing->value()), page.height());
        showGuides->setChecked(true);
        refresh();
    });
    QObject::connect(clear, &QPushButton::clicked, &dlg.d, [&, refresh]() {
        guides.h.clear();
        guides.v.clear();
        refresh();
    });
    refresh();
    dlg.v->addWidget(count);
    auto *row = new QHBoxLayout();
    row->addWidget(addV);
    row->addWidget(addH);
    row->addWidget(clear);
    row->addStretch(1);
    dlg.v->addLayout(row);
    auto *hint = plainLabel(new QLabel(QCoreApplication::translate("GuidesDialog", "Drag from a ruler to add a guide by hand. Hold Alt while dragging to leave snapping off."), &dlg.d));
    hint->setWordWrap(true);
    hint->setEnabled(false);
    dlg.v->addWidget(hint);
    if (!dlg.exec()) return false;
    s.spacing = std::max(kMinGridSpacing, spacing->value());
    s.showGrid = showGrid->isChecked();
    s.snapGrid = snapGrid->isChecked();
    s.snapObjects = snapObjects->isChecked();
    s.showGuides = showGuides->isChecked();
    s.guides = guides;
    return true;
}

} // namespace jp
