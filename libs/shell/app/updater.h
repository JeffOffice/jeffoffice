#pragma once
// Update check: asks GitHub's public releases list for a newer JeffOffice
// (the suite's releases: JeffPub's and JeffDeck's are the same) and offers to
// download and run its installer on Windows, which installs every app, or
// to open its release page. Sends nothing about the user.

#include <QNetworkAccessManager>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QVector>
#include <QPointer>

class QDialog;
class QWidget;

namespace jp {

class Updater : public QObject {
    Q_OBJECT
public:
    explicit Updater(QWidget *window);
    // interactive: also report "up to date" and errors (Check for Updates button).
    void check(bool interactive);
    // Check each time JeffPub starts, if automatic checks are on.
    void checkOnStartup();
    static bool checksOnStartup();
    // The Beta channel also offers pre-releases (the test builds in between);
    // Stable, the default, offers only regular releases.
    static bool betaChannel();
    static void setBetaChannel(bool beta);

    // With two apps open, each finds the same new version. The one that offers it first notes that in the suite's settings,
    // and another app is quiet about that version for the rest of the day (an app that offered it offers it again at each
    // start, as ever). "Check for Updates" always offers.
    static bool offeredByAnotherAppToday(const QString &tag);
    static void noteOffered(const QString &tag);

    // Version strings like "0.1.6" or tags like "v0.1.0-preview5", compared numerically.
    static bool isNewer(const QString &candidate, const QString &current);
    // Whether an installer from the releases list may be downloaded and run:
    // from this project's releases on GitHub, with a usable version tag and
    // the SHA-256 GitHub publishes for it.
    static bool trustedInstaller(const QString &url, const QString &tag, const QString &digest);

    // The part of a release's notes worth showing in the offer: its "New
    // in" sections (headings made bold), without the install steps.
    static QString releaseHighlights(const QString &notes);
    // The releases (not drafts, and pre-releases only on the Beta channel)
    // newer than `current`, newest version first, whatever order the list
    // gives them in (a release published again is listed as the newest made).
    static QVector<QJsonObject> newerReleases(const QJsonArray &releases, const QString &current, bool beta);
    // What the offer's exec() returns besides Rejected (Later).
    enum OfferChoice { Install = 1, Skip, OpenPage };
    // The update offer: the new version, its notes (Markdown) and a choice;
    // canInstall offers Update Now, otherwise the download page.
    static QDialog *offerDialog(QWidget *parent, const QString &version, const QString &notes, bool canInstall, const QString &pageUrl);

private:
    void offer(const QString &tag, const QString &notes, const QString &pageUrl, const QString &setupUrl, const QString &digest);
    void downloadAndRun(const QString &url, const QString &tag, const QString &digest, const QString &pageUrl);
    QNetworkAccessManager m_net;
    QPointer<QWidget> m_win;
    bool m_busy = false;
};

Updater *updater();   // the application's one updater (created by main)
void setUpdater(Updater *u);

} // namespace jp
