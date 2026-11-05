#include "core/headerops.h"

#include "core/editops.h"
#include "core/fieldops.h"
#include "core/rulerops.h"

#include <QCoreApplication>
#include <QTextBlock>
#include <QTextCursor>
#include <algorithm>

namespace jp::write {

QVector<HeaderDesign> headerFooterDesigns()
{
    auto name = [](const char *id, const char *label) { return HeaderDesign{QString::fromLatin1(id), QCoreApplication::translate("jp::write::HeaderDesigns", label)}; };
    return {name("blank", QT_TRANSLATE_NOOP("jp::write::HeaderDesigns", "Blank")),
            name("pageRight", QT_TRANSLATE_NOOP("jp::write::HeaderDesigns", "Page number at the right")),
            name("pageCenter", QT_TRANSLATE_NOOP("jp::write::HeaderDesigns", "Page number in the middle")),
            name("pageLeft", QT_TRANSLATE_NOOP("jp::write::HeaderDesigns", "Page number at the left")),
            name("titleAndPage", QT_TRANSLATE_NOOP("jp::write::HeaderDesigns", "Title and page number")),
            name("dateAndPage", QT_TRANSLATE_NOOP("jp::write::HeaderDesigns", "Date and page number")),
            name("threeColumns", QT_TRANSLATE_NOOP("jp::write::HeaderDesigns", "Three columns: date, title, page number"))};
}

bool applyHeaderFooterDesign(WriteDoc &doc, int section, bool footer, int slot, const QString &id)
{
    bool known = false;
    for (const HeaderDesign &d : headerFooterDesigns()) known = known || d.id == id;
    if (!known || section < 0 || section >= doc.sectionCount()) return false;
    if (!doc.createHeaderFooter(section, footer, slot)) return false;
    QTextDocument *story = doc.editableHeaderFooter(section, footer, slot);
    if (!story) return false;
    const office::wml::SectionProps sp = doc.sectionProps(section);
    const double width = std::max(72.0, (sp.pageW - sp.left - sp.right - sp.gutter) / 20.0);   // the text's width, in points
    const WriteDoc::StepGroup group(doc);
    QTextCursor c(story);
    c.select(QTextCursor::Document);
    c.removeSelectedText();
    c.movePosition(QTextCursor::Start);
    // What the paragraph had of its own alignment and tab stops goes; its style stays.
    QTextBlockFormat bf = c.blockFormat();
    bf.setAlignment(Qt::AlignLeft);
    setTabStopsOf(bf, {});
    if (id == QLatin1String("pageRight")) bf.setAlignment(Qt::AlignRight);
    else if (id == QLatin1String("pageCenter")) bf.setAlignment(Qt::AlignHCenter);
    c.setBlockFormat(bf);
    const QString date = QStringLiteral("DATE \\@ \"MMMM d, yyyy\"");
    if (id == QLatin1String("pageRight") || id == QLatin1String("pageCenter") || id == QLatin1String("pageLeft")) {
        insertPageNumber(doc, c);
    } else if (id == QLatin1String("titleAndPage") || id == QLatin1String("dateAndPage")) {
        insertField(doc, c, id == QLatin1String("titleAndPage") ? QStringLiteral("TITLE") : date);
        insertText(doc, c, QStringLiteral("\t"));
        insertPageNumber(doc, c);
        setTabStops(doc, c, {TabStopEntry{width, QTextOption::RightTab, QLatin1Char(' ')}});
    } else if (id == QLatin1String("threeColumns")) {
        insertField(doc, c, date);
        insertText(doc, c, QStringLiteral("\t"));
        insertField(doc, c, QStringLiteral("TITLE"));
        insertText(doc, c, QStringLiteral("\t"));
        insertPageNumber(doc, c);
        setTabStops(doc, c, {TabStopEntry{width / 2, QTextOption::CenterTab, QLatin1Char(' ')}, TabStopEntry{width, QTextOption::RightTab, QLatin1Char(' ')}});
    }
    return true;
}

bool insertDocumentInfo(WriteDoc &doc, QTextCursor &c, const QString &kind)
{
    static const QHash<QString, QString> codes = {{QStringLiteral("author"), QStringLiteral("AUTHOR")}, {QStringLiteral("title"), QStringLiteral("TITLE")},
                                                  {QStringLiteral("fileName"), QStringLiteral("FILENAME")}, {QStringLiteral("filePath"), QStringLiteral("FILENAME \\p")}};
    const auto it = codes.constFind(kind);
    return it != codes.constEnd() && insertField(doc, c, it.value());
}

} // namespace jp::write
