#include "app/formatdialog.h"

#include "app/icons.h"
#include "app/widgets.h"
#include "core/editsession.h"
#include "core/formatcodes.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace jp {

using namespace jp::office::sheet;
using jp::crunch::NumberCategory;
using jp::crunch::NumberOptions;

namespace {

// The edges of the border tab, in the order of the buttons.
enum Edge { Top, InsideH, Bottom, Left, InsideV, Right, DiagUp, DiagDown, EdgeCount };

constexpr int kEdgeBits[EdgeCount] = {jp::crunch::EditSession::EdgeTop,         jp::crunch::EditSession::EdgeInsideHorizontal, jp::crunch::EditSession::EdgeBottom,
                                      jp::crunch::EditSession::EdgeLeft,        jp::crunch::EditSession::EdgeInsideVertical,   jp::crunch::EditSession::EdgeRight,
                                      jp::crunch::EditSession::EdgeDiagonalUp, jp::crunch::EditSession::EdgeDiagonalDown};

const BorderStyle kLineStyles[] = {BorderStyle::None,   BorderStyle::Hair,    BorderStyle::Dotted,       BorderStyle::DashDotDot,       BorderStyle::DashDot,        BorderStyle::Dashed,
                                   BorderStyle::Thin,   BorderStyle::MediumDashDotDot, BorderStyle::SlantDashDot, BorderStyle::MediumDashDot, BorderStyle::MediumDashed, BorderStyle::Medium,
                                   BorderStyle::Double, BorderStyle::Thick};

const PatternKind kPatterns[] = {PatternKind::None,           PatternKind::Solid,          PatternKind::MediumGray,     PatternKind::DarkGray,       PatternKind::LightGray,
                                 PatternKind::DarkHorizontal, PatternKind::DarkVertical,   PatternKind::DarkDown,       PatternKind::DarkUp,         PatternKind::DarkGrid,
                                 PatternKind::DarkTrellis,    PatternKind::LightHorizontal, PatternKind::LightVertical, PatternKind::LightDown,      PatternKind::LightUp,
                                 PatternKind::LightGrid,      PatternKind::LightTrellis,   PatternKind::Gray125,       PatternKind::Gray0625};

const HAlign kHorizontal[] = {HAlign::General, HAlign::Left, HAlign::Center, HAlign::Right, HAlign::Fill, HAlign::Justify, HAlign::CenterContinuous, HAlign::Distributed};
const VAlign kVertical[] = {VAlign::Top, VAlign::Center, VAlign::Bottom, VAlign::Justify, VAlign::Distributed};

QString lineStyleName(BorderStyle s)
{
    switch (s) {
    case BorderStyle::None: return QCoreApplication::translate("jp::FormatCellsDialog", "None");
    case BorderStyle::Hair: return QCoreApplication::translate("jp::FormatCellsDialog", "Hairline");
    case BorderStyle::Dotted: return QCoreApplication::translate("jp::FormatCellsDialog", "Dotted");
    case BorderStyle::DashDotDot: return QCoreApplication::translate("jp::FormatCellsDialog", "Dash dot dot");
    case BorderStyle::DashDot: return QCoreApplication::translate("jp::FormatCellsDialog", "Dash dot");
    case BorderStyle::Dashed: return QCoreApplication::translate("jp::FormatCellsDialog", "Dashed");
    case BorderStyle::Thin: return QCoreApplication::translate("jp::FormatCellsDialog", "Thin");
    case BorderStyle::MediumDashDotDot: return QCoreApplication::translate("jp::FormatCellsDialog", "Medium dash dot dot");
    case BorderStyle::SlantDashDot: return QCoreApplication::translate("jp::FormatCellsDialog", "Slanted dash dot");
    case BorderStyle::MediumDashDot: return QCoreApplication::translate("jp::FormatCellsDialog", "Medium dash dot");
    case BorderStyle::MediumDashed: return QCoreApplication::translate("jp::FormatCellsDialog", "Medium dashed");
    case BorderStyle::Medium: return QCoreApplication::translate("jp::FormatCellsDialog", "Medium");
    case BorderStyle::Double: return QCoreApplication::translate("jp::FormatCellsDialog", "Double");
    case BorderStyle::Thick: return QCoreApplication::translate("jp::FormatCellsDialog", "Thick");
    }
    return QString();
}

// A pen that draws a border line in a style (a double line is two pens' worth, drawn by the caller).
QPen penOf(BorderStyle s, const QColor &color)
{
    QPen pen(color);
    pen.setCapStyle(Qt::FlatCap);
    switch (s) {
    case BorderStyle::Hair: pen.setWidthF(1); pen.setStyle(Qt::DotLine); break;
    case BorderStyle::Dotted: pen.setWidthF(1); pen.setDashPattern({1, 1}); break;
    case BorderStyle::DashDotDot: pen.setWidthF(1); pen.setStyle(Qt::DashDotDotLine); break;
    case BorderStyle::DashDot: pen.setWidthF(1); pen.setStyle(Qt::DashDotLine); break;
    case BorderStyle::Dashed: pen.setWidthF(1); pen.setStyle(Qt::DashLine); break;
    case BorderStyle::Thin: pen.setWidthF(1); break;
    case BorderStyle::MediumDashDotDot: pen.setWidthF(2); pen.setStyle(Qt::DashDotDotLine); break;
    case BorderStyle::SlantDashDot: pen.setWidthF(2); pen.setStyle(Qt::DashDotLine); break;
    case BorderStyle::MediumDashDot: pen.setWidthF(2); pen.setStyle(Qt::DashDotLine); break;
    case BorderStyle::MediumDashed: pen.setWidthF(2); pen.setStyle(Qt::DashLine); break;
    case BorderStyle::Medium: pen.setWidthF(2); break;
    case BorderStyle::Double: pen.setWidthF(1); break;
    case BorderStyle::Thick: pen.setWidthF(3); break;
    case BorderStyle::None: pen.setStyle(Qt::NoPen); break;
    }
    return pen;
}

void strokeLine(QPainter *p, const QPointF &a, const QPointF &b, BorderStyle style, const QColor &color)
{
    if (style == BorderStyle::None) return;
    if (style == BorderStyle::Double) {
        const bool horizontal = std::abs(a.y() - b.y()) < 0.5;
        const QPointF d = horizontal ? QPointF(0, 1.5) : QPointF(1.5, 0);
        p->setPen(penOf(style, color));
        p->drawLine(a - d, b - d);
        p->drawLine(a + d, b + d);
        return;
    }
    p->setPen(penOf(style, color));
    p->drawLine(a, b);
}

QIcon lineSwatch(BorderStyle style, const QColor &color)
{
    QPixmap pm(72, 14);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    strokeLine(&p, QPointF(2, 7), QPointF(70, 7), style, color);
    return QIcon(pm);
}

} // namespace

// The sample of the border tab: a block of four cells with the lines on its edges, and the diagonals.
class BorderPreview : public QWidget {
public:
    explicit BorderPreview(FormatCellsDialog *owner) : QWidget(owner), m_owner(owner) { setMinimumSize(190, 140); }
    QSize sizeHint() const override { return QSize(200, 150); }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, false);
        p.fillRect(rect(), palette().base());
        const QRectF box(24, 14, width() - 48, height() - 28);
        // the cells, faintly
        p.setPen(QPen(QColor(0xD0, 0xD0, 0xD0), 1, Qt::DotLine));
        p.drawRect(box);
        p.drawLine(QPointF(box.center().x(), box.top()), QPointF(box.center().x(), box.bottom()));
        p.drawLine(QPointF(box.left(), box.center().y()), QPointF(box.right(), box.center().y()));
        auto draw = [&](int edge, const QPointF &a, const QPointF &b) {
            const BorderSide &s = m_owner->m_edgeLines[edge];
            if (s.style == BorderStyle::None) return;
            const QColor c = s.color.kind == ColorSpec::Kind::Rgb ? QColor::fromRgb(s.color.rgb) : QColor(Qt::black);
            strokeLine(&p, a, b, s.style, c);
        };
        draw(Top, box.topLeft(), box.topRight());
        draw(Bottom, box.bottomLeft(), box.bottomRight());
        draw(Left, box.topLeft(), box.bottomLeft());
        draw(Right, box.topRight(), box.bottomRight());
        draw(InsideH, QPointF(box.left(), box.center().y()), QPointF(box.right(), box.center().y()));
        draw(InsideV, QPointF(box.center().x(), box.top()), QPointF(box.center().x(), box.bottom()));
        draw(DiagUp, box.bottomLeft(), box.topRight());
        draw(DiagDown, box.topLeft(), box.bottomRight());
    }

private:
    FormatCellsDialog *m_owner;
};

bool FormatCellsDialog::Result::isEmpty() const
{
    const FormatChange &c = change;
    const bool none = !c.fontName && !c.fontSize && !c.bold && !c.italic && !c.strike && !c.underline && !c.vertAlign && !c.fontColor && !c.fill && !c.left && !c.right && !c.top && !c.bottom &&
                      !c.diagonal && !c.diagonalUp && !c.diagonalDown && !c.numberFormat && !c.horizontal && !c.vertical && !c.wrap && !c.shrink && !c.indent && !c.rotation && !c.readingOrder &&
                      !c.locked && !c.formulaHidden && !c.namedStyle;
    return none && borders.isEmpty() && !merge;
}

FormatCellsDialog::FormatCellsDialog(const Input &input, QWidget *parent) : QDialog(parent), m_in(input)
{
    setObjectName(QStringLiteral("formatCellsDialog"));
    setWindowTitle(tr("Format Cells"));
    auto *layout = new QVBoxLayout(this);
    m_tabs = new QTabWidget(this);
    m_tabs->setObjectName(QStringLiteral("formatTabs"));
    const struct {
        const char *name;
        void (FormatCellsDialog::*build)(QWidget *);
    } pages[] = {{QT_TR_NOOP("Number"), &FormatCellsDialog::buildNumber},       {QT_TR_NOOP("Alignment"), &FormatCellsDialog::buildAlignment},
                 {QT_TR_NOOP("Font"), &FormatCellsDialog::buildFont},           {QT_TR_NOOP("Border"), &FormatCellsDialog::buildBorder},
                 {QT_TR_NOOP("Fill"), &FormatCellsDialog::buildFill},           {QT_TR_NOOP("Protection"), &FormatCellsDialog::buildProtection}};
    m_building = true;
    for (const auto &p : pages) {
        auto *page = new QWidget(m_tabs);
        (this->*p.build)(page);
        m_tabs->addTab(page, tr(p.name));
    }
    m_building = false;
    layout->addWidget(m_tabs);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    updateSample();
    updateFontPreview();
    updateFillSample();
    refreshBorderButtons();
}

void FormatCellsDialog::showTab(int tab) { m_tabs->setCurrentIndex(std::max(0, std::min(int(ProtectionTab), tab))); }

ColorSpec FormatCellsDialog::specOf(const QColor &c) const { return ColorSpec::fromRgb(quint32(c.rgb() & 0xFFFFFF)); }

QColor FormatCellsDialog::colorOf(ColorButton *b) const
{
    const ColorRef ref = b->current();
    if (ref.isNone()) return QColor();
    ColorScheme scheme;
    if (!m_in.palette.isEmpty()) {
        scheme.c[Main] = m_in.palette.value(0, Qt::black);
        for (int i = 0; i < 5; ++i) scheme.c[Accent1 + i] = m_in.palette.value(1 + i, Qt::gray);
        scheme.c[Hyperlink] = m_in.palette.value(6, Qt::blue);
        scheme.c[Followed] = m_in.palette.value(7, Qt::magenta);
    } else {
        scheme = builtinColorSchemes().first();
    }
    return ref.resolve(scheme);
}

// ---------------------------------------------------------------- the Number tab

void FormatCellsDialog::buildNumber(QWidget *page)
{
    auto *outer = new QHBoxLayout(page);
    auto *left = new QVBoxLayout();
    m_categories = new QListWidget(page);
    auto *categoryLabel = new QLabel(tr("&Category:"), page);
    categoryLabel->setBuddy(m_categories);
    left->addWidget(categoryLabel);
    m_categories->setObjectName(QStringLiteral("numberCategory"));
    m_categories->setMaximumWidth(160);
    for (int c = 0; c <= int(NumberCategory::Custom); ++c) m_categories->addItem(jp::crunch::categoryName(NumberCategory(c)));
    left->addWidget(m_categories);
    outer->addLayout(left);
    auto *right = new QVBoxLayout();
    auto *sampleBox = new QGroupBox(tr("Sample"), page);
    auto *sampleLayout = new QVBoxLayout(sampleBox);
    m_sample = new QLabel(page);
    m_sample->setObjectName(QStringLiteral("numberSample"));
    m_sample->setMinimumHeight(24);
    m_sample->setTextInteractionFlags(Qt::TextSelectableByMouse);
    sampleLayout->addWidget(m_sample);
    right->addWidget(sampleBox);
    m_description = new QLabel(page);
    m_description->setWordWrap(true);
    right->addWidget(m_description);
    // the options (each category shows the ones it has)
    auto *options = new QWidget(page);
    auto *form = new QVBoxLayout(options);
    form->setContentsMargins(0, 0, 0, 0);
    m_optionsBox = new QWidget(options);
    auto *places = new QHBoxLayout(m_optionsBox);
    places->setContentsMargins(0, 0, 0, 0);
    m_decimals = new QSpinBox(m_optionsBox);
    auto *decimalsLabel = new QLabel(tr("&Decimal places:"), m_optionsBox);
    decimalsLabel->setBuddy(m_decimals);
    places->addWidget(decimalsLabel);
    m_decimals->setObjectName(QStringLiteral("decimals"));
    m_decimals->setRange(0, 30);
    m_decimals->setValue(2);
    places->addWidget(m_decimals);
    places->addStretch(1);
    form->addWidget(m_optionsBox);
    m_symbolBox = new QWidget(options);
    auto *symbolRow = new QHBoxLayout(m_symbolBox);
    symbolRow->setContentsMargins(0, 0, 0, 0);
    m_symbol = new QComboBox(m_symbolBox);
    auto *symbolLabel = new QLabel(tr("&Symbol:"), m_symbolBox);
    symbolLabel->setBuddy(m_symbol);
    symbolRow->addWidget(symbolLabel);
    m_symbol->setObjectName(QStringLiteral("symbol"));
    bool first = true;
    const QString local = jp::crunch::currencySymbolOf(m_in.locale, &first);
    auto addSymbol = [&](const QString &text, const QString &symbol, bool isFirst) {
        for (int i = 0; i < m_symbol->count(); ++i)
            if (m_symbol->itemData(i).toString() == symbol && !symbol.isEmpty()) return;
        m_symbol->addItem(text, symbol);
        m_symbol->setItemData(m_symbol->count() - 1, isFirst, Qt::UserRole + 1);
    };
    addSymbol(tr("None"), QString(), true);
    if (!local.isEmpty()) addSymbol(local, local, first);
    for (const QString &s : {QStringLiteral("$"), QString(QChar(0x20AC)), QString(QChar(0x00A3)), QString(QChar(0x00A5)), QStringLiteral("CHF"), QStringLiteral("kr"), QString(QChar(0x20B9))})
        addSymbol(s, s, s != QLatin1String("kr") && s != QLatin1String("CHF") && s != QString(QChar(0x20AC)) ? true : first);
    symbolRow->addWidget(m_symbol);
    symbolRow->addStretch(1);
    form->addWidget(m_symbolBox);
    m_thousandsBox = new QWidget(options);
    auto *thousandsRow = new QHBoxLayout(m_thousandsBox);
    thousandsRow->setContentsMargins(0, 0, 0, 0);
    m_thousands = new QCheckBox(tr("Use 1000 &Separator"), m_thousandsBox);
    m_thousands->setObjectName(QStringLiteral("thousands"));
    thousandsRow->addWidget(m_thousands);
    form->addWidget(m_thousandsBox);
    m_negativeBox = new QWidget(options);
    auto *negativeRow = new QVBoxLayout(m_negativeBox);
    negativeRow->setContentsMargins(0, 0, 0, 0);
    m_negatives = new QListWidget(m_negativeBox);
    auto *negativeLabel = new QLabel(tr("&Negative numbers:"), m_negativeBox);
    negativeLabel->setBuddy(m_negatives);
    negativeRow->addWidget(negativeLabel);
    m_negatives->setObjectName(QStringLiteral("negatives"));
    m_negatives->setMaximumHeight(90);
    for (int i = 0; i < jp::crunch::negativeStyles(); ++i) m_negatives->addItem(QString());
    negativeRow->addWidget(m_negatives);
    form->addWidget(m_negativeBox);
    m_typesBox = new QWidget(options);
    auto *typeRow = new QVBoxLayout(m_typesBox);
    typeRow->setContentsMargins(0, 0, 0, 0);
    m_types = new QListWidget(m_typesBox);
    auto *typeLabel = new QLabel(tr("&Type:"), m_typesBox);
    typeLabel->setBuddy(m_types);
    typeRow->addWidget(typeLabel);
    m_types->setObjectName(QStringLiteral("types"));
    typeRow->addWidget(m_types);
    form->addWidget(m_typesBox);
    m_customBox = new QWidget(options);
    auto *customRow = new QVBoxLayout(m_customBox);
    customRow->setContentsMargins(0, 0, 0, 0);
    m_custom = new QLineEdit(m_customBox);
    auto *customLabel = new QLabel(tr("&Type:"), m_customBox);
    customLabel->setBuddy(m_custom);
    customRow->addWidget(customLabel);
    m_custom->setObjectName(QStringLiteral("customCode"));
    customRow->addWidget(m_custom);
    form->addWidget(m_customBox);
    form->addStretch(1);
    right->addWidget(options, 1);
    outer->addLayout(right, 1);

    connect(m_categories, &QListWidget::currentRowChanged, this, [this](int row) { categoryChanged(row); });
    connect(m_decimals, qOverload<int>(&QSpinBox::valueChanged), this, [this] { numberControlsChanged(); });
    connect(m_thousands, &QCheckBox::toggled, this, [this] { numberControlsChanged(); });
    connect(m_symbol, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { numberControlsChanged(); });
    connect(m_negatives, &QListWidget::currentRowChanged, this, [this] { numberControlsChanged(); });
    connect(m_types, &QListWidget::currentRowChanged, this, [this] { numberControlsChanged(); });
    connect(m_custom, &QLineEdit::textEdited, this, [this] { numberControlsChanged(); });
    setCode(m_in.view.numberFormat.isEmpty() ? QStringLiteral("General") : m_in.view.numberFormat);
}

namespace {

QString descriptionOf(NumberCategory c)
{
    const char *ctx = "jp::FormatCellsDialog";
    switch (c) {
    case NumberCategory::General: return QCoreApplication::translate(ctx, "A cell with no number format shows what it holds as it is, with as many digits as fit.");
    case NumberCategory::Number: return QCoreApplication::translate(ctx, "A number is shown with the digits after the point that you choose, with or without a mark between the thousands, and with the way you want negative numbers to look.");
    case NumberCategory::Currency: return QCoreApplication::translate(ctx, "An amount of money, shown with a currency symbol beside it. Use Accounting to line the amounts up down a column.");
    case NumberCategory::Accounting: return QCoreApplication::translate(ctx, "Money with the currency symbols and the decimal points lined up down a column, so that the amounts are easy to compare.");
    case NumberCategory::Date: return QCoreApplication::translate(ctx, "A number that counts days is shown as a date, in the order and with the names you pick.");
    case NumberCategory::Time: return QCoreApplication::translate(ctx, "The part of a day that a number holds is shown as a time of day.");
    case NumberCategory::Percentage: return QCoreApplication::translate(ctx, "The number is multiplied by 100 and shown with a percent sign: 0.25 is shown as 25%.");
    case NumberCategory::Fraction: return QCoreApplication::translate(ctx, "The number is shown as a fraction, such as 3/4, rounded to the size you pick.");
    case NumberCategory::Scientific: return QCoreApplication::translate(ctx, "The number is shown as a short number times a power of ten: 1500 is shown as 1.50E+03.");
    case NumberCategory::Text: return QCoreApplication::translate(ctx, "What is typed in the cell is kept as text, even if it is made of digits, and is shown exactly as it was typed: 00123 keeps its zeros.");
    case NumberCategory::Special: return QCoreApplication::translate(ctx, "Shapes for numbers that are not amounts, such as ZIP codes and phone numbers.");
    case NumberCategory::Custom: return QCoreApplication::translate(ctx, "Write your own code for how a number is shown, or pick one from the list and change it.");
    }
    return QString();
}

} // namespace

// The category on the left changed (by the person, or when the dialog begins): the options it has are shown, and the code is made from them.
void FormatCellsDialog::categoryChanged(int row)
{
    if (row < 0) return;
    const NumberCategory c = NumberCategory(row);
    const bool places = c == NumberCategory::Number || c == NumberCategory::Currency || c == NumberCategory::Accounting || c == NumberCategory::Percentage || c == NumberCategory::Scientific;
    m_optionsBox->setVisible(places);
    m_thousandsBox->setVisible(c == NumberCategory::Number);
    m_symbolBox->setVisible(c == NumberCategory::Currency || c == NumberCategory::Accounting);
    m_negativeBox->setVisible(c == NumberCategory::Number || c == NumberCategory::Currency);
    const QVector<jp::crunch::FormatType> types = jp::crunch::typesFor(c, m_in.locale);
    const bool listed = c == NumberCategory::Date || c == NumberCategory::Time || c == NumberCategory::Fraction || c == NumberCategory::Special;
    m_typesBox->setVisible(listed);
    m_customBox->setVisible(c == NumberCategory::Custom);
    m_description->setText(descriptionOf(c));
    const bool wasBuilding = m_building;
    m_building = true;
    // (a currency begins with the language's own symbol, as the programs people know do)
    if (!wasBuilding && (c == NumberCategory::Currency || c == NumberCategory::Accounting) && m_symbol->currentIndex() == 0 && m_symbol->count() > 1) m_symbol->setCurrentIndex(1);
    if (listed) {
        m_types->clear();
        const jp::calc::NumberSymbols symbols = jp::calc::NumberSymbols::of(m_in.locale);
        for (const jp::crunch::FormatType &t : types) {
            const jp::calc::Formatted f = jp::calc::NumberFormat(t.code).format(m_in.sampleIsNumber ? m_in.sampleNumber : 0, m_in.dateSystem, 0, &symbols);
            m_types->addItem(f.failed || f.text.isEmpty() ? t.code : f.text);
            m_types->item(m_types->count() - 1)->setData(Qt::UserRole, t.code);
            if (!t.label.isEmpty()) m_types->item(m_types->count() - 1)->setToolTip(t.label);
        }
        if (m_types->currentRow() < 0) m_types->setCurrentRow(0);
    }
    if (c == NumberCategory::Custom && m_custom->text().isEmpty()) m_custom->setText(m_code);
    m_building = wasBuilding;
    if (!wasBuilding) numberControlsChanged();
}

namespace {

// The options a currency combo item stands for.
void symbolOf(QComboBox *box, QString *symbol, bool *first)
{
    *symbol = box->currentData().toString();
    *first = box->currentData(Qt::UserRole + 1).toBool();
}

} // namespace

// An option changed: the code is made again from the category and its options, and the sample and the list of negatives follow.
void FormatCellsDialog::numberControlsChanged()
{
    if (m_building) return;
    const int row = m_categories->currentRow();
    if (row < 0) return;
    const NumberCategory c = NumberCategory(row);
    NumberOptions o;
    o.decimals = m_decimals->value();
    o.thousands = m_thousands->isChecked();
    o.negative = std::max(0, m_negatives->currentRow());
    symbolOf(m_symbol, &o.symbol, &o.symbolFirst);
    if (c == NumberCategory::Accounting || c == NumberCategory::Currency) o.thousands = true;   // (their codes always group the digits)
    QString code;
    switch (c) {
    case NumberCategory::Date:
    case NumberCategory::Time:
    case NumberCategory::Fraction:
    case NumberCategory::Special: {
        const QListWidgetItem *item = m_types->currentItem();
        code = item ? item->data(Qt::UserRole).toString() : QString();
        break;
    }
    case NumberCategory::Custom: code = m_custom->text(); break;
    default: code = jp::crunch::codeFor(c, o); break;
    }
    if (code.isEmpty()) code = QStringLiteral("General");
    m_code = code;
    // the negatives show how each style writes -1234.1 with the places and symbol now chosen
    if (c == NumberCategory::Number || c == NumberCategory::Currency) {
        const bool wasBuilding = m_building;
        m_building = true;
        const jp::calc::NumberSymbols symbols = jp::calc::NumberSymbols::of(m_in.locale);
        for (int i = 0; i < m_negatives->count(); ++i) {
            NumberOptions n = o;
            n.negative = i;
            const jp::calc::Formatted f = jp::calc::NumberFormat(jp::crunch::codeFor(c, n)).format(-1234.1, m_in.dateSystem, 0, &symbols);
            m_negatives->item(i)->setText(f.failed ? QString() : f.text);
            m_negatives->item(i)->setForeground(f.color == 3 ? QBrush(QColor(0xC0, 0, 0)) : QBrush());
        }
        m_negatives->setCurrentRow(o.negative);
        m_building = wasBuilding;
    }
    updateSample();
}

void FormatCellsDialog::updateSample()
{
    if (!m_sample) return;
    const jp::calc::NumberSymbols symbols = jp::calc::NumberSymbols::of(m_in.locale);
    const jp::calc::NumberFormat format(m_code);
    QString text;
    if (m_in.sampleIsNumber) {
        const jp::calc::Formatted f = format.format(m_in.sampleNumber, m_in.dateSystem, 0, &symbols);
        text = f.failed ? QStringLiteral("#####") : f.text;
    } else {
        const jp::calc::Formatted f = format.formatText(m_in.sampleText);
        text = f.failed ? m_in.sampleText : f.text;
    }
    m_sample->setText(text);
}

// Shows a code: the category it is in, with the options it was made from, or its place among the ready-made formats, or as a custom code.
void FormatCellsDialog::setCode(const QString &code)
{
    m_code = code;
    NumberCategory category = jp::crunch::categoryOf(code);
    NumberOptions o;
    NumberCategory optionCategory = category;
    const bool madeOfOptions = jp::crunch::optionsOf(code, &optionCategory, &o);
    m_building = true;
    int typeRow = -1;
    if (!madeOfOptions) {
        if (category == NumberCategory::Date || category == NumberCategory::Time || category == NumberCategory::Fraction || category == NumberCategory::Special) {
            const QVector<jp::crunch::FormatType> types = jp::crunch::typesFor(category, m_in.locale);
            for (int i = 0; i < types.size(); ++i)
                if (types[i].code == code) typeRow = i;
            if (typeRow < 0) category = NumberCategory::Custom;
        } else if (category != NumberCategory::General && category != NumberCategory::Text) {
            category = NumberCategory::Custom;
        }
    }
    if (madeOfOptions) {
        m_decimals->setValue(o.decimals);
        m_thousands->setChecked(o.thousands);
        for (int i = 0; i < m_symbol->count(); ++i)
            if (m_symbol->itemData(i).toString() == o.symbol) m_symbol->setCurrentIndex(i);
        if (!o.symbol.isEmpty() && m_symbol->currentData().toString() != o.symbol) {
            m_symbol->addItem(o.symbol, o.symbol);
            m_symbol->setItemData(m_symbol->count() - 1, o.symbolFirst, Qt::UserRole + 1);
            m_symbol->setCurrentIndex(m_symbol->count() - 1);
        }
        m_negatives->setCurrentRow(o.negative);
    }
    if (category == NumberCategory::Custom) m_custom->setText(code);
    m_categories->setCurrentRow(int(category));
    categoryChanged(int(category));
    if (typeRow >= 0) m_types->setCurrentRow(typeRow);
    m_building = false;
    // (the list of negatives and the sample are made from the options; the code stays as the file had it)
    const QString keep = code;
    numberControlsChanged();
    m_code = keep;
    updateSample();
}

// ---------------------------------------------------------------- the Alignment tab

void FormatCellsDialog::buildAlignment(QWidget *page)
{
    const FormatView &v = m_in.view;
    auto *layout = new QVBoxLayout(page);
    auto *text = new QGroupBox(tr("Text alignment"), page);
    auto *form = new QFormLayout(text);
    m_horizontal = new QComboBox(text);
    m_horizontal->setObjectName(QStringLiteral("horizontal"));
    m_horizontal->addItems({tr("General"), tr("Left"), tr("Center"), tr("Right"), tr("Repeat to fill"), tr("Justify"), tr("Center over the selected cells"), tr("Spread evenly")});
    for (int i = 0; i < 8; ++i)
        if (kHorizontal[i] == v.horizontal) m_horizontal->setCurrentIndex(i);
    form->addRow(tr("&Horizontal:"), m_horizontal);
    m_indent = new QSpinBox(text);
    m_indent->setObjectName(QStringLiteral("indent"));
    m_indent->setRange(0, 250);
    m_indent->setValue(v.indent);
    form->addRow(tr("&Indent:"), m_indent);
    m_vertical = new QComboBox(text);
    m_vertical->setObjectName(QStringLiteral("vertical"));
    m_vertical->addItems({tr("Top"), tr("Center"), tr("Bottom"), tr("Justify"), tr("Distributed")});
    for (int i = 0; i < 5; ++i)
        if (kVertical[i] == v.vertical) m_vertical->setCurrentIndex(i);
    form->addRow(tr("&Vertical:"), m_vertical);
    layout->addWidget(text);
    auto *control = new QGroupBox(tr("Text control"), page);
    auto *cl = new QVBoxLayout(control);
    m_wrap = new QCheckBox(tr("&Wrap text"), control);
    m_wrap->setObjectName(QStringLiteral("wrap"));
    m_wrap->setChecked(v.wrap);
    m_shrink = new QCheckBox(tr("&Shrink to fit"), control);
    m_shrink->setObjectName(QStringLiteral("shrink"));
    m_shrink->setChecked(v.shrink);
    m_merge = new QCheckBox(tr("&Merge cells"), control);
    m_merge->setObjectName(QStringLiteral("merge"));
    m_merge->setChecked(v.merged);
    cl->addWidget(m_wrap);
    cl->addWidget(m_shrink);
    cl->addWidget(m_merge);
    layout->addWidget(control);
    auto *orientation = new QGroupBox(tr("Orientation"), page);
    auto *ol = new QFormLayout(orientation);
    m_degrees = new QSpinBox(orientation);
    m_degrees->setObjectName(QStringLiteral("degrees"));
    m_degrees->setRange(-90, 90);
    m_degrees->setSuffix(QStringLiteral("°"));
    const int rotation = v.rotation == 255 ? 0 : v.rotation <= 90 ? v.rotation : -(v.rotation - 90);
    m_degrees->setValue(rotation);
    m_verticalText = new QCheckBox(tr("&Text stacked vertically"), orientation);
    m_verticalText->setObjectName(QStringLiteral("stacked"));
    m_verticalText->setChecked(v.rotation == 255);
    ol->addRow(tr("&Degrees:"), m_degrees);
    ol->addRow(m_verticalText);
    layout->addWidget(orientation);
    auto *dir = new QGroupBox(tr("Right-to-left"), page);
    auto *dl = new QFormLayout(dir);
    m_direction = new QComboBox(dir);
    m_direction->setObjectName(QStringLiteral("direction"));
    m_direction->addItems({tr("By the text"), tr("Left to right"), tr("Right to left")});
    m_direction->setCurrentIndex(std::max(0, std::min(2, v.readingOrder)));
    dl->addRow(tr("Text &direction:"), m_direction);
    layout->addWidget(dir);
    layout->addStretch(1);
    connect(m_horizontal, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { alignmentChanged(); });
    connect(m_verticalText, &QCheckBox::toggled, this, [this] { alignmentChanged(); });
    connect(m_wrap, &QCheckBox::toggled, this, [this](bool on) {
        // (text that wraps does not shrink, and the other way)
        if (on) m_shrink->setChecked(false);
    });
    connect(m_shrink, &QCheckBox::toggled, this, [this](bool on) {
        if (on) m_wrap->setChecked(false);
    });
    alignmentChanged();
}

void FormatCellsDialog::alignmentChanged()
{
    const HAlign h = kHorizontal[std::max(0, m_horizontal->currentIndex())];
    m_indent->setEnabled(h == HAlign::Left || h == HAlign::Right || h == HAlign::Distributed);
    m_degrees->setEnabled(!m_verticalText->isChecked());
}

// ---------------------------------------------------------------- the Font tab

void FormatCellsDialog::buildFont(QWidget *page)
{
    const FormatView &v = m_in.view;
    auto *layout = new QVBoxLayout(page);
    auto *form = new QFormLayout();
    m_fontName = new QComboBox(page);
    m_fontName->setObjectName(QStringLiteral("fontName"));
    m_fontName->setEditable(true);
    QStringList families = QFontDatabase::families();
    if (!v.fontName.isEmpty() && !families.contains(v.fontName, Qt::CaseInsensitive)) families.prepend(v.fontName);
    m_fontName->addItems(families);
    m_fontName->setCurrentText(v.fontName);
    form->addRow(tr("&Font:"), m_fontName);
    m_fontStyle = new QComboBox(page);
    m_fontStyle->setObjectName(QStringLiteral("fontStyle"));
    m_fontStyle->addItems({tr("Regular"), tr("Italic"), tr("Bold"), tr("Bold Italic")});
    m_fontStyle->setCurrentIndex((v.bold ? 2 : 0) + (v.italic ? 1 : 0));
    form->addRow(tr("Font st&yle:"), m_fontStyle);
    m_fontSize = new QComboBox(page);
    m_fontSize->setObjectName(QStringLiteral("fontSize"));
    m_fontSize->setEditable(true);
    for (int s : {8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72}) m_fontSize->addItem(QString::number(s));
    m_fontSize->setCurrentText(QString::number(v.fontSize, 'g', 4));
    form->addRow(tr("&Size:"), m_fontSize);
    m_underline = new QComboBox(page);
    m_underline->setObjectName(QStringLiteral("underline"));
    m_underline->addItems({tr("None"), tr("Single"), tr("Double"), tr("Single Accounting"), tr("Double Accounting")});
    m_underline->setCurrentIndex(int(v.underline));
    form->addRow(tr("&Underline:"), m_underline);
    m_fontColor = new ColorButton(QStringLiteral("baseline"), tr("Font Color"), true, tr("Automatic"), page);
    m_fontColor->setObjectName(QStringLiteral("fontColor"));
    m_fontColor->setWell(true);
    m_fontColor->setCurrent(v.fontColor.isValid() ? ColorRef::rgb(v.fontColor) : ColorRef::none());
    form->addRow(tr("&Color:"), m_fontColor);
    layout->addLayout(form);
    auto *effects = new QGroupBox(tr("Effects"), page);
    auto *el = new QVBoxLayout(effects);
    m_strike = new QCheckBox(tr("Stri&kethrough"), effects);
    m_strike->setObjectName(QStringLiteral("strike"));
    m_strike->setChecked(v.strike);
    m_superscript = new QCheckBox(tr("Su&perscript"), effects);
    m_superscript->setObjectName(QStringLiteral("superscript"));
    m_superscript->setChecked(v.vertAlign == 1);
    m_subscript = new QCheckBox(tr("Su&bscript"), effects);
    m_subscript->setObjectName(QStringLiteral("subscript"));
    m_subscript->setChecked(v.vertAlign == 2);
    el->addWidget(m_strike);
    el->addWidget(m_superscript);
    el->addWidget(m_subscript);
    layout->addWidget(effects);
    auto *preview = new QGroupBox(tr("Preview"), page);
    auto *pl = new QVBoxLayout(preview);
    m_fontPreview = new QLabel(tr("Sample text 0123"), preview);
    m_fontPreview->setObjectName(QStringLiteral("fontPreview"));
    m_fontPreview->setAlignment(Qt::AlignCenter);
    m_fontPreview->setMinimumHeight(44);
    pl->addWidget(m_fontPreview);
    layout->addWidget(preview);
    layout->addStretch(1);
    connect(m_fontName, &QComboBox::currentTextChanged, this, [this] { updateFontPreview(); });
    connect(m_fontStyle, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { updateFontPreview(); });
    connect(m_fontSize, &QComboBox::currentTextChanged, this, [this] { updateFontPreview(); });
    connect(m_underline, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { updateFontPreview(); });
    connect(m_strike, &QCheckBox::toggled, this, [this] { updateFontPreview(); });
    connect(m_superscript, &QCheckBox::toggled, this, [this](bool on) {
        if (on) m_subscript->setChecked(false);
        updateFontPreview();
    });
    connect(m_subscript, &QCheckBox::toggled, this, [this](bool on) {
        if (on) m_superscript->setChecked(false);
        updateFontPreview();
    });
    connect(m_fontColor, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
        m_fontColor->setCurrent(c);
        m_fontColorTouched = true;
        updateFontPreview();
    });
}

void FormatCellsDialog::updateFontPreview()
{
    if (!m_fontPreview) return;
    QFont f(m_fontName->currentText());
    bool ok = false;
    const double size = m_fontSize->currentText().toDouble(&ok);
    f.setPointSizeF(ok && size > 0 ? std::min(36.0, size) : 11.0);
    f.setBold(m_fontStyle->currentIndex() >= 2);
    f.setItalic(m_fontStyle->currentIndex() % 2 == 1);
    f.setUnderline(m_underline->currentIndex() != 0);
    f.setStrikeOut(m_strike->isChecked());
    m_fontPreview->setFont(f);
    const QColor c = colorOf(m_fontColor);
    QPalette p = m_fontPreview->palette();
    p.setColor(QPalette::WindowText, c.isValid() ? c : palette().color(QPalette::WindowText));
    m_fontPreview->setPalette(p);
}

// ---------------------------------------------------------------- the Border tab

BorderSide FormatCellsDialog::currentLine() const
{
    BorderSide s;
    const int row = m_lineStyles ? m_lineStyles->currentRow() : 6;
    s.style = row >= 0 && row < int(sizeof(kLineStyles) / sizeof(kLineStyles[0])) ? kLineStyles[row] : BorderStyle::Thin;
    const QColor c = colorOf(m_lineColor);
    s.color = ColorSpec::fromRgb(c.isValid() ? quint32(c.rgb() & 0xFFFFFF) : 0u);
    return s;
}

void FormatCellsDialog::buildBorder(QWidget *page)
{
    const FormatView &v = m_in.view;
    auto *layout = new QHBoxLayout(page);
    auto *left = new QVBoxLayout();
    auto *line = new QGroupBox(tr("Line"), page);
    auto *ll = new QVBoxLayout(line);
    m_lineStyles = new QListWidget(line);
    auto *styleLabel = new QLabel(tr("St&yle:"), line);
    styleLabel->setBuddy(m_lineStyles);
    ll->addWidget(styleLabel);
    m_lineStyles->setObjectName(QStringLiteral("lineStyle"));
    m_lineStyles->setIconSize(QSize(72, 14));
    for (const BorderStyle s : kLineStyles) {
        auto *item = new QListWidgetItem(lineSwatch(s, Qt::black), lineStyleName(s), m_lineStyles);
        item->setToolTip(lineStyleName(s));
    }
    m_lineStyles->setCurrentRow(6);   // thin
    m_lineStyles->setMinimumHeight(190);
    ll->addWidget(m_lineStyles);
    m_lineColor = new ColorButton(QStringLiteral("baseline"), tr("Line Color"), true, tr("Automatic"), line);
    m_lineColor->setObjectName(QStringLiteral("lineColor"));
    m_lineColor->setWell(true);
    m_lineColor->setCurrent(ColorRef::none());
    auto *colorLabel = new QLabel(tr("&Color:"), line);
    colorLabel->setBuddy(m_lineColor);
    ll->addWidget(colorLabel);
    ll->addWidget(m_lineColor);
    left->addWidget(line);
    layout->addLayout(left);
    auto *right = new QVBoxLayout();
    auto *presets = new QGroupBox(tr("Presets"), page);
    auto *pl = new QHBoxLayout(presets);
    const struct {
        const char *name;
        const char *id;
        const char *icon;
    } presetList[] = {{QT_TR_NOOP("None"), "presetNone", "square-dashed"}, {QT_TR_NOOP("Outline"), "presetOutline", "square"}, {QT_TR_NOOP("Inside"), "presetInside", "grid-2x2"}};
    for (int i = 0; i < 3; ++i) {
        auto *b = new QPushButton(icon(QString::fromLatin1(presetList[i].icon)), tr(presetList[i].name), presets);
        b->setObjectName(QString::fromLatin1(presetList[i].id));
        if (i == 2) b->setEnabled(!m_in.oneCell);
        pl->addWidget(b);
        connect(b, &QPushButton::clicked, this, [this, i] {
            if (i == 0) {
                for (int e = 0; e < EdgeCount; ++e) setEdge(e, BorderSide());
            } else if (i == 1) {
                for (int e : {int(Top), int(Bottom), int(Left), int(Right)}) setEdge(e, currentLine());
            } else {
                for (int e : {int(InsideH), int(InsideV)}) setEdge(e, currentLine());
            }
        });
    }
    right->addWidget(presets);
    auto *border = new QGroupBox(tr("Border"), page);
    auto *grid = new QGridLayout(border);
    m_borderPreview = new BorderPreview(this);
    m_borderPreview->setObjectName(QStringLiteral("borderPreview"));
    const struct {
        int edge;
        const char *name;
        const char *id;
        int row, col;
    } buttons[] = {{Top, QT_TR_NOOP("Top"), "borderTop", 0, 1},
                   {InsideH, QT_TR_NOOP("Inside horizontal"), "borderInsideH", 1, 0},
                   {Bottom, QT_TR_NOOP("Bottom"), "borderBottom", 2, 1},
                   {Left, QT_TR_NOOP("Left"), "borderLeft", 1, 0},
                   {InsideV, QT_TR_NOOP("Inside vertical"), "borderInsideV", 2, 0},
                   {Right, QT_TR_NOOP("Right"), "borderRight", 1, 2},
                   {DiagUp, QT_TR_NOOP("Diagonal up"), "borderDiagUp", 2, 0},
                   {DiagDown, QT_TR_NOOP("Diagonal down"), "borderDiagDown", 2, 2}};
    for (const auto &b : buttons) {
        auto *button = new QToolButton(border);
        button->setObjectName(QString::fromLatin1(b.id));
        button->setCheckable(true);
        button->setToolTip(tr(b.name));
        button->setText(tr(b.name));
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);
        button->setAccessibleName(tr(b.name));
        m_edgeButtons[b.edge] = button;
        connect(button, &QToolButton::clicked, this, [this, e = b.edge] { toggleEdge(e); });
        if (b.edge == InsideH || b.edge == InsideV) button->setEnabled(!m_in.oneCell);
    }
    // (the buttons sit round the sample: top above it, bottom under it, left and right at its sides, and the lines between the cells and the diagonals in a row under)
    grid->addWidget(m_edgeButtons[Top], 0, 1, Qt::AlignHCenter);
    grid->addWidget(m_edgeButtons[Left], 1, 0, Qt::AlignVCenter);
    grid->addWidget(m_borderPreview, 1, 1);
    grid->addWidget(m_edgeButtons[Right], 1, 2, Qt::AlignVCenter);
    grid->addWidget(m_edgeButtons[Bottom], 2, 1, Qt::AlignHCenter);
    auto *under = new QHBoxLayout();
    for (const int e : {int(InsideH), int(InsideV), int(DiagUp), int(DiagDown)}) under->addWidget(m_edgeButtons[e]);
    grid->addLayout(under, 3, 0, 1, 3);
    grid->setColumnStretch(1, 1);
    right->addWidget(border, 1);
    auto *note = new QLabel(tr("Click a button to put the line in the style and color chosen on the edge it names, or to take away the line that is there."), page);
    note->setWordWrap(true);
    right->addWidget(note);
    layout->addLayout(right, 1);
    // the lines the cell has
    auto take = [&](int edge, const BorderSide &s) { m_edgeLines[edge] = s; };
    take(Top, v.border.top);
    take(Bottom, v.border.bottom);
    take(Left, v.border.left);
    take(Right, v.border.right);
    if (v.border.diagonalUp) take(DiagUp, v.border.diagonal);
    if (v.border.diagonalDown) take(DiagDown, v.border.diagonal);
    connect(m_lineStyles, &QListWidget::currentRowChanged, this, [this] { m_borderPreview->update(); });
}

void FormatCellsDialog::refreshBorderButtons()
{
    if (!m_borderPreview) return;
    for (int e = 0; e < EdgeCount; ++e) {
        const QSignalBlocker block(m_edgeButtons[e]);
        m_edgeButtons[e]->setChecked(m_edgeLines[e].style != BorderStyle::None);
    }
    m_borderPreview->update();
}

void FormatCellsDialog::setEdge(int edge, const BorderSide &line)
{
    if (edge < 0 || edge >= EdgeCount) return;
    if (m_in.oneCell && (edge == InsideH || edge == InsideV)) return;
    m_edgeLines[edge] = line;
    m_edgeChanged[edge] = true;
    refreshBorderButtons();
}

void FormatCellsDialog::toggleEdge(int edge)
{
    const BorderSide now = currentLine();
    const BorderSide &has = m_edgeLines[edge];
    // a line like the one chosen is taken away; any other is put in
    if (has.style == now.style && has.style != BorderStyle::None && has.color == now.color) setEdge(edge, BorderSide());
    else setEdge(edge, now.style == BorderStyle::None ? BorderSide() : now);
}

// ---------------------------------------------------------------- the Fill tab

void FormatCellsDialog::buildFill(QWidget *page)
{
    const FormatView &v = m_in.view;
    auto *layout = new QVBoxLayout(page);
    auto *form = new QFormLayout();
    m_fillColor = new ColorButton(QStringLiteral("paint-bucket"), tr("Background Color"), true, tr("No Color"), page);
    m_fillColor->setObjectName(QStringLiteral("fillColor"));
    m_fillColor->setWell(true);
    m_fillColor->setCurrent(v.fillColor.isValid() ? ColorRef::rgb(v.fillColor) : ColorRef::none());
    form->addRow(tr("&Background Color:"), m_fillColor);
    m_patternColor = new ColorButton(QStringLiteral("paint-bucket"), tr("Pattern Color"), true, tr("Automatic"), page);
    m_patternColor->setObjectName(QStringLiteral("patternColor"));
    m_patternColor->setWell(true);
    m_patternColor->setCurrent(v.patternColor.isValid() ? ColorRef::rgb(v.patternColor) : ColorRef::none());
    form->addRow(tr("P&attern Color:"), m_patternColor);
    m_patternStyle = new QComboBox(page);
    m_patternStyle->setObjectName(QStringLiteral("patternStyle"));
    const QStringList names = {tr("No pattern"),
                               tr("All one color"),
                               tr("Dots, half covered"),
                               tr("Dots, mostly covered"),
                               tr("Dots, a quarter covered"),
                               tr("Wide horizontal lines"),
                               tr("Wide vertical lines"),
                               tr("Wide lines sloping down to the right"),
                               tr("Wide lines sloping up to the right"),
                               tr("Wide grid"),
                               tr("Wide crossed diagonal lines"),
                               tr("Fine horizontal lines"),
                               tr("Fine vertical lines"),
                               tr("Fine lines sloping down to the right"),
                               tr("Fine lines sloping up to the right"),
                               tr("Fine grid"),
                               tr("Fine crossed diagonal lines"),
                               tr("Dots, one in eight"),
                               tr("Dots, one in sixteen")};
    m_patternStyle->addItems(names);
    for (int i = 0; i < int(sizeof(kPatterns) / sizeof(kPatterns[0])); ++i)
        if (kPatterns[i] == v.pattern) m_patternStyle->setCurrentIndex(i);
    form->addRow(tr("Pattern &Style:"), m_patternStyle);
    layout->addLayout(form);
    auto *sample = new QGroupBox(tr("Sample"), page);
    auto *sl = new QVBoxLayout(sample);
    m_fillSample = new QLabel(page);
    m_fillSample->setObjectName(QStringLiteral("fillSample"));
    m_fillSample->setMinimumHeight(60);
    m_fillSample->setAutoFillBackground(true);
    sl->addWidget(m_fillSample);
    layout->addWidget(sample);
    layout->addStretch(1);
    connect(m_fillColor, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
        m_fillColor->setCurrent(c);
        m_fillTouched = true;
        // (a color with no pattern chosen fills the cell solid)
        if (!c.isNone() && m_patternStyle->currentIndex() == 0) m_patternStyle->setCurrentIndex(1);
        if (c.isNone() && m_patternStyle->currentIndex() == 1) m_patternStyle->setCurrentIndex(0);
        updateFillSample();
    });
    connect(m_patternColor, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
        m_patternColor->setCurrent(c);
        m_fillTouched = true;
        updateFillSample();
    });
    connect(m_patternStyle, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] {
        m_fillTouched = true;
        updateFillSample();
    });
}

void FormatCellsDialog::updateFillSample()
{
    if (!m_fillSample) return;
    const PatternKind kind = kPatterns[std::max(0, m_patternStyle->currentIndex())];
    const QColor background = colorOf(m_fillColor), pattern = colorOf(m_patternColor);
    QPixmap pm(220, 60);
    pm.fill(kind == PatternKind::Solid ? (background.isValid() ? background : Qt::white) : (background.isValid() ? background : Qt::white));
    if (kind != PatternKind::None && kind != PatternKind::Solid) {
        QPainter p(&pm);
        const QColor ink = pattern.isValid() ? pattern : QColor(Qt::black);
        Qt::BrushStyle brush = Qt::Dense4Pattern;
        switch (kind) {
        case PatternKind::MediumGray: brush = Qt::Dense4Pattern; break;
        case PatternKind::DarkGray: brush = Qt::Dense2Pattern; break;
        case PatternKind::LightGray: brush = Qt::Dense6Pattern; break;
        case PatternKind::DarkHorizontal:
        case PatternKind::LightHorizontal: brush = Qt::HorPattern; break;
        case PatternKind::DarkVertical:
        case PatternKind::LightVertical: brush = Qt::VerPattern; break;
        case PatternKind::DarkDown:
        case PatternKind::LightDown: brush = Qt::BDiagPattern; break;
        case PatternKind::DarkUp:
        case PatternKind::LightUp: brush = Qt::FDiagPattern; break;
        case PatternKind::DarkGrid:
        case PatternKind::LightGrid: brush = Qt::CrossPattern; break;
        case PatternKind::DarkTrellis:
        case PatternKind::LightTrellis: brush = Qt::DiagCrossPattern; break;
        case PatternKind::Gray125: brush = Qt::Dense7Pattern; break;
        case PatternKind::Gray0625: brush = Qt::Dense7Pattern; break;
        default: break;
        }
        p.fillRect(pm.rect(), QBrush(ink, brush));
    }
    m_fillSample->setPixmap(pm);
}

// ---------------------------------------------------------------- the Protection tab

void FormatCellsDialog::buildProtection(QWidget *page)
{
    const FormatView &v = m_in.view;
    auto *layout = new QVBoxLayout(page);
    m_locked = new QCheckBox(tr("&Locked"), page);
    m_locked->setObjectName(QStringLiteral("locked"));
    m_locked->setChecked(v.locked);
    m_hidden = new QCheckBox(tr("&Hidden"), page);
    m_hidden->setObjectName(QStringLiteral("hidden"));
    m_hidden->setChecked(v.formulaHidden);
    layout->addWidget(m_locked);
    layout->addWidget(m_hidden);
    auto *note = new QLabel(tr("These matter only on a sheet that is protected: there, a locked cell can't be changed and a hidden formula is not shown. A sheet that is not protected treats every cell alike."), page);
    note->setWordWrap(true);
    layout->addWidget(note);
    layout->addStretch(1);
}

// ---------------------------------------------------------------- what the person changed

FormatCellsDialog::Result FormatCellsDialog::result() const
{
    const FormatView &v = m_in.view;
    Result r;
    FormatChange &c = r.change;
    // number
    if (m_code != v.numberFormat && !(m_code.compare(QLatin1String("General"), Qt::CaseInsensitive) == 0 && v.numberFormat.compare(QLatin1String("General"), Qt::CaseInsensitive) == 0))
        c.numberFormat = m_code;
    // alignment
    const HAlign h = kHorizontal[std::max(0, m_horizontal->currentIndex())];
    if (h != v.horizontal) c.horizontal = h;
    const VAlign va = kVertical[std::max(0, m_vertical->currentIndex())];
    if (va != v.vertical) c.vertical = va;
    if (m_wrap->isChecked() != v.wrap) c.wrap = m_wrap->isChecked();
    if (m_shrink->isChecked() != v.shrink) c.shrink = m_shrink->isChecked();
    if (m_indent->value() != v.indent) c.indent = m_indent->value();
    int rotation = m_verticalText->isChecked() ? 255 : m_degrees->value() >= 0 ? m_degrees->value() : 90 - m_degrees->value();
    if (rotation != v.rotation) c.rotation = rotation;
    if (m_direction->currentIndex() != v.readingOrder) c.readingOrder = m_direction->currentIndex();
    if (m_merge->isChecked() != v.merged) r.merge = m_merge->isChecked();
    // font
    const QString name = m_fontName->currentText().trimmed();
    if (!name.isEmpty() && name != v.fontName) c.fontName = name;
    bool ok = false;
    const double size = m_fontSize->currentText().toDouble(&ok);
    if (ok && size >= 1 && size <= 409 && size != v.fontSize) c.fontSize = size;
    const bool bold = m_fontStyle->currentIndex() >= 2, italic = m_fontStyle->currentIndex() % 2 == 1;
    if (bold != v.bold) c.bold = bold;
    if (italic != v.italic) c.italic = italic;
    if (m_strike->isChecked() != v.strike) c.strike = m_strike->isChecked();
    const Underline u = Underline(m_underline->currentIndex());
    if (u != v.underline) c.underline = u;
    const quint8 vert = m_superscript->isChecked() ? 1 : m_subscript->isChecked() ? 2 : 0;
    if (vert != v.vertAlign) c.vertAlign = vert;
    if (m_fontColorTouched) {
        const QColor color = colorOf(m_fontColor);
        c.fontColor = color.isValid() ? specOf(color) : ColorSpec();   // (automatic: no color of its own)
    }
    // fill
    if (m_fillTouched) {
        FillSpec fill;
        const PatternKind kind = kPatterns[std::max(0, m_patternStyle->currentIndex())];
        const QColor background = colorOf(m_fillColor), pattern = colorOf(m_patternColor);
        if (kind == PatternKind::None || (kind == PatternKind::Solid && !background.isValid())) {
            fill.pattern = PatternKind::None;
        } else if (kind == PatternKind::Solid) {
            fill.pattern = PatternKind::Solid;
            fill.fg = specOf(background);
            fill.bg.kind = ColorSpec::Kind::Indexed;
            fill.bg.index = 64;
        } else {
            fill.pattern = kind;
            if (pattern.isValid()) fill.fg = specOf(pattern);
            else {
                fill.fg.kind = ColorSpec::Kind::Indexed;
                fill.fg.index = 64;
            }
            if (background.isValid()) fill.bg = specOf(background);
            else {
                fill.bg.kind = ColorSpec::Kind::Indexed;
                fill.bg.index = 65;
            }
        }
        c.fill = fill;
    }
    // protection
    if (m_locked->isChecked() != v.locked) c.locked = m_locked->isChecked();
    if (m_hidden->isChecked() != v.formulaHidden) c.formulaHidden = m_hidden->isChecked();
    // borders: each edge the person changed, with the others that got the same line put together
    QVector<BorderOp> ops;
    for (int e = 0; e < EdgeCount; ++e) {
        if (!m_edgeChanged[e]) continue;
        bool merged = false;
        for (BorderOp &op : ops)
            if (op.line == m_edgeLines[e]) {
                op.edges |= kEdgeBits[e];
                merged = true;
                break;
            }
        if (!merged) ops.append({kEdgeBits[e], m_edgeLines[e]});
    }
    r.borders = ops;
    return r;
}

} // namespace jp
