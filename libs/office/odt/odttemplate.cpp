#include "odt/odttemplate.h"

#include "io/zip.h"

#include <QMap>
#include <QStringList>

namespace jp::office::odt {

QByteArray odfAsTemplate(const QByteArray &odf)
{
    QMap<QString, QByteArray> parts;
    if (!readZip(odf, parts)) return odf;
    const QByteArray mime = parts.value(QStringLiteral("mimetype")).trimmed();
    if (!mime.startsWith("application/vnd.oasis.opendocument.") || mime.endsWith("-template")) return odf;
    const QByteArray templateMime = mime + "-template";
    // The "mimetype" part comes first and is stored, as the standard asks; the rest are as they were.
    ZipWriter z;
    z.add(QStringLiteral("mimetype"), templateMime, false);
    for (auto it = parts.cbegin(); it != parts.cend(); ++it) {
        if (it.key() == QLatin1String("mimetype")) continue;
        QByteArray data = it.value();
        if (it.key() == QLatin1String("META-INF/manifest.xml")) {
            // (the root entry's media type: the one place the manifest names the type of the whole package)
            const QByteArray from = "manifest:full-path=\"/\"";
            const qsizetype at = data.indexOf(from);
            if (at >= 0) {
                const qsizetype end = data.indexOf('>', at);
                QByteArray tag = data.mid(at, end - at);
                tag.replace("manifest:media-type=\"" + mime + "\"", "manifest:media-type=\"" + templateMime + "\"");
                data = data.left(at) + tag + data.mid(end);
            }
        }
        z.add(it.key(), data, true);
    }
    return z.finish();
}

} // namespace jp::office::odt
