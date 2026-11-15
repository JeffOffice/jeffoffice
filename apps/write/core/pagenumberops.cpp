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

// The names of the drawings a design makes: the box that holds the number, and (for a circle and a rounded box, which a text box can't be) the shape behind it.
const char kBoxName[] = "Page Number";
const char kShapeName[] = "Page Number Shape";
const char kWatermarkMark[] = "PowerPlusWaterMarkObject";

constexpr qint64 kEmuPerPoint = 12700;

qint64 emu(double points) { return qint64(std::llround(points * kEmuPerPoint)); }

QByteArray n(qint64 v) { return QByteArray::number(v); }

bool isPageField(const FieldPlace &f) { return f.name == QLatin1String("PAGE") || f.name == QLatin1String("NUMPAGES"); }

// The raw run XML a character of a story keeps, empty for a character that keeps none.
QByteArray atomBytesAt(QTextDocument *story, int position)
{
    QTextCursor c(story);
    c.setPosition(position + 1);
    const QTextCharFormat f = c.charFormat();
    return f.hasProperty(prop::Atom) ? unpackAtom(storedBytes(f, prop::Atom)).raw : QByteArray();
}

bool isDesignBox(const QByteArray &raw) { return raw.startsWith("<w:drawing") && (raw.contains(QByteArray("name=\"") + kBoxName + "\"") || raw.contains(QByteArray("name=\"") + kShapeName + "\"")); }
bool isWatermarkAtom(const QByteArray &raw) { return raw.contains(kWatermarkMark); }

// The positions of the drawings of a design in a story.
QVector<int> designBoxesIn(QTextDocument *story)
{
    QVector<int> at;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            if (!fr.charFormat().hasProperty(prop::Atom)) continue;
            if (!isDesignBox(unpackAtom(storedBytes(fr.charFormat(), prop::Atom)).raw)) continue;
            for (int i = 0; i < fr.length(); ++i) at << fr.position() + i;
        }
    return at;
}

// The positions that the PAGE and NUMPAGES fields take: from the first mark to the last, each as a range of [from, to).
QVector<QPair<int, int>> pageFieldsIn(QTextDocument *story)
{
    QVector<QPair<int, int>> out;
    const StoryScan scan = scanStory(story, false);
    for (const FieldPlace &f : scan.fields) {
        if (!isPageField(f) || f.inCode || f.parent >= 0) continue;
        const int from = f.begin, to = (f.form == FieldPlace::Group ? f.begin : f.end) + 1;
        if (from < 0 || to <= from) continue;
        out << qMakePair(from, to);
    }
    return out;
}

// Whether a paragraph holds nothing but spaces and a watermark.
bool blankBlock(QTextDocument *story, const QTextBlock &b)
{
    for (int i = 0; i < b.length() - 1; ++i) {
        const QChar ch = story->characterAt(b.position() + i);
        if (ch == QChar::ObjectReplacementCharacter) {
            if (!isWatermarkAtom(atomBytesAt(story, b.position() + i))) return false;
        } else if (!ch.isSpace()) {
            return false;
        }
    }
    return true;
}

QString pageWords()
{
    return QStringLiteral("(?:") + QRegularExpression::escape(QCoreApplication::translate("jp::PageNumbers", "Page")) + QStringLiteral(")?\\s*(?:") +
           QRegularExpression::escape(QCoreApplication::translate("jp::PageNumbers", "of")) + QStringLiteral(")?");
}

// Whether a paragraph's text, with the fields and drawings taken away, is only the words of a design's "Page X of Y".
bool onlyPageWords(const QString &text)
{
    static const QString objects = QString(QChar::ObjectReplacementCharacter);
    QString t = text;
    t.remove(QChar::ObjectReplacementCharacter);
    t = t.trimmed();
    if (t.isEmpty()) return true;
    const QRegularExpression words(QStringLiteral("^\\s*") + pageWords() + QStringLiteral("\\s*$"), QRegularExpression::CaseInsensitiveOption);
    return words.match(t).hasMatch();
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
        const QByteArray result = piece.kind == PageNumberPiece::Page ? "1" : "1";
        p += "<w:r>" + rPr + "<w:fldChar w:fldCharType=\"begin\"/></w:r><w:r>" + rPr + "<w:instrText xml:space=\"preserve\">" + code + "</w:instrText></w:r><w:r>" + rPr +
             "<w:fldChar w:fldCharType=\"separate\"/></w:r><w:r>" + rPr + "<w:t>" + result + "</w:t></w:r><w:r>" + rPr + "<w:fldChar w:fldCharType=\"end\"/></w:r>";
    }
    return p + "</w:p>";
}

// A drawing anchored to the page: a shape of the drawing format at `rect` (points from the page's corner), turned a quarter where `turned` (the frame is laid on its side, so the box
// shows as `rect` says and its text reads upward), filled with the accent where `fill`, with `content` (a text box's paragraphs) when it has any.
QByteArray anchoredDrawing(const QString &name, int id, const QString &preset, const QRectF &rect, bool turned, bool fill, const QByteArray &accentFill, const QByteArray &content, int order)
{
    double fw = rect.width(), fh = rect.height();
    QPointF origin = rect.topLeft();
    if (turned) {
        std::swap(fw, fh);
        origin = QPointF(rect.center().x() - fw / 2, rect.center().y() - fh / 2);
    }
    const QByteArray fillXml = fill ? "<a:solidFill>" + accentFill + "</a:solidFill>" : QByteArray("<a:noFill/>");
    QByteArray x = "<w:drawing xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                   "xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\" xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" "
                   "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">";
    x += "<wp:anchor distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\" simplePos=\"0\" relativeHeight=\"" + n(251659264 + order) + "\" behindDoc=\"0\" locked=\"0\" layoutInCell=\"1\" allowOverlap=\"1\">";
    x += "<wp:simplePos x=\"0\" y=\"0\"/><wp:positionH relativeFrom=\"page\"><wp:posOffset>" + n(emu(origin.x())) + "</wp:posOffset></wp:positionH>";
    x += "<wp:positionV relativeFrom=\"page\"><wp:posOffset>" + n(emu(origin.y())) + "</wp:posOffset></wp:positionV>";
    x += "<wp:extent cx=\"" + n(emu(fw)) + "\" cy=\"" + n(emu(fh)) + "\"/><wp:effectExtent l=\"0\" t=\"0\" r=\"0\" b=\"0\"/><wp:wrapNone/>";
    x += "<wp:docPr id=\"" + n(id) + "\" name=\"" + name.toUtf8() + "\"/><wp:cNvGraphicFramePr/>";
    x += "<a:graphic><a:graphicData uri=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\"><wps:wsp>";
    x += content.isEmpty() ? QByteArray("<wps:cNvSpPr/>") : QByteArray("<wps:cNvSpPr txBox=\"1\"/>");
    x += "<wps:spPr><a:xfrm" + QByteArray(turned ? " rot=\"16200000\"" : "") + "><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" + n(emu(fw)) + "\" cy=\"" + n(emu(fh)) + "\"/></a:xfrm><a:prstGeom prst=\"" +
         preset.toLatin1() + "\"><a:avLst/></a:prstGeom>" + fillXml + "<a:ln><a:noFill/></a:ln></wps:spPr>";
    if (!content.isEmpty()) x += "<wps:txbx><w:txbxContent>" + content + "</w:txbxContent></wps:txbx>";
    x += "<wps:bodyPr rot=\"0\" vert=\"horz\" wrap=\"square\" lIns=\"0\" tIns=\"0\" rIns=\"0\" bIns=\"0\" anchor=\"ctr\" anchorCtr=\"0\"><a:noAutofit/></wps:bodyPr></wps:wsp></a:graphicData></a:graphic></wp:anchor></w:drawing>";
    return x;
}

QByteArray accentFillXml(const WriteDoc &doc)
{
    if (doc.theme()) return "<a:schemeClr val=\"accent1\"/>";
    return "<a:srgbClr val=\"" + pageNumberAccent(doc).name(QColor::HexRgb).mid(1).toUpper().toLatin1() + "\"/>";
}

// Puts a drawing at the cursor of a story that is a header or footer (or the text): the way a drawing of the text is put, without the checks that keep a picture out of a header.
bool putDrawing(WriteDoc &doc, QTextCursor &c, const QByteArray &raw)
{
    Inline atom;
    atom.kind = Inline::RunAtom;
    atom.tag = QStringLiteral("drawing");
    atom.raw = raw;
    QTextCharFormat f = c.charFormat();
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

// The id of a new drawing in a story, past the ones it has.
int nextId(WriteDoc &doc, QTextDocument *story) { return doc.nextDrawingId(story); }

// The paragraph's own look for a design: its alignment, its rule, and the size and bold of its text.
void setParagraphLook(QTextCursor &c, const PageNumberDesign &d)
{
    QTextBlockFormat bf = c.blockFormat();
    bf.setAlignment(d.side == PageNumberSide::Left ? Qt::AlignLeft : d.side == PageNumberSide::Right ? Qt::AlignRight : Qt::AlignHCenter);
    bf.clearProperty(tp::Borders);
    const PageNumberRule rule = pageNumberRule(d);
    if (rule != PageNumberRule::None) {
        tp::ParaBox box;
        tp::BoxSide line;
        line.style = QStringLiteral("single");
        line.eighths = 4;
        line.space = rule == PageNumberRule::Above ? 4 : 3;
        (rule == PageNumberRule::Above ? box.top : box.bottom) = line;
        bf.setProperty(tp::Borders, box.spec());
    }
    c.setBlockFormat(bf);
}

// The fields and words of a design at the cursor, in the look the design has.
void putPieces(WriteDoc &doc, QTextCursor &c, const PageNumberDesign &d)
{
    const PageNumberType type = pageNumberType(d);
    for (const PageNumberPiece &piece : pageNumberPieces(d)) {
        const int from = c.position();
        if (piece.kind == PageNumberPiece::Text) insertText(doc, c, piece.text);
        else insertField(doc, c, piece.kind == PageNumberPiece::Page ? QStringLiteral("PAGE") : QStringLiteral("NUMPAGES"));
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
}

// The drawings a boxed design is: its shape behind (a circle or a rounded box) and the box with the number over it.
bool putBoxes(WriteDoc &doc, QTextDocument *story, QTextCursor &c, const PageNumberDesign &d, const QSizeF &page, const QMarginsF &margins)
{
    const PageNumberBox box = pageNumberBox(d, page, margins);
    if (box.preset.isEmpty()) return false;
    const QByteArray accent = accentFillXml(doc);
    const bool ownShape = box.preset != QLatin1String("rect");
    const QByteArray words = numberParagraph(d, 0);
    if (ownShape && !putDrawing(doc, c, anchoredDrawing(QString::fromLatin1(kShapeName), nextId(doc, story), box.preset, box.rect, box.turned, true, accent, QByteArray(), 0))) return false;
    return putDrawing(doc, c, anchoredDrawing(QString::fromLatin1(kBoxName), nextId(doc, story), QStringLiteral("rect"), box.rect, box.turned, !ownShape, accent, words, 1));
}

// A design's box set inline at the cursor (a circled number in a line of text): the shape and the number in a text box on top of it, both in line, as one piece.
bool putInlineCircle(WriteDoc &doc, QTextCursor &c, const PageNumberDesign &d)
{
    const QByteArray accent = accentFillXml(doc);
    const QByteArray words = numberParagraph(d, 9);
    const qint64 side = emu(18);
    QByteArray x = "<w:drawing xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                   "xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\" xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" "
                   "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">";
    x += "<wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"><wp:extent cx=\"" + n(side) + "\" cy=\"" + n(side) + "\"/><wp:effectExtent l=\"0\" t=\"0\" r=\"0\" b=\"0\"/>";
    x += "<wp:docPr id=\"" + n(nextId(doc, c.document())) + "\" name=\"" + QByteArray(kBoxName) + "\"/><wp:cNvGraphicFramePr/>";
    x += "<a:graphic><a:graphicData uri=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\"><wps:wsp><wps:cNvSpPr txBox=\"1\"/>";
    x += "<wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" + n(side) + "\" cy=\"" + n(side) + "\"/></a:xfrm><a:prstGeom prst=\"ellipse\"><a:avLst/></a:prstGeom><a:solidFill>" + accent +
         "</a:solidFill><a:ln><a:noFill/></a:ln></wps:spPr><wps:txbx><w:txbxContent>" + words + "</w:txbxContent></wps:txbx>";
    x += "<wps:bodyPr rot=\"0\" vert=\"horz\" wrap=\"square\" lIns=\"0\" tIns=\"0\" rIns=\"0\" bIns=\"0\" anchor=\"ctr\" anchorCtr=\"0\"><a:noAutofit/></wps:bodyPr></wps:wsp></a:graphicData></a:graphic></wp:inline></w:drawing>";
    return putDrawing(doc, c, x);
}

// The page the pages are laid out on, and the margins of a section, in points.
void pageOf(const WriteDoc &doc, int section, QSizeF *page, QMarginsF *margins)
{
    const SectionProps sp = doc.sectionProps(section);
    *page = QSizeF(sp.pageW / 20.0, sp.pageH / 20.0);
    *margins = QMarginsF(sp.left / 20.0, sp.top / 20.0, sp.right / 20.0, sp.bottom / 20.0);
    if (page->width() < 72 || page->height() < 72) *page = QSizeF(612, 792);
}

} // namespace

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
    place->footer = design.area == PageNumberArea::Bottom;
    place->slot = doc.headerSlotOf(place->section, page <= 0 || p.start.firstOfSection, p.number);
    return true;
}

bool pageNumberTargetHoldsText(WriteDoc &doc, const PageNumberPlace &place)
{
    QTextDocument *story = doc.headerFooterStory(place.section, place.footer, place.slot);
    if (!story) return false;
    const QSet<int> fields = [&] {
        QSet<int> s;
        for (const auto &r : pageFieldsIn(story))
            for (int i = r.first; i < r.second; ++i) s.insert(i);
        return s;
    }();
    const QVector<int> boxes = designBoxesIn(story);
    QString text;
    bool other = false;
    for (int i = 0; i < story->characterCount() - 1; ++i) {
        const QChar ch = story->characterAt(i);
        if (fields.contains(i) || boxes.contains(i)) continue;
        if (ch == QChar::ObjectReplacementCharacter) {
            const QByteArray raw = atomBytesAt(story, i);
            if (isWatermarkAtom(raw)) continue;
            // (A mark or the like that a field made: only a drawing or a field of another kind is something else.)
            other = other || !raw.isEmpty();
            continue;
        }
        if (ch == QChar::ParagraphSeparator) {
            text += QLatin1Char('\n');
            continue;
        }
        text += ch;
    }
    for (const QString &line : text.split(QLatin1Char('\n')))
        if (!onlyPageWords(line)) return true;
    return other;
}

bool insertPageNumberDesign(WriteDoc &doc, int page, const QString &id, PageNumberText what)
{
    const std::optional<PageNumberDesign> design = pageNumberDesignById(id);
    PageNumberPlace place;
    if (!design || !pageNumberPlaceOf(doc, page, *design, &place)) return false;
    const WriteDoc::StepGroup group(doc);   // (a header made for the design is part of its step)
    if (!doc.createHeaderFooter(place.section, place.footer, place.slot)) return false;
    QTextDocument *story = doc.editableHeaderFooter(place.section, place.footer, place.slot);
    if (!story) return false;
    QSizeF pageSize;
    QMarginsF margins;
    pageOf(doc, place.section, &pageSize, &margins);
    // What an earlier design left: its boxes go, and (when the text is replaced) so does everything but a watermark.
    {
        const QVector<int> boxes = designBoxesIn(story);
        QTextCursor c(story);
        c.beginEditBlock();
        for (int i = int(boxes.size()) - 1; i >= 0; --i) {
            c.setPosition(boxes[i]);
            c.setPosition(boxes[i] + 1, QTextCursor::KeepAnchor);
            c.removeSelectedText();
        }
        c.endEditBlock();
    }
    const bool margin = design->area == PageNumberArea::Margin;
    if (!margin && what == PageNumberText::Replace) {
        QVector<QTextCharFormat> marks;
        for (int i = 0; i < story->characterCount() - 1; ++i)
            if (story->characterAt(i) == QChar::ObjectReplacementCharacter && isWatermarkAtom(atomBytesAt(story, i))) {
                QTextCursor at(story);
                at.setPosition(i + 1);
                marks << at.charFormat();
            }
        QTextCursor c(story);
        c.select(QTextCursor::Document);
        c.removeSelectedText();
        c.movePosition(QTextCursor::Start);
        QTextBlockFormat bf = c.blockFormat();
        bf.clearProperty(tp::Borders);
        bf.setAlignment(Qt::AlignLeft);
        c.setBlockFormat(bf);
        for (const QTextCharFormat &f : marks) c.insertText(QString(QChar::ObjectReplacementCharacter), f);
    }
    QTextCursor c(story);
    if (pageNumberHasBox(*design)) {
        // (A drawing is anchored to a character of the first paragraph; the paragraph's own text stays as it is.)
        c.movePosition(QTextCursor::Start);
        return putBoxes(doc, story, c, *design, pageSize, margins);
    }
    // A paragraph of the design: the last when it is blank, else a new one after the text.
    c.movePosition(QTextCursor::End);
    if (!blankBlock(story, c.block())) {
        c.insertBlock();
        QTextBlockFormat bf = c.blockFormat();
        bf.clearProperty(tp::Borders);
        c.setBlockFormat(bf);
    }
    setParagraphLook(c, *design);
    putPieces(doc, c, *design);
    return true;
}

bool insertPageNumberAtCursor(WriteDoc &doc, QTextCursor &cursor, const QString &id)
{
    const std::optional<PageNumberDesign> design = pageNumberDesignById(id);
    if (!design || design->area != PageNumberArea::Cursor) return false;
    const WriteDoc::StepGroup group(doc);
    QTextCursor c = cursor;
    if (c.hasSelection()) {
        c.removeSelectedText();
    }
    if (design->look == PageNumberLook::Circle) {
        if (!putInlineCircle(doc, c, *design)) return false;
    } else {
        putPieces(doc, c, *design);
    }
    cursor = c;
    return true;
}

int removePageNumbers(WriteDoc &doc)
{
    const WriteDoc::StepGroup group(doc);
    int count = 0;
    QSet<QTextDocument *> seen;
    for (int s = 0; s < doc.sectionCount(); ++s)
        for (const bool footer : {false, true})
            for (int slot = 0; slot < 3; ++slot) {
                if (!doc.hasOwnHeaderFooter(s, footer, slot)) continue;
                QTextDocument *story = doc.editableHeaderFooter(s, footer, slot);
                if (!story || seen.contains(story)) continue;
                seen.insert(story);
                // The fields and boxes, from the last.
                QVector<QPair<int, int>> ranges = pageFieldsIn(story);
                for (const int at : designBoxesIn(story)) ranges << qMakePair(at, at + 1);
                if (ranges.isEmpty()) continue;
                std::sort(ranges.begin(), ranges.end(), [](const auto &a, const auto &b) { return a.first > b.first; });
                QSet<int> touched;
                for (const auto &r : std::as_const(ranges)) touched.insert(story->findBlock(r.first).blockNumber());
                QTextCursor c(story);
                c.beginEditBlock();
                for (const auto &r : std::as_const(ranges)) {
                    c.setPosition(r.first);
                    c.setPosition(r.second, QTextCursor::KeepAnchor);
                    c.removeSelectedText();
                    ++count;
                }
                // A paragraph that is left with only the words of "Page X of Y" has them taken too, and the rule it had.
                for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
                    if (!touched.contains(b.blockNumber())) continue;
                    QString text = b.text();
                    if (!onlyPageWords(text) || text.remove(QChar::ObjectReplacementCharacter).trimmed().isEmpty()) continue;
                    // (Only the words go, not a drawing of the paragraph's.)
                    for (int i = b.length() - 2; i >= 0; --i) {
                        if (story->characterAt(b.position() + i) == QChar::ObjectReplacementCharacter) continue;
                        QTextCursor one(story);
                        one.setPosition(b.position() + i);
                        one.setPosition(b.position() + i + 1, QTextCursor::KeepAnchor);
                        one.removeSelectedText();
                    }
                    QTextCursor para(story);
                    para.setPosition(b.position());
                    QTextBlockFormat bf = para.blockFormat();
                    bf.clearProperty(tp::Borders);
                    para.setBlockFormat(bf);
                }
                c.endEditBlock();
            }
    return count;
}

} // namespace jp::write
