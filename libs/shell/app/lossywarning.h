#pragma once
// The warning every app gives before a save to a kind of file that can't hold something in the file: it names what would change (the list comes from the app's own writer, what it leaves
// out), and offers to save anyway, to save in the app's own full format beside it instead, or to cancel. "Don't ask again for this file" is kept for the session. Check Compatibility in
// File > Info shows the same list without saving (lossyReport). A test stands in for the dialog with a function.

#include <QString>
#include <QStringList>
#include <functional>

class QWidget;

namespace jp {

enum class LossyChoice { SaveAnyway, SaveOwnFormat, Cancel };

struct LossyAsk {
    QString path;         // the file that would be written
    QString what;         // what the file holds, as "presentation" or "publication" (lower case, for a sentence)
    QString format;       // the kind that can't hold it all, as ".odp"
    QString ownFormat;    // the app's own full format offered instead, as ".pptx"
    QStringList lost;     // what the kind can't hold, one phrase each
};

// The most lines of the list that are shown in one box; the rest are counted.
constexpr int kLossyMaxShown = 12;

// Asks, unless the list is empty (nothing is lost: nothing is shown, and the answer is SaveAnyway) or the person asked not to be asked again for this file.
LossyChoice askLossy(QWidget *parent, const LossyAsk &ask);

// The sentence that names what the kind can't hold, with the list; the same words the box and Check Compatibility use. Empty `lost` gives the sentence that nothing is lost.
QString lossyReport(const LossyAsk &ask);

// The path of the app's own format beside `path` ("a.odp" becomes "a.pptx", or "a (2).pptx" if that exists). `ownFormat` has its dot.
QString lossyOwnFormatPath(const QString &path, const QString &ownFormat);

// What the person told the session not to ask about again, by file (the list is bounded).
bool lossyAskedNotAgain(const QString &path);
void lossyForgetAll();

// A test's answer: given the question and the state of the box's check box (set it to true to say "don't ask again"). An empty function asks as usual.
void setLossyStandIn(const std::function<LossyChoice(const LossyAsk &ask, bool *dontAskAgain)> &standIn);

} // namespace jp
