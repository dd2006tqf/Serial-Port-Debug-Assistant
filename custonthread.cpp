#include "custonthread.h"

CustonThread::CustonThread(QWidget *parent):QThread(parent)
{

}

void CustonThread::run()
{
    while(1)
    {
        msleep(1000);
        emit threadTimeout();
    }
}
