// JeffDeck's Notes Master and Handout Master views: the layout of the printed notes page and handout edited on a page (app/notespageview.h), with a tab each.
// Each command is one step that undoes; what is changed reaches the Notes Page view, printing, and the PDF.

#include "app/deckwindow.h"

#include "app/editor.h"
#include "app/notespageview.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "core/notesmaster.h"
#include "core/printheaderfooter.h"
#include "io/deckoutput.h"

#include <QAction>
#include <QActionGroup>
#include <QColorDialog>
#include "app/plaintext.h"

namespace jp {

namespace {
const int kCounts[] = {1, 2, 3, 4, 6, 9};
}

void DeckWindow::createNotesMasterActions()
{
    // A command of the tabs, used from anywhere else, opens the view it works in first (the notes master unless it is for the handout).
    auto inView = [this](bool handout) {
        const View want = handout ? View::HandoutMaster : View::NotesMaster;
        if (m_view != want && !(!handout && m_view == View::HandoutMaster)) setView(want);
    };
    // The header, date, footer, and number of the page being edited: on or off, as the Header & Footer choice of the notes pages or the handouts.
    struct Field {
        const char *id;
        QString text;
        bool deck::PrintHeaderFooter::*flag;
    };
    const Field fields[] = {{"pagemaster.header", tr("Header"), &deck::PrintHeaderFooter::header},
                            {"pagemaster.date", tr("Date"), &deck::PrintHeaderFooter::date},
                            {"pagemaster.footer", tr("Footer"), &deck::PrintHeaderFooter::footer},
                            {"pagemaster.number", tr("Slide Number"), &deck::PrintHeaderFooter::pageNumber}};
    for (const Field &f : fields) {
        const QString id = QString::fromLatin1(f.id);
        auto member = f.flag;
        mk(id, f.text, QString(), QKeySequence(), [this, id, member, inView] {
            inView(false);
            Document *d = m_ed->doc();
            if (!d || (m_view != View::NotesMaster && m_view != View::HandoutMaster)) return;
            const deck::PrintedPages pages = m_view == View::NotesMaster ? deck::PrintedPages::Notes : deck::PrintedPages::Handouts;
            deck::PrintHeaderFooter hf = deck::printHeaderFooter(*d, pages);
            hf.*member = act(id)->isChecked();
            m_ed->beginChange(tr("Header and Footer"));
            deck::setPrintHeaderFooter(*d, pages, hf);
            m_ed->endChange();
        }, true);
    }
    mk(QStringLiteral("pagemaster.headerFooter"), tr("Header and Footer…"), QStringLiteral("panel-top"), QKeySequence(), [this, inView] { inView(false); showPrintHeaderFooter(); });
    mk(QStringLiteral("pagemaster.reset"), tr("Reset Layout"), QStringLiteral("rotate-ccw"), QKeySequence(), [this, inView] { inView(false); m_notesPage->resetLayout(); });
    mk(QStringLiteral("pagemaster.background"), tr("Background Color…"), QStringLiteral("palette"), QKeySequence(), [this, inView] {
        inView(false);
        const QColor now = QColorDialog::getColor(Qt::white, this, tr("Background Color"));
        if (now.isValid()) m_notesPage->setBackground(now);
    });
    mk(QStringLiteral("pagemaster.backgroundNone"), tr("No Background Color"), QStringLiteral("eraser"), QKeySequence(), [this, inView] { inView(false); m_notesPage->setBackground(QColor()); });
    mk(QStringLiteral("pagemaster.close"), tr("Close Master View"), QStringLiteral("x"), QKeySequence(), [this] { closeMasters(); });
    act("pagemaster.close")->setEnabled(false);   // (as Close Master View of the slide masters: it waits for a master view)

    // Handout Master: slides on a page, and how the page lies.
    auto *counts = new QActionGroup(this);
    for (int n : kCounts) {
        QAction *a = mk(QStringLiteral("handout.per.%1").arg(n), n == 1 ? tr("1 Slide") : tr("%1 Slides").arg(n), QString(), QKeySequence(), [this, n, inView] { inView(true); m_notesPage->setHandoutCount(n); }, true);
        counts->addAction(a);
    }
    auto *shapes = new QActionGroup(this);
    shapes->addAction(mk(QStringLiteral("handout.portrait"), tr("Portrait"), QStringLiteral("rectangle-vertical"), QKeySequence(), [this, inView] { inView(true); m_notesPage->setHandoutLandscape(false); }, true));
    shapes->addAction(mk(QStringLiteral("handout.landscape"), tr("Landscape"), QStringLiteral("rectangle-horizontal"), QKeySequence(), [this, inView] { inView(true); m_notesPage->setHandoutLandscape(true); }, true));
    connect(m_ed, &Editor::changed, this, &DeckWindow::refreshNotesMasterActions);
    connect(m_ed, &Editor::pageChanged, this, &DeckWindow::refreshNotesMasterActions);
    refreshNotesMasterActions();
}

// The check boxes and the choices of slides per page and orientation show what the presentation has.
void DeckWindow::refreshNotesMasterActions()
{
    const Document *d = m_ed ? m_ed->doc() : nullptr;
    const bool notes = m_view == View::NotesMaster, handout = m_view == View::HandoutMaster;
    if (!d || !m_notesPage) return;
    act("pagemaster.close")->setEnabled(notes || handout);
    if (notes || handout) {
        const deck::PrintHeaderFooter hf = deck::printHeaderFooter(*d, notes ? deck::PrintedPages::Notes : deck::PrintedPages::Handouts);
        act("pagemaster.header")->setChecked(hf.header);
        act("pagemaster.date")->setChecked(hf.date);
        act("pagemaster.footer")->setChecked(hf.footer);
        act("pagemaster.number")->setChecked(hf.pageNumber);
    }
    if (handout) {
        const deck::HandoutMaster m = deck::handoutMasterOf(*d, deck::localePaper());
        for (int n : kCounts) act(QStringLiteral("handout.per.%1").arg(n))->setChecked(m.perPage == n);
        act("handout.portrait")->setChecked(!m.landscape);
        act("handout.landscape")->setChecked(m.landscape);
    }
}

// The widgets of the two tabs: the check boxes, each one a command's.
void DeckWindow::addNotesMasterParts(RibbonParts &parts)
{
    parts.colors[QStringLiteral("notesMaster")] = QColor(0x7A, 0x5C, 0x2B);
    const std::pair<const char *, const char *> boxes[] = {{"pmHeaderCheck", "pagemaster.header"}, {"pmDateCheck", "pagemaster.date"}, {"pmFooterCheck", "pagemaster.footer"}, {"pmNumberCheck", "pagemaster.number"}};
    for (const auto &b : boxes) {
        const QString id = QString::fromLatin1(b.second);
        parts.widgets[QString::fromLatin1(b.first)] = [this, id]() -> QWidget * { return actionCheckBox(id); };
    }
}

} // namespace jp
