// Fields and references: Update Field (F9), Unlink (Ctrl+Shift+F9), Lock and Unlock (Ctrl+F11 and Ctrl+Shift+F11), Insert Field, Bookmark, Cross-reference, the table of contents (Insert,
// Update, Remove), and Next and Previous Footnote. The work on the document is core/fieldops.h, core/refops.h, and core/tocops.h.

#include "app/writewindow.h"

#include "app/pageview.h"
#include "app/writedialogs.h"
#include "core/editops.h"
#include "core/fieldops.h"
#include "core/mergeops.h"
#include "core/linkops.h"
#include "core/refops.h"
#include "core/tocops.h"

#include <QAction>
#include <QLocale>
#include <QMenu>
#include <QStatusBar>

namespace jp {

namespace {

// Whether fields can be worked on here: in the text (a header, a footer, or a note has its own fields, which the pages answer).
bool inText(const write::PageView *view) { return view && !view->inHeaderFooter() && !view->inNote(); }

} // namespace

void WriteWindow::updateFieldsNow()
{
    if (!m_doc || !inText(m_view)) return;
    QTextCursor c = m_view->cursor();
    // F9 in a table of contents (with nothing selected) asks what to update.
    if (!c.hasSelection()) {
        const write::StoryScan scan = write::scanStory(m_doc->story(), false);
        const int at = write::fieldContaining(scan, c.position());
        int outer = at;
        while (outer >= 0 && scan.fields[outer].parent >= 0) outer = scan.fields[outer].parent;
        if (outer >= 0 && scan.fields[outer].name == QLatin1String("TOC")) {
            const int choice = write::updateTableDialog(this);
            if (choice == 0) return;
            write::updateTableOfContents(*m_doc, c, choice == 1);
            statusBar()->showMessage(tr("The table of contents is updated."), 5000);
            return;
        }
    }
    const write::UpdateReport r = write::updateFieldsAt(*m_doc, c);
    if (r.tocTruncated) statusBar()->showMessage(tr("The table of contents has more than %1 lines, so it stops there.").arg(QLocale().toString(write::kMostTocEntries)), 10000);
    else if (r.found == 0) statusBar()->showMessage(tr("There is no field here to update. Select text, or press Ctrl+A first, to update every field in it."), 6000);
    else statusBar()->showMessage(tr("Fields updated: %1.").arg(r.changed), 5000);
}

void WriteWindow::createFieldActions()
{
    auto after = [this] {
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
        refreshUi();
    };
    mk("fields.update", tr("Update Field"), "refresh-cw", QKeySequence(Qt::Key_F9), [this, after] {
        updateFieldsNow();
        after();
    });
    mk("fields.unlink", tr("Unlink Field"), "", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F9), [this, after] {
        if (m_doc && inText(m_view)) write::unlinkFields(*m_doc, m_view->cursor());
        after();
    });
    mk("fields.lock", tr("Lock Field"), "lock", QKeySequence(Qt::CTRL | Qt::Key_F11), [this, after] {
        if (m_doc && inText(m_view)) write::lockFields(*m_doc, m_view->cursor(), true);
        after();
    });
    mk("fields.unlock", tr("Unlock Field"), "lock-open", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F11), [this, after] {
        if (m_doc && inText(m_view)) write::lockFields(*m_doc, m_view->cursor(), false);
        after();
    });
    mk("fields.toggleCode", tr("Toggle Field Code"), "braces", QKeySequence(Qt::SHIFT | Qt::Key_F9), [this, after] {
        if (m_doc && inText(m_view)) write::toggleFieldCodes(*m_doc, m_view->cursor(), false);
        after();
    });
    mk("fields.toggleAllCodes", tr("Toggle All Field Codes"), "braces", QKeySequence(Qt::ALT | Qt::Key_F9), [this, after] {
        if (m_doc && inText(m_view)) write::toggleFieldCodes(*m_doc, m_view->cursor(), true);
        after();
    });
    mk("insert.field", tr("Field…"), "braces", QKeySequence(), [this, after] {
        if (m_doc && inText(m_view)) write::fieldDialog(this, m_doc.get(), m_view);
        after();
    });
    mk("insert.bookmark", tr("Bookmark…"), "bookmark", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F5), [this, after] {
        if (m_doc && inText(m_view)) write::bookmarkDialog(this, m_doc.get(), m_view);
        after();
    });
    mk("refs.crossReference", tr("Cross-reference…"), "link", QKeySequence(), [this, after] {
        if (m_doc && inText(m_view)) write::crossReferenceDialog(this, m_doc.get(), m_view);
        after();
    });
    mk("refs.toc", tr("Table of Contents…"), "list-tree", QKeySequence(), [this, after] {
        if (m_doc && inText(m_view)) write::tableOfContentsDialog(this, m_doc.get(), m_view);
        after();
    });
    mk("refs.updateTable", tr("Update Table"), "refresh-cw", QKeySequence(), [this, after] {
        if (m_doc && inText(m_view)) {
            const int choice = write::updateTableDialog(this);
            if (choice != 0 && write::updateTableOfContents(*m_doc, m_view->cursor(), choice == 1) == 0)
                statusBar()->showMessage(tr("Put the caret in a table of contents first."), 6000);
        }
        after();
    });
    mk("refs.removeTable", tr("Remove Table of Contents"), "", QKeySequence(), [this, after] {
        if (m_doc && inText(m_view) && !write::removeTableOfContents(*m_doc, m_view->cursor())) statusBar()->showMessage(tr("Put the caret in a table of contents first."), 6000);
        after();
    });
    auto note = [this, after](bool forward, bool endnote) {
        if (m_doc && inText(m_view)) {
            const int at = write::nextNoteReference(*m_doc, m_view->cursor().position(), forward, endnote);
            if (at >= 0) {
                QTextCursor c = m_view->cursor();
                c.setPosition(at + (forward ? 1 : 0));
                m_view->showCursor(c);
            } else {
                statusBar()->showMessage(endnote ? tr("There is no other endnote in that direction.") : tr("There is no other footnote in that direction."), 5000);
            }
        }
        after();
    };
    mk("refs.nextFootnote", tr("Next Footnote"), "arrow-down", QKeySequence(), [note] { note(true, false); });
    mk("refs.previousFootnote", tr("Previous Footnote"), "arrow-up", QKeySequence(), [note] { note(false, false); });
    mk("refs.nextEndnote", tr("Next Endnote"), "arrow-down", QKeySequence(), [note] { note(true, true); });
    mk("refs.previousEndnote", tr("Previous Endnote"), "arrow-up", QKeySequence(), [note] { note(false, true); });
}

// The right-click menu on a link (open, edit, remove), a field (update, show its code, lock, unlink), or a footnote's mark (go to the note): the commands are the ribbon's own.
void WriteWindow::addLinkFieldNoteContextMenu(QMenu &menu)
{
    if (!m_doc || !inText(m_view)) return;
    const int at = m_view->contextPosition();
    auto local = [&](const char *id, const QString &text, const std::function<void()> &run) {
        QAction *a = menu.addAction(text, this, run);
        a->setObjectName(QString::fromLatin1(id));
        return a;
    };
    bool sep = false;
    auto separate = [&] {
        if (!sep) menu.addSeparator();
        sep = true;
    };
    if (write::hyperlinkAt(*m_doc, at).valid()) {
        separate();
        local("ctx.openLink", tr("Open Link"), [this, at] { m_view->followLinkAt(at); });
        local("ctx.editLink", tr("Edit Link…"), [this] { act("insert.hyperlink")->trigger(); });
        menu.addAction(act("insert.removeHyperlink"));
    } else {
        menu.addAction(act("insert.hyperlink"));
    }
    QTextCursor where(m_doc->story());
    where.setPosition(qBound(0, at, m_doc->story()->characterCount() - 1));
    if (write::fieldsNearSelection(m_doc->story(), where).any) {
        separate();
        local("ctx.editField", tr("Edit Field…"), [this, at] {
            const QString now = write::fieldCodeAt(*m_doc, at);
            const QString code = m_fieldCodeAsker ? m_fieldCodeAsker(now) : write::editFieldDialog(this, now);
            if (!code.trimmed().isEmpty() && code.trimmed() != now && !write::replaceFieldCode(*m_doc, at, code)) statusBar()->showMessage(tr("This field can't be edited here."), 6000);
            m_view->setFocus(Qt::OtherFocusReason);
            m_view->viewport()->update();
            refreshUi();
        });
        if (const auto bf = write::mail::barcodeFieldAt(*m_doc, at); bf && bf->editable) local("ctx.editBarcode", tr("Edit Barcode…"), [this, at] { editBarcodeField(at); });
        for (const char *id : {"fields.update", "fields.toggleCode", "fields.lock", "fields.unlock", "fields.unlink"}) menu.addAction(act(QString::fromLatin1(id)));
    }
    if (const int note = write::footnoteReferenceAt(*m_doc, at); note >= 0) {
        separate();
        local("ctx.goToNote", tr("Go to Footnote"), [this, at] { m_view->openNoteAt(at); });
    }
}

} // namespace jp
