// Links to other workbooks, for Inspect Workbook: a formula that reads another workbook (`[1]Prices!A1`) is changed to the value it shows, a name that stands for such a place is #REF!, and
// the saver is told to leave the link parts out (libs/office/sheet/xlsxsave.h, dropExternalLinks).
#include "core/editsession.h"
#include "sheet/externalbooks.h"
#include "core/gotospecial.h"
#include "office/package.h"
#include "sheet/cellref.h"
#include "sheet/sheetreader.h"

#include <QDir>
#include <QFileInfo>
#include <algorithm>
#include <set>
#include <tuple>
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
            const QRegularExpressionMatch m = link.matchView(QStringView(formula).mid(i), 0, QRegularExpression::NormalMatch, QRegularExpression::AnchorAtOffsetMatchOption);
            if (m.hasMatch() && (only == 0 || m.captured(1).toInt() == only)) return true;
        }
    }
    return false;
}

// A formula that is only a reference to one cell of the source: `[1]Prices!A1` or `'[1]My Prices'!$A$1` (captures: the number and the sheet's name, or the number and the quoted name; the cell).
const QRegularExpression &directPattern()
{
    static const QRegularExpression direct(QStringLiteral("^=?\\s*(?:\\[(\\d+)\\]([^'!\\[\\]]+)|'\\[(\\d+)\\]((?:[^']|'')+)')!\\$?([A-Za-z]{1,3})\\$?(\\d+)\\s*$"));
    return direct;
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

// What Break Link does to the list of links, and Undo takes back: the saver leaves the link parts out, or not.
class LinksDropped : public QUndoCommand {
public:
    LinksDropped(EditSession *session, bool before, bool after, const QString &text) : QUndoCommand(text), m_session(session), m_before(before), m_after(after) {}
    void redo() override { m_session->setExternalLinksDropped(m_after); }
    void undo() override { m_session->setExternalLinksDropped(m_before); }

private:
    EditSession *m_session;
    bool m_before, m_after;
};

// That a link reads its source file (Update Values), and Undo takes it back to what it was.
class LiveMark : public QUndoCommand {
public:
    LiveMark(jp::office::sheet::ExternalBooks *links, int index, bool was) : m_links(links), m_index(index), m_was(was) {}
    void redo() override { m_links->setLive(m_index, true); }
    void undo() override { m_links->setLive(m_index, m_was); }

private:
    jp::office::sheet::ExternalBooks *m_links;
    int m_index;
    bool m_was;
};

// What the choice of Startup prompt does and Undo takes back (-1: what the file has).
class UpdateLinksChanged : public QUndoCommand {
public:
    UpdateLinksChanged(EditSession *session, int before, int after, const QString &text) : QUndoCommand(text), m_session(session), m_before(before), m_after(after) {}
    void redo() override { m_session->setUpdateLinksValue(m_after); }
    void undo() override { m_session->setUpdateLinksValue(m_before); }

private:
    EditSession *m_session;
    int m_before, m_after;
};

// That a link's values were read again, for the saver: Undo takes it back to what it was.
class UpdatedMark : public QUndoCommand {
public:
    UpdatedMark(std::set<int> *marks, int index, bool was) : m_marks(marks), m_index(index), m_was(was) {}
    void redo() override { m_marks->insert(m_index); }
    void undo() override
    {
        if (!m_was) m_marks->erase(m_index);
    }

private:
    std::set<int> *m_marks;
    int m_index;
    bool m_was;
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

std::vector<EditSession::ExternalSource> EditSession::externalLinkSources(bool withState) const
{
    std::vector<ExternalSource> out;
    if (m_dropExternalLinks || !m_book || !m_book->book) return out;
    const jp::office::Package &package = m_book->book->package();
    const QString main = m_book->book->mainPart();
    const QByteArray xml = package.part(main);
    QXmlStreamReader reader(xml);
    constexpr int kMostSources = 1000;
    // (what is shown of a link: nothing is looked at, whatever the address; only a file: address is percent-encoded)
    auto fill = [&](ExternalSource &s) {
        const auto changed = m_externalTargets.find(s.index);
        if (changed != m_externalTargets.end()) s.target = changed->second;
        QString shown = s.target;
        const bool web = shown.startsWith(QLatin1String("http://"), Qt::CaseInsensitive) || shown.startsWith(QLatin1String("https://"), Qt::CaseInsensitive) || shown.startsWith(QLatin1String("ftp://"), Qt::CaseInsensitive);
        const bool fileUrl = shown.startsWith(QLatin1String("file:"), Qt::CaseInsensitive);
        if (shown.startsWith(QLatin1String("file:///"), Qt::CaseInsensitive)) {
            shown.remove(0, 8);
            if (!(shown.size() > 1 && shown[1] == QLatin1Char(':'))) shown.prepend(QLatin1Char('/'));
        }
        s.shown = fileUrl || web ? QUrl::fromPercentEncoding(shown.toUtf8()) : shown;   // (what is shown of an address; a plain path is shown as it is)
        if (m_book->externals) {
            s.network = m_book->externals->isNetwork(s.index);
            if (s.network) s.shown = m_book->externals->filePath(s.index);
            if (withState) s.state = int(m_book->externals->state(s.index));
            s.live = m_book->externals->live(s.index);
        }
        s.local = !web && !s.network && !s.shown.isEmpty();
    };
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
        fill(s);
        out.push_back(s);
    }
    // the links that typing the name of an open workbook made (saved already, or used by a formula now)
    if (m_book->externals) {
        const int fileCount = int(out.size());
        for (int n = fileCount + 1; n <= m_book->externals->count() && int(out.size()) < kMostSources; ++n) {
            if (!m_savedLinks.count(n) && !newLinkInUse(n)) continue;
            ExternalSource s;
            s.index = n;
            s.target = m_book->externals->target(n);
            fill(s);
            out.push_back(s);
        }
    }
    return out;
}

bool EditSession::newLinkInUse(int index) const
{
    bool used = false;
    if (m_book && m_book->calc)
        m_book->calc->forEachFormulaText([&](int, quint32, quint32, const QString &formula) {
            if (!used && readsAnotherWorkbook(formula, index)) used = true;
        });
    return used;
}

void EditSession::setExternalTarget(int index, const QString &target)
{
    // (kept after a save too: what the file now holds is not what an Undo of an earlier step goes back to, so the address is always written, as the step says)
    if (target.isEmpty()) return;
    m_externalTargets[index] = target;
    // (the source is another file: what its link stores is read until the values are updated)
    if (m_book && m_book->externals) {
        m_book->externals->setTarget(index, target);
        m_book->externals->setLive(index, false);
    }
}

void EditSession::setUpdateLinksValue(int mode) { m_updateLinks = mode; }

std::map<QString, QString> EditSession::externalSourceParts() const
{
    std::map<QString, QString> out;
    if (m_externalTargets.empty() || m_dropExternalLinks || !m_book || !m_book->book) return out;
    const QStringList parts = linkPartsOf(m_book->book->package(), m_book->book->mainPart());
    for (const auto &[index, address] : m_externalTargets)
        if (index >= 1 && index <= parts.size() && !parts[index - 1].isEmpty()) out[parts[index - 1]] = address;
    return out;
}

std::map<QString, std::vector<jp::office::sheet::SaveChanges::ExternalCachedCell>> EditSession::externalCachedValues() const
{
    std::map<QString, std::vector<jp::office::sheet::SaveChanges::ExternalCachedCell>> out;
    if (m_externalUpdated.empty() || m_dropExternalLinks || !m_book || !m_book->book || !m_book->externals) return out;
    const QStringList parts = linkPartsOf(m_book->book->package(), m_book->book->mainPart());
    // (what the formulas read of each source that was updated, as the source has it now)
    for (const int index : m_externalUpdated) {
        if (index < 1 || index > parts.size() || parts[index - 1].isEmpty()) continue;
        std::vector<jp::office::sheet::SaveChanges::ExternalCachedCell> cells = m_book->externals->readCells(index);
        if (!cells.empty()) out[parts[index - 1]] = std::move(cells);
    }
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
    const std::vector<ExternalSource> sources = externalLinkSources(false);
    if (index < 1 || index > int(sources.size()) || path.trimmed().isEmpty()) {
        out.ok = false;
        out.problem = tr("That link is not in this workbook.");
        return out;
    }
    const QString address = addressOf(path);
    // (the formulas that read it are worked out from the new file at once, in the same step of Undo; a file that can't be read leaves the stored values)
    m_undo.beginMacro(tr("Change Source"));
    m_undo.push(new SourceChanged(this, index, sources[size_t(index - 1)].target, address, tr("Change Source")));
    if (m_book->externals && m_book->externals->isNetwork(index)) m_book->externals->setAllowNetwork(index, true);
    if (QFileInfo(path).isFile()) updateExternalValues(index, path);
    m_undo.endMacro();
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
    // (only a file is read: a pipe or a device has no end to be read to)
    if (!QFileInfo(path).isFile()) return refuse(tr("The workbook \"%1\" is not a file that can be read.").arg(path));
    jp::office::sheet::ExternalBooks *links = m_book->externals.get();
    if (!links || index < 1 || index > links->count()) return refuse(tr("That link is not in this workbook."));
    // (the link names a file; a file given that is another one is what the link points to from now)
    if (QFileInfo(links->localPath(index)).canonicalFilePath() != QFileInfo(path).canonicalFilePath()) {
        links->setTarget(index, addressOf(path));
        m_externalTargets[index] = addressOf(path);
    }
    // (the person asked for this source: one on another computer is read now, and only now)
    if (links->isNetwork(index)) links->setAllowNetwork(index, true);
    QString error;
    if (!links->probe(index, &error, false)) return refuse(tr("The workbook \"%1\" could not be opened. %2").arg(path, error));
    const bool wasLive = links->live(index);
    links->setLive(index, true);
    // the formulas that read a link: by the number in their text; one that reads it through a name is worked out whichever link is updated
    struct Place {
        int sheet;
        quint32 row, col;
        QString formula;
    };
    std::vector<Place> places;
    constexpr size_t kMostCells = 200000;
    bool cut = false;
    {
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) {
            links->setLive(index, wasLive);
            return refuse(tr("The workbook is still being opened. It can be changed when it is open."));
        }
        hold->calculation()->forEachExternalFormula([&](int sheet, quint32 row, quint32 col) {
            if (places.size() >= kMostCells) {
                cut = true;
                return;
            }
            const QString formula = hold->calculation()->content(sheet, row, col).formula;
            const bool numbered = readsAnotherWorkbook(formula);
            if (!numbered || readsAnotherWorkbook(formula, index)) places.push_back({sheet, row, col, formula});
        });
    }
    std::sort(places.begin(), places.end(), [](const Place &a, const Place &b) { return std::tie(a.sheet, a.row, a.col) < std::tie(b.sheet, b.row, b.col); });
    int updated = 0, kept = 0;
    m_undo.beginMacro(tr("Update Values"));
    // (a formula may read another that is updated in the same go: the cells are worked out again, a few rounds, until none changes)
    for (int pass = 0; pass < 4; ++pass) {
        Step step;
        {
            std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
            if (!hold) break;
            jp::office::sheet::Calculation *calc = hold->calculation();
            for (const Place &p : places) {
                const jp::office::sheet::Calculation::CellContent content = calc->content(p.sheet, p.row, p.col);
                if (!content.exists || !content.isFormula || content.arrayRange.valid || content.dataTable || content.spilled) {
                    if (pass == 0) ++kept;
                    continue;
                }
                QString text = content.formula;
                if (text.startsWith(QLatin1Char('='))) text.remove(0, 1);
                jp::calc::Value v;
                if (!calc->evaluateHere(p.sheet, p.row, p.col, text, &v) || v.isArray()) {
                    if (pass == 0) ++kept;
                    continue;
                }
                if (v.isEmpty()) v = jp::calc::Value::number(0);
                const jp::calc::Value now = content.hasStored ? content.stored : content.value;
                if (pass > 0 && now == v) continue;
                if (pass == 0) ++updated;
                CellChange ch;
                ch.sheet = p.sheet;
                ch.row = p.row;
                ch.col = quint16(p.col);
                ch.before = content;
                ch.after = content;
                ch.after.hasStored = true;
                ch.after.stored = v;
                step.cells.push_back(std::move(ch));
            }
        }
        if (step.cells.empty()) break;
        pushStep(std::move(step), tr("Update Values"));
    }
    if (result) {
        result->updated = updated;
        result->kept = kept;
        result->cut = cut;
    }
    m_undo.push(new UpdatedMark(&m_externalUpdated, index, m_externalUpdated.count(index) > 0));
    m_undo.push(new LiveMark(links, index, wasLive));
    m_undo.endMacro();
    return out;
}

QString EditSession::linkTypedReferences(const QString &typed)
{
    if (m_planDepth == 0 || !m_openBookFinder || !m_book || !m_book->book || m_dropExternalLinks) return typed;
    const QString trimmed = typed.trimmed();
    if (!trimmed.startsWith(QLatin1Char('=')) || !trimmed.contains(QLatin1Char('['))) return typed;
    // the link of an open workbook: the one of its file if there is one, else a new one (read at once)
    // (the number the link has or will have: the link is made when the entry is committed, not here; no file is asked about)
    auto numberOf = [&](const QString &path) -> int {
        auto *links = m_book->externals.get();
        if (!links) return 0;
        const QString clean = QDir::cleanPath(path);
        for (int i = 1; i <= links->count(); ++i)
            if (!links->isNetwork(i) && QDir::cleanPath(links->localPath(i)) == clean) return i;
        for (const LinkPlan &p : m_linkPlan)
            if (p.path == clean) return p.number;
        m_linkPlan.push_back({clean, links->count() + int(m_linkPlan.size()) + 1});
        return m_linkPlan.back().number;
    };
    static const QRegularExpression quoted(QStringLiteral("'\\[([^\\]\\[']*[^\\]\\[\\d'][^\\]\\[']*)\\]((?:[^']|'')+)'!"));
    static const QRegularExpression plain(QStringLiteral("\\[([^\\]\\[]*[^\\]\\[\\d][^\\]\\[]*)\\]([A-Za-z_][A-Za-z0-9_.]*)!"));
    auto rewrite = [&](const QString &segment) {
        QString out;
        int at = 0;
        while (at < segment.size()) {
            QRegularExpressionMatch q = quoted.match(segment, at), p = plain.match(segment, at);
            QRegularExpressionMatch m = q.hasMatch() && (!p.hasMatch() || q.capturedStart() <= p.capturedStart()) ? q : p;
            if (!m.hasMatch()) break;
            const bool isQuoted = m.regularExpression().pattern() == quoted.pattern();
            const QString book = m.captured(1);
            const QString path = m_openBookFinder(book);
            out += segment.mid(at, m.capturedStart() - at);
            const int n = path.isEmpty() ? 0 : numberOf(path);
            if (n == 0) {
                out += m.captured(0);
            } else {
                QString sheet = m.captured(2);
                if (isQuoted) sheet.replace(QStringLiteral("''"), QStringLiteral("'"));
                const bool simple = QRegularExpression(QStringLiteral("^[A-Za-z_][A-Za-z0-9_.]*$")).match(sheet).hasMatch();
                out += simple ? QStringLiteral("[%1]%2!").arg(n).arg(sheet) : QStringLiteral("'[%1]%2'!").arg(n).arg(QString(sheet).replace(QLatin1Char('\''), QStringLiteral("''")));
            }
            at = m.capturedEnd();
        }
        return out + segment.mid(at);
    };
    // (the texts in quotes are left as they are)
    QString result;
    int from = 0;
    while (from < typed.size()) {
        const int open = typed.indexOf(QLatin1Char('"'), from);
        const int end = open < 0 ? typed.size() : open;
        result += rewrite(typed.mid(from, end - from));
        if (open < 0) break;
        int close = open + 1;
        while (close < typed.size()) {
            if (typed[close] == QLatin1Char('"')) {
                if (close + 1 < typed.size() && typed[close + 1] == QLatin1Char('"')) close += 2;
                else break;
            } else {
                ++close;
            }
        }
        result += typed.mid(open, std::min<int>(typed.size(), close + 1) - open);
        from = close + 1;
    }
    return result;
}

void EditSession::commitLinkPlan()
{
    if (m_linkPlan.empty() || !m_book || !m_book->externals) {
        m_linkPlan.clear();
        return;
    }
    for (const LinkPlan &p : m_linkPlan) {
        const int n = m_book->externals->addLink(addressOf(p.path));
        m_book->externals->setLive(n, true);
        m_newLinks.insert(n);
    }
    m_linkPlan.clear();
}

std::vector<jp::office::sheet::SaveChanges::NewExternalLink> EditSession::newExternalLinksForSave() const
{
    std::vector<jp::office::sheet::SaveChanges::NewExternalLink> out;
    if (m_newLinks.empty() || m_dropExternalLinks || !m_book || !m_book->externals) return out;
    // (a link that no formula reads any more, because the entry was undone, is not written: the ones after the last that is used go, so the numbers stay as they are)
    int last = 0;
    for (const int n : m_newLinks)
        if (newLinkInUse(n)) last = n;
    for (const int n : m_newLinks) {
        if (n > last) break;
        jp::office::sheet::SaveChanges::NewExternalLink l;
        l.target = m_book->externals->target(n);
        l.sheetNames = m_book->externals->sheetsRead(n);
        l.cells = m_book->externals->readCells(n);
        out.push_back(std::move(l));
    }
    return out;
}

EditSession::UpdateLinks EditSession::updateLinksMode() const
{
    if (m_updateLinks >= 0) return UpdateLinks(m_updateLinks);
    return m_book && m_book->book ? UpdateLinks(int(m_book->book->info().updateLinks)) : UpdateLinks::Ask;
}

EditSession::UpdateLinks EditSession::openingChoice() const
{
    // (what a file says is never enough to read other files: "update" in the file is a question, so that a file cannot make the program open others; only the person's own choice in this session,
    // made in Edit Links, updates without asking)
    if (m_updateLinks >= 0) return UpdateLinks(m_updateLinks);
    return updateLinksMode() == UpdateLinks::Never ? UpdateLinks::Never : UpdateLinks::Ask;
}

EditSession::Outcome EditSession::setUpdateLinksMode(UpdateLinks mode)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (m_updateLinks == int(mode)) return out;
    m_undo.push(new UpdateLinksChanged(this, m_updateLinks, int(mode), tr("Startup prompt")));
    return out;
}

EditSession::Outcome EditSession::updateAllExternalLinks(int *updated)
{
    Outcome out;
    if (updated) *updated = 0;
    for (const ExternalSource &s : externalLinkSources()) {
        if (!s.local || s.network || !m_book->externals) continue;   // (one on another computer is read only when the person asks for that one)
        const QString path = m_book->externals->localPath(s.index);
        if (path.isEmpty() || !QFileInfo(path).isFile()) continue;
        ExternalUpdate one;
        const Outcome o = updateExternalValues(s.index, path, &one);
        if (!o.ok) {
            out.ok = false;
            out.problem = o.problem;
            continue;
        }
        if (updated) *updated += one.updated;
    }
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
    QVector<int> nameIds;
    for (int id = 0; id < m_nameTable.slots(); ++id)
        if (nameListed(id) && readsAnotherWorkbook(nameTextOf(id), only)) nameIds.append(id);
    // (no link at all, or nothing that reads the one asked for and none to leave out of the file: nothing is done, and nothing is a step of Undo)
    const bool leaveOut = !m_dropExternalLinks && !externalLinkSources().empty() && (only == 0 || externalLinkCells() - int(places.size()) <= 0);
    if (places.empty() && nameIds.isEmpty() && !leaveOut) return out;
    // (one step of Undo: the cells, the names, and the links that are left out of the file)
    m_undo.beginMacro(tr("Break Link"));
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
    for (int id : nameIds) {
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
    if (left == 0 && !m_dropExternalLinks && (only == 0 || externalLinkCells() == 0)) m_undo.push(new LinksDropped(this, false, true, tr("Break Link")));
    m_undo.endMacro();
    if (left > 0) {
        out.ok = false;
        out.problem = tr("Some cells that read another workbook could not be changed, because they are locked or part of an array. The links stay.");
        return out;
    }
    return out;
}

} // namespace jp::crunch
