#pragma once
#include <memory>
// JeffCrunch's File screen: Open (with the workbooks opened lately), Print (with a preview of the pages), Export to PDF, Close, Options, and About, on the frame every app's File screen shares.

#include "app/filescreen.h"

namespace jp {

class CrunchWindow;

// For tests: the closure that draws the Print screen's preview, as the last Print screen made it (expired once that screen is gone).
std::weak_ptr<void> printPaintClosureForTest();

class CrunchFileScreen : public FileScreen {
    Q_OBJECT
public:
    explicit CrunchFileScreen(CrunchWindow *win);

private:
    QWidget *buildPage(const QString &key);
    QWidget *buildOpen();
    QWidget *buildPrint();    // (crunchprintpage.cpp)
    QWidget *buildExport();
    QWidget *buildAbout();
    CrunchWindow *m_win;
};

} // namespace jp
