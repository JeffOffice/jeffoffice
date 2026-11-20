#include "core/shrinkops.h"

#include "core/contentcontrolops.h"
#include "core/editops.h"
#include "core/pictureops.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <algorithm>
#include <cmath>

namespace jp::write {

double shrunkSize(double points)
{
    return points <= kShrinkLeastSize ? points : std::max(kShrinkLeastSize, std::round(points * kShrinkShare * 10) / 10);
}

namespace {

// The size the text of this format has in the paragraph: its own, else its style's.
double effectiveSize(WriteDoc &doc, const QTextBlock &block, const QTextCharFormat &f)
{
    if (f.fontPointSize() > 0) return f.fontPointSize();
    return doc.styleCharFormat(block.blockFormat().stringProperty(tp::StyleName), f.stringProperty(tp::CharStyle)).fontPointSize();
}

// The stories whose text is made smaller: the body and the footnotes the text cites.
QVector<QTextDocument *> shrinkStories(WriteDoc &doc)
{
    QVector<QTextDocument *> out{doc.story()};
    for (const int id : doc.citedFootnotes())
        if (QTextDocument *note = doc.editableFootnote(id)) out.push_back(note);
    return out;
}

bool leftAlone(const QTextBlock &b) { return isLocked(b) || b.blockFormat().hasProperty(prop::EndnoteBlock); }

double scaledSpacing(double points) { return std::max(1.0, std::round(points * kShrinkShare * 20) / 20); }

bool shrinkStory(WriteDoc &doc, QTextDocument *story)
{
    bool any = false;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        if (leftAlone(b)) continue;
        QTextCursor c(story);
        c.setPosition(b.position());
        c.setPosition(std::min(b.position() + b.length(), story->characterCount() - 1), QTextCursor::KeepAnchor);
        bool here = false;
        const QTextBlock block = b;
        changeCharFormat(doc, c, [&](QTextCharFormat &f) {
            const double size = effectiveSize(doc, block, f), now = shrunkSize(size);
            if (size <= 0 || now == f.fontPointSize()) return;
            f.setFontPointSize(now);
            here = true;
        });
        // (an exact line height goes with the text: it is scaled by the same share, and only where the text was made smaller)
        const QTextBlockFormat bf = b.blockFormat();
        if (here && bf.lineHeightType() == QTextBlockFormat::FixedHeight && bf.lineHeight() > 0) {
            QTextBlockFormat nf;
            nf.setLineHeight(scaledSpacing(bf.lineHeight()), QTextBlockFormat::FixedHeight);
            QTextCursor(b).mergeBlockFormat(nf);
        }
        any = any || here;
    }
    return any;
}

} // namespace

ShrinkLook shrinkLook(WriteDoc &doc)
{
    ShrinkLook look;
    for (const ContentControl &c : contentControls(doc))
        if (c.props.lockContents) look.locked = true;
    for (int i = 1; i < doc.sectionCount(); ++i) {
        const QString type = doc.sectionProps(i).type;
        if (type != QLatin1String("continuous") && type != QLatin1String("nextColumn")) ++look.forcedBreaks;
    }
    double smallest = 0;
    const QVector<QTextDocument *> stories = shrinkStories(doc);
    for (QTextDocument *story : stories)
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            if (leftAlone(b)) continue;
            if (story == doc.story()) {
                if (b.blockFormat().pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore) ++look.forcedBreaks;
                look.forcedBreaks += int(b.text().count(flow::kPageBreak));
            }
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const double size = effectiveSize(doc, b, it.fragment().charFormat());
                if (size > kShrinkLeastSize && (smallest <= 0 || size < smallest)) smallest = size;
            }
        }
    // (the steps until the smallest text over the least size has reached it: the text is never made smaller than that)
    for (double size = smallest; size > kShrinkLeastSize && look.steps < 100; size = shrunkSize(size)) ++look.steps;
    return look;
}

bool shrinkStep(WriteDoc &doc)
{
    // (the steps are not tracked changes, and a style that follows what is done to its paragraphs is not redefined by them)
    const UntrackedEdit untracked;
    const AutoRedefineOff keepStyles;
    bool any = false;
    for (QTextDocument *story : shrinkStories(doc)) any = shrinkStory(doc, story) || any;
    return any;
}

void forgetShrinkRedo(WriteDoc &doc)
{
    for (QTextDocument *story : shrinkStories(doc)) story->clearUndoRedoStacks(QTextDocument::RedoStack);
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
            double size;
        };
        QVector<Run> runs;
        QVector<std::pair<int, double>> spacings;   // a paragraph's position, its new exact line height
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            bool here = false;
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const QTextFragment fragment = it.fragment();
                const double size = effectiveSize(doc, b, fragment.charFormat());
                double now = size;
                for (int i = 0; i < steps; ++i) now = shrunkSize(now);
                if (size <= 0 || now == fragment.charFormat().fontPointSize()) continue;
                runs.push_back({fragment.position(), fragment.position() + fragment.length(), now});
                here = true;
            }
            const QTextBlockFormat bf = b.blockFormat();
            if (here && bf.lineHeightType() == QTextBlockFormat::FixedHeight && bf.lineHeight() > 0) {
                double height = bf.lineHeight();
                for (int i = 0; i < steps; ++i) height = scaledSpacing(height);
                spacings.push_back({b.position(), height});
            }
        }
        for (const Run &r : std::as_const(runs)) {
            QTextCursor t(story.get());
            t.setPosition(r.from);
            t.setPosition(r.to, QTextCursor::KeepAnchor);
            QTextCharFormat nf;
            nf.setFontPointSize(r.size);
            t.mergeCharFormat(nf);
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
