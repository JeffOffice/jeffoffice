#include "app/translate.h"

#include "app/plaintext.h"
#include "app/settings.h"
#include "text/dictionaries.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QStatusBar>
#include <QUrlQuery>
#include <QVBoxLayout>

#include <algorithm>

namespace jp {

QString cutAtLetters(const QString &text, int most)
{
    if (text.size() <= most) return text;
    int n = most;
    if (n > 0 && text.at(n - 1).isHighSurrogate()) --n;
    return text.left(n);
}

TranslateRequest translateRequest(const QString &server, const QString &target, const QString &text)
{
    TranslateRequest out;
    QUrl base(server.trimmed().isEmpty() ? QStringLiteral("https://libretranslate.com/") : server.trimmed(), QUrl::StrictMode);
    if (!base.isValid() || base.host().isEmpty()) return out;
    const QString host = base.host().toLower();
    const bool thisComputer = host == QLatin1String("localhost") || host == QLatin1String("127.0.0.1") || host == QLatin1String("::1") || host == QLatin1String("[::1]");
    if (base.scheme() != QLatin1String("https") && !(base.scheme() == QLatin1String("http") && thisComputer)) return out;
    QString sent = cutAtLetters(text, kMostTranslateLetters);
    for (;;) {
        QUrl url = base;
        QUrlQuery q;
        q.addQueryItem(QStringLiteral("source"), QStringLiteral("auto"));
        q.addQueryItem(QStringLiteral("target"), target);
        q.addQueryItem(QStringLiteral("q"), sent);
        url.setQuery(q);
        const QString written = url.toString(QUrl::FullyEncoded);
        if (written.size() <= kMostTranslateAddress || sent.isEmpty()) {
            out.url = written;
            break;
        }
        sent = cutAtLetters(sent, std::max(0, int(sent.size() * 9 / 10) - 1));
    }
    out.sentText = sent;
    out.cut = sent.size() < text.size();
    return out;
}

QString translateUrl(const QString &server, const QString &target, const QString &text) { return translateRequest(server, target, text).url; }

// (The dialogs and messages keep the context of JeffWrite, where they began, so that their translations are kept.)
bool languageDialog(QWidget *parent, const QString &current, bool noProof, QString *tag, bool *noProofOut)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("languageDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::HomeDialogs", "Language"));
    auto *v = new QVBoxLayout(&dlg);
    v->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Mark the selected text as:"), &dlg)));
    auto *list = new QListWidget(&dlg);
    list->setObjectName(QStringLiteral("languages"));
    int row = 0;
    for (const dict::Language &l : dict::languages()) {
        auto *item = new QListWidgetItem(l.name, list);
        item->setData(Qt::UserRole, l.code);
        if (l.code == current) row = list->count() - 1;
    }
    list->setCurrentRow(row);
    v->addWidget(list);
    auto *skip = new QCheckBox(QCoreApplication::translate("jp::write::HomeDialogs", "Do not check spelling"), &dlg);
    skip->setObjectName(QStringLiteral("doNotCheck"));
    skip->setChecked(noProof);
    v->addWidget(skip);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    QObject::connect(list, &QListWidget::itemDoubleClicked, &dlg, &QDialog::accept);
    v->addWidget(bb);
    if (dlg.exec() != QDialog::Accepted || !list->currentItem()) return false;
    *tag = list->currentItem()->data(Qt::UserRole).toString();
    *noProofOut = skip->isChecked();
    return true;
}

bool translateDialog(QWidget *parent, bool wholeDocument, QString *targetCode, QString *server, int textLength)
{
    struct Target { const char *name; const char *code; };
    static const Target kTargets[] = {{QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "English"), "en"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Spanish"), "es"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "French"), "fr"},
                                      {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "German"), "de"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Italian"), "it"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Portuguese"), "pt"},
                                      {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Dutch"), "nl"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Polish"), "pl"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Russian"), "ru"},
                                      {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Ukrainian"), "uk"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Arabic"), "ar"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Hindi"), "hi"},
                                      {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Chinese (Simplified)"), "zh-Hans"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Japanese"), "ja"},
                                      {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Korean"), "ko"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Vietnamese"), "vi"}, {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Turkish"), "tr"},
                                      {QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Greek"), "el"}};
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("translateDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::write::HomeDialogs", "Translate"));
    auto *v = new QVBoxLayout(&dlg);
    auto *about = plainLabel(new QLabel(wholeDocument ? QCoreApplication::translate("jp::write::HomeDialogs", "Translate the whole document into:") : QCoreApplication::translate("jp::write::HomeDialogs", "Translate the selected text into:"), &dlg));
    v->addWidget(about);
    auto *target = new QComboBox(&dlg);
    target->setObjectName(QStringLiteral("translateTarget"));
    for (const Target &t : kTargets) {
        target->addItem(QCoreApplication::translate("jp::write::HomeDialogs", t.name), QString::fromLatin1(t.code));
        if (*targetCode == QLatin1String(t.code)) target->setCurrentIndex(target->count() - 1);
    }
    v->addWidget(target);
    v->addWidget(plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Translation service (a LibreTranslate address):"), &dlg)));
    auto *address = new QLineEdit(*server, &dlg);
    address->setObjectName(QStringLiteral("translateServer"));
    v->addWidget(address);
    auto *note = plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "Your web browser will open that address with the text. Nothing is sent until you click OK. Only secure (https) addresses are used, and http ones for this computer."), &dlg));
    note->setWordWrap(true);
    v->addWidget(note);
    if (textLength > kMostTranslateLetters) {
        auto *cut = plainLabel(new QLabel(QCoreApplication::translate("jp::write::HomeDialogs", "The text has %1 letters. Only the first %2 will be sent to be translated.").arg(textLength).arg(kMostTranslateLetters), &dlg));
        cut->setObjectName(QStringLiteral("translateCutNote"));
        cut->setWordWrap(true);
        v->addWidget(cut);
    }
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    v->addWidget(bb);
    if (dlg.exec() != QDialog::Accepted) return false;
    *targetCode = target->currentData().toString();
    *server = address->text().trimmed();
    return true;
}

QString translateReady(QString text)
{
    text.replace(QChar::ParagraphSeparator, QLatin1Char('\n')).replace(QChar::LineSeparator, QLatin1Char('\n')).remove(QChar::ObjectReplacementCharacter);
    return text.trimmed();
}

bool openTranslation(QStatusBar *bar, const QString &text, const QString &target, const QString &server, const std::function<bool(const QUrl &)> &opener)
{
    const TranslateRequest req = translateRequest(server, target, text);
    if (req.url.isEmpty()) {
        if (bar) bar->showMessage(QCoreApplication::translate("jp::WriteWindow", "The translation service needs a secure web address (https). An http address is only used for this computer."), 8000);
        return false;
    }
    const bool opened = opener ? opener(QUrl(req.url)) : QDesktopServices::openUrl(QUrl(req.url));
    if (!opened) {
        if (bar) bar->showMessage(QCoreApplication::translate("jp::WriteWindow", "The web browser could not be opened. Nothing was sent."), 8000);
        return false;
    }
    if (req.cut && bar) bar->showMessage(QCoreApplication::translate("jp::WriteWindow", "Only the first %n letter(s) of the text were sent to be translated.", nullptr, int(req.sentText.size())), 8000);
    return true;
}

bool translateText(QWidget *parent, QStatusBar *bar, const QString &text, bool whole, const std::function<bool(const QUrl &)> &opener)
{
    const QString ready = translateReady(text);
    if (ready.isEmpty()) {
        if (bar) bar->showMessage(QCoreApplication::translate("jp::WriteWindow", "There is no text to translate."), 5000);
        return false;
    }
    QString target = Settings::get().value(QStringLiteral("translate/targetCode"), QStringLiteral("es")).toString();
    QString server = Settings::get().value(QStringLiteral("translate/server"), QStringLiteral("https://libretranslate.com/")).toString();
    if (!translateDialog(parent, whole, &target, &server, int(ready.size()))) return false;
    Settings::get().setValue(QStringLiteral("translate/targetCode"), target);
    Settings::get().setValue(QStringLiteral("translate/server"), server);
    return openTranslation(bar, ready, target, server, opener);
}

} // namespace jp
