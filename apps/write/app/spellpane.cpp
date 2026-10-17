#include "app/spellpane.h"

#include "app/pageview.h"
#include "app/spelling.h"
#include "core/spellcheck.h"
#include "text/textprops.h"

#include <QAction>
#include <QCoreApplication>
#include <QFont>
#include <QMenu>
#include <QPointer>
#include <QTextCursor>
#include <QTextDocument>

namespace jp::write {

namespace {

// The language of the word at a position: the text's own, else the document's.
QString languageAt(PageView *view, int position)
{
    QTextCursor c(view->document()->story());
    c.setPosition(position + 1);
    const QString language = c.charFormat().stringProperty(tp::Language);
    return language.isEmpty() ? view->spelling()->defaultLanguage() : language;
}

} // namespace

bool addSpellingSuggestions(QMenu &menu, PageView *view)
{
    if (!view || !view->document() || !view->spelling() || !view->spelling()->enabled()) return false;
    const QPair<int, int> word = view->spelling()->misspellingAt(view->cursor().position());
    if (word.first < 0) return false;
    QTextCursor w(view->document()->story());
    w.setPosition(word.first);
    w.setPosition(word.second, QTextCursor::KeepAnchor);
    const QString text = w.selectedText();
    const QStringList suggestions = spellingSuggestions(text, languageAt(view, word.first)).mid(0, 6);
    QPointer<PageView> where = view;
    bool first = true;
    for (const QString &s : suggestions) {
        QAction *a = menu.addAction(s);
        if (first) {
            QFont f = a->font();
            f.setBold(true);
            a->setFont(f);
            first = false;
        }
        QObject::connect(a, &QAction::triggered, &menu, [where, word, s] {
            if (!where) return;
            replaceWord(where->document()->story(), word.first, word.second, s);
            QTextCursor c = where->cursor();
            c.setPosition(word.first + int(s.size()));
            where->setCursor(c);
        });
    }
    if (suggestions.isEmpty()) menu.addAction(QCoreApplication::translate("Proofing", "(No spelling suggestions)"))->setEnabled(false);
    QObject::connect(menu.addAction(QCoreApplication::translate("Proofing", "Ignore All")), &QAction::triggered, &menu, [where, text] {
        spellingIgnore(text);
        if (where) where->spelling()->refresh();
    });
    QObject::connect(menu.addAction(QCoreApplication::translate("Proofing", "Add to Dictionary")), &QAction::triggered, &menu, [where, text] {
        spellingAdd(text);
        if (where) where->spelling()->refresh();
    });
    menu.addSeparator();
    return true;
}

} // namespace jp::write
