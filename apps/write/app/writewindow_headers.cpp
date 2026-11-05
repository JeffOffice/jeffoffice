// Insert > Header and Footer, and the Header & Footer tab that is there while one is edited: Close, Insert Page Number, and the options (a different first page, different odd and
// even pages, and linking to the section before's).

#include "app/writewindow.h"

#include "app/pageview.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/writedialogs.h"
#include "core/editops.h"
#include "core/headerops.h"
#include "core/pagesetup.h"
#include "app/widgets.h"

#include <QAction>
#include <QMenu>

namespace jp {

void WriteWindow::createHeaderFooterActions()
{
    write::PageView *v = m_view;
    mk("insert.header", tr("Header"), "panel-top", QKeySequence(), [this] {
        if (m_doc) m_view->enterHeaderFooterAtCaret(false);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("insert.footer", tr("Footer"), "panel-bottom", QKeySequence(), [this] {
        if (m_doc) m_view->enterHeaderFooterAtCaret(true);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("hf.close", tr("Close Header and Footer"), "circle-x", QKeySequence(), [v] {
        v->leaveHeaderFooter();
        v->setFocus(Qt::OtherFocusReason);
    });
    mk("hf.pageNumber", tr("Page Number"), "hash", QKeySequence(), [this] {
        if (m_doc && m_view->inHeaderFooter()) write::insertPageNumber(*m_doc, m_view->cursor());
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
    });
    mk("hf.pageNumberFormat", tr("Page Number Format…"), "list-ordered", QKeySequence(), [this] {
        if (!m_doc || !m_view->inHeaderFooter()) return;
        write::pageNumberFormatDialog(this, m_doc.get(), m_view->headerFooterEdit().section);
        m_view->refreshHeaderFooter(false);
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
    });
    mk("hf.alignmentTab", tr("Insert Alignment Tab…"), "arrow-right-to-line", QKeySequence(), [this] {
        if (!m_doc || !m_view->inHeaderFooter()) return;
        QString alignment = QStringLiteral("center"), relativeTo = QStringLiteral("margin"), leader = QStringLiteral("none");
        const bool ok = m_alignmentTabAsker ? m_alignmentTabAsker(alignment, relativeTo, leader) : write::alignmentTabDialog(this, &alignment, &relativeTo, &leader);
        if (ok) write::insertAlignmentTab(*m_doc, m_view->cursor(), alignment, relativeTo, leader);
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
    });
    mk("hf.dateTime", tr("Date && Time…"), "calendar-clock", QKeySequence(), [this] {
        if (!m_doc || !m_view->inHeaderFooter()) return;
        write::dateTimeDialog(this, m_doc.get(), m_view);
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
    });
    mk("hf.previousSection", tr("Previous Section"), "chevron-up", QKeySequence(), [this] {
        if (m_doc && m_view->enterNeighborSection(-1)) m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("hf.nextSection", tr("Next Section"), "chevron-down", QKeySequence(), [this] {
        if (m_doc && m_view->enterNeighborSection(1)) m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("hf.goToHeader", tr("Go to Header"), "panel-top", QKeySequence(), [this] {
        if (m_doc && m_view->enterOtherHeaderFooter(false)) m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("hf.goToFooter", tr("Go to Footer"), "panel-bottom", QKeySequence(), [this] {
        if (m_doc && m_view->enterOtherHeaderFooter(true)) m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("hf.removeHeader", tr("Remove Header"), "panel-top-close", QKeySequence(), [this] { removeEdited(false); });
    mk("hf.removeFooter", tr("Remove Footer"), "panel-bottom-close", QKeySequence(), [this] { removeEdited(true); });
    mk("hf.differentFirst", tr("Different First Page"), "file-digit", QKeySequence(), [this] {
        if (!m_doc || !m_view->inHeaderFooter()) return;
        m_doc->setDifferentFirstPage(m_view->headerFooterEdit().section, act("hf.differentFirst")->isChecked());
        m_view->refreshHeaderFooter(true);
        m_view->setFocus(Qt::OtherFocusReason);
    }, true);
    mk("hf.differentOddEven", tr("Different Odd && Even Pages"), "file-stack", QKeySequence(), [this] {
        if (!m_doc || !m_view->inHeaderFooter()) return;
        m_doc->setDifferentOddEven(act("hf.differentOddEven")->isChecked());
        m_view->refreshHeaderFooter(true);
        m_view->setFocus(Qt::OtherFocusReason);
    }, true);
    mk("hf.linkPrevious", tr("Link to Previous"), "link", QKeySequence(), [this] {
        if (!m_doc || !m_view->inHeaderFooter()) return;
        const write::PageView::HeaderFooterEdit e = m_view->headerFooterEdit();
        const bool linked = act("hf.linkPrevious")->isChecked();
        m_doc->setLinkedToPrevious(e.section, e.footer, e.slot, linked);
        // (Linked, the section shows the section before's, or nothing if that has none; unlinked, the copy it has now.)
        m_view->refreshHeaderFooter(!linked);
        m_view->setFocus(Qt::OtherFocusReason);
    }, true);
    // Show Document Text: the page's own text is shown dimmed while a header or footer is edited, or left out.
    mk("hf.showDocumentText", tr("Show Document Text"), "eye", QKeySequence(), [this] {
        m_view->setShowBodyWhileEditingHeader(act("hf.showDocumentText")->isChecked());
        m_view->setFocus(Qt::OtherFocusReason);
    }, true);
    act("hf.showDocumentText")->setChecked(true);
}

// Remove Header (Footer): the one being edited, of its section, goes; editing ends, since what was edited is gone.
void WriteWindow::removeEdited(bool footer)
{
    if (!m_doc || !m_view->inHeaderFooter()) return;
    const int section = m_view->headerFooterEdit().section;
    m_view->leaveHeaderFooter();
    m_doc->removeHeaderFooter(section, footer);
    m_view->viewport()->update();
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::headerRibbonParts(RibbonParts &parts)
{
    // How far the header is from the top edge of the paper, and the footer from the bottom edge: the section being edited.
    for (const bool footer : {false, true}) {
        parts.widgets[footer ? QStringLiteral("footerFromBottom") : QStringLiteral("headerFromTop")] = [this, footer]() -> QWidget * {
            auto *spin = new MeasureSpin();
            spin->setObjectName(footer ? QStringLiteral("footerFromBottomField") : QStringLiteral("headerFromTopField"));
            spin->setAccessibleName(footer ? tr("Footer from bottom") : tr("Header from top"));
            m_hfFields << spin;
            connect(spin, &QDoubleSpinBox::editingFinished, this, [this, spin, footer] {
                if (m_updatingFields || !m_doc || !m_view->inHeaderFooter()) return;
                const int section = m_view->headerFooterEdit().section;
                const write::SectionSetup before = write::pageSetupOf(m_doc->sectionProps(section), m_doc->differentOddEven());
                write::SectionSetup after = before;
                (footer ? after.footer : after.header) = spin->value();
                write::applyPageSetup(*m_doc, section, write::PageSetupScope::ThisSection, before, after);
                m_view->refreshHeaderFooter(false);
                refreshUi();
            });
            return spin;
        };
    }
    // The designs a header or footer can start from, put on the one being edited.
    parts.menus[QStringLiteral("headerDesigns")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        for (const write::HeaderDesign &d : write::headerFooterDesigns())
            menu->addAction(d.label, menu, [this, id = d.id] {
                if (!m_doc || !m_view->inHeaderFooter()) return;
                const write::PageView::HeaderFooterEdit e = m_view->headerFooterEdit();
                if (write::applyHeaderFooterDesign(*m_doc, e.section, e.footer, e.slot, id)) m_view->refreshHeaderFooter(true);
                m_view->setFocus(Qt::OtherFocusReason);
            });
        return menu;
    };
    // The document's information, put in as fields.
    parts.menus[QStringLiteral("headerInfo")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        const std::pair<const char *, QString> kinds[] = {{"author", tr("Author")}, {"title", tr("Title")}, {"fileName", tr("File Name")}, {"filePath", tr("File Path")}};
        for (const auto &k : kinds)
            menu->addAction(k.second, menu, [this, kind = QString::fromLatin1(k.first)] {
                if (!m_doc || !m_view->inHeaderFooter()) return;
                write::insertDocumentInfo(*m_doc, m_view->cursor(), kind);
                m_view->setFocus(Qt::OtherFocusReason);
                m_view->viewport()->update();
            });
        return menu;
    };
}

// A header or footer is edited now, or none is any more: the tab that holds its controls comes and goes.
void WriteWindow::headerFooterModeChanged()
{
    const bool editing = m_view->inHeaderFooter();
    m_ribbon->setContextVisible(QStringLiteral("Header & Footer"), editing, true);
    refreshUi();
}

} // namespace jp
