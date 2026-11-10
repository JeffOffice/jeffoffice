#pragma once
// The About page of the File screen, the same in all four apps: the suite card, the app's name, version, and
// where to find it, Check for Updates (and the automatic check), the anonymous usage statistics choice, the name
// the app uses as an author, and the license, notices, and third-party software. The app gives the page what is its
// own: its one-line description, what its name is saved as, and the parts it bundles beyond the shared ones.

#include "app/filescreenparts.h"

#include <QList>
#include <QString>
#include <functional>

class QWidget;

namespace jp {

struct AboutHost {
    QString tagline;    // "Free, open-source desktop publishing for Windows, macOS, and Linux."
    QString nameNote;   // what "Your name" is used for in this app
    QList<ThirdParty> extras;   // the app's own bundled parts, after the shared ones
};

// The page for the app (its name, version, repository, and badge come from appInfo()). `screen` is the File screen.
QWidget *buildAboutPage(QWidget *parent, const QWidget *screen, const AboutHost &host);

} // namespace jp
