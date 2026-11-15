#pragma once
// Options > Save, one page for every app: the default save format (one of the app's own kinds of file), how often the automatic recovery information is saved (every 1 to 120
// minutes), and the default folder that Save As and Open start in. An app adds its own choices to the page (the box about personal information, say) with `more` and `applyMore`.

#include "app/optionsdialog.h"

#include <QString>
#include <QVector>
#include <functional>

class QFormLayout;
class QWidget;

namespace jp {

struct SaveFormat {
    QString key;     // the suffix of the kind, as the app tells it from the setting ("docx")
    QString label;   // as the box shows it
};

// The page. `more` adds rows to the page's form when it is made (its widget is the page); `applyMore` runs when OK is pressed, after the page's own choices are saved.
OptionsPage saveOptionsPage(const QVector<SaveFormat> &formats, const std::function<void(QFormLayout *form, QWidget *page)> &more = nullptr, const std::function<void()> &applyMore = nullptr);

} // namespace jp
