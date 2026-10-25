#pragma once
// How each sheet of an open workbook prints, as the person has it now: the page setup of every sheet (read from the file when the workbook was opened), changed by the page commands
// (one step of Undo each) and by rows, columns, and cells inserted and deleted (the print area, the titles, and the breaks follow them, and Undo puts them back). Saving asks it for
// what to put in the file; the file is changed only where it differs from this.

#include "sheet/calculation.h"
#include "sheet/pagefollow.h"
#include "sheet/pagesetup.h"
#include "sheet/xlsxsave.h"

#include <QStringList>

#include <set>
#include <vector>

namespace jp::crunch {

// A sheet's setup as a step changed it: as it was and as it is.
struct PageEdit {
    int sheet = 0;
    jp::office::sheet::PageSetup before, after;
};

class PageSettings {
public:
    // Takes the setup of every sheet of the workbook (the calculation has them all, whole).
    void open(const jp::office::sheet::Calculation &calc);
    int sheetCount() const { return int(m_sheets.size()); }
    const jp::office::sheet::PageSetup &sheet(int index) const;
    // The setup of a sheet now (a command; Undo and Redo).
    void set(int index, const jp::office::sheet::PageSetup &setup);

    // Rows, columns, or cells were inserted or deleted (`token` names the step): the setups of the sheets the edit is on follow it, and what they were is kept for undoEdit.
    void followEdit(const jp::office::sheet::StructEdit &edit, const QStringList &sheetNames, const void *token);
    // The step is undone: the setups are as they were before it (nothing if the step is not known).
    void undoEdit(const void *token);

    // The sheets are never renumbered (a sheet's place here is its place for good, as in the calculation): a sheet inserted or copied is added at the end with the setup it begins with (the
    // defaults, or the setup of the sheet it is a copy of, which the file is then to be told of), and undoing that takes the last one away. A sheet deleted, renamed, or moved keeps its
    // setup where it is: the names that carry it into the file are made at saving, from the sheets' names and order then.
    void addSheet(const jp::office::sheet::PageSetup &setup, bool touched);
    void removeLastSheet();

    // What saving needs: the setup of each sheet that may differ from the file's (a command changed it, or an edit moved what it names), and the names of its print area and titles.
    // `sheetNames` are the sheets' names by place (empty for one that is deleted) and `order` the places in the order of the tabs.
    void addToSave(jp::office::sheet::SaveChanges *changes, const QStringList &sheetNames, const QVector<int> &order) const;
    // The file has been written with these: the sheets are as the file has them.
    void saved();
    // How many sheets would be put in a save (for tests).
    int touchedCount() const { return int(m_touched.size()); }

private:
    struct Kept {
        const void *token = nullptr;
        std::vector<std::pair<int, jp::office::sheet::PageSetup>> before;
    };
    std::vector<jp::office::sheet::PageSetup> m_sheets;
    std::set<int> m_touched;
    std::vector<Kept> m_kept;
};

} // namespace jp::crunch
