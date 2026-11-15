#include "app/deckdialogs.h"
#include "app/uistyle.h"

#include "app/widgets.h"
#include "office/hyperlink.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
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
    auto *box = new jp::ButtonBox(dlg);
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

PrintHeaderFooterDialog::PrintHeaderFooterDialog(const deck::PrintHeaderFooter &start, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Header & Footer"));
    setObjectName(QStringLiteral("printHeaderFooterDialog"));
    auto *lay = new QVBoxLayout(this);
    lay->addWidget(new QLabel(tr("Include on notes pages and handouts")));
    auto *form = new QFormLayout();
    auto row = [&](QCheckBox *&box, QLineEdit *&text, const char *boxName, const char *textName, const QString &label, const QString &about, bool on, const QString &typed) {
        box = new QCheckBox(label, this);
        box->setObjectName(QString::fromLatin1(boxName));
        box->setChecked(on);
        text = new QLineEdit(typed, this);
        text->setObjectName(QString::fromLatin1(textName));
        text->setAccessibleName(about);
        text->setMinimumWidth(280);
        text->setEnabled(on);
        connect(box, &QCheckBox::toggled, text, &QWidget::setEnabled);
        form->addRow(box, text);
    };
    row(m_date, m_dateText, "hfDate", "hfDateText", tr("Date"), tr("Date text"), start.date, start.dateText);
    m_dateText->setPlaceholderText(tr("Today's date, when this is empty"));
    row(m_header, m_headerText, "hfHeader", "hfHeaderText", tr("Header"), tr("Header text"), start.header, start.headerText);
    row(m_footer, m_footerText, "hfFooter", "hfFooterText", tr("Footer"), tr("Footer text"), start.footer, start.footerText);
    lay->addLayout(form);
    m_number = new QCheckBox(tr("Page number"), this);
    m_number->setObjectName(QStringLiteral("hfPageNumber"));
    m_number->setChecked(start.pageNumber);
    lay->addWidget(m_number);
    auto *buttons = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    lay->addWidget(buttons);
}

deck::PrintHeaderFooter PrintHeaderFooterDialog::choice() const
{
    deck::PrintHeaderFooter hf;
    hf.date = m_date->isChecked();
    hf.dateText = m_dateText->text();
    hf.header = m_header->isChecked();
    hf.headerText = m_headerText->text();
    hf.footer = m_footer->isChecked();
    hf.footerText = m_footerText->text();
    hf.pageNumber = m_number->isChecked();
    return hf;
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

BackgroundDialog::BackgroundDialog(const QColor &current, const ColorScheme &scheme, QWidget *parent)
    : BackgroundDialog(Fill::solid(ColorRef::rgb(current)), scheme, {}, {}, parent)
{
}

BackgroundDialog::BackgroundDialog(const Fill &current, const ColorScheme &scheme, const ImageLookup &images, const std::function<QString()> &pickPicture, QWidget *parent)
    : QDialog(parent), m_fill(current), m_pick(pickPicture)
{
    setWindowTitle(tr("Format Background"));
    setObjectName(QStringLiteral("backgroundDialog"));
    auto *lay = new QVBoxLayout(this);
    m_editor = new FillEditor(this);
    m_editor->setObjectName(QStringLiteral("backgroundFill"));
    m_editor->setScheme(scheme);
    m_editor->setImageProvider(images);
    m_editor->setFills({m_fill});
    lay->addWidget(m_editor);
    m_hide = new QCheckBox(tr("Hide background graphics"));
    m_hide->setObjectName(QStringLiteral("hideGraphics"));
    lay->addWidget(m_hide);
    connect(m_editor, &FillEditor::changed, this, [this](const FillChange &change) {
        change(m_fill);
        m_editor->setFills({m_fill});
    });
    connect(m_editor, &FillEditor::pictureRequested, this, [this] {
        const QString id = m_pick ? m_pick() : QString();
        if (id.isEmpty()) return;
        const double t = m_fill.transparency;
        if (m_fill.type != Fill::Picture && m_fill.type != Fill::Texture) m_fill = Fill();
        m_fill.type = m_fill.type == Fill::Texture ? Fill::Texture : Fill::Picture;
        m_fill.imageId = id;
        m_fill.transparency = t;
        m_fill.blipEffects.clear();
        m_editor->setFills({m_fill});
    });
    lay->addSpacing(6);
    QDialogButtonBox *buttons = applyButtons(this, &m_all);
    QPushButton *reset = buttons->addButton(tr("Reset Background"), QDialogButtonBox::ResetRole);
    reset->setObjectName(QStringLiteral("resetBackground"));
    reset->setAutoDefault(false);
    connect(reset, &QPushButton::clicked, this, [this] {
        m_reset = true;
        accept();
    });
    lay->addWidget(buttons);
}

void BackgroundDialog::setHideGraphics(bool hide) { m_hide->setChecked(hide); }
bool BackgroundDialog::hideGraphics() const { return m_hide->isChecked(); }

LinkDialog::LinkDialog(const QString &current, int slides, const QVector<QPair<int, QString>> &shows, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Insert Link"));
    setObjectName(QStringLiteral("linkDialog"));
    auto *lay = new QVBoxLayout(this);
    auto *form = new QFormLayout();
    m_kind = new QComboBox(this);
    m_kind->setObjectName(QStringLiteral("linkKind"));
    m_kind->addItems({tr("Web page"), tr("Email address"), tr("Place in this presentation"), tr("File"), tr("Another presentation")});
    m_display = new QLineEdit(this);
    m_display->setObjectName(QStringLiteral("linkText"));
    m_address = new QLineEdit(this);
    m_address->setObjectName(QStringLiteral("linkAddress"));
    m_place = new QComboBox(this);
    m_place->setObjectName(QStringLiteral("linkPlace"));
    m_place->addItem(tr("Next slide"), QStringLiteral("#next"));
    m_place->addItem(tr("Previous slide"), QStringLiteral("#previous"));
    m_place->addItem(tr("First slide"), QStringLiteral("#first"));
    m_place->addItem(tr("Last slide"), QStringLiteral("#last"));
    m_place->addItem(tr("Last slide viewed"), QStringLiteral("#lastviewed"));
    for (int i = 1; i <= slides; ++i) m_place->addItem(tr("Slide %1").arg(i), office::slideLink(i));
    for (const auto &show : shows) {
        m_place->addItem(tr("Custom show: %1").arg(show.second), office::customShowLink(show.first, false));
        m_place->addItem(tr("Custom show, then back: %1").arg(show.second), office::customShowLink(show.first, true));
    }
    m_place->addItem(tr("End show"), QStringLiteral("#end"));
    form->addRow(tr("Link to"), m_kind);
    m_displayLabel = new QLabel(tr("Text to display"), this);
    form->addRow(m_displayLabel, m_display);
    auto *addressRow = new QWidget(this);
    auto *addressLay = new QHBoxLayout(addressRow);
    addressLay->setContentsMargins(0, 0, 0, 0);
    addressLay->addWidget(m_address);
    auto *browse = new QPushButton(tr("Browse…"), addressRow);
    addressLay->addWidget(browse);
    form->addRow(tr("Address"), addressRow);
    if (auto *label = qobject_cast<QLabel *>(form->labelForField(addressRow))) label->setBuddy(m_address);
    form->addRow(tr("Place"), m_place);
    lay->addLayout(form);
    setDisplayShown(QString(), false);
    const office::LinkTarget now = office::parseLink(current);
    switch (now.kind) {
    case office::LinkKind::Mail: m_kind->setCurrentIndex(1); m_address->setText(now.address.mid(7)); break;
    case office::LinkKind::File: m_kind->setCurrentIndex(3); m_address->setText(now.address); break;
    case office::LinkKind::Deck: m_kind->setCurrentIndex(4); m_address->setText(now.address); break;
    case office::LinkKind::None:
        if (!current.trimmed().isEmpty()) m_address->setText(current.trimmed());
        break;
    case office::LinkKind::Web: m_address->setText(now.address); break;
    default:
        m_kind->setCurrentIndex(2);
        if (const int at = m_place->findData(now.kind == office::LinkKind::Slide ? office::slideLink(now.slide) : now.kind == office::LinkKind::CustomShow ? office::customShowLink(now.slide, now.back) : current.trimmed().toLower()); at >= 0)
            m_place->setCurrentIndex(at);
        break;
    }
    auto refresh = [this, browse] {
        const int k = m_kind->currentIndex();
        m_place->setEnabled(k == 2);
        m_address->setEnabled(k != 2);
        browse->setEnabled(k == 3 || k == 4);
        m_address->setPlaceholderText(k == 1 ? tr("name@example.com") : k == 3 ? tr("Path of the file") : k == 4 ? tr("Path of the presentation") : QStringLiteral("https://"));
    };
    connect(m_kind, &QComboBox::currentIndexChanged, this, refresh);
    refresh();
    // (A file chosen here is a path on this computer; one typed may be anywhere, and is never looked for.)
    connect(browse, &QPushButton::clicked, this, [this] {
        const QString path = QFileDialog::getOpenFileName(this, tr("Link to File"));
        if (!path.isEmpty()) m_address->setText(path);
    });
    auto *box = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    auto *remove = box->addButton(tr("Remove Link"), QDialogButtonBox::ResetRole);
    connect(remove, &QPushButton::clicked, this, [this] {
        m_removed = true;
        accept();
    });
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    lay->addWidget(box);
}

QString LinkDialog::link() const
{
    const int k = m_kind->currentIndex();
    if (k == 2) return m_place->currentData().toString();
    QString text = m_address->text().trimmed();
    if (text.isEmpty()) return QString();
    if (k == 1) return QStringLiteral("mailto:") + text.remove(QStringLiteral("mailto:"), Qt::CaseInsensitive);
    if (k == 4) return office::presentationLink(text);
    // A web address typed without its scheme is one; a file is as typed, and is never looked for.
    if (k == 0 && !text.contains(QLatin1String("://"))) return QStringLiteral("https://") + text;
    return text;
}

QString LinkDialog::displayText() const { return m_display->text(); }

void LinkDialog::setDisplayShown(const QString &text, bool shown)
{
    m_display->setText(text);
    m_display->setVisible(shown);
    m_displayLabel->setVisible(shown);
}

constexpr double kLeastInches = 0.5, kMostInches = 56;

SlideSizeDialog::SlideSizeDialog(const QSizeF &current, bool notesPortrait, QWidget *parent) : QDialog(parent), m_was(current)
{
    setWindowTitle(tr("Slide Size"));
    setObjectName(QStringLiteral("slideSizeDialog"));
    auto *lay = new QVBoxLayout(this);
    auto *form = new QFormLayout();
    m_preset = new QComboBox(this);
    m_preset->setObjectName(QStringLiteral("slideSizePreset"));
    for (const deck::SlideSizePreset &p : deck::slideSizePresets()) {
        const QString text = p.name == QLatin1String("Standard") ? tr("Standard (4:3)") : p.name == QLatin1String("Widescreen") ? tr("Widescreen (16:9)") : p.name == QLatin1String("Banner") ? tr("Banner")
                           : p.name == QLatin1String("35mm Slides") ? tr("35mm Slides") : p.name.startsWith(QLatin1String("On-screen")) ? tr("On-screen Show (16:10)")
                           : p.name == QLatin1String("Letter Paper") ? tr("Letter Paper") : p.name == QLatin1String("Ledger Paper") ? tr("Ledger Paper") : p.name == QLatin1String("A3 Paper") ? tr("A3 Paper")
                           : p.name == QLatin1String("A4 Paper") ? tr("A4 Paper") : p.name == QLatin1String("B4 (ISO) Paper") ? tr("B4 (ISO) Paper") : tr("B5 (ISO) Paper");
        m_preset->addItem(text, p.key);
    }
    m_preset->addItem(tr("Custom"), QString());
    m_width = new QDoubleSpinBox(this);
    m_width->setObjectName(QStringLiteral("slideWidth"));
    m_height = new QDoubleSpinBox(this);
    m_height->setObjectName(QStringLiteral("slideHeight"));
    for (QDoubleSpinBox *b : {m_width, m_height}) {
        b->setRange(kLeastInches, kMostInches);
        b->setDecimals(3);
        b->setSuffix(tr(" in"));
    }
    m_landscape = new QRadioButton(tr("Landscape"), this);
    m_landscape->setObjectName(QStringLiteral("slideLandscape"));
    m_portrait = new QRadioButton(tr("Portrait"), this);
    m_portrait->setObjectName(QStringLiteral("slidePortrait"));
    form->addRow(tr("Slides sized for"), m_preset);
    form->addRow(tr("Width"), m_width);
    form->addRow(tr("Height"), m_height);
    auto *orient = new QHBoxLayout();
    orient->addWidget(m_landscape);
    orient->addWidget(m_portrait);
    form->addRow(tr("Orientation"), orient);
    m_notesPortrait = new QRadioButton(tr("Portrait"), this);
    m_notesPortrait->setObjectName(QStringLiteral("notesPortrait"));
    m_notesLandscape = new QRadioButton(tr("Landscape"), this);
    m_notesLandscape->setObjectName(QStringLiteral("notesLandscape"));
    (notesPortrait ? m_notesPortrait : m_notesLandscape)->setChecked(true);
    auto *notes = new QHBoxLayout();
    notes->addWidget(m_notesPortrait);
    notes->addWidget(m_notesLandscape);
    form->addRow(tr("Notes pages"), notes);
    // Each pair is a choice of its own.
    auto *slidesGroup = new QButtonGroup(this);
    slidesGroup->addButton(m_landscape);
    slidesGroup->addButton(m_portrait);
    auto *notesGroup = new QButtonGroup(this);
    notesGroup->addButton(m_notesPortrait);
    notesGroup->addButton(m_notesLandscape);
    lay->addLayout(form);
    m_loading = true;
    m_width->setValue(current.width() / 72);
    m_height->setValue(current.height() / 72);
    m_portrait->setChecked(current.height() > current.width());
    m_landscape->setChecked(!m_portrait->isChecked());
    // The list shows the preset the size is, in either orientation.
    int at = m_preset->count() - 1;
    for (int i = 0; i < deck::slideSizePresets().size(); ++i) {
        const QSizeF p = deck::slideSizePresets()[i].size;
        if ((std::abs(p.width() - current.width()) < 0.5 && std::abs(p.height() - current.height()) < 0.5) || (std::abs(p.height() - current.width()) < 0.5 && std::abs(p.width() - current.height()) < 0.5)) at = i;
    }
    m_preset->setCurrentIndex(at);
    m_loading = false;
    connect(m_preset, &QComboBox::currentIndexChanged, this, [this](int i) { showPreset(i); });
    connect(m_portrait, &QRadioButton::toggled, this, [this](bool on) { orientationChosen(on); });
    auto custom = [this] {
        if (!m_loading) m_preset->setCurrentIndex(m_preset->count() - 1);
    };
    connect(m_width, &QDoubleSpinBox::editingFinished, this, custom);
    connect(m_height, &QDoubleSpinBox::editingFinished, this, custom);
    auto *box = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    lay->addWidget(box);
}

void SlideSizeDialog::showPreset(int index)
{
    if (m_loading || index < 0 || index >= deck::slideSizePresets().size()) return;
    QSizeF s = deck::slideSizePresets()[index].size;
    if (m_portrait->isChecked()) s = s.transposed();
    m_loading = true;
    m_width->setValue(s.width() / 72);
    m_height->setValue(s.height() / 72);
    m_loading = false;
}

// Turning the slide turns the size (the longer side goes across for landscape, up for portrait).
void SlideSizeDialog::orientationChosen(bool portrait)
{
    if (m_loading) return;
    const double w = m_width->value(), h = m_height->value();
    if ((portrait && w > h) || (!portrait && h > w)) {
        m_loading = true;
        m_width->setValue(h);
        m_height->setValue(w);
        m_loading = false;
    }
}

// A width or height left as it was is the size it was, not the rounded one the box shows (13.333 inches is not 960 points).
QSizeF SlideSizeDialog::size() const
{
    auto side = [](const QDoubleSpinBox *box, double was) { return std::abs(box->value() * 72 - was) < 0.05 ? was : box->value() * 72; };
    return QSizeF(side(m_width, m_was.width()), side(m_height, m_was.height()));
}
bool SlideSizeDialog::isPortrait() const { return m_portrait->isChecked(); }

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
    form->addRow(tr("Number of &columns"), m_cols);
    form->addRow(tr("Number of &rows"), m_rows);
    lay->addLayout(form);
    auto *box = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    lay->addWidget(box);
}

int TableSizeDialog::rows() const { return m_rows->value(); }
int TableSizeDialog::columns() const { return m_cols->value(); }

} // namespace jp
