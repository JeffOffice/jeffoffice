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
        return;
    }
    // The style the caret is in: its paragraph's, or the character style of the character before it when the paragraph has none of its own listed.
    const QTextCursor &c = m_win->view()->cursor();
    QString paragraph = write::paragraphStyleAt(*doc, c);
    m_caretKey = QStringLiteral("p:") + paragraph;
    // (Typing changes none of this, unless it brings a style into use and only the styles in use are listed.)
    size_t used = 0;
    if (filter() == write::StyleFilter::InUse)
        for (const QString &id : doc->stylesInUse()) used += qHash(id);
    if (m_madeDoc == doc && m_madeStyles == doc->styleSheetRevision() && m_madeUsed == used && m_madeFilter == int(filter()) && m_madeCaretKey == m_caretKey) return;
    rebuild();
    m_madeDoc = doc;
    m_madeStyles = doc->styleSheetRevision();
    m_madeUsed = used;
    m_madeFilter = int(filter());
    m_madeCaretKey = m_caretKey;
}

void StylesPane::rebuild()
{
    write::WriteDoc *doc = m_win->document();
    const QSignalBlocker blocker(m_list);
    const QString chosen = keyAt(m_list->currentRow());
    const int scroll = m_list->verticalScrollBar()->value();
    m_list->clear();
    const QVector<write::ListedStyle> styles = write::listStyles(*doc, filter());
    int shown = 0;
    for (const write::ListedStyle &s : styles) {
        const QString key = (s.character ? QStringLiteral("c:") : QStringLiteral("p:")) + s.id;
        auto *item = new QListWidgetItem(s.name, m_list);
        item->setData(Qt::UserRole, key);
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
        if (key == m_caretKey) {
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
            item->setText(QStringLiteral("▸ ") + item->text());
        }
    }
    const int at = rowOf(chosen.isEmpty() ? m_caretKey : chosen);
    if (at >= 0) m_list->setCurrentRow(at);
    m_list->verticalScrollBar()->setValue(scroll);
}

} // namespace jp
