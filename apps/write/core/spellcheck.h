#pragma once
// Spelling as you type: which words of the story are misspelled, found a paragraph at a time with the suite's checker (libs/shell/app/spelling)
// and kept with the paragraph, so that a paragraph is checked again only when its text, the languages in it, or the words accepted have changed.
// Nothing here waits for the dictionary: the page view asks for the paragraphs it is drawing, a paragraph the pass has no time for is left
// for idle time, and the view is told when the answer is there.

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
    // Whether paragraphs asked for wait for idle time.
    bool pending() const { return !m_wanted.isEmpty(); }

    // The misspelled word that a position is in or at the end of, or (-1, -1). Checks the paragraph if need be.
    QPair<int, int> misspellingAt(int position);
    // The first misspelled word that starts at or after a position, or (-1, -1) when there is none; it checks as far as it goes.
    QPair<int, int> nextMisspelling(int from);

    // How many paragraphs have been checked (a paragraph checked again counts again), for the tests.
    qint64 paragraphsChecked() const { return m_checked; }

Q_SIGNALS:
    // Words have been checked in idle time, or the checking changed: the pages should be drawn again.
    void changed();

private:
    struct Answer;
    Answer *answerFor(const QTextBlock &block, int revision) const;
    Answer *check(const QTextBlock &block, int revision);
    int currentRevision() const;
    bool outOfTime() const;
    void want(int from, int to);
    void idle();

    QPointer<QTextDocument> m_story;
    QString m_language;
    bool m_enabled = true;
    qint64 m_checked = 0;
    // The pass under way: its budget (-1 for none), when it began, and how many paragraphs it has checked.
    int m_budget = -1;
    QElapsedTimer m_pass;
    int m_doneInPass = 0;
    QVector<QPair<int, int>> m_wanted;   // the ranges that paragraphs were left unchecked in
    QTimer m_idle;
};

} // namespace jp::write
