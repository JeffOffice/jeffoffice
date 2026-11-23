#pragma once
// Spelling as you type: which words of the story are misspelled, found a paragraph at a time with the suite's checker (libs/shell/app/spelling)
// and kept with the paragraph, so that a paragraph is checked again only when its text, the languages in it, or the words accepted have changed.
// Nothing here holds anything up for long: the page view asks for the paragraphs it is drawing, and what the pass has no time for is left for idle time
// (a long paragraph is checked in pieces, so one keystroke in it costs a piece, not the paragraph), and the view is told when the answer is there.
// What is kept with a paragraph is a stamp of its text, not the text.

#include <QElapsedTimer>
#include <QObject>
#include <QPair>
#include <QPointer>
#include <QTextCursor>
#include <QTimer>
#include <QVector>

class QTextBlock;
class QTextDocument;

namespace jp::write {

class SpellCheck : public QObject {
    Q_OBJECT
public:
    explicit SpellCheck(QTextDocument *story, QObject *parent = nullptr);
    ~SpellCheck() override;

    bool enabled() const { return m_enabled; }
    void setEnabled(bool on);
    // The language of text that says none (BCP-47); empty is US English.
    QString defaultLanguage() const { return m_language; }
    void setDefaultLanguage(const QString &tag);

    // The time the checking asked for within a scope may take, in all, in milliseconds: a paragraph that finds the time gone is left for idle time (at least
    // one is checked in each pass). A pass of -1 has no limit, as no pass at all.
    class Pass {
    public:
        Pass(SpellCheck &check, int budgetMs);
        ~Pass();

    private:
        SpellCheck &m_check;
    };

    // The misspelled words, as [start, end) positions in the story, that overlap [from, to), in order: what has been checked of the paragraphs there, and
    // those that are not yet are checked now while the pass has time.
    QVector<QPair<int, int>> misspelledIn(int from, int to);
    QTextDocument *story() const { return m_story; }
    // Whether paragraphs asked for wait for idle time (with counting on, the paragraphs an edit changed are among them).
    bool pending() const;

    // The misspelled word that a position is in or at the end of, or (-1, -1). Checks the paragraph as far as the position if need be. (Like the search, it goes by the
    // words whether or not they are marked as you type; what offers it to the person asks `enabled()` itself.)
    QPair<int, int> misspellingAt(int position);
    // Looking for the first misspelled word that starts at or after a position, in slices: it checks paragraphs for about `budgetMs` milliseconds (at least a piece of
    // one; -1 for no limit) and says what it came to: Found (the word is in `found`), None (there is none to the end of the story), or More (the time is gone: go on from `next`).
    // (Whether the words are marked as you type does not matter to it: the Spelling pane goes by it.)
    enum class Step { Found, None, More };
    Step search(int from, int budgetMs, QPair<int, int> *found, int *next);
    // The same without a limit.
    QPair<int, int> nextMisspelling(int from);

    // Tells the pages to be drawn again (the words accepted have changed: the answers kept are stale).
    // With counting on, every paragraph is then checked again to see that the count is right.
    void refresh();

    // The count for the status bar. With counting on, the paragraphs not yet checked are checked in idle time, a slice at a time, and the paragraphs an edit changed are checked
    // again in idle time, each marked as it is changed (the marks go with the paragraphs, so a second edit before idle time moves nothing). The total is kept for the story, as
    // paragraphs are checked, however many checks look at it (the page view and a split view of it), so asking for it looks at no text. `knownMisspellings` is how many
    // misspelled words the answers kept hold, and `counted` says whether every paragraph has been checked (the number is a lower bound until then).
    void setCounting(bool on);
    bool counting() const { return m_counting; }
    int knownMisspellings() const;
    bool counted() const { return m_swept; }
    // While the window is hidden or minimized nothing is counted (what is changed is still marked); counting goes on when it is shown.
    void setPaused(bool on);
    bool paused() const { return m_paused; }
    // For the tests: the paragraphs the counting pass has looked at (checked or found checked) in all, and the ranges waiting for idle time.
    qint64 paragraphsWalked() const { return m_walked; }
    int wantedRanges() const { return int(m_wanted.size()); }

    // How many paragraphs have been checked (a paragraph checked again counts again), and how many words were looked at in all, and the bytes that what is
    // kept takes (not counting the document's own): for the tests.
    qint64 paragraphsChecked() const { return m_checked; }
    qint64 wordsChecked() const { return m_words; }
    qint64 keptBytes() const;

Q_SIGNALS:
    // Words have been checked in idle time, or the checking changed: the pages should be drawn again.
    void changed();
    // The total of misspelled words known (or whether all of them are) is not what it was.
    void countChanged();

private:
    struct Answer;
    struct Tally;
    Answer *answerFor(const QTextBlock &block, int revision) const;
    // Checks a paragraph, on from where an earlier pass left off: until it is done, or the pass is out of time, or (`upTo`, an offset in it) it has been checked past
    // that place. `ignoreTime` goes on whatever the pass says.
    Answer *check(const QTextBlock &block, int revision, int upTo = -1, bool ignoreTime = false);
    int currentRevision() const;
    bool outOfTime() const;
    void want(int from, int to);
    void idle();
    void sweepStep();
    void checkMarked(int revision);
    void restartSweep();

    QPointer<QTextDocument> m_story;
    QString m_language;
    bool m_enabled = true;
    qint64 m_checked = 0, m_words = 0;
    // The pass under way: its budget (-1 for none), when it began, and how many paragraphs it has checked.
    int m_budget = -1;
    QElapsedTimer m_pass;
    int m_doneInPass = 0;
    QVector<QPair<int, int>> m_wanted;   // the ranges that paragraphs were left unchecked in
    QTimer m_idle;
    bool m_counting = false, m_swept = false, m_paused = false;
    QPointer<Tally> m_tally;   // the story's: the total and the paragraphs marked as changed, shared by every check of the story
    QTextCursor m_sweepCursor; // where the counting pass goes on from (it moves with the edits)
    int m_sweepRevision = -1;  // the words accepted and the options the pass began with
    qint64 m_walked = 0;
};

// Puts `text` in place of the words [from, to) of the story, in the look of their first letter, as one step that undoes.
void replaceWord(QTextDocument *story, int from, int to, const QString &text);
// Whether replaceWord would change the word at [from, to): not past the end of what is editable, and not one of the endnotes the pages set.
bool canReplaceWord(const QTextDocument *story, int from, int to);

} // namespace jp::write
