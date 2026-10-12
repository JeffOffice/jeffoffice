#include "app/showdialogs.h"

#include "app/appwindow.h"
#include "app/help.h"
#include "app/icons.h"
#include "app/widgets.h"
#include "core/document.h"
#include "core/reuse.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>

namespace jp {

namespace {

// F1 in a dialog: its Help, in a window of its own over the dialog (the window's Help pane can't be reached while the dialog holds it).
void showDialogHelp(QDialog *dlg, const QString &title)
{
    AppWindow *win = nullptr;
    for (QWidget *p = dlg->parentWidget(); p && !win; p = p->parentWidget()) win = qobject_cast<AppWindow *>(p);
    if (!win) return;
    QString topic = help::contextTopic(QStringLiteral("dialog:") + title);
    if (topic.isEmpty()) topic = QStringLiteral("show");
    auto *w = dlg->findChild<HelpWindow *>(QString(), Qt::FindDirectChildrenOnly);
    if (!w) w = new HelpWindow(win, dlg);
    w->view()->showTopic(topic);
    w->show();
    w->raise();
    w->activateWindow();
    w->view()->focusTopic();
}

// "3. Title of the slide", or its number alone when it has no title.
QString slideLabel(const Document &doc, int slide)
{
    const QString title = deck::slideTitle(doc, slide);
    return QStringLiteral("%1. %2").arg(slide + 1).arg(title.isEmpty() ? QCoreApplication::translate("ShowDialogs", "Slide %1").arg(slide + 1) : title);
}

struct LaserChoice {
    const char *name;
    QColor color;
};

const LaserChoice kLasers[] = {{QT_TRANSLATE_NOOP("ShowDialogs", "Red"), QColor(255, 0, 0)},
                               {QT_TRANSLATE_NOOP("ShowDialogs", "Green"), QColor(0, 255, 0)},
                               {QT_TRANSLATE_NOOP("ShowDialogs", "Blue"), QColor(0, 0, 255)}};

} // namespace

// ---- Set Up Show

SetUpShowDialog::SetUpShowDialog(const deck::ShowSettings &s, const QVector<deck::CustomShow> &shows, int slideCount, const ColorScheme &scheme, QWidget *parent)
    : QDialog(parent), m_base(s), m_scheme(scheme), m_pen(s.penColor), m_loopChosen(s.loop), m_timingsChosen(s.useTimings)
{
    setWindowTitle(tr("Set Up Show"));
    setObjectName(QStringLiteral("setUpShowDialog"));
    auto *lay = new QVBoxLayout(this);
    auto *grid = new QGridLayout();
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(10);
    lay->addLayout(grid);

    auto radio = [](QWidget *parent, const QString &name, const QString &text) {
        auto *r = new QRadioButton(text, parent);
        r->setObjectName(name);
        return r;
    };

    auto *typeBox = new QGroupBox(tr("Show type"));
    auto *typeLay = new QVBoxLayout(typeBox);
    m_speaker = radio(typeBox, QStringLiteral("typeSpeaker"), tr("Presented by a speaker (full screen)"));
    m_individual = radio(typeBox, QStringLiteral("typeIndividual"), tr("Browsed by an individual (window)"));
    m_kiosk = radio(typeBox, QStringLiteral("typeKiosk"), tr("Browsed at a kiosk (full screen)"));
    typeLay->addWidget(m_speaker);
    typeLay->addWidget(m_individual);
    typeLay->addWidget(m_kiosk);
    grid->addWidget(typeBox, 0, 0);

    auto *optionsBox = new QGroupBox(tr("Show options"));
    auto *optionsLay = new QVBoxLayout(optionsBox);
    m_loop = new QCheckBox(tr("Loop continuously until 'Esc'"));
    m_loop->setObjectName(QStringLiteral("loop"));
    m_noAnimation = new QCheckBox(tr("Show without animation"));
    m_noAnimation->setObjectName(QStringLiteral("noAnimation"));
    optionsLay->addWidget(m_loop);
    optionsLay->addWidget(m_noAnimation);
    auto *colors = new QFormLayout();
    m_penButton = new ColorButton(QStringLiteral("pen"), tr("Pen color"), false, QString());
    m_penButton->setScheme(scheme);
    m_penButton->setCurrent(ColorRef::rgb(m_pen));
    m_penButton->setWell(true);
    m_penButton->setObjectName(QStringLiteral("penColor"));
    m_laser = new QComboBox();
    m_laser->setObjectName(QStringLiteral("laserColor"));
    m_laser->setAccessibleName(tr("Laser pointer color"));
    for (const LaserChoice &c : kLasers) m_laser->addItem(QCoreApplication::translate("ShowDialogs", c.name), c.color);
    // A color the file asks for that the list lacks stays, under its code.
    int laserAt = -1;
    for (int i = 0; i < m_laser->count(); ++i)
        if (m_laser->itemData(i).value<QColor>() == s.laserColor) laserAt = i;
    if (laserAt < 0) {
        m_laser->addItem(s.laserColor.name(QColor::HexRgb).toUpper(), s.laserColor);
        laserAt = m_laser->count() - 1;
    }
    m_laser->setCurrentIndex(laserAt);
    colors->addRow(tr("Pen color:"), m_penButton);
    colors->addRow(tr("Laser pointer color:"), m_laser);
    optionsLay->addLayout(colors);
    grid->addWidget(optionsBox, 0, 1);

    auto *slidesBox = new QGroupBox(tr("Show slides"));
    auto *slidesLay = new QGridLayout(slidesBox);
    m_all = radio(slidesBox, QStringLiteral("slidesAll"), tr("All"));
    m_range = radio(slidesBox, QStringLiteral("slidesRange"), tr("From:"));
    m_custom = radio(slidesBox, QStringLiteral("slidesCustom"), tr("Custom show:"));
    m_from = new QSpinBox();
    m_from->setObjectName(QStringLiteral("rangeFrom"));
    m_from->setAccessibleName(tr("From slide"));
    m_to = new QSpinBox();
    m_to->setObjectName(QStringLiteral("rangeTo"));
    m_to->setAccessibleName(tr("To slide"));
    m_from->setRange(1, std::max(1, slideCount));
    m_to->setRange(1, std::max(1, slideCount));
    m_shows = new QComboBox();
    m_shows->setObjectName(QStringLiteral("customShow"));
    m_shows->setAccessibleName(tr("Custom show"));
    for (const deck::CustomShow &show : shows) m_shows->addItem(show.name, QVariant::fromValue(show.id));
    slidesLay->addWidget(m_all, 0, 0, 1, 4);
    slidesLay->addWidget(m_range, 1, 0);
    slidesLay->addWidget(m_from, 1, 1);
    slidesLay->addWidget(new QLabel(tr("To:")), 1, 2);
    slidesLay->addWidget(m_to, 1, 3);
    slidesLay->addWidget(m_custom, 2, 0);
    slidesLay->addWidget(m_shows, 2, 1, 1, 3);
    slidesLay->setColumnStretch(3, 1);
    grid->addWidget(slidesBox, 1, 0);

    auto *advanceBox = new QGroupBox(tr("Advance slides"));
    auto *advanceLay = new QVBoxLayout(advanceBox);
    m_manual = radio(advanceBox, QStringLiteral("advanceManual"), tr("Manually"));
    m_timings = radio(advanceBox, QStringLiteral("advanceTimings"), tr("Using timings, if present"));
    advanceLay->addWidget(m_manual);
    advanceLay->addWidget(m_timings);
    advanceLay->addStretch(1);
    grid->addWidget(advanceBox, 1, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    lay->addWidget(buttons);

    // What the settings say.
    (s.type == deck::ShowSettings::Type::Kiosk ? m_kiosk : s.type == deck::ShowSettings::Type::Individual ? m_individual : m_speaker)->setChecked(true);
    m_loop->setChecked(s.loop);
    m_noAnimation->setChecked(!s.animation);
    (s.useTimings ? m_timings : m_manual)->setChecked(true);
    const bool range = s.slides == deck::ShowSettings::Slides::Range;
    m_from->setValue(range ? std::min(s.from, slideCount) : 1);
    m_to->setValue(range ? std::min(std::max(s.from, s.to), slideCount) : slideCount);
    int showAt = 0;
    for (int i = 0; i < m_shows->count(); ++i)
        if (m_shows->itemData(i).toLongLong() == s.customShow) showAt = i;
    m_shows->setCurrentIndex(showAt);
    if (s.slides == deck::ShowSettings::Slides::Custom && m_shows->count()) m_custom->setChecked(true);
    else if (range) m_range->setChecked(true);
    else m_all->setChecked(true);
    m_custom->setEnabled(m_shows->count() > 0);

    for (QRadioButton *r : {m_speaker, m_individual, m_kiosk, m_all, m_range, m_custom}) connect(r, &QRadioButton::toggled, this, [this] { refresh(); });
    connect(m_loop, &QCheckBox::clicked, this, [this](bool on) { m_loopChosen = on; });
    connect(m_manual, &QRadioButton::clicked, this, [this] { m_timingsChosen = false; });
    connect(m_timings, &QRadioButton::clicked, this, [this] { m_timingsChosen = true; });
    connect(m_from, &QSpinBox::valueChanged, this, [this](int v) { if (m_to->value() < v) m_to->setValue(v); });
    connect(m_to, &QSpinBox::valueChanged, this, [this](int v) { if (m_from->value() > v) m_from->setValue(v); });
    connect(m_penButton, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
        m_pen = c.resolve(m_scheme);
        m_penButton->setCurrent(ColorRef::rgb(m_pen));
    });
    refresh();
}

// A kiosk loops and moves on by the times, whatever the boxes were: they are held until another kind of show is chosen.
void SetUpShowDialog::refresh()
{
    const bool kiosk = m_kiosk->isChecked();
    {
        const QSignalBlocker a(m_loop), b(m_manual), c(m_timings);
        m_loop->setChecked(kiosk || m_loopChosen);
        (kiosk || m_timingsChosen ? m_timings : m_manual)->setChecked(true);
    }
    m_loop->setEnabled(!kiosk);
    m_manual->setEnabled(!kiosk);
    m_timings->setEnabled(!kiosk);
    m_from->setEnabled(m_range->isChecked());
    m_to->setEnabled(m_range->isChecked());
    m_shows->setEnabled(m_custom->isChecked());
}

deck::ShowSettings SetUpShowDialog::settings() const
{
    deck::ShowSettings s = m_base;
    s.type = m_kiosk->isChecked() ? deck::ShowSettings::Type::Kiosk : m_individual->isChecked() ? deck::ShowSettings::Type::Individual : deck::ShowSettings::Type::Speaker;
    s.loop = m_loop->isChecked();
    s.animation = !m_noAnimation->isChecked();
    s.useTimings = m_timings->isChecked();
    s.penColor = m_pen;
    s.laserColor = m_laser->currentData().value<QColor>();
    // The range is kept only when it is the choice, so that a show left as it was stays as it was.
    s.from = s.to = 1;
    if (m_custom->isChecked() && m_shows->count()) {
        s.slides = deck::ShowSettings::Slides::Custom;
        s.customShow = m_shows->currentData().toLongLong();
    } else if (m_range->isChecked()) {
        s.slides = deck::ShowSettings::Slides::Range;
        s.from = m_from->value();
        s.to = std::max(m_from->value(), m_to->value());
        s.customShow = -1;
    } else {
        s.slides = deck::ShowSettings::Slides::All;
        s.customShow = -1;
    }
    return s;
}

void SetUpShowDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->matches(QKeySequence::HelpContents)) showDialogHelp(this, windowTitle());
    else QDialog::keyPressEvent(e);
}

// ---- one custom show

CustomShowEditDialog::CustomShowEditDialog(const Document &doc, const QVector<deck::CustomShow> &all, const deck::CustomShow *editing, QWidget *parent)
    : QDialog(parent), m_all(all), m_new(!editing)
{
    setWindowTitle(tr("Define Custom Show"));
    setObjectName(QStringLiteral("customShowEditDialog"));
    if (editing) m_show = *editing;
    else {
        m_show.id = deck::newCustomShowId(all);
        m_show.name = deck::newCustomShowName(all);
    }
    auto *lay = new QVBoxLayout(this);
    auto *nameRow = new QHBoxLayout();
    nameRow->addWidget(new QLabel(tr("Slide show name:")));
    m_name = new QLineEdit(m_show.name);
    m_name->setObjectName(QStringLiteral("showName"));
    m_name->setAccessibleName(tr("Slide show name"));
    m_name->setMaxLength(deck::kMaxCustomShowName);
    nameRow->addWidget(m_name, 1);
    lay->addLayout(nameRow);

    auto *lists = new QHBoxLayout();
    auto *left = new QVBoxLayout();
    left->addWidget(new QLabel(tr("Slides in presentation:")));
    m_presentation = new QListWidget();
    m_presentation->setObjectName(QStringLiteral("slidesInPresentation"));
    m_presentation->setAccessibleName(tr("Slides in presentation"));
    m_presentation->setSelectionMode(QAbstractItemView::ExtendedSelection);
    for (int i = 0; i < doc.pages.size(); ++i) {
        auto *it = new QListWidgetItem(slideLabel(doc, i), m_presentation);
        it->setData(Qt::UserRole, doc.pages[i]->id);
    }
    left->addWidget(m_presentation, 1);
    lists->addLayout(left, 1);

    auto *middle = new QVBoxLayout();
    middle->addStretch(1);
    m_add = new QPushButton(tr("Add >>"));
    m_add->setObjectName(QStringLiteral("addSlide"));
    m_remove = new QPushButton(tr("Remove"));
    m_remove->setObjectName(QStringLiteral("removeSlide"));
    middle->addWidget(m_add);
    middle->addWidget(m_remove);
    middle->addStretch(1);
    lists->addLayout(middle);

    auto *right = new QVBoxLayout();
    right->addWidget(new QLabel(tr("Slides in custom show:")));
    m_mine = new QListWidget();
    m_mine->setObjectName(QStringLiteral("slidesInShow"));
    m_mine->setAccessibleName(tr("Slides in custom show"));
    m_mine->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QHash<QString, int> index;
    for (int i = 0; i < doc.pages.size(); ++i) index.insert(doc.pages[i]->id, i);
    for (const QString &page : m_show.slides) {
        const auto at = index.constFind(page);
        if (at == index.constEnd()) continue;
        auto *it = new QListWidgetItem(slideLabel(doc, at.value()), m_mine);
        it->setData(Qt::UserRole, page);
    }
    right->addWidget(m_mine, 1);
    lists->addLayout(right, 1);

    auto *order = new QVBoxLayout();
    order->addStretch(1);
    m_up = new QPushButton();
    m_up->setObjectName(QStringLiteral("moveUp"));
    m_up->setIcon(icon(QStringLiteral("arrow-up")));
    m_up->setToolTip(tr("Move up"));
    m_up->setAccessibleName(tr("Move up"));
    m_down = new QPushButton();
    m_down->setObjectName(QStringLiteral("moveDown"));
    m_down->setIcon(icon(QStringLiteral("arrow-down")));
    m_down->setToolTip(tr("Move down"));
    m_down->setAccessibleName(tr("Move down"));
    order->addWidget(m_up);
    order->addWidget(m_down);
    order->addStretch(1);
    lists->addLayout(order);
    lay->addLayout(lists, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_ok = buttons->button(QDialogButtonBox::Ok);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    lay->addWidget(buttons);
    resize(620, 420);

    connect(m_add, &QPushButton::clicked, this, [this] { addSlides(); });
    connect(m_presentation, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        auto *it = new QListWidgetItem(item->text(), m_mine);
        it->setData(Qt::UserRole, item->data(Qt::UserRole));
        refresh();
    });
    connect(m_remove, &QPushButton::clicked, this, [this] {
        qDeleteAll(m_mine->selectedItems());
        refresh();
    });
    auto move = [this](int by) {
        const int row = m_mine->currentRow();
        if (row < 0 || row + by < 0 || row + by >= m_mine->count()) return;
        QListWidgetItem *item = m_mine->takeItem(row);
        m_mine->insertItem(row + by, item);
        m_mine->setCurrentRow(row + by);
    };
    connect(m_up, &QPushButton::clicked, this, [move] { move(-1); });
    connect(m_down, &QPushButton::clicked, this, [move] { move(1); });
    connect(m_name, &QLineEdit::textChanged, this, [this] { refresh(); });
    connect(m_mine, &QListWidget::itemSelectionChanged, this, [this] { refresh(); });
    connect(m_mine, &QListWidget::currentRowChanged, this, [this] { refresh(); });
    connect(m_presentation, &QListWidget::itemSelectionChanged, this, [this] { refresh(); });
    refresh();
}

// The slides selected on the left join the show at its end, in the order the presentation has them.
void CustomShowEditDialog::addSlides()
{
    for (int i = 0; i < m_presentation->count(); ++i) {
        QListWidgetItem *item = m_presentation->item(i);
        if (!item->isSelected()) continue;
        auto *it = new QListWidgetItem(item->text(), m_mine);
        it->setData(Qt::UserRole, item->data(Qt::UserRole));
    }
    refresh();
}

void CustomShowEditDialog::refresh()
{
    m_add->setEnabled(!m_presentation->selectedItems().isEmpty());
    m_remove->setEnabled(!m_mine->selectedItems().isEmpty());
    m_up->setEnabled(m_mine->currentRow() > 0);
    m_down->setEnabled(m_mine->currentRow() >= 0 && m_mine->currentRow() < m_mine->count() - 1);
    // A name, not another show's, and a slide in it.
    const QString name = m_name->text().trimmed();
    bool taken = false;
    for (const deck::CustomShow &s : std::as_const(m_all))
        if (s.id != m_show.id || m_new) taken |= s.name.compare(name, Qt::CaseInsensitive) == 0;
    if (m_ok) m_ok->setEnabled(!name.isEmpty() && !taken && m_mine->count() > 0);
}

deck::CustomShow CustomShowEditDialog::show() const
{
    deck::CustomShow s = m_show;
    s.name = m_name->text().trimmed();
    s.slides.clear();
    for (int i = 0; i < m_mine->count(); ++i) s.slides << m_mine->item(i)->data(Qt::UserRole).toString();
    return s;
}

void CustomShowEditDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->matches(QKeySequence::HelpContents)) showDialogHelp(this, windowTitle());
    else QDialog::keyPressEvent(e);
}

// ---- the list

CustomShowsDialog::CustomShowsDialog(const Document &doc, const QVector<deck::CustomShow> &shows, QWidget *parent) : QDialog(parent), m_doc(doc), m_shows(shows)
{
    setWindowTitle(tr("Custom Shows"));
    setObjectName(QStringLiteral("customShowsDialog"));
    auto *lay = new QVBoxLayout(this);
    lay->addWidget(new QLabel(tr("Custom shows:")));
    auto *row = new QHBoxLayout();
    m_list = new QListWidget();
    m_list->setObjectName(QStringLiteral("customShowList"));
    m_list->setAccessibleName(tr("Custom shows"));
    m_list->setMinimumSize(260, 180);
    row->addWidget(m_list, 1);
    auto *column = new QVBoxLayout();
    auto make = [&](const QString &name, const QString &text) {
        auto *b = new QPushButton(text);
        b->setObjectName(name);
        column->addWidget(b);
        return b;
    };
    m_new = make(QStringLiteral("newShow"), tr("New…"));
    m_edit = make(QStringLiteral("editShow"), tr("Edit…"));
    m_copy = make(QStringLiteral("copyShow"), tr("Copy"));
    m_remove = make(QStringLiteral("removeShow"), tr("Remove"));
    column->addStretch(1);
    row->addLayout(column);
    lay->addLayout(row, 1);
    auto *buttons = new QHBoxLayout();
    buttons->addStretch(1);
    m_play = new QPushButton(tr("Show"));
    m_play->setObjectName(QStringLiteral("playShow"));
    auto *close = new QPushButton(tr("Close"));
    close->setObjectName(QStringLiteral("closeShows"));
    close->setDefault(true);
    buttons->addWidget(m_play);
    buttons->addWidget(close);
    lay->addLayout(buttons);

    connect(m_new, &QPushButton::clicked, this, [this] {
        CustomShowEditDialog dlg(m_doc, m_shows, nullptr, this);
        if (dlg.exec() != QDialog::Accepted) return;
        m_shows << dlg.show();
        refill(int(m_shows.size()) - 1);
    });
    connect(m_edit, &QPushButton::clicked, this, [this] { edit(current()); });
    connect(m_list, &QListWidget::itemDoubleClicked, this, [this] { edit(current()); });
    connect(m_copy, &QPushButton::clicked, this, [this] {
        const int at = current();
        if (at < 0) return;
        deck::CustomShow copy = m_shows[at];
        copy.id = deck::newCustomShowId(m_shows);
        QString name = tr("Copy of %1").arg(m_shows[at].name);
        for (int n = 2;; ++n) {
            bool taken = false;
            for (const deck::CustomShow &s : std::as_const(m_shows)) taken |= s.name.compare(name, Qt::CaseInsensitive) == 0;
            if (!taken) break;
            name = tr("Copy %1 of %2").arg(n).arg(m_shows[at].name);
        }
        copy.name = name;
        m_shows.insert(at + 1, copy);
        refill(at + 1);
    });
    connect(m_remove, &QPushButton::clicked, this, [this] {
        const int at = current();
        if (at < 0) return;
        m_shows.removeAt(at);
        refill(std::min(at, int(m_shows.size()) - 1));
    });
    connect(m_play, &QPushButton::clicked, this, [this] {
        const int at = current();
        if (at < 0) return;
        m_playId = m_shows[at].id;
        accept();
    });
    connect(close, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_list, &QListWidget::currentRowChanged, this, [this] { refresh(); });
    refill(m_shows.isEmpty() ? -1 : 0);
}

int CustomShowsDialog::current() const
{
    const int row = m_list->currentRow();
    return row >= 0 && row < m_shows.size() ? row : -1;
}

void CustomShowsDialog::edit(int index)
{
    if (index < 0) return;
    CustomShowEditDialog dlg(m_doc, m_shows, &m_shows[index], this);
    if (dlg.exec() != QDialog::Accepted) return;
    m_shows[index] = dlg.show();
    refill(index);
}

void CustomShowsDialog::refill(int select)
{
    const QSignalBlocker block(m_list);
    m_list->clear();
    for (const deck::CustomShow &s : std::as_const(m_shows)) m_list->addItem(s.name);
    m_list->setCurrentRow(select);
    refresh();
}

void CustomShowsDialog::refresh()
{
    const bool any = current() >= 0;
    for (QPushButton *b : {m_edit, m_copy, m_remove, m_play}) b->setEnabled(any);
}

void CustomShowsDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->matches(QKeySequence::HelpContents)) showDialogHelp(this, windowTitle());
    else QDialog::keyPressEvent(e);
}

} // namespace jp
