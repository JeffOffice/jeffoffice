#pragma once
// Check Compatibility, the same box in every app (File > Info): the person picks a kind of file the app saves (an .odt, an .rtf; an .xls, an .ods, a .csv; an .odp), and the box says
// what that kind can't hold of this file, in the sentence the save warning uses (libs/shell/app/lossywarning.h). Each app supplies the kinds and, for each, how its writer finds
// what is left out. The two choices kept in the file itself (the final mark and "always open for reading only") are added here, once, for every kind that is not the app's own: no
// other kind holds them.

#include "app/lossywarning.h"
#include "core/export.h"
#include <QDialog>
#include <QStringList>
#include <QVector>
#include <functional>
#include <initializer_list>

class QComboBox;
class QLabel;

namespace jp {

// What a check came to: the list of what the kind can't hold, or that the check could not be made (and why). A check that could not be made is never shown as "nothing is lost".
struct CompatAnswer {
    QStringList found;   // what the kind can't hold, one phrase each
    qint64 more = 0;     // how many more lines the app counted but did not keep (its writer bounds its own list)
    QString couldNot;    // when not empty: no check was made; why, as a sentence
    CompatAnswer() = default;
    CompatAnswer(const QStringList &list) : found(list) {}
    CompatAnswer(std::initializer_list<QString> list) : found(list) {}
    static CompatAnswer failed(const QString &why);
};
using CompatDone = std::function<void(const CompatAnswer &)>;

struct CompatFormat {
    QString label;       // as the picker shows it, "OpenDocument text (.odt)"
    QString format;      // ".odt"
    QString ownFormat;   // the app's own full format, ".docx"
    QString what;        // what the file holds, for a sentence: "document"
    // Works out what the kind can't hold and calls `done` with the answer: at once, or later when it takes another thread. It is never called after the box is gone.
    std::function<void(const CompatDone &done)> lost;
};

struct CompatHost {
    QVector<CompatFormat> formats;
    std::function<bool()> markedFinal;
    std::function<bool()> alwaysReadOnly;
};

// The most lines of one list the box shows (the file's own choices and the line "And N more." included), and the longest one line is.
constexpr int kCompatMaxLines = 200;
constexpr int kCompatMaxLineChars = 400;

// The two choices a file holds that no other kind does, as lines of the list (each only when it is on).
JO_COMMON_EXPORT QStringList compatFileChoices(const QString &what, bool markedFinal, bool alwaysReadOnly);

// The list the box shows for a kind: what the app found (empty when no check was made), bounded, then the file's choices.
JO_COMMON_EXPORT QStringList compatLines(const CompatFormat &format, const CompatHost &host, const CompatAnswer &answer);

// The sentence and the list for a kind (lossyReport of compatLines).
JO_COMMON_EXPORT QString compatReport(const CompatFormat &format, const CompatHost &host, const CompatAnswer &answer);

class JO_COMMON_EXPORT CompatibilityDialog : public QDialog {
    Q_OBJECT
public:
    CompatibilityDialog(const CompatHost &host, int first, QWidget *parent);
    // For tests: what the box says now, whether it is still working it out, and a choice from the picker.
    QString reportText() const;
    bool working() const { return m_working; }
    int formatCount() const { return int(m_host.formats.size()); }
    void choose(int index);

private:
    void work();
    CompatHost m_host;
    QComboBox *m_picker = nullptr;
    QLabel *m_report = nullptr;
    int m_generation = 0;
    bool m_working = false;
};

// Runs the box. Where `notifier` is given (a test, or a window that says things in its status line) the first kind's report is handed to it instead, once it is worked out.
JO_COMMON_EXPORT void showCompatibility(QWidget *parent, const CompatHost &host, int first, const std::function<void(const QString &title, const QString &text, bool warn)> &notifier);

} // namespace jp
