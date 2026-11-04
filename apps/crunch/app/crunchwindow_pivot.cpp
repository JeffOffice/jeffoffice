// Pivot tables in the window (core/editpivot.cpp does the work): the Pivot Table tab (Refresh, Change Data Source, Clear, the Field List, and the Design choices: subtotals, grand totals, and
// the layout of the report), the Pivot Table Fields pane, and Refresh All on the Data tab. Every command is one step of Undo; a refusal is told in words.
#include "app/crunchwindow.h"

#include "app/pivotpane.h"
#include "app/slicerpane.h"
#include "app/settings.h"
#include "core/formulaspot.h"
#include "core/gotospecial.h"
#include "sheet/cellref.h"
#include "sheet/refshift.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "sheet/pivot.h"
#include "sheet/pivotbook.h"

#include <QDate>
#include <QUndoStack>
#include <QAction>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QRegularExpression>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QInputDialog>

#include <cmath>

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
    updateTableLooks();
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
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);   // (the data is read on this thread: a big range takes a moment)
    const auto o = m_session->createPivot(source, sheet, row, col, &made);
    QGuiApplication::restoreOverrideCursor();
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
            Settings::get().setValue(QStringLiteral("crunch/generateGetPivotData"), generate);
            return;
        }
        PivotOptionsChoice choice;
        choice.generateGetPivotData = generate;
        const int t = selectedPivot();
        const PivotTableEntry *entry = t >= 0 && m_session ? &m_session->workbook()->pivots().table(t) : nullptr;
        const PivotTable *table = entry ? entry->table.get() : nullptr;
        const PivotCache *cache = entry && entry->cache >= 0 ? m_session->workbook()->pivots().cache(entry->cache).cache.get() : nullptr;
        if (table && cache && m_session->pivotUsable(t)) {
            choice.hasTable = true;
            choice.name = table->name;
            choice.mergeLabels = table->mergeItem;
            choice.showError = table->showError;
            choice.errorText = table->errorCaption;
            choice.showMissing = table->showMissing;
            choice.missingText = table->missingCaption;
            choice.rowGrandTotals = table->rowGrandTotals;
            choice.colGrandTotals = table->colGrandTotals;
            choice.preserveFormatting = table->preserveFormatting;
            choice.autofit = table->useAutoFormatting;
            choice.showHeaders = table->showHeaders;
            choice.refreshOnOpen = cache->refreshOnLoad;
        }
        const PivotOptionsChoice was = choice;
        if (m_pivotOptionsChoiceAsker) {
            if (!m_pivotOptionsChoiceAsker(&choice)) return;
        } else {
            PivotOptionsDialog dialog(choice, this);
            if (dialog.exec() != QDialog::Accepted) return;
            choice = dialog.choice();
        }
        Settings::get().setValue(QStringLiteral("crunch/generateGetPivotData"), choice.generateGetPivotData);
        if (!choice.hasTable) return;
        if (choice.name.isEmpty()) choice.name = was.name;
        // a name no other table of the workbook has
        for (int i = 0; i < m_session->workbook()->pivots().tableCount(); ++i)
            if (i != t && m_session->workbook()->pivots().table(i).table && m_session->workbook()->pivots().table(i).table->name.compare(choice.name, Qt::CaseInsensitive) == 0) {
                notice(tr("Pivot Table Options"), tr("Another pivot table is called \"%1\". Pick another name.").arg(choice.name));
                return;
            }
        const bool tableChanged = choice.name != was.name || choice.mergeLabels != was.mergeLabels || choice.showError != was.showError || choice.errorText != was.errorText ||
                                  choice.showMissing != was.showMissing || choice.missingText != was.missingText || choice.rowGrandTotals != was.rowGrandTotals ||
                                  choice.colGrandTotals != was.colGrandTotals || choice.preserveFormatting != was.preserveFormatting || choice.autofit != was.autofit || choice.showHeaders != was.showHeaders;
        if (tableChanged)
            editPivot([&](PivotTable *table) {
                table->name = choice.name;
                table->mergeItem = choice.mergeLabels;
                table->showError = choice.showError;
                table->errorCaption = choice.errorText;
                table->showMissing = choice.showMissing;
                table->missingCaption = choice.missingText;
                table->rowGrandTotals = choice.rowGrandTotals;
                table->colGrandTotals = choice.colGrandTotals;
                table->preserveFormatting = choice.preserveFormatting;
                table->useAutoFormatting = choice.autofit;
                table->showHeaders = choice.showHeaders;
            }, tr("Pivot Table Options"), false);
        if (choice.refreshOnOpen != was.refreshOnOpen) pivotProblem(tr("Pivot Table Options"), m_session->setPivotRefreshOnOpen(t, choice.refreshOnOpen));
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
    mk("pivot.slicers", tr("Slicers"), "filter", QKeySequence(), [this] { showSlicers(act("pivot.slicers")->isChecked()); }, true);
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
    // Field Settings, expanding and collapsing, blank lines and repeated labels, the options of the style, and the styles
    mk("insert.pivotChart", tr("Pivot Chart"), "chart-column", QKeySequence(), [this] { insertPivotChart(); });
    m_opsActions << QStringLiteral("insert.pivotChart");
    mk("pivot.showDetail", tr("Show Details"), "table-properties", QKeySequence(), [this] {
        if (selectedPivot() < 0) {
            notice(tr("Show Details"), tr("Click a value of a pivot table first."), false);
            return;
        }
        showPivotDetailAtCursor(false);
    });
    m_opsActions << QStringLiteral("pivot.showDetail");
    // Select: the whole table, its values, or its labels (the lines of the rows; the heads of the columns when the rows have none)
    auto selectPart = [this](int part) {
        const int t = selectedPivot();
        if (t < 0 || !m_session) return;
        const PivotTableEntry &entry = m_session->workbook()->pivots().table(t);
        const CellRange at = m_session->workbook()->pivots().locationOf(t);
        if (!entry.table || !at.valid) return;
        quint32 r1 = at.row1, c1 = at.col1, r2 = at.row2, c2 = at.col2;
        const quint32 dataRow = std::min<quint32>(at.row1 + quint32(std::max(0, entry.table->firstDataRow)), at.row2);
        const quint32 dataCol = std::min<quint32>(at.col1 + quint32(std::max(0, entry.table->firstDataCol)), at.col2);
        if (part == 1) {
            r1 = dataRow;
            c1 = dataCol;
        } else if (part == 2) {
            if (dataCol > at.col1 && dataRow <= at.row2) {
                r1 = dataRow;
                c2 = dataCol - 1;
            } else {
                r2 = dataRow > at.row1 ? dataRow - 1 : at.row1;
                c1 = dataCol;
            }
        }
        m_view->selectRange(r1, c1, r2, c2);
        m_view->ensureVisible(r1, c1);
    };
    mk("pivot.selectAll", tr("Entire Pivot Table"), "", QKeySequence(), [selectPart] { selectPart(0); });
    mk("pivot.selectValues", tr("Values"), "", QKeySequence(), [selectPart] { selectPart(1); });
    mk("pivot.selectLabels", tr("Labels"), "", QKeySequence(), [selectPart] { selectPart(2); });
    mk("pivot.move", tr("Move Pivot Table…"), "move", QKeySequence(), [this] {
        const int t = selectedPivot();
        if (t < 0 || !editingAllowed()) return;
        if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
        const CellRange at = m_session->workbook()->pivots().locationOf(t);
        QString cell = at.valid ? columnName(at.col1) + QString::number(at.row1 + 1) : QString();
        if (m_pivotMoveAsker) {
            if (!m_pivotMoveAsker(&cell)) return;
        } else {
            bool ok = false;
            cell = QInputDialog::getText(this, tr("Move Pivot Table"), tr("Move the pivot table so that its first cell is in this cell of the same sheet:"), QLineEdit::Normal, cell, &ok);
            if (!ok) return;
        }
        quint32 row = 0, col = 0;
        QString ref = cell.trimmed();
        ref.remove(QLatin1Char('$'));
        if (!parseCellRef(ref, &row, &col)) {
            notice(tr("Move Pivot Table"), tr("Type a cell, for example J10."));
            return;
        }
        pivotProblem(tr("Move Pivot Table"), m_session->movePivot(t, row, col));
        m_pivotShown = nullptr;
        updatePivotTab();
    });
    m_opsActions << QStringLiteral("pivot.selectAll") << QStringLiteral("pivot.selectValues") << QStringLiteral("pivot.selectLabels") << QStringLiteral("pivot.move");
    mk("pivot.group", tr("Group Field…"), "group", QKeySequence(), [this] {
        const int f = m_pivotPane ? m_pivotPane->selectedField() : -1;
        if (selectedPivot() < 0) {
            notice(tr("Group Field"), tr("Click a cell in a pivot table first."), false);
            return;
        }
        if (f < 0) {
            showPivotFields(true);
            notice(tr("Group Field"), tr("Click a field in the Columns or Rows box of the Pivot Table Fields pane first."), false);
            return;
        }
        askGroupField(f);
    });
    mk("pivot.ungroup", tr("Ungroup"), "ungroup", QKeySequence(), [this] {
        const int f = m_pivotPane ? m_pivotPane->selectedField() : -1;
        const int t = selectedPivot();
        if (t < 0 || !editingAllowed()) return;
        if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
        if (f < 0) {
            notice(tr("Ungroup"), tr("Click a field in the Columns or Rows box of the Pivot Table Fields pane first."), false);
            return;
        }
        pivotProblem(tr("Ungroup"), m_session->ungroupPivotField(t, f));
        m_pivotShown = nullptr;
        updatePivotTab();
        updateTableLooks();
    });
    m_opsActions << QStringLiteral("pivot.group") << QStringLiteral("pivot.ungroup");
    mk("pivot.fieldSettings", tr("Field Settings…"), "settings", QKeySequence(), [this] {
        const int f = m_pivotPane ? m_pivotPane->selectedField() : -1;
        if (selectedPivot() < 0) {
            notice(tr("Field Settings"), tr("Click a cell in a pivot table first."), false);
            return;
        }
        if (f < 0) {
            showPivotFields(true);
            notice(tr("Field Settings"), tr("Click a field in the Filters, Columns, or Rows box of the Pivot Table Fields pane first."), false);
            return;
        }
        askFieldSettings(f);
    });
    auto expandAll = [this](bool open) {
        const int f = m_pivotPane ? m_pivotPane->selectedField() : -1;
        if (f < 0) {
            showPivotFields(true);   // (the pane opens, where the field is picked)
            notice(tr("Pivot Table"), tr("Click a field in the Columns or Rows box of the Pivot Table Fields pane first."), false);
            return;
        }
        editPivot([=](PivotTable *t) {
            if (f >= t->fields.size()) return;
            for (PivotTableItem &it : t->fields[f].items)
                if (it.t.isEmpty()) it.showDetails = open;
        }, open ? tr("Expand Field") : tr("Collapse Field"), true);
    };
    mk("pivot.expandField", tr("Expand Entire Field"), "plus", QKeySequence(), [expandAll] { expandAll(true); });
    mk("pivot.collapseField", tr("Collapse Entire Field"), "minus", QKeySequence(), [expandAll] { expandAll(false); });
    auto blankRows = [this](bool on) {
        editPivot([on](PivotTable *t) {
            for (int f : t->rowFields)
                if (f >= 0 && f < t->fields.size()) t->fields[f].insertBlankRow = on;
        }, on ? tr("Insert Blank Line after Each Item") : tr("Remove Blank Line after Each Item"), true);
    };
    mk("pivot.blank.insert", tr("Insert Blank Line after Each Item"), "", QKeySequence(), [blankRows] { blankRows(true); });
    mk("pivot.blank.remove", tr("Remove Blank Line after Each Item"), "", QKeySequence(), [blankRows] { blankRows(false); });
    auto repeat = [this](bool on) {
        editPivot([on](PivotTable *t) {
            for (PivotField &pf : t->fields) pf.fillDownLabels = on;
        }, on ? tr("Repeat All Item Labels") : tr("Do Not Repeat Item Labels"), true);
    };
    mk("pivot.repeat.on", tr("Repeat All Item Labels"), "", QKeySequence(), [repeat] { repeat(true); });
    mk("pivot.repeat.off", tr("Do Not Repeat Item Labels"), "", QKeySequence(), [repeat] { repeat(false); });
    auto styleOption = [this](const QString &text, std::function<void(PivotTable *)> change) {
        editPivot([change](PivotTable *t) {
            t->hasStyleInfo = true;
            change(t);
        }, text, false);
    };
    mk("pivot.style.rowHeaders", tr("Row Headers"), "", QKeySequence(), [this, styleOption] { const bool on = act("pivot.style.rowHeaders")->isChecked(); styleOption(tr("Row Headers"), [on](PivotTable *t) { t->showRowHeaders = on; }); }, true);
    mk("pivot.style.colHeaders", tr("Column Headers"), "", QKeySequence(), [this, styleOption] { const bool on = act("pivot.style.colHeaders")->isChecked(); styleOption(tr("Column Headers"), [on](PivotTable *t) { t->showColHeaders = on; }); }, true);
    mk("pivot.style.rowStripes", tr("Banded Rows"), "", QKeySequence(), [this, styleOption] { const bool on = act("pivot.style.rowStripes")->isChecked(); styleOption(tr("Banded Rows"), [on](PivotTable *t) { t->showRowStripes = on; }); }, true);
    mk("pivot.style.colStripes", tr("Banded Columns"), "", QKeySequence(), [this, styleOption] { const bool on = act("pivot.style.colStripes")->isChecked(); styleOption(tr("Banded Columns"), [on](PivotTable *t) { t->showColStripes = on; }); }, true);
    // the styles of the gallery: a name each (the file's names), drawn from our own looks
    static const char *const kStyles[] = {"PivotStyleLight1", "PivotStyleLight9", "PivotStyleLight16", "PivotStyleLight17", "PivotStyleLight21", "PivotStyleMedium2", "PivotStyleMedium9",
                                          "PivotStyleMedium16", "PivotStyleDark2", "PivotStyleDark9"};
    mk("pivot.style.none", tr("Clear Style"), "", QKeySequence(), [this, styleOption] { styleOption(tr("Pivot Table Style"), [](PivotTable *t) { t->styleName.clear(); }); });
    for (const char *name : kStyles) {
        const QString style = QString::fromLatin1(name);
        QString label = style.mid(10);
        label.replace(QRegularExpression(QStringLiteral("^([A-Za-z]+)(\\d+)$")), QStringLiteral("\\1 \\2"));
        mk(("pivot.style." + style).toLatin1().constData(), label, "", QKeySequence(), [styleOption, style, this] { styleOption(tr("Pivot Table Style"), [style](PivotTable *t) { t->styleName = style; }); });
        m_opsActions << (QStringLiteral("pivot.style.") + style);
    }
    for (const char *id : {"pivot.fieldSettings", "pivot.expandField", "pivot.collapseField", "pivot.blank.insert", "pivot.blank.remove", "pivot.repeat.on", "pivot.repeat.off", "pivot.style.rowHeaders",
                           "pivot.style.colHeaders", "pivot.style.rowStripes", "pivot.style.colStripes", "pivot.style.none"})
        m_opsActions << QString::fromLatin1(id);
    for (const char *id : {"pivot.refresh", "pivot.refreshAll", "pivot.changeSource", "pivot.clearAll", "pivot.clearFilters", "pivot.fieldList", "pivot.slicers", "pivot.valueSettings", "pivot.sub.none", "pivot.sub.top",
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
    ValueFieldSettings settings;
    settings.name = d.name;
    settings.function = d.subtotal;
    settings.showAs = d.showDataAs;
    settings.baseField = d.baseField;
    settings.baseItem = d.baseItem;
    settings.numFmtId = d.numFmtId;
    settings.sourceName = cache && d.fld >= 0 && d.fld < cache->fields.size() ? cache->fields[d.fld].name : QString();
    // the fields a value can be shown against: those on the axes, with their items
    if (cache) {
        auto addBase = [&](int f) {
            if (f < 0 || f >= entry.table->fields.size() || f >= cache->fields.size()) return;
            for (const auto &b : settings.bases)
                if (b.field == f) return;
            ValueFieldSettings::Base base;
            base.field = f;
            const PivotField &pf = entry.table->fields[f];
            base.name = pf.name.isEmpty() ? cache->fields[f].name : pf.name;
            auto label = [&](int shared) {
                if (shared < 0 || shared >= cache->fields[f].items.size()) return QString();
                const PivotValue &v = cache->fields[f].items[shared];
                if (v.kind == PivotValue::Missing) return tr("(blank)");
                return v.isNumeric() || v.kind == PivotValue::Bool ? QString::number(v.number, 'g', 15) : v.text;
            };
            if (pf.hasItems) {
                for (const PivotTableItem &it : pf.items)
                    if (it.t.isEmpty()) base.items << (it.n.isEmpty() ? label(it.x) : it.n);
            } else {
                for (int i = 0; i < cache->fields[f].items.size(); ++i) base.items << label(i);
            }
            settings.bases.append(base);
        };
        for (int f : entry.table->rowFields) addBase(f);
        for (int f : entry.table->colFields) addBase(f);
    }
    if (m_valueSettingsAsker) {
        if (!m_valueSettingsAsker(&settings)) return;
    } else if (m_valueFieldAsker) {
        QString n = settings.name, fn = settings.function;
        if (!m_valueFieldAsker(&n, &fn)) return;
        settings.name = n;
        settings.function = fn;
    } else {
        ValueFieldDialog dialog(settings, this);
        if (dialog.exec() != QDialog::Accepted) return;
        settings = dialog.settings();
    }
    QString name = settings.name.trimmed();
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
    const bool relative = settings.showAs == QLatin1String("difference") || settings.showAs == QLatin1String("percent") || settings.showAs == QLatin1String("percentDiff") || settings.showAs == QLatin1String("runTotal");
    if (relative && settings.bases.isEmpty()) {
        notice(tr("Value Field Settings"), tr("Put a field in the rows or the columns first. A value is shown against the items of a field."));
        return;
    }
    // (a file with no cell formats of its own has nowhere to keep a number format: the person is told, and the values keep the format they have)
    const bool canFormat = !m_session->workbook()->styles().formats.isEmpty();
    if (!canFormat && (!settings.numFmtCode.isEmpty() || settings.showAs == QLatin1String("percentOfTotal") || settings.showAs == QLatin1String("percentOfRow") || settings.showAs == QLatin1String("percentOfCol")))
        notice(tr("Value Field Settings"), tr("This workbook has no cell formats, so a number format can't be kept in it. The values keep the format they have."), false);
    editPivot([&](PivotTable *table) {
        PivotDataField &df = table->dataFields[dataIndex];
        df.name = name;
        df.subtotal = settings.function;
        df.showDataAs = settings.showAs;
        df.baseField = relative ? settings.baseField : 0;
        df.baseItem = relative && settings.showAs != QLatin1String("runTotal") ? settings.baseItem : 0;
        if (!canFormat) {
        } else if (!settings.numFmtCode.isEmpty()) {
            df.numFmtCode = settings.numFmtCode;
        } else if (settings.showAs != d.showDataAs && settings.showAs != QLatin1String("normal") && settings.showAs != QLatin1String("difference") && settings.showAs != QLatin1String("runTotal") && settings.showAs != QLatin1String("index")) {
            df.numFmtCode = QStringLiteral("0.00%");   // (a share shows as a percent unless a format is chosen)
        }
    }, tr("Value Field Settings"), false);
}

void CrunchWindow::askFieldSettings(int field)
{
    const int t = selectedPivot();
    if (t < 0 || !m_session) {
        notice(tr("Field Settings"), tr("Click a cell in a pivot table first."), false);
        return;
    }
    const PivotTableEntry &entry = m_session->workbook()->pivots().table(t);
    const PivotCache *cache = entry.cache >= 0 ? m_session->workbook()->pivots().cache(entry.cache).cache.get() : nullptr;
    if (!entry.table || !cache || field < 0 || field >= entry.table->fields.size() || field >= cache->fields.size()) return;
    const PivotField &pf = entry.table->fields[field];
    FieldSettings fs;
    fs.field = field;
    fs.sourceName = cache->fields[field].name;
    fs.name = pf.name.isEmpty() ? fs.sourceName : pf.name;
    bool total = pf.defaultSubtotal;
    if (pf.hasItems) {
        total = false;
        for (const PivotTableItem &it : pf.items) total = total || it.t == QLatin1String("default");
    }
    fs.subtotals = total;
    fs.form = pf.compact ? 2 : (pf.outline ? 1 : 0);
    fs.blankRow = pf.insertBlankRow;
    fs.repeatLabels = pf.fillDownLabels;
    fs.sort = pf.sortType == QLatin1String("ascending") ? 1 : pf.sortType == QLatin1String("descending") ? 2 : 0;
    if (pf.sortByData >= 0) {
        fs.sortByData = pf.sortByData;
        fs.sortByDescending = pf.sortType == QLatin1String("descending");
        fs.sort = 0;
    }
    for (const PivotDataField &d : entry.table->dataFields) fs.dataNames << d.name;
    auto label = [&](int shared) {
        if (shared < 0 || shared >= cache->fields[field].items.size()) return QString();
        const PivotValue &v = cache->fields[field].items[shared];
        if (v.kind == PivotValue::Missing) return tr("(blank)");
        return v.isNumeric() || v.kind == PivotValue::Bool ? QString::number(v.number, 'g', 15) : v.text;
    };
    if (pf.hasItems) {
        for (const PivotTableItem &it : pf.items)
            if (it.t.isEmpty()) fs.items.append({it.n.isEmpty() ? label(it.x) : it.n, it.x, !it.hidden});
    } else {
        for (int i = 0; i < cache->fields[field].items.size(); ++i) fs.items.append({label(i), i, true});
    }
    if (m_fieldSettingsAsker) {
        if (!m_fieldSettingsAsker(&fs)) return;
    } else {
        FieldSettingsDialog dialog(fs, this);
        if (dialog.exec() != QDialog::Accepted) return;
        fs = dialog.settings();
    }
    QString name = fs.name.trimmed();
    if (name == fs.sourceName) name.clear();
    if (!name.isEmpty()) {
        for (int i = 0; i < entry.table->dataFields.size(); ++i)
            if (entry.table->dataFields[i].name.compare(name, Qt::CaseInsensitive) == 0) {
                notice(tr("Field Settings"), tr("A value is called \"%1\". Pick another name.").arg(name));
                return;
            }
        for (int i = 0; i < cache->fields.size(); ++i)
            if (i != field && cache->fields[i].name.compare(name, Qt::CaseInsensitive) == 0) {
                notice(tr("Field Settings"), tr("Another field of the data is called \"%1\". Pick another name.").arg(name));
                return;
            }
    }
    QString problem;
    const PivotCache &c = *cache;
    bool ok = true;
    editPivot([&](PivotTable *table) {
        PivotField &f = table->fields[field];
        f.name = name;
        // the subtotal: automatic is the default line, none is no line
        QVector<PivotTableItem> kept;
        for (const PivotTableItem &it : f.items)
            if (it.t.isEmpty()) kept.append(it);
        if (fs.subtotals) {
            f.defaultSubtotal = true;
            if (f.hasItems) {
                PivotTableItem totalLine;
                totalLine.t = QStringLiteral("default");
                kept.append(totalLine);
            }
        } else {
            f.defaultSubtotal = false;
            f.subtotals.clear();
        }
        if (f.hasItems) f.items = kept;
        f.compact = fs.form == 2;
        f.outline = fs.form != 0;
        f.subtotalTop = fs.form != 0;
        f.insertBlankRow = fs.blankRow;
        f.fillDownLabels = fs.repeatLabels;
        // the items that show
        if (fs.itemsChanged || fs.filter.kind != PivotFilter::None) {
            if (!f.hasItems && !pivotFilterField(table, c, field, PivotFilter(), &problem)) ok = false;
            for (const auto &it : fs.items)
                for (PivotTableItem &ti : table->fields[field].items)
                    if (ti.t.isEmpty() && ti.x == it.x) ti.hidden = !it.shown;
            if (ok && fs.filter.kind != PivotFilter::None) ok = pivotFilterField(table, c, field, fs.filter, &problem);
        }
        if (fs.sortByData >= 0) pivotSortFieldByValue(table, field, fs.sortByData, fs.sortByDescending);
        else pivotSortField(table, c, field, fs.sort == 1 ? PivotSort::Ascending : fs.sort == 2 ? PivotSort::Descending : PivotSort::Manual);
    }, tr("Field Settings"), true);
    if (!ok) notice(tr("Field Settings"), tr("The filter can't be worked out: %1.").arg(problem));
}

bool CrunchWindow::showPivotDetailAtCursor(bool onlyForValues)
{
    if (!m_session || !m_loaded || !m_haveCalc || m_chartSheetShown || m_edit.active) return false;
    const grid::Selection &sel = m_view->selection();
    const quint32 row = quint32(std::max<qint64>(0, sel.activeRow)), col = quint32(std::max<qint64>(0, sel.activeCol));
    if (onlyForValues && !m_session->isPivotValue(m_sheet, row, col)) return false;
    if (!editingAllowed()) return true;
    int made = -1;
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    const auto o = m_session->showPivotDetail(m_sheet, row, col, &made);
    QGuiApplication::restoreOverrideCursor();
    if (!o.ok) {
        notice(tr("Show Details"), o.problem);
        return true;
    }
    if (made >= 0) goToFound(made, 0, 0);
    return true;
}

void CrunchWindow::askGroupField(int field)
{
    const int t = selectedPivot();
    if (t < 0 || !m_session) {
        notice(tr("Group Field"), tr("Click a cell in a pivot table first."), false);
        return;
    }
    if (!editingAllowed()) return;
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    QString why;
    if (!m_session->pivotUsable(t, &why)) {
        notice(tr("Group Field"), why);
        return;
    }
    const PivotTableEntry &entry = m_session->workbook()->pivots().table(t);
    const PivotCache *cache = entry.cache >= 0 ? m_session->workbook()->pivots().cache(entry.cache).cache.get() : nullptr;
    if (!cache || field < 0 || field >= cache->fields.size()) return;
    const PivotCacheField &cf = cache->fields[field];
    if (cf.group.kind != PivotFieldGroup::None) {
        notice(tr("Group Field"), tr("This field is already grouped. Ungroup it first to group it another way."));
        return;
    }
    GroupChoice choice;
    bool anyDate = false, first = true;
    double lo = 0, hi = 0;
    for (const PivotValue &v : cf.items) {
        if (!v.isNumeric()) {
            notice(tr("Group Field"), tr("This field has text or empty cells, so its values can't be grouped by number or date."));
            return;
        }
        anyDate = anyDate || v.kind == PivotValue::Date;
        lo = first ? v.number : std::min(lo, v.number);
        hi = first ? v.number : std::max(hi, v.number);
        first = false;
    }
    if (first) return;
    choice.isDate = anyDate;
    const jp::calc::DateSystem dates = m_session->workbook()->info().dateSystem;
    auto dateText = [&](double serial) {
        int y = 0, m = 0, d = 0;
        if (!jp::calc::serialToDate(std::floor(serial), dates, &y, &m, &d)) return QString::number(serial, 'g', 15);
        return QStringLiteral("%1/%2/%3").arg(m).arg(d).arg(y);
    };
    if (anyDate) {
        choice.levels = {QStringLiteral("months")};
        choice.startText = dateText(lo);
        choice.endText = dateText(hi);
    } else {
        choice.startText = QString::number(lo, 'g', 15);
        choice.endText = QString::number(hi, 'g', 15);
        choice.stepText = QString::number(std::max(1.0, std::round((hi - lo) / 10)), 'g', 15);
    }
    const GroupChoice shown = choice;
    if (m_groupAsker) {
        if (!m_groupAsker(&choice)) return;
        choice.autoStart = choice.startText == shown.startText;
        choice.autoEnd = choice.endText == shown.endText;
    } else {
        GroupDialog dialog(choice, this);
        if (dialog.exec() != QDialog::Accepted) return;
        choice = dialog.choice();
    }
    auto number = [&](const QString &text, double *out) {
        if (!anyDate) {
            bool ok = false;
            *out = text.toDouble(&ok);
            return ok;
        }
        return jp::calc::dateValueOf(text, dates, out);
    };
    double start = lo, end = hi, step = 1;
    if (!choice.autoStart && !number(choice.startText, &start)) {
        notice(tr("Group Field"), tr("Type a start the field's values can be compared to."));
        return;
    }
    if (!choice.autoEnd && !number(choice.endText, &end)) {
        notice(tr("Group Field"), tr("Type an end the field's values can be compared to."));
        return;
    }
    if (!anyDate) {
        bool ok = false;
        step = choice.stepText.toDouble(&ok);
        if (!ok || step <= 0) {
            notice(tr("Group Field"), tr("Type a step that is more than zero."));
            return;
        }
    } else if (choice.levels.isEmpty()) {
        notice(tr("Group Field"), tr("Choose what to group the dates by."));
        return;
    }
    pivotProblem(tr("Group Field"), m_session->groupPivotField(t, field, anyDate ? choice.levels : QStringList(), start, end, step, choice.autoStart, choice.autoEnd));
    m_pivotShown = nullptr;
    updatePivotTab();
    updateTableLooks();
}

void CrunchWindow::fillSlicers()
{
    if (!m_slicerPane || !m_session || m_slicerFilling) return;
    m_slicerFilling = true;
    ++m_slicerFills;
    m_slicersStamp = m_session->pivotTablesStamp();
    const struct Done { bool *flag; ~Done() { *flag = false; } } done{&m_slicerFilling};
    QVector<SlicerPane::Entry> entries;
    for (const crunch::EditSession::SlicerInfo &info : m_session->slicers()) {
        SlicerPane::Entry e;
        e.caption = info.caption;
        e.style = info.style;
        e.timeline = info.cache.timeline;
        e.usable = info.field >= 0;
        e.items = info.itemTexts;
        e.tables = info.tables;
        e.columns = info.columns;
        for (int i = 0; i < e.items.size(); ++i) e.chosen.append(info.shown.contains(i));
        if (e.timeline) {
            // (a span is shown when both its dates are real; else the timeline shows all dates)
            const QDate from = QDate::fromString(info.cache.startDate.left(10), Qt::ISODate), to = QDate::fromString(info.cache.endDate.left(10), Qt::ISODate);
            if (from.isValid() && to.isValid()) e.span = tr("%1 to %2").arg(from.toString(Qt::TextDate), to.toString(Qt::TextDate));
            e.level = info.cache.level;
        }
        entries.append(e);
    }
    m_slicerPane->setEntries(entries);
}

void CrunchWindow::showSlicers(bool show)
{
    if (show && !m_slicerPane) {
        m_slicerDock = new QDockWidget(tr("Slicers"), this);
        m_slicerDock->setObjectName(QStringLiteral("slicerDock"));
        m_slicerDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_slicerPane = new SlicerPane(m_slicerDock);
        m_slicerDock->setWidget(m_slicerPane);
        addDockWidget(Qt::RightDockWidgetArea, m_slicerDock);
        resizeDocks({m_slicerDock}, {260}, Qt::Horizontal);
        connect(m_slicerPane, &SlicerPane::itemClicked, this, [this](int slicer, int item, bool extend) {
            QSet<qint32> shown;
            qint64 count = 0;
            if (!m_session || !m_session->slicerChoice(slicer, &shown, &count)) return;
            if (extend) {
                if (shown.size() >= count) {
                    shown.clear();   // (all were chosen: Ctrl-click takes the item out of them)
                    for (qint32 i = 0; i < count; ++i)
                        if (i != item) shown.insert(i);
                } else if (shown.contains(item)) {
                    shown.remove(item);
                } else {
                    shown.insert(item);
                }
            } else if (shown.size() == 1 && shown.contains(item)) {
                shown.clear();   // (the only one chosen is clicked again: the filter goes)
            } else {
                shown = {qint32(item)};
            }
            if (shown.isEmpty() || shown.size() >= count) shown.clear();
            pivotProblem(tr("Slicer"), m_session->chooseSlicerItems(slicer, shown));
            m_pivotShown = nullptr;
            updatePivotTab();
            updateTableLooks();
            fillSlicers();
        });
        connect(m_slicerPane, &SlicerPane::connectionsRequested, this, [this](int slicer) {
            const auto all = m_session ? m_session->slicers() : std::vector<crunch::EditSession::SlicerInfo>();
            if (slicer < 0 || slicer >= int(all.size())) return;
            const auto &info = all[size_t(slicer)];
            notice(tr("Report Connections"), info.tables.isEmpty() ? tr("\"%1\" filters no pivot table that JeffCrunch can read.").arg(info.caption)
                                                                    : tr("\"%1\" filters: %2.").arg(info.caption, info.tables.join(QStringLiteral(", "))), false);
        });
        connect(m_slicerDock, &QDockWidget::visibilityChanged, this, [this](bool visible) {
            if (QAction *a = act(QStringLiteral("pivot.slicers"))) {
                const QSignalBlocker block(a);
                a->setChecked(visible);
            }
            if (visible) fillSlicers();
        });
    }
    if (!m_slicerDock) return;
    m_slicerDock->setVisible(show);
    if (show) fillSlicers();
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
        connect(m_pivotPane, &PivotFieldsPane::fieldSelectionChanged, this, [this] { updatePivotFieldActions(); });
        connect(m_pivotPane, &PivotFieldsPane::fieldSettingsRequested, this, [this](int f) { askFieldSettings(f); });
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

void CrunchWindow::updatePivotFieldActions()
{
    // (a command that works on a pivot table's cell is dimmed away from one, not left to say so when it is used)
    const bool open = m_session && !m_saving && m_loaded && m_loaded->problem.isEmpty() && !m_chartSheetShown;
    const bool inTable = selectedPivot() >= 0;
    for (const char *id : {"pivot.fieldSettings", "pivot.expandField", "pivot.collapseField", "pivot.group", "pivot.ungroup", "pivot.showDetail", "pivot.selectAll", "pivot.selectValues", "pivot.selectLabels", "pivot.move"})
        if (QAction *a = act(QString::fromLatin1(id))) a->setEnabled(open && inTable);
}

void CrunchWindow::updatePivotTab()
{
    updatePivotFieldActions();
    if (m_slicerPane && m_slicerDock && m_slicerDock->isVisible() && m_session && !m_slicerFilling && m_session->pivotTablesStamp() != m_slicersStamp) fillSlicers();   // (an Undo, a Redo, or an edit of a table changes what a slicer shows: not a move of the cursor)
    const int t = selectedPivot();
    if (m_ribbon) m_ribbon->setContextVisible(QStringLiteral("Pivot Table"), t >= 0);
    if (t >= 0 && m_session) {
        // (the style's choices show as they are for the table the cursor is in)
        const PivotTableEntry &now = m_session->workbook()->pivots().table(t);
        if (now.table) {
            const std::pair<const char *, bool> marks[] = {{"pivot.style.rowHeaders", now.table->showRowHeaders}, {"pivot.style.colHeaders", now.table->showColHeaders},
                                                           {"pivot.style.rowStripes", now.table->showRowStripes}, {"pivot.style.colStripes", now.table->showColStripes}};
            for (const auto &m : marks)
                if (QAction *a = act(QString::fromLatin1(m.first))) {
                    const QSignalBlocker block(a);
                    a->setChecked(m.second);
                }
        }
    }
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
    m_pivotPane->showTable(entry.table->name, *entry.table, names, numeric, entry.table.get());
}

} // namespace jp
