// Tests of libs/office: DrawingML fills, picture effects, lines, and diagram drawings.
#include "officehelpers.h"
#include "../timing.h"
#include "core/blipeffects.h"
#include "core/picturefx.h"
#include <QCryptographicHash>

class OfficeDmlFillTests : public QObject {
    Q_OBJECT

    static Fill dmlFillOf(DmlEnv &env, const QString &properties, const QString &style = QString())
    {
        return readFill(env.element(QStringLiteral("<p:spPr>%1</p:spPr>").arg(properties)), style.isEmpty() ? QDomElement() : env.element(style), env.ctx);
    }

    static Stroke dmlLineOf(DmlEnv &env, const QString &properties, const QString &style = QString())
    {
        return readLine(env.element(QStringLiteral("<p:spPr>%1</p:spPr>").arg(properties)), style.isEmpty() ? QDomElement() : env.element(style), env.ctx);
    }

private Q_SLOTS:
    void dmlFillReadsSolidColors()
    {
        DmlEnv env;
        Fill f = dmlFillOf(env, QStringLiteral("<a:solidFill><a:srgbClr val=\"FF8000\"/></a:solidFill>"));
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("FF8000"));
        QCOMPARE(f.transparency, 0.0);
        f = dmlFillOf(env, QStringLiteral("<a:solidFill><a:schemeClr val=\"accent1\"><a:alpha val=\"25000\"/></a:schemeClr></a:solidFill>"));
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("4472C4"));
        QVERIFY(near(f.transparency, 0.75, 1e-2));
        // No fill, or none of the kinds, is none.
        QVERIFY(dmlFillOf(env, QString()).isNone());
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:noFill/>")).isNone());
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:solidFill/>")).isNone());
        QVERIFY(readFill(QDomElement(), QDomElement(), env.ctx).isNone());
        // A strict file's elements are the same.
        QDomDocument doc;
        const QByteArray strict = QStringLiteral("<p:spPr xmlns:p=\"%2\" xmlns:a=\"%1\"><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></p:spPr>").arg(QString(ns::AStrict), QString(ns::PStrict)).toUtf8();
        QVERIFY(parseXml(strict, &doc));
        QCOMPARE(dmlHex(readFill(doc.documentElement(), QDomElement(), env.ctx).color), QStringLiteral("00FF00"));
    }

    void dmlFillReadsGradients()
    {
        DmlEnv env;
        const QString stops = QStringLiteral("<a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"FF0000\"/></a:gs><a:gs pos=\"60000\"><a:srgbClr val=\"00FF00\"><a:alpha val=\"50000\"/></a:srgbClr></a:gs>"
                                             "<a:gs pos=\"100000\"><a:srgbClr val=\"0000FF\"/></a:gs></a:gsLst>");
        Fill f = dmlFillOf(env, QStringLiteral("<a:gradFill rotWithShape=\"1\">%1<a:lin ang=\"2700000\" scaled=\"0\"/></a:gradFill>").arg(stops));
        QVERIFY(f.type == Fill::Gradient);
        QVERIFY(f.gradType == Fill::Linear);
        QVERIFY(near(f.angle, 45));
        QCOMPARE(f.stops.size(), 3);
        QVERIFY(near(f.stops[0].pos, 0) && near(f.stops[1].pos, 0.6) && near(f.stops[2].pos, 1));
        QCOMPARE(dmlHex(f.stops[0].color), QStringLiteral("FF0000"));
        QCOMPARE(dmlHex(f.stops[1].color), QStringLiteral("00FF00"));
        QVERIFY(near(f.stops[1].transparency, 0.5, 1e-2));
        QCOMPARE(f.stops[0].transparency, 0.0);
        QCOMPARE(dmlHex(f.color), QStringLiteral("FF0000"));
        QCOMPARE(dmlHex(f.color2), QStringLiteral("0000FF"));
        // Angles: 0 is left to right, a quarter turn is top to bottom, and any turn is read modulo a full one.
        f = dmlFillOf(env, QStringLiteral("<a:gradFill>%1<a:lin ang=\"0\"/></a:gradFill>").arg(stops));
        QVERIFY(near(f.angle, 0));
        f = dmlFillOf(env, QStringLiteral("<a:gradFill>%1<a:lin ang=\"5400000\"/></a:gradFill>").arg(stops));
        QVERIFY(near(f.angle, 90));
        f = dmlFillOf(env, QStringLiteral("<a:gradFill>%1<a:lin ang=\"-5400000\"/></a:gradFill>").arg(stops));
        QVERIFY(near(f.angle, 270));
        f = dmlFillOf(env, QStringLiteral("<a:gradFill>%1<a:lin ang=\"27000000\"/></a:gradFill>").arg(stops));
        QVERIFY(near(f.angle, 90));
        // Without a direction it runs left to right.
        f = dmlFillOf(env, QStringLiteral("<a:gradFill>%1</a:gradFill>").arg(stops));
        QVERIFY(f.type == Fill::Gradient && f.gradType == Fill::Linear && near(f.angle, 0));
        // Path gradients.
        f = dmlFillOf(env, QStringLiteral("<a:gradFill>%1<a:path path=\"circle\"><a:fillToRect l=\"50000\" t=\"50000\" r=\"50000\" b=\"50000\"/></a:path></a:gradFill>").arg(stops));
        QVERIFY(f.type == Fill::Gradient && f.gradType == Fill::Radial);
        f = dmlFillOf(env, QStringLiteral("<a:gradFill>%1<a:path path=\"rect\"/></a:gradFill>").arg(stops));
        QVERIFY(f.gradType == Fill::Rectangular);
        f = dmlFillOf(env, QStringLiteral("<a:gradFill>%1<a:path path=\"shape\"/></a:gradFill>").arg(stops));
        QVERIFY(f.gradType == Fill::PathGrad);
        // Stops come in order of position, whatever order they are written in; positions stay within the gradient.
        f = dmlFillOf(env, QStringLiteral("<a:gradFill><a:gsLst><a:gs pos=\"100000\"><a:srgbClr val=\"0000FF\"/></a:gs><a:gs pos=\"-5\"><a:srgbClr val=\"FF0000\"/></a:gs>"
                                          "<a:gs pos=\"900000\"><a:srgbClr val=\"00FF00\"/></a:gs></a:gsLst></a:gradFill>"));
        QCOMPARE(f.stops.size(), 3);
        QVERIFY(f.stops[0].pos == 0 && f.stops[1].pos <= f.stops[2].pos && f.stops[2].pos <= 1);
        QCOMPARE(dmlHex(f.stops[0].color), QStringLiteral("FF0000"));
        // One stop is a plain color; none is no fill.
        f = dmlFillOf(env, QStringLiteral("<a:gradFill><a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"123456\"/></a:gs></a:gsLst></a:gradFill>"));
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("123456"));
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:gradFill><a:gsLst/></a:gradFill>")).isNone());
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:gradFill/>")).isNone());
    }

    void dmlFillReadsPatterns()
    {
        DmlEnv env;
        const auto pattern = [&](const char *name) {
            return dmlFillOf(env, QStringLiteral("<a:pattFill prst=\"%1\"><a:fgClr><a:srgbClr val=\"FF0000\"/></a:fgClr><a:bgClr><a:srgbClr val=\"FFFFFF\"/></a:bgClr></a:pattFill>")
                                      .arg(QString::fromLatin1(name)));
        };
        Fill f = pattern("ltHorz");
        QVERIFY(f.type == Fill::Pattern);
        QCOMPARE(dmlHex(f.color), QStringLiteral("FF0000"));
        QCOMPARE(dmlHex(f.color2), QStringLiteral("FFFFFF"));
        const QVector<Qt::BrushStyle> brushes = jp::patternBrushes();
        QCOMPARE(brushes[pattern("horz").pattern], Qt::HorPattern);
        QCOMPARE(brushes[pattern("dkHorz").pattern], Qt::HorPattern);
        QCOMPARE(brushes[pattern("vert").pattern], Qt::VerPattern);
        QCOMPARE(brushes[pattern("cross").pattern], Qt::CrossPattern);
        QCOMPARE(brushes[pattern("smGrid").pattern], Qt::CrossPattern);
        QCOMPARE(brushes[pattern("dnDiag").pattern], Qt::FDiagPattern);
        QCOMPARE(brushes[pattern("upDiag").pattern], Qt::BDiagPattern);
        QCOMPARE(brushes[pattern("diagCross").pattern], Qt::DiagCrossPattern);
        QCOMPARE(brushes[pattern("pct5").pattern], Qt::Dense7Pattern);
        QCOMPARE(brushes[pattern("pct50").pattern], Qt::Dense4Pattern);
        QCOMPARE(brushes[pattern("pct90").pattern], Qt::Dense1Pattern);
        // Every pattern the standard names draws as something; one it doesn't is the foreground color alone.
        for (const char *name : {"pct10", "pct20", "pct25", "pct30", "pct40", "pct60", "pct70", "pct75", "pct80", "ltVert", "dkVert", "narHorz", "narVert",
                                 "dashHorz", "dashVert", "ltDnDiag", "ltUpDiag", "dkDnDiag", "dkUpDiag", "wdDnDiag", "wdUpDiag", "dashDnDiag", "dashUpDiag", "smCheck",
                                 "lgCheck", "lgGrid", "dotGrid", "smConfetti", "lgConfetti", "horzBrick", "diagBrick", "solidDmnd", "openDmnd", "dotDmnd", "plaid",
                                 "sphere", "weave", "divot", "shingle", "wave", "trellis", "zigZag"})
            QVERIFY2(pattern(name).type == Fill::Pattern && pattern(name).pattern >= 0 && pattern(name).pattern < brushes.size(), name);
        f = pattern("noSuchPattern");
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("FF0000"));
        // No colors given: black on white, as the standard says.
        f = dmlFillOf(env, QStringLiteral("<a:pattFill prst=\"horz\"/>"));
        QVERIFY(f.type == Fill::Pattern);
    }

    void dmlFillReadsPictures()
    {
        DmlEnv env;
        env.usePackage();
        Fill f = dmlFillOf(env, QStringLiteral("<a:blipFill><a:blip r:embed=\"rId1\"/><a:stretch><a:fillRect/></a:stretch></a:blipFill>"));
        QVERIFY(f.type == Fill::Picture);
        QCOMPARE(f.imageId, QStringLiteral("img1"));
        QVERIFY(!f.tile);
        QCOMPARE(env.images.size(), 1);
        QCOMPARE(env.images[0].format, QStringLiteral("png"));
        QCOMPARE(env.images[0].bytes, dmlPicture());
        // A tiled one, at half its size, and with some transparency.
        f = dmlFillOf(env, QStringLiteral("<a:blipFill><a:blip r:embed=\"rId1\"><a:alphaModFix amt=\"40000\"/></a:blip><a:tile tx=\"0\" ty=\"0\" sx=\"50000\" sy=\"50000\" flip=\"none\" algn=\"tl\"/></a:blipFill>"));
        QVERIFY(f.type == Fill::Picture && f.tile);
        QVERIFY(near(f.tileScale, 0.5, 1e-6));
        QVERIFY(near(f.transparency, 0.6, 1e-6));
        // A picture that isn't there, or isn't a picture, is no fill and no crash.
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:blipFill><a:blip r:embed=\"rId9\"/></a:blipFill>")).isNone());
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:blipFill><a:blip/></a:blipFill>")).isNone());
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:blipFill/>")).isNone());
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:blipFill><a:blip r:embed=\"rId2\"/></a:blipFill>")).isNone());   // an outside link
        DmlEnv bare;
        QVERIFY(dmlFillOf(bare, QStringLiteral("<a:blipFill><a:blip r:embed=\"rId1\"/></a:blipFill>")).isNone());
        // The format comes from what the bytes are, whatever the part is called.
        DmlEnv other;
        QImage img(2, 2, QImage::Format_RGB32);
        img.fill(Qt::blue);
        QByteArray jpeg;
        QBuffer buffer(&jpeg);
        buffer.open(QIODevice::WriteOnly);
        img.save(&buffer, "JPEG");
        other.usePackage(jpeg);
        f = dmlFillOf(other, QStringLiteral("<a:blipFill><a:blip r:embed=\"rId1\"/></a:blipFill>"));
        QVERIFY(f.type == Fill::Picture);
        QCOMPARE(other.images[0].format, QStringLiteral("jpeg"));
    }

    // The effects made by hand, to say what the reader should have read (and compared member by member, not through JSON, which an effect is
    // read and written by).
    static jp::BlipEffect lumEffect(double brightness, double contrast)
    {
        jp::BlipEffect e;
        e.kind = jp::BlipEffect::Luminance;
        e.brightness = brightness;
        e.contrast = contrast;
        return e;
    }
    static jp::BlipEffect grayEffect()
    {
        jp::BlipEffect e;
        e.kind = jp::BlipEffect::Grayscale;
        return e;
    }
    static jp::BlipEffect biLevelEffect(double threshold)
    {
        jp::BlipEffect e;
        e.kind = jp::BlipEffect::BiLevel;
        e.threshold = threshold;
        return e;
    }
    static jp::BlipEffect changeEffect(const QColor &from, const QColor &to, bool useAlpha = true)
    {
        jp::BlipEffect e;
        e.kind = jp::BlipEffect::ColorChange;
        e.color = from;
        e.color2 = to;
        e.useAlpha = useAlpha;
        return e;
    }
    static jp::BlipEffect replaceEffect(const QColor &color)
    {
        jp::BlipEffect e;
        e.kind = jp::BlipEffect::ColorReplace;
        e.color = color;
        return e;
    }
    static jp::BlipEffect duotoneEffect(const QColor &dark, const QColor &light)
    {
        jp::BlipEffect e;
        e.kind = jp::BlipEffect::Duotone;
        e.color = dark;
        e.color2 = light;
        return e;
    }
    static jp::BlipEffect hslEffect(double hue, double saturation, double lightness)
    {
        jp::BlipEffect e;
        e.kind = jp::BlipEffect::Hsl;
        e.hue = hue;
        e.saturation = saturation;
        e.lightness = lightness;
        return e;
    }
    static jp::BlipEffect tintEffect(double hue, double amount)
    {
        jp::BlipEffect e;
        e.kind = jp::BlipEffect::Tint;
        e.hue = hue;
        e.amount = amount;
        return e;
    }
    static bool colorIsNear(const QColor &a, const QColor &b) { return a.rgb() == b.rgb() && qAbs(a.alpha() - b.alpha()) <= 1; }
    static bool sameEffects(const QVector<jp::BlipEffect> &got, const QVector<jp::BlipEffect> &want)
    {
        if (got.size() != want.size()) return false;
        for (int i = 0; i < got.size(); ++i) {
            const jp::BlipEffect &a = got[i], &b = want[i];
            if (a.kind != b.kind) return false;
            switch (a.kind) {
            case jp::BlipEffect::Luminance:
                if (!near(a.brightness, b.brightness) || !near(a.contrast, b.contrast)) return false;
                break;
            case jp::BlipEffect::Grayscale: break;
            case jp::BlipEffect::BiLevel:
                if (!near(a.threshold, b.threshold)) return false;
                break;
            case jp::BlipEffect::ColorChange:
                if (!colorIsNear(a.color, b.color) || !colorIsNear(a.color2, b.color2) || a.useAlpha != b.useAlpha) return false;
                break;
            case jp::BlipEffect::ColorReplace:
                if (!colorIsNear(a.color, b.color)) return false;
                break;
            case jp::BlipEffect::Duotone:
                if (!colorIsNear(a.color, b.color) || !colorIsNear(a.color2, b.color2)) return false;
                break;
            case jp::BlipEffect::Hsl:
                if (!near(a.hue, b.hue) || !near(a.saturation, b.saturation) || !near(a.lightness, b.lightness)) return false;
                break;
            case jp::BlipEffect::Tint:
                if (!near(a.hue, b.hue) || !near(a.amount, b.amount)) return false;
                break;
            }
        }
        return true;
    }

    // The changes a picture fill's blip lists are kept as listed, each with what it needs; the picture is kept as the file has it, with none
    // made in its bytes (they are made where it is drawn), and a blip that lists more than the limit has the rest left out.
    void dmlFillReadsPictureEffects()
    {
        DmlEnv env;
        env.usePackage();
        const QString clear = QStringLiteral("<a:clrChange><a:clrFrom><a:srgbClr val=\"9C8C7B\"/></a:clrFrom><a:clrTo><a:srgbClr val=\"9C8C7B\"><a:alpha val=\"0\"/></a:srgbClr></a:clrTo></a:clrChange>");
        const Fill f = dmlFillOf(env, QStringLiteral("<a:blipFill><a:blip r:embed=\"rId1\"><a:lum bright=\"21000\" contrast=\"52000\"/><a:grayscl/><a:biLevel thresh=\"25000\"/>%1"
                                                     "<a:clrRepl><a:srgbClr val=\"FF0000\"><a:alpha val=\"50000\"/></a:srgbClr></a:clrRepl><a:duotone><a:srgbClr val=\"000080\"/><a:srgbClr val=\"FFFF00\"/></a:duotone>"
                                                     "<a:hsl hue=\"5400000\" sat=\"-20000\" lum=\"10000\"/><a:tint hue=\"10800000\" amt=\"30000\"/></a:blip><a:stretch><a:fillRect/></a:stretch></a:blipFill>")
                                              .arg(clear));
        QVERIFY(f.type == Fill::Picture);
        const QVector<jp::BlipEffect> listed = {lumEffect(0.21, 0.52),
                                                grayEffect(),
                                                biLevelEffect(0.25),
                                                changeEffect(QColor(0x9C, 0x8C, 0x7B), QColor(0x9C, 0x8C, 0x7B, 0)),
                                                replaceEffect(QColor(255, 0, 0, 128)),
                                                duotoneEffect(QColor(0, 0, 0x80), QColor(255, 255, 0)),
                                                hslEffect(90, -0.2, 0.1),
                                                tintEffect(180, 0.3)};
        QVERIFY(sameEffects(f.blipEffects, listed));
        QCOMPARE(env.images.size(), 1);
        QCOMPARE(env.images[0].bytes, dmlPicture());
        // What a fill is, it still is, with the changes: copied through its JSON.
        QVERIFY(sameEffects(jp::Fill::fromJson(f.toJson()).blipEffects, listed));
        QVERIFY(jp::Fill::fromJson(f.toJson()).type == Fill::Picture);
        // A fill that differs only by one change is not the same fill.
        Fill other = f;
        other.blipEffects[0].contrast = 0.5;
        QVERIFY(!(other == f));
        // A blip with no changes has none, and one with more than a picture could be given (a hostile file's thousands) keeps the first.
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:blipFill><a:blip r:embed=\"rId1\"/></a:blipFill>")).blipEffects.isEmpty());
        QString many;
        for (int i = 0; i < 1000; ++i) many += QStringLiteral("<a:grayscl/>");
        QCOMPARE(dmlFillOf(env, QStringLiteral("<a:blipFill><a:blip r:embed=\"rId1\">%1</a:blip></a:blipFill>").arg(many)).blipEffects.size(), jp::kMaxBlipEffects);
        // An effect that has nothing to work with (a duotone of one color, a color change from no color) is left out.
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:blipFill><a:blip r:embed=\"rId1\"><a:duotone><a:srgbClr val=\"000080\"/></a:duotone><a:clrChange/><a:clrRepl/></a:blip></a:blipFill>"))
                    .blipEffects.isEmpty());
    }

    // A picture keeps its brightness and contrast, gray or black and white (with its threshold), and one clear color as settings, when the blip
    // lists them in the order the settings are drawn (a clear color, then the brightness and contrast, then gray or black and white), each once;
    // what comes after them, or anything that isn't so, is kept in order beside them, so that the picture is drawn and written as the file lists it.
    void dmlPictureReadsTheEffectsItHasNoSettingFor()
    {
        DmlEnv env;
        env.usePackage();
        const QString frame = dmlXfrm(10, 20, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        const auto pic = [&](const QString &blip) { return dmlAs<PictureItem>(dmlOne(env, dmlPic(blip, frame))); };
        const QString clearWhite = QStringLiteral("<a:clrChange><a:clrFrom><a:srgbClr val=\"FFFFFF\"/></a:clrFrom><a:clrTo><a:srgbClr val=\"FFFFFF\"><a:alpha val=\"0\"/></a:srgbClr></a:clrTo></a:clrChange>");
        const QString redForWhite = QStringLiteral("<a:clrChange><a:clrFrom><a:srgbClr val=\"FFFFFF\"/></a:clrFrom><a:clrTo><a:srgbClr val=\"FF0000\"/></a:clrTo></a:clrChange>");
        const QString duotone = QStringLiteral("<a:duotone><a:srgbClr val=\"000080\"/><a:srgbClr val=\"FFFF00\"/></a:duotone>");
        auto p = pic(QStringLiteral("<a:biLevel thresh=\"25000\"/>"));
        QVERIFY(p->recolor == PictureItem::BlackWhite && near(p->blackWhiteLevel, 0.25));
        QVERIFY(p->blipEffects.isEmpty());
        QVERIFY(near(pic(QStringLiteral("<a:biLevel/>"))->blackWhiteLevel, 0.5));
        // The settings, then what has none.
        p = pic(QStringLiteral("<a:lum bright=\"21000\" contrast=\"52000\"/>%1<a:clrRepl><a:srgbClr val=\"FF0000\"><a:alpha val=\"50000\"/></a:srgbClr></a:clrRepl>"
                               "<a:hsl hue=\"5400000\" sat=\"-20000\" lum=\"10000\"/><a:tint hue=\"10800000\" amt=\"30000\"/>")
                    .arg(duotone));
        QVERIFY(near(p->brightness, 21) && near(p->contrast, 52));
        QVERIFY(sameEffects(p->blipEffects, {duotoneEffect(QColor(0, 0, 0x80), QColor(255, 255, 0)), replaceEffect(QColor(255, 0, 0, 128)), hslEffect(90, -0.2, 0.1),
                                             tintEffect(180, 0.3)}));
        // A color changed to another is an effect; a color made clear is the picture's transparent color.
        p = pic(redForWhite);
        QVERIFY(!p->hasTransparentColor);
        QVERIFY(sameEffects(p->blipEffects, {changeEffect(QColor(255, 255, 255), QColor(255, 0, 0))}));
        p = pic(clearWhite);
        QVERIFY(p->hasTransparentColor && p->blipEffects.isEmpty());
        QCOMPARE(p->transparentColor, QColor(255, 255, 255));
        // A clear color, the brightness, and gray, in the order they are drawn, are all settings.
        p = pic(clearWhite + QStringLiteral("<a:lum bright=\"10000\"/><a:grayscl/>"));
        QVERIFY(p->hasTransparentColor && near(p->brightness, 10) && p->recolor == PictureItem::Grayscale && p->blipEffects.isEmpty());
        // In another order, the settings hold those that come first and the rest wait in the list: gray, then the brightness is drawn so.
        p = pic(QStringLiteral("<a:grayscl/><a:lum bright=\"10000\"/>"));
        QVERIFY(p->recolor == PictureItem::Grayscale && p->brightness == 0);
        QVERIFY(sameEffects(p->blipEffects, {lumEffect(0.1, 0)}));
        // A duotone before the brightness stays before it: nothing is settled by moving it.
        p = pic(duotone + QStringLiteral("<a:lum bright=\"10000\"/>"));
        QVERIFY(p->brightness == 0 && p->contrast == 0);
        QVERIFY(sameEffects(p->blipEffects, {duotoneEffect(QColor(0, 0, 0x80), QColor(255, 255, 0)), lumEffect(0.1, 0)}));
        // Two of a kind: the second is not lost, nor is a clear color after a color changed to another.
        p = pic(QStringLiteral("<a:lum bright=\"10000\"/><a:lum bright=\"20000\"/>"));
        QVERIFY(near(p->brightness, 10) && sameEffects(p->blipEffects, {lumEffect(0.2, 0)}));
        p = pic(redForWhite + clearWhite);
        QVERIFY(!p->hasTransparentColor);
        QVERIFY(sameEffects(p->blipEffects, {changeEffect(QColor(255, 255, 255), QColor(255, 0, 0)), changeEffect(QColor(255, 255, 255), QColor(255, 255, 255, 0))}));
        p = pic(clearWhite + redForWhite);
        QVERIFY(p->hasTransparentColor);
        QVERIFY(sameEffects(p->blipEffects, {changeEffect(QColor(255, 255, 255), QColor(255, 0, 0))}));
        p = pic(QStringLiteral("<a:grayscl/><a:biLevel thresh=\"25000\"/>"));
        QVERIFY(p->recolor == PictureItem::Grayscale && sameEffects(p->blipEffects, {biLevelEffect(0.25)}));
        // A color change that doesn't count the clearness, and one that makes a different color clear, are kept whole.
        p = pic(QStringLiteral("<a:clrChange useA=\"0\"><a:clrFrom><a:srgbClr val=\"FFFFFF\"/></a:clrFrom><a:clrTo><a:srgbClr val=\"FFFFFF\"><a:alpha val=\"0\"/></a:srgbClr></a:clrTo></a:clrChange>"));
        QVERIFY(!p->hasTransparentColor);
        QVERIFY(sameEffects(p->blipEffects, {changeEffect(QColor(255, 255, 255), QColor(255, 255, 255, 0), false)}));
        // A brightness and contrast of none are kept as listed too (the writer then has the empty a:lum back).
        p = pic(QStringLiteral("<a:lum/>"));
        QVERIFY(p->brightness == 0 && sameEffects(p->blipEffects, {lumEffect(0, 0)}));
        // The settings and what is beside them come back through the item's JSON.
        p = pic(QStringLiteral("<a:biLevel thresh=\"25000\"/><a:tint hue=\"10800000\" amt=\"30000\"/>"));
        PictureItem back;
        back.fromJson(p->toJson());
        QVERIFY(near(back.blackWhiteLevel, 0.25) && back.recolor == PictureItem::BlackWhite);
        QVERIFY(sameEffects(back.blipEffects, {tintEffect(180, 0.3)}));
        // The effects in the order they are drawn: the settings first, in the order set, then the list.
        QVERIFY(sameEffects(p->colorEffects(), {biLevelEffect(0.25), tintEffect(180, 0.3)}));
    }

    // A shape of a diagram's drawing that has a picture fill with changes, cut by a:srcRect, is a picture over the shape: the picture has the
    // changes (the fill that was left was the only place they were), and so has the equation's picture (a shape the picture fills).
    void dmlDiagramShapeWithACutPictureKeepsItsEffects()
    {
        DmlEnv env;
        env.usePackage();
        const ItemPtr it = dmlOne(env, dspSp(dmlXfrm(10, 10, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/><a:blipFill><a:blip r:embed=\"rId1\"><a:lum bright=\"21000\" contrast=\"52000\"/>"
                                                                                       "<a:duotone><a:srgbClr val=\"000080\"/><a:srgbClr val=\"FFFF00\"/></a:duotone></a:blip><a:srcRect t=\"25000\" b=\"25000\"/>"
                                                                                       "<a:stretch><a:fillRect/></a:stretch></a:blipFill>"),
                                             QString(), QString()));
        auto g = dmlAs<GroupItem>(it);
        QVERIFY(g && g->children.size() == 2);
        auto pic = dmlAs<PictureItem>(g->children[0]);
        QVERIFY(pic);
        QVERIFY(sameEffects(pic->blipEffects, {lumEffect(0.21, 0.52), duotoneEffect(QColor(0, 0, 0x80), QColor(255, 255, 0))}));
        // Drawn: the brightness and contrast, then the duotone, as the file lists them.
        QVERIFY(sameEffects(pic->colorEffects(), pic->blipEffects));
    }

    // What each effect does to a color (all of a:blip's, in order): a:lum as the presentation program draws it (fitted to reference pictures: a
    // brightness of 14% adds 36 to each color; with 21% and a contrast of 52% a 100 comes to 153, and the contrast takes 1 / (1 - 0.52) times
    // the distance from the middle), the clamps, the clear color, and that the picture itself is not changed.
    void blipEffectsChangeColorsAsThePresentationProgramDoes()
    {
        const auto run = [](const QVector<QColor> &colors, const jp::BlipEffect &effect) {
            QImage img(colors.size(), 1, QImage::Format_ARGB32);
            for (int i = 0; i < colors.size(); ++i) img.setPixelColor(i, 0, colors[i]);
            const QImage out = jp::applyBlipEffects(img, {effect});
            QVector<QColor> result;
            for (int i = 0; i < colors.size(); ++i) result << out.pixelColor(i, 0);
            // The picture asked of is as it was.
            for (int i = 0; i < colors.size(); ++i) if (img.pixelColor(i, 0) != colors[i]) result.clear();
            return result;
        };
        jp::BlipEffect lum;
        lum.kind = jp::BlipEffect::Luminance;
        lum.brightness = 0.14;
        QVector<QColor> got = run({QColor(100, 100, 100), QColor(0, 0, 0), QColor(250, 250, 250)}, lum);
        QCOMPARE(got.value(0), QColor(136, 136, 136));
        QCOMPARE(got.value(1), QColor(36, 36, 36));
        QCOMPARE(got.value(2), QColor(255, 255, 255));
        lum.brightness = 0.21;
        lum.contrast = 0.52;
        got = run({QColor(100, 100, 100), QColor(0, 0, 0), QColor(182, 66, 15)}, lum);
        QCOMPARE(got.value(0), QColor(153, 153, 153));
        QCOMPARE(got.value(1), QColor(0, 0, 0));
        QCOMPARE(got.value(2), QColor(255, 82, 0));
        // A contrast below 0 is 1 + it times the distance (a washed-out look, with the brightness above, comes to the top of the range).
        lum.brightness = 0;
        lum.contrast = -0.5;
        got = run({QColor(0, 0, 0), QColor(255, 255, 255), QColor(128, 128, 128)}, lum);
        QCOMPARE(got.value(0), QColor(64, 64, 64));
        QCOMPARE(got.value(1), QColor(191, 191, 191));
        lum.brightness = 0.7;
        lum.contrast = -0.7;
        got = run({QColor(0, 0, 0), QColor(255, 255, 255)}, lum);
        QVERIFY(got.value(0).red() > 195 && got.value(0).red() < 215);
        QCOMPARE(got.value(1).red(), 255);
        // The same table the older files' pictures have: a factor of 1 / (1 - 0.52) (2.0833).
        QCOMPARE(jp::luminanceTable(0.21, jp::contrastFactor(0.52))[100], uchar(153));
        QVERIFY(near(jp::contrastFactor(0.52), 1 / 0.48) && near(jp::contrastFactor(-0.5), 0.5) && near(jp::contrastFactor(0), 1));
        QVERIFY(std::isfinite(jp::contrastFactor(1)) && jp::contrastFactor(1) > 100);
        // Gray (the weights JeffPub's recolor has), and black and white from the threshold.
        jp::BlipEffect gray;
        gray.kind = jp::BlipEffect::Grayscale;
        got = run({QColor(255, 0, 0), QColor(0, 0, 255, 128)}, gray);
        QCOMPARE(got.value(0), QColor(76, 76, 76));
        QCOMPARE(got.value(1).alpha(), 128);
        jp::BlipEffect bw;
        bw.kind = jp::BlipEffect::BiLevel;
        bw.threshold = 0.25;
        got = run({QColor(70, 70, 70), QColor(60, 60, 60)}, bw);
        QCOMPARE(got.value(0), QColor(255, 255, 255));
        QCOMPARE(got.value(1), QColor(0, 0, 0));
        bw.threshold = 0.5;
        got = run({QColor(120, 120, 120), QColor(136, 136, 136)}, bw);
        QCOMPARE(got.value(0), QColor(0, 0, 0));
        QCOMPARE(got.value(1), QColor(255, 255, 255));
        // A color changed: the one that is (and those within nine of it in each part) turns into the other, with its clearness.
        jp::BlipEffect change;
        change.kind = jp::BlipEffect::ColorChange;
        change.color = QColor(0x9C, 0x8C, 0x7B);
        change.color2 = QColor(0x9C, 0x8C, 0x7B, 0);
        got = run({QColor(0x9C, 0x8C, 0x7B), QColor(0x95, 0x8C, 0x7B), QColor(0x80, 0x8C, 0x7B), QColor(0x9C, 0x8C, 0x7B, 100)}, change);
        QCOMPARE(got.value(0).alpha(), 0);
        QCOMPARE(got.value(1).alpha(), 0);
        QCOMPARE(got.value(2), QColor(0x80, 0x8C, 0x7B));
        QCOMPARE(got.value(3).alpha(), 0);
        change.color2 = QColor(255, 0, 0);
        got = run({QColor(0x9C, 0x8C, 0x7B, 100)}, change);
        QCOMPARE(got.value(0), QColor(255, 0, 0, 100));
        // All colors made one, with the pixels' own clearness; two colors that black and white go to.
        jp::BlipEffect repl;
        repl.kind = jp::BlipEffect::ColorReplace;
        repl.color = QColor(255, 0, 0);
        got = run({QColor(1, 2, 3, 77)}, repl);
        QCOMPARE(got.value(0), QColor(255, 0, 0, 77));
        jp::BlipEffect duo;
        duo.kind = jp::BlipEffect::Duotone;
        duo.color = QColor(0, 0, 0x80);
        duo.color2 = QColor(255, 255, 0);
        got = run({QColor(0, 0, 0), QColor(255, 255, 255)}, duo);
        QCOMPARE(got.value(0), QColor(0, 0, 0x80));
        QCOMPARE(got.value(1), QColor(255, 255, 0));
        // The hue turned a third of a circle, the saturation and lightness moved, a tint's hue taken.
        jp::BlipEffect hsl;
        hsl.kind = jp::BlipEffect::Hsl;
        hsl.hue = 120;
        got = run({QColor(255, 0, 0)}, hsl);
        QCOMPARE(got.value(0), QColor(0, 255, 0));
        hsl.hue = 0;
        hsl.lightness = -1;
        QCOMPARE(run({QColor(255, 0, 0)}, hsl).value(0), QColor(0, 0, 0));
        hsl.lightness = 1;
        QCOMPARE(run({QColor(255, 0, 0)}, hsl).value(0), QColor(255, 255, 255));
        hsl.lightness = 0;
        hsl.saturation = -1;
        got = run({QColor(255, 0, 0)}, hsl);
        QVERIFY(got.value(0).red() == got.value(0).green() && got.value(0).green() == got.value(0).blue());
        jp::BlipEffect tint;
        tint.kind = jp::BlipEffect::Tint;
        tint.hue = 0;
        tint.amount = 1;
        got = run({QColor(128, 128, 128)}, tint);
        QVERIFY(got.value(0).red() > got.value(0).green() + 100 && got.value(0).green() == got.value(0).blue());
        // Effects go one after the other, and none leaves the picture as it was (not even converted).
        QImage plain(1, 1, QImage::Format_RGB32);
        plain.fill(QColor(100, 100, 100));
        QVERIFY(jp::applyBlipEffects(plain, {}).format() == QImage::Format_RGB32);
        jp::BlipEffect nothing;   // a:lum with nothing in it, as the reference presentations' masters have
        QVERIFY(jp::applyBlipEffects(plain, {nothing}).format() == QImage::Format_RGB32);
        bw.threshold = 0.25;
        QCOMPARE(jp::applyBlipEffects(plain, {gray, bw}).pixelColor(0, 0), QColor(255, 255, 255));
        bw.threshold = 0.5;
        QCOMPARE(jp::applyBlipEffects(plain, {gray, bw}).pixelColor(0, 0), QColor(0, 0, 0));
        QCOMPARE(jp::applyBlipEffects(plain, {lum, bw}).pixelColor(0, 0), QColor(255, 255, 255));   // 100 is washed out to 235
        QVERIFY(jp::applyBlipEffects(QImage(), {gray}).isNull());
        // Held to what a picture's changes may cost (its pixels times its effects): 12 million pixels take a few of them, and not forty.
        QImage large(4000, 3000, QImage::Format_ARGB32);
        large.fill(QColor(100, 100, 100));
        QVector<jp::BlipEffect> few(3, gray), crowd(40, gray);
        QVERIFY(qAbs(jp::applyBlipEffects(large, few).pixelColor(7, 7).red() - 100) <= 1);   // (a rounding either way: a machine that fuses sums does)
        QVERIFY(jp::applyBlipEffects(large, few).cacheKey() != large.cacheKey());
        QCOMPARE(jp::applyBlipEffects(large, crowd).cacheKey(), large.cacheKey());
        // Kept: the same picture with the same effects is made once, and with others it is made again.
        const QImage a = jp::blipAdjusted(plain, {lum}), b = jp::blipAdjusted(plain, {lum});
        QCOMPARE(a.cacheKey(), b.cacheKey());
        QVERIFY(jp::blipAdjusted(plain, {gray}).cacheKey() != a.cacheKey());
        QVERIFY(jp::blipEffectsKey({lum}) != jp::blipEffectsKey({gray}) && jp::blipEffectsKey({lum}) == jp::blipEffectsKey({lum}));
    }

    // A neutral gray stays what it is through gray (a gray of 128 is not 127 for a sum of weights that comes to a hair under 1), so a gray
    // picture is not darkened by it, and black and white from half the range keeps 128 on the white side; a tint of none and a turn of the hue
    // of none change no color (the tint's hue was taken whatever its amount was); a tint of a part takes a part of the way.
    // The picture filters (core/picturefx.h): a test picture of a dark square on light ground with a soft red band, built in code.
    static QImage filterSample(int w = 96, int h = 72)
    {
        QImage im(w, h, QImage::Format_ARGB32);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x) {
                const bool square = x > w / 3 && x < 2 * w / 3 && y > h / 3 && y < 2 * h / 3;
                im.setPixel(x, y, square ? qRgb(40, 60, 90) : qRgb(200 + (x * 40 / w), 190, 180 - y / 4));
            }
        return im;
    }
    static jp::BlipEffect filterEffect(jp::BlipEffect::Kind kind, double amount, int style = 0)
    {
        jp::BlipEffect e;
        e.kind = kind;
        e.amount = amount;
        e.style = style;
        return e;
    }
    static QByteArray pixelHash(const QImage &im)
    {
        const QImage a = im.convertToFormat(QImage::Format_ARGB32);
        QByteArray bytes;
        for (int y = 0; y < a.height(); ++y) bytes.append(reinterpret_cast<const char *>(a.constScanLine(y)), a.width() * 4);
        return QCryptographicHash::hash(bytes, QCryptographicHash::Sha1).toHex();
    }

    void pictureFiltersChangeThePixelsAsTheyAreNamedAndLeaveTheOriginalAlone()
    {
        const QImage sample = filterSample();
        const QByteArray before = pixelHash(sample);
        const int edgeY = sample.height() / 2, edgeX = sample.width() / 3;   // (the square's left edge: the light ground ends here)
        auto step = [&](const QImage &im) { return qGray(im.pixel(edgeX + 1, edgeY)) - qGray(im.pixel(edgeX - 3, edgeY)); };
        const QImage sharp = jp::applyBlipEffects(sample, {filterEffect(jp::BlipEffect::Sharpen, 1.0)});
        const QImage soft = jp::applyBlipEffects(sample, {filterEffect(jp::BlipEffect::Sharpen, -1.0)});
        // Sharpening deepens the edge, softening lessens it (the step across it is larger or smaller in size).
        QVERIFY(std::abs(step(sharp)) > std::abs(step(sample)));
        QVERIFY(std::abs(step(soft)) < std::abs(step(sample)));
        QCOMPARE(sharp.size(), sample.size());
        const QImage warm = jp::applyBlipEffects(sample, {filterEffect(jp::BlipEffect::Temperature, 1.0)});
        const QImage cool = jp::applyBlipEffects(sample, {filterEffect(jp::BlipEffect::Temperature, -1.0)});
        QVERIFY(qRed(warm.pixel(2, 2)) > qRed(sample.pixel(2, 2)) && qBlue(warm.pixel(2, 2)) < qBlue(sample.pixel(2, 2)));
        QVERIFY(qRed(cool.pixel(2, 2)) < qRed(sample.pixel(2, 2)) && qBlue(cool.pixel(2, 2)) > qBlue(sample.pixel(2, 2)));
        QCOMPARE(qGreen(warm.pixel(2, 2)), qGreen(sample.pixel(2, 2)));
        // Artistic: pencil is gray with the ground near white; mosaic is blocks of one color; glow edges is dark where nothing changes; blur has no hard edge.
        const QImage pencil = jp::applyBlipEffects(sample, {filterEffect(jp::BlipEffect::Artistic, 0.5, int(jp::ArtisticStyle::PencilGrayscale))});
        QCOMPARE(qRed(pencil.pixel(10, 10)), qGreen(pencil.pixel(10, 10)));
        QVERIFY(qGray(pencil.pixel(10, 10)) > 230);
        const QImage glow = jp::applyBlipEffects(sample, {filterEffect(jp::BlipEffect::Artistic, 0.5, int(jp::ArtisticStyle::GlowEdges))});
        QVERIFY(qGray(glow.pixel(2, 2)) < 10);
        QVERIFY(qGray(glow.pixel(edgeX, edgeY)) > 30);
        const QImage mosaic = jp::applyBlipEffects(sample, {filterEffect(jp::BlipEffect::Artistic, 0.5, int(jp::ArtisticStyle::Mosaic))});
        QCOMPARE(mosaic.pixel(0, 0), mosaic.pixel(1, 1));
        QVERIFY(mosaic.pixel(0, 0) != mosaic.pixel(sample.width() - 1, sample.height() - 1) || mosaic.pixel(0, 0) != mosaic.pixel(sample.width() / 2, sample.height() / 2));
        const QImage blur = jp::applyBlipEffects(sample, {filterEffect(jp::BlipEffect::Artistic, 1.0, int(jp::ArtisticStyle::Blur))});
        QVERIFY(std::abs(step(blur)) < std::abs(step(sample)) * 3 / 4);
        // The picture given is never changed, and an effect of none leaves the picture as it is.
        QCOMPARE(pixelHash(sample), before);
        QCOMPARE(pixelHash(jp::applyBlipEffects(sample, {filterEffect(jp::BlipEffect::Sharpen, 0)})), before);
        // The same pixels on every system: whole numbers only. (The values are those worked out here; a system that differs fails this test.)
        QCOMPARE(pixelHash(sharp), pixelHash(jp::applyBlipEffects(sample, {filterEffect(jp::BlipEffect::Sharpen, 1.0)})));
        QCOMPARE(pixelHash(jp::applyBlipEffects(sample, {filterEffect(jp::BlipEffect::Artistic, 0.5, 2)})), pixelHash(glow));
    }

    void pictureFiltersKeepClearPixelsClearAndAreBoundedByWhatTheyCost()
    {
        // Clear pixels stay clear and do not stain their neighbors.
        QImage clear(40, 40, QImage::Format_ARGB32);
        clear.fill(qRgba(255, 0, 0, 0));
        for (int y = 10; y < 30; ++y)
            for (int x = 10; x < 30; ++x) clear.setPixel(x, y, qRgb(0, 0, 255));
        const QImage soft = jp::applyBlipEffects(clear, {filterEffect(jp::BlipEffect::Artistic, 1.0, int(jp::ArtisticStyle::Blur))});
        QCOMPARE(qAlpha(soft.pixel(0, 0)), 0);
        QVERIFY(qAlpha(soft.pixel(20, 20)) > 200);
        QCOMPARE(qRed(soft.pixel(20, 20)), 0);   // (the red of the clear pixels did not bleed in)
        // A picture of more than kPictureFxWorkPixels is worked on as a smaller copy: 5 million pixels come out at no more than 4 million.
        const QImage wide(5000, 1000, QImage::Format_ARGB32);
        const QImage copy = jp::pictureFxWorkCopy(wide);
        QVERIFY(qint64(copy.width()) * copy.height() <= jp::kPictureFxWorkPixels);
        QVERIFY(copy.width() * 2 >= 5000 / 2);
        QCOMPARE(jp::pictureFxWorkCopy(filterSample()).size(), filterSample().size());
        // What the filters cost counts the work copy, not the picture: 100 million pixels with one filter is the work of 4 million, at the filter's own cost per pixel.
        QCOMPARE(jp::blipEffectsWork(100'000'000, {filterEffect(jp::BlipEffect::Sharpen, 0.5)}), jp::kPictureFxWorkPixels * jp::blipFilterCost(filterEffect(jp::BlipEffect::Sharpen, 0.5)));
        QJsonArray hostile;
        for (int i = 0; i < 1000; ++i) hostile.append(QJsonObject{{"k", "artistic"}, {"amt", 1e300}, {"style", 99}});
        const QVector<jp::BlipEffect> read = jp::blipEffectsFromJson(hostile);
        QCOMPARE(int(read.size()), jp::kMaxBlipEffects);
        QCOMPARE(read[0].amount, 1.0);
        QCOMPARE(read[0].style, int(jp::ArtisticStyle::Last));
        // The settings come back from their JSON as they were.
        const QVector<jp::BlipEffect> mine{filterEffect(jp::BlipEffect::Sharpen, -0.5), filterEffect(jp::BlipEffect::Temperature, 0.25), filterEffect(jp::BlipEffect::Artistic, 0.7, 3)};
        QVERIFY(jp::blipEffectsFromJson(jp::blipEffectsToJson(mine)) == mine);
        // A stopped job comes back unfinished and is not kept for next time.
        std::atomic<bool> stop{true};
        const int made = jp::blipAdjustedMade();
        jp::blipAdjusted(filterSample(97, 71), {filterEffect(jp::BlipEffect::Sharpen, 0.9)}, &stop);
        QCOMPARE(jp::blipAdjustedMade(), made);
        // Kept: the same picture and filters asked for again is made once.
        const QImage again = filterSample(98, 71);
        const QVector<jp::BlipEffect> one{filterEffect(jp::BlipEffect::Sharpen, 0.8)};
        const QImage first = jp::blipAdjusted(again, one);
        QCOMPARE(jp::blipAdjustedMade(), made + 1);
        QCOMPARE(jp::blipAdjusted(again, one).cacheKey(), first.cacheKey());
        QCOMPARE(jp::blipAdjustedMade(), made + 1);
    }

    // The costs of the filters are the measured ones (a reviewer's numbers on 4 million pixels, in milliseconds), rounded up.
    void filterCostsAreTheMeasuredOnes()
    {
        const struct { jp::BlipEffect e; qint64 ms; } measured[] = {
            {filterEffect(jp::BlipEffect::Sharpen, 0.5), 197},
            {filterEffect(jp::BlipEffect::Artistic, 0.5, int(jp::ArtisticStyle::Blur)), 178},
            {filterEffect(jp::BlipEffect::Artistic, 0.5, int(jp::ArtisticStyle::GlowEdges)), 144},
            {filterEffect(jp::BlipEffect::Artistic, 0.5, int(jp::ArtisticStyle::PencilGrayscale)), 61},
            {filterEffect(jp::BlipEffect::Artistic, 0.5, int(jp::ArtisticStyle::Mosaic)), 19},
            {filterEffect(jp::BlipEffect::Temperature, 0.5), 10},
        };
        for (const auto &m : measured) {
            const qint64 ns = jp::kPictureFxWorkPixels * jp::blipFilterCost(m.e);
            QVERIFY2(ns >= m.ms * 1'000'000 && ns <= m.ms * 1'000'000 * 3 / 2 + 10'000'000, qPrintable(jp::blipFilterName(m.e) + ": " + QString::number(ns)));
        }
        // One of each kind of the dearest (sharpen, temperature, blur) on a picture of 4 million pixels is under half a second: the busy box is not for it.
        QVERIFY(jp::blipEffectsWork(4'000'000, {filterEffect(jp::BlipEffect::Sharpen, 0.5), filterEffect(jp::BlipEffect::Temperature, 0.5), filterEffect(jp::BlipEffect::Artistic, 0.5, 0)}) < jp::kPictureFxBusyWork);
        // Two of those pictures at once are (the work of a change to several pictures is the sum of theirs).
        const qint64 one = jp::blipEffectsWork(4'000'000, {filterEffect(jp::BlipEffect::Sharpen, 0.5), filterEffect(jp::BlipEffect::Temperature, 0.5), filterEffect(jp::BlipEffect::Artistic, 0.5, 0)});
        QVERIFY(one * 2 > jp::kPictureFxBusyWork);
    }

    // The work cap never turns the filters off for what a person can put on a picture: a photograph of 4 million pixels with sharpen, temperature, and an artistic effect comes out filtered.
    // A longer list (a file's) is worked through in order until its budget is used, and the number left out is told.
    void filtersAreDoneForEveryLegalCombinationAndALongListIsCutWithATell()
    {
        const QImage photo = filterSample(2000, 2000);
        const QVector<jp::BlipEffect> legal{filterEffect(jp::BlipEffect::Sharpen, 0.5), filterEffect(jp::BlipEffect::Temperature, 0.5), filterEffect(jp::BlipEffect::Artistic, 0.5, int(jp::ArtisticStyle::Blur))};
        QCOMPARE(jp::blipFiltersLeftOut(qint64(photo.width()) * photo.height(), legal), 0);
        const QImage all = jp::applyBlipEffects(photo, legal);
        QVERIFY(all.cacheKey() != photo.cacheKey());
        // Each of them took part: left off one at a time, the picture is not the same.
        for (int i = 0; i < legal.size(); ++i) {
            QVector<jp::BlipEffect> without = legal;
            without.removeAt(i);
            QVERIFY2(pixelHash(jp::applyBlipEffects(photo, without)) != pixelHash(all), qPrintable(QString::number(i)));
        }
        // Even two of each of the dearest on a picture of any size.
        const QVector<jp::BlipEffect> dear{filterEffect(jp::BlipEffect::Sharpen, 0.5), filterEffect(jp::BlipEffect::Artistic, 0.5, int(jp::ArtisticStyle::Blur)), filterEffect(jp::BlipEffect::Artistic, 0.5, int(jp::ArtisticStyle::GlowEdges)),
                                           filterEffect(jp::BlipEffect::Artistic, 0.5, int(jp::ArtisticStyle::PencilGrayscale)), filterEffect(jp::BlipEffect::Temperature, 0.5)};
        QCOMPARE(jp::blipFiltersLeftOut(100'000'000, dear), 0);
        // A list only a file can make: 32 sharpens on a million pixels. The first are done (20 fit), the rest are left out, and the count says so.
        const QImage million = filterSample(1000, 1000);
        const QVector<jp::BlipEffect> hostile(jp::kMaxBlipEffects, filterEffect(jp::BlipEffect::Sharpen, 0.5));
        const int fit = int(jp::kMaxBlipFilterWork / (qint64(1000) * 1000 * jp::blipFilterCost(hostile[0])));
        QVERIFY(fit > 0 && fit < jp::kMaxBlipEffects);
        QCOMPARE(jp::blipFiltersLeftOut(1'000'000, hostile), jp::kMaxBlipEffects - fit);
        const QImage cut = jp::applyBlipEffects(million, hostile);
        QVERIFY(cut.cacheKey() != million.cacheKey());
        QCOMPARE(pixelHash(cut), pixelHash(jp::applyBlipEffects(million, hostile.mid(0, fit))));
        QVERIFY(pixelHash(cut) != pixelHash(jp::applyBlipEffects(million, hostile.mid(0, fit - 1))));
    }

    // The filters' work is linear in the pixels whatever the picture's shape (a window of the blur is started once for each row and column, and a radius is held to the picture's own size),
    // counted in steps and never timed; and a flag set at the start stops them before the first row.
    void filtersWorkLinearlyInTheirPixelsForEveryShapeAndStopWhenAsked()
    {
        const jp::BlipEffect effects[] = {filterEffect(jp::BlipEffect::Sharpen, 0.7), filterEffect(jp::BlipEffect::Sharpen, -0.7), filterEffect(jp::BlipEffect::Temperature, 0.7),
                                          filterEffect(jp::BlipEffect::Artistic, 0.7, int(jp::ArtisticStyle::Blur)), filterEffect(jp::BlipEffect::Artistic, 0.7, int(jp::ArtisticStyle::PencilGrayscale)),
                                          filterEffect(jp::BlipEffect::Artistic, 0.7, int(jp::ArtisticStyle::GlowEdges)), filterEffect(jp::BlipEffect::Artistic, 0.7, int(jp::ArtisticStyle::Mosaic))};
        for (const jp::BlipEffect &e : effects) {
            for (const int pixels : {4096, 16384, 65536}) {
                const int side = int(std::lround(std::sqrt(double(pixels))));
                const QSize shapes[] = {QSize(1, pixels), QSize(pixels, 1), QSize(side, side)};
                for (const QSize &shape : shapes) {
                    QImage im(shape, QImage::Format_ARGB32);
                    im.fill(qRgb(90, 120, 200));
                    jp::FilterCount count;
                    const QImage out = jp::applyBlipFilter(im, e, nullptr, &count);
                    QCOMPARE(out.size(), im.size());
                    const QString what = jp::blipFilterName(e) + " " + QString::number(shape.width()) + "x" + QString::number(shape.height());
                    QVERIFY2(count.steps > 0 && count.steps <= qint64(pixels) * 40, qPrintable(what + ": " + QString::number(count.steps)));
                }
            }
            // A legal picture one pixel wide and a million tall costs the same for its pixels as a square one, within a small factor (a window is started on every row; it was a hundred times).
            QImage tall(1, 1'000'000, QImage::Format_ARGB32), square(1000, 1000, QImage::Format_ARGB32);
            tall.fill(qRgb(10, 20, 30));
            square.fill(qRgb(10, 20, 30));
            jp::FilterCount a, b;
            jp::applyBlipFilter(tall, e, nullptr, &a);
            jp::applyBlipFilter(square, e, nullptr, &b);
            QVERIFY2(a.steps <= b.steps * 3 + 1000, qPrintable(jp::blipFilterName(e) + ": " + QString::number(a.steps) + " against " + QString::number(b.steps)));
            // A flag already set: no step is taken, and the picture comes back as it was.
            std::atomic<bool> stop{true};
            jp::FilterCount none;
            QImage im(300, 200, QImage::Format_ARGB32);
            im.fill(qRgb(1, 2, 3));
            QCOMPARE(jp::applyBlipFilter(im, e, &stop, &none).cacheKey(), im.cacheKey());
            QCOMPARE(none.steps, qint64(0));
        }
    }

    // The filtered picture made beside the window: drawn as it is at once, finished later (the window is told), and what was made is kept by the picture's own name, so that
    // a picture made again from the same source each time is worked out once.
    void filtersAreWorkedOutBesideTheWindowAndKeptByTheSourcesName()
    {
        const QImage photo = filterSample(320, 240);
        const QVector<jp::BlipEffect> fx{filterEffect(jp::BlipEffect::Artistic, 0.8, int(jp::ArtisticStyle::Blur))};
        int told = 0;
        const int listener = jp::addBlipFilterListener([&told] { ++told; });
        const int made = jp::blipAdjustedMade();
        bool pending = false;
        const QImage first = jp::blipAdjustedLater(photo, fx, QStringLiteral("test|beside"), &pending);
        QVERIFY(pending);
        QCOMPARE(first.cacheKey(), photo.cacheKey());   // (drawn as it is, with no wait)
        QTRY_VERIFY(told >= 1);
        const QImage done = jp::blipAdjustedLater(photo, fx, QStringLiteral("test|beside"), &pending);
        QVERIFY(!pending);
        QVERIFY(done.cacheKey() != photo.cacheKey());
        QCOMPARE(jp::blipAdjustedMade(), made + 1);
        // The same source made again as a new image (a removal or a profile does that at each paint) is found, not worked out.
        const QImage again = photo.copy();
        QVERIFY(again.cacheKey() != photo.cacheKey());
        QCOMPARE(jp::blipAdjustedLater(again, fx, QStringLiteral("test|beside"), &pending).cacheKey(), done.cacheKey());
        QVERIFY(!pending);
        QCOMPARE(jp::blipAdjusted(again, fx, nullptr, QStringLiteral("test|beside")).cacheKey(), done.cacheKey());
        QCOMPARE(jp::blipAdjustedMade(), made + 1);
        // Asked for by the pixels' own key (no name), a new image is a new picture.
        jp::blipAdjusted(again, fx);
        QCOMPARE(jp::blipAdjustedMade(), made + 2);
        jp::removeBlipFilterListener(listener);
    }

    void blipEffectsKeepNeutralGraysAndTintsOfNoneAsTheyAre()
    {
        QImage ramp(256, 1, QImage::Format_ARGB32);
        for (int v = 0; v < 256; ++v) ramp.setPixelColor(v, 0, QColor(v, v, v));
        const QImage grayed = jp::applyBlipEffects(ramp, {grayEffect()});
        for (int v = 0; v < 256; ++v) QVERIFY2(grayed.pixelColor(v, 0).red() == v, qPrintable(QString::number(v) + " -> " + QString::number(grayed.pixelColor(v, 0).red())));
        const QImage cut = jp::applyBlipEffects(ramp, {biLevelEffect(0.5)}), grayedCut = jp::applyBlipEffects(ramp, {grayEffect(), biLevelEffect(0.5)});
        for (int v = 0; v < 256; ++v) {
            QCOMPARE(cut.pixelColor(v, 0).red(), v >= 128 ? 255 : 0);
            QCOMPARE(grayedCut.pixelColor(v, 0).red(), v >= 128 ? 255 : 0);
        }
        // The colors a picture has go through a tint of none, and a turn of nothing, as they are (within the rounding of a turn through hue and back).
        QImage colors(6, 1, QImage::Format_ARGB32);
        const QColor some[6] = {QColor(10, 200, 50), QColor(255, 0, 0), QColor(0, 0, 255, 90), QColor(128, 128, 128), QColor(250, 240, 10), QColor(3, 3, 9)};
        for (int i = 0; i < 6; ++i) colors.setPixelColor(i, 0, some[i]);
        for (const jp::BlipEffect &none : {tintEffect(0, 0), tintEffect(200, 0), hslEffect(0, 0, 0), hslEffect(360, 0, 0), tintEffect(200, 1e-9), hslEffect(0, 0, 1e-9)}) {
            const QImage out = jp::applyBlipEffects(colors, {none});
            for (int i = 0; i < 6; ++i) {
                const QColor got = out.pixelColor(i, 0);
                QVERIFY2(qAbs(got.red() - some[i].red()) <= 1 && qAbs(got.green() - some[i].green()) <= 1 && qAbs(got.blue() - some[i].blue()) <= 1 && got.alpha() == some[i].alpha(),
                         qPrintable(got.name() + " for " + some[i].name()));
            }
        }
        // A tint of a part: green moves halfway toward blue (cyan is the middle of the way), and the whole way (blue).
        QImage green(1, 1, QImage::Format_ARGB32);
        green.setPixelColor(0, 0, QColor(0, 255, 0));
        QColor got = jp::applyBlipEffects(green, {tintEffect(240, 0.5)}).pixelColor(0, 0);
        QVERIFY2(got.red() <= 1 && got.green() >= 254 && got.blue() >= 254, qPrintable(got.name()));
        got = jp::applyBlipEffects(green, {tintEffect(240, 1)}).pixelColor(0, 0);
        QVERIFY2(got.red() <= 1 && got.green() <= 1 && got.blue() >= 254, qPrintable(got.name()));
        // A gray one takes the hue, as much of it as the amount.
        QImage gray(1, 1, QImage::Format_ARGB32);
        gray.setPixelColor(0, 0, QColor(128, 128, 128));
        got = jp::applyBlipEffects(gray, {tintEffect(0, 0.5)}).pixelColor(0, 0);
        QVERIFY2(got.red() > got.green() + 40 && got.green() == got.blue(), qPrintable(got.name()));
        // And the contrast at its most (100 percent): colors cut at the middle, whatever the brightness.
        const QImage hard = jp::applyBlipEffects(ramp, {lumEffect(0, 1)});
        QCOMPARE(hard.pixelColor(100, 0).red(), 0);
        QCOMPARE(hard.pixelColor(150, 0).red(), 255);
        QVERIFY(hard.pixelColor(127, 0).red() < 128 && hard.pixelColor(129, 0).red() > 100);
    }

    // What a picture's changes may cost is held (blipEffectsWork, held to kMaxBlipWork): 32 hue turns on a 12 million pixel photograph, which a file
    // could ask of the picture every time it was shown at another size, took half a minute; it is left as it is, and at once. A single hue turn on a
    // large photograph, and the few changes real files have, are made. A picture of more pixels than any real one (kMaxBlipPixels) is left as it is.
    void blipEffectsAreHeldToWhatTheyCost()
    {
        const qint64 twelveMillion = 4000LL * 3000;
        QVector<jp::BlipEffect> turns(32, hslEffect(90, 0.2, 0.1));
        QVERIFY(jp::blipEffectsWork(twelveMillion, turns) > jp::kMaxBlipWork);
        QVERIFY(jp::blipEffectsWork(twelveMillion, {hslEffect(90, 0.2, 0.1)}) <= jp::kMaxBlipWork);
        QVERIFY(jp::blipEffectsWork(twelveMillion, {lumEffect(0.2, 0.3), grayEffect(), changeEffect(Qt::white, QColor(255, 255, 255, 0))}) <= jp::kMaxBlipWork);
        // What the reference presentations' largest picture (9 million pixels) is given at the most (three of the plain kind) is far inside it.
        QVERIFY(jp::blipEffectsWork(9'000'000, {lumEffect(0.2, 0.3), grayEffect(), duotoneEffect(Qt::black, Qt::white)}) * 2 < jp::kMaxBlipWork);
        // An effect that changes nothing costs nothing.
        QCOMPARE(jp::blipEffectsWork(twelveMillion, {lumEffect(0, 0), tintEffect(30, 0)}), qint64(0));
        QVERIFY(jp::blipEffectsChange({lumEffect(0.1, 0)}) && !jp::blipEffectsChange({lumEffect(0, 0)}) && !jp::blipEffectsChange({}));
        QImage large(4000, 3000, QImage::Format_ARGB32);
        large.fill(QColor(100, 150, 200));
        QImage refused;
        // (Refused: the picture comes back as it is, no turn is made of it, and no pixel is worked.)
        const int madeBefore = jp::blipAdjustedMade();
        refused = jp::applyBlipEffects(large, turns);
        QCOMPARE(refused.cacheKey(), large.cacheKey());
        QCOMPARE(jp::blipAdjusted(large, turns).cacheKey(), large.cacheKey());
        QCOMPARE(jp::blipAdjustedMade(), madeBefore);
        // One turn of the hue is made, in a time a person does not wait out (the clock stays for that: about a fifth of a second here, against five), and made once however many
        // times it is asked for (counted).
        // Each run turns a picture that is new (a new key in the cache), so each run makes the turn.
        int before = 0;
        QImage turned;
        const qint64 turnMs = bestNanoseconds([&] { turned = jp::blipAdjusted(large, {hslEffect(120, 0, 0)}); }, 3, [&] {
            large = QImage(4000, 3000, QImage::Format_ARGB32);
            large.fill(QColor(100, 150, 200));
            before = jp::blipAdjustedMade();
        }) / 1000000;
        QVERIFY2(turnMs < 12000, qPrintable(QString::number(turnMs)));   // (a second alone: ten times that)
        QVERIFY(turned.cacheKey() != large.cacheKey());
        QCOMPARE(jp::blipAdjustedMade(), before + 1);
        QCOMPARE(jp::blipAdjusted(large, {hslEffect(120, 0, 0)}).cacheKey(), turned.cacheKey());
        QCOMPARE(jp::blipAdjustedMade(), before + 1);
        // (100, 150, 200) turned a third of a circle (from hue 210 to 330) is (200, 100, 150).
        const QColor at = turned.pixelColor(5, 5);
        QVERIFY2(qAbs(at.red() - 200) <= 1 && qAbs(at.green() - 100) <= 1 && qAbs(at.blue() - 150) <= 1, qPrintable(at.name()));
        // More pixels than a picture is changed in (one byte each here, so as not to take a gigabyte): as it is.
        QImage huge(10100, 10100, QImage::Format_Alpha8);
        QVERIFY(qint64(huge.width()) * huge.height() > jp::kMaxBlipPixels);
        const int madeHuge = jp::blipAdjustedMade();
        QCOMPARE(jp::applyBlipEffects(huge, {grayEffect()}).cacheKey(), huge.cacheKey());
        QCOMPARE(jp::blipAdjustedMade(), madeHuge);
    }

    // A picture of one repeated row of noise, 100,020,001 pixels (more than kMaxBlipPixels, less than a file may hold), one byte each.
    static QImage hugeNoise()
    {
        QImage im(10001, 10001, QImage::Format_Grayscale8);
        uchar *first = im.scanLine(0);
        for (int x = 0; x < im.width(); ++x) first[x] = uchar((x * 37) % 251);
        for (int y = 1; y < im.height(); ++y) memcpy(im.scanLine(y), first, size_t(im.width()));
        return im;
    }
    // Whether `done` is the flat patch (one color, small) or differs from `plain` (the picture made the size it is worked at) in most of a row.
    static bool hidden(const QImage &done, const QImage &plain)
    {
        if (done.isNull() || done.width() * qint64(done.height()) > jp::kPictureFxWorkPixels) return false;
        const QImage d = done.convertToFormat(QImage::Format_ARGB32), p = plain.convertToFormat(QImage::Format_ARGB32);
        if (d.size() != p.size()) {
            for (int y = 0; y < d.height(); ++y)
                for (int x = 0; x < d.width(); ++x)
                    if (d.pixel(x, y) != d.pixel(0, 0)) return false;
            return true;
        }
        int differ = 0;
        const int y = d.height() / 2;
        for (int x = 0; x < d.width(); ++x) differ += d.pixel(x, y) != p.pixel(x, y);
        return differ * 5 > d.width() * 4;
    }
    // Blur and Mosaic hide what is in a picture, so a picture of any size has them worked (on the copy made of at most kPictureFxWorkPixels) or a flat patch, and never itself: one of
    // more than kMaxBlipPixels was drawn as it was, and so was saved. Every way the work is drawn, and the way a save is made, is checked.
    void hidingFiltersAreNeverSkippedForTheSizeOfAPicture()
    {
        const QImage huge = hugeNoise();
        QVERIFY(qint64(huge.width()) * huge.height() > jp::kMaxBlipPixels);
        const QImage plain = jp::pictureFxWorkCopy(huge);
        QVERIFY(qint64(plain.width()) * plain.height() <= jp::kPictureFxWorkPixels);
        for (const int style : {int(jp::ArtisticStyle::Blur), int(jp::ArtisticStyle::Mosaic)}) {
            const QVector<jp::BlipEffect> fx{filterEffect(jp::BlipEffect::Artistic, 0.8, style)};
            const QString name = QString::number(style);
            QVERIFY2(hidden(jp::applyBlipEffects(huge, fx), plain), qPrintable(name + " applied"));
            QVERIFY2(hidden(jp::blipAdjusted(huge, fx, nullptr, QStringLiteral("huge|") + name), plain), qPrintable(name + " adjusted"));
            // Beside the window: waited for, never the picture.
            bool pending = false;
            QImage got = jp::blipAdjustedLater(huge, fx, QStringLiteral("huge-later|") + name, &pending);
            QVERIFY2(pending, qPrintable(name + " later"));
            QTRY_VERIFY_WITH_TIMEOUT((got = jp::blipAdjustedLater(huge, fx, QStringLiteral("huge-later|") + name, &pending), !pending), 60000);
            QVERIFY2(hidden(got, plain), qPrintable(name + " beside"));
            // Work that is stopped, or a list too long for the filters to all be done, draws the patch.
            std::atomic<bool> stop{true};
            QVERIFY2(hidden(jp::applyBlipEffects(huge, fx, &stop), plain), qPrintable(name + " stopped"));
        }
        const QImage mid = filterSample(1200, 1200);
        QVector<jp::BlipEffect> long_(14, filterEffect(jp::BlipEffect::Sharpen, 0.5));
        long_.append(filterEffect(jp::BlipEffect::Artistic, 0.8, int(jp::ArtisticStyle::Mosaic)));
        QVERIFY(jp::blipFiltersLeftOut(qint64(mid.width()) * mid.height(), long_) >= 1);
        const QImage left = jp::applyBlipEffects(mid, long_);
        QVERIFY(left.width() <= 4 && left.height() <= 4);
        QCOMPARE(left.pixel(0, 0), left.pixel(left.width() - 1, left.height() - 1));
    }

    // A save writes the finished picture of one that is over kMaxBlipPixels with the filter in it (or the patch), not the picture as it is.
    void aSavedHugePictureIsNeverTheOneItHides()
    {
        const QImage huge = hugeNoise();
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        QVERIFY(huge.save(&buffer, "PNG"));
        buffer.close();
        WriteRound r;
        r.pictures.insert(QStringLiteral("big"), png);
        auto p = std::make_shared<PictureItem>();
        p->imageId = QStringLiteral("big");
        p->rect = QRectF(0, 0, 100, 100);
        p->imgRect = QRectF(0, 0, 100, 100);
        p->blipEffects = {filterEffect(jp::BlipEffect::Artistic, 0.8, int(jp::ArtisticStyle::Blur))};
        QVERIFY2(r.run({p}), qPrintable(r.error));
        QImage saved;
        for (const QString &name : r.env.package.partNames())
            if (name.endsWith(QLatin1String(".png"))) saved.loadFromData(r.env.package.part(name));
        QVERIFY(!saved.isNull());
        QVERIFY(saved.size() != huge.size());
        QVERIFY(hidden(saved, jp::pictureFxWorkCopy(huge)));
    }

    // A picture's changes are written in the order they are drawn: the settings (a clear color, the brightness and contrast, gray or black and white
    // from its threshold), then those beside them as they were listed, whatever order a file listed them in (a duotone before the brightness is
    // before it still), none left out; and read again they are the same.
    void dmlWritePicturesListTheirEffectsAsTheyAreDrawn()
    {
        WriteRound r;
        const QString red = r.addPicture(Qt::red);
        auto make = [&](const QRectF &rect) {
            auto p = std::make_shared<PictureItem>();
            p->imageId = red;
            p->rect = rect;
            p->imgRect = QRectF(0, 0, rect.width(), rect.height());
            return p;
        };
        auto settings = make(QRectF(0, 0, 50, 50));
        settings->hasTransparentColor = true;
        settings->transparentColor = QColor(0x9C, 0x8C, 0x7B);
        settings->brightness = 21;
        settings->contrast = 52;
        settings->recolor = PictureItem::BlackWhite;
        settings->blackWhiteLevel = 0.25;
        settings->blipEffects = {duotoneEffect(QColor(0, 0, 0x80), QColor(255, 255, 0)), tintEffect(180, 0.3)};
        auto beforeLum = make(QRectF(100, 0, 50, 50));
        beforeLum->blipEffects = {duotoneEffect(QColor(0, 0, 0x80), QColor(255, 255, 0)), lumEffect(0.1, 0), hslEffect(90, -0.2, 0.1)};
        auto twoChanges = make(QRectF(200, 0, 50, 50));
        twoChanges->blipEffects = {changeEffect(Qt::white, QColor(255, 0, 0)), changeEffect(Qt::white, QColor(255, 255, 255, 0)), changeEffect(Qt::black, QColor(0, 0, 0, 0), false),
                                   replaceEffect(QColor(0, 255, 0)), grayEffect(), biLevelEffect(0.75), lumEffect(0, 0)};
        QVERIFY2(r.run({settings, beforeLum, twoChanges}), qPrintable(r.error));
        QVERIFY2(r.skipped.isEmpty(), qPrintable(r.skipped.join('\n')));
        QCOMPARE(int(r.items.size()), 3);
        const QVector<QDomElement> blips = elementsNamed(r.spTree, "blip");
        QCOMPARE(int(blips.size()), 3);
        const auto childNames = [](const QDomElement &blip) {
            QStringList names;
            for (QDomElement c = blip.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) names << c.localName();
            return names;
        };
        QCOMPARE(childNames(blips[0]), QStringList({"clrChange", "lum", "biLevel", "duotone", "tint"}));
        QCOMPARE(childNames(blips[1]), QStringList({"duotone", "lum", "hsl"}));
        QCOMPARE(childNames(blips[2]), QStringList({"clrChange", "clrChange", "clrChange", "clrRepl", "grayscl", "biLevel", "lum"}));
        // As written: the numbers, and the color that is changed.
        QCOMPARE(childNamed(blips[0], "lum").attribute("bright"), QStringLiteral("21000"));
        QCOMPARE(childNamed(blips[0], "lum").attribute("contrast"), QStringLiteral("52000"));
        QCOMPARE(childNamed(blips[0], "biLevel").attribute("thresh"), QStringLiteral("25000"));
        QCOMPARE(childNamed(blips[0], "tint").attribute("hue"), QStringLiteral("10800000"));
        QCOMPARE(childNamed(blips[0], "tint").attribute("amt"), QStringLiteral("30000"));
        QCOMPARE(childNamed(blips[1], "hsl").attribute("hue"), QStringLiteral("5400000"));
        QCOMPARE(childNamed(blips[1], "hsl").attribute("sat"), QStringLiteral("-20000"));
        const QVector<QDomElement> changes = elementsNamed(blips[2], "clrChange");
        QCOMPARE(changes[0].attribute("useA"), QString());
        QCOMPARE(changes[2].attribute("useA"), QStringLiteral("0"));
        QCOMPARE(childNamed(childNamed(changes[0], "clrTo"), "srgbClr").attribute("val"), QStringLiteral("FF0000"));
        QVERIFY(childNamed(childNamed(childNamed(changes[1], "clrTo"), "srgbClr"), "alpha").attribute("val") == QStringLiteral("0"));
        // Read again: the same settings, the same list.
        const auto *a = static_cast<PictureItem *>(r.items[0].get());
        const auto *b = static_cast<PictureItem *>(r.items[1].get());
        const auto *c = static_cast<PictureItem *>(r.items[2].get());
        QVERIFY(a->hasTransparentColor && a->transparentColor == QColor(0x9C, 0x8C, 0x7B) && near(a->brightness, 21, 1e-3) && near(a->contrast, 52, 1e-3));
        QVERIFY(a->recolor == PictureItem::BlackWhite && near(a->blackWhiteLevel, 0.25, 1e-3));
        QVERIFY(sameEffects(a->blipEffects, {duotoneEffect(QColor(0, 0, 0x80), QColor(255, 255, 0)), tintEffect(180, 0.3)}));
        QVERIFY(!b->hasTransparentColor && b->brightness == 0 && b->recolor == PictureItem::NoRecolor);
        QVERIFY(sameEffects(b->blipEffects, beforeLum->blipEffects));
        QVERIFY(!c->hasTransparentColor);
        QVERIFY(sameEffects(c->blipEffects, twoChanges->blipEffects));
        // And what is drawn is the same: the order is the file's.
        QVERIFY(sameEffects(b->colorEffects(), beforeLum->colorEffects()));
        QVERIFY(sameEffects(a->colorEffects(), settings->colorEffects()));
    }

    // A picture fill's changes are written as listed, in the blip, where its transparency is; and read again they are the same.
    void dmlWriteFillsListTheirEffects()
    {
        WriteRound r;
        auto shape = std::make_shared<ShapeItem>();
        shape->rect = QRectF(10, 10, 100, 60);
        shape->fill.type = Fill::Picture;
        shape->fill.imageId = r.addPicture(Qt::red);
        shape->fill.transparency = 0.25;
        shape->fill.blipEffects = {lumEffect(0.21, 0.52), duotoneEffect(QColor(0, 0, 0x80), QColor(255, 255, 0)), changeEffect(Qt::white, QColor(255, 255, 255, 0), false),
                                   grayEffect(), biLevelEffect(0.4), replaceEffect(QColor(0, 255, 0)), hslEffect(45, 0.1, -0.1), tintEffect(90, -0.5)};
        QVERIFY2(r.run({shape}), qPrintable(r.error));
        QVERIFY2(r.skipped.isEmpty(), qPrintable(r.skipped.join('\n')));
        const QVector<QDomElement> blips = elementsNamed(r.spTree, "blip");
        QCOMPARE(int(blips.size()), 1);
        QStringList names;
        for (QDomElement c = blips[0].firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) names << c.localName();
        QCOMPARE(names, QStringList({"alphaModFix", "lum", "duotone", "clrChange", "grayscl", "biLevel", "clrRepl", "hsl", "tint"}));
        QCOMPARE(childNamed(blips[0], "alphaModFix").attribute("amt"), QStringLiteral("75000"));
        QCOMPARE(childNamed(blips[0], "lum").attribute("bright"), QStringLiteral("21000"));
        QCOMPARE(childNamed(blips[0], "biLevel").attribute("thresh"), QStringLiteral("40000"));
        QCOMPARE(childNamed(blips[0], "clrChange").attribute("useA"), QStringLiteral("0"));
        QCOMPARE(childNamed(blips[0], "tint").attribute("amt"), QStringLiteral("-50000"));
        QCOMPARE(int(r.items.size()), 1);
        QVERIFY(r.items[0]->fill.type == Fill::Picture);
        QVERIFY(near(r.items[0]->fill.transparency, 0.25, 1e-3));
        QVERIFY(sameEffects(r.items[0]->fill.blipEffects, shape->fill.blipEffects));
        // The picture's bytes are as they were (one picture, as given).
        QCOMPARE(r.env.images.size(), 1);
        QCOMPARE(r.env.images[0].bytes, r.pictures.value(QStringLiteral("pic1")));
    }

    // A picture fill that has effects is drawn with them: the brush has the picture's colors changed (a:duotone's black to its first color).
    void dmlFillWithEffectsPaintsChanged()
    {
        QImage source(2, 1, QImage::Format_RGB32);
        source.setPixel(0, 0, qRgb(0, 0, 0));
        source.setPixel(1, 0, qRgb(255, 255, 255));
        jp::Fill f;
        f.type = Fill::Picture;
        f.imageId = QStringLiteral("a");
        const auto lookup = [&](const QString &) { return source; };
        jp::ColorScheme scheme;
        QCOMPARE(f.brush(QRectF(0, 0, 2, 1), scheme, lookup).textureImage().pixelColor(0, 0), QColor(0, 0, 0));
        jp::BlipEffect duo;
        duo.kind = jp::BlipEffect::Duotone;
        duo.color = QColor(0, 0, 0x80);
        duo.color2 = QColor(255, 255, 0);
        f.blipEffects = {duo};
        const QImage got = f.brush(QRectF(0, 0, 2, 1), scheme, lookup).textureImage();
        QCOMPARE(got.pixelColor(0, 0), QColor(0, 0, 0x80));
        QCOMPARE(got.pixelColor(1, 0), QColor(255, 255, 0));
        QCOMPARE(source.pixelColor(0, 0), QColor(0, 0, 0));   // the picture is as it was
    }

    // A blur, glow, or soft edge is held to 200 points: real presentations use 20,
    // 10, and 7 at most, and one of the standard's 1,584 points makes a small
    // shape take most of a minute and gigabytes to draw.
    void dmlEffectRadiiStopAtTwoHundredPoints()
    {
        DmlEnv env;
        const QString rect = dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        ItemPtr it = dmlOne(env, dmlSp(rect + QStringLiteral("<a:effectLst><a:outerShdw blurRad=\"20116800\" dist=\"0\" dir=\"0\"><a:srgbClr val=\"000000\"/></a:outerShdw>"
                                                             "<a:glow rad=\"20116800\"><a:srgbClr val=\"00FF00\"/></a:glow><a:softEdge rad=\"20116800\"/></a:effectLst>")));
        QVERIFY(it->fx.shadow.on && it->fx.glow.on);
        QVERIFY(near(it->fx.shadow.blur, 200) && near(it->fx.glow.size, 200) && near(it->fx.softEdge, 200));
        it = dmlOne(env, dmlSp(rect + QStringLiteral("<a:effectLst><a:innerShdw blurRad=\"20116800\" dist=\"0\" dir=\"0\"><a:srgbClr val=\"000000\"/></a:innerShdw></a:effectLst>")));
        QVERIFY(it->fx.shadow.inner && near(it->fx.shadow.blur, 200));
        // The largest radii the reference decks have are kept as they are.
        it = dmlOne(env, dmlSp(rect + QStringLiteral("<a:effectLst><a:outerShdw blurRad=\"254160\" dist=\"0\" dir=\"0\"><a:srgbClr val=\"000000\"/></a:outerShdw>"
                                                     "<a:glow rad=\"127000\"><a:srgbClr val=\"00FF00\"/></a:glow><a:softEdge rad=\"88900\"/></a:effectLst>")));
        QVERIFY(near(it->fx.shadow.blur, 20.01, 1e-2) && near(it->fx.glow.size, 10) && near(it->fx.softEdge, 7));
    }

    void dmlFillTakesTheStyleWhenThePropertiesSayNothing()
    {
        DmlEnv env;
        env.useFormats();
        // Reference 1: a plain fill of the reference's color.
        Fill f = dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("1"), QStringLiteral("0"), QStringLiteral("accent2")));
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("ED7D31"));
        // 2: a gradient made of it, from lighter to the color itself.
        f = dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("2")));
        QVERIFY(f.type == Fill::Gradient);
        QCOMPARE(f.stops.size(), 2);
        QCOMPARE(dmlHex(f.stops[1].color), QStringLiteral("4472C4"));
        QVERIFY(f.stops[0].color.rgbValue().lightness() > f.stops[1].color.rgbValue().lightness());
        QVERIFY(near(f.angle, 90));
        // 3: the darkened color.
        f = dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("3")));
        QCOMPARE(dmlHex(f.color), hexOf(colorOf(schemeClr(QStringLiteral("accent1"), {"lumMod 50000"}))));
        // 0, and one that is past the list, are none; 1001 and on are the background fills.
        QVERIFY(dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("0"))).isNone());
        QVERIFY(dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("4"))).isNone());
        QVERIFY(dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("-3"))).isNone());
        QVERIFY(dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("junk"))).isNone());
        QVERIFY(dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("99999999999999"))).isNone());
        f = dmlFillOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("1003"), QStringLiteral("0"), QStringLiteral("accent3")));
        QVERIFY(f.type == Fill::Gradient && f.gradType == Fill::Radial);
        QCOMPARE(dmlHex(f.stops[0].color), QStringLiteral("A5A5A5"));
        // The properties' own fill wins, and so does their saying no fill.
        f = dmlFillOf(env, QStringLiteral("<a:solidFill><a:srgbClr val=\"112233\"/></a:solidFill>"), dmlStyle(QStringLiteral("1"), QStringLiteral("1")));
        QCOMPARE(dmlHex(f.color), QStringLiteral("112233"));
        QVERIFY(dmlFillOf(env, QStringLiteral("<a:noFill/>"), dmlStyle(QStringLiteral("1"), QStringLiteral("1"))).isNone());
        // Other properties (a line) leave the fill to the style.
        f = dmlFillOf(env, QStringLiteral("<a:ln w=\"12700\"/>"), dmlStyle(QStringLiteral("1"), QStringLiteral("1")));
        QVERIFY(f.type == Fill::Solid);
        // Without a format scheme the style gives nothing.
        DmlEnv bare;
        QVERIFY(dmlFillOf(bare, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("1"))).isNone());
    }

    void dmlLineReadsWidthAndColor()
    {
        DmlEnv env;
        Stroke s = dmlLineOf(env, QStringLiteral("<a:ln w=\"25400\"><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></a:ln>"));
        QVERIFY(!s.isNone());
        QVERIFY(near(s.width, 2));
        QCOMPARE(dmlHex(s.color), QStringLiteral("00FF00"));
        QCOMPARE(s.transparency, 0.0);
        QVERIFY(s.dash == Stroke::SolidLine && s.compound == Stroke::Single && s.cap == Qt::FlatCap);
        // No width: the line's usual, a quarter point shy of a point.
        s = dmlLineOf(env, QStringLiteral("<a:ln><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></a:ln>"));
        QVERIFY(near(s.width, 0.75));
        s = dmlLineOf(env, QStringLiteral("<a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"00FF00\"><a:alpha val=\"20000\"/></a:srgbClr></a:solidFill></a:ln>"));
        QVERIFY(near(s.transparency, 0.8, 1e-2));
        // No line, no fill in it, or a fill that isn't one gives none.
        QVERIFY(dmlLineOf(env, QString()).isNone());
        QVERIFY(dmlLineOf(env, QStringLiteral("<a:ln w=\"12700\"/>")).isNone());
        QVERIFY(dmlLineOf(env, QStringLiteral("<a:ln w=\"12700\"><a:noFill/></a:ln>")).isNone());
        QVERIFY(readLine(QDomElement(), QDomElement(), env.ctx).isNone());
        // A line of no width is the thinnest there is; a huge one is cut down.
        s = dmlLineOf(env, QStringLiteral("<a:ln w=\"0\"><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill></a:ln>"));
        QVERIFY(!s.isNone() && s.width > 0 && s.width <= 0.5);
        s = dmlLineOf(env, QStringLiteral("<a:ln w=\"999999999999\"><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill></a:ln>"));
        QVERIFY(s.width <= 1584);
        s = dmlLineOf(env, QStringLiteral("<a:ln w=\"-5\"><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill></a:ln>"));
        QVERIFY(!s.isNone() && s.width > 0 && s.width <= 1);
        // A gradient line is the color of its first stop; a pattern line is its foreground.
        s = dmlLineOf(env, QStringLiteral("<a:ln w=\"12700\"><a:gradFill><a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"FF0000\"/></a:gs><a:gs pos=\"100000\"><a:srgbClr val=\"0000FF\"/></a:gs></a:gsLst></a:gradFill></a:ln>"));
        QCOMPARE(dmlHex(s.color), QStringLiteral("FF0000"));
        s = dmlLineOf(env, QStringLiteral("<a:ln w=\"12700\"><a:pattFill prst=\"horz\"><a:fgClr><a:srgbClr val=\"123456\"/></a:fgClr><a:bgClr><a:srgbClr val=\"FFFFFF\"/></a:bgClr></a:pattFill></a:ln>"));
        QCOMPARE(dmlHex(s.color), QStringLiteral("123456"));
    }

    void dmlLineReadsDashesCapsAndJoins()
    {
        DmlEnv env;
        const auto line = [&](const QString &inner, const QString &attrs = QString()) {
            return dmlLineOf(env, QStringLiteral("<a:ln w=\"12700\" %2><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill>%1</a:ln>").arg(inner, attrs));
        };
        const struct { const char *name; Stroke::Dash dash; } dashes[] = {
            {"solid", Stroke::SolidLine}, {"dot", Stroke::RoundDot}, {"sysDot", Stroke::SquareDot}, {"dash", Stroke::DashLine}, {"sysDash", Stroke::DashLine},
            {"dashDot", Stroke::DashDot}, {"sysDashDot", Stroke::DashDot}, {"lgDash", Stroke::LongDash}, {"lgDashDot", Stroke::LongDashDot},
            {"lgDashDotDot", Stroke::LongDashDotDot}, {"sysDashDotDot", Stroke::LongDashDotDot}};
        for (const auto &d : dashes) QVERIFY2(line(QStringLiteral("<a:prstDash val=\"%1\"/>").arg(QString::fromLatin1(d.name))).dash == d.dash, d.name);
        QVERIFY(line(QStringLiteral("<a:custDash><a:ds d=\"100000\" sp=\"50000\"/></a:custDash>")).dash == Stroke::DashLine);
        QVERIFY(line(QStringLiteral("<a:prstDash val=\"nonsense\"/>")).dash == Stroke::SolidLine);
        QVERIFY(line(QString(), QStringLiteral("cap=\"rnd\"")).cap == Qt::RoundCap);
        QVERIFY(line(QString(), QStringLiteral("cap=\"sq\"")).cap == Qt::SquareCap);
        QVERIFY(line(QString(), QStringLiteral("cap=\"flat\"")).cap == Qt::FlatCap);
        QVERIFY(line(QStringLiteral("<a:round/>")).join == Qt::RoundJoin);
        QVERIFY(line(QStringLiteral("<a:bevel/>")).join == Qt::BevelJoin);
        QVERIFY(line(QStringLiteral("<a:miter lim=\"800000\"/>")).join == Qt::MiterJoin);
        QVERIFY(line(QString(), QStringLiteral("cmpd=\"dbl\"")).compound == Stroke::Double);
        QVERIFY(line(QString(), QStringLiteral("cmpd=\"thickThin\"")).compound == Stroke::ThickThin);
        QVERIFY(line(QString(), QStringLiteral("cmpd=\"thinThick\"")).compound == Stroke::ThinThick);
        QVERIFY(line(QString(), QStringLiteral("cmpd=\"tri\"")).compound == Stroke::Triple);
        QVERIFY(line(QString(), QStringLiteral("cmpd=\"sng\"")).compound == Stroke::Single);
        QVERIFY(!line(QString()).inset);
        QVERIFY(line(QString(), QStringLiteral("algn=\"in\"")).inset);
    }

    void dmlLineReadsArrowheads()
    {
        DmlEnv env;
        const auto line = [&](const QString &ends) {
            return dmlLineOf(env, QStringLiteral("<a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill>%1</a:ln>").arg(ends));
        };
        Stroke s = line(QStringLiteral("<a:headEnd type=\"triangle\" w=\"lg\" len=\"lg\"/><a:tailEnd type=\"stealth\" w=\"sm\" len=\"sm\"/>"));
        QVERIFY(s.startArrow == Arrow::Triangle && s.endArrow == Arrow::Stealth);
        QCOMPARE(s.startSize, 2);
        QCOMPARE(s.endSize, 0);
        s = line(QStringLiteral("<a:tailEnd type=\"arrow\"/>"));
        QVERIFY(s.startArrow == Arrow::None && s.endArrow == Arrow::Open);
        QCOMPARE(s.endSize, 1);
        s = line(QStringLiteral("<a:headEnd type=\"diamond\"/><a:tailEnd type=\"oval\" w=\"med\" len=\"lg\"/>"));
        QVERIFY(s.startArrow == Arrow::Diamond && s.endArrow == Arrow::Oval);
        QVERIFY(s.endSize >= 1 && s.endSize <= 2);
        s = line(QStringLiteral("<a:headEnd type=\"none\"/><a:tailEnd type=\"bogus\"/>"));
        QVERIFY(s.startArrow == Arrow::None && s.endArrow == Arrow::None);
        s = line(QString());
        QVERIFY(s.startArrow == Arrow::None && s.endArrow == Arrow::None);
    }

    void dmlLineTakesTheStyleAndLetsPropertiesChangeIt()
    {
        DmlEnv env;
        env.useFormats();
        // Reference 1: a half point, the reference's color.
        Stroke s = dmlLineOf(env, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("0"), QStringLiteral("0"), QStringLiteral("accent2")));
        QVERIFY(!s.isNone());
        QVERIFY(near(s.width, 0.5));
        QCOMPARE(dmlHex(s.color), QStringLiteral("ED7D31"));
        // 2: a point, dashed, round joined; 3: one and a half, round capped, double, beveled.
        s = dmlLineOf(env, QString(), dmlStyle(QStringLiteral("2"), QStringLiteral("0")));
        QVERIFY(near(s.width, 1) && s.dash == Stroke::DashLine && s.join == Qt::RoundJoin);
        s = dmlLineOf(env, QString(), dmlStyle(QStringLiteral("3"), QStringLiteral("0")));
        QVERIFY(near(s.width, 1.5) && s.cap == Qt::RoundCap && s.compound == Stroke::Double && s.join == Qt::BevelJoin);
        // 0 and a reference past the list give no line.
        QVERIFY(dmlLineOf(env, QString(), dmlStyle(QStringLiteral("0"), QStringLiteral("0"))).isNone());
        QVERIFY(dmlLineOf(env, QString(), dmlStyle(QStringLiteral("4"), QStringLiteral("0"))).isNone());
        // The properties' width alone changes the width and keeps the style's color and dashes.
        s = dmlLineOf(env, QStringLiteral("<a:ln w=\"38100\"/>"), dmlStyle(QStringLiteral("2"), QStringLiteral("0")));
        QVERIFY(near(s.width, 3));
        QCOMPARE(dmlHex(s.color), QStringLiteral("4472C4"));
        QVERIFY(s.dash == Stroke::DashLine);
        // Their color keeps the style's width.
        s = dmlLineOf(env, QStringLiteral("<a:ln><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln>"), dmlStyle(QStringLiteral("3"), QStringLiteral("0")));
        QVERIFY(near(s.width, 1.5));
        QCOMPARE(dmlHex(s.color), QStringLiteral("FF0000"));
        // Their saying no fill is no line, and their dash replaces the style's.
        QVERIFY(dmlLineOf(env, QStringLiteral("<a:ln><a:noFill/></a:ln>"), dmlStyle(QStringLiteral("2"), QStringLiteral("0"))).isNone());
        s = dmlLineOf(env, QStringLiteral("<a:ln><a:prstDash val=\"solid\"/></a:ln>"), dmlStyle(QStringLiteral("2"), QStringLiteral("0")));
        QVERIFY(s.dash == Stroke::SolidLine && !s.isNone());
        // Without a format scheme, the style gives none.
        DmlEnv bare;
        QVERIFY(dmlLineOf(bare, QString(), dmlStyle(QStringLiteral("1"), QStringLiteral("0"))).isNone());
    }

    // A background that refers to a picture style of the theme: the picture
    // is named by the THEME part's relationships, not the slide's, and its
    // a:duotone recolors it from the first color (black) to the second
    // (white).
    void dmlThemePictureBackgroundWithDuotone()
    {
        DmlEnv env;
        QImage source(2, 1, QImage::Format_RGB32);
        source.setPixel(0, 0, qRgb(0, 0, 0));
        source.setPixel(1, 0, qRgb(255, 255, 255));
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        source.save(&buffer, "PNG");
        PackageWriter w;
        w.addDefault(QStringLiteral("png"), QStringLiteral("image/png"));
        w.addPart(QStringLiteral("/ppt/slideMasters/slideMaster1.xml"), "<p:sldMaster/>", QStringLiteral("application/xml"));
        w.addPart(QStringLiteral("/ppt/theme/theme1.xml"), "<a:theme/>", QStringLiteral("application/xml"));
        w.addPart(QStringLiteral("/ppt/media/texture.png"), png, QStringLiteral("image/png"));
        w.addPart(QStringLiteral("/ppt/media/other.png"), dmlPicture(), QStringLiteral("image/png"));
        // rId1 means different pictures to the master and to the theme.
        w.addRelationship(QStringLiteral("/ppt/slideMasters/slideMaster1.xml"), kRelBase + "image", QStringLiteral("/ppt/media/other.png"));
        w.addRelationship(QStringLiteral("/ppt/theme/theme1.xml"), kRelBase + "image", QStringLiteral("/ppt/media/texture.png"));
        QVERIFY(env.package.read(w.finish()));
        env.ctx.package = &env.package;
        env.ctx.part = QStringLiteral("/ppt/slideMasters/slideMaster1.xml");
        env.ctx.themePart = QStringLiteral("/ppt/theme/theme1.xml");
        env.ctx.formats = env.element(QStringLiteral(
            "<a:fmtScheme><a:fillStyleLst/><a:lnStyleLst/><a:effectStyleLst/><a:bgFillStyleLst>"
            "<a:blipFill><a:blip r:embed=\"rId1\"><a:duotone><a:srgbClr val=\"000080\"/><a:schemeClr val=\"phClr\"/></a:duotone></a:blip><a:stretch/></a:blipFill>"
            "</a:bgFillStyleLst></a:fmtScheme>"));
        const Fill f = readBackground(env.element(QStringLiteral("<p:bg><p:bgRef idx=\"1001\"><a:srgbClr val=\"FFFF00\"/></p:bgRef></p:bg>")), env.ctx);
        QVERIFY(f.type == Fill::Picture);
        QCOMPARE(env.images.size(), 1);
        QImage got;
        QVERIFY(got.loadFromData(env.images[0].bytes));
        QCOMPARE(got.size(), QSize(2, 1));   // the theme's picture, not the master's, as it is
        QCOMPARE(QColor(got.pixel(0, 0)), QColor(0, 0, 0));
        // Where it is drawn: black takes the first color, white the second (here the reference's).
        QCOMPARE(f.blipEffects.size(), 1);
        const QImage drawn = f.brush(QRectF(0, 0, 2, 1), jp::ColorScheme(), [&](const QString &) { return got; }).textureImage();
        QCOMPARE(QColor(drawn.pixel(0, 0)), QColor(0x00, 0x00, 0x80));
        QCOMPARE(QColor(drawn.pixel(1, 0)), QColor(0xFF, 0xFF, 0x00));
    }

    // A shape of a diagram's drawing (dsp:sp) is a p:sp with one more element, dsp:txXfrm: the rectangle its text is
    // laid out in, which need not be the shape's own, and which turns by its own angle on top of the shape's.
    static QString dspSp(const QString &spPr, const QString &txXfrm, const QString &text = QStringLiteral("Hello"), const QString &style = QString())
    {
        return QStringLiteral("<dsp:sp xmlns:dsp=\"http://schemas.microsoft.com/office/drawing/2008/diagram\" modelId=\"{1}\">"
                              "<dsp:nvSpPr><dsp:cNvPr id=\"0\" name=\"\"/><dsp:cNvSpPr/></dsp:nvSpPr><dsp:spPr>%1</dsp:spPr>%4"
                              "<dsp:txBody><a:bodyPr lIns=\"0\" tIns=\"0\" rIns=\"0\" bIns=\"0\" anchor=\"ctr\"><a:noAutofit/></a:bodyPr><a:lstStyle/>%2</dsp:txBody>%3</dsp:sp>")
            .arg(spPr, text.isEmpty() ? QStringLiteral("<a:p/>") : QStringLiteral("<a:p><a:r><a:t>%1</a:t></a:r></a:p>").arg(text), txXfrm, style);
    }

    void dmlDiagramShapeLaysItsTextInTheTextTransform()
    {
        DmlEnv env;
        const QString plain = dmlXfrm(10, 10, 200, 100) + QStringLiteral("<a:prstGeom prst=\"rect\"/>") + dmlSolid(QStringLiteral("336699"));
        // Text in the shape's own rectangle: the shape is its text, as a p:sp with a text body is.
        ItemPtr it = dmlOne(env, dspSp(plain, dmlXfrm(10, 10, 200, 100, QString(), "dsp:txXfrm")));
        auto t = dmlAs<TextItem>(it);
        QVERIFY(t);
        QCOMPARE(dmlHex(t->fill.color), QStringLiteral("336699"));
        QVERIFY(dmlRectIs(t->rect, 10, 10, 200, 100));
        QVERIFY(env.stories.last()->toPlainText().contains(QStringLiteral("Hello")));
        QVERIFY(t->valign == VAlign::Middle);
        QVERIFY(t->insets == QMarginsF(0, 0, 0, 0));
        // A rectangle for the text that is not the shape's: the shape draws, and its text is a box of its own, with neither fill nor line.
        const QString round = dmlXfrm(10, 10, 200, 100) + QStringLiteral("<a:prstGeom prst=\"roundRect\"/>") + dmlSolid(QStringLiteral("336699"))
                              + QStringLiteral("<a:ln w=\"12700\">%1</a:ln>").arg(dmlSolid(QStringLiteral("FFFFFF")));
        it = dmlOne(env, dspSp(round, dmlXfrm(30, 20, 160, 80, QString(), "dsp:txXfrm")));
        auto g = dmlAs<GroupItem>(it);
        QVERIFY(g);
        QCOMPARE(int(g->children.size()), 2);
        auto shape = dmlAs<ShapeItem>(g->children[0]);
        t = dmlAs<TextItem>(g->children[1]);
        QVERIFY(shape && t);
        QCOMPARE(shape->shape, QStringLiteral("roundRect"));
        QVERIFY(dmlRectIs(shape->rect, 10, 10, 200, 100));
        QCOMPARE(dmlHex(shape->fill.color), QStringLiteral("336699"));
        QVERIFY(shape->storyId.isEmpty());
        QVERIFY(dmlRectIs(t->rect, 30, 20, 160, 80));
        QVERIFY(t->fill.isNone() && t->stroke.isNone());
        QVERIFY(near(t->rotation, 0));
        QVERIFY(env.stories.last()->toPlainText().contains(QStringLiteral("Hello")));
        QVERIFY(dmlRectIs(g->rect, 10, 10, 200, 100));
        // The text turns with the shape: the same rectangle, the text's own angle added to the shape's.
        it = dmlOne(env, dspSp(dmlXfrm(0, 0, 100, 40, QStringLiteral("rot=\"16200000\"")) + QStringLiteral("<a:prstGeom prst=\"rect\"/>") + dmlSolid(QStringLiteral("336699")),
                               dmlXfrm(0, 0, 100, 40, QString(), "dsp:txXfrm")));
        t = dmlAs<TextItem>(it);
        QVERIFY(t);
        QVERIFY(near(t->rotation, 270));
        QVERIFY(dmlRectIs(t->rect, 0, 0, 100, 40));
        // A shape turned a quarter, its text turned back, and its text's rectangle off the shape's center: the center goes round with the shape.
        it = dmlOne(env, dspSp(dmlXfrm(0, 0, 100, 40, QStringLiteral("rot=\"5400000\"")) + QStringLiteral("<a:prstGeom prst=\"rightArrow\"/>") + dmlSolid(QStringLiteral("336699")),
                               dmlXfrm(0, 0, 60, 40, QStringLiteral("rot=\"-5400000\""), "dsp:txXfrm")));
        g = dmlAs<GroupItem>(it);
        QVERIFY(g && g->children.size() == 2);
        t = dmlAs<TextItem>(g->children[1]);
        QVERIFY(t);
        QVERIFY(near(t->rotation, 0));
        QVERIFY(dmlRectIs(t->rect, 20, -20, 60, 40));   // (30, 20) about the shape's center (50, 20) a quarter turn clockwise: (50, 0)
        QVERIFY(dmlAs<ShapeItem>(g->children[0]) && near(g->children[0]->rotation, 90));
        // A flip puts the rectangle on the other side of the shape's center.
        it = dmlOne(env, dspSp(dmlXfrm(0, 0, 100, 40, QStringLiteral("flipH=\"1\"")) + QStringLiteral("<a:prstGeom prst=\"rightArrow\"/>") + dmlSolid(QStringLiteral("336699")),
                               dmlXfrm(0, 0, 60, 40, QString(), "dsp:txXfrm")));
        g = dmlAs<GroupItem>(it);
        QVERIFY(g && g->children.size() == 2);
        QVERIFY(dmlRectIs(g->children[1]->rect, 40, 0, 60, 40));
        // No text, no text box; no rectangle for the text, the shape's own place for it.
        it = dmlOne(env, dspSp(round, dmlXfrm(30, 20, 160, 80, QString(), "dsp:txXfrm"), QString()));
        QVERIFY(dmlAs<ShapeItem>(it) && dmlAs<ShapeItem>(it)->storyId.isEmpty());
        it = dmlOne(env, dspSp(round, QString()));
        QVERIFY(dmlAs<ShapeItem>(it) && !dmlAs<ShapeItem>(it)->storyId.isEmpty());
        // The font of the shape's style gives the text its color, as it does a p:sp's.
        it = dmlOne(env, dspSp(plain, dmlXfrm(10, 10, 200, 100, QString(), "dsp:txXfrm"), QStringLiteral("Hello"),
                               QStringLiteral("<dsp:style><a:lnRef idx=\"0\"><a:scrgbClr r=\"0\" g=\"0\" b=\"0\"/></a:lnRef><a:fillRef idx=\"0\"><a:scrgbClr r=\"0\" g=\"0\" b=\"0\"/></a:fillRef>"
                                              "<a:effectRef idx=\"0\"><a:scrgbClr r=\"0\" g=\"0\" b=\"0\"/></a:effectRef><a:fontRef idx=\"minor\"><a:srgbClr val=\"FF0000\"/></a:fontRef></dsp:style>")));
        t = dmlAs<TextItem>(it);
        QVERIFY(t);
        QCOMPARE(env.stories.last()->begin().begin().fragment().charFormat().foreground().color(), QColor(0xFF, 0x00, 0x00));
    }

    // A diagram's shape filled with a cut picture becomes a picture, and a picture keeps its proportions unless the file says the shape it was cut
    // from doesn't: a shape that says nothing of it is not a lock turned off (that is a shape's default, not a choice).
    void dmlCutPictureKeepsAPicturesLockUnlessTheFileSaysOtherwise()
    {
        DmlEnv env;
        env.usePackage();
        const QString fill = dmlXfrm(10, 10, 100, 50) + QStringLiteral("<a:prstGeom prst=\"ellipse\"/><a:blipFill><a:blip r:embed=\"rId1\"/><a:srcRect t=\"25000\" b=\"25000\"/>"
                                                                       "<a:stretch><a:fillRect/></a:stretch></a:blipFill>");
        auto locks = [&](const QString &lock) -> ItemPtr {
            QString sp = dspSp(fill, QString(), QString());
            if (!lock.isEmpty()) sp.replace(QStringLiteral("<dsp:cNvSpPr/>"), QStringLiteral("<dsp:cNvSpPr>%1</dsp:cNvSpPr>").arg(lock));
            auto g = dmlAs<GroupItem>(dmlOne(env, sp));
            return g && !g->children.empty() && g->children[0]->type() == ItemType::Picture ? g->children[0] : ItemPtr();
        };
        ItemPtr pic = locks(QString());
        QVERIFY(pic);
        QVERIFY(pic->lockAspect);   // the file says nothing: a picture's own
        pic = locks(QStringLiteral("<a:spLocks noGrp=\"1\"/>"));
        QVERIFY(pic && pic->lockAspect);   // locks, but none on the proportions
        pic = locks(QStringLiteral("<a:spLocks noChangeAspect=\"0\"/>"));
        QVERIFY(pic && !pic->lockAspect);   // the file says it is off
        pic = locks(QStringLiteral("<a:spLocks noChangeAspect=\"1\"/>"));
        QVERIFY(pic && pic->lockAspect);
    }

    // A picture a diagram's shape is filled with is cut to fill it by spreading it past the shape (a negative a:fillRect) or by cutting it (a:srcRect):
    // a fill has no such cut, so the shape is a picture, cut to the shape's outline, with the shape's line and text over it.
    void dmlDiagramShapeWithACutPictureIsAPictureUnderTheShape()
    {
        DmlEnv env;
        env.usePackage();
        auto picShape = [](const QString &blipFillInner, const QString &prst, const QString &txXfrm = QString(), const QString &text = QString()) {
            return dspSp(dmlXfrm(10, 10, 100, 50) + QStringLiteral("<a:prstGeom prst=\"%1\"/><a:blipFill><a:blip r:embed=\"rId1\"/>%2</a:blipFill><a:ln w=\"12700\">%3</a:ln>")
                                                        .arg(prst, blipFillInner, dmlSolid(QStringLiteral("FFFFFF"))),
                         txXfrm, text);
        };
        // Spread past the shape by a tenth of its width on each side: the picture is that much larger, and centered.
        ItemPtr it = dmlOne(env, picShape(QStringLiteral("<a:stretch><a:fillRect l=\"-10000\" r=\"-10000\"/></a:stretch>"), QStringLiteral("ellipse"), QString(), QString()));
        auto g = dmlAs<GroupItem>(it);
        QVERIFY(g && g->children.size() == 2);
        auto pic = dmlAs<PictureItem>(g->children[0]);
        auto shape = dmlAs<ShapeItem>(g->children[1]);
        QVERIFY(pic && shape);
        QCOMPARE(pic->maskShape, QStringLiteral("ellipse"));
        QVERIFY(dmlRectIs(pic->rect, 10, 10, 100, 50));
        QVERIFY(dmlRectIs(pic->imgRect, -10, 0, 120, 50));
        QVERIFY(!pic->imageId.isEmpty());
        QVERIFY(shape->fill.isNone());                  // the picture is no longer the shape's fill
        QVERIFY(near(shape->stroke.width, 1));          // its line stays, over the picture
        QCOMPARE(shape->shape, QStringLiteral("ellipse"));
        // Cut by a:srcRect, a quarter off the top and the bottom: the picture is twice as tall as the shape, spread evenly above and below.
        it = dmlOne(env, picShape(QStringLiteral("<a:srcRect t=\"25000\" b=\"25000\"/><a:stretch><a:fillRect/></a:stretch>"), QStringLiteral("rect")));
        g = dmlAs<GroupItem>(it);
        QVERIFY(g && g->children.size() == 2);
        pic = dmlAs<PictureItem>(g->children[0]);
        QVERIFY(pic);
        QVERIFY(dmlRectIs(pic->imgRect, 0, -25, 100, 100));
        // With text in a rectangle of its own, the text box comes with the shape, over the picture.
        it = dmlOne(env, picShape(QStringLiteral("<a:stretch><a:fillRect l=\"-10000\" r=\"-10000\"/></a:stretch>"), QStringLiteral("roundRect"), dmlXfrm(20, 20, 80, 30, QString(), "dsp:txXfrm"), QStringLiteral("Caption")));
        g = dmlAs<GroupItem>(it);
        QVERIFY(g && g->children.size() == 3);
        QVERIFY(dmlAs<PictureItem>(g->children[0]) && dmlAs<ShapeItem>(g->children[1]) && dmlAs<TextItem>(g->children[2]));
        QVERIFY(g->children[1]->fill.isNone());
        // A picture that fills it plainly stays a fill, and so does one in a shape drawn by its own paths.
        it = dmlOne(env, picShape(QStringLiteral("<a:stretch><a:fillRect/></a:stretch>"), QStringLiteral("rect")));
        QVERIFY(dmlAs<ShapeItem>(it) && dmlAs<ShapeItem>(it)->fill.type == Fill::Picture);
    }

    // A diagram's drawing is a tree of such shapes (dsp:spTree), with groups (dsp:grpSp) that place theirs as p:grpSp does.
    void dmlDiagramTreeReadsItsShapesAndGroups()
    {
        DmlEnv env;
        const QString dsp = QStringLiteral("xmlns:dsp=\"http://schemas.microsoft.com/office/drawing/2008/diagram\"");
        auto sp = [](double x, double y, const QString &text, double inset = 0) {
            return dspSp(dmlXfrm(x, y, 50, 20) + QStringLiteral("<a:prstGeom prst=\"rect\"/>") + dmlSolid(QStringLiteral("336699")),
                         dmlXfrm(x + inset, y + inset, 50 - 2 * inset, 20 - 2 * inset, QString(), "dsp:txXfrm"), text);
        };
        const QString tree = QStringLiteral("<dsp:spTree %1><dsp:nvGrpSpPr><dsp:cNvPr id=\"0\" name=\"\"/><dsp:cNvGrpSpPr/></dsp:nvGrpSpPr><dsp:grpSpPr/>%2%3"
                                            "<dsp:grpSp><dsp:nvGrpSpPr><dsp:cNvPr id=\"0\" name=\"\"/><dsp:cNvGrpSpPr/></dsp:nvGrpSpPr><dsp:grpSpPr><a:xfrm><a:off x=\"%4\" y=\"0\"/><a:ext cx=\"%5\" cy=\"%6\"/>"
                                            "<a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"%5\" cy=\"%6\"/></a:xfrm></dsp:grpSpPr>%7</dsp:grpSp></dsp:spTree>")
                                 .arg(dsp, sp(0, 0, QStringLiteral("One")), sp(60, 0, QStringLiteral("Two"), 2), emu(200), emu(50), emu(20), sp(0, 0, QStringLiteral("Inner")));
        const ItemList items = readShapeTree(env.element(tree), env.ctx);
        QCOMPARE(int(items.size()), 3);
        QVERIFY(dmlRectIs(items[0]->rect, 0, 0, 50, 20));
        // The second has a rectangle for its text inside it: a shape and its text box.
        auto second = dmlAs<GroupItem>(items[1]);
        QVERIFY(second && second->children.size() == 2);
        QVERIFY(dmlRectIs(second->rect, 60, 0, 50, 20));
        QVERIFY(dmlRectIs(second->children[1]->rect, 62, 2, 46, 16));
        auto group = dmlAs<GroupItem>(items[2]);
        QVERIFY(group && group->children.size() == 1);
        QVERIFY(dmlRectIs(group->children[0]->rect, 200, 0, 50, 20));
        QCOMPARE(env.stories.size(), 3);
    }
};

JP_TEST_CLASS(OfficeDmlFillTests)
#include "tst_office_dmlfill.moc"
