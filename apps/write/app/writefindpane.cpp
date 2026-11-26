#include "app/writefindpane.h"
#include "core/syslocale.h"

#include "app/pageview.h"
#include "app/writewindow.h"
#include "core/contentcontrolops.h"
#include "core/editops.h"
#include "core/fieldops.h"
#include "core/findops.h"
#include "core/formfieldops.h"
#include "core/pictureops.h"
#include "core/styleops.h"
#include "app/shutdown.h"
#include "core/workthread.h"
#include "core/writeprops.h"
#include "flow/flowdocument.h"
#include "text/textprops.h"

#include <QAction>
#include <QApplication>
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
#include <QProgressDialog>
#include <QPushButton>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextFragment>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <array>

namespace jp {

namespace {

// A match of the words as a cursor in the story, and as a place or a range of it (a match of no characters is a place).
QTextCursor shownIn(StoryWords *text, QTextDocument *story, const QTextCursor &m);
QPair<int, int> storyRangeOf(StoryWords *text, const QTextCursor &m);

}

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
        // (Where fields start, for the search code ^d: only when it asks, since the field's start is no part of the words that show.)
        QSet<int> fieldStarts;
        if (fields)
            for (const write::FieldPlace &f : write::scanStory(m_story, false).fields)
                if (f.begin >= 0) fieldStarts.insert(f.begin);
        for (QTextBlock b = m_story->begin(); b.isValid(); b = b.next()) {
            if (note(b)) continue;
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const QTextFragment frag = it.fragment();
                const QTextCharFormat cf = frag.charFormat();
                if (cf.hasProperty(write::prop::TrackedDeleted)) continue;   // (what a change deleted is no part of the words)
                QString text = frag.text();
                // A footnote's or an endnote's mark, and the start of a field, are one character of their own, which the codes ^f, ^e, and ^d find.
                const QString code = cf.stringProperty(tp::Field);
                if (code.startsWith(QLatin1String("footnote:"))) text.replace(QChar(0xFFFC), QChar(write::kFindFootnoteMark));
                else if (code.startsWith(QLatin1String("endnote:"))) text.replace(QChar(0xFFFC), QChar(write::kFindEndnoteMark));
                else if (cf.hasProperty(write::prop::Atom) && text == QString(QChar(0xFFFC)) && write::unpackAtom(write::storedBytes(cf, write::prop::Atom)).tag == QLatin1String("endnoteReference")) text = QString(QChar(write::kFindEndnoteMark));
                if (!fieldStarts.isEmpty())
                    for (int i = 0; i < text.size(); ++i)
                        if (fieldStarts.contains(frag.position() + i)) text[i] = QChar(write::kFindField);
                if (write::isInvisibleAtom(cf)) {
                    if (text.size() == 1 && text[0] == QChar(write::kFindField)) extend(frag.position(), text);
                    continue;
                }
                extend(frag.position(), text);
            }
            QTextBlock after = b.next();
            while (after.isValid() && note(after)) after = after.next();
            if (after.isValid()) extend(b.position() + b.length() - 1, b.blockFormat().boolProperty(flow::fp::SectionBreakAfter) ? QString(QChar(write::kFindSectionBreak)) : QStringLiteral("\n"));
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
    // Whether the words have the start of each field (for the search code ^d); changing it makes them again.
    bool fields = false;
    void wantFields(bool on)
    {
        if (on == fields) return;
        fields = on;
        dirty = true;
    }

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

namespace {

QTextCursor shownIn(StoryWords *text, QTextDocument *story, const QTextCursor &m)
{
    QTextCursor shown(story);
    shown.setPosition(text->toStory(m.selectionStart()));
    if (m.hasSelection()) shown.setPosition(text->toStoryEnd(m.selectionEnd()), QTextCursor::KeepAnchor);
    return shown;
}

QPair<int, int> storyRangeOf(StoryWords *text, const QTextCursor &m)
{
    const int a = text->toStory(m.selectionStart());
    return {a, m.hasSelection() ? text->toStoryEnd(m.selectionEnd()) : a};
}

}

namespace {

// (Made with new and never deleted, so that no destructor of a static ever runs; used on the window's thread only.)
QVector<WriteFindPane *> &livePanes()
{
    static auto *panes = new QVector<WriteFindPane *>;
    return *panes;
}
int s_stepsAdded = 0;

}

WriteFindPane::WriteFindPane(WriteWindow *win, QWidget *parent) : QWidget(parent), m_win(win)
{
    setObjectName(QStringLiteral("findPane"));
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(6, 6, 6, 6);
    // Each label names its box for screen readers (its buddy).
    auto *findLabel = new QLabel(tr("Find what"), this);
    v->addWidget(findLabel);
    m_find = new QLineEdit(this);
    m_find->setObjectName(QStringLiteral("findWhat"));
    findLabel->setBuddy(m_find);
    v->addWidget(m_find);
    auto *replaceLabel = new QLabel(tr("Replace with"), this);
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
    m_regex = new QCheckBox(tr("Use regular expressions"), this);
    m_regex->setObjectName(QStringLiteral("useRegex"));
    m_regex->setToolTip(tr("Finds text by a pattern, such as \\d{3} for three digits. In Replace with, $1 is the first group in ( )."));
    v->addWidget(m_regex);
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
    // Which way Find Next and Replace go from the caret: down to the end, up to the start, or all the way round (Replace All always does every story).
    auto *directionRow = new QHBoxLayout();
    auto *directionLabel = new QLabel(tr("Search"), this);
    m_direction = new QComboBox(this);
    m_direction->setObjectName(QStringLiteral("findDirection"));
    m_direction->addItem(tr("Down"), int(Direction::Down));
    m_direction->addItem(tr("Up"), int(Direction::Up));
    m_direction->addItem(tr("All"), int(Direction::All));
    m_direction->setCurrentIndex(m_direction->findData(int(Direction::All)));
    directionLabel->setBuddy(m_direction);
    directionRow->addWidget(directionLabel);
    directionRow->addWidget(m_direction, 1);
    v->addLayout(directionRow);
    auto *more = new QHBoxLayout();
    m_special = new QToolButton(this);
    m_special->setObjectName(QStringLiteral("findSpecial"));
    m_special->setText(tr("Special"));
    m_special->setPopupMode(QToolButton::InstantPopup);
    m_special->setToolTip(tr("Puts a code in Find what for a paragraph mark, a tab, a digit, a letter, white space, a section break, a field, or a footnote or endnote mark."));
    auto *specials = new QMenu(m_special);
    struct Code { const char *name, *code; };
    static const Code codes[] = {{QT_TR_NOOP("Paragraph Mark"), "^p"}, {QT_TR_NOOP("Tab Character"), "^t"}, {QT_TR_NOOP("Any Digit"), "^#"}, {QT_TR_NOOP("Any Letter"), "^$"},
                                 {QT_TR_NOOP("White Space"), "^w"}, {QT_TR_NOOP("Section Break"), "^b"}, {QT_TR_NOOP("Field"), "^d"}, {QT_TR_NOOP("Footnote Mark"), "^f"},
                                 {QT_TR_NOOP("Endnote Mark"), "^e"}, {QT_TR_NOOP("Caret Character"), "^^"}};
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
    // What the person does (types in the box, ticks a choice, edits the text, or presses Find Next, Replace, or Replace All) ends the notice of a search that was cancelled or needed too much work; a count that is
    // made again for any other reason leaves it.
    connect(m_find, &QLineEdit::textChanged, this, [this] { m_keepNotice = false; });
    for (QCheckBox *c : {m_case, m_whole, m_wild, m_regex, m_sounds, m_forms, m_ignorePunct, m_ignoreSpace}) connect(c, &QCheckBox::toggled, this, [this] { m_keepNotice = false; });
    connect(m_find, &QLineEdit::textChanged, m_refresh, qOverload<>(&QTimer::start));
    connect(m_case, &QCheckBox::toggled, m_refresh, qOverload<>(&QTimer::start));
    connect(m_whole, &QCheckBox::toggled, m_refresh, qOverload<>(&QTimer::start));
    connect(m_wild, &QCheckBox::toggled, m_refresh, qOverload<>(&QTimer::start));
    connect(m_regex, &QCheckBox::toggled, m_refresh, qOverload<>(&QTimer::start));
    connect(m_wild, &QCheckBox::toggled, this, [this](bool on) { if (on) m_regex->setChecked(false); });
    connect(m_regex, &QCheckBox::toggled, this, [this](bool on) { if (on) m_wild->setChecked(false); });
    for (QCheckBox *c : {m_sounds, m_forms, m_ignorePunct, m_ignoreSpace}) connect(c, &QCheckBox::toggled, m_refresh, qOverload<>(&QTimer::start));
    for (QCheckBox *c : {m_wild, m_regex, m_sounds, m_forms}) connect(c, &QCheckBox::toggled, this, [this] { syncChoices(); });
    // The pages ask for the words found in a header, a footer, or a note when they draw it.
    m_win->view()->setFindSideMatcher([this](const QTextDocument *story) { return matchesInSideStory(story); });
    // The program's end stops a search that is under way (the thread is waited for, within the time the step is given): one step for every pane there is, added by the first.
    livePanes().push_back(this);
    if (s_stepsAdded == 0) {
        ++s_stepsAdded;
        addShutdownStep([](int) {
            cancelAllSearches();
            return 0;
        });
    }
}

int WriteFindPane::livePaneCount() { return int(livePanes().size()); }
int WriteFindPane::shutdownStepsAdded() { return s_stepsAdded; }

void WriteFindPane::cancelAllSearches()
{
    const QVector<WriteFindPane *> panes = livePanes();
    for (WriteFindPane *pane : panes) pane->cancelSearch();
}

// Some choices do not go with others (a pattern has its own way of saying these things; a word read by its sound or its forms is a whole word, in any case).
void WriteFindPane::syncChoices()
{
    const bool wild = m_wild->isChecked() || m_regex->isChecked(), words = m_sounds->isChecked() || m_forms->isChecked();
    m_whole->setDisabled(wild || words);
    m_sounds->setDisabled(wild || m_forms->isChecked());
    m_forms->setDisabled(wild || m_sounds->isChecked());
    m_wild->setDisabled(words);
    m_regex->setDisabled(words);
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

// Special codes typed in the box (^p, ^t, ^#, ^$, ^w, ^b, ^d, ^f, ^e, ^^) work without "Use wildcards": the other characters then are themselves.
bool WriteFindPane::codesTyped() const
{
    if (m_wild->isChecked() || m_regex->isChecked() || advancedOptions().active()) return false;
    const QString q = m_find->text();
    for (int i = 0; i + 1 < q.size(); ++i)
        if (q[i] == QLatin1Char('^') && QStringLiteral("pt#$w^bdfe").contains(q[i + 1])) return true;
    return false;
}

// The pattern the search is made with: the box as typed with wildcards on; with only codes, the box with each character that a pattern reads as more than itself held back.
QString WriteFindPane::patternText() const
{
    const QString q = m_find->text();
    if (m_wild->isChecked() || m_regex->isChecked()) return q;
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

bool WriteFindPane::searching() const { return !m_find->text().isEmpty() || m_format.any(); }

// The words of the story of a header, a footer, or a note, as they are now.
StoryWords *WriteFindPane::sideWords(const QTextDocument *story)
{
    Side &side = m_side[story];
    if (!side.words || side.words->story() != story) {
        side.words = std::make_shared<StoryWords>(const_cast<QTextDocument *>(story));
        side.revision = -1;
    }
    side.words->wantFields(!m_regex->isChecked() && m_find->text().contains(QLatin1String("^d")));
    if (side.words->dirty) {
        side.words->rebuild();
        side.revision = -1;   // (the places found were of the words as they were)
    }
    return side.words.get();
}

QVector<QPair<int, int>> WriteFindPane::matchesInSideStory(const QTextDocument *story)
{
    if (!story || !isVisible() || !searching() || m_find->text().isEmpty()) return {};
    const struct Inline { bool &flag; bool was; Inline(bool &f) : flag(f), was(f) { flag = true; } ~Inline() { flag = was; } } inlineScans(m_inlineScans);   // (the pages are drawing: no box over them)
    Side &side = m_side[story];
    StoryWords *words = sideWords(story);
    if (side.revision == story->revision() && side.generation == m_generation) return side.ranges;
    side.revision = story->revision();
    side.generation = m_generation;
    side.ranges.clear();
    QString error;
    for (int at = 0; side.ranges.size() < kMostMatches;) {
        const QTextCursor m = matchIn(words, at, &error);
        if (m.isNull()) break;
        at = nextPlace(m, at);
        side.ranges.push_back(storyRangeOf(words, m));
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
        const QTextCursor shown = shownIn(text, story, m);
        if (m_format.matchesStyle(styleOfBlock(shown.selectionStart())) && write::allSelected(shown, [&](const QTextCharFormat &f) { return m_format.matchesChar(f); })) return m;
        at = nextPlace(m, at);
    }
}

QTextCursor WriteFindPane::rawMatchIn(StoryWords *text, int from, QString *error)
{
    if (m_regex->isChecked()) return regexMatchIn(text, from, error);
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

// Where a search goes on after a match: its end, and one character on after a match of no characters (so that it is never found twice).
int WriteFindPane::nextPlace(const QTextCursor &match, int at) { return std::max(match.selectionEnd() + (match.hasSelection() ? 0 : 1), at + 1); }

namespace {

constexpr int kShortText = 2000;   // a text of no more characters than this is searched on the window's thread

int hitNextPlace(const write::RegexHit &h) { return h.second + (h.second > h.first ? 0 : 1); }

// What is said when a scan ended before the end of the text, and why (for a scan that was cancelled, hit the engine's limit of work, or met a match that is too long to be shown whole).
QString stoppedMessage(const write::RegexScan &scan)
{
    if (scan.tooLong) return QCoreApplication::translate("WriteFindPane", "A match longer than %1 characters was found, which is more than JeffWrite can work with at once. Nothing was changed.").arg(jp::workerLocale().toString(write::kRegexContextMost));
    if (scan.limited) return QCoreApplication::translate("WriteFindPane", "That pattern needed too much work on this text, so the search stopped. Nothing was changed.");
    return QCoreApplication::translate("WriteFindPane", "Cancelled. Nothing was changed.");
}

}

// One search on a thread of its own: what it reads is its own (a copy of the text, which the window's own copy is shared with, and the pattern), and what it says is read after it has ended.
struct WriteFindPane::Job {
    quint64 id = 0;
    QString plain, query, signature;
    write::FindPattern pattern;
    int from = 0, most = 0, revision = 0;
    bool fresh = true, matchCase = false;
    std::shared_ptr<std::atomic<bool>> cancel = std::make_shared<std::atomic<bool>>(false);
    std::function<bool(int)> hook;
    QPointer<QTextDocument> story;
    QPointer<QProgressDialog> box;
    QPointer<QWidget> focus;
    write::RegexScan result;
    std::unique_ptr<jp::WorkThread> thread;
};

QTextCursor WriteFindPane::regexMatchIn(StoryWords *text, int from, QString *error)
{
    m_lastGroups.clear();
    const write::FindPattern pattern = write::regexPattern(m_find->text(), m_case->isChecked());
    if (!pattern.error.isEmpty()) {
        if (error) *error = pattern.error;
        return QTextCursor();
    }
    const QString plain = text->plain();   // (a copy that shares the text: a search on a thread of its own reads it while the window goes on)
    QTextDocument *story = text->story();
    const bool large = plain.size() > kShortText;
    if (large && m_memos.size() >= 64 && !m_memos.contains(story)) m_memos.clear();
    RegexMemo &m = large ? m_memos[story] : m_smallMemo;
    // A batch of matches answers the asks that follow it in order (the place after the last match asked for, as the loops that count and swap go); any other place is searched afresh.
    int next = -1;
    if (m.held.constData() == plain.constData() && m.held.size() == plain.size() && m.revision == story->revision() && m.query == m_find->text() && m.matchCase == m_case->isChecked()) {
        if (from == m.from) {
            next = 0;
        } else {
            const auto it = std::lower_bound(m.hits.cbegin(), m.hits.cend(), from, [](const write::RegexHit &h, int at) { return hitNextPlace(h) < at; });
            if (it != m.hits.cend() && hitNextPlace(*it) == from) next = int(it - m.hits.cbegin()) + 1;
        }
    }
    if (next < 0 || (next >= m.hits.size() && !m.done)) {
        const bool fresh = next < 0;
        const int scanFrom = fresh ? from : m.hits.isEmpty() ? m.from : hitNextPlace(m.hits.last());
        const bool onThread = !m_inlineScans && plain.size() - scanFrom > kShortText;
        if (onThread && m_job) {   // (asked while a search is under way: after it)
            m_wait = true;
            return QTextCursor();
        }
        if (onThread && startScan(text, plain, pattern, scanFrom, 1024, fresh)) {
            m_wait = true;
            return QTextCursor();
        }
        const std::function<bool(int)> hook = m_scanHook;
        const write::RegexScan scan = write::scanRegex(plain, pattern, scanFrom, 1024, hook);
        if (scan.cancelled || scan.limited || scan.tooLong) {
            m_keepNotice = true;
            m = RegexMemo();
            if (error) *error = stoppedMessage(scan);
            return QTextCursor();
        }
        if (fresh) {
            m.held = plain;
            m.query = m_find->text();
            m.matchCase = m_case->isChecked();
            m.from = from;
            m.revision = story->revision();
            m.hits = scan.hits;
            next = 0;
        } else {
            next = int(m.hits.size());
            m.hits += scan.hits;
        }
        m.done = scan.done;
    }
    ++m_scans;
    if (next >= m.hits.size()) return QTextCursor();
    const write::RegexHit &h = m.hits[next];
    QTextCursor c(text->words());
    c.setPosition(h.first);
    c.setPosition(h.second, QTextCursor::KeepAnchor);
    m_lastGroups = h.groups;
    return c;
}

// The groups of a regular-expression match that is shown, found again.
QStringList WriteFindPane::regexGroupsOf(const QTextCursor &match, StoryWords *text)
{
    QString error;
    const QTextCursor c = regexMatchIn(text, match.selectionStart(), &error);
    return !c.isNull() && c.selectionStart() == match.selectionStart() ? m_lastGroups : QStringList();
}

// What the box and the choices say now: a search that was started with something else is not the one that is wanted.
QString WriteFindPane::searchSignature() const
{
    return QStringLiteral("%1\n%2\n%3%4%5%6%7").arg(m_find->text(), m_replace->text()).arg(m_case->isChecked()).arg(m_regex->isChecked()).arg(m_wild->isChecked()).arg(m_direction->currentData().toInt()).arg(m_format.any());
}

std::shared_ptr<std::atomic<bool>> WriteFindPane::searchCancelFlag() const { return m_job ? m_job->cancel : nullptr; }

// Starts the search of a text on a thread of its own, and blocks editing of the document until it is done: what the search reads is a copy, and what it finds is places in that text. False when
// there is no thread to be had.
bool WriteFindPane::startScan(StoryWords *text, const QString &plain, const write::FindPattern &pattern, int from, int most, bool fresh)
{
    auto job = std::make_unique<Job>();
    job->id = ++m_jobIds;
    job->plain = plain;
    job->pattern = pattern;
    job->from = from;
    job->most = most;
    job->fresh = fresh;
    job->query = m_find->text();
    job->matchCase = m_case->isChecked();
    job->signature = searchSignature();
    job->story = text->story();
    job->revision = text->story()->revision();
    job->hook = m_scanHook;
    Job *raw = job.get();
    const quint64 id = job->id;
    const std::shared_ptr<std::atomic<bool>> cancel = job->cancel;
    const std::function<bool(int)> hook = job->hook;
    job->thread = std::make_unique<jp::WorkThread>([raw, cancel, hook, this, id] {
        raw->result = write::scanRegex(raw->plain, raw->pattern, raw->from, raw->most, [&](int done) { return (!hook || hook(done)) && !cancel->load(); });
        QMetaObject::invokeMethod(this, [this, id] { jobDone(id); }, Qt::QueuedConnection);
    });
    if (!job->thread->started()) return false;
    // Nothing may be changed in the document from now until the search is done (its places are those of this text): a block of its own, which the document's read-only state has no part in.
    job->focus = QApplication::focusWidget();
    m_win->blockEditsForSearch(true);
    m_status->setText(tr("Searching…"));
    m_job = std::move(job);
    if (!m_busyTimer) {
        m_busyTimer = new QTimer(this);
        m_busyTimer->setSingleShot(true);
        connect(m_busyTimer, &QTimer::timeout, this, [this] { showBusyBox(); });
    }
    m_busyTimer->start(400);
    if (m_startedHook) m_startedHook();
    return true;
}

// The box that says a search is under way and offers Cancel, when it takes more than a moment.
void WriteFindPane::showBusyBox()
{
    if (!m_job || m_job->box) return;
    auto *box = new QProgressDialog(tr("Searching…"), tr("Cancel"), 0, 0, m_win);
    box->setObjectName(QStringLiteral("findBusyBox"));
    box->setWindowModality(Qt::WindowModal);
    box->setAutoClose(false);
    box->setAutoReset(false);
    const std::shared_ptr<std::atomic<bool>> cancel = m_job->cancel;
    connect(box, &QProgressDialog::canceled, box, [cancel] { cancel->store(true); });
    m_job->box = box;
    box->show();
}

// The search has ended: the thread is waited for, the box goes, and editing is allowed again.
void WriteFindPane::endJob(Job &job, bool restoreInput)
{
    if (m_busyTimer) m_busyTimer->stop();
    if (job.thread) job.thread->join();
    delete job.box.data();
    if (!restoreInput) return;   // (the window is going: it is not touched)
    m_win->blockEditsForSearch(false);
    if (job.focus) job.focus->setFocus(Qt::OtherFocusReason);
}

void WriteFindPane::cancelSearch(bool restoreInput)
{
    if (!m_job) return;
    m_job->cancel->store(true);
    const std::unique_ptr<Job> job = std::move(m_job);
    endJob(*job, restoreInput);
    m_retry = nullptr;
    m_wait = false;
}

// Registers what to do again when the search that was started is done (the ask is then answered from what it found).
void WriteFindPane::waitThen(const std::function<void()> &again, bool againWhenStale, bool replaces)
{
    m_retry = again;
    m_retryWhenStale = againWhenStale;
    m_retryReplaces = replaces;
    m_wait = false;
    if (!m_inRetry) m_retries = 0;
}

// The thread of a search has ended (on the window's thread). What it found is kept only if the text and the box are as they were when it began; else nothing is done with it.
void WriteFindPane::jobDone(quint64 id)
{
    if (!m_job || m_job->id != id) return;
    const std::unique_ptr<Job> job = std::move(m_job);
    endJob(*job, true);
    const std::function<void()> again = std::move(m_retry);
    const bool whenStale = m_retryWhenStale, replaces = m_retryReplaces;
    m_retry = nullptr;
    m_retryReplaces = false;
    QTextDocument *story = job->story.data();
    bool stale = !story || job->signature != searchSignature();
    if (!stale && story->revision() != job->revision) {
        // The text has been touched (a field that updates its own result, a mark that is drawn, a change of look): what counts is what the search read. When the words are the same, the places it found are right.
        StoryWords *now = storyWords();
        if (now && now->story() == story && now->plain() == job->plain) {
            job->plain = now->plain();   // (shares the words that asks from now on are given)
            job->revision = story->revision();
        } else {
            stale = true;
        }
    }
    if (stale) {
        if (again && whenStale && ++m_retries <= 100) {   // (a text that goes on changing is not searched for ever)
            m_inRetry = true;
            again();
            m_inRetry = false;
        } else {
            m_keepNotice = true;   // (the message stays until the person does something: a count of matches must not take its place)
            m_status->setText(replaces ? tr("The document changed during the search, so nothing was replaced.") : tr("The document changed during the search, so the search was stopped. Nothing was changed."));
        }
        return;
    }
    if (job->result.cancelled || job->result.limited || job->result.tooLong) {
        m_memos.remove(story);
        m_smallMemo = RegexMemo();
        m_keepNotice = true;
        m_status->setText(stoppedMessage(job->result));
        return;
    }
    // (The asks of one operation that are not answered go on, one search after another: a count that is never reached is no reason to wait for ever.)
    if (again && ++m_retries > 100000) {
        m_keepNotice = true;
        m_status->setText(tr("That pattern needed too much work on this text, so the search stopped. Nothing was changed."));
        return;
    }
    if (m_memos.size() >= 64 && !m_memos.contains(story)) m_memos.clear();
    RegexMemo &m = m_memos[story];
    if (job->fresh || m.held.constData() != job->plain.constData() || m.revision != job->revision) {
        m = RegexMemo();
        m.held = job->plain;
        m.query = job->query;
        m.matchCase = job->matchCase;
        m.from = job->from;
        m.revision = job->revision;
    }
    m.hits += job->result.hits;
    m.done = job->result.done;
    if (again) {
        m_inRetry = true;
        again();
        m_inRetry = false;
    }
}

// The groups of the pattern for a match of it, found again in the words with what is round it (so that the starts and ends of words are right).
QRegularExpressionMatch WriteFindPane::groupsOf(const QTextCursor &match, StoryWords *text)
{
    const write::FindPattern pattern = write::wildcardPattern(patternText(), m_case->isChecked());
    if (!text || !pattern.error.isEmpty()) return QRegularExpressionMatch();
    return pattern.regex.match(text->plain(), match.selectionStart());
}

// Finds every place the text of the box is (as findNext sees it), counts them, says so, and shows each on the pages.
void WriteFindPane::refreshMatches()
{
    if (m_job) {   // (asked while a search is under way: after it)
        m_refresh->start();
        return;
    }
    ++m_refreshesRun;
    if (m_keepNotice) return;   // (the message of a search that was cancelled or stopped stays until the person does something)
    m_wait = false;
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
            at = nextPlace(m, at);
            if (w.skip && w.skip(m)) continue;
            m_matches.push_back(storyRangeOf(text, m));
        }
        if (m_wait) {   // (the search goes on on a thread of its own: this is made again, from the start, when it is done)
            waitThen([this] { refreshMatches(); }, true);
            return;
        }
        if (text->story() != m_watched) {
            m_watched = text->story();
            connect(text->story(), &QTextDocument::contentsChanged, this, [this] { m_keepNotice = false; });
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

WriteFindPane::~WriteFindPane()
{
    livePanes().removeAll(this);
    cancelSearch(false);
}   // (the search's thread is waited for before anything it reads is gone)

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
    const bool fields = !m_regex->isChecked() && m_find->text().contains(QLatin1String("^d"));
    if (!m_text || m_text->story() != story) {
        m_text = std::make_unique<StoryWords>(story);
        m_text->wantFields(fields);
    } else {
        m_text->wantFields(fields);
        if (m_text->dirty) m_text->rebuild();
    }
    return m_text.get();
}

// The document's one story, as libs/page's search is given it: its words, and the places of the story that a swap puts new words in.
FindWords WriteFindPane::words() { return wordsOf(storyWords()); }

FindWords WriteFindPane::wordsOf(StoryWords *text)
{
    FindWords w;
    QTextDocument *story = text ? text->story() : nullptr;
    w.story = [text](const FindStory &) { return text ? text->words() : nullptr; };
    // Hidden text is not found (nor replaced): it is not shown.
    w.skip = [text, story](const QTextCursor &match) {
        if (match.hasSelection() ? text->toStory(match.selectionStart()) >= write::editableEnd(story) : text->toStory(match.selectionStart()) > write::editableEnd(story)) return true;   // (nor a match in the endnotes the pages set after the text)
        if (write::touchesEndnotes(story, text->toStory(match.selectionStart()), text->toStoryEnd(match.selectionEnd()))) return true;   // (nor one that goes over those at the end of a section)
        const QTextCursor shown = shownIn(text, story, match);
        return !write::allSelected(shown, [](const QTextCharFormat &f) { return !f.boolProperty(tp::HiddenText) || f.hasProperty(write::prop::TrackedDeleted); });   // (what a change deleted is no part of the words, and a match may go round it)
    };
    // The new words take the place of the first of the characters of the match, and the look of the first of them; the characters of it between pieces kept as XML
    // are taken away, and the pieces stay. (The matches come from the last to the first, so that no swap moves another.)
    write::WriteDoc *doc = m_win->document();
    w.put = [doc, text, story](QTextCursor &match, const QString &with) {
        if (!match.hasSelection()) {   // (a match of no characters: the new words go in at its place)
            if (with.isEmpty()) return;
            QTextCursor at(story);
            at.setPosition(text->toStory(match.selectionStart()));
            const QTextCharFormat here = at.charFormat();
            at.beginEditBlock();
            if (doc && doc->trackChanges() && (story == doc->story() || write::isTrackedStory(story))) write::insertText(*doc, at, with);
            else at.insertText(with, here);
            at.endEditBlock();
            return;
        }
        const QVector<QPair<int, int>> pieces = text->pieces(match.selectionStart(), match.selectionEnd());
        if (pieces.isEmpty()) return;
        QTextCursor first(story);
        first.setPosition(pieces.first().first + 1);
        const QTextCharFormat look = first.charFormat();
        QTextCursor edit(story);
        edit.beginEditBlock();
        // With Track Changes on, what goes is marked as deleted and the new words as inserted, as when they are typed over (the text, and a text box's words being changed, see TrackedStory).
        const bool record = doc && doc->trackChanges() && (story == doc->story() || write::isTrackedStory(story));
        for (int i = int(pieces.size()) - 1; i >= 1; --i) {
            edit.setPosition(pieces[i].first);
            edit.setPosition(pieces[i].second, QTextCursor::KeepAnchor);
            if (record) write::removeSelection(*doc, edit);
            else edit.removeSelectedText();
        }
        edit.setPosition(pieces.first().first);
        edit.setPosition(pieces.first().second, QTextCursor::KeepAnchor);
        if (!record) edit.insertText(with, look);
        else if (with.isEmpty()) write::removeSelection(*doc, edit);
        else write::insertText(*doc, edit, with);
        edit.endEditBlock();
    };
    return w;
}

// The words of the text, or of a header, a footer, or a note.
StoryWords *WriteFindPane::wordsOfStory(const QTextDocument *story)
{
    if (!story) return nullptr;
    if (m_win->document() && story == m_win->document()->story()) return storyWords();
    return sideWords(story);
}

// Whether what is selected is a match, as the search sees it (so that Replace does not change words the search would not have shown). The selection is in the text, or in the
// header, footer, or note that is being edited.
bool WriteFindPane::selectionIsMatch()
{
    const QTextCursor &sel = m_win->view()->cursor();
    StoryWords *text = wordsOfStory(sel.document());
    if (!text || (!sel.hasSelection() && !m_regex->isChecked()) || !searching()) return false;
    // (With a regular expression a caret alone may be at a match of no characters.)
    const int from = text->toWords(sel.selectionStart()), to = sel.hasSelection() ? text->toWords(sel.selectionEnd()) : from;
    const QTextCursor m = matchIn(text, from, nullptr);
    return !m.isNull() && m.selectionStart() == from && m.selectionEnd() == to && !wordsOf(text).skip(m);
}

// The first match at or after a place of a story's text, shown as a cursor in the story; false where there is none (or the pattern is wrong: `problem` says so).
bool WriteFindPane::searchStory(QTextDocument *story, int from, QTextCursor *found, QString *problem)
{
    StoryWords *text = wordsOfStory(story);
    if (!text) return false;
    ++m_scans;
    const FindWords w = wordsOf(text);
    for (int at = text->toWords(from);;) {
        const QTextCursor m = matchIn(text, at, problem);
        if (m.isNull()) return false;
        at = nextPlace(m, at);
        if (w.skip && w.skip(m)) continue;
        *found = shownIn(text, story, m);
        return true;
    }
}

// The headers, then the footers (by section), then the footnotes: the stories beside the text, each once, in the order a search goes through them.
QVector<QTextDocument *> WriteFindPane::sideStories()
{
    write::WriteDoc *doc = m_win->document();
    QVector<QTextDocument *> all;
    for (QTextDocument *d : doc->storiesWithChanges(true))
        if (d != doc->story() && !all.contains(d)) all.push_back(d);
    auto key = [doc](QTextDocument *d) {
        int section = 0, slot = 0;
        bool footer = false;
        if (!doc->headerFooterSlotOf(d, &section, &footer, &slot)) {
            int id = 0;
            doc->footnoteIdOf(d, &id);
            return std::array<int, 3>{2, id, 0};
        }
        return std::array<int, 3>{footer ? 1 : 0, section, slot};
    };
    std::stable_sort(all.begin(), all.end(), [&](QTextDocument *a, QTextDocument *b) { return key(a) < key(b); });
    return all;
}

// Where a header, a footer, or a note is, for the message under the boxes.
QString WriteFindPane::whereIs(QTextDocument *story, int page)
{
    int section = 0, slot = 0;
    bool footer = false;
    if (m_win->document()->headerFooterSlotOf(story, &section, &footer, &slot))
        return footer ? tr("Found in the footer of page %1.").arg(page + 1) : tr("Found in the header of page %1.").arg(page + 1);
    return tr("Found in a note on page %1.").arg(page + 1);
}

bool WriteFindPane::findNext()
{
    m_keepNotice = false;
    if (!m_win->document() || !searching()) return false;
    if (m_job) {   // (a search is under way: this is done when it is)
        waitThen([this] { findNext(); });
        return false;
    }
    m_wait = false;
    write::PageView *view = m_win->view();
    QTextDocument *body = m_win->document()->story();
    QTextDocument *here = const_cast<QTextDocument *>(view->editedSideStory());
    if (!here) here = body;
    const QString q = m_find->text();
    QString problem;
    QTextCursor found;
    bool got = false;
    auto look = [&](QTextDocument *story, int from) {
        if (!searchStory(story, from, &found, &problem)) return false;
        // (A header, footer, or note is opened for editing to show the match: where no page shows it, it is passed over.)
        if (story != body && view->editedSideStory() != story && !view->enterSideStory(story)) return false;
        return got = true;
    };
    const Direction direction = Direction(m_direction->currentData().toInt());
    QVector<QTextDocument *> order = sideStories();
    order.prepend(body);
    const int n = int(order.size()), at = std::max(0, int(order.indexOf(here)));
    // A match of no characters that was shown at the caret (or put words in) is not found again from there: the search goes on one character.
    const bool onEmpty = m_emptyAt.story == here && m_emptyAt.revision == here->revision() && view->cursor().document() == here && !view->cursor().hasSelection() && m_emptyAt.pos == view->cursor().position();
    const int after = view->cursor().selectionEnd() + (onEmpty ? 1 : 0), before = view->cursor().selectionStart() - (onEmpty ? 1 : 0);
    auto stillLooking = [&] { return !got && problem.isEmpty() && !m_wait; };
    if (direction == Direction::All) {
        // From the end of the selection to the end of the text, then the headers, footers, and notes, then the text again from the start.
        if (here == body || view->editedSideStory() == here) got = look(here, after);
        if (stillLooking()) {
            for (int k = 1; k < n && stillLooking(); ++k) got = look(order[(at + k) % n], 0);
            if (stillLooking()) got = look(here, 0);
        }
    } else if (direction == Direction::Down) {
        // To the end of the headers, footers, and notes, and no further.
        if (here == body || view->editedSideStory() == here) got = look(here, after);
        for (int k = at + 1; k < n && stillLooking(); ++k) got = look(order[k], 0);
    } else {
        // Up: the last match before the selection, then the matches of the stories before this one, and no further.
        QTextCursor last;
        if ((here == body || view->editedSideStory() == here) && before >= 0 && lastMatchBefore(here, before, &last, &problem)) {
            found = last;
            got = true;
        }
        for (int k = at - 1; k >= 0 && stillLooking(); --k) {
            if (!lastMatchBefore(order[k], -1, &last, &problem)) continue;
            if (order[k] != body && view->editedSideStory() != order[k] && !view->enterSideStory(order[k])) continue;
            found = last;
            got = true;
        }
    }
    if (m_wait) {   // (a search on a thread of its own is under way: this is asked again, from the start, when it is done)
        waitThen([this] { findNext(); });
        return false;
    }
    if (!problem.isEmpty()) {
        m_status->setText(problem);
        return false;
    }
    if (got) {
        if (!found.hasSelection()) m_emptyAt = {found.document(), found.position(), found.document()->revision()};
        view->showCursor(found);
        refreshMatches();
        if (found.document() != body) {
            int page = 0;
            view->enterSideStory(found.document(), &page);
            m_status->setText(whereIs(found.document(), page));
            return true;
        }
        int index = 0;
        for (int i = 0; i < m_matches.size(); ++i)
            if (m_matches[i].first == found.selectionStart()) index = i + 1;
        if (index > 0 && m_matches.size() < kMostMatches) m_status->setText(tr("Match %1 of %2").arg(index).arg(m_matches.size()));
        return true;
    }
    m_status->setText(direction == Direction::Down ? tr("JeffWrite reached the end of the document. \"%1\" was not found below.").arg(findTextForMessage(q))
                      : direction == Direction::Up ? tr("JeffWrite reached the start of the document. \"%1\" was not found above.").arg(findTextForMessage(q))
                                                   : tr("JeffWrite finished searching the document. \"%1\" was not found.").arg(findTextForMessage(q)));
    return false;
}

// The last match of a story that ends at or before a place of its text (before its end when `before` is negative), shown as a cursor in the story. The search goes backward from the place
// in stretches that grow, so that a press costs the matches near the caret, not a scan of the story from its start, and the matches are not counted off from the start (no limit).
bool WriteFindPane::lastMatchBefore(QTextDocument *story, int before, QTextCursor *found, QString *problem)
{
    if (before < 0) before = std::max(0, story->characterCount() - 1);
    QTextCursor m;
    for (int reach = 256;; reach *= 4) {
        const int start = std::max(0, before - reach);
        bool got = false;
        QTextCursor last;
        for (int at = start; searchStory(story, at, &m, problem);) {
            if (m.selectionEnd() > before) break;
            last = m;
            got = true;
            at = nextPlace(m, at);
        }
        if (got) {
            *found = last;
            return true;
        }
        if (start == 0 || !problem->isEmpty() || m_wait) return false;
    }
}

namespace {
// Whether a content control keeps the words of a match (in the words of `text`) from being swapped: a lock on its contents, on its being deleted, or the edge of one round paragraphs.
bool controlKeeps(write::WriteDoc *doc, QTextDocument *story, StoryWords *text, int from, int to)
{
    if (!doc || story != doc->story()) return false;
    const int a = text->toStory(from), z = to > from ? text->toStoryEnd(to) : a;
    return !write::contentControlsAllow(*doc, story, a, z, true);
}
} // namespace

// A document protected for forms is held read only, but Replace goes on in the text the protection leaves out (each match is asked where it is).
bool WriteFindPane::refusedReplace()
{
    if (m_win->formsHeld() && !m_win->searchBlocked()) return false;
    return m_win->refuseIfReadOnly();
}

bool WriteFindPane::heldByProtection(const Plan &plan, int from, int to) const
{
    if (!m_win->formsHeld()) return false;
    const int a = plan.text->toStory(from), z = to > from ? plan.text->toStoryEnd(to) : a;
    return !write::formsRangeOpen(*m_win->document(), plan.story, a, z, &plan.open);
}

void WriteFindPane::replaceOne()
{
    m_keepNotice = false;
    if (!m_win->document() || !searching() || m_job || refusedReplace()) return;
    m_wait = false;
    const bool isMatch = selectionIsMatch();
    if (m_wait) {   // (the search goes on on a thread of its own: this is asked again when it is done)
        waitThen([this] { replaceOne(); }, false, true);
        return;
    }
    if (isMatch) {
        const QTextCursor &sel = m_win->view()->cursor();
        QTextDocument *story = sel.document();
        StoryWords *text = wordsOfStory(story);
        const int from = text->toWords(sel.selectionStart()), to = sel.hasSelection() ? text->toWords(sel.selectionEnd()) : from;
        if (holdsMark(text->plain(), from, to)) {
            m_status->setText(tr("A section break, a field, or a note mark can be found but not replaced."));
            return;
        }
        if (controlKeeps(m_win->document(), story, text, from, to)) {
            m_status->setText(tr("Text in a content control that can't be changed can be found but not replaced."));
            return;
        }
        if (m_win->formsHeld() && !write::formsRangeOpen(*m_win->document(), story, text->toStory(from), to > from ? text->toStoryEnd(to) : text->toStory(from))) {
            m_status->setText(tr("Text that is protected can be found but not replaced."));
            return;
        }
        QTextCursor match(text->words());
        match.setPosition(text->toWords(sel.selectionStart()));
        if (sel.hasSelection()) match.setPosition(text->toWords(sel.selectionEnd()), QTextCursor::KeepAnchor);
        const int at = match.hasSelection() ? text->pieces(match.selectionStart(), match.selectionEnd()).first().first : text->toStory(match.selectionStart());
        const QString with = m_regex->isChecked() ? write::expandRegexReplacement(m_replace->text(), regexGroupsOf(match, text))
                             : m_wild->isChecked() ? write::expandReplacement(m_replace->text(), groupsOf(match, text)) : m_replace->text();
        const bool wasEmpty = !match.hasSelection();
        wordsOf(text).put(match, with);
        // The caret is after the new words, where the next search down goes on from; for a search up, before them (the words put in may hold what is searched for).
        QTextCursor after(story);
        after.setPosition(Direction(m_direction->currentData().toInt()) == Direction::Up ? at : at + int(with.size()));
        m_win->view()->setCursor(after);
        if (wasEmpty) m_emptyAt = {story, after.position(), story->revision()};   // (the next search does not find the same place again)
    }
    findNext();
}

// Whether the words from `from` to `to` have a section break, the start of a field, or a note's mark.
bool WriteFindPane::holdsMark(const QString &plain, int from, int to)
{
    for (int i = std::max(0, from); i < to && i < plain.size(); ++i)
        if (write::isFindMark(plain[i])) return true;
    return false;
}

bool WriteFindPane::holdsMark(const QString &plain, const QTextCursor &match) { return holdsMark(plain, match.selectionStart(), match.selectionEnd()); }

// Swaps every match in one story (the text, or a header, a footer, or a note), as one edit of that story; how many.
bool WriteFindPane::planStory(QTextDocument *story, QString *problem, StoryWords *given, Plan *plan)
{
    StoryWords *text = given ? given : wordsOfStory(story);
    if (!text) return true;
    const bool wild = m_wild->isChecked(), regex = m_regex->isChecked();
    plan->story = story;
    plan->text = text;
    if (m_win->formsHeld()) plan->open = write::formsOpenSections(*m_win->document());
    plan->pattern = regex ? write::regexPattern(m_find->text(), m_case->isChecked()) : wild || codesTyped() ? write::wildcardPattern(patternText(), m_case->isChecked()) : write::FindPattern();
    if (!plan->pattern.error.isEmpty()) {
        *problem = plan->pattern.error;
        return false;
    }
    // Every match is found first, then swapped from the last to the first (so that no swap moves another), each with its own groups.
    plan->w = wordsOf(text);
    plan->plain = text->plain();
    for (int at = 0; plan->found.size() < kMostMatches;) {
        const QTextCursor m = matchIn(text, at, problem);
        if (m.isNull()) break;
        at = nextPlace(m, at);
        if (!(plan->w.skip && plan->w.skip(m)) && !holdsMark(plan->plain, m)) {   // (a section break, a field's start, or a note's mark is found, not swapped: it would lose what it stands for)
            if (controlKeeps(m_win->document(), story, text, m.selectionStart(), m.selectionEnd())) {
                ++plan->kept;
                continue;
            }
            if (heldByProtection(*plan, m.selectionStart(), m.selectionEnd())) {
                ++plan->held;
                continue;
            }
            plan->found.push_back(m);
            if (regex) plan->groups.push_back(m_lastGroups);
        }
    }
    return !m_wait && problem->isEmpty();   // (a search that is not done, or was cancelled or stopped: nothing of this story is swapped)
}

int WriteFindPane::applyPlan(const Plan &plan)
{
    const bool wild = m_wild->isChecked(), regex = m_regex->isChecked();
    QTextCursor block(plan.story);
    block.beginEditBlock();
    int n = 0;
    for (int i = int(plan.found.size()) - 1; i >= 0; --i) {
        QTextCursor m = plan.found[i];
        plan.w.put(m, regex ? write::expandRegexReplacement(m_replace->text(), plan.groups[i]) : wild ? write::expandReplacement(m_replace->text(), plan.pattern.regex.match(plan.plain, m.selectionStart())) : m_replace->text());
        ++n;
    }
    block.endEditBlock();
    return n;
}

int WriteFindPane::replaceInStory(QTextDocument *story, QString *problem, StoryWords *given, bool onlyLook)
{
    Plan plan;
    if (!planStory(story, problem, given, &plan)) return 0;
    return onlyLook ? int(plan.found.size()) : applyPlan(plan);
}

// Swaps every match in the words of the text boxes and shapes of the text, each box's words as a story of their own, put back in the box (as the step the swaps are part of); how many.
// The paragraphs of a box that no swap touched stay as the file wrote them. `tooLarge` counts the boxes that hold more than the editor takes, or that could not take the new words.
int WriteFindPane::replaceInTextBoxes(QString *problem, int *tooLarge, bool onlyLook)
{
    const struct Inline { bool &flag; bool was; Inline(bool &f) : flag(f), was(f) { flag = true; } ~Inline() { flag = was; } } inlineScans(m_inlineScans);   // (a box's words are short, and are made for this: searched here)
    write::WriteDoc *doc = m_win->document();
    QTextDocument *body = doc->story();
    QVector<int> at;
    for (QTextBlock b = body->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            if (!f.charFormat().hasProperty(write::prop::Atom)) continue;
            for (int i = 0; i < int(f.length()); ++i)
                if (write::isDrawnObjectAt(body, f.position() + i)) at.push_back(f.position() + i);
        }
    int n = 0, boxes = 0;
    for (const int position : at) {
        const write::PictureInfo info = write::pictureAt(*doc, body, position);
        if (!info.valid || !info.editable || (info.kind != write::DrawnKind::Shape && info.kind != write::DrawnKind::TextBox)) continue;
        bool big = false;
        std::unique_ptr<QTextDocument> words = write::textBoxStory(*doc, position, &big, true);
        if (big) {
            ++*tooLarge;
            continue;
        }
        if (!words || words->toPlainText().trimmed().isEmpty()) continue;
        QStringList before;
        for (QTextBlock b = words->begin(); b.isValid(); b = b.next()) before << b.text();
        StoryWords text(words.get());
        if (text.dirty) text.rebuild();
        int swapped = 0;
        {
            const write::TrackedStory tracked(words.get());
            swapped = replaceInStory(words.get(), problem, &text, onlyLook);
        }
        if (!problem->isEmpty()) return n;
        if (swapped <= 0 || onlyLook) continue;
        if (boxes >= kMostBoxesChanged) {
            m_boxesStopped = true;
            break;
        }
        QVector<bool> changed;
        int k = 0;
        for (QTextBlock b = words->begin(); b.isValid(); b = b.next(), ++k) changed << (k >= before.size() || before[k] != b.text());
        bool refused = false;
        if (write::setTextBoxStory(*doc, position, words.get(), &refused, before.size() == words->blockCount() ? &changed : nullptr)) {
            n += swapped;
            ++boxes;
        } else if (refused) {
            ++*tooLarge;
        }
    }
    return n;
}

void WriteFindPane::replaceAll()
{
    m_keepNotice = false;
    const QString q = m_find->text();
    if (!m_win->document() || !searching() || refusedReplace()) return;
    if (m_job) {   // (a search is under way: this is done when it is)
        waitThen([this] { replaceAll(); }, false, true);
        return;
    }
    m_wait = false;
    QString problem;
    // Every story is searched first, before anything is changed: a search that is cancelled, that needs too much work, or that has to wait for a thread of its own, changes nothing. (What is
    // found is kept as it is, and the swaps are made from it: no search is made after the first swap.)
    QVector<Plan> plans(1);
    bool ok = planStory(m_win->document()->story(), &problem, nullptr, &plans[0]);
    if (ok && !m_win->formsHeld())   // (the headers, footers, notes, and text boxes are not filled in: a protection for forms holds them)
        for (QTextDocument *d : sideStories()) {
            plans.push_back(Plan());
            if (!planStory(d, &problem, nullptr, &plans.last())) {
                ok = false;
                break;
            }
        }
    if (m_wait) {   // (this is asked again, from the start, when the search is done)
        waitThen([this] { replaceAll(); }, false, true);
        return;
    }
    int none = 0;
    if (ok && m_regex->isChecked() && !m_win->formsHeld()) replaceInTextBoxes(&problem, &none, true);   // (the words of the boxes are short and searched here; this only looks)
    if (!problem.isEmpty()) {
        m_status->setText(problem);
        return;
    }
    // The text, then the headers, footers, and notes; the swaps in all of them are one step that undoes.
    const write::WriteDoc::StepGroup group(*m_win->document());
    int n = 0, locked = 0, held = 0;
    for (const Plan &plan : plans) {
        n += plan.story ? applyPlan(plan) : 0;
        locked += plan.kept;
        held += plan.held;
    }
    int tooLarge = 0;
    m_boxesStopped = false;
    if (!m_win->formsHeld()) n += replaceInTextBoxes(&problem, &tooLarge);
    if (!problem.isEmpty()) {
        m_status->setText(problem);
        return;
    }
    QString text = n == 0   ? (tooLarge > 0 || locked > 0 || held > 0 ? tr("JeffWrite made no replacements.") : tr("JeffWrite made no replacements. \"%1\" was not found.").arg(findTextForMessage(q)))
                   : n == 1 ? tr("JeffWrite made 1 replacement.")
                            : tr("JeffWrite made %1 replacements.").arg(n);
    if (locked > 0) text += QLatin1Char(' ') + tr("%n match is in content that can't be changed.", nullptr, locked);
    if (held > 0) text += QLatin1Char(' ') + tr("%n match is in text that is protected.", nullptr, held);
    if (tooLarge > 0) text += QLatin1Char(' ') + tr("%n text box was too large to change.", nullptr, tooLarge);
    if (m_boxesStopped) text += QLatin1Char(' ') + tr("Only the first %1 text boxes were changed; use Replace All again for the rest.").arg(kMostBoxesChanged);
    m_status->setText(text);
}

} // namespace jp
