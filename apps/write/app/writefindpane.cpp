#include "app/writefindpane.h"

#include "app/pageview.h"
#include "app/writewindow.h"
#include "core/editops.h"
#include "core/findops.h"
#include "core/styleops.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QAction>
#include <QCheckBox>
#include <QFontDatabase>
#include <QInputDialog>
#include <QMenu>
#include <QToolButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPair>
#include <QPointer>
#include <QPushButton>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextFragment>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>

namespace jp {

// The story's words as a search sees them: its characters without the pieces kept as XML that show nothing (a bookmark between two words does not part them),
// as a document of their own, and where each run of them is in the story. A story with something in it that a document of plain text would not keep as it is
// (so that places could not be told apart) is searched as it is.
class StoryWords : public QObject {
public:
    explicit StoryWords(QTextDocument *story) : m_story(story)
    {
        rebuild();
        // (The words are made again, the next time they are asked for, after the story changes.)
        connect(story, &QTextDocument::contentsChanged, this, [this] { dirty = true; });
    }
    void rebuild()
    {
        dirty = false;
        m_plainValid = false;
        m_runs.clear();
        QString text;
        auto extend = [&](int at, const QString &piece) {
            if (!m_runs.isEmpty() && m_runs.last().story + m_runs.last().length == at) m_runs.last().length += int(piece.size());
            else m_runs.push_back({int(text.size()), at, int(piece.size())});
            text += piece;
        };
        // (The endnotes the pages set are not searched: they are paragraphs marked as theirs, the last ones, or those at the end of a section; the paragraphs round them are the text.)
        auto note = [](const QTextBlock &b) { return b.blockFormat().hasProperty(write::prop::EndnoteBlock); };
        for (QTextBlock b = m_story->begin(); b.isValid(); b = b.next()) {
            if (note(b)) continue;
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it)
                if (!write::isInvisibleAtom(it.fragment().charFormat()) && !it.fragment().charFormat().hasProperty(write::prop::TrackedDeleted)) extend(it.fragment().position(), it.fragment().text());   // (what a change deleted is no part of the words)
            QTextBlock after = b.next();
            while (after.isValid() && note(after)) after = after.next();
            if (after.isValid()) extend(b.position() + b.length() - 1, QStringLiteral("\n"));
        }
        m_words.setUndoRedoEnabled(false);
        m_words.setPlainText(text);
        m_identity = m_words.characterCount() != text.size() + 1;
        if (m_identity) m_runs.clear();
    }
    QTextDocument *story() const { return m_story; }
    QTextDocument *words() { return m_identity ? m_story.data() : &m_words; }
    // The words as one string (a place in it is a place in the words).
    const QString &plain()
    {
        if (!m_plainValid) {
            m_plain = words()->toPlainText();
            m_plainValid = true;
        }
        return m_plain;
    }

    // A place in the story as a place in the words: the first of them at or after it.
    int toWords(int at) const
    {
        if (m_identity || m_runs.isEmpty()) return at;
        const Run &r = runAt(at, false);
        return r.words + std::clamp(at - r.story, 0, r.length);
    }
    // The place of the character that the words have at a place (the place before it), and the place after the character before it (the end of a match).
    int toStory(int at) const
    {
        if (m_identity || m_runs.isEmpty()) return at;
        const Run &r = runAt(at, true);
        return r.story + std::min(at - r.words, r.length);
    }
    int toStoryEnd(int at) const { return at > 0 ? toStory(at - 1) + 1 : toStory(at); }
    // The places of the story that the words [from, to) are: runs of characters, in order, with the pieces kept as XML between them left out.
    QVector<QPair<int, int>> pieces(int from, int to) const
    {
        if (m_identity || m_runs.isEmpty()) return {{from, to}};
        QVector<QPair<int, int>> out;
        for (const Run &r : m_runs) {
            const int a = std::max(from, r.words), b = std::min(to, r.words + r.length);
            if (a < b) out.push_back({r.story + a - r.words, r.story + b - r.words});
        }
        return out;
    }
    bool dirty = false;

private:
    struct Run {
        int words = 0, story = 0, length = 0;
    };
    // The run that holds the place (in the words, or the story): the last that starts at or before it.
    const Run &runAt(int at, bool inWords) const
    {
        int lo = 0, hi = int(m_runs.size()) - 1;
        while (lo < hi) {
            const int mid = (lo + hi + 1) / 2;
            if ((inWords ? m_runs[mid].words : m_runs[mid].story) <= at) lo = mid;
            else hi = mid - 1;
        }
        return m_runs[lo];
    }
    QPointer<QTextDocument> m_story;
    QTextDocument m_words;
    QVector<Run> m_runs;
    bool m_identity = false;
    QString m_plain;
    bool m_plainValid = false;
};

WriteFindPane::WriteFindPane(WriteWindow *win, QWidget *parent) : QWidget(parent), m_win(win)
{
    setObjectName(QStringLiteral("findPane"));
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(6, 6, 6, 6);
    // Each label names its box for screen readers (its buddy).
    auto *findLabel = new QLabel(tr("Find what:"), this);
    v->addWidget(findLabel);
    m_find = new QLineEdit(this);
    m_find->setObjectName(QStringLiteral("findWhat"));
    findLabel->setBuddy(m_find);
    v->addWidget(m_find);
    auto *replaceLabel = new QLabel(tr("Replace with:"), this);
    v->addWidget(replaceLabel);
    m_replace = new QLineEdit(this);
    m_replace->setObjectName(QStringLiteral("replaceWith"));
    replaceLabel->setBuddy(m_replace);
    v->addWidget(m_replace);
    m_case = new QCheckBox(tr("Match case"), this);
    m_whole = new QCheckBox(tr("Find whole words only"), this);
    v->addWidget(m_case);
    v->addWidget(m_whole);
    m_wild = new QCheckBox(tr("Use wildcards"), this);
    m_wild->setObjectName(QStringLiteral("useWildcards"));
    m_wild->setToolTip(tr("? is any letter, * is any run of letters, < and > are the start and end of a word, [a-c] is a range, @ is one or more, and ( ) make groups to use as \\1 in Replace with."));
    v->addWidget(m_wild);
    m_sounds = new QCheckBox(tr("Sounds like"), this);
    m_sounds->setObjectName(QStringLiteral("soundsLike"));
    m_sounds->setToolTip(tr("Finds words that are said like the one in Find what, such as Smith and Smyth."));
    m_forms = new QCheckBox(tr("Find all word forms"), this);
    m_forms->setObjectName(QStringLiteral("wordForms"));
    m_forms->setToolTip(tr("Finds every form of the word in Find what: run, runs, running, and ran."));
    m_ignorePunct = new QCheckBox(tr("Ignore punctuation characters"), this);
    m_ignorePunct->setObjectName(QStringLiteral("ignorePunctuation"));
    m_ignoreSpace = new QCheckBox(tr("Ignore white-space characters"), this);
    m_ignoreSpace->setObjectName(QStringLiteral("ignoreSpaces"));
    for (QCheckBox *c : {m_sounds, m_forms, m_ignorePunct, m_ignoreSpace}) v->addWidget(c);
    auto *more = new QHBoxLayout();
    m_special = new QToolButton(this);
    m_special->setObjectName(QStringLiteral("findSpecial"));
    m_special->setText(tr("Special"));
    m_special->setPopupMode(QToolButton::InstantPopup);
    m_special->setToolTip(tr("Puts a code in Find what for a paragraph mark, a tab, a digit, a letter, or white space."));
    auto *specials = new QMenu(m_special);
    struct Code { const char *name, *code; };
    static const Code codes[] = {{QT_TR_NOOP("Paragraph Mark"), "^p"}, {QT_TR_NOOP("Tab Character"), "^t"}, {QT_TR_NOOP("Any Digit"), "^#"}, {QT_TR_NOOP("Any Letter"), "^$"},
                                 {QT_TR_NOOP("White Space"), "^w"}, {QT_TR_NOOP("Caret Character"), "^^"}};
    for (const Code &c : codes) {
        QAction *a = specials->addAction(tr(c.name));
        a->setData(QString::fromLatin1(c.code));
        connect(a, &QAction::triggered, this, [this, a] {
            m_find->insert(a->data().toString());
            m_find->setFocus(Qt::OtherFocusReason);
        });
    }
    m_special->setMenu(specials);
    m_formatButton = new QToolButton(this);
    m_formatButton->setObjectName(QStringLiteral("findFormat"));
    m_formatButton->setText(tr("Format"));
    m_formatButton->setPopupMode(QToolButton::InstantPopup);
    m_formatButton->setToolTip(tr("Finds only text that has the look you choose."));
    auto *formats = new QMenu(m_formatButton);
    struct Look { const char *name; int write::FormatFilter::*field; };
    static const Look looks[] = {{QT_TR_NOOP("Bold"), &write::FormatFilter::bold}, {QT_TR_NOOP("Italic"), &write::FormatFilter::italic}, {QT_TR_NOOP("Underline"), &write::FormatFilter::underline},
                                 {QT_TR_NOOP("Strikethrough"), &write::FormatFilter::strike}, {QT_TR_NOOP("Highlighted"), &write::FormatFilter::highlighted}};
    for (const Look &l : looks) {
        QAction *a = formats->addAction(tr(l.name));
        a->setCheckable(true);
        connect(a, &QAction::toggled, this, [this, l](bool on) {
            write::FormatFilter f = m_format;
            f.*l.field = on ? 1 : -1;
            setFormatFilter(f);
        });
    }
    formats->addSeparator();
    connect(formats->addAction(tr("Font...")), &QAction::triggered, this, [this] {
        bool ok = false;
        const QString family = QInputDialog::getItem(this, tr("Find Font"), tr("Font:"), QFontDatabase::families(), 0, false, &ok);
        if (!ok) return;
        write::FormatFilter f = m_format;
        f.font = family;
        setFormatFilter(f);
    });
    connect(formats->addAction(tr("Style...")), &QAction::triggered, this, [this] {
        if (!m_win->document()) return;
        QStringList names;
        QHash<QString, QString> ids;
        for (const write::ListedStyle &st : write::listStyles(*m_win->document(), write::StyleFilter::All))
            if (!st.character) {
                names << st.name;
                ids.insert(st.name, st.id);
            }
        if (names.isEmpty()) return;
        bool ok = false;
        const QString name = QInputDialog::getItem(this, tr("Find Style"), tr("Paragraph style:"), names, 0, false, &ok);
        if (!ok) return;
        write::FormatFilter f = m_format;
        f.style = ids.value(name);
        setFormatFilter(f);
    });
    connect(formats->addAction(tr("No Formatting")), &QAction::triggered, this, [this, formats] {
        for (QAction *a : formats->actions()) a->setChecked(false);
        setFormatFilter(write::FormatFilter());
    });
    m_formatButton->setMenu(formats);
    more->addWidget(m_special);
    more->addWidget(m_formatButton);
    more->addStretch(1);
    v->addLayout(more);
    m_formatLabel = new QLabel(this);
    m_formatLabel->setObjectName(QStringLiteral("findFormatLabel"));
    m_formatLabel->setTextFormat(Qt::PlainText);
    m_formatLabel->setWordWrap(true);
    m_formatLabel->hide();
    v->addWidget(m_formatLabel);
    auto *row = new QHBoxLayout();
    auto *next = new QPushButton(tr("Find Next"), this);
    auto *rep = new QPushButton(tr("Replace"), this);
    auto *all = new QPushButton(tr("Replace All"), this);
    row->addWidget(next);
    row->addWidget(rep);
    row->addWidget(all);
    v->addLayout(row);
    m_status = new QLabel(this);
    m_status->setObjectName(QStringLiteral("findStatus"));
    m_status->setTextFormat(Qt::PlainText);   // it says what was searched for, which may look like markup
    m_status->setWordWrap(true);
    v->addWidget(m_status);
    v->addStretch(1);
    connect(next, &QPushButton::clicked, this, [this] { findNext(); });
    connect(m_find, &QLineEdit::returnPressed, this, [this] { findNext(); });
    connect(rep, &QPushButton::clicked, this, [this] { replaceOne(); });
    connect(all, &QPushButton::clicked, this, [this] { replaceAll(); });
    setMinimumWidth(240);
    // The count and the yellow bands follow the box, the choices, and the text (a moment after the last change).
    m_refresh = new QTimer(this);
    m_refresh->setSingleShot(true);
    m_refresh->setInterval(30);
    connect(m_refresh, &QTimer::timeout, this, [this] { refreshMatches(); });
    connect(m_find, &QLineEdit::textChanged, m_refresh, qOverload<>(&QTimer::start));
    connect(m_case, &QCheckBox::toggled, m_refresh, qOverload<>(&QTimer::start));
    connect(m_whole, &QCheckBox::toggled, m_refresh, qOverload<>(&QTimer::start));
    connect(m_wild, &QCheckBox::toggled, m_refresh, qOverload<>(&QTimer::start));
    for (QCheckBox *c : {m_sounds, m_forms, m_ignorePunct, m_ignoreSpace}) connect(c, &QCheckBox::toggled, m_refresh, qOverload<>(&QTimer::start));
    for (QCheckBox *c : {m_wild, m_sounds, m_forms}) connect(c, &QCheckBox::toggled, this, [this] { syncChoices(); });
    // The pages ask for the words found in a header, a footer, or a note when they draw it.
    m_win->view()->setFindSideMatcher([this](const QTextDocument *story) { return matchesInSideStory(story); });
}

// Some choices do not go with others (a pattern has its own way of saying these things; a word read by its sound or its forms is a whole word, in any case).
void WriteFindPane::syncChoices()
{
    const bool wild = m_wild->isChecked(), words = m_sounds->isChecked() || m_forms->isChecked();
    m_whole->setDisabled(wild || words);
    m_sounds->setDisabled(wild || m_forms->isChecked());
    m_forms->setDisabled(wild || m_sounds->isChecked());
    m_wild->setDisabled(words);
    m_ignorePunct->setDisabled(wild || words);
    m_ignoreSpace->setDisabled(wild || words);
    m_case->setDisabled(words);
}

void WriteFindPane::setFormatFilter(const write::FormatFilter &filter)
{
    m_format = filter;
    updateFormatLabel();
    m_refresh->start();
}

void WriteFindPane::updateFormatLabel()
{
    QStringList parts;
    auto name = [&](int v, const QString &what) {
        if (v >= 0) parts << (v ? what : tr("Not %1").arg(what.toLower()));
    };
    name(m_format.bold, tr("Bold"));
    name(m_format.italic, tr("Italic"));
    name(m_format.underline, tr("Underline"));
    name(m_format.strike, tr("Strikethrough"));
    name(m_format.highlighted, tr("Highlighted"));
    if (!m_format.font.isEmpty()) parts << tr("Font: %1").arg(m_format.font);
    if (!m_format.style.isEmpty()) parts << tr("Style: %1").arg(m_format.style);
    m_formatLabel->setText(parts.isEmpty() ? QString() : tr("Format: %1").arg(parts.join(QStringLiteral(", "))));
    m_formatLabel->setVisible(!parts.isEmpty());
}

write::AdvancedFind WriteFindPane::advancedOptions() const
{
    write::AdvancedFind o;
    o.query = m_find->text();
    o.matchCase = m_case->isChecked();
    o.wholeWords = m_whole->isChecked();
    o.soundsLike = m_sounds->isChecked();
    o.wordForms = m_forms->isChecked();
    o.ignorePunctuation = m_ignorePunct->isChecked();
    o.ignoreSpaces = m_ignoreSpace->isChecked();
    return o;
}

// Special codes typed in the box (^p, ^t, ^#, ^$, ^w, ^^) work without "Use wildcards": the other characters then are themselves.
bool WriteFindPane::codesTyped() const
{
    if (m_wild->isChecked() || advancedOptions().active()) return false;
    const QString q = m_find->text();
    for (int i = 0; i + 1 < q.size(); ++i)
        if (q[i] == QLatin1Char('^') && QStringLiteral("pt#$w^").contains(q[i + 1])) return true;
    return false;
}

// The pattern the search is made with: the box as typed with wildcards on; with only codes, the box with each character that a pattern reads as more than itself held back.
QString WriteFindPane::patternText() const
{
    const QString q = m_find->text();
    if (m_wild->isChecked()) return q;
    QString out;
    for (int i = 0; i < q.size(); ++i) {
        if (q[i] == QLatin1Char('^') && i + 1 < q.size()) {
            out += q[i];
            out += q[++i];
        } else {
            if (QStringLiteral("?*<>[]{}()@\\").contains(q[i])) out += QLatin1Char('\\');
            out += q[i];
        }
    }
    return out;
}

bool WriteFindPane::customSearch() const { return m_wild->isChecked() || codesTyped() || advancedOptions().active() || m_format.any(); }
bool WriteFindPane::searching() const { return !m_find->text().isEmpty() || m_format.any(); }

QVector<QPair<int, int>> WriteFindPane::matchesInSideStory(const QTextDocument *story)
{
    if (!story || !isVisible() || !searching() || m_find->text().isEmpty()) return {};
    Side &side = m_side[story];
    if (!side.words || side.words->story() != story) {
        side.words = std::make_shared<StoryWords>(const_cast<QTextDocument *>(story));
        side.revision = -1;
    }
    if (side.revision == story->revision() && side.generation == m_generation) return side.ranges;
    side.revision = story->revision();
    side.generation = m_generation;
    side.ranges.clear();
    if (side.words->dirty) side.words->rebuild();
    QString error;
    for (int at = 0; side.ranges.size() < kMostMatches;) {
        const QTextCursor m = matchIn(side.words.get(), at, &error);
        if (m.isNull()) break;
        at = std::max(m.selectionEnd(), at + 1);
        side.ranges.push_back({side.words->toStory(m.selectionStart()), side.words->toStoryEnd(m.selectionEnd())});
    }
    return side.ranges;
}

void WriteFindPane::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);
    m_win->view()->setFindHighlights({});
}

void WriteFindPane::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    m_refresh->start();
}

QTextCursor WriteFindPane::matchFrom(int from, QString *error)
{
    StoryWords *text = storyWords();
    return text ? matchIn(text, from, error) : QTextCursor();
}

// The first match of the search at or after a place of the words, with the look it is asked to have.
QTextCursor WriteFindPane::matchIn(StoryWords *text, int from, QString *error)
{
    if (!m_format.any()) return rawMatchIn(text, from, error);
    QTextDocument *story = text->story();
    auto styleOfBlock = [&](int storyPos) { return story->findBlock(storyPos).blockFormat().stringProperty(tp::StyleName); };
    if (m_find->text().isEmpty()) {
        // Only a look: each stretch of one paragraph's text that has it is a match.
        int runFrom = -1, runTo = -1;
        const int begin = text->toStory(from);
        for (QTextBlock b = story->findBlock(begin); b.isValid(); b = b.next()) {
            runFrom = -1;
            const bool styleOk = m_format.matchesStyle(styleOfBlock(b.position()));
            for (QTextBlock::iterator it = b.begin(); styleOk && !it.atEnd(); ++it) {
                const QTextFragment f = it.fragment();
                if (write::isInvisibleAtom(f.charFormat()) || f.charFormat().hasProperty(write::prop::TrackedDeleted)) continue;
                const int a = std::max(f.position(), begin), e = f.position() + f.length();
                if (a >= e) continue;
                if (m_format.matchesChar(f.charFormat())) {
                    if (runFrom < 0) runFrom = a;
                    runTo = e;
                } else if (runFrom >= 0) {
                    break;
                }
            }
            if (runFrom >= 0) break;
        }
        if (runFrom < 0) return QTextCursor();
        QTextCursor c(text->words());
        c.setPosition(text->toWords(runFrom));
        c.setPosition(text->toWords(runTo), QTextCursor::KeepAnchor);
        return c;
    }
    for (int at = from;;) {
        const QTextCursor m = rawMatchIn(text, at, error);
        if (m.isNull()) return m;
        QTextCursor shown(story);
        shown.setPosition(text->toStory(m.selectionStart()));
        shown.setPosition(text->toStoryEnd(m.selectionEnd()), QTextCursor::KeepAnchor);
        if (m_format.matchesStyle(styleOfBlock(shown.selectionStart())) && write::allSelected(shown, [&](const QTextCharFormat &f) { return m_format.matchesChar(f); })) return m;
        at = std::max(m.selectionEnd(), at + 1);
    }
}

QTextCursor WriteFindPane::rawMatchIn(StoryWords *text, int from, QString *error)
{
    const write::AdvancedFind advanced = advancedOptions();
    if (advanced.active()) {
        const QPair<int, int> r = write::advancedMatch(text->plain(), from, advanced);
        if (r.first < 0) return QTextCursor();
        QTextCursor c(text->words());
        c.setPosition(r.first);
        c.setPosition(r.second, QTextCursor::KeepAnchor);
        return c;
    }
    if (!m_wild->isChecked() && !codesTyped()) return text->words()->find(m_find->text(), from, flags());
    const write::FindPattern pattern = write::wildcardPattern(patternText(), m_case->isChecked());
    if (!pattern.error.isEmpty()) {
        if (error) *error = pattern.error;
        return QTextCursor();
    }
    // (Looked for in the words as one string: a pattern may cross a paragraph mark, which a search paragraph by paragraph could not.)
    const QString &plain = text->plain();
    for (int at = from; at <= plain.size();) {
        const QRegularExpressionMatch rm = pattern.regex.match(plain, at);
        if (!rm.hasMatch()) return QTextCursor();
        if (rm.capturedEnd() > rm.capturedStart()) {
            QTextCursor c(text->words());
            c.setPosition(int(rm.capturedStart()));
            c.setPosition(int(rm.capturedEnd()), QTextCursor::KeepAnchor);
            return c;
        }
        at = int(rm.capturedEnd()) + 1;   // (a match of no characters is none)
    }
    return QTextCursor();
}

// The groups of the pattern for a match of it, found again in the words with what is round it (so that the starts and ends of words are right).
QRegularExpressionMatch WriteFindPane::groupsOf(const QTextCursor &match)
{
    const write::FindPattern pattern = write::wildcardPattern(patternText(), m_case->isChecked());
    StoryWords *text = storyWords();
    if (!text || !pattern.error.isEmpty()) return QRegularExpressionMatch();
    return pattern.regex.match(text->plain(), match.selectionStart());
}

// Finds every place the text of the box is (as findNext sees it), counts them, says so, and shows each on the pages.
void WriteFindPane::refreshMatches()
{
    m_matches.clear();
    ++m_generation;
    QString patternError;
    StoryWords *text = storyWords();
    const QString q = m_find->text();
    if (text && searching() && isVisible()) {
        const FindWords w = words();
        int at = 0;
        while (m_matches.size() < kMostMatches) {
            const QTextCursor m = matchFrom(at, &patternError);
            if (m.isNull()) break;
            at = std::max(m.selectionEnd(), at + 1);
            if (w.skip && w.skip(m)) continue;
            m_matches.push_back({text->toStory(m.selectionStart()), text->toStoryEnd(m.selectionEnd())});
        }
        if (text->story() != m_watched) {
            m_watched = text->story();
            connect(text->story(), &QTextDocument::contentsChanged, m_refresh, qOverload<>(&QTimer::start));
        }
    }
    m_win->view()->setFindHighlights(m_matches);
    m_win->view()->viewport()->update();   // (the words found in headers, footers, and notes follow the box too)
    if (!searching()) m_status->clear();
    else if (!patternError.isEmpty()) m_status->setText(patternError);
    else if (m_matches.isEmpty()) m_status->setText(tr("No matches"));
    else if (m_matches.size() == 1) m_status->setText(tr("1 match"));
    else if (m_matches.size() >= kMostMatches) m_status->setText(tr("%1 or more matches").arg(kMostMatches));
    else m_status->setText(tr("%1 matches").arg(m_matches.size()));
}

WriteFindPane::~WriteFindPane() = default;

void WriteFindPane::focusBox(bool replace)
{
    QLineEdit *box = replace ? m_replace : m_find;
    box->setFocus(Qt::OtherFocusReason);
    box->selectAll();
}

QString WriteFindPane::status() const { return m_status->text(); }

QTextDocument::FindFlags WriteFindPane::flags() const
{
    QTextDocument::FindFlags f;
    if (m_case->isChecked()) f |= QTextDocument::FindCaseSensitively;
    if (m_whole->isChecked()) f |= QTextDocument::FindWholeWords;
    return f;
}

StoryWords *WriteFindPane::storyWords()
{
    QTextDocument *story = m_win->document() ? m_win->document()->story() : nullptr;
    if (!story) return nullptr;
    if (!m_text || m_text->story() != story) m_text = std::make_unique<StoryWords>(story);
    else if (m_text->dirty) m_text->rebuild();
    return m_text.get();
}

// The document's one story, as libs/page's search is given it: its words, and the places of the story that a swap puts new words in.
FindWords WriteFindPane::words()
{
    FindWords w;
    StoryWords *text = storyWords();
    QTextDocument *story = text ? text->story() : nullptr;
    w.story = [text](const FindStory &) { return text ? text->words() : nullptr; };
    // Hidden text is not found (nor replaced): it is not shown.
    w.skip = [text, story](const QTextCursor &match) {
        if (text->toStory(match.selectionStart()) >= write::editableEnd(story)) return true;   // (nor a match in the endnotes the pages set after the text)
        if (write::touchesEndnotes(story, text->toStory(match.selectionStart()), text->toStoryEnd(match.selectionEnd()))) return true;   // (nor one that goes over those at the end of a section)
        QTextCursor shown(story);
        shown.setPosition(text->toStory(match.selectionStart()));
        shown.setPosition(text->toStoryEnd(match.selectionEnd()), QTextCursor::KeepAnchor);
        return !write::allSelected(shown, [](const QTextCharFormat &f) { return !f.boolProperty(tp::HiddenText) || f.hasProperty(write::prop::TrackedDeleted); });   // (what a change deleted is no part of the words, and a match may go round it)
    };
    // The new words take the place of the first of the characters of the match, and the look of the first of them; the characters of it between pieces kept as XML
    // are taken away, and the pieces stay. (The matches come from the last to the first, so that no swap moves another.)
    w.put = [text, story](QTextCursor &match, const QString &with) {
        const QVector<QPair<int, int>> pieces = text->pieces(match.selectionStart(), match.selectionEnd());
        if (pieces.isEmpty()) return;
        QTextCursor first(story);
        first.setPosition(pieces.first().first + 1);
        const QTextCharFormat look = first.charFormat();
        QTextCursor edit(story);
        edit.beginEditBlock();
        for (int i = int(pieces.size()) - 1; i >= 1; --i) {
            edit.setPosition(pieces[i].first);
            edit.setPosition(pieces[i].second, QTextCursor::KeepAnchor);
            edit.removeSelectedText();
        }
        edit.setPosition(pieces.first().first);
        edit.setPosition(pieces.first().second, QTextCursor::KeepAnchor);
        edit.insertText(with, look);
        edit.endEditBlock();
    };
    return w;
}

QVector<FindStory> WriteFindPane::stories() const
{
    QVector<FindStory> all;
    all << FindStory{QString(), QStringLiteral("story")};
    return all;
}

// Whether what is selected is a match, as the search sees it (so that Replace does not change words the search would not have shown).
bool WriteFindPane::selectionIsMatch()
{
    m_win->view()->leaveHeaderFooter();   // (the search is of the text: its caret is the text's)
    m_win->view()->leaveNote();
    const QTextCursor &sel = m_win->view()->cursor();
    StoryWords *text = storyWords();
    if (!text || !sel.hasSelection() || !searching()) return false;
    const int from = text->toWords(sel.selectionStart()), to = text->toWords(sel.selectionEnd());
    const QTextCursor m = matchFrom(from);
    return !m.isNull() && m.selectionStart() == from && m.selectionEnd() == to && !words().skip(m);
}

bool WriteFindPane::findNext()
{
    m_win->view()->leaveHeaderFooter();
    m_win->view()->leaveNote();
    const QString q = m_find->text();
    StoryWords *text = storyWords();
    if (!text || !searching()) return false;
    int story = 0;
    QTextCursor found;
    bool got = false;
    if (customSearch()) {
        // A pattern: from the end of the selection to the end of the text, then again from the start.
        const FindWords w = words();
        const int start = text->toWords(m_win->view()->cursor().selectionEnd());
        QString problem;
        for (int pass = 0; pass < 2 && !got; ++pass)
            for (int at = pass == 0 ? start : 0;;) {
                const QTextCursor m = matchFrom(at, &problem);
                if (m.isNull()) break;
                if (w.skip && w.skip(m)) {
                    at = m.selectionEnd();
                    continue;
                }
                found = m;
                got = true;
                break;
            }
        if (!problem.isEmpty()) {
            m_status->setText(problem);
            return false;
        }
    } else {
        got = findInStories(words(), stories(), q, flags(), 0, text->toWords(m_win->view()->cursor().selectionEnd()), &story, &found);
    }
    if (got) {
        QTextCursor shown(text->story());
        shown.setPosition(text->toStory(found.selectionStart()));
        shown.setPosition(text->toStoryEnd(found.selectionEnd()), QTextCursor::KeepAnchor);
        m_win->view()->showCursor(shown);
        refreshMatches();
        int index = 0;
        for (int i = 0; i < m_matches.size(); ++i)
            if (m_matches[i].first == shown.selectionStart()) index = i + 1;
        if (index > 0 && m_matches.size() < kMostMatches) m_status->setText(tr("Match %1 of %2").arg(index).arg(m_matches.size()));
        return true;
    }
    m_status->setText(tr("JeffWrite finished searching the document. \"%1\" was not found.").arg(findTextForMessage(q)));
    return false;
}

void WriteFindPane::replaceOne()
{
    if (!m_win->document() || !searching()) return;
    m_win->view()->leaveHeaderFooter();
    m_win->view()->leaveNote();
    if (selectionIsMatch()) {
        StoryWords *text = storyWords();
        const QTextCursor &sel = m_win->view()->cursor();
        QTextCursor match(text->words());
        match.setPosition(text->toWords(sel.selectionStart()));
        match.setPosition(text->toWords(sel.selectionEnd()), QTextCursor::KeepAnchor);
        const int at = text->pieces(match.selectionStart(), match.selectionEnd()).first().first;
        const QString with = m_wild->isChecked() ? write::expandReplacement(m_replace->text(), groupsOf(match)) : m_replace->text();
        words().put(match, with);
        // The caret is after the new words, where the next search goes on from.
        QTextCursor after(text->story());
        after.setPosition(at + int(with.size()));
        m_win->view()->setCursor(after);
    }
    findNext();
}

void WriteFindPane::replaceAll()
{
    const QString q = m_find->text();
    if (!m_win->document() || !searching()) return;
    m_win->view()->leaveHeaderFooter();
    m_win->view()->leaveNote();
    // Each swap is a step of its own; the edit block makes them one.
    QTextCursor block(m_win->document()->story());
    block.beginEditBlock();
    int n = 0;
    if (customSearch()) {
        // A pattern: every match is found first, then swapped from the last to the first (so that no swap moves another), each with its own groups.
        const bool wild = m_wild->isChecked();
        const write::FindPattern pattern = wild || codesTyped() ? write::wildcardPattern(patternText(), m_case->isChecked()) : write::FindPattern();
        if (!pattern.error.isEmpty()) {
            block.endEditBlock();
            m_status->setText(pattern.error);
            return;
        }
        const FindWords w = words();
        StoryWords *text = storyWords();
        const QString plain = text->plain();
        QVector<QTextCursor> found;
        QString problem;
        for (int at = 0; found.size() < kMostMatches;) {
            const QTextCursor m = matchFrom(at, &problem);
            if (m.isNull()) break;
            at = std::max(m.selectionEnd(), at + 1);
            if (!(w.skip && w.skip(m))) found.push_back(m);
        }
        for (int i = int(found.size()) - 1; i >= 0; --i) {
            QTextCursor m = found[i];
            w.put(m, wild ? write::expandReplacement(m_replace->text(), pattern.regex.match(plain, m.selectionStart())) : m_replace->text());
            ++n;
        }
    } else {
        n = replaceInStories(words(), stories(), q, flags(), m_replace->text());
    }
    block.endEditBlock();
    m_status->setText(n == 0   ? tr("JeffWrite made no replacements. \"%1\" was not found.").arg(findTextForMessage(q))
                      : n == 1 ? tr("JeffWrite made 1 replacement.")
                               : tr("JeffWrite made %1 replacements.").arg(n));
}

} // namespace jp
