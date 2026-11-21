// The Paste Options button of the grid: after a paste of copied cells, a small button by the last pasted cell offers the other forms (the shared button: libs/shell/app/pasteoffer.h).

#include "app/crunchwindow.h"

#include "app/pasteoffer.h"
#include "app/settings.h"

#include <QApplication>
#include <QClipboard>
#include <QCoreApplication>
#include <QUndoCommand>
#include <algorithm>
#include <QUndoStack>

namespace jp {

namespace {
using Options = crunch::EditSession::PasteOptions;
}

void CrunchWindow::createPasteOffer()
{
    m_pasteOffer = new PasteOffer(m_view->viewport(), m_view, [this] { return pasteOfferChoices(); });
    m_pasteOffer->installOn(m_view);
    m_pasteOffer->setBusyCheck([this] { return pasteOfferBusy(); });
    connect(m_pasteOffer, &PasteOffer::escaped, this, [this] { endPasteOffer(); });
    connect(m_pasteOffer, &PasteOffer::opened, this, [this] { setStatus(tr("Paste Options. Use the arrow keys and Enter to choose, or Esc to leave them.")); });
    connect(m_view, &grid::GridView::viewChanged, this, [this] { updatePasteOffer(); });
    // The offer is for the last thing done: a copy elsewhere, or any other edit (a step, Undo, or Redo), ends it.
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [this] {
        if (!m_repasting) endPasteOffer();
    });
}

// Whether a step is under way (a large change put in slice by slice, a find or sort at work, a cell being edited, a save): the button offers nothing then, as the paste cannot be taken back.
bool CrunchWindow::pasteOfferBusy() const
{
    return m_applying || m_saving || m_edit.active || m_argsDialog || (m_session && m_session->isSlicing());
}

void CrunchWindow::endPasteOffer()
{
    m_pasteNote.live = false;
    if (m_pasteOffer) m_pasteOffer->hide();
}

// Whether the paste the button is for is still the last step: the same stack, the same place in it, and the same step on top (a stack that is full drops its oldest step, and its place stays).
bool CrunchWindow::pasteIsLast() const
{
    QUndoStack *u = undoStack();
    return m_pasteNote.live && u && u == m_pasteNote.stack && u->index() == m_pasteNote.index && u->index() > 0 && u->command(u->index() - 1) == m_pasteNote.top && m_pasteNote.sheet == m_sheet;
}

void CrunchWindow::notePaste(const jp::office::sheet::CellRange &range, const crunch::EditSession::PasteOptions &options)
{
    QUndoStack *u = undoStack();
    if (!u || u->index() <= 0 || !range.valid) {
        endPasteOffer();
        return;
    }
    m_pasteNote = PasteNote{true, u, u->index(), u->command(u->index() - 1), m_sheet, range, options};
    updatePasteOffer();
}

// The button sits by the last cell pasted; it goes when that cell is scrolled out of sight, or when anything was done since.
void CrunchWindow::updatePasteOffer()
{
    if (!m_pasteOffer) return;
    if (!pasteIsLast()) {
        m_pasteNote.live = false;
        m_pasteOffer->hide();
        return;
    }
    const QRect cell = m_view->cellRect(m_pasteNote.range.row2, m_pasteNote.range.col2);
    if (cell.isNull() || !m_view->viewport()->rect().intersects(cell)) {
        m_pasteOffer->hide();   // (scrolled away: it comes back when the cell does)
        return;
    }
    m_pasteOffer->setDescription(tr("How the pasted cells are kept. Tap the %1 key after a paste to open it.").arg(PasteOffer::keyName()));
    m_pasteOffer->showAt(cell);
    m_pasteOffer->refreshBusy();
}

// The button's choice: the last paste is taken back and done again in the form chosen, one step of Undo in all. When the form does nothing here the first paste stands.
bool CrunchWindow::repasteWith(const crunch::EditSession::PasteOptions &options)
{
    if (!pasteIsLast() || m_repasting || pasteOfferBusy()) return false;
    const PasteNote first = m_pasteNote;
    QUndoStack *u = undoStack();
    m_repasting = true;
    undoRedoNow(false);
    if (u->index() != first.index - 1) {   // (the paste was not taken back: nothing is done on top of it)
        m_repasting = false;
        return false;
    }
    m_pasteNote.live = false;
    pasteClipboard(options);
    m_repasting = false;
    if (m_pasteNote.live) return true;
    undoRedoNow(true);   // (not done: the first paste is put back)
    notePaste(first.range, first.options);
    return false;
}

// Set Default Paste: what Paste puts in when cells were copied in this workbook and in another; the first is the setting the Set Default Paste command has always kept.
QVector<PasteDefaultRow> CrunchWindow::pasteDefaultRows()
{
    const QVector<PasteDefaultChoice> choices{{QCoreApplication::translate("jp::CrunchWindow", "Everything"), int(Options::What::All)},
                                              {QCoreApplication::translate("jp::CrunchWindow", "Formulas"), int(Options::What::Formulas)},
                                              {QCoreApplication::translate("jp::CrunchWindow", "Values"), int(Options::What::Values)},
                                              {QCoreApplication::translate("jp::CrunchWindow", "Formatting only"), int(Options::What::Formats)},
                                              {QCoreApplication::translate("jp::CrunchWindow", "No Borders"), int(Options::What::AllExceptBorders)},
                                              {QCoreApplication::translate("jp::CrunchWindow", "Formulas and Number Formatting"), int(Options::What::FormulasAndNumberFormats)},
                                              {QCoreApplication::translate("jp::CrunchWindow", "Values and Number Formatting"), int(Options::What::ValuesAndNumberFormats)},
                                              {QCoreApplication::translate("jp::CrunchWindow", "Keep Source Column Widths"), int(Options::What::ColumnWidths)},
                                              {QCoreApplication::translate("jp::CrunchWindow", "Comments and Notes"), int(Options::What::Comments)}};
    // (what was kept before there were two: a workbook that has never been given a choice for cells from another follows the first)
    const int first = std::clamp(Settings::get().value(QStringLiteral("crunch/defaultPaste"), 0).toInt(), 0, int(Options::What::Comments));
    return {PasteDefaultRow{PasteFrom::SameDocument, QCoreApplication::translate("jp::CrunchWindow", "Cells copied in this workbook"), QStringLiteral("crunch/defaultPaste"), choices, int(Options::What::All)},
            PasteDefaultRow{PasteFrom::OtherDocument, QCoreApplication::translate("jp::CrunchWindow", "Cells copied in another workbook"), QStringLiteral("crunch/defaultPasteBetween"), choices, first}};
}

QVector<PasteChoice> CrunchWindow::pasteOfferChoices()
{
    QVector<PasteChoice> out;
    auto add = [this, &out](const QString &text, Options::What what, bool transpose = false, bool separator = false) {
        Options o;
        o.what = what;
        o.transpose = transpose;
        out.push_back({text, transpose ? QStringLiteral("pasteTranspose") : QStringLiteral("pasteOption%1").arg(int(what)), [this, o] { repasteWith(o); }, separator});
    };
    add(tr("Keep Source Formatting"), Options::What::All);
    add(tr("Values"), Options::What::Values);
    add(tr("Formulas"), Options::What::Formulas);
    add(tr("Formatting"), Options::What::Formats);
    add(tr("No Borders"), Options::What::AllExceptBorders);
    add(tr("Formulas and Number Formatting"), Options::What::FormulasAndNumberFormats, false, true);
    add(tr("Values and Number Formatting"), Options::What::ValuesAndNumberFormats);
    add(tr("Keep Source Column Widths"), Options::What::ColumnWidths);
    add(tr("Transpose"), Options::What::All, true, true);
    return out;
}

} // namespace jp
