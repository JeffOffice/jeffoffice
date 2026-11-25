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
#include <QPointer>
#include <QStringList>
#include <any>
#include <QString>
#include <functional>

class QMimeData;

namespace jp {

// What is offered on the clipboard, as the history asks about it: the names of the forms, a size where one is known without fetching, and a read that stops at a limit. The platform's own offer
// (a mime data) is wrapped by ClipboardHistory::noteClipboard; the tests give one that counts every byte it hands over.
class JO_COMMON_EXPORT ClipboardSource {
public:
    virtual ~ClipboardSource() = default;
    virtual QStringList formats() const = 0;
    virtual qint64 sizeOf(const QString &format) const { Q_UNUSED(format); return -1; }   // -1: not known without fetching
    // The bytes of a form, at most `most` of them: when there are more, `*over` is set and nothing is returned.
    virtual QByteArray read(const QString &format, qint64 most, bool *over) const = 0;
    // A picture of this program's own copy (already in memory, so its size is known without copying it).
    virtual QImage ownImage() const { return QImage(); }
    // Which offer this is, where the platform tells one offer from the next (0: it does not).
    virtual quint64 identity() const { return 0; }
};

class JO_COMMON_EXPORT ClipboardHistory : public QObject {
    Q_OBJECT
public:
    static constexpr int kMostItems = 24;
    static constexpr qint64 kMostItemBytes = qint64(8) << 20;
    static constexpr qint64 kMostTotalBytes = qint64(64) << 20;
    static constexpr int kMostFormats = 32;                    // forms kept of one of this program's copies (its own forms and the standard text and picture ones come first)
    static constexpr qint64 kMostImagePixels = kMostItemBytes / 4;   // a picture is counted at 4 bytes a pixel: one over this is refused from its header, before it is decoded

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
        std::any ownState;                                        // what the app keeps of its own copy besides the clipboard (see setOwnStateHooks), handed back when the item is put back
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
    // Called once as an app starts: when the person asked to keep collecting with the pane closed, the history is made now (one settings read), so that it collects from the first copy. When not,
    // nothing is made and nothing watches the clipboard.
    static void startIfAsked();
    bool collectWhenClosed() const { return m_whenClosed; }
    // What other programs copy (plain text and pictures only), off until the person turns it on, and not kept between runs. Not offered on a system whose secret hint the program cannot read.
    static bool canCollectOtherPrograms();
    void setCollectOtherPrograms(bool on);
    bool collectOtherPrograms() const { return m_others; }
    bool collecting() const { return m_openPanes > 0 || m_whenClosed; }

    // Makes an item of what is on the clipboard now (the seam of the tests: the slot that watches the clipboard calls this). `own` says this program put it there. Returns the item's id, or 0
    // when nothing was kept (it was secret, empty, too large, the same as the newest, or another program's and not wanted).
    quint64 noteClipboard(const QMimeData *mime, bool own);
    quint64 noteSource(const ClipboardSource &source, bool own);
    // The same, from a mime data that is not the clipboard (for the tests of the bounds).
    static bool isSecret(const QMimeData *mime);

    // An app that keeps something of its own copy outside the clipboard (JeffWrite keeps the copied text with its structure) gives the history a way to take it as an own copy is noted and to give it
    // back before the item is put on the clipboard again, so that an older item pastes as it did when it was the newest. `take` may return an empty value.
    static void setOwnStateHooks(const std::function<std::any(const QMimeData *)> &take, const std::function<void(const std::any &)> &give);

    // An ordinary copy of the item: its forms put on the clipboard (the history does not add it again). False when it is gone.
    bool putOnClipboard(quint64 id);   // (the change notice this makes, however late it comes, is not a copy)
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
    bool m_whenClosed = false, m_others = false, m_watching = false, m_putting = false, m_fetching = false;
    quint64 m_refusedIdentity = 0;
    int m_tooLarge = 0, m_dropped = 0;
    std::function<bool()> m_owns;
    QPointer<QMimeData> m_put;   // what putOnClipboard gave the clipboard: while it is what the clipboard holds, a change notice is not a new copy
};

} // namespace jp
