#include "odshelpers.h"

#include <QFile>
#include <QTemporaryDir>

namespace odstest {

QString namespaces()
{
    return QStringLiteral(
        "xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\" "
        "xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" "
        "xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" "
        "xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\" "
        "xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" xmlns:of=\"urn:oasis:names:tc:opendocument:xmlns:of:1.2\" "
        "xmlns:config=\"urn:oasis:names:tc:opendocument:xmlns:config:1.0\" xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\" "
        "xmlns:calcext=\"urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0\" xmlns:tableooo=\"http://openoffice.org/2009/table\"");
}

QString table(const QString &name, const QString &inner, const QString &attrs)
{
    return QStringLiteral("<table:table table:name=\"%1\"%2>%3</table:table>").arg(name.toHtmlEscaped(), attrs.isEmpty() ? QString() : QLatin1Char(' ') + attrs, inner);
}

QString row(const QString &cells, const QString &attrs)
{
    return QStringLiteral("<table:table-row%1>%2</table:table-row>").arg(attrs.isEmpty() ? QString() : QLatin1Char(' ') + attrs, cells);
}

QString columns(int count, const QString &attrs)
{
    return QStringLiteral("<table:table-column table:number-columns-repeated=\"%1\"%2/>").arg(count).arg(attrs.isEmpty() ? QString() : QLatin1Char(' ') + attrs);
}

QString cell(const QString &attrs, const QString &inside)
{
    const QString a = attrs.isEmpty() ? QString() : QLatin1Char(' ') + attrs;
    return inside.isEmpty() ? QStringLiteral("<table:table-cell%1/>").arg(a) : QStringLiteral("<table:table-cell%1>%2</table:table-cell>").arg(a, inside);
}

QString floatCell(double v, const QString &attrs)
{
    return cell(QStringLiteral("office:value-type=\"float\" office:value=\"%1\"%2").arg(QString::number(v, 'g', 17), attrs.isEmpty() ? QString() : QLatin1Char(' ') + attrs),
                QStringLiteral("<text:p>%1</text:p>").arg(QString::number(v, 'g', 15)));
}

QString stringCell(const QString &text, const QString &attrs)
{
    return cell(QStringLiteral("office:value-type=\"string\"%1").arg(attrs.isEmpty() ? QString() : QLatin1Char(' ') + attrs), QStringLiteral("<text:p>%1</text:p>").arg(text.toHtmlEscaped()));
}

QString emptyCell(const QString &attrs) { return cell(attrs); }

QByteArray contentXml(const QString &tables, const QString &automaticStyles, const QString &before, const QString &fontFaces)
{
    return ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<office:document-content " + namespaces() + " office:version=\"1.3\">"
            + (fontFaces.isEmpty() ? QString() : "<office:font-face-decls>" + fontFaces + "</office:font-face-decls>") + "<office:automatic-styles>" + automaticStyles
            + "</office:automatic-styles><office:body><office:spreadsheet>" + before + tables + "</office:spreadsheet></office:body></office:document-content>")
        .toUtf8();
}

QByteArray settingsXml(const QMap<QString, QMap<QString, QString>> &sheets, const QString &active, const QMap<QString, QString> &view)
{
    auto item = [](const QString &name, const QString &type, const QString &value) {
        return QStringLiteral("<config:config-item config:name=\"%1\" config:type=\"%2\">%3</config:config-item>").arg(name, type, value.toHtmlEscaped());
    };
    QString tables;
    for (auto it = sheets.cbegin(); it != sheets.cend(); ++it) {
        QString items;
        for (auto i = it.value().cbegin(); i != it.value().cend(); ++i) items += item(i.key(), QStringLiteral("int"), i.value());
        tables += QStringLiteral("<config:config-item-map-entry config:name=\"%1\">%2</config:config-item-map-entry>").arg(it.key().toHtmlEscaped(), items);
    }
    QString viewItems = item(QStringLiteral("ActiveTable"), QStringLiteral("string"), active);
    for (auto i = view.cbegin(); i != view.cend(); ++i) viewItems += item(i.key(), QStringLiteral("boolean"), i.value());
    return ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<office:document-settings " + namespaces() + " office:version=\"1.3\"><office:settings>"
            + "<config:config-item-set config:name=\"ooo:view-settings\"><config:config-item-map-indexed config:name=\"Views\"><config:config-item-map-entry>" + viewItems
            + "<config:config-item-map-named config:name=\"Tables\">" + tables + "</config:config-item-map-named></config:config-item-map-entry></config:config-item-map-indexed></config:config-item-set>"
            + "</office:settings></office:document-settings>")
        .toUtf8();
}

QByteArray buildOds(const QByteArray &content, const QString &commonStyles, const QByteArray &settings, const QMap<QString, QByteArray> &extra, const QString &mimetype, const QByteArray &manifest)
{
    jp::ZipWriter zip;
    zip.add(QStringLiteral("mimetype"), mimetype.toUtf8(), false);
    zip.add(QStringLiteral("content.xml"), content, true);
    zip.add(QStringLiteral("styles.xml"), ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<office:document-styles " + namespaces() + " office:version=\"1.3\"><office:styles>" + commonStyles
                                           + "</office:styles></office:document-styles>").toUtf8(), true);
    if (!settings.isEmpty()) zip.add(QStringLiteral("settings.xml"), settings, true);
    zip.add(QStringLiteral("META-INF/manifest.xml"),
            manifest.isEmpty() ? QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.3\"/>")
                               : manifest,
            true);
    for (auto it = extra.cbegin(); it != extra.cend(); ++it) zip.add(it.key(), it.value(), true);
    return zip.finish();
}

QByteArray buildFods(const QString &tables, const QString &automaticStyles, const QString &commonStyles, const QString &before)
{
    return ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<office:document " + namespaces()
            + " office:version=\"1.3\" office:mimetype=\"application/vnd.oasis.opendocument.spreadsheet\"><office:styles>" + commonStyles + "</office:styles><office:automatic-styles>"
            + automaticStyles + "</office:automatic-styles><office:body><office:spreadsheet>" + before + tables + "</office:spreadsheet></office:body></office:document>")
        .toUtf8();
}

bool openBytes(const QByteArray &bytes, const QString &name, Workbook *out, QString *why, const Limits &limits)
{
    static QTemporaryDir dir;
    const QString path = dir.filePath(name);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    f.write(bytes);
    f.close();
    return out->openFile(path, why, limits);
}

QString shown(const Workbook &wb, const Sheet &s, const QString &ref) { return show(s, wb.sharedStrings(), ref); }

QString formulaOf(const Sheet &s, const QString &ref)
{
    const Cell *c = at(s, ref);
    return c ? s.formulaText(*c) : QString();
}

} // namespace odstest
