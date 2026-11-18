// The view's objects of other apps: putting one in at the caret, the ones the body holds, and changing one (a save in its own program, Convert to Picture, Delete). The work on the
// document is core/pictureops.h (insertObject, objectAt, setObjectAt); what is here is the caret, the notices, and the redrawing.

#include "app/pageview.h"

#include "core/pictureops.h"
#include "core/writeprops.h"

#include <QCoreApplication>
#include <QTextBlock>
#include <QTextDocument>

namespace jp::write {

bool PageView::insertObjectAtCaret(const office::ole::EmbeddedObject &object)
{
    if (!m_doc || m_readOnly) return false;
    m_doc->pictureNotice.clear();
    QTextCursor c = m_cursor;
    const PictureResult result = insertObject(*m_doc, c, object);
    if (result == PictureResult::Ok) {
        m_cursor = c;
        selectPicture(c.position() - 1);
        Q_EMIT contentChanged();
        return true;
    }
    Q_EMIT notice(m_doc->pictureNotice.isEmpty() ? QCoreApplication::translate("jp::write::Pictures", "This can't be put in the document as an object.") : m_doc->pictureNotice);
    return false;
}

QVector<int> PageView::objectPositions() const
{
    QVector<int> found;
    if (!m_doc) return found;
    const QTextDocument *story = m_doc->story();
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        const QString text = b.text();
        for (int i = 0; i < text.size() && found.size() < office::ole::limits().objects; ++i)
            if (text.at(i) == QChar::ObjectReplacementCharacter && objectAt(*m_doc, b.position() + i)) found.push_back(b.position() + i);
    }
    return found;
}

bool PageView::objectRect(int position, QRectF *rect)
{
    if (!m_doc || m_hf.active || m_note.active) return false;
    const auto object = objectAt(*m_doc, position);
    if (!object) return false;
    PictureInfo info;
    info.valid = true;
    info.width = double(object->widthEmu) / office::ole::kEmuPerPoint;
    info.height = double(object->heightEmu) / office::ole::kEmuPerPoint;
    return inlinePictureRect(position, info, rect);
}

int PageView::objectAtPoint(const QPoint &pos)
{
    if (!m_doc || m_hf.active || m_note.active || m_rects.isEmpty()) return -1;
    const Hit hit = hitAt(pos);
    if (hit.page < 0) return -1;
    const int at = positionAt(hit);
    for (const int p : {at, at - 1}) {
        QRectF r;
        if (p >= 0 && p < m_doc->story()->characterCount() - 1 && m_doc->story()->characterAt(p) == QChar::ObjectReplacementCharacter && objectRect(p, &r) && r.contains(pos)) return p;
    }
    return -1;
}

bool PageView::replaceObject(int position, const office::ole::EmbeddedObject &object)
{
    return changeObjects([&](WriteDoc &doc) { return setObjectAt(doc, position, object); });
}

bool PageView::convertObjectToPicture(int position)
{
    if (!m_doc || m_readOnly) return false;
    const auto object = objectAt(*m_doc, position);
    if (!object || object->previewPng.isEmpty()) return false;
    const double width = double(object->widthEmu) / office::ole::kEmuPerPoint, height = double(object->heightEmu) / office::ole::kEmuPerPoint;
    const PictureInfo was = pictureAt(*m_doc, m_doc->story(), position);
    return changeObjects([&](WriteDoc &doc) {
        QTextCursor c(doc.story());
        c.beginEditBlock();
        c.setPosition(position);
        c.setPosition(position + 1, QTextCursor::KeepAnchor);
        const PictureResult made = insertPicture(doc, c, object->previewPng, QString(), PictureWrap::Inline, was.alt);
        if (made == PictureResult::Ok) setPictureSize(doc, position, was.width > 0 ? was.width : width, was.height > 0 ? was.height : height);
        c.endEditBlock();
        return made == PictureResult::Ok;
    });
}

bool PageView::deleteObject(int position)
{
    if (!m_doc || m_readOnly) return false;
    return changeObjects([&](WriteDoc &doc) {
        QTextCursor c(doc.story());
        c.setPosition(position);
        c.setPosition(position + 1, QTextCursor::KeepAnchor);
        if (!objectAt(doc, position)) return false;
        c.removeSelectedText();
        return true;
    });
}

} // namespace jp::write
