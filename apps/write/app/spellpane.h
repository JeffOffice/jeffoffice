#pragma once
// Spelling in the window: the help the right-click menu gives on a misspelled word, and the Spelling pane that steps through the misspellings.

class QMenu;

namespace jp::write {

class PageView;

// When the caret is on a misspelled word: the dictionary's suggestions (the first six, the first in bold; each puts itself in the word's place), Ignore All,
// and Add to Dictionary, added to the menu. False, and nothing added, when it is not.
bool addSpellingSuggestions(QMenu &menu, PageView *view);

} // namespace jp::write
