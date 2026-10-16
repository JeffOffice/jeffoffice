// The helpers declared in officehelpers.h.
#include "officehelpers.h"

namespace officehelp {

const QString kRelBase = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");

const QString kPresentationType = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml");

const QString kSlideType = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slide+xml");

QString rel(const QString &id, const QString &type, const QString &target, bool external)
{
    return QStringLiteral("<Relationship Id=\"%1\" Type=\"%2\" Target=\"%3\"%4/>")
        .arg(id, type, target, external ? QStringLiteral(" TargetMode=\"External\"") : QString());
}

QByteArray handWrittenTheme(const QString &a)
{
    return QStringLiteral(R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<a:theme xmlns:a="%1" name="Test Theme">
  <a:themeElements>
    <a:clrScheme name="Test Colors">
      <a:dk1><a:sysClr val="windowText" lastClr="101010"/></a:dk1>
      <a:lt1><a:sysClr val="window" lastClr="FEFEFE"/></a:lt1>
      <a:dk2><a:srgbClr val="44546A"/></a:dk2>
      <a:lt2><a:srgbClr val="E7E6E6"/></a:lt2>
      <a:accent1><a:srgbClr val="4472C4"/></a:accent1>
      <a:accent2><a:srgbClr val="ED7D31"/></a:accent2>
      <a:accent3><a:srgbClr val="A5A5A5"/></a:accent3>
      <a:accent4><a:srgbClr val="FFC000"/></a:accent4>
      <a:accent5><a:srgbClr val="5B9BD5"/></a:accent5>
      <a:accent6><a:srgbClr val="70AD47"/></a:accent6>
      <a:hlink><a:srgbClr val="0563C1"/></a:hlink>
      <a:folHlink><a:srgbClr val="954F72"/></a:folHlink>
    </a:clrScheme>
    <a:fontScheme name="Test Fonts">
      <a:majorFont>
        <a:latin typeface="Georgia" panose="02040502050405020303"/>
        <a:ea typeface="Noto Serif CJK JP"/>
        <a:cs typeface="Arial"/>
        <a:font script="Jpan" typeface="Yu Mincho"/>
      </a:majorFont>
      <a:minorFont>
        <a:latin typeface="Verdana"/>
        <a:ea typeface=""/>
        <a:cs typeface="Tahoma"/>
      </a:minorFont>
    </a:fontScheme>
    <a:fmtScheme name="Test Formats"><a:fillStyleLst><a:noFill/></a:fillStyleLst></a:fmtScheme>
  </a:themeElements>
  <a:objectDefaults/>
  <a:extraClrSchemeLst/>
</a:theme>)").arg(a).toUtf8();
}

const QColor kHandWrittenColors[Theme::ColorCount] = {
    QColor(0x10, 0x10, 0x10), QColor(0xFE, 0xFE, 0xFE), QColor(0x44, 0x54, 0x6A), QColor(0xE7, 0xE6, 0xE6),
    QColor(0x44, 0x72, 0xC4), QColor(0xED, 0x7D, 0x31), QColor(0xA5, 0xA5, 0xA5), QColor(0xFF, 0xC0, 0x00),
    QColor(0x5B, 0x9B, 0xD5), QColor(0x70, 0xAD, 0x47), QColor(0x05, 0x63, 0xC1), QColor(0x95, 0x4F, 0x72)};

Theme colorTestTheme()
{
    Theme t;
    for (int i = 0; i < Theme::ColorCount; ++i) t.colors[i] = kHandWrittenColors[i];
    return t;
}

QString hexOf(const QColor &c)
{
    return c.isValid() ? c.name(QColor::HexRgb).mid(1).toUpper() : QStringLiteral("(invalid)");
}

QString transforms(const QStringList &list)
{
    QString out;
    for (const QString &t : list) {
        const QStringList parts = t.split(QLatin1Char(' '));
        out += parts.size() > 1 ? QStringLiteral("<a:%1 val=\"%2\"/>").arg(parts[0], parts[1]) : QStringLiteral("<a:%1/>").arg(parts[0]);
    }
    return out;
}

QString schemeClr(const QString &val, const QStringList &list)
{
    return QStringLiteral("<a:schemeClr val=\"%1\">%2</a:schemeClr>").arg(val, transforms(list));
}

QColor colorOf(const QString &inner, const QColor &phClr, const ColorMap &map, const Theme &theme,
               const QString &a)
{
    QDomDocument doc;
    const QByteArray xml = QStringLiteral("<a:solidFill xmlns:a=\"%1\">%2</a:solidFill>").arg(a, inner).toUtf8();
    if (!parseXml(xml, &doc)) return QColor();
    return readColor(doc.documentElement(), theme, map, phClr);
}

QString dmlNamespaces()
{
    return QStringLiteral("xmlns:a=\"%1\" xmlns:p=\"%2\" xmlns:r=\"%3\"").arg(QString(ns::A), QString(ns::P), QString(ns::R));
}

QString emu(double points) { return QString::number(qint64(points * 12700)); }

bool near(double a, double b, double tolerance) { return qAbs(a - b) <= tolerance * qMax(1.0, qAbs(b)); }

QString dmlFormats()
{
    return QStringLiteral(
        "<a:fmtScheme name=\"T\"><a:fillStyleLst>"
        "<a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>"
        "<a:gradFill rotWithShape=\"1\"><a:gsLst><a:gs pos=\"0\"><a:schemeClr val=\"phClr\"><a:tint val=\"50000\"/></a:schemeClr></a:gs>"
        "<a:gs pos=\"100000\"><a:schemeClr val=\"phClr\"/></a:gs></a:gsLst><a:lin ang=\"5400000\" scaled=\"0\"/></a:gradFill>"
        "<a:solidFill><a:schemeClr val=\"phClr\"><a:lumMod val=\"50000\"/></a:schemeClr></a:solidFill></a:fillStyleLst>"
        "<a:lnStyleLst>"
        "<a:ln w=\"6350\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\"><a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill><a:prstDash val=\"solid\"/><a:miter lim=\"800000\"/></a:ln>"
        "<a:ln w=\"12700\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\"><a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill><a:prstDash val=\"dash\"/><a:round/></a:ln>"
        "<a:ln w=\"19050\" cap=\"rnd\" cmpd=\"dbl\" algn=\"ctr\"><a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill><a:prstDash val=\"solid\"/><a:bevel/></a:ln>"
        "</a:lnStyleLst><a:effectStyleLst>"
        "<a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle>"
        "<a:effectStyle><a:effectLst><a:outerShdw blurRad=\"57150\" dist=\"19050\" dir=\"5400000\" algn=\"ctr\" rotWithShape=\"0\">"
        "<a:srgbClr val=\"000000\"><a:alpha val=\"63000\"/></a:srgbClr></a:outerShdw></a:effectLst></a:effectStyle></a:effectStyleLst>"
        "<a:bgFillStyleLst><a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>"
        "<a:solidFill><a:schemeClr val=\"phClr\"><a:tint val=\"95000\"/></a:schemeClr></a:solidFill>"
        "<a:gradFill><a:gsLst><a:gs pos=\"0\"><a:schemeClr val=\"phClr\"/></a:gs><a:gs pos=\"100000\"><a:schemeClr val=\"phClr\"><a:shade val=\"80000\"/></a:schemeClr></a:gs></a:gsLst>"
        "<a:path path=\"circle\"><a:fillToRect l=\"50000\" t=\"50000\" r=\"50000\" b=\"50000\"/></a:path></a:gradFill></a:bgFillStyleLst></a:fmtScheme>");
}

QByteArray dmlPicture(const QColor &color)
{
    QImage img(4, 4, QImage::Format_RGB32);
    img.fill(color);
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG");
    return bytes;
}

QStringList dmlAllPresets()
{
    return QStringLiteral(
        "line lineInv triangle rtTriangle rect diamond parallelogram trapezoid nonIsoscelesTrapezoid pentagon hexagon heptagon octagon decagon "
        "dodecagon star4 star5 star6 star7 star8 star10 star12 star16 star24 star32 roundRect round1Rect round2SameRect round2DiagRect "
        "snipRoundRect snip1Rect snip2SameRect snip2DiagRect plaque ellipse teardrop homePlate chevron pieWedge pie blockArc donut noSmoking "
        "rightArrow leftArrow upArrow downArrow stripedRightArrow notchedRightArrow bentUpArrow leftRightArrow upDownArrow leftUpArrow "
        "leftRightUpArrow quadArrow leftArrowCallout rightArrowCallout upArrowCallout downArrowCallout leftRightArrowCallout upDownArrowCallout "
        "quadArrowCallout bentArrow uturnArrow circularArrow leftCircularArrow leftRightCircularArrow curvedRightArrow curvedLeftArrow "
        "curvedUpArrow curvedDownArrow swooshArrow cube can lightningBolt heart sun moon smileyFace irregularSeal1 irregularSeal2 foldedCorner "
        "bevel frame halfFrame corner diagStripe chord arc leftBracket rightBracket leftBrace rightBrace bracketPair bracePair straightConnector1 "
        "bentConnector2 bentConnector3 bentConnector4 bentConnector5 curvedConnector2 curvedConnector3 curvedConnector4 curvedConnector5 "
        "callout1 callout2 callout3 accentCallout1 accentCallout2 accentCallout3 borderCallout1 borderCallout2 borderCallout3 "
        "accentBorderCallout1 accentBorderCallout2 accentBorderCallout3 wedgeRectCallout wedgeRoundRectCallout wedgeEllipseCallout cloudCallout "
        "cloud ribbon ribbon2 ellipseRibbon ellipseRibbon2 leftRightRibbon verticalScroll horizontalScroll wave doubleWave plus "
        "flowChartProcess flowChartAlternateProcess flowChartDecision flowChartInputOutput flowChartPredefinedProcess flowChartInternalStorage "
        "flowChartDocument flowChartMultidocument flowChartTerminator flowChartPreparation flowChartManualInput flowChartManualOperation "
        "flowChartConnector flowChartOffpageConnector flowChartPunchedCard flowChartPunchedTape flowChartSummingJunction flowChartOr "
        "flowChartCollate flowChartSort flowChartExtract flowChartMerge flowChartOfflineStorage flowChartOnlineStorage flowChartMagneticTape "
        "flowChartMagneticDisk flowChartMagneticDrum flowChartDisplay flowChartDelay actionButtonBlank actionButtonHome actionButtonHelp "
        "actionButtonInformation actionButtonForwardNext actionButtonBackPrevious actionButtonEnd actionButtonBeginning actionButtonReturn "
        "actionButtonDocument actionButtonSound actionButtonMovie gear6 gear9 funnel mathPlus mathMinus mathMultiply mathDivide mathEqual "
        "mathNotEqual cornerTabs squareTabs plaqueTabs chartX chartStar chartPlus").split(QLatin1Char(' '), Qt::SkipEmptyParts);
}

QString dmlStyle(const QString &lnIdx, const QString &fillIdx, const QString &effectIdx, const QString &color)
{
    return QStringLiteral("<p:style><a:lnRef idx=\"%1\"><a:schemeClr val=\"%4\"/></a:lnRef><a:fillRef idx=\"%2\"><a:schemeClr val=\"%4\"/></a:fillRef>"
                          "<a:effectRef idx=\"%3\"><a:schemeClr val=\"%4\"/></a:effectRef><a:fontRef idx=\"minor\"><a:schemeClr val=\"lt1\"/></a:fontRef></p:style>")
        .arg(lnIdx, fillIdx, effectIdx, color);
}

QString dmlHex(const ColorRef &c) { return hexOf(c.rgbValue()); }

QVector<QDomElement> elementsNamed(const QDomElement &e, const char *local)
{
    QVector<QDomElement> out;
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        if (c.localName() == QLatin1String(local)) out << c;
        out += elementsNamed(c, local);
    }
    return out;
}

QString dmlTc(const QString &text, const QString &attrs, const QString &pr, const QString &prAttrs,
                     const QString &rPr)
{
    const QString run = text.isEmpty() ? QString() : QStringLiteral("<a:r><a:rPr lang=\"en-US\" %2/><a:t>%1</a:t></a:r>").arg(text, rPr);
    return QStringLiteral("<a:tc %1><a:txBody><a:bodyPr/><a:lstStyle/><a:p>%2</a:p></a:txBody><a:tcPr %3>%4</a:tcPr></a:tc>").arg(attrs, run, prAttrs, pr);
}

QString dmlTableFrame(const QString &tblPr, const QString &grid, const QString &rows, double x, double y, double w, double h)
{
    return QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"4\" name=\"Table 3\" descr=\"Figures\"/><p:cNvGraphicFramePr><a:graphicFrameLocks noGrp=\"1\"/></p:cNvGraphicFramePr>"
                          "<p:nvPr/></p:nvGraphicFramePr>%1<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/table\"><a:tbl>%2%3%4</a:tbl></a:graphicData></a:graphic></p:graphicFrame>")
        .arg(dmlXfrm(x, y, w, h, QString(), "p:xfrm"), tblPr, grid, rows);
}

QString dmlXfrm(double x, double y, double w, double h, const QString &attrs, const char *tag)
{
    return QStringLiteral("<%6 %5><a:off x=\"%1\" y=\"%2\"/><a:ext cx=\"%3\" cy=\"%4\"/></%6>")
        .arg(emu(x), emu(y), emu(w), emu(h), attrs, QString::fromLatin1(tag));
}

QString dmlSp(const QString &spPr, const QString &body, const QString &style, const QString &cNvSpPr,
                     const QString &nvPr, const QString &cNvPrInner, const QString &id)
{
    return QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%7\" name=\"Shape %7\" descr=\"alt %7\">%6</p:cNvPr><p:cNvSpPr %4/><p:nvPr>%5</p:nvPr></p:nvSpPr>"
                          "<p:spPr>%1</p:spPr>%3%2</p:sp>")
        .arg(spPr, body, style, cNvSpPr, nvPr, cNvPrInner, id);
}

QString dmlBody(const QString &text, const QString &bodyPr)
{
    return QStringLiteral("<p:txBody>%2<a:lstStyle/><a:p><a:r><a:t>%1</a:t></a:r></a:p></p:txBody>").arg(text, bodyPr);
}

QString dmlPic(const QString &blipInner, const QString &spPr, const QString &blipFillInner,
                      const QString &cNvPrInner, const QString &embed)
{
    return QStringLiteral("<p:pic><p:nvPicPr><p:cNvPr id=\"5\" name=\"Picture 5\" descr=\"a picture\">%4</p:cNvPr><p:cNvPicPr/><p:nvPr/></p:nvPicPr>"
                          "<p:blipFill><a:blip r:embed=\"%5\">%1</a:blip>%3</p:blipFill><p:spPr>%2</p:spPr></p:pic>")
        .arg(blipInner, spPr, blipFillInner, cNvPrInner, embed);
}

QString dmlGrp(const QString &grpSpPr, const QString &children, const QString &id)
{
    return QStringLiteral("<p:grpSp><p:nvGrpSpPr><p:cNvPr id=\"%3\" name=\"Group %3\" descr=\"group alt\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>"
                          "<p:grpSpPr>%1</p:grpSpPr>%2</p:grpSp>").arg(grpSpPr, children, id);
}

QString dmlGroupFrame(QRectF frame, QRectF child, const QString &attrs)
{
    return QStringLiteral("<a:xfrm %9><a:off x=\"%1\" y=\"%2\"/><a:ext cx=\"%3\" cy=\"%4\"/><a:chOff x=\"%5\" y=\"%6\"/><a:chExt cx=\"%7\" cy=\"%8\"/></a:xfrm>")
        .arg(emu(frame.x()), emu(frame.y()), emu(frame.width()), emu(frame.height()), emu(child.x()), emu(child.y()), emu(child.width()), emu(child.height()), attrs);
}

QString dmlSolid(const QString &hex) { return QStringLiteral("<a:solidFill><a:srgbClr val=\"%1\"/></a:solidFill>").arg(hex); }

ItemList dmlTree(DmlEnv &env, const QString &shapes, const TextStyleStack &styles)
{
    return readShapeTree(env.element(QStringLiteral("<p:spTree>%1</p:spTree>").arg(shapes)), env.ctx, styles);
}

ItemPtr dmlOne(DmlEnv &env, const QString &shape, const QVector<QString> &inherits)
{
    QVector<QDomElement> from;
    for (const QString &x : inherits) from.append(env.element(x));
    return readShape(env.element(shape), env.ctx, {}, from);
}

bool dmlRectIs(const QRectF &r, double x, double y, double w, double h, double tolerance)
{
    return near(r.x(), x, tolerance) && near(r.y(), y, tolerance) && near(r.width(), w, tolerance) && near(r.height(), h, tolerance);
}

QString dmlRectText(const QRectF &r) { return QStringLiteral("%1,%2 %3x%4").arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height()); }

QString run(const QString &text, const QString &attrs, const QString &children)
{
    return QStringLiteral("<a:r><a:rPr %1>%2</a:rPr><a:t>%3</a:t></a:r>").arg(attrs, children, text);
}

QString solid(const QString &hex) { return QStringLiteral("<a:solidFill><a:srgbClr val=\"%1\"/></a:solidFill>").arg(hex); }

} // namespace officehelp
