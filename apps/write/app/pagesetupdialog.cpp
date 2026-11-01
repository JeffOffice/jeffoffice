// The Page Setup dialog (the work on the document is core/pagesetup.h) and the Alt Text dialog.

#include "app/writedialogs.h"

#include "app/widgets.h"
#include "core/pagesetup.h"
#include "core/pictureops.h"
#include "core/writedoc.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <iterator>

namespace jp::write {

namespace {

struct Paper {
    const char *name;
    double w, h;
};
// The names are looked up with translate() at the place of use (each is marked here for the extractor).
const Paper kPapers[] = {
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Letter"), 612, 792},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Legal"), 612, 1008},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Executive"), 522, 756},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "A3"), 841.9, 1190.55},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "A4"), 595.3, 841.9},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "A5"), 419.55, 595.3},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "B5"), 498.95, 708.75},
};

} // namespace

bool pageSetupDialog(QWidget *parent, WriteDoc *doc, int section)
{
    if (!doc) return false;
    section = std::clamp(section, 0, doc->sectionCount() - 1);
    const SectionSetup before = pageSetupOf(doc->sectionProps(section), doc->differentOddEven());
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("pageSetupDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::Dialogs", "Page Setup"));
    auto *v = new QVBoxLayout(&dlg);
    auto *tabs = new QTabWidget(&dlg);
    tabs->setObjectName(QStringLiteral("pageSetupTabs"));
    v->addWidget(tabs);

    auto spin = [&dlg](const char *name, double points) {
        auto *s = new MeasureSpin(&dlg);
        s->setObjectName(QString::fromLatin1(name));
        s->setPoints(points);
        s->setValue(points);
        return s;
    };

    // ---- Margins
    auto *marginsPage = new QWidget;
    auto *mf = new QFormLayout(marginsPage);
    auto *top = spin("top", before.top), *bottom = spin("bottom", before.bottom), *left = spin("left", before.left), *right = spin("right", before.right), *gutter = spin("gutter", before.gutter);
    mf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Top:"), top);
    mf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Bottom:"), bottom);
    mf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Left:"), left);
    mf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Right:"), right);
    mf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Gutter:"), gutter);
    auto *portrait = new QRadioButton(QCoreApplication::translate("jp::write::Dialogs", "Portrait"), marginsPage), *landscape = new QRadioButton(QCoreApplication::translate("jp::write::Dialogs", "Landscape"), marginsPage);
    portrait->setObjectName(QStringLiteral("portrait"));
    landscape->setObjectName(QStringLiteral("landscape"));
    (before.landscape ? landscape : portrait)->setChecked(true);
    auto *orientation = new QGroupBox(QCoreApplication::translate("jp::write::Dialogs", "Orientation"), marginsPage);
    auto *ov = new QVBoxLayout(orientation);
    ov->addWidget(portrait);
    ov->addWidget(landscape);
    mf->addRow(orientation);
    tabs->addTab(marginsPage, QCoreApplication::translate("jp::write::Dialogs", "Margins"));

    // ---- Paper
    auto *paperPage = new QWidget;
    auto *pf = new QFormLayout(paperPage);
    auto *size = new QComboBox(paperPage);
    size->setObjectName(QStringLiteral("paperSize"));
    for (const Paper &p : kPapers) size->addItem(QCoreApplication::translate("jp::write::Dialogs", p.name));
    size->addItem(QCoreApplication::translate("jp::write::Dialogs", "Custom size"));
    const double shownW = before.landscape ? before.paperHeight : before.paperWidth, shownH = before.landscape ? before.paperWidth : before.paperHeight;
    auto *width = spin("paperWidth", shownW), *height = spin("paperHeight", shownH);
    pf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Paper size:"), size);
    pf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Width:"), width);
    pf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Height:"), height);
    tabs->addTab(paperPage, QCoreApplication::translate("jp::write::Dialogs", "Paper"));
    // The paper size box names the paper the width and height make (either way turned), else Custom.
    auto nameThePaper = [=] {
        int found = int(std::size(kPapers));
        for (int i = 0; i < int(std::size(kPapers)); ++i) {
            const double lo = std::min(kPapers[i].w, kPapers[i].h), hi = std::max(kPapers[i].w, kPapers[i].h);
            if (std::abs(std::min(width->value(), height->value()) - lo) < 2 && std::abs(std::max(width->value(), height->value()) - hi) < 2) found = i;
        }
        const QSignalBlocker block(size);
        size->setCurrentIndex(found);
    };
    nameThePaper();
    QObject::connect(size, &QComboBox::activated, &dlg, [=](int i) {
        if (i < 0 || i >= int(std::size(kPapers))) return;
        const bool wide = landscape->isChecked();
        width->setValue(wide ? kPapers[i].h : kPapers[i].w);
        height->setValue(wide ? kPapers[i].w : kPapers[i].h);
    });
    // The page is landscape when it is wider than it is high: turning it swaps the sides and turns the margins with it, as the Orientation commands do.
    auto turn = [=](bool toLandscape) {
        const bool now = width->value() > height->value();
        if (now == toLandscape) return;
        const double w = width->value(), h = height->value();
        width->setValue(h);
        height->setValue(w);
        const double t = top->value(), b = bottom->value(), l = left->value(), r = right->value();
        if (toLandscape) {
            top->setValue(l);
            left->setValue(b);
            bottom->setValue(r);
            right->setValue(t);
        } else {
            top->setValue(r);
            right->setValue(b);
            bottom->setValue(l);
            left->setValue(t);
        }
    };
    QObject::connect(portrait, &QRadioButton::toggled, &dlg, [=](bool on) {
        if (on) turn(false);
    });
    QObject::connect(landscape, &QRadioButton::toggled, &dlg, [=](bool on) {
        if (on) turn(true);
    });
    auto widthOrHeightEdited = [=] {
        const QSignalBlocker a(portrait), b(landscape);
        (width->value() > height->value() ? landscape : portrait)->setChecked(true);
        nameThePaper();
    };
    QObject::connect(width, &QDoubleSpinBox::editingFinished, &dlg, widthOrHeightEdited);
    QObject::connect(height, &QDoubleSpinBox::editingFinished, &dlg, widthOrHeightEdited);

    // ---- Layout
    auto *layoutPage = new QWidget;
    auto *lf = new QFormLayout(layoutPage);
    auto *start = new QComboBox(layoutPage);
    start->setObjectName(QStringLiteral("sectionStart"));
    const QStringList starts = {QStringLiteral("nextPage"), QStringLiteral("continuous"), QStringLiteral("evenPage"), QStringLiteral("oddPage")};
    start->addItem(QCoreApplication::translate("jp::write::Dialogs", "New page"), starts[0]);
    start->addItem(QCoreApplication::translate("jp::write::Dialogs", "Continuous"), starts[1]);
    start->addItem(QCoreApplication::translate("jp::write::Dialogs", "Even page"), starts[2]);
    start->addItem(QCoreApplication::translate("jp::write::Dialogs", "Odd page"), starts[3]);
    start->setCurrentIndex(std::max(0, int(starts.indexOf(before.start))));
    lf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Section start:"), start);
    auto *firstPage = new QCheckBox(QCoreApplication::translate("jp::write::Dialogs", "Different first page"), layoutPage), *oddEven = new QCheckBox(QCoreApplication::translate("jp::write::Dialogs", "Different odd and even pages"), layoutPage);
    firstPage->setObjectName(QStringLiteral("differentFirstPage"));
    oddEven->setObjectName(QStringLiteral("differentOddEven"));
    firstPage->setChecked(before.differentFirstPage);
    oddEven->setChecked(before.differentOddEven);
    lf->addRow(firstPage);
    lf->addRow(oddEven);
    auto *header = spin("header", before.header), *footer = spin("footer", before.footer);
    lf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Header, from the edge:"), header);
    lf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Footer, from the edge:"), footer);
    tabs->addTab(layoutPage, QCoreApplication::translate("jp::write::Dialogs", "Layout"));

    // ---- Apply to
    auto *applyTo = new QComboBox(&dlg);
    applyTo->setObjectName(QStringLiteral("applyTo"));
    applyTo->addItem(QCoreApplication::translate("jp::write::Dialogs", "This section"), int(PageSetupScope::ThisSection));
    applyTo->addItem(QCoreApplication::translate("jp::write::Dialogs", "Whole document"), int(PageSetupScope::WholeDocument));
    if (doc->sectionCount() == 1) applyTo->setCurrentIndex(1);
    auto *row = new QFormLayout;
    row->addRow(QCoreApplication::translate("jp::write::Dialogs", "Apply to:"), applyTo);
    v->addLayout(row);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return false;

    SectionSetup after = before;
    const bool wide = width->value() > height->value();
    after.landscape = wide;
    after.paperWidth = wide ? height->value() : width->value();
    after.paperHeight = wide ? width->value() : height->value();
    after.top = top->value();
    after.bottom = bottom->value();
    after.left = left->value();
    after.right = right->value();
    after.gutter = gutter->value();
    after.header = header->value();
    after.footer = footer->value();
    after.start = start->currentData().toString();
    after.differentFirstPage = firstPage->isChecked();
    after.differentOddEven = oddEven->isChecked();
    return applyPageSetup(*doc, section, PageSetupScope(applyTo->currentData().toInt()), before, after) > 0;
}

bool altTextDialog(QWidget *parent, QString *alt)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("altTextDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::Dialogs", "Alt Text"));
    auto *v = new QVBoxLayout(&dlg);
    auto *note = new QLabel(QCoreApplication::translate("jp::write::Dialogs", "Describe the picture for people who can't see it."), &dlg);
    note->setWordWrap(true);
    v->addWidget(note);
    auto *edit = new QPlainTextEdit(&dlg);
    edit->setObjectName(QStringLiteral("altText"));
    edit->setAccessibleName(QCoreApplication::translate("jp::write::Dialogs", "Alt text"));
    edit->setPlainText(*alt);
    edit->setMinimumSize(320, 110);
    v->addWidget(edit);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return false;
    // (One line: a line break in an attribute is a blank.)
    *alt = cleanAltText(edit->toPlainText().simplified());
    return true;
}

} // namespace jp::write
