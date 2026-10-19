#include "core/writedoc.h"

#include "core/wordformat.h"
#include "core/writeprops.h"
#include "doc/doc.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include <QDataStream>
#include <QLocale>
#include <QRegularExpression>
#include <QSet>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <algorithm>
#include <array>
#include <cmath>

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

// ---- sections

QByteArray packSection(const SectionProps &s)
{
    QByteArray out;
    QDataStream d(&out, QIODevice::WriteOnly);
    d.setVersion(QDataStream::Qt_6_0);
    d << s.type << qint32(s.pageW) << qint32(s.pageH) << s.orient << qint32(s.top) << qint32(s.right) << qint32(s.bottom) << qint32(s.left) << qint32(s.header)
      << qint32(s.footer) << qint32(s.gutter) << qint32(s.cols) << qint32(s.colSpace) << s.colSep << s.colEqual << qint32(s.colDefs.size());
    for (const ColumnDef &c : s.colDefs) d << qint32(c.width) << qint32(c.space);
    d << qint32(s.pageStart.value_or(-1)) << s.pageFormat << s.titlePg << s.hasPgSz << s.hasPgMar << s.hasCols << s.attrs << packRaw(s.extra) << s.pgSzAttrs << s.pgMarAttrs << s.pgNumAttrs << s.colsAttrs;
    return out;
}

SectionProps unpackSection(const QByteArray &packed)
{
    SectionProps s;
    QDataStream d(packed);
    d.setVersion(QDataStream::Qt_6_0);
    qint32 pageW = 0, pageH = 0, top = 0, right = 0, bottom = 0, left = 0, header = 0, footer = 0, gutter = 0, cols = 0, colSpace = 0, n = 0, start = 0;
    d >> s.type >> pageW >> pageH >> s.orient >> top >> right >> bottom >> left >> header >> footer >> gutter >> cols >> colSpace >> s.colSep >> s.colEqual >> n;
    s.pageW = pageW;
    s.pageH = pageH;
    s.top = top;
    s.right = right;
    s.bottom = bottom;
    s.left = left;
    s.header = header;
    s.footer = footer;
    s.gutter = gutter;
    s.cols = cols;
    s.colSpace = colSpace;
    for (qint32 i = 0; i < n && i < 100 && d.status() == QDataStream::Ok; ++i) {
        qint32 w = 0, sp = 0;
        d >> w >> sp;
        s.colDefs.push_back({w, sp});
    }
    QByteArray extra;
    d >> start >> s.pageFormat >> s.titlePg >> s.hasPgSz >> s.hasPgMar >> s.hasCols >> s.attrs >> extra >> s.pgSzAttrs >> s.pgMarAttrs >> s.pgNumAttrs >> s.colsAttrs;
    if (start >= 0) s.pageStart = start;
    s.extra = unpackRaw(extra);
    return s;
}

// A section's page borders (w:pgBorders, kept as written among the section's properties).
flow::PageBorders pageBordersOf(const QVector<RawChild> &extra)
{
    flow::PageBorders out;
    static const QRegularExpression isBorders(QStringLiteral("^<(?:[\\w.-]+:)?pgBorders\\b([^>]*)>"));
    static const QRegularExpression side(QStringLiteral("<(?:[\\w.-]+:)?(top|left|bottom|right)\\b([^>]*?)/?>"));
    static const QRegularExpression attribute(QStringLiteral("\\s(?!xmlns)(?:[\\w.-]+:)?([\\w.-]+)\\s*=\\s*\"([^\"]*)\""));
    auto attributes = [&](const QString &text) {
        QHash<QString, QString> at;
        auto it = attribute.globalMatch(QLatin1Char(' ') + text);
        while (it.hasNext()) {
            const QRegularExpressionMatch m = it.next();
            at.insert(m.captured(1), m.captured(2));
        }
        return at;
    };
    for (const RawChild &ch : extra) {
        const QString xml = QString::fromUtf8(ch.xml);
        const QRegularExpressionMatch root = isBorders.match(xml);
        if (!root.hasMatch()) continue;
        const QHash<QString, QString> rootAt = attributes(root.captured(1));
        out.fromPage = rootAt.value(QStringLiteral("offsetFrom")) == QLatin1String("page");
        const QString display = rootAt.value(QStringLiteral("display"));
        out.show = display == QLatin1String("firstPage") ? flow::PageBorders::Show::FirstPage : display == QLatin1String("notFirstPage") ? flow::PageBorders::Show::NotFirstPage : flow::PageBorders::Show::All;
        static const QStringList lines = {QStringLiteral("single"), QStringLiteral("thick"), QStringLiteral("double"), QStringLiteral("dotted"), QStringLiteral("dashed"), QStringLiteral("dotDash"),
                                          QStringLiteral("dotDotDash"), QStringLiteral("triple"), QStringLiteral("thinThickSmallGap"), QStringLiteral("thickThinSmallGap"),
                                          QStringLiteral("thinThickThinSmallGap"), QStringLiteral("thinThickMediumGap"), QStringLiteral("thickThinMediumGap"),
                                          QStringLiteral("thinThickThinMediumGap"), QStringLiteral("thinThickLargeGap"), QStringLiteral("thickThinLargeGap"), QStringLiteral("thinThickThinLargeGap"),
                                          QStringLiteral("wave"), QStringLiteral("doubleWave"), QStringLiteral("dashSmallGap"), QStringLiteral("dashDotStroked"), QStringLiteral("threeDEmboss"),
                                          QStringLiteral("threeDEngrave"), QStringLiteral("outset"), QStringLiteral("inset")};
        auto it = side.globalMatch(xml);
        while (it.hasNext()) {
            const QRegularExpressionMatch m = it.next();
            const QHash<QString, QString> at = attributes(m.captured(2));
            const QString val = at.value(QStringLiteral("val"));
            if (val.isEmpty() || val == QLatin1String("nil") || val == QLatin1String("none")) continue;
            flow::PageBorders::Side s;
            s.present = true;
            s.width = std::clamp(at.value(QStringLiteral("sz"), QStringLiteral("4")).toDouble() / 8.0, 0.25, 12.0);
            s.space = std::clamp(at.value(QStringLiteral("space")).toDouble(), 0.0, 31.0);
            const QString color = at.value(QStringLiteral("color"));
            s.color = color.size() == 6 && QColor(QLatin1Char('#') + color).isValid() ? QColor(QLatin1Char('#') + color) : QColor(Qt::black);
            s.art = !lines.contains(val);
            s.line = val == QLatin1String("double") ? flow::PageBorders::Side::Line::Double : val == QLatin1String("dotted") ? flow::PageBorders::Side::Line::Dotted
                     : val == QLatin1String("dashed") || val == QLatin1String("dashSmallGap") || val == QLatin1String("dotDash") || val == QLatin1String("dotDotDash") ? flow::PageBorders::Side::Line::Dashed
                                                                                                                                                        : flow::PageBorders::Side::Line::Single;
            const QString name = m.captured(1);
            (name == QLatin1String("top") ? out.top : name == QLatin1String("left") ? out.left : name == QLatin1String("bottom") ? out.bottom : out.right) = s;
        }
        break;
    }
    return out;
}

// A section's line numbers (w:lnNumType, kept as written among the section's properties).
flow::Section::LineNumbers lineNumbersOf(const QVector<RawChild> &extra)
{
    flow::Section::LineNumbers out;
    static const QRegularExpression isNumbers(QStringLiteral("^<(?:[\\w.-]+:)?lnNumType\\b([^>]*)>"));
    static const QRegularExpression attribute(QStringLiteral("\\s(?!xmlns)(?:[\\w.-]+:)?([\\w.-]+)\\s*=\\s*\"([^\"]*)\""));
    for (const RawChild &child : extra) {
        const QRegularExpressionMatch head = isNumbers.match(QString::fromUtf8(child.xml.left(400)));
        if (!head.hasMatch()) continue;
        QHash<QString, QString> at;
        auto it = attribute.globalMatch(QLatin1Char(' ') + head.captured(1));
        while (it.hasNext()) {
            const QRegularExpressionMatch m = it.next();
            at.insert(m.captured(1), m.captured(2));
        }
        out.countBy = std::clamp(at.value(QStringLiteral("countBy"), QStringLiteral("1")).toInt(), 1, 1000);
        out.start = std::clamp(at.value(QStringLiteral("start")).toInt(), 0, 32767) + 1;   // (the file's start is how many numbers are skipped: 3 numbers the first line 4)
        out.distance = std::clamp(at.value(QStringLiteral("distance")).toDouble() / 20.0, 0.0, 144.0);
        const QString restart = at.value(QStringLiteral("restart"));
        out.restart = restart == QLatin1String("newSection") ? flow::Section::LineNumbers::Restart::NewSection : restart == QLatin1String("continuous") ? flow::Section::LineNumbers::Restart::Continuous
                                                                                                                                             : flow::Section::LineNumbers::Restart::NewPage;
        break;
    }
    return out;
}

flow::Section flowSectionOf(const SectionProps &p, bool evenAndOdd)
{
    flow::Section s;
    const double w = p.pageW / 20.0, h = p.pageH / 20.0;
    const bool landscape = w > h;
    s.paper = landscape ? QSizeF(h, w) : QSizeF(w, h);
    s.orientation = landscape ? flow::Section::Orientation::Landscape : flow::Section::Orientation::Portrait;
    s.margins = QMarginsF(std::max(0, p.left + p.gutter) / 20.0, std::abs(p.top) / 20.0, std::max(0, p.right) / 20.0, std::abs(p.bottom) / 20.0);
    s.columns = std::max(1, p.cols);
    s.columnGap = p.colSpace / 20.0;
    s.headerDistance = p.header / 20.0;
    s.footerDistance = p.footer / 20.0;
    s.differentFirstPage = p.titlePg;
    s.differentOddEven = evenAndOdd;
    s.continuous = p.type == QLatin1String("continuous");
    s.startParity = p.type == QLatin1String("oddPage") ? flow::Section::Parity::Odd : p.type == QLatin1String("evenPage") ? flow::Section::Parity::Even : flow::Section::Parity::Any;
    s.pageBorders = pageBordersOf(p.extra);
    s.lineNumbers = lineNumbersOf(p.extra);
    s.firstPageNumber = p.pageStart;
    // (Page numbers that carry a chapter's number: the heading level of the style (w:chapStyle) and the separator (w:chapSep: a hyphen when it says none).)
    {
        static const QRegularExpression style(QStringLiteral("[\\s:]chapStyle\\s*=\\s*\"([0-9]+)\"")), separator(QStringLiteral("[\\s:]chapSep\\s*=\\s*\"(\\w+)\""));
        const QString attrs = QString::fromUtf8(p.pgNumAttrs.left(2000));
        if (const QRegularExpressionMatch m = style.match(attrs); m.hasMatch()) s.chapterStyle = std::clamp(m.captured(1).left(3).toInt(), 0, 9);
        const QString sep = separator.match(attrs).captured(1);
        s.chapterSeparator = sep == QLatin1String("period") ? QStringLiteral(".") : sep == QLatin1String("colon") ? QStringLiteral(":") : sep == QLatin1String("emDash") ? QString(QChar(0x2014))
                             : sep == QLatin1String("enDash") ? QString(QChar(0x2013)) : QStringLiteral("-");
    }
    s.pageNumberFormat = p.pageFormat == QLatin1String("lowerRoman") ? QStringLiteral("roman")
                         : p.pageFormat == QLatin1String("upperRoman") ? QStringLiteral("ROMAN")
                         : p.pageFormat == QLatin1String("lowerLetter") ? QStringLiteral("alpha")
                         : p.pageFormat == QLatin1String("upperLetter") ? QStringLiteral("ALPHA")
                         : p.pageFormat == QLatin1String("numberInDash") ? QStringLiteral("dash")
                                                                       : QString();
    return s;
}

void updateSectionProps(SectionProps *p, const flow::Section &s)
{
    const flow::Section was = flowSectionOf(*p, s.differentOddEven);
    const bool landscape = s.orientation == flow::Section::Orientation::Landscape;
    if (s.paper != was.paper || s.orientation != was.orientation) {
        const QSizeF shown = landscape ? QSizeF(s.paper.height(), s.paper.width()) : s.paper;
        p->pageW = int(std::lround(shown.width() * 20));
        p->pageH = int(std::lround(shown.height() * 20));
        p->orient = landscape ? QStringLiteral("landscape") : QStringLiteral("portrait");
        p->hasPgSz = true;
    }
    if (s.margins != was.margins) {
        p->left = std::max(0, int(std::lround(s.margins.left() * 20)) - p->gutter);
        p->right = int(std::lround(s.margins.right() * 20));
        p->top = int(std::lround(s.margins.top() * 20));
        p->bottom = int(std::lround(s.margins.bottom() * 20));
        p->hasPgMar = true;
    }
    if (s.columns != was.columns || s.columnGap != was.columnGap) {
        p->cols = std::max(1, s.columns);
        p->colSpace = int(std::lround(s.columnGap * 20));
        p->colDefs.clear();
        p->colEqual = true;
        p->hasCols = true;
    }
    if (s.headerDistance != was.headerDistance) p->header = int(std::lround(s.headerDistance * 20));
    if (s.footerDistance != was.footerDistance) p->footer = int(std::lround(s.footerDistance * 20));
    if (s.differentFirstPage != was.differentFirstPage) p->titlePg = s.differentFirstPage;
    if (s.firstPageNumber != was.firstPageNumber) p->pageStart = s.firstPageNumber;
    if (s.pageNumberFormat != was.pageNumberFormat)
        p->pageFormat = s.pageNumberFormat == QLatin1String("roman") ? QStringLiteral("lowerRoman")
                        : s.pageNumberFormat == QLatin1String("ROMAN") ? QStringLiteral("upperRoman")
                        : s.pageNumberFormat == QLatin1String("alpha") ? QStringLiteral("lowerLetter")
                        : s.pageNumberFormat == QLatin1String("ALPHA") ? QStringLiteral("upperLetter")
                        : s.pageNumberFormat == QLatin1String("dash") ? QStringLiteral("numberInDash")
                                                                       : QString();
}

bool isStoredProperty(int property)
{
    switch (property) {
    case prop::Atom: case prop::ParaAttrs: case prop::RawBefore: case prop::RawAfter: case prop::RawBlock: case prop::RawPPr: case prop::SpacingAttrs:
    case prop::IndAttrs: case prop::OddTabs: case prop::SectionData: case prop::RawRPr: case prop::LastSection: case prop::MarkFormat: return true;
    default: return false;
    }
}

QVariant resolveStored(const WriteDoc &, const QVariant &value, int property)
{
    return isStoredProperty(property) && value.typeId() == QMetaType::Int ? QVariant(storedBytesOf(value.toInt())) : value;
}

// ---- the document

namespace {
int s_openDocuments = 0;   // (the store of bytes the formats refer to is emptied with the last)
quint64 s_lastId = 0;
}

WriteDoc::WriteDoc() : m_id(++s_lastId), m_flow(std::make_unique<flow::FlowDocument>()) { ++s_openDocuments; }

WriteDoc::~WriteDoc()
{
    // (The paginator watches the story, which the flowing document owns: it goes first.)
    m_pager.reset();
    m_flow.reset();
    if (--s_openDocuments == 0) clearByteStore();
}

std::unique_ptr<WriteDoc> WriteDoc::blank()
{
    std::unique_ptr<WriteDoc> doc(new WriteDoc());
    office::wml::Document d = newDocument();
    if (QLocale::system().measurementSystem() != QLocale::ImperialUSSystem) {
        d.lastSection.pageW = 11906;   // A4
        d.lastSection.pageH = 16838;
    }
    doc->build(std::move(d));
    return doc;
}

std::unique_ptr<WriteDoc> WriteDoc::fromDocx(const QByteArray &zip, QString *error, QStringList *notes)
{
    office::wml::Document d;
    ReadNotes rn;
    if (!readDocx(zip, &d, error, &rn)) return nullptr;
    std::unique_ptr<WriteDoc> doc(new WriteDoc());
    doc->readNotes = rn.notes;
    doc->m_hadMacros = rn.hadMacros;
    doc->m_leftOut = rn.leftOut;
    if (notes) *notes = rn.notes;
    doc->build(std::move(d));
    return doc;
}

std::unique_ptr<WriteDoc> WriteDoc::fromDoc(const QByteArray &bytes, QString *error, QStringList *notes)
{
    office::wml::Document d;
    ReadNotes rn;
    if (!office::doc::readDoc(bytes, &d, error, &rn)) return nullptr;
    std::unique_ptr<WriteDoc> doc(new WriteDoc());
    doc->readNotes = rn.notes;
    doc->m_hadMacros = rn.hadMacros;
    doc->m_leftOut = rn.leftOut;
    if (notes) *notes = rn.notes;
    doc->build(std::move(d));
    return doc;
}

std::unique_ptr<WriteDoc> WriteDoc::fromFile(const QByteArray &bytes, QString *error, QStringList *notes)
{
    return office::doc::looksLikeCompoundFile(bytes) ? fromDoc(bytes, error, notes) : fromDocx(bytes, error, notes);
}

void WriteDoc::build(office::wml::Document &&doc)
{
    m_wml = std::move(doc);
    QVector<Block> body;
    body.swap(m_wml.body);
    m_last = m_wml.lastSection;
    setUpEnvironment();
    loadNotes();
    loadEndnotes();
    buildStory(body);
    measureNoteSeparator();
    settleTables();
    syncSections();
    m_pager = std::make_unique<flow::Paginator>(m_flow.get());
    QTextDocument *s = story();
    s->setUndoRedoEnabled(true);
    s->setModified(false);
    m_blockCount = s->blockCount();
    connect(s, &QTextDocument::contentsChange, this, &WriteDoc::storyChanged);
}

void WriteDoc::setUpEnvironment()
{
    flow::FlowDocument &f = *m_flow;
    if (m_wml.hasTheme) {
        const Theme &t = m_wml.theme;
        if (!t.majorLatin.isEmpty()) f.env.fonts.heading = t.majorLatin;
        if (!t.minorLatin.isEmpty()) f.env.fonts.body = t.minorLatin;
        f.env.fonts.name = t.fontSchemeName;
        ColorScheme cs = f.env.colors;
        cs.name = t.colorSchemeName;
        cs.c[Main] = t.colors[Theme::Dark1];
        cs.c[Accent1] = t.colors[Theme::Accent1];
        cs.c[Accent2] = t.colors[Theme::Accent2];
        cs.c[Accent3] = t.colors[Theme::Accent3];
        cs.c[Accent4] = t.colors[Theme::Accent4];
        cs.c[Accent5] = t.colors[Theme::Accent5];
        cs.c[jp::Hyperlink] = t.colors[Theme::Hyperlink];
        cs.c[Followed] = t.colors[Theme::FollowedHyperlink];
        f.env.colors = cs;
    }
    f.env.defaultTab = std::max(1, m_wml.defaultTabStop) / 20.0;
    // The page color (w:background), when it is a color.
    {
        static const QRegularExpression background(QStringLiteral("<(?:[\\w.-]+:)?background\\b[^>]*?\\bcolor=\"([0-9A-Fa-f]{6})\""));
        const QRegularExpressionMatch m = background.match(QString::fromUtf8(m_wml.beforeBody));
        f.pageColor = m.hasMatch() ? QColor(QLatin1Char('#') + m.captured(1)) : QColor();
    }
    f.env.wordLines = true;
    // Punctuation hangs past the margin only where the file has the East Asian layout on (and not its own option against it).
    f.env.wordNoHang = true;
    if (!m_wml.settingsName.isEmpty()) {
        // (One reading of the part for all of them.)
        const office::wml::CompatOptions compat = office::wml::readCompatOptions(partData(m_wml.settingsName));
        // Words are hyphenated where the file asks (w:autoHyphenation), but for the zone that it sets.
        f.hyphenate = compat.autoHyphenation;
        f.hyphenZone = compat.hyphenationZone / 20.0;
        // (And how the endnotes are numbered, read here once, not at each story's making.)
        const office::wml::NoteNumbering endnotes = office::wml::readNoteNumbering(partData(m_wml.settingsName), "endnotePr");
        if (!endnotes.format.isEmpty()) m_endnoteFormat = endnotes.format;
        if (endnotes.start > 0) m_endnoteStart = endnotes.start;
        f.env.wordNoHang = !compat.useFELayout || compat.doNotWrapTextWithPunct;
        f.env.wordSpaceSums = compat.doNotUseHTMLParagraphAutoSpacing;
        f.env.wordNoHangStop = compat.noTabHangInd;
        f.env.wordNoNumberIndentStop = compat.doNotUseIndentAsNumberingTabStop;
        // Lines of whole points are the 2003 programs' (every practice document that asks for them is of compatibility mode 11): not a file of the 2010 program's mode (14) or after.
        f.env.wordWholePointLines = compat.suppressTopSpacingWP && m_wml.compatMode < 14;
        f.env.wordNoLeading = compat.noLeading;
        // Full justification in the older program's way: a justified line may give up a little over a third of each space to take in one more word (the narrowest spaces in
        // the files' own pages are 0.64 of a space).
        f.env.wordSpaceShrink = compat.wpJustification ? 0.36 : 0.0;
        f.markFollowsBreak = m_wml.compatMode >= 15 || compat.splitPgBreakAndParaMark;
        f.spaceBeforeAfterBreak = !compat.suppressSpBfAfterPgBrk;
    }
    // The stand-ins for the fonts the file names that the machine doesn't have, by what its font table says of each (wordStandInFor).
    for (const KeptPart &part : m_wml.parts) {
        if (!part.name.endsWith(QLatin1String("fontTable.xml"), Qt::CaseInsensitive)) continue;
        QVector<office::wml::FontEntry> fonts;
        if (office::wml::readFonts(partData(part.name), &fonts))
            for (const office::wml::FontEntry &font : fonts) {
                if (font.name.isEmpty()) continue;
                if (const QString standIn = wordStandInFor(font.alternate, font.family, font.pitch, font.panose, font.charset); !standIn.isEmpty())
                    f.env.standIns.insert(font.name.toLower(), standIn);
            }
        break;
    }
    QFont body(f.env.fonts.body);
    body.setPointSizeF(m_wml.styles.defaultRun.size.value_or(22) / 2.0);
    f.story()->setDefaultFont(body);
}

void WriteDoc::storyChanged(int position, int removed, int added)
{
    if (m_syncing) return;
    const int blocks = story()->blockCount();
    if (blocks != m_blockCount || removed > 1 || added > 1) {
        m_blockCount = blocks;
        syncSections();
    } else if (position == 0 && storedBytes(story()->begin().blockFormat(), prop::LastSection) != m_lastBytes) {
        syncSections();   // (the last section's properties are the first paragraph's, and an undo may have changed them)
    }
}

// ---- the file's links and parts

Relationship WriteDoc::mainRelationship(const QString &id) const
{
    for (const Relationship &r : m_wml.mainRelationships)
        if (r.id == id) return r;
    return {};
}

QByteArray WriteDoc::partData(const QString &name, QString *contentType) const
{
    for (const KeptPart &p : m_wml.parts)
        if (p.name.compare(name, Qt::CaseInsensitive) == 0) {
            if (contentType) *contentType = p.contentType;
            return p.data;
        }
    return {};
}

namespace {
QString nextRelationshipId(const QVector<Relationship> &links)
{
    QSet<QString> used;
    for (const Relationship &r : links) used.insert(r.id);
    for (int n = int(links.size()) + 1;; ++n)
        if (!used.contains(QStringLiteral("rId%1").arg(n))) return QStringLiteral("rId%1").arg(n);
}
} // namespace

QString WriteDoc::addExternalLink(const QString &type, const QString &target)
{
    for (const Relationship &r : m_wml.mainRelationships)
        if (r.external && r.type == type && r.target == target) return r.id;
    Relationship r;
    r.id = nextRelationshipId(m_wml.mainRelationships);
    r.type = type;
    r.target = target;
    r.external = true;
    m_wml.mainRelationships.push_back(r);
    return r.id;
}

QString WriteDoc::addMedia(const QByteArray &bytes, const QString &extension, const QString &contentType)
{
    QSet<QString> taken;
    for (const KeptPart &p : m_wml.parts) taken.insert(p.name.toLower());
    taken.insert(m_wml.mainName.toLower());
    QString name;
    for (int n = 1;; ++n) {
        name = QStringLiteral("/word/media/image%1.%2").arg(n).arg(extension);
        if (!taken.contains(name.toLower())) break;
    }
    KeptPart part;
    part.name = name;
    part.contentType = contentType;
    part.data = bytes;
    m_wml.parts.push_back(part);
    Relationship r;
    r.id = nextRelationshipId(m_wml.mainRelationships);
    r.type = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/image");
    r.target = name;
    m_wml.mainRelationships.push_back(r);
    return r.id;
}

// ---- sections

SectionProps WriteDoc::sectionProps(int index) const
{
    return m_sections[std::clamp(index, 0, int(m_sections.size()) - 1)];
}

void WriteDoc::syncSections()
{
    if (m_syncing) return;
    m_syncing = true;
    // The last section's properties are the first paragraph's, so that a change to them is one step of the text's undo.
    {
        const QByteArray bytes = storedBytes(story()->begin().blockFormat(), prop::LastSection);
        if (!bytes.isEmpty()) m_last = unpackSection(bytes);
        m_lastBytes = bytes;
    }
    // Each break paragraph's record of its section is a key of bytes; which sections changed is found from the keys, and only those are made again
    // (a document of twenty thousand sections is not unpacked at each Enter).
    QVector<int> keys;
    bool absent = false;
    for (QTextBlock b = story()->begin(); b.isValid(); b = b.next()) {
        const QTextBlockFormat bf = b.blockFormat();
        if (!bf.boolProperty(flow::fp::SectionBreakAfter)) continue;
        if (bf.hasProperty(prop::SectionData)) keys.push_back(bf.intProperty(prop::SectionData));
        else absent = true;
    }
    keys.push_back(storeBytes(packSection(m_last)));
    flow::FlowDocument &f = *m_flow;
    int from = 0;
    bool changed = false;
    if (absent) {
        // (A break with no record of its own takes the section before's: found the long way.)
        std::vector<SectionProps> list;
        for (QTextBlock b = story()->begin(); b.isValid(); b = b.next()) {
            const QTextBlockFormat bf = b.blockFormat();
            if (!bf.boolProperty(flow::fp::SectionBreakAfter)) continue;
            list.push_back(bf.hasProperty(prop::SectionData) ? unpackSection(storedBytes(bf, prop::SectionData)) : (list.empty() ? m_last : list.back()));
        }
        list.push_back(m_last);
        keys.clear();
        for (const SectionProps &p : list) keys.push_back(storeBytes(packSection(p)));
        m_sectionsMade += qint64(list.size());
        changed = keys != m_sectionKeys;
        m_sections = list;
        f.sections.clear();
        for (const SectionProps &p : m_sections) f.sections << flowSectionOf(p, m_wml.evenAndOddHeaders);
    } else if (keys != m_sectionKeys || f.sections.size() != int(keys.size())) {
        changed = true;
        const int oldCount = int(m_sectionKeys.size()), newCount = int(keys.size());
        int prefix = 0;
        while (prefix < oldCount && prefix < newCount && m_sectionKeys[prefix] == keys[prefix]) ++prefix;
        int suffix = 0;
        while (suffix < oldCount - prefix && suffix < newCount - prefix && m_sectionKeys[oldCount - 1 - suffix] == keys[newCount - 1 - suffix]) ++suffix;
        from = prefix;
        std::vector<SectionProps> made;
        QVector<flow::Section> madeFlow;
        for (int i = prefix; i < newCount - suffix; ++i) {
            SectionProps p = i == newCount - 1 ? m_last : unpackSection(storedBytesOf(keys[i]));
            madeFlow << flowSectionOf(p, m_wml.evenAndOddHeaders);
            made.push_back(std::move(p));
            ++m_sectionsMade;
        }
        if (int(m_sections.size()) != oldCount || f.sections.size() != oldCount) {
            // (Out of step with the keys, as when the document is first built: all of them.)
            m_sections.assign(newCount, SectionProps());
            f.sections.resize(newCount);
            for (int i = 0; i < newCount; ++i) {
                m_sections[i] = i == newCount - 1 ? m_last : unpackSection(storedBytesOf(keys[i]));
                f.sections[i] = flowSectionOf(m_sections[i], m_wml.evenAndOddHeaders);
            }
            m_sectionsMade += newCount;
            from = 0;
        } else {
            m_sections.erase(m_sections.begin() + prefix, m_sections.begin() + (oldCount - suffix));
            m_sections.insert(m_sections.begin() + prefix, made.begin(), made.end());
            f.sections.remove(prefix, oldCount - suffix - prefix);
            for (int i = 0; i < madeFlow.size(); ++i) f.sections.insert(prefix + i, madeFlow[i]);
        }
    }
    m_sectionKeys = keys;
    if (changed) {
        attachHeaders(from);
        if (m_pager) m_pager->invalidateFrom(from);
        Q_EMIT sectionsChanged();
    }
    m_syncing = false;
}

namespace {
// The most distinct header and footer parts that are read; a file with more has the rest left out (a section's reference to one shows nothing).
constexpr int kMostHeaderParts = 64;

// A header or footer as the text stands: the paragraphs whose marks a tracked change deleted, with nothing of their own to show, are gone. (Not the last
// paragraph, which has no mark to join the next, nor one in a table.)
void leaveOutDeletedParagraphs(QVector<Block> *blocks)
{
    int last = -1;
    for (int i = 0; i < blocks->size(); ++i)
        if (!(*blocks)[i].isRaw && (*blocks)[i].tag.isEmpty()) last = i;
    QVector<Block> kept;
    int depth = 0;
    for (int i = 0; i < blocks->size(); ++i) {
        const Block &b = (*blocks)[i];
        if (!b.isRaw && b.tag == QLatin1String("tbl")) depth += b.role == Inline::Open ? 1 : b.role == Inline::Close ? -1 : 0;
        if (!b.isRaw && b.tag.isEmpty() && depth == 0 && i != last && isDeletedAway(b.para)) continue;
        kept << b;
    }
    *blocks = kept;
}
} // namespace

std::shared_ptr<QTextDocument> WriteDoc::headerStory(const QString &relationshipId)
{
    const Relationship rel = mainRelationship(relationshipId);
    if (rel.id.isEmpty() || rel.external) return nullptr;
    const auto found = m_headerStories.constFind(rel.target);
    if (found != m_headerStories.constEnd()) return found.value();
    if (m_headerStories.size() >= kMostHeaderParts) return nullptr;
    QVector<Block> blocks;
    auto doc = std::make_shared<QTextDocument>();
    if (readBlocks(partData(rel.target), &blocks, true)) {
        leaveOutDeletedParagraphs(&blocks);
        doc->setDocumentMargin(0);
        doc->setDefaultFont(story()->defaultFont());
        doc->setUndoRedoEnabled(false);
        fillStory(doc.get(), blocks, true, rel.target, true);
    }
    m_headerStories.insert(rel.target, doc);
    return doc;
}

void WriteDoc::attachHeaders(int from)
{
    flow::FlowDocument &f = *m_flow;
    if (m_headerRefs.size() > 4096) m_headerRefs.clear();
    for (int i = std::max(0, from); i < f.sections.size() && i < m_sectionKeys.size() && i < int(m_sections.size()); ++i) {
        auto it = m_headerRefs.find(m_sectionKeys[i]);
        if (it == m_headerRefs.end()) it = m_headerRefs.insert(m_sectionKeys[i], headerRefsOf(m_sections[i]));
        const HeaderRefs &refs = it.value();
        flow::Section &s = f.sections[i];
        const flow::Section *before = i > 0 ? &f.sections[i - 1] : nullptr;
        auto pick = [&](const QString &id, const std::shared_ptr<QTextDocument> &inherited) { return id.isEmpty() ? inherited : headerStory(id); };
        s.header.odd = pick(refs.header[0], before ? before->header.odd : nullptr);
        s.header.first = pick(refs.header[1], before ? before->header.first : nullptr);
        s.header.even = pick(refs.header[2], before ? before->header.even : nullptr);
        s.footer.odd = pick(refs.footer[0], before ? before->footer.odd : nullptr);
        s.footer.first = pick(refs.footer[1], before ? before->footer.first : nullptr);
        s.footer.even = pick(refs.footer[2], before ? before->footer.even : nullptr);
        // (The numbers of the lines are in the file's "line number" character style.)
        if (s.lineNumbers.countBy > 0) s.lineNumbers.format = styleCharFormat(defaultParagraphStyle(), QStringLiteral("LineNumber"));
    }
}

void WriteDoc::setSectionProps(int index, const SectionProps &props)
{
    index = std::clamp(index, 0, int(m_sections.size()) - 1);
    if (index == int(m_sections.size()) - 1) {
        QTextCursor c(story()->begin());
        QTextBlockFormat bf = story()->begin().blockFormat();
        setStored(bf, prop::LastSection, packSection(props));
        c.setBlockFormat(bf);
        m_last = props;
        syncSections();
        return;
    }
    int n = 0;
    for (QTextBlock b = story()->begin(); b.isValid(); b = b.next()) {
        if (!b.blockFormat().boolProperty(flow::fp::SectionBreakAfter)) continue;
        if (n++ != index) continue;
        QTextCursor c(b);
        QTextBlockFormat bf = b.blockFormat();
        setStored(bf, prop::SectionData, packSection(props));
        c.setBlockFormat(bf);
        break;
    }
    syncSections();
}

int WriteDoc::sectionAt(int position) const
{
    int n = 0;
    for (QTextBlock b = story()->begin(); b.isValid() && b.position() + b.length() <= position; b = b.next())
        if (b.blockFormat().boolProperty(flow::fp::SectionBreakAfter)) ++n;
    return std::min(n, int(m_sections.size()) - 1);
}

bool WriteDoc::insertSectionBreak(const QTextBlock &block, const QString &type)
{
    if (!block.isValid() || block.blockFormat().boolProperty(flow::fp::SectionBreakAfter)) return false;
    const int index = sectionAt(block.position());
    const SectionProps here = sectionProps(index);
    QTextCursor c(block);
    QTextBlockFormat bf = block.blockFormat();
    bf.setProperty(flow::fp::SectionBreakAfter, true);
    setStored(bf, prop::SectionData, packSection(here));
    c.setBlockFormat(bf);
    syncSections();
    // The section after the break keeps the properties the section had, as the file's own record of it; how it begins is the break's.
    SectionProps next = sectionProps(index + 1);
    const QString kind = type == QLatin1String("nextPage") ? QString() : type;
    if (next.type != kind) {
        next.type = kind;
        setSectionProps(index + 1, next);
    }
    return true;
}

// ---- styles

QString WriteDoc::defaultParagraphStyle() const
{
    const QString id = m_wml.styles.defaultId(QStringLiteral("paragraph"));
    return id.isEmpty() ? QStringLiteral("Normal") : id;
}

QVector<StyleEntry> WriteDoc::styleEntries() const
{
    QVector<StyleEntry> out;
    for (const Style &s : m_wml.styles.styles) {
        if (s.type != QLatin1String("paragraph") && s.type != QLatin1String("character")) continue;
        StyleEntry e;
        e.id = s.id;
        e.name = s.name.isEmpty() ? s.id : s.name;
        // The built-in names are lower case in the file ("heading 1"); the person sees them capitalized.
        if (!e.name.isEmpty() && e.name[0].isLower()) e.name[0] = e.name[0].toUpper();
        e.character = s.type == QLatin1String("character");
        e.priority = s.isDefault && !e.character ? -1 : s.uiPriority.value_or(99);
        e.quick = s.qFormat && !s.semiHidden;
        out.push_back(e);
    }
    // The styles a person may apply that this document doesn't have yet are offered too: they are added when first used.
    for (const QString &id : builtinStyleIds()) {
        if (m_wml.styles.find(id)) continue;
        const Style s = builtinStyle(id);
        if (s.id.isEmpty() || !s.qFormat || s.type == QLatin1String("table")) continue;
        StyleEntry e;
        e.id = s.id;
        e.name = s.name;
        if (!e.name.isEmpty() && e.name[0].isLower()) e.name[0] = e.name[0].toUpper();
        e.character = s.type == QLatin1String("character");
        e.priority = s.isDefault && !e.character ? -1 : s.uiPriority.value_or(99);
        e.quick = true;
        out.push_back(e);
    }
    std::stable_sort(out.begin(), out.end(), [](const StyleEntry &a, const StyleEntry &b) { return a.priority < b.priority; });
    return out;
}

bool WriteDoc::ensureStyle(const QString &id)
{
    if (id.isEmpty()) return false;
    if (m_wml.styles.find(id)) return true;
    const Style s = builtinStyle(id);
    if (s.id.isEmpty()) return false;
    if (!s.basedOn.isEmpty()) ensureStyle(s.basedOn);
    if (!s.next.isEmpty()) ensureStyle(s.next);
    m_wml.styles.add(s);
    m_scratch.clear();
    return true;
}

const StyleSheet &WriteDoc::sheetFor(const QString &id) const
{
    if (id.isEmpty() || m_wml.styles.find(id) || builtinStyle(id).id.isEmpty()) return m_wml.styles;
    auto it = m_scratch.constFind(id);
    if (it != m_scratch.constEnd()) return **it;
    auto sheet = std::make_shared<StyleSheet>(m_wml.styles);
    std::function<void(const QString &)> add = [&](const QString &sid) {
        if (sid.isEmpty() || sheet->find(sid)) return;
        const Style b = builtinStyle(sid);
        if (b.id.isEmpty()) return;
        add(b.basedOn);
        sheet->add(b);
    };
    add(id);
    m_scratch.insert(id, sheet);
    return *sheet;
}

ParaProps WriteDoc::levelParagraph(int numId, int level) const
{
    if (numId <= 0) return ParaProps();
    const Level *l = m_wml.numbering.levelOf(numId, level);
    return l ? l->pPr : ParaProps();
}

int WriteDoc::deepestLevel(int numId) const
{
    for (int i = 8; i >= 0; --i)
        if (m_wml.numbering.levelOf(numId, i)) return i;
    return -1;
}

ParaProps WriteDoc::baselineParagraph(const QString &paragraphStyle, int numId, int level, bool direct, QHash<QString, ParaProps> *cache, const TableLayer *table) const
{
    const QString style = paragraphStyle.isEmpty() ? defaultParagraphStyle() : paragraphStyle;
    QString key;
    if (cache && numId > 0) {
        key = style + QLatin1Char('\n') + QString::number(numId) + QLatin1Char('\n') + QString::number(level) + (direct ? QLatin1String("\nd") : QLatin1String()) + (table ? QLatin1Char('\n') + table->key : QString());
        const auto it = cache->constFind(key);
        if (it != cache->constEnd()) return it.value();
    }
    ParaProps p = m_wml.styles.paragraph(style, table);
    if (numId > 0) {
        // A paragraph numbered by its style keeps the style's own indents over the list's; one numbered by itself gets the list's.
        if (!direct && p.numId.value_or(0) == numId) p = m_wml.styles.paragraphWithNumbering(style, levelParagraph(numId, level), table);
        else p.overlay(levelParagraph(numId, level));
    }
    p.style = style;
    if (!key.isEmpty()) cache->insert(key, p);
    return p;
}

QTextBlockFormat WriteDoc::styleBlockFormat(const QString &paragraphStyle)
{
    const QString style = paragraphStyle.isEmpty() ? defaultParagraphStyle() : paragraphStyle;
    const StyleSheet &sheet = sheetFor(style);
    ParaProps p = sheet.paragraph(style);
    const int numId = p.numId.value_or(0), level = p.ilvl.value_or(0);
    if (numId > 0) p = sheet.paragraphWithNumbering(style, levelParagraph(numId, level));
    p.style = style;
    // (A style whose paragraphs are in a list: the list itself is made when the style is applied.)
    return paragraphFormatOf(p);
}

// A paragraph's format as its properties say. A file that turns the web page's way of spacing off (doNotUseHTMLParagraphAutoSpacing) has a fixed 5 points, 100 twentieths, where a
// paragraph asks for automatic spacing, as such a program writes it (the paragraph still asks, so that it is written back as it was).
QTextBlockFormat WriteDoc::paragraphFormatOf(const ParaProps &p) const
{
    QTextBlockFormat bf = blockFormatOf(p);
    if (m_flow->env.wordSpaceSums) {
        if (p.beforeAuto.value_or(false)) bf.setTopMargin(5);
        if (p.afterAuto.value_or(false)) bf.setBottomMargin(5);
    }
    return bf;
}

QTextCharFormat WriteDoc::styleCharFormat(const QString &paragraphStyle, const QString &characterStyle)
{
    const QString style = paragraphStyle.isEmpty() ? defaultParagraphStyle() : paragraphStyle;
    const StyleSheet &sheet = sheetFor(style);
    QTextCharFormat f = charFormatOf((characterStyle.isEmpty() ? sheet : sheetFor(characterStyle)).run(style, characterStyle), theme());
    if (const QString language = styleLanguage(style, characterStyle); !language.isEmpty() && language != proofingLanguage()) f.setProperty(tp::Language, language);
    return f;
}

QString WriteDoc::proofingLanguage() const { return languageOf(m_wml.styles.defaultRun.extra); }

QString WriteDoc::styleLanguage(const QString &paragraphStyle, const QString &characterStyle) const
{
    for (const QString &first : {characterStyle, paragraphStyle.isEmpty() ? defaultParagraphStyle() : paragraphStyle}) {
        const StyleSheet &sheet = first.isEmpty() ? m_wml.styles : sheetFor(first);
        QStringList seen;
        for (QString at = first; !at.isEmpty() && !seen.contains(at) && seen.size() < 32; ) {
            seen << at;
            const Style *s = sheet.find(at);
            if (!s) break;
            if (const QString language = languageOf(s->rPr.extra); !language.isEmpty()) return language;
            at = s->basedOn;
        }
    }
    return QString();
}

// ---- lists

namespace {

AbstractNum plainDefinition(int id, bool numbers)
{
    AbstractNum a;
    a.id = id;
    a.multiLevelType = QStringLiteral("hybridMultilevel");
    static const char *const bullets[] = {"•", "◦", "▪"};
    static const char *const formats[] = {"decimal", "lowerLetter", "lowerRoman"};
    for (int i = 0; i < 9; ++i) {
        Level &l = a.levels[i];
        l.present = true;
        l.jc = QStringLiteral("left");
        if (numbers) {
            l.numFmt = QString::fromLatin1(formats[i % 3]);
            l.text = QStringLiteral("%%1.").arg(i + 1);
        } else {
            l.numFmt = QStringLiteral("bullet");
            l.text = QString::fromUtf8(bullets[i % 3]);
        }
        l.pPr.indLeft = 720 * (i + 1);
        l.pPr.indFirst = -360;
    }
    return a;
}

} // namespace

int WriteDoc::newListInstance(bool numbered)
{
    Numbering &n = m_wml.numbering;
    // The definition that is plain bullets or numbers already, if the file has one.
    int abstractId = -1;
    for (const AbstractNum &a : n.abstracts) {
        const Level &l = a.levels[0];
        if (!l.present || l.legal || l.pPr.indLeft != 720 || l.pPr.indFirst != -360) continue;
        const bool isNumber = l.numFmt != QLatin1String("bullet");
        if (isNumber == numbered && l.text.count(QLatin1Char('%')) <= (numbered ? 1 : 0) && a.levels[1].present) {
            abstractId = a.id;
            break;
        }
    }
    if (!numbered && abstractId >= 0)
        for (const Num &num : n.nums)
            if (num.abstractId == abstractId && num.startOverrides.isEmpty()) return num.id;
    if (abstractId < 0) {
        AbstractNum a = plainDefinition(n.nextAbstractId(), numbered);
        abstractId = a.id;
        n.abstracts.push_back(a);
    }
    Num num;
    num.id = n.nextNumId();
    num.abstractId = abstractId;
    if (numbered) num.startOverrides.insert(0, 1);
    n.nums.push_back(num);
    n.modified = true;
    if (m_wml.numberingName.isEmpty()) {
        m_wml.numberingName = QStringLiteral("/word/numbering.xml");
        int id = 1;
        for (bool taken = true; taken;) {
            taken = false;
            for (const Relationship &r : m_wml.mainRelationships) taken = taken || r.id == QStringLiteral("rId%1").arg(id);
            if (taken) ++id;
        }
        m_wml.mainRelationships.push_back(Relationship{QStringLiteral("rId%1").arg(id), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering"),
                                                       m_wml.numberingName, false});
    }
    return num.id;
}

namespace { int s_levelsScannedForOutline = 0; }
int levelsScannedForOutlineForTest() { return s_levelsScannedForOutline; }

// Whether a level's run properties ask for the outline effect (w:outline, on unless it says off), which draws its bullets hollow. A level is looked at once: a file can keep any
// number of properties of its own in it, and every item of the list asks.
bool WriteDoc::levelIsOutlined(const Level &level, int key) const
{
    if (const auto found = m_levelOutlines.constFind(key); found != m_levelOutlines.constEnd()) return found.value();
    ++s_levelsScannedForOutline;
    static const QRegularExpression outline(QStringLiteral("^<[^\\s:>/]+:outline[\\s/>]"));
    static const QRegularExpression off(QStringLiteral("[\\s:]val\\s*=\\s*\"(0|false|off)\""));
    bool outlined = false;
    for (const RawChild &ch : level.rPr.extra) {
        const QString xml = QString::fromUtf8(ch.xml.left(120));
        if (outline.match(xml).hasMatch()) outlined = !off.match(xml).hasMatch();
    }
    m_levelOutlines.insert(key, outlined);
    return outlined;
}

QTextListFormat WriteDoc::listFormat(int numId, int level, int start) const
{
    QTextListFormat lf;
    level = std::clamp(level, 0, 8);
    lf.setIndent(level + 1);
    lf.setProperty(tp::ListKey, numId);   // (an item of a list is told from another list's by it, whether or not its level is in the definition)
    const Level *found = m_wml.numbering.levelOf(numId, level);
    if (!found) {
        lf.setStyle(QTextListFormat::ListDisc);
        return lf;
    }
    const Level &l = *found;
    // (A level's look is cached by the definition and level; one that an id writes out itself, by the id.)
    const Num *numbered = m_wml.numbering.num(numId);
    const int outlineKey = numbered && numbered->levelOverrides.contains(level) ? (1 << 28) + numId * 16 + level : m_wml.numbering.definitionOf(numId) * 16 + level;
    if (levelIsOutlined(l, outlineKey)) lf.setProperty(tp::BulletOutline, true);   // (the numbers of a level are drawn hollow as well as its bullets)
    if (l.legacy) lf.setProperty(tp::LegacyNumber, true);
    // What the level says of its marker's run (the rest is the paragraph mark's): bold and italic, and a color.
    if (l.rPr.bold || l.rPr.italic || l.rPr.color) {
        const QTextCharFormat run = charFormatOf(l.rPr, theme());
        if (l.rPr.bold) lf.setProperty(tp::BulletBold, run.fontWeight() >= QFont::Bold);
        if (l.rPr.italic) lf.setProperty(tp::BulletItalic, run.fontItalic());
        if (l.rPr.color && l.rPr.color->val == QLatin1String("auto")) lf.setProperty(tp::BulletColor, QStringLiteral("auto"));   // (the engine's word for it)
        else if (l.rPr.color && run.hasProperty(tp::ColorRefP)) lf.setProperty(tp::BulletColor, run.stringProperty(tp::ColorRefP));
    }
    const int first = start >= 0 ? start : m_wml.numbering.startOf(numId, level);
    if (l.numFmt == QLatin1String("bullet") || l.numFmt == QLatin1String("none")) {
        lf.setStyle(QTextListFormat::ListDisc);
        QString text = l.numFmt == QLatin1String("none") ? QStringLiteral(" ") : l.text;
        QString font = l.rPr.fonts ? (!l.rPr.fonts->ascii.isEmpty() ? l.rPr.fonts->ascii : l.rPr.fonts->hAnsi) : QString();
        if (text.size() == 1 && text[0].unicode() >= 0xF000 && text[0].unicode() <= 0xF0FF) text = QString(QChar(text[0].unicode() - 0xF000));
        else if (text.size() == 1 && text[0].unicode() < 0x80 && !font.isEmpty() && font.compare(QLatin1String("Courier New"), Qt::CaseInsensitive) != 0 && !font.contains(QLatin1String("ymbol")) && !font.contains(QLatin1String("ingdings")))
            font.clear();
        if (!text.isEmpty()) lf.setProperty(tp::BulletChar, text);
        if (!font.isEmpty()) lf.setProperty(tp::BulletFont, font);
        if (l.rPr.size) lf.setProperty(tp::BulletSize, *l.rPr.size / 2.0);
        return lf;
    }
    QTextListFormat::Style style = QTextListFormat::ListDecimal;
    if (l.numFmt == QLatin1String("lowerLetter")) style = QTextListFormat::ListLowerAlpha;
    else if (l.numFmt == QLatin1String("upperLetter")) style = QTextListFormat::ListUpperAlpha;
    else if (l.numFmt == QLatin1String("lowerRoman")) style = QTextListFormat::ListLowerRoman;
    else if (l.numFmt == QLatin1String("upperRoman")) style = QTextListFormat::ListUpperRoman;
    lf.setStyle(style);
    if (l.rPr.size) lf.setProperty(tp::BulletSize, *l.rPr.size / 2.0);
    if (l.rPr.fonts) {
        const QString family = !l.rPr.fonts->ascii.isEmpty() ? l.rPr.fonts->ascii : l.rPr.fonts->hAnsi;
        if (!family.isEmpty()) lf.setProperty(tp::BulletFont, family);
    }
    // The text around the number: "%1." is the number and a period.
    const QString own = QStringLiteral("%%1").arg(level + 1);
    const int at = l.text.indexOf(own);
    if (at >= 0) {
        lf.setNumberPrefix(l.text.left(at));
        lf.setNumberSuffix(l.text.mid(at + own.size()));
    } else {
        lf.setNumberSuffix(QStringLiteral("."));
    }
    lf.setStart(std::max(1, first));
    return lf;
}

void WriteDoc::renumberLists(QTextDocument *story)
{
    const Numbering &numbering = m_wml.numbering;
    if (!story || numbering.nums.isEmpty()) return;
    // A level's counter with the restarts it has seen: an item of a shallower level restarts every deeper counter of its definition, which is one more count of restarts of that
    // level (so no counter is visited to restart it).
    struct Counter {
        int number = -1;
        quint32 restarts = 0;
    };
    struct Count {
        quint32 restarts[9] = {};
        Counter shared[9];                          // the definition's own count, which the numbering ids that restart no level go on with
        QHash<int, std::array<Counter, 9>> own;     // the counts of the ids that restart a level, for that level
    };
    QHash<int, Count> counts, endCounts;               // (the endnotes the pages set after the text count on their own)
    QHash<QTextList *, int> before;                 // the items of each Qt list seen so far (Qt's own count of the next)
    struct Change {
        int position;
        QString marker;
    };
    QVector<Change> changes;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        QTextList *list = b.textList();
        if (!list) continue;
        const int qtIndex = before[list]++;
        const QTextBlockFormat bf = b.blockFormat();
        if (!bf.hasProperty(prop::NumId)) continue;
        if (StoryLayout::joinsNext(b)) continue;   // (a paragraph whose mark is hidden is one with the next, which is the item: the two count once)
        const int numId = bf.intProperty(prop::NumId), level = std::clamp(bf.intProperty(prop::NumLevel), 0, 8);
        const Level *l = numbering.levelOf(numId, level);
        if (!l) continue;
        Count &count = (bf.hasProperty(prop::EndnoteBlock) ? endCounts : counts)[numbering.definitionOf(numId)];
        for (int i = level + 1; i < 9; ++i) ++count.restarts[i];
        auto counterOf = [&](int k) -> Counter & {
            Counter &c = numbering.restartsLevel(numId, k) ? count.own[numId][k] : count.shared[k];
            if (c.restarts != count.restarts[k]) {
                c.number = -1;
                c.restarts = count.restarts[k];
            }
            return c;
        };
        int n = 0;
        {
            Counter &c = counterOf(level);
            c.number = c.number < 0 ? numbering.startOf(numId, level) : c.number + 1;
            n = c.number;
        }
        if (l->numFmt == QLatin1String("bullet") || l->numFmt == QLatin1String("none")) continue;
        // A level above the item's that no item of the list has been at has begun its count with this item: it stands at its start, and the next item at that level is one more
        // (a list that goes from 2.4 to 2.4.1.1 has its next item at the third level numbered 2.4.2).
        for (int k = 0; k < level; ++k) {
            Counter &above = counterOf(k);
            if (above.number < 0) above.number = numbering.startOf(numId, k);
        }
        // Qt can count a level with the one number of its own in the marker, in a style it has, and that is the number it would count.
        const bool simple = !l->legal && l->text.count(QLatin1Char('%')) <= 1 && l->text.contains(QStringLiteral("%%1").arg(level + 1));
        QString want;
        if (!simple || list->format().start() + qtIndex != n) {
            want = l->text;
            for (int k = 8; k >= 0; --k) {
                const QString token = QStringLiteral("%%1").arg(k + 1);
                if (!want.contains(token)) continue;
                const Level *lk = numbering.levelOf(numId, k);
                const int number = counterOf(k).number;
                want.replace(token, listNumberText(l->legal || !lk ? QStringLiteral("decimal") : lk->numFmt, number < 0 ? (lk ? numbering.startOf(numId, k) : 1) : number));
            }
        }
        if (bf.stringProperty(tp::BulletChar) != want) changes.push_back({b.position(), want});
    }
    if (changes.isEmpty()) return;
    QTextCursor edit(story);
    edit.joinPreviousEditBlock();
    for (const Change &change : std::as_const(changes)) {
        edit.setPosition(change.position);
        QTextBlockFormat f = edit.blockFormat();
        if (change.marker.isEmpty()) f.clearProperty(tp::BulletChar);
        else f.setProperty(tp::BulletChar, change.marker);
        edit.setBlockFormat(f);
    }
    edit.endEditBlock();
}

SectionProps WriteDoc::lastSectionProps() const { return m_last; }

} // namespace jp::write
