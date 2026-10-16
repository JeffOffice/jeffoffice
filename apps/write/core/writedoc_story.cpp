// Filling the story from the body of a file: paragraphs and runs become blocks and formatted text, lists become Qt lists, and what is kept
// as XML becomes the pieces of writeprops.h.

#include "core/writedoc.h"

#include "core/wordformat.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QHash>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

namespace {

QString letters(int n, bool upper)
{
    QString s;
    for (n = std::max(1, n); n > 0; n = (n - 1) / 26) s.prepend(QChar((upper ? 'A' : 'a') + (n - 1) % 26));
    return s;
}

QString roman(int n, bool upper)
{
    static const std::pair<int, const char *> table[] = {{1000, "m"}, {900, "cm"}, {500, "d"}, {400, "cd"}, {100, "c"}, {90, "xc"}, {50, "l"}, {40, "xl"}, {10, "x"}, {9, "ix"}, {5, "v"}, {4, "iv"}, {1, "i"}};
    QString s;
    n = std::clamp(n, 1, 3999);
    for (const auto &t : table)
        while (n >= t.first) {
            s += QLatin1String(t.second);
            n -= t.first;
        }
    return upper ? s.toUpper() : s;
}

QString counterText(const QString &format, int n)
{
    if (format == QLatin1String("lowerLetter")) return letters(n, false);
    if (format == QLatin1String("upperLetter")) return letters(n, true);
    if (format == QLatin1String("lowerRoman")) return roman(n, false);
    if (format == QLatin1String("upperRoman")) return roman(n, true);
    if (format == QLatin1String("decimalZero")) return QStringLiteral("%1").arg(n, 2, 10, QLatin1Char('0'));
    return QString::number(n);
}

struct NumState {
    int count[9];
    QTextList *list[9];
    NumState()
    {
        for (int i = 0; i < 9; ++i) {
            count[i] = -1;
            list[i] = nullptr;
        }
    }
};

} // namespace

void WriteDoc::buildStory(const QVector<Block> &body)
{
    QTextDocument *story = this->story();
    const StyleSheet &sheet = m_wml.styles;
    const Theme *thm = theme();
    const QString defaultStyle = defaultParagraphStyle();

    QTextCursor c(story);
    c.beginEditBlock();
    bool firstBlock = true;
    auto startBlock = [&](const QTextBlockFormat &bf, const QTextCharFormat &cf) {
        if (firstBlock) {
            c.setBlockFormat(bf);
            c.setBlockCharFormat(cf);
            firstBlock = false;
        } else {
            c.insertBlock(bf, cf);
        }
    };

    QHash<QByteArray, QTextCharFormat> formats;
    auto formatOf = [&](const QString &paraStyle, const RunProps &props) {
        const QByteArray key = paraStyle.toUtf8() + '\x01' + runPropsXml(props);
        auto it = formats.constFind(key);
        if (it != formats.constEnd()) return it.value();
        RunProps resolved = sheet.run(paraStyle, props.style);
        resolved.overlay(props);
        resolved.extra = props.extra;
        resolved.style = props.style;
        const QTextCharFormat f = charFormatOf(resolved, thm, &props);
        formats.insert(key, f);
        return f;
    };

    QHash<int, NumState> numbering;
    QHash<QString, ParaProps> baselines;
    QVector<Block> pending;   // body elements kept as XML that wait for the next paragraph

    for (const Block &blk : body) {
        if (blk.isRaw) {
            if (blk.display.isEmpty()) {
                pending.push_back(blk);
                continue;
            }
            // An element kept as XML that is shown: a stand-in paragraph that can't be edited.
            QTextBlockFormat pf;
            pf.setAlignment(Qt::AlignHCenter);
            pf.setTopMargin(6);
            pf.setBottomMargin(6);
            setStored(pf, prop::RawBlock, packBlocks({blk}));
            if (!pending.isEmpty()) {
                setStored(pf, prop::RawBefore, packBlocks(pending));
                pending.clear();
            }
            QTextCharFormat pc;
            pc.setFontPointSize(10);
            pc.setFontItalic(true);
            pc.setForeground(QBrush(QColor(0x80, 0x80, 0x80)));
            pc.setProperty(tp::ThemeFont, QStringLiteral("minor"));
            pc.setProperty(tp::Field, QStringLiteral("raw:") + blk.display);
            startBlock(pf, pc);
            c.insertText(QString(QChar::ObjectReplacementCharacter), pc);
            continue;
        }

        const Paragraph &p = blk.para;
        const ParaProps &direct = p.props;
        const QString styleId = direct.style.isEmpty() ? defaultStyle : direct.style;
        const ParaProps base = sheet.paragraph(styleId);
        int numId = direct.numId ? *direct.numId : base.numId.value_or(0);
        const int level = std::clamp(direct.ilvl ? *direct.ilvl : base.ilvl.value_or(0), 0, 8);
        const AbstractNum *definition = numId > 0 ? m_wml.numbering.abstractOf(numId) : nullptr;
        if (!definition || !definition->levels[level].present) numId = 0;
        // (The list's indents are the style's to override when the style says the paragraph is numbered, and the paragraph's own numbering
        // overrides its style's.)
        const bool numberedByItself = direct.numId.has_value();
        ParaProps resolved = numId > 0 ? baselineParagraph(styleId, numId, level, numberedByItself, &baselines) : base;
        resolved.overlay(direct);
        // (What the paragraph keeps as written is its own, not its style's.)
        resolved.extra = direct.extra;
        resolved.spacingAttrs = direct.spacingAttrs;
        resolved.indAttrs = direct.indAttrs;
        resolved.style = styleId;
        resolved.numId = numId;
        resolved.ilvl = level;
        QTextBlockFormat bf = blockFormatOf(resolved);
        if (!p.attrs.isEmpty()) setStored(bf, prop::ParaAttrs, p.attrs);
        // A paragraph that says it is numbered as its style says: that it said so is kept, for it changes whose indents win.
        if (numId > 0 && numberedByItself && base.numId.value_or(0) == numId) bf.setProperty(prop::NumDirect, true);
        // The tab stops of its own that the engine has no kind for (a bar) are kept beside the others.
        if (direct.tabs) {
            QVector<TabStop> odd;
            for (const TabStop &t : *direct.tabs)
                if (t.val == QLatin1String("bar") || t.val == QLatin1String("num")) odd.push_back(t);
            if (!odd.isEmpty()) setStored(bf, prop::OddTabs, packTabs(odd));
        }
        if (!pending.isEmpty()) {
            setStored(bf, prop::RawBefore, packBlocks(pending));
            pending.clear();
        }
        RunProps mark = sheet.run(styleId, direct.markRun ? direct.markRun->style : QString());   // (a character style on the mark counts)
        if (direct.markRun) {
            mark.overlay(*direct.markRun);
            mark.extra = direct.markRun->extra;
            mark.style = direct.markRun->style;
        }
        const QTextCharFormat markFormat = charFormatOf(mark, thm, direct.markRun ? &*direct.markRun : nullptr);

        // A list: the item's counter, and the marker where Qt's own counting can't make it.
        QTextListFormat lf;
        bool newList = false;
        QTextList *continuing = nullptr;
        if (numId > 0) {
            NumState &st = numbering[numId];
            const Level &l = definition->levels[level];
            const Num *num = m_wml.numbering.num(numId);
            auto startOf = [&](int lvl) { return num && num->startOverrides.contains(lvl) ? num->startOverrides.value(lvl) : definition->levels[lvl].start; };
            for (int i = level + 1; i < 9; ++i) {
                st.count[i] = -1;
                st.list[i] = nullptr;
            }
            st.count[level] = st.count[level] < 0 ? startOf(level) : st.count[level] + 1;
            lf = listFormat(numId, level);
            continuing = st.list[level];
            newList = !continuing;
            // Several levels in the marker, the legal style, or no number of its own: the marker is made here.
            const bool isBullet = l.numFmt == QLatin1String("bullet") || l.numFmt == QLatin1String("none");
            const int placeholders = l.text.count(QLatin1Char('%'));
            const bool own = l.text.contains(QStringLiteral("%%1").arg(level + 1));
            if (!isBullet && (l.legal || placeholders > 1 || !own)) {
                QString marker = l.text;
                for (int k = 8; k >= 0; --k) {
                    const QString token = QStringLiteral("%%1").arg(k + 1);
                    if (!marker.contains(token)) continue;
                    const Level &lk = definition->levels[k];
                    const int n = st.count[k] < 0 ? (lk.present ? startOf(k) : 1) : st.count[k];
                    marker.replace(token, counterText(l.legal ? QStringLiteral("decimal") : lk.numFmt, n));
                }
                bf.setProperty(tp::BulletChar, marker);
                lf.setStyle(QTextListFormat::ListDisc);
            }
        }

        startBlock(bf, markFormat);
        if (numId > 0) {
            NumState &st = numbering[numId];
            if (newList) st.list[level] = c.createList(lf);
            else continuing->add(c.block());
        }

        for (const Inline &in : p.content) {
            if (in.kind == Inline::Text) {
                if (!in.text.isEmpty()) c.insertText(in.text, formatOf(styleId, in.props));
                continue;
            }
            QTextCharFormat af = formatOf(styleId, in.props);
            setStored(af, prop::Atom, packAtom(in));
            af.setProperty(tp::Field, QStringLiteral("raw:") + in.display);
            c.insertText(QString(QChar::ObjectReplacementCharacter), af);
        }
        if (direct.section) {
            QTextBlockFormat last = c.blockFormat();
            last.setProperty(flow::fp::SectionBreakAfter, true);
            setStored(last, prop::SectionData, packSection(*direct.section));
            c.setBlockFormat(last);
        }
    }

    if (firstBlock) {
        // Nothing in the body: one empty paragraph in the default style.
        startBlock(styleBlockFormat(defaultStyle), styleCharFormat(defaultStyle));
    }
    if (!pending.isEmpty()) {
        QTextBlockFormat bf = c.blockFormat();
        setStored(bf, prop::RawAfter, packBlocks(pending));
        c.setBlockFormat(bf);
    }
    c.endEditBlock();
}

} // namespace jp::write
