// JeffDeck's objects of other apps: Insert > Object (a range or chart of a workbook, a page of a document), Paste Special as an object, double-clicking one to edit it in its own app, and the
// list of the presentation's objects in File > Info. The picture, the package, and the box that asks where an object comes from are libs/shell/app/objects.h; the work on the presentation is
// io/pptxexport.h (addPageObject, pageObjectOf, setPageObject).

#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/editor.h"
#include "canvas/canvas.h"
#include "io/pptxexport.h"

#include <QApplication>
#include <QClipboard>
#include <QJsonArray>
#include <QJsonObject>
#include <QMimeData>
#include <QStatusBar>

namespace jp {

namespace {

office::ole::Kind kindOfSource(objects::Source source) { return source == objects::Source::Page ? office::ole::Kind::Page : office::ole::Kind::Sheet; }

// An object of the presentation: the slide it is on and the picture that shows it.
struct Placed {
    int slide = 0;
    QString itemId;
};

QVector<Placed> objectsOf(const Document &d)
{
    QVector<Placed> found;
    for (int s = 0; s < d.pages.size(); ++s) {
        const QJsonArray charts = d.pages[s]->props.value(QStringLiteral("charts")).toArray();
        for (const QJsonValue &v : charts) {
            const QJsonObject e = v.toObject();
            if (e.value(QStringLiteral("kind")).toString() == QLatin1String("object") && found.size() < office::ole::limits().objects) found.push_back({s, e.value(QStringLiteral("item")).toString()});
        }
    }
    return found;
}

// The entry of the slide's kept parts that names the object (a picture that shows an object has one), taken out.
void dropEntry(PageBase &page, const QString &itemId)
{
    QJsonArray charts = page.props.value(QStringLiteral("charts")).toArray(), kept;
    for (const QJsonValue &v : std::as_const(charts))
        if (!(v.toObject().value(QStringLiteral("kind")).toString() == QLatin1String("object") && v.toObject().value(QStringLiteral("item")).toString() == itemId)) kept.append(v);
    page.props[QStringLiteral("charts")] = kept;
}

} // namespace

void DeckWindow::createObjectActions()
{
    mk("insert.object.range", tr("From JeffCrunch Range…"), "table", QKeySequence(), [this] { insertObjectFromAsk(objects::Source::Range); });
    mk("insert.object.chart", tr("From JeffCrunch Chart…"), "chart-column", QKeySequence(), [this] { insertObjectFromAsk(objects::Source::Chart); });
    mk("insert.object.page", tr("From JeffWrite Page…"), "file-text", QKeySequence(), [this] { insertObjectFromAsk(objects::Source::Page); });
    mk("edit.pasteObject", tr("As Object"), "box", QKeySequence(), [this] {
        if (!insertObjectFromCopy()) statusBar()->showMessage(tr("What was copied can't be put in as an object."), 6000);
    });
    mk("file.objects", tr("Objects…"), "boxes", QKeySequence(), [this] { showObjects(); });
    // A double click on the picture of an object edits it in its own app.
    connect(m_canvas, &Canvas::pictureTabWanted, this, [this] {
        if (Item *one = m_ed->single(); one && m_ed->surface() && deck::pageObjectOf(*m_ed->doc(), *m_ed->surface(), one->id)) editObject(one->id);
    });
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [this] { refreshObjectUi(); });
}

void DeckWindow::refreshObjectUi()
{
    if (!m_ed || !m_ed->doc() || !m_actions.contains(QStringLiteral("edit.pasteObject"))) return;
    act("edit.pasteObject")->setEnabled(objects::hasObject(QApplication::clipboard()->mimeData()));
}

EmbeddedEdit *DeckWindow::objectEditor()
{
    if (!m_objectEdit) {
        m_objectEdit = new EmbeddedEdit(this);
        objects::usePictures(m_objectEdit);
        m_objectEdit->setOpener(objects::openInOwnApp);
        connect(m_objectEdit, &EmbeddedEdit::objectSaved, this, &DeckWindow::objectSaved);
    }
    return m_objectEdit;
}

bool DeckWindow::insertObjectFrom(objects::Source source, const QString &path, const QString &detail)
{
    QString why;
    std::optional<office::ole::EmbeddedObject> object;
    if (source == objects::Source::Range) object = objects::rangeObjectFromFile(path, detail, &why);
    else if (source == objects::Source::Chart) object = objects::chartObjectFromFile(path, &why);
    else object = objects::pageObjectFromFile(path, detail.toInt(), &why);
    if (!object) {
        statusBar()->showMessage(why, 8000);
        return false;
    }
    Document *d = m_ed->doc();
    if (!d || !m_ed->surface()) return false;
    // In the middle of the slide, no more than 60 of 100 of it either way, in the object's own shape.
    const QSizeF ps = d->pageSize();
    const double w = double(object->widthEmu) / office::ole::kEmuPerPoint, h = double(object->heightEmu) / office::ole::kEmuPerPoint;
    const double fit = std::min({1.0, ps.width() * 0.6 / std::max(w, 1.0), ps.height() * 0.6 / std::max(h, 1.0)});
    const QSizeF size(w * fit, h * fit);
    const QRectF rect(QPointF((ps.width() - size.width()) / 2, (ps.height() - size.height()) / 2), size);
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Insert Object"));
    const ItemPtr put = deck::addPageObject(*d, *m_ed->surface(), *object, rect);
    m_ed->endChange();
    if (!put) {
        statusBar()->showMessage(tr("This can't be put in the presentation as an object."), 8000);
        return false;
    }
    m_ed->select(put->id);
    return true;
}

bool DeckWindow::insertObjectFromCopy()
{
    const auto object = objects::objectOf(QApplication::clipboard()->mimeData());
    if (!object) return false;
    // (Through the file side's own path, so that both ways put an object in alike.)
    Document *d = m_ed->doc();
    if (!d || !m_ed->surface()) return false;
    const QSizeF ps = d->pageSize();
    const double w = double(object->widthEmu) / office::ole::kEmuPerPoint, h = double(object->heightEmu) / office::ole::kEmuPerPoint;
    const double fit = std::min({1.0, ps.width() * 0.6 / std::max(w, 1.0), ps.height() * 0.6 / std::max(h, 1.0)});
    const QSizeF size(w * fit, h * fit);
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Insert Object"));
    const ItemPtr put = deck::addPageObject(*d, *m_ed->surface(), *object, QRectF(QPointF((ps.width() - size.width()) / 2, (ps.height() - size.height()) / 2), size));
    m_ed->endChange();
    if (!put) return false;
    m_ed->select(put->id);
    return true;
}

void DeckWindow::insertObjectFromAsk(objects::Source source)
{
    const auto copied = objects::objectOf(QApplication::clipboard()->mimeData());
    const bool copiedFits = copied && copied->kind() == kindOfSource(source) && (source == objects::Source::Chart) == objects::packageHasChart(copied->partBytes);
    objects::SourceChoice choice;
    if (m_objectAsker) {
        if (!m_objectAsker(source, copiedFits, &choice)) return;
    } else if (!objects::askSource(this, source, copiedFits, &choice)) {
        return;
    }
    if (choice.copied && copiedFits) {
        insertObjectFromCopy();
        return;
    }
    insertObjectFrom(source, choice.path, choice.detail);
}

bool DeckWindow::editObject(const QString &itemId)
{
    Document *d = m_ed->doc();
    if (!d) return false;
    for (const Placed &p : objectsOf(*d)) {
        if (p.itemId != itemId) continue;
        const auto object = deck::pageObjectOf(*d, *d->pages[p.slide], itemId);
        if (!object) return false;
        if (!object->isOurs) {
            statusBar()->showMessage(tr("This object was made in another program, so it can't be edited here."), 8000);
            return false;
        }
        const int id = objectEditor()->start(this, itemId, *object);
        if (id == 0) {
            statusBar()->showMessage(object->kind() == office::ole::Kind::Page ? tr("JeffWrite can't be opened here, so this object can't be edited.") : tr("JeffCrunch can't be opened here, so this object can't be edited."), 8000);
            return false;
        }
        m_objectEdits.insert(id, itemId);
        statusBar()->showMessage(tr("Editing the object. Save it in its own window to update it here."), 8000);
        return true;
    }
    return false;
}

void DeckWindow::objectSaved(int editId, const QByteArray &part, const QByteArray &png, bool stale)
{
    Document *d = m_ed->doc();
    if (!d || !m_objectEdits.contains(editId)) return;
    const QString itemId = m_objectEdits.value(editId);
    for (const Placed &p : objectsOf(*d)) {
        if (p.itemId != itemId) continue;
        PageBase &page = *d->pages[p.slide];
        auto now = deck::pageObjectOf(*d, page, itemId);
        if (!now) return;
        office::ole::SavedObject saved;
        saved.partBytes = part;
        saved.previewPng = png;
        saved.pictureStale = stale;
        office::ole::applySave(*now, saved);
        m_ed->endTextEdit();
        m_ed->beginChange(tr("Update Object"));
        const bool put = deck::setPageObject(*d, page, itemId, *now);
        m_ed->endChange();
        statusBar()->showMessage(put ? tr("The object was updated.") : tr("The changes to the object could not be put in the presentation."), put ? 4000 : 8000);
        return;
    }
}

void DeckWindow::showObjects()
{
    Document *d = m_ed->doc();
    if (!d) return;
    objects::ListHost host;
    host.rows = [this, d] {
        QVector<objects::ObjectRow> rows;
        const QVector<Placed> placed = objectsOf(*d);
        for (int i = 0; i < placed.size(); ++i) {
            const auto o = deck::pageObjectOf(*d, *d->pages[placed[i].slide], placed[i].itemId);
            if (!o) continue;
            rows.push_back({i, tr("Slide %1, object %2").arg(placed[i].slide + 1).arg(i + 1), objects::kindWords(*o), objects::sizeWords(o->widthEmu, o->heightEmu), true});
        }
        return rows;
    };
    host.open = [this, d](int i) {
        const QVector<Placed> placed = objectsOf(*d);
        if (i >= 0 && i < placed.size()) editObject(placed[i].itemId);
    };
    host.convertToPicture = [this, d](int i) {
        const QVector<Placed> placed = objectsOf(*d);
        if (i < 0 || i >= placed.size()) return false;
        m_ed->endTextEdit();
        m_ed->beginChange(tr("Convert Object to Picture"));
        dropEntry(*d->pages[placed[i].slide], placed[i].itemId);
        m_ed->endChange();
        return true;
    };
    host.remove = [this, d](int i) {
        const QVector<Placed> placed = objectsOf(*d);
        if (i < 0 || i >= placed.size()) return false;
        const Placed p = placed[i];
        m_ed->endTextEdit();
        m_ed->beginChange(tr("Delete Object"));
        dropEntry(*d->pages[p.slide], p.itemId);
        ItemList &items = d->pages[p.slide]->items;
        for (int k = int(items.size()) - 1; k >= 0; --k)
            if (items[k]->id == p.itemId) items.erase(items.begin() + k);
        m_ed->endChange();
        return true;
    };
    objects::ObjectListDialog dialog(host, this);
    dialog.exec();
}

} // namespace jp
