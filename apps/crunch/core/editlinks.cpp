// Links to other workbooks, for Inspect Workbook: a formula that reads another workbook (`[1]Prices!A1`) is changed to the value it shows, a name that stands for such a place is #REF!, and
// the saver is told to leave the link parts out (libs/office/sheet/xlsxsave.h, dropExternalLinks).
#include "core/editsession.h"
#include "core/gotospecial.h"

#include <QRegularExpression>

namespace jp::crunch {

namespace {

// Whether a formula (as `editText` gives it, with its "=") reads another workbook: a number in square brackets, outside the quotes of a text or of a sheet's name.
bool readsAnotherWorkbook(const QString &formula)
{
    static const QRegularExpression link(QStringLiteral("\\[[0-9]+\\]"));
    bool quoted = false;
    QChar quote;
    for (int i = 0; i < formula.size(); ++i) {
        const QChar c = formula.at(i);
        if (quoted) {
            if (c == quote) quoted = false;
            continue;
        }
        if (c == QLatin1Char('"')) {
            quoted = true;
            quote = c;
        } else if (c == QLatin1Char('[')) {
            if (link.matchView(QStringView(formula).mid(i), 0, QRegularExpression::NormalMatch, QRegularExpression::AnchoredMatchOption).hasMatch()) return true;
        }
    }
    return false;
}

} // namespace

int EditSession::externalLinkCells() const
{
    int n = 0;
    if (!m_book || !m_book->calc) return 0;
    m_book->calc->forEachFormulaText([&](int sheet, quint32 row, quint32 col, const QString &formula) {
        Q_UNUSED(sheet) Q_UNUSED(row) Q_UNUSED(col)
        if (readsAnotherWorkbook(formula)) ++n;
    });
    return n;
}

EditSession::Outcome EditSession::breakExternalLinks(int *cells, int *names)
{
    Outcome out;
    if (cells) *cells = 0;
    if (names) *names = 0;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    struct Place {
        int sheet;
        quint32 row, col;
    };
    std::vector<Place> places;
    m_book->calc->forEachFormulaText([&](int sheet, quint32 row, quint32 col, const QString &formula) {
        if (readsAnotherWorkbook(formula)) places.push_back({sheet, row, col});
    });
    int done = 0, left = 0;
    for (const Place &p : places) {
        jp::office::sheet::CellRange one;
        one.row1 = one.row2 = p.row;
        one.col1 = one.col2 = p.col;
        one.valid = true;
        PasteOptions options;
        options.what = PasteOptions::What::Values;
        const Outcome o = paste(p.sheet, one, copyCells(p.sheet, one, false), options);
        if (o.ok) ++done;
        else ++left;
    }
    int renamed = 0;
    for (int id = 0; id < m_nameTable.slots(); ++id) {
        if (!nameListed(id) || !readsAnotherWorkbook(nameTextOf(id))) continue;
        const NameEntry &e = m_nameTable.at(id);
        NameDraft d;
        d.name = e.name;
        d.scope = e.scope;
        d.text = QStringLiteral("=#REF!");
        d.comment = e.comment;
        d.hidden = e.hidden;
        if (changeName(id, d).ok) ++renamed;
    }
    if (cells) *cells = done;
    if (names) *names = renamed;
    if (left > 0) {
        out.ok = false;
        out.problem = tr("Some cells that read another workbook could not be changed, because they are locked or part of an array. The links stay.");
        return out;
    }
    // (the steps that held the formulas are let go of: the links are gone from the file, and a formula brought back would read one)
    forgetStepsDone();
    m_dropExternalLinks = true;
    return out;
}

} // namespace jp::crunch
