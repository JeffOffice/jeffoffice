// Create Style and Modify Style: the dialog that gives a style its name, what it is based on, the style that follows it, its font and paragraph look, and whether it is in the gallery and
// updates itself. What it makes of the person's choices is a definition the commands of core/styleops.h take; only what the person changed is put in the definition, so that a dialog
// closed with OK and nothing changed leaves the style, and the file, as they were.

#include "app/writedialogs.h"

#include "app/widgets.h"
#include "core/editops.h"
#include "core/styleops.h"
#include "core/wordformat.h"
#include "core/writedoc.h"
#include "text/textprops.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextBlock>
#include <QVBoxLayout>
#include <functional>

using namespace jp::office::wml;

namespace jp::write {

namespace {

// A color as the dialog's button shows it.
void showColor(QPushButton *b, const QColor &c)
{
    QPixmap pm(14, 14);
    pm.fill(c.isValid() ? c : QColor(Qt::black));
    b->setIcon(QIcon(pm));
    b->setText(c.isValid() ? c.name(QColor::HexRgb).toUpper() : QCoreApplication::translate("jp::write::StyleDialog", "Automatic"));
}

int twips(double points) { return int(std::lround(points * 20)); }

// A dialog that is accepted only when what it was asked for can be done (its OK, and an accept from code, run `tryAccept`; a refusal leaves it open, with the reason shown).
class StyleDialog : public QDialog {
public:
    using QDialog::QDialog;
    std::function<bool()> tryAccept;
    void accept() override
    {
        if (!tryAccept || tryAccept()) QDialog::accept();
    }
};

} // namespace

bool styleDialog(QWidget *parent, WriteDoc *doc, QTextCursor *cursor, const QString &modifyId)
{
    if (!doc || !cursor) return false;
    const bool modify = !modifyId.isEmpty();
    if (modify && !doc->ensureStyle(modifyId)) return false;
    Style start = modify ? *doc->styleSheet().find(modifyId) : styleFromSelection(*doc, *cursor, QString());
    const bool character = start.type == QLatin1String("character");
    // The look the controls begin with: what the text of the style (or of the selection) comes to.
    QString paragraphStyle = modify ? (character ? doc->defaultParagraphStyle() : modifyId) : paragraphStyleAt(*doc, *cursor);
    QTextCursor at(*cursor);
    if (at.hasSelection()) at.setPosition(at.selectionStart() + 1);
    const QTextCharFormat look = modify ? doc->styleCharFormat(paragraphStyle, character ? modifyId : QString()) : withoutAtom(at.charFormat());
    const QTextBlockFormat paragraph = modify ? doc->styleBlockFormat(paragraphStyle) : cursor->block().blockFormat();
    const FontScheme &fonts = doc->flow().env.fonts;

    StyleDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("styleDialog"));
    dlg.setWindowTitle(modify ? QCoreApplication::translate("jp::write::StyleDialog", "Modify Style") : QCoreApplication::translate("jp::write::StyleDialog", "Create New Style from Formatting"));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;
    v->addLayout(form);

    auto *name = new QLineEdit(&dlg);
    name->setObjectName(QStringLiteral("styleName"));
    name->setMaxLength(kMostStyleNameLength);
    name->setText((start.name.isEmpty() ? start.id : start.name).left(kMostStyleNameLength));
    name->setEnabled(!modify || start.customStyle);   // (a built-in style keeps its name)
    form->addRow(QCoreApplication::translate("jp::write::StyleDialog", "Name:"), name);

    auto *type = new QComboBox(&dlg);
    type->setObjectName(QStringLiteral("styleType"));
    type->addItem(QCoreApplication::translate("jp::write::StyleDialog", "Paragraph"), QStringLiteral("paragraph"));
    type->addItem(QCoreApplication::translate("jp::write::StyleDialog", "Character"), QStringLiteral("character"));
    type->setCurrentIndex(character ? 1 : 0);
    type->setEnabled(!modify);
    form->addRow(QCoreApplication::translate("jp::write::StyleDialog", "Style type:"), type);

    auto *basedOn = new QComboBox(&dlg);
    basedOn->setObjectName(QStringLiteral("styleBasedOn"));
    auto *next = new QComboBox(&dlg);
    next->setObjectName(QStringLiteral("styleNext"));
    // The styles of the kinds there are to choose among (at most as many as the pane lists).
    const QVector<ListedStyle> all = listStyles(*doc, StyleFilter::All);
    auto fillChoices = [&](bool characterKind) {
        basedOn->clear();
        next->clear();
        basedOn->addItem(QCoreApplication::translate("jp::write::StyleDialog", "(no style)"), QString());
        next->addItem(QCoreApplication::translate("jp::write::StyleDialog", "(same style)"), QString());
        for (const ListedStyle &s : all) {
            if (s.character != characterKind || s.id == modifyId) continue;
            basedOn->addItem(s.name, s.id);
            if (!characterKind) next->addItem(s.name, s.id);
        }
    };
    fillChoices(character);
    auto select = [](QComboBox *box, const QString &id) {
        const int i = box->findData(id);
        box->setCurrentIndex(i >= 0 ? i : 0);
    };
    select(basedOn, start.basedOn);
    select(next, start.next == modifyId ? QString() : start.next);
    QObject::connect(type, &QComboBox::currentIndexChanged, &dlg, [&] {
        const bool isCharacter = type->currentData().toString() == QLatin1String("character");
        fillChoices(isCharacter);
        next->setEnabled(!isCharacter);
    });
    next->setEnabled(!character);
    form->addRow(QCoreApplication::translate("jp::write::StyleDialog", "Based on:"), basedOn);
    form->addRow(QCoreApplication::translate("jp::write::StyleDialog", "Style for the following paragraph:"), next);

    auto *font = new FontCombo(&dlg);
    font->setObjectName(QStringLiteral("styleFont"));
    font->setSchemeFonts(fonts.heading, fonts.body);
    QString family;
    if (look.hasProperty(QTextFormat::FontFamilies) && !look.fontFamilies().toStringList().isEmpty()) family = look.fontFamilies().toStringList().first();
    else family = look.stringProperty(tp::ThemeFont) == QLatin1String("major") ? fonts.heading : fonts.body;
    font->setCurrentFamily(family);
    const QString familyStart = font->currentText();
    auto *size = new SizeCombo(&dlg);
    size->setObjectName(QStringLiteral("styleSize"));
    size->setSize(look.fontPointSize() > 0 ? look.fontPointSize() : 11);
    const QString sizeStart = size->currentText();
    auto *fontRow = new QHBoxLayout;
    fontRow->addWidget(font, 1);
    fontRow->addWidget(size);
    form->addRow(QCoreApplication::translate("jp::write::StyleDialog", "Font:"), fontRow);
    auto *bold = new QCheckBox(QCoreApplication::translate("jp::write::StyleDialog", "Bold"), &dlg), *italic = new QCheckBox(QCoreApplication::translate("jp::write::StyleDialog", "Italic"), &dlg), *underline = new QCheckBox(QCoreApplication::translate("jp::write::StyleDialog", "Underline"), &dlg);
    bold->setObjectName(QStringLiteral("styleBold"));
    italic->setObjectName(QStringLiteral("styleItalic"));
    underline->setObjectName(QStringLiteral("styleUnderline"));
    bold->setChecked(look.fontWeight() >= QFont::DemiBold);
    italic->setChecked(look.fontItalic());
    underline->setChecked(look.underlineStyle() != QTextCharFormat::NoUnderline);
    const bool boldStart = bold->isChecked(), italicStart = italic->isChecked(), underlineStart = underline->isChecked();
    auto *color = new QPushButton(&dlg);
    color->setObjectName(QStringLiteral("styleColor"));
    QColor colorNow = look.hasProperty(QTextFormat::ForegroundBrush) ? look.foreground().color() : QColor();
    const QColor colorStart = colorNow;
    showColor(color, colorNow);
    QObject::connect(color, &QPushButton::clicked, &dlg, [&] {
        const QColor picked = QColorDialog::getColor(colorNow.isValid() ? colorNow : QColor(Qt::black), &dlg, QCoreApplication::translate("jp::write::StyleDialog", "Font Color"));
        if (!picked.isValid()) return;
        colorNow = picked;
        showColor(color, colorNow);
    });
    auto *effects = new QHBoxLayout;
    effects->addWidget(bold);
    effects->addWidget(italic);
    effects->addWidget(underline);
    effects->addWidget(color);
    form->addRow(QString(), effects);

    auto *align = new QComboBox(&dlg);
    align->setObjectName(QStringLiteral("styleAlign"));
    align->addItem(QCoreApplication::translate("jp::write::StyleDialog", "Left"), QStringLiteral("left"));
    align->addItem(QCoreApplication::translate("jp::write::StyleDialog", "Centered"), QStringLiteral("center"));
    align->addItem(QCoreApplication::translate("jp::write::StyleDialog", "Right"), QStringLiteral("right"));
    align->addItem(QCoreApplication::translate("jp::write::StyleDialog", "Justified"), QStringLiteral("both"));
    const Qt::Alignment al = paragraph.alignment() & Qt::AlignHorizontal_Mask;
    align->setCurrentIndex(al == Qt::AlignHCenter ? 1 : (al == Qt::AlignRight || al == Qt::AlignTrailing) ? 2 : al == Qt::AlignJustify ? 3 : 0);
    const int alignStart = align->currentIndex();
    auto measure = [&](const char *objectName, double points) {
        auto *m = new MeasureSpin(&dlg);
        m->setObjectName(QString::fromLatin1(objectName));
        m->setPoints(points);
        return m;
    };
    auto *left = measure("styleLeft", paragraph.leftMargin()), *right = measure("styleRight", paragraph.rightMargin()), *first = measure("styleFirst", paragraph.textIndent()),
         *before = measure("styleBefore", paragraph.topMargin()), *after = measure("styleAfter", paragraph.bottomMargin());
    auto *line = new QDoubleSpinBox(&dlg);
    line->setObjectName(QStringLiteral("styleLine"));
    line->setRange(0.1, 20);
    line->setDecimals(2);
    line->setSingleStep(0.05);
    line->setValue(paragraph.lineHeightType() == QTextBlockFormat::ProportionalHeight && paragraph.lineHeight() > 0 ? paragraph.lineHeight() / 100.0 : 1.0);
    const double lineStart = line->value();
    if (!character) {
        form->addRow(QCoreApplication::translate("jp::write::StyleDialog", "Alignment:"), align);
        form->addRow(QCoreApplication::translate("jp::write::StyleDialog", "Indentation, left:"), left);
        form->addRow(QCoreApplication::translate("jp::write::StyleDialog", "Indentation, right:"), right);
        form->addRow(QCoreApplication::translate("jp::write::StyleDialog", "First line:"), first);
        form->addRow(QCoreApplication::translate("jp::write::StyleDialog", "Spacing, before:"), before);
        form->addRow(QCoreApplication::translate("jp::write::StyleDialog", "Spacing, after:"), after);
        form->addRow(QCoreApplication::translate("jp::write::StyleDialog", "Line spacing (lines):"), line);
    }
    const double leftStart = left->value(), rightStart = right->value(), firstStart = first->value(), beforeStart = before->value(), afterStart = after->value();

    auto *gallery = new QCheckBox(QCoreApplication::translate("jp::write::StyleDialog", "Add to the style gallery"), &dlg);
    gallery->setObjectName(QStringLiteral("styleGallery"));
    gallery->setChecked(start.qFormat);
    auto *automatic = new QCheckBox(QCoreApplication::translate("jp::write::StyleDialog", "Update automatically"), &dlg);
    automatic->setObjectName(QStringLiteral("styleAutomatic"));
    automatic->setChecked(start.autoRedefine);
    automatic->setEnabled(!character);
    v->addWidget(gallery);
    v->addWidget(automatic);
    auto *problem = new QLabel(&dlg);
    problem->setObjectName(QStringLiteral("styleProblem"));
    problem->setWordWrap(true);
    problem->setStyleSheet(QStringLiteral("color: #b00020"));
    v->addWidget(problem);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);

    bool done = false;
    dlg.tryAccept = [&]() -> bool {
        Style def = start;
        def.name = name->text();
        def.basedOn = basedOn->currentData().toString();
        def.next = next->currentData().toString();
        def.qFormat = gallery->isChecked();
        def.autoRedefine = automatic->isChecked() && !character;
        if (!modify) def.type = type->currentData().toString();
        // Only what was changed goes into the style.
        if (font->currentText() != familyStart) {
            const int i = font->findText(font->currentText());
            const QString chosen = i >= 0 ? font->itemData(i).toString() : font->currentText().trimmed();
            Fonts f = def.rPr.fonts.value_or(Fonts());
            f.ascii.clear();
            f.hAnsi.clear();
            f.asciiTheme.clear();
            f.hAnsiTheme.clear();
            if (chosen == QLatin1String("+Heading")) f.asciiTheme = f.hAnsiTheme = QStringLiteral("majorHAnsi");
            else if (chosen == QLatin1String("+Body")) f.asciiTheme = f.hAnsiTheme = QStringLiteral("minorHAnsi");
            else f.ascii = f.hAnsi = chosen;
            if (!chosen.isEmpty()) def.rPr.fonts = f;
        }
        if (size->currentText() != sizeStart) {
            bool ok = false;
            const double points = size->currentText().trimmed().toDouble(&ok);
            if (ok && points > 0 && points <= 1638) def.rPr.size = int(std::lround(points * 2));
        }
        if (bold->isChecked() != boldStart) def.rPr.bold = bold->isChecked();
        if (italic->isChecked() != italicStart) def.rPr.italic = italic->isChecked();
        if (underline->isChecked() != underlineStart) {
            Underline u;
            u.val = underline->isChecked() ? QStringLiteral("single") : QStringLiteral("none");
            def.rPr.underline = u;
        }
        if (colorNow != colorStart && colorNow.isValid()) {
            Color c;
            c.val = colorNow.name(QColor::HexRgb).mid(1).toUpper();
            def.rPr.color = c;
        }
        if (!character) {
            if (align->currentIndex() != alignStart) def.pPr.jc = align->currentData().toString();
            if (std::abs(left->value() - leftStart) > 0.001) def.pPr.indLeft = twips(left->value());
            if (std::abs(right->value() - rightStart) > 0.001) def.pPr.indRight = twips(right->value());
            if (std::abs(first->value() - firstStart) > 0.001) def.pPr.indFirst = twips(first->value());
            if (std::abs(before->value() - beforeStart) > 0.001) {
                def.pPr.before = twips(before->value());
                def.pPr.beforeAuto = false;
            }
            if (std::abs(after->value() - afterStart) > 0.001) {
                def.pPr.after = twips(after->value());
                def.pPr.afterAuto = false;
            }
            if (std::abs(line->value() - lineStart) > 0.001) {
                def.pPr.line = int(std::lround(line->value() * 240));
                def.pPr.lineRule = QStringLiteral("auto");
            }
        }
        QString why;
        if (modify) done = modifyStyle(*doc, *cursor, modifyId, def, &why);
        else done = !createStyle(*doc, *cursor, def, &why, true).isEmpty();
        if (!done) problem->setText(why);
        return done;
    };
    dlg.exec();
    return done;
}

} // namespace jp::write
