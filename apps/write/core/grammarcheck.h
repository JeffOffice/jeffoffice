#pragma once
// Grammar as you type: the grammar and style issues of the story, found a paragraph at a time with the suite's checker (libs/shell/app/grammar) and kept by the
// paragraph's text, so a paragraph that comes back to a text it had (an undo, a copy) is not checked again. Checking is done on a thread of its own, one paragraph
// at a time, a moment after the paragraph stopped changing (only its latest text is sent: a person typing makes one check, not one for each key): nothing here holds the
// window up, and a hostile paragraph cannot (the service caps what it looks at, and a budget caps what the whole document may spend). Only the English stretches of a
// paragraph are checked. The view is told, by `changed`, when an answer has come, and, by `budgetExhausted`, once, when the budget is gone and the checking stops.

#include "app/grammar.h"

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QTimer>
#include <QVector>
#include <memory>

class QTextBlock;
class QTextDocument;
class QThreadPool;

namespace jp::write {

class GrammarCheck : public QObject {
    Q_OBJECT
public:
    // An issue, as [start, end) positions in the story.
    struct Issue {
        int start = -1, end = -1;
        GrammarIssue::Kind kind = GrammarIssue::Grammar;
        QString message;
        QStringList suggestions;   // each is the whole text to put in place of [start, end)
        bool valid() const { return start >= 0; }
    };

    explicit GrammarCheck(QTextDocument *story, QObject *parent = nullptr);
    ~GrammarCheck() override;

    bool enabled() const { return m_enabled; }
    void setEnabled(bool on);
    // The language of text that says none (BCP-47); empty is US English.
    QString defaultLanguage() const { return m_language; }
    void setDefaultLanguage(const QString &tag);
    // How long a paragraph waits, unchanged, before it is sent to the checker (a person typing changes it every moment). 400 ms at first.
    void setDelayMs(int ms) { m_delay = ms; }

    // The issues that overlap [from, to), in order: those of the paragraphs already checked; the others are sent to the checker and `changed` says when they are back.
    QVector<Issue> issuesIn(int from, int to);
    QTextDocument *story() const { return m_story; }
    // The issue a position is in or at the end of (none: an invalid Issue), if its paragraph has been checked.
    Issue issueAt(int position);
    // The first issue that starts at or after a position, checking paragraphs on this thread for about `budgetMs` milliseconds (at least one): Found, None (to the
    // end of the story), or More (the time is gone: go on from `next`). For the Spelling and Grammar pane, which goes through the story in slices.
    enum class Step { Found, None, More };
    Step search(int from, int budgetMs, Issue *found, int *next);

    // Leaves an issue unmarked for the rest of the session: this one, in its place (the sentence it is in and where in it; edits elsewhere do not undo it), or every
    // issue with the same message on the same words.
    void ignoreHere(const Issue &issue);
    void ignoreAll(const Issue &issue);
    // Tells the pages to be drawn again.
    void refresh() { Q_EMIT changed(); }
    // Whether paragraphs are waiting to be sent or are being checked.
    bool pending() const { return !m_wanted.isEmpty() || m_inFlight > 0; }
    // Whether the document's budget of work is gone (nothing is checked after that, and what it would have found is not kept as "nothing").
    bool budgetGone() const { return m_exhausted; }
    // The budget of this document (the tests make it small): characters and milliseconds.
    void setBudget(qint64 chars, int ms);
    // Paragraphs sent to the checker, what is kept, and how many paragraphs have been looked at afresh (for the tests).
    int paragraphsChecked() const { return m_checked; }
    int keptEntries() const { return int(m_kept.size()); }
    int looksMade() const { return m_looksMade; }

Q_SIGNALS:
    void changed();
    void budgetExhausted();

private:
    struct Shared;
    struct Entry {
        QVector<GrammarIssue> issues;   // as [start, end) in the paragraph
    };
    // A paragraph as the checker takes it: the text, and the text with what is not English (or marked "do not check") blanked out, which is what is sent.
    struct Look {
        QString key, text, masked, language;
    };
    struct BlockLook {
        int position = -1, revision = -1, length = -1;
        Look look;
    };
    const Look &lookAt(const QTextBlock &block);
    QVector<Issue> issuesOf(const QTextBlock &block, const Look &look, const Entry &entry) const;
    QString ignoreKey(const Look &look, int start, int end, bool here) const;
    void want(int slot, const Look &look);
    void send();
    void done(const QString &key, const QVector<GrammarIssue> &issues, bool exhausted);
    void keep(const QString &key, const Entry &entry);
    void stopForBudget();

    QPointer<QTextDocument> m_story;
    QString m_language;
    bool m_enabled = true;
    int m_delay = 400;
    QHash<QString, Entry> m_kept;
    qint64 m_keptChars = 0;
    QHash<int, Look> m_wanted;           // by the paragraph's number: only the latest text of each
    QHash<int, BlockLook> m_looks;
    QSet<QString> m_sent, m_ignoredHere, m_ignoredAll;
    int m_inFlight = 0, m_checked = 0, m_looksMade = 0;
    bool m_exhausted = false;
    QTimer m_timer;
    std::shared_ptr<Shared> m_shared;
    std::unique_ptr<QThreadPool> m_pool;
};

} // namespace jp::write
