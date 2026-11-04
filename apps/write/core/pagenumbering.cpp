#include "core/pagenumbering.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QTextCursor>
#include <algorithm>

namespace jp::write {

using office::wml::SectionProps;

namespace {

const QRegularExpression &chapterAttributes()
{
    static const QRegularExpression re(QStringLiteral("\\s+w:chap(?:Style|Sep)\\s*=\\s*\"[^\"]*\""));
    return re;
}

} // namespace

PageNumbering pageNumberingOf(const SectionProps &p)
{
    PageNumbering n;
    n.format = p.pageFormat.isEmpty() ? QStringLiteral("decimal") : p.pageFormat;
    n.continueFromPrevious = !p.pageStart.has_value();
    n.startAt = p.pageStart.value_or(1);
    static const QRegularExpression style(QStringLiteral("[\\s:]chapStyle\\s*=\\s*\"([0-9]+)\"")), separator(QStringLiteral("[\\s:]chapSep\\s*=\\s*\"(\\w+)\""));
    const QString attrs = QString::fromUtf8(p.pgNumAttrs.left(2000));
    if (const QRegularExpressionMatch m = style.match(attrs); m.hasMatch()) n.chapterStyle = std::clamp(m.captured(1).left(3).toInt(), 0, 9);
    if (const QRegularExpressionMatch m = separator.match(attrs); m.hasMatch()) n.chapterSeparator = m.captured(1);
    return n;
}

QVector<QPair<QString, QString>> pageNumberFormats()
{
    auto t = [](const char *s) { return QCoreApplication::translate("jp::write::PageNumbering", s); };
    return {{QStringLiteral("decimal"), t("1, 2, 3, …")},     {QStringLiteral("lowerLetter"), t("a, b, c, …")}, {QStringLiteral("upperLetter"), t("A, B, C, …")},
            {QStringLiteral("lowerRoman"), t("i, ii, iii, …")}, {QStringLiteral("upperRoman"), t("I, II, III, …")}, {QStringLiteral("numberInDash"), t("- 1 -, - 2 -, - 3 -, …")}};
}

int applyPageNumbering(WriteDoc &doc, int section, bool wholeDocument, const PageNumbering &numbering)
{
    static const QStringList separators = {QStringLiteral("hyphen"), QStringLiteral("period"), QStringLiteral("colon"), QStringLiteral("emDash"), QStringLiteral("enDash")};
    const QString format = numbering.format == QLatin1String("decimal") ? QString() : numbering.format;
    const int chapter = std::clamp(numbering.chapterStyle, 0, 9);
    const QString separator = separators.contains(numbering.chapterSeparator) ? numbering.chapterSeparator : QStringLiteral("hyphen");
    const int start = std::clamp(numbering.startAt, 0, 100000);
    QTextCursor step(doc.story());
    step.beginEditBlock();
    const int from = wholeDocument ? 0 : section, to = wholeDocument ? doc.sectionCount() - 1 : section;
    const int changed = doc.changeSections(from, to, [&](SectionProps &p) {
        const SectionProps was = p;
        p.pageFormat = format;
        if (numbering.continueFromPrevious) p.pageStart.reset();
        else p.pageStart = start;
        QString attrs = QString::fromUtf8(p.pgNumAttrs);
        attrs.remove(chapterAttributes());
        if (chapter > 0) attrs += QStringLiteral(" w:chapStyle=\"%1\" w:chapSep=\"%2\"").arg(chapter).arg(separator);
        p.pgNumAttrs = attrs.toUtf8();
        return p.pageFormat != was.pageFormat || p.pageStart != was.pageStart || p.pgNumAttrs != was.pgNumAttrs;
    });
    step.endEditBlock();
    return changed;
}

} // namespace jp::write
