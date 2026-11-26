#pragma once
// The windows of saved blocks every app of the suite can use (libs/base/io/blocklibrary.h holds the blocks): the gallery that drops down from a ribbon button (a search box and each
// block drawn as the app draws it), the box that saves a selection under a name, and the box that manages the library. What a block is, how a preview is drawn, and how a block is put in are
// the app's: it gives a `BlockPreview` and an `insert`.

#include "app/widgets.h"
#include "io/blocklibrary.h"

#include <QDialog>
#include <QHash>
#include <QImage>
#include <QPixmap>
#include <QSize>
#include <functional>

class QLineEdit;
class QListWidget;
class QMenu;

namespace jp {

// A block drawn at `size` (pixels, of a screen of `dpr`) by the program that made it; a null image shows the block's name only.
using BlockPreview = std::function<QImage(const BlockEntry &entry, const QSize &size, qreal dpr)>;
using BlockInsertFn = std::function<void(const BlockInfo &info, const QByteArray &data)>;

// The gallery's body: a search box (over names, categories, and descriptions) above the blocks of one gallery of `format`, each drawn when it first comes into view, never all at once.
class JO_COMMON_EXPORT BlockGalleryList : public QWidget {
    Q_OBJECT
public:
    BlockGalleryList(const QString &gallery, const QString &format, const BlockPreview &preview, QWidget *parent = nullptr);
    void reload();                         // from the library as it is now (read on a thread of its own first, when it is large)
    void setGallery(const QString &gallery);
    QString currentId() const;
    QStringList shownIds() const;
    QListWidget *list() const { return m_list; }
    QLineEdit *search() const { return m_search; }
    int previewsMade() const { return m_previewsMade; }   // for a test: how many blocks were drawn
    QPixmap previewOf(const QString &id);                  // drawn at first use, then kept

Q_SIGNALS:
    void picked(const QString &id);

private:
    void fill();
    class Delegate;
    QString m_gallery, m_format;
    BlockPreview m_preview;
    QLineEdit *m_search;
    QListWidget *m_list;
    QWidget *m_empty;
    QHash<QString, QPixmap> m_pictures;
    int m_previewsMade = 0;
};

// A menu for a ribbon button: the gallery, then `footer` (Save Selection as a Block, Manage Saved Blocks). The block picked is handed to `insert` with its bytes.
JO_COMMON_EXPORT QMenu *blocksMenu(QWidget *owner, const QString &gallery, const QString &format, const BlockPreview &preview, const BlockInsertFn &insert, const QList<QAction *> &footer);

// The box that names a block: `info` gives the first name, category, description, and way of insertion, and takes what the person chose. False when canceled.
JO_COMMON_EXPORT bool saveBlockBox(QWidget *parent, BlockInfo *info, bool editing = false);
// Puts the block in the library; a name that is taken asks whether to replace it. False when it was not saved (the person has been told why).
JO_COMMON_EXPORT bool saveBlock(QWidget *parent, const BlockInfo &info, const QByteArray &data);

// The box that lists the library for the person to rename, change, delete (until the box closes, a delete can be taken back), export, and import.
// The galleries it lists are the blocks and the text shortcuts, and `more`: the galleries of one's own that the app uses (an app lists only the galleries it uses).
JO_COMMON_EXPORT void manageBlocksBox(QWidget *parent, const QString &format, const BlockPreview &preview, const QStringList &more = QStringList());

// The names of the galleries, as the person reads them.
JO_COMMON_EXPORT QString blockGalleryName(const QString &gallery);

} // namespace jp
