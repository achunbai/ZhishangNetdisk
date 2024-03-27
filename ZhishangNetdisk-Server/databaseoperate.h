#ifndef DATABASEOPERATE_H
#define DATABASEOPERATE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QSqlQuery>

class DatabaseOperate : public QObject
{
    Q_OBJECT
public:
    static DatabaseOperate& GetInstance();
    // 数据库对象通过析构函数关闭
    ~DatabaseOperate();
    // 定义连接函数
    void ConnectDB();

    // 处理注册的函数
    bool ProcessRegister(const char* UserName, const char* Password, int &UID);

    // 处理登录的函数
    bool ProcessLogin(const char* UserName, const char* Password, int &UID);

    // 处理下线函数
    void ProcessOffline(const char* UserName);

    // 处理查找用户函数
    int ProcessSearchUser(const char* UserName);

    // 处理在线用户列表
    QStringList ProcessOnlineUsersList();

    // 处理好友关系，查找是否为好友和是否在线
    int ProcessFriendSearch(const char *CurrentUserName, const char *TargetUserName);

    // 执行添加好友
    bool ProcessAddFriend(const char *CurrentUserName, const char *TargetUserName);

    // 执行查询好友列表
    QStringList ProcessFriendList(const char *UserName, QVector<int>& Online, bool& status);

    // 执行删除好友
    bool ProcessDeleteFriend(const char *CurrentUserName, const char *TargetUserName);

    // 设置参数
    void SetMySQLHost(QString MySQLHost);
    void SetMySQLPort(qint16 MySQLPort);
    void SetMySQLDatabase(QString MySQLDatabaseName);
    void SetMySQLUsername(QString MySQLUsername);
    void SetMySQLPassword(QString MySQLPassword);

private:
    // 实现单例模式
    explicit DatabaseOperate(QObject *parent = nullptr);
    DatabaseOperate(const DatabaseOperate& instance) = delete;
    DatabaseOperate& operator=(const DatabaseOperate&) = delete;
    // 定义数据仓库对象，一个QSqlDatabase对象箱单与一个数据库，通常在成员变量中声明
    // 一个工程中，通常只要一个对象即可
    QSqlDatabase MySQLDatabase;

    // 配置参数，从配置文件中读取
    QString MySQLHost;
    qint16 MySQLPort;
    QString MySQLDatabaseName;
    QString MySQLUsername;
    QString MySQLPassword;

signals:
};

#endif // DATABASEOPERATE_H
