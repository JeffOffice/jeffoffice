#pragma once
// Booklet imposition: which pages go on which side of a sheet so that the sheets, folded in half and stacked, read in order. JeffPub prints its booklets with it, and JeffWrite its
// book fold.

#include <QVector>

namespace jp {

// The sides of the sheets, in the order they are printed (front, back, front, back...): each side holds two pages, left then right, as indexes from 0 (-1 is a blank, where the
// number of pages is filled out to a multiple of four).
QVector<QVector<int>> bookletOrder(int pages);
// The same with the pages split into booklets of `pagesPerBooklet` pages each (0 for all of them in one; a number that is not a multiple of four is taken up to one): each booklet is
// imposed by itself, and its sheets follow those of the booklet before. The last booklet is filled out with blanks.
QVector<QVector<int>> bookletOrder(int pages, int pagesPerBooklet);

} // namespace jp
