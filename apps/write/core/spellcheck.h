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
#include <QTimer>
#include <QVector>

class QTextBlock;
class QTextDocument;

namespace jp::write {

class SpellCheck : public QObject {
    Q_OBJECT
public:
    explicit SpellCheck(QTextDocument *story, QObject *parent = nullptr);

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
    // Whether paragraphs asked for wait for idle time.
    bool pending() const { return !m_wanted.isEmpty(); }

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
    void refresh() { Q_EMIT changed(); }

    // How many paragraphs have been checked (a paragraph checked again counts again), and how many words were looked at in all, and the bytes that what is
    // kept takes (not counting the document's own): for the tests.
    qint64 paragraphsChecked() const { return m_checked; }
    qint64 wordsChecked() const { return m_words; }
    qint64 keptBytes() const;

Q_SIGNALS:
    // Words have been checked in idle time, or the checking changed: the pages should be drawn again.
    void changed();

private:
    struct Answer;
    Answer *answerFor(const QTextBlock &block, int revision) const;
    // Checks a paragraph, on from where an earlier pass left off: until it is done, or the pass is out of time, or (`upTo`, an offset in it) it has been checked past
    // that place. `ignoreTime` goes on whatever the pass says.
    Answer *check(const QTextBlock &block, int revision, int upTo = -1, bool ignoreTime = false);
    int currentRevision() const;
    bool outOfTime() const;
    void want(int from, int to);
    void idle();

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
};

// Puts `text` in place of the words [from, to) of the story, in the look of their first letter, as one step that undoes.
void replaceWord(QTextDocument *story, int from, int to, const QString &text);

} // namespace jp::write
