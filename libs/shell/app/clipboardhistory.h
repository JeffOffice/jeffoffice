#pragma once
// The history behind the Clipboard pane (clipboardpane.h): the last 24 things copied or cut in this run of the program, newest first, each kept as the forms it was put on the clipboard in, so that
// putting it back is an ordinary copy of it. Nothing here is ever written to disk, and the history ends with the program's last window. What is kept is bounded before it is kept (8 MB an item,
// 64 MB in all, the oldest dropped), and anything the program or another one marks as secret (a password manager's hint) is never kept. The history only watches the clipboard; it puts something
// there only when the person picks an item.

#include "core/export.h"

#include <QByteArray>
#include <QImage>
#include <QList>
#include <QObject>
#include <QPair>
#include <QString>
#include <functional>

class QMimeData;

namespace jp {

class JO_COMMON_EXPORT ClipboardHistory : public QObject {
    Q_OBJECT
public:
    static constexpr int kMostItems = 24;
    static constexpr qint64 kMostItemBytes = qint64(8) << 20;
    static constexpr qint64 kMostTotalBytes = qint64(64) << 20;
    static constexpr qint64 kMostImagePixels = 40000000;   // a picture over this is not decoded

    enum class Kind { Text, FormattedText, Cells, Picture, Shapes, Slides, Other };

    struct Item {
        quint64 id = 0;
        Kind kind = Kind::Other;
        bool fromOtherProgram = false;
        QString preview;                                          // the first words (at most 200 characters), for a small preview
        QImage thumbnail;                                         // at most 64 pixels each way; null when the item has no picture
        QList<QPair<QString, QByteArray>> formats;                // every form but the picture, in the order the copy made them
        QImage picture;                                           // the picture form, when there is one
        qint64 bytes = 0;
    };

    // The one history of this run (a child of the application: its end is the program's).
    static ClipboardHistory *instance();

    const QList<Item> &items() const { return m_items; }       // newest first
    qint64 totalBytes() const { return m_total; }
    // How many copies were too large to keep since the last time this was asked, and the number dropped to stay under the total.
    int tooLargeCount() const { return m_tooLarge; }
    int droppedCount() const { return m_dropped; }

    // Collecting: while the pane is open, or always when the person set it so. Nothing is watched otherwise (the cost for a person who never opens the pane is none).
    void addOpenPane(int by);   // +1 when a pane is shown, -1 when it is hidden or gone
    bool paneOpen() const { return m_openPanes > 0; }
    void setCollectWhenClosed(bool on);   // kept in the settings (an option of the pane)
    bool collectWhenClosed() const { return m_whenClosed; }
    // What other programs copy (plain text and pictures only), off until the person turns it on, and not kept between runs. Not offered on a system whose secret hint the program cannot read.
    static bool canCollectOtherPrograms();
    void setCollectOtherPrograms(bool on);
    bool collectOtherPrograms() const { return m_others; }
    bool collecting() const { return m_openPanes > 0 || m_whenClosed; }

    // Makes an item of what is on the clipboard now (the seam of the tests: the slot that watches the clipboard calls this). `own` says this program put it there. Returns the item's id, or 0
    // when nothing was kept (it was secret, empty, too large, the same as the newest, or another program's and not wanted).
    quint64 noteClipboard(const QMimeData *mime, bool own);
    // The same, from a mime data that is not the clipboard (for the tests of the bounds).
    static bool isSecret(const QMimeData *mime);

    // An ordinary copy of the item: its forms put on the clipboard (the history does not add it again). False when it is gone.
    bool putOnClipboard(quint64 id);
    QMimeData *mimeOf(quint64 id) const;   // a copy of the item's forms (the caller owns it); null when it is gone
    void remove(quint64 id);
    void clear();
    void takeCounts() { m_tooLarge = 0; m_dropped = 0; }
    // For a test: how the history tells that this program put the clipboard there (the platform's own answer when unset; the offscreen platform always says no).
    void setOwnerCheckForTest(const std::function<bool()> &owns) { m_owns = owns; }

Q_SIGNALS:
    void changed();
    void tooLarge();   // a copy was not kept for its size

private:
    explicit ClipboardHistory(QObject *parent);
    void updateWatching();
    void onClipboard();
    QList<Item> m_items;
    qint64 m_total = 0;
    quint64 m_lastId = 0;
    int m_openPanes = 0;
    bool m_whenClosed = false, m_others = false, m_watching = false, m_putting = false;
    int m_tooLarge = 0, m_dropped = 0;
    std::function<bool()> m_owns;
};

} // namespace jp
