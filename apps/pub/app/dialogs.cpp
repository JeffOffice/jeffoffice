#include "app/dialogs.h"
#include "app/mainwindow.h"

#include "app/appfuncs.h"
#include "app/optionsdialog.h"
#include "app/editor.h"
#include "app/icons.h"
#include "app/settings.h"
#include "app/widgets.h"
#include "core/presets.h"
#include "io/importers.h"
#include "templates/templates.h"
#include "text/textprops.h"

#include <QApplication>
#include <QButtonGroup>
#include <QCalendarWidget>
#include <QCheckBox>

#include <QComboBox>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QDateTimeEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRandomGenerator>
#include <QRadioButton>
#include <QRegularExpression>
#include <QSlider>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTimer>
#include <QTableWidget>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextList>
#include <QVBoxLayout>
#include <QBuffer>
#include <QStyleHints>
#include <QPointer>
#include <memory>
#include <QClipboard>
#include <QMimeData>
#include <QTextDocumentFragment>

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
        bb = new QDialogButtonBox(buttons, &d);
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

ColorButton *colorPick(Editor *ed, const ColorRef &c, bool allowNone, QWidget *p)
{
    auto *b = new ColorButton("square", QString(), allowNone, QCoreApplication::translate("Dialogs", "No Color"), p);
    b->setScheme(ed->doc()->colors);
    b->setCurrent(c);
    b->setPopupMode(QToolButton::InstantPopup);
    b->setWell(true);
    QObject::connect(b, &ColorButton::colorPicked, b, [b](const ColorRef &x) { b->setCurrent(x); });
    return b;
}

} // namespace

// ---------------- Page Setup ----------------
// ---------------- Page size, margins and margin guides ----------------
// The page fields Page Setup and Create New Page Size share. Margin guides
// are the non-printing guides on each page; margins and gaps place pages on
// the printed sheet when several share one (multiple pages per sheet, labels).
namespace {

struct PageForm {
    QComboBox *layout = nullptr, *preset = nullptr;
    MeasureSpin *w = nullptr, *h = nullptr, *sheetW = nullptr, *sheetH = nullptr;
    MeasureSpin *side = nullptr, *top = nullptr, *gapH = nullptr, *gapV = nullptr;
    MeasureSpin *gTop = nullptr, *gBottom = nullptr, *gLeft = nullptr, *gRight = nullptr;
    QLabel *fits = nullptr;
    QFormLayout *pageForm = nullptr;

    bool multiple() const
    {
        const int l = layout->currentIndex();
        return l == PageSetup::MultiplePerSheet || l == PageSetup::Labels;
    }
    // How many pages fit across and down the sheet.
    QPair<int, int> grid() const
    {
        const double cw = w->value() + gapH->value(), ch = h->value() + gapV->value();
        const int cols = cw > 0 ? std::max(1, int((sheetW->value() - 2 * side->value() + gapH->value() + 0.001) / cw)) : 1;
        const int rows = ch > 0 ? std::max(1, int((sheetH->value() - 2 * top->value() + gapV->value() + 0.001) / ch)) : 1;
        return qMakePair(cols, rows);
    }
    void update()
    {
        const bool m = multiple();
        const bool changed = pageForm->isRowVisible(side) != m;
        for (QWidget *f : {static_cast<QWidget *>(side), static_cast<QWidget *>(top), static_cast<QWidget *>(gapH), static_cast<QWidget *>(gapV),
                           static_cast<QWidget *>(fits)})
            pageForm->setRowVisible(f, m);
        if (m) {
            const auto g = grid();
            fits->setText(QCoreApplication::translate("Dialogs", "%1 across × %2 down = %3 per sheet").arg(g.first).arg(g.second).arg(g.first * g.second));
        }
        // Grow or shrink the dialog with the rows shown, so none are squeezed.
        if (QWidget *win = fits->window(); win && (changed || win->width() < win->sizeHint().width()))
            QTimer::singleShot(0, win, [win] { win->adjustSize(); });
    }
    void read(PageSetup &s) const
    {
        s.layout = PageSetup::Layout(layout->currentIndex());
        s.size = QSizeF(w->value(), h->value());
        s.sheet = QSizeF(sheetW->value(), sheetH->value());
        if (s.layout == PageSetup::OnePerSheet && (s.sheet.width() < s.size.width() || s.sheet.height() < s.size.height())) s.sheet = s.size;
        s.margins = QMarginsF(gLeft->value(), gTop->value(), gRight->value(), gBottom->value());
        s.sideMargin = side->value();
        s.topMargin = top->value();
        s.gapH = gapH->value();
        s.gapV = gapV->value();
        const auto g = grid();
        s.gridCols = multiple() ? g.first : 1;
        s.gridRows = multiple() ? g.second : 1;
    }
};

// Builds the Page and Margin Guides tabs into `tabs` from a page setup.
PageForm buildPageForm(QTabWidget *tabs, const PageSetup &s)
{
    PageForm f;
    auto *page = new QWidget();
    auto *form = new QFormLayout(page);
    f.pageForm = form;
    f.layout = new QComboBox(page);
    f.layout->addItems({QCoreApplication::translate("Dialogs", "One page per sheet"), QCoreApplication::translate("Dialogs", "Booklet"), QCoreApplication::translate("Dialogs", "Multiple pages per sheet"), QCoreApplication::translate("Dialogs", "Envelope"), QCoreApplication::translate("Dialogs", "Folded card"), QCoreApplication::translate("Dialogs", "Labels")});
    f.layout->setCurrentIndex(int(s.layout));
    f.preset = new QComboBox(page);
    f.preset->addItem(QCoreApplication::translate("Dialogs", "Custom"));
    for (const auto &bs : blankSizes()) f.preset->addItem(bs.name);
    const QJsonArray custom = Settings::get().customPageSizes();
    for (const QJsonValue &c : custom) f.preset->addItem(c.toObject()["name"].toString(), QStringLiteral("custom"));
    f.w = measure(s.size.width(), page, 9);
    f.h = measure(s.size.height(), page, 9);
    f.sheetW = measure(s.sheet.width(), page, 9);
    f.sheetH = measure(s.sheet.height(), page, 9);
    f.side = measure(s.sideMargin, page);
    f.top = measure(s.topMargin, page);
    f.gapH = measure(s.gapH, page);
    f.gapV = measure(s.gapV, page);
    f.fits = new QLabel(page);
    f.layout->setObjectName("pageLayout");
    f.w->setObjectName("pageWidth");
    f.h->setObjectName("pageHeight");
    f.sheetW->setObjectName("paperWidth");
    f.sheetH->setObjectName("paperHeight");
    f.side->setObjectName("sideMargin");
    f.top->setObjectName("topMargin");
    f.gapH->setObjectName("gapH");
    f.gapV->setObjectName("gapV");
    f.fits->setObjectName("perSheet");
    auto *portrait = new QRadioButton(QCoreApplication::translate("Dialogs", "Portrait"), page), *land = new QRadioButton(QCoreApplication::translate("Dialogs", "Landscape"), page);
    (s.size.width() > s.size.height() ? land : portrait)->setChecked(true);
    MeasureSpin *w = f.w, *h = f.h;
    auto swapIf = [=](bool landscape) {
        if ((w->value() > h->value()) != landscape) {
            const double t = w->value();
            w->setValue(h->value());
            h->setValue(t);
        }
    };
    QObject::connect(portrait, &QRadioButton::toggled, page, [=](bool on) { if (on) swapIf(false); });
    QObject::connect(land, &QRadioButton::toggled, page, [=](bool on) { if (on) swapIf(true); });
    auto *orient = new QHBoxLayout();
    orient->addWidget(portrait);
    orient->addWidget(land);
    form->addRow(QCoreApplication::translate("Dialogs", "Layout type:"), f.layout);
    form->addRow(QCoreApplication::translate("Dialogs", "Page size:"), f.preset);
    form->addRow(QCoreApplication::translate("Dialogs", "Width:"), f.w);
    form->addRow(QCoreApplication::translate("Dialogs", "Height:"), f.h);
    form->addRow(QCoreApplication::translate("Dialogs", "Orientation:"), orient);
    form->addRow(QCoreApplication::translate("Dialogs", "Paper width:"), f.sheetW);
    form->addRow(QCoreApplication::translate("Dialogs", "Paper height:"), f.sheetH);
    form->addRow(QCoreApplication::translate("Dialogs", "Side margin:"), f.side);
    form->addRow(QCoreApplication::translate("Dialogs", "Top margin:"), f.top);
    form->addRow(QCoreApplication::translate("Dialogs", "Horizontal gap:"), f.gapH);
    form->addRow(QCoreApplication::translate("Dialogs", "Vertical gap:"), f.gapV);
    form->addRow(QString(), f.fits);
    tabs->addTab(page, QCoreApplication::translate("Dialogs", "Page"));

    auto *mg = new QWidget();
    auto *mf = new QFormLayout(mg);
    auto *note = new QLabel(QCoreApplication::translate("Dialogs", "Margin guides are non-printing lines on every page for lining up objects."), mg);
    note->setWordWrap(true);
    mf->addRow(note);
    f.gTop = measure(s.margins.top(), mg);
    f.gBottom = measure(s.margins.bottom(), mg);
    f.gLeft = measure(s.margins.left(), mg);
    f.gRight = measure(s.margins.right(), mg);
    mf->addRow(QCoreApplication::translate("Dialogs", "Top:"), f.gTop);
    mf->addRow(QCoreApplication::translate("Dialogs", "Bottom:"), f.gBottom);
    mf->addRow(QCoreApplication::translate("Dialogs", "Left:"), f.gLeft);
    mf->addRow(QCoreApplication::translate("Dialogs", "Right:"), f.gRight);
    tabs->addTab(mg, QCoreApplication::translate("Dialogs", "Margin Guides"));

    // Choosing a size fills the fields; a custom size brings its whole setup.
    PageForm *pf = new PageForm(f);
    QObject::connect(page, &QObject::destroyed, [pf] { delete pf; });
    QObject::connect(f.preset, &QComboBox::currentIndexChanged, page, [pf](int i) {
        if (i <= 0) return;
        const int presets = int(blankSizes().size());
        if (i <= presets) {
            const auto &bs = blankSizes()[i - 1];
            pf->w->setValue(bs.size.width());
            pf->h->setValue(bs.size.height());
        } else {
            const PageSetup cs = PageSetup::fromJson(Settings::get().customPageSizes().at(i - 1 - presets).toObject()["setup"].toObject());
            pf->layout->setCurrentIndex(int(cs.layout));
            pf->w->setValue(cs.size.width());
            pf->h->setValue(cs.size.height());
            pf->sheetW->setValue(cs.sheet.width());
            pf->sheetH->setValue(cs.sheet.height());
            pf->side->setValue(cs.sideMargin);
            pf->top->setValue(cs.topMargin);
            pf->gapH->setValue(cs.gapH);
            pf->gapV->setValue(cs.gapV);
            pf->gTop->setValue(cs.margins.top());
            pf->gBottom->setValue(cs.margins.bottom());
            pf->gLeft->setValue(cs.margins.left());
            pf->gRight->setValue(cs.margins.right());
        }
        pf->update();
    });
    for (MeasureSpin *m : {f.w, f.h, f.sheetW, f.sheetH, f.side, f.top, f.gapH, f.gapV})
        QObject::connect(m, &QDoubleSpinBox::valueChanged, page, [pf] { pf->update(); });
    QObject::connect(f.layout, &QComboBox::currentIndexChanged, page, [pf] { pf->update(); });
    pf->update();
    return f;
}

} // namespace

void applyPageSetup(Editor *ed, const PageSetup &ns, const QString &sizeName, const QString &undoName)
{
    Document *doc = ed->doc();
    ed->change(undoName, [&] {
        const QSizeF oldSize = doc->setup.size;
        const PageSetup::Fold fold = doc->setup.fold;
        doc->setup = ns;
        doc->setup.fold = fold;
        doc->setup.sizeName = sizeName;
        doc->pageSizeChanged(oldSize);
    });
}

void applyPageSize(Editor *ed, const QSizeF &size, const QString &sizeName)
{
    PageSetup ns = ed->doc()->setup;
    // Keep the current orientation.
    QSizeF s = size;
    const QSizeF cur = ns.size;
    if ((cur.width() > cur.height()) != (s.width() > s.height()) && s.width() != s.height()) s = s.transposed();
    ns.size = s;
    ns.sheet = s;
    applyPageSetup(ed, ns, sizeName, QCoreApplication::translate("Dialogs", "Page Size"));
}

void pageSetupDialog(QWidget *p, Editor *ed)
{
    Document *doc = ed->doc();
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Page Setup"));
    auto *tabs = new QTabWidget(&dlg.d);
    const PageForm f = buildPageForm(tabs, doc->setup);
    int pi = f.preset->findText(doc->setup.sizeName);
    f.preset->blockSignals(true);
    f.preset->setCurrentIndex(pi > 0 ? pi : 0);
    f.preset->blockSignals(false);
    dlg.v->addWidget(tabs);
    if (!dlg.exec()) return;
    PageSetup ns = doc->setup;
    f.read(ns);
    applyPageSetup(ed, ns, f.preset->currentIndex() > 0 ? f.preset->currentText() : QStringLiteral("Custom"), QCoreApplication::translate("Dialogs", "Page Setup"));
}

QVector<QPair<QString, PageSetup>> customPageSizes()
{
    QVector<QPair<QString, PageSetup>> out;
    for (const QJsonValue &v : Settings::get().customPageSizes())
        out << qMakePair(v.toObject()["name"].toString(), PageSetup::fromJson(v.toObject()["setup"].toObject()));
    return out;
}

bool createPageSizeDialog(QWidget *p, Editor *ed, int editIndex, bool apply)
{
    QJsonArray sizes = Settings::get().customPageSizes();
    const bool editing = editIndex >= 0 && editIndex < sizes.size();
    PageSetup start = editing ? PageSetup::fromJson(sizes.at(editIndex).toObject()["setup"].toObject()) : ed->doc()->setup;
    Dlg dlg(p, editing ? QCoreApplication::translate("Dialogs", "Edit Page Size") : QCoreApplication::translate("Dialogs", "Create New Page Size"));
    auto *nameRow = new QFormLayout();
    auto *name = new QLineEdit(editing ? sizes.at(editIndex).toObject()["name"].toString() : QCoreApplication::translate("Dialogs", "Custom %1").arg(sizes.size() + 1), &dlg.d);
    name->setObjectName("pageSizeName");
    nameRow->addRow(QCoreApplication::translate("Dialogs", "Name:"), name);
    dlg.v->addLayout(nameRow);
    auto *tabs = new QTabWidget(&dlg.d);
    const PageForm f = buildPageForm(tabs, start);
    f.pageForm->setRowVisible(f.preset, false);
    dlg.v->addWidget(tabs);
    if (!dlg.exec()) return false;
    PageSetup ns = start;
    f.read(ns);
    const QString n = name->text().trimmed().isEmpty() ? QStringLiteral("Custom") : name->text().trimmed();
    const QJsonObject entry{{"name", n}, {"setup", ns.toJson()}};
    if (editing) sizes.replace(editIndex, entry);
    else sizes.append(entry);
    Settings::get().setCustomPageSizes(sizes);
    if (!editing && apply) applyPageSetup(ed, ns, n, QCoreApplication::translate("Dialogs", "Page Size"));
    return true;
}

void customPageSizesDialog(QWidget *p, Editor *ed)
{
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Custom Page Sizes"));
    auto *list = new QListWidget(&dlg.d);
    auto fill = [list] {
        list->clear();
        for (const auto &c : customPageSizes())
            list->addItem(QCoreApplication::translate("Dialogs", "%1 (%2 × %3)").arg(c.first, Settings::get().format(c.second.size.width()), Settings::get().format(c.second.size.height())));
        list->setCurrentRow(0);
    };
    fill();
    auto *row = new QHBoxLayout();
    auto *add = new QPushButton(QCoreApplication::translate("Dialogs", "New…"), &dlg.d), *edit = new QPushButton(QCoreApplication::translate("Dialogs", "Edit…"), &dlg.d),
         *del = new QPushButton(QCoreApplication::translate("Dialogs", "Delete"), &dlg.d), *use = new QPushButton(QCoreApplication::translate("Dialogs", "Use This Size"), &dlg.d);
    for (QPushButton *b : {add, edit, del, use}) row->addWidget(b);
    QObject::connect(add, &QPushButton::clicked, &dlg.d, [&] { if (createPageSizeDialog(&dlg.d, ed)) fill(); });
    QObject::connect(edit, &QPushButton::clicked, &dlg.d, [&] { if (list->currentRow() >= 0 && createPageSizeDialog(&dlg.d, ed, list->currentRow())) fill(); });
    QObject::connect(del, &QPushButton::clicked, &dlg.d, [&] {
        const int r = list->currentRow();
        QJsonArray sizes = Settings::get().customPageSizes();
        if (r < 0 || r >= sizes.size()) return;
        sizes.removeAt(r);
        Settings::get().setCustomPageSizes(sizes);
        fill();
    });
    QObject::connect(use, &QPushButton::clicked, &dlg.d, [&] {
        const auto all = customPageSizes();
        const int r = list->currentRow();
        if (r < 0 || r >= all.size()) return;
        applyPageSetup(ed, all[r].second, all[r].first, QCoreApplication::translate("Dialogs", "Page Size"));
        dlg.d.accept();
    });
    dlg.v->addWidget(list);
    dlg.v->addLayout(row);
    dlg.exec();
}

// ---------------- Grid and Baseline Guides ----------------
void gridGuidesDialog(QWidget *p, Editor *ed, int startTab)
{
    Document *doc = ed->doc();
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
    gf->addRow(QCoreApplication::translate("Dialogs", "Columns:"), cols);
    gf->addRow(QCoreApplication::translate("Dialogs", "Column spacing:"), cg);
    gf->addRow(QCoreApplication::translate("Dialogs", "Rows:"), rows);
    gf->addRow(QCoreApplication::translate("Dialogs", "Row spacing:"), rg);
    gf->addRow(center);
    tabs->addTab(g, QCoreApplication::translate("Dialogs", "Grid Guides"));
    auto *b = new QWidget();
    auto *bf = new QFormLayout(b);
    auto *spacing = points(mp->grid.baseline, b, 1, 200);
    auto *offset = points(mp->grid.baselineOffset, b, 0, 200);
    bf->addRow(QCoreApplication::translate("Dialogs", "Horizontal baseline spacing:"), spacing);
    bf->addRow(QCoreApplication::translate("Dialogs", "Offset:"), offset);
    tabs->addTab(b, QCoreApplication::translate("Dialogs", "Baseline Guides"));
    auto *mg = new QWidget();
    auto *mgf = new QFormLayout(mg);
    auto *twoPage = new QCheckBox(QCoreApplication::translate("Dialogs", "Two-page master"), mg);
    twoPage->setChecked(mp->twoPage);
    mgf->addRow(twoPage);
    const QMarginsF m = doc->setup.margins;
    auto *mt = measure(m.top(), mg), *mb = measure(m.bottom(), mg), *ml = measure(m.left(), mg), *mr = measure(m.right(), mg);
    mgf->addRow(QCoreApplication::translate("Dialogs", "Top:"), mt);
    mgf->addRow(QCoreApplication::translate("Dialogs", "Bottom:"), mb);
    mgf->addRow(QCoreApplication::translate("Dialogs", "Left:"), ml);
    mgf->addRow(QCoreApplication::translate("Dialogs", "Right:"), mr);
    // A two-page master's side guides are inside and outside.
    auto sideNames = [=](bool two) {
        if (auto *l = qobject_cast<QLabel *>(mgf->labelForField(ml))) l->setText(two ? QCoreApplication::translate("Dialogs", "Inside:") : QCoreApplication::translate("Dialogs", "Left:"));
        if (auto *l = qobject_cast<QLabel *>(mgf->labelForField(mr))) l->setText(two ? QCoreApplication::translate("Dialogs", "Outside:") : QCoreApplication::translate("Dialogs", "Right:"));
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

// ---------------- Ruler Guides ----------------
void rulerGuidesDialog(QWidget *p, Editor *ed)
{
    RulerGuides guides = ed->surface()->guides;
    const QSizeF page = ed->doc()->pageSize();
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Ruler Guides"));
    auto *row = new QHBoxLayout();
    row->setSpacing(18);
    // One side: its guides listed by position, with a field to add one, move
    // the chosen one or add a run of them at even spacing.
    auto side = [&](const QString &title, const QString &from, QVector<double> *list, double limit) {
        auto *box = new QGroupBox(title, &dlg.d);
        auto *v = new QVBoxLayout(box);
        auto *hint = new QLabel(from, box);
        hint->setEnabled(false);
        v->addWidget(hint);
        auto *items = new QListWidget(box);
        items->setMinimumHeight(150);
        items->setObjectName(QStringLiteral("guideList"));
        v->addWidget(items);
        auto refill = [=](int select) {
            std::sort(list->begin(), list->end());
            items->clear();
            for (double g : *list) items->addItem(Settings::get().format(g));
            if (select >= 0 && select < items->count()) items->setCurrentRow(select);
        };
        auto *pos = measure(limit / 2, box, 0, limit);
        auto *add = new QPushButton(QCoreApplication::translate("Dialogs", "Add"), box), *move = new QPushButton(QCoreApplication::translate("Dialogs", "Move"), box);
        auto *posRow = new QHBoxLayout();
        posRow->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "Position:"), box));
        posRow->addWidget(pos);
        posRow->addWidget(add);
        posRow->addWidget(move);
        posRow->addStretch(1);
        v->addLayout(posRow);
        auto *remove = new QPushButton(QCoreApplication::translate("Dialogs", "Remove"), box), *removeAll = new QPushButton(QCoreApplication::translate("Dialogs", "Remove All"), box);
        auto *rmRow = new QHBoxLayout();
        rmRow->addWidget(remove);
        rmRow->addWidget(removeAll);
        rmRow->addStretch(1);
        v->addLayout(rmRow);
        auto *series = new QGroupBox(QCoreApplication::translate("Dialogs", "Add a series"), box);
        auto *sf = new QFormLayout(series);
        auto *count = new QSpinBox(series);
        count->setRange(1, 200);
        count->setValue(4);
        count->setFixedWidth(110);
        auto *start = measure(0, series, 0, limit), *every = measure(72, series, 1, limit);
        auto *addSeries = new QPushButton(QCoreApplication::translate("Dialogs", "Add Series"), series);
        sf->addRow(QCoreApplication::translate("Dialogs", "Guides:"), count);
        sf->addRow(QCoreApplication::translate("Dialogs", "First at:"), start);
        sf->addRow(QCoreApplication::translate("Dialogs", "Spacing:"), every);
        sf->addRow(addSeries);
        v->addWidget(series);
        refill(-1);
        QObject::connect(items, &QListWidget::currentRowChanged, box, [=](int r) {
            if (r >= 0 && r < list->size()) pos->setValue(list->at(r));
            move->setEnabled(r >= 0);
            remove->setEnabled(r >= 0);
        });
        move->setEnabled(false);
        remove->setEnabled(false);
        QObject::connect(add, &QPushButton::clicked, box, [=] {
            const double x = pos->value();
            if (RulerGuides::addSeries(list, x, 0, 1, limit) == 0) return;   // one is already there
            std::sort(list->begin(), list->end());
            refill(int(std::lower_bound(list->begin(), list->end(), x - 0.01) - list->begin()));
        });
        QObject::connect(move, &QPushButton::clicked, box, [=] {
            const int r = items->currentRow();
            if (r < 0 || r >= list->size()) return;
            (*list)[r] = pos->value();
            std::sort(list->begin(), list->end());
            refill(int(std::lower_bound(list->begin(), list->end(), pos->value() - 0.01) - list->begin()));
        });
        QObject::connect(remove, &QPushButton::clicked, box, [=] {
            const int r = items->currentRow();
            if (r < 0 || r >= list->size()) return;
            list->remove(r);
            refill(std::min(r, int(list->size()) - 1));
        });
        QObject::connect(removeAll, &QPushButton::clicked, box, [=] {
            list->clear();
            refill(-1);
        });
        QObject::connect(addSeries, &QPushButton::clicked, box, [=] {
            RulerGuides::addSeries(list, start->value(), every->value(), count->value(), limit);
            refill(-1);
        });
        row->addWidget(box);
    };
    side(QCoreApplication::translate("Dialogs", "Horizontal Guides"), QCoreApplication::translate("Dialogs", "Measured from the top of the page"), &guides.h, page.height());
    side(QCoreApplication::translate("Dialogs", "Vertical Guides"), QCoreApplication::translate("Dialogs", "Measured from the left of the page"), &guides.v, page.width());
    dlg.v->addLayout(row);
    if (!dlg.exec()) return;
    ed->change(QCoreApplication::translate("Dialogs", "Ruler Guides"), [&] { ed->surface()->guides = guides; });
}

// ---------------- Drop Cap ----------------
void dropCapDialog(QWidget *p, Editor *ed)
{
    const QTextBlockFormat bf = ed->currentBlockFormat();
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Drop Cap"));
    auto *form = new QFormLayout();
    auto *pos = new QComboBox(&dlg.d);
    pos->addItems({QCoreApplication::translate("Dialogs", "Dropped"), QCoreApplication::translate("Dialogs", "Up")});
    pos->setCurrentIndex(bf.boolProperty(tp::DropCapUpper) ? 1 : 0);
    auto *lines = new QSpinBox(&dlg.d);
    lines->setRange(1, 20);
    lines->setValue(bf.intProperty(tp::DropCapLines) ? bf.intProperty(tp::DropCapLines) : 3);
    auto *chars = new QSpinBox(&dlg.d);
    chars->setRange(1, 15);
    chars->setValue(bf.intProperty(tp::DropCapChars) ? bf.intProperty(tp::DropCapChars) : 1);
    auto *font = new FontCombo(&dlg.d);
    font->setCurrentFamily(bf.stringProperty(tp::DropCapFont).isEmpty() ? QCoreApplication::translate("Dialogs", "(current font)") : bf.stringProperty(tp::DropCapFont));
    QString chosen = bf.stringProperty(tp::DropCapFont);
    QObject::connect(font, &FontCombo::familyChosen, &dlg.d, [&](const QString &f) { chosen = f.startsWith('+') ? QString() : f; });
    auto *color = colorPick(ed, bf.stringProperty(tp::DropCapColor).isEmpty() ? ColorRef::scheme(Accent1) : ColorRef::fromString(bf.stringProperty(tp::DropCapColor)), true, &dlg.d);
    form->addRow(QCoreApplication::translate("Dialogs", "Letter position:"), pos);
    form->addRow(QCoreApplication::translate("Dialogs", "Size of letters (lines high):"), lines);
    form->addRow(QCoreApplication::translate("Dialogs", "Number of letters:"), chars);
    form->addRow(QCoreApplication::translate("Dialogs", "Font:"), font);
    form->addRow(QCoreApplication::translate("Dialogs", "Color:"), color);
    dlg.v->addLayout(form);
    auto *remove = new QPushButton(QCoreApplication::translate("Dialogs", "Remove"), &dlg.d);
    bool removed = false;
    QObject::connect(remove, &QPushButton::clicked, &dlg.d, [&] { removed = true; dlg.d.accept(); });
    dlg.bb->addButton(remove, QDialogButtonBox::ResetRole);
    if (!dlg.exec()) return;
    if (removed) { ed->setDropCap(0); return; }
    QTextBlockFormat f;
    f.setProperty(tp::DropCapLines, lines->value());
    f.setProperty(tp::DropCapChars, chars->value());
    f.setProperty(tp::DropCapUpper, pos->currentIndex() == 1);
    f.setProperty(tp::DropCapFont, chosen);
    f.setProperty(tp::DropCapColor, color->current().isNone() ? QString() : color->current().toString());
    if (ed->isEditingText()) {
        ed->beginChange(QCoreApplication::translate("Dialogs", "Drop Cap"));
        ed->cursor().mergeBlockFormat(f);
        ed->endChange();
    } else {
        ed->mergeBlockFormat(f, QCoreApplication::translate("Dialogs", "Drop Cap"));
    }
}

// ---------------- Gradient text fill ----------------
void textGradientDialog(QWidget *p, Editor *ed)
{
    const QTextCharFormat cf = ed->currentCharFormat();
    Fill current = Fill::fromJson(QJsonDocument::fromJson(cf.stringProperty(tp::TextFill).toUtf8()).object());
    if (current.type != Fill::Gradient) current = Fill::gradient(ColorRef::scheme(Accent1), ColorRef::scheme(Accent2), 0);
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Gradient Text Fill"));
    auto *form = new QFormLayout();
    auto *from = colorPick(ed, current.color, false, &dlg.d);
    auto *to = colorPick(ed, current.color2, false, &dlg.d);
    auto *dir = new QComboBox(&dlg.d);
    dir->addItems({QCoreApplication::translate("Dialogs", "Left to right"), QCoreApplication::translate("Dialogs", "Top to bottom"), QCoreApplication::translate("Dialogs", "Diagonal down"), QCoreApplication::translate("Dialogs", "Diagonal up")});
    const double angles[] = {0, 90, 45, -45};
    for (int i = 0; i < 4; ++i)
        if (std::abs(current.angle - angles[i]) < 0.5) dir->setCurrentIndex(i);
    form->addRow(QCoreApplication::translate("Dialogs", "From:"), from);
    form->addRow(QCoreApplication::translate("Dialogs", "To:"), to);
    form->addRow(QCoreApplication::translate("Dialogs", "Direction:"), dir);
    dlg.v->addLayout(form);
    if (!dlg.exec()) return;
    const Fill g = Fill::gradient(from->current(), to->current(), angles[dir->currentIndex()]);
    QTextCharFormat f;
    f.setProperty(tp::TextFill, QString::fromUtf8(QJsonDocument(g.toJson()).toJson(QJsonDocument::Compact)));
    ed->mergeCharFormat(f, QCoreApplication::translate("Dialogs", "Text Fill"));
}

// ---------------- Format Object ----------------
void formatObjectDialog(QWidget *p, Editor *ed, int tab)
{
    Item *it = ed->isEditingText() ? ed->doc()->item(ed->textTarget().itemId) : ed->single();
    if (!it) {
        if (ed->selection().isEmpty()) return;
        it = ed->selectedItems().first();
    }
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Format %1").arg(itemTypeName(it->type())));
    auto *tabs = new QTabWidget(&dlg.d);
    // Colors and Lines.
    auto *cl = new QWidget();
    auto *clf = new QFormLayout(cl);
    Fill fill = it->fill;
    auto *fillColor = colorPick(ed, fill.type == Fill::Solid ? fill.color : ColorRef::none(), true, cl);
    auto *fillFx = new QPushButton(QCoreApplication::translate("Dialogs", "Fill Effects…"), cl);
    QObject::connect(fillFx, &QPushButton::clicked, cl, [&] { if (fillEffectsDialog(cl, ed, fill)) fillColor->setCurrent(fill.type == Fill::Solid ? fill.color : ColorRef::none()); });
    QObject::connect(fillColor, &ColorButton::colorPicked, cl, [&](const ColorRef &c) { fill = c.isNone() ? Fill() : Fill::solid(c, fill.transparency); });
    auto *fillRow = new QHBoxLayout();
    fillRow->addWidget(fillColor);
    fillRow->addWidget(fillFx);
    auto *trans = new QSlider(Qt::Horizontal, cl);
    trans->setRange(0, 100);
    trans->setValue(int(fill.transparency * 100));
    auto *lineColor = colorPick(ed, it->stroke.color, true, cl);
    auto *lineWidth = points(it->stroke.width, cl, 0, 200);
    auto *dash = new QComboBox(cl);
    for (int d = 0; d < 8; ++d) dash->addItem(dashName(Stroke::Dash(d)));
    dash->setCurrentIndex(int(it->stroke.dash));
    auto *compound = new QComboBox(cl);
    compound->addItems({QCoreApplication::translate("Dialogs", "Single"), QCoreApplication::translate("Dialogs", "Double"), QCoreApplication::translate("Dialogs", "Thick-thin"), QCoreApplication::translate("Dialogs", "Thin-thick"), QCoreApplication::translate("Dialogs", "Triple")});
    compound->setCurrentIndex(int(it->stroke.compound));
    auto *begin = new QComboBox(cl), *end = new QComboBox(cl);
    for (QComboBox *c : {begin, end}) c->addItems({QCoreApplication::translate("Dialogs", "None"), QCoreApplication::translate("Dialogs", "Arrow"), QCoreApplication::translate("Dialogs", "Open arrow"), QCoreApplication::translate("Dialogs", "Stealth arrow"), QCoreApplication::translate("Dialogs", "Diamond"), QCoreApplication::translate("Dialogs", "Oval")});
    begin->setCurrentIndex(int(it->stroke.startArrow));
    end->setCurrentIndex(int(it->stroke.endArrow));
    clf->addRow(QCoreApplication::translate("Dialogs", "Fill color:"), fillRow);
    clf->addRow(QCoreApplication::translate("Dialogs", "Transparency:"), trans);
    clf->addRow(QCoreApplication::translate("Dialogs", "Line color:"), lineColor);
    clf->addRow(QCoreApplication::translate("Dialogs", "Line weight:"), lineWidth);
    clf->addRow(QCoreApplication::translate("Dialogs", "Dash type:"), dash);
    clf->addRow(QCoreApplication::translate("Dialogs", "Compound type:"), compound);
    if (it->type() == ItemType::Line) {
        clf->addRow(QCoreApplication::translate("Dialogs", "Begin style:"), begin);
        clf->addRow(QCoreApplication::translate("Dialogs", "End style:"), end);
    }
    tabs->addTab(cl, QCoreApplication::translate("Dialogs", "Colors and Lines"));
    // Size.
    auto *sz = new QWidget();
    auto *szf = new QFormLayout(sz);
    auto *h = measure(it->rect.height(), sz, 1), *w = measure(it->rect.width(), sz, 1);
    auto *rot = new DecimalSpin(sz);
    rot->setRange(-360, 360);
    rot->setSuffix(QStringLiteral("°"));
    rot->setValue(it->rotation);
    auto *lockAspect = new QCheckBox(QCoreApplication::translate("Dialogs", "Lock aspect ratio"), sz);
    auto *table = dynamic_cast<TableItem *>(it);
    lockAspect->setObjectName(QStringLiteral("lockAspect"));
    lockAspect->setChecked(it->type() == ItemType::Picture || (table && table->lockSize));
    const double ratio = it->rect.width() / std::max(1.0, it->rect.height());
    QObject::connect(w, &QDoubleSpinBox::valueChanged, sz, [=](double v) { if (lockAspect->isChecked() && !h->hasFocus()) { const QSignalBlocker b(h); h->setValue(v / ratio); } });
    QObject::connect(h, &QDoubleSpinBox::valueChanged, sz, [=](double v) { if (lockAspect->isChecked() && !w->hasFocus()) { const QSignalBlocker b(w); w->setValue(v * ratio); } });
    szf->addRow(QCoreApplication::translate("Dialogs", "Height:"), h);
    szf->addRow(QCoreApplication::translate("Dialogs", "Width:"), w);
    szf->addRow(QCoreApplication::translate("Dialogs", "Rotation:"), rot);
    szf->addRow(lockAspect);
    tabs->addTab(sz, QCoreApplication::translate("Dialogs", "Size"));
    // Layout.
    auto *lay = new QWidget();
    auto *layf = new QFormLayout(lay);
    auto *x = measure(it->rect.x(), lay, -10000), *y = measure(it->rect.y(), lay, -10000);
    x->setMinimum(-10000);
    y->setMinimum(-10000);
    auto *wrap = new QComboBox(lay);
    wrap->addItems({QCoreApplication::translate("Dialogs", "None"), QCoreApplication::translate("Dialogs", "Square"), QCoreApplication::translate("Dialogs", "Tight"), QCoreApplication::translate("Dialogs", "Through"), QCoreApplication::translate("Dialogs", "Top and bottom")});
    wrap->setCurrentIndex(int(it->wrap.mode));
    auto *side = new QComboBox(lay);
    side->addItems({QCoreApplication::translate("Dialogs", "Both sides"), QCoreApplication::translate("Dialogs", "Left only"), QCoreApplication::translate("Dialogs", "Right only"), QCoreApplication::translate("Dialogs", "Largest side")});
    side->setCurrentIndex(int(it->wrap.side));
    auto *dt = measure(it->wrap.top, lay), *db = measure(it->wrap.bottom, lay), *dl = measure(it->wrap.left, lay), *dr = measure(it->wrap.right, lay);
    layf->addRow(QCoreApplication::translate("Dialogs", "Horizontal position:"), x);
    layf->addRow(QCoreApplication::translate("Dialogs", "Vertical position:"), y);
    layf->addRow(QCoreApplication::translate("Dialogs", "Wrapping style:"), wrap);
    layf->addRow(QCoreApplication::translate("Dialogs", "Wrap text:"), side);
    layf->addRow(QCoreApplication::translate("Dialogs", "Distance from text — top:"), dt);
    layf->addRow(QCoreApplication::translate("Dialogs", "bottom:"), db);
    layf->addRow(QCoreApplication::translate("Dialogs", "left:"), dl);
    layf->addRow(QCoreApplication::translate("Dialogs", "right:"), dr);
    tabs->addTab(lay, QCoreApplication::translate("Dialogs", "Layout"));
    // Text box.
    QMarginsF ins;
    int columns = 1;
    double colGap = 9;
    VAlign valign = VAlign::Top;
    int autofit = 0;
    bool contOn = false, contFrom = false;
    auto *tb = new QWidget();
    auto *tbf = new QFormLayout(tb);
    auto *tl = measure(0, tb), *tt = measure(0, tb), *tr = measure(0, tb), *tbm = measure(0, tb);
    auto *va = new QComboBox(tb);
    va->addItems({QCoreApplication::translate("Dialogs", "Top"), QCoreApplication::translate("Dialogs", "Middle"), QCoreApplication::translate("Dialogs", "Bottom")});
    auto *fit = new QComboBox(tb);
    fit->addItems({QCoreApplication::translate("Dialogs", "Do Not Autofit"), QCoreApplication::translate("Dialogs", "Best Fit"), QCoreApplication::translate("Dialogs", "Shrink Text On Overflow"), QCoreApplication::translate("Dialogs", "Grow Text Box to Fit")});
    auto *cols = new QSpinBox(tb);
    cols->setRange(1, 63);
    auto *gap = measure(9, tb);
    auto *cOn = new QCheckBox(QCoreApplication::translate("Dialogs", "Include \"Continued on page…\""), tb);
    auto *cFrom = new QCheckBox(QCoreApplication::translate("Dialogs", "Include \"Continued from page…\""), tb);
    const bool isText = it->type() == ItemType::Text, isShape = it->type() == ItemType::Shape;
    if (auto *t = dynamic_cast<TextItem *>(it)) {
        ins = t->insets; columns = t->columns; colGap = t->columnGap; valign = t->valign; autofit = int(t->autofit); contOn = t->continuedOn; contFrom = t->continuedFrom;
    } else if (auto *s = dynamic_cast<ShapeItem *>(it)) {
        ins = s->insets; valign = s->valign;
    }
    tl->setValue(ins.left()); tt->setValue(ins.top()); tr->setValue(ins.right()); tbm->setValue(ins.bottom());
    va->setCurrentIndex(int(valign));
    fit->setCurrentIndex(autofit);
    cols->setValue(columns);
    gap->setValue(colGap);
    cOn->setChecked(contOn);
    cFrom->setChecked(contFrom);
    tbf->addRow(QCoreApplication::translate("Dialogs", "Vertical alignment:"), va);
    if (isText) tbf->addRow(QCoreApplication::translate("Dialogs", "Text autofitting:"), fit);
    tbf->addRow(QCoreApplication::translate("Dialogs", "Left margin:"), tl);
    tbf->addRow(QCoreApplication::translate("Dialogs", "Right margin:"), tr);
    tbf->addRow(QCoreApplication::translate("Dialogs", "Top margin:"), tt);
    tbf->addRow(QCoreApplication::translate("Dialogs", "Bottom margin:"), tbm);
    if (isText) {
        tbf->addRow(QCoreApplication::translate("Dialogs", "Columns:"), cols);
        tbf->addRow(QCoreApplication::translate("Dialogs", "Column spacing:"), gap);
        tbf->addRow(cOn);
        tbf->addRow(cFrom);
    }
    if (isText || isShape) tabs->addTab(tb, QCoreApplication::translate("Dialogs", "Text Box"));
    // Cell Properties (tables): the vertical alignment and margins of the selected cells, the cell
    // the cursor is in, or every cell; and turning their text. Only what is changed is applied.
    QWidget *cp = nullptr;
    CellRange cells;
    auto *cellAlign = new QComboBox(&dlg.d);
    cellAlign->setObjectName(QStringLiteral("cellValign"));
    cellAlign->addItems({QCoreApplication::translate("Dialogs", "Top"), QCoreApplication::translate("Dialogs", "Middle"), QCoreApplication::translate("Dialogs", "Bottom")});
    auto cellMargin = [&](const char *name) {
        auto *m = measure(0, &dlg.d);
        m->setObjectName(QString::fromLatin1(name));
        return m;
    };
    auto *cellLeft = cellMargin("cellMarginLeft"), *cellRight = cellMargin("cellMarginRight"), *cellTop = cellMargin("cellMarginTop"), *cellBottom = cellMargin("cellMarginBottom");
    auto *cellRotate = new QCheckBox(QCoreApplication::translate("Dialogs", "Rotate text by 90°"), &dlg.d);
    cellRotate->setObjectName(QStringLiteral("cellRotate"));
    QMarginsF cellMargins0;
    int cellAlign0 = 0;
    bool cellRotate0 = false;
    if (table) {
        cells = ed->cellsToFormat(table);
        const TableCell &first = table->cell(cells.r0, cells.c0);
        cellMargins0 = first.margins;
        cellAlign0 = int(first.valign);
        cellRotate0 = first.vertical;
        cellAlign->setCurrentIndex(cellAlign0);
        cellLeft->setValue(cellMargins0.left());
        cellRight->setValue(cellMargins0.right());
        cellTop->setValue(cellMargins0.top());
        cellBottom->setValue(cellMargins0.bottom());
        cellRotate->setChecked(cellRotate0);
        cp = new QWidget();
        auto *cpf = new QFormLayout(cp);
        cpf->addRow(QCoreApplication::translate("Dialogs", "Vertical alignment:"), cellAlign);
        cpf->addRow(QCoreApplication::translate("Dialogs", "Left margin:"), cellLeft);
        cpf->addRow(QCoreApplication::translate("Dialogs", "Right margin:"), cellRight);
        cpf->addRow(QCoreApplication::translate("Dialogs", "Top margin:"), cellTop);
        cpf->addRow(QCoreApplication::translate("Dialogs", "Bottom margin:"), cellBottom);
        cpf->addRow(cellRotate);
        tabs->addTab(cp, QCoreApplication::translate("Dialogs", "Cell Properties"));
    }
    // Picture.
    auto *pic = dynamic_cast<PictureItem *>(it);
    auto *pw = new QWidget();
    auto *pf = new QFormLayout(pw);
    auto *bright = new QSlider(Qt::Horizontal, pw), *contrast = new QSlider(Qt::Horizontal, pw);
    bright->setRange(-100, 100);
    contrast->setRange(-100, 100);
    auto *recolor = new QComboBox(pw);
    recolor->addItems({QCoreApplication::translate("Dialogs", "None"), QCoreApplication::translate("Dialogs", "Grayscale"), QCoreApplication::translate("Dialogs", "Sepia"), QCoreApplication::translate("Dialogs", "Washout"), QCoreApplication::translate("Dialogs", "Black and white"), QCoreApplication::translate("Dialogs", "Tint")});
    auto *ptrans = new QSlider(Qt::Horizontal, pw);
    ptrans->setRange(0, 100);
    if (pic) {
        bright->setValue(int(pic->brightness));
        contrast->setValue(int(pic->contrast));
        recolor->setCurrentIndex(int(pic->recolor));
        ptrans->setValue(int(pic->transparency * 100));
        pf->addRow(QCoreApplication::translate("Dialogs", "Brightness:"), bright);
        pf->addRow(QCoreApplication::translate("Dialogs", "Contrast:"), contrast);
        pf->addRow(QCoreApplication::translate("Dialogs", "Color:"), recolor);
        pf->addRow(QCoreApplication::translate("Dialogs", "Transparency:"), ptrans);
        tabs->addTab(pw, QCoreApplication::translate("Dialogs", "Picture"));
    }
    // Web / Alt text.
    auto *wb = new QWidget();
    auto *wbf = new QFormLayout(wb);
    auto *alt = new QPlainTextEdit(it->altText, wb);
    auto *link = new QLineEdit(it->hyperlink, wb);
    wbf->addRow(QCoreApplication::translate("Dialogs", "Alternative text:"), alt);
    wbf->addRow(QCoreApplication::translate("Dialogs", "Hyperlink:"), link);
    tabs->addTab(wb, QCoreApplication::translate("Dialogs", "Alt Text"));
    // Map caller's tab index (0 colors, 1 size, 2 layout, 3 text box, 4 picture, 5 alt text, 6 cell properties).
    QWidget *want[] = {cl, sz, lay, tb, pw, wb, cp};
    const int idx = tabs->indexOf(want[std::clamp(tab, 0, 6)]);
    if (idx >= 0) tabs->setCurrentIndex(idx);
    dlg.v->addWidget(tabs);
    if (!dlg.exec()) return;
    ed->forEachSelected(QCoreApplication::translate("Dialogs", "Format Object"), [&](Item *o) {
        fill.transparency = trans->value() / 100.0;
        if (fill.type != Fill::NoFill || fillColor->current().isNone()) o->fill = fill;
        o->stroke.color = lineColor->current();
        o->stroke.width = lineWidth->value();
        o->stroke.dash = Stroke::Dash(dash->currentIndex());
        o->stroke.compound = Stroke::Compound(compound->currentIndex());
        o->stroke.startArrow = Arrow(begin->currentIndex());
        o->stroke.endArrow = Arrow(end->currentIndex());
        o->wrap.mode = Wrap::Mode(wrap->currentIndex());
        o->wrap.side = Wrap::Side(side->currentIndex());
        o->wrap.top = dt->value(); o->wrap.bottom = db->value(); o->wrap.left = dl->value(); o->wrap.right = dr->value();
        o->altText = alt->toPlainText();
        o->hyperlink = link->text();
        if (o == it && o->type() != ItemType::Line && o->type() != ItemType::Group) {
            const QPointF c0 = o->rect.center();
            Q_UNUSED(c0);
            const QRectF before = o->rect;
            const QRectF to(x->value(), y->value(), w->value(), h->value());
            if (to.size() != before.size()) o->scaleInto(before, QRectF(before.topLeft(), to.size()));
            o->rect.moveTopLeft(to.topLeft());
            o->rotation = std::fmod(rot->value() + 360, 360.0);
        }
        if (auto *t = dynamic_cast<TextItem *>(o)) {
            t->insets = QMarginsF(tl->value(), tt->value(), tr->value(), tbm->value());
            t->valign = VAlign(va->currentIndex());
            t->autofit = TextItem::Autofit(fit->currentIndex());
            t->columns = cols->value();
            t->columnGap = gap->value();
            t->continuedOn = cOn->isChecked();
            t->continuedFrom = cFrom->isChecked();
            ed->autoGrowText(t);
        }
        if (auto *s = dynamic_cast<ShapeItem *>(o)) {
            s->insets = QMarginsF(tl->value(), tt->value(), tr->value(), tbm->value());
            s->valign = VAlign(va->currentIndex());
        }
        if (auto *tbl = dynamic_cast<TableItem *>(o); tbl && o == it) {
            tbl->lockSize = lockAspect->isChecked();
            for (int r = cells.r0; r <= cells.r1; ++r)
                for (int c = cells.c0; c <= cells.c1; ++c) {
                    TableCell &cell = tbl->cell(r, c);
                    if (cellAlign->currentIndex() != cellAlign0) cell.valign = VAlign(cellAlign->currentIndex());
                    if (cellLeft->value() != cellMargins0.left()) cell.margins.setLeft(cellLeft->value());
                    if (cellRight->value() != cellMargins0.right()) cell.margins.setRight(cellRight->value());
                    if (cellTop->value() != cellMargins0.top()) cell.margins.setTop(cellTop->value());
                    if (cellBottom->value() != cellMargins0.bottom()) cell.margins.setBottom(cellBottom->value());
                    if (cellRotate->isChecked() != cellRotate0) cell.vertical = cellRotate->isChecked();
                }
            ed->fitTableRows(tbl);   // the size, margins, and turned text change what fits
        }
        if (auto *pp = dynamic_cast<PictureItem *>(o)) {
            pp->brightness = bright->value();
            pp->contrast = contrast->value();
            pp->recolor = PictureItem::Recolor(recolor->currentIndex());
            pp->transparency = ptrans->value() / 100.0;
            if (pp->recolor == PictureItem::ColorTint && pp->recolorColor.isNone()) pp->recolorColor = ColorRef::scheme(Accent1);
        }
    });
}

// ---------------- Change Template and Insert Page ----------------
int changeTemplateDialog(QWidget *p)
{
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Change Template"));
    dlg.v->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "Where do you want the new design?"), &dlg.d));
    auto *apply = new QRadioButton(QCoreApplication::translate("Dialogs", "Apply template to the current publication"), &dlg.d);
    auto *another = new QRadioButton(QCoreApplication::translate("Dialogs", "Create a new publication with my text and graphics"), &dlg.d);
    apply->setChecked(true);
    dlg.v->addWidget(apply);
    dlg.v->addWidget(another);
    if (!dlg.exec()) return 0;
    return another->isChecked() ? 2 : 1;
}

void insertPageDialog(QWidget *p, Editor *ed)
{
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Insert Page"));
    auto *form = new QFormLayout();
    auto *count = new QSpinBox(&dlg.d);
    count->setRange(1, 999);
    auto *before = new QRadioButton(QCoreApplication::translate("Dialogs", "Before current page"), &dlg.d), *after = new QRadioButton(QCoreApplication::translate("Dialogs", "After current page"), &dlg.d);
    after->setChecked(true);
    auto *blank = new QRadioButton(QCoreApplication::translate("Dialogs", "Insert blank pages"), &dlg.d), *textbox = new QRadioButton(QCoreApplication::translate("Dialogs", "Create one text box on each page"), &dlg.d),
         *dup = new QRadioButton(QCoreApplication::translate("Dialogs", "Duplicate all objects on page:"), &dlg.d);
    blank->setChecked(true);
    auto *srcPage = new QSpinBox(&dlg.d);
    srcPage->setRange(1, ed->doc()->pages.size());
    srcPage->setValue(ed->currentPage() + 1);
    form->addRow(QCoreApplication::translate("Dialogs", "Number of new pages:"), count);
    form->addRow(before);
    form->addRow(after);
    form->addRow(blank);
    form->addRow(textbox);
    auto *dupRow = new QHBoxLayout();
    dupRow->addWidget(dup);
    dupRow->addWidget(srcPage);
    form->addRow(dupRow);
    auto *master = new QComboBox(&dlg.d);
    for (const auto &m : ed->doc()->masters) master->addItem(QStringLiteral("(%1) %2").arg(m->abbr, m->name), m->id);
    form->addRow(QCoreApplication::translate("Dialogs", "Master page:"), master);
    dlg.v->addLayout(form);
    if (!dlg.exec()) return;
    int at = before->isChecked() ? ed->currentPage() - 1 : ed->currentPage();
    if (dup->isChecked()) {
        const int src = srcPage->value() - 1;
        ed->beginChange(QCoreApplication::translate("Dialogs", "Insert Page"));
        for (int i = 0; i < count->value(); ++i) {
            auto pg = ed->doc()->addPage(at + 1 + i, master->currentData().toString());
            for (const auto &it : ed->doc()->cloneItems(ed->doc()->pages[src < at + 1 + i ? src : src + 1]->items)) pg->items.push_back(it);
        }
        ed->endChange();
        ed->setCurrentPage(at + 1);
        return;
    }
    ed->insertPages(at, count->value(), false, textbox->isChecked(), master->currentData().toString());
}

// ---------------- Bookmark ----------------
void bookmarkDialog(QWidget *p, Editor *ed)
{
    bool ok = false;
    const QString name = QInputDialog::getText(p, QCoreApplication::translate("Dialogs", "Bookmark"), QCoreApplication::translate("Dialogs", "Bookmark name:"), QLineEdit::Normal, QString(), &ok);
    if (!ok || name.trimmed().isEmpty()) return;
    if (ed->isEditingText()) ed->setCharProperty(QTextFormat::AnchorName, QStringList{name.trimmed()}, QCoreApplication::translate("Dialogs", "Bookmark"));
    else ed->forEachSelected(QCoreApplication::translate("Dialogs", "Bookmark"), [&](Item *it) { it->name = "bookmark:" + name.trimmed(); });
}

// ---------------- Business Information ----------------
void businessInfoDialog(QWidget *p, Editor *ed)
{
    Document *doc = ed->doc();
    QVector<BusinessInfo> sets = doc->biz;
    int current = doc->bizCurrent;
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Business Information"));
    auto *top = new QHBoxLayout();
    auto *setBox = new QComboBox(&dlg.d);
    for (const auto &b : sets) setBox->addItem(b.setName);
    setBox->setCurrentIndex(current);
    auto *newSet = new QPushButton(QCoreApplication::translate("Dialogs", "New…"), &dlg.d);
    auto *delSet = new QPushButton(QCoreApplication::translate("Dialogs", "Delete"), &dlg.d);
    top->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "Select a Business Information set:")));
    top->addWidget(setBox, 1);
    top->addWidget(newSet);
    top->addWidget(delSet);
    dlg.v->addLayout(top);
    auto *form = new QFormLayout();
    QHash<QString, QWidget *> edits;
    for (const QString &k : BusinessInfo::keys()) {
        QWidget *w;
        if (k == "address") { auto *pe = new QPlainTextEdit(&dlg.d); pe->setFixedHeight(60); w = pe; }
        else w = new QLineEdit(&dlg.d);
        edits[k] = w;
        form->addRow(QCoreApplication::translate("Dialogs", "%1:").arg(BusinessInfo::label(k)), w);
    }
    auto *setName = new QLineEdit(&dlg.d);
    form->addRow(QCoreApplication::translate("Dialogs", "Business Information set name:"), setName);
    auto *logo = new QLabel(&dlg.d);
    logo->setFixedSize(96, 64);
    logo->setFrameShape(QFrame::StyledPanel);
    logo->setAlignment(Qt::AlignCenter);
    auto *addLogo = new QPushButton(QCoreApplication::translate("Dialogs", "Add Logo…"), &dlg.d);
    auto *removeLogo = new QPushButton(QCoreApplication::translate("Dialogs", "Remove Logo"), &dlg.d);
    auto *logoRow = new QHBoxLayout();
    logoRow->addWidget(logo);
    logoRow->addWidget(addLogo);
    logoRow->addWidget(removeLogo);
    form->addRow(QCoreApplication::translate("Dialogs", "Logo:"), logoRow);
    dlg.v->addLayout(form);
    auto load = [&](int i) {
        const BusinessInfo &b = sets[i];
        for (const QString &k : BusinessInfo::keys()) {
            if (auto *le = qobject_cast<QLineEdit *>(edits[k])) le->setText(b.field(k));
            else static_cast<QPlainTextEdit *>(edits[k])->setPlainText(b.field(k));
        }
        setName->setText(b.setName);
        const QImage img = b.logoImageId.isEmpty() ? QImage() : doc->image(b.logoImageId);
        logo->setPixmap(img.isNull() ? QPixmap() : QPixmap::fromImage(img.scaled(90, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    };
    auto store = [&](int i) {
        BusinessInfo &b = sets[i];
        for (const QString &k : BusinessInfo::keys()) {
            if (auto *le = qobject_cast<QLineEdit *>(edits[k])) b.setField(k, le->text());
            else b.setField(k, static_cast<QPlainTextEdit *>(edits[k])->toPlainText());
        }
        b.setName = setName->text().trimmed().isEmpty() ? b.setName : setName->text().trimmed();
    };
    load(current);
    QObject::connect(setBox, &QComboBox::currentIndexChanged, &dlg.d, [&](int i) {
        if (i < 0) return;
        store(current);
        current = i;
        load(i);
    });
    QObject::connect(newSet, &QPushButton::clicked, &dlg.d, [&] {
        store(current);
        BusinessInfo b;
        b.setName = QCoreApplication::translate("Dialogs", "Business Information %1").arg(sets.size() + 1);
        sets << b;
        setBox->addItem(b.setName);
        setBox->setCurrentIndex(sets.size() - 1);
    });
    QObject::connect(delSet, &QPushButton::clicked, &dlg.d, [&] {
        if (sets.size() <= 1) return;
        sets.removeAt(current);
        const QSignalBlocker blk(setBox);
        setBox->removeItem(current);
        current = std::min(current, int(sets.size()) - 1);
        setBox->setCurrentIndex(current);
        load(current);
    });
    QString pendingLogo;
    QObject::connect(addLogo, &QPushButton::clicked, &dlg.d, [&] {
        const QString f = QFileDialog::getOpenFileName(&dlg.d, QCoreApplication::translate("Dialogs", "Logo"), QString(), QCoreApplication::translate("Dialogs", "Pictures (*.png *.jpg *.jpeg *.gif *.bmp *.svg *.wmf *.emf)"));
        if (f.isEmpty()) return;
        QFile file(f);
        if (!file.open(QIODevice::ReadOnly)) return;
        sets[current].logoImageId = doc->addImage(file.readAll(), QFileInfo(f).suffix().toLower(), f);
        load(current);
    });
    QObject::connect(removeLogo, &QPushButton::clicked, &dlg.d, [&] { sets[current].logoImageId.clear(); load(current); });
    if (!dlg.exec()) return;
    store(current);
    ed->change(QCoreApplication::translate("Dialogs", "Business Information"), [&] {
        doc->biz = sets;
        doc->bizCurrent = current;
    });
    Q_UNUSED(pendingLogo);
}

// ---------------- Schemes ----------------
void colorSchemeDialog(QWidget *p, Editor *ed)
{
    ColorScheme s = ed->doc()->colors;
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Create New Color Scheme"));
    auto *form = new QFormLayout();
    QVector<QPushButton *> btns;
    for (int i = 0; i < SlotCount; ++i) {
        auto *b = new QPushButton(&dlg.d);
        b->setFixedSize(80, 26);
        auto paint = [b, &s, i] { b->setStyleSheet(QStringLiteral("background:%1; border:1px solid #888;").arg(s.c[i].name())); };
        paint();
        QObject::connect(b, &QPushButton::clicked, &dlg.d, [b, &s, i, paint] {
            const QColor c = colorsDialog(b, s.c[i], slotName(i));
            if (c.isValid()) { s.c[i] = c; paint(); }
        });
        form->addRow(QCoreApplication::translate("Dialogs", "%1:").arg(slotName(i)), b);
        btns << b;
    }
    auto *name = new QLineEdit(QCoreApplication::translate("Dialogs", "Custom 1"), &dlg.d);
    form->addRow(QCoreApplication::translate("Dialogs", "Color scheme name:"), name);
    dlg.v->addLayout(form);
    if (!dlg.exec()) return;
    s.name = name->text().trimmed().isEmpty() ? QStringLiteral("Custom") : name->text().trimmed();
    ed->change(QCoreApplication::translate("Dialogs", "Color Scheme"), [&] { ed->doc()->colors = s; });
}

void fontSchemeDialog(QWidget *p, Editor *ed)
{
    FontScheme s = ed->doc()->fonts;
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Create New Font Scheme"));
    auto *form = new QFormLayout();
    auto *heading = new QComboBox(&dlg.d), *body = new QComboBox(&dlg.d);
    heading->addItems(QFontDatabase::families());
    body->addItems(QFontDatabase::families());
    heading->setCurrentText(s.heading);
    body->setCurrentText(s.body);
    auto *name = new QLineEdit(QCoreApplication::translate("Dialogs", "Custom 1"), &dlg.d);
    form->addRow(QCoreApplication::translate("Dialogs", "Heading font:"), heading);
    form->addRow(QCoreApplication::translate("Dialogs", "Body font:"), body);
    form->addRow(QCoreApplication::translate("Dialogs", "Font scheme name:"), name);
    auto *sample = new QLabel(&dlg.d);
    auto upd = [&] {
        sample->setText(QStringLiteral("<span style=\"font-family:'%1'; font-size:18pt\">%3</span><br><span style=\"font-family:'%2'; font-size:10pt\">%4</span>")
                            .arg(heading->currentText(), body->currentText(), QCoreApplication::translate("Dialogs", "Heading").toHtmlEscaped(),
                                 QCoreApplication::translate("Dialogs", "Body text, lorem-free: the quick brown fox jumps over the lazy dog.").toHtmlEscaped()));
    };
    QObject::connect(heading, &QComboBox::currentTextChanged, &dlg.d, upd);
    QObject::connect(body, &QComboBox::currentTextChanged, &dlg.d, upd);
    upd();
    dlg.v->addLayout(form);
    dlg.v->addWidget(sample);
    if (!dlg.exec()) return;
    s = FontScheme{name->text().trimmed().isEmpty() ? QStringLiteral("Custom") : name->text().trimmed(), heading->currentText(), body->currentText()};
    ed->change(QCoreApplication::translate("Dialogs", "Font Scheme"), [&] { ed->doc()->fonts = s; });
}

void shadowDialog(QWidget *p, Editor *ed)
{
    Item *it = ed->single();
    if (!it) return;
    ShadowFx s = it->fx.shadow;
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Shadow"));
    auto *form = new QFormLayout();
    auto *on = new QCheckBox(QCoreApplication::translate("Dialogs", "Shadow"), &dlg.d);
    on->setChecked(s.on);
    auto *color = colorPick(ed, s.color, false, &dlg.d);
    auto *trans = new QSlider(Qt::Horizontal, &dlg.d);
    trans->setRange(0, 100);
    trans->setValue(int(s.transparency * 100));
    auto *blur = points(s.blur, &dlg.d, 0, 100);
    auto *dist = points(s.distance, &dlg.d, 0, 200);
    auto *angle = new QSpinBox(&dlg.d);
    angle->setRange(0, 359);
    angle->setSuffix(QStringLiteral("°"));
    angle->setValue(int(s.angle));
    form->addRow(on);
    form->addRow(QCoreApplication::translate("Dialogs", "Color:"), color);
    form->addRow(QCoreApplication::translate("Dialogs", "Transparency:"), trans);
    form->addRow(QCoreApplication::translate("Dialogs", "Blur:"), blur);
    form->addRow(QCoreApplication::translate("Dialogs", "Distance:"), dist);
    form->addRow(QCoreApplication::translate("Dialogs", "Angle:"), angle);
    dlg.v->addLayout(form);
    if (!dlg.exec()) return;
    ed->forEachSelected(QCoreApplication::translate("Dialogs", "Shadow"), [&](Item *x) {
        x->fx.shadow.on = on->isChecked();
        x->fx.shadow.color = color->current();
        x->fx.shadow.transparency = trans->value() / 100.0;
        x->fx.shadow.blur = blur->value();
        x->fx.shadow.distance = dist->value();
        x->fx.shadow.angle = angle->value();
    });
}

// ---------------- Calendar ----------------
void calendarDialog(QWidget *p, Editor *ed)
{
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Calendar"));
    auto *form = new QFormLayout();
    auto *month = new QComboBox(&dlg.d);
    for (int m = 1; m <= 12; ++m) month->addItem(QLocale().monthName(m));
    month->setCurrentIndex(QDate::currentDate().month() - 1);
    auto *year = new QSpinBox(&dlg.d);
    year->setRange(1900, 2200);
    year->setValue(QDate::currentDate().year());
    auto *style = new QComboBox(&dlg.d);
    style->addItems({QCoreApplication::translate("Dialogs", "Classic grid"), QCoreApplication::translate("Dialogs", "Banded"), QCoreApplication::translate("Dialogs", "Minimal"), QCoreApplication::translate("Dialogs", "Bold header"), QCoreApplication::translate("Dialogs", "One per page (12 months)")});
    form->addRow(QCoreApplication::translate("Dialogs", "Month:"), month);
    form->addRow(QCoreApplication::translate("Dialogs", "Year:"), year);
    form->addRow(QCoreApplication::translate("Dialogs", "Design:"), style);
    dlg.v->addLayout(form);
    if (!dlg.exec()) return;
    const QRectF content = QRectF(QPointF(0, 0), ed->doc()->pageSize()).marginsRemoved(ed->doc()->setup.margins);
    ed->beginChange(QCoreApplication::translate("Dialogs", "Insert Calendar"));
    QStringList ids;
    if (style->currentIndex() == 4) {
        for (int m = 1; m <= 12; ++m) {
            if (m > 1) ed->doc()->addPage(ed->currentPage() + m - 1, ed->doc()->pages[ed->currentPage()]->masterId);
            ItemList items = makeCalendar(*ed->doc(), content, year->value(), m, 1);
            for (auto &it : items) ed->doc()->pages[ed->currentPage() + m - 1]->items.push_back(it);
        }
    } else {
        ItemList items = makeCalendar(*ed->doc(), QRectF(content.left(), content.top() + content.height() * 0.35, content.width(), content.height() * 0.6),
                                      year->value(), month->currentIndex() + 1, style->currentIndex());
        for (auto &it : items) { ed->surfaceItems().push_back(it); ids << it->id; }
    }
    ed->endChange();
    ed->select(ids);
}

// ---------------- Styles ----------------
void styleDialog(QWidget *p, Editor *ed, const QString &styleName)
{
    Document *doc = ed->doc();
    const TextStyle *existing = styleName.isEmpty() ? nullptr : doc->style(styleName);
    TextStyle s = existing ? *existing : TextStyle();
    if (!existing) {
        s.name = QCoreApplication::translate("Dialogs", "New Style");
        s.basedOn = QStringLiteral("Normal");
        s.chr = ed->currentCharFormat();
        s.blk = ed->currentBlockFormat();
    }
    Dlg dlg(p, existing ? QCoreApplication::translate("Dialogs", "Modify Style") : QCoreApplication::translate("Dialogs", "New Style"));
    auto *form = new QFormLayout();
    auto *name = new QLineEdit(s.name, &dlg.d);
    auto *basedOn = new QComboBox(&dlg.d), *next = new QComboBox(&dlg.d);
    basedOn->addItem(QCoreApplication::translate("Dialogs", "(none)"));
    for (const auto &st : doc->styles) { basedOn->addItem(st.name); next->addItem(st.name); }
    basedOn->setCurrentText(s.basedOn.isEmpty() ? QCoreApplication::translate("Dialogs", "(none)") : s.basedOn);
    next->setCurrentText(s.next.isEmpty() ? s.name : s.next);
    auto *charOnly = new QCheckBox(QCoreApplication::translate("Dialogs", "Character style (applies to selected text only)"), &dlg.d);
    charOnly->setChecked(s.charOnly);
    auto *font = new QComboBox(&dlg.d);
    font->addItem(QCoreApplication::translate("Dialogs", "(scheme font)"));
    font->addItems(QFontDatabase::families());
    if (s.chr.hasProperty(QTextFormat::FontFamilies)) font->setCurrentText(s.chr.fontFamilies().toStringList().value(0));
    auto *size = new DecimalSpin(&dlg.d);
    size->setRange(1, 999);
    size->setValue(s.chr.hasProperty(QTextFormat::FontPointSize) ? s.chr.fontPointSize() : 11);
    auto *bold = new QCheckBox(QCoreApplication::translate("Dialogs", "Bold"), &dlg.d), *italic = new QCheckBox(QCoreApplication::translate("Dialogs", "Italic"), &dlg.d);
    bold->setChecked(s.chr.fontWeight() >= QFont::DemiBold);
    italic->setChecked(s.chr.fontItalic());
    auto *color = colorPick(ed, s.chr.stringProperty(tp::ColorRefP).isEmpty() ? ColorRef::scheme(Main) : ColorRef::fromString(s.chr.stringProperty(tp::ColorRefP)), false, &dlg.d);
    auto *before = points(s.blk.topMargin(), &dlg.d), *after = points(s.blk.bottomMargin(), &dlg.d);
    auto *align = new QComboBox(&dlg.d);
    align->addItems({QCoreApplication::translate("Dialogs", "Left"), QCoreApplication::translate("Dialogs", "Center"), QCoreApplication::translate("Dialogs", "Right"), QCoreApplication::translate("Dialogs", "Justify")});
    const Qt::Alignment al = s.blk.alignment() & Qt::AlignHorizontal_Mask;
    align->setCurrentIndex(al == Qt::AlignHCenter ? 1 : al == Qt::AlignRight ? 2 : al == Qt::AlignJustify ? 3 : 0);
    // Choosing a style to base this one on starts it with that style's look.
    QObject::connect(basedOn, &QComboBox::currentIndexChanged, &dlg.d, [&, doc](int i) {
        if (i <= 0) return;
        const TextStyle b = doc->resolvedStyle(basedOn->currentText());
        if (b.chr.hasProperty(QTextFormat::FontFamilies)) font->setCurrentText(b.chr.fontFamilies().toStringList().value(0));
        else font->setCurrentIndex(0);
        if (b.chr.hasProperty(QTextFormat::FontPointSize)) size->setValue(b.chr.fontPointSize());
        bold->setChecked(b.chr.fontWeight() >= QFont::DemiBold);
        italic->setChecked(b.chr.fontItalic());
        before->setValue(b.blk.topMargin());
        after->setValue(b.blk.bottomMargin());
        const Qt::Alignment ba = b.blk.alignment() & Qt::AlignHorizontal_Mask;
        align->setCurrentIndex(ba == Qt::AlignHCenter ? 1 : ba == Qt::AlignRight ? 2 : ba == Qt::AlignJustify ? 3 : 0);
    });
    form->addRow(QCoreApplication::translate("Dialogs", "Style name:"), name);
    form->addRow(QCoreApplication::translate("Dialogs", "Based on:"), basedOn);
    form->addRow(QCoreApplication::translate("Dialogs", "Style for the next paragraph:"), next);
    form->addRow(charOnly);
    form->addRow(QCoreApplication::translate("Dialogs", "Font:"), font);
    form->addRow(QCoreApplication::translate("Dialogs", "Size:"), size);
    auto *bi = new QHBoxLayout();
    bi->addWidget(bold);
    bi->addWidget(italic);
    form->addRow(bi);
    form->addRow(QCoreApplication::translate("Dialogs", "Color:"), color);
    form->addRow(QCoreApplication::translate("Dialogs", "Alignment:"), align);
    form->addRow(QCoreApplication::translate("Dialogs", "Space before:"), before);
    form->addRow(QCoreApplication::translate("Dialogs", "Space after:"), after);
    dlg.v->addLayout(form);
    if (!dlg.exec()) return;
    const QString newName = name->text().trimmed().isEmpty() ? s.name : name->text().trimmed();
    s.name = newName;
    s.basedOn = basedOn->currentIndex() == 0 ? QString() : basedOn->currentText();
    s.next = next->currentText();
    s.charOnly = charOnly->isChecked();
    if (font->currentIndex() > 0) s.chr.setFontFamilies(QStringList{font->currentText()});
    else s.chr.clearProperty(QTextFormat::FontFamilies);
    s.chr.setFontPointSize(size->value());
    s.chr.setFontWeight(bold->isChecked() ? QFont::Bold : QFont::Normal);
    s.chr.setFontItalic(italic->isChecked());
    s.chr.setProperty(tp::ColorRefP, color->current().toString());
    const Qt::Alignment als[] = {Qt::AlignLeft, Qt::AlignHCenter, Qt::AlignRight, Qt::AlignJustify};
    s.blk.setAlignment(als[align->currentIndex()]);
    s.blk.setTopMargin(before->value());
    s.blk.setBottomMargin(after->value());
    s.blk.setProperty(tp::StyleName, s.name);
    ed->change(existing ? QCoreApplication::translate("Dialogs", "Modify Style") : QCoreApplication::translate("Dialogs", "New Style"), [&] {
        bool replaced = false;
        for (auto &st : doc->styles)
            if (st.name == styleName && existing) { st = s; replaced = true; }
        if (!replaced) doc->styles << s;
        // Restyle paragraphs that use this style.
        if (existing) {
            for (auto it = doc->stories.begin(); it != doc->stories.end(); ++it) {
                QTextDocument *d = (*it)->doc.get();
                for (QTextBlock b = d->begin(); b.isValid(); b = b.next()) {
                    if (b.blockFormat().stringProperty(tp::StyleName) != styleName) continue;
                    QTextCursor c(b);
                    QTextBlockFormat bf = b.blockFormat();
                    tp::setProperties(bf, s.blk);
                    c.setBlockFormat(bf);
                    c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                    c.mergeCharFormat(s.chr);
                }
            }
        }
    });
}

// ---------------- Options ----------------
// The General and Proofing pages are the shared dialog's (optionsdialog.cpp);
// Save and Advanced are JeffPub's own.
void optionsDialog(QWidget *p, Editor *ed)
{
    Settings &st = Settings::get();
    struct Fields {
        QSpinBox *recover = nullptr;
        QCheckBox *backup = nullptr;
        QComboBox *units = nullptr;
        MeasureSpin *nudge = nullptr;
        QSpinBox *recent = nullptr;
        QCheckBox *wholeWord = nullptr, *dragText = nullptr, *hyphenate = nullptr;
    };
    auto f = std::make_shared<Fields>();
    QVector<OptionsPage> pages;
    pages << OptionsPage{QCoreApplication::translate("Dialogs", "Save"), [&st, f] {
        auto *save = new QWidget();
        auto *sf = new QFormLayout(save);
        auto *recover = new QSpinBox(save);
        recover->setObjectName(QStringLiteral("autoRecoverMinutes"));
        recover->setRange(1, 120);
        recover->setValue(st.autoRecoverMinutes());
        recover->setSuffix(QCoreApplication::translate("Dialogs", " minutes"));
        auto *backup = new QCheckBox(QCoreApplication::translate("Dialogs", "Always create backup copy"), save);
        backup->setChecked(st.value("save/backup", false).toBool());
        sf->addRow(QCoreApplication::translate("Dialogs", "Save AutoRecover information every:"), recover);
        sf->addRow(backup);
        f->recover = recover;
        f->backup = backup;
        return save;
    }, [&st, f] {
        st.setValue("save/autoRecoverMinutes", f->recover->value());
        st.setValue("save/backup", f->backup->isChecked());
    }};
    pages << OptionsPage{QCoreApplication::translate("Dialogs", "Advanced"), [&st, f] {
        auto *adv = new QWidget();
        auto *af = new QFormLayout(adv);
        auto *units = new QComboBox(adv);
        units->addItems({QCoreApplication::translate("Dialogs", "Inches"), QCoreApplication::translate("Dialogs", "Centimeters"), QCoreApplication::translate("Dialogs", "Millimeters"), QCoreApplication::translate("Dialogs", "Points"), QCoreApplication::translate("Dialogs", "Picas")});
        units->setCurrentIndex(int(st.unit()));
        auto *nudge = measure(st.nudge(), adv, 0.1, 720);
        auto *recent = new QSpinBox(adv);
        recent->setRange(0, 50);
        recent->setValue(st.value("recentCount", 25).toInt());
        auto *wholeWord = new QCheckBox(QCoreApplication::translate("Dialogs", "When selecting, automatically select entire word"), adv);
        wholeWord->setChecked(st.value("edit/wholeWord", true).toBool());
        auto *dragText = new QCheckBox(QCoreApplication::translate("Dialogs", "Allow text to be dragged and dropped"), adv);
        dragText->setChecked(st.value("edit/dragText", true).toBool());
        auto *hyphenate = new QCheckBox(QCoreApplication::translate("Dialogs", "Automatically hyphenate in new text boxes"), adv);
        hyphenate->setChecked(st.value("edit/hyphenate", true).toBool());
        af->addRow(QCoreApplication::translate("Dialogs", "Measurement units:"), units);
        af->addRow(QCoreApplication::translate("Dialogs", "Arrow keys nudge objects by:"), nudge);
        af->addRow(QCoreApplication::translate("Dialogs", "Show this number of Recent Publications:"), recent);
        af->addRow(wholeWord);
        af->addRow(dragText);
        af->addRow(hyphenate);
        f->units = units;
        f->nudge = nudge;
        f->recent = recent;
        f->wholeWord = wholeWord;
        f->dragText = dragText;
        f->hyphenate = hyphenate;
        return adv;
    }, [&st, f] {
        st.setUnit(Unit(f->units->currentIndex()));
        st.setValue("edit/nudge", f->nudge->value());
        st.setValue("recentCount", f->recent->value());
        st.setValue("edit/wholeWord", f->wholeWord->isChecked());
        st.setValue("edit/dragText", f->dragText->isChecked());
        st.setValue("edit/hyphenate", f->hyphenate->isChecked());
    }};
    if (!optionsDialog(p, ed, pages)) return;
    // Every open window saves AutoRecover copies at the new interval.
    for (QWidget *w : QApplication::topLevelWidgets())
        if (auto *mw = qobject_cast<MainWindow *>(w)) mw->settingsChanged();
}

// ---------------- Page numbers ----------------
void pageNumberDialog(QWidget *p, Editor *ed)
{
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Page Number"));
    auto *form = new QFormLayout();
    auto *pos = new QComboBox(&dlg.d);
    pos->addItems({QCoreApplication::translate("Dialogs", "Top left"), QCoreApplication::translate("Dialogs", "Top center"), QCoreApplication::translate("Dialogs", "Top right"), QCoreApplication::translate("Dialogs", "Bottom left"), QCoreApplication::translate("Dialogs", "Bottom center"), QCoreApplication::translate("Dialogs", "Bottom right")});
    pos->setCurrentIndex(4);
    auto *format = new QComboBox(&dlg.d);
    format->addItems({"1, 2, 3", "i, ii, iii", "I, II, III", "a, b, c"});
    auto *first = new QCheckBox(QCoreApplication::translate("Dialogs", "Show page number on first page"), &dlg.d);
    first->setChecked(true);
    form->addRow(QCoreApplication::translate("Dialogs", "Position:"), pos);
    form->addRow(QCoreApplication::translate("Dialogs", "Number format:"), format);
    form->addRow(first);
    dlg.v->addLayout(form);
    if (!dlg.exec()) return;
    Document *d = ed->doc();
    const QString code = QStringList{"page", "page:roman", "page:ROMAN", "page:alpha"}[format->currentIndex()];
    MasterPage *m = d->masterFor(*d->pages[ed->currentPage()]);
    if (!m) m = d->masters.first().get();
    const QSizeF ps = d->pageSize();
    const QRectF content = QRectF(QPointF(0, 0), ps).marginsRemoved(d->setup.margins);
    const int pi = pos->currentIndex();
    const double y = pi < 3 ? std::max(6.0, content.top() - 28) : std::min(ps.height() - 30, content.bottom() + 6);
    ed->change(QCoreApplication::translate("Dialogs", "Page Number"), [&] {
        auto t = std::static_pointer_cast<TextItem>(ed->newTextBox(QRectF(content.left(), y, content.width(), 22)));
        t->name = QStringLiteral("Page Number");
        QTextCursor c(d->storyDoc(t->storyId));
        QTextBlockFormat bf;
        bf.setAlignment(pi % 3 == 0 ? Qt::AlignLeft : pi % 3 == 1 ? Qt::AlignHCenter : Qt::AlignRight);
        c.mergeBlockFormat(bf);
        QTextCharFormat f;
        f.setProperty(tp::Field, code);
        c.insertText(QString(QChar::ObjectReplacementCharacter), f);
        m->items.push_back(t);
        if (!first->isChecked() && d->pages.size() > 1) {
            // A master page without the number for page 1.
            auto noNum = std::make_shared<MasterPage>();
            noNum->id = QStringLiteral("T");
            noNum->abbr = QStringLiteral("T");
            noNum->name = QStringLiteral("Title Page (no page number)");
            noNum->grid = m->grid;
            for (const auto &it : m->items) if (it != t) noNum->items.push_back(d->cloneItem(*it));
            if (!d->master("T")) d->masters << noNum;
            d->pages[0]->masterId = QStringLiteral("T");
        }
    });
}

void pageNumberFormatDialog(QWidget *p, Editor *ed)
{
    Document *d = ed->doc();
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Page Number Format"));
    auto *form = new QFormLayout();
    auto *format = new QComboBox(&dlg.d);
    format->setObjectName(QStringLiteral("format"));
    const QStringList codes{QString(), QStringLiteral("alpha"), QStringLiteral("ALPHA"), QStringLiteral("roman"), QStringLiteral("ROMAN")};
    format->addItems({QStringLiteral("1, 2, 3, ..."), QStringLiteral("a, b, c, ..."), QStringLiteral("A, B, C, ..."), QStringLiteral("i, ii, iii, ..."), QStringLiteral("I, II, III, ...")});
    format->setCurrentIndex(std::max(0, int(codes.indexOf(d->setup.pageNumberFormat))));
    auto *start = new QSpinBox(&dlg.d);
    start->setObjectName(QStringLiteral("start"));
    start->setRange(1, 1000);   // Publisher refuses a file starting past 1000
    start->setValue(d->setup.firstPageNumber);
    form->addRow(QCoreApplication::translate("Dialogs", "Number format:"), format);
    form->addRow(QCoreApplication::translate("Dialogs", "Start numbering at:"), start);
    dlg.v->addLayout(form);
    auto *note = new QLabel(QCoreApplication::translate("Dialogs", "This changes every page number in the publication. A page number given its own format keeps it."), &dlg.d);
    note->setWordWrap(true);
    dlg.v->addWidget(note);
    if (!dlg.exec()) return;
    ed->change(QCoreApplication::translate("Dialogs", "Page Number Format"), [&] {
        d->setup.pageNumberFormat = codes[format->currentIndex()];
        d->setup.firstPageNumber = start->value();
    });
}

// ---------------- Properties ----------------
void documentPropertiesDialog(QWidget *p, Editor *ed, int tab)
{
    DocProps pr = ed->doc()->props;
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Properties"));
    auto *tabs = new QTabWidget(&dlg.d);
    auto *sum = new QWidget();
    auto *form = new QFormLayout(sum);
    auto *title = new QLineEdit(pr.title), *subject = new QLineEdit(pr.subject), *author = new QLineEdit(pr.author), *manager = new QLineEdit(pr.manager);
    auto *company = new QLineEdit(pr.company), *category = new QLineEdit(pr.category), *keywords = new QLineEdit(pr.keywords);
    auto *comments = new QPlainTextEdit(pr.comments);
    form->addRow(QCoreApplication::translate("Dialogs", "Title:"), title);
    form->addRow(QCoreApplication::translate("Dialogs", "Subject:"), subject);
    form->addRow(QCoreApplication::translate("Dialogs", "Author:"), author);
    form->addRow(QCoreApplication::translate("Dialogs", "Manager:"), manager);
    form->addRow(QCoreApplication::translate("Dialogs", "Company:"), company);
    form->addRow(QCoreApplication::translate("Dialogs", "Category:"), category);
    form->addRow(QCoreApplication::translate("Dialogs", "Keywords:"), keywords);
    form->addRow(QCoreApplication::translate("Dialogs", "Comments:"), comments);
    tabs->addTab(sum, QCoreApplication::translate("Dialogs", "Summary"));
    auto *print = new QWidget();
    auto *pf = new QFormLayout(print);
    auto *model = new QComboBox(print);
    model->addItems({QCoreApplication::translate("Dialogs", "RGB (best for desktop printers and screens)"), QCoreApplication::translate("Dialogs", "Single color (spot)"), QCoreApplication::translate("Dialogs", "Spot colors"), QCoreApplication::translate("Dialogs", "Process colors (CMYK)"), QCoreApplication::translate("Dialogs", "Process plus spot colors")});
    model->setCurrentIndex(int(ed->doc()->print.model));
    pf->addRow(QCoreApplication::translate("Dialogs", "Color model:"), model);
    // Spot colors: inks the print shop mixes, by name and color. They show
    // in every color drop-down, and print on plates of their own.
    QVector<QColor> spotColors = ed->doc()->print.spotColors;
    QStringList spotNames;
    for (int i = 0; i < spotColors.size(); ++i) spotNames << ed->doc()->print.spotName(i);
    auto *spotBox = new QWidget(print);
    auto *sv = new QVBoxLayout(spotBox);
    sv->setContentsMargins(0, 0, 0, 0);
    auto *spotList = new QListWidget(spotBox);
    spotList->setIconSize(QSize(28, 16));
    spotList->setMinimumHeight(110);
    auto fillSpots = [=, &spotColors, &spotNames] {
        spotList->clear();
        for (int i = 0; i < spotColors.size(); ++i) {
            QPixmap pm(28, 16);
            pm.fill(spotColors[i]);
            spotList->addItem(new QListWidgetItem(QIcon(pm), spotNames[i]));
        }
    };
    fillSpots();
    auto *sr = new QHBoxLayout();
    auto *addSpot = new QPushButton(QCoreApplication::translate("Dialogs", "Add…"), spotBox), *editSpot = new QPushButton(QCoreApplication::translate("Dialogs", "Modify…"), spotBox),
         *delSpot = new QPushButton(QCoreApplication::translate("Dialogs", "Remove"), spotBox);
    sr->addWidget(addSpot);
    sr->addWidget(editSpot);
    sr->addWidget(delSpot);
    sr->addStretch(1);
    sv->addWidget(spotList);
    sv->addLayout(sr);
    pf->addRow(QCoreApplication::translate("Dialogs", "Spot colors:"), spotBox);
    // Name and color of one spot color.
    auto askSpot = [&](QString *name, QColor *color) {
        QDialog sd(&dlg.d);
        sd.setWindowTitle(QCoreApplication::translate("Dialogs", "Spot Color"));
        auto *f = new QFormLayout(&sd);
        auto *n = new QLineEdit(*name, &sd);
        n->setPlaceholderText(QCoreApplication::translate("Dialogs", "The ink's name, as the print shop knows it"));
        auto *c = new QPushButton(&sd);
        QColor chosen = *color;
        auto paintBtn = [c, &chosen] { c->setStyleSheet(QStringLiteral("background:%1; border:1px solid #888; min-width:80px; min-height:22px;").arg(chosen.name())); };
        paintBtn();
        QObject::connect(c, &QPushButton::clicked, &sd, [&] {
            const QColor got = colorsDialog(&sd, chosen, QCoreApplication::translate("Dialogs", "Spot Color"));
            if (got.isValid()) { chosen = got; paintBtn(); }
        });
        auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &sd);
        QObject::connect(bb, &QDialogButtonBox::accepted, &sd, &QDialog::accept);
        QObject::connect(bb, &QDialogButtonBox::rejected, &sd, &QDialog::reject);
        f->addRow(QCoreApplication::translate("Dialogs", "Name:"), n);
        f->addRow(QCoreApplication::translate("Dialogs", "Color:"), c);
        f->addRow(bb);
        if (sd.exec() != QDialog::Accepted) return false;
        *name = n->text().trimmed();
        *color = chosen;
        return true;
    };
    QObject::connect(addSpot, &QPushButton::clicked, &dlg.d, [&] {
        if (model->currentIndex() == int(PrintInfo::SingleSpot) && !spotColors.isEmpty()) return;
        QString name = QCoreApplication::translate("Dialogs", "Spot color %1").arg(spotColors.size() + 1);
        QColor color(0, 90, 170);
        if (!askSpot(&name, &color)) return;
        spotColors << color;
        spotNames << name;
        fillSpots();
    });
    QObject::connect(editSpot, &QPushButton::clicked, &dlg.d, [&] {
        const int i = spotList->currentRow();
        if (i < 0) return;
        if (askSpot(&spotNames[i], &spotColors[i])) fillSpots();
    });
    QObject::connect(delSpot, &QPushButton::clicked, &dlg.d, [&] {
        const int i = spotList->currentRow();
        if (i < 0) return;
        spotColors.remove(i);
        spotNames.removeAt(i);
        fillSpots();
    });
    auto showSpots = [=] {
        const auto m = PrintInfo::ColorModel(model->currentIndex());
        spotBox->setEnabled(m == PrintInfo::SingleSpot || m == PrintInfo::SpotColors || m == PrintInfo::ProcessPlusSpot);
    };
    QObject::connect(model, &QComboBox::currentIndexChanged, &dlg.d, showSpots);
    showSpots();
    // Black overprinting in files for a printer (PDF/X): black prints over
    // the inks under it, so a shifted plate leaves no white edge.
    const OverprintSettings op0 = ed->doc()->print.overprint;
    auto *opBox = new QGroupBox(QCoreApplication::translate("Dialogs", "Overprint black in files for a printer (PDF/X)"), print);
    auto *og = new QGridLayout(opBox);
    auto *opText = new QCheckBox(QCoreApplication::translate("Dialogs", "Text below:"), opBox);
    auto *opSize = new QDoubleSpinBox(opBox);
    opSize->setRange(1, 1638);
    opSize->setDecimals(1);
    opSize->setSuffix(QStringLiteral(" pt"));
    auto *opLines = new QCheckBox(QCoreApplication::translate("Dialogs", "Lines"), opBox);
    auto *opFills = new QCheckBox(QCoreApplication::translate("Dialogs", "Fills"), opBox);
    auto *opThreshold = new QSpinBox(opBox);
    opThreshold->setRange(1, 100);
    opThreshold->setSuffix(QStringLiteral("%"));
    auto *opReset = new QPushButton(QCoreApplication::translate("Dialogs", "Reset All"), opBox);
    auto showOverprint = [=](const OverprintSettings &v) {
        opText->setChecked(v.text);
        opSize->setValue(v.textBelow);
        opLines->setChecked(v.lines);
        opFills->setChecked(v.fills);
        opThreshold->setValue(v.threshold);
    };
    showOverprint(op0);
    QObject::connect(opText, &QCheckBox::toggled, opSize, &QWidget::setEnabled);
    opSize->setEnabled(op0.text);
    QObject::connect(opReset, &QPushButton::clicked, opBox, [=] { showOverprint(OverprintSettings()); });
    og->addWidget(opText, 0, 0);
    og->addWidget(opSize, 0, 1);
    og->addWidget(opLines, 1, 0);
    og->addWidget(opFills, 2, 0);
    og->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "Black counts from:"), opBox), 3, 0);
    og->addWidget(opThreshold, 3, 1);
    og->addWidget(opReset, 4, 0);
    og->setColumnStretch(2, 1);
    pf->addRow(opBox);
    tabs->addTab(print, QCoreApplication::translate("Dialogs", "Commercial Print"));
    tabs->setCurrentIndex(std::clamp(tab, 0, 1));
    dlg.v->addWidget(tabs);
    if (!dlg.exec()) return;
    ed->change(QCoreApplication::translate("Dialogs", "Properties"), [&] {
        DocProps &d = ed->doc()->props;
        d.title = title->text(); d.subject = subject->text(); d.author = author->text(); d.manager = manager->text();
        d.company = company->text(); d.category = category->text(); d.keywords = keywords->text(); d.comments = comments->toPlainText();
        ed->doc()->print.model = PrintInfo::ColorModel(model->currentIndex());
        OverprintSettings &op = ed->doc()->print.overprint;
        op.text = opText->isChecked();
        op.textBelow = opSize->value();
        op.lines = opLines->isChecked();
        op.fills = opFills->isChecked();
        op.threshold = opThreshold->value();
        ed->doc()->print.spotColors = spotColors;
        ed->doc()->print.spotNames = spotNames;
    });
}

// ---------------- Measurement toolbar ----------------
void measurementWindow(QWidget *p, Editor *ed)
{
    static QPointer<QDialog> win;
    if (win) { win->raise(); win->show(); return; }
    win = new QDialog(p, Qt::Tool);
    win->setWindowTitle(QCoreApplication::translate("Dialogs", "Measurement"));
    win->setAttribute(Qt::WA_DeleteOnClose);
    auto *form = new QFormLayout(win);
    auto *x = measure(0, win, -10000), *y = measure(0, win, -10000), *w = measure(0, win, 1), *h = measure(0, win, 1);
    x->setMinimum(-10000);
    y->setMinimum(-10000);
    auto *rot = new DecimalSpin(win);
    rot->setRange(-360, 360);
    rot->setSuffix(QStringLiteral("°"));
    auto *track = new DecimalSpin(win);
    track->setRange(0, 600);
    track->setSuffix(QStringLiteral("%"));
    auto *kern = new DecimalSpin(win);
    kern->setRange(-600, 600);
    kern->setSuffix(QStringLiteral(" pt"));
    auto *scale = new QSpinBox(win);
    scale->setRange(1, 600);
    scale->setSuffix(QStringLiteral("%"));
    auto *line = new DecimalSpin(win);
    line->setRange(0.1, 20);
    line->setSuffix(QStringLiteral(" sp"));
    form->addRow(QCoreApplication::translate("Dialogs", "Horizontal position:"), x);
    form->addRow(QCoreApplication::translate("Dialogs", "Vertical position:"), y);
    form->addRow(QCoreApplication::translate("Dialogs", "Width:"), w);
    form->addRow(QCoreApplication::translate("Dialogs", "Height:"), h);
    form->addRow(QCoreApplication::translate("Dialogs", "Rotation:"), rot);
    form->addRow(QCoreApplication::translate("Dialogs", "Tracking:"), track);
    form->addRow(QCoreApplication::translate("Dialogs", "Text scaling:"), scale);
    form->addRow(QCoreApplication::translate("Dialogs", "Kerning:"), kern);
    form->addRow(QCoreApplication::translate("Dialogs", "Line spacing:"), line);
    auto refresh = [=] {
        Item *it = ed->single();
        const bool on = it && it->type() != ItemType::Line && it->type() != ItemType::Group;
        for (QWidget *wd : QList<QWidget *>{x, y, w, h, rot}) wd->setEnabled(on);
        if (on) {
            x->setPoints(it->rect.x()); y->setPoints(it->rect.y()); w->setPoints(it->rect.width()); h->setPoints(it->rect.height());
            if (!rot->hasFocus()) { const QSignalBlocker b(rot); rot->setValue(it->rotation); }
        }
        const QTextCharFormat cf = ed->currentCharFormat();
        if (!track->hasFocus()) { const QSignalBlocker b(track); track->setValue(tp::trackingOf(cf)); }
        if (!kern->hasFocus()) { const QSignalBlocker b(kern); kern->setValue(tp::kerningOf(cf)); }
        if (!scale->hasFocus()) { const QSignalBlocker b(scale); scale->setValue(cf.hasProperty(QTextFormat::FontStretch) ? cf.fontStretch() : 100); }
        const QTextBlockFormat bf = ed->currentBlockFormat();
        if (!line->hasFocus()) { const QSignalBlocker b(line); line->setValue(bf.lineHeightType() == QTextBlockFormat::ProportionalHeight ? bf.lineHeight() / 100 : 1.0); }
    };
    QObject::connect(ed, &Editor::selectionChanged, win, refresh);
    QObject::connect(ed, &Editor::changed, win, refresh);
    QObject::connect(ed, &Editor::textCursorChanged, win, refresh);
    auto applyGeom = [=] {
        Item *it = ed->single();
        if (!it || it->type() == ItemType::Line || it->type() == ItemType::Group) return;
        ed->change(QCoreApplication::translate("Dialogs", "Measurement"), [&] {
            const QRectF before = it->rect;
            if (std::abs(w->value() - before.width()) > 0.01 || std::abs(h->value() - before.height()) > 0.01)
                it->scaleInto(before, QRectF(before.topLeft(), QSizeF(w->value(), h->value())));
            it->rect.moveTopLeft(QPointF(x->value(), y->value()));
            it->rotation = rot->value();
        });
    };
    for (QDoubleSpinBox *s : {static_cast<QDoubleSpinBox *>(x), static_cast<QDoubleSpinBox *>(y), static_cast<QDoubleSpinBox *>(w), static_cast<QDoubleSpinBox *>(h), static_cast<QDoubleSpinBox *>(rot)})
        QObject::connect(s, &QDoubleSpinBox::editingFinished, win, applyGeom);
    QObject::connect(track, &QDoubleSpinBox::editingFinished, win, [=] {
        QTextCharFormat f;
        f.setProperty(tp::Tracking, track->value());
        ed->mergeCharFormat(f, QCoreApplication::translate("Dialogs", "Tracking"));
    });
    QObject::connect(kern, &QDoubleSpinBox::editingFinished, win, [=] {
        QTextCharFormat f;
        f.setFontLetterSpacingType(QFont::AbsoluteSpacing);
        f.setFontLetterSpacing(kern->value());
        ed->mergeCharFormat(f, QCoreApplication::translate("Dialogs", "Kerning"));
    });
    QObject::connect(scale, &QSpinBox::editingFinished, win, [=] {
        QTextCharFormat f;
        f.setFontStretch(scale->value());
        ed->mergeCharFormat(f, QCoreApplication::translate("Dialogs", "Text Scaling"));
    });
    QObject::connect(line, &QDoubleSpinBox::editingFinished, win, [=] { ed->setLineSpacing(QTextBlockFormat::ProportionalHeight, line->value() * 100); });
    refresh();
    win->show();
}

// ---------------- Hyphenation, paste special, misc ----------------
void hyphenationDialog(QWidget *p, Editor *ed)
{
    // The story of the text box being typed in or selected: all its boxes.
    // In a table, the selected cells, the cell being typed in, or with the table selected, every cell:
    // the dialog shows the first of them and Add Optional Hyphens works on them. The setting itself
    // is the whole table's, as its cells are one story.
    Document *d = ed->doc();
    const QString id = ed->isEditingText() ? ed->textTarget().itemId : (ed->single() ? ed->single()->id : QString());
    auto *t = dynamic_cast<TextItem *>(d->item(id));
    auto *tb = dynamic_cast<TableItem *>(d->item(id));
    QVector<TableCell *> cells;
    if (tb) {
        const CellRange rg = ed->cellsToFormat(tb);
        for (int r = rg.r0; r <= rg.r1; ++r)
            for (int c = rg.c0; c <= rg.c1; ++c) cells << &tb->cell(r, c);
    }
    if (!t && cells.isEmpty()) {
        QMessageBox::information(p, QCoreApplication::translate("Dialogs", "Hyphenation"), QCoreApplication::translate("Dialogs", "Click in a text box or a table first."));
        return;
    }
    const QVector<TextItem *> chain = t ? d->chainOf(t->id) : QVector<TextItem *>();
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Hyphenation"));
    auto *autoH = new QCheckBox(QCoreApplication::translate("Dialogs", "Automatically hyphenate this story"), &dlg.d);
    autoH->setChecked(t ? t->hyphenate : cells.first()->hyphenate);
    auto *zone = measure(t ? t->hyphenZone : cells.first()->hyphenZone, &dlg.d, 0, 720);
    zone->setToolTip(QCoreApplication::translate("Dialogs", "A word is broken only if moving it whole to the next line would leave more space than this."));
    auto *form = new QFormLayout();
    form->addRow(autoH);
    form->addRow(QCoreApplication::translate("Dialogs", "Hyphenation zone:"), zone);
    dlg.v->addLayout(form);
    auto *manual = dlg.bb->addButton(QCoreApplication::translate("Dialogs", "Insert Optional Hyphen"), QDialogButtonBox::ActionRole);
    manual->setEnabled(ed->isEditingText());
    QObject::connect(manual, &QPushButton::clicked, &dlg.d, [&] { if (ed->isEditingText()) ed->insertTextBlock(QString(QChar(0x00AD)), QCoreApplication::translate("Dialogs", "Optional Hyphen")); });
    auto *everywhere = dlg.bb->addButton(QCoreApplication::translate("Dialogs", "Add Optional Hyphens"), QDialogButtonBox::ActionRole);
    everywhere->setToolTip(QCoreApplication::translate("Dialogs", "Put an optional hyphen at every place each long word can break; they show only where a line breaks."));
    QObject::connect(everywhere, &QPushButton::clicked, &dlg.d, [&] {
        int n = 0;
        QVector<QTextDocument *> stories;
        if (t) stories << d->storyDoc(t->storyId);
        for (const TableCell *c : cells) stories << d->storyDoc(c->storyId) << d->storyDoc(c->storyId2);
        stories.removeAll(nullptr);
        if (!stories.isEmpty()) ed->change(QCoreApplication::translate("Dialogs", "Hyphenate"), [&] { for (QTextDocument *sd : stories) n += hyphenateStory(sd); });
        Q_EMIT ed->status(QCoreApplication::translate("Dialogs", "Added %1 optional hyphens.").arg(n));
    });
    zone->setEnabled(autoH->isChecked());
    QObject::connect(autoH, &QCheckBox::toggled, zone, &QWidget::setEnabled);
    if (!dlg.exec()) return;
    ed->change(QCoreApplication::translate("Dialogs", "Hyphenation"), [&] {
        for (TextItem *f : chain) {
            f->hyphenate = autoH->isChecked();
            f->hyphenZone = zone->value();
        }
        if (tb)
            for (TableCell &c : tb->cells) {
                c.hyphenate = autoH->isChecked();
                c.hyphenZone = zone->value();
            }
        if (tb) ed->fitTableRows(tb);   // words wrap differently
    });
}

void pasteSpecialDialog(QWidget *p, Editor *ed)
{
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Paste Special"));
    auto *list = new QListWidget(&dlg.d);
    list->addItems({QCoreApplication::translate("Dialogs", "Keep source formatting"), QCoreApplication::translate("Dialogs", "Keep text only (unformatted text)"), QCoreApplication::translate("Dialogs", "Picture (PNG)")});
    list->setCurrentRow(0);
    dlg.v->addWidget(list);
    if (!dlg.exec()) return;
    const int i = list->currentRow();
    if (i == 1) { ed->paste(true); return; }
    if (i == 2) {
        const QMimeData *md = QApplication::clipboard()->mimeData();
        if (md && md->hasImage()) {
            const bool wasEditing = ed->isEditingText();
            ed->endTextEdit();
            ed->paste();
            Q_UNUSED(wasEditing);
        }
        return;
    }
    ed->paste(false);
}


// Autoflow: while a story doesn't fit, it continues on a new page after
// the last box's page, in a linked text box placed like that box.
int autoflowText(Editor *ed, const QString &boxId)
{
    Document *d = ed->doc();
    auto overflows = [&] {
        const QVector<TextItem *> chain = d->chainOf(boxId);
        if (chain.isEmpty()) return false;
        TextItem *last = chain.last();
        const int pg = d->find(last->id).page;
        if (pg < 0) return false;
        // A fresh layout each time: the boxes change as pages are added.
        LayoutCache cache;
        RenderOptions opt;
        const auto fl = cache.textFrame(*d, *last, pg + 1, opt);
        return fl.layout && fl.layout->overflow();
    };
    int added = 0;
    ed->beginChange(QCoreApplication::translate("Dialogs", "Autoflow"));
    while (added < 500 && overflows()) {
        TextItem *last = d->chainOf(boxId).last();
        const int pg = d->find(last->id).page;
        auto page = d->addPage(pg + 1, d->pages[pg]->masterId);
        auto box = std::make_shared<TextItem>();
        box->rect = last->rect;
        box->rotation = last->rotation;
        box->insets = last->insets;
        box->columns = last->columns;
        box->columnGap = last->columnGap;
        box->fill = last->fill;
        box->stroke = last->stroke;
        box->hyphenate = last->hyphenate;
        box->hyphenZone = last->hyphenZone;
        box->storyId = last->storyId;
        last->nextId = box->id;
        page->items.push_back(box);
        ++added;
    }
    ed->endChange();
    return added;
}

// Offer autoflow when inserted text doesn't fit its box.
static void offerAutoflow(QWidget *p, Editor *ed, const QString &boxId)
{
    Document *d = ed->doc();
    const QVector<TextItem *> chain = d->chainOf(boxId);
    if (chain.isEmpty() || d->find(chain.last()->id).page < 0) return;
    TextItem *last = chain.last();
    RenderOptions opt;
    const auto fl = ed->cache().textFrame(*d, *last, d->find(last->id).page + 1, opt);
    if (!fl.layout || !fl.layout->overflow()) return;
    if (QMessageBox::question(p, QCoreApplication::translate("Dialogs", "Autoflow"),
                              QCoreApplication::translate("Dialogs", "The inserted text doesn't fit in the text box. Do you want to continue it on new pages, in text boxes like this one?"))
        != QMessageBox::Yes)
        return;
    const int n = autoflowText(ed, boxId);
    Q_EMIT ed->status(QCoreApplication::translate("Dialogs", "Added %n page(s) for the rest of the text.", "", n));
}

void insertFileDialog(QWidget *p, Editor *ed)
{
    const QString f = QFileDialog::getOpenFileName(p, QCoreApplication::translate("Dialogs", "Insert Text"), QString(),
                                                   QCoreApplication::translate("Dialogs", "Text Files (*.txt *.rtf *.html *.htm *.docx *.md)") + QStringLiteral(";;")
                                                       + QCoreApplication::translate("Dialogs", "All Files (*)"));
    if (f.isEmpty()) return;
    if (ed->isEditingText()) {
        ed->beginChange(QCoreApplication::translate("Dialogs", "Insert File"));
        QTextDocument tmp;
        loadTextFileInto(&tmp, f);
        ed->cursor().insertFragment(QTextDocumentFragment(&tmp));
        ed->endChange();
        ed->textEdited();
        const QString box = ed->textTarget().itemId;
        if (dynamic_cast<TextItem *>(ed->doc()->item(box))) {
            ed->endTextEdit();
            offerAutoflow(p, ed, box);
        }
        return;
    }
    const QSizeF ps = ed->doc()->pageSize();
    const QRectF content = QRectF(QPointF(0, 0), ps).marginsRemoved(ed->doc()->setup.margins);
    auto t = std::static_pointer_cast<TextItem>(ed->newTextBox(content));
    loadTextFileInto(ed->doc()->storyDoc(t->storyId), f);
    ed->addItem(t);
    offerAutoflow(p, ed, t->id);
}

// ---------------- Mail merge recipients ----------------
void recipientsDialog(QWidget *p, Editor *ed, bool typeNew)
{
    MergeSource m = ed->doc()->merge;
    if (typeNew && m.isEmpty()) {
        m.fields = {"Title", "First Name", "Last Name", "Company Name", "Address Line 1", "Address Line 2", "City", "State", "ZIP Code", "Country or Region", "Home Phone", "Work Phone", "Email Address"};
        m.rows << QStringList(m.fields.size());
        m.include << true;
    }
    if (m.isEmpty()) {
        QMessageBox::information(p, QCoreApplication::translate("Dialogs", "Mail Merge Recipients"), QCoreApplication::translate("Dialogs", "Select a recipient list first (Mailings > Select Recipients)."));
        return;
    }
    Dlg dlg(p, typeNew ? QCoreApplication::translate("Dialogs", "New Address List") : QCoreApplication::translate("Dialogs", "Mail Merge Recipients"));
    dlg.d.resize(900, 520);
    auto *search = new QLineEdit(&dlg.d);
    search->setPlaceholderText(QCoreApplication::translate("Dialogs", "Filter recipients"));
    dlg.v->addWidget(search);
    auto *table = new QTableWidget(m.rows.size(), m.fields.size() + 1, &dlg.d);
    QStringList headers{QString()};
    headers << m.fields;
    table->setHorizontalHeaderLabels(headers);
    for (int r = 0; r < m.rows.size(); ++r) {
        auto *check = new QTableWidgetItem();
        check->setCheckState(m.include.value(r, true) ? Qt::Checked : Qt::Unchecked);
        table->setItem(r, 0, check);
        for (int c = 0; c < m.fields.size(); ++c) table->setItem(r, c + 1, new QTableWidgetItem(m.rows[r].value(c)));
    }
    table->setSortingEnabled(true);
    dlg.v->addWidget(table, 1);
    auto *row = new QHBoxLayout();
    auto *add = new QPushButton(QCoreApplication::translate("Dialogs", "New Entry"), &dlg.d);
    auto *del = new QPushButton(QCoreApplication::translate("Dialogs", "Delete Entry"), &dlg.d);
    auto *addField = new QPushButton(QCoreApplication::translate("Dialogs", "Customize Columns…"), &dlg.d);
    auto *dups = new QPushButton(QCoreApplication::translate("Dialogs", "Find Duplicates"), &dlg.d);
    auto *validate = new QPushButton(QCoreApplication::translate("Dialogs", "Validate Addresses"), &dlg.d);
    row->addWidget(add);
    row->addWidget(del);
    row->addWidget(addField);
    row->addWidget(dups);
    row->addWidget(validate);
    row->addStretch(1);
    dlg.v->addLayout(row);
    QObject::connect(search, &QLineEdit::textChanged, &dlg.d, [=](const QString &q) {
        for (int r = 0; r < table->rowCount(); ++r) {
            bool match = q.isEmpty();
            for (int c = 1; c < table->columnCount() && !match; ++c)
                if (table->item(r, c) && table->item(r, c)->text().contains(q, Qt::CaseInsensitive)) match = true;
            table->setRowHidden(r, !match);
        }
    });
    QObject::connect(add, &QPushButton::clicked, &dlg.d, [=] {
        table->setSortingEnabled(false);
        const int r = table->rowCount();
        table->insertRow(r);
        auto *check = new QTableWidgetItem();
        check->setCheckState(Qt::Checked);
        table->setItem(r, 0, check);
        for (int c = 1; c < table->columnCount(); ++c) table->setItem(r, c, new QTableWidgetItem());
        table->setCurrentCell(r, 1);
        table->editItem(table->item(r, 1));
        table->setSortingEnabled(true);
    });
    QObject::connect(del, &QPushButton::clicked, &dlg.d, [=] { if (table->currentRow() >= 0) table->removeRow(table->currentRow()); });
    QObject::connect(addField, &QPushButton::clicked, &dlg.d, [=] {
        bool ok = false;
        const QString n = QInputDialog::getText(table, QCoreApplication::translate("Dialogs", "Add Field"), QCoreApplication::translate("Dialogs", "Field name:"), QLineEdit::Normal, QString(), &ok);
        if (!ok || n.trimmed().isEmpty()) return;
        const int c = table->columnCount();
        table->insertColumn(c);
        table->setHorizontalHeaderItem(c, new QTableWidgetItem(n.trimmed()));
    });
    QObject::connect(dups, &QPushButton::clicked, &dlg.d, [=] {
        QHash<QString, int> seen;
        int n = 0;
        for (int r = 0; r < table->rowCount(); ++r) {
            QString key;
            for (int c = 1; c < table->columnCount(); ++c) key += (table->item(r, c) ? table->item(r, c)->text().trimmed().toLower() : QString()) + '|';
            if (seen.contains(key)) { table->item(r, 0)->setCheckState(Qt::Unchecked); ++n; }
            else seen.insert(key, r);
        }
        QMessageBox::information(table, QCoreApplication::translate("Dialogs", "Find Duplicates"), QCoreApplication::translate("Dialogs", "Unchecked %1 duplicate entries.").arg(n));
    });
    QObject::connect(validate, &QPushButton::clicked, &dlg.d, [=] {
        int bad = 0;
        int zipCol = -1, addrCol = -1;
        for (int c = 1; c < table->columnCount(); ++c) {
            const QString h = table->horizontalHeaderItem(c)->text().toLower();
            if (h.contains("zip") || h.contains("postal")) zipCol = c;
            if (h.contains("address") && addrCol < 0) addrCol = c;
        }
        for (int r = 0; r < table->rowCount(); ++r) {
            const QString zip = zipCol > 0 && table->item(r, zipCol) ? table->item(r, zipCol)->text().trimmed() : QString();
            const QString addr = addrCol > 0 && table->item(r, addrCol) ? table->item(r, addrCol)->text().trimmed() : QString();
            const bool ok = !addr.isEmpty() && (zip.isEmpty() || QRegularExpression("^\\d{5}(-\\d{4})?$").match(zip).hasMatch() || zip.size() >= 3);
            for (int c = 1; c < table->columnCount(); ++c)
                if (table->item(r, c)) table->item(r, c)->setBackground(ok ? QBrush() : QBrush(QColor(255, 220, 200)));
            bad += !ok;
        }
        QMessageBox::information(table, QCoreApplication::translate("Dialogs", "Validate Addresses"), bad ? QCoreApplication::translate("Dialogs", "%1 entries have a missing address or a ZIP code that doesn't look right (highlighted).").arg(bad)
                                                                                  : QCoreApplication::translate("Dialogs", "All entries have an address and a well-formed ZIP code."));
    });
    if (!dlg.exec()) return;
    MergeSource out;
    out.path = m.path;
    out.pictureField = m.pictureField;
    for (int c = 1; c < table->columnCount(); ++c) out.fields << table->horizontalHeaderItem(c)->text();
    for (int r = 0; r < table->rowCount(); ++r) {
        QStringList rowv;
        bool any = false;
        for (int c = 1; c < table->columnCount(); ++c) {
            const QString v = table->item(r, c) ? table->item(r, c)->text() : QString();
            any |= !v.isEmpty();
            rowv << v;
        }
        if (!any) continue;
        out.rows << rowv;
        out.include << (table->item(r, 0) && table->item(r, 0)->checkState() == Qt::Checked);
    }
    if (typeNew) {
        const QString path = askSavePath(p, QCoreApplication::translate("Dialogs", "Save Address List"), QStringLiteral("Recipients.csv"), QCoreApplication::translate("Dialogs", "CSV (*.csv)"));
        if (!path.isEmpty()) { saveMergeCsv(out, path); out.path = path; }
    }
    ed->change(QCoreApplication::translate("Dialogs", "Recipients"), [&] { ed->doc()->merge = out; });
}

void mergeFieldDialog(QWidget *p, Editor *ed, int kind)
{
    if (ed->doc()->merge.isEmpty()) {
        QMessageBox::information(p, QCoreApplication::translate("Dialogs", "Mail Merge"), QCoreApplication::translate("Dialogs", "Select a recipient list first."));
        return;
    }
    const QString code = kind == 0 ? QStringLiteral("mergeblock:address") : QStringLiteral("mergeblock:greeting");
    if (ed->isEditingText()) { ed->insertField(code); return; }
    const QSizeF ps = ed->doc()->pageSize();
    auto t = std::static_pointer_cast<TextItem>(ed->newTextBox(QRectF(ps.width() / 2 - 144, ps.height() / 2 - 40, 288, kind == 0 ? 80 : 28)));
    QTextCursor c(ed->doc()->storyDoc(t->storyId));
    QTextCharFormat f;
    f.setProperty(tp::Field, code);
    c.insertText(QString(QChar::ObjectReplacementCharacter), f);
    ed->addItem(t);
}

void spellingDialog(QWidget *p, Editor *ed);   // defined in proofing.cpp
void thesaurusDialog(QWidget *p, Editor *ed);

} // namespace jp
