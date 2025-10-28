#ifndef CUSTONTHREAD_H
#define CUSTONTHREAD_H

#include <QWidget>
#include<QThread>
class CustonThread : public QThread
{
    Q_OBJECT
public:
    CustonThread(QWidget *parent);
protected:
    void run() override;
signals:
    void threadTimeout();

};

#endif // CUSTONTHREAD_H
