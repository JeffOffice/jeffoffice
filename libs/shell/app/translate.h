#pragma once
// Translate and Set Proofing Language, the Review tab's Language group, for the programs that have it (JeffWrite, JeffDeck, and JeffCrunch): the address that opens the translation
// service with the text, the two dialog boxes, and the steps from the text to the browser. Nothing is sent before OK, and only to the address the person sees in the box.

#include <QString>
#include <QUrl>
#include <functional>

class QWidget;

namespace jp {

// What a command tells the person (the status bar's message for so many milliseconds).
using TranslateSay = std::function<void(const QString &text, int milliseconds)>;

constexpr int kMostTranslateLetters = 5000;   // the service's limit for one request (UTF-16 letters)
constexpr int kMostTranslateAddress = 8000;   // the longest address (once written out) that is opened
struct TranslateRequest {
    QString url;            // empty when the service address is refused
    QString sentText;       // the text in the address: the start of the text, cut at a whole letter
    bool cut = false;       // the text was longer than what is sent
};
// The address that opens the translation service `server` (a LibreTranslate address) with the text to be translated into `target`. Only https addresses are used, and http ones for this computer
// (localhost, 127.0.0.1, ::1); the text is cut to kMostTranslateLetters and then, if its address is too long once written out (letters outside the Latin ones take nine characters each), shorter,
// never in the middle of a pair of surrogates.
TranslateRequest translateRequest(const QString &server, const QString &target, const QString &text);
// translateRequest's address; empty when the address of the service is refused.
QString translateUrl(const QString &server, const QString &target, const QString &text);
// The text cut to at most `most` letters, never in the middle of a pair of surrogates.
QString cutAtLetters(const QString &text, int most);

// Language: the languages that can be checked, with the one the text is in chosen, and "Do not check spelling". False if canceled; else `tag` and `noProof` are the choice.
bool languageDialog(QWidget *parent, const QString &current, bool noProof, QString *tag, bool *noProofOut);
// Translate: (it says when the text, `textLength` letters, is longer than what is sent) the language to translate into (its LibreTranslate code, remembered) and the address of the service (changeable, remembered). False if canceled.
bool translateDialog(QWidget *parent, bool wholeDocument, QString *targetCode, QString *server, int textLength = 0);

// The text of a Translate command made ready for the service: paragraph and line breaks as line feeds, no object marks, and no space around it.
QString translateReady(QString text);
// Opens the translation service in the web browser with the text through `opener` (the browser's own when empty); whether it was opened. What is cut off, a refused address, and a browser
// that does not open are told through `say`.
bool openTranslation(const TranslateSay &say, const QString &text, const QString &target, const QString &server, const std::function<bool(const QUrl &)> &opener = {});
// The whole Translate command: `text` (as it is in the program; `whole` when no part was selected) made ready, the dialog (the target and the address are kept in the settings), and the
// browser. False when there was no text, the dialog was canceled, or nothing was opened.
bool translateText(QWidget *parent, const TranslateSay &say, const QString &text, bool whole, const std::function<bool(const QUrl &)> &opener = {});

} // namespace jp
