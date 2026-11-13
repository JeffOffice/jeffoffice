#pragma once
// Spelling and the thesaurus, as dialog boxes that work on an Editor.

#include <QString>
#include <QStringList>

class QMenu;
class QWidget;

namespace jp {

class Editor;

void spellingDialog(QWidget *p, Editor *ed);
void thesaurusDialog(QWidget *p, Editor *ed);
QStringList thesaurusLookup(const QString &word);

// The right-click menu's spelling help for the text being typed in: when the caret is on a misspelled word, the
// dictionary's suggestions (the first six, in bold; each replaces the word), Ignore All, Add to Dictionary, and a
// separator, added to the menu. False, and nothing added, when it is not.
bool addSpellingSuggestions(QMenu &menu, Editor *ed);
// The same for a grammar or style issue under the caret: its message (it does nothing), the suggestions (the first in bold; each replaces the issue's words),
// Ignore This Issue, Ignore All, and a separator. False, and nothing added, when there is none (or the setting is off, or the language has no rules).
bool addGrammarSuggestions(QMenu &menu, Editor *ed);

} // namespace jp
