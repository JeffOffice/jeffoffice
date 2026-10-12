#include "app/outlineview.h"

#include "app/editor.h"
#include "app/icons.h"
#include "app/theme.h"
#include "core/deck.h"
#include "text/textprops.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QKeyEvent>
#include <QMimeData>
#include <QPainter>
#include <QScrollBar>
#include <QSet>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextLayout>
#include <QTextList>

namespace jp {

namespace {
// What each line of the outline says in its block format: that it is a
// slide's title or a line of its body, and for a title the slide (its page
// id). A body line's level is tp::ListLevel, as in the slide's own text.
constexpr int kKind = QTextFormat::UserProperty + 200;
constexpr int kSlide = QTextFormat::UserProperty + 201;
enum { Title = 1, Body = 2 };

constexpr int kMaxLevel = 8;
constexpr int kGutter = 52;         // room for a title's number and slide icon at the left
constexpr int kLevelStep = 24;      // a body line is indented this much for each level, and once under its title
constexpr double kLevelPoints = 36; // in a slide's text, a level is this far in

bool isTitle(const QTextBlock &b) { return b.blockFormat().intProperty(kKind) == Title; }
int outlineLevel(const QTextBlock &b) { return std::clamp(b.blockFormat().intProperty(tp::ListLevel), 0, kMaxLevel); }

// A level of a paragraph of a slide's text.
int storyLevel(const QTextBlock &b)
{
    const QTextBlockFormat bf = b.blockFormat();
    int level = bf.intProperty(tp::ListLevel);
    if (!bf.hasProperty(tp::ListLevel))
        if (const QTextList *list = b.textList()) level = list->format().intProperty(tp::ListLevel);
    return std::clamp(level, 0, kMaxLevel);
}

TextItem *placeholderIn(Page &page, std::initializer_list<const char *> types)
{
    for (const auto &it : page.items) {
        if (it->type() != ItemType::Text) continue;
        const QString type = deck::placeholderType(*it);
        for (const char *t : types)
            if (type == QLatin1String(t)) return static_cast<TextItem *>(it.get());
    }
    return nullptr;
}
TextItem *titleOf(Page &page) { return placeholderIn(page, {"title", "ctrTitle"}); }
TextItem *bodyOf(Page &page) { return placeholderIn(page, {"body", "obj", "subTitle"}); }

// A title story's paragraphs as one line (they join with line breaks).
QString storyLine(const QTextDocument *story)
{
    QStringList lines;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) lines << b.text();
    return lines.join(QChar(QChar::LineSeparator));
}

// Gives a slide the placeholders the outline needs: if it has none for its
// title or its body, the layout that has them (Title Only, else Title and Content).
void ensurePlaceholders(Document &d, const std::shared_ptr<Page> &page, bool needTitle, bool needBody)
{
    const bool hasTitle = titleOf(*page), hasBody = bodyOf(*page);
    if ((!needTitle || hasTitle) && (!needBody || hasBody)) return;
    const QString type = needBody || hasBody ? QStringLiteral("obj") : QStringLiteral("titleOnly");
    const QString master = deck::slideMasterOf(d, int(d.pages.indexOf(page)));
    const MasterPage *layout = deck::layoutOfType(d, type, master);
    if (!layout) layout = deck::layoutOfType(d, type);
    if (layout) deck::applyLayout(d, *page, *layout);
}

QTextDocument *storyOf(Document &d, TextItem &t)
{
    if (!d.storyDoc(t.storyId)) t.storyId = d.createStory();
    return d.storyDoc(t.storyId);
}

// Paragraph `number` of the story reads `text`, changing only the letters that
// differ so the rest keeps its formatting; the letters put in look like those
// beside them.
void replaceText(QTextDocument *story, int number, const QString &text)
{
    const QTextBlock b = story->findBlockByNumber(number);
    const QString old = b.text();
    const int most = int(std::min(old.size(), text.size()));
    int pre = 0, suf = 0;
    while (pre < most && old[pre] == text[pre]) ++pre;
    while (suf < most - pre && old[old.size() - 1 - suf] == text[text.size() - 1 - suf]) ++suf;
    const QString middle = text.mid(pre, int(text.size()) - pre - suf);
    QTextCursor c(story);
    c.setPosition(b.position() + pre);
    c.setPosition(b.position() + int(old.size()) - suf, QTextCursor::KeepAnchor);
    if (middle.isEmpty()) {
        c.removeSelectedText();
        return;
    }
    QTextCharFormat look = b.charFormat();
    if (!old.isEmpty()) {
        QTextCursor probe(story);
        probe.setPosition(b.position() + (pre > 0 ? pre : 1));   // the letter before, else the first
        look = probe.charFormat();
    }
    // Typing after a field or a link doesn't make more of it.
    look.clearProperty(tp::Field);
    look.clearProperty(tp::InlineObject);
    look.setAnchor(false);
    look.setAnchorHref(QString());
    c.insertText(middle, look);
}

// Paragraph `number` at `level`, its left margin moving with it.
void setLevel(QTextDocument *story, int number, int level)
{
    const QTextBlock b = story->findBlockByNumber(number);
    const int old = storyLevel(b);
    if (old == level) return;
    QTextBlockFormat bf = b.blockFormat();
    bf.setLeftMargin(std::max(0.0, bf.leftMargin() + (level - old) * kLevelPoints));
    if (level) bf.setProperty(tp::ListLevel, level);
    else bf.clearProperty(tp::ListLevel);
    QTextCursor(b).setBlockFormat(bf);
}

// A new paragraph after paragraph `after` (before the first for -1), formatted like its neighbor.
void insertParagraph(QTextDocument *story, int after, const QString &text, int level)
{
    if (after < 0) {
        QTextCursor c(story->begin());
        const QTextBlockFormat bf = c.blockFormat();
        const QTextCharFormat cf = c.blockCharFormat();
        c.insertBlock(bf, cf);
        c.movePosition(QTextCursor::PreviousBlock);
        c.insertText(text, cf);
        setLevel(story, 0, level);
        return;
    }
    const QTextBlock prev = story->findBlockByNumber(after);
    QTextCursor c(prev);
    c.movePosition(QTextCursor::EndOfBlock);
    const QTextBlockFormat bf = prev.blockFormat();
    QTextCharFormat cf = c.charFormat();
    c.insertBlock(bf, cf);
    cf.clearProperty(tp::Field);
    cf.clearProperty(tp::InlineObject);
    cf.setAnchor(false);
    cf.setAnchorHref(QString());
    c.insertText(text, cf);
    setLevel(story, after + 1, level);
}

// Takes paragraphs [from, to) out of the story (never all of them).
void removeParagraphs(QTextDocument *story, int from, int to)
{
    QTextCursor c(story);
    if (from > 0) {
        const QTextBlock first = story->findBlockByNumber(from - 1), last = story->findBlockByNumber(to - 1);
        c.setPosition(first.position() + first.length() - 1);
        c.setPosition(last.position() + last.length() - 1, QTextCursor::KeepAnchor);
    } else {
        c.setPosition(0);
        c.setPosition(story->findBlockByNumber(to).position(), QTextCursor::KeepAnchor);
    }
    c.removeSelectedText();
}

// The story's paragraphs become `texts` (with `levels` for a body) with as
// little changed as possible: the same paragraphs at the start and the end
// stay untouched, the ones between are rewritten, added, or removed.
void setParagraphs(QTextDocument *story, const QStringList &texts, const QList<int> *levels)
{
    QStringList old;
    QList<int> oldLevels;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        old << b.text();
        oldLevels << storyLevel(b);
    }
    auto same = [&](int o, int n) { return old[o] == texts[n] && (!levels || oldLevels[o] == levels->at(n)); };
    const int no = int(old.size()), nn = int(texts.size());
    int pre = 0, suf = 0;
    while (pre < no && pre < nn && same(pre, pre)) ++pre;
    while (suf < no - pre && suf < nn - pre && same(no - 1 - suf, nn - 1 - suf)) ++suf;
    const int midOld = no - pre - suf, midNew = nn - pre - suf;
    for (int k = 0; k < std::min(midOld, midNew); ++k) {
        replaceText(story, pre + k, texts[pre + k]);
        if (levels) setLevel(story, pre + k, levels->at(pre + k));
    }
    if (midNew > midOld) {
        for (int k = midOld; k < midNew; ++k) insertParagraph(story, pre + k - 1, texts[pre + k], levels ? levels->at(pre + k) : 0);
    } else if (midOld > midNew) {
        removeParagraphs(story, pre + midNew, pre + midOld);
    }
}
}

struct OutlineView::Where {
    int slide = 0, line = -1, offset = 0;   // the slide, its title (-1) or body line, and the place in the line
};

// A slide as the outline holds it.
struct OutlineView::Entry {
    struct Line {
        QString text;
        int level = 0;
        bool operator==(const Line &o) const { return text == o.text && level == o.level; }
    };
    QString id;      // the slide's page; empty for a slide that doesn't exist yet
    QString title;
    QVector<Line> body;
    QTextBlock block;   // the title's line in the view
    bool operator==(const Entry &o) const { return id == o.id && title == o.title && body == o.body; }
};

OutlineView::OutlineView(Editor *ed, QWidget *parent) : QTextEdit(parent), m_ed(ed)
{
    setObjectName(QStringLiteral("outlineView"));
    setAccessibleName(tr("Outline"));
    setFrameShape(QFrame::NoFrame);
    setAcceptRichText(false);
    setUndoRedoEnabled(false);   // undo is the presentation's
    auto restyle = [this] {
        setStyleSheet(QStringLiteral("QTextEdit{background:%1; color:%2; border:none; border-right:1px solid %3; selection-background-color:%4; selection-color:white;}")
                          .arg(uiCard().name(), uiText().name(), uiLine().name(), uiAccent().name()));
    };
    restyle();
    connect(UiTheme::instance(), &UiTheme::changed, this, restyle);
    m_idle.setSingleShot(true);
    m_idle.setInterval(700);
    connect(&m_idle, &QTimer::timeout, this, &OutlineView::commit);
    connect(this, &QTextEdit::textChanged, this, &OutlineView::textTyped);
    connect(this, &QTextEdit::cursorPositionChanged, this, &OutlineView::cursorMoved);
    connect(m_ed, &Editor::changed, this, &OutlineView::reload);
    connect(m_ed, &Editor::pageChanged, this, &OutlineView::pageChanged);
    connect(m_ed, &Editor::documentReplaced, this, [this] {
        m_idle.stop();
        m_typed = false;
        qApp->removeEventFilter(this);
        m_dirty = true;
        reload();
    });
}

// ---- the lines, from the slides and back ----

QVector<OutlineView::Entry> OutlineView::fromDeck() const
{
    QVector<Entry> out;
    const Document *d = m_ed->doc();
    if (!d) return out;
    for (const auto &page : d->pages) {
        Entry e;
        e.id = page->id;
        if (const TextItem *t = titleOf(*page))
            if (const QTextDocument *story = d->storyDoc(t->storyId)) e.title = storyLine(story);
        if (const TextItem *t = bodyOf(*page))
            if (const QTextDocument *story = d->storyDoc(t->storyId))
                for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) e.body.append({b.text(), storyLevel(b)});
        // A body of one empty paragraph is no lines.
        if (e.body.size() == 1 && e.body[0].text.isEmpty()) e.body.clear();
        out.append(e);
    }
    return out;
}

// The slides as the view's lines now say them: each title line starts one, and the lines after it are its body.
QVector<OutlineView::Entry> OutlineView::parse() const
{
    QVector<Entry> out;
    QSet<QString> seen;
    for (QTextBlock b = document()->begin(); b.isValid(); b = b.next()) {
        if (isTitle(b)) {
            Entry e;
            e.id = b.blockFormat().stringProperty(kSlide);
            if (seen.contains(e.id)) e.id.clear();   // a title line copied: a slide of its own
            if (!e.id.isEmpty()) seen.insert(e.id);
            e.title = b.text();
            e.block = b;
            out.append(e);
        } else {
            if (out.isEmpty()) out.append(Entry());   // lines before any title belong to a slide that has none
            out.last().body.append({b.text(), outlineLevel(b)});
        }
    }
    for (Entry &e : out)
        if (e.body.size() == 1 && e.body[0].text.isEmpty()) e.body.clear();
    return out;
}

void OutlineView::rebuild(const QVector<Entry> &model)
{
    m_loading = true;
    const int scrolled = verticalScrollBar()->value();
    document()->clear();
    QTextCursor c(document());
    c.beginEditBlock();
    const double base = font().pointSizeF() > 0 ? font().pointSizeF() : 10;
    QTextCharFormat titleChars, bodyChars;
    titleChars.setFontWeight(QFont::Bold);
    titleChars.setFontPointSize(base * 1.15);
    bool first = true;
    auto line = [&](const QTextBlockFormat &bf, const QTextCharFormat &cf, const QString &text) {
        if (first) {
            c.setBlockFormat(bf);
            c.setBlockCharFormat(cf);
            first = false;
        } else {
            c.insertBlock(bf, cf);
        }
        c.insertText(text);
    };
    for (int i = 0; i < model.size(); ++i) {
        QTextBlockFormat title;
        title.setProperty(kKind, int(Title));
        title.setProperty(kSlide, model[i].id);
        title.setLeftMargin(kGutter);
        title.setTopMargin(i ? 12 : 4);
        title.setBottomMargin(2);
        line(title, titleChars, model[i].title);
        for (const Entry::Line &l : model[i].body) {
            QTextBlockFormat body;
            body.setProperty(kKind, int(Body));
            if (l.level) body.setProperty(tp::ListLevel, l.level);
            body.setLeftMargin(kGutter + kLevelStep * (l.level + 1));
            body.setBottomMargin(2);
            line(body, bodyChars, l.text);
        }
    }
    c.endEditBlock();
    // The view stays where it was scrolled to (asking where the last line is has the text laid out).
    document()->documentLayout()->blockBoundingRect(document()->lastBlock());
    verticalScrollBar()->setValue(scrolled);
    m_loading = false;
}

OutlineView::Where OutlineView::whereIs(const QTextCursor &c) const
{
    Where w;
    const QTextBlock b = c.block();
    QTextBlock title = b;
    while (title.isValid() && !isTitle(title)) {
        title = title.previous();
        ++w.line;
    }
    int n = 0;
    for (QTextBlock t = title; t.isValid(); t = t.previous()) n += isTitle(t);
    w.slide = std::max(0, n - 1);
    w.offset = c.position() - b.position();
    return w;
}

void OutlineView::moveTo(const Where &w, bool reveal)
{
    QTextBlock found, last;
    int n = -1;
    for (QTextBlock b = document()->begin(); b.isValid() && !found.isValid(); b = b.next()) {
        if (!isTitle(b)) continue;
        last = b;
        if (++n == w.slide) found = b;
    }
    if (!found.isValid()) found = last;
    if (!found.isValid()) return;
    QTextBlock target = found;
    for (int k = 0; k <= w.line; ++k) {
        const QTextBlock next = target.next();
        if (!next.isValid() || isTitle(next)) break;
        target = next;
    }
    QTextCursor c(target);
    c.setPosition(target.position() + std::clamp(w.offset, 0, std::max(0, target.length() - 1)));
    m_loading = true;
    setTextCursor(c);
    m_loading = false;
    if (reveal) ensureCursorVisible();
}

void OutlineView::reload()
{
    if (m_typed || m_applying || m_loading) return;
    if (!isVisible()) {
        m_dirty = true;
        return;
    }
    m_dirty = false;
    const QVector<Entry> model = fromDeck();
    if (parse() == model) return;
    Where at = whereIs(textCursor());
    rebuild(model);
    // Where the cursor was, unless the current slide is another one by now (then it is shown).
    const bool elsewhere = at.slide != m_ed->currentPage();
    if (elsewhere) at = Where{m_ed->currentPage(), -1, 0};
    moveTo(at, elsewhere);
}

// ---- saving the typing into the slides ----

void OutlineView::textTyped()
{
    if (m_loading) return;
    if (!m_typed) {
        m_typed = true;
        // Any click or command saves what is waiting first.
        qApp->installEventFilter(this);
    }
    m_idle.start();
}

void OutlineView::commit()
{
    m_idle.stop();
    if (!m_typed) return;
    m_typed = false;
    qApp->removeEventFilter(this);
    apply(tr("Outline"));
}

// A change to the lines themselves (a new slide, a line moved a level, lines
// pasted or cut): saved at once, and the lines made again to show it.
void OutlineView::settle(const QString &label)
{
    if (m_applying) return;
    m_idle.stop();
    m_typed = false;
    qApp->removeEventFilter(this);
    const Where at = whereIs(textCursor());
    apply(label);
    rebuild(fromDeck());
    moveTo(at);
}

// Gives the slides the lines now say, as one undo step: slides added for new
// title lines and dropped for title lines gone, in the lines' order, each
// one's title and body written into its placeholders.
void OutlineView::apply(const QString &label)
{
    Document *d = m_ed->doc();
    if (!d) return;
    QVector<Entry> entries = parse();
    if (entries.isEmpty() || entries == fromDeck()) return;
    m_applying = true;
    m_ed->endTextEdit();
    m_ed->beginChange(label);
    const QVector<std::shared_ptr<Page>> old = d->pages;
    QVector<std::shared_ptr<Page>> pages;
    for (const Entry &e : std::as_const(entries)) {
        std::shared_ptr<Page> page;
        if (!e.id.isEmpty())
            for (const auto &p : old)
                if (p->id == e.id) page = p;
        if (!page) {
            // A new slide takes the Title and Content layout of the slide before it.
            const QString master = pages.isEmpty() ? QString() : deck::slideMasterOf(*d, int(d->pages.indexOf(pages.last())));
            const MasterPage *layout = deck::layoutOfType(*d, QStringLiteral("obj"), master);
            if (!layout) layout = deck::layoutOfType(*d, QStringLiteral("obj"));
            if (layout) {
                page = d->pages[deck::addSlide(*d, *layout)];
            } else {
                page = std::make_shared<Page>();
                page->masterId = d->pages.isEmpty() ? QString() : d->pages.first()->masterId;
                page->notesStoryId = d->createStory();
                d->pages.append(page);
            }
        }
        pages << page;
    }
    for (int k = 0; k < entries.size(); ++k) {
        const Entry &e = entries[k];
        ensurePlaceholders(*d, pages[k], !e.title.isEmpty(), !e.body.isEmpty());
        if (TextItem *t = titleOf(*pages[k])) {
            QTextDocument *story = storyOf(*d, *t);
            if (storyLine(story) != e.title) setParagraphs(story, e.title.split(QChar(QChar::LineSeparator)), nullptr);
        }
        if (TextItem *t = bodyOf(*pages[k])) {
            QTextDocument *story = storyOf(*d, *t);
            QStringList texts;
            QList<int> levels;
            for (const Entry::Line &l : e.body) {
                texts << l.text;
                levels << l.level;
            }
            if (texts.isEmpty()) {
                texts << QString();
                levels << 0;
            }
            // The same lines, or no lines for a body that is one empty paragraph.
            QVector<Entry::Line> have;
            for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) have.append({b.text(), storyLevel(b)});
            if (have.size() == 1 && have[0].text.isEmpty()) have.clear();
            if (have != e.body) setParagraphs(story, texts, &levels);
        }
    }
    if (pages != d->pages) d->pages = pages;
    // The slide the cursor is in is the current one.
    int at = 0;
    const int pos = textCursor().block().position();
    for (int k = 0; k < entries.size(); ++k)
        if (entries[k].block.isValid() && entries[k].block.position() <= pos) at = k;
    m_ed->setCurrentPage(at);
    m_ed->endChange();
    m_applying = false;
    // The title lines name their slides now.
    m_loading = true;
    for (int k = 0; k < entries.size(); ++k) {
        if (!entries[k].block.isValid()) continue;
        QTextBlockFormat bf = entries[k].block.blockFormat();
        bf.setProperty(kSlide, pages[k]->id);
        QTextCursor(entries[k].block).setBlockFormat(bf);
    }
    m_loading = false;
}

// ---- the current slide, and the cursor ----

int OutlineView::slideAtCursor() const
{
    QTextBlock title = textCursor().block();
    while (title.isValid() && !isTitle(title)) title = title.previous();
    if (!title.isValid()) return -1;
    const Document *d = m_ed->doc();
    const QString id = title.blockFormat().stringProperty(kSlide);
    const int page = d && !id.isEmpty() ? d->pageIndexOf(id) : -1;
    if (page >= 0) return page;
    int n = -1;   // a slide not made yet: by its place among the titles
    for (QTextBlock t = title; t.isValid(); t = t.previous()) n += isTitle(t);
    return n;
}

void OutlineView::goToSlide(int slide)
{
    const Document *d = m_ed->doc();
    const QString id = d && slide >= 0 && slide < d->pages.size() ? d->pages[slide]->id : QString();
    QTextBlock found;
    int n = -1;
    for (QTextBlock b = document()->begin(); b.isValid() && !found.isValid(); b = b.next()) {
        if (!isTitle(b)) continue;
        ++n;
        if (!id.isEmpty() && b.blockFormat().stringProperty(kSlide) == id) found = b;
    }
    if (!found.isValid()) {
        n = -1;
        for (QTextBlock b = document()->begin(); b.isValid() && !found.isValid(); b = b.next())
            if (isTitle(b) && ++n == slide) found = b;
    }
    if (!found.isValid()) return;
    setTextCursor(QTextCursor(found));
    ensureCursorVisible();
}

void OutlineView::cursorMoved()
{
    if (m_loading || m_applying || m_following) return;
    const int slide = slideAtCursor();
    if (slide < 0 || slide == m_ed->currentPage()) return;
    m_following = true;
    m_ed->setCurrentPage(slide);
    m_following = false;
}

// The current slide was chosen elsewhere: the cursor goes to it.
void OutlineView::pageChanged()
{
    if (m_following || m_loading || m_applying || !isVisible()) return;
    if (slideAtCursor() == m_ed->currentPage()) return;
    m_following = true;
    goToSlide(m_ed->currentPage());
    m_following = false;
}

void OutlineView::showEvent(QShowEvent *e)
{
    QTextEdit::showEvent(e);
    if (m_dirty) reload();
    pageChanged();
}

// ---- keys ----

void OutlineView::keyPressEvent(QKeyEvent *e)
{
    if (e->matches(QKeySequence::Undo) || e->matches(QKeySequence::Redo)) {
        commit();
        if (e->matches(QKeySequence::Undo)) m_ed->undo();
        else m_ed->redo();
        return;
    }
    const bool plain = !(e->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier | Qt::ShiftModifier));
    if (plain && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) && isTitle(textCursor().block())) {
        newSlideAfterTitle();
        return;
    }
    if (!(e->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) {
        if (e->key() == Qt::Key_Backtab || (e->key() == Qt::Key_Tab && (e->modifiers() & Qt::ShiftModifier))) {
            indent(false);
            return;
        }
        if (e->key() == Qt::Key_Tab) {
            indent(true);
            return;
        }
    }
    const int blocks = document()->blockCount();
    QTextEdit::keyPressEvent(e);
    if (document()->blockCount() != blocks && !m_loading) settle(tr("Outline"));
}

void OutlineView::insertFromMimeData(const QMimeData *source)
{
    const int blocks = document()->blockCount();
    QTextEdit::insertFromMimeData(source);
    if (document()->blockCount() != blocks && !m_loading) settle(tr("Paste"));
}

// Enter on a title: a new slide after this one (after its body lines), and
// the text after the cursor goes to its title.
void OutlineView::newSlideAfterTitle()
{
    commit();
    m_loading = true;
    QTextCursor c = textCursor();
    if (c.hasSelection()) c.removeSelectedText();
    const QTextBlock title = c.block();
    const QString tail = title.text().mid(c.position() - title.position());
    QTextCursor cut(title);
    cut.setPosition(c.position());
    cut.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cut.removeSelectedText();
    QTextBlock last = title;
    while (last.next().isValid() && !isTitle(last.next())) last = last.next();
    QTextCursor at(last);
    at.movePosition(QTextCursor::EndOfBlock);
    QTextBlockFormat bf = title.blockFormat();
    bf.setProperty(kSlide, QString());
    bf.setTopMargin(12);
    at.insertBlock(bf, title.charFormat());
    at.insertText(tail);
    at.setPosition(at.block().position());
    setTextCursor(at);
    m_loading = false;
    settle(tr("New Slide"));
}

// Tab and Shift+Tab: a body line goes a level in or out; a first-level line
// out becomes a slide, and a title in becomes a line of the slide before.
// The lines under it come along.
void OutlineView::indent(bool deeper)
{
    commit();
    const QTextBlock b = textCursor().block();
    auto setOutlineLevel = [](const QTextBlock &line, int level) {
        QTextBlockFormat bf = line.blockFormat();
        if (level) bf.setProperty(tp::ListLevel, level);
        else bf.clearProperty(tp::ListLevel);
        bf.setLeftMargin(kGutter + kLevelStep * (level + 1));
        QTextCursor(line).setBlockFormat(bf);
    };
    m_loading = true;
    bool changed = false;
    if (isTitle(b)) {
        if (deeper && b.previous().isValid()) {
            QTextBlockFormat bf = b.blockFormat();
            bf.setProperty(kKind, int(Body));
            bf.clearProperty(kSlide);
            QTextCursor(b).setBlockFormat(bf);
            setOutlineLevel(b, 0);
            for (QTextBlock n = b.next(); n.isValid() && !isTitle(n); n = n.next()) setOutlineLevel(n, std::min(kMaxLevel, outlineLevel(n) + 1));
            changed = true;
        }
    } else {
        const int level = outlineLevel(b);
        if (deeper && level < kMaxLevel) {
            setOutlineLevel(b, level + 1);
            changed = true;
        } else if (!deeper && level > 0) {
            setOutlineLevel(b, level - 1);
            changed = true;
        } else if (!deeper) {
            QTextBlockFormat bf = b.blockFormat();
            bf.setProperty(kKind, int(Title));
            bf.setProperty(kSlide, QString());
            bf.clearProperty(tp::ListLevel);
            QTextCursor(b).setBlockFormat(bf);
            for (QTextBlock n = b.next(); n.isValid() && !isTitle(n); n = n.next())
                if (outlineLevel(n) > 0) setOutlineLevel(n, outlineLevel(n) - 1);
            changed = true;
        }
    }
    m_loading = false;
    if (changed) settle(deeper ? tr("Demote") : tr("Promote"));
}

// ---- events ----

bool OutlineView::event(QEvent *e)
{
    // A command's shortcut may be coming (Ctrl+S, F5): what is typed is saved first.
    if (e->type() == QEvent::ShortcutOverride) {
        const auto *k = static_cast<QKeyEvent *>(e);
        if ((k->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) || (k->key() >= Qt::Key_F1 && k->key() <= Qt::Key_F35)) commit();
    }
    return QTextEdit::event(e);
}

// While typing waits to be saved, a click anywhere or a window closing saves it first.
bool OutlineView::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::Close) commit();
    return QTextEdit::eventFilter(o, e);
}

void OutlineView::focusOutEvent(QFocusEvent *e)
{
    commit();
    QTextEdit::focusOutEvent(e);
}

// The numbers and slide icons in front of the title lines.
void OutlineView::paintEvent(QPaintEvent *e)
{
    QTextEdit::paintEvent(e);
    QPainter p(viewport());
    QAbstractTextDocumentLayout *layout = document()->documentLayout();
    const int current = slideAtCursor();
    QColor text = uiText();
    text.setAlphaF(0.7f);
    int number = 0;
    for (QTextBlock b = document()->begin(); b.isValid(); b = b.next()) {
        if (!isTitle(b)) continue;
        const int shown = ++number;
        const QRectF r = layout->blockBoundingRect(b).translated(-horizontalScrollBar()->value(), -verticalScrollBar()->value());
        if (r.bottom() < 0) continue;
        if (r.top() > viewport()->height()) break;
        const QTextLine line = b.layout()->lineCount() ? b.layout()->lineAt(0) : QTextLine();
        const double top = r.top(), height = line.isValid() ? line.height() : r.height();
        p.setPen(text);
        p.setFont(font());
        p.drawText(QRectF(0, top, 26, height), Qt::AlignRight | Qt::AlignVCenter, QString::number(shown));
        const bool here = shown - 1 == current;
        const QRectF icon(32, top + (height - 12) / 2, 17, 12);
        p.setBrush(uiDark() ? QColor(0x2C, 0x31, 0x3A) : QColor(Qt::white));
        p.setPen(QPen(here ? uiAccent() : text, here ? 1.5 : 1));
        p.drawRect(icon.adjusted(0.5, 0.5, -0.5, -0.5));
        p.setPen(Qt::NoPen);
        p.setBrush(here ? uiAccent() : text);
        p.drawRect(QRectF(icon.left() + 3, icon.top() + 3, 8, 1.5));
        p.drawRect(QRectF(icon.left() + 3, icon.top() + 6.5, 11, 1.5));
    }
}

} // namespace jp
