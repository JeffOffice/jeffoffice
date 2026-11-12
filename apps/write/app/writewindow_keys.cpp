// Keys for styles and symbols (Options > Customize > Keyboard): a command of the window for each one, made from the settings when the window starts and by the Keyboard page. The id
// holds what the key does ("style.apply:<name>", "symbol.insert:U+XXXX" with ":<font>" when the symbol keeps one); what the settings hold is data, so an id that is not well formed makes
// no command. A style is found by its name when the key is used, so a style that was renamed is not found by the old name.

#include "app/writewindow.h"

#include "app/keymap.h"
#include "app/pageview.h"
#include "app/plaintext.h"
#include "app/stylespane.h"
#include "app/textdialogs.h"
#include "core/editops.h"
#include "core/insertops.h"
#include "core/styleops.h"

#include <QAction>
#include <QInputDialog>
#include <QStatusBar>
#include <algorithm>

namespace jp {

namespace {

const QString kStylePrefix = QStringLiteral("style.apply:");
const QString kSymbolPrefix = QStringLiteral("symbol.insert:");
constexpr int kMostFontLength = 100;
constexpr int kMostStyleChoices = 1000;

bool plainText(const QString &s)
{
    return std::all_of(s.begin(), s.end(), [](QChar c) { return c.isPrint(); });
}

bool styleNameOk(const QString &name) { return !name.trimmed().isEmpty() && name.size() <= write::kMostStyleNameLength && plainText(name); }

// A symbol id: its character, and the font kept with it (empty for none). False for anything that is not one.
bool parseSymbol(const QString &id, QString *text, QString *font)
{
    if (!id.startsWith(kSymbolPrefix)) return false;
    const QString rest = id.mid(kSymbolPrefix.size());
    if (!rest.startsWith(QLatin1String("U+"))) return false;
    const QString code = rest.section(QLatin1Char(':'), 0, 0).mid(2);
    const QString fam = rest.section(QLatin1Char(':'), 1);
    if (code.size() < 4 || code.size() > 6) return false;
    bool ok = false;
    const uint cp = code.toUInt(&ok, 16);
    if (!ok || code != code.toUpper() || cp < 0x20 || cp > 0x10FFFF || (cp >= 0x7F && cp <= 0x9F) || (cp >= 0xD800 && cp <= 0xDFFF) || (cp >= 0xFDD0 && cp <= 0xFDEF) || (cp & 0xFFFE) == 0xFFFE)
        return false;
    if (fam.size() > kMostFontLength || !plainText(fam)) return false;
    if (text) *text = QString::fromUcs4(reinterpret_cast<const char32_t *>(&cp), 1);
    if (font) *font = fam;
    return true;
}

QString symbolId(uint cp, const QString &font)
{
    QString id = kSymbolPrefix + QStringLiteral("U+%1").arg(cp, 4, 16, QLatin1Char('0')).toUpper();
    if (!font.isEmpty()) id += QLatin1Char(':') + font;
    return parseSymbol(id, nullptr, nullptr) ? id : QString();
}

// The style of a name (without regard to case) among the document's and the built-in ones; a paragraph style before a character style. Empty when there is none.
QString styleKeyOf(write::WriteDoc &doc, const QString &name)
{
    QString found;
    for (const write::ListedStyle &s : write::listStyles(doc, write::StyleFilter::All)) {
        if (QString::compare(s.name, name, Qt::CaseInsensitive) != 0) continue;
        if (!s.character) return QStringLiteral("p:") + s.id;
        if (found.isEmpty()) found = QStringLiteral("c:") + s.id;
    }
    return found;
}

} // namespace

void WriteWindow::makeSavedKeyCommands()
{
    for (const QString &prefix : {kStylePrefix, kSymbolPrefix})
        for (const QString &id : KeyMap::savedIds(prefix)) makeKeyCommand(id);
}

QAction *WriteWindow::makeKeyCommand(const QString &id)
{
    if (QAction *have = act(id)) return have;
    QString text, group, font;
    std::function<void()> fn;
    if (id.startsWith(kStylePrefix)) {
        const QString name = id.mid(kStylePrefix.size());
        if (!styleNameOk(name)) return nullptr;
        text = name;
        group = tr("Styles");
        fn = [this, name] { applyStyleByName(name); };
    } else if (QString ch; parseSymbol(id, &ch, &font)) {
        text = tr("%1 (U+%2)").arg(ch, id.mid(kSymbolPrefix.size() + 2).section(QLatin1Char(':'), 0, 0));
        if (!font.isEmpty()) text = tr("%1 in %2").arg(text, font);
        group = tr("Symbols");
        fn = [this, ch, font] {
            if (!m_doc) return;
            write::insertSymbol(*m_doc, m_view->cursor(), ch, font);
            m_view->setFocus(Qt::OtherFocusReason);
            m_view->viewport()->update();
        };
    } else {
        return nullptr;
    }
    auto *a = new QAction(text, this);
    a->setObjectName(id);
    a->setProperty(KeyMap::kGroupProperty, group);
    connect(a, &QAction::triggered, this, [this, id, fn] {
        if (isReadOnly() && !commandAllowedReadOnly(id)) {
            refuseReadOnlyChange();
            return;
        }
        fn();
    });
    m_actions.insert(id, a);
    KeyMap::applyLater(this, a);
    return a;
}

void WriteWindow::applyStyleByName(const QString &name)
{
    if (!m_doc) return;
    const QString key = styleKeyOf(*m_doc, name);
    if (key.isEmpty()) {
        statusBar()->showMessage(tr("The document has no style named \"%1\", and it is not a built-in style, so the key changed nothing.").arg(name), 8000);
        return;
    }
    QTextCursor &c = m_view->cursor();
    if (key.startsWith(QLatin1String("c:"))) write::applyCharacterStyle(*m_doc, c, key.mid(2));
    else write::applyParagraphStyle(*m_doc, c, key.mid(2));
    m_view->setFocus(Qt::OtherFocusReason);
    if (m_styles) m_styles->refresh();
}

// A style key's name shows when the style is not in the document or the built-in ones, so that the Keyboard page tells which keys stopped matching.
void WriteWindow::refreshKeyCommands()
{
    for (const QString &id : actionIds()) {
        if (!id.startsWith(kStylePrefix)) continue;
        QAction *a = act(id);
        const QString name = id.mid(kStylePrefix.size());
        const bool missing = m_doc && styleKeyOf(*m_doc, name).isEmpty();
        a->setText(missing ? tr("%1 (no such style)").arg(name) : name);
    }
}

QList<KeyExtra> WriteWindow::keyExtras()
{
    QList<KeyExtra> out;
    out << KeyExtra{tr("Add Style…"), [this](QWidget *parent) -> QString {
        if (!m_doc) return {};
        QStringList names;
        for (const write::ListedStyle &s : write::listStyles(*m_doc, write::StyleFilter::All))
            if (names.size() < kMostStyleChoices && styleNameOk(s.name) && !names.contains(s.name, Qt::CaseInsensitive)) names << s.name;
        if (names.isEmpty()) return {};
        bool ok = false;
        const QString name = QInputDialog::getItem(parent, tr("Add Style"), tr("Style:"), names, 0, false, &ok);
        return ok ? kStylePrefix + name : QString();
    }};
    out << KeyExtra{tr("Add Symbol…"), [](QWidget *parent) -> QString {
        QString id;
        symbolDialog(parent, [&id](QWidget *, const QString &ch, const QString &font) {
            const QList<uint> cps = ch.toUcs4();
            id = cps.size() == 1 ? symbolId(cps.first(), font) : QString();
            return false;   // (nothing is inserted: the chosen symbol is the one the dialog was left with)
        });
        return id;
    }};
    out << KeyExtra{tr("Add Symbol by Code…"), [](QWidget *parent) -> QString {
        bool ok = false;
        QString code = QInputDialog::getText(parent, tr("Add Symbol by Code"), tr("Unicode code point, in hexadecimal (for example 1F600):"), QLineEdit::Normal, QString(), &ok).trimmed();
        if (!ok) return {};
        if (code.startsWith(QLatin1String("U+"), Qt::CaseInsensitive)) code = code.mid(2);
        bool good = false;
        const uint cp = code.toUInt(&good, 16);
        const QString id = good ? symbolId(cp, QString()) : QString();
        if (id.isEmpty()) plainInformation(parent, tr("Add Symbol by Code"), tr("That is not a character that can have a key."));
        return id;
    }};
    return out;
}

} // namespace jp
