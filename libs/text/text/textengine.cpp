#include "text/textengine.h"

#include "core/fonts.h"
#include "core/texteffects.h"
#include "text/hyphenation.h"
#include "text/textprops.h"

#include <QCache>
#include <QCoreApplication>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QLinearGradient>
#include <QtMath>
#include <QGlyphRun>
#include <QHash>
#include <QPaintEngine>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>
#include <QRawFont>
#include <QTextDocument>
#include <QTextBoundaryFinder>
#include <QTextDocumentFragment>
#include <QTextList>
#include <atomic>
#include <cmath>
#include <functional>
#include <limits>
#include <cstring>

namespace {
// Whether the application's fonts have a family, remembered until fonts are added or taken away (asking costs a font match, and a word processor's layout asks for each run).
bool hasFamilyCached(const QString &family)
{
    static QHash<QString, bool> known;
    static int generation = -1;
    if (generation != jp::fontGeneration()) {
        known.clear();
        generation = jp::fontGeneration();
    }
    const auto it = known.constFind(family);
    if (it != known.constEnd()) return it.value();
    const bool has = QFontDatabase::hasFamily(family);
    if (known.size() < 4096) known.insert(family, has);
    return has;
}
} // namespace

namespace jp {

// ---------- fields ----------
static QString toRoman(int n)
{
    static const QPair<int, const char *> t[] = {{1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"}, {90, "XC"},
                                                {50, "L"}, {40, "XL"}, {10, "X"}, {9, "IX"}, {5, "V"}, {4, "IV"}, {1, "I"}};
    QString s;
    for (const auto &p : t)
        while (n >= p.first) { s += QLatin1String(p.second); n -= p.first; }
    return s;
}

// A note's number outside layout (exports): its place among the story's
// notes of the same kind.
static QString noteNumber(const FieldSource *doc, const QString &kind, const QString &storyId)
{
    if (!doc) return QStringLiteral("*");
    const QString prefix = kind + QLatin1Char(':');
    for (const QTextDocument *sd : doc->storyDocs()) {
        int n = 0;
        for (QTextBlock b = sd->begin(); b.isValid(); b = b.next())
            for (auto f = b.begin(); !f.atEnd(); ++f) {
                const QString code = f.fragment().charFormat().stringProperty(tp::Field);
                if (!code.startsWith(prefix)) continue;
                for (int i = 0; i < f.fragment().length(); ++i) {
                    ++n;
                    if (code.mid(prefix.size()) == storyId) return QString::number(n);
                }
            }
    }
    return QStringLiteral("*");
}

static QDateTime g_fieldClock;   // the moment a tool set (invalid: none)

QDateTime fieldNow() { return g_fieldClock.isValid() ? g_fieldClock : QDateTime::currentDateTime(); }

void setFieldClock(const QDateTime &moment) { g_fieldClock = moment; }

QString FieldContext::resolve(const QString &code) const
{
    const QString kind = code.section(':', 0, 0);
    const QString arg = code.section(':', 1);
    if (kind == QLatin1String("footnote") || kind == QLatin1String("endnote")) return noteNumber(doc, kind, arg);
    if (kind == "page") {
        // Numbered from the publication's first page number, in its style
        // unless the field names one (Format Page Numbers).
        const int from = doc ? doc->firstPageNumber() - 1 : 0;
        const QString style = arg.isEmpty() && doc ? doc->pageNumberFormat() : arg;
        auto shown = [&](int n) {
            if (style == "roman" || style == "ROMAN") { const QString r = toRoman(n); return style == "roman" ? r.toLower() : r; }
            if (style == "alpha" || style == "ALPHA") {
                // a ... z, then aa ... zz, as the other program counts.
                const QString one(QChar((style == "alpha" ? 'a' : 'A') + (n - 1) % 26));
                return one.repeated((n - 1) / 26 + 1);
            }
            if (style == "dash") return QStringLiteral("- %1 -").arg(n);
            return QString::number(n);
        };
        // The next or previous linked box's page; "#" without one, as in Publisher.
        if (arg == "next") return nextPage > 0 ? QString::number(nextPage + from) : QStringLiteral("#");
        if (arg == "prev") return prevPage > 0 ? QString::number(prevPage + from) : QStringLiteral("#");
        return shown(pageNumber + from);
    }
    if (kind == "notemark") return noteMark;
    if (kind == "pages") {
        const int total = doc ? doc->pageTotal() : 0;
        return QString::number(total > 0 ? total : pageCount);
    }
    if (kind == "sectionpages") {
        const int total = doc ? doc->sectionPageTotal() : 0;
        return QString::number(total > 0 ? total : pageCount);
    }
    if (kind == "date") return now.date().toString(arg.isEmpty() ? QStringLiteral("MMMM d, yyyy") : arg);
    if (kind == "time") return now.time().toString(arg.isEmpty() ? QStringLiteral("h:mm AP") : arg);
    if (kind == "datetime") return now.toString(arg.isEmpty() ? QStringLiteral("M/d/yyyy h:mm AP") : arg);
    if (kind == "biz") {
        // "address:oneline" joins a multi-line value with commas, for use inside a line.
        const bool oneLine = arg.endsWith(QLatin1String(":oneline"));
        const QString key = oneLine ? arg.chopped(8) : arg;
        if (!doc) return QStringLiteral("[%1]").arg(key);
        QString v = doc->businessField(key);
        static const QRegularExpression lineBreak(QStringLiteral("\\s*\\r?\\n\\s*"));
        if (oneLine) v = v.split(lineBreak, Qt::SkipEmptyParts).join(QStringLiteral(", "));
        return v;
    }
    if (kind == "merge") {
        if (!doc || mergeRecord < 0) return QStringLiteral("«%1»").arg(arg);
        return doc->mergeValue(mergeRecord, arg);
    }
    if (kind == "mergeblock") {
        if (!doc || mergeRecord < 0) return arg == "greeting" ? QCoreApplication::translate("Text", "«Greeting Line»") : QCoreApplication::translate("Text", "«Address Block»");
        auto v = [&](const QStringList &names) {
            for (const auto &n : names)
                for (const auto &f : doc->mergeFields())
                    if (f.compare(n, Qt::CaseInsensitive) == 0) {
                        const QString x = doc->mergeValue(mergeRecord, f);
                        if (!x.isEmpty()) return x;
                    }
            return QString();
        };
        const QString first = v({"First Name", "FirstName", "First"}), last = v({"Last Name", "LastName", "Last", "Surname"});
        if (arg == "greeting") {
            const QString who = (first + ' ' + last).trimmed();
            return who.isEmpty() ? QCoreApplication::translate("Text", "Dear Friend,") : QCoreApplication::translate("Text", "Dear %1,").arg(who);
        }
        QStringList lines;
        const QString name = (first + ' ' + last).trimmed().isEmpty() ? v({"Name", "Full Name"}) : (first + ' ' + last).trimmed();
        if (!name.isEmpty()) lines << name;
        const QString co = v({"Company", "Company Name", "Organization"});
        if (!co.isEmpty()) lines << co;
        const QString a1 = v({"Address", "Address Line 1", "Address 1", "Street"}), a2 = v({"Address Line 2", "Address 2"});
        if (!a1.isEmpty()) lines << a1;
        if (!a2.isEmpty()) lines << a2;
        const QString city = v({"City"}), st = v({"State", "ST", "Province"}), zip = v({"ZIP Code", "ZIP", "Postal Code", "Zip"});
        QString cl = city;
        if (!st.isEmpty()) cl += (cl.isEmpty() ? "" : ", ") + st;
        if (!zip.isEmpty()) cl += (cl.isEmpty() ? "" : " ") + zip;
        if (!cl.isEmpty()) lines << cl;
        const QString country = v({"Country", "Country or Region"});
        if (!country.isEmpty()) lines << country;
        return lines.join(QChar(QChar::LineSeparator));
    }
    // Text that a word processing document keeps as XML and shows as it is (a footnote's mark, a stand-in for a table), or nothing.
    if (kind == "raw") return arg;
    if (kind == "conton") return continuedOnPage > 0 ? QCoreApplication::translate("Text", "(Continued on page %1)").arg(continuedOnPage) : QString();
    if (kind == "contfrom") return continuedFromPage > 0 ? QCoreApplication::translate("Text", "(Continued from page %1)").arg(continuedFromPage) : QString();
    return QStringLiteral("[%1]").arg(code);
}

const QStringList &dateTimeFormats()
{
    static const QStringList formats{"M/d/yyyy", "dddd, MMMM d, yyyy", "MMMM d, yyyy", "M/d/yy", "yyyy-MM-dd", "d-MMM-yy", "M.d.yyyy", "MMM. d, yy",
                                     "d MMMM yyyy", "MMMM yy", "MMM-yy", "M/d/yyyy h:mm AP", "M/d/yyyy h:mm:ss AP", "h:mm AP", "h:mm:ss AP", "HH:mm",
                                     "HH:mm:ss"};
    return formats;
}

StoryFields storyFields(const QTextDocument *doc, const FieldSource *source)
{
    StoryFields out;
    QSet<QString> seen;
    auto scan = [&](const QTextDocument *d, bool own) {
        for (QTextBlock b = d->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QString field = it.fragment().charFormat().stringProperty(tp::Field);
                if (field.isEmpty()) continue;
                const QString kind = field.section(QLatin1Char(':'), 0, 0);
                if (kind == QLatin1String("footnote") || kind == QLatin1String("endnote")) {
                    const QString id = field.section(QLatin1Char(':'), 1);
                    if (own && !seen.contains(id)) {
                        seen.insert(id);
                        out.notes << id;
                    }
                } else if (kind == QLatin1String("date") || kind == QLatin1String("time") || kind == QLatin1String("datetime") || kind == QLatin1String("biz") ||
                           kind == QLatin1String("merge") || kind == QLatin1String("mergeblock")) {
                    if (!seen.contains(QLatin1Char('#') + field)) {
                        seen.insert(QLatin1Char('#') + field);
                        out.codes << field;
                    }
                }
            }
    };
    scan(doc, true);
    // (A note's own notes are not laid out, so a note is looked at for its own fields only.)
    if (source)
        for (const QString &id : out.notes)
            if (const QTextDocument *nd = source->storyDoc(id)) scan(nd, false);
    return out;
}

QString fieldValuesKey(const StoryFields &fields, const FieldContext &ctx)
{
    QString k;
    for (const QString &code : fields.codes) k += code + QLatin1Char('=') + ctx.resolve(code) + QLatin1Char('\n');
    return k;
}

quint64 notesKey(const QStringList &notes, const FieldSource *source)
{
    quint64 h = 1469598103934665603ull;
    auto mix = [&](quint64 x) { h = (h ^ x) * 1099511628211ull; };
    for (const QString &id : notes) {
        const QTextDocument *nd = source ? source->storyDoc(id) : nullptr;
        mix(quint64(quintptr(nd)));
        mix(nd ? quint64(nd->revision()) : 0);
    }
    return h;
}

QString FieldContext::key(bool withClock) const
{
    // With Format Page Numbers' settings, which change every page number.
    return QStringLiteral("%1/%2/%3/%4/%5/%6/%7/%8/%9").arg(pageNumber).arg(pageCount).arg(mergeRecord).arg(continuedOnPage).arg(continuedFromPage)
        .arg(nextPage).arg(prevPage).arg(withClock ? now.toString("yyyyMMddhhmm") : QString())
        .arg(doc ? QString::number(doc->firstPageNumber()) + doc->pageNumberFormat() + QLatin1Char('/') + QString::number(doc->pageTotal()) + QLatin1Char('/') + QString::number(doc->sectionPageTotal()) : QString());
}

QString LayoutEnv::key() const
{
    QString k = fonts.heading + '|' + fonts.body + '|' + QString::number(fontScale, 'f', 4) + (showFieldShading ? "|s" : "") + (keepInks() ? "|k" : "") + (slideLines ? "|l" : "") + (wordLines ? "|w" : "") + (wordNoHang ? "|n" : "") + (wordSpaceSums ? "|a" : "") + (wordNoHangStop ? "|h" : "") + (wordNoNumberIndentStop ? "|i" : "") + (wordWholePointLines ? "|u" : "") + (wordNoLeading ? "|z" : "") + (wordSpaceShrink > 0 ? QStringLiteral("|p%1").arg(wordSpaceShrink) : QString()) + (noLigatures ? "|g" : "") + (lineCut > 0 ? QStringLiteral("|c%1").arg(lineCut) : QString());
    if (defaultTab != 36) k += QStringLiteral("|t%1").arg(defaultTab);
    for (auto it = familyAliases.cbegin(); it != familyAliases.cend(); ++it) k += QStringLiteral("|a") + it.key() + QLatin1Char('=') + it.value();
    for (auto it = standIns.cbegin(); it != standIns.cend(); ++it) k += QStringLiteral("|i") + it.key() + QLatin1Char('=') + it.value();
    for (int i = 0; i < SlotCount; ++i) k += colors.c[i].name();
    return k;
}

// ---------- fine layout ----------
// Qt sizes fonts in whole pixels: 13.25 pt text lays out as 13 pt and 13.5 pt
// as 14, up to 4% off for fractional sizes (shrink-to-fit makes them common;
// 1 pixel is 1 point here). So text is laid out at kFine times its size and
// every measure that comes back is divided by it (8 x keeps sizes to within
// 0.1% and positions within Qt's fixed-point range).
namespace {
constexpr double kFine = 8;
// Qt keeps a pixel of the fine layout (an eighth of a point) back from the width a line is given, and the width of a line the room of which it exactly fills is
// to be given back that much and a hair more (the layout's rounding).
constexpr double kQtPixelBack = 0.126;
// How far past its room a word processor's line may run: a pixel of the fine layout short of it, which is where the original program's lines break on the practice documents
// (set to the room exactly, the lines of 37 of them end further from its own, and a page of V032 that matches goes out of step). Every place that fits a line to its room (the
// first try, a line that ends at a dash, the word wider than the line, the space shrinking) uses it, so that they agree. The text before a mark that hangs is the one exception.
constexpr double kRoomOffset = -0.125;
// Where a line is set to end at its room (text that ends at a right tab stop): as wide as it, to what the layout's rounding leaves.
constexpr double kExactFit = 0.13;
// What the space shrinking of justified lines has done, for a test of its cost: characters looked at in finding the lines that could take another word, paragraphs with such a line,
// paragraphs laid out again, paragraphs set from what an earlier layout of them came to, and characters measured in cutting the lines of a paragraph.
static qint64 s_shrinkLooked = 0, s_shrinkPasses = 0, s_shrinkRelaid = 0, s_shrinkShared = 0, s_shrinkFull = 0;
// What the shrinking made of paragraphs, kept for the layouts of the same paragraph that follow (the paginator's again, and the page's that draws it).
static QCache<QString, ShrunkParagraph> &shrinkMemory()
{
    static QCache<QString, ShrunkParagraph> memory(3000);
    return memory;
}
// Layout only: an underline's style while JeffPub draws it (drawUnderline).
constexpr int kOwnUnderline = QTextFormat::UserProperty + 90;
// Layout only: a missing stock font measured letter by letter, "family|bold|italic"
// (applyLetterWidths gives each letter the original's width).
constexpr int kLetterWidths = QTextFormat::UserProperty + 91;
// The face Courier New is drawn in where the machine lacks it (resolveCharFormat): its font's families are this and then the family whose line it keeps.
const QString kThinCourier = QStringLiteral("Courier Prime");

QFont fineFont(QFont f)
{
    f.setPointSizeF(f.pointSizeF() * kFine);
    if (f.letterSpacingType() == QFont::AbsoluteSpacing) f.setLetterSpacing(QFont::AbsoluteSpacing, f.letterSpacing() * kFine);
    f.setWordSpacing(f.wordSpacing() * kFine);
    return f;
}

// The other way: a font from a fine layout, in layout units.
QFont pointFont(QFont f)
{
    f.setPointSizeF(f.pointSizeF() / kFine);
    if (f.letterSpacingType() == QFont::AbsoluteSpacing) f.setLetterSpacing(QFont::AbsoluteSpacing, f.letterSpacing() / kFine);
    f.setWordSpacing(f.wordSpacing() / kFine);
    return f;
}

QTextCharFormat fineFormat(QTextCharFormat cf)
{
    if (cf.hasProperty(QTextFormat::FontPointSize)) cf.setFontPointSize(cf.fontPointSize() * kFine);
    if (cf.hasProperty(QTextFormat::FontLetterSpacing) && cf.fontLetterSpacingType() == QFont::AbsoluteSpacing)
        cf.setFontLetterSpacing(cf.fontLetterSpacing() * kFine);
    if (cf.hasProperty(QTextFormat::FontWordSpacing)) cf.setFontWordSpacing(cf.fontWordSpacing() * kFine);
    if (cf.hasProperty(QTextFormat::TextOutline)) {
        QPen pen = cf.textOutline();
        pen.setWidthF(pen.widthF() * kFine);
        cf.setTextOutline(pen);
    }
    // Underlines Qt can't draw (double, words only, thick) are drawn here
    // (drawUnderline): Qt is told of none, and kOwnUnderline keeps the style.
    if (cf.intProperty(tp::UnderlineKind) && cf.underlineStyle() != QTextCharFormat::NoUnderline) {
        cf.setProperty(kOwnUnderline, int(cf.underlineStyle()));
        cf.setUnderlineStyle(QTextCharFormat::NoUnderline);
    }
    return cf;
}

// An underline under [s, e) of a fine layout's line, in its units: the
// format's own style, or the one fineFormat set aside, with its kind.
void drawUnderline(QPainter *p, const QTextLine &line, int s, int e, const QTextCharFormat &cf, const QBrush &br, const QString &text)
{
    const auto style = cf.hasProperty(kOwnUnderline) ? QTextCharFormat::UnderlineStyle(cf.intProperty(kOwnUnderline)) : cf.underlineStyle();
    if (style == QTextCharFormat::NoUnderline) return;
    const int kind = cf.intProperty(tp::UnderlineKind);
    const QFontMetricsF fm(cf.font());
    QPen up(cf.underlineColor().isValid() ? QBrush(cf.underlineColor()) : br, std::max(1.0, fm.lineWidth()) * ((kind & 4) ? 2 : 1));
    up.setCapStyle(Qt::FlatCap);
    switch (style) {
    case QTextCharFormat::DotLine: up.setStyle(Qt::DotLine); break;
    case QTextCharFormat::DashUnderline: up.setStyle(Qt::DashLine); break;
    case QTextCharFormat::DashDotLine: up.setStyle(Qt::DashDotLine); break;
    case QTextCharFormat::DashDotDotLine: up.setStyle(Qt::DashDotDotLine); break;
    default: break;
    }
    // Words only: under each run of letters, not the spaces between. A span
    // runs from its left end to its right, whichever way the text reads.
    QVector<QPair<double, double>> spans;
    auto span = [&](int from, int to) {
        const double a = line.cursorToX(from), b = line.cursorToX(to);
        return QPair<double, double>(std::min(a, b), std::max(a, b));
    };
    if (kind & 2) {
        int k = s;
        while (k < e) {
            while (k < e && text[k].isSpace()) ++k;
            const int w0 = k;
            while (k < e && !text[k].isSpace()) ++k;
            if (k > w0) spans.append(span(w0, k));
        }
    } else spans.append(span(s, e));
    const double y = line.y() + line.ascent() + fm.underlinePos(), gap = up.widthF() * 2;
    p->save();
    p->setPen(up);
    p->setBrush(Qt::NoBrush);
    for (const auto &sp : spans) {
        if (style == QTextCharFormat::WaveUnderline) {
            // A wave about the underline, a little taller than the line is thick.
            const double amp = std::max(0.75 * kFine, up.widthF()), half = amp * 2;
            QPainterPath wave(QPointF(sp.first, y));
            int n = 0;
            for (double x = sp.first; x < sp.second; x += half, ++n)
                wave.quadTo(QPointF(x + half / 2, y + (n % 2 ? amp : -amp)), QPointF(std::min(x + half, sp.second), y));
            p->drawPath(wave);
            if (kind & 1) p->drawPath(wave.translated(0, amp * 2 + up.widthF()));
            continue;
        }
        p->drawLine(QLineF(sp.first, y, sp.second, y));
        if (kind & 1) p->drawLine(QLineF(sp.first, y + gap, sp.second, y + gap));   // double
    }
    p->restore();
}

// Whether two format ranges draw one underline: the same style, kind, and
// color, and the same font but for the letter spacing applyLetterWidths gives
// each piece of a run.
bool sameUnderline(const QTextCharFormat &a, const QTextCharFormat &b)
{
    auto font = [](const QTextCharFormat &f) {
        QFont x = f.font();
        x.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        return x;
    };
    return a.underlineStyle() == b.underlineStyle() && a.intProperty(kOwnUnderline) == b.intProperty(kOwnUnderline) &&
           a.intProperty(tp::UnderlineKind) == b.intProperty(tp::UnderlineKind) && a.underlineColor() == b.underlineColor() &&
           a.foreground() == b.foreground() && font(a) == font(b);
}

// The part of a line [ls, le) that the underline of fmts[n] is drawn over:
// with the neighboring ranges it is one underline with, so that a dashed
// line's pattern runs on; empty where an earlier range of the line draws it.
QPair<int, int> underlineSpan(const QList<QTextLayout::FormatRange> &fmts, int n, int ls, int le)
{
    auto joined = [&](int a, int b) { return fmts[a].start + fmts[a].length == fmts[b].start && sameUnderline(fmts[a].format, fmts[b].format); };
    if (fmts[n].start >= le || fmts[n].start + fmts[n].length <= ls) return {0, 0};
    if (n > 0 && fmts[n - 1].start + fmts[n - 1].length > ls && joined(n - 1, n)) return {0, 0};
    int e = fmts[n].start + fmts[n].length;
    for (int k = n; k + 1 < fmts.size() && e < le && joined(k, k + 1); ++k) e = fmts[k + 1].start + fmts[k + 1].length;
    return {std::max(fmts[n].start, ls), std::min(e, le)};
}

// Bold drawn by hand (tp::HandBold): the letters of a font with no bold face
// take a stroke in their own color, whose width is what the stems grow by.
// Publisher draws it by where the text goes (measured Oct 11 in Arial Black,
// Rockwell Extra Bold, and Franklin Gothic Demi, 12 to 300 pt; docs/pub-format.md):
// on the screen and in pictures bold Franklin Gothic Demi and Forte grow
// 0.017 em (the system's synthetic bold) and Arial Black and Rockwell Extra
// Bold don't; in a PDF it exports every such letter grows 1/35 em; on paper
// 0.01 em. From 216 pt it sets such text as outlines, as the screen has it.
// Into pixels at 96 to 216 dpi the growth is a whole pixel for letters under
// 30 pixels to the em (10 point text at 96 dpi grows 1 pixel, not 0.2).
double handBoldWidth(int kind, double em, TextOutput out, double pxPerPt)
{
    constexpr double kOutlinesFrom = 216, kScreen = 0.017, kPdf = 1.0 / 35, kPaper = 0.01;
    const bool outlines = em >= kOutlinesFrom;
    if (!outlines && out == TextOutput::Pdf) return em * kPdf;
    if (!outlines && out == TextOutput::Print) return em * kPaper;
    if (kind != 1) return 0;
    if (!outlines && out == TextOutput::Picture && pxPerPt >= 1 && pxPerPt <= 3 && em * pxPerPt < 30) return 1 / pxPerPt;
    return em * kScreen;
}

// The stroke of [s, e) of a fine layout's line, over letters already drawn
// (the painter is in fine units).
void drawHandBold(QPainter *p, const QTextLine &line, int s, int e, const QTextCharFormat &cf, TextOutput out)
{
    if (!cf.hasProperty(tp::HandBold) || cf.foreground().style() != Qt::SolidPattern) return;
    const double em = cf.fontPointSize() / (fontPointFactor() * kFine);
    const double pxPerPt = std::sqrt(std::abs(p->deviceTransform().determinant())) * kFine;
    const double w = handBoldWidth(cf.intProperty(tp::HandBold), em, out, pxPerPt) * kFine;
    if (w <= 0) return;
    QPen pen(cf.foreground(), w);
    pen.setJoinStyle(Qt::MiterJoin);   // the PDF default
    pen.setMiterLimit(10);
    p->save();
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);
    for (const QGlyphRun &g : line.glyphRuns(s, e - s)) {
        const QRawFont raw = g.rawFont();
        const QList<quint32> glyphs = g.glyphIndexes();
        const QList<QPointF> where = g.positions();
        for (int k = 0; k < glyphs.size() && k < where.size(); ++k) {
            QPainterPath path = raw.pathForGlyph(glyphs[k]);
            path.translate(where[k]);
            p->drawPath(path);
        }
    }
    p->restore();
}

QList<QTextLayout::FormatRange> fineRanges(const QVector<QTextLayout::FormatRange> &ranges)
{
    QList<QTextLayout::FormatRange> out;
    out.reserve(ranges.size());
    for (QTextLayout::FormatRange r : ranges) {
        r.format = fineFormat(r.format);
        out << r;
    }
    return out;
}

// Measures of a font in layout units.
double advanceOf(const QFont &f, const QString &text) { return QFontMetricsF(fineFont(f)).horizontalAdvance(text) / kFine; }
double capHeightOf(const QFont &f) { return QFontMetricsF(fineFont(f)).capHeight() / kFine; }
double heightOf(const QFont &f) { return QFontMetricsF(fineFont(f)).height() / kFine; }

// A line of a fine layout, in layout units.
class PtLine {
public:
    explicit PtLine(const QTextLine &l) : m_l(l) {}
    bool isValid() const { return m_l.isValid(); }
    int textStart() const { return m_l.textStart(); }
    int textLength() const { return m_l.textLength(); }
    double ascent() const { return m_l.ascent() / kFine; }
    double height() const { return m_l.height() / kFine; }
    double naturalTextWidth() const { return m_l.naturalTextWidth() / kFine; }
    double y() const { return m_l.y() / kFine; }
    QPointF position() const { return m_l.position() / kFine; }
    void setPosition(const QPointF &p) { m_l.setPosition(p * kFine); }
    void setLineWidth(double w) { m_l.setLineWidth(w * kFine); }
    void setNumColumns(int n, double w) { m_l.setNumColumns(n, w * kFine); }
    double cursorToX(int pos) const { return m_l.cursorToX(pos) / kFine; }
    int xToCursor(double x, QTextLine::CursorPosition cp) const { return m_l.xToCursor(x * kFine, cp); }
    // Glyph runs in fine units: draw them through drawFine.
    QList<QGlyphRun> glyphRuns(int from, int length) const { return m_l.glyphRuns(from, length); }

private:
    QTextLine m_l;
};

// Draws fine glyph runs with their origin at `at` (layout units).
void drawFine(QPainter *p, const QPointF &at, const QList<QGlyphRun> &runs)
{
    p->save();
    p->translate(at);
    p->scale(1 / kFine, 1 / kFine);
    for (const QGlyphRun &g : runs) p->drawGlyphRun(QPointF(0, 0), g);
    p->restore();
}

// How far to move a centered or right-aligned line so it aligns on its
// letters: Publisher lets the spaces a line ends with hang outside, Qt
// counts them whenever they fit. In a right-to-left paragraph the spaces
// are at the letters' left, so it is the line set at the left (the end) that
// hangs them, and one set at the right already ends on its letters. A line
// wider than its room stays as Qt set it.
double hangTrailingSpaces(const PtLine &line, const QString &disp, const QTextOption &opt, double width)
{
    const Qt::Alignment al = opt.alignment();
    if (al & Qt::AlignJustify) return 0;
    const bool rtl = opt.textDirection() == Qt::RightToLeft;
    const bool center = al & Qt::AlignHCenter, right = (al & Qt::AlignRight) && !rtl, toLeft = (al & Qt::AlignLeft) && rtl;
    if (!center && !right && !toLeft) return 0;
    const int s = line.textStart(), e = std::min<int>(s + line.textLength(), disp.size());
    int k = e;
    auto hangs = [](QChar c) { return c.isSpace() && c.unicode() != 0x00A0 && c.unicode() != 0x202F; };
    while (k > s && hangs(disp[k - 1])) --k;
    if (k == e || k == s) return 0;
    const double a = line.cursorToX(s), b = line.cursorToX(k);
    const double ink = std::abs(b - a);
    if (ink > width) return 0;
    const double left = std::min(a, b) - line.position().x();
    return (center ? (width - ink) / 2 : right ? width - ink : 0) - left;
}
} // namespace

void ParagraphLayout::resetTabs()
{
    m_sets.clear();
    m_lineSet.clear();
    m_current = -1;
    QTextOption o = textOption();
    o.setTabs({});
    setTextOption(o);
}

void ParagraphLayout::setLineTabs(const QList<QTextOption::Tab> &tabs)
{
    int k = m_sets.indexOf(tabs);
    if (k < 0) {
        m_sets << tabs;
        k = int(m_sets.size()) - 1;
    }
    use(k);
}

void ParagraphLayout::keepLineTabs(int line)
{
    if (m_lineSet.size() <= line) m_lineSet.resize(line + 1);
    m_lineSet[line] = m_current;
}

void ParagraphLayout::use(int set)
{
    if (set == m_current || set < 0 || set >= m_sets.size()) return;
    QTextOption o = textOption();
    o.setTabs(m_sets[set]);
    setTextOption(o);
    m_current = set;
}

QTextLine ParagraphLayout::lineAt(int i)
{
    if (m_sets.size() > 1) use(m_lineSet.value(i, m_current));
    return QTextLayout::lineAt(i);
}

void ParagraphLayout::draw(QPainter *p, const QPointF &pos, const QList<FormatRange> &selections, const QRectF &clip)
{
    if (m_sets.size() < 2) {
        QTextLayout::draw(p, pos, selections, clip);
        return;
    }
    // Line by line, each with its own stops. Qt draws a selection's background
    // behind its text and the text in the selection's color where it has one,
    // the rest in the painter's: the same here, from the selected span of each line.
    QVector<bool> shown(lineCount());
    for (int i = 0; i < lineCount(); ++i) {
        const QTextLine l = lineAt(i);
        shown[i] = !clip.isValid() || !(l.y() > clip.bottom() - pos.y() || l.y() + l.height() < clip.top() - pos.y());
    }
    auto drawLines = [&] {
        for (int i = 0; i < lineCount(); ++i)
            if (shown[i]) lineAt(i).draw(p, pos);
    };
    QPainterPath textDone;
    for (const FormatRange &sel : selections) {
        QPainterPath region;
        region.setFillRule(Qt::WindingFill);
        for (int i = 0; i < lineCount(); ++i) {
            if (!shown[i]) continue;
            const QTextLine l = lineAt(i);
            const int from = std::max(sel.start, l.textStart()), to = std::min(sel.start + sel.length, l.textStart() + l.textLength());
            if (to <= from) continue;
            // Not past the line's text: the spaces that end a line aren't highlighted.
            const QRectF text = l.naturalTextRect();
            const double x1 = std::clamp(l.cursorToX(from), text.left(), text.right()), x2 = std::clamp(l.cursorToX(to), text.left(), text.right());
            region.addRect(QRectF(QPointF(std::min(x1, x2), l.y()) + pos, QSizeF(std::abs(x2 - x1), l.height())));
        }
        p->save();
        p->setPen(sel.format.penProperty(QTextFormat::OutlinePen));
        p->setBrush(sel.format.brushProperty(QTextFormat::BackgroundBrush));
        p->drawPath(region);
        p->restore();
        if (sel.format.foreground().style() != Qt::NoBrush) {
            p->save();
            p->setPen(QPen(sel.format.foreground(), p->pen().widthF()));
            p->setClipPath(region, Qt::IntersectClip);
            drawLines();
            p->restore();
            textDone += region;
        }
    }
    if (textDone.isEmpty()) {
        drawLines();
        return;
    }
    // The text of the rest.
    QPainterPath rest;
    rest.addRect(QRectF(-1e7, -1e7, 2e7, 2e7));
    rest -= textDone;
    p->save();
    p->setClipPath(rest, Qt::IntersectClip);
    drawLines();
    p->restore();
}

void drawPlainText(QPainter *p, const QPointF &baseline, const QFont &font, const QString &text)
{
    if (text.isEmpty()) return;
    QTextLayout tl(text, fineFont(font));
    tl.setCacheEnabled(true);
    tl.beginLayout();
    QTextLine line = tl.createLine();
    if (!line.isValid()) { tl.endLayout(); return; }
    line.setLineWidth(1e7);
    line.setPosition(QPointF(0, 0));
    tl.endLayout();
    drawFine(p, baseline - QPointF(0, line.ascent() / kFine), line.glyphRuns());
}

// `text` drawn as an outline (the effect of a word processor's outline text): the shapes of its letters stroked with a hairline, and not filled.
static void drawOutlinedText(QPainter *p, const QPointF &baseline, const QFont &font, const QString &text, const QColor &color)
{
    if (text.isEmpty()) return;
    QTextLayout tl(text, fineFont(font));
    tl.setCacheEnabled(true);
    tl.beginLayout();
    QTextLine line = tl.createLine();
    if (!line.isValid()) { tl.endLayout(); return; }
    line.setLineWidth(1e7);
    line.setPosition(QPointF(0, 0));
    tl.endLayout();
    QPainterPath outline;
    for (const QGlyphRun &g : line.glyphRuns()) {
        const QRawFont raw = g.rawFont();
        const QList<quint32> glyphs = g.glyphIndexes();
        const QList<QPointF> places = g.positions();
        for (int i = 0; i < glyphs.size() && i < places.size(); ++i) outline.addPath(raw.pathForGlyph(glyphs[i]).translated(places[i]));
    }
    p->save();
    p->translate(baseline - QPointF(0, line.ascent() / kFine));
    p->scale(1 / kFine, 1 / kFine);
    QPen pen(color, 0.25 * kFine);
    pen.setJoinStyle(Qt::MiterJoin);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);
    p->drawPath(outline);
    p->restore();
}

static int s_fittedOutlinesBuilt = 0;
int fittedMarkerOutlinesBuiltForTest() { return s_fittedOutlinesBuilt; }

// The outline of `text` in `font`, scaled to fill a box `size` large with its
// corner at the origin; empty if the font has nothing for it. Made once for
// each font, text, and box: a page of bullets is painted again at every change.
static QPainterPath fittedGlyphOutline(const QFont &font, const QString &text, const QSizeF &size)
{
    static QHash<QString, QPainterPath> cache;   // main thread only, like all layout
    QFont f = font;
    f.setPointSizeF(100);
    const QString key = f.key() + QLatin1Char('|') + text + QLatin1Char('|') + QString::number(size.width(), 'f', 3) + QLatin1Char('x') + QString::number(size.height(), 'f', 3);
    if (const auto it = cache.constFind(key); it != cache.constEnd()) return *it;
    ++s_fittedOutlinesBuilt;
    QPainterPath glyph, fitted;
    glyph.addText(QPointF(), f, text);
    const QRectF b = glyph.boundingRect();
    if (b.width() > 0 && b.height() > 0) {
        QTransform t;
        t.scale(size.width() / b.width(), size.height() / b.height());
        t.translate(-b.left(), -b.top());
        fitted = t.map(glyph);
    }
    if (cache.size() >= 256) cache.clear();
    cache.insert(key, fitted);
    return fitted;
}

// A bullet drawn to the box Publisher's glyph fills, whose origin is `at` on
// the baseline: a circle, or the glyph the font has for it scaled to fit (a
// missing symbol font's stand-in, whose own size and place don't match).
static void drawFittedMarker(QPainter *p, const QPointF &at, const QFont &font, const QString &text, const QRectF &ink, bool round, const QColor &color, bool hollow = false)
{
    const QRectF box = ink.translated(at);
    QPen hairline(color, 0.25);   // (an outlined marker's shape is stroked, as outlined text's is, and not filled)
    hairline.setJoinStyle(Qt::MiterJoin);
    if (round) {
        QPainterPath circle;
        circle.addEllipse(box);
        if (hollow) p->strokePath(circle, hairline);
        else p->fillPath(circle, color);
        return;
    }
    const QPainterPath fitted = fittedGlyphOutline(font, text, box.size());
    if (fitted.isEmpty()) {
        drawPlainText(p, at, font, text);
        return;
    }
    p->save();
    p->translate(box.topLeft());
    if (hollow) p->strokePath(fitted, hairline);
    else p->fillPath(fitted, color);
    p->restore();
}

// ---------- formats ----------
namespace { static double trackingSpace(const QTextCharFormat &f, const QFont &resolved); }

// The space width (ems) of the font a format is drawn with, without letter
// or word spacing.
static double standInSpaceEm(QFont f)
{
    static QHash<QString, double> cache;   // main thread only, like all layout
    f.setLetterSpacing(QFont::AbsoluteSpacing, 0);
    f.setWordSpacing(0);
    f.setPixelSize(1000);
    const QString key = f.key();
    if (const auto it = cache.constFind(key); it != cache.constEnd()) return *it;
    const double em = QFontMetricsF(f).horizontalAdvance(QLatin1Char(' ')) / 1000;
    cache.insert(key, em);
    return em;
}

// A run's size once the text is scaled. A slide's text the presentation program shrank to fit (a:normAutofit's fontScale) is drawn in
// whole points there: the size times the scale, rounded to the nearest point (halves up), 1 point at least. Probed in the program
// over sizes of 1 to 60 points and scales of 25 to 92.5 percent: 28 points at 85 percent are 24, not 23.8; 10.5 points at 92.5 are
// 10. A text's own fractional size (10.5 points at 100 percent) stays. Everything else keeps the exact product.
double slideShrunkSize(double size, double scale)
{
    if (scale < 1.0) return std::max(1.0, std::floor(size * scale + 0.5 + 1e-9));
    return std::max(1.0, size * scale);
}

static double scaledSize(double size, const LayoutEnv &env)
{
    return env.slideLines ? slideShrunkSize(size, env.fontScale) : std::max(1.0, size * env.fontScale);
}

namespace { static bool wordKnowsFamily(const QString &lower); }

// Whether the word processor's machine has a family: the ones it measures (the tables of its fonts' lines and letters), the document's own fonts, and
// the ones this machine has. The look-alikes JeffPub keeps for fonts a publisher's files ask for are not the fonts of that machine.
static bool wordHasFamily(const QString &family, const LayoutEnv &env)
{
    const QString lower = family.toLower();
    return wordKnowsFamily(lower) || env.familyAliases.contains(lower) || hasFamilyCached(family) || originalLetterWidths(family, false, false);
}

QString wordStandInFor(const QString &alternate, const QString &family, const QString &pitch, const QString &panose, const QString &charset)
{
    if (!alternate.isEmpty() && wordHasFamily(alternate, LayoutEnv())) return alternate;
    if (charset == QLatin1String("02")) return QStringLiteral("Wingdings");
    // A PANOSE of Latin text (digits 1-2 are 02) says its stroke in digits 3-4 (0B to 0F sans, 02 to 0A serif) and its proportion in digits 7-8 (09 is monospaced).
    const bool latin = panose.size() == 20 && panose.startsWith(QLatin1String("02"));
    // A fixed-pitch font, or one whose PANOSE says monospaced, is a typewriter's whatever its strokes or its kind say (Courier New's own PANOSE has a serif's).
    if (pitch == QLatin1String("fixed") || (latin && panose.mid(6, 2) == QLatin1String("09"))) return QStringLiteral("Courier New");
    if (latin) {
        bool ok = false;
        const int style = panose.mid(2, 2).toInt(&ok, 16);
        if (ok && style >= 0x0B && style <= 0x0F) return QStringLiteral("Arial");
        if (ok && style >= 0x02 && style <= 0x0A) return QStringLiteral("Times New Roman");
    }
    if (family == QLatin1String("roman")) return QStringLiteral("Times New Roman");
    if (family == QLatin1String("modern")) return QStringLiteral("Courier New");
    return {};
}

// Whether a run set in a symbol font (a symbol of it: DisplayFamily) that the machine lacks shows its pictures in the run's own family: they take the font's widths.
static bool picturesInRunsFamily(const QString &family)
{
    return isSymbolFont(family) && !hasFamilyCached(family) && symbolFontLetterWidths(family);
}

QTextCharFormat resolveCharFormat(const QTextCharFormat &original, const LayoutEnv &env, const QColor &behind)
{
    QTextCharFormat f = original;
    // (Not a symbol font this machine lacks, whose pictures the run's own family draws, by that font's widths below: fontconfig stands a symbol-encoded face in
    // for the name Symbol, which leaves a bullet and a diamond blank.)
    if (env.wordLines && f.hasProperty(tp::DisplayFamily) && !picturesInRunsFamily(f.stringProperty(tp::DisplayFamily))) f.setFontFamilies({f.stringProperty(tp::DisplayFamily)});
    // A word processor sets a family that isn't on its machine in Calibri (its stand-in): the names it knows, the ones this machine has, and those the
    // document carries a font of are not such.
    if (env.wordLines && f.hasProperty(QTextFormat::FontFamilies)) {
        QStringList families = f.fontFamilies().toStringList();
        bool changed = false;
        for (QString &family : families) {
            const QString lower = family.toLower();
            if (family.isEmpty() || wordHasFamily(family, env)) continue;
            const QString named = env.standIns.value(lower);
            family = !named.isEmpty() && wordHasFamily(named, env) ? named : QStringLiteral("Calibri");
            changed = true;
        }
        if (changed) f.setFontFamilies(families);
    }
    // A family the document carries a font of is that font's private name from here on, before the stand-ins are looked for.
    if (!env.familyAliases.isEmpty()) {
        QStringList families;
        if (f.hasProperty(QTextFormat::FontFamilies)) families = f.fontFamilies().toStringList();
        else families << (f.stringProperty(tp::ThemeFont) == QLatin1String("major") ? env.fonts.heading : env.fonts.body);
        bool changed = false;
        for (QString &family : families)
            if (const QString own = env.aliasOf(family); own != family) {
                family = own;
                changed = true;
            }
        if (changed) f.setFontFamilies(families);
    }
    QTextCharFormat r = f;
    // A run that doesn't say it's bold isn't: a paragraph's text layout
    // fills what a run leaves unsaid from its own font, the first run's, so
    // after a bold, underlined lead-in the whole paragraph came out bold and
    // underlined (.pub runs state only what they turn on).
    if (!f.hasProperty(QTextFormat::FontWeight)) r.setFontWeight(QFont::Normal);
    if (!f.hasProperty(QTextFormat::FontItalic)) r.setFontItalic(false);
    if (!f.hasProperty(QTextFormat::FontUnderline) && !f.hasProperty(QTextFormat::TextUnderlineStyle)) r.setFontUnderline(false);
    if (!f.hasProperty(QTextFormat::FontStrikeOut)) r.setFontStrikeOut(false);
    if (!f.hasProperty(QTextFormat::FontOverline)) r.setFontOverline(false);
    if (!f.hasProperty(QTextFormat::FontCapitalization)) r.setFontCapitalization(QFont::MixedCase);
    if (!f.hasProperty(QTextFormat::FontFamilies)) {
        const QString theme = f.stringProperty(tp::ThemeFont);
        r.setFontFamilies(QStringList{theme == QLatin1String("major") ? env.fonts.heading : env.fonts.body});
    }
    // Imitate condensed fonts that are missing on this computer, times the
    // run's own character scaling. A stated 100% is left unsaid: an
    // explicit 100 makes font matching pass over a stand-in's narrower faces.
    bool fauxBold = false, boldAlways = false;
    if (f.hasProperty(QTextFormat::FontFamilies)) {
        const QStringList fams = f.fontFamilies().toStringList();
        if (!fams.isEmpty()) {
            const int own = f.hasProperty(QTextFormat::FontStretch) ? f.fontStretch() : 100;
            const int st = int(std::lround(own * substituteStretch(fams.first(), f.fontWeight() >= QFont::DemiBold, f.fontItalic()) / 100.0));
            if (st != 100) r.setFontStretch(stretchToAsk(st));
            else r.clearProperty(QTextFormat::FontStretch);
            // "Medium", "Demi", "Black"... in a missing font's name become a real weight.
            const bool setBold = f.hasProperty(QTextFormat::FontWeight) && f.fontWeight() >= QFont::Bold;
            // Bold set on a font with no bold face is drawn by hand: its
            // regular face with a stroke added as it is painted (below).
            fauxBold = !substituteFor(fams.first()).isEmpty() && originalBoldByHand(fams.first(), setBold);
            boldAlways = fauxBold && originalBoldAlwaysByHand(fams.first());
            const bool boldRun = setBold && !fauxBold;
            if (const int wt = substituteWeight(fams.first(), boldRun); wt > 0) {
                r.setFontWeight(wt);
                // Selawik's Light and Semilight weigh the same where they are styles of one family.
                if (const QString style = substituteStyleName(fams.first(), boldRun, f.fontItalic()); !style.isEmpty()) r.setFontStyleName(style);
            } else if (fauxBold) {
                r.setFontWeight(QFont::Normal);
            }
            // Letters as high as the missing font's (drawing only), by the
            // weight asked for: a regular run's stand-in may be drawn bold.
            if (const double hs = substituteHeightScale(fams.first(), f.fontWeight() >= QFont::DemiBold); hs != 1)
                r.setProperty(tp::GlyphScaleY, hs);
            // Where the original has no italic face for this weight, its
            // upright face is slanted (drawing only): not the stand-in's own
            // italic. Outlined, highlighted and wavy-underlined text is
            // drawn by Qt, which can't slant it.
            if (f.fontItalic() && !f.hasProperty(QTextFormat::TextOutline) && !f.hasProperty(QTextFormat::BackgroundBrush) &&
                f.underlineStyle() != QTextCharFormat::WaveUnderline)
                if (const double shear = substituteItalicShear(fams.first(), f.fontWeight() >= QFont::DemiBold); shear > 0) {
                    r.setFontItalic(false);
                    r.setProperty(tp::SyntheticItalic, shear);
                }
        }
    } else if (f.hasProperty(QTextFormat::FontStretch)) {
        // A run in the theme's font: its own character scaling only.
        if (f.fontStretch() != 100) r.setFontStretch(stretchToAsk(f.fontStretch()));
        else r.clearProperty(QTextFormat::FontStretch);
    }
    // A missing stock font JeffPub has measured: its letters take the
    // original's widths (applyLetterWidths), so lines break as they do there.
    {
        const QStringList fams = r.fontFamilies().toStringList();
        const bool bold = f.hasProperty(QTextFormat::FontWeight) && f.fontWeight() >= QFont::DemiBold, italic = f.fontItalic();
        if (env.wordLines && f.hasProperty(tp::DisplayFamily) && picturesInRunsFamily(f.stringProperty(tp::DisplayFamily))) {
            // (A symbol of the font: its picture is as wide as the font makes it, and its line is the font's.)
            r.setProperty(kLetterWidths, f.stringProperty(tp::DisplayFamily) + QStringLiteral("|0|0"));
            r.setFontFamilies(fams + QStringList{f.stringProperty(tp::DisplayFamily)});
        } else if (!fams.isEmpty() && !substituteFor(fams.first()).isEmpty() && originalLetterWidths(fams.first(), bold, italic))
            r.setProperty(kLetterWidths, fams.first() + (bold ? QStringLiteral("|1") : QStringLiteral("|0")) + (italic ? QStringLiteral("|1") : QStringLiteral("|0")));
        else if (env.wordLines && !fams.isEmpty() && isSymbolFont(fams.first()) && !hasFamilyCached(fams.first()) && symbolFontLetterWidths(fams.first()))
            r.setProperty(kLetterWidths, fams.first() + QStringLiteral("|0|0"));   // (its characters are shown as their pictures, by the widths of the font's)
    }
    // Courier New, where the machine lacks it, is drawn in Courier Prime, a face of the same widths with strokes as thin as its own (its usual stand-in, Cousine,
    // is heavy); its line stays Courier New's.
    if (env.wordLines) {
        QStringList fams = r.fontFamilies().toStringList();
        const QString lower = fams.value(0).toLower();
        if ((lower == QLatin1String("courier new") || lower == QLatin1String("courier")) && !hasFamilyCached(fams.first()) && hasFamilyCached(QStringLiteral("Courier Prime"))) {
            // (The family it is drawn in, then the one whose line it keeps: wordFaceOf reads the second wherever a line is measured, from a run's font, a paragraph's, a
            // list marker's, or the next paragraph's.)
            fams.insert(0, kThinCourier);
            r.setFontFamilies(fams);
        }
    }
    // Sizes within the other program's range (to 1,638 pt): a file can claim
    // anything, and a huge size makes drawing a single glyph very costly.
    const double sz = std::clamp(f.hasProperty(QTextFormat::FontPointSize) ? f.fontPointSize() : 11.0, 0.5, 1638.0);
    r.setFontPointSize(scaledSize(sz, env) * fontPointFactor());
    // Automatic pair kerning applies from a size up, 14 pt unless the text
    // says otherwise (smaller text keeps the font's plain widths, as .pub
    // files are laid out); text can also turn kerning off.
    // (A slide's counts the size the text is drawn at, shrunk or not: 28 points kerned from 12 are kerned at 15 and not at 9, as probed.)
    {
        // (A word processor's text is kerned only where its file asks, from the size it says: a run that says nothing isn't, at any size.)
        const double kernFrom = f.hasProperty(tp::KernAbove) ? f.property(tp::KernAbove).toDouble() : env.wordLines ? 1e9 : 14.0;
        r.setFontKerning((!f.hasProperty(QTextFormat::FontKerning) || f.fontKerning()) && (env.slideLines ? scaledSize(sz, env) : sz) >= kernFrom);
    }
    // Unhinted design widths: hinting differs between Windows and Linux and
    // would move line breaks; publications must lay out the same everywhere.
    r.setFontHintingPreference(QFont::PreferNoHinting);
    // Kerning and tracking both become space after each letter.
    {
        const double kern = tp::kerningOf(f) * (env.slideLines ? 1.0 : env.fontScale), track = trackingSpace(f, r.font());   // (a slide's letter spacing isn't shrunk)
        if (kern != 0 || track != 0 || f.hasProperty(QTextFormat::FontLetterSpacing)) {
            r.setFontLetterSpacingType(QFont::AbsoluteSpacing);
            r.setFontLetterSpacing(kern + track);
        }
    }
    // A missing font's word spaces, where its stand-in's differ (a measured
    // one's are set with its letters, in applyLetterWidths).
    if (f.hasProperty(QTextFormat::FontFamilies) && !f.hasProperty(QTextFormat::FontWordSpacing) && !r.hasProperty(kLetterWidths)) {
        const QStringList fams = f.fontFamilies().toStringList();
        if (const double want = fams.isEmpty() ? 0 : substituteSpaceEm(fams.first(), f.fontWeight() >= QFont::DemiBold, f.fontItalic()); want > 0) {
            const QFont rf = r.font();
            const double em = rf.pointSizeF() / fontPointFactor();
            r.setFontWordSpacing((want - standInSpaceEm(rf)) * em);
        }
    }
    const QString cref = f.stringProperty(tp::ColorRefP);
    if (!cref.isEmpty()) r.setForeground(ColorRef::fromString(cref).resolve(env.colors));
    else if (!f.hasProperty(QTextFormat::ForegroundBrush)) r.setForeground(env.colors.slot(Main));
    // A gradient text fill, spread over each run of text as it is drawn.
    const QString tf = f.stringProperty(tp::TextFill);
    if (!tf.isEmpty()) {
        const Fill fill = Fill::fromJson(QJsonDocument::fromJson(tf.toUtf8()).object());
        if (fill.type == Fill::Gradient) {
            const double a = qDegreesToRadians(fill.angle);
            const QPointF dir(std::cos(a) / 2, std::sin(a) / 2);
            QLinearGradient g(QPointF(0.5, 0.5) - dir, QPointF(0.5, 0.5) + dir);
            g.setCoordinateMode(QGradient::ObjectBoundingMode);
            if (fill.stops.isEmpty()) {
                g.setColorAt(0, fill.color.resolve(env.colors));
                g.setColorAt(1, fill.color2.resolve(env.colors));
            } else {
                for (const GradientStop &st : fill.stops) g.setColorAt(st.pos, st.color.resolve(env.colors));
            }
            r.setForeground(QBrush(g));
        }
    }
    if (f.isAnchor() && cref.isEmpty()) {
        r.setForeground(env.colors.slot(Hyperlink));
        r.setFontUnderline(true);
    }
    const QString href = f.stringProperty(tp::HighlightRefP);
    if (!href.isEmpty()) r.setBackground(ColorRef::fromString(href).resolve(env.colors));
    const QString oref = f.stringProperty(tp::OutlineRef);
    if (!oref.isEmpty()) {
        QPen pen(ColorRef::fromString(oref).resolve(env.colors), f.hasProperty(tp::OutlineWidth) ? f.property(tp::OutlineWidth).toDouble() : 0.5);
        pen.setJoinStyle(Qt::RoundJoin);
        r.setTextOutline(pen);
    } else if (fauxBold) {
        // Drawn with a stroke added when the text is painted (StoryLayout::paint),
        // as wide as the place it goes to calls for: the layout is the same in all.
        r.setProperty(tp::HandBold, boldAlways ? 2 : 1);
    }
    if (f.boolProperty(tp::TrueSmallCaps)) r.setFontCapitalization(QFont::SmallCaps);
    // Text of the automatic color is white where the fill under it is dark: the run's own (its highlight), else the one behind the text (its paragraph's shading, or its cell's).
    if (env.wordLines && cref.isEmpty() && !f.hasProperty(QTextFormat::ForegroundBrush) && !f.isAnchor() && tf.isEmpty()) {
        const QBrush own = r.background();
        const bool shaded = own.style() == Qt::SolidPattern && own.color().alpha() > 0;
        if (tp::isDarkFill(shaded ? own.color() : behind)) r.setForeground(Qt::white);
    }
    return r;
}

// A paragraph has true small capitals (the font's own, an OpenType feature)
// when every run with letters in it asks for them. Qt takes a feature from the
// paragraph's layout font, for all its runs alike, so in a paragraph where only
// some ask, the runs that do have small capitals made of capitals instead
// (buildOnce) and the others are as they are.
static bool hasTrueSmallCaps(const QTextBlock &b)
{
    if (b.begin().atEnd()) return b.charFormat().boolProperty(tp::TrueSmallCaps);
    bool any = false;
    for (auto it = b.begin(); !it.atEnd(); ++it) {
        const QTextFragment frag = it.fragment();
        if (!frag.isValid()) continue;
        const QString text = frag.text();
        if (std::none_of(text.cbegin(), text.cend(), [](QChar ch) { return ch.isLetter(); })) continue;
        if (!frag.charFormat().boolProperty(tp::TrueSmallCaps)) return false;
        any = true;
    }
    return any;
}

// The sizes (before any scale) of a paragraph's text: the largest, and the first run's. An empty paragraph has its mark's.
static double fileSize(const QTextCharFormat &cf)
{
    return cf.hasProperty(QTextFormat::FontPointSize) ? cf.fontPointSize() : 11.0;
}
static double rawLargestSize(const QTextBlock &b)
{
    double m = 0;
    for (auto it = b.begin(); !it.atEnd(); ++it) m = std::max(m, fileSize(it.fragment().charFormat()));
    return m > 0 ? m : fileSize(b.charFormat());
}

// A slide paragraph's space before (tp::SpaceBeforeShare) or after (tp::SpaceAfterShare), in points, with its text shrunk by `scale`: the
// share of a line of the largest text at the size it is drawn at where the file gave it as one (and the margin hasn't been set to points
// since: tp::spaceShareOf), the margin itself, unscaled, otherwise. A share follows the text's size, as in the program, whether the size
// came from shrinking or from being changed.
double slideSpaceOf(const QTextBlock &b, int shareProperty, double scale)
{
    const QTextBlockFormat bf = b.blockFormat();
    const bool before = shareProperty == tp::SpaceBeforeShare;
    if (const double share = tp::spaceShareOf(bf, before); share > 0) return std::clamp(share * 1.2 * slideShrunkSize(rawLargestSize(b), scale), 0.0, 4000.0);
    return before ? bf.topMargin() : bf.bottomMargin();
}
static double rawFirstSize(const QTextBlock &b)
{
    auto it = b.begin();
    return std::max(0.5, it.atEnd() ? fileSize(b.charFormat()) : fileSize(it.fragment().charFormat()));
}

// Whether a text has a letter of a script that is shaped with ligatures of its own (Arabic, the scripts of India and of southeast Asia...), which no setting
// of ligatures is to touch.
static bool hasComplexScript(const QString &text)
{
    for (const QChar ch : text) {
        const char16_t u = ch.unicode();
        if ((u >= 0x0590 && u < 0x1E00) || (u >= 0xFB1D && u <= 0xFDFF) || (u >= 0xFE70 && u <= 0xFEFF)) return true;
    }
    return false;
}

QFont baseFontFor(const QTextBlock &b, const LayoutEnv &env)
{
    QTextCharFormat cf = b.charFormat();
    // Prefer the first fragment's format so empty-paragraph height matches its text.
    auto it = b.begin();
    if (!it.atEnd()) cf = it.fragment().charFormat();
    QFont f = resolveCharFormat(cf, env).font();
    // Paragraph-level OpenType features (Qt 6.8 applies features per layout font).
    const int num = cf.intProperty(tp::NumberStyle), sp = cf.intProperty(tp::NumberSpacing), lig = cf.intProperty(tp::Ligatures);
    if (num == 1) f.setFeature(QFont::Tag("lnum"), 1);
    if (num == 2) f.setFeature(QFont::Tag("onum"), 1);
    if (sp == 1) f.setFeature(QFont::Tag("pnum"), 1);
    if (sp == 2) f.setFeature(QFont::Tag("tnum"), 1);
    // (A word processor's text has none unless its file asks, and not the letters of the scripts that need them: those are shaped as the font does.)
    const bool wordDefault = env.wordLines && !cf.hasProperty(tp::Ligatures) && !hasComplexScript(b.text());
    if (lig == 1 || env.noLigatures || wordDefault) { f.setFeature(QFont::Tag("liga"), 0); f.setFeature(QFont::Tag("clig"), 0); }
    if (lig == 2) { f.setFeature(QFont::Tag("dlig"), 1); f.setFeature(QFont::Tag("hlig"), 1); }
    const int ss = cf.intProperty(tp::StylisticSet);
    if (ss > 0 && ss <= 20) f.setFeature(QFont::Tag::fromString(QStringLiteral("ss%1").arg(ss, 2, 10, QChar('0'))).value_or(QFont::Tag()), 1);
    if (cf.boolProperty(tp::Swash)) f.setFeature(QFont::Tag("swsh"), 1);
    if (cf.boolProperty(tp::Alternates)) f.setFeature(QFont::Tag("salt"), 1);
    // Other capitals are made in the layout's text, run by run (buildOnce); Qt
    // would take them from this font, the first run's, for the whole paragraph.
    if (hasTrueSmallCaps(b)) f.setFeature(QFont::Tag("smcp"), 1);
    else f.setCapitalization(QFont::MixedCase);
    return f;
}

// ---------- block mapping ----------
int StoryLayout::Block::dispFromDoc(int rel) const
{
    for (const Seg &s : map) {
        if (rel < s.docPos) return s.dispPos;
        if (rel < s.docPos + s.docLen) return s.docLen == s.dispLen ? s.dispPos + (rel - s.docPos) : s.dispPos;
    }
    return disp.size();
}

int StoryLayout::Block::docFromDisp(int d) const
{
    for (const Seg &s : map) {
        // (Characters that show nothing, hidden text, belong to the line that follows them: a line ends where they begin.)
        if (s.hidden) {
            if (d == s.dispPos) return s.docPos;
            continue;
        }
        if (d < s.dispPos) return s.docPos;
        if (d < s.dispPos + s.dispLen) {
            if (s.docLen == s.dispLen) return s.docPos + (d - s.dispPos);
            return d - s.dispPos > s.dispLen / 2 ? s.docPos + s.docLen : s.docPos;
        }
    }
    return docLen;
}

// ---------- layout ----------
StoryLayout::StoryLayout() = default;
StoryLayout::~StoryLayout() = default;

namespace {
struct Iv { double x0, x1; };

QVector<Iv> freeIntervals(const QVector<QPolygonF> &obstacles, double x0, double x1, double y0, double y1)
{
    QVector<Iv> res{{x0, x1}};
    for (const QPolygonF &poly : obstacles) {
        const QRectF pb = poly.boundingRect();
        if (pb.bottom() <= y0 || pb.top() >= y1 || pb.right() <= x0 || pb.left() >= x1) continue;
        double bl = pb.left(), br = pb.right();
        if (poly.size() > 4 || !poly.isClosed() || true) {
            const QPolygonF clip = poly.intersected(QPolygonF(QRectF(pb.left() - 1, y0, pb.width() + 2, y1 - y0)));
            if (clip.isEmpty()) continue;
            const QRectF cb = clip.boundingRect();
            if (cb.width() <= 0 && cb.height() <= 0) continue;
            bl = cb.left(); br = cb.right();
        }
        QVector<Iv> next;
        for (const Iv &iv : res) {
            if (br <= iv.x0 || bl >= iv.x1) { next << iv; continue; }
            if (bl > iv.x0) next << Iv{iv.x0, bl};
            if (br < iv.x1) next << Iv{br, iv.x1};
        }
        res = next;
    }
    return res;
}

// A font's single line height in ems, as .pub layouts compute it. Measured
// against reference PDFs of .pub files: for the common system fonts it uses the OS/2
// typographic metrics (typoAscender - typoDescender + typoLineGap); for every
// other font the hhea metrics (ascender - descender + lineGap). Metric-compatible
// substitutes (Arimo for Arial, Tinos for Times New Roman...) follow the font
// that was asked for, which is what the publication was laid out with.
static bool usesTypoMetrics(const QString &family)
{
    static const QStringList core = {
        "Arial", "Arial Black", "Times New Roman", "Georgia", "Verdana", "Tahoma", "Trebuchet MS",
        "Courier New", "Calibri", "Cambria", "Candara", "Consolas", "Constantia", "Corbel", "Segoe UI", "Garamond",
        "Book Antiqua", "Century Gothic", "Franklin Gothic Medium", "Gill Sans MT", "Palatino Linotype",
        "Lucida Sans Unicode", "Comic Sans MS", "Impact"};
    return core.contains(family, Qt::CaseInsensitive);
}

// Line height and descent (ems) of common proprietary fonts, read from the
// fonts embedded in reference PDFs of .pub files. When such a font is missing and a
// substitute is drawn, lines keep the original's spacing and baseline. Which
// of a font's metrics Publisher spaces lines by (OS/2 typo or hhea) goes by
// the font, not by any flag in it: checked line by line against the
// baselines in its PDFs of 284 publications (Oct 7; Arial Narrow and
// Bookman Old Style use hhea, Times New Roman Bold its own typo ascender).
struct KnownMetrics { double line = 0, descent = 0, lift = 0; };
static KnownMetrics knownMetricsOf(const QString &family, bool bold)
{
    const QString f = family.toLower();
    // OS/2 typographic metrics of the core fonts, which their stand-ins
    // (Arimo, Tinos...) don't share: theirs give 1.15 em lines.
    if (f == "arial") return {(1491.0 + 431 + 307) / 2048, 431.0 / 2048};
    if (f == "arial narrow") return {(1916.0 + 434) / 2048, 434.0 / 2048};                  // hhea
    if (f == "times new roman") return {((bold ? 1387.0 : 1420.0) + 442 + 307) / 2048, 442.0 / 2048};
    if (f == "tahoma") return {(1566.0 + 423 + 59) / 2048, 423.0 / 2048};
    if (f == "garamond") return {(1765.0 + 539) / 2048, 539.0 / 2048};                    // win
    if (f == "book antiqua") return bold ? KnownMetrics{(1891.0 + 578) / 2048, 543.0 / 2048}   // win, its gap above
                                         : KnownMetrics{(1967.0 + 578) / 2048, 578.0 / 2048};  // win
    if (f == "bookman old style") return {(1929.0 + 475) / 2048, 475.0 / 2048};             // hhea
    if (f == "century gothic") return {(1536.0 + 426 + 229) / 2048, 426.0 / 2048};
    if (f == "gill sans mt condensed") return {(1913.0 + 553) / 2048, 553.0 / 2048};   // hhea
    if (f == "gill sans mt ext condensed bold") return {1.227, 407.0 / 2048};   // measured (Publisher test page); nearest metric hhea = 1.197
    if (f.startsWith("franklin gothic")) return {(1877.0 + 445) / 2048, 445.0 / 2048, 0.088};   // hhea
    if (f == "georgia") return {(1549.0 + 444 + 198) / 2048, 444.0 / 2048};              // OS/2 typo
    if (f == "arial black") return {(1466.0 + 434 + 291) / 2048, 434.0 / 2048};
    if (f.startsWith("gill sans mt")) return {(1903.0 + 472) / 2048, 472.0 / 2048, 0.096};
    if (f == "century schoolbook") return {(2019.0 + 443) / 2048, 443.0 / 2048};
    if (f == "calibri") return {(1536.0 + 512 + 452) / 2048, 512.0 / 2048};
    if (f == "lucida handwriting") return {(2098.0 + 727) / 2048, 727.0 / 2048};
    if (f == "juice itc") return {(1903.0 + 532) / 2048, 532.0 / 2048};
    if (f == "symbol") return {(2059.0 + 450) / 2048, 450.0 / 2048};                    // hhea
    if (f == "ag_futura") return bold ? KnownMetrics{(4264.0 + 1049) / 4096, 1049.0 / 4096}   // win, bold
                                      : KnownMetrics{(4051.0 + 1081) / 4096, 1081.0 / 4096};  // win
    if (f == "wingdings") return {(1841.0 + 432) / 2048, 432.0 / 2048};                 // hhea
    if (f == "arial rounded mt bold") return {(1938.0 + 432) / 2048, 432.0 / 2048};     // hhea
    if (f == "verdana") return {(1566.0 + 423 + 202) / 2048, 423.0 / 2048};              // OS/2 typo
    if (f == "comic sans ms") return {(1638.0 + 564) / 2048, 564.0 / 2048};              // OS/2 typo
    if (f == "agency fb") return {((bold ? 2042.0 : 2015.0) + 410) / 2048, 410.0 / 2048}; // hhea
    if (f == "ocr a extended") return {(1757.0 + 362) / 2048, 362.0 / 2048};             // hhea
    if (f == "castellar") return {(1878.0 + 571) / 2048, 571.0 / 2048};                  // hhea
    if (f == "imprint mt shadow") return {(1903.0 + 510) / 2048, 510.0 / 2048};          // hhea
    if (f == "abadi") return {(1817.0 + 504) / 2048, 504.0 / 2048};                      // hhea
    if (f == "eras medium itc") return {(1825.0 + 512) / 2048, 512.0 / 2048};            // hhea
    if (f == "tw cen mt condensed") return {(1694.0 + 494) / 2048, 494.0 / 2048};        // hhea
    if (f == "calisto mt") return {(1894.0 + 470) / 2048, 470.0 / 2048};                 // hhea
    if (f == "lucida sans typewriter") return {(1974.0 + 432) / 2048, 432.0 / 2048};     // hhea
    if (f == "rockwell") return {(1937.0 + 468) / 2048, 468.0 / 2048};                   // hhea
    // From Publisher's own test page of each font (Oct 8): the Windows
    // metric its line spacing matches, or the spacing measured there.
    if (f == "algerian") return {(1819.0 + 458 + 426) / 2048, 458.0 / 2048};   // hhea
    if (f == "bahnschrift") return {(1626.0 + 422 + 410) / 2048, 422.0 / 2048};   // hhea
    if (f == "bahnschrift condensed") return {(1626.0 + 422 + 410) / 2048, 422.0 / 2048};   // hhea
    if (f == "bahnschrift light") return {(1626.0 + 422 + 410) / 2048, 422.0 / 2048};   // hhea
    if (f == "bahnschrift light condensed") return {(1626.0 + 422 + 410) / 2048, 422.0 / 2048};   // hhea
    if (f == "bahnschrift light semicondensed") return {(1626.0 + 422 + 410) / 2048, 422.0 / 2048};   // hhea
    if (f == "bahnschrift semibold") return {(1626.0 + 422 + 410) / 2048, 422.0 / 2048};   // hhea
    if (f == "bahnschrift semibold condensed") return {(1626.0 + 422 + 410) / 2048, 422.0 / 2048};   // hhea
    if (f == "bahnschrift semibold semicondensed") return {(1626.0 + 422 + 410) / 2048, 422.0 / 2048};   // hhea
    if (f == "bahnschrift semicondensed") return {(1626.0 + 422 + 410) / 2048, 422.0 / 2048};   // hhea
    if (f == "bahnschrift semilight") return {(1626.0 + 422 + 410) / 2048, 422.0 / 2048};   // hhea
    if (f == "bahnschrift semilight condensed") return {(1626.0 + 422 + 410) / 2048, 422.0 / 2048};   // hhea
    if (f == "bahnschrift semilight semicondensed") return {(1626.0 + 422 + 410) / 2048, 422.0 / 2048};   // hhea
    if (f == "bauhaus 93") return {(1841.0 + 477 + 673) / 2048, 477.0 / 2048};   // hhea
    if (f == "bell mt") return bold ? KnownMetrics{(1772.0 + 547) / 2048, 547.0 / 2048}
                               : KnownMetrics{(1716.0 + 546) / 2048, 546.0 / 2048};   // win, bold differs
    if (f == "berlin sans fb") return bold ? KnownMetrics{(1857.0 + 678) / 2048, 678.0 / 2048}
                                      : KnownMetrics{(1835.0 + 416) / 2048, 416.0 / 2048};   // hhea, bold differs
    if (f == "berlin sans fb demi") return {(1880.0 + 419) / 2048, 419.0 / 2048};   // win
    if (f == "bernard mt condensed") return {(1986.0 + 446) / 2048, 446.0 / 2048};   // hhea
    if (f == "blackadder itc") return {(1836.0 + 791) / 2048, 791.0 / 2048};   // typo
    if (f == "bodoni mt") return {(1878.0 + 578) / 2048, 578.0 / 2048};   // hhea
    if (f == "bodoni mt black") return {(1878.0 + 514) / 2048, 514.0 / 2048};   // hhea
    if (f == "bodoni mt condensed") return {(1878.0 + 532) / 2048, 532.0 / 2048};   // hhea
    if (f == "bodoni mt poster compressed") return {1.174, 442.0 / 2048};   // measured (Publisher test page); nearest metric hhea = 1.147
    if (f == "bradley hand itc") return {(1739.0 + 815) / 2048, 815.0 / 2048};   // hhea
    if (f == "britannic bold") return {(1835.0 + 434) / 2048, 434.0 / 2048};   // win
    if (f == "broadway") return {(1869.0 + 450) / 2048, 450.0 / 2048};   // hhea
    if (f == "brush script mt") return {(1820.0 + 692) / 2048, 692.0 / 2048};   // hhea
    if (f == "californian fb") return {(1803.0 + 523) / 2048, 523.0 / 2048};   // hhea
    if (f == "cambria") return {(1946.0 + 455) / 2048, 455.0 / 2048};   // hhea
    if (f == "cambria math") return {(1595.0 + 455 + 353) / 2048, 455.0 / 2048};   // hhea
    if (f == "cascadia code") return {(1900.0 + 480) / 2048, 480.0 / 2048};   // hhea
    if (f == "cascadia code extralight") return {(1900.0 + 480) / 2048, 480.0 / 2048};   // hhea
    if (f == "cascadia code light") return {(1900.0 + 480) / 2048, 480.0 / 2048};   // hhea
    if (f == "cascadia code semibold") return {(1900.0 + 480) / 2048, 480.0 / 2048};   // hhea
    if (f == "cascadia code semilight") return {(1900.0 + 480) / 2048, 480.0 / 2048};   // hhea
    if (f == "cascadia mono") return {(1900.0 + 480) / 2048, 480.0 / 2048};   // hhea
    if (f == "cascadia mono extralight") return {(1900.0 + 480) / 2048, 480.0 / 2048};   // hhea
    if (f == "cascadia mono light") return {(1900.0 + 480) / 2048, 480.0 / 2048};   // hhea
    if (f == "cascadia mono semibold") return {(1900.0 + 480) / 2048, 480.0 / 2048};   // hhea
    if (f == "cascadia mono semilight") return {(1900.0 + 480) / 2048, 480.0 / 2048};   // hhea
    if (f == "centaur") return {(1745.0 + 590) / 2048, 590.0 / 2048};   // hhea
    if (f == "century") return {(2019.0 + 443) / 2048, 443.0 / 2048};   // hhea
    if (f == "chiller") return {(1738.0 + 625) / 2048, 625.0 / 2048};   // hhea
    if (f == "colonna mt") return {(1220.0 + 682 + 289) / 2048, 682.0 / 2048};   // typo
    if (f == "consolas") return {(1521.0 + 527 + 350) / 2048, 527.0 / 2048};   // hhea
    if (f == "constantia") return {(1538.0 + 510 + 452) / 2048, 510.0 / 2048};   // hhea
    if (f == "cooper black") return {(1880.0 + 469) / 2048, 469.0 / 2048};   // hhea
    if (f == "copperplate gothic bold") return {(1813.0 + 465) / 2048, 465.0 / 2048};   // hhea
    if (f == "copperplate gothic light") return {(1786.0 + 465) / 2048, 465.0 / 2048};   // hhea
    if (f == "corbel") return {(1523.0 + 525 + 425) / 2048, 525.0 / 2048};   // hhea
    if (f == "curlz mt") return {(2103.0 + 614) / 2048, 614.0 / 2048};   // hhea
    if (f == "dubai") return {(1129.0 + 559) / 1000, 559.0 / 1000};   // hhea
    if (f == "dubai light") return {(1129.0 + 559) / 1000, 559.0 / 1000};   // hhea
    if (f == "dubai medium") return {(1129.0 + 559) / 1000, 559.0 / 1000};   // hhea
    if (f == "edwardian script itc") return {1.200, 672.0 / 2048};   // measured (Publisher test page); nearest metric hhea = 1.179
    if (f == "elephant") return {(983.0 + 306) / 1000, 306.0 / 1000};   // win
    if (f == "engravers mt") return {(1920.0 + 465) / 2048, 465.0 / 2048};   // hhea
    if (f == "eras bold itc") return {(1859.0 + 512) / 2048, 512.0 / 2048};   // hhea
    if (f == "eras demi itc") return {(1841.0 + 512) / 2048, 512.0 / 2048};   // hhea
    if (f == "eras light itc") return {(1810.0 + 512) / 2048, 512.0 / 2048};   // hhea
    if (f == "felix titling") return {(1901.0 + 494) / 2048, 494.0 / 2048};   // hhea
    if (f == "footlight mt light") return {(1702.0 + 460) / 2048, 460.0 / 2048};   // win
    if (f == "forte") return {(1802.0 + 438 + 541) / 2048, 438.0 / 2048};   // hhea
    if (f == "freestyle script") return {(1742.0 + 670) / 2048, 670.0 / 2048};   // hhea
    if (f == "french script mt") return {1.160, 625.0 / 2048};   // measured (Publisher test page); nearest metric hhea = 1.140
    if (f == "gabriola") return {(2800.0 + 1296 + 2867) / 4096, 1296.0 / 4096};   // hhea
    if (f == "gigi") return {(1988.0 + 842) / 2048, 842.0 / 2048};   // hhea
    if (f == "gill sans ultra bold") return {(2036.0 + 516) / 2048, 516.0 / 2048};   // hhea
    if (f == "gill sans ultra bold condensed") return {(2036.0 + 514) / 2048, 514.0 / 2048};   // hhea
    if (f == "gloucester mt extra condensed") return {(1936.0 + 446) / 2048, 446.0 / 2048};   // hhea
    if (f == "goudy old style") return {(1832.0 + 486 + 140) / 2048, 486.0 / 2048};   // hhea
    if (f == "haettenschweiler") return {(1802.0 + 246 + 137) / 2048, 246.0 / 2048};   // hhea
    if (f == "harlow solid italic") return {(1791.0 + 795 + 1) / 2048, 795.0 / 2048};   // hhea
    if (f == "harrington") return {(1936.0 + 472) / 2048, 472.0 / 2048};   // hhea
    if (f == "high tower text") return {(1516.0 + 532 + 335) / 2048, 532.0 / 2048};   // hhea
    if (f == "impact") return {(1619.0 + 229 + 343) / 2048, 229.0 / 2048};   // typo
    if (f == "informal roman") return {(1946.0 + 512) / 2048, 512.0 / 2048};   // hhea
    if (f == "ink free") return {(910.0 + 328) / 1000, 328.0 / 1000};   // win
    if (f == "jokerman") return {(2368.0 + 729) / 2048, 729.0 / 2048};   // hhea
    if (f == "kristen itc") return {(2105.0 + 680) / 2048, 680.0 / 2048};   // hhea
    if (f == "kunstler script") return {1.120, 547.0 / 2048};   // measured (Publisher test page); nearest metric hhea = 1.094
    if (f == "lucida bright") return {(1980.0 + 432) / 2048, 432.0 / 2048};   // hhea
    if (f == "lucida calligraphy") return {(2122.0 + 666) / 2048, 666.0 / 2048};   // hhea
    if (f == "lucida console") return {(1616.0 + 432) / 2048, 432.0 / 2048};   // hhea
    if (f == "lucida fax") return {(1980.0 + 432) / 2048, 432.0 / 2048};   // hhea
    if (f == "lucida sans") return {(1980.0 + 432) / 2048, 432.0 / 2048};   // hhea
    if (f == "lucida sans unicode") return {(2246.0 + 901) / 2048, 901.0 / 2048};   // hhea
    if (f == "magneto") return {(1633.0 + 415 + 433) / 2048, 415.0 / 2048};   // hhea
    if (f == "matura mt script capitals") return {(1755.0 + 970) / 2048, 970.0 / 2048};   // hhea
    if (f == "microsoft sans serif") return {(1491.0 + 431 + 269) / 2048, 431.0 / 2048};   // typo
    if (f == "mistral") return {1.240, 647.0 / 2048};   // measured (Publisher test page); nearest metric win = 1.218
    if (f == "modern no. 20") return {1.088, 209.0 / 1000};   // measured (Publisher test page); nearest metric hhea = 1.070
    if (f == "monotype corsiva") return {(1618.0 + 620 + 60) / 2048, 620.0 / 2048};   // hhea
    if (f == "ms reference sans serif") return {(2059.0 + 430) / 2048, 430.0 / 2048};   // hhea
    if (f == "mv boli") return {(2333.0 + 967) / 2048, 967.0 / 2048};   // hhea
    if (f == "niagara engraved") return {(1792.0 + 401) / 2048, 401.0 / 2048};   // hhea
    if (f == "niagara solid") return {(1792.0 + 401) / 2048, 401.0 / 2048};   // hhea
    if (f == "old english text mt") return {(2054.0 + 442) / 2048, 442.0 / 2048};   // win
    if (f == "onyx") return {1.174, 442.0 / 2048};   // measured (Publisher test page); nearest metric win = 1.147
    if (f == "palace script mt") return {0.907, 496.0 / 2048};   // measured (Publisher test page); nearest metric hhea = 0.926
    if (f == "palatino linotype") return {(1499.0 + 582 + 682) / 2048, 582.0 / 2048};   // hhea
    if (f == "papyrus") return {(2155.0 + 1068) / 2048, 1068.0 / 2048};   // hhea
    if (f == "parchment") return {1.040, 651.0 / 2048};   // measured (Publisher test page); nearest metric win = 1.067
    if (f == "perpetua") return {(1679.0 + 668) / 2048, 668.0 / 2048};   // hhea
    if (f == "perpetua titling mt") return bold ? KnownMetrics{(1536.0 + 512 + 416) / 2048, 512.0 / 2048}
                                           : KnownMetrics{(1536.0 + 512 + 375) / 2048, 512.0 / 2048};   // hhea, bold differs
    if (f == "playbill") return {(1696.0 + 376) / 2048, 376.0 / 2048};   // win
    if (f == "poor richard") return {(1861.0 + 442) / 2048, 442.0 / 2048};   // win
    if (f == "pristina") return {(1770.0 + 915) / 2048, 915.0 / 2048};   // win
    if (f == "rage italic") return {(1733.0 + 840) / 2048, 840.0 / 2048};   // hhea
    if (f == "rockwell condensed") return {(1927.0 + 481) / 2048, 481.0 / 2048};   // hhea
    if (f == "rockwell extra bold") return {(1937.0 + 468) / 2048, 468.0 / 2048};   // hhea
    if (f == "script mt bold") return {(1950.0 + 516) / 2048, 516.0 / 2048};   // hhea
    if (f == "segoe print") return {(2555.0 + 1014 + 46) / 2048, 1014.0 / 2048};   // hhea
    if (f == "segoe script") return {(1526.0 + 494 + 171) / 2048, 494.0 / 2048};   // typo
    if (f == "segoe ui black") return {(2210.0 + 514) / 2048, 514.0 / 2048};   // hhea
    if (f == "segoe ui light") return {(1491.0 + 431 + 269) / 2048, 431.0 / 2048};   // typo
    if (f == "segoe ui semibold") return {(1491.0 + 431 + 269) / 2048, 431.0 / 2048};   // typo
    if (f == "segoe ui semilight") return {(1491.0 + 431 + 269) / 2048, 431.0 / 2048};   // typo
    if (f == "segoe ui variable display") return {(1491.0 + 431 + 269) / 2048, 431.0 / 2048};   // typo
    if (f == "segoe ui variable display light") return {(1491.0 + 431 + 269) / 2048, 431.0 / 2048};   // typo
    if (f == "segoe ui variable display semibold") return {(1491.0 + 431 + 269) / 2048, 431.0 / 2048};   // typo
    if (f == "segoe ui variable display semilight") return {(1491.0 + 431 + 269) / 2048, 431.0 / 2048};   // typo
    if (f == "segoe ui variable small") return {(1491.0 + 431 + 269) / 2048, 431.0 / 2048};   // typo
    if (f == "segoe ui variable small light") return {(1491.0 + 431 + 269) / 2048, 431.0 / 2048};   // typo
    if (f == "segoe ui variable small semibold") return {(1491.0 + 431 + 269) / 2048, 431.0 / 2048};   // typo
    if (f == "segoe ui variable small semilight") return {(1491.0 + 431 + 269) / 2048, 431.0 / 2048};   // typo
    if (f == "segoe ui variable text") return {(1491.0 + 431 + 269) / 2048, 431.0 / 2048};   // typo
    if (f == "segoe ui variable text light") return {(1491.0 + 431 + 269) / 2048, 431.0 / 2048};   // typo
    if (f == "segoe ui variable text semibold") return {(1491.0 + 431 + 269) / 2048, 431.0 / 2048};   // typo
    if (f == "segoe ui variable text semilight") return {(1491.0 + 431 + 269) / 2048, 431.0 / 2048};   // typo
    if (f == "showcard gothic") return {(1633.0 + 415 + 487) / 2048, 415.0 / 2048};   // hhea
    if (f == "snap itc") return {(2075.0 + 557) / 2048, 557.0 / 2048};   // hhea
    if (f == "stencil") return {(1860.0 + 567) / 2048, 567.0 / 2048};   // win
    if (f == "sylfaen") return {(1510.0 + 576 + 611) / 2048, 576.0 / 2048};   // hhea
    if (f == "tempus sans itc") return {(1974.0 + 698) / 2048, 698.0 / 2048};   // hhea
    if (f == "trebuchet ms") return {(1510.0 + 420) / 2048, 420.0 / 2048};   // typo
    if (f == "tw cen mt") return {(1753.0 + 477) / 2048, 477.0 / 2048};   // hhea
    if (f == "tw cen mt condensed extra bold") return {(1788.0 + 430) / 2048, 430.0 / 2048};   // hhea
    if (f == "viner hand itc") return {(2046.0 + 1260) / 2048, 1260.0 / 2048};   // hhea
    if (f == "vladimir script") return {(1843.0 + 639) / 2048, 639.0 / 2048};   // hhea
    if (f == "wide latin") return {(1975.0 + 546) / 2048, 546.0 / 2048};   // win
    return {};
}

// (Looked up by name for every line that is laid out, among the fifty or so fonts it knows: the answer for a font is kept.)
static KnownMetrics knownMetrics(const QString &family, bool bold = false)
{
    static QHash<QString, KnownMetrics> cache;
    const QString key = family + (bold ? QStringLiteral("|b") : QStringLiteral("|r"));
    const auto it = cache.constFind(key);
    if (it != cache.constEnd()) return it.value();
    const KnownMetrics m = knownMetricsOf(family, bold);
    if (cache.size() < 4096) cache.insert(key, m);
    return m;
}
static bool isSubstituted(const QString &family) { return !substituteFor(family).isEmpty() || substituteStretch(family) != 100; }
static QString requestedFamily(const QFont &f) { return f.families().isEmpty() ? f.family() : f.families().first(); }

// A cache key for metrics given in ems, which don't change with size: the
// font's families, weight, slant and stretch (QFont::key() includes the size,
// so every new size would read the font's tables again).
static QString emKey(const QFont &f)
{
    return f.families().join(QLatin1Char(',')) + QLatin1Char('|') + f.family() + QLatin1Char('|') + QString::number(f.weight()) + QLatin1Char('|') +
           QString::number(int(f.style())) + QLatin1Char('|') + QString::number(f.stretch());
}

// A font's average character width in ems (OS/2 xAvgCharWidth), the unit of
// .pub tracking. Substituted fonts use the original's, read from fonts
// embedded in reference PDFs.
static double averageCharEm(const QFont &f, const QString &requested)
{
    // A substituted font's tracking is the original's, measured: its
    // stand-in's letters differ (a squeezed stand-in's average is far wider
    // than an extra-condensed original's).
    if (isSubstituted(requested))
        if (const double avg = originalAverageCharEm(requested, f.weight() >= QFont::DemiBold, f.italic()); avg > 0) return avg;
    static QHash<QString, double> cache;
    const QString key = emKey(f);
    auto it = cache.constFind(key);
    if (it != cache.constEnd()) return *it;
    double em = 0.5;
    const QRawFont raw = QRawFont::fromFont(f);
    const QByteArray head = raw.fontTable("head"), os2 = raw.fontTable("OS/2");
    if (head.size() >= 20 && os2.size() >= 4) {
        const int upm = (uchar(head[18]) << 8) | uchar(head[19]);
        const int avg = qint16((uchar(os2[2]) << 8) | uchar(os2[3]));
        if (upm > 0 && avg > 0) em = double(avg) / upm;
    }
    if (!(em > 0.1 && em < 2.0)) em = 0.5;
    cache.insert(key, em);
    return em;
}

// The space (layout points) tracking adds after each letter: the font's
// average character width times the tracking beyond 100%. Measured against
// reference PDFs of .pub files at 87.5%, 112.5% and 115%.
static double trackingSpace(const QTextCharFormat &f, const QFont &resolved)
{
    const double t = tp::trackingOf(f);
    if (std::abs(t - 100) < 0.01) return 0;
    return (t - 100) / 100 * averageCharEm(resolved, requestedFamily(resolved)) * resolved.pointSizeF() / fontPointFactor();
}

double lineEmOf(const QFont &f, const QString &requestedFamily)
{
    // The known metrics are Publisher's own spacing for the font, so they
    // hold whether or not it's installed: the real Gill Sans MT's tables
    // give lines 8% tighter than Publisher's.
    const double known = knownMetrics(requestedFamily, f.weight() >= QFont::DemiBold).line;
    if (known > 0) return known;
    static QHash<QString, double> cache;
    const bool typo = usesTypoMetrics(requestedFamily);
    const QString key = emKey(f) + (typo ? QStringLiteral("|t") : QStringLiteral("|h"));
    auto it = cache.constFind(key);
    if (it != cache.constEnd()) return *it;
    double em = 1.15;
    const QRawFont raw = QRawFont::fromFont(f);
    auto be16 = [](const QByteArray &t, int off) { return off + 2 <= t.size() ? qint16((uchar(t[off]) << 8) | uchar(t[off + 1])) : qint16(0); };
    const QByteArray head = raw.fontTable("head"), os2 = raw.fontTable("OS/2"), hhea = raw.fontTable("hhea");
    const int upm = head.size() >= 20 ? quint16(be16(head, 18)) : 0;
    if (upm > 0) {
        if (typo && os2.size() >= 74) em = double(be16(os2, 68) - be16(os2, 70) + be16(os2, 72)) / upm;
        else if (hhea.size() >= 10) em = double(be16(hhea, 4) - be16(hhea, 6) + be16(hhea, 8)) / upm;
        else if (os2.size() >= 74) em = double(be16(os2, 68) - be16(os2, 70) + be16(os2, 72)) / upm;
    }
    if (!(em > 0.6 && em < 3.0)) em = 1.15;
    cache.insert(key, em);
    return em;
}

// A line's text for wrapping: its font's ascent and descent. A missing font's
// stand-in (not drawn to the same metrics) has taller or shorter ones: the
// original's line spacing counts where it is known.
static double wrapTextHeight(const QFont &f)
{
    const double own = heightOf(f);
    const QString fam = requestedFamily(f), standIn = substituteFor(fam);
    if (standIn.isEmpty() || interchangeFontName(standIn) != standIn) return own;
    const double known = knownMetrics(fam, f.weight() >= QFont::DemiBold).line;
    return known > 0 ? std::min(own, known * f.pointSizeF() / fontPointFactor()) : own;
}

// Single spacing in document points: the largest run's line height on the line.
double singleSpacing(const QVector<QTextLayout::FormatRange> &ranges, int from, int len, const QFont &fallback)
{
    auto lineOf = [](const QFont &f, double size) {
        const QString fam = f.families().isEmpty() ? f.family() : f.families().first();
        return size / fontPointFactor() * lineEmOf(f, fam);
    };
    double m = 0;
    for (const auto &r : ranges)
        if (r.start < from + len && r.start + r.length > from) {
            const QFont f = r.format.font();
            m = std::max(m, lineOf(f, r.format.hasProperty(tp::LineSize) ? r.format.property(tp::LineSize).toDouble() : f.pointSizeF()));
        }
    if (m <= 0) m = lineOf(fallback, fallback.pointSizeF());
    return m;
}

// A line as a word processor sets it, in document points: the text above the baseline, below it, and the leading the font asks for between lines.
struct WordLine {
    double ascent = 0, descent = 0, leading = 0;
    double total() const { return ascent + descent + leading; }
};

// The vertical metrics of the fonts a word processor's documents ask for, in font units: the Windows ascent and descent, then the ascent,
// descent, and line gap of the font's horizontal header. A font's line is its Windows ascent and descent, plus the part of its gap that
// they don't already hold (Calibri's gap is its Windows height less its header's: no leading). The faces the word processor's machine has
// are in the generated table (every face of each family: a bold or italic face can differ from the regular one); a few names it knows that
// the table has no face of are read from the tables embedded in reference PDFs, as the stand-ins' (Tinos, Arimo, Cousine) are the same for
// the three core fonts but not for the rest.
#include "wordfontlines.inc"

struct WordFontRow {
    const char *family;
    int winAscent, winDescent, ascent, descent, gap;
};
static const WordFontRow kWordFonts[] = {
    {"times", 1825, 443, 1825, 443, 87},     {"helvetica", 1854, 434, 1854, 434, 67}, {"courier", 1705, 615, 1705, 615, 0},
    {"century", 2019, 442, 2019, 443, 0},    {"monotype corsiva", 1618, 620, 1618, 620, 60}, {"broadway", 1869, 450, 1869, 450, 0},
    {"tw cen mt condensed extra bold", 1788, 430, 1788, 430, 0}, {"news gothic mt", 2003, 458, 2003, 458, 0}, {"univers", 2025, 512, 2025, 512, 0},
};

// The measured face a family asks for, by its style: the face itself, else what a word processor makes it from (a bold italic from the
// italic face, or from the bold one, else the regular face); null when the family has no measured face.
struct WordFace {
    int upm, winAscent, winDescent, ascent, descent, gap;
};
static bool wordFaceNamed(const QString &lower, bool bold, bool italic, WordFace *out)
{
    const int want[4] = {bold && italic ? 3 : bold ? 1 : italic ? 2 : 0, bold && italic ? 2 : 0, bold && italic ? 1 : 0, 0};
    for (int k = 0; k < 4; ++k) {
        if (k > 0 && want[k] == want[k - 1]) continue;
        for (const auto &r : kWordFontFaces)
            if (r.style == want[k] && lower == QLatin1String(r.family)) {
                *out = {r.upm, r.winAscent, r.winDescent, r.ascent, r.descent, r.gap};
                return true;
            }
    }
    return false;
}

static bool wordKnowsFamily(const QString &lower)
{
    // (Every family of the table and the few names beside it, made once: a family is asked about for every run of text.)
    static const QSet<QString> known = [] {
        QSet<QString> s;
        for (const auto &r : kWordFontFaces) s.insert(QLatin1String(r.family));
        for (const WordFontRow &r : kWordFonts) s.insert(QLatin1String(r.family));
        return s;
    }();
    return known.contains(lower);
}

// The family whose line a font keeps: the one the text asked for, which for Courier New drawn in its thin stand-in is the family after it in the font's list.
static QString metricFamilyOf(const QFont &f)
{
    const QStringList families = f.families();
    // (And for a run that shows the pictures of a symbol font the machine lacks, drawn in the run's own family: the font follows it in the list.)
    const bool pictures = families.size() >= 2 && isSymbolFont(families[1]) && !isSymbolFont(families.first()) && !hasFamilyCached(families[1]);
    return families.size() >= 2 && (families.first() == kThinCourier || pictures) ? families[1] : requestedFamily(f);
}

// A font's ascent, descent, and leading in ems, as the word processor lays a line out with it (by the family the text asked for): the one place a line is
// measured from a font, so the run, the mark, the number of a list item, and the paragraph after one all keep the line of the family they were set as.
static WordLine wordFaceOf(const QFont &f)
{
    static QHash<QString, WordLine> cache;   // main thread only, like all layout
    const QString key = emKey(f);
    auto it = cache.constFind(key);
    if (it != cache.constEnd()) return *it;
    auto make = [](double winA, double winD, double hA, double hD, double gap, double upm) {
        WordLine l;
        l.ascent = winA / upm;
        l.descent = winD / upm;
        l.leading = std::max(0.0, gap - ((winA + winD) - (hA + hD))) / upm;
        return l;
    };
    WordLine line = make(1825, 443, 1825, 443, 87, 2048);   // (a Times New Roman, when nothing is known)
    const QString family = metricFamilyOf(f).toLower();
    bool found = false;
    WordFace face;
    if (wordFaceNamed(family, f.weight() >= QFont::DemiBold, f.italic(), &face)) {
        line = make(face.winAscent, face.winDescent, face.ascent, face.descent, face.gap, face.upm);
        found = true;
    }
    if (!found)
        for (const WordFontRow &r : kWordFonts)
            if (family == QLatin1String(r.family)) {
                line = make(r.winAscent, r.winDescent, r.ascent, r.descent, r.gap, 2048);
                found = true;
                break;
            }
    // A family the word processor doesn't have on the machine is set in its stand-in, which is Calibri's line.
    if (!found && !hasFamilyCached(requestedFamily(f))) {
        line = make(1950, 550, 1536, 512, 452, 2048);
        found = true;
    }
    if (!found) {
        const QRawFont raw = QRawFont::fromFont(f);
        auto be16 = [](const QByteArray &t, int off) { return off + 2 <= t.size() ? qint16((uchar(t[off]) << 8) | uchar(t[off + 1])) : qint16(0); };
        const QByteArray head = raw.fontTable("head"), os2 = raw.fontTable("OS/2"), hhea = raw.fontTable("hhea");
        const int upm = head.size() >= 20 ? quint16(be16(head, 18)) : 0;
        if (upm > 0 && hhea.size() >= 10) {
            const double hA = be16(hhea, 4), hD = -be16(hhea, 6), gap = be16(hhea, 8);
            const bool win = os2.size() >= 78 && quint16(be16(os2, 74)) + quint16(be16(os2, 76)) > 0;
            const WordLine own = win ? make(quint16(be16(os2, 74)), quint16(be16(os2, 76)), hA, hD, gap, upm) : make(hA, hD, hA, hD, gap, upm);
            if (own.total() > 0.6 && own.total() < 3.0) line = own;
        }
    }
    cache.insert(key, line);
    return line;
}

// Two runs on one line: the line is that of the taller (a Symbol bullet's line is Symbol's own; a Courier bullet beside Times text makes none taller).
static WordLine mergeWord(const WordLine &a, const WordLine &b) { return a.total() >= b.total() ? a : b; }

// The line a stretch of text sets, from its runs. The spaces and tabs that end the line (`disp` is the paragraph's text) take no part in its
// height, as a word processor sets them; a line that shows nothing else is the paragraph mark's (`markFont`, at `markSize`), not the font of
// its blanks.
static WordLine wordLineOf(const QVector<QTextLayout::FormatRange> &ranges, int from, int len, const QFont &fallback, const LayoutEnv &env, const QString *disp = nullptr, const QFont *markFont = nullptr, double markSize = 0)
{
    WordLine m;
    auto add = [&](const QFont &f, double size) {
        const WordLine face = wordFaceOf(f);
        const double em = size / fontPointFactor();
        WordLine run;
        run.ascent = face.ascent * em;
        run.descent = face.descent * em;
        run.leading = env.wordNoLeading ? 0.0 : face.leading * em;
        m = m.total() > 0 ? mergeWord(m, run) : run;
    };
    int shown = from + len;   // (where the line's text ends, trailing spaces and tabs not counted)
    if (disp)
        while (shown > from && shown <= disp->size() && (disp->at(shown - 1) == QLatin1Char(' ') || disp->at(shown - 1) == QLatin1Char('\t'))) --shown;
    for (const auto &r : ranges)
        if (r.length > 0 && r.start < shown && r.start + r.length > from) {   // (a mark that shows nothing is no run)
            // A tab sets no height, whatever size it is set in and wherever it stands (the tab that indents a first line, in the size the paragraph's last edit left
            // it), nor does a run of one space (a lone space between runs in the paragraph's own font, or before text of another size): the line is the text's.
            // A longer run of spaces is as tall as its font makes it, as it is in a line of its own.
            if (disp) {
                const int a = std::max(r.start, from), b = std::min(r.start + r.length, shown);
                bool onlyBlanks = true, onlyTabs = true;
                for (int i = a; i < b && onlyBlanks; ++i) {
                    const QChar c = i < disp->size() ? disp->at(i) : QLatin1Char('x');
                    onlyBlanks = c == QLatin1Char('\t') || c == QLatin1Char(' ');
                    onlyTabs = onlyTabs && c == QLatin1Char('\t');
                }
                if (onlyTabs || (onlyBlanks && b - a < 2)) continue;
            }
            const QFont f = r.format.font();
            add(f, r.format.hasProperty(tp::LineSize) ? r.format.property(tp::LineSize).toDouble() : f.pointSizeF());
        }
    if (m.total() <= 0 && markFont) add(*markFont, markSize);
    if (m.total() <= 0) add(fallback, fallback.pointSizeF());
    // An older word processor's spacing: the line is of whole points, the leading and what is added to reach them going above the text.
    if (env.wordWholePointLines) {
        m.ascent = std::ceil(m.total() - 1e-6) - m.descent;
        m.leading = 0;
    }
    return m;
}

// A slide's single line: 1.2 times the largest text on it, whatever the font.
double slideSingle(const QVector<QTextLayout::FormatRange> &ranges, int from, int len, const QFont &fallback)
{
    double m = 0;
    for (const auto &r : ranges)
        if (r.start < from + len && r.start + r.length > from)
            m = std::max(m, (r.format.hasProperty(tp::LineSize) ? r.format.property(tp::LineSize).toDouble() : r.format.font().pointSizeF()) / fontPointFactor() * 1.2);
    if (m <= 0) m = fallback.pointSizeF() / fontPointFactor() * 1.2;
    return m;
}

// Descent (document points) of the proprietary fonts with known metrics on a
// line (installed or not), or 0 when none is known; the baseline sits this far above the line's bottom.
// Single spaced text up to 31 pt sits higher in some fonts' lines (KnownMetrics::lift).
double knownDescent(const QVector<QTextLayout::FormatRange> &ranges, int from, int len, bool fullSize, bool single)
{
    double d = 0;
    for (const auto &r : ranges)
        if (r.start < from + len && r.start + r.length > from) {
            const QFont f = r.format.font();
            const QString fam = requestedFamily(f);
            const KnownMetrics km = knownMetrics(fam, f.weight() >= QFont::DemiBold);
            const double size = fullSize && r.format.hasProperty(tp::LineSize) ? r.format.property(tp::LineSize).toDouble() : f.pointSizeF();
            if (km.descent > 0) d = std::max(d, size / fontPointFactor() * (km.descent + (single && size / fontPointFactor() < 32 ? km.lift : 0)));
        }
    return d;
}

// The line's descent in document points from the fonts' own metrics, each
// run as drawn or (fullSize) small capitals at the full size.
double fontDescent(const QVector<QTextLayout::FormatRange> &ranges, int from, int len, bool fullSize)
{
    static QHash<QString, double> emCache;   // main thread only, like all layout
    double d = 0;
    for (const auto &r : ranges)
        if (r.start < from + len && r.start + r.length > from) {
            QFont f = r.format.font();
            const QString key = emKey(f);
            auto it = emCache.constFind(key);
            if (it == emCache.constEnd()) {
                f.setPixelSize(1000);
                it = emCache.insert(key, QFontMetricsF(f).descent() / 1000);
            }
            const double size = fullSize && r.format.hasProperty(tp::LineSize) ? r.format.property(tp::LineSize).toDouble() : r.format.fontPointSize();
            d = std::max(d, *it * size / fontPointFactor());
        }
    return d;
}

// A font's ascent and descent in ems by its Windows metrics (OS/2 usWinAscent and usWinDescent; the hhea ones where it has no OS/2 table).
struct WinMetrics { double ascent = 0, descent = 0; };

// Those of the fonts the presentation program drew with in reference PDFs (read from the OS/2 tables of the fonts embedded in them), for a
// stand-in whose own differ: a style is the regular face's unless named.
static WinMetrics presentationMetrics(const QString &family, bool bold, bool italic)
{
    struct Entry { const char *family; double regular[2], bold[2], boldItalic[2]; };   // ascent, descent (units of 2048; 0: as the regular face)
    static const Entry table[] = {
        {"arial", {1854, 434}, {0, 0}, {0, 0}},
        {"arial black", {2254, 634}, {0, 0}, {0, 0}},
        {"arial narrow", {1888, 431}, {1910, 431}, {0, 0}},
        {"book antiqua", {1967, 578}, {1891, 543}, {1888, 575}},
        {"bookman old style", {1831, 473}, {0, 0}, {0, 0}},
        {"calibri", {1950, 550}, {0, 0}, {0, 0}},
        {"calibri light", {1950, 550}, {0, 0}, {0, 0}},
        {"century gothic", {1989, 451}, {2032, 451}, {0, 0}},
        {"consolas", {1884, 514}, {0, 0}, {0, 0}},
        {"corbel", {1950, 550}, {0, 0}, {0, 0}},
        {"franklin gothic book", {1877, 445}, {0, 0}, {0, 0}},
        {"garamond", {1765, 539}, {0, 0}, {0, 0}},
        {"gill sans mt", {1903, 472}, {0, 0}, {0, 0}},
        {"impact", {2066, 432}, {0, 0}, {0, 0}},
        {"perpetua", {1679, 668}, {0, 0}, {0, 0}},
        {"script mt bold", {1950, 516}, {0, 0}, {0, 0}},
        {"times", {1825, 443}, {0, 0}, {0, 0}},
        {"times new roman", {1825, 443}, {0, 0}, {0, 0}},
        {"trebuchet ms", {1923, 455}, {0, 0}, {0, 0}},
        {"tw cen mt", {1753, 477}, {0, 0}, {0, 0}},
        {"tw cen mt condensed", {1694, 494}, {0, 0}, {0, 0}},
        {"verdana", {2059, 430}, {0, 0}, {0, 0}},
    };
    for (const Entry &e : table)
        if (family.compare(QLatin1String(e.family), Qt::CaseInsensitive) == 0) {
            const double *v = bold && italic && e.boldItalic[0] > 0 ? e.boldItalic : bold && e.bold[0] > 0 ? e.bold : e.regular;
            return {v[0] / 2048, v[1] / 2048};
        }
    return {};
}

static WinMetrics drawnMetrics(const QFont &f)
{
    static QHash<QString, WinMetrics> cache;   // main thread only, like all layout
    const QString key = emKey(f);
    auto it = cache.constFind(key);
    if (it != cache.constEnd()) return *it;
    WinMetrics m;
    const QRawFont raw = QRawFont::fromFont(f);
    auto be16 = [](const QByteArray &t, int off) { return off + 2 <= t.size() ? (uchar(t[off]) << 8) | uchar(t[off + 1]) : 0; };
    const QByteArray head = raw.fontTable("head"), os2 = raw.fontTable("OS/2"), hhea = raw.fontTable("hhea");
    const int upm = head.size() >= 20 ? be16(head, 18) : 0;
    if (upm > 0) {
        if (os2.size() >= 78 && be16(os2, 74) + be16(os2, 76) > 0) m = {double(be16(os2, 74)) / upm, double(be16(os2, 76)) / upm};
        else if (hhea.size() >= 8) m = {double(qint16(be16(hhea, 4))) / upm, -double(qint16(be16(hhea, 6))) / upm};
    }
    if (!(m.ascent > 0 && m.descent > 0 && m.ascent + m.descent < 3.0)) m = {};
    cache.insert(key, m);
    return m;
}

// A slide's line: the presentation program makes a single line 1.2 em tall and
// shares that between above and below the baseline in the proportion of the
// font's Windows ascent to its descent (checked in 1,000 first lines of its
// PDFs: Arial 0.974 em from the top of a line to the baseline, Times New Roman
// 0.966, Calibri 0.936, Book Antiqua 0.928, Gill Sans MT 0.961). This is the
// largest descent, below the baseline, among the line's text.
double slideDescent(const QVector<QTextLayout::FormatRange> &ranges, int from, int len, const QFont &fallback)
{
    static QHash<QString, WinMetrics> cache;   // main thread only, like all layout
    auto descentOf = [](const QFont &f, double size) {
        const QString family = requestedFamily(f);
        const bool bold = f.weight() >= QFont::DemiBold;
        const QString key = emKey(f) + QLatin1Char('|') + family;
        auto it = cache.constFind(key);
        if (it == cache.constEnd()) {
            WinMetrics m = presentationMetrics(family, bold, f.italic());
            // A font on the computer is its own; a stand-in's tables are not the original's (its line is as before).
            if (m.ascent <= 0 && hasFamilyCached(family)) m = drawnMetrics(f);
            it = cache.insert(key, m);
        }
        return it->ascent > 0 ? size / fontPointFactor() * 1.2 * it->descent / (it->ascent + it->descent) : 0.0;
    };
    double d = 0;
    for (const auto &r : ranges)
        if (r.start < from + len && r.start + r.length > from) {
            const QFont f = r.format.font();
            d = std::max(d, descentOf(f, r.format.hasProperty(tp::LineSize) ? r.format.property(tp::LineSize).toDouble() : f.pointSizeF()));
        }
    return d > 0 ? d : descentOf(fallback, fallback.pointSizeF());
}

// Line spacing in "spaces" (sp) scales the single spacing; points are absolute.
double lineHeightFor(const QTextBlockFormat &bf, double scale, double single)
{
    switch (bf.lineHeightType()) {
    case QTextBlockFormat::ProportionalHeight: return single * bf.lineHeight() / 100.0;
    case QTextBlockFormat::FixedHeight: return bf.lineHeight() * scale;
    case QTextBlockFormat::MinimumHeight: return std::max(single, bf.lineHeight() * scale);
    case QTextBlockFormat::LineDistanceHeight: return single + bf.lineHeight() * scale;
    default: return single;
    }
}
} // namespace

#include "fontaverages.inc"

double originalAverageCharEm(const QString &family, bool bold, bool italic)
{
    static const QHash<QString, double> table = [] {
        QHash<QString, double> t;
        for (const auto &e : kPublisherAverageCharEm) t.insert(QString::fromUtf8(e.family).toLower() + QLatin1Char('|') + QString::number(e.style), e.avg);
        return t;
    }();
    const QString fam = family.toLower() + QLatin1Char('|');
    const int style = (bold ? 1 : 0) + (italic ? 2 : 0);
    // The face asked for, else the nearest the family has (one-face
    // families such as Gill Sans MT Ext Condensed Bold).
    for (int s : {style, style & 1, style & 2, 0})
        if (auto it = table.constFind(fam + QString::number(s)); it != table.constEnd()) return *it;
    return -1;
}

// Each letter of a run in a missing, measured stock font gets the original's
// width: the difference from its stand-in's goes into the letter's spacing,
// so a run is cut into pieces of (near) equal difference. Spaces keep theirs
// (word spacing sets those). A stand-in already within 0.4% of the original on
// letters and digits (Arimo for Arial) is left whole.
static QVector<QTextLayout::FormatRange> applyLetterWidthsTo(const QVector<QTextLayout::FormatRange> &in, const QString &disp, bool *adjusted)
{
    QVector<QTextLayout::FormatRange> out;
    out.reserve(in.size());
    static QHash<QString, double> advCache;     // stand-in advance per em, by font and letter (main thread only, like all layout)
    static QHash<QString, bool> needed;         // whether a font pairing needs it (likewise)
    for (const auto &r : in) {
        const QString key = r.format.stringProperty(kLetterWidths);
        if (key.isEmpty() || r.length <= 0) { out << r; continue; }
        const QStringList k = key.split(QLatin1Char('|'));
        const bool ofSymbols = isSymbolFont(k.value(0)) && !hasFamilyCached(k.value(0));
        const QHash<char16_t, double> *table = ofSymbols ? symbolFontLetterWidths(k.value(0))
                                               : originalLetterWidths(k.value(0), k.value(1) == QLatin1String("1"), k.value(2) == QLatin1String("1"));
        QFont plain = r.format.font();
        if (!table || (plain.letterSpacingType() == QFont::PercentageSpacing && plain.letterSpacing() != 0 && plain.letterSpacing() != 100)) { out << r; continue; }
        const double base = plain.letterSpacingType() == QFont::AbsoluteSpacing ? plain.letterSpacing() : 0;
        plain.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        plain.setWordSpacing(0);
        // Letters are measured as typed, whatever the run's capitals: a small
        // capitals run's advances would be kept under the same font as the
        // normal runs', which are set by them from then on.
        plain.setCapitalization(QFont::MixedCase);
        const double em = plain.pointSizeF() / fontPointFactor();
        if (!(em > 0)) { out << r; continue; }
        const QString fk = emKey(plain) + QLatin1Char('|');
        auto adv = [&](QChar c) {
            const QString ck = fk + c;
            auto it = advCache.constFind(ck);
            if (it != advCache.constEnd()) return *it * em;
            QFont unit = plain;
            unit.setPointSizeF(100 * fontPointFactor());   // advances scale with size (unhinted)
            const double v = advanceOf(unit, QString(c)) / 100;
            advCache.insert(ck, v);
            return v * em;
        };
        const QString nk = key + QLatin1Char('|') + fk;
        auto nit = needed.constFind(nk);
        if (nit == needed.constEnd()) {
            double worst = 0;
            for (char16_t c : u"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")
                if (auto t = table->constFind(c); t != table->constEnd()) worst = std::max(worst, std::abs(*t - adv(QChar(c)) / em));
            nit = needed.insert(nk, ofSymbols || worst > 0.004);   // (a symbol font's pictures are none of the letters and digits the stand-in is compared by)
        }
        const auto space = table->constFind(u' ');
        // Kerning (from the size it applies at) is the original's pairs: a
        // font measured to have none doesn't kern, whatever its stand-in does.
        const bool kerns = plain.kerning();
        const QHash<quint32, double> *kerning = kerns ? originalKerning(k.value(0), k.value(1) == QLatin1String("1"), k.value(2) == QLatin1String("1")) : nullptr;
        if (!*nit) {
            // Letters alike: only the space, by word spacing.
            QTextLayout::FormatRange x = r;
            if (space != table->constEnd()) x.format.setFontWordSpacing(r.format.fontWordSpacing() + *space * em - adv(QLatin1Char(' ')));
            if (kerns && !kerning) x.format.setFontKerning(false);
            out << x;
            continue;
        }
        if (adjusted) *adjusted = true;
        // Each stand-in letter sits centered in the original's width, the
        // difference split before and after it, so a narrower letter leaves
        // even gaps on both sides (the first letter of a run starts where
        // the original's does). Spaces take the original's width too, by
        // letter spacing: word spacing does nothing on macOS. want[j]: the
        // space after character j.
        const int end = std::min(r.start + r.length, int(disp.size()));
        auto transparent = [](QChar c) { return c.unicode() == 0x00AD || c.unicode() == 0x200B || c.unicode() == 0x2060 || c.unicode() == 0xFFFC; };
        QVector<double> diff(end - r.start, 0.0), shift(end - r.start, 0.0), want(end - r.start, 0.0);
        bool first = true;
        for (int i = r.start; i < end; ++i) {
            const QChar c = disp[i];
            if (transparent(c)) continue;
            if (auto t = table->constFind(c.unicode()); t != table->constEnd()) diff[i - r.start] = *t * em - adv(c);
            if (!first && !c.isSpace()) shift[i - r.start] = diff[i - r.start] / 2;
            first = false;
        }
        // Its pairs are taken into the spacing; the pieces don't kern by the stand-in's.
        for (int i = r.start, next = -1; i < end; ++i) {
            if (transparent(disp[i])) continue;
            next = i + 1;
            while (next < end && transparent(disp[next])) ++next;
            want[i - r.start] = diff[i - r.start] - shift[i - r.start] + (next < end ? shift[next - r.start] : 0);
            if (kerning && next < end) want[i - r.start] += kerning->value(quint32(disp[i].unicode()) << 16 | disp[next].unicode()) * em;
        }
        // A piece is shaped alone, so a cut may fall only between two
        // characters the original's table measures, neither a surrogate, a
        // combining mark, a joiner or variation selector, nor a letter that
        // joins its neighbors: any other cut splits an emoji, parts an accent
        // from its letter, or draws a joined letter on its own. The rest ride
        // in a piece with their neighbors, at the stand-in's own widths.
        auto cuttable = [&](QChar c) {
            const char16_t u = c.unicode();
            return table->contains(u) && !c.isSurrogate() && !c.isMark() && u != 0x200C && u != 0x200D && !(u >= 0xFE00 && u <= 0xFE0F) &&
                   c.joiningType() == QChar::Joining_None;
        };
        int pieceStart = r.start;
        double pieceDelta = 0, drift = 0;   // drift: how far the letters so far sit from their places
        bool started = false;
        auto flush = [&](int to) {
            if (to <= pieceStart) return;
            auto emit = [&](int from, int upTo, double spacing) {
                if (upTo <= from) return;
                QTextLayout::FormatRange x = r;
                x.start = from;
                x.length = upTo - from;
                x.format.setFontLetterSpacingType(QFont::AbsoluteSpacing);
                x.format.setFontLetterSpacing(spacing);
                if (kerns) x.format.setFontKerning(false);
                out << x;
            };
            // A word joiner (set after a slash, so that no line breaks there) is no letter, and takes no spacing, where it would be a gap.
            int from = pieceStart;
            for (int t = pieceStart; t < to; ++t)
                if (disp[t] == QChar(0x2060)) {
                    emit(from, t, base + pieceDelta);
                    emit(t, t + 1, 0);
                    from = t + 1;
                }
            emit(from, to, base + pieceDelta);
        };
        for (int i = r.start, prev = -1; i < end; ++i) {
            if (transparent(disp[i])) continue;
            const double w = want[i - r.start];
            const int before = prev;
            prev = i;
            // A character joins the piece while every one stays within
            // 0.001 em of its place, or where no cut may fall; a new piece
            // takes up the drift.
            if (started && (std::abs(drift + pieceDelta - w) <= 0.001 * em || !cuttable(disp[before]) || !cuttable(disp[i]))) {
                drift += pieceDelta - w;
                continue;
            }
            if (started) {
                flush(i);
                pieceStart = i;
            }
            pieceDelta = w - drift;
            drift = 0;
            started = true;
        }
        flush(end);
    }
    return out;
}

QVector<QTextLayout::FormatRange> applyLetterWidths(const QVector<QTextLayout::FormatRange> &in, const QString &disp) { return applyLetterWidthsTo(in, disp, nullptr); }

double naturalLineEm(const QFont &f, const QString &requestedFamily) { return lineEmOf(f, requestedFamily); }
qint64 spaceShrinkWorkForTest(int what) { return what == 0 ? s_shrinkLooked : what == 1 ? s_shrinkPasses : what == 2 ? s_shrinkRelaid : what == 3 ? s_shrinkShared : s_shrinkFull; }

static quint64 nextBuildSerial()
{
    static std::atomic<quint64> builds{0};
    return ++builds;
}

void StoryLayout::build(const QTextDocument *doc, const QVector<FrameSpec> &frames, const LayoutEnv &env, const Span &span)
{
    m_serial = nextBuildSerial();
    m_span = span;
    buildOnce(doc, frames, env);
    m_frames = frames;
    m_span = Span();
}

void StoryLayout::build(const QTextDocument *doc, const QVector<FrameSpec> &frames, const LayoutEnv &env)
{
    m_serial = nextBuildSerial();
    m_span = Span();
    // Text centered or at the bottom of a polygon is set as far down as it fits there: laid out from a
    // top inset, with the polygon where it is, which can leave it above the middle or the bottom where
    // the polygon narrows.
    if (frames.size() == 1 && frames[0].fitToShape && frames[0].valign != VAlign::Top && frames[0].columns <= 1) {
        FrameSpec top = frames[0];
        top.valign = VAlign::Top;
        const double room = std::max(0.0, top.size.height() - top.insets.top() - top.insets.bottom());
        // Lays the text out from t down; whether it all fits, and how tall it is.
        auto at = [&](double t, double *height) {
            FrameSpec g = top;
            g.insets.setTop(top.insets.top() + t);
            buildOnce(doc, {g}, env);
            *height = m_used.value(0) - g.insets.top() - g.insets.bottom();
            return !m_overflow && *height > 0 && t + *height <= room + 0.5;
        };
        double t = 0, h = 0;
        if (at(0, &h)) {
            // The middle or the bottom for what the text takes: a few rounds, as its height depends on where it is.
            const double share = frames[0].valign == VAlign::Middle ? 0.5 : 1.0;
            for (int i = 0; i < 4; ++i) {
                const double next = std::max(0.0, (room - h) * share);
                double nh = 0;
                if (at(next, &nh)) {
                    t = next;
                    h = nh;
                    continue;
                }
                // Not there: the lowest place before it where it fits.
                double lo = t, hi = next;
                for (int k = 0; k < 14; ++k) {
                    const double mid = (lo + hi) / 2;
                    (at(mid, &nh) ? lo : hi) = mid;
                }
                t = lo;
                break;
            }
            at(t, &h);
        }
        FrameSpec g = top;
        g.insets.setTop(top.insets.top() + t);
        m_frames = {g};
        return;
    }
    QVector<double> shifts = buildOnce(doc, frames, env);
    // (Only blank paragraphs run over: the text is set in its own frame, and moved by the height they add.)
    const bool onlyBlankRunsOver = !m_overflow && m_blankPast;
    if ((m_overflow || m_blankPast) && frames.size() == 1 && frames[0].runsOver && frames[0].columns <= 1 && frames[0].obstacles.isEmpty() && !frames[0].fitToShape) {
        // Set from the top in a frame that holds it all, then moved as far up
        // as its alignment says for the frame it has.
        FrameSpec tall = frames[0];
        tall.valign = VAlign::Top;
        tall.size.setHeight(std::max(tall.size.height(), 100000.0));
        buildOnce(doc, {tall}, env);
        double top = 1e18, bottom = -1e18;
        for (const auto &B : m_blocks)
            for (const Line &l : B->lines)
                if (l.frame == 0) {
                    top = std::min(top, l.rect.top());
                    bottom = std::max(bottom, l.rect.bottom() - l.below);
                }
        const double room = frames[0].size.height() - frames[0].insets.top() - frames[0].insets.bottom();
        const double share = frames[0].valign == VAlign::Middle ? 0.5 : frames[0].valign == VAlign::Bottom ? 1.0 : 0.0;
        const double shift = bottom >= top ? (room - (bottom - frames[0].insets.top())) * share : 0;
        if (onlyBlankRunsOver) {
            // The text as the frame sets it, from the top, with the blank paragraphs past its end (no overflow, and not among its lines).
            FrameSpec own = frames[0];
            own.valign = VAlign::Top;
            buildOnce(doc, {own}, env);
        }
        if (shift != 0)
            for (auto &B : m_blocks)
                for (int i = 0; i < B->lines.size(); ++i)
                    if (B->lines[i].frame == 0) {
                        PtLine tl(B->tl->lineAt(i));
                        tl.setPosition(tl.position() + QPointF(0, shift));
                        B->lines[i].rect.translate(0, shift);
                    }
        if (!m_used.isEmpty()) m_used[0] += shift;
        m_frames = frames;
        return;
    }
    // Text centered or set at the bottom meets the objects it wraps around
    // where it ends up, not at the frame's top (as in Publisher: a centered
    // line just below a picture stays whole). Lay it out again with the
    // objects moved up by that shift, until the shift settles.
    QVector<double> applied(frames.size(), 0.0);
    for (int pass = 0; pass < 3; ++pass) {
        bool again = false;
        QVector<FrameSpec> moved = frames;
        for (int fi = 0; fi < frames.size(); ++fi) {
            if (frames[fi].obstacles.isEmpty() || frames[fi].valign == VAlign::Top || frames[fi].columns > 1) continue;
            const double s = fi < shifts.size() ? shifts[fi] : 0;
            if (std::abs(s - applied[fi]) > 0.25) {
                applied[fi] = s;
                again = true;
            }
            for (QPolygonF &poly : moved[fi].obstacles) poly.translate(0, -applied[fi]);
        }
        if (!again) break;
        shifts = buildOnce(doc, moved, env);
    }
    m_frames = frames;
}

QVector<double> StoryLayout::buildOnce(const QTextDocument *doc, const QVector<FrameSpec> &frames, const LayoutEnv &env)
{
    QVector<double> shifts(frames.size(), 0.0);
    m_blocks.clear();
    m_frames = frames;
    m_env = env;
    m_overflow = false;
    m_blankPast = false;
    m_windowShort = false;
    m_used = QVector<double>(frames.size(), 0.0);
    // The distances in the text (indents, tab stops, space before and after, an exact line spacing) shrink with its size, except a slide's
    // (see LayoutEnv::slideLines): there only the text itself does.
    const double scale = env.slideLines ? 1.0 : env.fontScale;
    const int nF = frames.size();
    auto singleOf = [&](const QVector<QTextLayout::FormatRange> &ranges, int from, int len, const QFont &fallback) {
        return env.slideLines ? slideSingle(ranges, from, len, fallback) : env.wordLines ? wordLineOf(ranges, from, len, fallback, env).total() : singleSpacing(ranges, from, len, fallback);
    };

    auto colRect = [&](int f, int c) {
        const FrameSpec &fs = frames[f];
        const QRectF content(fs.insets.left(), fs.insets.top(), std::max(1.0, fs.size.width() - fs.insets.left() - fs.insets.right()),
                             std::max(1.0, fs.size.height() - fs.insets.top() - fs.insets.bottom()));
        const int n = std::max(1, fs.columns);
        const double cw = std::max(1.0, (content.width() - fs.gap * (n - 1)) / n);
        return QRectF(content.left() + c * (cw + fs.gap), content.top(), cw, content.height());
    };

    // Each frame's lines sit below the frames before it in the paragraph
    // layouts (fine units must stay within Qt's fixed-point range).
    m_frameY = QVector<double>(nF + 1, 0.0);
    for (int i = 0; i < nF; ++i) m_frameY[i + 1] = m_frameY[i] + std::max(1.0, frames[i].size.height()) + 1000;

    int f = 0, c = 0;
    double y = 0;              // relative to column top
    double lastAfter = 0;      // the space after the paragraph before, which a word processor's space before counts against
    double lastRoom = 0;       // the room under the paragraph before: the lower edge of its box (borders, shading), and the space after
    double overflowY = 0;
    QVector<Iv> row;
    int rowIdx = 0;
    double rowH = 0;
    bool rowActive = false;
    double rowPushed = 0;      // how far down the row was moved by the objects the text goes round (of the line it begins, and every piece of it)

    bool columnEmpty = true;   // no line placed yet in the current column
    if (m_span.startY > 0.001) {   // (a stretch that goes below something in its first frame)
        y = m_span.startY;
        columnEmpty = false;
    }
    const bool hyphenate = !frames.isEmpty() && frames.first().hyphenate;
    const double zone = (frames.isEmpty() ? 18.0 : frames.first().hyphenZone) * env.fontScale;

    // Notes. A note's own text is laid out without notes of its own.
    m_notes.clear();
    m_noteRules.clear();
    m_inline.clear();
    m_notesHeading = Heading();
    static thread_local int noteDepth = 0;
    const bool withNotes = noteDepth == 0 && nF > 0;
    const FieldSource *notesDoc = nF > 0 ? frames.first().ctx.doc : nullptr;
    int footCount = m_span.footnotesBefore + (notesDoc ? notesDoc->footnotesBefore() : 0), endCount = m_span.endnotesBefore + (notesDoc ? notesDoc->endnotesBefore() : 0);
    QHash<int, double> reserve;   // a column's room for footnotes, by frame * 64 + column
    auto rkey = [](int fi, int ci) { return fi * 64 + ci; };
    for (int k = 0; k < m_span.noteReserves.size() && k < nF; ++k)
        if (m_span.noteReserves[k] > 0) reserve[rkey(k, 0)] = m_span.noteReserves[k];
    QVector<int> carry;           // footnotes (in m_notes) with text left for the next column, in order
    QVector<int> seeded;          // those a stretch is given, until the lines that came before its first frame are done
    // (A word processor's notes follow a paragraph's own height under the text's last line and have no gap but their own spacing; their numbers begin their text.)
    const double roomLimit = frames.isEmpty() ? 0.0 : frames.first().noteRoomLimit;
    auto limited = [&](double h) { return roomLimit > 0 ? std::min(h, roomLimit) : h; };
    const double separatorH = env.wordLines && notesDoc ? limited(notesDoc->noteSeparatorHeight()) : 0.0;
    // (A word processor keeps the room of the file's continuation notice under the last note: the gap above the notes holds it, and they are set that much higher.)
    const double noticeGap = (env.wordLines && notesDoc ? limited(notesDoc->noteNoticeHeight()) : 0.0) * scale;
    const double ruleGap = (separatorH > 0 ? separatorH : 9) * scale + noticeGap, noteGap = env.wordLines ? 0.0 : 2 * scale;
    m_ruleGap = ruleGap;
    m_noticeGap = noticeGap;
    m_noteGap = noteGap;
    // (The part of a note that goes on from the column before has the gap of the file's own separator for it, when the file has one: a paragraph with its text and rule.)
    const double continuationH = env.wordLines && notesDoc ? limited(notesDoc->noteContinuationHeight()) : 0.0;
    const double continuedGap = continuationH > 0 ? continuationH * scale + noticeGap : ruleGap;
    m_continuedGap = continuedGap;
    QHash<int, double> gapOf;   // the gap above the notes of a column that begins with the part of a note from the column before, by frame * 64 + column
    auto noteFrame = [&](const Note &n, double width, double height) {
        FrameSpec fs = frames.first();
        fs.size = QSizeF(width, height);
        fs.insets = QMarginsF(n.numberWidth, 0, 0, 0);
        fs.ctx.noteMark = n.number > 0 ? QString::number(n.number) : QString();   // (a note of a custom mark has no number: the mark its text begins with shows nothing)
        fs.columns = 1;
        fs.valign = VAlign::Top;
        fs.obstacles.clear();
        fs.baselineGrid = 0;
        return fs;
    };
    auto measureNote = [&](Note &n, double width, bool whole = true) {
        if (n.layout && std::abs(n.width - width) < 0.01) return n.height;
        if (!whole && n.numberFont.pointSizeF() > 0 && std::abs(n.width - width) < 0.01) return n.height;
        n.width = width;
        if (whole) n.layout = std::make_shared<StoryLayout>();
        const QTextDocument *nd = notesDoc ? notesDoc->storyDoc(n.storyId) : nullptr;
        QTextCharFormat first;
        if (nd) {
            const QTextBlock b0 = nd->begin();
            first = b0.begin().atEnd() ? b0.charFormat() : b0.begin().fragment().charFormat();
        }
        const QTextCharFormat rf = resolveCharFormat(first, env);
        n.numberFont = rf.font();
        n.numberColor = rf.foreground().color();
        n.numberWidth = env.wordLines ? 0.0 : advanceOf(n.numberFont, QStringLiteral("00.")) + 3 * scale;
        const FrameSpec fs = noteFrame(n, width, 100000);
        if (!whole) {
            // (Only what the note is set with, not the whole of its text: for the rest of one that goes on, whose pieces are laid out as they are placed.)
            n.height = heightOf(n.numberFont);
            n.firstBaseline = n.height * 0.8;
        } else if (nd) {
            ++noteDepth;
            n.layout->build(nd, {fs}, env);
            --noteDepth;
            const auto li = n.layout->lineInfo(0);
            n.firstBaseline = li.isEmpty() ? heightOf(n.numberFont) * 0.8 : li.first().baseline;
            n.height = std::max(heightOf(n.numberFont), n.layout->usedHeight(0) + n.layout->trailingSpace());
        } else {
            n.height = heightOf(n.numberFont);
            n.firstBaseline = n.height * 0.8;
        }
        return n.height;
    };
    // A note's text from `from` on, laid out for a column `width` wide with
    // `room` to spare: what fits, and in `rest` where the rest starts in the
    // note's text (-1: all of it fits). A line always fits.
    auto cutNote = [&](const Note &n, int from, double width, double room, int *rest) {
        auto lay = std::make_shared<StoryLayout>();
        *rest = -1;
        const QTextDocument *nd = notesDoc ? notesDoc->storyDoc(n.storyId) : nullptr;
        if (!nd) return lay;
        // (A note of many thousand characters is laid out as much as the room holds, and that much more if it did not fill it: a page would otherwise lay out all that is left of it.)
        const int total = nd->characterCount() - 1;
        int window = total - from > 4000 ? std::clamp(int(room * width / 24) + 500, 2000, 1000000) : 0;
        for (;;) {
            std::unique_ptr<QTextDocument> tail;
            if (from > 0 || window > 0) {
                // The text from there on as a story of its own (the part of it that is wanted); a paragraph cut in the middle has no first-line indent.
                if (!(window > 0 && from + window < total)) window = 0;
                const int to = window > 0 ? from + window : total;
                tail = std::make_unique<QTextDocument>();
                tail->setDocumentMargin(nd->documentMargin());
                tail->setDefaultFont(nd->defaultFont());
                tail->setDefaultTextOption(nd->defaultTextOption());
                {
                    QTextCursor src(const_cast<QTextDocument *>(nd));
                    src.setPosition(from);
                    src.setPosition(std::min(to, total), QTextCursor::KeepAnchor);
                    QTextCursor(tail.get()).insertFragment(QTextDocumentFragment(src));
                }
                const QTextBlock at = nd->findBlock(from);
                QTextBlockFormat bf = at.blockFormat();
                if (from > at.position()) bf.setTextIndent(0);
                QTextCursor(tail->begin()).setBlockFormat(bf);
            }
            ++noteDepth;
            lay->build(tail ? tail.get() : nd, {noteFrame(n, width, room)}, env);
            --noteDepth;
            if (window > 0 && !lay->overflow()) {   // (it did not fill the room: the window may have cut it short)
                window *= 4;
                lay = std::make_shared<StoryLayout>();
                continue;
            }
            break;
        }
        const int last = lay->lastPosition(0);
        if (lay->overflow() && last >= 0) {
            // A cut at a paragraph's end goes on with the next paragraph.
            int end = from + last;
            const QTextBlock at = nd->findBlock(end);
            if (at.isValid() && end >= at.position() + at.length() - 1) end = at.next().isValid() ? at.next().position() : -1;
            if (end > from) *rest = end;
        }
        return lay;
    };
    // The notes of one line in a column with `room` for them: those that fit
    // whole, then the first that doesn't, cut where the room ends. That fails
    // (ok is false) when not even a line of it fits, unless `force`.
    struct Cut { bool ok = true; int at = -1; std::shared_ptr<StoryLayout> layout; double height = 0, used = 0; int rest = -1; };
    auto cutNotes = [&](const QVector<int> &ids, double room, double width, bool force) {
        Cut cut;
        for (int k = 0; k < ids.size(); ++k) {
            const Note &n = m_notes[ids[k]];
            if (n.height + noteGap <= room + 0.01) {
                room -= n.height + noteGap;
                cut.used += n.height + noteGap;
                continue;
            }
            cut.at = k;
            cut.layout = cutNote(n, 0, width, std::max(0.0, room - noteGap), &cut.rest);
            cut.height = std::max(heightOf(n.numberFont), cut.layout->usedHeight(0) + (cut.rest < 0 ? cut.layout->trailingSpace() : 0.0));
            if (force && cut.rest < 0 && cut.layout->usedHeight(0) + noteGap <= room + 0.01) cut.height = std::min(cut.height, std::max(room - noteGap, cut.layout->usedHeight(0)));   // (the space after its last line is let go at the foot)
            cut.ok = force || cut.height + noteGap <= room + 0.01;
            cut.used += cut.height + noteGap;
            break;
        }
        return cut;
    };
    // Notes that go on from the column before come first in the column just
    // entered, under a rule as wide as the column, each with what fits of
    // its text; what's left goes on again in the next column.
    auto settleCarry = [&]() {
        if (carry.isEmpty() || f >= nF) return;
        const QRectF col = colRect(f, c);
        double used = continuedGap;
        gapOf[rkey(f, c)] = continuedGap;
        while (!carry.isEmpty()) {
            Note &n = m_notes[carry.first()];
            measureNote(n, col.width(), n.frame != -2 && !n.layout);   // (the rest of a note that began on the page before need not be measured whole)
            const double room = col.height() - used - noteGap;
            int rest = -1;
            const auto lay = cutNote(n, n.from, col.width(), std::max(0.0, room), &rest);
            double h = std::max(heightOf(n.numberFont), lay->usedHeight(0) + (rest < 0 ? lay->trailingSpace() : 0.0));
            if (rest < 0 && lay->usedHeight(0) <= room + 0.01) h = std::min(h, std::max(room, lay->usedHeight(0)));   // (the space after its last line is let go at the foot)
            if (h > room + 0.01 && used > continuedGap) break;   // not even a line fits under the notes before it
            if (n.frame == -1) {   // a note of the same line that hadn't begun
                n.frame = f;
                n.column = c;
                n.width = col.width();
                n.layout = lay;
                n.height = h;
            } else {
                Note::Piece piece{f, c, QRectF(), h, lay};
                piece.from = n.from;
                n.more << piece;
            }
            used += h + noteGap;
            if (rest < 0) {
                carry.removeFirst();
                continue;
            }
            n.from = rest;
            break;
        }
        if (used > continuedGap) reserve[rkey(f, c)] = used;
        else gapOf.remove(rkey(f, c));
    };
    auto advance = [&]() {
        rowActive = false;
        rowPushed = 0;
        columnEmpty = true;
        y = 0;
        if (f < nF && ++c >= std::max(1, frames[f].columns)) { c = 0; ++f; }
        if (!seeded.isEmpty()) {
            carry = seeded;
            seeded.clear();
        }
        // (Not in the frame that the stretch only runs over into: what is left of the notes goes on to the page after, which lays them out.)
        if (m_span.stopAtFrame < 0 || f < m_span.stopAtFrame) settleCarry();
    };

    // Notes that went on from the page before come first in the first frame.
    QVector<CarriedNote> carriedIn = m_span.carried;
    if (carriedIn.isEmpty() && notesDoc)
        for (const QString &c : notesDoc->carriedNotes()) {
            const QStringList f = c.split(QLatin1Char('|'));
            if (f.size() == 3) carriedIn << CarriedNote{f[0], f[1].toInt(), f[2].toInt()};
        }
    for (const CarriedNote &cn : carriedIn) {
        if (!withNotes || cn.storyId.isEmpty()) continue;
        Note n;
        n.storyId = cn.storyId;
        n.number = cn.number;
        n.from = cn.from;
        n.frame = -2;   // begun before the first frame: only the rest of its text is here (it is measured when its room is: a page can have thousands)
        (m_span.firstBreakAfter >= 0 ? seeded : carry) << int(m_notes.size());
        m_notes << n;
    }
    if (m_span.firstBreakAfter < 0) settleCarry();   // (with lines of the first paragraph in a frame before, it's the frame after that)

    bool stoppedEarly = false;
    for (QTextBlock b = m_span.from > 0 ? doc->findBlock(m_span.from) : doc->begin(); b.isValid(); b = b.next()) {
        if (m_span.end >= 0 && b.position() >= m_span.end) {
            stoppedEarly = true;
            break;
        }
        const bool first = m_blocks.empty();   // the first paragraph laid out
        const bool tail = first && m_span.continued && m_span.from > b.position();   // the rest of one that began before
        if (!m_span.restartNotesAt.isEmpty() && std::binary_search(m_span.restartNotesAt.cbegin(), m_span.restartNotesAt.cend(), b.position())) footCount = endCount = 0;
        auto B = std::make_unique<Block>();
        B->docStart = b.position();
        B->docLen = b.length() - 1;
        const QTextBlockFormat bf = b.blockFormat();
        // The fill under the paragraph's text, for text of the automatic color: its own shading, else what its frame is on (a table cell's fill).
        QColor behind = frames.isEmpty() ? QColor() : frames[std::min(f, nF - 1)].backdrop;
        if (env.wordLines && bf.hasProperty(tp::Borders))
            if (const QColor fill = tp::ParaBox::parse(bf.stringProperty(tp::Borders)).fill; fill.isValid() && fill.alpha() > 0) behind = fill;
        // A paragraph set apart from the text shows nothing of it, unless the caller says it is laid out as the others are.
        const bool apart = env.wordLines && bf.boolProperty(tp::OutOfLayout) && !(m_span.inLayout && std::binary_search(m_span.inLayout->cbegin(), m_span.inLayout->cend(), b.position()));
        const FieldContext &ctx = frames.isEmpty() ? FieldContext() : frames[std::min(f, nF - 1)].ctx;

        // Build the display string, mapping fields and the drop cap.
        const bool trueSmallCaps = hasTrueSmallCaps(b);
        const int dropLines = tail ? 0 : bf.intProperty(tp::DropCapLines);
        int dropRemaining = dropLines > 0 ? std::max(1, bf.intProperty(tp::DropCapChars)) : 0;
        QVector<QTextLayout::FormatRange> ranges;
        QTextCharFormat dropFmt;
        // The paragraph's text from where the rest of it begins, and no more than the window holds.
        const int skipChars = tail ? m_span.from - b.position() : 0;
        const int limit = m_span.window > 0 ? m_span.window : std::numeric_limits<int>::max();
        int taken = 0;
        bool cutShort = false;
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            if (cutShort) break;
            const QTextFragment frag = it.fragment();
            if (!frag.isValid()) continue;
            int rel = frag.position() - b.position();
            const QString whole = frag.text();
            int begin = 0, count = int(whole.size());
            if (rel + count <= skipChars) continue;
            if (rel < skipChars) {
                begin = skipChars - rel;
                count -= begin;
                rel = skipChars;
            }
            if (taken + count > limit) {
                count = limit - taken;
                cutShort = true;
                B->cutAt = rel + count;
            }
            const QString text = whole.mid(begin, count);   // (the part wanted of a long run only is copied)
            taken += count;
            const QTextCharFormat cf = frag.charFormat();
            // A paragraph that is set apart from the text shows nothing of it, a picture in it included.
            if (apart) {
                B->map << Seg{rel, int(text.size()), int(B->disp.size()), 0, true};
                continue;
            }
            QTextCharFormat rf = resolveCharFormat(cf, env, behind);
            // An object set in the text: an em dash (a line may break on
            // either side, as at a picture) in the run's own font, so the
            // line's text metrics stay; letter spacing makes it as wide as the
            // object and its side wrap distances, and it's drawn clear.
            const QString object = cf.stringProperty(tp::InlineObject);
            if (!object.isEmpty()) {
                const ItemPtr item = Item::fromJsonAny(QJsonDocument::fromJson(object.toUtf8()).object());
                const QChar standIn(0x2014);
                QFont plain = rf.font();
                plain.setLetterSpacing(QFont::AbsoluteSpacing, 0);
                const double adv = advanceOf(plain, QString(standIn));
                for (int i = 0; i < text.size(); ++i) {
                    Block::Box box;
                    box.disp = int(B->disp.size());
                    box.docPos = b.position() + rel + i;
                    box.json = object;
                    if (item) {
                        // Sizes from a file: finite and within a page or so.
                        auto sane = [](double v, double most) { return std::isfinite(v) ? std::clamp(v, 0.0, most) : 0.0; };
                        box.size = QSizeF(sane(item->rect.width(), 20000), sane(item->rect.height(), 20000));
                        box.pad = QMarginsF(sane(item->wrap.left, 1000), sane(item->wrap.top, 1000), sane(item->wrap.right, 1000), sane(item->wrap.bottom, 1000));
                    }
                    QTextCharFormat of = rf;
                    of.setFontLetterSpacingType(QFont::AbsoluteSpacing);
                    of.setFontLetterSpacing(box.pad.left() + box.size.width() + box.pad.right() - adv);
                    of.setForeground(QColor(Qt::transparent));
                    of.setFontUnderline(false);
                    of.setUnderlineStyle(QTextCharFormat::NoUnderline);
                    of.setFontStrikeOut(false);
                    of.setFontOverline(false);
                    of.clearBackground();
                    B->map << Seg{rel + i, 1, int(B->disp.size()), 1};
                    ranges << QTextLayout::FormatRange{int(B->disp.size()), 1, of};
                    B->objects << box;
                    B->disp += standIn;
                }
                continue;
            }
            // Hidden text takes no room and shows nothing, though its characters are the story's (so does a paragraph that is set apart from the text).
            if (env.wordLines && cf.boolProperty(tp::HiddenText)) {
                B->map << Seg{rel, int(text.size()), int(B->disp.size()), 0, true};
                continue;
            }
            const QString field = cf.stringProperty(tp::Field);
            if (!field.isEmpty()) {
                const bool isNote = field.startsWith(QLatin1String("footnote:")) || field.startsWith(QLatin1String("endnote:"));
                for (int i = 0; i < text.size(); ++i) {
                    // A note shows its number, counted here in the story's order.
                    if (isNote) {
                        QString v;
                        if (withNotes) {
                            Note n;
                            n.storyId = field.section(QLatin1Char(':'), 1);
                            n.endnote = field.startsWith(QLatin1String("endnote:"));
                            const bool customMark = env.wordLines && cf.boolProperty(tp::NoteMarkCustom);
                            n.number = customMark ? 0 : n.endnote ? ++endCount : ++footCount;
                            n.refPos = b.position() + rel + i;
                            if (!customMark) v = QString::number(n.number);
                            B->noteRefs << qMakePair(int(B->disp.size()), int(m_notes.size()));
                            m_notes << n;
                        }
                        B->map << Seg{rel + i, 1, int(B->disp.size()), int(v.size()), env.wordLines && v.isEmpty()};
                        // (The numbers of references side by side are one run of text: Qt shapes each run of its own, and a paragraph of a
                        // thousand takes each page more than a tenth of a second.)
                        if (env.wordLines && !ranges.isEmpty() && ranges.last().start + ranges.last().length == int(B->disp.size()) && ranges.last().format == rf) ranges.last().length += int(v.size());
                        else ranges << QTextLayout::FormatRange{int(B->disp.size()), int(v.size()), rf};
                        B->fieldRanges << qMakePair(int(B->disp.size()), int(v.size()));
                        B->disp += v;
                        continue;
                    }
                    // Multi-line values (an address) break lines inside the paragraph.
                    QString v = ctx.resolve(field);
                    v.replace(QLatin1String("\r\n"), QString(QChar::LineSeparator)).replace(QLatin1Char('\n'), QChar::LineSeparator);
                    B->map << Seg{rel + i, 1, int(B->disp.size()), int(v.size()), env.wordLines && v.isEmpty()};
                    ranges << QTextLayout::FormatRange{int(B->disp.size()), int(v.size()), rf};
                    B->fieldRanges << qMakePair(int(B->disp.size()), int(v.size()));
                    B->disp += v;
                }
                continue;
            }
            int start = 0;
            if (dropRemaining > 0) {
                const int k = std::min<int>(dropRemaining, text.size());
                if (B->dropText.isEmpty()) dropFmt = rf;
                B->dropText += text.left(k);
                B->map << Seg{rel, k, int(B->disp.size()), 0};
                dropRemaining -= k;
                start = k;
            }
            if (start < text.size()) {
                const QString raw = text.mid(start);
                QString shown;
                // Places a line may break that Qt's own rules leave out, each
                // with the character that marks it in the displayed text.
                QVector<QPair<int, QChar>> cuts;
                if (hyphenate) {
                    // Automatic hyphenation: soft hyphens (shown only where a line
                    // breaks) at the allowed points of words of five letters or more.
                    static const QRegularExpression wordRe(QStringLiteral("\\p{L}{5,}"));
                    const QString language = cf.stringProperty(tp::Language);   // empty: US English
                    auto it = wordRe.globalMatch(raw);
                    while (it.hasNext()) {
                        const auto m = it.next();
                        const QString w = m.captured();
                        if (w == w.toUpper()) continue;   // leave all-caps words whole
                        if (!hyphenationKnows(w, language)) continue;   // names and coined words stay whole
                        for (int pt : hyphenationPoints(w, language)) cuts.append({int(m.capturedStart()) + pt, QChar(0x00AD)});
                    }
                }
                // After a hyphen before a digit ("012-3456", "pages 4-12"):
                // Publisher breaks there, Qt's rules keep the number whole.
                for (int i = 1; i + 1 < int(raw.size()); ++i)
                    if (raw[i] == QLatin1Char('-') && raw[i - 1].isLetterOrNumber() && raw[i + 1].isDigit()) cuts.append({i + 1, QChar(0x200B)});
                // A slide's or a word processor's line doesn't break after a slash ("expression/regulation", an address): the program breaks at
                // spaces and hyphens, and a word that is longer than the line anywhere. A word joiner after it keeps Qt's rules from breaking there.
                if (env.slideLines || env.wordLines) {
                    for (int i = 0; i + 1 < int(raw.size()); ++i)
                        if (raw[i] == QLatin1Char('/') && !raw[i + 1].isSpace() && raw[i + 1] != QChar::LineSeparator) cuts.append({i + 1, QChar(0x2060)});
                    // (And one that ends a run, where the text goes on in another format: the next run's first letter says.)
                    if (!cutShort && raw.endsWith(QLatin1Char('/'))) {
                        auto next = it;
                        ++next;
                        if (!next.atEnd())
                            if (const QString after = next.fragment().text(); !after.isEmpty() && !after[0].isSpace() && after[0] != QChar::LineSeparator) cuts.append({int(raw.size()), QChar(0x2060)});
                    }
                }
                std::sort(cuts.begin(), cuts.end(), [](const auto &x, const auto &y) { return x.first < y.first; });
                int piece = 0;
                for (const auto &[cut, mark] : cuts) {
                    const int n = cut - piece;
                    B->map << Seg{rel + start + piece, n, int(B->disp.size() + shown.size()), n};
                    shown += raw.mid(piece, n);
                    B->map << Seg{rel + start + cut, 0, int(B->disp.size() + shown.size()), 1};
                    shown += mark;
                    piece = cut;
                }
                const int n = int(raw.size()) - piece;
                B->map << Seg{rel + start + piece, n, int(B->disp.size() + shown.size()), n};
                shown += raw.mid(piece);
                // A form feed ends the line as U+2028 does (a word processor's page break: the paginator ends the page after that line). One for
                // one, so positions still map.
                if (shown.contains(QChar(0x000C))) shown.replace(QChar(0x000C), QChar(QChar::LineSeparator));
                // All capitals (or all lowercase): Qt ignores these in a
                // layout's format ranges, so the letters are changed here, one
                // for one so positions still map. That is deliberate: a letter
                // whose capital is two letters (German ß, "SS") stays as it is,
                // since the caret and selection count display letters 1:1.
                if (rf.fontCapitalization() == QFont::AllUppercase || rf.fontCapitalization() == QFont::AllLowercase) {
                    const bool up = rf.fontCapitalization() == QFont::AllUppercase;
                    for (QChar &ch : shown) ch = up ? ch.toUpper() : ch.toLower();
                    rf.setFontCapitalization(QFont::MixedCase);
                }
                // The characters of a symbol font that this machine lacks are shown as the pictures they stand for (the font's own widths are given
                // them in applyLetterWidths): a degree sign from Symbol, an arrow from Wingdings, a banner's block from Webdings.
                if (env.wordLines) {
                    const QString family = rf.fontFamilies().toStringList().value(0);
                    if (const SymbolPictures *pictures = hasFamilyCached(family) ? nullptr : symbolPicturesOf(family))
                        for (QChar &ch : shown)
                            if (const QChar picture = pictures->of(ch.unicode()); !picture.isNull()) ch = picture;
                }
                const int len = int(shown.size());
                QTextLayout::FormatRange fr{int(B->disp.size()), len, rf};
                if (rf.fontCapitalization() == QFont::SmallCaps && !(cf.boolProperty(tp::TrueSmallCaps) && trueSmallCaps)) {
                    // .pub small capitals: lowercase letters become capitals at
                    // 0.8 x the size (Qt would use 0.7). Display text keeps its length,
                    // so positions still map one to one.
                    QTextCharFormat big = rf, small = rf;
                    big.setFontCapitalization(QFont::MixedCase);
                    small.setFontCapitalization(QFont::MixedCase);
                    small.setFontPointSize(rf.fontPointSize() * 0.8);
                    // Line spacing counts the letters at the full size (in
                    // reference PDFs, a line of small capitals only is as far
                    // from the next as one at the full size).
                    small.setProperty(tp::LineSize, rf.fontPointSize());
                    // Tracking follows the smaller size.
                    if (const double tr = trackingSpace(cf, rf.font())) small.setFontLetterSpacing(rf.fontLetterSpacing() - 0.2 * tr);
                    int runStart = 0;
                    // A soft hyphen (a possible break) goes with the letter before it,
                    // so an invisible full-size mark doesn't deepen the line.
                    auto isSmall = [&](int i) { return shown[i].isLower() || (shown[i] == QChar(0x00AD) && i > 0 && shown[i - 1].isLower()); };
                    for (int i = 1; i <= shown.size(); ++i)
                        if (i == shown.size() || isSmall(i) != isSmall(runStart)) {
                            ranges << QTextLayout::FormatRange{int(B->disp.size()) + runStart, i - runStart, isSmall(runStart) ? small : big};
                            runStart = i;
                        }
                    for (QChar &ch : shown)
                        if (ch.isLower()) ch = ch.toUpper();
                } else if (fr.format.fontLetterSpacing() != 0 && shown.contains(QChar(0x2060))) {
                    // (A word joiner takes no letter spacing: it is no letter, and would be a gap where it stands.)
                    QTextCharFormat none = fr.format;
                    none.setFontLetterSpacing(0);
                    int from = 0;
                    for (int at = int(shown.indexOf(QChar(0x2060))); at >= 0; at = int(shown.indexOf(QChar(0x2060), at + 1))) {
                        if (at > from) ranges << QTextLayout::FormatRange{fr.start + from, at - from, fr.format};
                        ranges << QTextLayout::FormatRange{fr.start + at, 1, none};
                        from = at + 1;
                    }
                    if (from < int(shown.size())) ranges << QTextLayout::FormatRange{fr.start + from, int(shown.size()) - from, fr.format};
                } else {
                    ranges << fr;
                }
                // A slide's text with the Shadow button on has the shadow the presentation program gives it.
                const bool programShadow = env.slideLines && cf.boolProperty(tp::Shadow) && cf.stringProperty(tp::TextFx).isEmpty();
                if (cf.boolProperty(tp::Shadow) || cf.boolProperty(tp::Emboss) || cf.boolProperty(tp::Engrave) ||
                    !cf.stringProperty(tp::GlowRef).isEmpty() || !cf.stringProperty(tp::TextFx).isEmpty()) {
                    QTextLayout::FormatRange er = fr;
                    er.format = cf;   // keep raw props for the effect pass
                    static const QString kProgramShadow = defaultTextShadow().toJson();
                    if (programShadow) er.format.setProperty(tp::TextFx, kProgramShadow);
                    if (!er.format.stringProperty(tp::TextFx).isEmpty()) B->textFx = true;
                    er.format.setForeground(rf.foreground());
                    er.format.setFontPointSize(rf.fontPointSize());
                    if (rf.hasProperty(tp::GlyphScaleY)) er.format.setProperty(tp::GlyphScaleY, rf.property(tp::GlyphScaleY));
                    if (rf.hasProperty(tp::SyntheticItalic)) er.format.setProperty(tp::SyntheticItalic, rf.property(tp::SyntheticItalic));
                    B->effects << er;
                }
                B->disp += shown;
            }
        }
        B->cut = cutShort;
        // Qt draws U+2028 as a line break; keep it.
        const QFont base = baseFontFor(b, env);
        // (A word processor's line that shows no text is the height of the paragraph mark's font.)
        QFont markFont;
        double markSize = 0;
        if (env.wordLines) {
            const QTextCharFormat mf = resolveCharFormat(b.charFormat(), env);
            markFont = mf.font();
            markSize = mf.hasProperty(tp::LineSize) ? mf.property(tp::LineSize).toDouble() : markFont.pointSizeF();
        }
        B->tl = std::make_unique<ParagraphLayout>(B->disp, fineFont(base));
        bool adjusted = false;
        ranges = applyLetterWidthsTo(ranges, B->disp, &adjusted);
        B->lettersAdjusted = adjusted;
        B->tl->setFormats(fineRanges(ranges));
        // Letters drawn taller, shorter, or slanted are drawn run by run, unless
        // the paragraph has highlighted or wavy-underlined text (Qt's own
        // drawing then) or text no run covers.
        {
            bool scaled = false, plain = true;
            int covered = 0;
            for (const auto &r : ranges) {
                scaled |= r.format.hasProperty(tp::GlyphScaleY) || r.format.hasProperty(tp::SyntheticItalic);
                plain &= !r.format.hasProperty(QTextFormat::BackgroundBrush) && r.format.underlineStyle() != QTextCharFormat::WaveUnderline;
                covered += r.length;
            }
            B->directGlyphs = scaled && plain && covered == B->disp.size();
        }
        QTextOption opt;
        Qt::Alignment al = bf.alignment() & Qt::AlignHorizontal_Mask;
        if (!al) al = Qt::AlignLeft;
        // A paragraph's left and right alignment are its start and end (as in
        // .pub files): a right-to-left paragraph starts at the right. Qt's
        // text layout takes them as the page's sides, so they swap here.
        if (bf.layoutDirection() == Qt::RightToLeft && !(al & Qt::AlignAbsolute)) {
            if (al & Qt::AlignLeft) al = (al & ~Qt::AlignLeft) | Qt::AlignRight;
            else if (al & Qt::AlignRight) al = (al & ~Qt::AlignRight) | Qt::AlignLeft;
        }
        opt.setAlignment(al);
        opt.setWrapMode(frames.size() == 1 && frames[0].noWrap && frames[0].columns <= 1 ? QTextOption::NoWrap : QTextOption::WrapAtWordBoundaryOrAnywhere);
        QList<QTextOption::Tab> tabs = bf.tabPositions();
        for (auto &t : tabs) t.position *= scale * kFine;
        opt.setTabs(tabs);
        opt.setTabStopDistance(std::max(1.0, env.defaultTab) * scale * kFine);
        B->leaders = bf.stringProperty(tp::TabLeaders);
        if (bf.layoutDirection() == Qt::RightToLeft) opt.setTextDirection(Qt::RightToLeft);
        opt.setFlags(QTextOption::IncludeTrailingSpaces);
        B->tl->setTextOption(opt);
        B->tl->setCacheEnabled(true);
        // A justified paragraph that the space shrinking cut before (the same text, format, and option) is set as it came to, its lines checked once they are laid out.
        B->shrunk.reset();
        B->shrinkKey.clear();
        if (env.wordLines && env.wordSpaceShrink > 0 && (al & Qt::AlignJustify) && !bf.boolProperty(tp::Distribute) && !hyphenate && B->disp.size() <= 2000 &&
            !B->disp.contains(QLatin1Char('\t')) && !B->disp.contains(QChar::ObjectReplacementCharacter) && B->objects.isEmpty()) {
            QString key = B->disp + QChar(1) + QString::number(env.wordSpaceShrink) + (B->lettersAdjusted ? QLatin1Char('a') : QLatin1Char('n')) + (m_span.keepRules ? QLatin1Char('k') : QLatin1Char('c'));
            for (const QTextLayout::FormatRange &r : std::as_const(ranges)) {
                const QTextCharFormat &f = r.format;
                key += QStringLiteral("|%1,%2,%3,%4,%5").arg(r.start).arg(r.length).arg(f.font().key()).arg(f.fontLetterSpacing()).arg(f.fontWordSpacing());
            }
            B->shrinkKey = key;
            if (const ShrunkParagraph *kept = shrinkMemory().object(key)) {
                B->shrunk = *kept;
                B->tl->setFormats(fineRanges(kept->ranges));
            }
        }

        // (The text after a page break that begins a paragraph is set as the paragraph's first line, indent and all, as a word processor sets it.)
        // (A break that is hidden text, as what a tracked change deleted is, is none.)
        const bool startsWithBreak = env.wordLines && doc->characterAt(b.position()) == QChar(0x000C) && b.length() > 1 && !b.begin().fragment().charFormat().boolProperty(tp::HiddenText);
        const bool firstAfterBreak = startsWithBreak && (!tail || m_span.indentContinued);
        const double leftM = bf.leftMargin() * scale, rightM = bf.rightMargin() * scale, textIndent = tail && !firstAfterBreak ? 0.0 : bf.textIndent() * scale;
        // A slide's space given as a share of the line is of the line at the size the text was shrunk to; one given in points is not.
        double before = tail && !m_span.spaceBeforeContinued ? 0.0 : bf.topMargin() * scale, after = bf.bottomMargin() * scale;
        if (env.slideLines) {
            before = slideSpaceOf(b, tp::SpaceBeforeShare, env.fontScale);
            after = slideSpaceOf(b, tp::SpaceAfterShare, env.fontScale);
        }
        // (A paragraph that begins with a page break has no space before where it begins: the text after the break has it, on the next page.)
        if (startsWithBreak && !tail) before = 0;
        // A word processor's automatic spacing (a paragraph from a web page) is a fixed 14 points.
        if (env.wordLines) {
            // (Between two items of a list there is none; a list that follows another has it.)
            const bool item = b.textList() != nullptr;
            auto inList = [&](const QTextBlock &other) {
                return item && other.isValid() && other.textList() && other.textList()->format().intProperty(tp::ListKey) == b.textList()->format().intProperty(tp::ListKey);
            };
            if (bf.boolProperty(tp::SpaceBeforeAuto) && (!tail || m_span.spaceBeforeContinued)) before = inList(b.previous()) ? 0.0 : 14 * scale;
            if (bf.boolProperty(tp::SpaceAfterAuto)) after = inList(b.next()) ? 0.0 : 14 * scale;
        }

        // List marker.
        double markerW = 0, markerSingle = 0;
        WordLine markerWord;
        if (QTextList *list = (env.slideLines && b.length() <= 1) || tail ? nullptr : b.textList()) {
            const QTextListFormat lf = list->format();
            B->markerSkipsIndent = env.wordNoNumberIndentStop && !lf.boolProperty(tp::LegacyNumber);
            QString custom = bf.stringProperty(tp::BulletChar);
            if (custom.isEmpty()) custom = lf.stringProperty(tp::BulletChar);
            if (!custom.isEmpty()) B->marker = custom;
            else if (lf.style() == QTextListFormat::ListDisc) B->marker = QStringLiteral("•");
            else if (lf.style() == QTextListFormat::ListCircle) B->marker = QStringLiteral("◦");
            else if (lf.style() == QTextListFormat::ListSquare) B->marker = QStringLiteral("▪");
            else B->marker = list->itemText(b);
            // (A word processor's number is of its paragraph's mark, not of the text that happens to come first.)
            // (A level that says its color is automatic has a number of the automatic color, not the mark's.)
            QTextCharFormat markFormat = b.charFormat();
            const bool autoColor = env.wordLines && lf.stringProperty(tp::BulletColor) == QLatin1String("auto");
            if (autoColor) {
                markFormat.clearForeground();
                markFormat.clearProperty(tp::ColorRefP);
            }
            QTextCharFormat mf = ranges.isEmpty() || env.wordLines ? resolveCharFormat(markFormat, env, behind) : ranges.first().format;
            B->markerFont = mf.font();
            if (env.wordLines) {
                if (lf.hasProperty(tp::BulletBold)) B->markerFont.setWeight(lf.boolProperty(tp::BulletBold) ? QFont::Bold : QFont::Normal);
                if (lf.hasProperty(tp::BulletItalic)) B->markerFont.setItalic(lf.boolProperty(tp::BulletItalic));
            }
            if (lf.hasProperty(tp::BulletSize)) {
                // (A slide's bullet, a share of its text, follows the whole-point size the text was shrunk to.)
                const double first = rawFirstSize(b);
                const double grow = env.slideLines && env.fontScale < 1.0 ? scaledSize(first, env) / first : env.fontScale;
                B->markerFont.setPointSizeF(std::max(1.0, lf.property(tp::BulletSize).toDouble() * grow) * fontPointFactor());
            }
            const QString bfont = lf.stringProperty(tp::BulletFont);
            // A Symbol or Wingdings bullet whose glyph Publisher's PDFs measured
            // is drawn to that glyph's size and advance: its own font's, or a
            // stand-in fitted to it when the font is missing. (The plain round
            // Symbol bullet has no character of its own, and is drawn so
            // wherever Symbol is: the text font's bullet is smaller.)
            SymbolBullet sb;
            const uint code = custom.size() == 1 ? custom.at(0).unicode() : custom.isEmpty() && bfont.compare(QLatin1String("Symbol"), Qt::CaseInsensitive) == 0 ? 0xB7 : 0;
            const bool measured = code && isSymbolFont(bfont) && (custom.isEmpty() || !hasFamilyCached(bfont)) && symbolBullet(bfont, code, &sb);
            if (!bfont.isEmpty() && (!custom.isEmpty() || measured)) {
                // A bullet in its own font is that font's regular face, not
                // the weight or slant of the item's first run (Publisher draws
                // an asterisk bullet thin beside bold text).
                // (A word processor's number or bullet in a text face is bold or slanted as its paragraph's mark and its level say.)
                if (!env.wordLines || isSymbolFont(bfont)) {
                    B->markerFont.setWeight(QFont::Normal);
                    B->markerFont.setItalic(false);
                }
                if (measured) {
                    B->marker = sb.standIn.isEmpty() ? QStringLiteral("•") : sb.standIn;
                    const double em = B->markerFont.pointSizeF() / fontPointFactor();
                    if (!sb.ink.isEmpty()) {
                        B->markerFit = true;
                        B->markerRound = sb.standIn.isEmpty();
                        B->markerInk = QRectF(sb.ink.x() * em, -sb.ink.bottom() * em, sb.ink.width() * em, sb.ink.height() * em);
                    }
                    if (sb.advance > 0) B->markerAdv = sb.advance * em;
                } else if (isSymbolFont(bfont) && !hasFamilyCached(bfont)) {
                    // A missing symbol font: the same picture from Unicode, in
                    // the text's font (or a round bullet if it has none).
                    const QString uni = symbolToUnicode(bfont, custom.at(0).unicode());
                    B->marker = uni.isEmpty() ? QStringLiteral("•") : uni;
                } else {
                    B->markerFont.setFamily(env.aliasOf(bfont));
                    // Symbol fonts find their pictures at 0xF020-0xF0FF.
                    if (isSymbolFont(bfont) && custom.size() == 1 && custom.at(0).unicode() >= 0x20 && custom.at(0).unicode() <= 0xFF)
                        B->marker = QString(QChar(char16_t(0xF000 + custom.at(0).unicode())));
                }
            }
            // A word processor's number is in the font its level names.
            else if (env.wordLines && !bfont.isEmpty() && custom.isEmpty() && !isBulletList(lf.style())) B->markerFont.setFamily(env.aliasOf(bfont));
            B->markerColor = mf.foreground().color();
            B->markerOutline = env.wordLines && lf.boolProperty(tp::BulletOutline);
            const QString bc = lf.stringProperty(tp::BulletColor);
            if (!bc.isEmpty() && !autoColor) B->markerColor = ColorRef::fromString(bc).resolve(env.colors);
            if (B->markerAdv <= 0) B->markerAdv = advanceOf(B->markerFont, B->marker);
            // A bullet from a .pub list is followed by its text at once (Publisher's
            // PDFs: the text starts at the glyph's advance) unless the list's
            // indent, measured from the text area's edge, is further; one of
            // JeffPub's own has a little room.
            B->markerGap = bfont.isEmpty() && !env.slideLines ? 4 * scale : 0;
            markerW = B->markerAdv + B->markerGap;
            if (lf.hasProperty(tp::BulletTab)) {
                markerW = std::max(markerW, lf.property(tp::BulletTab).toDouble() * scale - (leftM + textIndent));
            } else if (env.wordLines && (textIndent >= 0 || B->markerSkipsIndent)) {
                // A word processor's number or bullet is followed by a tab: the text begins at the next stop past it, one of the paragraph's own or,
                // past the last of those, a default one (from the text area's edge). (A hanging indent is that stop unless the file says it is not, and
                // then the number's own list tab is one.)
                const double first = leftM + textIndent, end = first + B->markerAdv;
                const double step = std::max(1.0, env.defaultTab) * scale;
                double stop = -1, last = -1;
                auto consider = [&](double at) {
                    last = std::max(last, at);
                    if (at > end + 0.01 && (stop < 0 || at < stop)) stop = at;
                };
                for (const QTextOption::Tab &t : bf.tabPositions()) consider(t.position * scale);
                if (B->markerSkipsIndent && bf.hasProperty(tp::NumberTab)) {
                    consider(bf.doubleProperty(tp::NumberTab) * scale);
                } else if (B->markerSkipsIndent) {
                    // (With no list tab, the paragraph clearing the level's or setting a stop of its own in its place, the text goes to the nearest of the paragraph's stops, the
                    // hanging indent, and the default stops, which as in the text lie only past the last stop of the paragraph's own.)
                    const double defaultStop = (std::floor((std::max(end, last) + 0.01) / step) + 1) * step;
                    if (textIndent < 0) consider(leftM);
                    consider(defaultStop);
                }
                if (stop < 0) stop = (std::floor((std::max(end, last) + 0.01) / step) + 1) * step;
                markerW = stop - first;
            }
            B->markerOff = markerW;
            // The bullet's own line height counts on the item's first line: a
            // Symbol bullet (as .pub lists have) is taller than most text.
            const QString mfam = bfont.isEmpty() ? requestedFamily(B->markerFont) : bfont;
            const double known = knownMetrics(mfam, B->markerFont.weight() >= QFont::DemiBold).line;
            // (A slide's bullet doesn't: its line is the text's, however big the bullet is drawn.)
            if (env.wordLines) {
                // The number or bullet is a run of its own font on the first line, which the line has room for.
                QFont mf = B->markerFont;
                if (!bfont.isEmpty()) mf.setFamilies({bfont});
                const WordLine face = wordFaceOf(mf);
                const double em = mf.pointSizeF() / fontPointFactor();
                markerWord.ascent = face.ascent * em;
                markerWord.descent = face.descent * em;
                markerWord.leading = env.wordNoLeading ? 0.0 : face.leading * em;
                markerSingle = markerWord.total();
            } else {
                markerSingle = env.slideLines ? 0.0 : B->markerFont.pointSizeF() / fontPointFactor() * (known > 0 ? known : lineEmOf(B->markerFont, mfam));
            }
        }

        // Drop cap metrics: cap height spans dropLines lines.
        if (!B->dropText.isEmpty()) {
            B->dropLines = dropLines;
            B->dropUp = bf.boolProperty(tp::DropCapUpper);
            QFont df = dropFmt.font();
            const QString dfam = bf.stringProperty(tp::DropCapFont);
            if (!dfam.isEmpty()) df.setFamily(dfam);
            const double lineH = heightOf(base) * (bf.lineHeightType() == QTextBlockFormat::ProportionalHeight ? bf.lineHeight() / 100.0 : 1.0);
            const double target = (dropLines - 1) * lineH + capHeightOf(base);
            QFont probe = df;
            probe.setPointSizeF(100);
            const double cap100 = std::max(1.0, capHeightOf(probe));
            df.setPointSizeF(std::max(4.0, target * 100.0 / cap100));
            B->dropFont = df;
            B->dropColor = dropFmt.foreground().color();
            const QString dc = bf.stringProperty(tp::DropCapColor);
            if (!dc.isEmpty()) B->dropColor = ColorRef::fromString(dc).resolve(env.colors);
            B->dropWidth = advanceOf(df, B->dropText) + 3 * scale;
        }

        // Placing the paragraph's lines can be redone from the same spot:
        // starting in the next column, or breaking a line early.
        struct Spot {
            int f, c;
            double y, overflowY, rowH;
            QVector<Iv> row;
            int rowIdx;
            bool rowActive, columnEmpty, overflow, blank;
            QVector<double> used;
            QHash<int, double> reserve;
            QVector<Note> notes;
            QVector<int> carry, seeded;
        };
        // A word processor's paragraph borders and shading: the box, whether it goes on from the paragraph before and to the one after, and the room the
        // lines above and below the text take.
        if (env.wordLines && bf.hasProperty(tp::Borders)) {
            B->box = tp::ParaBox::parse(bf.stringProperty(tp::Borders));
            auto sameBox = [&](const QTextBlock &x) {
                if (!x.isValid()) return false;
                const QTextBlockFormat xf = x.blockFormat();
                return xf.stringProperty(tp::Borders) == bf.stringProperty(tp::Borders) && std::abs(xf.leftMargin() - bf.leftMargin()) < 0.01 &&
                       std::abs(xf.rightMargin() - bf.rightMargin()) < 0.01;
            };
            B->boxTopOff = tail || bf.boolProperty(tp::BorderTopOff);
            B->boxJoinedBefore = !tail && sameBox(b.previous());
            B->boxJoinedAfter = sameBox(b.next()) || bf.boolProperty(tp::BorderBottomOff);
            B->boxTopRoom = B->boxJoinedBefore ? B->box.between.reserve() : B->boxTopOff ? 0.0 : B->box.top.reserve();
            B->boxBottomRoom = B->boxJoinedAfter ? 0.0 : B->box.bottom.reserve();
        }
        const double prevAfter = lastAfter, prevRoom = lastRoom;
        // Between paragraphs of one style that ask for no space between them, a word processor's contextual spacing leaves none.
        auto sameStyle = [](const QTextBlock &x, const QTextBlock &z) {
            return x.isValid() && z.isValid() && x.blockFormat().stringProperty(tp::StyleName) == z.blockFormat().stringProperty(tp::StyleName);
        };
        if (env.wordLines && bf.boolProperty(tp::ContextualSpacing)) {
            if (sameStyle(b, b.previous())) before = 0;
            if (sameStyle(b, b.next())) after = 0;
        }
        const Spot startSpot{f, c, y, overflowY, rowH, row, rowIdx, rowActive, columnEmpty, m_overflow, m_blankPast, m_used, reserve, m_notes, carry, seeded};
        const bool windowCut = B->cut;
        const int windowCutAt = B->cutAt;
        auto placeLines = [&](bool startNext, int breakAfter) {
            // Start in next text box.
            if (!tail && bf.boolProperty(tp::StartInNextBox) && f < nF && (y > 0 || c > 0)) {
                rowActive = false;
                y = 0; c = 0; ++f;
                settleCarry();
            }
            if (startNext && f < nF) advance();
            const double yBeforeSpace = y;
            if (f < nF && y > 0) y += env.wordLines && !env.wordSpaceSums ? std::max(0.0, before - prevAfter) : before;
            else if (f < nF && !env.slideLines && !(first && !m_span.spaceBeforeFirst)) y += env.wordLines && bf.boolProperty(tp::SpaceBeforeAuto) ? 0.0 : before;  // space before also applies at the top of a frame (not a slide's, nor a word processor's automatic)
            B->spaceAbove = y - yBeforeSpace;
            B->spaceFrame = f;
            B->spaceColumn = c;
            if (f < nF) y += B->boxTopRoom;   // (the line above a paragraph's text)
            // A raised cap rises above the first line: room for it.
            if (f < nF && B->dropUp && !B->dropText.isEmpty())
                y += std::max(0.0, capHeightOf(B->dropFont) - capHeightOf(base));

            // Estimate with the same rule the placed line will use, so wrap checks
            // see the obstacles the real line will meet.
            const double estSingle = singleOf({}, 0, 0, base);
            const double estH = std::max(1.0, lineHeightFor(bf, scale, estSingle));
            // Wrapping counts a line's text (its fonts' ascent and descent),
            // not the spacing around it: in Publisher a line moves aside only
            // for an object reaching above the bottom of its text (checked with
            // 48-point Times New Roman lines: the 53 points of its ascent and
            // descent, not the 55 of its single spacing).
            const double estText = std::min(estH, wrapTextHeight(base));
            // Tab stops are the publication's default distance (36 points, or
            // what it says) apart from the text area's edge, not from where a
            // line starts (Publisher), and a line that hangs (the first of a
            // paragraph indented less than the rest) has its left indent as a
            // stop, so "b.<tab>text" lines up its text there, the stops before
            // it gone. Qt counts stops from each line's own start, so each line
            // is laid out with a list of its own, or none when it starts on a
            // stop (Qt's spacing is the same places, and a list makes it go over
            // the rest of the paragraph for every tab it answers). A paragraph
            // with tab stops of its own keeps Qt's.
            const bool hangs = textIndent < -0.01;
            // (A word processor's tab stops are from the text area's edge whatever the line's indent, and the default stops go on past the last of the
            // paragraph's own: each line is laid out with a list of the paragraph's stops and the default ones after them.)
            const QList<QTextOption::Tab> ownTabs = bf.tabPositions();
            const bool wordTabs = env.wordLines && !ownTabs.isEmpty();
            // (Text aligned at a tab stop ends at the stop: a line with such a tab is as wide as its room when the stop is at the margin, which no pixel is to be kept back from.
            // Nor are the letters of a font whose widths are the original's (not a stand-in's own) measured so closely: they are a hair over or under what the original has.
            // Nor is the text of a table's cell, whose column the text's own width may have set.)
            const bool alignedTab = wordTabs && B->disp.contains(QLatin1Char('\t')) &&
                                    std::any_of(ownTabs.cbegin(), ownTabs.cend(), [](const QTextOption::Tab &t) { return t.type != QTextOption::LeftTab; });
            const double fit = env.wordLines ? (alignedTab || B->lettersAdjusted || !m_span.keepRules ? kQtPixelBack : kQtPixelBack + kRoomOffset) : 0.0;
            const bool gridTabs = (bf.tabPositions().isEmpty() || wordTabs) && bf.layoutDirection() != Qt::RightToLeft && B->disp.contains(QLatin1Char('\t'));
            const double step = std::max(1.0, env.defaultTab) * scale;
            auto hangStop = [&](double from, bool hangLine) { return hangLine && hangs && !env.wordNoHangStop && B->marker.isEmpty() && leftM > from + 0.01; };
            // The first stop past `pen`, measured from the text area's edge.
            auto nextStop = [&](double pen, bool hangLine) { return hangStop(pen, hangLine) ? leftM : (std::floor((pen + 0.01) / step) + 1) * step; };
            // The list for a line that starts `off` from the edge and is `w` wide, measured from its start.
            auto stopsFor = [&](double off, double w, bool hangLine) {
                QList<QTextOption::Tab> tabs;
                if (wordTabs) {
                    // (A word processor's left indent of a hanging line is a stop only where the paragraph has none of its own: the default stops before
                    // it stay, and with stops of its own they are where tabs go.)
                    double last = -1e9;
                    bool hung = true;
                    int own = 0;
                    for (QTextOption::Tab t : ownTabs) {
                        const double at = t.position * scale;
                        // (A stop's leader goes with it: a stop that isn't the paragraph's own has none, wherever it falls in the list.)
                        if (t.type != QTextOption::DelimiterTab) t.delimiter = own < B->leaders.size() && !B->leaders[own].isSpace() ? B->leaders[own] : QChar();
                        ++own;
                        if (!hung && at > leftM + 0.01) {
                            tabs << QTextOption::Tab((leftM - off) * kFine, QTextOption::LeftTab);   // (the left indent of a hanging line is a stop)
                            hung = true;
                        }
                        last = std::max(last, at);
                        t.position = (at - off) * kFine;
                        tabs << t;
                    }
                    if (!hung) {
                        tabs << QTextOption::Tab((leftM - off) * kFine, QTextOption::LeftTab);
                        last = std::max(last, leftM);
                    }
                    for (double pen = std::max(off, last); tabs.size() < 1000;) {
                        const double stop = (std::floor((pen + 0.01) / step) + 1) * step;
                        tabs << QTextOption::Tab((stop - off) * kFine, QTextOption::LeftTab);
                        if (stop - off > w) break;
                        pen = stop;
                    }
                    return tabs;
                }
                if (!hangStop(off, hangLine) && std::abs(off - std::round(off / step) * step) < 0.01) return tabs;
                for (double pen = off; tabs.size() < 1000;) {
                    const double stop = nextStop(pen, hangLine);
                    tabs << QTextOption::Tab((stop - off) * kFine, QTextOption::LeftTab);
                    if (stop - off > w) break;
                    pen = stop;
                }
                return tabs;
            };
            auto runFont = [&](int i) {
                const auto r = std::upper_bound(ranges.cbegin(), ranges.cend(), i, [](int at, const QTextLayout::FormatRange &x) { return at < x.start; });
                if (r != ranges.cbegin() && i < (r - 1)->start + (r - 1)->length) return (r - 1)->format.font();
                return base;
            };
            auto blank = [&](int i) { return B->disp[i] == QLatin1Char(' ') || B->disp[i] == QLatin1Char('\t'); };
            auto advanceAt = [&](int i) { return advanceOf(runFont(i), QString(B->disp[i])); };
            // A tab goes with the text after it: when that text doesn't fit, the
            // tab starts the next line (Publisher; Qt leaves it at the end of
            // this one), as do the tabs before it whose stops are past this
            // line's end. It is laid out as Qt does, and the next line starts
            // that far in (the tab's width, and the spaces after it) instead of
            // the character moving. That is done only if the text after the tab
            // (its first word) fits on the next line from there: when it doesn't
            // (a box 36 wide, a word 46 long in a line 60 wide) the line starts at
            // the left indent, as Qt sets it, and the tab stays where it is.
            int moveFrom = -1, moveTo = 0;   // the characters going on, from the tab to where the next line starts
            auto goesOn = [&](const PtLine &line, double off, double w, bool hangLine) {
                const int s0 = line.textStart(), endPos = s0 + line.textLength();
                if (endPos >= B->disp.size()) return -1;
                int w0 = endPos, last = -1;
                while (w0 > s0 && blank(w0 - 1)) {
                    --w0;
                    if (last < 0 && B->disp[w0] == QLatin1Char('\t')) last = w0;
                }
                if (last < 0) return -1;
                // The last tab of a group goes with the text after it: when the next
                // line starts with more blanks and a tab, that tab is the last, and
                // these stay, but for the ones whose stops are past this line's end.
                bool lastOnNext = false;
                for (int j = endPos; j < B->disp.size() && blank(j); ++j) lastOnNext |= B->disp[j] == QLatin1Char('\t');
                // From the line's text, not Qt's: a centered or right line's text is shifted, as the stops are not.
                double pen = off + (w0 > s0 ? line.cursorToX(w0) - line.cursorToX(s0) : 0);
                for (int i = w0; i < endPos; ++i) {
                    if (B->disp[i] != QLatin1Char('\t')) {
                        pen += advanceAt(i);
                        continue;
                    }
                    const double stop = nextStop(pen, hangLine);
                    if ((i == last && !lastOnNext) || stop - off > w + 0.01) return i > s0 ? i : -1;
                    pen = stop;
                }
                return -1;
            };
            // How far in a line from `left` to `right` (from the edge) begins, for what went on to it.
            auto movedBy = [&](double left, double right) {
                const int n = int(B->disp.size());
                double pen = left;
                int i = moveFrom;
                auto walk = [&](int to) {
                    for (; i < to; ++i) pen = B->disp[i] == QLatin1Char('\t') ? nextStop(pen, false) : pen + advanceAt(i);
                };
                walk(moveTo);
                const double shift = pen - left;
                // The blanks the line starts with, and the word after them: whole,
                // as a line may break at its soft hyphens only when it must.
                while (i < n && blank(i)) walk(i + 1);
                for (; i < n; ++i) {
                    const QChar ch = B->disp[i];
                    if (ch.isSpace() && ch.unicode() != 0x00A0 && ch.unicode() != 0x202F) break;
                    if (ch.unicode() == 0x00AD || ch.unicode() == 0x200B || ch.unicode() == 0x2060) continue;
                    pen += advanceAt(i);
                    if (ch == QLatin1Char('-')) break;
                }
                return pen <= right + 0.01 ? shift : 0;
            };
            if (gridTabs) B->tl->resetTabs();
            B->tl->beginLayout();
            int lineNo = 0;
            std::unique_ptr<QTextBoundaryFinder> lineBreaks;   // (where a line may break, made when a line first ends inside a word)
            // (The lines of a paragraph past the stretch's last frame are laid out only as far as the caller asks: the text after them is cut as a
            // window cuts it. Where the window cut it, or it was not, it is as the window left it.)
            B->cut = windowCut;
            B->cutAt = windowCutAt;
            B->markerClamp = 0;
            int overPlaced = 0;
            bool overFull = false;
            while (true) {
                PtLine line(B->tl->createLine());
                if (!line.isValid()) break;
                double indL = leftM, indR = rightM, lead = 0;
                if (lineNo == 0 || (startsWithBreak && !tail && lineNo == 1)) {
                    if (!B->marker.isEmpty()) indL = textIndent < 0 ? leftM : leftM + textIndent + markerW;
                    else indL = leftM + textIndent;
                    // A line that hangs starts where it has room, not left of
                    // the text area or inside a picture's text wrap; a list's
                    // text comes one hang after the bullet, wherever that is.
                    if (hangs) {
                        // (A word processor's goes out into the margin as far as its indents say.)
                        indL = env.wordLines ? std::max(leftM + textIndent, -(frames[f].outdentLimit > 0 ? frames[f].outdentLimit : frames[f].size.width())) : std::max(0.0, leftM + textIndent);
                        if (!B->marker.isEmpty()) lead = B->markerSkipsIndent ? leftM + textIndent + markerW - indL : -textIndent;   // (to the number's tab stop, or the indent)
                    }
                }
                if (B->dropLines > 0 && lineNo < (B->dropUp ? 1 : B->dropLines)) indL += B->dropWidth;
                if (lineNo == breakAfter && f < nF && !columnEmpty) advance();   // widow control: carry this line over

                bool placed = false;
                double lineHeightAtWalls = -1;   // (what the objects are checked against for this line, in a column: once, not at each step down)
                int measuredFrame = -1, measuredColumn = -1;
                for (int guard = 0; guard < 2000 && !placed; ++guard) {
                    if (f >= nF) {
                        line.setLineWidth(nF ? std::max(10.0, colRect(nF - 1, 0).width() - indL - indR) : 300);
                        line.setPosition(QPointF(indL + lead, m_frameY[nF] + overflowY));
                        overflowY += line.height();
                        B->lines << Line{-1, 0, QRectF(indL + lead, overflowY, line.naturalTextWidth(), line.height())};
                        // Blank paragraphs past the end don't count as overflow
                        // (as in .pub layouts): a box doesn't shrink or warn for them.
                        // (In a slide's text box that runs over, they count in the place the text is centered at: the presentation program counts
                        // the empty paragraph that ends a text in the height it centers.)
                        if (!B->disp.trimmed().isEmpty()) m_overflow = true;
                        else if (nF == 1 && frames[0].runsOver) m_blankPast = true;
                        placed = true;
                        break;
                    }
                    const QRectF col = colRect(f, c);
                    if (!rowActive) {
                        const double x0 = col.left() + indL, x1 = col.right() - indR;
                        // (A word processor's text that only touches an object's edge is clear of it, whichever way the last digits of two places that are the same round:
                        // the page that is drawn lays the text out again with the object's place made from another transform.)
                        const double touch = env.wordLines ? 0.001 : 0.0;
                        // (A word processor's line that the objects are checked against is as tall as the text on it makes it, not as its paragraph's first run does: the
                        // line a paragraph goes on with, in smaller type, is no taller than that.)
                        double wrapH = estText;
                        if (env.wordLines && !frames[f].obstacles.isEmpty()) {
                            if (lineHeightAtWalls < 0 || measuredFrame != f || measuredColumn != c) {
                                line.setLineWidth(std::max(1.0, x1 - x0));
                                lineHeightAtWalls = std::min(estText, wordLineOf(ranges, line.textStart(), std::max(1, line.textLength()), base, env, &B->disp, &markFont, markSize).total());
                                measuredFrame = f;
                                measuredColumn = c;
                            }
                            wrapH = lineHeightAtWalls;
                        }
                        QVector<Iv> ivs = freeIntervals(frames[f].obstacles, x0, x1, col.top() + y + touch, col.top() + y + wrapH - touch);
                        QVector<Iv> ok;
                        // (A word processor's empty paragraph is only its mark: it goes in any gap that is a few points wide, between two pictures a little apart as in a wider one.)
                        const double minW = env.wordLines && B->disp.trimmed().isEmpty() ? 2.0 : std::max(18.0, base.pointSizeF() / fontPointFactor() * 2.5);
                        // The first line of a list item starts a bullet's hang in from the
                        // first place it has: that room is part of what it needs.
                        for (const Iv &iv : ivs)
                            if (iv.x1 - iv.x0 >= std::min(minW + (lineNo == 0 && ok.isEmpty() ? lead : 0), x1 - x0)) ok << iv;
                        if (ok.isEmpty()) {
                            // (Steps of two points; a word processor's text goes straight to where the first room that blocks it ends, which is where it fits if that is
                            // the only one, as it is for a rectangle.)
                            double step = 2;
                            if (env.wordLines) {
                                double nearest = 1e18;
                                bool plain = true;
                                for (const QPolygonF &poly : frames[f].obstacles) {
                                    const QRectF pb = poly.boundingRect();
                                    if (pb.bottom() <= col.top() + y + touch || pb.top() >= col.top() + y + wrapH - touch || pb.right() <= x0 || pb.left() >= x1) continue;
                                    plain &= poly.size() <= 5 && std::all_of(poly.cbegin(), poly.cend(), [&](const QPointF &pt) {
                                        return (std::abs(pt.x() - pb.left()) < 1e-6 || std::abs(pt.x() - pb.right()) < 1e-6) && (std::abs(pt.y() - pb.top()) < 1e-6 || std::abs(pt.y() - pb.bottom()) < 1e-6);
                                    });
                                    nearest = std::min(nearest, pb.bottom() - col.top() - y);
                                }
                                if (plain && nearest < 1e17 && nearest > 0.001) step = nearest;
                            }
                            y += step;
                            rowPushed += step;
                            if (y + estH > col.height() - reserve.value(rkey(f, c)) + 0.01) advance();
                            continue;
                        }
                        row = ok; rowIdx = 0; rowH = 0; rowActive = true;
                    }
                    Iv iv = row[rowIdx];
                    if (lead > 0 && lineNo == 0 && rowIdx == 0) {
                        const double want = iv.x0 + lead;
                        iv.x0 = std::min(want, iv.x1);
                        B->markerClamp = env.wordLines ? want - iv.x0 : 0.0;
                    }
                    if (gridTabs) {
                        if (moveFrom >= 0) iv.x0 = std::min(iv.x0 + movedBy(iv.x0 - col.left(), iv.x1 - col.left()), iv.x1);
                        B->tl->setLineTabs(stopsFor(iv.x0 - col.left(), iv.x1 - iv.x0, lineNo == 0));
                    }
                    // (A line that is as wide as its room, to what rounding in the layout leaves, fits.)
                    const double room = std::max(1.0, iv.x1 - iv.x0);
                    line.setLineWidth(room + fit);
                    // A word wider than the line is broken where it must (Qt breaks it anywhere), and the letters that fit are as many as the room holds: a row of underscores as
                    // wide as the line is one line. A line that ends where a break may be (after a space, a dash, a slash) is no such, and fits as the others do.
                    if (env.wordLines) {
                        const int s0 = line.textStart(), e0 = s0 + line.textLength();
                        if (e0 > s0 && e0 < B->disp.size() && !B->disp[e0 - 1].isSpace() && !B->disp[e0].isSpace() && B->disp[e0 - 1] != QLatin1Char('-') && B->disp[e0 - 1].unicode() != 0x00AD &&
                            B->disp[e0 - 1].unicode() != 0x200B && B->disp[e0 - 1].unicode() != 0x2060) {
                            if (!lineBreaks) lineBreaks = std::make_unique<QTextBoundaryFinder>(QTextBoundaryFinder::Line, B->disp);
                            lineBreaks->setPosition(e0);
                            if (!lineBreaks->isAtBoundary()) line.setLineWidth(room + kQtPixelBack);
                        }
                    }
                    // A word processor's right tab stop that is past the paragraph's right indent and inside the column still takes the text after its tab (the page number of
                    // an entry in a table of contents, whose stop is at the margin where the paragraph keeps its right indent inside it): the line is as wide as the stop. What is
                    // before the last tab wraps at the line's room as any text does, and what is after it is that right-aligned text, ending at the stop: where the line is not
                    // so, it is the line the room makes.
                    if (env.wordLines && wordTabs && indR > 0.01) {
                        double farthest = 0;
                        for (const QTextOption::Tab &t : ownTabs) {
                            const double at = t.position * scale;
                            if (t.type == QTextOption::RightTab && at > col.width() - indR + 0.01 && at <= col.width() + 0.01) farthest = std::max(farthest, at);
                        }
                        if (farthest > 0) {
                            const double stop = col.left() + farthest - iv.x0;   // (from the line's start)
                            line.setLineWidth(stop + kExactFit);
                            const int s0 = line.textStart(), e0 = s0 + line.textLength();
                            int lastTab = -1;
                            for (int i = s0; i < e0; ++i)
                                if (B->disp[i] == QLatin1Char('\t')) lastTab = i;
                            int endText = e0;
                            while (endText > lastTab + 1 && B->disp[endText - 1].isSpace()) --endText;
                            const bool aligned = lastTab >= s0 && endText > lastTab + 1 && line.cursorToX(lastTab) - line.cursorToX(s0) <= room + kExactFit &&
                                                 std::abs(line.cursorToX(endText) - line.cursorToX(s0) - stop) < 1.0;
                            if (!aligned) line.setLineWidth(room + fit);
                        }
                    }
                    // A word processor lets a mark of punctuation at the end of a line hang past the margin (a period, a comma, a closing quote or bracket),
                    // so a word followed by one can be the last of a line it is a mark too wide for: the line is tried that much wider, and kept so if
                    // it is only that last mark that is past the margin. The text before the mark may fill the room exactly (the original program's lines go so on the practice
                    // documents, where a line that ends in a word stops a pixel short of its room, as the others here do).
                    bool markHangs = false;   // (a mark at the end of the line is past the margin)
                    if (env.wordLines && !env.wordNoHang && !bf.boolProperty(tp::NoHangPunct) && (al == Qt::AlignLeft || al == Qt::AlignJustify) && !bf.boolProperty(tp::Distribute)) {
                        const double hang = 0.34 * base.pointSizeF() / fontPointFactor();
                        const int endBefore = line.textStart() + line.textLength();
                        if (endBefore < B->disp.size()) {
                            line.setLineWidth(room + kQtPixelBack + hang);
                            const int s0 = line.textStart(), e0 = s0 + line.textLength();
                            int k = e0 - 1;
                            while (k > s0 && B->disp[k].isSpace()) --k;
                            const double end = line.cursorToX(k + 1);
                            if (end > room + fit - kQtPixelBack) {
                                static const QString marks = QStringLiteral(".,;:!?)]}\"'\u2019\u201D\u00BB\u2026");
                                markHangs = marks.contains(B->disp[k]) && line.cursorToX(k) <= room + 0.001 && k > s0;
                                if (!markHangs) line.setLineWidth(room + fit);
                            } else if (al == Qt::AlignJustify) {
                                line.setLineWidth(room + fit);   // (a justified line goes to the margin, not past it)
                            }
                        }
                    }
                    // Hyphenation zone: a word is broken only if moving it whole to
                    // the next line would leave more than the zone empty here.
                    if (hyphenate && zone > 0) {
                        const int s0 = line.textStart(), n = line.textLength();
                        if (n > 1 && B->disp[s0 + n - 1] == QChar(0x00AD)) {
                            int k = s0 + n - 1;
                            while (k > s0 && !B->disp[k - 1].isSpace()) --k;
                            // The width is given again: without it Qt treats the line
                            // as endless, and justified text spreads off the page.
                            if (k > s0 && (iv.x1 - iv.x0) - (line.cursorToX(k) - line.cursorToX(s0)) < zone) line.setNumColumns(k - s0, std::max(1.0, iv.x1 - iv.x0));
                        }
                    }
                    // In a polygon a word too long for the place left isn't broken to fit it: the line moves down to where it does.
                    if (frames[f].fitToShape && iv.x1 - iv.x0 < col.width() - indL - indR - 0.5) {
                        const int e = line.textStart() + line.textLength();
                        if (e > 0 && e < B->disp.size() && !B->disp[e - 1].isSpace() && B->disp[e - 1] != QChar(0x00AD) && B->disp[e - 1] != QLatin1Char('-') && !B->disp[e].isSpace()) {
                            y += 2;
                            rowActive = false;
                            if (y + estH > col.height() - reserve.value(rkey(f, c)) + 0.01) advance();
                            continue;
                        }
                    }
                    double single = singleOf(ranges, line.textStart(), std::max(1, line.textLength()), base);
                    if (line.textStart() == 0) single = std::max(single, markerSingle);
                    double h = lineHeightFor(bf, scale, single);
                    if (env.lineCut > 0 && (bf.lineHeightType() == QTextBlockFormat::SingleHeight || bf.lineHeightType() == QTextBlockFormat::ProportionalHeight)) {
                        const double share = bf.lineHeightType() == QTextBlockFormat::ProportionalHeight ? bf.lineHeight() : 100.0;
                        h = single * std::max(1.0, share - env.lineCut) / 100.0;
                    }
                    // .pub layouts put a line's extra spacing below it: the first
                    // line of a column sits one ascent below the top, and a
                    // paragraph after one with wider spacing starts that much
                    // lower. Whether a line fits counts its text, not that space.
                    // (A slide's goes mostly above it.)
                    double below = !env.slideLines && h > single && bf.lineHeightType() == QTextBlockFormat::ProportionalHeight ? h - single : 0;
                    double textH = h - below;
                    // Baseline one descent above the text's bottom (for a substituted
                    // proprietary font, the original font's descent). Set closer than
                    // single, the whole line shrinks in proportion, its descent too,
                    // small capitals at the full size: in Publisher's PDFs the baseline
                    // sits spacing x (single - descent) below the top (0.75 and 0.94
                    // spacing; Times, Futura, Oswald). A line of exactly so many points
                    // does the same when that is less than the font's own spacing (1 to
                    // 10 points of 10 point Times, Arial, and Calibri, and up to 24 of
                    // 24 point text; the lines are the value apart whatever the text).
                    const bool closer = (bf.lineHeightType() == QTextBlockFormat::ProportionalHeight || bf.lineHeightType() == QTextBlockFormat::FixedHeight) && h < single && single > 0;
                    const double kd = knownDescent(ranges, line.textStart(), std::max(1, line.textLength()), closer,
                                                   bf.lineHeightType() != QTextBlockFormat::FixedHeight && std::abs(h - single) < 0.01);
                    const double fd = kd > 0 ? 0 : fontDescent(ranges, line.textStart(), std::max(1, line.textLength()), closer);
                    const double sd = env.slideLines ? slideDescent(ranges, line.textStart(), std::max(1, line.textLength()), base) : 0;
                    const double descent = sd > 0 ? sd : kd > 0 ? kd : fd > 0 ? fd : line.height() - line.ascent();
                    double baseDown = closer && !(env.slideLines && bf.lineHeightType() != QTextBlockFormat::FixedHeight) ? h / single * (single - descent) : textH - descent;
                    // A slide's line that isn't single (a share of the line other than 100 percent, lnSpcReduction counted, or an exact height)
                    // has its baseline 3/4 of the way down it when it is taller than a single line, whatever the font (shares of 110 to
                    // 150 percent and exact heights of 30 and 40 points, 20 fonts). A shorter one keeps its descent, the font's, below the
                    // baseline, but not above 3/4 down (shares of 60 to 90 percent and exact heights of 15 and 24 points: a Calibri line at
                    // 70 percent sits at 3/4, an Arial one at 90 percent a descent above its bottom). The program's single line sits a
                    // descent above its bottom.
                    if (env.slideLines && (bf.lineHeightType() == QTextBlockFormat::FixedHeight ||
                                           ((bf.lineHeightType() == QTextBlockFormat::ProportionalHeight || bf.lineHeightType() == QTextBlockFormat::SingleHeight) && std::abs(h - single) >= 0.01)))
                        baseDown = h > single ? 0.75 * h : std::max(0.75 * h, h - descent);
                    // An object set in the line sits on the baseline with its wrap
                    // distances around it; one reaching above the text's ascent
                    // lowers the baseline, and the line, by the difference. The
                    // line's spacing stays the text's (Publisher, Oct 7: a 72 pt
                    // box in 12 pt Times gave the line 2.88 + 72 + 2.88 above the
                    // baseline, single or double spaced, and the next line its
                    // usual distance).
                    double objUp = 0;
                    for (const auto &ob : B->objects)
                        if (ob.disp >= line.textStart() && ob.disp < line.textStart() + line.textLength())
                            objUp = std::max(objUp, ob.pad.bottom() + ob.size.height() + ob.pad.top());
                    // A word processor's line: the text's ascent above the baseline, the descent and leading below it, and what a multiple of single
                    // spacing adds under that (an exact height keeps the line's parts in proportion, an at-least height the baseline a descent above the
                    // line's bottom). The whole line must fit.
                    double roomUnder = 0;   // (a word processor's multiple of single spacing puts its room under the text, which a line need not have left to be a column's)
                    if (env.wordLines) {
                        WordLine wl = wordLineOf(ranges, line.textStart(), std::max(1, line.textLength()), base, env, &B->disp, &markFont, markSize);
                        // (A number or bullet counts by its ascent over the text's descent, without a leading of its own or any room below its baseline: it raises the
                        // first line when that is taller than the text's line.)
                        if (line.textStart() == 0 && markerWord.ascent + wl.descent > wl.total()) {
                            wl.ascent = markerWord.ascent;
                            wl.leading = 0;
                        }
                        const double ownSingle = wl.total();
                        switch (bf.lineHeightType()) {
                        case QTextBlockFormat::ProportionalHeight: h = ownSingle * bf.lineHeight() / 100.0; roomUnder = std::max(0.0, h - ownSingle); break;
                        case QTextBlockFormat::FixedHeight: h = bf.lineHeight() * scale; break;
                        case QTextBlockFormat::MinimumHeight: h = std::max(ownSingle, bf.lineHeight() * scale); break;
                        default: h = ownSingle; break;
                        }
                        below = 0;
                        textH = h;
                        const bool bottom = bf.lineHeightType() == QTextBlockFormat::FixedHeight || (bf.lineHeightType() == QTextBlockFormat::MinimumHeight && h > ownSingle);
                        // (An exact height keeps the line's parts in proportion: the baseline is the ascent's share of the single line, of the exact height.)
                        baseDown = bf.lineHeightType() == QTextBlockFormat::FixedHeight && ownSingle > 0 ? wl.ascent * h / ownSingle : bottom ? std::max(0.0, h - wl.descent) : wl.ascent;
                        // A paragraph's first line that holds only a page break (the paragraph begins with it) takes no room: it is at the foot of the page
                        // the paragraph's text is to leave, whether or not there is room for a line there.
                        if (lineNo == 0 && line.textLength() == 1 && doc->characterAt(b.position() + B->docFromDisp(line.textStart())) == QChar(0x000C)) {
                            h = textH = 0;
                            baseDown = 0;
                            roomUnder = 0;
                        }
                    }
                    const double rise = objUp > baseDown && bf.lineHeightType() != QTextBlockFormat::FixedHeight ? objUp - baseDown : 0;
                    h += rise;
                    textH += rise;
                    baseDown += rise;
                    // A line of nothing but objects has no text above or below
                    // them: it is as tall as the tallest, inside its distances,
                    // whatever the paragraph's font, and single or wider spaced
                    // (Publisher, Oct 11: a 100 pt box alone in 12, 36, and 72
                    // pt text, single, 1.5, and double spaced, took 2.88 + 100 +
                    // 2.88; one 20 pt tall took 25.76, less than a text line).
                    if (objUp > 0 && (bf.lineHeightType() == QTextBlockFormat::SingleHeight || (bf.lineHeightType() == QTextBlockFormat::ProportionalHeight && bf.lineHeight() >= 100))) {
                        bool onlyObjects = true;
                        for (int k = line.textStart(); k < line.textStart() + line.textLength() && onlyObjects; ++k) {
                            if (B->disp[k].isSpace() || B->disp[k] == QChar::LineSeparator) continue;
                            onlyObjects = std::any_of(B->objects.cbegin(), B->objects.cend(), [k](const auto &ob) { return ob.disp == k; });
                        }
                        if (onlyObjects) {
                            h = textH = baseDown = objUp;
                            below = 0;
                            roomUnder = 0;
                        }
                    }
                    const double fitH = std::max(0.0, textH - roomUnder);   // (what of the line has to fit where the column ends)
                    // The first line of a column goes there however tall it
                    // is, unless notes going on from the column before fill it.
                    const bool firstInColumn = (y <= 0.001) && rowIdx == 0 && !reserve.contains(rkey(f, c));
                    // Footnotes referred to on this line go at the bottom of
                    // its column, with it: both fit, or both move on. Notes
                    // too tall for a whole column are cut instead: the line
                    // stays with the start of its note, and the rest goes on
                    // at the bottom of the next column.
                    QVector<int> lineNotes;
                    double pending = 0, notesH = 0;
                    // (A line in the frame that lays out what runs over the stretch isn't on it: its notes are not measured, or the lines of a paragraph
                    // full of notes cost each page all the notes it has left.)
                    const bool onStretch = m_span.stopAtFrame < 0 || f < m_span.stopAtFrame;
                    if (onStretch)
                        for (const auto &ref : B->noteRefs)
                            if (!m_notes[ref.second].endnote && ref.first >= line.textStart() && ref.first < line.textStart() + std::max(1, line.textLength()))
                                lineNotes << ref.second;
                    // A note still going on comes before any later one.
                    if (!lineNotes.isEmpty() && !carry.isEmpty()) {
                        advance();
                        continue;
                    }
                    if (!lineNotes.isEmpty()) {
                        if (!reserve.contains(rkey(f, c))) pending += ruleGap;
                        for (int ni : lineNotes) notesH += measureNote(m_notes[ni], col.width()) + noteGap;
                        pending += notesH;
                    }
                    double bottom = col.bottom() - reserve.value(rkey(f, c)) - pending;
                    bool cutting = false;
                    if (col.top() + y + fitH > bottom + 0.01) {
                        if (!lineNotes.isEmpty() && (env.wordLines || fitH + ruleGap + notesH > col.height() + 0.01)) {
                            // Room here for the line and a line of the note?
                            const double least = heightOf(m_notes[lineNotes.first()].numberFont) * (env.wordLines ? 3 : 1) + noteGap;   // (a word processor cuts a note that does not fit where three of its lines do)
                            cutting = firstInColumn || col.top() + y + fitH + (reserve.contains(rkey(f, c)) ? 0 : ruleGap) + least <= col.bottom() - reserve.value(rkey(f, c)) + 0.01;
                        }
                        if (!cutting && !firstInColumn) {
                            advance();
                            continue;
                        }
                    }
                    // Re-check wrap for a line taller than estimated.
                    const double inkH = rise > 0 ? textH : std::min(textH, line.height());
                    double band = estText;   // (the height of the line's text that the objects it goes round were kept out of)
                    if (inkH > estText * 1.05 && !frames[f].obstacles.isEmpty() && rowIdx == 0) {
                        band = inkH;
                        const double touch = env.wordLines ? 0.001 : 0.0;
                        const QVector<Iv> recheck = freeIntervals(frames[f].obstacles, iv.x0, iv.x1, col.top() + y + touch, col.top() + y + inkH - touch);
                        if (recheck.size() != 1 || recheck[0].x0 > iv.x0 + 0.5 || recheck[0].x1 < iv.x1 - 0.5) {
                            y += 2;
                            rowPushed += 2;
                            rowActive = false;
                            if (y + h > col.height() - reserve.value(rkey(f, c)) + 0.01) advance();
                            continue;
                        }
                    }
                    const double lead = baseDown - line.ascent();
                    // Align to baseline guides: the baseline moves down onto the next guide.
                    if (bf.boolProperty(tp::AlignToBaseline) && frames[f].baselineGrid > 0.5) {
                        const double grid = frames[f].baselineGrid * scale, origin = frames[f].baselineOrigin * scale;
                        const double base = col.top() + y + lead + line.ascent();
                        const double snapped = origin + std::ceil((base - origin) / grid - 1e-6) * grid;
                        if (snapped > base) y += snapped - base;
                    }
                    Cut cut;
                    if (cutting) {
                        const bool firstNotes = !reserve.contains(rkey(f, c));
                        cut = cutNotes(lineNotes, col.bottom() - reserve.value(rkey(f, c)) - (col.top() + y) - fitH - (firstNotes ? ruleGap : 0), col.width(), firstInColumn);
                        if (!cut.ok) {
                            advance();
                            continue;
                        }
                        bottom = col.bottom() - reserve.value(rkey(f, c)) - (firstNotes ? ruleGap : 0) - cut.used;
                    }
                    if (col.top() + y + fitH > bottom + 0.01 && !firstInColumn) {   // snapped past the bottom
                        advance();
                        continue;
                    }
                    line.setPosition(QPointF(iv.x0, m_frameY[f] + col.top() + y + lead));
                    if (const double dx = hangTrailingSpaces(line, B->disp, B->tl->textOption(), iv.x1 - iv.x0); dx != 0) line.setPosition(line.position() + QPointF(dx, 0));
                    if (B->lines.isEmpty()) {
                        B->boxLeft = col.left() + leftM;
                        B->boxRight = col.right() - rightM;
                    }
                    // A slide's or a word processor's justified line has its words stretched to the edge: the spaces that hang at its end are not part of
                    // the width the words are spread over (Qt counts them in it, which leaves the last word a space short of the edge). The line is laid
                    // out for the width with them added, and as it was if that takes another word in.
                    if ((env.slideLines || (env.wordLines && !markHangs)) && (B->tl->textOption().alignment() & Qt::AlignJustify)) {
                        const int s0 = line.textStart(), e = s0 + line.textLength();
                        if (e < B->disp.size() && B->disp[e - 1] != QChar::LineSeparator) {
                            double spaces = 0;
                            int k = e;
                            while (k > s0 && B->disp[k - 1] == QLatin1Char(' ')) spaces += advanceAt(--k);
                            if (k > s0 && spaces > 0) {
                                const double room = iv.x1 - iv.x0 + fit;
                                line.setLineWidth(room + spaces);
                                if (line.textStart() + line.textLength() != e) line.setLineWidth(room);
                            }
                        }
                    }
                    B->lines << Line{f, c, QRectF(iv.x0, col.top() + y, iv.x1 - iv.x0, h), below, rowPushed, band, lineNo == 0 && f == B->spaceFrame && c == B->spaceColumn ? B->spaceAbove : 0.0};
                    if (!lineNotes.isEmpty()) {
                        const bool firstNotes = !reserve.contains(rkey(f, c));
                        double &room = reserve[rkey(f, c)];
                        if (firstNotes) room += ruleGap;
                        for (int k = 0; k < lineNotes.size(); ++k) {
                            Note &n = m_notes[lineNotes[k]];
                            if (cut.at >= 0 && k > cut.at) {   // after the one cut: first in the next column
                                carry << lineNotes[k];
                                continue;
                            }
                            n.frame = f;
                            n.column = c;
                            if (k == cut.at) {
                                n.layout = cut.layout;
                                n.height = cut.height;
                                n.from = cut.rest;
                                if (cut.rest >= 0) carry << lineNotes[k];
                            }
                            room += n.height + noteGap;
                        }
                    }
                    m_used[f] = std::max(m_used[f], col.top() + y + textH);
                    columnEmpty = false;
                    rowH = std::max(rowH, h);
                    if (++rowIdx >= row.size()) { y += rowH; rowActive = false; rowPushed = 0; }
                    if (gridTabs) {
                        B->tl->keepLineTabs(lineNo);
                        moveFrom = wordTabs ? -1 : goesOn(line, iv.x0 - col.left(), iv.x1 - iv.x0, lineNo == 0);
                        moveTo = line.textStart() + line.textLength();
                    }
                    placed = true;
                    if (m_span.overLines > 0 && m_span.stopAtFrame >= 0 && f >= m_span.stopAtFrame && ++overPlaced >= m_span.overLines) {
                        overFull = true;
                        B->cutAt = B->docFromDisp(line.textStart() + line.textLength());
                    }
                }
                ++lineNo;
                if (overFull) {
                    B->cut = true;
                    break;
                }
            }
            B->tl->endLayout();
            if (rowActive) { y += rowH; rowActive = false; }
            rowPushed = 0;
            if (f < nF) {
                y += B->boxBottomRoom;   // (the line below it)
                y += after;
                lastAfter = after;
                lastRoom = B->boxBottomRoom + after;
            }
        };
        // (The first paragraph of a stretch may have lines that went before it.)
        const int firstBreak = first ? m_span.firstBreakAfter : -1;
        // A paragraph whose text and mark are both hidden is no paragraph: it takes no room.
        // A paragraph that shows nothing and follows one that shows text and whose mark a tracked change deleted is the rest of that one (the two are one
        // paragraph without the mark): it takes no room.
        auto showsText = [](const QTextBlock &x) {
            for (QTextBlock::iterator it = x.begin(); !it.atEnd(); ++it) {
                const QTextCharFormat cf = it.fragment().charFormat();
                if (!cf.boolProperty(tp::HiddenText) && cf.stringProperty(tp::Field) != QLatin1String("raw:")) return true;
            }
            return false;
        };
        const bool restOfDeleted = env.wordLines && B->disp.isEmpty() && !tail && !b.charFormat().boolProperty(tp::HiddenText) && b.previous().isValid() &&
                                   b.previous().charFormat().boolProperty(tp::HiddenText) && !b.previous().blockFormat().boolProperty(tp::OutOfLayout) && showsText(b.previous());
        if (!(env.wordLines && B->disp.isEmpty() && (b.charFormat().boolProperty(tp::HiddenText) || apart) && !tail) && !restOfDeleted) placeLines(false, firstBreak);
        // Where each line landed: the first line in another column or box.
        auto splitAt = [&] {
            for (int i = 1; i < B->lines.size(); ++i)
                if (B->lines[i].frame != B->lines[0].frame || B->lines[i].column != B->lines[0].column) return i;
            return -1;
        };
        auto redo = [&](bool startNext, int breakAfter) {
            f = startSpot.f; c = startSpot.c; y = startSpot.y; overflowY = startSpot.overflowY; rowH = startSpot.rowH; row = startSpot.row;
            rowIdx = startSpot.rowIdx; rowActive = startSpot.rowActive; rowPushed = 0; columnEmpty = startSpot.columnEmpty; m_overflow = startSpot.overflow; m_blankPast = startSpot.blank; m_used = startSpot.used;
            reserve = startSpot.reserve;
            m_notes = startSpot.notes;
            carry = startSpot.carry;
            seeded = startSpot.seeded;
            B->lines.clear();
            placeLines(startNext, breakAfter < 0 ? firstBreak : breakAfter);
        };
        // A justified line of a word processor's text that may give up a share of each space (wordSpaceShrink) takes in the next word when it fits so: the lines are cut as the
        // option says, and the paragraph is laid out again with the spaces of each line that needs it narrower by what it needs. Where Qt does not cut the lines as that did,
        // the paragraph is as it was. Only a paragraph with a line that could take the next word is worked on, and those lines are found from the lines as they are laid out.
        bool shrinkDone = false;
        if (B->shrunk) {
            const ShrunkParagraph &kept = *B->shrunk;
            bool same = !B->cut && B->lines.size() == kept.starts.size() && !B->lines.isEmpty() && B->lines.first().frame >= 0;
            for (int i = 0; same && i < kept.starts.size(); ++i)
                same = PtLine(B->tl->lineAt(i)).textStart() == kept.starts[i] && std::abs(B->lines[i].rect.width() - kept.rooms[i]) < 0.01;
            if (same) {
                ++s_shrinkShared;
                shrinkDone = true;
            } else {   // (the room is not what it was: the paragraph as it was, and worked on)
                B->tl->setFormats(fineRanges(ranges));
                redo(false, -1);
            }
            B->shrunk.reset();
        }
        if (!shrinkDone && env.wordLines && env.wordSpaceShrink > 0 && (al & Qt::AlignJustify) && !bf.boolProperty(tp::Distribute) && !B->cut && !hyphenate && B->lines.size() > 1 &&
            B->lines.first().frame >= 0 && B->disp.size() <= 2000 && !B->disp.contains(QLatin1Char('\t')) && !B->disp.contains(QChar::ObjectReplacementCharacter) && B->objects.isEmpty()) {
            const int n = int(B->disp.size());
            const double fitOffset = B->lettersAdjusted || !m_span.keepRules ? 0.0 : kRoomOffset;   // (as the lines were laid out)
            const QVector<QTextLayout::FormatRange> fine = [&] {
                QVector<QTextLayout::FormatRange> out;
                out.reserve(ranges.size());
                for (QTextLayout::FormatRange r : ranges) {
                    r.format = fineFormat(r.format);
                    out << r;
                }
                return out;
            }();
            // The advance of a character as Qt sets it: its glyph's, the letter spacing after it, and a space's word spacing (in layout units); the metrics of each range once.
            struct RunMetrics {
                std::unique_ptr<QFontMetricsF> fm;
                double ascii[128];
                RunMetrics() { std::fill(std::begin(ascii), std::end(ascii), -1.0); }
            };
            std::vector<RunMetrics> metrics(size_t(fine.size()));
            int lastRange = -1;
            auto advance = [&](int i) {
                int r = lastRange;
                if (r < 0 || i < fine[r].start || i >= fine[r].start + fine[r].length) {
                    const auto it = std::upper_bound(fine.cbegin(), fine.cend(), i, [](int at, const QTextLayout::FormatRange &x) { return at < x.start; });
                    r = it == fine.cbegin() ? -1 : int(it - fine.cbegin()) - 1;
                    if (r >= 0 && i >= fine[r].start + fine[r].length) r = -1;
                }
                lastRange = r;
                const QChar ch = B->disp[i];
                ++s_shrinkLooked;
                if (r < 0) return advanceOf(base, QString(ch));
                RunMetrics &m = metrics[size_t(r)];
                if (!m.fm) m.fm = std::make_unique<QFontMetricsF>(fine[r].format.font());
                if (ch.unicode() < 128 && m.ascii[ch.unicode()] >= 0) return m.ascii[ch.unicode()];
                const QTextCharFormat &f = fine[r].format;
                const double a = (m.fm->horizontalAdvance(ch) + (f.fontLetterSpacingType() == QFont::AbsoluteSpacing ? f.fontLetterSpacing() : 0.0) +
                                  (ch == QLatin1Char(' ') ? f.fontWordSpacing() : 0.0)) / kFine;
                if (ch.unicode() < 128) m.ascii[ch.unicode()] = a;
                return a;
            };
            // The lines that could take the next word if their spaces gave way: the room left by the line as Qt set it (its natural width without the spaces at its end) and
            // the share of its spaces, against the width of the word that begins the next.
            bool could = false;
            for (int k = 0; k + 1 < B->lines.size() && !could; ++k) {
                const PtLine line(B->tl->lineAt(k));
                const int s0 = line.textStart(), e0 = s0 + line.textLength();
                if (e0 >= n || e0 <= s0 || B->disp[e0 - 1] == QChar::LineSeparator) continue;
                int te = e0;
                double trailing = 0;
                while (te > s0 && B->disp[te - 1].isSpace()) trailing += advance(--te);
                double spaces = trailing;
                for (int i = s0; i < te; ++i)
                    if (B->disp[i] == QLatin1Char(' ')) spaces += advance(i);
                double word = 0;
                for (int i = e0; i < n && !B->disp[i].isSpace() && i < e0 + 40; ++i) word += advance(i);
                const double room = std::max(1.0, B->lines[k].rect.width());
                could = line.naturalTextWidth() + word <= room + fitOffset + env.wordSpaceShrink * spaces;   // (its natural width has the spaces at its end)
            }
            if (could) {
                ++s_shrinkPasses;
                // Where each character begins on one long line of the paragraph (the spaces whole), and the room the spaces before it take.
                QVector<double> at(n + 1, 0.0), spaces(n + 1, 0.0);
                s_shrinkFull += n;
                for (int i = 0; i < n; ++i) {
                    const double a = advance(i);
                    at[i + 1] = at[i] + a;
                    spaces[i + 1] = spaces[i] + (B->disp[i] == QLatin1Char(' ') ? a : 0.0);
                }
                QVector<int> allowed;   // (where a line may begin)
                QTextBoundaryFinder finder(QTextBoundaryFinder::Line, B->disp);
                for (int p = finder.toNextBoundary(); p > 0 && p < n; p = finder.toNextBoundary()) allowed << p;
                allowed << n;
                struct Cut { int start = 0, end = 0, textEnd = 0; double share = 0, narrower = 0; };   // (share of a space given up; how much narrower each space of the line is)
                QVector<Cut> cuts;
                bool any = false, usable = at[n] > 0;
                for (int pos = 0, k = 0; usable && pos < n && k <= n; ++k) {
                    const double room = std::max(1.0, B->lines[std::min(k, int(B->lines.size()) - 1)].rect.width()), limit = room + fitOffset;
                    int hard = n;
                    for (int i = pos; i < n; ++i)
                        if (B->disp[i] == QChar::LineSeparator) {
                            hard = i + 1;
                            break;
                        }
                    Cut cut;
                    cut.start = pos;
                    for (auto it = std::upper_bound(allowed.cbegin(), allowed.cend(), pos); it != allowed.cend(); ++it) {
                        const int e = *it;
                        if (e > hard) break;
                        int te = e;
                        while (te > pos && B->disp[te - 1].isSpace()) --te;
                        const double natural = at[te] - at[pos], give = env.wordSpaceShrink * (spaces[te] - spaces[pos]);
                        if (natural > limit + give) break;
                        cut.end = e;
                        cut.textEnd = te;
                    }
                    if (cut.end <= pos) {   // (not even the first word fits: Qt breaks it where it must)
                        usable = false;
                        break;
                    }
                    const double natural = at[cut.textEnd] - at[pos], taken = spaces[cut.textEnd] - spaces[pos];
                    if (natural > limit && taken > 0) {
                        cut.share = std::min(env.wordSpaceShrink, (natural - limit + 0.15) / taken);
                        cut.narrower = cut.share * taken / std::max(1, int(std::count(B->disp.cbegin() + cut.start, B->disp.cbegin() + cut.textEnd, QLatin1Char(' '))));
                        any = true;
                    }
                    cuts << cut;
                    pos = cut.end;
                }
                if (usable && any) {
                    // The spaces of the lines that give way, narrower. Qt adds a word spacing once to a run of spaces, so each space of a run has a range of its own; the rest of a line
                    // is one.
                    struct Piece { int start, end; double narrower; };
                    QVector<Piece> pieces;
                    for (const Cut &cut : cuts) {
                        if (cut.share <= 0) continue;
                        int from = cut.start;
                        for (int i = cut.start; i < cut.textEnd; ++i) {
                            if (B->disp[i] != QLatin1Char(' ') || !((i > 0 && B->disp[i - 1] == QLatin1Char(' ')) || (i + 1 < n && B->disp[i + 1] == QLatin1Char(' ')))) continue;
                            if (i > from) pieces << Piece{from, i, cut.narrower};
                            pieces << Piece{i, i + 1, cut.narrower};
                            from = i + 1;
                        }
                        if (cut.textEnd > from) pieces << Piece{from, cut.textEnd, cut.narrower};
                    }
                    QVector<QTextLayout::FormatRange> narrowed;
                    int next = 0;
                    for (const QTextLayout::FormatRange &r : ranges) {
                        int from = r.start;
                        const int to = r.start + r.length;
                        while (next < pieces.size() && pieces[next].end <= from) ++next;
                        for (int g = next; g < pieces.size() && pieces[g].start < to; ++g) {
                            const int a = std::max(from, pieces[g].start), b = std::min(to, pieces[g].end);
                            if (b <= a) continue;
                            if (a > from) {
                                QTextLayout::FormatRange before = r;
                                before.start = from;
                                before.length = a - from;
                                narrowed << before;
                            }
                            QTextLayout::FormatRange m = r;
                            m.start = a;
                            m.length = b - a;
                            m.format.setFontWordSpacing(r.format.fontWordSpacing() - pieces[g].narrower);
                            narrowed << m;
                            from = b;
                        }
                        if (from < to) {
                            QTextLayout::FormatRange z = r;
                            z.start = from;
                            z.length = to - from;
                            narrowed << z;
                        }
                    }
                    B->tl->setFormats(fineRanges(narrowed));
                    ++s_shrinkRelaid;
                    redo(false, -1);
                    bool same = B->lines.size() == cuts.size();
                    for (int i = 0; same && i < cuts.size(); ++i) same = PtLine(B->tl->lineAt(i)).textStart() == cuts[i].start;
                    if (!same) {
                        B->tl->setFormats(fineRanges(ranges));
                        redo(false, -1);
                    } else if (!B->shrinkKey.isEmpty()) {
                        auto *kept = new ShrunkParagraph;
                        kept->ranges = narrowed;
                        for (int i = 0; i < cuts.size(); ++i) {
                            kept->starts << cuts[i].start;
                            kept->rooms << B->lines[i].rect.width();
                        }
                        shrinkMemory().insert(B->shrinkKey, kept);
                    }
                }
            }
        }
        // Only when there is somewhere to go: a later column or text box.
        const bool roomAhead = startSpot.f < nF && (startSpot.c + 1 < std::max(1, frames[startSpot.f].columns) || startSpot.f + 1 < nF);
        if (roomAhead && m_span.keepRules && !startSpot.columnEmpty && B->lines.size() > 1 && B->lines.first().frame >= 0) {
            const int at = splitAt();
            // (A paragraph that goes on at the next page because a page break ends a line is not cut for want of room: no rule of the lines applies.)
            bool broken = false;
            if (at > 0 && env.wordLines) {
                const PtLine before(B->tl->lineAt(at - 1));
                const int last = before.textStart() + before.textLength() - 1;
                // (The display shows a page break as a line break. What stands between the last letter and the break and shows nothing, an anchored object, is the break's too.)
                if (last >= 0)
                    for (int k = B->docFromDisp(last), to = std::max(B->docFromDisp(last + 1), k + 1); k < to && !broken; ++k) broken = doc->characterAt(B->docStart + k) == QChar(0x000C);
            }
            const bool split = at > 0 && B->lines[at].frame >= 0 && !broken;
            if (split && bf.boolProperty(tp::KeepTogether)) redo(true, -1);                 // keep lines together
            else if (split && at == 1 && bf.boolProperty(tp::WidowControl)) redo(true, -1);  // an orphan: one line left behind
            else if (split && int(B->lines.size()) - at == 1 && at >= 2 && bf.boolProperty(tp::WidowControl))
                redo(false, at - 1);                                                          // a widow: one line carried over
        }
        // Distribute: the last line is spread across the line too (Qt
        // justifies every line but the last), by spacing its letters out.
        if (bf.boolProperty(tp::Distribute) && !B->lines.isEmpty() && B->lines.last().frame >= 0) {
            const int li = int(B->lines.size()) - 1;
            const PtLine last(B->tl->lineAt(li));
            int n = last.textLength();
            while (n > 0 && B->disp.at(last.textStart() + n - 1).isSpace()) --n;
            const double room = B->lines[li].rect.width() - last.naturalTextWidth();
            if (n > 1 && room > 0.5) {
                QList<QTextLayout::FormatRange> fmts = B->tl->formats();
                const double extra = room * 0.995 / (n - 1);
                QList<QTextLayout::FormatRange> spread;
                for (const auto &r : fmts) {
                    // Split each range at the last line's start; spacing on its part.
                    const int s0 = r.start, e0 = r.start + r.length, ls = last.textStart(), le = last.textStart() + n - 1;
                    if (e0 <= ls || s0 >= le) { spread << r; continue; }
                    if (s0 < ls) { auto a = r; a.length = ls - s0; spread << a; }
                    auto mid = r;
                    mid.start = std::max(s0, ls);
                    mid.length = std::min(e0, le) - mid.start;
                    QFont mf = mid.format.font();
                    mid.format.setFontLetterSpacingType(QFont::AbsoluteSpacing);
                    mid.format.setFontLetterSpacing((mf.letterSpacingType() == QFont::AbsoluteSpacing ? mf.letterSpacing() : 0) + extra * kFine);
                    spread << mid;
                    if (e0 > le) { auto z = r; z.start = le; z.length = e0 - le; spread << z; }
                }
                B->tl->setFormats(spread);
                redo(false, -1);
            }
        }
        // Keep with next: when the next paragraph's first line won't fit
        // after this one, this one moves on with it.
        if (roomAhead && m_span.keepRules && bf.boolProperty(tp::KeepWithNext) && !startSpot.columnEmpty && b.next().isValid() && f < nF && !B->lines.isEmpty() && B->lines.first().frame >= 0) {
            const QTextBlock nb = b.next();
            const QTextBlockFormat nbf = nb.blockFormat();
            const double nextH = (nbf.topMargin() * scale) + std::max(1.0, lineHeightFor(nbf, scale, singleOf({}, 0, 0, baseFontFor(nb, env))));
            const QRectF col = colRect(f, c);
            if (y + nextH > col.height() + 0.01 && B->lines.first().frame == f && B->lines.first().column == c) redo(true, -1);
        }
        // A paragraph out of the flow (a frame's, in a word processor) is where the next begins, and leaves the room as it was.
        if (env.wordLines && bf.boolProperty(tp::OutOfFlow) && f == startSpot.f && c == startSpot.c && !B->cut) {
            y = startSpot.y;
            lastAfter = prevAfter;
            lastRoom = prevRoom;
            rowActive = startSpot.rowActive;
            rowPushed = 0;
            rowIdx = startSpot.rowIdx;
            rowH = startSpot.rowH;
        }
        const bool cutBlock = B->cut;
        m_blocks.push_back(std::move(B));
        if (cutBlock) {
            // The paragraph goes on past the text that was laid out, so nothing after it can be: if its lines are not yet at the frame laying
            // out stops at, the window was too small for what is wanted of it.
            m_windowShort = m_span.stopAtFrame < 0 || f < m_span.stopAtFrame;
            stoppedEarly = true;
            break;
        }
        // (A paragraph with no lines, hidden or set apart, does not end it: the text after it is what the stretch is for.)
        if (m_span.stopAtFrame >= 0 && f >= m_span.stopAtFrame && !(m_blocks.back()->lines.isEmpty() && env.wordLines)) {
            stoppedEarly = b.next().isValid();
            break;
        }
    }

    m_trailingAfter = env.wordLines ? lastAfter : 0.0;
    m_trailingRoom = lastRoom;
    // A footnote still going on after the story's last line: the columns
    // after it take the rest, or the story overflows. (A stretch that stops at a frame leaves the rest to the page after it, which lays them out: not in the frame that
    // runs over, where a page with thousands of notes going on would lay out all of them.)
    while (!carry.isEmpty() && f < nF) {
        if (m_span.stopAtFrame >= 0 && f + 1 >= m_span.stopAtFrame) break;
        advance();
    }
    if (!carry.isEmpty()) {
        m_overflow = true;
        for (int ni : carry) m_notes[ni].goesOn = true;
    }

    // Endnotes: after the story's last line, under a heading, in order;
    // each one whole in a column (a note too tall for an empty column
    // starts there anyway).
    if (withNotes && endCount > m_span.endnotesBefore && !stoppedEarly) {
        auto place = [&](double h, double gapBefore, int *frameOut) -> QRectF {
            while (f < nF) {
                const QRectF col = colRect(f, c);
                const double top = y > 0.001 ? y + gapBefore : 0;
                if (col.top() + top + h <= col.bottom() - reserve.value(rkey(f, c)) + 0.01 || (y <= 0.001 && !reserve.contains(rkey(f, c)))) {
                    *frameOut = f;
                    const QRectF r(col.left(), col.top() + top, col.width(), h);
                    y = top + h;
                    columnEmpty = false;
                    m_used[f] = std::max(m_used[f], r.bottom());
                    return r;
                }
                advance();
            }
            *frameOut = -1;
            m_overflow = true;
            return {};
        };
        const QFont lastFont = baseFontFor(doc->lastBlock(), env);
        QFont headFont = resolveCharFormat(QTextCharFormat(), env).font();
        headFont.setPointSizeF(lastFont.pointSizeF());
        headFont.setFamilies(lastFont.families());
        headFont.setBold(true);
        const double headH = heightOf(headFont) * 1.4;
        int hf = -1;
        const QRectF hr = place(headH, 12 * scale, &hf);
        if (hf >= 0) {
            m_notesHeading.frame = hf;
            m_notesHeading.font = headFont;
            m_notesHeading.color = resolveCharFormat(QTextCharFormat(), env).foreground().color();
            m_notesHeading.baseline = QPointF(hr.left(), hr.top() + QFontMetricsF(fineFont(headFont)).ascent() / kFine);
        }
        for (Note &n : m_notes) {
            if (!n.endnote) continue;
            const double h = measureNote(n, f < nF ? colRect(f, c).width() : 300);
            int nf2 = -1;
            const QRectF r = place(h, noteGap * 2, &nf2);
            // A narrower column than measured for: measured again there.
            if (nf2 >= 0 && std::abs(r.width() - n.width) > 0.01) measureNote(n, r.width());
            n.frame = nf2;
            n.rect = nf2 >= 0 ? QRectF(r.topLeft(), QSizeF(r.width(), n.height)) : QRectF();
        }
    }
    m_noteReserves = QVector<double>(nF, 0.0);
    for (auto it = reserve.cbegin(); it != reserve.cend(); ++it)
        if (it.key() % 64 == 0 && it.key() / 64 >= 0 && it.key() / 64 < nF) m_noteReserves[it.key() / 64] = it.value();
    // Footnotes: stacked at the bottom of their column, under a short rule,
    // or, when one goes on from the column before, as wide as the column.
    for (auto it = reserve.cbegin(); it != reserve.cend(); ++it) {
        const int fi = it.key() / 64, ci = it.key() % 64;
        if (fi < 0 || fi >= nF) continue;
        const QRectF col = colRect(fi, ci);
        const double areaTop = col.bottom() - it.value();
        const double gap = gapOf.value(it.key(), ruleGap);
        double ny = areaTop + gap - noticeGap;
        bool goesOn = false, first = true;
        for (Note &n : m_notes) {
            if (n.endnote) continue;
            if (n.frame == fi && n.column == ci) {
                n.rect = QRectF(col.left(), ny, col.width(), n.height);
                ny += n.height + noteGap;
                first = false;
            }
            for (Note::Piece &pc : n.more) {
                if (pc.frame != fi || pc.column != ci) continue;
                pc.rect = QRectF(col.left(), ny, col.width(), pc.height);
                ny += pc.height + noteGap;
                goesOn |= first;
                first = false;
            }
        }
        // (A word processor's rule is two inches, and is set in its separator paragraph three fifths of the way down it.)
        const double ruleY = areaTop + (gap - noticeGap) * (env.wordLines ? 0.6 : 0.45), ruleW = goesOn ? col.width() : env.wordLines ? std::min(col.width(), 144.0) : col.width() / 3;
        m_noteRules << Rule{fi, QLineF(col.left(), ruleY, col.left() + ruleW, ruleY)};
        m_used[fi] = std::max(m_used[fi], col.bottom());
    }

    // Marker x for hanging lists: the line's left edge minus the hang.
    for (auto &B : m_blocks) {
        if (B->marker.isEmpty() || B->lines.isEmpty()) continue;
        const PtLine l0(B->tl->lineAt(0));
        const double lineX = l0.position().x();
        QTextBlock tb = doc->findBlock(B->docStart);
        const QTextBlockFormat bf = tb.blockFormat();
        const double ti = bf.textIndent() * scale, lm = bf.leftMargin() * scale;
        B->markerX = (ti < 0 && !B->markerSkipsIndent ? lineX + ti : lineX - B->markerOff) + B->markerClamp;
        Q_UNUSED(lm);
    }

    // Vertical alignment for single-column frames.
    for (int fi = 0; fi < nF; ++fi) {
        const FrameSpec &fs = frames[fi];
        if (fs.valign == VAlign::Top || fs.columns > 1) continue;
        const QRectF col = colRect(fi, 0);
        double top = 1e18, bottom = -1e18;
        for (const auto &B : m_blocks)
            for (const Line &l : B->lines)
                if (l.frame == fi) { top = std::min(top, l.rect.top()); bottom = std::max(bottom, l.rect.bottom() - l.below); }
        if (bottom < top) continue;
        for (const Note &n : m_notes)
            if (n.endnote && n.frame == fi) bottom = std::max(bottom, n.rect.bottom());
        const double usedH = bottom - col.top();
        const double shift = (col.height() - reserve.value(rkey(fi, 0)) - usedH) * (fs.valign == VAlign::Middle ? 0.5 : 1.0);
        if (shift <= 0) continue;
        for (Note &n : m_notes)
            if (n.endnote && n.frame == fi) n.rect.translate(0, shift);
        if (m_notesHeading.frame == fi) m_notesHeading.baseline += QPointF(0, shift);
        for (auto &B : m_blocks)
            for (int i = 0; i < B->lines.size(); ++i)
                if (B->lines[i].frame == fi) {
                    PtLine tl(B->tl->lineAt(i));
                    tl.setPosition(tl.position() + QPointF(0, shift));
                    B->lines[i].rect.translate(0, shift);
                }
        m_used[fi] += shift;
        shifts[fi] = shift;
    }
    for (int fi = 0; fi < nF; ++fi) m_used[fi] += frames[fi].insets.bottom();

    // Text anchored centered: the block of a frame's lines (the widest of them, and the bullets before them) sits in the middle of the
    // frame, each line aligned in it as its paragraph says. A block as wide as the frame stays where it is.
    for (int fi = 0; fi < nF; ++fi) {
        if (!frames[fi].centerBlock || frames[fi].columns > 1) continue;
        const QRectF col = colRect(fi, 0);
        double left = 1e18, right = -1e18;
        for (const auto &B : m_blocks) {
            bool marked = false;
            for (int i = 0; i < B->lines.size(); ++i) {
                if (B->lines[i].frame != fi) continue;
                const PtLine tl(B->tl->lineAt(i));
                // The line's text without the spaces that hang at its end (an empty line is the place its paragraph starts at), in the
                // frame's coordinates (the line's place is in them).
                const int from = tl.textStart();
                int to = from + tl.textLength();
                while (to > from && B->disp[to - 1] == QLatin1Char(' ')) --to;
                const double x0 = tl.cursorToX(from), x1 = tl.cursorToX(to);
                left = std::min(left, std::min(x0, x1));
                right = std::max(right, std::max(x0, x1));
                marked |= i == 0 && !B->marker.isEmpty();
            }
            if (marked) left = std::min(left, B->markerX);
        }
        if (right < left) continue;
        const double shift = (col.left() + col.right() - left - right) / 2;
        if (std::abs(shift) < 0.01) continue;
        for (auto &B : m_blocks) {
            bool any = false;
            for (int i = 0; i < B->lines.size(); ++i)
                if (B->lines[i].frame == fi) {
                    PtLine tl(B->tl->lineAt(i));
                    tl.setPosition(tl.position() + QPointF(shift, 0));
                    B->lines[i].rect.translate(shift, 0);
                    any = true;
                }
            if (any && !B->marker.isEmpty()) B->markerX += shift;
        }
    }

    // Where each object set in the text is drawn: on its line's baseline,
    // inside its wrap distances.
    for (const auto &B : m_blocks)
        for (const auto &ob : B->objects)
            for (int i = 0; i < B->lines.size(); ++i) {
                const PtLine l(B->tl->lineAt(i));
                if (ob.disp < l.textStart() || ob.disp >= l.textStart() + l.textLength()) continue;
                const double x = std::min(l.cursorToX(ob.disp), l.cursorToX(ob.disp + 1)) + ob.pad.left();
                const double baseline = l.y() - frameY(B->lines[i].frame) + l.ascent();
                m_inline << InlineObject{B->lines[i].frame, QRectF(QPointF(x, baseline - ob.pad.bottom() - ob.size.height()), ob.size), ob.json, ob.docPos};
                break;
            }
    return shifts;
}

QVector<QRectF> StoryLayout::lineRects(int frame) const
{
    QVector<QRectF> v;
    for (const auto &B : m_blocks)
        for (const Line &l : B->lines)
            if (l.frame == frame) v << l.rect;
    return v;
}

QVector<StoryLayout::LineInfo> StoryLayout::lineInfo(int frame) const
{
    QVector<LineInfo> v;
    for (const auto &B : m_blocks)
        for (int i = 0; i < B->lines.size(); ++i) {
            if (B->lines[i].frame != frame) continue;
            const PtLine l(B->tl->lineAt(i));
            LineInfo li;
            li.rect = B->lines[i].rect;
            li.below = B->lines[i].below;
            li.above = B->lines[i].above;
            li.baseline = l.y() - frameY(B->lines[i].frame) + l.ascent();
            if (i == 0 && !B->marker.isEmpty()) li.markerX = B->markerX;
            li.text = B->disp.mid(l.textStart(), l.textLength());
            li.docStart = B->docStart;
            li.docPos = B->docStart + B->docFromDisp(l.textStart());
            li.docEnd = B->docStart + B->docFromDisp(l.textStart() + l.textLength());
            for (const auto &r : B->tl->formats())
                if (r.start <= l.textStart() && r.start + r.length > l.textStart()) {
                    li.family = r.format.font().family() + QStringLiteral(" w%1%2").arg(r.format.font().weight()).arg(r.format.font().italic() ? QStringLiteral(" italic") : QString());
                    li.pointSize = r.format.fontPointSize() / kFine / fontPointFactor();
                }
            v << li;
        }
    return v;
}

QVector<StoryLayout::LineBrief> StoryLayout::lineBriefs() const
{
    QVector<LineBrief> v;
    for (const auto &B : m_blocks)
        for (int i = 0; i < B->lines.size(); ++i) {
            const PtLine l(B->tl->lineAt(i));
            LineBrief b;
            b.frame = B->lines[i].frame;
            b.block = B->docStart;
            b.line = i;
            b.lines = int(B->lines.size());
            b.pos = B->docStart + B->docFromDisp(l.textStart());
            b.end = B->docStart + B->docFromDisp(l.textStart() + l.textLength());
            b.rect = B->lines[i].rect;
            b.below = B->lines[i].below;
            b.pushed = B->lines[i].pushed;
            b.above = B->lines[i].above;
            b.band = B->lines[i].band;
            b.cut = B->cut;
            if (B->cut && i + 1 == B->lines.size()) b.end = std::min(b.end, B->docStart + B->cutAt);
            v << b;
        }
    return v;
}

QVector<StoryLayout::NotePlace> StoryLayout::notePlaces() const
{
    QVector<NotePlace> v;
    for (const Note &n : m_notes) {
        if (n.endnote) continue;
        if (n.frame >= 0) v << NotePlace{n.storyId, n.number, n.frame, n.column, n.rect, 0, n.refPos};
        for (const Note::Piece &pc : n.more)
            if (pc.frame >= 0) v << NotePlace{n.storyId, n.number, pc.frame, pc.column, pc.rect, pc.from, n.refPos};
    }
    return v;
}

QVector<StoryLayout::CarriedNote> StoryLayout::carriedNotes(int frame) const
{
    QVector<CarriedNote> v;
    for (const Note &n : m_notes) {
        if (n.endnote) continue;
        if (n.goesOn) {   // (still going on, or not begun, when the frames ran out: from where its rest starts)
            v << CarriedNote{n.storyId, n.number, std::max(0, n.from), n.refPos};
            continue;
        }
        if (n.frame >= frame && n.frame != -2) {   // (one of a line's notes that had not begun when its first was cut: it begins in the frame after)
            v << CarriedNote{n.storyId, n.number, 0, n.refPos};
            continue;
        }
        for (const Note::Piece &pc : n.more)
            if (pc.frame >= frame && (n.frame < frame)) {
                v << CarriedNote{n.storyId, n.number, pc.from, n.refPos};
                break;
            }
    }
    return v;
}

double StoryLayout::usedHeight(int frame) const { return frame >= 0 && frame < m_used.size() ? m_used[frame] : 0; }

double StoryLayout::frameY(int frame) const
{
    if (m_frameY.isEmpty()) return 0;
    return m_frameY[frame < 0 ? m_frameY.size() - 1 : std::min<int>(frame, m_frameY.size() - 1)];
}

int StoryLayout::firstPosition(int frame) const
{
    for (const auto &B : m_blocks)
        for (int i = 0; i < B->lines.size(); ++i)
            if (B->lines[i].frame == frame) return B->docStart + B->docFromDisp(B->tl->lineAt(i).textStart());
    return -1;
}

int StoryLayout::lastPosition(int frame) const
{
    int out = -1;
    for (const auto &B : m_blocks)
        for (int i = 0; i < B->lines.size(); ++i)
            if (B->lines[i].frame == frame) {
                const PtLine l(B->tl->lineAt(i));
                out = B->docStart + B->docFromDisp(l.textStart() + l.textLength());
            }
    return out;
}

const StoryLayout::Block *StoryLayout::blockAt(int pos, int *rel) const
{
    for (const auto &B : m_blocks)
        if (pos >= B->docStart && pos <= B->docStart + B->docLen) {
            if (rel) *rel = pos - B->docStart;
            return B.get();
        }
    if (!m_blocks.empty()) {
        const auto &B = m_blocks.back();
        if (rel) *rel = B->docLen;
        return B.get();
    }
    return nullptr;
}

int StoryLayout::frameOf(int pos) const
{
    int f = -1;
    QRectF r;
    caretRect(pos, &f, &r);
    return f;
}

bool StoryLayout::caretRect(int pos, int *frame, QRectF *rect) const
{
    int rel = 0;
    const Block *B = blockAt(pos, &rel);
    if (!B || B->lines.isEmpty()) return false;
    const int d = B->dispFromDoc(rel);
    int li = 0;
    for (int i = 0; i < B->lines.size(); ++i) {
        const PtLine l(B->tl->lineAt(i));
        if (d >= l.textStart() && (d < l.textStart() + l.textLength() || i == B->lines.size() - 1)) { li = i; break; }
        if (d >= l.textStart()) li = i;
    }
    const PtLine l(B->tl->lineAt(li));
    const Line &info = B->lines[li];
    if (frame) *frame = info.frame;
    const double x = l.cursorToX(d);
    if (rect) *rect = QRectF(x, info.rect.top(), 0, info.rect.height());
    return info.frame >= 0;
}

int StoryLayout::hitTest(int frame, const QPointF &pt) const
{
    const Block *bestB = nullptr;
    int bestLine = -1;
    double bestScore = 1e18;
    for (const auto &B : m_blocks)
        for (int i = 0; i < B->lines.size(); ++i) {
            const Line &L = B->lines[i];
            if (L.frame != frame) continue;
            double dy = 0;
            if (pt.y() < L.rect.top()) dy = L.rect.top() - pt.y();
            else if (pt.y() > L.rect.bottom()) dy = pt.y() - L.rect.bottom();
            double dx = 0;
            if (pt.x() < L.rect.left()) dx = L.rect.left() - pt.x();
            else if (pt.x() > L.rect.right()) dx = pt.x() - L.rect.right();
            const double score = dy * 1000 + dx;
            if (score < bestScore) { bestScore = score; bestB = B.get(); bestLine = i; }
        }
    if (!bestB) return -1;
    const PtLine l(bestB->tl->lineAt(bestLine));
    int d = l.xToCursor(pt.x(), QTextLine::CursorBetweenCharacters);
    // Stay before a trailing line break or space-wrapped end.
    if (bestLine < bestB->lines.size() - 1 && d >= l.textStart() + l.textLength()) d = l.textStart() + l.textLength() - 1;
    return bestB->docStart + bestB->docFromDisp(d);
}

int StoryLayout::moveVertical(int pos, int dir, double x) const
{
    QVector<QPair<const Block *, int>> flat;
    int cur = -1;
    int rel = 0;
    const Block *cb = blockAt(pos, &rel);
    const int d = cb ? cb->dispFromDoc(rel) : 0;
    for (const auto &B : m_blocks)
        for (int i = 0; i < B->lines.size(); ++i) {
            if (B->lines[i].frame < 0) continue;
            if (B.get() == cb) {
                const PtLine l(B->tl->lineAt(i));
                if (d >= l.textStart() && (d < l.textStart() + l.textLength() || i == B->lines.size() - 1)) cur = flat.size();
            }
            flat << qMakePair(B.get(), i);
        }
    if (cur < 0) return pos;
    const int t = cur + dir;
    if (t < 0) return 0;
    if (t >= flat.size()) {
        const Block *B = m_blocks.back().get();
        return B->docStart + B->docLen;
    }
    const Block *B = flat[t].first;
    const PtLine l(B->tl->lineAt(flat[t].second));
    int nd = l.xToCursor(x, QTextLine::CursorBetweenCharacters);
    if (flat[t].second < B->lines.size() - 1 && nd >= l.textStart() + l.textLength()) nd = l.textStart() + l.textLength() - 1;
    return B->docStart + B->docFromDisp(nd);
}

int StoryLayout::lineStart(int pos) const
{
    int rel = 0;
    const Block *B = blockAt(pos, &rel);
    if (!B) return pos;
    const int d = B->dispFromDoc(rel);
    for (int i = B->lines.size() - 1; i >= 0; --i)
        if (B->tl->lineAt(i).textStart() <= d) return B->docStart + B->docFromDisp(B->tl->lineAt(i).textStart());
    return B->docStart;
}

int StoryLayout::lineEnd(int pos) const
{
    int rel = 0;
    const Block *B = blockAt(pos, &rel);
    if (!B) return pos;
    const int d = B->dispFromDoc(rel);
    for (int i = 0; i < B->lines.size(); ++i) {
        const PtLine l(B->tl->lineAt(i));
        const int end = l.textStart() + l.textLength();
        if (d < end || i == B->lines.size() - 1) {
            int e = end;
            if (i < B->lines.size() - 1 && e > l.textStart() && (B->disp[e - 1].isSpace())) --e;
            return B->docStart + B->docFromDisp(e);
        }
    }
    return B->docStart + B->docLen;
}

QVector<QRectF> StoryLayout::rangeRects(int frame, int from, int to) const
{
    QVector<QRectF> out;
    if (from > to) std::swap(from, to);
    for (const auto &B : m_blocks) {
        const int bs = B->docStart, be = B->docStart + B->docLen;
        if (to < bs || from > be) continue;
        const int df = B->dispFromDoc(std::max(from, bs) - bs);
        const int dt = B->dispFromDoc(std::min(to, be) - bs);
        for (int i = 0; i < B->lines.size(); ++i) {
            if (B->lines[i].frame != frame) continue;
            const PtLine l(B->tl->lineAt(i));
            const int ls = l.textStart(), le = l.textStart() + l.textLength();
            const int s = std::max(df, ls), e = std::min(dt, le);
            const bool paraEnd = (to > be) && i == B->lines.size() - 1;
            if (s > e || (s == e && !paraEnd)) continue;
            double x1 = l.cursorToX(s), x2 = l.cursorToX(e);
            if (x1 > x2) std::swap(x1, x2);
            if (paraEnd) x2 += 4;   // show the paragraph mark as selected
            out << QRectF(x1, B->lines[i].rect.top(), std::max(1.0, x2 - x1), B->lines[i].rect.height());
        }
    }
    return out;
}

namespace {

// The letters of one run on one line, as an effect needs them.
struct FxPart {
    QList<QGlyphRun> runs;
    QTransform draw;   // the run's own taller, shorter, or slanted drawing, in frame units
    QColor color;
};

// A stretch of one line that carries an effect list: runs side by side with the same look are one.
struct FxRange {
    const void *block;
    int line, from, to;
    double ys, shear;
    QColor color;
};

// Every run that carries the same effect list.
struct FxGroup {
    QString json;
    QVector<FxPart> parts;
    QVector<FxRange> ranges;
    QRectF box;        // its letters and the lines they stand on, frame-local
};

// A stranger's file can give every run an effect list of its own, each a picture to draw: no more than this many different lists on a
// frame (and this many, 128, looked at). A slide has one to three lists (the most of any in the sample decks is two); this is over ten times that.
constexpr int kMaxEffectLists = 32, kMaxEffectListsSeen = 128;
// The pictures of one painting's effects together (every frame, both passes): four times the device's pixels, but at least 16 million and
// at most 64 million (a second or less at 50 nanoseconds a pixel, as measured). The words of a slide with a shadow on all of them make one
// to two million pixels on a screen and about ten at 4K. A device that is not pixels (a PDF) has 48 million.
constexpr qint64 kEffectPixelsLeast = 16000000, kEffectPixelsMost = 64000000, kEffectPixelsVector = 48000000;
// Cached pictures of effects, in bytes, for every layout together.
constexpr qint64 kEffectCacheBytes = 192LL * 1024 * 1024;

thread_local qint64 *t_effectPixels = nullptr;
std::atomic<int> s_effectPictures{0};

qint64 effectPixelsFor(const QPainter *p)
{
    const bool pixels = p->paintEngine() && p->paintEngine()->type() == QPaintEngine::Raster;
    if (!pixels || !p->device()) return kEffectPixelsVector;
    return std::clamp<qint64>(4 * qint64(p->device()->width()) * p->device()->height(), kEffectPixelsLeast, kEffectPixelsMost);
}

// A picture of an effect, kept for the next time the same layout is painted at the same size.
struct FxPicture {
    QImage image;
    QRectF where;   // frame-local
};

QMutex &effectCacheLock()
{
    static QMutex lock;
    return lock;
}

QCache<QString, FxPicture> &effectCache()
{
    static QCache<QString, FxPicture> cache(static_cast<int>(kEffectCacheBytes));
    return cache;
}

// Pixels to a unit of the page that effects are drawn at: the device's, up to what a PDF's picture needs (300 dpi, a point being the unit).
double effectResolution(const QPainter *p)
{
    constexpr double kPdf = 300.0 / 72;
    const QTransform t = p->deviceTransform();
    const double ds = std::sqrt(std::abs(t.determinant()));
    const bool pixels = p->paintEngine() && p->paintEngine()->type() == QPaintEngine::Raster;
    if (!pixels) return kPdf;
    return std::clamp(ds, 0.25, 6.0);
}

// The letters in an image covering `area`, `s` pixels to the unit: black (what an effect is cast from) or in their own colors.
QImage drawLetters(const QVector<FxPart> &parts, const QPointF &off, const QRectF &area, double s, bool colored)
{
    const QSize px(std::max(1, int(std::ceil(area.width() * s))), std::max(1, int(std::ceil(area.height() * s))));
    QImage img(px, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter q(&img);
    q.setRenderHint(QPainter::Antialiasing);
    q.setRenderHint(QPainter::TextAntialiasing);
    q.scale(s, s);
    q.translate(-area.topLeft());
    for (const FxPart &part : parts) {
        q.save();
        q.setTransform(part.draw, true);
        q.setPen(colored ? part.color : QColor(Qt::black));
        drawFine(&q, off, part.runs);
        q.restore();
    }
    q.end();
    return img;
}

// An alpha image in a color (its own alpha counting too), ready to draw.
QImage tinted(const QImage &alpha8, const QColor &c)
{
    QImage out(alpha8.size(), QImage::Format_ARGB32_Premultiplied);
    const int ca = c.alpha();
    for (int y = 0; y < alpha8.height(); ++y) {
        const uchar *in = alpha8.constScanLine(y);
        QRgb *o = reinterpret_cast<QRgb *>(out.scanLine(y));
        for (int x = 0; x < alpha8.width(); ++x) {
            const int a = (in[x] * ca + 127) / 255;
            o[x] = qRgba((c.red() * a + 127) / 255, (c.green() * a + 127) / 255, (c.blue() * a + 127) / 255, a);
        }
    }
    return out;
}

// The point of a box a letter says (algn: tl, t, tr, l, ctr, r, bl, b, br).
QPointF anchorOf(const QRectF &r, const QString &algn)
{
    const double x = algn.endsWith(QLatin1Char('l')) ? r.left() : algn.endsWith(QLatin1Char('r')) ? r.right() : r.center().x();
    const double y = algn.startsWith(QLatin1Char('t')) ? r.top() : algn.startsWith(QLatin1Char('b')) ? r.bottom() : r.center().y();
    return QPointF(x, y);
}

// Where a shadow or reflection lands: the letters scaled and skewed about their anchor, then moved `distance` along `angle`.
QTransform castTransform(double sx, double sy, double kx, double ky, double distance, double angle, const QRectF &box, const QString &algn)
{
    const double a = qDegreesToRadians(angle);
    const QPointF at = anchorOf(box, algn);
    QTransform t;
    t.translate(distance * std::cos(a), distance * std::sin(a));
    t.translate(at.x(), at.y());
    if (kx != 0 || ky != 0) t.shear(std::tan(qDegreesToRadians(kx)), std::tan(qDegreesToRadians(ky)));
    if (sx != 1 || sy != 1) t.scale(sx, sy);
    t.translate(-at.x(), -at.y());
    return t;
}

// A premultiplied picture blurred as the Gaussian of the given spread (pixels): each of its four channels is blurred alike.
void blurPicture(QImage &img, double sigma)
{
    if (!(sigma >= 0.3)) return;
    img = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage channels[4];
    for (int c = 0; c < 4; ++c) {
        channels[c] = QImage(img.size(), QImage::Format_Alpha8);
        for (int y = 0; y < img.height(); ++y) {
            const QRgb *in = reinterpret_cast<const QRgb *>(img.constScanLine(y));
            uchar *o = channels[c].scanLine(y);
            for (int x = 0; x < img.width(); ++x) o[x] = uchar(c == 0 ? qAlpha(in[x]) : c == 1 ? qRed(in[x]) : c == 2 ? qGreen(in[x]) : qBlue(in[x]));
        }
        blurAlpha(channels[c], sigma);
    }
    for (int y = 0; y < img.height(); ++y) {
        QRgb *o = reinterpret_cast<QRgb *>(img.scanLine(y));
        const uchar *a = channels[0].constScanLine(y), *r = channels[1].constScanLine(y), *g = channels[2].constScanLine(y), *b = channels[3].constScanLine(y);
        // (a premultiplied channel never goes over the alpha)
        for (int x = 0; x < img.width(); ++x) o[x] = qRgba(std::min(r[x], a[x]), std::min(g[x], a[x]), std::min(b[x], a[x]), a[x]);
    }
}

QImage alphaOf(const QImage &img)
{
    return img.convertToFormat(QImage::Format_Alpha8);
}

} // namespace

EffectBudget::EffectBudget(const QPainter *p, qint64 pixels)
{
    if (t_effectPixels) return;   // inside another painting's: it is theirs
    m_left = pixels >= 0 ? pixels : effectPixelsFor(p);
    t_effectPixels = &m_left;
    m_owner = true;
}

EffectBudget::~EffectBudget()
{
    if (m_owner) t_effectPixels = nullptr;
}

int effectPicturesBuiltForTest() { return s_effectPictures; }

void StoryLayout::paintTextEffects(QPainter *p, int frame, bool over) const
{
    bool any = false;
    for (const auto &B : m_blocks) any |= B->textFx;
    if (!any) return;
    const QPointF off(0, -frameY(frame));
    // The letters of each effect list (a run's, the same string for every run that has it).
    QVector<FxGroup> groups;
    QHash<QString, int> seen;   // each list met: its group, or -1
    for (const auto &B : m_blocks) {
        if (!B->textFx) continue;
        for (const auto &er : B->effects) {
            const QString json = er.format.stringProperty(tp::TextFx);
            if (json.isEmpty()) continue;
            // Each different list is looked at once (to see whether it has an effect for this pass); only so many are.
            int gi = -1;
            const auto known = seen.constFind(json);
            if (known != seen.constEnd()) {
                gi = *known;
            } else {
                if (seen.size() >= kMaxEffectListsSeen) continue;
                const TextEffects &fx = TextEffects::cached(json);
                if ((over ? fx.inner.on : (fx.outer.on || fx.glow.on || fx.reflection.on)) && groups.size() < kMaxEffectLists) {
                    gi = int(groups.size());
                    groups.push_back(FxGroup{json, {}, {}, QRectF()});
                }
                seen.insert(json, gi);
            }
            if (gi < 0) continue;
            const double ys = er.format.hasProperty(tp::GlyphScaleY) ? er.format.property(tp::GlyphScaleY).toDouble() : 1;
            const double shear = er.format.hasProperty(tp::SyntheticItalic) ? er.format.property(tp::SyntheticItalic).toDouble() : 0;
            const QColor color = er.format.foreground().style() != Qt::NoBrush ? er.format.foreground().color() : QColor(Qt::black);
            for (int i = 0; i < B->lines.size(); ++i) {
                if (B->lines[i].frame != frame) continue;
                const PtLine l(B->tl->lineAt(i));
                const int from = std::max(er.start, l.textStart()), to = std::min(er.start + er.length, l.textStart() + l.textLength());
                if (from >= to) continue;
                FxGroup &g = groups[gi];
                if (!g.ranges.isEmpty()) {
                    FxRange &last = g.ranges.last();
                    if (last.block == B.get() && last.line == i && last.to == from && last.ys == ys && last.shear == shear && last.color == color) {
                        last.to = to;
                        continue;
                    }
                }
                g.ranges.push_back(FxRange{B.get(), i, from, to, ys, shear, color});
            }
        }
    }
    // The letters of each stretch, and the box they and their lines fill.
    for (FxGroup &g : groups) {
        for (const FxRange &r : g.ranges) {
            const Block *B = static_cast<const Block *>(r.block);
            const PtLine l(B->tl->lineAt(r.line));
            FxPart part;
            part.runs = l.glyphRuns(r.from, r.to - r.from);
            const double base = l.y() + l.ascent() + off.y();
            if (r.ys != 1 || r.shear != 0) {
                part.draw.translate(0, base);
                part.draw.shear(-r.shear, 0);
                part.draw.scale(1, r.ys);
                part.draw.translate(0, -base);
            }
            part.color = r.color;
            QRectF box;
            for (const QGlyphRun &gr : part.runs) box |= gr.boundingRect();
            box = part.draw.mapRect(QRectF(box.topLeft() / kFine, box.bottomRight() / kFine).translated(off));
            // The line stands from its top to the foot of its text, whatever the letters reach.
            const QRectF line = B->lines[r.line].rect;
            box |= QRectF(box.left(), line.top(), box.width(), std::max(0.0, line.height() - B->lines[r.line].below));
            g.box = g.box.isNull() ? box : g.box | box;
            g.parts << part;
        }
    }
    const double scale = m_env.fontScale;   // the effects of text the presentation program shrank are shrunk with it
    const double res = effectResolution(p);
    // The painting's share of pixels (all its frames, both passes); alone, this paint has the whole.
    qint64 own = effectPixelsFor(p);
    qint64 &pixels = t_effectPixels ? *t_effectPixels : own;
    // Whether a picture of this size is still in the share (and takes it); `more` is work on pictures of other sizes that goes with it.
    auto afford = [&](const QRectF &area, double s, qint64 more = 0) {
        const qint64 n = qint64(std::ceil(area.width() * s)) * qint64(std::ceil(area.height() * s)) + more;
        if (n < 0 || n > pixels) return false;
        pixels -= n;
        return true;
    };
    auto sizeFor = [&](const QRectF &area) { return std::min(res, 4096.0 / std::max(1.0, std::max(area.width(), area.height()))); };
    const int resKey = int(std::lround(res * 64));
    // A group's picture of one of its effects: the kept one when this layout was painted at this size before, else a new one (when the
    // painting can still afford it), which is kept. A caret that blinks, a window that scrolls, and a slide's next animation step paint
    // the same layout again.
    auto picture = [&](int group, int kind, const std::function<bool(FxPicture *)> &make, FxPicture *out) {
        const QString key = QStringLiteral("%1/%2/%3/%4/%5/%6").arg(m_serial).arg(frame).arg(over ? 1 : 0).arg(group).arg(kind).arg(resKey);
        {
            QMutexLocker locked(&effectCacheLock());
            if (const FxPicture *kept = effectCache().object(key)) {
                *out = *kept;
                return true;
            }
        }
        if (!make(out)) return false;
        ++s_effectPictures;
        QMutexLocker locked(&effectCacheLock());
        effectCache().insert(key, new FxPicture(*out), int(std::min<qint64>(out->image.sizeInBytes(), 1LL << 30)));
        return true;
    };
    auto drawPicture = [&](const FxPicture &pic, const QTransform &t) {
        p->save();
        p->setTransform(t, true);
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->drawImage(pic.where, pic.image);
        p->restore();
    };
    for (int gi = 0; gi < groups.size(); ++gi) {
        const FxGroup &g = groups[gi];
        if (g.parts.isEmpty() || g.box.isNull()) continue;
        const TextEffects &fx = TextEffects::cached(g.json);
        if (!over) {
            if (fx.reflection.on) {
                const TextReflection &r = fx.reflection;
                const double pad = 3 * blurSigma(r.blur * scale) + 1;
                const QTransform t = castTransform(r.sx, r.sy, r.kx, r.ky, r.distance * scale, r.angle, g.box, r.align);
                FxPicture pic;
                const bool made = picture(gi, 0, [&](FxPicture *out) {
                    const QRectF area = t.mapRect(g.box.adjusted(-pad, -pad, pad, pad));
                    const double s = sizeFor(area);
                    // (a blurred one is four pictures' work: each of its channels is blurred)
                    if (!afford(area, s, r.blur > 0 ? qint64(std::ceil(area.width() * s)) * qint64(std::ceil(area.height() * s)) * 3 : 0)) return false;
                    // The letters as they look, turned over, then faded.
                    QImage img(QSize(std::max(1, int(std::ceil(area.width() * s))), std::max(1, int(std::ceil(area.height() * s)))), QImage::Format_ARGB32_Premultiplied);
                    img.fill(Qt::transparent);
                    {
                        QPainter q(&img);
                        q.setRenderHint(QPainter::Antialiasing);
                        q.setRenderHint(QPainter::TextAntialiasing);
                        q.scale(s, s);
                        q.translate(-area.topLeft());
                        q.setTransform(t, true);
                        for (const FxPart &part : g.parts) {
                            q.save();
                            q.setTransform(part.draw, true);
                            q.setPen(part.color);
                            drawFine(&q, off, part.runs);
                            q.restore();
                        }
                        q.end();
                    }
                    if (r.blur > 0) blurPicture(img, blurSigma(r.blur * scale) * s);
                    // The fade runs along fadeAngle across the reflection.
                    const QRectF shown(area.topLeft(), QSizeF(img.width() / s, img.height() / s));
                    const double fa = qDegreesToRadians(r.fadeAngle);
                    const QPointF dir(std::cos(fa), std::sin(fa));
                    double lo = 1e300, hi = -1e300;
                    for (const QPointF &c : {shown.topLeft(), shown.topRight(), shown.bottomLeft(), shown.bottomRight()}) {
                        const double d = c.x() * dir.x() + c.y() * dir.y();
                        lo = std::min(lo, d);
                        hi = std::max(hi, d);
                    }
                    if (hi - lo > 1e-6) {
                        const QPointF c = shown.center();
                        const double mid = c.x() * dir.x() + c.y() * dir.y();
                        const QPointF from = c + dir * (lo - mid), to = c + dir * (hi - mid);
                        QLinearGradient grad((from - shown.topLeft()) * s, (to - shown.topLeft()) * s);
                        const double sp = std::clamp(r.startPos, 0.0, 1.0), ep = std::clamp(std::max(r.endPos, r.startPos + 1e-4), 0.0, 1.0);
                        grad.setColorAt(0, QColor(0, 0, 0, int(std::lround(r.startAlpha * 255))));
                        grad.setColorAt(sp, QColor(0, 0, 0, int(std::lround(r.startAlpha * 255))));
                        grad.setColorAt(ep, QColor(0, 0, 0, int(std::lround(r.endAlpha * 255))));
                        grad.setColorAt(1, QColor(0, 0, 0, int(std::lround(r.endAlpha * 255))));
                        QPainter q(&img);
                        q.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                        q.fillRect(img.rect(), grad);
                        q.end();
                    }
                    out->image = img;
                    out->where = shown;
                    return true;
                }, &pic);
                if (made) drawPicture(pic, QTransform());
            }
            if (fx.outer.on) {
                const TextShadow &sh = fx.outer;
                const QTransform t = castTransform(sh.sx, sh.sy, sh.kx, sh.ky, sh.distance * scale * kShadowNear, sh.angle, g.box, sh.align);
                if (sh.blur <= 0) {
                    p->save();
                    p->setTransform(t, true);
                    p->setPen(sh.color);
                    for (const FxPart &part : g.parts) {
                        p->save();
                        p->setTransform(part.draw, true);
                        drawFine(p, off, part.runs);
                        p->restore();
                    }
                    p->restore();
                } else {
                    const double sigma = blurSigma(sh.blur * scale) * kShadowSoft;
                    FxPicture pic;
                    const bool made = picture(gi, 1, [&](FxPicture *out) {
                        const QRectF area = g.box.adjusted(-3 * sigma - 1, -3 * sigma - 1, 3 * sigma + 1, 3 * sigma + 1);
                        const double s = sizeFor(area);
                        if (!afford(area, s)) return false;
                        QImage letters = alphaOf(drawLetters(g.parts, off, area, s, false));
                        blurAlpha(letters, sigma * s);
                        out->image = tinted(letters, sh.color);
                        out->where = QRectF(area.topLeft(), QSizeF(letters.width() / s, letters.height() / s));
                        return true;
                    }, &pic);
                    if (made) drawPicture(pic, t);
                }
            }
            if (fx.glow.on && fx.glow.radius > 0) {
                // The glow reaches as far as its radius: the letters' picture grown by half of it, and blurred by a quarter. (Of the picture
                // as a whole, so the work doesn't grow with the number of letters.)
                const double reach = fx.glow.radius * scale;
                const double grow = reach / 2, sigma = reach / 4;
                FxPicture pic;
                const bool made = picture(gi, 2, [&](FxPicture *out) {
                    const QRectF area = g.box.adjusted(-grow - 3 * sigma - 1, -grow - 3 * sigma - 1, grow + 3 * sigma + 1, grow + 3 * sigma + 1);
                    const double s = sizeFor(area);
                    if (!afford(area, s)) return false;
                    QImage letters = alphaOf(drawLetters(g.parts, off, area, s, false));
                    dilateAlpha(letters, int(std::lround(grow * s)));
                    blurAlpha(letters, sigma * s);
                    out->image = tinted(letters, fx.glow.color);
                    out->where = QRectF(area.topLeft(), QSizeF(letters.width() / s, letters.height() / s));
                    return true;
                }, &pic);
                if (made) drawPicture(pic, QTransform());
            }
        } else if (fx.inner.on) {
            // The inside of the letters, darkened from the side the shadow comes from: the outside, moved in and blurred.
            const TextShadow &sh = fx.inner;
            const double sigma = blurSigma(sh.blur * scale);
            const double a = qDegreesToRadians(sh.angle);
            const QPointF d(sh.distance * scale * std::cos(a), sh.distance * scale * std::sin(a));
            FxPicture pic;
            const bool made = picture(gi, 3, [&](FxPicture *out) {
                const double pad = 3 * sigma + 1 + std::max(std::abs(d.x()), std::abs(d.y()));
                const QRectF area = g.box.adjusted(-pad, -pad, pad, pad);
                const double s = sizeFor(area);
                // (the picture, and the wider one it is blurred in)
                const int m = int(std::ceil(3 * sigma * s)) + 1;
                const qint64 wide = (qint64(std::ceil(area.width() * s)) + 2 * m) * (qint64(std::ceil(area.height() * s)) + 2 * m);
                if (!afford(area, s, wide)) return false;
                const QImage letters = alphaOf(drawLetters(g.parts, off, area, s, false));
                QImage outside(letters.size(), QImage::Format_Alpha8);
                for (int y = 0; y < letters.height(); ++y) {
                    const uchar *in = letters.constScanLine(y);
                    uchar *o = outside.scanLine(y);
                    for (int x = 0; x < letters.width(); ++x) o[x] = uchar(255 - in[x]);
                }
                QImage cover(letters.size(), QImage::Format_ARGB32_Premultiplied);
                cover.fill(Qt::transparent);
                // Blurred inside a frame of solid pixels (the outside goes on past the picture), so the edge doesn't pull the shadow down.
                QImage frameOfOutside(letters.width() + 2 * m, letters.height() + 2 * m, QImage::Format_Alpha8);
                frameOfOutside.fill(255);
                for (int y = 0; y < outside.height(); ++y) std::memcpy(frameOfOutside.scanLine(y + m) + m, outside.constScanLine(y), size_t(outside.width()));
                blurAlpha(frameOfOutside, sigma * s);
                const QImage blurred = frameOfOutside.copy(m, m, outside.width(), outside.height());
                QPainter q(&cover);
                q.setRenderHint(QPainter::SmoothPixmapTransform);
                q.drawImage(QPointF(d.x() * s, d.y() * s), tinted(blurred, sh.color));
                q.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                q.drawImage(0, 0, letters);
                q.end();
                out->image = cover;
                out->where = QRectF(area.topLeft(), QSizeF(cover.width() / s, cover.height() / s));
                return true;
            }, &pic);
            if (made) drawPicture(pic, QTransform());
        }
    }
}

// A paragraph's borders and shading as a word processor draws them, on the part of it that is in `frame`: the box reaches from a distance outside the
// text's indents (each side's width and space), the top line is above the first line and the bottom below the last (an open end where the box goes
// on to or from another page, or on from another paragraph with the same borders, which has the line between them instead), and the fill is
// inside all of it.
void StoryLayout::paintBox(QPainter *p, int bi, int frame) const
{
    const Block &B = *m_blocks[bi];
    const tp::ParaBox &box = B.box;
    int first = -1, last = -1;
    for (int i = 0; i < B.lines.size(); ++i)
        if (B.lines[i].frame == frame) {
            if (first < 0) first = i;
            last = i;
        }
    if (first < 0) return;
    const bool startsHere = first == 0, endsHere = last == B.lines.size() - 1 && !B.cut;
    double yTop = B.lines[first].rect.top() - (startsHere ? B.boxTopRoom : 0), yBottom = B.lines[last].rect.bottom() + (endsHere ? B.boxBottomRoom : 0);
    // A box that goes on into the next paragraph reaches to where that one's first line (and its line between) begins, so that the sides and the fill are whole.
    if (endsHere && B.boxJoinedAfter && bi + 1 < m_blocks.size()) {
        const Block &N = *m_blocks[bi + 1];
        if (!N.lines.isEmpty() && N.lines.first().frame == frame) yBottom = std::max(yBottom, N.lines.first().rect.top() - N.boxTopRoom);
    }
    const double x0 = B.boxLeft - box.left.reserve(), x1 = B.boxRight + box.right.reserve();
    if (box.fill.isValid()) p->fillRect(QRectF(x0, yTop, x1 - x0, yBottom - yTop), box.fill);
    auto line = [&](const tp::BoxSide &s, const QRectF &r, bool horizontal) {
        if (!s.visible()) return;
        const QColor color = s.color.isValid() ? s.color : QColor(Qt::black);
        const double w = s.eighths / 8.0;
        if (s.style == QLatin1String("double")) {
            // (two lines of the width, with a line's width between them)
            for (int k : {0, 2}) {
                const QRectF part = horizontal ? QRectF(r.left(), r.top() + k * w, r.width(), w) : QRectF(r.left() + k * w, r.top(), w, r.height());
                p->fillRect(part, color);
            }
            return;
        }
        const bool dashed = s.style.startsWith(QLatin1String("dash")) || s.style == QLatin1String("dotted") || s.style.startsWith(QLatin1String("dot"));
        if (!dashed) {
            p->fillRect(r, color);
            return;
        }
        QPen pen(color, w, s.style == QLatin1String("dotted") ? Qt::DotLine : Qt::DashLine);
        pen.setCapStyle(Qt::FlatCap);
        p->save();
        p->setPen(pen);
        if (horizontal) p->drawLine(QPointF(r.left(), r.center().y()), QPointF(r.right(), r.center().y()));
        else p->drawLine(QPointF(r.center().x(), r.top()), QPointF(r.center().x(), r.bottom()));
        p->restore();
    };
    if (startsHere && !B.boxTopOff && !B.boxJoinedBefore) line(box.top, QRectF(x0, yTop, x1 - x0, box.top.width()), true);
    if (startsHere && B.boxJoinedBefore) line(box.between, QRectF(x0, yTop, x1 - x0, box.between.width()), true);
    if (endsHere && !B.boxJoinedAfter) line(box.bottom, QRectF(x0, yBottom - box.bottom.width(), x1 - x0, box.bottom.width()), true);
    line(box.left, QRectF(x0, yTop, box.left.width(), yBottom - yTop), false);
    line(box.right, QRectF(x1 - box.right.width(), yTop, box.right.width(), yBottom - yTop), false);
}

void StoryLayout::paint(QPainter *p, int frame, const PaintOptions &o) const
{
    if (frame < 0 || frame >= m_frames.size()) return;
    const FrameSpec &fs = m_frames[frame];
    const QPointF off(0, -frameY(frame));
    // In the paragraph layouts' own units, where this frame's lines sit
    // below the frames before it (frameY), not from the frame's top.
    const QRectF clip(-1e5, frameY(frame) - 1e3, 2e5, fs.size.height() + 2e3);
    p->save();
    paintTextEffects(p, frame, false);
    for (int bi = 0; bi < m_blocks.size(); ++bi) {
        const auto &B = m_blocks[bi];
        bool any = false;
        for (const Line &l : B->lines) any |= (l.frame == frame);
        if (!any) continue;
        if (m_env.wordLines && B->box.any()) paintBox(p, bi, frame);

        if (o.shadeFields)
            for (const auto &fr : B->fieldRanges)
                for (int i = 0; i < B->lines.size(); ++i) {
                    if (B->lines[i].frame != frame) continue;
                    const PtLine l(B->tl->lineAt(i));
                    const int s = std::max(fr.first, l.textStart()), e = std::min(fr.first + fr.second, l.textStart() + l.textLength());
                    if (s >= e) continue;
                    const double x1 = l.cursorToX(s), x2 = l.cursorToX(e);
                    p->fillRect(QRectF(std::min(x1, x2), B->lines[i].rect.top(), std::abs(x2 - x1), B->lines[i].rect.height()), QColor(0, 0, 0, 28));
                }

        // Text effects drawn beneath the text from the same glyph runs.
        for (const auto &er : B->effects) {
            const double sz = er.format.fontPointSize() / fontPointFactor();
            const bool shadow = er.format.boolProperty(tp::Shadow), emboss = er.format.boolProperty(tp::Emboss),
                       engrave = er.format.boolProperty(tp::Engrave);
            const QString glow = er.format.stringProperty(tp::GlowRef);
            for (int i = 0; i < B->lines.size(); ++i) {
                if (B->lines[i].frame != frame) continue;
                const PtLine l(B->tl->lineAt(i));
                const int s = std::max(er.start, l.textStart()), e = std::min(er.start + er.length, l.textStart() + l.textLength());
                if (s >= e) continue;
                const auto runs = l.glyphRuns(s, e - s);
                const double ys = er.format.hasProperty(tp::GlyphScaleY) ? er.format.property(tp::GlyphScaleY).toDouble() : 1;
                const double shear = er.format.hasProperty(tp::SyntheticItalic) ? er.format.property(tp::SyntheticItalic).toDouble() : 0;
                const double base = l.y() + l.ascent();
                auto drawRuns = [&](const QPointF &d, const QColor &c) {
                    p->setPen(c);
                    p->save();
                    if (ys != 1 || shear != 0) {
                        p->translate(0, off.y() + base);
                        p->shear(-shear, 0);
                        p->scale(1, ys);
                        p->translate(0, -off.y() - base);
                    }
                    drawFine(p, off + d, runs);
                    p->restore();
                };
                const double k = std::max(0.6, sz / 18.0);
                if (!glow.isEmpty()) {
                    QColor gc = ColorRef::fromString(glow).resolve(m_env.colors);
                    gc.setAlphaF(0.35f);
                    for (int a = 0; a < 8; ++a) {
                        const double ang = a * M_PI / 4;
                        drawRuns(QPointF(std::cos(ang), std::sin(ang)) * 1.6 * k, gc);
                    }
                }
                if (shadow && er.format.stringProperty(tp::TextFx).isEmpty()) drawRuns(QPointF(1.2, 1.2) * k, QColor(0, 0, 0, 110));
                if (emboss) { drawRuns(QPointF(-0.7, -0.7) * k, QColor(255, 255, 255, 220)); drawRuns(QPointF(0.7, 0.7) * k, QColor(0, 0, 0, 140)); }
                if (engrave) { drawRuns(QPointF(-0.7, -0.7) * k, QColor(0, 0, 0, 140)); drawRuns(QPointF(0.7, 0.7) * k, QColor(255, 255, 255, 220)); }
            }
        }

        p->save();
        p->translate(off);
        p->scale(1 / kFine, 1 / kFine);
        if (!B->directGlyphs) {
            B->tl->draw(p, QPointF(0, 0), {}, QRectF(clip.topLeft() * kFine, clip.size() * kFine));
            const QList<QTextLayout::FormatRange> fmts = B->tl->formats();
            for (const auto &r : fmts) {
                if (!r.format.hasProperty(tp::HandBold)) continue;
                for (int i = 0; i < B->lines.size(); ++i) {
                    if (B->lines[i].frame != frame) continue;
                    const QTextLine line = B->tl->lineAt(i);
                    const int s = std::max(r.start, line.textStart()), e = std::min(r.start + r.length, line.textStart() + line.textLength());
                    if (s < e) drawHandBold(p, line, s, e, r.format, o.output);
                }
            }
            for (int n = 0; n < fmts.size(); ++n) {
                const auto &r = fmts[n];
                if (!r.format.hasProperty(kOwnUnderline)) continue;
                const QBrush br = r.format.hasProperty(QTextFormat::ForegroundBrush) ? r.format.foreground() : QBrush(Qt::black);
                for (int i = 0; i < B->lines.size(); ++i) {
                    if (B->lines[i].frame != frame) continue;
                    const QTextLine line = B->tl->lineAt(i);
                    const auto [s, e] = underlineSpan(fmts, n, line.textStart(), line.textStart() + line.textLength());
                    if (s < e) drawUnderline(p, line, s, e, r.format, br, B->disp);
                }
            }
        } else {
            // Run by run, each in its color; a run drawn taller or shorter, or
            // slanted, is scaled and sheared from its baseline (its widths stay as laid out).
            const QList<QTextLayout::FormatRange> fmts = B->tl->formats();
            for (int i = 0; i < B->lines.size(); ++i) {
                if (B->lines[i].frame != frame) continue;
                const QTextLine line = B->tl->lineAt(i);
                const int ls = line.textStart(), le = ls + line.textLength();
                const double base = line.y() + line.ascent();
                for (int n = 0; n < fmts.size(); ++n) {
                    const auto &r = fmts[n];
                    const int s = std::max(r.start, ls), e = std::min(r.start + r.length, le);
                    if (s >= e) continue;
                    const QBrush br = r.format.hasProperty(QTextFormat::ForegroundBrush) ? r.format.foreground() : QBrush(Qt::black);
                    const double ys = r.format.hasProperty(tp::GlyphScaleY) ? r.format.property(tp::GlyphScaleY).toDouble() : 1;
                    const double shear = r.format.hasProperty(tp::SyntheticItalic) ? r.format.property(tp::SyntheticItalic).toDouble() : 0;
                    p->save();
                    p->setPen(QPen(br, 0));
                    if (ys != 1 || shear != 0) {
                        p->translate(0, base);
                        p->shear(-shear, 0);
                        p->scale(1, ys);
                        p->translate(0, -base);
                    }
                    for (const QGlyphRun &g : line.glyphRuns(s, e - s)) {
                        p->drawGlyphRun(QPointF(0, 0), g);
                        // Outlined letters (an outline, or bold drawn by hand) take their stroke too.
                        if (r.format.hasProperty(QTextFormat::TextOutline)) {
                            const QRawFont raw = g.rawFont();
                            const QList<quint32> glyphs = g.glyphIndexes();
                            const QList<QPointF> where = g.positions();
                            p->setPen(r.format.textOutline());
                            p->setBrush(Qt::NoBrush);
                            for (int k = 0; k < glyphs.size() && k < where.size(); ++k) {
                                QPainterPath path = raw.pathForGlyph(glyphs[k]);
                                path.translate(where[k]);
                                p->drawPath(path);
                            }
                            p->setPen(QPen(br, 0));
                        }
                    }
                    drawHandBold(p, line, s, e, r.format, o.output);
                    p->restore();
                    // Lines under, through and over the run, unscaled, where
                    // Qt's own drawing puts them (glyph runs carry none).
                    const QTextCharFormat &cf = r.format;
                    const bool under = cf.underlineStyle() != QTextCharFormat::NoUnderline || cf.hasProperty(kOwnUnderline), through = cf.fontStrikeOut(), over = cf.fontOverline();
                    if (under || through || over) {
                        const QFontMetricsF fm(cf.font());
                        const double x1 = line.cursorToX(s), x2 = line.cursorToX(e);
                        QPen pen(br, std::max(1.0, fm.lineWidth()));
                        pen.setCapStyle(Qt::FlatCap);
                        p->save();
                        if (under)
                            if (const auto [us, ue] = underlineSpan(fmts, n, ls, le); us < ue) drawUnderline(p, line, us, ue, cf, br, B->disp);
                        p->setPen(pen);
                        if (through) p->drawLine(QLineF(x1, base - fm.strikeOutPos(), x2, base - fm.strikeOutPos()));
                        if (over) p->drawLine(QLineF(x1, base - fm.overlinePos(), x2, base - fm.overlinePos()));
                        p->restore();
                    }
                }
            }
        }
        p->restore();

        // Tab leaders: the stop each tab goes to is the first one past where
        // it starts; its leader fills the gap, on a grid so rows line up.
        if (!B->leaders.trimmed().isEmpty()) {
            const QList<QTextLayout::FormatRange> fmts = B->tl->formats();
            for (int i = 0; i < B->lines.size(); ++i) {
                if (B->lines[i].frame != frame) continue;
                const PtLine l(B->tl->lineAt(i));
                const QList<QTextOption::Tab> stops = B->tl->textOption().tabs();   // (the stops of this line's list)
                for (int k = l.textStart(); k < l.textStart() + l.textLength(); ++k) {
                    if (B->disp[k] != QLatin1Char('\t')) continue;
                    const double x0 = l.cursorToX(k), x1 = l.cursorToX(k + 1);
                    int idx = -1;
                    // (A word processor's lists are from each line's own start; the other lists are from the layout's.)
                    const double fromStart = m_env.wordLines ? x0 - l.position().x() : x0;
                    for (int s = 0; s < stops.size() && idx < 0; ++s)
                        if (stops[s].position / kFine > fromStart + 0.01) idx = s;
                    // (A word processor's stops carry their own leaders: the lines' lists differ.)
                    const QChar leader = m_env.wordLines && !stops.isEmpty() ? (idx >= 0 && stops[idx].type != QTextOption::DelimiterTab ? stops[idx].delimiter : QChar())
                                                                              : (idx >= 0 && idx < B->leaders.size() ? B->leaders[idx] : QChar());
                    if (idx < 0 || leader.isSpace() || leader.isNull()) continue;
                    QTextCharFormat cf;
                    for (const auto &r : fmts)
                        if (k >= r.start && k < r.start + r.length) cf = r.format;
                    const QFont f = pointFont(cf.hasProperty(QTextFormat::FontFamilies) ? cf.font() : B->tl->font());
                    const QString ch(leader);
                    const double w = advanceOf(f, ch);
                    if (w <= 0.01) continue;
                    const double gap = w * 0.4;
                    double x = std::ceil((x0 + gap) / w) * w;
                    QString run;
                    const double startX = x;
                    for (; x + w <= x1 - gap; x += w) run += ch;
                    if (run.isEmpty()) continue;
                    p->setPen(cf.foreground().style() != Qt::NoBrush ? cf.foreground().color() : m_env.colors.slot(Main));
                    drawPlainText(p, off + QPointF(startX, l.position().y() + l.ascent()), f, run);
                }
            }
        }

        // Misspelling squiggles.
        if (o.showSpelling)
            for (const auto &m : o.misspelled) {
                if (m.second < B->docStart || m.first > B->docStart + B->docLen) continue;
                const int ds = B->dispFromDoc(m.first - B->docStart), de = B->dispFromDoc(m.second - B->docStart);
                for (int i = 0; i < B->lines.size(); ++i) {
                    if (B->lines[i].frame != frame) continue;
                    const PtLine l(B->tl->lineAt(i));
                    const int s = std::max(ds, l.textStart()), e = std::min(de, l.textStart() + l.textLength());
                    if (s >= e) continue;
                    const double x1 = l.cursorToX(s), x2 = l.cursorToX(e);
                    const double yb = l.position().y() - frameY(frame) + l.ascent() + 2;
                    QPainterPath wave;
                    wave.moveTo(std::min(x1, x2), yb);
                    for (double x = std::min(x1, x2); x < std::max(x1, x2); x += 2) wave.lineTo(x + 1, yb + ((int(x / 2) % 2) ? -1 : 1));
                    p->setPen(QPen(QColor(220, 30, 30), 0.6));
                    p->setBrush(Qt::NoBrush);
                    p->drawPath(wave);
                }
            }

        if (B->lines.first().frame == frame) {
            const PtLine l0(B->tl->lineAt(0));
            if (!B->marker.isEmpty()) {
                p->setPen(B->markerColor);
                const QPointF at(B->markerX, l0.position().y() - frameY(frame) + l0.ascent());
                if (B->markerFit) drawFittedMarker(p, at, B->markerFont, B->marker, B->markerInk, B->markerRound, B->markerColor, B->markerOutline);
                else if (B->markerOutline) drawOutlinedText(p, at, B->markerFont, B->marker, B->markerColor);
                else drawPlainText(p, at, B->markerFont, B->marker);
            }
            if (!B->dropText.isEmpty()) {
                // A raised cap stands on the first line; a dropped one on the last line it spans.
                const int n = B->dropUp ? 1 : std::min<int>(B->dropLines, B->lines.size());
                const PtLine ln(B->tl->lineAt(n - 1));
                const double baseline = ln.position().y() - frameY(frame) + ln.ascent();
                p->setPen(B->dropColor);
                drawPlainText(p, QPointF(l0.position().x() - B->dropWidth, baseline), B->dropFont, B->dropText);
            }
        }

        if (o.showSpecial) {
            const QFont sf = pointFont(B->tl->font());
            p->setPen(QColor(90, 120, 200));
            for (int i = 0; i < B->lines.size(); ++i) {
                if (B->lines[i].frame != frame) continue;
                const PtLine l(B->tl->lineAt(i));
                const double base = l.position().y() - frameY(frame) + l.ascent();
                for (int k = l.textStart(); k < l.textStart() + l.textLength(); ++k) {
                    const QChar ch = B->disp[k];
                    if (ch == ' ' || ch == '\t' || ch == QChar::LineSeparator) {
                        const double x1 = l.cursorToX(k), x2 = l.cursorToX(k + 1);
                        const QString mark = ch == ' ' ? QStringLiteral("·") : ch == '\t' ? QStringLiteral("→") : QStringLiteral("↵");
                        const double w = advanceOf(sf, mark);
                        drawPlainText(p, QPointF((x1 + x2 - w) / 2, base), sf, mark);
                    }
                }
                if (i == B->lines.size() - 1) drawPlainText(p, QPointF(l.cursorToX(l.textStart() + l.textLength()) + 1, base), sf, QStringLiteral("¶"));
            }
        }
    }
    paintTextEffects(p, frame, true);
    // Notes: their rules, the endnotes' heading, each number and text.
    for (const Rule &r : m_noteRules)
        if (r.frame == frame) {
            p->setPen(QPen(QColor(0, 0, 0), 0.5 * m_env.fontScale));
            p->drawLine(r.line);
        }
    if (m_notesHeading.frame == frame) {
        p->setPen(m_notesHeading.color);
        drawPlainText(p, m_notesHeading.baseline, m_notesHeading.font, QCoreApplication::translate("Text", "Notes"));
    }
    for (const Note &n : m_notes) {
        if (n.frame == frame && n.layout) {
            p->setPen(n.numberColor);
            if (!m_env.wordLines) drawPlainText(p, QPointF(n.rect.left(), n.rect.top() + n.firstBaseline), n.numberFont, QString::number(n.number) + (n.endnote ? QStringLiteral(".") : QString()));
            p->save();
            p->translate(n.rect.topLeft());
            PaintOptions inNote;
            inNote.output = o.output;
            n.layout->paint(p, 0, inNote);
            p->restore();
        }
        for (const Note::Piece &pc : n.more) {
            if (pc.frame != frame || !pc.layout) continue;
            p->save();
            p->translate(pc.rect.topLeft());
            PaintOptions inNote;
            inNote.output = o.output;
            pc.layout->paint(p, 0, inNote);
            p->restore();
        }
    }
    if (o.selFrom >= 0 && o.selTo > o.selFrom)
        for (const QRectF &r : rangeRects(frame, o.selFrom, o.selTo)) p->fillRect(r, o.selColor);
    p->restore();
}

} // namespace jp
