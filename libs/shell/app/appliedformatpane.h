#pragma once
// A pane that shows what is applied where the caret is and where it comes from: a few sections, each with a title (a kind of style), the name of the style the text has, a line for what the
// text says besides it, and a button that takes that off. An app fills the sections from its own text and runs the buttons; the pane knows nothing of styles (Write's Style Inspector).

#include "core/export.h"
#include <QStringList>
#include <QWidget>
#include <QVector>

class QLabel;
class QPushButton;

namespace jp {

class JO_COMMON_EXPORT AppliedFormatPane : public QWidget {
    Q_OBJECT
public:
    // `titles` and `buttons` have one entry per section (the button's text names what it takes off).
    AppliedFormatPane(const QStringList &titles, const QStringList &buttons, QWidget *parent = nullptr);
    // Section `i` shows `name` (`none` where it is empty), the `extras` the text says besides (one line, "Plus: ..."; nothing where empty), and its button is on when `canClear`.
    void setSection(int i, const QString &name, const QStringList &extras, bool canClear, const QString &none = QString());
    int sectionCount() const { return int(m_sections.size()); }
    QString nameOf(int i) const;
    QString extrasOf(int i) const;
    QPushButton *buttonOf(int i) const { return m_sections.value(i).button; }

Q_SIGNALS:
    void clearRequested(int section);

private:
    struct Section {
        QLabel *name = nullptr, *extras = nullptr;
        QPushButton *button = nullptr;
    };
    QVector<Section> m_sections;
};

} // namespace jp
