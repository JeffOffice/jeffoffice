// The Page Setup dialog (the work on the document is core/pagesetup.h) and the Alt Text dialog.

#include "app/uistyle.h"
#include "app/writedialogs.h"

#include "app/widgets.h"
#include "app/userdefaults.h"
#include "app/normaltemplateui.h"
#include "core/normaltemplate.h"
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
#include <QPushButton>
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
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Envelope #10"), 297, 684},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Envelope Monarch"), 279, 540},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Envelope DL"), 311.81, 623.62},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Envelope C5"), 459.21, 649.13},
};

} // namespace

bool pageSetupDialog(QWidget *parent, WriteDoc *doc, int section, QTextCursor *cursor)
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
    mf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Top"), top);
    mf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Bottom"), bottom);
    mf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Left"), left);
    mf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Right"), right);
    mf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Gutter"), gutter);
    auto *portrait = new QRadioButton(QCoreApplication::translate("jp::write::Dialogs", "Portrait"), marginsPage), *landscape = new QRadioButton(QCoreApplication::translate("jp::write::Dialogs", "Landscape"), marginsPage);
    portrait->setObjectName(QStringLiteral("portrait"));
    landscape->setObjectName(QStringLiteral("landscape"));
    (before.landscape ? landscape : portrait)->setChecked(true);
    auto *orientation = new jp::Section(QCoreApplication::translate("jp::write::Dialogs", "Orientation"), marginsPage);
    auto *ov = new QVBoxLayout(orientation);
    ov->addWidget(portrait);
    ov->addWidget(landscape);
    mf->addRow(orientation);
    // Multiple pages: Normal, or Book fold (the pages are printed in booklet order, two to a side), of all the pages in one booklet or of 4 to 40 pages each.
    auto *multiple = new QComboBox(marginsPage);
    multiple->setObjectName(QStringLiteral("multiplePages"));
    multiple->addItem(QCoreApplication::translate("jp::write::Dialogs", "Normal"), false);
    multiple->addItem(QCoreApplication::translate("jp::write::Dialogs", "Book fold"), true);
    multiple->setCurrentIndex(doc->bookFold() ? 1 : 0);
    mf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Multiple pages"), multiple);
    auto *booklet = new QComboBox(marginsPage);
    booklet->setObjectName(QStringLiteral("sheetsPerBooklet"));
    booklet->addItem(QCoreApplication::translate("jp::write::Dialogs", "All"), 0);
    for (int n = 4; n <= 40; n += 4) booklet->addItem(QString::number(n), n);
    booklet->setCurrentIndex(std::max(0, int(booklet->findData(doc->bookFoldSheets()))));
    booklet->setEnabled(doc->bookFold());
    mf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Sheets per booklet"), booklet);
    // (book fold is facing pages: the side margins are the inside, where the gutter is, and the outside)
    auto leftLabel = mf->labelForField(left), rightLabel = mf->labelForField(right);
    auto facing = [=] {
        const bool fold = multiple->currentData().toBool();
        booklet->setEnabled(fold);
        if (auto *l = qobject_cast<QLabel *>(leftLabel)) l->setText(fold ? QCoreApplication::translate("jp::write::Dialogs", "Inside") : QCoreApplication::translate("jp::write::Dialogs", "Left"));
        if (auto *r = qobject_cast<QLabel *>(rightLabel)) r->setText(fold ? QCoreApplication::translate("jp::write::Dialogs", "Outside") : QCoreApplication::translate("jp::write::Dialogs", "Right"));
    };
    QObject::connect(multiple, &QComboBox::currentIndexChanged, &dlg, facing);
    facing();
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
    pf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Paper size"), size);
    pf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Width"), width);
    pf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Height"), height);
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
    lf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Section start"), start);
    auto *firstPage = new QCheckBox(QCoreApplication::translate("jp::write::Dialogs", "Different first page"), layoutPage), *oddEven = new QCheckBox(QCoreApplication::translate("jp::write::Dialogs", "Different odd and even pages"), layoutPage);
    firstPage->setObjectName(QStringLiteral("differentFirstPage"));
    oddEven->setObjectName(QStringLiteral("differentOddEven"));
    firstPage->setChecked(before.differentFirstPage);
    oddEven->setChecked(before.differentOddEven);
    lf->addRow(firstPage);
    lf->addRow(oddEven);
    auto *header = spin("header", before.header), *footer = spin("footer", before.footer);
    lf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Header, from the edge"), header);
    lf->addRow(QCoreApplication::translate("jp::write::Dialogs", "Footer, from the edge"), footer);
    tabs->addTab(layoutPage, QCoreApplication::translate("jp::write::Dialogs", "Layout"));

    // ---- Apply to
    auto *applyTo = new QComboBox(&dlg);
    applyTo->setObjectName(QStringLiteral("applyTo"));
    applyTo->addItem(QCoreApplication::translate("jp::write::Dialogs", "This section"), int(PageSetupScope::ThisSection));
    applyTo->addItem(QCoreApplication::translate("jp::write::Dialogs", "Whole document"), int(PageSetupScope::WholeDocument));
    if (cursor) applyTo->addItem(QCoreApplication::translate("jp::write::Dialogs", "This point forward"), int(PageSetupScope::ThisPointForward));
    if (doc->sectionCount() == 1) applyTo->setCurrentIndex(1);
    auto *row = new QFormLayout;
    row->addRow(QCoreApplication::translate("jp::write::Dialogs", "Apply to"), applyTo);
    v->addLayout(row);
    auto *buttons = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    auto *setDefault = buttons->addButton(QCoreApplication::translate("jp::write::Dialogs", "Set as Default…"), QDialogButtonBox::ResetRole);
    setDefault->setObjectName(QStringLiteral("setAsDefault"));
    DefaultScope makeDefault = DefaultScope::Cancel;
    QObject::connect(setDefault, &QPushButton::clicked, &dlg, [&] {
        makeDefault = askDefaultScope(&dlg, QCoreApplication::translate("jp::write::Dialogs", "the margins and paper size chosen here"));
        if (makeDefault != DefaultScope::Cancel) dlg.accept();
    });
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
    // (book fold is the document's, whatever the margins apply to)
    doc->setBookFold(multiple->currentData().toBool(), booklet->currentData().toInt());
    // Set as Default: the margins and the paper (not how a section starts, or its headers) go to the Normal template, which new documents start from.
    if (makeDefault == DefaultScope::AllNew)
        setNormalTemplate(&dlg, [after, fold = multiple->currentData().toBool(), perBooklet = booklet->currentData().toInt()](WriteDoc &target) {
            target.setBookFold(fold, perBooklet);
            SectionSetup now = pageSetupOf(target.sectionProps(0), target.differentOddEven());
            SectionSetup chosen = now;
            chosen.paperWidth = after.paperWidth;
            chosen.paperHeight = after.paperHeight;
            chosen.landscape = after.landscape;
            chosen.top = after.top;
            chosen.bottom = after.bottom;
            chosen.left = after.left;
            chosen.right = after.right;
            chosen.gutter = after.gutter;
            applyPageSetup(target, 0, PageSetupScope::WholeDocument, now, chosen);
        });
    const PageSetupScope scope = makeDefault == DefaultScope::ThisDocument ? PageSetupScope::WholeDocument : PageSetupScope(applyTo->currentData().toInt());
    if (scope == PageSetupScope::ThisPointForward && cursor) return applyPageSetupFromCursor(*doc, *cursor, before, after) > 0;
    return applyPageSetup(*doc, section, scope, before, after) > 0;
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
    auto *buttons = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return false;
    // (One line: a line break in an attribute is a blank.)
    *alt = cleanAltText(edit->toPlainText().simplified());
    return true;
}

} // namespace jp::write
