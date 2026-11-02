// Pivot tables in the window (core/editpivot.cpp does the work): the PivotTable tab (Refresh, Change Data Source, Clear, the Field List, and the Design choices: subtotals, grand totals, and
// the layout of the report), the PivotTable Fields pane, and Refresh All on the Data tab. Every command is one step of Undo; a refusal is told in words.
#include "app/crunchwindow.h"

#include "app/pivotpane.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "sheet/pivot.h"
#include "sheet/pivotbook.h"

#include <QAction>
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
        notice(tr("PivotTable"), tr("Click a cell in a PivotTable first."), false);
        return;
    }
    if (!editingAllowed()) return;
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    QString why;
    if (!m_session->pivotUsable(t, &why)) {
        notice(tr("PivotTable"), why);
        return;
    }
    PivotTable table = *m_session->workbook()->pivots().table(t).table;
    change(&table);
    pivotProblem(text, m_session->setPivot(t, table, structureChanged, text));
}

void CrunchWindow::createPivotActions()
{
    mk("pivot.refresh", tr("Refresh"), "refresh-cw", QKeySequence(), [this] {
        const int t = selectedPivot();
        if (t < 0) {
            notice(tr("Refresh"), tr("Click a cell in a PivotTable first."), false);
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
            notice(tr("Change Data Source"), tr("Click a cell in a PivotTable first."), false);
            return;
        }
        if (!editingAllowed()) return;
        if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
        QString where = m_session->pivotSourceText(t);
        if (m_pivotSourceAsker) {
            if (!m_pivotSourceAsker(&where)) return;
        } else {
            bool ok = false;
            where = QInputDialog::getText(this, tr("Change PivotTable Data Source"), tr("The cells the data is in, with a row of names above it (for example Sheet1!A1:D100), or the name of a table:"), QLineEdit::Normal, where, &ok);
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
    // (the PivotTable tab is made of commands alone; it has the Table tab's color)
}

void CrunchWindow::askValueField(int dataIndex)
{
    const int t = selectedPivot();
    if (t < 0 || !m_session) {
        notice(tr("Value Field Settings"), tr("Click a cell in a PivotTable first."), false);
        return;
    }
    const PivotTableEntry &entry = m_session->workbook()->pivots().table(t);
    if (!entry.table || dataIndex < 0 || dataIndex >= entry.table->dataFields.size()) {
        notice(tr("Value Field Settings"), tr("This PivotTable has no values to set."), false);
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
        m_pivotDock = new QDockWidget(tr("PivotTable Fields"), this);
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
    if (m_ribbon) m_ribbon->setContextVisible(QStringLiteral("PivotTable"), t >= 0);
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
    if (!entry.table || m_pivotShown == entry.table.get()) return;
    m_pivotShown = entry.table.get();
    m_pivotShownIndex = t;
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
