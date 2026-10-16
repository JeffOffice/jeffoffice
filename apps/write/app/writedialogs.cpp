#include "app/writedialogs.h"

#include "app/pageview.h"
#include "app/widgets.h"
#include "core/editops.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QTextCursor>
#include <QVBoxLayout>

namespace jp::write {

namespace {
QString tr(const char *text) { return QCoreApplication::translate("jp::write::Dialogs", text); }
}

void paragraphDialog(QWidget *parent, WriteDoc *doc, QTextCursor *cursor)
{
    if (!doc || !cursor) return;
    const QTextBlockFormat start = cursor->block().blockFormat();
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("paragraphDialog"));
    dlg.setWindowTitle(tr("Paragraph"));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;
    v->addLayout(form);

    auto *align = new QComboBox(&dlg);
    align->setObjectName(QStringLiteral("alignment"));
    align->addItem(tr("Left"), int(Qt::AlignLeft));
    align->addItem(tr("Centered"), int(Qt::AlignHCenter));
    align->addItem(tr("Right"), int(Qt::AlignRight));
    align->addItem(tr("Justified"), int(Qt::AlignJustify));
    const Qt::Alignment al = start.alignment() & Qt::AlignHorizontal_Mask;
    align->setCurrentIndex(al == Qt::AlignHCenter ? 1 : (al == Qt::AlignRight || al == Qt::AlignTrailing) ? 2 : al == Qt::AlignJustify ? 3 : 0);
    form->addRow(tr("Alignment:"), align);

    auto *left = new MeasureSpin(&dlg), *right = new MeasureSpin(&dlg), *by = new MeasureSpin(&dlg);
    left->setObjectName(QStringLiteral("left"));
    right->setObjectName(QStringLiteral("right"));
    by->setObjectName(QStringLiteral("by"));
    left->setPoints(start.leftMargin());
    right->setPoints(start.rightMargin());
    auto *special = new QComboBox(&dlg);
    special->setObjectName(QStringLiteral("special"));
    special->addItems({tr("(none)"), tr("First line"), tr("Hanging")});
    special->setCurrentIndex(start.textIndent() > 0.01 ? 1 : start.textIndent() < -0.01 ? 2 : 0);
    by->setPoints(std::abs(start.textIndent()));
    form->addRow(tr("Indentation, left:"), left);
    form->addRow(tr("Indentation, right:"), right);
    auto *row = new QHBoxLayout;
    row->addWidget(special);
    row->addWidget(by);
    form->addRow(tr("Special:"), row);

    auto *before = new MeasureSpin(&dlg), *after = new MeasureSpin(&dlg), *at = new MeasureSpin(&dlg);
    before->setObjectName(QStringLiteral("before"));
    after->setObjectName(QStringLiteral("after"));
    at->setObjectName(QStringLiteral("at"));
    before->setPoints(start.topMargin());
    after->setPoints(start.bottomMargin());
    form->addRow(tr("Spacing, before:"), before);
    form->addRow(tr("Spacing, after:"), after);
    auto *lineType = new QComboBox(&dlg);
    lineType->setObjectName(QStringLiteral("lineSpacing"));
    lineType->addItems({tr("Single"), tr("1.5 lines"), tr("Double"), tr("At least"), tr("Exactly"), tr("Multiple")});
    auto *multiple = new QDoubleSpinBox(&dlg);
    multiple->setObjectName(QStringLiteral("multiple"));
    multiple->setRange(0.1, 20);
    multiple->setDecimals(2);
    multiple->setSingleStep(0.05);
    int lineKind = 0;
    double lineValue = 1;
    if (start.lineHeightType() == QTextBlockFormat::FixedHeight) {
        lineKind = 4;
        lineValue = start.lineHeight();
    } else if (start.lineHeightType() == QTextBlockFormat::MinimumHeight) {
        lineKind = 3;
        lineValue = start.lineHeight();
    } else if (start.lineHeightType() == QTextBlockFormat::ProportionalHeight) {
        const double m = start.lineHeight() / 100.0;
        lineValue = m;
        lineKind = std::abs(m - 1) < 0.01 ? 0 : std::abs(m - 1.5) < 0.01 ? 1 : std::abs(m - 2) < 0.01 ? 2 : 5;
    }
    lineType->setCurrentIndex(lineKind);
    multiple->setValue(lineKind >= 3 && lineKind < 5 ? 1.0 : lineValue);
    at->setPoints(lineKind == 3 || lineKind == 4 ? lineValue : 12);
    auto *lineRow = new QHBoxLayout;
    lineRow->addWidget(lineType);
    lineRow->addWidget(at);
    lineRow->addWidget(multiple);
    form->addRow(tr("Line spacing:"), lineRow);
    auto show = [=] {
        const int k = lineType->currentIndex();
        at->setVisible(k == 3 || k == 4);
        multiple->setVisible(k == 5);
        by->setEnabled(special->currentIndex() != 0);
    };
    show();
    QObject::connect(lineType, &QComboBox::currentIndexChanged, &dlg, show);
    QObject::connect(special, &QComboBox::currentIndexChanged, &dlg, show);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return;

    // Only what the person changed goes to the paragraphs.
    const double firstNow = special->currentIndex() == 0 ? 0 : special->currentIndex() == 1 ? by->value() : -by->value();
    const bool alignChanged = align->currentIndex() != (al == Qt::AlignHCenter ? 1 : (al == Qt::AlignRight || al == Qt::AlignTrailing) ? 2 : al == Qt::AlignJustify ? 3 : 0);
    const bool leftChanged = std::abs(left->value() - start.leftMargin()) > 0.001, rightChanged = std::abs(right->value() - start.rightMargin()) > 0.001;
    const bool firstChanged = std::abs(firstNow - start.textIndent()) > 0.001;
    const bool beforeChanged = std::abs(before->value() - start.topMargin()) > 0.001, afterChanged = std::abs(after->value() - start.bottomMargin()) > 0.001;
    const int k = lineType->currentIndex();
    const bool lineChanged = k != lineKind || (k == 5 && std::abs(multiple->value() - lineValue) > 0.001) || ((k == 3 || k == 4) && std::abs(at->value() - lineValue) > 0.001);
    const Qt::Alignment chosen = Qt::Alignment(align->currentData().toInt());
    changeBlockFormat(*doc, *cursor, [&](QTextBlockFormat &bf) {
        if (alignChanged) bf.setAlignment(chosen);
        if (leftChanged) bf.setLeftMargin(left->value());
        if (rightChanged) bf.setRightMargin(right->value());
        if (firstChanged) bf.setTextIndent(firstNow);
        if (beforeChanged) bf.setTopMargin(before->value());
        if (afterChanged) bf.setBottomMargin(after->value());
        if (lineChanged) {
            if (k == 0) bf.setLineHeight(100, QTextBlockFormat::ProportionalHeight);
            else if (k == 1) bf.setLineHeight(150, QTextBlockFormat::ProportionalHeight);
            else if (k == 2) bf.setLineHeight(200, QTextBlockFormat::ProportionalHeight);
            else if (k == 3) bf.setLineHeight(at->value(), QTextBlockFormat::MinimumHeight);
            else if (k == 4) bf.setLineHeight(at->value(), QTextBlockFormat::FixedHeight);
            else bf.setLineHeight(multiple->value() * 100, QTextBlockFormat::ProportionalHeight);
        }
    });
}

void zoomDialog(QWidget *parent, PageView *view)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("zoomDialog"));
    dlg.setWindowTitle(tr("Zoom"));
    auto *v = new QVBoxLayout(&dlg);
    auto *box = new QGroupBox(tr("Zoom to"), &dlg);
    auto *bv = new QVBoxLayout(box);
    auto *group = new QButtonGroup(&dlg);
    struct Choice { const char *name; int id; };
    QVector<QRadioButton *> radios;
    const Choice choices[] = {{"200%", 200}, {"100%", 100}, {"75%", 75}, {"Page width", -1}, {"Whole page", -2}, {"Many pages", -3}, {"Percent:", 0}};
    for (const Choice &c : choices) {
        auto *r = new QRadioButton(QCoreApplication::translate("jp::write::Dialogs", c.name), box);
        r->setProperty("zoomChoice", c.id);
        group->addButton(r);
        bv->addWidget(r);
        radios << r;
    }
    auto *percent = new QSpinBox(box);
    percent->setObjectName(QStringLiteral("percent"));
    percent->setRange(10, 500);
    percent->setSuffix(QStringLiteral("%"));
    percent->setValue(std::clamp(int(std::lround(view->zoom() * 100)), 10, 500));
    bv->addWidget(percent);
    auto *across = new QSpinBox(box);
    across->setObjectName(QStringLiteral("across"));
    across->setRange(1, 4);
    across->setValue(std::max(2, view->pagesAcross()));
    auto *acrossRow = new QHBoxLayout;
    acrossRow->addWidget(new QLabel(tr("Pages across:"), box));
    acrossRow->addWidget(across);
    bv->addLayout(acrossRow);
    v->addWidget(box);
    // The choice that fits now.
    int now = 0;
    if (view->zoomMode() == PageView::ZoomMode::PageWidth) now = -1;
    else if (view->zoomMode() == PageView::ZoomMode::OnePage) now = -2;
    else if (view->zoomMode() == PageView::ZoomMode::ManyPages) now = -3;
    else if (std::abs(view->zoom() - 2) < 0.005) now = 200;
    else if (std::abs(view->zoom() - 1) < 0.005) now = 100;
    else if (std::abs(view->zoom() - 0.75) < 0.005) now = 75;
    for (QRadioButton *r : std::as_const(radios))
        if (r->property("zoomChoice").toInt() == now) r->setChecked(true);
    auto sync = [=] {
        const int c = group->checkedButton() ? group->checkedButton()->property("zoomChoice").toInt() : 0;
        percent->setEnabled(c == 0);
        across->setEnabled(c == -3);
    };
    sync();
    QObject::connect(group, &QButtonGroup::buttonToggled, &dlg, sync);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted || !group->checkedButton()) return;
    const int c = group->checkedButton()->property("zoomChoice").toInt();
    if (c == -1) view->setZoomMode(PageView::ZoomMode::PageWidth);
    else if (c == -2) view->setZoomMode(PageView::ZoomMode::OnePage);
    else if (c == -3) view->setZoomMode(PageView::ZoomMode::ManyPages, across->value(), 1);
    else if (c == 0) view->setZoom(percent->value() / 100.0);
    else view->setZoom(c / 100.0);
}

} // namespace jp::write
