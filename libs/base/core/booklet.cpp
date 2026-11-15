#include "core/booklet.h"

#include <algorithm>

namespace jp {

QVector<QVector<int>> bookletOrder(int pages)
{
    const int n = ((pages + 3) / 4) * 4;
    QVector<QVector<int>> sides;
    for (int s = 0; s < n / 2; s += 2) {
        auto pg = [&](int i) { return i < pages ? i : -1; };
        sides << QVector<int>{pg(n - 1 - s), pg(s)};
        sides << QVector<int>{pg(s + 1), pg(n - 2 - s)};
    }
    return sides;
}

QVector<QVector<int>> bookletOrder(int pages, int pagesPerBooklet)
{
    const int per = pagesPerBooklet <= 0 ? 0 : ((pagesPerBooklet + 3) / 4) * 4;
    if (per == 0 || per >= pages) return bookletOrder(pages);
    QVector<QVector<int>> sides;
    for (int first = 0; first < pages; first += per) {
        for (const QVector<int> &side : bookletOrder(std::min(per, pages - first))) {
            QVector<int> shifted;
            for (int page : side) shifted << (page < 0 ? -1 : page + first);
            sides << shifted;
        }
    }
    return sides;
}

} // namespace jp
