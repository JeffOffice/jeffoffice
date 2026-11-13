// JeffWrite's Home tab commands that are small and have a dialog or a menu of their own: Change Case and Sort, and Word Count (Review).

#include "app/writewindow.h"

#include "app/homedialogs.h"
#include "app/widgets.h"
#include "app/pageview.h"
#include "app/writefilescreen.h"
#include "app/pageview.h"
#include "app/writefilescreen.h"
#include "core/editops.h"
#include "core/tableops.h"
#include "core/homeops.h"
#include "core/proofops.h"
#include "translate/languages.h"

#include "text/dictionaries.h"

#include "app/settings.h"

#include <QAction>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
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
    mk("para.horizontalLine", tr("Horizontal Line"), "minus", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        QTextCursor c = m_view->cursor();
        if (write::insertHorizontalLine(*m_doc, c)) m_view->setCursor(c);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("style.inspector", tr("Reveal Formatting"), "search", QKeySequence(Qt::SHIFT | Qt::Key_F1), [this] {
        if (!m_doc) return;
        write::formattingDialog(this, write::describeFormatting(*m_doc, m_view->cursor()));
        m_view->setFocus(Qt::OtherFocusReason);
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
        // (a program built without the translator keeps the way through a website)
        if (translate::builtInBackend()) {
            showTranslatePane();
            return;
        }
        const QTextCursor sel = m_view->cursor();
        const bool whole = !sel.hasSelection();
        jp::translateText(this, [this](const QString &message, int ms) { statusBar()->showMessage(message, ms); }, whole ? m_doc->story()->toPlainText() : sel.selectedText(), whole, m_urlOpener);
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
    mk("review.wordCount", tr("Word Count"), "hash", QKeySequence(), [this] {
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
        if (write::selectedBlocks(m_view->cursor()).size() > write::kMostSortParagraphs) {
            statusBar()->showMessage(tr("There are too many paragraphs to sort at once. Select %n paragraph(s) or fewer.", nullptr, write::kMostSortParagraphs), 8000);
            return 0;
        }
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

namespace jp {

void WriteWindow::createMiniToolbar()
{
    m_mini = new QFrame(m_view->viewport());
    m_mini->setObjectName(QStringLiteral("miniToolbar"));
    m_mini->setFrameShape(QFrame::StyledPanel);
    m_mini->setAutoFillBackground(true);
    auto *column = new QVBoxLayout(m_mini);
    column->setContentsMargins(3, 2, 3, 2);
    column->setSpacing(1);
    // The font box on top, then the buttons and the font color (the same commands as the ribbon's, and they follow the selection as its boxes do).
    auto *font = new FontCombo(m_mini);
    connect(font, &FontCombo::familyChosen, this, [this](const QString &f) {
        if (refuseIfReadOnly()) return;
        if (m_doc) write::setFontFamily(*m_doc, m_view->cursor(), f);
        m_view->setFocus();
    });
    m_fontCombos << font;
    column->addWidget(font);
    auto *row = new QHBoxLayout;
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(1);
    column->addLayout(row);
    for (const char *id : {"fmt.bold", "fmt.italic", "fmt.underline", "fmt.shrink", "fmt.grow", "fmt.alignLeft", "fmt.alignCenter", "fmt.alignRight", "para.bullets", "para.numbers"}) {
        auto *button = new QToolButton(m_mini);
        button->setDefaultAction(act(QString::fromLatin1(id)));
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setAutoRaise(true);
        button->setFocusPolicy(Qt::NoFocus);   // (the text keeps the keyboard)
        row->addWidget(button);
    }
    auto *color = new ColorButton(QStringLiteral("baseline"), tr("Font Color"), true, tr("Automatic"), m_mini);
    color->setCurrent(ColorRef::rgb(QColor(0xC0, 0, 0)));
    connect(color, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
        if (!m_doc || refuseIfReadOnly()) return;
        write::setTextColor(*m_doc, m_view->cursor(), c.isNone() ? QColor() : c.resolve(m_doc->flow().env.colors));
        m_view->setFocus();
    });
    m_colorButtons << color;
    row->addWidget(color);
    m_mini->hide();
    connect(m_view, &write::PageView::selectionMade, this, [this] { updateMiniToolbar(true); });
    connect(m_view, &write::PageView::caretChanged, this, [this] { updateMiniToolbar(false); });
}

// Shown above the first line of the selection (below the last when there is no room above) once a mouse selection ends; follows the selection while it stays; gone with it.
void WriteWindow::updateMiniToolbar(bool show)
{
    if (!m_mini) return;
    const QTextCursor &c = m_view->cursor();
    if (!m_doc || !c.hasSelection() || c.document() != m_doc->story() || isReadOnly() || (!show && !m_mini->isVisible())) {
        m_mini->hide();
        return;
    }
    QRect first, last;
    if (!m_view->positionRect(c.selectionStart(), &first) || !m_view->positionRect(c.selectionEnd(), &last)) {
        m_mini->hide();
        return;
    }
    const QSize size = m_mini->sizeHint();
    const QRect area = m_view->viewport()->rect();
    int y = first.top() - size.height() - 6;
    if (y < 0) y = last.bottom() + 8;
    m_mini->setGeometry(QRect(QPoint(std::clamp(first.left(), 0, std::max(0, area.width() - size.width())), std::clamp(y, 0, std::max(0, area.height() - size.height()))), size));
    m_mini->show();
    m_mini->raise();
}


namespace {

// How the translation goes into the document: with the document's own edits where it keeps track of changes, and the language of the text marked.
class WriteTranslateHooks : public translate::EditHooks {
public:
    explicit WriteTranslateHooks(std::function<write::WriteDoc *()> doc) : m_doc(std::move(doc)) {}
    QTextCharFormat languageFormat(const QTextCharFormat &format, const QString &language) override
    {
        const QString tag = dict::match(language);
        return tag.isEmpty() ? format : write::withProofingLanguage(format, tag);
    }
    void replaceSelection(QTextCursor &c, const QVector<translate::Run> &runs) override
    {
        write::WriteDoc *doc = m_doc();
        if (!doc || !doc->trackChanges()) {
            translate::EditHooks::replaceSelection(c, runs);
            return;
        }
        // (a change that is tracked is made by the document's own typing, which has no looks to give: the text takes the look the typing has)
        QString text;
        for (const translate::Run &r : runs) text += r.text;
        write::insertText(*doc, c, text);
    }
    void insertParagraphAfter(QTextCursor &c, const QTextBlockFormat &like, const QVector<translate::Run> &runs) override
    {
        Q_UNUSED(like)
        write::WriteDoc *doc = m_doc();
        if (!doc) return;
        c.beginEditBlock();
        write::insertParagraph(*doc, c);
        if (doc->trackChanges()) {
            QString text;
            for (const translate::Run &r : runs) text += r.text;
            write::insertText(*doc, c, text);
        } else {
            for (const translate::Run &r : runs) c.insertText(r.text, r.format);
        }
        c.endEditBlock();
    }

private:
    std::function<write::WriteDoc *()> m_doc;
};

} // namespace

void WriteWindow::showTranslatePane()
{
    if (!m_doc) return;
    if (m_fileScreen->isVisible()) hideFileScreen();
    if (!m_translatePane) {
        m_translateHost = std::make_shared<FunctionHost>();
        m_translateHooks = std::make_shared<WriteTranslateHooks>([this] { return m_doc.get(); });
        FunctionHost *host = m_translateHost.get();
        host->sourceFn = [this] {
            if (!m_doc) return translate::Source();
            return translate::readSource(m_view->cursor());
        };
        host->languageFn = [this] { return m_doc ? translate::languageOfTag(write::proofingLanguageAt(*m_doc, m_view->cursor())) : QString(); };
        host->browserFn = [this](const QString &text, const QString &code) {
            const QString server = Settings::get().value(QStringLiteral("translate/server"), QStringLiteral("https://libretranslate.com/")).toString();
            openTranslation(translateReady(text), translate::websiteCode(code), server);
        };
        host->replaceFn = [this](const translate::Source &s, const translate::Result &r) {
            if (!m_doc || refuseIfReadOnly()) return TranslateApplied::Refused;
            return translate::applyReplace(s, r, *m_translateHooks) ? TranslateApplied::Done : TranslateApplied::Changed;
        };
        host->insertBelowFn = [this](const translate::Source &s, const translate::Result &r) {
            if (!m_doc || refuseIfReadOnly()) return TranslateApplied::Refused;
            return translate::applyInsertBelow(s, r, *m_translateHooks) ? TranslateApplied::Done : TranslateApplied::Changed;
        };
        m_translateDock = new QDockWidget(tr("Translate"), this);
        m_translateDock->setObjectName(QStringLiteral("translateDock"));
        m_translateDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_translatePane = TranslatePane::create(host);
        // With the margins the window's other panes have.
        auto *frame = new QWidget(m_translateDock);
        auto *column = new QVBoxLayout(frame);
        column->setContentsMargins(8, 8, 8, 8);
        column->addWidget(m_translatePane);
        m_translateDock->setWidget(frame);
        addDockWidget(Qt::RightDockWidgetArea, m_translateDock);
        resizeDocks({m_translateDock}, {320}, Qt::Horizontal);
    }
    m_translateDock->show();
    m_translateDock->raise();
    m_translatePane->refresh();
}

} // namespace jp
