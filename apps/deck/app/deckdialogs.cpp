#include "app/deckdialogs.h"

#include "app/widgets.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace jp {

namespace {
// The row of buttons that end the Apply and Apply to All dialogs.
QDialogButtonBox *applyButtons(QDialog *dlg, bool *all)
{
    auto *box = new QDialogButtonBox(dlg);
    QPushButton *apply = box->addButton(QCoreApplication::translate("DeckDialogs", "Apply"), QDialogButtonBox::AcceptRole);
    QPushButton *applyAll = box->addButton(QCoreApplication::translate("DeckDialogs", "Apply to All"), QDialogButtonBox::AcceptRole);
    box->addButton(QDialogButtonBox::Cancel);
    apply->setObjectName(QStringLiteral("apply"));
    applyAll->setObjectName(QStringLiteral("applyToAll"));
    apply->setDefault(true);
    QObject::connect(apply, &QPushButton::clicked, dlg, [dlg, all] { *all = false; dlg->accept(); });
    QObject::connect(applyAll, &QPushButton::clicked, dlg, [dlg, all] { *all = true; dlg->accept(); });
    QObject::connect(box, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
    return box;
}
} // namespace

HeaderFooterDialog::HeaderFooterDialog(const deck::FooterOptions &o, QWidget *parent) : QDialog(parent), m_start(o)
{
    setWindowTitle(tr("Header & Footer"));
    setObjectName(QStringLiteral("headerFooterDialog"));
    auto *lay = new QVBoxLayout(this);
    lay->addWidget(new QLabel(tr("Include on slide")));
    m_date = new QCheckBox(tr("Date and time"));
    m_date->setObjectName(QStringLiteral("date"));
    m_number = new QCheckBox(tr("Slide number"));
    m_number->setObjectName(QStringLiteral("slideNumber"));
    m_footer = new QCheckBox(tr("Footer"));
    m_footer->setObjectName(QStringLiteral("footer"));
    m_text = new QLineEdit(o.footerText);
    m_text->setObjectName(QStringLiteral("footerText"));
    m_text->setAccessibleName(tr("Footer text"));
    m_text->setMinimumWidth(280);
    m_skipTitle = new QCheckBox(tr("Don't show on title slide"));
    m_skipTitle->setObjectName(QStringLiteral("skipTitle"));
    m_date->setChecked(o.date);
    m_number->setChecked(o.slideNumber);
    m_footer->setChecked(o.footer);
    m_skipTitle->setChecked(o.skipTitleSlide);
    m_text->setEnabled(o.footer);
    connect(m_footer, &QCheckBox::toggled, m_text, &QWidget::setEnabled);
    lay->addWidget(m_date);
    lay->addWidget(m_number);
    auto *footerRow = new QHBoxLayout();
    footerRow->addWidget(m_footer);
    footerRow->addWidget(m_text, 1);
    lay->addLayout(footerRow);
    lay->addSpacing(6);
    lay->addWidget(m_skipTitle);
    lay->addWidget(applyButtons(this, &m_all));
}

deck::FooterOptions HeaderFooterDialog::options() const
{
    deck::FooterOptions o = m_start;
    o.date = m_date->isChecked();
    o.slideNumber = m_number->isChecked();
    o.footer = m_footer->isChecked();
    o.footerText = m_text->text();
    o.skipTitleSlide = m_skipTitle->isChecked();
    return o;
}

BackgroundDialog::BackgroundDialog(const QColor &current, const ColorScheme &scheme, QWidget *parent) : QDialog(parent), m_color(current), m_scheme(scheme)
{
    setWindowTitle(tr("Format Background"));
    setObjectName(QStringLiteral("backgroundDialog"));
    auto *lay = new QVBoxLayout(this);
    auto *form = new QFormLayout();
    m_button = new ColorButton(QStringLiteral("paint-bucket"), tr("Background Color"), false, QString());
    m_button->setScheme(scheme);
    m_button->setCurrent(ColorRef::rgb(current));
    m_button->setWell(true);
    m_button->setObjectName(QStringLiteral("backgroundColor"));
    form->addRow(tr("Solid fill color:"), m_button);
    lay->addLayout(form);
    m_hide = new QCheckBox(tr("Hide background graphics"));
    m_hide->setObjectName(QStringLiteral("hideGraphics"));
    lay->addWidget(m_hide);
    connect(m_button, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
        m_color = c.resolve(m_scheme);
        m_button->setCurrent(ColorRef::rgb(m_color));
    });
    lay->addSpacing(6);
    lay->addWidget(applyButtons(this, &m_all));
}

void BackgroundDialog::setHideGraphics(bool hide) { m_hide->setChecked(hide); }
bool BackgroundDialog::hideGraphics() const { return m_hide->isChecked(); }

TableSizeDialog::TableSizeDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Insert Table"));
    auto *lay = new QVBoxLayout(this);
    auto *form = new QFormLayout();
    m_cols = new QSpinBox();
    m_cols->setRange(1, 30);
    m_cols->setValue(5);
    m_rows = new QSpinBox();
    m_rows->setRange(1, 100);
    m_rows->setValue(2);
    form->addRow(tr("Number of &columns:"), m_cols);
    form->addRow(tr("Number of &rows:"), m_rows);
    lay->addLayout(form);
    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    lay->addWidget(box);
}

int TableSizeDialog::rows() const { return m_rows->value(); }
int TableSizeDialog::columns() const { return m_cols->value(); }

} // namespace jp
