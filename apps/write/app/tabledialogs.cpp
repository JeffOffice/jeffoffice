#include "app/tabledialogs.h"

#include "app/settings.h"
#include "app/widgets.h"
#include "core/writedoc.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHash>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextCursor>
#include <QVBoxLayout>

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
    dlg.setWindowTitle(tr("Insert Table"));
    auto *v = new QVBoxLayout(&dlg);
    auto *size = new QGroupBox(tr("Table size"), &dlg);
    auto *form = new QFormLayout(size);
    auto *columns = new QSpinBox(size), *rows = new QSpinBox(size);
    columns->setObjectName(QStringLiteral("columns"));
    rows->setObjectName(QStringLiteral("rows"));
    columns->setRange(1, 63);
    rows->setRange(1, 500);
    columns->setValue(Settings::get().value(QStringLiteral("write/table/columns"), table->columns).toInt());
    rows->setValue(Settings::get().value(QStringLiteral("write/table/rows"), table->rows).toInt());
    form->addRow(tr("Number of columns:"), columns);
    form->addRow(tr("Number of rows:"), rows);
    v->addWidget(size);
    auto *fit = new QGroupBox(tr("AutoFit behavior"), &dlg);
    auto *fv = new QVBoxLayout(fit);
    auto *fixed = new QRadioButton(tr("Fixed column width"), fit), *contents = new QRadioButton(tr("AutoFit to contents"), fit), *window = new QRadioButton(tr("AutoFit to window"), fit);
    fixed->setObjectName(QStringLiteral("fixedWidth"));
    contents->setObjectName(QStringLiteral("fitContents"));
    window->setObjectName(QStringLiteral("fitWindow"));
    auto *width = new MeasureSpin(fit);
    width->setObjectName(QStringLiteral("columnWidth"));
    width->setPoints(0);
    width->setSpecialValueText(tr("Auto"));
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
    auto *remember = new QCheckBox(tr("Remember dimensions for new tables"), &dlg);
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
    dlg.setWindowTitle(tr("Split Cells"));
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
    form->addRow(tr("Number of columns:"), c);
    form->addRow(tr("Number of rows:"), r);
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
    dlg.setWindowTitle(tr("Table Properties"));
    auto *v = new QVBoxLayout(&dlg);
    auto *tabs = new QTabWidget(&dlg);
    v->addWidget(tabs);

    // ---- Table
    auto *tablePage = new QWidget;
    auto *tf = new QFormLayout(tablePage);
    auto *align = new QComboBox(tablePage);
    align->setObjectName(QStringLiteral("alignment"));
    align->addItem(tr("Left"), int(flow::TableData::Align::Left));
    align->addItem(tr("Center"), int(flow::TableData::Align::Center));
    align->addItem(tr("Right"), int(flow::TableData::Align::Right));
    align->setCurrentIndex(int(was.align));
    tf->addRow(tr("Alignment:"), align);
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
    tf->addRow(tr("Indent from left:"), indent);
    auto *widthKind = new QComboBox(tablePage);
    widthKind->setObjectName(QStringLiteral("widthKind"));
    widthKind->addItem(tr("Automatic"), int(TableProps::Width::Auto));
    widthKind->addItem(tr("Points"), int(TableProps::Width::Points));
    widthKind->addItem(tr("Percent"), int(TableProps::Width::Percent));
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
    tf->addRow(tr("Preferred width:"), widthRow);
    auto showWidth = [=] {
        const int k = widthKind->currentData().toInt();
        widthPoints->setVisible(k == int(TableProps::Width::Points));
        widthPercent->setVisible(k == int(TableProps::Width::Percent));
    };
    showWidth();
    QObject::connect(widthKind, &QComboBox::currentIndexChanged, &dlg, showWidth);
    auto *fixedLayout = new QCheckBox(tr("Fixed column widths"), tablePage);
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
    tf->addRow(tr("Default cell margin, top:"), top);
    tf->addRow(tr("Default cell margin, left:"), left);
    tf->addRow(tr("Default cell margin, bottom:"), bottom);
    tf->addRow(tr("Default cell margin, right:"), right);
    auto *spacing = new MeasureSpin(tablePage);
    spacing->setObjectName(QStringLiteral("spacing"));
    set(spacing, was.spacing, false);
    tf->addRow(tr("Space between cells:"), spacing);
    tabs->addTab(tablePage, tr("Table"));

    // ---- Row
    auto *rowPage = new QWidget;
    auto *rf = new QFormLayout(rowPage);
    auto *height = new MeasureSpin(rowPage);
    height->setObjectName(QStringLiteral("rowHeight"));
    set(height, was.rowHeight, false);
    auto *rule = new QComboBox(rowPage);
    rule->setObjectName(QStringLiteral("rowRule"));
    rule->addItem(tr("At least"), false);
    rule->addItem(tr("Exactly"), true);
    rule->setCurrentIndex(was.rowExact ? 1 : 0);
    auto *heightRow = new QHBoxLayout;
    heightRow->addWidget(height);
    heightRow->addWidget(rule);
    rf->addRow(tr("Row height (0 for as tall as the cells need):"), heightRow);
    auto *cantSplit = new QCheckBox(tr("Do not let the row break across pages"), rowPage);
    cantSplit->setObjectName(QStringLiteral("cantSplit"));
    cantSplit->setChecked(was.cantSplit);
    rf->addRow(QString(), cantSplit);
    auto *header = new QCheckBox(tr("Repeat as header row at the top of each page"), rowPage);
    header->setObjectName(QStringLiteral("headerRow"));
    header->setChecked(was.header);
    rf->addRow(QString(), header);
    tabs->addTab(rowPage, tr("Row"));

    // ---- Column
    auto *columnPage = new QWidget;
    auto *cf = new QFormLayout(columnPage);
    auto *columnWidth = new MeasureSpin(columnPage);
    columnWidth->setObjectName(QStringLiteral("columnWidth"));
    set(columnWidth, was.columnWidth, false);
    cf->addRow(tr("Column width:"), columnWidth);
    tabs->addTab(columnPage, tr("Column"));

    // ---- Cell
    auto *cellPage = new QWidget;
    auto *cl = new QFormLayout(cellPage);
    auto *valign = new QComboBox(cellPage);
    valign->setObjectName(QStringLiteral("verticalAlignment"));
    valign->addItem(tr("Top"), int(flow::CellAlign::Top));
    valign->addItem(tr("Center"), int(flow::CellAlign::Center));
    valign->addItem(tr("Bottom"), int(flow::CellAlign::Bottom));
    valign->setCurrentIndex(int(was.valign));
    cl->addRow(tr("Vertical alignment:"), valign);
    auto *noWrap = new QCheckBox(tr("Do not wrap text"), cellPage);
    noWrap->setObjectName(QStringLiteral("noWrap"));
    noWrap->setChecked(was.noWrap);
    cl->addRow(QString(), noWrap);
    tabs->addTab(cellPage, tr("Cell"));

    // ---- Alt Text
    auto *altPage = new QWidget;
    auto *af = new QFormLayout(altPage);
    auto *title = new QLineEdit(was.altTitle, altPage), *text = new QLineEdit(was.altText, altPage);
    title->setObjectName(QStringLiteral("altTitle"));
    text->setObjectName(QStringLiteral("altText"));
    af->addRow(tr("Title:"), title);
    af->addRow(tr("Description:"), text);
    tabs->addTab(altPage, tr("Alt Text"));

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
    dlg.setWindowTitle(tr("Borders and Shading"));
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
    auto *sides = new QGroupBox(tr("Lines on"), bordersPage);
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
    style->addItem(tr("None"), int(flow::TableLine::Style::None));
    style->addItem(tr("Single"), int(flow::TableLine::Style::Single));
    style->addItem(tr("Double"), int(flow::TableLine::Style::Double));
    style->addItem(tr("Dotted"), int(flow::TableLine::Style::Dotted));
    style->addItem(tr("Dashed"), int(flow::TableLine::Style::Dashed));
    style->addItem(tr("Triple"), int(flow::TableLine::Style::Triple));
    style->setCurrentIndex(1);
    auto *widthBox = new QComboBox(bordersPage);
    widthBox->setObjectName(QStringLiteral("lineWidth"));
    for (double w : {0.25, 0.5, 0.75, 1.0, 1.5, 2.25, 3.0, 4.5, 6.0}) widthBox->addItem(QStringLiteral("%1 pt").arg(w), w);
    widthBox->setCurrentIndex(1);
    auto *color = new ColorButton(QStringLiteral("baseline"), tr("Line color"), true, tr("Automatic"), bordersPage);
    color->setObjectName(QStringLiteral("lineColor"));
    color->setWell(true);
    auto *lf = new QFormLayout;
    lf->addRow(tr("Style:"), style);
    lf->addRow(tr("Width:"), widthBox);
    lf->addRow(tr("Color:"), color);
    bl->addLayout(lf, 0, 1);
    tabs->addTab(bordersPage, tr("Borders"));

    auto *shadingPage = new QWidget;
    auto *shl = new QFormLayout(shadingPage);
    auto *fill = new ColorButton(QStringLiteral("paint-bucket"), tr("Fill"), true, tr("No Color"), shadingPage);
    fill->setObjectName(QStringLiteral("fillColor"));
    fill->setWell(true);
    auto *useFill = new QCheckBox(tr("Change the fill of the cells"), shadingPage);
    useFill->setObjectName(QStringLiteral("changeFill"));
    shl->addRow(QString(), useFill);
    shl->addRow(tr("Fill:"), fill);
    tabs->addTab(shadingPage, tr("Shading"));

    auto *applyTo = new QComboBox(&dlg);
    applyTo->setObjectName(QStringLiteral("applyTo"));
    applyTo->addItem(tr("Selected cells"), 0);
    applyTo->addItem(tr("Table"), 1);
    auto *applyRow = new QFormLayout;
    applyRow->addRow(tr("Apply to:"), applyTo);
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

} // namespace jp::write
