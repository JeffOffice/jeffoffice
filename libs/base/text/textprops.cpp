#include "text/textprops.h"

namespace jp::tp {

RevisionMarkup &revisionMarkup()
{
    static RevisionMarkup markup;
    return markup;
}

QHash<int, QSet<int>> &shownFieldCodeSets()
{
    static QHash<int, QSet<int>> sets;
    return sets;
}

}   // namespace jp::tp
