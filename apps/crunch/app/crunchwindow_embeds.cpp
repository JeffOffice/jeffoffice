// JeffCrunch's embedded objects: Insert > Object (a range or chart of a workbook, a page of a document), Paste Special as an object, copy and paste of an object, double-clicking one to edit it
// in its own app, and the list of the workbook's objects in File > Info. An object is its package with its picture (core/sheetembeds.h), not a picture: the sheet moves, sizes, saves, and undoes
// it as one step each. The picture and the box that asks where an object comes from are libs/shell/app/objects.h.

#include "app/crunchwindow.h"

#include "core/objectclip.h"
#include "core/sheetobjects.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QMimeData>
#include <QUndoStack>

namespace jp {

using namespace jp::crunch;

namespace {

office::ole::Kind kindOfSource(objects::Source source) { return source == objects::Source::Page ? office::ole::Kind::Page : office::ole::Kind::Sheet; }

// What a copy of an object of this workbook carries beside what the other apps paste as an object: this app puts it back as an object of its own.
constexpr char kEmbedMime[] = "application/x-jeffcrunch-embedded-object";

constexpr double kEmuPerUnit = 9525;   // the grid's unit is a pixel at 100%

} // namespace

void CrunchWindow::createObjectActions()
{
    mk("insert.object.range", tr("From JeffCrunch Range…"), "table", QKeySequence(), [this] { insertObjectFromAsk(objects::Source::Range); });
    mk("insert.object.chart", tr("From JeffCrunch Chart…"), "chart-column", QKeySequence(), [this] { insertObjectFromAsk(objects::Source::Chart); });
    mk("insert.object.page", tr("From JeffWrite Page…"), "file-text", QKeySequence(), [this] { insertObjectFromAsk(objects::Source::Page); });
    mk("edit.pasteObject", tr("As Object"), "box", QKeySequence(), [this] {
        if (!insertObjectFromCopy()) setStatus(tr("What was copied can't be put in as an object."));
    });
    mk("file.objects", tr("Objects…"), "boxes", QKeySequence(), [this] { showObjects(); });
    for (const char *id : {"insert.object.range", "insert.object.chart", "insert.object.page"}) m_opsActions << QString::fromLatin1(id);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [this] {
        if (QAction *a = act(QStringLiteral("edit.pasteObject"))) a->setEnabled(m_session && objects::hasObject(QApplication::clipboard()->mimeData()));
    });
}

jp::EmbeddedEdit *CrunchWindow::objectEditor()
{
    if (!m_objectEdit) {
        m_objectEdit = new jp::EmbeddedEdit(this);
        objects::usePictures(m_objectEdit);
        m_objectEdit->setOpener(objects::openInOwnApp);
        connect(m_objectEdit, &jp::EmbeddedEdit::objectSaved, this, &CrunchWindow::objectSaved);
    }
    return m_objectEdit;
}

// The object on the sheet at the cursor (or where `at` says), as one step of Undo; the id it has, or -1 with the reason told.
int CrunchWindow::placeObject(const office::ole::EmbeddedObject &object, const QString &step, const office::sheet::DrawingAnchor *at)
{
    if (!m_session || !m_objects || m_chartSheetShown) return -1;
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return -1;
    // the size it is shown at: its own, no larger than 560 by 420 of the grid's units
    double w = std::max(24.0, double(object.widthEmu) / kEmuPerUnit), h = std::max(24.0, double(object.heightEmu) / kEmuPerUnit);
    const double fit = std::min({1.0, 560.0 / w, 420.0 / h});
    w *= fit;
    h *= fit;
    const office::sheet::DrawingAnchor anchor = at ? *at : newObjectAnchor(w, h);
    QString why;
    const int id = m_objects->embeds()->make(m_sheet, object, anchor, &why);
    if (id < 0) {
        notice(step, why);
        return -1;
    }
    crunch::ObjectEdit e;
    e.sheet = m_sheet;
    e.id = id;
    e.before = m_objects->stateOf(m_sheet, id);
    e.after = e.before;
    e.after.deleted = false;
    const crunch::EditSession::Outcome o = m_session->changeObjects({e}, step);
    if (!o.ok) {
        m_objects->embeds()->discard(m_sheet, id);
        notice(step, o.problem);
        return -1;
    }
    m_objects->layer()->select(id);
    m_view->setFocus(Qt::OtherFocusReason);
    return id;
}

bool CrunchWindow::insertObjectFrom(objects::Source source, const QString &path, const QString &detail)
{
    QString why;
    std::optional<office::ole::EmbeddedObject> object;
    if (source == objects::Source::Range) object = objects::rangeObjectFromFile(path, detail, &why);
    else if (source == objects::Source::Chart) object = objects::chartObjectFromFile(path, &why);
    else object = objects::pageObjectFromFile(path, detail.toInt(), &why);
    if (!object) {
        setStatus(why);
        return false;
    }
    if (placeObject(*object, tr("Insert Object")) < 0) return false;
    setStatus(tr("Object inserted"));
    return true;
}

bool CrunchWindow::insertObjectFromCopy()
{
    const auto object = objects::objectOf(QApplication::clipboard()->mimeData());
    if (!object || placeObject(*object, tr("Paste Object")) < 0) return false;
    setStatus(tr("Object inserted"));
    return true;
}

void CrunchWindow::insertObjectFromAsk(objects::Source source)
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

// Copy and Cut of the picked object: the other apps paste it as an object, and this one puts it back as an object of its own (its package copied, so that a change to one is no change to the other).
bool CrunchWindow::copyEmbed(int id, bool cut)
{
    if (!m_session || !m_objects || (cut && !editingAllowed())) return false;
    const auto object = m_objects->embeds()->copyOf(m_sheet, id);
    if (!object) return false;
    if (!object->isOurs) {
        setStatus(tr("This object was made in another program, so it can't be copied here."));
        return true;
    }
    auto *md = new QMimeData;
    md->setData(QLatin1String(kEmbedMime), objects::encode(*object));
    const QImage picture = SheetEmbeds::previewImage(object->previewPng);
    if (!picture.isNull()) md->setImageData(picture);
    md = static_cast<QMimeData *>(objects::withObject(md, [object]() -> std::optional<office::ole::EmbeddedObject> { return object; }));
    m_settingClipboard = true;
    QApplication::clipboard()->setMimeData(md);
    m_settingClipboard = false;
    m_clip = crunch::ClipData();
    m_view->clearCopyBorder();
    if (cut) deleteObject(id);
    setStatus(cut ? tr("Object cut") : tr("Object copied"));
    return true;
}

bool CrunchWindow::pasteEmbed()
{
    const QMimeData *md = QApplication::clipboard()->mimeData();
    if (!md || !md->hasFormat(QLatin1String(kEmbedMime))) return false;
    const auto object = objects::decode(md->data(QLatin1String(kEmbedMime)));
    if (!object) return false;
    if (!m_session || !m_objects || m_chartSheetShown) return true;
    placeObject(*object, tr("Paste Object"));
    return true;
}

bool CrunchWindow::editObject(int sheet, int id)
{
    if (!m_objects) return false;
    const auto object = m_objects->embeds()->objectOf(sheet, id);
    if (!object || m_objects->embeds()->isGone(sheet, id)) return false;
    if (!object->isOurs) {
        setStatus(tr("This object was made in another program, so it can't be edited here."));
        return false;
    }
    // (the picture is made again at the size the object has on the sheet)
    office::ole::EmbeddedObject sized = *object;
    m_objects->embeds()->sizeOf(sheet, id, &sized.widthEmu, &sized.heightEmu);
    const int edit = objectEditor()->start(this, QStringLiteral("%1:%2").arg(sheet).arg(id), sized);
    if (edit == 0) {
        setStatus(object->kind() == office::ole::Kind::Page ? tr("JeffWrite can't be opened here, so this object can't be edited.") : tr("JeffCrunch can't be opened here, so this object can't be edited."));
        return false;
    }
    m_objectEdits.insert(edit, {sheet, id});
    setStatus(tr("Editing the object. Save it in its own window to update it here."));
    return true;
}

// The app that made the object saved it: the new package and picture are put in the object as one step. (A cell being typed is left as it is: the step is made beside it.)
void CrunchWindow::objectSaved(int editId, const QByteArray &part, const QByteArray &png, bool stale)
{
    if (!m_objectEdits.contains(editId) || !m_session || !m_objects) return;
    const EmbedEditing target = m_objectEdits.value(editId);
    const auto now = m_objects->embeds()->objectOf(target.sheet, target.id);
    if (!now || m_objects->embeds()->isGone(target.sheet, target.id)) return;
    office::ole::EmbeddedObject object = *now;
    office::ole::SavedObject saved;
    saved.partBytes = part;
    saved.previewPng = png;
    saved.pictureStale = stale;
    office::ole::applySave(object, saved);
    crunch::ObjectEdit e;
    e.sheet = target.sheet;
    e.id = target.id;
    e.before = m_objects->stateOf(target.sheet, target.id);
    e.after = m_objects->stateWithEmbed(target.sheet, target.id, object);
    const crunch::EditSession::Outcome o = m_session->changeObjects({e}, tr("Update Object"));
    setStatus(o.ok ? tr("The object was updated.") : tr("The changes to the object could not be put in the workbook."));
}

void CrunchWindow::showObjects()
{
    if (!m_session || !m_objects || !m_loaded || !m_loaded->book) return;
    struct Row {
        int sheet = 0;
        int id = 0;
    };
    auto list = std::make_shared<std::vector<Row>>();
    objects::ListHost host;
    host.rows = [this, list] {
        list->clear();
        QVector<objects::ObjectRow> rows;
        const auto &sheets = m_loaded->book->info().sheets;
        for (int s = 0; s < sheets.size(); ++s) {
            if (sheets[s].deleted) continue;
            for (const int id : m_objects->embeds()->idsOf(s)) {
                const auto o = m_objects->embeds()->objectOf(s, id);
                if (!o) continue;
                qint64 w = o->widthEmu, h = o->heightEmu;
                m_objects->embeds()->sizeOf(s, id, &w, &h);
                rows.push_back({int(list->size()), tr("%1, object %2").arg(sheets[s].name).arg(int(list->size()) + 1), objects::kindWords(*o), objects::sizeWords(w, h), o->isOurs && !o->previewPng.isEmpty()});
                list->push_back({s, id});
            }
        }
        return rows;
    };
    host.open = [this, list](int i) {
        if (i >= 0 && i < int(list->size())) editObject((*list)[size_t(i)].sheet, (*list)[size_t(i)].id);
    };
    host.convertToPicture = [this, list](int i) {
        if (i < 0 || i >= int(list->size()) || !editingAllowed()) return false;
        const Row r = (*list)[size_t(i)];
        const auto o = m_objects->embeds()->objectOf(r.sheet, r.id);
        if (!o || o->previewPng.isEmpty()) return false;
        if (m_sheet != r.sheet) showSheet(r.sheet, true);
        const office::sheet::DrawingAnchor anchor = m_objects->embeds()->anchorOf(r.sheet, r.id);
        ObjectClip clip;
        if (!pictureClip(o->previewPng, tr("Picture"), 0, 0, objects::kindWords(*o), &clip)) return false;
        QUndoStack *stack = m_session->undoStack();
        stack->beginMacro(tr("Convert Object to Picture"));
        deleteObject(r.id);
        const bool put = m_objects->embeds()->isDeleted(r.sheet, r.id) && placeObjectClip(clip, tr("Convert Object to Picture"), &anchor);
        stack->endMacro();
        if (!put) stack->undo();
        return put;
    };
    host.remove = [this, list](int i) {
        if (i < 0 || i >= int(list->size()) || !editingAllowed()) return false;
        const Row r = (*list)[size_t(i)];
        if (m_sheet != r.sheet) showSheet(r.sheet, true);
        deleteObject(r.id);
        return m_objects->embeds()->isDeleted(r.sheet, r.id);
    };
    objects::ObjectListDialog dialog(host, this);
    dialog.exec();
}

} // namespace jp
