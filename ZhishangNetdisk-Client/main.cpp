#include "loginpage.h"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 修改完私有后这两行会报错
    //  error: calling a private constructor of class 'LoginPage'
    // 达到了防止创建多个实例的效果，此时只能通过设定好的公有的静态方法获取实例
    // LoginPage w;
    // w.show();
    // 此时需要改成设定好的公有静态方法获取实例
    LoginPage::GetInstance().show();

    // 事件循环，相当于加入的while循环，防止其闪退
    return a.exec();
}
