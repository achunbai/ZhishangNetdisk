#ifndef DOWNLOADPROGRESS_H
#define DOWNLOADPROGRESS_H

#include <QWidget>

namespace Ui {
class DownloadProgress;
}

class DownloadProgress : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadProgress(QWidget *parent = nullptr);
    ~DownloadProgress();
    void SetDownloadProgressRange(int minimum, int maximum);
    void SetDownloadProgress(int value);
    void SetTitle(QString FileName, QString Path);
    void SetStatus(int SizeTotal, int SizeRemain, double Percentage);

private:
    Ui::DownloadProgress *ui;
};

#endif // DOWNLOADPROGRESS_H
