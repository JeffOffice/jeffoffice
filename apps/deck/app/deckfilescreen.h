#pragma once
// JeffDeck's File screen: New, Open, Save, Save As, Print, Export, Close, and
// About, on the frame every app's File screen shares.

#include "app/filescreen.h"

#include <functional>
#include <memory>

namespace jp {

class DeckWindow;
class Document;

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
    void startFrom(const std::function<std::unique_ptr<Document>()> &make);
    DeckWindow *m_win;
};

} // namespace jp
