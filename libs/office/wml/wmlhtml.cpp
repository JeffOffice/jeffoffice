#include "wml/wmlhtml.h"

#include <QRegularExpression>
#include <QStringList>

#include <algorithm>
#include <functional>

namespace jp::office::wml {

namespace {

QString escaped(const QString &text)
{
    QString out;
    out.reserve(text.size() + 16);
    for (qsizetype i = 0; i < text.size(); ++i) {
        const QChar ch = text[i];
        if (ch == QLatin1Char('&')) out += QLatin1String("&amp;");
        else if (ch == QLatin1Char('<')) out += QLatin1String("&lt;");
        else if (ch == QLatin1Char('>')) out += QLatin1String("&gt;");
        else if (ch == QLatin1Char('"')) out += QLatin1String("&quot;");
        else if (ch == QLatin1Char(' ') && i + 1 < text.size() && text[i + 1] == QLatin1Char(' ')) out += QLatin1String("&nbsp;");
        else if (ch.unicode() < 0x20 && ch != QLatin1Char('\n')) continue;
        else out += ch;
    }
    return out;
}

QString unescapedXml(QString text)
{
    text.replace(QLatin1String("&lt;"), QLatin1String("<")).replace(QLatin1String("&gt;"), QLatin1String(">")).replace(QLatin1String("&quot;"), QLatin1String("\""));
    text.replace(QLatin1String("&apos;"), QLatin1String("'")).replace(QLatin1String("&amp;"), QLatin1String("&"));
    return text;
}

QString highlightColor(const QString &name)
{
    static const QHash<QString, QString> colors = {
        {QStringLiteral("yellow"), QStringLiteral("#ffff00")},     {QStringLiteral("green"), QStringLiteral("#00ff00")},     {QStringLiteral("cyan"), QStringLiteral("#00ffff")},
        {QStringLiteral("magenta"), QStringLiteral("#ff00ff")},    {QStringLiteral("blue"), QStringLiteral("#0000ff")},      {QStringLiteral("red"), QStringLiteral("#ff0000")},
        {QStringLiteral("darkBlue"), QStringLiteral("#000080")},   {QStringLiteral("darkCyan"), QStringLiteral("#008080")},  {QStringLiteral("darkGreen"), QStringLiteral("#008000")},
        {QStringLiteral("darkMagenta"), QStringLiteral("#800080")}, {QStringLiteral("darkRed"), QStringLiteral("#800000")},  {QStringLiteral("darkYellow"), QStringLiteral("#808000")},
        {QStringLiteral("darkGray"), QStringLiteral("#808080")},   {QStringLiteral("lightGray"), QStringLiteral("#c0c0c0")}, {QStringLiteral("black"), QStringLiteral("#000000")},
        {QStringLiteral("white"), QStringLiteral("#ffffff")}};
    return colors.value(name);
}

bool isHex6(const QString &s)
{
    static const QRegularExpression hex(QStringLiteral("^[0-9A-Fa-f]{6}$"));
    return hex.match(s).hasMatch();
}

bool bordered(const TableBorder &b) { return b.stated() && b.val != QLatin1String("nil") && b.val != QLatin1String("none"); }

bool anyBorder(const TableMark &m) { return bordered(m.top) || bordered(m.left) || bordered(m.bottom) || bordered(m.right) || bordered(m.insideH) || bordered(m.insideV); }

QString numberText(double v)
{
    return QString::number(v, 'f', 1).remove(QRegularExpression(QStringLiteral("\\.0$")));
}

struct Segment {
    QString open, close, text;
};

class Writer {
public:
    Writer(const Document &doc, const HtmlOptions &options) : d(doc), o(options) {}

    QByteArray run()
    {
        QString body = range(0, int(d.body.size()), 0);
        QString head = QStringLiteral("<!DOCTYPE html>\n<html");
        if (!o.language.isEmpty()) head += QStringLiteral(" lang=\"") + escaped(o.language) + QLatin1Char('"');
        head += QStringLiteral(">\n<head>\n<meta charset=\"utf-8\">\n<title>") + escaped(o.title) + QStringLiteral("</title>\n");
        if (o.singleFile) head += css();
        head += QStringLiteral("</head>\n<body>\n");
        return (head + body + QStringLiteral("</body>\n</html>\n")).toUtf8();
    }

private:
    struct ListLevel {
        QString tag;
        int definition = 0;
        bool itemOpen = false;
    };
    struct FieldState {
        bool code = true, link = false;
        QString instruction;
    };
    struct Cell {
        const TableMark *mark = nullptr;
        int from = 0, to = 0, column = 0, rowSpan = 1;
        bool skip = false;
    };
    struct Row {
        const TableMark *mark = nullptr;
        QVector<Cell> cells;
    };

    const Document &d;
    HtmlOptions o;
    qint64 pictureBytes = 0;

    QString css() const
    {
        const QString normal = d.styles.defaultId(QStringLiteral("paragraph"));
        RunProps r = d.styles.run(normal, QString());
        QString family;
        if (r.fonts) family = !r.fonts->ascii.isEmpty() ? r.fonts->ascii : r.fonts->hAnsi;
        QString body = QStringLiteral("margin:2em auto;max-width:50em;padding:0 1em;line-height:1.4;");
        if (!family.isEmpty() && !family.contains(QLatin1Char('"')) && !family.contains(QLatin1Char('<')))
            body += QStringLiteral("font-family:\"") + family + QStringLiteral("\",serif;");
        if (r.size && *r.size > 0) body += QStringLiteral("font-size:") + numberText(*r.size / 2.0) + QStringLiteral("pt;");
        return QStringLiteral("<style>\nbody{") + body + QStringLiteral("}\np,li{margin:0 0 .6em 0}\ntable{border-collapse:collapse;margin:0 0 .6em 0}\ntd,th{vertical-align:top;padding:.2em .4em}\nimg{max-width:100%;height:auto}\n</style>\n");
    }

    // The index of the block that closes the Open block at `at` (the last block when a damaged file has none).
    int closeOf(int at, int limit) const
    {
        const QString tag = d.body[at].tag;
        int depth = 0;
        for (int i = at; i < limit; ++i) {
            const Block &b = d.body[i];
            if (!b.isRaw || b.tag != tag) continue;
            if (b.role == Inline::Open) ++depth;
            else if (b.role == Inline::Close && --depth == 0) return i;
        }
        return limit - 1;
    }

    QString range(int from, int to, int depth)
    {
        QString out;
        QVector<ListLevel> lists;
        auto closeLists = [&](int keep) {
            while (lists.size() > keep) {
                if (lists.last().itemOpen) out += QStringLiteral("</li>");
                out += QStringLiteral("</") + lists.last().tag + QStringLiteral(">\n");
                lists.removeLast();
            }
        };
        for (int i = from; i < to; ++i) {
            const Block &b = d.body[i];
            if (!b.isRaw) {
                paragraph(b.para, &out, &lists, closeLists);
            } else if (b.tag == QLatin1String("tbl") && b.role == Inline::Open) {
                closeLists(0);
                const int end = closeOf(i, to);
                if (depth < kMostHtmlDepth) out += table(i, end, depth + 1);
                i = end;
            }
        }
        closeLists(0);
        return out;
    }

    QString table(int from, int to, int depth)
    {
        QVector<Row> rows;
        for (int k = from + 1; k < to; ++k) {
            const Block &r = d.body[k];
            if (!(r.isRaw && r.tag == QLatin1String("tr") && r.role == Inline::Open)) continue;
            const int rowEnd = closeOf(k, to);
            Row row;
            row.mark = r.table.get();
            int column = row.mark ? row.mark->gridBefore : 0;
            for (int c = k + 1; c < rowEnd; ++c) {
                const Block &cb = d.body[c];
                if (!(cb.isRaw && cb.tag == QLatin1String("tc") && cb.role == Inline::Open)) continue;
                Cell cell;
                cell.mark = cb.table.get();
                cell.from = c + 1;
                cell.to = closeOf(c, rowEnd + 1);
                cell.column = column;
                column += cell.mark ? std::max(1, cell.mark->span) : 1;
                row.cells.push_back(cell);
                c = cell.to;
            }
            rows.push_back(row);
            k = rowEnd;
        }
        // A cell that continues the one above it makes that one taller, and is not written.
        for (int r = 0; r < rows.size(); ++r)
            for (Cell &cell : rows[r].cells) {
                if (!cell.mark) continue;
                if (cell.mark->vMerge == QLatin1String("continue") && r > 0) {
                    cell.skip = true;
                    continue;
                }
                if (cell.mark->vMerge != QLatin1String("restart")) continue;
                for (int below = r + 1; below < rows.size(); ++below) {
                    bool found = false;
                    for (Cell &other : rows[below].cells)
                        if (other.column == cell.column && other.mark && other.mark->vMerge == QLatin1String("continue")) {
                            found = true;
                            break;
                        }
                    if (!found) break;
                    ++cell.rowSpan;
                }
            }
        const TableMark *tm = d.body[from].table.get();
        bool grid = false;
        QString tableStyle;
        if (tm) {
            grid = anyBorder(*tm);
            if (!grid && !tm->style.isEmpty()) grid = anyBorder(d.styles.tableStyle(tm->style).base.tbl);
            if (o.singleFile && tm->width.type == QLatin1String("pct") && tm->width.w > 0) tableStyle = QStringLiteral(" style=\"width:%1%\"").arg(numberText(tm->width.w / 50.0));
        }
        QString out = QStringLiteral("<table") + tableStyle + QStringLiteral(">\n");
        for (int r = 0; r < rows.size(); ++r) {
            out += QStringLiteral("<tr>");
            const bool header = rows[r].mark && rows[r].mark->header;
            const QString tag = header ? QStringLiteral("th") : QStringLiteral("td");
            for (const Cell &cell : rows[r].cells) {
                if (cell.skip) continue;
                QString attrs, style;
                if (cell.mark && cell.mark->span > 1) attrs += QStringLiteral(" colspan=\"%1\"").arg(cell.mark->span);
                if (cell.rowSpan > 1) attrs += QStringLiteral(" rowspan=\"%1\"").arg(cell.rowSpan);
                if (o.singleFile) {
                    if (grid) style += QStringLiteral("border:1px solid #000;");
                    if (cell.mark) {
                        if (isHex6(cell.mark->shdFill)) style += QStringLiteral("background-color:#") + cell.mark->shdFill.toLower() + QLatin1Char(';');
                        if (cell.mark->vAlign == QLatin1String("center")) style += QStringLiteral("vertical-align:middle;");
                        else if (cell.mark->vAlign == QLatin1String("bottom")) style += QStringLiteral("vertical-align:bottom;");
                        if (r == 0 && cell.mark->cellWidth.type == QLatin1String("dxa") && cell.mark->cellWidth.w > 0) style += QStringLiteral("width:") + numberText(cell.mark->cellWidth.w / 20.0) + QStringLiteral("pt;");
                    }
                }
                if (!style.isEmpty()) attrs += QStringLiteral(" style=\"") + style + QLatin1Char('"');
                QString inner = range(cell.from, cell.to, depth);
                if (inner.trimmed().isEmpty()) inner = QStringLiteral("&nbsp;");
                out += QLatin1Char('<') + tag + attrs + QLatin1Char('>') + inner + QStringLiteral("</") + tag + QLatin1Char('>');
            }
            out += QStringLiteral("</tr>\n");
        }
        return out + QStringLiteral("</table>\n");
    }

    QString styleId(const ParaProps &p) const { return p.style.isEmpty() ? d.styles.defaultId(QStringLiteral("paragraph")) : p.style; }

    void paragraph(const Paragraph &para, QString *out, QVector<ListLevel> *lists, const std::function<void(int)> &closeLists)
    {
        const QString sid = styleId(para.props);
        ParaProps pp = d.styles.paragraph(sid);
        pp.overlay(para.props);
        int heading = 0;
        if (pp.outlineLvl && *pp.outlineLvl >= 0 && *pp.outlineLvl < 9) heading = std::min(6, *pp.outlineLvl + 1);
        else if (const Style *st = d.styles.find(sid))
            if (st->name.compare(QLatin1String("Title"), Qt::CaseInsensitive) == 0) heading = 1;
        const int numId = pp.numId.value_or(0);
        const bool item = !heading && numId > 0 && d.numbering.levelOf(numId, std::clamp(pp.ilvl.value_or(0), 0, 8));
        const QString inner = inlines(para, sid, heading != 0);
        if (item) {
            const int level = std::clamp(pp.ilvl.value_or(0), 0, 8);
            const int definition = d.numbering.definitionOf(numId);
            // (Each list level has its own kind; a level the paragraph skips to is made from the ones between.)
            closeLists(std::min<int>(level + 1, int(lists->size())));
            if (lists->size() == level + 1 && (lists->at(level).definition != definition)) closeLists(level);
            while (lists->size() < level + 1) {
                const int at = int(lists->size());
                const Level *lv = d.numbering.levelOf(numId, at);
                ListLevel made;
                made.tag = lv && lv->numFmt != QLatin1String("bullet") && lv->numFmt != QLatin1String("none") ? QStringLiteral("ol") : QStringLiteral("ul");
                made.definition = definition;
                if (!lists->isEmpty() && !lists->last().itemOpen) {
                    *out += QStringLiteral("<li>");
                    lists->last().itemOpen = true;
                }
                *out += QLatin1Char('<') + made.tag;
                if (made.tag == QLatin1String("ol") && lv && lv->start != 1) *out += QStringLiteral(" start=\"%1\"").arg(lv->start);
                *out += QStringLiteral(">\n");
                lists->push_back(made);
            }
            ListLevel &top = lists->last();
            if (top.itemOpen) *out += QStringLiteral("</li>\n");
            *out += QStringLiteral("<li") + styleAttribute(pp, true) + QLatin1Char('>') + inner;
            top.itemOpen = true;
            return;
        }
        closeLists(0);
        if (heading) {
            *out += QStringLiteral("<h%1").arg(heading) + styleAttribute(pp, false) + QLatin1Char('>') + inner + QStringLiteral("</h%1>\n").arg(heading);
            return;
        }
        if (inner.trimmed().isEmpty()) {
            if (o.singleFile) *out += QStringLiteral("<p>&nbsp;</p>\n");
            return;
        }
        *out += QStringLiteral("<p") + styleAttribute(pp, false) + QLatin1Char('>') + inner + QStringLiteral("</p>\n");
    }

    QString styleAttribute(const ParaProps &pp, bool listItem) const
    {
        if (!o.singleFile) return QString();
        QString s;
        const QString jc = pp.jc;
        if (jc == QLatin1String("center")) s += QStringLiteral("text-align:center;");
        else if (jc == QLatin1String("right") || jc == QLatin1String("end")) s += QStringLiteral("text-align:right;");
        else if (jc == QLatin1String("both") || jc == QLatin1String("distribute")) s += QStringLiteral("text-align:justify;");
        if (!listItem) {
            if (pp.indLeft && *pp.indLeft > 0) s += QStringLiteral("margin-left:") + numberText(*pp.indLeft / 20.0) + QStringLiteral("pt;");
            if (pp.indFirst && *pp.indFirst != 0) s += QStringLiteral("text-indent:") + numberText(*pp.indFirst / 20.0) + QStringLiteral("pt;");
        }
        if (pp.indRight && *pp.indRight > 0) s += QStringLiteral("margin-right:") + numberText(*pp.indRight / 20.0) + QStringLiteral("pt;");
        if (pp.before && !pp.beforeAuto.value_or(false)) s += QStringLiteral("margin-top:") + numberText(*pp.before / 20.0) + QStringLiteral("pt;");
        if (pp.after && !pp.afterAuto.value_or(false)) s += QStringLiteral("margin-bottom:") + numberText(*pp.after / 20.0) + QStringLiteral("pt;");
        if (pp.line && *pp.line > 0 && (pp.lineRule.isEmpty() || pp.lineRule == QLatin1String("auto")) && *pp.line != 240) s += QStringLiteral("line-height:") + numberText(*pp.line / 240.0) + QLatin1Char(';');
        return s.isEmpty() ? QString() : QStringLiteral(" style=\"") + s + QLatin1Char('"');
    }

    // The tags a run's looks make, outermost first.
    void marks(const RunProps &effective, const RunProps &base, bool inLink, Segment *seg) const
    {
        auto on = [](const std::optional<bool> &v) { return v.value_or(false); };
        auto wrap = [&](const QString &tag, const QString &attrs = QString()) {
            seg->open += QLatin1Char('<') + tag + attrs + QLatin1Char('>');
            seg->close.prepend(QStringLiteral("</") + tag + QLatin1Char('>'));
        };
        if (o.singleFile) {
            QString s;
            const QString fam = effective.fonts ? (!effective.fonts->ascii.isEmpty() ? effective.fonts->ascii : effective.fonts->hAnsi) : QString();
            const QString baseFam = base.fonts ? (!base.fonts->ascii.isEmpty() ? base.fonts->ascii : base.fonts->hAnsi) : QString();
            if (!fam.isEmpty() && fam != baseFam && !fam.contains(QLatin1Char('"')) && !fam.contains(QLatin1Char('<'))) s += QStringLiteral("font-family:\"") + fam + QStringLiteral("\";");
            if (effective.size && effective.size != base.size && *effective.size > 0) s += QStringLiteral("font-size:") + numberText(*effective.size / 2.0) + QStringLiteral("pt;");
            if (effective.color && isHex6(effective.color->val) && (!base.color || base.color->val != effective.color->val)) s += QStringLiteral("color:#") + effective.color->val.toLower() + QLatin1Char(';');
            const QString hl = highlightColor(effective.highlight);
            if (!hl.isEmpty()) s += QStringLiteral("background-color:") + hl + QLatin1Char(';');
            if (on(effective.smallCaps) && !on(base.smallCaps)) s += QStringLiteral("font-variant:small-caps;");
            if (on(effective.caps) && !on(base.caps)) s += QStringLiteral("text-transform:uppercase;");
            if (!s.isEmpty()) wrap(QStringLiteral("span"), QStringLiteral(" style=\"") + s + QLatin1Char('"'));
        }
        if (on(effective.bold) && !on(base.bold)) wrap(QStringLiteral("b"));
        if (on(effective.italic) && !on(base.italic)) wrap(QStringLiteral("i"));
        const bool underline = effective.underline && effective.underline->val != QLatin1String("none") && !effective.underline->val.isEmpty();
        const bool baseUnderline = base.underline && base.underline->val != QLatin1String("none") && !base.underline->val.isEmpty();
        if (underline && !baseUnderline && !inLink) wrap(QStringLiteral("u"));
        if (on(effective.strike) || on(effective.dstrike)) wrap(QStringLiteral("s"));
        if (effective.vertAlign == QLatin1String("superscript")) wrap(QStringLiteral("sup"));
        else if (effective.vertAlign == QLatin1String("subscript")) wrap(QStringLiteral("sub"));
    }

    QString addressOf(const QString &relationshipId) const
    {
        for (const Relationship &r : d.mainRelationships)
            if (r.id == relationshipId) return r.target;
        return QString();
    }

    static QString safeAddress(const QString &address)
    {
        const QString a = address.trimmed();
        const QString lower = a.toLower();
        if (a.startsWith(QLatin1Char('#')) || lower.startsWith(QLatin1String("http://")) || lower.startsWith(QLatin1String("https://")) || lower.startsWith(QLatin1String("mailto:")) || lower.startsWith(QLatin1String("tel:")))
            return a;
        return QString();
    }

    static QString attributeOf(const QByteArray &raw, const QString &name)
    {
        const QRegularExpression re(QStringLiteral("\\b(?:\\w+:)?") + QRegularExpression::escape(name) + QStringLiteral("=\"([^\"]*)\""));
        const QRegularExpressionMatch m = re.match(QString::fromUtf8(raw.left(8000)));
        return m.hasMatch() ? unescapedXml(m.captured(1)) : QString();
    }

    QString linkOpen(const QByteArray &raw) const
    {
        QString address = attributeOf(raw, QStringLiteral("anchor"));
        if (!address.isEmpty()) address = QLatin1Char('#') + address;
        else address = safeAddress(addressOf(attributeOf(raw, QStringLiteral("id"))));
        if (address.isEmpty()) return QString();
        return QStringLiteral("<a href=\"") + escaped(address) + QStringLiteral("\">");
    }

    QString picture(const Inline &atom)
    {
        static const QRegularExpression embed(QStringLiteral("(?:r:embed|r:id|o:relid)=\"([^\"]+)\""));
        const QString xml = QString::fromUtf8(atom.raw.left(20000));
        const QRegularExpressionMatch m = embed.match(xml);
        if (!m.hasMatch()) return QString();
        QString target = addressOf(m.captured(1));
        if (target.isEmpty()) return QString();
        if (!target.startsWith(QLatin1Char('/'))) target = d.mainName.left(d.mainName.lastIndexOf(QLatin1Char('/')) + 1) + target;
        const KeptPart *part = nullptr;
        for (const KeptPart &p : d.parts)
            if (p.name == target) {
                part = &p;
                break;
            }
        if (!part || part->data.isEmpty()) return QString();
        const QString lower = target.toLower();
        QString mime;
        if (lower.endsWith(QLatin1String(".png"))) mime = QStringLiteral("image/png");
        else if (lower.endsWith(QLatin1String(".jpg")) || lower.endsWith(QLatin1String(".jpeg"))) mime = QStringLiteral("image/jpeg");
        else if (lower.endsWith(QLatin1String(".gif"))) mime = QStringLiteral("image/gif");
        else if (lower.endsWith(QLatin1String(".svg"))) mime = QStringLiteral("image/svg+xml");
        else if (lower.endsWith(QLatin1String(".webp"))) mime = QStringLiteral("image/webp");
        else if (lower.endsWith(QLatin1String(".bmp"))) mime = QStringLiteral("image/bmp");
        if (mime.isEmpty() || pictureBytes + part->data.size() > kMostHtmlPictureBytes) return QString();
        pictureBytes += part->data.size();
        QString img = QStringLiteral("<img src=\"data:") + mime + QStringLiteral(";base64,") + QString::fromLatin1(part->data.toBase64()) + QStringLiteral("\" alt=\"") + escaped(attributeOf(atom.raw, QStringLiteral("descr"))) + QLatin1Char('"');
        if (o.singleFile) {
            static const QRegularExpression extent(QStringLiteral("<wp:extent\\s+cx=\"(\\d+)\"\\s+cy=\"(\\d+)\""));
            const QRegularExpressionMatch e = extent.match(xml);
            if (e.hasMatch()) {
                const qint64 cx = e.captured(1).toLongLong() / 9525, cy = e.captured(2).toLongLong() / 9525;
                if (cx > 0 && cy > 0 && cx < 100000 && cy < 100000) img += QStringLiteral(" width=\"%1\" height=\"%2\"").arg(cx).arg(cy);
            }
        }
        return img + QLatin1Char('>');
    }

    QString inlines(const Paragraph &para, const QString &sid, bool heading)
    {
        const RunProps paragraphLook = d.styles.run(sid, QString());
        // What a run is compared with: the heading's own look; in a single file, the page's (its style sheet gives it to the body); else nothing.
        const RunProps base = heading ? paragraphLook : o.singleFile ? d.styles.run(d.styles.defaultId(QStringLiteral("paragraph")), QString()) : RunProps();
        QString out;
        QVector<Segment> pending;
        auto flush = [&] {
            for (const Segment &s : std::as_const(pending)) out += s.open + s.text + s.close;
            pending.clear();
        };
        auto addText = [&](const QString &text, const Segment &marks) {
            if (!pending.isEmpty() && pending.last().open == marks.open) pending.last().text += text;
            else {
                Segment s = marks;
                s.text = text;
                pending.push_back(s);
            }
        };
        int deleted = 0, linkDepth = 0;
        QVector<FieldState> fields;
        auto hidden = [&] {
            if (deleted > 0) return true;
            for (const FieldState &f : std::as_const(fields))
                if (f.code) return true;
            return false;
        };
        auto marksFor = [&](const RunProps &run) {
            RunProps eff = d.styles.run(sid, run.style);
            eff.overlay(run);
            Segment s;
            if (!eff.vanish.value_or(false)) marks(eff, base, linkDepth > 0, &s);
            return std::pair<RunProps, Segment>(eff, s);
        };
        for (const Inline &in : para.content) {
            if (in.kind == Inline::Text) {
                if (hidden()) continue;
                const auto [eff, seg] = marksFor(in.props);
                if (eff.vanish.value_or(false)) continue;
                QString text = escaped(in.text);
                text.replace(QLatin1Char('\t'), QStringLiteral("&nbsp;&nbsp;&nbsp;&nbsp;"));
                text.replace(QChar(QChar::LineSeparator), QStringLiteral("<br>"));
                text.remove(QChar(0x000C)).remove(QChar(0x000E));
                if (!text.isEmpty()) addText(text, seg);
            } else if (in.kind == Inline::ParaAtom) {
                if (in.tag == QLatin1String("hyperlink")) {
                    flush();
                    if (in.role == Inline::Open) {
                        const QString open = linkOpen(in.raw);
                        if (!open.isEmpty()) {
                            out += open;
                            ++linkDepth;
                        }
                        // (A link with no address we may write is kept as plain text: its depth is not counted.)
                    } else if (in.role == Inline::Close && linkDepth > 0) {
                        out += QStringLiteral("</a>");
                        --linkDepth;
                    }
                } else if (in.tag == QLatin1String("del") || in.tag == QLatin1String("moveFrom")) {
                    if (in.role == Inline::Open) ++deleted;
                    else if (in.role == Inline::Close && deleted > 0) --deleted;
                } else if (in.tag == kFieldGroupTag) {
                    if (hidden()) continue;
                    const QString xml = QString::fromUtf8(in.raw.left(200000));
                    static const QRegularExpression instr(QStringLiteral("<w:instrText[^>]*>([^<]*)</w:instrText>"));
                    static const QRegularExpression text(QStringLiteral("<w:t(?:\\s[^>]*)?>([^<]*)</w:t>"));
                    const qsizetype sep = xml.indexOf(QLatin1String("w:fldCharType=\"separate\""));
                    QString code, result;
                    const QRegularExpressionMatch im = instr.match(xml);
                    if (im.hasMatch()) code = unescapedXml(im.captured(1));
                    QRegularExpressionMatchIterator it = text.globalMatch(xml, sep < 0 ? 0 : sep);
                    while (it.hasNext()) result += unescapedXml(it.next().captured(1));
                    if (result.isEmpty()) continue;
                    const auto [eff, seg] = marksFor(in.props);
                    const QString link = hyperlinkOf(code);
                    flush();
                    if (!link.isEmpty()) out += QStringLiteral("<a href=\"") + escaped(link) + QStringLiteral("\">");
                    addText(escaped(result), seg);
                    flush();
                    if (!link.isEmpty()) out += QStringLiteral("</a>");
                }
            } else {   // a RunAtom
                if (in.tag == QLatin1String("fldChar")) {
                    if (in.key == QLatin1String("begin")) fields.push_back(FieldState());
                    else if (in.key == QLatin1String("separate") && !fields.isEmpty()) {
                        fields.last().code = false;
                        const QString link = hyperlinkOf(fields.last().instruction);
                        if (!link.isEmpty() && !hidden()) {
                            flush();
                            out += QStringLiteral("<a href=\"") + escaped(link) + QStringLiteral("\">");
                            fields.last().link = true;
                            ++linkDepth;
                        }
                    } else if (in.key == QLatin1String("end") && !fields.isEmpty()) {
                        if (fields.last().link) {
                            flush();
                            out += QStringLiteral("</a>");
                            --linkDepth;
                        }
                        fields.removeLast();
                    }
                    continue;
                }
                if (in.tag == QLatin1String("instrText")) {
                    if (!fields.isEmpty() && fields.last().code) {
                        static const QRegularExpression instr(QStringLiteral("<w:instrText[^>]*>([^<]*)</w:instrText>"));
                        const QRegularExpressionMatch im = instr.match(QString::fromUtf8(in.raw.left(4000)));
                        if (im.hasMatch()) fields.last().instruction += unescapedXml(im.captured(1));
                    }
                    continue;
                }
                if (hidden()) continue;
                if (in.tag == QLatin1String("drawing") || in.tag == QLatin1String("pict")) {
                    const QString img = picture(in);
                    if (!img.isEmpty()) {
                        flush();
                        out += img;
                    }
                } else if (in.tag == QLatin1String("br") || in.tag == QLatin1String("cr")) {
                    if (in.key.isEmpty() || in.key == QLatin1String("textWrapping")) {
                        flush();
                        out += QStringLiteral("<br>");
                    }
                } else if (in.tag == QLatin1String("footnoteReference") || in.tag == QLatin1String("endnoteReference")) {
                    if (!in.display.isEmpty()) {
                        Segment s;
                        s.open = QStringLiteral("<sup>");
                        s.close = QStringLiteral("</sup>");
                        addText(escaped(in.display), s);
                    }
                } else if (!in.display.isEmpty() && !in.display.startsWith(QLatin1Char('['))) {
                    addText(escaped(in.display).replace(QChar(QChar::LineSeparator), QStringLiteral("<br>")), marksFor(in.props).second);
                }
            }
        }
        flush();
        while (linkDepth-- > 0) out += QStringLiteral("</a>");
        return out;
    }

    static QString hyperlinkOf(const QString &code)
    {
        static const QRegularExpression link(QStringLiteral("^\\s*HYPERLINK\\s+\"([^\"]+)\""), QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch m = link.match(code);
        return m.hasMatch() ? safeAddress(m.captured(1)) : QString();
    }
};

} // namespace

QByteArray documentToHtml(const Document &doc, const HtmlOptions &options)
{
    return Writer(doc, options).run();
}

} // namespace jp::office::wml
