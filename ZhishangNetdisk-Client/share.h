#ifndef SHARE_H
#define SHARE_H

#include <QWidget>

namespace Ui {
class Share;
}

class Share : public QWidget
{
    Q_OBJECT

public:
    explicit Share(QWidget *parent = nullptr);
    ~Share();
    void SetTitle(QString FileName);
    void SetTargetFilePath(QString CurrentFilePath);
    void RefreshFriendList();

private slots:
    void on_SelectAllPB_clicked();

    void on_CancelSelectPB_clicked();

    void on_ConfirmPB_clicked();

private:
    Ui::Share *ui;
    QString TargetFileName;
    QString TargetFilePath;
};

#endif // SHARE_H
