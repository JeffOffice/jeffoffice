#pragma once
// The Custom Views box: the workbook's named views in a list, a name to type and Add (the window makes the view from how the workbook looks now; a view with the same name is replaced), Delete,
// and Show. The window asks for it through AnchorHooks::customViewsAsker (a test gives its own).

#include "sheet/customviews.h"

#include <QDialog>
#include <QVector>

#include <functional>

class QLineEdit;
class QListWidget;
class QPushButton;

namespace jp {

struct CustomViewsRequest {
    QVector<jp::office::sheet::CustomView> views;   // in: the views now; out: the views after
    // Makes a view of how the workbook looks now, under a name.
    std::function<jp::office::sheet::CustomView(const QString &)> capture;
    QString show;                                    // out: the name of the view to show (empty for none)
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
    QPushButton *m_add = nullptr, *m_delete = nullptr, *m_show = nullptr;
};

} // namespace jp
