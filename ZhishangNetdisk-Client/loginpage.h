#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include "zhishangprotocol.h"
#include "msghandler.h"

#include <QTcpSocket>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginPage;
}
QT_END_NAMESPACE

class LoginPage : public QWidget
{
    Q_OBJECT

public:
    // 为了保证单例模式只有一个实例被创建，构造方法需要钡放在私有中
    // LoginPage(QWidget *parent = nullptr);
    ~LoginPage();

    // 添加载入配置文件函数
    void LoadConfig();

    // 将客户端设计为单例模式
    // 单例模式(Singleton Pattern)，使用最广泛的设计模式之一。
    // 其意图是保证一个类仅有一个实例被构造，并提供一个访问它的全局访问接口，该实例被程序的所有模块共享。
    // 大致的做法为：
    //  1. 定义一个单例类；
    //  2. 私有化构造函数，防止外界直接创建单例类的对象；
    //  3. 禁用拷贝构造，移动赋值等函数，可以私有化，也可以直接使用=delete；
    //  4. 使用一个公有的静态方法获取该实例；
    //  5. 确保在第一次调用之前该实例被构造。
    // 关于构造时机的不同，有以下两种习惯的称呼：
    //  饿汉模式（Eager Singleton），在程序启动后立刻构造单例；
    //  懒汉模式（Lazy Singleton），在第一次调用前构造单例。

    // 定义公有的静态方法获取实例
    // 静态函数不需要通过实例去调用
    // 静态成员函数：
    //  静态成员函数是类的一部分，但是它可以在没有创建类的对象的情况下被调用。静态成员函数只能访问静态成员变量，不能访问类的非静态成员变量。
    static LoginPage& GetInstance();

    // 外部获取CLientSocket的方法
    QTcpSocket& GetSocket();

    // 外部获取用户名
    QString& GetCurrentUser();

    PDU* ReadPDU();
    void ProcessPDU(PDU *rPDU);
    void SendPDU(PDU *sPDU);

// 槽函数
public slots:

    // 创建槽函数来处理连接的情况
    void ShowConnect();
    void ReceiveMsg();


private slots:

    void on_RegisterPB_clicked();
    void on_LoginPB_clicked();

private:
    Ui::LoginPage *ui;

    // 服务器端口号和IP
    // QString ServerIP;
    QString ServerHost;
    quint16 ServerPort;

    // 创建客户端套接字，用于和服务器通信
    QTcpSocket ClientSocket;

    // 单例模式，保证在一个进程中只能运行一个客户端
    // 单例模式实现原理
    //      静态局部变量 -> 初始化一次，生命周期从声明到函数结束
    //      返回引用 -> 确保每次返回的都是同一个实例
    //      限制条件 -> 构造函数私有，拷贝构造和赋值符号删除，保证不会创建多个实例
    // 构造方法放在私有中，防止类外创建实例
    // 除了通过构造函数以外，可能还可以通过拷贝构造或者赋值运算符创建
    // 所以要删除拷贝构造和赋值运算符
    // 效果
    //  保证只能通过GetInstance方法获取实例
    LoginPage(QWidget *parent = nullptr);
    // 删除拷贝构造
    LoginPage(const LoginPage& instance) = delete;
    // 删除赋值运算符
    LoginPage& operator=(const LoginPage&) = delete;
    // 创建消息处理实例
    MsgHandler *MsgProcessor;
    // 存储当前用户名，添加好友的时候需要用到
    QString CurrentUser;
    // 缓冲区
    QByteArray buffer;
};
#endif // LOGINPAGE_H
