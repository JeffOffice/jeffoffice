#include "app/chartdialogs.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include <algorithm>

namespace jp {

using namespace jp::office;
using namespace jp::office::sheet;

const QVector<ChartKindEntry> &chartKindEntries()
{
    static const QVector<ChartKindEntry> entries = {
        {ChartKind::Column, QT_TRANSLATE_NOOP("jp::ChartKinds", "Columns")},
        {ChartKind::ColumnStacked, QT_TRANSLATE_NOOP("jp::ChartKinds", "Stacked columns")},
        {ChartKind::ColumnPercent, QT_TRANSLATE_NOOP("jp::ChartKinds", "Columns to 100%")},
        {ChartKind::Bar, QT_TRANSLATE_NOOP("jp::ChartKinds", "Bars")},
        {ChartKind::BarStacked, QT_TRANSLATE_NOOP("jp::ChartKinds", "Stacked bars")},
        {ChartKind::BarPercent, QT_TRANSLATE_NOOP("jp::ChartKinds", "Bars to 100%")},
        {ChartKind::Line, QT_TRANSLATE_NOOP("jp::ChartKinds", "Line")},
        {ChartKind::LineMarkers, QT_TRANSLATE_NOOP("jp::ChartKinds", "Line with markers")},
        {ChartKind::LineStacked, QT_TRANSLATE_NOOP("jp::ChartKinds", "Stacked line")},
        {ChartKind::Pie, QT_TRANSLATE_NOOP("jp::ChartKinds", "Pie")},
        {ChartKind::Doughnut, QT_TRANSLATE_NOOP("jp::ChartKinds", "Doughnut")},
        {ChartKind::Area, QT_TRANSLATE_NOOP("jp::ChartKinds", "Area")},
        {ChartKind::AreaStacked, QT_TRANSLATE_NOOP("jp::ChartKinds", "Stacked area")},
        {ChartKind::Scatter, QT_TRANSLATE_NOOP("jp::ChartKinds", "Scatter, points")},
        {ChartKind::ScatterLines, QT_TRANSLATE_NOOP("jp::ChartKinds", "Scatter, joined points")},
        {ChartKind::Combo, QT_TRANSLATE_NOOP("jp::ChartKinds", "Columns with a line")},
    };
    return entries;
}

QString chartKindName(ChartKind kind)
{
    for (const ChartKindEntry &e : chartKindEntries())
        if (e.kind == kind) return QCoreApplication::translate("jp::ChartKinds", e.name);
    return QString();
}

namespace {

// Kinds a series may be drawn as in a combination (bars across are not among them: they do not mix).
const QVector<ChartKind> &comboKinds()
{
    static const QVector<ChartKind> k = {ChartKind::Column, ChartKind::ColumnStacked, ChartKind::Line, ChartKind::LineMarkers, ChartKind::Area, ChartKind::AreaStacked, ChartKind::Scatter};
    return k;
}

} // namespace

// ---- New Chart

InsertChartDialog::InsertChartDialog(const ChartPreviewFn &preview, ChartKind initial, QWidget *parent) : QDialog(parent), m_preview(preview)
{
    setWindowTitle(tr("New Chart"));
    setObjectName(QStringLiteral("insertChartDialog"));
    auto *outer = new QVBoxLayout(this);
    auto *row = new QHBoxLayout;
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("chartKinds"));
    m_list->setAccessibleName(tr("Kind of chart"));
    m_list->setIconSize(QSize(64, 44));
    m_list->setMinimumWidth(220);
    for (const ChartKindEntry &e : chartKindEntries()) {
        auto *item = new QListWidgetItem(chartKindName(e.kind), m_list);
        item->setData(Qt::UserRole, int(e.kind));
        // (drawn large and made small: the words of a chart drawn at 64 pixels would be bigger than the chart)
        const QImage thumb = preview ? preview(e.kind, -1, QSize(256, 176)) : QImage();
        if (!thumb.isNull()) item->setIcon(QIcon(QPixmap::fromImage(thumb.scaled(QSize(64, 44), Qt::IgnoreAspectRatio, Qt::SmoothTransformation))));
    }
    m_big = new QLabel(this);
    m_big->setObjectName(QStringLiteral("chartPreview"));
    m_big->setAccessibleName(tr("Preview of the chart"));
    m_big->setMinimumSize(360, 260);
    m_big->setAlignment(Qt::AlignCenter);
    m_big->setFrameShape(QFrame::StyledPanel);
    row->addWidget(m_list);
    row->addWidget(m_big, 1);
    outer->addLayout(row, 1);
    m_switch = new QCheckBox(tr("Swap rows and columns"), this);
    m_switch->setAccessibleName(tr("Swap rows and columns"));
    outer->addWidget(m_switch);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    outer->addWidget(buttons);
    connect(m_list, &QListWidget::currentRowChanged, this, [this] { refresh(); });
    connect(m_list, &QListWidget::itemDoubleClicked, this, [this] { accept(); });
    connect(m_switch, &QCheckBox::toggled, this, [this] { refresh(); });
    selectKind(initial);
    resize(640, 420);
}

ChartKind InsertChartDialog::kind() const
{
    const QListWidgetItem *item = m_list->currentItem();
    return item ? ChartKind(item->data(Qt::UserRole).toInt()) : ChartKind::Column;
}

int InsertChartDialog::inRows() const { return m_switch->isChecked() ? 2 : -1; }

void InsertChartDialog::selectKind(ChartKind kind)
{
    for (int i = 0; i < m_list->count(); ++i)
        if (ChartKind(m_list->item(i)->data(Qt::UserRole).toInt()) == kind) m_list->setCurrentRow(i);
    refresh();
}

void InsertChartDialog::setRowsAndColumns(bool switched) { m_switch->setChecked(switched); }

QImage InsertChartDialog::previewShown() const
{
    const QPixmap p = m_big->pixmap();
    return p.isNull() ? QImage() : p.toImage();
}

void InsertChartDialog::refresh()
{
    if (!m_preview) return;
    // (the switch asks for the other way from the one the shape of the cells says: the window decides which that is)
    const QImage image = m_preview(kind(), m_switch->isChecked() ? 2 : -1, QSize(360, 260));
    if (image.isNull()) {
        m_big->setPixmap(QPixmap());
        m_big->setText(tr("There is nothing to chart in the cells selected."));
    } else {
        m_big->setPixmap(QPixmap::fromImage(image));
    }
}

// ---- Chart Data

namespace {

class SeriesDialog : public QDialog {
public:
    SeriesDialog(const ChartSeries &series, bool scatter, const SelectDataRequest &request, QWidget *parent) : QDialog(parent), m_series(series), m_scatter(scatter), m_request(request)
    {
        setWindowTitle(tr("Edit Series"));
        auto *form = new QFormLayout(this);
        m_name = new QLineEdit(series.nameRef.isEmpty() ? series.name : series.nameRef, this);
        m_name->setAccessibleName(tr("Series name, a reference to a cell or the name itself"));
        m_values = new QLineEdit(series.valueRef, this);
        m_values->setAccessibleName(tr("Series values, a reference to cells"));
        form->addRow(tr("Name"), m_name);
        if (scatter) {
            m_x = new QLineEdit(series.xRef, this);
            m_x->setAccessibleName(tr("X values, a reference to cells"));
            form->addRow(tr("X values"), m_x);
        }
        form->addRow(scatter ? tr("Y values") : tr("Values"), m_values);
        m_problem = new QLabel(this);
        m_problem->setWordWrap(true);
        form->addRow(m_problem);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(buttons, &QDialogButtonBox::accepted, this, [this] {
            if (check()) accept();
        });
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        form->addRow(buttons);
    }
    ChartSeries result() const { return m_series; }

private:
    bool check()
    {
        QString why;
        auto ok = [&](const QString &ref) { return ref.isEmpty() || !m_request.checkRef || m_request.checkRef(ref, &why); };
        const QString values = m_values->text().trimmed(), x = m_x ? m_x->text().trimmed() : QString();
        if (values.isEmpty() || !ok(values) || !ok(x)) {
            m_problem->setText(why.isEmpty() ? tr("Type the cells the values are in.") : why);
            return false;
        }
        const QString name = m_name->text().trimmed();
        QString nameWhy;
        const bool isRef = name.contains(QLatin1Char('!')) && (!m_request.checkRef || m_request.checkRef(name, &nameWhy));
        m_series.nameRef = isRef ? name : QString();
        if (!isRef && !name.isEmpty()) m_series.name = name;
        m_series.valueRef = values;
        if (m_scatter) m_series.xRef = x;
        return true;
    }
    ChartSeries m_series;
    bool m_scatter;
    SelectDataRequest m_request;
    QLineEdit *m_name = nullptr, *m_values = nullptr, *m_x = nullptr;
    QLabel *m_problem = nullptr;
};

} // namespace

SelectDataDialog::SelectDataDialog(const SelectDataRequest &request, QWidget *parent) : QDialog(parent), m_request(request), m_chart(request.chart)
{
    setWindowTitle(tr("Chart Data"));
    setObjectName(QStringLiteral("selectDataDialog"));
    m_series = seriesOf(m_chart);
    m_kinds = seriesKindsOf(m_chart);
    auto *outer = new QVBoxLayout(this);
    auto *rangeRow = new QHBoxLayout;
    m_range = new QLineEdit(this);
    m_range->setObjectName(QStringLiteral("dataRange"));
    m_range->setAccessibleName(tr("Cells the chart reads"));
    m_range->setPlaceholderText(tr("For example Sheet1!$A$1:$D$8"));
    auto *useRange = new QPushButton(tr("Use These Cells"), this);
    auto *switchButton = new QPushButton(tr("Swap Rows and Columns"), this);
    rangeRow->addWidget(new QLabel(tr("Cells the chart reads"), this));
    rangeRow->addWidget(m_range, 1);
    rangeRow->addWidget(useRange);
    outer->addLayout(rangeRow);
    auto *body = new QHBoxLayout;
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("seriesList"));
    m_list->setAccessibleName(tr("Series of the chart"));
    body->addWidget(m_list, 1);
    auto *column = new QVBoxLayout;
    auto *add = new QPushButton(tr("Add…"), this);
    auto *edit = new QPushButton(tr("Edit…"), this);
    auto *remove = new QPushButton(tr("Remove"), this);
    auto *up = new QPushButton(tr("Move Up"), this);
    auto *down = new QPushButton(tr("Move Down"), this);
    for (QPushButton *b : {add, edit, remove, up, down}) column->addWidget(b);
    column->addStretch(1);
    column->addWidget(switchButton);
    body->addLayout(column);
    outer->addLayout(body, 1);
    auto *form = new QFormLayout;
    m_categories = new QLineEdit(this);
    m_categories->setObjectName(QStringLiteral("categoryRef"));
    m_categories->setAccessibleName(tr("Cells with the category labels"));
    form->addRow(tr("Category labels"), m_categories);
    m_blanks = new QComboBox(this);
    m_blanks->setObjectName(QStringLiteral("blanksAs"));
    m_blanks->setAccessibleName(tr("Empty cells are drawn as"));
    m_blanks->addItem(tr("Gaps"), QStringLiteral("gap"));
    m_blanks->addItem(tr("Zero"), QStringLiteral("zero"));
    m_blanks->addItem(tr("Connect the points with a line"), QStringLiteral("span"));
    form->addRow(tr("Empty cells are drawn as"), m_blanks);
    m_hidden = new QCheckBox(tr("Include hidden rows and columns"), this);
    m_hidden->setAccessibleName(tr("Include hidden rows and columns"));
    form->addRow(m_hidden);
    outer->addLayout(form);
    m_problem = new QLabel(this);
    m_problem->setWordWrap(true);
    outer->addWidget(m_problem);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    outer->addWidget(buttons);
    // the cells now: the block all the series read, if they read one
    QString first;
    for (const ChartSeries &s : m_series) {
        if (first.isEmpty()) first = s.valueRef;
    }
    m_categories->setText(m_series.isEmpty() ? QString() : (m_chart.type() == ChartType::Scatter ? m_series.first().xRef : m_series.first().categoryRef));
    m_range->setText(chartDataRange(m_chart));
    const int blanksAt = m_blanks->findData(m_chart.blanksAs);
    m_blanks->setCurrentIndex(blanksAt < 0 ? 0 : blanksAt);
    m_hidden->setChecked(!m_chart.plotVisibleOnly);
    rebuildList();
    connect(add, &QPushButton::clicked, this, [this] {
        ChartSeries s;
        s.name = tr("Series%1").arg(m_series.size() + 1);
        s.color = m_chart.palette.value(int(m_series.size()) % std::max<int>(1, m_chart.palette.size()), QColor(0x44, 0x72, 0xC4));
        s.colorFromFile = true;
        if (m_chart.type() == ChartType::Scatter && !m_series.isEmpty()) s.xRef = m_series.first().xRef;
        else if (!m_series.isEmpty()) s.categoryRef = m_series.first().categoryRef;
        m_series.push_back(s);
        m_kinds.push_back(m_kinds.isEmpty() ? SeriesKind() : m_kinds.last());
        if (!editSeries(int(m_series.size()) - 1)) {
            m_series.removeLast();
            m_kinds.removeLast();
        }
        rebuildList();
    });
    connect(edit, &QPushButton::clicked, this, [this] {
        if (m_list->currentRow() >= 0 && editSeries(m_list->currentRow())) rebuildList();
    });
    connect(remove, &QPushButton::clicked, this, [this] { removeSeries(m_list->currentRow()); });
    connect(up, &QPushButton::clicked, this, [this] { moveSeries(m_list->currentRow(), -1); });
    connect(down, &QPushButton::clicked, this, [this] { moveSeries(m_list->currentRow(), 1); });
    connect(switchButton, &QPushButton::clicked, this, [this] { switchRowsAndColumns(); });
    connect(useRange, &QPushButton::clicked, this, [this] { applyDataRange(m_range->text()); });
    connect(m_list, &QListWidget::itemDoubleClicked, this, [this] {
        if (m_list->currentRow() >= 0 && editSeries(m_list->currentRow())) rebuildList();
    });
    resize(560, 460);
}

void SelectDataDialog::rebuildList()
{
    const int row = m_list->currentRow();
    m_list->clear();
    for (const ChartSeries &s : std::as_const(m_series)) {
        const QString name = m_request.nameOf ? m_request.nameOf(s) : s.name;
        m_list->addItem(name.isEmpty() ? s.name : name);
    }
    if (!m_series.isEmpty()) m_list->setCurrentRow(std::clamp(row, 0, int(m_series.size()) - 1));
}

bool SelectDataDialog::editSeries(int index)
{
    if (index < 0 || index >= m_series.size()) return false;
    SeriesDialog d(m_series[index], m_chart.type() == ChartType::Scatter, m_request, this);
    if (d.exec() != QDialog::Accepted) return false;
    m_series[index] = d.result();
    return true;
}

int SelectDataDialog::seriesShown() const { return int(m_series.size()); }

void SelectDataDialog::addSeries(const ChartSeries &series)
{
    m_series.push_back(series);
    m_kinds.push_back(m_kinds.isEmpty() ? SeriesKind() : m_kinds.last());
    rebuildList();
}

void SelectDataDialog::removeSeries(int index)
{
    if (index < 0 || index >= m_series.size()) return;
    m_series.removeAt(index);
    m_kinds.removeAt(index);
    rebuildList();
}

void SelectDataDialog::moveSeries(int index, int delta)
{
    const int to = index + delta;
    if (index < 0 || to < 0 || index >= m_series.size() || to >= m_series.size()) return;
    std::swap(m_series[index], m_series[to]);
    m_list->setCurrentRow(to);
    rebuildList();
    m_list->setCurrentRow(to);
}

void SelectDataDialog::setSeries(int index, const ChartSeries &series)
{
    if (index < 0 || index >= m_series.size()) return;
    m_series[index] = series;
    rebuildList();
}

ChartSeries SelectDataDialog::seriesAt(int index) const { return index >= 0 && index < m_series.size() ? m_series[index] : ChartSeries(); }

void SelectDataDialog::setCategoryRef(const QString &ref) { m_categories->setText(ref); }

void SelectDataDialog::applyDataRange(const QString &range)
{
    m_problem->clear();
    if (!m_request.planFor) return;
    const ChartPlan plan = m_request.planFor(range.trimmed(), -1);
    if (plan.series.isEmpty()) {
        m_problem->setText(tr("JeffCrunch can't find numbers to chart in those cells."));
        return;
    }
    QVector<ChartSeries> fresh;
    for (int i = 0; i < plan.series.size(); ++i) {
        ChartSeries s = i < m_series.size() ? m_series[i] : ChartSeries();
        if (i >= m_series.size()) {
            s.name = tr("Series%1").arg(i + 1);
            s.color = m_chart.palette.value(i % std::max<int>(1, m_chart.palette.size()), QColor(0x44, 0x72, 0xC4));
            s.colorFromFile = true;
        }
        s.nameRef = plan.series[i].nameRef;
        s.categoryRef = plan.series[i].categoryRef;
        s.valueRef = plan.series[i].valueRef;
        s.xRef = plan.series[i].xRef;
        fresh << s;
    }
    m_series = fresh;
    while (m_kinds.size() < m_series.size()) m_kinds.push_back(m_kinds.isEmpty() ? SeriesKind() : m_kinds.last());
    m_kinds.resize(m_series.size());
    m_categories->setText(m_series.first().categoryRef.isEmpty() ? m_series.first().xRef : m_series.first().categoryRef);
    rebuildList();
}

void SelectDataDialog::switchRowsAndColumns()
{
    m_problem->clear();
    Chart now = result();
    Chart switched;
    if (!m_request.switcher || !m_request.switcher(now, &switched)) {
        m_problem->setText(tr("The series don't read one block of cells that can be turned the other way."));
        return;
    }
    m_chart = switched;
    m_series = seriesOf(switched);
    m_kinds = seriesKindsOf(switched);
    m_categories->setText(m_series.isEmpty() ? QString() : (switched.type() == ChartType::Scatter ? m_series.first().xRef : m_series.first().categoryRef));
    rebuildList();
}

void SelectDataDialog::setBlanks(const QString &how, bool plotHidden)
{
    const int at = m_blanks->findData(how);
    if (at >= 0) m_blanks->setCurrentIndex(at);
    m_hidden->setChecked(plotHidden);
}

Chart SelectDataDialog::result() const
{
    QVector<ChartSeries> series = m_series;
    const QString cats = m_categories->text().trimmed();
    const bool scatter = m_chart.type() == ChartType::Scatter;
    for (ChartSeries &s : series)
        if (!scatter && cats != s.categoryRef) {
            s.categoryRef = cats;
            if (cats.isEmpty()) s.categories.clear();
        } else if (scatter && cats != s.xRef) {
            s.xRef = cats;
        }
    Chart c = withSeries(m_chart, series, m_kinds, m_request.theme);
    c.blanksAs = m_blanks->currentData().toString();
    c.plotVisibleOnly = !m_hidden->isChecked();
    return c;
}

// ---- Chart Kind

ChangeChartTypeDialog::ChangeChartTypeDialog(const QStringList &seriesNames, const QVector<SeriesKind> &now, const KindsPreviewFn &preview, QWidget *parent)
    : QDialog(parent), m_names(seriesNames), m_preview(preview)
{
    setWindowTitle(tr("Chart Kind"));
    setObjectName(QStringLiteral("changeTypeDialog"));
    auto *outer = new QVBoxLayout(this);
    auto *row = new QHBoxLayout;
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("chartKinds"));
    m_list->setAccessibleName(tr("Kind of chart"));
    m_list->setMinimumWidth(200);
    for (const ChartKindEntry &e : chartKindEntries()) {
        auto *item = new QListWidgetItem(chartKindName(e.kind), m_list);
        item->setData(Qt::UserRole, int(e.kind));
    }
    auto *combination = new QListWidgetItem(tr("Combination, series by series"), m_list);
    combination->setData(Qt::UserRole, -1);
    auto *right = new QVBoxLayout;
    m_big = new QLabel(this);
    m_big->setObjectName(QStringLiteral("chartPreview"));
    m_big->setAccessibleName(tr("Preview of the chart"));
    m_big->setMinimumSize(320, 220);
    m_big->setAlignment(Qt::AlignCenter);
    m_big->setFrameShape(QFrame::StyledPanel);
    right->addWidget(m_big, 1);
    m_table = new QTableWidget(int(m_names.size()), 3, this);
    m_table->setObjectName(QStringLiteral("seriesKinds"));
    m_table->setAccessibleName(tr("Kind of each series"));
    m_table->setHorizontalHeaderLabels({tr("Series"), tr("Drawn as"), tr("Second axis")});
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setMaximumHeight(150);
    for (int i = 0; i < m_names.size(); ++i) {
        auto *name = new QTableWidgetItem(m_names[i]);
        name->setFlags(name->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 0, name);
        auto *box = new QComboBox(m_table);
        for (const ChartKind k : comboKinds()) box->addItem(chartKindName(k), int(k));
        m_table->setCellWidget(i, 1, box);
        auto *second = new QCheckBox(m_table);
        m_table->setCellWidget(i, 2, second);
        connect(box, &QComboBox::currentIndexChanged, this, [this] { refresh(); });
        connect(second, &QCheckBox::toggled, this, [this] { refresh(); });
    }
    right->addWidget(m_table);
    row->addWidget(m_list);
    row->addLayout(right, 1);
    outer->addLayout(row, 1);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    outer->addWidget(buttons);
    connect(m_list, &QListWidget::currentRowChanged, this, [this] { refresh(); });
    // what the chart is now: one kind for all, or a combination
    bool same = !now.isEmpty();
    for (const SeriesKind &k : now) same = same && k.kind == now.first().kind && !k.secondary;
    if (same) chooseKind(now.first().kind);
    else chooseCombination(now);
    resize(640, 460);
}

bool ChangeChartTypeDialog::isCombination() const
{
    const QListWidgetItem *item = m_list->currentItem();
    return item && item->data(Qt::UserRole).toInt() < 0;
}

void ChangeChartTypeDialog::chooseKind(ChartKind kind)
{
    for (int i = 0; i < m_list->count(); ++i)
        if (m_list->item(i)->data(Qt::UserRole).toInt() == int(kind)) m_list->setCurrentRow(i);
    refresh();
}

void ChangeChartTypeDialog::chooseCombination(const QVector<SeriesKind> &kinds)
{
    m_loading = true;
    for (int i = 0; i < m_names.size(); ++i) {
        const SeriesKind k = kinds.value(i);
        auto *box = qobject_cast<QComboBox *>(m_table->cellWidget(i, 1));
        auto *second = qobject_cast<QCheckBox *>(m_table->cellWidget(i, 2));
        int at = box->findData(int(k.kind));
        if (at < 0) at = 0;   // (a kind that does not mix is shown as columns)
        box->setCurrentIndex(at);
        second->setChecked(k.secondary);
    }
    m_loading = false;
    m_list->setCurrentRow(m_list->count() - 1);
    refresh();
}

QVector<SeriesKind> ChangeChartTypeDialog::kinds() const
{
    QVector<SeriesKind> out;
    const QListWidgetItem *item = m_list->currentItem();
    const int chosen = item ? item->data(Qt::UserRole).toInt() : 0;
    for (int i = 0; i < m_names.size(); ++i) {
        if (chosen >= 0) {
            const ChartKind k = ChartKind(chosen);
            // (the combination of the list: columns, and the last series a line against the second axes)
            out << (k == ChartKind::Combo && m_names.size() >= 2 ? (i == m_names.size() - 1 ? SeriesKind{ChartKind::LineMarkers, true} : SeriesKind{ChartKind::Column, false})
                                                                  : SeriesKind{k == ChartKind::Combo ? ChartKind::Column : k, false});
            continue;
        }
        SeriesKind s;
        s.kind = ChartKind(qobject_cast<QComboBox *>(m_table->cellWidget(i, 1))->currentData().toInt());
        s.secondary = qobject_cast<QCheckBox *>(m_table->cellWidget(i, 2))->isChecked();
        out << s;
    }
    return out;
}

void ChangeChartTypeDialog::refresh()
{
    if (m_loading) return;
    m_table->setEnabled(isCombination());
    if (!m_preview) return;
    const QImage image = m_preview(kinds(), QSize(320, 220));
    if (image.isNull()) m_big->setPixmap(QPixmap());
    else m_big->setPixmap(QPixmap::fromImage(image));
}

// ---- Properties

ObjectPropertiesDialog::ObjectPropertiesDialog(const ObjectProperties &now, QWidget *parent) : QDialog(parent), m_now(now)
{
    setWindowTitle(tr("Properties"));
    setObjectName(QStringLiteral("objectPropertiesDialog"));
    auto *outer = new QVBoxLayout(this);
    auto *form = new QFormLayout;
    m_width = new QDoubleSpinBox(this);
    m_width->setObjectName(QStringLiteral("objectWidth"));
    m_width->setAccessibleName(tr("Width in points"));
    m_height = new QDoubleSpinBox(this);
    m_height->setObjectName(QStringLiteral("objectHeight"));
    m_height->setAccessibleName(tr("Height in points"));
    for (QDoubleSpinBox *b : {m_width, m_height}) {
        b->setRange(3, 3000);
        b->setDecimals(1);
        b->setSuffix(tr(" pt"));
    }
    m_width->setValue(now.widthPoints);
    m_height->setValue(now.heightPoints);
    m_ratioOf = now.heightPoints > 0 ? now.widthPoints / now.heightPoints : 1;
    form->addRow(tr("Width"), m_width);
    form->addRow(tr("Height"), m_height);
    m_ratio = new QCheckBox(tr("Keep the proportions"), this);
    m_ratio->setAccessibleName(tr("Keep the proportions"));
    m_ratio->setChecked(now.keepProportions);
    form->addRow(m_ratio);
    outer->addLayout(form);
    auto *group = new QGroupBox(tr("When cells change"), this);
    auto *g = new QVBoxLayout(group);
    m_sizes = new QRadioButton(tr("Moves and stretches with its cells"), group);
    m_moves = new QRadioButton(tr("Moves with its cells, keeps its size"), group);
    m_stays = new QRadioButton(tr("Stay where it is"), group);
    m_sizes->setObjectName(QStringLiteral("followSizes"));
    m_moves->setObjectName(QStringLiteral("followMoves"));
    m_stays->setObjectName(QStringLiteral("followStays"));
    g->addWidget(m_sizes);
    g->addWidget(m_moves);
    g->addWidget(m_stays);
    (now.follows == AnchorKind::OneCell ? m_moves : now.follows == AnchorKind::Absolute ? m_stays : m_sizes)->setChecked(true);
    outer->addWidget(group);
    connect(m_width, &QDoubleSpinBox::valueChanged, this, [this](double v) {
        if (m_ratio->isChecked() && m_ratioOf > 0 && !m_height->hasFocus()) m_height->setValue(v / m_ratioOf);
    });
    connect(m_height, &QDoubleSpinBox::valueChanged, this, [this](double v) {
        if (m_ratio->isChecked() && m_ratioOf > 0 && !m_width->hasFocus()) m_width->setValue(v * m_ratioOf);
    });
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    outer->addWidget(buttons);
}

ObjectProperties ObjectPropertiesDialog::result() const
{
    ObjectProperties p = m_now;
    p.widthPoints = m_width->value();
    p.heightPoints = m_height->value();
    p.follows = m_moves->isChecked() ? AnchorKind::OneCell : m_stays->isChecked() ? AnchorKind::Absolute : AnchorKind::TwoCell;
    p.keepProportions = m_ratio->isChecked();
    return p;
}

} // namespace jp
