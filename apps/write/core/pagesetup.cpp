#include "core/pagesetup.h"

#include <QTextCursor>
#include <algorithm>
#include <cmath>

namespace jp::write {

using office::wml::SectionProps;

namespace {

constexpr double kSame = 0.04;   // points: a change smaller than this is none (a twentieth of a point is the file's own step)

bool differs(double a, double b) { return std::abs(a - b) > kSame; }

double number(double v, double fallback, double least, double most)
{
    if (!std::isfinite(v)) v = fallback;
    return std::clamp(v, least, most);
}

int twips(double points) { return int(std::lround(points * 20)); }

// A section's margins made to leave the least text on its page: the sides of a margin scaled down together.
void fitMargins(SectionProps *p)
{
    auto fit = [](int *a, int *b, int *extra, int page) {
        const int room = std::max(0, page - twips(kLeastText));
        const qint64 sum = qint64(std::max(0, *a)) + std::max(0, *b) + std::max(0, *extra);
        if (sum <= room || sum <= 0) return;
        const double k = double(room) / double(sum);
        *a = int(std::max(0, *a) * k);
        *b = int(std::max(0, *b) * k);
        *extra = int(std::max(0, *extra) * k);
    };
    int none = 0;
    fit(&p->left, &p->right, &p->gutter, p->pageW);
    fit(&p->top, &p->bottom, &none, p->pageH);
    p->header = std::clamp(p->header, 0, p->pageH / 2);
    p->footer = std::clamp(p->footer, 0, p->pageH / 2);
}

} // namespace

bool SectionSetup::operator==(const SectionSetup &o) const
{
    return !differs(paperWidth, o.paperWidth) && !differs(paperHeight, o.paperHeight) && landscape == o.landscape && !differs(top, o.top) && !differs(bottom, o.bottom) && !differs(left, o.left) &&
           !differs(right, o.right) && !differs(gutter, o.gutter) && !differs(header, o.header) && !differs(footer, o.footer) && start == o.start && differentFirstPage == o.differentFirstPage &&
           differentOddEven == o.differentOddEven;
}

SectionSetup pageSetupOf(const SectionProps &p, bool oddEven)
{
    SectionSetup s;
    const double w = p.pageW / 20.0, h = p.pageH / 20.0;
    s.landscape = w > h;
    s.paperWidth = s.landscape ? h : w;
    s.paperHeight = s.landscape ? w : h;
    s.top = std::abs(p.top) / 20.0;
    s.bottom = std::abs(p.bottom) / 20.0;
    s.left = std::max(0, p.left) / 20.0;
    s.right = std::max(0, p.right) / 20.0;
    s.gutter = std::max(0, p.gutter) / 20.0;
    s.header = std::max(0, p.header) / 20.0;
    s.footer = std::max(0, p.footer) / 20.0;
    s.start = p.type == QLatin1String("continuous") || p.type == QLatin1String("oddPage") || p.type == QLatin1String("evenPage") ? p.type : QStringLiteral("nextPage");
    s.differentFirstPage = p.titlePg;
    s.differentOddEven = oddEven;
    return s;
}

SectionSetup normalized(SectionSetup s)
{
    const SectionSetup defaults;
    s.paperWidth = number(s.paperWidth, defaults.paperWidth, kLeastPaper, kMostPaper);
    s.paperHeight = number(s.paperHeight, defaults.paperHeight, kLeastPaper, kMostPaper);
    const double w = s.landscape ? s.paperHeight : s.paperWidth, h = s.landscape ? s.paperWidth : s.paperHeight;
    auto margin = [](double v, double fallback, double most) { return number(v, fallback, 0, std::max(0.0, most)); };
    const double acrossRoom = std::max(0.0, w - kLeastText), downRoom = std::max(0.0, h - kLeastText);
    s.gutter = margin(s.gutter, 0, acrossRoom);
    s.left = margin(s.left, defaults.left, acrossRoom - s.gutter);
    s.right = margin(s.right, defaults.right, acrossRoom - s.gutter - s.left);
    s.top = margin(s.top, defaults.top, downRoom);
    s.bottom = margin(s.bottom, defaults.bottom, downRoom - s.top);
    s.header = margin(s.header, defaults.header, h / 2);
    s.footer = margin(s.footer, defaults.footer, h / 2);
    if (s.start != QLatin1String("continuous") && s.start != QLatin1String("oddPage") && s.start != QLatin1String("evenPage")) s.start = QStringLiteral("nextPage");
    return s;
}

int applyPageSetup(WriteDoc &doc, int section, PageSetupScope scope, const SectionSetup &before, const SectionSetup &afterAsGiven)
{
    const SectionSetup after = normalized(afterAsGiven);
    const bool paperChanged = differs(after.paperWidth, before.paperWidth) || differs(after.paperHeight, before.paperHeight) || after.landscape != before.landscape;
    int changed = 0;
    QTextCursor step(doc.story());
    step.beginEditBlock();
    const int from = scope == PageSetupScope::WholeDocument ? 0 : section, to = scope == PageSetupScope::WholeDocument ? doc.sectionCount() - 1 : section;
    changed += doc.changeSections(from, to, [&](SectionProps &p) {
        const SectionProps was = p;
        if (paperChanged) {
            // A section keeps the way it is turned unless the person turned the page.
            const bool landscape = after.landscape != before.landscape ? after.landscape : was.pageW > was.pageH;
            const double w = landscape ? after.paperHeight : after.paperWidth, h = landscape ? after.paperWidth : after.paperHeight;
            p.pageW = twips(w);
            p.pageH = twips(h);
            p.orient = landscape ? QStringLiteral("landscape") : QStringLiteral("portrait");
            p.hasPgSz = true;
        }
        bool margins = false;
        auto setMargin = [&](int *field, double now, double old) {
            if (!differs(now, old)) return;
            *field = twips(now);
            margins = true;
        };
        setMargin(&p.top, after.top, before.top);
        setMargin(&p.bottom, after.bottom, before.bottom);
        setMargin(&p.left, after.left, before.left);
        setMargin(&p.right, after.right, before.right);
        setMargin(&p.gutter, after.gutter, before.gutter);
        if (margins) p.hasPgMar = true;
        if (differs(after.header, before.header)) p.header = twips(after.header);
        if (differs(after.footer, before.footer)) p.footer = twips(after.footer);
        if (after.start != before.start) p.type = after.start == QLatin1String("nextPage") ? QString() : after.start;
        if (after.differentFirstPage != before.differentFirstPage) p.titlePg = after.differentFirstPage;
        if (paperChanged || margins) fitMargins(&p);
        return p.pageW != was.pageW || p.pageH != was.pageH || p.orient != was.orient || p.top != was.top || p.bottom != was.bottom || p.left != was.left || p.right != was.right ||
               p.gutter != was.gutter || p.header != was.header || p.footer != was.footer || p.type != was.type || p.titlePg != was.titlePg || p.hasPgSz != was.hasPgSz || p.hasPgMar != was.hasPgMar;
    });
    if (after.differentOddEven != before.differentOddEven && doc.differentOddEven() != after.differentOddEven) {
        doc.setDifferentOddEven(after.differentOddEven);
        ++changed;
    }
    step.endEditBlock();
    return changed;
}

} // namespace jp::write
