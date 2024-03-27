#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include <QDebug>
#include <QFile>
#include "zhishangprotocol.h"

// 防止循环依赖，前向声明ZhishangTcpSocket
// 在cpp中再包含头文件
class ZhishangTcpSocket;

class MsgHandler
{
public:
    MsgHandler();
    PDU *ProcessRegisterRequest(const PDU *rPDU);
    PDU *ProcessLoginRequest(const PDU *rPDU, QString& CurrentUserName, int& CurrentUserID, QString& CurrentUserFolderPath);
    PDU *ProcessSearchUserRequest(const PDU *rPDU);
    PDU *ProcessShowOnlieneUserRequest();
    PDU *ProcessAddFriendRequest(PDU *rPDU);
    PDU *ProcessAddFriendRespond(const PDU *rPDU);
    PDU *ProcessRefreshFriendListRequest(const PDU *rPDU);
    PDU *ProcessDeleteFriendRequest(const PDU *rPDU);
    PDU *ProcessChat(PDU *rPDU);
    PDU *ProcessNewFolderRequest(const PDU *rPDU, const QString &CurrentUserFolderPath);
    PDU *ProcessRefreshFolderRequest(const PDU *rPDU, const QString &CurrentUserFolderPath);
    PDU *ProcessDeleteFolderRequest(const PDU *rPDU, const QString &CurrentUserFolderPath);
    PDU *ProcessDeleteFileRequest(const PDU *rPDU, const QString &CurrentUserFolderPath);
    PDU *ProcessRenameRequest(const PDU *rPDU, const QString &CurrentUserFolderPath);
    PDU *ProcessMoveRequest(const PDU *rPDU, const QString &CurrentUserFolderPath);
    PDU *ProcessUploadRequest(const PDU *rPDU, const QString &CurrentUserFolderPath);
    PDU *ProcessUploadFileSendDataRequest(const PDU *rPDU);
    PDU *ProcessUploadFileError(const PDU *rPDU);
    PDU *ProcessDownloadRequest(const PDU *rPDU, const QString &CurrentUserFolderPath);
    PDU *ProcessDonloadReady(ZhishangTcpSocket *CurrentSocket);
    PDU *ProcessShareFileRequest(const PDU *rPDU, const QString &CurrentUserFolderPath);
    PDU *ProcessShareFileRespond(PDU *rPDU, const QString &CurrentUserFolderPath, QString &SourceFilePath);

private:
    QFile FileToReceive;
    bool IsUploading;
    qint64 SizeToReceive;
    qint64 SizeReceived;
    bool IsDownloading;
    QFile FileToSend;

    bool ProcessCopyFolder(const QString &SourceFilePath, const QString TargetFilePath);
};

#endif // MSGHANDLER_H
