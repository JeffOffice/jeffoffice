#include "core/proofstories.h"

#include "core/pictureops.h"
#include "core/tableops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QSet>
#include <QTextBlock>
#include <QTextDocument>

namespace jp::write {

QString storyKindName(StoryKind kind)
{
    static const char *const names[] = {"body", "table", "endnote", "header", "footer", "footnote", "textbox"};
    return QLatin1String(names[int(kind)]);
}

namespace {

struct Found {
    StoryKind kind;
    std::shared_ptr<QTextDocument> story;   // (a box's story is made here; the others belong to the document)
};

qint64 wordsIn(const QString &t)
{
    qint64 n = 0;
    bool in = false;
    for (QChar c : t) {
        const bool letter = c.isLetterOrNumber();
        if (letter && !in) ++n;
        in = letter || (in && (c == QLatin1Char('\'') || c == QChar(0x2019) || c == QLatin1Char('-')));
    }
    return n;
}

// The text boxes of `story` (and the boxes in them, down to kMostCheckedBoxDepth), as stories of their own.
// A box the file sets at a place (a floating one) is in the document's list of objects with its own story; one the program keeps as a piece of the text's XML (inline, in a note, or in
// another box) is made into a story here. Both can be the same box, so a box whose words are those of one already found is left out.
void addBoxesOf(WriteDoc &doc, const QTextDocument *story, QVector<Found> &out, QSet<QString> *seen, int *counted, bool *truncated, int depth)
{
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            if (!f.charFormat().hasProperty(prop::Atom) || f.charFormat().hasProperty(prop::TrackedDeleted) || f.charFormat().boolProperty(tp::HiddenText)) continue;
            for (int i = 0; i < int(f.length()); ++i) {
                if (!isDrawnObjectAt(story, f.position() + i)) continue;
                const PictureInfo info = pictureAt(doc, story, f.position() + i);
                if (!info.valid || !info.editable || (info.kind != DrawnKind::Shape && info.kind != DrawnKind::TextBox)) continue;
                if (*counted >= kMostCheckedBoxes) {
                    *truncated = true;
                    return;
                }
                ++*counted;
                std::shared_ptr<QTextDocument> words = textBoxStory(doc, f.position() + i, nullptr, false, story);
                if (!words || words->isEmpty()) continue;
                if (!seen->contains(words->toPlainText())) {
                    seen->insert(words->toPlainText());
                    out.push_back({StoryKind::TextBox, words});
                }
                if (depth < kMostCheckedBoxDepth) addBoxesOf(doc, words.get(), out, seen, counted, truncated, depth + 1);
            }
        }
}

} // namespace

ProofReport checkEveryStory(WriteDoc &doc, quint32 kinds)
{
    ProofReport report;
    QVector<Found> stories;
    auto noShare = [](QTextDocument *) {};
    stories.push_back({StoryKind::Body, std::shared_ptr<QTextDocument>(doc.story(), noShare)});
    for (QTextDocument *side : doc.everySideStory()) {
        int section = 0, slot = 0;
        bool footer = false;
        if (doc.headerFooterSlotOf(side, &section, &footer, &slot)) stories.push_back({footer ? StoryKind::Footer : StoryKind::Header, std::shared_ptr<QTextDocument>(side, noShare)});
        else if (doc.footnoteIdOf(side)) stories.push_back({StoryKind::Footnote, std::shared_ptr<QTextDocument>(side, noShare)});
    }
    for (const int id : doc.citedFootnotes())
        if (QTextDocument *note = doc.editableFootnote(id)) {
            bool known = false;
            for (const Found &f : stories) known = known || f.story.get() == note;
            if (!known) stories.push_back({StoryKind::Footnote, std::shared_ptr<QTextDocument>(note, noShare)});
        }
    int boxes = 0;
    QSet<QString> seen;
    for (const flow::FloatingObject &o : std::as_const(doc.flow().objects)) {
        if (!o.story || o.story->isEmpty()) continue;
        if (boxes >= kMostCheckedBoxes) {
            report.truncated = true;
            break;
        }
        ++boxes;
        seen.insert(o.story->toPlainText());
        stories.push_back({StoryKind::TextBox, std::shared_ptr<QTextDocument>(o.story.get(), noShare)});
    }
    const int before = int(stories.size());
    for (int i = 0; i < before; ++i) addBoxesOf(doc, stories[i].story.get(), stories, &seen, &boxes, &report.truncated, 0);
    const QString language = doc.proofingLanguage();
    for (const Found &f : std::as_const(stories)) {
        QTextDocument *story = f.story.get();
        GrammarCheck check(story);
        check.setDefaultLanguage(language);
        check.setEnabled(true);
        auto kindOf = [&](const QTextBlock &b) {
            if (f.kind != StoryKind::Body) return f.kind;
            return inTable(b) ? StoryKind::TableCell : doc.endnoteIdOf(b) ? StoryKind::Endnote : StoryKind::Body;
        };
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            if (b.length() <= 1) continue;
            report.words[kindOf(b)] += wordsIn(b.text());
            ++report.paragraphs[kindOf(b)];
        }
        int from = 0, next = 0;
        GrammarCheck::Issue found;
        for (;;) {
            if (check.search(from, -1, &found, &next) != GrammarCheck::Step::Found) break;
            ProofIssue p;
            const QTextBlock at = story->findBlock(found.start);
            p.kind = kindOf(at);
            p.issue = found;
            const QString t = at.text();
            const int s = found.start - at.position(), e = std::min(int(t.size()), found.end - at.position()), lo = std::max(0, s - 50);
            p.text = t.mid(s, e - s);
            p.context = t.mid(lo, s - lo) + QStringLiteral("[[") + p.text + QStringLiteral("]]") + t.mid(e, 50);
            report.issues << p;
            from = std::max(found.end, found.start + 1);
        }
        if (check.budgetGone()) report.truncated = true;
    }
    return report;
}

} // namespace jp::write
