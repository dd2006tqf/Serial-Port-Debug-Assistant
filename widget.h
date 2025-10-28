#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QSerialPort>
#include<QTimer>
#include<QList>
#include<QPushButton>
#include<QCheckBox>
#include "mycomboBox.h"
#include"custonthread.h"
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_22_clicked();
    void on_SerialData_readyToRead();
    void on_btnOpen_clicked();
    void on_checkBox_16_clicked(bool checked);
    void on_pushButton_16_clicked();
    void on_pushButton_17_clicked();
    void time_reflash();
    QString getSysTime();
    void on_checkBox_14_clicked(bool checked);
    void on_pushButton_20_clicked(bool checked);
    void on_pushButton_15_clicked(bool checked);
    void refreshSerialName();
    void on_pushButton_clicked();
    void on_command_button_clicked();
    void buttons_handler();
    void on_checkBox_12_clicked(bool checked);

    void on_pushButton_14_clicked();

    void on_pushButton_12_clicked();

    void on_pushButton_13_clicked();

private:
    Ui::Widget *ui;
    QSerialPort *serialPort;
    int writeCntTotal;
    int readCntTotal;
    int buttonsIndex;
    QString sendBack;
    bool serialStatus;
    QTimer* timer;
    QTimer *getSysTimeTimer;
    QString myTime;
    QString getFormattedTime();
    QList<QPushButton*>buttons;
    QList<QLineEdit*>lineEdits;
    QList<QCheckBox*>checkBoxes;
    QTimer *buttonsContimer;
    CustonThread *myThread;
};
#endif // WIDGET_H
