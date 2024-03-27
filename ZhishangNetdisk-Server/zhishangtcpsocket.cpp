#include "zhishangtcpsocket.h"

void ZhishangTcpSocket::ReceiveMsg()
{
    qDebug() << "接收到的数据长度：" << bytesAvailable();
    // 读取当前接收到的所有数据
    QByteArray data = readAll();
    // 将读取到的数据存入缓冲区
    buffer.append(data);

    while(buffer.size() >= sizeof(PDU))
    {
        // 将缓冲区的指针转换为PDU*类型
        PDU* rPDU = (PDU*) buffer.data();
        int MsgLen = rPDU->MsgLen;

        // 如果缓冲区大小小于消息大小，那么直接跳出循环
        if(MsgLen > buffer.size())
            break;

        // 如果有一个完整的PDU的话那就继续处理
        qDebug() << "读取PDU";
        qDebug() << "PDU 总长度：" << rPDU->PDULen;
        qDebug() << "PDU 消息长度：" << rPDU->MsgLen;
        qDebug() << "PDU 消息类型：" << rPDU->MsgType;
        qDebug() << "PDU 参数1：" << rPDU->ParaData;
        qDebug() << "PDU 参数2：" << rPDU->ParaData + 24;
        qDebug() << "PDU 参数3：" << rPDU->ParaData + 48;
        qDebug() << "PDU 消息内容：" << rPDU->Msg;

        PDU *sPDU = ProcessPDU(rPDU);

        // 此时PDU已经被处理完成，缓冲区删除对应数据
        buffer.remove(0, rPDU->PDULen);

        SendPDU(sPDU);
    }
}

void ZhishangTcpSocket::ReadyOffline()
{
    //if(!CurrentUserName.isEmpty() && CurrentUserID != 0)
        // 调用数据库处理函数处理下线的数据库修改
        DatabaseOperate::GetInstance().ProcessOffline(CurrentUserName.toStdString().c_str());
    // 发出用户下线的信号，注意后续需要删除当前socket，所以需要传递this指针
    qDebug() << "emit UserOffline(this);";
    emit UserOffline(this);

}

ZhishangTcpSocket::ZhishangTcpSocket()
    : SourceFilePath("")
{
    // 新建新的处理类对象
    MsgProcessor = new MsgHandler;
    // 连接接收消息槽函数和可以读取socket信号
    connect(this, &QTcpSocket::readyRead, this, &ZhishangTcpSocket::ReceiveMsg);
    // 连接断开链接信号和准备离线槽函数
    connect(this, &QTcpSocket::disconnected, this, &ZhishangTcpSocket::ReadyOffline);
}

PDU *ZhishangTcpSocket::ProcessPDU(PDU *rPDU)
{
    if(rPDU == NULL)
        return NULL;
    else
    {
        // error: cannot jump from switch statement to this case label
        // note: jump bypasses variable initialization
        // C++ 中switch语句不允许跳过变量初始化，需要添加花括号来限制每个case语句的作用域
        switch(rPDU->MsgType)
        {
            case MSG_TYPE_REGISTER_REQUEST:
                return MsgProcessor->ProcessRegisterRequest(rPDU);

            case MSG_TYPE_LOGIN_REQUEST:
                return MsgProcessor->ProcessLoginRequest(rPDU, CurrentUserName, CurrentUserID, CurrentUserFolderPath);

            case MSG_TYPE_SEARCH_USER_REQUEST:
                return MsgProcessor->ProcessSearchUserRequest(rPDU);

            case MSG_TYPE_SHOW_ONLINE_USERS_REQUEST:
                return MsgProcessor->ProcessShowOnlieneUserRequest();

            case MSG_TYPE_ADD_FRIEND_REQUEST:
                return MsgProcessor->ProcessAddFriendRequest(rPDU);

            case MSG_TYPE_ADD_FRIEND_RESPOND:
                return MsgProcessor->ProcessAddFriendRespond(rPDU);

            case MSG_TYPE_REFRESH_FRIEND_LIST_REQUEST:
                return MsgProcessor->ProcessRefreshFriendListRequest(rPDU);

            case MSG_TYPE_DELETE_FRIEND_REQUEST:
                return MsgProcessor->ProcessDeleteFriendRequest(rPDU);

            case MSG_TYPE_CHAT:
                return MsgProcessor->ProcessChat(rPDU);

            case MSG_TYPE_NEW_FOLDER_REQUEST:
                return MsgProcessor->ProcessNewFolderRequest(rPDU, CurrentUserFolderPath);

            case MSG_TYPE_REFRESH_FILES_LIST_REQUEST:
                return MsgProcessor->ProcessRefreshFolderRequest(rPDU, CurrentUserFolderPath);

            case MSG_TYPE_DELETE_FOLDER_REQUEST:
                return MsgProcessor->ProcessDeleteFolderRequest(rPDU, CurrentUserFolderPath);

            case MSG_TYPE_DELETE_FILE_REQUEST:
                return MsgProcessor->ProcessDeleteFileRequest(rPDU, CurrentUserFolderPath);

            case MSG_TYPE_RENAME_REQUEST:
                return MsgProcessor->ProcessRenameRequest(rPDU, CurrentUserFolderPath);

            case MSG_TYPE_MOVE_REQUEST:
                return MsgProcessor->ProcessMoveRequest(rPDU, CurrentUserFolderPath);

            case MSG_TYPE_UPLOAD_FILE_REQUEST:
                return MsgProcessor->ProcessUploadRequest(rPDU, CurrentUserFolderPath);

            case MSG_TYPE_UPLOAD_FILE_DATA:
                return MsgProcessor->ProcessUploadFileSendDataRequest(rPDU);

            case MSG_TYPE_UPLOAD_FILE_ERROR:
                return MsgProcessor->ProcessUploadFileError(rPDU);

            case MSG_TYPE_DOWNLOAD_FILE_REQUEST:
                return MsgProcessor->ProcessDownloadRequest(rPDU, CurrentUserFolderPath);

            case MSG_TYPE_DOWNLOAD_FILE_READY:
                return MsgProcessor->ProcessDonloadReady(this);

            case MSG_TYPE_SHARE_FILE_REQUEST:
                return MsgProcessor->ProcessShareFileRequest(rPDU, CurrentUserFolderPath);

            case MSG_TYPE_SHARE_FILE_RESPOND:
                return MsgProcessor->ProcessShareFileRespond(rPDU, CurrentUserFolderPath, SourceFilePath);
        }
            return NULL;
    }
}

void ZhishangTcpSocket::SendPDU(PDU *sPDU)
{
    if(sPDU == NULL)
        return;
    // socket就在tcpcocket类中，用this或者直接write都行
    write((char*)sPDU, sPDU->PDULen);

    qDebug() << "发送PDU";
    qDebug() << "PDU 总长度：" << sPDU->PDULen;
    qDebug() << "PDU 消息长度：" << sPDU->MsgLen;
    qDebug() << "PDU 消息类型：" << sPDU->MsgType;
    qDebug() << "PDU 参数1：" << sPDU->ParaData;
    qDebug() << "PDU 参数2：" << sPDU->ParaData + 24;
    qDebug() << "PDU 参数3：" << sPDU->ParaData + 48;
    qDebug() << "PDU 消息内容：" << sPDU->Msg;
    // 用完释放sPDU
    delete sPDU;
    sPDU = NULL;
}

QString ZhishangTcpSocket::GetCurrentUserName()
{
    return CurrentUserName;
}

QString ZhishangTcpSocket::GetCurrentUserFolderPath()
{
    return CurrentUserFolderPath;
}

void ZhishangTcpSocket::SetCurrentUserFolderPath(const QString &PathName)
{
    CurrentUserFolderPath = PathName;
}

QString ZhishangTcpSocket::GetSourceFilePath()
{
    return SourceFilePath;
}

void ZhishangTcpSocket::SetSourceFilePath(QString SourceFilePath)
{
    this->SourceFilePath = SourceFilePath;
}

ZhishangTcpSocket::~ZhishangTcpSocket()
{
    if(MsgProcessor != NULL)
        delete MsgProcessor;
    MsgProcessor = NULL;
}
