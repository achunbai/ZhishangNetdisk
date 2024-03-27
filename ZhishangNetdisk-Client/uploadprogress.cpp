#include "uploadprogress.h"
#include "ui_uploadprogress.h"

UploadProgress::UploadProgress(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UploadProgress)
{
    ui->setupUi(this);
    ui->UploadProgressBar->setValue(0);
}

UploadProgress::~UploadProgress()
{
    delete ui;
}

void UploadProgress::SetUploadProgressRange(int minimum, int maximum)
{
    ui->UploadProgressBar->setRange(minimum, maximum);
}

void UploadProgress::SetUploadProgress(int value)
{
    ui->UploadProgressBar->setValue(value);
}

void UploadProgress::SetTitle(QString FileName, QString Path)
{
    setWindowTitle(QString("正在将 %1 上传至 %2").arg(FileName, Path));
    ui->TitleLabel->setText(QString("正在将 **%1** 上传至 **%2**").arg(FileName, Path));
}

void UploadProgress::SetStatus(int SizeTotal, int SizeRemain, double Percentage)
{
    // 转换为百分比
    Percentage *= 100;
    // 转换为字符串，保留两位小数
    QString PercentageText = QString::number(Percentage, 'f', 2);
    ui->StatusLabel->setText(QString("总大小 **%1** 剩余 **%2** 目前进度 **%3%**").arg(QString::number(SizeTotal), QString::number(SizeRemain), PercentageText));
}
