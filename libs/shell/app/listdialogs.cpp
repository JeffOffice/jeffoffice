#include "app/listdialogs.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFontComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace jp {

namespace {

QString tr(const char *s) { return QCoreApplication::translate("jp::ListDialogs", s); }

QDialogButtonBox *buttons(QDialog *d)
{
    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, d);
    QObject::connect(box, &QDialogButtonBox::accepted, d, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, d, &QDialog::reject);
    return box;
}

QComboBox *alignmentBox(const QString &current, QWidget *parent)
{
    auto *box = new QComboBox(parent);
    box->addItem(tr("Left"), QStringLiteral("left"));
    box->addItem(tr("Centered"), QStringLiteral("center"));
    box->addItem(tr("Right"), QStringLiteral("right"));
    box->setCurrentIndex(std::max(0, box->findData(current)));
    return box;
}

} // namespace

const NumberStyleName *numberStyleNames(int *count)
{
    static const NumberStyleName names[] = {{"decimal", "1, 2, 3"}, {"upperLetter", "A, B, C"}, {"lowerLetter", "a, b, c"}, {"upperRoman", "I, II, III"}, {"lowerRoman", "i, ii, iii"},
                                              {"decimalZero", "01, 02, 03"}, {"ordinal", "1st, 2nd, 3rd"}, {"cardinalText", "One, Two, Three"}, {"ordinalText", "First, Second, Third"}};
    if (count) *count = int(sizeof names / sizeof names[0]);
    return names;
}

bool defineBulletDialog(QWidget *parent, ListLevelLook *look, const SymbolPicker &pick)
{
    QDialog d(parent);
    d.setObjectName(QStringLiteral("defineBulletDialog"));
    d.setWindowTitle(tr("Define New Bullet"));
    auto *form = new QFormLayout;
    auto *character = new QLineEdit(look->text.left(2), &d);
    character->setObjectName(QStringLiteral("bulletCharacter"));
    character->setMaxLength(2);
    auto *font = new QFontComboBox(&d);
    font->setObjectName(QStringLiteral("bulletFont"));
    auto *useFont = new QCheckBox(tr("Use this font for the bullet"), &d);
    useFont->setChecked(!look->font.isEmpty());
    if (!look->font.isEmpty()) font->setCurrentFont(QFont(look->font));
    font->setEnabled(useFont->isChecked());
    QObject::connect(useFont, &QCheckBox::toggled, font, &QWidget::setEnabled);
    auto *row = new QHBoxLayout;
    row->addWidget(character);
    if (pick) {
        auto *symbol = new QPushButton(tr("Symbol…"), &d);
        symbol->setObjectName(QStringLiteral("bulletSymbol"));
        QObject::connect(symbol, &QPushButton::clicked, &d, [&] {
            QString ch, f;
            if (!pick(&d, &ch, &f) || ch.isEmpty()) return;
            character->setText(ch.left(2));
            useFont->setChecked(!f.isEmpty());
            if (!f.isEmpty()) font->setCurrentFont(QFont(f));
        });
        row->addWidget(symbol);
    }
    form->addRow(tr("Bullet character:"), row);
    form->addRow(useFont);
    form->addRow(tr("Font:"), font);
    auto *size = new QDoubleSpinBox(&d);
    size->setObjectName(QStringLiteral("bulletSize"));
    size->setRange(0, 200);
    size->setDecimals(1);
    size->setSuffix(tr(" pt"));
    size->setSpecialValueText(tr("Same as the text"));
    size->setValue(look->sizePoints);
    form->addRow(tr("Size:"), size);
    QColor color = look->color;
    auto *colorButton = new QPushButton(&d);
    colorButton->setObjectName(QStringLiteral("bulletColor"));
    auto showColor = [&] { colorButton->setText(color.isValid() ? color.name().toUpper() : tr("Same as the text")); };
    showColor();
    QObject::connect(colorButton, &QPushButton::clicked, &d, [&] {
        const QColor picked = QColorDialog::getColor(color.isValid() ? color : Qt::black, &d, tr("Bullet Color"));
        if (picked.isValid()) color = picked;
        showColor();
    });
    form->addRow(tr("Color:"), colorButton);
    auto *align = alignmentBox(look->align, &d);
    form->addRow(tr("Alignment:"), align);
    auto *column = new QVBoxLayout(&d);
    column->addLayout(form);
    column->addWidget(buttons(&d));
    if (d.exec() != QDialog::Accepted || character->text().isEmpty()) return false;
    look->bullet = true;
    look->text = character->text();
    look->font = useFont->isChecked() ? font->currentFont().family() : QString();
    look->sizePoints = size->value();
    look->color = color;
    look->align = align->currentData().toString();
    return true;
}

bool defineNumberFormatDialog(QWidget *parent, ListLevelLook *look)
{
    QDialog d(parent);
    d.setObjectName(QStringLiteral("defineNumberFormatDialog"));
    d.setWindowTitle(tr("Define New Number Format"));
    auto *form = new QFormLayout;
    auto *style = new QComboBox(&d);
    style->setObjectName(QStringLiteral("numberStyle"));
    int count = 0;
    const NumberStyleName *names = numberStyleNames(&count);
    for (int i = 0; i < count; ++i) style->addItem(QString::fromLatin1(names[i].example), QString::fromLatin1(names[i].id));
    style->setCurrentIndex(std::max(0, style->findData(look->numFmt)));
    form->addRow(tr("Number style:"), style);
    auto *text = new QLineEdit(look->bullet || look->text.isEmpty() ? QStringLiteral("%1.") : look->text, &d);
    text->setObjectName(QStringLiteral("numberFormat"));
    form->addRow(tr("Number format (%1 is the number):"), text);
    auto *align = alignmentBox(look->align, &d);
    form->addRow(tr("Alignment:"), align);
    auto *start = new QSpinBox(&d);
    start->setObjectName(QStringLiteral("numberStart"));
    start->setRange(0, 9999);
    start->setValue(look->start);
    form->addRow(tr("Start at:"), start);
    auto *preview = new QLabel(&d);
    auto update = [&] {
        QString shown = text->text();
        shown.replace(QStringLiteral("%1"), style->currentText().section(QLatin1String(", "), 0, 0));
        preview->setText(shown);
    };
    QObject::connect(text, &QLineEdit::textChanged, &d, update);
    QObject::connect(style, &QComboBox::currentIndexChanged, &d, update);
    update();
    form->addRow(tr("Preview:"), preview);
    auto *column = new QVBoxLayout(&d);
    column->addLayout(form);
    column->addWidget(buttons(&d));
    if (d.exec() != QDialog::Accepted) return false;
    QString format = text->text();
    if (!format.contains(QLatin1String("%1"))) format += QStringLiteral("%1");   // (a format with no number would number nothing)
    look->bullet = false;
    look->numFmt = style->currentData().toString();
    look->text = format;
    look->align = align->currentData().toString();
    look->start = start->value();
    return true;
}

bool setNumberingValueDialog(QWidget *parent, bool canContinue, NumberingValueChoice *choice)
{
    QDialog d(parent);
    d.setObjectName(QStringLiteral("setNumberingValueDialog"));
    d.setWindowTitle(tr("Set Numbering Value"));
    auto *column = new QVBoxLayout(&d);
    auto *fresh = new QRadioButton(tr("Start new list"), &d);
    fresh->setObjectName(QStringLiteral("startNewList"));
    auto *again = new QRadioButton(tr("Continue previous list"), &d);
    again->setObjectName(QStringLiteral("continuePreviousList"));
    again->setEnabled(canContinue);
    (choice->continuePrevious && canContinue ? again : fresh)->setChecked(true);
    column->addWidget(fresh);
    column->addWidget(again);
    auto *set = new QCheckBox(tr("Set the value to:"), &d);
    set->setObjectName(QStringLiteral("setValue"));
    set->setChecked(choice->setValue || !canContinue);
    auto *value = new QSpinBox(&d);
    value->setObjectName(QStringLiteral("numberValue"));
    value->setRange(0, 9999);
    value->setValue(choice->value);
    value->setEnabled(set->isChecked());
    QObject::connect(set, &QCheckBox::toggled, value, &QWidget::setEnabled);
    auto *row = new QHBoxLayout;
    row->addWidget(set);
    row->addWidget(value);
    column->addLayout(row);
    column->addWidget(buttons(&d));
    if (d.exec() != QDialog::Accepted) return false;
    choice->continuePrevious = again->isChecked();
    choice->setValue = set->isChecked();
    choice->value = value->value();
    return true;
}

} // namespace jp
