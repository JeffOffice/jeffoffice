// The Remove Background mode of Editor (core/removebg.h has the work itself).
#include "app/editor.h"

#include "core/workthread.h"

#include <QBuffer>
#include <algorithm>
#include <QCoreApplication>
#include <QImageReader>
#include <QMetaObject>
#include <QPointer>

namespace jp {

namespace {

// The color the part that goes is shown in.
const QColor kRemoveTint(230, 40, 150, 170);

} // namespace

bool Editor::beginRemoveBackground(const QString &id, QString *why)
{
    auto refuse = [why](const QString &text) {
        if (why) *why = text;
        return false;
    };
    auto *pic = dynamic_cast<PictureItem *>(m_doc->item(id));
    if (!pic || pic->imageId.isEmpty() || !pic->mediaKind.isEmpty() || !pic->chartXml.isEmpty()) return refuse(tr("Select a picture to remove its background."));
    const ImageData data = m_doc->images.value(pic->imageId);
    if (const QString f = data.format.toLower(); f == QLatin1String("svg") || f == QLatin1String("svgz") || f == QLatin1String("emf") || f == QLatin1String("wmf") || f == QLatin1String("pdf") || f == QLatin1String("eps"))
        return refuse(tr("The background of a drawing made of lines and shapes can't be removed. Only pictures made of dots, such as photographs, can."));
    QBuffer buffer;
    QByteArray bytes = data.bytes;
    buffer.setData(bytes);
    buffer.open(QIODevice::ReadOnly);
    if (QImageReader reader(&buffer); reader.imageCount() > 1) return refuse(tr("The background of an animated picture can't be removed."));
    const QImage picture = m_doc->image(pic->imageId);
    if (picture.isNull()) return refuse(tr("This picture can't be read."));
    if (qint64(picture.width()) * picture.height() > kRemoveBgMaxSourcePixels) return refuse(tr("This picture is too large for its background to be removed."));
    if (m_rbOpen) endRemoveBackground(true);
    if (!cropItem.isEmpty()) setCropItem(QString());
    beginCropChange(tr("Remove Background"));
    m_rbOpen = true;
    m_rbPage = m_page;
    m_rbSel = m_sel;
    removeBgItem = id;
    removeBgTool = RemoveBgTool::Keep;
    m_rbMarks = pic->removal.isValid() ? pic->removal.marks : QVector<BackgroundMark>();
    if (!(pic->removal.isValid() && pic->removal.source == picture.size())) {
        pic->removal = BackgroundRemoval();
        startRemoveBackground({});
    } else {
        pic->removalTint = kRemoveTint;
        Q_EMIT removeBackgroundProgress(100);
    }
    pic->removalTint = kRemoveTint;
    Q_EMIT viewChanged();
    return true;
}

void Editor::startRemoveBackground(const QVector<BackgroundMark> &marks)
{
    auto *pic = dynamic_cast<PictureItem *>(m_doc->item(removeBgItem));
    if (!pic) return;
    for (auto &j : m_rbJobs) j->cancel = true;
    // (the jobs that have ended are let go; one that has not stops at its next check, and is waited for when the editor goes)
    m_rbJobs.erase(std::remove_if(m_rbJobs.begin(), m_rbJobs.end(), [](const std::shared_ptr<RemoveBgJob> &j) { return j->done; }), m_rbJobs.end());
    auto job = std::make_shared<RemoveBgJob>();
    job->picture = m_doc->image(pic->imageId);
    job->marks = marks;
    RemoveBgJob *raw = job.get();
    QPointer<Editor> self(this);
    const std::weak_ptr<RemoveBgJob> weak = job;
    job->thread = std::make_unique<WorkThread>([raw, self, weak] {
        raw->result = findBackground(raw->picture, raw->marks, [raw, self](int percent) {
            if (raw->cancel) return false;
            if (percent != raw->told.exchange(percent) && self) QMetaObject::invokeMethod(self.data(), [self, percent] { if (self) Q_EMIT self->removeBackgroundProgress(percent == 100 ? 99 : percent); }, Qt::QueuedConnection);
            return true;
        });
        // (the answer is taken on the window's thread, if the editor and the job are still there)
        if (self) QMetaObject::invokeMethod(self.data(), [self, weak] {
            if (const auto j = weak.lock(); j && self) self->takeRemoveBackgroundResult(j);
        }, Qt::QueuedConnection);
    });
    m_rbJobs.push_back(job);
    Q_EMIT removeBackgroundProgress(0);
}

bool Editor::removeBackgroundBusy() const
{
    for (const auto &j : m_rbJobs)
        if (!j->done) return true;
    return false;
}

// Takes the answer of the last job asked for into the picture (one asked for before another is thrown away).
void Editor::takeRemoveBackgroundResult(const std::shared_ptr<RemoveBgJob> &job)
{
    job->thread->join();
    job->done = true;
    if (job != m_rbJobs.back() || !m_rbOpen || job->cancel) return;
    auto *pic = dynamic_cast<PictureItem *>(m_doc->item(removeBgItem));
    if (!pic || !job->result.isValid()) return;
    pic->removal = job->result;
    pic->removalTint = kRemoveTint;
    Q_EMIT removeBackgroundProgress(100);
    Q_EMIT changed();
    Q_EMIT viewChanged();
}

void Editor::finishRemoveBackgroundWork()
{
    for (size_t i = 0; i < m_rbJobs.size(); ++i)
        if (!m_rbJobs[i]->done) takeRemoveBackgroundResult(m_rbJobs[i]);
}

void Editor::addRemoveBackgroundMark(const BackgroundMark &mark)
{
    auto *pic = dynamic_cast<PictureItem *>(m_doc->item(removeBgItem));
    if (!pic || !m_rbOpen || mark.points.isEmpty()) return;
    // A stroke asked for while an earlier one is still being worked on is added to it: the picture's marks are those of the last one asked for.
    QVector<BackgroundMark> marks = m_rbMarks;
    if (marks.size() < kRemoveBgMaxMarks) marks << mark;
    m_rbMarks = marks;
    startRemoveBackground(marks);
}

void Editor::endRemoveBackground(bool keep)
{
    if (!m_rbOpen) return;
    if (keep) finishRemoveBackgroundWork();
    else for (auto &j : m_rbJobs) j->cancel = true;
    for (auto &j : m_rbJobs)
        if (j->thread) j->thread->join();
    m_rbJobs.clear();
    m_rbOpen = false;
    m_rbMarks.clear();
    if (auto *pic = dynamic_cast<PictureItem *>(m_doc->item(removeBgItem))) pic->removalTint = QColor();
    removeBgItem.clear();
    if (keep) finishChange(m_rbPage, m_rbSel);
    else cancelChange();
    Q_EMIT viewChanged();
}

} // namespace jp
