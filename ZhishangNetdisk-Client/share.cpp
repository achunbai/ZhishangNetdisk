#include "share.h"
#include "ui_share.h"
#include "loginpage.h"
#include "mainwindow.h"

Share::Share(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Share)
{
    ui->setupUi(this);
}

Share::~Share()
{
    delete ui;
}

void Share::SetTitle(QString FileName)
{
    setWindowTitle(QString("将 %1 分享给").arg(FileName));
    ui->FriendLabel->setText(QString("将 **%1** 分享给").arg(FileName));
    TargetFileName = FileName;
}

void Share::SetTargetFilePath(QString CurrentFilePath)
{
    TargetFilePath = CurrentFilePath;
}

void Share::RefreshFriendList()
{
    MainWindow::GetInstance().GetFriendsPage()->SendRefreshFriendListRequest();
    QListWidget *FriendPageLW = MainWindow::GetInstance().GetFriendsPage()->GetFriendsLW();
    int len = FriendPageLW->count();
    qDebug() << "len = " << len;
    for(int i = 0; i < len; i++)
    {
        QListWidgetItem *cur = FriendPageLW->item(i);
        qDebug() << "foreground = " << cur->foreground();
        qDebug() << "foreground == Qt::red" << (cur->foreground() == Qt::red);
        if(cur->foreground() == Qt::red)
        {
            QListWidgetItem *NewItem = new QListWidgetItem(*cur);
            ui->FriendLW->addItem(NewItem);
        }
    }
}

void Share::on_SelectAllPB_clicked()
{
    ui->FriendLW->selectAll();
}

void Share::on_CancelSelectPB_clicked()
{
    ui->FriendLW->clearSelection();
}

void Share::on_ConfirmPB_clicked()
{
    QList<QListWidgetItem*> items = ui->FriendLW->selectedItems();
    int FriendsLen = items.size();
    int FriendsSize = FriendsLen * 24 + 1;

    if(FriendsLen == 0)
    {
        QMessageBox::warning(this, "分享", "请选择要分享的好友！");
        return;
    }

    int PathLen = TargetFilePath.toStdString().size() + 1;
    int NameLen = TargetFileName.toStdString().size() + 1;
    PDU *sPDU = CreatePDU(PathLen + NameLen + FriendsSize);

    sPDU->MsgType = MSG_TYPE_SHARE_FILE_REQUEST;

    memcpy(sPDU->ParaData, LoginPage::GetInstance().GetCurrentUser().toStdString().c_str(), 24);
    memcpy(sPDU->ParaData + 24, &PathLen, sizeof(int));
    memcpy(sPDU->ParaData + 24 + sizeof(int), &NameLen, sizeof(int));
    memcpy(sPDU->ParaData + 24 + sizeof(int) + sizeof(int), &FriendsSize, sizeof(int));

    for(int i = 0; i < FriendsLen; i++)
    {
        memcpy(sPDU->Msg + i * 24, items.at(i)->text().toStdString().c_str(), 24);
    }
    memcpy(sPDU->Msg + FriendsSize, TargetFilePath.toStdString().c_str(), PathLen);
    memcpy(sPDU->Msg + FriendsSize + PathLen, TargetFileName.toStdString().c_str(), NameLen);
    LoginPage::GetInstance().SendPDU(sPDU);

    // 隐藏界面
    hide();
}

