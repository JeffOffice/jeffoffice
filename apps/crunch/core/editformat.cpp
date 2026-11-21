// Formatting the cells: fonts, fills, borders, number formats, alignment, protection, named styles, merged cells, and clearing. One step each, made of the cells whose formats changed (as they
// were and as they are), the formats of whole columns and rows, and the merged ranges. See EditSession::formatCells.
#include "core/editsession.h"
#include "sheet/fonttext.h"

#include "core/formatfind.h"
#include "sheet/builtinstyles.h"

#include <QCoreApplication>

#include <algorithm>
#include <map>
#include <set>
#include <QUndoCommand>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::Value;

namespace {

// The format the cells of a block that hold nothing have: their row's when the row has one, else their column's, else the first.
struct Implicit {
    quint32 row1 = 0, col1 = 0;
    std::vector<quint32> rows, cols;
    quint32 at(quint32 row, quint32 col) const
    {
        if (row >= row1 && row - row1 < rows.size() && rows[row - row1]) return rows[row - row1];
        if (col >= col1 && col - col1 < cols.size()) return cols[col - col1];
        return 0;
    }
};

Implicit implicitOf(const Calculation &calc, int sheet, quint32 row1, quint32 row2, quint32 col1, quint32 col2)
{
    Implicit out;
    out.row1 = row1;
    out.col1 = col1;
    if (row2 - row1 < 200000) {
        out.rows.assign(size_t(row2 - row1) + 1, 0);
        for (const RowSpec &r : calc.rowSpecsIn(sheet, row1, row2))
            if (r.customFormat && r.row >= row1 && r.row <= row2) out.rows[r.row - row1] = r.style;
    }
    out.cols.assign(size_t(col2 - col1) + 1, 0);
    for (const ColumnSpan &c : calc.columnRuns(sheet, col1, col2))
        for (quint32 col = std::max(c.first, col1); col <= std::min(c.last, col2); ++col) out.cols[col - col1] = c.style;
    return out;
}

} // namespace

EditSession::Outcome EditSession::restyle(int sheet, const std::vector<RestyleTarget> &targets, const QString &text, const SizeBuilder &sizes)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (targets.empty() && !sizes) return out;
    // what a protected sheet's protection lets be formatted does not include the lock and the hiding of formulas: they are what the protection stands on
    if (jp::office::sheet::FormatChange const *last = nullptr; m_book->calc->sheet(sheet).isProtected) {
        for (const RestyleTarget &t : targets) {
            if (t.change == last) continue;
            last = t.change;
            if (t.change && (t.change->locked || t.change->formulaHidden)) {
                out.ok = false;
                out.problem = tr("This sheet is protected, so whether its cells are locked, and whether their formulas are hidden, can't be changed. Unprotect the sheet first (Review, Unprotect Sheet).");
                return out;
            }
        }
    }
    Step step;
    {
        // (the formats are made with the worker held back: it reads the workbook's styles)
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) {
            out.ok = false;
            out.problem = tr("The workbook is busy. Try again in a moment.");
            return out;
        }
        Calculation *calc = hold->calculation();
        Workbook &book = *m_book->book;
        book.clearFormatLimit();
        quint32 row1 = targets.empty() ? 0 : targets.front().row, row2 = row1, col1 = targets.empty() ? 0 : targets.front().col, col2 = col1;
        for (const RestyleTarget &t : targets) {
            row1 = std::min(row1, t.row);
            row2 = std::max(row2, t.row);
            col1 = std::min(col1, t.col);
            col2 = std::max(col2, t.col);
        }
        const Implicit implicitStyles = implicitOf(*calc, sheet, row1, row2, col1, col2);
        std::map<std::pair<quint32, const FormatChange *>, quint32> made;
        step.styles.reserve(targets.size());
        for (const RestyleTarget &t : targets) {
            const Calculation::CellContent before = calc->content(sheet, t.row, t.col);
            const quint32 base = before.exists ? before.style : implicitStyles.at(t.row, t.col);
            if (QString notAllowed; !allows(sheet, Allow::FormatCells, &notAllowed)) {
                out.ok = false;
                out.problem = notAllowed;
                return out;
            }
            quint32 style = 0;
            if (t.change) {
                const auto key = std::make_pair(base, t.change);
                const auto found = made.find(key);
                if (found != made.end()) style = found->second;
                else made[key] = style = book.formatWith(base, *t.change);
            }
            if (style == base) continue;
            if (!before.exists && style == 0) continue;
            StyleCell ch;
            ch.sheet = sheet;
            ch.row = t.row;
            ch.col = t.col;
            ch.before = before.exists ? before.style : 0;
            ch.after = style;
            step.styles.push_back(ch);
        }
        if (sizes) sizes(book, *calc, &step);
        if (book.formatLimitHit()) {
            // (a format that could not be made would have been left out of the step without a word)
            out.ok = false;
            out.problem = tr("This workbook has as many different formats as a workbook can hold (%1 cell formats). Give the cells a format the workbook already has, or clear the formats of some cells, and try again.")
                              .arg(QLocale().toString(jp::office::sheet::kMaxCellFormatsWritten));
            book.clearFormatLimit();
            return out;
        }
    }
    if (step.empty()) return out;
    pushStep(std::move(step), text);
    return out;
}

EditSession::Outcome EditSession::restyleRange(int sheet, const CellRange &range, const std::shared_ptr<const FormatChange> &change, const QString &text)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (!range.valid || range.row2 >= kMaxRows || range.col2 >= kMaxColumns) {
        out.ok = false;
        out.problem = tr("Those are not cells of the sheet.");
        return out;
    }
    out.range = range;
    const bool wholeColumns = range.row1 == 0 && range.row2 == kMaxRows - 1;
    const bool wholeRows = !wholeColumns && range.col1 == 0 && range.col2 == kMaxColumns - 1;
    const Calculation *calc = m_book->calc.get();
    std::vector<RestyleTarget> targets;
    if (!wholeColumns && !wholeRows) {
        const qint64 count = (qint64(range.row2) - range.row1 + 1) * (qint64(range.col2) - range.col1 + 1);
        if (count > kMaxStyledCells && change) {
            out.ok = false;
            out.problem = tr("That is more cells than JeffCrunch formats at once (%1). Select whole columns or rows, or fewer cells.").arg(QLocale().toString(kMaxStyledCells));
            return out;
        }
        if (change) {
            targets.reserve(size_t(count));
            for (quint32 r = range.row1; r <= range.row2; ++r)
                for (quint32 c = range.col1; c <= range.col2; ++c) targets.push_back({r, c, change.get()});
        } else {
            // (the cells that are there: one that is not has no format of its own to clear)
            qint64 seen = 0;
            calc->sheet(sheet).cells.forEachInRange(range.row1, range.col1, range.row2, range.col2, [&](quint32 row, quint32 col, const Cell &cell) {
                if (++seen > kMaxStyledCells) return false;
                targets.push_back({row, col, nullptr});
                return true;
            });
            if (seen > kMaxStyledCells) {
                out.ok = false;
                out.problem = tr("That is more cells than JeffCrunch clears the formats of at once (%1). Select fewer cells.").arg(QLocale().toString(kMaxStyledCells));
                return out;
            }
        }
        return restyle(sheet, targets, text);
    }
    // whole columns or rows: the cells that have formats of their own are changed, and the columns' (rows') own format is
    qint64 seen = 0;
    calc->sheet(sheet).cells.forEachInRange(range.row1, range.col1, range.row2, range.col2, [&](quint32 row, quint32 col, const Cell &cell) {
        if (++seen > kMaxStyledCells) return false;
        if (cell.style != 0 || change) targets.push_back({row, col, change.get()});
        return true;
    });
    if (seen > kMaxStyledCells) {
        out.ok = false;
        out.problem = tr("That is more cells than JeffCrunch formats at once (%1). Select fewer columns or rows.").arg(QLocale().toString(kMaxStyledCells));
        return out;
    }
    if (wholeRows && qint64(range.row2) - range.row1 + 1 > kMaxRowsPerResize) {
        out.ok = false;
        out.problem = tr("That is more rows than JeffCrunch formats at once (%1). Select fewer rows.").arg(QLocale().toString(kMaxRowsPerResize));
        return out;
    }
    // (the cells' formats and the columns' or rows' own are made in the same step)
    auto sizes = [=](Workbook &book, const Calculation &c, Step *step) {
        SizeEdit e;
        e.sheet = sheet;
        auto styled = [&](quint32 base) { return change ? book.formatWith(base, *change) : 0u; };
        if (wholeColumns) {
            e.columns = true;
            e.first = range.col1;
            e.last = range.col2;
            e.columnsBefore = c.columnRuns(sheet, range.col1, range.col2);
            quint32 pos = range.col1;
            auto emit = [&](quint32 a, quint32 b, const ColumnSpan *base) {
                ColumnSpan col;
                if (base) col = *base;
                col.first = a;
                col.last = b;
                col.style = styled(col.style);
                if (base || col.width >= 0 || col.hidden || col.style != 0) e.columnsAfter.push_back(col);
            };
            for (const ColumnSpan &r : e.columnsBefore) {
                if (r.first > pos) emit(pos, r.first - 1, nullptr);
                emit(r.first, r.last, &r);
                pos = r.last + 1;
            }
            if (pos <= range.col2) emit(pos, range.col2, nullptr);
        } else {
            e.columns = false;
            e.first = range.row1;
            e.last = range.row2;
            e.rowsBefore = c.rowSpecsIn(sheet, range.row1, range.row2);
            size_t at = 0;
            for (quint32 r = range.row1; r <= range.row2; ++r) {
                RowSpec spec;
                if (at < e.rowsBefore.size() && e.rowsBefore[at].row == r) spec = e.rowsBefore[at++];
                else spec.row = r;
                const quint32 now = styled(spec.customFormat ? spec.style : 0);
                spec.style = now;
                spec.customFormat = now != 0;
                // (a row with nothing to say is not listed)
                if (spec.height >= 0 || spec.hidden || spec.customFormat || spec.customHeight) e.rowsAfter.push_back(spec);
            }
        }
        step->sizes.push_back(std::move(e));
    };
    return restyle(sheet, targets, text, sizes);
}

namespace {

// The most cells of a sheet looked at for the fonts they use (a sheet of millions of cells is read as far as this).
constexpr qint64 kMostCellsForFonts = 2000000;

}

QList<EditSession::FontUse> EditSession::fontsInUse() const
{
    QList<FontUse> out;
    if (!m_book || !m_book->book || !m_book->calc) return out;
    const Styles &st = m_book->book->styles();
    const auto &sheets = m_book->book->info().sheets;
    const Calculation *calc = m_book->calc.get();
    QHash<quint32, QString> names;
    auto fontOf = [&](quint32 xf) {
        auto it = names.find(xf);
        if (it == names.end()) {
            const CellFormat &f = xf < quint32(st.formats.size()) ? st.formats[int(xf)] : CellFormat();
            it = names.insert(xf, st.fontNameOf(st.font(f.font)));
        }
        return it.value();
    };
    auto add = [&](const QString &name, const QString &sheetName) {
        if (name.isEmpty()) return;
        int at = -1;
        for (int i = 0; i < out.size() && at < 0; ++i)
            if (out[i].family == name) at = i;
        if (at < 0) {
            out.push_back({name, {}});
            at = int(out.size()) - 1;
        }
        if (!out[at].sheets.contains(sheetName)) out[at].sheets << sheetName;
    };
    for (int s = 0; s < calc->sheetCount(); ++s) {
        const auto &sh = calc->sheet(s);
        const QString sheetName = size_t(s) < sheets.size() ? sheets[size_t(s)].name : QString();
        // The formats of the columns and rows (what the cells that hold nothing have), and the fonts of the runs of formatted text.
        for (const ColumnSpan &c : calc->columnRuns(s, 0, kMaxColumns - 1))
            if (c.style != 0) add(fontOf(c.style), sheetName);
        for (const RowSpec &r : calc->rowSpecsOf(s))
            if (r.customFormat && r.style != 0) add(fontOf(r.style), sheetName);
        for (const FontSpec &f : sh.richFonts) add(st.fontNameOf(f), sheetName);
        if (sh.extentRows == 0 || sh.extentCols == 0) continue;
        qint64 looked = 0;
        sh.cells.forEachInRange(0, 0, sh.extentRows - 1, sh.extentCols - 1, [&](quint32, quint32, const Cell &cell) {
            add(fontOf(cell.style), sheetName);
            return ++looked < kMostCellsForFonts;
        });
    }
    return out;
}

EditSession::Outcome EditSession::replaceFont(const QString &from, const QString &to, const QString &text, int *cells)
{
    Outcome out;
    if (cells) *cells = 0;
    if (!m_book || !m_book->book || !m_book->calc) return out;
    const Styles &st = m_book->book->styles();
    const Calculation *calc = m_book->calc.get();
    FormatChange change;
    change.fontName = to;
    QHash<quint32, bool> hasFrom;
    auto isFrom = [&](quint32 xf) {
        auto it = hasFrom.find(xf);
        if (it == hasFrom.end()) {
            const CellFormat &f = xf < quint32(st.formats.size()) ? st.formats[int(xf)] : CellFormat();
            it = hasFrom.insert(xf, st.fontNameOf(st.font(f.font)).compare(from, Qt::CaseInsensitive) == 0);
        }
        return it.value();
    };
    // Every sheet is looked at first: a font that the program can't replace everywhere, or more cells than it changes at once, is refused with nothing changed.
    std::vector<std::vector<RestyleTarget>> all(size_t(std::max(0, calc->sheetCount())));
    qint64 total = 0;
    bool elsewhere = false;
    for (int s = 0; s < calc->sheetCount(); ++s) {
        const auto &sh = calc->sheet(s);
        for (const ColumnSpan &c : calc->columnRuns(s, 0, kMaxColumns - 1)) elsewhere = elsewhere || (c.style != 0 && isFrom(c.style));
        for (const RowSpec &r : calc->rowSpecsOf(s)) elsewhere = elsewhere || (r.customFormat && r.style != 0 && isFrom(r.style));
        for (const FontSpec &f : sh.richFonts) elsewhere = elsewhere || st.fontNameOf(f).compare(from, Qt::CaseInsensitive) == 0;
        if (sh.extentRows == 0 || sh.extentCols == 0) continue;
        bool tooMany = false;
        sh.cells.forEachInRange(0, 0, sh.extentRows - 1, sh.extentCols - 1, [&](quint32 row, quint32 col, const Cell &cell) {
            if (isFrom(cell.style)) {
                all[size_t(s)].push_back({row, col, &change});
                tooMany = ++total > kMaxStyledCells;
            }
            return !tooMany;
        });
        if (tooMany) {
            out.ok = false;
            out.problem = tr("That is more cells than JeffCrunch changes at once (%1), so no font was replaced.").arg(QLocale().toString(kMaxStyledCells));
            return out;
        }
    }
    if (elsewhere) {
        out.ok = false;
        out.problem = tr("The font is also used by whole columns or rows, or by formatted text inside cells, which JeffCrunch can't replace yet, so no font was replaced.");
        return out;
    }
    for (int s = 0; s < calc->sheetCount(); ++s) {
        if (all[size_t(s)].empty()) continue;
        const Outcome one = restyle(s, all[size_t(s)], text);
        if (!one.ok) return one;   // (a sheet that is protected: the sheets before it are changed, and the window says so)
        if (cells) *cells += int(all[size_t(s)].size());
    }
    return out;
}

EditSession::Outcome EditSession::formatCells(int sheet, const CellRange &range, const FormatChange &change, const QString &text)
{
    if (change.fontName && change.fontName->size() > jp::office::sheet::kMaxFontNameFileChars) {
        // (the other program refuses a file with a font named by more than 31 characters)
        Outcome out;
        out.ok = false;
        out.problem = tr("A font's name can have up to %1 characters in a workbook, and this one has %2. Choose another font.").arg(jp::office::sheet::kMaxFontNameFileChars).arg(change.fontName->size());
        return out;
    }
    return restyleRange(sheet, range, std::make_shared<FormatChange>(change), text);
}

namespace {

// The name of the font a cell format is set in; nothing for one that follows the theme (its scheme says "major" or "minor").
QString fontNamed(const jp::office::sheet::Styles &styles, quint32 style, QHash<quint32, QString> *seen)
{
    const auto found = seen->constFind(style);
    if (found != seen->constEnd()) return *found;
    const jp::office::sheet::FontSpec &font = styles.font(styles.format(style).font);
    const QString name = font.scheme.isEmpty() ? font.name : QString();
    seen->insert(style, name);
    return name;
}

} // namespace

// Fonts put back, or given, as a step: Undo and Redo name the fonts as they were and are (the first Redo is a no-op: the step is pushed after the fonts are renamed).
class FontsRenamed : public QUndoCommand {
public:
    FontsRenamed(EditSession *session, QVector<QPair<quint32, QString>> before, QString from, QString to, EditSession::FontText fontText, const QString &text)
        : QUndoCommand(text), m_session(session), m_before(std::move(before)), m_from(std::move(from)), m_to(std::move(to)), m_text(std::move(fontText)) {}
    void redo() override
    {
        if (m_first) {
            m_first = false;
            return;
        }
        QVector<QPair<quint32, QString>> now;
        for (const auto &f : std::as_const(m_before)) now.append({f.first, m_to});
        if (!m_session->setFontNames(now, m_from, m_to, true, &m_text)) m_session->giveUpUndo(text(), true);
    }
    void undo() override
    {
        if (!m_session->setFontNames(m_before, m_from, m_to, false, &m_text)) m_session->giveUpUndo(text(), false);
    }

private:
    EditSession *m_session;
    QVector<QPair<quint32, QString>> m_before;
    QString m_from, m_to;
    EditSession::FontText m_text;
    bool m_first = true;
};

// The theme's heading or body font given another name, as a step.
class ThemeFontRenamed : public QUndoCommand {
public:
    ThemeFontRenamed(EditSession *session, bool major, QString before, QString to, const QString &text)
        : QUndoCommand(text), m_session(session), m_major(major), m_before(std::move(before)), m_to(std::move(to)) {}
    void redo() override
    {
        if (m_first) {
            m_first = false;
            return;
        }
        if (!m_session->setThemeFontName(m_major, m_to)) m_session->giveUpUndo(text(), true);
    }
    void undo() override
    {
        if (!m_session->setThemeFontName(m_major, m_before)) m_session->giveUpUndo(text(), false);
    }

private:
    EditSession *m_session;
    bool m_major;
    QString m_before, m_to;
    bool m_first = true;
};

// A theme as a step holds it: its colors and names, and its part as a buffer shared with every other step that names the same theme (the part may be up to 4 MB).
struct HeldTheme {
    jp::office::Theme theme;   // (its part is not here, but in `xml`)
    std::shared_ptr<const QByteArray> xml;
    jp::office::Theme get() const
    {
        jp::office::Theme t = theme;
        if (xml) t.xml = *xml;
        return t;
    }
};

// The workbook's theme given another, as a step: Undo and Redo give the whole theme as it was and is (the first Redo is a no-op: the step is pushed after the theme is set).
class ThemeChanged : public QUndoCommand {
public:
    ThemeChanged(EditSession *session, HeldTheme before, HeldTheme after, const QString &text)
        : QUndoCommand(text), m_session(session), m_before(std::move(before)), m_after(std::move(after)) {}
    void redo() override
    {
        if (m_first) {
            m_first = false;
            return;
        }
        if (!m_session->setThemeNow(m_after.get())) m_session->giveUpUndo(text(), true);
    }
    void undo() override
    {
        if (!m_session->setThemeNow(m_before.get())) m_session->giveUpUndo(text(), false);
    }

private:
    EditSession *m_session;
    HeldTheme m_before, m_after;
    bool m_first = true;
};

std::shared_ptr<const QByteArray> EditSession::sharedThemePart(const QByteArray &xml)
{
    if (xml.isEmpty()) return nullptr;
    // (the buffers the steps hold are looked at by what they say: a theme set again is the buffer already held, not another; a buffer no step holds any more is let go)
    std::shared_ptr<const QByteArray> found;
    for (auto it = m_themeParts.begin(); it != m_themeParts.end();) {
        const std::shared_ptr<const QByteArray> held = it->lock();
        if (!held) {
            it = m_themeParts.erase(it);
            continue;
        }
        if (!found && held->size() == xml.size() && *held == xml) found = held;
        ++it;
    }
    if (found) return found;
    found = std::make_shared<const QByteArray>(xml);
    m_themeParts.push_back(found);
    return found;
}

int EditSession::themePartsHeld() const
{
    int n = 0;
    for (const std::weak_ptr<const QByteArray> &p : m_themeParts)
        if (!p.expired()) ++n;
    return n;
}

bool EditSession::hasTheme() const { return m_book && m_book->book && m_book->book->hasTheme(); }

jp::office::Theme EditSession::theme() const
{
    if (!m_book || !m_book->book) return jp::office::Theme();
    std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
    return hold ? m_book->book->theme() : jp::office::Theme();
}

bool EditSession::setThemeNow(const jp::office::Theme &theme)
{
    {
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold || !m_book->book->setTheme(theme)) return false;
    }
    Q_EMIT themeChanged();
    Q_EMIT stylesChanged();
    return true;
}

EditSession::Outcome EditSession::applyTheme(const jp::office::Theme &theme, const QString &text)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (!m_book || !m_book->book || !m_book->calc) return out;
    jp::office::Theme was;
    {
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) {
            out.ok = false;
            out.problem = tr("The workbook is busy. Try again in a moment.");
            return out;
        }
        if (!m_book->book->hasTheme()) {
            out.ok = false;
            out.problem = tr("This workbook has no theme part, so it can't take a theme. Nothing was changed.");
            return out;
        }
        was = m_book->book->theme();
        bool same = was.xml == theme.xml && was.majorLatin == theme.majorLatin && was.minorLatin == theme.minorLatin;
        for (int i = 0; same && i < jp::office::Theme::ColorCount; ++i) same = was.colors[i] == theme.colors[i];
        if (same) return out;
        if (!m_book->book->setTheme(theme)) {
            out.ok = false;
            out.problem = tr("The theme could not be applied to this workbook. Nothing was changed.");
            return out;
        }
    }
    m_lastCalculated = false;
    auto held = [this](const jp::office::Theme &t) {
        HeldTheme h;
        h.xml = sharedThemePart(t.xml);
        h.theme = t;
        h.theme.xml.clear();
        return h;
    };
    m_undo.push(new ThemeChanged(this, held(was), held(theme), text));
    Q_EMIT themeChanged();
    Q_EMIT stylesChanged();
    return out;
}

bool EditSession::setThemeFontName(bool major, const QString &name)
{
    std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
    if (!hold) return false;
    m_book->book->setThemeFont(major, name);
    Q_EMIT stylesChanged();
    return true;
}

bool EditSession::setFontNames(const QVector<QPair<quint32, QString>> &fonts, const QString &from, const QString &to, bool doing, const FontText *text)
{
    std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
    if (!hold) return false;
    for (const auto &f : fonts) m_book->book->setFontName(f.first, f.second);
    QStringList changedParts;
    if (text) {
        // (the parts, and the runs of rich text, as they were or are: what is made from them next shows it)
        m_book->book->setPartBytes(doing ? text->partsAfter : text->partsBefore);
        for (const auto &p : (doing ? text->partsAfter : text->partsBefore)) changedParts << p.first;
        if (doing) {
            QVector<QPair<quint32, QString>> named;
            for (const auto &f : text->shared) named.append({f.first, text->to});
            m_book->book->setSharedRichFontNames(named);
            QVector<std::tuple<int, quint32, QString>> inSheets;
            for (const auto &f : text->sheets) inSheets.append({std::get<0>(f), std::get<1>(f), text->to});
            m_book->calc->setRichFontNames(inSheets);
        } else {
            m_book->book->setSharedRichFontNames(text->shared);
            m_book->calc->setRichFontNames(text->sheets);
        }
    }
    // (and the fonts the text of the parts names: written into them when the workbook is saved)
    if (doing) m_book->book->addFontTextRename(from, to);
    else m_book->book->undoFontTextRename(from, to);
    Q_EMIT stylesChanged();
    if (text) Q_EMIT fontTextChanged(changedParts);
    return true;
}

QMap<QString, qint64> EditSession::cellFontsUsed(QMap<QString, qint64> *stylesOnly, QMap<QString, QString> *theme, QMap<QString, qint64> *themeCells) const
{
    QMap<QString, qint64> out;
    if (!m_book || !m_book->calc || !m_book->book) return out;
    const jp::office::sheet::Styles &styles = m_book->book->styles();
    QHash<quint32, QString> seen;
    for (int sheet = 0; sheet < m_book->calc->sheetCount(); ++sheet) {
        m_book->calc->sheet(sheet).cells.forEachInRange(0, 0, kMaxRows - 1, kMaxColumns - 1, [&](quint32, quint32, const Cell &c) {
            const QString name = fontNamed(styles, c.style, &seen);
            if (!name.isEmpty()) ++out[name];
            else if (themeCells) {
                const QString& scheme = styles.font(styles.format(c.style).font).scheme;
                if (scheme == QLatin1String("major")) ++(*themeCells)[QStringLiteral("+Heading")];
                else if (scheme == QLatin1String("minor")) ++(*themeCells)[QStringLiteral("+Body")];
            }
            return true;
        });
    }
    {
        // (the fonts the text of shared strings, shapes, charts, and comments names)
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (hold) {
            const QMap<QString, qint64> inParts = m_book->book->partFontsUsed();
            for (auto it = inParts.constBegin(); it != inParts.constEnd(); ++it) out[it.key()] += it.value();
            // (the runs of inline strings: the fonts they name now)
            for (int sheet = 0; sheet < m_book->calc->sheetCount(); ++sheet)
                for (const jp::office::sheet::FontSpec &f : m_book->calc->sheet(sheet).richFonts)
                    if (f.scheme.isEmpty() && !f.name.isEmpty()) ++out[f.name];
        }
    }
    if (theme) {
        if (!styles.majorFont.isEmpty()) theme->insert(QStringLiteral("+Heading"), styles.majorFont);
        if (!styles.minorFont.isEmpty()) theme->insert(QStringLiteral("+Body"), styles.minorFont);
    }
    if (stylesOnly) {
        // The named styles (the style formats, the Normal style first among them) and the formats they are among: a font only they use is a font of styles.
        auto add = [&](quint32 fontIndex) {
            const jp::office::sheet::FontSpec &f = styles.font(fontIndex);
            if (f.scheme.isEmpty() && !f.name.isEmpty()) ++(*stylesOnly)[f.name];
        };
        for (const jp::office::sheet::CellFormat &x : styles.styleXfs) add(x.font);
        if (styles.fonts.size() > 0) add(0);   // (the default font: the first of the file's)
    }
    return out;
}

EditSession::Outcome EditSession::replaceCellFont(const QString &from, const QString &to, qint64 *cells)
{
    Outcome out;
    if (cells) *cells = 0;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (to.size() > jp::office::sheet::kMaxFontNameFileChars) {
        out.ok = false;
        out.problem = tr("A font's name can have up to %1 characters in a workbook, and this one has %2. Choose another font.").arg(jp::office::sheet::kMaxFontNameFileChars).arg(to.size());
        return out;
    }
    if (from.isEmpty() || to.isEmpty() || from.compare(to, Qt::CaseInsensitive) == 0 || !m_book || !m_book->calc || !m_book->book) return out;
    if (from == QLatin1String("+Heading") || from == QLatin1String("+Body")) {
        // The theme's own font: changed only when it is the one picked (the cells that follow it keep following it).
        const bool major = from == QLatin1String("+Heading");
        QString was;
        {
            std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
            if (!hold) {
                out.ok = false;
                out.problem = tr("The workbook is busy. Try again in a moment.");
                return out;
            }
            was = major ? m_book->book->styles().majorFont : m_book->book->styles().minorFont;
            if (was.isEmpty() || was == to) return out;
            m_book->book->setThemeFont(major, to);
        }
        QMap<QString, qint64> themeCells;
        cellFontsUsed(nullptr, nullptr, &themeCells);
        if (cells) *cells = std::max<qint64>(themeCells.value(from), 1);
        m_lastCalculated = false;
        m_undo.push(new ThemeFontRenamed(this, major, was, to, tr("Replace Fonts")));
        Q_EMIT stylesChanged();
        return out;
    }
    // A sheet with cells in the font that won't take formatting refuses the whole change, before anything changes.
    {
        const jp::office::sheet::Styles &styles = m_book->book->styles();
        QHash<quint32, QString> seen;
        for (int sheet = 0; sheet < m_book->calc->sheetCount(); ++sheet) {
            bool uses = false;
            m_book->calc->sheet(sheet).cells.forEachInRange(0, 0, kMaxRows - 1, kMaxColumns - 1, [&](quint32, quint32, const Cell &c) {
                if (fontNamed(styles, c.style, &seen).compare(from, Qt::CaseInsensitive) != 0) return true;
                uses = true;
                if (cells) ++*cells;
                return true;
            });
            if (!uses) continue;
            if (QString notAllowed; !canEditSheet(sheet, &notAllowed) || !allows(sheet, Allow::FormatCells, &notAllowed)) {
                out.ok = false;
                out.problem = notAllowed;
                if (cells) *cells = 0;
                return out;
            }
        }
    }
    QVector<QPair<quint32, QString>> before;
    qint64 inParts = 0;
    QString named;
    QStringList changedNow;
    FontText fontText;
    {
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) {
            out.ok = false;
            out.problem = tr("The workbook is busy. Try again in a moment.");
            if (cells) *cells = 0;
            return out;
        }
        before = m_book->book->renameFonts(from, to);
        // (what the text of the parts names, counted before it is renamed)
        const QMap<QString, qint64> parts = m_book->book->partFontsUsed();
        for (auto it = parts.constBegin(); it != parts.constEnd(); ++it)
            if (it.key().compare(from, Qt::CaseInsensitive) == 0) {
                inParts += it.value();
                if (named.isEmpty()) named = it.key();
            }
        // (the parts and the runs of rich text, in memory as well: the window shows them at once)
        fontText.partsBefore = m_book->book->renameFontsInParts(from, to);
        for (const auto &p : std::as_const(fontText.partsBefore)) {
            fontText.partsAfter.append({p.first, jp::office::sheet::renameFontsInText(p.second, from, to)});
            changedNow << p.first;
        }
        fontText.shared = m_book->book->renameSharedRichFonts(from, to);
        fontText.sheets = m_book->calc->renameRichFonts(from, to);
        fontText.to = to;
        inParts += fontText.sheets.size();
        // (Undo gives the name back as the file spells it, not as it was typed)
        if (!before.isEmpty()) named = before.first().second;
        else if (named.isEmpty() && !fontText.sheets.isEmpty()) named = std::get<2>(fontText.sheets.first());
        if (named.isEmpty()) named = from;
        if (inParts > 0) m_book->book->addFontTextRename(named, to);
    }
    if (before.isEmpty() && inParts == 0 && fontText.partsBefore.isEmpty() && fontText.shared.isEmpty() && fontText.sheets.isEmpty()) return out;
    if (cells) *cells += inParts;
    m_lastCalculated = false;
    m_undo.push(new FontsRenamed(this, before, named, to, std::move(fontText), tr("Replace Fonts")));
    Q_EMIT stylesChanged();
    Q_EMIT fontTextChanged(changedNow);
    return out;
}

EditSession::Outcome EditSession::applyStyle(int sheet, const CellRange &range, quint32 style, const QString &text)
{
    FormatChange c;
    c.namedStyle = style;
    return formatCells(sheet, range, c, text);
}

EditSession::Outcome EditSession::applyStyleNamed(int sheet, const CellRange &range, const QString &name, const QString &text)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    qint32 place = -1;
    {
        // (the style is made with the worker held back: it reads the workbook's styles)
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) {
            out.ok = false;
            out.problem = tr("The workbook is busy. Try again in a moment.");
            return out;
        }
        place = m_book->book->styleNamed(name);
    }
    if (place < 0) {
        out.ok = false;
        out.problem = tr("This workbook has no place to keep the style \"%1\".").arg(name);
        return out;
    }
    return applyStyle(sheet, range, quint32(place), text);
}

EditSession::Outcome EditSession::newCellStyle(int sheet, quint32 row, quint32 col, const CellRange &range, const QString &name)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    const QString trimmed = name.trimmed();
    bool control = false;
    for (const QChar c : trimmed) control = control || c.category() == QChar::Other_Control;
    if (trimmed.isEmpty() || trimmed.size() > 255 || control) {
        out.ok = false;
        out.problem = tr("Type a name for the style, up to 255 characters, with no control characters.");
        return out;
    }
    qint32 place = -1;
    bool taken = false;
    quint32 fromFormat = 0;
    {
        // (the style is made with the worker held back: it reads the workbook's styles)
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) {
            out.ok = false;
            out.problem = tr("The workbook is busy. Try again in a moment.");
            return out;
        }
        for (const jp::office::sheet::CellStyleName &n : m_book->book->styles().cellStyles)
            if (n.name.compare(trimmed, Qt::CaseInsensitive) == 0) taken = true;
        jp::office::sheet::BuiltinStyle def;
        if (jp::office::sheet::builtinStyle(trimmed, &def)) taken = true;   // (a name of the built-in styles is theirs, whether or not the workbook has it yet)
        if (!taken) {
            fromFormat = m_book->calc->content(sheet, row, col).style;
            place = m_book->book->addNamedStyle(trimmed, fromFormat);
        }
    }
    if (place < 0) {
        out.ok = false;
        out.problem = taken ? tr("A style called \"%1\" is already there. Pick another name.").arg(trimmed) : tr("This workbook has no place to keep a new style.");
        return out;
    }
    // (one step: the style is made, and the cells are given it; Undo takes both back)
    m_undo.beginMacro(tr("New Cell Style"));
    pushStyleMade(trimmed, fromFormat);
    out = applyStyle(sheet, range, quint32(place), tr("New Cell Style"));
    m_undo.endMacro();
    if (!out.ok) m_undo.undo();   // (the cells could not be given it: the style goes too)
    return out;
}

EditSession::Outcome EditSession::deleteCellStyle(const QString &name)
{
    Outcome out;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    QString why;
    if (!canEdit(&why)) return refuse(why);
    const QString trimmed = name.trimmed();
    qint32 place = -1, normal = -1;
    struct Run {
        int sheet;
        CellRange range;
    };
    std::vector<Run> runs;
    {
        // (read with the worker held back: it reads the workbook's styles)
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) return refuse(tr("The workbook is busy. Try again in a moment."));
        const jp::office::sheet::Styles &styles = m_book->book->styles();
        bool found = false;
        for (const jp::office::sheet::CellStyleName &n : styles.cellStyles)
            if (n.name.compare(trimmed, Qt::CaseInsensitive) == 0 && !n.hidden) {
                found = true;
                place = qint32(n.xfId);
                if (n.builtin == 0) return refuse(tr("The Normal style can't be deleted. Every cell without a style of its own uses it."));
            }
        if (!found) return refuse(tr("This workbook has no style called \"%1\".").arg(trimmed));
        for (const jp::office::sheet::CellStyleName &n : styles.cellStyles)
            if (n.builtin == 0) normal = qint32(n.xfId);
        if (normal < 0 || normal == place) return refuse(tr("This workbook has no Normal style to return the cells to."));
        for (int sheet = 0; sheet < m_book->calc->sheetCount(); ++sheet) {
            const jp::office::sheet::Sheet &model = m_book->calc->sheet(sheet);
            if (model.extentRows == 0 || model.extentCols == 0) continue;
            quint32 lastRow = model.extentRows - 1, lastCol = model.extentCols - 1;
            Run open{sheet, CellRange()};
            quint32 openRow = 0;
            model.cells.forEachInRange(0, 0, lastRow, lastCol, [&](quint32 row, quint32 col, const jp::office::sheet::Cell &) {
                const quint32 style = m_book->calc->content(sheet, row, col).style;
                if (styles.format(style).xfId != quint32(place)) return true;
                if (open.range.valid && openRow == row && open.range.col2 + 1 == col) {
                    open.range.col2 = col;
                    return true;
                }
                if (open.range.valid) runs.push_back(open);
                open.range.valid = true;
                open.range.row1 = open.range.row2 = row;
                open.range.col1 = open.range.col2 = col;
                openRow = row;
                return runs.size() < 5000;
            });
            if (open.range.valid) runs.push_back(open);
            if (runs.size() >= 5000) return refuse(tr("That style is used by too many cells to take out at once (more than 5,000 runs of cells)."));
        }
        m_book->book->setStyleHidden(trimmed, true);   // (hidden now, so that nothing reads it again; Undo shows it)
    }
    m_undo.beginMacro(tr("Delete Cell Style"));
    pushStyleHidden(trimmed);
    for (const Run &r : runs) {
        out = applyStyle(r.sheet, r.range, quint32(normal), tr("Delete Cell Style"));
        if (!out.ok) break;
    }
    m_undo.endMacro();
    if (!out.ok) m_undo.undo();   // (the cells could not all be changed: the style stays)
    return out;
}

EditSession::Outcome EditSession::clearFormats(int sheet, const CellRange &range)
{
    return restyleRange(sheet, range, nullptr, tr("Clearing formats"));
}

EditSession::Outcome EditSession::setBorders(int sheet, const CellRange &range, int edges, const BorderSide &line, const QString &text)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (!range.valid || range.row2 >= kMaxRows || range.col2 >= kMaxColumns) {
        out.ok = false;
        out.problem = tr("Those are not cells of the sheet.");
        return out;
    }
    out.range = range;
    CellRange r = range;
    // (a selection of whole columns or rows gets lines in its used cells only)
    const Sheet &s = m_book->calc->sheet(sheet);
    if (r.row2 == kMaxRows - 1) r.row2 = std::min<quint32>(r.row2, std::max(r.row1, s.extentRows > 0 ? s.extentRows - 1 : 0));
    if (r.col2 == kMaxColumns - 1) r.col2 = std::min<quint32>(r.col2, std::max(r.col1, s.extentCols > 0 ? s.extentCols - 1 : 0));
    const qint64 count = (qint64(r.row2) - r.row1 + 1) * (qint64(r.col2) - r.col1 + 1);
    if (count > kMaxCellsPerEdit) {
        out.ok = false;
        out.problem = tr("That is more cells than JeffCrunch draws borders around at once (%1). Select fewer cells.").arg(QLocale().toString(kMaxCellsPerEdit));
        return out;
    }
    std::map<std::pair<quint32, quint32>, std::shared_ptr<FormatChange>> changes;
    auto at = [&](quint32 row, quint32 col) -> FormatChange & {
        auto &p = changes[{row, col}];
        if (!p) p = std::make_shared<FormatChange>();
        return *p;
    };
    for (quint32 row = r.row1; row <= r.row2; ++row)
        for (quint32 col = r.col1; col <= r.col2; ++col) {
            const bool first = col == r.col1, last = col == r.col2, top = row == r.row1, bottom = row == r.row2;
            FormatChange *c = nullptr;
            auto change = [&]() -> FormatChange & { return c ? *c : *(c = &at(row, col)); };
            if (first ? (edges & EdgeLeft) : (edges & EdgeInsideVertical)) change().left = line;
            if (last ? (edges & EdgeRight) : (edges & EdgeInsideVertical)) change().right = line;
            if (top ? (edges & EdgeTop) : (edges & EdgeInsideHorizontal)) change().top = line;
            if (bottom ? (edges & EdgeBottom) : (edges & EdgeInsideHorizontal)) change().bottom = line;
            if (edges & EdgeDiagonalUp) {
                change().diagonal = line;
                change().diagonalUp = line.style != BorderStyle::None;
            }
            if (edges & EdgeDiagonalDown) {
                change().diagonal = line;
                change().diagonalDown = line.style != BorderStyle::None;
            }
            // the cells beside the block meet its outer edges: the line is on their side too
            if (first && col > 0 && (edges & EdgeLeft)) at(row, col - 1).right = line;
            if (last && col + 1 < kMaxColumns && (edges & EdgeRight)) at(row, col + 1).left = line;
            if (top && row > 0 && (edges & EdgeTop)) at(row - 1, col).bottom = line;
            if (bottom && row + 1 < kMaxRows && (edges & EdgeBottom)) at(row + 1, col).top = line;
        }
    std::vector<RestyleTarget> targets;
    targets.reserve(changes.size());
    for (const auto &[place, change] : changes) targets.push_back({place.first, place.second, change.get()});
    Outcome result = restyle(sheet, targets, text);
    result.range = range;
    return result;
}

EditSession::Outcome EditSession::clearAll(int sheet, const CellRange &range)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (!range.valid) return out;
    // the contents: Clear's step (the cells' formats stay in it), then the formats are cleared too, in one step
    const Calculation *calc = m_book->calc.get();
    const Sheet &s = calc->sheet(sheet);
    std::vector<CellChange> changes;
    bool array = false, locked = false, many = false;
    s.cells.forEachInRange(range.row1, range.col1, range.row2, range.col2, [&](quint32 cellRow, quint32 cellCol, const Cell &c) {
        const Calculation::CellContent before = calc->content(sheet, cellRow, cellCol);
        if (!before.editable) {
            array = true;
            return false;
        }
        if (isLocked(sheet, before.style, cellRow, cellCol)) {
            locked = true;
            return false;
        }
        if (m_book->book->tableHeaderAt(sheet, cellRow, cellCol, nullptr) >= 0 && (before.isFormula || !before.value.isEmpty())) {
            locked = false;
        }
        if (qint64(changes.size()) >= kMaxCellsPerEdit) {
            many = true;
            return false;
        }
        CellChange ch;
        ch.sheet = sheet;
        ch.row = cellRow;
        ch.col = cellCol;
        ch.before = before;
        ch.after = Calculation::CellContent();   // nothing: no value, no formula, the first format
        changes.push_back(std::move(ch));
        return true;
    });
    if (array || locked || many) {
        out.ok = false;
        out.problem = array ? tr("You can't change part of an array or a data table.") : locked ? lockedMessage()
                                                                                                : tr("That is more cells than JeffCrunch clears at once (%1). Select fewer cells.").arg(QLocale().toString(kMaxCellsPerEdit));
        return out;
    }
    Step step;
    step.cells = std::move(changes);
    // whole columns and rows lose their formats too
    if (range.row1 == 0 && range.row2 == kMaxRows - 1) {
        SizeEdit e;
        e.sheet = sheet;
        e.columns = true;
        e.first = range.col1;
        e.last = range.col2;
        e.columnsBefore = calc->columnRuns(sheet, range.col1, range.col2);
        for (ColumnSpan c : e.columnsBefore) {
            c.style = 0;
            if (c.width >= 0 || c.hidden) e.columnsAfter.push_back(c);
        }
        step.sizes.push_back(std::move(e));
    } else if (range.col1 == 0 && range.col2 == kMaxColumns - 1 && qint64(range.row2) - range.row1 < kMaxRowsPerResize) {
        SizeEdit e;
        e.sheet = sheet;
        e.columns = false;
        e.first = range.row1;
        e.last = range.row2;
        e.rowsBefore = calc->rowSpecsIn(sheet, range.row1, range.row2);
        for (RowSpec r : e.rowsBefore) {
            r.style = 0;
            r.customFormat = false;
            if (r.height >= 0 || r.hidden || r.customHeight) e.rowsAfter.push_back(r);
        }
        step.sizes.push_back(std::move(e));
    }
    // the notes, the comments, and the links on the cells go with them (a thread with more comments than are read is kept, and the person is told)
    if (QString notReady; canEditNotes(sheet, &notReady) && allows(sheet, Allow::Objects)) {
        int keptCut = 0;
        SideEdit side = notesAndLinksIn(sheet, range, &keptCut);
        if (!side.empty()) step.sides.push_back(std::move(side));
        if (keptCut) out.problem = tr("The comments on some cells have more replies than JeffCrunch reads (%1), so they were not cleared: an Undo could not bring back what it never read.").arg(jp::office::sheet::kMaxThreadComments);
    }
    if (step.empty()) return out;
    pushStep(std::move(step), tr("Clearing all"));
    return out;
}

EditSession::Outcome EditSession::mergeCells(int sheet, const CellRange &range, MergeKind kind, bool discard)
{
    Outcome out;
    QString why;
    auto refuse = [&](const QString &problem) {
        out.ok = false;
        out.problem = problem;
        return out;
    };
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) return refuse(why);
    if (!range.valid || range.row2 >= kMaxRows || range.col2 >= kMaxColumns) return refuse(tr("Those are not cells of the sheet."));
    out.range = range;
    Calculation *calcEdit = m_book->calc.get();
    const Calculation *calc = calcEdit;
    const Sheet &s = calc->sheet(sheet);
    if (s.isProtected) return refuse(tr("This sheet is protected, so cells can't be merged or unmerged on it. To do that, choose Unprotect Sheet on the Review tab."));
    if (calc->mergeCountOf(sheet) != qint64(calc->mergesOf(sheet).size())) return refuse(tr("This sheet has more merged cells than JeffCrunch changes. Merge and unmerge them in the program that made it."));
    const QVector<CellRange> before = calc->mergesOf(sheet);
    auto meets = [](const CellRange &a, const CellRange &b) { return !(a.row2 < b.row1 || a.row1 > b.row2 || a.col2 < b.col1 || a.col1 > b.col2); };
    QVector<CellRange> after;
    std::vector<CellRange> blocks;
    if (kind == MergeKind::Unmerge) {
        for (const CellRange &m : before)
            if (!meets(m, range)) after.append(m);
    } else {
        if (range.row1 == range.row2 && range.col1 == range.col2) return out;   // (one cell)
        if (kind == MergeKind::Across) {
            if (range.col1 == range.col2) return out;
            for (quint32 r = range.row1; r <= range.row2; ++r) {
                CellRange one = range;
                one.row1 = one.row2 = r;
                blocks.push_back(one);
            }
        } else {
            blocks.push_back(range);
        }
        if (qint64(blocks.size()) > kMaxCellsPerEdit) return refuse(tr("That is more rows than JeffCrunch merges at once. Select fewer rows."));
        for (const CellRange &m : before) {
            bool taken = false;
            for (const CellRange &b : blocks) taken = taken || meets(m, b);
            if (!taken) after.append(m);
        }
        for (const CellRange &b : blocks) after.append(b);
        if (after.size() > kMaxMerges) return refuse(tr("That would make more merged cells than JeffCrunch makes."));
    }
    // the cells that lose their contents (all but the first of each block)
    Step step;
    bool needAsk = false;
    qint64 emptied = 0;
    for (const CellRange &b : blocks) {
        qint64 seen = 0;
        bool bad = false;
        s.cells.forEachInRange(b.row1, b.col1, b.row2, b.col2, [&](quint32 cellRow, quint32 cellCol, const Cell &c) {
            if (++seen > kMaxCellsPerEdit) return false;
            const Calculation::CellContent content = calc->content(sheet, cellRow, cellCol);
            if (!content.editable) {
                bad = true;
                return false;
            }
            if (cellRow == b.row1 && cellCol == b.col1) return true;
            if (!content.isFormula && content.value.isEmpty()) return true;
            needAsk = true;
            if (isLocked(sheet, content.style, cellRow, cellCol)) {
                bad = true;
                return false;
            }
            if (++emptied > kMaxCellsPerEdit) return false;
            if (!discard) return true;
            CellChange ch;
            ch.sheet = sheet;
            ch.row = cellRow;
            ch.col = cellCol;
            ch.before = content;
            ch.after = content;
            ch.after.isFormula = false;
            ch.after.formula.clear();
            ch.after.value = Value();
            ch.after.sharedString = -1;
            step.cells.push_back(std::move(ch));
            return true;
        });
        if (bad) return refuse(tr("You can't merge part of an array or a data table, or a locked cell."));
    }
    if (needAsk && !discard) {
        out.ok = false;
        out.confirm = true;
        out.problem = tr("When cells are joined into one, only what the cell at the top left holds is kept, and what the others hold is lost. Join them anyway?");
        return out;
    }
    MergeEdit m;
    m.sheet = sheet;
    m.before = before;
    m.after = after;
    m.countBefore = calc->mergeCountOf(sheet);
    m.countAfter = after.size();
    if (!(m.after == m.before)) step.merges.push_back(std::move(m));
    if (kind == MergeKind::MergeAndCenter) {
        // the merged cell is centered
        auto change = std::make_shared<FormatChange>();
        change->horizontal = HAlign::Center;
        std::vector<RestyleTarget> targets;
        for (const CellRange &b : blocks) targets.push_back({b.row1, b.col1, change.get()});
        // (the format is made first, then the contents and the merges go in the same step)
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) return refuse(tr("The workbook is busy. Try again in a moment."));
        Workbook &book = *m_book->book;
        for (const RestyleTarget &t : targets) {
            const Calculation::CellContent content = hold->calculation()->content(sheet, t.row, t.col);
            const quint32 style = book.formatWith(content.style, *t.change);
            if (style == content.style && content.exists) continue;
            bool known = false;
            for (CellChange &ch : step.cells)
                if (ch.row == t.row && ch.col == t.col) {
                    ch.after.style = style;
                    known = true;
                }
            if (known) continue;
            CellChange ch;
            ch.sheet = sheet;
            ch.row = t.row;
            ch.col = quint16(t.col);
            ch.before = content;
            ch.after = content;
            ch.after.exists = true;
            ch.after.style = style;
            ch.styleOnly = true;
            step.cells.push_back(std::move(ch));
        }
    }
    if (step.empty()) return out;
    pushStep(std::move(step), kind == MergeKind::Unmerge ? tr("Unmerging cells") : kind == MergeKind::Across ? tr("Merging across") : kind == MergeKind::MergeAndCenter ? tr("Merging and centering") : tr("Merging cells"));
    return out;
}

// The look of a style changed (Modify, Merge Styles): the workbook has it already when the step is pushed (the first Redo does nothing); Undo gives the old look back, Redo the new one again.
class StyleLookChanged : public QUndoCommand {
public:
    StyleLookChanged(EditSession *session, quint32 xfId, const CellFormat &before, const CellFormat &after, const QString &text)
        : QUndoCommand(text), m_session(session), m_xfId(xfId), m_before(before), m_after(after) {}
    void redo() override
    {
        if (m_first) {
            m_first = false;
            return;
        }
        if (!m_session->setStyleLookNow(m_xfId, m_after)) m_session->giveUpUndo(text(), true);
    }
    void undo() override
    {
        if (!m_session->setStyleLookNow(m_xfId, m_before)) m_session->giveUpUndo(text(), false);
    }

private:
    EditSession *m_session;
    quint32 m_xfId;
    CellFormat m_before, m_after;
    bool m_first = true;
};

bool EditSession::setStyleLookNow(quint32 xfId, const CellFormat &look)
{
    std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
    if (!hold) return false;
    const bool ok = m_book->book->setStyleLook(xfId, look);
    if (ok) Q_EMIT stylesChanged();
    return ok;
}

// What follows the styles that are changed: the cells that hold something, and the formats of the columns and rows, each with the part of the change it takes. Made with the worker held back,
// before anything is changed.
struct EditSession::FollowPlan {
    struct Sheet {
        int sheet = 0;
        std::vector<RestyleTarget> targets;
        std::vector<std::pair<ColumnSpan, std::shared_ptr<const FormatChange>>> columns;
        std::vector<std::pair<RowSpec, std::shared_ptr<const FormatChange>>> rows;
    };
    std::vector<Sheet> sheets;
    std::vector<std::shared_ptr<const FormatChange>> owned;
};

// The cells, columns, and rows that follow the styles, each with what it takes of the change: the fields of the change in which it has what the style had (a field it has another value in is
// its own, and stays). False, with the reason, for a sheet that cannot be formatted, or more cells than are changed at once.
bool EditSession::followersOf(const std::vector<StyleChange> &styles, FollowPlan *plan, QString *why) const
{
    const Styles &st = m_book->book->styles();
    std::map<quint32, size_t> byPlace;
    for (size_t i = 0; i < styles.size(); ++i) byPlace[styles[i].place] = i;
    std::map<std::pair<size_t, quint32>, std::shared_ptr<const FormatChange>> made;
    auto changeFor = [&](quint32 format) -> std::shared_ptr<const FormatChange> {
        const CellFormat &f = st.format(format);
        const auto found = byPlace.find(f.xfId);
        if (found == byPlace.end()) return nullptr;
        const auto key = std::make_pair(found->second, format);
        const auto seen = made.find(key);
        if (seen != made.end()) return seen->second;
        FormatChange c = inheritedFields(st, f, styles[found->second].before, styles[found->second].change);
        std::shared_ptr<const FormatChange> out = formatIsEmpty(c) ? nullptr : std::make_shared<const FormatChange>(c);
        made[key] = out;
        if (out) plan->owned.push_back(out);
        return out;
    };
    qint64 total = 0;
    const Calculation &calc = *m_book->calc;
    for (int sheet = 0; sheet < calc.sheetCount(); ++sheet) {
        const jp::office::sheet::Sheet &model = calc.sheet(sheet);
        FollowPlan::Sheet one;
        one.sheet = sheet;
        bool tooMany = false;
        if (model.extentRows > 0 && model.extentCols > 0)
            model.cells.forEachInRange(0, 0, model.extentRows - 1, model.extentCols - 1, [&](quint32 row, quint32 col, const jp::office::sheet::Cell &) {
                if (const auto c = changeFor(calc.content(sheet, row, col).style)) {
                    one.targets.push_back({row, col, c.get()});
                    tooMany = ++total > kMaxStyledCells;
                }
                return !tooMany;
            });
        for (const ColumnSpan &c : calc.columnRuns(sheet, 0, kMaxColumns - 1))
            if (c.style != 0)
                if (const auto change = changeFor(c.style)) {
                    one.columns.push_back({c, change});
                    tooMany = tooMany || ++total > kMaxStyledCells;
                }
        for (const RowSpec &r : calc.rowSpecsOf(sheet))
            if (r.customFormat && r.style != 0)
                if (const auto change = changeFor(r.style)) {
                    one.rows.push_back({r, change});
                    tooMany = tooMany || ++total > kMaxStyledCells;
                }
        if (tooMany) {
            *why = tr("That style is used by too many cells to change at once (more than %1).").arg(QLocale().toString(kMaxStyledCells));
            return false;
        }
        if (one.targets.empty() && one.columns.empty() && one.rows.empty()) continue;
        if (!canEditSheet(sheet, why) || !allows(sheet, Allow::FormatCells, why)) return false;
        if (model.isProtected) {
            // (what a protected sheet lets be formatted does not include the lock and the hiding of formulas)
            auto lockChange = [](const FormatChange *c) { return c->locked || c->formulaHidden; };
            bool locks = false;
            for (const RestyleTarget &t : one.targets) locks = locks || lockChange(t.change);
            for (const auto &c : one.columns) locks = locks || lockChange(c.second.get());
            for (const auto &r : one.rows) locks = locks || lockChange(r.second.get());
            if (locks) {
                *why = tr("This sheet is protected, so whether its cells are locked, and whether their formulas are hidden, can't be changed. Unprotect the sheet first (Review, Unprotect Sheet).");
                return false;
            }
        }
        plan->sheets.push_back(std::move(one));
    }
    return true;
}

EditSession::Outcome EditSession::restyleFollowers(const FollowPlan &plan, const QString &text)
{
    Outcome out;
    for (const FollowPlan::Sheet &s : plan.sheets) {
        SizeBuilder sizes;
        if (!s.columns.empty() || !s.rows.empty()) {
            sizes = [&s](Workbook &book, const Calculation &calc, Step *step) {
                for (const auto &c : s.columns) {
                    SizeEdit e;
                    e.sheet = s.sheet;
                    e.columns = true;
                    e.first = c.first.first;
                    e.last = c.first.last;
                    e.columnsBefore = calc.columnRuns(s.sheet, e.first, e.last);
                    ColumnSpan after = c.first;
                    after.style = book.formatWith(after.style, *c.second);
                    e.columnsAfter.push_back(after);
                    step->sizes.push_back(std::move(e));
                }
                for (const auto &r : s.rows) {
                    SizeEdit e;
                    e.sheet = s.sheet;
                    e.columns = false;
                    e.first = e.last = r.first.row;
                    e.rowsBefore = calc.rowSpecsIn(s.sheet, e.first, e.last);
                    RowSpec after = r.first;
                    after.style = book.formatWith(after.style, *r.second);
                    after.customFormat = after.style != 0;
                    e.rowsAfter.push_back(after);
                    step->sizes.push_back(std::move(e));
                }
            };
        }
        out = restyle(s.sheet, s.targets, text, sizes);
        if (!out.ok) return out;
    }
    return out;
}

EditSession::Outcome EditSession::modifyCellStyle(const QString &name, const FormatChange &change)
{
    Outcome out;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    QString why;
    if (!canEdit(&why)) return refuse(why);
    if (formatIsEmpty(change)) return out;
    const QString trimmed = name.trimmed();
    quint32 place = 0;
    bool made = false;
    CellFormat before, after;
    FollowPlan plan;
    {
        // (everything that can be refused is looked at before anything is changed)
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) return refuse(tr("The workbook is busy. Try again in a moment."));
        const Styles &styles = m_book->book->styles();
        const CellStyleName *have = nullptr;
        for (const CellStyleName &n : styles.cellStyles)
            if (n.name.compare(trimmed, Qt::CaseInsensitive) == 0) have = &n;
        if (have && have->builtin == 0) return refuse(tr("The Normal style can't be modified here. Every cell without a style of its own uses it."));
        BuiltinStyle def;
        if (have ? (have->hidden || have->xfId >= quint32(styles.styleXfs.size())) : !(builtinStyle(trimmed, &def) && m_book->book->styleRoom() > 0))
            return refuse(tr("This workbook has no style called \"%1\", and no place to keep it.").arg(trimmed));
        if (have) {
            place = have->xfId;
        } else {
            // (a built-in style the workbook has not used yet is made now, as a step of this change; Undo takes it out again)
            place = quint32(m_book->book->styleNamed(trimmed));
            made = true;
        }
        before = styles.styleXfs[int(place)];
        m_book->book->clearFormatLimit();
        after = m_book->book->changedFormat(before, change);
        after.xfId = before.xfId;
        after.parent = before.parent;
        after.isStyle = before.isStyle;
        QString notAllowed;
        bool refused = false;
        if (m_book->book->formatLimitHit()) {
            m_book->book->clearFormatLimit();
            notAllowed = tr("This workbook can't take more formats.");
            refused = true;
        } else if (!made) {
            const std::vector<StyleChange> styleChanges{{place, before, changedFields(styles, before, after)}};
            refused = !followersOf(styleChanges, &plan, &notAllowed);
        }
        if (refused) {
            if (made) m_book->book->removeAddedStyle(trimmed);
            return refuse(notAllowed);
        }
        if (after == before && !made) return out;   // (nothing to change: the style looks so already)
        m_book->book->setStyleLook(place, after);
    }
    const int steps = m_undo.count();
    m_undo.beginMacro(tr("Modify Cell Style"));
    if (made) pushStyleMade(trimmed, 0);
    m_undo.push(new StyleLookChanged(this, place, before, after, tr("Modify Cell Style")));
    out = restyleFollowers(plan, tr("Modify Cell Style"));
    m_undo.endMacro();
    if (!out.ok && m_undo.count() > steps) m_undo.undo();   // (the cells could not all be changed: the style keeps its look)
    Q_EMIT stylesChanged();
    return out;
}

EditSession::StyleLooks EditSession::namedStyleLooks() const
{
    StyleLooks out;
    std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
    if (!hold || !m_book || !m_book->book) {
        out.busy = true;
        return out;
    }
    const Styles &styles = m_book->book->styles();
    out.themeKey = themeKeyOf(styles);
    for (const CellStyleName &n : styles.cellStyles) {
        if (n.hidden || n.builtin == 0 || n.xfId >= quint32(styles.styleXfs.size())) continue;   // (Normal is every workbook's own)
        StyleLook look;
        look.name = n.name;
        look.look = fullFormatChange(styles, styles.styleXfs[int(n.xfId)]);
        look.resolved = withColorsResolved(styles, look.look);
        out.styles.push_back(std::move(look));
    }
    return out;
}

EditSession::Outcome EditSession::mergeStyles(const StyleLooks &sources, Same same, MergeReport *report)
{
    Outcome out;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    MergeReport rep;
    if (report) *report = rep;
    QString why;
    if (!canEdit(&why)) return refuse(why);
    if (sources.styles.size() > size_t(kMaxMergedStyles)) return refuse(tr("There are more styles to merge than JeffCrunch brings in at once (%1). Nothing was merged.").arg(kMaxMergedStyles));
    enum class Kind { Add, Make, Replace };
    struct Item {
        Kind kind = Kind::Add;
        QString name;
        const FormatChange *look = nullptr;
        quint32 place = 0;
        CellFormat before, after;
    };
    std::vector<Item> items;
    std::vector<StyleChange> changes;
    FollowPlan plan;
    {
        // (everything that can be refused is looked at before anything is changed)
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) return refuse(tr("The workbook is busy. Try again in a moment."));
        const Styles &styles = m_book->book->styles();
        const bool sameTheme = sources.themeKey == themeKeyOf(styles);
        std::set<QString> seen;
        for (const StyleLook &s : sources.styles) {
            const QString name = s.name.trimmed();
            // (colors that name a theme are copied as they are only between workbooks that have the same theme; else as the color they show)
            const FormatChange &look = sameTheme ? s.look : s.resolved;
            bool control = false;
            for (const QChar c : name) control = control || c.category() == QChar::Other_Control;
            if (name.isEmpty() || name.size() > 255 || control || formatIsEmpty(look) || !seen.insert(name.toCaseFolded()).second) {
                ++rep.left;
                continue;
            }
            const CellStyleName *have = nullptr;
            for (const CellStyleName &n : styles.cellStyles)
                if (n.name.compare(name, Qt::CaseInsensitive) == 0) have = &n;
            if (have && (have->builtin == 0 || have->hidden || have->xfId >= quint32(styles.styleXfs.size()))) {
                ++rep.left;   // (Normal stays as it is; a style the workbook took out stays out)
                continue;
            }
            Item item;
            item.name = name;
            item.look = &look;
            if (!have) {
                // (a built-in style this workbook has not used yet is made with its usual look, which the style's look then changes)
                BuiltinStyle def;
                item.kind = builtinStyle(name, &def) ? Kind::Make : Kind::Add;
                items.push_back(item);
                continue;
            }
            if (formatMatches(styles, styles.styleXfs[int(have->xfId)], look)) {
                ++rep.alike;
                continue;
            }
            item.kind = Kind::Replace;
            item.place = have->xfId;
            ++rep.conflicts;
            if (same == Same::Keep) {
                ++rep.kept;
                continue;
            }
            items.push_back(item);
        }
        if (report) *report = rep;
        if (rep.conflicts > 0 && same == Same::Ask) {
            out.ok = false;
            out.confirm = true;
            out.problem = QString::number(rep.conflicts);
            return out;
        }
        if (items.empty()) return out;
        qint64 made = 0;
        for (const Item &item : items) made += item.kind == Kind::Replace ? 0 : 1;
        if (made > m_book->book->styleRoom()) return refuse(tr("This workbook has no room for the styles to bring in. Nothing was merged."));
        m_book->book->clearFormatLimit();
        for (Item &item : items) {
            if (item.kind != Kind::Replace) continue;
            item.before = styles.styleXfs[int(item.place)];
            item.after = m_book->book->changedFormat(item.before, *item.look);
            item.after.xfId = item.before.xfId;
            item.after.parent = item.before.parent;
            item.after.isStyle = item.before.isStyle;
            changes.push_back({item.place, item.before, changedFields(styles, item.before, item.after)});
        }
        if (m_book->book->formatLimitHit()) {
            m_book->book->clearFormatLimit();
            return refuse(tr("This workbook can't take more formats. Nothing was merged."));
        }
        QString notAllowed;
        if (!followersOf(changes, &plan, &notAllowed)) return refuse(notAllowed + QLatin1Char(' ') + tr("Nothing was merged."));
    }
    const int steps = m_undo.count();
    auto giveUp = [&](const QString &problem) {
        m_undo.endMacro();
        if (m_undo.count() > steps) m_undo.undo();
        return refuse(problem);
    };
    m_undo.beginMacro(tr("Merge Styles"));
    for (Item &item : items) {
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) return giveUp(tr("The workbook is busy. Try again in a moment.") + QLatin1Char(' ') + tr("Nothing was merged."));
        if (item.kind == Kind::Replace) {
            m_book->book->setStyleLook(item.place, item.after);
            m_undo.push(new StyleLookChanged(this, item.place, item.before, item.after, tr("Merge Styles")));
            ++rep.replaced;
            continue;
        }
        // (a style made here is a step of its own, then its look is a step: Undo takes both back, Redo puts both in again)
        quint32 format = 0;
        qint32 place = -1;
        if (item.kind == Kind::Add) {
            format = m_book->book->formatWith(0, *item.look);
            place = m_book->book->addNamedStyle(item.name, format);
        } else {
            place = m_book->book->styleNamed(item.name);
        }
        if (place < 0) {
            hold.reset();
            return giveUp(tr("This workbook has no room for the style \"%1\". Nothing was merged.").arg(item.name));
        }
        pushStyleMade(item.name, format);
        item.before = m_book->book->styles().styleXfs[place];
        item.after = m_book->book->changedFormat(item.before, *item.look);
        item.after.xfId = item.before.xfId;
        item.after.parent = item.before.parent;
        item.after.isStyle = item.before.isStyle;
        m_book->book->setStyleLook(quint32(place), item.after);
        m_undo.push(new StyleLookChanged(this, quint32(place), item.before, item.after, tr("Merge Styles")));
        ++rep.added;
    }
    out = restyleFollowers(plan, tr("Merge Styles"));
    if (!out.ok) {
        const QString problem = out.problem + QLatin1Char(' ') + tr("Nothing was merged.");
        rep.added = rep.replaced = 0;
        if (report) *report = rep;
        return giveUp(problem);
    }
    m_undo.endMacro();
    if (report) *report = rep;
    Q_EMIT stylesChanged();
    return out;
}

EditSession::Outcome EditSession::removeLinksAndLook(int sheet, const CellRange &range, const QString &stepText)
{
    Outcome out;
    QString why;
    if (!canEditNotes(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    std::vector<CellRange> linked;
    for (const jp::office::sheet::LinkSpec &l : m_anchors.links(sheet))
        if (l.range.row1 <= range.row2 && l.range.row2 >= range.row1 && l.range.col1 <= range.col2 && l.range.col2 >= range.col1) linked.push_back(l.range);
    if (linked.empty()) return out;
    // The cells of those links that follow the Hyperlink style: each goes back to Normal and keeps every other part of its look as direct formatting (a fill, a number format, borders,
    // alignment, its own font), so that only the link's own look, its font color and underline, is gone. A cell that has a color or underline of its own, not the style's, keeps it.
    std::vector<RestyleTarget> targets;
    std::vector<std::shared_ptr<const FormatChange>> owned;
    qint32 normal = -1;
    {
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) {
            out.ok = false;
            out.problem = tr("The workbook is busy. Try again in a moment.");
            return out;
        }
        const Styles &styles = m_book->book->styles();
        qint32 link = -1;
        for (const CellStyleName &n : styles.cellStyles) {
            if (n.name.compare(QStringLiteral("Hyperlink"), Qt::CaseInsensitive) == 0 && !n.hidden) link = qint32(n.xfId);
            if (n.builtin == 0) normal = qint32(n.xfId);
        }
        if (link >= 0 && normal >= 0 && link != normal && link < styles.styleXfs.size() && normal < styles.styleXfs.size()) {
            const jp::office::sheet::Sheet &model = m_book->calc->sheet(sheet);
            const CellFormat linkLook = styles.styleXfs[link], normalLook = styles.styleXfs[normal];
            const FormatChange linkFull = fullFormatChange(styles, linkLook), normalFull = fullFormatChange(styles, normalLook);
            std::map<quint32, std::shared_ptr<const FormatChange>> byFormat;
            auto changeFor = [&](quint32 format) {
                const auto seen = byFormat.find(format);
                if (seen != byFormat.end()) return seen->second;
                const CellFormat &cell = styles.format(format);
                FormatChange want = fullFormatChange(styles, cell);
                // (the link's own look is taken off: the color and the underline the style gives)
                FormatChange colorOfLink, underlineOfLink;
                colorOfLink.fontColor = linkFull.fontColor;
                underlineOfLink.underline = linkFull.underline;
                if (formatMatches(styles, cell, colorOfLink)) want.fontColor = normalFull.fontColor;
                if (formatMatches(styles, cell, underlineOfLink)) want.underline = normalFull.underline;
                FormatChange c = changedFields(styles, normalFull, want);
                c.namedStyle = quint32(normal);
                std::shared_ptr<const FormatChange> given = std::make_shared<const FormatChange>(c);
                owned.push_back(given);
                byFormat[format] = given;
                return given;
            };
            qint64 count = 0;
            for (const CellRange &r : linked) {
                if (!r.valid) continue;
                bool tooMany = false;
                model.cells.forEachInRange(r.row1, r.col1, r.row2, r.col2, [&](quint32 row, quint32 col, const jp::office::sheet::Cell &) {
                    const quint32 format = m_book->calc->content(sheet, row, col).style;
                    if (styles.format(format).xfId != quint32(link)) return true;
                    targets.push_back({row, col, changeFor(format).get()});
                    tooMany = ++count > kMaxStyledCells;
                    return !tooMany;
                });
                if (tooMany) {
                    out.ok = false;
                    out.problem = tr("Those links are on too many cells to take the look off at once (more than %1).").arg(QLocale().toString(kMaxStyledCells));
                    return out;
                }
            }
        }
    }
    const int steps = m_undo.count();
    m_undo.beginMacro(stepText);
    out = removeLinks(sheet, range, stepText);
    if (out.ok && !targets.empty()) out = restyle(sheet, targets, stepText);
    m_undo.endMacro();
    if (!out.ok && m_undo.count() > steps) m_undo.undo();   // (the look could not be taken off: the links stay)
    return out;
}

} // namespace jp::crunch
