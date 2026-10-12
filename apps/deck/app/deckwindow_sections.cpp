// JeffDeck's sections: the Section menu of the Home tab, and the commands the
// slides pane and the slide sorter ask for. Each is one step that undoes.

#include "app/deckwindow.h"

#include "app/editor.h"
#include "app/notespane.h"
#include "app/outlineview.h"
#include "app/slidesorter.h"
#include "app/slidespane.h"
#include "core/sections.h"

#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

namespace jp {

namespace {

// Asks for a section's name, the old one selected. False on Cancel.
bool askSectionName(QWidget *parent, QString *name)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("renameSectionDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::DeckWindow", "Rename Section"));
    auto *column = new QVBoxLayout(&dlg);
    auto *label = new QLabel(QCoreApplication::translate("jp::DeckWindow", "Section &name:"), &dlg);
    auto *edit = new QLineEdit(*name, &dlg);
    edit->setObjectName(QStringLiteral("sectionName"));
    label->setBuddy(edit);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    column->addWidget(label);
    column->addWidget(edit);
    column->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    QObject::connect(edit, &QLineEdit::textChanged, &dlg, [buttons](const QString &text) { buttons->button(QDialogButtonBox::Ok)->setEnabled(!text.trimmed().isEmpty()); });
    buttons->button(QDialogButtonBox::Ok)->setEnabled(!name->trimmed().isEmpty());
    edit->setFocus();
    edit->selectAll();
    QTimer::singleShot(0, edit, &QLineEdit::selectAll);
    if (dlg.exec() != QDialog::Accepted) return false;
    *name = edit->text().trimmed();
    return !name->isEmpty();
}

} // namespace

void DeckWindow::createSectionActions()
{
    mk("section.add", tr("Add Section"), "separator-horizontal", QKeySequence(), [this] { addSection(); });
    mk("section.rename", tr("Rename Section"), "pencil-line", QKeySequence(), [this] { renameSection(); });
    mk("section.remove", tr("Remove Section"), "folder-minus", QKeySequence(), [this] { removeSection(); });
    mk("section.removeAll", tr("Remove All Sections"), "list-x", QKeySequence(), [this] { removeAllSections(); });
    mk("section.collapseAll", tr("Collapse All"), "chevrons-down-up", QKeySequence(), [this] { m_slides->setAllCollapsed(true); });
    mk("section.expandAll", tr("Expand All"), "chevrons-up-down", QKeySequence(), [this] { m_slides->setAllCollapsed(false); });

    // The panes' menus ask for the same commands, and fold the same sections.
    connect(m_slides, &SlidesPane::addSectionWanted, this, [this](int slide) { addSection(slide); });
    connect(m_slides, &SlidesPane::renameSectionWanted, this, [this](int section) { renameSection(section); });
    connect(m_slides, &SlidesPane::removeSectionWanted, this, [this](int section) { removeSection(section); });
    connect(m_slides, &SlidesPane::removeSectionAndSlidesWanted, this, [this](int section) { removeSectionAndSlides(section); });
    connect(m_slides, &SlidesPane::moveSectionWanted, this, [this](int section, int by) { moveSection(section, by); });
    connect(m_sorter, &SlideSorter::addSectionWanted, this, [this](int slide) { addSection(slide); });
    connect(m_sorter, &SlideSorter::renameSectionWanted, this, [this](int section) { renameSection(section); });
    connect(m_sorter, &SlideSorter::removeSectionWanted, this, [this](int section) { removeSection(section); });
    connect(m_sorter, &SlideSorter::removeSectionAndSlidesWanted, this, [this](int section) { removeSectionAndSlides(section); });
    connect(m_sorter, &SlideSorter::moveSectionWanted, this, [this](int section, int by) { moveSection(section, by); });
    connect(m_slides, &SlidesPane::collapsedChanged, this, [this] { m_sorter->setCollapsedSections(m_slides->collapsedSections()); });
    connect(m_sorter, &SlideSorter::collapsedChanged, this, [this] { m_slides->setCollapsedSections(m_sorter->collapsedSections()); });

    // Everything but Add Section waits for a section to act on.
    auto update = [this] {
        const bool any = m_ed->doc() && deck::hasSections(*m_ed->doc());
        for (const char *id : {"section.rename", "section.remove", "section.removeAll", "section.collapseAll", "section.expandAll"}) act(id)->setEnabled(any);
    };
    connect(m_ed, &Editor::changed, this, update);
    connect(m_ed, &Editor::documentReplaced, this, update);
    update();
}

void DeckWindow::addSection(int before)
{
    Document *d = m_ed->doc();
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Add Section"));
    deck::addSection(*d, std::clamp(before < 0 ? m_ed->currentPage() : before, 0, int(d->pages.size()) - 1));
    m_ed->endChange();
}

void DeckWindow::renameSection(int section)
{
    Document *d = m_ed->doc();
    if (section < 0) section = deck::sectionOfSlide(*d, m_ed->currentPage());
    const QVector<deck::Section> sections = deck::sectionsOf(*d);
    if (section < 0 || section >= sections.size()) return;
    QString name = sections[section].name;
    if (!askSectionName(this, &name) || name == sections[section].name) return;
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Rename Section"));
    deck::renameSection(*d, section, name);
    m_ed->endChange();
}

void DeckWindow::removeSection(int section)
{
    Document *d = m_ed->doc();
    if (section < 0) section = deck::sectionOfSlide(*d, m_ed->currentPage());
    if (section < 0 || section >= deck::sectionsOf(*d).size()) return;
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Remove Section"));
    deck::removeSection(*d, section);
    m_ed->endChange();
}

void DeckWindow::removeSectionAndSlides(int section)
{
    Document *d = m_ed->doc();
    if (section < 0) section = deck::sectionOfSlide(*d, m_ed->currentPage());
    if (section < 0 || section >= deck::sectionsOf(*d).size() || deck::slideCountOf(*d, section) >= d->pages.size()) return;
    const int first = deck::firstSlideOf(*d, section), count = deck::slideCountOf(*d, section), current = m_ed->currentPage();
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    m_ed->clearSelection();
    m_ed->beginChange(tr("Remove Section and Slides"));
    deck::removeSectionAndSlides(*d, section);
    // The slide being edited stays, or the one that takes its place.
    m_ed->setCurrentPage(current < first ? current : current >= first + count ? current - count : first);
    m_ed->endChange();
}

void DeckWindow::moveSection(int section, int by)
{
    Document *d = m_ed->doc();
    if (section < 0 || section >= deck::sectionsOf(*d).size()) return;
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    const std::shared_ptr<Page> current = d->pages[std::clamp(m_ed->currentPage(), 0, int(d->pages.size()) - 1)];
    m_ed->beginChange(by < 0 ? tr("Move Section Up") : tr("Move Section Down"));
    if (deck::moveSection(*d, section, by) >= 0) m_ed->setCurrentPage(int(d->pages.indexOf(current)));
    m_ed->endChange();
}

void DeckWindow::removeAllSections()
{
    Document *d = m_ed->doc();
    if (!deck::hasSections(*d)) return;
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Remove All Sections"));
    deck::removeAllSections(*d);
    m_ed->endChange();
}

} // namespace jp
