#include "core/fieldops.h"

#include "core/writeprops.h"
#include "wml/wml.h"

#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocument>
#include <QXmlStreamReader>
#include <atomic>

using namespace jp::office::wml;

namespace jp::write {

int nextFieldId()
{
    static std::atomic<int> last{0};
    return 1000000 + ++last;   // (above the numbers the story is filled with)
}

namespace {

QString unescaped(QString text)
{
    if (!text.contains(QLatin1Char('&'))) return text;
    text.replace(QLatin1String("&lt;"), QLatin1String("<")).replace(QLatin1String("&gt;"), QLatin1String(">")).replace(QLatin1String("&quot;"), QLatin1String("\"")).replace(QLatin1String("&apos;"), QLatin1String("'"));
    text.replace(QLatin1String("&amp;"), QLatin1String("&"));
    return text;
}

// The words of an element kept as written.
QString wordsOf(const QByteArray &xml)
{
    QString text = QString::fromUtf8(xml);
    text.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
    return unescaped(text);
}

QString attributeOf(const QByteArray &xml, const char *name)
{
    const QByteArray head = xml.left(xml.indexOf('>') < 0 ? 1000 : int(xml.indexOf('>')));
    for (const QByteArray &prefix : {QByteArray(" w:"), QByteArray(" ")}) {
        const QByteArray key = prefix + name + "=\"";
        const qsizetype at = head.indexOf(key);
        if (at < 0) continue;
        const qsizetype to = head.indexOf('"', at + key.size());
        if (to > 0) return unescaped(QString::fromUtf8(head.mid(at + key.size(), to - at - key.size())));
    }
    return QString();
}

bool flagOf(const QByteArray &xml, const char *name)
{
    const QString v = attributeOf(xml, name);
    return v == QLatin1String("1") || v == QLatin1String("true") || v == QLatin1String("on");
}

QString nameOfCode(const QString &code)
{
    const QString t = code.trimmed();
    if (t.startsWith(QLatin1Char('='))) return QStringLiteral("=");
    int i = 0;
    while (i < t.size() && !t[i].isSpace() && t[i] != QLatin1Char('\\') && t[i] != QLatin1Char('"')) ++i;
    return t.left(i).toUpper();
}

struct Open {
    int index;
    bool separated;
};

} // namespace

namespace {
std::atomic<qint64> g_parsedMarks{0}, g_scans{0};
}

const ParsedFields &parsedFieldsNamed(const StoryScan &scan, const QString &name)
{
    const QString key = QStringLiteral("fields:") + name;
    auto found = scan.memo.constFind(key);
    if (found == scan.memo.constEnd()) {
        auto made = std::make_shared<ParsedFields>();
        for (int i = 0; i < scan.fields.size(); ++i) {
            if (scan.fields[i].name != name) continue;
            fields::Code code = fields::parseCode(scan.instructionOf(i));
            ++g_parsedMarks;
            if (code.name != name) continue;
            made->index.push_back(i);
            made->code.push_back(std::move(code));
        }
        found = scan.memo.insert(key, made);
    }
    return *static_cast<const ParsedFields *>(found.value().get());
}

qint64 parsedMarkCodesForTest() { return g_parsedMarks; }
void resetParsedMarkCodesForTest() { g_parsedMarks = 0; }
qint64 storyScansForTest() { return g_scans; }
void resetStoryScansForTest() { g_scans = 0; }

QString StoryScan::resultOf(int field) const
{
    const FieldPlace &f = fields.value(field);
    if (f.form == FieldPlace::Group) return f.display;
    if (f.separate < 0 || f.end <= f.separate) return QString();
    QString t = plain.mid(f.separate + 1, f.end - f.separate - 1);
    t.remove(QChar::ObjectReplacementCharacter);
    return t;
}

QString StoryScan::instructionOf(int field, const QHash<int, QString> &override) const
{
    const FieldPlace &f = fields.value(field);
    QString out;
    for (const CodePiece &p : f.code) {
        if (p.field < 0) out += p.text;
        else out += override.contains(p.field) ? override.value(p.field) : resultOf(p.field);
        if (out.size() > fields::kMostCodeChars) break;
    }
    return out;
}

StoryScan scanStory(QTextDocument *story, bool withBookmarks)
{
    StoryScan scan;
    static std::atomic<quint64> lastRevision{0};
    scan.revision = ++lastRevision;
    ++g_scans;
    scan.plain = story->toPlainText();
    QVector<Open> open;
    int overflow = 0;   // (the marks of fields that are nested deeper than the most: they are matched and left out)
    QHash<QString, int> startOfId;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        const QTextBlockFormat bf = b.blockFormat();
        if (bf.hasProperty(prop::EndnoteBlock) || bf.hasProperty(prop::RawBlock)) continue;
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            const QTextCharFormat cf = frag.charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            const bool mark = bytes.contains("fldChar") || bytes.contains("instrText") || bytes.contains("fldSimple") || bytes.contains("jwfield");
            const bool bookmark = withBookmarks && (bytes.contains("bookmarkStart") || bytes.contains("bookmarkEnd"));
            if (!mark && !bookmark) continue;
            const Inline atom = unpackAtom(bytes);
            for (int k = 0; k < frag.length(); ++k) {
                const int at = frag.position() + k;
                if (bookmark && atom.kind == Inline::ParaAtom && (atom.tag == QLatin1String("bookmarkStart") || atom.tag == QLatin1String("bookmarkEnd"))) {
                    const QString id = attributeOf(atom.raw, "id");
                    scan.maxBookmarkId = std::max(scan.maxBookmarkId, id.toLong());
                    if (atom.tag == QLatin1String("bookmarkStart")) {
                        if (scan.bookmarks.size() >= kMostBookmarks) {
                            scan.truncated = true;
                            continue;
                        }
                        BookmarkSpan s;
                        s.name = attributeOf(atom.raw, "name");
                        s.id = id;
                        s.start = at;
                        s.hidden = s.name.startsWith(QLatin1Char('_'));
                        startOfId.insert(id, int(scan.bookmarks.size()));
                        if (!s.name.isEmpty() && !scan.bookmarkByName.contains(s.name.toLower())) scan.bookmarkByName.insert(s.name.toLower(), int(scan.bookmarks.size()));
                        scan.bookmarks.push_back(s);
                    } else if (startOfId.contains(id)) {
                        BookmarkSpan &s = scan.bookmarks[startOfId.take(id)];
                        s.end = at;
                    }
                    continue;
                }
                if (!mark) continue;
                if (atom.kind == Inline::RunAtom && atom.tag == QLatin1String("fldChar")) {
                    if (atom.key == QLatin1String("begin")) {
                        if (open.size() + overflow >= kMostFieldDepth || scan.fields.size() >= kMostFields) {
                            ++overflow;
                            scan.truncated = true;
                            continue;
                        }
                        FieldPlace f;
                        f.form = FieldPlace::Complex;
                        f.begin = at;
                        f.depth = int(open.size());
                        f.locked = flagOf(atom.raw, "fldLock");
                        f.dirty = flagOf(atom.raw, "dirty");
                        if (!open.isEmpty()) {
                            f.parent = open.last().index;
                            f.inCode = !open.last().separated;
                            if (f.inCode) scan.fields[f.parent].code.push_back({QString(), int(scan.fields.size())});
                        }
                        open.push_back({int(scan.fields.size()), false});
                        scan.fields.push_back(f);
                    } else if (overflow > 0) {
                        if (atom.key == QLatin1String("end")) --overflow;
                    } else if (open.isEmpty()) {
                        continue;
                    } else if (atom.key == QLatin1String("separate")) {
                        if (!open.last().separated) {
                            open.last().separated = true;
                            scan.fields[open.last().index].separate = at;
                        }
                    } else if (atom.key == QLatin1String("end")) {
                        scan.fields[open.last().index].end = at;
                        open.removeLast();
                    }
                } else if (atom.kind == Inline::RunAtom && atom.tag == QLatin1String("instrText")) {
                    if (overflow == 0 && !open.isEmpty() && !open.last().separated) scan.fields[open.last().index].code.push_back({wordsOf(atom.raw), -1});
                } else if (atom.kind == Inline::ParaAtom && atom.tag == QLatin1String("fldSimple")) {
                    if (atom.role == Inline::Open) {
                        if (open.size() + overflow >= kMostFieldDepth || scan.fields.size() >= kMostFields) {
                            ++overflow;
                            scan.truncated = true;
                            continue;
                        }
                        FieldPlace f;
                        f.form = FieldPlace::Simple;
                        f.begin = f.separate = at;
                        f.depth = int(open.size());
                        f.locked = flagOf(atom.raw, "fldLock");
                        f.dirty = flagOf(atom.raw, "dirty");
                        f.code.push_back({attributeOf(atom.raw, "instr"), -1});
                        if (!open.isEmpty()) {
                            f.parent = open.last().index;
                            f.inCode = !open.last().separated;
                            if (f.inCode) scan.fields[f.parent].code.push_back({QString(), int(scan.fields.size())});
                        }
                        open.push_back({int(scan.fields.size()), true});
                        scan.fields.push_back(f);
                    } else if (atom.role == Inline::Close) {
                        if (overflow > 0) --overflow;
                        else if (!open.isEmpty()) {
                            scan.fields[open.last().index].end = at;
                            open.removeLast();
                        }
                    }
                } else if (atom.kind == Inline::ParaAtom && atom.tag == kFieldGroupTag) {
                    if (open.size() + overflow >= kMostFieldDepth || scan.fields.size() >= kMostFields) {
                        scan.truncated = true;
                        continue;
                    }
                    FieldPlace f;
                    f.form = FieldPlace::Group;
                    f.begin = f.separate = f.end = at;
                    f.depth = int(open.size());
                    f.live = !atom.key.isEmpty();
                    f.display = atom.display;
                    f.groupRaw = atom.raw;
                    f.locked = flagOf(atom.raw.mid(atom.raw.indexOf("fldChar")), "fldLock");
                    f.dirty = flagOf(atom.raw.mid(atom.raw.indexOf("fldChar")), "dirty");
                    QString code;
                    static const QRegularExpression instr(QStringLiteral("<w:instrText[^>]*>(.*?)</w:instrText>"), QRegularExpression::DotMatchesEverythingOption);
                    for (auto m = instr.globalMatch(QString::fromUtf8(atom.raw.left(20000))); m.hasNext();) code += unescaped(m.next().captured(1));
                    f.code.push_back({code, -1});
                    if (!open.isEmpty()) {
                        f.parent = open.last().index;
                        f.inCode = !open.last().separated;
                    }
                    scan.fields.push_back(f);
                }
            }
        }
    }
    // A field that never ended is no field. The names are read from the codes (a code with a nested field has the nested result in place of it, which the first word seldom is).
    QVector<int> keep;
    for (int i = 0; i < scan.fields.size(); ++i)
        if (scan.fields[i].end >= 0) keep << i;
    if (keep.size() != scan.fields.size()) {
        QVector<int> mapping(scan.fields.size(), -1);
        QVector<FieldPlace> kept;
        for (int i : keep) {
            mapping[i] = int(kept.size());
            kept.push_back(scan.fields[i]);
        }
        for (FieldPlace &f : kept) {
            f.parent = f.parent >= 0 ? mapping[f.parent] : -1;
            for (CodePiece &p : f.code) p.field = p.field >= 0 ? mapping[p.field] : -1;
        }
        scan.fields = kept;
    }
    for (int i = 0; i < scan.fields.size(); ++i) scan.fields[i].name = nameOfCode(scan.instructionOf(i));
    return scan;
}

namespace {

// What a mark of a field is, read from the atom: 1 a mark that begins a field (a begin mark, the open tag of a simple field, a field that is one character), -1 one that ends it, 0 neither.
struct MarkKind {
    int begins = 0;
    bool group = false, locked = false, complexBegin = false;
    QString code;   // the code of a field that is one tag or one character
};

MarkKind markKindOf(const QByteArray &bytes)
{
    MarkKind k;
    if (!bytes.contains("fldChar") && !bytes.contains("fldSimple") && !bytes.contains("jwfield")) return k;
    const Inline a = unpackAtom(bytes);
    auto locked = [&](const QByteArray &raw) { return flagOf(raw.mid(std::max<qsizetype>(0, raw.indexOf("fld"))), "fldLock"); };
    if (a.kind == Inline::RunAtom && a.tag == QLatin1String("fldChar")) {
        if (a.key == QLatin1String("begin")) {
            k.begins = 1;
            k.complexBegin = true;
            k.locked = locked(a.raw);
        } else if (a.key == QLatin1String("end")) {
            k.begins = -1;
        }
    } else if (a.kind == Inline::ParaAtom && a.tag == QLatin1String("fldSimple")) {
        k.begins = a.role == Inline::Open ? 1 : a.role == Inline::Close ? -1 : 0;
        k.locked = a.role == Inline::Open && locked(a.raw);
        if (a.role == Inline::Open) k.code = attributeOf(a.raw, "instr");
    } else if (a.kind == Inline::ParaAtom && a.tag == kFieldGroupTag) {
        k.group = true;
        k.begins = 1;
        k.locked = locked(a.raw);
        static const QRegularExpression instr(QStringLiteral("<w:instrText[^>]*>(.*?)</w:instrText>"), QRegularExpression::DotMatchesEverythingOption);
        k.code = unescaped(instr.match(QString::fromUtf8(a.raw.left(20000))).captured(1));
    }
    return k;
}

constexpr int kMostParagraphsLookedBack = 3000;

// The code of the field whose begin mark is at a position: the words of the instruction marks after it, up to the separator.
QString codeAfter(QTextDocument *story, int beginPos)
{
    QString code;
    int atoms = 0;
    for (QTextBlock b = story->findBlock(beginPos); b.isValid() && atoms < 64; b = b.next())
        for (auto it = b.begin(); !it.atEnd() && atoms < 64; ++it) {
            const QTextFragment frag = it.fragment();
            if (frag.position() <= beginPos) continue;
            const QTextCharFormat cf = frag.charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            ++atoms;
            if (bytes.contains("instrText")) code += wordsOf(unpackAtom(bytes).raw);
            else if (bytes.contains("fldChar") && unpackAtom(bytes).key == QLatin1String("separate")) return code;
            else if (bytes.contains("fldChar") && code.size() > 0) return code;
        }
    return code;
}

} // namespace

FieldsNear fieldsNearSelection(QTextDocument *story, const QTextCursor &c)
{
    FieldsNear out;
    auto note = [&](const MarkKind &k, int beginPos) {
        out.any = true;
        (k.locked ? out.locked : out.unlocked) = true;
        const QString code = (k.complexBegin ? codeAfter(story, beginPos) : k.code).trimmed();
        if (code.startsWith(QLatin1String("TOC"), Qt::CaseInsensitive) && (code.size() == 3 || !code[3].isLetterOrNumber())) out.toc = true;
        if (code.startsWith(QLatin1String("INDEX"), Qt::CaseInsensitive) && (code.size() == 5 || !code[5].isLetterOrNumber())) out.index = true;
        if (code.startsWith(QLatin1String("TOA"), Qt::CaseInsensitive) && (code.size() == 3 || !code[3].isLetterOrNumber())) out.authorities = true;
    };
    const int from = c.selectionStart(), to = c.selectionEnd();
    // The marks that begin in the selection (and, for a caret, a field that is one character at its side).
    for (QTextBlock b = story->findBlock(from); b.isValid() && b.position() <= to; b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (frag.position() + frag.length() < from || frag.position() > to) continue;
            const QTextCharFormat cf = frag.charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const MarkKind k = markKindOf(storedBytes(cf, prop::Atom));
            if (k.begins > 0 && (k.group || frag.position() >= from)) note(k, frag.position());
        }
    if (out.any) return out;
    // The field the start of the selection is in: backward from there, the marks that end a field and those that begin one cancel, the first begin that is left is it.
    int depth = 0;
    int looked = 0;
    for (QTextBlock b = story->findBlock(from); b.isValid() && looked < kMostParagraphsLookedBack; b = b.previous(), ++looked) {
        QVector<std::pair<int, MarkKind>> marks;
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (frag.position() >= from) break;
            const QTextCharFormat cf = frag.charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const MarkKind k = markKindOf(storedBytes(cf, prop::Atom));
            if (k.begins != 0) marks.push_back({frag.position(), k});
        }
        for (int i = int(marks.size()) - 1; i >= 0; --i) {
            const MarkKind &k = marks[i].second;
            if (k.group) continue;
            if (k.begins < 0) ++depth;
            else if (depth == 0) {
                note(k, marks[i].first);
                return out;
            } else {
                --depth;
            }
        }
    }
    return out;
}

int fieldContaining(const StoryScan &scan, int position)
{
    int best = -1;
    for (int i = 0; i < scan.fields.size(); ++i) {
        const FieldPlace &f = scan.fields[i];
        if (position < f.begin || position > f.end + (f.form == FieldPlace::Group ? 1 : 0)) continue;
        if (best < 0 || f.depth >= scan.fields[best].depth) best = i;
    }
    return best;
}

fields::Properties documentProperties(WriteDoc &doc)
{
    fields::Properties p;
    auto read = [&](const QString &part, const QHash<QString, QString *> &want) {
        const QByteArray bytes = doc.partData(part);
        if (bytes.isEmpty()) return;
        QXmlStreamReader xml(bytes);
        int steps = 0;
        while (!xml.atEnd() && ++steps < 20000) {
            if (xml.readNext() != QXmlStreamReader::StartElement) continue;
            const QString name = xml.name().toString();
            if (!want.contains(name)) continue;
            *want.value(name) = xml.readElementText().trimmed();
        }
    };
    QString created, modified, printed, totalTime, pages, words, characters, charactersWithSpaces, lines, paragraphs, revision;
    read(QStringLiteral("/docProps/core.xml"), {{QStringLiteral("title"), &p.title}, {QStringLiteral("subject"), &p.subject}, {QStringLiteral("creator"), &p.author}, {QStringLiteral("keywords"), &p.keywords},
                                                {QStringLiteral("description"), &p.comments}, {QStringLiteral("lastModifiedBy"), &p.lastSavedBy}, {QStringLiteral("category"), &p.category},
                                                {QStringLiteral("created"), &created}, {QStringLiteral("modified"), &modified}, {QStringLiteral("lastPrinted"), &printed}, {QStringLiteral("revision"), &revision}});
    read(QStringLiteral("/docProps/app.xml"), {{QStringLiteral("Template"), &p.template_}, {QStringLiteral("Company"), &p.company}, {QStringLiteral("Manager"), &p.manager}, {QStringLiteral("TotalTime"), &totalTime},
                                               {QStringLiteral("Pages"), &pages}, {QStringLiteral("Words"), &words}, {QStringLiteral("Characters"), &characters},
                                               {QStringLiteral("CharactersWithSpaces"), &charactersWithSpaces}, {QStringLiteral("Lines"), &lines}, {QStringLiteral("Paragraphs"), &paragraphs}});
    p.created = QDateTime::fromString(created, Qt::ISODate);
    p.saved = QDateTime::fromString(modified, Qt::ISODate);
    p.printed = QDateTime::fromString(printed, Qt::ISODate);
    if (p.created.isValid()) p.created = p.created.toLocalTime();
    if (p.saved.isValid()) p.saved = p.saved.toLocalTime();
    if (p.printed.isValid()) p.printed = p.printed.toLocalTime();
    p.revision = revision;
    p.totalMinutes = totalTime.toLongLong();
    if (!doc.filePath.isEmpty()) {
        p.filePath = doc.filePath;
        p.fileName = doc.filePath.section(QLatin1Char('/'), -1);
    }
    return p;
}

} // namespace jp::write
