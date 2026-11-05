#include "core/dropcapops.h"

#include "core/editops.h"
#include "core/writeprops.h"
#include "flow/flowdocument.h"
#include "text/textprops.h"
#include "wml/wml.h"

#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>

using namespace jp::office::wml;

namespace jp::write {

namespace {

bool hasDropCapFrame(const QTextBlockFormat &bf)
{
    if (!bf.hasProperty(prop::RawPPr)) return false;
    for (const RawChild &ch : unpackRaw(storedBytes(bf, prop::RawPPr)))
        if (ch.xml.startsWith("<w:framePr") && ch.xml.contains("dropCap=\"") && !ch.xml.contains("dropCap=\"none\"")) return true;
    return false;
}

} // namespace

bool isDropCapFrame(const QTextBlock &block) { return block.isValid() && hasDropCapFrame(block.blockFormat()); }

DropCapSetup dropCapOf(const QTextBlock &block)
{
    DropCapSetup s;
    if (!block.isValid()) return s;
    const QTextBlockFormat bf = block.blockFormat();
    if (bf.intProperty(tp::DropCapLines) > 0) {
        s.lines = std::clamp(bf.intProperty(tp::DropCapLines), 1, kMostDropCapLines);
        s.chars = std::clamp(bf.intProperty(tp::DropCapChars), 1, kMostDropCapChars);
        s.font = bf.stringProperty(tp::DropCapFont);
        return s;
    }
    // (A file's own: the frame, or the paragraph after it.)
    QTextBlock frame = hasDropCapFrame(bf) ? block : block.previous();
    if (frame.isValid() && hasDropCapFrame(frame.blockFormat())) {
        for (const RawChild &ch : unpackRaw(storedBytes(frame.blockFormat(), prop::RawPPr))) {
            if (!ch.xml.startsWith("<w:framePr")) continue;
            const int at = ch.xml.indexOf("w:lines=\"");
            const int end = at < 0 ? -1 : ch.xml.indexOf('"', at + 9);
            s.lines = at < 0 || end < 0 ? 3 : std::clamp(ch.xml.mid(at + 9, end - at - 9).toInt(), 1, kMostDropCapLines);
            s.chars = std::clamp(int(frame.length()) - 1, 1, kMostDropCapChars);
            break;
        }
    }
    return s;
}

bool canHaveDropCap(const QTextBlock &block)
{
    if (!block.isValid()) return false;
    const QTextBlockFormat bf = block.blockFormat();
    if (bf.hasProperty(prop::RawBlock) || bf.hasProperty(prop::NumId) || bf.intProperty(flow::fp::Cell) != 0 || bf.hasProperty(prop::EndnoteBlock) || hasDropCapFrame(bf)) return false;
    if (block.previous().isValid() && hasDropCapFrame(block.previous().blockFormat())) return false;
    const QString text = block.text();
    return !text.isEmpty() && !text.at(0).isSpace() && text.at(0) != QChar::ObjectReplacementCharacter && text.at(0).category() != QChar::Other_Control;
}

bool setDropCap(WriteDoc &doc, QTextCursor &c, const DropCapSetup &setup)
{
    const NoteScopeGuard scopeGuard(doc, c);
    if (!intoBody(c)) return false;
    const QTextBlock block = c.block();
    const DropCapSetup now = dropCapOf(block);
    if (setup.lines <= 0 && now.lines <= 0) return false;
    if (setup.lines > 0 && !canHaveDropCap(block)) return false;
    if (setup.lines <= 0 && block.blockFormat().intProperty(tp::DropCapLines) <= 0) return false;   // (a file's own is left as it is)
    DropCapSetup want = setup;
    want.lines = std::clamp(want.lines, 0, kMostDropCapLines);
    want.chars = std::clamp(want.chars, 1, kMostDropCapChars);
    if (want == now) return true;
    QTextCursor at(block);
    changeBlockFormat(doc, at, [want](QTextBlockFormat &bf) {
        if (want.lines <= 0) {
            bf.clearProperty(tp::DropCapLines);
            bf.clearProperty(tp::DropCapChars);
            bf.clearProperty(tp::DropCapFont);
            return;
        }
        bf.setProperty(tp::DropCapLines, want.lines);
        bf.setProperty(tp::DropCapChars, want.chars);
        if (want.font.isEmpty()) bf.clearProperty(tp::DropCapFont);
        else bf.setProperty(tp::DropCapFont, want.font);
    });
    return true;
}

} // namespace jp::write
