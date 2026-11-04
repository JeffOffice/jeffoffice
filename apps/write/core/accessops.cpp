#include "core/accessops.h"

#include "core/editops.h"
#include "core/pictureops.h"
#include "core/tableops.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "core/color.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QSet>
#include <QTextBlock>
#include <cmath>

namespace jp::write {

namespace {

double luminance(const QColor &c)
{
    auto channel = [](double v) { return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
    return 0.2126 * channel(c.redF()) + 0.7152 * channel(c.greenF()) + 0.0722 * channel(c.blueF());
}

} // namespace

double contrastRatio(const QColor &a, const QColor &b)
{
    const double la = luminance(a), lb = luminance(b);
    return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

QVector<AccessIssue> checkAccessibility(WriteDoc &doc, int most)
{
    QVector<AccessIssue> out;
    QTextDocument *story = doc.story();
    const ColorScheme &colors = doc.flow().env.colors;
    static const QRegularExpression headingId(QStringLiteral("^Heading([1-9])$"));
    int lastHeading = 0;
    QSet<int> cellsSeen;
    auto add = [&](AccessIssue::Kind kind, AccessIssue::Level level, int position, const QString &message) {
        if (out.size() < most) out.push_back({kind, level, position, message});
    };
    for (QTextBlock b = story->begin(); b.isValid() && out.size() < most; b = b.next()) {
        if (isLocked(b)) continue;
        const int key = cellKeyOf(b);
        if (key != 0 && !cellsSeen.contains(key)) {
            cellsSeen.insert(key);
            QTextCursor at(story);
            at.setPosition(b.position());
            const CellPlace place = cellPlace(doc, at);
            if (place.inTable) {
                if (place.row == 0 && place.column == 0 && !tablePropsAt(doc, at).header)
                    add(AccessIssue::TableWithoutHeader, AccessIssue::Error, b.position(), QCoreApplication::translate("jp::write::Access", "The table has no header row. Screen readers read the header row as the names of the columns."));
                // The cell is blank when all of its paragraphs are.
                bool blank = true;
                for (QTextBlock c = b; c.isValid() && cellKeyOf(c) == key; c = c.next())
                    if (!c.text().trimmed().isEmpty()) blank = false;
                if (blank) add(AccessIssue::BlankTableCell, AccessIssue::Warning, b.position(), QCoreApplication::translate("jp::write::Access", "A cell of the table is blank. A blank cell can make a reader think the table has ended."));
            }
        }
        // Headings in order: one that skips a level (Heading 3 after Heading 1) breaks the outline a reader moves by.
        const QString style = b.blockFormat().stringProperty(tp::StyleName);
        const QRegularExpressionMatch heading = headingId.match(style);
        if (heading.hasMatch() && !b.text().trimmed().isEmpty()) {
            const int level = heading.captured(1).toInt();
            if (level > lastHeading + 1)
                add(AccessIssue::HeadingOrder, AccessIssue::Warning, b.position(),
                    QCoreApplication::translate("jp::write::Access", "A Heading %1 follows a %2. Headings should go down one level at a time.").arg(level).arg(lastHeading == 0 ? QCoreApplication::translate("jp::write::Access", "start of the document") : QCoreApplication::translate("jp::write::Access", "Heading %1").arg(lastHeading)));
            lastHeading = level;
        }
        bool contrastTold = false;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const QTextCharFormat cf = f.charFormat();
            if (cf.hasProperty(prop::TrackedDeleted) || cf.boolProperty(tp::HiddenText)) continue;
            if (isAtom(cf)) {
                for (int k = 0; k < f.length(); ++k) {
                    const int pos = f.position() + k;
                    if (!isPictureAt(story, pos)) continue;
                    const PictureInfo pic = pictureAt(doc, story, pos);
                    if (!pic.valid) continue;
                    if (pic.alt.trimmed().isEmpty()) add(AccessIssue::MissingAltText, AccessIssue::Error, pos, QCoreApplication::translate("jp::write::Access", "A picture has no alt text. Alt text tells a person who cannot see it what the picture shows."));
                    if (pic.floating) add(AccessIssue::FloatingPicture, AccessIssue::Tip, pos, QCoreApplication::translate("jp::write::Access", "A picture floats on the page. A screen reader may skip it. Put it in line with the text."));
                }
                continue;
            }
            if (f.text().trimmed().isEmpty()) continue;
            // Colors: the letters against the highlight, else against the white page.
            QColor ink(Qt::black);
            if (cf.hasProperty(QTextFormat::ForegroundBrush) && cf.foreground().style() != Qt::NoBrush) ink = cf.foreground().color();
            QColor paper(Qt::white);
            const QString highlight = cf.stringProperty(tp::HighlightRefP);
            if (!highlight.isEmpty()) {
                const QColor h = ColorRef::fromString(highlight).resolve(colors);
                if (h.isValid()) paper = h;
            }
            const double size = cf.fontPointSize() > 0 ? cf.fontPointSize() : 11.0;
            const bool large = size >= 18.0 || (size >= 14.0 && cf.fontWeight() >= QFont::DemiBold);
            const double need = large ? 3.0 : 4.5;
            if (!contrastTold && contrastRatio(ink, paper) < need) {
                contrastTold = true;
                add(AccessIssue::LowContrast, AccessIssue::Warning, f.position(), QCoreApplication::translate("jp::write::Access", "Some text is hard to read: its color is too close to the color behind it."));
            }
        }
    }
    return out;
}

} // namespace jp::write
