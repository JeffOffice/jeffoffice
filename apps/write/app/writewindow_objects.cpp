// JeffWrite's objects of other apps: Insert > Object (a range or chart of a workbook), Paste Special as an object, double-clicking one to edit it in its own app, and the list of the
// document's objects in File > Info. The picture, the package, and the box that asks where an object comes from are libs/shell/app/objects.h; the work on the document is
// core/pictureops.h.

#include "app/writewindow.h"

#include "app/pageview.h"

#include <QApplication>
#include <QClipboard>
#include <QFileInfo>
#include <QMimeData>
#include <QStatusBar>

namespace jp {

namespace {

office::ole::Kind kindOfSource(objects::Source source) { return source == objects::Source::Page ? office::ole::Kind::Page : office::ole::Kind::Sheet; }

} // namespace

void WriteWindow::createObjectActions()
{
    mk("insert.object.range", tr("From JeffCrunch Range…"), "table", QKeySequence(), [this] { insertObjectFromAsk(objects::Source::Range); });
    mk("insert.object.chart", tr("From JeffCrunch Chart…"), "chart-column", QKeySequence(), [this] { insertObjectFromAsk(objects::Source::Chart); });
    mk("edit.pasteObject", tr("As Object"), "box", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        if (!insertObjectFromCopy()) statusBar()->showMessage(tr("What was copied can't be put in as an object."), 6000);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    connect(m_view, &write::PageView::editObjectWanted, this, [this](int position) { editObjectAt(position); });
    mk("file.objects", tr("Objects…"), "boxes", QKeySequence(), [this] { showObjects(); });
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [this] {
        if (m_view) refreshObjectUi();
    });
}

void WriteWindow::refreshObjectUi()
{
    const bool can = m_doc && !m_view->inHeaderFooter() && !m_view->inNote();
    act("insert.object.range")->setEnabled(can);
    act("insert.object.chart")->setEnabled(can);
    act("edit.pasteObject")->setEnabled(can && objects::hasObject(QApplication::clipboard()->mimeData()));
}

EmbeddedEdit *WriteWindow::objectEditor()
{
    if (!m_objectEdit) {
        m_objectEdit = new EmbeddedEdit(this);
        objects::usePictures(m_objectEdit);
        m_objectEdit->setOpener(objects::openInOwnApp);
        connect(m_objectEdit, &EmbeddedEdit::objectSaved, this, &WriteWindow::objectSaved);
    }
    return m_objectEdit;
}

bool WriteWindow::insertObjectFrom(objects::Source source, const QString &path, const QString &detail)
{
    if (!m_doc || refuseIfReadOnly()) return false;
    QString why;
    std::optional<office::ole::EmbeddedObject> object;
    if (source == objects::Source::Range) object = objects::rangeObjectFromFile(path, detail, &why);
    else if (source == objects::Source::Chart) object = objects::chartObjectFromFile(path, &why);
    else object = objects::pageObjectFromFile(path, detail.toInt(), &why);
    if (!object) {
        statusBar()->showMessage(why, 8000);
        return false;
    }
    const bool put = m_view->insertObjectAtCaret(*object);
    m_view->setFocus(Qt::OtherFocusReason);
    return put;
}

bool WriteWindow::insertObjectFromCopy()
{
    const auto object = objects::objectOf(QApplication::clipboard()->mimeData());
    if (!object) return false;
    const bool put = m_view->insertObjectAtCaret(*object);
    m_view->setFocus(Qt::OtherFocusReason);
    return put;
}

void WriteWindow::insertObjectFromAsk(objects::Source source)
{
    if (!m_doc || refuseIfReadOnly()) return;
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

bool WriteWindow::editObjectAt(int position)
{
    if (!m_doc) return false;
    const auto object = write::objectAt(*m_doc, position);
    if (!object) return false;
    if (!object->isOurs) {
        statusBar()->showMessage(tr("This object was made in another program, so it can't be edited here."), 8000);
        return false;
    }
    const int id = objectEditor()->start(this, object->origin, *object);
    if (id == 0) {
        statusBar()->showMessage(object->kind() == office::ole::Kind::Page ? tr("JeffWrite can't be opened here, so this object can't be edited.") : tr("JeffCrunch can't be opened here, so this object can't be edited."), 8000);
        return false;
    }
    m_objectEdits.insert(id, object->origin);
    statusBar()->showMessage(tr("Editing the object. Save it in its own window to update it here."), 8000);
    return true;
}

void WriteWindow::objectSaved(int editId, const QByteArray &part, const QByteArray &png, bool stale)
{
    if (!m_doc || !m_objectEdits.contains(editId)) return;
    const QString origin = m_objectEdits.value(editId);
    int position = -1;
    std::optional<office::ole::EmbeddedObject> now;
    for (const int at : m_view->objectPositions()) {
        if (const auto o = write::objectAt(*m_doc, at); o && o->origin == origin) {
            position = at;
            now = o;
            break;
        }
    }
    if (position < 0) return;
    office::ole::SavedObject saved;
    saved.partBytes = part;
    saved.previewPng = png;
    saved.pictureStale = stale;
    office::ole::applySave(*now, saved);
    if (!m_view->replaceObject(position, *now)) {
        statusBar()->showMessage(tr("The changes to the object could not be put in the document."), 8000);
        return;
    }
    if (const auto after = write::objectAt(*m_doc, position)) m_objectEdits[editId] = after->origin;
    statusBar()->showMessage(tr("The object was updated."), 4000);
}

void WriteWindow::showObjects()
{
    if (!m_doc) return;
    objects::ListHost host;
    host.rows = [this] {
        QVector<objects::ObjectRow> rows;
        for (const int at : m_view->objectPositions()) {
            const auto o = write::objectAt(*m_doc, at);
            if (!o) continue;
            rows.push_back({at, tr("Object %1").arg(rows.size() + 1), objects::kindWords(*o), objects::sizeWords(o->widthEmu, o->heightEmu), !o->previewPng.isEmpty()});
        }
        return rows;
    };
    host.open = [this](int at) { editObjectAt(at); };
    host.convertToPicture = [this](int at) { return !refuseIfReadOnly() && m_view->convertObjectToPicture(at); };
    host.remove = [this](int at) { return !refuseIfReadOnly() && m_view->deleteObject(at); };
    objects::ObjectListDialog dialog(host, this);
    dialog.exec();
}

} // namespace jp
