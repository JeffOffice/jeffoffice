// Workbooks with drawings, built in memory (never a real-world file): the anchors, a chart part that reads its series from cells, pictures and shapes, and the package that holds
// them, for the tests of the drawing reader, of the charts that follow their cells, and of the window that shows them.
#pragma once

#include "sheethelpers.h"

#include <QBuffer>
#include <QImage>

namespace drawtest {

inline const QString kXdr = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing");
inline const QString kDml = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/main");
inline const QString kChartNs = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/chart");

// <xdr:from> or <xdr:to>.
inline QString cellPoint(const char *tag, int col, int row, qint64 colOff = 0, qint64 rowOff = 0)
{
    return QStringLiteral("<xdr:%1><xdr:col>%2</xdr:col><xdr:colOff>%3</xdr:colOff><xdr:row>%4</xdr:row><xdr:rowOff>%5</xdr:rowOff></xdr:%1>").arg(QLatin1String(tag)).arg(col).arg(colOff).arg(row).arg(rowOff);
}
inline QString twoCell(int col1, int row1, int col2, int row2, const QString &object, const QString &editAs = QString(), qint64 colOff1 = 0, qint64 rowOff1 = 0, qint64 colOff2 = 0, qint64 rowOff2 = 0)
{
    return QStringLiteral("<xdr:twoCellAnchor%1>").arg(editAs.isEmpty() ? QString() : QStringLiteral(" editAs=\"%1\"").arg(editAs)) + cellPoint("from", col1, row1, colOff1, rowOff1)
           + cellPoint("to", col2, row2, colOff2, rowOff2) + object + QStringLiteral("<xdr:clientData/></xdr:twoCellAnchor>");
}
inline QString oneCell(int col, int row, qint64 cx, qint64 cy, const QString &object, qint64 colOff = 0, qint64 rowOff = 0)
{
    return QStringLiteral("<xdr:oneCellAnchor>") + cellPoint("from", col, row, colOff, rowOff) + QStringLiteral("<xdr:ext cx=\"%1\" cy=\"%2\"/>").arg(cx).arg(cy) + object
           + QStringLiteral("<xdr:clientData/></xdr:oneCellAnchor>");
}
inline QString absolute(qint64 x, qint64 y, qint64 cx, qint64 cy, const QString &object)
{
    return QStringLiteral("<xdr:absoluteAnchor><xdr:pos x=\"%1\" y=\"%2\"/><xdr:ext cx=\"%3\" cy=\"%4\"/>").arg(x).arg(y).arg(cx).arg(cy) + object + QStringLiteral("<xdr:clientData/></xdr:absoluteAnchor>");
}

inline QString chartFrame(int id, const QString &rId, const QString &name = QStringLiteral("Chart"))
{
    return QStringLiteral("<xdr:graphicFrame macro=\"\"><xdr:nvGraphicFramePr><xdr:cNvPr id=\"%1\" name=\"%2\"/><xdr:cNvGraphicFramePr/></xdr:nvGraphicFramePr>"
                          "<xdr:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/></xdr:xfrm><a:graphic><a:graphicData uri=\"%3\"><c:chart xmlns:c=\"%3\" r:id=\"%4\"/></a:graphicData></a:graphic></xdr:graphicFrame>")
        .arg(id).arg(name, kChartNs, rId);
}
inline QString pictureElement(int id, const QString &rId, const QString &name = QStringLiteral("Picture"), const QString &blipExtra = QString())
{
    return QStringLiteral("<xdr:pic><xdr:nvPicPr><xdr:cNvPr id=\"%1\" name=\"%2\" descr=\"a picture\"/><xdr:cNvPicPr/></xdr:nvPicPr><xdr:blipFill><a:blip r:embed=\"%3\"/>%4<a:stretch><a:fillRect/></a:stretch></xdr:blipFill>"
                          "<xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"952500\" cy=\"476250\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></xdr:spPr></xdr:pic>")
        .arg(id).arg(name, rId, blipExtra);
}
// A rectangle (or another preset) with a fill, and text if given.
inline QString shapeElement(int id, const QString &name, const QString &prst = QStringLiteral("rect"), const QString &text = QString(), const QString &fill = QStringLiteral("3366CC"), qint64 cx = 952500, qint64 cy = 476250)
{
    return QStringLiteral("<xdr:sp macro=\"\" textlink=\"\"><xdr:nvSpPr><xdr:cNvPr id=\"%1\" name=\"%2\"/><xdr:cNvSpPr/></xdr:nvSpPr><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"%6\" cy=\"%7\"/></a:xfrm>"
                          "<a:prstGeom prst=\"%3\"><a:avLst/></a:prstGeom><a:solidFill><a:srgbClr val=\"%4\"/></a:solidFill></xdr:spPr>%5</xdr:sp>")
        .arg(id).arg(name, prst, fill, text.isEmpty() ? QString() : QStringLiteral("<xdr:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:t>%1</a:t></a:r></a:p></xdr:txBody>").arg(text.toHtmlEscaped())).arg(cx).arg(cy);
}
inline QString groupElement(int id, const QString &inner, qint64 cx = 1905000, qint64 cy = 476250)
{
    return QStringLiteral("<xdr:grpSp><xdr:nvGrpSpPr><xdr:cNvPr id=\"%1\" name=\"Group\"/><xdr:cNvGrpSpPr/></xdr:nvGrpSpPr><xdr:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"%2\" cy=\"%3\"/>"
                          "<a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"%2\" cy=\"%3\"/></a:xfrm></xdr:grpSpPr>%4</xdr:grpSp>")
        .arg(id).arg(cx).arg(cy).arg(inner);
}
inline QString connectorElement(int id)
{
    return QStringLiteral("<xdr:cxnSp macro=\"\"><xdr:nvCxnSpPr><xdr:cNvPr id=\"%1\" name=\"Connector\"/><xdr:cNvCxnSpPr/></xdr:nvCxnSpPr><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"952500\" cy=\"476250\"/></a:xfrm>"
                          "<a:prstGeom prst=\"line\"><a:avLst/></a:prstGeom><a:ln w=\"19050\"><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill></a:ln></xdr:spPr></xdr:cxnSp>")
        .arg(id);
}

inline QByteArray drawingPart(const QString &anchors)
{
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<xdr:wsDr xmlns:xdr=\"%1\" xmlns:a=\"%2\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">%3</xdr:wsDr>")
        .arg(kXdr, kDml, anchors).toUtf8();
}

// One series of a chart part: where it reads (the c:f of its name, categories, and values, which may be empty) and the numbers stored for it.
struct SeriesSpec {
    QString nameRef, catRef, valRef;
    QString name;
    QStringList categories;
    QVector<double> values;
    QString xRef;   // a scatter's x values (stored in `categories` as numbers)
};

inline QString cacheOfStrings(const QStringList &items)
{
    QString pts;
    for (int i = 0; i < items.size(); ++i) pts += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(items[i].toHtmlEscaped());
    return QStringLiteral("<c:ptCount val=\"%1\"/>").arg(items.size()) + pts;
}
inline QString cacheOfNumbers(const QVector<double> &items, const QString &format = QStringLiteral("General"))
{
    QString pts;
    for (int i = 0; i < items.size(); ++i) pts += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(items[i], 0, 'g', 17);
    return QStringLiteral("<c:formatCode>%1</c:formatCode><c:ptCount val=\"%2\"/>").arg(format).arg(items.size()) + pts;
}

inline QString seriesElement(int index, const SeriesSpec &s, bool scatter = false)
{
    QString out = QStringLiteral("<c:ser><c:idx val=\"%1\"/><c:order val=\"%1\"/>").arg(index);
    if (!s.nameRef.isEmpty() || !s.name.isEmpty()) {
        if (s.nameRef.isEmpty()) out += QStringLiteral("<c:tx><c:v>%1</c:v></c:tx>").arg(s.name.toHtmlEscaped());
        else out += QStringLiteral("<c:tx><c:strRef><c:f>%1</c:f><c:strCache>%2</c:strCache></c:strRef></c:tx>").arg(s.nameRef.toHtmlEscaped(), cacheOfStrings({s.name}));
    }
    if (scatter) {
        QVector<double> xs;
        for (const QString &c : s.categories) xs << c.toDouble();
        out += QStringLiteral("<c:xVal><c:numRef><c:f>%1</c:f><c:numCache>%2</c:numCache></c:numRef></c:xVal>").arg(s.xRef.toHtmlEscaped(), cacheOfNumbers(xs));
        out += QStringLiteral("<c:yVal><c:numRef><c:f>%1</c:f><c:numCache>%2</c:numCache></c:numRef></c:yVal>").arg(s.valRef.toHtmlEscaped(), cacheOfNumbers(s.values));
    } else {
        if (!s.catRef.isEmpty() || !s.categories.isEmpty())
            out += QStringLiteral("<c:cat><c:strRef><c:f>%1</c:f><c:strCache>%2</c:strCache></c:strRef></c:cat>").arg(s.catRef.toHtmlEscaped(), cacheOfStrings(s.categories));
        out += QStringLiteral("<c:val><c:numRef><c:f>%1</c:f><c:numCache>%2</c:numCache></c:numRef></c:val>").arg(s.valRef.toHtmlEscaped(), cacheOfNumbers(s.values));
    }
    return out + QStringLiteral("</c:ser>");
}

// A chart part: "barChart", "lineChart", "pieChart", "areaChart", or "scatterChart", the series, the title if any, and c:plotVisOnly and c:dispBlanksAs.
inline QByteArray chartPart(const QString &kind, const QVector<SeriesSpec> &series, const QString &title = QString(), bool plotVisOnly = true, const QString &blanksAs = QStringLiteral("gap"))
{
    const bool scatter = kind == QLatin1String("scatterChart");
    QString plot = QStringLiteral("<c:%1>").arg(kind);
    if (kind == QLatin1String("barChart")) plot += QStringLiteral("<c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>");
    else if (kind == QLatin1String("lineChart") || kind == QLatin1String("areaChart")) plot += QStringLiteral("<c:grouping val=\"standard\"/>");
    else if (scatter) plot += QStringLiteral("<c:scatterStyle val=\"lineMarker\"/>");
    plot += QStringLiteral("<c:varyColors val=\"0\"/>");
    for (int i = 0; i < series.size(); ++i) plot += seriesElement(i, series[i], scatter);
    const bool pie = kind == QLatin1String("pieChart");
    if (!pie) plot += QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/>");
    plot += QStringLiteral("</c:%1>").arg(kind);
    if (!pie) {
        const QString first = scatter ? QStringLiteral("valAx") : QStringLiteral("catAx");
        plot += QStringLiteral("<c:%1><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:crossAx val=\"222\"/></c:%1>").arg(first);
        plot += QStringLiteral("<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorGridlines/><c:crossAx val=\"111\"/></c:valAx>");
    }
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<c:chartSpace xmlns:c=\"%1\" xmlns:a=\"%2\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
                          "<c:chart>%3<c:plotArea><c:layout/>%4</c:plotArea><c:legend><c:legendPos val=\"r\"/></c:legend><c:plotVisOnly val=\"%5\"/><c:dispBlanksAs val=\"%6\"/></c:chart></c:chartSpace>")
        .arg(kChartNs, kDml,
             title.isEmpty() ? QStringLiteral("<c:autoTitleDeleted val=\"1\"/>") : QStringLiteral("<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:r><a:t>%1</a:t></a:r></a:p></c:rich></c:tx></c:title>").arg(title.toHtmlEscaped()), plot)
        .arg(plotVisOnly ? 1 : 0)
        .arg(blanksAs)
        .toUtf8();
}

// A small picture as PNG bytes: `w` by `h` pixels of one color.
inline QByteArray pngBytes(int w, int h, const QColor &color)
{
    QImage img(w, h, QImage::Format_ARGB32);
    img.fill(color);
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG");
    return bytes;
}

// A part of the drawing's: a chart or a picture the drawing names by `rId`.
struct DrawingTarget {
    QString rId;
    QString name;          // the part's name in the package ("xl/charts/chart1.xml")
    QByteArray data;
    QString contentType;
    QString relType;       // "chart" or "image"
};

// The workbook: `sheets` are the sheet parts (the first has the drawing), `drawing` is the drawing part, `targets` the parts it relates to. The drawing is related to the first sheet by `<drawing r:id>`.
inline QByteArray bookWithDrawing(QVector<QByteArray> sheets, const QByteArray &drawing, const QVector<DrawingTarget> &targets, const QStringList &sheetNames = QStringList(), const QString &definedNames = QString(),
                                  const QByteArray &styles = QByteArray())
{
    sheets[0].replace("</worksheet>", "<drawing r:id=\"rIdD1\"/></worksheet>");
    QVector<sheettest::ExtraPart> parts;
    parts.append({QStringLiteral("xl/drawings/drawing1.xml"), drawing, QStringLiteral("application/vnd.openxmlformats-officedocument.drawing+xml"), 0, QStringLiteral("rIdD1"), QStringLiteral("drawing"),
                  QStringLiteral("../drawings/drawing1.xml")});
    QString rels = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">");
    for (const DrawingTarget &t : targets) {
        parts.append({t.name, t.data, t.contentType, -1, QString(), QString(), QString()});
        rels += QStringLiteral("<Relationship Id=\"%1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/%2\" Target=\"/%3\"/>").arg(t.rId, t.relType, t.name);
    }
    rels += QStringLiteral("</Relationships>");
    parts.append({QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"), rels.toUtf8(), QString(), -1, QString(), QString(), QString()});
    return sheettest::withParts(sheettest::buildXlsx(sheets, QByteArray(), QString(), QString(), styles, sheetNames, definedNames), parts);
}

inline DrawingTarget chartTarget(const QString &rId, int n, const QByteArray &data)
{
    return {rId, QStringLiteral("xl/charts/chart%1.xml").arg(n), data, QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"), QStringLiteral("chart")};
}
inline DrawingTarget imageTarget(const QString &rId, int n, const QByteArray &png)
{
    return {rId, QStringLiteral("xl/media/image%1.png").arg(n), png, QStringLiteral("image/png"), QStringLiteral("image")};
}

// A worksheet whose cells A1 to D5 hold what the rows say: row 1 the headers ("Region", "East", "West"), then three rows of a label and two numbers.
inline QByteArray dataSheet()
{
    auto text = [](const QString &ref, const QString &t) { return QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, t.toHtmlEscaped()); };
    auto num = [](const QString &ref, double v) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref).arg(v, 0, 'g', 15); };
    QString rows = QStringLiteral("<row r=\"1\">") + text("A1", "Region") + text("B1", "East") + text("C1", "West") + QStringLiteral("</row>");
    const char *labels[] = {"Q1", "Q2", "Q3"};
    for (int i = 0; i < 3; ++i)
        rows += QStringLiteral("<row r=\"%1\">%2%3%4</row>").arg(i + 2).arg(text(QStringLiteral("A%1").arg(i + 2), QLatin1String(labels[i])), num(QStringLiteral("B%1").arg(i + 2), 10 * (i + 1)),
                                                                           num(QStringLiteral("C%1").arg(i + 2), 5 * (i + 1)));
    return sheettest::worksheet(rows);
}

} // namespace drawtest
