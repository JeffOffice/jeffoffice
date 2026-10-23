#pragma once
// The sizes along one side of a grid, rows or columns. A sheet has a million rows, so no row is stored: there is a default size and a list of the places that differ
// (a size of their own, or none at all when hidden), kept in order with the running sum of how much each differs. The start of any row, and the row at any place, are then
// found in time in proportion to the logarithm of the places that differ, so that scrolling to row 1,000,000 costs what scrolling to row 10 does.
//
// A size is in the grid's own units (pixels at 100%). A place with no size (size 0, or hidden) is not shown: the row at a place is never one of those, and a walk from row to
// row steps over a run of them in one step.

#include <QtGlobal>

#include <memory>
#include <vector>

namespace jp::grid {

class Axis {
public:
    // The most a size may be (a row of 4,000 points is not made taller than a screen).
    static constexpr int kMaxSize = 20000;

    struct Place {
        qint64 index = 0;
        int size = 0;           // 0 for a place that is hidden
    };

    explicit Axis(qint64 count = 0, int defaultSize = 20);
    // A copy shares the list of places with the axis it is made from (a million rows with sizes of their own are forty megabytes), until either is changed.
    Axis(const Axis &other);
    Axis &operator=(const Axis &other);
    ~Axis();

    qint64 count() const { return m_count; }
    int defaultSize() const { return m_default; }
    void setCount(qint64 count);
    void setDefaultSize(int size);

    // A place with a size of its own (0 hides it). A size equal to the default is the default again. For a few edits; a sheet's own list is given whole to assign().
    void setSize(qint64 index, int size);
    // Replaces every place that differs with these, which are in order of index with none twice (a place that is not, or past the count, or with a size equal to the
    // default, is dropped).
    void assign(std::vector<Place> places);
    int placeCount() const;

    int size(qint64 index) const;
    bool isHidden(qint64 index) const { return size(index) == 0; }
    // Where the row begins, from the first row's top; position(count()) is total().
    qint64 position(qint64 index) const;
    qint64 total() const { return position(m_count); }
    // The row that holds the place (never a hidden one); -1 for a place before the first or after the last.
    qint64 indexAt(qint64 position) const;
    // The first row after `index` that is shown (count() if there is none), and the last row before it (-1 if there is none).
    qint64 nextVisible(qint64 index) const;
    qint64 previousVisible(qint64 index) const;
    qint64 firstVisible() const { return nextVisible(-1); }
    qint64 lastVisible() const { return previousVisible(m_count); }

    // True if the two hold the same list of places (a copy of an axis shares it until one is changed).
    bool sharesStorageWith(const Axis &other) const;
    // Steps the searches have taken (a test counts them to see that a lookup costs the logarithm of the places, whatever the count).
    qint64 probes() const { return m_probes; }

private:
    struct Data;
    void rebuild() const;
    void ensureBuilt() const;
    Data &writable();
    // The place with this index, or -1.
    int find(qint64 index) const;

    qint64 m_count = 0;
    int m_default = 20;
    std::shared_ptr<Data> m_d;   // the places that differ, and what is worked out from them
    mutable qint64 m_probes = 0;
};

} // namespace jp::grid
