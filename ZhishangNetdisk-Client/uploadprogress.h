#ifndef UPLOADPROGRESS_H
#define UPLOADPROGRESS_H

#include <QWidget>

namespace Ui {
class UploadProgress;
}

class UploadProgress : public QWidget
{
    Q_OBJECT

public:
    explicit UploadProgress(QWidget *parent = nullptr);
    ~UploadProgress();
    void SetUploadProgressRange(int minimum, int maximum);
    void SetUploadProgress(int value);
    void SetTitle(QString FileName, QString Path);
    void SetStatus(int SizeTotal, int SizeRemain, double Percentage);

private:
    Ui::UploadProgress *ui;
};

#endif // UPLOADPROGRESS_H
