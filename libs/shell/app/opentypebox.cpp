#include "app/opentypebox.h"

#include "text/opentype.h"
#include "text/textprops.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QFormLayout>
#include <QSignalBlocker>

#include <algorithm>
#include <iterator>

namespace jp {

namespace {
QString words(const char *text) { return QCoreApplication::translate("jp::OpenTypeBox", text); }

// The kinds of ligature each item of the box turns on (item 0 is the font's own choice, which has none of its own).
constexpr int kLigatureKinds[] = {-1, 0, tp::ot::kLigStandard, tp::ot::kLigStandard | tp::ot::kLigContextual,
                                  tp::ot::kLigHistorical | tp::ot::kLigDiscretionary, tp::ot::kLigKinds};
constexpr int kLigatureItems = int(std::size(kLigatureKinds));
}

OpenTypeBox::OpenTypeBox(QWidget *parent) : QGroupBox(QCoreApplication::translate("jp::OpenTypeBox", "OpenType features"), parent)
{
    setObjectName(QStringLiteral("openTypeBox"));
    auto *form = new QFormLayout(this);
    auto make = [&](Row &row, const char *name, const QString &label, int property, const QStringList &items) {
        row.box = new QComboBox(this);
        row.box->setObjectName(QString::fromLatin1(name));
        row.box->addItems(items);
        row.property = property;
        form->addRow(label, row.box);
        connect(row.box, &QComboBox::currentIndexChanged, this, &OpenTypeBox::edited);
    };
    make(m_ligatures, "ligatures", words("Ligatures:"), tp::Ligatures,
         {words("(default)"), words("None"), words("Standard only"), words("Standard and contextual"), words("Historical and discretionary"), words("All")});
    make(m_spacing, "numberSpacing", words("Number spacing:"), tp::NumberSpacing, {words("(default)"), words("Proportional"), words("Tabular")});
    make(m_forms, "numberForms", words("Number forms:"), tp::NumberStyle, {words("(default)"), words("Lining"), words("Old-style")});
    QStringList sets{words("(default)")};
    for (int i = 1; i <= tp::ot::kMostStylisticSet; ++i) sets << QString::number(i);
    make(m_set, "stylisticSet", words("Stylistic set:"), tp::StylisticSet, sets);
    make(m_alternates, "contextualAlternates", words("Contextual alternates:"), tp::ContextualAlts, {words("(default)"), words("On"), words("Off")});
}

int OpenTypeBox::valueAt(const Row &row, int index)
{
    if (index <= 0 || index >= row.box->count() - (row.unknown ? 1 : 0)) return -1;
    if (row.property == tp::Ligatures) return index < kLigatureItems ? tp::ot::ligatureValueOfKinds(kLigatureKinds[index]) : -1;
    return index;   // (the others: the item's place is its number)
}

int OpenTypeBox::itemOf(const Row &row, int value)
{
    if (row.property == tp::Ligatures) {
        const int kinds = tp::ot::ligatureKindsOf(value);
        for (int i = 1; i < kLigatureItems; ++i)
            if (kLigatureKinds[i] == kinds) return i;
        return -1;
    }
    const int most = row.property == tp::StylisticSet ? tp::ot::kMostStylisticSet : 2;
    return value >= 1 && value <= most ? value : value == 0 ? 0 : -1;
}

void OpenTypeBox::fill(Row &row, const QVector<QTextCharFormat> &formats)
{
    // (Where the text differs, or has a choice that has no item, the last item says so and the box changes nothing until another is picked.)
    bool mixed = false;
    int shown = 0;
    for (int i = 0; i < formats.size(); ++i) {
        const int item = formats[i].hasProperty(row.property) ? itemOf(row, formats[i].intProperty(row.property)) : 0;
        if (item < 0 || (i > 0 && item != shown)) {
            mixed = true;
            if (item < 0) shown = -1;
        } else if (i == 0) {
            shown = item;
        }
    }
    const QSignalBlocker block(row.box);
    row.unknown = mixed;
    if (mixed) {
        row.box->addItem(formats.size() > 1 ? words("(mixed)") : words("(other)"));
        shown = row.box->count() - 1;
    }
    row.box->setCurrentIndex(shown);
    row.was = shown;
}

void OpenTypeBox::setFormats(const QVector<QTextCharFormat> &formats)
{
    for (Row *r : rows()) {
        if (r->unknown) {
            const QSignalBlocker block(r->box);
            r->box->removeItem(r->box->count() - 1);
            r->unknown = false;
        }
        fill(*r, formats);
    }
}

OpenTypeBox::Change OpenTypeBox::change() const
{
    Change out;
    for (const Row *r : rows()) {
        const int now = r->box->currentIndex();
        if (now == r->was || (r->unknown && now == r->box->count() - 1)) continue;
        const int value = valueAt(*r, now);
        if (value < 0) out.clear << r->property;
        else out.set << qMakePair(r->property, value);
    }
    return out;
}

void OpenTypeBox::apply(const Change &change, QTextCharFormat *format)
{
    for (const auto &[property, value] : change.set) format->setProperty(property, value);
    for (const int property : change.clear) format->clearProperty(property);
}

QMap<QFont::Tag, quint32> OpenTypeBox::features() const
{
    QTextCharFormat f;
    for (const Row *r : rows()) {
        if (r->unknown && r->box->currentIndex() == r->box->count() - 1) continue;
        const int value = valueAt(*r, r->box->currentIndex());
        if (value >= 0) f.setProperty(r->property, value);
    }
    return tp::ot::features(f, false);
}

} // namespace jp
