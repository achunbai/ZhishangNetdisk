#include "chat.h"
#include "ui_chat.h"
#include "zhishangprotocol.h"
#include "loginpage.h"

Chat::Chat(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Chat)
{
    ui->setupUi(this);
}

Chat::~Chat()
{
    delete ui;
}

QString &Chat::GetTargetUserName()
{
    return TargetUserName;
}

void Chat::SetTitle(const QString &UserName)
{
    ui->ChatTitle->setText(QString("**%1** 与 **%2** 的聊天").arg(LoginPage::GetInstance().GetCurrentUser(), UserName));
    setWindowTitle(QString("%1 与 %2 的聊天").arg(LoginPage::GetInstance().GetCurrentUser(), UserName));
}

void Chat::SetMsg(const char *UserName, const char *Message)
{
    qDebug() << "写入来自" << UserName << "的消息";
    // 将对方的聊天内容写入窗口
    // 可以使用QTextEdit的append()函数来添加带颜色的文本
    // 需要先将文本转换为HTML格式，然后使用HTML的<font>标签来设置颜色
    // textEdit->append("<font color=\"" + color + "\">" + text + "</font>");
    // 自己的ID为红色，别人的ID为蓝色
    ui->ChatContentTE->append(QString("<font color=\"blue\">%1：</font>").arg(UserName));
    ui->ChatContentTE->append(QString::fromUtf8(Message));
}

void Chat::on_SendPB_clicked()
{
    QString Message = ui->ChatLE->text();
    if(Message.isEmpty())
        return;

    // 创建发送的消息
    int length = Message.toStdString().size();
    PDU *sPDU = CreatePDU(length);
    // 协议类型设置为聊天
    sPDU->MsgType = MSG_TYPE_CHAT;
    // 参数写入双方用户名
    QString CurrentUserName = LoginPage::GetInstance().GetCurrentUser();
    memcpy(sPDU->ParaData, CurrentUserName.toStdString().c_str(), 24);
    memcpy(sPDU->ParaData + 24, TargetUserName.toStdString().c_str(), 24);
    // 写入聊天内容
    memcpy(sPDU->Msg, Message.toStdString().c_str(), length);
    // 发送PDU
    LoginPage::GetInstance().SendPDU(sPDU);
    // 清空输入
    ui->ChatLE->clear();

    // 将自己的聊天内容写入窗口
    // 可以使用QTextEdit的append()函数来添加带颜色的文本
    // 需要先将文本转换为HTML格式，然后使用HTML的<font>标签来设置颜色
    // textEdit->append("<font color=\"" + color + "\">" + text + "</font>");
    // 自己的ID为红色，别人的ID为蓝色
    ui->ChatContentTE->append("<font color=\"red\">" + CurrentUserName + "：" + "</font>");
    ui->ChatContentTE->append(Message);
}

