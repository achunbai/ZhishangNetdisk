#include "msghandler.h"
#include "databaseoperate.h"
#include "zhishangtcpserver.h"
#include "zhishangtcpsocket.h"
#include "server_main.h"
#include <QDir>
#include <QProcess>
#include <algorithm>

MsgHandler::MsgHandler()
    : IsUploading(false)
    , SizeToReceive(0)
    , SizeReceived(0)
    , IsDownloading(0)
{}

PDU *MsgHandler::ProcessRegisterRequest(const PDU *rPDU)
{
    if(rPDU == NULL)
        return NULL;
    else
    {
        qDebug() << "MSG_TYPE_REGISTER_REQUEST";
        // 处理注册
        char UserName[24] = { 0 }, Password[32] = { 0 };
        memcpy(UserName, rPDU->ParaData, 24);
        memcpy(Password, rPDU->ParaData + 24, 32);

        // 调用函数返回注册结果
        int UID = 0;
        bool ret = DatabaseOperate::GetInstance().ProcessRegister(UserName, Password, UID);
        qDebug() << "ret = " << ret;

        // 如果注册成功，创建用户对应的数据文件夹
        if(ret && UID)
        {
            QDir UserData;
            QString Path = QString("%1/%2").arg(Server_Main::GetInstance().GetUserFilePath(), QString::number(UID));
            qDebug() << "Path = " << Path;
            bool stat = UserData.mkdir(Path);
            qDebug() << "UserData.mkdir(Path) = " << stat;
        }

        // 创建传回的PDU，注意使用CreatePDU避免没有初始化PDU导致的问题
        PDU* sPDU = CreatePDU(0);
        sPDU->MsgType = MSG_TYPE_REGISTER_RESPOND;
        // 记得将返回值写入消息
        memcpy(sPDU->ParaData, (char*)&ret, sizeof(bool));
        return sPDU;
    }
}

PDU *MsgHandler::ProcessLoginRequest(const PDU *rPDU, QString& CurrentUserName, int& CurrentUserID, QString& CurrentUserFolderPath)
{
    if(rPDU == NULL)
        return NULL;
    else
    {
        qDebug() << "MSG_TYPE_LOGIN_REQUEST";
        // 处理登录
        char UserName[24] = { 0 }, Password[32] = { 0 };
        memcpy(UserName, rPDU->ParaData, 24);
        memcpy(Password, rPDU->ParaData + 24, 32);

        // 调用函数返回注册结果
        int UID = 0;
        bool ret = DatabaseOperate::GetInstance().ProcessLogin(UserName, Password, UID);
        qDebug() << "ret = " << ret;
        // 如果登陆成功，那么保存用户名和用户ID
        if(ret && UID)
        {
            CurrentUserName = UserName;
            CurrentUserID = UID;
        }
        // 保存当前用户主路径
        CurrentUserFolderPath = QString("%1/%2").arg(Server_Main::GetInstance().GetUserFilePath(), QString::number(UID));
        // 创建传回的PDU，注意使用CreatePDU避免没有初始化PDU导致的问题
        PDU* sPDU = CreatePDU(0);
        sPDU->MsgType = MSG_TYPE_LOGIN_RESPOND;
        // 记得将返回值写入消息
        memcpy(sPDU->ParaData, (char*)&ret, sizeof(bool));
        return sPDU;
    }
}

PDU *MsgHandler::ProcessSearchUserRequest(const PDU *rPDU)
{
    if(rPDU == NULL)
        return NULL;
    else
    {
        qDebug() << "MSG_TYPE_SEARCH_USER_REQUEST";
        // 处理查询
        char UserName[24] = { 0 };
        memcpy(UserName, rPDU->ParaData, 24);

        // 调用函数返回搜索结果
        int ret = DatabaseOperate::GetInstance().ProcessSearchUser(UserName);
        qDebug() << "ret = " << ret;
        // 记录查询返回值，-1表示没找到，1表示在线，0表示离线
        // 创建传回的PDU，注意使用CreatePDU避免没有初始化PDU导致的问题
        PDU* sPDU = CreatePDU(0);
        sPDU->MsgType = MSG_TYPE_SEARCH_USER_RESPOND;
        // 记得将返回值写入消息
        memcpy(sPDU->ParaData, UserName, 24);
        memcpy(sPDU->ParaData + 24, (char*)&ret, sizeof(int));
        return sPDU;
    }
}

PDU *MsgHandler::ProcessShowOnlieneUserRequest()
{
    qDebug() << "MSG_TYPE_SHOW_ONLINE_USERS_REQUEST";
    // 执行查找，记得需要通过实例运行函数
    QStringList OnlineUsersList = DatabaseOperate::GetInstance().ProcessOnlineUsersList();
    int size = OnlineUsersList.size();
    // 创建传回的PDU，注意使用CreatePDU避免没有初始化PDU导致的问题
    PDU* sPDU = CreatePDU(size * 24);
    sPDU->MsgType = MSG_TYPE_SHOW_ONLINE_USERS_RESPOND;
    // 将返回值写入PDU，用户名长度不超过24，每次写入24字节会造成后面数据有随机值，影像显示，所以需要写入他的大小
    for(int i = 0; i < size; i++)
    {
        qDebug() << "OnlineUsersList.at(" << i << ").toStdString().size(): " << OnlineUsersList.at(i).toStdString().size();
        qDebug() << "OnlineUsersList.at(" << i << ")" << OnlineUsersList.at(i);
        memcpy(sPDU->Msg + i * 24, OnlineUsersList.at(i).toStdString().c_str(), 24);
    }
    return sPDU;
}

PDU *MsgHandler::ProcessAddFriendRequest(PDU *rPDU)
{
    if(rPDU == NULL)
        return NULL;
    else
    {
        qDebug() << "MSG_TYPE_ADD_FRIEND_REQUEST";
        // 获取两个用户名
        char CurrentUserName[24] = { 0 };
        char TargetUserName[24] = { 0 };
        memcpy(CurrentUserName, rPDU->ParaData, 24);
        memcpy(TargetUserName, rPDU->ParaData + 24, 24);

        // 调用函数返回搜索结果
        // 记录查询返回值，-1表示错误
        int ret = DatabaseOperate::GetInstance().ProcessFriendSearch(CurrentUserName, TargetUserName);
        qDebug() << "ret = " << ret;

        // 如果返回值为1，表示目标用户在线，调用转发
        if(ret == 1)
        {
            ZhishangTcpServer::GetInstance().ForwardPDU(rPDU, TargetUserName);
        }

        // 创建传回的PDU，注意使用CreatePDU避免没有初始化PDU导致的问题
        PDU *sPDU = CreatePDU(0);
        sPDU->MsgType = MSG_TYPE_ADD_FRIEND_RESPOND;
        // 记得将返回值写入消息
        // 目前的状态 -2 已经是好友 -1 错误 0 用户离线 1 用户在线
        memcpy(sPDU->ParaData, (char*)&ret, sizeof(int));
        return sPDU;
    }
}

PDU *MsgHandler::ProcessAddFriendRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return NULL;
    else
    {
        qDebug() << "MSG_TYPE_ADD_FRIEND_RESPOND";
        // 获取两个用户名
        char CurrentUserName[24] = { 0 };
        char TargetUserName[24] = { 0 };
        memcpy(CurrentUserName, rPDU->ParaData, 24);
        memcpy(TargetUserName, rPDU->ParaData + 24, 24);
        // 获取是否同意
        // 目前的状态 QMessageBox::Yes 同意 QMessageBox::No 拒绝
        int status;
        memcpy(&status, rPDU->ParaData + 48, sizeof(int));

        // 创建返回给添加方的PDU
        PDU *sPDU = CreatePDU(0);
        sPDU->MsgType = MSG_TYPE_ADD_FRIEND_RESULT;
        memcpy(sPDU->ParaData, rPDU->ParaData, 64);

        // int a = 2;
        // memcpy(sPDU->ParaData + 48, (char*)&a, sizeof(int));
        // return sPDU;

        int ret;
        if(status == QMessageBox::Yes)
        {
            // 只有对方同意了才需要将后续的状态告知对方，不然不需要创建转发的PDU
            PDU *fPDU = CreatePDU(0);
            fPDU->MsgType = MSG_TYPE_ADD_FRIEND_RESULT;
            memcpy(fPDU->ParaData, TargetUserName, 24);
            memcpy(fPDU->ParaData + 24, CurrentUserName, 24);
            // 同意的话执行数据库操作函数
            bool dbstat = DatabaseOperate::GetInstance().ProcessAddFriend(CurrentUserName, TargetUserName);
            if(dbstat)
            {
                // 2表示已经成为了好友
                ret = 2;
                memcpy(fPDU->ParaData + 48, (char*)&ret, sizeof(int));
                memcpy(sPDU->ParaData + 48, (char*)&ret, sizeof(int));
                ZhishangTcpServer::GetInstance().ForwardPDU(fPDU, TargetUserName);
            }
            else
            {
                ret = -1;
                memcpy(fPDU->ParaData + 48, (char*)&ret, sizeof(int));
                memcpy(sPDU->ParaData + 48, (char*)&ret, sizeof(int));
                ZhishangTcpServer::GetInstance().ForwardPDU(fPDU, TargetUserName);
            }
        }
        return sPDU;
    }
}

PDU *MsgHandler::ProcessRefreshFriendListRequest(const PDU *rPDU)
{
    if(rPDU == NULL)
        return NULL;
    else
    {
        qDebug() << "MSG_TYPE_REFRESH_FRIEND_LIST_RESPOND";
        // 获取要查询的用户名
        char UserName[24] = { 0 };
        memcpy(UserName, rPDU->ParaData, 24);
        // 执行数据库查询
        bool status;
        QVector<int> Online;
        QStringList FriendList = DatabaseOperate::GetInstance().ProcessFriendList(UserName, Online, status);
        // 构建传回的PDU
        int size = FriendList.size();
        PDU *sPDU = CreatePDU(size * (24 + sizeof(int)));
        sPDU->MsgType = MSG_TYPE_REFRESH_FRIEND_LIST_RESPOND;
        memcpy(sPDU->ParaData, &status, sizeof(int));
        // 输出日志并将好友列表写入消息中
        for(int i = 0; i < size; i++)
        {
            qDebug() << "FriendList.at(" << i << ").toStdString().size(): " << FriendList.at(i).toStdString().size();
            qDebug() << "FriendList.at(" << i << ")" << FriendList.at(i);
            qDebug() << "Online.[" << i <<"]" << Online[i];
            memcpy(sPDU->Msg + i * (24 + sizeof(int)), FriendList.at(i).toStdString().c_str(), 24);
            memcpy(sPDU->Msg + i * (24 + sizeof(int)) + 24, &Online[i], sizeof(int));
        }
        return sPDU;
    }
}

PDU *MsgHandler::ProcessDeleteFriendRequest(const PDU *rPDU)
{
    if(rPDU == NULL)
        return NULL;
    char CurrentUserName[24] = { 0 }, TargetUserName[24] = { 0 };
    memcpy(CurrentUserName, rPDU->ParaData, 24);
    memcpy(TargetUserName, rPDU->ParaData + 24, 24);

    // 创建回传给客户端的状态
    // 参数，0失败 1成功 2被删除 3已经不是好友
    int param;
    PDU *sPDU = CreatePDU(0);
    sPDU->MsgType = MSG_TYPE_DELETE_FRIEND_RESPOND;
    memcpy(sPDU->ParaData, TargetUserName, 24);

    // 先查询好友状态，-1错误，-2已经是好友，1用户在线，2用户离线
    int ret = DatabaseOperate::GetInstance().ProcessFriendSearch(CurrentUserName, TargetUserName);

    switch(ret)
    {
        case -1:
            param = 0;
            memcpy(sPDU->ParaData + 24, (char*)&param, sizeof(int));
            break;
        case -2:
            // 获取执行结果，是好友才能开始删除
            ret = DatabaseOperate::GetInstance().ProcessDeleteFriend(CurrentUserName, TargetUserName);
            param = ret;
            memcpy(sPDU->ParaData + 24, (char*)&param, sizeof(int));
            // 成功了再通知对方，当前用户告知结果，所以可以先获取参数
            if(ret)
            {
                // 获取对方在线状态
                int status = DatabaseOperate::GetInstance().ProcessSearchUser(TargetUserName);
                if(status == 1)
                {
                    // 创建转发PDU，如果对方在线，那就转发，状态2表示被删除
                    PDU *fPDU = CreatePDU(0);
                    fPDU->MsgType = MSG_TYPE_DELETE_FRIEND_RESPOND;
                    memcpy(fPDU->ParaData, CurrentUserName, 24);
                    param = 2;
                    memcpy(fPDU->ParaData + 24, (char*)&param, sizeof(int));
                    ZhishangTcpServer::GetInstance().ForwardPDU(fPDU, TargetUserName);
                }
            }
            break;
        default:
            param = 3;
            memcpy(sPDU->ParaData + 24, (char*)&param, sizeof(int));
            break;
    }
        return sPDU;
}

PDU *MsgHandler::ProcessChat(PDU *rPDU)
{
    if(rPDU == NULL)
        return NULL;
    // 读取目标用户
    char TargetUserName[24];
    memcpy(TargetUserName, rPDU->ParaData + 24, 24);
    // 调用转发函数
    ZhishangTcpServer::GetInstance().ForwardPDU(rPDU, TargetUserName);
    return nullptr;
}

PDU *MsgHandler::ProcessNewFolderRequest(const PDU *rPDU, const QString &CurrentUserFolderPath)
{
    if(rPDU == NULL)
        return NULL;

    qDebug() << "MSG_TYPE_NEW_FOLDER_RESQUEST";

    // 读取目标文件夹
    char tmp[64] = { 0 };
    memcpy(tmp, rPDU->ParaData, 64);
    char *Path = new char[rPDU->MsgLen]();
    memcpy(Path, rPDU->Msg, rPDU->MsgLen);
    QString FolderName = QString::fromUtf8(tmp);
    qDebug() << "FolderName = " << FolderName;
    bool ret = true;

    // 检查文件夹名是否包含非法字符
    QString IllegalCharacters = "\\/:?\"'<>|";
    for (int i = 0; i < FolderName.length(); i++)
    {
        if (IllegalCharacters.contains(FolderName.at(i)))
        {
            qDebug() << "文件夹名称包含非法字符";
            ret = false;
            break;
        }
    }

    // 创建文件夹
    if(ret)
    {
        QDir Folder;
        QString FolderPath;
        if(Path == NULL || *Path == 0)
            FolderPath = QString("%1/%2").arg(CurrentUserFolderPath, FolderName);
        else
            FolderPath = QString("%1/%2/%3").arg(CurrentUserFolderPath, Path, FolderName);
        qDebug() << "FolderPath = " << FolderPath;
        ret = Folder.mkdir(FolderPath);
        qDebug() << "Folder.mkdir(FolderPath) = " << ret;
    }

    PDU *sPDU = CreatePDU(64);
    sPDU->MsgType = MSG_TYPE_NEW_FOLDER_RESPOND;
    memcpy(sPDU->Msg, tmp, 64);
    memcpy(sPDU->ParaData, (char*)&ret, sizeof(bool));
    // 释放Path
    delete[] Path;
    return sPDU;
}

// 定义比较函数
bool cmp(const QFileInfo &a, const QFileInfo &b)
{
    // 文件夹优先
    if(a.isDir() != b.isDir())
        return a.isDir() > b.isDir();
    // 文件名a-z升序
    return a.fileName().toStdString() < b.fileName().toStdString();
}

PDU *MsgHandler::ProcessRefreshFolderRequest(const PDU *rPDU, const QString &CurrentUserFolderPath)
{
    if(rPDU == NULL)
        return NULL;
    qDebug() << "MSG_TYPE_REFRESH_FOLDER_LIST_RESQUEST";
    // 获取接收到的路径
    char *Path = new char[rPDU->MsgLen]();
    memcpy(Path, rPDU->Msg, rPDU->MsgLen);
    // 组合形成需要获取的路径
    QString RequestedPath;
    if(Path == NULL || *Path == 0)
        RequestedPath = CurrentUserFolderPath;
    else
        RequestedPath = QString("%1/%2").arg(CurrentUserFolderPath, Path);

    qDebug() << "RequestedPath = " << RequestedPath;
    // 设置路径
    QDir Folder(RequestedPath);
    // 取路径下的所有文件

    qDebug() << "Folder absolute path = " << Folder.absolutePath();

    // QDir::NoDotAndDotDot为过滤器，过滤掉了.和..
    Folder.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    QFileInfoList info = Folder.entryInfoList();
    // 排序
    std::sort(info.begin(), info.end(), cmp);
    // 获取项目数量
    int ItemCount = info.size();

    qDebug() << "Folder items count = " << ItemCount;

    // 根据项目数量创建PDU
    PDU *sPDU = CreatePDU(ItemCount * sizeof(FI));
    // 注意添加协议类型
    sPDU->MsgType = MSG_TYPE_REFRESH_FILES_LIST_RESPOND;
    FI *items = (FI*)sPDU->Msg;
    for(int i = 0; i < ItemCount; i++)
    {
        // 获取文件名
        QString name = info[i].fileName();
        qDebug() << "info[" << i << "].fileName() = " << info[i].fileName();

        // 创建新的文件信息
        FI *fi = new FI;
        // 写入文件名
        memcpy(fi->FileName, name.toStdString().c_str(), 64);
        // 确保字符串以 \0 结尾
        fi->FileName[sizeof(fi->FileName) - 1] = '\0';
        // 获取文件类型
        if(info[i].isDir())
            fi->FileType = FILE_TYPE_FOLDER;
        else if(info[i].isFile())
        {
            QString suffix = info[i].suffix().toLower();
            fi->FileType = GetFileType(suffix);
        }
        else
        {
            fi->FileType = FILE_TYPE_OTHER;
        }
        // 将结果写入PDU
        memcpy(items + i, fi, sizeof(FI));
        // 删除创建的文件信息
        delete fi;
    }
    // 释放保存的Path
    delete[] Path;
    return sPDU;
}

PDU *MsgHandler::ProcessDeleteFileRequest(const PDU *rPDU, const QString &CurrentUserFolderPath)
{
    if(rPDU == NULL)
        return NULL;
    qDebug() << "MSG_TYPE_DELETE_FILE_RESQUEST";
    char *PathRecv = new char[rPDU->MsgLen]();
    char FileNameRecv[64] = { 0 };
    memcpy(PathRecv, rPDU->Msg, rPDU->MsgLen);
    memcpy(FileNameRecv, rPDU->ParaData, 64);

    QString TargetPath;
    if(*PathRecv == 0)
        TargetPath = QString("%1/%2").arg(CurrentUserFolderPath, FileNameRecv);
    else
        TargetPath = QString("%1/%2/%3").arg(CurrentUserFolderPath, PathRecv, FileNameRecv);

    qDebug() << "CurrentUserFolderPath " << CurrentUserFolderPath;
    qDebug() << "TargetPath" << TargetPath;
    qDebug() << "FileNameRecv" << FileNameRecv;

    bool ret;
    QFileInfo info(TargetPath);
    if(info.isFile())
    {
        ret = QFile::remove(TargetPath);
        qDebug() << "删除文件" << FileNameRecv << " " << ret;
    }

    PDU *sPDU = CreatePDU(64);
    sPDU->MsgType = MSG_TYPE_DELETE_FILE_RESPOND;
    memcpy(sPDU->Msg, FileNameRecv, 64);
    memcpy(sPDU->ParaData, &ret, sizeof(bool));
    delete[] PathRecv;
    return sPDU;
}

PDU *MsgHandler::ProcessRenameRequest(const PDU *rPDU, const QString &CurrentUserFolderPath)
{
    if(rPDU == NULL)
        return NULL;
    qDebug() << "MSG_TYPE_RENAME_REQUEST";
    // 获取三个长度
    int PathLen, NameBeforeLen, NameAfterLen;
    memcpy(&PathLen, rPDU->ParaData, sizeof(int));
    memcpy(&NameBeforeLen, rPDU->ParaData + sizeof(int), sizeof(int));
    memcpy(&NameAfterLen, rPDU->ParaData + sizeof(int) + sizeof(int), sizeof(int));
    // 读取 当前路径 原文件名 目标文件名
    char *CurPath = new char[PathLen]();
    char *NameBefore = new char[NameBeforeLen]();
    char *NameAfter = new char[NameAfterLen]();

    // 别忘了拷贝必要的参数
    memcpy(CurPath, rPDU->Msg, PathLen);
    memcpy(NameBefore, rPDU->Msg + PathLen, NameBeforeLen);
    memcpy(NameAfter, rPDU->Msg + PathLen + NameBeforeLen, NameAfterLen);

    qDebug() << "要重命名文件所在路径：" << CurPath;
    qDebug() << "原始文件名：" << NameBefore;
    qDebug() << "新文件名：" << NameAfter;

    QString bef;
    QString aft;
    // 如果在根目录，少拼一个参数
    if(*CurPath == 0)
    {
        bef = QString("%1/%2").arg(CurrentUserFolderPath, NameBefore);
        aft = QString("%1/%2").arg(CurrentUserFolderPath, NameAfter);
    }
    else
    {
        bef = QString("%1/%2/%3").arg(CurrentUserFolderPath, CurPath, NameBefore);
        aft = QString("%1/%2/%3").arg(CurrentUserFolderPath, CurPath, NameAfter);
    }

    QDir dir;
    bool ret = dir.rename(bef, aft);

    qDebug() << "重命名状态" << ret;

    // 创建返回的PDU
    PDU *sPDU = CreatePDU(NameBeforeLen);
    sPDU->MsgType = MSG_TYPE_RENAME_RESPOND;
    memcpy(sPDU->ParaData, &ret, sizeof(bool));
    memcpy(sPDU->Msg, NameBefore, NameBeforeLen);

    // 用完删除
    delete[] CurPath;
    delete[] NameBefore;
    delete[] NameAfter;

    return sPDU;
}

PDU *MsgHandler::ProcessMoveRequest(const PDU *rPDU, const QString &CurrentUserFolderPath)
{
    if(rPDU == NULL)
        return NULL;
    qDebug() << "MSG_TYPE_MOVE_REQUEST";

    // 先获取两个长度
    int PathBeforeLen, PathAfterLen;
    memcpy(&PathBeforeLen, rPDU->ParaData, sizeof(int));
    memcpy(&PathAfterLen, rPDU->ParaData + sizeof(int), sizeof(int));

    // 读取两个路径
    char *PathBefore = new char[PathBeforeLen]();
    char *PathAfter = new char[PathAfterLen]();
    memcpy(PathBefore, rPDU->Msg, PathBeforeLen);
    memcpy(PathAfter, rPDU->Msg + PathBeforeLen, PathAfterLen);

    // 拼接路径
    QString bef = QString("%1/%2").arg(CurrentUserFolderPath, PathBefore);
    QString aft = QString("%1/%2").arg(CurrentUserFolderPath, PathAfter);

    qDebug() << "原始路径：" << bef;
    qDebug() << "新路径：" << aft;

    bool ret = QFile::rename(bef, aft);

    qDebug() << "移动结果：" << ret;

    // 创建返回数据包
    PDU *sPDU = CreatePDU(0);
    sPDU->MsgType = MSG_TYPE_MOVE_RESPOND;
    memcpy(sPDU->ParaData, &ret, sizeof(bool));

    // 用完记得删除
    delete[] PathBefore;
    delete[] PathAfter;

    return sPDU;
}

PDU *MsgHandler::ProcessUploadRequest(const PDU *rPDU, const QString &CurrentUserFolderPath)
{
    if(rPDU == NULL)
    {
        qDebug() << "啊？";
        return NULL;
    }
    qDebug() << "MSG_TYPE_UPLOAD_FILE_REQUEST";

    qDebug() << "上传文件状态：" << IsUploading;

    // 0 成功 1 已有文件在上传 -1 打开文件失败
    PDU *sPDU = CreatePDU(rPDU->MsgLen);
    sPDU->MsgType = MSG_TYPE_UPLOAD_FILE_RESPOND;
    memcpy(sPDU->Msg, rPDU->Msg, rPDU->MsgLen);
    int ret;
    if(IsUploading)
    {
        ret = 1;
        qDebug() << "已有文件正在上传";
        memcpy(sPDU->ParaData, &ret, sizeof(int));
    }
    else
    {
        qDebug() << "设置上传状态";
        // 设置上传状态
        IsUploading = true;
        // 获取上传路径，需要接收的文件大小
        int NameLen, PathLen;
        memcpy(&SizeToReceive, rPDU->ParaData, sizeof(qint64));
        memcpy(&PathLen, rPDU->ParaData + sizeof(qint64), sizeof(int));
        memcpy(&NameLen, rPDU->ParaData + sizeof(qint64) + sizeof(int), sizeof(int));
        char *PathRecv = new char[PathLen]();
        char *NameRecv = new char[NameLen]();
        memcpy(PathRecv, rPDU->Msg, PathLen);
        memcpy(NameRecv, rPDU->Msg + PathLen, NameLen);

        qDebug() << "接收到的上传的文件大小：" << SizeToReceive;
        qDebug() << "接收到的上传的文件路径：" << PathRecv;
        qDebug() << "接收到的上传的文件名：" << NameRecv;

        QString TargetPath;
        if(*PathRecv == 0)
            TargetPath = QString("%1/%2").arg(CurrentUserFolderPath, NameRecv);
        else
            TargetPath = QString("%1/%2/%3").arg(CurrentUserFolderPath, PathRecv, NameRecv);

        qDebug() << "实际上传的文件路径：" << TargetPath;

        FileToReceive.setFileName(TargetPath);
        bool status = FileToReceive.open(QIODevice::WriteOnly);
        qDebug() << "打开文件状态：" << status;
        if(!status)
            ret = -1;
        else
            ret = 0;

        memcpy(sPDU->ParaData, &ret, sizeof(int));

        delete[] PathRecv;
        delete[] NameRecv;
    }
    return sPDU;
}

PDU *MsgHandler::ProcessUploadFileSendDataRequest(const PDU *rPDU)
{
    if(rPDU == NULL)
        return NULL;

    qDebug() << "MSG_TYPE_UPLOAD_FILE_DATA";

    // 输出目标文件路径
    qDebug() << "FileToReceive.fileName() = " << FileToReceive.fileName();
    // 输出目标文件是否存在
    qDebug() << "FileToReceive.exists() = " << FileToReceive.exists();
    // 输出目标文件是否打开
    qDebug() << "FileToReceive.isOpen() = " << FileToReceive.isOpen();

    // 如果目标文件不存在或未打开，那么返回错误
    if(!FileToReceive.exists() || !FileToReceive.isOpen())
    {
        qDebug() << "目标文件不存在或未打开";
        IsUploading = false;
        PDU *sPDU = CreatePDU(0);
        sPDU->MsgType = MSG_TYPE_UPLOAD_FILE_ERROR;
        return sPDU;
    }
    
    // 获取接收到的数据长度
    int WriteCount = FileToReceive.write(rPDU->Msg, rPDU->MsgLen);
    // 更新上传的文件大小
    SizeReceived += WriteCount;

    qDebug() << "接收到的数据长度 = " << rPDU->MsgLen;
    qDebug() << "写入的文件大小 = " << WriteCount;
    qDebug() << "接收到的文件大小 = " << SizeReceived;
    qDebug() << "剩余文件大小 = " << SizeToReceive - SizeReceived;

    // 出错了回报错误
    if(WriteCount == -1 || rPDU->MsgLen != WriteCount || (rPDU->MsgLen != 4096 && SizeReceived != SizeToReceive))
    {
        qDebug() << "写入文件错误";
        // 传输错误就关闭文件并删除
        // 如果目标文件已经打开，那么关闭
        if(FileToReceive.isOpen())
            FileToReceive.close();
        // 如果目标文件存在，那么删除
        if(FileToReceive.exists())
            FileToReceive.remove();
        IsUploading = false;
        PDU *sPDU = CreatePDU(0);
        sPDU->MsgType = MSG_TYPE_UPLOAD_FILE_ERROR;
        return sPDU;
    }

    // 写完了关闭文件，发送成功结果给客户端
    if(SizeReceived == SizeToReceive)
    {
        // 如果目标文件已经打开，那么关闭
        if(FileToReceive.isOpen())
            FileToReceive.close();
        IsUploading = false;
        PDU *sPDU1 = CreatePDU(0);
        sPDU1->MsgType = MSG_TYPE_UPLOAD_FILE_SUCCESS;
        return sPDU1;
    }

    return NULL;
}

PDU *MsgHandler::ProcessUploadFileError(const PDU *rPDU)
{
    if(rPDU == NULL)
        return NULL;

    qDebug() << "MSG_TYPE_UPLOAD_FILE_ERROR";

    // 传输错误就关闭文件并删除
    // 如果目标文件已经打开，那么关闭
    if(FileToReceive.isOpen())
        FileToReceive.close();
    // 如果目标文件存在，那么删除
    if(FileToReceive.exists())
        FileToReceive.remove();

    // 状态重置
    IsUploading = false;

    return NULL;
}

PDU *MsgHandler::ProcessDownloadRequest(const PDU *rPDU, const QString &CurrentUserFolderPath)
{
    if(rPDU == NULL)
        return NULL;
    qDebug() << "MSG_TYPE_DOWNLOAD_FILE_REQUEST";

    PDU *sPDU = CreatePDU(0);
    sPDU->MsgType = MSG_TYPE_DOWNLOAD_FILE_RESPOND;

    // 返回是否有文件在下载，有文件在下载就没必要获取信息了
    // 三个状态 -2打开失败，-1已有文件在下载，0可以下载
    int ret = 0;
    if(IsDownloading)
    {
        qDebug() << "已有文件正在下载";
        ret = -1;
        memcpy(sPDU->ParaData, &ret, sizeof(int));
        return sPDU;
    }

    // 获取需要下载的文件路径
    char *PathRecv = new char[rPDU->MsgLen]();
    memcpy(PathRecv, rPDU->Msg, rPDU->MsgLen);
    qDebug() << "接收到的文件路径 = " << PathRecv;

    // 拼接实际文件路径
    QString TargetFilePath = QString("%1/%2").arg(CurrentUserFolderPath, PathRecv);

    // 设置文件路径
    FileToSend.setFileName(TargetFilePath);

    // 获取文件打开状态
    if(!FileToSend.open(QIODevice::ReadOnly))
    {
        qDebug() << "文件打开失败";
        ret = -2;
        memcpy(sPDU->ParaData, &ret, sizeof(int));
        return sPDU;
    }

    qint64 FileSize = FileToSend.size();

    // 输出文件的基本信息
    qDebug() << "下载文件大小：" << FileSize;
    qDebug() << "下载文件路径：" << TargetFilePath;

    // 将文件大小信息回传给客户端
    memcpy(sPDU->ParaData, &ret, sizeof(int));
    memcpy(sPDU->ParaData + sizeof(int), &FileSize, sizeof(qint64));

    return sPDU;
}

PDU* MsgHandler::ProcessDonloadReady(ZhishangTcpSocket *CurrentSocket)
{
    PDU *sPDU = CreatePDU(4096);
    char *Data = new char[4096]();
    sPDU->MsgType = MSG_TYPE_DOWNLOAD_FILE_DATA;
    int ReadCount;

    while(true)
    {
        ReadCount = FileToSend.read(Data, 4096);

        // 文件读取完成
        if(ReadCount == 0)
        {
            FileToSend.close();
            IsDownloading = false;
            break;
        }

        // 文件读取失败
        if(ReadCount == -1)
        {
            FileToSend.close();
            IsDownloading = false;
            sPDU->MsgType = MSG_TYPE_DOWNLOAD_FILE_ERROR;
            sPDU->MsgLen = 0;
            sPDU->PDULen = sizeof(PDU);
            return sPDU;
        }

        sPDU->MsgLen = ReadCount;
        sPDU->PDULen = ReadCount + sizeof(PDU);
        memcpy(sPDU->Msg, Data, ReadCount);
        CurrentSocket->write((char*)sPDU, sPDU->PDULen);
    }

    delete sPDU;
    delete[] Data;
    return NULL;
}

PDU *MsgHandler::ProcessShareFileRequest(const PDU *rPDU, const QString &CurrentUserFolderPath)
{
    if(rPDU == NULL)
        return NULL;
    qDebug() << "MSG_TYPE_SHARE_FILE_REQUEST";

    char CurrentUserName[24];
    int PathLen, NameLen, FriendsSize;

    // 获取基本信息
    memcpy(CurrentUserName, rPDU->ParaData, 24);
    memcpy(&PathLen, rPDU->ParaData + 24, sizeof(int));
    memcpy(&NameLen, rPDU->ParaData + 24 + sizeof(int), sizeof(int));
    memcpy(&FriendsSize, rPDU->ParaData + 24 + sizeof(int) + sizeof(int), sizeof(int));

    char *NameRecv = new char[NameLen]();
    char *PathRecv = new char[PathLen]();
    memcpy(PathRecv, rPDU->Msg + FriendsSize, PathLen);
    memcpy(NameRecv, rPDU->Msg + FriendsSize + PathLen, NameLen);

    // 创建转发PDU
    PDU *fPDU = CreatePDU(NameLen);
    fPDU->MsgType = MSG_TYPE_SHARE_FILE_REQUEST;
    memcpy(fPDU->ParaData, CurrentUserName, 24);
    memcpy(fPDU->Msg, rPDU->Msg + FriendsSize + PathLen, NameLen);

    QString SourceFilePath;
    if(PathLen == 1)
        SourceFilePath = QString("%1/%2").arg(CurrentUserFolderPath, NameRecv);
    else
        SourceFilePath = QString("%1/%2/%3").arg(CurrentUserFolderPath, PathRecv, NameRecv);

    qDebug() << "来自 " << CurrentUserName << " 的分享";
    qDebug() << "路径名 " << PathRecv;
    qDebug() << "文件名 " << NameRecv;
    qDebug() << "完整路径名 " << SourceFilePath;

    int FriendsLen = (FriendsSize - 1) / 24, cnt = 0;
    qDebug() << "FriendsLen = " << FriendsLen;

    for(int i = 0; i < FriendsLen; i++)
    {
        char FriendName[24];
        memcpy(FriendName, rPDU->Msg + i * 24, 24);
        qDebug() << "FriendName = " << FriendName;
        if(!ZhishangTcpServer::GetInstance().ForwardPDU(fPDU, FriendName, SourceFilePath))
            cnt++;
    }

    qDebug() << "cnt = " << cnt;

    PDU *sPDU = CreatePDU(0);
    sPDU->MsgType = MSG_TYPE_SHARE_FILE_RESPOND;
    memcpy(sPDU->ParaData, &cnt, sizeof(int));

    delete[] NameRecv;
    delete[] PathRecv;

    return sPDU;
}

PDU *MsgHandler::ProcessShareFileRespond(PDU *rPDU, const QString &CurrentUserFolderPath, QString &SourceFilePath)
{
    if(rPDU == NULL || SourceFilePath.isEmpty())
        return NULL;
    qDebug() << "MSG_TYPE_SHARE_FILE_RESPOND";

    int status;
    memcpy(&status, rPDU->ParaData, sizeof(int));

    if(status == QMessageBox::No)
    {
        SourceFilePath.clear();
        return NULL;
    }

    // 如果不存在的话则创建接收文件文件夹
    QDir dir(CurrentUserFolderPath + "/Pack From Shared");
    if (!dir.exists())
        dir.mkpath(".");

    int idx = SourceFilePath.lastIndexOf('/');
    if(idx == -1)
        idx = 1;
    QString SourceFileName = SourceFilePath.right(SourceFilePath.size() - idx - 1);
    QString TargetPath = CurrentUserFolderPath + "/Pack From Shared/" + SourceFileName;

    qDebug() << "保存的目标文件 " << TargetPath;

    QFileInfo info(SourceFilePath);
    int ret;
    if(info.isFile())
    {
        qDebug() << "info.isFile() " << info.isFile();
        if(QFile::copy(SourceFilePath, TargetPath))
            ret = -1;
        else
            ret = -2;
    }
    else if(info.isDir())
    {
        qDebug() << "info.isDir() " << info.isDir();
        if(ProcessCopyFolder(SourceFilePath, TargetPath))
            ret = -1;
        else
            ret = -2;
    }

    PDU *sPDU = CreatePDU(0);
    sPDU->MsgType = MSG_TYPE_SHARE_FILE_RESPOND;
    memcpy(sPDU->ParaData, &ret, sizeof(int));
    return sPDU;
}

bool MsgHandler::ProcessCopyFolder(const QString &SourceFilePath, const QString TargetFilePath)
{
    QDir dir;
    dir.mkdir(TargetFilePath);
    dir.setPath(SourceFilePath);
    QFileInfoList InfoList = dir.entryInfoList();
    bool ret = true;
    QString src, dst;
    for(int i = 0; i < InfoList.size(); i++)
    {
        if(InfoList[i].isDir())
        {
            src = SourceFilePath + '/' + InfoList[i].fileName();
            dst = TargetFilePath + '/' + InfoList[i].fileName();
            qDebug() << "src = " << src;
            qDebug() << "dst = " << dst;
            if(!QFile::copy(src, dst))
                ret = false;
        }
        else if(InfoList[i].isDir())
        {
            // 跳过.和..文件夹
            if(InfoList[i].fileName() == "." || InfoList[i].fileName() == "..")
                continue;
            // 归递复制文件夹
            if(!ProcessCopyFolder(src, dst))
                ret = false;
        }
    }
    return ret;
}

PDU *MsgHandler::ProcessDeleteFolderRequest(const PDU *rPDU, const QString &CurrentUserFolderPath)
{
    if(rPDU == NULL)
        return NULL;
    qDebug() << "MSG_TYPE_DELETE_FOLDER_RESQUEST";
    char *PathRecv = new char[rPDU->MsgLen]();
    memcpy(PathRecv, rPDU->Msg, rPDU->MsgLen);
    QString TargetPath = QString("%1/%2").arg(CurrentUserFolderPath, PathRecv);
    qDebug() << "CurrentUserFolderPath " << CurrentUserFolderPath;
    qDebug() << "PathRecv " << PathRecv;
    qDebug() << "要删除的文件夹路径 " << TargetPath;

    bool ret = false;
    QFileInfo info(TargetPath);
    if(info.isDir())
    {
        QDir dir;
        dir.setPath(TargetPath);
        qDebug() << "删除的绝对路径" << dir.absolutePath();
        ret = dir.removeRecursively();
    }

    PDU *sPDU = CreatePDU(rPDU->MsgLen);
    sPDU->MsgType = MSG_TYPE_DELETE_FOLDER_RESPOND;
    memcpy(sPDU->Msg, PathRecv, rPDU->MsgLen);
    memcpy(sPDU->ParaData, &ret, sizeof(bool));

    return sPDU;
}
