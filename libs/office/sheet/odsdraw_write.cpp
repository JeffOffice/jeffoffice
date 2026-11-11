// The drawing of a sheet (a part of the XML form, read by the shared drawing code into its own objects: core/items.h) as the elements of an OpenDocument spreadsheet's `table:shapes` (see
// odsdraw.h): a picture is a frame with an image, a shape a custom shape, a text box a frame with a text box, Text Art a fontwork custom shape, a line a line, a group a group.

#include "sheet/odsdraw.h"
#include "sheet/odsdraw_p.h"

#include "core/items.h"
#include "office/drawingml.h"
#include "office/drawingml_write.h"
#include "office/package.h"
#include "office/theme.h"
#include "office/xml.h"

#include <QCryptographicHash>
#include <QCoreApplication>
#include <QDomDocument>
#include <QTextBlock>
#include <QTextDocument>

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

namespace jp::office::sheet::odsdraw {

namespace {

constexpr double kEmuPerPoint = 12700.0;
constexpr int kBudgetStories = 30000, kBudgetCells = 50000, kBudgetParagraphs = 300000, kBudgetRuns = 500000;
constexpr qint64 kBudgetLetters = 20000000;

QString decimal(double v) { return QString::number(v, 'g', 10); }

class Writer {
public:
    Writer(const Package &package, const QString &mainPart, const QString &drawingPart, const Grid &grid, const QString &sheetRef, int sheetNumber, Written *out, qint64 *pictureBytes, qint64 *objects)
        : m_package(package), m_drawingPart(drawingPart), m_grid(grid), m_sheetRef(sheetRef), m_number(sheetNumber), m_out(out), m_pictureBytes(pictureBytes), m_objects(objects)
    {
        m_scheme = builtinColorSchemes().value(0);
        m_theme = defaultTheme();
        for (const Relationship &rel : package.relationshipsOfType(mainPart, QStringLiteral("/theme"))) {
            if (rel.external || !package.hasPart(rel.target)) continue;
            const QByteArray xml = package.part(rel.target);
            Theme t;
            if (!readTheme(xml, &t)) break;
            m_theme = t;
            if (parseXml(xml, &m_themeDoc)) {
                for (QDomElement e = m_themeDoc.documentElement().firstChildElement(); !e.isNull(); e = e.nextSiblingElement())
                    if (e.localName() == QLatin1String("themeElements"))
                        for (QDomElement f = e.firstChildElement(); !f.isNull(); f = f.nextSiblingElement())
                            if (f.localName() == QLatin1String("fmtScheme")) m_formats = f;
            }
            break;
        }
        using B = DrawingBudget;
        m_budget.left[B::Objects] = kMaxWorkbookObjects;
        m_budget.left[B::Stories] = kBudgetStories;
        m_budget.left[B::Cells] = kBudgetCells;
        m_budget.left[B::Paragraphs] = kBudgetParagraphs;
        m_budget.left[B::Runs] = kBudgetRuns;
        m_budget.left[B::Letters] = kBudgetLetters;
    }

    void run()
    {
        SheetDrawing drawing;
        QString why;
        if (!readDrawing(m_package, m_drawingPart, &drawing, &why)) {
            note(QCoreApplication::translate("Sheet", "drawing objects the file does not let be read"));
            return;
        }
        DrawingContext ctx;
        ctx.package = &m_package;
        ctx.part = m_drawingPart;
        ctx.theme = m_theme;
        ctx.formats = m_formats;
        ctx.budget = &m_budget;
        ctx.images = &m_imageOfPart;   // (a picture that several objects use is handed over once)
        ctx.addImage = [this](const QByteArray &bytes, const QString &format) {
            const QString id = QStringLiteral("i%1").arg(m_images.size() + 1);
            m_images[id] = bytes;
            m_imageFormats[id] = format;
            return id;
        };
        ctx.newStory = [this] {
            const QString id = QStringLiteral("s%1").arg(m_stories.size() + 1);
            m_stories[id] = std::make_unique<QTextDocument>();
            return id;
        };
        ctx.storyDoc = [this](const QString &id) -> QTextDocument * {
            const auto it = m_stories.find(id);
            return it == m_stories.end() ? nullptr : it->second.get();
        };
        int z = 0;
        for (const DrawingObject &o : std::as_const(drawing.objects)) {
            if (o.kind == ObjectKind::Chart) {
                note(QCoreApplication::translate("Sheet", "charts"));
                continue;
            }
            if (o.kind == ObjectKind::Diagram || o.kind == ObjectKind::Other) {
                note(QCoreApplication::translate("Sheet", "drawing objects of a kind OpenDocument has no element for"));
                continue;
            }
            if (o.hidden) {
                note(QCoreApplication::translate("Sheet", "hidden drawing objects"));
                continue;
            }
            if (*m_objects >= kMaxWorkbookObjects) {
                note(QCoreApplication::translate("Sheet", "drawing objects past the %1 a workbook may have").arg(kMaxWorkbookObjects));
                break;
            }
            const ItemPtr item = readShape(o.element, ctx);
            if (!item) {
                note(QCoreApplication::translate("Sheet", "drawing objects the drawing code does not read"));
                continue;
            }
            qint64 x = 0, y = 0, cx = 0, cy = 0;
            m_grid.boxOf(o.anchor, &x, &y, &cx, &cy);
            const QString xml = emit(*item, double(x), double(y), double(std::max<qint64>(cx, 1)), double(std::max<qint64>(cy, 1)), 0, &o.anchor, z);
            if (xml.isEmpty()) continue;
            ++z;
            quint32 bottom = 0;
            qint64 inRow = 0;
            m_grid.rowAt(qint64(y) + cy, &bottom, &inRow);
            m_out->lastRow = std::max(m_out->lastRow, bottom);
            ++*m_objects;
            ++m_out->count;
            m_out->shapes += xml;
        }
        if (!m_dashes.isEmpty()) {
            for (const QString &d : std::as_const(m_dashes)) m_out->commonStyles += d;
        }
    }

private:
    const Package &m_package;
    QString m_drawingPart;
    const Grid &m_grid;
    QString m_sheetRef;
    int m_number;
    Written *m_out;
    qint64 *m_pictureBytes;
    qint64 *m_objects;
    ColorScheme m_scheme;
    Theme m_theme;
    QDomDocument m_themeDoc;
    QDomElement m_formats;
    DrawingBudget m_budget;
    QHash<QString, QByteArray> m_images;
    QHash<QString, QString> m_imageFormats;
    QHash<QString, QString> m_imageOfPart;    // the id kept for each picture of the package, by part and relationship
    QHash<QString, QString> m_fileOfImage;    // the file a picture was put in, by its id: hashed once
    QHash<QString, QSizeF> m_naturalOfFile;   // and measured once
    std::map<QString, std::unique_ptr<QTextDocument>> m_stories;
    QHash<QString, QString> m_styleOf;    // the XML of a style's properties, by the name it was given
    QHash<QString, QString> m_textStyleOf;
    QSet<QString> m_pictureNames;
    std::set<QString> m_notes;
    QStringList m_dashes;
    QSet<QString> m_dashNames;
    int m_styles = 0;

    void note(const QString &what)
    {
        if (m_notes.insert(what).second) m_out->notWritten << what;
    }

    QString name(const char *kind) { return QStringLiteral("dr%1%2%3").arg(m_number).arg(QLatin1String(kind)).arg(++m_styles); }

    // ---- styles
    QString color(const ColorRef &c) const
    {
        const QColor q = c.kind() == ColorRef::Rgb ? c.rgbValue() : c.resolve(m_scheme);
        return QStringLiteral("#%1").arg(q.rgb() & 0xFFFFFF, 6, 16, QLatin1Char('0'));
    }

    QString lookProps(const Item &item, bool noFillAtAll)
    {
        QString p;
        const Fill &f = item.fill;
        if (noFillAtAll || f.isNone()) {
            p += QStringLiteral(" draw:fill=\"none\"");
        } else if (f.type == Fill::Solid || f.type == Fill::Gradient) {
            const ColorRef c = f.type == Fill::Gradient && !f.stops.isEmpty() ? f.stops.first().color : f.color;
            p += QStringLiteral(" draw:fill=\"solid\" draw:fill-color=\"%1\"").arg(color(c));
            if (f.transparency > 0.001) p += QStringLiteral(" draw:opacity=\"%1%\"").arg(decimal(std::clamp((1.0 - f.transparency) * 100.0, 0.0, 100.0)));
            if (f.type == Fill::Gradient) note(QCoreApplication::translate("Sheet", "gradient fills of drawing objects (they are written as the first color)"));
        } else {
            p += QStringLiteral(" draw:fill=\"none\"");
            note(QCoreApplication::translate("Sheet", "picture and pattern fills of drawing objects"));
        }
        const Stroke &s = item.stroke;
        if (s.isNone()) {
            p += QStringLiteral(" draw:stroke=\"none\"");
        } else {
            const bool dashed = s.dash != Stroke::SolidLine;
            p += QStringLiteral(" draw:stroke=\"%1\"").arg(dashed ? QStringLiteral("dash") : QStringLiteral("solid"));
            if (dashed) p += QStringLiteral(" draw:stroke-dash=\"%1\"").arg(dash(s.dash));
            p += QStringLiteral(" svg:stroke-width=\"%1\" svg:stroke-color=\"%2\"").arg(lengthText(qint64(std::llround(s.width * kEmuPerPoint))), color(s.color));
        }
        if (item.fx.any()) note(QCoreApplication::translate("Sheet", "shadows and other effects of drawing objects"));
        return p;
    }

    QString dash(Stroke::Dash d)
    {
        QString key, def;
        switch (d) {
        case Stroke::RoundDot:
        case Stroke::SquareDot:
            key = QStringLiteral("Dotted");
            def = QStringLiteral("draw:style=\"rect\" draw:dots1=\"1\" draw:dots1-length=\"100%\" draw:distance=\"100%\"");
            break;
        case Stroke::DashDot:
        case Stroke::LongDashDot:
        case Stroke::LongDashDotDot:
            key = QStringLiteral("Dash_20_Dot");
            def = QStringLiteral("draw:style=\"rect\" draw:dots1=\"1\" draw:dots1-length=\"300%\" draw:dots2=\"1\" draw:dots2-length=\"100%\" draw:distance=\"100%\"");
            break;
        default:
            key = QStringLiteral("Dashed");
            def = QStringLiteral("draw:style=\"rect\" draw:dots1=\"1\" draw:dots1-length=\"300%\" draw:distance=\"200%\"");
            break;
        }
        if (!m_dashNames.contains(key)) {
            m_dashNames.insert(key);
            m_dashes << QStringLiteral("<draw:stroke-dash draw:name=\"%1\" draw:display-name=\"%1\" %2/>").arg(key, def);
        }
        return key;
    }

    QString graphicStyle(const QString &props)
    {
        const auto it = m_styleOf.constFind(props);
        if (it != m_styleOf.cend()) return it.value();
        const QString n = name("g");
        m_styleOf.insert(props, n);
        m_out->styles += QStringLiteral("<style:style style:name=\"%1\" style:family=\"graphic\"><style:graphic-properties%2/></style:style>").arg(n, props);
        return n;
    }

    QString textStyle(const QTextCharFormat &f)
    {
        QString p;
        if (f.hasProperty(QTextFormat::FontPointSize) && f.fontPointSize() > 0) p += QStringLiteral(" fo:font-size=\"%1pt\"").arg(decimal(f.fontPointSize()));
        if (f.hasProperty(QTextFormat::FontWeight)) p += QStringLiteral(" fo:font-weight=\"%1\"").arg(f.fontWeight() >= QFont::Bold ? QStringLiteral("bold") : QStringLiteral("normal"));
        if (f.hasProperty(QTextFormat::FontItalic)) p += QStringLiteral(" fo:font-style=\"%1\"").arg(f.fontItalic() ? QStringLiteral("italic") : QStringLiteral("normal"));
        if (f.hasProperty(QTextFormat::TextUnderlineStyle) && f.fontUnderline()) p += QStringLiteral(" style:text-underline-style=\"solid\" style:text-underline-width=\"auto\" style:text-underline-color=\"font-color\"");
        if (f.hasProperty(QTextFormat::FontStrikeOut) && f.fontStrikeOut()) p += QStringLiteral(" style:text-line-through-style=\"solid\"");
        if (f.foreground().style() != Qt::NoBrush) p += QStringLiteral(" fo:color=\"#%1\"").arg(f.foreground().color().rgb() & 0xFFFFFF, 6, 16, QLatin1Char('0'));
        const QStringList families = f.fontFamilies().toStringList();
        if (!families.isEmpty() && !families.first().isEmpty()) p += QStringLiteral(" fo:font-family=\"%1\"").arg(xmlText(families.first()));
        if (p.isEmpty()) return QString();
        const auto it = m_textStyleOf.constFind(p);
        if (it != m_textStyleOf.cend()) return it.value();
        const QString n = name("t");
        m_textStyleOf.insert(p, n);
        m_out->styles += QStringLiteral("<style:style style:name=\"%1\" style:family=\"text\"><style:text-properties%2/></style:style>").arg(n, p);
        return n;
    }

    QString paragraphStyle(Qt::Alignment a)
    {
        const QString align = a & Qt::AlignHCenter ? QStringLiteral("center") : a & Qt::AlignRight ? QStringLiteral("end") : a & Qt::AlignJustify ? QStringLiteral("justify") : QStringLiteral("start");
        const QString key = QStringLiteral("P ") + align;
        const auto it = m_textStyleOf.constFind(key);
        if (it != m_textStyleOf.cend()) return it.value();
        const QString n = name("p");
        m_textStyleOf.insert(key, n);
        m_out->styles += QStringLiteral("<style:style style:name=\"%1\" style:family=\"paragraph\"><style:paragraph-properties fo:text-align=\"%2\"/></style:style>").arg(n, align);
        return n;
    }

    // ---- text
    static QString textXml(const QString &text)
    {
        // (runs of plain characters are escaped whole, so that a letter written as two UTF-16 units is never cut in the middle)
        QString out, run;
        int spaces = 0;
        auto flushRun = [&] {
            if (!run.isEmpty()) out += xmlText(run);
            run.clear();
        };
        auto flushSpaces = [&] {
            if (spaces > 1) out += QStringLiteral("<text:s text:c=\"%1\"/>").arg(spaces - 1);
            spaces = 0;
        };
        for (const QChar c : text) {
            if (c == QLatin1Char(' ')) {
                if (spaces++ == 0) {
                    flushRun();
                    out += QLatin1Char(' ');
                }
                continue;
            }
            flushSpaces();
            if (c == QLatin1Char('\t')) {
                flushRun();
                out += QStringLiteral("<text:tab/>");
            } else if (c == QLatin1Char('\n') || c == QChar(QChar::LineSeparator)) {
                flushRun();
                out += QStringLiteral("<text:line-break/>");
            } else {
                run += c;
            }
        }
        flushRun();
        flushSpaces();
        return out;
    }

    QString paragraphs(const QTextDocument *doc)
    {
        if (!doc) return QString();
        QString out;
        int paragraphs = 0, chars = 0;
        for (QTextBlock b = doc->begin(); b.isValid() && paragraphs < kMaxParagraphs; b = b.next(), ++paragraphs) {
            QString inner;
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const QTextFragment fr = it.fragment();
                if (!fr.isValid()) continue;
                QString t = fr.text();
                if (chars + t.size() > kMaxTextChars) t.truncate(std::max(0, kMaxTextChars - chars));
                chars += t.size();
                if (t.isEmpty()) continue;
                const QString style = textStyle(fr.charFormat());
                inner += style.isEmpty() ? textXml(t) : QStringLiteral("<text:span text:style-name=\"%1\">%2</text:span>").arg(style, textXml(t));
            }
            out += QStringLiteral("<text:p text:style-name=\"%1\">%2</text:p>").arg(paragraphStyle(b.blockFormat().alignment()), inner);
            if (chars >= kMaxTextChars) break;
        }
        return out;
    }

    // ---- placing
    QString geometry(double x, double y, double w, double h, double degrees) const
    {
        QString g = QStringLiteral(" svg:width=\"%1\" svg:height=\"%2\"").arg(lengthText(qint64(std::llround(w))), lengthText(qint64(std::llround(h))));
        if (std::abs(degrees) < 0.0005) return g + QStringLiteral(" svg:x=\"%1\" svg:y=\"%2\"").arg(lengthText(qint64(std::llround(x))), lengthText(qint64(std::llround(y))));
        // turned: the standard turns against the clock, about the box's top left, and then moves it where its top left goes
        const double phi = degrees * M_PI / 180.0, c = std::cos(phi), s = std::sin(phi);
        const double tx = x + w / 2 + c * (-w / 2) - s * (-h / 2), ty = y + h / 2 + s * (-w / 2) + c * (-h / 2);
        return g + QStringLiteral(" draw:transform=\"rotate (%1) translate (%2 %3)\"").arg(decimal(-phi), lengthText(qint64(std::llround(tx))), lengthText(qint64(std::llround(ty))));
    }

    // Where the object is anchored, said as the standard says it: the cell its far corner is in for one that moves and sizes with cells, and that it is anchored to a cell for one that moves.
    QString anchoring(const DrawingAnchor *a) const
    {
        if (!a) return QString();
        if (a->kind == AnchorKind::TwoCell)
            return QStringLiteral(" draw:anchor-type=\"cell\" table:end-cell-address=\"%1.%2\" table:end-x=\"%3\" table:end-y=\"%4\"")
                .arg(m_sheetRef, cellName(a->to.row, a->to.col), lengthText(a->to.colOff), lengthText(a->to.rowOff));
        if (a->kind == AnchorKind::OneCell) return QStringLiteral(" draw:anchor-type=\"cell\"");
        return QString();
    }

    static QString titles(const Item &item)
    {
        QString t;
        if (!item.altTitle.isEmpty()) t += QStringLiteral("<svg:title>%1</svg:title>").arg(xmlText(item.altTitle));
        if (!item.altText.isEmpty()) t += QStringLiteral("<svg:desc>%1</svg:desc>").arg(xmlText(item.altText));
        return t;
    }

    QString pictureFile(const QString &imageId, const QByteArray &bytes, const QString &format)
    {
        const auto known = m_fileOfImage.constFind(imageId);
        if (known != m_fileOfImage.constEnd()) return known.value();
        const QString file = pictureFileOf(bytes, format);
        m_fileOfImage.insert(imageId, file);
        return file;
    }

    QString pictureFileOf(const QByteArray &bytes, const QString &format)
    {
        static const QHash<QString, QString> ext = {{"png", "png"}, {"jpeg", "jpg"}, {"gif", "gif"}, {"bmp", "bmp"}, {"tiff", "tif"}, {"svg", "svg"}, {"emf", "emf"}, {"wmf", "wmf"}};
        ++testHooks().pictureHashes;
        const QString file = QStringLiteral("Pictures/") + QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha1).toHex().left(24)) + QLatin1Char('.') + ext.value(format, QStringLiteral("bin"));
        if (!m_pictureNames.contains(file)) {
            if (*m_pictureBytes + bytes.size() > kMaxPictureTotal) return QString();
            *m_pictureBytes += bytes.size();
            m_pictureNames.insert(file);
            m_out->pictures.push_back({file, bytes});
        }
        return file;
    }

    // ---- the kinds
    QString emit(const Item &item, double x, double y, double w, double h, int depth, const DrawingAnchor *anchor, int z)
    {
        const QString order = anchor ? QStringLiteral(" draw:z-index=\"%1\"").arg(z) : QString();
        const QString common = QStringLiteral(" draw:name=\"%1\"").arg(xmlText(item.name)) + order;
        switch (item.type()) {
        case ItemType::Picture: {
            const auto &p = static_cast<const PictureItem &>(item);
            const QByteArray bytes = m_images.value(p.imageId);
            const QString format = m_imageFormats.value(p.imageId);
            if (bytes.isEmpty()) {
                note(QCoreApplication::translate("Sheet", "pictures the file does not hold"));
                return QString();
            }
            if (bytes.size() > kMaxPictureBytes) {
                note(QCoreApplication::translate("Sheet", "pictures larger than %1 MB").arg(kMaxPictureBytes >> 20));
                return QString();
            }
            const QString file = pictureFile(p.imageId, bytes, format);
            if (file.isEmpty()) {
                note(QCoreApplication::translate("Sheet", "pictures past the %1 MB a workbook may keep").arg(kMaxPictureTotal >> 20));
                return QString();
            }
            QString props = lookProps(p, true);
            // the part that is cut off, in lengths of the picture's own size
            const QRectF whole = p.imgRect, frame(0, 0, p.rect.width(), p.rect.height());
            if (whole.width() > 0 && whole.height() > 0 && whole != frame) {
                if (!m_naturalOfFile.contains(file)) m_naturalOfFile.insert(file, naturalSizePoints(bytes));
                const QSizeF natural = m_naturalOfFile.value(file);
                if (natural.width() > 0) {
                    const double l = (frame.left() - whole.left()) / whole.width(), t = (frame.top() - whole.top()) / whole.height();
                    const double r = (whole.right() - frame.right()) / whole.width(), b = (whole.bottom() - frame.bottom()) / whole.height();
                    if (l < 0 || t < 0 || r < 0 || b < 0) note(QCoreApplication::translate("Sheet", "pictures with room around them instead of a crop (they are written whole)"));
                    else if (l || t || r || b) {
                        auto len = [&](double v, double of) { return lengthText(qint64(std::llround(v * of * kEmuPerPoint))); };
                        props += QStringLiteral(" fo:clip=\"rect(%1, %2, %3, %4)\"").arg(len(t, natural.height()), len(r, natural.width()), len(b, natural.height()), len(l, natural.width()));
                    }
                } else {
                    note(QCoreApplication::translate("Sheet", "the crop of pictures in a form whose size is not known"));
                }
            }
            if (p.flipH || p.flipV) props += QStringLiteral(" style:mirror=\"%1\"").arg(p.flipH && p.flipV ? QStringLiteral("horizontal vertical") : p.flipH ? QStringLiteral("horizontal") : QStringLiteral("vertical"));
            return QStringLiteral("<draw:frame draw:style-name=\"%1\"%2%3%4><draw:image xlink:href=\"%5\" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\"/>%6</draw:frame>")
                .arg(graphicStyle(props), common, geometry(x, y, w, h, p.rotation), anchoring(anchor), file, titles(p));
        }
        case ItemType::Text: {
            const auto &t = static_cast<const TextItem &>(item);
            QString props = lookProps(t, false) + insets(t.insets, t.valign);
            if (t.noWrap) props += QStringLiteral(" fo:wrap-option=\"no-wrap\"");
            const QString words = paragraphs(t.storyId.isEmpty() ? nullptr : m_stories[t.storyId].get());
            return QStringLiteral("<draw:frame draw:style-name=\"%1\"%2%3%4><draw:text-box>%5</draw:text-box>%6</draw:frame>")
                .arg(graphicStyle(props), common, geometry(x, y, w, h, t.rotation), anchoring(anchor), words, titles(t));
        }
        case ItemType::Shape: {
            const auto &s = static_cast<const ShapeItem &>(item);
            QString prst = presetForShape(s.shape);
            if (prst.isEmpty() || !s.customPath.isEmpty()) {
                note(QCoreApplication::translate("Sheet", "freeform shapes (they are written as rectangles)"));
                prst = QStringLiteral("rect");
            }
            QString props = lookProps(s, false) + insets(s.insets, s.valign);
            const QString words = paragraphs(s.storyId.isEmpty() ? nullptr : m_stories[s.storyId].get());
            QString geom = QStringLiteral("<draw:enhanced-geometry svg:viewBox=\"0 0 21600 21600\" draw:type=\"%1\"").arg(odfTypeOfPreset(prst));
            if (s.flipH) geom += QStringLiteral(" draw:mirror-horizontal=\"true\"");
            if (s.flipV) geom += QStringLiteral(" draw:mirror-vertical=\"true\"");
            geom += QStringLiteral("/>");
            return QStringLiteral("<draw:custom-shape draw:style-name=\"%1\"%2%3%4>%5%6%7</draw:custom-shape>").arg(graphicStyle(props), common, geometry(x, y, w, h, s.rotation), anchoring(anchor), titles(s), words, geom);
        }
        case ItemType::TextArt: {
            const auto &a = static_cast<const TextArtItem &>(item);
            const QString warp = textWarpPreset(a.transform_);
            QString type = fontworkOfWarp(warp);
            if (type.isEmpty()) {
                note(QCoreApplication::translate("Sheet", "Text Art warps OpenDocument has no fontwork shape for (the words and style are kept, the warp is written as plain)"));
                type = QStringLiteral("fontwork-plain-text");
            }
            QString props = lookProps(a, false);
            props += QStringLiteral(" draw:fontwork-style=\"rotate\" draw:fontwork-adjust=\"%1\" draw:fontwork-distance=\"0in\" draw:fontwork-form=\"none\" draw:fontwork-hide-form=\"false\""
                                    " draw:fontwork-mirror=\"false\" draw:fontwork-shadow=\"none\" draw:fontwork-same-letter-heights=\"%2\"")
                         .arg(a.align == 0 ? QStringLiteral("left") : a.align == 2 ? QStringLiteral("right") : a.align == 5 ? QStringLiteral("autosize") : QStringLiteral("center"), a.evenHeight ? QStringLiteral("true") : QStringLiteral("false"));
            QTextCharFormat f;
            f.setFontFamilies({a.font});
            f.setFontPointSize(a.size);
            f.setFontWeight(a.bold ? QFont::Bold : QFont::Normal);
            f.setFontItalic(a.italic);
            const QString span = textStyle(f);
            const QString para = paragraphStyle(a.align == 0 ? Qt::AlignLeft : a.align == 2 ? Qt::AlignRight : Qt::AlignHCenter);
            QString words;
            for (const QString &line : a.text.split(QLatin1Char('\n')))
                words += QStringLiteral("<text:p text:style-name=\"%1\"><text:span text:style-name=\"%2\">%3</text:span></text:p>").arg(para, span, textXml(line.left(kMaxTextChars)));
            QString geom = QStringLiteral("<draw:enhanced-geometry svg:viewBox=\"0 0 21600 21600\" draw:text-path=\"true\" draw:text-path-mode=\"normal\" draw:text-path-scale=\"path\" draw:text-path-same-letter-heights=\"%1\""
                                          " draw:path-stretchpoint-x=\"10800\" draw:path-stretchpoint-y=\"10800\" draw:type=\"%2\"")
                               .arg(a.evenHeight ? QStringLiteral("true") : QStringLiteral("false"), type);
            if (a.flipH) geom += QStringLiteral(" draw:mirror-horizontal=\"true\"");
            if (a.flipV) geom += QStringLiteral(" draw:mirror-vertical=\"true\"");
            geom += QStringLiteral("/>");
            return QStringLiteral("<draw:custom-shape draw:style-name=\"%1\"%2%3%4>%5%6%7</draw:custom-shape>").arg(graphicStyle(props), common, geometry(x, y, w, h, a.rotation), anchoring(anchor), titles(a), words, geom);
        }
        case ItemType::Line: {
            const auto &l = static_cast<const LineItem &>(item);
            const QRectF r = l.rect;
            auto at = [&](const QPointF &p, double *px, double *py) {
                *px = x + (r.width() > 0 ? (p.x() - r.x()) / r.width() : 0) * w;
                *py = y + (r.height() > 0 ? (p.y() - r.y()) / r.height() : 0) * h;
            };
            double x1, y1, x2, y2;
            at(l.p1, &x1, &y1);
            at(l.p2, &x2, &y2);
            QString props = lookProps(l, true);
            props.replace(QStringLiteral(" draw:fill=\"none\""), QString());
            auto len = [](double v) { return lengthText(qint64(std::llround(v))); };
            return QStringLiteral("<draw:line draw:style-name=\"%1\"%2 svg:x1=\"%3\" svg:y1=\"%4\" svg:x2=\"%5\" svg:y2=\"%6\"%7>%8</draw:line>")
                .arg(graphicStyle(props), common, len(x1), len(y1), len(x2), len(y2), anchoring(anchor), titles(l));
        }
        case ItemType::Group: {
            if (depth >= kMaxGroupDepth) {
                note(QCoreApplication::translate("Sheet", "groups nested deeper than %1").arg(kMaxGroupDepth));
                return QString();
            }
            const auto &g = static_cast<const GroupItem &>(item);
            const QRectF r = g.rect;
            const double sx = r.width() > 0 ? w / (r.width() * kEmuPerPoint) : 1, sy = r.height() > 0 ? h / (r.height() * kEmuPerPoint) : 1;
            QString inner;
            for (const ItemPtr &c : g.children) {
                const QRectF cr = c->type() == ItemType::Line ? c->bounds() : c->rect;
                inner += emit(*c, x + (cr.x() - r.x()) * kEmuPerPoint * sx, y + (cr.y() - r.y()) * kEmuPerPoint * sy, std::max(1.0, cr.width() * kEmuPerPoint * sx), std::max(1.0, cr.height() * kEmuPerPoint * sy),
                              depth + 1, nullptr, 0);
            }
            if (inner.isEmpty()) return QString();
            return QStringLiteral("<draw:g%1%2>%3%4</draw:g>").arg(common, anchoring(anchor), titles(g), inner);
        }
        default:
            note(QCoreApplication::translate("Sheet", "drawing objects of a kind OpenDocument has no element for"));
            return QString();
        }
    }

    static QString insets(const QMarginsF &m, jp::VAlign v)
    {
        auto len = [](double pt) { return lengthText(qint64(std::llround(pt * kEmuPerPoint))); };
        return QStringLiteral(" fo:padding-left=\"%1\" fo:padding-top=\"%2\" fo:padding-right=\"%3\" fo:padding-bottom=\"%4\" draw:textarea-vertical-align=\"%5\"")
            .arg(len(m.left()), len(m.top()), len(m.right()), len(m.bottom()), v == jp::VAlign::Top ? QStringLiteral("top") : v == jp::VAlign::Bottom ? QStringLiteral("bottom") : QStringLiteral("middle"));
    }
};

} // namespace

void drawingToShapes(const Package &package, const QString &mainPart, const QString &drawingPart, const Grid &grid, const QString &sheetRef, int sheetNumber, Written *out, qint64 *pictureBytes, qint64 *objects)
{
    Writer w(package, mainPart, drawingPart, grid, sheetRef, sheetNumber, out, pictureBytes, objects);
    w.run();
}

} // namespace jp::office::sheet::odsdraw
