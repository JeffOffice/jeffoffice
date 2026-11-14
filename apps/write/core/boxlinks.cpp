#include "core/boxlinks.h"

#include "wml/xmltree.h"

#include <QHash>

using namespace jp::office::wml;

namespace jp::write {

namespace {

const QByteArray &declared()
{
    static const QByteArray d =
        "<x xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
        "xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\" "
        "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
        "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" "
        "xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\" xmlns:wpg=\"http://schemas.microsoft.com/office/word/2010/wordprocessingGroup\" "
        "xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\" xmlns:wp14=\"http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing\">";
    return d;
}

// The box element of the drawing's own shape: the child of the first shape (wps:wsp) that is the graphic data's own, not one of a group's members or of a box's words.
struct Own {
    qsizetype begin = -1, end = -1;
    QByteArray prefix;
    bool member = false;
    BoxChain chain;
};

Own ownBox(const QByteArray &drawing)
{
    Own o;
    if (!drawing.contains("xbx")) return o;
    XmlTree t;
    if (!t.parse(declared() + drawing + "</x>", nullptr)) return o;
    // The first graphic data, and its shape.
    int shape = -1;
    std::vector<int> stack{t.root()};
    for (int steps = 0; !stack.empty() && steps < 2000 && shape < 0; ++steps) {
        const int n = stack.back();
        stack.pop_back();
        if (t.localName(n) == "graphicData") {
            for (int c = t.firstChild(n); c >= 0; c = t.next(c))
                if (t.localName(c) == "wsp") {
                    shape = c;
                    break;
                }
            break;
        }
        std::vector<int> kids;
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) kids.push_back(c);
        for (auto it = kids.rbegin(); it != kids.rend(); ++it) stack.push_back(*it);
    }
    if (shape < 0) return o;
    for (int c = t.firstChild(shape); c >= 0; c = t.next(c)) {
        const QByteArray name = t.localName(c);
        if (name != "txbx" && name != "linkedTxbx") continue;
        // Where it is in the drawing: its first tag (the box's own comes before any in its words), taken from its "<" to its ">".
        const qsizetype colon = drawing.indexOf(":" + name);
        const qsizetype at = colon > 0 ? drawing.lastIndexOf('<', colon) : -1;
        const qsizetype gt = at >= 0 ? drawing.indexOf('>', at) : -1;
        if (at < 0 || gt < 0) return o;
        const QByteArray tag = drawing.mid(at, gt + 1 - at);
        o.begin = at;
        o.prefix = tag.mid(1, tag.indexOf(':') - 1);
        if (name == "linkedTxbx" || tag.endsWith("/>")) {
            o.end = at + tag.size();
        } else {
            // The matching end tag, past any box of the same kind in the words.
            const QByteArray open = "<" + o.prefix + ":txbx", close = "</" + o.prefix + ":txbx>";
            qsizetype depth = 1, from = at + tag.size();
            o.end = -1;
            for (int steps = 0; steps < 100000 && depth > 0; ++steps) {
                const qsizetype nextOpen = drawing.indexOf(open, from), nextClose = drawing.indexOf(close, from);
                if (nextClose < 0) break;
                if (nextOpen >= 0 && nextOpen < nextClose) {
                    ++depth;
                    from = nextOpen + open.size();
                } else {
                    --depth;
                    from = nextClose + close.size();
                    if (depth == 0) o.end = from;
                }
            }
            if (o.end < 0) return o;
        }
        o.member = name == "linkedTxbx";
        o.chain.member = o.member;
        if (t.has(c, "id")) o.chain.number = int(std::clamp<qint64>(t.attr(c, "id").toLongLong(), 0, 2000000000));
        if (o.member) o.chain.seq = int(std::clamp<qint64>(t.attr(c, "seq").toLongLong(), 0, 2000000000));
        return o;
    }
    return o;
}

} // namespace

BoxChain boxChainOf(const QByteArray &drawing) { return ownBox(drawing).chain; }

QByteArray boxAsMember(const QByteArray &drawing, int number, int seq)
{
    const Own o = ownBox(drawing);
    if (o.begin < 0) return drawing;
    return drawing.left(o.begin) + "<" + o.prefix + ":linkedTxbx id=\"" + QByteArray::number(number) + "\" seq=\"" + QByteArray::number(seq) + "\"/>" + drawing.mid(o.end);
}

QByteArray boxAsPlain(const QByteArray &drawing)
{
    const Own o = ownBox(drawing);
    if (o.begin < 0 || !o.member) return drawing;
    return drawing.left(o.begin) + "<" + o.prefix + ":txbx><w:txbxContent><w:p/></w:txbxContent></" + o.prefix + ":txbx>" + drawing.mid(o.end);
}

QByteArray boxWithNumber(const QByteArray &drawing, int number)
{
    const Own o = ownBox(drawing);
    if (o.begin < 0 || o.member) return drawing;
    const qsizetype gt = drawing.indexOf('>', o.begin);
    if (gt < 0 || gt > o.end) return drawing;
    // The tag with any id it has taken out, then the new one put in.
    QByteArray tag = drawing.mid(o.begin, gt - o.begin);
    const bool selfClosed = tag.endsWith('/');
    if (selfClosed) tag.chop(1);
    const qsizetype at = tag.indexOf(" id=\"");
    if (at >= 0) {
        const qsizetype stop = tag.indexOf('"', at + 5);
        if (stop > 0) tag.remove(at, stop + 1 - at);
    }
    if (number >= 0) tag += " id=\"" + QByteArray::number(number) + "\"";
    if (selfClosed) tag += "/";
    return drawing.left(o.begin) + tag + drawing.mid(gt);
}

int freeChainNumber(const QSet<int> &used)
{
    for (int n = 0; n <= kMostChainNumber; ++n)
        if (!used.contains(n)) return n;
    return -1;
}

bool repairBoxChains(const QVector<QByteArray *> &drawings)
{
    QVector<BoxChain> chains;
    QSet<int> firsts;
    for (const QByteArray *d : drawings) {
        chains.push_back(boxChainOf(*d));
        if (!chains.last().member && chains.last().number >= 0) firsts.insert(chains.last().number);
    }
    bool changed = false;
    for (int i = 0; i < drawings.size(); ++i)
        if (chains[i].member && !firsts.contains(chains[i].number)) {
            *drawings[i] = boxAsPlain(*drawings[i]);
            changed = true;
        }
    return changed;
}

bool renumberPastedBoxChains(const QVector<QByteArray *> &pasted, const QSet<int> &usedElsewhere)
{
    QHash<int, int> mapped;   // an old number of a first box that was pasted -> the number it has now (-1: none was free)
    QSet<int> used = usedElsewhere;
    QVector<BoxChain> chains;
    for (const QByteArray *d : pasted) chains.push_back(boxChainOf(*d));
    bool changed = false;
    for (int i = 0; i < pasted.size(); ++i) {
        if (chains[i].member || chains[i].number < 0 || mapped.contains(chains[i].number)) continue;
        const int fresh = freeChainNumber(used);
        if (fresh >= 0) used.insert(fresh);
        mapped.insert(chains[i].number, fresh);
    }
    for (int i = 0; i < pasted.size(); ++i) {
        if (chains[i].number < 0) continue;
        const int now = mapped.value(chains[i].number, -2);
        if (chains[i].member) {
            *pasted[i] = now >= 0 ? boxAsMember(*pasted[i], now, chains[i].seq) : boxAsPlain(*pasted[i]);
        } else {
            *pasted[i] = boxWithNumber(*pasted[i], now);
        }
        changed = true;
    }
    return changed;
}

} // namespace jp::write
