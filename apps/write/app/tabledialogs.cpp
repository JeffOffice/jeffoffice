#include "app/tabledialogs.h"

#include "app/settings.h"
#include "app/widgets.h"
#include "core/styleops.h"
#include "core/writedoc.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHash>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QRegularExpression>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextCursor>
#include <QVBoxLayout>
#include <memory>

namespace jp::write {

namespace {

QString tr(const char *text) { return QCoreApplication::translate("jp::write::TableDialogs", text); }

// A length field as the file can hold the length: from -50,000 to 50,000 points (a table's indent can be negative), or from 0 for one that is not.
void lengthRange(MeasureSpin *spin, bool negative)
{
    spin->setRange(negative ? -50000 : 0, 50000);
}

QDialogButtonBox *okCancel(QDialog *dlg, QVBoxLayout *v)
{
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlg);
    v->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
    return buttons;
}

} // namespace

bool insertTableDialog(QWidget *parent, NewTable *table)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("insertTableDialog"));
    dlg.setWindowTitle(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Insert Table")));
    auto *v = new QVBoxLayout(&dlg);
    auto *size = new QGroupBox(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Table size")), &dlg);
    auto *form = new QFormLayout(size);
    auto *columns = new QSpinBox(size), *rows = new QSpinBox(size);
    columns->setObjectName(QStringLiteral("columns"));
    rows->setObjectName(QStringLiteral("rows"));
    columns->setRange(1, 63);
    rows->setRange(1, 500);
    columns->setValue(Settings::get().value(QStringLiteral("write/table/columns"), table->columns).toInt());
    rows->setValue(Settings::get().value(QStringLiteral("write/table/rows"), table->rows).toInt());
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Number of columns:")), columns);
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Number of rows:")), rows);
    v->addWidget(size);
    auto *fit = new QGroupBox(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Fit behavior")), &dlg);
    auto *fv = new QVBoxLayout(fit);
    auto *fixed = new QRadioButton(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Fixed column width")), fit), *contents = new QRadioButton(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Fit to contents")), fit), *window = new QRadioButton(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Fit to window")), fit);
    fixed->setObjectName(QStringLiteral("fixedWidth"));
    contents->setObjectName(QStringLiteral("fitContents"));
    window->setObjectName(QStringLiteral("fitWindow"));
    auto *width = new MeasureSpin(fit);
    width->setObjectName(QStringLiteral("columnWidth"));
    width->setPoints(0);
    width->setSpecialValueText(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Auto")));
    width->setMinimum(0);
    auto *fixedRow = new QHBoxLayout;
    fixedRow->addWidget(fixed);
    fixedRow->addWidget(width);
    fv->addLayout(fixedRow);
    fv->addWidget(contents);
    fv->addWidget(window);
    const int was = Settings::get().value(QStringLiteral("write/table/fit"), int(table->fit)).toInt();
    (was == int(TableFit::Contents) ? contents : was == int(TableFit::Window) ? window : fixed)->setChecked(true);
    v->addWidget(fit);
    auto *remember = new QCheckBox(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Remember dimensions for new tables")), &dlg);
    remember->setObjectName(QStringLiteral("remember"));
    v->addWidget(remember);
    okCancel(&dlg, v);
    if (dlg.exec() != QDialog::Accepted) return false;
    table->columns = columns->value();
    table->rows = rows->value();
    table->fit = contents->isChecked() ? TableFit::Contents : window->isChecked() ? TableFit::Window : TableFit::Fixed;
    table->columnWidth = fixed->isChecked() ? width->value() : 0;
    if (remember->isChecked()) {
        Settings::get().setValue(QStringLiteral("write/table/columns"), table->columns);
        Settings::get().setValue(QStringLiteral("write/table/rows"), table->rows);
        Settings::get().setValue(QStringLiteral("write/table/fit"), int(table->fit));
    }
    return true;
}

bool splitCellsDialog(QWidget *parent, int *columns, int *rows, int mostRows)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("splitCellsDialog"));
    dlg.setWindowTitle(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Split Cells")));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;
    v->addLayout(form);
    auto *c = new QSpinBox(&dlg), *r = new QSpinBox(&dlg);
    c->setObjectName(QStringLiteral("columns"));
    r->setObjectName(QStringLiteral("rows"));
    c->setRange(1, 63);
    r->setRange(1, mostRows);
    c->setValue(*columns);
    r->setValue(*rows);
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Number of columns:")), c);
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Number of rows:")), r);
    okCancel(&dlg, v);
    if (dlg.exec() != QDialog::Accepted) return false;
    *columns = c->value();
    *rows = r->value();
    return true;
}

bool tablePropertiesDialog(QWidget *parent, WriteDoc *doc, QTextCursor *cursor)
{
    if (!doc || !cursor) return false;
    const TableProps was = tablePropsAt(*doc, *cursor);
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("tablePropertiesDialog"));
    dlg.setWindowTitle(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Table Properties")));
    auto *v = new QVBoxLayout(&dlg);
    auto *tabs = new QTabWidget(&dlg);
    v->addWidget(tabs);

    // ---- Table
    auto *tablePage = new QWidget;
    auto *tf = new QFormLayout(tablePage);
    auto *align = new QComboBox(tablePage);
    align->setObjectName(QStringLiteral("alignment"));
    align->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Left")), int(flow::TableData::Align::Left));
    align->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Center")), int(flow::TableData::Align::Center));
    align->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Right")), int(flow::TableData::Align::Right));
    align->setCurrentIndex(int(was.align));
    tf->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Alignment:")), align);
    // What each length box held when the dialog opened, to write only the ones the person changed (a box holds a length to a precision, and one the file has outside its range is
    // not made to fit it).
    QHash<const QDoubleSpinBox *, double> shown;
    auto set = [&](MeasureSpin *spin, double points, bool negative) {
        lengthRange(spin, negative);
        spin->setPoints(points);
        shown.insert(spin, spin->value());
    };
    auto took = [&](const QDoubleSpinBox *spin, double was) { return spin->value() == shown.value(spin) ? was : spin->value(); };
    auto *indent = new MeasureSpin(tablePage);
    indent->setObjectName(QStringLiteral("indent"));
    set(indent, was.indent, true);
    tf->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Indent from left:")), indent);
    auto *widthKind = new QComboBox(tablePage);
    widthKind->setObjectName(QStringLiteral("widthKind"));
    widthKind->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Automatic")), int(TableProps::Width::Auto));
    widthKind->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Points")), int(TableProps::Width::Points));
    widthKind->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Percent")), int(TableProps::Width::Percent));
    widthKind->setCurrentIndex(int(was.widthKind));
    auto *widthPoints = new MeasureSpin(tablePage);
    widthPoints->setObjectName(QStringLiteral("widthPoints"));
    set(widthPoints, was.widthKind == TableProps::Width::Points ? was.width : 468, false);
    auto *widthPercent = new QDoubleSpinBox(tablePage);
    widthPercent->setObjectName(QStringLiteral("widthPercent"));
    widthPercent->setRange(0, 20000);
    widthPercent->setSuffix(QStringLiteral("%"));
    widthPercent->setValue(was.widthKind == TableProps::Width::Percent ? was.width : 100);
    shown.insert(widthPercent, widthPercent->value());
    auto *widthRow = new QHBoxLayout;
    widthRow->addWidget(widthKind);
    widthRow->addWidget(widthPoints);
    widthRow->addWidget(widthPercent);
    tf->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Preferred width:")), widthRow);
    auto showWidth = [=] {
        const int k = widthKind->currentData().toInt();
        widthPoints->setVisible(k == int(TableProps::Width::Points));
        widthPercent->setVisible(k == int(TableProps::Width::Percent));
    };
    showWidth();
    QObject::connect(widthKind, &QComboBox::currentIndexChanged, &dlg, showWidth);
    auto *fixedLayout = new QCheckBox(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Fixed column widths")), tablePage);
    fixedLayout->setObjectName(QStringLiteral("fixedLayout"));
    fixedLayout->setChecked(was.fixedLayout);
    tf->addRow(QString(), fixedLayout);
    auto *top = new MeasureSpin(tablePage), *left = new MeasureSpin(tablePage), *bottom = new MeasureSpin(tablePage), *right = new MeasureSpin(tablePage);
    top->setObjectName(QStringLiteral("marginTop"));
    left->setObjectName(QStringLiteral("marginLeft"));
    bottom->setObjectName(QStringLiteral("marginBottom"));
    right->setObjectName(QStringLiteral("marginRight"));
    set(top, was.margins.top(), true);
    set(left, was.margins.left(), true);
    set(bottom, was.margins.bottom(), true);
    set(right, was.margins.right(), true);
    tf->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Default cell margin, top:")), top);
    tf->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Default cell margin, left:")), left);
    tf->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Default cell margin, bottom:")), bottom);
    tf->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Default cell margin, right:")), right);
    auto *spacing = new MeasureSpin(tablePage);
    spacing->setObjectName(QStringLiteral("spacing"));
    set(spacing, was.spacing, false);
    tf->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Space between cells:")), spacing);
    tabs->addTab(tablePage, tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Table")));

    // ---- Row
    auto *rowPage = new QWidget;
    auto *rf = new QFormLayout(rowPage);
    auto *height = new MeasureSpin(rowPage);
    height->setObjectName(QStringLiteral("rowHeight"));
    set(height, was.rowHeight, false);
    auto *rule = new QComboBox(rowPage);
    rule->setObjectName(QStringLiteral("rowRule"));
    rule->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "At least")), false);
    rule->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Exactly")), true);
    rule->setCurrentIndex(was.rowExact ? 1 : 0);
    auto *heightRow = new QHBoxLayout;
    heightRow->addWidget(height);
    heightRow->addWidget(rule);
    rf->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Row height (0 for as tall as the cells need):")), heightRow);
    auto *cantSplit = new QCheckBox(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Do not let the row break across pages")), rowPage);
    cantSplit->setObjectName(QStringLiteral("cantSplit"));
    cantSplit->setChecked(was.cantSplit);
    rf->addRow(QString(), cantSplit);
    auto *header = new QCheckBox(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Repeat as header row at the top of each page")), rowPage);
    header->setObjectName(QStringLiteral("headerRow"));
    header->setChecked(was.header);
    rf->addRow(QString(), header);
    tabs->addTab(rowPage, tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Row")));

    // ---- Column
    auto *columnPage = new QWidget;
    auto *cf = new QFormLayout(columnPage);
    auto *columnWidth = new MeasureSpin(columnPage);
    columnWidth->setObjectName(QStringLiteral("columnWidth"));
    set(columnWidth, was.columnWidth, false);
    cf->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Column width:")), columnWidth);
    // The same width as a share of the table's (the two follow each other).
    auto *columnPercent = new QDoubleSpinBox(columnPage);
    columnPercent->setObjectName(QStringLiteral("columnPercent"));
    columnPercent->setRange(0, 100);
    columnPercent->setDecimals(1);
    columnPercent->setSuffix(QStringLiteral(" %"));
    const double whole = was.gridWidth;
    columnPercent->setEnabled(whole > 0);
    columnPercent->setValue(whole > 0 ? 100.0 * was.columnWidth / whole : 0);
    cf->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Column width as a percent of the table:")), columnPercent);
    if (whole > 0) {
        auto syncing = std::make_shared<bool>(false);
        QObject::connect(columnPercent, &QDoubleSpinBox::valueChanged, columnPage, [=](double percent) {
            if (*syncing) return;
            *syncing = true;
            columnWidth->setPoints(std::round(percent * whole) / 100.0);
            *syncing = false;
        });
        QObject::connect(columnWidth, &QDoubleSpinBox::valueChanged, columnPage, [=] {
            if (*syncing) return;
            *syncing = true;
            columnPercent->setValue(100.0 * columnWidth->value() / whole);
            *syncing = false;
        });
    }
    tabs->addTab(columnPage, tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Column")));

    // ---- Cell
    auto *cellPage = new QWidget;
    auto *cl = new QFormLayout(cellPage);
    auto *valign = new QComboBox(cellPage);
    valign->setObjectName(QStringLiteral("verticalAlignment"));
    valign->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Top")), int(flow::CellAlign::Top));
    valign->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Center")), int(flow::CellAlign::Center));
    valign->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Bottom")), int(flow::CellAlign::Bottom));
    valign->setCurrentIndex(int(was.valign));
    cl->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Vertical alignment:")), valign);
    auto *noWrap = new QCheckBox(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Do not wrap text")), cellPage);
    noWrap->setObjectName(QStringLiteral("noWrap"));
    noWrap->setChecked(was.noWrap);
    cl->addRow(QString(), noWrap);
    tabs->addTab(cellPage, tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Cell")));

    // ---- Alt Text
    auto *altPage = new QWidget;
    auto *af = new QFormLayout(altPage);
    auto *title = new QLineEdit(was.altTitle, altPage), *text = new QLineEdit(was.altText, altPage);
    title->setObjectName(QStringLiteral("altTitle"));
    text->setObjectName(QStringLiteral("altText"));
    af->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Title:")), title);
    af->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Description:")), text);
    tabs->addTab(altPage, tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Alt Text")));

    okCancel(&dlg, v);
    if (dlg.exec() != QDialog::Accepted) return false;
    TableProps now = was;
    now.align = flow::TableData::Align(align->currentData().toInt());
    now.indent = took(indent, was.indent);
    now.widthKind = TableProps::Width(widthKind->currentData().toInt());
    now.width = now.widthKind == TableProps::Width::Points ? took(widthPoints, was.widthKind == now.widthKind ? was.width : widthPoints->value())
                : now.widthKind == TableProps::Width::Percent ? took(widthPercent, was.widthKind == now.widthKind ? was.width : widthPercent->value()) : 0;
    now.fixedLayout = fixedLayout->isChecked();
    now.margins = QMarginsF(took(left, was.margins.left()), took(top, was.margins.top()), took(right, was.margins.right()), took(bottom, was.margins.bottom()));
    now.spacing = took(spacing, was.spacing);
    now.rowHeight = took(height, was.rowHeight);
    now.rowExact = rule->currentData().toBool();
    now.cantSplit = cantSplit->isChecked();
    now.header = header->isChecked();
    now.columnWidth = took(columnWidth, was.columnWidth);
    now.valign = flow::CellAlign(valign->currentData().toInt());
    now.noWrap = noWrap->isChecked();
    now.altTitle = title->text();
    now.altText = text->text();
    if (now == was) return true;
    setTableProps(*doc, *cursor, was, now);
    return true;
}

bool bordersShadingDialog(QWidget *parent, WriteDoc *doc, QTextCursor *cursor)
{
    if (!doc || !cursor) return false;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("bordersShadingDialog"));
    dlg.setWindowTitle(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Borders and Shading")));
    auto *v = new QVBoxLayout(&dlg);
    auto *tabs = new QTabWidget(&dlg);
    v->addWidget(tabs);

    auto *bordersPage = new QWidget;
    auto *bl = new QGridLayout(bordersPage);
    struct SideBox {
        const char *name, *label;
        int side;
    };
    const SideBox sideBoxes[] = {{"top", QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Top"), BorderSide::Top},
                                 {"bottom", QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Bottom"), BorderSide::Bottom},
                                 {"left", QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Left"), BorderSide::Left},
                                 {"right", QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Right"), BorderSide::Right},
                                 {"insideH", QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Between rows"), BorderSide::InsideH},
                                 {"insideV", QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Between columns"), BorderSide::InsideV}};
    auto *sides = new QGroupBox(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Lines on")), bordersPage);
    auto *sv = new QVBoxLayout(sides);
    QVector<QCheckBox *> boxes;
    for (const SideBox &s : sideBoxes) {
        auto *box = new QCheckBox(QCoreApplication::translate("jp::write::TableDialogs", s.label), sides);
        box->setObjectName(QString::fromLatin1(s.name));
        box->setProperty("side", s.side);
        sv->addWidget(box);
        boxes << box;
    }
    boxes[0]->setChecked(true);
    boxes[1]->setChecked(true);
    boxes[2]->setChecked(true);
    boxes[3]->setChecked(true);
    bl->addWidget(sides, 0, 0, 2, 1);
    auto *style = new QComboBox(bordersPage);
    style->setObjectName(QStringLiteral("lineStyle"));
    style->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "None")), int(flow::TableLine::Style::None));
    style->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Single")), int(flow::TableLine::Style::Single));
    style->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Double")), int(flow::TableLine::Style::Double));
    style->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Dotted")), int(flow::TableLine::Style::Dotted));
    style->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Dashed")), int(flow::TableLine::Style::Dashed));
    style->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Triple")), int(flow::TableLine::Style::Triple));
    style->setCurrentIndex(1);
    auto *widthBox = new QComboBox(bordersPage);
    widthBox->setObjectName(QStringLiteral("lineWidth"));
    for (double w : {0.25, 0.5, 0.75, 1.0, 1.5, 2.25, 3.0, 4.5, 6.0}) widthBox->addItem(QStringLiteral("%1 pt").arg(w), w);
    widthBox->setCurrentIndex(1);
    auto *color = new ColorButton(QStringLiteral("baseline"), tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Line color")), true, tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Automatic")), bordersPage);
    color->setObjectName(QStringLiteral("lineColor"));
    color->setWell(true);
    auto *lf = new QFormLayout;
    lf->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Style:")), style);
    lf->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Width:")), widthBox);
    lf->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Color:")), color);
    bl->addLayout(lf, 0, 1);
    tabs->addTab(bordersPage, tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Borders")));

    auto *shadingPage = new QWidget;
    auto *shl = new QFormLayout(shadingPage);
    auto *fill = new ColorButton(QStringLiteral("paint-bucket"), tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Fill")), true, tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "No Color")), shadingPage);
    fill->setObjectName(QStringLiteral("fillColor"));
    fill->setWell(true);
    auto *useFill = new QCheckBox(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Change the fill of the cells")), shadingPage);
    useFill->setObjectName(QStringLiteral("changeFill"));
    shl->addRow(QString(), useFill);
    shl->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Fill:")), fill);
    tabs->addTab(shadingPage, tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Shading")));

    auto *applyTo = new QComboBox(&dlg);
    applyTo->setObjectName(QStringLiteral("applyTo"));
    applyTo->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Selected cells")), 0);
    applyTo->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Table")), 1);
    auto *applyRow = new QFormLayout;
    applyRow->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Apply to:")), applyTo);
    v->addLayout(applyRow);
    okCancel(&dlg, v);
    if (dlg.exec() != QDialog::Accepted) return false;

    QTextCursor target = *cursor;
    if (applyTo->currentData().toInt() == 1) selectInTable(*doc, target, TableUnit::Table);
    const QColor lineColor = color->current().isNone() ? QColor() : color->current().resolve(doc->flow().env.colors);
    flow::TableLine line;
    line.style = flow::TableLine::Style(style->currentData().toInt());
    line.width = widthBox->currentData().toDouble();
    line.color = lineColor;
    int chosen = 0;
    for (QCheckBox *b : std::as_const(boxes))
        if (b->isChecked()) chosen |= b->property("side").toInt();
    // (One step for both.)
    target.beginEditBlock();
    if (chosen) setBorders(*doc, target, chosen, line);
    if (useFill->isChecked()) setShading(*doc, target, fill->current().isNone() ? QColor() : fill->current().resolve(doc->flow().env.colors));
    target.endEditBlock();
    return true;
}

namespace {

// The choices of what separates cells in text, as radio buttons in a row; `paragraphs` leaves out (or puts in) the one that makes each paragraph a cell.
struct SeparatorChoice {
    QRadioButton *paragraphs, *tabs, *commas, *other;
    QLineEdit *character;
    TextSeparator value() const
    {
        return paragraphs->isChecked() ? TextSeparator::Paragraphs : commas->isChecked() ? TextSeparator::Commas : other->isChecked() ? TextSeparator::Other : TextSeparator::Tabs;
    }
};

SeparatorChoice addSeparatorChoice(QDialog *dlg, QVBoxLayout *v, bool withParagraphs, TextSeparator start, QChar startOther)
{
    auto *box = new QGroupBox(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Separate text at")), dlg);
    auto *h = new QHBoxLayout(box);
    SeparatorChoice c;
    c.paragraphs = new QRadioButton(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Paragraphs")), box);
    c.tabs = new QRadioButton(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Tabs")), box);
    c.commas = new QRadioButton(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Commas")), box);
    c.other = new QRadioButton(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Other:")), box);
    c.character = new QLineEdit(box);
    c.paragraphs->setObjectName(QStringLiteral("paragraphs"));
    c.tabs->setObjectName(QStringLiteral("tabs"));
    c.commas->setObjectName(QStringLiteral("commas"));
    c.other->setObjectName(QStringLiteral("other"));
    c.character->setObjectName(QStringLiteral("character"));
    c.character->setMaxLength(1);
    c.character->setFixedWidth(40);
    if (!startOther.isNull()) c.character->setText(QString(startOther));
    h->addWidget(c.paragraphs);
    h->addWidget(c.tabs);
    h->addWidget(c.commas);
    h->addWidget(c.other);
    h->addWidget(c.character);
    h->addStretch(1);
    c.paragraphs->setVisible(withParagraphs);
    (start == TextSeparator::Paragraphs && withParagraphs ? c.paragraphs : start == TextSeparator::Commas ? c.commas : start == TextSeparator::Other ? c.other : c.tabs)->setChecked(true);
    c.character->setEnabled(c.other->isChecked());
    QObject::connect(c.other, &QRadioButton::toggled, c.character, &QWidget::setEnabled);
    v->addWidget(box);
    return c;
}

} // namespace

bool convertTextToTableDialog(QWidget *parent, TextSeparator *separator, QChar *other, int *columns)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("convertTextToTableDialog"));
    dlg.setWindowTitle(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Convert Text to Table")));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;
    v->addLayout(form);
    auto *c = new QSpinBox(&dlg);
    c->setObjectName(QStringLiteral("columns"));
    c->setRange(1, 63);
    c->setValue(std::max(1, *columns));
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Number of columns:")), c);
    const SeparatorChoice choice = addSeparatorChoice(&dlg, v, true, *separator, *other);
    okCancel(&dlg, v);
    if (dlg.exec() != QDialog::Accepted) return false;
    *separator = choice.value();
    *other = choice.character->text().isEmpty() ? QChar() : choice.character->text().at(0);
    *columns = c->value();
    return true;
}

bool convertTableToTextDialog(QWidget *parent, TextSeparator *separator, QChar *other)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("convertTableToTextDialog"));
    dlg.setWindowTitle(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Convert Table to Text")));
    auto *v = new QVBoxLayout(&dlg);
    const SeparatorChoice choice = addSeparatorChoice(&dlg, v, true, *separator, *other);
    choice.paragraphs->setText(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Paragraph marks")));
    okCancel(&dlg, v);
    if (dlg.exec() != QDialog::Accepted) return false;
    *separator = choice.value();
    *other = choice.character->text().isEmpty() ? QChar() : choice.character->text().at(0);
    return true;
}

bool sortTableDialog(QWidget *parent, int columns, QVector<TableSortKey> *keys, bool *header)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("sortTableDialog"));
    dlg.setWindowTitle(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Sort")));
    auto *v = new QVBoxLayout(&dlg);
    auto *grid = new QGridLayout;
    v->addLayout(grid);
    struct Row {
        QComboBox *column, *kind;
        QRadioButton *up, *down;
    };
    QVector<Row> rows;
    const char *const names[] = {QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Sort by"), QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Then by"), QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Then by")};
    for (int i = 0; i < 3; ++i) {
        Row r;
        r.column = new QComboBox(&dlg);
        r.column->setObjectName(QStringLiteral("column%1").arg(i));
        if (i > 0) r.column->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "(none)")), -1);
        for (int c = 0; c < columns; ++c) r.column->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Column %1")).arg(c + 1), c);
        r.kind = new QComboBox(&dlg);
        r.kind->setObjectName(QStringLiteral("kind%1").arg(i));
        r.kind->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Text")), int(TableSortKey::Kind::Text));
        r.kind->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Number")), int(TableSortKey::Kind::Number));
        r.kind->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Date")), int(TableSortKey::Kind::Date));
        r.up = new QRadioButton(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Ascending")), &dlg);
        r.down = new QRadioButton(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Descending")), &dlg);
        r.up->setObjectName(QStringLiteral("up%1").arg(i));
        r.down->setObjectName(QStringLiteral("down%1").arg(i));
        r.up->setChecked(true);
        if (i < keys->size()) {
            r.column->setCurrentIndex(std::max(0, r.column->findData((*keys)[i].column)));
            r.kind->setCurrentIndex(std::max(0, r.kind->findData(int((*keys)[i].kind))));
            (*keys)[i].ascending ? r.up->setChecked(true) : r.down->setChecked(true);
        }
        auto *group = new QButtonGroup(&dlg);
        group->addButton(r.up);
        group->addButton(r.down);
        grid->addWidget(new QLabel(tr(names[i]), &dlg), i, 0);
        grid->addWidget(r.column, i, 1);
        grid->addWidget(r.kind, i, 2);
        grid->addWidget(r.up, i, 3);
        grid->addWidget(r.down, i, 4);
        rows << r;
    }
    auto *head = new QCheckBox(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "The first row is a header row (it stays where it is)")), &dlg);
    head->setObjectName(QStringLiteral("header"));
    head->setChecked(*header);
    v->addWidget(head);
    okCancel(&dlg, v);
    if (dlg.exec() != QDialog::Accepted) return false;
    keys->clear();
    for (const Row &r : std::as_const(rows)) {
        const int column = r.column->currentData().toInt();
        if (column < 0) continue;
        TableSortKey k;
        k.column = column;
        k.kind = TableSortKey::Kind(r.kind->currentData().toInt());
        k.ascending = r.up->isChecked();
        keys->push_back(k);
    }
    *header = head->isChecked();
    return !keys->isEmpty();
}

bool tableFormulaDialog(QWidget *parent, QString *formula, QString *picture)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("tableFormulaDialog"));
    dlg.setWindowTitle(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Formula")));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;
    v->addLayout(form);
    auto *f = new QLineEdit(*formula, &dlg);
    f->setObjectName(QStringLiteral("formula"));
    auto *p = new QComboBox(&dlg);
    p->setObjectName(QStringLiteral("picture"));
    p->setEditable(true);
    p->addItems({QString(), QStringLiteral("0"), QStringLiteral("0.00"), QStringLiteral("#,##0"), QStringLiteral("#,##0.00"), QStringLiteral("$#,##0.00"), QStringLiteral("0%"), QStringLiteral("0.00%")});
    p->setCurrentText(*picture);
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Formula:")), f);
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Number format:")), p);
    auto *hint = new QLabel(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Use cells as A1 (a column's letter and a row's number), A1:B3, or ABOVE, BELOW, LEFT, and RIGHT, in SUM, AVERAGE, COUNT, MIN, MAX, and the like.")), &dlg);
    hint->setWordWrap(true);
    v->addWidget(hint);
    okCancel(&dlg, v);
    if (dlg.exec() != QDialog::Accepted) return false;
    *formula = f->text().trimmed();
    *picture = p->currentText().trimmed();
    return !formula->isEmpty();
}

namespace {

QString hexOf(const ColorButton *b, WriteDoc &doc)
{
    if (b->current().isNone()) return QString();
    return b->current().resolve(doc.flow().env.colors).name().mid(1).toUpper();
}

void setHex(ColorButton *b, const QString &hex)
{
    static const QRegularExpression valid(QStringLiteral("^[0-9A-Fa-f]{6}$"));
    b->setCurrent(valid.match(hex).hasMatch() ? ColorRef::rgb(QColor(QLatin1Char('#') + hex)) : ColorRef::none());
}

} // namespace

bool tableStyleDialog(QWidget *parent, WriteDoc *doc, const QString &modifyId, QString *madeId)
{
    if (!doc) return false;
    const bool modify = !modifyId.isEmpty();
    office::wml::Style existing;
    if (modify) {
        if (const office::wml::Style *s = doc->styleSheet().find(modifyId)) existing = *s;
        else existing = office::wml::builtinStyle(modifyId);
        if (existing.id.isEmpty() || existing.type != QLatin1String("table")) return false;
    }
    const office::wml::TableStyleSpec start = modify ? office::wml::tableStyleSpecOf(existing) : office::wml::TableStyleSpec();
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("tableStyleDialog"));
    dlg.setWindowTitle(modify ? tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Modify Table Style")) : tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "New Table Style")));
    auto *v = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;
    v->addLayout(form);
    auto *name = new QLineEdit(start.name, &dlg);
    name->setObjectName(QStringLiteral("styleName"));
    name->setMaxLength(kMostStyleNameLength);
    name->setEnabled(!modify || tableStyleNameIsEditable(*doc, modifyId));   // (a built-in style keeps its name)
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Name:")), name);
    auto *basedOn = new QComboBox(&dlg);
    basedOn->setObjectName(QStringLiteral("basedOn"));
    QStringList ids{QStringLiteral("TableNormal")};
    for (const QString &id : office::wml::builtinTableStyleIds()) ids << id;
    for (const office::wml::Style &s : doc->styleSheet().styles)
        if (s.type == QLatin1String("table") && !ids.contains(s.id)) ids << s.id;
    for (const QString &id : std::as_const(ids)) {
        if (id == modifyId) continue;
        const office::wml::Style *own = doc->styleSheet().find(id);
        const office::wml::Style s = own ? *own : office::wml::builtinStyle(id);
        basedOn->addItem(s.name.isEmpty() ? id : s.name, id);
    }
    basedOn->setCurrentIndex(std::max(0, basedOn->findData(start.basedOn)));
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Based on:")), basedOn);
    auto *lines = new QComboBox(&dlg);
    lines->setObjectName(QStringLiteral("lines"));
    lines->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "No lines")), int(office::wml::TableStyleSpec::NoLines));
    lines->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Outside lines only")), int(office::wml::TableStyleSpec::Box));
    lines->addItem(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "All lines")), int(office::wml::TableStyleSpec::Grid));
    lines->setCurrentIndex(int(start.lines));
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Lines:")), lines);
    auto *width = new QComboBox(&dlg);
    width->setObjectName(QStringLiteral("lineWidth"));
    for (int eighths : {2, 4, 6, 8, 12, 18, 24, 36, 48}) width->addItem(QStringLiteral("%1 pt").arg(eighths / 8.0), eighths);
    if (width->findData(start.lineWidth) < 0) width->addItem(QStringLiteral("%1 pt").arg(start.lineWidth / 8.0), start.lineWidth);
    width->setCurrentIndex(width->findData(start.lineWidth));
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Line width:")), width);
    auto makeColor = [&](const char *objectName, const QString &label, const QString &none) {
        auto *b = new ColorButton(QStringLiteral("paint-bucket"), label, true, none, &dlg);
        b->setObjectName(QLatin1String(objectName));
        b->setWell(true);
        return b;
    };
    auto *lineColor = makeColor("lineColor", tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Line color")), tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Automatic")));
    setHex(lineColor, start.lineColor);
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Line color:")), lineColor);
    auto *headerBold = new QCheckBox(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Bold")), &dlg);
    headerBold->setObjectName(QStringLiteral("headerBold"));
    headerBold->setChecked(start.headerBold);
    auto *headerFill = makeColor("headerFill", tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Header row fill")), tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "No Color")));
    setHex(headerFill, start.headerFill);
    auto *headerText = makeColor("headerText", tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Header row text color")), tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Automatic")));
    setHex(headerText, start.headerText);
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Header row:")), headerBold);
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Header row fill:")), headerFill);
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Header row text:")), headerText);
    auto *totalBold = new QCheckBox(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Bold total row")), &dlg);
    totalBold->setObjectName(QStringLiteral("totalBold"));
    totalBold->setChecked(start.totalBold);
    auto *firstBold = new QCheckBox(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Bold first column")), &dlg);
    firstBold->setObjectName(QStringLiteral("firstColumnBold"));
    firstBold->setChecked(start.firstColumnBold);
    auto *lastBold = new QCheckBox(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Bold last column")), &dlg);
    lastBold->setObjectName(QStringLiteral("lastColumnBold"));
    lastBold->setChecked(start.lastColumnBold);
    form->addRow(QString(), totalBold);
    form->addRow(QString(), firstBold);
    form->addRow(QString(), lastBold);
    auto *bandRows = makeColor("bandedRowFill", tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Banded rows fill")), tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "No Color")));
    setHex(bandRows, start.bandedRowFill);
    auto *bandColumns = makeColor("bandedColumnFill", tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Banded columns fill")), tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "No Color")));
    setHex(bandColumns, start.bandedColumnFill);
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Banded rows:")), bandRows);
    form->addRow(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "Banded columns:")), bandColumns);
    auto *note = new QLabel(tr(QT_TRANSLATE_NOOP("jp::write::TableDialogs", "The header row, total row, first and last columns, and bands show when the table's style options turn them on.")), &dlg);
    note->setWordWrap(true);
    v->addWidget(note);
    okCancel(&dlg, v);
    while (dlg.exec() == QDialog::Accepted) {
        office::wml::TableStyleSpec now;
        now.name = name->text();
        now.basedOn = basedOn->currentData().toString();
        now.lines = office::wml::TableStyleSpec::Lines(lines->currentData().toInt());
        now.lineColor = hexOf(lineColor, *doc);
        if (now.lineColor.isEmpty()) now.lineColor = QStringLiteral("auto");
        now.lineWidth = width->currentData().toInt();
        now.headerBold = headerBold->isChecked();
        now.headerFill = hexOf(headerFill, *doc);
        now.headerText = hexOf(headerText, *doc);
        now.totalBold = totalBold->isChecked();
        now.firstColumnBold = firstBold->isChecked();
        now.lastColumnBold = lastBold->isChecked();
        now.bandedRowFill = hexOf(bandRows, *doc);
        now.bandedColumnFill = hexOf(bandColumns, *doc);
        QString problem;
        QString made;
        if (!modify) made = createTableStyle(*doc, now, &problem);
        const bool done = modify ? modifyTableStyle(*doc, modifyId, now, &problem) : !made.isEmpty();
        if (done && madeId) *madeId = made;
        if (done) return true;
        QMessageBox::warning(&dlg, dlg.windowTitle(), problem);
    }
    return false;
}

} // namespace jp::write
