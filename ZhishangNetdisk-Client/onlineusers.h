#ifndef ONLINEUSERS_H
#define ONLINEUSERS_H

#include <QWidget>
#include <QMessageBox>
#include <QListWidgetItem>

namespace Ui {
class OnlineUsers;
}

class OnlineUsers : public QWidget
{
    Q_OBJECT

public:
    ~OnlineUsers();
    explicit OnlineUsers(QWidget *parent = nullptr);
    void SetOnlineUserList(const QStringList OnlineUserList);
    void ClearOnlineUserList();
    void SetTitle(const QString &UserName);

private slots:
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::OnlineUsers *ui;

};

#endif // ONLINEUSERS_H
