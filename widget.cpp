#include "widget.h"
#include "ui_widget.h"
#include"custonthread.h"
#include <QSerialPortInfo>
#include<QDebug>
#include<QComboBox>
#include<QIODevice>
#include<QString>
#include<QMessageBox>
#include<QTimer>
#include<QFileDialog>
#include<QDateTime>
#include<QChar>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    // 在 Widget 构造函数中
    lineEdits = findChildren<QLineEdit*>();
    checkBoxes = findChildren<QCheckBox*>();
    serialPort=new QSerialPort(this);
    int buttonsIndex=0;
    writeCntTotal=0;
    readCntTotal=0;
    serialStatus=false;
    //控件初始化
    ui->lineEdit_13->setEnabled(false);
    ui->checkBox_18->setEnabled(false);
    ui->checkBox_17->setEnabled(false);
    ui->pushButton_22->setEnabled(false);
    ui->checkBox_16->setEnabled(false);

    myThread = new CustonThread(this);
    timer =new QTimer(this);
    connect(myThread,&CustonThread::threadTimeout,this,&Widget::buttons_handler);

   // buttonsContimer=new QTimer(this);
   // connect(buttonsContimer,&QTimer::timeout,this,&Widget::buttons_handler);
    QTimer *getSysTimeTimer=new QTimer(this);
    connect(getSysTimeTimer,SIGNAL(timeout()),this,SLOT(time_reflash()));
    getSysTimeTimer->start(1000);

    connect(serialPort,&QSerialPort::readyRead,this,&Widget::on_SerialData_readyToRead);
    connect(timer,&QTimer::timeout,[=](){
        on_pushButton_22_clicked();
    });
    connect(ui->pushButton_15, &QPushButton::clicked, this, &Widget::on_pushButton_15_clicked);
    connect(ui->comboBox_6,&MyComboBox::refresh,this,&Widget::refreshSerialName);
    ui->comboBox_5->setCurrentIndex(6);
    ui->comboBox->setCurrentIndex(3);
    QList<QSerialPortInfo>serialList=QSerialPortInfo::availablePorts();
    for(QSerialPortInfo serialInfo:serialList)
    {
        qDebug()<<serialInfo.portName();
        ui->comboBox_6->addItem(serialInfo.portName());
    }

    for(int i=1;i<=11;i++)
    {
        QString btnName=QString("pushButton_%1").arg(i);
        QPushButton* btn= findChild<QPushButton*>(btnName);
        if(btn)
        {
            btn->setProperty("buttonId",i);
            buttons.append(btn);
            connect(btn,SIGNAL(clicked()),this,SLOT(on_command_button_clicked()));
        }
        QString lineEditName = QString("lineEdit_%1").arg(i);
        QLineEdit *lineEdit = findChild<QLineEdit *>(lineEditName);
        lineEdits.append(lineEdit);

        QString checkBoxName = QString("checkBox_%1").arg(i);
        QCheckBox *checkBox = findChild<QCheckBox *>(checkBoxName);
        checkBoxes.append(checkBox);
    }

}

Widget::~Widget()
{
    delete ui;
}
void Widget::on_pushButton_22_clicked()
{


        if (!serialPort->isOpen()) { // 先检查串口是否打开
            ui->label_12->setText("串口未打开");
            return;
        }
        QString currentData = ui->lineEdit_13->text(); // 获取当前要发送的内容
        QString timeStamp=getFormattedTime();
        QString sendWithTime=QString("[%1]%2").arg(timeStamp).arg(currentData);
        QByteArray sendData = currentData.toUtf8();
        int writeCnt = serialPort->write(sendData);

        if (writeCnt == -1) {
            ui->label_12->setText("send error");
        } else {
            writeCntTotal += writeCnt;
            qDebug() << "SendOK" << sendData;

            // 比对当前内容与上一次（sendback），不同时才显示
            if (currentData != sendBack) {
                ui->textEditRec->append(sendWithTime); // 发送记录区只显示新内容

                sendBack = currentData; // 更新sendback为当前内容
            }
            ui->textEditRev->append(sendWithTime);
            ui->label_12->setText("SendOk!");
            ui->label_14->setNum(writeCntTotal);
        }

}
void Widget::on_SerialData_readyToRead()
{
    QString revMessage = QString::fromUtf8(serialPort->readAll());
    qDebug()<<"revCnt:"<<revMessage.size();
    if (!revMessage.isEmpty()) {
        if(ui->checkBox_15->isChecked())
            revMessage.append("\r\n");
        if (ui->checkBox_16->isChecked()) {
            QByteArray tmpHexString = revMessage.toUtf8().toHex().toUpper();
            QString tmpStringHex = ui->textEditRev->toPlainText();
            tmpHexString = tmpStringHex.toUtf8() + tmpHexString;
            ui->textEditRev->setText(QString::fromUtf8(tmpHexString));
        } else {
            if (ui->checkBox_13->checkState() == Qt::Unchecked) {
                ui->textEditRev->insertPlainText(revMessage);
            } else if (ui->checkBox_13->checkState() == Qt::Checked) {
                getSysTime();
                ui->textEditRev->insertPlainText("[" + myTime + "] " + revMessage);
            }
        }
        readCntTotal += revMessage.size();
        ui->label_13->setText("Received:" + QString::number(readCntTotal));
    }
}

void Widget::on_btnOpen_clicked()
{
    if(!serialStatus){
    //1.选择端口号
    serialPort->setPortName(ui->comboBox_6->currentText());
    //2.配置波特率
     serialPort->setBaudRate(ui->comboBox_5->currentText().toInt());
    //3.配置数据位
      serialPort->setDataBits(QSerialPort::DataBits(ui->comboBox->currentText().toUInt()));
    //4.配置校验位
      switch(ui->comboBox_4->currentIndex()){
      case 0:
          serialPort->setParity(QSerialPort::NoParity);
          break;
      case 1: serialPort->setParity(QSerialPort::EvenParity);
          break;
      case 2: serialPort->setParity(QSerialPort::MarkParity);
          break;
      case 3: serialPort->setParity(QSerialPort::OddParity);
          break;
      case 4: serialPort->setParity(QSerialPort::SpaceParity);
          break;
      default:
           serialPort->setParity(QSerialPort::UnknownParity);
          break;
      }
      // 5. 配置停止位
      serialPort->setStopBits(QSerialPort::StopBits(ui->comboBox_2->currentText().toInt()));

      // 6. 流控
      if (ui->comboBox_3->currentText() == "None") {
          serialPort->setFlowControl(QSerialPort::NoFlowControl);
      }

      //7.打开串口
      if(serialPort->open(QIODevice::ReadWrite)){
         qDebug() << "串口打开成功，端口：" << ui->comboBox_6->currentText();
         ui->comboBox->setEnabled(false);
         ui->comboBox_2->setEnabled(false);
         ui->comboBox_3->setEnabled(false);
         ui->comboBox_4->setEnabled(false);
         ui->comboBox_5->setEnabled(false);
         ui->comboBox_6->setEnabled(false);
         ui->pushButton_22->setEnabled(true);
         ui->checkBox_16->setEnabled(true);
         ui->lineEdit_13->setEnabled(true);
         ui->btnOpen->setText("关闭串口");
         ui->checkBox_17->setEnabled(false);


         serialStatus=true;
      }
      else{
          QMessageBox msgBox;
          msgBox.setWindowTitle("打开串口错误");
          msgBox.setText("打开失败，串口可能被占用");
          msgBox.exec();
      }
    }else
    {
        serialPort->close();
         ui->btnOpen->setText("打开串口");
         ui->comboBox->setEnabled(true);
         ui->comboBox_2->setEnabled(true);
         ui->comboBox_3->setEnabled(true);
         ui->comboBox_4->setEnabled(true);
         ui->comboBox_5->setEnabled(true);
         ui->comboBox_6->setEnabled(true);
        serialStatus=false;
        ui->pushButton_22->setEnabled(false);
        ui->checkBox_16->setEnabled(false);
        ui->checkBox_16->setCheckState(Qt::Unchecked);
        timer->stop();
        ui->lineEdit_12->setEnabled(true);
        ui->lineEdit_13->setEnabled(false);
        qDebug() << "串口关闭成功，端口：" << ui->comboBox_6->currentText();
    }
};

void Widget::on_checkBox_16_clicked(bool checked)
{
    qDebug()<<"";
    if(checked)
    {
        ui->lineEdit_12->setEnabled(false);
        ui->lineEdit_13->setEnabled(false);
        timer->start(ui->lineEdit_12->text().toInt());
    }
    else
    {
        timer->stop();
        ui->lineEdit_12->setEnabled(true);
        ui->lineEdit_13->setEnabled(true);
    }
}

void Widget::on_pushButton_16_clicked()
{
    ui->textEditRec->setText("");
    ui->textEditRev->setText("");
}

void Widget::on_pushButton_17_clicked()
{
    QString fileName=QFileDialog::getSaveFileName(this,tr("save file"),
                                                  "D:/qtexercise/serialdata.txt",
                                                  tr("Text(*.txt"));
    if(fileName!=NULL)
    {
        QFile file(fileName);
        if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
            return;
        QTextStream out(&file);
        out<<ui->textEditRev->toPlainText();
        file.close();
    }
}

void Widget::time_reflash()
{
    getSysTime();
    ui->label_15->setText(myTime);

}
QString Widget::getSysTime()
{
    QDateTime currentTime = QDateTime::currentDateTime();

       QDate date = currentTime.date();
       int year = date.year();
       int month = date.month();
       int day = date.day();

       QTime time = currentTime.time();
       int hour = time.hour();
       int minute = time.minute();
       int second = time.second();

       myTime = QString("%1-%2-%3 %4:%5:%6")
               .arg(year,2,10,QChar('0'))
               .arg(month,2,10,QChar('0'))
               .arg(day,2,10,QChar('0'))
               .arg(hour,2,10,QChar('0'))
               .arg(minute,2,10,QChar('0'))
               .arg(second,2,10,QChar('0'));

       //qDebug() << "格式化后的时间：" << myTime;
       return myTime;
}

void Widget::on_checkBox_14_clicked(bool checked)
{
    if(checked)
    {
        QString tmp=ui->textEditRev->toPlainText();
        QByteArray qtmp=tmp.toUtf8();
        qtmp=qtmp.toHex();
        QString lastShow;
        tmp=QString::fromUtf8(qtmp);
        for(int i=0;i<tmp.size();i+=2)
        {
            lastShow+=tmp.mid(1,2)+" ";
        }
        ui->textEditRev->setText(QString::fromUtf8(qtmp));
    }
    else
    {
        QString tmpHexString =ui->textEditRev->toPlainText();
        QByteArray tmpHexQByteArray=tmpHexString.toUtf8();
        QByteArray tmpQByteString=QByteArray::fromHex(tmpHexQByteArray);
        ui->textEditRev->setText(QString::fromUtf8(tmpQByteString));
    }
}
QString Widget::getFormattedTime()
{
    QDateTime current=QDateTime::currentDateTime();
    return current.toString("yyyy-MM-dd hh:mm:ss");
}

void Widget::on_pushButton_20_clicked(bool checked)//隐藏面板
{
    if(checked)
    {
        ui->pushButton_20->setText("扩展面板");
        ui->groupBox->hide();
    }
    else
    {
        ui->pushButton_20->setText("隐藏面板");
        ui->groupBox->show();
    }
}



void Widget::on_pushButton_15_clicked(bool checked)
{
    if(checked)
        {
            ui->pushButton_15->setText("扩展历史");
            ui->groupBoxRecord->hide();
             ui->groupBox->hide();
        }
        else
        {
            ui->pushButton_15->setText("隐藏历史");
            ui->groupBoxRecord->show();
            ui->groupBox->show();
    }
}

void Widget::refreshSerialName()
{
    ui->comboBox_6->clear(); // 清空下拉框
        QList<QSerialPortInfo> serialInfoList = QSerialPortInfo::availablePorts(); // 获取可用串口列表
        for (const QSerialPortInfo &serialInfo : serialInfoList) {
            qDebug() << serialInfo.portName(); // 调试输出串口名称
            ui->comboBox_6->addItem(serialInfo.portName()); // 将串口名称添加到下拉框
        }
        ui->label_12->setText("Com Refresh"); // 更新状态标签
}

void Widget::on_pushButton_clicked()
{
    ui->lineEdit_13->setText(ui->lineEdit_1->text());
    on_pushButton_22_clicked();
    if(ui->checkBox_1->isChecked())
    {
        ui->checkBox_1->setChecked(true);
    }
}

void Widget::on_command_button_clicked()
{
    QPushButton *btn=qobject_cast<QPushButton *>(sender());
    if(btn)
    {
        int num=btn->property("buttonId").toInt();
        QString lineEditName=QString("lineEdit_%1").arg(num);
        QLineEdit *lineEdit=findChild<QLineEdit*>(lineEditName);
        if(lineEdit)
        {
            ui->lineEdit_13->setText(lineEdit->text());
        }
        QString checkBoxName=QString("checkBox_%1").arg(num);
        QCheckBox *checkBox=findChild<QCheckBox*>(checkBoxName);
        if(checkBox)
        {
            ui->checkBox_14->setChecked(checkBox->isChecked());
        }
        on_pushButton_22_clicked();
    }
}
void Widget::buttons_handler()
{
    if(buttonsIndex<buttons.size())
    {
        QPushButton*btnTemp=buttons[buttonsIndex];
        emit btnTemp->clicked();
        buttonsIndex++;
    }
    else
    {
        buttonsIndex=0;
    }
}
void Widget::on_checkBox_12_clicked(bool checked)
{
    if(checked)
    {
        ui->spinBox->setEnabled(false);
        myThread->start();
        //buttonsContimer->start(ui->spinBox->text().toUInt());
    }
    else
    {
        ui->spinBox->setEnabled(true);
        myThread->terminate();
        //buttonsContimer->stop();
    }
}

void Widget::on_pushButton_14_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("提示");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText("重置列表不可逆，确认是否重置？");
    //msgBox.setstandardButtons(QMessageBox::Yes | QMessageBox::No);
    QPushButton *yesButton = msgBox.addButton("是",QMessageBox::YesRole);
    QPushButton *noButton = msgBox.addButton("否",QMessageBox::NoRole);
    msgBox.exec();
    if(msgBox.clickedButton() == yesButton){
        qDebug() << "yesButton";
        for(int i=0;i<lineEdits.size();i++)
        {
        //遍历lineEdit，并清空内容
    lineEdits[i]->clear();
        //遍历checkBox，并取消勾选
    checkBoxes[i]->setChecked(false);
        }
    }
    if(msgBox.clickedButton() == noButton){
        qDebug() << "noButton";
    }


}

void Widget::on_pushButton_12_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    "D:/",
                                                    tr("Text (*.txt)"));
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);
    for (int i = 0; i < lineEdits.size(); i++) {
        out << checkBoxes[i]->isChecked() << "," << lineEdits[i]->text() << "\n";
    }
    file.close();
}

void Widget::on_pushButton_13_clicked()
{
    int i = 0;
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开文件"),
                                                    "D:/",
                                                    tr("文本类型 (*.txt)"));
    if (fileName != NULL) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;

        QTextStream in(&file);
        while (!in.atEnd() && i <= 11) {
            QString line = in.readLine();
            QStringList parts = line.split("|");
            if (parts.count() == 2) {
                checkBoxes[i]->setChecked(parts[0].toInt());
                lineEdits[i]->setText(parts[1]);
            }
            i++;
        }
    }
}
