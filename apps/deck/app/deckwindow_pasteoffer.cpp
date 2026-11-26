// The Paste Options button of the slide: after Paste, a small button by what was pasted offers the other forms (the shared button: libs/shell/app/pasteoffer.h).

#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/editor.h"
#include "app/pastedefaults.h"
#include "app/pasteoffer.h"
#include "canvas/canvas.h"

#include <QApplication>
#include <QClipboard>
#include <QCoreApplication>
#include <QMimeData>
#include <QScrollBar>
#include <QStatusBar>

namespace jp {

namespace {
// The forms a paste is offered in, as the numbers of the choices' names (pasteOption0, ...).
enum PasteForm { KeepSource = 0, TextOnly = 1, Picture = 2, CellsAsTable = 3 };
}

void DeckWindow::createPasteOffer()
{
    m_pasteOffer = new PasteOffer(m_canvas->viewport(), m_canvas, [this] { return pasteOfferChoices(); });
    m_pasteOffer->installOn(m_canvas);
    connect(m_pasteOffer, &PasteOffer::escaped, this, [this] { endPasteOffer(); });
    connect(m_pasteOffer, &PasteOffer::opened, this, [this] { statusBar()->showMessage(tr("Paste Options. Use the arrow keys and Enter to choose, or Esc to leave them."), 6000); });
    // The offer is for the last thing done: the next edit, selection, caret move, page, or copy ends it (what the offer itself does while it is at work does not).
    connect(m_ed, &Editor::changed, this, [this] {
        ++m_changes;
        if (!m_pasting) endPasteOffer();
    });
    for (auto sig : {&Editor::selectionChanged, &Editor::pageChanged, &Editor::documentReplaced, &Editor::textCursorChanged})
        connect(m_ed, sig, this, [this] {
            if (!m_pasting) endPasteOffer();
        });
    connect(m_ed, &Editor::viewChanged, this, [this] { updatePasteOffer(); });
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [this] { endPasteOffer(); });
    connect(m_canvas->verticalScrollBar(), &QScrollBar::valueChanged, this, [this] { updatePasteOffer(); });
    connect(m_canvas->horizontalScrollBar(), &QScrollBar::valueChanged, this, [this] { updatePasteOffer(); });
}

void DeckWindow::endPasteOffer()
{
    m_pasteLive = false;
    if (m_pasteOffer) m_pasteOffer->hide();
}

// The button sits by the caret of the text just pasted, or by the object pasted; it goes when that is scrolled out of sight.
void DeckWindow::updatePasteOffer()
{
    if (!m_pasteOffer) return;
    if (!m_pasteLive || m_pasteAt != m_changes || !m_ed->doc()) {
        m_pasteOffer->hide();
        return;
    }
    QRectF where = m_ed->isEditingText() ? m_canvas->caretViewRect() : QRectF();
    if (where.isNull()) {
        const QRectF page = m_ed->selectionBounds();
        if (page.isNull()) {
            m_pasteOffer->hide();
            return;
        }
        where = QRectF(m_canvas->pageToView(page.topLeft()), m_canvas->pageToView(page.bottomRight())).normalized();
    }
    if (!where.toRect().intersects(m_canvas->viewport()->rect())) {
        m_pasteOffer->hide();   // (scrolled away)
        return;
    }
    m_pasteOffer->setDescription(tr("How the pasted content is kept. Tap the %1 key after a paste to open it.").arg(PasteOffer::keyName()));
    m_pasteOffer->showAt(where.toRect());
}

// Set Default Paste: the forms a paste may take by default, for what was copied in this presentation, in another, and in another program.
QVector<PasteDefaultRow> DeckWindow::pasteDefaultRows()
{
    const QVector<PasteDefaultChoice> choices{{QCoreApplication::translate("jp::DeckWindow", "Keep Source Formatting"), KeepSource}, {QCoreApplication::translate("jp::DeckWindow", "Keep Text Only"), TextOnly}};
    return {PasteDefaultRow{PasteFrom::SameDocument, QCoreApplication::translate("jp::DeckWindow", "Pasting within the same presentation"), QStringLiteral("edit/pasteDefaultSame"), choices, KeepSource},
            PasteDefaultRow{PasteFrom::OtherDocument, QCoreApplication::translate("jp::DeckWindow", "Pasting between presentations"), QStringLiteral("edit/pasteDefaultOther"), choices, KeepSource},
            PasteDefaultRow{PasteFrom::OtherProgram, QCoreApplication::translate("jp::DeckWindow", "Pasting from other programs"), QStringLiteral("edit/pasteDefaultProgram"), choices, KeepSource}};
}

// Paste, in the form Set Default Paste says for where what is on the clipboard came from: and the offer of the others.
void DeckWindow::pasteAndOffer()
{
    endPasteOffer();
    const quint64 before = m_changes;
    const QVector<PasteDefaultRow> rows = pasteDefaultRows();
    const PasteDefaultRow *row = PasteDefaultsSection::rowFor(rows, pasteOriginOf(QApplication::clipboard()->mimeData(), m_ed->originId()));
    const int form = row ? PasteDefaultsSection::value(*row) : int(KeepSource);
    m_pasting = true;
    pasteOnce(form);
    m_pasting = false;
    if (m_changes == before) return;
    m_pasteLive = true;
    m_pasteAt = m_changes;
    updatePasteOffer();
}

// One paste in one form; true when it changed the slide.
bool DeckWindow::pasteOnce(int form)
{
    const QMimeData *md = QApplication::clipboard()->mimeData();
    if (!md) return false;
    const quint64 before = m_changes;
    if (m_ed->editingStoryOnly() && (form == Picture || form == CellsAsTable)) return false;   // (the notes under the slide hold words only: no picture, no table)
    switch (form) {
    case TextOnly:
        m_ed->paste(true);
        break;
    case Picture:
        if (!md->hasImage()) return false;
        m_ed->endTextEdit();
        m_ed->paste(false);
        break;
    case CellsAsTable:
        m_ed->pasteCells(md);
        break;
    default:
        m_ed->paste(false);
        break;
    }
    return m_changes != before;
}

// The button's choice: the last paste is taken back and done again in the form chosen, one step of Undo in all (the taken-back paste is not left to redo). When the form does nothing here the
// first paste stands.
bool DeckWindow::repasteAs(int form)
{
    if (!m_pasteLive || m_pasteAt != m_changes) return false;
    QUndoStack *stack = m_ed->undoStack();
    const int index = stack->index();
    m_pasting = true;
    m_ed->undo();
    if (stack->index() != index - 1) {   // (the paste was not taken back, as with a crop open: nothing is done on top of it)
        m_pasting = false;
        return false;
    }
    const quint64 undone = m_changes;
    const bool did = pasteOnce(form);
    if (!did) {
        if (m_changes != undone) m_ed->undo();
        m_ed->redo();
    }
    m_pasting = false;
    m_pasteAt = m_changes;
    m_pasteLive = true;
    updatePasteOffer();
    return did;
}

QVector<PasteChoice> DeckWindow::pasteOfferChoices()
{
    const QMimeData *md = QApplication::clipboard()->mimeData();
    QVector<PasteChoice> out;
    auto add = [this, &out](const QString &text, int form, bool separator = false) {
        out.push_back({text, QStringLiteral("pasteOption%1").arg(form), [this, form] { repasteAs(form); }, separator});
    };
    add(tr("Keep Source Formatting"), KeepSource);
    add(tr("Keep Text Only"), TextOnly);
    if (md && md->hasImage()) add(tr("Picture"), Picture);
    if (md && Editor::clipboardMayHaveCells(md)) add(tr("Spreadsheet Cells as Table"), CellsAsTable, true);
    return out;
}

} // namespace jp
