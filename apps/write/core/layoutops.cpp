#include "core/layoutops.h"

#include "wml/wml.h"

#include <QRegularExpression>
#include <algorithm>
#include <cmath>

namespace jp::write {

using office::wml::ColumnDef;
using office::wml::RawChild;
using office::wml::SectionProps;

namespace {

// Takes the section's kept element of this name out (it is what the new one replaces).
void dropExtra(SectionProps *props, const char *name)
{
    const QRegularExpression head(QStringLiteral("^<(?:[\\w.-]+:)?%1\\b").arg(QLatin1String(name)));
    for (qsizetype i = props->extra.size() - 1; i >= 0; --i)
        if (head.match(QString::fromUtf8(props->extra[i].xml.left(64))).hasMatch()) props->extra.removeAt(i);
}

// The attributes of a kept element of this name, by local name (empty where there is none).
QHash<QString, QString> extraAttributes(const SectionProps &props, const char *name, QString *xml = nullptr)
{
    const QRegularExpression head(QStringLiteral("^<(?:[\\w.-]+:)?%1\\b([^>]*)>").arg(QLatin1String(name)));
    static const QRegularExpression attribute(QStringLiteral("\\s(?!xmlns)(?:[\\w.-]+:)?([\\w.-]+)\\s*=\\s*\"([^\"]*)\""));
    for (const RawChild &c : props.extra) {
        const QString text = QString::fromUtf8(c.xml);
        const QRegularExpressionMatch m = head.match(text.left(2000));
        if (!m.hasMatch()) continue;
        if (xml) *xml = text;
        QHash<QString, QString> at;
        auto it = attribute.globalMatch(QLatin1Char(' ') + m.captured(1));
        while (it.hasNext()) {
            const QRegularExpressionMatch a = it.next();
            at.insert(a.captured(1), a.captured(2));
        }
        return at;
    }
    return {};
}

int twips(double points) { return int(std::lround(points * 20)); }

} // namespace

ColumnSetup columnSetupOf(const SectionProps &p)
{
    ColumnSetup s;
    s.count = std::clamp(p.cols, 1, kMostColumnsSet);
    s.space = p.colSpace / 20.0;
    s.line = p.colSep;
    if (!p.colEqual && int(p.colDefs.size()) == p.cols && p.cols > 1) {
        s.equal = false;
        for (const ColumnDef &c : p.colDefs) {
            s.widths << c.width / 20.0;
            s.spaces << c.space / 20.0;
        }
    }
    return s;
}

ColumnSetup normalized(ColumnSetup s)
{
    s.count = std::clamp(s.count, 1, kMostColumnsSet);
    s.space = std::isfinite(s.space) ? std::clamp(s.space, 0.0, 1584.0) : 36.0;
    if (s.equal || s.count == 1 || int(s.widths.size()) != s.count) {
        s.equal = true;
        s.widths.clear();
        s.spaces.clear();
    } else {
        s.spaces.resize(s.count);
        for (double &w : s.widths) w = std::isfinite(w) ? std::clamp(w, 1.0, 1584.0) : 36.0;
        for (double &w : s.spaces) w = std::isfinite(w) ? std::clamp(w, 0.0, 1584.0) : 0.0;
    }
    return s;
}

void setColumnSetup(SectionProps *p, const ColumnSetup &in)
{
    const ColumnSetup s = normalized(in);
    p->cols = s.count;
    p->colSpace = twips(s.space);
    p->colSep = s.line && s.count > 1;
    p->colDefs.clear();
    p->colEqual = true;
    if (!s.equal) {
        p->colEqual = false;
        for (int i = 0; i < s.count; ++i) p->colDefs.push_back({std::max(1, twips(s.widths[i])), i + 1 < s.count ? twips(s.spaces[i]) : 0});
    }
    p->hasCols = true;
}

LineNumberSetup lineNumbersOf(const SectionProps &p)
{
    LineNumberSetup s;
    QString xml;
    const QHash<QString, QString> at = extraAttributes(p, "lnNumType", &xml);
    if (xml.isEmpty()) return s;
    s.countBy = std::clamp(at.value(QStringLiteral("countBy"), QStringLiteral("1")).toInt(), 1, 1000);
    s.start = std::clamp(at.value(QStringLiteral("start")).toInt(), 0, 32767) + 1;
    s.distance = std::clamp(at.value(QStringLiteral("distance")).toDouble() / 20.0, 0.0, 144.0);
    const QString restart = at.value(QStringLiteral("restart"));
    s.restart = restart == QLatin1String("newSection") ? LineNumberSetup::Restart::EachSection : restart == QLatin1String("continuous") ? LineNumberSetup::Restart::Continuous : LineNumberSetup::Restart::EachPage;
    return s;
}

void setLineNumbers(SectionProps *p, const LineNumberSetup &s)
{
    dropExtra(p, "lnNumType");
    if (s.countBy <= 0) return;
    QByteArray e = "<w:lnNumType w:countBy=\"" + QByteArray::number(std::clamp(s.countBy, 1, 1000)) + "\"";
    if (s.start != 1) e += " w:start=\"" + QByteArray::number(std::clamp(s.start, 1, 32768) - 1) + "\"";
    if (s.distance > 0) e += " w:distance=\"" + QByteArray::number(twips(std::clamp(s.distance, 0.0, 144.0))) + "\"";
    e += s.restart == LineNumberSetup::Restart::EachSection ? " w:restart=\"newSection\"" : s.restart == LineNumberSetup::Restart::Continuous ? " w:restart=\"continuous\"" : " w:restart=\"newPage\"";
    e += "/>";
    p->extra.push_back({office::wml::sectionChildRank("lnNumType"), e});
    std::stable_sort(p->extra.begin(), p->extra.end(), [](const RawChild &a, const RawChild &b) { return a.rank < b.rank; });
}

PageBorderSetup pageBordersOf(const SectionProps &p)
{
    PageBorderSetup s;
    QString xml;
    const QHash<QString, QString> root = extraAttributes(p, "pgBorders", &xml);
    if (xml.isEmpty()) return s;
    s.fromPage = root.value(QStringLiteral("offsetFrom")) == QLatin1String("page");
    s.display = root.value(QStringLiteral("display"), QStringLiteral("all"));
    static const QRegularExpression side(QStringLiteral("<(?:[\\w.-]+:)?(top|left|bottom|right)\\b([^>]*?)/?>"));
    static const QRegularExpression attribute(QStringLiteral("\\s(?!xmlns)(?:[\\w.-]+:)?([\\w.-]+)\\s*=\\s*\"([^\"]*)\""));
    bool first = true;
    auto it = side.globalMatch(xml);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        QHash<QString, QString> at;
        auto a = attribute.globalMatch(QLatin1Char(' ') + m.captured(2));
        while (a.hasNext()) {
            const QRegularExpressionMatch x = a.next();
            at.insert(x.captured(1), x.captured(2));
        }
        const QString val = at.value(QStringLiteral("val"));
        if (val.isEmpty() || val == QLatin1String("nil") || val == QLatin1String("none")) continue;
        const QString name = m.captured(1);
        (name == QLatin1String("top") ? s.top : name == QLatin1String("left") ? s.left : name == QLatin1String("bottom") ? s.bottom : s.right) = true;
        if (first) {
            first = false;
            s.style = val;
            s.width = std::clamp(at.value(QStringLiteral("sz"), QStringLiteral("4")).toDouble() / 8.0, 0.25, 12.0);
            s.space = std::clamp(at.value(QStringLiteral("space")).toDouble(), 0.0, 31.0);
            const QString color = at.value(QStringLiteral("color"));
            if (color.size() == 6 && QColor(QLatin1Char('#') + color).isValid()) s.color = QColor(QLatin1Char('#') + color);
        }
    }
    return s;
}

void setPageBorders(SectionProps *p, const PageBorderSetup &s)
{
    dropExtra(p, "pgBorders");
    if (!s.any()) return;
    const QByteArray line = " w:val=\"" + s.style.toUtf8() + "\" w:sz=\"" + QByteArray::number(std::clamp(int(std::lround(s.width * 8)), 2, 96)) + "\" w:space=\""
                            + QByteArray::number(std::clamp(int(std::lround(s.space)), 0, 31)) + "\" w:color=\"" + s.color.name().mid(1).toUpper().toUtf8() + "\"";
    QByteArray e = "<w:pgBorders";
    if (s.fromPage) e += " w:offsetFrom=\"page\"";
    if (s.display != QLatin1String("all")) e += " w:display=\"" + s.display.toUtf8() + "\"";
    e += ">";
    if (s.top) e += "<w:top" + line + "/>";
    if (s.left) e += "<w:left" + line + "/>";
    if (s.bottom) e += "<w:bottom" + line + "/>";
    if (s.right) e += "<w:right" + line + "/>";
    e += "</w:pgBorders>";
    p->extra.push_back({office::wml::sectionChildRank("pgBorders"), e});
    std::stable_sort(p->extra.begin(), p->extra.end(), [](const RawChild &a, const RawChild &b) { return a.rank < b.rank; });
}

int applyToSections(WriteDoc &doc, int section, bool whole, const std::function<void(SectionProps *)> &change)
{
    const WriteDoc::StepGroup group(doc);
    int changed = 0;
    const int from = whole ? 0 : std::clamp(section, 0, doc.sectionCount() - 1), to = whole ? doc.sectionCount() : from + 1;
    for (int i = from; i < to; ++i) {
        const SectionProps before = doc.sectionProps(i);
        SectionProps after = before;
        change(&after);
        if (packSection(after) == packSection(before)) continue;
        doc.setSectionProps(i, after);
        ++changed;
    }
    return changed;
}

} // namespace jp::write
