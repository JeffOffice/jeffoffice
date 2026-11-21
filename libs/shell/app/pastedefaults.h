#pragma once
// Set Default Paste: the form a paste takes when nothing else is asked, chosen for each way of pasting (within a document, between documents, from other programs). The Options section
// that chooses them, the settings that keep them, and the stamp a copy carries so that a paste can tell where it came from are here, once, for every app that pastes; what a form is
// (keep the source formatting, merge, text only, values, ...) is the app's own, a number it gives each choice.

#include "core/export.h"

#include <QString>
#include <QVector>
#include <QWidget>

class QMimeData;

namespace jp {

enum class PasteFrom { SameDocument, OtherDocument, OtherProgram, Last = OtherProgram };

// A form the app offers, and the way of pasting a row of the section is for: `key` is the setting that keeps the choice (what the app has always called it, if it had one).
struct PasteDefaultChoice {
    QString text;
    int value = 0;
};
struct PasteDefaultRow {
    PasteFrom from = PasteFrom::SameDocument;
    QString label, key;
    QVector<PasteDefaultChoice> choices;
    int fallback = 0;   // the value that is the default until one is chosen
};

class JO_COMMON_EXPORT PasteDefaultsSection : public QWidget {
    Q_OBJECT
public:
    PasteDefaultsSection(const QVector<PasteDefaultRow> &rows, QWidget *parent = nullptr);
    // Keeps what is chosen.
    void apply();
    // The value kept for the row, or its fallback when none is kept or the one kept is not among the choices.
    static int value(const PasteDefaultRow &row);
    // The row of the way of pasting, or null.
    static const PasteDefaultRow *rowFor(const QVector<PasteDefaultRow> &rows, PasteFrom from);

private:
    QVector<PasteDefaultRow> m_rows;
};

// A copy carries where it was made: this program (its name) and which document in it. Stamped on what the app puts on the clipboard; read by a paste.
JO_COMMON_EXPORT void stampPasteOrigin(QMimeData *mime, const QString &documentId);
// Where the clipboard's content came from, for the document `documentId`: its own, another document of this program, or another program (no stamp, or another program's).
JO_COMMON_EXPORT PasteFrom pasteOriginOf(const QMimeData *mime, const QString &documentId);
// A name for a document that no other has, for the stamp.
JO_COMMON_EXPORT QString newPasteDocumentId();

} // namespace jp
