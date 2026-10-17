#include "app/writedialogs.h"

#include "app/pageview.h"
#include "app/widgets.h"
#include "core/editops.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextFragment>
#include <QVBoxLayout>
#include <initializer_list>
#include <iterator>

namespace jp::write {

namespace {

// The look of the first characters of the selection (of the text typed next, with none selected).
QTextCharFormat firstFormat(const QTextCursor &c)
{
    if (c.hasSelection()) {
        const int from = c.selectionStart(), to = c.selectionEnd();
        for (QTextBlock b = c.document()->findBlock(from); b.isValid() && b.position() < to; b = b.next())
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const QTextFragment f = it.fragment();
                if (f.position() + f.length() <= from || f.position() >= to || isAtom(f.charFormat())) continue;
                return f.charFormat();
            }
    }
    QTextCharFormat cf = c.charFormat();
    if (isAtom(cf) || cf.fontPointSize() <= 0) cf = markFormat(c.block());
    return cf;
}

// Each choice of underline: Qt's line style and the tp::UnderlineKind flags beside it (as the file's reading and writing map them).
struct UnderlineChoice {
    const char *name;
    QTextCharFormat::UnderlineStyle style;
    int kind;
};
const UnderlineChoice kUnderlines[] = {
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "(none)"), QTextCharFormat::NoUnderline, 0},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Single"), QTextCharFormat::SingleUnderline, 0},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Words only"), QTextCharFormat::SingleUnderline, 2},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Double"), QTextCharFormat::SingleUnderline, 1},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Thick"), QTextCharFormat::SingleUnderline, 4},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Dotted"), QTextCharFormat::DotLine, 0},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Dashed"), QTextCharFormat::DashUnderline, 0},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Dot dash"), QTextCharFormat::DashDotLine, 0},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Dot dot dash"), QTextCharFormat::DashDotDotLine, 0},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Wave"), QTextCharFormat::WaveUnderline, 0},
    {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Double wave"), QTextCharFormat::WaveUnderline, 1},
};

// What the dialog's box for an effect was when it opened, and what it says now: -1 for unchanged (or left partly checked), else 0 for off and 1 for on.
struct Effect {
    QCheckBox *box = nullptr;
    Qt::CheckState was = Qt::Unchecked;
    int change() const { return box->checkState() == was || box->checkState() == Qt::PartiallyChecked ? -1 : box->checkState() == Qt::Checked ? 1 : 0; }
};

} // namespace

void fontDialog(QWidget *parent, WriteDoc *doc, QTextCursor *cursor)
{
    if (!doc || !cursor) return;
    QTextCursor &c = *cursor;
    const QTextCharFormat first = firstFormat(c);
    const FontScheme &fonts = doc->flow().env.fonts;
    const ColorScheme &colors = doc->flow().env.colors;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("fontDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::Dialogs", "Font"));
    auto *v = new QVBoxLayout(&dlg);
    auto *grid = new QGridLayout;
    v->addLayout(grid);

    // Font, style, and size.
    auto *font = new FontCombo(&dlg);
    font->setObjectName(QStringLiteral("fontName"));
    font->setSchemeFonts(fonts.heading, fonts.body);
    const QString family = first.hasProperty(QTextFormat::FontFamilies) && !first.fontFamilies().toStringList().isEmpty() ? first.fontFamilies().toStringList().first()
                           : first.stringProperty(tp::ThemeFont) == QLatin1String("major") ? fonts.heading
                                                                                           : fonts.body;
    font->setCurrentFamily(family);
    const QString familyShown = font->currentText();
    QString chosenFamily;
    auto *style = new QComboBox(&dlg);
    style->setObjectName(QStringLiteral("fontStyle"));
    style->addItems({QCoreApplication::translate("jp::write::Dialogs", "Regular"), QCoreApplication::translate("jp::write::Dialogs", "Italic"), QCoreApplication::translate("jp::write::Dialogs", "Bold"),
                     QCoreApplication::translate("jp::write::Dialogs", "Bold Italic")});
    const int style0 = (first.fontWeight() >= QFont::DemiBold ? 2 : 0) + (first.fontItalic() ? 1 : 0);
    style->setCurrentIndex(style0);
    auto *size = new DecimalSpin(&dlg);
    size->setObjectName(QStringLiteral("fontSize"));
    size->setRange(1, 1638);
    const double size0 = first.fontPointSize() > 0 ? first.fontPointSize() : 11;
    size->setValue(size0);

    // Color, and the underline's style and color.
    auto *color = new ColorButton(QStringLiteral("square"), QString(), true, QCoreApplication::translate("jp::write::Dialogs", "Automatic"), &dlg);
    color->setObjectName(QStringLiteral("fontColor"));
    color->setScheme(colors);
    color->setPopupMode(QToolButton::InstantPopup);
    color->setWell(true);
    const ColorRef color0 = first.hasProperty(QTextFormat::ForegroundBrush) && first.foreground().style() != Qt::NoBrush ? ColorRef::rgb(first.foreground().color()) : ColorRef::none();
    color->setCurrent(color0);
    auto *underline = new QComboBox(&dlg);
    underline->setObjectName(QStringLiteral("underlineStyle"));
    for (const UnderlineChoice &u : kUnderlines) underline->addItem(QCoreApplication::translate("jp::write::Dialogs", u.name));
    // The text's own choice; else the first with its line style.
    int underline0 = -1;
    for (int i = 0; i < int(std::size(kUnderlines)) && underline0 < 0; ++i)
        if (kUnderlines[i].style == first.underlineStyle() && kUnderlines[i].kind == first.intProperty(tp::UnderlineKind)) underline0 = i;
    for (int i = 0; i < int(std::size(kUnderlines)) && underline0 < 0; ++i)
        if (kUnderlines[i].style == first.underlineStyle()) underline0 = i;
    underline0 = std::max(0, underline0);
    underline->setCurrentIndex(underline0);
    auto *underlineColor = new ColorButton(QStringLiteral("square"), QString(), true, QCoreApplication::translate("jp::write::Dialogs", "Automatic"), &dlg);
    underlineColor->setObjectName(QStringLiteral("underlineColor"));
    underlineColor->setScheme(colors);
    underlineColor->setPopupMode(QToolButton::InstantPopup);
    underlineColor->setWell(true);
    const ColorRef underlineColor0 = underline0 > 0 && first.hasProperty(QTextFormat::TextUnderlineColor) ? ColorRef::rgb(first.underlineColor()) : ColorRef::none();
    underlineColor->setCurrent(underlineColor0);
    underlineColor->setEnabled(underline0 > 0);
    QObject::connect(underline, &QComboBox::currentIndexChanged, &dlg, [underlineColor](int i) { underlineColor->setEnabled(i > 0); });

    auto place = [&](const QString &text, QWidget *box, int row, int column) {
        auto *label = new QLabel(text, &dlg);
        label->setBuddy(box);
        grid->addWidget(label, row, column);
        grid->addWidget(box, row + 1, column);
    };
    place(QCoreApplication::translate("jp::write::Dialogs", "Font:"), font, 0, 0);
    place(QCoreApplication::translate("jp::write::Dialogs", "Font style:"), style, 0, 1);
    place(QCoreApplication::translate("jp::write::Dialogs", "Size:"), size, 0, 2);
    place(QCoreApplication::translate("jp::write::Dialogs", "Font color:"), color, 2, 0);
    place(QCoreApplication::translate("jp::write::Dialogs", "Underline style:"), underline, 2, 1);
    place(QCoreApplication::translate("jp::write::Dialogs", "Underline color:"), underlineColor, 2, 2);

    // The effects: checked when all of the selection has one, clear when none does, and partly when some does.
    auto *effects = new QGroupBox(QCoreApplication::translate("jp::write::Dialogs", "Effects"), &dlg);
    auto *eg = new QGridLayout(effects);
    int slot = 0;
    auto effect = [&](const char *name, const QString &text, const std::function<bool(const QTextCharFormat &)> &has) {
        Effect e;
        e.box = new QCheckBox(text, effects);
        e.box->setObjectName(QString::fromLatin1(name));
        e.was = allSelected(c, has) ? Qt::Checked : allSelected(c, [&has](const QTextCharFormat &f) { return !has(f); }) ? Qt::Unchecked : Qt::PartiallyChecked;
        e.box->setTristate(e.was == Qt::PartiallyChecked);
        e.box->setCheckState(e.was);
        eg->addWidget(e.box, slot % 4, slot / 4);
        ++slot;
        return e;
    };
    const auto isDouble = [](const QTextCharFormat &f) { return f.fontStrikeOut() && f.boolProperty(prop::DoubleStrike); };
    const Effect strike = effect("strikethrough", QCoreApplication::translate("jp::write::Dialogs", "Strikethrough"), [&](const QTextCharFormat &f) { return f.fontStrikeOut() && !isDouble(f); });
    const Effect doubleStrike = effect("doubleStrikethrough", QCoreApplication::translate("jp::write::Dialogs", "Double strikethrough"), isDouble);
    const Effect superscript = effect("superscript", QCoreApplication::translate("jp::write::Dialogs", "Superscript"), [](const QTextCharFormat &f) { return f.verticalAlignment() == QTextCharFormat::AlignSuperScript; });
    const Effect subscript = effect("subscript", QCoreApplication::translate("jp::write::Dialogs", "Subscript"), [](const QTextCharFormat &f) { return f.verticalAlignment() == QTextCharFormat::AlignSubScript; });
    const Effect smallCaps = effect("smallCaps", QCoreApplication::translate("jp::write::Dialogs", "Small caps"), [](const QTextCharFormat &f) { return f.fontCapitalization() == QFont::SmallCaps; });
    const Effect allCaps = effect("allCaps", QCoreApplication::translate("jp::write::Dialogs", "All caps"), [](const QTextCharFormat &f) { return f.fontCapitalization() == QFont::AllUppercase; });
    const Effect hidden = effect("hidden", QCoreApplication::translate("jp::write::Dialogs", "Hidden"), [](const QTextCharFormat &f) { return f.boolProperty(tp::HiddenText); });
    // Two of a kind are not both on: choosing one clears the other.
    auto either = [](const Effect &a, const Effect &b) {
        QObject::connect(a.box, &QCheckBox::checkStateChanged, b.box, [a, b](Qt::CheckState state) {
            if (state == Qt::Checked) b.box->setCheckState(Qt::Unchecked);
        });
        QObject::connect(b.box, &QCheckBox::checkStateChanged, a.box, [a, b](Qt::CheckState state) {
            if (state == Qt::Checked) a.box->setCheckState(Qt::Unchecked);
        });
    };
    either(strike, doubleStrike);
    either(superscript, subscript);
    either(smallCaps, allCaps);
    v->addWidget(effects);

    // The preview shows the look the boxes describe.
    auto *preview = new QLabel(QCoreApplication::translate("jp::write::Dialogs", "AaBbYyZz"), &dlg);
    preview->setObjectName(QStringLiteral("preview"));
    preview->setAccessibleName(QCoreApplication::translate("jp::write::Dialogs", "Preview"));
    preview->setMinimumHeight(56);
    preview->setAlignment(Qt::AlignCenter);
    preview->setFrameShape(QFrame::StyledPanel);
    auto updatePreview = [&] {
        QString f = chosenFamily.isEmpty() ? family : chosenFamily;
        if (f == QLatin1String("+Heading")) f = fonts.heading;
        else if (f == QLatin1String("+Body")) f = fonts.body;
        QFont pf(f);
        pf.setPointSizeF(std::min(size->value(), 28.0));
        pf.setBold(style->currentIndex() >= 2);
        pf.setItalic(style->currentIndex() % 2);
        pf.setUnderline(underline->currentIndex() > 0);
        pf.setStrikeOut(strike.box->isChecked() || doubleStrike.box->isChecked());
        pf.setCapitalization(smallCaps.box->isChecked() ? QFont::SmallCaps : allCaps.box->isChecked() ? QFont::AllUppercase : QFont::MixedCase);
        preview->setFont(pf);
        QPalette pal = dlg.palette();
        if (!color->current().isNone()) pal.setColor(QPalette::WindowText, color->current().resolve(colors));
        preview->setPalette(pal);
    };
    QObject::connect(font, &FontCombo::familyChosen, &dlg, [&](const QString &f) {
        chosenFamily = f;
        updatePreview();
    });
    QObject::connect(style, &QComboBox::currentIndexChanged, &dlg, updatePreview);
    QObject::connect(size, &QDoubleSpinBox::valueChanged, &dlg, updatePreview);
    QObject::connect(underline, &QComboBox::currentIndexChanged, &dlg, updatePreview);
    QObject::connect(color, &ColorButton::colorPicked, &dlg, updatePreview);
    for (const Effect &e : {strike, doubleStrike, superscript, subscript, smallCaps, allCaps, hidden}) QObject::connect(e.box, &QCheckBox::checkStateChanged, &dlg, updatePreview);
    updatePreview();
    v->addWidget(preview);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return;

    // Only what the person changed goes to the characters: what they left alone stays as each character had it.
    QString chosen = chosenFamily;
    if (chosen.isEmpty()) {
        const QString typed = font->currentText().trimmed();
        if (typed != familyShown) chosen = typed;
    }
    if (chosen.contains(QLatin1String(" (Headings)"))) chosen = QStringLiteral("+Heading");
    else if (chosen.contains(QLatin1String(" (Body)"))) chosen = QStringLiteral("+Body");
    size->interpretText();
    const bool familyChanged = !chosen.isEmpty() && (chosen.startsWith(QLatin1Char('+')) || chosen != family);
    const bool styleChanged = style->currentIndex() != style0;
    const bool sizeChanged = std::abs(size->value() - size0) > 0.0001;
    const bool colorChanged = color->current() != color0;
    const bool underlineChanged = underline->currentIndex() != underline0;
    const bool underlineColorChanged = underline->currentIndex() > 0 && underlineColor->current() != underlineColor0;
    const int strikeChange = strike.change(), doubleChange = doubleStrike.change(), upChange = superscript.change(), downChange = subscript.change();
    const int smallChange = smallCaps.change(), allChange = allCaps.change(), hiddenChange = hidden.change();
    c.beginEditBlock();   // (one step for all of it)
    if (familyChanged) setFontFamily(*doc, c, chosen);
    if (sizeChanged) setFontSize(*doc, c, size->value());
    if (colorChanged) setTextColor(*doc, c, color->current().isNone() ? QColor() : color->current().resolve(colors));
    changeCharFormat(*doc, c, [&](QTextCharFormat &f) {
        if (styleChanged) {
            f.setFontWeight(style->currentIndex() >= 2 ? QFont::Bold : QFont::Normal);
            f.setFontItalic(style->currentIndex() % 2);
        }
        if (underlineChanged) {
            const UnderlineChoice &u = kUnderlines[underline->currentIndex()];
            f.setUnderlineStyle(u.style);
            f.clearProperty(QTextFormat::FontUnderline);
            if (u.kind) f.setProperty(tp::UnderlineKind, u.kind);
            else f.clearProperty(tp::UnderlineKind);
            f.clearProperty(prop::UnderlineAttrs);
            if (u.style == QTextCharFormat::NoUnderline) f.clearProperty(QTextFormat::TextUnderlineColor);
        }
        if (underlineColorChanged) {
            f.clearProperty(prop::UnderlineAttrs);
            if (underlineColor->current().isNone()) f.clearProperty(QTextFormat::TextUnderlineColor);
            else f.setUnderlineColor(underlineColor->current().resolve(colors));
        }
        if (strikeChange >= 0 || doubleChange >= 0) {
            bool single = f.fontStrikeOut() && !isDouble(f), twice = isDouble(f);
            if (strikeChange >= 0) {
                single = strikeChange == 1;
                if (single) twice = false;
            }
            if (doubleChange >= 0) {
                twice = doubleChange == 1;
                if (twice) single = false;
            }
            f.setFontStrikeOut(single || twice);
            if (twice) f.setProperty(prop::DoubleStrike, true);
            else f.clearProperty(prop::DoubleStrike);
        }
        if (upChange >= 0 || downChange >= 0) {
            bool up = f.verticalAlignment() == QTextCharFormat::AlignSuperScript, down = f.verticalAlignment() == QTextCharFormat::AlignSubScript;
            if (upChange >= 0) {
                up = upChange == 1;
                if (up) down = false;
            }
            if (downChange >= 0) {
                down = downChange == 1;
                if (down) up = false;
            }
            f.setVerticalAlignment(up ? QTextCharFormat::AlignSuperScript : down ? QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal);
        }
        if (smallChange >= 0 || allChange >= 0) {
            bool small = f.fontCapitalization() == QFont::SmallCaps, all = f.fontCapitalization() == QFont::AllUppercase;
            if (smallChange >= 0) {
                small = smallChange == 1;
                if (small) all = false;
            }
            if (allChange >= 0) {
                all = allChange == 1;
                if (all) small = false;
            }
            if (all) f.setFontCapitalization(QFont::AllUppercase);
            else if (small) f.setFontCapitalization(QFont::SmallCaps);
            else f.clearProperty(QTextFormat::FontCapitalization);
        }
        if (hiddenChange == 1) f.setProperty(tp::HiddenText, true);
        else if (hiddenChange == 0) f.clearProperty(tp::HiddenText);
    });
    c.endEditBlock();
}

void paragraphDialog(QWidget *parent, WriteDoc *doc, QTextCursor *cursor)
{
    if (!doc || !cursor) return;
    const QTextBlockFormat start = cursor->block().blockFormat();
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("paragraphDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::Dialogs", "Paragraph"));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;
    v->addLayout(form);

    auto *align = new QComboBox(&dlg);
    align->setObjectName(QStringLiteral("alignment"));
    align->addItem(QCoreApplication::translate("jp::write::Dialogs", "Left"), int(Qt::AlignLeft));
    align->addItem(QCoreApplication::translate("jp::write::Dialogs", "Centered"), int(Qt::AlignHCenter));
    align->addItem(QCoreApplication::translate("jp::write::Dialogs", "Right"), int(Qt::AlignRight));
    align->addItem(QCoreApplication::translate("jp::write::Dialogs", "Justified"), int(Qt::AlignJustify));
    const Qt::Alignment al = start.alignment() & Qt::AlignHorizontal_Mask;
    align->setCurrentIndex(al == Qt::AlignHCenter ? 1 : (al == Qt::AlignRight || al == Qt::AlignTrailing) ? 2 : al == Qt::AlignJustify ? 3 : 0);
    form->addRow(QCoreApplication::translate("jp::write::Dialogs", "Alignment:"), align);

    auto *left = new MeasureSpin(&dlg), *right = new MeasureSpin(&dlg), *by = new MeasureSpin(&dlg);
    left->setObjectName(QStringLiteral("left"));
    right->setObjectName(QStringLiteral("right"));
    by->setObjectName(QStringLiteral("by"));
    left->setPoints(start.leftMargin());
    right->setPoints(start.rightMargin());
    auto *special = new QComboBox(&dlg);
    special->setObjectName(QStringLiteral("special"));
    special->addItems({QCoreApplication::translate("jp::write::Dialogs", "(none)"), QCoreApplication::translate("jp::write::Dialogs", "First line"), QCoreApplication::translate("jp::write::Dialogs", "Hanging")});
    special->setCurrentIndex(start.textIndent() > 0.01 ? 1 : start.textIndent() < -0.01 ? 2 : 0);
    by->setPoints(std::abs(start.textIndent()));
    form->addRow(QCoreApplication::translate("jp::write::Dialogs", "Indentation, left:"), left);
    form->addRow(QCoreApplication::translate("jp::write::Dialogs", "Indentation, right:"), right);
    auto *row = new QHBoxLayout;
    row->addWidget(special);
    row->addWidget(by);
    form->addRow(QCoreApplication::translate("jp::write::Dialogs", "Special:"), row);

    auto *before = new MeasureSpin(&dlg), *after = new MeasureSpin(&dlg), *at = new MeasureSpin(&dlg);
    before->setObjectName(QStringLiteral("before"));
    after->setObjectName(QStringLiteral("after"));
    at->setObjectName(QStringLiteral("at"));
    before->setPoints(start.topMargin());
    after->setPoints(start.bottomMargin());
    form->addRow(QCoreApplication::translate("jp::write::Dialogs", "Spacing, before:"), before);
    form->addRow(QCoreApplication::translate("jp::write::Dialogs", "Spacing, after:"), after);
    auto *lineType = new QComboBox(&dlg);
    lineType->setObjectName(QStringLiteral("lineSpacing"));
    lineType->addItems({QCoreApplication::translate("jp::write::Dialogs", "Single"), QCoreApplication::translate("jp::write::Dialogs", "1.5 lines"), QCoreApplication::translate("jp::write::Dialogs", "Double"), QCoreApplication::translate("jp::write::Dialogs", "At least"), QCoreApplication::translate("jp::write::Dialogs", "Exactly"), QCoreApplication::translate("jp::write::Dialogs", "Multiple")});
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
    form->addRow(QCoreApplication::translate("jp::write::Dialogs", "Line spacing:"), lineRow);
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
        if (alignChanged) {
            bf.setAlignment(chosen);
            bf.clearProperty(prop::JcRaw);
        }
        if (leftChanged) bf.setLeftMargin(left->value());
        if (rightChanged) bf.setRightMargin(right->value());
        if (firstChanged) bf.setTextIndent(firstNow);
        if (beforeChanged) write::setSpaceBefore(bf, before->value());
        if (afterChanged) write::setSpaceAfter(bf, after->value());
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
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::Dialogs", "Zoom"));
    auto *v = new QVBoxLayout(&dlg);
    auto *box = new QGroupBox(QCoreApplication::translate("jp::write::Dialogs", "Zoom to"), &dlg);
    auto *bv = new QVBoxLayout(box);
    auto *group = new QButtonGroup(&dlg);
    struct Choice { const char *name; int id; };
    QVector<QRadioButton *> radios;
    const Choice choices[] = {{"200%", 200}, {"100%", 100}, {"75%", 75}, {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Page width"), -1}, {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Whole page"), -2},
                              {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Many pages"), -3}, {QT_TRANSLATE_NOOP("jp::write::Dialogs", "Percent:"), 0}};
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
    acrossRow->addWidget(new QLabel(QCoreApplication::translate("jp::write::Dialogs", "Pages across:"), box));
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
