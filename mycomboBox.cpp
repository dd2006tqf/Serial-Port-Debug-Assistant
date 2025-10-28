#include"mycomboBox.h"
#include<QMouseEvent>
MyComboBox::MyComboBox(QWidget *parent):QComboBox(parent)
{

}
void MyComboBox::mousePressEvent(QMouseEvent *e)
{
    if(e->button()==Qt::LeftButton)
    {
        emit refresh();
    }
    QComboBox::mousePressEvent(e);
}
