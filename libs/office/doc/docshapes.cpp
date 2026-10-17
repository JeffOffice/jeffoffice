// The drawings of an old binary document: the shapes a paragraph anchors (pictures, boxes, lines, text boxes) are in the binary drawing
// records of the table stream, each with its place on the page in a table of the document's own. They are written as the VML shapes of a
// .docx, as the original program wrote them, so the .docx reader's own reading of them is what shows them.

#include "doc/docconvert.h"

#include "office/odraw.h"
#include "office/odraw_read.h"

#include <QCoreApplication>
#include <algorithm>
#include <cmath>
#include <functional>

namespace jp::office::doc {

namespace {

constexpr int kMostGroupDepth = 8;
constexpr int kMostProps = 1500;

QByteArray pt(double points)
{
    return QByteArray::number(std::round(points * 100) / 100, 'f', 2).replace(".00", "");
}

} // namespace

struct DrawnShape {
    int spid = 0, type = 0;
    quint32 flags = 0;
    QHash<int, quint32> props;
    bool childAnchor = false;
    qint32 anchor[4] = {0, 0, 0, 0};       // in the group's coordinates: left, top, right, bottom
    qint32 space[4] = {0, 0, 0, 0};        // a group's own coordinates for its members
    bool isGroup = false;
    QVector<DrawnShape> members;
};

namespace {

// The properties in a drawing's property table (a record 0xF00B, 0xF121, or 0xF122), by number.
void readProps(const QByteArray &d, const OdrawRecord &r, QHash<int, quint32> *out)
{
    const int count = std::min<int>(r.instance, kMostProps);
    for (int i = 0; i < count; ++i) {
        const qint64 at = qint64(r.offset) + 6 * qint64(i);
        if (!odraw::fits(d, at, 6) || at + 6 > qint64(r.offset) + r.length) break;
        out->insert(odraw::u16(d, at) & 0x3FFF, odraw::u32(d, at + 2));
    }
}

bool readShape(const QByteArray &d, const OdrawRecord &sp, DrawnShape *s)
{
    if (sp.type != 0xF004) return false;
    for (const OdrawRecord &k : odrawRecords(d, sp.offset, qsizetype(qint64(sp.offset) + sp.length))) {
        if (k.type == 0xF00A && k.length >= 8) {
            s->type = k.instance;
            s->spid = int(odraw::u32(d, k.offset));
            s->flags = odraw::u32(d, k.offset + 4);
        } else if (k.type == 0xF00B || k.type == 0xF121 || k.type == 0xF122) {
            readProps(d, k, &s->props);
        } else if (k.type == 0xF00F && k.length >= 16) {
            s->childAnchor = true;
            for (int i = 0; i < 4; ++i) s->anchor[i] = odraw::i32(d, k.offset + 4 * qint64(i));
        } else if (k.type == 0xF009 && k.length >= 16) {
            s->isGroup = true;
            for (int i = 0; i < 4; ++i) s->space[i] = odraw::i32(d, k.offset + 4 * qint64(i));
        }
    }
    return true;
}

// The shapes of a group container (0xF003): its first shape is the group itself.
void readGroup(const QByteArray &d, const OdrawRecord &g, DrawnShape *group, int depth, int *count)
{
    bool first = true;
    for (const OdrawRecord &k : odrawRecords(d, g.offset, qsizetype(qint64(g.offset) + g.length))) {
        if (*count >= kMaxShapes) return;
        if (k.type == 0xF004) {
            DrawnShape s;
            if (!readShape(d, k, &s)) continue;
            ++*count;
            if (first) {
                // (The group's own: its coordinates for the members, and its own place among its parent's.)
                for (int i = 0; i < 4; ++i) {
                    group->space[i] = s.space[i];
                    if (s.childAnchor) group->anchor[i] = s.anchor[i];
                }
                group->childAnchor = group->childAnchor || s.childAnchor;
                group->spid = group->spid ? group->spid : s.spid;
                group->props = s.props;
                group->flags = s.flags;
                group->type = s.type;
                first = false;
            } else {
                group->members.push_back(s);
            }
        } else if (k.type == 0xF003 && depth < kMostGroupDepth) {
            DrawnShape sub;
            sub.isGroup = true;
            readGroup(d, k, &sub, depth + 1, count);
            group->members.push_back(sub);
        }
    }
}

} // namespace

// The drawings of the file and the pictures its shapes use, read once.
struct Converter::Drawings {
    bool read = false;
    QVector<OdrawStoreEntry> store;
    QHash<int, DrawnShape> shapes[2];      // the top-level shapes of the main drawing and of the headers', by id
    QHash<int, Picture> pictures;          // by 1-based place in the picture store
    QSet<qint64> textBoxesUsed;            // (a shape's id doubled, and one more for a header's: two ids can't make one key)
    QHash<int, int> boxOf[2];              // the text box of a shape (by its id), in the table of the main text's boxes and the headers'
    bool boxesRead[2] = {false, false};
    int emitted = 0;
    qint64 visits = 0;                     // the shapes walked over for all the places together, shown or not
};

void Converter::readDrawings()
{
    if (m_drawings) return;
    m_drawings = std::make_shared<Drawings>();
    Drawings &dr = *m_drawings;
    const Span s = tableRange(Fib::DggInfo);
    if (s.size() < 8) return;
    const QByteArray d = s.bytes(0, s.size());
    qint64 pos = 0;
    // The group's records (the picture store among them), then each drawing after a byte that says whose it is.
    {
        if (odraw::u16(d, 2) != 0xF000) return;
        const qint64 groupEnd = std::min<qint64>(d.size(), 8 + qint64(odraw::u32(d, 4)));
        for (const OdrawRecord &k : odrawRecords(d, 8, qsizetype(groupEnd)))
            if (k.type == 0xF001) dr.store = odrawStore(d.mid(qsizetype(k.offset) - 8, qsizetype(8 + qint64(k.length))));
        pos = groupEnd;
    }
    for (int which = 0; pos + 9 <= d.size() && which < 2; ++which) {
        const int label = odraw::u8(d, pos);
        ++pos;
        if (odraw::u16(d, pos + 2) != 0xF002) break;
        const qint64 len = odraw::u32(d, pos + 4);
        const qint64 end = std::min<qint64>(d.size(), pos + 8 + len);
        for (const OdrawRecord &k : odrawRecords(d, qsizetype(pos + 8), qsizetype(end))) {
            if (k.type != 0xF003) continue;
            // The top group: its first shape is the page's own; the others are what is anchored.
            DrawnShape top;
            top.isGroup = true;
            int count = 0;
            readGroup(d, k, &top, 0, &count);
            for (const DrawnShape &s2 : top.members) dr.shapes[label == 1 ? 1 : 0].insert(s2.spid, s2);
        }
        pos = end;
    }
}

Picture *Converter::storedPicture(int index)
{
    readDrawings();
    Drawings &dr = *m_drawings;
    auto it = dr.pictures.find(index);
    if (it == dr.pictures.end()) {
        Picture pic;
        if (index >= 1 && index <= dr.store.size() && m_pictureBytes <= kMaxPictureTotal) {
            const OdrawStoreEntry &e = dr.store[index - 1];
            QByteArray blip = e.embedded;
            if (blip.isEmpty() && e.delayOffset && wd.fits(e.delayOffset, 8)) {
                const qint64 len = wd.u32(qint64(e.delayOffset) + 4);
                const quint16 type = wd.u16(qint64(e.delayOffset) + 2);
                if (type >= 0xF018 && type <= 0xF117 && len <= kMaxPictureBytes && wd.fits(e.delayOffset, 8 + len)) blip = wd.bytes(e.delayOffset, 8 + len);
            }
            if (!blip.isEmpty()) {
                QString format;
                pic.bytes = odrawBlip(blip, &format, &m_pictureWork);
                pic.extension = format;
                if (pic.bytes.size() > kMaxPictureBytes || format.isEmpty()) pic.bytes.clear();
            }
        }
        // (The size a picture is shown at is its shape's, not its own.)
        it = dr.pictures.insert(index, pic);
    }
    return it->bytes.isEmpty() ? nullptr : &it.value();
}

namespace {

// A color of a drawing's property as "#RRGGBB" (the file's own, or the one it means by an index into a scheme or the system's: black or white).
QByteArray colorOf(quint32 v, bool fill)
{
    if (v & 0x08000000 || v & 0x10000000) {
        const int index = v & 0xFF;
        // (The fill and the line of the scheme: white and black; the system's own, by its index.)
        if (v & 0x10000000) return index == 0x08 || index == 0x0F ? "#000000" : index == 0x05 || index == 0x09 || index == 0x0E ? "#FFFFFF" : (fill ? "#FFFFFF" : "#000000");
        return fill ? "#FFFFFF" : "#000000";
    }
    return "#" + QByteArray::number(((v & 0xFF) << 16) | (v & 0xFF00) | ((v >> 16) & 0xFF), 16).rightJustified(6, '0').toUpper();
}

// A flag among a boolean property's bits: said only where its "use" bit (sixteen places up) is.
std::optional<bool> flagOf(const QHash<int, quint32> &props, int id, int bit)
{
    const auto it = props.constFind(id);
    if (it == props.constEnd()) return std::nullopt;
    if (!(it.value() & (quint32(1) << (bit + 16)))) return std::nullopt;
    return bool(it.value() & (quint32(1) << bit));
}

} // namespace

// A shape in the line of text that has no picture of its own: a picture of the drawing's own store (its place in the property table), a rule across the
// width, or a box.
QByteArray Converter::inlineShapeXml(const Picture &pic, const QString &part)
{
    DrawnShape s;
    const QVector<OdrawRecord> first = odrawRecords(pic.shape);
    if (first.isEmpty() || !readShape(pic.shape, first.first(), &s)) return QByteArray();
    if (const int pib = int(s.props.value(0x104, 0))) {
        if (Picture *stored = storedPicture(pib)) {
            Picture shown = *stored;
            shown.width = std::max(1.0, pic.width);
            shown.height = std::max(1.0, pic.height);
            return drawingXml(shown, addMedia(part, *stored, QStringLiteral("s%1").arg(pib)));
        }
    }
    const bool filled = flagOf(s.props, 0x1BF, 4).value_or(true), stroked = flagOf(s.props, 0x1FF, 3).value_or(false);
    if (s.type != 1 && s.type != 3 && s.type != 2) {
        if (!wasLeft(QStringLiteral("drawings"))) left(QStringLiteral("drawings"), QCoreApplication::translate("jp::office::doc", "Some drawings could not be shown, and were left out."));
        return QByteArray();
    }
    const bool rule = s.props.contains(0x394);
    const QByteArray tag = s.type == 3 ? "v:oval" : "v:rect";
    QByteArray x = "<w:pict><" + tag + " id=\"_x0000_i" + QByteArray::number(++m_drawingId) + "\" style=\"width:" + (pic.width > 0 ? pt(pic.width) + "pt" : QByteArray("0")) + ";height:" + pt(pic.height) + "pt\"";
    if (rule) x += " o:hralign=\"center\" o:hrstd=\"t\" o:hr=\"t\"";
    if (filled) x += " filled=\"t\" fillcolor=\"" + (s.props.contains(0x181) ? colorOf(s.props.value(0x181), true) : QByteArray("#FFFFFF")) + "\"";
    else x += " filled=\"f\"";
    if (stroked) x += " stroked=\"t\"" + (s.props.contains(0x1C0) ? " strokecolor=\"" + colorOf(s.props.value(0x1C0), false) + "\"" : QByteArray());
    else x += " stroked=\"f\"";
    x += "/></w:pict>";
    return x;
}

// The VML shapes for the anchor at character `cp` of a story: empty if there are none.
QByteArray Converter::shapeXml(qint64 cp, Kind kind, const QString &part)
{
    if (kind != Kind::Main && kind != Kind::HeaderFooter) return QByteArray();
    const bool header = kind == Kind::HeaderFooter;
    const Plc spa(tableRange(header ? Fib::PlcSpaHdr : Fib::PlcSpaMom), 26);
    const qint64 rel = header ? cp - fib.headerStart() : cp;
    int found = -1;
    {
        int lo = 0, hi = spa.n - 1;
        while (lo <= hi) {
            const int mid = (lo + hi) / 2;
            const qint64 c = spa.cp(mid);
            if (c == rel) { found = mid; break; }
            if (c < rel) lo = mid + 1; else hi = mid - 1;
        }
    }
    if (found < 0) return QByteArray();
    readDrawings();
    Drawings &dr = *m_drawings;
    const Span rec = spa.record(found);
    const int spid = rec.i32(0);
    const int x0 = rec.i32(4), y0 = rec.i32(8), x1 = rec.i32(12), y1 = rec.i32(16);
    const int sflags = rec.u16(20);
    const auto top = dr.shapes[header ? 1 : 0].constFind(spid);
    if (top == dr.shapes[header ? 1 : 0].constEnd()) {
        if (!wasLeft(QStringLiteral("shapes"))) left(QStringLiteral("shapes"), QCoreApplication::translate("jp::office::doc", "Drawings and text boxes in the document were not brought in."));
        return QByteArray();
    }
    struct Frame { double x0, y0, x1, y1; };   // where a group is among the text, in twips from where the shape is anchored
    QByteArray out;
    const int bx = (sflags >> 1) & 3, by = (sflags >> 3) & 3, wr = (sflags >> 5) & 15;
    const bool below = sflags & 0x4000;

    // Writes one shape (and, for a group, its members) with `frame` the box it is in and `space` the coordinates its members are in.
    std::function<void(const DrawnShape &, const Frame &, const qint32 *, int)> emit = [&](const DrawnShape &s, const Frame &frame, const qint32 *space, int depth) {
        if (dr.emitted >= kMaxShapes) return;
        // (A group that many places anchor is walked for each of them, its members that are not shown as well: the walk has a budget of its own.)
        if (++dr.visits > kMaxShapeVisits) {
            if (!wasLeft(QStringLiteral("drawings"))) left(QStringLiteral("drawings"), QCoreApplication::translate("jp::office::doc", "Some drawings could not be shown, and were left out."));
            return;
        }
        // Where it is: in a group, its anchor scaled from the group's coordinates into the group's frame.
        Frame box = frame;
        if (s.childAnchor && space) {
            const double sw = double(space[2]) - space[0], sh = double(space[3]) - space[1];
            const double fw = frame.x1 - frame.x0, fh = frame.y1 - frame.y0;
            auto mapX = [&](double v) { return frame.x0 + (sw != 0 ? (v - space[0]) * fw / sw : 0); };
            auto mapY = [&](double v) { return frame.y0 + (sh != 0 ? (v - space[1]) * fh / sh : 0); };
            box = {mapX(s.anchor[0]), mapY(s.anchor[1]), mapX(s.anchor[2]), mapY(s.anchor[3])};
        }
        if (s.isGroup) {
            if (depth >= kMostGroupDepth) return;
            for (const DrawnShape &m : s.members) emit(m, box, s.space, depth + 1);
            return;
        }
        if (s.flags & 0x400) return;                 // (the page's background)
        if (flagOf(s.props, 0x3BF, 1).value_or(false)) return;   // hidden
        const double w = std::max(0.0, (box.x1 - box.x0) / 20.0), h = std::max(0.0, (box.y1 - box.y0) / 20.0);
        const int pib = int(s.props.value(0x104, 0));
        // The kind of shape, and what it holds.
        enum { TextBox, PictureFrame, Rect, Oval, Line, Skip } what = Skip;
        int textBox = -1;
        {
            const int which = header ? 1 : 0;
            if (!dr.boxesRead[which]) {
                dr.boxesRead[which] = true;
                const Plc boxes(tableRange(header ? Fib::PlcfHdrtxbxTxt : Fib::PlcftxbxTxt), 22);
                qint64 last = 0;
                bool sorted = true;
                for (int i = 0; i <= boxes.n && i < kMaxTextBoxes; ++i) {
                    if (boxes.cp(i) < last) sorted = false;
                    last = boxes.cp(i);
                }
                // (The stories are one after another: a table that says otherwise would have a story read again and again.)
                if (sorted)
                    for (int i = 0; i < boxes.n && i < kMaxTextBoxes; ++i)
                        if (!dr.boxOf[which].contains(boxes.record(i).i32(14))) dr.boxOf[which].insert(boxes.record(i).i32(14), i);
            }
            textBox = dr.boxOf[which].value(s.spid, -1);
        }
        Picture *pic = pib ? storedPicture(pib) : nullptr;
        if (textBox >= 0) what = TextBox;
        else if (pic) what = PictureFrame;
        else if (s.type == 20) what = Line;
        else if (s.type == 3) what = Oval;
        else if (s.type == 1 || s.type == 2 || s.type == 202) what = Rect;
        if (what == Skip) {
            if (!wasLeft(QStringLiteral("drawings"))) left(QStringLiteral("drawings"), QCoreApplication::translate("jp::office::doc", "Some drawings could not be shown, and were left out."));
            return;
        }
        ++dr.emitted;
        const QByteArray id = "_x0000_s" + QByteArray::number(s.spid);
        QByteArray style = "position:absolute;margin-left:" + pt(box.x0 / 20.0) + "pt;margin-top:" + pt(box.y0 / 20.0) + "pt;width:" + pt(w) + "pt;height:" + pt(h) + "pt;z-index:" +
                           QByteArray::number(below ? -251658240 + 1024 * dr.emitted : 251658240 + 1024 * dr.emitted);
        // What it is placed by: the margin, the page, or the text (the column and the paragraph); where the file says so in the drawing, that.
        auto relName = [](int code, bool horizontal) { return code == 0 ? "margin" : code == 1 ? "page" : horizontal ? "text" : "text"; };
        const int hrel = s.props.contains(0x390) ? int(std::min<quint32>(s.props.value(0x390), 2)) : bx;
        const int vrel = s.props.contains(0x392) ? int(std::min<quint32>(s.props.value(0x392), 2)) : by;
        if (hrel != 2) style += QByteArray(";mso-position-horizontal-relative:") + relName(hrel, true);
        if (vrel != 2) style += QByteArray(";mso-position-vertical-relative:") + relName(vrel, false);
        auto emu = [&](int id2, double def) { return s.props.contains(id2) ? double(qint32(s.props.value(id2))) / 12700.0 : def; };
        if (s.props.contains(0x384) || s.props.contains(0x386)) style += ";mso-wrap-distance-left:" + pt(emu(0x384, 9)) + "pt;mso-wrap-distance-right:" + pt(emu(0x386, 9)) + "pt";
        if (s.props.contains(0x385) || s.props.contains(0x387)) style += ";mso-wrap-distance-top:" + pt(emu(0x385, 0)) + "pt;mso-wrap-distance-bottom:" + pt(emu(0x387, 0)) + "pt";
        if (const double rot = double(qint32(s.props.value(0x4, 0))) / 65536.0; rot != 0) style += ";rotation:" + pt(rot);
        if ((s.flags & 0x40) || (s.flags & 0x80)) style += QByteArray(";flip:") + ((s.flags & 0x40) ? "x" : "") + ((s.flags & 0x80) ? "y" : "");

        // The fill and the line.
        const bool filled = flagOf(s.props, 0x1BF, 4).value_or(what == Rect || what == Oval);
        const bool stroked = flagOf(s.props, 0x1FF, 3).value_or(what == Rect || what == Oval || what == Line);
        QByteArray attrs = " id=\"" + id + "\" style=\"" + style + "\"";
        attrs += filled ? QByteArray(" filled=\"t\"") + (s.props.contains(0x181) ? " fillcolor=\"" + colorOf(s.props.value(0x181), true) + "\"" : QByteArray()) : QByteArray(" filled=\"f\"");
        if (stroked) {
            attrs += " stroked=\"t\"";
            if (s.props.contains(0x1C0)) attrs += " strokecolor=\"" + colorOf(s.props.value(0x1C0), false) + "\"";
            attrs += " strokeweight=\"" + pt(emu(0x1CB, 0.75)) + "pt\"";
        } else {
            attrs += " stroked=\"f\"";
        }
        QByteArray inner;
        if (what == PictureFrame) {
            attrs = " type=\"#_x0000_t75\"" + attrs;
            inner = "<v:imagedata r:id=\"" + addMedia(part, *pic, QStringLiteral("s%1").arg(pib)).toUtf8() + "\" o:title=\"\"/>";
        } else if (what == TextBox) {
            attrs = " type=\"#_x0000_t202\"" + attrs;
            const Plc boxes(tableRange(header ? Fib::PlcfHdrtxbxTxt : Fib::PlcftxbxTxt), 22);
            const qint64 base = header ? fib.headerTextBoxStart() : fib.textBoxStart(), size = header ? fib.ccpHdrTxbx : fib.ccpTxbx;
            qint64 a = boxes.cp(textBox), b = boxes.cp(textBox + 1);
            b = std::min(b, size);
            QByteArray content;
            const qint64 key = qint64(s.spid) * 2 + (header ? 1 : 0);
            if (b > a && a >= 0 && !dr.textBoxesUsed.contains(key)) {
                dr.textBoxesUsed.insert(key);
                // (A text box's text ends in one paragraph mark more than it has paragraphs.)
                qint64 stop = base + b;
                if (stop > base + a && pieces.text().size() >= stop && pieces.text()[stop - 1] == QChar(0x0D)) --stop;
                content = story(base + a, stop, Kind::TextBox, part);
            }
            if (content.isEmpty()) content = "<w:p/>";
            QByteArray textbox = "<v:textbox";
            if (s.props.contains(0x81) || s.props.contains(0x82) || s.props.contains(0x83) || s.props.contains(0x84))
                textbox += " inset=\"" + pt(emu(0x81, 7.2)) + "pt," + pt(emu(0x82, 3.6)) + "pt," + pt(emu(0x83, 7.2)) + "pt," + pt(emu(0x84, 3.6)) + "pt\"";
            inner = textbox + "><w:txbxContent>" + content + "</w:txbxContent></v:textbox>";
        }
        QByteArray wrap;
        if (wr == 1) wrap = "<w10:wrap type=\"topAndBottom\"/>";
        else if (wr == 4) wrap = "<w10:wrap type=\"tight\"/>";
        else if (wr == 5) wrap = "<w10:wrap type=\"through\"/>";
        else if (wr != 3) wrap = "<w10:wrap type=\"square\"/>";
        if (what == Line) {
            const bool fx = s.flags & 0x40, fy = s.flags & 0x80;
            const QByteArray from = (fx ? pt(w) : "0") + "pt," + (fy ? pt(h) : "0") + "pt", to = (fx ? "0" : pt(w)) + "pt," + (fy ? "0" : pt(h)) + "pt";
            QByteArray line = "<v:line" + attrs + " from=\"" + from + "\" to=\"" + to + "\">";
            const quint32 end = s.props.value(0x1D1, 0), start = s.props.value(0x1D0, 0);
            if (end || start) line += QByteArray("<v:stroke") + (start ? " startarrow=\"block\"" : "") + (end ? " endarrow=\"block\"" : "") + "/>";
            out += "<w:pict>" + line + wrap + "</v:line></w:pict>";
        } else {
            const QByteArray tag = what == Oval ? "v:oval" : what == Rect ? "v:rect" : "v:shape";
            out += "<w:pict><" + tag + attrs + ">" + inner + wrap + "</" + tag + "></w:pict>";
        }
    };
    emit(*top, Frame{double(x0), double(y0), double(x1), double(y1)}, nullptr, 0);
    return out;
}

} // namespace jp::office::doc
