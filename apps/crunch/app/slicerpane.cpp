#include "app/slicerpane.h"

#include "app/plaintext.h"

#include <QAbstractButton>
#include <QApplication>
#include <QColor>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QSignalBlocker>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

namespace jp {

namespace {

// The color a style's chosen buttons have: the style's number picks one of six accents (the named looks are our own).
QColor accentOf(const QString &style, bool timeline)
{
    static const char *const colors[] = {"#2f6fb5", "#c4571f", "#6a8f2a", "#b8860b", "#7a4fa0", "#2a8c8c"};
    static const QRegularExpression digits(QStringLiteral("(\\d+)$"));
    const auto m = digits.match(style);
    const int n = m.hasMatch() ? std::clamp(m.captured(1).toInt(), 1, 6) : 1;
    Q_UNUSED(timeline)
    return QColor(QLatin1String(colors[n - 1]));
}

} // namespace

SlicerPane::SlicerPane(QWidget *parent) : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    m_body = new QWidget(scroll);
    m_layout = new QVBoxLayout(m_body);
    scroll->setWidget(m_body);
    outer->addWidget(scroll);
    setEntries({});
}

void SlicerPane::setEntries(const QVector<Entry> &entries)
{
    // (the same slicers with other choices: the buttons are only set, so that a button that is being clicked is not taken away under the click)
    QStringList shape;
    for (const Entry &e : entries) shape << e.caption + QLatin1Char('|') + e.style + QLatin1Char('|') + e.items.join(QLatin1Char('\x1f')) + QLatin1Char('|') + e.tables.join(QLatin1Char('\x1f')) + QLatin1Char('|') + e.span + QLatin1Char('|') + e.level + QLatin1Char('|') + (e.usable ? QLatin1Char('1') : QLatin1Char('0')) + QString::number(e.columns);
    if (m_built && shape == m_shape) {
        for (int s = 0; s < entries.size() && s < m_buttons.size(); ++s)
            for (int i = 0; i < m_buttons[s].size(); ++i) {
                const QSignalBlocker block(m_buttons[s][i]);
                m_buttons[s][i]->setChecked(i < entries[s].chosen.size() && entries[s].chosen[i]);
            }
        return;
    }
    m_built = true;
    m_shape = shape;
    while (QLayoutItem *item = m_layout->takeAt(0)) {
        if (QWidget *w = item->widget()) w->deleteLater();
        delete item;
    }
    m_buttons.clear();
    m_notes.clear();
    m_captions.clear();
    if (entries.isEmpty()) {
        auto *none = new QLabel(tr("This workbook has no slicers or timelines."), m_body);
        none->setWordWrap(true);
        m_layout->addWidget(none);
    }
    for (int s = 0; s < entries.size(); ++s) {
        const Entry &e = entries[s];
        // (an & in a caption is shown as written, not taken for a shortcut mark)
        QString shownCaption = e.caption;
        shownCaption.replace(QLatin1Char('&'), QStringLiteral("&&"));
        m_captions << shownCaption;
        auto *box = new QGroupBox(shownCaption, m_body);
        auto *inner = new QVBoxLayout(box);
        m_buttons.append(QVector<QAbstractButton *>());
        if (e.timeline) {
            const QString span = e.span.isEmpty() ? tr("All dates") : e.span;
            auto *label = new QLabel(e.level.isEmpty() ? span : tr("%1 (by %2)").arg(span, e.level), box);
            label->setTextFormat(Qt::PlainText);
            label->setWordWrap(true);
            inner->addWidget(label);
        } else {
            auto *grid = new QGridLayout();
            const QColor accent = accentOf(e.style, false);
            const int shownItems = int(std::min<qsizetype>(e.items.size(), kMaxButtons));
            for (int i = 0; i < shownItems; ++i) {
                QString label = e.items[i];
                label.replace(QLatin1Char('&'), QStringLiteral("&&"));
                auto *b = new QPushButton(label, box);
                b->setCheckable(true);
                b->setChecked(i < e.chosen.size() && e.chosen[i]);
                b->setEnabled(e.usable);
                b->setStyleSheet(QStringLiteral("QPushButton:checked { background: %1; color: white; border: 1px solid %1; }").arg(accent.name()));
                connect(b, &QPushButton::clicked, this, [this, s, i] { clickItem(s, i, QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)); });
                grid->addWidget(b, i / std::max(1, e.columns), i % std::max(1, e.columns));
                m_buttons.last().append(b);
            }
            inner->addLayout(grid);
            QLabel *more = nullptr;
            if (e.items.size() > shownItems) {
                more = new QLabel(tr("and %1 more items").arg(e.items.size() - shownItems), box);
                more->setTextFormat(Qt::PlainText);
                inner->addWidget(more);
            }
            m_notes.append(more);
        }
        if (e.timeline) m_notes.append(nullptr);
        auto *connections = new QToolButton(box);
        connections->setText(tr("Report Connections…"));
        connections->setToolTip(jp::plainToolTip(e.tables.isEmpty() ? tr("No pivot table that can be read is filtered by this one.") : tr("Filters: %1").arg(e.tables.join(QStringLiteral(", ")))));
        connect(connections, &QToolButton::clicked, this, [this, s] { Q_EMIT connectionsRequested(s); });
        inner->addWidget(connections);
        m_layout->addWidget(box);
    }
    m_layout->addStretch(1);
}

QString SlicerPane::moreNoteText(int slicer) const
{
    return hasMoreNote(slicer) ? m_notes[slicer]->text() : QString();
}

QAbstractButton *SlicerPane::button(int slicer, int item) const
{
    if (slicer < 0 || slicer >= m_buttons.size() || item < 0 || item >= m_buttons[slicer].size()) return nullptr;
    return m_buttons[slicer][item];
}

} // namespace jp
