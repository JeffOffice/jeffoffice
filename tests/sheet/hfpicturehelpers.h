// Workbooks whose headers and footers hold pictures, for the tests of reading, drawing, and saving them: a sheet with the header and footer texts, the legacy drawing the sheet names with a
// shape for each picture, and the image parts the shapes name.
#pragma once

#include "drawinghelpers.h"
#include "sheethelpers.h"

namespace sheettest {

struct HfShape {
    QString id;                    // the place: "CH", "LF", ...
    QString style = QStringLiteral("position:absolute;margin-left:0;margin-top:0;width:72pt;height:36pt;z-index:1");
    QString relId = QStringLiteral("rId1");
    QString title = QStringLiteral("logo");
};

inline HfShape hfShape(const QString &id, const QString &relId = QStringLiteral("rId1"))
{
    HfShape s;
    s.id = id;
    s.relId = relId;
    return s;
}

inline QByteArray hfVml(const QVector<HfShape> &shapes)
{
    QString xml = QStringLiteral("<xml xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\">"
                                 "<v:shapetype id=\"_x0000_t75\" coordsize=\"21600,21600\" o:spt=\"75\" path=\"m@4@5l@4@11@9@11@9@5xe\" filled=\"f\" stroked=\"f\"><v:stroke joinstyle=\"miter\"/></v:shapetype>");
    int n = 0;
    for (const HfShape &s : shapes)
        xml += QStringLiteral("<v:shape id=\"%1\" o:spid=\"_x0000_s%2\" type=\"#_x0000_t75\" style=\"%3\"><v:imagedata%4 o:title=\"%5\"/><o:lock v:ext=\"edit\" rotation=\"t\"/></v:shape>")
                   .arg(s.id).arg(1025 + n++).arg(s.style, s.relId.isEmpty() ? QString() : QStringLiteral(" o:relid=\"%1\"").arg(s.relId), s.title);
    return (xml + QStringLiteral("</xml>")).toUtf8();
}

// A workbook of one sheet (12 rows of numbers) with the header and footer element given (the inside of headerFooter), the drawing the shapes make, and the images by relationship id
// (rId1 ...) as png files; `relType` is the type of the relationships to the images.
inline QByteArray bookWithHeaderPictures(const QString &headerFooter, const QVector<HfShape> &shapes, const QVector<QByteArray> &images, const QString &relType = QStringLiteral("image"))
{
    QString rows;
    for (int r = 1; r <= 12; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><v>%2</v></c></row>").arg(r).arg(r * 2);
    const QByteArray sheet = worksheet(rows, QStringLiteral("<headerFooter>%1</headerFooter><legacyDrawingHF r:id=\"rIdH1\"/>").arg(headerFooter));
    QVector<ExtraPart> parts;
    parts.append({QStringLiteral("xl/drawings/vmlDrawing1.vml"), hfVml(shapes), QStringLiteral("application/vnd.openxmlformats-officedocument.vmlDrawing"), 0, QStringLiteral("rIdH1"), QStringLiteral("vmlDrawing"),
                  QStringLiteral("../drawings/vmlDrawing1.vml")});
    QString rels = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">");
    for (int i = 0; i < images.size(); ++i) {
        const QString name = QStringLiteral("xl/media/hfimage%1.png").arg(i + 1);
        parts.append({name, images[i], QStringLiteral("image/png"), -1, QString(), QString(), QString()});
        rels += QStringLiteral("<Relationship Id=\"rId%1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/%2\" Target=\"../media/hfimage%1.png\"/>").arg(i + 1).arg(relType);
    }
    rels += QStringLiteral("</Relationships>");
    parts.append({QStringLiteral("xl/drawings/_rels/vmlDrawing1.vml.rels"), rels.toUtf8(), QString(), -1, QString(), QString(), QString()});
    return withParts(buildXlsx({sheet}), parts);
}

} // namespace sheettest
