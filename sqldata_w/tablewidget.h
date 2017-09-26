#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include <QTableWidget>

namespace Ui {
    class TableWidget;
}

class TableWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit TableWidget(QWidget *parent = 0);
    ~TableWidget();

    void keyPressEvent(QKeyEvent * event);
};

#endif // TABLEWIDGET_H
