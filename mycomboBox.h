#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H


#include<QWidget>
#include<QComboBox>
class MyComboBox:public QComboBox
{
    Q_OBJECT;
public:
    MyComboBox(QWidget *parent);
protected:
    void mousePressEvent(QMouseEvent *e) override;
signals:
    void refresh();
};

#endif // MYCOMBOBOX_H
