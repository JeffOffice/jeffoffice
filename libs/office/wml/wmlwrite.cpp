// Writing a .docx: a wml::Document into its package. Elements are written in the order ECMA-376 gives, the parts the document kept
// are written as they were read, and the pieces that were kept as XML but whose partner is gone are left out so that what is written
// is whole (a field with no end, or a bookmark with no start, would make a program that opens the file complain).

#include "wml/wml.h"

#include "office/package.h"
#include "wml/xmltree.h"

#include <QCoreApplication>
#include <QHash>
#include <QSet>
#include <algorithm>
#include <vector>

namespace jp::office::wml {

namespace {

const char kXmlHead[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n";
const char kWNs[] = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
const char kRNs[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

// The qualified name at the start of a start tag ("w:document").
QByteArray qnameOf(const QByteArray &startTag)
{
    int i = 1;
    while (i < startTag.size() && startTag[i] != ' ' && startTag[i] != '>' && startTag[i] != '/' && startTag[i] != '\t' && startTag[i] != '\r' && startTag[i] != '\n') ++i;
    return startTag.mid(1, i - 1);
}

// The start tag with the word processing namespace declared on the prefix "w", which the elements written here use.
QByteArray withW(QByteArray tag)
{
    if (!tag.contains("xmlns:w=")) {
        const int at = int(qnameOf(tag).size()) + 1;
        tag.insert(at, QByteArray(" xmlns:w=\"") + kWNs + "\"");
    }
    return tag;
}

// The prefixes of the namespaces word processing files use besides the main ones, with their names. A piece kept as XML may come from
// another file (copied across), whose root declared a prefix that this file's does not; the prefixes the pieces use are declared on the root.
const char *const kKnownPrefixes[][2] = {
    {"wp", "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"},
    {"a", "http://schemas.openxmlformats.org/drawingml/2006/main"},
    {"pic", "http://schemas.openxmlformats.org/drawingml/2006/picture"},
    {"c", "http://schemas.openxmlformats.org/drawingml/2006/chart"},
    {"dgm", "http://schemas.openxmlformats.org/drawingml/2006/diagram"},
    {"mc", "http://schemas.openxmlformats.org/markup-compatibility/2006"},
    {"m", "http://schemas.openxmlformats.org/officeDocument/2006/math"},
    {"v", "urn:schemas-microsoft-com:vml"},
    {"o", "urn:schemas-microsoft-com:office:office"},
    {"w10", "urn:schemas-microsoft-com:office:word"},
    {"w14", "http://schemas.microsoft.com/office/word/2010/wordml"},
    {"w15", "http://schemas.microsoft.com/office/word/2012/wordml"},
    {"wp14", "http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing"},
    {"wps", "http://schemas.microsoft.com/office/word/2010/wordprocessingShape"},
    {"wpg", "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup"},
    {"wpc", "http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas"},
};

void collectPrefixes(const QByteArray &xml, QSet<QByteArray> *used)
{
    for (qsizetype at = xml.indexOf(':'); at > 0; at = xml.indexOf(':', at + 1)) {
        qsizetype from = at;
        while (from > 0 && ((xml[from - 1] >= 'a' && xml[from - 1] <= 'z') || (xml[from - 1] >= 'A' && xml[from - 1] <= 'Z') || (xml[from - 1] >= '0' && xml[from - 1] <= '9'))) --from;
        if (from == at || from == 0) continue;
        const char before = xml[from - 1];
        if (before != '<' && before != ' ' && before != '/') continue;
        used->insert(xml.mid(from, at - from));
    }
}

// The prefixes a start tag declares, read from its attributes (they may be separated by any white space).
QSet<QByteArray> declaredPrefixes(const QByteArray &tag)
{
    QSet<QByteArray> out;
    qsizetype i = 1;
    auto space = [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; };
    while (i < tag.size() && !space(tag[i]) && tag[i] != '>' && tag[i] != '/') ++i;
    while (i < tag.size()) {
        while (i < tag.size() && space(tag[i])) ++i;
        if (i >= tag.size() || tag[i] == '>' || tag[i] == '/') break;
        const qsizetype nameAt = i;
        while (i < tag.size() && !space(tag[i]) && tag[i] != '=' && tag[i] != '>') ++i;
        const QByteArray name = tag.mid(nameAt, i - nameAt);
        while (i < tag.size() && space(tag[i])) ++i;
        if (i >= tag.size() || tag[i] != '=') continue;
        ++i;
        while (i < tag.size() && space(tag[i])) ++i;
        if (i >= tag.size() || (tag[i] != '"' && tag[i] != '\'')) continue;
        const char quote = tag[i++];
        while (i < tag.size() && tag[i] != quote) ++i;
        ++i;
        if (name.startsWith("xmlns:")) out.insert(name.mid(6));
    }
    return out;
}

QByteArray withKnownPrefixes(QByteArray rootTag, const Document &doc)
{
    const QSet<QByteArray> declared = declaredPrefixes(rootTag);
    QSet<QByteArray> used;
    for (const Block &b : doc.body) {
        if (b.isRaw) {
            collectPrefixes(b.raw, &used);
            continue;
        }
        for (const Inline &a : b.para.content)
            if (a.kind != Inline::Text) collectPrefixes(a.raw, &used);
    }
    for (const auto &known : kKnownPrefixes) {
        const QByteArray prefix(known[0]);
        if (!used.contains(prefix) || declared.contains(prefix)) continue;
        rootTag.insert(int(qnameOf(rootTag).size()) + 1, " xmlns:" + prefix + "=\"" + known[1] + "\"");
    }
    return rootTag;
}

constexpr quint64 coord(int block, int inl) { return (quint64(block) << 32) | quint32(inl + 1); }

// Which of the pieces kept as XML are written: the ones whose partners are there.
class Whole {
public:
    explicit Whole(const Document &doc)
    {
        const int n = int(doc.body.size());
        m_blocks.assign(n, true);
        m_inlines.resize(n);
        for (int b = 0; b < n; ++b) m_inlines[b].assign(doc.body[b].para.content.size(), true);

        QVector<int> stack;
        // Tags with a start and an end (hyperlink, content controls...), in the body and in each paragraph.
        for (int b = 0; b < n; ++b) {
            const Block &blk = doc.body[b];
            if (blk.isRaw) {
                if (blk.role == Inline::Open) {
                    stack.push_back(b);
                } else if (blk.role == Inline::Close) {
                    if (!stack.isEmpty() && doc.body[stack.back()].tag == blk.tag) stack.removeLast();
                    else m_blocks[b] = false;
                }
                continue;
            }
            QVector<int> inner;
            const auto &content = blk.para.content;
            for (int i = 0; i < content.size(); ++i) {
                if (content[i].kind != Inline::ParaAtom) continue;
                if (content[i].role == Inline::Open) {
                    inner.push_back(i);
                } else if (content[i].role == Inline::Close) {
                    if (!inner.isEmpty() && content[inner.back()].tag == content[i].tag) inner.removeLast();
                    else m_inlines[b][i] = false;
                }
            }
            for (int i : std::as_const(inner)) m_inlines[b][i] = false;
        }
        for (int b : std::as_const(stack)) m_blocks[b] = false;

        // A bookmark or a range begins before it ends; the first of the same Id stands.
        struct Open { int block, inl; };
        QHash<QString, Open> open;
        QSet<QString> done;
        auto base = [](const QString &tag, bool *start) {
            static const char *const pairs[][2] = {{"bookmarkStart", "bookmarkEnd"},
                                                   {"commentRangeStart", "commentRangeEnd"},
                                                   {"permStart", "permEnd"},
                                                   {"moveFromRangeStart", "moveFromRangeEnd"},
                                                   {"moveToRangeStart", "moveToRangeEnd"}};
            for (const auto &p : pairs) {
                if (tag == QLatin1String(p[0])) { *start = true; return QString::fromLatin1(p[0]); }
                if (tag == QLatin1String(p[1])) { *start = false; return QString::fromLatin1(p[0]); }
            }
            return QString();
        };
        auto visit = [&](const QString &tag, const QString &id, int block, int inl) {
            bool start = false;
            const QString kind = base(tag, &start);
            if (kind.isEmpty()) return;
            const QString key = kind + QLatin1Char('/') + id;
            auto drop = [&](int b, int i) {
                if (i < 0) m_blocks[b] = false;
                else m_inlines[b][i] = false;
            };
            if (start) {
                if (open.contains(key) || done.contains(key)) drop(block, inl);
                else open.insert(key, {block, inl});
            } else if (open.contains(key)) {
                open.remove(key);
                done.insert(key);
            } else {
                drop(block, inl);
            }
        };
        for (int b = 0; b < n; ++b) {
            const Block &blk = doc.body[b];
            if (blk.isRaw) {
                visit(blk.tag, blk.key, b, -1);
                continue;
            }
            for (int i = 0; i < blk.para.content.size(); ++i)
                if (blk.para.content[i].kind == Inline::ParaAtom) visit(blk.para.content[i].tag, blk.para.content[i].key, b, i);
        }
        for (auto it = open.cbegin(); it != open.cend(); ++it) {
            if (it.value().inl < 0) m_blocks[it.value().block] = false;
            else m_inlines[it.value().block][it.value().inl] = false;
        }

        // Fields: a begin, its code, its separator, and its end go together or not at all.
        struct Field { QVector<QPair<int, int>> pieces; bool separated = false; };
        QVector<Field> fields;
        for (int b = 0; b < n; ++b) {
            const auto &content = doc.body[b].para.content;
            for (int i = 0; i < content.size(); ++i) {
                const Inline &a = content[i];
                if (a.kind != Inline::RunAtom || (a.tag != QLatin1String("fldChar") && a.tag != QLatin1String("instrText"))) continue;
                m_inlines[b][i] = false;
                if (a.tag == QLatin1String("instrText")) {
                    if (!fields.isEmpty() && !fields.back().separated) fields.back().pieces.push_back({b, i});
                } else if (a.key == QLatin1String("begin")) {
                    if (fields.size() < 200) fields.push_back(Field{{{b, i}}, false});
                } else if (a.key == QLatin1String("separate")) {
                    if (!fields.isEmpty() && !fields.back().separated) {
                        fields.back().separated = true;
                        fields.back().pieces.push_back({b, i});
                    }
                } else if (a.key == QLatin1String("end")) {
                    if (!fields.isEmpty()) {
                        fields.back().pieces.push_back({b, i});
                        for (const auto &piece : std::as_const(fields.back().pieces)) m_inlines[piece.first][piece.second] = true;
                        fields.removeLast();
                    }
                }
            }
        }
    }

    bool block(int b) const { return m_blocks[b]; }
    bool inlineKept(int b, int i) const { return m_inlines[b][i]; }

private:
    std::vector<bool> m_blocks;
    QVector<std::vector<bool>> m_inlines;
};

// Text made fit for a w:t: the characters XML has no place for left out.
QString xmlSafe(const QString &s)
{
    QString out;
    out.reserve(s.size());
    for (const QChar c : s) {
        const ushort u = c.unicode();
        if (u < 0x20 && u != 9 && u != 10 && u != 13) continue;
        if (u == 0xFFFE || u == 0xFFFF) continue;
        out += c;
    }
    return out;
}

void writeRunText(QByteArray &out, const RunProps &props, const QString &text)
{
    const QByteArray rpr = runPropsXml(props);
    out += "<w:r>";
    if (!rpr.isEmpty()) out += "<w:rPr>" + rpr + "</w:rPr>";
    QString seg;
    auto flush = [&] {
        if (seg.isEmpty()) return;
        out += "<w:t xml:space=\"preserve\">" + escapeXml(xmlSafe(seg)) + "</w:t>";
        seg.clear();
    };
    for (const QChar c : text) {
        const ushort u = c.unicode();
        if (u == '\t') { flush(); out += "<w:tab/>"; }
        else if (u == QChar::LineSeparator || u == '\n') { flush(); out += "<w:br/>"; }
        else if (u == 0x2011) { flush(); out += "<w:noBreakHyphen/>"; }
        else seg += c;
    }
    flush();
    out += "</w:r>";
}

QByteArray documentXml(const Document &doc)
{
    const Whole whole(doc);
    QByteArray out;
    out.reserve(1 << 16);
    out += kXmlHead;
    QByteArray root = doc.rootStart;
    if (root.isEmpty()) root = QByteArray("<w:document xmlns:w=\"") + kWNs + "\" xmlns:r=\"" + kRNs + "\">";
    root = withKnownPrefixes(withW(root), doc);
    out += root;
    out += doc.beforeBody;
    out += "<w:body>";
    for (int b = 0; b < doc.body.size(); ++b) {
        const Block &blk = doc.body[b];
        if (!whole.block(b)) continue;
        if (blk.isRaw) {
            out += blk.raw;
            continue;
        }
        out += "<w:p" + blk.para.attrs + ">";
        const QByteArray ppr = paraPropsXml(blk.para.props);
        if (!ppr.isEmpty()) out += "<w:pPr>" + ppr + "</w:pPr>";
        const auto &content = blk.para.content;
        // Runs of text with the same properties are one run (those that pieces kept as XML, left out above, had kept apart, too).
        QString pending;
        const RunProps *pendingProps = nullptr;
        auto flush = [&] {
            if (!pendingProps) return;
            writeRunText(out, *pendingProps, pending);
            pendingProps = nullptr;
            pending.clear();
        };
        for (int i = 0; i < content.size(); ++i) {
            if (!whole.inlineKept(b, i)) continue;
            const Inline &a = content[i];
            if (a.kind == Inline::Text) {
                if (pendingProps && *pendingProps == a.props) {
                    pending += a.text;
                } else {
                    flush();
                    pendingProps = &a.props;
                    pending = a.text;
                }
                continue;
            }
            flush();
            if (a.kind == Inline::RunAtom) {
                const QByteArray rpr = runPropsXml(a.props);
                out += "<w:r>";
                if (!rpr.isEmpty()) out += "<w:rPr>" + rpr + "</w:rPr>";
                out += a.raw + "</w:r>";
            } else {
                out += a.raw;
            }
        }
        flush();
        out += "</w:p>";
    }
    out += sectionPropsXml(doc.lastSection);
    out += "</w:body>";
    out += doc.afterBody;
    out += "</" + qnameOf(root) + ">";
    return out;
}

QByteArray stylesXml(const StyleSheet &sheet)
{
    if (!sheet.original.isEmpty() && !sheet.modified) return sheet.original;
    QByteArray out = kXmlHead;
    QByteArray root = sheet.rootStart.isEmpty() ? QByteArray("<w:styles xmlns:w=\"") + kWNs + "\">" : sheet.rootStart;
    root = withW(root);
    out += root;
    if (!sheet.docDefaults.isEmpty()) {
        out += sheet.docDefaults;
    } else {
        out += "<w:docDefaults>";
        const QByteArray r = runPropsXml(sheet.defaultRun), p = paraPropsXml(sheet.defaultPara);
        if (!r.isEmpty()) out += "<w:rPrDefault><w:rPr>" + r + "</w:rPr></w:rPrDefault>";
        if (!p.isEmpty()) out += "<w:pPrDefault><w:pPr>" + p + "</w:pPr></w:pPrDefault>";
        out += "</w:docDefaults>";
    }
    out += sheet.latentStyles;
    for (const Style &s : sheet.styles) {
        if (!s.raw.isEmpty()) {
            out += s.raw;
            continue;
        }
        out += "<w:style w:type=\"" + escapeXml(s.type, true) + "\"";
        if (s.isDefault) out += " w:default=\"1\"";
        if (s.customStyle) out += " w:customStyle=\"1\"";
        out += " w:styleId=\"" + escapeXml(s.id, true) + "\">";
        out += "<w:name w:val=\"" + escapeXml(s.name.isEmpty() ? s.id : s.name, true) + "\"/>";
        if (!s.basedOn.isEmpty()) out += "<w:basedOn w:val=\"" + escapeXml(s.basedOn, true) + "\"/>";
        if (!s.next.isEmpty()) out += "<w:next w:val=\"" + escapeXml(s.next, true) + "\"/>";
        if (!s.link.isEmpty()) out += "<w:link w:val=\"" + escapeXml(s.link, true) + "\"/>";
        if (s.uiPriority) out += "<w:uiPriority w:val=\"" + QByteArray::number(*s.uiPriority) + "\"/>";
        if (s.semiHidden) out += "<w:semiHidden/>";
        if (s.unhideWhenUsed) out += "<w:unhideWhenUsed/>";
        if (s.qFormat) out += "<w:qFormat/>";
        const QByteArray p = paraPropsXml(s.pPr), r = runPropsXml(s.rPr);
        if (!p.isEmpty()) out += "<w:pPr>" + p + "</w:pPr>";
        if (!r.isEmpty()) out += "<w:rPr>" + r + "</w:rPr>";
        out += s.tail;
        out += "</w:style>";
    }
    out += "</" + qnameOf(root) + ">";
    return out;
}

QByteArray numberingXml(const Numbering &num)
{
    if (!num.original.isEmpty() && !num.modified) return num.original;
    QByteArray out = kXmlHead;
    QByteArray root = num.rootStart.isEmpty() ? QByteArray("<w:numbering xmlns:w=\"") + kWNs + "\">" : num.rootStart;
    root = withW(root);
    out += root;
    out += num.prefix;
    for (const AbstractNum &a : num.abstracts) {
        if (!a.raw.isEmpty()) {
            out += a.raw;
            continue;
        }
        out += "<w:abstractNum w:abstractNumId=\"" + QByteArray::number(a.id) + "\">" + a.head;
        if (a.head.isEmpty()) out += "<w:multiLevelType w:val=\"" + escapeXml(a.multiLevelType.isEmpty() ? QStringLiteral("hybridMultilevel") : a.multiLevelType, true) + "\"/>";
        for (int i = 0; i < 9; ++i) {
            const Level &l = a.levels[i];
            if (!l.present) continue;
            out += "<w:lvl w:ilvl=\"" + QByteArray::number(i) + "\">";
            out += "<w:start w:val=\"" + QByteArray::number(l.start) + "\"/>";
            out += "<w:numFmt w:val=\"" + escapeXml(l.numFmt, true) + "\"/>";
            if (l.restart) out += "<w:lvlRestart w:val=\"" + QByteArray::number(*l.restart) + "\"/>";
            if (!l.pStyle.isEmpty()) out += "<w:pStyle w:val=\"" + escapeXml(l.pStyle, true) + "\"/>";
            if (l.legal) out += "<w:isLgl/>";
            if (l.suff != QLatin1String("tab")) out += "<w:suff w:val=\"" + escapeXml(l.suff, true) + "\"/>";
            out += "<w:lvlText w:val=\"" + escapeXml(l.text, true) + "\"/>";
            out += "<w:lvlJc w:val=\"" + escapeXml(l.jc, true) + "\"/>";
            const QByteArray p = paraPropsXml(l.pPr), r = runPropsXml(l.rPr);
            if (!p.isEmpty()) out += "<w:pPr>" + p + "</w:pPr>";
            if (!r.isEmpty()) out += "<w:rPr>" + r + "</w:rPr>";
            out += "</w:lvl>";
        }
        out += a.tail + "</w:abstractNum>";
    }
    for (const Num &n : num.nums) {
        if (!n.raw.isEmpty()) {
            out += n.raw;
            continue;
        }
        out += "<w:num w:numId=\"" + QByteArray::number(n.id) + "\"><w:abstractNumId w:val=\"" + QByteArray::number(n.abstractId) + "\"/>";
        for (auto it = n.startOverrides.cbegin(); it != n.startOverrides.cend(); ++it)
            out += "<w:lvlOverride w:ilvl=\"" + QByteArray::number(it.key()) + "\"><w:startOverride w:val=\"" + QByteArray::number(it.value()) + "\"/></w:lvlOverride>";
        out += "</w:num>";
    }
    out += num.suffix;
    out += "</" + qnameOf(root) + ">";
    return out;
}

} // namespace

QByteArray writeDocx(const Document &doc, QString *error)
{
    PackageWriter w;
    // The extensions whose parts all have one type are declared once.
    QHash<QString, QString> extensionType;
    QSet<QString> mixed;
    for (const KeptPart &p : doc.parts) {
        const QString ext = p.name.section(QLatin1Char('.'), -1).toLower();
        if (ext.isEmpty() || ext == QLatin1String("xml") || ext == QLatin1String("rels")) continue;
        if (!extensionType.contains(ext)) extensionType.insert(ext, p.contentType);
        else if (extensionType.value(ext) != p.contentType) mixed.insert(ext);
    }
    for (auto it = extensionType.cbegin(); it != extensionType.cend(); ++it)
        if (!mixed.contains(it.key()) && !it.value().isEmpty()) w.addDefault(it.key(), it.value());

    static const char mainType[] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml";
    w.addPart(doc.mainName, documentXml(doc), doc.mainContentType.isEmpty() ? QString::fromLatin1(mainType) : doc.mainContentType);
    if (!doc.stylesName.isEmpty())
        w.addPart(doc.stylesName, stylesXml(doc.styles), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
    if (!doc.numberingName.isEmpty())
        w.addPart(doc.numberingName, numberingXml(doc.numbering), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"));
    for (const KeptPart &p : doc.parts) w.addPart(p.name, p.data, p.contentType);

    for (const Relationship &r : doc.packageRelationships) w.addRelationshipWithId(QStringLiteral("/"), r.id, r.type, r.target, r.external);
    for (const Relationship &r : doc.mainRelationships) w.addRelationshipWithId(doc.mainName, r.id, r.type, r.target, r.external);
    for (auto it = doc.partRelationships.cbegin(); it != doc.partRelationships.cend(); ++it)
        for (const Relationship &r : it.value()) w.addRelationshipWithId(it.key(), r.id, r.type, r.target, r.external);
    const QByteArray zip = w.finish();
    if (zip.isEmpty() && error) *error = QCoreApplication::translate("jp::office::wml", "The document could not be packaged.");
    return zip;
}

} // namespace jp::office::wml
