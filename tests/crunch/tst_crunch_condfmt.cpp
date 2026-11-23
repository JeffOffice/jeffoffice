// Tests of the conditional formats JeffCrunch makes from the Home tab and changes in Manage Rules: every kind of rule is made, drawn, saved through the window's own save, and read back.
#include "app/crunchwindow.h"
#include "savedok.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "app/condpresets.h"
#include "app/theme.h"
#include "app/condrulesdialog.h"
#include "grid/condformat.h"
#include "grid/gridview.h"
#include "core/printjob.h"
#include "io/zip.h"
#include "render/pdfpage.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QMenu>
#include <QSignalSpy>
#include <QUndoStack>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;
using namespace jp::office::sheet;

namespace {

//        A      B        C
//   1    1      apple    #N/A
//   2    2      banana   (empty)
//   3    3      cherry   45000
//   4    4      apple    (empty)
//   5    5      (empty)  2
QByteArray condBook()
{
    const QString lists = QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/><scheme val=\"minor\"/></font></fonts>"
                                         "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                                         "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                                         "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>");
    const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"), lists);
    const char *fruit[] = {"apple", "banana", "cherry", "apple", nullptr};
    QString rows;
    for (int r = 1; r <= 5; ++r) {
        rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c>").arg(r);
        if (fruit[r - 1]) rows += QStringLiteral("<c r=\"B%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(r).arg(QLatin1String(fruit[r - 1]));
        if (r == 1) rows += QStringLiteral("<c r=\"C1\" t=\"e\"><v>#N/A</v></c>");
        if (r == 3) rows += QStringLiteral("<c r=\"C3\"><v>45000</v></c>");
        if (r == 5) rows += QStringLiteral("<c r=\"C5\"><v>2</v></c>");
        rows += QStringLiteral("</row>");
    }
    return buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), styles);
}

CondRule rule(CondRule::Type type, const char *range = "A1:C5")
{
    CondRule r;
    r.type = type;
    r.ranges.append(rangeOf(QString::fromLatin1(range)));
    return r;
}

CondValue val(CondValue::Type t, const char *v = "")
{
    CondValue c;
    c.type = t;
    c.value = QString::fromLatin1(v);
    return c;
}

// One rule of each kind, as the menus make them.
QVector<QPair<QString, CondRule>> everyKind()
{
    using T = CondRule::Type;
    QVector<QPair<QString, CondRule>> list;
    auto add = [&](const char *name, const CondRule &r) { list.append({QString::fromLatin1(name), r}); };
    CondRule r = rule(T::CellIs, "A1:A5");
    r.op = CondRule::Operator::GreaterThan;
    r.formula1 = QStringLiteral("3");
    add("cellIs", r);
    r = rule(T::Expression, "A1:A5");
    r.formula1 = QStringLiteral("MOD(A1,2)=0");
    add("expression", r);
    r = rule(T::ColorScale, "A1:A5");
    r.values = {val(CondValue::Type::Min), val(CondValue::Type::Percentile, "50"), val(CondValue::Type::Max)};
    r.colors = {ColorSpec::fromRgb(0xF8696B), ColorSpec::fromRgb(0xFFEB84), ColorSpec::fromRgb(0x63BE7B)};
    add("colorScale", r);
    r = rule(T::DataBar, "A1:A5");
    r.values = {val(CondValue::Type::Min), val(CondValue::Type::Max)};
    r.colors = {ColorSpec::fromRgb(0x638EC6)};
    r.gradient = false;
    add("dataBar", r);
    r = rule(T::IconSet, "A1:A5");
    r.iconSet = QStringLiteral("3Arrows");
    r.values = {val(CondValue::Type::Percent, "0"), val(CondValue::Type::Percent, "33"), val(CondValue::Type::Percent, "67")};
    add("iconSet", r);
    r = rule(T::Top10, "A1:A5");
    r.rank = 2;
    add("top10", r);
    add("uniqueValues", rule(T::UniqueValues, "B1:B5"));
    add("duplicateValues", rule(T::DuplicateValues, "B1:B5"));
    r = rule(T::ContainsText, "B1:B5");
    r.text = QStringLiteral("an");
    r.formula1 = QStringLiteral("NOT(ISERROR(SEARCH(\"an\",B1)))");
    add("containsText", r);
    r = rule(T::NotContainsText, "B1:B5");
    r.text = QStringLiteral("an");
    add("notContainsText", r);
    r = rule(T::BeginsWith, "B1:B5");
    r.text = QStringLiteral("ap");
    add("beginsWith", r);
    r = rule(T::EndsWith, "B1:B5");
    r.text = QStringLiteral("rry");
    add("endsWith", r);
    add("containsBlanks", rule(T::ContainsBlanks, "B1:B5"));
    add("notContainsBlanks", rule(T::NotContainsBlanks, "B1:B5"));
    add("containsErrors", rule(T::ContainsErrors, "C1:C5"));
    add("notContainsErrors", rule(T::NotContainsErrors, "C1:C5"));
    r = rule(T::TimePeriod, "C1:C5");
    r.timePeriod = QStringLiteral("today");
    add("timePeriod", r);
    r = rule(T::AboveAverage, "A1:A5");
    r.above = true;
    add("aboveAverage", r);
    return list;
}

DifferentialFormat redFill()
{
    DifferentialFormat d;
    d.hasFill = true;
    d.fill.pattern = PatternKind::Solid;
    d.fill.bg = ColorSpec::fromRgb(0xFFC7CE);
    return d;
}

void selectRange(EditWindow &e, const QString &range)
{
    const CellRange r = rangeOf(range);
    e.view()->selectRange(r.row1, r.col1, r.row2, r.col2);
}

void run(EditWindow &e, const char *action)
{
    QAction *a = e.win.w.act(QString::fromLatin1(action));
    QVERIFY2(a, action);
    QVERIFY2(a->isEnabled(), action);
    a->trigger();
}

QString shape(const grid::CondCell &c)
{
    return QStringLiteral("%1%2%3%4").arg(c.any ? "any" : "-").arg(c.format >= 0 ? " fmt" : "").arg(c.hasScale ? " scale" : "").arg(c.hasBar ? " bar" : "") + (c.icon >= 0 ? QStringLiteral(" icon") : QString());
}

} // namespace

class CrunchCondFormatTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // One line per kind: made through the session, drawn, saved by the window's own save, opened again, and drawn again.
    void everyKindOfRuleIsMadeDrawnSavedAndReadBack()
    {
        for (const auto &entry : everyKind()) {
            EditWindow e;
            QVERIFY(e.open(condBook()));
            const bool formatted = entry.second.type != CondRule::Type::ColorScale && entry.second.type != CondRule::Type::DataBar && entry.second.type != CondRule::Type::IconSet;
            const auto made = e.win.w.session()->addCondRule(0, entry.second, formatted ? redFill() : DifferentialFormat());
            QVERIFY2(made.ok, qPrintable(entry.first + ": " + made.problem));
            e.view()->sheet().conditional().setToday(45000);
            e.view()->sheet().conditional().settle();
            QString drawn;
            for (quint32 r = 0; r < 5; ++r) drawn += shape(e.view()->sheet().conditional().at(r, entry.second.ranges[0].col1)) + "|";
            QVERIFY(e.settle());
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            QVERIFY(e.win.w.save());
            QVERIFY2(savedOk(saved), qPrintable(e.win.told.join('|')));
            Window second;
            QVERIFY(second.open(e.copyOfFile()));
            const QVector<CondRule> back = second.w.session()->condRules(0);
            second.w.view()->sheet().conditional().setToday(45000);
            second.w.view()->sheet().conditional().settle();
            QString again;
            for (quint32 r = 0; r < 5; ++r) again += shape(second.w.view()->sheet().conditional().at(r, entry.second.ranges[0].col1)) + "|";
            QVERIFY2(back.size() == 1 && back[0].type == entry.second.type, qPrintable(entry.first));
            QVERIFY2(drawn.contains(QLatin1String("any")), qPrintable(entry.first + " " + drawn));
            QCOMPARE(again, drawn);
        }
    }

    // The menu's galleries: every data bar, color scale, and icon set puts one rule of the right look on the selected cells, as one step; it is drawn at once, and is the same after the window's own
    // save and an open.
    void theGalleriesMakeBarsScalesAndIcons()
    {
        EditWindow e;
        QVERIFY(e.open(condBook()));
        selectRange(e, QStringLiteral("A1:A5"));
        auto &cond = e.view()->sheet().conditional();
        int steps = 0;
        // data bars: six colors, gradient and solid
        for (int i = 0; i < int(condpresets::barColors().size()); ++i)
            for (const bool gradient : {true, false}) {
                run(e, QStringLiteral("cond.bar.%1.%2").arg(i).arg(gradient ? "gradient" : "solid").toLatin1().constData());
                QCOMPARE(e.win.w.undoStack()->count(), ++steps);
                const CondRule r = e.win.w.session()->condRules(0).first();
                QVERIFY(r.type == CondRule::Type::DataBar);
                QCOMPARE(r.gradient, gradient);
                QCOMPARE(r.border, !gradient);
                QCOMPARE(r.colors.first().rgb & 0xFFFFFF, condpresets::barColors()[i].rgb);
            }
        cond.settle();
        QVERIFY(cond.at(4, 0).hasBar && cond.at(0, 0).hasBar);
        QVERIFY(cond.at(4, 0).bar.to > cond.at(0, 0).bar.to);
        // color scales: two and three colors
        for (int i = 0; i < int(condpresets::scalePresets().size()); ++i) {
            run(e, QStringLiteral("cond.scale.%1").arg(i).toLatin1().constData());
            QCOMPARE(e.win.w.undoStack()->count(), ++steps);
            const CondRule r = e.win.w.session()->condRules(0).first();
            QVERIFY(r.type == CondRule::Type::ColorScale);
            QCOMPARE(int(r.colors.size()), int(condpresets::scalePresets()[i].colors.size()));
            QCOMPARE(int(r.values.size()), int(r.colors.size()));
        }
        // icon sets: every one the painter draws, with an icon for each step
        int sets = 0;
        for (const auto &group : condpresets::iconGroups())
            for (const QString &set : group.sets) {
                ++sets;
                run(e, QStringLiteral("cond.icons.%1").arg(set).toLatin1().constData());
                QCOMPARE(e.win.w.undoStack()->count(), ++steps);
                const CondRule r = e.win.w.session()->condRules(0).first();
                QCOMPARE(r.iconSet, set);
                QCOMPARE(int(r.values.size()), jp::grid::iconSetCount(set));
                cond.settle();
                QCOMPARE(cond.at(0, 0).icon, 0);
                QCOMPARE(cond.at(4, 0).icon, jp::grid::iconSetCount(set) - 1);
            }
        QCOMPARE(sets, int(jp::grid::iconSetNames().size()));
        // one Undo takes one gallery choice off
        const int rules = int(e.win.w.session()->condRules(0).size());
        pressUndo(&e.win.w);
        QCOMPARE(int(e.win.w.session()->condRules(0).size()), rules - 1);
        e.win.w.undoStack()->redo();
        // saved by the window and opened again: the same rules, each with its look
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join('|')));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const QVector<CondRule> before = e.win.w.session()->condRules(0), after = second.w.session()->condRules(0);
        QCOMPARE(int(after.size()), int(before.size()));
        for (int i = 0; i < before.size(); ++i) {
            QVERIFY(after[i].type == before[i].type);
            QCOMPARE(after[i].gradient, before[i].gradient);
            QCOMPARE(after[i].border, before[i].border);
            QCOMPARE(after[i].iconSet, before[i].iconSet);
            QCOMPARE(int(after[i].colors.size()), int(before[i].colors.size()));
            if (!before[i].colors.isEmpty()) QCOMPARE(after[i].colors[0].rgb, before[i].colors[0].rgb);
        }
    }

    // A rule on a whole column is one range, not a cell for each row; the figures a bar needs are worked out from the cells that have values, and a paint asks for the cells it shows.
    void aRuleOnAWholeColumnCostsWhatTheColumnHolds()
    {
        EditWindow e;
        QVERIFY(e.open(condBook()));
        const qint64 cellsBefore = e.view()->sheet().data().sheet->cells.cellCount();
        e.view()->selectRange(0, 0, 1048575, 0);
        run(e, "cond.bar.0.solid");
        const CondRule r = e.win.w.session()->condRules(0).first();
        QCOMPARE(int(r.ranges.size()), 1);
        QCOMPARE(r.ranges[0].row2, quint32(1048575));
        QCOMPARE(e.view()->sheet().data().sheet->cells.cellCount(), cellsBefore);
        auto &cond = e.view()->sheet().conditional();
        cond.settle();
        QVERIFY(cond.at(4, 0).hasBar);
        QVERIFY(!cond.at(900000, 0).any || cond.at(900000, 0).hasBar);
        // five cells hold values: the figures are passed over for them, not for a million rows
        QVERIFY2(cond.figureCells() <= 20, qPrintable(QString::number(cond.figureCells())));
        QVERIFY2(cond.statCells() <= 5, qPrintable(QString::number(cond.statCells())));
        // a whole sheet is the same
        e.view()->selectRange(0, 0, 1048575, 16383);
        run(e, "cond.scale.0");
        QCOMPARE(e.view()->sheet().data().sheet->cells.cellCount(), cellsBefore);
        QCOMPARE(int(e.win.w.session()->condRules(0).first().ranges.size()), 1);
    }

    // A date occurring is a quick rule too, with the dates chosen from the list.
    void aDateOccurringRuleIsMadeFromTheMenu()
    {
        EditWindow e;
        QVERIFY(e.open(condBook()));
        selectRange(e, QStringLiteral("C1:C5"));
        e.win.w.setCondAsker([](CrunchWindow::CondAnswer *a) {
            a->value1 = QStringLiteral("thisMonth");
            a->preset = 1;
            return true;
        });
        run(e, "cond.date");
        const CondRule r = e.win.w.session()->condRules(0).first();
        QVERIFY(r.type == CondRule::Type::TimePeriod);
        QCOMPARE(r.timePeriod, QStringLiteral("thisMonth"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
    }

    // The window for one rule: it starts from a rule that exists, and what it gives back makes the rule again with the changes.
    void theRuleWindowEditsEveryKindOfGalleryRule()
    {
        // data bar
        {
            CondRule bar = condpresets::dataBarRule(rangeOf(QStringLiteral("A1:A5")), 2, false);
            CondSpec s = specOfRule(bar);
            QVERIFY(s.kind == CondSpec::Kind::DataBar);
            CondRuleDialog d(s, true);
            auto *min = d.findChild<QComboBox *>(QStringLiteral("barMin"));
            auto *minValue = d.findChild<QLineEdit *>(QStringLiteral("barMinValue"));
            auto *max = d.findChild<QComboBox *>(QStringLiteral("barMax"));
            auto *maxValue = d.findChild<QLineEdit *>(QStringLiteral("barMaxValue"));
            min->setCurrentIndex(min->findData(int(CondValue::Type::Number)));
            minValue->setText(QStringLiteral("0"));
            max->setCurrentIndex(max->findData(int(CondValue::Type::Percent)));
            maxValue->setText(QStringLiteral("90"));
            d.findChild<QComboBox *>(QStringLiteral("barFill"))->setCurrentIndex(0);
            d.findChild<QComboBox *>(QStringLiteral("barDirection"))->setCurrentIndex(1);
            d.findChild<QComboBox *>(QStringLiteral("barAxis"))->setCurrentIndex(1);
            d.findChild<QCheckBox *>(QStringLiteral("barOnly"))->setChecked(true);
            d.findChild<QCheckBox *>(QStringLiteral("barNegativeSame"))->setChecked(true);
            d.findChild<CondColorButton *>(QStringLiteral("barColor"))->setColor(ColorSpec::fromRgb(0x123456));
            const CondRule r = d.spec().visual;
            QVERIFY(r.type == CondRule::Type::DataBar);
            QVERIFY(r.values[0].type == CondValue::Type::Number && r.values[0].value == QLatin1String("0"));
            QVERIFY(r.values[1].type == CondValue::Type::Percent && r.values[1].value == QLatin1String("90"));
            QVERIFY(r.gradient && r.rightToLeft && !r.showValue && r.negativeSameColor && r.axis == CondRule::Axis::Middle);
            QCOMPARE(r.colors[0].rgb & 0xFFFFFF, quint32(0x123456));
        }
        // color scale: two colors, then three
        {
            CondRule scale = condpresets::colorScaleRule(rangeOf(QStringLiteral("A1:A5")), 0);
            CondRuleDialog d(specOfRule(scale), true);
            auto *three = d.findChild<QCheckBox *>(QStringLiteral("scaleThree"));
            QVERIFY(three->isChecked());
            QCOMPARE(int(d.spec().visual.colors.size()), 3);
            three->setChecked(false);
            d.findChild<QComboBox *>(QStringLiteral("scaleType2"))->setCurrentIndex(d.findChild<QComboBox *>(QStringLiteral("scaleType2"))->findData(int(CondValue::Type::Number)));
            d.findChild<QLineEdit *>(QStringLiteral("scaleValue2"))->setText(QStringLiteral("100"));
            const CondRule r = d.spec().visual;
            QCOMPARE(int(r.colors.size()), 2);
            QCOMPARE(int(r.values.size()), 2);
            QVERIFY(r.values[1].type == CondValue::Type::Number && r.values[1].value == QLatin1String("100"));
            QCOMPARE(r.colors[1].rgb & 0xFFFFFF, quint32(0x63BE7B));
        }
        // icon set: another set, thresholds, reverse, icon only
        {
            CondRule icons = condpresets::iconSetRule(rangeOf(QStringLiteral("A1:A5")), QStringLiteral("4Arrows"));
            CondRuleDialog d(specOfRule(icons), true);
            QCOMPARE(d.findChildren<QLineEdit *>(QRegularExpression(QStringLiteral("^iconValue"))).size(), 3);
            d.findChild<QLineEdit *>(QStringLiteral("iconValue2"))->setText(QStringLiteral("60"));
            d.findChild<QComboBox *>(QStringLiteral("iconGte3"))->setCurrentIndex(1);
            d.findChild<QCheckBox *>(QStringLiteral("iconReverse"))->setChecked(true);
            d.findChild<QCheckBox *>(QStringLiteral("iconOnly"))->setChecked(true);
            const CondRule r = d.spec().visual;
            QCOMPARE(r.iconSet, QStringLiteral("4Arrows"));
            QCOMPARE(int(r.values.size()), 4);
            QCOMPARE(r.values[2].value, QStringLiteral("60"));
            QVERIFY(!r.values[3].gte && r.reverse && !r.showValue);
            auto *set = d.findChild<QComboBox *>(QStringLiteral("iconSet"));
            set->setCurrentIndex(set->findData(QStringLiteral("5Rating")));
            QCOMPARE(int(d.spec().visual.values.size()), 5);
        }
    }

    // Manage Rules: edit one of each kind, change the order and the cells, delete one, and the whole is one step that Undo takes back; saved and opened, the rules are as left.
    void manageRulesEditsReordersAndDeletesAndIsSaved()
    {
        EditWindow e;
        QVERIFY(e.open(condBook()));
        selectRange(e, QStringLiteral("A1:A5"));
        run(e, "cond.bar.0.gradient");
        run(e, "cond.scale.0");
        run(e, "cond.icons.3Arrows");
        e.win.w.setCondAsker([](CrunchWindow::CondAnswer *a) {
            a->value1 = QStringLiteral("2");
            return true;
        });
        run(e, "cond.greater");
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join('|')));    // (these four are the file's own now)
        const int steps = e.win.w.undoStack()->count();
        QCOMPARE(int(e.win.w.session()->condRules(0).size()), 4);
        // one rule made here that the manager then changes, next to the file's
        run(e, "cond.bar.2.solid");
        const int stepsBefore = e.win.w.undoStack()->count();
        e.win.w.setCondManageAsker([&](const QVector<CondManageDialog::Item> &items, const CrunchWindow::CondEditor &editor, QVector<crunch::EditSession::CondManaged> *out) {
            CondManageDialog d(items, editor);
            d.select(3);                                    // the 3-color scale (a rule of the file)
            e.win.w.setCondSpecAsker([](CondSpec *spec, bool editing) {
                if (!editing || spec->kind != CondSpec::Kind::ColorScale) return false;
                spec->visual.colors = {ColorSpec::fromRgb(0x111111), ColorSpec::fromRgb(0x222222)};
                spec->visual.values.removeAt(1);
                return true;
            });
            d.editSelected();
            d.select(4);
            d.moveSelected(-1);
            d.select(0);
            d.deleteSelected();
            *out = d.result();
            return true;
        });
        run(e, "cond.manage");
        QCOMPARE(e.win.w.undoStack()->count(), stepsBefore + 1);
        const QVector<CondRule> now = e.win.w.session()->condRules(0);
        QCOMPARE(int(now.size()), 4);
        bool twoColors = false, solid = false;
        for (const CondRule &r : now) {
            twoColors = twoColors || (r.type == CondRule::Type::ColorScale && r.colors.size() == 2 && r.colors[0].rgb == 0x111111);
            solid = solid || (r.type == CondRule::Type::DataBar && !r.gradient);
        }
        QVERIFY(twoColors);
        QVERIFY(!solid);                                    // (the rule made here was the first in the list and is deleted)
        pressUndo(&e.win.w);
        QCOMPARE(int(e.win.w.session()->condRules(0).size()), 5);
        e.win.w.undoStack()->redo();
        QCOMPARE(e.win.w.undoStack()->count(), stepsBefore + 1);
        Q_UNUSED(steps);
        QVERIFY(e.settle());
        QSignalSpy savedAgain(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(savedAgain, 20000), qPrintable(e.win.told.join('|')));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const QVector<CondRule> back = second.w.session()->condRules(0);
        QCOMPARE(int(back.size()), 4);
        for (int i = 0; i < 4; ++i) {
            QVERIFY(back[i].type == now[i].type);
            QCOMPARE(int(back[i].colors.size()), int(now[i].colors.size()));
        }
    }

    // Check Compatibility names what an older file cannot hold of conditional formatting.
    void checkCompatibilityNamesBarsScalesAndIcons()
    {
        EditWindow e;
        QVERIFY(e.open(condBook()));
        selectRange(e, QStringLiteral("A1:A5"));
        run(e, "cond.bar.0.gradient");
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join('|')));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        second.w.checkCompatibility();
        QVERIFY(QTest::qWaitFor([&] { return !second.told.isEmpty(); }, 20000));
        QVERIFY2(second.told.last().contains(QLatin1String("data bars, color scales, and icon sets")), qPrintable(second.told.last()));
    }

    // Pictures for the docs, when JC_SHOT_DIR names a folder: the menu and its galleries, a sheet with bars, scales, and icons, and Manage Rules with the window of each kind of rule.
    void picturesOfTheMenuAndTheRules()
    {
        const QString dir = QString::fromLocal8Bit(qgetenv("JC_SHOT_DIR"));
        if (dir.isEmpty()) QSKIP("JC_SHOT_DIR names the folder for the pictures");
        jp::applyUiTheme(1);
        EditWindow e;
        QString rows;
        for (int r = 1; r <= 14; ++r)
            rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%2</v></c><c r=\"B%1\"><v>%3</v></c><c r=\"C%1\"><v>%4</v></c><c r=\"D%1\"><v>%5</v></c></row>")
                        .arg(r).arg(r * 7 % 23 - 5).arg(r * 11 % 19).arg(r * 13 % 17).arg(r * 5 % 29);
        const QString lists = QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/><scheme val=\"minor\"/></font></fonts>"
                                             "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                                             "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                                             "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>");
        QVERIFY(e.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"), lists))));
        e.win.w.resize(1000, 520);
        e.win.w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&e.win.w));
        selectRange(e, QStringLiteral("A1:A14"));
        run(e, "cond.bar.0.gradient");
        selectRange(e, QStringLiteral("B1:B14"));
        run(e, "cond.scale.0");
        selectRange(e, QStringLiteral("C1:C14"));
        run(e, "cond.icons.3Arrows");
        selectRange(e, QStringLiteral("D1:D14"));
        run(e, "cond.bar.1.solid");
        e.view()->sheet().conditional().settle();
        QTest::qWait(300);
        e.win.w.grab().save(dir + QStringLiteral("/sheet-bars-scales-icons.png"));
        // the menu and its galleries
        QMenu *top = nullptr;
        for (QMenu *m : e.win.w.findChildren<QMenu *>())
            if (m->actions().contains(e.win.w.act(QStringLiteral("cond.greater")))) top = m;
        QVERIFY(top);
        top->show();
        top->grab().save(dir + QStringLiteral("/menu.png"));
        const char *names[] = {"Data Bars", "Color Scales", "Icon Sets"};
        const char *files[] = {"gallery-data-bars.png", "gallery-color-scales.png", "gallery-icon-sets.png"};
        for (int i = 0; i < 3; ++i)
            for (QAction *a : top->actions())
                if (a->menu() && a->text() == QLatin1String(names[i])) {
                    a->menu()->show();
                    a->menu()->grab().save(dir + QLatin1Char('/') + QLatin1String(files[i]));
                }
        top->hide();
        // Manage Rules, and the window of each kind
        QVector<CondManageDialog::Item> items;
        for (const CondRule &r : e.win.w.session()->condRules(0)) {
            CondManageDialog::Item it;
            it.rule = r;
            it.look = e.win.w.session()->condLookOf(r);
            items.append(it);
        }
        CondManageDialog manage(items, nullptr);
        manage.show();
        manage.select(0);
        manage.grab().save(dir + QStringLiteral("/manage-rules.png"));
        const char *kinds[] = {"edit-data-bar.png", "edit-color-scale.png", "edit-icon-set.png"};
        const CondRule samples[] = {condpresets::dataBarRule(rangeOf(QStringLiteral("A1:A14")), 0, true), condpresets::colorScaleRule(rangeOf(QStringLiteral("A1:A14")), 0),
                                    condpresets::iconSetRule(rangeOf(QStringLiteral("A1:A14")), QStringLiteral("5Rating"))};
        for (int i = 0; i < 3; ++i) {
            CondRuleDialog d(specOfRule(samples[i]), true);
            d.show();
            d.grab().save(dir + QLatin1Char('/') + QLatin1String(kinds[i]));
        }
        CondRuleDialog fresh(CondSpec(), false);
        fresh.show();
        fresh.grab().save(dir + QStringLiteral("/new-rule.png"));
    }

    // What the menu makes is printed: a PDF of the sheet has the bar's color and the scale's colors.
    void barsAndScalesMadeFromTheMenuArePrinted()
    {
        EditWindow e;
        QVERIFY(e.open(condBook()));
        selectRange(e, QStringLiteral("C1:C5"));
        run(e, "cond.bar.2.solid");                                   // red bars (FF555A)
        selectRange(e, QStringLiteral("A1:A5"));
        run(e, "cond.scale.0");
        const QString path = e.folder.dir.filePath(QStringLiteral("rules.pdf"));
        crunch::PrintJobOptions options;
        QString why;
        QVERIFY2(e.win.w.exportPdfTo(path, options, QString(), &why), qPrintable(why));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const jp::PdfDocument pdf(f.readAll());
        QVERIFY(pdf.isValid());
        const QImage page = pdf.render(0, QSize(612, 792)).convertToFormat(QImage::Format_RGB32);
        auto near = [&](quint32 rgb) {
            int n = 0;
            for (int y = 0; y < page.height(); ++y)
                for (int x = 0; x < page.width(); ++x) {
                    const QRgb p = page.pixel(x, y);
                    n += qAbs(qRed(p) - int((rgb >> 16) & 255)) < 12 && qAbs(qGreen(p) - int((rgb >> 8) & 255)) < 12 && qAbs(qBlue(p) - int(rgb & 255)) < 12;
                }
            return n;
        };
        QVERIFY2(near(0xFF555A) > 20, "no bar on the printed page");
        QVERIFY2(near(0x63BE7B) > 20, "no scale color on the printed page");
    }
};

JP_TEST_CLASS(CrunchCondFormatTests)
#include "tst_crunch_condfmt.moc"
