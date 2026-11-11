#include "app/chartedit.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include "app/plaintext.h"

namespace jp {

QString insertChart(Editor *ed, office::sheet::ChartForm form)
{
    if (!ed || !ed->doc() || ed->readOnly()) return QString();
    if (chartObjectsIn(*ed->doc()) >= office::sheet::kMostChartsInFile) {
        Q_EMIT ed->status(QCoreApplication::translate("jp::ChartDialogs", "This file already holds the most charts it can (%1), so no chart was added.").arg(office::sheet::kMostChartsInFile));
        return QString();
    }
    ed->endTextEdit();
    const QSizeF page = ed->surfaceSize();
    const double w = std::min(360.0, page.width() * 0.8), h = std::min(216.0, page.height() * 0.8);
    const QRectF rect((page.width() - w) / 2, (page.height() - h) / 2, w, h);
    auto chart = makeChartObject(*ed->doc(), form, office::sheet::sampleChartData(), rect);
    if (!chart) return QString();
    ed->beginChange(QCoreApplication::translate("jp::ChartDialogs", "Insert Chart"));
    ed->surfaceItems().push_back(chart);
    ed->endChange();
    ed->select(chart->id);
    return chart->id;
}

PictureItem *selectedChart(Editor *ed)
{
    Item *it = ed ? ed->single() : nullptr;
    if (!it || it->type() != ItemType::Picture) return nullptr;
    auto *pic = static_cast<PictureItem *>(it);
    return pic->chartXml.isEmpty() ? nullptr : pic;
}

bool changeSelectedChart(Editor *ed, const QString &label, const std::function<bool(office::Chart &)> &edit)
{
    PictureItem *pic = selectedChart(ed);
    if (!pic || ed->readOnly()) return false;
    // (A change that does nothing, or that can't be made, leaves the document as it was, so it makes no step.)
    ed->beginChange(label);
    const bool done = changeChartOf(*ed->doc(), *pic, edit);
    ed->endChange();
    return done;
}

office::sheet::ChartInfo selectedChartInfo(Editor *ed)
{
    const PictureItem *pic = selectedChart(ed);
    return pic ? chartInfoOfPicture(*ed->doc(), *pic) : office::sheet::ChartInfo();
}

static bool askText(QWidget *parent, const QString &name, const QString &editName, const QString &title, const QString &prompt, QString *text)
{
    QDialog dlg(parent);
    dlg.setObjectName(name);
    dlg.setWindowTitle(title);
    auto *layout = new QVBoxLayout(&dlg);
    layout->addWidget(plainLabel(new QLabel(prompt, &dlg)));
    auto *edit = new QLineEdit(*text, &dlg);
    edit->setObjectName(editName);
    edit->setMinimumWidth(320);
    layout->addWidget(edit);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);
    if (dlg.exec() != QDialog::Accepted) return false;
    *text = edit->text();
    return true;
}

bool askChartTitle(QWidget *parent, QString *text)
{
    return askText(parent, QStringLiteral("chartTitleDialog"), QStringLiteral("chartTitleEdit"), QCoreApplication::translate("jp::ChartDialogs", "Chart Title"), QCoreApplication::translate("jp::ChartDialogs", "Words for the title of the chart:"), text);
}

bool askChartAxisTitle(QWidget *parent, bool values, QString *text)
{
    return askText(parent, QStringLiteral("chartAxisTitleDialog"), QStringLiteral("chartAxisTitleEdit"),
                   values ? QCoreApplication::translate("jp::ChartDialogs", "Title of the Axis of Values") : QCoreApplication::translate("jp::ChartDialogs", "Title of the Axis of Categories"), QCoreApplication::translate("jp::ChartDialogs", "Words for the title of the axis (leave empty for none):"), text);
}

bool askChartData(QWidget *parent, office::sheet::ChartData *data, QString *notice)
{
    using namespace office::sheet;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("chartDataDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::ChartDialogs", "Chart Data"));
    auto *layout = new QVBoxLayout(&dlg);
    auto *help = plainLabel(new QLabel(QCoreApplication::translate("jp::ChartDialogs", "The first column holds the words under the points. The first row holds the names of the series. The other cells are numbers."), &dlg));
    help->setWordWrap(true);
    layout->addWidget(help);
    auto *table = new QTableWidget(data->categories.size() + 1, data->values.size() + 1, &dlg);
    table->setObjectName(QStringLiteral("chartDataTable"));
    table->horizontalHeader()->setVisible(false);
    table->verticalHeader()->setVisible(false);
    table->setMinimumSize(460, 220);
    for (int s = 0; s < data->values.size(); ++s) table->setItem(0, s + 1, new QTableWidgetItem(data->seriesNames.value(s)));
    for (int r = 0; r < data->categories.size(); ++r) {
        table->setItem(r + 1, 0, new QTableWidgetItem(data->categories[r]));
        for (int s = 0; s < data->values.size(); ++s) table->setItem(r + 1, s + 1, new QTableWidgetItem(QString::number(data->values[s].value(r), 'g', 15)));
    }
    layout->addWidget(table);
    auto *row = new QHBoxLayout();
    auto *addRow = new QPushButton(QCoreApplication::translate("jp::ChartDialogs", "Add Row"), &dlg);
    auto *addColumn = new QPushButton(QCoreApplication::translate("jp::ChartDialogs", "Add Column"), &dlg);
    auto *removeRow = new QPushButton(QCoreApplication::translate("jp::ChartDialogs", "Remove Last Row"), &dlg);
    auto *removeColumn = new QPushButton(QCoreApplication::translate("jp::ChartDialogs", "Remove Last Column"), &dlg);
    for (QPushButton *b : {addRow, addColumn, removeRow, removeColumn}) row->addWidget(b);
    layout->addLayout(row);
    QObject::connect(addRow, &QPushButton::clicked, table, [table] {
        if (table->rowCount() <= kMostChartCategories) table->setRowCount(table->rowCount() + 1);
    });
    QObject::connect(addColumn, &QPushButton::clicked, table, [table] {
        if (table->columnCount() <= kMostChartSeries) table->setColumnCount(table->columnCount() + 1);
    });
    QObject::connect(removeRow, &QPushButton::clicked, table, [table] {
        if (table->rowCount() > 2) table->setRowCount(table->rowCount() - 1);
    });
    QObject::connect(removeColumn, &QPushButton::clicked, table, [table] {
        if (table->columnCount() > 2) table->setColumnCount(table->columnCount() - 1);
    });
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);
    if (dlg.exec() != QDialog::Accepted) return false;
    ChartData made;
    for (int c = 1; c < table->columnCount(); ++c) made.seriesNames << (table->item(0, c) ? table->item(0, c)->text() : QString());
    made.values.resize(table->columnCount() - 1);
    for (int r = 1; r < table->rowCount(); ++r) {
        made.categories << (table->item(r, 0) ? table->item(r, 0)->text() : QString());
        for (int c = 1; c < table->columnCount(); ++c) {
            bool ok = false;
            const double v = table->item(r, c) ? table->item(r, c)->text().toDouble(&ok) : 0.0;
            if (!ok && table->item(r, c) && !table->item(r, c)->text().trimmed().isEmpty()) {
                if (notice) *notice = QCoreApplication::translate("jp::ChartDialogs", "Row %1 has a cell that is not a number, so the data was not changed.").arg(r);
                return false;
            }
            made.values[c - 1] << (ok ? v : 0.0);
        }
    }
    *data = made;
    return true;
}

} // namespace jp
