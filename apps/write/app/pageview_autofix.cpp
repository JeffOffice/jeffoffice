// The AutoCorrect options button: after a word is changed as it is typed, the choices to take the change back, stop it, or open the settings.

#include "app/pageview.h"

#include "app/autocorrectconfig.h"
#include "app/autocorrectdialog.h"
#include "app/settings.h"
#include "text/autocorrect.h"

#include <QAccessible>
#include <QAction>
#include <QCoreApplication>
#include <QLocale>
#include <QMenu>
#include <QScrollBar>
#include <QTextDocument>
#include <QToolButton>
#include <algorithm>

namespace jp::write {

bool PageView::autoFixAvailable() const
{
    return m_doc && m_autoFix.valid && m_autoFixStory && m_autoFixStory == m_doc->story() && m_cursor.document() == m_doc->story() && !m_cursor.hasSelection() &&
           m_cursor.position() == m_autoFixCaret && m_doc->story()->revision() == m_autoFix.revision;
}

// Called as a typed character has been put in (and AutoCorrect has had its say): where the caret is, and the story's revision, are what the offer holds to.
void PageView::noteAutoFix()
{
    if (!m_doc) return;
    if (m_autoFix.valid) {
        // (The change is offered only when nothing was done after it: no link, no dash, no list.)
        if (m_doc->story()->revision() != m_autoFix.revision) {
            m_autoFix = write::AutoFix();
        } else {
            m_autoFixCaret = m_cursor.position();
            m_autoFixStory = m_doc->story();
            m_autoFixTold = false;
        }
    }
    if (!m_autoFix.valid && !m_autoFixButton) return;
    if (!m_autoFixButton) {
        m_autoFixButton = new QToolButton(viewport());
        m_autoFixButton->setObjectName(QStringLiteral("autoCorrectOptionsButton"));
        m_autoFixButton->setText(QCoreApplication::translate("jp::write::PageView", "AutoCorrect Options"));
        m_autoFixButton->setAccessibleName(QCoreApplication::translate("jp::write::PageView", "AutoCorrect Options"));
        m_autoFixButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
        m_autoFixButton->setPopupMode(QToolButton::InstantPopup);
        m_autoFixButton->setFocusPolicy(Qt::NoFocus);
        auto *menu = new QMenu(m_autoFixButton);
        m_autoFixButton->setMenu(menu);
        connect(menu, &QMenu::aboutToShow, this, [this] { fillAutoFixMenu(); });
        m_autoFixButton->hide();
        connect(this, &PageView::caretChanged, this, [this] { updateAutoFixButton(); });
        connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this] { updateAutoFixButton(); });
    }
    if (m_autoFixStory) connect(m_autoFixStory.data(), &QTextDocument::contentsChanged, this, &PageView::updateAutoFixButton, Qt::UniqueConnection);
}

void PageView::updateAutoFixButton()
{
    if (!m_autoFixButton) return;
    QRect r;
    if (!autoFixAvailable() || !caretRect(&r)) {
        if (!autoFixAvailable()) m_autoFix = write::AutoFix();   // (over: a later edit or a move of the caret ends the offer for good)
        m_autoFixButton->hide();
        return;
    }
    // (The keyboard's way to the same choices is the command "AutoCorrect Options Menu": its key is said in the description, as the person has it now.)
    const QString key = autoFixKeyText();
    m_autoFixButton->setAccessibleDescription(key.isEmpty() ? QCoreApplication::translate("jp::write::PageView", "Choices for the change AutoCorrect just made.")
                                                            : QCoreApplication::translate("jp::write::PageView", "Choices for the change AutoCorrect just made. Press %1 to open them.").arg(key));
    if (!m_autoFixTold) {
        // (Said once, as the button appears: a person who does not see it is told that there are choices, and how to get to them.)
        m_autoFixTold = true;
        QAccessibleAnnouncementEvent event(m_autoFixButton, key.isEmpty() ? QCoreApplication::translate("jp::write::PageView", "AutoCorrect changed a word. The AutoCorrect Options button has choices.")
                                                                         : QCoreApplication::translate("jp::write::PageView", "AutoCorrect changed a word. Press %1 for choices.").arg(key));
        QAccessible::updateAccessibility(&event);
    }
    m_autoFixButton->adjustSize();
    const QSize size = m_autoFixButton->size();
    m_autoFixButton->move(std::clamp(r.right() + 6, 0, std::max(0, viewport()->width() - size.width())), std::clamp(r.bottom() + 4, 0, std::max(0, viewport()->height() - size.height())));
    m_autoFixButton->show();
    m_autoFixButton->raise();
}

// The key of the command that opens the menu as the person has it now (empty when it has none).
QString PageView::autoFixKeyText()
{
    const QWidget *top = window();
    const QAction *a = top ? top->findChild<QAction *>(QStringLiteral("review.autoCorrectOptions")) : nullptr;
    return a && !a->shortcut().isEmpty() ? a->shortcut().toString(QKeySequence::NativeText) : QString();
}

bool PageView::openAutoFixMenu()
{
    if (!m_autoFixButton || !autoFixAvailable()) return false;
    updateAutoFixButton();
    if (!m_autoFixButton->isVisible()) return false;
    m_autoFixButton->showMenu();
    return true;
}

void PageView::fillAutoFixMenu()
{
    QMenu *menu = m_autoFixButton->menu();
    // (An action is let go later: this runs from the step an action of the menu took.)
    for (QAction *old : menu->actions()) {
        menu->removeAction(old);
        old->deleteLater();
    }
    if (!autoFixAvailable()) return;
    auto word = [](const QString &w) { return QString(w).replace(QLatin1Char('&'), QStringLiteral("&&")); };
    auto add = [this, menu](const QString &text, const char *name, const std::function<void()> &run) {
        QAction *a = menu->addAction(text);
        a->setObjectName(QString::fromLatin1(name));
        connect(a, &QAction::triggered, this, [this, run] {
            run();
            setFocus(Qt::OtherFocusReason);
        });
    };
    add(QCoreApplication::translate("jp::write::PageView", "Undo Automatic Correction"), "autoFixUndo", [this] { undoAutoFix(); });
    QString stop;
    switch (m_autoFix.kind) {
    case autotext::Fix::Kind::Replacement: stop = QCoreApplication::translate("jp::write::PageView", "Stop Replacing \"%1\"").arg(word(m_autoFix.entry)); break;
    case autotext::Fix::Kind::TwoInitials:
    case autotext::Fix::Kind::CapsLock: stop = QCoreApplication::translate("jp::write::PageView", "Keep \"%1\" As Typed").arg(word(m_autoFix.typed)); break;
    case autotext::Fix::Kind::Day: stop = QCoreApplication::translate("jp::write::PageView", "Stop Capitalizing Names of Days Everywhere"); break;
    case autotext::Fix::Kind::Sentence: stop = QCoreApplication::translate("jp::write::PageView", "Stop Capitalizing First Letter of Sentences Everywhere"); break;
    case autotext::Fix::Kind::Cell: stop = QCoreApplication::translate("jp::write::PageView", "Stop Capitalizing First Letter of Table Cells Everywhere"); break;
    case autotext::Fix::Kind::None: break;
    }
    if (!stop.isEmpty()) add(stop, "autoFixStop", [this] { stopAutoFix(); });
    menu->addSeparator();
    add(QCoreApplication::translate("jp::write::PageView", "AutoCorrect Options..."), "autoFixOptions", [this] { openAutoCorrectOptions(); });
}

bool PageView::undoAutoFix()
{
    if (!autoFixAvailable()) return false;
    m_autoFix = write::AutoFix();
    undo();
    updateAutoFixButton();
    return true;
}

bool PageView::stopAutoFix()
{
    if (!autoFixAvailable()) return false;
    const write::AutoFix fix = m_autoFix;
    autotext::Config config = autoCorrectConfig(fix.language);
    switch (fix.kind) {
    case autotext::Fix::Kind::Replacement:
        for (int i = int(config.replacements.size()) - 1; i >= 0; --i)
            if (config.replacements[i].first == fix.entry) config.replacements.removeAt(i);
        break;
    case autotext::Fix::Kind::TwoInitials:
    case autotext::Fix::Kind::CapsLock:
        if (!config.initialExceptions.contains(fix.typed)) config.initialExceptions << fix.typed;
        break;
    case autotext::Fix::Kind::Day: config.days = false; break;
    case autotext::Fix::Kind::Sentence: config.sentences = false; break;
    case autotext::Fix::Kind::Cell: config.cells = false; break;
    case autotext::Fix::Kind::None: return false;
    }
    undoAutoFix();
    saveAutoCorrectConfig(config);
    return true;
}

void PageView::openAutoCorrectOptions()
{
    const QString language = m_doc && !m_doc->proofingLanguage().isEmpty() ? m_doc->proofingLanguage() : QStringLiteral("en-US");
    autoCorrectDialog(window(), m_autoFix.language.isEmpty() ? language : m_autoFix.language);
}

} // namespace jp::write
