#pragma once
// The dialogs of Slide Master view: Master Layout, which says which places the slide master has.

#include "core/masteredit.h"

#include <QDialog>

class QCheckBox;

namespace jp {

// The title, text, date, slide number, and footer places a slide master can have, one check box each.
class MasterLayoutDialog : public QDialog {
    Q_OBJECT
public:
    explicit MasterLayoutDialog(const deck::MasterParts &parts, QWidget *parent = nullptr);
    deck::MasterParts parts() const;

private:
    QCheckBox *m_title, *m_text, *m_date, *m_number, *m_footer;
};

} // namespace jp
