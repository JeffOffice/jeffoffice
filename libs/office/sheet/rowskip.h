#pragma once
// The first screen of a sheet that was saved scrolled far down: the rows above the view are not shown, and what they hold is not kept, so the reader should not have to parse them. This device
// stands between the sheet's inflated bytes and the parser and gives the parser the same XML with those rows cut down to what the parser keeps of a row it does not read the cells of: the
// row's height and whether it is hidden or has a format, and the cell that holds the text of a shared formula (the cells below that share it name it). A row the parser would not keep anything of is not given
// at all. The rows are found with plain searches of the bytes, which go over a hundred megabytes of rows in the time the parser takes for a few thousand cells.
//
// It passes the bytes on as they are when it can't be sure what it is doing: the part is not in UTF-8, its elements have a prefix, the view is not before the cells, there is nothing to leave
// out, or a row is not in the form it looks for.

#include "sheet/sheetreader.h"

#include <QIODevice>

namespace jp::office::sheet {

class RowSkipDevice : public QIODevice {
public:
    // `in` must be open and stay where it is as long as this is used. `options` says the screen (ReadOptions::screenRows) and the limits.
    RowSkipDevice(QIODevice *in, const ReadOptions &options);
    bool isSequential() const override { return true; }
    // What was left out, counted: rows given as they were, rows cut down or not given, and the bytes of the rows that were not given whole.
    qint64 rowsKept() const { return m_kept; }
    qint64 rowsLeftOut() const { return m_left; }
    qint64 bytesLeftOut() const { return m_bytesLeft; }
    // Whether it found rows to leave out (else it passed everything on as it was).
    bool skipping() const { return m_skippingFound; }

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *, qint64) override { return -1; }

private:
    enum class State { Head, Rows, PassThrough, Done };
    bool produce();
    bool fill();
    bool head();
    bool nextRow();
    void passRest();
    QIODevice *m_in;
    const ReadOptions &m_options;
    State m_state = State::Head;
    QByteArray m_buf;          // read from the sheet, from m_pos on not yet looked at
    qsizetype m_pos = 0;
    QByteArray m_out;          // to give
    qsizetype m_outPos = 0;
    bool m_eof = false, m_failed = false, m_skippingFound = false;
    qint64 m_skipTop = 0, m_frozenEnd = 0, m_begin = 0;   // the rows the view leaves out are those before m_skipTop and from m_frozenEnd to m_begin
    bool m_zeroHeight = false;
    quint32 m_nextRow = 0;      // the row that a row with no number is
    qint64 m_kept = 0, m_left = 0, m_bytesLeft = 0;
};

} // namespace jp::office::sheet
