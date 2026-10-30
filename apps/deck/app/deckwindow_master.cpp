// JeffDeck's Slide Master view: the slide masters and layouts opened and edited like slides, in the place of the slide thumbnails, with the
// Slide Master tab. Each command is one step that undoes; an edit of a master or layout reaches the slides that stand on it as the same step
// (core/masteredit.h).

#include "app/deckwindow.h"

#include "app/editor.h"
#include "app/masterdialogs.h"
#include "app/masterpane.h"
#include "app/notespane.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/themegallery.h"
#include "core/masteredit.h"

#include <QAction>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QWidgetAction>
#include "app/plaintext.h"

namespace jp {

namespace {
// The tabs that work on a slide's transition, animation, show, and comments wait while a master or layout is open.
const char *const kSlideTabs[] = {"Transitions", "Animations", "Slide Show", "Review"};
} // namespace

void DeckWindow::createMasterActions()
{
    mk(QStringLiteral("master.insertMaster"), tr("Insert Slide Master"), QStringLiteral("layers"), QKeySequence(), [this] { insertSlideMaster(); });
    mk(QStringLiteral("master.insertLayout"), tr("Insert Layout"), QStringLiteral("panel-top"), QKeySequence(), [this] { insertLayout(); });
    mk(QStringLiteral("master.delete"), tr("Delete"), QStringLiteral("trash-2"), QKeySequence(), [this] {
        QString why;
        if (deleteMasterPage(m_ed->masterView(), &why)) return;
        if (m_masterNotice) m_masterNotice(why);
        else jp::plainInformation(this, tr("Delete"), why);
    });
    mk(QStringLiteral("master.rename"), tr("Rename"), QStringLiteral("pencil-line"), QKeySequence(), [this] {
        const MasterPage *m = m_ed->doc()->master(m_ed->masterView());
        if (!m) return;
        bool ok = false;
        const QString name = QInputDialog::getText(this, tr("Rename"), deck::isSlideMaster(*m) ? tr("Slide master name:") : tr("Layout name:"), QLineEdit::Normal, m->name, &ok);
        if (ok) renameMasterPage(m->id, name);
    });
    mk(QStringLiteral("master.preserve"), tr("Preserve"), QStringLiteral("pin"), QKeySequence(), [this] { setPreserve(m_ed->masterView(), act("master.preserve")->isChecked()); }, true);
    mk(QStringLiteral("master.layout"), tr("Master Layout"), QStringLiteral("layout-template"), QKeySequence(), [this] { askMasterLayout(); });

    struct Kind {
        const char *id, *icon;
        deck::PlaceholderKind kind;
        const char *text;
    };
    // Insert Placeholder's menu.
    const Kind kinds[] = {{"content", "square-plus", deck::PlaceholderKind::Content, QT_TR_NOOP("Content")},
                          {"text", "type", deck::PlaceholderKind::Text, QT_TR_NOOP("Text")},
                          {"picture", "image", deck::PlaceholderKind::Picture, QT_TR_NOOP("Picture")},
                          {"chart", "chart-column", deck::PlaceholderKind::Chart, QT_TR_NOOP("Chart")},
                          {"table", "table", deck::PlaceholderKind::Table, QT_TR_NOOP("Table")},
                          {"media", "film", deck::PlaceholderKind::Media, QT_TR_NOOP("Media")},
                          {"online", "globe", deck::PlaceholderKind::OnlineImage, QT_TR_NOOP("Online Image")}};
    for (const Kind &k : kinds) {
        const deck::PlaceholderKind kind = k.kind;
        mk(QStringLiteral("master.placeholder.") + QString::fromLatin1(k.id), tr(k.text), QString::fromLatin1(k.icon), QKeySequence(), [this, kind] { insertPlaceholder(kind); });
    }
    mk(QStringLiteral("master.title"), tr("Title"), QString(), QKeySequence(), [this] { setLayoutTitle(act("master.title")->isChecked()); }, true);
    mk(QStringLiteral("master.footers"), tr("Footers"), QString(), QKeySequence(), [this] { setLayoutFooters(act("master.footers")->isChecked()); }, true);
    mk(QStringLiteral("master.hideBackground"), tr("Hide Background Graphics"), QStringLiteral("eye-off"), QKeySequence(),
       [this] { hideBackgroundGraphics(act("master.hideBackground")->isChecked()); }, true);
    mk(QStringLiteral("master.close"), tr("Close Master View"), QStringLiteral("x"), QKeySequence(), [this] { closeMasters(); });

    m_masterPane->setMenuActions({act("master.insertMaster"), act("master.insertLayout"), nullptr, act("master.rename"), act("master.delete"), act("master.preserve")});
    // What the undo of an edit that was made in the view shows, the view shows again; and the thumbnails follow the presentation while it
    // shows (they are drawn when it opens, not for each presentation that is opened).
    connect(m_ed, &Editor::pageChanged, this, &DeckWindow::syncMasterView);
    connect(m_ed, &Editor::pageChanged, m_masterPane, &MasterPane::followCurrent);
    connect(m_ed, &Editor::pageChanged, this, &DeckWindow::refreshMasterActions);
    connect(m_ed, &Editor::changed, m_masterPane, [this] {
        if (m_view == View::Master) m_masterPane->refresh(true);
    });
    // An edit of a master or layout reaches what stands on it, in the same step.
    m_ed->masterEdited = [this](Document &now, const Document &before) { return m_carryEdits && deck::carryMasterEdits(now, before); };
}

// The Slide Master tab's check boxes, and the menus of the theme's gallery, colors, fonts, and background styles (the Design tab's own).
void DeckWindow::addMasterParts(RibbonParts &parts)
{
    parts.colors[QStringLiteral("slideMaster")] = QColor(0x2B, 0x7A, 0x78);
    parts.widgets[QStringLiteral("masterTitleCheck")] = [this]() -> QWidget * { return actionCheckBox(QStringLiteral("master.title")); };
    parts.widgets[QStringLiteral("masterFootersCheck")] = [this]() -> QWidget * { return actionCheckBox(QStringLiteral("master.footers")); };
    parts.widgets[QStringLiteral("masterHideCheck")] = [this]() -> QWidget * { return actionCheckBox(QStringLiteral("master.hideBackground")); };
    parts.menus[QStringLiteral("masterThemes")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        auto *gallery = new ThemeGallery(m_ed, menu);
        auto *wa = new QWidgetAction(menu);
        wa->setDefaultWidget(gallery);
        menu->addAction(wa);
        menu->addSeparator();
        menu->addAction(tr("Browse for Themes…"), this, [this] { browseForTheme(true); });
        connect(menu, &QMenu::aboutToShow, gallery, &ThemeGallery::reload);
        connect(gallery, &ThemeGallery::themeChosen, this, [this, menu](int theme, bool all) {
            menu->close();
            applyTheme(theme, all);
        });
        connect(gallery, &ThemeGallery::browseRequested, this, [this, menu] {
            menu->close();
            browseForTheme(true);
        });
        return menu;
    };
    parts.menus[QStringLiteral("masterColors")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        connect(menu, &QMenu::aboutToShow, menu, [this, menu] { fillColorsMenu(menu); });
        return menu;
    };
    parts.menus[QStringLiteral("masterFonts")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        connect(menu, &QMenu::aboutToShow, menu, [this, menu] { fillFontsMenu(menu); });
        return menu;
    };
    parts.menus[QStringLiteral("masterBackgrounds")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        connect(menu, &QMenu::aboutToShow, menu, [this, menu] { fillBackgroundsMenu(menu, menu); });
        return menu;
    };
}

// ---- the view

void DeckWindow::editMasters(const QString &page)
{
    const Document *d = m_ed->doc();
    if (!d) return;
    if (m_view == View::Master) {
        if (!page.isEmpty() && d->master(page)) m_ed->setMasterView(page);
        return;
    }
    m_masterFirst = page;
    setView(View::Master);
}

void DeckWindow::closeMasters()
{
    if (m_view == View::Master) setView(View::Normal);
}

void DeckWindow::ensureMasterView()
{
    if (m_view != View::Master) editMasters();
}

// The editor shows a master or layout, the thumbnails and the ribbon follow. It opens on the page asked for, else the one the editor has
// open (an undo shows one), else the layout of the slide being edited.
bool DeckWindow::enterMasterView()
{
    Document *d = m_ed->doc();
    if (!d) return false;
    m_ed->endTextEdit();
    QString target = m_masterFirst;
    m_masterFirst.clear();
    if (target.isEmpty() || !d->master(target)) target = d->master(m_ed->masterView()) ? m_ed->masterView() : QString();
    if (target.isEmpty()) {
        const int i = m_ed->currentPage();
        const MasterPage *layout = i >= 0 && i < d->pages.size() ? d->masterFor(*d->pages[i]) : nullptr;
        if (!layout)
            for (const auto &m : d->masters)
                if (deck::isSlideMaster(*m)) {
                    layout = m.get();
                    break;
                }
        if (layout) target = layout->id;
    }
    if (target.isEmpty()) return false;
    m_notesWereShown = !m_notes->isHidden();
    m_notes->hide();
    m_masterPane->refresh();
    m_ed->setMasterView(target);
    m_ribbon->setContextVisible(QStringLiteral("Slide Master"), true, true);
    for (const char *name : kSlideTabs) m_ribbon->setTabVisible(QString::fromLatin1(name), false);
    return true;
}

void DeckWindow::leaveMasterView()
{
    if (!m_ed->masterView().isEmpty()) m_ed->setCurrentPage(m_ed->currentPage());
    m_notes->setVisible(m_notesWereShown);
    m_ribbon->setContextVisible(QStringLiteral("Slide Master"), false);
    for (const char *name : kSlideTabs) m_ribbon->setTabVisible(QString::fromLatin1(name), true);
}

// The editor went into or out of showing a master page on its own (an undo of an edit made there, a new presentation): the window follows.
void DeckWindow::syncMasterView()
{
    if (m_switchingView) return;
    const bool editing = !m_ed->masterView().isEmpty();
    if (editing && m_view != View::Master) setView(View::Master);
    else if (!editing && m_view == View::Master) setView(View::Normal);
}

// What the Slide Master tab's commands and the slide commands let be done, for the page being edited.
void DeckWindow::refreshMasterActions()
{
    const Document *d = m_ed->doc();
    const bool inMaster = m_view == View::Master;
    const MasterPage *page = inMaster && d ? d->master(m_ed->masterView()) : nullptr;
    const bool layout = page && deck::isLayout(*page), master = page && deck::isSlideMaster(*page);
    for (const char *id : {"master.insertMaster", "master.insertLayout", "master.delete", "master.rename", "master.preserve", "master.close"}) act(id)->setEnabled(page != nullptr);
    if (page && m_slideLabel) {
        // Where the editor is, in the place the slide number has in Normal view.
        m_slideLabel->setText((master ? tr("Slide master: %1") : tr("Layout: %1")).arg(page->name));
        m_slideLabel->setMinimumWidth(std::max(110, m_slideLabel->fontMetrics().horizontalAdvance(m_slideLabel->text()) + 12));
    }
    act("master.preserve")->setChecked(page && deck::isKept(*d, *page));
    act("master.layout")->setEnabled(master);
    for (QAction *a : m_actions)
        if (a->objectName().startsWith(QLatin1String("master.placeholder."))) a->setEnabled(layout);
    act("master.title")->setEnabled(layout);
    act("master.title")->setChecked(layout && deck::layoutHasTitle(*page));
    act("master.footers")->setEnabled(layout);
    act("master.footers")->setChecked(layout && deck::layoutHasFooters(*page));
    act("master.hideBackground")->setEnabled(layout);
    act("master.hideBackground")->setChecked(layout && !page->showMasterItems);
    // What works on a slide waits while a master or layout is open (the ones that were available come back with the view).
    // (A command of the ribbon alone, such as Layout and Section, has no entry of the window's: it is found by its name.)
    static const char *const kSlideOnly[] = {"slide.new", "slide.duplicate", "slide.hide", "slide.reset", "slide.reuse", "view.notes", "insert.headerFooter", "insert.video",
                                             "insert.audio", "ribbon.layout", "ribbon.section", "ribbon.video", "ribbon.audio", "comment.new", "section.add", "section.rename",
                                             "section.remove", "section.removeAll"};
    auto command = [this](const QString &id) { return act(id) ? act(id) : findChild<QAction *>(id); };
    if (inMaster) {
        for (const char *id : kSlideOnly)
            if (QAction *a = command(QString::fromLatin1(id)); a && a->isEnabled()) {
                a->setEnabled(false);
                m_waiting << QString::fromLatin1(id);
            }
    } else if (!m_waiting.isEmpty()) {
        for (const QString &id : std::as_const(m_waiting))
            if (QAction *a = command(id)) a->setEnabled(true);
        m_waiting.clear();
    }
}

// ---- the commands

void DeckWindow::changeMasters(const QString &label, const std::function<void(Document &)> &fn, const QString &touched)
{
    Document *d = m_ed->doc();
    if (!d) return;
    m_notes->commit();
    m_ed->endTextEdit();
    m_ed->beginChange(label);
    // A slide master changed here is the user's own from then on (a change of theme won't take it away), however little the change is.
    deck::keepMasterOf(*d, m_ed->masterView());
    deck::keepMasterOf(*d, touched);
    fn(*d);
    deck::keepMasterOf(*d, m_ed->masterView());
    deck::keepMasterOf(*d, touched);
    m_ed->endChange();
    if (m_view == View::Master) m_masterPane->refresh();
    refreshMasterActions();
}

QString DeckWindow::insertSlideMaster()
{
    ensureMasterView();
    QString made;
    changeMasters(tr("Insert Slide Master"), [&](Document &d) {
        if (MasterPage *m = deck::addSlideMaster(d, m_ed->masterView())) {
            made = m->id;
            m_ed->setMasterView(made);
        }
    });
    return made;
}

QString DeckWindow::insertLayout()
{
    ensureMasterView();
    QString made;
    changeMasters(tr("Insert Layout"), [&](Document &d) {
        const MasterPage *open = d.master(m_ed->masterView());
        if (!open) return;
        const bool onLayout = deck::isLayout(*open);
        if (MasterPage *l = deck::addLayout(d, onLayout ? open->baseId : open->id, onLayout ? open->id : QString())) {
            made = l->id;
            m_ed->setMasterView(made);
        }
    });
    return made;
}

bool DeckWindow::deleteMasterPage(const QString &id, QString *why)
{
    Document *d = m_ed->doc();
    const MasterPage *page = d ? d->master(id) : nullptr;
    const QString reason = page ? deck::whyCantDelete(*d, *page) : tr("There is nothing to delete.");
    if (!reason.isEmpty()) {
        if (why) *why = reason;
        return false;
    }
    // The page being edited goes with it: the one before it is shown (for a slide master, the slide master before it, else the one after).
    const QString shown = m_ed->masterView();
    const MasterPage *open = d->master(shown);
    const bool goes = open && (open == page || (deck::isSlideMaster(*page) && open->baseId == page->id));
    QString next;
    if (goes) {
        QVector<const MasterPage *> masters;
        int at = -1;
        for (int i = 0; i < d->masters.size(); ++i) {
            if (d->masters[i].get() == page) at = i;
            if (deck::isSlideMaster(*d->masters[i])) masters << d->masters[i].get();
        }
        if (deck::isSlideMaster(*page)) {
            const int k = int(masters.indexOf(page));
            next = k > 0 ? masters[k - 1]->id : masters.value(k + 1) ? masters[k + 1]->id : QString();
        } else {
            next = at > 0 ? d->masters[at - 1]->id : QString();
        }
    }
    if (why) why->clear();
    changeMasters(tr("Delete"), [&](Document &doc) {
        if (goes && !next.isEmpty()) m_ed->setMasterView(next);
        deck::deleteMasterPage(doc, id);
    });
    return true;
}

void DeckWindow::renameMasterPage(const QString &id, const QString &name)
{
    const MasterPage *page = m_ed->doc() ? m_ed->doc()->master(id) : nullptr;
    if (!page || name.trimmed().isEmpty() || name.trimmed() == page->name) return;
    changeMasters(tr("Rename"), [&](Document &d) { deck::renameMasterPage(d, id, name); }, id);
}

void DeckWindow::setPreserve(const QString &id, bool on)
{
    const MasterPage *page = m_ed->doc() ? m_ed->doc()->master(id) : nullptr;
    if (!page || deck::isKept(*m_ed->doc(), *page) == on) return;
    changeMasters(tr("Preserve"), [&](Document &d) {
        if (MasterPage *m = d.master(id)) deck::setPreserved(d, *m, on);
    }, id);
}

QString DeckWindow::insertPlaceholder(deck::PlaceholderKind kind)
{
    const MasterPage *open = m_ed->doc() ? m_ed->doc()->master(m_ed->masterView()) : nullptr;
    if (m_view != View::Master || !open || !deck::isLayout(*open)) return QString();
    QString made;
    changeMasters(tr("Insert Placeholder"), [&](Document &d) {
        if (MasterPage *layout = d.master(m_ed->masterView()))
            if (Item *it = deck::insertPlaceholder(d, *layout, kind)) made = it->id;
    });
    if (!made.isEmpty()) m_ed->select(made);
    return made;
}

void DeckWindow::setLayoutTitle(bool on)
{
    const MasterPage *open = m_ed->doc() ? m_ed->doc()->master(m_ed->masterView()) : nullptr;
    if (m_view != View::Master || !open || !deck::isLayout(*open) || deck::layoutHasTitle(*open) == on) return;
    changeMasters(tr("Title"), [&](Document &d) { deck::setLayoutTitle(d, *d.master(m_ed->masterView()), on); });
}

void DeckWindow::setLayoutFooters(bool on)
{
    const MasterPage *open = m_ed->doc() ? m_ed->doc()->master(m_ed->masterView()) : nullptr;
    if (m_view != View::Master || !open || !deck::isLayout(*open) || deck::layoutHasFooters(*open) == on) return;
    changeMasters(tr("Footers"), [&](Document &d) { deck::setLayoutFooters(d, *d.master(m_ed->masterView()), on); });
}

void DeckWindow::setMasterParts(const deck::MasterParts &parts)
{
    const MasterPage *open = m_ed->doc() ? m_ed->doc()->master(m_ed->masterView()) : nullptr;
    if (m_view != View::Master || !open || !deck::isSlideMaster(*open) || deck::masterParts(*open) == parts) return;
    changeMasters(tr("Master Layout"), [&](Document &d) { deck::setMasterParts(d, *d.master(m_ed->masterView()), parts); });
}

void DeckWindow::hideBackgroundGraphics(bool hide)
{
    Document *d = m_ed->doc();
    if (!d) return;
    if (m_view == View::Master) {
        const MasterPage *open = d->master(m_ed->masterView());
        if (!open || !deck::isLayout(*open) || open->showMasterItems == !hide) return;
        changeMasters(tr("Hide Background Graphics"), [&](Document &doc) { doc.master(m_ed->masterView())->showMasterItems = !hide; });
        return;
    }
    const int i = m_ed->currentPage();
    if (i < 0 || i >= d->pages.size() || d->pages[i]->showMasterItems == !hide) return;
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Hide Background Graphics"));
    d->pages[i]->showMasterItems = !hide;
    m_ed->endChange();
    refreshMasterActions();
}

void DeckWindow::askMasterLayout()
{
    const MasterPage *open = m_ed->doc() ? m_ed->doc()->master(m_ed->masterView()) : nullptr;
    if (!open || !deck::isSlideMaster(*open)) return;
    MasterLayoutDialog dlg(deck::masterParts(*open), this);
    if (dlg.exec() == QDialog::Accepted) setMasterParts(dlg.parts());
}

} // namespace jp
