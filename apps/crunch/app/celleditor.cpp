#include "app/celleditor.h"

#include "app/icons.h"
#include "app/settings.h"
#include "app/spelling.h"
#include "core/spellwalk.h"
#include "text/autocorrect.h"

#include <QAction>
#include <algorithm>
#include <QContextMenuEvent>
#include <QFont>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>
#include <optional>
#include "sheet/entry.h"

namespace jp {

CellEditor::CellEditor(QWidget *parent) : QLineEdit(parent)
{
    setObjectName(QStringLiteral("cellEditor"));
    setFrame(false);
    setMaxLength(1 << 20);   // (a line holds 32,767 characters by default and cuts what is typed or pasted past that; a cell's limit is told to the person when the entry is finished)
    setContextMenuPolicy(Qt::DefaultContextMenu);
    // over the cell: the program's accent round it, and the window's own base inside (a cell is white in a light window and dark in a dark one)
    setStyleSheet(QStringLiteral("QLineEdit#cellEditor { border: 2px solid %1; background: palette(base); color: palette(text); padding: 0px 2px; }").arg(uiAccent().name()));
    // (the words are looked at a moment after the typing stops)
    m_spellTimer.setSingleShot(true);
    m_spellTimer.setInterval(120);
    connect(&m_spellTimer, &QTimer::timeout, this, &CellEditor::checkSpelling);
    // (the choice is the suite's, and a change shows in an open editor at once)
    connect(&jp::SpellSettingNotifier::get(), &jp::SpellSettingNotifier::changed, this, [this] { checkSpelling(); });
    // (with "Check spelling as you type" off, nothing is started at a key or a move of the caret)
    connect(this, &QLineEdit::textChanged, this, [this] { if (spellingOn()) m_spellTimer.start(); });
    connect(this, &QLineEdit::cursorPositionChanged, this, [this] { if (spellingOn()) m_spellTimer.start(); });
}

bool CellEditor::spellingOn() { return jp::spellAsYouType(); }

bool CellEditor::isFormulaEntry(const QString &text) { return jp::office::sheet::isFormulaEntry(text); }

void CellEditor::checkSpelling()
{
    m_spellTimer.stop();
    QVector<QPair<int, int>> wrong;
    const QString entry = text();
    // (a text of many thousands of characters is not looked at as it is typed)
    if (spellingOn() && entry.size() <= kSpellLimit && !isFormulaEntry(entry) && isVisible()) {
        ++m_checkCount;
        wrong = crunch::misspelledRanges(entry);
        // (the word that is still being typed is not told of: the caret is at its end)
        const int caret = cursorPosition();
        if (hasFocus())
            wrong.erase(std::remove_if(wrong.begin(), wrong.end(), [&](const QPair<int, int> &w) { return w.second == caret; }), wrong.end());
    }
    // The places of the words are measured here, once for a text and a font, and not at every paint: how far along the line each character starts.
    if (!wrong.isEmpty() && (m_measuredText != entry || m_measuredFont != font())) {
        const QFontMetrics fm(font());
        m_advances.resize(entry.size() + 1);
        int x = 0;
        for (int i = 0; i < entry.size(); ++i) {
            m_advances[i] = x;
            x += fm.horizontalAdvance(entry[i]);
        }
        m_advances[entry.size()] = x;
        m_measuredText = entry;
        ++m_measureCount;
        m_measuredFont = font();
    }
    if (wrong == m_wrong) return;
    m_wrong = wrong;
    update();
}

void CellEditor::paintEvent(QPaintEvent *e)
{
    QLineEdit::paintEvent(e);
    if (m_wrong.isEmpty() || m_measuredText != text()) return;   // (the words are told of again when the measuring is done for the new text)
    QPainter p(this);
    p.setPen(QPen(QColor(0xD1, 0x34, 0x38), 1));
    const QRect caret = cursorRect();
    const int origin = caret.x() - m_advances.value(cursorPosition());   // (where the text begins, as it is scrolled)
    const int y = caret.bottom() - 1;
    const QRect visible = e->rect();
    for (const auto &w : m_wrong) {
        const int x1 = origin + m_advances.value(w.first), x2 = origin + m_advances.value(w.second);
        if (x2 < visible.left() || x1 > visible.right()) continue;   // (only the words that are in the part being painted)
        QPainterPath wave;
        wave.moveTo(x1, y);
        bool up = true;
        for (int x = x1 + 2; x < x2 + 2; x += 2, up = !up) wave.lineTo(std::min(x, x2), up ? y - 2 : y);
        p.drawPath(wave);
    }
}

bool CellEditor::addSpellingSuggestions(QMenu &menu, int position)
{
    if (!spellingOn() || isFormulaEntry(text())) return false;
    for (const auto &w : crunch::misspelledRanges(text())) {
        if (position < w.first || position > w.second) continue;
        const QString word = text().mid(w.first, w.second - w.first);
        const QStringList suggestions = spellingSuggestions(word, crunch::proofingLanguage()).mid(0, 6);
        QFont bold = menu.font();
        bold.setBold(true);
        for (const QString &s : suggestions) {
            QAction *a = menu.addAction(s, &menu, [this, w, s] {
                replaceRange(w.first, w.second, s);   // (a step of the entry's own Undo)
            });
            a->setFont(bold);
        }
        if (suggestions.isEmpty()) menu.addAction(QCoreApplication::translate("Proofing", "(No spelling suggestions)"))->setEnabled(false);
        menu.addAction(QCoreApplication::translate("Proofing", "Ignore All"), &menu, [this, word] {
            spellingIgnore(word);
            checkSpelling();
        });
        menu.addAction(QCoreApplication::translate("Proofing", "Add to Dictionary"), &menu, [this, word] {
            QString failure;
            if (!spellingAdd(word, &failure)) spellingReportFailure(this, failure);
            checkSpelling();
        });
        menu.addSeparator();
        return true;
    }
    return false;
}

void CellEditor::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);
    if (!hasSelectedText()) addSpellingSuggestions(menu, cursorPositionAt(e->pos()));
    QMenu *standard = createStandardContextMenu();
    standard->setParent(&menu);
    menu.addActions(standard->actions());
    menu.exec(e->globalPos());
}

void CellEditor::replaceRange(int from, int to, const QString &with)
{
    setSelection(from, to - from);
    insert(with);
}

bool CellEditor::typeWithAutoCorrect(QKeyEvent *e)
{
    const QString typed = e->text();
    if (typed.size() != 1 || isReadOnly() || !typed[0].isPrint() || (e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier | Qt::AltModifier))) return false;
    Settings &st = Settings::get();
    const bool correct = st.value(QStringLiteral("proof/autocorrect"), true).toBool();
    const bool quotes = st.value(QStringLiteral("proof/smartQuotes"), true).toBool();
    const bool format = st.value(QStringLiteral("proof/autoformat"), true).toBool();
    if (!correct && !quotes && !format) return false;
    if (isFormulaEntry(text())) return false;
    const int start = hasSelectedText() ? selectionStart() : cursorPosition();
    // A straight quote becomes the curly one the place calls for (a ' that begins the entry marks it as text, and stays).
    if (quotes && (typed == QLatin1String("\"") || (typed == QLatin1String("'") && start > 0))) {
        insert(autotext::smartQuote(typed, start > 0 ? text()[start - 1] : QChar()));
        e->accept();
        return true;
    }
    QLineEdit::keyPressEvent(e);
    e->accept();
    // The fixes are made when a word is ended by a space or a mark, each as a step of its own, so that one Undo takes the fix back and leaves what was typed.
    if (!QStringLiteral(" .,;:!?").contains(typed[0]) || hasSelectedText()) return true;
    int pos = cursorPosition();
    if (pos < 1 || text()[pos - 1] != typed[0] || isFormulaEntry(text())) return true;
    const QString before = text().left(pos - 1);
    if (correct) {
        if (const auto fix = proseFix(before, typed[0], quotes)) {
            replaceRange(fix->from, fix->to, fix->with);
            pos += int(fix->with.size()) - (fix->to - fix->from);
            setCursorPosition(pos);
        }
    }
    if (format) {
        const QString now = text().left(pos - 1);
        // (a fraction or an ordinal is made of what an entry of numbers is: only an entry with words in it is changed)
        if (std::any_of(now.cbegin(), now.cend(), [](QChar c) { return c.isLetter(); }))
            if (const auto fix = autotext::formatWord(now); fix && !fix->superscript) {
                replaceRange(fix->from, fix->to, fix->with);
                setCursorPosition(pos + int(fix->with.size()) - (fix->to - fix->from));
            }
    }
    return true;
}

// What AutoCorrect makes of the word that ends `before`, as an entry in a cell takes it. The rules are the programs' shared ones, with these for a cell, where an entry is as often a code, a
// file name, or an address as a sentence: a word in a token with a dot, an at sign, or a slash (report.xlsx, john.smith@x.com, a path) is left alone; the first word of the entry is not
// made a capital (a sentence has to end first: a period, !, or ?, and a space); and the replacements (a typing slip, (c), IM) are made only after the first word of the entry, in text with
// words in it, so that an identifier typed alone is not changed.
std::optional<autotext::Fix> CellEditor::proseFix(const QString &before, QChar typed, bool smartQuotes)
{
    const int tokenStart = int(before.lastIndexOf(QRegularExpression(QStringLiteral("\\s")))) + 1;
    const QString token = before.mid(tokenStart);
    if (token.contains(QRegularExpression(QStringLiteral("[./@\\\\]")))) return std::nullopt;
    // the end of a sentence before the word, and a space after it
    const QString head = before.left(tokenStart);
    static const QRegularExpression sentenceEnd(QStringLiteral("[.!?]\\s+$"));
    const bool capital = sentenceEnd.match(head).hasMatch() && typed != QLatin1Char('.') && typed != QLatin1Char(':');
    const bool first = head.trimmed().isEmpty();
    // (without a capital to give: the word is looked at in a text where it is no sentence's first)
    static const QString lead = QStringLiteral("a ");
    // (a sentence's end before the word is no end, when the word is not to be a capital)
    QString text = lead + before;
    if (capital) text = before;
    else
        for (int i = 0; i < tokenStart; ++i)
            if (QStringLiteral(".!?").contains(text[i + lead.size()])) text[i + lead.size()] = QLatin1Char(',');
    auto fix = autotext::correctWord(text, int(text.size()), smartQuotes);
    if (!fix) return std::nullopt;
    if (!capital) {
        fix->from -= int(lead.size());
        fix->to -= int(lead.size());
    }
    if (first) {
        // (the only fix that is made to a first word is two initial capitals made one: "THursday")
        const QString word = before.mid(fix->from, fix->to - fix->from);
        if (word.size() < 3 || !word[0].isUpper() || !word[1].isUpper() || fix->with != word.left(1) + word.mid(1).toLower()) return std::nullopt;
    }
    return fix;
}

void CellEditor::keyPressEvent(QKeyEvent *e)
{
    const bool shift = e->modifiers() & Qt::ShiftModifier;
    const bool alt = e->modifiers() & Qt::AltModifier;
    const bool ctrlOrMeta = e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier);
    switch (e->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (alt) {
            insert(QStringLiteral("\n"));
        } else if (e->modifiers() & Qt::ControlModifier) {
            Q_EMIT finished(Way::Fill);
        } else {
            Q_EMIT finished(shift ? Way::ShiftEnter : Way::Enter);
        }
        e->accept();
        return;
    case Qt::Key_Tab:
        Q_EMIT finished(Way::Tab);
        e->accept();
        return;
    case Qt::Key_Backtab:
        Q_EMIT finished(Way::ShiftTab);
        e->accept();
        return;
    case Qt::Key_Escape:
        Q_EMIT cancelled();
        e->accept();
        return;
    case Qt::Key_F2:
        m_mode = m_mode == Mode::Typing ? Mode::Editing : Mode::Typing;
        e->accept();
        return;
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:
        // typing over a cell: an arrow finishes the entry and moves the selection on
        if (m_mode == Mode::Typing && !shift && !ctrlOrMeta && !alt) {
            Q_EMIT finished(e->key() == Qt::Key_Up ? Way::Up : e->key() == Qt::Key_Down ? Way::Down : e->key() == Qt::Key_Left ? Way::Left : Way::Right);
            e->accept();
            return;
        }
        if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
            e->accept();   // (one line: nothing above or below to go to)
            return;
        }
        break;
    default: break;
    }
    if (typeWithAutoCorrect(e)) return;
    QLineEdit::keyPressEvent(e);
}

} // namespace jp
