// Tests of the conditional formats JeffCrunch makes from the Home tab and changes in Manage Rules: every kind of rule is made, drawn, saved through the window's own save, and read back.
#include "app/crunchwindow.h"
#include "savedok.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "app/condpresets.h"
#include "app/theme.h"
#include "app/condlook.h"
#include "app/condrulesdialog.h"
#include "app/formatdialog.h"
#include "grid/condformat.h"
#include "grid/gridview.h"
#include "core/printjob.h"
#include "io/zip.h"
#include "render/pdfpage.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QLineEdit>
#include <QListWidget>
#include <QTabWidget>
#include <QMenu>
#include <QPushButton>
#include <QRegularExpression>
#include <QStackedWidget>
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
QByteArray condBook(const QString &after = QString(), const QString &dxfs = QString(), const QString &numFmts = QString())
{
    const QString lists = QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/><scheme val=\"minor\"/></font></fonts>"
                                         "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                                         "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                                         "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>") + dxfs;
    const QByteArray styles = stylesXml(numFmts, QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"), lists);
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
    return buildXlsx({worksheet(rows, after)}, QByteArray(), QString(), QString(), styles);
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

// ---- a file with nine rules, for the tests of Manage Rules: seven are in the main list (one has pivot="1", the data bar has a 2010 entry that says its negative border is not the positive one's), and
// two live only in the 2010 block (a custom icon set each, at priorities 1 and 6, which Manage Rules does not list).
const char *kGt = "<cfRule type=\"cellIs\" priority=\"2\" operator=\"greaterThan\"><formula>3</formula></cfRule>";
const char *kEven = "<cfRule type=\"expression\" priority=\"3\"><formula>MOD(A1,2)=0</formula></cfRule>";
const char *kTop = "<cfRule type=\"top10\" priority=\"4\" pivot=\"1\" rank=\"2\"/>";
const char *kBar = "<cfRule type=\"dataBar\" priority=\"5\"><dataBar><cfvo type=\"min\"/><cfvo type=\"max\"/><color rgb=\"FF638EC6\"/></dataBar><extLst><ext xmlns:x14=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\" "
                   "uri=\"{B025F937-C7B1-47D3-B67F-A62EFF666E3E}\"><x14:id>{11111111-1111-4111-8111-111111111111}</x14:id></ext></extLst></cfRule>";
const char *kScale = "<cfRule type=\"colorScale\" priority=\"7\"><colorScale><cfvo type=\"min\"/><cfvo type=\"max\"/><color rgb=\"FFF8696B\"/><color rgb=\"FF63BE7B\"/></colorScale></cfRule>";
const char *kIcons = "<cfRule type=\"iconSet\" priority=\"8\"><iconSet iconSet=\"3Arrows\"><cfvo type=\"percent\" val=\"0\"/><cfvo type=\"percent\" val=\"33\"/><cfvo type=\"percent\" val=\"67\"/></iconSet></cfRule>";
const char *kBlank = "<cfRule type=\"containsBlanks\" priority=\"9\"><formula>LEN(TRIM(B1))=0</formula></cfRule>";
const char *kBarX14 = "<x14:cfRule type=\"dataBar\" id=\"{11111111-1111-4111-8111-111111111111}\"><x14:dataBar minLength=\"0\" maxLength=\"100\" gradient=\"0\" border=\"1\" negativeBarBorderColorSameAsPositive=\"0\">"
                      "<x14:cfvo type=\"autoMin\"/><x14:cfvo type=\"autoMax\"/><x14:borderColor rgb=\"FF638EC6\"/><x14:negativeFillColor rgb=\"FFFF0000\"/><x14:negativeBorderColor rgb=\"FF00FF00\"/>"
                      "<x14:axisColor rgb=\"FF000000\"/></x14:dataBar></x14:cfRule>";
const char *kCustom1 = "<x14:cfRule type=\"iconSet\" priority=\"1\" id=\"{22222222-2222-4222-8222-222222222222}\"><x14:iconSet iconSet=\"3Triangles\" custom=\"1\"><x14:cfvo type=\"percent\"><xm:f>0</xm:f></x14:cfvo>"
                       "<x14:cfvo type=\"percent\"><xm:f>33</xm:f></x14:cfvo><x14:cfvo type=\"percent\"><xm:f>67</xm:f></x14:cfvo><x14:cfIcon iconSet=\"3Triangles\" iconId=\"0\"/><x14:cfIcon iconSet=\"3Triangles\" iconId=\"1\"/>"
                       "<x14:cfIcon iconSet=\"3Triangles\" iconId=\"2\"/></x14:iconSet></x14:cfRule>";
const char *kCustom6 = "<x14:cfRule type=\"iconSet\" priority=\"6\" id=\"{33333333-3333-4333-8333-333333333333}\"><x14:iconSet iconSet=\"3Arrows\" custom=\"1\"><x14:cfvo type=\"percent\"><xm:f>0</xm:f></x14:cfvo>"
                       "<x14:cfvo type=\"percent\"><xm:f>33</xm:f></x14:cfvo><x14:cfvo type=\"percent\"><xm:f>67</xm:f></x14:cfvo><x14:cfIcon iconSet=\"3Arrows\" iconId=\"2\"/><x14:cfIcon iconSet=\"3Arrows\" iconId=\"1\"/>"
                       "<x14:cfIcon iconSet=\"3Arrows\" iconId=\"0\"/></x14:iconSet></x14:cfRule>";

QString nineRulesTail()
{
    const QString x14Ns = QStringLiteral("xmlns:xm=\"http://schemas.microsoft.com/office/excel/2006/main\"");
    auto x14 = [&](const char *rule, const char *cells) { return QStringLiteral("<x14:conditionalFormatting %1>%2<xm:sqref>%3</xm:sqref></x14:conditionalFormatting>").arg(x14Ns, QString::fromLatin1(rule), QString::fromLatin1(cells)); };
    QString main;
    for (const char *rule : {kGt, kEven, kTop, kBar}) main += QStringLiteral("<conditionalFormatting sqref=\"A1:A5\">%1</conditionalFormatting>").arg(QString::fromLatin1(rule));
    main += QStringLiteral("<conditionalFormatting sqref=\"A1:A5\">%1%2</conditionalFormatting>").arg(QString::fromLatin1(kScale), QString::fromLatin1(kIcons));
    main += QStringLiteral("<conditionalFormatting sqref=\"B1:B5\">%1</conditionalFormatting>").arg(QString::fromLatin1(kBlank));
    return main + QStringLiteral("<extLst><ext uri=\"{78C0D931-6437-407d-A8EE-F0AAD7539E65}\" xmlns:x14=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\"><x14:conditionalFormattings>")
           + x14(kBarX14, "A1:A5") + x14(kCustom1, "B1:B5") + x14(kCustom6, "C1:C5") + QStringLiteral("</x14:conditionalFormattings></ext></extLst>");
}

QByteArray savedSheet(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return QByteArray();
    QMap<QString, QByteArray> parts;
    jp::readZip(f.readAll(), parts);
    return parts.value(QStringLiteral("xl/worksheets/sheet1.xml"));
}

// The elements named `tag` (with its prefix: "cfRule", "x14:cfRule") in the part, whole.
QList<QByteArray> elementsNamed(const QByteArray &xml, const QByteArray &tag)
{
    QList<QByteArray> out;
    const QByteArray open = "<" + tag;
    for (qsizetype at = xml.indexOf(open); at >= 0; at = xml.indexOf(open, at + 1)) {
        const char next = at + open.size() < xml.size() ? xml[at + open.size()] : 0;
        if (next != ' ' && next != '>' && next != '/') continue;
        const qsizetype gt = xml.indexOf('>', at);
        if (gt < 0) break;
        const qsizetype end = xml[gt - 1] == '/' ? gt + 1 : xml.indexOf("</" + tag + ">", gt) + qsizetype(tag.size()) + 3;
        out.append(xml.mid(at, end - at));
    }
    return out;
}

QByteArray withoutPriority(const QByteArray &element)
{
    QString text = QString::fromUtf8(element);
    text.replace(QRegularExpression(QStringLiteral(" priority=\"\\d+\"")), QStringLiteral(" priority=\"#\""));
    return text.toUtf8();
}

int priorityOfElement(const QByteArray &element)
{
    const QRegularExpressionMatch m = QRegularExpression(QStringLiteral(" priority=\"(\\d+)\"")).match(QString::fromUtf8(element.left(element.indexOf('>'))));
    return m.hasMatch() ? m.captured(1).toInt() : 0;
}

// The rules of the saved sheet in the order of their priority, by the names the nine rules have above (one an edit made again has its name too); the priorities must all be different.
QStringList savedOrder(const QByteArray &xml)
{
    const QMap<QString, QString> byType = {{"cellIs", "gt"}, {"expression", "even"}, {"top10", "top"}, {"dataBar", "bar"}, {"colorScale", "scale"}, {"iconSet", "icons"}, {"containsBlanks", "blank"}};
    QMap<int, QString> order;
    int count = 0;
    for (const QByteArray &e : elementsNamed(xml, "cfRule")) {
        const QRegularExpressionMatch m = QRegularExpression(QStringLiteral("type=\"(\\w+)\"")).match(QString::fromUtf8(e));
        order.insert(priorityOfElement(e), byType.value(m.captured(1), QStringLiteral("?")));
        ++count;
    }
    for (const QByteArray &e : elementsNamed(xml, "x14:cfRule")) {
        const int p = priorityOfElement(e);
        if (p <= 0) continue;
        order.insert(p, e.contains("2222-2222") ? QStringLiteral("custom1") : e.contains("3333-3333") ? QStringLiteral("custom6") : QStringLiteral("?"));
        ++count;
    }
    return order.size() == count ? QStringList(order.values()) : QStringList({QStringLiteral("two rules have one priority")});
}

// What is wrong, if anything, with the rules named: each must be in the saved sheet as the file had it, apart from its priority.
QString notKeptAsTheyWere(const QByteArray &xml, const QStringList &names)
{
    const QMap<QString, const char *> was = {{"gt", kGt}, {"even", kEven}, {"top", kTop}, {"bar", kBar}, {"scale", kScale}, {"icons", kIcons}, {"blank", kBlank}, {"custom1", kCustom1}, {"custom6", kCustom6}};
    QList<QByteArray> saved;
    for (const QByteArray &e : elementsNamed(xml, "cfRule")) saved << withoutPriority(e);
    for (const QByteArray &e : elementsNamed(xml, "x14:cfRule")) saved << withoutPriority(e);
    for (const QString &name : names)
        if (!saved.contains(withoutPriority(QByteArray(was.value(name))))) return name + QStringLiteral(" is not as the file had it");
    if (names.contains(QStringLiteral("bar")) && !saved.contains(QByteArray(kBarX14))) return QStringLiteral("the data bar's 2010 entry is not as the file had it");
    return QString();
}

// Manage Rules on the nine-rule file: `drive` does what the person does in the window, the file is saved through the window's own save, and the saved sheet part comes back (empty, with why).
QByteArray manageAndSave(EditWindow &e, const std::function<void(CondManageDialog &)> &drive, QString *why)
{
    e.win.w.setCondManageAsker([&](const QVector<CondManageDialog::Item> &items, const CrunchWindow::CondEditor &editor, QVector<crunch::EditSession::CondManaged> *out) {
        CondManageDialog d(items, editor);
        drive(d);
        *out = d.result();
        return true;
    });
    QAction *manage = e.win.w.act(QStringLiteral("cond.manage"));
    if (!manage || !manage->isEnabled()) {
        *why = QStringLiteral("Manage Rules is not on");
        return QByteArray();
    }
    manage->trigger();
    if (!e.settle()) {
        *why = QStringLiteral("the window did not settle");
        return QByteArray();
    }
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    if (!e.win.w.save() || !savedOk(saved, 20000)) {
        *why = QStringLiteral("the save failed: ") + e.win.told.join('|');
        return QByteArray();
    }
    return savedSheet(e.path);
}

QString shape(const grid::CondCell &c)
{
    return QStringLiteral("%1%2%3%4").arg(c.any ? "any" : "-").arg(c.format >= 0 ? " fmt" : "").arg(c.hasScale ? " scale" : "").arg(c.hasBar ? " bar" : "") + (c.icon >= 0 ? QStringLiteral(" icon") : QString());
}

// A look of every part a rule can carry: bold, italic, double underline, strikethrough, a font color, a pattern fill, a dotted line under and a thin line at the left, and a number format.
DifferentialFormat everyPartLook()
{
    DifferentialFormat d;
    d.bold = true;
    d.italic = true;
    d.strike = true;
    d.underline = Underline::Double;
    d.fontColor = ColorSpec::fromRgb(0x336699);
    d.hasFill = true;
    d.fill.pattern = PatternKind::LightGrid;
    d.fill.fg = ColorSpec::fromRgb(0x112233);
    d.fill.bg = ColorSpec::fromRgb(0xFFEEDD);
    d.hasBorder = true;
    d.border.bottom.style = BorderStyle::Dotted;
    d.border.bottom.color = ColorSpec::fromRgb(0xFF0000);
    d.border.left.style = BorderStyle::Thin;
    d.border.left.color = ColorSpec::fromRgb(0x00AA00);
    d.numberFormat = QStringLiteral("0.00");
    return d;
}

QByteArray fileBytes(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

// The part of the saved package: "xl/styles.xml" and the like.
QByteArray savedPart(const QString &path, const QString &name)
{
    QMap<QString, QByteArray> parts;
    jp::readZip(fileBytes(path), parts);
    return parts.value(name);
}

bool saveThrough(EditWindow &e)
{
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    return e.win.w.save() && savedOk(saved, 20000);
}

// New Rule on `range` with the person's own look: the window's answer is the whole rule.
bool newRuleWithLook(EditWindow &e, const QString &range, const QString &value, const DifferentialFormat &look)
{
    selectRange(e, range);
    e.win.w.setCondSpecAsker([=](CondSpec *spec, bool) {
        spec->kind = CondSpec::Kind::CellValue;
        spec->op = CondRule::Operator::GreaterThan;
        spec->v1 = value;
        spec->useCustom = true;
        spec->custom = look;
        return true;
    });
    QAction *a = e.win.w.act(QStringLiteral("cond.new"));
    if (!a || !a->isEnabled()) return false;
    a->trigger();
    return true;
}

// The look of the first rule of the sheet, as the workbook holds it.
DifferentialFormat firstRuleLook(EditWindow &e)
{
    const QVector<CondRule> rules = e.win.w.session()->condRules(0);
    return rules.isEmpty() ? DifferentialFormat() : e.win.w.session()->condLookOf(rules.first());
}

// Manage Rules, with the first rule edited through the window's answer.
void editFirstRule(EditWindow &e)
{
    e.win.w.setCondManageAsker([](const QVector<CondManageDialog::Item> &items, const CrunchWindow::CondEditor &editor, QVector<crunch::EditSession::CondManaged> *out) {
        CondManageDialog d(items, editor);
        d.select(0);
        d.editSelected();
        *out = d.result();
        return true;
    });
    run(e, "cond.manage");
}


// The differential formats of a styles part, each as written.
QList<QByteArray> dxfEntries(const QByteArray &styles)
{
    QList<QByteArray> out;
    const qsizetype from = styles.indexOf("<dxfs"), to = styles.indexOf("</dxfs>");
    if (from < 0 || to < 0) return out;
    const QByteArray list = styles.mid(from, to - from);
    qsizetype at = list.indexOf("<dxf>");
    while (at >= 0) {
        const qsizetype end = list.indexOf("</dxf>", at);
        if (end < 0) break;
        out << list.mid(at, end + 6 - at);
        at = list.indexOf("<dxf>", end);
    }
    return out;
}

// The first rule's look changed by `change`, as the Format Cells box would: the look the rule has, with only what the person changed put in.
void changeFirstRuleLook(EditWindow &e, const std::function<void(DifferentialFormat *)> &change)
{
    e.win.w.setCondSpecAsker([change](CondSpec *s, bool) {
        s->keepLook = false;
        s->useCustom = true;
        s->custom = s->currentLook;
        change(&s->custom);
        return true;
    });
    editFirstRule(e);
}

// A look that is only a font color, a different one for each number.
DifferentialFormat numberedLook(int n)
{
    DifferentialFormat d;
    d.fontColor = ColorSpec::fromRgb(0x100000 + quint32(n) * 7);
    return d;
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
        QCOMPARE(cond.figureCells(), qint64(5));
        QCOMPARE(cond.statCells(), qint64(5));
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

    // ---- Manage Rules on a file as it was opened (no rule has been touched in the window yet): what the person did not change is kept as the file had it, apart from its priority, and the order of
    // every rule in the file, those Manage Rules does not list too, is what it was but for the move the person made.
    void manageRulesOnAFreshFileOkWithNothingChangedKeepsEveryRule()
    {
        EditWindow e;
        QVERIFY(e.open(condBook(nineRulesTail())));
        const int steps = e.win.w.undoStack()->count();
        e.win.w.setCondManageAsker([](const QVector<CondManageDialog::Item> &items, const CrunchWindow::CondEditor &editor, QVector<crunch::EditSession::CondManaged> *out) {
            CondManageDialog d(items, editor);
            *out = d.result();
            return true;
        });
        run(e, "cond.manage");
        QCOMPARE(e.win.w.undoStack()->count(), steps);          // (nothing was changed: nothing is made again)
        QVERIFY(!e.win.w.isModified());
    }

    void manageRulesEditOneRuleKeepsTheOthersAsTheFileHadThem()
    {
        EditWindow e;
        QVERIFY(e.open(condBook(nineRulesTail())));
        e.win.w.setCondSpecAsker([](CondSpec *spec, bool editing) {
            if (!editing || spec->kind != CondSpec::Kind::Formula) return false;
            spec->v1 = QStringLiteral("=MOD(A1,3)=0");
            return true;
        });
        QString why;
        const QByteArray xml = manageAndSave(e, [](CondManageDialog &d) {
            d.select(1);                                    // (the rule with the formula)
            d.editSelected();
        }, &why);
        QVERIFY2(!xml.isEmpty(), qPrintable(why));
        QVERIFY(xml.contains("MOD(A1,3)=0"));
        QVERIFY(!xml.contains("MOD(A1,2)=0"));
        QCOMPARE(savedOrder(xml), QStringList({"custom1", "gt", "even", "top", "bar", "custom6", "scale", "icons", "blank"}));
        const QString bad = notKeptAsTheyWere(xml, {"gt", "top", "bar", "scale", "icons", "blank", "custom1", "custom6"});
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    void manageRulesMoveUpKeepsTheOthersAsTheFileHadThem()
    {
        EditWindow e;
        QVERIFY(e.open(condBook(nineRulesTail())));
        QString why;
        const QByteArray xml = manageAndSave(e, [](CondManageDialog &d) {
            d.select(4);                                    // (the color scale)
            d.moveSelected(-1);
        }, &why);
        QVERIFY2(!xml.isEmpty(), qPrintable(why));
        QCOMPARE(savedOrder(xml), QStringList({"custom1", "gt", "even", "top", "scale", "bar", "custom6", "icons", "blank"}));
        const QString bad = notKeptAsTheyWere(xml, {"gt", "even", "top", "bar", "scale", "icons", "blank", "custom1", "custom6"});
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    void manageRulesMoveDownKeepsTheOthersAsTheFileHadThem()
    {
        EditWindow e;
        QVERIFY(e.open(condBook(nineRulesTail())));
        QString why;
        const QByteArray xml = manageAndSave(e, [](CondManageDialog &d) {
            d.select(0);                                    // (the first rule: the rule that lives only in the 2010 block stays above it)
            d.moveSelected(1);
        }, &why);
        QVERIFY2(!xml.isEmpty(), qPrintable(why));
        QCOMPARE(savedOrder(xml), QStringList({"custom1", "even", "gt", "top", "bar", "custom6", "scale", "icons", "blank"}));
        const QString bad = notKeptAsTheyWere(xml, {"gt", "even", "top", "bar", "scale", "icons", "blank", "custom1", "custom6"});
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    void manageRulesDeleteOneRuleKeepsTheOthersAsTheFileHadThem()
    {
        EditWindow e;
        QVERIFY(e.open(condBook(nineRulesTail())));
        QString why;
        const QByteArray xml = manageAndSave(e, [](CondManageDialog &d) {
            d.select(2);                                    // (the rule of a pivot table's area)
            d.deleteSelected();
        }, &why);
        QVERIFY2(!xml.isEmpty(), qPrintable(why));
        QVERIFY(!xml.contains("pivot=\"1\""));
        QCOMPARE(savedOrder(xml), QStringList({"custom1", "gt", "even", "bar", "custom6", "scale", "icons", "blank"}));
        const QString bad = notKeptAsTheyWere(xml, {"gt", "even", "bar", "scale", "icons", "blank", "custom1", "custom6"});
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        // and the data bar, which has a 2010 entry: it goes with its entry, and the rule that lives only in the 2010 block stays where it was
        EditWindow second;
        QVERIFY(second.open(condBook(nineRulesTail())));
        const QByteArray other = manageAndSave(second, [](CondManageDialog &d) {
            d.select(3);
            d.deleteSelected();
        }, &why);
        QVERIFY2(!other.isEmpty(), qPrintable(why));
        QVERIFY(!other.contains("negativeBarBorderColorSameAsPositive"));
        QCOMPARE(savedOrder(other), QStringList({"custom1", "gt", "even", "top", "custom6", "scale", "icons", "blank"}));
        const QString lost = notKeptAsTheyWere(other, {"gt", "even", "top", "scale", "icons", "blank", "custom1", "custom6"});
        QVERIFY2(lost.isEmpty(), qPrintable(lost));
    }

    // A new rule made in Manage Rules goes first, above the rule that lives only in the 2010 block too; what is saved opens again with the order it had.
    void manageRulesNewRuleGoesFirstAndTheFileOpensAgainInTheSameOrder()
    {
        EditWindow e;
        QVERIFY(e.open(condBook(nineRulesTail())));
        e.win.w.setCondSpecAsker([](CondSpec *spec, bool editing) {
            if (editing) return false;
            spec->kind = CondSpec::Kind::Duplicate;
            return true;
        });
        QString why;
        const QByteArray xml = manageAndSave(e, [](CondManageDialog &d) { d.addRule(); }, &why);
        QVERIFY2(!xml.isEmpty(), qPrintable(why));
        QCOMPARE(savedOrder(xml), QStringList({"?", "custom1", "gt", "even", "top", "bar", "custom6", "scale", "icons", "blank"}));   // ("?": the new rule)
        const QString bad = notKeptAsTheyWere(xml, {"gt", "even", "top", "bar", "scale", "icons", "blank", "custom1", "custom6"});
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        f.close();
        EditWindow again;
        QVERIFY(again.open(bytes));
        const QVector<CondRule> back = again.win.w.session()->condRules(0);
        QCOMPARE(int(back.size()), 8);
        QCOMPARE(int(back.first().type), int(CondRule::Type::DuplicateValues));
    }

    // Stop If True, set in Manage Rules: it is kept by the save, it is there when the file is opened again, and a rule below the one that stops is not applied to the cells it is true for.
    void manageRulesStopIfTrueIsSavedAndStopsTheRulesBelow()
    {
        EditWindow e;
        QVERIFY(e.open(condBook()));
        DifferentialFormat italic, bold;
        italic.italic = true;
        bold.bold = true;
        CondRule below = rule(CondRule::Type::CellIs, "A1:A5"), above = rule(CondRule::Type::CellIs, "A1:A5");
        below.op = above.op = CondRule::Operator::GreaterThan;
        below.formula1 = QStringLiteral("0");
        above.formula1 = QStringLiteral("3");
        QVERIFY(e.win.w.session()->addCondRule(0, below, italic).ok);
        QVERIFY(e.win.w.session()->addCondRule(0, above, bold).ok);
        QVERIFY(e.settle());
        auto &cond = e.view()->sheet().conditional();
        cond.settle();
        const grid::CondCell both = cond.at(4, 0);                     // (5 is over 3, and over 0: both rules apply)
        QVERIFY(both.format >= 0 && cond.format(both.format).bold.value_or(false) && cond.format(both.format).italic.value_or(false));
        QString why;
        const QByteArray xml = manageAndSave(e, [](CondManageDialog &d) {
            d.select(0);                                    // (the rule that is over 3)
            d.setStopIfTrue(0, true);
        }, &why);
        QVERIFY2(!xml.isEmpty(), qPrintable(why));
        QVERIFY(xml.contains("stopIfTrue=\"1\""));
        cond.settle();
        const grid::CondCell stopped = cond.at(4, 0);
        QVERIFY(stopped.format >= 0 && cond.format(stopped.format).bold.value_or(false));
        QVERIFY(!cond.format(stopped.format).italic.value_or(false));
        const grid::CondCell low = cond.at(2, 0);                      // (3 is not over 3: only the rule below applies)
        QVERIFY(low.format >= 0 && cond.format(low.format).italic.value_or(false));
        // opened again
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        f.close();
        EditWindow again;
        QVERIFY(again.open(bytes));
        const QVector<CondRule> back = again.win.w.session()->condRules(0);
        QCOMPARE(int(back.size()), 2);
        QVERIFY(back[0].stopIfTrue);
        QVERIFY(!back[1].stopIfTrue);
        auto &condAgain = again.view()->sheet().conditional();
        condAgain.settle();
        const grid::CondCell reopened = condAgain.at(4, 0);
        QVERIFY(reopened.format >= 0 && condAgain.format(reopened.format).bold.value_or(false));
        QVERIFY(!condAgain.format(reopened.format).italic.value_or(false));
    }

    // The cells of a rule changed in Manage Rules (Applies to): the saved file and the drawing follow.
    void manageRulesChangingTheCellsIsSavedAndDrawn()
    {
        EditWindow e;
        QVERIFY(e.open(condBook(nineRulesTail())));
        QString why;
        const QByteArray xml = manageAndSave(e, [](CondManageDialog &d) {
            QVERIFY(d.setRangesOf(1, QStringLiteral("A1:A2 A4")));   // (the rule with the formula: even numbers)
        }, &why);
        QVERIFY2(!xml.isEmpty(), qPrintable(why));
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        f.close();
        EditWindow again;
        QVERIFY(again.open(bytes));
        const QVector<CondRule> back = again.win.w.session()->condRules(0);
        QCOMPARE(int(back.size()), 7);
        QCOMPARE(int(back[1].type), int(CondRule::Type::Expression));
        QCOMPARE(int(back[1].ranges.size()), 2);
        QCOMPARE(rangeName(back[1].ranges[0]), QStringLiteral("A1:A2"));
        QCOMPARE(rangeName(back[1].ranges[1]), QStringLiteral("A4"));
        QCOMPARE(savedOrder(xml), QStringList({"custom1", "gt", "even", "top", "bar", "custom6", "scale", "icons", "blank"}));
        const QString bad = notKeptAsTheyWere(xml, {"gt", "top", "bar", "scale", "icons", "blank", "custom1", "custom6"});
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    // A sheet whose protection does not let cells be formatted gets no rule from the menus, the window of a new rule, or Manage Rules, and its Clear Rules choices are off; when the protection lets
    // cells be formatted, the same changes are made.
    void aProtectedSheetRefusesRulesThatTheProtectionDoesNotAllow()
    {
        EditWindow e;
        QVERIFY(e.open(condBook(QStringLiteral("<sheetProtection sheet=\"1\"/>") + nineRulesTail())));
        const int steps = e.win.w.undoStack()->count();
        selectRange(e, QStringLiteral("B1:B5"));
        run(e, "cond.bar.0.gradient");
        QVERIFY2(e.win.told.join('|').contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        CondRule made = rule(CondRule::Type::UniqueValues, "B1:B5");
        const auto added = e.win.w.session()->addCondRule(0, made, DifferentialFormat());
        QVERIFY(!added.ok);
        QVERIFY2(added.problem.contains(QLatin1String("protected")), qPrintable(added.problem));
        e.win.told.clear();
        bool asked = false;
        e.win.w.setCondManageAsker([&](const QVector<CondManageDialog::Item> &, const CrunchWindow::CondEditor &, QVector<crunch::EditSession::CondManaged> *) {
            asked = true;
            return false;
        });
        run(e, "cond.manage");
        QVERIFY(!asked);
        QVERIFY2(e.win.told.join('|').contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
        const auto managed = e.win.w.session()->setManagedCondRules(0, {});
        QVERIFY(!managed.ok);
        QVERIFY2(managed.problem.contains(QLatin1String("protected")), qPrintable(managed.problem));
        QCOMPARE(int(e.win.w.session()->condRules(0).size()), 7);
        QVERIFY(!e.win.w.act(QStringLiteral("cond.clearSheet"))->isEnabled());
        QVERIFY(!e.win.w.session()->clearCondRules(0, CellRange()).ok);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        // formatting allowed: the same sheet takes a rule
        EditWindow open;
        QVERIFY(open.open(condBook(QStringLiteral("<sheetProtection sheet=\"1\" formatCells=\"0\"/>") + nineRulesTail())));
        selectRange(open, QStringLiteral("B1:B5"));
        run(open, "cond.bar.0.gradient");
        QCOMPARE(int(open.win.w.session()->condRules(0).size()), 8);
        QVERIFY(open.win.w.act(QStringLiteral("cond.clearSheet"))->isEnabled());
    }

    // A New Rule made in Manage Rules reads a typed formula as seen from the cell with the cursor, as the New Rule of the menu does, whatever corner of the selection that is.
    void aNewRuleInManageRulesReadsItsFormulaFromTheCellWithTheCursor()
    {
        EditWindow e;
        QVERIFY(e.open(condBook()));
        e.view()->selectRange(2, 2, 1, 1);                  // (B2:C3, the cursor in C3)
        QCOMPARE(e.view()->selection().activeRow, qint64(2));
        QVERIFY(e.view()->selection().range.row1 == 1);
        e.win.w.setCondSpecAsker([](CondSpec *spec, bool editing) {
            if (editing) return false;
            spec->kind = CondSpec::Kind::Formula;
            spec->v1 = QStringLiteral("=C3>1");             // (as seen from C3: "this cell is over 1")
            return true;
        });
        run(e, "cond.new");
        QCOMPARE(e.win.w.session()->condRules(0).first().formula1, QStringLiteral("B2>1"));
        e.win.w.session()->clearCondRules(0, CellRange());
        QString why;
        e.win.w.setCondManageAsker([&](const QVector<CondManageDialog::Item> &items, const CrunchWindow::CondEditor &editor, QVector<crunch::EditSession::CondManaged> *out) {
            CondManageDialog d(items, editor);
            d.addRule();
            *out = d.result();
            return true;
        });
        run(e, "cond.manage");
        const QVector<CondRule> now = e.win.w.session()->condRules(0);
        QCOMPARE(int(now.size()), 1);
        QCOMPARE(now.first().formula1, QStringLiteral("B2>1"));
        // (an edited rule shows its formula as the top left cell reads it, and keeps it)
        e.win.w.setCondManageAsker([&](const QVector<CondManageDialog::Item> &items, const CrunchWindow::CondEditor &editor, QVector<crunch::EditSession::CondManaged> *out) {
            CondManageDialog d(items, editor);
            d.select(0);
            d.editSelected();
            *out = d.result();
            return true;
        });
        QString shown;
        e.win.w.setCondSpecAsker([&shown](CondSpec *spec, bool editing) {
            if (editing) shown = spec->v1;
            return false;
        });
        run(e, "cond.manage");
        QCOMPARE(shown, QStringLiteral("=B2>1"));
        QCOMPARE(e.win.w.session()->condRules(0).first().formula1, QStringLiteral("B2>1"));
    }

    // A reference to a sheet the workbook does not have is refused as any other formula that cannot be read is, in a rule's formula and in a value a rule compares with.
    void aRuleThatNamesASheetThatIsNotThereIsRefused()
    {
        EditWindow e;
        QVERIFY(e.open(condBook()));
        selectRange(e, QStringLiteral("A1:A5"));
        const int steps = e.win.w.undoStack()->count();
        for (const bool formula : {true, false}) {
            e.win.told.clear();
            e.win.w.setCondSpecAsker([formula](CondSpec *spec, bool editing) {
                if (editing) return false;
                if (formula) {
                    spec->kind = CondSpec::Kind::Formula;
                    spec->v1 = QStringLiteral("=Nowhere!A1>1");
                } else {
                    spec->kind = CondSpec::Kind::CellValue;
                    spec->v1 = QStringLiteral("='Not Here'!B2");
                }
                return true;
            });
            run(e, "cond.new");
            QVERIFY2(e.win.told.join('|').contains(QLatin1String("is not a formula JeffCrunch can read")), qPrintable(e.win.told.join('|')));
            QCOMPARE(e.win.w.undoStack()->count(), steps);
        }
        // a sheet the workbook has is fine
        e.win.told.clear();
        e.win.w.setCondSpecAsker([](CondSpec *spec, bool editing) {
            if (editing) return false;
            spec->kind = CondSpec::Kind::Formula;
            spec->v1 = QStringLiteral("=Sheet1!B1<>\"\"");
            return true;
        });
        run(e, "cond.new");
        QVERIFY2(e.win.told.isEmpty(), qPrintable(e.win.told.join('|')));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
    }

    // The gallery's solid bars have a border of their own for negative numbers: the file says so, as a newer file's bars with a negative border do.
    void aSolidBarSaysItsNegativeBorderIsNotThePositiveOne()
    {
        EditWindow e;
        QVERIFY(e.open(condBook()));
        selectRange(e, QStringLiteral("A1:A5"));
        run(e, "cond.bar.0.solid");
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join('|')));
        const QByteArray xml = savedSheet(e.path);
        QVERIFY(xml.contains("<x14:negativeBorderColor"));
        QVERIFY2(xml.contains("negativeBarBorderColorSameAsPositive=\"0\""), xml.constData());
    }

    // The description of a rule for the top items never says "1 items".
    void theDescriptionOfATopRuleHasNoWrongPlural()
    {
        for (const bool bottom : {false, true})
            for (const int n : {1, 2, 10}) {
                CondRule r = rule(CondRule::Type::Top10);
                r.rank = n;
                r.bottom = bottom;
                const QString text = condpresets::describe(r);
                QVERIFY2(text.contains(QString::number(n)), qPrintable(text));
                if (n == 1) QVERIFY2(!text.contains(QLatin1String("1 items")), qPrintable(text));
            }
    }

    // The rule window: a color scale, data bar, or icon set has no look to choose, so the row is not there; the pages do not leave room for the tallest page under a short one; the border's
    // color is in the column of the other colors, level with its check box; and the icon sets are named in words a person can follow.
    void theRuleWindowShowsOnlyWhatTheKindNeedsAndAlignsTheBorderColor()
    {
        jp::applyUiTheme(1);
        CondSpec start;
        CondRuleDialog d(start, false);
        d.show();
        QTRY_VERIFY(d.isVisible());
        auto *look = d.findChild<QComboBox *>(QStringLiteral("condPreset"));
        auto *pages = d.findChild<QStackedWidget *>();
        QVERIFY(look && pages);
        QVERIFY(look->isVisibleTo(&d));
        d.setKind(CondSpec::Kind::Duplicate);
        QVERIFY(look->isVisibleTo(&d));
        QVERIFY2(pages->sizeHint().height() <= pages->currentWidget()->sizeHint().height() + 2, "the pages keep the room of the tallest");
        for (const auto kind : {CondSpec::Kind::ColorScale, CondSpec::Kind::DataBar, CondSpec::Kind::IconSet}) {
            d.setKind(kind);
            QVERIFY(!look->isVisibleTo(&d));
        }
        d.setKind(CondSpec::Kind::DataBar);
        auto *color = d.findChild<QPushButton *>(QStringLiteral("barColor"));
        auto *border = d.findChild<QPushButton *>(QStringLiteral("barBorder"));
        auto *on = d.findChild<QCheckBox *>(QStringLiteral("barBorderOn"));
        QVERIFY(color && border && on);
        QTRY_COMPARE(border->mapTo(&d, QPoint(0, 0)).x(), color->mapTo(&d, QPoint(0, 0)).x());
        QTRY_VERIFY(std::abs(border->mapTo(&d, border->rect().center()).y() - on->mapTo(&d, on->rect().center()).y()) <= 2);
        QCOMPARE(condpresets::iconSetLabel(QStringLiteral("3TrafficLights1")), QStringLiteral("3 Traffic Lights (No Border)"));
        QCOMPARE(condpresets::iconSetLabel(QStringLiteral("3TrafficLights2")), QStringLiteral("3 Traffic Lights (with Border)"));
        QCOMPARE(condpresets::iconSetLabel(QStringLiteral("3Symbols")), QStringLiteral("3 Symbols (in Circles)"));
        QCOMPARE(condpresets::iconSetLabel(QStringLiteral("3Symbols2")), QStringLiteral("3 Symbols (Plain)"));
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

    // New Rule with Custom Format: every part of the look (font style and color, pattern fill, lines, number format) is held, is one step of Undo, is saved through the window's own save with its
    // parts in the styles part, and reads back the same.
    void aCustomLookIsMadeSavedAndReadBackWithEveryPart()
    {
        EditWindow e;
        QVERIFY(e.open(condBook()));
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(newRuleWithLook(e, QStringLiteral("A1:A5"), QStringLiteral("2"), everyPartLook()));
        QVERIFY2(e.win.told.isEmpty(), qPrintable(e.win.told.join('|')));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);      // (one step)
        QVERIFY(sameCondLook(firstRuleLook(e), everyPartLook()));
        QVERIFY(saveThrough(e));
        const QByteArray styles = savedPart(e.path, QStringLiteral("xl/styles.xml"));
        QVERIFY(styles.contains("patternType=\"lightGrid\""));
        QVERIFY(styles.contains("formatCode=\"0.00\""));
        QVERIFY(styles.contains("style=\"dotted\""));
        QVERIFY(styles.contains("<u val=\"double\"/>"));
        EditWindow again;
        QVERIFY(again.open(fileBytes(e.path)));
        QVERIFY2(sameCondLook(firstRuleLook(again), everyPartLook()), "the look read back is not the look made");
        // undo takes the rule away, and redo gives it back with its look
        e.win.w.undoStack()->undo();
        QCOMPARE(int(e.win.w.session()->condRules(0).size()), 0);
        e.win.w.undoStack()->redo();
        QVERIFY(sameCondLook(firstRuleLook(e), everyPartLook()));
    }

    // 50 edits that give a rule the same look add one differential format to the styles part, not 50.
    void fiftyEditsWithOneCustomLookShareOneDifferentialFormat()
    {
        EditWindow e;
        QVERIFY(e.open(condBook()));
        QVERIFY(newRuleWithLook(e, QStringLiteral("A1:A5"), QStringLiteral("2"), everyPartLook()));
        for (int i = 0; i < 50; ++i) {
            e.win.w.setCondSpecAsker([i](CondSpec *spec, bool editing) {
                if (!editing) return false;
                spec->v1 = QString::number(10 + i);
                spec->keepLook = false;
                spec->useCustom = true;
                spec->custom = everyPartLook();
                return true;
            });
            editFirstRule(e);
            QVERIFY2(e.win.told.isEmpty(), qPrintable(e.win.told.join('|')));
        }
        QVERIFY(saveThrough(e));
        const QByteArray styles = savedPart(e.path, QStringLiteral("xl/styles.xml"));
        QCOMPARE(styles.count("<dxf>"), 1);
        QVERIFY(sameCondLook(firstRuleLook(e), everyPartLook()));
    }

    // A rule read from a file with a look of its own shows as Custom Format with its true sample, and an edit of something else leaves the look as the file has it: the same differential format,
    // the same bytes.
    void aFilesOwnCustomLookShowsAsCustomFormatAndKeepsItsBytes()
    {
        const QString dxf = QStringLiteral("<dxfs count=\"1\"><dxf><font><b/><color rgb=\"FF336699\"/></font><numFmt numFmtId=\"2\" formatCode=\"0.00\"/>"
                                           "<border><left style=\"thin\"><color rgb=\"FF00AA00\"/></left><right/><top/><bottom/></border></dxf></dxfs>");
        const QString tail = QStringLiteral("<conditionalFormatting sqref=\"A1:A5\"><cfRule type=\"cellIs\" dxfId=\"0\" priority=\"1\" operator=\"greaterThan\"><formula>3</formula></cfRule></conditionalFormatting>");
        EditWindow e;
        QVERIFY(e.open(condBook(tail, dxf)));
        const QVector<CondRule> rules = e.win.w.session()->condRules(0);
        QCOMPARE(int(rules.size()), 1);
        const DifferentialFormat theirs = e.win.w.session()->condLookOf(rules.first());
        QVERIFY(theirs.bold.value_or(false) && theirs.hasBorder && theirs.numberFormat == QStringLiteral("0.00"));
        // the rule window
        CondSpec spec = specOfRule(rules.first());
        spec.currentLook = theirs;
        CondRuleDialog dialog(spec, true);
        QCOMPARE(dialog.lookBox()->list()->itemText(0), QStringLiteral("Custom Format"));
        QVERIFY(sameCondLook(dialog.lookBox()->sample()->look(), theirs));
        QVERIFY(dialog.spec().keepLook);
        // an edit of the value alone
        e.win.w.setCondSpecAsker([](CondSpec *s, bool) {
            s->v1 = QStringLiteral("4");
            return true;
        });
        editFirstRule(e);
        QVERIFY(saveThrough(e));
        const QByteArray sheet = savedSheet(e.path);
        QVERIFY(sheet.contains("<formula>4</formula>"));
        QVERIFY(sheet.contains("dxfId=\"0\""));
        const QByteArray styles = savedPart(e.path, QStringLiteral("xl/styles.xml"));
        QCOMPARE(styles.count("<dxf>"), 1);
        QVERIFY(styles.contains(QByteArray("<dxf><font><b/><color rgb=\"FF336699\"/></font><numFmt numFmtId=\"2\" formatCode=\"0.00\"/>")));
        // choosing Custom Format... and leaving the look as it is changes nothing either
        EditWindow same;
        QVERIFY(same.open(condBook(tail, dxf)));
        same.win.w.setCondSpecAsker([](CondSpec *s, bool) {
            s->v1 = QStringLiteral("5");
            s->keepLook = false;
            s->useCustom = true;
            s->custom = s->currentLook;
            return true;
        });
        editFirstRule(same);
        QVERIFY(saveThrough(same));
        QCOMPARE(savedPart(same.path, QStringLiteral("xl/styles.xml")).count("<dxf>"), 1);
        QVERIFY(savedSheet(same.path).contains("<formula>5</formula>"));
        QVERIFY(savedSheet(same.path).contains("dxfId=\"0\""));
    }

    // A rule from a file whose look has parts the box does not hold (a gradient fill, a font's name, size, and raised text, an alignment, a protection, a line across the corner): changing
    // only the italic writes a new entry that is the file's own with the italic put in, every other byte as the file had it, and it never writes a solid fill with no color.
    void aFilesLookKeepsEveryPartTheBoxDoesNotHold()
    {
        const QByteArray base = "<dxf><font><b/><sz val=\"14\"/><vertAlign val=\"superscript\"/><color rgb=\"FF336699\"/><name val=\"Cambria\"/></font><numFmt numFmtId=\"164\" formatCode=\"0.0\"/>"
                                "<fill><gradientFill degree=\"90\"><stop position=\"0\"><color rgb=\"FFFF0000\"/></stop><stop position=\"1\"><color rgb=\"FF0000FF\"/></stop></gradientFill></fill>"
                                "<alignment horizontal=\"center\" indent=\"1\"/>"
                                "<border diagonalDown=\"1\"><left style=\"thin\"><color rgb=\"FF00AA00\"/></left><right/><top/><bottom/><diagonal style=\"thin\"><color rgb=\"FF112233\"/></diagonal></border>"
                                "<protection locked=\"0\" hidden=\"1\"/></dxf>";
        const QString dxf = QStringLiteral("<dxfs count=\"1\">") + QString::fromLatin1(base) + QStringLiteral("</dxfs>");
        const QString tail = QStringLiteral("<conditionalFormatting sqref=\"A1:A5\"><cfRule type=\"cellIs\" dxfId=\"0\" priority=\"1\" operator=\"greaterThan\"><formula>3</formula></cfRule></conditionalFormatting>");
        {
            EditWindow e;
            QVERIFY(e.open(condBook(tail, dxf)));
            changeFirstRuleLook(e, [](DifferentialFormat *d) { d->italic = true; });
            QVERIFY2(e.win.told.isEmpty(), qPrintable(e.win.told.join('|')));
            QVERIFY(saveThrough(e));
            const QByteArray styles = savedPart(e.path, QStringLiteral("xl/styles.xml"));
            const QList<QByteArray> entries = dxfEntries(styles);
            QCOMPARE(int(entries.size()), 2);
            QCOMPARE(entries[0], base);
            QByteArray want = base;
            want.replace("<font><b/>", "<font><b/><i/>");
            QCOMPARE(entries[1], want);
            QVERIFY(!styles.contains("<patternFill patternType=\"solid\"></patternFill>"));
            QVERIFY(savedSheet(e.path).contains("dxfId=\"1\""));
            EditWindow again;
            QVERIFY(again.open(fileBytes(e.path)));
            QVERIFY(firstRuleLook(again).italic.value_or(false));
        }
        {
            // a fill the person sets replaces the gradient (what they asked), and nothing else of the entry moves
            EditWindow e;
            QVERIFY(e.open(condBook(tail, dxf)));
            changeFirstRuleLook(e, [](DifferentialFormat *d) {
                d->hasFill = true;
                d->fill = FillSpec();
                d->fill.pattern = PatternKind::Solid;
                d->fill.fg = d->fill.bg = ColorSpec::fromRgb(0xFF0000);
            });
            QVERIFY(saveThrough(e));
            const QList<QByteArray> entries = dxfEntries(savedPart(e.path, QStringLiteral("xl/styles.xml")));
            QCOMPARE(int(entries.size()), 2);
            QByteArray want = base;
            const qsizetype from = want.indexOf("<fill>"), to = want.indexOf("</fill>") + 7;
            want.replace(from, to - from, "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FFFF0000\"/><bgColor rgb=\"FFFF0000\"/></patternFill></fill>");
            QCOMPARE(entries[1], want);
        }
        {
            // a look the person leaves as it is stays the file's entry (no new one)
            EditWindow e;
            QVERIFY(e.open(condBook(tail, dxf)));
            changeFirstRuleLook(e, [](DifferentialFormat *) {});
            QVERIFY(saveThrough(e));
            QCOMPARE(int(dxfEntries(savedPart(e.path, QStringLiteral("xl/styles.xml"))).size()), 1);
        }
    }

    // The number a differential format gives its number format is one above every number the file uses, is the file's own number when it has the code, and is kept by a later save that adds
    // another code.
    void differentialNumberFormatsAreNumberedAboveEveryNumberTheFileUses()
    {
        const QString numFmts = QStringLiteral("<numFmt numFmtId=\"300\" formatCode=\"0.0000\"/>");
        EditWindow e;
        QVERIFY(e.open(condBook(QString(), QString(), numFmts)));
        auto look = [](const char *code) {
            DifferentialFormat d;
            d.numberFormat = QString::fromLatin1(code);
            return d;
        };
        auto idsOf = [&]() {
            QList<int> ids;
            static const QRegularExpression pattern(QStringLiteral("numFmtId=\"(\\d+)\""));
            for (const QByteArray &entry : dxfEntries(savedPart(e.path, QStringLiteral("xl/styles.xml")))) {
                const QRegularExpressionMatch m = pattern.match(QString::fromLatin1(entry));
                ids << (m.hasMatch() ? m.captured(1).toInt() : -1);
            }
            return ids;
        };
        QVERIFY(newRuleWithLook(e, QStringLiteral("A1:A5"), QStringLiteral("2"), look("0.00")));
        QVERIFY(saveThrough(e));
        QList<int> ids = idsOf();
        QCOMPARE(int(ids.size()), 1);
        QVERIFY2(ids[0] > 300, "the number is not above the file's own");
        const int first = ids[0];
        QVERIFY(newRuleWithLook(e, QStringLiteral("C1:C5"), QStringLiteral("2"), look("0.000")));
        QVERIFY(saveThrough(e));
        ids = idsOf();
        QCOMPARE(int(ids.size()), 2);
        QCOMPARE(ids[0], first);                  // (the first look keeps its number)
        QVERIFY(ids[1] > first && ids[1] != 300);
        QVERIFY(newRuleWithLook(e, QStringLiteral("B1:B5"), QStringLiteral("2"), look("0.0000")));
        QVERIFY(saveThrough(e));
        ids = idsOf();
        QCOMPARE(int(ids.size()), 3);
        QCOMPARE(ids[2], 300);                    // (the file has that code under 300)
        EditWindow again;
        QVERIFY(again.open(fileBytes(e.path)));
        QCOMPARE(again.win.w.session()->condLookOf(again.win.w.session()->condRules(0).last()).numberFormat.value_or(QString()), QStringLiteral("0.00"));
    }

    // Looks that no rule, table style, or color filter uses any more are not written, wherever they are among the looks: 50 looks made, the rules deleted but the first and the last, and exactly the
    // two looks they use are written; a second round on the saved file adds two more; a table style's look is written too; and the looks of rules an Undo brings back after a save are written by the
    // next save.
    void looksNothingUsesAreNotWritten()
    {
        auto round = [&](EditWindow &e, int base) {
            QVERIFY(newRuleWithLook(e, QStringLiteral("E1:E5"), QStringLiteral("2"), numberedLook(base)));            // (kept: made first)
            for (int i = 1; i < 49; ++i) QVERIFY(newRuleWithLook(e, QStringLiteral("A1:A5"), QStringLiteral("2"), numberedLook(base + i)));
            QVERIFY(newRuleWithLook(e, QStringLiteral("F1:F5"), QStringLiteral("2"), numberedLook(base + 49)));       // (kept: made last)
            QVERIFY(e.win.w.session()->clearCondRules(0, rangeOf(QStringLiteral("A1:C5"))).ok);
            QCOMPARE(int(e.win.w.session()->condRules(0).size()), 2 + (base > 0 ? 2 : 0));
        };
        auto looksOf = [](EditWindow &e) {
            QList<quint32> colors;
            for (const CondRule &r : e.win.w.session()->condRules(0)) colors << quint32(e.win.w.session()->condLookOf(r).fontColor.rgb & 0xFFFFFF);
            std::sort(colors.begin(), colors.end());
            return colors;
        };
        auto colorOf = [](int n) { return quint32(numberedLook(n).fontColor.rgb & 0xFFFFFF); };
        EditWindow e;
        QVERIFY(e.open(condBook()));
        round(e, 0);
        crunch::EditSession *session = e.win.w.session();
        // a table style with a look of its own
        QVERIFY(session->makeTable(0, rangeOf(QStringLiteral("A1:C5")), true, QString()).ok);
        QVector<jp::office::sheet::Workbook::TableStylePart> parts;
        jp::office::sheet::Workbook::TableStylePart part;
        part.part = TablePart::HeaderRow;
        part.format = numberedLook(2000);
        parts << part;
        QVERIFY(session->defineTableStyle(QStringLiteral("Mine"), parts).ok);
        QVERIFY(saveThrough(e));
        QCOMPARE(int(dxfEntries(savedPart(e.path, QStringLiteral("xl/styles.xml"))).size()), 3);   // (two rules' looks and the table style's)
        {
            EditWindow again;
            QVERIFY(again.open(fileBytes(e.path)));
            QCOMPARE(looksOf(again), (QList<quint32>{colorOf(0), colorOf(49)}));
            bool mine = false;
            for (const jp::office::sheet::CustomTableStyle &c : again.win.w.session()->workbook()->styles().tableStyles) mine = mine || c.name == QStringLiteral("Mine");
            QVERIFY(mine);
            // the second round, on the saved file
            round(again, 100);
            QVERIFY(saveThrough(again));
            QCOMPARE(int(dxfEntries(savedPart(again.path, QStringLiteral("xl/styles.xml"))).size()), 5);
            EditWindow third;
            QVERIFY(third.open(fileBytes(again.path)));
            QCOMPARE(looksOf(third), (QList<quint32>{colorOf(0), colorOf(49), colorOf(100), colorOf(149)}));
        }
        // the first window goes on after its save: its rules keep their looks, the rules cleared come back by Undo, and the next save writes their looks
        QCOMPARE(looksOf(e), (QList<quint32>{colorOf(0), colorOf(49)}));
        for (int i = 0; i < 5 && session->condRules(0).size() != 50; ++i) e.win.w.undoStack()->undo();   // (the table style, the table, then the clearing)
        QCOMPARE(int(session->condRules(0).size()), 50);
        QVERIFY(saveThrough(e));
        EditWindow undone;
        QVERIFY(undone.open(fileBytes(e.path)));
        QList<quint32> want;
        for (int i = 0; i < 50; ++i) want << colorOf(i);
        std::sort(want.begin(), want.end());
        QCOMPARE(looksOf(undone), want);
    }

    // A color filter keeps the look it names through a save: its differential format is written once and the filter reads back with the same color.
    void aColorFilterKeepsItsLookThroughASave()
    {
        EditWindow e;
        QVERIFY(e.open(condBook()));
        jp::office::sheet::AutoFilter f;
        f.ref = rangeOf(QStringLiteral("A1:C5"));
        jp::office::sheet::FilterColumn c;
        c.kind = jp::office::sheet::FilterColumn::Kind::Color;
        c.column = 0;
        c.color = ColorSpec::fromRgb(0xFF0000);
        f.columns.append(c);
        const crunch::EditSession::Outcome o = e.win.w.session()->setFilter(0, f, QStringLiteral("Filtering"));
        QVERIFY2(o.ok, qPrintable(o.problem));
        QVERIFY(saveThrough(e));
        QCOMPARE(int(dxfEntries(savedPart(e.path, QStringLiteral("xl/styles.xml"))).size()), 1);
        EditWindow again;
        QVERIFY(again.open(fileBytes(e.path)));
        const jp::office::sheet::AutoFilter back = again.win.w.session()->calculation()->autoFilterOf(0);
        QCOMPARE(int(back.columns.size()), 1);
        QVERIFY(back.columns[0].dxf >= 0);
        QVERIFY(again.win.w.session()->workbook()->styles().dxfs[back.columns[0].dxf].fill.bg == ColorSpec::fromRgb(0xFF0000));
    }

    // Two looks that are the same on the screen and in the written bytes are one entry: a solid fill whose color is kept as the foreground and one whose color is kept as the background.
    void looksThatAreWrittenTheSameShareOneEntry()
    {
        EditWindow e;
        QVERIFY(e.open(condBook()));
        DifferentialFormat fg, bg;
        fg.hasFill = bg.hasFill = true;
        fg.fill.pattern = bg.fill.pattern = PatternKind::Solid;
        fg.fill.fg = ColorSpec::fromRgb(0x00AA00);
        bg.fill.bg = ColorSpec::fromRgb(0x00AA00);
        QVERIFY(newRuleWithLook(e, QStringLiteral("A1:A5"), QStringLiteral("2"), fg));
        QVERIFY(newRuleWithLook(e, QStringLiteral("C1:C5"), QStringLiteral("2"), bg));
        QVERIFY(saveThrough(e));
        QCOMPARE(int(dxfEntries(savedPart(e.path, QStringLiteral("xl/styles.xml"))).size()), 1);
    }

    // Every ready-made look is still the ready-made look after the workbook is saved and opened again: the rule's list says "Keep the current look", not "Custom Format".
    void everyReadyMadeLookIsRecognizedAfterASaveAndReopen()
    {
        for (int i = 0; i < condpresets::lookCount(); ++i) {
            EditWindow e;
            QVERIFY(e.open(condBook()));
            QVERIFY(newRuleWithLook(e, QStringLiteral("A1:A5"), QStringLiteral("2"), condpresets::lookOf(i)));
            QVERIFY(saveThrough(e));
            EditWindow again;
            QVERIFY(again.open(fileBytes(e.path)));
            const QVector<CondRule> rules = again.win.w.session()->condRules(0);
            QCOMPARE(int(rules.size()), 1);
            CondSpec spec = specOfRule(rules.first());
            spec.currentLook = again.win.w.session()->condLookOf(rules.first());
            CondRuleDialog dialog(spec, true);
            QVERIFY2(dialog.lookBox()->list()->itemText(0) == QStringLiteral("Keep the current look"), qPrintable(QStringLiteral("look %1 reads back as %2").arg(i).arg(dialog.lookBox()->list()->itemText(0))));
        }
    }

    // The list "Cells look like this" has Custom Format...; choosing it opens the box with the look that was shown, a cancelled box leaves the list where it was, and an answer is the look of the rule
    // and of the sample. A quick rule (Greater Than and the others) takes the look the same way.
    void theLookListOffersCustomFormatAndAsksForTheLook()
    {
        CondRuleDialog dialog(CondSpec(), false);
        CondLookContext context;
        int asked = 0;
        DifferentialFormat got;
        bool answer = false;
        const DifferentialFormat made = everyPartLook();
        context.asker = [&](const DifferentialFormat &start, DifferentialFormat *out) {
            ++asked;
            got = start;
            if (!answer) return false;
            *out = made;
            return true;
        };
        dialog.setLookContext(context);
        QComboBox *list = dialog.lookBox()->list();
        const int custom = list->findData(-2);
        QVERIFY(custom >= 0);
        QCOMPARE(list->itemText(custom), QStringLiteral("Custom Format..."));
        const int before = list->currentIndex();
        list->setCurrentIndex(custom);                 // (cancelled)
        QCOMPARE(asked, 1);
        QVERIFY(sameCondLook(got, condpresets::lookOf(0)));   // (it starts from the look that was shown)
        QCOMPARE(list->currentIndex(), before);
        QVERIFY(!dialog.spec().useCustom);
        answer = true;
        list->setCurrentIndex(custom);
        QCOMPARE(asked, 2);
        QVERIFY(dialog.spec().useCustom);
        QVERIFY(sameCondLook(dialog.spec().custom, made));
        QVERIFY(sameCondLook(dialog.lookBox()->sample()->look(), made));
        QCOMPARE(dialog.lookBox()->sample()->text(), QStringLiteral("1234.50"));   // (the sample shows the number format)
        // picked again, it starts from the person's look
        Q_EMIT list->activated(custom);
        QCOMPARE(asked, 3);
        QVERIFY(sameCondLook(got, made));
        // a quick rule
        EditWindow e;
        QVERIFY(e.open(condBook()));
        selectRange(e, QStringLiteral("A1:A5"));
        e.win.w.setCondAsker([&](CrunchWindow::CondAnswer *a) {
            a->value1 = QStringLiteral("2");
            a->custom = true;
            a->customLook = made;
            return true;
        });
        run(e, "cond.greater");
        QVERIFY(sameCondLook(firstRuleLook(e), made));
    }

    // The Format Cells box for a rule has the Number, Font, Border, and Fill tabs, only the thin kinds of line, and what it answers is merged into the look; a part it leaves alone stays.
    void theFormatBoxForARuleOffersOnlyWhatARuleCarries()
    {
        FormatCellsDialog::Input in;
        in.view = viewOfCondLook(DifferentialFormat(), nullptr);
        in.ruleLook = true;
        in.oneCell = true;
        FormatCellsDialog box(in);
        QTabWidget *tabs = box.findChild<QTabWidget *>(QStringLiteral("formatTabs"));
        QVERIFY(tabs);
        QStringList names;
        for (int i = 0; i < tabs->count(); ++i) names << tabs->tabText(i);
        QCOMPARE(names, QStringList({"Number", "Font", "Border", "Fill"}));
        QListWidget *lines = box.findChild<QListWidget *>(QStringLiteral("lineStyle"));
        QVERIFY(lines);
        int shown = 0;
        for (int i = 0; i < lines->count(); ++i) shown += !lines->item(i)->isHidden();
        QCOMPARE(shown, 7);   // none, hair, dotted, dash-dot-dot, dash-dot, dashed, thin
        // what the box is told to do ends in the look
        box.findChild<QComboBox *>(QStringLiteral("fontStyle"))->setCurrentIndex(3);   // bold italic
        box.findChild<QComboBox *>(QStringLiteral("underline"))->setCurrentIndex(2);   // double
        box.findChild<QCheckBox *>(QStringLiteral("strike"))->setChecked(true);
        const DifferentialFormat made = condLookAfter(DifferentialFormat(), box.result());
        QCOMPARE(made.bold, std::optional<bool>(true));
        QCOMPARE(made.italic, std::optional<bool>(true));
        QCOMPARE(made.strike, std::optional<bool>(true));
        QVERIFY(made.underline == Underline::Double);
        QVERIFY(!made.hasFill && !made.hasBorder && !made.numberFormat);
        // a look started from keeps what the box did not touch
        FormatCellsDialog::Input again = in;
        again.view = viewOfCondLook(everyPartLook(), nullptr);
        FormatCellsDialog second(again);
        second.findChild<QCheckBox *>(QStringLiteral("strike"))->setChecked(false);
        DifferentialFormat after = condLookAfter(everyPartLook(), second.result());
        QVERIFY(!after.strike);
        after.strike = true;
        QVERIFY(sameCondLook(after, everyPartLook()));
    }

    // A rule's look is drawn from every part of it, in priority order, and a rule with Stop If True hides the ones below: measured on the cell's look (what the screen and the print draw from), on
    // the window, and on a PDF of the sheet.
    void everyPartOfARulesLookIsDrawnOnScreenAndInThePdfInPriorityOrder()
    {
        const QString dxfs = QStringLiteral("<dxfs count=\"2\"><dxf><font><b/><i/><u val=\"double\"/><color rgb=\"FF0000FF\"/></font><numFmt numFmtId=\"164\" formatCode=\"0.00\"/>"
                                            "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FF00FF00\"/><bgColor rgb=\"FF00FF00\"/></patternFill></fill>"
                                            "<border><left style=\"thin\"><color rgb=\"FFFF0000\"/></left><right style=\"dotted\"><color rgb=\"FFFF0000\"/></right><top style=\"thin\"><color rgb=\"FFFF0000\"/></top>"
                                            "<bottom style=\"dashed\"><color rgb=\"FFFF0000\"/></bottom></border></dxf>"
                                            "<dxf><font><strike/><color rgb=\"FFFF00FF\"/></font><fill><patternFill patternType=\"solid\"><fgColor rgb=\"FFFFFF00\"/><bgColor rgb=\"FFFFFF00\"/></patternFill></fill></dxf></dxfs>");
        // A1:A3 have the first rule (every part) and the second under it (magenta text, yellow fill): the first rule's color and fill win. A4:A5 have the second alone.
        // C5 has a first rule with Stop If True and a second below it: only the first is drawn.
        const QString tail = QStringLiteral("<conditionalFormatting sqref=\"A1:A3\"><cfRule type=\"cellIs\" dxfId=\"0\" priority=\"1\" operator=\"greaterThan\"><formula>0</formula></cfRule></conditionalFormatting>"
                                            "<conditionalFormatting sqref=\"A1:A5\"><cfRule type=\"cellIs\" dxfId=\"1\" priority=\"2\" operator=\"greaterThan\"><formula>0</formula></cfRule></conditionalFormatting>"
                                            "<conditionalFormatting sqref=\"C5\"><cfRule type=\"cellIs\" dxfId=\"0\" priority=\"3\" stopIfTrue=\"1\" operator=\"greaterThan\"><formula>0</formula></cfRule>"
                                            "<cfRule type=\"cellIs\" dxfId=\"1\" priority=\"4\" operator=\"greaterThan\"><formula>0</formula></cfRule></conditionalFormatting>");
        EditWindow e;
        QVERIFY(e.open(condBook(tail, dxfs)));
        e.win.w.resize(900, 500);
        e.win.w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&e.win.w));
        QVERIFY(e.settle());
        grid::GridSheet &gs = e.view()->sheet();
        gs.conditional().settle();
        auto lookAt = [&](quint32 row, quint32 col) -> const grid::CellLook & {
            const std::optional<grid::Cell> cell = gs.cellAt(row, col);
            return gs.lookOf(row, col, cell ? cell->style : 0);
        };
        const grid::CellLook &a1 = lookAt(0, 0);
        QVERIFY(a1.bold && a1.italic);
        QVERIFY(a1.strike);                                                 // (the second rule adds what the first does not say)
        QVERIFY(a1.underline == Underline::Double);
        QCOMPARE(a1.textColor & 0xFFFFFF, 0x0000FFu);                       // (the first rule's color, not the second's)
        QCOMPARE(a1.fg & 0xFFFFFF, 0x00FF00u);                              // (the first rule's fill)
        QVERIFY(a1.left.style != BorderStyle::None && a1.right.style != BorderStyle::None && a1.top.style != BorderStyle::None && a1.bottom.style != BorderStyle::None);
        QVERIFY(a1.hasFormat);
        const std::optional<grid::Cell> a1Cell = gs.cellAt(0, 0);
        QVERIFY(a1Cell);
        QCOMPARE(gs.textOf(0, 0, *a1Cell), QStringLiteral("1.00"));         // (the rule's number format is what the cell shows)
        const std::optional<grid::Cell> a4Cell = gs.cellAt(3, 0);
        QVERIFY(a4Cell);
        QVERIFY(gs.textOf(3, 0, *a4Cell) != QStringLiteral("4.00"));        // (and a cell the rule is not for shows its own)
        const grid::CellLook &a4 = lookAt(3, 0);
        QCOMPARE(a4.textColor & 0xFFFFFF, 0xFF00FFu);                       // (the second rule alone)
        QCOMPARE(a4.fg & 0xFFFFFF, 0xFFFF00u);
        QVERIFY(!a4.bold && !a4.hasFormat && a4.strike);
        const grid::CellLook &c5 = lookAt(4, 2);
        QCOMPARE(c5.textColor & 0xFFFFFF, 0x0000FFu);                       // (Stop If True: the rule below is not added)
        QCOMPARE(c5.fg & 0xFFFFFF, 0x00FF00u);
        QVERIFY(!c5.strike);
        auto count = [](const QImage &img, quint32 rgb) {
            int n = 0;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x) {
                    const QRgb p = img.pixel(x, y);
                    n += qAbs(qRed(p) - int((rgb >> 16) & 255)) < 12 && qAbs(qGreen(p) - int((rgb >> 8) & 255)) < 12 && qAbs(qBlue(p) - int(rgb & 255)) < 12;
                }
            return n;
        };
        // the window, grabbed
        const QImage shot = e.win.w.grab().toImage().convertToFormat(QImage::Format_RGB32);
        QVERIFY2(count(shot, 0x00FF00) > 40, "the first rule's fill is not on the screen");
        QVERIFY2(count(shot, 0xFFFF00) > 20, "the second rule's fill is not on the screen");
        // the PDF
        const QString path = e.folder.dir.filePath(QStringLiteral("looks.pdf"));
        crunch::PrintJobOptions options;
        QString why;
        QVERIFY2(e.win.w.exportPdfTo(path, options, QString(), &why), qPrintable(why));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const jp::PdfDocument pdf(f.readAll());
        QVERIFY(pdf.isValid());
        const QImage page = pdf.render(0, QSize(612, 792)).convertToFormat(QImage::Format_RGB32);
        QVERIFY2(count(page, 0x00FF00) > 20, "the first rule's fill is not on the printed page");
        QVERIFY2(count(page, 0xFFFF00) > 10, "the second rule's fill is not on the printed page");
        QVERIFY2(count(page, 0xFF0000) > 10, "the rule's lines are not on the printed page");
        QVERIFY2(count(page, 0x0000FF) > 5, "the rule's font color is not on the printed page");
        const QString words = pdf.text(0);
        QVERIFY2(words.contains(QStringLiteral("1.00")) && words.contains(QStringLiteral("3.00")), qPrintable(words));   // (the rule's number format is what is printed)
    }

    // A protected sheet that does not allow formatting refuses a rule with a look of its own, as it refuses every other rule change, and the refusal leaves nothing to undo.
    void aProtectedSheetRefusesACustomLook()
    {
        EditWindow e;
        QVERIFY(e.open(condBook(QStringLiteral("<sheetProtection sheet=\"1\"/>"))));
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(newRuleWithLook(e, QStringLiteral("A1:A5"), QStringLiteral("2"), everyPartLook()));
        QVERIFY2(e.win.told.join('|').contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(int(e.win.w.session()->condRules(0).size()), 0);
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

    // Pictures for the docs, when JC_SHOT_DIR names a folder: the rule window with Custom Format chosen, the box for a rule on each tab, and Manage Rules with a rule that has a look of its own.
    void picturesOfTheCustomLook()
    {
        const QString dir = QString::fromLocal8Bit(qgetenv("JC_SHOT_DIR"));
        if (dir.isEmpty()) QSKIP("JC_SHOT_DIR names the folder for the pictures");
        jp::applyUiTheme(1);
        CondSpec spec;
        spec.v1 = QStringLiteral("100");
        spec.useCustom = true;
        spec.custom = everyPartLook();
        CondRuleDialog window(spec, false);
        window.show();
        window.grab().save(dir + QStringLiteral("/rule-custom-format.png"));
        FormatCellsDialog::Input in;
        in.view = viewOfCondLook(everyPartLook(), nullptr);
        in.ruleLook = true;
        in.oneCell = true;
        FormatCellsDialog box(in);
        box.show();
        const char *tabs[] = {"box-number.png", "box-font.png", "box-border.png", "box-fill.png"};
        QTabWidget *t = box.findChild<QTabWidget *>(QStringLiteral("formatTabs"));
        for (int i = 0; i < 4; ++i) {
            t->setCurrentIndex(i);
            box.grab().save(dir + QLatin1Char('/') + QLatin1String(tabs[i]));
        }
        QVector<CondManageDialog::Item> items;
        CondManageDialog::Item item;
        item.rule = rule(CondRule::Type::CellIs, "A1:A5");
        item.rule.op = CondRule::Operator::GreaterThan;
        item.rule.formula1 = QStringLiteral("100");
        item.look = everyPartLook();
        items.append(item);
        CondManageDialog manage(items, nullptr);
        manage.show();
        manage.grab().save(dir + QStringLiteral("/manage-custom-look.png"));
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
