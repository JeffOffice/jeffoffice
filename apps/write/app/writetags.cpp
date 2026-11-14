#include "app/writetags.h"

#include "core/writeprops.h"

#include <QPainter>
#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>

namespace jp::write {

WriteTagger::WriteTagger() : m_page(&m_tags) {}

void WriteTagger::beginPage()
{
    m_section = m_page.beginSection();
    m_list = -1;
    m_blocks.clear();
}

void WriteTagger::install(PaintContext *ctx, const flow::MaterializedPage &page)
{
    m_current = &page;
    m_columns.clear();
    for (const QString &id : page.boxes)
        if (const Item *box = page.doc->item(id)) m_columns.push_back(box->rect);
    ctx->mark = [this](QPainter *p, const Item &item, bool opens) { markItem(p, item, opens); };
    ctx->markBlock = [this](QPainter *p, const Item &item, int position, double top, bool opens) { markBlock(p, item, position, top, opens); };
    ctx->markInline = [this](QPainter *p, const Item &item, bool opens) {
        if (!opens) {
            if (m_inlineArtifact) m_page.artifactEnd(p);
            else if (m_inline >= 0) m_tags.end(p, m_inline);
            m_inlineArtifact = false;
            m_inline = -1;
            return;
        }
        PdfTag tag = m_page.tagOf(item);
        tag.parent = m_section;
        tag.order = readingOrder(m_bodyLeft, m_bodyTop + item.rect.top());
        if (tag.type == QLatin1String("Artifact")) {
            m_inlineArtifact = true;
            m_page.artifactBegin(p);
            return;
        }
        m_inline = m_tags.add(tag);
        m_tags.begin(p, m_inline);
    };
    ctx->markArtifact = [this](QPainter *p, bool opens) {
        if (opens) m_page.artifactBegin(p);
        else m_page.artifactEnd(p);
    };
}

void WriteTagger::install(flow::TablePaint *tables)
{
    tables->markTable = [this](QPainter *p, const flow::TableFragment &table, bool opens) { markTable(p, table, opens); };
    tables->markCell = [this](QPainter *p, const flow::TableFragment &table, int cell, bool opens) { markCell(p, table, cell, opens); };
    tables->markArtifact = [this](QPainter *p, bool opens) {
        if (opens) m_page.artifactBegin(p);
        else m_page.artifactEnd(p);
    };
}

void WriteTagger::markItem(QPainter *p, const Item &item, bool opens)
{
    if (!opens) {
        if (m_artifact) m_page.artifactEnd(p);
        else if (m_open >= 0) m_tags.end(p, m_open);
        m_artifact = false;
        m_open = -1;
        m_body = false;
        return;
    }
    m_body = false;
    const QString &id = item.id;
    // A column of the page's text is tagged paragraph by paragraph (markBlock).
    if (id.startsWith(QLatin1String("flow-box-"))) {
        m_body = true;
        m_bodyTop = item.rect.top();
        m_bodyLeft = item.rect.left() + 1;
        return;
    }
    // The objects set on the page and the notes are tagged as the page layer tags objects; the rest the page makes (headers and footers, the page's color and borders, the lines
    // between columns, line numbers, and the walls text is set round) is decoration.
    const bool own = id.startsWith(QLatin1String("flow-object-")) || id.startsWith(QLatin1String("flow-boxtext-")) || id.startsWith(QLatin1String("flow-note-box-")) || !id.startsWith(QLatin1String("flow-"));
    PdfTag tag;
    tag.type = QStringLiteral("Artifact");
    if (own) tag = m_page.tagOf(item);
    tag.parent = m_section;
    tag.order = readingOrder(item.rect.center().x(), item.rect.top());
    m_list = -1;
    if (tag.type == QLatin1String("Artifact")) {
        m_artifact = true;
        m_page.artifactBegin(p);
        return;
    }
    m_open = m_tags.add(tag);
    m_tags.begin(p, m_open);
}

void WriteTagger::markBlock(QPainter *p, const Item &item, int position, double top, bool opens)
{
    if (!m_body) return;
    if (!opens) {
        if (m_open >= 0) m_tags.end(p, m_open);
        m_open = -1;
        return;
    }
    if (item.type() != ItemType::Text || !m_current) return;
    const auto found = m_blocks.constFind(position);
    if (found != m_blocks.cend()) {
        m_open = found.value();
        m_tags.begin(p, m_open);
        return;
    }
    const auto &text = static_cast<const TextItem &>(item);
    const QVector<TextItem *> chain = m_current->doc->chainOf(text.id);
    const Story *story = m_current->doc->story(chain.isEmpty() ? text.storyId : chain.first()->storyId);
    const QTextBlock block = story && story->doc ? story->doc->findBlock(position) : QTextBlock();
    const QTextBlockFormat format = block.isValid() ? block.blockFormat() : QTextBlockFormat();
    const bool heading = format.hasProperty(prop::Outline) && format.intProperty(prop::Outline) < 9 && !format.hasProperty(prop::EndnoteBlock) && !format.hasProperty(prop::RawBlock);
    PdfTag tag;
    tag.parent = m_section;
    tag.order = readingOrder(m_bodyLeft, m_bodyTop + top);
    if (heading) {
        m_list = -1;
        tag.type = QStringLiteral("H%1").arg(std::clamp(format.intProperty(prop::Outline) + 1, 1, 6));
    } else if (format.hasProperty(prop::NumId)) {
        // The paragraphs of a list in a row are the items of one list.
        if (m_list < 0) {
            PdfTag list;
            list.type = QStringLiteral("L");
            list.content = false;
            list.parent = m_section;
            list.order = tag.order;
            m_list = m_tags.add(list);
        }
        PdfTag item;
        item.type = QStringLiteral("LI");
        item.content = false;
        item.parent = m_list;
        tag.type = QStringLiteral("LBody");
        tag.parent = m_tags.add(item);
    } else {
        m_list = -1;
        tag.type = QStringLiteral("P");
    }
    m_open = m_tags.add(tag);
    m_blocks.insert(position, m_open);
    m_tags.begin(p, m_open);
}

double WriteTagger::readingOrder(double x, double y) const
{
    int column = 0;
    for (int i = 0; i < m_columns.size(); ++i)
        if (x >= m_columns[i].left() - 1 && x <= m_columns[i].right() + 1) column = i;
    return column * 1e6 + y;
}

void WriteTagger::markTable(QPainter *p, const flow::TableFragment &fragment, bool opens)
{
    if (!opens) {
        if (m_tableArtifact) m_page.artifactEnd(p);
        m_tableArtifact = false;
        m_table = -1;
        m_rows.clear();
        m_cells.clear();
        return;
    }
    PdfTag table;
    table.type = QStringLiteral("Table");
    table.content = false;
    table.parent = m_section;
    table.order = readingOrder(fragment.origin.x() + 1, fragment.origin.y());
    m_table = m_tags.add(table);
    m_list = -1;
    m_rows.clear();
    m_cells.clear();
    // what the table draws but for its cells' text (its fills and borders) is decoration
    m_tableArtifact = true;
    m_page.artifactBegin(p);
}

void WriteTagger::markCell(QPainter *p, const flow::TableFragment &table, int cell, bool opens)
{
    if (m_table < 0) return;
    if (!opens) {
        const auto at = m_cells.constFind(cell);
        if (at != m_cells.cend()) m_tags.end(p, at.value());
        if (m_tableArtifact) m_page.artifactBegin(p);
        return;
    }
    const flow::TableLayout::Cell &c = table.layout->cell(cell);
    int row = m_rows.value(c.row, -1);
    if (row < 0) {
        PdfTag tr;
        tr.type = QStringLiteral("TR");
        tr.content = false;
        tr.parent = m_table;
        row = m_tags.add(tr);
        m_rows.insert(c.row, row);
    }
    int tag = m_cells.value(cell, -1);
    if (tag < 0) {
        PdfTag td;
        const auto *data = table.layout->row(c.row).data;
        td.type = data && data->header ? QStringLiteral("TH") : QStringLiteral("TD");
        td.parent = row;
        tag = m_tags.add(td);
        m_cells.insert(cell, tag);
    }
    // (the decoration is paused while a cell's text is drawn)
    if (m_tableArtifact) m_page.artifactEnd(p);
    m_tags.begin(p, tag);
}

} // namespace jp::write
