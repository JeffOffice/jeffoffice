#pragma once
// JeffCrunch's File screen: Open (with the workbooks opened lately), Close, Options, and About, on the frame every app's File screen shares.

#include "app/filescreen.h"

namespace jp {

class CrunchWindow;

class CrunchFileScreen : public FileScreen {
    Q_OBJECT
public:
    explicit CrunchFileScreen(CrunchWindow *win);

private:
    QWidget *buildPage(const QString &key);
    QWidget *buildOpen();
    QWidget *buildAbout();
    CrunchWindow *m_win;
};

} // namespace jp
