#pragma once
// For the Pack and Go tests of the apps: what is wrong with a packed package, found without the program's own readers (empty when nothing is): every internal relationship names a part that is
// there, an external one stays external (and is a web address, or a name inside the folder, never a path out), and every part has a content type.
#include "io/zip.h"

#include <QDir>
#include <QMap>
#include <QSet>
#include <QString>
#include <QUrl>
#include <QXmlStreamReader>

namespace packedcheck {

inline QString problemsOf(const QByteArray &package, int *externalLinks = nullptr)
{
    QMap<QString, QByteArray> parts;
    if (!jp::readZip(package, parts)) return QStringLiteral("not a ZIP");
    if (!parts.contains(QStringLiteral("[Content_Types].xml"))) return QStringLiteral("no content types");
    QSet<QString> defaults, overrides;
    QXmlStreamReader types(parts.value(QStringLiteral("[Content_Types].xml")));
    while (!types.atEnd()) {
        types.readNext();
        if (!types.isStartElement()) continue;
        if (types.name() == QLatin1String("Default")) defaults.insert(types.attributes().value(QLatin1String("Extension")).toString().toLower());
        else if (types.name() == QLatin1String("Override")) overrides.insert(types.attributes().value(QLatin1String("PartName")).toString());
    }
    if (types.hasError()) return QStringLiteral("content types: ") + types.errorString();
    QStringList problems;
    int external = 0;
    for (auto it = parts.cbegin(); it != parts.cend(); ++it) {
        const QString &name = it.key();
        if (name != QLatin1String("[Content_Types].xml") && !overrides.contains(QLatin1Char('/') + name) && !defaults.contains(QFileInfo(name).suffix().toLower())) problems << name + QStringLiteral(": no content type");
        if (!name.endsWith(QLatin1String(".rels"))) continue;
        const QString dir = name.left(name.lastIndexOf(QLatin1String("_rels/")));
        QXmlStreamReader rels(it.value());
        while (!rels.atEnd()) {
            rels.readNext();
            if (!rels.isStartElement() || rels.name() != QLatin1String("Relationship")) continue;
            const QString target = rels.attributes().value(QLatin1String("Target")).toString();
            if (rels.attributes().value(QLatin1String("TargetMode")).toString() == QLatin1String("External")) {
                ++external;
                const QString scheme = QUrl(target).scheme();
                const bool address = scheme.size() > 1 && scheme != QLatin1String("file");
                if (!address && !target.startsWith(QLatin1Char('#')) && (QDir::isAbsolutePath(target) || target.contains(QLatin1String("..")) || target.contains(QLatin1Char(':')))) problems << name + QStringLiteral(": ") + target + QStringLiteral(" leaves the folder");
                continue;
            }
            const QString path = target.startsWith(QLatin1Char('/')) ? target.mid(1) : QDir::cleanPath(dir + target);
            if (path.startsWith(QLatin1String("..")) || !parts.contains(path)) problems << name + QStringLiteral(": ") + target + QStringLiteral(" is not in the package");
        }
        if (rels.hasError()) problems << name + QStringLiteral(": ") + rels.errorString();
    }
    if (externalLinks) *externalLinks = external;
    return problems.join(QLatin1Char('\n'));
}

} // namespace packedcheck
