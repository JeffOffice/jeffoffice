#include "app/pageaccesspane.h"

#include "app/editor.h"

namespace jp {

PageAccessibilityPane::PageAccessibilityPane(Editor *editor, bool slides, const std::function<QString(int)> &titleOf, QWidget *parent)
    : AccessibilityPane(parent), m_ed(editor), m_slides(slides), m_titleOf(titleOf)
{
    connect(this, &AccessibilityPane::recheckRequested, this, [this] { recheck(); });
    connect(this, &AccessibilityPane::goToRequested, this, [this](const QVariantList &where) {
        if (where.size() != 2) return;
        m_ed->setCurrentPage(where[0].toInt());
        const QString id = where[1].toString();
        if (!id.isEmpty() && m_ed->doc()->item(id)) m_ed->select(id);
    });
}

QVector<PageAccessIssue> PageAccessibilityPane::issues() const
{
    return m_ed->doc() ? checkPageAccessibility(*m_ed->doc(), m_slides, m_titleOf) : QVector<PageAccessIssue>();
}

void PageAccessibilityPane::recheck()
{
    QVector<AccessFinding> findings;
    for (const PageAccessIssue &i : issues())
        findings.push_back({i.level == PageAccessIssue::Error ? AccessFinding::Error : i.level == PageAccessIssue::Warning ? AccessFinding::Warning : AccessFinding::Tip, i.message, {i.page, i.itemId}});
    setFindings(findings);
}

} // namespace jp
