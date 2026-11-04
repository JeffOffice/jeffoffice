// JeffWrite's Home tab commands that are small and have a dialog or a menu of their own: Change Case and Sort, and Word Count (Review).

#include "app/writewindow.h"

#include "app/homedialogs.h"
#include "app/pageview.h"
#include "app/writefilescreen.h"
#include "app/pageview.h"
#include "app/writefilescreen.h"
#include "core/editops.h"
#include "core/tableops.h"
#include "core/homeops.h"
#include "core/proofops.h"

#include "text/dictionaries.h"

#include "app/settings.h"

#include <QAction>
#include <QDockWidget>
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QStatusBar>
#include <QToolButton>

namespace jp {

void WriteWindow::createHomeActions()
{
    struct Case { const char *id; const char *text; write::CaseMode mode; };
    for (const Case &k : {Case{"fmt.case.sentence", QT_TRANSLATE_NOOP("jp::WriteWindow", "Sentence case"), write::CaseMode::Sentence}, Case{"fmt.case.lower", QT_TRANSLATE_NOOP("jp::WriteWindow", "lowercase"), write::CaseMode::Lower},
                          Case{"fmt.case.upper", QT_TRANSLATE_NOOP("jp::WriteWindow", "UPPERCASE"), write::CaseMode::Upper}, Case{"fmt.case.capitalize", QT_TRANSLATE_NOOP("jp::WriteWindow", "Capitalize Words"), write::CaseMode::Capitalize},
                          Case{"fmt.case.toggle", QT_TRANSLATE_NOOP("jp::WriteWindow", "tOGGLE cASE"), write::CaseMode::Toggle}}) {
        const write::CaseMode mode = k.mode;
        mk(QString::fromLatin1(k.id), QCoreApplication::translate("jp::WriteWindow", k.text), "", QKeySequence(), [this, mode] {
            if (!m_doc || refuseIfReadOnly()) return;
            write::changeCase(*m_doc, m_view->cursor(), mode);
            m_view->setFocus(Qt::OtherFocusReason);
            m_view->viewport()->update();
        });
    }
    using Kind = write::PageView::PasteKind;
    auto pasteWith = [this](Kind kind) {
        if (!m_doc || refuseIfReadOnly()) return;
        m_view->pasteAs(kind);
        m_view->setFocus(Qt::OtherFocusReason);
    };
    mk("edit.pasteMerge", tr("Merge Formatting"), "clipboard-paste", QKeySequence(), [pasteWith] { pasteWith(Kind::Merge); });
    mk("edit.pasteText", tr("Keep Text Only"), "clipboard-type", QKeySequence(), [pasteWith] { pasteWith(Kind::TextOnly); });
    mk("edit.pastePicture", tr("Picture"), "image", QKeySequence(), [pasteWith] { pasteWith(Kind::Picture); });
    mk("edit.pasteSpecial", tr("Paste Special…"), "clipboard-list", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_V), [this, pasteWith] {
        if (!m_doc || refuseIfReadOnly()) return;
        struct Choice { Kind kind; QString name; };
        QVector<Choice> choices;
        if (m_view->canPaste(Kind::Keep) && !QApplication::clipboard()->mimeData()->hasImage()) choices.push_back({Kind::Keep, tr("Formatted text")});
        if (m_view->canPaste(Kind::Merge)) choices.push_back({Kind::Merge, tr("Formatted text, merged with the formatting around it")});
        if (m_view->canPaste(Kind::TextOnly)) choices.push_back({Kind::TextOnly, tr("Unformatted text")});
        if (m_view->canPaste(Kind::Html)) choices.push_back({Kind::Html, tr("HTML")});
        if (m_view->canPaste(Kind::Picture)) choices.push_back({Kind::Picture, tr("Picture")});
        if (choices.isEmpty()) {
            statusBar()->showMessage(tr("There is nothing on the clipboard to paste."), 5000);
            return;
        }
        QStringList names;
        for (const Choice &c : std::as_const(choices)) names << c.name;
        const int pick = write::pasteSpecialDialog(this, names);
        if (pick >= 0) pasteWith(choices.at(pick).kind);
    });
    auto syncPaste = [this] {
        if (!m_view) return;
        act("edit.pasteMerge")->setEnabled(bool(m_doc) && m_view->canPaste(Kind::Merge));
        act("edit.pasteText")->setEnabled(bool(m_doc) && m_view->canPaste(Kind::TextOnly));
        act("edit.pastePicture")->setEnabled(bool(m_doc) && m_view->canPaste(Kind::Picture));
        act("edit.pasteSpecial")->setEnabled(bool(m_doc));
    };
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, syncPaste);
    syncPaste();
    mk("fmt.case.cycle", tr("Change Case"), "case-sensitive", QKeySequence(Qt::SHIFT | Qt::Key_F3), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        QTextCursor probe = m_view->cursor();
        if (!probe.hasSelection()) probe.select(QTextCursor::WordUnderCursor);
        write::changeCase(*m_doc, m_view->cursor(), write::nextCaseInCycle(probe.selectedText()));
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
    });
    mk("para.sort", tr("Sort…"), "arrow-down-a-z", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        if (m_doc->trackChanges()) {
            statusBar()->showMessage(tr("Turn off Track Changes to sort: a sort is not recorded as a change."), 6000);
            return;
        }
        const bool table = cursorInTable();
        if (!table && (!m_view->cursor().hasSelection() || write::selectedBlocks(m_view->cursor()).size() < 2)) {
            statusBar()->showMessage(tr("Select two or more paragraphs to sort."), 5000);
            return;
        }
        if (!write::sortDialog(this, &m_sortOptions, table)) return;
        sortSelection(m_sortOptions);
    });
    mk("review.language", tr("Set Proofing Language…"), "languages", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        QString tag;
        bool noProof = false;
        if (!write::languageDialog(this, write::proofingLanguageAt(*m_doc, m_view->cursor()), write::noProofAt(m_view->cursor()), &tag, &noProof)) return;
        QTextCursor group = m_view->cursor();
        group.beginEditBlock();
        write::setProofingLanguage(*m_doc, m_view->cursor(), tag);
        write::setNoProof(*m_doc, m_view->cursor(), noProof);
        group.endEditBlock();
        m_view->setFocus(Qt::OtherFocusReason);
        refreshUi();
    });
    mk("review.noProof", tr("Do Not Check Spelling"), "spell-check-2", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        write::setNoProof(*m_doc, m_view->cursor(), act("review.noProof")->isChecked());
        m_view->setFocus(Qt::OtherFocusReason);
    }, true);
    mk("review.thesaurus", tr("Thesaurus"), "book-a", QKeySequence(Qt::SHIFT | Qt::Key_F7), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        QTextCursor word = m_view->cursor();
        if (!word.hasSelection()) word.select(QTextCursor::WordUnderCursor);
        const QString chosen = write::thesaurusDialog(this, word.selectedText().trimmed());
        if (!chosen.isEmpty() && word.hasSelection()) {
            // (The word is replaced by the word chosen, in the case of the word: a capital stays a capital.)
            QString replacement = chosen;
            const QString was = word.selectedText().trimmed();
            if (!was.isEmpty() && was.at(0).isUpper() && !replacement.isEmpty()) replacement[0] = replacement.at(0).toUpper();
            write::insertText(*m_doc, word, replacement);
            m_view->setCursor(word);
        }
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("review.translate", tr("Translate"), "languages", QKeySequence(), [this] {
        if (!m_doc) return;
        const QTextCursor sel = m_view->cursor();
        const bool whole = !sel.hasSelection();
        QString text = whole ? m_doc->story()->toPlainText() : sel.selectedText();
        text.replace(QChar::ParagraphSeparator, QLatin1Char('\n')).replace(QChar::LineSeparator, QLatin1Char('\n')).remove(QChar::ObjectReplacementCharacter);
        text = text.trimmed();
        if (text.isEmpty()) {
            statusBar()->showMessage(tr("There is no text to translate."), 5000);
            return;
        }
        QString target = Settings::get().value(QStringLiteral("translate/targetCode"), QStringLiteral("es")).toString();
        QString server = Settings::get().value(QStringLiteral("translate/server"), QStringLiteral("https://libretranslate.com/")).toString();
        if (!write::translateDialog(this, whole, &target, &server)) return;
        const QString url = write::translateUrl(server, target, text);
        if (url.isEmpty()) {
            statusBar()->showMessage(tr("The translation service needs a web address (http or https)."), 6000);
            return;
        }
        Settings::get().setValue(QStringLiteral("translate/targetCode"), target);
        Settings::get().setValue(QStringLiteral("translate/server"), server);
        QDesktopServices::openUrl(QUrl(url));
    });
    mk("review.accessibility", tr("Check Accessibility"), "accessibility", QKeySequence(), [this] {
        if (!m_doc) return;
        m_view->leaveHeaderFooter();
        m_view->leaveNote();
        if (m_fileScreen->isVisible()) hideFileScreen();
        if (!m_accessDock) {
            m_accessDock = new QDockWidget(tr("Accessibility"), this);
            m_accessDock->setObjectName(QStringLiteral("accessDock"));
            m_accessDock->setFeatures(QDockWidget::DockWidgetClosable);
            m_accessPane = new write::AccessPane(m_view, m_accessDock);
            m_accessDock->setWidget(m_accessPane);
            addDockWidget(Qt::RightDockWidgetArea, m_accessDock);
            resizeDocks({m_accessDock}, {340}, Qt::Horizontal);
        }
        m_accessDock->show();
        m_accessDock->raise();
        m_accessPane->check();
    });
    mk("review.wordCount", tr("Count Words"), "hash", QKeySequence(), [this] {
        if (!m_doc) return;
        const QTextCursor sel = m_view->cursor();
        write::wordCountDialog(this, m_view->pageCount(), sel.hasSelection(), [this, sel](bool notes) { return write::countWords(*m_doc, sel, notes); });
        m_view->setFocus(Qt::OtherFocusReason);
    });
}

// Sorts the paragraphs selected, or the rows of the table the caret is in; how many were sorted.
int WriteWindow::sortSelection(const write::SortOptions &options)
{
    if (!m_doc) return 0;
    int n = 0;
    if (cursorInTable()) {
        QVector<write::TableSortKey> keys;
        for (const write::SortKey &k : options.keys)
            keys.push_back({k.type == write::SortKey::Number ? write::TableSortKey::Kind::Number : k.type == write::SortKey::Date ? write::TableSortKey::Kind::Date : write::TableSortKey::Kind::Text, std::max(0, k.field - 1), !k.descending});
        QTextCursor c = m_view->cursor();
        n = write::sortTableRows(*m_doc, c, keys, options.header) ? 1 : 0;
        statusBar()->showMessage(n > 0 ? tr("The rows are sorted.") : tr("Nothing was sorted."), 5000);
    } else {
        n = write::sortParagraphs(*m_doc, m_view->cursor(), options);
        statusBar()->showMessage(n > 0 ? tr("%n paragraph(s) sorted.", nullptr, n) : tr("Nothing was sorted."), 5000);
    }
    m_view->setFocus(Qt::OtherFocusReason);
    m_view->viewport()->update();
    return n;
}

// The language at the caret in the status bar, and the check mark of "Do Not Check Spelling".
void WriteWindow::updateLanguage()
{
    if (!m_langLabel) return;
    if (!m_doc) {
        m_langLabel->setText(QString());
        return;
    }
    m_langLabel->setText(dict::languageName(write::proofingLanguageAt(*m_doc, m_view->cursor())));
    QAction *skip = act("review.noProof");
    const QSignalBlocker block(skip);
    skip->setChecked(write::noProofAt(m_view->cursor()));
}

} // namespace jp
