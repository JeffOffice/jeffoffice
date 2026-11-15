#include "app/pagenumbers.h"

#include "app/editor.h"
#include "app/formatops.h"
#include "core/document.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <algorithm>

namespace jp {

namespace {

const char kName[] = "Page Number";
const char kRuleName[] = "Page Number Rule";

bool isDesignItem(const Item &it) { return it.name == QLatin1String(kName) || it.name == QLatin1String(kRuleName); }

bool isPageField(const QString &code) { return code == QLatin1String("page") || code.startsWith(QLatin1String("page:")) || code == QLatin1String("pages"); }

QTextCharFormat lookOf(const PageNumberType &type)
{
    QTextCharFormat f;
    if (type.bold) f.setFontWeight(QFont::Bold);
    if (type.size > 0) f.setFontPointSize(type.size);
    if (type.onAccent) formatops::setTextFill(f, ColorRef::rgb(Qt::white));
    return f;
}

// The design's words and fields at the cursor, in its look.
void putPieces(QTextCursor &c, const PageNumberDesign &d)
{
    const QTextCharFormat look = lookOf(pageNumberType(d));
    for (const PageNumberPiece &piece : pageNumberPieces(d)) {
        QTextCharFormat f = look;
        if (piece.kind == PageNumberPiece::Text) {
            c.insertText(piece.text, f);
        } else {
            f.setProperty(tp::Field, piece.kind == PageNumberPiece::Page ? QStringLiteral("page") : QStringLiteral("pages"));
            c.insertText(QString(QChar::ObjectReplacementCharacter), f);
        }
    }
}

Qt::Alignment alignmentOf(PageNumberSide side) { return side == PageNumberSide::Left ? Qt::AlignLeft : side == PageNumberSide::Right ? Qt::AlignRight : Qt::AlignHCenter; }

} // namespace

bool pubOffersPageNumberDesign(const PageNumberDesign &d) { return d.area != PageNumberArea::Cursor || d.look != PageNumberLook::Circle; }

QColor pubPageNumberAccent(const Editor *ed)
{
    if (!ed || !ed->doc()) return QColor(0x2F, 0x55, 0x97);
    return ColorRef::scheme(Accent1).resolve(ed->doc()->colors);
}

bool placePageNumberDesign(Editor *ed, const QString &id)
{
    const std::optional<PageNumberDesign> design = pageNumberDesignById(id);
    if (!ed || !design) return false;
    if (design->area == PageNumberArea::Cursor) {
        if (!ed->isEditingText() || !pubOffersPageNumberDesign(*design)) return false;
        ed->editTextAs(QCoreApplication::translate("jp::PageNumbers", "Page Number"), [&](QTextCursor &c) { putPieces(c, *design); });
        return true;
    }
    Document *d = ed->doc();
    if (!d || d->pages.isEmpty() || d->masters.isEmpty()) return false;
    const int at = std::clamp(ed->currentPage(), 0, int(d->pages.size()) - 1);
    MasterPage *master = d->masterFor(*d->pages[at]);
    if (!master) master = d->masters.first().get();
    const QSizeF page = d->pageSize();
    const QMarginsF margins = d->setup.margins;
    const bool top = design->area == PageNumberArea::Top;
    ed->change(QCoreApplication::translate("jp::PageNumbers", "Page Number"), [&] {
        auto &items = master->items;
        items.erase(std::remove_if(items.begin(), items.end(), [](const ItemPtr &it) { return isDesignItem(*it); }), items.end());
        const QRectF content = QRectF(QPointF(0, 0), page).marginsRemoved(margins);
        const PageNumberType type = pageNumberType(*design);
        if (pageNumberHasBox(*design)) {
            const PageNumberBox box = pageNumberBox(*design, page, margins);
            auto shape = std::make_shared<ShapeItem>();
            shape->name = QString::fromLatin1(kName);
            shape->shape = box.preset;
            shape->fill = Fill::solid(ColorRef::scheme(Accent1));
            shape->stroke = Stroke::none();
            shape->storyId = d->createStory();
            shape->insets = QMarginsF(0, 0, 0, 0);
            shape->valign = VAlign::Middle;
            shape->noWrap = true;
            if (box.turned) {
                // (Laid on its side: the frame is the box turned back, so that its words read upward.)
                const QPointF center = box.rect.center();
                shape->rect = QRectF(center.x() - box.rect.height() / 2, center.y() - box.rect.width() / 2, box.rect.height(), box.rect.width());
                shape->rotation = 270;
            } else {
                shape->rect = box.rect;
            }
            QTextCursor c(d->storyDoc(shape->storyId));
            QTextBlockFormat bf;
            bf.setAlignment(Qt::AlignHCenter);
            c.setBlockFormat(bf);
            putPieces(c, *design);
            items.push_back(shape);
            return;
        }
        // A box of text across the page's text, with the rule that goes with the design above or below it.
        const double height = std::max(22.0, type.size * 1.3);
        const double y = top ? std::max(6.0, margins.top() / 2 - height / 2) : std::min(page.height() - height - 6, page.height() - margins.bottom() / 2 - height / 2);
        auto box = std::static_pointer_cast<TextItem>(ed->newTextBox(QRectF(content.left(), y, content.width(), height)));
        box->name = QString::fromLatin1(kName);
        box->valign = VAlign::Middle;
        QTextCursor c(d->storyDoc(box->storyId));
        QTextBlockFormat bf;
        bf.setAlignment(alignmentOf(design->side));
        c.setBlockFormat(bf);
        putPieces(c, *design);
        items.push_back(box);
        const PageNumberRule rule = pageNumberRule(*design);
        if (rule != PageNumberRule::None) {
            auto line = std::make_shared<ShapeItem>();
            line->name = QString::fromLatin1(kRuleName);
            line->shape = QStringLiteral("rect");
            line->fill = Fill::solid(ColorRef::scheme(Main));
            line->stroke = Stroke::none();
            line->rect = QRectF(content.left(), rule == PageNumberRule::Above ? y - 2 : y + height + 1, content.width(), 0.75);
            items.push_back(line);
        }
    });
    return true;
}

int removePageNumbers(Editor *ed)
{
    Document *d = ed ? ed->doc() : nullptr;
    if (!d) return 0;
    int count = 0;
    ed->change(QCoreApplication::translate("jp::PageNumbers", "Remove Page Numbers"), [&] {
        for (const auto &master : d->masters) {
            auto &items = master->items;
            const auto gone = std::remove_if(items.begin(), items.end(), [](const ItemPtr &it) { return isDesignItem(*it); });
            count += int(items.end() - gone);
            items.erase(gone, items.end());
            for (const ItemPtr &it : items) {
                QString storyId;
                if (it->type() == ItemType::Text) storyId = static_cast<TextItem *>(it.get())->storyId;
                else if (it->type() == ItemType::Shape) storyId = static_cast<ShapeItem *>(it.get())->storyId;
                QTextDocument *story = storyId.isEmpty() ? nullptr : d->storyDoc(storyId);
                if (!story) continue;
                QVector<int> at;
                for (int i = 0; i < story->characterCount() - 1; ++i) {
                    if (story->characterAt(i) != QChar::ObjectReplacementCharacter) continue;
                    QTextCursor probe(story);
                    probe.setPosition(i + 1);
                    if (isPageField(probe.charFormat().stringProperty(tp::Field))) at << i;
                }
                for (int i = int(at.size()) - 1; i >= 0; --i) {
                    QTextCursor c(story);
                    c.setPosition(at[i]);
                    c.setPosition(at[i] + 1, QTextCursor::KeepAnchor);
                    c.removeSelectedText();
                    ++count;
                }
            }
        }
    });
    return count;
}

} // namespace jp
