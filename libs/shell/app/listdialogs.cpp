#include "app/listdialogs.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFontComboBox>
#include <QImage>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace jp {

namespace {

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
    box->addItem(QCoreApplication::translate("jp::ListDialogs", "Left"), QStringLiteral("left"));
    box->addItem(QCoreApplication::translate("jp::ListDialogs", "Centered"), QStringLiteral("center"));
    box->addItem(QCoreApplication::translate("jp::ListDialogs", "Right"), QStringLiteral("right"));
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

bool defineBulletDialog(QWidget *parent, ListLevelLook *look, const SymbolPicker &pick, ListBoxes mode)
{
    const bool plain = mode == ListBoxes::Plain;
    QDialog d(parent);
    d.setObjectName(QStringLiteral("defineBulletDialog"));
    d.setWindowTitle(QCoreApplication::translate("jp::ListDialogs", "Define New Bullet"));
    auto *form = new QFormLayout;
    auto *character = new QLineEdit(look->text.left(2), &d);
    character->setObjectName(QStringLiteral("bulletCharacter"));
    character->setMaxLength(2);
    auto *font = new QFontComboBox(&d);
    font->setObjectName(QStringLiteral("bulletFont"));
    auto *useFont = new QCheckBox(QCoreApplication::translate("jp::ListDialogs", "Use this font for the bullet"), &d);
    useFont->setChecked(!look->font.isEmpty());
    if (!look->font.isEmpty()) font->setCurrentFont(QFont(look->font));
    font->setEnabled(useFont->isChecked());
    QObject::connect(useFont, &QCheckBox::toggled, font, &QWidget::setEnabled);
    auto *row = new QHBoxLayout;
    row->addWidget(character);
    if (pick) {
        auto *symbol = new QPushButton(QCoreApplication::translate("jp::ListDialogs", "Symbol…"), &d);
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
    // A picture in place of a character (not for the apps whose files keep only characters).
    QByteArray picture = look->picture;
    auto *pictureButton = new QPushButton(picture.isEmpty() ? QCoreApplication::translate("jp::ListDialogs", "Picture…") : QCoreApplication::translate("jp::ListDialogs", "Picture… (chosen)"), &d);
    pictureButton->setObjectName(QStringLiteral("bulletPicture"));
    QObject::connect(pictureButton, &QPushButton::clicked, &d, [&] {
        const QString file = QFileDialog::getOpenFileName(&d, QCoreApplication::translate("jp::ListDialogs", "Choose a Picture for the Bullet"), QString(), QCoreApplication::translate("jp::ListDialogs", "Pictures (*.png *.jpg *.jpeg *.gif *.bmp)"));
        if (file.isEmpty()) return;
        QFile f(file);
        if (!f.open(QIODevice::ReadOnly) || f.size() > 2000000) return;
        const QByteArray bytes = f.readAll();
        if (QImage::fromData(bytes).isNull()) return;
        picture = bytes;
        pictureButton->setText(QCoreApplication::translate("jp::ListDialogs", "Picture… (chosen)"));
    });
    if (mode != ListBoxes::Plain) row->addWidget(pictureButton);
    form->addRow(QCoreApplication::translate("jp::ListDialogs", "Bullet character:"), row);
    form->addRow(useFont);
    form->addRow(QCoreApplication::translate("jp::ListDialogs", "Font:"), font);
    auto *size = new QDoubleSpinBox(&d);
    size->setObjectName(QStringLiteral("bulletSize"));
    size->setRange(0, 200);
    size->setDecimals(1);
    size->setSuffix(QCoreApplication::translate("jp::ListDialogs", " pt"));
    size->setSpecialValueText(QCoreApplication::translate("jp::ListDialogs", "Same as the text"));
    size->setValue(look->sizePoints);
    form->addRow(QCoreApplication::translate("jp::ListDialogs", "Size:"), size);
    QColor color = look->color;
    auto *colorButton = new QPushButton(&d);
    colorButton->setObjectName(QStringLiteral("bulletColor"));
    auto showColor = [&] { colorButton->setText(color.isValid() ? color.name().toUpper() : QCoreApplication::translate("jp::ListDialogs", "Same as the text")); };
    showColor();
    QObject::connect(colorButton, &QPushButton::clicked, &d, [&] {
        const QColor picked = QColorDialog::getColor(color.isValid() ? color : Qt::black, &d, QCoreApplication::translate("jp::ListDialogs", "Bullet Color"));
        if (picked.isValid()) color = picked;
        showColor();
    });
    form->addRow(QCoreApplication::translate("jp::ListDialogs", "Color:"), colorButton);
    auto *align = alignmentBox(look->align, &d);
    if (mode == ListBoxes::Full) form->addRow(QCoreApplication::translate("jp::ListDialogs", "Alignment:"), align);
    auto *column = new QVBoxLayout(&d);
    column->addLayout(form);
    column->addWidget(buttons(&d));
    if (d.exec() != QDialog::Accepted || character->text().isEmpty()) return false;
    look->bullet = true;
    look->picture = picture;
    look->text = character->text();
    look->font = useFont->isChecked() ? font->currentFont().family() : QString();
    look->sizePoints = size->value();
    look->color = color;
    look->align = align->currentData().toString();
    return true;
}

bool defineNumberFormatDialog(QWidget *parent, ListLevelLook *look, ListBoxes mode)
{
    const bool plain = mode == ListBoxes::Plain;
    QDialog d(parent);
    d.setObjectName(QStringLiteral("defineNumberFormatDialog"));
    d.setWindowTitle(QCoreApplication::translate("jp::ListDialogs", "Define New Number Format"));
    auto *form = new QFormLayout;
    auto *style = new QComboBox(&d);
    style->setObjectName(QStringLiteral("numberStyle"));
    int count = 0;
    const NumberStyleName *names = numberStyleNames(&count);
    for (int i = 0; i < (mode != ListBoxes::Full ? std::min(count, 5) : count); ++i) style->addItem(QString::fromLatin1(names[i].example), QString::fromLatin1(names[i].id));
    style->setCurrentIndex(std::max(0, style->findData(look->numFmt)));
    form->addRow(QCoreApplication::translate("jp::ListDialogs", "Number style:"), style);
    auto *text = new QLineEdit(look->bullet || look->text.isEmpty() ? QStringLiteral("%1.") : look->text, &d);
    text->setObjectName(QStringLiteral("numberFormat"));
    auto *punctuation = new QComboBox(&d);
    punctuation->setObjectName(QStringLiteral("numberPunctuation"));
    for (const char *f : {"%1.", "%1)", "(%1)", "%1"}) punctuation->addItem(QString::fromLatin1(f).replace(QStringLiteral("%1"), QStringLiteral("1")), QString::fromLatin1(f));
    punctuation->setCurrentIndex(std::max(0, punctuation->findData(look->text)));
    if (plain) form->addRow(QCoreApplication::translate("jp::ListDialogs", "Punctuation:"), punctuation);
    else form->addRow(QCoreApplication::translate("jp::ListDialogs", "Number format (%1 is the number):"), text);
    auto *align = alignmentBox(look->align, &d);
    if (mode == ListBoxes::Full) form->addRow(QCoreApplication::translate("jp::ListDialogs", "Alignment:"), align);
    auto *start = new QSpinBox(&d);
    start->setObjectName(QStringLiteral("numberStart"));
    start->setRange(0, 9999);
    start->setValue(look->start);
    form->addRow(QCoreApplication::translate("jp::ListDialogs", "Start at:"), start);
    auto *preview = new QLabel(&d);
    auto update = [&] {
        QString shown = text->text();
        if (plain) shown = punctuation->currentData().toString();
        shown.replace(QStringLiteral("%1"), style->currentText().section(QLatin1String(", "), 0, 0));
        preview->setText(shown);
    };
    QObject::connect(punctuation, &QComboBox::currentIndexChanged, &d, update);
    QObject::connect(text, &QLineEdit::textChanged, &d, update);
    QObject::connect(style, &QComboBox::currentIndexChanged, &d, update);
    update();
    form->addRow(QCoreApplication::translate("jp::ListDialogs", "Preview:"), preview);
    auto *column = new QVBoxLayout(&d);
    column->addLayout(form);
    column->addWidget(buttons(&d));
    if (d.exec() != QDialog::Accepted) return false;
    QString format = plain ? punctuation->currentData().toString() : text->text();
    if (!format.contains(QLatin1String("%1"))) format += QStringLiteral("%1");   // (a format with no number would number nothing)
    look->bullet = false;
    look->numFmt = style->currentData().toString();
    look->text = format;
    look->align = align->currentData().toString();
    look->start = start->value();
    return true;
}

bool defineMultilevelPlainDialog(QWidget *parent, QVector<ListLevelLook> *levels, const SymbolPicker &pick, ListBoxes mode)
{
    const bool typed = mode == ListBoxes::Jpub;
    QVector<ListLevelLook> work = *levels;
    while (work.size() < 9) work.push_back(work.isEmpty() ? ListLevelLook() : work.last());
    QDialog d(parent);
    d.setObjectName(QStringLiteral("defineMultilevelDialog"));
    d.setWindowTitle(QCoreApplication::translate("jp::ListDialogs", "Define New Multilevel List"));
    auto *list = new QListWidget(&d);
    list->setObjectName(QStringLiteral("levels"));
    for (int i = 1; i <= 9; ++i) list->addItem(QCoreApplication::translate("jp::ListDialogs", "Level %1").arg(i));
    list->setMaximumWidth(110);
    auto *form = new QFormLayout;
    auto *kind = new QComboBox(&d);
    kind->setObjectName(QStringLiteral("levelKind"));
    kind->addItem(QCoreApplication::translate("jp::ListDialogs", "Bullet"), true);
    kind->addItem(QCoreApplication::translate("jp::ListDialogs", "Number"), false);
    form->addRow(QCoreApplication::translate("jp::ListDialogs", "Level is a:"), kind);
    auto *character = new QLineEdit(&d);
    character->setObjectName(QStringLiteral("bulletCharacter"));
    character->setMaxLength(2);
    auto *row = new QHBoxLayout;
    row->addWidget(character);
    QString pickedFont;
    if (pick) {
        auto *symbol = new QPushButton(QCoreApplication::translate("jp::ListDialogs", "Symbol…"), &d);
        QObject::connect(symbol, &QPushButton::clicked, &d, [&] {
            QString ch, f;
            if (!pick(&d, &ch, &f) || ch.isEmpty()) return;
            character->setText(ch.left(2));
            pickedFont = f;
        });
        row->addWidget(symbol);
    }
    form->addRow(QCoreApplication::translate("jp::ListDialogs", "Bullet character:"), row);
    auto *style = new QComboBox(&d);
    style->setObjectName(QStringLiteral("numberStyle"));
    int count = 0;
    const NumberStyleName *names = numberStyleNames(&count);
    for (int i = 0; i < std::min(count, 5); ++i) style->addItem(QString::fromLatin1(names[i].example), QString::fromLatin1(names[i].id));
    form->addRow(QCoreApplication::translate("jp::ListDialogs", "Number style:"), style);
    auto *punctuation = new QComboBox(&d);
    punctuation->setObjectName(QStringLiteral("numberPunctuation"));
    for (const char *f : {"%1.", "%1)", "(%1)", "%1"}) punctuation->addItem(QString::fromLatin1(f).replace(QStringLiteral("%1"), QStringLiteral("1")), QString::fromLatin1(f));
    auto *format = new QLineEdit(&d);
    format->setObjectName(QStringLiteral("numberFormat"));
    if (typed) form->addRow(QCoreApplication::translate("jp::ListDialogs", "Number format (%1 is the number):"), format);
    else form->addRow(QCoreApplication::translate("jp::ListDialogs", "Punctuation:"), punctuation);
    auto *start = new QSpinBox(&d);
    start->setObjectName(QStringLiteral("numberStart"));
    start->setRange(1, 9999);
    form->addRow(QCoreApplication::translate("jp::ListDialogs", "Start at:"), start);
    int current = 0;
    bool loading = false;
    auto store = [&] {
        if (loading) return;
        ListLevelLook &l = work[current];
        l.bullet = kind->currentData().toBool();
        if (l.bullet) {
            if (!character->text().isEmpty()) l.text = character->text();
            if (!pickedFont.isEmpty()) l.font = pickedFont;
        } else {
            l.numFmt = style->currentData().toString();
            l.text = typed ? format->text() : punctuation->currentData().toString();
            if (!l.text.contains(QLatin1String("%1"))) l.text += QStringLiteral("%1");
            l.start = start->value();
        }
    };
    auto load = [&](int level) {
        loading = true;
        current = level;
        const ListLevelLook &l = work[level];
        kind->setCurrentIndex(l.bullet ? 0 : 1);
        character->setText(l.bullet ? l.text : QStringLiteral("•"));
        style->setCurrentIndex(std::max(0, style->findData(l.numFmt)));
        punctuation->setCurrentIndex(std::max(0, punctuation->findData(l.bullet ? QStringLiteral("%1.") : l.text)));
        format->setText(l.bullet ? QStringLiteral("%1.") : l.text);
        start->setValue(std::max(1, l.start));
        loading = false;
    };
    QObject::connect(list, &QListWidget::currentRowChanged, &d, [&](int row) {
        if (row >= 0) load(row);
    });
    QObject::connect(kind, &QComboBox::currentIndexChanged, &d, [&] { store(); });
    QObject::connect(character, &QLineEdit::textChanged, &d, [&] { store(); });
    QObject::connect(style, &QComboBox::currentIndexChanged, &d, [&] { store(); });
    QObject::connect(punctuation, &QComboBox::currentIndexChanged, &d, [&] { store(); });
    QObject::connect(format, &QLineEdit::textChanged, &d, [&] { store(); });
    QObject::connect(start, &QSpinBox::valueChanged, &d, [&] { store(); });
    auto *outer = new QHBoxLayout;
    outer->addWidget(list);
    outer->addLayout(form, 1);
    auto *column = new QVBoxLayout(&d);
    column->addLayout(outer);
    column->addWidget(buttons(&d));
    load(0);
    list->setCurrentRow(0);
    if (d.exec() != QDialog::Accepted) return false;
    *levels = work;
    return true;
}

bool setNumberingValueDialog(QWidget *parent, bool canContinue, NumberingValueChoice *choice)
{
    QDialog d(parent);
    d.setObjectName(QStringLiteral("setNumberingValueDialog"));
    d.setWindowTitle(QCoreApplication::translate("jp::ListDialogs", "Set Numbering Value"));
    auto *column = new QVBoxLayout(&d);
    auto *fresh = new QRadioButton(QCoreApplication::translate("jp::ListDialogs", "Start new list"), &d);
    fresh->setObjectName(QStringLiteral("startNewList"));
    auto *again = new QRadioButton(QCoreApplication::translate("jp::ListDialogs", "Continue previous list"), &d);
    again->setObjectName(QStringLiteral("continuePreviousList"));
    again->setEnabled(canContinue);
    (choice->continuePrevious && canContinue ? again : fresh)->setChecked(true);
    column->addWidget(fresh);
    column->addWidget(again);
    auto *set = new QCheckBox(QCoreApplication::translate("jp::ListDialogs", "Set the value to:"), &d);
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
