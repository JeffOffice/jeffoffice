#pragma once
// Where a connector attaches to an object (the editors that glue connectors to shapes: Pub and Deck). A shape with connection sites of its own (core/connsites.h) has them all,
// counted as the standard counts them; any other object has the middle of each side of its frame (0 top, 1 left, 2 bottom, 3 right).

#include "core/items.h"

namespace jp {

// How many sites the object has.
int connectionSiteCount(const Item &it);
// A site's place in page coordinates, turned and flipped with the object. `vertical` says whether a line leaves it upright.
QPointF connectionSite(const Item &it, int site, bool *vertical = nullptr);
// The site on a side of the object (0 top, 1 left, 2 bottom, 3 right): the one that leaves that way and is nearest the side's middle; -1 for none.
int connectionSiteOnSide(const Item &it, int side);
// The side (0 top, 1 left, 2 bottom, 3 right) a site leaves by; -1 for a site that does not exist.
int sideOfConnectionSite(const Item &it, int site);

} // namespace jp
