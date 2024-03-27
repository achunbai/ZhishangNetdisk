#include "downloadprogress.h"
#include "ui_downloadprogress.h"

DownloadProgress::DownloadProgress(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DownloadProgress)
{
    ui->setupUi(this);
    ui->DownloadProgressBar->setValue(0);
}

DownloadProgress::~DownloadProgress()
{
    delete ui;
}

void DownloadProgress::SetDownloadProgressRange(int minimum, int maximum)
{
    ui->DownloadProgressBar->setRange(minimum, maximum);
}

void DownloadProgress::SetDownloadProgress(int value)
{
    ui->DownloadProgressBar->setValue(value);
}

void DownloadProgress::SetTitle(QString FileName, QString Path)
{
    setWindowTitle(QString("正在将 %1 下载至 %2").arg(FileName, Path));
    ui->TitleLabel->setText(QString("正在将 **%1** 下载至 **%2**").arg(FileName, Path));
}

void DownloadProgress::SetStatus(int SizeTotal, int SizeRemain, double Percentage)
{
    // 转换为百分比
    Percentage *= 100;
    // 转换为字符串，保留两位小数
    QString PercentageText = QString::number(Percentage, 'f', 2);
    ui->StatusLabel->setText(QString("总大小 **%1** 剩余 **%2** 目前进度 **%3%**").arg(QString::number(SizeTotal), QString::number(SizeRemain), PercentageText));
}
