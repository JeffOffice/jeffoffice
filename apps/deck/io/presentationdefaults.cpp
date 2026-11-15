#include "io/presentationdefaults.h"

#include "app/userdefaults.h"
#include "core/themes.h"
#include "io/presentationfile.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>

namespace jp::deck {

namespace {
const QString kSizeKey = QStringLiteral("deckSlideSize");

// The theme file as it was read, kept until the file changes (its size and time), so that a new presentation does not read and parse it again.
struct ThemeCache {
    QString path;
    qint64 size = -1;
    QDateTime modified;
    std::shared_ptr<const Document> theme;   // none for a file that gave no theme
};
ThemeCache g_cache;
int g_reads = 0;

std::shared_ptr<const Document> cachedDefaultTheme()
{
    const QFileInfo info(defaultThemePath());
    if (!info.exists() || info.size() > kMaxDefaultThemeBytes) return nullptr;
    if (g_cache.path == info.absoluteFilePath() && g_cache.size == info.size() && g_cache.modified == info.lastModified()) return g_cache.theme;
    ++g_reads;
    std::shared_ptr<const Document> source = importAnyPresentationFile(defaultThemePath());
    if (source && themeSourceProblem(*source) != ThemeSource::Fine) source.reset();
    g_cache = {info.absoluteFilePath(), info.size(), info.lastModified(), source};
    return source;
}
}

int defaultThemeReadsForTest() { return g_reads; }

QString defaultThemePath() { return userDefaultFile(QStringLiteral("Theme.pptx")); }

bool setDefaultTheme(const Document &current, QString *error)
{
    // The theme goes on a new presentation, so the file holds none of this one's slides.
    std::unique_ptr<Document> sample = newPresentation();
    if (!applyThemeToAll(*sample, current)) {
        if (error) *error = QCoreApplication::translate("Dialogs", "The presentation has no theme to keep.");
        return false;
    }
    g_cache = ThemeCache();
    return saveAnyPresentationFile(*sample, defaultThemePath(), error);
}

void setDefaultSlideSize(const QSizeF &size)
{
    setUserDefault(kSizeKey, QJsonObject{{QStringLiteral("width"), size.width()}, {QStringLiteral("height"), size.height()}});
}

void clearDefaultSlideSize() { clearUserDefault(kSizeKey); }

std::unique_ptr<Document> newDefaultPresentation()
{
    std::unique_ptr<Document> d = newPresentation();
    if (const std::shared_ptr<const Document> source = cachedDefaultTheme()) applyThemeToAll(*d, *source);
    const QJsonObject size = userDefault(kSizeKey);
    const QSizeF wanted(size.value(QStringLiteral("width")).toDouble(), size.value(QStringLiteral("height")).toDouble());
    if (wanted.width() >= 72 && wanted.height() >= 72 && wanted != d->pageSize()) setSlideSize(*d, wanted);
    return d;
}

} // namespace jp::deck
