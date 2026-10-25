// Dialog boxes for text, lists, symbols, links, fills, and tables that every
// app with pages shares (they work on an Editor).

#include "app/textdialogs.h"

#include "app/appwords.h"
#include "app/editor.h"
#include "app/settings.h"
#include "app/widgets.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include "core/fontfiles.h"
#include <QBuffer>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextBlock>
#include <QTextDocument>
#include <QVBoxLayout>
#include "app/plaintext.h"

namespace jp {

namespace {

struct Dlg {
    QDialog d;
    QVBoxLayout *v;
    QDialogButtonBox *bb;
    explicit Dlg(QWidget *p, const QString &title, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok | QDialogButtonBox::Cancel) : d(p)
    {
        d.setWindowTitle(title);
        v = new QVBoxLayout(&d);
        bb = new QDialogButtonBox(buttons, &d);
        QObject::connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
        QObject::connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    }
    bool exec()
    {
        v->addWidget(bb);
        return d.exec() == QDialog::Accepted;
    }
};

MeasureSpin *measure(double pt, QWidget *p, double min = 0, double max = 72 * 240)
{
    auto *m = new MeasureSpin(p);
    m->setRange(min, max);
    m->setValue(pt);
    m->setFixedWidth(110);
    return m;
}

QDoubleSpinBox *points(double v, QWidget *p, double min = 0, double max = 1000, const QString &suffix = QStringLiteral(" pt"))
{
    auto *s = new DecimalSpin(p);
    s->setRange(min, max);
    s->setValue(v);
    s->setSuffix(suffix);
    return s;
}

ColorButton *colorPick(Editor *ed, const ColorRef &c, bool allowNone, QWidget *p)
{
    auto *b = new ColorButton("square", QString(), allowNone, QCoreApplication::translate("Dialogs", "No Color"), p);
    b->setScheme(ed->doc()->colors);
    b->setCurrent(c);
    b->setPopupMode(QToolButton::InstantPopup);
    b->setWell(true);
    QObject::connect(b, &ColorButton::colorPicked, b, [b](const ColorRef &x) { b->setCurrent(x); });
    return b;
}

} // namespace

// ---------------- Font ----------------
void fontDialog(QWidget *p, Editor *ed)
{
    const QTextCharFormat cur = ed->currentCharFormat();
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Font"));
    auto *grid = new QGridLayout();
    auto *font = new FontCombo(&dlg.d);
    font->setSchemeFonts(ed->doc()->fonts.heading, ed->doc()->fonts.body);
    const QString fam = cur.hasProperty(QTextFormat::FontFamilies) ? cur.fontFamilies().toStringList().value(0) : (cur.stringProperty(tp::ThemeFont) == "major" ? QStringLiteral("+Heading") : QStringLiteral("+Body"));
    font->setCurrentFamily(fam);
    QString chosenFamily;
    QObject::connect(font, &FontCombo::familyChosen, &dlg.d, [&](const QString &f) { chosenFamily = f; });
    auto *style = new QComboBox(&dlg.d);
    style->addItems({QCoreApplication::translate("Dialogs", "Regular"), QCoreApplication::translate("Dialogs", "Italic"), QCoreApplication::translate("Dialogs", "Bold"), QCoreApplication::translate("Dialogs", "Bold Italic")});
    style->setCurrentIndex((cur.fontWeight() >= QFont::DemiBold ? 2 : 0) + (cur.fontItalic() ? 1 : 0));
    auto *size = new DecimalSpin(&dlg.d);
    size->setRange(0.5, 999);
    size->setValue(cur.hasProperty(QTextFormat::FontPointSize) ? cur.fontPointSize() : 11);
    auto *color = colorPick(ed, cur.stringProperty(tp::ColorRefP).isEmpty() ? ColorRef::scheme(Main) : ColorRef::fromString(cur.stringProperty(tp::ColorRefP)), false, &dlg.d);
    auto *underline = new QComboBox(&dlg.d);
    // Each choice: Qt's line style and the tp::UnderlineKind flags beside it.
    struct UnderlineChoice { const char *name; QTextCharFormat::UnderlineStyle style; int kind; };
    static const UnderlineChoice underlines[] = {
        {QT_TRANSLATE_NOOP("Dialogs", "(none)"), QTextCharFormat::NoUnderline, 0},
        {QT_TRANSLATE_NOOP("Dialogs", "Single"), QTextCharFormat::SingleUnderline, 0},
        {QT_TRANSLATE_NOOP("Dialogs", "Words only"), QTextCharFormat::SingleUnderline, 2},
        {QT_TRANSLATE_NOOP("Dialogs", "Double"), QTextCharFormat::SingleUnderline, 1},
        {QT_TRANSLATE_NOOP("Dialogs", "Thick"), QTextCharFormat::SingleUnderline, 4},
        {QT_TRANSLATE_NOOP("Dialogs", "Dotted"), QTextCharFormat::DotLine, 0},
        {QT_TRANSLATE_NOOP("Dialogs", "Dashed"), QTextCharFormat::DashUnderline, 0},
        {QT_TRANSLATE_NOOP("Dialogs", "Dot dash"), QTextCharFormat::DashDotLine, 0},
        {QT_TRANSLATE_NOOP("Dialogs", "Dot dot dash"), QTextCharFormat::DashDotDotLine, 0},
        {QT_TRANSLATE_NOOP("Dialogs", "Wave"), QTextCharFormat::WaveUnderline, 0},
        {QT_TRANSLATE_NOOP("Dialogs", "Double wave"), QTextCharFormat::WaveUnderline, 1},
    };
    for (const auto &u : underlines) underline->addItem(QCoreApplication::translate("Dialogs", u.name));
    // The text's own choice; else the first with its line style.
    int pick = -1;
    for (int i = int(std::size(underlines)) - 1; i >= 0; --i)
        if (underlines[i].style == cur.underlineStyle()) {
            if (underlines[i].kind == cur.intProperty(tp::UnderlineKind) || cur.underlineStyle() == QTextCharFormat::NoUnderline) { pick = i; break; }
            pick = i;
        }
    underline->setCurrentIndex(std::max(0, pick));
    grid->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "Font:")), 0, 0);
    grid->addWidget(font, 1, 0);
    grid->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "Font style:")), 0, 1);
    grid->addWidget(style, 1, 1);
    grid->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "Size:")), 0, 2);
    grid->addWidget(size, 1, 2);
    grid->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "Underline:")), 2, 0);
    grid->addWidget(underline, 3, 0);
    grid->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "Color:")), 2, 1);
    grid->addWidget(color, 3, 1);
    dlg.v->addLayout(grid);
    auto *effects = new QGroupBox(QCoreApplication::translate("Dialogs", "Effects"), &dlg.d);
    auto *eg = new QGridLayout(effects);
    struct Fx { const char *name; QCheckBox *box = nullptr; };
    QCheckBox *sup = new QCheckBox(QCoreApplication::translate("Dialogs", "Superscript")), *sub = new QCheckBox(QCoreApplication::translate("Dialogs", "Subscript")), *strike = new QCheckBox(QCoreApplication::translate("Dialogs", "Strikethrough"));
    QCheckBox *smallcaps = new QCheckBox(QCoreApplication::translate("Dialogs", "Small caps")), *allcaps = new QCheckBox(QCoreApplication::translate("Dialogs", "All caps")), *shadow = new QCheckBox(QCoreApplication::translate("Dialogs", "Shadow"));
    QCheckBox *outline = new QCheckBox(QCoreApplication::translate("Dialogs", "Outline")), *emboss = new QCheckBox(QCoreApplication::translate("Dialogs", "Emboss")), *engrave = new QCheckBox(QCoreApplication::translate("Dialogs", "Engrave"));
    sup->setChecked(cur.verticalAlignment() == QTextCharFormat::AlignSuperScript);
    sub->setChecked(cur.verticalAlignment() == QTextCharFormat::AlignSubScript);
    strike->setChecked(cur.fontStrikeOut());
    smallcaps->setChecked(cur.fontCapitalization() == QFont::SmallCaps);
    allcaps->setChecked(cur.fontCapitalization() == QFont::AllUppercase);
    // A slide's words have their shadow in their effect list (as the Shadow button gives it); a publication's in its own property.
    const bool slide = ed->doc()->slideText, shadowWas = slide ? Editor::hasTextShadow(cur) : cur.boolProperty(tp::Shadow);
    shadow->setChecked(shadowWas);
    outline->setChecked(!cur.stringProperty(tp::OutlineRef).isEmpty());
    emboss->setChecked(cur.boolProperty(tp::Emboss));
    engrave->setChecked(cur.boolProperty(tp::Engrave));
    const QList<QCheckBox *> boxes{sup, sub, strike, smallcaps, allcaps, shadow, outline, emboss, engrave};
    for (int i = 0; i < boxes.size(); ++i) eg->addWidget(boxes[i], i / 3, i % 3);
    dlg.v->addWidget(effects);
    auto *preview = plainLabel(new QLabel(forThisApp(QCoreApplication::translate("Dialogs", "JeffPub sample text")), &dlg.d));
    preview->setMinimumHeight(48);
    preview->setAlignment(Qt::AlignCenter);
    preview->setFrameShape(QFrame::StyledPanel);
    auto updatePreview = [&] {
        QFont f(chosenFamily.isEmpty() || chosenFamily.startsWith('+') ? (fam.startsWith('+') ? (fam == "+Heading" ? ed->doc()->fonts.heading : ed->doc()->fonts.body) : fam) : chosenFamily);
        f.setPointSizeF(std::min(size->value(), 28.0));
        f.setBold(style->currentIndex() >= 2);
        f.setItalic(style->currentIndex() % 2);
        f.setUnderline(underline->currentIndex() > 0);
        f.setStrikeOut(strike->isChecked());
        f.setCapitalization(smallcaps->isChecked() ? QFont::SmallCaps : allcaps->isChecked() ? QFont::AllUppercase : QFont::MixedCase);
        preview->setFont(f);
    };
    QObject::connect(font, &FontCombo::familyChosen, &dlg.d, updatePreview);
    QObject::connect(style, &QComboBox::currentIndexChanged, &dlg.d, updatePreview);
    QObject::connect(size, &QDoubleSpinBox::valueChanged, &dlg.d, updatePreview);
    for (QCheckBox *b : boxes) QObject::connect(b, &QCheckBox::toggled, &dlg.d, updatePreview);
    updatePreview();
    dlg.v->addWidget(preview);
    if (!dlg.exec()) return;
    QString f = chosenFamily.isEmpty() ? font->currentText() : chosenFamily;
    ed->beginChange(QCoreApplication::translate("Dialogs", "Font"));
    if (!f.isEmpty() && f != fam) ed->setFontFamily(f.contains(" (Headings)") ? "+Heading" : f.contains(" (Body)") ? "+Body" : f);
    QTextCharFormat cf;
    cf.setFontPointSize(size->value());
    cf.setFontWeight(style->currentIndex() >= 2 ? QFont::Bold : QFont::Normal);
    cf.setFontItalic(style->currentIndex() % 2);
    cf.setUnderlineStyle(underlines[underline->currentIndex()].style);
    cf.setProperty(tp::UnderlineKind, underlines[underline->currentIndex()].kind);
    cf.setFontStrikeOut(strike->isChecked());
    cf.setVerticalAlignment(sup->isChecked() ? QTextCharFormat::AlignSuperScript : sub->isChecked() ? QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal);
    cf.setFontCapitalization(smallcaps->isChecked() ? QFont::SmallCaps : allcaps->isChecked() ? QFont::AllUppercase : QFont::MixedCase);
    if (!slide) cf.setProperty(tp::Shadow, shadow->isChecked());
    cf.setProperty(tp::Emboss, emboss->isChecked());
    cf.setProperty(tp::Engrave, engrave->isChecked());
    if (outline->isChecked()) cf.setProperty(tp::OutlineRef, color->current().toString());
    cf.setProperty(tp::ColorRefP, color->current().toString());
    ed->mergeCharFormat(cf, QCoreApplication::translate("Dialogs", "Font"));
    if (slide && shadow->isChecked() != shadowWas) ed->setTextShadow(shadow->isChecked(), QCoreApplication::translate("Dialogs", "Font"));
    if (!outline->isChecked()) ed->clearCharProperty(tp::OutlineRef, QCoreApplication::translate("Dialogs", "Font"));
    ed->endChange();
}

// ---------------- Paragraph ----------------
void paragraphDialog(QWidget *p, Editor *ed, int tab)
{
    const QTextBlockFormat bf = ed->currentBlockFormat();
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Paragraph"));
    auto *tabs = new QTabWidget(&dlg.d);
    auto *t1 = new QWidget();
    auto *f1 = new QFormLayout(t1);
    auto *align = new QComboBox(t1);
    align->addItems({QCoreApplication::translate("Dialogs", "Left"), QCoreApplication::translate("Dialogs", "Center"), QCoreApplication::translate("Dialogs", "Right"), QCoreApplication::translate("Dialogs", "Justified"), QCoreApplication::translate("Dialogs", "Distributed")});
    const Qt::Alignment al = bf.alignment() & Qt::AlignHorizontal_Mask;
    align->setCurrentIndex(bf.boolProperty(tp::Distribute) ? 4 : al == Qt::AlignHCenter ? 1 : al == Qt::AlignRight ? 2 : al == Qt::AlignJustify ? 3 : 0);
    auto *left = measure(bf.leftMargin(), t1, 0), *first = measure(bf.textIndent(), t1, -2000), *right = measure(bf.rightMargin(), t1, 0);
    first->setMinimum(-2000);
    auto *before = points(bf.topMargin(), t1), *after = points(bf.bottomMargin(), t1);
    auto *lineType = new QComboBox(t1);
    lineType->addItems({QCoreApplication::translate("Dialogs", "Single"), QCoreApplication::translate("Dialogs", "Multiple (sp)"), QCoreApplication::translate("Dialogs", "Exactly (pt)"), QCoreApplication::translate("Dialogs", "At least (pt)")});
    auto *lineVal = new DecimalSpin(t1);
    lineVal->setRange(0.1, 1000);
    switch (bf.lineHeightType()) {
    case QTextBlockFormat::ProportionalHeight: lineType->setCurrentIndex(1); lineVal->setValue(bf.lineHeight() / 100); break;
    case QTextBlockFormat::FixedHeight: lineType->setCurrentIndex(2); lineVal->setValue(bf.lineHeight()); break;
    case QTextBlockFormat::MinimumHeight: lineType->setCurrentIndex(3); lineVal->setValue(bf.lineHeight()); break;
    default: lineType->setCurrentIndex(0); lineVal->setValue(1.0); break;
    }
    auto *baseline = new QCheckBox(QCoreApplication::translate("Dialogs", "Align text to baseline guides"), t1);
    baseline->setChecked(bf.boolProperty(tp::AlignToBaseline));
    auto *direction = new QComboBox(t1);
    direction->addItems({QCoreApplication::translate("Dialogs", "Left to right"), QCoreApplication::translate("Dialogs", "Right to left")});
    direction->setCurrentIndex(bf.layoutDirection() == Qt::RightToLeft ? 1 : 0);
    f1->addRow(QCoreApplication::translate("Dialogs", "Alignment:"), align);
    f1->addRow(QCoreApplication::translate("Dialogs", "Direction:"), direction);
    f1->addRow(QCoreApplication::translate("Dialogs", "Left indent:"), left);
    f1->addRow(QCoreApplication::translate("Dialogs", "First line indent:"), first);
    f1->addRow(QCoreApplication::translate("Dialogs", "Right indent:"), right);
    f1->addRow(QCoreApplication::translate("Dialogs", "Space before:"), before);
    f1->addRow(QCoreApplication::translate("Dialogs", "Space after:"), after);
    auto *lrow = new QHBoxLayout();
    lrow->addWidget(lineType);
    lrow->addWidget(lineVal);
    f1->addRow(QCoreApplication::translate("Dialogs", "Line spacing:"), lrow);
    f1->addRow(baseline);
    tabs->addTab(t1, QCoreApplication::translate("Dialogs", "Indents and Spacing"));
    auto *t2 = new QWidget();
    auto *f2 = new QVBoxLayout(t2);
    auto *widow = new QCheckBox(QCoreApplication::translate("Dialogs", "Widow/Orphan control"), t2);
    auto *withNext = new QCheckBox(QCoreApplication::translate("Dialogs", "Keep with next"), t2);
    auto *together = new QCheckBox(QCoreApplication::translate("Dialogs", "Keep lines together"), t2);
    auto *nextBox = new QCheckBox(QCoreApplication::translate("Dialogs", "Start in next text box"), t2);
    widow->setChecked(bf.boolProperty(tp::WidowControl));
    withNext->setChecked(bf.boolProperty(tp::KeepWithNext));
    together->setChecked(bf.boolProperty(tp::KeepTogether));
    nextBox->setChecked(bf.boolProperty(tp::StartInNextBox));
    f2->addWidget(widow);
    f2->addWidget(withNext);
    f2->addWidget(together);
    f2->addWidget(nextBox);
    f2->addStretch(1);
    tabs->addTab(t2, QCoreApplication::translate("Dialogs", "Line and Paragraph Breaks"));
    // Tabs.
    auto *t3 = new QWidget();
    auto *f3 = new QVBoxLayout(t3);
    auto *tabList = new QTableWidget(0, 3, t3);
    tabList->setHorizontalHeaderLabels({QCoreApplication::translate("Dialogs", "Position"), QCoreApplication::translate("Dialogs", "Alignment"), QCoreApplication::translate("Dialogs", "Leader")});
    tabList->horizontalHeader()->setStretchLastSection(true);
    QList<QTextOption::Tab> tabs0 = bf.tabPositions();
    const QString leaders = bf.stringProperty(tp::TabLeaders);
    auto addTabRow = [&](const QTextOption::Tab &t, QChar leader) {
        const int r = tabList->rowCount();
        tabList->insertRow(r);
        tabList->setItem(r, 0, new QTableWidgetItem(Settings::get().format(t.position)));
        auto *a = new QComboBox();
        a->addItems({QCoreApplication::translate("Dialogs", "Left"), QCoreApplication::translate("Dialogs", "Center"), QCoreApplication::translate("Dialogs", "Right"), QCoreApplication::translate("Dialogs", "Decimal")});
        a->setCurrentIndex(t.type == QTextOption::LeftTab ? 0 : t.type == QTextOption::CenterTab ? 1 : t.type == QTextOption::RightTab ? 2 : 3);
        tabList->setCellWidget(r, 1, a);
        auto *l = new QComboBox();
        l->addItems({QCoreApplication::translate("Dialogs", "None"), QCoreApplication::translate("Dialogs", "Dot"), QCoreApplication::translate("Dialogs", "Dash"), QCoreApplication::translate("Dialogs", "Line"), QCoreApplication::translate("Dialogs", "Bullet")});
        l->setCurrentIndex(leader == '.' ? 1 : leader == '-' ? 2 : leader == '_' ? 3 : leader == QChar(0x2022) ? 4 : 0);
        tabList->setCellWidget(r, 2, l);
    };
    for (int i = 0; i < tabs0.size(); ++i) addTabRow(tabs0[i], i < leaders.size() ? leaders.at(i) : QChar());
    auto *trow = new QHBoxLayout();
    auto *addTab = new QPushButton(QCoreApplication::translate("Dialogs", "Add"), t3);
    auto *delTab = new QPushButton(QCoreApplication::translate("Dialogs", "Delete"), t3);
    auto *clrTab = new QPushButton(QCoreApplication::translate("Dialogs", "Clear All"), t3);
    trow->addWidget(addTab);
    trow->addWidget(delTab);
    trow->addWidget(clrTab);
    QObject::connect(addTab, &QPushButton::clicked, t3, [&] { addTabRow(QTextOption::Tab(36 * (tabList->rowCount() + 1), QTextOption::LeftTab), QChar()); });
    QObject::connect(delTab, &QPushButton::clicked, t3, [&] { if (tabList->currentRow() >= 0) tabList->removeRow(tabList->currentRow()); });
    QObject::connect(clrTab, &QPushButton::clicked, t3, [&] { tabList->setRowCount(0); });
    f3->addWidget(tabList);
    f3->addLayout(trow);
    // The publication's default tab stops, for every paragraph without stops of its own (Publisher takes 1 to 1,584 points).
    auto *defaultTab = measure(ed->doc()->defaultTabStop, t3, 1, 1584);
    defaultTab->setObjectName(QStringLiteral("defaultTabStop"));
    defaultTab->setAccessibleName(QCoreApplication::translate("Dialogs", "Default tab stops"));
    auto *defaultRow = new QFormLayout();
    defaultRow->addRow(QCoreApplication::translate("Dialogs", "Default tab stops:"), defaultTab);
    f3->addLayout(defaultRow);
    tabs->addTab(t3, QCoreApplication::translate("Dialogs", "Tabs"));
    tabs->setCurrentIndex(std::clamp(tab, 0, 2));
    dlg.v->addWidget(tabs);
    if (!dlg.exec()) return;
    QTextBlockFormat f;
    const Qt::Alignment als[] = {Qt::AlignLeft, Qt::AlignHCenter, Qt::AlignRight, Qt::AlignJustify, Qt::AlignJustify};
    f.setAlignment(als[align->currentIndex()]);
    f.setProperty(tp::Distribute, align->currentIndex() == 4);
    f.setLeftMargin(left->value());
    f.setTextIndent(first->value());
    f.setRightMargin(right->value());
    f.setTopMargin(before->value());
    f.setBottomMargin(after->value());
    switch (lineType->currentIndex()) {
    case 0: f.setLineHeight(100, QTextBlockFormat::ProportionalHeight); break;
    case 1: f.setLineHeight(lineVal->value() * 100, QTextBlockFormat::ProportionalHeight); break;
    case 2: f.setLineHeight(lineVal->value(), QTextBlockFormat::FixedHeight); break;
    case 3: f.setLineHeight(lineVal->value(), QTextBlockFormat::MinimumHeight); break;
    }
    f.setProperty(tp::AlignToBaseline, baseline->isChecked());
    f.setLayoutDirection(direction->currentIndex() == 1 ? Qt::RightToLeft : Qt::LeftToRight);
    f.setProperty(tp::WidowControl, widow->isChecked());
    f.setProperty(tp::KeepWithNext, withNext->isChecked());
    f.setProperty(tp::KeepTogether, together->isChecked());
    f.setProperty(tp::StartInNextBox, nextBox->isChecked());
    // Tab stops sorted by position, each keeping its own leader.
    QVector<QPair<QTextOption::Tab, QChar>> rows;
    for (int r = 0; r < tabList->rowCount(); ++r) {
        double pt = 0;
        if (!Settings::get().parse(tabList->item(r, 0)->text(), &pt)) continue;
        const int a = static_cast<QComboBox *>(tabList->cellWidget(r, 1))->currentIndex();
        const int l = static_cast<QComboBox *>(tabList->cellWidget(r, 2))->currentIndex();
        QTextOption::Tab t(pt, a == 0 ? QTextOption::LeftTab : a == 1 ? QTextOption::CenterTab : a == 2 ? QTextOption::RightTab : QTextOption::DelimiterTab, a == 3 ? QChar('.') : QChar());
        rows << qMakePair(t, QString(" .-_•").at(l));
    }
    std::sort(rows.begin(), rows.end(), [](const auto &a, const auto &b) { return a.first.position < b.first.position; });
    QList<QTextOption::Tab> newTabs;
    QString newLeaders;
    for (const auto &r : rows) {
        newTabs << r.first;
        newLeaders += r.second;
    }
    f.setTabPositions(newTabs);
    f.setProperty(tp::TabLeaders, newLeaders);
    if (std::abs(defaultTab->value() - ed->doc()->defaultTabStop) < 1e-4) {
        ed->mergeBlockFormat(f, QCoreApplication::translate("Dialogs", "Paragraph"));
        return;
    }
    // The publication's default tab stops move with the paragraph's changes as one step.
    ed->change(QCoreApplication::translate("Dialogs", "Default Tab Stops"), [&] {
        ed->mergeBlockFormat(f, QCoreApplication::translate("Dialogs", "Paragraph"));
        ed->doc()->defaultTabStop = defaultTab->value();
        // Boxes that grow to fit their text, and table rows, follow words that now wrap differently.
        auto hasTab = [&](const QString &story) {
            const QTextDocument *sd = ed->doc()->storyDoc(story);
            return sd && sd->toPlainText().contains(QLatin1Char('\t'));
        };
        ed->doc()->forEachItem([&](Item *it, int, const QString &) {
            if (auto *t = dynamic_cast<TextItem *>(it)) {
                if (t->autofit == TextItem::GrowBox && hasTab(t->storyId)) ed->autoGrowText(t);
            } else if (auto *tb = dynamic_cast<TableItem *>(it)) {
                if (std::any_of(tb->cells.begin(), tb->cells.end(), [&](const TableCell &c) { return hasTab(c.storyId) || hasTab(c.storyId2); })) ed->fitTableRows(tb);
            }
        });
    });
}

// ---------------- Bullets and Numbering ----------------
void bulletsDialog(QWidget *p, Editor *ed, bool numbering)
{
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Bullets and Numbering"));
    auto *tabs = new QTabWidget(&dlg.d);
    auto *bw = new QWidget();
    auto *bl = new QGridLayout(bw);
    auto *group = new QButtonGroup(bw);
    const QStringList chars{"•", "◦", "▪", "❖", "➢", "✔", "★", "–", "►", "♥", "☺", "→"};
    for (int i = 0; i < chars.size(); ++i) {
        auto *b = new QPushButton(chars[i], bw);
        b->setCheckable(true);
        b->setFixedSize(52, 52);
        QFont f = b->font();
        f.setPointSize(20);
        b->setFont(f);
        group->addButton(b, i);
        bl->addWidget(b, i / 6, i % 6);
    }
    group->button(0)->setChecked(true);
    auto *custom = new QPushButton(QCoreApplication::translate("Dialogs", "Character…"), bw);
    QString customChar;
    QObject::connect(custom, &QPushButton::clicked, bw, [&] {
        bool ok = false;
        const QString c = QInputDialog::getText(bw, QCoreApplication::translate("Dialogs", "Bullet Character"), QCoreApplication::translate("Dialogs", "Character:"), QLineEdit::Normal, QString(), &ok);
        if (ok && !c.isEmpty()) customChar = c.left(2);
    });
    bl->addWidget(custom, 2, 0, 1, 2);
    tabs->addTab(bw, QCoreApplication::translate("Dialogs", "Bullets"));
    auto *nw = new QWidget();
    auto *nf = new QFormLayout(nw);
    auto *format = new QComboBox(nw);
    format->addItems({"1. 2. 3.", "a. b. c.", "A. B. C.", "i. ii. iii.", "I. II. III.", "1) 2) 3)", "(1) (2) (3)"});
    auto *start = new QSpinBox(nw);
    start->setRange(1, 9999);
    nf->addRow(QCoreApplication::translate("Dialogs", "Format:"), format);
    nf->addRow(QCoreApplication::translate("Dialogs", "Start at:"), start);
    tabs->addTab(nw, QCoreApplication::translate("Dialogs", "Numbering"));
    if (numbering) tabs->setCurrentIndex(1);
    dlg.v->addWidget(tabs);
    // From the bullet or number to the text, for either kind of list; it
    // starts at the paragraph's own when it is in a list already.
    const QTextBlockFormat bf = ed->currentBlockFormat();
    auto *indent = measure(bf.textIndent() < 0 ? -bf.textIndent() : 18, &dlg.d);
    indent->setObjectName(QStringLiteral("indent"));
    auto *indentRow = new QFormLayout();
    indentRow->addRow(QCoreApplication::translate("Dialogs", "Indent list by:"), indent);
    dlg.v->addLayout(indentRow);
    if (!dlg.exec()) return;
    if (tabs->currentIndex() == 0) ed->setList(1, 0, customChar.isEmpty() ? chars[group->checkedId()] : customChar, 1, indent->value());
    else ed->setList(2, format->currentIndex() + 1, QString(), start->value(), indent->value());
}

// ---------------- Character Spacing ----------------
void characterSpacingDialog(QWidget *p, Editor *ed)
{
    const QTextCharFormat cf = ed->currentCharFormat();
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Character Spacing"));
    auto *form = new QFormLayout();
    auto *scale = new QSpinBox(&dlg.d);
    scale->setRange(1, 600);
    scale->setSuffix(QStringLiteral("%"));
    scale->setValue(cf.hasProperty(QTextFormat::FontStretch) ? cf.fontStretch() : 100);
    // Tracking: a preset or a percentage of normal spacing, as Publisher has it.
    auto *trackKind = new QComboBox(&dlg.d);
    trackKind->addItem(QCoreApplication::translate("Dialogs", "Very tight"), 75.0);
    trackKind->addItem(QCoreApplication::translate("Dialogs", "Tight"), 87.5);
    trackKind->addItem(QCoreApplication::translate("Dialogs", "Normal"), 100.0);
    trackKind->addItem(QCoreApplication::translate("Dialogs", "Loose"), 112.5);
    trackKind->addItem(QCoreApplication::translate("Dialogs", "Very loose"), 125.0);
    trackKind->addItem(QCoreApplication::translate("Dialogs", "Custom"), -1.0);
    auto *track = new DecimalSpin(&dlg.d);
    track->setRange(0, 600);
    track->setSuffix(QStringLiteral("%"));
    track->setValue(tp::trackingOf(cf));
    auto syncTrackKind = [=] {
        const QSignalBlocker b(trackKind);
        trackKind->setCurrentIndex(trackKind->count() - 1);
        for (int i = 0; i + 1 < trackKind->count(); ++i)
            if (std::abs(trackKind->itemData(i).toDouble() - track->value()) < 0.01) trackKind->setCurrentIndex(i);
    };
    syncTrackKind();
    QObject::connect(trackKind, &QComboBox::currentIndexChanged, &dlg.d, [=](int i) {
        const double v = trackKind->itemData(i).toDouble();
        if (v > 0) { const QSignalBlocker b(track); track->setValue(v); }
    });
    QObject::connect(track, &QDoubleSpinBox::valueChanged, &dlg.d, syncTrackKind);
    // Kerning: space added (or taken) after each selected letter.
    auto *kernKind = new QComboBox(&dlg.d);
    kernKind->addItems({QCoreApplication::translate("Dialogs", "Normal"), QCoreApplication::translate("Dialogs", "Expand"), QCoreApplication::translate("Dialogs", "Condense")});
    const double kernNow = tp::kerningOf(cf);
    auto *kernBy = points(std::abs(kernNow), &dlg.d, 0, 600);
    kernKind->setCurrentIndex(kernNow > 0.0005 ? 1 : kernNow < -0.0005 ? 2 : 0);
    kernBy->setEnabled(kernKind->currentIndex() != 0);
    QObject::connect(kernKind, &QComboBox::currentIndexChanged, &dlg.d, [=](int i) {
        kernBy->setEnabled(i != 0);
        if (i == 0) kernBy->setValue(0);
        else if (kernBy->value() < 0.0005) kernBy->setValue(1);
    });
    auto *kern = new QCheckBox(QCoreApplication::translate("Dialogs", "Automatic pair kerning for fonts"), &dlg.d);
    kern->setChecked(!cf.hasProperty(QTextFormat::FontKerning) || cf.fontKerning());
    auto *kernFrom = points(cf.hasProperty(tp::KernAbove) ? cf.property(tp::KernAbove).toDouble() : 14.0, &dlg.d, 0, 1638);
    kernFrom->setEnabled(kern->isChecked());
    QObject::connect(kern, &QCheckBox::toggled, kernFrom, &QWidget::setEnabled);
    auto *word = points(cf.fontWordSpacing(), &dlg.d, -50, 200);
    form->addRow(QCoreApplication::translate("Dialogs", "Scaling (shrink or stretch):"), scale);
    auto *trackRow = new QHBoxLayout();
    trackRow->addWidget(trackKind);
    trackRow->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "By this amount:"), &dlg.d));
    trackRow->addWidget(track);
    form->addRow(QCoreApplication::translate("Dialogs", "Tracking (selected text):"), trackRow);
    auto *kernByRow = new QHBoxLayout();
    kernByRow->addWidget(kernKind);
    kernByRow->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "By this amount:"), &dlg.d));
    kernByRow->addWidget(kernBy);
    form->addRow(QCoreApplication::translate("Dialogs", "Kerning (selected letters):"), kernByRow);
    form->addRow(QCoreApplication::translate("Dialogs", "Word spacing:"), word);
    auto *kernRow = new QHBoxLayout();
    kernRow->addWidget(kern);
    kernRow->addWidget(kernFrom);
    kernRow->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "and above"), &dlg.d));
    kernRow->addStretch(1);
    form->addRow(kernRow);
    dlg.v->addLayout(form);
    if (!dlg.exec()) return;
    QTextCharFormat f;
    f.setFontStretch(scale->value());
    f.setProperty(tp::Tracking, track->value());
    f.setFontLetterSpacingType(QFont::AbsoluteSpacing);
    f.setFontLetterSpacing(kernKind->currentIndex() == 2 ? -kernBy->value() : kernKind->currentIndex() == 1 ? kernBy->value() : 0.0);
    f.setFontKerning(kern->isChecked());
    f.setProperty(tp::KernAbove, kernFrom->value());
    f.setFontWordSpacing(word->value());
    ed->mergeCharFormat(f, QCoreApplication::translate("Dialogs", "Character Spacing"));
}

// ---------------- Insert Table ----------------
void insertTableDialog(QWidget *p, Editor *ed)
{
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Create Table"));
    auto *form = new QFormLayout();
    auto *rows = new QSpinBox(&dlg.d), *cols = new QSpinBox(&dlg.d);
    rows->setRange(1, 128);
    cols->setRange(1, 128);
    rows->setValue(4);
    cols->setValue(4);
    auto *fmt = new QComboBox(&dlg.d);
    // Each format shows in the program's language; its English name, which
    // the table code knows it by, rides along as the item's data.
    fmt->addItem(QCoreApplication::translate("Dialogs", "None"), QStringLiteral("None"));
    for (int i = 1; i <= 20; ++i) fmt->addItem(QCoreApplication::translate("Dialogs", "Table Style %1").arg(i), QStringLiteral("Table Style %1").arg(i));
    fmt->addItem(QCoreApplication::translate("Dialogs", "Basic 1"), QStringLiteral("Basic 1"));
    fmt->addItem(QCoreApplication::translate("Dialogs", "Basic 2"), QStringLiteral("Basic 2"));
    fmt->addItem(QCoreApplication::translate("Dialogs", "Basic 3"), QStringLiteral("Basic 3"));
    fmt->addItem(QCoreApplication::translate("Dialogs", "Checkbook Register"), QStringLiteral("Checkbook Register"));
    fmt->addItem(QCoreApplication::translate("Dialogs", "List 1"), QStringLiteral("List 1"));
    fmt->addItem(QCoreApplication::translate("Dialogs", "List 2"), QStringLiteral("List 2"));
    fmt->addItem(QCoreApplication::translate("Dialogs", "List 3"), QStringLiteral("List 3"));
    fmt->addItem(QCoreApplication::translate("Dialogs", "Numbers 1"), QStringLiteral("Numbers 1"));
    fmt->addItem(QCoreApplication::translate("Dialogs", "Numbers 2"), QStringLiteral("Numbers 2"));
    fmt->setCurrentIndex(1);
    form->addRow(QCoreApplication::translate("Dialogs", "Number of rows:"), rows);
    form->addRow(QCoreApplication::translate("Dialogs", "Number of columns:"), cols);
    form->addRow(QCoreApplication::translate("Dialogs", "Table format:"), fmt);
    dlg.v->addLayout(form);
    if (!dlg.exec()) return;
    const QSizeF ps = ed->doc()->pageSize();
    const double w = std::min(ps.width() * 0.8, 72.0 * cols->value()), h = 22.0 * rows->value();
    ed->addItem(ed->newTable(QRectF((ps.width() - w) / 2, (ps.height() - h) / 2, w, h), rows->value(), cols->value(), fmt->currentData().toString()));
}

// ---------------- Symbol ----------------
QStringList recentSymbols() { return Settings::get().value(QStringLiteral("symbols/recent")).toStringList(); }

bool insertSymbol(QWidget *p, Editor *ed, const QString &ch, const QString &font)
{
    if (!ed->isEditingText()) {
        jp::plainInformation(p, QCoreApplication::translate("Dialogs", "Symbol"), QCoreApplication::translate("Dialogs", "Click in a text box first."));
        return false;
    }
    if (!font.isEmpty()) {
        QTextCharFormat cf = ed->cursor().charFormat();
        cf.setFontFamilies(QStringList{font});
        ed->beginChange(QCoreApplication::translate("Dialogs", "Insert Symbol"));
        ed->cursor().insertText(ch, cf);
        ed->endChange();
        ed->textEdited();
    } else {
        ed->insertTextBlock(ch, QCoreApplication::translate("Dialogs", "Insert Symbol"));
    }
    QStringList recents = recentSymbols();
    const QString entry = font.isEmpty() ? ch : ch + QLatin1Char('\t') + font;
    recents.removeAll(entry);
    recents.prepend(entry);
    while (recents.size() > 20) recents.removeLast();
    Settings::get().setValue(QStringLiteral("symbols/recent"), recents);
    return true;
}

void symbolDialog(QWidget *p, Editor *ed)
{
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Symbol"), QDialogButtonBox::Close);
    auto *font = new QComboBox(&dlg.d);
    font->addItem(QCoreApplication::translate("Dialogs", "(normal text)"));
    font->addItems(visibleFontFamilies());
    auto *subset = new QComboBox(&dlg.d);
    const QVector<QPair<QString, QPair<int, int>>> ranges = {{QCoreApplication::translate("Dialogs", "Basic Latin"), {0x20, 0x7E}}, {QCoreApplication::translate("Dialogs", "Latin-1 Supplement"), {0xA0, 0xFF}}, {QCoreApplication::translate("Dialogs", "Latin Extended-A"), {0x100, 0x17F}},
                                                            {QCoreApplication::translate("Dialogs", "Greek"), {0x370, 0x3FF}}, {QCoreApplication::translate("Dialogs", "Cyrillic"), {0x400, 0x4FF}}, {QCoreApplication::translate("Dialogs", "General Punctuation"), {0x2000, 0x206F}},
                                                            {QCoreApplication::translate("Dialogs", "Currency Symbols"), {0x20A0, 0x20CF}}, {QCoreApplication::translate("Dialogs", "Letterlike Symbols"), {0x2100, 0x214F}}, {QCoreApplication::translate("Dialogs", "Number Forms"), {0x2150, 0x218F}},
                                                            {QCoreApplication::translate("Dialogs", "Arrows"), {0x2190, 0x21FF}}, {QCoreApplication::translate("Dialogs", "Mathematical Operators"), {0x2200, 0x22FF}}, {QCoreApplication::translate("Dialogs", "Box Drawing"), {0x2500, 0x257F}},
                                                            {QCoreApplication::translate("Dialogs", "Geometric Shapes"), {0x25A0, 0x25FF}}, {QCoreApplication::translate("Dialogs", "Miscellaneous Symbols"), {0x2600, 0x26FF}}, {QCoreApplication::translate("Dialogs", "Dingbats"), {0x2700, 0x27BF}}};
    for (const auto &r : ranges) subset->addItem(r.first);
    auto *top = new QHBoxLayout();
    top->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "Font:")));
    top->addWidget(font, 1);
    top->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "Subset:")));
    top->addWidget(subset, 1);
    dlg.v->addLayout(top);
    auto makeGrid = [&](int rows) {
        auto *g = new QTableWidget(rows, 16, &dlg.d);
        g->horizontalHeader()->hide();
        g->verticalHeader()->hide();
        g->setEditTriggers(QAbstractItemView::NoEditTriggers);
        for (int c = 0; c < 16; ++c) g->setColumnWidth(c, 32);
        return g;
    };
    auto *grid = makeGrid(0);
    grid->setMinimumSize(560, 300);
    // Recently used symbols: one row, each in the font it was used in.
    auto *recent = makeGrid(1);
    recent->setRowHeight(0, 32);
    recent->setFixedHeight(36);
    recent->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    recent->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto *code = plainLabel(new QLabel(&dlg.d));
    auto fill = [=] {
        const auto r = ranges[subset->currentIndex()].second;
        const int n = r.second - r.first + 1;
        grid->setRowCount((n + 15) / 16);
        grid->clearContents();
        QFont f = grid->font();
        if (font->currentIndex() > 0) f.setFamily(font->currentText());
        f.setPointSize(14);
        for (int i = 0; i < n; ++i) {
            auto *cell = new QTableWidgetItem(QString(QChar(char16_t(r.first + i))));
            cell->setTextAlignment(Qt::AlignCenter);
            cell->setFont(f);
            cell->setData(Qt::UserRole, r.first + i);
            cell->setData(Qt::UserRole + 1, font->currentIndex() > 0 ? font->currentText() : QString());
            grid->setItem(i / 16, i % 16, cell);
        }
        for (int row = 0; row < grid->rowCount(); ++row) grid->setRowHeight(row, 32);
    };
    auto fillRecent = [=] {
        recent->clearContents();
        const QStringList recents = recentSymbols();
        for (int i = 0; i < recents.size() && i < 16; ++i) {
            const QString ch = recents[i].section(QLatin1Char('\t'), 0, 0), fam = recents[i].section(QLatin1Char('\t'), 1);
            auto *cell = new QTableWidgetItem(ch);
            QFont f = recent->font();
            if (!fam.isEmpty()) f.setFamily(fam);
            f.setPointSize(14);
            cell->setFont(f);
            cell->setTextAlignment(Qt::AlignCenter);
            cell->setData(Qt::UserRole, ch.isEmpty() ? 0 : int(ch.at(0).unicode()));
            cell->setData(Qt::UserRole + 1, fam);
            recent->setItem(0, i, cell);
        }
    };
    QObject::connect(subset, &QComboBox::currentIndexChanged, &dlg.d, fill);
    QObject::connect(font, &QComboBox::currentIndexChanged, &dlg.d, fill);
    fill();
    fillRecent();
    QTableWidget *chosenFrom = grid;
    auto showCode = [=](QTableWidgetItem *it) {
        if (it) code->setText(QCoreApplication::translate("Dialogs", "Character code: %1 (Unicode U+%2)").arg(it->data(Qt::UserRole).toInt()).arg(it->data(Qt::UserRole).toInt(), 4, 16, QChar('0')).toUpper());
    };
    QObject::connect(grid, &QTableWidget::currentItemChanged, &dlg.d, [&, showCode](QTableWidgetItem *it) { chosenFrom = grid; showCode(it); });
    QObject::connect(recent, &QTableWidget::currentItemChanged, &dlg.d, [&, showCode](QTableWidgetItem *it) { chosenFrom = recent; showCode(it); });
    auto insert = [&, fillRecent](QTableWidgetItem *it) {
        if (!it || it->text().isEmpty()) return;
        if (insertSymbol(&dlg.d, ed, it->text(), it->data(Qt::UserRole + 1).toString())) fillRecent();
    };
    QObject::connect(grid, &QTableWidget::itemDoubleClicked, &dlg.d, insert);
    QObject::connect(recent, &QTableWidget::itemDoubleClicked, &dlg.d, insert);
    auto *ins = dlg.bb->addButton(QCoreApplication::translate("Dialogs", "Insert"), QDialogButtonBox::ActionRole);
    QObject::connect(ins, &QPushButton::clicked, &dlg.d, [&] { insert(chosenFrom->currentItem()); });
    dlg.v->addWidget(grid, 1);
    dlg.v->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "Recently used symbols:"), &dlg.d));
    dlg.v->addWidget(recent);
    dlg.v->addWidget(code);
    dlg.exec();
}

// ---------------- Date & Time ----------------
void dateTimeDialog(QWidget *p, Editor *ed)
{
    if (!ed->isEditingText()) {
        jp::plainInformation(p, QCoreApplication::translate("Dialogs", "Date and Time"), QCoreApplication::translate("Dialogs", "Click in a text box first."));
        return;
    }
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Date and Time"));
    auto *list = new QListWidget(&dlg.d);
    const QStringList &formats = dateTimeFormats();
    const QDateTime now = QDateTime::currentDateTime();
    for (const QString &f : formats) {
        auto *it = new QListWidgetItem(now.toString(f));
        it->setData(Qt::UserRole, f);
        list->addItem(it);
    }
    list->setCurrentRow(0);
    auto *update = new QCheckBox(QCoreApplication::translate("Dialogs", "Update automatically"), &dlg.d);
    dlg.v->addWidget(list);
    dlg.v->addWidget(update);
    QObject::connect(list, &QListWidget::itemDoubleClicked, &dlg.d, &QDialog::accept);
    if (!dlg.exec()) return;
    const QString f = list->currentItem()->data(Qt::UserRole).toString();
    if (update->isChecked()) ed->insertField(QStringLiteral("datetime:") + f);
    else ed->insertTextBlock(now.toString(f), QCoreApplication::translate("Dialogs", "Insert Date and Time"));
}

// ---------------- Hyperlink ----------------
void hyperlinkDialog(QWidget *p, Editor *ed)
{
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Insert Hyperlink"));
    auto *kind = new QComboBox(&dlg.d);
    kind->addItems({QCoreApplication::translate("Dialogs", "Existing file or web page"), QCoreApplication::translate("Dialogs", "Place in this document"), QCoreApplication::translate("Dialogs", "Email address")});
    auto *addr = new QLineEdit(&dlg.d);
    auto *display = new QLineEdit(&dlg.d);
    const QTextCharFormat cf = ed->currentCharFormat();
    addr->setText(cf.anchorHref());
    if (ed->isEditingText() && ed->cursor().hasSelection()) display->setText(ed->cursor().selectedText());
    auto *pages = new QComboBox(&dlg.d);
    for (int i = 0; i < ed->doc()->pages.size(); ++i) pages->addItem(QCoreApplication::translate("Dialogs", "Page %1").arg(i + 1));
    auto *form = new QFormLayout();
    form->addRow(QCoreApplication::translate("Dialogs", "Link to:"), kind);
    form->addRow(QCoreApplication::translate("Dialogs", "Text to display:"), display);
    form->addRow(QCoreApplication::translate("Dialogs", "Address:"), addr);
    form->addRow(QCoreApplication::translate("Dialogs", "Page:"), pages);
    pages->setEnabled(false);
    QObject::connect(kind, &QComboBox::currentIndexChanged, &dlg.d, [=](int i) {
        pages->setEnabled(i == 1);
        addr->setEnabled(i != 1);
        addr->setPlaceholderText(i == 2 ? QCoreApplication::translate("Dialogs", "name@example.com") : QStringLiteral("https://"));
    });
    dlg.v->addLayout(form);
    auto *remove = dlg.bb->addButton(QCoreApplication::translate("Dialogs", "Remove Link"), QDialogButtonBox::ResetRole);
    bool removing = false;
    QObject::connect(remove, &QPushButton::clicked, &dlg.d, [&] { removing = true; dlg.d.accept(); });
    if (!dlg.exec()) return;
    QString href = kind->currentIndex() == 1 ? QStringLiteral("#page%1").arg(pages->currentIndex() + 1)
                 : kind->currentIndex() == 2 ? "mailto:" + addr->text().remove("mailto:") : addr->text().trimmed();
    if (kind->currentIndex() == 0 && !href.contains("://") && !href.isEmpty() && !QFileInfo::exists(href)) href = "https://" + href;
    if (!ed->isEditingText()) {
        ed->forEachSelected(QCoreApplication::translate("Dialogs", "Hyperlink"), [&](Item *it) { it->hyperlink = removing ? QString() : href; });
        return;
    }
    QTextCharFormat f;
    if (removing) {
        f.setAnchor(false);
        f.setAnchorHref(QString());
        f.setFontUnderline(false);
        ed->mergeCharFormat(f, QCoreApplication::translate("Dialogs", "Remove Hyperlink"));
        return;
    }
    f.setAnchor(true);
    f.setAnchorHref(href);
    if (!ed->cursor().hasSelection()) {
        ed->beginChange(QCoreApplication::translate("Dialogs", "Insert Hyperlink"));
        QTextCharFormat cf2 = ed->cursor().charFormat();
        cf2.merge(f);
        ed->cursor().insertText(display->text().isEmpty() ? href : display->text(), cf2);
        ed->endChange();
        ed->textEdited();
    } else {
        ed->mergeCharFormat(f, QCoreApplication::translate("Dialogs", "Insert Hyperlink"));
    }
}

// Seamless textures generated from noise, so no texture images need shipping.
QImage proceduralTexture(const QString &name, int size)
{
    QImage img(size, size, QImage::Format_RGB32);
    QRandomGenerator rng(qHash(name));
    struct Spec { QColor a, b; double grain; int kind; };
    static const QHash<QString, Spec> specs{
        {"Canvas", {QColor(232, 224, 205), QColor(206, 196, 172), 0.6, 1}}, {"Denim", {QColor(70, 96, 140), QColor(46, 64, 104), 0.7, 1}},
        {"Linen", {QColor(240, 234, 222), QColor(220, 210, 192), 0.4, 1}}, {"Paper", {QColor(250, 248, 242), QColor(232, 228, 216), 0.3, 0}},
        {"Parchment", {QColor(242, 228, 196), QColor(214, 190, 150), 0.5, 0}}, {"Recycled", {QColor(214, 206, 190), QColor(176, 166, 148), 0.9, 0}},
        {"Wood", {QColor(176, 128, 82), QColor(132, 88, 52), 0.5, 2}}, {"Marble", {QColor(238, 238, 236), QColor(170, 170, 176), 0.4, 3}},
        {"Granite", {QColor(150, 146, 140), QColor(90, 86, 84), 1.0, 0}}, {"Cork", {QColor(196, 150, 100), QColor(150, 108, 64), 1.0, 0}},
        {"Sand", {QColor(232, 212, 170), QColor(204, 180, 134), 0.8, 0}}, {"Stationery", {QColor(252, 252, 250), QColor(214, 226, 240), 0.2, 4}},
    };
    const Spec s = specs.value(name, specs["Paper"]);
    // Tileable value noise.
    const int g = 16;
    QVector<double> grid(g * g);
    for (double &v : grid) v = rng.generateDouble();
    auto noise = [&](double x, double y) {
        x = std::fmod(x, g); y = std::fmod(y, g);
        const int x0 = int(x), y0 = int(y), x1 = (x0 + 1) % g, y1 = (y0 + 1) % g;
        const double fx = x - x0, fy = y - y0;
        auto at = [&](int i, int j) { return grid[j * g + i]; };
        const double a = at(x0, y0) * (1 - fx) + at(x1, y0) * fx, b = at(x0, y1) * (1 - fx) + at(x1, y1) * fx;
        return a * (1 - fy) + b * fy;
    };
    for (int y = 0; y < size; ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < size; ++x) {
            const double u = double(x) / size * g, v = double(y) / size * g;
            double t = 0.5 * noise(u, v) + 0.25 * noise(u * 2, v * 2) + 0.125 * noise(u * 4, v * 4) + 0.125 * rng.generateDouble() * s.grain;
            if (s.kind == 1) t = 0.6 * t + 0.4 * (0.5 + 0.5 * std::sin(x * 1.3) * std::sin(y * 1.3));
            if (s.kind == 2) t = 0.5 + 0.5 * std::sin((double(y) / size * 2 * M_PI * 6) + 4 * noise(u, v));
            if (s.kind == 3) t = std::pow(std::abs(std::sin((double(x + y) / size * 2 * M_PI * 2) + 5 * noise(u, v))), 0.3);
            if (s.kind == 4) t = (y % std::max(4, size / 16) == 0) ? 1.0 : 0.1 * t;
            t = std::clamp(t, 0.0, 1.0);
            line[x] = qRgb(int(s.a.red() + (s.b.red() - s.a.red()) * t), int(s.a.green() + (s.b.green() - s.a.green()) * t), int(s.a.blue() + (s.b.blue() - s.a.blue()) * t));
        }
    }
    return img;
}

// ---------------- Fill Effects ----------------
bool fillEffectsDialog(QWidget *p, Editor *ed, Fill &fill, const QString &title)
{
    Fill f = fill;
    Dlg dlg(p, title.isEmpty() ? QCoreApplication::translate("Dialogs", "Fill Effects") : title);
    auto *tabs = new QTabWidget(&dlg.d);
    // Gradient.
    auto *g = new QWidget();
    auto *gf = new QFormLayout(g);
    auto *oneTwo = new QComboBox(g);
    oneTwo->addItems({QCoreApplication::translate("Dialogs", "Two colors"), QCoreApplication::translate("Dialogs", "Preset")});
    auto *c1 = colorPick(ed, f.type == Fill::Gradient ? f.color : ColorRef::scheme(Accent1), false, g);
    auto *c2 = colorPick(ed, f.type == Fill::Gradient ? f.color2 : ColorRef::rgb(Qt::white), false, g);
    auto *gstyle = new QComboBox(g);
    gstyle->addItems({QCoreApplication::translate("Dialogs", "Linear"), QCoreApplication::translate("Dialogs", "Radial"), QCoreApplication::translate("Dialogs", "Rectangular"), QCoreApplication::translate("Dialogs", "From center")});
    gstyle->setCurrentIndex(int(f.gradType));
    auto *angle = new QSpinBox(g);
    angle->setRange(0, 359);
    angle->setSuffix(QStringLiteral("°"));
    angle->setValue(int(f.angle));
    auto *preset = new QComboBox(g);
    preset->addItems({QCoreApplication::translate("Dialogs", "Early Sunset"), QCoreApplication::translate("Dialogs", "Late Sunset"), QCoreApplication::translate("Dialogs", "Nightfall"), QCoreApplication::translate("Dialogs", "Daybreak"), QCoreApplication::translate("Dialogs", "Horizon"), QCoreApplication::translate("Dialogs", "Desert"), QCoreApplication::translate("Dialogs", "Ocean"), QCoreApplication::translate("Dialogs", "Calm Water"), QCoreApplication::translate("Dialogs", "Fire"), QCoreApplication::translate("Dialogs", "Fog"), QCoreApplication::translate("Dialogs", "Moss"), QCoreApplication::translate("Dialogs", "Peacock"), QCoreApplication::translate("Dialogs", "Wheat"), QCoreApplication::translate("Dialogs", "Parchment"), QCoreApplication::translate("Dialogs", "Mahogany"), QCoreApplication::translate("Dialogs", "Rainbow"), QCoreApplication::translate("Dialogs", "Gold"), QCoreApplication::translate("Dialogs", "Silver"), QCoreApplication::translate("Dialogs", "Chrome"), QCoreApplication::translate("Dialogs", "Brass")});
    gf->addRow(QCoreApplication::translate("Dialogs", "Colors:"), oneTwo);
    gf->addRow(QCoreApplication::translate("Dialogs", "Color 1:"), c1);
    gf->addRow(QCoreApplication::translate("Dialogs", "Color 2:"), c2);
    gf->addRow(QCoreApplication::translate("Dialogs", "Preset:"), preset);
    gf->addRow(QCoreApplication::translate("Dialogs", "Shading style:"), gstyle);
    gf->addRow(QCoreApplication::translate("Dialogs", "Angle:"), angle);
    tabs->addTab(g, QCoreApplication::translate("Dialogs", "Gradient"));
    // Texture / picture.
    auto *t = new QWidget();
    auto *tf = new QVBoxLayout(t);
    auto *texList = new QListWidget(t);
    texList->setViewMode(QListView::IconMode);
    texList->setIconSize(QSize(56, 56));
    // Each texture shows in the program's language; its English name, which
    // makes the texture, rides along as the item's data.
    const QVector<QPair<QString, QString>> textures{
        {QStringLiteral("Canvas"), QCoreApplication::translate("Dialogs", "Canvas")},
        {QStringLiteral("Denim"), QCoreApplication::translate("Dialogs", "Denim")},
        {QStringLiteral("Linen"), QCoreApplication::translate("Dialogs", "Linen")},
        {QStringLiteral("Paper"), QCoreApplication::translate("Dialogs", "Paper")},
        {QStringLiteral("Parchment"), QCoreApplication::translate("Dialogs", "Parchment")},
        {QStringLiteral("Recycled"), QCoreApplication::translate("Dialogs", "Recycled")},
        {QStringLiteral("Wood"), QCoreApplication::translate("Dialogs", "Wood")},
        {QStringLiteral("Marble"), QCoreApplication::translate("Dialogs", "Marble")},
        {QStringLiteral("Granite"), QCoreApplication::translate("Dialogs", "Granite")},
        {QStringLiteral("Cork"), QCoreApplication::translate("Dialogs", "Cork")},
        {QStringLiteral("Sand"), QCoreApplication::translate("Dialogs", "Sand")},
        {QStringLiteral("Stationery"), QCoreApplication::translate("Dialogs", "Stationery")}};
    for (const auto &[key, shown] : textures) {
        auto *it = new QListWidgetItem(QIcon(QPixmap::fromImage(proceduralTexture(key, 112))), shown);
        it->setData(Qt::UserRole, key);
        texList->addItem(it);
    }
    tf->addWidget(texList);
    auto *otherTex = new QPushButton(QCoreApplication::translate("Dialogs", "Other Texture…"), t);
    tf->addWidget(otherTex);
    tabs->addTab(t, QCoreApplication::translate("Dialogs", "Texture"));
    auto *pt = new QWidget();
    auto *pv = new QVBoxLayout(pt);
    auto *pickPic = new QPushButton(QCoreApplication::translate("Dialogs", "Select Picture…"), pt);
    auto *picLabel = plainLabel(new QLabel(pt));
    picLabel->setFixedSize(200, 140);
    picLabel->setFrameShape(QFrame::StyledPanel);
    auto *tile = new QCheckBox(QCoreApplication::translate("Dialogs", "Tile picture as texture"), pt);
    tile->setChecked(f.tile);
    pv->addWidget(pickPic);
    pv->addWidget(picLabel);
    pv->addWidget(tile);
    pv->addStretch(1);
    if (f.type == Fill::Picture && !f.imageId.isEmpty())
        picLabel->setPixmap(QPixmap::fromImage(ed->doc()->image(f.imageId).scaled(200, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    tabs->addTab(pt, QCoreApplication::translate("Dialogs", "Picture"));
    // Pattern.
    auto *pa = new QWidget();
    auto *paf = new QFormLayout(pa);
    auto *patList = new QListWidget(pa);
    patList->setViewMode(QListView::IconMode);
    patList->setIconSize(QSize(32, 32));
    const auto pats = patternBrushes();
    for (int i = 0; i < pats.size(); ++i) {
        QPixmap pm(32, 32);
        pm.fill(Qt::white);
        QPainter pp(&pm);
        pp.fillRect(pm.rect(), QBrush(Qt::black, pats[i]));
        pp.end();
        patList->addItem(new QListWidgetItem(QIcon(pm), QString()));
    }
    patList->setCurrentRow(f.type == Fill::Pattern ? f.pattern : 0);
    auto *fg = colorPick(ed, f.type == Fill::Pattern ? f.color : ColorRef::scheme(Main), false, pa);
    auto *bg = colorPick(ed, f.type == Fill::Pattern ? f.color2 : ColorRef::rgb(Qt::white), true, pa);
    paf->addRow(patList);
    paf->addRow(QCoreApplication::translate("Dialogs", "Foreground:"), fg);
    paf->addRow(QCoreApplication::translate("Dialogs", "Background:"), bg);
    tabs->addTab(pa, QCoreApplication::translate("Dialogs", "Pattern"));
    // Tint.
    auto *ti = new QWidget();
    auto *tif = new QFormLayout(ti);
    auto *base = colorPick(ed, f.type == Fill::Solid ? f.color : ColorRef::scheme(Accent1), false, ti);
    auto *tintSlider = new QSlider(Qt::Horizontal, ti);
    tintSlider->setRange(-90, 90);
    tintSlider->setValue(f.color.lighten() ? -f.color.lighten() : f.color.darken());
    tif->addRow(QCoreApplication::translate("Dialogs", "Base color:"), base);
    tif->addRow(QCoreApplication::translate("Dialogs", "Tint (left) or shade (right):"), tintSlider);
    tabs->addTab(ti, QCoreApplication::translate("Dialogs", "Tint"));
    tabs->setCurrentIndex(f.type == Fill::Gradient ? 0 : f.type == Fill::Texture ? 1 : f.type == Fill::Picture ? 2 : f.type == Fill::Pattern ? 3 : 0);
    dlg.v->addWidget(tabs);
    QString texChoice, picChoice = f.imageId;
    QObject::connect(texList, &QListWidget::currentItemChanged, &dlg.d, [&](QListWidgetItem *cur) { texChoice = cur ? cur->data(Qt::UserRole).toString() : QString(); });
    QObject::connect(otherTex, &QPushButton::clicked, &dlg.d, [&] {
        const QString file = QFileDialog::getOpenFileName(&dlg.d, QCoreApplication::translate("Dialogs", "Texture"), QString(), QCoreApplication::translate("Dialogs", "Pictures (*.png *.jpg *.jpeg *.bmp *.gif)"));
        if (file.isEmpty()) return;
        QFile fl(file);
        if (fl.open(QIODevice::ReadOnly)) texChoice = "file:" + ed->doc()->addImage(fl.readAll(), QFileInfo(file).suffix().toLower(), file);
    });
    QObject::connect(pickPic, &QPushButton::clicked, &dlg.d, [&] {
        const QString file = QFileDialog::getOpenFileName(&dlg.d, QCoreApplication::translate("Dialogs", "Picture"), QString(), QCoreApplication::translate("Dialogs", "Pictures (*.png *.jpg *.jpeg *.bmp *.gif *.svg)"));
        if (file.isEmpty()) return;
        QFile fl(file);
        if (!fl.open(QIODevice::ReadOnly)) return;
        picChoice = ed->doc()->addImage(fl.readAll(), QFileInfo(file).suffix().toLower(), file);
        picLabel->setPixmap(QPixmap::fromImage(ed->doc()->image(picChoice).scaled(200, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    });
    if (!dlg.exec()) return false;
    switch (tabs->currentIndex()) {
    case 0: {
        Fill n;
        n.type = Fill::Gradient;
        n.gradType = Fill::GradType(std::min(gstyle->currentIndex(), 2));
        n.angle = angle->value();
        if (oneTwo->currentIndex() == 1) {
            static const char *presets[][2] = {{"#5B2C83", "#F28A30"}, {"#1A0B2E", "#E8533B"}, {"#0B1026", "#2B4F81"}, {"#9FC4E8", "#FFF4D6"}, {"#F2B134", "#3A86FF"}, {"#C2925A", "#F4E3C1"},
                                               {"#00456E", "#5BC0EB"}, {"#B8E1F0", "#3A7CA5"}, {"#FFD200", "#C0121B"}, {"#DDE3E8", "#8796A3"}, {"#2E5D34", "#9BBF6A"}, {"#00808C", "#2C2C7A"},
                                               {"#F5DEB3", "#C69C5A"}, {"#F4EDD7", "#D6C29A"}, {"#5C1F0E", "#A0522D"}, {"#E53935", "#1E88E5"}, {"#F7D774", "#A8741A"},
                                               {"#F4F4F4", "#8A8A8A"}, {"#FFFFFF", "#5A5A5A"}, {"#E7C66A", "#8B6914"}};
            const int i = preset->currentIndex();
            n.color = ColorRef::rgb(QColor(presets[i][0]));
            n.color2 = ColorRef::rgb(QColor(presets[i][1]));
        } else {
            n.color = c1->current();
            n.color2 = c2->current();
        }
        n.transparency = f.transparency;
        f = n;
        break;
    }
    case 1: {
        if (texChoice.isEmpty()) return false;
        Fill n;
        n.type = Fill::Texture;
        n.tile = true;
        if (texChoice.startsWith("file:")) n.imageId = texChoice.mid(5);
        else {
            QByteArray png;
            QBuffer b(&png);
            b.open(QIODevice::WriteOnly);
            proceduralTexture(texChoice, 256).save(&b, "PNG");
            n.imageId = ed->doc()->addImage(png, "png", "texture:" + texChoice);
        }
        f = n;
        break;
    }
    case 2: {
        if (picChoice.isEmpty()) return false;
        Fill n;
        n.type = Fill::Picture;
        n.imageId = picChoice;
        n.tile = tile->isChecked();
        f = n;
        break;
    }
    case 3: {
        Fill n;
        n.type = Fill::Pattern;
        n.pattern = std::max(0, patList->currentRow());
        n.color = fg->current();
        n.color2 = bg->current();
        f = n;
        break;
    }
    case 4: {
        const ColorRef b = base->current();
        const int v = tintSlider->value();
        ColorRef c = b.kind() == ColorRef::Scheme ? ColorRef::scheme(b.slot(), v < 0 ? -v : 0, v > 0 ? v : 0) : b;
        if (b.kind() == ColorRef::Rgb) c = ColorRef::rgb(v < 0 ? mix(b.rgbValue(), Qt::white, -v / 100.0) : mix(b.rgbValue(), Qt::black, v / 100.0));
        f = Fill::solid(c);
        break;
    }
    }
    fill = f;
    return true;
}

// ---------------- Word count ----------------
void wordCountDialog(QWidget *p, Editor *ed)
{
    Document *d = ed->doc();
    int words = 0, chars = 0, charsNoSpace = 0, paras = 0, lines = 0, boxes = 0, pics = 0;
    for (auto it = d->stories.cbegin(); it != d->stories.cend(); ++it) {
        const QString t = (*it)->doc->toPlainText();
        words += t.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
        chars += t.size();
        charsNoSpace += QString(t).remove(QRegularExpression("\\s")).size();
        paras += (*it)->doc->blockCount();
        lines += t.count('\n') + 1;
    }
    d->forEachItem([&](Item *it, int, const QString &) {
        if (it->type() == ItemType::Text) ++boxes;
        if (it->type() == ItemType::Picture) ++pics;
    });
    QString sel;
    if (ed->isEditingText() && ed->cursor().hasSelection())
        sel = QCoreApplication::translate("Dialogs", "<br><br><b>Selection</b><br>Words: %1").arg(ed->cursor().selectedText().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size());
    jp::plainInformation(p, QCoreApplication::translate("Dialogs", "Word Count"),
                             QCoreApplication::translate("Dialogs", "<b>Publication</b><br>Pages: %1<br>Words: %2<br>Characters (no spaces): %3<br>Characters (with spaces): %4"
                                                                    "<br>Paragraphs: %5<br>Text boxes: %6<br>Pictures: %7%8")
                                 .arg(d->pages.size()).arg(words).arg(charsNoSpace).arg(chars).arg(paras).arg(boxes).arg(pics).arg(sel));
    Q_UNUSED(lines);
}

// ---------------- Tints ----------------
void tintsDialog(QWidget *p, Editor *ed, const std::function<void(const ColorRef &)> &apply)
{
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Tints"));
    auto *form = new QFormLayout();
    auto *base = new QComboBox(&dlg.d);
    for (int i = 0; i < SlotCount; ++i) base->addItem(slotName(i));
    base->setCurrentIndex(Accent1);
    auto *list = new QListWidget(&dlg.d);
    list->setViewMode(QListView::IconMode);
    list->setIconSize(QSize(36, 24));
    auto fill = [=] {
        list->clear();
        for (int t = -90; t <= 90; t += 10) {
            const ColorRef c = ColorRef::scheme(base->currentIndex(), t < 0 ? -t : 0, t > 0 ? t : 0);
            QPixmap pm(36, 24);
            pm.fill(c.resolve(ed->doc()->colors));
            auto *it = new QListWidgetItem(QIcon(pm), t < 0 ? QCoreApplication::translate("Dialogs", "%1% tint").arg(100 + t) : t > 0 ? QCoreApplication::translate("Dialogs", "%1% shade").arg(100 - t) : QCoreApplication::translate("Dialogs", "Base"));
            it->setData(Qt::UserRole, c.toString());
            list->addItem(it);
        }
    };
    QObject::connect(base, &QComboBox::currentIndexChanged, &dlg.d, fill);
    fill();
    form->addRow(QCoreApplication::translate("Dialogs", "Base color:"), base);
    form->addRow(list);
    dlg.v->addLayout(form);
    if (!dlg.exec() || !list->currentItem()) return;
    apply(ColorRef::fromString(list->currentItem()->data(Qt::UserRole).toString()));
}

// ---------------- TextArt ----------------
void textArtTextDialog(QWidget *p, Editor *ed, const QString &itemId)
{
    auto *w = dynamic_cast<TextArtItem *>(ed->doc()->item(itemId));
    if (!w) return;
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Edit Text Art Text"));
    auto *row = new QHBoxLayout();
    auto *font = new QComboBox(&dlg.d);
    font->addItems(visibleFontFamilies());
    font->setCurrentText(w->font);
    auto *size = new QSpinBox(&dlg.d);
    size->setRange(4, 400);
    size->setValue(int(w->size));
    auto *bold = new QCheckBox(QCoreApplication::translate("Dialogs", "Bold"), &dlg.d);
    bold->setChecked(w->bold);
    auto *italic = new QCheckBox(QCoreApplication::translate("Dialogs", "Italic"), &dlg.d);
    italic->setChecked(w->italic);
    row->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "Font:")));
    row->addWidget(font, 1);
    row->addWidget(new QLabel(QCoreApplication::translate("Dialogs", "Size:")));
    row->addWidget(size);
    row->addWidget(bold);
    row->addWidget(italic);
    dlg.v->addLayout(row);
    auto *text = new QPlainTextEdit(w->text, &dlg.d);
    text->setMinimumSize(420, 140);
    QFont tf(w->font);
    tf.setPointSize(16);
    text->setFont(tf);
    QObject::connect(font, &QComboBox::currentTextChanged, text, [text](const QString &f) { QFont x(f); x.setPointSize(16); text->setFont(x); });
    dlg.v->addWidget(text);
    text->selectAll();
    if (!dlg.exec()) return;
    ed->change(QCoreApplication::translate("Dialogs", "Edit Text Art Text"), [&] {
        const double oldSize = w->size;
        w->text = text->toPlainText().isEmpty() ? QStringLiteral(" ") : text->toPlainText();
        w->font = font->currentText();
        w->bold = bold->isChecked();
        w->italic = italic->isChecked();
        w->size = size->value();
        // A new size scales the frame.
        if (std::abs(oldSize - w->size) > 0.1 && oldSize > 0) {
            const double k = w->size / oldSize;
            const QPointF c = w->rect.center();
            w->rect.setSize(w->rect.size() * k);
            w->rect.moveCenter(c);
        }
    });
}

} // namespace jp
