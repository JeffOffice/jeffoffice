#include "app/multileveldialog.h"

#include "app/listdialogs.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QColorDialog>
#include <QFontComboBox>
#include <QFormLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QSpinBox>
#include <QVBoxLayout>

using namespace jp::office::wml;

namespace jp::write {

namespace {

} // namespace

bool multilevelListDialog(QWidget *parent, AbstractNum *definition, const QList<QPair<QString, QString>> &styles)
{
    AbstractNum work = *definition;
    for (Level &l : work.levels) {
        if (l.present) continue;
        l.present = true;
        l.numFmt = QStringLiteral("decimal");
        l.text = QStringLiteral("%1.");
    }
    work.multiLevelType = QStringLiteral("multilevel");
    QDialog d(parent);
    d.setObjectName(QStringLiteral("multilevelListDialog"));
    d.setWindowTitle(QCoreApplication::translate("jp::write::MultilevelDialog", "Define New Multilevel List"));
    auto *levels = new QListWidget(&d);
    levels->setObjectName(QStringLiteral("levels"));
    for (int i = 1; i <= 9; ++i) levels->addItem(QString::number(i));
    levels->setMaximumWidth(60);
    auto *form = new QFormLayout;
    auto *style = new QComboBox(&d);
    style->setObjectName(QStringLiteral("numberStyle"));
    int count = 0;
    const NumberStyleName *names = numberStyleNames(&count);
    for (int i = 0; i < count; ++i) style->addItem(QString::fromLatin1(names[i].example), QString::fromLatin1(names[i].id));
    style->addItem(QCoreApplication::translate("jp::write::MultilevelDialog", "Bullet"), QStringLiteral("bullet"));
    style->addItem(QCoreApplication::translate("jp::write::MultilevelDialog", "(none)"), QStringLiteral("none"));
    form->addRow(QCoreApplication::translate("jp::write::MultilevelDialog", "Number style for this level:"), style);
    auto *text = new QLineEdit(&d);
    text->setObjectName(QStringLiteral("numberFormat"));
    form->addRow(QCoreApplication::translate("jp::write::MultilevelDialog", "Number format (%1 to %9 are the numbers of the levels):"), text);
    auto *include = new QComboBox(&d);
    include->setObjectName(QStringLiteral("includeLevel"));
    form->addRow(QCoreApplication::translate("jp::write::MultilevelDialog", "Include the number from level:"), include);
    auto *start = new QSpinBox(&d);
    start->setObjectName(QStringLiteral("startAt"));
    start->setRange(0, 9999);
    form->addRow(QCoreApplication::translate("jp::write::MultilevelDialog", "Start at:"), start);
    auto *restart = new QComboBox(&d);
    restart->setObjectName(QStringLiteral("restartAfter"));
    form->addRow(QCoreApplication::translate("jp::write::MultilevelDialog", "Restart the list after:"), restart);
    auto *align = new QComboBox(&d);
    align->setObjectName(QStringLiteral("numberAlignment"));
    align->addItem(QCoreApplication::translate("jp::write::MultilevelDialog", "Left"), QStringLiteral("left"));
    align->addItem(QCoreApplication::translate("jp::write::MultilevelDialog", "Centered"), QStringLiteral("center"));
    align->addItem(QCoreApplication::translate("jp::write::MultilevelDialog", "Right"), QStringLiteral("right"));
    form->addRow(QCoreApplication::translate("jp::write::MultilevelDialog", "Number alignment:"), align);
    auto *position = new QDoubleSpinBox(&d);
    position->setObjectName(QStringLiteral("numberPosition"));
    position->setRange(0, 1500);
    position->setDecimals(2);
    position->setSuffix(QCoreApplication::translate("jp::write::MultilevelDialog", " pt"));
    form->addRow(QCoreApplication::translate("jp::write::MultilevelDialog", "Number position at:"), position);
    auto *indent = new QDoubleSpinBox(&d);
    indent->setObjectName(QStringLiteral("textIndent"));
    indent->setRange(0, 1500);
    indent->setDecimals(2);
    indent->setSuffix(QCoreApplication::translate("jp::write::MultilevelDialog", " pt"));
    form->addRow(QCoreApplication::translate("jp::write::MultilevelDialog", "Text indent at:"), indent);
    auto *follow = new QComboBox(&d);
    follow->setObjectName(QStringLiteral("followWith"));
    follow->addItem(QCoreApplication::translate("jp::write::MultilevelDialog", "Tab character"), QStringLiteral("tab"));
    follow->addItem(QCoreApplication::translate("jp::write::MultilevelDialog", "Space"), QStringLiteral("space"));
    follow->addItem(QCoreApplication::translate("jp::write::MultilevelDialog", "Nothing"), QStringLiteral("nothing"));
    form->addRow(QCoreApplication::translate("jp::write::MultilevelDialog", "Follow the number with:"), follow);
    auto *legal = new QCheckBox(QCoreApplication::translate("jp::write::MultilevelDialog", "Legal style numbering (1.1.1 in place of I.A.1)"), &d);
    legal->setObjectName(QStringLiteral("legalStyle"));
    form->addRow(legal);
    // How the number looks: its font, size, bold, italic, and color (a bullet's symbol font too).
    auto *fontCheck = new QCheckBox(QCoreApplication::translate("jp::write::MultilevelDialog", "Use a font of its own for the number"), &d);
    fontCheck->setObjectName(QStringLiteral("numberFontOn"));
    auto *font = new QFontComboBox(&d);
    font->setObjectName(QStringLiteral("numberFont"));
    form->addRow(fontCheck);
    form->addRow(QCoreApplication::translate("jp::write::MultilevelDialog", "Font:"), font);
    auto *size = new QDoubleSpinBox(&d);
    size->setObjectName(QStringLiteral("numberSize"));
    size->setRange(0, 200);
    size->setDecimals(1);
    size->setSuffix(QCoreApplication::translate("jp::write::MultilevelDialog", " pt"));
    size->setSpecialValueText(QCoreApplication::translate("jp::write::MultilevelDialog", "Same as the text"));
    form->addRow(QCoreApplication::translate("jp::write::MultilevelDialog", "Size:"), size);
    auto *bold = new QCheckBox(QCoreApplication::translate("jp::write::MultilevelDialog", "Bold"), &d);
    bold->setObjectName(QStringLiteral("numberBold"));
    auto *italic = new QCheckBox(QCoreApplication::translate("jp::write::MultilevelDialog", "Italic"), &d);
    italic->setObjectName(QStringLiteral("numberItalic"));
    auto *emphasis = new QHBoxLayout;
    emphasis->addWidget(bold);
    emphasis->addWidget(italic);
    form->addRow(emphasis);
    auto *colorButton = new QPushButton(&d);
    colorButton->setObjectName(QStringLiteral("numberColor"));
    QString colorName;   // "RRGGBB", empty for the text's own
    auto showColor = [&] { colorButton->setText(colorName.isEmpty() ? QCoreApplication::translate("jp::write::MultilevelDialog", "Same as the text") : QLatin1Char('#') + colorName); };
    form->addRow(QCoreApplication::translate("jp::write::MultilevelDialog", "Color:"), colorButton);
    auto *link = new QComboBox(&d);
    link->setObjectName(QStringLiteral("linkToStyle"));
    link->addItem(QCoreApplication::translate("jp::write::MultilevelDialog", "(none)"), QString());
    for (const auto &s : styles) link->addItem(s.second, s.first);
    form->addRow(QCoreApplication::translate("jp::write::MultilevelDialog", "Link level to style:"), link);
    auto *preview = new QLabel(&d);
    form->addRow(QCoreApplication::translate("jp::write::MultilevelDialog", "Preview:"), preview);

    int current = 0;
    bool loading = false;
    auto shown = [&] {
        QString out;
        for (int i = 0; i < 9; ++i) {
            const Level &l = work.levels[i];
            if (l.numFmt == QLatin1String("bullet") || l.numFmt == QLatin1String("none")) {
                out += QString(i * 2, QLatin1Char(' ')) + (l.numFmt == QLatin1String("none") ? QString() : l.text) + QLatin1Char('\n');
                continue;
            }
            QString t = l.text;
            for (int k = 1; k <= 9; ++k) {
                const Level &of = work.levels[k - 1];
                const char *sample = of.numFmt == QLatin1String("upperRoman") ? "I" : of.numFmt == QLatin1String("lowerRoman") ? "i" : of.numFmt == QLatin1String("upperLetter") ? "A" :
                                     of.numFmt == QLatin1String("lowerLetter") ? "a" : "1";
                t.replace(QStringLiteral("%") + QString::number(k), QString::fromLatin1(sample));
            }
            out += QString(i * 2, QLatin1Char(' ')) + t + QLatin1Char('\n');
            if (i >= 3) break;
        }
        return out.trimmed();
    };
    // The fields show the level `current`; what is in them is kept into `work` when the level changes and when the box is accepted.
    auto store = [&] {
        if (loading) return;
        Level &l = work.levels[current];
        l.numFmt = style->currentData().toString();
        l.text = text->text();
        l.start = start->value();
        const int r = restart->currentData().toInt();
        if (r == -1) l.restart.reset();
        else l.restart = r;
        l.jc = align->currentData().toString();
        const int number = int(std::lround(position->value() * 20)), textAt = int(std::lround(indent->value() * 20));
        l.pPr.indLeft = textAt;
        l.pPr.indFirst = number - textAt;
        l.suff = follow->currentData().toString();
        l.legal = legal->isChecked();
        l.pStyle = link->currentData().toString();
        if (fontCheck->isChecked()) {
            Fonts f = l.rPr.fonts.value_or(Fonts());
            f.ascii = f.hAnsi = font->currentFont().family();
            f.asciiTheme.clear();
            f.hAnsiTheme.clear();
            l.rPr.fonts = f;
        } else if (l.rPr.fonts && !l.rPr.fonts->hasOtherScripts()) {
            l.rPr.fonts.reset();
        } else if (l.rPr.fonts) {
            l.rPr.fonts->ascii.clear();
            l.rPr.fonts->hAnsi.clear();
        }
        if (size->value() > 0) l.rPr.size = int(std::lround(size->value() * 2));
        else l.rPr.size.reset();
        if (bold->isChecked()) l.rPr.bold = true;
        else l.rPr.bold.reset();
        if (italic->isChecked()) l.rPr.italic = true;
        else l.rPr.italic.reset();
        if (!colorName.isEmpty()) l.rPr.color = Color{colorName, {}, {}, {}};
        else l.rPr.color.reset();
        preview->setText(shown());
    };
    auto load = [&](int level) {
        loading = true;
        current = level;
        const Level &l = work.levels[level];
        if (style->findData(l.numFmt) < 0) style->addItem(l.numFmt, l.numFmt);   // (a style this box has no name for stays as the file has it)
        style->setCurrentIndex(std::max(0, style->findData(l.numFmt)));
        text->setText(l.text);
        start->setValue(l.start);
        include->clear();
        include->addItem(QString());
        for (int k = 1; k <= level + 1; ++k) include->addItem(QCoreApplication::translate("jp::write::MultilevelDialog", "Level %1").arg(k), k);
        restart->clear();
        restart->addItem(QCoreApplication::translate("jp::write::MultilevelDialog", "Any higher level"), -1);
        restart->addItem(QCoreApplication::translate("jp::write::MultilevelDialog", "Never"), 0);
        for (int k = 1; k <= level; ++k) restart->addItem(QCoreApplication::translate("jp::write::MultilevelDialog", "Level %1").arg(k), k);
        restart->setCurrentIndex(std::max(0, restart->findData(l.restart ? *l.restart : -1)));
        align->setCurrentIndex(std::max(0, align->findData(l.jc)));
        const int left = l.pPr.indLeft.value_or(0), first = l.pPr.indFirst.value_or(0);
        position->setValue((left + first) / 20.0);
        indent->setValue(left / 20.0);
        follow->setCurrentIndex(std::max(0, follow->findData(l.suff)));
        legal->setChecked(l.legal);
        link->setCurrentIndex(std::max(0, link->findData(l.pStyle)));
        const QString family = l.rPr.fonts ? (!l.rPr.fonts->ascii.isEmpty() ? l.rPr.fonts->ascii : l.rPr.fonts->hAnsi) : QString();
        fontCheck->setChecked(!family.isEmpty());
        if (!family.isEmpty()) font->setCurrentFont(QFont(family));
        font->setEnabled(fontCheck->isChecked());
        size->setValue(l.rPr.size ? *l.rPr.size / 2.0 : 0);
        bold->setChecked(l.rPr.bold.value_or(false));
        italic->setChecked(l.rPr.italic.value_or(false));
        colorName = l.rPr.color && l.rPr.color->val != QLatin1String("auto") ? l.rPr.color->val : QString();
        showColor();
        loading = false;
        preview->setText(shown());
    };
    QObject::connect(levels, &QListWidget::currentRowChanged, &d, [&](int row) {
        if (row < 0) return;
        load(row);   // (each field stores its change as it is made, so the level that is left is already kept)
    });
    QObject::connect(include, &QComboBox::activated, &d, [&](int) {
        const int k = include->currentData().toInt();
        if (k > 0) text->setText(text->text() + QStringLiteral("%") + QString::number(k));
        include->setCurrentIndex(0);
    });
    for (QComboBox *box : {style, restart, align, follow, link}) QObject::connect(box, &QComboBox::currentIndexChanged, &d, [&] { store(); });
    for (QDoubleSpinBox *box : {position, indent}) QObject::connect(box, &QDoubleSpinBox::valueChanged, &d, [&] { store(); });
    QObject::connect(start, &QSpinBox::valueChanged, &d, [&] { store(); });
    QObject::connect(text, &QLineEdit::textChanged, &d, [&] { store(); });
    QObject::connect(legal, &QCheckBox::toggled, &d, [&] { store(); });
    QObject::connect(fontCheck, &QCheckBox::toggled, &d, [&](bool on) {
        font->setEnabled(on);
        store();
    });
    QObject::connect(font, &QFontComboBox::currentFontChanged, &d, [&] { store(); });
    QObject::connect(size, &QDoubleSpinBox::valueChanged, &d, [&] { store(); });
    QObject::connect(bold, &QCheckBox::toggled, &d, [&] { store(); });
    QObject::connect(italic, &QCheckBox::toggled, &d, [&] { store(); });
    QObject::connect(colorButton, &QPushButton::clicked, &d, [&] {
        const QColor picked = QColorDialog::getColor(colorName.isEmpty() ? QColor(Qt::black) : QColor(QLatin1Char('#') + colorName), &d, QCoreApplication::translate("jp::write::MultilevelDialog", "Number Color"));
        if (picked.isValid()) colorName = picked.name().mid(1).toUpper();
        showColor();
        store();
    });

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    auto *row = new QHBoxLayout;
    row->addWidget(levels);
    row->addLayout(form, 1);
    auto *column = new QVBoxLayout(&d);
    column->addLayout(row);
    column->addWidget(buttons);
    load(0);
    levels->setCurrentRow(0);
    if (d.exec() != QDialog::Accepted) return false;
    for (int i = 0; i < 9; ++i)
        if (work.levels[i].numFmt != QLatin1String("bullet") && work.levels[i].numFmt != QLatin1String("none") && !work.levels[i].text.contains(QLatin1Char('%'))) work.levels[i].text += QStringLiteral("%") + QString::number(i + 1);
    *definition = work;
    return true;
}

} // namespace jp::write
