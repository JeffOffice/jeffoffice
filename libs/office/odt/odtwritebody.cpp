// .docx to OpenDocument text: the body (paragraphs, lists, tables, fields, pictures, notes).

#include "odt/odtwrite.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>

namespace jp::office::odt {

namespace {

QByteArray num(qint64 v)
{
    return QByteArray::number(v);
}

const char kNsWp[] = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
const char kNsA[] = "http://schemas.openxmlformats.org/drawingml/2006/main";
const char kNsWps[] = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
const char kNsMc[] = "http://schemas.openxmlformats.org/markup-compatibility/2006";
const char kNsV[] = "urn:schemas-microsoft-com:vml";
const char kNsM[] = "http://schemas.openxmlformats.org/officeDocument/2006/math";

bool isNs(const wml::XmlTree &t, int n, const char *local, const char *uri)
{
    const int id = t.namespaceId(uri);
    return id >= 0 && t.is(n, local, id);
}

QString xmlSafe(const QString &s)
{
    QString out;
    out.reserve(s.size());
    for (const QChar c : s) {
        const ushort u = c.unicode();
        if (u < 0x20 && u != 0x9 && u != 0xA && u != 0xD) continue;
        if (u == 0xFFFE || u == 0xFFFF) continue;
        out += c;
    }
    return out;
}

bool onValue(const wml::XmlTree &t, int n)
{
    const QString v = t.val(n);
    return v.isEmpty() || v == QLatin1String("1") || v == QLatin1String("true") || v == QLatin1String("on");
}

} // namespace

struct Writer::ParaCtx {
    QByteArray xml;
    QString plain;
    bool lastSpace = true;
    bool pageBreak = false;          // a page break was met: what follows begins a page
    bool firstHasBreak = false;
    QVector<QPair<QByteArray, bool>> fragments;   // paragraphs made by page breaks inside: their content and whether each begins a page
    struct Field {
        QString instr;
        bool result = false;
        QByteArray savedXml;
        QString savedPlain;
    };
    QVector<Field> fields;
    bool inNote = false;
    int depth = 0;
    int pendingSpaces = 0;
    bool hasContent = false;
};

// ---- blocks

void Writer::closeLists(ListCtx *lc)
{
    if (!lc || lc->depth == 0) return;
    while (lc->depth > 0) {
        *lc->out += "</text:list-item></text:list>";
        --lc->depth;
    }
    lc->numId = 0;
}

void Writer::emitBlocks(const T &t, int parent, QByteArray *out, int depth, bool body)
{
    if (parent < 0) return;
    if (depth > kMaxNesting) {
        left(QCoreApplication::translate("odt", "content nested too deeply"));
        return;
    }
    ListCtx lc;
    lc.out = out;
    for (int c = t.firstChild(parent); c >= 0; c = t.next(c)) emitBlock(t, c, out, &lc, depth, body);
    closeLists(&lc);
}

void Writer::emitBlock(const T &t, int n, QByteArray *out, ListCtx *lc, int depth, bool body)
{
    if (t.is(n, "p")) emitParagraph(t, n, out, lc, depth, body);
    else if (t.is(n, "tbl")) {
        closeLists(lc);
        emitTable(t, n, out, depth, body);
    } else if (t.is(n, "sdt")) {
        const int content = t.child(n, "sdtContent");
        if (content >= 0)
            for (int c = t.firstChild(content); c >= 0; c = t.next(c)) emitBlock(t, c, out, lc, depth, body);
    } else if (t.is(n, "ins") || t.is(n, "moveTo") || t.is(n, "customXml") || t.is(n, "smartTag")) {
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) emitBlock(t, c, out, lc, depth, body);
    } else if (t.is(n, "bookmarkStart")) {
        const QString name = t.attr(n, "name");
        if (!name.isEmpty()) {
            m_bookmarkNames.insert(t.attr(n, "id").toInt(), name);
            m_pendingBookmarks += "<text:bookmark-start text:name=\"" + esc(name) + "\"/>";
        }
    } else if (t.is(n, "bookmarkEnd")) {
        const int id = t.attr(n, "id").toInt();
        if (m_bookmarkNames.contains(id)) m_pendingBookmarks += "<text:bookmark-end text:name=\"" + esc(m_bookmarkNames.value(id)) + "\"/>";
    }
}

namespace {

// A paragraph's field activity: how many fields it opens and closes, and whether the first is a table of contents.
void scanFields(const wml::XmlTree &t, int n, int *depth, int *net, bool *startsToc, QString *instr, QString *top, int level)
{
    if (level > 24) return;
    for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
        if (t.is(c, "fldChar")) {
            const QString type = t.attr(c, "fldCharType");
            if (type == QLatin1String("begin")) {
                if (*depth == 0) top->clear();
                ++*depth;
                ++*net;
            } else if (type == QLatin1String("end")) {
                if (*depth > 0) --*depth;
                --*net;
            }
        } else if (t.is(c, "instrText")) {
            if (*depth == 1) *top += t.text(c);
        } else if (t.is(c, "fldSimple")) {
        } else if (t.is(c, "r") || t.is(c, "hyperlink") || t.is(c, "ins") || t.is(c, "sdt") || t.is(c, "sdtContent") || t.is(c, "smartTag")) {
            scanFields(t, c, depth, net, startsToc, instr, top, level + 1);
        }
        if (*depth == 1 && instr->isEmpty() && !top->isEmpty() && t.is(c, "r")) {
            // (The first field's instruction, as far as it has been read.)
            if (top->trimmed().startsWith(QLatin1String("TOC"))) {
                *startsToc = true;
                *instr = *top;
            }
        }
    }
}

} // namespace

int Writer::tocDepthOf(const T &t, int p, bool *startsToc, QString *instr) const
{
    int depth = 0, net = 0;
    QString top;
    *startsToc = false;
    instr->clear();
    scanFields(t, p, &depth, &net, startsToc, instr, &top, 0);
    return net;
}

int Writer::numLevelOf(const T &t, int pPr, int *numId, QString pStyle) const
{
    // The numbering of a paragraph: its own, else its style's (or a style it is based on).
    int level = -1;
    *numId = -1;
    if (pPr >= 0) {
        const int np = t.child(pPr, "numPr");
        if (np >= 0) {
            const int il = t.child(np, "ilvl"), id = t.child(np, "numId");
            if (id >= 0) *numId = t.val(id).toInt();
            if (il >= 0) level = t.val(il).toInt();
        }
    }
    if (*numId < 0 && m_styles) {
        // The style chain.
        QString id = pStyle.isEmpty() ? m_defaultParaStyle : pStyle;
        for (int guard = 0; guard < kMaxStyleChain && !id.isEmpty(); ++guard) {
            const auto it = m_dstyles.constFind(id);
            if (it == m_dstyles.constEnd()) break;
            const int sp = m_styles->child(it->node, "pPr");
            if (sp >= 0) {
                const int np = m_styles->child(sp, "numPr");
                if (np >= 0) {
                    const int idn = m_styles->child(np, "numId"), il = m_styles->child(np, "ilvl");
                    if (idn >= 0 && *numId < 0) *numId = m_styles->val(idn).toInt();
                    if (il >= 0 && level < 0) level = m_styles->val(il).toInt();
                    if (*numId >= 0) break;
                }
            }
            id = it->basedOn;
        }
    }
    return level < 0 ? 0 : std::min(level, 8);
}

void Writer::emitParagraph(const T &t, int p, QByteArray *out, ListCtx *lc, int depth, bool body)
{
    const int pPr = t.child(p, "pPr");
    QString pStyle;
    if (pPr >= 0) {
        const int ps = t.child(pPr, "pStyle");
        if (ps >= 0) pStyle = t.val(ps);
    }
    QString odfStyle = QStringLiteral("Standard");
    if (!pStyle.isEmpty() && m_dstyles.contains(pStyle)) odfStyle = m_dstyles[pStyle].odfName;
    else if (pStyle.isEmpty() && !m_defaultParaStyle.isEmpty() && m_dstyles.contains(m_defaultParaStyle)) odfStyle = m_dstyles[m_defaultParaStyle].odfName;
    // The table of contents a field holds.
    const bool atBody = body && depth == 0;
    bool openedToc = false;
    if (atBody) {
        bool starts = false;
        QString instr;
        const int delta = tocDepthOf(t, p, &starts, &instr);
        if (!m_inToc && starts && delta > 0) {
            closeLists(lc);
            int levels = 3;
            static const QRegularExpression re(QStringLiteral("\\\\o\\s+\"(\\d)-(\\d)\""));
            const auto m = re.match(instr);
            if (m.hasMatch()) levels = m.captured(2).toInt();
            *out += "<text:table-of-content text:protected=\"false\" text:name=\"Table of Contents" + num(++m_sectionCount) + "\"><text:table-of-content-source text:outline-level=\"" + num(levels) +
                    "\"><text:index-title-template text:style-name=\"Contents_20_Heading\"/></text:table-of-content-source><text:index-body>";
            m_inToc = true;
            m_tocDepth = delta;
            openedToc = true;
        } else if (m_inToc) m_tocDepth += delta;
    }
    // Numbering.
    int numId = -1;
    const int level = numLevelOf(t, pPr, &numId, pStyle);
    bool inList = numId > 0 && m_numDefs.contains(numId) && !m_inToc;
    // Section: the paragraph that begins one.
    QString master;
    bool startsInline = false;
    if (atBody) {
        if (m_sectionIndex < m_sections.size() && m_pendingMaster != QLatin1String("-")) {
            master = m_pendingMaster;
        }
        Q_UNUSED(startsInline);
    }
    // The paragraph's own properties.
    bool breakBefore = false;
    QByteArray tabs;
    int outline = 0;
    const QByteArray pAttrs = paragraphAttrs(t, pPr, false, &breakBefore, &tabs, &outline);
    // Heading: an outline level (of its style, up the chain) that is the paragraph's.
    int styleOutline = 0;
    {
        QString id = pStyle.isEmpty() ? m_defaultParaStyle : pStyle;
        for (int guard = 0; guard < kMaxStyleChain && !id.isEmpty() && styleOutline == 0; ++guard) {
            const auto it = m_dstyles.constFind(id);
            if (it == m_dstyles.constEnd() || !m_styles) break;
            const int sp = m_styles->child(it->node, "pPr");
            const int ol = sp >= 0 ? m_styles->child(sp, "outlineLvl") : -1;
            if (ol >= 0) styleOutline = m_styles->val(ol).toInt() + 1;
            id = it->basedOn;
        }
    }
    const int headingLevel = outline > 0 ? outline : styleOutline;
    const bool heading = headingLevel >= 1 && headingLevel <= 9 && !m_inToc;
    // Its content.
    ParaCtx pc;
    pc.depth = depth;
    pc.inNote = m_inNote;
    pc.xml = m_pendingBookmarks;
    m_pendingBookmarks.clear();
    emitInline(t, p, &pc, 0);
    // Fields left open at the end of the paragraph keep what they hold.
    while (!pc.fields.isEmpty()) {
        const auto f = pc.fields.takeLast();
        pc.xml = f.savedXml + pc.xml;
        pc.plain = f.savedPlain + pc.plain;
    }
    if (pc.fragments.isEmpty() || !pc.xml.isEmpty()) pc.fragments.push_back({pc.xml, pc.pageBreak});
    else if (pc.pageBreak) m_pendingBreak = true;
    // A paragraph that was only a page break gives its break to the next one.
    bool anyContent = false;
    for (const auto &f : std::as_const(pc.fragments))
        if (!f.first.isEmpty()) anyContent = true;
    const bool carriesSection = atBody && m_sectionOfPara.contains(p);
    if (!anyContent && pc.fragments.size() == 1 && pc.fragments[0].second && !carriesSection) {
        m_pendingBreak = true;
        // (Closed lists and the table of contents are still as they were.)
        if (openedToc || m_inToc) {
        } else return;
    }
    if (!inList) closeLists(lc);
    bool first = true;
    for (const auto &frag : std::as_const(pc.fragments)) {
        const bool pageBreak = frag.second || (first && (breakBefore || m_pendingBreak));
        if (first && !master.isEmpty()) m_pendingBreak = false;
        QByteArray attrs = " style:parent-style-name=\"" + esc(odfStyle) + "\"";
        QByteArray inner;
        QByteArray pa = pAttrs;
        if (pageBreak && master.isEmpty()) pa += " fo:break-before=\"page\"";
        if (first && !master.isEmpty()) attrs += " style:master-page-name=\"" + master.toUtf8() + "\"";
        if (!pa.isEmpty() || !tabs.isEmpty()) inner = "<style:paragraph-properties" + pa + (tabs.isEmpty() ? "/>" : ">" + tabs + "</style:paragraph-properties>");
        QString styleName = odfStyle;
        if (attrs.size() > 0 && (!inner.isEmpty() || !master.isEmpty() || pageBreak)) styleName = m_sink->make("P", "paragraph", attrs, inner);
        if (first) m_pendingBreak = false;
        // The list it is in.
        if (inList && first) {
            const int want = level + 1;
            if (lc->numId != numId || lc->depth == 0) {
                closeLists(lc);
                const bool cont = m_numsSeen.contains(numId);
                m_numsSeen.insert(numId);
                m_numsUsed.insert(numId);
                *out += "<text:list text:style-name=\"L" + num(numId) + "\"" + (cont ? " text:continue-numbering=\"true\"" : "") + ">";
                *out += "<text:list-item>";
                lc->numId = numId;
                lc->depth = 1;
                while (lc->depth < want) {
                    *out += "<text:list><text:list-item>";
                    ++lc->depth;
                }
            } else {
                while (lc->depth > want) {
                    *out += "</text:list-item></text:list>";
                    --lc->depth;
                }
                if (lc->depth == want) *out += "</text:list-item><text:list-item>";
                while (lc->depth < want) {
                    *out += "<text:list><text:list-item>";
                    ++lc->depth;
                }
            }
        }
        const QByteArray tag = heading ? "text:h" : "text:p";
        *out += "<" + tag + " text:style-name=\"" + esc(styleName) + "\"" + (heading ? " text:outline-level=\"" + num(headingLevel) + "\"" : QByteArray());
        *out += frag.first.isEmpty() ? QByteArray("/>") : ">" + frag.first + "</" + tag + ">";
        first = false;
    }
    if (atBody) {
        if (!master.isEmpty()) m_pendingMaster.clear();
        if (m_inToc && !openedToc && m_tocDepth <= 0) {
            closeLists(lc);
            *out += "</text:index-body></text:table-of-content>";
            m_inToc = false;
        } else if (openedToc && m_tocDepth <= 0) {
            *out += "</text:index-body></text:table-of-content>";
            m_inToc = false;
        }
        // The end of a section.
        if (carriesSection) {
            closeLists(lc);
            sectionEnded();
            if (m_sectionWrapperOpen) {
                *out += "</text:section>";
                m_sectionWrapperOpen = false;
            }
            ++m_sectionIndex;
            beginSection(out);
        }
    }
}

// ---- sections in the body

void Writer::collectSections()
{
    const T &t = *m_doc;
    const int body = t.child(t.root(), "body");
    if (body < 0) return;
    // Walks the body's paragraphs (and those in content controls) for the ones that end a section.
    QVector<int> stack{body};
    std::vector<int> order;
    QVector<int> paras;
    struct Frame {
        int parent, next;
    };
    QVector<Frame> frames{{body, t.firstChild(body)}};
    while (!frames.isEmpty()) {
        Frame &f = frames.back();
        if (f.next < 0) {
            frames.pop_back();
            continue;
        }
        const int c = f.next;
        f.next = t.next(c);
        if (t.is(c, "p")) {
            const int pPr = t.child(c, "pPr");
            const int sp = pPr >= 0 ? t.child(pPr, "sectPr") : -1;
            if (sp >= 0) {
                m_sectionOfPara.insert(c, int(m_sections.size()));
                m_sections.push_back(sectionOf(t, sp));
            }
        } else if (t.is(c, "sdt")) {
            const int content = t.child(c, "sdtContent");
            if (content >= 0 && frames.size() < kMaxNesting) frames.push_back(Frame{content, t.firstChild(content)});
        }
    }
    m_sections.push_back(sectionOf(t, t.child(body, "sectPr")));
    // A section that has none of a header's or footer's kinds has the section before's.
    for (int i = 1; i < m_sections.size(); ++i)
        for (int k = 0; k < 3; ++k) {
            if (m_sections[i].headerRef[k].isEmpty()) m_sections[i].headerRef[k] = m_sections[i - 1].headerRef[k];
            if (m_sections[i].footerRef[k].isEmpty()) m_sections[i].footerRef[k] = m_sections[i - 1].footerRef[k];
        }
    // Which sections are a page of their own, and which share the page before's (their columns in a section of the text).
    for (int i = 0; i < m_sections.size(); ++i) {
        SectionInfo &s = m_sections[i];
        bool inlineSection = false;
        if (i > 0 && s.continuous) {
            const SectionInfo &p = m_sections[i - 1];
            inlineSection = s.pageW == p.pageW && s.pageH == p.pageH && s.top == p.top && s.bottom == p.bottom && s.left == p.left && s.right == p.right && s.landscape == p.landscape;
        }
        s.inlineSection = inlineSection;
        s.master = inlineSection ? m_sections[i - 1].master : masterFor(s);
    }
    m_sectionIndex = 0;
    m_pendingMaster = m_sections.isEmpty() ? QString() : m_sections[0].master;
}

void Writer::sectionEnded() {}

void Writer::beginSection(QByteArray *out)
{
    if (m_sectionIndex >= m_sections.size()) return;
    const SectionInfo &s = m_sections[m_sectionIndex];
    if (!s.inlineSection) {
        m_pendingMaster = s.master;
        return;
    }
    m_pendingMaster.clear();
    if (s.columns > 1) {
        const QString style = m_sink->make("Sect", "section", "", "<style:section-properties style:editable=\"false\"><style:columns fo:column-count=\"" + num(s.columns) + "\" fo:column-gap=\"" +
                                                                       inchesFromTwips(double(s.columnGap)).toUtf8() + "\">" + (s.separator ? "<style:column-sep style:width=\"0.0069in\" style:color=\"#000000\" style:height=\"100%\" style:style=\"solid\"/>" : "") +
                                                                       "</style:columns></style:section-properties>");
        *out += "<text:section text:style-name=\"" + style.toUtf8() + "\" text:name=\"Section" + num(++m_sectionCount) + "\">";
        m_sectionWrapperOpen = true;
    }
}

// ---- runs and what is in them

void Writer::emitText(ParaCtx *pc, const QString &raw, const QString &span)
{
    Q_UNUSED(span);
    const QString text = xmlSafe(raw);
    QByteArray x;
    QString seg;
    int spaces = 0;
    auto flushSeg = [&] {
        if (!seg.isEmpty()) x += esc(seg, false);
        seg.clear();
    };
    auto flushSpaces = [&] {
        if (spaces > 0) {
            flushSeg();
            x += spaces == 1 ? QByteArray("<text:s/>") : "<text:s text:c=\"" + num(spaces) + "\"/>";
        }
        spaces = 0;
    };
    for (const QChar c : text) {
        if (c == QLatin1Char(' ')) {
            if (!pc->lastSpace) {
                seg += c;
                pc->lastSpace = true;
            } else ++spaces;
        } else if (c == QLatin1Char('\t')) {
            flushSpaces();
            flushSeg();
            x += "<text:tab/>";
            pc->lastSpace = true;
        } else if (c == QLatin1Char('\n') || c == QLatin1Char('\r')) {
            flushSpaces();
            flushSeg();
            x += "<text:line-break/>";
            pc->lastSpace = true;
        } else {
            flushSpaces();
            seg += c;
            pc->lastSpace = false;
        }
    }
    flushSpaces();
    flushSeg();
    pc->xml += x;
    pc->plain += raw;
    if (!x.isEmpty()) pc->hasContent = true;
}

QString Writer::spanFor(const T &t, int rPr, ParaCtx *pc)
{
    Q_UNUSED(pc);
    QString rStyle;
    if (rPr >= 0) {
        const int rs = t.child(rPr, "rStyle");
        if (rs >= 0) rStyle = t.val(rs);
    }
    QString parent;
    if (!rStyle.isEmpty() && m_dstyles.contains(rStyle) && rStyle != m_defaultCharStyle) parent = m_dstyles[rStyle].odfName;
    const QByteArray attrs = textAttrs(t, rPr);
    if (attrs.isEmpty()) return parent;
    return m_sink->make("T", "text", parent.isEmpty() ? QByteArray() : " style:parent-style-name=\"" + esc(parent) + "\"", "<style:text-properties" + attrs + "/>");
}

void Writer::emitSpan(ParaCtx *pc, const QString &span, const QByteArray &content)
{
    if (content.isEmpty()) return;
    if (span.isEmpty()) pc->xml += content;
    else pc->xml += "<text:span text:style-name=\"" + esc(span) + "\">" + content + "</text:span>";
}

QByteArray Writer::fieldXml(const QString &instrRaw, const QString &resultText, const QByteArray &resultXml, ParaCtx *pc, bool *known)
{
    Q_UNUSED(pc);
    *known = true;
    const QString instr = instrRaw.trimmed();
    static const QRegularExpression tokens(QStringLiteral("\"[^\"]*\"|\\S+"));
    QStringList parts;
    for (auto it = tokens.globalMatch(instr); it.hasNext();) {
        QString p = it.next().captured(0);
        if (p.size() >= 2 && p.startsWith(QLatin1Char('"')) && p.endsWith(QLatin1Char('"'))) p = p.mid(1, p.size() - 2);
        parts << p;
    }
    if (parts.isEmpty()) {
        *known = false;
        return resultXml;
    }
    const QString name = parts[0].toUpper();
    const QByteArray text = esc(resultText, false);
    auto hasSwitch = [&](const char *sw) {
        for (int i = 1; i < parts.size(); ++i)
            if (parts[i].compare(QLatin1String(sw), Qt::CaseInsensitive) == 0) return true;
        return false;
    };
    if (name == QLatin1String("PAGE")) return "<text:page-number text:select-page=\"current\">" + text + "</text:page-number>";
    if (name == QLatin1String("NUMPAGES")) return "<text:page-count>" + text + "</text:page-count>";
    if (name == QLatin1String("NUMWORDS")) return "<text:word-count>" + text + "</text:word-count>";
    if (name == QLatin1String("NUMCHARS")) return "<text:character-count>" + text + "</text:character-count>";
    if (name == QLatin1String("TITLE")) return "<text:title>" + text + "</text:title>";
    if (name == QLatin1String("SUBJECT")) return "<text:subject>" + text + "</text:subject>";
    if (name == QLatin1String("AUTHOR")) return "<text:author-name>" + text + "</text:author-name>";
    if (name == QLatin1String("FILENAME")) return "<text:file-name text:display=\"name\">" + text + "</text:file-name>";
    if ((name == QLatin1String("REF") || name == QLatin1String("PAGEREF") || name == QLatin1String("NOTEREF")) && parts.size() >= 2) {
        const QString bm = parts[1];
        const char *format = name == QLatin1String("PAGEREF") ? "page" : hasSwitch("\\p") ? "direction" : (hasSwitch("\\n") || hasSwitch("\\r") || hasSwitch("\\w")) ? "number" : "text";
        return "<text:bookmark-ref text:reference-format=\"" + QByteArray(format) + "\" text:ref-name=\"" + esc(bm) + "\">" + text + "</text:bookmark-ref>";
    }
    if (name == QLatin1String("SEQ") && parts.size() >= 2) {
        m_sequences.insert(parts[1]);
        return "<text:sequence text:ref-name=\"ref" + esc(parts[1]) + num(++m_bookmarkSeq) + "\" text:name=\"" + esc(parts[1]) + "\" text:formula=\"ooow:" + esc(parts[1]) + "+1\" style:num-format=\"1\">" + text + "</text:sequence>";
    }
    if (name == QLatin1String("HYPERLINK")) {
        QString target;
        bool anchor = false;
        for (int i = 1; i < parts.size(); ++i) {
            if (parts[i].compare(QLatin1String("\\l"), Qt::CaseInsensitive) == 0) {
                anchor = true;
                if (i + 1 < parts.size()) target = parts[i + 1];
                break;
            }
            if (!parts[i].startsWith(QLatin1Char('\\')) && target.isEmpty()) target = parts[i];
        }
        if (target.isEmpty()) {
            *known = false;
            return resultXml;
        }
        return "<text:a xlink:type=\"simple\" xlink:href=\"" + esc(anchor ? QStringLiteral("#") + target : target) + "\">" + resultXml + "</text:a>";
    }
    *known = false;
    return resultXml;
}

void Writer::fieldEnd(ParaCtx *pc)
{
    if (pc->fields.isEmpty()) return;
    const ParaCtx::Field f = pc->fields.takeLast();
    bool known = false;
    const QByteArray made = f.result || f.instr.isEmpty() ? fieldXml(f.instr, pc->plain, pc->xml, pc, &known) : QByteArray();
    pc->xml = f.savedXml + made;
    pc->plain = f.savedPlain + pc->plain;
    pc->lastSpace = false;
}

QByteArray Writer::imagePart(const QString &rid, QString *href)
{
    const Relationship r = m_pkg.relationship(m_source, rid);
    if (r.id.isEmpty() || r.external) return QByteArray();
    const auto done = m_images.constFind(m_source + QLatin1Char('|') + r.target);
    if (done != m_images.constEnd()) {
        *href = done.value();
        return QByteArray("x");
    }
    if (m_imageCount >= kMaxMadeParts || m_pkg.partSize(r.target) > (qint64(64) << 20)) return QByteArray();
    const QByteArray bytes = m_pkg.part(r.target);
    if (bytes.isEmpty()) return QByteArray();
    QString ext = QFileInfo(r.target).suffix().toLower();
    if (ext.isEmpty() || ext.size() > 5) ext = QStringLiteral("bin");
    *href = QStringLiteral("Pictures/image%1.%2").arg(++m_imageCount).arg(ext);
    m_pictures.push_back({*href, bytes});
    m_images.insert(m_source + QLatin1Char('|') + r.target, *href);
    return QByteArray("x");
}

namespace {

QByteArray emuLength(qint64 emu)
{
    return QString::number(double(emu) / 914400.0, 'f', 4).toUtf8() + "in";
}

} // namespace

QByteArray Writer::drawingXml(const T &t, int drawing, ParaCtx *pc, int depth)
{
    const int wp = t.namespaceId(kNsWp);
    if (wp < 0) return QByteArray();
    int anchor = -1;
    for (int c = t.firstChild(drawing); c >= 0; c = t.next(c))
        if (t.is(c, "inline", wp) || t.is(c, "anchor", wp)) {
            anchor = c;
            break;
        }
    if (anchor < 0) return QByteArray();
    const bool inlineFrame = t.is(anchor, "inline", wp);
    // The size, the name, and what it holds.
    qint64 cx = 914400, cy = 914400;
    QString name, descr;
    int blip = -1, txbx = -1;
    {
        const int ext = t.child(anchor, "extent", wp);
        if (ext >= 0) {
            cx = std::max<qint64>(9525, std::min<qint64>(t.attr(ext, "cx").toLongLong(), qint64(914400) * 60));
            cy = std::max<qint64>(9525, std::min<qint64>(t.attr(ext, "cy").toLongLong(), qint64(914400) * 60));
        }
        const int dp = t.child(anchor, "docPr", wp);
        if (dp >= 0) {
            name = t.attr(dp, "name");
            descr = t.attr(dp, "descr");
        }
    }
    {
        QVector<int> stack{anchor};
        int guard = 0;
        while (!stack.isEmpty() && guard++ < 4000 && blip < 0 && txbx < 0) {
            const int n = stack.takeLast();
            if (t.localName(n) == "blip" && blip < 0) blip = n;
            else if (t.localName(n) == "txbxContent" && txbx < 0) txbx = n;
            for (int k = t.firstChild(n); k >= 0; k = t.next(k)) stack.push_back(k);
        }
    }
    if (blip < 0 && txbx < 0) {
        left(QCoreApplication::translate("odt", "drawings, shapes, and charts"));
        return QByteArray();
    }
    // The frame's style: how it is set among the text.
    QByteArray gp;
    QByteArray frameAttrs = " draw:name=\"" + esc(name.isEmpty() ? QStringLiteral("Object%1").arg(++m_frameCount) : name) + "\"";
    if (inlineFrame) {
        gp = " style:vertical-pos=\"top\" style:vertical-rel=\"baseline\" style:horizontal-pos=\"center\" style:horizontal-rel=\"paragraph\" fo:margin-left=\"0in\" fo:margin-right=\"0in\" fo:margin-top=\"0in\" fo:margin-bottom=\"0in\" style:wrap=\"none\"";
        frameAttrs += " text:anchor-type=\"as-char\"";
    } else {
        QByteArray xy;
        qint64 offX = 0, offY = 0;
        QString hPos = QStringLiteral("from-left"), vPos = QStringLiteral("from-top"), hRel = QStringLiteral("paragraph"), vRel = QStringLiteral("paragraph");
        for (int c = t.firstChild(anchor); c >= 0; c = t.next(c)) {
            const bool h = t.is(c, "positionH", wp), v = t.is(c, "positionV", wp);
            if (!h && !v) continue;
            const QString from = t.attr(c, "relativeFrom");
            QString align;
            qint64 off = 0;
            for (int k = t.firstChild(c); k >= 0; k = t.next(k)) {
                if (t.is(k, "align", wp)) align = t.text(k).trimmed();
                else if (t.is(k, "posOffset", wp)) off = t.text(k).trimmed().toLongLong();
            }
            if (h) {
                hRel = from == QLatin1String("page") ? QStringLiteral("page") : from == QLatin1String("margin") ? QStringLiteral("page-content") : from == QLatin1String("character") ? QStringLiteral("char") : QStringLiteral("paragraph");
                if (align == QLatin1String("left")) hPos = QStringLiteral("left");
                else if (align == QLatin1String("center")) hPos = QStringLiteral("center");
                else if (align == QLatin1String("right")) hPos = QStringLiteral("right");
                else if (align == QLatin1String("inside")) hPos = QStringLiteral("inside");
                else if (align == QLatin1String("outside")) hPos = QStringLiteral("outside");
                else offX = off;
            } else {
                vRel = from == QLatin1String("page") ? QStringLiteral("page") : from == QLatin1String("margin") ? QStringLiteral("page-content") : from == QLatin1String("line") ? QStringLiteral("line") : QStringLiteral("paragraph");
                if (align == QLatin1String("top")) vPos = QStringLiteral("top");
                else if (align == QLatin1String("center")) vPos = QStringLiteral("middle");
                else if (align == QLatin1String("bottom")) vPos = QStringLiteral("bottom");
                else offY = off;
            }
        }
        QByteArray wrap = "none";
        QByteArray extra;
        for (int c = t.firstChild(anchor); c >= 0; c = t.next(c)) {
            if (t.is(c, "wrapSquare", wp) || t.is(c, "wrapTight", wp) || t.is(c, "wrapThrough", wp)) {
                const QString side = t.attr(c, "wrapText");
                wrap = side == QLatin1String("left") ? "left" : side == QLatin1String("right") ? "right" : "parallel";
            } else if (t.is(c, "wrapTopAndBottom", wp)) wrap = "none";
            else if (t.is(c, "wrapNone", wp)) {
                wrap = "run-through";
                if (t.attr(anchor, "behindDoc") == QLatin1String("1") || t.attr(anchor, "behindDoc") == QLatin1String("true")) extra = " style:run-through=\"background\"";
                else extra = " style:run-through=\"foreground\"";
            }
        }
        auto dist = [&](const char *a) { return emuLength(t.attr(anchor, a).toLongLong()); };
        gp = " style:vertical-pos=\"" + vPos.toUtf8() + "\" style:vertical-rel=\"" + vRel.toUtf8() + "\" style:horizontal-pos=\"" + hPos.toUtf8() + "\" style:horizontal-rel=\"" + hRel.toUtf8() + "\" style:wrap=\"" + wrap + "\"" + extra +
             " fo:margin-left=\"" + dist("distL") + "\" fo:margin-right=\"" + dist("distR") + "\" fo:margin-top=\"" + dist("distT") + "\" fo:margin-bottom=\"" + dist("distB") + "\"";
        frameAttrs += " text:anchor-type=\"paragraph\"";
        frameAttrs += " svg:x=\"" + emuLength(offX) + "\" svg:y=\"" + emuLength(offY) + "\"";
        frameAttrs += " draw:z-index=\"" + num(std::max<qint64>(0, std::min<qint64>(t.attr(anchor, "relativeHeight").toLongLong() - 251658240, 100000))) + "\"";
    }
    // A picture's cut edges.
    if (blip >= 0) {
        const int fill = t.parent(blip);
        for (int k = t.firstChild(fill); k >= 0; k = t.next(k))
            if (t.localName(k) == "srcRect") {
                auto share = [&](const char *e) { return std::clamp(t.attr(k, e).toDouble() / 100000.0, 0.0, 0.95); };
                const double l = share("l"), tp = share("t"), r = share("r"), b = share("b");
                if (l + r < 0.95 && tp + b < 0.95 && (l > 0 || tp > 0 || r > 0 || b > 0)) {
                    const double w0 = double(cx) / (1.0 - l - r), h0 = double(cy) / (1.0 - tp - b);
                    gp += " fo:clip=\"rect(" + emuLength(qint64(tp * h0)) + ", " + emuLength(qint64(r * w0)) + ", " + emuLength(qint64(b * h0)) + ", " + emuLength(qint64(l * w0)) + ")\"";
                }
            }
    }
    const QString style = m_sink->make("fr", "graphic", " style:parent-style-name=\"" + QByteArray(blip >= 0 ? "Graphics" : "Frame") + "\"", "<style:graphic-properties" + gp + (txbx >= 0 ? " fo:padding=\"0in\" fo:border=\"none\"" : "") + "/>");
    QByteArray x = "<draw:frame draw:style-name=\"" + esc(style) + "\"" + frameAttrs + " svg:width=\"" + emuLength(cx) + "\"";
    if (blip >= 0) {
        QString href;
        const QString rid = t.attr(blip, "embed", wml::NsR);
        if (imagePart(rid, &href).isEmpty()) {
            left(QCoreApplication::translate("odt", "pictures that can't be read"));
            return QByteArray();
        }
        x += " svg:height=\"" + emuLength(cy) + "\"><draw:image xlink:href=\"" + esc(href) + "\" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\"/>";
        if (!descr.isEmpty()) x += "<svg:desc>" + esc(descr, false) + "</svg:desc>";
        x += "</draw:frame>";
        return x;
    }
    // A text box: the paragraphs it holds.
    if (pc->depth >= kMaxNesting - 6) return QByteArray();
    QByteArray inner;
    emitBlocks(t, txbx, &inner, pc->depth + 6, false);
    x += "><draw:text-box fo:min-height=\"" + emuLength(cy) + "\">" + (inner.isEmpty() ? QByteArray("<text:p text:style-name=\"Standard\"/>") : inner) + "</draw:text-box></draw:frame>";
    Q_UNUSED(depth);
    return x;
}

QByteArray Writer::noteXml(const T &t, int ref, bool endnote, int depth)
{
    Q_UNUSED(t);
    const T *tree = endnote ? m_endnotes : m_footnotes;
    if (!tree) return QByteArray();
    const int id = t.attr(ref, "id").toInt();
    int note = -1;
    for (int c = tree->firstChild(tree->root()); c >= 0; c = tree->next(c))
        if (tree->is(c, endnote ? "endnote" : "footnote") && tree->attr(c, "id").toInt() == id) {
            note = c;
            break;
        }
    if (note < 0) return QByteArray();
    const int count = endnote ? ++m_endnoteCount : ++m_noteCount;
    const QString oldSource = m_source;
    const auto rels = m_pkg.relationshipsOfType(m_main, endnote ? QStringLiteral("/endnotes") : QStringLiteral("/footnotes"));
    if (!rels.isEmpty()) m_source = rels.first().target;
    QByteArray inner;
    m_inNote = true;
    emitBlocks(*tree, note, &inner, depth + 4, false);
    m_inNote = false;
    m_source = oldSource;
    if (inner.isEmpty()) inner = "<text:p text:style-name=\"Standard\"/>";
    return "<text:note text:id=\"" + QByteArray(endnote ? "edn" : "ftn") + num(count) + "\" text:note-class=\"" + (endnote ? "endnote" : "footnote") + "\"><text:note-citation>" + num(count) +
           "</text:note-citation><text:note-body>" + inner + "</text:note-body></text:note>";
}

QByteArray Writer::commentStart(int id)
{
    if (!m_comments) return QByteArray();
    const T &t = *m_comments;
    int comment = -1;
    for (int c = t.firstChild(t.root()); c >= 0; c = t.next(c))
        if (t.is(c, "comment") && t.attr(c, "id").toInt() == id) {
            comment = c;
            break;
        }
    if (comment < 0 || m_commentsStarted.contains(QString::number(id))) return QByteArray();
    m_commentsStarted.insert(QString::number(id));
    QString date = t.attr(comment, "date");
    if (date.endsWith(QLatin1Char('Z'))) date.chop(1);
    const QString oldSource = m_source;
    const auto rels = m_pkg.relationshipsOfType(m_main, QStringLiteral("/comments"));
    if (!rels.isEmpty()) m_source = rels.first().target;
    QByteArray inner;
    emitBlocks(t, comment, &inner, kMaxNesting - 8, false);
    m_source = oldSource;
    return "<office:annotation office:name=\"cmt" + num(id) + "\"><dc:creator>" + esc(t.attr(comment, "author"), false) + "</dc:creator>" + (date.isEmpty() ? QByteArray() : "<dc:date>" + esc(date, false) + "</dc:date>") +
           (t.attr(comment, "initials").isEmpty() ? QByteArray() : "<meta:creator-initials>" + esc(t.attr(comment, "initials"), false) + "</meta:creator-initials>") + inner + "</office:annotation>";
}

void Writer::emitRun(const T &t, int run, ParaCtx *pc, int depth)
{
    const int rPr = t.child(run, "rPr");
    const QString span = spanFor(t, rPr, pc);
    // (The text and marks of the run, made into one span.)
    const QByteArray before = pc->xml;
    pc->xml.clear();
    const QString plainBefore = pc->plain;
    bool spanned = true;
    for (int c = t.firstChild(run); c >= 0; c = t.next(c)) {
        const QByteArray name = t.localName(c);
        if (!pc->fields.isEmpty() && !pc->fields.last().result && name != "fldChar" && name != "instrText") continue;
        if (name == "t" || name == "delText") emitText(pc, t.text(c), span);
        else if (name == "tab" || name == "ptab") {
            pc->xml += "<text:tab/>";
            pc->lastSpace = true;
            pc->plain += QLatin1Char('\t');
            pc->hasContent = true;
        } else if (name == "br") {
            const QString type = t.attr(c, "type");
            if (type == QLatin1String("page")) {
                // A page break ends the paragraph so far; what follows begins a page.
                const QByteArray content = pc->xml;
                pc->xml.clear();
                if (!pc->fields.isEmpty()) {
                    pc->xml = content;
                    continue;
                }
                const QByteArray all = before + (span.isEmpty() || content.isEmpty() ? content : "<text:span text:style-name=\"" + esc(span) + "\">" + content + "</text:span>");
                const_cast<QByteArray &>(before) = QByteArray();
                if (!all.isEmpty() || !pc->fragments.isEmpty() || pc->hasContent) pc->fragments.push_back({all, pc->pageBreak || pc->firstHasBreak});
                else pc->firstHasBreak = true;
                pc->pageBreak = true;
                pc->hasContent = false;
                pc->lastSpace = true;
                spanned = false;
                // (the span is applied to what is left of the run)
            } else if (type == QLatin1String("column")) {
            } else {
                pc->xml += "<text:line-break/>";
                pc->lastSpace = true;
                pc->plain += QLatin1Char('\n');
                pc->hasContent = true;
            }
        } else if (name == "cr") {
            pc->xml += "<text:line-break/>";
            pc->lastSpace = true;
            pc->hasContent = true;
        } else if (name == "noBreakHyphen") emitText(pc, QString(QChar(0x2011)), span);
        else if (name == "softHyphen") emitText(pc, QString(QChar(0x00AD)), span);
        else if (name == "sym") {
            bool ok = false;
            uint ch = t.attr(c, "char").toUInt(&ok, 16);
            if (ok && ch >= 0xF000 && ch <= 0xF0FF) ch -= 0xF000;
            if (ok && ch > 0x1F && ch < 0xFFFE) emitText(pc, QString(QChar(ushort(ch))), span);
        } else if (name == "fldChar") {
            const QString type = t.attr(c, "fldCharType");
            if (type == QLatin1String("begin")) {
                if (pc->fields.size() < 32) {
                    ParaCtx::Field f;
                    f.savedXml = pc->xml;
                    f.savedPlain = pc->plain;
                    pc->fields.push_back(f);
                    pc->xml.clear();
                    pc->plain.clear();
                }
            } else if (type == QLatin1String("separate")) {
                if (!pc->fields.isEmpty()) {
                    pc->fields.last().result = true;
                    pc->xml.clear();
                    pc->plain.clear();
                }
            } else if (type == QLatin1String("end")) {
                // (The run's own text goes into the field that ends.)
                if (!span.isEmpty() && !pc->xml.isEmpty()) {
                    pc->xml = "<text:span text:style-name=\"" + esc(span) + "\">" + pc->xml + "</text:span>";
                    spanned = false;
                }
                fieldEnd(pc);
            }
        } else if (name == "instrText") {
            if (!pc->fields.isEmpty()) pc->fields.last().instr += t.text(c);
        } else if (name == "footnoteReference" || name == "endnoteReference") {
            if (!pc->inNote) {
                const QByteArray note = noteXml(t, c, name == "endnoteReference", depth);
                pc->xml += note;
                pc->hasContent = true;
            }
        } else if (name == "commentReference") {
            const int id = t.attr(c, "id").toInt();
            if (!m_commentsStarted.contains(QString::number(id))) {
                const QByteArray a = commentStart(id);
                if (!a.isEmpty()) pc->xml += a;
            }
        } else if (name == "drawing") {
            const QByteArray d = drawingXml(t, c, pc, depth);
            if (!d.isEmpty()) {
                pc->xml += d;
                pc->hasContent = true;
                pc->lastSpace = false;
            }
        } else if (name == "AlternateContent") {
            for (int k = t.firstChild(c); k >= 0; k = t.next(k)) {
                if (t.localName(k) != "Choice") continue;
                for (int d = t.firstChild(k); d >= 0; d = t.next(d))
                    if (t.is(d, "drawing")) {
                        const QByteArray x = drawingXml(t, d, pc, depth);
                        if (!x.isEmpty()) {
                            pc->xml += x;
                            pc->hasContent = true;
                        }
                    }
                break;
            }
        } else if (name == "pict") {
            // A picture of the older form: its image data and size.
            int image = -1;
            QVector<int> stack{c};
            int guard = 0;
            while (!stack.isEmpty() && guard++ < 2000 && image < 0) {
                const int n = stack.takeLast();
                if (t.localName(n) == "imagedata" && image < 0) image = n;
                for (int k = t.firstChild(n); k >= 0; k = t.next(k)) stack.push_back(k);
            }
            if (image >= 0) {
                QString href;
                if (!imagePart(t.attr(image, "id", wml::NsR), &href).isEmpty()) {
                    // The size: the shape's style says it (width:...pt;height:...pt).
                    qint64 cx = 914400, cy = 914400;
                    const int shape = t.parent(image);
                    const QString st = t.attrAny(shape, "style");
                    static const QRegularExpression w(QStringLiteral("width:([0-9.]+)(pt|in|cm|mm)")), h(QStringLiteral("height:([0-9.]+)(pt|in|cm|mm)"));
                    auto toEmu = [](const QRegularExpressionMatch &m) {
                        const double v = m.captured(1).toDouble();
                        const QString u = m.captured(2);
                        return qint64(v * (u == QLatin1String("pt") ? 12700 : u == QLatin1String("in") ? 914400 : u == QLatin1String("cm") ? 360000 : 36000));
                    };
                    if (const auto m = w.match(st); m.hasMatch()) cx = std::max<qint64>(9525, toEmu(m));
                    if (const auto m = h.match(st); m.hasMatch()) cy = std::max<qint64>(9525, toEmu(m));
                    const QString style = m_sink->make("fr", "graphic", " style:parent-style-name=\"Graphics\"",
                                                       "<style:graphic-properties style:vertical-pos=\"top\" style:vertical-rel=\"baseline\" style:horizontal-pos=\"center\" style:horizontal-rel=\"paragraph\" style:wrap=\"none\"/>");
                    pc->xml += "<draw:frame draw:style-name=\"" + esc(style) + "\" draw:name=\"Object" + num(++m_frameCount) + "\" text:anchor-type=\"as-char\" svg:width=\"" + emuLength(cx) + "\" svg:height=\"" +
                               emuLength(cy) + "\"><draw:image xlink:href=\"" + esc(href) + "\" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\"/></draw:frame>";
                    pc->hasContent = true;
                }
            } else left(QCoreApplication::translate("odt", "drawings, shapes, and charts"));
        } else if (name == "object") left(QCoreApplication::translate("odt", "embedded objects"));
    }
    // The span round the run's content.
    QByteArray content = pc->xml;
    pc->xml = before;
    if (spanned && !content.isEmpty()) emitSpan(pc, span, content);
    else pc->xml += content;
    Q_UNUSED(plainBefore);
}

void Writer::emitInline(const T &t, int parent, ParaCtx *pc, int depth)
{
    if (depth > kMaxNesting * 2) return;
    for (int c = t.firstChild(parent); c >= 0; c = t.next(c)) {
        const QByteArray name = t.localName(c);
        if (t.ns(c) != wml::NsW) {
            if (isNs(t, c, "oMath", kNsM) || isNs(t, c, "oMathPara", kNsM)) {
                // Equations are kept as the text they hold.
                QString text;
                QVector<int> stack{c};
                int guard = 0;
                QVector<int> order;
                while (!stack.isEmpty() && guard++ < 4000) {
                    const int n = stack.takeLast();
                    if (t.localName(n) == "t" && t.ns(n) == t.ns(c)) text += t.text(n);
                    QVector<int> kids;
                    for (int k = t.firstChild(n); k >= 0; k = t.next(k)) kids.push_back(k);
                    for (int i = kids.size() - 1; i >= 0; --i) stack.push_back(kids[i]);
                }
                emitText(pc, text, QString());
                left(QCoreApplication::translate("odt", "equations (kept as plain text)"));
            }
            continue;
        }
        if (name == "r") emitRun(t, c, pc, depth + 1);
        else if (name == "hyperlink") {
            const QByteArray saved = pc->xml;
            pc->xml.clear();
            emitInline(t, c, pc, depth + 1);
            const QByteArray inner = pc->xml;
            pc->xml = saved;
            QString href;
            if (t.has(c, "id", wml::NsR)) {
                const Relationship r = m_pkg.relationship(m_source, t.attr(c, "id", wml::NsR));
                if (!r.id.isEmpty()) href = r.target;
            }
            if (t.has(c, "anchor")) href = (href.isEmpty() ? QString() : href) + QLatin1Char('#') + t.attr(c, "anchor");
            if (href.isEmpty() || inner.isEmpty()) pc->xml += inner;
            else pc->xml += "<text:a xlink:type=\"simple\" xlink:href=\"" + esc(href) + "\">" + inner + "</text:a>";
        } else if (name == "fldSimple") {
            if (pc->fields.size() < 32) {
                ParaCtx::Field f;
                f.instr = t.attr(c, "instr");
                f.result = true;
                f.savedXml = pc->xml;
                f.savedPlain = pc->plain;
                pc->fields.push_back(f);
                pc->xml.clear();
                pc->plain.clear();
                emitInline(t, c, pc, depth + 1);
                fieldEnd(pc);
            }
        } else if (name == "bookmarkStart") {
            const QString bm = t.attr(c, "name");
            if (!bm.isEmpty()) {
                m_bookmarkNames.insert(t.attr(c, "id").toInt(), bm);
                pc->xml += "<text:bookmark-start text:name=\"" + esc(bm) + "\"/>";
            }
        } else if (name == "bookmarkEnd") {
            const int id = t.attr(c, "id").toInt();
            if (m_bookmarkNames.contains(id)) pc->xml += "<text:bookmark-end text:name=\"" + esc(m_bookmarkNames.value(id)) + "\"/>";
        } else if (name == "commentRangeStart") {
            pc->xml += commentStart(t.attr(c, "id").toInt());
        } else if (name == "commentRangeEnd") {
            const int id = t.attr(c, "id").toInt();
            if (m_commentsStarted.contains(QString::number(id))) pc->xml += "<office:annotation-end office:name=\"cmt" + num(id) + "\"/>";
        } else if ((name == "ins" || name == "moveTo") && m_sink == &m_content && m_changeCount < kMaxMadeParts) {
            const QString id = QStringLiteral("ct%1").arg(++m_changeCount);
            QString date = t.attr(c, "date");
            if (date.endsWith(QLatin1Char('Z'))) date.chop(1);
            m_changeRegions += "<text:changed-region text:id=\"" + id.toUtf8() + "\"><text:insertion><office:change-info><dc:creator>" + esc(t.attr(c, "author"), false) + "</dc:creator>" +
                               (date.isEmpty() ? QByteArray() : "<dc:date>" + esc(date, false) + "</dc:date>") + "</office:change-info></text:insertion></text:changed-region>";
            pc->xml += "<text:change-start text:change-id=\"" + id.toUtf8() + "\"/>";
            emitInline(t, c, pc, depth + 1);
            pc->xml += "<text:change-end text:change-id=\"" + id.toUtf8() + "\"/>";
        } else if (name == "smartTag" || name == "customXml" || name == "dir" || name == "bdo" || name == "ins" || name == "moveTo") emitInline(t, c, pc, depth + 1);
        else if (name == "sdt") {
            const int content = t.child(c, "sdtContent");
            if (content >= 0) emitInline(t, content, pc, depth + 1);
        } else if ((name == "del" || name == "moveFrom") && m_sink == &m_content && m_changeCount < kMaxMadeParts) {
            // The deleted text is kept with the change, and the change is marked where it was.
            ParaCtx dpc;
            dpc.depth = pc->depth;
            dpc.inNote = true;
            emitInline(t, c, &dpc, depth + 1);
            if (!dpc.xml.isEmpty()) {
                const QString id = QStringLiteral("ct%1").arg(++m_changeCount);
                QString date = t.attr(c, "date");
                if (date.endsWith(QLatin1Char('Z'))) date.chop(1);
                m_changeRegions += "<text:changed-region text:id=\"" + id.toUtf8() + "\"><text:deletion><office:change-info><dc:creator>" + esc(t.attr(c, "author"), false) + "</dc:creator>" +
                                   (date.isEmpty() ? QByteArray() : "<dc:date>" + esc(date, false) + "</dc:date>") + "</office:change-info><text:p text:style-name=\"Standard\">" + dpc.xml + "</text:p></text:deletion></text:changed-region>";
                pc->xml += "<text:change text:change-id=\"" + id.toUtf8() + "\"/>";
            }
        } else if (name == "del" || name == "moveFrom") left(QCoreApplication::translate("odt", "tracked changes in headers and footers (saved as they stand now)"));
        else if (name == "sdtContent") emitInline(t, c, pc, depth + 1);
    }
}

// ---- tables

void Writer::emitTable(const T &t, int tbl, QByteArray *out, int depth, bool body)
{
    if (depth >= kMaxNesting) {
        left(QCoreApplication::translate("odt", "tables nested too deeply"));
        return;
    }
    const int tblPr = t.child(tbl, "tblPr");
    const int grid = t.child(tbl, "tblGrid");
    QVector<qint64> cols;
    if (grid >= 0)
        for (int g = t.firstChild(grid); g >= 0 && cols.size() < kMaxColumns; g = t.next(g))
            if (t.is(g, "gridCol")) cols.push_back(std::max<qint64>(0, qint64(t.attr(g, "w").toDouble())));
    // The cells of each row, with where each begins in the grid.
    struct Cell {
        int node = -1, col = 0, span = 1, vmerge = 0, rowspan = 1;
    };
    QVector<QVector<Cell>> rows;
    QVector<int> rowNodes;
    for (int r = t.firstChild(tbl); r >= 0; r = t.next(r)) {
        int row = r;
        if (t.is(r, "sdt")) {
            const int content = t.child(r, "sdtContent");
            row = content >= 0 ? t.child(content, "tr") : -1;
        }
        if (row < 0 || !t.is(row, "tr")) continue;
        QVector<Cell> cells;
        int col = 0;
        const int trPr = t.child(row, "trPr");
        if (trPr >= 0) {
            const int gb = t.child(trPr, "gridBefore");
            if (gb >= 0) col = std::max(0, std::min(t.val(gb).toInt(), kMaxColumns));
        }
        for (int c = t.firstChild(row); c >= 0; c = t.next(c)) {
            int tc = c;
            if (t.is(c, "sdt")) {
                const int content = t.child(c, "sdtContent");
                tc = content >= 0 ? t.child(content, "tc") : -1;
            }
            if (tc < 0 || !t.is(tc, "tc")) continue;
            Cell cell;
            cell.node = tc;
            cell.col = col;
            const int tcPr = t.child(tc, "tcPr");
            if (tcPr >= 0) {
                const int gs = t.child(tcPr, "gridSpan");
                if (gs >= 0) cell.span = std::max(1, std::min(t.val(gs).toInt(), kMaxColumns));
                const int vm = t.child(tcPr, "vMerge");
                if (vm >= 0) cell.vmerge = t.val(vm) == QLatin1String("restart") ? 1 : 2;
            }
            col += cell.span;
            if (col > kMaxColumns) break;
            cells.push_back(cell);
        }
        rows.push_back(cells);
        rowNodes.push_back(row);
        if (rows.size() > 100000) break;
    }
    if (rows.isEmpty()) return;
    int ncols = int(cols.size());
    for (const auto &r : std::as_const(rows))
        for (const Cell &c : r) ncols = std::max(ncols, c.col + c.span);
    while (cols.size() < ncols) cols.push_back(1440);
    // Rows that a cell spans.
    for (int r = 0; r < rows.size(); ++r)
        for (Cell &c : rows[r]) {
            if (c.vmerge != 1) continue;
            int n = 1;
            for (int r2 = r + 1; r2 < rows.size(); ++r2) {
                bool found = false;
                for (const Cell &d : rows[r2])
                    if (d.col == c.col && d.vmerge == 2) found = true;
                if (!found) break;
                ++n;
            }
            c.rowspan = n;
        }
    // The table's borders: its own, or its style's.
    const T *bt = &t;
    int borders = tblPr >= 0 ? t.child(tblPr, "tblBorders") : -1;
    int styleMar = -1;
    const T *styleTree = m_styles;
    if (borders < 0 && tblPr >= 0 && m_styles) {
        const int ts = t.child(tblPr, "tblStyle");
        QString id = ts >= 0 ? t.val(ts) : QString();
        for (int guard = 0; guard < kMaxStyleChain && !id.isEmpty(); ++guard) {
            const auto it = m_dstyles.constFind(id);
            if (it == m_dstyles.constEnd()) break;
            const int sp = m_styles->child(it->node, "tblPr");
            if (sp >= 0) {
                if (borders < 0 && m_styles->child(sp, "tblBorders") >= 0) {
                    borders = m_styles->child(sp, "tblBorders");
                    bt = styleTree;
                }
                if (styleMar < 0) styleMar = m_styles->child(sp, "tblCellMar");
            }
            id = it->basedOn;
        }
    }
    auto tableBorder = [&](const char *side) -> QString {
        if (borders < 0) return QString();
        const int s = bt->child(borders, side);
        if (s < 0) return QString();
        int space = 0;
        QString lw;
        return borderValue(*bt, s, &space, &lw);
    };
    // Cell margins: the table's, or its style's, or the usual.
    qint64 mar[4] = {0, 108, 0, 108};   // top, left, bottom, right
    {
        const T *mt = &t;
        int cm = tblPr >= 0 ? t.child(tblPr, "tblCellMar") : -1;
        if (cm < 0 && styleMar >= 0) {
            cm = styleMar;
            mt = styleTree;
        }
        if (cm >= 0) {
            static const char *const names[] = {"top", "left", "bottom", "right"};
            for (int i = 0; i < 4; ++i) {
                int n = mt->child(cm, names[i]);
                if (n < 0 && i == 1) n = mt->child(cm, "start");
                if (n < 0 && i == 3) n = mt->child(cm, "end");
                if (n >= 0) mar[i] = qint64(mt->attr(n, "w").toDouble());
            }
        }
    }
    // The table's width and place.
    qint64 gridSum = 0;
    for (qint64 w : std::as_const(cols)) gridSum += w;
    QByteArray tprops;
    QByteArray align = "left";
    qint64 indent = 0;
    bool relWidth = false;
    double rel = 100;
    if (tblPr >= 0) {
        const int jc = t.child(tblPr, "jc");
        if (jc >= 0) {
            const QString v = t.val(jc);
            align = v == QLatin1String("center") ? "center" : (v == QLatin1String("right") || v == QLatin1String("end")) ? "right" : "left";
        }
        const int ind = t.child(tblPr, "tblInd");
        if (ind >= 0) indent = qint64(t.attr(ind, "w").toDouble());
        const int tw = t.child(tblPr, "tblW");
        if (tw >= 0 && t.attr(tw, "type") == QLatin1String("pct")) {
            relWidth = true;
            rel = t.attr(tw, "w").endsWith(QLatin1Char('%')) ? t.attr(tw, "w").chopped(1).toDouble() : t.attr(tw, "w").toDouble() / 50.0;
        }
    }
    const QByteArray widthAttr = relWidth ? " style:width=\"6.5in\" style:rel-width=\"" + QByteArray::number(std::max(1.0, std::min(rel, 100.0))) + "%\"" : " style:width=\"" + inchesFromTwips(double(gridSum)).toUtf8() + "\"";
    if (align == "left" && indent != 0) tprops = widthAttr + " table:align=\"left\" fo:margin-left=\"" + inchesFromTwips(double(indent)).toUtf8() + "\"";
    else tprops = widthAttr + " table:align=\"" + (align == "left" ? QByteArray("margins") : align) + "\"";
    QByteArray tattrs;
    QByteArray extraBreak;
    if (body && depth == 0 && !m_pendingMaster.isEmpty() && !m_inToc) {
        tattrs += " style:master-page-name=\"" + m_pendingMaster.toUtf8() + "\"";
        m_pendingMaster.clear();
        m_pendingBreak = false;
    } else if (m_pendingBreak) {
        tprops += " fo:break-before=\"page\"";
        m_pendingBreak = false;
    }
    const QString tableStyle = m_sink->make("Table", "table", tattrs, "<style:table-properties" + tprops + " table:border-model=\"collapsing\"/>");
    QByteArray x = "<table:table table:name=\"Table" + num(++m_tableCount) + "\" table:style-name=\"" + esc(tableStyle) + "\">";
    for (int i = 0; i < ncols; ++i) {
        const QString cs = m_sink->make("Col", "table-column", "", "<style:table-column-properties style:column-width=\"" + inchesFromTwips(double(cols[i])).toUtf8() + "\"/>");
        x += "<table:table-column table:style-name=\"" + esc(cs) + "\"/>";
    }
    bool inHeader = false;
    int headerOpen = 0;
    for (int r = 0; r < rows.size(); ++r) {
        const int row = rowNodes[r];
        const int trPr = t.child(row, "trPr");
        bool header = false, cantSplit = false;
        QByteArray rowProps;
        if (trPr >= 0) {
            for (int k = t.firstChild(trPr); k >= 0; k = t.next(k)) {
                if (t.is(k, "tblHeader")) header = onValue(t, k);
                else if (t.is(k, "cantSplit")) cantSplit = onValue(t, k);
                else if (t.is(k, "trHeight")) {
                    const QString rule = t.attr(k, "hRule");
                    const double h = t.attr(k, "val").toDouble();
                    if (h > 0) rowProps += (rule == QLatin1String("exact") ? " style:row-height=\"" : " style:min-row-height=\"") + inchesFromTwips(h).toUtf8() + "\"";
                }
            }
        }
        if (header && r == headerOpen && !inHeader) {
            x += "<table:table-header-rows>";
            inHeader = true;
        } else if (inHeader && !header) {
            x += "</table:table-header-rows>";
            inHeader = false;
            headerOpen = -1;
        }
        if (header && inHeader) headerOpen = r + 1;
        if (cantSplit) rowProps += " fo:keep-together=\"always\"";
        QString rs;
        if (!rowProps.isEmpty()) rs = m_sink->make("Row", "table-row", "", "<style:table-row-properties" + rowProps + "/>");
        x += rs.isEmpty() ? QByteArray("<table:table-row>") : "<table:table-row table:style-name=\"" + esc(rs) + "\">";
        // The grid, column by column.
        QHash<int, int> startAt;
        for (int i = 0; i < rows[r].size(); ++i) startAt.insert(rows[r][i].col, i);
        for (int col = 0; col < ncols;) {
            if (!startAt.contains(col)) {
                x += "<table:table-cell office:value-type=\"string\"><text:p text:style-name=\"Standard\"/></table:table-cell>";
                ++col;
                continue;
            }
            const Cell &cell = rows[r][startAt[col]];
            if (cell.vmerge == 2) {
                for (int k = 0; k < cell.span; ++k) x += "<table:covered-table-cell/>";
                col += cell.span;
                continue;
            }
            // The cell's own look.
            const T &ct = t;
            const int tcPr = ct.child(cell.node, "tcPr");
            QByteArray cp;
            {
                // borders: the cell's own, else the table's for the side it is on
                const int cb = tcPr >= 0 ? ct.child(tcPr, "tcBorders") : -1;
                static const struct {
                    const char *side, *odf;
                } sides[] = {{"top", "top"}, {"left", "left"}, {"bottom", "bottom"}, {"right", "right"}};
                for (const auto &s : sides) {
                    QString v;
                    int sideNode = cb >= 0 ? ct.child(cb, s.side) : -1;
                    if (sideNode < 0 && cb >= 0 && std::strcmp(s.side, "left") == 0) sideNode = ct.child(cb, "start");
                    if (sideNode < 0 && cb >= 0 && std::strcmp(s.side, "right") == 0) sideNode = ct.child(cb, "end");
                    QString lw;
                    int space = 0;
                    if (sideNode >= 0) v = borderValue(ct, sideNode, &space, &lw);
                    if (v.isEmpty()) {
                        const bool edge = (std::strcmp(s.side, "top") == 0 && r == 0) || (std::strcmp(s.side, "bottom") == 0 && r + cell.rowspan >= rows.size()) ||
                                          (std::strcmp(s.side, "left") == 0 && cell.col == 0) || (std::strcmp(s.side, "right") == 0 && cell.col + cell.span >= ncols);
                        const bool horizontal = std::strcmp(s.side, "top") == 0 || std::strcmp(s.side, "bottom") == 0;
                        v = tableBorder(edge ? s.side : horizontal ? "insideH" : "insideV");
                    }
                    if (!v.isEmpty()) cp += " fo:border-" + QByteArray(s.odf) + "=\"" + v.toUtf8() + "\"";
                    if (!lw.isEmpty()) cp += " style:border-line-width-" + QByteArray(s.odf) + "=\"" + lw.toUtf8() + "\"";
                }
                if (cb < 0 && borders < 0) cp += "";
                if (tcPr >= 0) {
                    const int shd = ct.child(tcPr, "shd");
                    if (shd >= 0) {
                        const QString col2 = shadeColor(ct, shd);
                        if (!col2.isEmpty()) cp += " fo:background-color=\"" + col2.toUtf8() + "\"";
                    }
                    const int va = ct.child(tcPr, "vAlign");
                    if (va >= 0) {
                        const QString v = ct.val(va);
                        cp += v == QLatin1String("center") ? " style:vertical-align=\"middle\"" : v == QLatin1String("bottom") ? " style:vertical-align=\"bottom\"" : " style:vertical-align=\"top\"";
                    }
                }
                qint64 m4[4] = {mar[0], mar[1], mar[2], mar[3]};
                const int tm = tcPr >= 0 ? ct.child(tcPr, "tcMar") : -1;
                if (tm >= 0) {
                    static const char *const names[] = {"top", "left", "bottom", "right"};
                    for (int i = 0; i < 4; ++i) {
                        int n = ct.child(tm, names[i]);
                        if (n < 0 && i == 1) n = ct.child(tm, "start");
                        if (n < 0 && i == 3) n = ct.child(tm, "end");
                        if (n >= 0) m4[i] = qint64(ct.attr(n, "w").toDouble());
                    }
                }
                cp += " fo:padding-top=\"" + inchesFromTwips(double(m4[0])).toUtf8() + "\" fo:padding-left=\"" + inchesFromTwips(double(m4[1])).toUtf8() + "\" fo:padding-bottom=\"" +
                      inchesFromTwips(double(m4[2])).toUtf8() + "\" fo:padding-right=\"" + inchesFromTwips(double(m4[3])).toUtf8() + "\"";
            }
            const QString cstyle = m_sink->make("Cell", "table-cell", "", "<style:table-cell-properties" + cp + "/>");
            x += "<table:table-cell table:style-name=\"" + esc(cstyle) + "\" office:value-type=\"string\"";
            if (cell.span > 1) x += " table:number-columns-spanned=\"" + num(cell.span) + "\"";
            if (cell.rowspan > 1) x += " table:number-rows-spanned=\"" + num(cell.rowspan) + "\"";
            x += ">";
            QByteArray inner;
            emitBlocks(ct, cell.node, &inner, depth + 1, false);
            if (inner.isEmpty()) inner = "<text:p text:style-name=\"Standard\"/>";
            x += inner + "</table:table-cell>";
            for (int k = 1; k < cell.span; ++k) x += "<table:covered-table-cell/>";
            col += cell.span;
        }
        x += "</table:table-row>";
    }
    if (inHeader) x += "</table:table-header-rows>";
    x += "</table:table>";
    *out += x;
}

} // namespace jp::office::odt
