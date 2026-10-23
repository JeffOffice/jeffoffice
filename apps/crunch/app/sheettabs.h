#pragma once
// The row of sheet tabs under the grid: a tab for each sheet that is shown (hidden and very hidden sheets are kept out), the color a sheet asks for as a stripe under its name.

#include <QColor>
#include <QTabBar>
#include <QVector>

namespace jp::crunch {

class SheetTabBar : public QTabBar {
    Q_OBJECT
public:
    explicit SheetTabBar(QWidget *parent = nullptr);
    // The color of a tab's stripe (invalid for none).
    void setStripe(int tab, const QColor &color);
    QColor stripe(int tab) const { return m_stripes.value(tab); }
    void clearTabs();

protected:
    void paintEvent(QPaintEvent *e) override;
    QSize tabSizeHint(int index) const override;

private:
    QVector<QColor> m_stripes;
};

} // namespace jp::crunch
