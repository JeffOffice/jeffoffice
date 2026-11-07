#pragma once
// The id for a new item among the ids a file gave its other items. A file's ids are whatever its writer put, so one past the greatest may be past what an int holds.

#include <algorithm>
#include <climits>
#include <vector>

namespace jp::office {

// One past the greatest id; if the greatest is the last an int has, the smallest id above 0 that none has instead. (Ids of 0 or less are not counted.) 0 if all are taken.
inline int nextFreeId(std::vector<int> ids)
{
    int most = 0;
    for (const int id : ids) most = std::max(most, id);
    if (most < INT_MAX) return most + 1;
    std::sort(ids.begin(), ids.end());
    int candidate = 1;
    for (const int id : ids) {
        if (id < candidate) continue;
        if (id > candidate) break;
        if (candidate == INT_MAX) return 0;
        ++candidate;
    }
    return candidate;
}

} // namespace jp::office
