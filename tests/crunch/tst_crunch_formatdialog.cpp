// Tests of the Format Cells dialog (apps/crunch/app/formatdialog.h): what it shows of a cell's format, that an untouched dialog changes nothing, what each tab gives back when it is
// changed (the number format with its sample, the alignment, the font, the lines of the border, the fill, and protection), and how it reads in another language. Controls are driven by name;
// every locale is set explicitly.
#include "app/formatdialog.h"
#include "app/widgets.h"
#include "core/formatcodes.h"
#include "core/color.h"
#include "core/editsession.h"
#include "core/fonts.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QToolButton>
#include <QtTest>

#include "../testrunner.h"

using namespace jp;
using namespace jp::office::sheet;
using E = jp::crunch::EditSession;

namespace {

FormatCellsDialog::Input inputFor(const QLocale &locale = QLocale(QLocale::English, QLocale::UnitedStates))
{
    FormatCellsDialog::Input in;
    in.locale = locale;
    in.sampleIsNumber = true;
    in.sampleNumber = 1234.5;
    return in;
}

template <class T> T *control(FormatCellsDialog &d, const char *name)
{
    T *w = d.findChild<T *>(QString::fromLatin1(name));
    if (!w) qWarning("no control named %s", name);
    return w;
}

} // namespace

class CrunchFormatDialogTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // The words of the dialog are the program's own: none of the descriptions, notes, sample text, and names of choices is a sentence or a name that another program's dialog uses. Every
    // piece of text on every tab (the descriptions of all twelve number categories among them) is checked for the wording the review found copied.
    void theWordsOfTheDialogAreOurOwn()
    {
        const QStringList copied = {QStringLiteral("general display of numbers"),     QStringLiteral("specialized formatting"),        QStringLiteral("used for general monetary"),
                                    QStringLiteral("serial numbers as"),              QStringLiteral("multiply the cell value"),        QStringLiteral("exponential notation"),
                                    QStringLiteral("treated as text even when"),      QStringLiteral("tracking list and database"),     QStringLiteral("existing codes as a starting point"),
                                    QStringLiteral("no specific number format"),      QStringLiteral("no effect until you protect"),    QStringLiteral("(indent)"),
                                    QStringLiteral("center across selection"),        QStringLiteral("aabbccyyzz"),                     QStringLiteral("reverse diagonal stripe"),
                                    QStringLiteral("diagonal crosshatch"),            QStringLiteral("% gray"),                         QStringLiteral("line up the currency symbols")};
        const QString contextWord = QStringLiteral("context");
        FormatCellsDialog d(inputFor());
        QStringList seen;
        auto collect = [&] {
            for (QLabel *l : d.findChildren<QLabel *>()) seen << l->text();
            for (QCheckBox *c : d.findChildren<QCheckBox *>()) seen << c->text();
            for (QComboBox *c : d.findChildren<QComboBox *>())
                for (int i = 0; i < c->count(); ++i) seen << c->itemText(i);
            for (QListWidget *l : d.findChildren<QListWidget *>())
                for (int i = 0; i < l->count(); ++i) seen << l->item(i)->text();
        };
        QListWidget *categories = control<QListWidget>(d, "numberCategory");
        QVERIFY(categories && categories->count() == 12);
        for (int row = 0; row < categories->count(); ++row) {
            categories->setCurrentRow(row);
            collect();
        }
        QCOMPARE(seen.size() > 100, true);
        int descriptions = 0;
        for (const QString &text : seen) {
            const QString lower = text.toLower();
            for (const QString &phrase : copied) QVERIFY2(!lower.contains(phrase), qPrintable(text));
            QVERIFY2(lower != contextWord, qPrintable(text));
            if (text.size() > 40) ++descriptions;
        }
        QVERIFY(descriptions >= 12);   // (the twelve descriptions are there to be checked)
        // each category has a description of its own
        QSet<QString> distinct;
        for (int row = 0; row < categories->count(); ++row) {
            categories->setCurrentRow(row);
            QString text;
            for (QLabel *l : d.findChildren<QLabel *>())
                if (l->wordWrap() && l->isVisibleTo(&d) && l->text().size() > 30 && !l->text().contains(QLatin1String("sheet that is protected"))) text += l->text() + QLatin1Char('|');
            distinct.insert(text);
        }
        QVERIFY2(distinct.size() >= 10, qPrintable(QString::number(distinct.size())));
    }

    // The dialog begins with the format it was given on every tab, and a dialog that is accepted untouched gives back nothing to change.
    void anUntouchedDialogChangesNothing()
    {
        FormatCellsDialog::Input in = inputFor();
        in.view.numberFormat = QStringLiteral("#,##0.00");
        in.view.bold = true;
        in.view.fontName = QStringLiteral("Georgia");
        in.view.fontSize = 14;
        in.view.fontColor = QColor(0xC0, 0, 0);
        in.view.horizontal = HAlign::Right;
        in.view.indent = 2;
        in.view.wrap = true;
        in.view.rotation = 45;
        in.view.vertical = VAlign::Top;
        in.view.pattern = PatternKind::Solid;
        in.view.fillColor = QColor(0xFF, 0xFF, 0);
        in.view.border.bottom.style = BorderStyle::Double;
        in.view.border.bottom.color = ColorSpec::fromRgb(0x0000FF);
        in.view.locked = false;
        in.view.formulaHidden = true;
        FormatCellsDialog d(in);
        QVERIFY(d.result().isEmpty());
        QCOMPARE(d.code(), QStringLiteral("#,##0.00"));
        QCOMPARE(control<QComboBox>(d, "fontName")->currentText(), QStringLiteral("Georgia"));
        QCOMPARE(control<QComboBox>(d, "fontStyle")->currentIndex(), 2);
        QCOMPARE(control<QComboBox>(d, "fontSize")->currentText(), QStringLiteral("14"));
        QCOMPARE(control<QComboBox>(d, "horizontal")->currentIndex(), 3);
        QCOMPARE(control<QSpinBox>(d, "indent")->value(), 2);
        QVERIFY(control<QCheckBox>(d, "wrap")->isChecked());
        QCOMPARE(control<QSpinBox>(d, "degrees")->value(), 45);
        QCOMPARE(control<QComboBox>(d, "vertical")->currentIndex(), 0);
        QVERIFY(!control<QCheckBox>(d, "locked")->isChecked());
        QVERIFY(control<QCheckBox>(d, "hidden")->isChecked());
        QVERIFY(control<QToolButton>(d, "borderBottom")->isChecked());
        QVERIFY(!control<QToolButton>(d, "borderTop")->isChecked());
        QCOMPARE(control<QListWidget>(d, "numberCategory")->currentRow(), int(jp::crunch::NumberCategory::Number));
        QCOMPARE(control<QSpinBox>(d, "decimals")->value(), 2);
        QVERIFY(control<QCheckBox>(d, "thousands")->isChecked());
    }

    // The Number tab: a category and its options make the code and the sample (in the person's language); the list of negative numbers shows each style as it writes -1234.1.
    void theNumberTabMakesTheCodeAndTheSample()
    {
        FormatCellsDialog d(inputFor());
        QCOMPARE(d.code(), QStringLiteral("General"));
        QCOMPARE(control<QLabel>(d, "numberSample")->text(), QStringLiteral("1234.5"));
        auto *categories = control<QListWidget>(d, "numberCategory");
        categories->setCurrentRow(int(jp::crunch::NumberCategory::Number));
        QCOMPARE(d.code(), QStringLiteral("0.00"));
        QCOMPARE(control<QLabel>(d, "numberSample")->text(), QStringLiteral("1234.50"));
        control<QCheckBox>(d, "thousands")->setChecked(true);
        QCOMPARE(d.code(), QStringLiteral("#,##0.00"));
        QCOMPARE(control<QLabel>(d, "numberSample")->text(), QStringLiteral("1,234.50"));
        control<QSpinBox>(d, "decimals")->setValue(0);
        QCOMPARE(d.code(), QStringLiteral("#,##0"));
        QCOMPARE(control<QLabel>(d, "numberSample")->text(), QStringLiteral("1,235"));
        control<QSpinBox>(d, "decimals")->setValue(1);
        auto *negatives = control<QListWidget>(d, "negatives");
        QCOMPARE(negatives->count(), 4);
        QCOMPARE(negatives->item(0)->text(), QStringLiteral("-1,234.1"));
        QCOMPARE(negatives->item(1)->text(), QStringLiteral("-1,234.1"));
        QCOMPARE(negatives->item(2)->text(), QStringLiteral("(1,234.1)"));
        negatives->setCurrentRow(3);
        QVERIFY(d.code().contains(QStringLiteral("[Red]\\(")));
        // currency begins with the language's symbol
        categories->setCurrentRow(int(jp::crunch::NumberCategory::Currency));
        QCOMPARE(control<QComboBox>(d, "symbol")->currentData().toString(), QStringLiteral("$"));
        QVERIFY(d.code().startsWith(QStringLiteral("\"$\"#,##0.0")));
        negatives->setCurrentRow(0);
        QCOMPARE(d.code(), QStringLiteral("\"$\"#,##0.0"));
        QCOMPARE(control<QLabel>(d, "numberSample")->text(), QStringLiteral("$1,234.5"));
        // percentage, scientific, text, and a custom code
        categories->setCurrentRow(int(jp::crunch::NumberCategory::Percentage));
        control<QSpinBox>(d, "decimals")->setValue(2);
        QCOMPARE(d.code(), QStringLiteral("0.00%"));
        QCOMPARE(control<QLabel>(d, "numberSample")->text(), QStringLiteral("123450.00%"));
        categories->setCurrentRow(int(jp::crunch::NumberCategory::Scientific));
        QCOMPARE(d.code(), QStringLiteral("0.00E+00"));
        QCOMPARE(control<QLabel>(d, "numberSample")->text(), QStringLiteral("1.23E+03"));
        categories->setCurrentRow(int(jp::crunch::NumberCategory::Custom));
        auto *custom = control<QLineEdit>(d, "customCode");
        custom->setText(QStringLiteral("0.0\" kg\""));
        custom->textEdited(custom->text());
        QCOMPARE(d.code(), QStringLiteral("0.0\" kg\""));
        QCOMPARE(control<QLabel>(d, "numberSample")->text(), QStringLiteral("1234.5 kg"));
        // what the dialog gives back: the code, once
        QCOMPARE(d.result().change.numberFormat.value_or(QString()), QStringLiteral("0.0\" kg\""));
    }

    // A code the file has is shown in its category with its options; one the dialog does not make is shown as a custom code; a date or time is picked from the list of its category.
    void aCodeIsShownWhereItBelongs()
    {
        struct Case {
            const char *code;
            jp::crunch::NumberCategory category;
        };
        const Case cases[] = {{"0.00E+00", jp::crunch::NumberCategory::Scientific}, {"0%", jp::crunch::NumberCategory::Percentage}, {"@", jp::crunch::NumberCategory::Text},
                              {"h:mm:ss", jp::crunch::NumberCategory::Time},         {"# ?/?", jp::crunch::NumberCategory::Fraction}, {"00000", jp::crunch::NumberCategory::Special},
                              {"0.00;[Blue]0.00", jp::crunch::NumberCategory::Custom}, {"\"Total: \"#,##0.00", jp::crunch::NumberCategory::Custom}};
        for (const Case &c : cases) {
            FormatCellsDialog::Input in = inputFor();
            in.view.numberFormat = QString::fromLatin1(c.code);
            FormatCellsDialog d(in);
            QVERIFY2(control<QListWidget>(d, "numberCategory")->currentRow() == int(c.category), c.code);
            QCOMPARE(d.code(), QString::fromLatin1(c.code));
            QVERIFY2(d.result().isEmpty(), c.code);
        }
        FormatCellsDialog::Input in = inputFor();
        in.view.numberFormat = QStringLiteral("d-mmm-yy");
        in.sampleNumber = 45000;
        FormatCellsDialog d(in);
        auto *types = control<QListWidget>(d, "types");
        QCOMPARE(control<QListWidget>(d, "numberCategory")->currentRow(), int(jp::crunch::NumberCategory::Date));
        QCOMPARE(types->currentItem()->data(Qt::UserRole).toString(), QStringLiteral("d-mmm-yy"));
        QCOMPARE(control<QLabel>(d, "numberSample")->text(), QStringLiteral("15-Mar-23"));
        // another date is picked
        for (int i = 0; i < types->count(); ++i)
            if (types->item(i)->data(Qt::UserRole).toString() == QStringLiteral("yyyy-mm-dd")) types->setCurrentRow(i);
        QCOMPARE(d.code(), QStringLiteral("yyyy-mm-dd"));
        QCOMPARE(control<QLabel>(d, "numberSample")->text(), QStringLiteral("2023-03-15"));
        QCOMPARE(d.result().change.numberFormat.value_or(QString()), QStringLiteral("yyyy-mm-dd"));
    }

    // In German the sample has the language's marks, the short date is the language's, and the symbol of a currency is the euro, after the number.
    void theSampleIsInThePersonsLanguage()
    {
        FormatCellsDialog::Input in = inputFor(QLocale(QLocale::German, QLocale::Germany));
        in.view.numberFormat = QStringLiteral("#,##0.00");
        FormatCellsDialog d(in);
        QCOMPARE(control<QLabel>(d, "numberSample")->text(), QStringLiteral("1.234,50"));
        control<QListWidget>(d, "numberCategory")->setCurrentRow(int(jp::crunch::NumberCategory::Currency));
        QCOMPARE(control<QComboBox>(d, "symbol")->currentData().toString(), QString(QChar(0x20AC)));
        QVERIFY2(d.code().endsWith(QStringLiteral("\"€\"")), qPrintable(d.code()));
        const QString sample = control<QLabel>(d, "numberSample")->text();
        QVERIFY2(sample.startsWith(QStringLiteral("1.234,50")) && sample.endsWith(QChar(0x20AC)), qPrintable(sample));
        control<QListWidget>(d, "numberCategory")->setCurrentRow(int(jp::crunch::NumberCategory::Date));
        auto *types = control<QListWidget>(d, "types");
        QCOMPARE(types->item(0)->data(Qt::UserRole).toString(), QStringLiteral("dd\\.mm\\.yyyy"));
    }

    void theAlignmentTabGivesTheChanges()
    {
        FormatCellsDialog d(inputFor());
        control<QComboBox>(d, "horizontal")->setCurrentIndex(1);   // left (indent)
        control<QSpinBox>(d, "indent")->setValue(3);
        control<QComboBox>(d, "vertical")->setCurrentIndex(1);     // center
        control<QCheckBox>(d, "wrap")->setChecked(true);
        control<QSpinBox>(d, "degrees")->setValue(-45);
        control<QComboBox>(d, "direction")->setCurrentIndex(2);
        control<QCheckBox>(d, "merge")->setChecked(true);
        const FormatCellsDialog::Result r = d.result();
        QVERIFY(r.change.horizontal && *r.change.horizontal == HAlign::Left);
        QCOMPARE(r.change.indent.value_or(-1), 3);
        QVERIFY(r.change.vertical && *r.change.vertical == VAlign::Center);
        QCOMPARE(r.change.wrap.value_or(false), true);
        QVERIFY(!r.change.shrink);
        QCOMPARE(r.change.rotation.value_or(-1), 135);   // 45 degrees clockwise is 135 in the file
        QCOMPARE(r.change.readingOrder.value_or(-1), 2);
        QVERIFY(r.merge && *r.merge);
        // stacked text is the file's 255, and turns the degrees off
        control<QCheckBox>(d, "stacked")->setChecked(true);
        QCOMPARE(d.result().change.rotation.value_or(-1), 255);
        QVERIFY(!control<QSpinBox>(d, "degrees")->isEnabled());
        // wrap and shrink exclude each other
        control<QCheckBox>(d, "shrink")->setChecked(true);
        QVERIFY(!control<QCheckBox>(d, "wrap")->isChecked());
    }

    void theFontTabGivesTheChanges()
    {
        FormatCellsDialog::Input in = inputFor();
        in.view.fontName = QStringLiteral("Calibri");
        FormatCellsDialog d(in);
        control<QComboBox>(d, "fontName")->setCurrentText(QStringLiteral("Georgia"));
        control<QComboBox>(d, "fontStyle")->setCurrentIndex(3);   // bold italic
        control<QComboBox>(d, "fontSize")->setCurrentText(QStringLiteral("14.5"));
        control<QComboBox>(d, "underline")->setCurrentIndex(2);   // double
        control<QCheckBox>(d, "strike")->setChecked(true);
        control<QCheckBox>(d, "superscript")->setChecked(true);
        const FormatCellsDialog::Result r = d.result();
        QCOMPARE(r.change.fontName.value_or(QString()), QStringLiteral("Georgia"));
        QCOMPARE(r.change.fontSize.value_or(0), 14.5);
        QVERIFY(r.change.bold.value_or(false) && r.change.italic.value_or(false));
        QVERIFY(r.change.underline && *r.change.underline == Underline::Double);
        QVERIFY(r.change.strike.value_or(false));
        QCOMPARE(int(r.change.vertAlign.value_or(9)), 1);
        QVERIFY(!r.change.fontColor);   // (not touched)
        // a size past what a font can be is not taken
        control<QComboBox>(d, "fontSize")->setCurrentText(QStringLiteral("5000"));
        QVERIFY(!d.result().change.fontSize);
    }

    // The border tab: a button puts the line of the style and color chosen on its edge (and takes it away on a second click); the presets put lines on edges together; the lines that are
    // the same are one operation.
    void theBorderTabGivesTheLines()
    {
        FormatCellsDialog::Input in = inputFor();
        in.oneCell = false;
        FormatCellsDialog d(in);
        auto *styles = control<QListWidget>(d, "lineStyle");
        QCOMPARE(styles->currentRow(), 6);   // thin
        control<QToolButton>(d, "borderTop")->click();
        control<QToolButton>(d, "borderLeft")->click();
        FormatCellsDialog::Result r = d.result();
        QCOMPARE(r.borders.size(), 1);
        QCOMPARE(r.borders[0].edges, int(E::EdgeTop | E::EdgeLeft));
        QVERIFY(r.borders[0].line.style == BorderStyle::Thin);
        // another style on another edge is its own operation
        styles->setCurrentRow(12);   // double
        control<QToolButton>(d, "borderBottom")->click();
        r = d.result();
        QCOMPARE(r.borders.size(), 2);
        QVERIFY(r.borders[1].line.style == BorderStyle::Double && r.borders[1].edges == E::EdgeBottom);
        // a second click on the same line takes it away
        control<QToolButton>(d, "borderBottom")->click();
        r = d.result();
        bool bottomNone = false;
        for (const auto &op : r.borders)
            if ((op.edges & E::EdgeBottom) && op.line.style == BorderStyle::None) bottomNone = true;
        QVERIFY(bottomNone);
        // presets
        control<QPushButton>(d, "presetOutline")->click();
        r = d.result();
        int outside = 0;
        for (const auto &op : r.borders)
            if (op.line.style == BorderStyle::Double) outside |= op.edges;
        QCOMPARE(outside & (E::EdgeLeft | E::EdgeRight | E::EdgeTop | E::EdgeBottom), int(E::EdgeLeft | E::EdgeRight | E::EdgeTop | E::EdgeBottom));
        control<QPushButton>(d, "presetInside")->click();
        QVERIFY(d.result().borders.last().edges & E::EdgeInsideHorizontal);
        control<QPushButton>(d, "presetNone")->click();
        for (const auto &op : d.result().borders) QVERIFY(op.line.style == BorderStyle::None);
        // for one cell the lines between cells are not offered
        FormatCellsDialog one(inputFor());
        QVERIFY(!control<QToolButton>(one, "borderInsideH")->isEnabled());
        QVERIFY(!control<QPushButton>(one, "presetInside")->isEnabled());
    }

    void theFillAndProtectionTabsGiveTheChanges()
    {
        FormatCellsDialog d(inputFor());
        QVERIFY(!d.result().change.fill);
        control<ColorButton>(d, "fillColor")->colorPicked(ColorRef::rgb(QColor(0x12, 0x34, 0x56)));
        FormatCellsDialog::Result r = d.result();
        QVERIFY(r.change.fill);
        QVERIFY(r.change.fill->pattern == PatternKind::Solid);
        QCOMPARE(r.change.fill->fg.rgb, 0x123456u);
        control<QComboBox>(d, "patternStyle")->setCurrentIndex(5);   // a pattern of the pattern color over the background
        control<ColorButton>(d, "patternColor")->colorPicked(ColorRef::rgb(QColor(0xFF, 0, 0)));
        r = d.result();
        QVERIFY(r.change.fill->pattern == PatternKind::DarkHorizontal);
        QCOMPARE(r.change.fill->fg.rgb, 0xFF0000u);
        QCOMPARE(r.change.fill->bg.rgb, 0x123456u);
        // no color takes the fill away
        control<QComboBox>(d, "patternStyle")->setCurrentIndex(0);
        QVERIFY(d.result().change.fill->pattern == PatternKind::None);
        control<QCheckBox>(d, "locked")->setChecked(false);
        control<QCheckBox>(d, "hidden")->setChecked(true);
        r = d.result();
        QCOMPARE(r.change.locked.value_or(true), false);
        QCOMPARE(r.change.formulaHidden.value_or(false), true);
    }

    // The dialog drawn, a tab at a time (JC_SCREENSHOT_FORMAT names a folder to keep the pictures in): each tab has something on it.
    void everyTabIsDrawn()
    {
        FormatCellsDialog::Input in = inputFor();
        in.view.numberFormat = QStringLiteral("\"$\"#,##0.00");
        in.view.bold = true;
        in.view.fontName = QStringLiteral("Calibri");
        in.view.pattern = PatternKind::Solid;
        in.view.fillColor = QColor(0xFF, 0xEB, 0x9C);
        in.view.border.bottom.style = BorderStyle::Double;
        in.view.border.bottom.color = ColorSpec::fromRgb(0x1F4E79);
        in.view.border.top.style = BorderStyle::Thin;
        in.view.border.top.color = ColorSpec::fromRgb(0x1F4E79);
        in.view.horizontal = HAlign::Center;
        in.oneCell = false;
        FormatCellsDialog d(in);
        d.resize(560, 520);
        d.show();
        const QString dir = qEnvironmentVariable("JC_SCREENSHOT_FORMAT");
        const char *names[] = {"number", "alignment", "font", "border", "fill", "protection"};
        for (int tab = 0; tab < 6; ++tab) {
            d.showTab(tab);
            QApplication::processEvents();
            const QImage shot = d.grab().toImage();
            QVERIFY(!shot.isNull() && shot.width() > 300 && shot.height() > 300);
            // (a tab that drew nothing would be one color all over)
            QSet<QRgb> colors;
            for (int y = 0; y < shot.height(); y += 3)
                for (int x = 0; x < shot.width(); x += 3) colors.insert(shot.pixel(x, y));
            QVERIFY2(colors.size() > 6, names[tab]);
            if (!dir.isEmpty()) QVERIFY(shot.save(QStringLiteral("%1/format-%2.png").arg(dir, QLatin1String(names[tab]))));
        }
    }

    void aTabCanBeOpenedFirst()
    {
        FormatCellsDialog d(inputFor());
        auto *tabs = d.findChild<QTabWidget *>(QStringLiteral("formatTabs"));
        QVERIFY(tabs);
        QCOMPARE(tabs->count(), 6);
        d.showTab(FormatCellsDialog::BorderTab);
        QCOMPARE(tabs->currentIndex(), 3);
        QCOMPARE(tabs->tabText(0), QStringLiteral("Number"));
        QCOMPARE(tabs->tabText(5), QStringLiteral("Protection"));
    }
};

JP_TEST_CLASS(CrunchFormatDialogTests)
#include "tst_crunch_formatdialog.moc"
