#include "app/notesdisplay.h"

#include <QTextBlock>
#include <QTextBoundaryFinder>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <algorithm>

namespace jp {

std::unique_ptr<QTextDocument> notesDisplayCopy(const QTextDocument *story, int limit, double points, bool *cut)
{
    auto out = std::make_unique<QTextDocument>();
    if (cut) *cut = false;
    if (!story) return out;
    QTextDocument *from = const_cast<QTextDocument *>(story);   // (only read: a cursor needs a document that is not const)
    int stop = std::max(0, from->characterCount() - 1);
    if (limit >= 0 && stop > limit) {
        QTextCursor head(from);
        head.setPosition(0);
        head.setPosition(std::min(stop, limit + 1024), QTextCursor::KeepAnchor);
        const QString text = head.selectedText();
        QTextBoundaryFinder finder(QTextBoundaryFinder::Grapheme, text);
        finder.setPosition(limit);
        const int at = finder.isAtBoundary() ? limit : finder.toPreviousBoundary();
        stop = std::max(at, 0);
        if (cut) *cut = true;
    }
    QTextCursor src(from);
    src.setPosition(0);
    src.setPosition(stop, QTextCursor::KeepAnchor);
    QTextCursor dst(out.get());
    dst.insertFragment(src.selection());
    // Sizes: the story's own default is what `points` is for; the sizes its runs name go in the same proportion.
    QFont base = story->defaultFont();
    const double was = base.pointSizeF() > 0 ? base.pointSizeF() : 11.0;
    const double k = points > 0 ? points / was : 1.0;
    base.setPointSizeF(points > 0 ? points : was);
    out->setDefaultFont(base);
    out->setDocumentMargin(0);
    if (!qFuzzyCompare(k, 1.0)) {
        QTextCursor c(out.get());
        c.beginEditBlock();
        for (QTextBlock b = out->begin(); b.isValid(); b = b.next()) {
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const QTextFragment f = it.fragment();
                if (!f.charFormat().hasProperty(QTextFormat::FontPointSize)) continue;
                QTextCharFormat sized;
                sized.setFontPointSize(f.charFormat().fontPointSize() * k);
                c.setPosition(f.position());
                c.setPosition(f.position() + f.length(), QTextCursor::KeepAnchor);
                c.mergeCharFormat(sized);
            }
        }
        c.endEditBlock();
    }
    return out;
}

} // namespace jp
