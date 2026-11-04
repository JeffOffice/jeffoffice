#include "core/watermarkops.h"

#include "core/writeprops.h"
#include "text/textprops.h"
#include "wml/xmltree.h"

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

// (The name a shape of this kind has: how the headers' shapes that are watermarks are told from a logo.)
const char kMark[] = "PowerPlusWaterMarkObject";

bool isWatermark(const QTextCharFormat &f)
{
    if (!f.hasProperty(prop::Atom)) return false;
    return unpackAtom(storedBytes(f, prop::Atom)).raw.contains(kMark);
}

// Where the watermarks are in a story: the positions of their characters.
QVector<int> marksIn(QTextDocument *story)
{
    QVector<int> at;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            if (!isWatermark(fr.charFormat())) continue;
            for (int i = 0; i < fr.length(); ++i) at << fr.position() + i;
        }
    return at;
}

int removeFrom(QTextDocument *story)
{
    const QVector<int> at = marksIn(story);
    if (at.isEmpty()) return 0;
    QTextCursor c(story);
    c.beginEditBlock();
    for (int i = int(at.size()) - 1; i >= 0; --i) {
        c.setPosition(at[i]);
        c.setPosition(at[i] + 1, QTextCursor::KeepAnchor);
        c.removeSelectedText();
    }
    c.endEditBlock();
    return 1;
}

// The headers of the document the program can edit, each once: a section's own, its first page's when it has one, and its even pages' when the document has them.
QVector<QTextDocument *> headersOf(WriteDoc &doc, bool make)
{
    QVector<QTextDocument *> out;
    QSet<QTextDocument *> seen;
    for (int s = 0; s < doc.sectionCount(); ++s)
        for (int slot = 0; slot < 3; ++slot) {
            if (slot == WriteDoc::kFirstSlot && !doc.differentFirstPage(s)) continue;
            if (slot == WriteDoc::kEvenSlot && !doc.differentOddEven()) continue;
            if (make) {
                if (!doc.createHeaderFooter(s, false, slot)) continue;
            } else if (!doc.headerFooterStory(s, false, slot)) {
                continue;
            }
            QTextDocument *story = doc.editableHeaderFooter(s, false, slot);
            if (!story || seen.contains(story)) continue;
            seen.insert(story);
            out << story;
        }
    return out;
}

QByteArray colorWord(const QColor &c) { return "#" + c.name(QColor::HexRgb).mid(1).toLatin1(); }

} // namespace

QStringList builtInWatermarks()
{
    return {QCoreApplication::translate("jp::WriteWindow", "CONFIDENTIAL"), QCoreApplication::translate("jp::WriteWindow", "DO NOT COPY"), QCoreApplication::translate("jp::WriteWindow", "DRAFT"),
            QCoreApplication::translate("jp::WriteWindow", "SAMPLE"), QCoreApplication::translate("jp::WriteWindow", "ASAP"), QCoreApplication::translate("jp::WriteWindow", "URGENT")};
}

QByteArray watermarkXml(const WatermarkSetup &setup, int number)
{
    const QString words = setup.text.left(kMostWatermarkText);
    const QString face = setup.font.isEmpty() ? QStringLiteral("Arial") : setup.font.left(80);
    // The shape's size, in points: the words are fitted to it, so its height is the letters' (about one and a half times a letter's width). With a size given, the letters are that tall;
    // else the words are as wide as the text of a page, or as wide as they can be at 58 points a letter.
    const double letters = std::max<double>(1, words.size());
    const double width = setup.size > 0 ? std::clamp(letters * setup.size * 0.7, 20.0, 1500.0) : std::clamp(letters * 58.0, 120.0, 460.0);
    const double shownHeight = setup.size > 0 ? std::clamp(setup.size, 1.0, 1000.0) : std::clamp(width / letters * 1.43, 20.0, 200.0);
    QByteArray x = "<w:pict xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\">"
                   "<v:shapetype id=\"_x0000_t136\" coordsize=\"21600,21600\" o:spt=\"136\" adj=\"10800\" path=\"m@7,l@8,m@5,21600l@6,21600e\">"
                   "<v:formulas><v:f eqn=\"sum #0 0 10800\"/><v:f eqn=\"prod #0 2 1\"/><v:f eqn=\"sum 21600 0 @1\"/><v:f eqn=\"sum 0 0 @2\"/><v:f eqn=\"sum 21600 0 @3\"/>"
                   "<v:f eqn=\"if @0 @3 0\"/><v:f eqn=\"if @0 21600 @1\"/><v:f eqn=\"if @0 0 @2\"/><v:f eqn=\"if @0 @4 21600\"/><v:f eqn=\"mid @5 @6\"/><v:f eqn=\"mid @8 @5\"/>"
                   "<v:f eqn=\"mid @7 @8\"/><v:f eqn=\"mid @6 @7\"/><v:f eqn=\"sum @6 0 @5\"/></v:formulas>"
                   "<v:path textpathok=\"t\" o:connecttype=\"custom\" o:connectlocs=\"@9,0;@10,10800;@11,21600;@12,10800\" o:connectangles=\"270,180,90,0\"/>"
                   "<v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h position=\"#0,bottomRight\" xrange=\"6629,14971\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" shapetype=\"t\"/></v:shapetype>";
    x += "<v:shape id=\"" + QByteArray(kMark) + QByteArray::number(number) + "\" o:spid=\"_x0000_s" + QByteArray::number(2048 + number) + "\" type=\"#_x0000_t136\" style=\"position:absolute;margin-left:0;margin-top:0;width:" +
         QByteArray::number(width, 'f', 1) + "pt;height:" + QByteArray::number(shownHeight, 'f', 1) + "pt;" + (setup.diagonal ? "rotation:315;" : "") +
         "z-index:-251658752;mso-position-horizontal:center;mso-position-horizontal-relative:margin;mso-position-vertical:center;mso-position-vertical-relative:margin\" o:allowincell=\"f\" fillcolor=\"" +
         colorWord(setup.color) + "\" stroked=\"f\">";
    x += setup.semitransparent ? "<v:fill opacity=\".5\"/>" : "<v:fill/>";
    x += "<v:textpath style=\"font-family:&quot;" + escapeXml(face, true) + "&quot;;font-size:1pt\" string=\"" + escapeXml(words, true) + "\"/><w10:wrap anchorx=\"margin\" anchory=\"margin\"/></v:shape></w:pict>";
    return x;
}

int insertWatermark(WriteDoc &doc, const WatermarkSetup &setup)
{
    if (setup.text.trimmed().isEmpty()) return 0;
    const WriteDoc::StepGroup together(doc);   // (the headers are stories of their own: one undo takes the watermark from all of them)
    removeWatermark(doc);
    int n = 0;
    const QVector<QTextDocument *> stories = headersOf(doc, true);
    for (int i = 0; i < stories.size(); ++i) {
        QTextDocument *story = stories[i];
        Inline atom;
        atom.kind = Inline::RunAtom;
        atom.tag = QStringLiteral("pict");
        atom.raw = watermarkXml(setup, i + 1);
        QTextCursor c(story);
        c.setPosition(0);
        QTextCharFormat look = c.charFormat();
        look.clearProperty(prop::Atom);
        look.clearProperty(tp::Field);
        look.clearProperty(tp::LinkResult);
        setStored(look, prop::Atom, packAtom(atom));
        look.setProperty(tp::Field, QStringLiteral("raw:"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), look);
        ++n;
    }
    return n;
}

int removeWatermark(WriteDoc &doc)
{
    const WriteDoc::StepGroup together(doc);
    int n = 0;
    for (QTextDocument *story : headersOf(doc, false)) n += removeFrom(story);
    return n;
}

std::optional<WatermarkSetup> watermarkOf(WriteDoc &doc)
{
    static const QRegularExpression string(QStringLiteral("<v:textpath\\b[^>]*?\\bstring=\"([^\"]*)\"")), family(QStringLiteral("font-family:&quot;(.*?)&quot;")),
        fill(QStringLiteral("\\bfillcolor=\"(#[0-9A-Fa-f]{6})\"")), opacity(QStringLiteral("<v:fill\\b[^>]*\\bopacity="));
    for (QTextDocument *story : headersOf(doc, false)) {
        const QVector<int> at = marksIn(story);
        if (at.isEmpty()) continue;
        QTextCursor c(story);
        c.setPosition(at.first() + 1);
        const QString raw = QString::fromUtf8(unpackAtom(storedBytes(c.charFormat(), prop::Atom)).raw);
        WatermarkSetup w;
        const QRegularExpressionMatch s = string.match(raw);
        if (!s.hasMatch()) continue;
        w.text = s.captured(1);
        w.text.replace(QLatin1String("&lt;"), QLatin1String("<")).replace(QLatin1String("&gt;"), QLatin1String(">")).replace(QLatin1String("&quot;"), QLatin1String("\"")).replace(QLatin1String("&apos;"), QLatin1String("'")).replace(QLatin1String("&amp;"), QLatin1String("&"));
        if (const auto f = family.match(raw); f.hasMatch()) w.font = f.captured(1);
        if (const auto f = fill.match(raw); f.hasMatch()) w.color = QColor(f.captured(1));
        w.semitransparent = opacity.match(raw).hasMatch();
        w.diagonal = raw.contains(QLatin1String("rotation:315"));
        return w;
    }
    return std::nullopt;
}

} // namespace jp::write
