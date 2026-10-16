#pragma once
// JeffWrite's File screen: New, Open, Save, Save As, Print, Export, Close, Options, and About, on the frame every app's File screen shares.

#include "app/filescreen.h"

namespace jp {

class WriteWindow;

class WriteFileScreen : public FileScreen {
    Q_OBJECT
public:
    explicit WriteFileScreen(WriteWindow *win);

private:
    QWidget *buildPage(const QString &key);
    QWidget *buildNew();
    QWidget *buildOpen();
    QWidget *buildPrint();
    QWidget *buildExport();
    QWidget *buildAbout();
    void startBlank();
    WriteWindow *m_win;
};

} // namespace jp
