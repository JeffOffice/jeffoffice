#pragma once
// Read Aloud (libs/shell/app/readaloud) for a Document of placed objects, JeffPub's and JeffDeck's: its stories (text boxes, shapes' words, table cells) in the order Find walks them, from the
// selection or the text cursor, the word shown selected in its box.

#include "app/findreplace.h"
#include "app/readaloud.h"
#include "core/export.h"

#include <functional>

namespace jp {

class Editor;

// `show` brings a match to the screen (showFindMatch by default: a program with a story of its own, such as speaker notes, passes its own).
JO_COMMON_EXPORT ReadAloudHost pageReadAloudHost(Editor *editor, const FindScope &scope = FindScope{false, false, 0, false},
                                                 const std::function<void(const FindStory &, const QTextCursor &)> &show = {});

} // namespace jp
