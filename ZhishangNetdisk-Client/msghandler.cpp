#include "msghandler.h"
#include "mainwindow.h"
#include "loginpage.h"
#include "friends.h"
#include "files.h"

#include <QMessageBox>
#include <QDebug>
// memcpy在string.h中
#include <string>

MsgHandler::MsgHandler() {}

void MsgHandler::ProcessRegisterRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;
    bool respond;
    memcpy((char*)&respond, rPDU->ParaData, sizeof(bool));
    if(respond)
    {
        qDebug() << "注册成功";
        // 处理消息函数从LoginPage中移了出来，所以this需要更改为指向LoginPage实例的指针
        QMessageBox::information((QWidget*)&LoginPage::GetInstance(), "注册结果", "注册成功");
    }
    else
    {
        qDebug() << "注册失败，请检查用户名或密码";
        QMessageBox::critical((QWidget*)&LoginPage::GetInstance(), "注册结果", "注册失败，请检查用户名或密码");
    }
}

void MsgHandler::ProcessLoginRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;
    bool respond;
    memcpy((char*)&respond, rPDU->ParaData, sizeof(bool));
    if(respond)
    {
        qDebug() << "登录成功";
        QMessageBox::information((QWidget*)&LoginPage::GetInstance(), "登录结果", "登录成功");
        // 显示主界面
        MainWindow::GetInstance().show();
        // 隐藏登录窗口
        // 从LoginPage中移出来后不能直接调用hide，记得加上作用域
        LoginPage::GetInstance().hide();
        MainWindow::GetInstance().SetTitle(LoginPage::GetInstance().GetCurrentUser());
    }
    else
    {
        qDebug() << "登录失败，请检查用户名或密码";
        QMessageBox::critical((QWidget*)&LoginPage::GetInstance(), "登录结果", "登录失败，请检查用户名或密码");
    }
}

void MsgHandler::ProcessSearchUserRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;
    // 服务器传回的respond为int类型，从收到的PDU中读取对应的信息
    int respond;
    char UserName[25];
    memcpy(UserName, rPDU->ParaData, sizeof(UserName));
    memcpy((char*)&respond, rPDU->ParaData + 24, sizeof(int));
    if(respond == -1)
    {
        qDebug() << "用户不存在";
        // 注意GetFriendsPage函数返回的值本来就是指针，就别加取地址了
        QMessageBox::information((QWidget*)MainWindow::GetInstance().GetFriendsPage(), "搜索结果", "用户 " + QString::fromUtf8(UserName) + " 不存在");
    }
    else if(respond == 1)
    {
        qDebug() << "用户在线";
        int status = QMessageBox::information((QWidget*)MainWindow::GetInstance().GetFriendsPage(), "搜索结果", QString("用户 %1 在线，是否要添加为好友？").arg(UserName), "是", "否");
        if(!status)
        {
            PDU* sPDU = CreatePDU(0);
            sPDU->MsgType = MSG_TYPE_ADD_FRIEND_REQUEST;
            memcpy(sPDU->ParaData, LoginPage::GetInstance().GetCurrentUser().toStdString().c_str(), 24);
            memcpy(sPDU->ParaData + 24, UserName, 24);
            LoginPage::GetInstance().SendPDU(sPDU);
            qDebug() << "发送好友请求成功";
        }
    }
    else
    {
        qDebug() << "用户离线";
        QMessageBox::critical((QWidget*)MainWindow::GetInstance().GetFriendsPage(), "搜索结果", "用户 " + QString::fromUtf8(UserName) + " 离线");
    }
}

void MsgHandler::ProcessShowOnlieneUserRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;
    // 接收服务器传回的在线用户列表
    QStringList UserList;
    int size = rPDU->MsgLen / 24;
    for(int i = 0; i < size; i++)
    {
        // 先创建一个std::string对象，从pdu中逐个取出在线用户名，因为用户名固定长度为24
        // 然后使用fromStdSreing将PDU中的在线用户名转换为QString类型对象
        // QString UserName = QString::fromStdString(std::string(rPDU->Msg + i * 24, 24));
        // 上述麻烦的方法可能会导致接收的值存在乱码，搞太花里胡哨容易寄，直接复制更香
        char UserName[25];
        // 不添加当前用户
        memcpy(UserName, rPDU->Msg + i * 24, 24);
        if((QString)UserName == LoginPage::GetInstance().GetCurrentUser())
            continue;
        // 使用trimmed()去除可能存在的多余空格
        // UserList.append(UserName.trimmed());
        // 可以直接append字符数组的，不需要其他处理
        UserList.append(UserName);
    }
    // 注意不要直接调用函数，记得通过实例
    MainWindow::GetInstance().GetFriendsPage()->SetOnlineUserList(UserList, size);
}

void MsgHandler::ProcessAddFriendRequest(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;
    char UserName[25] = { 0 };
    memcpy(UserName, rPDU->ParaData, 24);
    int ret = QMessageBox::question((QWidget*)MainWindow::GetInstance().GetFriendsPage(), "添加好友？", QString("您是否需要添加 %1 为好友？").arg(UserName));
    PDU *sPDU = CreatePDU(0);
    sPDU->MsgType = MSG_TYPE_ADD_FRIEND_RESPOND;
    memcpy(sPDU->ParaData, rPDU->ParaData, 48);
    // 第三个参数带着同意或者拒绝
    memcpy(sPDU->ParaData + 48, (char*)&ret, sizeof(int));
    if(ret == QMessageBox::Yes)
    {
        qDebug() << QString("同意添加 %1 为好友").arg(UserName);
    }
    else
    {
        qDebug() << QString("拒绝添加 %1 为好友").arg(UserName);
    }
    LoginPage::GetInstance().SendPDU(sPDU);
}

void MsgHandler::ProcessAddFriendRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;
    int status;
    memcpy(&status, rPDU->ParaData, sizeof(int));
    switch(status)
    {
        case -2:
            QMessageBox::critical((QWidget*)MainWindow::GetInstance().GetFriendsPage(), "添加好友失败", "你们已经是好友了！");
            break;
        case 0:
            QMessageBox::information((QWidget*)MainWindow::GetInstance().GetFriendsPage(), "添加好友失败", "用户离线，可以尝试发送电波通知对方上线！");
            break;
        case 1:
            QMessageBox::information((QWidget*)MainWindow::GetInstance().GetFriendsPage(), "添加好友中", "用户在线，请求已发送，请静候佳音！");
            break;
        default:
            QMessageBox::critical((QWidget*)MainWindow::GetInstance().GetFriendsPage(), "添加好友错误", "添加好友错误！");
            break;
    }
}

void MsgHandler::ProcessAddFriendResult(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;
    char UserName[25] = { 0 };
    memcpy(UserName, rPDU->ParaData + 24, 24);
    int result;
    memcpy(&result, rPDU->ParaData + 48, sizeof(int));
    switch(result)
    {
        case QMessageBox::Yes:
            // GetFriendPage返回的就是指针，不需要取地址
            QMessageBox::information((QWidget*)MainWindow::GetInstance().GetFriendsPage(), "添加好友中", QString("用户 %1 同意了您的好友请求！").arg(UserName));
            qDebug() << QString("用户 %1 同意了好友请求！").arg(UserName);
            break;
        case QMessageBox::No:
            QMessageBox::information((QWidget*)MainWindow::GetInstance().GetFriendsPage(), "添加好友失败", QString("用户 %1 拒绝了您的好友请求！").arg(UserName));
            qDebug() << QString("用户 %1 拒绝了好友请求！").arg(UserName);
            break;
        case 2:
            QMessageBox::information((QWidget*)MainWindow::GetInstance().GetFriendsPage(), "添加好友成功", QString("用户 %1 现在是您的好友！").arg(UserName));
            qDebug() << QString("和 %1 成为了好友！").arg(UserName);
            // 会导致好友列表变动，需要发送请求刷新好友列表
            MainWindow::GetInstance().GetFriendsPage()->SendRefreshFriendListRequest();
            break;
        default:
            QMessageBox::critical((QWidget*)MainWindow::GetInstance().GetFriendsPage(), "添加好友错误", QString("添加 %1 错误！").arg(UserName));
            qDebug() << QString("添加 %1 错误！").arg(UserName);
            break;
    }
}

void MsgHandler::ProcessRefreshFriendListRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;
    bool status;
    memcpy(&status, rPDU->ParaData, sizeof(bool));
    if(!status)
    {
        QMessageBox::critical((QWidget*)MainWindow::GetInstance().GetFriendsPage(), "错误", "刷新好友列表失败！");
        return;
    }
    int len = rPDU->MsgLen / (24 + sizeof(int));
    // 读取回传数据包中的好友列表和是否在线信息
    QStringList FriendList;
    QVector<int> Online;
    int cnt = 0;
    for(int i = 0; i < len; i++)
    {
        char FriendName[25];
        int online;
        memcpy(FriendName, rPDU->Msg + i * (24 + sizeof(int)), 24);
        memcpy(&online, rPDU->Msg + i * (24 + sizeof(int)) + 24, sizeof(int));
        FriendList.append(FriendName);
        Online.push_back(online);
        if(online)
            cnt++;
        // 输出日志
        qDebug() << "FriendList.at(" << i << ").toStdString().size(): " << FriendList.at(i).toStdString().size();
        qDebug() << "FriendList.at(" << i << ")" << FriendList.at(i);
        qDebug() << "Online.[" << i <<"]" << Online[i];
    }
    MainWindow::GetInstance().GetFriendsPage()->SetFriendList(FriendList, Online, len, cnt);
}

void MsgHandler::ProcessDeleteFriendRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;
    // 读取数据包内容
    char UserName[25] = { 0 };
    int status;
    memcpy(UserName, rPDU->ParaData, 24);
    memcpy(&status, rPDU->ParaData + 24, sizeof(int));
    Friends *FriendPage = MainWindow::GetInstance().GetFriendsPage();
    // 参数，0失败 1成功 2被删除 3已经不是好友
    switch(status)
    {
        case 0:
            QMessageBox::critical(FriendPage, "失败了", QString("删除好友 %1 失败！").arg(UserName));
            break;
        case 1:
            QMessageBox::information(FriendPage, "成功", QString("删除好友 %1 成功！").arg(UserName));
            // 成功了就刷新好友列表
            MainWindow::GetInstance().GetFriendsPage()->SendRefreshFriendListRequest();
            break;
        case 2:
            QMessageBox::information(FriendPage, "您被好友删除了", QString("很遗憾的通知您，您被好友 %1 删除了！").arg(UserName));
            // 被删了也需要刷新好友列表
            MainWindow::GetInstance().GetFriendsPage()->SendRefreshFriendListRequest();
            break;
        case 3:
            QMessageBox::critical(FriendPage, "失败了", QString("删除好友 %1 失败，您已被好友单删，执行刷新好友列表！").arg(UserName));
            MainWindow::GetInstance().GetFriendsPage()->SendRefreshFriendListRequest();
            break;
        default:
            break;
        }
}

void MsgHandler::ProcessChat(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;
    // 读取来源用户名
    char UserName[25];
    memcpy(UserName, rPDU->ParaData, 24);
    qDebug() << "读取来自" << UserName << "的消息";
    // 读取消息
    char *Msg = new char[rPDU->MsgLen]();
    memcpy(Msg, rPDU->Msg, rPDU->MsgLen);
    // 调用好友界面函数，显示聊天窗口同时设置消息
    MainWindow::GetInstance().GetFriendsPage()->SetMsg(UserName, Msg);
}

void MsgHandler::ProcessNewFolderRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;
    char FolderName[64] = { 0 };
    bool ret;
    memcpy(FolderName, rPDU->Msg, 64);
    memcpy(&ret, rPDU->ParaData, sizeof(bool));

    Files *FilesPage = MainWindow::GetInstance().GetFilesPage();

    if(ret)
    {
        QMessageBox::information(FilesPage, "成功", QString("创建文件夹 %1 成功！").arg(FolderName));
        // 创建成功就刷新当前文件夹
        MainWindow::GetInstance().GetFilesPage()->RefreshFilesList();
    }
    else
        QMessageBox::warning(FilesPage, "失败", QString("创建文件夹 %1 失败！").arg(FolderName));
}

void MsgHandler::ProcessRefreshFilesListRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;
    // 读取文件列表长度
    int len = rPDU->MsgLen / sizeof(FI);
    // 取出文件列表
    QList<FI*> pFileList;
    for(int i = 0; i < len; i++)
    {
        FI *item = new FI;
        memcpy(item, rPDU->Msg + i * sizeof(FI), sizeof(FI));
        pFileList.append(item);
    }

    MainWindow::GetInstance().GetFilesPage()->SetFileList(pFileList);;
}

void MsgHandler::ProcessDeleteFolderRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;

    bool ret;
    memcpy(&ret, rPDU->ParaData, sizeof(bool));
    if(!ret)
    {
        QMessageBox::critical(MainWindow::GetInstance().GetFilesPage(), "删除文件夹", "删除文件夹失败！");
        return;
    }

    int len = rPDU->MsgLen;
    char *path = new char[len]();
    memcpy(path, rPDU->Msg, len);
    QMessageBox::information(MainWindow::GetInstance().GetFilesPage(), "删除文件夹", QString("删除文件夹 %1 成功！").arg(path));
    MainWindow::GetInstance().GetFilesPage()->RefreshFilesList();
    delete[] path;
}

void MsgHandler::ProcessDeleteFileRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;

    bool ret;
    memcpy(&ret, rPDU->ParaData, sizeof(bool));
    if(!ret)
    {
        QMessageBox::critical(MainWindow::GetInstance().GetFilesPage(), "删除文件", "删除文件失败！");
        return;
    }

    char name[64];
    memcpy(name, rPDU->Msg, 64);
    qDebug() << "删除的文件名" << name;
    QMessageBox::information(MainWindow::GetInstance().GetFilesPage(), "删除文件", QString("删除文件 %1 成功！").arg(name));
    MainWindow::GetInstance().GetFilesPage()->RefreshFilesList();
}

void MsgHandler::ProcessRenameRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;

    // 获取状态
    bool status;
    memcpy(&status, rPDU->ParaData, sizeof(bool));

    // 获取名称
    char *name = new char[rPDU->MsgLen]();
    memcpy(name, rPDU->Msg, rPDU->MsgLen);

    if(!status)
    {
        qDebug() << "重命名 " << name << " 失败";
        QMessageBox::warning(MainWindow::GetInstance().GetFilesPage(), "失败", QString("重命名 %1 失败了！").arg(name));
    }
    else
    {
        qDebug() << "重命名 " << name << " 成功";
        QMessageBox::warning(MainWindow::GetInstance().GetFilesPage(), "失败", QString("重命名 %1 成功！").arg(name));
        // 成功后需要刷新文件列表
        MainWindow::GetInstance().GetFilesPage()->RefreshFilesList();
    }

    // 用完记得释放
    delete[] name;
}

void MsgHandler::ProcessMoveRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;

    // 获取状态
    bool status;
    memcpy(&status, rPDU->ParaData, sizeof(bool));

    QString TargetFileName = MainWindow::GetInstance().GetFilesPage()->GetTargetFileName();

    if(!status)
    {
        qDebug() << "移动 " << TargetFileName << " 失败";
        QMessageBox::warning(MainWindow::GetInstance().GetFilesPage(), "失败", QString("移动 %1 失败了！").arg(TargetFileName));
    }
    else
    {
        qDebug() << "移动 " << TargetFileName << " 成功";
        QMessageBox::warning(MainWindow::GetInstance().GetFilesPage(), "失败", QString("移动 %1 成功！").arg(TargetFileName));
    }

    // 不论成功失败，都需要返回之前的路径
    MainWindow::GetInstance().GetFilesPage()->RefreshFilesListAfterMove();
}

void MsgHandler::ProcessUploadRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;

    int status;
    memcpy(&status, rPDU->ParaData, sizeof(int));

    qDebug() << "ProcessUploadRespond = " << status;

    switch(status)
    {
        case -1:
            QMessageBox::warning(MainWindow::GetInstance().GetFilesPage(), "上传文件失败", "服务端打开文件失败！");
            MainWindow::GetInstance().GetFilesPage()->GetIsUploading() = false;
            break;
        case 1:
            QMessageBox::warning(MainWindow::GetInstance().GetFilesPage(), "上传文件失败", "已有文件正在上传！");
            MainWindow::GetInstance().GetFilesPage()->GetIsUploading() = false;
            break;
        default:
            // 可以上传就调用函数上传，先设置上传状态
            MainWindow::GetInstance().GetFilesPage()->GetIsUploading() = true;
            MainWindow::GetInstance().GetFilesPage()->UploadFile();
            break;
        }
}

void MsgHandler::ProcessUploadSuccess()
{
    qDebug() << "上传文件成功";
    QMessageBox::information(MainWindow::GetInstance().GetFilesPage(), "上传文件成功", "服务器接收完成！");
}

void MsgHandler::ProcessUploadError()
{
    qDebug() << "RecvMsg: UploadError";
    MainWindow::GetInstance().GetFilesPage()->ProcessUploadError();
    QMessageBox::warning(MainWindow::GetInstance().GetFilesPage(), "上传文件失败", "服务端写入错误！");
}

void MsgHandler::ProcessDownloadRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;

    int status;
    memcpy(&status, rPDU->ParaData, sizeof(int));

    qDebug() << "服务端返回状态：" << status;

    switch(status)
    {
        case -2:
            QMessageBox::warning(MainWindow::GetInstance().GetFilesPage(), "下载文件失败", "服务端打开文件失败！");
            MainWindow::GetInstance().GetFilesPage()->ProcessDownloadError();
            break;
        case -1:
            QMessageBox::warning(MainWindow::GetInstance().GetFilesPage(), "下载文件失败", "已有文件正在下载！");
            MainWindow::GetInstance().GetFilesPage()->ProcessDownloadError();
            break;
        default:
        {
            qint64 SizeToReceive;
            memcpy(&SizeToReceive, rPDU->ParaData + sizeof(int), sizeof(qint64));
            qDebug() << "需要接收的文件大小：" << SizeToReceive;
            MainWindow::GetInstance().GetFilesPage()->SetSizeToReceive(SizeToReceive);
            // 设置下载状态
            qDebug() << "之前的下载状态：" << MainWindow::GetInstance().GetFilesPage()->GetIsDownloading();
            MainWindow::GetInstance().GetFilesPage()->SetIsDownloading(true);
            qDebug() << "之后的下载状态：" << MainWindow::GetInstance().GetFilesPage()->GetIsDownloading();

            // 设置下载进度条参数
            MainWindow::GetInstance().GetFilesPage()->GetDownBar()->SetStatus(SizeToReceive, 0, 0);
            MainWindow::GetInstance().GetFilesPage()->GetDownBar()->SetDownloadProgressRange(0, SizeToReceive);
            if(MainWindow::GetInstance().GetFilesPage()->GetDownBar()->isHidden());
                MainWindow::GetInstance().GetFilesPage()->GetDownBar()->show();

            // 创建数据包返回服务器，告诉服务器可以开始下载
            PDU *sPDU = CreatePDU(0);
            sPDU->MsgType = MSG_TYPE_DOWNLOAD_FILE_READY;
            LoginPage::GetInstance().SendPDU(sPDU);
            break;
        }
    }
}

void MsgHandler::ProcessDownloadData(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;

    qDebug() << "接收的数据长度：" << rPDU->MsgLen;
    // 交给文件类去处理
    MainWindow::GetInstance().GetFilesPage()->ProcessDownloadData(rPDU->Msg, rPDU->MsgLen);
}

void MsgHandler::ProcessDownloadError()
{
    QMessageBox::warning(MainWindow::GetInstance().GetFilesPage(), "下载文件失败", "服务端错误！");
    MainWindow::GetInstance().GetFilesPage()->ProcessDownloadError();
}

void MsgHandler::ProcessShareFileRequest(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;

    char UserNameRecv[25];
    char *FileNameRecv = new char[rPDU->MsgLen]();
    memcpy(UserNameRecv, rPDU->ParaData, 24);
    memcpy(FileNameRecv, rPDU->Msg, rPDU->MsgLen);

    qDebug() << "来源用户名 " << UserNameRecv;
    qDebug() << "来源文件名 " << FileNameRecv;

    int ret = QMessageBox::question(MainWindow::GetInstance().GetFilesPage(), "收到分享的文件", QString("您收到了来自 %1 \n的 %2 \n您是否需要接收?\nTips:\n新接收的文件会存放在 Pack From Shared 文件夹中。")
                                                                                                    .arg(UserNameRecv, FileNameRecv));

    PDU *sPDU = CreatePDU(0);
    sPDU->MsgType = MSG_TYPE_SHARE_FILE_RESPOND;
    memcpy(sPDU->ParaData, &ret, sizeof(int));
    LoginPage::GetInstance().SendPDU(sPDU);
}

void MsgHandler::ProcessShareFileRespond(const PDU *rPDU)
{
    if(rPDU == NULL)
        return;

    int status;
    memcpy(&status, rPDU->ParaData, sizeof(int));
    qDebug() << "分享文件传回值 = " << status;

    if(status > 0)
    {
        QMessageBox::information(MainWindow::GetInstance().GetFilesPage(), "分享文件成功", QString("有 %1 位好友已收到消息！").arg(QString::number(status)));
        return;
    }
    else if(status < 0)
    {
        if(status == -1)
        {
            QMessageBox::information(MainWindow::GetInstance().GetFilesPage(), "接收文件成功", "接收分享的文件成功！");
            MainWindow::GetInstance().GetFilesPage()->RefreshFilesList();
            return;
        }
        else
        {
            QMessageBox::warning(MainWindow::GetInstance().GetFilesPage(), "接收文件失败", "接收分享的文件失败！");
            return;
        }
    }
    else
    {
        QMessageBox::warning(MainWindow::GetInstance().GetFilesPage(), "分享文件失败", "分享文件失败，没有好友收到消息！");
        return;
    }
}
