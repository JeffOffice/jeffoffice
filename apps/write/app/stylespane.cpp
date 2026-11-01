#include "app/stylespane.h"

#include "app/pageview.h"
#include "app/writewindow.h"
#include "core/editops.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>

namespace jp {

namespace {

// How many styles are given the look of their own in the list (the look of each is worked out from its properties, which costs a little for each).
constexpr int kMostPreviewed = 300;

} // namespace

StylesPane::StylesPane(WriteWindow *win, QWidget *parent) : QWidget(parent), m_win(win)
{
    setObjectName(QStringLiteral("stylesPane"));
    auto *v = new QVBoxLayout(this);
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("stylesList"));
    m_list->setAccessibleName(QCoreApplication::translate("jp::StylesPane", "Styles"));
    m_list->setUniformItemSizes(true);
    v->addWidget(m_list, 1);
    m_filter = new QComboBox(this);
    m_filter->setObjectName(QStringLiteral("stylesFilter"));
    m_filter->setAccessibleName(QCoreApplication::translate("jp::StylesPane", "Styles to show"));
    m_filter->addItem(QCoreApplication::translate("jp::StylesPane", "In Use"), int(write::StyleFilter::InUse));
    m_filter->addItem(QCoreApplication::translate("jp::StylesPane", "All Styles"), int(write::StyleFilter::All));
    m_filter->addItem(QCoreApplication::translate("jp::StylesPane", "Recommended"), int(write::StyleFilter::Recommended));
    m_filter->setCurrentIndex(2);
    v->addWidget(m_filter);
    auto *row = new QHBoxLayout;
    m_new = new QPushButton(QCoreApplication::translate("jp::StylesPane", "New Style…"), this);
    m_new->setObjectName(QStringLiteral("newStyleButton"));
    m_modify = new QPushButton(QCoreApplication::translate("jp::StylesPane", "Modify…"), this);
    m_modify->setObjectName(QStringLiteral("modifyStyleButton"));
    row->addWidget(m_new);
    row->addWidget(m_modify);
    v->addLayout(row);
    m_idle.setSingleShot(true);
    m_idle.setInterval(600);
    connect(&m_idle, &QTimer::timeout, this, &StylesPane::recheckUse);
    connect(m_filter, &QComboBox::currentIndexChanged, this, [this] { refresh(); });
    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) { apply(m_list->row(item)); });
    connect(m_new, &QPushButton::clicked, this, [this] { m_win->act(QStringLiteral("style.new"))->trigger(); });
    connect(m_modify, &QPushButton::clicked, this, [this] { m_win->act(QStringLiteral("style.modify"))->trigger(); });
}

write::StyleFilter StylesPane::filter() const { return write::StyleFilter(m_filter->currentData().toInt()); }

void StylesPane::setFilter(write::StyleFilter f) { m_filter->setCurrentIndex(m_filter->findData(int(f))); }

QString StylesPane::keyAt(int row) const { return row >= 0 && row < m_list->count() ? m_list->item(row)->data(Qt::UserRole).toString() : QString(); }

int StylesPane::rowOf(const QString &key) const
{
    for (int i = 0; i < m_list->count(); ++i)
        if (keyAt(i) == key) return i;
    return -1;
}

QString StylesPane::chosenId() const
{
    const QString key = keyAt(m_list->currentRow());
    return key.isEmpty() ? m_caretKey.mid(2) : key.mid(2);
}

void StylesPane::apply(int row)
{
    write::WriteDoc *doc = m_win->document();
    const QString key = keyAt(row);
    if (!doc || key.size() < 3) return;
    QTextCursor &c = m_win->view()->cursor();
    if (key.startsWith(QLatin1String("c:"))) write::applyCharacterStyle(*doc, c, key.mid(2));
    else write::applyParagraphStyle(*doc, c, key.mid(2));
    m_win->view()->setFocus(Qt::OtherFocusReason);
    refresh();
}

void StylesPane::refresh()
{
    write::WriteDoc *doc = m_win->document();
    if (!doc) {
        m_list->clear();
        m_madeDoc = nullptr;
        m_markedKey.clear();
        return;
    }
    // The style the caret is in: its paragraph's.
    m_caretKey = QStringLiteral("p:") + write::paragraphStyleAt(*doc, m_win->view()->cursor());
    // The list is made again only when the styles or the choice of what to list changed. Which styles the text uses can change with any edit, and finding out takes a pass over the
    // text, so while the list is of those in use that is looked at a moment after the last edit (typing costs no pass).
    if (m_madeDoc != doc || m_madeStyles != doc->styleSheetRevision() || m_madeFilter != int(filter())) {
        rebuild();
    } else {
        if (filter() == write::StyleFilter::InUse && m_usedAtRevision != doc->story()->revision()) m_idle.start();
        if (m_markedKey != m_caretKey) mark(m_caretKey);
    }
}

void StylesPane::recheckUse()
{
    write::WriteDoc *doc = m_win->document();
    if (!doc || m_madeDoc != doc || filter() != write::StyleFilter::InUse) return;
    size_t used = 0;
    for (const QString &id : doc->stylesInUse()) used += qHash(id);
    m_usedAtRevision = doc->story()->revision();
    if (used != m_madeUsed) rebuild();
}

// Marks the style the caret is in (the row in bold, with an arrow before its name), and unmarks the one marked before.
void StylesPane::mark(const QString &key)
{
    auto set = [this](const QString &k, bool on) {
        const int row = rowOf(k);
        if (row < 0) return;
        QListWidgetItem *item = m_list->item(row);
        QFont f = item->data(Qt::UserRole + 2).value<QFont>();
        if (on) f.setBold(true);
        item->setFont(f);
        const QString name = item->data(Qt::UserRole + 1).toString();
        item->setText(on ? QStringLiteral("▸ ") + name : name);
    };
    if (!m_markedKey.isEmpty()) set(m_markedKey, false);
    m_markedKey = key;
    set(key, true);
    const int row = rowOf(key);
    if (row >= 0) {
        const QSignalBlocker blocker(m_list);
        m_list->setCurrentRow(row);
    }
}

void StylesPane::rebuild()
{
    write::WriteDoc *doc = m_win->document();
    ++m_rebuilds;
    m_idle.stop();
    const QSignalBlocker blocker(m_list);
    const QString chosen = keyAt(m_list->currentRow());
    const int scroll = m_list->verticalScrollBar()->value();
    m_list->clear();
    m_markedKey.clear();
    const QVector<write::ListedStyle> styles = write::listStyles(*doc, filter(), m_caretKey.mid(2));
    int shown = 0;
    for (const write::ListedStyle &s : styles) {
        const QString key = (s.character ? QStringLiteral("c:") : QStringLiteral("p:")) + s.id;
        auto *item = new QListWidgetItem(s.name, m_list);
        item->setData(Qt::UserRole, key);
        item->setData(Qt::UserRole + 1, s.name);
        QString tip = s.character ? QCoreApplication::translate("jp::StylesPane", "Character style") : s.linked ? QCoreApplication::translate("jp::StylesPane", "Linked style") : QCoreApplication::translate("jp::StylesPane", "Paragraph style");
        if (s.inUse) tip += QStringLiteral(", ") + QCoreApplication::translate("jp::StylesPane", "in use");
        // plain: reviewed: the tooltip is our own words (a style's name is never in it)
        item->setToolTip(tip);
        // The look of the style in its name (for the first of them).
        if (shown++ < kMostPreviewed && s.inDocument) {
            const QTextCharFormat cf = doc->styleCharFormat(s.character ? doc->defaultParagraphStyle() : s.id, s.character ? s.id : QString());
            QFont f = cf.font();
            f.setPointSizeF(std::clamp(cf.fontPointSize() > 0 ? cf.fontPointSize() : 11.0, 9.0, 16.0));
            item->setFont(f);
        }
        item->setData(Qt::UserRole + 2, item->font());
    }
    m_madeDoc = doc;
    m_madeStyles = doc->styleSheetRevision();
    m_madeFilter = int(filter());
    m_madeUsed = 0;
    if (filter() == write::StyleFilter::InUse)
        for (const QString &id : doc->stylesInUse()) m_madeUsed += qHash(id);
    m_usedAtRevision = doc->story()->revision();
    mark(m_caretKey);
    const int at = rowOf(chosen.isEmpty() ? m_caretKey : chosen);
    if (at >= 0) m_list->setCurrentRow(at);
    m_list->verticalScrollBar()->setValue(scroll);
}

} // namespace jp
