#include "core/pagenumberops.h"

#include "core/editops.h"
#include "core/fieldops.h"
#include "core/writeprops.h"

#include "flow/paginator.h"
#include "text/textprops.h"
#include "wml/wml.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QSet>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <algorithm>
#include <cmath>

using namespace jp::office::wml;

namespace jp::write {

namespace {

// What the gallery puts in is marked as the file format marks gallery content: a content control (w:sdt) whose w:docPartObj names the gallery ("Page Numbers (Top of Page)"). Remove Page
// Numbers takes out what is marked so, whoever made it, and nothing else: a number or a box the person made is theirs.
const char kWatermarkMark[] = "PowerPlusWaterMarkObject";
const char kBoxName[] = "Page Number";
const char kShapeName[] = "Page Number Shape";

constexpr qint64 kEmuPerPoint = 12700;

int s_failAt = 0, s_operations = 0;   // (for tests: the operation of a design that fails)

qint64 emu(double points) { return qint64(std::llround(points * kEmuPerPoint)); }

QByteArray n(qint64 v) { return QByteArray::number(v); }

QString galleryName(const PageNumberDesign &d)
{
    switch (d.area) {
    case PageNumberArea::Top: return QStringLiteral("Page Numbers (Top of Page)");
    case PageNumberArea::Bottom: return QStringLiteral("Page Numbers (Bottom of Page)");
    case PageNumberArea::Margin: return QStringLiteral("Page Numbers (Margins)");
    case PageNumberArea::Cursor: return QStringLiteral("Page Numbers (Current Position)");
    }
    return {};
}

// The gallery a content control's start names, or nothing for a control that is not a page numbers one. (Compiled once.)
QString galleryOfControl(const QByteArray &raw)
{
    static const QRegularExpression gallery(QStringLiteral("docPartGallery\\b[^>]*?\\bval=\"(Page Numbers \\([^\"]*\\))\""));
    if (!raw.contains("docPartGallery")) return {};
    const QRegularExpressionMatch m = gallery.match(QString::fromUtf8(raw));
    return m.hasMatch() ? m.captured(1) : QString();
}

Inline controlAtom(const QString &gallery, bool open)
{
    Inline a;
    a.kind = Inline::ParaAtom;
    a.role = open ? Inline::Open : Inline::Close;
    a.tag = QStringLiteral("sdt");
    a.raw = open ? "<w:sdt><w:sdtPr><w:docPartObj><w:docPartGallery w:val=\"" + gallery.toUtf8() + "\"/><w:docPartUnique/></w:docPartObj></w:sdtPr><w:sdtContent>" : QByteArray("</w:sdtContent></w:sdt>");
    return a;
}

bool isWatermarkRaw(const QByteArray &raw) { return raw.contains(kWatermarkMark); }

// The raw XML of the atom a fragment holds, once for the fragment (empty for none).
Inline atomOf(const QTextFragment &fr, bool *has)
{
    *has = fr.charFormat().hasProperty(prop::Atom);
    return *has ? unpackAtom(storedBytes(fr.charFormat(), prop::Atom)) : Inline();
}

// What is marked in a story: runs of its paragraphs (from the control's start to its end, as positions [from, to)), and whole paragraphs (the first and last block numbers) that a control
// of the paragraphs' level (kept before and after them) marks. `gallery`, when given, says only the controls of that gallery.
struct Marks {
    QVector<QPair<int, int>> runs;
    QVector<QPair<int, int>> blocks;
    bool any() const { return !runs.isEmpty() || !blocks.isEmpty(); }
};

Marks marksIn(QTextDocument *story, const QString &gallery = QString(), bool stopAtFirst = false)
{
    Marks out;
    int depth = 0, start = -1;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        // The controls of the paragraphs' level, in the order they come: before the paragraph, then (after its content) after it.
        auto level = [&](const QByteArray &packed, bool after) {
            for (const Block &blk : unpackBlocks(packed)) {
                if (blk.tag != QLatin1String("sdt")) continue;
                if (blk.role == Inline::Open) {
                    if (depth == 0) {
                        const QString g = galleryOfControl(blk.raw);
                        if (!g.isEmpty() && (gallery.isEmpty() || g == gallery)) {
                            start = b.blockNumber() + (after ? 1 : 0);
                            depth = 1;
                        }
                    } else {
                        ++depth;
                    }
                } else if (blk.role == Inline::Close && depth > 0) {
                    if (--depth == 0) {
                        out.blocks << qMakePair(start, b.blockNumber() - (after ? 0 : 1));
                        start = -1;
                    }
                }
            }
        };
        const QTextBlockFormat bf = b.blockFormat();
        if (bf.hasProperty(prop::RawBefore)) level(storedBytes(bf, prop::RawBefore), false);
        if (stopAtFirst && out.any()) return out;
        // The controls inside the paragraph.
        int inner = 0, from = -1;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            bool has = false;
            const Inline atom = atomOf(fr, &has);
            if (!has || atom.kind != Inline::ParaAtom || atom.tag != QLatin1String("sdt")) continue;
            for (int i = 0; i < fr.length(); ++i) {
                const int at = fr.position() + i;
                if (atom.role == Inline::Open) {
                    if (inner == 0) {
                        const QString g = galleryOfControl(atom.raw);
                        if (!g.isEmpty() && (gallery.isEmpty() || g == gallery)) {
                            from = at;
                            inner = 1;
                        }
                    } else {
                        ++inner;
                    }
                } else if (atom.role == Inline::Close && inner > 0) {
                    if (--inner == 0) out.runs << qMakePair(from, at + 1);
                }
            }
        }
        if (stopAtFirst && out.any()) return out;
        if (bf.hasProperty(prop::RawAfter)) level(storedBytes(bf, prop::RawAfter), true);
        if (stopAtFirst && out.any()) return out;
    }
    return out;
}

// Whether a story holds anything but marked page numbers, spaces, and a watermark: what a design put in with Replace would take away.
bool holdsOtherContent(QTextDocument *story)
{
    const Marks marks = marksIn(story);
    QSet<int> inRuns;
    for (const auto &r : marks.runs)
        for (int i = r.first; i < r.second; ++i) inRuns.insert(i);
    QSet<int> inBlocks;
    for (const auto &r : marks.blocks)
        for (int i = r.first; i <= r.second; ++i) inBlocks.insert(i);
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        if (inBlocks.contains(b.blockNumber())) continue;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            bool has = false;
            const Inline atom = atomOf(fr, &has);
            for (int i = 0; i < fr.length(); ++i) {
                if (inRuns.contains(fr.position() + i)) continue;
                const QChar ch = fr.text().at(i);
                if (ch.isSpace()) continue;
                if (ch == QChar::ObjectReplacementCharacter && has && isWatermarkRaw(atom.raw)) continue;
                return true;
            }
        }
    }
    return false;
}

// Whether a part of a file, as read, mentions the gallery's controls (a quick look, before the part is made editable to be sure).
bool mentionsMarks(const QByteArray &source) { return source.contains("docPartGallery") && source.contains("Page Numbers ("); }

// Whether a header or footer holds marked page numbers, without making it editable where the file's part says there are none.
bool storyHasMarks(WriteDoc &doc, QTextDocument *story)
{
    QByteArray source;
    if (doc.headerFooterAsRead(story, &source)) return mentionsMarks(source);
    return marksIn(story, QString(), true).any();
}

QByteArray runLook(const PageNumberType &type, const QColor &white)
{
    QByteArray rPr;
    if (type.bold) rPr += "<w:b/><w:bCs/>";
    if (type.onAccent) rPr += "<w:color w:val=\"" + white.name(QColor::HexRgb).mid(1).toUpper().toLatin1() + "\"/>";
    if (type.size > 0) rPr += "<w:sz w:val=\"" + n(qint64(std::llround(type.size * 2))) + "\"/><w:szCs w:val=\"" + n(qint64(std::llround(type.size * 2))) + "\"/>";
    return rPr.isEmpty() ? QByteArray() : "<w:rPr>" + rPr + "</w:rPr>";
}

// The paragraph of a design's number in a box: its fields in runs of the look, centered.
QByteArray numberParagraph(const PageNumberDesign &d, int sizeOverride)
{
    PageNumberType type = pageNumberType(d);
    if (sizeOverride > 0) type.size = sizeOverride;
    const QByteArray rPr = runLook(type, Qt::white);
    QByteArray p = "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/><w:jc w:val=\"center\"/></w:pPr>";
    for (const PageNumberPiece &piece : pageNumberPieces(d)) {
        if (piece.kind == PageNumberPiece::Text) {
            p += "<w:r>" + rPr + "<w:t xml:space=\"preserve\">" + piece.text.toHtmlEscaped().toUtf8() + "</w:t></w:r>";
            continue;
        }
        const QByteArray code = piece.kind == PageNumberPiece::Page ? " PAGE " : " NUMPAGES ";
        p += "<w:r>" + rPr + "<w:fldChar w:fldCharType=\"begin\"/></w:r><w:r>" + rPr + "<w:instrText xml:space=\"preserve\">" + code + "</w:instrText></w:r><w:r>" + rPr +
             "<w:fldChar w:fldCharType=\"separate\"/></w:r><w:r>" + rPr + "<w:t>1</w:t></w:r><w:r>" + rPr + "<w:fldChar w:fldCharType=\"end\"/></w:r>";
    }
    return p + "</w:p>";
}

// How a box is set on the page: from the page, or the margin, or the strip of margin it is in, by an alignment, so that a section with another page size or later margins still lines it up.
struct Place {
    const char *hFrom, *hAlign, *vFrom, *vAlign;
};

Place placeOf(const PageNumberDesign &d)
{
    auto side = [](PageNumberSide s) { return s == PageNumberSide::Left ? "left" : s == PageNumberSide::Right ? "right" : "center"; };
    if (d.area == PageNumberArea::Margin) {
        const bool left = d.side == PageNumberSide::Left;
        return {left ? "leftMargin" : "rightMargin", "center", "margin", "center"};
    }
    const char *v = d.area == PageNumberArea::Top ? "topMargin" : "bottomMargin";
    if (d.look == PageNumberLook::Tab) return {"page", side(d.side), v, "center"};
    return {"margin", d.look == PageNumberLook::Bar ? "left" : side(d.side), v, "center"};
}

// A drawing anchored by an alignment: a shape of the drawing format of this size (points), turned a quarter where `turned` (the frame is laid on its side, so that the box shows as it was
// asked and its text reads upward), filled with the accent where `fill`, with `content` (a text box's paragraphs) when it has any.
QByteArray anchoredDrawing(const QString &name, int id, const QString &preset, double width, double height, bool turned, bool fill, const QByteArray &accentFill, const QByteArray &content, const Place &place,
                           int order)
{
    double fw = width, fh = height;
    if (turned) std::swap(fw, fh);
    const QByteArray fillXml = fill ? "<a:solidFill>" + accentFill + "</a:solidFill>" : QByteArray("<a:noFill/>");
    QByteArray x = "<w:drawing xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                   "xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\" xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" "
                   "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">";
    x += "<wp:anchor distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\" simplePos=\"0\" relativeHeight=\"" + n(251659264 + order) + "\" behindDoc=\"0\" locked=\"0\" layoutInCell=\"1\" allowOverlap=\"1\">";
    x += "<wp:simplePos x=\"0\" y=\"0\"/><wp:positionH relativeFrom=\"" + QByteArray(place.hFrom) + "\"><wp:align>" + place.hAlign + "</wp:align></wp:positionH>";
    x += "<wp:positionV relativeFrom=\"" + QByteArray(place.vFrom) + "\"><wp:align>" + place.vAlign + "</wp:align></wp:positionV>";
    x += "<wp:extent cx=\"" + n(emu(fw)) + "\" cy=\"" + n(emu(fh)) + "\"/><wp:effectExtent l=\"0\" t=\"0\" r=\"0\" b=\"0\"/><wp:wrapNone/>";
    x += "<wp:docPr id=\"" + n(id) + "\" name=\"" + name.toUtf8() + "\"/><wp:cNvGraphicFramePr/>";
    x += "<a:graphic><a:graphicData uri=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\"><wps:wsp>";
    x += content.isEmpty() ? QByteArray("<wps:cNvSpPr/>") : QByteArray("<wps:cNvSpPr txBox=\"1\"/>");
    x += "<wps:spPr><a:xfrm" + QByteArray(turned ? " rot=\"16200000\"" : "") + "><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" + n(emu(fw)) + "\" cy=\"" + n(emu(fh)) + "\"/></a:xfrm><a:prstGeom prst=\"" +
         preset.toLatin1() + "\"><a:avLst/></a:prstGeom>" + fillXml + "<a:ln><a:noFill/></a:ln></wps:spPr>";
    if (!content.isEmpty()) x += "<wps:txbx><w:txbxContent>" + content + "</w:txbxContent></wps:txbx>";
    x += "<wps:bodyPr rot=\"0\" vert=\"horz\" wrap=\"none\" lIns=\"0\" tIns=\"0\" rIns=\"0\" bIns=\"0\" anchor=\"ctr\" anchorCtr=\"0\"><a:noAutofit/></wps:bodyPr></wps:wsp></a:graphicData></a:graphic></wp:anchor></w:drawing>";
    return x;
}

QByteArray accentFillXml(const WriteDoc &doc)
{
    if (doc.theme()) return "<a:schemeClr val=\"accent1\"/>";
    return "<a:srgbClr val=\"" + pageNumberAccent(doc).name(QColor::HexRgb).mid(1).toUpper().toLatin1() + "\"/>";
}

// One step of a design: false where a test makes it fail.
bool operation() { return !(s_failAt > 0 && ++s_operations == s_failAt); }

// Puts a drawing at the cursor of a story that is a header or footer (or the text): the way a drawing of the text is put, without the checks that keep a picture out of a header.
bool putDrawing(WriteDoc &doc, QTextCursor &c, const QByteArray &raw)
{
    if (!operation()) return false;
    Inline atom;
    atom.kind = Inline::RunAtom;
    atom.tag = QStringLiteral("drawing");
    atom.raw = raw;
    QTextCharFormat f = c.blockCharFormat();
    f.clearProperty(flow::fp::AnchorId);
    f.clearProperty(tp::InlineObject);
    f.clearProperty(tp::Field);
    f.clearProperty(tp::LinkResult);
    setStored(f, prop::Atom, packAtom(atom));
    if (!doc.makeDrawnFormat(atom, &f)) return false;
    c.insertText(QString(QChar::ObjectReplacementCharacter), f);
    doc.keepObjectsForTheHistory();
    return true;
}

// The mark that begins or ends a design: a content control's start or end, a character of the paragraph that shows nothing.
bool putControl(QTextCursor &c, const QString &gallery, bool open)
{
    if (!operation()) return false;
    QTextCharFormat f = c.blockCharFormat();
    f.clearProperty(tp::Field);
    f.clearProperty(tp::LinkResult);
    setStored(f, prop::Atom, packAtom(controlAtom(gallery, open)));
    f.setProperty(tp::Field, QStringLiteral("raw:"));
    c.insertText(QString(QChar::ObjectReplacementCharacter), f);
    return true;
}

// The paragraph's own look for a design: its alignment and its rule.
void setParagraphLook(QTextCursor &c, const PageNumberDesign &d)
{
    QTextBlockFormat bf = c.blockFormat();
    bf.setAlignment(d.side == PageNumberSide::Left ? Qt::AlignLeft : d.side == PageNumberSide::Right ? Qt::AlignRight : Qt::AlignHCenter);
    bf.clearProperty(tp::Borders);
    bf.clearProperty(prop::BoxOwn);
    const PageNumberRule rule = pageNumberRule(d);
    if (rule != PageNumberRule::None) {
        tp::ParaBox box;
        tp::BoxSide line;
        line.style = QStringLiteral("single");
        line.eighths = 4;
        line.space = rule == PageNumberRule::Above ? 4 : 3;
        (rule == PageNumberRule::Above ? box.top : box.bottom) = line;
        setBoxOn(bf, box, rule == PageNumberRule::Above ? boxpart::Top : boxpart::Bottom);
    }
    c.setBlockFormat(bf);
}

// The fields and words of a design at the cursor, in the look the design has.
bool putPieces(WriteDoc &doc, QTextCursor &c, const PageNumberDesign &d)
{
    const PageNumberType type = pageNumberType(d);
    for (const PageNumberPiece &piece : pageNumberPieces(d)) {
        if (!operation()) return false;
        const int from = c.position();
        if (piece.kind == PageNumberPiece::Text) insertText(doc, c, piece.text);
        else if (!insertField(doc, c, piece.kind == PageNumberPiece::Page ? QStringLiteral("PAGE") : QStringLiteral("NUMPAGES"))) return false;
        if (type.bold || type.size > 0) {
            QTextCursor look(c.document());
            look.setPosition(from);
            look.setPosition(c.position(), QTextCursor::KeepAnchor);
            QTextCharFormat f;
            if (type.bold) f.setFontWeight(QFont::Bold);
            if (type.size > 0) f.setFontPointSize(type.size);
            look.mergeCharFormat(f);
        }
    }
    return true;
}

// The drawings a boxed design is: its shape behind (a circle or a rounded box) and the box with the number over it.
bool putBoxes(WriteDoc &doc, QTextDocument *story, QTextCursor &c, const PageNumberDesign &d, const QSizeF &page, const QMarginsF &margins)
{
    const PageNumberBox box = pageNumberBox(d, page, margins);
    if (box.preset.isEmpty()) return false;
    const QByteArray accent = accentFillXml(doc);
    const bool ownShape = box.preset != QLatin1String("rect");
    const QByteArray words = numberParagraph(d, 0);
    const Place place = placeOf(d);
    if (ownShape && !putDrawing(doc, c, anchoredDrawing(QString::fromLatin1(kShapeName), doc.nextDrawingId(story), box.preset, box.rect.width(), box.rect.height(), box.turned, true, accent, QByteArray(), place, 0)))
        return false;
    return putDrawing(doc, c, anchoredDrawing(QString::fromLatin1(kBoxName), doc.nextDrawingId(story), QStringLiteral("rect"), box.rect.width(), box.rect.height(), box.turned, !ownShape, accent, words, place, 1));
}

// A design's box set inline at the cursor (a circled number in a line of text): the shape and the number in a text box on top of it, both in line, as one piece.
bool putInlineCircle(WriteDoc &doc, QTextCursor &c, const PageNumberDesign &d)
{
    const QByteArray accent = accentFillXml(doc);
    const QByteArray words = numberParagraph(d, 9);
    const qint64 side = emu(22);
    QByteArray x = "<w:drawing xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                   "xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\" xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" "
                   "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">";
    x += "<wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"><wp:extent cx=\"" + n(side) + "\" cy=\"" + n(side) + "\"/><wp:effectExtent l=\"0\" t=\"0\" r=\"0\" b=\"0\"/>";
    x += "<wp:docPr id=\"" + n(doc.nextDrawingId(c.document())) + "\" name=\"" + QByteArray(kBoxName) + "\"/><wp:cNvGraphicFramePr/>";
    x += "<a:graphic><a:graphicData uri=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\"><wps:wsp><wps:cNvSpPr txBox=\"1\"/>";
    x += "<wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" + n(side) + "\" cy=\"" + n(side) + "\"/></a:xfrm><a:prstGeom prst=\"ellipse\"><a:avLst/></a:prstGeom><a:solidFill>" + accent +
         "</a:solidFill><a:ln><a:noFill/></a:ln></wps:spPr><wps:txbx><w:txbxContent>" + words + "</w:txbxContent></wps:txbx>";
    x += "<wps:bodyPr rot=\"0\" vert=\"horz\" wrap=\"none\" lIns=\"0\" tIns=\"0\" rIns=\"0\" bIns=\"0\" anchor=\"ctr\" anchorCtr=\"0\"><a:noAutofit/></wps:bodyPr></wps:wsp></a:graphicData></a:graphic></wp:inline></w:drawing>";
    return putDrawing(doc, c, x);
}

// The page a section is laid out on, and its margins, in points.
void pageOf(const WriteDoc &doc, int section, QSizeF *page, QMarginsF *margins)
{
    const SectionProps sp = doc.sectionProps(section);
    *page = QSizeF(sp.pageW / 20.0, sp.pageH / 20.0);
    *margins = QMarginsF(sp.left / 20.0, sp.top / 20.0, sp.right / 20.0, sp.bottom / 20.0);
    if (page->width() < 72 || page->height() < 72) *page = QSizeF(612, 792);
}

// Deletes the marked page numbers of a story (those of the gallery, when one is named): the runs, and the paragraphs that a control of their level marks; a paragraph the runs leave
// empty goes too. How many marks went.
int removeMarks(QTextDocument *story, const QString &gallery = QString())
{
    int count = 0;
    {
        const Marks marks = marksIn(story, gallery);
        QSet<int> blocks;
        QTextCursor c(story);
        c.beginEditBlock();
        for (int i = int(marks.runs.size()) - 1; i >= 0; --i) {
            blocks.insert(story->findBlock(marks.runs[i].first).blockNumber());
            c.setPosition(marks.runs[i].first);
            c.setPosition(marks.runs[i].second, QTextCursor::KeepAnchor);
            c.removeSelectedText();
            ++count;
        }
        // The paragraphs the runs left empty (and the story has others).
        QVector<int> empty(blocks.cbegin(), blocks.cend());
        std::sort(empty.begin(), empty.end(), std::greater<int>());
        for (const int number : std::as_const(empty)) {
            const QTextBlock b = story->findBlockByNumber(number);
            if (!b.isValid() || !b.text().isEmpty() || story->blockCount() < 2) continue;
            if (b.blockFormat().hasProperty(prop::RawBefore) || b.blockFormat().hasProperty(prop::RawAfter)) continue;
            if (b.next().isValid()) {
                c.setPosition(b.position());
                c.setPosition(b.next().position(), QTextCursor::KeepAnchor);
            } else {
                c.setPosition(b.previous().position() + b.previous().length() - 1);
                c.setPosition(b.position() + b.length() - 1, QTextCursor::KeepAnchor);
            }
            c.removeSelectedText();
        }
        c.endEditBlock();
    }
    const Marks marks = marksIn(story, gallery);
    QTextCursor c(story);
    c.beginEditBlock();
    for (int i = int(marks.blocks.size()) - 1; i >= 0; --i) {
        const QTextBlock first = story->findBlockByNumber(marks.blocks[i].first), last = story->findBlockByNumber(marks.blocks[i].second);
        if (!first.isValid() || !last.isValid() || marks.blocks[i].second < marks.blocks[i].first) continue;
        if (last.next().isValid()) {
            c.setPosition(first.position());
            c.setPosition(last.next().position(), QTextCursor::KeepAnchor);
        } else if (first.previous().isValid()) {
            c.setPosition(first.previous().position() + first.previous().length() - 1);
            c.setPosition(last.position() + last.length() - 1, QTextCursor::KeepAnchor);
        } else {
            // (All of the story: one empty paragraph is left, with nothing of the marked ones.)
            c.setPosition(first.position());
            c.setPosition(last.position() + last.length() - 1, QTextCursor::KeepAnchor);
            c.removeSelectedText();
            QTextBlockFormat bf = story->begin().blockFormat();
            bf.clearProperty(prop::RawBefore);
            bf.clearProperty(prop::RawAfter);
            bf.clearProperty(tp::Borders);
            bf.clearProperty(prop::BoxOwn);
            QTextCursor one(story);
            one.setBlockFormat(bf);
            ++count;
            continue;
        }
        c.removeSelectedText();
        ++count;
    }
    c.endEditBlock();
    return count;
}

// Takes out everything of a story but what is marked as page numbers (of another gallery: the design's own gallery was taken out already) and a watermark, so that a number at the top and
// one in the margin can both be there, and the person's words are what is replaced. At least one paragraph is left.
void wipeUnmarked(QTextDocument *story)
{
    const Marks marks = marksIn(story);
    QSet<int> keepChars, keepBlocks;
    for (const auto &r : marks.runs)
        for (int i = r.first; i < r.second; ++i) keepChars.insert(i);
    for (const auto &r : marks.blocks)
        for (int i = r.first; i <= r.second; ++i) keepBlocks.insert(i);
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            bool has = false;
            const Inline atom = atomOf(it.fragment(), &has);
            if (has && isWatermarkRaw(atom.raw))
                for (int i = 0; i < it.fragment().length(); ++i) keepChars.insert(it.fragment().position() + i);
        }
    QTextCursor c(story);
    c.beginEditBlock();
    for (int pos = story->characterCount() - 2; pos >= 0; --pos) {
        if (keepChars.contains(pos) || keepBlocks.contains(story->findBlock(pos).blockNumber())) continue;
        const QChar ch = story->characterAt(pos);
        if (ch == QChar::ParagraphSeparator || ch == QLatin1Char('\n')) continue;
        c.setPosition(pos);
        c.setPosition(pos + 1, QTextCursor::KeepAnchor);
        c.removeSelectedText();
    }
    // The paragraphs that nothing is left in (and that no marked block holds) go, but one.
    for (QTextBlock b = story->lastBlock(); b.isValid() && story->blockCount() > 1;) {
        const QTextBlock before = b.previous();
        if (b.text().isEmpty() && !keepBlocks.contains(b.blockNumber()) && !b.blockFormat().hasProperty(prop::RawBefore) && !b.blockFormat().hasProperty(prop::RawAfter)) {
            if (b.next().isValid()) {
                c.setPosition(b.position());
                c.setPosition(b.next().position(), QTextCursor::KeepAnchor);
            } else {
                c.setPosition(b.previous().position() + b.previous().length() - 1);
                c.setPosition(b.position() + b.length() - 1, QTextCursor::KeepAnchor);
            }
            c.removeSelectedText();
        }
        b = before;
    }
    // (What a design's paragraph had of its own look does not stay on the paragraph that is left.)
    QTextBlock first = story->begin();
    if (first.isValid() && first.text().isEmpty() && !keepBlocks.contains(first.blockNumber())) {
        QTextBlockFormat bf = first.blockFormat();
        bf.clearProperty(tp::Borders);
        bf.clearProperty(prop::BoxOwn);
        bf.setAlignment(Qt::AlignLeft);
        QTextCursor one(first);
        one.setBlockFormat(bf);
    }
    c.endEditBlock();
}

} // namespace

void setPageNumberFailureForTest(int operation)
{
    s_failAt = operation;
    s_operations = 0;
}

QColor pageNumberAccent(const WriteDoc &doc)
{
    if (const office::Theme *t = doc.theme(); t && t->colors[office::Theme::Accent1].isValid()) return t->colors[office::Theme::Accent1];
    return QColor(0x2F, 0x55, 0x97);
}

bool pageNumberPlaceOf(WriteDoc &doc, int page, const PageNumberDesign &design, PageNumberPlace *place)
{
    if (design.area == PageNumberArea::Cursor) return false;
    flow::Paginator &pg = doc.pager();
    pg.paginate(page);
    if (page < 0 || page >= pg.pagesKnown()) return false;
    const flow::Paginator::Page p = pg.page(page);
    if (p.start.blank) return false;
    place->section = p.start.section;
    // (The margins' designs are in the header: a box is set from the page's own edges and margins.)
    place->footer = design.area == PageNumberArea::Bottom;
    place->slot = doc.headerSlotOf(place->section, page <= 0 || p.start.firstOfSection, p.number);
    return true;
}

bool pageNumberTargetHoldsText(WriteDoc &doc, const PageNumberPlace &place)
{
    QTextDocument *story = doc.headerFooterStory(place.section, place.footer, place.slot);
    if (!story) return false;
    QByteArray source;
    if (doc.headerFooterAsRead(story, &source) && mentionsMarks(source)) story = doc.editableHeaderFooter(place.section, place.footer, place.slot);   // (its marks are seen in that form)
    return story && holdsOtherContent(story);
}

bool insertPageNumberDesign(WriteDoc &doc, int page, const QString &id, PageNumberText what)
{
    const std::optional<PageNumberDesign> design = pageNumberDesignById(id);
    PageNumberPlace place;
    if (!design || !pageNumberPlaceOf(doc, page, *design, &place)) return false;
    const bool hadSteps = doc.canUndo();
    const quint64 serial = doc.lastStepSerial();
    bool ok = true;
    {
        const WriteDoc::StepGroup group(doc);   // (a header made for the design is part of its step)
        ok = [&] {
            if (!doc.createHeaderFooter(place.section, place.footer, place.slot)) return false;
            QTextDocument *story = doc.editableHeaderFooter(place.section, place.footer, place.slot);
            if (!story) return false;
            QSizeF pageSize;
            QMarginsF margins;
            pageOf(doc, place.section, &pageSize, &margins);
            const QString gallery = galleryName(*design);
            // What an earlier design of this gallery left goes.
            removeMarks(story, gallery);
            if (what == PageNumberText::Replace) wipeUnmarked(story);
            QTextCursor c(story);
            if (pageNumberHasBox(*design)) {
                // (A drawing is anchored to a character of the first paragraph; the paragraph's own text stays as it is.)
                c.movePosition(QTextCursor::Start);
                return putControl(c, gallery, true) && putBoxes(doc, story, c, *design, pageSize, margins) && putControl(c, gallery, false);
            }
            // A paragraph of the design: the last when it holds nothing, else a new one after the text.
            c.movePosition(QTextCursor::End);
            bool blank = true;
            for (QTextBlock::iterator it = c.block().begin(); !it.atEnd() && blank; ++it) {
                bool has = false;
                const Inline atom = atomOf(it.fragment(), &has);
                for (int i = 0; i < it.fragment().length() && blank; ++i)
                    blank = it.fragment().text().at(i).isSpace() || (has && isWatermarkRaw(atom.raw));
            }
            if (!blank) {
                c.insertBlock();
                QTextBlockFormat bf = c.blockFormat();
                bf.clearProperty(tp::Borders);
                bf.clearProperty(prop::BoxOwn);
                bf.clearProperty(prop::RawBefore);
                bf.clearProperty(prop::RawAfter);
                c.setBlockFormat(bf);
            }
            setParagraphLook(c, *design);
            return putControl(c, gallery, true) && putPieces(doc, c, *design) && putControl(c, gallery, false);
        }();
    }
    if (!ok) {
        // All or nothing: what the steps made so far is taken back, and cannot be done again.
        if (doc.canUndo() != hadSteps || doc.lastStepSerial() != serial) {
            doc.undo();
            doc.discardRedo();
        }
        return false;
    }
    return true;
}

bool insertPageNumberAtCursor(WriteDoc &doc, QTextCursor &cursor, const QString &id)
{
    const std::optional<PageNumberDesign> design = pageNumberDesignById(id);
    if (!design || design->area != PageNumberArea::Cursor) return false;
    const bool hadSteps = doc.canUndo();
    const quint64 serial = doc.lastStepSerial();
    QTextCursor c = cursor;
    bool ok = true;
    {
        const WriteDoc::StepGroup group(doc);
        if (c.hasSelection()) c.removeSelectedText();
        const QString gallery = galleryName(*design);
        ok = putControl(c, gallery, true) && (design->look == PageNumberLook::Circle ? putInlineCircle(doc, c, *design) : putPieces(doc, c, *design)) && putControl(c, gallery, false);
    }
    if (!ok) {
        if (doc.canUndo() != hadSteps || doc.lastStepSerial() != serial) {
            doc.undo();
            doc.discardRedo();
        }
        return false;
    }
    cursor = c;
    return true;
}

int s_scans = 0;

int pageNumberScansForTest() { return s_scans; }

bool hasPageNumbers(WriteDoc &doc)
{
    ++s_scans;
    QSet<const QTextDocument *> seen;
    for (int s = 0; s < doc.sectionCount(); ++s)
        for (const bool footer : {false, true})
            for (int slot = 0; slot < 3; ++slot) {
                if (!doc.hasOwnHeaderFooter(s, footer, slot)) continue;
                QTextDocument *story = doc.headerFooterStory(s, footer, slot);
                if (!story || seen.contains(story)) continue;
                seen.insert(story);
                if (storyHasMarks(doc, story)) return true;
            }
    return false;
}

int removePageNumbers(WriteDoc &doc)
{
    const WriteDoc::StepGroup group(doc);
    int count = 0;
    QSet<const QTextDocument *> seen;
    for (int s = 0; s < doc.sectionCount(); ++s)
        for (const bool footer : {false, true})
            for (int slot = 0; slot < 3; ++slot) {
                if (!doc.hasOwnHeaderFooter(s, footer, slot)) continue;
                // (A header with nothing marked is not made editable, so it is left exactly as it was.)
                const QTextDocument *shown = doc.headerFooterStory(s, footer, slot);
                if (!shown || seen.contains(shown)) continue;
                seen.insert(shown);
                if (!storyHasMarks(doc, doc.headerFooterStory(s, footer, slot))) continue;
                QTextDocument *story = doc.editableHeaderFooter(s, footer, slot);
                if (story) count += removeMarks(story);
            }
    return count;
}

} // namespace jp::write
