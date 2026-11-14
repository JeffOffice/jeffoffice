#pragma once
// The AutoCorrect dialog (Options > Proofing > AutoCorrect Options): the switches for the capital letters, the replacement list of a language, and the lists of exceptions.
// Everything is kept in the settings (app/autocorrectconfig.h) when the dialog is accepted.

#include "core/export.h"
#include "text/autocorrect.h"

#include <QMap>
#include <QString>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QListWidget;
class QTableWidget;

namespace jp {

class JO_COMMON_EXPORT AutoCorrectPage : public QWidget {
    Q_OBJECT
public:
    explicit AutoCorrectPage(const QString &language, QWidget *parent = nullptr);
    // The configuration of the language that is shown, with what the person has changed.
    autotext::Config config();
    // Keeps every language that was shown, and the switches, in the settings.
    void apply();

private:
    void showLanguage(const QString &language);
    void store();
    void fillReplacements();
    void fillExceptions();
    void addReplacement();
    void deleteReplacement();
    void restoreDefaults();
    QString m_language;
    QMap<QString, autotext::Config> m_configs;   // (by the language's first part)
    QComboBox *m_languages = nullptr;
    QCheckBox *m_replace = nullptr, *m_sentences = nullptr, *m_cells = nullptr, *m_days = nullptr, *m_twoInitials = nullptr, *m_capsLock = nullptr;
    QTableWidget *m_table = nullptr;
    QLineEdit *m_from = nullptr, *m_with = nullptr;
    QListWidget *m_exceptions = nullptr, *m_initials = nullptr;
    QLineEdit *m_exceptionEdit = nullptr, *m_initialEdit = nullptr;
    bool m_filling = false;
    bool m_switchesSet = false;
};

// The dialog, for the text in `language`; true when the person accepted it (and the settings changed).
JO_COMMON_EXPORT bool autoCorrectDialog(QWidget *parent, const QString &language);

} // namespace jp
