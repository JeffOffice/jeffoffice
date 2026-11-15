#pragma once
// Read Aloud (libs/shell/app/readaloud) for a Document of placed objects, JeffPub's and JeffDeck's: its stories (text boxes, shapes' words, table cells) in the order Find walks them, from the
// selection or the text cursor, the word shown as a search match is (a tint, never the selection: what is typed during the reading is not put over it). Any change of the document, or
// a document put in its place, stops the reading and closes the bar.

#include "app/findreplace.h"
#include "app/readaloud.h"
#include "core/export.h"

#include <functional>

namespace jp {

class Editor;

// `reveal` is called before a story's words are shown, for a program that has to bring the story on screen (JeffDeck leaves its slide sorter).
JO_COMMON_EXPORT ReadAloudHost pageReadAloudHost(Editor *editor, const FindScope &scope = FindScope{false, false, 0, false}, const std::function<void(const FindStory &)> &reveal = {});

} // namespace jp
