// The property sets of the old binary format: applying a group of sprms to them, and writing them as the XML of a .docx.

#include "doc/docprops.h"

#include <algorithm>
#include <functional>

namespace jp::office::doc {

namespace {

// The color of a COLORREF (red, green, blue, and a byte that is 0xFF for automatic) as 0xRRGGBB, or -1.
qint32 colorOfRef(quint32 v) { return (v >> 24) == 0xFF ? -1 : qint32(((v & 0xFF) << 16) | (v & 0xFF00) | ((v >> 16) & 0xFF)); }

qint32 rgbOfIco(int ico)
{
    static const qint32 palette[17] = {-1, 0x000000, 0x0000FF, 0x00FFFF, 0x00FF00, 0xFF00FF, 0xFF0000, 0xFFFF00, 0xFFFFFF, 0x000080, 0x008080, 0x008000, 0x800080, 0x800000, 0x808000, 0x808080, 0xC0C0C0};
    return ico >= 0 && ico <= 16 ? palette[ico] : -1;
}

Brc brcOld(const Span &a)
{
    Brc b;
    b.stated = true;
    if (a.u32(0) == 0xFFFFFFFFu) {
        b.nil = true;
        return b;
    }
    b.width = a.u8(0);
    b.type = a.u8(1);
    b.color = rgbOfIco(a.u8(2));
    const int f = a.u8(3);
    b.space = f & 0x1F;
    b.shadow = f & 0x20;
    b.frame = f & 0x40;
    return b;
}

Brc brcNew(const Span &a)
{
    Brc b;
    b.stated = true;
    if (a.u8(4) == 0xFF && a.u8(5) == 0xFF) {
        b.nil = true;
        return b;
    }
    b.color = colorOfRef(a.u32(0));
    b.width = a.u8(4);
    b.type = a.u8(5);
    const int f = a.u8(6);
    b.space = f & 0x1F;
    b.shadow = f & 0x20;
    b.frame = f & 0x40;
    return b;
}

Shd shdOld(quint16 v)
{
    Shd s;
    s.stated = true;
    s.fore = rgbOfIco(v & 0x1F);
    s.back = rgbOfIco((v >> 5) & 0x1F);
    s.pattern = v >> 10;
    return s;
}

Shd shdNew(const Span &a)
{
    Shd s;
    s.stated = true;
    s.fore = colorOfRef(a.u32(0));
    s.back = colorOfRef(a.u32(4));
    s.pattern = a.u16(8);
    return s;
}

} // namespace

QString hexColor(qint32 rgb)
{
    if (rgb < 0) return QStringLiteral("auto");
    return QString::asprintf("%06X", unsigned(rgb) & 0xFFFFFF);
}

QString colorOfIndex(int ico) { return hexColor(rgbOfIco(ico)); }

quint8 settleToggle(quint8 stated, quint8 parent)
{
    if (stated == 0x80) return parent;
    if (stated == 0x81) return parent ? 0 : 1;
    return stated ? 1 : 0;
}

// ---- characters

void Chp::apply(const Span &g)
{
    forEachSprm(g, [&](const Sprm &s) {
        const Span &a = s.arg;
        auto toggle = [&](Toggle t) { tog[t] = a.u8(0); };
        switch (s.op) {
        case 0x0800: deleted = a.u8(0) & 1; break;
        case 0x0801: inserted = a.u8(0) & 1; break;
        case 0x4804: if (inserted) revisionAuthor = a.u16(0); break;
        case 0x4863: if (deleted) revisionAuthor = a.u16(0); break;
        case 0x6A03: picture = qint32(a.u32(0)); break;
        case 0x0806: data = a.u8(0) & 1; break;
        case 0x0855: special = a.u8(0) & 1; break;
        case 0x0856: object = a.u8(0) & 1; break;
        case 0x080A: ole2 = a.u8(0) & 1; break;
        case 0x6A09: symbolFont = a.u16(0); symbolChar = a.u16(2); break;
        case 0x4A30: istd = a.u16(0); break;
        case 0x2A33: {   // plain: back to what the style has
            const bool sp = special, da = data, ob = object, ol = ole2;
            const qint32 pic = picture;
            const int sf = symbolFont, sc = symbolChar;
            *this = Chp();
            special = sp; data = da; object = ob; ole2 = ol; picture = pic; symbolFont = sf; symbolChar = sc;
            break;
        }
        case 0x0835: toggle(TogBold); break;
        case 0x0836: toggle(TogItalic); break;
        case 0x0837: toggle(TogStrike); break;
        case 0x2A53: toggle(TogDStrike); break;
        case 0x0838: toggle(TogOutline); break;
        case 0x0839: toggle(TogShadow); break;
        case 0x0858: toggle(TogEmboss); break;
        case 0x0854: toggle(TogImprint); break;
        case 0x083A: toggle(TogSmallCaps); break;
        case 0x083B: toggle(TogCaps); break;
        case 0x083C: toggle(TogVanish); break;
        case 0x085C: toggle(TogBoldBi); break;
        case 0x085D: toggle(TogItalicBi); break;
        case 0x4A4F: ftcAscii = a.u16(0); break;
        case 0x4A50: ftcFE = a.u16(0); break;
        case 0x4A51: ftcOther = a.u16(0); break;
        case 0x4A5E: ftcBi = a.u16(0); break;
        case 0x4A43: hps = a.u16(0); break;
        case 0x4A61: hpsBi = a.u16(0); break;
        case 0x4845: hpsPos = a.i16(0); break;
        case 0x8840: dxaSpace = a.i16(0); break;
        case 0x484B: hpsKern = a.u16(0); break;
        case 0x4852: scale = a.u16(0); break;
        case 0x2A3E: kul = a.u8(0); break;
        case 0x2A48: iss = a.u8(0); break;
        case 0x2A0C: highlight = a.u8(0); break;
        case 0x2A42: color = rgbOfIco(a.u8(0)); break;
        case 0x6870: color = colorOfRef(a.u32(0)); break;
        case 0x6877: underlineColor = colorOfRef(a.u32(0)); break;
        case 0x4866: shd = shdOld(a.u16(0)); break;
        case 0xCA71: shd = shdNew(a); break;
        case 0x6865: brc = brcOld(a); break;
        case 0xCA72: brc = brcNew(a); break;
        case 0x0875: noProof = a.u8(0) & 1; break;
        case 0x0811: webHidden = a.u8(0) & 1; break;
        case 0x085A: bidi = a.u8(0) & 1; break;
        case 0x0882: complexScripts = a.u8(0) & 1; break;
        case 0x486D: lid = a.u16(0); break;
        default: break;
        }
    });
}

// ---- paragraphs

void Pap::apply(const Span &g, const Span &data)
{
    forEachSprm(g, [&](const Sprm &s) {
        const Span &a = s.arg;
        if (s.sgc() == 5) return;   // (a row's properties ride on the paragraph that ends the row: applied below)
        switch (s.op) {
        case 0x6646: {   // sprmPHugePapx: the sprms are in the data stream
            const qint64 at = a.u32(0);
            const Span huge = data.sub(at + 2, data.u16(at));
            if (!huge.isEmpty() && data.fits(at, 2)) apply(huge, Span());
            break;
        }
        case 0x4600: istd = a.u16(0); break;
        case 0x2461: case 0x2403: jc = a.u8(0); break;
        case 0x2405: keep = a.u8(0) & 1; break;
        case 0x2406: keepNext = a.u8(0) & 1; break;
        case 0x2407: pageBreakBefore = a.u8(0) & 1; break;
        case 0x2431: widow = a.u8(0) & 1; break;
        case 0x245B: beforeAuto = a.u8(0) & 1; break;
        case 0x245C: afterAuto = a.u8(0) & 1; break;
        case 0x240C: noLineNumber = a.u8(0) & 1; break;
        case 0x242A: noAutoHyph = a.u8(0) & 1; break;
        case 0x2441: bidi = a.u8(0) & 1; break;
        case 0x246D: contextual = a.u8(0) & 1; break;
        case 0x845E: case 0x840F: dxaLeft = a.i16(0); break;
        case 0x8460: case 0x8411: dxaLeft1 = a.i16(0); break;
        case 0x845D: case 0x840E: dxaRight = a.i16(0); break;
        case 0x6412: dyaLine = a.i16(0); multiLine = a.u16(2) == 1; break;
        case 0xA413: dyaBefore = a.u16(0); break;
        case 0xA414: dyaAfter = a.u16(0); break;
        case 0x460B: ilfo = a.i16(0); break;
        case 0x260A: ilvl = a.u8(0); break;
        case 0x2640: outline = a.u8(0); break;
        case 0x2416: inTable = a.u8(0) & 1; break;
        case 0x2417: ttp = a.u8(0) & 1; break;
        case 0x6649: itap = int(std::clamp<qint32>(a.i32(0), 0, 63)); break;
        case 0x244B: innerCell = a.u8(0) & 1; break;
        case 0x244C: innerTtp = a.u8(0) & 1; break;
        case 0x442D: shd = shdOld(a.u16(0)); break;
        case 0xC64D: shd = shdNew(a); break;
        case 0x6424: brcTop = brcOld(a); break;
        case 0x6425: brcLeft = brcOld(a); break;
        case 0x6426: brcBottom = brcOld(a); break;
        case 0x6427: brcRight = brcOld(a); break;
        case 0x6428: brcBetween = brcOld(a); break;
        case 0x6629: brcBar = brcOld(a); break;
        case 0xC64E: brcTop = brcNew(a); break;
        case 0xC64F: brcLeft = brcNew(a); break;
        case 0xC650: brcBottom = brcNew(a); break;
        case 0xC651: brcRight = brcNew(a); break;
        case 0xC652: brcBetween = brcNew(a); break;
        case 0xC653: brcBar = brcNew(a); break;
        case 0x261B: frame.stated = true; frame.anchors = a.u8(0); break;
        case 0x8418: frame.stated = true; frame.xStated = true; frame.x = a.i16(0); break;
        case 0x8419: frame.stated = true; frame.yStated = true; frame.y = a.i16(0); break;
        case 0x841A: frame.stated = true; frame.w = a.u16(0); break;
        case 0x442B: frame.stated = true; frame.hStated = true; frame.h = a.u16(0); break;
        case 0x2423: frame.stated = true; frame.wrap = a.u8(0); break;
        case 0x842F: frame.distX = a.u16(0); break;
        case 0x842E: frame.distY = a.u16(0); break;
        case 0xC60D: case 0xC615: {
            // Tabs deleted, then tabs added (each with a kind and a leader): the changes to the style's tabs.
            const bool withClose = s.op == 0xC615;
            const int del = a.u8(0);
            const qint64 addAt = 1 + (withClose ? 4 : 2) * qint64(del);
            const int add = a.u8(addAt);
            tabsStated = true;
            for (int i = 0; i < del; ++i) {
                const int pos = a.i16(1 + 2 * qint64(i));
                const auto it = std::find_if(tabsAdded.begin(), tabsAdded.end(), [&](const TabStop &t) { return t.pos == pos; });
                if (it != tabsAdded.end()) tabsAdded.erase(it);
                else if (!tabsCleared.contains(pos) && tabsCleared.size() < 128) tabsCleared.push_back(pos);
            }
            for (int i = 0; i < add && tabsAdded.size() < 128; ++i) {
                TabStop t;
                t.pos = a.i16(addAt + 1 + 2 * qint64(i));
                const int tbd = a.u8(addAt + 1 + 2 * qint64(add) + i);
                t.jc = tbd & 7;
                t.leader = (tbd >> 3) & 7;
                const auto it = std::find_if(tabsAdded.begin(), tabsAdded.end(), [&](const TabStop &o) { return o.pos == t.pos; });
                if (it != tabsAdded.end()) *it = t;
                else tabsAdded.push_back(t);
            }
            break;
        }
        default: break;
        }
    });
    // The table sprms, all in one pass over the group.
    bool any = false;
    forEachSprm(g, [&](const Sprm &s) { any = any || s.sgc() == 5; });
    if (any) tap.apply(g);
}

// ---- table rows

void Tap::apply(const Span &g)
{
    auto cellsIn = [&](const Span &a, int from, const std::function<void(Tc &)> &f) {
        const int first = a.u8(from), lim = std::min<int>(a.u8(from + 1), int(cells.size()));
        for (int i = first; i < lim; ++i) f(cells[i]);
    };
    forEachSprm(g, [&](const Sprm &s) {
        if (s.sgc() != 5) return;
        const Span &a = s.arg;
        switch (s.op) {
        case 0x5400: case 0x548A: jc = a.u16(0); break;
        case 0x9601: {
            if (!center.isEmpty()) {
                const int delta = a.i16(0) - center[0];
                for (int &c : center) c += delta;
            }
            break;
        }
        case 0x9602: gapHalf = a.i16(0); break;
        case 0x3403: cantSplit = a.u8(0) & 1; break;
        case 0x3404: header = a.u8(0) & 1; break;
        case 0x9407: rowHeight = a.i16(0); break;
        case 0x563A: istd = a.u16(0); break;
        case 0x560B: bidi = a.u8(0) & 1; break;
        case 0x3615: autofit = a.u8(0) & 1; break;
        case 0xF614: tableWidthType = a.u8(0); tableWidth = a.u16(1); break;
        case 0xF661: if (a.u8(0) == 3) indent = a.i16(1); break;
        case 0xF617: widthBefore = a.u8(0) == 3 ? a.u16(1) : 0; break;
        case 0xF618: widthAfter = a.u8(0) == 3 ? a.u16(1) : 0; break;
        case 0xD605:
            top = brcOld(a.sub(0, 4)); left = brcOld(a.sub(4, 4)); bottom = brcOld(a.sub(8, 4)); right = brcOld(a.sub(12, 4));
            insideH = brcOld(a.sub(16, 4)); insideV = brcOld(a.sub(20, 4));
            break;
        case 0xD613:
            top = brcNew(a.sub(0, 8)); left = brcNew(a.sub(8, 8)); bottom = brcNew(a.sub(16, 8)); right = brcNew(a.sub(24, 8));
            insideH = brcNew(a.sub(32, 8)); insideV = brcNew(a.sub(40, 8));
            break;
        case 0xD608: {
            stated = true;
            const int n = std::min<int>(a.u8(0), kMaxCells);
            center.clear();
            for (int i = 0; i <= n; ++i) center.push_back(a.i16(1 + 2 * qint64(i)));
            const qint64 tcAt = 1 + 2 * qint64(n + 1);
            cells = QVector<Tc>(n);
            for (int i = 0; i < n; ++i) {
                const Span t = a.sub(tcAt + 20 * qint64(i), 20);
                if (t.size() < 20) continue;
                Tc &c = cells[i];
                const int grf = t.u16(0);
                c.firstMerged = grf & 1;
                c.merged = grf & 2;
                c.textFlow = (grf >> 2) & 7;
                c.vertMerge = grf & 0x20;
                c.vertRestart = grf & 0x40;
                c.vAlign = (grf >> 7) & 3;
                c.fitText = grf & 0x1000;
                c.noWrap = grf & 0x2000;
                c.hideMark = grf & 0x4000;
                c.width = t.u16(2);
                c.widthType = (grf >> 9) & 7;
                c.top = brcOld(t.sub(4, 4));
                c.left = brcOld(t.sub(8, 4));
                c.bottom = brcOld(t.sub(12, 4));
                c.right = brcOld(t.sub(16, 4));
                // (An all-zero border is no border stated.)
                for (Brc *b : {&c.top, &c.left, &c.bottom, &c.right})
                    if (!b->nil && b->width == 0 && b->type == 0 && b->color == -1 && b->space == 0) b->stated = false;
            }
            break;
        }
        case 0xD609: {
            for (int i = 0; i < a.size() / 2 && i < cells.size(); ++i) cells[i].shd = shdOld(a.u16(2 * qint64(i)));
            break;
        }
        case 0xD612: case 0xD66F: case 0xD670: {
            // The shading of each cell, with colors of their own (the first twenty-one, the next, and a copy of the first).
            const int base = s.op == 0xD66F ? 21 : 0;
            for (int i = 0; i < a.size() / 10 && base + i < cells.size(); ++i) cells[base + i].shd = shdNew(a.sub(10 * qint64(i), 10));
            break;
        }
        case 0x7627: case 0x7628: cellsIn(a, 0, [&](Tc &c) { c.shd = shdOld(a.u16(2)); }); break;
        case 0xD62F: cellsIn(a, 0, [&](Tc &c) { c.shd = shdNew(a.sub(2, 10)); }); break;
        case 0xD620: case 0xD62D: {
            const int bits = a.u8(2);
            const Brc b = s.op == 0xD620 ? brcOld(a.sub(3, 4)) : brcNew(a.sub(3, 8));
            cellsIn(a, 0, [&](Tc &c) {
                if (bits & 1) c.top = b;
                if (bits & 2) c.left = b;
                if (bits & 4) c.bottom = b;
                if (bits & 8) c.right = b;
            });
            break;
        }
        case 0xD62B: {
            const int f = a.u8(1);
            const int i = a.u8(0);
            if (i < cells.size()) {
                cells[i].vertMerge = f & 1;
                cells[i].vertRestart = f & 2;
                if (cells[i].vertRestart) cells[i].vertMerge = true;
            }
            break;
        }
        case 0xD62C: cellsIn(a, 0, [&](Tc &c) { c.vAlign = a.u8(2) & 3; }); break;
        case 0x5624: {
            const int first = a.u8(0), lim = std::min<int>(a.u8(1), int(cells.size()));
            for (int i = first; i < lim; ++i) {
                cells[i].firstMerged = i == first;
                cells[i].merged = i != first;
            }
            break;
        }
        case 0xF636: {
            const int type = a.u8(2), w = a.u16(3);
            cellsIn(a, 0, [&](Tc &c) { c.width = w; c.widthType = type; });
            break;
        }
        case 0xD633: {
            // The space between the cells of the table (the same for each).
            const int bits = a.u8(2);
            if (bits && a.u8(3) == 3) spacing = a.u16(4);
            break;
        }
        case 0xD639: cellsIn(a, 0, [&](Tc &c) { c.noWrap = a.u8(2) != 0; }); break;
        case 0xD632: case 0xD634: {
            // Space inside the cells' edges: which edges (1 top, 2 left, 4 bottom, 8 right), the unit (3 is twips), and the amount.
            const int bits = a.u8(2), w = a.u8(3) == 3 ? a.u16(4) : -1;
            if (s.op == 0xD634) {
                if (bits & 1) padTop = w;
                if (bits & 2) padLeft = w;
                if (bits & 4) padBottom = w;
                if (bits & 8) padRight = w;
            } else {
                cellsIn(a, 0, [&](Tc &c) {
                    if (bits & 1) c.padTop = w;
                    if (bits & 2) c.padLeft = w;
                    if (bits & 4) c.padBottom = w;
                    if (bits & 8) c.padRight = w;
                });
            }
            break;
        }
        case 0x3616: floating = true; anchors = a.u8(0); break;
        case 0x940E: floating = true; xAbs = a.i16(0); break;
        case 0x940F: floating = true; yAbs = a.i16(0); break;
        case 0x9410: fromLeft = a.u16(0); break;
        case 0x9411: fromTop = a.u16(0); break;
        case 0x941E: fromRight = a.u16(0); break;
        case 0x941F: fromBottom = a.u16(0); break;
        case 0x3465: noOverlap = a.u8(0) & 1; break;
        default: break;
        }
    });
}

bool Tap::sameTable(const Tap &o) const
{
    return istd == o.istd && tableWidthType == o.tableWidthType && tableWidth == o.tableWidth && autofit == o.autofit && indent == o.indent && jc == o.jc &&
           spacing == o.spacing && top == o.top && left == o.left && bottom == o.bottom && right == o.right && insideH == o.insideH &&
           insideV == o.insideV && floating == o.floating &&
           (!floating || (anchors == o.anchors && xAbs == o.xAbs && yAbs == o.yAbs)) && bidi == o.bidi;
}

// ---- sections

void Sep::apply(const Span &g)
{
    forEachSprm(g, [&](const Sprm &s) {
        const Span &a = s.arg;
        stated = true;
        switch (s.op) {
        case 0x3009: type = a.u8(0); break;
        case 0x300A: titlePage = a.u8(0) & 1; break;
        case 0x500B: columns = std::clamp(int(a.u16(0)) + 1, 1, 45); break;
        case 0x900C: colSpace = a.i16(0); break;
        case 0x3005: evenlySpaced = a.u8(0) & 1; break;
        case 0x3019: lineBetween = a.u8(0) & 1; break;
        case 0x301D: landscape = a.u8(0) == 2; break;
        case 0xB01F: pageW = a.u16(0); break;
        case 0xB020: pageH = a.u16(0); break;
        case 0xB021: left = a.u16(0); break;
        case 0xB022: right = a.u16(0); break;
        case 0x9023: top = a.i16(0); break;
        case 0x9024: bottom = a.i16(0); break;
        case 0xB025: gutter = a.u16(0); break;
        case 0xB017: header = a.u16(0); break;
        case 0xB018: footer = a.u16(0); break;
        case 0x300E: numberFormat = a.u8(0); break;
        case 0x501C: pageStart = a.u16(0); break;
        case 0x7044: pageStart = int(a.u32(0)); break;
        case 0x3011: restart = a.u8(0) & 1; break;
        case 0x301A: vAlign = a.u8(0); break;
        case 0x3228: bidi = a.u8(0) & 1; break;
        case 0x303C: footnoteNumbering = a.u8(0); break;
        case 0x303E: endnoteNumbering = a.u8(0); break;
        case 0x5041: footnoteFormat = a.u16(0); break;
        case 0x5042: endnoteFormat = a.u16(0); break;
        case 0x503F: footnoteStart = a.u16(0); break;
        case 0x5040: endnoteStart = a.u16(0); break;
        case 0xF203: case 0xF204: {
            const int i = a.u8(0);
            if (i >= 0 && i < 45) {
                QVector<int> &v = s.op == 0xF203 ? colWidth : colGap;
                if (v.size() <= i) v.resize(i + 1);
                v[i] = a.u16(1);
            }
            break;
        }
        case 0x5015: lineNumberModulus = a.u16(0); hasLineNumbers = true; break;
        case 0x3013: lineNumbering = a.u8(0); hasLineNumbers = true; break;
        case 0x9016: lineNumberDistance = a.u16(0); break;
        case 0x501B: lineNumberStart = a.u16(0) + 1; break;
        case 0x522F: pageBorderProps = a.u16(0); break;
        case 0xD234: pageBorder[0] = brcNew(a); break;
        case 0xD235: pageBorder[1] = brcNew(a); break;
        case 0xD236: pageBorder[2] = brcNew(a); break;
        case 0xD237: pageBorder[3] = brcNew(a); break;
        default: break;
        }
    });
}

} // namespace jp::office::doc
