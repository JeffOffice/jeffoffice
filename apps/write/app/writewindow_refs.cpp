// References tab, beyond fields and notes: the gallery of tables of contents, Add Text and Mark Entry for them, captions and the table of figures, citations and the bibliography, the
// index, and the table of authorities. The dialogs are app/refsdialogs.cpp, the work on the document core/tocops.h, captionops.h, citeops.h, and indexops.h.

#include "app/writewindow.h"

#include "app/pageview.h"
#include "app/writedialogs.h"
#include "core/captionops.h"
#include "core/citeops.h"
#include "core/editops.h"
#include "core/indexops.h"
#include "core/tocops.h"

#include <QAction>
#include <QStatusBar>

namespace jp {

namespace {

bool inText(const write::PageView *view) { return view && !view->inHeaderFooter() && !view->inNote(); }

} // namespace

// Reads the document again from what it would save, as it is now: what the endnotes' settings changed is made again in the text. The history begins again, the caret stays where it was, and
// the document is still changed.
void WriteWindow::reloadDocument()
{
    if (!m_doc) return;
    QString err;
    QStringList notes;
    const QByteArray bytes = m_doc->toDocx(&err);
    std::unique_ptr<write::WriteDoc> fresh = bytes.isEmpty() ? nullptr : write::WriteDoc::fromDocx(bytes, &err, &notes);
    if (!fresh) {
        statusBar()->showMessage(tr("The change could not be made: %1").arg(err), 8000);
        return;
    }
    const int caret = m_view->cursor().position();
    setDocument(std::move(fresh), m_path);
    m_doc->setModified(true);
    QTextCursor c(m_doc->story());
    c.setPosition(std::min(caret, m_doc->story()->characterCount() - 1));
    m_view->showCursor(c);
}

void WriteWindow::createRefsActions()
{
    auto after = [this] {
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
        refreshUi();
    };
    // ---- footnote and endnote options
    mk("refs.notesOptions", tr("Footnote and Endnote…"), "settings-2", QKeySequence(), [this, after] {
        if (m_doc) {
            bool endnotes = false;
            if (write::notesOptionsDialog(this, m_doc.get(), &endnotes) && endnotes) reloadDocument();
        }
        after();
    });
    // ---- table of contents
    auto gallery = [this, after](write::TocGallery kind) {
        if (m_doc && inText(m_view)) {
            QTextCursor c = m_view->cursor();
            if (write::insertTocGallery(*m_doc, c, kind)) m_view->showCursor(c);
        }
        after();
    };
    mk("refs.tocAutomatic1", tr("Automatic Table 1"), "table-of-contents", QKeySequence(), [gallery] { gallery(write::TocGallery::Automatic1); });
    mk("refs.tocAutomatic2", tr("Automatic Table 2"), "table-of-contents", QKeySequence(), [gallery] { gallery(write::TocGallery::Automatic2); });
    mk("refs.tocManual", tr("Manual Table"), "table-of-contents", QKeySequence(), [gallery] { gallery(write::TocGallery::Manual); });
    auto addText = [this, after](int level) {
        if (m_doc && inText(m_view)) {
            QTextCursor c = m_view->cursor();
            write::addTextToToc(*m_doc, c, level);
        }
        after();
    };
    mk("refs.addText.none", tr("Do Not Show in Table of Contents"), "", QKeySequence(), [addText] { addText(0); });
    mk("refs.addText.1", tr("Level 1"), "heading-1", QKeySequence(), [addText] { addText(1); });
    mk("refs.addText.2", tr("Level 2"), "heading-2", QKeySequence(), [addText] { addText(2); });
    mk("refs.addText.3", tr("Level 3"), "heading-3", QKeySequence(), [addText] { addText(3); });
    mk("refs.markTocEntry", tr("Mark Entry…"), "tag", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_O), [this, after] {
        if (m_doc && inText(m_view)) write::tocEntryDialog(this, m_doc.get(), m_view);
        after();
    });
    // ---- captions
    mk("refs.insertCaption", tr("Insert Caption…"), "captions", QKeySequence(), [this, after] {
        if (m_doc && inText(m_view)) write::captionDialog(this, m_doc.get(), m_view);
        after();
    });
    mk("refs.tableOfFigures", tr("Insert Table of Figures…"), "list-ordered", QKeySequence(), [this, after] {
        if (m_doc && inText(m_view)) write::tableOfFiguresDialog(this, m_doc.get(), m_view);
        after();
    });
    // ---- citations and the bibliography
    mk("refs.insertCitation", tr("Insert Citation…"), "quote", QKeySequence(), [this, after] {
        if (m_doc && inText(m_view)) write::insertCitationDialog(this, m_doc.get(), m_view);
        after();
    });
    mk("refs.manageSources", tr("Manage Sources…"), "library", QKeySequence(), [this, after] {
        if (m_doc) write::manageSourcesDialog(this, m_doc.get());
        after();
    });
    mk("refs.citationStyle", tr("Citation Style…"), "text-quote", QKeySequence(), [this, after] {
        if (m_doc) write::citationStyleDialog(this, m_doc.get());
        after();
    });
    auto bibliography = [this, after](write::BibliographyKind kind) {
        if (m_doc && inText(m_view)) {
            QTextCursor c = m_view->cursor();
            if (write::insertBibliography(*m_doc, c, kind)) m_view->showCursor(c);
        }
        after();
    };
    mk("refs.bibliography.bibliography", tr("Bibliography"), "book-open", QKeySequence(), [bibliography] { bibliography(write::BibliographyKind::Bibliography); });
    mk("refs.bibliography.references", tr("References"), "book-open", QKeySequence(), [bibliography] { bibliography(write::BibliographyKind::References); });
    mk("refs.bibliography.worksCited", tr("Works Cited"), "book-open", QKeySequence(), [bibliography] { bibliography(write::BibliographyKind::WorksCited); });
    // ---- index
    mk("refs.markEntry", tr("Mark Entry…"), "tag", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_X), [this, after] {
        if (m_doc && inText(m_view)) write::markIndexEntryDialog(this, m_doc.get(), m_view);
        after();
    });
    mk("refs.insertIndex", tr("Insert Index…"), "book-a", QKeySequence(), [this, after] {
        if (m_doc && inText(m_view)) write::indexDialog(this, m_doc.get(), m_view);
        after();
    });
    mk("refs.updateIndex", tr("Update Index"), "refresh-cw", QKeySequence(), [this, after] {
        if (m_doc && inText(m_view) && write::updateIndexOrAuthorities(*m_doc, m_view->cursor(), false) == 0) statusBar()->showMessage(tr("Put the caret in an index first."), 6000);
        after();
    });
    // ---- table of authorities
    mk("refs.markCitation", tr("Mark Citation…"), "gavel", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_I), [this, after] {
        if (m_doc && inText(m_view)) write::markCitationDialog(this, m_doc.get(), m_view);
        after();
    });
    mk("refs.insertAuthorities", tr("Insert Table of Authorities…"), "scale", QKeySequence(), [this, after] {
        if (m_doc && inText(m_view)) write::tableOfAuthoritiesDialog(this, m_doc.get(), m_view);
        after();
    });
    mk("refs.updateAuthorities", tr("Update Table of Authorities"), "refresh-cw", QKeySequence(), [this, after] {
        if (m_doc && inText(m_view) && write::updateIndexOrAuthorities(*m_doc, m_view->cursor(), true) == 0) statusBar()->showMessage(tr("Put the caret in a table of authorities first."), 6000);
        after();
    });
}

} // namespace jp
