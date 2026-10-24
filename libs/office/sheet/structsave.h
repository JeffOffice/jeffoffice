#pragma once
// The parts of a file as rows, columns, and cells are inserted and deleted in the workbook: the sheet's cells are renumbered (those deleted are left out, shared formulas are written out so that
// no cell loses its formula), and everything in the file that refers to cells follows them: merged ranges, conditional formats, validations, hyperlinks, page breaks, filters, the names of the
// workbook (print areas and titles among them), charts, the anchors of drawings, notes, and the sources of PivotTables. The edits are a list, in the order they were made, applied to
// each part in turn. The cells that the workbook changed (and the formulas whose text changed because of an edit) are not here: they are put in whole afterwards (see xlsxsave.h).

#include "sheet/refshift.h"
#include "sheet/xlsxsave.h"

#include <atomic>
#include <vector>

namespace jp::office::sheet {

struct MoveContext {
    std::vector<StructEdit> edits;
    QStringList sheetNames;   // the workbook's, in order
};

// Whether an edit of the list moves cells on the sheet (inserted or deleted rows or columns, cells shifted).
bool movesCellsOf(const MoveContext &ctx, int sheet);

// A worksheet part with the edits made: `in` is read to its end and the new XML written to `out` (null: nothing is written, and `changed` says only whether the part would be). The rows
// are renumbered only for a sheet the edits move cells on; any sheet has what refers to cells (in the part after its cells) followed.
bool moveSheetPart(QIODevice *in, PartSink *out, const MoveContext &ctx, int sheet, bool *changed, QString *error, const std::atomic<bool> *cancel);

// The other parts, as bytes: `changed` says whether the output is not the input.
bool moveWorkbookPart(const QByteArray &xml, const MoveContext &ctx, QByteArray *out, bool *changed);
bool moveChartPart(const QByteArray &xml, const MoveContext &ctx, QByteArray *out, bool *changed);
bool movePivotCachePart(const QByteArray &xml, const MoveContext &ctx, QByteArray *out, bool *changed);
// A drawing of `sheet`: the cells its anchors name.
bool moveDrawingPart(const QByteArray &xml, const MoveContext &ctx, int sheet, QByteArray *out, bool *changed);
// The notes of `sheet`: the comments part, and the drawing that draws them.
bool moveCommentsPart(const QByteArray &xml, const MoveContext &ctx, int sheet, QByteArray *out, bool *changed);
bool moveNotesDrawing(const QByteArray &vml, const MoveContext &ctx, int sheet, QByteArray *out, bool *changed);

} // namespace jp::office::sheet
