// Tests of libs/office: packages, the theme, and DrawingML colors. Every package they read is built in memory
// (never a real-world file). This is what the test classes (tst_office_*.cpp) share: the packages and the readers of their parts.
#pragma once

#include "office/chart.h"
#include "office/drawingml.h"
#include "office/drawingml_props.h"
#include "office/drawingml_color.h"
#include "office/drawingml_geometry.h"
#include "office/drawingml_style.h"
#include "office/drawingml_table.h"
#include "office/drawingml_write.h"
#include "office/eot.h"
#include "office/odraw.h"
#include "office/package.h"
#include "office/theme.h"
#include "office/xml.h"
#include "core/drawnwords.h"
#include "core/fontfiles.h"
#include "core/fonts.h"
#include "core/sfnt.h"
#include "io/zip.h"
#include "render/metafile.h"
#include "render/shapes.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include <QBuffer>
#include <QElapsedTimer>
#include <QFile>
#include <QFontDatabase>
#include <QImage>
#include <QPainter>
#include <QTextDocument>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QStringConverter>
#include <QSvgRenderer>
#include <QTemporaryDir>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QtEndian>
#include <QtTest>

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <memory>
#include <vector>

#include "../testrunner.h"

using namespace jp::office;
using jp::Arrow;
using jp::ColorRef;
using jp::Fill;
using jp::FontFileInfo;
using jp::GradientStop;
using jp::GroupItem;
using jp::ItemList;
using jp::ItemPtr;
using jp::ItemType;
using jp::LineItem;
using jp::PictureItem;
using jp::ShapeItem;
using jp::Stroke;
using jp::TableItem;
using jp::TextArtItem;
using jp::TextItem;

namespace officehelp {

extern const QString kRelBase;
extern const QString kPresentationType;
extern const QString kSlideType;

QString rel(const QString &id, const QString &type, const QString &target, bool external = false);


// A theme part written by hand, with the twelve colors (the first two as
// system colors, whose lastClr is the value) and a font scheme with a
// per-script font the struct has no place for.
QByteArray handWrittenTheme(const QString &a = QString(ns::A));

extern const QColor kHandWrittenColors[Theme::ColorCount];


// ---- colors

Theme colorTestTheme();

QString hexOf(const QColor &c);

// <a:name val="value"/> for each "name value" given.
QString transforms(const QStringList &list);

QString schemeClr(const QString &val, const QStringList &list = {});

// The color in `<a:solidFill>` holding `inner`.
QColor colorOf(const QString &inner, const QColor &phClr = QColor(), const ColorMap &map = ColorMap(), const Theme &theme = colorTestTheme(),
               const QString &a = QString(ns::A));
// ---- DrawingML shapes: what the tests below build their drawings from

using Arrow = jp::Arrow;
using ColorRef = jp::ColorRef;
using Fill = jp::Fill;
using GroupItem = jp::GroupItem;
using Item = jp::Item;
using ItemList = jp::ItemList;
using ItemPtr = jp::ItemPtr;
using ItemType = jp::ItemType;
using LineItem = jp::LineItem;
using PictureItem = jp::PictureItem;
using ShapeItem = jp::ShapeItem;
using Stroke = jp::Stroke;
using TextItem = jp::TextItem;
using VAlign = jp::VAlign;
using Wrap = jp::Wrap;

QString dmlNamespaces();

QString emu(double points);

bool near(double a, double b, double tolerance = 1e-6);

// A small format scheme, as a theme holds: each list has the three styles the references 1 to 3 (and 1001 to 1003) pick.
QString dmlFormats();

// A 4 by 4 picture of one color, as PNG bytes.
QByteArray dmlPicture(const QColor &color = Qt::red);

// What reading a drawing is given, with a record of what it asks for: the pictures it keeps and the stories it starts.
struct DmlEnv {
    struct Image { QByteArray bytes; QString format; };
    DrawingContext ctx;
    Package package;
    QVector<Image> images;
    QVector<std::shared_ptr<QTextDocument>> stories;
    QVector<std::shared_ptr<QDomDocument>> documents;

    DmlEnv()
    {
        ctx.theme = colorTestTheme();
        ctx.addImage = [this](const QByteArray &bytes, const QString &format) {
            images.append({bytes, format});
            return QStringLiteral("img%1").arg(images.size());
        };
        ctx.newStory = [this]() {
            stories.append(std::make_shared<QTextDocument>());
            return QStringLiteral("story%1").arg(stories.size());
        };
        ctx.storyDoc = [this](const QString &id) -> QTextDocument * {
            const int n = id.mid(5).toInt();
            return n >= 1 && n <= stories.size() ? stories[n - 1].get() : nullptr;
        };
    }
    DmlEnv(const DmlEnv &) = delete;
    DmlEnv &operator=(const DmlEnv &) = delete;

    // The first element in `xml`, with a, p, and r declared; it stays valid as long as this does.
    QDomElement element(const QString &xml)
    {
        auto doc = std::make_shared<QDomDocument>();
        documents.append(doc);
        const QByteArray text = QStringLiteral("<root %1>%2</root>").arg(dmlNamespaces(), xml).toUtf8();
        if (!parseXml(text, doc.get())) return QDomElement();
        return doc->documentElement().firstChildElement();
    }
    void useFormats() { ctx.formats = element(dmlFormats()); }
    // A package whose slide part has a picture (rId1) and an outside link (rId2, to `link`).
    void usePackage(const QByteArray &picture = dmlPicture(), const QString &link = QStringLiteral("https://example.com/x"))
    {
        PackageWriter w;
        w.addDefault(QStringLiteral("png"), QStringLiteral("image/png"));
        w.addPart(QStringLiteral("/ppt/slides/slide1.xml"), "<p:sld/>", kSlideType);
        w.addPart(QStringLiteral("/ppt/media/image1.png"), picture, QStringLiteral("image/png"));
        w.addRelationship(QStringLiteral("/ppt/slides/slide1.xml"), kRelBase + "image", QStringLiteral("/ppt/media/image1.png"));
        w.addRelationship(QStringLiteral("/ppt/slides/slide1.xml"), kRelBase + "hyperlink", link, true);
        package.read(w.finish());
        ctx.package = &package;
        ctx.part = QStringLiteral("/ppt/slides/slide1.xml");
    }
};
QStringList dmlAllPresets();

// <p:style> with the references given.
QString dmlStyle(const QString &lnIdx, const QString &fillIdx, const QString &effectIdx = QStringLiteral("0"), const QString &color = QStringLiteral("accent1"));

QString dmlHex(const ColorRef &c);
// Items written as the shape tree of a slide part, then the package read
// and the tree read back, with a record of what was written.
struct WriteRound {
    PackageWriter writer;
    QHash<QString, QString> media;
    QMap<QString, QByteArray> pictures;                      // by image id
    QMap<QString, std::shared_ptr<QTextDocument>> stories;   // by story id
    DrawingWriteContext ctx;
    QStringList skipped;
    DmlEnv env;
    QByteArray tree;       // the shape tree as written
    QDomDocument dom;
    QDomElement spTree;
    ItemList items;        // what reading it gave
    QString error;

    WriteRound()
    {
        ctx.package = &writer;
        ctx.part = QStringLiteral("/ppt/slides/slide1.xml");
        ctx.theme = colorTestTheme();
        ctx.colors = jp::builtinColorSchemes().value(0);
        ctx.mediaParts = &media;
        ctx.imageBytes = [this](const QString &id, QString *format) {
            if (format) *format = QStringLiteral("png");
            return pictures.value(id);
        };
        ctx.storyDoc = [this](const QString &id) -> const QTextDocument * { return stories.value(id).get(); };
    }
    QString addPicture(const QColor &color = Qt::red)
    {
        const QString id = QStringLiteral("pic%1").arg(pictures.size() + 1);
        pictures.insert(id, dmlPicture(color));
        return id;
    }
    QString addStory(const QString &text = QString())
    {
        const QString id = QStringLiteral("st%1").arg(stories.size() + 1);
        auto doc = std::make_shared<QTextDocument>();
        doc->setPlainText(text);
        stories.insert(id, doc);
        return id;
    }
    QTextDocument *story(const QString &id) const { return stories.value(id).get(); }
    // The story an item read back holds.
    QTextDocument *storyOf(const ItemPtr &it) const
    {
        const QString id = it->type() == ItemType::Text ? static_cast<TextItem *>(it.get())->storyId : static_cast<ShapeItem *>(it.get())->storyId;
        const int n = id.mid(5).toInt();
        return n >= 1 && n <= env.stories.size() ? env.stories[n - 1].get() : nullptr;
    }
    bool runWith(const std::function<void(QXmlStreamWriter &)> &write)
    {
        QByteArray body;
        QXmlStreamWriter w(&body);
        w.writeStartElement(QStringLiteral("p:spTree"));
        write(w);
        w.writeEndElement();
        tree = body;
        const QByteArray part = "<p:sld " + dmlNamespaces().toUtf8() + "><p:cSld>" + body + "</p:cSld></p:sld>";
        writer.addPart(ctx.part, part, kSlideType);
        if (!env.package.read(writer.finish(), &error) || !parseXml(part, &dom, &error)) return false;
        env.ctx.package = &env.package;
        env.ctx.part = ctx.part;
        spTree = child(child(dom.documentElement(), QString(ns::P), QStringLiteral("cSld")), QString(ns::P), QStringLiteral("spTree"));
        return true;
    }
    bool run(const ItemList &list)
    {
        if (!runWith([&](QXmlStreamWriter &w) {
                for (const ItemPtr &it : list) writeShape(w, *it, ctx, &skipped);
            }))
            return false;
        items = readShapeTree(spTree, env.ctx);
        return true;
    }
    QString text() const { return QString::fromUtf8(tree); }
};
// Every element called `local` below `e`, in document order.
QVector<QDomElement> elementsNamed(const QDomElement &e, const char *local);
// A cell: its text (none if empty), attributes, the children and attributes of its a:tcPr, and its run's attributes.
QString dmlTc(const QString &text, const QString &attrs = QString(), const QString &pr = QString(), const QString &prAttrs = QString(),
                     const QString &rPr = QString());

// A graphic frame holding a table, at the place given in points.
QString dmlTableFrame(const QString &tblPr, const QString &grid, const QString &rows, double x, double y, double w, double h);
QString dmlXfrm(double x, double y, double w, double h, const QString &attrs = QString(), const char *tag = "a:xfrm");

// A p:sp with the properties, text body, and style given.
QString dmlSp(const QString &spPr, const QString &body = QString(), const QString &style = QString(), const QString &cNvSpPr = QString(),
                     const QString &nvPr = QString(), const QString &cNvPrInner = QString(), const QString &id = QStringLiteral("2"));

QString dmlBody(const QString &text = QStringLiteral("Hello"), const QString &bodyPr = QStringLiteral("<a:bodyPr/>"));

QString dmlPic(const QString &blipInner, const QString &spPr, const QString &blipFillInner = QStringLiteral("<a:stretch><a:fillRect/></a:stretch>"),
                      const QString &cNvPrInner = QString(), const QString &embed = QStringLiteral("rId1"));

QString dmlGrp(const QString &grpSpPr, const QString &children, const QString &id = QStringLiteral("20"));

// The group frame (off, ext) with the child space (chOff, chExt), all in points.
QString dmlGroupFrame(QRectF frame, QRectF child, const QString &attrs = QString());

QString dmlSolid(const QString &hex);

ItemList dmlTree(DmlEnv &env, const QString &shapes, const TextStyleStack &styles = {});

ItemPtr dmlOne(DmlEnv &env, const QString &shape, const QVector<QString> &inherits = {});

template <typename T> static std::shared_ptr<T> dmlAs(const ItemPtr &it) { return std::dynamic_pointer_cast<T>(it); }

bool dmlRectIs(const QRectF &r, double x, double y, double w, double h, double tolerance = 1e-6);

QString dmlRectText(const QRectF &r);

// <a:p> with one run of `text` whose a:rPr has the given attributes and children.
QString run(const QString &text, const QString &attrs = QString(), const QString &children = QString());
QString solid(const QString &hex);

} // namespace officehelp

using namespace officehelp;
