#include "grid/hfpicturecache.h"

#include "sheet/pagesetup.h"

#include <QBuffer>
#include <QImageReader>
#include <QMutexLocker>
#include <QThreadPool>

#include <algorithm>

namespace jp::grid {

HeaderPictureCache &HeaderPictureCache::instance()
{
    static HeaderPictureCache *cache = new HeaderPictureCache;
    return *cache;
}

QImage HeaderPictureCache::make(const QByteArray &bytes) const
{
    QImage image;
    QBuffer buffer;
    buffer.setData(bytes);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer);
    const QSize size = reader.size();
    if (size.isValid() && qint64(size.width()) * size.height() <= jp::office::sheet::kMaxHeaderPicturePixels && reader.read(&image)) {
        const int most = 2400;   // (a page's width at 300 to the inch)
        if (image.width() > most || image.height() > most) image = image.scaled(most, most, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return image;
}

QImage HeaderPictureCache::get(const QByteArray &image, bool wait, bool *pending)
{
    if (pending) *pending = false;
    if (image.isEmpty()) return QImage();
    const Key key(image.constData(), image.size());
    {
        QMutexLocker lock(&m_mutex);
        const auto have = m_entries.find(key);
        if (have != m_entries.end()) {
            have->used = ++m_tick;
            return have->image;
        }
        if (!wait) {
            if (pending) *pending = true;
            if (m_pending.count(key) || int(m_pending.size()) >= kMaxPending) return QImage();
            m_pending.insert(key);
            const QByteArray pin = image;
            QThreadPool::globalInstance()->start([this, key, pin] {
                const QImage made = make(pin);
                QMutexLocker lock2(&m_mutex);
                insert(key, pin, made);
                m_pending.erase(key);
            });
            const std::function<void()> hook = m_afterStart;
            lock.unlock();
            if (hook) hook();
            return QImage();
        }
        ++m_here;
    }
    const QImage made = make(image);
    QMutexLocker lock(&m_mutex);
    insert(key, image, made);
    return made;
}

// (called with the lock held)
void HeaderPictureCache::insert(const Key &key, const QByteArray &pin, const QImage &image)
{
    const auto old = m_entries.find(key);
    if (old != m_entries.end()) {
        m_bytes -= old->image.sizeInBytes() + old->pin.size();
        m_entries.erase(old);
    }
    Entry e;
    e.pin = pin;
    e.image = image;
    e.used = ++m_tick;
    m_bytes += image.sizeInBytes() + pin.size();
    m_entries.insert(key, e);
    trim();
}

// (called with the lock held) What is kept is within the budget, but for the picture just put in.
void HeaderPictureCache::trim()
{
    while (m_bytes > m_budget && m_entries.size() > 1) {
        auto oldest = m_entries.begin();
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
            if (it->used < oldest->used) oldest = it;
        m_bytes -= oldest->image.sizeInBytes() + oldest->pin.size();
        m_entries.erase(oldest);
    }
}

void HeaderPictureCache::setBudgetBytes(qint64 bytes)
{
    QMutexLocker lock(&m_mutex);
    m_budget = std::max<qint64>(0, bytes);
    trim();
}

qint64 HeaderPictureCache::budgetBytes() const
{
    QMutexLocker lock(&m_mutex);
    return m_budget;
}
qint64 HeaderPictureCache::bytes() const
{
    QMutexLocker lock(&m_mutex);
    return m_bytes;
}
int HeaderPictureCache::count() const
{
    QMutexLocker lock(&m_mutex);
    return int(m_entries.size());
}
int HeaderPictureCache::pendingCount() const
{
    QMutexLocker lock(&m_mutex);
    return int(m_pending.size());
}
int HeaderPictureCache::decodesHere() const
{
    QMutexLocker lock(&m_mutex);
    return m_here;
}

void HeaderPictureCache::setAfterStartHook(const std::function<void()> &hook)
{
    QMutexLocker lock(&m_mutex);
    m_afterStart = hook;
}

void HeaderPictureCache::waitForWorkers() { QThreadPool::globalInstance()->waitForDone(); }

void HeaderPictureCache::clear()
{
    waitForWorkers();
    QMutexLocker lock(&m_mutex);
    m_entries.clear();
    m_bytes = 0;
}

} // namespace jp::grid
