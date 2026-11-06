#pragma once
// The Custom Views box: the workbook's named views in a list, a name to type and Add (the window makes the view from how the workbook looks now; a view with the same name is replaced), Delete,
// and Show. The window asks for it through AnchorHooks::customViewsAsker (a test gives its own).

#include "sheet/customviews.h"

#include <QDialog>
#include <QVector>

#include <functional>

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;

namespace jp {

struct CustomViewsRequest {
    QVector<jp::office::sheet::CustomView> views;   // in: the views now; out: the views after
    // Makes a view of how the workbook looks now, under a name; false, with what is the matter, when it can't be kept whole.
    std::function<bool(const QString &, jp::office::sheet::CustomView *, QString *)> capture;
    int show = -1;                                   // out: the place in `views` of the view to show (-1 for none)
};

class CustomViewsDialog : public QDialog {
    Q_OBJECT
public:
    explicit CustomViewsDialog(const CustomViewsRequest &request, QWidget *parent = nullptr);
    CustomViewsRequest result() const;

private:
    void refresh();
    void addView();
    void deleteView();
    void showView();
    CustomViewsRequest m_request;
    QListWidget *m_list = nullptr;
    QLineEdit *m_name = nullptr;
    QLabel *m_problem = nullptr;
    QPushButton *m_add = nullptr, *m_delete = nullptr, *m_show = nullptr;
};

} // namespace jp
