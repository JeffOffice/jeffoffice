#pragma once
// The Find and Replace pane, on the right of the window (Home > Editing, Ctrl+F and Ctrl+H). Find Next looks in the text of the document from
// the end of the selection, round to the start again, and shows each match selected, scrolling the pages to it. Replace swaps the match that
// is shown and goes to the next. Replace All swaps every one as a single step that undoes, and says how many there were. Hidden text is not
// searched, a bookmark or a field's code between two words does not part them (the search is of the characters that show, and a swap leaves those pieces where
// they are), and the search is made by the code of libs/page that JeffDeck's pane uses (app/findreplace.h), given JeffWrite's one story.

#include "app/findreplace.h"
#include "core/findops.h"

#include <QPair>
#include <QHash>
#include <QRegularExpression>
#include <QPointer>
#include <QTextDocument>
#include <QVector>
#include <QWidget>
#include <memory>

class QCheckBox;
class QLabel;
class QLineEdit;
class QTimer;
class QToolButton;

namespace jp {

class StoryWords;
class WriteWindow;

class WriteFindPane : public QWidget {
    Q_OBJECT
public:
    explicit WriteFindPane(WriteWindow *win, QWidget *parent = nullptr);
    ~WriteFindPane() override;
    // Puts the cursor in Find what, or in Replace with.
    void focusBox(bool replace);
    // False, with the reason in status(), when there is no such text.
    bool findNext();
    void replaceOne();
    void replaceAll();
    QString status() const;
    // How many places the search text is found in the document now (at most kMostMatches), and where: ranges of the story. The count and the yellow bands on the pages follow the text of
    // the box, the two choices, and the document as they change.
    int matchCount() const { return m_matches.size(); }
    static constexpr int kMostMatches = 20000;
    QLineEdit *findBox() const { return m_find; }
    QLineEdit *replaceBox() const { return m_replace; }
    QCheckBox *matchCase() const { return m_case; }
    QCheckBox *wholeWords() const { return m_whole; }
    QCheckBox *useWildcards() const { return m_wild; }
    // Advanced Find: words that sound like the one typed, every form of it, and letters found with punctuation or spaces between them that the box leaves out.
    QCheckBox *soundsLike() const { return m_sounds; }
    QCheckBox *wordForms() const { return m_forms; }
    QCheckBox *ignorePunctuation() const { return m_ignorePunct; }
    QCheckBox *ignoreSpaces() const { return m_ignoreSpace; }
    // Find by format: what the found text must be (bold, a font, a paragraph style, and so on). With nothing in Find what, every stretch of text that has the format is a match.
    void setFormatFilter(const write::FormatFilter &filter);
    const write::FormatFilter &formatFilter() const { return m_format; }
    QToolButton *specialButton() const { return m_special; }
    QToolButton *formatButton() const { return m_formatButton; }
    // Where the words are that the search finds in the story of a header, a footer, or a note (positions in that story); what the pages draw as bands there.
    QVector<QPair<int, int>> matchesInSideStory(const QTextDocument *story);

private:
    QTextDocument::FindFlags flags() const;
    void refreshMatches();
    // The first match of the search at or after a place of the words, as the search sees it (a pattern, or the plain words); null when there is none. `error` says what is wrong with a pattern.
    QTextCursor matchFrom(int from, QString *error = nullptr);
    QTextCursor matchIn(StoryWords *text, int from, QString *error);
    QTextCursor rawMatchIn(StoryWords *text, int from, QString *error);
    // The search is by a pattern (wildcards, or special codes such as ^p typed in the box), by the other readings of Advanced Find, or by format: not by the plain words.
    bool customSearch() const;
    bool searching() const;
    bool codesTyped() const;
    QString patternText() const;
    write::AdvancedFind advancedOptions() const;
    void syncChoices();
    void updateFormatLabel();
    QRegularExpressionMatch groupsOf(const QTextCursor &match, StoryWords *text);
    void hideEvent(QHideEvent *e) override;
    void showEvent(QShowEvent *e) override;
    // The story's words as they are now (made again when the story has changed), and the search's way to them.
    StoryWords *storyWords();
    FindWords words();
    FindWords wordsOf(StoryWords *text);
    StoryWords *sideWords(const QTextDocument *story);
    StoryWords *wordsOfStory(const QTextDocument *story);
    bool searchStory(QTextDocument *story, int from, QTextCursor *found, QString *problem);
    QString whereIs(QTextDocument *story, int page);
    QVector<QTextDocument *> sideStories();
    int replaceInStory(QTextDocument *story, QString *problem);
    bool selectionIsMatch();
    WriteWindow *m_win;
    std::unique_ptr<StoryWords> m_text;
    QLineEdit *m_find, *m_replace;
    QCheckBox *m_case, *m_whole, *m_wild, *m_sounds, *m_forms, *m_ignorePunct, *m_ignoreSpace;
    QToolButton *m_special = nullptr, *m_formatButton = nullptr;
    QLabel *m_formatLabel = nullptr;
    write::FormatFilter m_format;
    struct Side {
        std::shared_ptr<StoryWords> words;
        int revision = -1, generation = -1;
        QVector<QPair<int, int>> ranges;
    };
    QHash<const QTextDocument *, Side> m_side;
    int m_generation = 0;
    QLabel *m_status;
    QVector<QPair<int, int>> m_matches;
    QTimer *m_refresh = nullptr;
    QPointer<QTextDocument> m_watched;
};

} // namespace jp
