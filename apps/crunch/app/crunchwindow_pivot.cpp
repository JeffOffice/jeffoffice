// Pivot tables in the window (core/editpivot.cpp does the work): the Pivot Table tab (Refresh, Change Data Source, Clear, the Field List, and the Design choices: subtotals, grand totals, and
// the layout of the report), the Pivot Table Fields pane, and Refresh All on the Data tab. Every command is one step of Undo; a refusal is told in words.
#include "app/crunchwindow.h"

#include "app/pivotpane.h"
#include "app/settings.h"
#include "core/formulaspot.h"
#include "core/gotospecial.h"
#include "sheet/cellref.h"
#include "sheet/refshift.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "sheet/pivot.h"
#include "sheet/pivotbook.h"

#include <QAction>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QInputDialog>

namespace jp {

using namespace jp::office::sheet;

int CrunchWindow::selectedPivot() const
{
    if (!m_session || !m_loaded || !m_haveCalc || m_chartSheetShown) return -1;
    const grid::Selection &sel = m_view->selection();
    return m_session->pivotAt(m_sheet, quint32(std::max<qint64>(0, sel.activeRow)), quint32(std::max<qint64>(0, sel.activeCol)));
}

void CrunchWindow::pivotProblem(const QString &title, const crunch::EditSession::Outcome &o)
{
    if (!o.ok && !o.problem.isEmpty()) notice(title, o.problem);
}

void CrunchWindow::editPivot(const std::function<void(PivotTable *)> &change, const QString &text, bool structureChanged)
{
    const int t = selectedPivot();
    if (t < 0) {
        notice(tr("Pivot Table"), tr("Click a cell in a pivot table first."), false);
        return;
    }
    if (!editingAllowed()) return;
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    QString why;
    if (!m_session->pivotUsable(t, &why)) {
        notice(tr("Pivot Table"), why);
        return;
    }
    PivotTable table = *m_session->workbook()->pivots().table(t).table;
    change(&table);
    pivotProblem(text, m_session->setPivot(t, table, structureChanged, text));
}

namespace {

// Insert > Pivot Table: where the data is, and where the table goes.
class PivotCreateDialog : public QDialog {
public:
    PivotCreateDialog(const QString &source, QWidget *parent) : QDialog(parent)
    {
        setWindowTitle(tr("Create Pivot Table"));
        setObjectName(QStringLiteral("pivotCreateDialog"));
        auto *layout = new QVBoxLayout(this);
        auto *form = new QFormLayout;
        m_source = new QLineEdit(source, this);
        m_source->setObjectName(QStringLiteral("pivotCreate.source"));
        form->addRow(tr("The cells the data is in, with a row of names above it, or the name of a table:"), m_source);
        layout->addLayout(form);
        m_newSheet = new QRadioButton(tr("A new sheet"), this);
        m_newSheet->setObjectName(QStringLiteral("pivotCreate.newSheet"));
        m_newSheet->setChecked(true);
        m_existing = new QRadioButton(tr("This sheet or another, at the cell:"), this);
        m_existing->setObjectName(QStringLiteral("pivotCreate.existing"));
        m_where = new QLineEdit(this);
        m_where->setObjectName(QStringLiteral("pivotCreate.where"));
        m_where->setEnabled(false);
        QObject::connect(m_existing, &QRadioButton::toggled, m_where, &QLineEdit::setEnabled);
        layout->addWidget(new QLabel(tr("Where the pivot table goes:"), this));
        layout->addWidget(m_newSheet);
        layout->addWidget(m_existing);
        layout->addWidget(m_where);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        QObject::connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        QObject::connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttons);
    }
    QString source() const { return m_source->text(); }
    bool newSheet() const { return m_newSheet->isChecked(); }
    QString where() const { return m_where->text(); }

private:
    QLineEdit *m_source = nullptr, *m_where = nullptr;
    QRadioButton *m_newSheet = nullptr, *m_existing = nullptr;
};

} // namespace

void CrunchWindow::insertPivotCommand()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || !m_session || !m_loaded) return;
    // the data: the table the cursor is in, the cells selected, or the block of data around the cursor
    QString source;
    if (const int table = m_session->tableAt(m_sheet, quint32(std::max<qint64>(0, m_view->selection().activeRow)), quint32(std::max<qint64>(0, m_view->selection().activeCol))); table >= 0) {
        source = m_session->workbook()->tables()[table].name;
    } else {
        CellRange range = selectedRange();
        const grid::Selection &sel = m_view->selection();
        if (range.row1 == range.row2 && range.col1 == range.col2 && sel.whole == grid::Selection::Whole::None) {
            bool truncated = false;
            range = crunch::currentRegion(*m_session, m_sheet, range.row1, range.col1, &truncated);
        }
        if (range.valid && range.row2 > range.row1) source = sheetNamePrefix(m_loaded->book->info().sheets[m_sheet].name) + rangeName(range);
    }
    bool newSheet = true;
    QString where;
    if (m_pivotCreateAsker) {
        if (!m_pivotCreateAsker(&source, &newSheet, &where)) return;
    } else {
        PivotCreateDialog dialog(source, this);
        if (dialog.exec() != QDialog::Accepted) return;
        source = dialog.source();
        newSheet = dialog.newSheet();
        where = dialog.where();
    }
    int sheet = -1;
    quint32 row = 0, col = 0;
    if (!newSheet) {
        // "J10", or "Sheet2!J10"
        sheet = m_sheet;
        QString cell = where.trimmed();
        const int bang = cell.lastIndexOf(QLatin1Char('!'));
        if (bang > 0) {
            QString name = cell.left(bang);
            if (name.startsWith(QLatin1Char('\'')) && name.endsWith(QLatin1Char('\'')) && name.size() > 1) name = name.mid(1, name.size() - 2).replace(QLatin1String("''"), QLatin1String("'"));
            sheet = -1;
            const auto &sheets = m_loaded->book->info().sheets;
            for (int s = 0; s < sheets.size(); ++s)
                if (!sheets[s].deleted && sheets[s].name.compare(name, Qt::CaseInsensitive) == 0) sheet = s;
            cell = cell.mid(bang + 1);
        }
        cell.remove(QLatin1Char('$'));
        if (sheet < 0 || !parseCellRef(cell, &row, &col)) {
            notice(tr("Create Pivot Table"), tr("Type a cell for the pivot table, for example J10 or Sheet2!A3."));
            return;
        }
    }
    int made = -1;
    const auto o = m_session->createPivot(source, sheet, row, col, &made);
    if (!o.ok) {
        notice(tr("Create Pivot Table"), o.problem);
        return;
    }
    // the table is shown, with its fields to be chosen
    if (made >= 0) {
        const PivotTableEntry &entry = m_session->workbook()->pivots().table(made);
        const CellRange at = m_session->workbook()->pivots().locationOf(made);
        if (at.valid) goToFound(entry.sheet, at.row1, at.col1);
    }
    showPivotFields(true);
}

// While a formula is typed, a click on a value of a pivot table is GETPIVOTDATA with the names of its lines, as the original program writes it (on unless the pivot table tab's option says not).
bool CrunchWindow::pointAtPivotCell(qint64 row, qint64 col)
{
    if (!m_edit.active || !m_session || !m_loaded || row < 0 || col < 0) return false;
    if (!Settings::get().value(QStringLiteral("crunch/generateGetPivotData"), true).toBool()) return false;
    QLineEdit *line = m_formulaBar && m_formulaBar->hasFocus() ? static_cast<QLineEdit *>(m_formulaBar) : static_cast<QLineEdit *>(m_cellEditor);
    if (!line || !line->text().startsWith(QLatin1Char('='))) return false;
    if (m_session->pivotAt(m_sheet, quint32(row), quint32(col)) < 0) return false;
    const crunch::FormulaSpot spot = crunch::spotAt(line->text(), line->cursorPosition(), crunch::argumentSeparator(m_session->locale()));
    if (!spot.canPoint) return false;
    QString text = m_session->pivotDataFormula(m_sheet, quint32(row), quint32(col), m_edit.sheet);
    if (text.isEmpty()) return false;
    text.remove(0, 1);   // (the formula's own "=" is already typed)
    line->insert(text);
    Q_EMIT line->textEdited(line->text());
    return true;
}

void CrunchWindow::createPivotActions()
{
    mk("insert.pivot", tr("Pivot Table"), "table-properties", QKeySequence(), [this] { insertPivotCommand(); });
    mk("pivot.options", tr("Pivot Table Options…"), "settings", QKeySequence(), [this] {
        bool generate = Settings::get().value(QStringLiteral("crunch/generateGetPivotData"), true).toBool();
        if (m_pivotOptionsAsker) {
            if (!m_pivotOptionsAsker(&generate)) return;
        } else {
            QDialog dialog(this);
            dialog.setWindowTitle(tr("Pivot Table Options"));
            dialog.setObjectName(QStringLiteral("pivotOptionsDialog"));
            auto *layout = new QVBoxLayout(&dialog);
            auto *box = new QCheckBox(tr("Write GETPIVOTDATA when I click a value of a pivot table while typing a formula"), &dialog);
            box->setObjectName(QStringLiteral("pivotOptions.generate"));
            box->setChecked(generate);
            layout->addWidget(box);
            auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
            QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
            QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
            layout->addWidget(buttons);
            if (dialog.exec() != QDialog::Accepted) return;
            generate = box->isChecked();
        }
        Settings::get().setValue(QStringLiteral("crunch/generateGetPivotData"), generate);
    });
    m_opsActions << QStringLiteral("insert.pivot") << QStringLiteral("pivot.options");
    mk("pivot.refresh", tr("Refresh"), "refresh-cw", QKeySequence(), [this] {
        const int t = selectedPivot();
        if (t < 0) {
            notice(tr("Refresh"), tr("Click a cell in a pivot table first."), false);
            return;
        }
        if (!editingAllowed()) return;
        if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
        pivotProblem(tr("Refresh"), m_session->refreshPivot(t));
    });
    mk("pivot.refreshAll", tr("Refresh All"), "refresh-ccw", QKeySequence(), [this] {
        if (!editingAllowed()) return;
        if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
        pivotProblem(tr("Refresh All"), m_session->refreshAllPivots());
    });
    mk("pivot.changeSource", tr("Change Data Source…"), "database", QKeySequence(), [this] {
        const int t = selectedPivot();
        if (t < 0) {
            notice(tr("Change Data Source"), tr("Click a cell in a pivot table first."), false);
            return;
        }
        if (!editingAllowed()) return;
        if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
        QString where = m_session->pivotSourceText(t);
        if (m_pivotSourceAsker) {
            if (!m_pivotSourceAsker(&where)) return;
        } else {
            bool ok = false;
            where = QInputDialog::getText(this, tr("Change Pivot Table Data Source"), tr("The cells the data is in, with a row of names above it (for example Sheet1!A1:D100), or the name of a table:"), QLineEdit::Normal, where, &ok);
            if (!ok) return;
        }
        pivotProblem(tr("Change Data Source"), m_session->changePivotSource(t, where));
    });
    mk("pivot.clearAll", tr("Clear All"), "eraser", QKeySequence(), [this] {
        const int t = selectedPivot();
        if (t < 0 || !editingAllowed()) return;
        if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
        pivotProblem(tr("Clear"), m_session->clearPivot(t, false));
    });
    mk("pivot.clearFilters", tr("Clear Filters"), "filter-x", QKeySequence(), [this] {
        const int t = selectedPivot();
        if (t < 0 || !editingAllowed()) return;
        if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
        pivotProblem(tr("Clear"), m_session->clearPivot(t, true));
    });
    mk("pivot.fieldList", tr("Field List"), "list-checks", QKeySequence(), [this] { showPivotFields(act("pivot.fieldList")->isChecked()); }, true);
    mk("pivot.valueSettings", tr("Value Field Settings…"), "settings", QKeySequence(), [this] { askValueField(0); });

    // Design: subtotals, grand totals, and the layout
    auto subtotals = [this](int how) {
        editPivot([how](PivotTable *t) {
            for (int f : t->rowFields) {
                if (f < 0 || f >= t->fields.size()) continue;
                PivotField &pf = t->fields[f];
                // (the lines for the field: the default one, or none)
                QVector<PivotTableItem> kept;
                for (const PivotTableItem &it : pf.items)
                    if (it.t.isEmpty()) kept.append(it);
                if (how == 0) {
                    pf.defaultSubtotal = false;
                    pf.subtotals.clear();
                } else {
                    pf.defaultSubtotal = true;
                    pf.subtotals.clear();
                    pf.subtotalTop = how == 1;
                    PivotTableItem total;
                    total.t = QStringLiteral("default");
                    kept.append(total);
                }
                if (pf.hasItems) pf.items = kept;
                if (how == 1 && !pf.compact && !pf.outline) pf.outline = true;   // (subtotals at the top need the outline or the compact form)
            }
        }, tr("Subtotals"), true);
    };
    mk("pivot.sub.none", tr("Do Not Show Subtotals"), "", QKeySequence(), [subtotals] { subtotals(0); });
    mk("pivot.sub.top", tr("Show All Subtotals at Top of Group"), "", QKeySequence(), [subtotals] { subtotals(1); });
    mk("pivot.sub.bottom", tr("Show All Subtotals at Bottom of Group"), "", QKeySequence(), [subtotals] { subtotals(2); });
    // ("on for rows" is the column of totals at the right, one for each row)
    auto grand = [this](bool rowsOn, bool colsOn) {
        editPivot([=](PivotTable *t) {
            t->colGrandTotals = rowsOn;
            t->rowGrandTotals = colsOn;
        }, tr("Grand Totals"), false);
    };
    mk("pivot.grand.off", tr("Off for Rows and Columns"), "", QKeySequence(), [grand] { grand(false, false); });
    mk("pivot.grand.both", tr("On for Rows and Columns"), "", QKeySequence(), [grand] { grand(true, true); });
    mk("pivot.grand.rows", tr("On for Rows Only"), "", QKeySequence(), [grand] { grand(true, false); });
    mk("pivot.grand.cols", tr("On for Columns Only"), "", QKeySequence(), [grand] { grand(false, true); });
    auto layout = [this](int form) {
        editPivot([form](PivotTable *t) {
            t->compact = form == 2;
            t->outline = form == 1;
            for (PivotField &pf : t->fields) {
                pf.compact = form == 2;
                pf.outline = form != 0;
                pf.subtotalTop = form != 0;   // (the compact and outline forms have their subtotals at the top of the group, the tabular form at the bottom)
            }
        }, tr("Report Layout"), true);
    };
    mk("pivot.layout.compact", tr("Show in Compact Form"), "", QKeySequence(), [layout] { layout(2); });
    mk("pivot.layout.outline", tr("Show in Outline Form"), "", QKeySequence(), [layout] { layout(1); });
    mk("pivot.layout.tabular", tr("Show in Tabular Form"), "", QKeySequence(), [layout] { layout(0); });
    for (const char *id : {"pivot.refresh", "pivot.refreshAll", "pivot.changeSource", "pivot.clearAll", "pivot.clearFilters", "pivot.fieldList", "pivot.valueSettings", "pivot.sub.none", "pivot.sub.top",
                           "pivot.sub.bottom", "pivot.grand.off", "pivot.grand.both", "pivot.grand.rows", "pivot.grand.cols", "pivot.layout.compact", "pivot.layout.outline", "pivot.layout.tabular"})
        m_opsActions << QString::fromLatin1(id);
}

void CrunchWindow::pivotRibbonParts(RibbonParts &)
{
    // (the Pivot Table tab is made of commands alone; it has the Table tab's color)
}

void CrunchWindow::askValueField(int dataIndex)
{
    const int t = selectedPivot();
    if (t < 0 || !m_session) {
        notice(tr("Value Field Settings"), tr("Click a cell in a pivot table first."), false);
        return;
    }
    const PivotTableEntry &entry = m_session->workbook()->pivots().table(t);
    if (!entry.table || dataIndex < 0 || dataIndex >= entry.table->dataFields.size()) {
        notice(tr("Value Field Settings"), tr("This pivot table has no values to set."), false);
        return;
    }
    const PivotDataField &d = entry.table->dataFields[dataIndex];
    const PivotCache *cache = m_session->workbook()->pivots().cache(entry.cache).cache.get();
    QString name = d.name, function = d.subtotal;
    const QString source = cache && d.fld >= 0 && d.fld < cache->fields.size() ? cache->fields[d.fld].name : QString();
    if (m_valueFieldAsker) {
        if (!m_valueFieldAsker(&name, &function)) return;
    } else {
        ValueFieldDialog dialog(source, name, function, this);
        if (dialog.exec() != QDialog::Accepted) return;
        name = dialog.name();
        function = dialog.function();
    }
    name = name.trimmed();
    if (name.isEmpty()) name = d.name;
    // a name no field and no other value has (the program that opens the file would refuse it)
    for (int i = 0; i < entry.table->dataFields.size(); ++i)
        if (i != dataIndex && entry.table->dataFields[i].name.compare(name, Qt::CaseInsensitive) == 0) {
            notice(tr("Value Field Settings"), tr("Another value is called \"%1\". Pick another name.").arg(name));
            return;
        }
    if (cache)
        for (const PivotCacheField &f : cache->fields)
            if (f.name.compare(name, Qt::CaseInsensitive) == 0) {
                notice(tr("Value Field Settings"), tr("A field of the data is called \"%1\". Pick another name.").arg(name));
                return;
            }
    editPivot([&](PivotTable *table) {
        table->dataFields[dataIndex].name = name;
        table->dataFields[dataIndex].subtotal = function;
    }, tr("Value Field Settings"), false);
}

void CrunchWindow::showPivotFields(bool show)
{
    if (show && !m_pivotPane) {
        m_pivotDock = new QDockWidget(tr("Pivot Table Fields"), this);
        m_pivotDock->setObjectName(QStringLiteral("pivotDock"));
        m_pivotDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_pivotPane = new PivotFieldsPane(m_pivotDock);
        m_pivotDock->setWidget(m_pivotPane);
        addDockWidget(Qt::RightDockWidgetArea, m_pivotDock);
        resizeDocks({m_pivotDock}, {300}, Qt::Horizontal);
        connect(m_pivotPane, &PivotFieldsPane::changeRequested, this, [this](const PivotTable &table, const QString &text) {
            editPivot([&](PivotTable *t) { *t = table; }, text, true);
            m_pivotShown = nullptr;
            updatePivotTab();
        });
        connect(m_pivotPane, &PivotFieldsPane::valueSettingsRequested, this, [this](int i) { askValueField(i); });
        connect(m_pivotDock, &QDockWidget::visibilityChanged, this, [this](bool visible) {
            if (QAction *a = act(QStringLiteral("pivot.fieldList"))) {
                const QSignalBlocker block(a);
                a->setChecked(visible);
            }
            if (visible) {
                m_pivotShown = nullptr;
                updatePivotTab();
            }
        });
    }
    if (!m_pivotDock) return;
    m_pivotDock->setVisible(show);
    if (show) {
        m_pivotShown = nullptr;
        updatePivotTab();
    }
}

void CrunchWindow::updatePivotTab()
{
    const int t = selectedPivot();
    if (m_ribbon) m_ribbon->setContextVisible(QStringLiteral("Pivot Table"), t >= 0);
    if (!m_pivotPane || !m_pivotDock || !m_pivotDock->isVisible()) return;
    if (t < 0 || !m_session) {
        if (m_pivotShownIndex != -1 || m_pivotShown) {
            m_pivotPane->showNothing();
            m_pivotShown = nullptr;
            m_pivotShownIndex = -1;
        }
        return;
    }
    const PivotTableEntry &entry = m_session->workbook()->pivots().table(t);
    if (!entry.table || m_pivotShown == entry.table) return;
    m_pivotShown = entry.table;
    m_pivotShownIndex = t;
    // (the first time a table of a big workbook is clicked its data is read: the wait is shown)
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    const struct Restore { ~Restore() { QGuiApplication::restoreOverrideCursor(); } } restore;
    const PivotCache *cache = entry.cache >= 0 ? m_session->workbook()->pivots().cache(entry.cache).cache.get() : nullptr;
    if (!cache) {
        m_pivotPane->showNothing();
        return;
    }
    QStringList names;
    QVector<bool> numeric;
    for (int i = 0; i < cache->fields.size(); ++i) {
        names << (i < entry.table->fields.size() && !entry.table->fields[i].name.isEmpty() ? entry.table->fields[i].name : cache->fields[i].name);
        numeric << pivotFieldIsNumeric(*cache, i);
    }
    m_pivotPane->showTable(entry.table->name, *entry.table, names, numeric);
}

} // namespace jp
