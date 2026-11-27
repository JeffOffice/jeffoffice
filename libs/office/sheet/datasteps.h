#pragma once
// The steps of a query made by Data > Get Data: a short, closed list of operations on the rows a file gave (datastepmodel.h), saved in the query's text (datasource.h), run again by every Refresh, and
// never trusted: every count is checked before what it counts is made, a step that can't be run says which step and why, and nothing is changed when one fails. The steps work on DataRows as the
// readers made them and know nothing of the sheet, so they run on any thread.

#include "sheet/datasource.h"

namespace jp::office::sheet {

struct DataStepsRun {
    bool ok = false;
    QString why;                 // "Step 3 (Filter rows: Price is greater than 10): the column is not in the file. The columns are: ..."
    int failedStep = -1;         // from 0
    QVector<char> forcedKinds;   // for each column of the result: 't', 'n', or 'd' when a step fixed it, else 0 (the kind is judged from the values)
    qint64 work = 0;             // the cell visits made (for tests)
};

// Runs the steps in order on `rows`; on failure `rows` is of no use (the caller reads the source again). `cancelled` is asked as it goes.
DataStepsRun applyDataSteps(const QVector<DataStep> &steps, DataRows *rows, const std::function<bool()> &cancelled, qint64 workLimit = kMaxDataStepWork);

}   // namespace jp::office::sheet
