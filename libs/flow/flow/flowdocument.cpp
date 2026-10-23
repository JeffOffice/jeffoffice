#include "flow/flowdocument.h"

#include <QAbstractTextDocumentLayout>
#include <QFont>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <algorithm>

namespace jp::flow {

namespace {

// Qt's text documents say what an edit changed (QTextDocument::contentsChange) only if they have a layout; the page layout of a document
// that is never shown would lay all of it out. This one does nothing, so that the story tells the paginator what was edited, at no cost.
class NoLayout : public QAbstractTextDocumentLayout {
public:
    explicit NoLayout(QTextDocument *doc) : QAbstractTextDocumentLayout(doc) {}
    void draw(QPainter *, const PaintContext &) override {}
    int hitTest(const QPointF &, Qt::HitTestAccuracy) const override { return -1; }
    int pageCount() const override { return 1; }
    QSizeF documentSize() const override { return QSizeF(); }
    QRectF frameBoundingRect(QTextFrame *) const override { return QRectF(); }
    QRectF blockBoundingRect(const QTextBlock &) const override { return QRectF(); }

protected:
    void documentChanged(int, int, int) override {}
};

} // namespace

static const QTextDocument *pickSlot(const HeaderFooter &hf, bool firstPage, bool evenPage)
{
    if (firstPage) return hf.first.get();
    return evenPage ? hf.even.get() : hf.odd.get();
}

int Section::slotFor(bool firstOfSection, int pageNumber) const
{
    if (differentFirstPage && firstOfSection) return 1;
    return differentOddEven && pageNumber % 2 == 0 ? 2 : 0;
}

const QTextDocument *Section::headerFor(bool firstOfSection, int pageNumber) const
{
    return pickSlot(header, differentFirstPage && firstOfSection, differentOddEven && pageNumber % 2 == 0);
}

const QTextDocument *Section::footerFor(bool firstOfSection, int pageNumber) const
{
    return pickSlot(footer, differentFirstPage && firstOfSection, differentOddEven && pageNumber % 2 == 0);
}

int Section::columnCount() const { return std::clamp(columns, 1, kMostColumns); }

QSizeF Section::pageSize() const
{
    return orientation == Orientation::Landscape ? QSizeF(paper.height(), paper.width()) : paper;
}

QRectF Section::content() const
{
    const QSizeF page = pageSize();
    return QRectF(margins.left(), margins.top(), std::max(36.0, page.width() - margins.left() - margins.right()),
                  std::max(36.0, page.height() - margins.top() - margins.bottom()));
}

double Section::columnWidth() const
{
    const int n = columnCount();
    return std::max(1.0, (content().width() - columnGap * (n - 1)) / n);
}

QRectF Section::columnRect(int column) const
{
    const QRectF all = content();
    const double w = columnWidth();
    return QRectF(all.left() + column * (w + columnGap), all.top(), w, all.height());
}

void watchEdits(QTextDocument *story) { story->setDocumentLayout(new NoLayout(story)); }

FlowDocument::FlowDocument() : m_story(std::make_unique<QTextDocument>())
{
    sections << Section();
    // The colors and fonts a new JeffPub publication has.
    env.colors = builtinColorSchemes().front();
    env.fonts = builtinFontSchemes().front();
    // As JeffPub's stories: no margin of the document's own, and the body font.
    m_story->setDocumentMargin(0);
    QFont body(env.fonts.body);
    body.setPointSizeF(11);
    m_story->setDefaultFont(body);
    m_story->setUndoRedoEnabled(false);
    m_story->setDocumentLayout(new NoLayout(m_story.get()));
}

FlowDocument::~FlowDocument() = default;

const Section &FlowDocument::section(int index) const
{
    static const Section none;   // (a document whose sections were all taken out is laid out as if it had one)
    return sections.isEmpty() ? none : sections[std::clamp(index, 0, int(sections.size()) - 1)];
}

std::unique_ptr<QTextDocument> cloneStory(const QTextDocument *story)
{
    std::unique_ptr<QTextDocument> copy(story->clone());
    const QTextBlock first = story->begin();
    if (first.isValid() && first.length() <= 1) {
        QTextCursor c(copy.get());
        c.setPosition(0);
        c.setBlockFormat(first.blockFormat());
        c.setBlockCharFormat(first.charFormat());
    }
    return copy;
}

} // namespace jp::flow
