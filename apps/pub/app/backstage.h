#pragma once
// The File screen: a sidebar of grouped commands (New, Open, Save, Print,
// Export, Share, Properties, Close, Settings, About) beside the chosen page.

#include "app/filescreen.h"

#include <QHash>
#include <QIcon>

namespace jp {

class MainWindow;

class Backstage : public FileScreen {
    Q_OBJECT
public:
    explicit Backstage(MainWindow *win);

private:
    QWidget *buildPage(const QString &key);
    QWidget *buildInfo();
    // The gallery of designs: for a new publication, or, with `change`, for
    // Change Template (the open publication's text and pictures move into it).
    QWidget *buildNew(bool change = false);
    QWidget *buildOpen();
    QWidget *buildPrint();
    QWidget *buildShare();
    QWidget *buildExport();
    QWidget *buildAbout();
    MainWindow *m_win;
    // New page thumbnails made so far, by template id, all made with the
    // options in m_thumbOptions (other options start the collection afresh).
    QHash<QString, QIcon> m_thumbs;
    QString m_thumbOptions;
};

} // namespace jp
