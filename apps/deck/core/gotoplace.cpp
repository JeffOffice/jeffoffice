#include "core/gotoplace.h"

#include "app/pageselectionpane.h"
#include "core/comments.h"
#include "core/sections.h"

#include <QCoreApplication>
#include <algorithm>

namespace jp::deck {

namespace {

// The whole number in `text`, or -1.
int numberIn(const QString &text)
{
    bool ok = false;
    const int n = text.trimmed().toInt(&ok);
    return ok && n >= 1 ? n : -1;
}

GoToPlace section(const Document &d, const QString &text, int step, const GoToFrom &from, QString *why)
{
    const QVector<Section> list = sectionsOf(d);
    const int n = int(list.size());
    if (n == 0) {
        *why = QCoreApplication::translate("jp::deck::GoToPlace", "This presentation has no sections.");
        return {};
    }
    int at = -1;
    if (step != 0) {
        const int here = sectionOfSlide(d, from.slide);
        at = here + step;
        if (at < 0 || at >= n) {
            *why = step > 0 ? QCoreApplication::translate("jp::deck::GoToPlace", "There is no next section.") : QCoreApplication::translate("jp::deck::GoToPlace", "There is no previous section.");
            return {};
        }
    } else if (const int k = numberIn(text); k >= 1) {
        if (k > n) {
            *why = QCoreApplication::translate("jp::deck::GoToPlace", "There are only %1 sections.").arg(n);
            return {};
        }
        at = k - 1;
    } else {
        const QString want = text.trimmed();
        if (want.isEmpty()) {
            *why = QCoreApplication::translate("jp::deck::GoToPlace", "Type a section name or number.");
            return {};
        }
        for (int i = 0; i < n && at < 0; ++i)
            if (list[i].name.compare(want, Qt::CaseInsensitive) == 0) at = i;
        for (int i = 0; i < n && at < 0; ++i)
            if (list[i].name.contains(want, Qt::CaseInsensitive)) at = i;
        if (at < 0) {
            *why = QCoreApplication::translate("jp::deck::GoToPlace", "There is no section named \"%1\".").arg(want);
            return {};
        }
    }
    GoToPlace p;
    p.slide = std::clamp(firstSlideOf(d, at), 0, std::max(0, int(d.pages.size()) - 1));
    return p;
}

GoToPlace comment(const Document &d, const QString &text, int step, const GoToFrom &from, QString *why)
{
    const QVector<comments::CommentRef> all = comments::allComments(d);
    const int n = int(all.size());
    if (n == 0) {
        *why = QCoreApplication::translate("jp::deck::GoToPlace", "This presentation has no comments.");
        return {};
    }
    int target = -1;
    if (step == 0) {
        const int k = numberIn(text);
        if (k < 1 || k > n) {
            *why = k < 1 ? QCoreApplication::translate("jp::deck::GoToPlace", "Type a comment number.") : QCoreApplication::translate("jp::deck::GoToPlace", "There are only %1 comments.").arg(n);
            return {};
        }
        target = k - 1;
    } else {
        // From the open comment; with none open, the first one from this slide on (the last one before it, going back), round to the other end.
        for (int i = 0; i < n && target < 0; ++i)
            if (all[i].slide == from.slide && all[i].id == from.commentId) target = (i + (step < 0 ? -1 : 1) + n) % n;
        if (target < 0 && step > 0) {
            target = 0;
            for (int i = 0; i < n; ++i)
                if (all[i].slide >= from.slide) {
                    target = i;
                    break;
                }
        } else if (target < 0) {
            target = n - 1;
            for (int i = n - 1; i >= 0; --i)
                if (all[i].slide < from.slide) {
                    target = i;
                    break;
                }
        }
    }
    GoToPlace p;
    p.slide = all[target].slide;
    p.commentId = all[target].id;
    return p;
}

GoToPlace object(const Document &d, const QString &text, int step, const GoToFrom &from, QString *why)
{
    const int slides = int(d.pages.size());
    GoToPlace p;
    if (step != 0) {
        if (from.slide < 0 || from.slide >= slides || d.pages[size_t(from.slide)]->items.empty()) {
            *why = QCoreApplication::translate("jp::deck::GoToPlace", "This slide has no objects.");
            return {};
        }
        const ItemList &items = d.pages[size_t(from.slide)]->items;
        const int n = int(items.size());
        int here = -1;
        for (int i = 0; i < n && here < 0; ++i)
            if (items[size_t(i)]->id == from.itemId) here = i;
        const int to = here < 0 ? (step > 0 ? 0 : n - 1) : (here + step + n) % n;
        p.slide = from.slide;
        p.itemId = items[size_t(to)]->id;
        return p;
    }
    const QString want = text.trimmed();
    if (want.isEmpty()) {
        *why = QCoreApplication::translate("jp::deck::GoToPlace", "Type an object name.");
        return {};
    }
    // One pass from the slide being edited, round to the start: the first whole name that matches, else the first that holds the text.
    for (int k = 0, partialSlide = -1; k < slides; ++k) {
        const int s = (std::clamp(from.slide, 0, slides - 1) + k) % slides;
        const ItemList &items = d.pages[size_t(s)]->items;
        const int n = int(items.size());
        for (int i = 0; i < n; ++i) {
            const Item &it = *items[size_t(i)];
            // (The name the Selection pane shows for an object nobody named is its kind and its place in the stack.)
            const QString name = it.name.isEmpty() ? defaultObjectName(it, i + 1) : it.name;
            if (name.compare(want, Qt::CaseInsensitive) == 0) {
                p.slide = s;
                p.itemId = it.id;
                return p;
            }
            if (partialSlide < 0 && name.contains(want, Qt::CaseInsensitive)) {
                partialSlide = s;
                p.slide = s;
                p.itemId = it.id;
            }
        }
        if (k == slides - 1 && p.slide >= 0) return p;
    }
    *why = QCoreApplication::translate("jp::deck::GoToPlace", "There is no object named \"%1\".").arg(want);
    return {};
}

} // namespace

GoToPlace goToPlace(const Document &d, GoToKind kind, const QString &text, int step, const GoToFrom &from, QString *why)
{
    QString local;
    if (!why) why = &local;
    why->clear();
    if (d.pages.empty()) {
        *why = QCoreApplication::translate("jp::deck::GoToPlace", "This presentation has no slides.");
        return {};
    }
    switch (kind) {
    case GoToKind::Section: return section(d, text, step, from, why);
    case GoToKind::Comment: return comment(d, text, step, from, why);
    case GoToKind::Object: return object(d, text, step, from, why);
    case GoToKind::Slide: {
        const int n = int(d.pages.size());
        GoToPlace p;
        if (step != 0) {
            p.slide = from.slide + step;
            if (p.slide < 0 || p.slide >= n) {
                p.slide = -1;
                *why = step > 0 ? QCoreApplication::translate("jp::deck::GoToPlace", "There is no next slide.") : QCoreApplication::translate("jp::deck::GoToPlace", "There is no previous slide.");
            }
        } else if (const int k = numberIn(text); k >= 1 && k <= n) {
            p.slide = k - 1;
        } else {
            *why = QCoreApplication::translate("jp::deck::GoToPlace", "There are only %1 slides.").arg(n);
        }
        return p;
    }
    }
    return {};
}

} // namespace jp::deck
