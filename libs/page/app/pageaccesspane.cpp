#include "app/pageaccesspane.h"

#include "app/editor.h"
#include "app/pageselectionpane.h"

#include <QPushButton>

namespace jp {

PageAccessibilityPane::PageAccessibilityPane(Editor *editor, bool slides, const std::function<QString(int)> &titleOf, QWidget *parent)
    : AccessibilityPane(parent), m_ed(editor), m_slides(slides), m_titleOf(titleOf)
{
    connect(this, &AccessibilityPane::recheckRequested, this, [this] { recheck(); });
    connect(addButton(tr("Reading Order"), QStringLiteral("accessReadingOrder")), &QPushButton::clicked, this, [this] { showReadingOrder(); });
    // What the list says of the order follows a change made in the Reading Order list.
    connect(m_ed, &Editor::changed, this, [this] {
        if (m_reading && m_reading->isVisible()) recheck();
    });
    connect(this, &AccessibilityPane::goToRequested, this, [this](const QVariantList &where) {
        if (where.size() != 2) return;
        m_ed->setCurrentPage(where[0].toInt());
        const QString id = where[1].toString();
        if (!id.isEmpty() && m_ed->doc()->item(id)) m_ed->select(id);
    });
    // The fix for a table with no header row: its Header Row on, as the Table Design tab does it (one step that undoes).
    connect(this, &AccessibilityPane::fixRequested, this, [this](const QVariantList &where) {
        if (where.size() != 2) return;
        Item *item = m_ed->doc()->item(where[1].toString());
        if (!item || item->type() != ItemType::Table) return;
        auto *table = static_cast<TableItem *>(item);
        m_ed->setCurrentPage(where[0].toInt());
        TableStyleParts parts = TableStyleParts::of(*table);
        parts.header = true;
        setTableStyle(m_ed, table, table->format, parts, m_styles ? m_styles() : TableStyles(), false);
        recheck();
    });
}

QVector<PageAccessIssue> PageAccessibilityPane::issues() const
{
    return m_ed->doc() ? checkPageAccessibility(*m_ed->doc(), m_slides, m_titleOf) : QVector<PageAccessIssue>();
}

void PageAccessibilityPane::showReadingOrder() { showPageReadingOrderPane(this, m_ed, &m_reading); }

void PageAccessibilityPane::recheck()
{
    QVector<AccessFinding> findings;
    for (const PageAccessIssue &i : issues())
        findings.push_back({i.level == PageAccessIssue::Error ? AccessFinding::Error : i.level == PageAccessIssue::Warning ? AccessFinding::Warning : AccessFinding::Tip, i.message, {i.page, i.itemId},
                        i.kind == PageAccessIssue::NoTableHeader ? tr("Use the first row as a header") : QString()});
    setFindings(findings);
}

} // namespace jp
