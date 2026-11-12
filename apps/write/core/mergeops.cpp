// Mail merge (core/mergeops.h): the merge settings kept with the document, the fields of a merge, and the merge of a document with a list of recipients.

#include "core/mergeops.h"

#include "core/editops.h"
#include "core/fieldops.h"
#include "core/barcodeitem.h"
#include "core/labelops.h"
#include "core/pictureops.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSaveFile>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QUrl>

namespace jp::write::mail {

namespace {

constexpr int kMostPromptChars = 1000;
constexpr int kMostFormatChars = 2000;

QString normalized(const QString &name) { return name.trimmed().toLower().replace(QLatin1Char('_'), QLatin1Char(' ')); }

int columnIndex(const MergeSource &data, const QString &name)
{
    const QString want = normalized(name);
    if (want.isEmpty()) return -1;
    for (int i = 0; i < data.fields.size(); ++i)
        if (normalized(data.fields[i]) == want) return i;
    return -1;
}

const QStringList &synonyms(const QString &logical)
{
    static const QHash<QString, QStringList> table = {
        {QStringLiteral("Title"), {QStringLiteral("Title"), QStringLiteral("Courtesy Title"), QStringLiteral("Salutation"), QStringLiteral("Prefix")}},
        {QStringLiteral("First"), {QStringLiteral("First Name"), QStringLiteral("FirstName"), QStringLiteral("First"), QStringLiteral("Given Name"), QStringLiteral("Forename")}},
        {QStringLiteral("Last"), {QStringLiteral("Last Name"), QStringLiteral("LastName"), QStringLiteral("Last"), QStringLiteral("Surname"), QStringLiteral("Family Name")}},
        {QStringLiteral("Suffix"), {QStringLiteral("Suffix")}},
        {QStringLiteral("Name"), {QStringLiteral("Name"), QStringLiteral("Full Name")}},
        {QStringLiteral("Company"), {QStringLiteral("Company"), QStringLiteral("Company Name"), QStringLiteral("Organization"), QStringLiteral("Organisation")}},
        {QStringLiteral("Street1"), {QStringLiteral("Address Line 1"), QStringLiteral("Address 1"), QStringLiteral("Address"), QStringLiteral("Street"), QStringLiteral("Street Address"), QStringLiteral("Street1")}},
        {QStringLiteral("Street2"), {QStringLiteral("Address Line 2"), QStringLiteral("Address 2"), QStringLiteral("Street2")}},
        {QStringLiteral("City"), {QStringLiteral("City"), QStringLiteral("Town")}},
        {QStringLiteral("State"), {QStringLiteral("State"), QStringLiteral("Province"), QStringLiteral("Region"), QStringLiteral("ST")}},
        {QStringLiteral("Postal"), {QStringLiteral("ZIP Code"), QStringLiteral("ZIP"), QStringLiteral("Postal Code"), QStringLiteral("Postcode"), QStringLiteral("Postal")}},
        {QStringLiteral("Country"), {QStringLiteral("Country"), QStringLiteral("Country or Region")}},
        {QStringLiteral("Email"), {QStringLiteral("Email"), QStringLiteral("Email Address"), QStringLiteral("E-mail")}},
    };
    static const QStringList none;
    const auto it = table.constFind(logical);
    return it == table.constEnd() ? none : it.value();
}

// The value of a part of a name or an address for a record: where the list has no first or last name but a whole name, the first stands for the whole.
QString partValue(const MergeSource &data, int row, const QHash<QString, QString> &map, const QString &logical)
{
    const QString column = columnFor(data, map, logical);
    if (!column.isEmpty()) return data.value(row, column).trimmed();
    if (logical == QLatin1String("First") && columnFor(data, map, QStringLiteral("Last")).isEmpty()) {
        const QString whole = columnFor(data, map, QStringLiteral("Name"));
        if (!whole.isEmpty()) return data.value(row, whole).trimmed();
    }
    return QString();
}

// A format of the document's own kind, or one with the marks of the other program's (<< _FIRST0_>>), as one with {Field}s and '|' between the lines.
QString plainFormat(QString format)
{
    format = format.left(kMostFormatChars);
    if (format.contains(QLatin1String("<<"))) {
        static const QHash<QString, QString> tokens = {{QStringLiteral("TITLE"), QStringLiteral("Title")},   {QStringLiteral("FIRST"), QStringLiteral("First")},   {QStringLiteral("LAST"), QStringLiteral("Last")},
                                                       {QStringLiteral("SUFFIX"), QStringLiteral("Suffix")}, {QStringLiteral("COMPANY"), QStringLiteral("Company")}, {QStringLiteral("STREET1"), QStringLiteral("Street1")},
                                                       {QStringLiteral("STREET2"), QStringLiteral("Street2")}, {QStringLiteral("CITY"), QStringLiteral("City")},   {QStringLiteral("STATE"), QStringLiteral("State")},
                                                       {QStringLiteral("POSTAL"), QStringLiteral("Postal")}, {QStringLiteral("COUNTRY"), QStringLiteral("Country")}, {QStringLiteral("MIDDLE"), QStringLiteral("Title")}};
        static const QRegularExpression group(QStringLiteral("<<(.*?)>>"));
        static const QRegularExpression token(QStringLiteral("_([A-Z]+)\\d*_"));
        QString out;
        int last = 0;
        for (auto it = group.globalMatch(format); it.hasNext();) {
            const QRegularExpressionMatch m = it.next();
            out += format.mid(last, m.capturedStart() - last);
            last = m.capturedEnd();
            QString inner = m.captured(1);
            const QRegularExpressionMatch t = token.match(inner);
            const QString word = t.hasMatch() ? t.captured(1) : QString();
            if (word == QLatin1String("BEFORE") || word == QLatin1String("AFTER")) {
                QString text = inner.mid(t.capturedEnd()).trimmed();
                out += word == QLatin1String("BEFORE") ? text + QLatin1Char(' ') : text;
            } else if (tokens.contains(word)) {
                out += QLatin1Char('{') + tokens.value(word) + QLatin1Char('}') + QLatin1Char(' ');
            }
        }
        out += format.mid(last);
        out.replace(QLatin1String("\\n"), QLatin1String("|")).replace(QLatin1Char('\n'), QLatin1Char('|')).replace(QLatin1Char('\r'), QString());
        format = out;
    }
    return format;
}

// A format's {Field}s for the values of a record (`any`, if given, says whether any of the fields had a value). A field with no value is left out with the words that join it to the next
// (or, where a comma comes before it, with that comma): "{City}, {State} {Postal}" for no State is "London N1 9GU".
QString fill(const QString &line, const std::function<QString(const QString &)> &value, bool *any)
{
    struct Piece {
        bool field;
        QString text;
        bool dropped = false;
    };
    QVector<Piece> pieces;
    QString literal;
    for (int i = 0; i < line.size();) {
        if (line[i] == QLatin1Char('{')) {
            const int close = line.indexOf(QLatin1Char('}'), i);
            if (close > i) {
                if (!literal.isEmpty()) pieces.push_back({false, literal});
                literal.clear();
                pieces.push_back({true, value(line.mid(i + 1, close - i - 1))});
                i = close + 1;
                continue;
            }
        }
        literal += line[i++];
    }
    if (!literal.isEmpty()) pieces.push_back({false, literal});
    bool got = false;
    for (int i = 0; i < pieces.size(); ++i) {
        if (!pieces[i].field) continue;
        if (!pieces[i].text.isEmpty()) {
            got = true;
            continue;
        }
        pieces[i].dropped = true;
        if (i > 0 && !pieces[i - 1].field && pieces[i - 1].text.contains(QLatin1Char(','))) pieces[i - 1].dropped = true;
        else if (i + 1 < pieces.size() && !pieces[i + 1].field) pieces[i + 1].dropped = true;
    }
    QString out;
    for (const Piece &p : std::as_const(pieces))
        if (!p.dropped) out += p.text;
    if (any) *any = got;
    return out;
}

QString tidy(QString s, bool stripEnds)
{
    static const QRegularExpression spaces(QStringLiteral("[ \\t]+")), emptyList(QStringLiteral("\\s*,(\\s*,)+")), beforeComma(QStringLiteral("\\s+(?=[,.;:!?])")), leading(QStringLiteral("^[,;\\s]+")),
        trailing(QStringLiteral("[,;\\s]+$"));
    s.replace(spaces, QStringLiteral(" "));
    s.replace(emptyList, QStringLiteral(","));
    s.replace(beforeComma, QString());
    s = s.trimmed();
    if (stripEnds) {
        s.remove(leading);
        s.remove(trailing);
    }
    return s;
}

} // namespace

namespace {

// The names the file format gives the parts of an address in the field map of a data source, by the program's names for them.
const QVector<QPair<QString, QString>> &predefinedNames()
{
    static const QVector<QPair<QString, QString>> names = {
        {QStringLiteral("Title"), QStringLiteral("Courtesy Title")}, {QStringLiteral("First"), QStringLiteral("First Name")},   {QStringLiteral("Last"), QStringLiteral("Last Name")},
        {QStringLiteral("Suffix"), QStringLiteral("Suffix")},        {QStringLiteral("Company"), QStringLiteral("Company Name")}, {QStringLiteral("Street1"), QStringLiteral("Address 1")},
        {QStringLiteral("Street2"), QStringLiteral("Address 2")},    {QStringLiteral("City"), QStringLiteral("City")},          {QStringLiteral("State"), QStringLiteral("State")},
        {QStringLiteral("Postal"), QStringLiteral("Postal Code")},   {QStringLiteral("Country"), QStringLiteral("Country or Region")}, {QStringLiteral("Email"), QStringLiteral("E-mail Address")}};
    return names;
}

QString unescapedXml(QString s)
{
    s.replace(QLatin1String("&lt;"), QLatin1String("<")).replace(QLatin1String("&gt;"), QLatin1String(">")).replace(QLatin1String("&quot;"), QLatin1String("\"")).replace(QLatin1String("&apos;"), QLatin1String("'"));
    return s.replace(QLatin1String("&amp;"), QLatin1String("&"));
}

// The field map a data source's odso element holds: the column each predefined name is matched to.
QHash<QString, QString> fieldMapOf(const QByteArray &odso)
{
    QHash<QString, QString> map;
    static const QRegularExpression entry(QStringLiteral("<(?:w:)?fieldMapData>(.*?)</(?:w:)?fieldMapData>"), QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression name(QStringLiteral("<(?:w:)?name\\s+(?:w:)?val=\"([^\"]*)\"")), mapped(QStringLiteral("<(?:w:)?mappedName\\s+(?:w:)?val=\"([^\"]*)\""));
    int seen = 0;
    for (auto it = entry.globalMatch(QString::fromUtf8(odso)); it.hasNext() && seen++ < 200;) {
        const QString inner = it.next().captured(1);
        const QRegularExpressionMatch n = name.match(inner), m = mapped.match(inner);
        if (!n.hasMatch() || !m.hasMatch() || n.captured(1).isEmpty()) continue;
        for (const auto &p : predefinedNames())
            if (p.second == unescapedXml(m.captured(1))) map.insert(p.first, unescapedXml(n.captured(1)));
    }
    return map;
}

// The odso element with its field map made as `map` says (what else it holds stays as it was).
QByteArray odsoWithFieldMap(const QByteArray &odso, const QHash<QString, QString> &map, const QStringList &columns)
{
    auto esc = [](const QString &s) {
        QString t = s;
        t.replace(QLatin1Char('&'), QLatin1String("&amp;")).replace(QLatin1Char('<'), QLatin1String("&lt;")).replace(QLatin1Char('>'), QLatin1String("&gt;")).replace(QLatin1Char('"'), QLatin1String("&quot;"));
        return t.toUtf8();
    };
    QByteArray entries;
    for (const auto &p : predefinedNames()) {
        const QString column = map.value(p.first);
        if (column.isEmpty()) continue;
        entries += "<w:fieldMapData><w:type w:val=\"dbColumn\"/><w:name w:val=\"" + esc(column) + "\"/><w:mappedName w:val=\"" + esc(p.second) + "\"/><w:column w:val=\"" +
                   QByteArray::number(std::max<qsizetype>(0, columns.indexOf(column))) + "\"/><w:lid w:val=\"en-US\"/></w:fieldMapData>";
    }
    static const QRegularExpression old(QStringLiteral("<(?:w:)?fieldMapData>.*?</(?:w:)?fieldMapData>"), QRegularExpression::DotMatchesEverythingOption);
    QString raw = QString::fromUtf8(odso);
    raw.remove(old);
    if (raw.trimmed().isEmpty()) return entries.isEmpty() ? QByteArray() : "<w:odso>" + entries + "</w:odso>";
    if (raw.endsWith(QLatin1String("/>")) && !raw.contains(QLatin1String("</"))) raw = raw.left(raw.size() - 2) + QStringLiteral("></w:odso>");
    int at = raw.indexOf(QLatin1String("<w:recipientData"));
    if (at < 0) at = raw.lastIndexOf(QLatin1String("</w:odso>"));
    if (at < 0) return raw.toUtf8();
    return (raw.left(at).toUtf8() + entries + raw.mid(at).toUtf8());
}

} // namespace

QString kindValue(Kind kind)
{
    switch (kind) {
    case Kind::Letters: return QStringLiteral("formLetters");
    case Kind::Email: return QStringLiteral("email");
    case Kind::Envelopes: return QStringLiteral("envelopes");
    case Kind::Labels: return QStringLiteral("mailingLabels");
    case Kind::Directory: return QStringLiteral("catalog");
    case Kind::None: break;
    }
    return QString();
}

Kind kindOfValue(const QString &value)
{
    for (Kind k : {Kind::Letters, Kind::Email, Kind::Envelopes, Kind::Labels, Kind::Directory})
        if (kindValue(k) == value) return k;
    return Kind::None;
}

// ---- the merge a document has

State &stateOf(WriteDoc &doc)
{
    if (!doc.mailMerge) doc.mailMerge = std::make_shared<State>();
    return *doc.mailMerge;
}

State *stateIfAny(const WriteDoc &doc) { return doc.mailMerge.get(); }

void limitRecords(MergeSource &m)
{
    if (m.rows.size() > kMostRecords) m.truncated = true;
    if (m.rows.size() > kMostRecords) m.rows.resize(kMostRecords);
    if (m.include.size() > kMostRecords) m.include.resize(kMostRecords);
}

void dropBlankRecords(MergeSource &m)
{
    QVector<QStringList> rows;
    QVector<bool> include;
    for (int i = 0; i < m.rows.size(); ++i) {
        if (m.rows[i].join(QString()).isEmpty()) continue;
        rows << m.rows[i];
        include << m.include.value(i, true);
    }
    m.rows = rows;
    m.include = include;
}

bool loadSource(State &state, const QString &fromPath)
{
    const QString path = fromPath.isEmpty() ? state.sourcePath : fromPath;
    if (path.isEmpty() || !QFile::exists(path)) return false;
    MergeSource m;
    if (!loadMergeSource(path, &m, nullptr)) return false;
    dropBlankRecords(m);
    limitRecords(m);
    state.data = m;
    state.sourcePath = path;
    state.loaded = true;
    return true;
}

} // namespace jp::write::mail

namespace jp::write {

// What the settings part says of the merge, and where the list is (the settings part's relationship of the data source).
void WriteDoc::readMailSettings(const QByteArray &settings)
{
    office::wml::MailMergeSettings found;
    if (!office::wml::readMailMerge(settings, &found)) return;
    auto state = std::make_shared<mail::State>();
    state->settings = found;
    for (const office::Relationship &r : m_wml.partRelationships.value(m_wml.settingsName))
        if (r.id == found.dataSourceId && !found.dataSourceId.isEmpty()) {
            QString target = r.target;
            if (target.startsWith(QLatin1String("file:"), Qt::CaseInsensitive)) {
                QString local = QUrl(target).toLocalFile();
                if (local.isEmpty()) local = QUrl::fromPercentEncoding(target.mid(8).toUtf8());
                target = local;
            }
            state->sourcePath = target;
        }
    state->fieldMap = state->fieldMapRead = mail::fieldMapOf(found.odso);
    mailMerge = state;
}

// The merge is written when it was changed: the settings with the element, and the data source as a link to the file.
void WriteDoc::writeMailSettings(office::wml::Document *out)
{
    if (!mailMerge || !mailMerge->changed) return;
    mail::State &state = *mailMerge;
    office::wml::MailMergeSettings m = state.settings;
    office::wml::KeptPart *part = settingsPartFor(out);
    QVector<office::Relationship> &rels = out->partRelationships[part->name];
    const QString type = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/mailMergeSource");
    // The link to the list: the file's own relationship where it is still the one, else one of its own.
    for (int i = int(rels.size()) - 1; i >= 0; --i)
        if (rels[i].type == type && rels[i].id == m.dataSourceId && state.sourcePath.isEmpty()) rels.removeAt(i);
    if (!state.sourcePath.isEmpty() && state.kind() != mail::Kind::None) {
        const QString target = QUrl::fromLocalFile(state.sourcePath).toString(QUrl::FullyEncoded);
        bool found = false;
        for (office::Relationship &r : rels)
            if (r.type == type && r.id == m.dataSourceId) {
                r.target = target;
                r.external = true;
                found = true;
            }
        if (!found) {
            office::Relationship r;
            r.id = nextRelationshipId(rels);
            r.type = type;
            r.target = target;
            r.external = true;
            rels.push_back(r);
            m.dataSourceId = r.id;
        }
        m.dataType = QStringLiteral("textFile");
        m.connectString.clear();
        m.query = QStringLiteral("SELECT * FROM ") + QDir::toNativeSeparators(state.sourcePath);
        m.linkToQuery = true;
    } else {
        m.dataSourceId.clear();
    }
    if (state.fieldMap != state.fieldMapRead) m.odso = mail::odsoWithFieldMap(m.odso, state.fieldMap, state.data.fields);
    part->data = office::wml::settingsWithMailMerge(part->data, m);
}

} // namespace jp::write

namespace jp::write::mail {

// ---- the fields of the address block and greeting line

QStringList logicalFields()
{
    return {QStringLiteral("Title"), QStringLiteral("First"),   QStringLiteral("Last"),  QStringLiteral("Suffix"), QStringLiteral("Company"), QStringLiteral("Street1"),
            QStringLiteral("Street2"), QStringLiteral("City"), QStringLiteral("State"), QStringLiteral("Postal"), QStringLiteral("Country"), QStringLiteral("Email")};
}

QString columnFor(const MergeSource &data, const QHash<QString, QString> &map, const QString &logical)
{
    const QString chosen = map.value(logical);
    if (!chosen.isEmpty()) {
        const int i = columnIndex(data, chosen);
        return i >= 0 ? data.fields[i] : QString();
    }
    for (const QString &name : synonyms(logical)) {
        const int i = columnIndex(data, name);
        if (i >= 0) return data.fields[i];
    }
    return QString();
}

QStringList addressBlockFormats()
{
    return {QStringLiteral("{Title} {First} {Last} {Suffix}|{Company}|{Street1}|{Street2}|{City}, {State} {Postal}|{Country}"),
            QStringLiteral("{First} {Last}|{Company}|{Street1}|{Street2}|{City}, {State} {Postal}|{Country}"),
            QStringLiteral("{Title} {Last}|{Company}|{Street1}|{Street2}|{City}, {State} {Postal}|{Country}"),
            QStringLiteral("{Company}|{Title} {First} {Last}|{Street1}|{Street2}|{Postal} {City}|{Country}")};
}

QString addressBlockText(const MergeSource &data, int row, const QHash<QString, QString> &map, const QString &formatIn, const QString &homeCountry)
{
    const QString format = plainFormat(formatIn.isEmpty() ? addressBlockFormats().first() : formatIn);
    const QString country = partValue(data, row, map, QStringLiteral("Country"));
    const bool home = !homeCountry.isEmpty() && country.compare(homeCountry.trimmed(), Qt::CaseInsensitive) == 0;
    auto value = [&](const QString &name) {
        if (name == QLatin1String("Country") && home) return QString();
        return partValue(data, row, map, name);
    };
    QStringList lines;
    for (const QString &line : format.split(QLatin1Char('|'))) {
        bool any = false;
        const QString text = tidy(fill(line, value, &any), true);
        if (any && !text.isEmpty()) lines << text;
    }
    return lines.join(QChar(QChar::LineSeparator));
}

QStringList greetingFormats()
{
    return {QStringLiteral("Dear {Title} {Last},"), QStringLiteral("Dear {First} {Last},"), QStringLiteral("Dear {First},"), QStringLiteral("Dear {Title} {First} {Last},"), QStringLiteral("To {Title} {Last}:"),
            QStringLiteral("Hello {First},")};
}

QString greetingText(const MergeSource &data, int row, const QHash<QString, QString> &map, const QString &formatIn, const QString &fallback)
{
    auto value = [&](const QString &name) { return partValue(data, row, map, name); };
    // (The default format, "Dear {Title} {Last},", reads badly with no title: it uses the first name too then.)
    QString chosen = formatIn.isEmpty() ? greetingFormats().first() : formatIn;
    if (chosen == greetingFormats().first() && value(QStringLiteral("Title")).isEmpty()) chosen = greetingFormats().value(1);
    const QString format = plainFormat(chosen);
    bool any = false;
    // (The name's parts the format asks for: the greeting is the fallback when the record has none of them.)
    bool asks = false;
    for (const QString &name : {QStringLiteral("Title"), QStringLiteral("First"), QStringLiteral("Last"), QStringLiteral("Suffix")}) asks = asks || format.contains(QLatin1Char('{') + name + QLatin1Char('}'));
    const QString text = tidy(fill(format, [&](const QString &name) {
                                  const QString v = value(name);
                                  if (name == QLatin1String("Title") || name == QLatin1String("First") || name == QLatin1String("Last") || name == QLatin1String("Suffix")) any = any || !v.isEmpty();
                                  return v;
                              }, nullptr), false);
    if (asks && !any) return fallback;
    return text;
}

QString mergeFieldCode(const QString &column)
{
    QString name = column.trimmed();
    name.replace(QLatin1Char(' '), QLatin1Char('_'));
    name.replace(QLatin1Char('"'), QLatin1Char('\''));
    return QStringLiteral("MERGEFIELD ") + name;
}

namespace {

QByteArray xmlEscaped(const QString &s)
{
    QString t = s;
    t.replace(QLatin1Char('&'), QLatin1String("&amp;")).replace(QLatin1Char('<'), QLatin1String("&lt;")).replace(QLatin1Char('>'), QLatin1String("&gt;"));
    return t.toUtf8();
}

// The runs of a field whose code may have fields in braces; a field in a code shows what it names as its result.
QByteArray ruleRuns(const QString &code, const QString &result, int depth)
{
    QByteArray raw = "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r>";
    int at = 0;
    auto instr = [&](const QString &text) {
        if (!text.isEmpty()) raw += "<w:r><w:instrText xml:space=\"preserve\">" + xmlEscaped(text) + "</w:instrText></w:r>";
    };
    for (;;) {
        const int open = depth < 4 ? code.indexOf(QLatin1String("{ "), at) : -1;
        const int close = open >= 0 ? code.indexOf(QLatin1String(" }"), open + 2) : -1;
        if (open < 0 || close < 0) {
            instr(code.mid(at));
            break;
        }
        instr(code.mid(at, open - at));
        const QString inner = code.mid(open + 2, close - open - 2);
        const fields::Code parsed = fields::parseCode(inner);
        const QString shown = parsed.name == QLatin1String("MERGEFIELD") ? QString::fromUtf8("\xC2\xAB") + QString(parsed.arguments.value(0)).replace(QLatin1Char('_'), QLatin1Char(' ')) + QString::fromUtf8("\xC2\xBB") : QString();
        raw += ruleRuns(inner, shown, depth + 1);
        at = close + 2;
    }
    raw += "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>";
    if (!result.isEmpty()) raw += "<w:r><w:t xml:space=\"preserve\">" + xmlEscaped(result) + "</w:t></w:r>";
    raw += "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
    return raw;
}

} // namespace

bool insertRule(WriteDoc &doc, QTextCursor &c, const QString &code, const QString &resultIn)
{
    // (A field with no result would show nothing in the editor: it shows its name in guillemets.)
    const QString result = resultIn.isEmpty() ? QStringLiteral("\u00AB") + code.trimmed().section(QLatin1Char(' '), 0, 0) + QStringLiteral("\u2026\u00BB") : resultIn;
    if (!code.contains(QLatin1String("{ "))) return insertField(doc, c, code, result);
    office::wml::Inline field;
    field.kind = office::wml::Inline::ParaAtom;
    field.role = office::wml::Inline::Single;
    field.tag = office::wml::kFieldGroupTag;
    field.raw = ruleRuns(code, result, 0);
    field.display = result;
    return insertFieldAtom(doc, c, field);
}

namespace {

struct BarcodeKind {
    const char *name;
    barcode::Type type;
};
// The types of the field syntax that are drawn here.
const BarcodeKind kBarcodeKinds[] = {{"QR", barcode::Type::Qr},       {"CODE128", barcode::Type::Code128}, {"CODE39", barcode::Type::Code39}, {"EAN13", barcode::Type::Ean13},
                                     {"JAN13", barcode::Type::Ean13}, {"EAN8", barcode::Type::Ean8},       {"JAN8", barcode::Type::Ean8},     {"UPCA", barcode::Type::UpcA}};

} // namespace

namespace {

// The drawn height, in twips, of a barcode of this type at the size and bar height (the whole barcode with its quiet zone and digits); it does not depend on the value, so a sample stands in.
double drawnHeightTwips(barcode::Type type, double magnification, double barHeight, bool text)
{
    barcode::Options o;
    o.type = type;
    o.magnification = magnification;
    o.barHeight = barHeight;
    o.text = text;
    switch (type) {
    case barcode::Type::Code128:
    case barcode::Type::Code39: o.data = QStringLiteral("A"); break;
    case barcode::Type::Ean8: o.data = QStringLiteral("96385074"); break;
    case barcode::Type::UpcA: o.data = QStringLiteral("036000291452"); break;
    default: o.data = QStringLiteral("5901234123457"); break;
    }
    const barcode::Layout l = barcode::make(o);
    return l.error.isEmpty() ? l.size.height() * 20.0 : 0;
}

// QR error correction as the field says it: 0 to 3, or the letters L, M, Q, and H.
int qrLevelOf(const QString &word)
{
    const int at = QStringLiteral("LMQH").indexOf(word.trimmed().toUpper().left(1));
    if (word.trimmed().size() == 1 && at >= 0) return at;
    return std::clamp(word.trimmed().toInt(), 0, 3);
}

} // namespace

QJsonObject barcodeSettingsOfField(const fields::Code &code, const QString &value)
{
    const QString name = code.arguments.value(1).trimmed().toUpper();
    for (const BarcodeKind &k : kBarcodeKinds) {
        if (name != QLatin1String(k.name)) continue;
        QJsonObject s;
        s.insert(QStringLiteral("type"), int(k.type));
        s.insert(QStringLiteral("data"), value);
        const int scale = code.argumentOf(QLatin1Char('s')).toInt();
        s.insert(QStringLiteral("magnification"), std::clamp(scale > 0 ? scale : 100, 50, 300));
        // (\h is the height drawn, in twips: the share of the full bar height it is, at the size asked for, is the dialog's bar height.)
        const int twips = code.argumentOf(QLatin1Char('h')).toInt();
        const double full = twips > 0 ? drawnHeightTwips(k.type, s.value(QLatin1String("magnification")).toInt() / 100.0, 1, code.has(QLatin1Char('t'))) : 0;
        s.insert(QStringLiteral("height"), full > 0 ? std::clamp(int(std::lround(100.0 * twips / full)), 40, 100) : 100);
        s.insert(QStringLiteral("text"), code.has(QLatin1Char('t')));
        s.insert(QStringLiteral("qrLevel"), code.has(QLatin1Char('q')) ? qrLevelOf(code.argumentOf(QLatin1Char('q'))) : 1);
        s.insert(QStringLiteral("white"), true);
        return s;
    }
    return QJsonObject();
}

QString barcodeMark(const QString &nonce, int n) { return QChar(0xE000) + nonce + QLatin1Char(':') + QString::number(n) + QChar(0xE001); }

bool insertBarcodeField(WriteDoc &doc, QTextCursor &c, const QString &column, const QJsonObject &settings)
{
    const barcode::Options o = barcodeOptions(settings);
    // (A book's ISBN is written as EAN-13, the type the field syntax has for its 13 digits.)
    const barcode::Type type = o.type == barcode::Type::Isbn ? barcode::Type::Ean13 : o.type;
    QString kind;
    for (const BarcodeKind &k : kBarcodeKinds)
        if (k.type == type) {
            kind = QString::fromLatin1(k.name);
            break;
        }
    QString name = column.trimmed();
    name.replace(QLatin1Char(' '), QLatin1Char('_'));
    name.replace(QLatin1Char('"'), QLatin1Char('\''));
    if (kind.isEmpty() || name.isEmpty()) return false;
    QString code = QStringLiteral("DISPLAYBARCODE \"{ MERGEFIELD %1 }\" %2 \\s %3").arg(name, kind).arg(int(std::lround(o.magnification * 100)));
    if (type == barcode::Type::Qr) code += QStringLiteral(" \\q %1").arg(o.qrLevel);
    else code += QStringLiteral(" \\h %1").arg(int(std::lround(drawnHeightTwips(type, o.magnification, o.barHeight, o.text))));
    if (o.text && type != barcode::Type::Qr) code += QStringLiteral(" \\t");
    return insertRule(doc, c, code, QString::fromUtf8("\xC2\xAB") + QCoreApplication::translate("jp::write::Mail", "Barcode: %1").arg(column.trimmed()) + QString::fromUtf8("\xC2\xBB"));
}

QString addressBlockCode(const QString &formatIn, const QString &homeCountry)
{
    QString format = formatIn.isEmpty() ? addressBlockFormats().first() : formatIn;
    format.replace(QLatin1Char('"'), QLatin1Char('\''));
    QString code = QStringLiteral("ADDRESSBLOCK \\f \"") + format + QLatin1Char('"');
    if (!homeCountry.isEmpty()) {
        QString home = homeCountry;
        home.replace(QLatin1Char('"'), QLatin1Char('\''));
        code += QStringLiteral(" \\e \"") + home + QLatin1Char('"');
    }
    return code;
}

QString greetingCode(const QString &formatIn, const QString &fallbackIn)
{
    QString format = formatIn.isEmpty() ? greetingFormats().first() : formatIn;
    QString fallback = fallbackIn.isEmpty() ? QCoreApplication::translate("jp::write::Mail", "Dear Sir or Madam,") : fallbackIn;
    format.replace(QLatin1Char('"'), QLatin1Char('\''));
    fallback.replace(QLatin1Char('"'), QLatin1Char('\''));
    return QStringLiteral("GREETINGLINE \\f \"") + format + QStringLiteral("\" \\e \"") + fallback + QLatin1Char('"');
}

// ---- the answers of the fields

QString Evaluator::value(const QString &column) const
{
    const int r = row();
    if (r < 0) return QString();
    const int i = columnIndex(m_data, column);
    return i < 0 ? QString() : m_data.value(r, m_data.fields[i]);
}

bool Evaluator::isMergeField(const fields::Code &code) const
{
    static const QSet<QString> names = {QStringLiteral("MERGEFIELD"), QStringLiteral("ADDRESSBLOCK"), QStringLiteral("GREETINGLINE"), QStringLiteral("NEXT"),   QStringLiteral("NEXTIF"),
                                        QStringLiteral("SKIPIF"),     QStringLiteral("SET"),          QStringLiteral("ASK"),          QStringLiteral("FILLIN"), QStringLiteral("MERGEREC"),
                                        QStringLiteral("MERGESEQ"),   QStringLiteral("IF"),           QStringLiteral("COMPARE"),   QStringLiteral("DISPLAYBARCODE")};
    if (names.contains(code.name)) return true;
    return code.name == QLatin1String("REF") && variables.contains(code.arguments.value(0).trimmed().toLower());
}

bool Evaluator::test(const fields::Code &code) const
{
    fields::Code asIf = code;
    asIf.name = QStringLiteral("IF");
    asIf.arguments << QStringLiteral("1") << QStringLiteral("0");
    fields::Env env;
    return fields::compute(asIf, env).value_or(QStringLiteral("0")) == QLatin1String("1");
}

std::optional<QString> Evaluator::answer(const fields::Code &code)
{
    const QString &n = code.name;
    if (n == QLatin1String("MERGEFIELD")) {
        const QString v = value(code.arguments.value(0));
        if (v.isEmpty()) return QString();
        QString out = fields::formatText(v, code.general);
        if (code.has(QLatin1Char('b'))) out = code.argumentOf(QLatin1Char('b')) + out;
        if (code.has(QLatin1Char('f'))) out += code.argumentOf(QLatin1Char('f'));
        return out;
    }
    if (n == QLatin1String("ADDRESSBLOCK")) {
        if (row() < 0) return QString();
        return addressBlockText(m_data, row(), m_map, code.argumentOf(QLatin1Char('f')), code.argumentOf(QLatin1Char('e')));
    }
    if (n == QLatin1String("GREETINGLINE")) {
        if (row() < 0) return QString();
        return greetingText(m_data, row(), m_map, code.argumentOf(QLatin1Char('f')), code.argumentOf(QLatin1Char('e')).isEmpty() ? QCoreApplication::translate("jp::write::Mail", "Dear Sir or Madam,") : code.argumentOf(QLatin1Char('e')));
    }
    if (n == QLatin1String("DISPLAYBARCODE")) {
        // Only a barcode whose value comes from the list is made (one with the value written out, no type, or a type not drawn here stays as it is).
        if (code.nested.isEmpty()) return std::nullopt;
        static const QRegularExpression markOfResult(QStringLiteral("\\x{E002}(\\d{1,4})\\x{E003}"));
        QString v = code.arguments.value(0);
        QString resolved;
        int at = 0;
        for (auto it = markOfResult.globalMatch(v); it.hasNext();) {
            const QRegularExpressionMatch m = it.next();
            resolved += v.mid(at, int(m.capturedStart()) - at) + code.nested.value(m.captured(1).toInt());
            at = int(m.capturedEnd());
        }
        v = resolved + v.mid(at);
        const QJsonObject settings = barcodeSettingsOfField(code, v);
        if (settings.isEmpty()) return std::nullopt;
        if (v.isEmpty()) return QString();
        if (!barcodePictures) return v;
        if (++barcodesMade > mostBarcodes) {
            tooManyBarcodes = true;
            return QString();
        }
        QString why = barcode::tooLong(barcodeOptions(settings));
        if (why.isEmpty()) why = barcodeLayout(settings).error;
        if (!why.isEmpty()) return QCoreApplication::translate("jp::write::Mail", "[Barcode: %1]").arg(why);
        barcodes << settings;
        return barcodeMark(barcodeNonce, int(barcodes.size()) - 1);
    }
    if (n == QLatin1String("MERGEREC")) return row() < 0 ? QString() : QString::number(row() + 1);
    if (n == QLatin1String("MERGESEQ")) return QString::number(sequence);
    if (n == QLatin1String("NEXT")) {
        ++position;
        return QString();
    }
    if (n == QLatin1String("NEXTIF")) {
        if (test(code)) ++position;
        return QString();
    }
    if (n == QLatin1String("SKIPIF")) {
        if (test(code)) skipped = true;
        return QString();
    }
    if (n == QLatin1String("SET")) {
        const QString name = code.arguments.value(0).trimmed().toLower();
        if (!name.isEmpty() && variables.size() < 1000) variables.insert(name, code.arguments.mid(1).join(QLatin1Char(' ')).left(kMostPromptChars));
        return QString();
    }
    if (n == QLatin1String("REF")) {
        const auto it = variables.constFind(code.arguments.value(0).trimmed().toLower());
        if (it == variables.constEnd()) return std::nullopt;
        return fields::formatText(it.value(), code.general);
    }
    if (n == QLatin1String("ASK") || n == QLatin1String("FILLIN")) {
        const bool fillIn = n == QLatin1String("FILLIN");
        const QString name = fillIn ? QString() : code.arguments.value(0).trimmed().toLower();
        const QString prompt = (fillIn ? code.arguments.value(0) : code.arguments.value(1)).left(kMostPromptChars);
        const QString def = code.argumentOf(QLatin1Char('d')).left(kMostPromptChars);
        const QString key = n + QLatin1Char('|') + name + QLatin1Char('|') + prompt;
        if (!m_answers.contains(key)) {
            const bool may = ask && m_answers.size() < mostPrompts && !(cancelled && *cancelled);
            m_answers.insert(key, (may ? ask(prompt, def) : def).left(kMostPromptChars));
        }
        if (fillIn) return m_answers.value(key);
        if (!name.isEmpty() && variables.size() < 1000) variables.insert(name, m_answers.value(key));
        return QString();
    }
    return std::nullopt;
}

// ---- the merge

namespace {

// Freezes the fields of a merge in a document to their results: from the last to the first, each is replaced by the text of its result.
void freeze(WriteDoc &doc, const Evaluator &ev)
{
    QTextDocument *story = doc.story();
    const StoryScan scan = scanStory(story, false);
    QVector<QPair<int, int>> spans;   // (begin, end) of the fields to freeze that no other of them holds
    for (int i = 0; i < scan.fields.size(); ++i) {
        const FieldPlace &f = scan.fields[i];
        if (f.parent >= 0) continue;
        const fields::Code code = fields::parseCode(scan.instructionOf(i));
        if (ev.isMergeField(code)) spans.push_back({f.begin, f.end});
    }
    for (int k = int(spans.size()) - 1; k >= 0; --k) {
        QTextCursor c(story);
        c.setPosition(spans[k].first);
        c.setPosition(spans[k].second + 1, QTextCursor::KeepAnchor);
        unlinkFields(doc, c);
    }
}

bool pageDependent(const QString &name)
{
    return name == QLatin1String("PAGE") || name == QLatin1String("NUMPAGES") || name == QLatin1String("SECTIONPAGES") || name == QLatin1String("PAGEREF") || name == QLatin1String("TOC") ||
           name == QLatin1String("STYLEREF");
}

QVector<int> rowsOf(const MergeSource &data, const MergeOptions &options)
{
    QVector<int> rows = options.rows.isEmpty() ? data.includedRows() : options.rows;
    rows.erase(std::remove_if(rows.begin(), rows.end(), [&](int r) { return r < 0 || r >= data.rows.size(); }), rows.end());
    if (rows.size() > kMostRecords) rows.resize(kMostRecords);
    return rows;
}

// One merge: runs `each` with the text of the source made for each group of records (a rule may take several records into one), until the records are used.
struct Run {
    std::unique_ptr<WriteDoc> scratch;
    QTextDocumentFragment pristine;
    QByteArray bytes;
};

bool prepare(WriteDoc &source, Run *run, QString *error)
{
    run->bytes = source.toDocx(error);
    if (run->bytes.isEmpty()) return false;
    run->scratch = WriteDoc::fromDocx(run->bytes, error);
    if (!run->scratch) return false;
    run->scratch->story()->setUndoRedoEnabled(false);
    run->pristine = QTextDocumentFragment(run->scratch->story());
    return true;
}

// The source's text for the records from `ev.position` on, merged; `ev.position` is left on the last record the text used. False where a rule skipped the record.
bool stopped(const MergeOptions &o) { return o.cancelled && *o.cancelled; }

// The pages the text of one record will make, as near as its size says (a page holds about 3,000 characters or 45 paragraphs), and at least one.
int pagesOf(const QTextDocument *story) { return std::max({1, int((story->characterCount() + 2999) / 3000), (story->blockCount() + 44) / 45}); }

std::unique_ptr<WriteDoc> stoppedMerge(QString *error)
{
    if (error) *error = QCoreApplication::translate("jp::write::Mail", "The merge was stopped.");
    return nullptr;
}

QStringList stoppedTexts(QString *error)
{
    stoppedMerge(error);
    return QStringList();
}

std::unique_ptr<WriteDoc> tooManyPages(const MergeOptions &o, QString *error)
{
    if (error) *error = QCoreApplication::translate("jp::write::Mail", "The merge would make more than %1 pages. Merge fewer recipients, or a shorter document.").arg(o.mostPages);
    return nullptr;
}

std::unique_ptr<WriteDoc> tooManyBarcodes(const MergeOptions &o, QString *error)
{
    if (error) *error = QCoreApplication::translate("jp::write::Mail", "The merge would make more than %1 barcodes. Merge fewer recipients.").arg(o.mostBarcodes);
    return nullptr;
}

// Puts the barcode pictures in the text of `target` from `from` to the end where the marks of a record's barcodes are (from the last to the first, so that the places stay).
void putBarcodes(WriteDoc &doc, int from, const QVector<QJsonObject> &barcodes, const QString &nonce)
{
    QTextDocument *story = doc.story();
    QTextCursor all(story);
    all.setPosition(from);
    all.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    const QString text = all.selectedText();
    const QRegularExpression mark(QStringLiteral("\\x{E000}") + QRegularExpression::escape(nonce) + QStringLiteral(":(\\d{1,9})\\x{E001}"));
    QVector<QRegularExpressionMatch> found;
    for (auto it = mark.globalMatch(text); it.hasNext();) found << it.next();
    for (int k = int(found.size()) - 1; k >= 0; --k) {
        bool numbered = false;
        const int n = found[k].captured(1).toInt(&numbered);
        QTextCursor c(story);
        c.setPosition(from + int(found[k].capturedStart()));
        c.setPosition(from + int(found[k].capturedEnd()), QTextCursor::KeepAnchor);
        if (!numbered || n < 0 || n >= barcodes.size()) continue;
        c.removeSelectedText();
        if (insertBarcodeInline(doc, c, barcodes[n]) != PictureResult::Ok) c.insertText(QCoreApplication::translate("jp::write::Mail", "[Barcode could not be added]"));
    }
}

bool mergeOne(Run &run, Evaluator &ev)
{
    QTextDocument *story = run.scratch->story();
    {
        QTextCursor c(story);
        c.select(QTextCursor::Document);
        c.insertFragment(run.pristine);
    }
    ev.skipped = false;
    ev.barcodes.clear();
    UpdateOptions o;
    o.merge = [&](const fields::Code &code) { return ev.answer(code); };
    o.skip = [](const QString &name) { return pageDependent(name); };
    updateFields(*run.scratch, o);
    freeze(*run.scratch, ev);
    return !ev.skipped;
}

} // namespace

std::unique_ptr<WriteDoc> mergeToDocument(WriteDoc &source, const MergeSource &data, const QHash<QString, QString> &fieldMap, const MergeOptions &options, QString *error)
{
    Run run;
    if (!prepare(source, &run, error)) return nullptr;
    std::unique_ptr<WriteDoc> out = WriteDoc::fromDocx(run.bytes, error);
    if (!out) return nullptr;
    const QVector<int> rows = rowsOf(data, options);
    QTextDocument *target = out->story();
    target->setUndoRedoEnabled(false);
    {
        QTextCursor c(target);
        c.select(QTextCursor::Document);
        c.removeSelectedText();
    }
    Evaluator ev(data, fieldMap, rows);
    ev.ask = options.ask;
    ev.cancelled = options.cancelled;
    ev.mostBarcodes = options.mostBarcodes;
    ev.barcodeNonce = QString::number(QRandomGenerator::global()->generate64(), 16);
    QTextCursor oc(target);
    bool first = true;
    int made = 0;
    int pages = 0;
    for (ev.position = 0; ev.position < rows.size(); ++ev.position) {
        ++made;
        ev.sequence = made;
        const int begin = ev.position;
        if (stopped(options)) return stoppedMerge(error);
        if (!mergeOne(run, ev)) continue;
        if (ev.tooManyBarcodes) return tooManyBarcodes(options, error);
        if ((pages += pagesOf(run.scratch->story())) > options.mostPages) return tooManyPages(options, error);
        QTextDocumentFragment frag(run.scratch->story());
        oc.movePosition(QTextCursor::End);
        if (!first) oc.insertBlock();
        const int at = oc.position();
        oc.insertFragment(frag);
        {
            // (The fragment's first paragraph joined the empty one made for it, which keeps its own look: it is given the fragment's.)
            const QTextBlock firstOfCopy = run.scratch->story()->begin();
            QTextCursor b(target);
            b.setPosition(at);
            QTextBlockFormat bf = firstOfCopy.blockFormat();
            if (!first && options.pageBreaks) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
            b.setBlockFormat(bf);
            b.setBlockCharFormat(firstOfCopy.charFormat());
        }
        if (!ev.barcodes.isEmpty()) putBarcodes(*out, at, ev.barcodes, ev.barcodeNonce);
        first = false;
        if (options.progress && !options.progress(begin + 1, int(rows.size()))) return stoppedMerge(error);
    }
    if (stopped(options)) return stoppedMerge(error);
    target->setUndoRedoEnabled(true);
    out->syncSections();
    out->setModified(true);
    return out;
}

QStringList mergeToTexts(WriteDoc &source, const MergeSource &data, const QHash<QString, QString> &fieldMap, const MergeOptions &options, QString *error, QVector<int> *sourceRows)
{
    QStringList texts;
    if (sourceRows) sourceRows->clear();
    Run run;
    if (!prepare(source, &run, error)) return texts;
    const QVector<int> rows = rowsOf(data, options);
    Evaluator ev(data, fieldMap, rows);
    ev.ask = options.ask;
    ev.cancelled = options.cancelled;
    ev.barcodePictures = false;
    int made = 0;
    int pages = 0;
    for (ev.position = 0; ev.position < rows.size(); ++ev.position) {
        ev.sequence = ++made;
        const int first = ev.row();
        if (stopped(options)) return stoppedTexts(error);
        if (!mergeOne(run, ev)) continue;
        if ((pages += pagesOf(run.scratch->story())) > options.mostPages) {
            tooManyPages(options, error);
            return QStringList();
        }
        if (sourceRows) *sourceRows << first;
        QString t = run.scratch->story()->toPlainText();
        t.replace(QChar::LineSeparator, QLatin1Char('\n')).replace(QChar::ParagraphSeparator, QLatin1Char('\n')).remove(QChar::ObjectReplacementCharacter);
        texts << t;
        if (options.progress && !options.progress(ev.position + 1, int(rows.size()))) return stoppedTexts(error);
    }
    if (stopped(options)) return stoppedTexts(error);
    return texts;
}

bool addAsFirstSection(WriteDoc &doc, WriteDoc &page)
{
    QTextDocument *story = doc.story();
    const QTextDocumentFragment fragment(page.story());
    const office::wml::SectionProps lastBefore = doc.sectionProps(doc.sectionCount() - 1);   // (the record of the last section is kept on the first paragraph, which the page's text now begins with)
    WriteDoc::StepGroup group(doc);
    QTextCursor c(story);
    c.beginEditBlock();
    c.setPosition(0);
    c.insertBlock();
    c.setPosition(0);
    c.insertFragment(fragment);
    c.endEditBlock();
    // (The fragment's last paragraph went into the empty one made for it: it is the last of the page's section.)
    const QTextBlock last = story->findBlock(c.position());
    doc.syncSections();
    if (!doc.insertSectionBreak(last, QStringLiteral("nextPage"))) return false;
    doc.setSectionProps(doc.sectionCount() - 1, lastBefore);
    doc.setSectionProps(0, page.sectionProps(0));
    return true;
}

QStringList missingColumns(WriteDoc &doc, const MergeSource &data)
{
    QStringList missing;
    const StoryScan scan = scanStory(doc.story(), false);
    for (int i = 0; i < scan.fields.size(); ++i) {
        const fields::Code code = fields::parseCode(scan.instructionOf(i));
        if (code.name != QLatin1String("MERGEFIELD")) continue;
        const QString name = code.arguments.value(0);
        if (!name.isEmpty() && columnIndex(data, name) < 0 && !missing.contains(name, Qt::CaseInsensitive)) missing << name;
    }
    return missing;
}

namespace {

// A header's text as RFC 2047 encoded words of at most 75 characters each (45 bytes of text, whole characters; the first 36, for the room the header's name takes in a line of 76), one to a line,
// the lines after the first folded with a space.
QByteArray encodedWords(const QString &text)
{
    QList<QByteArray> words;
    QByteArray chunk;
    for (int i = 0; i < text.size();) {
        const int length = text[i].isHighSurrogate() && i + 1 < text.size() ? 2 : 1;
        const QByteArray bytes = text.mid(i, length).toUtf8();
        if (chunk.size() + bytes.size() > (words.isEmpty() ? 36 : 45)) {
            words << chunk;
            chunk.clear();
        }
        chunk += bytes;
        i += length;
    }
    if (!chunk.isEmpty() || words.isEmpty()) words << chunk;
    QByteArray out;
    for (int i = 0; i < words.size(); ++i) out += (i ? "\r\n " : "") + QByteArray("=?UTF-8?B?") + words[i].toBase64() + "?=";
    return out;
}

} // namespace

int writeEmailFiles(const QStringList &texts, const MergeSource &data, const QVector<int> &rows, const QString &addressColumn, const QString &subject, const QString &folder, QString *error, int *refused)
{
    int written = 0;
    if (refused) *refused = 0;
    QDir dir(folder);
    if (!dir.exists() && !QDir().mkpath(folder)) {
        if (error) *error = QCoreApplication::translate("jp::write::Mail", "The folder can't be made.");
        return 0;
    }
    for (int i = 0; i < texts.size(); ++i) {
        const int row = rows.value(i, -1);
        const QString to = row >= 0 ? data.value(row, addressColumn).trimmed() : QString();
        if (to.isEmpty()) continue;   // (a record with no address has no message)
        // One person's address (letters, digits, and marks of any language), as RFC 5322 has it without a display name or comments: a comma, a semicolon, a line break, or a name in angle brackets would
        // make the message go to others or add a header.
        static const QRegularExpression one(QStringLiteral("^[^\\s@,;<>()\\[\\]\\\\\":]+@([\\p{L}\\p{N}\\p{M}-]+\\.)+[\\p{L}\\p{N}\\p{M}-]{2,}$"));
        if (!one.match(to).hasMatch()) {
            if (refused) ++*refused;
            continue;
        }
        QString name = to;
        name.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9@._-]")), QStringLiteral("_"));
        QString subj = subject;
        subj.replace(QRegularExpression(QStringLiteral("[\\r\\n]+")), QStringLiteral(" "));
        QByteArray body = texts[i].toUtf8().toBase64(QByteArray::Base64Encoding);
        QByteArray wrapped;
        for (int k = 0; k < body.size(); k += 76) wrapped += body.mid(k, 76) + "\r\n";
        QByteArray head = "To: " + to.toUtf8() + "\r\nSubject: " + encodedWords(subj) + "\r\nMIME-Version: 1.0\r\nContent-Type: text/plain; charset=UTF-8\r\nContent-Transfer-Encoding: base64\r\n\r\n";
        QSaveFile f(dir.filePath(QStringLiteral("%1-%2.eml").arg(row + 1, 5, 10, QLatin1Char('0')).arg(name.left(80))));
        if (!f.open(QIODevice::WriteOnly)) {
            if (error) *error = f.errorString();
            return written;
        }
        f.write(head + wrapped);
        if (!f.commit()) {
            if (error) *error = f.errorString();
            return written;
        }
        ++written;
    }
    return written;
}

} // namespace jp::write::mail
