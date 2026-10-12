#pragma once
// JeffDeck's File screen: New, Open, Save, Save As, Print, Export, Close, and
// About, on the frame every app's File screen shares.

#include "app/filescreen.h"

namespace jp {

class DeckWindow;

class DeckFileScreen : public FileScreen {
    Q_OBJECT
public:
    explicit DeckFileScreen(DeckWindow *win);

private:
    QWidget *buildPage(const QString &key);
    QWidget *buildNew();
    QWidget *buildOpen();
    QWidget *buildPrint();
    QWidget *buildExport();
    QWidget *buildAbout();
    void startBlank();
    DeckWindow *m_win;
};

} // namespace jp
