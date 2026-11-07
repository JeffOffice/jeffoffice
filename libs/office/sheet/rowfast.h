#pragma once
// The rows of a big sheet are most of its bytes, and nearly all of them are cells of a form a few dozen lines can read: a number, a shared string, a formula with its result. The XML parser takes
// several times longer over them than that reader does. This device stands between the sheet's bytes and the parser, and gives the parser the XML as it is, except at the one place the
// reader of the sheet asks it to stop: after the start of `sheetData` and after the end of a row the parser was given. There the reader takes over, scans the rows that are of the plain form,
// and gives the parser the first row that is not (as one row, after which the parser is stopped again), or everything that is left.
//
// It gives the parser every byte there is, in order, except the rows the reader scanned; and a read that stops short stops the parser as it stops the reader.

#include <QByteArray>
#include <QByteArrayView>
#include <QIODevice>

namespace jp::office::sheet {

class RowFeed : public QIODevice {
public:
    // The source is not owned, and must be open for reading and stay so as long as this is used.
    explicit RowFeed(QIODevice *source);
    bool isSequential() const override { return true; }

    // Whether the parser has been given the bytes up to a place the reader can take over from, and no more.
    bool paused() const { return m_state == State::Paused; }
    // The reader takes over: the bytes that are ready (bytes()) are its. False if it is not paused.
    bool takeOver();
    QByteArrayView bytes() const { return QByteArrayView(m_buf).sliced(m_pos); }
    // Reads more from the source behind the bytes; false at its end, when it fails, or when the reader holds more than it should.
    bool more();
    void consume(qsizetype n) { m_pos += n; }
    // The parser is given the rest, as it is.
    void giveBack();
    // The parser is given the first `length` bytes, and stops there to be taken over again.
    void giveBackThenPause(qsizetype length);

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *, qint64) override { return -1; }

private:
    enum class State { Watch, Pass, Paused, Scan, Serve };
    qint64 pull();
    qint64 give(char *data, qint64 maxlen, qsizetype upTo);
    QIODevice *m_src;
    State m_state = State::Watch;
    QByteArray m_buf;
    qsizetype m_pos = 0;
    qsizetype m_pauseAt = -1;     // Watch: the end of the tag to stop after; Serve: where the row ends
    qsizetype m_searched = 0;     // Watch: where the bytes have been looked through for the tag
    bool m_eof = false;
};

} // namespace jp::office::sheet
