#include "core/fontfiles.h"

#include "core/fonts.h"
#include "core/sfnt.h"

#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QHash>
#include <QSet>

namespace jp {

namespace {

struct Index {
    QHash<QString, BundledFontFile> files;   // lower-cased family, style
    QSet<QString> families;
};

QString keyOf(const QString &family, bool bold, bool italic) { return family.toLower() + QLatin1Char('|') + QLatin1Char(bold ? 'b' : 'r') + QLatin1Char(italic ? 'i' : 'u'); }

const Index &index()
{
    static const Index built = [] {
        Index out;
        for (const QString &d : bundledFontDirs()) {
            const QDir dir(d);
            if (!dir.exists()) continue;
            for (const QString &name : dir.entryList({QStringLiteral("*.ttf"), QStringLiteral("*.otf")}, QDir::Files, QDir::Name)) {
                QFile file(dir.filePath(name));
                if (!file.open(QIODevice::ReadOnly)) continue;
                const qint64 size = file.size();
                FontFileInfo info;
                bool ok = false;
                if (uchar *map = file.map(0, size)) {
                    ok = inspectFont(map, size, &info);
                    file.unmap(map);
                } else {
                    ok = inspectFont(file.readAll(), &info);
                }
                if (!ok || info.family.isEmpty()) continue;
                BundledFontFile entry{file.fileName(), info.family, info.bold, info.italic, info.fsType};
                for (const QString &family : {info.family, info.typographicFamily}) {
                    if (family.isEmpty()) continue;
                    out.families.insert(family.toLower());
                    if (!out.files.contains(keyOf(family, info.bold, info.italic))) out.files.insert(keyOf(family, info.bold, info.italic), entry);
                }
            }
            if (!out.files.isEmpty()) break;
        }
        return out;
    }();
    return built;
}

} // namespace

BundledFontFile bundledFontFile(const QString &family, bool bold, bool italic) { return index().files.value(keyOf(family, bold, italic)); }

bool isBundledFontFamily(const QString &family) { return index().families.contains(family.toLower()); }

QStringList visibleFontFamilies()
{
    QStringList out = QFontDatabase::families();
    out.removeIf([](const QString &family) { return family.startsWith(kEmbeddedFamilyPrefix); });
    return out;
}

} // namespace jp
