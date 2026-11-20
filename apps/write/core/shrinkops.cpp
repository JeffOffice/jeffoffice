#include "core/shrinkops.h"

#include "core/contentcontrolops.h"
#include "core/editops.h"
#include "core/pictureops.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QSet>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <algorithm>
#include <cmath>

namespace jp::write {

double shrunkSize(double points)
{
    // (sizes are kept in half-points, as the file format has them: what is measured is what comes back from a saved file)
    auto half = [](double v) { return std::round(v * 2) / 2; };
    const double base = half(points);
    return base <= kShrinkLeastSize ? points : std::max(kShrinkLeastSize, half(base * kShrinkShare));
}

namespace {

using office::wml::RawChild;
using office::wml::RunProps;

// The size the text of this format has in the paragraph: its own, else what its styles give it (the paragraph's, the run's, and the style of the table it is in).
double effectiveSize(WriteDoc &doc, const QTextBlock &block, const QTextCharFormat &f)
{
    if (f.fontPointSize() > 0) return f.fontPointSize();
    const office::wml::TableLayer *layer = doc.tableLayerOf(block.blockFormat().intProperty(flow::fp::Cell));
    return doc.styleCharFormat(block.blockFormat().stringProperty(tp::StyleName), f.stringProperty(tp::CharStyle), layer).fontPointSize();
}

// A size after `steps` steps; `changedAt` (when given, `steps` long) says which of the steps changed it.
double sizeAfter(double size, int steps, QVector<bool> *changedAt = nullptr)
{
    for (int i = 0; i < steps; ++i) {
        const double next = shrunkSize(size);
        if (changedAt && next != size) (*changedAt)[i] = true;
        size = next;
    }
    return size;
}

// The old look that a tracked change of a run's properties keeps is the text's other state, which the shrink scales as well: Reject All must not bring back unshrunk text. Its size (the
// one it names, or the one its styles give when it names none) goes through the same steps.
void scaleRecordedSize(WriteDoc &doc, const QTextBlock &block, QTextCharFormat &f, int steps)
{
    if (!f.hasProperty(prop::RawRPr)) return;
    QVector<RawChild> extra = unpackRaw(storedBytes(f, prop::RawRPr));
    bool changed = false;
    for (RawChild &record : extra) {
        if (!record.xml.startsWith("<w:rPrChange")) continue;
        const qsizetype open = record.xml.indexOf("<w:rPr", 12);
        if (open < 0) continue;
        const qsizetype gt = record.xml.indexOf('>', open);
        if (gt < 0) continue;
        qsizetype end = gt + 1;
        if (record.xml[gt - 1] != '/') {
            const qsizetype close = record.xml.indexOf("</w:rPr>", gt);
            if (close < 0) continue;
            end = close + 8;
        }
        RunProps old = office::wml::readRunPropsXml(record.xml.mid(open, end - open));
        double size = old.size ? *old.size / 2.0 : 0;
        if (size <= 0) {
            const office::wml::TableLayer *layer = doc.tableLayerOf(block.blockFormat().intProperty(flow::fp::Cell));
            size = doc.styleCharFormat(block.blockFormat().stringProperty(tp::StyleName), old.style, layer).fontPointSize();
        }
        const double now = size > 0 ? sizeAfter(size, steps) : size;
        if (now == size) continue;
        old.size = int(std::lround(now * 2));
        record.xml = record.xml.left(open) + "<w:rPr>" + office::wml::runPropsXml(old) + "</w:rPr>" + record.xml.mid(end);
        changed = true;
    }
    if (changed) setStored(f, prop::RawRPr, packRaw(extra));
}

// The stories whose text is made smaller: the body and the footnotes the text cites (the endnotes are paragraphs of the body).
QVector<QTextDocument *> shrinkStories(WriteDoc &doc)
{
    QVector<QTextDocument *> out{doc.story()};
    for (const int id : doc.citedFootnotes())
        if (QTextDocument *note = doc.editableFootnote(id)) out.push_back(note);
    return out;
}

// Whether the paragraph is not made smaller: a piece kept as XML, and the paragraphs of an endnote's copies (the pages set them from the note's first copy, whose paragraphs are the text's,
// which is found by a guard that holds the cursor's note as the one being edited).
bool leftAlone(const QTextBlock &b) { return isLocked(b); }

double scaledSpacing(double points) { return std::max(1.0, std::round(points * kShrinkShare * 20) / 20); }

// The number that begins a note (the first character of a footnote's story, and of the first paragraph of an endnote): edits to the words never reach it, so it is made smaller by the
// same steps on its own. Returns whether it was changed.
bool shrinkNoteNumber(WriteDoc &doc, QTextDocument *story, const QTextBlock &b, int steps)
{
    bool number = false;
    if (story != doc.story()) {
        number = b == story->begin() && noteTextStart(story) > 0;
    } else {
        const EndnoteSpan span = endnoteSpanAt(story, b.position());
        number = span.valid && span.editable && span.blockFrom == b.position() && span.textFrom > span.blockFrom;
    }
    if (!number || b.length() < 2) return false;
    const QTextFragment fragment = b.begin().fragment();
    QTextCharFormat format = fragment.charFormat();
    const double size = effectiveSize(doc, b, format), now = size > 0 ? sizeAfter(size, steps) : size;
    if (size <= 0 || now == size) return false;
    format.setFontPointSize(now);
    QTextCursor t(story);
    t.setPosition(b.position());
    t.setPosition(b.position() + 1, QTextCursor::KeepAnchor);
    t.setCharFormat(format);
    return true;
}

// `steps` steps on one story, as one edit of it.
bool shrinkStory(WriteDoc &doc, QTextDocument *story, int steps)
{
    bool any = false;
    QTextCursor edit(story);
    edit.beginEditBlock();
    {
        // (the copies of an endnote that was changed are set again from its first copy when this ends, as part of the step)
        const NoteScopeGuard whole(doc, edit);
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            QTextCursor c(story);
            c.setPosition(b.position());
            c.setPosition(std::min(b.position() + b.length(), story->characterCount() - 1), QTextCursor::KeepAnchor);
            const NoteScopeGuard scope(c);
            if (leftAlone(b)) continue;
            bool here = false;
            QVector<bool> changedAt(steps, false);
            const QTextBlock block = b;
            changeCharFormat(doc, c, [&](QTextCharFormat &f) {
                const double size = effectiveSize(doc, block, f), now = size > 0 ? sizeAfter(size, steps, &changedAt) : size;
                if (size <= 0 || now == size) return;
                f.setFontPointSize(now);
                scaleRecordedSize(doc, block, f, steps);
                here = true;
            });
            // (an exact line height goes with the text: it is scaled by the same share, once for each step that made the text of the paragraph smaller)
            const QTextBlockFormat bf = b.blockFormat();
            if (here && bf.lineHeightType() == QTextBlockFormat::FixedHeight && bf.lineHeight() > 0) {
                double height = bf.lineHeight();
                for (int i = 0; i < steps; ++i)
                    if (changedAt[i]) height = scaledSpacing(height);
                QTextBlockFormat nf;
                nf.setLineHeight(height, QTextBlockFormat::FixedHeight);
                QTextCursor(b).mergeBlockFormat(nf);
            }
            any = shrinkNoteNumber(doc, story, b, steps) || any;
            any = any || here;
        }
    }
    edit.endEditBlock();
    return any;
}

} // namespace

ShrinkLook shrinkLook(WriteDoc &doc)
{
    ShrinkLook look;
    for (const ContentControl &c : contentControls(doc))
        if (c.props.lockContents) look.locked = true;
    double smallest = 0;
    // (the places where a page starts, counted once each: a section break that starts a page and a paragraph that starts it on its first paragraph are one break)
    QSet<int> starts;
    const QVector<QTextDocument *> stories = shrinkStories(doc);
    for (QTextDocument *story : stories) {
        int section = 0;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            if (story == doc.story()) {
                const int now = doc.sectionAt(b.position());
                if (now != section) {
                    section = now;
                    const QString type = section >= 1 && section < doc.sectionCount() ? doc.sectionProps(section).type : QString();
                    if (section >= 1 && type != QLatin1String("continuous") && type != QLatin1String("nextColumn")) starts.insert(b.position());
                }
                if (b.blockFormat().pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore) starts.insert(b.position());
                const QString text = b.text();
                for (qsizetype at = text.indexOf(flow::kPageBreak); at >= 0; at = text.indexOf(flow::kPageBreak, at + 1))
                    starts.insert(at + 1 < text.size() ? b.position() + int(at) + 1 : b.next().isValid() ? b.next().position() : b.position() + int(at) + 1);
            }
            const NoteScopeGuard scope{QTextCursor(b)};
            if (leftAlone(b)) continue;
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const double size = effectiveSize(doc, b, it.fragment().charFormat());
                if (size > kShrinkLeastSize && (smallest <= 0 || size < smallest)) smallest = size;
            }
        }
    }
    starts.remove(0);   // (the first paragraph of the document starts a page whatever it says)
    look.forcedBreaks = int(starts.size());
    // (the steps until the smallest text over the least size has reached it: the text is never made smaller than that)
    for (double size = smallest; size > kShrinkLeastSize && look.steps < 100; size = shrunkSize(size)) ++look.steps;
    return look;
}

bool shrinkBy(WriteDoc &doc, int steps)
{
    if (steps <= 0) return false;
    // (the steps are not tracked changes, and a style that follows what is done to its paragraphs is not redefined by them)
    const UntrackedEdit untracked;
    const AutoRedefineOff keepStyles;
    bool any = false;
    for (QTextDocument *story : shrinkStories(doc)) any = shrinkStory(doc, story, steps) || any;
    return any;
}

int shrinkTextBoxes(WriteDoc &doc, int steps)
{
    const UntrackedEdit untracked;
    int changedBoxes = 0;
    for (const FloatingObject &o : floatingObjects(doc)) {
        std::unique_ptr<QTextDocument> story = textBoxStory(doc, o.position);
        if (!story || story->characterCount() <= 1) continue;
        // (what is to be changed is found first: a change of a run's look can split or join the runs)
        struct Run {
            int from, to;
            QTextCharFormat format;
        };
        QVector<Run> runs;
        QVector<std::pair<int, double>> spacings;   // a paragraph's position, its new exact line height
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            bool here = false;
            QVector<bool> changedAt(steps, false);
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const QTextFragment fragment = it.fragment();
                QTextCharFormat format = fragment.charFormat();
                const double size = effectiveSize(doc, b, format), now = size > 0 ? sizeAfter(size, steps, &changedAt) : size;
                if (size <= 0 || now == size) continue;
                format.setFontPointSize(now);
                scaleRecordedSize(doc, b, format, steps);
                runs.push_back({fragment.position(), fragment.position() + fragment.length(), format});
                here = true;
            }
            const QTextBlockFormat bf = b.blockFormat();
            if (here && bf.lineHeightType() == QTextBlockFormat::FixedHeight && bf.lineHeight() > 0) {
                double height = bf.lineHeight();
                for (int i = 0; i < steps; ++i)
                    if (changedAt[i]) height = scaledSpacing(height);
                spacings.push_back({b.position(), height});
            }
        }
        for (const Run &r : std::as_const(runs)) {
            QTextCursor t(story.get());
            t.setPosition(r.from);
            t.setPosition(r.to, QTextCursor::KeepAnchor);
            t.setCharFormat(r.format);
        }
        for (const auto &s : std::as_const(spacings)) {
            QTextBlockFormat nf;
            nf.setLineHeight(s.second, QTextBlockFormat::FixedHeight);
            QTextCursor(story->findBlock(s.first)).mergeBlockFormat(nf);
        }
        const bool changed = !runs.isEmpty();
        if (changed && setTextBoxStory(doc, o.position, story.get())) ++changedBoxes;
    }
    return changedBoxes;
}

} // namespace jp::write
