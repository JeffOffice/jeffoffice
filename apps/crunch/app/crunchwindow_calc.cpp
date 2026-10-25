// The Formulas tab, the parts about how the workbook calculates: by itself, when told, or by itself except for data tables; recalculating all of it or one sheet; going round circles of
// formulas that read themselves, and the notice when there are circles that are not gone round.
#include "app/crunchwindow.h"

#include <QAction>
#include <QActionGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QListWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QStatusBar>
#include <QVBoxLayout>
#include "app/plaintext.h"

namespace jp {

using jp::office::sheet::CalcMode;

void CrunchWindow::createCalcActions()
{
    // the note at the right of the status bar: formulas wait to be worked out, or some read themselves
    m_calcNote = plainLabel(new QLabel(this));
    m_calcNote->setObjectName(QStringLiteral("calcNote"));
    statusBar()->insertPermanentWidget(0, m_calcNote);
    auto *modes = new QActionGroup(this);
    modes->setExclusive(true);
    struct Mode {
        const char *id;
        CalcMode mode;
        QString text;
    };
    const Mode kinds[] = {{"formulas.calcAuto", CalcMode::Auto, tr("Automatic")},
                          {"formulas.calcNoTables", CalcMode::AutoNoTable, tr("Automatic, Tables by Hand")},
                          {"formulas.calcManual", CalcMode::Manual, tr("Manual")}};
    for (const Mode &k : kinds) {
        QAction *a = mk(QString::fromLatin1(k.id), k.text, "", QKeySequence(), [this, mode = k.mode] { setCalculationMode(mode); }, true, false);
        modes->addAction(a);
        m_opsActions << QString::fromLatin1(k.id);
    }
    act("formulas.calcAuto")->setChecked(true);
    mk("formulas.iteration", tr("Go Round Circular Formulas…"), "refresh-ccw", QKeySequence(), [this] {
        if (!m_session || !m_loaded) return;
        crunch::CalcSettings s = m_session->calcSettings();
        if (m_iterationAsker) {
            if (!m_iterationAsker(&s)) return;
        } else {
            QDialog dialog(this);
            dialog.setWindowTitle(tr("Circular Formulas"));
            auto *layout = new QVBoxLayout(&dialog);
            auto *on = new QCheckBox(tr("Work out a formula that reads itself by going round it"));
            on->setChecked(s.iterate);
            layout->addWidget(on);
            auto *form = new QFormLayout;
            auto *count = new QSpinBox;
            count->setAccessibleName(tr("Most rounds"));
            count->setRange(1, jp::office::sheet::Calculation::kMaxIterations);
            count->setValue(s.count);
            form->addRow(tr("Most rounds:"), count);
            auto *delta = new QDoubleSpinBox;
            delta->setAccessibleName(tr("Stop when no value changes by more than"));
            delta->setDecimals(6);
            delta->setRange(0, 1e9);
            delta->setValue(s.delta);
            form->addRow(tr("Stop when no value changes by more than:"), delta);
            layout->addLayout(form);
            auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
            layout->addWidget(buttons);
            connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
            connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
            auto sync = [=] {
                count->setEnabled(on->isChecked());
                delta->setEnabled(on->isChecked());
            };
            connect(on, &QCheckBox::toggled, &dialog, sync);
            sync();
            if (dialog.exec() != QDialog::Accepted) return;
            s.iterate = on->isChecked();
            s.count = count->value();
            s.delta = delta->value();
        }
        const crunch::EditSession::Outcome o = m_session->setCalcSettings(s);
        if (!o.ok) notice(tr("Circular Formulas"), o.problem);
    }, false, false);
    mk("formulas.options", tr("Calculation Mode"), "settings-2", QKeySequence(), [] {}, false, false);
    mk("formulas.calculateSheet", tr("Recalculate This Sheet"), "calculator", QKeySequence(Qt::SHIFT | Qt::Key_F9), [this] {
        if (m_loading || !m_haveCalc || !m_loader.recalculateSheet(m_sheet)) return;
        m_calculating = true;
        setStatus(tr("Calculating…"));
    }, false, false);
    mk("formulas.circular", tr("Circular References…"), "triangle-alert", QKeySequence(), [this] { showCircularReferences(); }, false, false);
    for (const char *id : {"formulas.iteration", "formulas.options", "formulas.calculateSheet", "formulas.circular"}) m_opsActions << QString::fromLatin1(id);
    updateEditActions();
}

void CrunchWindow::setCalculationMode(CalcMode mode)
{
    if (!m_session || !m_loaded) return;
    crunch::CalcSettings s = m_session->calcSettings();
    s.mode = mode;
    const crunch::EditSession::Outcome o = m_session->setCalcSettings(s);
    if (!o.ok) {
        notice(tr("Calculation"), o.problem);
        sendCalcSettings();   // (the buttons show what it is)
    }
}

// How the workbook calculates is as the session says: the worker is told whether to work by hand, and the buttons show it.
void CrunchWindow::sendCalcSettings()
{
    if (!m_session) return;
    const crunch::CalcSettings s = m_session->calcSettings();
    const bool manual = s.mode == CalcMode::Manual;
    m_loader.setManual(manual);
    // (back to automatic: what waited for the key, being worked out by the next change, is worked out now)
    if (m_manualNow && !manual && m_loader.recalculate()) m_calculating = true;
    m_manualNow = manual;
    if (s.mode != CalcMode::Manual) m_calcDue = false;
    const char *id = s.mode == CalcMode::Manual ? "formulas.calcManual" : s.mode == CalcMode::AutoNoTable ? "formulas.calcNoTables" : "formulas.calcAuto";
    if (QAction *a = act(QString::fromLatin1(id))) a->setChecked(true);
    if (QAction *a = act(QStringLiteral("formulas.iteration"))) a->setToolTip(s.iterate ? tr("Circular formulas are worked out by going round them") : tr("Circular formulas are not worked out"));
    updateCalcStatus();
}

void CrunchWindow::updateCalcStatus()
{
    if (!m_calcNote) return;
    QString text;
    if (m_session && m_session->calcSettings().mode == CalcMode::Manual && m_calcDue) text = tr("Out of date: press F9 to recalculate");
    else if (m_circular > 0) {
        const int shown = int(std::min<qint64>(m_circular, 1000000));
        text = tr("Circular references: %n cell(s)", nullptr, shown);
    }
    m_calcNote->setText(text);
    m_calcNote->setVisible(!text.isEmpty());
    if (QAction *a = act(QStringLiteral("formulas.circular"))) a->setEnabled(m_circular > 0 && m_loaded);
}

// What a last batch says: whether formulas wait (a workbook worked out by hand), and how many formulas are in circles.
void CrunchWindow::followCalcBatch(const crunch::CalcBatch &batch)
{
    if (batch.stopped) return;
    m_calcDue = batch.due && m_session && m_session->calcSettings().mode == CalcMode::Manual;
    const qint64 was = m_circular;
    m_circular = m_session && m_session->calcSettings().iterate ? 0 : batch.circular;
    m_circleCells = m_circular > 0 ? batch.circleCells : std::vector<jp::office::sheet::Calculation::CircularCell>();
    if (m_circular == 0) m_circularNoticed = false;
    updateCalcStatus();
    if (m_circular > 0 && (!m_circularNoticed || was == 0)) announceCircles();
    else if (m_circular != was) refreshCheckMarks();
}

// The circles are told once when they begin (not at every edit while they stay): the cells in them are named, and outlined on the sheet.
void CrunchWindow::announceCircles()
{
    if (m_circularNoticed || m_circular <= 0 || !m_session) return;
    m_circularNoticed = true;
    ++m_circularNotices;
    const std::vector<jp::office::sheet::Calculation::CircularCell> cells(m_circleCells.begin(), m_circleCells.begin() + std::min<size_t>(8, m_circleCells.size()));
    QStringList names;
    for (const auto &c : cells)
        names << (c.sheet == m_sheet ? QString() : m_loaded->book->info().sheets[c.sheet].name + QLatin1Char('!')) + jp::office::sheet::cellName(c.row, c.col);
    QString text = tr("A formula reads its own result, directly or through other cells, so it can't be worked out. The cells keep the values they had.");
    text += QStringLiteral("\n\n") + tr("Cells: %1").arg(names.join(tr(", ")));
    if (m_circular > qint64(cells.size())) {
        const int more = int(std::min<qint64>(m_circular - qint64(cells.size()), 1000000));
        text += tr(" and %n more", nullptr, more);
    }
    text += QStringLiteral("\n\n") + tr("To see them all, click Circular References on the Formulas tab. To go round them, turn on Go Round Circular Formulas.");
    refreshCheckMarks();
    notice(tr("Circular reference"), text);
}

bool CrunchWindow::showCircularReferences()
{
    if (!m_session || !m_loaded) return false;
    if (m_circular <= 0) {
        notice(tr("Circular References"), tr("No formula reads its own result."));
        return false;
    }
    const std::vector<jp::office::sheet::Calculation::CircularCell> cells(m_circleCells.begin(), m_circleCells.begin() + std::min<size_t>(200, m_circleCells.size()));
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Circular References"));
    auto *layout = new QVBoxLayout(&dialog);
    layout->addWidget(new QLabel(tr("These cells read their own result:")));
    auto *list = new QListWidget;
    list->setObjectName(QStringLiteral("circularList"));
    list->setAccessibleName(tr("Cells in circles"));
    for (const auto &c : cells) {
        auto *item = new QListWidgetItem(m_loaded->book->info().sheets[c.sheet].name + QLatin1Char('!') + jp::office::sheet::cellName(c.row, c.col));
        item->setData(Qt::UserRole, c.sheet);
        item->setData(Qt::UserRole + 1, c.row);
        item->setData(Qt::UserRole + 2, c.col);
        list->addItem(item);
    }
    layout->addWidget(list, 1);
    if (m_circular > qint64(cells.size())) layout->addWidget(new QLabel(tr("The first %n cell(s) are listed.", nullptr, int(cells.size()))));
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(list, &QListWidget::itemDoubleClicked, &dialog, [this, &dialog](QListWidgetItem *item) {
        const int sheet = item->data(Qt::UserRole).toInt();
        if (sheet != m_sheet) setActiveSheet(sheet);
        m_view->selectCell(item->data(Qt::UserRole + 1).toLongLong(), item->data(Qt::UserRole + 2).toLongLong());
        m_view->ensureVisible(item->data(Qt::UserRole + 1).toLongLong(), item->data(Qt::UserRole + 2).toLongLong());
        dialog.accept();
    });
    dialog.exec();
    return true;
}

} // namespace jp
