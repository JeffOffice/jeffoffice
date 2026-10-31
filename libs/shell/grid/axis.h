#pragma once
// The sizes along one side of a grid, rows or columns. A sheet has a million rows, so no row is stored: there is a default size and a list of the places that differ
// (a size of their own, or none at all when hidden), kept in order with the running sum of how much each differs. The start of any row, and the row at any place, are then
// found in time in proportion to the logarithm of the places that differ, so that scrolling to row 1,000,000 costs what scrolling to row 10 does.
//
// A size is in the grid's own units (pixels at 100%). A place with no size (size 0, or hidden) is not shown: the row at a place is never one of those, and a walk from row to
// row steps over a run of them in one step.

#include <QtGlobal>

#include <limits>
#include <memory>
#include <vector>

namespace jp::grid {

// A count of the work a slice of a long job may do before it stops and leaves the rest for the next turn. The work is counted in steps of about the same small cost (a place, a row, a
// band, a page), never timed, so a slice does the same on a slow machine as on a fast one. A step that cannot be cut (a row's cells) may take the slice a little past its limit.
struct Work {
    qint64 limit = std::numeric_limits<qint64>::max();
    qint64 used = 0;
    Work() = default;
    explicit Work(qint64 maxSteps) : limit(maxSteps) {}
    bool spent() const { return used >= limit; }
    void add(qint64 steps = 1) { used += steps; }
};

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
    // The first `count` rows are not shown (as if hidden, and not kept apart from the rest: a view whose frozen rows begin below the first has the rows above them scrolled out of
    // sight). Nothing in them is a row any search finds, and the rows after them begin where the first of them would have.
    void setSkipped(qint64 count);
    qint64 skipped() const { return m_skip; }

    // A place with a size of its own (0 hides it). A size equal to the default is the default again. For a few edits; a sheet's own list is given whole to assign().
    void setSize(qint64 index, int size);
    // Replaces every place that differs with these, which are in order of index with none twice (a place that is not, or past the count, or with a size equal to the
    // default, is dropped).
    void assign(std::vector<Place> places);
    // The same one place at a time, for a list built in slices: begin, append each in order of index (with the same drops), and then build in slices until it says it is built. An
    // axis that is asked anything before it is built builds itself at once.
    void beginAssign(size_t expected = 0);
    void appendPlace(Place place);
    bool buildSome(Work &work) const;
    int placeCount() const;

    // Rows put in (or taken out) in the middle of the sheet, for an axis whose sums are built: the places after move on (back) by `count`, and what was worked out of the places is moved
    // with them by a memory move and a sum added (the run of hidden places the edit cuts or joins is worked out again, and no more), not worked out again from the first place. The count
    // of rows stays what it was: places pushed past the last row are dropped, and the rows at the end after a take are the default size. `own` are the new rows that are not the default
    // size (their indexes are in the new rows, in order, with none twice; a size equal to the default is dropped).
    void insertPlaces(qint64 at, qint64 count, const std::vector<Place> &own = {});
    void erasePlaces(qint64 at, qint64 count);
    // The places that differ, all of them, in order (a copy, for a test that compares two axes).
    std::vector<Place> places() const;
    // How many times an axis has worked its sums out from the first place, in all (a test counts them to see that an insert or a delete worked none out).
    static qint64 buildsForTest();

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

    qint64 rawPosition(qint64 index) const;   // where a row begins, with the skipped rows counted at their own sizes

    qint64 m_count = 0;
    qint64 m_skip = 0;
    int m_default = 20;
    std::shared_ptr<Data> m_d;   // the places that differ, and what is worked out from them
    mutable qint64 m_probes = 0;
};

} // namespace jp::grid
