#include "io/presentationdefaults.h"

#include "app/userdefaults.h"
#include "core/themes.h"
#include "io/presentationfile.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonObject>

namespace jp::deck {

namespace {
const QString kSizeKey = QStringLiteral("deckSlideSize");
}

QString defaultThemePath() { return userDefaultFile(QStringLiteral("Theme.pptx")); }

bool setDefaultTheme(const Document &current, QString *error)
{
    // The theme goes on a new presentation, so the file holds none of this one's slides.
    std::unique_ptr<Document> sample = newPresentation();
    if (!applyThemeToAll(*sample, current)) {
        if (error) *error = QCoreApplication::translate("Dialogs", "The presentation has no theme to keep.");
        return false;
    }
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
    if (QFile::exists(defaultThemePath())) {
        if (const std::unique_ptr<Document> source = importAnyPresentationFile(defaultThemePath()); source && themeSourceProblem(*source) == ThemeSource::Fine) applyThemeToAll(*d, *source);
    }
    const QJsonObject size = userDefault(kSizeKey);
    const QSizeF wanted(size.value(QStringLiteral("width")).toDouble(), size.value(QStringLiteral("height")).toDouble());
    if (wanted.width() >= 72 && wanted.height() >= 72 && wanted != d->pageSize()) setSlideSize(*d, wanted);
    return d;
}

} // namespace jp::deck
