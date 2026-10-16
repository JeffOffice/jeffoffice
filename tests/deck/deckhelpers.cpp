// The helpers declared in deckhelpers.h.
#include "deckhelpers.h"

namespace odptest {

const QString kNs = QStringLiteral(
    " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""
    " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\""
    " xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\""
    " xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\""
    " xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\" xmlns:smil=\"urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0\""
    " xmlns:anim=\"urn:oasis:names:tc:opendocument:xmlns:animation:1.0\" xmlns:drawooo=\"http://openoffice.org/2010/draw\""
    " xmlns:loext=\"urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\""
    " xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\" xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\" office:version=\"1.3\"");

const QString kBullets = QStringLiteral(
    "<text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"●\"><style:list-level-properties text:space-before=\"0.3cm\" text:min-label-width=\"0.9cm\"/>"
    "<style:text-properties fo:font-family=\"OpenSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/></text:list-level-style-bullet>"
    "<text:list-level-style-bullet text:level=\"2\" text:bullet-char=\"–\"><style:list-level-properties text:space-before=\"1.5cm\" text:min-label-width=\"0.9cm\"/>"
    "<style:text-properties fo:font-family=\"OpenSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"75%\"/></text:list-level-style-bullet>");

QString odpStyles()
{
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-styles") + kNs + QStringLiteral(
        "><office:font-face-decls><style:font-face style:name=\"Liberation Sans\" svg:font-family=\"'Liberation Sans'\"/>"
        "<style:font-face style:name=\"Trebuchet MS1\" svg:font-family=\"'Trebuchet MS'\"/></office:font-face-decls>"
        "<office:styles>"
        "<style:default-style style:family=\"graphic\"><style:graphic-properties svg:stroke-color=\"#3465a4\" draw:fill-color=\"#729fcf\"/>"
        "<style:text-properties style:font-name=\"Liberation Sans\" fo:font-size=\"24pt\"/></style:default-style>"
        "<style:style style:name=\"standard\" style:family=\"graphic\"><style:graphic-properties draw:stroke=\"solid\" svg:stroke-width=\"0cm\" svg:stroke-color=\"#3465a4\""
        " draw:fill=\"solid\" draw:fill-color=\"#729fcf\" fo:padding-top=\"0.125cm\" fo:padding-bottom=\"0.125cm\" fo:padding-left=\"0.25cm\" fo:padding-right=\"0.25cm\"/>"
        "<style:text-properties fo:font-size=\"18pt\"/></style:style>"
        "<style:style style:name=\"Default-title\" style:family=\"presentation\"><style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\" draw:textarea-vertical-align=\"middle\"/>"
        "<style:paragraph-properties fo:text-align=\"center\"/><style:text-properties style:font-name=\"Trebuchet MS1\" fo:font-size=\"44pt\" fo:font-weight=\"bold\" fo:color=\"#1f3864\"/></style:style>"
        "<style:style style:name=\"Default-subtitle\" style:family=\"presentation\"><style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\" draw:textarea-vertical-align=\"middle\"/>"
        "<style:paragraph-properties fo:text-align=\"center\"/><style:text-properties fo:font-size=\"32pt\"/></style:style>"
        "<style:style style:name=\"Default-outline1\" style:family=\"presentation\"><style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\" draw:auto-grow-height=\"false\" style:shrink-to-fit=\"true\">"
        "<text:list-style style:name=\"Default-outline1\">") + kBullets + QStringLiteral("</text:list-style></style:graphic-properties>"
        "<style:paragraph-properties fo:margin-top=\"0.5cm\" fo:margin-bottom=\"0cm\"/><style:text-properties fo:font-size=\"32pt\"/></style:style>"
        "<style:style style:name=\"Default-outline2\" style:family=\"presentation\" style:parent-style-name=\"Default-outline1\"><style:paragraph-properties fo:margin-top=\"0.4cm\"/>"
        "<style:text-properties fo:font-size=\"28pt\"/></style:style>"
        "<style:style style:name=\"Default-notes\" style:family=\"presentation\"><style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\"/><style:text-properties fo:font-size=\"20pt\"/></style:style>"
        "<draw:marker draw:name=\"Arrow\" svg:viewBox=\"0 0 20 30\" svg:d=\"M10 0l-10 30h20z\"/>"
        "<draw:fill-image draw:name=\"Tile\" xlink:href=\"Pictures/a.png\" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\"/>"
        "</office:styles><office:automatic-styles>"
        "<style:page-layout style:name=\"PM1\"><style:page-layout-properties fo:page-width=\"28cm\" fo:page-height=\"15.75cm\" style:print-orientation=\"landscape\"/></style:page-layout>"
        "<style:page-layout style:name=\"PM0\"><style:page-layout-properties fo:page-width=\"21.59cm\" fo:page-height=\"27.94cm\" style:print-orientation=\"portrait\"/></style:page-layout>"
        "<style:style style:name=\"Mdp1\" style:family=\"drawing-page\"><style:drawing-page-properties draw:background-size=\"border\" draw:fill=\"solid\" draw:fill-color=\"#fff2cc\"/></style:style>"
        "<style:style style:name=\"Mgr1\" style:family=\"graphic\" style:parent-style-name=\"standard\"><style:graphic-properties draw:stroke=\"none\" draw:fill=\"solid\" draw:fill-color=\"#c55a11\"/></style:style>"
        "<style:style style:name=\"Mpr1\" style:family=\"presentation\" style:parent-style-name=\"Default-title\"><style:graphic-properties fo:min-height=\"2.629cm\"/></style:style>"
        "</office:automatic-styles><office:master-styles>"
        "<style:master-page style:name=\"Default\" style:display-name=\"Warm\" style:page-layout-name=\"PM1\" draw:style-name=\"Mdp1\">"
        "<draw:custom-shape draw:style-name=\"Mgr1\" draw:layer=\"backgroundobjects\" svg:width=\"28cm\" svg:height=\"0.5cm\" svg:x=\"0cm\" svg:y=\"15.25cm\"><text:p/>"
        "<draw:enhanced-geometry svg:viewBox=\"0 0 21600 21600\" draw:type=\"rectangle\" draw:enhanced-path=\"M 0 0 L 21600 0 21600 21600 0 21600 0 0 Z N\"/></draw:custom-shape>"
        "<draw:frame presentation:style-name=\"Default-title\" draw:layer=\"backgroundobjects\" svg:width=\"25.199cm\" svg:height=\"2.629cm\" svg:x=\"1.4cm\" svg:y=\"0.628cm\""
        " presentation:class=\"title\" presentation:placeholder=\"true\"><draw:text-box/></draw:frame>"
        "<draw:frame presentation:style-name=\"Default-outline1\" draw:layer=\"backgroundobjects\" svg:width=\"25.199cm\" svg:height=\"9.134cm\" svg:x=\"1.4cm\" svg:y=\"3.685cm\""
        " presentation:class=\"outline\" presentation:placeholder=\"true\"><draw:text-box/></draw:frame>"
        "<draw:frame presentation:style-name=\"Default-title\" draw:layer=\"backgroundobjects\" svg:width=\"6.5cm\" svg:height=\"1.085cm\" svg:x=\"1.4cm\" svg:y=\"14.348cm\""
        " presentation:class=\"date-time\"><draw:text-box><text:p><presentation:date-time/></text:p></draw:text-box></draw:frame>"
        "<draw:frame presentation:style-name=\"Default-title\" draw:layer=\"backgroundobjects\" svg:width=\"8.8cm\" svg:height=\"1.085cm\" svg:x=\"9.5cm\" svg:y=\"14.348cm\""
        " presentation:class=\"footer\"><draw:text-box><text:p><presentation:footer/></text:p></draw:text-box></draw:frame>"
        "<draw:frame presentation:style-name=\"Default-title\" draw:layer=\"backgroundobjects\" svg:width=\"6.5cm\" svg:height=\"1.085cm\" svg:x=\"20cm\" svg:y=\"14.348cm\""
        " presentation:class=\"page-number\"><draw:text-box><text:p><text:page-number>&lt;number&gt;</text:page-number></text:p></draw:text-box></draw:frame>"
        "<presentation:notes style:page-layout-name=\"PM0\"><draw:frame presentation:style-name=\"Default-notes\" svg:width=\"17.271cm\" svg:height=\"12.572cm\" svg:x=\"2.159cm\""
        " svg:y=\"13.271cm\" presentation:class=\"notes\" presentation:placeholder=\"true\"><draw:text-box/></draw:frame></presentation:notes></style:master-page>"
        "</office:master-styles></office:document-styles>");
}

QByteArray png(int w, int h, const QColor &color)
{
    QImage img(w, h, QImage::Format_RGB32);
    img.fill(color);
    img.setDotsPerMeterX(3780);   // 96 to the inch
    img.setDotsPerMeterY(3780);
    QByteArray bytes;
    QBuffer b(&bytes);
    b.open(QIODevice::WriteOnly);
    img.save(&b, "PNG");
    return bytes;
}

QByteArray odpZip(const QString &content, const QString &styles, const QMap<QString, QByteArray> &extra,
                  const QByteArray &mime, const QString &manifest, bool mimeFirst)
{
    ZipWriter z;
    if (mimeFirst) z.add(QStringLiteral("mimetype"), mime, false);
    z.add(QStringLiteral("META-INF/manifest.xml"),
          (manifest.isEmpty() ? QStringLiteral("<?xml version=\"1.0\"?><manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\"><manifest:file-entry manifest:full-path=\"/\" manifest:media-type=\"") + QString::fromLatin1(mime) + QStringLiteral("\"/></manifest:manifest>") : manifest).toUtf8(), true);
    z.add(QStringLiteral("meta.xml"), QStringLiteral("<?xml version=\"1.0\"?><office:document-meta") .toUtf8() + kNs.toUtf8() + QByteArray("><office:meta><dc:title>Quarterly</dc:title><meta:initial-creator>Pat</meta:initial-creator></office:meta></office:document-meta>"), true);
    z.add(QStringLiteral("styles.xml"), styles.toUtf8(), true);
    z.add(QStringLiteral("content.xml"), content.toUtf8(), true);
    for (auto it = extra.cbegin(); it != extra.cend(); ++it) z.add(it.key(), it.value(), true);
    if (!mimeFirst) z.add(QStringLiteral("mimetype"), mime, true);
    return z.finish();
}

QString odpContent(const QString &autoStyles, const QString &pages, const QString &decls)
{
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-content") + kNs + QStringLiteral("><office:font-face-decls>"
        "<style:font-face style:name=\"Liberation Sans\" svg:font-family=\"'Liberation Sans'\"/></office:font-face-decls><office:automatic-styles>")
        + autoStyles + QStringLiteral("</office:automatic-styles><office:body><office:presentation>") + decls + pages
        + QStringLiteral("</office:presentation></office:body></office:document-content>");
}

QString sampleAutomaticStyles()
{
    return QStringLiteral(
        "<style:style style:name=\"dp1\" style:family=\"drawing-page\"><style:drawing-page-properties presentation:background-visible=\"true\" presentation:background-objects-visible=\"true\""
        " presentation:display-footer=\"false\" presentation:display-page-number=\"false\" presentation:display-date-time=\"false\"/></style:style>"
        "<style:style style:name=\"dp2\" style:family=\"drawing-page\"><style:drawing-page-properties presentation:display-header=\"true\" presentation:display-footer=\"true\"/></style:style>"
        "<style:style style:name=\"dp3\" style:family=\"drawing-page\"><style:drawing-page-properties presentation:transition-type=\"automatic\" presentation:duration=\"PT00H00M05S\""
        " presentation:transition-speed=\"slow\" smil:type=\"pushWipe\" smil:subtype=\"fromLeft\" presentation:display-footer=\"true\" presentation:display-page-number=\"true\""
        " presentation:display-date-time=\"false\"/></style:style>"
        "<style:style style:name=\"dp4\" style:family=\"drawing-page\"><style:drawing-page-properties presentation:visibility=\"hidden\" presentation:background-objects-visible=\"false\""
        " draw:fill=\"solid\" draw:fill-color=\"#112233\"/></style:style>"
        "<style:style style:name=\"pr1\" style:family=\"presentation\" style:parent-style-name=\"Default-title\"><style:graphic-properties fo:min-height=\"2.629cm\"/></style:style>"
        "<style:style style:name=\"pr2\" style:family=\"presentation\" style:parent-style-name=\"Default-subtitle\"/>"
        "<style:style style:name=\"pr3\" style:family=\"presentation\" style:parent-style-name=\"Default-notes\"/>"
        "<style:style style:name=\"pr4\" style:family=\"presentation\" style:parent-style-name=\"Default-outline1\"/>"
        "<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties fo:font-style=\"italic\" style:text-underline-style=\"solid\" style:text-underline-width=\"auto\" fo:color=\"#ff0000\"/></style:style>"
        "<style:style style:name=\"T2\" style:family=\"text\"><style:text-properties fo:font-size=\"150%\" style:text-position=\"super 58%\"/></style:style>"
        "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties fo:text-align=\"end\"/></style:style>"
        "<text:list-style style:name=\"L1\">") + kBullets + QStringLiteral("</text:list-style>"
        "<text:list-style style:name=\"L2\"><text:list-level-style-number text:level=\"1\" style:num-format=\"1\" style:num-suffix=\".\" text:start-value=\"3\">"
        "<style:list-level-properties text:space-before=\"0cm\" text:min-label-width=\"0.8cm\"/></text:list-level-style-number></text:list-style>"
        "<style:style style:name=\"gr1\" style:family=\"graphic\" style:parent-style-name=\"standard\"><style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\""
        " fo:clip=\"rect(7.5pt, 15pt, 3.75pt, 30pt)\" style:mirror=\"horizontal\"/></style:style>"
        "<style:style style:name=\"gr2\" style:family=\"graphic\" style:parent-style-name=\"standard\"><style:graphic-properties draw:stroke=\"none\" draw:fill=\"solid\""
        " draw:fill-color=\"#3366cc\" draw:opacity=\"50%\"/></style:style>"
        "<style:style style:name=\"gr3\" style:family=\"graphic\" style:parent-style-name=\"standard\"><style:graphic-properties draw:stroke=\"solid\" svg:stroke-width=\"0.1cm\""
        " svg:stroke-color=\"#003300\" draw:marker-end=\"Arrow\" draw:marker-end-width=\"0.3cm\" draw:fill=\"none\"/></style:style>"
        "<style:style style:name=\"gr4\" style:family=\"graphic\" style:parent-style-name=\"standard\"><style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\""
        " draw:auto-grow-height=\"true\" fo:min-height=\"1cm\"/></style:style>"
        "<style:style style:name=\"gr5\" style:family=\"graphic\" style:parent-style-name=\"standard\"><style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\"/></style:style>"
        "<style:style style:name=\"gr6\" style:family=\"graphic\" style:parent-style-name=\"standard\"><style:graphic-properties draw:stroke=\"none\" draw:fill=\"bitmap\""
        " draw:fill-image-name=\"Tile\"/></style:style>"
        "<style:style style:name=\"co1\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"3cm\"/></style:style>"
        "<style:style style:name=\"co2\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"4cm\"/></style:style>"
        "<style:style style:name=\"ro1\" style:family=\"table-row\"><style:table-row-properties style:row-height=\"1cm\"/></style:style>"
        "<style:style style:name=\"ce1\" style:family=\"table-cell\"><loext:graphic-properties draw:fill=\"solid\" draw:fill-color=\"#f9f9f9\" draw:textarea-vertical-align=\"middle\""
        " fo:padding=\"0.1cm\"/><style:paragraph-properties fo:border=\"0.5pt solid #ff0000\"/><style:text-properties fo:font-size=\"8pt\"/></style:style>");
}

QString samplePages()
{
    // The frame of a placeholder: class, style, and place.
    auto frame = [](const char *cls, const char *style, const char *place, const QString &body) {
        return QStringLiteral("<draw:frame presentation:style-name=\"%1\" draw:layer=\"layout\" %2 presentation:class=\"%3\"><draw:text-box>%4</draw:text-box></draw:frame>")
            .arg(QLatin1String(style), QLatin1String(place), QLatin1String(cls), body);
    };
    const QString notes = QStringLiteral("<presentation:notes draw:style-name=\"dp2\"><draw:page-thumbnail draw:layer=\"layout\" svg:width=\"18.6cm\" svg:height=\"10.5cm\" svg:x=\"1.5cm\" svg:y=\"2cm\""
                                         " draw:page-number=\"1\" presentation:class=\"page\"/><draw:frame presentation:style-name=\"pr3\" draw:layer=\"layout\" svg:width=\"17.2cm\" svg:height=\"12.5cm\""
                                         " svg:x=\"2.1cm\" svg:y=\"13.2cm\" presentation:class=\"notes\"><draw:text-box><text:p>Say hello first</text:p><text:p>Then the numbers</text:p></draw:text-box></draw:frame></presentation:notes>");
    QString pages;
    // 1: a title and a subtitle, notes, and an animation (which is left out).
    pages += QStringLiteral("<draw:page draw:name=\"page1\" draw:style-name=\"dp1\" draw:master-page-name=\"Default\" presentation:presentation-page-layout-name=\"AL1T0\">")
        + frame("title", "pr1", "svg:width=\"25.199cm\" svg:height=\"2.629cm\" svg:x=\"1.4cm\" svg:y=\"0.628cm\"", QStringLiteral("<text:p>Quarterly <text:span text:style-name=\"T1\">results</text:span></text:p>"))
        + frame("subtitle", "pr2", "svg:width=\"25.199cm\" svg:height=\"9.134cm\" svg:x=\"1.4cm\" svg:y=\"3.685cm\"", QStringLiteral("<text:p>Prepared by the team</text:p>"))
        + notes + QStringLiteral("<anim:par smil:begin=\"next\"/></draw:page>");
    // 2: a title, an outline with two levels, a numbered text box, one with a link, and a picture.
    pages += QStringLiteral("<draw:page draw:name=\"page2\" draw:style-name=\"dp1\" draw:master-page-name=\"Default\" presentation:presentation-page-layout-name=\"AL2T1\">")
        + frame("title", "pr1", "svg:width=\"25.199cm\" svg:height=\"2.629cm\" svg:x=\"1.4cm\" svg:y=\"0.628cm\"", QStringLiteral("<text:p>Agenda</text:p>"))
        + frame("outline", "pr4", "svg:width=\"25.199cm\" svg:height=\"9.134cm\" svg:x=\"1.4cm\" svg:y=\"3.685cm\"",
                QStringLiteral("<text:list text:style-name=\"L1\"><text:list-item><text:p>First point</text:p></text:list-item><text:list-item><text:list><text:list-item>"
                               "<text:p text:style-name=\"P1\">Second <text:span text:style-name=\"T2\">point</text:span></text:p></text:list-item></text:list></text:list-item></text:list>"
                               "<text:p>Plain third</text:p>"))
        + QStringLiteral("<draw:frame draw:style-name=\"gr4\" draw:layer=\"layout\" svg:width=\"6cm\" svg:height=\"2cm\" svg:x=\"2cm\" svg:y=\"13cm\"><draw:text-box>"
                         "<text:list text:style-name=\"L2\"><text:list-item><text:p>Three</text:p></text:list-item><text:list-item><text:p>Four</text:p></text:list-item></text:list></draw:text-box></draw:frame>")
        + QStringLiteral("<draw:frame draw:style-name=\"gr4\" draw:layer=\"layout\" svg:width=\"8cm\" svg:height=\"1cm\" svg:x=\"12cm\" svg:y=\"13cm\"><draw:text-box>"
                         "<text:p>  Visit   <text:a xlink:href=\"https://example.com/a?b=1&amp;c=2\" xlink:type=\"simple\">the site</text:a><text:s/>now <text:a xlink:href=\"file:///bin/sh\">run</text:a></text:p></draw:text-box></draw:frame>")
        + QStringLiteral("<draw:frame draw:style-name=\"gr1\" draw:layer=\"layout\" svg:width=\"3.5278cm\" svg:height=\"1.7639cm\" svg:x=\"2cm\" svg:y=\"10cm\">"
                         "<draw:image xlink:href=\"Pictures/a.png\" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\"><text:p/></draw:image></draw:frame>")
        + QStringLiteral("</draw:page>");
    // 3: shapes, a line, a group, a table, a turned rectangle; a transition; the footer and the number.
    pages += QStringLiteral("<draw:page draw:name=\"page3\" draw:style-name=\"dp3\" draw:master-page-name=\"Default\" presentation:use-footer-name=\"ftr1\">")
        + QStringLiteral("<draw:custom-shape draw:style-name=\"gr2\" draw:layer=\"layout\" svg:width=\"4cm\" svg:height=\"2cm\" svg:x=\"1cm\" svg:y=\"1cm\"><text:p/>"
                         "<draw:enhanced-geometry svg:viewBox=\"0 0 21600 21600\" draw:type=\"right-arrow\"/></draw:custom-shape>")
        + QStringLiteral("<draw:custom-shape draw:style-name=\"gr2\" draw:layer=\"layout\" svg:width=\"3cm\" svg:height=\"3cm\" svg:x=\"6cm\" svg:y=\"1cm\"><text:p>Round</text:p>"
                         "<draw:enhanced-geometry svg:viewBox=\"0 0 0 0\" draw:type=\"ooxml-ellipse\"/></draw:custom-shape>")
        + QStringLiteral("<draw:custom-shape draw:style-name=\"gr2\" draw:layer=\"layout\" svg:width=\"5cm\" svg:height=\"2.5cm\" svg:x=\"1cm\" svg:y=\"4cm\"><text:p/>"
                         "<draw:enhanced-geometry svg:viewBox=\"0 0 0 0\" drawooo:sub-view-size=\"100 50\" draw:type=\"ooxml-non-primitive\""
                         " draw:enhanced-path=\"M 0 0 L 100 0 50 50 0 50 Z N\"/></draw:custom-shape>")
        + QStringLiteral("<draw:line draw:style-name=\"gr3\" draw:layer=\"layout\" svg:x1=\"1cm\" svg:y1=\"8cm\" svg:x2=\"9cm\" svg:y2=\"9cm\"><text:p/></draw:line>")
        + QStringLiteral("<draw:g><draw:rect draw:style-name=\"gr2\" svg:width=\"2cm\" svg:height=\"1cm\" svg:x=\"2cm\" svg:y=\"5cm\"/>"
                         "<draw:ellipse draw:style-name=\"gr2\" svg:width=\"2cm\" svg:height=\"2cm\" svg:x=\"5cm\" svg:y=\"6cm\"/></draw:g>")
        + QStringLiteral("<draw:frame draw:style-name=\"gr5\" draw:layer=\"layout\" svg:width=\"7cm\" svg:height=\"2cm\" svg:x=\"14cm\" svg:y=\"2cm\"><table:table>"
                         "<table:table-column table:style-name=\"co1\"/><table:table-column table:style-name=\"co2\"/>"
                         "<table:table-row table:style-name=\"ro1\" table:default-cell-style-name=\"ce1\"><table:table-cell table:number-columns-spanned=\"2\"><text:p>Header</text:p></table:table-cell>"
                         "<table:covered-table-cell/></table:table-row>"
                         "<table:table-row table:style-name=\"ro1\" table:default-cell-style-name=\"ce1\"><table:table-cell><text:p>a</text:p></table:table-cell>"
                         "<table:table-cell><text:p>b</text:p></table:table-cell></table:table-row></table:table></draw:frame>")
        + QStringLiteral("<draw:rect draw:style-name=\"gr2\" svg:width=\"4cm\" svg:height=\"2cm\" draw:transform=\"rotate (1.5707963267949) translate (10cm 5cm)\"/>")
        + QStringLiteral("<draw:frame draw:style-name=\"gr5\" draw:layer=\"layout\" svg:width=\"4cm\" svg:height=\"3cm\" svg:x=\"10cm\" svg:y=\"10cm\"><draw:object xlink:href=\"./Object 1\"/>"
                         "<draw:image xlink:href=\"./ObjectReplacements/Object 1\"/><svg:desc>A chart of sales</svg:desc></draw:frame>")
        + QStringLiteral("<draw:frame draw:style-name=\"gr5\" draw:layer=\"layout\" svg:width=\"8cm\" svg:height=\"5cm\" svg:x=\"15cm\" svg:y=\"7cm\"><draw:object xlink:href=\"./Object 2\"/>"
                         "<draw:image xlink:href=\"./ObjectReplacements/Object 2\"/></draw:frame>")
        + QStringLiteral("<draw:rect draw:style-name=\"gr6\" svg:width=\"2cm\" svg:height=\"2cm\" svg:x=\"10cm\" svg:y=\"14cm\"/>")
        + QStringLiteral("</draw:page>");
    // 4: a hidden slide on a background of its own.
    pages += QStringLiteral("<draw:page draw:name=\"page4\" draw:style-name=\"dp4\" draw:master-page-name=\"Default\"/>");
    return pages;
}

QString sampleContent()
{
    return odpContent(sampleAutomaticStyles(), samplePages(), QStringLiteral("<presentation:footer-decl presentation:name=\"ftr1\">Confidential</presentation:footer-decl>"));
}

QList<QDomElement> byName(const QDomNode &from, const char *qualified)
{
    static const QHash<QString, QString> spaces = {
        {QStringLiteral("office"), QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:office:1.0")}, {QStringLiteral("style"), QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:style:1.0")},
        {QStringLiteral("text"), QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:text:1.0")}, {QStringLiteral("draw"), QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0")},
        {QStringLiteral("presentation"), QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:presentation:1.0")}, {QStringLiteral("anim"), QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:animation:1.0")},
        {QStringLiteral("smil"), QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0")}, {QStringLiteral("officeooo"), QStringLiteral("http://openoffice.org/2009/office")},
        {QStringLiteral("dc"), QStringLiteral("http://purl.org/dc/elements/1.1/")}, {QStringLiteral("meta"), QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:meta:1.0")},
        {QStringLiteral("svg"), QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0")}, {QStringLiteral("table"), QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:table:1.0")}};
    const QString q = QString::fromLatin1(qualified);
    const QString uri = spaces.value(q.section(QLatin1Char(':'), 0, 0)), local = q.section(QLatin1Char(':'), 1);
    const QDomNodeList list = from.isDocument() ? from.toDocument().elementsByTagNameNS(uri, local) : from.toElement().elementsByTagNameNS(uri, local);
    QList<QDomElement> out;
    for (int i = 0; i < list.size(); ++i) out << list.item(i).toElement();
    return out;
}

} // namespace odptest

namespace deckhelp {

void fillSlides(DeckWindow &w, const QStringList &titles, const QList<QStringList> &bodies)
{
    Editor *ed = w.editor();
    for (int i = 1; i < titles.size(); ++i) w.newSlide();
    Document *d = ed->doc();
    auto type = [](const QString &text, QTextDocument *story) {
        QTextCursor c(story);
        const QStringList lines = text.split(QLatin1Char('\n'));
        for (int k = 0; k < lines.size(); ++k) {
            if (k) c.insertBlock();
            c.insertText(lines[k]);
        }
    };
    for (int i = 0; i < titles.size(); ++i)
        for (const auto &it : d->pages[i]->items) {
            const QString kind = deck::placeholderType(*it);
            QTextDocument *story = d->storyDoc(static_cast<TextItem *>(it.get())->storyId);
            if (kind == QLatin1String("title") || kind == QLatin1String("ctrTitle")) type(titles[i], story);
            else if ((kind == QLatin1String("obj") || kind == QLatin1String("subTitle")) && !bodies.value(i).isEmpty()) type(bodies[i].join(QLatin1Char('\n')), story);
        }
    ed->setCurrentPage(0);
    ed->notifyLive();
    ed->undoStack()->clear();
}

QString titleOfSlide(const Document &d, int i)
{
    for (const auto &it : d.pages[i]->items) {
        const QString kind = deck::placeholderType(*it);
        if (kind == QLatin1String("title") || kind == QLatin1String("ctrTitle")) return d.storyDoc(static_cast<const TextItem *>(it.get())->storyId)->toPlainText();
    }
    return QString();
}

QStringList titlesOf(const Document &d)
{
    QStringList out;
    for (int i = 0; i < d.pages.size(); ++i) out << titleOfSlide(d, i);
    return out;
}

QTextDocument *bodyOfSlide(const Document &d, int i)
{
    for (const auto &it : d.pages[i]->items) {
        const QString kind = deck::placeholderType(*it);
        if (kind == QLatin1String("obj") || kind == QLatin1String("body") || kind == QLatin1String("subTitle")) return d.storyDoc(static_cast<const TextItem *>(it.get())->storyId);
    }
    return nullptr;
}

QStringList sectionNames(const Document &d)
{
    QStringList out;
    for (const deck::Section &s : deck::sectionsOf(d)) out << s.name;
    return out;
}

QList<int> sectionCounts(const Document &d)
{
    QList<int> out;
    for (const deck::Section &s : deck::sectionsOf(d)) out << s.count;
    return out;
}

std::unique_ptr<Document> lettered(int n)
{
    auto d = deck::newPresentation();
    const MasterPage *layout = deck::layoutOfType(*d, QStringLiteral("obj"));
    while (d->pages.size() < n) deck::addSlide(*d, *layout);
    for (int i = 0; i < n; ++i) d->pages[i]->title = QString(QChar('A' + i));
    return d;
}

QString letters(const Document &d)
{
    QString out;
    for (const auto &p : d.pages) out += p->title;
    return out;
}

void untilDone(const std::function<bool()> &poll, int tries)
{
    if (poll() || tries <= 0) return;
    QTimer::singleShot(10, [poll, tries] { untilDone(poll, tries - 1); });
}

void shot(QWidget *w, const QString &name)
{
    if (qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) return;
    QDir().mkpath(qEnvironmentVariable("JP_SHOT_DIR"));
    w->grab().save(qEnvironmentVariable("JP_SHOT_DIR") + QLatin1Char('/') + name + QStringLiteral(".png"));
}

void rightClick(QWidget *viewport, const QPoint &pos, const std::function<void(QMenu *)> &inspect)
{
    QTimer::singleShot(0, [inspect] {
        untilDone([inspect] {
            auto *menu = qobject_cast<QMenu *>(QApplication::activePopupWidget());
            if (!menu) return false;
            inspect(menu);
            menu->close();
            return true;
        });
    });
    QContextMenuEvent e(QContextMenuEvent::Mouse, pos, viewport->mapToGlobal(pos));
    QApplication::sendEvent(viewport, &e);
}

QByteArray le(quint64 v, int n)
{
    QByteArray b;
    for (int i = 0; i < n; ++i) b.append(char((v >> (8 * i)) & 0xFF));
    return b;
}

QByteArray pptRec(quint16 type, const QByteArray &data, quint16 inst, quint8 ver)
{
    return le(quint16(ver | (inst << 4)), 2) + le(type, 2) + le(quint32(data.size()), 4) + data;
}

QByteArray pptBox(quint16 type, const QByteArray &kids, quint16 inst) { return pptRec(type, kids, inst, 0xF); }

QByteArray utf16(const QString &s)
{
    QByteArray b;
    for (const QChar c : s) b += le(c.unicode(), 2);
    return b;
}

QByteArray raw(std::initializer_list<int> values)
{
    QByteArray b;
    for (const int v : values) b.append(char(v));
    return b;
}

QByteArray pptColor(int r, int g, int b, int index) { return raw({r, g, b, index}); }

QByteArray fontEntity(const QString &name)
{
    QByteArray face = utf16(name);
    face.append(QByteArray(64 - face.size(), '\0'));
    return pptRec(4023, face + QByteArray(4, '\0'));
}

QString pptDocumentStream() { return QStringLiteral("Power") + QStringLiteral("Point Document"); }

QByteArray pptText(int type, const QString &s, const QByteArray &styles, bool wide)
{
    QByteArray out = pptRec(3999, le(type, 4));
    out += wide ? pptRec(4000, utf16(s)) : pptRec(4008, s.toLatin1());
    if (!styles.isEmpty()) out += pptRec(4001, styles);
    return out;
}

QByteArray pfRun(int count, int level, quint32 masks, const QByteArray &data) { return le(count, 4) + le(level, 2) + le(masks, 4) + data; }

QByteArray cfRun(int count, quint32 masks, const QByteArray &data) { return le(count, 4) + le(masks, 4) + data; }

QByteArray slidePersist(quint32 persistId, int texts, quint32 slideId) { return pptRec(1011, le(persistId, 4) + le(4, 4) + le(texts, 4) + le(slideId, 4) + le(0, 4)); }

QByteArray slideAtom(int geom, const QByteArray &placeholders, quint32 master, quint32 notes, int flags)
{
    QByteArray ph = placeholders;
    ph.append(QByteArray(8 - ph.size(), '\0'));
    return pptRec(1007, le(geom, 4) + ph + le(master, 4) + le(notes, 4) + le(flags, 2) + le(0, 2), 0, 2);
}

QByteArray colorScheme(int instance)
{
    // background, text, shadow, title text, fills, accent, accent and hyperlink, accent and followed hyperlink
    return pptRec(2032, pptColor(0xFA, 0xFA, 0xFA, 0) + pptColor(0x11, 0x22, 0x33, 0) + pptColor(0xDD, 0xDD, 0xCC, 0) + pptColor(0x44, 0x55, 0x66, 0)
                             + pptColor(0x77, 0x88, 0x99, 0) + pptColor(0xAA, 0xBB, 0xCC, 0) + pptColor(1, 2, 3, 0) + pptColor(4, 5, 6, 0), instance);
}

QByteArray pptDrawing(const QByteArray &shapes)
{
    const QByteArray group = pptBox(0xF004, pptRec(0xF009, QByteArray(16, '\0'), 0, 1) + pptRec(0xF00A, le(0x400, 4) + le(5, 4), 0, 2));
    return pptBox(1036, pptBox(0xF002, pptRec(0xF008, le(2, 4) + le(0x402, 4)) + pptBox(0xF003, group + shapes)));
}

QByteArray pptShape(quint32 id, int placeholder, const QByteArray &textbox, int top, int left, int right, int bottom, const QByteArray &props, quint32 flags, quint16 type,
                           const QByteArray &clientExtra)
{
    const QByteArray anchor = le(top, 2) + le(left, 2) + le(right, 2) + le(bottom, 2);
    const QByteArray data = pptBox(0xF011, pptRec(3011, le(0xFFFFFFFF, 4) + raw({placeholder, 0, 0, 0})) + clientExtra);
    return pptBox(0xF004, pptRec(0xF00A, le(id, 4) + le(flags, 4), type, 2) + props + pptRec(0xF010, anchor) + data + (textbox.isEmpty() ? QByteArray() : pptBox(0xF00D, textbox)));
}

QByteArray outlineRef(int index) { return pptRec(3998, le(index, 4)); }

QByteArray userEdit(quint32 lastEdit, quint32 directory, quint32 document, int extra)
{
    QByteArray d = le(0x100, 4) + le(0, 2) + raw({0, 3}) + le(lastEdit, 4) + le(directory, 4) + le(document, 4) + le(0x20, 4) + le(1, 2) + le(0, 2);
    if (extra) d += le(extra, 4);
    return pptRec(4085, d);
}

QByteArray persistDirectory(const QVector<QPair<quint32, quint32>> &entries)
{
    QByteArray d;
    for (const auto &e : entries) d += le(e.first | (1u << 20), 4) + le(e.second, 4);
    return pptRec(6002, d);
}

QByteArray buildPpt(bool titleMaster, bool cycle, bool links, bool notesByAtom, const QByteArray &bodyGroup,
                           const QByteArray &drawingGroup, const QByteArray &firstSlideShapes, const QByteArray &masterTitleProps, const QByteArray &masterBackground,
                           const QByteArray &masterExtra, quint16 titleMasterRecord, const QByteArray &documentExtra,
                           const QByteArray &firstSlideExtra, const QByteArray &titleMasterShapes)
{
    constexpr quint32 kMaster = 0x80000001, kTitleMaster = 0x80000002;
    const QByteArray fonts = pptBox(2005, fontEntity(QStringLiteral("Alpha Sans")) + fontEntity(QStringLiteral("Beta Serif")) + fontEntity(QStringLiteral("Symbol")) + fontEntity(QStringLiteral("Wingdings 2")) + fontEntity(QStringLiteral("Wingdings")));
    // The default body text: bullets, 28 and 24 points, the first font, the text color.
    const QByteArray bodyStyle = le(2, 2)
        + le(0x593, 4) + le(3, 2) + le(0x2022, 2) + le(0, 2) + le(360, 2) + le(0, 2)
        + le(0x70000, 4) + le(0, 2) + le(28, 2) + pptColor(0, 0, 0, 1)
        + le(0x581, 4) + le(1, 2) + le(0x2013, 2) + le(720, 2) + le(360, 2)
        + le(0x20000, 4) + le(24, 2);
    const QByteArray environment = pptBox(1010, fonts + pptRec(4003, bodyStyle, 1));
    const QByteArray document = pptBox(1000, pptRec(1001, le(5760, 4) + le(4320, 4) + le(4320, 4) + le(5760, 4) + le(5, 4) + le(10, 4) + le(0, 4) + le(0, 4) + le(1, 2) + le(0, 2) + le(0, 4))
        + environment
        + drawingGroup
        + documentExtra
        + (links ? pptBox(1033, pptRec(1034, le(1, 4)) + pptBox(4055, pptRec(4051, le(7, 4)) + pptRec(4026, utf16(QStringLiteral("the site")), 0) + pptRec(4026, utf16(QStringLiteral("https://example.org/site")), 1)))
                 : QByteArray())
        + pptBox(4080, slidePersist(3, 0, kMaster) + (titleMaster ? slidePersist(7, 0, kTitleMaster) : QByteArray()), 1)
        + pptBox(4080,
                 slidePersist(5, 2, 0x100) + pptText(0, QStringLiteral("Quarterly Results"), pfRun(18, 0, 0) + cfRun(18, 0), true)
                     + (!bodyGroup.isEmpty() ? bodyGroup : pptText(1, QStringLiteral("First point\rSecond point\rThird point"), pfRun(12, 0, 0) + pfRun(13, 1, 0) + pfRun(12, 1, 0) + cfRun(12, 0) + cfRun(6, 1, le(1, 2)) + cfRun(19, 0)))
                     + slidePersist(4, 2, 0x101) + pptText(0, QStringLiteral("Plan"), QByteArray(), true)
                     // The first paragraph turns its bullet off: the bullet flags name only hasBullet, and it is clear.
                     + pptText(1, QStringLiteral("Ship\vit"), pfRun(8, 0, 1, le(0, 2)) + cfRun(8, 0))
                     + (titleMaster ? slidePersist(8, 2, 0x102) + pptText(6, QStringLiteral("Welcome"), QByteArray(), true) + pptText(5, QStringLiteral("To the talk")) : QByteArray())
                     + (links ? slidePersist(9, 2, 0x103) + pptText(0, QStringLiteral("Links"), QByteArray(), true)
                            + pptText(1, QStringLiteral("Go to the site. Slide *"), pfRun(24, 0, 0) + cfRun(24, 0))
                            // The ruler sets the first level's margin and indent; the language is French for the first 15
                            // characters; the slide number is the last; the link covers "the site".
                            + pptRec(4006, le(0x108, 4) + le(720, 2) + le(288, 2)) + pptRec(4010, le(15, 4) + le(2, 4) + le(0x040C, 2) + le(9, 4) + le(0, 4))
                            + pptRec(4056, le(22, 4)) + pptBox(4082, pptRec(4083, le(0, 4) + le(7, 4) + raw({4, 0, 0, 0, 8, 0, 0, 0})))
                            + pptRec(4063, le(6, 4) + le(14, 4))
                              : QByteArray()),
                 0)
        + pptBox(4080, slidePersist(6, 0, 0x200), 2)
        + pptRec(1002, QByteArray()));

    const QByteArray mainMaster = pptBox(1016, slideAtom(1, QByteArray("\x01\x02\x07\x08\x09", 5), 0, 0, 3) + colorScheme(1)
        // The title text: centered, the second font, 44 points, the title text color.
        + pptRec(4003, le(1, 2) + le(0x800, 4) + le(1, 2) + le(0x70000, 4) + le(1, 2) + le(44, 2) + pptColor(0, 0, 0, 3), 0)
        + masterExtra
        + pptDrawing(pptShape(0x402, 1, QByteArray(), 100, 200, 5000, 900, masterTitleProps) + pptShape(0x403, 2, QByteArray()) + masterBackground));
    const QByteArray titleMasterRec = pptBox(titleMasterRecord, slideAtom(3, QByteArray("\x03\x04\x07\x08\x09", 5), 0, 0, 1) + colorScheme(1)
        // The centered title and the subtitle, with only their sizes (the levels of these two start with their number).
        + pptRec(4003, le(1, 2) + le(0, 2) + le(0x800, 4) + le(1, 2) + le(0x20000, 4) + le(54, 2), 6)
        + pptRec(4003, le(1, 2) + le(0, 2) + le(0, 4) + le(0x20000, 4) + le(30, 2), 5)
        + pptDrawing(titleMasterShapes));

    auto slide = [&](int geom, const QByteArray &placeholders, quint32 master, quint32 notes, bool hidden, const QByteArray &shapes, const QByteArray &transition = QByteArray(), const QByteArray &extra = QByteArray()) {
        QByteArray info = le(0, 4) + le(0, 4) + raw({2, 0}) + le(hidden ? 0x4 : 0x400, 2) + raw({2, 0, 0, 0});
        return pptBox(1006, slideAtom(geom, placeholders, master, notes, 7) + pptRec(1017, transition.isEmpty() ? info : transition) + pptDrawing(shapes) + extra);
    };
    const QByteArray titleOnBody = pptShape(0x402, 0xD, outlineRef(0)) + pptShape(0x403, 0xE, outlineRef(1), 1000, 200, 5000, 3800);
    const QByteArray first = slide(1, QByteArray("\x0D\x0E", 2), kMaster, notesByAtom ? 0 : 0x200, false, titleOnBody + firstSlideShapes, QByteArray(), firstSlideExtra);
    const QByteArray secondOld = slide(1, QByteArray("\x0D\x0E", 2), kMaster, 0, false, titleOnBody);
    const QByteArray second = slide(1, QByteArray("\x0D\x0E", 2), kMaster, 0, true, titleOnBody);
    const QByteArray third = slide(0, QByteArray("\x0F\x10", 2), kTitleMaster, 0, false, pptShape(0x402, 0xF, outlineRef(0)) + pptShape(0x403, 0x10, outlineRef(1)));
    // Three seconds, then the next slide; effect 6, direction 1, speed 2.
    const QByteArray linksSlide = slide(1, QByteArray("\x0D\x0E", 2), kMaster, 0, false, titleOnBody, le(3000, 4) + le(0, 4) + raw({1, 6}) + le(0x501, 2) + raw({2, 0, 0, 0}));
    const QByteArray notes = pptBox(1008, pptRec(1009, le(0x100, 4) + le(0, 4), 0, 1)
        + pptDrawing(pptShape(0x402, 0xC, pptRec(3999, le(2, 4)) + pptRec(4000, utf16(QStringLiteral("Remember the demo"))))));

    QByteArray s = document;
    QHash<quint32, quint32> at;
    auto place = [&](quint32 persistId, const QByteArray &rec) {
        at[persistId] = quint32(s.size());
        s += rec;
    };
    at[1] = 0;
    place(3, mainMaster);
    if (titleMaster) {
        place(7, titleMasterRec);
        place(8, third);
    }
    place(4, secondOld);
    place(5, first);
    place(6, notes);
    if (links) place(9, linksSlide);
    QVector<QPair<quint32, quint32>> older{{1, 0}, {3, at[3]}, {4, at[4]}, {5, at[5]}, {6, at[6]}};
    if (titleMaster) older << qMakePair(quint32(7), at[7]) << qMakePair(quint32(8), at[8]);
    if (links) older << qMakePair(quint32(9), at[9]);
    std::sort(older.begin(), older.end());
    const quint32 olderDirectory = quint32(s.size());
    // Each entry names one id (a run of one), so the ids need not be next to each other.
    s += persistDirectory(older);
    const quint32 olderEdit = quint32(s.size());
    s += userEdit(0, olderDirectory, 1);
    place(4, second);
    const quint32 newerDirectory = quint32(s.size());
    s += persistDirectory({{4, at[4]}});
    const quint32 newerEdit = quint32(s.size());
    s += userEdit(olderEdit, newerDirectory, 1);
    if (cycle) s.replace(olderEdit + 8 + 8, 4, le(newerEdit, 4));

    cfb::File f;
    cfb::Entry root;
    root.type = cfb::Entry::Root;
    root.name = QStringLiteral("Root Entry");
    f.entries << root;
    auto stream = [&](const QString &name, const QByteArray &data) {
        cfb::Entry e;
        e.type = cfb::Entry::Stream;
        e.name = name;
        e.data = data;
        f.entries[0].children << int(f.entries.size());
        f.entries << e;
    };
    const QByteArray user = QByteArray("Tester");
    stream(QStringLiteral("Current User"), pptRec(4086, le(20, 4) + le(0xE391C05F, 4) + le(newerEdit, 4) + le(user.size(), 2) + le(0x3F4, 2) + raw({3, 0, 0, 0}) + user + le(8, 4)));
    stream(pptDocumentStream(), s);
    stream(QStringLiteral("Pictures"), QByteArray());
    return cfb::write(f);
}

QString placeholderText(const Document &d, const Page &slide, const QString &role)
{
    for (const auto &it : slide.items)
        if (it->role == role && it->type() == ItemType::Text) return d.storyDoc(static_cast<const TextItem *>(it.get())->storyId)->toPlainText();
    return QStringLiteral("<none>");
}

QTextDocument *placeholderStory(const Document &d, const Page &slide, const QString &role)
{
    for (const auto &it : slide.items)
        if (it->role == role && it->type() == ItemType::Text) return d.storyDoc(static_cast<const TextItem *>(it.get())->storyId);
    return nullptr;
}

QByteArray wmfWords(std::initializer_list<int> v)
{
    QByteArray b;
    for (int w : v) {
        b.append(char(w & 0xFF));
        b.append(char((w >> 8) & 0xFF));
    }
    return b;
}

QByteArray wmfDword(quint32 v)
{
    QByteArray b;
    for (int i = 0; i < 4; ++i) b.append(char((v >> (8 * i)) & 0xFF));
    return b;
}

QByteArray wmfRecord(int fn, const QByteArray &params)
{
    QByteArray b = wmfDword(quint32((6 + params.size() + (params.size() & 1)) / 2)) + wmfWords({fn});
    b += params;
    if (params.size() & 1) b.append('\0');
    return b;
}

TableItem *theTable(Editor *ed)
{
    for (const auto &it : ed->doc()->pages[ed->currentPage()]->items)
        if (it->type() == ItemType::Table) return static_cast<TableItem *>(it.get());
    return nullptr;
}

bool command(DeckWindow &w, const char *id)
{
    QAction *a = w.act(QString::fromLatin1(id));
    if (!a) {
        QTest::qFail(id, __FILE__, __LINE__);
        return false;
    }
    a->trigger();
    return true;
}

const PictureItem *firstPicture(const Document &d)
{
    for (const auto &p : d.pages)
        for (const auto &it : p->items)
            if (it->type() == ItemType::Picture) return static_cast<const PictureItem *>(it.get());
    return nullptr;
}

TextEffects shadowOf(const QColor &color, double blur, double distance, double angle)
{
    TextEffects fx;
    fx.outer.on = true;
    fx.outer.color = color;
    fx.outer.blur = blur;
    fx.outer.distance = distance;
    fx.outer.angle = angle;
    return fx;
}

std::unique_ptr<Document> showDeck(int n)
{
    auto d = deck::newPresentation();
    while (int(d->pages.size()) < n) deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
    for (int i = 0; i < n; ++i) {
        Page &s = *d->pages[i];
        d->storyDoc(static_cast<TextItem *>(s.items[0].get())->storyId)->setPlainText(QStringLiteral("Slide %1 title").arg(i + 1));
        d->storyDoc(s.notesStoryId)->setPlainText(QStringLiteral("Notes %1").arg(i + 1));
    }
    return d;
}

deck::Transition showTransition(const char *type, double seconds)
{
    deck::Transition t;
    t.type = QString::fromLatin1(type);
    t.duration = seconds;
    return t;
}

void numberShowShapes(Page &slide)
{
    slide.items[0]->sourceId = 2;
    slide.items[1]->sourceId = 3;
}

QImage paintedSlide(Document *d, LayoutCache *cache, int slide, const std::function<void(PaintContext &)> &hook)
{
    QImage img(QSize(960, 540), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    PaintContext ctx;
    ctx.doc = d;
    ctx.cache = cache;
    if (hook) hook(ctx);
    Renderer::paintPage(&p, ctx, slide);
    p.end();
    return img;
}

QString animXml(const char *xml) { return QString::fromLatin1(xml).replace(QLatin1Char('\''), QLatin1Char('"')); }

int animId() { static int n = 10; return ++n; }

QString animTarget(int spid, int paragraph)
{
    if (paragraph < 0) return animXml("<p:tgtEl><p:spTgt spid='%1'/></p:tgtEl>").arg(spid);
    return animXml("<p:tgtEl><p:spTgt spid='%1'><p:txEl><p:pRg st='%2' end='%2'/></p:txEl></p:spTgt></p:tgtEl>").arg(spid).arg(paragraph);
}

QString animShow(int spid, bool visible, int delay, int paragraph)
{
    return animXml("<p:set><p:cBhvr><p:cTn id='%1' dur='1' fill='hold'><p:stCondLst><p:cond delay='%2'/></p:stCondLst></p:cTn>%3<p:attrNameLst><p:attrName>style.visibility</p:attrName></p:attrNameLst></p:cBhvr><p:to><p:strVal val='%4'/></p:to></p:set>")
        .arg(animId()).arg(delay).arg(animTarget(spid, paragraph), visible ? QStringLiteral("visible") : QStringLiteral("hidden"));
}

QString animFilter(int spid, const char *filter, bool entering, int ms, int paragraph)
{
    return animXml("<p:animEffect transition='%1' filter='%2'><p:cBhvr><p:cTn id='%3' dur='%4'/>%5</p:cBhvr></p:animEffect>")
        .arg(entering ? QStringLiteral("in") : QStringLiteral("out"), QString::fromLatin1(filter)).arg(animId()).arg(ms).arg(animTarget(spid, paragraph));
}

QString animNode(const char *cls, int preset, int subtype, const char *nodeType, int delay, const QStringList &behaviors)
{
    return animXml("<p:par><p:cTn id='%1' presetID='%2' presetClass='%3' presetSubtype='%4' fill='hold' nodeType='%5'><p:stCondLst><p:cond delay='%6'/></p:stCondLst><p:childTnLst>%7</p:childTnLst></p:cTn></p:par>")
        .arg(animId()).arg(preset).arg(QString::fromLatin1(cls)).arg(subtype).arg(QString::fromLatin1(nodeType)).arg(delay).arg(behaviors.join(QString()));
}

QString animFadeIn(int spid, int ms, const char *nodeType, int delay, int paragraph)
{
    return animNode("entr", 10, 0, nodeType, delay, {animShow(spid, true, 0, paragraph), animFilter(spid, "fade", true, ms, paragraph)});
}

QString animAppear(int spid, const char *nodeType, int delay)
{
    return animNode("entr", 1, 0, nodeType, delay, {animShow(spid, true)});
}

QString animGroup(int delay, const QStringList &effects)
{
    return animXml("<p:par><p:cTn id='%1' fill='hold'><p:stCondLst><p:cond delay='%2'/></p:stCondLst><p:childTnLst>%3</p:childTnLst></p:cTn></p:par>")
        .arg(animId()).arg(delay).arg(effects.join(QString()));
}

QString animStep(const QStringList &groups, bool automatic)
{
    return animXml("<p:par><p:cTn id='%1' fill='hold'><p:stCondLst><p:cond delay='indefinite'/>%2</p:stCondLst><p:childTnLst>%3</p:childTnLst></p:cTn></p:par>")
        .arg(animId())
        .arg(automatic ? animXml("<p:cond evt='onBegin' delay='0'><p:tn val='2'/></p:cond>") : QString(), groups.join(QString()));
}

QString animSteps(const QStringList &effects)
{
    QString out;
    for (const QString &e : effects) out += animStep({animGroup(0, {e})});
    return out;
}

QString animTiming(const QString &steps, const QString &after)
{
    return animXml("<p:timing xmlns:p='http://schemas.openxmlformats.org/presentationml/2006/main'><p:tnLst><p:par><p:cTn id='1' dur='indefinite' restart='never' nodeType='tmRoot'><p:childTnLst><p:seq concurrent='1' nextAc='seek'><p:cTn id='2' dur='indefinite' nodeType='mainSeq'><p:childTnLst>")
        + steps
        + animXml("</p:childTnLst></p:cTn><p:prevCondLst><p:cond evt='onPrev' delay='0'><p:tgtEl><p:sldTgt/></p:tgtEl></p:cond></p:prevCondLst><p:nextCondLst><p:cond evt='onNext' delay='0'><p:tgtEl><p:sldTgt/></p:tgtEl></p:cond></p:nextCondLst></p:seq>")
        + after + QStringLiteral("</p:childTnLst></p:cTn></p:par></p:tnLst></p:timing>");
}

deck::Geometry animGeometry()
{
    deck::Geometry g;
    g.slide = QSizeF(960, 540);
    g.shapes.insert(4, QRectF(100, 200, 200, 100));   // its middle is at 200, 250
    g.shapes.insert(5, QRectF(500, 100, 100, 100));
    g.shapes.insert(6, QRectF(700, 300, 100, 100));
    return g;
}

bool animNear(double a, double b) { return std::abs(a - b) < 1e-6; }

std::unique_ptr<Document> editDeck(int boxes)
{
    auto d = showDeck(1);
    Page &slide = *d->pages[0];
    numberShowShapes(slide);
    d->storyDoc(static_cast<TextItem *>(slide.items[1].get())->storyId)->setPlainText(QStringLiteral("First paragraph\nSecond paragraph\nThird paragraph"));
    for (int i = 0; i < boxes; ++i) {
        auto box = std::make_shared<ShapeItem>();
        box->rect = QRectF(20 + 90 * (i % 8), 400 + 40 * (i / 8), 80, 30);
        box->fill = Fill::solid(ColorRef::rgb(QColor(200, 60, 40)));
        box->sourceId = 10 + i;
        box->name = QStringLiteral("Box %1").arg(i + 1);
        slide.items.push_back(box);
    }
    return d;
}

QString animIdProblem(const QString &timing)
{
    QDomDocument dom;
    if (!office::parseXml(timing.toUtf8(), &dom)) return QStringLiteral("not XML");
    QSet<QString> ids;
    QStringList refs;
    QVector<QDomElement> stack{dom.documentElement()};
    while (!stack.isEmpty()) {
        const QDomElement e = stack.takeLast();
        if (e.localName() == QLatin1String("cTn") && e.hasAttribute(QStringLiteral("id"))) {
            const QString id = e.attribute(QStringLiteral("id"));
            if (ids.contains(id)) return QStringLiteral("the id %1 is on two time nodes").arg(id);
            ids.insert(id);
        }
        if (e.localName() == QLatin1String("tn")) refs << e.attribute(QStringLiteral("val"));
        for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) stack << c;
    }
    for (const QString &r : std::as_const(refs))
        if (!ids.contains(r)) return QStringLiteral("a reference to the time node %1, which is not there").arg(r);
    return QString();
}

std::unique_ptr<Document> animatedDeck(int fadeMs)
{
    auto d = showDeck(3);
    numberShowShapes(*d->pages[0]);
    d->storyDoc(static_cast<TextItem *>(d->pages[0]->items[1].get())->storyId)->setPlainText(QStringLiteral("The body of the slide"));
    d->pages[0]->props[QStringLiteral("timingXml")] = animTiming(animSteps({animFadeIn(2, fadeMs), animAppear(3)}));
    return d;
}

QTextDocument *editTitle(DeckWindow &w, const QString &text, int from, int to)
{
    Editor *ed = w.editor();
    auto *title = static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get());
    QTextCursor(ed->doc()->storyDoc(title->storyId)).insertText(text);
    ed->select(title->id);
    ed->beginTextEdit(title->id, from);
    if (to > from) {
        QTextCursor c = ed->cursor();
        c.setPosition(from);
        c.setPosition(to, QTextCursor::KeepAnchor);
        ed->setCursor(c);
    }
    return ed->doc()->storyDoc(title->storyId);
}

QString nsDecl()
{
    return QStringLiteral(
        "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
        "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
        "xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"");
}

const QString &relBase()
{
    static const QString base = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    return base;
}

QByteArray tinyPng()
{
    QImage img(4, 4, QImage::Format_RGB32);
    img.fill(QColor(0xB6, 0x42, 0x0F));
    QByteArray bytes;
    QBuffer buf(&bytes);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    return bytes;
}

QString treeStart() { return QStringLiteral("<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>"); }

QString treeEnd() { return QStringLiteral("</p:spTree></p:cSld>"); }

QString placeholderShape(int id, const QString &name, const QString &ph, const QString &xfrm, const QString &body)
{
    return QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"%2\"/><p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr><p:nvPr><p:ph %3/></p:nvPr></p:nvSpPr>"
                          "<p:spPr>%4</p:spPr><p:txBody><a:bodyPr/><a:lstStyle/>%5</p:txBody></p:sp>")
        .arg(id).arg(name, ph, xfrm, body);
}

QString xfrm(qint64 x, qint64 y, qint64 cx, qint64 cy)
{
    return QStringLiteral("<a:xfrm><a:off x=\"%1\" y=\"%2\"/><a:ext cx=\"%3\" cy=\"%4\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>").arg(x).arg(y).arg(cx).arg(cy);
}

QString para(const QString &text)
{
    return text.isEmpty() ? QStringLiteral("<a:p><a:endParaRPr lang=\"en-US\"/></a:p>") : QStringLiteral("<a:p><a:r><a:rPr lang=\"en-US\"/><a:t>%1</a:t></a:r></a:p>").arg(text);
}

QByteArray themeXml()
{
    QString colors;
    const char *const names[12] = {"dk1", "lt1", "dk2", "lt2", "accent1", "accent2", "accent3", "accent4", "accent5", "accent6", "hlink", "folHlink"};
    const char *const values[12] = {"111111", "FEFEFE", "224466", "EEEEDD", "B6420F", "13773A", "1D55C4", "9E1F63", "0A6C8C", "5E2CA5", "0000EE", "551A8B"};
    for (int i = 0; i < 12; ++i) colors += QStringLiteral("<a:%1><a:srgbClr val=\"%2\"/></a:%1>").arg(QLatin1String(names[i]), QLatin1String(values[i]));
    const QString fill = QStringLiteral("<a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>");
    const QString line = QStringLiteral("<a:ln w=\"9525\">") + fill + QStringLiteral("</a:ln>");
    return QStringLiteral(
               "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" name=\"Test Theme\">"
               "<a:themeElements><a:clrScheme name=\"Test Colors\">%1</a:clrScheme>"
               "<a:fontScheme name=\"Test Fonts\"><a:majorFont><a:latin typeface=\"Georgia\"/><a:ea typeface=\"\"/><a:cs typeface=\"\"/></a:majorFont>"
               "<a:minorFont><a:latin typeface=\"Verdana\"/><a:ea typeface=\"\"/><a:cs typeface=\"\"/></a:minorFont></a:fontScheme>"
               "<a:fmtScheme name=\"Test Formats\"><a:fillStyleLst>%2%2%2</a:fillStyleLst><a:lnStyleLst>%3%3%3</a:lnStyleLst>"
               "<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>"
               "<a:bgFillStyleLst>%2%2%2</a:bgFillStyleLst></a:fmtScheme></a:themeElements></a:theme>")
        .arg(colors, fill, line)
        .toUtf8();
}

QString masterXml()
{
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:sldMaster %1>").arg(nsDecl())
        + QStringLiteral("<p:cSld name=\"Test Master\"><p:bg><p:bgPr><a:solidFill><a:srgbClr val=\"FFFFFF\"/></a:solidFill><a:effectLst/></p:bgPr></p:bg><p:spTree>"
                         "<p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>")
        + placeholderShape(2, QStringLiteral("Title Placeholder 1"), QStringLiteral("type=\"title\""), xfrm(838200, 365125, 10515600, 1325563), para(QStringLiteral("Click to edit Master title style")))
        + placeholderShape(3, QStringLiteral("Text Placeholder 2"), QStringLiteral("type=\"body\" idx=\"1\""), xfrm(838200, 1825625, 10515600, 4351338), para(QStringLiteral("Click to edit Master text styles")))
        + placeholderShape(4, QStringLiteral("Date Placeholder 3"), QStringLiteral("type=\"dt\" sz=\"half\" idx=\"2\""), xfrm(838200, 6356350, 2743200, 365125), para(QStringLiteral("10/11/2026")))
        + QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"5\" name=\"Master Band\"/><p:cNvSpPr/><p:nvPr userDrawn=\"1\"/></p:nvSpPr><p:spPr>") + xfrm(0, 0, 12192000, 182880)
        + QStringLiteral("<a:solidFill><a:schemeClr val=\"accent1\"/></a:solidFill></p:spPr></p:sp>")
        + QStringLiteral("</p:spTree></p:cSld>"
                         "<p:clrMap bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\" accent1=\"accent1\" accent2=\"accent2\" accent3=\"accent3\" accent4=\"accent4\" accent5=\"accent5\" accent6=\"accent6\" hlink=\"hlink\" folHlink=\"folHlink\"/>"
                         "<p:sldLayoutIdLst><p:sldLayoutId id=\"2147483649\" r:id=\"rId1\"/><p:sldLayoutId id=\"2147483650\" r:id=\"rId2\"/></p:sldLayoutIdLst>"
                         "<p:txStyles><p:titleStyle><a:lvl1pPr algn=\"l\"><a:defRPr sz=\"4400\"><a:latin typeface=\"+mj-lt\"/></a:defRPr></a:lvl1pPr></p:titleStyle>"
                         "<p:bodyStyle><a:lvl1pPr marL=\"228600\" indent=\"-228600\"><a:buChar char=\"&#8226;\"/><a:defRPr sz=\"2800\"/></a:lvl1pPr></p:bodyStyle>"
                         "<p:otherStyle><a:lvl1pPr><a:defRPr sz=\"1800\"/></a:lvl1pPr></p:otherStyle></p:txStyles></p:sldMaster>");
}

QString layoutXml(const QString &type, const QString &name, const QString &attrs, const QString &shapes)
{
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:sldLayout %1 type=\"%2\" %3>").arg(nsDecl(), type, attrs)
        + QStringLiteral("<p:cSld name=\"%1\"><p:spTree><p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>").arg(name) + shapes
        + QStringLiteral("</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sldLayout>");
}

QString slideXml(const QString &attrs, const QString &background, const QString &shapes, const QString &after)
{
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:sld %1 %2>").arg(nsDecl(), attrs)
        + QStringLiteral("<p:cSld>") + background + QStringLiteral("<p:spTree><p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>") + shapes
        + QStringLiteral("</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr>") + after + QStringLiteral("</p:sld>");
}

QString pictureShape(int id, qint64 x, qint64 y)
{
    return QStringLiteral("<p:pic><p:nvPicPr><p:cNvPr id=\"%1\" name=\"Picture %1\" descr=\"A small square\"/><p:cNvPicPr><a:picLocks noChangeAspect=\"1\"/></p:cNvPicPr><p:nvPr/></p:nvPicPr>"
                          "<p:blipFill><a:blip r:embed=\"rId2\"/><a:stretch><a:fillRect/></a:stretch></p:blipFill><p:spPr>%2</p:spPr></p:pic>")
        .arg(id).arg(xfrm(x, y, 914400, 914400));
}

QString unknownFrame(int id)
{
    return QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"%1\" name=\"Table %1\"/><p:cNvGraphicFramePr><a:graphicFrameLocks noGrp=\"1\"/></p:cNvGraphicFramePr><p:nvPr/></p:nvGraphicFramePr>"
                          "<p:xfrm><a:off x=\"914400\" y=\"1828800\"/><a:ext cx=\"3657600\" cy=\"1371600\"/></p:xfrm>"
                          "<a:graphic><a:graphicData uri=\"urn:test:unknown-kind\"><a:tbl><a:tblPr/><a:tblGrid><a:gridCol w=\"3657600\"/></a:tblGrid>"
                          "<a:tr h=\"370840\"><a:tc><a:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:t>Cell &amp; &lt;1&gt;</a:t></a:r></a:p></a:txBody><a:tcPr/></a:tc></a:tr></a:tbl></a:graphicData></a:graphic></p:graphicFrame>")
        .arg(id);
}

QString chartWithPicture(int id)
{
    return QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\">"
                          "<mc:Choice xmlns:cx1=\"urn:test:chart\" Requires=\"cx1\"><p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"%1\" name=\"Chart %1\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                          "<p:xfrm><a:off x=\"1219200\" y=\"1219200\"/><a:ext cx=\"6096000\" cy=\"3048000\"/></p:xfrm><a:graphic><a:graphicData uri=\"urn:test:chart\"><cx1:chart r:id=\"rId9\"/></a:graphicData></a:graphic></p:graphicFrame></mc:Choice>"
                          "<mc:Fallback>%2</mc:Fallback></mc:AlternateContent>")
        .arg(id)
        .arg(pictureShape(id + 1, 1219200, 1219200));
}

QString objectWithPicture(int id)
{
    return QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"%1\" name=\"Object %1\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                          "<p:xfrm><a:off x=\"7315200\" y=\"1219200\"/><a:ext cx=\"1828800\" cy=\"1828800\"/></p:xfrm><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/presentationml/2006/ole\">"
                          "<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice xmlns:v=\"urn:test:drawing\" Requires=\"v\">"
                          "<p:oleObj spid=\"_x0000_s1040\" name=\"Object\" r:id=\"rId7\"><p:embed/></p:oleObj></mc:Choice>"
                          "<mc:Fallback><p:oleObj name=\"Object\" r:id=\"rId7\"><p:embed/>%2</p:oleObj></mc:Fallback></mc:AlternateContent></a:graphicData></a:graphic></p:graphicFrame>")
        .arg(id)
        .arg(pictureShape(id + 1, 7315200, 1219200));
}

Parts samplePptx()
{
    const QString pres = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml");
    Parts p;
    p.data["/ppt/presentation.xml"] = {pres,
        (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:presentation %1>").arg(nsDecl())
         + QStringLiteral("<p:sldMasterIdLst><p:sldMasterId id=\"2147483648\" r:id=\"rId1\"/></p:sldMasterIdLst>"
                          "<p:notesMasterIdLst><p:notesMasterId r:id=\"rId4\"/></p:notesMasterIdLst>"
                          "<p:sldIdLst><p:sldId id=\"256\" r:id=\"rId2\"/><p:sldId id=\"257\" r:id=\"rId3\"/></p:sldIdLst>"
                          "<p:sldSz cx=\"12192000\" cy=\"6858000\"/><p:notesSz cx=\"6858000\" cy=\"9144000\"/>"
                          "<p:defaultTextStyle><a:defPPr><a:defRPr lang=\"en-US\"/></a:defPPr><a:lvl1pPr marL=\"0\" algn=\"l\"><a:defRPr sz=\"1800\"><a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill>"
                          "<a:latin typeface=\"+mn-lt\"/></a:defRPr></a:lvl1pPr></p:defaultTextStyle></p:presentation>")).toUtf8()};
    p.rels << Rel{"/ppt/presentation.xml", "slideMaster", "/ppt/slideMasters/slideMaster1.xml"}
           << Rel{"/ppt/presentation.xml", "slide", "/ppt/slides/slide1.xml"}
           << Rel{"/ppt/presentation.xml", "slide", "/ppt/slides/slide2.xml"}
           << Rel{"/ppt/presentation.xml", "notesMaster", "/ppt/notesMasters/notesMaster1.xml"};

    p.data["/ppt/theme/theme1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"), themeXml()};
    p.data["/ppt/slideMasters/slideMaster1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"), masterXml().toUtf8()};
    p.rels << Rel{"/ppt/slideMasters/slideMaster1.xml", "slideLayout", "/ppt/slideLayouts/slideLayout1.xml"}
           << Rel{"/ppt/slideMasters/slideMaster1.xml", "slideLayout", "/ppt/slideLayouts/slideLayout2.xml"}
           << Rel{"/ppt/slideMasters/slideMaster1.xml", "theme", "/ppt/theme/theme1.xml"};

    const QString layoutType = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml");
    p.data["/ppt/slideLayouts/slideLayout1.xml"] = {layoutType,
        layoutXml(QStringLiteral("title"), QStringLiteral("Title Slide"), QStringLiteral("showMasterSp=\"0\""),
                  placeholderShape(2, QStringLiteral("Title 1"), QStringLiteral("type=\"ctrTitle\""), xfrm(1524000, 1122363, 9144000, 2387600), para(QStringLiteral("Click to edit Master title style")))
                      + placeholderShape(3, QStringLiteral("Subtitle 2"), QStringLiteral("type=\"subTitle\" idx=\"1\""), xfrm(1524000, 3602038, 9144000, 1655762), para(QStringLiteral("Click to edit Master subtitle style"))))
            .toUtf8()};
    p.data["/ppt/slideLayouts/slideLayout2.xml"] = {layoutType,
        layoutXml(QStringLiteral("obj"), QStringLiteral("Title and Content"), QString(),
                  placeholderShape(2, QStringLiteral("Title 1"), QStringLiteral("type=\"title\""), QString(), para(QStringLiteral("Click to edit Master title style")))
                      + placeholderShape(3, QStringLiteral("Content Placeholder 2"), QStringLiteral("idx=\"1\""), xfrm(838200, 1825625, 5181600, 4351338), para(QStringLiteral("Click to edit Master text styles")))
                      + placeholderShape(4, QStringLiteral("Date Placeholder 3"), QStringLiteral("type=\"dt\" sz=\"half\" idx=\"10\""), QString(), para(QStringLiteral("10/11/2026"))))
            .toUtf8()};
    p.rels << Rel{"/ppt/slideLayouts/slideLayout1.xml", "slideMaster", "/ppt/slideMasters/slideMaster1.xml"}
           << Rel{"/ppt/slideLayouts/slideLayout2.xml", "slideMaster", "/ppt/slideMasters/slideMaster1.xml"};

    const QString slideType = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slide+xml");
    const QString dateField = QStringLiteral("<a:p><a:fld id=\"{B5D1E7E0-0000-4000-8000-000000000001}\" type=\"datetime1\"><a:rPr lang=\"en-US\"/><a:t>10/11/2026</a:t></a:fld></a:p>");
    p.data["/ppt/slides/slide1.xml"] = {slideType,
        slideXml(QString(), QString(),
                 placeholderShape(2, QStringLiteral("Title 1"), QStringLiteral("type=\"title\""), QString(), para(QStringLiteral("Quarterly results")))
                     + placeholderShape(3, QStringLiteral("Content Placeholder 2"), QStringLiteral("idx=\"1\""), QString(), para(QString()))
                     + placeholderShape(4, QStringLiteral("Date Placeholder 3"), QStringLiteral("type=\"dt\" sz=\"half\" idx=\"10\""), QString(), dateField)
                     + QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"5\" name=\"TextBox 4\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>") + xfrm(914400, 5029200, 3048000, 457200)
                     + QStringLiteral("<a:noFill/></p:spPr><p:txBody><a:bodyPr wrap=\"square\"/><a:lstStyle/>") + para(QStringLiteral("Plain text")) + QStringLiteral("</p:txBody></p:sp>")
                     + QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"6\" name=\"Rectangle 5\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr>") + xfrm(5486400, 5029200, 1828800, 914400)
                     + QStringLiteral("<a:solidFill><a:schemeClr val=\"accent1\"/></a:solidFill></p:spPr></p:sp>")
                     + pictureShape(7, 8229600, 5029200) + unknownFrame(8),
                 QStringLiteral("<p:transition spd=\"med\"><p:fade/></p:transition><p:timing><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" restart=\"never\" nodeType=\"tmRoot\"/></p:par></p:tnLst></p:timing>"))
            .toUtf8()};
    p.data["/ppt/slides/slide2.xml"] = {slideType,
        slideXml(QStringLiteral("show=\"0\" showMasterSp=\"0\""),
                 QStringLiteral("<p:bg><p:bgPr><a:solidFill><a:srgbClr val=\"112233\"/></a:solidFill><a:effectLst/></p:bgPr></p:bg>"),
                 placeholderShape(2, QStringLiteral("Title 1"), QStringLiteral("type=\"ctrTitle\""), QString(), para(QStringLiteral("Second")))
                     + placeholderShape(3, QStringLiteral("Subtitle 2"), QStringLiteral("type=\"subTitle\" idx=\"1\""), QString(), para(QString()))
                     + chartWithPicture(4) + objectWithPicture(6))
            .toUtf8()};
    p.rels << Rel{"/ppt/slides/slide1.xml", "slideLayout", "/ppt/slideLayouts/slideLayout2.xml"}
           << Rel{"/ppt/slides/slide1.xml", "image", "/ppt/media/image1.png"}
           << Rel{"/ppt/slides/slide1.xml", "notesSlide", "/ppt/notesSlides/notesSlide1.xml"}
           << Rel{"/ppt/slides/slide2.xml", "slideLayout", "/ppt/slideLayouts/slideLayout1.xml"}
           << Rel{"/ppt/slides/slide2.xml", "image", "/ppt/media/image1.png"};
    p.data["/ppt/media/image1.png"] = {QStringLiteral("image/png"), tinyPng()};

    p.data["/ppt/notesSlides/notesSlide1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.notesSlide+xml"),
        (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:notes %1>").arg(nsDecl()) + treeStart()
         + QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"2\" name=\"Slide Image Placeholder 1\"/><p:cNvSpPr/><p:nvPr><p:ph type=\"sldImg\"/></p:nvPr></p:nvSpPr><p:spPr/></p:sp>")
         + placeholderShape(3, QStringLiteral("Notes Placeholder 2"), QStringLiteral("type=\"body\" idx=\"1\""), QString(),
                            para(QStringLiteral("Say hello first")) + para(QStringLiteral("Then the numbers")))
         + treeEnd() + QStringLiteral("</p:notes>")).toUtf8()};
    p.rels << Rel{"/ppt/notesSlides/notesSlide1.xml", "notesMaster", "/ppt/notesMasters/notesMaster1.xml"}
           << Rel{"/ppt/notesSlides/notesSlide1.xml", "slide", "/ppt/slides/slide1.xml"};
    p.data["/ppt/notesMasters/notesMaster1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.notesMaster+xml"),
        (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:notesMaster %1>").arg(nsDecl()) + treeStart() + treeEnd()
         + QStringLiteral("<p:clrMap bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\" accent1=\"accent1\" accent2=\"accent2\" accent3=\"accent3\" accent4=\"accent4\" accent5=\"accent5\" accent6=\"accent6\" hlink=\"hlink\" folHlink=\"folHlink\"/>"
                          "<p:notesStyle><a:lvl1pPr marL=\"0\"><a:defRPr sz=\"1200\"/></a:lvl1pPr></p:notesStyle></p:notesMaster>")).toUtf8()};
    p.rels << Rel{"/ppt/notesMasters/notesMaster1.xml", "theme", "/ppt/theme/theme1.xml"};
    return p;
}

QByteArray pack(const Parts &p)
{
    office::PackageWriter w;
    w.addDefault(QStringLiteral("png"), QStringLiteral("image/png"));
    for (auto it = p.data.cbegin(); it != p.data.cend(); ++it) w.addPart(it.key(), it.value().xml, it.value().type);
    w.addRelationship(QStringLiteral("/"), relBase() + QStringLiteral("officeDocument"), QStringLiteral("/ppt/presentation.xml"));
    for (const Rel &r : p.rels) w.addRelationship(r.source, r.type.contains(QLatin1String("://")) ? r.type : relBase() + r.type, r.target, r.external);
    return w.finish();
}

QString legacyComment(int author, int idx, const QString &when, qint64 x, qint64 y, const QString &text, int bias, const QString &parent)
{
    QString ext;
    if (bias != INT_MIN || !parent.isEmpty()) {
        ext = QStringLiteral("<p:extLst><p:ext uri=\"{C676402C-5697-4E1C-873F-D02D1690AC5C}\"><p15:threadingInfo xmlns:p15=\"http://schemas.microsoft.com/office/powerpoint/2012/main\"");
        if (bias != INT_MIN) ext += QStringLiteral(" timeZoneBias=\"%1\"").arg(bias);
        if (parent.isEmpty()) ext += QStringLiteral("/>");
        else ext += QStringLiteral("><p15:parentCm authorId=\"%1\" idx=\"%2\"/></p15:threadingInfo>").arg(parent.section(QLatin1Char(':'), 0, 0), parent.section(QLatin1Char(':'), 1, 1));
        ext += QStringLiteral("</p:ext></p:extLst>");
    }
    return QStringLiteral("<p:cm authorId=\"%1\" dt=\"%2\" idx=\"%3\"><p:pos x=\"%4\" y=\"%5\"/><p:text>%6</p:text>%7</p:cm>")
        .arg(author).arg(when).arg(idx).arg(x).arg(y).arg(text.toHtmlEscaped(), ext);
}

QString legacyCommentList(const QString &body)
{
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:cmLst ") + nsDecl() + QStringLiteral(">") + body + QStringLiteral("</p:cmLst>");
}

Parts withLegacyComments()
{
    Parts p = samplePptx();
    p.data["/ppt/commentAuthors.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.commentAuthors+xml"),
        (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:cmAuthorLst ") + nsDecl() + QStringLiteral(">"
         "<p:cmAuthor id=\"0\" name=\"Pat Doe\" initials=\"PD\" lastIdx=\"1\" clrIdx=\"0\"/>"
         "<p:cmAuthor id=\"1\" name=\"Lee Roy\" initials=\"LR\" lastIdx=\"2\" clrIdx=\"1\"/></p:cmAuthorLst>")).toUtf8()};
    p.rels << Rel{"/ppt/presentation.xml", "commentAuthors", "/ppt/commentAuthors.xml"};
    QString body = legacyComment(0, 1, QStringLiteral("2026-10-12T10:15:30.250"), 5760, 1152, QStringLiteral("Check the numbers\non this slide"), 420);
    body += legacyComment(1, 1, QStringLiteral("2026-10-12T10:20:00.000"), 5760, 1152, QStringLiteral("Done, thanks"), 420, QStringLiteral("0:1"));
    body += legacyComment(1, 2, QStringLiteral("2026-10-12T11:00:00.000Z"), 800, 400, QStringLiteral("Second thread"));
    p.data["/ppt/comments/comment1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.comments+xml"), legacyCommentList(body).toUtf8()};
    p.rels << Rel{"/ppt/slides/slide1.xml", "comments", "/ppt/comments/comment1.xml"};
    return p;
}

bool readerReadsPictures()
{
    QDomDocument dom;
    if (!office::parseXml(("<p:pic " + nsDecl() + "><p:nvPicPr><p:cNvPr id=\"1\" name=\"P\"/><p:cNvPicPr/><p:nvPr/></p:nvPicPr><p:blipFill><a:blip/></p:blipFill><p:spPr>" + xfrm(0, 0, 1270, 1270) + "</p:spPr></p:pic>").toUtf8(), &dom))
        return false;
    return office::readShape(dom.documentElement(), office::DrawingContext()) != nullptr;
}

QString textOf(const Document &d, const Item &it)
{
    const auto *t = static_cast<const TextItem *>(&it);
    const QTextDocument *sd = d.storyDoc(t->storyId);
    return sd ? sd->toPlainText() : QString();
}

QByteArray imageBytes(const QImage &img, const char *format)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, format);
    return bytes;
}

QByteArray colorPng(const QColor &color)
{
    QImage img(4, 4, QImage::Format_RGB32);
    img.fill(color);
    QByteArray bytes;
    QBuffer buf(&bytes);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    return bytes;
}

QString dspNamespace() { return QStringLiteral("http://schemas.microsoft.com/office/drawing/2008/diagram"); }

QString diagramDrawingType() { return QStringLiteral("http://schemas.microsoft.com/office/2007/relationships/diagramDrawing"); }

QString diagramShape(const QString &place, const QString &geometry, const QString &fill, const QString &text, const QString &textPlace)
{
    return QStringLiteral("<dsp:sp modelId=\"{AAAAAAAA-0000-4000-8000-%1}\"><dsp:nvSpPr><dsp:cNvPr id=\"0\" name=\"\"/><dsp:cNvSpPr/></dsp:nvSpPr>"
                          "<dsp:spPr><a:xfrm>%2</a:xfrm><a:prstGeom prst=\"%3\"><a:avLst/></a:prstGeom>%4<a:ln w=\"12700\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\">"
                          "<a:solidFill><a:srgbClr val=\"FFFFFF\"/></a:solidFill><a:prstDash val=\"solid\"/><a:miter lim=\"800000\"/></a:ln><a:effectLst/></dsp:spPr>"
                          "<dsp:style><a:lnRef idx=\"2\"><a:scrgbClr r=\"0\" g=\"0\" b=\"0\"/></a:lnRef><a:fillRef idx=\"1\"><a:scrgbClr r=\"0\" g=\"0\" b=\"0\"/></a:fillRef>"
                          "<a:effectRef idx=\"0\"><a:scrgbClr r=\"0\" g=\"0\" b=\"0\"/></a:effectRef><a:fontRef idx=\"minor\"><a:schemeClr val=\"lt1\"/></a:fontRef></dsp:style>"
                          "<dsp:txBody><a:bodyPr spcFirstLastPara=\"0\" vert=\"horz\" wrap=\"square\" lIns=\"0\" tIns=\"0\" rIns=\"0\" bIns=\"0\" numCol=\"1\" spcCol=\"1270\" anchor=\"ctr\" anchorCtr=\"0\"><a:noAutofit/></a:bodyPr>"
                          "<a:lstStyle/><a:p><a:pPr algn=\"ctr\"/><a:r><a:rPr lang=\"en-US\" sz=\"2400\" kern=\"1200\"/><a:t>%5</a:t></a:r></a:p></dsp:txBody>"
                          "<dsp:txXfrm>%6</dsp:txXfrm></dsp:sp>")
        .arg(QStringLiteral("%1").arg(qHash(text) % 1000000000000ULL, 12, 10, QLatin1Char('0')), place, geometry, fill, text, textPlace);
}

QString diagramPlace(qint64 x, qint64 y, qint64 cx, qint64 cy)
{
    return QStringLiteral("<a:off x=\"%1\" y=\"%2\"/><a:ext cx=\"%3\" cy=\"%4\"/>").arg(x).arg(y).arg(cx).arg(cy);
}

QByteArray diagramDrawing()
{
    const QString head = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><dsp:drawing xmlns:dgm=\"http://schemas.openxmlformats.org/drawingml/2006/diagram\" "
                                        "xmlns:dsp=\"%1\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
                                        "<dsp:spTree><dsp:nvGrpSpPr><dsp:cNvPr id=\"0\" name=\"\"/><dsp:cNvGrpSpPr/></dsp:nvGrpSpPr><dsp:grpSpPr/>").arg(dspNamespace());
    const QString accent = QStringLiteral("<a:solidFill><a:schemeClr val=\"accent1\"><a:hueOff val=\"0\"/><a:satOff val=\"0\"/><a:lumOff val=\"0\"/><a:alphaOff val=\"0\"/></a:schemeClr></a:solidFill>");
    const QString picture = QStringLiteral("<a:blipFill><a:blip r:embed=\"rId1\"/><a:stretch><a:fillRect/></a:stretch></a:blipFill>");
    return (head
            + diagramShape(diagramPlace(0, 457200, 1066800, 914400), QStringLiteral("rect"), accent, QStringLiteral("First"), diagramPlace(0, 457200, 1066800, 914400))
            + diagramShape(diagramPlace(1295400, 457200, 1066800, 914400), QStringLiteral("roundRect"), QStringLiteral("<a:solidFill><a:srgbClr val=\"336699\"/></a:solidFill>"),
                           QStringLiteral("Second"), diagramPlace(1447800, 609600, 762000, 609600))
            + diagramShape(diagramPlace(2590800, 457200, 1066800, 914400), QStringLiteral("rect"), picture, QStringLiteral("Third"), diagramPlace(2590800, 457200, 1066800, 914400))
            + QStringLiteral("</dsp:spTree></dsp:drawing>")).toUtf8();
}

Parts smartArtPptx(bool alone, bool withDrawing)
{
    Parts p = samplePptx();
    const QString slide = QStringLiteral("/ppt/slides/slide1.xml");
    if (alone) {
        p.rels.erase(std::remove_if(p.rels.begin(), p.rels.end(), [&](const Rel &r) { return r.source == slide && r.type != QLatin1String("slideLayout"); }), p.rels.end());
        p.data[slide].xml = slideXml(QString(), QString(), QString()).toUtf8();
    }
    int next = 1;
    for (const Rel &r : std::as_const(p.rels)) next += r.source == slide;
    const QString frame = QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"9\" name=\"Diagram 8\" descr=\"A process\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                                         "<p:xfrm><a:off x=\"914400\" y=\"914400\"/><a:ext cx=\"3657600\" cy=\"1828800\"/></p:xfrm><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/diagram\">"
                                         "<dgm:relIds xmlns:dgm=\"http://schemas.openxmlformats.org/drawingml/2006/diagram\" r:dm=\"rId%1\" r:lo=\"rId%2\" r:qs=\"rId%3\" r:cs=\"rId%4\"/>"
                                         "</a:graphicData></a:graphic></p:graphicFrame>").arg(next).arg(next + 1).arg(next + 2).arg(next + 3);
    QString xml = QString::fromUtf8(p.data[slide].xml);
    xml.replace(QStringLiteral("</p:spTree>"), frame + QStringLiteral("</p:spTree>"));
    p.data[slide].xml = xml.toUtf8();
    const QString diagramType = QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.diagram");
    const QString ext = withDrawing ? QStringLiteral("<dgm:extLst><a:ext uri=\"http://schemas.microsoft.com/office/drawing/2008/diagram\"><dsp:dataModelExt xmlns:dsp=\"%1\" relId=\"rId%2\" "
                                                     "minVer=\"http://schemas.openxmlformats.org/drawingml/2006/diagram\"/></a:ext></dgm:extLst>").arg(dspNamespace()).arg(next + 4)
                                    : QString();
    const QString head = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>");
    const QString dgmNs = QStringLiteral("xmlns:dgm=\"http://schemas.openxmlformats.org/drawingml/2006/diagram\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"");
    p.data["/ppt/diagrams/data1.xml"] = {diagramType + QStringLiteral("Data+xml"),
        (head + QStringLiteral("<dgm:dataModel %1><dgm:ptLst><dgm:pt modelId=\"{1}\" type=\"doc\"><dgm:prSet loTypeId=\"urn:test:layout\"/><dgm:spPr/><dgm:t><a:bodyPr/><a:p><a:r><a:t>First</a:t></a:r></a:p></dgm:t></dgm:pt></dgm:ptLst>"
                             "<dgm:cxnLst/><dgm:bg/><dgm:whole/>%2</dgm:dataModel>").arg(dgmNs, ext)).toUtf8()};
    p.data["/ppt/diagrams/layout1.xml"] = {diagramType + QStringLiteral("Layout+xml"), (head + QStringLiteral("<dgm:layoutDef %1 uniqueId=\"urn:test:layout\"><dgm:title val=\"\"/><dgm:desc val=\"\"/></dgm:layoutDef>").arg(dgmNs)).toUtf8()};
    p.data["/ppt/diagrams/quickStyle1.xml"] = {diagramType + QStringLiteral("Style+xml"), (head + QStringLiteral("<dgm:styleDef %1 uniqueId=\"urn:test:style\"><dgm:title val=\"\"/><dgm:desc val=\"\"/></dgm:styleDef>").arg(dgmNs)).toUtf8()};
    p.data["/ppt/diagrams/colors1.xml"] = {diagramType + QStringLiteral("Colors+xml"), (head + QStringLiteral("<dgm:colorsDef %1 uniqueId=\"urn:test:colors\"><dgm:title val=\"\"/><dgm:desc val=\"\"/></dgm:colorsDef>").arg(dgmNs)).toUtf8()};
    p.rels << Rel{slide, "diagramData", "/ppt/diagrams/data1.xml"} << Rel{slide, "diagramLayout", "/ppt/diagrams/layout1.xml"} << Rel{slide, "diagramQuickStyle", "/ppt/diagrams/quickStyle1.xml"}
           << Rel{slide, "diagramColors", "/ppt/diagrams/colors1.xml"};
    if (withDrawing) {
        p.data["/ppt/diagrams/drawing1.xml"] = {QStringLiteral("application/vnd.ms-office.drawingml.diagramDrawing+xml"), diagramDrawing()};
        p.data["/ppt/media/diagram1.png"] = {QStringLiteral("image/png"), colorPng(QColor(0x20, 0x90, 0x40))};
        p.rels << Rel{slide, diagramDrawingType(), "/ppt/diagrams/drawing1.xml"} << Rel{"/ppt/diagrams/drawing1.xml", "image", "/ppt/media/diagram1.png"};
    }
    return p;
}

const PictureItem *keptPicture(const Document &d, const Page &page, const QString &kind)
{
    for (const QJsonValue &v : page.props.value(QStringLiteral("charts")).toArray()) {
        if (v.toObject().value(QStringLiteral("kind")).toString() != kind) continue;
        const Document::Loc at = d.find(v.toObject().value(QStringLiteral("item")).toString());
        if (at.item && at.item->type() == ItemType::Picture) return static_cast<const PictureItem *>(at.item);
    }
    return nullptr;
}

QVector<qint64> placeOf(const QDomElement &xfrm)
{
    const QDomElement off = office::child(xfrm, office::ns::A, QStringLiteral("off")), ext = office::child(xfrm, office::ns::A, QStringLiteral("ext"));
    return {off.attribute(QStringLiteral("x")).toLongLong(), off.attribute(QStringLiteral("y")).toLongLong(), ext.attribute(QStringLiteral("cx")).toLongLong(), ext.attribute(QStringLiteral("cy")).toLongLong()};
}

QString mathNamespace() { return QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/math"); }

QByteArray equationPicture()
{
    QImage img(60, 20, QImage::Format_RGB32);
    img.fill(QColor(0xC0, 0x30, 0x30));
    QByteArray bytes;
    QBuffer buf(&bytes);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    return bytes;
}

QString equationAlternative(int id, int embed, const QString &fallback, bool mixed)
{
    const QString math = QStringLiteral("<m:oMath><m:r><a:rPr lang=\"en-US\" sz=\"2800\" i=\"1\"><a:latin typeface=\"Cambria Math\" panose=\"02040503050406030204\" pitchFamily=\"18\" charset=\"0\"/></a:rPr>"
                                        "<m:t>E=mc</m:t></m:r><m:sSup><m:sSupPr><m:ctrlPr><a:rPr lang=\"en-US\" sz=\"2800\" i=\"1\"/></m:ctrlPr></m:sSupPr><m:e/><m:sup><m:r><m:t>2</m:t></m:r></m:sup></m:sSup></m:oMath>");
    const QString body = mixed ? QStringLiteral("<a:r><a:rPr lang=\"en-US\" sz=\"2800\"/><a:t>Energy </a:t></a:r><a14:m>%1</a14:m><a:r><a:rPr lang=\"en-US\" sz=\"2800\"/><a:t> is mass</a:t></a:r>").arg(math)
                               : QStringLiteral("<a14:m><m:oMathPara><m:oMathParaPr><m:jc m:val=\"centerGroup\"/></m:oMathParaPr>%1</m:oMathPara></a14:m>").arg(math);
    const QString place = QStringLiteral("<a:xfrm><a:off x=\"1524000\" y=\"2209800\"/><a:ext cx=\"1905000\" cy=\"508000\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>");
    const QString fallbackShape = fallback == QLatin1String("pic")
        ? QStringLiteral("<p:pic><p:nvPicPr><p:cNvPr id=\"%1\" name=\"TextBox %2\"/><p:cNvPicPr><a:picLocks noRot=\"1\" noChangeAspect=\"1\"/></p:cNvPicPr><p:nvPr/></p:nvPicPr>"
                         "<p:blipFill><a:blip r:embed=\"rId%3\"/><a:stretch><a:fillRect/></a:stretch></p:blipFill><p:spPr>%4</p:spPr></p:pic>")
              .arg(id).arg(id - 1).arg(embed).arg(place)
        : QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"TextBox %2\"/><p:cNvSpPr txBox=\"1\"><a:spLocks noRot=\"1\" noChangeArrowheads=\"1\"/></p:cNvSpPr><p:nvPr/></p:nvSpPr>"
                         "<p:spPr bwMode=\"auto\">%4<a:blipFill><a:blip r:embed=\"rId%3\"/><a:stretch><a:fillRect/></a:stretch></a:blipFill><a:ln><a:noFill/></a:ln></p:spPr>"
                         "<p:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang=\"en-US\"><a:noFill/></a:rPr><a:t> </a:t></a:r></a:p></p:txBody></p:sp>")
              .arg(id).arg(id - 1).arg(embed).arg(place);
    return QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\">"
                          "<mc:Choice xmlns:a14=\"http://schemas.microsoft.com/office/drawing/2010/main\" Requires=\"a14\"><p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"TextBox %2\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr>"
                          "<p:spPr>%3<a:noFill/></p:spPr><p:txBody><a:bodyPr wrap=\"none\" rtlCol=\"0\"><a:spAutoFit/></a:bodyPr><a:lstStyle/><a:p>%4<a:endParaRPr lang=\"en-US\" sz=\"2800\"/></a:p></p:txBody></p:sp></mc:Choice>"
                          "<mc:Fallback>%5</mc:Fallback></mc:AlternateContent>")
        .arg(id).arg(id - 1).arg(place, body, fallbackShape)
        .replace(QStringLiteral("<m:oMathPara>"), QStringLiteral("<m:oMathPara xmlns:m=\"%1\">").arg(mathNamespace()))
        .replace(QStringLiteral("<m:oMath>"), mixed ? QStringLiteral("<m:oMath xmlns:m=\"%1\">").arg(mathNamespace()) : QStringLiteral("<m:oMath>"));
}

Parts equationPptx(const QString &fallback, bool mixed, bool withPicture)
{
    Parts p = samplePptx();
    const QString slide = QStringLiteral("/ppt/slides/slide1.xml");
    int next = 1;
    for (const Rel &r : std::as_const(p.rels)) next += r.source == slide;
    QString xml = QString::fromUtf8(p.data[slide].xml);
    xml.replace(QStringLiteral("</p:spTree>"), equationAlternative(9, next, fallback, mixed) + QStringLiteral("</p:spTree>"));
    p.data[slide].xml = xml.toUtf8();
    if (withPicture) {
        p.data["/ppt/media/equation1.png"] = {QStringLiteral("image/png"), equationPicture()};
        p.rels << Rel{slide, "image", "/ppt/media/equation1.png"};
    }
    return p;
}

Parts chartPptx()
{
    Parts p = samplePptx();
    QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
    const QString frame = QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"9\" name=\"Chart 8\" descr=\"Sales by region\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                                         "<p:xfrm><a:off x=\"914400\" y=\"914400\"/><a:ext cx=\"3657600\" cy=\"2743200\"/></p:xfrm><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/chart\">"
                                         "<c:chart xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" r:id=\"rId4\"/></a:graphicData></a:graphic></p:graphicFrame>");
    xml.replace(QStringLiteral("</p:spTree>"), frame + QStringLiteral("</p:spTree>"));
    p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
    p.rels << Rel{"/ppt/slides/slide1.xml", "chart", "/ppt/charts/chart1.xml"};
    auto series = [](int idx, const QString &name, const QString &values) {
        return QStringLiteral("<c:ser><c:idx val=\"%1\"/><c:order val=\"%1\"/><c:tx><c:strRef><c:f>Sheet1!$B$1</c:f><c:strCache><c:ptCount val=\"1\"/><c:pt idx=\"0\"><c:v>%2</c:v></c:pt></c:strCache></c:strRef></c:tx>"
                              "<c:cat><c:strRef><c:f>Sheet1!$A$2:$A$4</c:f><c:strCache><c:ptCount val=\"3\"/><c:pt idx=\"0\"><c:v>East</c:v></c:pt><c:pt idx=\"1\"><c:v>West</c:v></c:pt><c:pt idx=\"2\"><c:v>North</c:v></c:pt></c:strCache></c:strRef></c:cat>"
                              "<c:val><c:numRef><c:f>Sheet1!$B$2:$B$4</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"3\"/>%3</c:numCache></c:numRef></c:val></c:ser>")
            .arg(idx).arg(name, values);
    };
    const QString chart = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" "
                                         "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
                                         "<c:chart><c:autoTitleDeleted val=\"1\"/><c:plotArea><c:layout/><c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>")
                          + series(0, QStringLiteral("Q1"), QStringLiteral("<c:pt idx=\"0\"><c:v>10</c:v></c:pt><c:pt idx=\"1\"><c:v>20</c:v></c:pt><c:pt idx=\"2\"><c:v>40</c:v></c:pt>"))
                          + series(1, QStringLiteral("Q2"), QStringLiteral("<c:pt idx=\"0\"><c:v>5</c:v></c:pt><c:pt idx=\"1\"><c:v>15</c:v></c:pt><c:pt idx=\"2\"><c:v>25</c:v></c:pt>"))
                          + QStringLiteral("<c:axId val=\"1\"/><c:axId val=\"2\"/></c:barChart>"
                                           "<c:catAx><c:axId val=\"1\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:crossAx val=\"2\"/></c:catAx>"
                                           "<c:valAx><c:axId val=\"2\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorGridlines/><c:crossAx val=\"1\"/></c:valAx>"
                                           "</c:plotArea><c:legend><c:legendPos val=\"b\"/></c:legend><c:plotVisOnly val=\"1\"/></c:chart>"
                                           "<c:externalData r:id=\"rId1\"><c:autoUpdate val=\"0\"/></c:externalData><c:userShapes r:id=\"rId2\"/></c:chartSpace>");
    p.data["/ppt/charts/chart1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"), chart.toUtf8()};
    p.rels << Rel{"/ppt/charts/chart1.xml", "package", "/ppt/embeddings/Sheet1.xlsx"}
           << Rel{"/ppt/charts/chart1.xml", "chartUserShapes", "/ppt/drawings/drawing1.xml"}
           << Rel{"/ppt/charts/chart1.xml", "themeOverride", "/ppt/theme/themeOverride1.xml"};
    p.data["/ppt/embeddings/Sheet1.xlsx"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"), QByteArray("PK\x03\x04 a workbook, as far as this test goes")};
    p.data["/ppt/drawings/drawing1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chartshapes+xml"),
        QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><c:userShapes xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"/>")};
    p.rels << Rel{"/ppt/drawings/drawing1.xml", "image", "/ppt/media/image1.png"};
    p.data["/ppt/theme/themeOverride1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.themeOverride+xml"), themeXml()};
    return p;
}

const PictureItem *chartPicture(const Document &d, const Page &page)
{
    const QJsonArray charts = page.props.value(QStringLiteral("charts")).toArray();
    for (const QJsonValue &v : charts) {
        const Document::Loc at = d.find(v.toObject().value(QStringLiteral("item")).toString());
        if (at.item && at.item->type() == ItemType::Picture) return static_cast<const PictureItem *>(at.item);
    }
    // A chart that was not kept shows as no picture at all: pictures with an SVG of their own and the name the frame had.
    return nullptr;
}

std::unique_ptr<Document> deckWithPictureBlip(const QString &blip, QString *err)
{
    Parts p = samplePptx();
    p.data["/ppt/slides/slide1.xml"].xml.replace("<a:blip r:embed=\"rId2\"/>", blip.toUtf8());
    return deck::importPresentation(pack(p), err);
}

QColor colorOfSamplePicture(Document &d)
{
    LayoutCache cache;
    PaintContext ctx;
    ctx.doc = &d;
    ctx.cache = &cache;
    return Renderer::renderToImage(ctx, 0, 1.0).pixelColor(684, 432);
}

bool colorsNear(const QColor &a, const QColor &b, int by)
{
    return qAbs(a.red() - b.red()) <= by && qAbs(a.green() - b.green()) <= by && qAbs(a.blue() - b.blue()) <= by;
}

bool unpack(const QByteArray &bytes, Exported *out, QString *why)
{
    return readZip(bytes, out->zip, why) && out->pkg.read(bytes, why);
}

QString part(const Exported &e, const QString &name) { return QString::fromUtf8(e.zip.value(name.mid(1))); }

QImage slideImage(Document &doc)
{
    LayoutCache cache;
    PaintContext ctx;
    ctx.doc = &doc;
    ctx.cache = &cache;
    return Renderer::renderToImage(ctx, 0, 0.5);
}

QString packageProblem(const Exported &e)
{
    QDomDocument types;
    QString why;
    if (!e.zip.contains(QStringLiteral("[Content_Types].xml"))) return QStringLiteral("no [Content_Types].xml");
    if (!office::parseXml(e.zip.value(QStringLiteral("[Content_Types].xml")), &types, &why)) return QStringLiteral("[Content_Types].xml: ") + why;
    QSet<QString> defaults, overrides;
    for (const QDomElement &d : office::children(types.documentElement(), office::ns::ContentTypes, QStringLiteral("Default"))) defaults << d.attribute(QStringLiteral("Extension")).toLower();
    for (const QDomElement &o : office::children(types.documentElement(), office::ns::ContentTypes, QStringLiteral("Override"))) {
        const QString name = o.attribute(QStringLiteral("PartName"));
        if (!e.zip.contains(name.mid(1))) return QStringLiteral("the content types list a part that is missing: ") + name;
        overrides << name.toLower();
    }
    for (auto it = e.zip.cbegin(); it != e.zip.cend(); ++it) {
        if (it.key() == QLatin1String("[Content_Types].xml")) continue;
        if (!defaults.contains(QFileInfo(it.key()).suffix().toLower()) && !overrides.contains(QLatin1Char('/') + it.key().toLower()))
            return QStringLiteral("no content type for ") + it.key();
        if (!it.key().endsWith(QLatin1String(".xml")) && !it.key().endsWith(QLatin1String(".rels"))) continue;
        QDomDocument dom;
        if (!office::parseXml(it.value(), &dom, &why)) return it.key() + QStringLiteral(" doesn't parse: ") + why;
        if (!it.value().startsWith("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>")) return it.key() + QStringLiteral(" has no XML declaration");
        // Every r:id (and r:embed...) in a part is one of its relationships; an empty one
        // points nowhere (a picture's click that plays its movie, as the presentation program writes it).
        if (it.key().endsWith(QLatin1String(".rels"))) continue;
        QVector<QDomElement> stack{dom.documentElement()};
        while (!stack.isEmpty()) {
            const QDomElement el = stack.takeLast();
            const QDomNamedNodeMap attrs = el.attributes();
            for (int i = 0; i < attrs.length(); ++i) {
                const QDomAttr a = attrs.item(i).toAttr();
                if (!office::sameNamespace(a.namespaceURI(), office::ns::R) || a.value().isEmpty()) continue;
                if (e.pkg.relationship(QLatin1Char('/') + it.key(), a.value()).id.isEmpty()) return it.key() + QStringLiteral(" uses ") + a.name() + QStringLiteral("=") + a.value() + QStringLiteral(", which isn't a relationship");
            }
            for (QDomElement c = el.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) stack << c;
        }
    }
    QStringList sources = e.pkg.partNames();
    sources << QStringLiteral("/");
    for (const QString &s : std::as_const(sources))
        for (const office::Relationship &r : e.pkg.relationships(s)) {
            if (!r.external && !e.pkg.hasPart(r.target)) return s + QStringLiteral(" links to a part that is missing: ") + r.target;
            if (!r.external && r.type.isEmpty()) return s + QStringLiteral(" has a link without a type");
        }
    QSet<QString> reached;
    QStringList queue{QStringLiteral("/")};
    while (!queue.isEmpty()) {
        const QString s = queue.takeLast();
        for (const office::Relationship &r : e.pkg.relationships(s))
            if (!r.external && !reached.contains(r.target.toLower())) {
                reached << r.target.toLower();
                queue << r.target;
            }
    }
    for (const QString &name : e.pkg.partNames())
        if (!reached.contains(name.toLower())) return name + QStringLiteral(" is not linked from anywhere");
    return QString();
}

QDomElement rootOf(const QByteArray &xml, QDomDocument *dom)
{
    office::parseXml(xml, dom);
    return dom->documentElement();
}

QStringList relationshipTargets(const Exported &e, const QString &source, const QString &typeSuffix)
{
    QStringList out;
    for (const office::Relationship &r : e.pkg.relationshipsOfType(source, typeSuffix)) out << r.target;
    return out;
}

void answerDialogs(QList<QMessageBox::StandardButton> answers, QString *label, QDeadlineTimer until)
{
    if (answers.isEmpty() || until.hasExpired()) return;
    QTimer::singleShot(0, [answers, label, until]() mutable {
        QWidget *dialog = QApplication::activeModalWidget();
        if (!dialog) {
            answerDialogs(answers, label, until);
            return;
        }
        const QMessageBox::StandardButton which = answers.takeFirst();
        auto *box = qobject_cast<QMessageBox *>(dialog);
        QAbstractButton *button = box && which != QMessageBox::NoButton ? box->button(which) : nullptr;
        if (button && label && label->isEmpty()) *label = button->text();
        if (button) button->click();
        else dialog->close();
        answerDialogs(answers, label, until);
    });
}

QByteArray fileBytes(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

QColor textColorOf(const Document &d, const TextItem &t)
{
    const QTextDocument *sd = d.storyDoc(t.storyId);
    const QTextBlock b = sd->begin();
    const QTextCharFormat cf = b.begin().atEnd() ? b.charFormat() : b.begin().fragment().charFormat();
    const QString ref = cf.stringProperty(tp::ColorRefP);
    return ref.isEmpty() ? cf.foreground().color() : ColorRef::fromString(ref).resolve(d.colors);
}

const TextItem *placeholderOn(const Page &slide, const QString &type)
{
    for (const auto &it : slide.items)
        if (deck::placeholderType(*it) == type && it->type() == ItemType::Text) return static_cast<const TextItem *>(it.get());
    return nullptr;
}

QStringList layoutTypes(const Document &d)
{
    QStringList out;
    for (const auto &p : d.pages) out << deck::layoutType(*d.masterFor(*p));
    return out;
}

const Item *placeholderOn(const PageBase &page, const QString &type)
{
    for (const auto &it : page.items)
        if (deck::placeholderType(*it) == type) return it.get();
    return nullptr;
}

int pdfPageCount(const QByteArray &pdf)
{
    static const QRegularExpression page(QStringLiteral("/Type\\s*/Page(?![A-Za-z])"));
    int n = 0;
    for (auto it = page.globalMatch(QString::fromLatin1(pdf)); it.hasNext(); it.next()) ++n;
    return n;
}

void answerOutputDialog(const std::function<void(QDialog *)> &set, QDialogButtonBox::StandardButton button,
                               QDeadlineTimer until)
{
    QTimer::singleShot(0, [set, button, until] {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog || dialog->objectName() != QLatin1String("outputDialog")) {
            if (!until.hasExpired()) answerOutputDialog(set, button, until);
            return;
        }
        if (set) set(dialog);
        dialog->findChild<QDialogButtonBox *>()->button(button)->click();
    });
}

} // namespace deckhelp

void initDeckTestCase()
{
    static QTemporaryDir settingsDir;
    Q_INIT_RESOURCE(resources);
    Q_INIT_RESOURCE(deck);
    setAppInfo(deckAppInfo());
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settingsDir.path());
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDir.path());
    initCore();
}
