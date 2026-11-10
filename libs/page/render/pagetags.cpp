#include "render/pagetags.h"

namespace jp {

PageTagger::PageTagger(PdfTagger *tags, std::function<bool(const Item &)> isTitle) : m_tags(tags), m_isTitle(std::move(isTitle)) {}

void PageTagger::install(PaintContext *ctx)
{
    ctx->mark = [this](QPainter *p, const Item &it, bool opens) { mark(p, it, opens); };
    ctx->markCell = [this](QPainter *p, const TableItem &t, int row, int col, bool opens) { markCell(p, t, row, col, opens); };
}

PdfTag PageTagger::tagOf(const Item &it) const
{
    PdfTag tag;
    const bool title = m_isTitle && m_isTitle(it);
    tag.type = QStringLiteral("Artifact");
    if (it.decorative) return tag;
    switch (it.type()) {
    case ItemType::Text: tag.type = title ? QStringLiteral("H1") : QStringLiteral("P"); break;
    case ItemType::Shape:
        if (it.hasText()) tag.type = title ? QStringLiteral("H1") : QStringLiteral("P");
        else if (!it.altText.trimmed().isEmpty()) {
            tag.type = QStringLiteral("Figure");
            tag.alt = it.altText.trimmed();
        }
        break;
    case ItemType::Picture:
        tag.type = QStringLiteral("Figure");
        tag.alt = it.altText.trimmed();
        break;
    case ItemType::TextArt:
        tag.type = QStringLiteral("Figure");
        tag.alt = it.altText.trimmed().isEmpty() ? static_cast<const TextArtItem &>(it).text : it.altText.trimmed();
        break;
    case ItemType::Group:
        tag.type = it.altText.trimmed().isEmpty() ? QStringLiteral("Div") : QStringLiteral("Figure");
        tag.alt = it.altText.trimmed();
        break;
    case ItemType::Line:
    case ItemType::Table: break;
    }
    return tag;
}

void PageTagger::mark(QPainter *p, const Item &it, bool opens)
{
    if (!opens) {
        if (m_open >= 0) m_tags->end(p, m_open);
        m_open = -1;
        m_cells.clear();
        return;
    }
    m_cells.clear();
    if (it.type() == ItemType::Table && !it.decorative) {
        // A table is its rows and their cells; each cell holds what the table paints in it.
        const auto &t = static_cast<const TableItem &>(it);
        PdfTag table;
        table.type = QStringLiteral("Table");
        table.alt = t.altText.trimmed();
        table.content = false;
        table.parent = m_parent;
        const int tableTag = m_tags->add(table);
        for (int row = 0; row < t.rows; ++row) {
            PdfTag tr;
            tr.type = QStringLiteral("TR");
            tr.parent = tableTag;
            tr.content = false;
            const int rowTag = m_tags->add(tr);
            for (int col = 0; col < t.cols; ++col) {
                if (t.cell(row, col).covered) continue;
                PdfTag td;
                td.type = (t.header && row == 0) || (t.firstCol && col == 0) ? QStringLiteral("TH") : QStringLiteral("TD");
                td.parent = rowTag;
                m_cells.insert(row * t.cols + col, m_tags->add(td));
            }
        }
        m_open = -1;
        return;
    }
    PdfTag tag = tagOf(it);
    tag.parent = m_parent;
    m_open = m_tags->add(tag);
    m_tags->begin(p, m_open);
}

void PageTagger::markCell(QPainter *p, const TableItem &table, int row, int col, bool opens)
{
    const auto at = m_cells.constFind(row * table.cols + col);
    if (at == m_cells.cend()) return;
    if (opens) m_tags->begin(p, at.value());
    else m_tags->end(p, at.value());
}

} // namespace jp
