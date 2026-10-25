// The Formulas tab, the parts about writing formulas and defined names: the help while a formula is typed (functions and names that fit, the arguments of the function being typed), the
// list of names, a name made for the cells selected or from the labels round them, and a name put in a formula.
#include "app/crunchwindow.h"

#include "app/formulahelper.h"
#include "app/namesdialog.h"
#include "core/functioninfo.h"
#include "core/namesfromcells.h"
#include "sheet/refshift.h"

#include <QAction>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QVBoxLayout>

#include <algorithm>

namespace jp {

namespace {

// The cells selected as cells of the model.
jp::office::sheet::CellRange cellsOf(const grid::GridRange &r)
{
    jp::office::sheet::CellRange c;
    c.valid = true;
    c.row1 = quint32(std::max<qint64>(0, r.row1));
    c.col1 = quint32(std::max<qint64>(0, r.col1));
    c.row2 = quint32(std::max<qint64>(0, r.row2));
    c.col2 = quint32(std::max<qint64>(0, r.col2));
    return c;
}

// The cells as a name writes them: "Sheet1!$A$1:$B$2".
QString absoluteRef(const QString &sheetName, const jp::office::sheet::CellRange &r)
{
    using namespace jp::office::sheet;
    QString text = sheetNamePrefix(sheetName) + QLatin1Char('$') + columnName(r.col1) + QLatin1Char('$') + QString::number(r.row1 + 1);
    if (r.row1 != r.row2 || r.col1 != r.col2) text += QStringLiteral(":$") + columnName(r.col2) + QLatin1Char('$') + QString::number(r.row2 + 1);
    return text;
}

} // namespace

// ---- the help while typing

void CrunchWindow::createFormulaHelper()
{
    m_helper = new FormulaHelper(this);
    m_helper->attach(m_cellEditor);
    m_helper->attach(m_formulaBar);
    m_helper->setSources([this]() -> const crunch::CompletionIndex & { return completionIndex(); }, [this](const crunch::Completion &c) { return completionDetail(c); });
    m_helper->setSeparator([this] { return crunch::argumentSeparator(m_session ? m_session->locale() : QLocale()); });
}

// The index the lists are made from: made again when a name was made, changed, or taken away, or another sheet is open.
const crunch::CompletionIndex &CrunchWindow::completionIndex()
{
    if (!m_session) {
        static const crunch::CompletionIndex none;
        return none;
    }
    const qint64 generation = m_session->names().generation();
    if (generation != m_completionsGeneration || m_sheet != m_completionsSheet) {
        QStringList tables;
        if (const jp::office::sheet::Workbook *book = m_session->workbook())
            for (const jp::office::sheet::Table &t : book->tables()) tables << t.name;
        m_completions.build(m_session->names(), m_sheet, tables);
        m_completionsGeneration = generation;
        m_completionsSheet = m_sheet;
    }
    return m_completions;
}

QString CrunchWindow::completionDetail(const crunch::Completion &c) const
{
    switch (c.kind) {
    case crunch::Completion::Kind::Function: {
        const crunch::FunctionDoc *doc = crunch::docOf(c.text);
        return doc ? crunch::summaryOf(*doc) : QString();
    }
    case crunch::Completion::Kind::Name: return m_session ? m_session->nameTextOf(c.id) : QString();
    case crunch::Completion::Kind::Table: return tr("A table.");
    }
    return QString();
}

// ---- names

void CrunchWindow::createNameActions()
{
    mk("formulas.names", tr("Names…"), "variable", QKeySequence(Qt::CTRL | Qt::Key_F3), [this] { showNames(); }, false, false);
    mk("formulas.defineName", tr("Define Name…"), "tag", QKeySequence(), [this] { defineNameCommand(); }, false, false);
    mk("formulas.nameFromCells", tr("Name From Cells…"), "tags", QKeySequence(), [this] { nameFromCellsCommand(); }, false, false);
    mk("formulas.useName", tr("Insert Name"), "text-cursor-input", QKeySequence(Qt::Key_F3), [this] {
        if (QMenu *menu = namesMenu()) {
            menu->exec(QCursor::pos());
            delete menu;
        }
    }, false, false);
    for (const char *id : {"formulas.names", "formulas.defineName", "formulas.nameFromCells", "formulas.useName"}) m_opsActions << QString::fromLatin1(id);
    updateEditActions();
}

void CrunchWindow::showNames()
{
    if (!m_session || !m_loaded || !commitEdit()) return;
    QString selection;
    const grid::Selection &sel = m_view->selection();
    if (sel.activeRow >= 0 && m_sheet >= 0 && m_sheet < m_loaded->book->info().sheets.size())
        selection = absoluteRef(m_loaded->book->info().sheets[m_sheet].name, cellsOf(sel.range));
    NamesDialog dialog(m_session.get(), m_sheet, selection, this);
    dialog.exec();
}

bool CrunchWindow::defineNameCommand()
{
    if (!m_session || !m_loaded || !editingAllowed() || !commitEdit()) return false;
    const grid::Selection &sel = m_view->selection();
    crunch::EditSession::NameDraft draft;
    if (sel.activeRow >= 0)
        draft.text = QLatin1Char('=') + absoluteRef(m_loaded->book->info().sheets[m_sheet].name, cellsOf(sel.range));
    if (m_nameAsker) {
        if (!m_nameAsker(&draft)) return false;
    } else {
        QStringList sheets;
        for (const jp::office::sheet::SheetEntry &e : m_loaded->book->info().sheets) sheets << (e.deleted || e.kind != jp::office::sheet::SheetKind::Worksheet ? QString() : e.name);
        NameEditDialog dialog(m_session.get(), -1, draft, sheets, this);
        if (dialog.exec() != QDialog::Accepted) return false;
        draft = dialog.draft();
    }
    const crunch::EditSession::Outcome o = m_session->defineName(draft);
    if (!o.ok) {
        notice(tr("Define Name"), o.problem);
        return false;
    }
    return true;
}

bool CrunchWindow::nameFromCellsCommand()
{
    if (!m_session || !m_loaded || !editingAllowed() || !commitEdit()) return false;
    const grid::Selection &sel = m_view->selection();
    if (sel.isOneCell()) {
        notice(tr("Name From Cells"), tr("Select the labels and the cells they name first: for example a row of headings with the numbers under them."));
        return false;
    }
    int edges = sel.range.row2 > sel.range.row1 ? crunch::TopEdge : crunch::LeftEdge;
    if (m_edgesAsker) {
        if (!m_edgesAsker(&edges)) return false;
    } else {
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Name From Cells"));
        auto *layout = new QVBoxLayout(&dialog);
        layout->addWidget(new QLabel(tr("Where are the labels?")));
        QCheckBox *boxes[4];
        const struct {
            int edge;
            QString text;
        } kinds[] = {{crunch::TopEdge, tr("In the top row")}, {crunch::LeftEdge, tr("In the left column")}, {crunch::BottomEdge, tr("In the bottom row")}, {crunch::RightEdge, tr("In the right column")}};
        for (int i = 0; i < 4; ++i) {
            boxes[i] = new QCheckBox(kinds[i].text);
            boxes[i]->setChecked(edges & kinds[i].edge);
            layout->addWidget(boxes[i]);
        }
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() != QDialog::Accepted) return false;
        edges = 0;
        for (int i = 0; i < 4; ++i)
            if (boxes[i]->isChecked()) edges |= kinds[i].edge;
    }
    if (edges == 0) return false;
    int made = 0, skipped = 0;
    const crunch::EditSession::Outcome o = m_session->defineNamesFromLabels(m_sheet, cellsOf(sel.range), edges, &made, &skipped);
    if (!o.ok) {
        notice(tr("Name From Cells"), o.problem);
        return false;
    }
    if (made == 0) {
        notice(tr("Name From Cells"), tr("No name was made: the labels are empty, repeat each other, or are names the workbook has already."));
        return false;
    }
    if (skipped > 0) setStatus(tr("%n name(s) made. %1 left out: empty, repeated, or already there.", nullptr, made).arg(skipped));
    else setStatus(tr("%n name(s) made.", nullptr, made));
    return true;
}

// The names to choose from (those of the workbook, and of the sheet in view), most of them left to the list; the caller owns the menu.
QMenu *CrunchWindow::namesMenu()
{
    if (!m_session || !m_loaded) return nullptr;
    auto *menu = new QMenu(this);
    menu->setObjectName(QStringLiteral("namesMenu"));
    std::vector<int> ids;
    for (int id = 0; id < m_session->names().slots(); ++id) {
        if (!m_session->nameListed(id)) continue;
        const crunch::NameEntry &e = m_session->names().at(id);
        if (e.hidden || (e.scope >= 0 && e.scope != m_sheet)) continue;
        ids.push_back(id);
    }
    std::sort(ids.begin(), ids.end(), [this](int a, int b) { return m_session->names().at(a).name.compare(m_session->names().at(b).name, Qt::CaseInsensitive) < 0; });
    constexpr size_t kShown = 40;
    for (size_t i = 0; i < ids.size() && i < kShown; ++i) {
        const QString name = m_session->names().at(ids[i]).name;
        QAction *a = menu->addAction(name);
        a->setToolTip(m_session->nameTextOf(ids[i]));
        connect(a, &QAction::triggered, this, [this, name] { insertName(name); });
    }
    if (ids.empty()) menu->addAction(tr("There are no names yet."))->setEnabled(false);
    menu->addSeparator();
    connect(menu->addAction(tr("All Names…")), &QAction::triggered, this, [this] { showNames(); });
    return menu;
}

// A name in the formula being typed, at the cursor (or the formula begun with it).
bool CrunchWindow::insertName(const QString &name)
{
    if (!m_session || !m_loaded || name.isEmpty()) return false;
    return insertTextInFormula(name, false);
}

bool CrunchWindow::insertTextInFormula(const QString &text, bool asFunction)
{
    Q_UNUSED(asFunction);
    if (m_edit.active) {
        QLineEdit *line = m_formulaBar->hasFocus() ? m_formulaBar : m_cellEditor;
        const QString now = line->text();
        if (now.isEmpty()) {
            line->setText(QLatin1Char('=') + text);
            Q_EMIT line->textEdited(line->text());   // (the other line shows it too)
        } else if (now.startsWith(QLatin1Char('='))) {
            line->insert(text);
            Q_EMIT line->textEdited(line->text());   // (the other line shows it too)
        } else {
            return false;   // (text is being typed: not a formula)
        }
        line->setFocus(Qt::OtherFocusReason);
        return true;
    }
    if (!editingAllowed()) return false;
    beginEdit(QLatin1Char('=') + text, CellEditor::Mode::Editing, false);
    return m_edit.active;
}

} // namespace jp
