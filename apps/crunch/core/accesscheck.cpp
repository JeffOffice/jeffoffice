#include "core/accesscheck.h"

#include "core/sheetobjects.h"
#include "sheet/calculation.h"
#include "sheet/cellref.h"
#include "sheet/tables.h"

#include <QCoreApplication>
#include <QRegularExpression>

namespace jp::crunch {

using namespace jp::office::sheet;

std::vector<AccessIssue> checkAccessibility(const EditSession &session, SheetObjects *objects, int most)
{
    std::vector<AccessIssue> out;
    const Workbook *book = session.workbook();
    const Calculation *calc = session.calculation();
    if (!book) return out;
    const char *ctx = "jp::crunch::AccessCheck";
    auto add = [&](AccessIssue::Kind kind, AccessIssue::Level level, int sheet, quint32 row, quint32 col, const QString &message) {
        if (int(out.size()) >= most) return;
        AccessIssue i;
        i.kind = kind;
        i.level = level;
        i.sheet = sheet;
        i.row = row;
        i.col = col;
        i.message = message;
        out.push_back(i);
    };
    static const QRegularExpression defaultName(QStringLiteral("^Sheet[0-9]+$"));
    const auto &sheets = book->info().sheets;
    for (int s = 0; s < int(sheets.size()) && int(out.size()) < most; ++s) {
        if (sheets[size_t(s)].deleted) continue;
        const QString name = sheets[size_t(s)].name;
        if (defaultName.match(name).hasMatch())
            add(AccessIssue::DefaultSheetName, AccessIssue::Tip, s, 0, 0, QCoreApplication::translate(ctx, "The sheet \"%1\" has the name it was given. A name that says what is on the sheet helps a reader find it.").arg(name));
        if (objects)
            for (const SheetObjects::ObjectInfo &o : objects->describe(s)) {
                using K = jp::office::sheet::ObjectKind;
                if (o.kind != K::Chart && o.kind != K::Picture) continue;
                if (!o.description.trimmed().isEmpty()) continue;
                add(AccessIssue::MissingAltText, AccessIssue::Error, s, o.row, o.col,
                    (o.kind == K::Chart ? QCoreApplication::translate(ctx, "The chart \"%1\" on %2 has no alt text, so a screen reader can't say what it shows.")
                                        : QCoreApplication::translate(ctx, "The picture \"%1\" on %2 has no alt text, so a screen reader can't say what it shows."))
                        .arg(o.name.isEmpty() ? QCoreApplication::translate(ctx, "(no name)") : o.name, name));
            }
        if (calc && s < calc->sheetCount())
            for (const jp::office::sheet::CellRange &m : calc->mergesOf(s))
                add(AccessIssue::MergedCells, AccessIssue::Warning, s, m.row1, m.col1,
                    QCoreApplication::translate(ctx, "The cells %1 on %2 are merged. Merged cells are hard to move through with the keyboard and a screen reader.").arg(rangeName(m), name));
    }
    for (const Table &t : book->tables()) {
        if (t.headerRows != 0 || t.sheet < 0 || t.sheet >= int(sheets.size()) || sheets[size_t(t.sheet)].deleted) continue;
        add(AccessIssue::TableWithoutHeader, AccessIssue::Error, t.sheet, t.row1, t.col1,
            QCoreApplication::translate(ctx, "The table %1 on %2 has no header row, so a screen reader can't name its columns.").arg(t.name, sheets[size_t(t.sheet)].name));
    }
    return out;
}

} // namespace jp::crunch
