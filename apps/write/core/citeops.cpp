#include "core/citeops.h"

#include <atomic>

#include "core/editops.h"
#include "wml/xmltree.h"

#include <QCollator>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextCursor>
#include <QFile>
#include <QSaveFile>
#include <QSet>
#include <QStandardPaths>
#include <QUuid>
#include <algorithm>

using namespace jp::office::wml;

namespace jp::write {

namespace {

const char kNsUri[] = "http://schemas.openxmlformats.org/officeDocument/2006/bibliography";

struct TypeInfo {
    const char *name;
    const char *label;
    const char *fields;   // the element names, separated by commas
};

const TypeInfo kTypes[] = {
    {"Book", "Book", "Title,Year,City,Publisher,Edition,StandardNumber,Pages,URL"},
    {"BookSection", "Book Section", "Title,BookTitle,Year,City,Publisher,Pages,Edition"},
    {"JournalArticle", "Journal Article", "Title,JournalName,Year,Month,Pages,Volume,Issue,URL"},
    {"ArticleInAPeriodical", "Article in a Periodical", "Title,PeriodicalTitle,Year,Month,Day,Pages,Volume,Issue,URL"},
    {"ConferenceProceedings", "Conference Proceedings", "Title,ConferenceName,Year,City,Publisher,Pages,Volume"},
    {"Report", "Report", "Title,Institution,Year,City,Publisher,ReportNumber,URL"},
    {"SoundRecording", "Sound Recording", "Title,AlbumTitle,Year,City,Publisher,Medium"},
    {"Performance", "Performance", "Title,Year,City,Publisher,Medium"},
    {"Art", "Art", "Title,Year,City,Institution,Medium"},
    {"DocumentFromInternetSite", "Document From Web Site", "Title,InternetSiteTitle,Year,Month,Day,YearAccessed,MonthAccessed,DayAccessed,URL"},
    {"InternetSite", "Web Site", "Title,Year,Month,Day,YearAccessed,MonthAccessed,DayAccessed,URL"},
    {"Film", "Film", "Title,Year,City,Publisher,Medium"},
    {"Interview", "Interview", "Title,Year,Month,Day,Publisher,Medium"},
    {"Patent", "Patent", "Title,Year,Month,Day,CountryRegion,PatentNumber"},
    {"ElectronicSource", "Electronic Source", "Title,Year,City,Publisher,Medium,URL"},
    {"Case", "Case", "Title,Year,Court,CaseNumber,Reporter,Pages"},
    {"Misc", "Miscellaneous", "Title,Year,City,Publisher,Pages,URL"},
};

const struct {
    const char *name;
    const char *label;
} kFields[] = {
    {"Title", "Title"}, {"Year", "Year"}, {"Month", "Month"}, {"Day", "Day"}, {"City", "City"}, {"Publisher", "Publisher"}, {"Edition", "Edition"}, {"StandardNumber", "Standard number"},
    {"Pages", "Pages"}, {"URL", "Web address"}, {"BookTitle", "Book title"}, {"JournalName", "Journal name"}, {"Volume", "Volume"}, {"Issue", "Issue"}, {"PeriodicalTitle", "Periodical title"},
    {"ConferenceName", "Conference name"}, {"Institution", "Institution"}, {"ReportNumber", "Report number"}, {"AlbumTitle", "Album title"}, {"Medium", "Medium"}, {"InternetSiteTitle", "Web site title"},
    {"YearAccessed", "Year accessed"}, {"MonthAccessed", "Month accessed"}, {"DayAccessed", "Day accessed"}, {"CountryRegion", "Country or region"}, {"PatentNumber", "Patent number"},
    {"Court", "Court"}, {"CaseNumber", "Case number"}, {"Reporter", "Reporter"}, {"ShortTitle", "Short title"},
};

const char *const kStyleNames[] = {"APA", "MLA", "Chicago", "Harvard", "IEEE"};

QString styleFile(const QString &style)
{
    // The file format's own identifier of each style's file, which the sources part names its style by: a file only round-trips when these are written exactly as the format has them.
    if (style == QLatin1String("APA")) return QStringLiteral("\\APASixthEditionOfficeOnline.xsl");
    if (style == QLatin1String("MLA")) return QStringLiteral("\\MLASeventhEditionOfficeOnline.xsl");
    if (style == QLatin1String("Chicago")) return QStringLiteral("\\ChicagoFifteenthEditionSortByAuthor.xsl");
    if (style == QLatin1String("Harvard")) return QStringLiteral("\\HarvardAnglia2008OfficeOnline.xsl");
    return QStringLiteral("\\IEEE2006OfficeOnline.xsl");
}

QString styleFromNames(const QString &styleName, const QString &selected)
{
    for (const QString &candidate : {styleName, selected})
        for (const char *name : kStyleNames)
            if (candidate.contains(QLatin1String(name), Qt::CaseInsensitive)) return QString::fromLatin1(name);
    return QString();
}

bool isSimple(const XmlTree &t, int n) { return t.firstChild(n) < 0; }

} // namespace

QStringList sourceTypes()
{
    QStringList out;
    for (const TypeInfo &t : kTypes) out << QString::fromLatin1(t.name);
    return out;
}

QString sourceTypeLabel(const QString &type)
{
    for (const TypeInfo &t : kTypes)
        if (type == QLatin1String(t.name)) return QCoreApplication::translate("jp::write::Sources", t.label);
    return type;
}

QStringList sourceFieldsFor(const QString &type)
{
    for (const TypeInfo &t : kTypes)
        if (type == QLatin1String(t.name)) return QString::fromLatin1(t.fields).split(QLatin1Char(','));
    return QString::fromLatin1(kTypes[16].fields).split(QLatin1Char(','));
}

QString sourceFieldLabel(const QString &field)
{
    for (const auto &f : kFields)
        if (field == QLatin1String(f.name)) return QCoreApplication::translate("jp::write::Sources", f.label);
    return field;
}

QStringList citationStyles()
{
    QStringList out;
    for (const char *n : kStyleNames) out << QString::fromLatin1(n);
    return out;
}

QString citationStyleLabel(const QString &style)
{
    // (These are the editions the styles follow; newer ones are not built in.)
    if (style == QLatin1String("APA")) return QCoreApplication::translate("jp::write::Sources", "APA (6th edition)");
    if (style == QLatin1String("MLA")) return QCoreApplication::translate("jp::write::Sources", "MLA (7th edition)");
    if (style == QLatin1String("Chicago")) return QCoreApplication::translate("jp::write::Sources", "Chicago (15th edition)");
    return style;
}

QString defaultCitationStyle() { return QStringLiteral("APA"); }

QVector<Source> parseSources(const QByteArray &xml, QString *style, bool *truncated)
{
    QVector<Source> out;
    if (truncated) *truncated = false;
    if (style) style->clear();
    XmlTree t;
    if (xml.isEmpty() || !t.parse(xml, nullptr)) return out;
    const int root = t.root();
    if (root < 0 || t.localName(root) != "Sources") return out;
    if (style) *style = styleFromNames(t.attrAny(root, "StyleName"), t.attrAny(root, "SelectedStyle"));
    for (int n = t.firstChild(root); n >= 0; n = t.next(n)) {
        if (t.localName(n) != "Source") continue;
        if (out.size() >= kMostSources) {
            if (truncated) *truncated = true;
            break;
        }
        Source s;
        s.type.clear();
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            const QByteArray name = t.localName(c);
            if (name == "Tag") s.tag = t.text(c).trimmed();
            else if (name == "SourceType") s.type = t.text(c).trimmed();
            else if (name == "Author") {
                for (int a = t.firstChild(c); a >= 0; a = t.next(a)) {
                    if (t.localName(a) != "Author") {
                        s.authorExtra += t.raw(a);
                        continue;
                    }
                    for (int k = t.firstChild(a); k >= 0; k = t.next(k)) {
                        if (t.localName(k) == "Corporate") s.corporateAuthor = t.text(k).trimmed();
                        else if (t.localName(k) == "NameList") {
                            for (int p = t.firstChild(k); p >= 0 && s.authors.size() < 200; p = t.next(p)) {
                                if (t.localName(p) != "Person") continue;
                                Person person;
                                for (int q = t.firstChild(p); q >= 0; q = t.next(q)) {
                                    const QByteArray pn = t.localName(q);
                                    if (pn == "Last") person.last = t.text(q).trimmed();
                                    else if (pn == "First") person.first = t.text(q).trimmed();
                                    else if (pn == "Middle") person.middle = t.text(q).trimmed();
                                }
                                s.authors.push_back(person);
                            }
                        }
                    }
                }
            } else if (name == "RefOrder") {
                // (Written again, in the order the sources are in.)
            } else if (isSimple(t, c)) {
                s.fields.insert(QString::fromLatin1(name), t.text(c).trimmed());
            } else {
                s.extra += t.raw(c);
            }
        }
        if (s.type.isEmpty()) s.type = QStringLiteral("Misc");
        if (!s.tag.isEmpty()) out.push_back(s);
    }
    return out;
}

QByteArray sourcesToXml(const QVector<Source> &sources, const QString &styleIn)
{
    const QString style = styleIn.isEmpty() ? defaultCitationStyle() : styleIn;
    auto esc = [](const QString &s) { return escapeXml(s, false); };
    QByteArray out = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\r\n<b:Sources SelectedStyle=\"" + escapeXml(styleFile(style), true) + "\" StyleName=\"" + escapeXml(style, true) +
                     "\" xmlns:b=\"" + kNsUri + "\" xmlns=\"" + kNsUri + "\">";
    int order = 0;
    for (const Source &s : sources) {
        out += "<b:Source><b:Tag>" + esc(s.tag) + "</b:Tag><b:SourceType>" + esc(s.type) + "</b:SourceType>";
        for (auto it = s.fields.cbegin(); it != s.fields.cend(); ++it) {
            if (it.value().isEmpty()) continue;
            const QByteArray name = it.key().toLatin1();
            out += "<b:" + name + ">" + esc(it.value()) + "</b:" + name + ">";
        }
        if (!s.authors.isEmpty() || !s.corporateAuthor.isEmpty() || !s.authorExtra.isEmpty()) {
            out += "<b:Author>";
            if (!s.authors.isEmpty() || !s.corporateAuthor.isEmpty()) {
                out += "<b:Author>";
                if (!s.authors.isEmpty()) {
                    out += "<b:NameList>";
                    for (const Person &p : s.authors) {
                        out += "<b:Person>";
                        if (!p.last.isEmpty()) out += "<b:Last>" + esc(p.last) + "</b:Last>";
                        if (!p.first.isEmpty()) out += "<b:First>" + esc(p.first) + "</b:First>";
                        if (!p.middle.isEmpty()) out += "<b:Middle>" + esc(p.middle) + "</b:Middle>";
                        out += "</b:Person>";
                    }
                    out += "</b:NameList>";
                } else {
                    out += "<b:Corporate>" + esc(s.corporateAuthor) + "</b:Corporate>";
                }
                out += "</b:Author>";
            }
            out += s.authorExtra + "</b:Author>";
        }
        out += s.extra;
        out += "<b:RefOrder>" + QByteArray::number(++order) + "</b:RefOrder></b:Source>";
    }
    return out + "</b:Sources>";
}

namespace {
std::atomic<qint64> g_sourceParses{0};

// The sources of one revision of the document's sources part, read once for the many CITATION fields a pass works out.
struct Cached {
    quint64 revision = 0;
    QVector<Source> sources;
    QHash<QString, int> byTag;   // lower case tag -> the first source with it
    QString style;
    bool valid = false;
};

const Cached &cachedSources(WriteDoc &doc)
{
    static thread_local Cached c;
    const QByteArray xml = doc.sourcesXml();   // (first: it gives the revision of what it returns)
    const quint64 revision = doc.sourcesRevision();
    if (!c.valid || c.revision != revision) {
        c.revision = revision;
        c.byTag.clear();
        c.sources = parseSources(xml, &c.style);
        ++g_sourceParses;
        for (int i = int(c.sources.size()) - 1; i >= 0; --i) c.byTag.insert(c.sources[i].tag.toLower(), i);   // (backward, so the first source of a tag is the one kept)
        c.valid = true;
    }
    return c;
}

const Source *findSource(const Cached &c, const QString &tag)
{
    const auto found = c.byTag.constFind(tag.toLower());
    return found == c.byTag.constEnd() ? nullptr : &c.sources[found.value()];
}

} // namespace

qint64 sourcesParsesForTest() { return g_sourceParses; }
void resetSourcesParsesForTest() { g_sourceParses = 0; }

QVector<Source> documentSources(WriteDoc &doc) { return cachedSources(doc).sources; }

QString documentCitationStyle(WriteDoc &doc)
{
    const QString s = cachedSources(doc).style;
    return s.isEmpty() ? defaultCitationStyle() : s;
}

void setDocumentSources(WriteDoc &doc, const QVector<Source> &sources) { doc.setSourcesXml(sourcesToXml(sources, documentCitationStyle(doc))); }

QString masterSourcesPath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dir.isEmpty() ? QString() : QDir(dir).filePath(QStringLiteral("sources.xml"));
}

QVector<Source> masterSources(const QString &pathIn)
{
    const QString path = pathIn.isEmpty() ? masterSourcesPath() : pathIn;
    QFile f(path);
    if (path.isEmpty() || !f.open(QIODevice::ReadOnly) || f.size() > (qint64(32) << 20)) return {};
    return parseSources(f.readAll());
}

bool setMasterSources(const QVector<Source> &sources, const QString &pathIn)
{
    const QString path = pathIn.isEmpty() ? masterSourcesPath() : pathIn;
    if (path.isEmpty()) return false;
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(sourcesToXml(sources, defaultCitationStyle()));
    return f.commit();
}

QString newSourceTag(const Source &s, const QVector<Source> &taken)
{
    QString base;
    if (!s.authors.isEmpty()) base = s.authors.first().last;
    else if (!s.corporateAuthor.isEmpty()) base = s.corporateAuthor;
    else base = s.fields.value(QStringLiteral("Title"));
    QString letters;
    for (const QChar ch : base)
        if (ch.isLetter() && letters.size() < 3) letters += ch;
    if (letters.isEmpty()) letters = QStringLiteral("Src");
    letters[0] = letters[0].toUpper();
    const QString year = s.fields.value(QStringLiteral("Year")).trimmed();
    QString tag = letters + (year.size() >= 2 ? year.right(2) : QString());
    QSet<QString> used;
    for (const Source &o : taken) used.insert(o.tag.toLower());
    QString candidate = tag;
    for (int n = 2; used.contains(candidate.toLower()); ++n) candidate = tag + QString::number(n);
    return candidate;
}

namespace {

QString authorLastNames(const Source &s)
{
    QStringList l;
    for (const Person &p : s.authors) l << p.last;
    return l.join(QLatin1Char(' '));
}

QString title(const Source &s) { return s.fields.value(QStringLiteral("Title")); }

} // namespace

QString sourceSummary(const Source &s)
{
    QString who = authorLastNames(s);
    if (s.authors.size() > 1) {
        QStringList l;
        for (const Person &p : s.authors) l << p.last;
        who = l.join(QStringLiteral("; "));
    }
    if (who.isEmpty()) who = s.corporateAuthor;
    QStringList parts;
    if (!who.isEmpty()) parts << who;
    if (!s.fields.value(QStringLiteral("Year")).isEmpty()) parts << s.fields.value(QStringLiteral("Year"));
    if (!title(s).isEmpty()) parts << title(s);
    return parts.join(QStringLiteral(", "));
}

namespace {

// "J. A." from a first and a middle name.
QString initials(const Person &p, bool spaced)
{
    QString out;
    auto add = [&](const QString &name) {
        for (const QString &part : name.split(QRegularExpression(QStringLiteral("[\\s]+")), Qt::SkipEmptyParts)) {
            if (!out.isEmpty()) out += spaced ? QStringLiteral(" ") : QString();
            out += part[0].toUpper() + QLatin1Char('.');
        }
    };
    add(p.first);
    add(p.middle);
    return out;
}

QString fullFirst(const Person &p) { return (p.first + QLatin1Char(' ') + p.middle).simplified(); }

QString citedNames(const Source &s, const QString &style)
{
    const QString and_ = style == QLatin1String("APA") ? QStringLiteral("&") : QCoreApplication::translate("jp::write::Sources", "and");
    if (s.authors.isEmpty()) return s.corporateAuthor;
    if (s.authors.size() == 1) return s.authors[0].last;
    if (s.authors.size() == 2) return s.authors[0].last + QLatin1Char(' ') + and_ + QLatin1Char(' ') + s.authors[1].last;
    return s.authors[0].last + QLatin1Char(' ') + QCoreApplication::translate("jp::write::Sources", "et al.");
}

QString pagesLabel(const QString &pages)
{
    const bool many = pages.contains(QLatin1Char('-')) || pages.contains(QChar(0x2013)) || pages.contains(QLatin1Char(','));
    return (many ? QStringLiteral("pp. ") : QStringLiteral("p. ")) + pages;
}

} // namespace

QString citationText(const Source &s, const QString &style, const fields::Code &code, int number)
{
    const QString pages = code.argumentOf(QLatin1Char('p')).trimmed();
    const bool noAuthor = code.has(QLatin1Char('n')), noYear = code.has(QLatin1Char('y')), withTitle = code.has(QLatin1Char('t'));
    const QString year = s.fields.value(QStringLiteral("Year")).trimmed();
    QString who = noAuthor ? QString() : citedNames(s, style);
    if (who.isEmpty() && !noAuthor) who = s.fields.value(QStringLiteral("ShortTitle")).isEmpty() ? title(s) : s.fields.value(QStringLiteral("ShortTitle"));
    const QString t = withTitle ? (s.fields.value(QStringLiteral("ShortTitle")).isEmpty() ? title(s) : s.fields.value(QStringLiteral("ShortTitle"))) : QString();
    QStringList parts;
    if (style == QLatin1String("IEEE")) {
        QString inner = QString::number(std::max(number, 1));
        if (!pages.isEmpty()) inner += QStringLiteral(", ") + pagesLabel(pages);
        return QLatin1Char('[') + inner + QLatin1Char(']');
    }
    if (style == QLatin1String("MLA")) {
        QString inner = who;
        if (!t.isEmpty() && who != t) inner += (inner.isEmpty() ? QString() : QStringLiteral(", ")) + t;
        if (!pages.isEmpty()) inner += (inner.isEmpty() ? QString() : QStringLiteral(" ")) + pages;
        return QLatin1Char('(') + inner + QLatin1Char(')');
    }
    if (style == QLatin1String("Chicago")) {
        QString inner = who;
        if (!t.isEmpty() && who != t) inner += (inner.isEmpty() ? QString() : QStringLiteral(", ")) + t;
        if (!noYear && !year.isEmpty()) inner += (inner.isEmpty() ? QString() : QStringLiteral(" ")) + year;
        if (!pages.isEmpty()) inner += (inner.isEmpty() ? QString() : QStringLiteral(", ")) + pages;
        return QLatin1Char('(') + inner + QLatin1Char(')');
    }
    // APA and Harvard.
    if (!who.isEmpty()) parts << who;
    if (!t.isEmpty() && who != t) parts << t;
    if (!noYear && !year.isEmpty()) parts << year;
    if (!pages.isEmpty()) parts << pagesLabel(pages);
    if (style == QLatin1String("Harvard") && parts.size() >= 2 && !noYear && !year.isEmpty() && !noAuthor) {
        // (Harvard has no comma between the author and the year.)
        const int at = int(parts.indexOf(year));
        if (at > 0) {
            parts[at - 1] += QLatin1Char(' ') + parts[at];
            parts.removeAt(at);
        }
    }
    return QLatin1Char('(') + parts.join(QStringLiteral(", ")) + QLatin1Char(')');
}

namespace {

// The sources the text cites, in the order of their first citation, and the number each has.
struct Citations {
    QStringList order;            // tags in the order of their first citation (as the sources spell them)
    QHash<QString, int> number;   // lower case tag -> 1...
};

Citations citationsOf(const StoryScan &scan, int exceptWithin = -1)
{
    Citations c;
    const ParsedFields &cites = parsedFieldsNamed(scan, QStringLiteral("CITATION"));
    for (int k = 0; k < cites.index.size(); ++k) {
        const FieldPlace &f = scan.fields[cites.index[k]];
        if (exceptWithin >= 0 && f.begin > scan.fields[exceptWithin].begin && f.begin < scan.fields[exceptWithin].end) continue;
        const QString tag = cites.code[k].arguments.value(0).trimmed();
        if (tag.isEmpty() || c.number.contains(tag.toLower())) continue;
        c.number.insert(tag.toLower(), int(c.order.size()) + 1);
        c.order << tag;
    }
    return c;
}

// The citations of the whole scan, worked out once for it (a new scan, whatever its address, has its own).
const Citations &citationsOfScan(const StoryScan &scan)
{
    const QString key = QStringLiteral("citations");
    auto found = scan.memo.constFind(key);
    if (found == scan.memo.constEnd()) found = scan.memo.insert(key, std::make_shared<Citations>(citationsOf(scan)));
    return *static_cast<const Citations *>(found.value().get());
}

} // namespace

std::optional<QString> citationFieldText(WriteDoc &doc, const StoryScan &scan, const QString &tag, const fields::Code &code)
{
    const Cached &c = cachedSources(doc);
    const Source *s = findSource(c, tag);
    if (!s) return std::nullopt;
    const QString style = c.style.isEmpty() ? defaultCitationStyle() : c.style;
    int number = 0;
    if (style == QLatin1String("IEEE")) {
        const Citations &cache = citationsOfScan(scan);
        number = cache.number.value(tag.toLower(), 0);
    }
    return fields::formatText(citationText(*s, style, code, number), code.general);
}

namespace {

struct Entry {
    QVector<GenRun> runs;
};

void T(QVector<GenRun> *r, const QString &text, bool italic = false)
{
    if (text.isEmpty()) return;
    r->push_back(GenRun{text, false, italic});
}

QString endWith(const QString &s, const QString &mark)
{
    return s.isEmpty() || s.endsWith(mark) || s.endsWith(QLatin1Char('.')) || s.endsWith(QLatin1Char('?')) || s.endsWith(QLatin1Char('!')) ? s : s + mark;
}

QString authorsFor(const Source &s, const QString &style)
{
    QStringList names;
    const int n = int(s.authors.size());
    if (n == 0) return s.corporateAuthor;
    if (style == QLatin1String("APA")) {
        for (const Person &p : s.authors) names << (p.last + (initials(p, true).isEmpty() ? QString() : QStringLiteral(", ") + initials(p, true)));
        if (n == 1) return names[0];
        return names.mid(0, n - 1).join(QStringLiteral(", ")) + (n > 2 ? QStringLiteral(", & ") : QStringLiteral(", & ")) + names.last();
    }
    if (style == QLatin1String("Harvard")) {
        for (const Person &p : s.authors) names << (p.last + (initials(p, false).isEmpty() ? QString() : QStringLiteral(", ") + initials(p, false)));
        if (n == 1) return names[0];
        return names.mid(0, n - 1).join(QStringLiteral(", ")) + QStringLiteral(" and ") + names.last();
    }
    if (style == QLatin1String("IEEE")) {
        for (const Person &p : s.authors) names << ((initials(p, true) + QLatin1Char(' ') + p.last).trimmed());
        if (n == 1) return names[0];
        if (n == 2) return names[0] + QStringLiteral(" and ") + names[1];
        return names.mid(0, n - 1).join(QStringLiteral(", ")) + QStringLiteral(", and ") + names.last();
    }
    // MLA and Chicago: the first name reversed.
    for (int i = 0; i < n; ++i) {
        const Person &p = s.authors[i];
        names << (i == 0 ? (p.last + (fullFirst(p).isEmpty() ? QString() : QStringLiteral(", ") + fullFirst(p))) : (fullFirst(p) + QLatin1Char(' ') + p.last).trimmed());
    }
    if (style == QLatin1String("MLA") && n >= 3) return names[0] + QStringLiteral(", et al");
    if (n == 1) return names[0];
    if (n == 2) return names[0] + QStringLiteral(", and ") + names[1];
    return names.mid(0, n - 1).join(QStringLiteral(", ")) + QStringLiteral(", and ") + names.last();
}

} // namespace

QVector<GenRun> bibliographyEntry(const Source &s, const QString &style, int number)
{
    QVector<GenRun> r;
    auto f = [&](const char *name) { return s.fields.value(QString::fromLatin1(name)).trimmed(); };
    const QString type = s.type;
    const bool article = type == QLatin1String("JournalArticle") || type == QLatin1String("ArticleInAPeriodical") || type == QLatin1String("ConferenceProceedings") || type == QLatin1String("BookSection");
    const bool web = type == QLatin1String("InternetSite") || type == QLatin1String("DocumentFromInternetSite") || type == QLatin1String("ElectronicSource");
    const QString who = authorsFor(s, style);
    const QString ttl = f("Title"), year = f("Year"), city = f("City"), publisher = f("Publisher"), pages = f("Pages"), volume = f("Volume"), issue = f("Issue"), url = f("URL");
    QString journal = type == QLatin1String("ArticleInAPeriodical") ? f("PeriodicalTitle") : type == QLatin1String("ConferenceProceedings") ? f("ConferenceName") : type == QLatin1String("BookSection") ? f("BookTitle") : f("JournalName");
    const QString site = f("InternetSiteTitle");
    const QString lq(QChar(0x201C)), rq(QChar(0x201D));
    auto place = [&](const QString &sep) { return city.isEmpty() ? publisher : publisher.isEmpty() ? city : city + sep + publisher; };
    if (style == QLatin1String("APA")) {
        T(&r, who.isEmpty() ? QString() : endWith(who, QStringLiteral(".")) + QLatin1Char(' '));
        T(&r, QStringLiteral("(") + (year.isEmpty() ? QStringLiteral("n.d.") : year) + QStringLiteral("). "));
        if (article) {
            T(&r, endWith(ttl, QStringLiteral(".")) + QLatin1Char(' '));
            T(&r, journal, true);
            if (!volume.isEmpty()) T(&r, QStringLiteral(", ") + volume, true);
            if (!issue.isEmpty()) T(&r, QStringLiteral("(") + issue + QLatin1Char(')'));
            if (!pages.isEmpty()) T(&r, QStringLiteral(", ") + pages);
            T(&r, QStringLiteral("."));
        } else if (web) {
            T(&r, endWith(ttl, QStringLiteral(".")), false);
            if (!url.isEmpty()) T(&r, QStringLiteral(" Retrieved from ") + url);
        } else {
            T(&r, ttl, true);
            T(&r, QStringLiteral(". "));
            T(&r, place(QStringLiteral(": ")));
            T(&r, place(QString()).isEmpty() ? QString() : QStringLiteral("."));
        }
    } else if (style == QLatin1String("MLA")) {
        T(&r, who.isEmpty() ? QString() : endWith(who, QStringLiteral(".")) + QLatin1Char(' '));
        if (article) {
            T(&r, lq + endWith(ttl, QStringLiteral(".")) + rq + QLatin1Char(' '));
            T(&r, journal, true);
            if (!volume.isEmpty()) T(&r, QLatin1Char(' ') + volume + (issue.isEmpty() ? QString() : QLatin1Char('.') + issue));
            if (!year.isEmpty()) T(&r, QStringLiteral(" (") + year + QLatin1Char(')'));
            if (!pages.isEmpty()) T(&r, QStringLiteral(": ") + pages);
            T(&r, QStringLiteral(". Print."));
        } else if (web) {
            T(&r, lq + endWith(ttl, QStringLiteral(".")) + rq + QLatin1Char(' '));
            if (!site.isEmpty()) T(&r, endWith(site, QStringLiteral(".")), true), T(&r, QStringLiteral(" "));
            if (!publisher.isEmpty()) T(&r, publisher + (year.isEmpty() ? QString() : QStringLiteral(", ")));
            T(&r, year.isEmpty() ? QString() : year + QStringLiteral(". "));
            T(&r, QStringLiteral("Web."));
            if (!url.isEmpty()) T(&r, QStringLiteral(" <") + url + QStringLiteral(">."));
        } else {
            T(&r, endWith(ttl, QStringLiteral(".")), true);
            T(&r, QStringLiteral(" "));
            T(&r, place(QStringLiteral(": ")) + (year.isEmpty() ? QString() : QStringLiteral(", ")));
            T(&r, year.isEmpty() ? QString() : year);
            T(&r, QStringLiteral(". Print."));
        }
    } else if (style == QLatin1String("Chicago")) {
        T(&r, who.isEmpty() ? QString() : endWith(who, QStringLiteral(".")) + QLatin1Char(' '));
        if (article) {
            T(&r, lq + endWith(ttl, QStringLiteral(".")) + rq + QLatin1Char(' '));
            T(&r, journal, true);
            if (!volume.isEmpty()) T(&r, QLatin1Char(' ') + volume);
            if (!issue.isEmpty()) T(&r, QStringLiteral(", no. ") + issue);
            if (!year.isEmpty()) T(&r, QStringLiteral(" (") + year + QLatin1Char(')'));
            if (!pages.isEmpty()) T(&r, QStringLiteral(": ") + pages);
            T(&r, QStringLiteral("."));
        } else if (web) {
            T(&r, lq + endWith(ttl, QStringLiteral(".")) + rq + QLatin1Char(' '));
            if (!site.isEmpty()) T(&r, endWith(site, QStringLiteral(".")), true), T(&r, QStringLiteral(" "));
            if (!url.isEmpty()) T(&r, url + QStringLiteral("."));
        } else {
            T(&r, endWith(ttl, QStringLiteral(".")), true);
            T(&r, QStringLiteral(" "));
            T(&r, place(QStringLiteral(": ")) + (year.isEmpty() ? QString() : QStringLiteral(", ")));
            T(&r, year.isEmpty() ? QString() : year);
            T(&r, QStringLiteral("."));
        }
    } else if (style == QLatin1String("Harvard")) {
        T(&r, who.isEmpty() ? QString() : who + QLatin1Char(' '));
        T(&r, QStringLiteral("(") + (year.isEmpty() ? QStringLiteral("n.d.") : year) + QStringLiteral(") "));
        if (article) {
            T(&r, QStringLiteral("'") + ttl + QStringLiteral("', "));
            T(&r, journal, true);
            if (!volume.isEmpty()) T(&r, QStringLiteral(", ") + volume);
            if (!issue.isEmpty()) T(&r, QStringLiteral("(") + issue + QLatin1Char(')'));
            if (!pages.isEmpty()) T(&r, QStringLiteral(", pp. ") + pages);
            T(&r, QStringLiteral("."));
        } else if (web) {
            T(&r, ttl, true);
            T(&r, QStringLiteral("."));
            if (!url.isEmpty()) T(&r, QStringLiteral(" Available at: ") + url + QLatin1Char('.'));
        } else {
            T(&r, ttl, true);
            T(&r, QStringLiteral(". "));
            T(&r, place(QStringLiteral(": ")));
            T(&r, place(QString()).isEmpty() ? QString() : QStringLiteral("."));
        }
    } else {
        // IEEE.
        T(&r, QStringLiteral("[%1] ").arg(std::max(number, 1)));
        T(&r, who.isEmpty() ? QString() : who + QStringLiteral(", "));
        if (article) {
            T(&r, lq + ttl + QStringLiteral(",") + rq + QLatin1Char(' '));
            T(&r, journal, true);
            if (!volume.isEmpty()) T(&r, QStringLiteral(", vol. ") + volume);
            if (!issue.isEmpty()) T(&r, QStringLiteral(", no. ") + issue);
            if (!pages.isEmpty()) T(&r, QStringLiteral(", pp. ") + pages);
            if (!year.isEmpty()) T(&r, QStringLiteral(", ") + year);
            T(&r, QStringLiteral("."));
        } else if (web) {
            T(&r, lq + ttl + QStringLiteral(",") + rq + QLatin1Char(' '));
            if (!year.isEmpty()) T(&r, year + QStringLiteral(". "));
            if (!url.isEmpty()) T(&r, QStringLiteral("[Online]. Available: ") + url);
        } else {
            T(&r, ttl, true);
            if (!place(QStringLiteral(": ")).isEmpty()) T(&r, QStringLiteral(", ") + place(QStringLiteral(": ")));
            if (!year.isEmpty()) T(&r, QStringLiteral(", ") + year);
            T(&r, QStringLiteral("."));
        }
    }
    return r;
}

QString noBibliography() { return QCoreApplication::translate("jp::write::Fields", "No sources are cited in this document."); }

QVector<GenLine> bibliographyLines(WriteDoc &doc, const StoryScan &scan, int field, const fields::Code &)
{
    const Cached &cached = cachedSources(doc);
    const QString style = cached.style.isEmpty() ? defaultCitationStyle() : cached.style;
    const Citations cites = citationsOf(scan, field);
    struct Item {
        const Source *source;
        int number;
    };
    QVector<Item> items;
    for (const QString &tag : cites.order)
        if (const Source *s = findSource(cached, tag)) items.push_back(Item{s, cites.number.value(tag.toLower())});
    if (style != QLatin1String("IEEE")) {
        QCollator collator;
        collator.setCaseSensitivity(Qt::CaseInsensitive);
        auto key = [](const Source &s) {
            QStringList names;
            for (const Person &p : s.authors) names << p.last << p.first;
            if (names.isEmpty()) names << s.corporateAuthor;
            return names.join(QLatin1Char(' ')).trimmed();
        };
        std::stable_sort(items.begin(), items.end(), [&](const Item &a, const Item &b) {
            const QString ka = key(*a.source).isEmpty() ? title(*a.source) : key(*a.source), kb = key(*b.source).isEmpty() ? title(*b.source) : key(*b.source);
            if (const int c = collator.compare(ka, kb); c != 0) return c < 0;
            if (const int c = collator.compare(a.source->fields.value(QStringLiteral("Year")), b.source->fields.value(QStringLiteral("Year"))); c != 0) return c < 0;
            return collator.compare(title(*a.source), title(*b.source)) < 0;
        });
    }
    QVector<GenLine> out;
    for (const Item &i : items) {
        GenLine l;
        l.styleName = QStringLiteral("Bibliography");
        l.styleId = QStringLiteral("Bibliography");
        l.runs = bibliographyEntry(*i.source, style, i.number);
        out.push_back(l);
    }
    return out;
}

void setDocumentCitationStyle(WriteDoc &doc, const QString &style)
{
    const QVector<Source> sources = documentSources(doc);
    doc.setSourcesXml(sourcesToXml(sources, style));
    UpdateOptions u;
    u.skip = [](const QString &name) { return name != QLatin1String("CITATION") && name != QLatin1String("BIBLIOGRAPHY"); };
    updateFields(doc, u);
}

bool insertCitation(WriteDoc &doc, QTextCursor &c, const QString &tag, const CitationOptions &o)
{
    const Source *s = findSource(cachedSources(doc), tag);
    if (!s) return false;
    QString code = QStringLiteral("CITATION %1").arg(s->tag);
    if (!o.pages.trimmed().isEmpty()) code += QStringLiteral(" \\p \"%1\"").arg(QString(o.pages.trimmed()).replace(QLatin1Char('"'), QString()));
    if (o.suppressAuthor) code += QStringLiteral(" \\n");
    if (o.suppressYear) code += QStringLiteral(" \\y");
    if (!o.suppressTitle) code += QStringLiteral(" \\t");
    code += QStringLiteral(" \\l 1033");
    QTextCursor at = c;
    at.setPosition(c.selectionEnd());
    if (!insertField(doc, at, code)) return false;
    c = at;
    return true;
}

bool insertBibliography(WriteDoc &doc, QTextCursor &c, BibliographyKind kind)
{
    QTextDocument *story = doc.story();
    if (c.document() != story || !intoBody(c)) return false;
    WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    const QString heading = kind == BibliographyKind::References ? QCoreApplication::translate("jp::write::Sources", "References")
                            : kind == BibliographyKind::WorksCited ? QCoreApplication::translate("jp::write::Sources", "Works Cited") : QCoreApplication::translate("jp::write::Sources", "Bibliography");
    const quint64 sheet = doc.styleSheetRevision();
    const QString headingStyle = QStringLiteral("Heading1");
    if (doc.ensureStyle(headingStyle) && doc.styleSheetRevision() != sheet) doc.markStyleStep();
    QTextCursor at(story);
    at.setPosition(c.selectionEnd());
    if (!at.block().text().isEmpty()) {
        at.movePosition(QTextCursor::EndOfBlock);
        at.insertBlock(doc.styleBlockFormat(doc.defaultParagraphStyle()), doc.styleCharFormat(doc.defaultParagraphStyle()));
    }
    at.setBlockFormat(doc.styleBlockFormat(headingStyle));
    at.insertText(heading, doc.styleCharFormat(headingStyle));
    at.insertBlock(doc.styleBlockFormat(doc.defaultParagraphStyle()), doc.styleCharFormat(doc.defaultParagraphStyle()));
    QTextCursor put(story);
    put.setPosition(at.position());
    if (!insertTableFromCode(doc, put, QStringLiteral(" BIBLIOGRAPHY \\l 1033 "))) return false;
    c = put;
    return true;
}

} // namespace jp::write
