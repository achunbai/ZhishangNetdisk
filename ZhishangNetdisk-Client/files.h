#ifndef FILES_H
#define FILES_H

#include <QWidget>
#include <QListWidgetItem>
#include <QFile>
#include "zhishangprotocol.h"
#include "uploadprogress.h"
#include "downloadprogress.h"
#include "share.h"

namespace Ui {
class Files;
}

class Files : public QWidget
{
    Q_OBJECT

public:
    explicit Files(QWidget *parent = nullptr);
    ~Files();
    void SetTitle(const QString &UserName);
    QString GetCurrentPath();
    void SetCurrentPath(const QString &Path);
    void RefreshFilesList();
    void SetFileList(QList<FI*> pFileList);
    QString GetTargetFileName();
    void RefreshFilesListAfterMove();
    void UploadFile();
    void ProcessUploadError();
    bool &GetIsUploading();
    QString GetUploadFileName();
    qint64 GetSizeToReceive();
    bool GetIsDownloading();
    void SetSizeToReceive(qint64 SizeToReceive);
    void SetIsDownloading(bool IsDownloading);
    void ProcessDownloadError();
    void ProcessDownloadData(const char *Data, const int Length);
    UploadProgress *GetUpBar();
    DownloadProgress *GetDownBar();



private slots:
    void on_NewFolderPB_clicked();

    void on_RefreshPB_clicked();

    void on_DeleteFolderPB_clicked();

    void on_DeleteFilePB_clicked();

    void on_RenamePB_clicked();

    void on_FileListLW_itemDoubleClicked(QListWidgetItem *item);

    void on_ReturnPB_clicked();

    void on_MovePB_clicked();

    void on_UploadPB_clicked();

    void on_DownloadPB_clicked();

    void on_SharePB_clicked();

private:
    Ui::Files *ui;
    QString CurrentPath;
    QList<FI*> FileList;

    // 移动文件对应参数
    QString PathBefore;
    QString PathAfter;
    QString TargetFileName;

    // 上传文件对应参数
    QString UploadFilePath;
    QString UploadFileName;
    bool IsUploading;
    qint64 SizeToSend;

    // 下载文件对应参数
    bool IsDownloading;
    QFile FileToReceive;
    QString FileToReceiveName;
    qint64 SizeToReceive;
    qint64 SizeReceived;

    // 上传下载进度条
    UploadProgress *UpBar;
    DownloadProgress *DownBar;

    // 分享页面
    Share *SharePage;

};

#endif // FILES_H
