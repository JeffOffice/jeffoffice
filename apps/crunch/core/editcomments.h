#pragma once
// The rules JeffCrunch's threads share with the other apps (libs/page/core/threads.h) as functions of the workbook's cell anchors: Previous and Next along the threads, and the
// threads as the shared model of the Reviewing Pane lists them (libs/base/text/review.h; read only). The edits are EditSession's (core/editsession.h).

#include "core/anchors.h"
#include "text/review.h"

#include <functional>

namespace jp::crunch {

// The thread after (before, going back) the cell in the workbook's order, sheet then row then column; false where there is none (they do not wrap).
bool stepThread(const CellAnchors &anchors, int sheet, quint32 row, quint32 col, bool forward, int *toSheet, quint32 *toRow, quint32 *toCol);

// Each thread as a Commented change: container the sheet's name (`sheetName` says it), object the cell's address, path "thread"; the comments after the first are its replies, a
// resolved thread is Accepted; sorted by review::crunchPosition.
// For tests: the most comments a workbook may hold in threads (0 puts the real one back).
void setBookCommentCapForTest(qint64 most);
// The most comments a workbook may hold in threads that a person adds (the reader's own limit), the comments in the threads now, and what is said at the limit.
qint64 bookCommentCap();
QString bookCapText();
qint64 threadCommentsIn(const CellAnchors &anchors);

jp::review::ChangeList threadChanges(const CellAnchors &anchors, const std::function<QString(int)> &sheetName);

} // namespace jp::crunch
