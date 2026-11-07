// Links to other workbooks, for Inspect Workbook: a formula that reads another workbook (`[1]Prices!A1`) is changed to the value it shows, a name that stands for such a place is #REF!, and
// the saver is told to leave the link parts out (libs/office/sheet/xlsxsave.h, dropExternalLinks).
#include "core/editsession.h"
#include "core/gotospecial.h"
#include "office/package.h"
#include "sheet/cellref.h"
#include "sheet/sheetreader.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QUndoCommand>
#include <QUrl>
#include <QXmlStreamReader>

namespace jp::crunch {

namespace {

// Whether a formula (as `editText` gives it, with its "=") reads another workbook: a number in square brackets, outside the quotes of a text or of a sheet's name.
bool readsAnotherWorkbook(const QString &formula, int only = 0)
{
    static const QRegularExpression link(QStringLiteral("\\[([0-9]+)\\]"));
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
            const QRegularExpressionMatch m = link.matchView(QStringView(formula).mid(i), 0, QRegularExpression::NormalMatch, QRegularExpression::AnchoredMatchOption);
            if (m.hasMatch() && (only == 0 || m.captured(1).toInt() == only)) return true;
        }
    }
    return false;
}

// The part of each source the workbook names, in the order of the numbers in its formulas.
QStringList linkPartsOf(const jp::office::Package &package, const QString &main)
{
    QStringList out;
    const QByteArray xml = package.part(main);
    QXmlStreamReader reader(xml);
    constexpr int kMostSources = 1000;
    while (!reader.atEnd() && out.size() < kMostSources) {
        if (reader.readNext() != QXmlStreamReader::StartElement || reader.name() != QLatin1String("externalReference")) continue;
        QString id;
        for (const QXmlStreamAttribute &a : reader.attributes())
            if (a.name() == QLatin1String("id")) id = a.value().toString();
        const jp::office::Relationship rel = package.relationship(main, id);
        out << (rel.id.isEmpty() ? QString() : rel.target);
    }
    return out;
}

// What Change Source does and Undo takes back.
class SourceChanged : public QUndoCommand {
public:
    SourceChanged(EditSession *session, int index, const QString &before, const QString &after, const QString &text) : QUndoCommand(text), m_session(session), m_index(index), m_before(before), m_after(after) {}
    void redo() override { m_session->setExternalTarget(m_index, m_after); }
    void undo() override { m_session->setExternalTarget(m_index, m_before); }

private:
    EditSession *m_session;
    int m_index;
    QString m_before, m_after;
};

// The address a file on this computer is written as in a link.
QString addressOf(const QString &path) { return QUrl::fromLocalFile(QFileInfo(path).absoluteFilePath()).toString(QUrl::FullyEncoded); }

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

std::vector<EditSession::ExternalSource> EditSession::externalLinkSources() const
{
    std::vector<ExternalSource> out;
    if (m_dropExternalLinks || !m_book || !m_book->book) return out;
    const jp::office::Package &package = m_book->book->package();
    const QString main = m_book->book->mainPart();
    const QByteArray xml = package.part(main);
    QXmlStreamReader reader(xml);
    constexpr int kMostSources = 1000;
    while (!reader.atEnd() && int(out.size()) < kMostSources) {
        if (reader.readNext() != QXmlStreamReader::StartElement || reader.name() != QLatin1String("externalReference")) continue;
        QString id;
        for (const QXmlStreamAttribute &a : reader.attributes())
            if (a.name() == QLatin1String("id")) id = a.value().toString();
        ExternalSource s;
        s.index = int(out.size()) + 1;
        const jp::office::Relationship rel = package.relationship(main, id);
        if (!rel.id.isEmpty()) {
            const QVector<jp::office::Relationship> paths = package.relationshipsOfType(rel.target, QStringLiteral("/externalLinkPath"));
            if (!paths.isEmpty()) s.target = paths.first().target.left(8192);
        }
        const auto changed = m_externalTargets.find(s.index);
        if (changed != m_externalTargets.end()) s.target = changed->second;
        QString shown = s.target;
        const bool web = shown.startsWith(QLatin1String("http://"), Qt::CaseInsensitive) || shown.startsWith(QLatin1String("https://"), Qt::CaseInsensitive) || shown.startsWith(QLatin1String("ftp://"), Qt::CaseInsensitive);
        if (shown.startsWith(QLatin1String("file:///"), Qt::CaseInsensitive)) {
            shown.remove(0, 8);
            if (!(shown.size() > 1 && shown[1] == QLatin1Char(':'))) shown.prepend(QLatin1Char('/'));
        }
        s.shown = QUrl::fromPercentEncoding(shown.toUtf8());
        s.local = !web && !s.shown.isEmpty();
        out.push_back(s);
    }
    return out;
}

void EditSession::setExternalTarget(int index, const QString &target)
{
    if (target.isEmpty()) m_externalTargets.erase(index);
    else m_externalTargets[index] = target;
}

std::map<QString, QString> EditSession::externalSourceParts() const
{
    std::map<QString, QString> out;
    if (m_externalTargets.empty() || m_dropExternalLinks || !m_book || !m_book->book) return out;
    const QStringList parts = linkPartsOf(m_book->book->package(), m_book->book->mainPart());
    for (const auto &[index, address] : m_externalTargets)
        if (index >= 1 && index <= parts.size() && !parts[index - 1].isEmpty()) out[parts[index - 1]] = address;
    return out;
}

EditSession::Outcome EditSession::changeExternalSource(int index, const QString &path)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    const std::vector<ExternalSource> sources = externalLinkSources();
    if (index < 1 || index > int(sources.size()) || path.trimmed().isEmpty()) {
        out.ok = false;
        out.problem = tr("That link is not in this workbook.");
        return out;
    }
    const QString address = addressOf(path);
    const auto was = m_externalTargets.find(index);
    m_undo.push(new SourceChanged(this, index, was == m_externalTargets.end() ? QString() : was->second, address, tr("Change Source")));
    return out;
}

EditSession::Outcome EditSession::updateExternalValues(int index, const QString &path, ExternalUpdate *result)
{
    Outcome out;
    if (result) *result = ExternalUpdate();
    auto refuse = [&](const QString &problem) {
        out.ok = false;
        out.problem = problem;
        return out;
    };
    QString why;
    if (!canEdit(&why)) return refuse(why);
    jp::office::sheet::Workbook source;
    QString error;
    if (path.isEmpty() || !source.openFile(path, &error)) return refuse(tr("The workbook \"%1\" could not be opened. %2").arg(path, error));
    // a formula that is only a reference to one cell of the source: `[1]Prices!A1` or `'[1]My Prices'!$A$1`
    static const QRegularExpression direct(QStringLiteral("^=?\\s*(?:\\[(\\d+)\\]([^'!\\[\\]]+)|'\\[(\\d+)\\]((?:[^']|'')+)')!\\$?([A-Za-z]{1,3})\\$?(\\d+)\\s*$"));
    struct Place {
        int sheet;
        quint32 row, col;
        QString formula;
    };
    std::vector<Place> places;
    constexpr size_t kMostCells = 200000;
    m_book->calc->forEachFormulaText([&](int sheet, quint32 row, quint32 col, const QString &formula) {
        if (places.size() < kMostCells && readsAnotherWorkbook(formula, index)) places.push_back({sheet, row, col, formula});
    });
    std::map<QString, std::shared_ptr<jp::office::sheet::Sheet>> read;
    Step step;
    int kept = 0;
    for (const Place &p : places) {
        const QRegularExpressionMatch m = direct.match(p.formula);
        if (!m.hasMatch() || (m.captured(1).isEmpty() ? m.captured(3) : m.captured(1)).toInt() != index) {
            ++kept;
            continue;
        }
        QString name = m.captured(1).isEmpty() ? m.captured(4).replace(QStringLiteral("''"), QStringLiteral("'")) : m.captured(2);
        int found = -1;
        const auto &sheets = source.info().sheets;
        for (int i = 0; i < int(sheets.size()) && found < 0; ++i)
            if (!sheets[size_t(i)].deleted && sheets[size_t(i)].name.compare(name, Qt::CaseInsensitive) == 0) found = i;
        quint32 row = 0, col = 0;
        if (found < 0 || !jp::office::sheet::parseCellRef(m.captured(5) + m.captured(6), &row, &col)) {
            ++kept;
            continue;
        }
        std::shared_ptr<jp::office::sheet::Sheet> &sheet = read[name.toLower()];
        if (!sheet) {
            sheet = std::make_shared<jp::office::sheet::Sheet>();
            source.readSheet(found, sheet.get());
        }
        jp::calc::Value v = jp::calc::Value::number(0);
        if (const jp::office::sheet::Cell *c = sheet->cells.find(row, col)) {
            using T = jp::office::sheet::CellType;
            switch (c->type) {
            case T::Number:
            case T::Date: v = jp::calc::Value::number(c->number); break;
            case T::Bool: v = jp::calc::Value::boolean(c->number != 0); break;
            case T::Error: v = jp::calc::Value::error(static_cast<jp::calc::ErrorKind>(quint8(c->number))); break;
            case T::SharedString: v = jp::calc::Value::text(source.sharedStrings().at(c->textIndex())); break;
            case T::InlineString:
            case T::FormulaString: v = jp::calc::Value::text(c->textIndex() >= 0 && c->textIndex() < sheet->texts.size() ? sheet->texts[c->textIndex()] : QString()); break;
            case T::Empty: break;
            }
        }
        CellChange ch;
        ch.sheet = p.sheet;
        ch.row = p.row;
        ch.col = quint16(p.col);
        ch.before = m_book->calc->content(p.sheet, p.row, p.col);
        ch.after = ch.before;
        ch.after.hasStored = true;
        ch.after.stored = v;
        step.cells.push_back(std::move(ch));
    }
    if (result) {
        result->updated = int(step.cells.size());
        result->kept = kept;
    }
    if (step.cells.empty()) return out;
    pushStep(std::move(step), tr("Update Values"));
    return out;
}

EditSession::Outcome EditSession::breakExternalLinks(int *cells, int *names, int only)
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
        if (readsAnotherWorkbook(formula, only)) places.push_back({sheet, row, col});
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
        if (!nameListed(id) || !readsAnotherWorkbook(nameTextOf(id), only)) continue;
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
    if (only == 0 || externalLinkCells() == 0) m_dropExternalLinks = true;
    return out;
}

} // namespace jp::crunch
