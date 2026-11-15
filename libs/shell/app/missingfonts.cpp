#include "app/missingfonts.h"
#include "app/icons.h"
#include "app/settings.h"
#include "core/fonts.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDockWidget>
#include <QFontDatabase>
#include <QFontInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QTextBlock>
#include <QTextDocument>
#include <QToolButton>
#include <QVBoxLayout>

#include <memory>

namespace jp {

void FontUsage::add(const QString &family, const QString &place)
{
    if (family.trimmed().isEmpty()) return;
    if (!m_places.contains(family)) m_order << family;
    QStringList &where = m_places[family];
    if (!place.isEmpty() && !where.contains(place)) where << place;
}

void FontUsage::addDocument(const QTextDocument *doc, const QString &place)
{
    if (!doc) return;
    bool unnamed = false;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next())
        for (auto f = b.begin(); !f.atEnd(); ++f) {
            const QTextCharFormat cf = f.fragment().charFormat();
            const QStringList fams = cf.fontFamilies().toStringList();
            if (!fams.isEmpty()) add(fams.first(), place);
            else unnamed = true;
        }
    if (unnamed) add(doc->defaultFont().family(), place);
}

bool fontIsInstalled(const QString &family) { return QFontDatabase::hasFamily(family); }

namespace {

QString choiceKey(const QString &family) { return QStringLiteral("fonts/replace/") + family.toLower(); }

}

QString fontChoice(const QString &family) { return Settings::get().value(choiceKey(family)).toString(); }

void setFontChoice(const QString &family, const QString &replacement)
{
    Settings::get().setValue(choiceKey(family), replacement);
    setFontReplacement(family, replacement);
}

bool applyFontChoices(const QStringList &families)
{
    bool any = false;
    for (const QString &family : families) {
        const QString choice = fontChoice(family);
        if (choice.isEmpty() || fontReplacement(family) == choice) continue;
        setFontReplacement(family, choice);
        any = true;
    }
    return any;
}

QList<MissingFont> missingFonts(const FontUsage &usage, const FontInstalled &installed)
{
    QList<MissingFont> out;
    for (const QString &family : usage.families()) {
        if (installed ? installed(family) : fontIsInstalled(family)) continue;
        MissingFont m;
        m.family = family;
        m.places = usage.places(family);
        const QString mine = fontReplacement(family);
        // What stands in without a choice: the table's stand-in, else the font the system picks.
        if (!mine.isEmpty()) setFontReplacement(family, QString());
        const QString table = substituteFor(family);
        m.inPlace = m.automatic = table.isEmpty() ? QFontInfo(QFont(family)).family() : table;
        if (!mine.isEmpty()) {
            setFontReplacement(family, mine);
            m.inPlace = mine;
            m.chosen = true;
        }
        out.push_back(m);
    }
    return out;
}

namespace {

// What the bar's Details button calls: set at each check.
struct Opener : QObject {
    using QObject::QObject;
    std::function<void()> open;
};

QString placesText(const QStringList &places)
{
    const int shown = 6;
    QStringList some = places.mid(0, shown);
    QString text = some.join(QStringLiteral(", "));
    if (places.size() > shown) text += QCoreApplication::translate("jp::MissingFonts", ", and more");
    return text;
}

// The dialog's rows follow the fonts of the document as it is now (they change after Replace in Document).
class DialogState {
public:
    MissingFontsHost host;
    FontInstalled installed;
    QTableWidget *table = nullptr;
    QLabel *note = nullptr;
    QPushButton *replace = nullptr;
    QStringList families;
    QList<MissingFont> rows;

    void fill()
    {
        rows = missingFonts(host.usage ? host.usage() : FontUsage(), installed);
        table->setRowCount(0);
        table->setRowCount(rows.size());
        QStringList all = QFontDatabase::families();
        for (int r = 0; r < rows.size(); ++r) {
            const MissingFont &m = rows.at(r);
            auto *name = new QTableWidgetItem(m.family);
            name->setFlags(name->flags() & ~Qt::ItemIsEditable);
            table->setItem(r, 0, name);
            auto *combo = new QComboBox(table);
            combo->setObjectName(QStringLiteral("missingFontsChoice"));
            combo->setAccessibleName(QCoreApplication::translate("jp::MissingFonts", "Font used instead of %1").arg(m.family));
            combo->addItem(QCoreApplication::translate("jp::MissingFonts", "%1 (automatic)").arg(m.automatic), QString());
            for (const QString &f : all)
                if (f != m.family) combo->addItem(f, f);
            const int at = m.chosen ? combo->findData(m.inPlace) : 0;
            combo->setCurrentIndex(at >= 0 ? at : 0);
            const QString family = m.family;
            QObject::connect(combo, &QComboBox::activated, combo, [this, family, combo] {
                const QString choice = combo->currentData().toString();
                setFontChoice(family, choice);
                for (MissingFont &m : rows)
                    if (m.family == family) {
                        m.chosen = !choice.isEmpty();
                        m.inPlace = choice.isEmpty() ? m.automatic : choice;
                    }
                if (host.refresh) host.refresh();
                updateButtons();
            });
            table->setCellWidget(r, 1, combo);
            auto *where = new QTableWidgetItem(placesText(m.places));
            where->setFlags(where->flags() & ~Qt::ItemIsEditable);
            table->setItem(r, 2, where);
        }
        if (!rows.isEmpty()) table->selectRow(0);
        updateButtons();
    }
    void updateButtons() { replace->setEnabled(table->currentRow() >= 0 && table->currentRow() < rows.size() && host.replace); }
};

} // namespace

void showMissingFontsDialog(QWidget *parent, const MissingFontsHost &host, const FontInstalled &installed)
{
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("missingFontsDialog"));
    dlg.setWindowTitle(QCoreApplication::translate("jp::MissingFonts", "Missing Fonts"));
    dlg.resize(640, 360);
    auto *layout = new QVBoxLayout(&dlg);
    auto *intro = new QLabel(QCoreApplication::translate("jp::MissingFonts",
                                                         "This computer has no font for each font below, so another font is shown and written to PDF in its place. The file keeps the font's name unless you press Replace in Document."),
                             &dlg);
    intro->setWordWrap(true);
    layout->addWidget(intro);
    auto state = std::make_shared<DialogState>();
    state->host = host;
    state->installed = installed;
    auto *table = new QTableWidget(0, 3, &dlg);
    table->setObjectName(QStringLiteral("missingFontsTable"));
    table->setHorizontalHeaderLabels({QCoreApplication::translate("jp::MissingFonts", "Missing font"), QCoreApplication::translate("jp::MissingFonts", "Used instead"),
                                      QCoreApplication::translate("jp::MissingFonts", "Used in")});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->setColumnWidth(1, 220);
    table->verticalHeader()->hide();
    layout->addWidget(table, 1);
    auto *note = new QLabel(&dlg);
    note->setObjectName(QStringLiteral("missingFontsNote"));
    note->setTextFormat(Qt::PlainText);
    layout->addWidget(note);
    auto *buttons = new QHBoxLayout;
    auto *replace = new QPushButton(QCoreApplication::translate("jp::MissingFonts", "Replace in Document"), &dlg);
    replace->setObjectName(QStringLiteral("missingFontsReplace"));
    replace->setToolTip(QCoreApplication::translate("jp::MissingFonts", "Changes the selected font to the font used instead, in the file itself"));
    auto *close = new QPushButton(QCoreApplication::translate("jp::MissingFonts", "Close"), &dlg);
    close->setObjectName(QStringLiteral("missingFontsClose"));
    close->setDefault(true);
    buttons->addWidget(replace);
    buttons->addStretch(1);
    buttons->addWidget(close);
    layout->addLayout(buttons);
    state->table = table;
    state->note = note;
    state->replace = replace;
    QObject::connect(close, &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(table, &QTableWidget::currentCellChanged, &dlg, [state] { state->updateButtons(); });
    QObject::connect(replace, &QPushButton::clicked, &dlg, [state] {
        const int r = state->table->currentRow();
        if (r < 0 || r >= state->rows.size() || !state->host.replace) return;
        const MissingFont m = state->rows.at(r);
        const int n = state->host.replace(m.family, m.inPlace);
        if (n < 0) {
            state->note->setText(QCoreApplication::translate("jp::MissingFonts", "The document can't be changed."));
            return;
        }
        state->fill();
        state->note->setText(QCoreApplication::translate("jp::MissingFonts", "%1 is replaced in the document.").arg(m.family));
    });
    state->fill();
    dlg.exec();
}

void checkMissingFonts(QMainWindow *win, const MissingFontsHost &host, const FontInstalled &installed)
{
    if (!win) return;
    QDockWidget *dock = win->findChild<QDockWidget *>(QStringLiteral("missingFontsDock"));
    const FontUsage usage = host.usage ? host.usage() : FontUsage();
    // The choices made before hold for this document too (the text is drawn again with them).
    QStringList lacking;
    for (const QString &family : usage.families())
        if (!(installed ? installed(family) : fontIsInstalled(family))) lacking << family;
    if (applyFontChoices(lacking) && host.refresh) host.refresh();
    if (lacking.isEmpty()) {
        if (dock) dock->hide();
        return;
    }
    if (!dock) {
        dock = new QDockWidget(win);
        dock->setObjectName(QStringLiteral("missingFontsDock"));
        dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
        dock->setTitleBarWidget(new QWidget(dock));
        auto *bar = new QWidget(dock);
        bar->setObjectName(QStringLiteral("missingFontsBar"));
        auto *row = new QHBoxLayout(bar);
        row->setContentsMargins(8, 2, 8, 2);
        auto *icon = new QLabel(bar);
        icon->setPixmap(jp::icon(QStringLiteral("info")).pixmap(16, 16));
        row->addWidget(icon);
        auto *text = new QLabel(QCoreApplication::translate("jp::MissingFonts", "This document uses fonts that aren't on this computer"), bar);
        text->setObjectName(QStringLiteral("missingFontsText"));
        text->setTextFormat(Qt::PlainText);
        row->addWidget(text);
        auto *details = new QPushButton(QCoreApplication::translate("jp::MissingFonts", "Details"), bar);
        details->setObjectName(QStringLiteral("missingFontsDetails"));
        details->setFocusPolicy(Qt::NoFocus);
        row->addWidget(details);
        row->addStretch(1);
        auto *close = new QToolButton(bar);
        close->setObjectName(QStringLiteral("missingFontsDismiss"));
        close->setIcon(jp::icon(QStringLiteral("x")));
        close->setToolTip(QCoreApplication::translate("jp::MissingFonts", "Close"));
        close->setAccessibleName(close->toolTip());
        close->setAutoRaise(true);
        close->setFocusPolicy(Qt::NoFocus);
        row->addWidget(close);
        dock->setWidget(bar);
        win->addDockWidget(Qt::TopDockWidgetArea, dock);
        QObject::connect(close, &QToolButton::clicked, dock, &QDockWidget::hide);
        auto *opener = new Opener(dock);
        opener->setObjectName(QStringLiteral("missingFontsOpener"));
        QObject::connect(details, &QPushButton::clicked, opener, [opener] {
            if (opener->open) opener->open();
        });
    }
    // The dialog opens with the host and the test of the latest check.
    static_cast<Opener *>(dock->findChild<QObject *>(QStringLiteral("missingFontsOpener")))->open = [win, host, installed] {
        showMissingFontsDialog(win, host, installed);
        checkMissingFonts(win, host, installed);
    };
    dock->show();
}

} // namespace jp
