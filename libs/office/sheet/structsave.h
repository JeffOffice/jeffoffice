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
    std::vector<ColumnRename> renames;   // columns of tables renamed (the formulas that name them follow)
    QStringList sheetNames;   // the workbook's, in order
    std::vector<SheetOp> sheetOps;       // sheets renamed or deleted, in the order made, after the edits above (what names a sheet follows them)
};

// Whether an edit of the list moves cells on the sheet (inserted or deleted rows or columns, cells shifted).
bool movesCellsOf(const MoveContext &ctx, int sheet);

// A worksheet part with the edits made: `in` is read to its end and the new XML written to `out` (null: nothing is written, and `changed` says only whether the part would be). The rows
// are renumbered only for a sheet the edits move cells on; any sheet has what refers to cells (in the part after its cells) followed.
bool moveSheetPart(QIODevice *in, PartSink *out, const MoveContext &ctx, int sheet, bool *changed, QString *error, const std::atomic<bool> *cancel);

// What a copy of a sheet takes from the file of the sheet it was copied from, of what the program keeps no model of: the namespaces its elements use, the properties of the sheet (how it is
// outlined and printed), what follows the cells (the protection, protected ranges, filter, and sort), and what ends the sheet (links, print options, margins, page setup, headers and footers,
// page breaks, and the errors that are ignored). The parts that name a part of the package (a printer's settings) are left out.
struct SheetExtras {
    QByteArray namespaces, sheetPr, afterData, tail;
    bool found = false;
};
// Reads them from the head and the tail of the sheet's part; the cells between are passed over and not kept.
bool readSheetExtras(QIODevice *in, SheetExtras *out, QString *error = nullptr);
// The extras with the edits made to rows and columns applied (the edits are `ctx.edits`, with the copy's place as `sheet` for those made to the cells it was copied from before the copy).
bool moveSheetExtras(SheetExtras *extras, const MoveContext &ctx, int sheet, QString *error, const std::atomic<bool> *cancel);
// The relationships of the sheet's part that its links name (the Relationship elements for the copy's own relationships part); empty if there are none.
QByteArray hyperlinkRelationshipsXml(const QVector<Relationship> &relationships);
// The XML a copy begins as (see newSheetSkeleton) with the extras put in their places; a sheet that is protected is given protection if the extras have none.
QByteArray skeletonWithExtras(const QByteArray &skeleton, const SheetExtras &extras, bool isProtected);

// The other parts, as bytes: `changed` says whether the output is not the input.
bool moveWorkbookPart(const QByteArray &xml, const MoveContext &ctx, QByteArray *out, bool *changed);
// `ownSheet` is the sheet the chart is on (a reference with no sheet in it, and a name of that sheet, are its).
bool moveChartPart(const QByteArray &xml, const MoveContext &ctx, QByteArray *out, bool *changed, int ownSheet = 0);
bool movePivotCachePart(const QByteArray &xml, const MoveContext &ctx, QByteArray *out, bool *changed);
// A drawing of `sheet`: the cells its anchors name.
bool moveDrawingPart(const QByteArray &xml, const MoveContext &ctx, int sheet, QByteArray *out, bool *changed);
// The notes of `sheet`: the comments part, and the drawing that draws them.
bool moveCommentsPart(const QByteArray &xml, const MoveContext &ctx, int sheet, QByteArray *out, bool *changed);
bool moveNotesDrawing(const QByteArray &vml, const MoveContext &ctx, int sheet, QByteArray *out, bool *changed);
// The threads of comments of `sheet` (the part names the cell of each), and the properties of a form control (the cell it sets, the range it lists).
bool moveThreadedCommentsPart(const QByteArray &xml, const MoveContext &ctx, int sheet, QByteArray *out, bool *changed);
bool moveControlPart(const QByteArray &xml, const MoveContext &ctx, int sheet, QByteArray *out, bool *changed);

} // namespace jp::office::sheet
