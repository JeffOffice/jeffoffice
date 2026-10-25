// Find and Replace, Go To, and Select by Kind on the window: the dialog, the search made a few thousand cells at a time between events, and the selection the results lead to. The matching is
// textmatch.h, the walk over the cells FindRun, and Replace one step of the edit session.
#include "app/crunchwindow.h"

#include "app/crunchinfo.h"
#include "app/finddialog.h"
#include "sheet/cellref.h"

#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QVBoxLayout>

namespace jp {

using jp::office::sheet::CellRange;

namespace {
}

void CrunchWindow::createFindActions()
{
    mk("edit.find", tr("Find"), "search", QKeySequence(), [this] { showFindDialog(false); });
    mk("find.find", tr("Find…"), "search", QKeySequence(QKeySequence::Find), [this] { showFindDialog(false); });
    mk("find.replace", tr("Replace…"), "replace", QKeySequence(QKeySequence::Replace), [this] { showFindDialog(true); });
    mk("find.next", tr("Next Match"), "", QKeySequence(QKeySequence::FindNext), [this] {
        if (m_findDialog && !m_findDialog->findText().isEmpty()) startSearch(SearchFor::Next);
        else showFindDialog(false);
    });
    mk("find.goto", tr("Go To…"), "locate", QKeySequence(Qt::CTRL | Qt::Key_G), [this] { goToDialog(); });
    act("find.goto")->setShortcuts({QKeySequence(Qt::Key_F5), QKeySequence(Qt::CTRL | Qt::Key_G)});
    mk("find.gotoSpecial", tr("Select by Kind…"), "", QKeySequence(), [this] { goToSpecialDialog(); });
    for (const char *id : {"edit.find", "find.find", "find.replace", "find.next", "find.goto", "find.gotoSpecial"}) m_opsActions << QString::fromLatin1(id);
    m_findTimer.setInterval(0);
    m_findTimer.setSingleShot(false);
    connect(&m_findTimer, &QTimer::timeout, this, [this] { stepSearch(); });
    m_replaceTimer.setInterval(0);
    connect(&m_replaceTimer, &QTimer::timeout, this, [this] { stepReplace(); });
    m_applyTimer.setInterval(0);
    connect(&m_applyTimer, &QTimer::timeout, this, [this] { stepApply(); });
    connect(m_view, &grid::GridView::selectionChanged, this, [this] {
        if (!m_settingMarks && !m_view->marks().empty()) m_view->setMarks({});
    });
    updateEditActions();
}

void CrunchWindow::showFindDialog(bool replace)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !m_session || !m_loaded) return;
    if (!m_findDialog) {
        m_findDialog = new FindDialog(this);
        m_findDialog->setAttribute(Qt::WA_DeleteOnClose, false);
        connect(m_findDialog, &FindDialog::findNextRequested, this, [this] { startSearch(SearchFor::Next); });
        connect(m_findDialog, &FindDialog::findAllRequested, this, [this] { startSearch(SearchFor::All); });
        connect(m_findDialog, &FindDialog::replaceRequested, this, [this] { replaceCurrent(); });
        connect(m_findDialog, &FindDialog::replaceAllRequested, this, [this] { startSearch(SearchFor::ReplaceAll); });
        connect(m_findDialog, &FindDialog::cancelRequested, this, [this] { cancelSearch(); });
        connect(m_findDialog, &FindDialog::resultChosen, this, [this](int sheet, quint32 row, quint32 col) { goToFound(sheet, row, col); });
        connect(m_findDialog, &QDialog::finished, this, [this] { cancelSearch(); });
    }
    m_findDialog->setTab(replace ? FindDialog::Tab::Replace : FindDialog::Tab::Find);
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

crunch::FindOptions CrunchWindow::searchOptions(bool forReplace) const
{
    crunch::FindOptions o = m_findDialog->options();
    if (forReplace) o.lookIn = crunch::FindOptions::LookIn::Formulas;
    o.sheet = m_sheet;
    // (with more than one cell selected, the search is in them)
    const grid::GridRange r = m_view->selection().range;
    if (o.scope == crunch::FindOptions::Scope::Sheet && (r.rows() > 1 || r.cols() > 1) && m_view->selection().whole == grid::Selection::Whole::None) o.within = selectedRange();
    return o;
}

void CrunchWindow::startSearch(SearchFor what)
{
    if (!m_findDialog || !m_session || !m_loaded) return;
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active) return;
    if (m_findRun) cancelSearch();
    const bool replacing = what == SearchFor::ReplaceAll;
    if (replacing) {
        QString why;
        if (!m_session->canEdit(&why)) {
            m_findDialog->setStatus(why);
            return;
        }
    }
    const crunch::FindOptions options = searchOptions(replacing);
    crunch::FoundCell from;
    from.sheet = m_sheet;
    from.row = quint32(std::max<qint64>(0, m_view->selection().activeRow));
    from.col = quint32(std::max<qint64>(0, m_view->selection().activeCol));
    m_searchFor = what;
    m_findRun = std::make_unique<crunch::FindRun>(m_session.get(), options, what == SearchFor::Next ? crunch::FindRun::Want::Next : crunch::FindRun::Want::All, from);
    m_searchChanges = m_loader.changesMade();
    m_searchSteps = m_session->undoStack()->index();
    if (what == SearchFor::All) m_findDialog->clearResults();
    m_findDialog->setStatus(QString());
    if (m_findRun->done()) {
        finishSearch();
        return;
    }
    m_findDialog->setBusy(true);
    setStatus(tr("Searching…"));
    m_findTimer.start();
}

void CrunchWindow::stepSearch()
{
    if (!m_findRun) {
        m_findTimer.stop();
        return;
    }
    // (the workbook was changed while the search was going on: it is given up)
    if (!m_session || m_loader.changesMade() != m_searchChanges || m_session->undoStack()->index() != m_searchSteps) {
        cancelSearch();
        return;
    }
    m_searchTuner.begin();
    const qint64 before = m_findRun->cellsLooked();
    const bool done = m_findRun->step(m_searchTuner.budget());
    m_searchTuner.end(m_findRun->cellsLooked() - before);
    if (done) finishSearch();
}

void CrunchWindow::cancelSearch()
{
    const bool was = bool(m_findRun) || bool(m_replacePlan);
    m_findTimer.stop();
    m_replaceTimer.stop();
    m_findRun.reset();
    m_replacePlan.reset();
    if (m_findDialog) {
        m_findDialog->setBusy(false);
        if (was) m_findDialog->setStatus(tr("The search was stopped."));
    }
    if (was && !m_calculating && !m_loading) setStatus(tr("Ready"));
}

void CrunchWindow::goToFound(int sheet, quint32 row, quint32 col)
{
    if (sheet != m_sheet) {
        setActiveSheet(sheet);
        if (m_sheet != sheet) return;
    }
    m_view->selectCell(row, col);
    m_view->ensureVisible(row, col);
}

void CrunchWindow::finishSearch()
{
    m_findTimer.stop();
    std::unique_ptr<crunch::FindRun> run = std::move(m_findRun);
    if (!run || !m_findDialog) return;
    m_findDialog->setBusy(false);
    if (!m_calculating && !m_loading) setStatus(tr("Ready"));
    QStringList sheetNames;
    if (m_loaded && m_loaded->book)
        for (const auto &e : m_loaded->book->info().sheets) sheetNames << e.name;
    const std::vector<crunch::FoundCell> &hits = run->hits();
    switch (m_searchFor) {
    case SearchFor::Next:
        if (!run->problem().isEmpty()) {
            m_findDialog->setStatus(run->problem());
        } else if (hits.empty()) {
            m_findDialog->setStatus(tr("Nothing matches \"%1\".").arg(m_findDialog->findText().left(60)));
        } else {
            goToFound(hits.front().sheet, hits.front().row, hits.front().col);
            m_findDialog->setStatus(tr("Match in %1.").arg((hits.front().sheet >= 0 && hits.front().sheet < sheetNames.size() ? sheetNames[hits.front().sheet] + QLatin1Char('!') : QString())
                                                           + jp::office::sheet::cellName(hits.front().row, hits.front().col)));
        }
        break;
    case SearchFor::All:
        if (!run->problem().isEmpty()) m_findDialog->setStatus(run->problem());
        else m_findDialog->showResults(hits, sheetNames, run->truncated());
        break;
    case SearchFor::ReplaceAll: {
        if (!run->problem().isEmpty()) {
            m_findDialog->setStatus(run->problem());
            break;
        }
        if (hits.empty()) {
            m_findDialog->setStatus(tr("Nothing matches \"%1\", so nothing was changed.").arg(m_findDialog->findText().left(60)));
            break;
        }
        if (run->truncated()) {
            const QString text = tr("More cells match than JeffCrunch changes in one go. Narrow the search, then change again.");
            m_findDialog->setStatus(text);
            notice(tr("Change All"), text);
            break;
        }
        std::vector<crunch::EditSession::CellSpot> spots;
        spots.reserve(hits.size());
        for (const crunch::FoundCell &h : hits) spots.push_back({h.sheet, h.row, h.col});
        QString why;
        m_replacePlan = m_session->planReplace(spots, m_findDialog->options().pattern, m_findDialog->replacement(), &why);
        if (!m_replacePlan) {
            m_findDialog->setStatus(why);
            notice(tr("Change All"), why);
            break;
        }
        m_searchChanges = m_loader.changesMade();
        m_searchSteps = m_session->undoStack()->index();
        m_findDialog->setBusy(true);
        setStatus(tr("Changing…"));
        m_replaceTimer.start();
        break;
    }
    }
}

void CrunchWindow::stepReplace()
{
    if (!m_replacePlan) {
        m_replaceTimer.stop();
        return;
    }
    if (!m_session || m_loader.changesMade() != m_searchChanges || m_session->undoStack()->index() != m_searchSteps) {
        cancelSearch();
        return;
    }
    m_replaceTuner.begin();
    const bool done = m_session->prepareReplace(*m_replacePlan, m_replaceTuner.budget());
    m_replaceTuner.end(m_replaceTuner.budget());
    if (done) finishReplace(true);
}

void CrunchWindow::setFixedSlices(qint64 cells)
{
    for (SliceTuner *t : {&m_searchTuner, &m_replaceTuner, &m_listTuner, &m_filterTuner, &m_sortTuner, &m_applyTuner}) t->setFixed(cells);
}

// A change of many cells (a sort, a replace) is put in a slice at a time, each of a few milliseconds, and is one step on the undo stack when it is all in. `step` does a slice (about the cells it
// is told) and says whether there is more; `done` is told how it ended.
void CrunchWindow::runApply(int what, const std::function<crunch::EditSession::Outcome(qint64)> &step, const std::function<void(const crunch::EditSession::Outcome &)> &done)
{
    m_applying = what;
    m_applyStep = step;
    m_applyDone = done;
    m_applyTuner.reset();
    m_applyTimer.start();
}

void CrunchWindow::stepApply()
{
    if (!m_applyStep) {
        m_applyTimer.stop();
        m_applying = 0;
        return;
    }
    m_applyTuner.begin();
    const qint64 budget = m_applyTuner.budget();
    const crunch::EditSession::Outcome o = m_applyStep(budget);
    m_applyTuner.end(budget);
    if (o.ok && o.pending) return;
    m_applyTimer.stop();
    const auto done = std::move(m_applyDone);
    m_applyStep = nullptr;
    m_applyDone = nullptr;
    m_applying = 0;
    if (done) done(o);
}

// The new text of every cell is worked out: the cells are changed in one step.
void CrunchWindow::finishReplace(bool done)
{
    m_replaceTimer.stop();
    const auto plan = std::move(m_replacePlan);
    m_replacePlan.reset();
    if (!plan || !m_findDialog) return;
    m_findDialog->setBusy(false);
    if (!m_calculating && !m_loading) setStatus(tr("Ready"));
    if (!done || !m_session) return;
    const qint64 before = m_loader.changesMade();
    const auto result = std::make_shared<crunch::EditSession::ReplaceResult>();
    m_findDialog->setBusy(true);
    setStatus(tr("Changing…"));
    runApply(
        2, [this, plan, result](qint64 budget) { return m_session->applyReplace(*plan, result.get(), budget); },
        [this, before, result](const crunch::EditSession::Outcome &o) {
            if (m_findDialog) m_findDialog->setBusy(false);
            if (!m_calculating && !m_loading) setStatus(tr("Ready"));
            if (!m_findDialog) return;
            if (!o.ok) {
                m_findDialog->setStatus(o.problem);
                if (!o.problem.isEmpty()) notice(tr("Change All"), o.problem);
                return;
            }
            if (m_loader.changesMade() != before) m_calculating = true;
            const int places = result->occurrences, cells = result->cells;
            m_findDialog->setStatus(cells == 0 ? tr("Nothing needed to change.") : tr("Changed %1 in %2.").arg(tr("%n place(s)", nullptr, places), tr("%n cell(s)", nullptr, cells)));
        });
}

// Replace: the cell with the cursor, if it has the text, is changed and the search goes on to the next; else the search goes on to the first.
void CrunchWindow::replaceCurrent()
{
    if (!m_findDialog || !m_session || !m_loaded) return;
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active) return;
    const crunch::FindOptions options = searchOptions(true);
    const quint32 row = quint32(std::max<qint64>(0, m_view->selection().activeRow)), col = quint32(std::max<qint64>(0, m_view->selection().activeCol));
    const jp::office::sheet::TextMatcher matcher(options.pattern);
    if (!matcher.valid()) {
        m_findDialog->setStatus(tr("Type the text to look for."));
        return;
    }
    if (matcher.matches(m_session->editText(m_sheet, row, col))) {
        const qint64 before = m_loader.changesMade();
        crunch::EditSession::ReplaceResult result;
        const crunch::EditSession::Outcome o = doStructural([&] { return m_session->replaceInCells({{m_sheet, row, col}}, options.pattern, m_findDialog->replacement(), &result); });
        if (!o.ok) {
            m_findDialog->setStatus(o.problem);
            if (!o.problem.isEmpty()) notice(tr("Change"), o.problem);
            return;
        }
        if (m_loader.changesMade() != before) m_calculating = true;
    }
    startSearch(SearchFor::Next);
}

// ---- Go To

bool CrunchWindow::goToReference(const QString &text)
{
    if (!m_session || !m_loaded) return false;
    if (!commitEdit()) return false;
    const crunch::GoToTarget t = crunch::resolveGoTo(*m_session, text, m_sheet);
    if (!t.ok) {
        notice(tr("Go To"), t.problem);
        return false;
    }
    if (t.sheet != m_sheet) {
        setActiveSheet(t.sheet);
        if (m_sheet != t.sheet) return false;
    }
    if (t.wholeRows) m_view->selectRows(t.range.row1, t.range.row2);
    else if (t.wholeColumns) m_view->selectColumns(t.range.col1, t.range.col2);
    else m_view->selectRange(t.range.row1, t.range.col1, t.range.row2, t.range.col2);
    m_view->ensureVisible(t.range.row1, t.range.col1);
    m_view->setFocus(Qt::OtherFocusReason);
    return true;
}

void CrunchWindow::goToDialog()
{
    if (!m_session || !m_loaded) return;
    QString reference;
    if (m_goToAsker) {
        if (!m_goToAsker(&reference)) return;
    } else {
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Go To"));
        auto *layout = new QVBoxLayout(&dialog);
        auto *names = new QListWidget;
        names->setAccessibleName(tr("Names"));
        for (const auto &n : crunch::goToNames(*m_session, m_sheet)) {
            auto *item = new QListWidgetItem(n.first);
            item->setToolTip(n.second);
            names->addItem(item);
        }
        auto *edit = new QLineEdit;
        edit->setAccessibleName(tr("Reference"));
        auto *form = new QFormLayout;
        form->addRow(tr("Go to:"), names);
        form->addRow(tr("Reference:"), edit);
        layout->addLayout(form);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        auto *special = buttons->addButton(tr("Special…"), QDialogButtonBox::ActionRole);
        layout->addWidget(buttons);
        connect(names, &QListWidget::currentTextChanged, edit, &QLineEdit::setText);
        connect(names, &QListWidget::itemDoubleClicked, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        bool wantSpecial = false;
        connect(special, &QPushButton::clicked, &dialog, [&] {
            wantSpecial = true;
            dialog.accept();
        });
        if (dialog.exec() != QDialog::Accepted) return;
        if (wantSpecial) {
            goToSpecialDialog();
            return;
        }
        reference = edit->text();
    }
    goToReference(reference);
}

void CrunchWindow::goToSpecialDialog()
{
    if (!m_session || !m_loaded) return;
    crunch::SpecialOptions options;
    if (m_specialAsker) {
        if (!m_specialAsker(&options)) return;
    } else {
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Select by Kind"));
        auto *layout = new QVBoxLayout(&dialog);
        struct Kind {
            crunch::SpecialKind kind;
            QString text;
        };
        const Kind kinds[] = {{crunch::SpecialKind::Blanks, tr("Blank cells")},
                              {crunch::SpecialKind::Constants, tr("Constants (typed in, not worked out)")},
                              {crunch::SpecialKind::Formulas, tr("Formulas")},
                              {crunch::SpecialKind::Errors, tr("Cells that show an error")},
                              {crunch::SpecialKind::CurrentRegion, tr("The block around the cell with the cursor")},
                              {crunch::SpecialKind::LastCell, tr("The last cell that is used")},
                              {crunch::SpecialKind::VisibleCells, tr("Cells that show (not in hidden rows or columns)")},
                              {crunch::SpecialKind::ConditionalFormats, tr("Cells with conditional formatting")},
                              {crunch::SpecialKind::DataValidation, tr("Cells with a rule for what may be entered")}};
        auto *group = new QButtonGroup(&dialog);
        int id = 0;
        for (const Kind &k : kinds) {
            auto *button = new QRadioButton(k.text);
            button->setChecked(id == 0);
            group->addButton(button, id++);
            layout->addWidget(button);
        }
        auto *which = new QGroupBox(tr("For constants and formulas, include"));
        auto *whichLayout = new QVBoxLayout(which);
        auto *numbers = new QCheckBox(tr("Numbers"));
        auto *text = new QCheckBox(tr("Text"));
        auto *logicals = new QCheckBox(tr("TRUE and FALSE"));
        auto *errors = new QCheckBox(tr("Errors"));
        for (QCheckBox *c : {numbers, text, logicals, errors}) {
            c->setChecked(true);
            whichLayout->addWidget(c);
        }
        layout->addWidget(which);
        auto *same = new QCheckBox(tr("For a rule for what may be entered: only the same rule as the cell with the cursor"));
        layout->addWidget(same);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() != QDialog::Accepted) return;
        options.kind = kinds[std::max(0, group->checkedId())].kind;
        options.numbers = numbers->isChecked();
        options.text = text->isChecked();
        options.logicals = logicals->isChecked();
        options.errors = errors->isChecked();
        options.sameRule = same->isChecked();
    }
    goToSpecial(options);
}

bool CrunchWindow::goToSpecial(const crunch::SpecialOptions &options)
{
    if (!m_session || !m_loaded) return false;
    if (!commitEdit()) return false;
    const grid::Selection &sel = m_view->selection();
    const grid::GridRange r = sel.range;
    CellRange selection = selectedRange();
    // (the whole sheet, or whole rows and columns, are looked in as the cells of the sheet that are used)
    if (sel.whole != grid::Selection::Whole::None || (r.rows() == 1 && r.cols() == 1)) selection = CellRange();
    const crunch::SpecialResult found = crunch::goToSpecial(*m_session, m_sheet, selection, quint32(std::max<qint64>(0, sel.activeRow)), quint32(std::max<qint64>(0, sel.activeCol)), options);
    if (found.ranges.isEmpty()) {
        m_view->setMarks({});
        notice(tr("Select by Kind"), found.problem.isEmpty() ? tr("No cell matches.") : found.problem);
        return false;
    }
    m_settingMarks = true;
    const CellRange first = found.ranges.first();
    m_view->selectRange(first.row1, first.col1, first.row2, first.col2);
    m_view->ensureVisible(first.row1, first.col1);
    std::vector<grid::GridRange> marks;
    for (qsizetype i = 1; i < found.ranges.size(); ++i) marks.push_back(grid::GridRange{qint64(found.ranges[i].row1), qint64(found.ranges[i].col1), qint64(found.ranges[i].row2), qint64(found.ranges[i].col2)});
    m_view->setMarks(marks);
    m_settingMarks = false;
    const int cells = int(std::min<qint64>(found.cells, 100000000)), ranges = int(found.ranges.size());
    QString text = tr("Picked %1 in %2.").arg(tr("%n cell(s)", nullptr, cells), tr("%n range(s)", nullptr, ranges));
    if (ranges > 1) text += QLatin1Char(' ') + tr("The first range is selected, and the others are tinted.");
    if (found.truncated) text += QLatin1Char(' ') + tr("The search stopped at a limit, so there may be more.");
    setStatus(text);
    return true;
}

} // namespace jp
