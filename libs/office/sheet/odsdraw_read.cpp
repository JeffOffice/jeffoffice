// An OpenDocument spreadsheet's shapes into the drawing the XML form keeps (see odsdraw.h): each shape is read into the drawing code's own objects (core/items.h: a picture, a shape, a text
// box, Text Art, a line, a group), and those are written by the shared writer (office/drawingml_write.h) as the elements of a drawing part, each in the anchor its place in the sheet says.

#include "sheet/odsdraw.h"
#include "sheet/odsdraw_p.h"

#include "core/items.h"
#include "office/drawingml.h"
#include "office/drawingml_write.h"
#include "office/package.h"
#include "sheet/odsstyles.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QXmlStreamWriter>

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

namespace jp::office::sheet::odsdraw {

namespace {

constexpr double kEmuPerPoint = 12700.0;
using Props = QHash<QString, QString>;

struct Placed {
    ItemPtr item;
    qint64 x = 0, y = 0, cx = 0, cy = 0;   // EMU, from the sheet's top left
    bool hasZ = false;
    double z = 0;
    int order = 0;
    // where the shape says it is anchored
    bool inCell = false;
    quint32 row = 0, col = 0;
    bool hasEnd = false, cellAnchored = false;
    quint32 endCol = 0, endRow = 0;
    qint64 endX = 0, endY = 0;
};

class Converter {
public:
    Converter(const Grid &grid, const StyleLookup &styles, const std::function<QByteArray(const QString &)> &parts, Budget *budget, ImportedDrawing *out)
        : m_grid(grid), m_styles(styles), m_parts(parts), m_budget(budget), m_out(out)
    {
        m_scheme = builtinColorSchemes().value(0);
    }

    void run(const QVector<SheetShape> &shapes, const Counters &counters)
    {
        if (counters.over) m_notRead.insert(QCoreApplication::translate("jp::office::sheet::Ods", "drawing objects past the limits on drawings"));
        std::vector<Placed> placed;
        int order = 0;
        for (const SheetShape &s : shapes) {
            if (int(placed.size()) >= kMaxDrawingAnchors) {
                m_notRead.insert(QCoreApplication::translate("jp::office::sheet::Ods", "drawing objects past the %1 a sheet may have").arg(kMaxDrawingAnchors));
                break;
            }
            if (m_budget && m_budget->objects.load() >= kMaxWorkbookObjects) {
                m_notRead.insert(QCoreApplication::translate("jp::office::sheet::Ods", "drawing objects past the %1 a workbook may have").arg(kMaxWorkbookObjects));
                break;
            }
            Placed p;
            p.order = order++;
            if (!shape(s.node, 0, s.inCell ? m_grid.columnStart(s.col) : 0, s.inCell ? m_grid.rowStart(s.row) : 0, &p)) continue;
            p.inCell = s.inCell;
            p.row = s.row;
            p.col = s.col;
            if (m_budget) m_budget->objects.fetch_add(1);
            placed.push_back(std::move(p));
        }
        std::stable_sort(placed.begin(), placed.end(), [](const Placed &a, const Placed &b) {
            const double za = a.hasZ ? a.z : a.order, zb = b.hasZ ? b.z : b.order;
            return za < zb;
        });
        write(placed);
        for (const QString &n : std::as_const(m_notRead)) m_out->notRead << n;
    }

private:
    const Grid &m_grid;
    const StyleLookup &m_styles;
    const std::function<QByteArray(const QString &)> &m_parts;
    Budget *m_budget;
    ImportedDrawing *m_out;
    ColorScheme m_scheme;
    std::set<QString> m_notRead;
    // the pictures and the stories kept for the shapes
    QHash<QString, QByteArray> m_images;
    QHash<QString, QString> m_imageFormats;
    QHash<QByteArray, QString> m_imageOfBytes;
    std::map<QString, std::unique_ptr<QTextDocument>> m_stories;
    qint64 m_pictureBytes = 0;

    Props style(const QString &family, const QString &name) const { return name.isEmpty() ? Props() : m_styles(family, name); }

    static bool number(const QString &text, double *v)
    {
        bool ok = false;
        const double d = text.trimmed().toDouble(&ok);
        if (!ok || !std::isfinite(d)) return false;
        *v = d;
        return true;
    }
    static ColorRef colorOf(const QString &text, bool *ok)
    {
        quint32 rgb = 0;
        *ok = ods::colorOf(text, &rgb);
        return *ok ? ColorRef::rgb(QColor::fromRgb(rgb)) : ColorRef();
    }

    // ---- the look of a shape: its graphic style's fill and line
    void look(Item &item, const Props &p, bool defaultLine) const
    {
        const QString fill = p.value(QStringLiteral("graphic:draw:fill"));
        bool ok = false;
        const ColorRef color = colorOf(p.value(QStringLiteral("graphic:draw:fill-color")), &ok);
        if (fill == QLatin1String("solid")) {
            item.fill = Fill::solid(ok ? color : ColorRef::rgb(QColor(0x72, 0x9f, 0xcf)));
            double opacity = 100;
            const QString o = p.value(QStringLiteral("graphic:draw:opacity"));
            if (o.endsWith(QLatin1Char('%')) && number(o.chopped(1), &opacity)) item.fill.transparency = std::clamp(1.0 - opacity / 100.0, 0.0, 1.0);
        } else if ((fill == QLatin1String("gradient") || fill == QLatin1String("hatch") || fill == QLatin1String("bitmap")) && ok) {
            item.fill = Fill::solid(color);
        } else {
            item.fill = Fill::none();
        }
        const QString stroke = p.value(QStringLiteral("graphic:draw:stroke"));
        if (stroke == QLatin1String("none") || (stroke.isEmpty() && !defaultLine)) {
            item.stroke = Stroke::none();
            return;
        }
        const ColorRef sc = colorOf(p.value(QStringLiteral("graphic:svg:stroke-color")), &ok);
        double width = 0;
        qint64 emu = 0;
        if (lengthEmu(p.value(QStringLiteral("graphic:svg:stroke-width")), &emu)) width = double(emu) / kEmuPerPoint;
        Stroke s = Stroke::line(ok ? sc : ColorRef::rgb(Qt::black), width > 0 ? std::min(width, 1584.0) : 0.75);
        if (stroke == QLatin1String("dash")) {
            const QString dash = p.value(QStringLiteral("graphic:draw:stroke-dash")).toLower();
            s.dash = dash.contains(QLatin1String("dot")) && dash.contains(QLatin1String("dash")) ? Stroke::DashDot : dash.contains(QLatin1String("dot")) ? Stroke::RoundDot : dash.contains(QLatin1String("long")) ? Stroke::LongDash : Stroke::DashLine;
        }
        item.stroke = s;
    }

    // ---- text
    struct TextState {
        int chars = 0, paragraphs = 0;
        bool cut = false;
    };

    static QString collapse(const QString &s)
    {
        QString out;
        out.reserve(s.size());
        bool space = false;
        for (const QChar c : s) {
            if (c == QLatin1Char(' ') || c == QLatin1Char('\t') || c == QLatin1Char('\n') || c == QLatin1Char('\r')) {
                if (!space) out += QLatin1Char(' ');
                space = true;
            } else {
                out += c;
                space = false;
            }
        }
        return out;
    }

    QTextCharFormat formatOf(const QTextCharFormat &base, const Props &p) const
    {
        QTextCharFormat f = base;
        qint64 emu = 0;
        if (lengthEmu(p.value(QStringLiteral("text:fo:font-size")), &emu) && emu > 0) f.setFontPointSize(std::min(409.0, double(emu) / kEmuPerPoint));
        const QString weight = p.value(QStringLiteral("text:fo:font-weight"));
        if (!weight.isEmpty()) f.setFontWeight(weight == QLatin1String("bold") || weight.toInt() >= 600 ? QFont::Bold : QFont::Normal);
        const QString italic = p.value(QStringLiteral("text:fo:font-style"));
        if (!italic.isEmpty()) f.setFontItalic(italic == QLatin1String("italic") || italic == QLatin1String("oblique"));
        const QString underline = p.value(QStringLiteral("text:style:text-underline-style"));
        if (!underline.isEmpty()) f.setFontUnderline(underline != QLatin1String("none"));
        const QString strike = p.value(QStringLiteral("text:style:text-line-through-style"));
        if (!strike.isEmpty()) f.setFontStrikeOut(strike != QLatin1String("none"));
        bool ok = false;
        const ColorRef c = colorOf(p.value(QStringLiteral("text:fo:color")), &ok);
        if (ok) f.setForeground(QBrush(c.rgbValue()));
        QString family = p.value(QStringLiteral("text:fo:font-family"));
        if (family.isEmpty()) family = p.value(QStringLiteral("text:style:font-name"));
        family.remove(QLatin1Char('\''));
        if (!family.isEmpty() && family.size() < 200) f.setFontFamilies({family});
        return f;
    }

    // The characters of an element of text (a paragraph or a span) into the cursor, in the format the styles give.
    void runs(QTextCursor &cur, const NodePtr &n, const QTextCharFormat &format, TextState *st, bool *atStart, int depth) const
    {
        if (depth > 12) return;
        auto put = [&](QString s, const QTextCharFormat &f) {
            if (s.isEmpty() || st->cut) return;
            if (st->chars + s.size() > kMaxTextChars) {
                s.truncate(kMaxTextChars - st->chars);
                st->cut = true;
            }
            st->chars += s.size();
            cur.insertText(s, f);
        };
        for (int i = 0; i <= int(n->children.size()); ++i) {
            QString t = collapse(n->texts.value(i));
            if (*atStart && !t.isEmpty()) {
                if (t.startsWith(QLatin1Char(' '))) t.remove(0, 1);
                if (!t.isEmpty()) *atStart = false;
            }
            put(t, format);
            if (i == n->children.size()) break;
            const NodePtr &c = n->children[i];
            if (c->name == QLatin1String("text:span") || c->name == QLatin1String("text:a")) {
                runs(cur, c, formatOf(format, style(QStringLiteral("text"), c->attrs.value(QStringLiteral("text:style-name")))), st, atStart, depth + 1);
            } else if (c->name == QLatin1String("text:s")) {
                bool ok = false;
                const int count = c->attrs.value(QStringLiteral("text:c")).toInt(&ok);
                put(QString(std::clamp(ok ? count : 1, 1, 1000), QLatin1Char(' ')), format);
                *atStart = false;
            } else if (c->name == QLatin1String("text:tab")) {
                put(QStringLiteral("\t"), format);
                *atStart = false;
            } else if (c->name == QLatin1String("text:line-break")) {
                put(QString(QChar(QChar::LineSeparator)), format);
                *atStart = true;
            }
        }
    }

    void paragraphs(QTextCursor &cur, const NodePtr &n, TextState *st, bool *first, int depth) const
    {
        if (depth > 8) return;
        for (const NodePtr &c : n->children) {
            if (c->name == QLatin1String("text:p") || c->name == QLatin1String("text:h")) {
                if (st->paragraphs >= kMaxParagraphs || st->cut) {
                    st->cut = true;
                    return;
                }
                ++st->paragraphs;
                const Props pp = style(QStringLiteral("paragraph"), c->attrs.value(QStringLiteral("text:style-name")));
                QTextBlockFormat block;
                const QString align = pp.value(QStringLiteral("para:fo:text-align"));
                if (align == QLatin1String("center")) block.setAlignment(Qt::AlignHCenter);
                else if (align == QLatin1String("end") || align == QLatin1String("right")) block.setAlignment(Qt::AlignRight);
                else if (align == QLatin1String("justify")) block.setAlignment(Qt::AlignJustify);
                else block.setAlignment(Qt::AlignLeft);
                if (*first) cur.setBlockFormat(block);
                else cur.insertBlock(block);
                *first = false;
                QTextCharFormat base;
                base.setFontPointSize(18);
                const QTextCharFormat format = formatOf(base, pp);
                cur.setBlockCharFormat(format);
                bool atStart = true;
                runs(cur, c, format, st, &atStart, 0);
            } else if (c->name == QLatin1String("text:list") || c->name == QLatin1String("text:list-item") || c->name == QLatin1String("text:list-header")) {
                paragraphs(cur, c, st, first, depth + 1);
            }
        }
    }

    bool hasText(const NodePtr &n) const
    {
        for (const NodePtr &c : n->children)
            if (c->name == QLatin1String("text:p") || c->name == QLatin1String("text:h") || c->name == QLatin1String("text:list")) return true;
        return false;
    }

    // The words of the shape's paragraphs as a story of the items; its id, or empty if there are none.
    QString story(const NodePtr &n)
    {
        if (!hasText(n)) return QString();
        auto doc = std::make_unique<QTextDocument>();
        QTextCursor cur(doc.get());
        TextState st;
        bool first = true;
        paragraphs(cur, n, &st, &first, 0);
        if (st.cut) m_notRead.insert(QCoreApplication::translate("jp::office::sheet::Ods", "words of drawing objects past the %1 characters an object may have").arg(kMaxTextChars));
        const QString id = QStringLiteral("s%1").arg(m_stories.size() + 1);
        m_stories[id] = std::move(doc);
        return id;
    }

    // ---- the shapes

    static NodePtr childOf(const NodePtr &n, const char *name)
    {
        for (const NodePtr &c : n->children)
            if (c->name == QLatin1String(name)) return c;
        return nullptr;
    }

    // Where the shape's box is, its turn, and its name and alternative text, for any kind of shape with a width and a height.
    bool placeBox(const NodePtr &n, qint64 ox, qint64 oy, Placed *p, Item *item, double *turnDegrees) const
    {
        qint64 w = 0, h = 0, x = 0, y = 0;
        if (!lengthEmu(n->attrs.value(QStringLiteral("svg:width")), &w) || !lengthEmu(n->attrs.value(QStringLiteral("svg:height")), &h) || w <= 0 || h <= 0) return false;
        lengthEmu(n->attrs.value(QStringLiteral("svg:x")), &x);
        lengthEmu(n->attrs.value(QStringLiteral("svg:y")), &y);
        w = std::min(w, kMaxDrawingEmu);
        h = std::min(h, kMaxDrawingEmu);
        double turn = 0;
        const QString transform = n->attrs.value(QStringLiteral("draw:transform"));
        if (!transform.isEmpty()) {
            static const QRegularExpression rotate(QStringLiteral("rotate\\s*\\(\\s*([-+0-9.eE]+)\\s*\\)")), translate(QStringLiteral("translate\\s*\\(\\s*([^\\s,)]+)[\\s,]+([^\\s)]+)\\s*\\)"));
            const QRegularExpressionMatch r = rotate.match(transform), t = translate.match(transform);
            double angle = 0;
            if (r.hasMatch() && number(r.captured(1), &angle)) turn = -angle;   // (the standard turns against the clock; the drawing code with it)
            qint64 tx = x, ty = y;
            if (t.hasMatch()) {
                lengthEmu(t.captured(1), &tx);
                lengthEmu(t.captured(2), &ty);
            }
            if (turn != 0) {
                // the box's top left after the turn is (tx, ty): its center is half a box from it, turned
                const double c = std::cos(turn), s = std::sin(turn);
                const double hx = double(w) / 2, hy = double(h) / 2;
                x = qint64(std::llround(double(tx) + c * hx - s * hy - hx));
                y = qint64(std::llround(double(ty) + s * hx + c * hy - hy));
            } else {
                x = tx;
                y = ty;
            }
        }
        p->x = std::clamp<qint64>(x + ox, 0, kMaxDrawingEmu);
        p->y = std::clamp<qint64>(y + oy, 0, kMaxDrawingEmu);
        p->cx = w;
        p->cy = h;
        double degrees = std::fmod(turn * 180.0 / M_PI, 360.0);
        if (degrees < 0) degrees += 360.0;
        *turnDegrees = degrees;
        item->rect = QRectF(double(p->x) / kEmuPerPoint, double(p->y) / kEmuPerPoint, double(w) / kEmuPerPoint, double(h) / kEmuPerPoint);
        item->rotation = degrees;
        return true;
    }

    void names(const NodePtr &n, Item *item) const
    {
        item->name = n->attrs.value(QStringLiteral("draw:name")).left(255);
        const NodePtr title = childOf(n, "svg:title"), desc = childOf(n, "svg:desc");
        if (title) item->altTitle = title->texts.value(0).left(1000);
        if (desc) item->altText = desc->texts.value(0).left(5000);
    }

    // The picture a frame holds: its bytes, by the file's name or inside the document, and the format; empty if there is none or it is past a limit.
    QByteArray pictureOf(const NodePtr &image, QString *format)
    {
        QByteArray bytes;
        const QString href = image->attrs.value(QStringLiteral("xlink:href"));
        if (const NodePtr data = childOf(image, "office:binary-data")) {
            // (the picture is in the document: its bytes, as Base 64)
            qint64 chars = 0;
            for (const QString &t : data->texts) chars += t.size();
            if (chars > kMaxPictureBytes * 4 / 3 + 16) {
                m_notRead.insert(QCoreApplication::translate("jp::office::sheet::Ods", "pictures larger than %1 MB").arg(kMaxPictureBytes >> 20));
                return QByteArray();
            }
            bytes = QByteArray::fromBase64(data->texts.join(QString()).toLatin1());
        } else if (!href.isEmpty()) {
            QString path = href;
            if (path.startsWith(QLatin1String("./"))) path.remove(0, 2);
            if (path.contains(QLatin1String("://")) || path.startsWith(QLatin1Char('/')) || path.startsWith(QLatin1Char('#')) || path.contains(QLatin1String("..")) || path.contains(QLatin1Char(':'))) {
                m_notRead.insert(QCoreApplication::translate("jp::office::sheet::Ods", "pictures that are not inside the file"));
                return QByteArray();
            }
            bytes = m_parts(path);
        }
        if (bytes.isEmpty()) {
            m_notRead.insert(QCoreApplication::translate("jp::office::sheet::Ods", "pictures the file names and does not hold"));
            return QByteArray();
        }
        if (bytes.size() > kMaxPictureBytes) {
            m_notRead.insert(QCoreApplication::translate("jp::office::sheet::Ods", "pictures larger than %1 MB").arg(kMaxPictureBytes >> 20));
            return QByteArray();
        }
        *format = imageFormatOf(bytes);
        if (format->isEmpty()) {
            m_notRead.insert(QCoreApplication::translate("jp::office::sheet::Ods", "pictures in a form this program does not read"));
            return QByteArray();
        }
        return bytes;
    }

    QString imageId(const QByteArray &bytes, const QString &format)
    {
        const auto known = m_imageOfBytes.constFind(bytes);
        if (known != m_imageOfBytes.constEnd()) return known.value();
        // (the room all the workbook's pictures may take, and this drawing's)
        if (m_pictureBytes + bytes.size() > kMaxPictureTotal || (m_budget && m_budget->pictureBytes.load() + bytes.size() > kMaxPictureTotal)) {
            m_notRead.insert(QCoreApplication::translate("jp::office::sheet::Ods", "pictures past the %1 MB a workbook may keep").arg(kMaxPictureTotal >> 20));
            return QString();
        }
        m_pictureBytes += bytes.size();
        if (m_budget) m_budget->pictureBytes.fetch_add(bytes.size());
        const QString id = QStringLiteral("i%1").arg(m_images.size() + 1);
        m_images[id] = bytes;
        m_imageFormats[id] = format;
        m_imageOfBytes[bytes] = id;
        return id;
    }

    bool picture(const NodePtr &frame, const NodePtr &image, qint64 ox, qint64 oy, Placed *p)
    {
        auto item = std::make_shared<PictureItem>();
        double turn = 0;
        if (!placeBox(frame, ox, oy, p, item.get(), &turn)) return false;
        names(frame, item.get());
        QString format;
        const QByteArray bytes = pictureOf(image, &format);
        if (bytes.isEmpty()) return false;
        const QString id = imageId(bytes, format);
        if (id.isEmpty()) return false;
        item->imageId = id;
        const Props props = style(QStringLiteral("graphic"), frame->attrs.value(QStringLiteral("draw:style-name")));
        item->imgRect = QRectF(0, 0, item->rect.width(), item->rect.height());
        // the part of the picture that is cut off: fo:clip="rect(top, right, bottom, left)" in lengths of the picture's own size
        static const QRegularExpression clip(QStringLiteral("rect\\s*\\(\\s*([^,\\s)]+)[\\s,]+([^,\\s)]+)[\\s,]+([^,\\s)]+)[\\s,]+([^,\\s)]+)\\s*\\)"));
        const QRegularExpressionMatch m = clip.match(props.value(QStringLiteral("graphic:fo:clip")));
        const QSizeF natural = naturalSizePoints(bytes);
        if (m.hasMatch() && natural.width() > 0 && natural.height() > 0) {
            double v[4] = {0, 0, 0, 0};
            for (int i = 0; i < 4; ++i) {
                qint64 emu = 0;
                if (m.captured(i + 1) != QLatin1String("auto") && lengthEmu(m.captured(i + 1), &emu)) v[i] = double(emu) / kEmuPerPoint;
            }
            const double t = v[0] / natural.height(), r = v[1] / natural.width(), b = v[2] / natural.height(), l = v[3] / natural.width();
            if (l + r < 0.999 && t + b < 0.999 && (l || r || t || b)) {
                const double wholeW = item->rect.width() / (1 - l - r), wholeH = item->rect.height() / (1 - t - b);
                item->imgRect = QRectF(-l * wholeW, -t * wholeH, wholeW, wholeH);
            }
        }
        const QString mirror = props.value(QStringLiteral("graphic:style:mirror"));
        item->flipH = mirror.contains(QLatin1String("horizontal"));
        item->flipV = mirror.contains(QLatin1String("vertical"));
        item->lockAspect = true;
        look(*item, props, false);
        item->fill = Fill::none();
        p->item = item;
        return true;
    }

    bool textBox(const NodePtr &frame, const NodePtr &box, qint64 ox, qint64 oy, Placed *p)
    {
        auto item = std::make_shared<TextItem>();
        double turn = 0;
        if (!placeBox(frame, ox, oy, p, item.get(), &turn)) return false;
        names(frame, item.get());
        const Props props = style(QStringLiteral("graphic"), frame->attrs.value(QStringLiteral("draw:style-name")));
        look(*item, props, false);
        item->storyId = story(box);
        padding(props, &item->insets);
        const QString v = props.value(QStringLiteral("graphic:draw:textarea-vertical-align"));
        item->valign = v == QLatin1String("middle") ? jp::VAlign::Middle : v == QLatin1String("bottom") ? jp::VAlign::Bottom : jp::VAlign::Top;
        item->noWrap = props.value(QStringLiteral("graphic:fo:wrap-option")) == QLatin1String("no-wrap");
        p->item = item;
        return true;
    }

    static void padding(const Props &props, QMarginsF *insets)
    {
        auto one = [&](const char *name, double *into) {
            qint64 emu = 0;
            const QString key = QStringLiteral("graphic:fo:padding-") + QLatin1String(name);
            if (lengthEmu(props.value(key), &emu) || lengthEmu(props.value(QStringLiteral("graphic:fo:padding")), &emu)) *into = std::clamp(double(emu) / kEmuPerPoint, 0.0, 1000.0);
        };
        double l = insets->left(), t = insets->top(), r = insets->right(), b = insets->bottom();
        one("left", &l);
        one("top", &t);
        one("right", &r);
        one("bottom", &b);
        *insets = QMarginsF(l, t, r, b);
    }

    // A custom shape: Text Art if its geometry says the words follow a path, else a preset shape (or a rectangle) with its words.
    bool customShape(const NodePtr &n, qint64 ox, qint64 oy, Placed *p)
    {
        const NodePtr geometry = childOf(n, "draw:enhanced-geometry");
        const QString type = geometry ? geometry->attrs.value(QStringLiteral("draw:type")) : QString();
        const bool art = geometry && (type.startsWith(QLatin1String("fontwork-")) || geometry->attrs.value(QStringLiteral("draw:text-path")) == QLatin1String("true"));
        const Props props = style(QStringLiteral("graphic"), n->attrs.value(QStringLiteral("draw:style-name")));
        if (art) {
            auto item = std::make_shared<TextArtItem>();
            double turn = 0;
            if (!placeBox(n, ox, oy, p, item.get(), &turn)) return false;
            names(n, item.get());
            look(*item, props, false);
            // the words, one line a paragraph, and the look of the first letters
            QStringList lines;
            QTextCharFormat first;
            bool haveFirst = false;
            const QString id = story(n);
            if (!id.isEmpty()) {
                const QTextDocument *doc = m_stories[id].get();
                for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
                    lines << b.text().replace(QChar(QChar::LineSeparator), QLatin1Char('\n'));
                    if (!haveFirst) {
                        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it)
                            if (it.fragment().isValid() && !it.fragment().text().isEmpty()) {
                                first = it.fragment().charFormat();
                                haveFirst = true;
                                break;
                            }
                        if (!haveFirst) first = b.charFormat();
                        haveFirst = true;
                        item->align = b.blockFormat().alignment() & Qt::AlignRight ? 2 : b.blockFormat().alignment() & Qt::AlignHCenter ? 1 : b.blockFormat().alignment() & Qt::AlignJustify ? 3 : 0;
                    }
                }
                m_stories.erase(id);
            }
            item->text = lines.join(QLatin1Char('\n'));
            if (haveFirst) {
                if (!first.fontFamilies().toStringList().isEmpty()) item->font = first.fontFamilies().toStringList().value(0);
                item->bold = first.fontWeight() >= QFont::Bold;
                item->italic = first.fontItalic();
                if (first.fontPointSize() > 0) item->size = first.fontPointSize();
                // (the letters of Text Art take the shape's fill and line; the words' own color, if the shape has none)
                if (item->fill.isNone() && first.foreground().style() != Qt::NoBrush) item->fill = Fill::solid(ColorRef::rgb(first.foreground().color()));
            }
            const QString adjust = props.value(QStringLiteral("graphic:draw:fontwork-adjust"));
            if (adjust == QLatin1String("left")) item->align = 0;
            else if (adjust == QLatin1String("right")) item->align = 2;
            else if (adjust == QLatin1String("center")) item->align = 1;
            else if (adjust == QLatin1String("autosize")) item->align = 5;
            item->evenHeight = props.value(QStringLiteral("graphic:draw:fontwork-same-letter-heights")) == QLatin1String("true");
            item->vertical = false;
            const QString warp = type.startsWith(QLatin1String("fontwork-")) ? warpOfFontwork(type) : QStringLiteral("textPlain");
            const QString transform = textArtTransformOfWarp(warp);
            item->transform_ = transform.isEmpty() ? QStringLiteral("plain") : transform;
            item->flipH = geometry->attrs.value(QStringLiteral("draw:mirror-horizontal")) == QLatin1String("true");
            item->flipV = geometry->attrs.value(QStringLiteral("draw:mirror-vertical")) == QLatin1String("true");
            p->item = item;
            return true;
        }
        auto item = std::make_shared<ShapeItem>();
        double turn = 0;
        if (!placeBox(n, ox, oy, p, item.get(), &turn)) return false;
        names(n, item.get());
        look(*item, props, false);
        const QString shape = shapeForPreset(presetOfOdfType(type));
        item->shape = shape.isEmpty() ? QStringLiteral("rect") : shape;
        item->storyId = story(n);
        padding(props, &item->insets);
        const QString v = props.value(QStringLiteral("graphic:draw:textarea-vertical-align"));
        item->valign = v == QLatin1String("top") ? jp::VAlign::Top : v == QLatin1String("bottom") ? jp::VAlign::Bottom : jp::VAlign::Middle;
        item->flipH = geometry && geometry->attrs.value(QStringLiteral("draw:mirror-horizontal")) == QLatin1String("true");
        item->flipV = geometry && geometry->attrs.value(QStringLiteral("draw:mirror-vertical")) == QLatin1String("true");
        p->item = item;
        return true;
    }

    // The simple shapes of the older kind: a rectangle or an ellipse with its words.
    bool simpleShape(const NodePtr &n, const QString &preset, qint64 ox, qint64 oy, Placed *p)
    {
        auto item = std::make_shared<ShapeItem>();
        double turn = 0;
        if (!placeBox(n, ox, oy, p, item.get(), &turn)) return false;
        names(n, item.get());
        look(*item, style(QStringLiteral("graphic"), n->attrs.value(QStringLiteral("draw:style-name"))), false);
        const QString shape = shapeForPreset(preset);
        item->shape = shape.isEmpty() ? QStringLiteral("rect") : shape;
        item->storyId = story(n);
        p->item = item;
        return true;
    }

    bool line(const NodePtr &n, qint64 ox, qint64 oy, Placed *p)
    {
        qint64 v[4] = {0, 0, 0, 0};
        const char *keys[4] = {"svg:x1", "svg:y1", "svg:x2", "svg:y2"};
        for (int i = 0; i < 4; ++i)
            if (!lengthEmu(n->attrs.value(QLatin1String(keys[i])), &v[i])) return false;
        for (int i = 0; i < 4; ++i) v[i] = std::clamp<qint64>(v[i] + (i % 2 ? oy : ox), 0, kMaxDrawingEmu);
        auto item = std::make_shared<LineItem>();
        item->p1 = QPointF(double(v[0]) / kEmuPerPoint, double(v[1]) / kEmuPerPoint);
        item->p2 = QPointF(double(v[2]) / kEmuPerPoint, double(v[3]) / kEmuPerPoint);
        item->syncRect();
        names(n, item.get());
        look(*item, style(QStringLiteral("graphic"), n->attrs.value(QStringLiteral("draw:style-name"))), true);
        item->fill = Fill::none();
        p->x = std::min(v[0], v[2]);
        p->y = std::min(v[1], v[3]);
        p->cx = std::abs(v[2] - v[0]);
        p->cy = std::abs(v[3] - v[1]);
        p->item = item;
        return true;
    }

    bool group(const NodePtr &n, int depth, qint64 ox, qint64 oy, Placed *p)
    {
        if (depth >= kMaxGroupDepth) {
            m_notRead.insert(QCoreApplication::translate("jp::office::sheet::Ods", "groups nested deeper than %1").arg(kMaxGroupDepth));
            return false;
        }
        auto item = std::make_shared<GroupItem>();
        qint64 x0 = kMaxDrawingEmu, y0 = kMaxDrawingEmu, x1 = 0, y1 = 0;
        for (const NodePtr &c : n->children) {
            Placed q;
            if (!shape(c, depth + 1, ox, oy, &q)) continue;
            item->children.push_back(q.item);
            x0 = std::min(x0, q.x);
            y0 = std::min(y0, q.y);
            x1 = std::max(x1, q.x + q.cx);
            y1 = std::max(y1, q.y + q.cy);
        }
        if (item->children.empty()) return false;
        names(n, item.get());
        p->x = x0;
        p->y = y0;
        p->cx = std::max<qint64>(1, x1 - x0);
        p->cy = std::max<qint64>(1, y1 - y0);
        item->rect = QRectF(double(x0) / kEmuPerPoint, double(y0) / kEmuPerPoint, double(p->cx) / kEmuPerPoint, double(p->cy) / kEmuPerPoint);
        p->item = item;
        return true;
    }

    bool shape(const NodePtr &n, int depth, qint64 ox, qint64 oy, Placed *p)
    {
        if (!n) return false;
        bool ok = false;
        const QString &name = n->name;
        if (name == QLatin1String("draw:frame")) {
            if (const NodePtr image = childOf(n, "draw:image")) ok = picture(n, image, ox, oy, p);
            else if (const NodePtr box = childOf(n, "draw:text-box")) ok = textBox(n, box, ox, oy, p);
            else if (childOf(n, "draw:object") || childOf(n, "draw:object-ole")) return false;   // (a chart: said by the reader of the document)
            else m_notRead.insert(QCoreApplication::translate("jp::office::sheet::Ods", "frames of a kind this program does not read"));
        } else if (name == QLatin1String("draw:custom-shape")) {
            ok = customShape(n, ox, oy, p);
        } else if (name == QLatin1String("draw:rect")) {
            ok = simpleShape(n, QStringLiteral("rect"), ox, oy, p);
        } else if (name == QLatin1String("draw:ellipse") || name == QLatin1String("draw:circle")) {
            ok = simpleShape(n, QStringLiteral("ellipse"), ox, oy, p);
        } else if (name == QLatin1String("draw:line") || name == QLatin1String("draw:connector")) {
            ok = line(n, ox, oy, p);
        } else if (name == QLatin1String("draw:g")) {
            ok = group(n, depth, ox, oy, p);
        } else if (name == QLatin1String("draw:polygon") || name == QLatin1String("draw:polyline") || name == QLatin1String("draw:path") || name == QLatin1String("draw:caption")
                   || name == QLatin1String("draw:measure") || name == QLatin1String("draw:regular-polygon")) {
            m_notRead.insert(QCoreApplication::translate("jp::office::sheet::Ods", "freehand shapes and callouts"));
        } else if (name == QLatin1String("draw:control")) {
            m_notRead.insert(QCoreApplication::translate("jp::office::sheet::Ods", "form controls"));
        } else if (name.startsWith(QLatin1String("draw:"))) {
            m_notRead.insert(QCoreApplication::translate("jp::office::sheet::Ods", "drawing objects of a kind this program does not read"));
        }
        if (!ok) return false;
        if (depth == 0) {
            bool z = false;
            const int index = n->attrs.value(QStringLiteral("draw:z-index")).toInt(&z);
            p->hasZ = z;
            p->z = index;
            // where it is anchored: the end cell it names, or the cell it is in
            const QString end = n->attrs.value(QStringLiteral("table:end-cell-address"));
            if (!end.isEmpty()) {
                QString cell = end.mid(end.lastIndexOf(QLatin1Char('.')) + 1);
                cell.remove(QLatin1Char('$'));
                quint32 col = 0, row = 0;
                if (parseCell(cell, &col, &row)) {
                    p->hasEnd = true;
                    p->endCol = col;
                    p->endRow = row;
                    lengthEmu(n->attrs.value(QStringLiteral("table:end-x")), &p->endX);
                    lengthEmu(n->attrs.value(QStringLiteral("table:end-y")), &p->endY);
                }
            }
            p->cellAnchored = n->attrs.value(QStringLiteral("draw:anchor-type")) == QLatin1String("cell");
        }
        return true;
    }

    static bool parseCell(const QString &text, quint32 *col, quint32 *row)
    {
        int i = 0;
        quint64 c = 0;
        while (i < text.size() && text[i].isLetter() && text[i].unicode() < 128) {
            c = c * 26 + quint64(text[i].toUpper().unicode() - 'A' + 1);
            if (c > 100000) return false;
            ++i;
        }
        bool ok = false;
        const qint64 r = text.mid(i).toLongLong(&ok);
        if (i == 0 || !ok || c < 1 || r < 1) return false;
        *col = quint32(std::min<quint64>(c - 1, kMaxColumns - 1));
        *row = quint32(std::min<qint64>(r - 1, qint64(kMaxRows) - 1));
        return true;
    }

    // ---- the drawing

    DrawingAnchor anchorOf(const Placed &p) const
    {
        DrawingAnchor a;
        quint32 col = 0, row = 0;
        qint64 dx = 0, dy = 0;
        m_grid.columnAt(p.x, &col, &dx);
        m_grid.rowAt(p.y, &row, &dy);
        a.from = {col, row, dx, dy};
        a.cx = p.cx;
        a.cy = p.cy;
        a.x = p.x;
        a.y = p.y;
        if (p.hasEnd) {
            a.kind = AnchorKind::TwoCell;
            a.to = {p.endCol, p.endRow, std::max<qint64>(0, p.endX), std::max<qint64>(0, p.endY)};
            // (an end before the start is the start)
            if (a.to.col < a.from.col || (a.to.col == a.from.col && a.to.colOff < a.from.colOff)) {
                a.to.col = a.from.col;
                a.to.colOff = a.from.colOff;
            }
            if (a.to.row < a.from.row || (a.to.row == a.from.row && a.to.rowOff < a.from.rowOff)) {
                a.to.row = a.from.row;
                a.to.rowOff = a.from.rowOff;
            }
        } else if (p.inCell || p.cellAnchored) {
            a.kind = AnchorKind::OneCell;
        } else {
            a.kind = AnchorKind::Absolute;
        }
        a.editAs = a.kind == AnchorKind::Absolute ? AnchorKind::Absolute : a.kind == AnchorKind::OneCell ? AnchorKind::OneCell : AnchorKind::TwoCell;
        return a;
    }

    static QByteArray pointXml(const char *tag, const AnchorPoint &p)
    {
        return QByteArray("<xdr:") + tag + "><xdr:col>" + QByteArray::number(p.col) + "</xdr:col><xdr:colOff>" + QByteArray::number(p.colOff) + "</xdr:colOff><xdr:row>" + QByteArray::number(p.row)
               + "</xdr:row><xdr:rowOff>" + QByteArray::number(p.rowOff) + "</xdr:rowOff></xdr:" + tag + ">";
    }

    static QByteArray anchorXml(const DrawingAnchor &a, const QByteArray &element)
    {
        const QByteArray spaces = " xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                                  "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
        if (a.kind == AnchorKind::TwoCell) return "<xdr:twoCellAnchor" + spaces + ">" + pointXml("from", a.from) + pointXml("to", a.to) + element + "<xdr:clientData/></xdr:twoCellAnchor>";
        const QByteArray ext = "<xdr:ext cx=\"" + QByteArray::number(a.cx) + "\" cy=\"" + QByteArray::number(a.cy) + "\"/>";
        if (a.kind == AnchorKind::OneCell) return "<xdr:oneCellAnchor" + spaces + ">" + pointXml("from", a.from) + ext + element + "<xdr:clientData/></xdr:oneCellAnchor>";
        return "<xdr:absoluteAnchor" + spaces + "><xdr:pos x=\"" + QByteArray::number(a.x) + "\" y=\"" + QByteArray::number(a.y) + "\"/>" + ext + element + "<xdr:clientData/></xdr:absoluteAnchor>";
    }

    void write(const std::vector<Placed> &placed)
    {
        if (placed.empty()) return;
        PackageWriter writer;
        const QString part = QStringLiteral("/xl/drawings/drawing1.xml");
        QHash<QString, QString> mediaParts;
        DrawingWriteContext ctx;
        ctx.package = &writer;
        ctx.part = part;
        ctx.colors = m_scheme;
        ctx.mediaParts = &mediaParts;
        ctx.imageBytes = [this](const QString &id, QString *format) {
            if (format) *format = m_imageFormats.value(id);
            return m_images.value(id);
        };
        ctx.storyDoc = [this](const QString &id) -> const QTextDocument * {
            const auto it = m_stories.find(id);
            return it == m_stories.end() ? nullptr : it->second.get();
        };
        ctx.nextShapeId = 2;
        for (const Placed &p : placed) {
            QByteArray element;
            QBuffer buffer(&element);
            buffer.open(QIODevice::WriteOnly);
            QXmlStreamWriter w(&buffer);
            QStringList skipped;
            jp::office::writeShape(w, *p.item, ctx, &skipped);
            buffer.close();
            if (element.isEmpty()) {
                m_notRead.insert(QCoreApplication::translate("jp::office::sheet::Ods", "drawing objects the drawing code can't write"));
                continue;
            }
            element.replace("<p:", "<xdr:").replace("</p:", "</xdr:");
            element.replace("<xdr:nvPr/>", "");
            m_out->anchors.push_back(anchorXml(anchorOf(p), element));
        }
        if (m_out->anchors.isEmpty()) return;
        // the pictures the elements name: the writer made the relationships and parts; they are taken out of its package
        const QByteArray zip = writer.finish();
        Package made;
        QString why;
        if (!made.readLazy(zip, &why)) return;
        QHash<QString, int> mediaOf;
        for (const Relationship &rel : made.relationships(part)) {
            if (rel.external || !made.hasPart(rel.target)) continue;
            int index = mediaOf.value(rel.target, -1);
            if (index < 0) {
                index = int(m_out->media.size());
                ImportedMedia m;
                m.name = rel.target.mid(rel.target.lastIndexOf(QLatin1Char('/')) + 1);
                m.bytes = made.part(rel.target);
                m_out->media.push_back(m);
                mediaOf.insert(rel.target, index);
            }
            m_out->rels.push_back({rel.id, index});
        }
    }
};

} // namespace

void shapesToDrawing(const QVector<SheetShape> &shapes, const Counters &counters, const Grid &grid, const StyleLookup &styles, const std::function<QByteArray(const QString &)> &parts, Budget *budget,
                     ImportedDrawing *out)
{
    *out = ImportedDrawing();
    Converter c(grid, styles, parts, budget, out);
    c.run(shapes, counters);
}

} // namespace jp::office::sheet::odsdraw
