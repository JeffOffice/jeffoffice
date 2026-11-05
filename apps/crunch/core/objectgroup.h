#pragma once
// Putting several drawing objects together and lining them up: the places Align gives each object (left, center, right, top, middle, bottom, and evenly spaced), a group made of objects'
// elements (xdr:grpSp), and a group taken apart again. Pure functions of boxes and XML: the window and the objects of a sheet do the rest.

#include "core/objectclip.h"
#include "grid/objectlayer.h"

#include <vector>

namespace jp::crunch {

enum class ObjectAlign { Left, Center, Right, Top, Middle, Bottom, DistributeHorizontally, DistributeVertically };

// Where each box goes when the boxes are aligned as asked (the grid's units): to the edge or middle of all of them, or, for Distribute, with the same space between neighbors, the first and
// last (by place) where they are. Boxes that stay are given as they are.
std::vector<jp::grid::ObjectBox> alignedBoxes(const std::vector<jp::grid::ObjectBox> &boxes, ObjectAlign how);

// The most objects in one group, and how many groups one inside the other are taken apart (a drawing of a stranger is bounded).
constexpr int kMaxGroupMembers = 500;

struct GroupMember {
    ObjectClip clip;
    jp::grid::ObjectBox box;   // where it is on the sheet (the grid's units)
};

// A group of the members, as a clip that puts the group on a sheet like any pasted object: its xdr:grpSp holds each member's element with its place on the sheet written into it, and its
// size is the box all of them lie in. False, with the reason, for fewer than two members, one that is a chart, or too many.
bool buildGroupClip(const std::vector<GroupMember> &members, ObjectClip *group, jp::grid::ObjectBox *box, QString *why = nullptr);

// A group taken apart: each of its members as a clip of its own, with the box it has on the sheet when the group lies at `groupBox`. False, with the reason, if the clip is no group.
bool splitGroupClip(const ObjectClip &group, const jp::grid::ObjectBox &groupBox, std::vector<GroupMember> *members, QString *why = nullptr);

} // namespace jp::crunch
