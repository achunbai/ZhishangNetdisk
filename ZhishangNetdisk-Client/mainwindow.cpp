#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "altmainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 刚开始好友列表为空，需要发送请求刷新好友列表
    ui->FriendsTab->SendRefreshFriendListRequest();
    // 刷新文件列表
    ui->FilesTab->RefreshFilesList();
}

MainWindow &MainWindow::GetInstance()
{
    static MainWindow m;
    return m;
}

Files *MainWindow::GetFilesPage()
{
    return ui->FilesTab;
}

Friends *MainWindow::GetFriendsPage()
{
    return ui->FriendsTab;
}

void MainWindow::SetTitle(const QString &UserName)
{
    QString Title = QString("%1 - 主界面 - 智商网盘").arg(UserName);
    setWindowTitle(Title);
    ui->FriendsTab->SetTitle(UserName);
    ui->FilesTab->SetTitle(UserName);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    AltMainWindow::GetInstance().show();
}


void MainWindow::on_pushButton_2_clicked()
{
    AltMainWindow::GetInstance().hide();
}

