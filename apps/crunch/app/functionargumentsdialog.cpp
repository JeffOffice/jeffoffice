#include "app/functionargumentsdialog.h"

#include "app/icons.h"
#include "core/functioncall.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QElapsedTimer>
#include <QScrollArea>
#include <QScrollBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include "app/plaintext.h"

namespace jp {

using namespace jp::crunch;

namespace {

// The most arguments a call may have.
constexpr int kMostArguments = 255;

// A label that never makes its dialog wider: a text that does not fit is cut with "…" (the whole of it is in the tooltip).
class ElidedLabel : public QLabel {
public:
    using QLabel::QLabel;
    QSize minimumSizeHint() const override { return QSize(fontMetrics().horizontalAdvance(QLatin1Char('x')) * 8, QLabel::minimumSizeHint().height()); }
    QSize sizeHint() const override { return QSize(fontMetrics().horizontalAdvance(QLatin1Char('x')) * 24, QLabel::sizeHint().height()); }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setPen(palette().color(isEnabled() ? QPalette::Active : QPalette::Disabled, QPalette::WindowText));
        p.drawText(contentsRect(), int(alignment()) | Qt::TextSingleLine, fontMetrics().elidedText(text(), Qt::ElideRight, contentsRect().width()));
    }
};

QColor problemColor(const QPalette &p) { return p.color(QPalette::Window).lightness() < 128 ? QColor(0xff, 0x8a, 0x80) : QColor(0xb0, 0x00, 0x20); }

}

FunctionArgumentsDialog::FunctionArgumentsDialog(QWidget *parent, const FunctionDoc &doc, const QStringList &arguments, QChar separator)
    : QDialog(parent), m_name(QString::fromLatin1(doc.name)), m_callName(m_name), m_doc(doc), m_plan(argumentPlanOf(doc)), m_separator(separator), m_keep(int(arguments.size()))
{
    setObjectName(QStringLiteral("functionArgumentsDialog"));
    setWindowTitle(tr("Function Arguments"));
    setWindowModality(Qt::WindowModal);
    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel(m_name);
    title->setObjectName(QStringLiteral("functionArgumentsName"));
    title->setTextFormat(Qt::PlainText);
    QFont tf = title->font();
    tf.setBold(true);
    tf.setPointSizeF(tf.pointSizeF() * 1.25);
    title->setFont(tf);
    layout->addWidget(title);

    auto *holder = new QWidget;
    m_grid = new QGridLayout(holder);
    m_grid->setContentsMargins(0, 0, 0, 0);
    m_grid->setColumnStretch(1, 3);
    m_grid->setColumnStretch(3, 2);
    m_scroll = new QScrollArea;
    m_scroll->setObjectName(QStringLiteral("functionArgumentsRows"));
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setWidget(holder);
    m_scroll->setMinimumHeight(int(fontMetrics().height() * 7.5));
    layout->addWidget(m_scroll, 1);

    m_summary = new QLabel(summaryOf(doc));
    m_summary->setObjectName(QStringLiteral("functionArgumentsSummary"));
    m_summary->setTextFormat(Qt::PlainText);
    m_summary->setWordWrap(true);
    m_summary->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    layout->addWidget(m_summary);
    m_argumentHelp = new QLabel;
    m_argumentHelp->setObjectName(QStringLiteral("functionArgumentsHelp"));
    m_argumentHelp->setTextFormat(Qt::PlainText);
    m_argumentHelp->setWordWrap(true);
    m_argumentHelp->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_argumentHelp->setMinimumHeight(2 * m_argumentHelp->fontMetrics().lineSpacing());
    m_argumentHelp->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(m_argumentHelp);
    m_result = new ElidedLabel;
    m_result->setObjectName(QStringLiteral("functionArgumentsResult"));
    m_result->setTextFormat(Qt::PlainText);
    m_result->setAccessibleName(tr("Formula result"));
    QFont rf = m_result->font();
    rf.setBold(true);
    m_result->setFont(rf);
    layout->addWidget(m_result);
    m_problem = new QLabel;
    m_problem->setObjectName(QStringLiteral("functionArgumentsProblem"));
    m_problem->setTextFormat(Qt::PlainText);
    m_problem->setWordWrap(true);
    m_problem->setMinimumHeight(m_problem->fontMetrics().lineSpacing());
    m_problem->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    layout->addWidget(m_problem);
    auto *bottom = new QHBoxLayout;
    m_helpLink = new QLabel(QStringLiteral("<a href=\"help\">%1</a>").arg(tr("Help on this function").toHtmlEscaped()));
    m_helpLink->setObjectName(QStringLiteral("functionArgumentsHelpLink"));
    m_helpLink->setTextFormat(Qt::RichText);
    m_helpLink->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
    m_helpLink->setAccessibleName(tr("Help on this function"));
    {
        QPalette lp = m_helpLink->palette();
        lp.setColor(QPalette::Link, uiDark() ? QColor(0x8a, 0xb4, 0xf8) : QColor(0x0b, 0x57, 0xd0));   // (the link's own blue is unreadable on the dark look)
        lp.setColor(QPalette::LinkVisited, lp.color(QPalette::Link));
        m_helpLink->setPalette(lp);
    }
    connect(m_helpLink, &QLabel::linkActivated, this, [this] { Q_EMIT helpRequested(m_helpTopic); });
    m_helpLink->hide();
    bottom->addWidget(m_helpLink);
    bottom->addStretch(1);
    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bottom->addWidget(m_buttons);
    layout->addLayout(bottom);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    m_timer.setSingleShot(true);
    m_timer.setInterval(300);
    connect(&m_timer, &QTimer::timeout, this, [this] { step(30); });
    m_result->setText(tr("Formula result ="));

    m_building = true;
    // the rows: the ones that are always there, then the group that repeats (the first of it, and one more to go on with)
    int next = 0;
    for (int i = 0; i < m_plan.fixed.size(); ++i) addRow(m_plan.fixed[i], m_plan.fixedOptional[i], -1, next < arguments.size() ? arguments[next++] : QString());
    if (!m_plan.repeating.isEmpty()) {
        const int rest = int(arguments.size()) - next;
        const int g = int(m_plan.repeating.size());
        const int groups = std::max(2, (rest + g - 1) / g + 1);
        for (int k = 0; k < groups; ++k) {
            addGroup();
            for (int j = 0; j < g; ++j) {
                Row &row = m_rows[m_rows.size() - size_t(g) + size_t(j)];
                if (next < arguments.size()) row.edit->setText(arguments[next++]);
            }
        }
    }
    // (more arguments than the function takes: kept, so that nothing the person wrote is lost)
    for (int extra = 1; next < arguments.size(); ++extra) addRow(tr("extra %1").arg(extra), true, -1, arguments[next++]);
    m_building = false;
    m_grid->setRowStretch(int(m_rows.size()), 1);
    if (!m_rows.empty()) {
        m_rows.front().edit->setFocus();
        focusRow(0);
        m_rows.front().edit->selectAll();
    }
    // sizes follow the font (and so the screen's scale), and never pass the screen
    const int em = fontMetrics().horizontalAdvance(QLatin1Char('x'));
    setMinimumWidth(std::min(em * 60, screenRoom().width()));
    resize(std::min(em * 70, screenRoom().width()), std::min(sizeHint().height() + fontMetrics().height() * 2 * std::min<int>(int(m_rows.size()), 4), screenRoom().height()));
    m_timer.start(0);   // (what each box comes to is worked out after the dialog is shown, a little at a time)
}

QSize FunctionArgumentsDialog::screenRoom() const
{
    const QScreen *screen = parentWidget() ? parentWidget()->screen() : QGuiApplication::primaryScreen();
    return screen ? screen->availableGeometry().size() * 9 / 10 : QSize(1000, 700);
}

void FunctionArgumentsDialog::setEvaluator(const Evaluator &evaluator)
{
    m_evaluator = evaluator;
    m_known.clear();
    markStale();
    m_timer.start(0);
}

void FunctionArgumentsDialog::setHelpTopic(const QString &topic)
{
    m_helpTopic = topic;
    m_helpLink->setVisible(!topic.isEmpty());
}

void FunctionArgumentsDialog::addGroup()
{
    const int k = m_groups++;
    for (const QString &n : m_plan.repeating) addRow(repeatedName(n, k), k > 0, k, QString(), n);
}

void FunctionArgumentsDialog::addRow(const QString &name, bool optional, int group, const QString &text, const QString &helpName)
{
    const int at = int(m_rows.size());
    Row row;
    row.name = name;
    row.optional = optional;
    row.group = group;
    row.helpName = helpName.isEmpty() ? name : helpName;
    row.label = new QLabel(name);
    row.label->setTextFormat(Qt::PlainText);
    if (!optional) {
        QFont f = row.label->font();
        f.setBold(true);
        row.label->setFont(f);
    }
    row.edit = new QLineEdit(text);
    row.edit->setObjectName(QStringLiteral("functionArgument%1").arg(at));
    row.edit->setAccessibleName(optional ? tr("%1, optional").arg(name) : name);
    row.edit->setAccessibleDescription(argumentHelpOf(m_name, row.helpName));
    row.edit->installEventFilter(this);
    row.label->setBuddy(row.edit);
    row.pick = new QToolButton;
    row.pick->setObjectName(QStringLiteral("functionArgumentPick%1").arg(at));
    row.pick->setIcon(icon(QStringLiteral("grid-3x3")));
    row.pick->setAutoRaise(true);
    row.pick->setFocusPolicy(Qt::NoFocus);
    row.pick->setToolTip(jp::plainToolTip(tr("Pick cells on the sheet")));
    row.pick->setAccessibleName(tr("Pick cells on the sheet for %1").arg(name));
    row.value = new ElidedLabel;
    row.value->setObjectName(QStringLiteral("functionArgumentValue%1").arg(at));
    row.value->setTextFormat(Qt::PlainText);
    row.value->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    row.value->setAccessibleName(tr("Value of %1").arg(name));
    m_grid->addWidget(row.label, at, 0);
    m_grid->addWidget(row.edit, at, 1);
    m_grid->addWidget(row.pick, at, 2);
    m_grid->addWidget(row.value, at, 3);
    m_rows.push_back(row);
    connect(row.edit, &QLineEdit::textChanged, this, [this, edit = row.edit] {
        for (size_t i = 0; i < m_rows.size(); ++i)
            if (m_rows[i].edit == edit) {
                textChanged(int(i));
                return;
            }
    });
    connect(row.pick, &QToolButton::clicked, this, [this, edit = row.edit] {
        for (size_t i = 0; i < m_rows.size(); ++i)
            if (m_rows[i].edit == edit) {
                beginPicking(int(i));
                return;
            }
    });
    // Tab goes from one box to the next, and the buttons come after the last
    if (at > 0) setTabOrder(m_rows[size_t(at) - 1].edit, row.edit);
    if (m_buttons) setTabOrder(row.edit, m_buttons->button(QDialogButtonBox::Ok));
}

QString FunctionArgumentsDialog::argumentName(int row) const { return m_rows[size_t(row)].name; }
bool FunctionArgumentsDialog::isOptional(int row) const { return m_rows[size_t(row)].optional; }

QStringList FunctionArgumentsDialog::arguments() const
{
    QStringList out;
    for (const Row &r : m_rows) out << r.edit->text().trimmed();
    return out;
}

void FunctionArgumentsDialog::setRawArguments(const QStringList &raw) { m_raw = raw; }

// The call as it goes in the formula: the name as it was written, each argument as it was written when it was not changed (its spaces too), and the empty ones at the end that the call had.
QString FunctionArgumentsDialog::callText() const
{
    QStringList args;
    for (size_t i = 0; i < m_rows.size(); ++i) {
        const QString text = m_rows[i].edit->text().trimmed();
        args << (int(i) < m_raw.size() && m_raw[int(i)].trimmed() == text ? m_raw[int(i)] : text);
    }
    return crunch::callText(m_callName, args, m_separator, m_keep);
}

void FunctionArgumentsDialog::setArgumentText(int row, const QString &text)
{
    if (row < 0 || row >= rowCount()) return;
    m_rows[size_t(row)].edit->setText(text);
    if (m_strip && row == m_pickRow) {
        if (auto *e = stripEdit(); e && e->text() != text) {
            m_syncing = true;
            e->setText(text);
            m_syncing = false;
        }
    }
}

void FunctionArgumentsDialog::textChanged(int row)
{
    // the last repeat of a group has something in it: another one is made, so that there is always a box to go on in
    Row &r = m_rows[size_t(row)];
    if (m_building) return;
    r.valueDone = false;
    m_resultDone = false;
    if (r.group >= 0 && r.group == m_groups - 1 && !r.edit->text().trimmed().isEmpty() && int(m_rows.size()) + int(m_plan.repeating.size()) <= kMostArguments) {
        addGroup();
        m_grid->setRowStretch(int(m_rows.size()) - int(m_plan.repeating.size()), 0);
        m_grid->setRowStretch(int(m_rows.size()), 1);
    }
    m_timer.start(300);
}

void FunctionArgumentsDialog::markStale()
{
    for (Row &r : m_rows) r.valueDone = false;
    m_resultDone = false;
}

bool FunctionArgumentsDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        for (size_t i = 0; i < m_rows.size(); ++i)
            if (m_rows[i].edit == watched) {
                focusRow(int(i));
                m_scroll->ensureWidgetVisible(m_rows[i].edit);
                break;
            }
    } else if (m_strip && (watched == m_strip || watched == stripEdit())) {
        if (event->type() == QEvent::KeyPress) {
            const int key = static_cast<QKeyEvent *>(event)->key();
            if (key == Qt::Key_Escape || key == Qt::Key_Return || key == Qt::Key_Enter) {
                endPicking();
                return true;
            }
        } else if (event->type() == QEvent::Close && watched == m_strip) {
            // (closed from the window's frame: the dialog comes back, as with its own button)
            event->ignore();
            QTimer::singleShot(0, this, [this] { endPicking(); });
            return true;
        }
    }
    return QDialog::eventFilter(watched, event);
}

void FunctionArgumentsDialog::focusRow(int row)
{
    const Row &r = m_rows[size_t(row)];
    const QString what = argumentHelpOf(m_name, r.helpName);
    m_argumentHelp->setText(what.isEmpty() ? r.name : (r.optional ? tr("%1 (optional): %2") : tr("%1: %2")).arg(r.name, what));
}

EditSession::Preview FunctionArgumentsDialog::previewOf(const QString &expression)
{
    const auto it = m_known.find(expression);
    if (it != m_known.end()) return it->second;
    const EditSession::Preview p = m_evaluator ? m_evaluator(expression) : EditSession::Preview();
    if (!p.busy && !p.volatileValue) m_known.emplace(expression, p);   // (what changes each time it is worked out is not kept)
    return p;
}

void FunctionArgumentsDialog::showPreview(QLabel *label, const EditSession::Preview &p)
{
    QString text;
    if (p.ok) text = QStringLiteral("= ") + p.shown;
    else if (p.busy || p.tooMuch) text = QStringLiteral("…");
    else if (p.unsupported) text = tr("not shown");
    label->setText(text);
    label->setToolTip(jp::plainToolTip(p.ok ? p.shown : p.problem));
}

void FunctionArgumentsDialog::evaluateNow()
{
    m_timer.stop();
    step(-1);
}

// Works out what is waiting, for about `budgetMs` (at least one thing each time; a negative number is no limit): the boxes in order, then the whole call. What is left is done on the timer, so a
// call with many heavy arguments does not stop the dialog from opening or the typing from going on.
void FunctionArgumentsDialog::step(int budgetMs)
{
    if (!m_evaluator) return;
    QElapsedTimer clock;
    clock.start();
    bool any = false, busy = false, more = false;
    QStringList missing;
    auto outOfTime = [&] { return any && budgetMs >= 0 && clock.elapsed() >= budgetMs; };
    for (Row &r : m_rows) {
        const QString text = r.edit->text().trimmed();
        if (text.isEmpty()) {
            r.value->clear();
            r.value->setToolTip(QString());
            r.valueDone = true;
            if (!r.optional) missing << r.name;
            continue;
        }
        if (r.valueDone) continue;
        if (outOfTime()) {
            more = true;
            r.value->setText(QStringLiteral("…"));
            continue;
        }
        const EditSession::Preview p = previewOf(text);
        any = true;
        showPreview(r.value, p);
        if (p.busy) busy = true;
        else r.valueDone = true;
    }
    QPalette pal = m_problem->palette();
    pal.setColor(QPalette::WindowText, palette().color(QPalette::WindowText));
    if (!missing.isEmpty()) {
        m_result->setText(tr("Formula result ="));
        m_result->setToolTip(QString());
        m_problem->setPalette(pal);
        m_problem->setText(tr("Still needed: %1").arg(missing.join(QStringLiteral(", "))));
        m_resultDone = true;
    } else if (!m_resultDone && !more && !outOfTime()) {
        const EditSession::Preview whole = previewOf(callText());
        any = true;
        m_result->setText(whole.ok ? tr("Formula result = %1").arg(whole.shown) : tr("Formula result ="));
        m_result->setToolTip(jp::plainToolTip(whole.ok ? whole.shown : QString()));
        m_problem->setText(whole.problem);
        if (whole.error || whole.tooMuch || whole.unsupported) pal.setColor(QPalette::WindowText, problemColor(palette()));
        m_problem->setPalette(pal);
        if (whole.busy) busy = true;
        else m_resultDone = true;
    } else if (!m_resultDone) {
        more = true;
    }
    if (busy) m_timer.start(300);   // (the sheet is being worked out: again in a moment)
    else if (more) m_timer.start(0);
}

QLineEdit *FunctionArgumentsDialog::stripEdit() const { return m_strip ? m_strip->findChild<QLineEdit *>(QStringLiteral("functionArgumentsStripEdit")) : nullptr; }

// The dialog gives way to a strip with just the box, so the sheet can be seen and cells pointed at; the strip's button (or Enter) brings the dialog back.
void FunctionArgumentsDialog::beginPicking(int row)
{
    if (m_strip || row < 0 || row >= rowCount()) return;
    m_pickRow = row;
    m_strip = new QWidget(parentWidget(), Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    m_strip->setObjectName(QStringLiteral("functionArgumentsStrip"));
    m_strip->setWindowTitle(tr("Function Arguments: %1").arg(m_rows[size_t(row)].name));
    auto *l = new QHBoxLayout(m_strip);
    auto *edit = new QLineEdit(m_rows[size_t(row)].edit->text());
    edit->setObjectName(QStringLiteral("functionArgumentsStripEdit"));
    edit->setAccessibleName(tr("%1: select cells on the sheet").arg(m_rows[size_t(row)].name));
    edit->setMinimumWidth(fontMetrics().horizontalAdvance(QLatin1Char('x')) * 30);
    l->addWidget(edit, 1);
    auto *back = new QToolButton;
    back->setObjectName(QStringLiteral("functionArgumentsStripBack"));
    back->setIcon(icon(QStringLiteral("expand")));
    back->setAutoRaise(true);
    back->setToolTip(jp::plainToolTip(tr("Back to the dialog")));
    back->setAccessibleName(tr("Back to the dialog"));
    l->addWidget(back);
    connect(back, &QToolButton::clicked, this, &FunctionArgumentsDialog::endPicking);
    connect(edit, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (m_syncing || m_pickRow < 0) return;
        m_syncing = true;
        m_rows[size_t(m_pickRow)].edit->setText(text);
        m_syncing = false;
    });
    m_strip->installEventFilter(this);
    edit->installEventFilter(this);
    m_strip->adjustSize();
    // near the top right of the window, and in the screen whatever the window is
    const QWidget *w = parentWidget();
    const QScreen *screen = w ? w->screen() : QGuiApplication::primaryScreen();
    const QPoint wanted = w ? w->mapToGlobal(QPoint(std::max(0, w->width() - m_strip->width() - 40), 120)) : QPoint(100, 100);
    auto place = [this, screen, wanted] {
        QPoint at = wanted;
        if (screen) {
            const QRect room = screen->availableGeometry();
            at.setX(std::clamp(at.x(), room.left(), std::max(room.left(), room.right() + 1 - m_strip->width())));
            at.setY(std::clamp(at.y(), room.top(), std::max(room.top(), room.bottom() + 1 - m_strip->height())));
        }
        m_strip->move(at);
    };
    place();
    hide();
    m_strip->show();
    place();   // (once more: the size it has when shown may differ from the one asked for)
    edit->setFocus();
    edit->selectAll();
    Q_EMIT pickingStarted(row);
}

void FunctionArgumentsDialog::dropStrip()
{
    if (!m_strip) return;
    QWidget *strip = m_strip;
    m_strip = nullptr;
    m_pickRow = -1;
    strip->hide();
    strip->deleteLater();
}

void FunctionArgumentsDialog::endPicking()
{
    if (!m_strip) return;
    const int row = m_pickRow;
    dropStrip();
    Q_EMIT pickingEnded();
    // (what the boxes come to is worked out again: the cells pointed at are new, and nothing kept from before is trusted)
    m_known.clear();
    markStale();
    m_timer.start(0);
    show();
    raise();
    activateWindow();
    if (row >= 0 && row < rowCount()) m_rows[size_t(row)].edit->setFocus();
}

void FunctionArgumentsDialog::done(int result)
{
    if (m_strip) {
        dropStrip();
        Q_EMIT pickingEnded();
    }
    QDialog::done(result);
}

} // namespace jp
