#ifndef ZHISHANGTCPSERVER_H
#define ZHISHANGTCPSERVER_H

#include <QTcpServer>
#include "zhishangtcpsocket.h"

// 重写函数，继承QTcpServer父类，重写子类中的实现
class ZhishangTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    // 私有化构造函数
    // ZhishangTcpServer();

    // 定义获取实例的静态函数，实例的全局访问节点
    static ZhishangTcpServer& GetInstance();

    // 当有链接传入时，会自动调用incomingConnection，此时需要重写这个函数
    void incomingConnection(qintptr handle);

    // 定义转发PDU函数
    void ForwardPDU(PDU *fPDU, char* TargetUserName);
    int ForwardPDU(PDU *fPDU, char* TargetUserName, QString SourceFilePath);

public slots:
    // 定义用户离线的处理函数
    void ProcessUserOffline(ZhishangTcpSocket *CurrentSocket);

private:
    // 单例模式，一个进程中只允许一个服务端
    // 私有化构造函数
    ZhishangTcpServer();
    // 防止通过拷贝构造和赋值来获取新的实例，删除拷贝构造和赋值运算符
    ZhishangTcpServer(const ZhishangTcpServer& instance) = delete;
    ZhishangTcpServer& operator=(const ZhishangTcpServer&) = delete;

    // 服务端需要面对多个客户端，故需要建立一个Socket列表来保存所有连接上的客户端
    QList<ZhishangTcpSocket*> ClientTcpSocketList;

};

#endif // ZHISHANGTCPSERVER_H
