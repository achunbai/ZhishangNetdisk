#include "zhishangtcpserver.h"

ZhishangTcpServer::ZhishangTcpServer() {}

ZhishangTcpServer &ZhishangTcpServer::GetInstance()
{
    // 获取实例函数返回静态局部对象的引用
    static ZhishangTcpServer instance;
    return instance;
}

// 其实此处的handle就是接收到套接字描述符
// 因为QTcpSocket中有些函数也需要重写，所以此时还需要新建一个自定义的类
void ZhishangTcpServer::incomingConnection(qintptr handle)
{
    qDebug() << "您有一个新的客户端连接";
    // 新建立一个socket
    ZhishangTcpSocket *pTcpSocket = new ZhishangTcpSocket;
    // 将socket与套接字描述符关联起来
    pTcpSocket->setSocketDescriptor(handle);
    // 将连接的Tcp客户端储存，重写QTcpSocket中的函数，需要新建一个自定义的类
    ClientTcpSocketList.append(pTcpSocket);

    // 连接下线信号和下线处理函数，之前定义了一个ZhishangTcpSocket类型的pTcpSocket对象，所以此时发出信号的是pTcpSocket
    // 将pTcpSocket发出的UserOffline信号与ProcessUserOffline函数进行连接
    connect(pTcpSocket, &ZhishangTcpSocket::UserOffline, this, &ZhishangTcpServer::ProcessUserOffline);
    qDebug() << "connect(pTcpSocket, &ZhishangTcpSocket::UserOffline, this, &ZhishangTcpServer::ProcessUserOffline);";
}

void ZhishangTcpServer::ProcessUserOffline(ZhishangTcpSocket *CurrentSocket)
{
    qDebug() << "ProcessUserOffline";
    // 从当前客户端连接列表中删除离线的Socket这里传的是指针，就别用引用了
    ClientTcpSocketList.removeOne(CurrentSocket);
    // 一定要稍后删除，立即删除会引发问题
    CurrentSocket->deleteLater();
}

void ZhishangTcpServer::ForwardPDU(PDU *fPDU, char* TargetUserName)
{
    if(TargetUserName == NULL || fPDU == NULL)
        return;
    for(int i = 0; i < ClientTcpSocketList.size(); i++)
    {
        if(TargetUserName == ClientTcpSocketList.at(i)->GetCurrentUserName())
        {
            ClientTcpSocketList.at(i)->write((char*)fPDU, fPDU->PDULen);
            qDebug() << "转发消息成功";
            return;
        }
    }
    qDebug() << "转发消息失败";
}

int ZhishangTcpServer::ForwardPDU(PDU *fPDU, char *TargetUserName, QString SourceFilePath)
{
    if(TargetUserName == NULL || fPDU == NULL)
        return -1;
    for(int i = 0; i < ClientTcpSocketList.size(); i++)
    {
        if(TargetUserName == ClientTcpSocketList.at(i)->GetCurrentUserName())
        {
            qDebug() << "ClientTcpSocketList.at(i)->GetSourceFilePath().isEmpty()" << ClientTcpSocketList.at(i)->GetSourceFilePath().isEmpty();
            if(!ClientTcpSocketList.at(i)->GetSourceFilePath().isEmpty())
                return -2;
            ClientTcpSocketList.at(i)->SetSourceFilePath(SourceFilePath);
            qDebug() << "ClientTcpSocketList.at(i)->GetSourceFilePath() " << ClientTcpSocketList.at(i)->GetSourceFilePath();
            ClientTcpSocketList.at(i)->write((char*)fPDU, fPDU->PDULen);
            qDebug() << "转发消息成功";
            return 0;
        }
    }
    qDebug() << "转发消息失败";
    return -1;
}
