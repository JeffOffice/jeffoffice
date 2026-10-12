#pragma once
// The Reuse Slides pane, on the right of the window: Browse picks a
// presentation (any file JeffDeck opens), its slides are listed as thumbnails
// with their titles, and a click on one (or Enter) takes a copy of it for the
// presentation being edited (core/reuse.h). "Keep source formatting" is off
// until it is turned on. The thumbnails are drawn one at a time while idle.

#include "core/document.h"

#include <QTimer>
#include <QWidget>
#include <memory>

class QCheckBox;
class QLabel;
class QListWidget;

namespace jp {

class LayoutCache;

class ReusePane : public QWidget {
    Q_OBJECT
public:
    explicit ReusePane(QWidget *parent = nullptr);
    ~ReusePane() override;

    // Reads the presentation at `path` and lists its slides. False, with the
    // reason in message(), when it can't be read: the list is then empty.
    bool open(const QString &path);
    // Asks which file (what Browse does), then opens it.
    void browse();
    const Document *source() const { return m_doc.get(); }
    QString message() const { return m_error; }
    QListWidget *list() const { return m_list; }
    int slideCount() const;
    // How many slides have their thumbnail drawn.
    int renderedCount() const { return m_rendered; }
    bool keepFormatting() const;
    void setKeepFormatting(bool keep);

Q_SIGNALS:
    // The slide `slide` of the presentation was picked.
    void slideChosen(int slide, bool keepFormatting);

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    void renderNext();
    void choose(int row);

    QLabel *m_status = nullptr;
    QListWidget *m_list = nullptr;
    QCheckBox *m_keep = nullptr;
    std::unique_ptr<Document> m_doc;
    std::unique_ptr<LayoutCache> m_cache;
    QTimer m_timer;
    QString m_error;
    int m_next = 0, m_rendered = 0;
};

} // namespace jp
