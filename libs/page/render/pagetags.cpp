#include "render/pagetags.h"

#include "text/textprops.h"

#include <QTextBlock>
#include <QTextDocument>

namespace jp {

PageTagger::PageTagger(PdfTagger *tags, std::function<bool(const Item &)> isTitle) : m_tags(tags), m_isTitle(std::move(isTitle)) {}

void PageTagger::install(PaintContext *ctx)
{
    ctx->mark = [this](QPainter *p, const Item &it, bool opens) { mark(p, it, opens); };
    ctx->markCell = [this](QPainter *p, const TableItem &t, int row, int col, bool opens) { markCell(p, t, row, col, opens); };
    ctx->markArtifact = [this](QPainter *p, bool opens) {
        if (opens) artifactBegin(p);
        else artifactEnd(p);
    };
}

int PageTagger::beginSection()
{
    PdfTag sect;
    sect.type = QStringLiteral("Sect");
    sect.content = false;
    m_parent = m_tags->add(sect);
    return m_parent;
}

void PageTagger::artifactBegin(QPainter *p)
{
    if (m_artifact >= 0) return;
    PdfTag tag;
    tag.type = QStringLiteral("Artifact");
    m_artifact = m_tags->add(tag);
    m_tags->begin(p, m_artifact);
}

void PageTagger::artifactEnd(QPainter *p)
{
    if (m_artifact < 0) return;
    m_tags->end(p, m_artifact);
    m_artifact = -1;
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
    case ItemType::Equation:
        tag.type = QStringLiteral("Formula");
        tag.alt = it.altText.trimmed().isEmpty() ? static_cast<const EquationItem &>(it).linear : it.altText.trimmed();
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
        if (m_tableArtifact) artifactEnd(p);
        m_tableArtifact = false;
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
        // what the table draws but for its cells' text (its fills and borders) is decoration
        m_tableArtifact = true;
        artifactBegin(p);
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
    if (opens) {
        // (the decoration is paused while a cell's text is drawn)
        if (m_tableArtifact) artifactEnd(p);
        m_tags->begin(p, at.value());
    } else {
        m_tags->end(p, at.value());
        if (m_tableArtifact) artifactBegin(p);
    }
}

QString documentLanguage(const Document &doc, const QString &fallback)
{
    QHash<QString, qint64> letters;
    for (const QTextDocument *sd : doc.storyDocs()) {
        if (!sd) continue;
        for (QTextBlock block = sd->begin(); block.isValid(); block = block.next())
            for (auto frag = block.begin(); !frag.atEnd(); ++frag) {
                int n = 0;
                for (const QChar c : frag.fragment().text()) n += c.isLetter();
                if (!n) continue;
                QString lang = frag.fragment().charFormat().stringProperty(tp::Language).trimmed();
                if (lang.isEmpty()) lang = fallback;
                letters[lang.replace(QLatin1Char('_'), QLatin1Char('-'))] += n;
            }
    }
    QString best = fallback;
    qint64 most = 0;
    for (auto it = letters.cbegin(); it != letters.cend(); ++it)
        if (it.value() > most || (it.value() == most && it.key() < best)) {
            most = it.value();
            best = it.key();
        }
    return best;
}

} // namespace jp
