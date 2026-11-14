#pragma once
// Linked text boxes as the file says them (see jeffwrite.md): the first box of a chain holds the words (`wps:txbx` with an id, its number), and each box after it is `wps:linkedTxbx` with that
// number and its place in the chain. These read and write that form on a drawing kept as written (the element of the box itself: a box inside another box's words is no part of it), and make
// right what a document's boxes say among them (a chain whose first box is gone, boxes pasted from another chain).

#include <QByteArray>
#include <QSet>
#include <QVector>

namespace jp::write {

// The most a chain's number may be (the schema's unsignedShort).
constexpr int kMostChainNumber = 65535;

struct BoxChain {
    int number = -1;      // the chain's number (0 is a number like any other); -1 for a box of its own
    int seq = 0;          // a member's place in the chain (the first box is 0)
    bool member = false;  // wps:linkedTxbx: the box has no words of its own
};

// What the drawing's own box element says (empty chain for a drawing that is not a text box, or whose box says no number).
BoxChain boxChainOf(const QByteArray &drawing);
// The drawing with its box a member of chain `number` at place `seq`; with its box the first of chain `number` (its words kept), `number` -1 taking the number away; with its box a box of its own
// with no words. Each returns the drawing as it was where it has no box element of the kind.
QByteArray boxAsMember(const QByteArray &drawing, int number, int seq);
QByteArray boxWithNumber(const QByteArray &drawing, int number);
QByteArray boxAsPlain(const QByteArray &drawing);

// The least number from 0 up to kMostChainNumber that is not in `used`, or -1 if all are.
int freeChainNumber(const QSet<int> &used);

// Makes the chains of a document's drawings sound: a member whose first box is not among them becomes a box of its own. Returns whether any drawing changed.
bool repairBoxChains(const QVector<QByteArray *> &drawings);
// Drawings that were pasted (or duplicated) into a document whose other boxes use the numbers `usedElsewhere`: each chain among them gets a number of its own, so that it is a chain of its own;
// a member whose first box was not pasted with it becomes a box of its own. Returns whether any drawing changed.
bool renumberPastedBoxChains(const QVector<QByteArray *> &pasted, const QSet<int> &usedElsewhere);

} // namespace jp::write
