#ifndef FRIENDS_H
#define FRIENDS_H

#include "onlineusers.h"
#include "chat.h"
#include <QWidget>

namespace Ui {
class Friends;
}

class Friends : public QWidget
{
    Q_OBJECT

public:
    explicit Friends(QWidget *parent = nullptr);
    OnlineUsers *GetOnlineUsersList();
    ~Friends();
    void SetOnlineUserList(const QStringList UserList, const int UserNumber);
    void SetFriendList(const QStringList FriendList, const QVector<int> Online, const int FriendNumber, const int OnlineNumber);
    void ProcessSetFriendList(const QStringList FriendList, const QVector<int> Online);
    void SendRefreshFriendListRequest();
    void SetTitle(const QString &UserName);
    void SetMsg(const char *UserName, const char *Message);
    QListWidget *GetFriendsLW();

private slots:
    void on_SearchUserPB_clicked();

    void on_OnlineUsersPB_clicked();

    void on_RefreshPB_clicked();

    void on_DeletePB_clicked();

    void on_ChatPB_clicked();

private:
    Ui::Friends *ui;
    OnlineUsers *OnlineUsersList;
    Chat *ChatWindow;
};

#endif // FRIENDS_H
