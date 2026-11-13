#pragma once
// Go To (Ctrl+G) in JeffPub: a "Go to what" list (page, comment), the entry, and Next, Previous, and Go To, laid out as JeffDeck's is (pagegoto.cpp).

namespace jp {

class MainWindow;

// Runs the dialog; a comment opens in the Comments pane.
void showPageGoToDialog(MainWindow *window);

} // namespace jp
