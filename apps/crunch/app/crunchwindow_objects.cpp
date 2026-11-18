// JeffCrunch's objects of other apps: Insert > Object (a page of a document), Paste Special as an object, double-clicking one to edit it in its own app, and the list of the workbook's
// objects in File > Info. An object is a picture that carries its package (libs/shell/app/objects.h, carryObject): the picture is the object's own, so the sheet moves, sizes, saves, and
// undoes it as it does any picture, and it is still the object when the workbook is opened again. What an edit's save changes is the picture's bytes, one step.

#include "app/crunchwindow.h"

#include "core/objectclip.h"
#include "core/sheetobjects.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QStatusBar>

namespace jp {

using namespace jp::crunch;

namespace {

office::ole::Kind kindOfSource(objects::Source source) { return source == objects::Source::Page ? office::ole::Kind::Page : office::ole::Kind::Sheet; }

} // namespace

void CrunchWindow::createObjectActions()
{
    mk("insert.object.page", tr("From JeffWrite Page…"), "file-text", QKeySequence(), [this] { insertObjectFromAsk(objects::Source::Page); });
    mk("edit.pasteObject", tr("As Object"), "box", QKeySequence(), [this] {
        if (!insertObjectFromCopy()) setStatus(tr("What was copied can't be put in as an object."));
    });
    mk("file.objects", tr("Objects…"), "boxes", QKeySequence(), [this] { showObjects(); });
    m_opsActions << QStringLiteral("insert.object.page");
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
        connect(m_objectEdit, &jp::EmbeddedEdit::objectSaved, this, &CrunchWindow::carriedObjectSaved);
    }
    return m_objectEdit;
}

bool CrunchWindow::placeCarried(const office::ole::EmbeddedObject &object)
{
    if (!m_session || !m_objects || m_chartSheetShown || !editingAllowed()) return false;
    const QByteArray png = objects::carryObject(object);
    if (png.isEmpty() || png.size() > kMaxClipMediaBytes) {
        notice(tr("Insert Object"), tr("This is too large to put on a sheet as an object."));
        return false;
    }
    ObjectClip clip;
    if (!pictureClip(png, tr("Object"), 0, 0, objects::kindWords(object), &clip)) return false;
    if (!placeObjectClip(clip, tr("Insert Object"))) return false;
    setStatus(tr("Object inserted"));
    return true;
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
    return placeCarried(*object);
}

bool CrunchWindow::insertObjectFromCopy()
{
    const auto object = objects::objectOf(QApplication::clipboard()->mimeData());
    return object && placeCarried(*object);
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

std::optional<objects::Carried> CrunchWindow::carriedAt(int sheet, int id)
{
    if (!m_objects || m_objects->kindOf(sheet, id) != office::sheet::ObjectKind::Picture) return std::nullopt;
    ObjectClip clip;
    QString why;
    if (!m_objects->copyElement(sheet, id, &clip, &why) || clip.media.size() != 1) return std::nullopt;
    return objects::carriedObject(clip.media[0].bytes);
}

std::vector<CrunchWindow::CarriedObject> CrunchWindow::carriedObjects()
{
    std::vector<CarriedObject> found;
    if (!m_objects || !m_loaded || !m_loaded->book) return found;
    const auto &sheets = m_loaded->book->info().sheets;
    for (int s = 0; s < sheets.size() && int(found.size()) < office::ole::limits().objects; ++s) {
        if (sheets[s].deleted) continue;
        for (const SheetObjects::ObjectInfo &info : m_objects->describe(s))
            if (info.kind == office::sheet::ObjectKind::Picture)
                if (auto c = carriedAt(s, info.id)) found.push_back({s, info.id, *c});
    }
    return found;
}

bool CrunchWindow::editCarriedObject(int sheet, int id)
{
    const auto carried = carriedAt(sheet, id);
    if (!carried) return false;
    if (!carried->object.isOurs) {
        setStatus(tr("This object was made in another program, so it can't be edited here."));
        return false;
    }
    const int edit = objectEditor()->start(this, carried->token, carried->object);
    if (edit == 0) {
        setStatus(carried->object.kind() == office::ole::Kind::Page ? tr("JeffWrite can't be opened here, so this object can't be edited.") : tr("JeffCrunch can't be opened here, so this object can't be edited."));
        return false;
    }
    m_objectEdits.insert(edit, {sheet, carried->token});
    setStatus(tr("Editing the object. Save it in its own window to update it here."));
    return true;
}

void CrunchWindow::carriedObjectSaved(int editId, const QByteArray &part, const QByteArray &png, bool stale)
{
    if (!m_objectEdits.contains(editId) || !m_session) return;
    const QString token = m_objectEdits.value(editId).token;
    for (const CarriedObject &o : carriedObjects()) {
        if (o.carried.token != token) continue;
        office::ole::EmbeddedObject object = o.carried.object;
        office::ole::SavedObject saved;
        saved.partBytes = part;
        saved.previewPng = png;
        saved.pictureStale = stale;
        office::ole::applySave(object, saved);
        const QByteArray carried = objects::carryObject(object, token);
        if (carried.isEmpty() || carried.size() > kMaxClipMediaBytes) {
            setStatus(tr("The changes to the object could not be put in the workbook."));
            return;
        }
        if (m_sheet != o.sheet) showSheet(o.sheet, true);
        const bool done = restyleObject(tr("Update Object"), [](QDomElement &, grid::ObjectBox *) { return true; }, true, o.id, [&](ObjectClip &clip) {
            if (clip.media.size() == 1) clip.media[0].bytes = carried;
        });
        setStatus(done ? tr("The object was updated.") : tr("The changes to the object could not be put in the workbook."));
        return;
    }
}

bool CrunchWindow::convertCarriedToPicture(int sheet, int id)
{
    const auto carried = carriedAt(sheet, id);
    if (!carried || carried->object.previewPng.isEmpty()) return false;
    if (m_sheet != sheet) showSheet(sheet, true);
    const QByteArray plain = carried->object.previewPng;
    return restyleObject(tr("Convert Object to Picture"), [](QDomElement &, grid::ObjectBox *) { return true; }, true, id, [&](ObjectClip &clip) {
        if (clip.media.size() == 1) clip.media[0].bytes = plain;
    });
}

void CrunchWindow::showObjects()
{
    if (!m_session || !m_objects) return;
    auto list = std::make_shared<std::vector<CarriedObject>>();
    objects::ListHost host;
    host.rows = [this, list] {
        *list = carriedObjects();
        QVector<objects::ObjectRow> rows;
        const auto &sheets = m_loaded->book->info().sheets;
        for (int i = 0; i < int(list->size()); ++i) {
            const CarriedObject &o = (*list)[size_t(i)];
            rows.push_back({i, tr("%1, object %2").arg(sheets[o.sheet].name).arg(i + 1), objects::kindWords(o.carried.object), objects::sizeWords(o.carried.object.widthEmu, o.carried.object.heightEmu), true});
        }
        return rows;
    };
    host.open = [this, list](int i) {
        if (i >= 0 && i < int(list->size())) editCarriedObject((*list)[size_t(i)].sheet, (*list)[size_t(i)].id);
    };
    host.convertToPicture = [this, list](int i) { return i >= 0 && i < int(list->size()) && convertCarriedToPicture((*list)[size_t(i)].sheet, (*list)[size_t(i)].id); };
    host.remove = [this, list](int i) {
        if (i < 0 || i >= int(list->size()) || !editingAllowed()) return false;
        const CarriedObject o = (*list)[size_t(i)];
        if (m_sheet != o.sheet) showSheet(o.sheet, true);
        deleteObject(o.id);
        return m_objects->isDeleted(o.sheet, o.id);
    };
    objects::ObjectListDialog dialog(host, this);
    dialog.exec();
}

} // namespace jp
