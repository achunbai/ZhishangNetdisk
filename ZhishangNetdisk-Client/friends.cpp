#include "friends.h"
#include "ui_friends.h"
#include "zhishangprotocol.h"
#include "loginpage.h"
#include <QInputDialog>
#include <QMessageBox>

Friends::Friends(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Friends)
{
    ui->setupUi(this);
    OnlineUsersList = new OnlineUsers;
    ChatWindow = new Chat;
}

OnlineUsers *Friends::GetOnlineUsersList()
{
    return OnlineUsersList;
}

Friends::~Friends()
{
    delete ui;
    delete OnlineUsersList;
    delete ChatWindow;
}

void Friends::SetOnlineUserList(const QStringList UserList, const int UserNumber)
{
    if(!UserList.isEmpty() && UserNumber != 0)
    {
        QMessageBox::information(this, "提示", "在线用户数为：" + QString::number(UserNumber));
        // listWidget清空并添加在线用户列表
        OnlineUsersList->SetOnlineUserList(UserList);
    }
    else
    {
        // 肯定不可能没有用户在线，没有的话那就剩自己了
        QMessageBox::critical(this, "提示", "只有您在线！");
        OnlineUsersList->ClearOnlineUserList();
    }
}

void Friends::SetFriendList(const QStringList FriendList, const QVector<int> Online, const int FriendNumber, const int OnlineNumber)
{
    if(!FriendList.isEmpty() && FriendNumber != 0)
    {
        QMessageBox::information(this, "提示", "您的好友数为：" + QString::number(FriendNumber) +  "\n在线：" + QString::number(OnlineNumber) + "\n离线：" + QString::number(FriendNumber - OnlineNumber));
        ProcessSetFriendList(FriendList, Online);
        // ui->UserListLW->addItems(FriendList);
    }
    else
    {
        QMessageBox::critical(this, "真抱歉", "您没有好友！");
        // 可能存在删完了好友的情况，也需要清空列表
        ui->UserListLW->clear();
    }
}

void Friends::ProcessSetFriendList(const QStringList FriendList, const QVector<int> Online)
{
    // listWidget清空并添加在线用户列表
    ui->UserListLW->clear();

    // 确保用户名列表和在线状态列表的长度相同
    if(FriendList.size() == Online.size())
    {
        // 遍历列表和在线状态，在线设置为红色，离线设置为黑色
        for(int i = 0; i < FriendList.size(); i++)
        {
            QListWidgetItem *item = new QListWidgetItem(FriendList[i]);
            if(Online[i] == 1)
                item->setForeground(Qt::red);
            else
                item->setForeground(Qt::black);
            ui->UserListLW->addItem(item);
            qDebug() << "Friends";
            qDebug() << "foreground = " << item->foreground();
            qDebug() << "foreground == Qt::red" << (item->foreground() == Qt::red);
        }
    }
    else
        QMessageBox::critical(this, "错误", "好友列表数据接收错误，请检查网络连接！");
}

void Friends::SendRefreshFriendListRequest()
{
    // 创建刷新好友请求的数据包并发送给服务器
    PDU *sPDU = CreatePDU(0);
    sPDU->MsgType = MSG_TYPE_REFRESH_FRIEND_LIST_REQUEST;
    memcpy(sPDU->ParaData, LoginPage::GetInstance().GetCurrentUser().toStdString().c_str(), 24);
    LoginPage::GetInstance().SendPDU(sPDU);
}

void Friends::SetTitle(const QString &UserName)
{
    QString Title = QString("**%1** 的好友列表").arg(UserName);
    setWindowTitle(Title);
    ui->UserListLabel->setText(Title);
}

void Friends::SetMsg(const char *UserName, const char *Message)
{
    qDebug() << "得到来自" << UserName << "的消息";
    // 展示聊天界面
    if(ChatWindow->isHidden())
        ChatWindow->show();
    // 更新对方的用户名
    ChatWindow->GetTargetUserName() = UserName;
    // 更新标题
    ChatWindow->SetTitle(UserName);
    // 设置消息
    ChatWindow->SetMsg(UserName, Message);
}

QListWidget *Friends::GetFriendsLW()
{
    return ui->UserListLW;
}

void Friends::on_SearchUserPB_clicked()
{
    // 弹出对话框输入用户名
    QString Name4Search = QInputDialog::getText(this, "查找用户", "用户名：");
    // 后面有两个地方要用到名称大小，所以先存着，注意中文用的是UTF-8编码，一个占三个字节。
    uint NameSize = Name4Search.toStdString().size();
    // 用户名判空或者过长
    if(Name4Search.isNull())
    {
        QMessageBox::critical(this, "输入错误", "用户名不能为空，请检查输入！");
        return;
    }
    else if(NameSize > 24)
    {
        QMessageBox::critical(this, "输入错误", "用户名过长，请检查输入！");
        return;
    }
    // 创建发送的PDU
    PDU* sPDU = CreatePDU(0);
    // 设置消息类型
    sPDU->MsgType = MSG_TYPE_SEARCH_USER_REQUEST;
    // QString::size()和std::string::size()计算字符串长度的方式不同。
    // QString::size()返回的是字符串中的字符数，而std::string::size()返回的是字符串中的字节数。在UTF-8编码中，一个英文字符占用1个字节，而一个中文字符占用3个字节。
    // 所以这里需要使用toStdString().size()来获取字符串的字节数
    // 也可以使用QString::toUtf8().size()这个函数会返回QString转换为UTF-8编码后的字节数。
    memcpy(sPDU->ParaData, Name4Search.toStdString().c_str(), NameSize);
    // qDebug() << "Name4Search.toStdString().size(): " << Name4Search.toStdString().size() << " Name4Search.size(): " << Name4Search.size();
    // 发送PDU，因为ClientSocket是LoginPage中的私有类，需要创建公有的GetSocket函数来获取ClientSocket
    // 直接调用SendPDU函数发送PDU
    LoginPage::GetInstance().SendPDU(sPDU);
}


void Friends::on_OnlineUsersPB_clicked()
{
    // 判断是否已经展示了好友列表，没有展示则先展示
    if(OnlineUsersList->isHidden())
        OnlineUsersList->show();
    // 创建PDU，不需要发送任何参数，选择消息类型为查询用户列表类型
    PDU* sPDU = CreatePDU(0);
    sPDU->MsgType=MSG_TYPE_SHOW_ONLINE_USERS_REQUEST;
    // 一定要先编写一个GetSocket函数，此处的返回的是对ClientSocket的引用
    // ClientSocket是LoginPage中的私有类，类外无法直接访问
    // 直接调用SendPDU函数发送PDU
    LoginPage::GetInstance().SendPDU(sPDU);
}


void Friends::on_RefreshPB_clicked()
{
    // 调用发送刷新好友列表请求函数
    // 别处也会用到，需要单独定义
    SendRefreshFriendListRequest();
}


void Friends::on_DeletePB_clicked()
{
    // 先取地址，判断是否选择了，没选择就弹窗提示
    QListWidgetItem *CurrentItem = ui->UserListLW->currentItem();
    if(!CurrentItem)
    {
        QMessageBox::critical(this, "错误", "请选择一个要删除的好友！");
        return;
    }
    // 获取当前选择的项目内容
    QString TargetUserName = CurrentItem->text();
    // 提示用户是否要进行删除
    int status = QMessageBox::question(this, "你确定吗", QString("你确定要删除好友 %1 吗？").arg(TargetUserName));
    // 用户确认进行删除了，那就进行接下来的过程
    if(status == QMessageBox::Yes)
    {
        // 发送数据包，标准流程
        PDU *sPDU = CreatePDU(0);
        sPDU->MsgType = MSG_TYPE_DELETE_FRIEND_REQUEST;
        memcpy(sPDU->ParaData, LoginPage::GetInstance().GetCurrentUser().toStdString().c_str(), 24);
        memcpy(sPDU->ParaData + 24, TargetUserName.toStdString().c_str(), 24);
        LoginPage::GetInstance().SendPDU(sPDU);
    }
}


void Friends::on_ChatPB_clicked()
{
    // 先取地址，判断是否选择了，没选择就弹窗提示
    QListWidgetItem *CurrentItem = ui->UserListLW->currentItem();
    if(!CurrentItem)
    {
        QMessageBox::critical(this, "错误", "请选择一个要聊天的好友！");
        return;
    }

    // 判断用户是否在线
    qDebug() << "CurrentItem->foreground() " << CurrentItem->foreground();
    if(CurrentItem->foreground() == Qt::black)
    {
        QMessageBox::critical(this, "错误", "用户离线，在线用户会被标红！");
        return;
    }

    // 展示聊天界面
    if(ChatWindow->isHidden())
        ChatWindow->show();
    QString TargetUserName = CurrentItem->text();
    ChatWindow->GetTargetUserName() = TargetUserName;
    // 设置聊天标题
    ChatWindow->SetTitle(TargetUserName);
}

