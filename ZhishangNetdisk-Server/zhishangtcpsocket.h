#ifndef ZHISHANGTCPSOCKET_H
#define ZHISHANGTCPSOCKET_H

#include <QTcpSocket>
#include "zhishangprotocol.h"
#include "databaseoperate.h"
#include "msghandler.h"

class ZhishangTcpSocket : public QTcpSocket
{
    Q_OBJECT

public slots:
    // 处理客户端请求在这里处理更合适，因为如果在tcpserver中处理，我们不知道到底是哪个客户端发送过来的数据
    // 而这个类则刚好是用来处理客户端连接的
    // 声明接收消息的槽函数
    void ReceiveMsg();

    // 声明处理下线的槽函数
    void ReadyOffline();

public:
    ZhishangTcpSocket();
    ~ZhishangTcpSocket();

    PDU* ReadPDU();
    PDU* ProcessPDU(PDU *rPDU);
    void SendPDU(PDU *sPDU);

    QString GetCurrentUserName();
    QString GetCurrentUserFolderPath();
    void SetCurrentUserFolderPath(const QString &FolderName);
    QString GetSourceFilePath();
    void SetSourceFilePath(QString SourceFilePath);

signals:
    // 声明用户下线发出的信号，注意后续删除用的指针
    void UserOffline(ZhishangTcpSocket* CurrentSocket);

private:
    MsgHandler *MsgProcessor;
    // 声明保存的该socket中的用户名
    QString CurrentUserName;
    QString CurrentUserFolderPath;
    int CurrentUserID = 0;
    // 创建缓存区，防止粘包和半包
    QByteArray buffer;

    // 分享
    // 保存来源的路径
    QString SourceFilePath;
};

#endif // ZHISHANGTCPSOCKET_H
