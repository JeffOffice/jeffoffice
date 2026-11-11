#pragma once
// Go To (Ctrl+G) in JeffDeck: laid out as JeffWrite's is, a "Go to what" list (slide, section, comment, object), the entry, and Next, Previous, and Go To (gotodialog.cpp; the work of finding the place is core/gotoplace.h).

namespace jp {

class DeckWindow;

// Runs the dialog; the window goes where it is told (a comment opens in the Comments pane, an object is selected on its slide).
void showGoToDialog(DeckWindow *window);

} // namespace jp
