#include "app/writefindpane.h"

#include "app/pageview.h"
#include "app/writewindow.h"
#include "core/editops.h"
#include "text/textprops.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextCursor>
#include <QVBoxLayout>

namespace jp {

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
}

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

// The document's one story, as libs/page's search is given it.
FindWords WriteFindPane::words() const
{
    FindWords w;
    QTextDocument *story = m_win->document() ? m_win->document()->story() : nullptr;
    w.story = [story](const FindStory &) { return story; };
    // Hidden text is not found (nor replaced): it is not shown.
    w.skip = [](const QTextCursor &match) { return !write::allSelected(match, [](const QTextCharFormat &f) { return !f.boolProperty(tp::HiddenText); }); };
    // The new words look as the first of the old ones did.
    w.put = [](QTextCursor &match, const QString &with) {
        QTextCursor first(match.document());
        first.setPosition(match.selectionStart() + 1);
        match.insertText(with, first.charFormat());
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
bool WriteFindPane::selectionIsMatch() const
{
    const QTextCursor &sel = m_win->view()->cursor();
    if (!sel.hasSelection() || m_find->text().isEmpty()) return false;
    const QTextCursor m = m_win->document()->story()->find(m_find->text(), sel.selectionStart(), flags());
    return !m.isNull() && m.selectionStart() == sel.selectionStart() && m.selectionEnd() == sel.selectionEnd() && !words().skip(m);
}

bool WriteFindPane::findNext()
{
    const QString q = m_find->text();
    if (!m_win->document() || q.isEmpty()) return false;
    int story = 0;
    QTextCursor found;
    if (findInStories(words(), stories(), q, flags(), 0, m_win->view()->cursor().selectionEnd(), &story, &found)) {
        m_win->view()->showCursor(found);
        m_status->clear();
        return true;
    }
    m_status->setText(tr("JeffWrite finished searching the document. \"%1\" was not found.").arg(findTextForMessage(q)));
    return false;
}

void WriteFindPane::replaceOne()
{
    if (!m_win->document() || m_find->text().isEmpty()) return;
    if (selectionIsMatch()) {
        QTextCursor c = m_win->view()->cursor();
        words().put(c, m_replace->text());
        m_win->view()->setCursor(c);
    }
    findNext();
}

void WriteFindPane::replaceAll()
{
    const QString q = m_find->text();
    if (!m_win->document() || q.isEmpty()) return;
    // Each swap is a step of its own; the edit block makes them one.
    QTextCursor block(m_win->document()->story());
    block.beginEditBlock();
    const int n = replaceInStories(words(), stories(), q, flags(), m_replace->text());
    block.endEditBlock();
    m_status->setText(n == 0   ? tr("JeffWrite made no replacements. \"%1\" was not found.").arg(findTextForMessage(q))
                      : n == 1 ? tr("JeffWrite made 1 replacement.")
                               : tr("JeffWrite made %1 replacements.").arg(n));
}

} // namespace jp
