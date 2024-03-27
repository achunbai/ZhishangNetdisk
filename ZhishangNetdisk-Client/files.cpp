#include "files.h"
#include "ui_files.h"
#include "zhishangprotocol.h"
#include "loginpage.h"
#include <QInputDialog>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>

Files::Files(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Files)
    , IsUploading(false)
    , SizeToSend(0)
    , IsDownloading(false)
    , SizeToReceive(0)
    , SizeReceived(0)
{
    ui->setupUi(this);
    UpBar = new UploadProgress;
    DownBar = new DownloadProgress;
    SharePage = new Share;
    qDebug() << "IsUploading " << IsUploading;
    qDebug() << "SizeToSend " << SizeToSend;
    qDebug() << "IsDownloading " << IsDownloading;
    qDebug() << "SizeToReceive " << SizeToReceive;
    qDebug() << "SizeReceived " << SizeReceived;
}

Files::~Files()
{
    delete ui;
    delete UpBar;
    delete DownBar;
    delete SharePage;
}

void Files::SetTitle(const QString &UserName)
{
    QString Title = QString("**%1** 的文件 - **根目录**").arg(UserName);
    setWindowTitle(Title);
    ui->FileListLabel->setText(Title);
}

QString Files::GetCurrentPath()
{
    return CurrentPath;
}

void Files::SetCurrentPath(const QString &Path)
{
    CurrentPath = Path;
}

void Files::RefreshFilesList()
{
    // 创建PDU
    int len = CurrentPath.toStdString().size() + 1;
    PDU *sPDU = CreatePDU(len);
    sPDU->MsgType = MSG_TYPE_REFRESH_FILES_LIST_REQUEST;
    memcpy(sPDU->Msg, CurrentPath.toStdString().c_str(), len);
    LoginPage::GetInstance().SendPDU(sPDU);
}

void Files::RefreshFilesListAfterMove()
{
    qDebug() << "PathBefore = " << PathBefore;
    qDebug() << "CurrentPathBefore = " << CurrentPath;

    // 和直接刷新文件列表不同，需要返回到之前的路径
    int idx = PathBefore.lastIndexOf('/');
    qDebug() << "PathBefore.lastIndexOf('/')  idx = " << idx;
    // 判断是否在根目录，根目录的话那就直接将当前路径改为空即可
    if(idx == -1)
        CurrentPath = "";
    else
        CurrentPath = PathBefore.remove(idx, PathBefore.size() - idx);

    qDebug() << "CurrentPathAfter = " << CurrentPath;
    RefreshFilesList();
}

void Files::UploadFile()
{
    PDU *sPDU = CreatePDU(4096);
    sPDU->MsgType = MSG_TYPE_UPLOAD_FILE_DATA;
    QFile file(UploadFilePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        ProcessUploadError();
        qDebug() << "本地文件打开失败";
        QMessageBox::warning(this, "上传文件失败", QString("打开 %1 失败，请检查是否有权限！").arg(UploadFileName));

        // 隐藏上传进度条
        if(!UpBar->isHidden())
            UpBar->hide();
    }

    // 显示上传进度条
    if(UpBar->isHidden())
        UpBar->show();

    int ReadCount, SizeSended = 0;
    char *pData = new char[4096]();
    while(true)
    {
        ReadCount = file.read(pData, 4096);

        // 设置上传进度条
        SizeSended += ReadCount;
        UpBar->SetUploadProgress(SizeSended);
        UpBar->SetStatus(SizeToSend, SizeSended, (double)SizeSended / (double)SizeToSend);

        // 传输完成或者失败都需要关闭文件，设置传输状态
        if(ReadCount == 0)
        {
            // QMessageBox::information(this, "上传文件", "文件传输完成");
            qDebug() << "文件传输完成";
            file.close();
            IsUploading = false;
            RefreshFilesList();
            delete[] sPDU;
            sPDU = NULL;

            // 隐藏上传进度条
            // if(!UpBar->isHidden())
            //     UpBar->hide();
            return;
        }
        if(ReadCount == -1)
        {
            ProcessUploadError();
            QMessageBox::critical(this, "上传文件失败", QString("上传 %1 失败，请重试！").arg(UploadFileName));
            file.close();
            IsUploading = false;
            delete[] sPDU;
            sPDU = NULL;

            // 隐藏上传进度条
            if(!UpBar->isHidden())
                UpBar->hide();
            return;
        }
        sPDU->PDULen =  ReadCount + sizeof(PDU);
        sPDU->MsgLen = ReadCount;
        memcpy(sPDU->ParaData, &ReadCount, sizeof(int));
        memcpy(sPDU->Msg, pData, ReadCount);

        LoginPage::GetInstance().GetSocket().write((char*)sPDU, sPDU->PDULen);
    }

    delete sPDU;
    delete[] pData;
}

void Files::on_NewFolderPB_clicked()
{
    QString FolderName = QInputDialog::getText(this, "输入您想创建的文件夹名", "文件夹名称：");
    // 去除多余空格
    FolderName = FolderName.trimmed();
    // 检查文件夹名称是否为空
    if(FolderName.isEmpty())
    {
        qDebug() << "文件夹名称为空";
        QMessageBox::warning(this, "警告", "文件夹名称为空！");
        return;
    }

    if(FolderName.toStdString().size() > 64)
    {
        qDebug() << "文件夹名称过长";
        QMessageBox::warning(this, "警告", "文件夹名称过长！");
        return;
    }

    // 检查文件夹名是否包含非法字符
    QString IllegalCharacters = "\\/:?\"'<>|";
    for (int i = 0; i < FolderName.length(); i++)
    {
        if (IllegalCharacters.contains(FolderName.at(i)))
        {
            qDebug() << "文件夹名称包含非法字符";
            QMessageBox::warning(this, "警告", "文件夹名称包含非法字符！");
            return;
        }
    }

    // 创建PDU，将创建的文件夹名称发送给服务器
    int len = CurrentPath.toStdString().size() + 1;
    PDU *sPDU = CreatePDU(len);
    sPDU->MsgType = MSG_TYPE_NEW_FOLDER_REQUEST;
    memcpy(sPDU->ParaData, FolderName.toStdString().c_str(), 64);
    memcpy(sPDU->Msg, CurrentPath.toStdString().c_str(), len);
    LoginPage::GetInstance().SendPDU(sPDU);
}

void Files::on_RefreshPB_clicked()
{
    RefreshFilesList();
}

void Files::SetFileList(QList<FileInfo *> pFileList)
{
    // 释放上一次的文件列表
    foreach(FI* pItem, FileList)
        delete pItem;

    // 清空当前的文件列表，并更新
    FileList.clear();
    FileList = pFileList;

    // 先清空列表
    ui->FileListLW->clear();

    // 遍历文件列表，逐个设置项目类型
    foreach (FI* FileInfo, FileList)
    {
        QListWidgetItem *item = new QListWidgetItem;
        switch(FileInfo->FileType)
        {
            case FILE_TYPE_AUDIO:
                item->setIcon(QIcon(QPixmap(":/icons/music-fill.svg")));
                break;
            case FILE_TYPE_VIDEO:
                item->setIcon(QIcon(QPixmap(":/icons/file-video-fill.svg")));
                break;
            case FILE_TYPE_DOCUMENT:
                item->setIcon(QIcon(QPixmap(":/icons/file-pdf-fill.svg")));
                break;
            case FILE_TYPE_IMAGE:
                item->setIcon(QIcon(QPixmap(":/icons/file-image-fill.svg")));
                break;
            case FILE_TYPE_FILE:
                item->setIcon(QIcon(QPixmap(":/icons/file-fill.svg")));
                break;
            case FILE_TYPE_FOLDER:
                item->setIcon(QIcon(QPixmap(":/icons/folder-6-fill.svg")));
                break;
            case FILE_TYPE_TEXT:
                item->setIcon(QIcon(QPixmap(":/icons/file-text-fill.svg")));
                break;
            default:
                item->setIcon(QIcon(QPixmap(":/icons/file-unknow-fill.svg")));
                break;
        }
        item->setText(FileInfo->FileName);
        qDebug() << "文件名：" << FileInfo->FileName;
        ui->FileListLW->addItem(item);
    }
}

QString Files::GetTargetFileName()
{
    return TargetFileName;
}

void Files::on_DeleteFolderPB_clicked()
{
    QListWidgetItem *item = ui->FileListLW->currentItem();
    if(item == NULL)
    {
        QMessageBox::warning(this, "删除文件夹", "请选择要删除的文件夹！");
        return;
    }

    int index = ui->FileListLW->currentIndex().row();
    FI* pFile = FileList.at(index);
    if(pFile && QString(pFile->FileName) == item->text())
    {
        // qDebug() << "通过index判断";
        if(pFile->FileType != FILE_TYPE_FOLDER)
        {
            QMessageBox::warning(this, "删除文件夹", "请选择一个文件夹！");
            return;
        }
    }

    int status = QMessageBox::question(this, "你确定吗？", QString("您确定要删除文件夹 %1 吗？").arg(item->text()));
    if(status == QMessageBox::No)
        return;

    QString TargetPath;
    if(CurrentPath.isEmpty())
        TargetPath = item->text();
    else
        TargetPath = QString("%1/%2").arg(CurrentPath, item->text());
    qDebug() << "删除的文件夹路径：" << TargetPath;
    int len = TargetPath.toStdString().size() + 1;
    PDU *sPDU = CreatePDU(len);
    sPDU->MsgType = MSG_TYPE_DELETE_FOLDER_REQUEST;
    memcpy(sPDU->Msg, TargetPath.toStdString().c_str(), len);
    LoginPage::GetInstance().SendPDU(sPDU);
}

void Files::on_DeleteFilePB_clicked()
{
    QListWidgetItem *item = ui->FileListLW->currentItem();
    if(item == NULL)
    {
        QMessageBox::warning(this, "删除文件", "请选择要删除的文件！");
        return;
    }

    int index = ui->FileListLW->currentIndex().row();
    FI* pFile = FileList.at(index);
    if(pFile && QString(pFile->FileName) == item->text())
    {
        // qDebug() << "通过index判断";
        if(pFile->FileType == FILE_TYPE_FOLDER)
        {
            QMessageBox::warning(this, "删除文件", "请选择一个文件！");
            return;
        }
    }

    int status = QMessageBox::question(this, "你确定吗？", QString("您确定要删除文件 %1 吗？").arg(item->text()));
    if(status == QMessageBox::No)
        return;

    qDebug() << "CurrentPath " << CurrentPath;
    qDebug() << "删除的文件路径：" << QString("%1/%2").arg(CurrentPath, item->text());
    int len = CurrentPath.toStdString().size() + 1;
    PDU *sPDU = CreatePDU(len);
    sPDU->MsgType = MSG_TYPE_DELETE_FILE_REQUEST;
    memcpy(sPDU->ParaData, item->text().toStdString().c_str(), 64);
    memcpy(sPDU->Msg, CurrentPath.toStdString().c_str(), len);
    LoginPage::GetInstance().SendPDU(sPDU);
}


void Files::on_RenamePB_clicked()
{
    // 清空三个参数
    PathBefore = PathAfter = TargetFileName = "";

    QListWidgetItem *item = ui->FileListLW->currentItem();
    if(item == NULL)
    {
        QMessageBox::warning(this, "重命名", "请选择要重命名的项目！");
        return;
    }

    // 获取新的名称
    QString NameAfter = QInputDialog::getText(this, "请输入新的名称", "新名称：");

    // 检查文件名称长度是否符合要求
    if(NameAfter.toStdString().size() > 64)
    {
        qDebug() << "文件名过长";
        QMessageBox::warning(this, "警告", "文件名过长！");
        return;
    }
    if(NameAfter.isEmpty())
    {
        qDebug() << "文件名为空";
        QMessageBox::warning(this, "警告", "文件名为空！");
        return;
    }

    // 检查文件夹名是否包含非法字符
    QString IllegalCharacters = "\\/:?\"'<>|";
    for (int i = 0; i < NameAfter.length(); i++)
    {
        if (IllegalCharacters.contains(NameAfter.at(i)))
        {
            qDebug() << "文件名称包含非法字符";
            QMessageBox::warning(this, "警告", "文件名称包含非法字符！");
            return;
        }
    }

    QString NameBefore = item->text();
    qDebug() << "原名称：" << NameBefore << " 新名称" << NameAfter;

    // 获取三个参数 当前路径长度 原文件名长度 目标文件名长度
    int PathLen = CurrentPath.toStdString().size() + 1;
    int NameBeforeLen = NameBefore.toStdString().size() + 1;
    int NameAfterLen = NameAfter.toStdString().size() + 1;

    PDU *sPDU = CreatePDU(PathLen + NameBeforeLen + NameAfterLen);
    sPDU->MsgType = MSG_TYPE_RENAME_REQUEST;
    // 顺序 当前路径 原文件名 目标文件名
    memcpy(sPDU->ParaData, &PathLen, sizeof(int));
    memcpy(sPDU->ParaData + sizeof(int), &NameBeforeLen, sizeof(int));
    memcpy(sPDU->ParaData + sizeof(int) + sizeof(int), &NameAfterLen, sizeof(int));
    memcpy(sPDU->Msg, CurrentPath.toStdString().c_str(), PathLen);
    memcpy(sPDU->Msg + PathLen, NameBefore.toStdString().c_str(), NameBeforeLen);
    memcpy(sPDU->Msg + PathLen + NameBeforeLen, NameAfter.toStdString().c_str(), NameAfterLen);
    LoginPage::GetInstance().SendPDU(sPDU);
}


void Files::on_FileListLW_itemDoubleClicked(QListWidgetItem *item)
{
    QString FolderName = item->text();

    int index = ui->FileListLW->currentIndex().row();
    FI* pFile = FileList.at(index);
    if(pFile && QString(pFile->FileName) == item->text())
    {
        // qDebug() << "通过index判断";
        if(pFile->FileType != FILE_TYPE_FOLDER)
        {
            QMessageBox::warning(this, "进入文件夹", "请选择一个文件夹！");
            return;
        }
    }

    // 修改当前路径
    // 分两种情况，第一种：在根目录，就不需要加上"/"，直接赋FolderName即可
    // 第二种：不在根目录，加上"/"
    if(CurrentPath.isEmpty())
        CurrentPath = FolderName;
    else
        CurrentPath = CurrentPath + '/' + FolderName;
    // 设置上面的标签
    QString Title = QString("**%1** 的文件 - **%2**").arg(LoginPage::GetInstance().GetCurrentUser(), FolderName);
    ui->FileListLabel->setText(Title);
    // 刷新文件列表
    RefreshFilesList();
}


void Files::on_ReturnPB_clicked()
{
    // 判断当前路径是否为根目录
    if(CurrentPath.isEmpty())
        QMessageBox::warning(this, "返回失败", "您现在处于根目录！");

    // 改变当前路径
    int idx = CurrentPath.lastIndexOf('/');
    // 如果idx = -1，代表这个文件夹是根目录下面的一个文件夹，那么将当前路径置空即可
    if(idx == -1)
        CurrentPath = "";
    else
        // 否则从最后一个斜杠开始删除到最后即为上一级路径
        CurrentPath.remove(idx, CurrentPath.length() - idx);
    // 设置上面的标签
    QString Title;

    if(CurrentPath.isEmpty())
        Title = QString("**%1** 的文件 - **根目录**").arg(LoginPage::GetInstance().GetCurrentUser());
    else
    {
        idx = CurrentPath.lastIndexOf('/');
        QString FolderName;
        // 同样文件夹名有两种情况
        if(idx == -1)
            FolderName = CurrentPath;
        else
            FolderName = CurrentPath.right(CurrentPath.size() - idx - 1);
        Title = QString("**%1** 的文件 - **%2**").arg(LoginPage::GetInstance().GetCurrentUser(), FolderName);
    }
    ui->FileListLabel->setText(Title);
    // 刷新文件列表
    RefreshFilesList();
}


void Files::on_MovePB_clicked()
{
    QListWidgetItem *item = ui->FileListLW->currentItem();

    // 因为这个按钮承担了确认和取消的作用，所以需要判断当前状态是移动文件还是确认取消
    if(ui->MovePB->text() == "移动")
    {
        // 判断是否选择了需要移动的文件
        if(item == NULL)
        {
            QMessageBox::warning(this, "移动文件", "请选择需要移动的文件！");
            return;
        }
        // 先获取文件名
        TargetFileName = item->text();
        // 获取当前路径
        PathBefore = CurrentPath;

        // 最后将按钮设置为确认/取消
        ui->MovePB->setText("确认/取消");
    }
    else
    {
        // 如果路径一致，表示取消，不需要进行后续操作
        if(CurrentPath != PathBefore)
        {
            // 获取当前路径
            PathAfter = CurrentPath;
            // 判断是否选择了文件夹
            if(item != NULL)
            {
                int index = ui->FileListLW->currentIndex().row();
                FI* pFile = FileList.at(index);
                if(pFile && QString(pFile->FileName) == item->text())
                {
                    // qDebug() << "通过index判断";
                    if(pFile->FileType == FILE_TYPE_FOLDER)
                    {
                        qDebug() << "选择了文件夹";
                        // 是文件夹那就路径加一层
                        PathAfter += '/' + item->text();
                    }
                }
            }

            // 询问是否需要移动文件
            int status = QMessageBox::question(this, "您确定吗？", QString("您是否需要将 %1\n从\n%2\n移动到\n%3？").arg(TargetFileName, PathBefore, PathAfter));

            if(status == QMessageBox::Yes)
            {
                // 没选择文件夹那就是当前路径，路径不变，选择了的话也加上了
                // 构建路径
                if(PathBefore.isEmpty())
                    PathBefore = TargetFileName;
                else
                    PathBefore += '/' + TargetFileName;
                if(PathAfter.isEmpty())
                    PathAfter = TargetFileName;
                else
                    PathAfter += '/' + TargetFileName;
                // 计算长度
                int PathBeforeLen = PathBefore.toStdString().size() + 1;
                int PathAfterLen = PathAfter.toStdString().size() + 1;

                // 发送数据包
                PDU *sPDU = CreatePDU(PathBeforeLen + PathAfterLen);
                sPDU->MsgType = MSG_TYPE_MOVE_REQUEST;
                memcpy(sPDU->ParaData, &PathBeforeLen, sizeof(int));
                memcpy(sPDU->ParaData + sizeof(int), &PathAfterLen, sizeof(int));
                memcpy(sPDU->Msg, PathBefore.toStdString().c_str(), PathBeforeLen);
                memcpy(sPDU->Msg + PathBeforeLen, PathAfter.toStdString().c_str(), PathAfterLen);
                LoginPage::GetInstance().SendPDU(sPDU);
            }
        }

        // 记得将按钮还原回移动文件
        ui->MovePB->setText("移动");
    }
}


void Files::on_UploadPB_clicked()
{
    // 防止多个文件同时传输
    if(IsUploading)
    {
        QMessageBox::critical(this, "文件上传失败", "已有文件正在上传！");
        return;
    }

    // 先隐藏上传进度条
    if(!UpBar->isHidden())
        UpBar->hide();

    // 文件最好不要一次性传输，防止文件过大传输失败
    // 获取本地文件路径和文件名
    UploadFilePath.clear();
    UploadFilePath = QFileDialog::getOpenFileName();
    qDebug() << "FilePath = " << UploadFilePath;
    int idx = UploadFilePath.lastIndexOf('/');
    UploadFileName.clear();
    UploadFileName = UploadFilePath.right(UploadFilePath.length() - idx - 1);
    qDebug() << "FileName = " << UploadFileName;

    // 打开本地文件
    QFile UploadFile(UploadFilePath);

    // 存储文件大小
    // 折算成文件大小，qint64 可以表示的最大文件大小是 9,223,372,036,854,775,807 字节，
    // 约等于 8 ZB（Zettabytes，1 ZB = 10^21 字节）。这个大小远远超过了现代硬盘的容量，所以 qint64 足以表示任何现实中可能存在的文件大小。
    SizeToSend = UploadFile.size();

    // 设置上传进度页面参数
    UpBar->SetTitle(UploadFileName, CurrentPath);
    UpBar->SetStatus(SizeToSend, 0, 0);
    UpBar->SetUploadProgressRange(0, SizeToSend);

    // 将文件路径存放在Msg中，文件大小存放在ParaData中
    int PathLen = (int)CurrentPath.toStdString().size() + 1;
    int NameLen = (int)UploadFileName.toStdString().size() + 1;

    // 输出文件的基本信息
    qDebug() << "文件大小：" << SizeToSend;
    qDebug() << "当前文件路径：" << CurrentPath;
    qDebug() << "文件路径长度：" << PathLen;
    qDebug() << "文件名长度：" << NameLen;

    PDU *sPDU = CreatePDU(PathLen + NameLen);
    sPDU->MsgType = MSG_TYPE_UPLOAD_FILE_REQUEST;
    memcpy(sPDU->Msg, CurrentPath.toStdString().c_str(), PathLen);
    memcpy(sPDU->Msg + PathLen, UploadFileName.toStdString().c_str(), NameLen);
    memcpy(sPDU->ParaData, &SizeToSend, sizeof(qint64));
    memcpy(sPDU->ParaData + sizeof(qint64), &PathLen, sizeof(int));
    memcpy(sPDU->ParaData + sizeof(qint64) + sizeof(int), &NameLen, sizeof(int));
    LoginPage::GetInstance().SendPDU(sPDU);
}

void Files::ProcessUploadError()
{
    PDU *sPDU = CreatePDU(0);
    sPDU->MsgType = MSG_TYPE_UPLOAD_FILE_ERROR;
    LoginPage::GetInstance().SendPDU(sPDU);
    IsUploading = false;

    // 隐藏上传进度条
    if(!UpBar->isHidden())
        UpBar->hide();
}

bool &Files::GetIsUploading()
{
    return IsUploading;
}

QString Files::GetUploadFileName()
{
    return UploadFileName;
}

qint64 Files::GetSizeToReceive()
{
    return SizeToReceive;
}

bool Files::GetIsDownloading()
{
    return IsDownloading;
}

void Files::SetSizeToReceive(qint64 SizeToReceive)
{
    this->SizeToReceive = SizeToReceive;
    qDebug() << "SetSizeToReceive = " << SizeToReceive;
}

void Files::SetIsDownloading(bool IsDownloading)
{
    this->IsDownloading = IsDownloading;
    qDebug() << "IsDownloading = " << IsDownloading;
}

void Files::ProcessDownloadError()
{
    // 下载失败，文件存在就继续判断是否打开，关闭后删除
    if(!FileToReceive.exists())
        return;
    if(FileToReceive.isOpen())
        FileToReceive.close();
    FileToReceive.close();

    IsDownloading = false;

    // 隐藏下载进度条
    if(!UpBar->isHidden())
        UpBar->hide();
}

void Files::ProcessDownloadData(const char *Data, const int Length)
{
    if(Length == 0)
        return;


    int WriteCount = FileToReceive.write(Data);
    SizeReceived += WriteCount;
    qDebug() << "文件 " << FileToReceiveName << " 写入长度：" << WriteCount;

    // 设置下载进度条
    DownBar->SetDownloadProgress(SizeReceived);
    DownBar->SetStatus(SizeToReceive, SizeReceived, (double)SizeReceived / (double)SizeToReceive);

    // 文件写入错误就处理
    if(WriteCount != Length || WriteCount == -1 || (Length < 4096 && SizeReceived != SizeToReceive))
    {
        ProcessDownloadError();
        QMessageBox::warning(this, "下载文件失败", "文件写入错误！");
        // 隐藏下载进度条
        if(!UpBar->isHidden())
            UpBar->hide();
        return;
    }

    // 成功提示
    if(WriteCount < 4096)
    {
        FileToReceive.close();
        QMessageBox::warning(this, "下载文件成功", "文件" + FileToReceiveName +"下载成功！");
        // 隐藏下载进度条
        if(!UpBar->isHidden())
            UpBar->hide();
        return;
    }
}

UploadProgress *Files::GetUpBar()
{
    return UpBar;
}

DownloadProgress *Files::GetDownBar()
{
    return DownBar;
}

void Files::on_DownloadPB_clicked()
{
    // 隐藏下载进度条
    if(!DownBar->isHidden())
        DownBar->hide();

    // 先判断是否有文件在下载
    if(IsDownloading)
    {
        QMessageBox::information(this, "下载文件", "已有文件正在下载！");
        return;
    }

    QListWidgetItem *item = ui->FileListLW->currentItem();

    // 判断是否选择了文件
    if(item == NULL)
    {
        qDebug() << "没有选择下载的文件";
        QMessageBox::warning(this, "下载文件", "请选择需要下载的文件！");
        return;
    }

    // 判断是否选择了文件夹
    int index = ui->FileListLW->currentIndex().row();
    FI* pFile = FileList.at(index);
    if(pFile && QString(pFile->FileName) == item->text())
    {
        // qDebug() << "通过index判断";
        if(pFile->FileType == FILE_TYPE_FOLDER)
        {
            qDebug() << "选择了文件夹";
            QMessageBox::warning(this, "下载文件", "请选择需要下载的文件，不是文件夹！");
            return;
        }
    }

    QString DownloadFilePath = QFileDialog::getSaveFileName();

    // 提示用户是否需要下载文件
    if(QMessageBox::question(this, "下载文件", QString("您确定要下载 %1 \n至\n %2 吗？").arg(item->text(), DownloadFilePath)) == QMessageBox::No)
        return;

    FileToReceive.setFileName(DownloadFilePath);
    if(!FileToReceive.open(QIODevice::WriteOnly))
        return;

    // 保存当前选择文件名
    FileToReceiveName = item->text();

    // 拼接当前文件路径
    QString TargetFilePath;
    if(CurrentPath.isEmpty())
        TargetFilePath = item->text();
    else
        TargetFilePath = QString("%1/%2").arg(CurrentPath, item->text());

    // 设置下载进度页面参数
    DownBar->SetTitle(FileToReceiveName, DownloadFilePath);

    // 发送给服务器
    int PathLen = TargetFilePath.toStdString().size() + 1;
    PDU *sPDU = CreatePDU(PathLen);
    sPDU->MsgType = MSG_TYPE_DOWNLOAD_FILE_REQUEST;
    memcpy(sPDU->Msg, TargetFilePath.toStdString().c_str(), PathLen);
    LoginPage::GetInstance().SendPDU(sPDU);
}


void Files::on_SharePB_clicked()
{
    QListWidgetItem *item = ui->FileListLW->currentItem();

    if(item == NULL)
    {
        QMessageBox::warning(this, "分享", "请选择要分享的项目！");
        return;
    }

    SharePage->SetTitle(item->text());
    SharePage->SetTargetFilePath(CurrentPath);
    SharePage->RefreshFriendList();

    if(SharePage->isHidden())
        SharePage->show();
}

