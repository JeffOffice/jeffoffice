#pragma once
// Labels and envelopes for a mail merge: the sheets of labels and the envelopes the program offers (by their sizes alone), and documents made for them: a sheet of labels is a table of one
// cell for each label (and a narrow cell between labels where the sheet leaves a gap), and an envelope is a page of the envelope's size with the return address and the delivery address.

#include "core/writedoc.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>
#include <memory>

namespace jp::write::mail {

// A sheet of labels, in points: the paper, how many labels across and down, the size of one, the margin above the first row and left of the first column, and how far apart the labels' tops
// and lefts are (the pitch).
struct LabelSheet {
    QString name;
    double pageW = 612, pageH = 792;
    int columns = 3, rows = 10;
    double labelW = 189, labelH = 72, top = 36, left = 13.5, hPitch = 198, vPitch = 72;
    int perPage() const { return columns * rows; }
};
// The sheets of the list, by the size of the labels and how many are on a sheet ("30 per sheet, 1 x 2 5/8 in"), for letter paper and for A4 paper, and a sheet of one label.
QVector<LabelSheet> labelSheets();

struct EnvelopeSize {
    QString name;
    double w = 0, h = 0;   // points; the long side is the width (the envelope is laid on its long edge)
};
QVector<EnvelopeSize> envelopeSizes();

// How a run of text is put in a paragraph's XML: the text, with a line break in it as a break element.
QByteArray textRuns(const QString &text);
// A complex field (its code and the result it shows) as the runs a paragraph holds.
QByteArray fieldRuns(const QString &code, const QString &result);
// A paragraph of the runs, set with no space round it (for the cells of a sheet of labels).
QByteArray tightParagraph(const QByteArray &runs);

// A document of one sheet of labels: `cell(i)` is the XML of the paragraphs of the i-th label (0 for the first, across then down); an empty one leaves the label blank.
std::unique_ptr<WriteDoc> labelDocument(const LabelSheet &sheet, const std::function<QByteArray(int index)> &cell);

// An envelope document: a page of the envelope's size, laid on its long edge, with the return address (lines; may be empty) at the top left and the delivery address (the runs of its paragraph, see
// textRuns and fieldRuns) in the lower right part.
std::unique_ptr<WriteDoc> envelopeDocument(const EnvelopeSize &size, const QString &returnAddress, const QByteArray &deliveryRuns);

} // namespace jp::write::mail
