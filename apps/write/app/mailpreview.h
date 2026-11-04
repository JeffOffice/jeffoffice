#pragma once
// The pane that previews a mail merge (Mailings > Preview Results): the document as it is for one recipient, with buttons to go to the first, previous, next, and last recipient. It is a
// view of a merged copy, made again for each recipient: the document itself is not changed.

#include <QWidget>
#include <memory>

class QLabel;
class QSpinBox;

namespace jp::write {

class PageView;
class WriteDoc;

class MailPreviewPane : public QWidget {
    Q_OBJECT
public:
    // `source` makes the document to preview (called when the pane needs a copy: it returns null where it can't), `rows` the rows of the list that are recipients, in order.
    MailPreviewPane(const std::function<std::unique_ptr<WriteDoc>(int row)> &merge, QWidget *parent = nullptr);
    ~MailPreviewPane() override;
    void setRows(const QVector<int> &rows, int total);
    // The place in the list of rows (from 0) that is shown, and showing another.
    int place() const { return m_place; }
    void setPlace(int place);
    void first() { setPlace(0); }
    void previous() { setPlace(m_place - 1); }
    void next() { setPlace(m_place + 1); }
    void last() { setPlace(int(m_rows.size()) - 1); }
    // The row of the list that is shown (-1: none).
    int row() const { return m_place >= 0 && m_place < m_rows.size() ? m_rows[m_place] : -1; }
    // Shows the merge again (the text changed).
    void refresh();
    PageView *view() const { return m_view; }
    WriteDoc *shown() const { return m_doc.get(); }

Q_SIGNALS:
    void placeChanged(int place);

private:
    std::function<std::unique_ptr<WriteDoc>(int row)> m_merge;
    QVector<int> m_rows;
    int m_total = 0, m_place = 0;
    std::unique_ptr<WriteDoc> m_doc;
    PageView *m_view = nullptr;
    QLabel *m_label = nullptr;
    QSpinBox *m_spin = nullptr;
    bool m_changing = false;
};

} // namespace jp::write
