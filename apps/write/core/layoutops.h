#pragma once
// Layout and Design tabs' work on a section's properties: columns (with their widths and the line between), line numbers, and the lines round the pages. Each function changes a
// SectionProps (what the file keeps), the way the file states it; applyToSections puts a change on a section or on all of them as one step of the history.

#include "core/writedoc.h"

#include <QColor>
#include <QVector>
#include <functional>

namespace jp::write {

// What the Columns dialog shows of a section, in points.
struct ColumnSetup {
    int count = 1;
    bool equal = true;               // equal widths: `space` between them
    double space = 36;
    QVector<double> widths, spaces;   // when not equal: each column's width, and the space after it (the last one's is not used)
    bool line = false;               // a line between the columns
    bool operator==(const ColumnSetup &o) const { return count == o.count && equal == o.equal && space == o.space && widths == o.widths && spaces == o.spaces && line == o.line; }
};
constexpr int kMostColumnsSet = 12;   // the most columns the dialog sets (a file may have up to flow::kMostColumns)

ColumnSetup columnSetupOf(const office::wml::SectionProps &props);
// The columns of the setup, held to what a page holds (1 to kMostColumns columns, spaces and widths not negative). Unequal widths are as many as columns, or equal ones.
ColumnSetup normalized(ColumnSetup setup);
void setColumnSetup(office::wml::SectionProps *props, const ColumnSetup &setup);

// How a section's lines are numbered: nothing, or counted by `countBy`, from `start`, beginning again by `restart`.
struct LineNumberSetup {
    int countBy = 0, start = 1;
    double distance = 0;               // from the text, in points (0: the usual)
    enum class Restart { EachPage, EachSection, Continuous } restart = Restart::EachPage;
    bool operator==(const LineNumberSetup &o) const { return countBy == o.countBy && start == o.start && distance == o.distance && restart == o.restart; }
};
LineNumberSetup lineNumbersOf(const office::wml::SectionProps &props);
void setLineNumbers(office::wml::SectionProps *props, const LineNumberSetup &setup);

// The lines round the pages: one kind of line on any of the four sides.
struct PageBorderSetup {
    bool top = false, left = false, bottom = false, right = false;
    QString style = QStringLiteral("single");   // single, double, dotted, dashed, thick
    double width = 0.5;                        // points
    QColor color{Qt::black};
    double space = 24;                         // from the edge, points
    bool fromPage = false;                     // measured from the edge of the page, else from the text
    QString display = QStringLiteral("all");   // all, firstPage, or notFirstPage
    bool any() const { return top || left || bottom || right; }
};
PageBorderSetup pageBordersOf(const office::wml::SectionProps &props);
void setPageBorders(office::wml::SectionProps *props, const PageBorderSetup &setup);

// Runs `change` on the properties of the section (the one at `section`, or every one) as one step; returns how many sections it changed.
int applyToSections(WriteDoc &doc, int section, bool whole, const std::function<void(office::wml::SectionProps *)> &change);

} // namespace jp::write
