#include "app/formulahelper.h"

#include "core/functioninfo.h"
#include "sheet/tables.h"

#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QScreen>
#include <QVBoxLayout>

namespace jp {

using namespace crunch;

FormulaHelper::FormulaHelper(QObject *parent) : QObject(parent)
{
    // the list and the sentence under it are one window that takes no focus; the arguments are another
    m_box = new QWidget(nullptr, Qt::ToolTip | Qt::FramelessWindowHint);
    m_box->setObjectName(QStringLiteral("formulaList"));
    m_box->setAttribute(Qt::WA_ShowWithoutActivating);
    m_box->setAutoFillBackground(true);
    auto *layout = new QVBoxLayout(m_box);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);
    m_list = new QListWidget;
    m_list->setObjectName(QStringLiteral("formulaListItems"));
    m_list->setAccessibleName(tr("Functions and names that fit"));
    m_list->setFocusPolicy(Qt::NoFocus);
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(m_list);
    m_detail = new QLabel;
    m_detail->setObjectName(QStringLiteral("formulaListDetail"));
    m_detail->setTextFormat(Qt::PlainText);
    m_detail->setWordWrap(true);
    m_detail->setMargin(4);
    layout->addWidget(m_detail);
    m_hint = new QLabel(nullptr, Qt::ToolTip | Qt::FramelessWindowHint);
    m_hint->setObjectName(QStringLiteral("formulaHint"));
    m_hint->setAttribute(Qt::WA_ShowWithoutActivating);
    m_hint->setTextFormat(Qt::RichText);
    m_hint->setMargin(4);
    m_hint->setAutoFillBackground(true);
    m_hint->setAccessibleName(tr("Arguments of the function"));
    connect(m_list, &QListWidget::currentRowChanged, this, [this] { markChanged(); });
    connect(m_list, &QListWidget::itemClicked, this, [this] { acceptMarked(); });
}

FormulaHelper::~FormulaHelper()
{
    delete m_box;
    delete m_hint;
}

void FormulaHelper::attach(QLineEdit *line)
{
    line->installEventFilter(this);
    connect(line, &QLineEdit::textEdited, this, [this, line] {
        m_line = line;
        m_arrowed = false;
        refresh();
    });
    connect(line, &QLineEdit::cursorPositionChanged, this, [this, line] {
        if (m_working || !line->hasFocus()) return;
        m_line = line;
        refresh();
    });
}

void FormulaHelper::setSources(const std::function<const CompletionIndex &()> &index, const std::function<QString(const Completion &)> &detail)
{
    m_index = index;
    m_detailOf = detail;
}

void FormulaHelper::close()
{
    m_box->hide();
    m_hint->hide();
    m_found.clear();
    m_arrowed = false;
}

bool FormulaHelper::isListOpen() const { return m_box->isVisible(); }
bool FormulaHelper::isHintOpen() const { return m_hint->isVisible(); }

QStringList FormulaHelper::offered() const
{
    QStringList out;
    for (const Completion &c : m_found) out << c.text;
    return out;
}

void FormulaHelper::refresh()
{
    QLineEdit *line = m_line;
    if (!line || !m_index) {
        close();
        return;
    }
    const QChar separator = m_separator ? m_separator() : QLatin1Char(',');
    const FormulaSpot spot = spotAt(line->text(), line->cursorPosition(), separator);
    m_spot = spot;
    std::vector<Completion> found;
    if (spot.canComplete) found = m_index().matches(spot.word, kMaxOffered);
    else if (spot.inColumns) found = m_index().columnMatches(spot.table, spot.columnWord, kMaxOffered);
    // a column that is typed whole already has nothing to offer
    if (spot.inColumns && found.size() == 1 && found.front().text.compare(spot.columnWord, Qt::CaseInsensitive) == 0) found.clear();
    // a word that is a whole function or name already (and nothing else begins so) has nothing to offer
    if (found.size() == 1 && found.front().text.compare(spot.word, Qt::CaseInsensitive) == 0 && found.front().kind != Completion::Kind::Function) found.clear();
    if (found.empty()) {
        m_found.clear();
        m_list->clear();
        m_box->hide();
    } else {
        showList(line, found, spot);
    }
    showHint(line, spot);
    place(line);
}

void FormulaHelper::showList(QLineEdit *line, const std::vector<Completion> &found, const FormulaSpot &spot)
{
    Q_UNUSED(line);
    Q_UNUSED(spot);
    m_found = found;
    const QSignalBlocker block(m_list);
    m_list->clear();
    for (const Completion &c : found) {
        auto *item = new QListWidgetItem(c.text);
        item->setToolTip(m_detailOf ? m_detailOf(c) : QString());
        m_list->addItem(item);
    }
    m_list->setCurrentRow(0);
    markChanged();
    m_list->doItemsLayout();
    const int rowHeight = std::max(m_list->fontMetrics().height() + 4, m_list->visualItemRect(m_list->item(0)).height());
    m_list->setFixedHeight(rowHeight * int(found.size()) + 4);
    int widest = 120;
    for (int i = 0; i < m_list->count(); ++i) widest = std::max(widest, m_list->fontMetrics().horizontalAdvance(m_list->item(i)->text()) + 30);
    m_box->setFixedWidth(std::min(360, widest + 40));
    m_box->adjustSize();
}

void FormulaHelper::markChanged()
{
    const int row = m_list->currentRow();
    m_detail->setText(row >= 0 && row < int(m_found.size()) && m_detailOf ? m_detailOf(m_found[size_t(row)]) : QString());
}

void FormulaHelper::showHint(QLineEdit *line, const FormulaSpot &spot)
{
    Q_UNUSED(line);
    const FunctionDoc *doc = spot.function.isEmpty() ? nullptr : docOf(spot.function);
    if (!doc) {
        m_hint->hide();
        return;
    }
    const QChar separator = m_separator ? m_separator() : QLatin1Char(',');
    m_hint->setText(hintHtml(*doc, spot.argument, separator));
    m_hint->adjustSize();
}

// The arguments go right under the line; the list under them (or under the line, with no arguments to show).
void FormulaHelper::place(QLineEdit *line)
{
    QPoint below = line->mapToGlobal(QPoint(0, line->height()));
    if (m_hint->text().isEmpty() || !docOf(m_spot.function)) m_hint->hide();
    else {
        m_hint->move(below);
        m_hint->show();
        m_hint->raise();
        below.ry() += m_hint->height();
    }
    if (m_found.empty() || m_list->count() == 0) {
        m_box->hide();
        return;
    }
    // under the word being typed, not the start of the line
    const int wordX = line->fontMetrics().horizontalAdvance(line->text().left(m_spot.wordStart));
    below.rx() += std::min(wordX, std::max(0, line->width() - 40));
    m_box->move(below);
    m_box->show();
    m_box->raise();
}

bool FormulaHelper::acceptMarked()
{
    QLineEdit *line = m_line;
    const int row = m_list->currentRow();
    if (!isListOpen() || !line || row < 0 || row >= int(m_found.size())) return false;
    const Completion c = m_found[size_t(row)];
    const QString text = line->text();
    const int cursor = line->cursorPosition();
    if (c.kind == Completion::Kind::Column || c.kind == Completion::Kind::SpecialItem) {
        // a column's name goes in its brackets: the typed part of it is replaced (a name with a mark in it needs double brackets), and the bracket is closed
        const int from = m_spot.columnStart;
        QString put = c.kind == Completion::Kind::SpecialItem ? c.text : jp::office::sheet::escapeTableColumn(c.text);
        const bool double_ = c.kind == Completion::Kind::Column && jp::office::sheet::tableColumnNeedsDoubleBrackets(c.text);
        // (a name that needs the double brackets, written where the single ones are, is made into the inner pair)
        if (double_ && from > 0 && text.mid(from - 1, 1) == QLatin1String("[") && (from < 2 || text[from - 2] != QLatin1Char('['))) {
            put = QLatin1Char('[') + put;
        }
        int end = cursor;
        while (end < text.size() && text[end] != QLatin1Char(']') && text[end] != QLatin1Char(',') && text[end] != QLatin1Char(':') && text[end] != QLatin1Char('[')) ++end;
        const QString closing = double_ && put.startsWith(QLatin1Char('[')) ? QStringLiteral("]") : QString();
        const bool hasClose = end < text.size() && text[end] == QLatin1Char(']');
        const QString now = text.left(from) + put + closing + (hasClose ? QString() : QStringLiteral("]")) + text.mid(end + (hasClose ? 0 : 0));
        m_working = true;
        line->setText(now);
        line->setCursorPosition(from + int(put.size()) + int(closing.size()) + 1);
        m_working = false;
        Q_EMIT line->textEdited(now);
        m_arrowed = false;
        return true;
    }
    // the word is replaced by the entry; a function is given its bracket
    const int start = m_spot.wordStart;
    QString put = c.text;
    if (c.kind == Completion::Kind::Function && !(cursor < text.size() && text[cursor] == QLatin1Char('('))) put += QLatin1Char('(');
    // (the rest of the word after the cursor, if the cursor was in the middle of one, goes too)
    int end = cursor;
    while (end < text.size() && (text[end].isLetterOrNumber() || text[end] == QLatin1Char('_') || text[end] == QLatin1Char('.'))) ++end;
    const QString now = text.left(start) + put + text.mid(end);
    m_working = true;
    line->setText(now);
    line->setCursorPosition(start + int(put.size()));
    m_working = false;
    // the other line (the cell and the formula bar show one text) is told the text changed, as a keystroke tells it
    Q_EMIT line->textEdited(now);
    m_arrowed = false;
    return true;
}

bool FormulaHelper::eventFilter(QObject *watched, QEvent *event)
{
    auto *line = qobject_cast<QLineEdit *>(watched);
    if (!line) return false;
    if (event->type() == QEvent::FocusOut) {
        // (the list is a window of its own, that takes no focus: a line that loses it has done with the help)
        if (m_line == line) close();
        return false;
    }
    if (event->type() != QEvent::KeyPress || !isListOpen()) return false;
    auto *key = static_cast<QKeyEvent *>(event);
    switch (key->key()) {
    case Qt::Key_Down:
        m_list->setCurrentRow(std::min(m_list->count() - 1, m_list->currentRow() + 1));
        m_arrowed = true;
        return true;
    case Qt::Key_Up:
        m_list->setCurrentRow(std::max(0, m_list->currentRow() - 1));
        m_arrowed = true;
        return true;
    case Qt::Key_Tab:
        return acceptMarked();
    case Qt::Key_Return:
    case Qt::Key_Enter:
        // (Enter finishes the entry unless the list was walked: then it takes the entry marked)
        return m_arrowed && acceptMarked();
    case Qt::Key_Escape:
        close();
        return true;
    default: return false;
    }
}

} // namespace jp
