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
    // While held, the New page makes no template thumbnails (they are laid out with the bundled fonts, which a start adds after its first window shows); its list shows
    // the blank page in their places until released. The New page can be up and drawn at the first paint that way.
    void holdThumbnails(bool hold) { m_holdThumbs = hold; }
    int thumbnailCount() const { return int(m_thumbs.size()); }
    int heldThumbnailTicks() const { return m_heldTicks; }   // (for tests: how many times the thumbnail timer ran and found them held)

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
    bool m_holdThumbs = false;
    int m_heldTicks = 0;
};

} // namespace jp
