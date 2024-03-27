#include "onlineusers.h"
#include "ui_onlineusers.h"
#include "zhishangprotocol.h"
#include "loginpage.h"

OnlineUsers::OnlineUsers(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OnlineUsers)
{
    ui->setupUi(this);
}

OnlineUsers::~OnlineUsers()
{
    delete ui;
}

void OnlineUsers::SetOnlineUserList(const QStringList OnlineUserList)
{
    ui->listWidget->clear();
    ui->listWidget->addItems(OnlineUserList);
}

void OnlineUsers::ClearOnlineUserList()
{
    ui->listWidget->clear();
}

void OnlineUsers::SetTitle(const QString &UserName)
{
    QString Title = QString("%1 - 在线用户列表 - 智商网盘").arg(UserName);
    setWindowTitle(Title);
}

void OnlineUsers::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QString TargetUser = item->text();
    int ret = QMessageBox::question(this, "添加好友？", "您是否需要添加 "+ TargetUser + " 为好友？");
    if(ret == QMessageBox::Yes)
    {
        PDU* sPDU = CreatePDU(0);
        sPDU->MsgType = MSG_TYPE_ADD_FRIEND_REQUEST;
        memcpy(sPDU->ParaData, LoginPage::GetInstance().GetCurrentUser().toStdString().c_str(), 24);
        memcpy(sPDU->ParaData + 24, TargetUser.toStdString().c_str(), 24);
        LoginPage::GetInstance().SendPDU(sPDU);
        qDebug() << "发送好友请求成功";
    }
    else
        qDebug() << "用户拒绝添加好友";
}

